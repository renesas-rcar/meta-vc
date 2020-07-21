/**
    @brief  Snow Stream Test Tool

    This SNOW Stream Test Tool is able to generates given number of 1722 frames with
    given Stream ID, PCP, frame rate, on a specified interface. It is also able to
    receive 1722 frames and is able to store it in a file(optional) and gives the number
    of 1722 frames received  every 10 seconds.

    @file   SNOTT.c

    @todo
    - Get RACE_GW_HOST_CONTROL_DEVICE_NAME from race_gwhost.h

    @author Asad Kamal

    @copyright
    Copyright (c) 2014, Renesas
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.

    3. Neither the name of the Intel Corporation nor the names of its
       contributors may be used to endorse or promote products derived from
       this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.
*/


#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <sched.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <signal.h>
#include <getopt.h>
#include <unistd.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <pthread.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
//#include <linux/drivers/char/renesas/race_gwhost.h>
//#include <linux/drivers/char/renesas/race_icusa.h>

//#include "./race_gwhost.h"
//#include "./race_icusa.h"
//#include </home/taraka/CEES2/linux/drivers/char/renesas/race_icusa.h>




#ifndef NOT
 #define  NOT !
#endif

#ifndef AND
 #define AND &&
#endif

#ifndef OR
 #define OR ||
#endif

#define FALSE       0
#define TRUE        1
#define bool        int


#define VERSION_STR         "1.6.0"


#define GWCPU_INTERFACE     "gwcpu"
#define ICUSA_INTERFACE     "icusa"
#define GWHOST_DEV_NAME0     "/dev/"
#define GWHOST_DEV_NAME1     "/dev/"
#define GWHOST_DEV_NAME2     "/dev/"
#define GWHOST_DEV_NAME3     "/dev/"

static const char *GWHOST_DEV_NAME[4] = {GWHOST_DEV_NAME0, GWHOST_DEV_NAME1, GWHOST_DEV_NAME2, GWHOST_DEV_NAME3};
#define ICUSA_DEV_NAME      "/dev/"
/* Macros for 1722 TP HEADER */
#define DEST_MAC0       0x0
#define DEST_MAC1       0x0
#define DEST_MAC2       x0
#define DEST_MAC3       0x0
#define DEST_MAC4       0x00
#define DEST_MAC5       0x0
#define ETHER_TYPE      0x0800
#define PCP             7
#define MY_DEST_MAC0    0x91
#define MY_DEST_MAC1    0xE0
#define MY_DEST_MAC2    0xF0
#define MY_DEST_MAC3    0x00
#define MY_DEST_MAC4    0x01
#define MY_DEST_MAC5    0x10
#define MY_SRC_MAC0     0x60
#define MY_SRC_MAC1     0x12
#define MY_SRC_MAC2     0xB2
#define MY_SRC_MAC3     0x50
#define MY_SRC_MAC4     0x75
#define MY_SRC_MAC5     0x16
#define TP_ID0          0x81
#define TP_ID1          0x00
#define CFE             0
#define VLAN0           ((((PCP << 1) | CFE ) << 4) | VID0 )
#define VLAN1           VID1
#define BUF_SIZ         2048
#define ETH_TYPE0       0x22
#define ETH_TYPE1       0xF0
#define STREAM_ID0      0x00
#define STREAM_ID1      0x12
#define STREAM_ID2      0x34
#define STREAM_ID3      0x56
#define STREAM_ID4      0x78
#define STREAM_ID5      0x01
#define STREAM_ID6      0x00
#define STREAM_ID7      0x01
#define PAYLOAD0        0x82
#define PAYLOAD1        0x80
/* NTCF DATA LENGTH 16 bytes = 0x10*/
//#define PAYLOAD2        0x10
/* NTCF DATA LENGTH 72 bytes = 0x48*/
#define PAYLOAD2        0x48

#define PAYLOAD3        0x00
#define PAYLOAD4        STREAM_ID0
#define PAYLOAD5        STREAM_ID1
#define PAYLOAD6        STREAM_ID2
#define PAYLOAD7        STREAM_ID3
#define PAYLOAD8        STREAM_ID4
#define PAYLOAD9        STREAM_ID5
#define PAYLOAD10       STREAM_ID6
#define PAYLOAD11       STREAM_ID7
#define PAYLOAD12       0x04
/* ACF MESSAGE LENGTH =
   8 BYTES + 8 bytes header Info =
      16 BYTES =  4 Quadlets = 0x04 */
//#define PAYLOAD13       0x04
/* ACF MESSAGE LENGTH =
   64 BYTES + 8 bytes header Info =
      72 BYTES =  18 Quadlets = 0x12 */

#define PAYLOAD13       0x012
/* NON FD Mode*/
//#define PAYLOAD14       0x00
/* FD Mode Without BRS*/
#define PAYLOAD14       0x02
/*  FD Mode With BRS*/
//#define PAYLOAD14       0x06
#define PAYLOAD15       0x00
#define PAYLOAD16       0x00
#define PAYLOAD17       0x00
#define PAYLOAD18       0x07
#define PAYLOAD19       0x80
#define PAYLOAD20       0x00
#define PAYLOAD21       0x02
#define PAYLOAD22       0x00
#define PAYLOAD23       0x04
#define PAYLOAD24       0xAA
#define PAYLOAD25       0xBB
#define PAYLOAD26       0xCC
#define PAYLOAD27       0xDD
#define PAYLOAD28       0xEE
#define PAYLOAD29       0xFF
#define PAYLOAD30       0xAA
#define PAYLOAD31       0xBB
#define PAYLOAD32       0x00
#define ASCII_0_VALU    48
#define ASCII_9_VALU    57
#define ASCII_A_VALU    65
#define ASCII_F_VALU    70
#define STREAM_ID_SIZE  23
#define MAC_ID_SIZE     17
#define ETH_TYPE_SIZE   4
//#define MAX_PAYLOAD     1475
#define MAX_PAYLOAD     1958
#define ICUSA_GWHOST_MSG_LENGTH 64


#define RTAG_ID0      0xF1
#define RTAG_ID1      0xC1

/* Function Prototype*/
void stop_timer();



/* Global Variables Definition */
FILE              * fp = NULL;
int                 dp = -1;
int                 ds = -1;
pthread_t           thread_id = 0;
pthread_t           thread_id1 = 0;

pthread_attr_t tattr;
pthread_mutex_t lock;

struct sched_param param;

static unsigned int queue = 0;
bool rate_flag = FALSE;
bool no_verify_flag = FALSE;
bool fail_security_flag = FALSE;
static char         ifName[IFNAMSIZ];
static int          sockfd = -1;


static unsigned int first_rx_sec;
static unsigned int payload_count;

volatile sig_atomic_t
                    stop;
static char         sendbuf[BUF_SIZ];
static char         sendbuf_preempt[BUF_SIZ];
static char         sendbuf_icusa[BUF_SIZ];


char                test_iface[5];

