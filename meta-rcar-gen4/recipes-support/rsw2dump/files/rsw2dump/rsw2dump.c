#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/mman.h>

#define RSW2_BASE_FPGA   0xC9040000   //VC3
#define RSW2_BASE_S4     0xe68c0000   //VC4
#define RSW2_SIZE        0x00020000

#define RSW2_BASE_SERDES 0xE6444000   //VC4
#define RSW2_SIZE_SERDES 0x1000

#include "rswitch.h"
#include "vt100.hh"

#define RSWITCH_MAX_NUM_GWCA	(RSWITCH_NUM_HW-RSWITCH_MAX_NUM_ETHA)

class c_tty tty;

static volatile uint32_t *rsw2;
static volatile uint32_t *rsw2_serdes;

static int ports = RSWITCH_MAX_NUM_ETHA;
static int do_rxtx = 0;
static int do_eth_error = 0;
static int do_eth_cnt = 0;
static int do_eth_conf = 0;
static int do_l2 = 0;
static int do_vlan = 0;
static int do_l3 = 0;
static int do_base = 0;
static int do_all = 0;

static int do_chains = 0;

static int rst_serdes = 0;

//------------------------------------------------------------------------------
static void usage(char *argv0)
{
	fprintf(stderr, "%s ?options? what\n"
	    "Dumps the RSW2 status\n"
	    "  Options:\n"
	    "    -f          Use VC3/FPGA base address %08x instead of S4 %08x\n"
	    "  What\n"
	    "    rxtx        Selected Rx/Tx counters of Ethernet ports\n"
	    "    rxtx-all    All x/Tx counters of Ethernet ports\n"
	    "    eth-err     Error flags of Ethernet ports\n"
	    "    eth-cnt     Counters of Ethernet ports\n"
	    "    eth-conf    Configuration of Ethernet ports\n"
	    "\n"
	    "    base        Basic forwarding configuration\n"
	    "    l2          Dump MAC forwarding table\n"
	    "    vlan        Dump VLAN table\n"
	    "    l3          Dump stream forwarding table\n"
		"\n"
		"    rst-serdes  Reset the SERDES channels\n"
	    , argv0
	    , RSW2_BASE_FPGA, RSW2_BASE_S4
	);
	exit(EXIT_FAILURE);
}


//------------------------------------------------------------------------------
static inline uint32_t read32(int offset, int idx)
{
	//printf("\nread32(%x, %x) as rsw2[%x / %x]\n", offset, idx, offset/4 + idx, (offset/4 + idx)*4); fflush(stdout);
	//usleep(10000);
	return rsw2[offset/4 + idx];
}

static inline void write32(int offset, int idx, uint32_t data)
{
	//printf("\nwrite32(%x, %x, %x) to rsw2[%x / %x]\n", offset, idx, data, offset/4 + idx, (offset/4 + idx)*4); fflush(stdout);
	//usleep(10000);
	rsw2[offset/4 + idx] = data;
}


static int waitPoll(int offset, int idx, int shift, int mask, int exp)
{
	uint32_t value;
	for (int i = 0; i < 500; i++) {
		value = read32(offset, idx);
		if (((value >> shift) & mask) == exp)
			return value;
		usleep(100);
	}
	fprintf(stderr, "Error timeout when read rsw2[%x / %x].  value=%08x  shift=%d  mask=%08x  exp=%0x8\n",
		offset/4 + idx, (offset/4 + idx)*4, value, shift, mask, exp);
	exit(-1);
}


//------------------------------------------------------------------------------
#define S_LABEL 20
#define S_PORT   6
#define LABEL "%-20s"
#define F_XNUM  " %8x"
#define F_DNUM  " %8d"
#define F_STR   " %8s"

static char * portnames[10];
static char * sbuf[10];
static char headerLine[1000];
static int  first = 1;

//------------------------------------------------------------------------------
static void print_strings(const char * title, char ** s, const char **colour, int withCPU, const char *comment)
{
	printf(LABEL, title);
	for (int i=0; i<ports+ withCPU; i++) {
		if (colour && colour[i])
			printf("%s" F_STR "%s", colour[i], s[i], tty.norm());
		else
			printf(F_STR, s[i]);
	}
	if (comment)
		printf("  %s\n", comment);
	else
		printf("\n");
}

static void print_regs_D(const char * title, int reg, int incr, int withCPUreg, const char *comment)
{
	int i;
	int p = ports;
	if (withCPUreg < 0)
		p -= withCPUreg;

	printf(LABEL, title);
	for (i=0; i<p; i++)
		printf(F_DNUM, read32(reg, (incr/4)*i));
	for (; i<RSWITCH_NUM_HW; i++) {
		if (withCPUreg>0)
			printf(F_DNUM, read32(withCPUreg, (incr/4)*(i-p)));
		else if (withCPUreg==0)
			printf(F_STR, "");
	}
	if (comment)
		printf("  %s\n", comment);
	else
		printf("\n");
}

static void print_regs_H(const char * title, int reg, int incr, int withCPUreg, const char *comment)
{
	int p = ports;
	if (withCPUreg < 0)
		p -= withCPUreg;

	printf(LABEL, title);
	for (int i=0; i<p; i++)
		printf(F_XNUM, read32(reg, (incr/4)*i));
	if (withCPUreg>0)
		printf(F_XNUM, read32(withCPUreg, 0));
	else if (withCPUreg==0)
		printf(F_STR, "");
	if (comment)
		printf("  %s\n", comment);
	else
		printf("\n");
}

static void print_regs_D_bits(const char * title, int reg, int incr, int withCPUreg, int shift, int mask)
{
	printf(LABEL, title);
	for (int i=0; i<ports; i++)
		printf(F_DNUM, (read32(reg, (incr/4)*i)>>shift)&mask);
	printf("\n");
}

static void print_regs_H_bits(const char * title, int reg, int incr, int withCPUreg, int shift, int mask)
{
	printf(LABEL, title);
	for (int i=0; i<ports; i++)
		printf(F_DNUM, (read32(reg, (incr/4)*i)>>shift)&mask);
	printf("\n");
}


//------------------------------------------------------------------------------
static char * getMACstr(char *buf, uint64_t mac)
{
	char *s = buf;
	for (int i=0; i<6; i++) {
		s += sprintf(s, ":%02lX", (mac>>40)&0xff);
		mac <<= 8;
	}
	return buf+1;
}


//------------------------------------------------------------------------------
static char *fillPortVector(char*buf, int value, uint32_t *csdn)
{
	memset(buf, '.', RSWITCH_NUM_HW);
	buf[RSWITCH_NUM_HW+1] = 0;
	for (int i=0; i<RSWITCH_NUM_HW; i++) {
		if (value & 1)
			buf[i] = i + ((i<ports)?'0':('A'-ports));
		value >>= 1;
	}
	if (csdn != NULL) {
		char *s = buf + RSWITCH_NUM_HW;
		for (int i=ports; i<RSWITCH_NUM_HW; i++) {
			s += sprintf(s, " %-3d", csdn[i-ports]);
		}
	}
	return buf;
}


//------------------------------------------------------------------------------
static void print_rxtx(int longReport)
{
	if (!first)
		printf("\n");
	first = 0;
	print_strings("", portnames, NULL, RSWITCH_MAX_NUM_GWCA, NULL);
	printf("%s\n", headerLine);

	print_regs_D("Rx (eMAC)", MRGFCE, RSWITCH_ETHA_SIZE,  GWTDCN, "from CPU");
	print_regs_D("Rx (pMAC)", MRGFCP, RSWITCH_ETHA_SIZE, 0, NULL);

	/*if (longReport) {
		rxtx->ebyte[port] += (portreg_read(ndev, MRXBCEU) << 32) |
				     portreg_read(ndev, MRXBCEL);
		rxtx->pbyte[port] += (portreg_read(ndev, MRXBCPU) << 32) |
				     portreg_read(ndev, MRXBCPL);
		rxtx->bframe[port] += portreg_read(ndev, MRBFC);
		rxtx->mframe[port] += portreg_read(ndev, MRMFC);
		rxtx->uframe[port] += portreg_read(ndev, MRUFC);

		proc_print_u32(m, "  Broadcast    ", " %10d", rxtx->bframe,
			       board_config.eth_ports, "");
		proc_print_u32(m, "  Multicast    ", " %10d", rxtx->mframe,
			       board_config.eth_ports, "");
		proc_print_u32(m, "  Unicast      ", " %10d", rxtx->uframe,
			       board_config.eth_ports, "");
	}*/

	print_regs_D("Rx CRC Error", MRFMEFC, RSWITCH_ETHA_SIZE, 0, NULL);
	print_regs_D("Rx Phy Error", MRPEFC, RSWITCH_ETHA_SIZE, 0, NULL);
	print_regs_D("Rx RMAC Filter", MRRCFEFC, RSWITCH_ETHA_SIZE, 0, NULL);

	/*if (longReport) {
		rxtx->nibbleerror[port] += portreg_read(ndev, MRNEFC);
		rxtx->undersizeerrorgood[port] += portreg_read(ndev, MRGUEFC);
		rxtx->undersizeerrorbad[port] += portreg_read(ndev, MRBUEFC);
		rxtx->oversizeerrorgood[port] += portreg_read(ndev, MRGOEFC);
		rxtx->oversizeerrorbad[port] += portreg_read(ndev, MRBOEFC);
		rxtx->fragmisserror[port] += portreg_read(ndev, MRFFMEFC);
		rxtx->cfragerror[port] += portreg_read(ndev, MRCFCEFC);
		rxtx->fragerror[port] += portreg_read(ndev, MRFCEFC);

		proc_print_u32(m, "Nibble Error   ", " %10d",
			       rxtx->nibbleerror, board_config.eth_ports, "");
		proc_print_u32(m, "Undersize Good ", " %10d",
			       rxtx->undersizeerrorgood, board_config.eth_ports, "");
		proc_print_u32(m, "Undersize Bad  ", " %10d",
			       rxtx->undersizeerrorbad, board_config.eth_ports, "");
		proc_print_u32(m, "Oversize Good  ", " %10d",
			       rxtx->oversizeerrorgood, board_config.eth_ports, "");
		proc_print_u32(m, "Oversize Bad   ", " %10d",
			       rxtx->oversizeerrorbad, board_config.eth_ports, "");
		proc_print_u32(m, "Frag Miss      ", " %10d",
			       rxtx->fragmisserror, board_config.eth_ports, "");
		proc_print_u32(m, "C Frag Error   ", " %10d", rxtx->cfragerror,
			       board_config.eth_ports, "");
		proc_print_u32(m, "Frag Error     ", " %10d", rxtx->fragerror,
			       board_config.eth_ports, "");
		proc_print_u64(m, "Rx bytes (eMAC)", " %10d", rxtx->ebyte,
			       board_config.eth_ports);
		proc_print_u64(m, "Rx bytes (pMAC)", " %10d", rxtx->pbyte,
			       board_config.eth_ports);
	}*/

	print_regs_D("Tx (eMAC)", MTGFCE, RSWITCH_ETHA_SIZE, GWRDCN, "to CPU");
	print_regs_D("Tx (pMAC)", MTGFCP, RSWITCH_ETHA_SIZE, 0, NULL);

	/*if (longReport) {
		rxtx->txebyte[port] += (portreg_read(ndev, MTXBCEU) << 32) |
				       portreg_read(ndev, MTXBCEL);
		rxtx->txpbyte[port] += (portreg_read(ndev, MTXBCPU) << 32) |
				       portreg_read(ndev, MTXBCPL);
		rxtx->txbframe[port] += portreg_read(ndev, MTBFC);
		rxtx->txmframe[port] += portreg_read(ndev, MTMFC);
		rxtx->txuframe[port] += portreg_read(ndev, MTUFC);

		proc_print_u32(m, "  Broadcast    ", " %10d", rxtx->txbframe,
			       board_config.eth_ports, "");
		proc_print_u32(m, "  Multicast    ", " %10d", rxtx->txmframe,
			       board_config.eth_ports, "");
		proc_print_u32(m, "  Unicast      ", " %10d", rxtx->txuframe,
			       board_config.eth_ports, "");
	}*/

	print_regs_D("Tx Errors", MTEFC, RSWITCH_ETHA_SIZE, 0, NULL);

	/*if (longReport) {
		proc_print_u64(m, "Tx bytes (eMAC)", " %10d", rxtx->txebyte,
			       board_config.eth_ports);
		proc_print_u64(m, "Tx bytes (pMAC)", " %10d", rxtx->txpbyte,
			       board_config.eth_ports);
	}*/

}


