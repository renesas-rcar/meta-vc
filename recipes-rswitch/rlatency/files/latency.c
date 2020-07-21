/**
    @brief Latency Test Tool

    This Latency TEST tool will be capturing the RX timestamp for
    RAW, TCP & UDP connections and calculate latency based on the 
    inserted TX timestamp information inserted in the frame. 

    @file   latency.c

    @todo
    - 

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




/* Example applicationfor capturing the RX timestamp for   
 * RAW, TCP & UDP connections and calculate latency based on the
 * inserted TX timestamp information inserted in the frame 
 * This application will also display their 
 * hardware timestamps.
 *
 * Invoke with "--help" to see the options it supports.
 *
 * Example:
 * (host1)$ rx_timestamping
 * UDP socket created, listening on port 9000
 * Selecting software timestamping mode.
 * (host2)$ echo payload | nc -u host1 9000
 * Packet 0 - 8 bytes timestamp 1395768726.443243000
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <time.h>
#include <getopt.h>
#include <linux/filter.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if_ether.h>
#include <net/ethernet.h>
#include <linux/sockios.h>
#include <linux/if_packet.h>
#define CONST_ROUT_DELAY 0xED8

/* Use the kernel definitions if possible -
 * But if not, use our own local definitions, and Onload will allow it.
 * - Though you still need a reasonably recent kernel to get hardware
 *   timestamping.
 */
#ifdef NO_KERNEL_TS_INCLUDE
#include <time.h>
struct hwtstamp_config {
        int flags;           /* no flags defined right now, must be zero */
        int tx_type;         /* HWTSTAMP_TX_* */
        int rx_filter;       /* HWTSTAMP_FILTER_* */
};
enum {
        SOF_TIMESTAMPING_TX_HARDWARE = (1<<0),
        SOF_TIMESTAMPING_TX_SOFTWARE = (1<<1),
        SOF_TIMESTAMPING_RX_HARDWARE = (1<<2),
        SOF_TIMESTAMPING_RX_SOFTWARE = (1<<3),
        SOF_TIMESTAMPING_SOFTWARE = (1<<4),
        SOF_TIMESTAMPING_SYS_HARDWARE = (1<<5),
        SOF_TIMESTAMPING_RAW_HARDWARE = (1<<6),
        SOF_TIMESTAMPING_MASK =
            (SOF_TIMESTAMPING_RAW_HARDWARE - 1) |
            SOF_TIMESTAMPING_RAW_HARDWARE
};
#else
#include <linux/net_tstamp.h>
#include <linux/sockios.h>
#endif

//Added by TRK
#define MAX_RAND_NUM 115
#define MAX_CHAR_ARR 10
struct  Plot_graph{                               
  //  unsigned char char_arr[MAX_CHAR_ARR][4];
    unsigned char char_arr[4];
    int arr[MAX_RAND_NUM];
//    unsigned int x_samples[MAX_RAND_NUM];
};          

struct Plot_graph graph_input;
int server_fd2;

/* These are defined in socket.h, but older versions might not have all 3 */
#ifndef SO_TIMESTAMP
#define SO_TIMESTAMP            29
#endif
#ifndef SO_TIMESTAMPNS
#define SO_TIMESTAMPNS          35
#endif
#ifndef SO_TIMESTAMPING
#define SO_TIMESTAMPING         37
#endif

/* Seconds.nanoseconds format */
#define TIME_FMT "%" PRIu64 ".%.9" PRIu64 " "