bool                count_flag = FALSE;
bool                icusa_write_flag;
unsigned int        frame_rate = 0;
unsigned int        seq_num = 0;
unsigned int        seq_num_preempt = 0;
unsigned int        seq_index = 0;
unsigned int        seq_index_preempt = 0;
unsigned int        tx_count = 0;
timer_t             gTimerid;
unsigned int        tx_len = 0;
unsigned int        tx_len_preempt = 0;
static struct sockaddr_ll
                    socket_address;
unsigned int        second_count = 0;
unsigned char       file_name[1000] = {'\0'};
bool                file_flag = FALSE;
unsigned int        frame_num = 0;
unsigned int        frame_sent = 0;
bool                end_after_send = 0;
static unsigned int pay_load_start;
static const char * Version_String =
    "SNOTT " VERSION_STR "\n" "Copyright (c) 2015, Renesas Electronics\n"
    "Built on " __DATE__ " at " __TIME__ "";
static struct ifreq if_ip;    /* get ip addr */

int rtag_control = -1;  //-1=no, >1 skip frames
unsigned short *rtag_seq_number = NULL;
unsigned char VID0 = 0, VID1 = 5;

uint8_t buf[BUF_SIZ];
//GwHost_RxD rxd_buf;

int payload_array[MAX_PAYLOAD] =
{
    PAYLOAD20,
    PAYLOAD21,
    PAYLOAD22,
    PAYLOAD23,
    PAYLOAD24,
    PAYLOAD25,
    PAYLOAD26,
    PAYLOAD27,
    PAYLOAD28,
};


/* Signal Handler for Stop */
void inthand(int signum)
{
    stop = 1;
}


/* Hex String to Integer Convertor*/
unsigned int HexStringToUInt(char const* hexstring)
{
    unsigned int result = 0;
    char const *c = hexstring;
    char thisC;

    while (*c != '\0')
    {
        unsigned int add;

        thisC = *c;
        thisC = toupper(thisC);
        result <<= 4;

        if ( thisC >= ASCII_0_VALU && thisC <= ASCII_9_VALU )
        {
            add = thisC - ASCII_0_VALU;
        }
        else if ( thisC >= ASCII_A_VALU && thisC <= ASCII_F_VALU)
        {
            add = thisC - ASCII_A_VALU + 10;
        }
        else
        {
            fprintf(stderr, "ERROR: Unrecognised hex character \"%c\"\n", thisC);
            exit(1);
        }
        result += add;
        ++c;
    }

    return result;
}

static int applyRTagSeq(void) //returns 1 if frame send shall skipped
{
    if (rtag_control!=1) {
        if (rtag_seq_number)
            (*rtag_seq_number) = htons(ntohs(*rtag_seq_number)+1);
        if (rtag_control > 1 && (*rtag_seq_number+1)%rtag_control == 0)
            return 1;
    }
    return 0;
}