//------------------------------------------------------------------------------
static void print_eth_error(int longReport)
{
	if (!first)
		printf("\n");
	first = 0;
	print_strings("", portnames, NULL, 0, NULL);
	printf("%s\n", headerLine);

	printf("ECC Errors in\n");
	print_regs_D_bits("  Data", EAEIS0, RSWITCH_ETHA_SIZE, 0, 0, 1);
	print_regs_D_bits("  TAG", EAEIS0, RSWITCH_ETHA_SIZE, 0, 1, 1);
	print_regs_D_bits("  PTR", EAEIS0, RSWITCH_ETHA_SIZE, 0, 2, 1);
	print_regs_D_bits("  DESC", EAEIS0, RSWITCH_ETHA_SIZE, 0, 3, 1);
	print_regs_D_bits("  L2/3 Update", EAEIS0, RSWITCH_ETHA_SIZE, 0, 4, 1);

	printf("Frame acceptance Errors\n");
	print_regs_D_bits("  Too small frame", EAEIS0, RSWITCH_ETHA_SIZE, 0, 5, 1);
	print_regs_D_bits("  TAG Filtering", EAEIS0, RSWITCH_ETHA_SIZE, 0, 6, 1);
	print_regs_D_bits("  Checksum", EAEIS0, RSWITCH_ETHA_SIZE, 0, 7, 1);

	printf("Errors for all 8 queues individually\n");
	print_regs_D_bits("  Frame Size", EAEIS0, RSWITCH_ETHA_SIZE, 0, 8, 0xff);
	print_regs_D_bits("  Queue Overflow", EAEIS2, RSWITCH_ETHA_SIZE, 0, 0, 0xff);
	print_regs_D_bits("  CT Queue Overflow", EAEIS2, RSWITCH_ETHA_SIZE, 0, 8, 0xff);
	print_regs_D_bits("  Security error", EAEIS2, RSWITCH_ETHA_SIZE, 0, 16, 0xff);
}


//------------------------------------------------------------------------------
static void print_eth_cnt(int longReport)
{
	char s[100];
	if (!first)
		printf("\n");
	first = 0;
	print_strings("", portnames, NULL, 0, NULL);
	printf("%s\n", headerLine);

	printf("Pending frames in Queue (transient)\n");
	for (int j = 0; j < 8; j++) {
		sprintf(s, "  Priority #%d", j);
		print_regs_D(s, EATDQM0+(j*4), RSWITCH_ETHA_SIZE, 0, NULL);
	}

	print_regs_D("Minimum Frame Size", EAUSMFSECN, RSWITCH_ETHA_SIZE, 0, NULL);
	print_regs_D("Tag Filtering", EATFECN, RSWITCH_ETHA_SIZE, 0, NULL);
	print_regs_D("Frame Size", EAFSECN, RSWITCH_ETHA_SIZE, 0, NULL);
	print_regs_D("Queue Overflow", EADQOECN, RSWITCH_ETHA_SIZE, 0, NULL);
	print_regs_D("Security error", EADQSECN, RSWITCH_ETHA_SIZE, 0, NULL);
	print_regs_D("Checksum Error", EACKSECN, RSWITCH_ETHA_SIZE, 0, NULL);
}

//------------------------------------------------------------------------------
static void action_rst_serdes()
{
	for (int i=0; i<3; i++) {
		printf("RX_RST on channel %d\n", i);
		//VR_XS_PMA_MP_12G_16G_25G_RX_GENCTRL1.RX_RST = 1
		rsw2_serdes[(i*0x400+0x3fc)/4] = 0x180;  //BANK 180
		rsw2_serdes[(i*0x400+0x144)/4] |= 0x0010;
		usleep(10);
		rsw2_serdes[(i*0x400+0x144)/4] &= 0xFFEF;
	}
}

//------------------------------------------------------------------------------
static void print_eth_conf(int longReport)
{
	char *s;
	uint32_t value;
	static const char *pis[] = {"MII", "001", "GMII", "011", "XG", "101", "110", "111"};
	static const char *lsc[] = {"000", "100M", "1G", "2G5", "100", "101", "110", "111"};
	const char * colour[10];

	if (!first)
		printf("\n");
	first = 0;
	print_strings("", portnames, NULL, 0, NULL);
	printf("%s\n", headerLine);

	print_regs_H_bits("LocAddFragSize", EATPEC, RSWITCH_ETHA_SIZE, 0, 16, 3);
	print_regs_H_bits("Rx Preemption Sup", MMIS0, RSWITCH_ETHA_SIZE, 0, 2, 1);
	//print_regs_H("MAC Config", MPIC, RSWITCH_ETHA_SIZE, 0, NULL);
	for (int i=0; i<ports; i++) {
		value = read32(MPIC+RSWITCH_ETHA_SIZE*i, 0);
		sprintf(sbuf[i], "%s/%s", pis[value&7], lsc[(value>>3)&7]);
		colour[i] = ((value&7)==4)?tty.L_red():NULL;
	}
	print_strings("MAC MII Config", sbuf, NULL, 0, NULL);
	if (rsw2_serdes) {
		for (int i=0; i<ports; i++) {
			rsw2_serdes[(i*0x400+0x3fc)/4] = 0x300;  //BANK_300
			value = rsw2_serdes[(i*0x400+0x004)/4];
			if (value&0x04) {
				strcpy(sbuf[i], "up");
				if (colour[i])  //only colour on XGMII
					colour[i] = tty.green();
			}
			else
				strcpy(sbuf[i], "down");
		}
		print_strings("SERDES", sbuf, colour, 0, NULL);
	}
/*
	for( queue = 0; queue < RENESAS_RSWITCH2_TX_QUEUES; queue++) {
		seq_printf(m, "%-2s","");
		strcpy(qstr,"");
		sprintf(qstr, "Q%d",queue);
		seq_printf(m, "%-8s",qstr);
		seq_printf(m, "%-8s","MAC");
		for (port = 0; port < board_config.eth_ports; port++) {
			ndev = ppndev[port];
			eatpec = portreg_read(ndev,EATPEC);
			seq_printf(m, "%-13s",((eatpec >> queue) & 0x01)?"P":"E");
		}
		seq_printf(m,"\n");
		seq_printf(m, "%-10s","");
		seq_printf(m, "%-6s","MFS");
		for (port = 0; port < board_config.eth_ports; port++) {
			ndev = ppndev[port];
			mfs = portreg_read(ndev,EATMFSC0 + (queue * 4));
			seq_printf(m, "%-13u",mfs);
		}
		seq_printf(m,"\n");
		seq_printf(m, "%-10s","");
		seq_printf(m, "%-6s","BW");
		for (port = 0; port < board_config.eth_ports; port++) {
			ndev = ppndev[port];
			mdp = netdev_priv(ndev);
			cbs = portreg_read(ndev,EACOEM);
			if ((ndev->phydev != NULL) && (mdp->link)) {
				strcpy(bwstr,"");
				if((cbs >> queue) & 0x01) {
					civ= portreg_read(ndev,EACOIVM0 + (queue *4));
					civexp = civ & 0xFFFF;
					civexp = civexp * 1000000;
					civexp = civexp / 65535;
					civman = civ >> 16;
					civman =  civman * 1000000;
					bw = civman + civexp;
					bw =  bw * 8;
					bw = bw * config.sysfreqkhz*1000;
					bw =  bw /1000000;
					bwbps = bw;
					bwqg = bwbps / 1000000000;
					bwqm = bwbps / 1000000;
					bwqk = bwbps / 1000;
					if(bwqg > 0) {
						sprintf(bwstr, "%dGbps",bwqg);
					} else if (bwqm > 0){
						bwrm = bwbps % 1000000;
						bwrm = bwrm /1000;
						if(bwrm != 0) {
							sprintf(bwstr, "%d.%03dMbps" ,bwqm, bwrm);
						} else {
							sprintf(bwstr, "%dMbps" ,bwqm);
						}
					} else if (bwqk > 0){
						bwrk = bwbps % 1000;
						if(bwrk != 0) {
							sprintf(bwstr, "%d.%03dKbps" ,bwqk, bwrk);
						} else {
							sprintf(bwstr, "%dKbps" ,bwqk);
						}
					}
					seq_printf(m, "%-13s", bwstr);

				} else {

					seq_printf(m, "%-13s", phy_speed_to_str(mdp->speed));
				}
			} else {
				seq_printf(m, "%-13s", "");
			}
		}
		seq_printf(m,"\n");
		if(longreport) {
			seq_printf(m, "%-10s","");
			seq_printf(m, "%-6s","CIV");
			for (port = 0; port < board_config.eth_ports; port++) {
				ndev = ppndev[port];
				cbs = portreg_read(ndev,EACOEM);
				strcpy(hexstr,"");
				if((cbs >> queue) & 0x01) {
					civ = portreg_read(ndev,EACOIVM0 + (queue *4));
					sprintf(hexstr,"%x_%04x",(civ>> 16) & 0xFFFF, civ & 0xFFFF);
					seq_printf(m, "%-13s",hexstr);
				} else {
					seq_printf(m, "%-13s", "");
				}

			}
			seq_printf(m,"\n");
			seq_printf(m, "%-10s","");
			seq_printf(m, "%-6s","CUL");
			for (port = 0; port < board_config.eth_ports; port++) {
				ndev = ppndev[port];
				cbs = portreg_read(ndev,EACOEM);
				strcpy(hexstr,"");
				if((cbs >> queue) & 0x01) {
					cul = portreg_read(ndev,EACOULM0 + (queue*4));
					sprintf(hexstr,"%04x_%04x", (cul>> 16) & 0xFFFF, cul & 0xFFFF);
					seq_printf(m, "%-13s",hexstr);
				} else {
					seq_printf(m, "%-13s", "");
				}

			}
			seq_printf(m,"\n");
		}
		seq_printf(m,"\n");
	}
*/
}

