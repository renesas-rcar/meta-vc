/*                                                               
* @brief RSwitch TSN Configuration (header)
*
* 
* 
*
*/


#include <mxml.h>


#define SWITCH_AVB_TSN_DEV_NAME      "/dev/"RSWITCH_ETHERNET_DEVICE_NAME
#define DEBUG_CBS




extern bool TSN_Open_Driver(void);
extern bool TSN_Close_Driver(void);
extern bool TSN_Configure_Device(void);
extern bool TSN_Report_Device(void);
void        TSN_Reset_Configuration(void);
bool        TSN_Set_Config(mxml_node_t * Tree);
