/*
* @brief RSwitch Configuration & Control tool (header)
*
* @author
* -
*
*/

#include <mxml.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <stdint.h>


#include <sys/ioctl.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <math.h>
#include <pthread.h>
#include <poll.h>
#include <errno.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>

#include  <include/linux/renesas_rswitch.h> 

#define BUILD_OPTION_DEBUG_FPGA         
#define BUILD_OPTION_DEBUG_FPGA_ALLOW_UPDATE    

#ifndef NOT
#define  NOT !
#endif

#ifndef AND
#define AND &&
#endif

#ifndef OR
#define OR ||
#endif


#define bool int

#define RSWITCHTOOL_VERSION          "3.0.2"

#define DEFAULT_CONFIG_FILE         "/etc/rst/switch_static.xml"
#define DEFAULT_BIT_FILE         "/home/root/bitfile.bin"
#define FALSE                       0
#define TRUE                        1

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define MIN(a,b)        (((a)<(b))?(a):(b))
#define MAX(a,b)        (((a)>(b))?(a):(b))

const char * const  SetConfig_GetText(mxml_node_t * Stream, mxml_node_t * tree, char const * const Tag);
