/*
 * NVCR utility (using spidev driver)
 *
 * Small utility to check and set NVCR on a Micro  MT25QU flash device
 * Copyright (c) 2020  Renesas Electronic
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * Cross-compile with cross-gcc -I/path/to/cross-kernel/include
 *
 */

#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define MT25_READ_ID 			(0x9E)
#define MT25_WRITE_ENABLE		(0x06)
#define MT25_WRITE_DISABLE		(0x04)
#define MT25_READ_NV_CONF_REG 	(0xB5)
#define MT25_WRITE_NV_CONF_REG 	(0xB1)


static void pabort(const char *s)
{
	perror(s);
	abort();
}

static const char *device = "/dev/spidev0.1";
static uint8_t mode = 0;
static uint8_t bits = 8;
static uint32_t speed = 1000000;
static uint16_t delay = 0;
static uint32_t nvcr_val = 0u;
static bool is_nvcr_val_valid = false;
static bool check_only = false;
static uint8_t tx_data_buf[256];
static uint8_t rx_data_buf[256];

static bool check_dev_id(uint8_t *rx_buf)
{
    return (rx_buf[0] == 0x20) || (rx_buf[1] == 0xBB) || (rx_buf[3] == 0x21); 
}



static int do_spi_transfer(int fd, uint8_t *tx_buf, size_t tx_len,
		                           uint8_t *rx_buf, size_t rx_len)
{
	int ret;
	int buf_pos;

	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx_buf,
		.rx_buf = (unsigned long)rx_buf,
		.len = (tx_len + rx_len),
		.delay_usecs = delay,
		.speed_hz = 0,
		.bits_per_word = 0,
		.cs_change = false,
	};

//	printf("DEBUG: tx_buf: %p    tx_len: %d     rx_buf: %p     rx_len: %d\n", tx_buf, tx_len, rx_buf, rx_len);
//	printf("DEBUG: tx_buf + tx_len: %p \n", tx_buf + tx_len);
        
        if(rx_len > 0) {
            memset((tx_buf + tx_len), 0xff, rx_len);
	}


//	printf("Send:     ");
//	fflush(stdout);
//	for(buf_pos = 0; buf_pos < (tx_len + rx_len); buf_pos++)
//	{
//		printf("0x%.2X ", tx_buf[buf_pos]);
//	}
//	printf("\n");


	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
//	printf("%d bytes transferred\n", ret);
	errno = 0;

	if(errno != 0)
		return -1;

//	if(rx_len > 0) {
//
//		printf("Received: ");
//	    for (buf_pos = tx_len; buf_pos < (tx_len + rx_len); buf_pos++) {
//	    	if ((buf_pos != 0) && (!(buf_pos % 8)))
//	    		puts("");
//	    	printf("0x%.2X ", rx_buf[buf_pos]);
//	    }
//	    puts("");
//	}
//	else {
//	    printf("No receive data\n");
//	}

	return 0;
}


void print_usage(const char *prog)
{
	printf("Usage: %s [-Dcv] <value>\n", prog);
	puts("  -D --device      device to use (default /dev/spidev0.1)\n"
		 "  -c --check-only  Only check NVCR, do not write\n"
		 "  -v --value       Value to write to NVCR\n");

	exit(1);
}

void parse_opts(int argc, char *argv[])
{
	while (1) {
		static const struct option lopts[] = {
			{ "device",  1, 0, 'D' },
			{ "echo",    0, 0, 'c' },
			{ "value",   1, 0, 'v' },
			{ NULL, 0, 0, 0 },
		};
		int c;

		c = getopt_long(argc, argv, "D:cv:", lopts, NULL);

		if (c == -1)
			break;

		switch (c) {
		case 'D':
			device = optarg;
			break;
		case 'c':
			check_only = true;
			break;
		case 'v':
		{
			nvcr_val =  strtol(optarg, NULL, 16);
			if(nvcr_val > 0xFFFF) {
				printf("Invalid nvcr value!\n");
				exit(-1);
			}
			else {
				is_nvcr_val_valid = true;
			}

		}
		break;
		default:
			print_usage(argv[0]);
			break;
		}
	}
}

