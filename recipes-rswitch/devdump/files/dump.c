// gcc -Wall -O2 dump.c -o dump

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/mman.h>

int main(int argc, char **argv)
{
	int fd;
	int page_size;
	char *map;
	off_t offset;
	
	long addr;
	int cnt;
	int width;
	int start, lines, end, lend;
	int o;
	
	
	if (!argv[1]) {
		fprintf(stderr, "%s addr ?cnt? ?1|2|4?\n"
			"Dump cnt elements of 1|2|4 bytes from address addr\n",
			argv[0]);
		return -1;
	}
	addr = strtoul(argv[1], NULL, 0); /* allows hex, oct etc */
	cnt = 4;
	width = 4;

	if (argv[2]) {
		cnt = strtoul(argv[2], NULL, 0); /* allows hex, oct etc */
		if (cnt <= 0 || cnt > 0x1000) {
			fprintf(stderr, "Error: Parameter cnt=%d is out of range\n", cnt);
			return 1;
		}
		if (argv[3]) {
			width = strtoul(argv[3], NULL, 0); /* allows hex, oct etc */
			if (width != 1 && width != 2 && width != 4) {
				fprintf(stderr, "Error: Parameter with=%d is out of range\n", width);
				return 1;
			}
		}
	}

	if (width != 1 && (addr & (width-1) || (addr+cnt) & (width-1))) {
		fprintf(stderr, "Error: For %d bit access address=%08lx to %08lx needs to be 2 byte alligned\n", width*8, addr, addr+cnt);
		return -1;
	}
	
	fd = open("/dev/mem", (O_RDONLY | O_SYNC));
	if (fd < 0) {
		fprintf(stderr, "Error: Cannot open \"/dev/mem\" for reading\n");
		return -1;
	}
	page_size = getpagesize();

	lines = ((cnt+((addr & 0x0f) / width)+15)/16);
	
	if (width==4) 
		printf(" Address    +00        +04        +08        +0C\n"
		       "------------------------------------------------------\n");
	
	while (lines) {

		offset = addr & ~(page_size-1);
		o = addr & (page_size-1);
		start = (addr & 0x0f) / width;
		lend = ((addr + cnt -1) & 0x0f) / width;
		//printf("  addr=%lx  page_size=%x  offset=%lx  o=%x  lines=%d  start=%d  lend=%d\n", addr, page_size, offset, o, lines, start, lend);

		map = mmap(NULL, page_size, PROT_READ, MAP_SHARED, fd, offset);
		if (map == MAP_FAILED) {
			fprintf(stderr, "Error: Cannot open map address %lx to %lx\n", offset, offset+page_size);
			return -1;
		}
		
		end = 16;
		for (int i=0; lines && o<page_size; i++) {
			if (i+1 == lines)
				end = lend;
			//printf("  start=%d  lines=%d  end=%d  addr=%lx\n", start, lines, end, addr);
			printf("%04lx_%04lx: ", addr>>16, addr&0xfff0);
			for (int j=0; j<16/width; j++) {
				if (width==4) {
					uint32_t v = ((int32_t*)&map[o])[0];
					printf((j<start || j>end)?"           ":" %04x_%04x ", v>>16, v&0xffff);
				}
				else if (width==2) {
					printf((j<start || j>end)?"     ":" %04x", ((int16_t*)&map[o])[0]&0xffff);
					if (j%4==3)
						printf(" ");
				}
				else {
					printf((j<start || j>end)?"   ":" %02x", map[o]&0xff);
					if (j%4==3)
						printf(" ");
				}
				o += width;
			}

			if (width==1) {
				o -= 16;
				printf(" ");
				for (int j=0; j<16/width; j++) {
					unsigned char c = map[o];
					if (c < 0x20 || c >= 0x7f) c = '.';
					printf((j<start || j>end)?" ":"%c", c);
					if (j%8==7)
						printf(" ");
					o++;
				}
			}


			printf("\n");
			start = 0;
			addr += 16;
			lines--;
		}

		if (munmap(map, page_size) == -1) {
			fprintf(stderr, "Error: Cannot unmap address %lx\n", offset);
			return -1;
		}
	}
	
	
	close(fd);
	return 0;
}