//------------------------------------------------------------------------------
static void print_l2()
{
	char s[100];
	if (!first)
		printf("\n");
	first = 0;

	int i, entry;
	uint32_t value;
	uint32_t fwmactrr1, fwmactrr2, fwmactrr3, fwmactrr4, fwmactrr6;
	uint32_t csdn[RSWITCH_MAX_NUM_GWCA];
	int found = 0;

	printf("=========== MAC-TABLE =========================================================\n");
	printf("Line        MAC        Mode    Target-Ports   Mirror  SrcVal DstVal IPV\n");
	printf("===============================================================================\n");
	for (entry = 0; entry <= 1023; entry++) {
		write32(FWMACTR, 0, entry);
		value = waitPoll(FWMACTRR0, 0, 31, 1, 0);
		//printf("%d %d %x\n", entry, i, value);
		if ((value & 0x07) == 1) {
			found++;
			fwmactrr1 = read32(FWMACTRR1, 0);
			fwmactrr2 = read32(FWMACTRR2, 0);
			fwmactrr3 = read32(FWMACTRR3, 0);
			fwmactrr4 = read32(FWMACTRR4, 0);
			csdn[0]   = read32(FWMACTRR5, 0);
			csdn[1]   = read32(FWMACTRR5, 1);
			fwmactrr6 = read32(FWMACTRR6, 0);
			//printf("%x %x %x  %x %x %x %x\n", fwmactrr1, fwmactrr2, fwmactrr3, fwmactrr4, csdn[0], csdn[1], fwmactrr6);

			int ageing = (fwmactrr1 >> 11) & 0x01;
			int dynamic = (fwmactrr1 >> 9) & 0x01;
			int cpu_mirror = (fwmactrr6 >> 21) & 0x01;
			int eth_mirror = (fwmactrr6 >> 20) & 0x01;
			int ipv_enable = (fwmactrr6 >> 19) & 0x01;
			int ipv_value = (fwmactrr6 >> 16) & 0xFF;
			int learn_disable = (fwmactrr1 >> 10) & 0x01;
			int security = (fwmactrr1 >> 8) & 0x01;

			if (ipv_enable)
				sprintf(sbuf[4], "%d", ipv_value);
			else
				sprintf(sbuf[4], "-");
			printf("%-4d %s %-7s %-14s %s %s %-6s %-6s %s",
				entry, getMACstr(sbuf[0], ((uint64_t)fwmactrr2<<32)+fwmactrr3),
				dynamic ? ((ageing) ? "Aged" : "Dynamic") : "Static",
				fillPortVector(sbuf[1], fwmactrr6, csdn),
				eth_mirror ? "Eth" : "No ",
				cpu_mirror ? "CPU" : "No ",
				fillPortVector(sbuf[2], fwmactrr4, NULL),
				fillPortVector(sbuf[3], fwmactrr4>>16, NULL),
				sbuf[4]
			);

			//default values are not printed
			if (security)
				printf("   SecurityLevel=%d", security);
			if (learn_disable)
				printf("   HwLearnDisable=%d", learn_disable);
			printf("\n");
		}
	}
	if (found == 0)
		printf(" *** empty ***\n");

	value = read32(FWMACTEM, 0);
	if ((value & 0xffff) != found)
		printf(" !!! found %d entries but %d entries reported (%d secured)\n",
		found, value&0xffff, value>>16);
}


//------------------------------------------------------------------------------
static void print_vlan()
{
	char s[100];
	if (!first)
		printf("\n");
	first = 0;

	int i, entry;
	uint32_t value;
	uint32_t fwvlantsr1, fwvlantsr3;
	uint32_t csdn;
	int empty = 1;
	//~ u8 learn_disable = 0;
	//~ u8 security = 0;
	//~ u32 srclockdpv = 0;
	//~ u32 dpv = 0;
	//~ u32 ipv_value = 0;
	//~ u8 ipv_enable = 0;
	//~ u8 cpu_mirror = 0;
	//~ u8 eth_mirror = 0;

	//~ char portName[10];
	//~ int  portNumber = 10;
	//~ getPortNameString(portName, &portNumber);

	printf("=========== VLAN-TABLE ===========================\n");
	printf("VID   Mirror   Src-Valid     Dst-Valid        IPV\n");
	printf("==================================================\n");
	for (entry = 0; entry <= 0xFFF; entry++) {
		write32(FWVLANTS, 0, entry);
		value = waitPoll(FWVLANTSR0, 0, 31, 1, 0);
		//printf("%d %d %x\n", entry, i, value);
		if ((value & 0x03) == 0) {
			empty = 0;
			fwvlantsr1 = read32(FWVLANTSR1, 0);
			csdn       = read32(FWVLANTSR2, 0);
			fwvlantsr3 = read32(FWVLANTSR3, 0);

			printf("%x %x %x\n", fwvlantsr1, csdn, fwvlantsr3);
/*			learn_disable = (value >> 10) & 0x01;
			security = (value >> 8) & 0x01;
			srclockdpv = (fwvlantsr1 >> 0) & 0xFF;
			cpu_mirror = (fwvlantsr3 >> 21) & 0x01;
			eth_mirror = (fwvlantsr3 >> 20) & 0x01;
			ipv_enable = (fwvlantsr3 >> 19) & 0x01;
			ipv_value = (fwvlantsr3 >> 16) & 0xFF;
			dpv = (fwvlantsr3 >> 0) & 0xFF;

			seq_printf(m,"%4d ", entry);

			writeMirrorEtcFields(m, portName, portNumber, cpu_mirror, eth_mirror,
				srclockdpv, dpv, csdn, ipv_enable, ipv_value);

			//default values are not printed
			if (security)
				seq_printf(m, "   SecurityLevel=%d", security);
			if (learn_disable)
				seq_printf(m, "   HwLearnDisable=%d", learn_disable);

			seq_printf(m, "\n");
			*/
		}
	}
	if (empty)
		printf("*** empty ***\n");
}

//------------------------------------------------------------------------------
static void print_base()
{
	if (!first)
		printf("\n");
	first = 0;

	uint32_t v1, v2;


	v1 = read32(FWGC, 0);
	printf(LABEL": %s\n", "Switch VLAN mode",
		(v1==0)?"No":(v1==1)?"C-Tag":(v1==2)?"SC-TAG":"Reserved");

	v1 = read32(FWTTC0, 0);
	v2 = read32(FWTTC0, 0);
	printf(LABEL": C=%04x  S=%04x  R=%04x\n", "Configured Tags",
		v1&0xffff, v1>>16, v2);

	v1 = read32(FWMACHEC, 0);
	printf(LABEL": MaxCollision=%d  MaxUnsecure=%d\n", "MAC table config",
		v1&0xffff, v1>>16);
	v1 = read32(FWVLANTEC, 0);
	printf(LABEL": MaxUnsecure=%d\n", "VLAN table config",
		v1&0xffff);


	//Exceptional path

	//Learning by software

	//Mirroring

	//Wartermark

	//Port configurations

	printf("\n");
	print_strings("", portnames, NULL, RSWITCH_MAX_NUM_GWCA, NULL);
	printf("%s\n", headerLine);

	print_regs_H("VLAN serach enabled", FWPC00, 0x10,  -RSWITCH_MAX_NUM_GWCA, NULL);
	for (int i=0; i<RSWITCH_NUM_HW; i++) {
		v1 = read32(FWPC00 + 0x10*i, 0);
		sbuf[i][0] = (v1&0x00800000)?'s':' ';
		sbuf[i][1] = (v1&0x00100000)?'d':' ';
		sbuf[i][2] = (v1&0x08000000)?'M':' ';
		sbuf[i][3] = (v1&0x04000000)?'L':' ';
		sbuf[i][4] = 0;
	}
	print_strings("MAC learning", sbuf, NULL, RSWITCH_MAX_NUM_GWCA, "Learn, Migrate");
	for (int i=0; i<RSWITCH_NUM_HW; i++) {
		v1 = read32(FWPC10 + 0x10*i, 0);
		fillPortVector(sbuf[i], (v1 >> 16)^0xff, NULL);
	}
	print_strings("Valid L3 target", sbuf, NULL, RSWITCH_MAX_NUM_GWCA, NULL);
	for (int i=0; i<RSWITCH_NUM_HW; i++) {
		v1 = read32(FWPC20 + 0x10*i, 0);
		fillPortVector(sbuf[i], (v1 >> 16)^0xff, NULL);
	}
	print_strings("Valid L2 target", sbuf, NULL, RSWITCH_MAX_NUM_GWCA, NULL);
	for (int i=0; i<RSWITCH_NUM_HW; i++) {
		v1 = read32(FWPC10 + 0x10*i, 0);
		if ((v1&3)==0)
			strcpy(sbuf[i], "no");
		else {
			sbuf[i][0] = (v1&2)?'s':' ';
			sbuf[i][1] = (v1&1)?'d':' ';
			sbuf[i][2] = 0;
		}
	}
	print_strings("Direct Descr target", sbuf, NULL, RSWITCH_MAX_NUM_GWCA, "secured, default");
	print_regs_H("  FWPC0i", FWPC00, 0x10, -RSWITCH_MAX_NUM_GWCA, NULL);
	print_regs_H("  FWPC1i", FWPC10, 0x10, -RSWITCH_MAX_NUM_GWCA, NULL);
	print_regs_H("  FWPC2i", FWPC20, 0x10, -RSWITCH_MAX_NUM_GWCA, NULL);


	print_regs_H("Integrity ET", FWICETCi, 0x40,  -RSWITCH_MAX_NUM_GWCA, "0=accept");
	print_regs_H("Integrity IPv4", FWICIP4Ci, 0x40,  -RSWITCH_MAX_NUM_GWCA, "0=accept");
	print_regs_H("Integrity IPv6", FWICIP6Ci, 0x40,  -RSWITCH_MAX_NUM_GWCA, "0=accept");

	for (int i=0; i<RSWITCH_NUM_HW; i++) {
		v1 = read32(FWPBFCi + 0x10*i, 0);
		fillPortVector(sbuf[i], v1 & 0xff, NULL);
	}
	print_strings("Port based targets", sbuf, NULL, RSWITCH_MAX_NUM_GWCA, NULL);
	print_regs_H("  FWPBFCi", FWPBFCi, 0x10, -RSWITCH_MAX_NUM_GWCA, NULL);


}