int main(int argc, char *argv[])
{
	int ret = 0;
	int fd;

	memset(tx_data_buf, 0, sizeof(tx_data_buf));
	memset(rx_data_buf, 0, sizeof(rx_data_buf));

	parse_opts(argc, argv);

	fd = open(device, O_RDWR);
	if (fd < 0)
		pabort("can't open device");

	/*
	 * spi mode
	 */
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		pabort("can't get spi mode");

	/*
	 * bits per word
	 */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");

	/*
	 * max speed hz
	 */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");

//	printf("spi mode: %d\n", mode);
//	printf("bits per word: %d\n", bits);
//	printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);


	if((is_nvcr_val_valid == true) || (check_only == true))
	{
		size_t rx_len, tx_len;
		bool is_dev_valid = false;
		uint32_t actual_nvcr_val;
		int ret;

		/* Read device ID */
		memset(tx_data_buf, 0, sizeof(tx_data_buf));
		memset(rx_data_buf, 0, sizeof(rx_data_buf));

		tx_data_buf[0] = MT25_READ_ID;
		tx_len = 1;
		rx_len = 20;


		ret = do_spi_transfer(fd, tx_data_buf, tx_len, rx_data_buf, rx_len);
		if(ret != 0) {
			printf("SPI transaction error\n");
			close(fd);

			exit(-1);
		}

		is_dev_valid = check_dev_id(rx_data_buf);
		if(is_dev_valid != true) {
			printf("Could not identify MT25QU01G device\n");
			close(fd);

			exit(-1);
		}

		/* Read NVCR */
		memset(tx_data_buf, 0, sizeof(tx_data_buf));
		memset(rx_data_buf, 0, sizeof(rx_data_buf));

		tx_data_buf[0] = MT25_READ_NV_CONF_REG;
		tx_len = 1;
		rx_len = 2;
		ret = do_spi_transfer(fd, tx_data_buf, tx_len, rx_data_buf, rx_len);
		if(ret != 0) {
			printf("SPI transaction error\n");
			close(fd);
			exit(-1);
		}

		actual_nvcr_val = (rx_data_buf[tx_len] | (rx_data_buf[(tx_len + 1)] << 8));
		printf("Got NVCR     : 0x%.4x\n", actual_nvcr_val);
		printf("Expected NVCR: 0x%.4x\n", nvcr_val);
		if(actual_nvcr_val == nvcr_val) {
			printf("NVCR already matches expected value.\n");
			close(fd);
			exit(0);
		}
		else {
			printf("NVCR needs to be updated.\n");                
                }

		if(check_only != true) {
			/* Enable write */
			memset(tx_data_buf, 0, sizeof(tx_data_buf));
			memset(rx_data_buf, 0, sizeof(rx_data_buf));

			tx_data_buf[0] = MT25_WRITE_ENABLE;
			tx_len = 1;
			rx_len = 0;
			ret = do_spi_transfer(fd, tx_data_buf, tx_len, rx_data_buf, rx_len);
			if(ret != 0) {
				printf("SPI transaction error\n");
				close(fd);
				exit(-1);
			}
		

		    /* Write NVCR */
		    memset(tx_data_buf, 0, sizeof(tx_data_buf));
		    memset(rx_data_buf, 0, sizeof(rx_data_buf));

		    tx_data_buf[0] = MT25_WRITE_NV_CONF_REG;
		    tx_data_buf[1] = (nvcr_val & 0xFF);
		    tx_data_buf[2] = ((nvcr_val >> 8) & 0xFF);
		    tx_len = 3;
		    rx_len = 0;
		    ret = do_spi_transfer(fd, tx_data_buf, tx_len, rx_data_buf, rx_len);
		    if(ret != 0) {
	    		printf("SPI transaction error\n");
			close(fd);
			exit(-1);
		    }
		    usleep(500000);

		    /* Read back NVCR */
		    memset(tx_data_buf, 0, sizeof(tx_data_buf));
		    memset(rx_data_buf, 0, sizeof(rx_data_buf));

		    tx_data_buf[0] = MT25_READ_NV_CONF_REG;
		    tx_len = 1;
		    rx_len = 2;
		    ret = do_spi_transfer(fd, tx_data_buf, tx_len, rx_data_buf, rx_len);
		    if(ret != 0) {
		   	printf("SPI transaction error\n");
			close(fd);
			exit(-1);
		    }
  		    actual_nvcr_val = (rx_data_buf[tx_len] | (rx_data_buf[(tx_len + 1)] << 8));

		    printf("Got NVCR: 0x%.4x\n", actual_nvcr_val);
		    if(actual_nvcr_val == nvcr_val) {
			printf("NVCR written successfully.\n");
		    }
		    else {
			printf("NVCR update failed.\n");
		   }
             }


	}

	close(fd);

	return ret;
}