void timer_function(int sig)
{
    int delay_ms=0;
    int i  = 0;
    int mysockfd;
    char myifName[IFNAMSIZ] = {'\0'};

    useconds_t delay_per_frame_us = 0;
    /* Send Packets based on count number */
    delay_ms = (1000 / 2);
    delay_per_frame_us = (frame_rate) ? ((delay_ms * 1000) / frame_rate) : 1;
    strcpy(myifName,ifName);
    mysockfd = sockfd;

    if ( (rate_flag) AND (strcasecmp(ifName, GWCPU_INTERFACE) != 0) AND (strcasecmp(ifName, ICUSA_INTERFACE) != 0) )
    {
        if (count_flag)
        {
            if (tx_count > 0)
            {
                if (tx_count <= frame_rate)
                {
                    for (i = 0; i < tx_count; i++)
                    {
                        /* Attach Sequence Number */
                        sendbuf[seq_index] = seq_num++;
						if (applyRTagSeq())
							;
                        else if (sendto(mysockfd, sendbuf, tx_len, 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0)
                        {
                            fprintf(stderr, "ERROR: Could Not Send over interface '%s' - %d(%s) \n", myifName, errno, strerror(errno));
                            exit(1);
                        }
                        frame_sent++;

                        usleep(delay_per_frame_us);
                    }
                    tx_count  = 0;
                }
                else
                {
                    for (i = 0; i < frame_rate; i++)
                    {
                        frame_sent++;
                        /* Attach Sequence Number */
                        sendbuf[seq_index] = seq_num++;
						if (applyRTagSeq())
							;
                        else if (sendto(mysockfd, sendbuf, tx_len, 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0)
                        {
                            fprintf(stderr, "ERROR: Could Not Send over interface '%s' - %d(%s) \n",myifName, errno, strerror(errno));
                            exit(1);
                        }

                        usleep(delay_per_frame_us);
                    }
                    tx_count = tx_count - frame_rate;
                }
            }
        }
        else if (NOT stop)
        {
            for (i = 0; i < frame_rate; i++)
            {
                /* Attach Sequence Number */
                sendbuf[seq_index] = seq_num++;
                /* Send packet */
				if (applyRTagSeq())
					;
				else if (sendto(mysockfd, sendbuf, tx_len, 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0)
                {
                    fprintf(stderr, "ERROR: could Not Send over interface '%s' - %d(%s) \n",myifName, errno, strerror(errno));
                    exit(1);
                }
                usleep(delay_per_frame_us);
            }
        }
    }

    //printf("frame_rate=%d  rate_flag=%d  count_flag=%d  tx_count=%d\n", frame_rate, rate_flag, count_flag, tx_count);
    second_count++;
}


/* 1 second timer init function*/
int proc_timer_init(void)
{
    struct itimerspec value;
    value.it_value.tv_sec = 1;
    value.it_value.tv_nsec = 0;
    value.it_interval.tv_sec = 1;
    value.it_interval.tv_nsec = 0;
    timer_create(CLOCK_REALTIME, NULL, &gTimerid);
    timer_settime(gTimerid, 0, &value, NULL);

    return 0;
}


/* Timer Stop Function */
void stop_timer()
{
    struct itimerspec value;
    value.it_value.tv_sec = 0;
    value.it_value.tv_nsec = 0;
    value.it_interval.tv_sec = 0;
    value.it_interval.tv_nsec = 0;
    timer_settime(gTimerid, 0, &value, NULL);
}


void show_console_help()
{
     printf("\n"
                    "  h|H    Show this message\n"
                    "  s|S    Show Statistics\n"
                    "  x|X    Exit\n");
}


void console_thread()
{
    int unused __attribute__((unused));
    char ch;

    while (TRUE)
    {
        static bool ShownHelp = TRUE;

        if (ShownHelp)
        {
           show_console_help();
           ShownHelp = FALSE;
           printf("\nSNOTT > ");
        }

        unused = scanf("%c", &ch);

        if ((ch == 'h') || (ch == 'H'))
        {
            show_console_help();
        }
        else if ((ch == 'x') || (ch == 'X'))
        {
            stop = 1;
            break;
        }
        else if ((ch == 's') || (ch == 'S'))
        {

            printf("Frames received  %d \n \rSeconds Passed  %d \n",frame_num, second_count);

        }
        else if (ch == '\n')
        {
            printf("\nSNOTT > ");
        }
        else
        {
            printf("Invalid Command \n");
            show_console_help();
        }
    }
}


/* Receive thread for Receiving Packets */
int rcv_thread(void *arg)
{
    char    sender[INET6_ADDRSTRLEN];
    int     ret;
    int     i;
    int     j;
    int     k;
    ssize_t numbytes;
    struct sockaddr_storage their_addr;
    struct iphdr *iph = (struct iphdr *) (buf + sizeof(struct ether_header));
    struct udphdr *udph = (struct udphdr *) (buf + sizeof(struct iphdr) + sizeof(struct ether_header));

    if (file_flag)
    {
        fp = fopen(file_name, "w+");
        if (fp == NULL)
        {
            fprintf(stderr, "ERROR: Could not open '%s' for writing - %d (%s)\n", file_name, errno, strerror(errno));
            exit(1);
        }
    }

    while (TRUE)
    {
        if ( (strcasecmp(ifName, GWCPU_INTERFACE) != 0) AND (strcasecmp(ifName, ICUSA_INTERFACE) != 0))
        {
            /* It's a blocking call */
            numbytes = recvfrom(sockfd, buf, BUF_SIZ, 0, NULL, NULL);
            ((struct sockaddr_in *)&their_addr)->sin_addr.s_addr = iph->saddr;
            inet_ntop(AF_INET, &((struct sockaddr_in*)&their_addr)->sin_addr, sender, sizeof sender);

            /* Look up my device IP addr if possible */
            strncpy(if_ip.ifr_name, ifName, IFNAMSIZ-1);
            if (ioctl(sockfd, SIOCGIFADDR, &if_ip) >= 0)
            {
                /* ignore if I sent it */
                if (strcmp(sender, inet_ntoa(((struct sockaddr_in *)&if_ip.ifr_addr)->sin_addr)) == 0)
                {
                    continue;
                }
            }

            /* UDP payload length */
            ret = ntohs(udph->len) - sizeof(struct udphdr);

            /* Print packet to file if its 1722 Packet */
            if (no_verify_flag)
            {
                if (file_flag)
                {
                    fprintf(fp, "Frame Num:%d  ",frame_num );
                    for (i = 0; i < numbytes; i++)
                    {
                        fprintf(fp, "%02x", buf[i]);
                    }
                    fprintf(fp, "\n");
                }
                frame_num++;
            }
            /* Print packet to file if its 1722 Packet */
            else if ((buf[12] == ETH_TYPE0) && (buf[13] == ETH_TYPE1))
            {
                if (file_flag)
                {
                    fprintf(fp, "Frame Num:%d  ",frame_num );
                    for (i = 0; i < numbytes; i++)
                    {
                        fprintf(fp, "%02x", buf[i]);
                    }
                    fprintf(fp, "\n");
                }
                frame_num++;
                if (frame_num == 1)
                {
                    first_rx_sec = second_count;
                }
            }
        }
        else if (strcasecmp(ifName, GWCPU_INTERFACE) == 0)
        {
            /* Its a blocking call */
            //numbytes = read(dp, &rxd_buf, 1);
            if (numbytes > 0)
            {
                //printf("Read happened \n");
                if (file_flag)
                {
                    fprintf(fp, "Frame Num:%d  ",frame_num );


                    //for (i = 0; i < (rxd_buf.BufferLength); i++)
                    //{
                        //fprintf(fp, "%02X", rxd_buf.Buffer[i]);
                        /* Just to add delay for file writing */
                    //    printf("");

                    //}


                    fprintf(fp, "\n");
                }
                frame_num++;
            }
       }
        else if (strcasecmp(ifName, ICUSA_INTERFACE) == 0)
        {
            if (count_flag == 1)
            {
                for (i = 0; i<tx_count; i++)
                {
                    if (icusa_write_flag)
                    {
                        icusa_write_flag = FALSE;
                        numbytes = read(ds,buf,1);

                        k = 0;
                        for (j = pay_load_start + 48; j < pay_load_start + ICUSA_GWHOST_MSG_LENGTH; j++)
                        {
                            sendbuf[j] =  buf[k];
                            k++;
                        }
                        write(dp,sendbuf,ICUSA_GWHOST_MSG_LENGTH + pay_load_start);
                        /* Its a blocking call */
                        //numbytes = read(dp,&rxd_buf,1);
                        if (numbytes > 0)
                        {
                            k = 0;
                            for(j = pay_load_start; j < ICUSA_GWHOST_MSG_LENGTH + pay_load_start; j++)
                            {
                                //sendbuf_icusa[k] = rxd_buf.Buffer[j];
                                k++;
                            }
                            /* Below statement needs to be removed once GW host driver fixes */
                            //sendbuf_icusa[k -2] = 0xE2; // For PLAIN KEY
                            //sendbuf_icusa[k -2] = 0xA9; // For CMD KEY
                            if (fail_security_flag)
                            {
                                sendbuf_icusa[0] = 0xFF;
                            }
                            write(ds,sendbuf_icusa,ICUSA_GWHOST_MSG_LENGTH);
                        }
                        frame_num++;
                        if( (i+1) != tx_count)
                        {
                            k = 0;
                            for(j = pay_load_start; j < payload_count + pay_load_start; j++)
                            {
                                sendbuf_icusa[k] = sendbuf[j];
                                k++;
                            }
                            write(ds,sendbuf_icusa,k);
                            icusa_write_flag = TRUE;
                        }

                    }
                }
                pthread_mutex_unlock(&lock);
            }
            else
            {
                while (NOT stop)
                {
                    k = 0;
                    for(j = pay_load_start; j < payload_count + pay_load_start; j++)
                    {
                        sendbuf_icusa[k] = sendbuf[j];
                        k++;
                    }
                    write(ds,sendbuf_icusa,k);
                    icusa_write_flag = TRUE;
                    if (icusa_write_flag)
                    {
                        icusa_write_flag = FALSE;


                        numbytes = read(ds,buf,1);

                        k = 0;

                        for(j = pay_load_start+48; j < pay_load_start+ICUSA_GWHOST_MSG_LENGTH; j++)
                        {

                            sendbuf[j] =  buf[k];
                            k++;
                        }
                        write(dp,sendbuf,ICUSA_GWHOST_MSG_LENGTH + pay_load_start);

                        /* Its a blocking call */
                        //numbytes = read(dp,&rxd_buf,1);
                        if(numbytes > 0)
                        {
                            k = 0;
                            for(j = pay_load_start; j < ICUSA_GWHOST_MSG_LENGTH + pay_load_start; j++)
                            {
                                //sendbuf_icusa[k] = rxd_buf.Buffer[j];
                                k++;
                            }
                            /* Below statement needs to be removed once GW host driver fixes */
                            //sendbuf_icusa[k -2] = 0xE2;// For PLAIN KEY
                            //sendbuf_icusa[k -2] = 0xA9; // For CMD KEY
                            if (fail_security_flag)
                            {
                                sendbuf_icusa[0] = 0xFF;
                            }
                            write(ds,sendbuf_icusa,ICUSA_GWHOST_MSG_LENGTH);
                        }
                        frame_num++;

                    }
                }
            }

        }
    }

    return ret;
}


/* Help usage */
void ShowUsage(char ** argv)
{
    printf("\n"
            "SNOTT\n\n"
            "usage: %s {-V} {-h} {-i dev} {-r fps} {-n count} {-l size} {-s streamID} {-t {c|s}} {-d {a|0|5|1}} {-p PCP} {-w file} {-r skip}\n", argv[0]);
    printf("options:\n"
            " -h  show this message\n"
            " -i  Interface name, Mandatory\n"
            " -n  For Tx. Number of frames to be sent (Default infinity)\n"
            " -r  For Tx. Frame rate in frames per second Ethernet only. Default network dependent)\n"
            " -l  For Tx. Stream payload to be added to 42 octet header (default is 10)\n"
            " -s  For Tx. Stream ID in format xx:xx:xx:xx:xx:xx:xx:xx (Default %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X)\n"
            " -t  For Tx. c for CAN 1us delay per 50 frames. s for ethernet 1us delay per 500 frames\n"
            " -p  For Tx. VLAN PCP (0 to 7. Default is %u)\n"
            " -v  For Tx. VLAN ID (0 to 4096. Default is %u)\n"
            " -d  For Data all 'a' type a, all '0' type 0, all '5' type 5, all '1s' type 1, (default 0)\n"
            " -w  For Rx. File for received frames if not used frame will not be stored\n"
            " -z  For Rx. Incoming frame will not be verified\n"
            " -f  For ICUSA Module Testing Corrupting Received Data to trigger FAIL condition\n"
            " -e  For Tx. eth type in format xxxx  (Default %02X%02X)\n"
            " -m  For Tx. DEST MAC in format xx:xx:xx:xx:xx:xx (Default %02X:%02X:%02X:%02X:%02X:%02X)\n"
			" -S  For Tx. SRC MAC in format xx:xx:xx:xx:xx:xx (Default %02X:%02X:%02X:%02X:%02X:%02X)\n"
            " -q  For GW-Host RX and Tx queue to be used\n"
            " -j  For testing premption with express frame over queue 0\n"
    		" -R  For .CB testing. Add R-Tag. skip=0 means no, skip=10 means every 10th frame skipped, default 0\n"
    		" -1  For run once. Tool ends after all frames transmitted\n"
            "Signals:\n"
            " -TERM Terminate daemon\n"
            "Notes:\n"
            " If -n is 0, no frames will be transmitted.\n"
            " The reception thread is always started.\n"
            " If -t is not specified then no delay between frames transmitted.\n"
            " The -t option should be used when transferring from GW-HOST as transmission speed is limited due to FPGA restrictions\n"
            " For CAN interface c(1 µs delay for 50 frames sent) option is used and for stress s(1 µs delay for 500 frames sent) option is used.\n"
            " A SNOTT console is created and you can enter the following commands\n"
            "    h|H    Show this message\n"
            "    s|S    Show Statistics\n"
            "    x|X    Exit\n"
            "\n" "%s" "\n",
            STREAM_ID0, STREAM_ID1, STREAM_ID2, STREAM_ID3, STREAM_ID4, STREAM_ID5, STREAM_ID6, STREAM_ID7,
            PCP, VID0*256+VID1,
            ETH_TYPE0, ETH_TYPE1,
            MY_DEST_MAC0, MY_DEST_MAC1, MY_DEST_MAC2, MY_DEST_MAC3, MY_DEST_MAC4, MY_DEST_MAC5,
			MY_SRC_MAC0, MY_SRC_MAC1, MY_SRC_MAC2, MY_SRC_MAC3, MY_SRC_MAC4, MY_SRC_MAC5,
            Version_String);
}


int main(int argc, char *argv[])
{
    struct ifreq if_idx;
    int     i = 0;
    //int flip = 0;
    //int VLAN_index = 0;
    int     j = 0;
    int     k = 0;
    int pcp = 0;
    struct ifreq ifopts;    /* set promiscuous mode */
    bool pcp_flag = FALSE;
    char *strid[8];
    char *macid[6];
    char *ethtype[2];
    int istrid[8] = {0};
    int iethtype[2] = {0};
    int option = 0;
    bool strm_id_flag = FALSE;
    bool mac_id_flag = FALSE;
	bool src_mac_id_flag = FALSE;
    char *str_id = NULL;
    char *mac_id = NULL;
	char *src_mac_id = NULL;
    int  imacid[6] = {0};
    int err;
    int sockopt;
    bool payload_flag = FALSE;
    int n = 0;
    struct ifreq if_mac;
    char data_type[10] = {'0'};
    bool if_flag = FALSE;
    bool dtype_flag = FALSE;
    bool preempt_flag = FALSE;
    bool ethtype_flag = FALSE;
    struct ether_header *eh = (struct ether_header *) sendbuf;
    static unsigned int delay_count = 0;
    char *eth_type = NULL;
    /* Install Signal Handler */
    signal(SIGINT, inthand);
    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }

    while ((option = getopt(argc, argv,"hi:n:r:l:s:S:p:d:w:t:q:e:m:VjzfR:v:1")) != -1)
    {
        switch (option)
        {
            case 'h':
                ShowUsage(argv);
                exit(0);
            case 'i':
                if (optarg[0] == '-')
                {
                    fprintf(stderr, "ERROR: Interface Name is required\n");
                    ShowUsage(argv);
                    exit(1);
                }
                printf("Interface Name: %s \n", optarg);
                if (((strlen(optarg) <= 0) || (strlen(optarg) >= IFNAMSIZ)))
                {
                    fprintf(stderr, "ERROR: Interface Name should be between 1 to %u characters \n", IFNAMSIZ);
                    ShowUsage(argv);
                    exit(1);
                }
                strcpy(ifName, optarg);
                if_flag = TRUE;
                break;
            case 'n':
                if (optarg[0] == '-')
                {
                    fprintf(stderr, "ERROR: Numner of frames to send is required\n");
                    ShowUsage(argv);
                    exit(1);
                }
                tx_count = atoi(optarg);
                printf("Tx Frame count: %d \n", tx_count);
                count_flag = TRUE;
                break;
            case 'r':
                if (optarg[0] == '-')
                {
                    fprintf(stderr, "ERROR: Transmit Frame Rate is required\n");
                    ShowUsage(argv);
                    exit(1);
                }
                frame_rate = atoi(optarg);
                printf("Tx Frame rate: %d Frames Per Second\n", frame_rate);
                rate_flag = TRUE;
                break;
            case 'l':
                if (optarg[0] == '-')
                {
                    fprintf(stderr, "ERROR: Transmit Payload size is required\n");
                    ShowUsage(argv);
                    exit(1);
                }
                payload_count = atoi(optarg);
                if (payload_count > MAX_PAYLOAD)
                {
                    fprintf(stderr, "ERROR, Maximum Ethernet Type 2 VLAN tagged Frame cannot have payload data more than %u bytes", MAX_PAYLOAD);
                    ShowUsage(argv);
                    exit(1);
                }
                printf("Payload size is %d \n", payload_count);
                payload_flag = TRUE;
                break;
            case 's':
                if (optarg[0] == '-')
                {
                    fprintf(stderr, "ERROR: Transmit Stream ID is required\n");
                    ShowUsage(argv);
                    exit(1);
                }
                printf("Tx Stream ID: %s \n", optarg);
                if ((strlen(optarg) != STREAM_ID_SIZE))
                {
                    fprintf(stderr, "ERROR: Invalid Stream ID format \n");
                    ShowUsage(argv);
                    exit(1);
                }
                strm_id_flag = TRUE;
                str_id = strdup(optarg);
                break;
            case 'R':
            	rtag_control = strtol(optarg, NULL, 0);
            	printf("R-Tag control is %d\n", rtag_control);
            	break;
            case 'v':
            	VID0 = strtol(optarg, NULL, 0) / 256;
            	VID1 = strtol(optarg, NULL, 0) & 255;
            	break;
            case 'm':
                if (optarg[0] == '-')
                {
                    fprintf(stderr, "ERROR: Transmit Dest MAC is required\n");
                    ShowUsage(argv);
                    exit(1);
                }
                printf("Dest MAC is: %s \n", optarg);
                if ((strlen(optarg) != MAC_ID_SIZE))
                {
                    fprintf(stderr, "ERROR: Invalid MAC ID format \n");
                    ShowUsage(argv);
                    exit(1);
                }
                mac_id_flag = TRUE;
                mac_id = strdup(optarg);
                break;
			case 'S':
                if (optarg[0] == '-')
                {
                    fprintf(stderr, "ERROR: Transmit Src MAC is required\n");
                    ShowUsage(argv);
                    exit(1);
                }
                printf("Src MAC is: %s \n", optarg);
                if ((strlen(optarg) != MAC_ID_SIZE))
                {
                    fprintf(stderr, "ERROR: Invalid MAC ID format \n");
                    ShowUsage(argv);
                    exit(1);
                }
                src_mac_id_flag = TRUE;
                src_mac_id = strdup(optarg);
                break;
            case 'p':
                if (optarg[0] == '-')
                {
                    fprintf(stderr, "ERROR: Transmit VLAN PCP number is required\n");
                    ShowUsage(argv);
                    exit(1);
                }
                pcp = atoi(optarg);
                printf("Tx PCP: %u \n", pcp);
                pcp_flag = TRUE;
                break;
            case 'd':
                if (optarg[0] == '-')
                {
                    fprintf(stderr, "ERROR: Transmit data type is required");
                    ShowUsage(argv);
                    exit(1);
                }
                dtype_flag = TRUE;
                strcpy(data_type, optarg);
                printf("Data type is %c \n", data_type[0]);
                break;
            case 'w':
                if (optarg[0] == '-')
                {
                    fprintf(stderr, "ERROR: Reception Filename is required\n");
                    ShowUsage(argv);
                    exit(1);
                }
                printf("Rx frames saved to File: %s \n",optarg);
                file_flag = TRUE;
                strcpy(file_name, optarg);
                break;
            case 't':
                if (optarg[0] == '-')
                {
                    fprintf(stderr, "ERROR: Test Interface Name is required\n");
                    ShowUsage(argv);
                    exit(1);
                }
                strcpy(test_iface, optarg);
                break;
            case 'q':
                if (optarg[0] == '-')
                {
                    fprintf(stderr, "ERROR: Queue Number is required\n");
                    ShowUsage(argv);
                    exit(1);
                }
                queue = atoi(optarg);
                break;
            case 'V':
                printf("%s\n", Version_String);
                exit(0);
            case 'z':
                no_verify_flag = TRUE;
                break;
            case 'j':
                preempt_flag = TRUE;
                break;
            case 'f':
                fail_security_flag = TRUE;
                break;
            case 'e':
                if (optarg[0] == '-')
                {
                    fprintf(stderr, "ERROR: Eth type is required\n");
                    ShowUsage(argv);
                    exit(1);
                }
                if ((strlen(optarg) != ETH_TYPE_SIZE))
                {
                    fprintf(stderr, "ERROR: Invalid ETH TYPE format \n");
                    ShowUsage(argv);
                    exit(1);
                }


                ethtype_flag = TRUE;
                eth_type = strdup(optarg);
                break;

            case '1':
            	end_after_send = 1;
            	break;
            default:
                ShowUsage(argv);
                exit(1);
        }

    }

    if (NOT if_flag)
    {
        ShowUsage(argv);
        exit(1);
    }

//Try to change to root permission if not started as root
    if (getuid() != 0) {
    	if (setuid(0)) {
        	fprintf(stderr, "Error: Can't set user id to 0.\nsudo chown root /usr/bin/SNOTT\nsudo chmod +s /usr/bin/SNOTT\n");
        	return -EACCES; /* Permission denied */
    	}
    }



    if (strcasecmp(ifName, GWCPU_INTERFACE) == 0)
    {

       if ((dp = open(GWHOST_DEV_NAME[queue], O_RDWR)) < 0)
       {
           perror("Open GWCPU Interface");
           exit(1);
       }
    }

    else if (strcasecmp(ifName, ICUSA_INTERFACE) == 0)
    {
       if ((ds = open(ICUSA_DEV_NAME, O_RDWR)) < 0)
       {
           perror("Open ICUSA Interface");
           exit(1);
       }
       if ((dp = open(GWHOST_DEV_NAME[queue], O_RDWR)) < 0)
       {
           perror("Open GWCPU Interface");
           exit(1);
       }
    }
    else
    {
        /* Open RAW socket to send on and receive from */
        if ((sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL)/*IPPROTO_RAW*/)) == -1)
        {
            perror("Open RAW socket");
            exit(1);
        }
    }
    if((strcasecmp(ifName,ICUSA_INTERFACE)) !=  0)
    {

        /* initialized with default attributes */
        err = pthread_attr_init (&tattr);
        /* safe to get existing scheduling param */
        err = pthread_attr_getschedparam (&tattr, &param);

        /* set the priority; others are unchanged */
        param.sched_priority = sched_get_priority_max(SCHED_RR);


        /* setting the new scheduling param */
        err = pthread_attr_setschedparam (&tattr, &param);


        err = pthread_create(&thread_id,&tattr, (void * (*)(void *))&rcv_thread, NULL);

        if (err != 0)
        {
            fprintf(stderr, "ERROR: can't create Reception Thread - %u(%s)\n", err, strerror(err));
            exit(1);
        }
    }
    if ( (strcasecmp(ifName, GWCPU_INTERFACE) != 0) AND (strcasecmp(ifName, ICUSA_INTERFACE) != 0) )
    {
        /* Get the index of the interface to send on */
        memset(&if_idx, 0, sizeof(struct ifreq));
        strncpy(if_idx.ifr_name, ifName, IFNAMSIZ-1);
        if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0)
        {
            fprintf(stderr, "ERROR: Failed to do SIOCGIFINDEX on RAW Socket");
            exit(1);
        }
        memset(&if_ip, 0, sizeof(struct ifreq));

        /* Get the MAC address of the interface to send on */
        memset(&if_mac, 0, sizeof(struct ifreq));
        strncpy(if_mac.ifr_name, ifName, IFNAMSIZ-1);
        if (ioctl(sockfd, SIOCGIFHWADDR, &if_mac) < 0)
        {
            fprintf(stderr, "ERROR: Failed to do SIOCGIFHWADDR on RAW Socket");
            exit(1);
        }

        /* Set interface to promiscuous mode  */
        strncpy(ifopts.ifr_name, ifName, IFNAMSIZ-1);
        ioctl(sockfd, SIOCGIFFLAGS, &ifopts);
        ifopts.ifr_flags |= IFF_PROMISC;
        ioctl(sockfd, SIOCSIFFLAGS, &ifopts);

        /* Allow the socket to be reused - in case connection is closed prematurely */
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof sockopt) == -1)
        {
            fprintf(stderr, "ERROR: Failed to set SO_REUSEADDR on RAW Socket");
            exit(1);
        }

        /* Bind to device */
        if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, ifName, IFNAMSIZ-1) == -1)
        {
            fprintf(stderr, "ERROR: Failed to set SO_BINDTODEVICE on RAW Socket");
            exit(1);
        }
    }

    /* Construct the Ethernet header */
    memset(sendbuf, 0, BUF_SIZ);
    if(preempt_flag)
    {
        memset(sendbuf_preempt, 0, BUF_SIZ);

    }
	if(src_mac_id_flag)
    {
        for (j = 0; j < 6; j++)
        {
            macid[j] = (char *)malloc( 3 * sizeof(char));
            strncpy(macid[j], src_mac_id, 2);
            macid[j][3] = '\0';
            src_mac_id = src_mac_id + 1;
            src_mac_id = src_mac_id + 1;
            src_mac_id = src_mac_id + 1;
            imacid[j] = HexStringToUInt(macid[j]);
            eh->ether_shost[j] = imacid[j];
        }

    }
	else
	{
        eh->ether_shost[0] = MY_SRC_MAC0;
        eh->ether_shost[1] = MY_SRC_MAC1;
        eh->ether_shost[2] = MY_SRC_MAC2;
        eh->ether_shost[3] = MY_SRC_MAC3;
        eh->ether_shost[4] = MY_SRC_MAC4;
        eh->ether_shost[5] = MY_SRC_MAC5;
	}
    if(mac_id_flag)
    {
        for (j = 0; j < 6; j++)
        {
            macid[j] = (char *)malloc( 3 * sizeof(char));
            strncpy(macid[j], mac_id, 2);
            macid[j][3] = '\0';
            mac_id = mac_id + 1;
            mac_id = mac_id + 1;
            mac_id = mac_id + 1;
            imacid[j] = HexStringToUInt(macid[j]);
            eh->ether_dhost[j] = imacid[j];
        }

    }
    else
    {
        eh->ether_dhost[0] = MY_DEST_MAC0;
        eh->ether_dhost[1] = MY_DEST_MAC1;
        eh->ether_dhost[2] = MY_DEST_MAC2;
        eh->ether_dhost[3] = MY_DEST_MAC3;
        eh->ether_dhost[4] = MY_DEST_MAC4;
        eh->ether_dhost[5] = MY_DEST_MAC5;
    }
    if(preempt_flag)
    {
        sendbuf_preempt[0] = MY_SRC_MAC0;
        sendbuf_preempt[1] = MY_SRC_MAC1;
        sendbuf_preempt[2] = MY_SRC_MAC2;
        sendbuf_preempt[3] = MY_SRC_MAC3;
        sendbuf_preempt[4] = MY_SRC_MAC4;
        sendbuf_preempt[5] = MY_SRC_MAC5;
        sendbuf_preempt[6] = MY_DEST_MAC0;
        sendbuf_preempt[7] = MY_DEST_MAC1;
        sendbuf_preempt[8] = MY_DEST_MAC2;
        sendbuf_preempt[9] = MY_DEST_MAC3;
        sendbuf_preempt[10] = MY_DEST_MAC4;
        sendbuf_preempt[11] = MY_DEST_MAC5;

    }

    /* Ethertype field */
    eh->ether_type = htons(0x22F0);
    tx_len += (sizeof(struct ether_header) - sizeof(eh->ether_type));
    tx_len_preempt += (sizeof(struct ether_header) - sizeof(eh->ether_type));
    /* Packet data */
    sendbuf[tx_len++] = TP_ID0;
    sendbuf[tx_len++] = TP_ID1;
    if(preempt_flag)
    {
        sendbuf_preempt[tx_len_preempt++] = TP_ID0;
        sendbuf_preempt[tx_len_preempt++] = TP_ID1;
        sendbuf_preempt[tx_len_preempt++] = VLAN0;
        sendbuf_preempt[tx_len_preempt++] = VLAN1;
        sendbuf_preempt[tx_len_preempt++] = ETH_TYPE0;
        sendbuf_preempt[tx_len_preempt++] = ETH_TYPE1;
        sendbuf_preempt[tx_len_preempt++] = PAYLOAD0;
        sendbuf_preempt[tx_len_preempt++] = PAYLOAD1;
        sendbuf_preempt[tx_len_preempt++] = PAYLOAD2;
        seq_index_preempt = tx_len_preempt;
        tx_len_preempt++;
        sendbuf_preempt[tx_len_preempt++] = PAYLOAD4;
        sendbuf_preempt[tx_len_preempt++] = PAYLOAD5;
        sendbuf_preempt[tx_len_preempt++] = PAYLOAD6;
        sendbuf_preempt[tx_len_preempt++] = PAYLOAD7;
        sendbuf_preempt[tx_len_preempt++] = PAYLOAD8;
        sendbuf_preempt[tx_len_preempt++] = PAYLOAD9;
        sendbuf_preempt[tx_len_preempt++] = PAYLOAD10;
        sendbuf_preempt[tx_len_preempt++] = PAYLOAD11;
        sendbuf_preempt[tx_len_preempt++] = PAYLOAD12;
        sendbuf_preempt[tx_len_preempt++] = PAYLOAD13;
        sendbuf_preempt[tx_len_preempt++] = PAYLOAD14;
        sendbuf_preempt[tx_len_preempt++] = PAYLOAD15;
        sendbuf_preempt[tx_len_preempt++] = PAYLOAD16;
        sendbuf_preempt[tx_len_preempt++] = PAYLOAD17;
        sendbuf_preempt[tx_len_preempt++] = PAYLOAD18;
        sendbuf_preempt[tx_len_preempt++] = PAYLOAD19;
        sendbuf_preempt[tx_len_preempt++] = PAYLOAD20;
        sendbuf_preempt[tx_len_preempt++] = PAYLOAD21;
        sendbuf_preempt[tx_len_preempt++] = PAYLOAD22;
        sendbuf_preempt[tx_len_preempt++] = PAYLOAD23;
        sendbuf_preempt[tx_len_preempt++] = PAYLOAD24;
        sendbuf_preempt[tx_len_preempt++] = PAYLOAD25;
        sendbuf_preempt[tx_len_preempt++] = PAYLOAD26;
        sendbuf_preempt[tx_len_preempt++] = PAYLOAD27;

    }
    //VLAN_index = tx_len;
    if (pcp_flag)
    {
        sendbuf[tx_len++] = ((((pcp << 1) | CFE ) << 4) | VID0 );
    }
    else
    {
        sendbuf[tx_len++] = VLAN0;
    }
    //VLAN_index = tx_len;
    sendbuf[tx_len++] = VLAN1;