//------------------------------------------------------------------------------
static void print_chains(int fd, int gwca)
{
	if (!first)
		printf("\n");
	first = 0;

	uint64_t batadr;
	uint32_t *bat;


	batadr = ((uint64_t)read32(GWDCBAC0, 0)<<32) + read32(GWDCBAC1, 0);
	printf(LABEL": %10lx  %02lx_%04lx_%04lx\n", "Base Address Table", batadr, batadr>>32, (batadr>>16)&0xffff, batadr&0xffff);

	bat = (uint32_t*) mmap(NULL, 0x1000, PROT_READ, MAP_SHARED, fd, batadr);
	printf("%p\n", bat);
	usleep(5000);
	for (int j=0; j<4; j++) {
		printf("%08x\n", bat[j]);
	}

}


//------------------------------------------------------------------------------
int main(int argc, char **argv)
{
	int fd;
	int page_size;
	long addr;
	uint32_t v;
	char *s;

	//check the alignment
	page_size = getpagesize();
	addr = RSW2_BASE_S4;
	if ((addr & ~(page_size-1)) != addr) {
		fprintf(stderr, "CPU page size of %08x does not fit to RSW2 base address %08lx\n", page_size, addr);
		return -1;
	}

	//parameters from command line
	if (argc <= 1)
		usage(argv[0]);
	for (int i=1; i< argc; i++) {
		if (!strcmp("rxtx", argv[i]))
			do_rxtx = 1;
		else if (!strcmp("rxtx-all", argv[i]))
			do_rxtx = 2;
		else if (!strcmp("eth-error", argv[i]))
			do_eth_error = 1;
		else if (!strcmp("eth-cnt", argv[i]))
			do_eth_cnt = 1;
		else if (!strcmp("eth-conf", argv[i]))
			do_eth_conf = 1;

		else if (!strcmp("base", argv[i]))
			do_base = 1;
		else if (!strcmp("l2", argv[i]))
			do_l2 = 1;
		else if (!strcmp("vlan", argv[i]))
			do_vlan = 1;
		else if (!strcmp("l3", argv[i]))
			do_l3 = 1;

		else if (!strcmp("all", argv[i]))
			do_all = 1;

		else if (!strcmp("chains", argv[i]))
			do_chains = 1;

		else if (!strcmp("rst-serdes", argv[i]))
			rst_serdes = 1;

		else if (!strcmp("-f", argv[i]))
			addr = RSW2_BASE_FPGA;

		else {
			fprintf(stderr, "Unknown infomration '%s' selected.\n", argv[i]);
			usage(argv[0]);
		}
	}


	//open the memory dump
	fd = open("/dev/mem", (O_RDWR | O_SYNC));
	if (fd < 0) {
		fprintf(stderr, "Error: Cannot open \"/dev/mem\" for reading\n");
		return -1;
	}

	//map the RSW2 SFR
	printf("Use RSW2 @%08lx + %x\n", addr, (RSW2_SIZE+page_size-1)/page_size*page_size);
	rsw2 = (uint32_t*) mmap(NULL, (RSW2_SIZE+page_size-1)/page_size*page_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, addr);
	if (addr == RSW2_BASE_S4) {
		printf("Use RSW2/SERDES @%08x + %x\n", RSW2_BASE_SERDES, RSW2_SIZE_SERDES);
		rsw2_serdes = (uint32_t*) mmap(NULL, RSW2_SIZE_SERDES, PROT_READ|PROT_WRITE, MAP_SHARED, fd, RSW2_BASE_SERDES);
	}
	else
		rsw2_serdes = NULL;

	//prepare data fields for reports
	memset(headerLine, 0, sizeof(headerLine));
	memset(headerLine, '-', S_LABEL + (S_PORT+1) * (RSWITCH_NUM_HW+1) +1);
	for (int i=0; i<ports; i++) {
		s = (char*) malloc(20);
		sprintf(s, "tsn%d", i);
		portnames[i] = s;
		sbuf[i] = (char*) malloc(100);
	}
	for (int i=ports; i<RSWITCH_NUM_HW; i++) {
		s = (char*) malloc(20);
		sprintf(s, "gwca%d", i-ports);
		portnames[i] = s;
		sbuf[i] = (char*) malloc(100);
	}

	//Some actions
	if (rst_serdes)
		action_rst_serdes();

	//And generate the reports
	if (do_rxtx || do_all)
		print_rxtx(do_rxtx>1);
	if (do_eth_error || do_all)
		print_eth_error(do_eth_error>1);
	if (do_eth_cnt || do_all)
		print_eth_cnt(do_eth_cnt>1);
	if (do_eth_conf || do_all)
		print_eth_conf(do_eth_conf>1);

	if (do_base || do_all)
		print_base();
	if (do_l2 || do_all)
		print_l2();
	if (do_vlan || do_all)
		print_vlan();

	if (do_chains)
		print_chains(fd, do_chains);

//~ static int do_l2 = 0;
//~ static int do_l3 = 0;
//~ static int do_vlan = 0;

	//All done
	close(fd);
	return 0;
}


#if 0

static void getPortNameString(char *buffer, int *number)
{
	int i;
	char s[10];

	if (board_config.eth_ports < *number)
		*number = board_config.eth_ports;

	for (i=0; i < *number; i++) {
		buffer[i] = '?';
		strncpy(s, ppndev[i]->name, 10);
		s[9] = 0;
		if (strcmp(s, "eth1") == 0)
			buffer[i] = 'E';
		else if (strcmp(s, "tsn0") == 0)
			buffer[i] = 'M';
		else {
			s[3] = 0;
			if (strcmp(s, "tsn") == 0)
				buffer[i] = ppndev[i]->name[3];  //the interface number
		}
	}
}


static void writeTargetFields(struct seq_file * m, 	const char *portName,
	const int portNumber, const u32 dpv, const s32 csdn)
{
	int c = 0;

	for (c = 0; c < portNumber; c++) {
		if (((dpv >> c) & 0x01) == 1)
			seq_printf(m, " %c", portName[c]);
		else
			seq_printf(m, "  ");
	}
	if (((dpv >> board_config.eth_ports) & 0x01) == 0x01) {
		seq_printf(m, " CPU");
		if (csdn >= 0)
			seq_printf(m, ":%-3d", csdn);
	}
	else {
		seq_printf(m, "    ");
		if (csdn >= 0)
			seq_printf(m, "    ");
	}
}


static void writeMirrorEtcFields(struct seq_file * m, 	const char *portName,
	const int portNumber, const u8 cpu_mirror, const u8 eth_mirror,
	const u32 srclockdpv, const s64 destlockdpv, const s64 destcsdn, const u8 ipv_enable, const u32 ipv_value)
{
	int c = 0;

	seq_printf(m," %-3s", eth_mirror ? "Eth" : "No");
	seq_printf(m," %-3s", cpu_mirror ? "CPU" : "No");

	seq_printf(m, " ");
	for (c = 0; c < portNumber; c++) {
		if (((srclockdpv >> c) & 0x01) == 1)
			seq_printf(m, " %c", portName[c]);
		else
			seq_printf(m, "  ");
	}
	if (((srclockdpv >> board_config.eth_ports) & 0x01) == 0x01)
		seq_printf(m, " CPU");
	else
		seq_printf(m, "    ");

	if (destlockdpv >= 0) {
		seq_printf(m, "  ");
		for (c = 0; c < portNumber; c++) {
			if (((destlockdpv >> c) & 0x01) == 1)
				seq_printf(m, " %c", portName[c]);
			else
				seq_printf(m, "  ");
		}
		if (((destlockdpv >> board_config.eth_ports) & 0x01) == 0x01) {
			seq_printf(m, " %s", "CPU");
			if (destcsdn >= 0)
				seq_printf(m, ":%-3lld", destcsdn);
		}
		else {
			seq_printf(m, "    ");
			if (destcsdn >= 0)
				seq_printf(m, "    ");
		}
	}

	if (ipv_enable)
		seq_printf(m, "  %d", ipv_value);
	else
		seq_printf(m, "  -");
}


/**
    @brief L2 Show Show Proc Function

    @param  seq_file *

    @param  void *

    @return int

*/

/**
    @brief VLAN Show Show Proc Function

    @param  seq_file *
    @param  void *

    @return int
*/