/* Assert-like macros */
#define TEST(x)                                                 \
    do {                                                          \
            if( ! (x) ) {                                               \
                    fprintf(stderr, "ERROR: '%s' failed\n", #x);              \
                    fprintf(stderr, "ERROR: at %s:%d\n", __FILE__, __LINE__); \
                    exit(1);                                                  \
            }                                                           \
    } while( 0 )

#define TRY(x)                                                          \
    do {                                                                  \
            int __rc = (x);                                                     \
            if( __rc < 0 ) {                                                  \
                    fprintf(stderr, "ERROR: TRY(%s) failed\n", #x);                 \
                    fprintf(stderr, "ERROR: at %s:%d\n", __FILE__, __LINE__);       \
                    fprintf(stderr, "ERROR: rc=%d errno=%d (%s)\n",                 \
                            __rc, errno, strerror(errno));                          \
                    exit(1);                                                        \
            }                                                                 \
    } while( 0 )

struct configuration {
        char const*    cfg_ioctl;     /* e.g. eth6  - calls the ts enable ioctl */
        unsigned short cfg_port;      /* listen port */
        int            cfg_protocol;  /* udp or tcp? */
        unsigned int   cfg_max_packets; /* Stop after this many (0=forever) */
};

/* Commandline options, configuration etc. */

void print_help(void)
{
    printf("Usage:\n"
            "\t--ioctl\t<ethX>\tDevice to send timestamping enable ioctl.  "
            "Default: None\n"
            "\t--port\t<num>\tPort to listen on.  "
            "Default: 9000\n"
            "\t--proto\t[TCP|UDP|RAW].  "
            "Default: UDP\n"
            "\t--max\t<num>\tStop after n packets.  "
            "Default: Run forever\n"
          );
    exit(-1);
}


static void get_protcol(struct configuration* cfg, const char* protocol)
{
    if( 0 == strcasecmp(protocol, "UDP") ) {
            cfg->cfg_protocol = IPPROTO_UDP;
    }
    else if( 0 == strcasecmp(protocol, "TCP") ) {
            cfg->cfg_protocol = IPPROTO_TCP;
    }
    else if( 0 == strcasecmp(protocol, "RAW") ) {
            cfg->cfg_protocol = ETH_P_ALL;
    }
    else {
            printf("ERROR: '%s' is not a recognised protocol (TCP or UCP).\n",
                    protocol);
            exit(-EINVAL);
    }

}


static void parse_options( int argc, char** argv, struct configuration* cfg )
{
    int option_index = 0;
    int opt;
    static struct option long_options[] = {
            { "ioctl", required_argument, 0, 'i' },
            { "port", required_argument, 0, 'p' },
            { "proto", required_argument, 0, 'P' },
            { "max", required_argument, 0, 'n' },
            { 0, no_argument, 0, 0 }
    };
    const char* optstring = "i:p:P:n:";

    /* Defaults */
    bzero(cfg, sizeof(struct configuration));
    cfg->cfg_port = 9000;
    cfg->cfg_protocol = IPPROTO_UDP;

    opt = getopt_long(argc, argv, optstring, long_options, &option_index);
    while( opt != -1 ) {
            switch( opt ) {
                    case 'i':
                        cfg->cfg_ioctl = optarg;
                    break;
                    case 'p':
                        cfg->cfg_port = atoi(optarg);
                    break;
                    case 'P':
                        get_protcol(cfg, optarg);
                    break;
                    case 'n':
                        cfg->cfg_max_packets = atoi(optarg);
                    break;
                    default:
                        print_help();
                    break;
            }
            opt = getopt_long(argc, argv, optstring, long_options, &option_index);
    }
}


/* Connection */
static void make_address(unsigned short port, struct sockaddr_in* host_address)
{
    bzero(host_address, sizeof(struct sockaddr_in));

    host_address->sin_family = AF_INET;
    host_address->sin_port = htons(port);
    host_address->sin_addr.s_addr = INADDR_ANY;
}


/* This requires a bit of explanation.
 * Typically, you have to enable hardware timestamping on an interface.
 * Any application can do it, and then it's available to everyone.
 * The easiest way to do this, is just to run sfptpd.
 *
 * But in case you need to do it manually; here is the code, but
 * that's only supported on reasonably recent versions
 *
 * Option: --ioctl ethX
 *
 * NOTE:
 * Usage of the ioctl call is discouraged. A better method, if using
 * hardware timestamping, would be to use sfptpd as it will effectively
 * make the ioctl call for you.
 *
 */
static void do_ioctl(struct configuration* cfg, int sock)
{
#ifdef SIOCSHWTSTAMP
    struct ifreq ifr;
    struct hwtstamp_config hwc;
#endif

    if( cfg->cfg_ioctl == NULL )
            return;

#ifdef SIOCSHWTSTAMP
    bzero(&ifr, sizeof(ifr));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", cfg->cfg_ioctl);

    /* Standard kernel ioctl options */
    hwc.flags = 0;
    hwc.tx_type = 0;
    hwc.rx_filter = HWTSTAMP_FILTER_ALL;

    ifr.ifr_data = (char*)&hwc;

    TRY( ioctl(sock, SIOCSHWTSTAMP, &ifr) );
    return;
#else
    (void) sock;
    printf("SIOCHWTSTAMP ioctl not supported on this kernel.\n");
    exit(-ENOTSUP);
    return;
#endif
}


/* This routine selects the correct socket option to enable timestamping. */
static void do_ts_sockopt(int sock)
{
    int enable = 1;

    //  printf("Selecting hardware timestamping mode.\n"); //TRK
    enable = SOF_TIMESTAMPING_RX_HARDWARE | SOF_TIMESTAMPING_RAW_HARDWARE
        | SOF_TIMESTAMPING_SYS_HARDWARE | SOF_TIMESTAMPING_SOFTWARE;
    TRY(setsockopt(sock, SOL_SOCKET, SO_TIMESTAMPING, &enable, sizeof(int)));
}

static int add_socket(struct configuration* cfg)
{
    int s;
    struct sockaddr_ll host_address;
    int domain = SOCK_DGRAM;
    if ( cfg->cfg_protocol == IPPROTO_TCP )
            domain = SOCK_STREAM;
    if ( cfg->cfg_protocol != ETH_P_ALL)
    {
        make_address(cfg->cfg_port, &host_address);
        make_address(cfg->cfg_port, &host_address);

        s = socket(PF_INET, domain, cfg->cfg_protocol);
        TEST(s >= 0);
        TRY(bind(s, (struct sockaddr*)&host_address, sizeof(host_address)) );

        printf("Socket created, listening on port %d\n", cfg->cfg_port);

    }
    else
    {
        struct ifreq ifreq;
        int err;
        s = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
        TEST(s >= 0);
        memset(&ifreq, 0, sizeof(ifreq));
        strcpy(ifreq.ifr_name, cfg->cfg_ioctl);
        //     printf("cfg->ioctl is %s \n", cfg->cfg_ioctl); //TRK
        err = ioctl(s, SIOCGIFINDEX, &ifreq);
        if (err < 0) {
                printf("Port %s: ioctl SIOCGIFINDEX failed", cfg->cfg_ioctl);
                return err;
        }
        memset(&host_address, 0, sizeof(host_address));
        host_address.sll_ifindex = ifreq.ifr_ifindex;
        host_address.sll_family = AF_PACKET;
        host_address.sll_protocol = htons(ETH_P_ALL);
        TRY(bind(s, (struct sockaddr*)&host_address, sizeof(host_address)) );

        //      printf("Socket created, listening on port %d\n", cfg->cfg_port); //TRK
    }
    return s;
}


static int accept_child(int parent)
{
    int child;
    printf("In accept child \n");
    socklen_t clilen;
    struct sockaddr_in cli_addr;
    clilen = sizeof(cli_addr);

    TRY(listen(parent, 1));
    child = accept(parent, (struct sockaddr* ) &cli_addr, &clilen);
    TEST(child >= 0);

    printf("Socket accepted\n");
    return child;
}


/* Processing */
static void print_time(struct timespec* ts, char buffer[2048],int got)
{

    struct timespec tx_ts;
    struct timespec latency;
    int i  = 0;
    int itr = 0;
#ifdef DEBUG
    int i  = 0;
#endif 
    if( ts != NULL ) {
            /* Hardware timestamping provides three timestamps -
             *   system (software)
             *   transformed (hw converted to sw)
             *   raw (hardware)
             * in that order - though depending on socket option, you may have 0 in
             * some of them.
             */
#if 0
            printf("timestamps " TIME_FMT TIME_FMT TIME_FMT "\n",
                    (uint64_t)ts[0].tv_sec, (uint64_t)ts[0].tv_nsec,
                    (uint64_t)ts[1].tv_sec, (uint64_t)ts[1].tv_nsec,
                    (uint64_t)ts[2].tv_sec, (uint64_t)ts[2].tv_nsec );
#endif
            //tx_ts.tv_sec = ((uint64_t)(buffer[44] & 0xFF) << 40) | ((uint64_t)(buffer[45] & 0xFF) << 32) | ((uint64_t)(buffer[46] & 0xFF) << 24) | 
              //  ((uint64_t)(buffer[47] & 0xFF) << 16) | ((uint64_t)(buffer[48] & 0xFF) << 8) | ((uint64_t)buffer[49] & 0xFF);
            //tx_ts.tv_nsec =  ((uint64_t)(buffer[50] & 0xFF) << 24) |  ((uint64_t)(buffer[51] & 0xFF) << 16) | ((uint64_t)(buffer[52] & 0xFF) << 8) | ((uint64_t)buffer[53] & 0xFF);
            tx_ts.tv_sec = ((uint64_t)(buffer[48] & 0xFF) << 40) | ((uint64_t)(buffer[49] & 0xFF) << 32) | ((uint64_t)(buffer[50] & 0xFF) << 24) | 
                ((uint64_t)(buffer[51] & 0xFF) << 16) | ((uint64_t)(buffer[52] & 0xFF) << 8) | ((uint64_t)buffer[53] & 0xFF);
            tx_ts.tv_nsec =  ((uint64_t)(buffer[54] & 0xFF) << 24) |  ((uint64_t)(buffer[55] & 0xFF) << 16) | ((uint64_t)(buffer[56] & 0xFF) << 8) | ((uint64_t)buffer[57] & 0xFF);

            if((tx_ts.tv_sec !=0) || (tx_ts.tv_nsec !=0)  && (buffer[16] ==0x22) && (buffer[17] == 0xF0))
            {
                if(ts[2].tv_nsec > tx_ts.tv_nsec)
                {
                    latency.tv_sec = ts[2].tv_sec - tx_ts.tv_sec;
                    latency.tv_nsec =( ts[2].tv_nsec - tx_ts.tv_nsec) + CONST_ROUT_DELAY;
                    if ((uint64_t)latency.tv_nsec < 120000) //Added by TRK
                    {
    //                        printf("" TIME_FMT  "\n",
      //                             (uint64_t)latency.tv_sec, (uint64_t)latency.tv_nsec );
        //            printf("%d\n", (uint64_t)(latency.tv_nsec/10));
//
#if 1 
    //                    printf("buffer[15] : %x\n",(buffer[40]&0xFF));
                        if((buffer[40]&0xFF) == 0x77)
                        {
                            graph_input.char_arr[0] = '1';
                        }
                        else if((buffer[40]&0xFF) == 0x88)
                        {
                            graph_input.char_arr[0] = '2';
                        }
						else if((buffer[40]&0xFF) == 0x66)
                        {
                            graph_input.char_arr[0] = '3';
                        }
                        else if((buffer[40]&0xFF) == 0x55)
                        {
                            graph_input.char_arr[0] = '4';
                        }
						else
						{
                            graph_input.char_arr[0] = '9';
                        }	
#endif                        
                         graph_input.arr[0] =  (latency.tv_nsec);
	    }
            if( (buffer[16] ==0x22) && (buffer[17] == 0xF0))
	    {

            printf("Latency " TIME_FMT  "\n",
                    (uint64_t)latency.tv_sec, (uint64_t)latency.tv_nsec );


#if 0
#if 0
/*Left Shift*/
                    memmove(&graph_input.arr[0], &graph_input.arr[1], MAX_RAND_NUM*sizeof(int));
                    graph_input.arr[MAX_RAND_NUM] =  (latency.tv_nsec/10);
/*Right Shift*/
#else
                 graph_input.arr[0] =  (latency.tv_nsec/10);
                 for(itr = (MAX_RAND_NUM-1); itr>0; itr--)
                 {
                    graph_input.arr[itr] = graph_input.arr[itr-1];
                 }
#endif
#endif

                        if (send(server_fd2, &graph_input, sizeof(graph_input), 0) < 0)
                        {   
                            printf("sending of array of integers failed");
                        }     
                        printf("value sent :%d\n ", graph_input.arr[0]);
                    }
                }
            }
#if 0
            printf("Latency " TIME_FMT  "\n",
                    (uint64_t)latency.tv_sec, (uint64_t)latency.tv_nsec );
#endif
#if 1
            for(i=16; i < 18; i++)
            {
                printf("RX_byte[%d]=%x\n",i,(buffer[i]&0xFF));
            }
#endif
#if 0
            printf("TX timestamps " TIME_FMT  "\n",
                    (uint64_t)tx_ts.tv_sec, (uint64_t)tx_ts.tv_nsec );
            for(i=0; i < got; i++)
            {
                printf("RX_byte[%d]=%x\n",i,(buffer[i]&0xFF));
            }
#endif

    } 
#if 0
    else
    {
        printf( "no timestamp\n" );
    }
#endif
}


/* Given a packet, extract the timestamp(s) */
static void handle_time(struct msghdr* msg, char buffer[2048],int got)
{
    struct timespec* ts = NULL;
    struct cmsghdr* cmsg;

    for( cmsg = CMSG_FIRSTHDR(msg); cmsg; cmsg = CMSG_NXTHDR(msg,cmsg) ) {
            if( cmsg->cmsg_level != SOL_SOCKET )
                    continue;

            switch( cmsg->cmsg_type ) {
                    case SO_TIMESTAMPNS:
                        ts = (struct timespec*) CMSG_DATA(cmsg);
                    break;
                    case SO_TIMESTAMPING:
                        ts = (struct timespec*) CMSG_DATA(cmsg);
                    break;
                    default:
                        /* Ignore other cmsg options */
                    break;
            }
    }

    print_time(ts,buffer,got);
}

/* Receive a packet, and print out the timestamps from it */
static int do_recv(int sock, unsigned int pkt_num)
{
    struct msghdr msg;
    struct iovec iov;

    struct sockaddr_in host_address;
    char buffer[2048];
    char control[1024];
    int got;
    //printf("In do recv\n");
    /* recvmsg header structure */
    make_address(0, &host_address);
    iov.iov_base = buffer;
    iov.iov_len = 2048;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_name = &host_address;
    msg.msg_namelen = sizeof(struct sockaddr_in);
    msg.msg_control = control;
    msg.msg_controllen = 1024;

    /* block for message */
    got = recvmsg(sock, &msg, 0);
    if( !got && errno == EAGAIN )
            return 0;

     // printf("Packet %d - %d bytes\t", pkt_num, got);
    handle_time(&msg,buffer,got);

    return got;
};


int main(int argc, char** argv)
{
    struct configuration cfg;
    int parent, sock, got;
    unsigned int pkt_num = 0;
    char hostname[17];
     struct sockaddr_in serveraddr;
struct hostent *rserver;

    int client_fd2, i;
    struct sockaddr_in server2, client2;
    


    parse_options(argc, argv, &cfg);


#if 1 //Added by TRK

     /*Duplicate socket start */
    server_fd2 = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd2 < 0) 
    {
        printf("Could not create socket\n");
        exit(0);
    }

    strcpy(hostname, "192.168.0.15");
    /* gethostbyname: get the server's DNS entry */                     
    rserver = gethostbyname(hostname);
    if (rserver == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", hostname);
        exit(0);
    }

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)rserver->h_addr, 
            (char *)&serveraddr.sin_addr.s_addr, rserver->h_length);
    serveraddr.sin_port = htons(9999);
    /* connect: create a connection with the server */
    if (connect(server_fd2, &serveraddr, sizeof(serveraddr)) < 0)
        printf("ERROR connecting");

//    strcpy( graph_input.char_arr, "AA");

    memset(graph_input.arr, 0, MAX_RAND_NUM);
//    graph_input.char_arr[0] = '1';

#endif

    /* Initialise */
    parent = add_socket(&cfg);
    do_ioctl(&cfg, parent);
    sock = parent;
    if( cfg.cfg_protocol == IPPROTO_TCP )
            sock = accept_child(parent);
    do_ts_sockopt(sock);
    //printf("After TS SOCKOP \n");
    /* Run forever */
    while((pkt_num++ < cfg.cfg_max_packets || (cfg.cfg_max_packets == 0) ) ) {
            got = do_recv(sock, pkt_num);
            /* TCP can detect an exit; for UDP, zero payload packets are valid */
            if ( got == 0 && cfg.cfg_protocol == IPPROTO_TCP ) {
                    printf( "recvmsg returned 0 - end of stream\n" );
                    break;
            }
    }

    close(sock);
    if( cfg.cfg_protocol == IPPROTO_TCP )
            close(parent);
    return 0;
}

/*
    Change History
    2018-06-27  0.0.1  AK   Fist version


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