//Add R-Tag
    if (rtag_control != -1) {
    	sendbuf[tx_len++] = RTAG_ID0;
    	sendbuf[tx_len++] = RTAG_ID1;
    	sendbuf[tx_len++] = 0;	//reserved
    	sendbuf[tx_len++] = 0;
    	rtag_seq_number = (unsigned short*)&sendbuf[tx_len];
        if (rtag_control==1) {
            sendbuf[tx_len++] = 0x12;	//magic number of stuck of transmitter
            sendbuf[tx_len++] = 0x34;
        }
        else {
            sendbuf[tx_len++] = 0xff;	//sequence number to -1 as it is incremented first
            sendbuf[tx_len++] = 0xff;
        }
    }
	if(ethtype_flag)
    {
        for (j = 0; j < 2; j++)
        {
            ethtype[j] = (char *)malloc( 3 * sizeof(char));
            strncpy(ethtype[j], eth_type, 2);
            ethtype[j][3] = '\0';
            eth_type = eth_type + 1;
            eth_type = eth_type + 1;
            iethtype[j] = HexStringToUInt(ethtype[j]);
            sendbuf[tx_len++] = iethtype[j];
        }

    }
    else
    {
        sendbuf[tx_len++] = ETH_TYPE0;
        sendbuf[tx_len++] = ETH_TYPE1;
    }
    sendbuf[tx_len++] = PAYLOAD0;
    sendbuf[tx_len++] = PAYLOAD1;
    sendbuf[tx_len++] = PAYLOAD2;
    seq_index = tx_len;
    tx_len++;


    if (strm_id_flag)
    {
        for (j = 0; j < 8; j++)
        {
            strid[j] = (char *)malloc( 3 * sizeof(char));
            strncpy(strid[j], str_id, 2);
            strid[j][3] = '\0';
            str_id = str_id + 1;
            str_id = str_id + 1;
            str_id = str_id + 1;
            istrid[j] = HexStringToUInt(strid[j]);
            sendbuf[tx_len+j] = istrid[j];
        }
        tx_len = tx_len + 8;
    }
    else
    {
        sendbuf[tx_len++] = PAYLOAD4;
        sendbuf[tx_len++] = PAYLOAD5;
        sendbuf[tx_len++] = PAYLOAD6;
        sendbuf[tx_len++] = PAYLOAD7;
        sendbuf[tx_len++] = PAYLOAD8;
        sendbuf[tx_len++] = PAYLOAD9;
        sendbuf[tx_len++] = PAYLOAD10;
        sendbuf[tx_len++] = PAYLOAD11;
    }
    sendbuf[tx_len++] = PAYLOAD12;
    sendbuf[tx_len++] = PAYLOAD13;
    sendbuf[tx_len++] = PAYLOAD14;
    sendbuf[tx_len++] = PAYLOAD15;
    sendbuf[tx_len++] = PAYLOAD16;
    sendbuf[tx_len++] = PAYLOAD17;
    sendbuf[tx_len++] = PAYLOAD18;
    sendbuf[tx_len++] = PAYLOAD19;
    pay_load_start = tx_len;

    if (NOT payload_flag)
    {
        sendbuf[tx_len++] = PAYLOAD20;
        sendbuf[tx_len++] = PAYLOAD21;
        sendbuf[tx_len++] = PAYLOAD22;
        sendbuf[tx_len++] = PAYLOAD23;
        sendbuf[tx_len++] = PAYLOAD24;
        sendbuf[tx_len++] = PAYLOAD25;
        sendbuf[tx_len++] = PAYLOAD26;
        sendbuf[tx_len++] = PAYLOAD27;
        //sendbuf[tx_len++] = PAYLOAD28;
    }
    else
    {
        if (NOT dtype_flag)
        {
            for (n = 0; n < (payload_count); n++)
            {
                sendbuf[tx_len++] = payload_array[n];
            }

        }
        else
        {
            switch (data_type[0])
            {
                case'a':
                    memset(payload_array, 0xAA, MAX_PAYLOAD);
                    break;
                case'0':
                    memset(payload_array, 0x00, MAX_PAYLOAD);
                    break;
                case'1':
                    memset(payload_array, 0x11, MAX_PAYLOAD);
                    break;
                case'5':
                    memset(payload_array, 0x55, MAX_PAYLOAD);
                    break;
                default:
                    memset(payload_array, 0x00, MAX_PAYLOAD);
                    break;
            }
            for (n = 0; n <= (payload_count); n++)
            {
                sendbuf[tx_len++] = payload_array[n];
            }
        }
    }

     /* Index of the network device */
    socket_address.sll_ifindex = if_idx.ifr_ifindex;
    /* Address length*/
    socket_address.sll_halen = ETH_ALEN;
    /* Destination MAC */
    socket_address.sll_addr[0] = MY_DEST_MAC0;
    socket_address.sll_addr[1] = MY_DEST_MAC1;
    socket_address.sll_addr[2] = MY_DEST_MAC2;
    socket_address.sll_addr[3] = MY_DEST_MAC3;
    socket_address.sll_addr[4] = MY_DEST_MAC4;
    socket_address.sll_addr[5] = MY_DEST_MAC5;

    (void) signal(SIGALRM, timer_function);

    /* Install Timer of 1 sec */
    proc_timer_init();

    /* initialized with default attributes */
    err = pthread_attr_init (&tattr);
    /* safe to get existing scheduling param */
    err = pthread_attr_getschedparam (&tattr, &param);

    /* set the priority; others are unchanged */
    param.sched_priority = 15;

    /* setting the new scheduling param */
    err = pthread_attr_setschedparam (&tattr, &param);



    err = pthread_create(&thread_id1, &tattr, (void * (*)(void *))&console_thread, NULL);

    if (err != 0)
    {
        fprintf(stderr, "ERROR: can't create Console Thread - %u(%s)", err, strerror(err));
        exit(1);
    }

    /* Send Packets based on count number */
    if (NOT rate_flag)
    {
        if (count_flag)
        {
            //struct timespec now;
            for (i = 0; i < tx_count; i++)
            {

   // if(flip == 0)
   // {
     //   sendbuf[VLAN_index] = ((((4 << 1) | CFE ) << 4) | VID0 );
       // flip = 0;
   // }
   // else if(flip == 1)
    //{
      //  sendbuf[VLAN_index] = ((((7 << 1) | CFE ) << 4) | VID0 );
       // flip = 2;

   // }
   // else if(flip == 2)
   // {
     //   sendbuf[VLAN_index] = ((((3 << 1) | CFE ) << 4) | VID0 );
      //  flip = 0;

   // }

          /* Attach Sequence Number */
                sendbuf[seq_index] = seq_num++;
                sendbuf_preempt[seq_index_preempt] = seq_num_preempt++;
				if (applyRTagSeq()) {
                	//printf("Skip R-Tag %d\n", *rtag_seq_number);
				}
                else if ( (strcasecmp(ifName, GWCPU_INTERFACE) != 0) AND (strcasecmp(ifName, ICUSA_INTERFACE) != 0) )
                {


                    if (sendto(sockfd, sendbuf, tx_len, 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0)
                    {
                        fprintf(stderr, "ERROR: Could Not Send over interface '%s' - %d(%s) \n", ifName, errno, strerror(errno));
                        exit(1);
                    }

                    usleep(1);

                }
                else if (strcasecmp(ifName, GWCPU_INTERFACE) == 0)
                {
                    write(dp, sendbuf, tx_len);
                    delay_count++;
                    if ((test_iface[0] == 'c') AND (delay_count == 50))
                    {
                        usleep(1);
                        delay_count = 0;
                    }
                    else if ((test_iface[0] == 's') AND (delay_count == 500))
                    {
                        usleep(1);
                        delay_count = 0;
                    }
                    if(preempt_flag)
                    {
                        printf("Sending Express Frame1 \n");
                        write(dp, sendbuf_preempt, tx_len_preempt);
                        printf("Sending Express Frame2 \n");
                        write(dp, sendbuf_preempt, tx_len_preempt);
                    }
                }
                else if (strcasecmp(ifName, ICUSA_INTERFACE) == 0)
                {
                    k = 0;
                    for (j = pay_load_start; j < payload_count + pay_load_start; j++)
                    {
                        sendbuf_icusa[k] = sendbuf[j];
                        k++;
                    }

                    write(ds,sendbuf_icusa,k);

                    icusa_write_flag = TRUE;


                    /* initialized with default attributes */
                    err = pthread_attr_init (&tattr);
                    /* safe to get existing scheduling param */
                    err = pthread_attr_getschedparam (&tattr, &param);

                    /* set the priority; others are unchanged */
                    param.sched_priority = sched_get_priority_max(SCHED_RR);


                    /* setting the new scheduling param */
                    err = pthread_attr_setschedparam (&tattr, &param);


                    err = pthread_create(&thread_id,&tattr, (void * (*)(void *))&rcv_thread, NULL);

                    if (err != 0)
                    {
                        fprintf(stderr, "ERROR: can't create Reception Thread - %u(%s)\n", err, strerror(err));
                        exit(1);
                    }

                    pthread_mutex_destroy(&lock);

                    break; /* Write only once for ICUSA Rest in Receive thread */

                }
            }
            printf("All frames send\n");
            if (end_after_send)
				stop = 1;
        }
        /* Send till TERM signal */
        else
        {

            if((strcasecmp(ifName,ICUSA_INTERFACE)) == 0)
            {
                /* initialized with default attributes */
                err = pthread_attr_init (&tattr);
                /* safe to get existing scheduling param */
                err = pthread_attr_getschedparam (&tattr, &param);

                /* set the priority; others are unchanged */
                param.sched_priority = sched_get_priority_max(SCHED_RR);


                /* setting the new scheduling param */
                err = pthread_attr_setschedparam (&tattr, &param);


                err = pthread_create(&thread_id,&tattr, (void * (*)(void *))&rcv_thread, NULL);

                if (err != 0)
                {
                    fprintf(stderr, "ERROR: can't create Reception Thread - %u(%s)\n", err, strerror(err));
                    exit(1);
                }

                pthread_mutex_destroy(&lock);

            }

            while (!stop)
            {
                //  if(flip == 0)
   // {
     //   sendbuf[VLAN_index] = ((((4 << 1) | CFE ) << 4) | VID0 );
       // flip = 1;
   // }
   // else if(flip == 0)
   // {
     //   sendbuf[VLAN_index] = ((((7 << 1) | CFE ) << 4) | VID0 );
      //  flip = 2;

   // }
   // else if(flip == 2)
   // {
     //   sendbuf[VLAN_index] = ((((3 << 1) | CFE ) << 4) | VID0 );
       // flip = 0;

   // }


                /* Attach Sequence Number */
                sendbuf[seq_index] = seq_num++;
                /* Send packet */
                if ( (strcasecmp(ifName, GWCPU_INTERFACE) != 0) AND (strcasecmp(ifName,ICUSA_INTERFACE) != 0) )
                {
                    if (sendto(sockfd, sendbuf, tx_len, 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0)
                    {
                        fprintf(stderr , "ERROR: Could Not Send over interface '%s' - %d(%s) \n",ifName, errno, strerror(errno));
                        exit(1);
                    }
                }
                else if(strcasecmp(ifName, GWCPU_INTERFACE) == 0)
                {
                    write(dp, sendbuf, tx_len);
                    delay_count++;
                    if ((test_iface[0] == 'c') AND (delay_count == 50))
                    {
                        usleep(1);
                        delay_count = 0;
                    }
                    else if((test_iface[0] == 's') AND (delay_count == 500))
                    {
                        usleep(1);
                        delay_count = 0;
                    }
                }
            }
        }
    }

    /* Execute Till TERM SIGNAL */
    while (NOT stop)
    {
        //sleep(1);
        usleep(100000);  //100 ms
    }

    if (file_flag)
    {
        fclose(fp);
    }

    if (dp != -1)
    {
        err = close(dp);
        if (err < 0)
        {
            fprintf(stderr, "ERROR: GWCPU close");
        }
        else
        {
            printf("GWCPU close success \n");
        }
    }

    if (ds != -1)
    {
        err = close(ds);
        if (err < 0)
        {
            fprintf(stderr, "ERROR: ICUSA close");
        }
        else
        {
            printf("ICUSA close success \n");
        }
    }

    if (sockfd != -1)
    {
        close(sockfd);
    }

    return 0;
}

/*
    Change History
    2015-11-27  1.0.0   Fist version
                1.1.0   AK
    2015-12-04  1.2.0   Add console. More user-friendly
    2015-12-07  1.2.1   Show more defaults on help display. Move console thread to own function
    2015-12-08  1.2.2   Cleanup. Error messages
    2016-01-26  1.2.3   Add race_gwhost
    2016-02-01  1.2.4   New -t parameters to slow down Tx through Gw-Host interface (work-around for FPGA problems)
    2016-02-25  1.2.5   Add race_icusa
    2016-03-17  1.2.6   Removed delays and optimised for ICUSA CEE2
    2016-03-17  1.2.7   Bug fix for NTCF length
    2016-05-26  1.3.0   Changed as per new structure of gw-host, added option for destination mac and ether type
    2016-06-21  1.4.0   Increased Maximum frame Size to 2000
    2016-09-07  1.5.0   Functionality to test Preemption
*/


/*
* Local variables:
* Mode: C
* tab-width: 4
* indent-tabs-mode: nil
* c-basic-offset: 4
* End:
* vim: ts=4 expandtab sw=4
*/