/**
    @brief L3 Show Show Proc Function

    @param  seq_file *
    @param  void *

    @return int
*/
static int rswitch2_fwd_l3_show(struct seq_file * m, void * v)
{
	char str1[30], str2[30];
	u32 srcipaddr[4];
	u32 destipaddr[4];
	int mode;
	u32 entry = 0;
	u32 value = 0;
	u32 fwlthtrr1 = 0;
	u32 routingnum = 0;
	u32 fwlthtrr2 = 0;
	u32 fwlthtrr3 = 0;
	u32 fwlthtrr4 = 0;
	u32 fwlthtrr5 = 0;
	u32 fwlthtrr8 = 0;
	u32 fwlthtrr10 = 0;
	u32 routingvalid = 0;
	u8 security = 0;
	u32 ctag = 0;
	u32 stag = 0;
	u32 destipport = 0;
	u32 hash = 0;
	u32 csdn = 0;
	u32 cpu_mirror = 0;
	u32 eth_mirror = 0;
	u32 ipv_enable = 0;
	u32 ipv_value = 0;
	u32 srclockdpv = 0;
	u32 dpv = 0;
	u8 i = 0;
	u32 filter_number = 0;

	char portName[10];
	int  portNumber = 10;
	getPortNameString(portName, &portNumber);

	seq_printf(m, "=========== L3-TABLE ==================================================\n");
	seq_printf(m, "Line Format    Target-Ports    Mirror   Src-Valid    IPV  Rule#\n");
	seq_printf(m, "=======================================================================\n");

	for (entry = 0; entry <= 1023; entry++) {
		iowrite32(entry, ioaddr + FWLTHTR);
		for (i = 0; i < RSWITCH2_PORT_CONFIG_TIMEOUT_MS; i++) {
			value = ioread32(ioaddr + FWLTHTRR0);
			if (((value >> 31) & 0x01) == 0x0) {
				if ((((value >> 2) & 0x01) == 0) && (((value >> 1) & 0x01) == 0x01) && (((value >> 0) & 0x01) == 0x0) ) {
					fwlthtrr1 = ioread32(ioaddr + FWLTHTRR1);
					security = (fwlthtrr1 >> 8);
					mode = (fwlthtrr1 >> 0) & 0x03;

					fwlthtrr2 = ioread32(ioaddr + FWLTHTRR2);
					stag = (fwlthtrr2 >> 16) & 0xFFFF;
					ctag = (fwlthtrr2 >> 0) & 0xFFFF;

					fwlthtrr3 = ioread32(ioaddr + FWLTHTRR3);
					destipport = (fwlthtrr3 >> 16) & 0xFFFF;
					hash = (fwlthtrr3 >> 0) & 0xFFFF;

					fwlthtrr4 = ioread32(ioaddr + FWLTHTRR4);
					srcipaddr[0] = ((fwlthtrr4 >> 24) & 0xFF);
					srcipaddr[1] = ((fwlthtrr4 >> 16) & 0xFF);
					srcipaddr[2] = ((fwlthtrr4 >> 8) & 0xFF);
					srcipaddr[3] = ((fwlthtrr4 >> 0) & 0xFF);

					fwlthtrr5 = ioread32(ioaddr + FWLTHTRR5);
					destipaddr[0] = ((fwlthtrr5 >> 24) & 0xFF);
					destipaddr[1] = ((fwlthtrr5 >> 16) & 0xFF);
					destipaddr[2] = ((fwlthtrr5 >> 8) & 0xFF);
					destipaddr[3] = ((fwlthtrr5 >> 0) & 0xFF);
					filter_number = fwlthtrr5;

					fwlthtrr8 = ioread32(ioaddr + FWLTHTRR8);
					srclockdpv = (fwlthtrr8 >> 16) & 0xFF;
					if ((fwlthtrr8 >> 15) & 0x01) {
						routingnum = fwlthtrr8 & 0xFF;
						routingvalid = 1;
					}
					else
						routingvalid = 0;

					csdn = ioread32(ioaddr + FWLTHTRR9);

					fwlthtrr10 = ioread32(ioaddr + FWLTHTRR10);
					cpu_mirror = (fwlthtrr10 >> 21) & 0x01;
					eth_mirror = (fwlthtrr10 >> 20) & 0x01;
					ipv_enable = (fwlthtrr10 >> 19) & 0x01;
					ipv_value = (fwlthtrr10 >> 15) & 0xFF;
					dpv = (fwlthtrr10 >> 0) & 0xFF;


					seq_printf(m,"%-4d", entry);

					if (mode == 0x01)
						seq_printf(m, " %-9s", "IPv4");
					else if (mode == 0x02)
						seq_printf(m, " %-9s", "IPv4+UDP");
					else if (mode == 0x03)
						seq_printf(m, " %-9s", "IPv4+TCP");
					else if (mode == 0x00)
						seq_printf(m, " %-9s", "Filter");

					writeTargetFields(m, portName, portNumber, dpv, csdn);
					writeMirrorEtcFields(m, portName, portNumber, cpu_mirror, eth_mirror,
						srclockdpv, -1, -1, ipv_enable, ipv_value);

					if (routingvalid)
						seq_printf(m, "    %-2d", routingnum);
					else
						seq_printf(m, "    - ");

					//default values are not printed
					if (security)
						seq_printf(m, "   SecurityLevel=%d", security);

					seq_printf(m, "\n");


				//the 2nd line
					if (mode == 0) {
						//seq_printf(m, "FilterNr=%d\n", filter_number);
					}
					else {
						seq_printf(m, "      \\-> ");
						sprintf(str1, "%d.%d.%d.%d", srcipaddr[0], srcipaddr[2], srcipaddr[2], srcipaddr[3]);
						sprintf(str2, "%d.%d.%d.%d:%d", destipaddr[0], destipaddr[1], destipaddr[2], destipaddr[3], destipport);
						seq_printf(m, "SA=%s  DA=%s  CTag=%04x  STag=%04x  Hash=%04x\n", str1, str2, ctag, stag, hash);
					}
				}
				break;
			}
			mdelay(1);
		}
	}
	return 0;
}


/**
    @brief L2 L3 Update Show Proc Function

    @param  seq_file *
    @param  void *

    @return int
*/
static int rswitch2_fwd_l2_l3_update_show(struct seq_file * m, void * v)
{
	u32 i;
	u32 entry;
	u32 value;
	u32 fwl23urrr1;
	u32 fwl23urrr2;
	u32 fwl23urrr3;

	u8 mac[6] ;
	char str1[30];
	int dpv;
	int rtag;
	int sdieul;
	int spcpul;
	int sidul;
	int cdieul;
	int cpcpul;
	int cidul;
	int msaul;
	int mdaul;
	int ttlul;

	char portName[10];
	int  portNumber = 10;
	getPortNameString(portName, &portNumber);

	seq_printf(m, "=========== Routing-Update-Table ==========================================\n");
	seq_printf(m, "Rule# Dst-Valid    TTL  DA-MAC            SA-MAC   CTAG       STAG    RTAG\n");
	seq_printf(m, "================================================== Id P D === Id P D ======\n");

	for (entry = 0; entry <= 255; entry++) {
		iowrite32(entry, ioaddr + FWL23URR);
		for (i = 0; i < RSWITCH2_PORT_CONFIG_TIMEOUT_MS; i++) {
			value = ioread32(ioaddr + FWL23URRR0);
			if (((value >> 31) & 0x01) == 0x0) {
				if ((((value >> 16) & 0x01) == 0) && (value & 0xFFFF)) {
					fwl23urrr1 = ioread32(ioaddr + FWL23URRR1);
					fwl23urrr2 = ioread32(ioaddr + FWL23URRR2);
					fwl23urrr3 = ioread32(ioaddr + FWL23URRR3);

					dpv    = value & 0xFF;
					rtag   = (fwl23urrr1 >> 25) & 0x03;
					sdieul = (fwl23urrr1 >> 24) & 0x01;
					spcpul = (fwl23urrr1 >> 23) & 0x01;
					sidul  = (fwl23urrr1 >> 22) & 0x01;
					cdieul = (fwl23urrr1 >> 21) & 0x01;
					cpcpul = (fwl23urrr1 >> 20) & 0x01;
					cidul  = (fwl23urrr1 >> 19) & 0x01;
					msaul  = (fwl23urrr1 >> 18) & 0x01;
					mdaul  = (fwl23urrr1 >> 17) & 0x01;
					ttlul  = (fwl23urrr1 >> 16) & 0x01;

					mac[0] = (fwl23urrr1 >> 8) & 0xFF;
					mac[1] = (fwl23urrr1 >> 0) & 0xFF;
					mac[2] = (fwl23urrr2 >> 24) & 0xFF;
					mac[3] = (fwl23urrr2 >> 16) & 0xFF;
					mac[4] = (fwl23urrr2 >> 8) & 0xFF;
					mac[5] = (fwl23urrr2 >> 0) & 0xFF;

					seq_printf(m, "%-4d ", entry);

					writeTargetFields(m, portName, portNumber, dpv, -1);

					seq_printf(m, "  %-3s", (ttlul)?"dec":"-");

					if (mdaul)
						sprintf(str1, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
					else
						strcpy(str1, "-");
					seq_printf(m, "  %-18s", str1);

					seq_printf(m, "  %-3s", (msaul)?"Yes":"-");

					if (cidul)
						seq_printf(m, "  %4d", (fwl23urrr3>>0) & 0xFFF);
					else
						seq_printf(m, "     -");
					if (cpcpul)
						seq_printf(m, " %d", (fwl23urrr3>>12) & 0x07);
					else
						seq_printf(m, " -");
					if (cdieul)
						seq_printf(m, " %d", (fwl23urrr3>>15) & 0x01);
					else
						seq_printf(m, " -");

					if (sidul)
						seq_printf(m, "   %4d", (fwl23urrr3>>16) & 0xFFF);
					else
						seq_printf(m, "      -");
					if (spcpul)
						seq_printf(m, " %d", (fwl23urrr3>>28) & 0x07);
					else
						seq_printf(m, " -");
					if (sdieul)
						seq_printf(m, " %d", (fwl23urrr3>>31) & 0x01);
					else
						seq_printf(m, " -");

					seq_printf(m, "   %d", rtag);

					seq_printf(m, "\n");
				}
				break;
			}
			mdelay(1);
		}
	}
	return 0;
}


/**
    @brief Errors Show Show Proc Function

    @param  seq_file *
    @param  void *

    @return int
    Tas and CBS error not covered tbd later when functionality implemented
*/
static int rswitch2_fwd_errors_show(struct seq_file * m, int longreport)
{
	u8 i = 0;
	char buffer[4];
	seq_printf(m, "           ");
	for (i = 0; i < board_config.eth_ports; i++) {
		getPortNameStringFull(buffer, &i);
		seq_printf(m, "%8s", buffer);
	}
	seq_printf(m, "%7s\n","CPU");
#if 1
	//seq_printf(m, "\n");
	for (i = 0; i <= board_config.eth_ports; i++) {
		errors.ddntfs.error[i] = (ioread32(ioaddr + FWEIS00 + (i *0x10)) >> 29) & 0x01;
		if((!errors.ddntfs.set_any) && (errors.ddntfs.error[i])) {
			errors.ddntfs.set_any = 1;
		}
	}
	for (i = 0; i <= board_config.eth_ports; i++) {
		errors.ddses.error[i] = (ioread32(ioaddr + FWEIS00 + (i *0x10)) >> 28) & 0x01;
		if((!errors.ddses.set_any) && (errors.ddses.error[i])) {
			errors.ddses.set_any = 1;
		}
	}
	for (i = 0; i <= board_config.eth_ports; i++) {
		errors.ddfes.error[i] = (ioread32(ioaddr + FWEIS00 + (i *0x10)) >> 27) & 0x01;
		if((!errors.ddfes.set_any) && (errors.ddfes.error[i])) {
			errors.ddfes.set_any = 1;
		}
	}
	for (i = 0; i <= board_config.eth_ports; i++) {
		errors.ddes.error[i] = (ioread32(ioaddr + FWEIS00 + (i *0x10)) >> 26) & 0x01;
		if((!errors.ddes.set_any) && (errors.ddes.error[i])){
			errors.ddes.set_any = 1;
		}
	}
	for (i = 0; i <= board_config.eth_ports; i++) {
		errors.wmiufs.error[i] = (ioread32(ioaddr + FWEIS00 + (i *0x10)) >> 25) & 0x01;
		if((!errors.wmiufs.set_any) && (errors.wmiufs.error[i])) {
			errors.wmiufs.set_any = 1;
		}
	}
	for (i = 0; i <= board_config.eth_ports; i++) {
		errors.wmisfs.error[i] = (ioread32(ioaddr + FWEIS00 + (i *0x10)) >> 24) & 0x01;
		if((!errors.wmisfs.set_any) && (errors.wmisfs.error[i])) {
			errors.wmisfs.set_any = 1;
		}
	}
	for (i = 0; i <= board_config.eth_ports; i++) {
		errors.wmffs.error[i] = (ioread32(ioaddr + FWEIS00 + (i *0x10)) >> 23) & 0x01;
		if((!errors.wmffs.set_any) && (errors.wmffs.error[i])){
			errors.wmffs.set_any = 1;
		}
	}
	for (i = 0; i <= board_config.eth_ports; i++) {
		errors.wmcfs.error[i] = (ioread32(ioaddr + FWEIS00 + (i *0x10)) >> 22) & 0x01;
		if((!errors.wmcfs.set_any) && (errors.wmcfs.error[i])){
			errors.wmcfs.set_any = 1;
		}
	}
	for (i = 0; i <= board_config.eth_ports; i++) {
		errors.smhmfs.error[i] = (ioread32(ioaddr + FWEIS00 + (i *0x10)) >> 19) & 0x01;
		if((!errors.smhmfs.set_any) && (errors.smhmfs.error[i])) {
			errors.smhmfs.set_any = 1;
		}
	}
	for (i = 0; i <= board_config.eth_ports; i++) {
		errors.smhlfs.error[i] = (ioread32(ioaddr + FWEIS00 + (i *0x10)) >> 18) & 0x01;
		if((!errors.smhlfs.set_any) && (errors.smhlfs.error[i])) {
			errors.smhlfs.set_any = 1;
		}
	}
	for (i = 0; i <= board_config.eth_ports; i++) {
		errors.pbntfs.error[i] = (ioread32(ioaddr + FWEIS00 + (i *0x10)) >> 17) & 0x01;
		if((!errors.pbntfs.set_any) && (errors.pbntfs.error[i])) {
			errors.pbntfs.set_any = 1;
		}
	}
	for (i = 0; i <= board_config.eth_ports; i++) {
		errors.ltwvufs.error[i] = (ioread32(ioaddr + FWEIS00 + (i *0x10)) >> 16) & 0x01;
		if((!errors.ltwvufs.set_any) && (errors.ltwvufs.error[i])) {
			errors.ltwvufs.set_any = 1;
		}
	}
	for (i = 0; i <= board_config.eth_ports; i++) {
		errors.ltwdufs.error[i] = (ioread32(ioaddr + FWEIS00 + (i *0x10)) >> 15) & 0x01;
		if((!errors.ltwdufs.set_any) && (errors.ltwdufs.error[i])) {
			errors.ltwdufs.set_any = 1;
		}
	}
	for (i = 0; i <= board_config.eth_ports; i++) {
		errors.ltwsufs.error[i] = (ioread32(ioaddr + FWEIS00 + (i *0x10)) >> 14) & 0x01;
		if((!errors.ltwsufs.set_any) && (errors.ltwsufs.error[i])) {
			errors.ltwsufs.set_any = 1;
		}
	}
	for (i = 0; i <= board_config.eth_ports; i++) {
		errors.ltwntfs.error[i] = (ioread32(ioaddr + FWEIS00 + (i *0x10)) >> 13) & 0x01;
		if((!errors.ltwntfs.set_any) && (errors.ltwntfs.error[i])) {
			errors.ltwntfs.set_any = 1;
		}
	}
	for (i = 0; i <= board_config.eth_ports; i++) {
		errors.ltwvspfs.error[i] = (ioread32(ioaddr + FWEIS00 + (i *0x10)) >> 12) & 0x01;
		if((!errors.ltwvspfs.set_any) && (errors.ltwvspfs.error[i])) {
			errors.ltwvspfs.set_any = 1;
		}
	}
	for (i = 0; i <= board_config.eth_ports; i++) {
		errors.ltwsspfs.error[i] = (ioread32(ioaddr + FWEIS00 + (i *0x10)) >> 11) & 0x01;
		if((!errors.ltwsspfs.set_any) && (errors.ltwsspfs.error[i])) {
			errors.ltwsspfs.set_any = 1;
		}
	}
	for (i = 0; i <= board_config.eth_ports; i++) {
		errors.ltwdspfs.error[i] = (ioread32(ioaddr + FWEIS00 + (i *0x10)) >> 10) & 0x01;
		if((!errors.ltwdspfs.set_any) && (errors.ltwdspfs.error[i])) {
			errors.ltwdspfs.set_any = 1;
		}
	}
	for (i = 0; i <= board_config.eth_ports; i++) {
		errors.lthufs.error[i] = (ioread32(ioaddr + FWEIS00 + (i *0x10)) >> 3) & 0x01;
		if((!errors.lthufs.set_any) && (errors.lthufs.error[i])) {
			errors.lthufs.set_any = 1;
		}
	}
	for (i = 0; i <= board_config.eth_ports; i++) {
		errors.lthntfs.error[i] = (ioread32(ioaddr + FWEIS00 + (i *0x10)) >> 2) & 0x01;
		if((!errors.lthntfs.set_any) && (errors.lthntfs.error[i])) {
			errors.lthntfs.set_any = 1;
		}
	}
	for (i = 0; i <= board_config.eth_ports; i++) {
		errors.lthfsfs.error[i] = (ioread32(ioaddr + FWEIS00 + (i *0x10)) >> 1) & 0x01;
		if((!errors.lthfsfs.set_any) && (errors.lthfsfs.error[i])) {
			errors.lthfsfs.set_any = 1;
		}
	}
	for (i = 0; i <= board_config.eth_ports; i++) {
		errors.lthspfs.error[i] = (ioread32(ioaddr + FWEIS00 + (i *0x10)) >> 0) & 0x01;
		if((!errors.lthspfs.set_any) && (errors.lthspfs.error[i])) {
			errors.lthspfs.set_any = 1;
		}
	}
	if(errors.ddntfs.set_any || longreport) {
		seq_printf(m, "%-8s","DDNTFS");
		for (i = 0; i <= board_config.eth_ports; i++) {
			seq_printf(m, "%8d", errors.ddntfs.error[i]);
		}
		seq_printf(m, "  Direct Descr, No Target\n");
	}
	if(errors.ddses.set_any || longreport) {
		seq_printf(m, "%-8s","DDSES");
		for (i = 0; i <= board_config.eth_ports; i++) {
			seq_printf(m, "%8d", errors.ddses.error[i]);
		}
		seq_printf(m, "  Direct Descr, Security\n");
	}
	if(errors.ddfes.set_any || longreport) {
		seq_printf(m, "%-8s","DDFES");
		for (i = 0; i <= board_config.eth_ports; i++) {
			seq_printf(m, "%8d", errors.ddfes.error[i]);
		}
		seq_printf(m, "  Direct Descr, Format\n");
	}
	if(errors.ddes.set_any || longreport) {
		seq_printf(m, "%-8s","DDES");
		for (i = 0; i <= board_config.eth_ports; i++) {
			seq_printf(m, "%8d", errors.ddes.error[i]);
		}
		seq_printf(m, "  Direct Descr, Error\n");
	}
	if(errors.wmiufs.set_any || longreport) {
		seq_printf(m, "%-8s","WMIUFS");
		for (i = 0; i <= board_config.eth_ports; i++) {
			seq_printf(m, "%8d", errors.wmiufs.error[i]);
		}
		seq_printf(m, "  WaterMark IPV Unsecure\n");
	}
	if(errors.wmisfs.set_any || longreport) {
		seq_printf(m, "%-8s","WMISFS");
		for (i = 0; i <= board_config.eth_ports; i++) {
			seq_printf(m, "%8d", errors.wmisfs.error[i]);
		}
		seq_printf(m, "  WaterMark IPV Secure\n");
	}
	if(errors.wmffs.set_any || longreport) {
		seq_printf(m, "%-8s","WMFFS");
		for (i = 0; i <= board_config.eth_ports; i++) {
			seq_printf(m, "%8d", errors.wmffs.error[i]);
		}
		seq_printf(m, "  WaterMark Flush\n");
	}
	if(errors.wmcfs.set_any || longreport) {
		seq_printf(m, "%-8s","WMCFS");
		for (i = 0; i <= board_config.eth_ports; i++) {
			seq_printf(m, "%8d", errors.wmcfs.error[i]);
		}
		seq_printf(m, "  WaterMark Critical\n");
	}
	if(errors.smhmfs.set_any || longreport) {
		seq_printf(m, "%-8s","SMHLFS");
		for (i = 0; i <= board_config.eth_ports; i++) {
			seq_printf(m, "%8d", errors.smhmfs.error[i]);
		}
		seq_printf(m, "  Source MAC Migration\n");
	}
	if(errors.smhlfs.set_any || longreport) {
		seq_printf(m, "%-8s","SMHLFS");
		for (i = 0; i <= board_config.eth_ports; i++) {
			seq_printf(m, "%8d", errors.smhlfs.error[i]);
		}
		seq_printf(m, "  Source MAC Learning\n");
	}
	if(errors.pbntfs.set_any || longreport) {
		seq_printf(m, "%-8s","PBNTFS");
		for (i = 0; i <= board_config.eth_ports; i++) {
			seq_printf(m, "%8d", errors.pbntfs.error[i]);
		}
		seq_printf(m, "  Port Based No Target\n");
	}
	if(errors.ltwvufs.set_any || longreport) {
		seq_printf(m, "%-8s","LTWVUFS");
		for (i = 0; i <= board_config.eth_ports; i++) {
			seq_printf(m, "%8d", errors.ltwvufs.error[i]);
		}
		seq_printf(m, "  L2 VLAN Unknown\n");
	}
	if(errors.ltwdufs.set_any || longreport) {
		seq_printf(m, "%-8s","LTWDUFS");
		for (i = 0; i <= board_config.eth_ports; i++) {
			seq_printf(m, "%8d", errors.ltwdufs.error[i]);
		}
		seq_printf(m, "  L2 Dest Unknown\n");
	}
	if(errors.ltwsufs.set_any || longreport) {
		seq_printf(m, "%-8s","LTWSUFS");
		for (i = 0; i <= board_config.eth_ports; i++) {
			seq_printf(m, "%8d", errors.ltwsufs.error[i]);
		}
		seq_printf(m, "  L2 Source Unknown\n");
	}
	if(errors.ltwntfs.set_any || longreport) {
		seq_printf(m, "%-8s","LTWNTFS");
		for (i = 0; i <= board_config.eth_ports; i++) {
			seq_printf(m, "%8d", errors.ltwntfs.error[i]);
		}
		seq_printf(m, "  L2 No Target\n");
	}
	if(errors.ltwvspfs.set_any || longreport) {
		seq_printf(m, "%-8s","LTWVSPFS");
		for (i = 0; i <= board_config.eth_ports; i++) {
			seq_printf(m, "%8d", errors.ltwvspfs.error[i]);
		}
		seq_printf(m, "  L2 VLAN Source Port\n");
	}
	if(errors.ltwsspfs.set_any || longreport) {
		seq_printf(m, "%-8s","LTWSSPFS");
		for (i = 0; i <= board_config.eth_ports; i++) {
			seq_printf(m, "%8d", errors.ltwsspfs.error[i]);
		}
		seq_printf(m, "  L2 Source Source Port\n");
	}
	if(errors.ltwdspfs.set_any || longreport) {
		seq_printf(m, "%-8s","LTWDSPFS");
		for (i = 0; i <= board_config.eth_ports; i++) {
			seq_printf(m, "%8d", errors.ltwdspfs.error[i]);
		}
		seq_printf(m, "  L2 Destination Source Port\n");
	}
	if(errors.lthufs.set_any || longreport) {
		seq_printf(m, "%-8s","LTHUFS");
		for (i = 0; i <= board_config.eth_ports; i++) {
			seq_printf(m, "%8d", errors.lthufs.error[i]);
		}

		seq_printf(m, "  L3 Unknown\n");
	}
	if(errors.lthntfs.set_any || longreport) {
		seq_printf(m, "%-8s","LTHNTFS");
		for (i = 0; i <= board_config.eth_ports; i++) {
			seq_printf(m, "%8d", errors.lthntfs.error[i]);
		}
		seq_printf(m, "  L3 No Target\n");
	}
	if(errors.lthfsfs.set_any || longreport) {
		seq_printf(m, "%-8s","LTHFSFS");
		for (i = 0; i <= board_config.eth_ports; i++) {
			seq_printf(m, "%8d", errors.lthfsfs.error[i]);
		}
		seq_printf(m, "  L3 Format Security\n");
	}
	if(errors.lthspfs.set_any || longreport) {
		seq_printf(m, "%-8s","LTHSPFS");
		for (i = 0; i <= board_config.eth_ports; i++) {
			seq_printf(m, "%8d", errors.lthspfs.error[i]);
		}
		seq_printf(m, "  L3 Source Port\n");
	}
	errors.global_error.arees = (ioread32(ioaddr + FWEIS1) >> 16) & 0x01;
	errors.global_error.l23uses = (ioread32(ioaddr + FWEIS1) >> 9) & 0x01;
	errors.global_error.l23uees = (ioread32(ioaddr + FWEIS1) >> 8) & 0x01;
	errors.global_error.vlantses = (ioread32(ioaddr + FWEIS1) >> 7) & 0x01;
	errors.global_error.vlantees = (ioread32(ioaddr + FWEIS1) >> 6) & 0x01;
	errors.global_error.mactses = (ioread32(ioaddr + FWEIS1) >> 5) & 0x01;
	errors.global_error.mactees = (ioread32(ioaddr + FWEIS1) >> 4) & 0x01;
	errors.global_error.lthtses = (ioread32(ioaddr + FWEIS1) >> 1) & 0x01;
	errors.global_error.lthtees = (ioread32(ioaddr + FWEIS1) >> 0) & 0x01;
	errors.global_error.macadas = (ioread32(ioaddr + FWMIS0) >> 17) & 0x01;
	errors.global_error.vlantfs = (ioread32(ioaddr + FWMIS0) >> 3) & 0x01;
	errors.global_error.mactfs = (ioread32(ioaddr + FWMIS0) >> 2) & 0x01;
	errors.global_error.lthtfs = (ioread32(ioaddr + FWMIS0) >> 0) & 0x01;
	if(errors.global_error.arees || errors.global_error.l23uses ||
	errors.global_error.l23uees || errors.global_error.vlantses ||
	errors.global_error.vlantees || errors.global_error.mactses ||
	errors.global_error.mactees ||	errors.global_error.lthtses ||
	errors.global_error.lthtees ||	errors.global_error.macadas ||
	errors.global_error.vlantfs ||	errors.global_error.mactfs || errors.global_error.lthtfs) {
		errors.global_error.set_any = 1;
	}
	if(errors.global_error.set_any  || longreport) {
		seq_printf(m, "\n");
		seq_printf(m, "=======================Port-Independent-Errors===========================\n");
		if(errors.global_error.arees || longreport) {
			seq_printf(m, "AREES    %8d  ATS RAM ECC\n",errors.global_error.arees );
		}
		if(errors.global_error.l23uses || longreport) {
			seq_printf(m, "L23USES  %8d  L2/L3 Update Security\n",errors.global_error.l23uses );
		}
		if(errors.global_error.l23uees || longreport) {
			seq_printf(m, "L23UEES  %8d  Layer2/Layer3 Update ECC\n",errors.global_error.l23uees);
		}
		if(errors.global_error.vlantses || longreport) {
			seq_printf(m, "VLANTSES %8d  VLAN Table Security\n",errors.global_error.vlantses);
		}
		if(errors.global_error.vlantees || longreport) {
			seq_printf(m, "VLANTEES %8d  VLAN Table ECC\n",errors.global_error.vlantees );
		}
		if(errors.global_error.mactses || longreport) {
			seq_printf(m, "MACTSES  %8d  MAC Table Security\n",errors.global_error.mactses);
		}
		if(errors.global_error.mactees || longreport) {
			seq_printf(m, "MACTEES  %8d  MAC Table ECC\n",errors.global_error.mactees);
		}
		if(errors.global_error.lthtses || longreport) {
			seq_printf(m, "LTHTSES  %8d  L3 Table Security\n",errors.global_error.lthtses);
		}
		if(errors.global_error.lthtees || longreport) {
			seq_printf(m, "LTHTEES  %8d  L3 Table ECC\n",errors.global_error.lthtees);
		}
		if(errors.global_error.macadas || longreport) {
			seq_printf(m, "MACADAS  %8d  MAC Address Deleted Aging\n",errors.global_error.macadas);
		}
		if(errors.global_error.vlantfs || longreport) {
			seq_printf(m, "VLANTFS  %8d  VLAN Table Full\n",errors.global_error.vlantfs );
		}
		if(errors.global_error.mactfs || longreport) {
			seq_printf(m, "MACTFS   %8d  MAC Table Full\n",errors.global_error.mactfs);
		}
		if(errors.global_error.lthtfs || longreport) {
			seq_printf(m, "LTHTFS   %8d  L3 Table Full\n",errors.global_error.lthtfs);
		}
	}
#endif
	return 0;
}


/**
    @brief Wrapper function to show Error in /proc either in long format

    @param  seq_file *
    @param  void *

    @return int
*/
static int rswitch2_fwd_errors_all_show(struct seq_file * m, void * v)
{
	return rswitch2_fwd_errors_show(m, 1);
}


/**
    @brief Wrapper function to show error in /proc either in short format

    @param  seq_file *
    @param  void *

    @return int
*/
static int rswitch2_fwd_errors_short_show(struct seq_file * m, void * v)
{
	return rswitch2_fwd_errors_show(m, 0);
}

/**
    @brief rswitch2 Fwd counters clear function

    @param  void

    @return int
*/
static int rswitch2_fwd_counter_clear_func(void)
{
	memset(&counter,0, sizeof(struct rswitch2_fwd_counter) );
	return 0;
}

/**
    @brief rswitch2 Fwd Errors clear function

    @param  void

    @return int
*/
static int rswitch2_fwd_error_clear_func(void)
{
	memset(&errors,0, sizeof(struct rswitch2_fwd_error) );
	return 0;
}




/**
    @brief Write FWD Counter Clear Function for Proc

    @param  struct file *
    @param  const char *
    @param  size_t
    @param  loff_t *

    @return ssize_t
*/
static ssize_t rswitch2_fwd_counters_clear(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	u32 ret = 0;
	u64 fwd_counter_clear = 0;
	ret = kstrtoull_from_user(buff, len, 10, &fwd_counter_clear);
	if (ret) {
		/* Negative error code. */
		pr_info("ko = %d\n", ret);
		return ret;
	}
	else {
		rswitch2_fwd_counter_clear_func();
		return len;
	}
}


/**
    @brief Counters Show Show Proc Function

    @param  seq_file *
    @param  void *

    @return int
*/
static int rswitch2_fwd_counters_show(struct seq_file * m, void * v)
{
	u8 i = 0;
	char buffer[4];
	seq_printf(m, "           ");
	for (i = 0; i < board_config.eth_ports; i++) {
		getPortNameStringFull(buffer, &i);
		seq_printf(m, "%8s", buffer);
	}
	seq_printf(m, "%7s\n","CPU");
	/*seq_printf(m, "%-8s","CTFDN");
	for (i = 0; i < board_config.eth_ports; i++) {
		counter.ctfdn[i] += ioread32(ioaddr + FWCTFDCN0 + (i *20));
		seq_printf(m, "%8d", (counter.ctfdn[i]));
	}
	seq_printf(m, "  Cut-Through Descr\n");*/
	seq_printf(m,"%-8s", "DDFDN");
	for (i = 0; i < board_config.eth_ports; i++) {
		seq_printf(m, "%8s","-");
	}
	counter.ddfdn[i] += ioread32(ioaddr + FWDDFDCN0 + (board_config.eth_ports *20));
	seq_printf(m, "%8d", counter.ddfdn[i]);

	seq_printf(m, "  Direct Descr\n");
	seq_printf(m, "%-8s","LTHFDN");
	for (i = 0; i <= board_config.eth_ports; i++) {
		counter.lthfdn[i] += ioread32(ioaddr + FWLTHFDCN0 + (i *20));
		seq_printf(m, "%8d", (counter.lthfdn[i])) ;

	}
	seq_printf(m, "  L3 Descr\n");
	/*seq_printf(m, "%-8s","IPFDN");
	for (i = 0; i <= board_config.eth_ports; i++) {
		counter.ipfdn[i] += ioread32(ioaddr + FWIPFDCN0 + (i *20));
		seq_printf(m, "%8d", (counter.ipfdn[i])) ;
	}
	seq_printf(m, "  IP Descr\n");*/
	seq_printf(m, "%-8s","LTWFDN");
	for (i = 0; i <= board_config.eth_ports; i++) {
		counter.ltwfdn[i] += ioread32(ioaddr + FWLTWFDCN0 + (i *20));
		seq_printf(m, "%8d", (counter.ltwfdn[i])) ;

	}
	seq_printf(m, "  L2 Descr\n");
	seq_printf(m, "%-8s","PBFDN");
	for (i = 0; i <= board_config.eth_ports; i++) {
		counter.pbfdn[i] += ioread32(ioaddr + FWPBFDCN0 + (i *20));
		seq_printf(m, "%8d", (counter.pbfdn[i])) ;

	}
	seq_printf(m, "  Port-Based Descr\n");
	seq_printf(m, "%-8s","MHLN");
	for (i = 0; i <= board_config.eth_ports; i++) {
		counter.mhln[i] += ioread32(ioaddr + FWMHLCN0 + (i *20));
		seq_printf(m, "%8d", (counter.mhln[i])) ;
	}
	seq_printf(m, "  MAC Learned\n");
	/*seq_printf(m, "%-8s","IHLN");
	for (i = 0; i <= board_config.eth_ports; i++) {
		counter.ihln[i] += ioread32(ioaddr + FWIHLCN0 + (i *20));
		seq_printf(m, "%8d", (counter.ihln[i])) ;
	}
	seq_printf(m, "\n");*/
	seq_printf(m, "%-8s","ICRDN");
	for (i = 0; i <= board_config.eth_ports; i++) {
		counter.icrdn[i] += ioread32(ioaddr + FWICRDCN0 + (i *20));
		seq_printf(m, "%8d", (counter.icrdn[i])) ;
	}
	seq_printf(m, "  Reject Integrity Check\n");
	seq_printf(m, "%-8s","WMRDN");
	for (i = 0; i <= board_config.eth_ports; i++) {
		counter.wmrdn[i] += ioread32(ioaddr + FWWMRDCN0 + (i *20));
		seq_printf(m, "%8d", (counter.wmrdn[i])) ;
	}
	seq_printf(m, "  Reject WaterMark\n");
	/*seq_printf(m, "%-8s","CTRDN");
	for (i = 0; i < board_config.eth_ports; i++) {
		counter.ctrdn[i] += ioread32(ioaddr + FWCTRDCN0 + (i *20));
		seq_printf(m, "%8d", (counter.ctrdn[i])) ;
	}
	seq_printf(m, "  Reject Cut-through\n");*/
	seq_printf(m, "%-8s","DDRDN");
	for (i = 0; i < board_config.eth_ports; i++) {
		seq_printf(m, "%8s","-");
	}
	counter.ddrdn[i] += ioread32(ioaddr + FWDDRDCN0 + (board_config.eth_ports *20));
	seq_printf(m, "%8d", counter.ddrdn[i]);
	seq_printf(m, "  Reject Direct Descr\n");
	seq_printf(m, "%-8s","LTHRDN");
	for (i = 0; i <= board_config.eth_ports; i++) {
		counter.lthrdn[i] += ioread32(ioaddr + FWLTHRDCN0 + (i *20));
		seq_printf(m, "%8d", (counter.lthrdn[i])) ;
	}
	seq_printf(m, "  Reject L3\n");
	/*seq_printf(m, "%-8s","IPRDN");
	for (i = 0; i <= board_config.eth_ports; i++) {
		counter.iprdn[i] += ioread32(ioaddr + FWIPRDCN0 + (i *20));
		seq_printf(m, "%8d", (counter.iprdn[i]));
	}
	seq_printf(m, "  Reject IP\n");*/
	seq_printf(m, "%-8s","LTWRDN");
	for (i = 0; i <= board_config.eth_ports; i++) {
		counter.ltwrdn[i] += ioread32(ioaddr + FWLTWRDCN0 + (i *20));
		seq_printf(m, "%8d", (counter.ltwrdn[i])) ;
	}
	seq_printf(m, "  Reject L2\n");
	seq_printf(m, "%-8s","PBRDN");
	for (i = 0; i <= board_config.eth_ports; i++) {
		counter.pbrdn[i] += ioread32(ioaddr +  FWPBRDCN0 + (i *20));
		seq_printf(m, "%8d", (counter.pbrdn[i])) ;
	}
	seq_printf(m, "  Reject Port-Based\n");
	return 0;
}


/**
    @brief L2 L3 Update Show Proc Function

    @param  seq_file *
    @param  void *

    @return int
*/
static int rswitch2_fwd_buffer_show(struct seq_file * m, void * v)
{
	int i, j;
	u32 r, rl, rh;
	u32 total;
	const u32 ports = 5;
	int sum=0;

	r = ioread32(ioaddr + CABPPCM);
	rh = (r >> 16);
	rl = (r & 0xffff);
	total = rh;
	sum = total - rl;
	seq_printf(m, "Total existing      %04x\n", total);
	seq_printf(m, "Remaining current   %04x  %d%%\n", rl, rl*100/total);
	r = ioread32(ioaddr + CABPLCM);
	seq_printf(m, "Remaining minimum   %04x  %d%%\n\n", r, r*100/total);

	seq_printf(m, "               tsn6   tsn7   tsn5   tsn4  tsngw  --  reject\n");
	seq_printf(m, "Used current");
	for (i = 0; i < ports; i++) {
		r = ioread32(ioaddr + CABPCPM+(i*4));
		sum -= r;
		seq_printf(m, "   %04x", r);
	}
	r = ioread32(ioaddr + CARDNM);
	seq_printf(m, "  --  %04x\n", r);
	seq_printf(m, "Used maximum");
	for (i = 0; i < ports; i++) {
		r = ioread32(ioaddr + CABPMCPM+(i*4));
		seq_printf(m, "   %04x", r);
	}
	r = ioread32(ioaddr + CARDMNM);
	seq_printf(m, "  --  %04x\n", r);

	r = ioread32(ioaddr + CARDCN);
	seq_printf(m, "Reject total   %08x  %d\n", r, r);

	if (sum != 0)
		seq_printf(m, "Lost buffers %d (small differences normal in case of ongoing traffic)\n", sum);

	seq_printf(m, "\n                        tsn6   tsn7   tsn5   tsn4  tsngw\n");
	for (j = 0; j < 8; j++) {
		seq_printf(m, "Pending frames in Q#%d", j);
		for (i = 0; i < ports; i++) {
			r = ioread32(ioaddr + EATDQM0+(i*4) + 0xA000 + j*0x2000);
			seq_printf(m, " %6d", r);
		}
		seq_printf(m, "\n");
	}

	return 0;
}









static int read_mcast_entry_value(int index)
{
	int timeout, value;

	iowrite32(index, gwca_addr + GWMSTSS);
	timeout = 0;
	do {
		value = ioread32(gwca_addr + GWMSTSR);
	} while (value & 0x80000000 && ++timeout < 10);
	return value;
}

static int rswitch2_gwca_mcast_show(struct seq_file *m, void *v)
{
	int c, i, next;
	int value;

	seq_puts(m, "Chain     Additional targets\n");
	seq_puts(m, "=================================================\n");

    //we have 128 entries and show only the used ones
	for (c = 0; c < RSWITCH2_MAX_MCAST_CHAINS; c++) {
		value = read_mcast_entry_value(c);
		i = (value >> 8) & 0x7;  //how many links to follow
		if (i) {
			next = value & (RSWITCH2_MAX_MCAST_CHAINS - 1);
			seq_printf(m, "%3d   #%d ", c, i);
			while (i) {
				seq_printf(m, " -> %d", next);
				value = read_mcast_entry_value(next);
				next = value & (RSWITCH2_MAX_MCAST_CHAINS - 1);
				i--;
			}
			seq_puts(m, "\n");
		}
	}
	return 0;
}
#endif
