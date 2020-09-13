/*                                                                                           
*  Rswitch Configuration Tool - Ethernet TSN Port
*
*  Copyright (C) 2014 Renesas Electronics Corporation
*
*  This program is free software; you can redistribute it and/or modify it
*  under the terms and conditions of the GNU General Public License,
*  version 2, as published by the Free Software Foundation.
*
*  This program is distributed in the hope it will be useful, but WITHOUT
*  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
*  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
*  more details.
*  You should have received a copy of the GNU General Public License along wit
*  this program; if not, write to the Free Software Foundation, Inc.,
*  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
*
*  The full GNU General Public License is included in this distribution in
*  the file called "COPYING".
*/

#include "rswitchtool.h"
#include "rswitch_FWD.h"
#include <stdlib.h>
#include <math.h>
#include <inttypes.h>
#include <drivers/net/ethernet/renesas/rswitch_fwd/rswitch_fwd.h> 


#define DEFAULT_REPORT_PRINT '-'
#define INVALID_THRESHOLD_CONFIG_VAL 20
#define MAX_CSDN 255
#define MAX_PRIORITY 7
unsigned int           G_Reconfig = 0; 
extern char          * gOptConfigFile;
static int             gFwdFd          = -1;
char gMACArray[RENESAS_RSWITCH_MAX_FWD_TBL_ENTRY][18];

struct rswitch_fwd_Config FWDConfig_t;



static void Print_uint128(uint128_t value)
{
    uint128_t hightemp = value;
    uint128_t lowtemp = 0;
    int length = 0, hlength = 0, llength = 0;
    if(hightemp > UINT64_MAX)
    {
        for(int i = 0; hightemp > UINT64_MAX; i++)
        {
            lowtemp = lowtemp + pow(10,i) * (hightemp % 10);
            hightemp = hightemp / 10;
        }
    }
    length = (int)((ceil(log10(value))+1)*sizeof(char)) - 1;
    hlength = (int)((ceil(log10(hightemp))+1)*sizeof(char)) - 1;
    llength = (int)((ceil(log10(lowtemp))+1)*sizeof(char)) - 1;
    
    printf("%"PRIu64"", hightemp);
    
    if((lowtemp != 0) && ((hlength+llength) < length))
    {
        for(int i = (hlength + llength); i < length; i++)
        printf("0");                                    //in case lowtemp begins with one or multiple 0 characters
        printf("%"PRIu64"", lowtemp);
    }
    for(int i = 0; i <= (24 - (length)); i++)
    printf(" ");
}

void FWD_Print_Configuration()
{
    int Count = 0;
    int Count2 = 0;
    int Count3 = 0;
    /*MAC Printing*/
    printf("\n======================================= FORWARDING ENGINE =====================================\n");
    if(FWDConfig_t.qci_config.bEnable)
    {
        printf("----------------------------  Ingress Filter  ---------------------------------");
        for(Count = 0; Count < FWDConfig_t.qci_config.Schedules; Count++)
        {
            printf("\n-----------------------------  Schedule %d  ------------------------------------\n", Count);
            printf("PortNumber      JitterClock      Latency      AdminStartTime           gptp-timer-domain      CycleTimeExtension\n");
            printf("--------------------------------------------------------------------------------\n");
            printf("%-16u", FWDConfig_t.qci_config.QCI_Gate_Config[Count].PortNumber);
            printf("%-17u", FWDConfig_t.qci_config.QCI_Gate_Config[Count].jitter_time);
            printf("%-13u", FWDConfig_t.qci_config.QCI_Gate_Config[Count].latency_time);
            Print_uint128(FWDConfig_t.qci_config.QCI_Gate_Config[Count].AdminBaseTime.nseconds);
            printf("%u                      ", FWDConfig_t.qci_config.QCI_Gate_Config[Count].timer_domain);
            Print_uint128(FWDConfig_t.qci_config.QCI_Gate_Config[Count].CycleTimeExtension.nseconds);
            
            printf("\n----------------------------  GateControl  -------------------------\n");
            printf("GateID   InvalidRx    IPV    GateTimeTickMultiplier    State     IPV\n");
            printf("--------------------------------------------------------------------\n");
            for(Count2 = 0; Count2 < FWDConfig_t.qci_config.QCI_Gate_Config[Count].Gates; Count2++)
            {
                printf("%-9d", FWDConfig_t.qci_config.QCI_Gate_Config[Count].Gate[Count2].GateID);
                printf("%-13s", FWDConfig_t.qci_config.QCI_Gate_Config[Count].Gate[Count2].Invalid_Rx ? "True" : "False");
                printf("%-7d", FWDConfig_t.qci_config.QCI_Gate_Config[Count].Gate[Count2].IPV_enable);
                printf("%-26u", FWDConfig_t.qci_config.QCI_Gate_Config[Count].Gate[Count2].GateControl[0].GateTimeTickMultiplier);
                printf("%-10s", FWDConfig_t.qci_config.QCI_Gate_Config[Count].Gate[Count2].GateControl[0].State ? "Open" : "Closed");
                printf("%u\n", FWDConfig_t.qci_config.QCI_Gate_Config[Count].Gate[Count2].GateControl[0].IPV);
                for(Count3 = 1; Count3 < FWDConfig_t.qci_config.QCI_Gate_Config[Count].Gate[Count2].GateControls; Count3++)
                {
                    printf("                             ");
                    printf("%-26u", FWDConfig_t.qci_config.QCI_Gate_Config[Count].Gate[Count2].GateControl[Count3].GateTimeTickMultiplier);
                    printf("%-10s", FWDConfig_t.qci_config.QCI_Gate_Config[Count].Gate[Count2].GateControl[Count3].State ? "Open" : "Closed");
                    printf("%u\n", FWDConfig_t.qci_config.QCI_Gate_Config[Count].Gate[Count2].GateControl[Count3].IPV);
                }
            }
        }
        printf("\n------------------------------  SDUFilters  -----------------------------------");
        printf("\nStreamBlockedDueToOversizeFrame      MSDUFilterNum      MaxSDUSize\n");
        printf("-------------------------------------------------------------------------------\n");
        for(Count = 0; Count < FWDConfig_t.qci_config.MSDU_Filters; Count++)
        {
            printf("%-37s", FWDConfig_t.qci_config.Msdu_Config[Count].msdu_enable ? "True" : "False");
            printf("%-19u", FWDConfig_t.qci_config.Msdu_Config[Count].msdu_id);
            printf("%u\n", FWDConfig_t.qci_config.Msdu_Config[Count].msdu_size);
        }
        printf("\n\n---------------------------------  Filters  -----------------------------------");
        printf("\nFilterID               PCP-ID      VLAN-ID      DEI      GateID      MeterID\n");
        printf("-------------------------------------------------------------------------------\n");
        for(Count = 0; Count < FWDConfig_t.qci_config.Filters; Count++)
        {
            printf("%02x:%02x:%02x:%02x:%02x:%02x", FWDConfig_t.qci_config.Ingress_Filters_Config[Count].Filter_ID[0], FWDConfig_t.qci_config.Ingress_Filters_Config[Count].Filter_ID[1], FWDConfig_t.qci_config.Ingress_Filters_Config[Count].Filter_ID[2], FWDConfig_t.qci_config.Ingress_Filters_Config[Count].Filter_ID[3], FWDConfig_t.qci_config.Ingress_Filters_Config[Count].Filter_ID[4], FWDConfig_t.qci_config.Ingress_Filters_Config[Count].Filter_ID[5]);
            printf("      %-12u", FWDConfig_t.qci_config.Ingress_Filters_Config[Count].Ingress_PCP_Config.PCP_ID);
            printf("%-13u", FWDConfig_t.qci_config.Ingress_Filters_Config[Count].Ingress_VLAN_Config.VLAN_ID);
            printf("%-9u", FWDConfig_t.qci_config.Ingress_Filters_Config[Count].DEI);
            printf("%-12u", FWDConfig_t.qci_config.Ingress_Filters_Config[Count].FilterGate.Gate_ID);
            printf("%u\n", FWDConfig_t.qci_config.Ingress_Filters_Config[Count].Ingress_Filter_Meter.Meter_ID);
        }
        
        printf("\n\n----------------------------------  Meters  -----------------------------------");
        printf("\nMeterID      CBS      CIR      EBS      EIR      DropOnYellow\n");
        printf("-------------------------------------------------------------------------------\n");
        for(Count = 0; Count < FWDConfig_t.qci_config.Meters; Count++)
        {
            printf("%-13u", FWDConfig_t.qci_config.Ingress_Meter_Config[Count].Meter_Id);
            printf("%-9u", FWDConfig_t.qci_config.Ingress_Meter_Config[Count].CBS);
            printf("%-9u", FWDConfig_t.qci_config.Ingress_Meter_Config[Count].CIR);
            printf("%-9u", FWDConfig_t.qci_config.Ingress_Meter_Config[Count].EBS);
            printf("%-9u", FWDConfig_t.qci_config.Ingress_Meter_Config[Count].EIR);
            printf("%s\n", FWDConfig_t.qci_config.Ingress_Meter_Config[Count].Drop_On_Yellow ? "True" : "False");
        }
        
        printf("\n-------------------------------------------------------------------------------\n");
        printf("MarkAllFramesEnable      MarkAllFramesRed      VLAN-ID      DEI      Colour\n");
        printf("-------------------------------------------------------------------------------\n");
        for(Count = 0; Count < FWDConfig_t.qci_config.Meters; Count++)
        {
            printf("%-25s", FWDConfig_t.qci_config.Ingress_Meter_Config[Count].Mark_All_Red_Enable ? "True" : "False");
            printf("%-22s", FWDConfig_t.qci_config.Ingress_Meter_Config[Count].Mark_All_Frames_Red ? "True" : "False");
            printf("%-13u", FWDConfig_t.qci_config.Ingress_Meter_Config[Count].vlan_colour_map[0].PCP_value);
            printf("%-9u", FWDConfig_t.qci_config.Ingress_Meter_Config[Count].vlan_colour_map[0].DEI_value);
            printf("%s\n", FWDConfig_t.qci_config.Ingress_Meter_Config[Count].vlan_colour_map[0].colour_code ? "Red" : "Yellow");
            for(Count2 = 1; Count2 < RENESAS_RSWITCH_MAX_PCP_DEI_CONFIG; Count2++)
            {
                printf("                                               ");
                printf("%-13u", FWDConfig_t.qci_config.Ingress_Meter_Config[Count].vlan_colour_map[Count2].PCP_value);
                printf("%-9u", FWDConfig_t.qci_config.Ingress_Meter_Config[Count].vlan_colour_map[Count2].DEI_value);
                printf("%s\n", FWDConfig_t.qci_config.Ingress_Meter_Config[Count].vlan_colour_map[Count2].colour_code ? "Red" : "Yellow");
            }
        }
    }

    if(FWDConfig_t.Broadcast_Config_Ports != 0)
    {
        printf("\n\n\n---------------------------  Broadcast   --------------------------------------\n");
        printf("CPU-Source      PortNumber      CSDN         Ports\n");
        printf("-------------------------------------------------------------------------------\n");
        for(Count = 0; Count < FWDConfig_t.Broadcast_Config_Ports; Count++)
        {
            printf("%-16s", FWDConfig_t.Broadcast_Config[Count].Broadcast_Config_Port.CPU_Port? "Yes" : "No");
            if(FWDConfig_t.Broadcast_Config[Count].Broadcast_Config_Port.CPU_Port)
            printf("-               ");
            else
            printf("%-16d", FWDConfig_t.Broadcast_Config[Count].Broadcast_Config_Port.PortNumber);  
            printf("%-13d", FWDConfig_t.Broadcast_Config[Count].CSDN);
            if(FWDConfig_t.Broadcast_Config[Count].Dest_Eths)
            printf("%d\n", FWDConfig_t.Broadcast_Config[Count].Dest_Eth[0].PortNumber);
            for(Count2 = 1; Count2 < FWDConfig_t.Broadcast_Config[Count].Dest_Eths; Count2++)
            {
                printf("%46d\n", FWDConfig_t.Broadcast_Config[Count].Dest_Eth[Count2].PortNumber);
            }
            if(FWDConfig_t.Broadcast_Config[Count].CPU_Enable)
            printf("%48s\n", "CPU");
        }
    }

    if(FWDConfig_t.Source_Fwd.bEnable)
    {
        printf("\n\n\n---------------------------  Source-Forwarding  -------------------------------\n");
        //~ //printf("                                 Ports\n");
        printf("PortNumber      CPU-Host      CPU-Mirror      Eth-Mirror      CPU      CSDN      Dest-Ports\n");
        printf("-------------------------------------------------------------------------------\n");
        for(Count = 0; Count < FWDConfig_t.Source_Fwd.source_fwd_port_configs; Count++)
        {
            printf("%-16d", FWDConfig_t.Source_Fwd.Source_Port_Config[Count].PortNumber);
            printf("%-14s", FWDConfig_t.Source_Fwd.Source_Port_Config[Count].CPU_Host ? "Yes" : "No");
            printf("%-16s", FWDConfig_t.Source_Fwd.Source_Port_Config[Count].CPU_Mirror ? "Enable" : "Disable");
            printf("%-16s", FWDConfig_t.Source_Fwd.Source_Port_Config[Count].Eth_Mirror ? "Enable" : "Disable");
            printf("%-9s", FWDConfig_t.Source_Fwd.Source_Port_Config[Count].Dest_Config.CPU_Enable ? "Yes" : "No");
            printf("%-10d", FWDConfig_t.Source_Fwd.Source_Port_Config[Count].Dest_Config.CSDN);
            printf("%d\n", FWDConfig_t.Source_Fwd.Source_Port_Config[Count].Dest_Config.Dest_Eth[0].PortNumber);
            for(Count2 = 1; Count2 < FWDConfig_t.Source_Fwd.Source_Port_Config[Count].Dest_Config.Dest_Eths; Count2++)
            {
                printf("%82d\n", FWDConfig_t.Source_Fwd.Source_Port_Config[Count].Dest_Config.Dest_Eth[Count2].PortNumber);
            }
        }
    }

    if(FWDConfig_t.Dynamic_Auth.bEnable)
    {
        printf("\n\n\n---------------------------  Dynamic-Authentication  --------------------------\n");
        printf("MACe      MAC3      MAC0      Priority      CSDN      CPU      Ports\n");
        printf("-------------------------------------------------------------------------------\n");
        printf("%-10s", FWDConfig_t.Dynamic_Auth.mac_e_enable ? "Enable" : "Disable");
        printf("%-10s", FWDConfig_t.Dynamic_Auth.mac_3_enable ? "Enable" : "Disable");
        printf("%-10s", FWDConfig_t.Dynamic_Auth.mac_0_enable ? "Enable" : "Disable");
        if(FWDConfig_t.Dynamic_Auth.Dest.Priority_Enable)
        printf("%-14d", FWDConfig_t.Dynamic_Auth.Dest.Priority);
        else
        printf("%-14s", "-");
        printf("%-10d", FWDConfig_t.Dynamic_Auth.Dest.CSDN);
        printf("%-9s", FWDConfig_t.Dynamic_Auth.Dest.CPU_Enable ? "Yes" : "No");
        if(NOT FWDConfig_t.Dynamic_Auth.Dest.Dest_Eth[0].PortNumber)
        printf("-\n");
        else
        printf("%d\n", FWDConfig_t.Dynamic_Auth.Dest.Dest_Eth[0].PortNumber);
        for(Count = 1; Count < FWDConfig_t.Dynamic_Auth.Dest.Dest_Eths; Count++)
        {
            printf("%64d\n", FWDConfig_t.Dynamic_Auth.Dest.Dest_Eth[Count].PortNumber);
        }
    }

    if(FWDConfig_t.Dest_Fwd.bEnable)
    {
        printf("\n\n\n---------------------------  Destination-Forwarding ---------------------------\n");
        printf("MAC                 Mask                 Priority      CSDN      CPU      Ports\n");
        printf("-------------------------------------------------------------------------------\n");
        for(Count = 0; Count < FWDConfig_t.Dest_Fwd.DestFwdEntries; Count++)
        {
            printf("%02x:%02x:%02x:%02x:%02x:%02x", FWDConfig_t.Dest_Fwd.DestFwd_Entry[Count].MAC[0], FWDConfig_t.Dest_Fwd.DestFwd_Entry[Count].MAC[1], FWDConfig_t.Dest_Fwd.DestFwd_Entry[Count].MAC[2], FWDConfig_t.Dest_Fwd.DestFwd_Entry[Count].MAC[3], FWDConfig_t.Dest_Fwd.DestFwd_Entry[Count].MAC[4], FWDConfig_t.Dest_Fwd.DestFwd_Entry[Count].MAC[5]);
            printf("   %02x:%02x:%02x:%02x:%02x:%02x", FWDConfig_t.Dest_Fwd.DestFwd_Entry[Count].Mask[0], FWDConfig_t.Dest_Fwd.DestFwd_Entry[Count].Mask[1], FWDConfig_t.Dest_Fwd.DestFwd_Entry[Count].Mask[2], FWDConfig_t.Dest_Fwd.DestFwd_Entry[Count].Mask[3], FWDConfig_t.Dest_Fwd.DestFwd_Entry[Count].Mask[4], FWDConfig_t.Dest_Fwd.DestFwd_Entry[Count].Mask[5]);
            if(FWDConfig_t.Dest_Fwd.DestFwd_Entry[Count].Destination.Priority_Enable)
            printf("    %-14d", FWDConfig_t.Dest_Fwd.DestFwd_Entry[Count].Destination.Priority);
            else
            printf("    %-14s", "-");
            printf("%-10d", FWDConfig_t.Dest_Fwd.DestFwd_Entry[Count].Destination.CSDN);
            printf("%-9s", FWDConfig_t.Dest_Fwd.DestFwd_Entry[Count].Destination.CPU_Enable ? "Yes" : "No");
            if(NOT FWDConfig_t.Dest_Fwd.DestFwd_Entry[Count].Destination.Dest_Eth[0].PortNumber)
            printf("-\n");
            else
            printf("%d\n", FWDConfig_t.Dest_Fwd.DestFwd_Entry[Count].Destination.Dest_Eth[0].PortNumber);
            for(Count2 = 1; Count2 < FWDConfig_t.Dest_Fwd.DestFwd_Entry[Count].Destination.Dest_Eths; Count2++)
            printf("%75d\n", FWDConfig_t.Dest_Fwd.DestFwd_Entry[Count].Destination.Dest_Eth[Count2].PortNumber);
        }
    }

    if(FWDConfig_t.BPDU_Fwd.bEnable)
    {
        printf("\n\n\n---------------------------  BPDU-Forwarding ----------------------------------\n");
        printf("MAC                 Mask                 Priority      CSDN      CPU      Ports\n");
        printf("-------------------------------------------------------------------------------\n");
        
        printf("%02x:%02x:%02x:%02x:%02x:%02x", FWDConfig_t.BPDU_Fwd.MAC[0], FWDConfig_t.BPDU_Fwd.MAC[1], FWDConfig_t.BPDU_Fwd.MAC[2], FWDConfig_t.BPDU_Fwd.MAC[3], FWDConfig_t.BPDU_Fwd.MAC[4], FWDConfig_t.BPDU_Fwd.MAC[5]);
        printf("   %02x:%02x:%02x:%02x:%02x:%02x", FWDConfig_t.BPDU_Fwd.Mask[0], FWDConfig_t.BPDU_Fwd.Mask[1], FWDConfig_t.BPDU_Fwd.Mask[2], FWDConfig_t.BPDU_Fwd.Mask[3], FWDConfig_t.BPDU_Fwd.Mask[4], FWDConfig_t.BPDU_Fwd.Mask[5]);
        if(FWDConfig_t.BPDU_Fwd.BPDU_Dest.Priority_Enable)
        printf("    %-14d", FWDConfig_t.BPDU_Fwd.BPDU_Dest.Priority);
        else
        printf("    %-14s", "-");
        printf("%-10d", FWDConfig_t.BPDU_Fwd.BPDU_Dest.CSDN);
        printf("%-9s", FWDConfig_t.BPDU_Fwd.BPDU_Dest.CPU_Enable ? "Yes" : "No");
        if(NOT FWDConfig_t.BPDU_Fwd.BPDU_Dest.Dest_Eth[0].PortNumber)
        printf("-\n");
        else
        printf("%d\n", FWDConfig_t.BPDU_Fwd.BPDU_Dest.Dest_Eth[0].PortNumber);
        for(Count2 = 1; Count2 < FWDConfig_t.BPDU_Fwd.BPDU_Dest.Dest_Eths; Count2++)
        printf("%75d\n", FWDConfig_t.BPDU_Fwd.BPDU_Dest.Dest_Eth[Count2].PortNumber);
    }

    if(FWDConfig_t.Priv_VLAN.bEnable)
    {
        printf("\n\n\n---------------------------  Private-VLAN  ------------------------------------\n");
        printf("\t\t\tSource-Independent = %s\n", FWDConfig_t.Priv_VLAN.source_independent ? "Yes" : "No");
        printf("Group-ID      CPU      Ports\n");
        printf("-------------------------------------------------------------------------------\n");
        for(Count = 0; Count < FWDConfig_t.Priv_VLAN.VLAN_Groups; Count++)
        {
            printf("%-14d", FWDConfig_t.Priv_VLAN.vlan_group[Count].GrpID);
            printf("%-9s", FWDConfig_t.Priv_VLAN.vlan_group[Count].CPU_Enable ? "Yes" : "No");
            if(NOT FWDConfig_t.Priv_VLAN.vlan_group[Count].Dest_Eth[0].PortNumber)
            printf("-\n");
            else
            printf("%d\n", FWDConfig_t.Priv_VLAN.vlan_group[Count].Dest_Eth[0].PortNumber);
            for(Count2 = 1; Count2 < FWDConfig_t.Priv_VLAN.vlan_group[Count].Dest_Eths; Count2++)
            printf("%24d\n", FWDConfig_t.Priv_VLAN.vlan_group[Count].Dest_Eth[Count2].PortNumber);
        }
    }

    if(FWDConfig_t.Double_Tag.bEnable)
    {
        printf("\n\n\n---------------------------  Double-Tag  --------------------------------------\n");
        printf("Double-Tag = %d\n", FWDConfig_t.Double_Tag.tag_value);
    }

    if(FWDConfig_t.AgentFilterCSDPorts.Eths OR FWDConfig_t.AgentFilterCSDPorts.CPU_Enable)
    {
        printf("\n\n\n---------------------------  Agent-Filter-CSD-Ports  --------------------------\n");
        printf("CPU      PortNumber\n");
        printf("-------------------------------------------------------------------------------\n");
        printf("%-9s", FWDConfig_t.AgentFilterCSDPorts.CPU_Enable ? "Yes" : "No");
        if(NOT FWDConfig_t.AgentFilterCSDPorts.Eth[0].PortNumber)
        printf("-\n");
        else
        printf("%d\n", FWDConfig_t.AgentFilterCSDPorts.Eth[0].PortNumber);
        for(Count = 1; Count < FWDConfig_t.AgentFilterCSDPorts.Eths; Count++)
        printf("%10d\n", FWDConfig_t.AgentFilterCSDPorts.Eth[Count].PortNumber);
    }

    if(FWDConfig_t.WaterMarkControl.bEnable)
    {
        printf("\n\n\n---------------------------  WaterMarkControl  --------------------------------\n");
        printf("Flush-Level    Critical-Level    Remaining-Pause-    Remaining-Pause-    PortNumber   CPU   NoVLAN-Flush   NoVLAN-CLevel   PCP-ID   DEI   Flush    CLevel\n");
        printf("                                 Frame-Assert        Frame-DAssert\n");
        printf("-------------------------------------------------------------------------------\n");
        printf("%-15u", FWDConfig_t.WaterMarkControl.flush_level);
        printf("%-18u", FWDConfig_t.WaterMarkControl.critical_level);
        printf("%-20u", FWDConfig_t.WaterMarkControl.rmn_pause_frm_assrt);
        printf("%-20u", FWDConfig_t.WaterMarkControl.rmn_pause_frm_dassrt);
        if(NOT FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[0].PortNumber)
        printf("%-13s", "-");
        else
        printf("%-13d", FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[0].PortNumber);
        printf("%-6s", FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[0].CPU_Enable ? "Yes" : "No");
        printf("%-15s", FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[0].novlanflush ? "Accept" : "Discard");
        printf("%-16s", FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[0].novlanclevel ? "Accept" : "Discard");
        printf("%-9d", FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[0].pcp_dei_config[0].PCP_ID);
        printf("%-6d", FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[0].pcp_dei_config[0].DEI);
        printf("%-9s", FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[0].pcp_dei_config[0].flush ? "Accept" : "Discard");
        printf("%s\n", FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[0].pcp_dei_config[0].clevel ? "Accept" : "Discard");
        for(Count3 = 1; Count3 < FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[0].PCP_DEI_Configs; Count3++)
        {
            printf("%124d        ", FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[0].pcp_dei_config[Count3].PCP_ID);
            printf("%-6d", FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[0].pcp_dei_config[Count3].DEI);
            printf("%-9s", FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[0].pcp_dei_config[Count3].flush ? "Accept" : "Discard");
            printf("%s\n", FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[0].pcp_dei_config[Count3].clevel ? "Accept" : "Discard");
        }
        for(Count = 1; Count < FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfigs; Count++)
        {
            printf("%-74d", FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[Count].PortNumber);
            printf("%-6d", FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[0].CPU_Enable ? "Yes" : "No");
            printf("%-15s", FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[Count].novlanflush ? "Accept" : "Discard");
            printf("%-16s", FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[Count].novlanclevel ? "Accept" : "Discard");
            printf("%-9d", FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[Count].pcp_dei_config[0].PCP_ID);
            printf("%-6d", FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[Count].pcp_dei_config[0].DEI);
            printf("%-9s", FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[Count].pcp_dei_config[0].flush ? "Accept" : "Discard");
            printf("%s\n", FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[Count].pcp_dei_config[0].clevel ? "Accept" : "Discard");
            for(Count2 = 1; Count2 < FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[Count].PCP_DEI_Configs; Count2++)
            {
                printf("%124d        ", FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[Count].pcp_dei_config[Count2].PCP_ID);
                printf("%-6d", FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[Count].pcp_dei_config[Count2].DEI);
                printf("%-9s", FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[Count].pcp_dei_config[Count2].flush ? "Accept" : "Discard");
                printf("%s\n", FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[Count].pcp_dei_config[Count2].clevel ? "Accept" : "Discard");
            }
            printf("\n");
        }
    }

    if(FWDConfig_t.ExceptionalPath.bEnable)
    {
        printf("\n\n---------------------------  Exceptional-Path  --------------------------------\n");
        printf("AuthFailtoCPU      BPDUFailtoCPU      WMarktoCPU      QCIrejecttoCPU      LearnstatictoCPU      UnmatchSIDtoCPU      UnmatchVLANtoCPU     UnmatchMACtoCPU      NullDPVtoCPU      ErrorDesctoCPU      FilterRejecttoCPU      SPLViolationtoCPU      Priority      CSDN\n");
        printf("-------------------------------------------------------------------------------\n");
        printf("%-19s", FWDConfig_t.ExceptionalPath.AuthFailtoCPU ? "Enable" : "Disable");
        printf("%-19s", FWDConfig_t.ExceptionalPath.BPDUFailtoCPU ? "Enable" : "Disable");
        printf("%-16s", FWDConfig_t.ExceptionalPath.WMarktoCPU ? "Enable" : "Disable");
        printf("%-20s", FWDConfig_t.ExceptionalPath.QCIrejecttoCPU ? "Enable" : "Disable");
        printf("%-22s", FWDConfig_t.ExceptionalPath.LearnstatictoCPU ? "Enable" : "Disable");
        printf("%-21s", FWDConfig_t.ExceptionalPath.UnmatchSIDtoCPU ? "Enable" : "Disable");
        printf("%-21s", FWDConfig_t.ExceptionalPath.UnmatchVLANtoCPU ? "Enable" : "Disable");
        printf("%-21s", FWDConfig_t.ExceptionalPath.UnmatchMACtoCPU ? "Enable" : "Disable");
        printf("%-18s", FWDConfig_t.ExceptionalPath.NullDPVtoCPU ? "Enable" : "Disable");
        printf("%-20s", FWDConfig_t.ExceptionalPath.ErrorDesctoCPU ? "Enable" : "Disable");
        printf("%-23s", FWDConfig_t.ExceptionalPath.FilterRejecttoCPU ? "Enable" : "Disable");
        printf("%-23s", FWDConfig_t.ExceptionalPath.SPLViolationtoCPU ? "Enable" : "Disable");
        printf("%-14d", FWDConfig_t.ExceptionalPath.Priority);
        printf("%d\n", FWDConfig_t.ExceptionalPath.CSDN);
    }

    if(FWDConfig_t.Learning.bEnable)
    {
        printf("\n\n\n---------------------------  Learning  ----------------------------------------\n");
        printf("UnknownSourcePortLearn     HW-Learning      VLAN-Learn      MAC-Learn           Priority      CSDN\n");
        printf("-------------------------------------------------------------------------------\n");
        printf("%-27s", FWDConfig_t.Learning.UnknownSourcePortLearn ? "Enable" : "Disable");
        printf("%-17s", FWDConfig_t.Learning.HWLearning ? "Enable" : "Disable");
        printf("%-16s", FWDConfig_t.Learning.VLANLearning ? "Enable" : "Disable");
        switch(FWDConfig_t.Learning.MACLearning)
        {
        case rswitch_fwd_MAC_learn_Inactive :
            printf("Inactive          ");
            break;
        case rswitch_fwd_MAC_learn_ActiveSucceedFailed :
            printf("ActiveSucceedFailed ");
            break;
        case rswitch_fwd_MAC_learn_ActiveSucceed :
            printf("ActiveSucceed       ");
            break;
        case rswitch_fwd_MAC_learn_ActiveFailed :
            printf("ActiveFailed        ");
            break;
        }
        printf("%-14d", FWDConfig_t.Learning.Priority);
        printf("%d\n", FWDConfig_t.Learning.CSDN);
    }

    if(FWDConfig_t.Mirroring.bEnable)
    {
        printf("\n\n\n---------------------------  Mirroring  ---------------------------------------\n");
        printf("ForwardSourceDest      Error-Mirror      SDPriority-Type      SDPriority      EthPriority-Type      EthPriority      CPUPriority-Type      CPUPriority      CSDN\n");
        printf("-------------------------------------------------------------------------------\n");
        printf("%-23s", FWDConfig_t.Mirroring.ForwardSourceDest ? "SourceAndDest" : "SourceOrDest");
        printf("%-18s", FWDConfig_t.Mirroring.Error_Mirror ? "Enable" : "Disable");
        printf("%-21s", FWDConfig_t.Mirroring.SD_Mirror_Priority_Type ? "MirrorCtrl" : "Desc");
        printf("%-16d", FWDConfig_t.Mirroring.SD_Mirror_Priority_Value);
        printf("%-22s", FWDConfig_t.Mirroring.Eth_Mirror_Priority_Type ? "MirrorCtrl" : "Desc");
        printf("%-17d", FWDConfig_t.Mirroring.Eth_Mirror_Priority_Value);
        printf("%-22s", FWDConfig_t.Mirroring.CPU_Mirror_Priority_Type ? "MirrorCtrl" : "Desc");
        printf("%-17d", FWDConfig_t.Mirroring.CPU_Mirror_Priority_Value);
        printf("%d\n", FWDConfig_t.Mirroring.CSDN);
        printf("\n-------------------------------------------------------------------------------\n");
        printf("SourceMirrorPorts      CPU      DestMirrorPorts      CPU      EthMirrorDest      CPU      SDMirrorDest      CPU\n");
        printf("-------------------------------------------------------------------------------\n");
        if(NOT FWDConfig_t.Mirroring.Source_PortConfig.Port[0].PortNumber)
        printf("%-23s", "-");
        else
        printf("%-23d", FWDConfig_t.Mirroring.Source_PortConfig.Port[0].PortNumber);
        printf("%-9s", FWDConfig_t.Mirroring.Source_PortConfig.CPU ? "Yes" : "No");
        if(NOT FWDConfig_t.Mirroring.Dest_PortConfig.Port[0].PortNumber)
        printf("%-21s", "-");
        else
        printf("%-21d", FWDConfig_t.Mirroring.Dest_PortConfig.Port[0].PortNumber);
        printf("%-9s", FWDConfig_t.Mirroring.Dest_PortConfig.CPU ? "Yes" : "No");
        if(NOT FWDConfig_t.Mirroring.DestEThMirrorPort.Port[0].PortNumber)
        printf("%-19s", "-");
        else
        printf("%-19d", FWDConfig_t.Mirroring.DestEThMirrorPort.Port[0].PortNumber);
        printf("%-9s", FWDConfig_t.Mirroring.DestEThMirrorPort.CPU ? "Yes" : "No");
        if(NOT FWDConfig_t.Mirroring.DestCPUMirrorPort.Port[0].PortNumber)
        printf("%-18s", "-");
        else
        printf("%-18d", FWDConfig_t.Mirroring.DestCPUMirrorPort.Port[0].PortNumber);
        printf("%s\n", FWDConfig_t.Mirroring.DestCPUMirrorPort.CPU ? "Yes" : "No");
        int nl = 0;
        for(Count = 1; Count < RENESAS_RSWITCH_MAX_ETHERNET_PORTS; Count++)
        {
            if(FWDConfig_t.Mirroring.DestCPUMirrorPort.Ports > Count)
            {
                printf("%91d", FWDConfig_t.Mirroring.DestCPUMirrorPort.Port[Count].PortNumber);
                nl = 1;
            }
            if(FWDConfig_t.Mirroring.DestEThMirrorPort.Ports > Count)
            {
                printf("\r%63d", FWDConfig_t.Mirroring.DestEThMirrorPort.Port[Count].PortNumber);
                nl = 1;
            }
            if(FWDConfig_t.Mirroring.Dest_PortConfig.Ports > Count)
            {
                printf("\r%33d", FWDConfig_t.Mirroring.Dest_PortConfig.Port[Count].PortNumber);
                nl = 1;
            }
            if(FWDConfig_t.Mirroring.Source_PortConfig.Ports > Count)
            {
                printf("\r%d", FWDConfig_t.Mirroring.Source_PortConfig.Port[Count].PortNumber);
                nl = 1;
            }
            if(nl)
            printf("\n");
            nl = 0;
        }
    }

    if(FWDConfig_t.Port_Lock.bEnable)
    {
        printf("\n\n\n---------------------------  Port-Lock  ---------------------------------------\n");
        printf("CPU      PortNumber\n");
        printf("-------------------------------------------------------------------------------\n");
        printf("%-9s", FWDConfig_t.Port_Lock.LockPort.CPU ? "Yes" : "No");
        if(NOT FWDConfig_t.Port_Lock.LockPort.Port[0].PortNumber)
        printf("-\n");
        else
        printf("%d\n", FWDConfig_t.Port_Lock.LockPort.Port[0].PortNumber);
        for(Count = 1; Count < FWDConfig_t.Port_Lock.LockPort.Ports; Count++)
        printf("%10d\n", FWDConfig_t.Port_Lock.LockPort.Port[Count].PortNumber);
    }

    if(FWDConfig_t.Spanning_Tree.Ports)
    {
        printf("\n\n\n---------------------------  SpanningTreeProtocol  ----------------------------\n");
        printf("PortNumber      PortState\n");
        printf("-------------------------------------------------------------------------------\n");
        for(Count = 0; Count < FWDConfig_t.Spanning_Tree.Ports; Count++)
        {
            if(FWDConfig_t.Spanning_Tree.SPT_Port_Config[Count].CPU)
            printf("%-16s", "CPU");
            else
            printf("%-16d", FWDConfig_t.Spanning_Tree.SPT_Port_Config[Count].PortNumber);
            switch(FWDConfig_t.Spanning_Tree.SPT_Port_Config[Count].STP_State)
            {
            case rswitch_Config_SPT_State_Disabled :
                printf("Disabled");
                break;
            case rswitch_Config_SPT_State_Blocked :
                printf("Blocking");
                break;
            case rswitch_Config_SPT_State_Learning :
                printf("Learn");
                break;
            case rswitch_Config_SPT_State_Forwarding :
                printf("Forward");
                break;
            case rswitch_Config_SPT_State_LearnandFrwrd :
                printf("LearnandForward");
                break;
            }
            printf("\n");
        }
    }

    if(FWDConfig_t.Migration.bEnable)
    {
        printf("\n\n\n---------------------------  Migration  ---------------------------------------\n");
        printf("CPU      PortNumber\n");
        printf("-------------------------------------------------------------------------------\n");
        printf("%-9s", FWDConfig_t.Migration.Migration_Port.CPU ? "Yes" : "No");
        if(NOT FWDConfig_t.Migration.Migration_Port.Port[0].PortNumber)
        printf("-\n");
        else
        printf("%d\n", FWDConfig_t.Migration.Migration_Port.Port[0].PortNumber);
        for(Count = 1; Count < FWDConfig_t.Migration.Migration_Port.Ports; Count++)
        printf("%10d\n", FWDConfig_t.Migration.Migration_Port.Port[Count].PortNumber);
    }

    if(FWDConfig_t.Static_Auth.bEnable)
    {
        printf("\n\n\n---------------------------  Static-Authentication  ---------------------------\n");
        printf("CPU      PortNumber\n");
        printf("-------------------------------------------------------------------------------\n");
        printf("%-9s", FWDConfig_t.Static_Auth.Static_Auth_Port.CPU ? "Yes" : "No");
        if(FWDConfig_t.Static_Auth.Static_Auth_Port.Ports)
        printf("%d\n", FWDConfig_t.Static_Auth.Static_Auth_Port.Port[0].PortNumber);
        else
        printf("-\n");
        for(Count = 1; Count < FWDConfig_t.Static_Auth.Static_Auth_Port.Ports; Count++)
        printf("%10d\n", FWDConfig_t.Static_Auth.Static_Auth_Port.Port[Count].PortNumber);
    }

    if(FWDConfig_t.Pvt_VLAN_Settings.Ports)
    {
        printf("\n\n\n---------------------------  Private-VLAN-Setting  ----------------------------\n");
        printf("CPU      PortNumber      Type         Community\n");
        printf("-------------------------------------------------------------------------------\n");
        for(Count = 0; Count < FWDConfig_t.Pvt_VLAN_Settings.Ports; Count++)
        {
            printf("%-9s", FWDConfig_t.Pvt_VLAN_Settings.Port[Count].CPU ? "Yes" : "No");
            if(FWDConfig_t.Pvt_VLAN_Settings.Port[Count].CPU)
            printf("%-16s", "-");
            else
            printf("%-16d", FWDConfig_t.Pvt_VLAN_Settings.Port[Count].PortNumber);
            switch (FWDConfig_t.Pvt_VLAN_Settings.Port[Count].Type)
            {
            case rswitch_Config_fwd_Pvt_VLAN_Isolated :
                printf("Isolated     -\n");
                break;
            case rswitch_Config_fwd_Pvt_VLAN_Promiscous :
                printf("Promiscous   -\n");
                break;
            case rswitch_Config_fwd_Pvt_VLAN_Community :
                printf("Community    ");
                printf("%d\n", FWDConfig_t.Pvt_VLAN_Settings.Port[Count].Pvt_VLAN_Community[0].Community_ID);
                for(Count2 = 1; Count2 < FWDConfig_t.Pvt_VLAN_Settings.Port[Count].Pvt_VLAN_Communities; Count2++)
                printf("%30d\n", FWDConfig_t.Pvt_VLAN_Settings.Port[Count].Pvt_VLAN_Community[Count2].Community_ID);
                break;
            }
        }
    }

    if(FWDConfig_t.Search_Config.bEnable)
    {
        printf("\n\n\n---------------------------  FWD-Search-Config  -------------------------------\n");
        printf("QueuePriority      QCI-MAC      QCI-VLAN      QCI-PCP      QCI-DEI      QCI-SPN      QCI-MAC-Select      QCI-Filtering      Stream-ID-Unique-Number-Select      Stream-ID-MAC-Select      Stream-ID-Filter-Select      Stream-ID-Filter\n");
        printf("-------------------------------------------------------------------------------\n");
        printf("%-19s", FWDConfig_t.Search_Config.Queue_Priority ? "PCP" : "IPV");
        printf("%-13s", FWDConfig_t.Search_Config.QCIMAC ? "Include" : "Exclude");
        printf("%-14s", FWDConfig_t.Search_Config.QCIVLAN ? "Include" : "Exclude");
        printf("%-13s", FWDConfig_t.Search_Config.QCIPCP ? "Include" : "Exclude");
        printf("%-13s", FWDConfig_t.Search_Config.QCIDEI ? "Include" : "Exclude");
        printf("%-13s", FWDConfig_t.Search_Config.QCISPN ? "Include" : "Exclude");
        printf("%-20s", FWDConfig_t.Search_Config.QCI_MAC_Select ? "Dest" : "Source");
        printf("%-19s", FWDConfig_t.Search_Config.QCI_Filtering ? "Enable" : "Disable");
        printf("%-36s", FWDConfig_t.Search_Config.SID_UniqueNum_Select ? "MAC" : "VLAN");
        printf("%-26s", FWDConfig_t.Search_Config.SID_MAC_Select ? "Dest" : "Source");
        printf("%-29s", FWDConfig_t.Search_Config.SID_Filter_Select ? "Black" : "White");
        printf("%s", FWDConfig_t.Search_Config.SID_Filter ? "Active" : "Inactive");
        printf("\n\n-------------------------------------------------------------------------------\n");
        printf("Stream-ID-Tbl      SrcPortFilter      MAC-Filter-Select      MAC-Filter      MAC-Tbl      VLAN-MAC-Priority     VLAN-MAC-Fwding      VLAN-MAC-CSDN     VLAN-Filter-Select      VLAN-Filter      VLAN-Tbl\n");
        printf("-------------------------------------------------------------------------------\n");
        printf("%-19s", FWDConfig_t.Search_Config.SID_Tbl ? "Active" : "Inactive");
        printf("%-19s", FWDConfig_t.Search_Config.Src_Port_Filter ? "Active" : "Inactive");
        printf("%-23s", FWDConfig_t.Search_Config.MAC_Filter_Select ? "Black" : "White");
        printf("%-16s", FWDConfig_t.Search_Config.MAC_Filter ? "Active" : "Inactive");
        printf("%-13s", FWDConfig_t.Search_Config.MAC_Tbl ? "Active" : "Inactive");
        switch(FWDConfig_t.Search_Config.MAC_VLAN_Priority)
        {
        case rswitch_Config_Priority_MAC :
            printf("MAC                   ");
            break;
        case rswitch_Config_Priority_VLAN :
            printf("VLAN                  ");
            break;
        case rswitch_Config_Priority_Highest :
            printf("Highest               ");
            break;
        case rswitch_Config_Priority_Lowest :
            printf("Lowest                ");
            break;
        }
        switch(FWDConfig_t.Search_Config.VLAN_MAC_Fwd)
        {
        case rswitch_Fwd_VLANorMAC :
            printf("VLAN_OR_MAC          ");
            break;
        case rswitch_Fwd_VLANandMAC :
            printf("VLAN_AND_MAC         ");
            break;
        case rswitch_Fwd_VLAN :
            printf("VLAN                 ");
            break;
        case rswitch_Fwd_MAC :
            printf("MAC                  ");
            break;
        }
        printf("%-18s", FWDConfig_t.Search_Config.VLAN_MAC_CSDN ? "MAC" : "VLAN");
        printf("%-24s", FWDConfig_t.Search_Config.VLAN_Filter_Select ? "Black" : "White");
        printf("%-17s", FWDConfig_t.Search_Config.VLAN_Filter ? "Active" : "Inactive");
        printf("%s\n", FWDConfig_t.Search_Config.VLAN_Tbl ? "Active" : "Inactive");
    }

    if(FWDConfig_t.Insert_L2_Config.L2FwdEntries)
    {
        printf("\n\n\n---------------------------  Insert-MAC-entries  ------------------------------\n");
        printf("MAC                  CPU      Dynamic      EthMirror      CPUMirror      PCPUpdate      ListFilter      PCP      CSDN      PortNumbers\n");
        printf("-------------------------------------------------------------------------------\n");
        for(Count = 0; Count < FWDConfig_t.Insert_L2_Config.L2FwdEntries; Count++)
        {
            printf("%02x:%02x:%02x:%02x:%02x:%02x", FWDConfig_t.Insert_L2_Config.L2Fwd[Count].MAC[0], FWDConfig_t.Insert_L2_Config.L2Fwd[Count].MAC[1], FWDConfig_t.Insert_L2_Config.L2Fwd[Count].MAC[2], FWDConfig_t.Insert_L2_Config.L2Fwd[Count].MAC[3], FWDConfig_t.Insert_L2_Config.L2Fwd[Count].MAC[4], FWDConfig_t.Insert_L2_Config.L2Fwd[Count].MAC[5]);
            printf("    %-9s", FWDConfig_t.Insert_L2_Config.L2Fwd[Count].Destination.CPU_Enable ? "Yes" : "No");
            printf("%-13s", FWDConfig_t.Insert_L2_Config.L2Fwd[Count].Destination.Dynamic ? "Enable" : "Disable");
            printf("%-15s", FWDConfig_t.Insert_L2_Config.L2Fwd[Count].Destination.EthMirror ? "Enable" : "Disable");
            printf("%-15s", FWDConfig_t.Insert_L2_Config.L2Fwd[Count].Destination.CPUMirror ? "Enable" : "Disable");
            printf("%-15s", FWDConfig_t.Insert_L2_Config.L2Fwd[Count].Destination.PCPUpdate ? "Enable" : "Disable");
            printf("%-16s", FWDConfig_t.Insert_L2_Config.L2Fwd[Count].Destination.ListFilter ? "Enable" : "Disable");
            printf("%-9d", FWDConfig_t.Insert_L2_Config.L2Fwd[Count].Destination.PCP);
            printf("%-10d", FWDConfig_t.Insert_L2_Config.L2Fwd[Count].Destination.CSDN);
            if(FWDConfig_t.Insert_L2_Config.L2Fwd[Count].Destination.DestEths)
            printf("%d\n", FWDConfig_t.Insert_L2_Config.L2Fwd[Count].Destination.DestEth[0].PortNumber);
            else
            printf("-\n");
            for(Count2 = 1; Count2 < FWDConfig_t.Insert_L2_Config.L2Fwd[Count].Destination.DestEths; Count2++)
            printf("%124d\n", FWDConfig_t.Insert_L2_Config.L2Fwd[Count].Destination.DestEth[Count2].PortNumber);
        }
    }

    if(FWDConfig_t.Insert_VLAN_Config.VLANFwdEntries)
    {
        printf("\n\n\n---------------------------  Insert-VLAN-entries  -----------------------------\n");
        printf("VLAN      EthMirror      CPUMirror      PCPUpdate      PCP      CSDN      VLAN-Filter-Ports      CPU      VLAN-Routing-Ports      CPU\n");
        printf("-------------------------------------------------------------------------------\n");
        for(Count = 0; Count < FWDConfig_t.Insert_VLAN_Config.VLANFwdEntries; Count++)
        {
            printf("%-10d", FWDConfig_t.Insert_VLAN_Config.VLANFwd[Count].VLAN_ID);
            printf("%-15s", FWDConfig_t.Insert_VLAN_Config.VLANFwd[Count].EthMirror ? "Enable" : "Disable");
            printf("%-15s", FWDConfig_t.Insert_VLAN_Config.VLANFwd[Count].CPUMirror ? "Enable" : "Disable");
            printf("%-15s", FWDConfig_t.Insert_VLAN_Config.VLANFwd[Count].PCPUpdate ? "Enable" : "Disable");
            printf("%-9d", FWDConfig_t.Insert_VLAN_Config.VLANFwd[Count].PCP);
            printf("%-10d", FWDConfig_t.Insert_VLAN_Config.VLANFwd[Count].CSDN);
            if(NOT FWDConfig_t.Insert_VLAN_Config.VLANFwd[Count].VLAN_Filter.Port[0].PortNumber)
            printf("%-23s", "-");
            else
            printf("%-23d", FWDConfig_t.Insert_VLAN_Config.VLANFwd[Count].VLAN_Filter.Port[0].PortNumber);
            printf("%-9s", FWDConfig_t.Insert_VLAN_Config.VLANFwd[Count].VLAN_Filter.CPU_Enable ? "Yes" : "No");
            if(NOT FWDConfig_t.Insert_VLAN_Config.VLANFwd[Count].VLAN_Routing.Port[0].PortNumber)
            printf("%-24s", "-");
            else
            printf("%-24d", FWDConfig_t.Insert_VLAN_Config.VLANFwd[Count].VLAN_Routing.Port[0].PortNumber);
            printf("%s\n", FWDConfig_t.Insert_VLAN_Config.VLANFwd[Count].VLAN_Routing.CPU_Enable ? "Yes" : "No");
            for(Count2 = 1; Count2 < RENESAS_RSWITCH_MAX_ETHERNET_PORTS; Count2++)
            {
                if(Count2 < FWDConfig_t.Insert_VLAN_Config.VLANFwd[Count].VLAN_Routing.Ports)
                printf("\r%107d", FWDConfig_t.Insert_VLAN_Config.VLANFwd[Count].VLAN_Routing.Port[Count2].PortNumber);
                if(Count2 < FWDConfig_t.Insert_VLAN_Config.VLANFwd[Count].VLAN_Filter.Ports)
                printf("\r%75d\n", FWDConfig_t.Insert_VLAN_Config.VLANFwd[Count].VLAN_Filter.Port[Count2].PortNumber);
                if((FWDConfig_t.Insert_VLAN_Config.VLANFwd[Count].VLAN_Routing.Ports >= Count2) && (FWDConfig_t.Insert_VLAN_Config.VLANFwd[Count].VLAN_Filter.Ports < Count2))
                printf("\n");
            }
        }
    }   
}

extern bool FWD_Configure_Device(void)
{
    int ret = 0;
    if (gFwdFd != -1)
    {
        if ((ret = ioctl(gFwdFd, RSWITCH_FWD_SET_CONFIG, &FWDConfig_t)) != 0)   
        {
            fprintf(stderr, "\nERROR : RSWITCH_FWD_SET_CONFIG failed (%d) : %s\n", ret, strerror(errno));
            return FALSE;
        }
    }
    else
    {
        fprintf(stderr, "WARNING, Omitting RSWITCH_FWD_SET_CONFIG as module not open\n");
    }

    return TRUE;
}

extern bool FWD_Report_Device(void)
{
    int ret = 0;
    if (gFwdFd != -1)
    {
        if ((ret = ioctl(gFwdFd, RSWITCH_FWD_GET_CONFIG, &FWDConfig_t)) != 0)   
        {
            fprintf(stderr, "\nERROR : RSWITCH_FWD_GET_CONFIG failed (%d) : %s\n", ret, strerror(errno));
            return FALSE;
        }
        FWD_Print_Configuration();
    }
    else
    {
        fprintf(stderr, "WARNING, Omitting RSWITCH_FWD_GET_CONFIG as module not open\n");
    }
    return TRUE;
}

static bool Is_Any_Duplicate_MAC(char MACAddr[][18], int MACSize)
{   
    char temp[18];
    int j, i;
    
    //~ // Sorting strings using bubble sort
    for (j = 0; j < MACSize - 1; j++)                                                                                                      
    {   
        for (i = j + 1; i < MACSize; i++)
        {   
            if (strcmp(MACAddr[j], MACAddr[i]) > 0)
            {   
                strcpy(temp, MACAddr[j]);
                strcpy(MACAddr[j], MACAddr[i]);
                strcpy(MACAddr[i], temp);
            }
        }
    }
    for (i = 0; i < MACSize; i++)
    { 
        if (strcmp(MACAddr[i], MACAddr[i+1]) == 0)
        {   
            fprintf(stderr, "\nDuplicate MAC Address with MAC : %s\n", MACAddr[i]);
            return FALSE;
        }
    }
    return TRUE;
}



static bool SetConfig_PortNumber(mxml_node_t *PortNumber, uint32_t *structure, uint32_t PortCount)
{
    uint32_t PortInt = 0;
    mxml_node_t *value = NULL;
    unsigned char  * PortChar[RENESAS_RSWITCH_MAX_ETHERNET_PORTS];
    
    
    if (strcasecmp("PortNumber", PortNumber->value.element.name) == 0)
    {
        if ((value = mxmlGetFirstChild(PortNumber)) == NULL)
        {
            fprintf(stderr, "\n No <PortNumber>");
            return FALSE;
        }
        if (value->type != MXML_TEXT)
        {
            fprintf(stderr, "\n Invalid  <PortNumber>");
            return FALSE;

        }

        PortChar[PortCount] = value->value.text.string;

        if (sscanf(PortChar[PortCount], "%u", &PortInt) != 1)
        {
            fprintf(stderr, "\n Invalid  <PortNumber>");
            return FALSE;
        }

        if (PortInt >= RENESAS_RSWITCH_MAX_ETHERNET_PORTS)
        {
            fprintf(stderr, "\nInvalid  <PortNumber>, Range (0-%d)\n", (RENESAS_RSWITCH_MAX_ETHERNET_PORTS-1));
            return FALSE;
        }
        
        *structure = PortInt;
    }
    return TRUE;
}


static bool SetConfig_Integer(mxml_node_t * ParentNode, char const * const Tag, uint32_t *structure, uint32_t ulimit)
{
    uint64_t Data = 0;
    char const * ch = NULL;
    if ((ch = SetConfig_GetText(ParentNode, ParentNode, Tag)) != NULL)
    {
        if (sscanf(ch, "%"PRIu64, &Data) != 1)
        {
            fprintf(stderr, "\nInvalid <%s>\n", Tag);
            return FALSE;
        }
        if(Data > ulimit)
        {
            printf("%s = %"PRIu64"\n", Tag, Data);
            fprintf(stderr, "Invalid <%s> - cannot be larger than %lu", Tag, ulimit);
            return FALSE;
        }
        *structure = Data;
        return TRUE;
    }
    return FALSE;
}


static bool SetConfig_BinaryText(mxml_node_t * ParentNode, char const * const Tag, uint32_t *structure, 
char const * const s1, char const * const s2, uint32_t l1, uint32_t l2)
{
    char const * ch = NULL;
    if((ch = SetConfig_GetText(ParentNode, ParentNode, Tag)) != NULL)
    {
        if(strncasecmp(ch, s1, l1) == 0)
        {
            
            *structure = TRUE;
            return TRUE;
        }
        else if (strncasecmp(ch, s2, l2) == 0)
        {
            
            *structure = FALSE;
            return TRUE;
        }
        else
        {
            fprintf(stderr, "\nInvalid <%s> - can only be \"%s\" or \"%s\"\n", 
            Tag, s1, s2);
            return FALSE;
        }
    }
    return FALSE;
}


static bool SetConfig_MAC(mxml_node_t * ParentNode, uint8_t structure[], const char * const Tag, uint32_t Count)
{
    char const    * XMLResult = NULL;
    char            MACChar0[32];
    char            MACChar1[32];
    char            MACChar2[32];
    char            MACChar3[32];
    char            MACChar4[32];
    char            MACChar5[32];
    uint8_t         sMAC[6];
    mxml_node_t   * MAC = NULL;
    if((MAC = mxmlFindElement(ParentNode, ParentNode, Tag, NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
    {
        if ((XMLResult = SetConfig_GetText(ParentNode, ParentNode, Tag)) != NULL)
        {
            if (strlen(XMLResult) != 17)
            {
                fprintf(stderr, "\nInvalid <%s> (length) '%.31s'\n", Tag, XMLResult);
                return FALSE;
            }
            if(Tag != "Mask")
            strcpy (gMACArray[Count], XMLResult);

            if(sscanf(XMLResult, "%2s:%2s:%2s:%2s:%2s:%2s", MACChar0, MACChar1, MACChar2, MACChar3, MACChar4, MACChar5) != 6)
            {
                fprintf(stderr, "\nInvalid <%s> (hex) '%.31s'\n",
                Tag, XMLResult);
                return FALSE;
            }

            sMAC[0] = strtoul(MACChar0, 0, 16);
            sMAC[1] = strtoul(MACChar1, 0, 16);
            sMAC[2] = strtoul(MACChar2, 0, 16);
            sMAC[3] = strtoul(MACChar3, 0, 16);
            sMAC[4] = strtoul(MACChar4, 0, 16);
            sMAC[5] = strtoul(MACChar5, 0, 16);
            structure[0] = sMAC[0];
            structure[1] = sMAC[1];
            structure[2] = sMAC[2];
            structure[3] = sMAC[3];
            structure[4] = sMAC[4];
            structure[5] = sMAC[5];
        }
    }
    if (NOT Is_Any_Duplicate_MAC(gMACArray, Count))
    {
        fprintf(stderr, "ERROR : Duplicate <MAC>\n", gOptConfigFile);
        return FALSE;
    }
    return TRUE;
}

/*
* Set_Config : Function called by rswitch tool to read the Forwarding Engine configuration from the XML
* Tree       : TSN XML Tree
* PortNode   :
*/
static bool Set_Config(mxml_node_t * Tree, const char * const PortNode)
{
    mxml_node_t   * ForwardingEngine = NULL;
    mxml_node_t   * PortNumber = NULL;
    mxml_node_t   * PortNumber1 = NULL;
    mxml_node_t   * FirstNode = NULL;
    mxml_node_t   * SecondNode = NULL;
    mxml_node_t   * ThirdNode = NULL;
    mxml_node_t   * FourthNode = NULL;
    mxml_node_t   * FifthNode = NULL;
    mxml_node_t   * value = NULL;

    uint32_t    GateCount = 0;
    uint32_t    GateControls = 0;
    uint32_t    Count = 0;
    uint32_t    PortCount = 0;
    uint32_t    MACSCount = 0;
    uint32_t    PCPCount = 0;
    uint32_t    CommunityCount = 0;
    uint32_t    CommunityInt = 0;
    uint128_t   Databig = 0;
    
    char const    * XMLResult = NULL;
    char  const   * ch = NULL;
    uint8_t         sFilter[8];
    char            FilterChar0[32];
    char            FilterChar1[32];
    char            FilterChar2[32];
    char            FilterChar3[32];
    char            FilterChar4[32];
    char            FilterChar5[32];
    char            Data128 = 0;
    unsigned char  * CommunityChar[100];
    
    if ((ForwardingEngine = mxmlFindElement(Tree, Tree, PortNode, NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
    {
        fprintf(stderr, "\nERROR : Unable to find <%s> in '%s'\n",
        PortNode, gOptConfigFile);
        return FALSE;
    }

    /*IngressFilter*/
    if ((FirstNode = mxmlFindElement(ForwardingEngine, ForwardingEngine, "IngressFilter", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
    {
        
        
        /*Schedules*/
        if((SecondNode = mxmlFindElement(FirstNode, FirstNode, "Schedules", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
        {
            /*Schedule*/
            if((ThirdNode = mxmlFindElement(SecondNode, SecondNode, "Schedule", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
            {
                fprintf(stderr, "\nERROR : No <Schedule> in <Schedules> in <IngressFilter> in <Forwarding> in '%s'\n",
                gOptConfigFile);
                return FALSE;
            }
            else
            {
                Count = 0;
                for(; ThirdNode != NULL; ThirdNode = mxmlWalkNext(ThirdNode, SecondNode, MXML_NO_DESCEND))
                {
                    if (ThirdNode->type != MXML_ELEMENT)
                    {
                        continue;
                    }
                    
                    /*PortNumber*/
                    if((PortNumber = mxmlFindElement(ThirdNode, ThirdNode, "PortNumber", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
                    {
                        fprintf(stderr, "\nERROR : Unable to find <PortNumber> in <Schedule> in <Schedules> in <IngressFilter> in <Forwarding> in '%s'\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                    
                    if(NOT SetConfig_PortNumber(PortNumber, &FWDConfig_t.qci_config.QCI_Gate_Config[Count].PortNumber, PortCount))
                    {
                        fprintf(stderr, "\nERROR: Unable to find <PortNumber> in <Schedule> in <Schedules> in <IngressFilter> in <Forwarding> in '%s'\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                    for(int i = 0; i < Count; i++)
                    {
                        if(FWDConfig_t.qci_config.QCI_Gate_Config[Count].PortNumber == FWDConfig_t.qci_config.QCI_Gate_Config[i].PortNumber)
                        {
                            fprintf(stderr, "\nERROR: Duplicate <PortNumber> '%d' in <Schedule> in <Schedules> in <IngressFilter> in <Forwarding> in '%s'\n",
                            FWDConfig_t.qci_config.QCI_Gate_Config[Count].PortNumber, gOptConfigFile);
                            return FALSE;                               
                        }
                    }
                    /*JitterClock*/
                    if(NOT SetConfig_Integer(ThirdNode, "JitterClock", 
                                &FWDConfig_t.qci_config.QCI_Gate_Config[Count].jitter_time, 255))
                    {
                        fprintf(stderr, "\nERROR: Unable to find <JitterClock> in <Schedule> in <Schedules> in <IngressFilter> in <Forwarding> in '%s'\n",
                        gOptConfigFile);
                        return FALSE;
                    }

                    /*Latency*/
                    if(NOT SetConfig_Integer(ThirdNode, "Latency", 
                                &FWDConfig_t.qci_config.QCI_Gate_Config[Count].latency_time, 65535))
                    {
                        fprintf(stderr, "\nERROR: Unable to find <Latency> in <Schedule> in <Schedules> in <IngressFilter> in <Forwarding> in '%s'\n",
                        gOptConfigFile);
                        return FALSE;
                    }

                    /*AdminStartTime*/
                    if ((ch = SetConfig_GetText(ThirdNode, ThirdNode, "AdminStartTime")) != NULL)
                    {
                        if (sscanf(ch, "%s", &Data128) != 1)
                        {
                            fprintf(stderr, "\nERROR: Invalid <AdminStartTime> in <Schedule> in <Schedules> in <IngressFilter> in <Forwarding> in '%s'\n", gOptConfigFile);
                            return FALSE;
                        }
                        else
                        {
                            Databig = 0;
                            char *p = &Data128; //add software restriction for maximum value?
                            for(; *p != 0; *p++)
                            Databig = Databig * 10 + *p - '0';
                            FWDConfig_t.qci_config.QCI_Gate_Config[Count].AdminBaseTime.nseconds = Databig;
                        }
                        if(Databig > 302231454903657293676543)  //2^78-1
                        {
                            fprintf(stderr, "\nERROR: Invalid <AdminStartTime> in <Schedule> in <Schedules> in <IngressFilter> in <Forwarding> in '%s', Range (0-302231454903657293676543)\n", gOptConfigFile);
                            return FALSE;                           
                        }
                    }
                    else
                    {
                        fprintf(stderr, "\nERROR: Unable to find <AdminStartTime> in <Schedule> in <Schedules> in <IngressFilter> in <Forwarding> in '%s'\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                    /*CycleTimeExtension*/
                    if ((ch = SetConfig_GetText(ThirdNode, ThirdNode, "CycleTimeExtension")) != NULL)
                    {
                        if (sscanf(ch, "%s", &Data128) != 1)
                        {
                            fprintf(stderr, "\nERROR: Invalid <CycleTimeExtension> in <Schedule> in <Schedules> in <IngressFilter> in <Forwarding> in '%s'\n", gOptConfigFile);
                            return FALSE;
                        }
                        else
                        {
                            Databig = 0;
                            char *p = &Data128; //add software restriction for maximum value?
                            for(; *p != 0; *p++)
                            Databig = Databig * 10 + *p - '0';
                            FWDConfig_t.qci_config.QCI_Gate_Config[Count].CycleTimeExtension.nseconds = Databig;
                        }
                        if(Databig > 302231454903657293676543)
                        {
                            fprintf(stderr, "\nERROR: Invalid <CycleTimeExtension> in <Schedule> in <Schedules> in <IngressFilter> in <Forwarding> in '%s', Range (0-302231454903657293676543)\n", gOptConfigFile);
                            return FALSE;                           
                        }
                    }
                    else
                    {
                        fprintf(stderr, "\nERROR: Unable to find <CycleTimeExtension> in <Schedule> in <Schedules> in <IngressFilter> in <Forwarding> in '%s'\n",
                        gOptConfigFile);
                        return FALSE;
                    }

                    /*gptp-timer-domain*/
                    if(NOT SetConfig_BinaryText(ThirdNode, "gptp-timer-domain", &FWDConfig_t.qci_config.QCI_Gate_Config[Count].timer_domain,
                                "1", "0", 1, 1))
                    {
                        fprintf(stderr, "\nERROR: Unable to find <gptp-timer-domain> in <Schedule> in <Schedules> in <IngressFilter> in <Forwarding> in '%s'\n",
                        gOptConfigFile);
                        return FALSE;
                    }

                    /*GateControl*/
                    if((FourthNode = mxmlFindElement(ThirdNode, ThirdNode, "GateControl", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
                    {
                        fprintf(stderr, "\nERROR : Unable to find <GateControl> in <Schedule> in <Schedules> in <IngressFilter> in <Forwarding> in '%s'\n",
                        gOptConfigFile);
                        return FALSE;
                    }

                    GateControls = 0;
                    for(; FourthNode != NULL; FourthNode = mxmlWalkNext(FourthNode, ThirdNode, MXML_NO_DESCEND))
                    {
                        if (FourthNode->type != MXML_ELEMENT)
                        {
                            continue;
                        }
                        
                        if(GateControls >= RENESAS_RSWITCH_MAX_INGRESS_GATE_CONTROLS)
                        {
                            fprintf(stderr, "ERROR : A maximum of %d <GateControl> entries allowed in <Schedule> in <Schedules> in <IngressFilter> in <Forwarding> in %s\n",
                            RENESAS_RSWITCH_MAX_INGRESS_GATE_CONTROLS, gOptConfigFile);
                            return FALSE;
                        }

                        /*GateID*/  
                        if(NOT SetConfig_Integer(FourthNode, "GateID", 
                                    &FWDConfig_t.qci_config.QCI_Gate_Config[Count].Gate[GateControls].GateID, RENESAS_RSWITCH_ETH_MAX_GATE_CONTROLS))
                        {
                            fprintf(stderr, "\nERROR: Unable to find <GateID> in <GateControl> in <Schedule> in <Schedules> in <IngressFilter> in <Forwarding> in '%s'\n",
                            gOptConfigFile);
                            return FALSE;
                        }
                        if ((FWDConfig_t.qci_config.QCI_Gate_Config[Count].Gate[GateControls].GateID < (Count*8)) ||
                                (FWDConfig_t.qci_config.QCI_Gate_Config[Count].Gate[GateControls].GateID > (Count*8+7)))
                        {
                            fprintf(stderr, "\nERROR: Invalid <GateID> in <GateControl> in <Schedule> in <Schedules> in <IngressFilter> in <Forwarding> in '%s' Range (%u-%u) for Schedule %u\n",
                            gOptConfigFile, Count*8, Count*8+7, Count);
                            return FALSE;
                        }
                        for(int i = 0; i < Count; i++)
                        {
                            if(FWDConfig_t.qci_config.QCI_Gate_Config[Count].Gate[GateControls].GateID == FWDConfig_t.qci_config.QCI_Gate_Config[i].Gate[GateControls].GateID)
                            {
                                fprintf(stderr, "\nERROR: Duplicate <GateID> '%d' in <GateControl> in <Schedule> in <Schedules> in <IngressFilter> in <Forwarding> in '%s'\n",
                                FWDConfig_t.qci_config.QCI_Gate_Config[Count].Gate[GateControls].GateID, gOptConfigFile);
                                return FALSE;                               
                            }
                        }
                        /*InvalidRx*/
                        if(NOT SetConfig_BinaryText(FourthNode, "InvalidRx", &FWDConfig_t.qci_config.QCI_Gate_Config[Count].Gate[GateControls].Invalid_Rx, 
                                    "TRUE", "FALSE", 4, 5))
                        {
                            fprintf(stderr, "ERROR : Unable to find <InvalidRx> in <GateControl> in <Schedule> in <Schedules> in <IngressFilter> in <Forwarding> in %s\n",
                            gOptConfigFile);
                            return FALSE;
                        }
                        /*IPV*/
                        if(NOT SetConfig_Integer(FourthNode, "IPV", 
                                    &FWDConfig_t.qci_config.QCI_Gate_Config[Count].Gate[GateControls].IPV_enable, MAX_PRIORITY))
                        {
                            //~ fprintf(stderr, "\nERROR: Unable to find <IPV> in <GateControl> in <Schedule> in <Schedules> in <IngressFilter> in <Forwarding> in '%s'\n",
                            //~ gOptConfigFile);
                            //~ return FALSE;
                        }
                        /*Gate*/
                        if((FifthNode = mxmlFindElement(FourthNode, FourthNode, "Gate", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
                        {
                            fprintf(stderr, "\nERROR : Unable to find <Gate> in <GateControl> in <Schedule> in <Schedules> in <IngressFilter> in <Forwarding> in '%s'\n",
                            gOptConfigFile);
                            return FALSE;
                        }
                        else
                        {
                            GateCount = 0;
                            for(; FifthNode!= NULL; FifthNode = mxmlWalkNext(FifthNode, FourthNode, MXML_NO_DESCEND))
                            {
                                if (FifthNode->type != MXML_ELEMENT)
                                {
                                    continue;
                                }

                                if(GateCount >= RENESAS_RSWITCH_INGRESS_GATES_PER_SCHEDULE)
                                {
                                    fprintf(stderr, "\nERROR: A maximum of %d <Gate> entries allowed in <GateControl> in <Schedule> in <Schedules> in <IngressFilter> in <Forwarding> in '%s'\n",
                                    RENESAS_RSWITCH_INGRESS_GATES_PER_SCHEDULE, gOptConfigFile);
                                    return FALSE;
                                }
                                /*GateTimeTickMultiplier*/
                                if(NOT SetConfig_Integer(FifthNode, "GateTimeTickMultiplier", 
                                            &FWDConfig_t.qci_config.QCI_Gate_Config[Count].Gate[GateControls].GateControl[GateCount].GateTimeTickMultiplier, 65535))
                                {
                                    fprintf(stderr, "\nERROR: Unable to find <GateTimeTickMultiplier> in <Gate> in <GateControl> <Schedule> in <Schedules> in in <IngressFilter> in <Forwarding> in '%s'\n",
                                    gOptConfigFile);
                                    return FALSE;
                                }
                                /*State*/
                                if(NOT SetConfig_BinaryText(FifthNode, "State", &FWDConfig_t.qci_config.QCI_Gate_Config[Count].Gate[GateControls].GateControl[GateCount].State, 
                                            "Open", "Closed", 4, 6))
                                {
                                    fprintf(stderr, "ERROR : Unable to find <State> in <Gate> in <GateControl> in <Schedule> in <Schedules> in <IngressFilter> in <Forwarding> in %s\n",
                                    gOptConfigFile);
                                    return FALSE;
                                }

                                /*IPV*/
                                if(NOT SetConfig_Integer(FifthNode, "IPV", 
                                            &FWDConfig_t.qci_config.QCI_Gate_Config[Count].Gate[GateControls].GateControl[GateCount].IPV, MAX_PRIORITY))
                                {
                                    fprintf(stderr, "ERROR : Unable to find <IPV> in <Gate> in <GateControl> in <Schedule> in <Schedules> in <IngressFilter> in <Forwarding> in %s\n",
                                    gOptConfigFile);
                                    return FALSE;
                                }
                                GateCount++;
                            }
                            FWDConfig_t.qci_config.QCI_Gate_Config[Count].Gate[GateControls].GateControls = GateCount;
                        }
                        GateControls++;
                    }
                    
                    FWDConfig_t.qci_config.QCI_Gate_Config[Count].Gates = GateControls;
                    Count++;
                }
                FWDConfig_t.qci_config.Schedules = Count;
            }
        }
        
        
        /*SDUFilters*/
        if((SecondNode = mxmlFindElement(FirstNode, FirstNode, "SDUFilters", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
        {
            /*SDUFilter*/
            if((ThirdNode = mxmlFindElement(SecondNode, SecondNode, "SDUFilter", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
            {
                fprintf(stderr, "\nERROR : No <SDUFilter> in <SDUFilters> in <IngressFilter> in <Forwarding> in '%s'\n",
                gOptConfigFile);
                return FALSE;
            }
            else
            {
                Count = 0;
                for(; ThirdNode!= NULL; ThirdNode = mxmlWalkNext(ThirdNode, SecondNode, MXML_NO_DESCEND))
                {
                    if (ThirdNode->type != MXML_ELEMENT)
                    {
                        continue;
                    }
                    /*StreamBlockedDueToOversizeFrame*/
                    if(NOT SetConfig_BinaryText(ThirdNode, "StreamBlockedDueToOversizeFrame", &FWDConfig_t.qci_config.Msdu_Config[Count].msdu_enable,
                                "TRUE", "FALSE", 4, 5))
                    {
                        fprintf(stderr, "ERROR : Unable to find <StreamBlockedDueToOversizeFrame> in <SDUFilter> in <SDUFilters> in <IngressFilter> in <Forwarding> in %s\n",
                        gOptConfigFile);
                        return FALSE;
                    }

                    /*MSDUFilterNum*/
                    if(NOT SetConfig_Integer(ThirdNode, "MSDUFilterNum", 
                                &FWDConfig_t.qci_config.Msdu_Config[Count].msdu_id, RENESAS_RSWITCH_MSDU_FILTERS))
                    {
                        fprintf(stderr, "\nERROR: Unable to find <MSDUFilterNum> in <SDUFilter> in <SDUFilters> in <IngressFilter> in <Forwarding> in '%s'\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                    for(int i = 0; i < Count; i++)
                    {
                        if(FWDConfig_t.qci_config.Msdu_Config[Count].msdu_id == FWDConfig_t.qci_config.Msdu_Config[i].msdu_id)
                        {
                            fprintf(stderr, "\nERROR: Duplicate <MSDUFilterNum> '%d' in <SDUFilter> in <SDUFilters> in <IngressFilter> in <Forwarding> in '%s'\n",
                            FWDConfig_t.qci_config.Msdu_Config[Count].msdu_id, gOptConfigFile);
                            return FALSE;                               
                        }
                    }

                    /*MaxSDUSize*/
                    if(NOT SetConfig_Integer(ThirdNode, "MaxSDUSize", 
                                &FWDConfig_t.qci_config.Msdu_Config[Count].msdu_size, 65535))
                    {
                        fprintf(stderr, "\nERROR: Unable to find <MaxSDUSize> in <SDUFilter> in <SDUFilters> in <IngressFilter> in <Forwarding> in '%s'\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                    Count++;
                }
                if(Count >= RENESAS_RSWITCH_MSDU_FILTERS)
                {
                    fprintf(stderr, "ERROR : A maximum of %d <SDUFilter> entries allowed in <SDUFilters> in <IngressFilter> in <Forwarding> in %s\n",
                    RENESAS_RSWITCH_MSDU_FILTERS, gOptConfigFile);
                    return FALSE;
                }
                
                FWDConfig_t.qci_config.MSDU_Filters = Count;
            }
        }
        

        /*Filters*/
        if((SecondNode = mxmlFindElement(FirstNode, FirstNode, "Filters", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
        {

            /*Filter*/
            if((ThirdNode = mxmlFindElement(SecondNode, SecondNode, "Filter", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
            {
                fprintf(stderr, "\nERROR : No <Filter> in <Filters> in <IngressFilter> in <Forwarding> in '%s'\n",
                gOptConfigFile);
                return FALSE;
            }
            else
            {
                Count = 0;
                for(; ThirdNode!= NULL; ThirdNode = mxmlWalkNext(ThirdNode, SecondNode, MXML_NO_DESCEND))
                {
                    if (ThirdNode->type != MXML_ELEMENT)
                    {
                        continue;
                    }

                    /*FilterID*/
                    if ((XMLResult = SetConfig_GetText(ThirdNode, ThirdNode, "FilterID")) != NULL)
                    {
                        if (strlen(XMLResult) != 17)
                        {
                            fprintf(stderr, "\nERROR: Invalid <FilterID> (length) '%.31s' in <Filter> in <Filters> in <IngressFilter> in <Forwarding> in '%s'\n",
                            XMLResult, gOptConfigFile);
                            return FALSE;
                        }

                        if(sscanf(XMLResult, "%2s:%2s:%2s:%2s:%2s:%2s", FilterChar0, FilterChar1, FilterChar2, FilterChar3, FilterChar4, FilterChar5) != 6)
                        {
                            fprintf(stderr, "\nERROR: Invalid <FilterID> (hex) '%.31s' in <Filter> in <Filters> in <IngressFilter> in <Forwarding> in '%s'\n",
                            XMLResult, gOptConfigFile);
                            return FALSE;
                        }
                        strcpy (gMACArray[Count], XMLResult);

                        sFilter[0] = strtoul(FilterChar0, 0, 16);
                        sFilter[1] = strtoul(FilterChar1, 0, 16);
                        sFilter[2] = strtoul(FilterChar2, 0, 16);
                        sFilter[3] = strtoul(FilterChar3, 0, 16);
                        sFilter[4] = strtoul(FilterChar4, 0, 16);
                        sFilter[5] = strtoul(FilterChar5, 0, 16);
                        FWDConfig_t.qci_config.Ingress_Filters_Config[Count].Filter_ID[0] = sFilter[0];
                        FWDConfig_t.qci_config.Ingress_Filters_Config[Count].Filter_ID[1] = sFilter[1];
                        FWDConfig_t.qci_config.Ingress_Filters_Config[Count].Filter_ID[2] = sFilter[2];
                        FWDConfig_t.qci_config.Ingress_Filters_Config[Count].Filter_ID[3] = sFilter[3];
                        FWDConfig_t.qci_config.Ingress_Filters_Config[Count].Filter_ID[4] = sFilter[4];
                        FWDConfig_t.qci_config.Ingress_Filters_Config[Count].Filter_ID[5] = sFilter[5];
                    }
                    else
                    {
                        fprintf(stderr, "ERROR : Unable to find <FilterID> in <Filter> in <Filters> in <IngressFilter> in <Forwarding> in %s\n",
                        gOptConfigFile);
                        return FALSE;
                    }


                    /*PCP*/
                    if((FourthNode = mxmlFindElement(ThirdNode, ThirdNode, "PCP", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
                    {
                        /*PCP-ID*/
                        if(NOT SetConfig_Integer(FourthNode, "PCP-ID", 
                                    &FWDConfig_t.qci_config.Ingress_Filters_Config[Count].Ingress_PCP_Config.PCP_ID, RENESAS_RSWITCH_VLAN_PCP_VALUES))
                        {
                            fprintf(stderr, "\nERROR: Unable to find <PCP-ID> in <PCP> in <Filter> in <Filters> in <IngressFilter> in <Forwarding> in '%s'\n",
                            gOptConfigFile);
                            return FALSE;
                        }
                        for(int i = 0; i < Count; i++)
                        {
                            if(FWDConfig_t.qci_config.Ingress_Filters_Config[Count].Ingress_PCP_Config.PCP_ID == FWDConfig_t.qci_config.Ingress_Filters_Config[i].Ingress_PCP_Config.PCP_ID)
                            {
                                fprintf(stderr, "\nERROR: Duplicate <PCP-ID> '%d' in <PCP> in <Filter> in <Filters> in <IngressFilter> in <Forwarding> in '%s'\n",
                                FWDConfig_t.qci_config.Ingress_Filters_Config[Count].Ingress_PCP_Config.PCP_ID, gOptConfigFile);
                                return FALSE;                               
                            }
                        }
                        FWDConfig_t.qci_config.Ingress_Filters_Config[Count].Ingress_PCP_Config.bEnable = TRUE;
                    }
                    else
                    {
                        fprintf(stderr, "ERROR : Unable to find <PCP> in <Filter> in <Filters> in <IngressFilter> in <Forwarding> in %s\n",
                        gOptConfigFile);
                        return FALSE;
                    }

                    /*VLAN*/
                    if((FourthNode = mxmlFindElement(ThirdNode, ThirdNode, "VLAN", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
                    {

                        /*VLAN-ID*/
                        if(NOT SetConfig_Integer(FourthNode, "VLAN-ID",
                                    &FWDConfig_t.qci_config.Ingress_Filters_Config[Count].Ingress_VLAN_Config.VLAN_ID, RENESAS_RSWITCH_MAX_VLAN_ID))
                        {
                            fprintf(stderr, "\nERROR: Unable to find <VLAN-ID> in <VLAN> in <Filter> in <Filters> in <IngressFilter> in <Forwarding> in '%s'\n",
                            gOptConfigFile);
                            return FALSE;
                        }
                        for(int i = 0; i < Count; i++)
                        {
                            if(FWDConfig_t.qci_config.Ingress_Filters_Config[Count].Ingress_VLAN_Config.VLAN_ID == FWDConfig_t.qci_config.Ingress_Filters_Config[i].Ingress_VLAN_Config.VLAN_ID)
                            {
                                fprintf(stderr, "\nERROR: Duplicate <VLAN-ID> '%d' in <VLAN> in <Filter> in <Filters> in <IngressFilter> in <Forwarding> in '%s'\n",
                                FWDConfig_t.qci_config.Ingress_Filters_Config[Count].Ingress_VLAN_Config.VLAN_ID, gOptConfigFile);
                                return FALSE;                               
                            }
                        }
                        FWDConfig_t.qci_config.Ingress_Filters_Config[Count].Ingress_VLAN_Config.bEnable = TRUE;
                    }
                    else
                    {
                        fprintf(stderr, "ERROR : Unable to find <VLAN> in <Filter> in <Filters> in <IngressFilter> in <Forwarding> in %s\n",
                        gOptConfigFile);
                        return FALSE;
                    }

                    /*DEI*/
                    if(NOT SetConfig_BinaryText(ThirdNode, "DEI", &FWDConfig_t.qci_config.Ingress_Filters_Config[Count].DEI,
                                "1", "0", 1, 1))
                    {
                        fprintf(stderr, "ERROR : Unable to find <DEI> in <Filter> in <Filters> in <IngressFilter> in <Forwarding> in %s\n",
                        gOptConfigFile);
                        return FALSE;
                    }

                    /*GateID*/
                    if(NOT SetConfig_Integer(ThirdNode, "GateID",
                                &FWDConfig_t.qci_config.Ingress_Filters_Config[Count].FilterGate.Gate_ID, RENESAS_RSWITCH_ETH_MAX_GATE_CONTROLS))
                    {
                        fprintf(stderr, "\nERROR: Unable to find <GateID> in <Filter> in <Filters> in <IngressFilter> in <Forwarding> in '%s'\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                    for(int i = 0; i < Count; i++)
                    {
                        if(FWDConfig_t.qci_config.Ingress_Filters_Config[Count].FilterGate.Gate_ID == FWDConfig_t.qci_config.Ingress_Filters_Config[i].FilterGate.Gate_ID)
                        {
                            fprintf(stderr, "\nERROR: Duplicate <GateID> '%d' in <Filter> in <Filters> in <IngressFilter> in <Forwarding> in '%s'\n",
                            FWDConfig_t.qci_config.Ingress_Filters_Config[Count].FilterGate.Gate_ID, gOptConfigFile);
                            return FALSE;                               
                        }
                    }
                    FWDConfig_t.qci_config.Ingress_Filters_Config[Count].FilterGate.bEnable = TRUE;

                    /*MeterID*/
                    if(NOT SetConfig_Integer(ThirdNode, "MeterID",
                                &FWDConfig_t.qci_config.Ingress_Filters_Config[Count].Ingress_Filter_Meter.Meter_ID, RENESAS_RSWITCH_INGRESS_METERS))
                    {
                        fprintf(stderr, "\nERROR: Unable to find <MeterID> in <Filter> in <Filters> in <IngressFilter> in <Forwarding> in '%s'\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                    for(int i = 0; i < Count; i++)
                    {
                        if(FWDConfig_t.qci_config.Ingress_Filters_Config[Count].Ingress_Filter_Meter.Meter_ID == FWDConfig_t.qci_config.Ingress_Filters_Config[i].Ingress_Filter_Meter.Meter_ID)
                        {
                            fprintf(stderr, "\nERROR: Duplicate <MeterID> '%d' in <Filter> in <Filters> in <IngressFilter> in <Forwarding> in '%s'\n",
                            FWDConfig_t.qci_config.Ingress_Filters_Config[Count].Ingress_Filter_Meter.Meter_ID, gOptConfigFile);
                            return FALSE;                               
                        }
                    }
                    
                    FWDConfig_t.qci_config.Ingress_Filters_Config[Count].Ingress_Filter_Meter.bEnable = TRUE;
                    Count++;
                }
                if(Count >= RENESAS_RSWITCH_INGRESS_FILTERS)
                {
                    fprintf(stderr, "ERROR : A maximum of %d <Filter> entries allowed in <Filters> in <IngressFilter> in <Forwarding> in %s\n",
                    RENESAS_RSWITCH_INGRESS_FILTERS, gOptConfigFile);
                    return FALSE;
                }
                if (NOT Is_Any_Duplicate_MAC(gMACArray, Count))
                {
                    fprintf(stderr, "ERROR : Invalid <FilterID> in <Filter> in <Filters> in <IngressFilter> in <Forwarding> in %s\n",
                    gOptConfigFile);
                    return FALSE;
                }
                
                FWDConfig_t.qci_config.Filters = Count;
            }

        }
        

        /*Meters*/
        if((SecondNode = mxmlFindElement(FirstNode, FirstNode, "Meters", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
        {
            /*Meter*/
            if((ThirdNode = mxmlFindElement(SecondNode, SecondNode, "Meter", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
            {
                fprintf(stderr, "\nERROR : No <Meter> in <Meters> in <IngressFilter> in <Forwarding> in '%s'\n",
                gOptConfigFile);
                return FALSE;
            }
            else
            {
                Count = 0;
                for(; ThirdNode != NULL; ThirdNode = mxmlWalkNext(ThirdNode, SecondNode, MXML_NO_DESCEND))
                {
                    if (ThirdNode->type != MXML_ELEMENT)
                    {
                        continue;
                    }
                    /*MeterID*/
                    if(NOT SetConfig_Integer(ThirdNode, "MeterID",
                                &FWDConfig_t.qci_config.Ingress_Meter_Config[Count].Meter_Id, RENESAS_RSWITCH_INGRESS_METERS))
                    {
                        fprintf(stderr, "\nERROR: Unable to find <MeterID> in <Meter> in <Meters> in <IngressFilter> in <Forwarding> in '%s'\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                    for(int i = 0; i < Count; i++)
                    {
                        if(FWDConfig_t.qci_config.Ingress_Meter_Config[Count].Meter_Id == FWDConfig_t.qci_config.Ingress_Meter_Config[i].Meter_Id)
                        {
                            fprintf(stderr, "\nERROR: Duplicate <MeterID> '%d' in <Meter> in <Meters> in <IngressFilter> in <Forwarding> in '%s'\n",
                            FWDConfig_t.qci_config.Ingress_Meter_Config[Count].Meter_Id, gOptConfigFile);
                            return FALSE;                               
                        }
                    }
                    /*CBS*/
                    if(NOT SetConfig_Integer(ThirdNode, "CBS",
                                &FWDConfig_t.qci_config.Ingress_Meter_Config[Count].CBS, 16383))
                    {
                        fprintf(stderr, "\nERROR: Unable to find <CBS> in <Meter> in <Meters> in <IngressFilter> in <Forwarding> in '%s'\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                    if (FWDConfig_t.qci_config.Ingress_Meter_Config[Count].CBS < 1)
                    {
                        fprintf(stderr, "\nERROR: Invalid <CBS> in <Meter> in <Meters> in <IngressFilter> in <Forwarding> in '%s' Range (1-%d)\n",
                        gOptConfigFile, 16383);
                        return FALSE;
                    }
                    /*CIR*/
                    if(NOT SetConfig_Integer(ThirdNode, "CIR",
                                &FWDConfig_t.qci_config.Ingress_Meter_Config[Count].CIR, 65535))
                    {
                        fprintf(stderr, "\nERROR: Unable to find <CIR> in <Meter> in <Meters> in <IngressFilter> in <Forwarding> in '%s'\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                    if (FWDConfig_t.qci_config.Ingress_Meter_Config[Count].CIR < 1)
                    {
                        fprintf(stderr, "\nERROR: Invalid <CIR> in <Meter> in <Meters> in <IngressFilter> in <Forwarding> in '%s' Range (1-%d)\n",
                        gOptConfigFile, 65535);
                        return FALSE;
                    }
                    /*EBS*/
                    if(NOT SetConfig_Integer(ThirdNode, "EBS",
                                &FWDConfig_t.qci_config.Ingress_Meter_Config[Count].EBS, 16383))
                    {
                        fprintf(stderr, "\nERROR: Unable to find <EBS> in <Meter> in <Meters> in <IngressFilter> in <Forwarding> in '%s'\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                    if (FWDConfig_t.qci_config.Ingress_Meter_Config[Count].EBS < 1)
                    {
                        fprintf(stderr, "\nERROR: Invalid <EBS> in <Meter> in <Meters> in <IngressFilter> in <Forwarding> in '%s' Range (1-%d)\n",
                        gOptConfigFile, 16383);
                        return FALSE;
                    }
                    /*EIR*/
                    if(NOT SetConfig_Integer(ThirdNode, "EIR",
                                &FWDConfig_t.qci_config.Ingress_Meter_Config[Count].EIR, 65535))
                    {
                        fprintf(stderr, "\nERROR: Unable to find <EIR> in <Meter> in <Meters> in <IngressFilter> in <Forwarding> in '%s'\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                    if (FWDConfig_t.qci_config.Ingress_Meter_Config[Count].EIR < 1)
                    {
                        fprintf(stderr, "\nERROR: Invalid <EIR> in <Meter> in <Meters> in <IngressFilter> in <Forwarding> in '%s' Range (1-%d)\n",
                        gOptConfigFile, 65535);
                        return FALSE;
                    }
                    /*DropOnYellow*/
                    if(NOT SetConfig_BinaryText(ThirdNode, "DropOnYellow", &FWDConfig_t.qci_config.Ingress_Meter_Config[Count].Drop_On_Yellow,
                                "TRUE", "FALSE", 4, 5))
                    {
                        fprintf(stderr, "ERROR : Unable to find <DropOnYellow> in <Meter> in <Meters> in <IngressFilter> in <Forwarding> in %s\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                    /*VLANMap*/
                    if((FourthNode = mxmlFindElement(ThirdNode, ThirdNode, "VLANMap", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
                    {
                        fprintf(stderr, "\nERROR: Unable to find <VLANMap> in <Meter> in <Meters> in <IngressFilter> in <Forwarding> in '%s'\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                    
                    PortCount = 0;
                    for(; FourthNode!= NULL; FourthNode = mxmlWalkNext(FourthNode, ThirdNode, MXML_NO_DESCEND))
                    {
                        if (FourthNode->type != MXML_ELEMENT)
                        {
                            continue;
                        }

                        if (strcasecmp("VLANMap", FourthNode->value.element.name) == 0)
                        {
                            if(PortCount > RENESAS_RSWITCH_MAX_PCP_DEI_CONFIG)
                            {
                                fprintf(stderr, "\nERROR: A maximum of %d <VLANMap> entries allowed in <Meter> in <Meters> in <IngressFilter> in <Forwarding> in '%s' are supported\n",
                                RENESAS_RSWITCH_MAX_PCP_DEI_CONFIG, gOptConfigFile);
                                return FALSE;
                            }
                            
                            /*VLAN-ID*/
                            if(NOT SetConfig_Integer(FourthNode, "VLAN-ID",
                                        &FWDConfig_t.qci_config.Ingress_Meter_Config[Count].vlan_colour_map[PortCount].PCP_value, RENESAS_RSWITCH_MAX_VLAN_ID))
                            {
                                fprintf(stderr, "\nERROR: Unable to find <VLAN-ID> in <Meter> in <Meters> in <IngressFilter> in <Forwarding> in '%s'\n",
                                gOptConfigFile);
                                return FALSE;
                            }
                            
                            /*DEI*/
                            if(NOT SetConfig_Integer(FourthNode, "DEI",
                                        &FWDConfig_t.qci_config.Ingress_Meter_Config[Count].vlan_colour_map[PortCount].DEI_value, RENESAS_RSWITCH_VLAN_DEI_VALUES))
                            {
                                fprintf(stderr, "\nERROR: Unable to find <DEI> in <Meter> in <Meters> in <IngressFilter> in <Forwarding> in '%s'\n",
                                gOptConfigFile);
                                return FALSE;
                            }
                            
                            /*Colour*/
                            if(NOT SetConfig_BinaryText(FourthNode, "Colour", &FWDConfig_t.qci_config.Ingress_Meter_Config[Count].vlan_colour_map[PortCount].colour_code,
                                        "Red", "Yellow", 3, 6))
                            {
                                fprintf(stderr, "\nERROR: Unable to find <Colour> in <Meter> in <Meters> in <IngressFilter> in <Forwarding> in '%s'\n",
                                gOptConfigFile);
                                return FALSE;
                            }
                            PortCount++;
                        }
                    }
                    
                    if(PortCount != RENESAS_RSWITCH_MAX_PCP_DEI_CONFIG)
                    {
                        fprintf(stderr, "\nERROR: A minimum of %d <VLANMap> entries are supported in <Meter> in <Meters> in <IngressFilter> in <Forwarding> in '%s'\n",
                        RENESAS_RSWITCH_MAX_PCP_DEI_CONFIG, gOptConfigFile);
                        return FALSE;
                    }
                    
                    /*MarkAllFramesEnable*/
                    if(NOT SetConfig_BinaryText(ThirdNode, "MarkAllFramesEnable", &FWDConfig_t.qci_config.Ingress_Meter_Config[Count].Mark_All_Red_Enable,
                                "TRUE", "FALSE", 4, 5))
                    {
                        fprintf(stderr, "ERROR : Unable to find <MarkAllFramesEnable> in <Meter> in <Meters> in <IngressFilter> in <Forwarding> in %s\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                    
                    /*MarkAllFramesRed*/
                    if(NOT SetConfig_BinaryText(ThirdNode, "MarkAllFramesRed", &FWDConfig_t.qci_config.Ingress_Meter_Config[Count].Mark_All_Frames_Red,
                                "TRUE", "FALSE", 4, 5))
                    {
                        fprintf(stderr, "ERROR : Unable to find <MarkAllFramesRed> in <Meter> in <Meters> in <IngressFilter> in <Forwarding> in %s\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                    Count++;
                }
                if(Count >= RENESAS_RSWITCH_INGRESS_METERS)
                {
                    fprintf(stderr, "ERROR : A maximum of %d <Meter> entries allowed in <Meters> in <IngressFilter> in <Forwarding> in %s\n",
                    RENESAS_RSWITCH_INGRESS_METERS, gOptConfigFile);
                    return FALSE;
                }
                FWDConfig_t.qci_config.Meters = Count;
            }
        }
        else
        {
            
        }
        FWDConfig_t.qci_config.bEnable = TRUE;
    }

    /*Broadcast*/
    if ((FirstNode = mxmlFindElement(ForwardingEngine, ForwardingEngine, "Broadcast", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
    {
        /*ConfigPorts*/
        if ((SecondNode = mxmlFindElement(FirstNode, FirstNode, "ConfigPorts", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
        {
            /*ConfigPort*/
            if ((ThirdNode = mxmlFindElement(SecondNode, SecondNode, "ConfigPort", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
            {
                fprintf(stderr, "\nERROR : No <ConfigPort> in <ConfigPorts> in <Broadcast> in <Forwarding> in '%s'\n",
                gOptConfigFile);
                return FALSE;
            }

            Count = 0;
            for(; ThirdNode != NULL; ThirdNode = mxmlWalkNext(ThirdNode, SecondNode, MXML_NO_DESCEND))
            {
                if (ThirdNode->type != MXML_ELEMENT)
                {
                    continue;
                }
                /*CPU*/
                if(NOT SetConfig_BinaryText(ThirdNode, "CPU", &FWDConfig_t.Broadcast_Config[Count].Broadcast_Config_Port.CPU_Port,
                            "Yes", "No", 3, 2))
                {
                    
                }
                
                if(NOT FWDConfig_t.Broadcast_Config[Count].Broadcast_Config_Port.CPU_Port)
                {
                    /*PortNumber*/
                    if((PortNumber = mxmlFindElement(ThirdNode, ThirdNode, "PortNumber", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
                    {
                        fprintf(stderr, "\nERROR : No <PortNumber> in <ConfigPort> in <ConfigPorts> in <Broadcast> in <Forwarding> in '%s'\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                    if(NOT SetConfig_PortNumber(PortNumber, &FWDConfig_t.Broadcast_Config[Count].Broadcast_Config_Port.PortNumber, Count))
                    {
                        fprintf(stderr, "ERROR : Unable to find <PortNumber> in <ConfigPort> in <ConfigPorts> in <Broadcast> in <Forwarding> in %s\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                    for(int i = 0; i < Count; i++)
                    {
                        if((FWDConfig_t.Broadcast_Config[Count].Broadcast_Config_Port.PortNumber == FWDConfig_t.Broadcast_Config[i].Broadcast_Config_Port.PortNumber) && (FWDConfig_t.Broadcast_Config[Count].Broadcast_Config_Port.CPU_Port == 0))
                        {
                            fprintf(stderr, "\nERROR: Duplicate <PortNumber> '%d' in <ConfigPort> in <ConfigPorts>  <Broadcast> in <Forwarding> in '%s'\n",
                            FWDConfig_t.Broadcast_Config[Count].Broadcast_Config_Port.PortNumber, gOptConfigFile);
                            return FALSE;                               
                        }
                    }
                }
                

                /*CSDN*/
                if(NOT SetConfig_Integer(ThirdNode, "CSDN",
                            &FWDConfig_t.Broadcast_Config[Count].CSDN, MAX_CSDN))
                {
                    fprintf(stderr, "ERROR : Unable to find <CSDN> in <ConfigPort> in <ConfigPorts> in <Broadcast> in <Forwarding> in %s\n",
                    gOptConfigFile);
                    return FALSE;
                }

                /*Ports*/
                if((FourthNode = mxmlFindElement(ThirdNode, ThirdNode, "Ports", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
                {
                    /*PortNumber*/
                    if((PortNumber = mxmlFindElement(FourthNode, FourthNode, "PortNumber", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
                    {
                        
                    }
                    PortCount = 0;
                    for(; PortNumber != NULL; PortNumber = mxmlWalkNext(PortNumber, FourthNode, MXML_NO_DESCEND))
                    {
                        if (PortNumber->type != MXML_ELEMENT)
                        {
                            continue;
                        }
                        
                        if(NOT SetConfig_PortNumber(PortNumber, &FWDConfig_t.Broadcast_Config[Count].Dest_Eth[PortCount].PortNumber, PortCount))
                        {
                            
                        }
                        for(int i = 0; i < PortCount; i++)
                        {
                            if(FWDConfig_t.Broadcast_Config[Count].Dest_Eth[PortCount].PortNumber == FWDConfig_t.Broadcast_Config[Count].Dest_Eth[i].PortNumber)
                            {
                                fprintf(stderr, "\nERROR: Duplicate <PortNumber> '%d' in <Ports> in <ConfigPort> in <ConfigPorts> in <Broadcast> in <Forwarding> in '%s'\n",
                                FWDConfig_t.Broadcast_Config[Count].Dest_Eth[PortCount].PortNumber, gOptConfigFile);
                                return FALSE;                               
                            }
                        }
                        PortCount++;
                    }
                    FWDConfig_t.Broadcast_Config[Count].Dest_Eths = PortCount;
                }
                /*Dest-CPU*/
                if(NOT SetConfig_BinaryText(ThirdNode, "Dest-CPU", &FWDConfig_t.Broadcast_Config[Count].CPU_Enable,
                            "Yes", "No", 3, 2))
                {
                    if(NOT PortCount)
                    {
                        fprintf(stderr, "ERROR : Unable to find <Dest-CPU> or <PortNumber> in <Ports> in <ConfigPort> in <ConfigPorts> in <Broadcast> in <Forwarding> in %s\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                }
                Count++;
            }
            FWDConfig_t.Broadcast_Config_Ports = Count;
        }
        else if (G_Reconfig != TRUE)
        {
            fprintf(stderr, "ERROR : Unable to find <ConfigPorts> in <Broadcast> in <Forwarding> in %s\n",
            gOptConfigFile);
            return FALSE;
        }
    }


    /*Source-Forwarding*/
    if ((FirstNode = mxmlFindElement(ForwardingEngine, ForwardingEngine, "Source-Forwarding", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
    {
        /*Ports*/
        if ((SecondNode = mxmlFindElement(FirstNode, FirstNode, "Ports", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
        {
            /*Port*/
            if((ThirdNode = mxmlFindElement(SecondNode, SecondNode, "Port", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
            {
                fprintf(stderr, "\nERROR : No <Port> in <Ports> in <Source-Forwarding> in <Forwarding> in '%s'\n",
                gOptConfigFile);
                return FALSE;
            }
            else
            {
                PortCount = 0;
                for(; ThirdNode != NULL; ThirdNode = mxmlWalkNext(ThirdNode, SecondNode, MXML_NO_DESCEND))
                {
                    if (ThirdNode->type != MXML_ELEMENT)
                    {
                        continue;
                    }

                    /*PortNumber*/
                    if((PortNumber = mxmlFindElement(ThirdNode, ThirdNode, "PortNumber", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
                    {
                        fprintf(stderr, "\nERROR : No <PortNumber> in <Port> in <Ports> in <Source-Forwarding> in <IngressFilter> in <Forwarding> in '%s'\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                    if(NOT SetConfig_PortNumber(PortNumber, &FWDConfig_t.Source_Fwd.Source_Port_Config[PortCount].PortNumber, PortCount))
                    {
                        fprintf(stderr, "\nERROR: Unable to find <PortNumber> in <Port> in <Ports> in <Source-Forwarding> in <IngressFilter> in <Forwarding> in '%s'\n",
                        gOptConfigFile);
                        return FALSE;
                    }

                    for(int i = 0; i < PortCount; i++)
                    {
                        if(FWDConfig_t.Source_Fwd.Source_Port_Config[PortCount].PortNumber == FWDConfig_t.Source_Fwd.Source_Port_Config[i].PortNumber)
                        {
                            fprintf(stderr, "\nERROR: Duplicate <PortNumber> '%d' in <Port> in <Ports> in <Source-Forwarding> in <Forwarding> in '%s'\n",
                            FWDConfig_t.Source_Fwd.Source_Port_Config[PortCount].PortNumber, gOptConfigFile);
                            return FALSE;                               
                        }
                    }

                    /*CSDN*/
                    if(NOT SetConfig_Integer(ThirdNode, "CSDN",
                                &FWDConfig_t.Source_Fwd.Source_Port_Config[PortCount].Dest_Config.CSDN, MAX_CSDN))
                    {
                        fprintf(stderr, "ERROR : Unable to find <CSDN> in <Port> in <Ports> in <Source-Forwarding> in <Forwarding> in %s\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                    /*CPU-Host*/
                    if(NOT SetConfig_BinaryText(ThirdNode, "CPU-Host", &FWDConfig_t.Source_Fwd.Source_Port_Config[PortCount].CPU_Host,
                                "Yes", "No", 3, 2))
                    {
                        fprintf(stderr, "ERROR : Unable to find <CPU> in <Port> in <Ports> in <Source-Forwarding> in <Forwarding> in %s\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                    
                    /*CPU-Mirror*/
                    if(NOT SetConfig_BinaryText(ThirdNode, "CPU-Mirror", &FWDConfig_t.Source_Fwd.Source_Port_Config[PortCount].CPU_Mirror,
                                "Enable", "Disable", 6, 7))
                    {
                        fprintf(stderr, "ERROR : Unable to find <CPU-Mirror> in <Port> in <Ports> in <Source-Forwarding> in <Forwarding> in %s\n",
                        gOptConfigFile);
                        return FALSE;
                    }

                    /*Eth-Mirror*/
                    if(NOT SetConfig_BinaryText(ThirdNode, "Eth-Mirror", &FWDConfig_t.Source_Fwd.Source_Port_Config[PortCount].Eth_Mirror,
                                "Enable", "Disable", 6, 7))
                    {
                        fprintf(stderr, "ERROR : Unable to find <Eth-Mirror> in <Port> in <Ports> in <Source-Forwarding> in <Forwarding> in %s\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                    
                    
                    /*CPU*/
                    if(SetConfig_BinaryText(ThirdNode, "CPU", &FWDConfig_t.Source_Fwd.Source_Port_Config[PortCount].Dest_Config.CPU_Enable,
                                "Yes", "No", 3, 2))
                    {
                        
                    }
                    /*Dest-Ports*/
                    if((FourthNode = mxmlFindElement(ThirdNode, ThirdNode, "Dest-Ports", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
                    {
                        /*PortNumber*/
                        if((PortNumber = mxmlFindElement(FourthNode, FourthNode, "PortNumber", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
                        {
                            
                            fprintf(stderr, "\nERROR : No <PortNumber> in <Dest-Ports> in <Port> in <Ports> in <Source-Forwarding> in <Forwarding> in %s\n",
                            gOptConfigFile);
                            return FALSE;
                            
                        }
                        
                        Count = 0;
                        for(; PortNumber != NULL; PortNumber = mxmlWalkNext(PortNumber, FourthNode, MXML_NO_DESCEND))
                        {
                            if (PortNumber->type != MXML_ELEMENT)
                            {
                                continue;
                            }
                            
                            if((ch = mxmlGetElement(PortNumber)) != NULL)
                            {
                                if (strcasecmp("PortNumber", ch) == 0)
                                {
                                    if (PortNumber->type != MXML_ELEMENT)
                                    {
                                        continue;
                                    }
                                    if(NOT SetConfig_PortNumber(PortNumber, &FWDConfig_t.Source_Fwd.Source_Port_Config[PortCount].Dest_Config.Dest_Eth[Count].PortNumber, PortCount))
                                    {
                                        
                                    }
                                    for(int i = 0; i < Count; i++)
                                    {
                                        if(FWDConfig_t.Source_Fwd.Source_Port_Config[PortCount].Dest_Config.Dest_Eth[Count].PortNumber == FWDConfig_t.Source_Fwd.Source_Port_Config[PortCount].Dest_Config.Dest_Eth[i].PortNumber)
                                        {
                                            fprintf(stderr, "\nERROR: Duplicate <PortNumber> '%d' in <Dest-Ports> in <Port> in <Ports> in <Source-Forwarding> in <Forwarding> in '%s'\n",
                                            FWDConfig_t.Source_Fwd.Source_Port_Config[PortCount].Dest_Config.Dest_Eth[Count].PortNumber, gOptConfigFile);
                                            return FALSE;                               
                                        }
                                    }
                                }
                            }                   
                            Count++;
                        }
                        FWDConfig_t.Source_Fwd.Source_Port_Config[PortCount].Dest_Config.Dest_Eths = Count;
                    }
                    else
                    {
                        if(NOT FWDConfig_t.Source_Fwd.Source_Port_Config[PortCount].Dest_Config.CPU_Enable)
                        {
                            fprintf(stderr, "ERROR : Unable to find <CPU> or <Dest-Ports> in <Port> in <Ports> in <Source-Forwarding> in <Forwarding> in %s\n",
                            gOptConfigFile);
                            return FALSE;
                        }
                    }
                    
                    PortCount++;
                }
                FWDConfig_t.Source_Fwd.source_fwd_port_configs = PortCount;
            }
        }
        else
        {
            fprintf(stderr, "ERROR : Unable to find <Ports> in <SourceForwarding> in <Forwarding> in %s\n",
            gOptConfigFile);
            return FALSE;
        }
        FWDConfig_t.Source_Fwd.bEnable = TRUE;
    }


    /*Dynamic-Authentication*/
    if ((FirstNode = mxmlFindElement(ForwardingEngine, ForwardingEngine, "Dynamic-Authentication", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
    {
        /*MACe*/
        if(NOT SetConfig_BinaryText(FirstNode, "MACe", &FWDConfig_t.Dynamic_Auth.mac_e_enable,
                    "Enable", "Disable", 6, 7))
        {
            fprintf(stderr, "ERROR : Unable to find <MACe> in <Dynamic-Authentication> in <Forwarding> in %s\n",
            gOptConfigFile);
            return FALSE;
        }

        /*MAC3*/
        if(NOT SetConfig_BinaryText(FirstNode, "MAC3", &FWDConfig_t.Dynamic_Auth.mac_3_enable,
                    "Enable", "Disable", 6, 7))
        {
            fprintf(stderr, "ERROR : Unable to find <MAC3> in <Dynamic-Authentication> in <Forwarding> in %s\n",
            gOptConfigFile);
            return FALSE;
        }

        /*MAC0*/
        if(NOT SetConfig_BinaryText(FirstNode, "MAC0", &FWDConfig_t.Dynamic_Auth.mac_0_enable,
                    "Enable", "Disable", 6, 7))
        {
            fprintf(stderr, "ERROR : Unable to find <MAC0> in <Dynamic-Authentication> in <Forwarding> in %s\n",
            gOptConfigFile);
            return FALSE;
        }

        /*Priority*/
        if(SetConfig_Integer(FirstNode, "Priority",
                    &FWDConfig_t.Dynamic_Auth.Dest.Priority, MAX_PRIORITY))
        {
            FWDConfig_t.Dynamic_Auth.Dest.Priority_Enable = TRUE;
            
        }

        /*CSDN*/
        if(NOT SetConfig_Integer(FirstNode, "CSDN",
                    &FWDConfig_t.Dynamic_Auth.Dest.CSDN, MAX_CSDN))
        {
            fprintf(stderr, "ERROR : Unable to find <CSDN> in <Dynamic-Authentication> in <Forwarding> in %s\n",
            gOptConfigFile);
            return FALSE;
        }
        /*CPU*/
        if(NOT SetConfig_BinaryText(FirstNode, "CPU", &FWDConfig_t.Dynamic_Auth.Dest.CPU_Enable,
                    "Yes", "No", 3, 2))
        {
            
        }

        /*Ports*/
        if((SecondNode = mxmlFindElement(FirstNode, FirstNode, "Ports", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
        {
            /*PortNumber*/
            if((PortNumber = mxmlFindElement(SecondNode, SecondNode, "PortNumber", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
            {
                fprintf(stderr, "\nERROR : No <PortNumber> in <Ports> in <Dynamic-Authentication> in <Forwarding> in '%s'\n",
                gOptConfigFile);
                return FALSE;
            }

            PortCount = 0;
            for(; PortNumber != NULL; PortNumber = mxmlWalkNext(PortNumber, SecondNode, MXML_NO_DESCEND))
            {
                if (PortNumber->type != MXML_ELEMENT)
                {
                    continue;
                }

                if(NOT SetConfig_PortNumber(PortNumber, &FWDConfig_t.Dynamic_Auth.Dest.Dest_Eth[PortCount].PortNumber, PortCount))
                {
                    fprintf(stderr, "ERROR : Unable to find <PortNumber> in <Ports> in <Dynamic-Authentication> in <Forwarding> in %s\n",
                    gOptConfigFile);
                    return FALSE;
                }        
                for(int i = 0; i < PortCount; i++)
                {
                    if(FWDConfig_t.Dynamic_Auth.Dest.Dest_Eth[PortCount].PortNumber == FWDConfig_t.Dynamic_Auth.Dest.Dest_Eth[i].PortNumber)
                    {
                        fprintf(stderr, "\nERROR: Duplicate <PortNumber> '%d' in <Ports> in <Dynamic-Authentication> in <Forwarding> in '%s'\n",
                        FWDConfig_t.Dynamic_Auth.Dest.Dest_Eth[PortCount].PortNumber, gOptConfigFile);
                        return FALSE;                               
                    }
                }
                PortCount++;
            }
            FWDConfig_t.Dynamic_Auth.Dest.Dest_Eths = PortCount;
        }
        else
        {
            if(NOT FWDConfig_t.Dynamic_Auth.Dest.CPU_Enable)
            {
                fprintf(stderr, "ERROR : Unable to find <CPU> or <Ports> in <Dynamic-Authentication> in <Forwarding> in %s\n",
                gOptConfigFile);
                return FALSE;
            }
        }
        FWDConfig_t.Dynamic_Auth.bEnable = TRUE;
        
    }


    /*Destination-Forwarding*/
    if ((FirstNode = mxmlFindElement(ForwardingEngine, ForwardingEngine, "Destination-Forwarding", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
    {
        /*Dest-MACS*/
        if((SecondNode = mxmlFindElement(FirstNode, FirstNode, "Dest-MACS", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
        {
            fprintf(stderr, "\nERROR : No <Dest-MACS> in <Destination-Forwarding> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }
        else
        {

            MACSCount = 0;
            for(; SecondNode != NULL; SecondNode = mxmlWalkNext(SecondNode, FirstNode, MXML_NO_DESCEND))
            {
                if (SecondNode->type != MXML_ELEMENT)
                {
                    continue;
                }

                if(MACSCount >= 2)
                {
                    fprintf(stderr, "\nERROR : A maximum of 2 <Dest-MAC> entries allowed in <Destination-Forwarding> in <Forwarding> in '%s'\n",
                    gOptConfigFile);
                    return FALSE;
                }

                /*MAC*/
                if (NOT SetConfig_MAC(SecondNode, FWDConfig_t.Dest_Fwd.DestFwd_Entry[MACSCount].MAC, "MAC", MACSCount))
                {
                    fprintf(stderr, "\nERROR : Unable to find <MAC> in <Dest-MACS> in <Destination-Forwarding> in <Forwarding> in '%s'\n",
                    gOptConfigFile);
                    return FALSE;
                }


                /*Mask*/
                if (NOT SetConfig_MAC(SecondNode, FWDConfig_t.Dest_Fwd.DestFwd_Entry[MACSCount].Mask, "Mask", MACSCount))
                {
                    fprintf(stderr, "\nERROR : Unable to find <Mask> in <Dest-MACS> in <Destination-Forwarding> in <Forwarding> in '%s'\n",
                    gOptConfigFile);
                    return FALSE;
                }

                /*Priority*/
                if(SetConfig_Integer(SecondNode, "Priority",
                            &FWDConfig_t.Dest_Fwd.DestFwd_Entry[MACSCount].Destination.Priority, MAX_PRIORITY))
                {
                    FWDConfig_t.Dest_Fwd.DestFwd_Entry[MACSCount].Destination.Priority_Enable = TRUE;
                    
                }
                /*CPU*/
                if(NOT SetConfig_BinaryText(SecondNode, "CPU", &FWDConfig_t.Dest_Fwd.DestFwd_Entry[MACSCount].Destination.CPU_Enable,
                            "Yes", "No", 3, 2))
                {
                    
                }
                /*CSDN*/
                if(NOT SetConfig_Integer(SecondNode, "CSDN",
                            &FWDConfig_t.Dest_Fwd.DestFwd_Entry[MACSCount].Destination.CSDN, MAX_CSDN))
                {
                    fprintf(stderr, "ERROR : Unable to find <CSDN> in <Destination-Forwarding> in <Forwarding> in %s\n",
                    gOptConfigFile);
                    return FALSE;
                }

                /*Ports*/
                if((ThirdNode = mxmlFindElement(SecondNode, SecondNode, "Ports", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
                {
                    /*PortNumber*/
                    if((PortNumber = mxmlFindElement(ThirdNode, ThirdNode, "PortNumber", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
                    {
                        fprintf(stderr, "\nERROR : No <PortNumber> in <Ports> in <Destination-Forwarding> in <Forwarding> in '%s'\n",
                        gOptConfigFile);
                        return FALSE;
                    }

                    PortCount = 0;
                    for(; PortNumber != NULL; PortNumber = mxmlWalkNext(PortNumber, ThirdNode, MXML_NO_DESCEND))
                    {
                        if (PortNumber->type != MXML_ELEMENT)
                        {
                            continue;
                        }

                        if(NOT SetConfig_PortNumber(PortNumber, &FWDConfig_t.Dest_Fwd.DestFwd_Entry[MACSCount].Destination.Dest_Eth[PortCount].PortNumber, PortCount))
                        {
                            fprintf(stderr, "ERROR : Unable to find <PortNumber> in <Ports> in <Destination-Forwarding> in <Forwarding> in %s\n",
                            gOptConfigFile);
                            return FALSE;
                        }     
                        for(int i = 0; i < PortCount; i++)
                        {
                            if(FWDConfig_t.Dest_Fwd.DestFwd_Entry[MACSCount].Destination.Dest_Eth[PortCount].PortNumber == FWDConfig_t.Dest_Fwd.DestFwd_Entry[MACSCount].Destination.Dest_Eth[i].PortNumber)
                            {
                                fprintf(stderr, "\nERROR: Duplicate <PortNumber> '%d' in <Ports> in <Destination-Forwarding> in <Forwarding> in '%s'\n",
                                FWDConfig_t.Dest_Fwd.DestFwd_Entry[MACSCount].Destination.Dest_Eth[PortCount].PortNumber, gOptConfigFile);
                                return FALSE;                               
                            }
                        }
                        PortCount++;
                    }
                    FWDConfig_t.Dest_Fwd.DestFwd_Entry[MACSCount].Destination.Dest_Eths = PortCount;
                }
                else
                {
                    if(NOT FWDConfig_t.Dest_Fwd.DestFwd_Entry[MACSCount].Destination.CPU_Enable)
                    {
                        fprintf(stderr, "ERROR : Unable to find <CPU> or <Ports> in <Destination-Forwarding> in <Forwarding> in %s\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                }
                MACSCount++;
            }
            if (NOT Is_Any_Duplicate_MAC(gMACArray, MACSCount))
            {
                fprintf(stderr, "ERROR : Invalid <MAC> in <Dest-MACS> in <Destination-Forwarding> in <Forwarding> in %s\n",
                gOptConfigFile);
                return FALSE;
            }
            FWDConfig_t.Dest_Fwd.DestFwdEntries = MACSCount;
        }
        FWDConfig_t.Dest_Fwd.bEnable = TRUE;
    }


    /*BPDU-Forwarding*/
    if ((FirstNode = mxmlFindElement(ForwardingEngine, ForwardingEngine, "BPDU-Forwarding", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
    {
        /*Dest-MACS*/
        if((SecondNode = mxmlFindElement(FirstNode, FirstNode, "Dest-MACS", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
        {
            /*MAC*/
            if (NOT SetConfig_MAC(SecondNode, FWDConfig_t.BPDU_Fwd.MAC, "MAC", 0))
            {
                fprintf(stderr, "ERROR : Unable to find <MAC> in <Dest-MACS> in <BPDU-Forwarding> in <Forwarding> in %s\n",
                gOptConfigFile);
                return FALSE;
            }

            /*Mask*/
            if (NOT SetConfig_MAC(SecondNode, FWDConfig_t.BPDU_Fwd.Mask, "Mask", 0))
            {
                fprintf(stderr, "ERROR : Unable to find <Mask> in <Dest-MACS> in <BPDU-Forwarding> in <Forwarding> in %s\n",
                gOptConfigFile);
                return FALSE;
            }

            /*Priority*/
            if(SetConfig_Integer(SecondNode, "Priority",
                        &FWDConfig_t.BPDU_Fwd.BPDU_Dest.Priority, MAX_PRIORITY))
            {
                FWDConfig_t.BPDU_Fwd.BPDU_Dest.Priority_Enable = TRUE;
                
            }

            /*CSDN*/
            if(NOT SetConfig_Integer(SecondNode, "CSDN",
                        &FWDConfig_t.BPDU_Fwd.BPDU_Dest.CSDN, MAX_CSDN))
            {
                fprintf(stderr, "ERROR : Unable to find <CSDN> in <BPDU-Forwarding> in <Forwarding> in %s\n",
                gOptConfigFile);
                return FALSE;
            }
            /*CPU*/
            if(NOT SetConfig_BinaryText(SecondNode, "CPU", &FWDConfig_t.BPDU_Fwd.BPDU_Dest.CPU_Enable,
                        "Yes", "No", 3, 2))
            {
                
            }
            
            /*Ports*/
            if((ThirdNode = mxmlFindElement(SecondNode, SecondNode, "Ports", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
            {
                if((PortNumber = mxmlFindElement(ThirdNode, ThirdNode, "PortNumber", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
                {
                    fprintf(stderr, "\nERROR : No <PortNumber> in <Ports> in <BPDU-Forwarding> in <Forwarding> in '%s'\n",
                    gOptConfigFile);
                    return FALSE;
                }

                PortCount = 0;
                for(; PortNumber != NULL; PortNumber = mxmlWalkNext(PortNumber, ThirdNode, MXML_NO_DESCEND))
                {
                    if (PortNumber->type != MXML_ELEMENT)
                    {
                        continue;
                    }
                    
                    if(NOT SetConfig_PortNumber(PortNumber, &FWDConfig_t.BPDU_Fwd.BPDU_Dest.Dest_Eth[PortCount].PortNumber, PortCount))
                    {
                        fprintf(stderr, "\nERROR : Unable to find <PortNumber> in <Ports> in <BPDU-Forwarding> in <Forwarding> in '%s'\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                    for(int i = 0; i < PortCount; i++)
                    {
                        if(FWDConfig_t.BPDU_Fwd.BPDU_Dest.Dest_Eth[PortCount].PortNumber == FWDConfig_t.BPDU_Fwd.BPDU_Dest.Dest_Eth[i].PortNumber)
                        {
                            fprintf(stderr, "\nERROR: Duplicate <PortNumber> '%d' in <Ports> in <BPDU-Forwarding> in <Forwarding> in '%s'\n",
                            FWDConfig_t.BPDU_Fwd.BPDU_Dest.Dest_Eth[PortCount].PortNumber, gOptConfigFile);
                            return FALSE;                               
                        }
                    }
                    PortCount++;
                }
                FWDConfig_t.BPDU_Fwd.BPDU_Dest.Dest_Eths = PortCount;
            }
            else
            {
                if(NOT FWDConfig_t.BPDU_Fwd.BPDU_Dest.CPU_Enable)
                {
                    fprintf(stderr, "ERROR : Unable to find <CPU> or <Ports> in <BPDU-Forwarding> in <Forwarding> in %s\n",
                    gOptConfigFile);
                    return FALSE;
                }
            }
        }
        else
        {
            fprintf(stderr, "\nERROR : Unable to find <Dest-MACS> in <BPDU-Forwarding> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }
        FWDConfig_t.BPDU_Fwd.bEnable = TRUE;
    }


    /*Private-VLAN*/
    if ((FirstNode = mxmlFindElement(ForwardingEngine, ForwardingEngine, "Private-VLAN", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
    {
        FWDConfig_t.Priv_VLAN.source_independent = rswitch_fwd_Config_No;

        /*Source-Independent*/
        if(NOT SetConfig_BinaryText(FirstNode, "Source-Independent", &FWDConfig_t.Priv_VLAN.source_independent,
                    "Yes", "No", 3, 2))
        {
            fprintf(stderr, "ERROR : Unable to find <Source-Independent> in <Private-VLAN> in <Forwarding> in %s\n",
            gOptConfigFile);
            return FALSE;
        }
        
        if(FWDConfig_t.Priv_VLAN.source_independent == TRUE)
        {
            /*VLAN-Groups*/
            if ((SecondNode = mxmlFindElement(FirstNode, FirstNode, "VLAN-Groups", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
            {
                /*Group*/
                if((ThirdNode = mxmlFindElement(SecondNode, SecondNode, "Group", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
                {
                    fprintf(stderr, "\nERROR : Unable to find <Group> in <Group-Independent> in <Private-VLAN> in <Forwarding> in '%s'\n",
                    gOptConfigFile);
                    return FALSE;
                }
                Count = 0;
                for(; ThirdNode != NULL; ThirdNode = mxmlWalkNext(ThirdNode, SecondNode, MXML_NO_DESCEND))
                {
                    if (ThirdNode->type != MXML_ELEMENT)
                    {
                        continue;
                    }
                    
                    /*Group-ID*/
                    if(NOT SetConfig_Integer(ThirdNode, "Group-ID",
                                &FWDConfig_t.Priv_VLAN.vlan_group[Count].GrpID, RENESAS_RSWITCH_MAX_VLAN_GROUPS))
                    {
                        fprintf(stderr, "\nERROR: Unable to find <Group-ID> in <Group> in <VLAN-Groups> in <Private-VLAN> in <Forwarding> in '%s'\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                    for(int i = 0; i < Count; i++)
                    {
                        if(FWDConfig_t.Priv_VLAN.vlan_group[Count].GrpID == FWDConfig_t.Priv_VLAN.vlan_group[i].GrpID)
                        {
                            fprintf(stderr, "\nERROR: Duplicate <Group-ID> '%d' in <Group> in <VLAN-Groups> in <Private-VLAN> in <Forwarding> in '%s'\n",
                            FWDConfig_t.Priv_VLAN.vlan_group[Count].GrpID, gOptConfigFile);
                            return FALSE;                               
                        }
                    }
                    /*CPU*/
                    if(NOT SetConfig_BinaryText(ThirdNode, "CPU", &FWDConfig_t.Priv_VLAN.vlan_group[Count].CPU_Enable,
                                "Yes", "No", 3, 2))
                    {
                        
                    }
                    /*Ports*/
                    if((FourthNode = mxmlFindElement(ThirdNode, ThirdNode, "Ports", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
                    {
                        /*PortNumber*/
                        if((PortNumber = mxmlFindElement(FourthNode, FourthNode, "PortNumber", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
                        {
                            fprintf(stderr, "\nERROR : No <PortNumber> in <Ports> in <Group> in <VLAN-Groups> in <Private-VLAN> in <Forwarding> in '%s'\n",
                            gOptConfigFile);
                            return FALSE;
                        }

                        PortCount = 0;
                        for(; PortNumber != NULL; PortNumber = mxmlWalkNext(PortNumber, FourthNode, MXML_NO_DESCEND))
                        {
                            if (PortNumber->type != MXML_ELEMENT)
                            {
                                continue;
                            }

                            if(NOT SetConfig_PortNumber(PortNumber, &FWDConfig_t.Priv_VLAN.vlan_group[Count].Dest_Eth[PortCount].PortNumber, PortCount))
                            {
                                fprintf(stderr, "\nERROR : Unable to find <PortNumber> in <Ports> in <Group> in <VLAN-Groups> in <Private-VLAN> in <Forwarding> in '%s'\n",
                                gOptConfigFile);
                                return FALSE;
                            }
                            PortCount++;
                        }
                        FWDConfig_t.Priv_VLAN.vlan_group[Count].Dest_Eths = PortCount;
                    }
                    else
                    {
                        if(NOT FWDConfig_t.Priv_VLAN.vlan_group[Count].CPU_Enable)
                        {
                            fprintf(stderr, "ERROR : Unable to find <CPU> or <Ports> in <Group> in <VLAN-Groups> in <Private-VLAN> in <Forwarding> in %s\n",
                            gOptConfigFile);
                            return FALSE;
                        }
                    }
                    Count++;
                }
                FWDConfig_t.Priv_VLAN.VLAN_Groups = Count;
                for(int g1 = 0; g1 < Count; g1++)
                {
                    for(int p1 = 0; p1 < FWDConfig_t.Priv_VLAN.vlan_group[g1].Dest_Eths; p1++)
                    {
                        for(int g2 = 0; g2 < Count; g2++)
                        {
                            for(int p2 = 0; p2 < FWDConfig_t.Priv_VLAN.vlan_group[g2].Dest_Eths; p2++)
                            {
                                if((g2 != g1) || (p2 != p1))
                                {
                                    if((FWDConfig_t.Priv_VLAN.vlan_group[g1].Dest_Eth[p1].PortNumber == FWDConfig_t.Priv_VLAN.vlan_group[g2].Dest_Eth[p2].PortNumber))
                                    {
                                        fprintf(stderr, "\nERROR : Invalid <PortNumber> in <Ports> in <Group> in <VLAN-Groups> in <Private-VLAN> in <Forwarding> in '%s' Ports in each group are mutually exclusive from others\n",
                                        gOptConfigFile);
                                        return FALSE;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                fprintf(stderr, "ERROR : Unable to find <VLAN-Groups> in <Private-VLAN> in <Forwarding> in %s\n",
                gOptConfigFile);
                return FALSE;
            }
        }
        FWDConfig_t.Priv_VLAN.bEnable = TRUE;
    }


    /*Double-Tag*/
    if(SetConfig_Integer(ForwardingEngine, "Double-Tag", &FWDConfig_t.Double_Tag.tag_value, 65535))
    FWDConfig_t.Double_Tag.bEnable = TRUE; 

    /*Agent-Filter-CSD-Ports*/
    if((FirstNode = mxmlFindElement(ForwardingEngine, ForwardingEngine, "Agent-Filter-CSD-Ports", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
    {
        /*CPU*/
        if(NOT SetConfig_BinaryText(FirstNode, "CPU", &FWDConfig_t.AgentFilterCSDPorts.CPU_Enable,
                    "Yes", "No", 3, 2))
        {
            
        }
        /*Ports*/
        if((SecondNode = mxmlFindElement(FirstNode, FirstNode, "Ports", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
        {
            /*PortNumber*/
            if((PortNumber = mxmlFindElement(SecondNode, SecondNode, "PortNumber", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
            {
                fprintf(stderr, "\nERROR : No <PortNumber> in <Agent-Filter-CSD-Ports> in <Forwarding> in '%s'\n",
                gOptConfigFile);
                return FALSE;
            }
            
            PortCount = 0;
            for(; PortNumber != NULL; PortNumber = mxmlWalkNext(PortNumber, SecondNode, MXML_NO_DESCEND))
            {
                if (PortNumber->type != MXML_ELEMENT)
                {
                    continue;
                }
                
                if(NOT SetConfig_PortNumber(PortNumber, &FWDConfig_t.AgentFilterCSDPorts.Eth[PortCount].PortNumber, PortCount))
                {
                    fprintf(stderr, "\nERROR: Unable to find <PortNumber> in <Agent-Filter-CSD-Ports> in <Forwarding> in '%s'\n",
                    gOptConfigFile);
                    return FALSE;
                }
                for(int i = 0; i < PortCount; i++)
                {
                    if(FWDConfig_t.AgentFilterCSDPorts.Eth[PortCount].PortNumber == FWDConfig_t.AgentFilterCSDPorts.Eth[i].PortNumber)
                    {
                        fprintf(stderr, "\nERROR: Duplicate <PortNumber> '%d' in <Agent-Filter-CSD-Ports> in <Forwarding> in '%s'\n",
                        FWDConfig_t.AgentFilterCSDPorts.Eth[PortCount].PortNumber, gOptConfigFile);
                        return FALSE;                               
                    }
                }
                PortCount++;
            }
            FWDConfig_t.AgentFilterCSDPorts.Eths = PortCount;
        }
        else
        {
            if(NOT FWDConfig_t.AgentFilterCSDPorts.CPU_Enable)
            {
                fprintf(stderr, "ERROR : Unable to find <CPU> or <Ports> in <Agent-Filter-CSD-Ports> in <Forwarding> in %s\n",
                gOptConfigFile);
                return FALSE;
            }
        }
    }


    /*WaterMarkControl*/
    if((FirstNode = mxmlFindElement(ForwardingEngine, ForwardingEngine, "WaterMarkControl", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
    {
        /*Flush-Level*/
        if(NOT SetConfig_Integer(FirstNode, "Flush-Level",
                    &FWDConfig_t.WaterMarkControl.flush_level, 65535))
        {
            fprintf(stderr, "\nERROR: Unable to find <Flush-Level> in <WaterMarkControl> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }
        /*Critical-Level*/
        if(NOT SetConfig_Integer(FirstNode, "Critical-Level",
                    &FWDConfig_t.WaterMarkControl.critical_level, 65535))
        {
            fprintf(stderr, "\nERROR: Unable to find <Critical-Level> in <WaterMarkControl> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }
        /*Remaining-Pause-Frame-Assert*/
        if(NOT SetConfig_Integer(FirstNode, "Remaining-Pause-Frame-Assert",
                    &FWDConfig_t.WaterMarkControl.rmn_pause_frm_assrt, 65535))
        {
            fprintf(stderr, "\nERROR: Unable to find <Remaining-Pause-Frame-Assert> in <WaterMarkControl> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }
        /*Remaining-Pause-Frame-DAssert*/
        if(NOT SetConfig_Integer(FirstNode, "Remaining-Pause-Frame-DAssert",
                    &FWDConfig_t.WaterMarkControl.rmn_pause_frm_dassrt, 65535))
        {
            fprintf(stderr, "\nERROR: Unable to find <Remaining-Pause-Frame-DAssert> in <WaterMarkControl> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }

        /*Ports*/
        if((SecondNode = mxmlFindElement(FirstNode, FirstNode, "Ports", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
        {
            /*Port*/
            if((ThirdNode = mxmlFindElement(SecondNode, SecondNode, "Port", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
            {
                fprintf(stderr, "\nERROR : No <Port> in <Ports> in <WaterMarkControl> in <Forwarding> in '%s'\n",
                gOptConfigFile);
                return FALSE;
            }

            PortCount = 0;
            for(; ThirdNode!= NULL; ThirdNode = mxmlWalkNext(ThirdNode, SecondNode, MXML_NO_DESCEND))
            {
                if (ThirdNode->type != MXML_ELEMENT)
                {
                    continue;
                }
                
                
                
                if(NOT SetConfig_BinaryText(ThirdNode, "CPU", &FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[PortCount].CPU_Enable,
                            "Yes", "No", 3, 2))
                {
                    
                }
                if(NOT FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[PortCount].CPU_Enable)
                {
                    if((PortNumber = mxmlFindElement(ThirdNode, ThirdNode, "PortNumber", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
                    {
                        fprintf(stderr, "\nERROR : No <PortNumber>\n");
                        return FALSE;
                    }
                    if(NOT SetConfig_PortNumber(PortNumber, &FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[PortCount].PortNumber, PortCount))
                    {
                        fprintf(stderr, "\nERROR: Unable to find <CPU> or <PortNumber> in <Port> in <Ports> in <WaterMarkControl> in <Forwarding> in '%s'\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                    for(int i = 0; i < PortCount; i++)
                    {
                        if((FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[PortCount].PortNumber == FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[i].PortNumber) && (FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[i].CPU_Enable == 0))
                        {
                            fprintf(stderr, "\nERROR: Duplicate <PortNumber> '%d' in <WaterMarkControl> in <Forwarding> in '%s'\n",
                            FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[PortCount].PortNumber, gOptConfigFile);
                            return FALSE;                               
                        }
                    }
                }
                else
                {
                    if((PortNumber = mxmlFindElement(ThirdNode, ThirdNode, "PortNumber", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
                    {
                        
                        if(SetConfig_PortNumber(PortNumber, &FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[PortCount].PortNumber, PortCount))
                        {
                            fprintf(stderr, "\nERROR: Can't have <CPU> and <PortNumber> in <Port> in <Ports> in <WaterMarkControl> in <Forwarding> in '%s'\n",
                            gOptConfigFile);
                            return FALSE;
                        }
                    }
                    
                }

                /*NoVLAN-Flush*/
                if(NOT SetConfig_BinaryText(ThirdNode, "NoVLAN-Flush", &FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[PortCount].novlanflush,
                            "Accept", "Discard", 6, 7))
                {
                    fprintf(stderr, "\nERROR: Unable to find <NoVLAN-Flush> in <Port> in <Ports> in <WaterMarkControl> in <Forwarding> in '%s'\n",
                    gOptConfigFile);
                    return FALSE;
                }

                /*NoVLAN-CLevel*/
                if(NOT SetConfig_BinaryText(ThirdNode, "NoVLAN-CLevel", &FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[PortCount].novlanclevel,
                            "Accept", "Discard", 6, 7))
                {
                    fprintf(stderr, "\nERROR: Unable to find <NoVLAN-CLevel> in <Port> in <Ports> in <WaterMarkControl> in <Forwarding> in '%s'\n",
                    gOptConfigFile);
                    return FALSE;
                }

                /*PCP-DEIs*/
                if ((FourthNode = mxmlFindElement(ThirdNode, ThirdNode, "PCP-DEIs", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
                {
                    /*PCP-DEI*/
                    if((FifthNode = mxmlFindElement(FourthNode, FourthNode, "PCP-DEI", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
                    {
                        fprintf(stderr, "\nERROR : Unable to find <PCP-DEI> in <PCP-DEIs> in <Port> in <Ports> in <WaterMarkControl> in <Forwarding> in '%s'\n",
                        gOptConfigFile);
                        return FALSE;
                    }

                    PCPCount = 0;
                    for(; FifthNode != NULL; FifthNode = mxmlWalkNext(FifthNode, FourthNode, MXML_NO_DESCEND))
                    {
                        if(FifthNode->type != MXML_ELEMENT)
                        {
                            continue;
                        }

                        if(PCPCount >= RENESAS_RSWITCH_MAX_PCP_DEI_CONFIG)
                        {
                            fprintf(stderr, "\nERROR : A maximum of 16 <PCP-DEI> entries allowed in <PCP-DEIs> in <WaterMarkControl> in <Forwarding> in '%s'\n",
                            gOptConfigFile);
                            return FALSE;
                        }
                        
                        if (strcasecmp("PCP-DEI", FifthNode->value.element.name) == 0)
                        {
                            /*PCP-ID*/
                            if(NOT SetConfig_Integer(FifthNode, "PCP-ID",
                                        &FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[PortCount].pcp_dei_config[PCPCount].PCP_ID, RENESAS_RSWITCH_VLAN_PCP_VALUES))
                            {
                                fprintf(stderr, "\nERROR: No <PCP-ID> in <PCP-DEI> in <PCP-DEIs> in <Port> in <Ports> in <WaterMarkControl> in <Forwarding> in '%s'\n",
                                gOptConfigFile);
                                return FALSE;
                            }
                            /*DEI*/
                            if(NOT SetConfig_Integer(FifthNode, "DEI",
                                        &FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[PortCount].pcp_dei_config[PCPCount].DEI, RENESAS_RSWITCH_VLAN_DEI_VALUES))
                            {
                                fprintf(stderr, "\nERROR: No <DEI> in <PCP-DEI> in <PCP-DEIs> in <Port> in <Ports> in <WaterMarkControl> in <Forwarding> in '%s'\n",
                                gOptConfigFile);
                                return FALSE;
                            }
                            
                            /*Flush*/
                            if(NOT SetConfig_BinaryText(FifthNode, "Flush", &FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[PortCount].pcp_dei_config[PCPCount].flush,
                                        "Accept", "Discard", 6, 7))
                            {
                                fprintf(stderr, "\nERROR: Unable to find <Flush> in <PCP-DEI> in <PCP-DEIs> in <Port> in <Ports> in <WaterMarkControl> in  <Forwarding> in '%s'\n",
                                gOptConfigFile);
                                return FALSE;
                            }
                            /*CLevel*/
                            if(NOT SetConfig_BinaryText(FifthNode, "CLevel", &FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[PortCount].pcp_dei_config[PCPCount].clevel,
                                        "Accept", "Discard", 6, 7))
                            {
                                fprintf(stderr, "\nERROR: Unable to find <CLevel> in <PCP-DEI> in <PCP-DEIs> in <Port> in <Ports> in <WaterMarkControl> in  <Forwarding> in '%s'\n",
                                gOptConfigFile);
                                return FALSE;
                            }
                        }
                        PCPCount++;
                    }
                    FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[PortCount].PCP_DEI_Configs = PCPCount;

                    if(PCPCount != 16)
                    {
                        fprintf(stderr, "\nERROR: All 16 combinations of <PCP-ID> and <DEI> must be there <PCP-DEIs> in <Port> in <Ports> in <WaterMarkControl> in  <Forwarding> in '%s'\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                    for(int j = 0; j < PCPCount-1; j++)
                    {
                        for(int i = j + 1; i < PCPCount; i++)
                        {
                            if((FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[PortCount].pcp_dei_config[j].PCP_ID == FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[PortCount].pcp_dei_config[i].PCP_ID)
                                    && (FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[PortCount].pcp_dei_config[j].DEI == FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[PortCount].pcp_dei_config[i].DEI))
                            {
                                fprintf(stderr, "\nERROR: Duplicate PCP-DEI entry data <PCP-ID> '%d' in <PCP-DEI> in <PCP-DEIs> in <Port> in <Ports> in <WaterMarkControl> in  <Forwarding> in '%s'\n",
                                FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfig[PortCount].pcp_dei_config[i].PCP_ID, gOptConfigFile);
                                return FALSE;
                            }
                        }
                    }
                }
                else
                {
                    fprintf(stderr, "\nERROR: Unable to find <PCP-DEIs> in <Port> in <Ports> in <WaterMarkControl> in  <Forwarding> in '%s'\n",
                    gOptConfigFile);
                    return FALSE;
                }
                PortCount++;
            }
            FWDConfig_t.WaterMarkControl.WaterMarkControlPortConfigs = PortCount;
        }
        else
        {
            fprintf(stderr, "\nERROR: Unable to find <Ports> in <WaterMarkControl> in  <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }
        FWDConfig_t.WaterMarkControl.bEnable = TRUE;
    }


    /*Exceptional-Path*/
    if ((FirstNode = mxmlFindElement(ForwardingEngine, ForwardingEngine, "Exceptional-Path", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
    {
        /*AuthFailtoCPU*/
        if(NOT SetConfig_BinaryText(FirstNode, "AuthFailtoCPU", &FWDConfig_t.ExceptionalPath.AuthFailtoCPU,
                    "Enable", "Disable", 6, 7))
        {
            fprintf(stderr, "\nERROR: Unable to find <AuthFailtoCPU> in <ExceptionalPath> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }

        /*BPDUFailtoCPU*/
        if(NOT SetConfig_BinaryText(FirstNode, "BPDUFailtoCPU", &FWDConfig_t.ExceptionalPath.BPDUFailtoCPU,
                    "Enable", "Disable", 6, 7))
        {
            fprintf(stderr, "\nERROR: Unable to find <BPDUFailtoCPU> in <ExceptionalPath> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }

        /*WMarktoCPU*/
        if(NOT SetConfig_BinaryText(FirstNode, "WMarktoCPU", &FWDConfig_t.ExceptionalPath.WMarktoCPU,
                    "Enable", "Disable", 6, 7))
        {
            fprintf(stderr, "\nERROR: Unable to find <WMarktoCPU> in <ExceptionalPath> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }

        /*QCIrejecttoCPU*/
        if(NOT SetConfig_BinaryText(FirstNode, "QCIrejecttoCPU", &FWDConfig_t.ExceptionalPath.QCIrejecttoCPU,
                    "Enable", "Disable", 6, 7))
        {
            fprintf(stderr, "\nERROR: Unable to find <QCIrejecttoCPU> in <ExceptionalPath> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }

        /*LearnstatictoCPU*/
        if(NOT SetConfig_BinaryText(FirstNode, "LearnstatictoCPU", &FWDConfig_t.ExceptionalPath.LearnstatictoCPU,
                    "Enable", "Disable", 6, 7))
        {
            fprintf(stderr, "\nERROR: Unable to find <LearnstatictoCPU> in <ExceptionalPath> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }

        /*UnmatchSIDtoCPU*/
        if(NOT SetConfig_BinaryText(FirstNode, "UnmatchSIDtoCPU", &FWDConfig_t.ExceptionalPath.UnmatchSIDtoCPU,
                    "Enable", "Disable", 6, 7))
        {
            fprintf(stderr, "\nERROR: Unable to find <UnmatchSIDtoCPU> in <ExceptionalPath> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }

        /*UnmatchVLANtoCPU*/
        if(NOT SetConfig_BinaryText(FirstNode, "UnmatchVLANtoCPU", &FWDConfig_t.ExceptionalPath.UnmatchVLANtoCPU,
                    "Enable", "Disable", 6, 7))
        {
            fprintf(stderr, "\nERROR: Unable to find <UnmatchVLANtoCPU> in <ExceptionalPath> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }

        /*UnmatchMACtoCPU*/
        if(NOT SetConfig_BinaryText(FirstNode, "UnmatchMACtoCPU", &FWDConfig_t.ExceptionalPath.UnmatchMACtoCPU,
                    "Enable", "Disable", 6, 7))
        {
            fprintf(stderr, "\nERROR: Unable to find <UnmatchMACtoCPU> in <ExceptionalPath> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }

        /*NullDPVtoCPU*/
        if(NOT SetConfig_BinaryText(FirstNode, "NullDPVtoCPU", &FWDConfig_t.ExceptionalPath.NullDPVtoCPU,
                    "Enable", "Disable", 6, 7))
        {
            fprintf(stderr, "\nERROR: Unable to find <NullDPVtoCPU> in <ExceptionalPath> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }

        /*ErrorDesctoCPU*/
        if(NOT SetConfig_BinaryText(FirstNode, "ErrorDesctoCPU", &FWDConfig_t.ExceptionalPath.ErrorDesctoCPU,
                    "Enable", "Disable", 6, 7))
        {
            fprintf(stderr, "\nERROR: Unable to find <ErrorDesctoCPU> in <ExceptionalPath> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }

        /*FilterRejecttoCPU*/
        if(NOT SetConfig_BinaryText(FirstNode, "FilterRejecttoCPU", &FWDConfig_t.ExceptionalPath.FilterRejecttoCPU,
                    "Enable", "Disable", 6, 7))
        {
            fprintf(stderr, "\nERROR: Unable to find <FilterRejecttoCPU> in <ExceptionalPath> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }

        /*SPLViolationtoCPU*/
        if(NOT SetConfig_BinaryText(FirstNode, "SPLViolationtoCPU", &FWDConfig_t.ExceptionalPath.SPLViolationtoCPU,
                    "Enable", "Disable", 6, 7))
        {
            fprintf(stderr, "\nERROR: Unable to find <SPLViolationtoCPU> in <ExceptionalPath> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }
        
        /*Priority*/
        if(NOT SetConfig_Integer(FirstNode, "Priority",
                    &FWDConfig_t.ExceptionalPath.Priority, MAX_PRIORITY))
        {
            fprintf(stderr, "ERROR : Unable to find <Priority> in <Exceptional-Path> in <Forwarding> in %s\n",
            gOptConfigFile);
            return FALSE;
        }

        /*CSDN*/
        if(NOT SetConfig_Integer(FirstNode, "CSDN",
                    &FWDConfig_t.ExceptionalPath.CSDN, MAX_CSDN))
        {
            fprintf(stderr, "ERROR : Unable to find <CSDN> in <Exceptional-Path> in <Forwarding> in %s\n",
            gOptConfigFile);
            return FALSE;
        }
        FWDConfig_t.ExceptionalPath.bEnable = TRUE;
    }


    /*Learning*/
    if ((FirstNode = mxmlFindElement(ForwardingEngine, ForwardingEngine, "Learning", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
    {
        /*UnknownSourcePortLearn*/
        if(NOT SetConfig_BinaryText(FirstNode, "UnknownSourcePortLearn", &FWDConfig_t.Learning.UnknownSourcePortLearn,
                    "Enable", "Disable", 6, 7))
        {
            fprintf(stderr, "\nERROR: Unable to find <UnknownSourcePortLearn> in <Learning> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }

        /*HW-Learning*/
        if(NOT SetConfig_BinaryText(FirstNode, "HW-Learning", &FWDConfig_t.Learning.HWLearning,
                    "Enable", "Disable", 6, 7))
        {
            fprintf(stderr, "\nERROR: Unable to find <HW-Learning> in <Learning> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }

        /*VLAN-Learn*/
        if(NOT SetConfig_BinaryText(FirstNode, "VLAN-Learn", &FWDConfig_t.Learning.VLANLearning,
                    "Enable", "Disable", 6, 7))
        {
            fprintf(stderr, "\nERROR: Unable to find <VLAN-Learn> in <Learning> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }

        /*MAC-Learn*/
        if ((ch = SetConfig_GetText(FirstNode, FirstNode, "MAC-Learn")) != NULL)
        {
            if (strncasecmp(ch, "Inactive", 8) == 0)
            {
                FWDConfig_t.Learning.MACLearning = rswitch_fwd_MAC_learn_Inactive;
            }
            else if (strncasecmp(ch, "ActiveSucceedFailed", 19) == 0)
            {
                FWDConfig_t.Learning.MACLearning = rswitch_fwd_MAC_learn_ActiveSucceedFailed;
            }
            else if (strncasecmp(ch, "ActiveSucceed", 13) == 0)
            {
                FWDConfig_t.Learning.MACLearning = rswitch_fwd_MAC_learn_ActiveSucceed;
            }
            else if (strncasecmp(ch, "ActiveFailed", 12) == 0)
            {
                FWDConfig_t.Learning.MACLearning = rswitch_fwd_MAC_learn_ActiveFailed;
            }
            else
            {
                fprintf(stderr, "\nERROR: Invalid <MAC-Learn> in <Learning> in <Forwarding> in '%s'\n",
                gOptConfigFile);
                return FALSE;
            }
        }
        else
        {
            fprintf(stderr, "\nERROR: Unable to find <MAC-Learn> in <Learning> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }

        /*Priority*/
        if(NOT SetConfig_Integer(FirstNode, "Priority",
                    &FWDConfig_t.Learning.Priority, MAX_PRIORITY))
        {
            fprintf(stderr, "ERROR : Unable to find <Priority> in <Learning> in <Forwarding> in %s\n",
            gOptConfigFile);
            return FALSE;
        }

        /*CSDN*/
        if(NOT SetConfig_Integer(FirstNode, "CSDN",
                    &FWDConfig_t.Learning.CSDN, MAX_CSDN))
        {
            fprintf(stderr, "ERROR : Unable to find <CSDN> in <Learning> in <Forwarding> in %s\n",
            gOptConfigFile);
            return FALSE;
        }
        FWDConfig_t.Learning.bEnable = TRUE;
    }


    /*Mirroring*/
    if((FirstNode = mxmlFindElement(ForwardingEngine, ForwardingEngine, "Mirroring", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
    {
        /*SourceMirrorPorts*/
        if ((SecondNode = mxmlFindElement(FirstNode, FirstNode, "SourceMirrorPorts", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
        {
            /*CPU*/
            if(NOT SetConfig_BinaryText(SecondNode, "CPU", &FWDConfig_t.Mirroring.Source_PortConfig.CPU,
                        "Yes", "No", 3, 2))
            {
                
            }
            /*Ports*/
            if((ThirdNode = mxmlFindElement(SecondNode, SecondNode, "Ports", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
            {
                /*PortNumber*/
                if((PortNumber = mxmlFindElement(ThirdNode, ThirdNode, "PortNumber", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
                {
                    fprintf(stderr, "\nERROR : No <PortNumber> in <SourceMirrorPort> in <Mirroring> in <Forwarding> in '%s'\n",
                    gOptConfigFile);
                    return FALSE;
                }
                
                PortCount = 0;
                for(; PortNumber != NULL; PortNumber = mxmlWalkNext(PortNumber, ThirdNode, MXML_NO_DESCEND))
                {
                    if (PortNumber->type != MXML_ELEMENT)
                    {
                        continue;
                    }
                    
                    if(NOT SetConfig_PortNumber(PortNumber, &FWDConfig_t.Mirroring.Source_PortConfig.Port[PortCount].PortNumber, PortCount))
                    {
                        fprintf(stderr, "\nERROR : Unable to find <PortNumber> in <SourceMirrorPort> in <Mirroring> in <Forwarding> in '%s'\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                    for(int i = 0; i < PortCount; i++)
                    {
                        if(FWDConfig_t.Mirroring.Source_PortConfig.Port[PortCount].PortNumber == FWDConfig_t.Mirroring.Source_PortConfig.Port[i].PortNumber)
                        {
                            fprintf(stderr, "\nERROR: Duplicate <PortNumber> '%d' in <SourceMirrorPorts> in <Mirroring> in <Forwarding> in '%s'\n",
                            FWDConfig_t.Mirroring.Source_PortConfig.Port[PortCount].PortNumber, gOptConfigFile);
                            return FALSE;                               
                        }
                    }
                    PortCount++;
                }
                FWDConfig_t.Mirroring.Source_PortConfig.Ports = PortCount;
            }   
            else
            {
                if(NOT FWDConfig_t.Mirroring.Source_PortConfig.CPU)
                {
                    fprintf(stderr, "\nERROR: Unable to find <CPU> or <Ports> in <SourceMirrorPorts> in <Mirroring> in <Forwarding> in '%s'\n",
                    gOptConfigFile);
                    return FALSE;
                }
            }
        }
        else
        {
            fprintf(stderr, "\nERROR: Unable to find <SourceMirrorPorts> in <Mirroring> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }

        /*DestMirrorPorts*/
        if ((SecondNode = mxmlFindElement(FirstNode, FirstNode, "DestMirrorPorts", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
        {
            /*CPU*/
            if(NOT SetConfig_BinaryText(SecondNode, "CPU", &FWDConfig_t.Mirroring.Dest_PortConfig.CPU,
                        "Yes", "No", 3, 2))
            {
                
            }
            /*Ports*/
            if((ThirdNode = mxmlFindElement(SecondNode, SecondNode, "Ports", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
            {
                /*PortNumber*/
                if((PortNumber = mxmlFindElement(ThirdNode, ThirdNode, "PortNumber", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
                {
                    fprintf(stderr, "\nERROR : No <PortNumber> in <DestMirrorPorts> in <Mirroring> in <Forwarding> in '%s'\n",
                    gOptConfigFile);
                    return FALSE;
                }
                
                PortCount = 0;
                for(; PortNumber != NULL; PortNumber = mxmlWalkNext(PortNumber, ThirdNode, MXML_NO_DESCEND))
                {
                    if (PortNumber->type != MXML_ELEMENT)
                    {
                        continue;
                    }
                    
                    if(NOT SetConfig_PortNumber(PortNumber, &FWDConfig_t.Mirroring.Dest_PortConfig.Port[PortCount].PortNumber, PortCount))
                    {
                        fprintf(stderr, "\nERROR : Unable to find <PortNumber> in <DestMirrorPorts> in <Mirroring> in <Forwarding> in '%s'\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                    for(int i = 0; i < PortCount; i++)
                    {
                        if(FWDConfig_t.Mirroring.Dest_PortConfig.Port[PortCount].PortNumber == FWDConfig_t.Mirroring.Dest_PortConfig.Port[i].PortNumber)
                        {
                            fprintf(stderr, "\nERROR: Duplicate <PortNumber> '%d' in <DestMirrorPorts> in <Mirroring> in <Forwarding> in '%s'\n",
                            FWDConfig_t.Mirroring.Dest_PortConfig.Port[PortCount].PortNumber, gOptConfigFile);
                            return FALSE;                               
                        }
                    }
                    PortCount++;
                }
                FWDConfig_t.Mirroring.Dest_PortConfig.Ports = PortCount;
                
            }
            else
            {
                if(NOT FWDConfig_t.Mirroring.Dest_PortConfig.CPU)
                {
                    fprintf(stderr, "\nERROR: Unable to find <CPU> or <Ports> in <DestMirrorPorts> in <Mirroring> in <Forwarding> in '%s'\n",
                    gOptConfigFile);
                    return FALSE;
                }
            }
        }
        else
        {
            fprintf(stderr, "\nERROR: Unable to find <DestMirrorPorts> in <Mirroring> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }

        /*ForwardSourceDest*/
        if(NOT SetConfig_BinaryText(FirstNode, "ForwardSourceDest", &FWDConfig_t.Mirroring.ForwardSourceDest,
                    "SourceAndDest", "SourceOrDest", 13, 12))
        {
            fprintf(stderr, "\nERROR: Unable to find <ForwardSourceDest> in <Mirroring> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }

        /*Error-Mirror*/
        if(NOT SetConfig_BinaryText(FirstNode, "Error-Mirror", &FWDConfig_t.Mirroring.Error_Mirror,
                    "Enable", "Disable", 6, 7))
        {
            fprintf(stderr, "\nERROR: Unable to find <Error-Mirror> in <Mirroring> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }

        /*SDPriority-Type*/
        if(NOT SetConfig_BinaryText(FirstNode, "SDPriority-Type", &FWDConfig_t.Mirroring.SD_Mirror_Priority_Type,
                    "MirrorCtrl", "Desc", 10, 4))
        {
            fprintf(stderr, "\nERROR: Unable to find <SDPriority-Type> in <Mirroring> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }

        /*SDPriority*/
        if(NOT SetConfig_Integer(FirstNode, "SDPriority",
                    &FWDConfig_t.Mirroring.SD_Mirror_Priority_Value, MAX_PRIORITY))
        {
            fprintf(stderr, "\nERROR: No <SDPriority> in <Mirroring> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }

        /*EthPriority-Type*/
        if(NOT SetConfig_BinaryText(FirstNode, "EthPriority-Type", &FWDConfig_t.Mirroring.Eth_Mirror_Priority_Type,
                    "MirrorCtrl", "Desc", 10, 4))
        {
            fprintf(stderr, "\nERROR: Unable to find <EthPriority-Type> in <Mirroring> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }

        /*EthPriority*/
        if(NOT SetConfig_Integer(FirstNode, "EthPriority",
                    &FWDConfig_t.Mirroring.Eth_Mirror_Priority_Value, MAX_PRIORITY))
        {
            fprintf(stderr, "\nERROR: No <EthPriority> in <Mirroring> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }

        /*CPUPriority-Type*/
        if(NOT SetConfig_BinaryText(FirstNode, "CPUPriority-Type", &FWDConfig_t.Mirroring.CPU_Mirror_Priority_Type,
                    "MirrorCtrl", "Desc", 10, 4))
        {
            fprintf(stderr, "\nERROR: Unable to find <CPUPriority-Type> in <Mirroring> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }

        /*CPUPriority*/
        if(NOT SetConfig_Integer(FirstNode, "CPUPriority",
                    &FWDConfig_t.Mirroring.CPU_Mirror_Priority_Value, MAX_PRIORITY))
        {
            fprintf(stderr, "\nERROR: No <CPUPriority> in <Mirroring> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }

        /*CSDN*/
        if(NOT SetConfig_Integer(FirstNode, "CSDN",
                    &FWDConfig_t.Mirroring.CSDN, MAX_CSDN))
        {
            fprintf(stderr, "ERROR : Unable to find <CSDN> in <Mirroring> in <Forwarding> in %s\n",
            gOptConfigFile);
            return FALSE;
        }

        /*EthMirrorDest*/
        if ((SecondNode = mxmlFindElement(FirstNode, FirstNode, "EthMirrorDest", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
        {
            /*CPU*/
            if(NOT SetConfig_BinaryText(SecondNode, "CPU", &FWDConfig_t.Mirroring.DestEThMirrorPort.CPU,
                        "Yes", "No", 3, 2))
            {
                
            }
            /*Ports*/
            if((ThirdNode = mxmlFindElement(SecondNode, SecondNode, "Ports", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
            {
                /*PortNumber*/
                if((PortNumber = mxmlFindElement(ThirdNode, ThirdNode, "PortNumber", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
                {
                    fprintf(stderr, "\nERROR : No <PortNumber> in <EthMirrorDest> in <Mirroring> in <Forwarding> in '%s'\n",
                    gOptConfigFile);
                    return FALSE;
                }
                
                PortCount = 0;
                for(; PortNumber != NULL; PortNumber = mxmlWalkNext(PortNumber, ThirdNode, MXML_NO_DESCEND))
                {
                    if (PortNumber->type != MXML_ELEMENT)
                    {
                        continue;
                    }
                    
                    if(NOT SetConfig_PortNumber(PortNumber, &FWDConfig_t.Mirroring.DestEThMirrorPort.Port[PortCount].PortNumber, PortCount))
                    {
                        fprintf(stderr, "\nERROR : Unable to find <PortNumber> in <EthMirrorDest> in <Mirroring> in <Forwarding> in '%s'\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                    for(int i = 0; i < PortCount; i++)
                    {
                        if(FWDConfig_t.Mirroring.DestEThMirrorPort.Port[PortCount].PortNumber == FWDConfig_t.Mirroring.DestEThMirrorPort.Port[i].PortNumber)
                        {
                            fprintf(stderr, "\nERROR: Duplicate <PortNumber> '%d' in <EthMirrorDest> in <Mirroring> in <Forwarding> in '%s'\n",
                            FWDConfig_t.Mirroring.DestEThMirrorPort.Port[PortCount].PortNumber, gOptConfigFile);
                            return FALSE;                               
                        }
                    }
                    PortCount++;
                }
                FWDConfig_t.Mirroring.DestEThMirrorPort.Ports = PortCount;
            }
            else
            {
                if(NOT FWDConfig_t.Mirroring.DestEThMirrorPort.CPU)
                {
                    fprintf(stderr, "\nERROR: Unable to find <CPU> or <Ports> in <EthMirrorDest> in <Mirroring> in <Forwarding> in '%s'\n",
                    gOptConfigFile);
                    return FALSE;
                }
            }
        }
        else
        {
            fprintf(stderr, "\nERROR: Unable to find <EthMirrorDest> in <Mirroring> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }

        /*CPUMirrorDest*/
        if ((SecondNode = mxmlFindElement(FirstNode, FirstNode, "SDMirrorDest", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
        {   
            /*CPU*/
            if(NOT SetConfig_BinaryText(SecondNode, "CPU", &FWDConfig_t.Mirroring.DestCPUMirrorPort.CPU,
                        "Yes", "No", 3, 2))
            {
                
            }
            /*Ports*/
            if((ThirdNode = mxmlFindElement(SecondNode, SecondNode, "Ports", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
            {
                /*PortNumber*/
                if((PortNumber = mxmlFindElement(ThirdNode, ThirdNode, "PortNumber", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
                {
                    fprintf(stderr, "\nERROR : No <PortNumber> in <CPUMirrorDest> in <Mirroring> in <Forwarding> in '%s'\n",
                    gOptConfigFile);
                    return FALSE;
                }
                PortCount = 0;
                for(; PortNumber != NULL; PortNumber = mxmlWalkNext(PortNumber, ThirdNode, MXML_NO_DESCEND))
                {
                    if (PortNumber->type != MXML_ELEMENT)
                    {
                        continue;
                    }
                    
                    if(NOT SetConfig_PortNumber(PortNumber, &FWDConfig_t.Mirroring.DestCPUMirrorPort.Port[PortCount].PortNumber, PortCount))
                    {
                        fprintf(stderr, "\nERROR : No <PortNumber> in <CPUMirrorDest> in <Mirroring> in <Forwarding> in '%s'\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                    for(int i = 0; i < PortCount; i++)
                    {
                        if(FWDConfig_t.Mirroring.DestCPUMirrorPort.Port[PortCount].PortNumber == FWDConfig_t.Mirroring.DestCPUMirrorPort.Port[i].PortNumber)
                        {
                            fprintf(stderr, "\nERROR: Duplicate <PortNumber> '%d' in <CPUMirrorDest> in <Mirroring> in <Forwarding> in '%s'\n",
                            FWDConfig_t.Mirroring.DestCPUMirrorPort.Port[PortCount].PortNumber, gOptConfigFile);
                            return FALSE;                               
                        }
                    }
                    PortCount++;
                }
                FWDConfig_t.Mirroring.DestCPUMirrorPort.Ports = PortCount;
            }
            else
            {
                if(NOT FWDConfig_t.Mirroring.DestCPUMirrorPort.CPU)
                {
                    fprintf(stderr, "\nERROR: Unable to find <CPU> or <Ports> in <CPUMirrorDest> in <Mirroring> in <Forwarding> in '%s'\n",
                    gOptConfigFile);
                    return FALSE;
                }
            }
        }
        else
        {
            fprintf(stderr, "\nERROR: Unable to find <CPUMirrorDest> in <Mirroring> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }
        FWDConfig_t.Mirroring.bEnable = TRUE;
    }


    /*Port-Lock*/
    if ((FirstNode = mxmlFindElement(ForwardingEngine, ForwardingEngine, "Port-Lock", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
    {
        /*CPU*/
        if(NOT SetConfig_BinaryText(FirstNode, "CPU", &FWDConfig_t.Port_Lock.LockPort.CPU,
                    "Yes", "No", 3, 2))
        {
            
        }
        /*Ports*/
        if((SecondNode = mxmlFindElement(FirstNode, FirstNode, "Ports", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
        {
            /*PortNumber*/
            if((PortNumber = mxmlFindElement(SecondNode, SecondNode, "PortNumber", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
            {
                fprintf(stderr, "\nERROR : No <PortNumber> in <Port-Lock> in <Forwarding> in '%s'\n",
                gOptConfigFile);
                return FALSE;
            }
            
            PortCount = 0;
            for(; PortNumber != NULL; PortNumber = mxmlWalkNext(PortNumber, SecondNode, MXML_NO_DESCEND))
            {
                if (PortNumber->type != MXML_ELEMENT)
                {
                    continue;
                }
                
                if(NOT SetConfig_PortNumber(PortNumber, &FWDConfig_t.Port_Lock.LockPort.Port[PortCount].PortNumber, PortCount))
                {
                    fprintf(stderr, "\nERROR : No <PortNumber> in <Port-Lock> in <Forwarding> in '%s'\n",
                    gOptConfigFile);
                    return FALSE;
                }
                for(int i = 0; i < PortCount; i++)
                {
                    if(FWDConfig_t.Port_Lock.LockPort.Port[PortCount].PortNumber == FWDConfig_t.Port_Lock.LockPort.Port[i].PortNumber)
                    {
                        fprintf(stderr, "\nERROR: Duplicate <PortNumber> '%d' in <Port-Lock> in <Forwarding> in '%s'\n",
                        FWDConfig_t.Port_Lock.LockPort.Port[PortCount].PortNumber, gOptConfigFile);
                        return FALSE;                               
                    }
                }
                PortCount++;
            }
            FWDConfig_t.Port_Lock.LockPort.Ports = PortCount;
        }
        else
        {
            if(NOT FWDConfig_t.Port_Lock.LockPort.CPU)
            {
                fprintf(stderr, "\nERROR: Unable to find <CPU> or <Ports> in <Port-Lock> in <Forwarding> '%s'\n",
                gOptConfigFile);
                return FALSE;
            }
        }
        FWDConfig_t.Port_Lock.bEnable = TRUE;
    }


    /*SpanningTreeProtocol*/
    if((FirstNode = mxmlFindElement(ForwardingEngine, ForwardingEngine, "SpanningTreeProtocol", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
    {
        /*Ports*/
        if((SecondNode = mxmlFindElement(FirstNode, FirstNode, "Ports", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
        {
            /*Port*/
            if((ThirdNode = mxmlFindElement(SecondNode, SecondNode, "Port", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
            {
                
            }

            PortCount = 0;
            for(; ThirdNode!= NULL; ThirdNode = mxmlWalkNext(ThirdNode, SecondNode, MXML_NO_DESCEND))
            {
                if (ThirdNode->type != MXML_ELEMENT)
                {
                    continue;
                }
                if((PortNumber = mxmlFindElement(ThirdNode, ThirdNode, "PortNumber", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
                {
                    
                    
                    /*PortNumber*/
                    if(SetConfig_PortNumber(PortNumber, &FWDConfig_t.Spanning_Tree.SPT_Port_Config[PortCount].PortNumber, PortCount))
                    {
                        
                        for(int i = 0; i < PortCount; i++)
                        {
                            if((FWDConfig_t.Spanning_Tree.SPT_Port_Config[PortCount].PortNumber == FWDConfig_t.Spanning_Tree.SPT_Port_Config[i].PortNumber) && (FWDConfig_t.Spanning_Tree.SPT_Port_Config[i].CPU == 0))
                            {
                                fprintf(stderr, "\nERROR: Duplicate <PortNumber> '%d' in <Port> in <Ports> in <SpanningTreeProtocol> in <Forwarding> in '%s'\n",
                                FWDConfig_t.Spanning_Tree.SPT_Port_Config[PortCount].PortNumber, gOptConfigFile);
                                return FALSE;                               
                            }
                        }
                        /*PortState*/
                        if ((ch = SetConfig_GetText(ThirdNode, ThirdNode, "PortState")) != NULL)
                        {
                            if (strncasecmp(ch, "Disabled", 8) == 0)
                            {
                                FWDConfig_t.Spanning_Tree.SPT_Port_Config[PortCount].STP_State = rswitch_Config_SPT_State_Disabled;
                            }
                            else if (strncasecmp(ch, "Blocking", 8) == 0)
                            {
                                FWDConfig_t.Spanning_Tree.SPT_Port_Config[PortCount].STP_State = rswitch_Config_SPT_State_Blocked;
                            }
                            else if (strncasecmp(ch, "LearnandForward", 15) == 0)
                            {
                                FWDConfig_t.Spanning_Tree.SPT_Port_Config[PortCount].STP_State = rswitch_Config_SPT_State_LearnandFrwrd;
                            }
                            else if (strncasecmp(ch, "Learn", 5) == 0)
                            {
                                FWDConfig_t.Spanning_Tree.SPT_Port_Config[PortCount].STP_State = rswitch_Config_SPT_State_Learning;
                            }
                            else if (strncasecmp(ch, "Forward", 7) == 0)
                            {
                                FWDConfig_t.Spanning_Tree.SPT_Port_Config[PortCount].STP_State = rswitch_Config_SPT_State_Forwarding;
                            }
                            else
                            {
                                fprintf(stderr, "\nERROR: Invalid <PortState> in <Port> in <Ports> in <SpanningTreeProtocol> in <Forwarding> in '%s'\n",
                                gOptConfigFile);
                                return FALSE;
                            }
                        }
                        else
                        {
                            fprintf(stderr, "\nERROR: Unable to find <PortState> in <Port> in <Ports> in <SpanningTreeProtocol> in <Forwarding> in '%s'\n",
                            gOptConfigFile);
                            return FALSE;
                        }
                    }
                    PortCount++;
                }
            }
            /*CPU*/
            if(SetConfig_BinaryText(SecondNode, "CPU", &FWDConfig_t.Spanning_Tree.SPT_Port_Config[PortCount].CPU,
                        "Yes", "No", 3, 2))
            {
                /*PortState*/
                if ((ch = SetConfig_GetText(SecondNode, SecondNode, "PortState")) != NULL)
                {
                    if (strncasecmp(ch, "Disabled", 8) == 0)
                    {
                        FWDConfig_t.Spanning_Tree.SPT_Port_Config[PortCount].STP_State = rswitch_Config_SPT_State_Disabled;
                    }
                    else if (strncasecmp(ch, "Blocking", 8) == 0)
                    {
                        FWDConfig_t.Spanning_Tree.SPT_Port_Config[PortCount].STP_State = rswitch_Config_SPT_State_Blocked;
                    }
                    else if (strncasecmp(ch, "LearnandForward", 15) == 0)
                    {
                        FWDConfig_t.Spanning_Tree.SPT_Port_Config[PortCount].STP_State = rswitch_Config_SPT_State_LearnandFrwrd;
                    }
                    else if (strncasecmp(ch, "Learn", 5) == 0)
                    {
                        FWDConfig_t.Spanning_Tree.SPT_Port_Config[PortCount].STP_State = rswitch_Config_SPT_State_Learning;
                    }
                    else if (strncasecmp(ch, "Forward", 7) == 0)
                    {
                        FWDConfig_t.Spanning_Tree.SPT_Port_Config[PortCount].STP_State = rswitch_Config_SPT_State_Forwarding;
                    }
                    else
                    {
                        fprintf(stderr, "\nERROR: Invalid <PortState> in <Ports> in <SpanningTreeProtocol> in <Forwarding> in '%s'\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                }
                else
                {
                    fprintf(stderr, "\nERROR: Unable to find <PortState> in <Ports> in <SpanningTreeProtocol> in <Forwarding> in '%s'\n",
                    gOptConfigFile);
                    return FALSE;
                }
                PortCount++;
            }
            
            FWDConfig_t.Spanning_Tree.Ports = PortCount;
        }
        else
        {
            fprintf(stderr, "\nERROR: Unable to find <Ports> in <SpanningTreeProtocol> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }
    }
    else if (G_Reconfig != TRUE)
    {
        fprintf(stderr, "\nERROR : Unable to find <SpanningTreeProtocol> in <Forwarding> in '%s'\n",
        gOptConfigFile);
        return FALSE;
    }



    /*Migration*/
    if ((FirstNode = mxmlFindElement(ForwardingEngine, ForwardingEngine, "Migration", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
    {
        /*CPU*/
        if(NOT SetConfig_BinaryText(FirstNode, "CPU", &FWDConfig_t.Migration.Migration_Port.CPU,
                    "Yes", "No", 3, 2))
        {
            
        }
        /*Ports*/
        if((SecondNode = mxmlFindElement(FirstNode, FirstNode, "Ports", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
        {
            /*PortNumber*/
            if((PortNumber = mxmlFindElement(SecondNode, SecondNode, "PortNumber", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
            {
                fprintf(stderr, "\nERROR : No <PortNumber> in <Migration> in <Forwarding> in '%s'\n",
                gOptConfigFile);
                return FALSE;
            }
            
            PortCount = 0;
            for(; PortNumber != NULL; PortNumber = mxmlWalkNext(PortNumber, SecondNode, MXML_NO_DESCEND))
            {
                if (PortNumber->type != MXML_ELEMENT)
                {
                    continue;
                }
                
                if(NOT SetConfig_PortNumber(PortNumber, &FWDConfig_t.Migration.Migration_Port.Port[PortCount].PortNumber, PortCount))
                {
                    fprintf(stderr, "\nERROR : Unable to find <PortNumber> in <Migration> in <Forwarding> in '%s'\n",
                    gOptConfigFile);
                    return FALSE;
                }
                for(int i = 0; i < PortCount; i++)
                {
                    if(FWDConfig_t.Migration.Migration_Port.Port[PortCount].PortNumber == FWDConfig_t.Migration.Migration_Port.Port[i].PortNumber)
                    {
                        fprintf(stderr, "\nERROR: Duplicate <PortNumber> '%d' in <Migration> in <Forwarding> in '%s'\n",
                        FWDConfig_t.Migration.Migration_Port.Port[PortCount].PortNumber, gOptConfigFile);
                        return FALSE;                               
                    }
                }
                PortCount++;
            }
            FWDConfig_t.Migration.Migration_Port.Ports = PortCount;
        }
        else
        {
            if(NOT FWDConfig_t.Migration.Migration_Port.CPU)
            {
                fprintf(stderr, "\nERROR: Unable to find <CPU> or <Ports> in <Migration> in <Forwarding> in '%s'\n",
                gOptConfigFile);
                return FALSE;
            }
        }
        FWDConfig_t.Migration.bEnable = TRUE;
    }


    /*Static-Authentication*/
    if ((FirstNode = mxmlFindElement(ForwardingEngine, ForwardingEngine, "Static-Authentication", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
    {
        /*Ports*/
        if((SecondNode = mxmlFindElement(FirstNode, FirstNode, "Ports", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
        {
            /*PortNumber*/
            if((PortNumber = mxmlFindElement(SecondNode, SecondNode, "PortNumber", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
            {
                
            }
            PortCount = 0;
            for(; PortNumber != NULL; PortNumber = mxmlWalkNext(PortNumber, SecondNode, MXML_NO_DESCEND))
            {
                if (PortNumber->type != MXML_ELEMENT)
                {
                    continue;
                }
                
                if(NOT SetConfig_PortNumber(PortNumber, &FWDConfig_t.Static_Auth.Static_Auth_Port.Port[PortCount].PortNumber, PortCount))
                {
                    
                }
                for(int i = 0; i < PortCount; i++)
                {
                    if(FWDConfig_t.Static_Auth.Static_Auth_Port.Port[PortCount].PortNumber == FWDConfig_t.Static_Auth.Static_Auth_Port.Port[i].PortNumber)
                    {
                        fprintf(stderr, "\nERROR: Duplicate <PortNumber> '%d' in <Ports> in <Static-Authentication> in <Forwarding> in '%s'\n",
                        FWDConfig_t.Static_Auth.Static_Auth_Port.Port[PortCount].PortNumber, gOptConfigFile);
                        return FALSE;                               
                    }
                }
                PortCount++;
            }
            FWDConfig_t.Static_Auth.Static_Auth_Port.Ports = PortCount;
        }
        /*CPU*/
        if(NOT SetConfig_BinaryText(FirstNode, "CPU", &FWDConfig_t.Static_Auth.Static_Auth_Port.CPU,
                    "Yes", "No", 3, 2))
        {
            if(NOT PortCount)
            {
                fprintf(stderr, "ERROR : Unable to find <CPU> or <PortNumber> in <Ports> in <Static-Authentication> in <Forwarding> in %s\n",
                gOptConfigFile);
                return FALSE;
            }
        }
        FWDConfig_t.Static_Auth.bEnable = TRUE;
    }
    else if (G_Reconfig != TRUE)
    {
        fprintf(stderr, "\nERROR : Unable to find <Static-Authentication> in <Forwarding> in '%s'\n",
        gOptConfigFile);
        return FALSE;
    }

    
    /*Private-VLAN-Setting*/
    if ((FirstNode = mxmlFindElement(ForwardingEngine, ForwardingEngine, "Private-VLAN-Setting", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
    {
        if ((SecondNode = mxmlFindElement(FirstNode, FirstNode, "Ports", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
        {
            /*Port*/
            if((ThirdNode = mxmlFindElement(SecondNode, SecondNode, "Port", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
            {
                fprintf(stderr, "\nERROR : No <Port> in <Ports> in <Private-VLAN-Setting> in <Forwarding> in '%s'\n",
                gOptConfigFile);
                return FALSE;
            }

            PortCount = 0;
            for(; ThirdNode != NULL; ThirdNode = mxmlWalkNext(ThirdNode, SecondNode, MXML_NO_DESCEND))
            {
                if (ThirdNode->type != MXML_ELEMENT)
                {
                    continue;
                }
                
                /*CPU*/
                if(NOT SetConfig_BinaryText(ThirdNode, "CPU", &FWDConfig_t.Pvt_VLAN_Settings.Port[PortCount].CPU,
                            "Yes", "No", 3, 2))
                {
                    
                }
                if(NOT FWDConfig_t.Pvt_VLAN_Settings.Port[PortCount].CPU)
                {
                    /*PortNumber*/
                    if ((PortNumber = mxmlFindElement(ThirdNode, ThirdNode, "PortNumber", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
                    {
                        fprintf(stderr, "\nERROR : Unable to find <CPU> or <PortNumber> in <Port> in <Ports> in <Private-VLAN-Setting> in <Forwarding> in '%s'\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                    if(NOT SetConfig_PortNumber(PortNumber, &FWDConfig_t.Pvt_VLAN_Settings.Port[PortCount].PortNumber, PortCount))
                    {
                        fprintf(stderr, "\nERROR : Unable to find <PortNumber> in <Port> in <Ports> in <Private-VLAN-Setting> in <Forwarding> in '%s'\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                }
                else
                {
                    /*PortNumber*/
                    if ((PortNumber = mxmlFindElement(ThirdNode, ThirdNode, "PortNumber", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
                    {
                        fprintf(stderr, "\nERROR : Port cannot be both <CPU> and <PortNumber> in <Port> in <Ports> in <Private-VLAN-Setting> in <Forwarding> in '%s'\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                }
                
                /*Private-VLAN*/
                if ((ch = SetConfig_GetText(ThirdNode, ThirdNode, "Private-VLAN")) != NULL)
                {
                    if (strncasecmp(ch, "Isolated", 8) == 0)
                    {
                        FWDConfig_t.Pvt_VLAN_Settings.Port[PortCount].Type = rswitch_Config_fwd_Pvt_VLAN_Isolated;
                    }
                    else if (strncasecmp(ch, "Promiscous", 10) == 0)
                    {
                        FWDConfig_t.Pvt_VLAN_Settings.Port[PortCount].Type = rswitch_Config_fwd_Pvt_VLAN_Promiscous;
                    }
                    else if (strncasecmp(ch, "Community", 9) == 0)
                    {
                        FWDConfig_t.Pvt_VLAN_Settings.Port[PortCount].Type = rswitch_Config_fwd_Pvt_VLAN_Community;
                    }
                    else
                    {
                        fprintf(stderr, "\nERROR: Invalid <Private-VLAN> in <Port> in <Ports> in <Private-VLAN-Setting> in <Forwarding> in '%s'\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                }
                else
                {
                    fprintf(stderr, "\nERROR: Unable to find <Private-VLAN> in <Port> in <Ports> in <Private-VLAN-Setting> in <Forwarding> in '%s'\n",
                    gOptConfigFile);
                    return FALSE;
                }

                if(FWDConfig_t.Pvt_VLAN_Settings.Port[PortCount].Type == rswitch_Config_fwd_Pvt_VLAN_Community)
                {
                    /*Communities*/
                    if((FourthNode = mxmlFindElement(ThirdNode, ThirdNode, "Communities", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
                    {
                        /*Community*/
                        if((FifthNode = mxmlFindElement(FourthNode, FourthNode, "Community", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
                        {
                            fprintf(stderr, "\nERROR : No <Community> in <Communities> in <Port> in <Ports> in <Private-VLAN-Setting> in <Forwarding> in '%s'\n",
                            gOptConfigFile);
                            return FALSE;
                        }
                        
                        CommunityCount = 0;
                        for(; FifthNode != NULL; FifthNode = mxmlWalkNext(FifthNode, FourthNode, MXML_DESCEND))
                        {
                            if (FifthNode->type != MXML_ELEMENT)
                            {
                                continue;
                            }
                            
                            if (strcasecmp("Community", FifthNode->value.element.name) == 0)
                            {
                                if ((value = mxmlGetFirstChild(FifthNode)) == NULL)
                                {
                                    fprintf(stderr, "\n No <Community>");
                                    return FALSE;
                                }
                                if (value->type != MXML_TEXT)
                                {
                                    fprintf(stderr, "\n Invalid  <Community>");
                                    return FALSE;
                                    
                                }
                                
                                CommunityChar[CommunityCount] = value->value.text.string;
                                
                                if (sscanf(CommunityChar[CommunityCount], "%u", &CommunityInt) != 1)
                                {
                                    fprintf(stderr, "\n Invalid  <Community>");
                                    return FALSE;
                                }
                                if (CommunityInt >= RENESAS_RSWITCH_MAX_VLAN_COMMUNITIES)
                                {
                                    fprintf(stderr, "\nInvalid  <Community>, Range (0-%d)\n", RENESAS_RSWITCH_MAX_VLAN_COMMUNITIES);
                                    return FALSE;
                                }
                                FWDConfig_t.Pvt_VLAN_Settings.Port[PortCount].Pvt_VLAN_Community[CommunityCount].Community_ID = CommunityInt;
                            }
                            else
                            {
                                fprintf(stderr, "\nERROR: Unable to find <Community> in <Communities> in <Port> in <Ports> in <Private-VLAN-Setting> in <Forwarding> in '%s'\n",
                                gOptConfigFile);
                                return FALSE;
                            }
                            for(int i = 0; i < CommunityCount; i++)
                            {
                                if(FWDConfig_t.Pvt_VLAN_Settings.Port[PortCount].Pvt_VLAN_Community[CommunityCount].Community_ID == FWDConfig_t.Pvt_VLAN_Settings.Port[PortCount].Pvt_VLAN_Community[i].Community_ID)
                                {
                                    fprintf(stderr, "\nERROR: Duplicate <Community> '%d' in <Communities> in <Port> in <Ports> in <Private-VLAN-Setting> in <Forwarding> in '%s'\n",
                                    FWDConfig_t.Pvt_VLAN_Settings.Port[PortCount].Pvt_VLAN_Community[CommunityCount].Community_ID, gOptConfigFile);
                                    return FALSE;                               
                                }
                            }
                            CommunityCount++;
                        }
                        FWDConfig_t.Pvt_VLAN_Settings.Port[PortCount].Pvt_VLAN_Communities = CommunityCount;
                    }
                    else
                    {
                        fprintf(stderr, "\nERROR: Unable to find <Communities> in <Port> in <Ports> in <Private-VLAN-Setting> in <Forwarding> in '%s'\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                }
                for(int i = 0; i < PortCount; i++)
                {
                    if(FWDConfig_t.Pvt_VLAN_Settings.Port[PortCount].PortNumber == FWDConfig_t.Pvt_VLAN_Settings.Port[i].PortNumber)
                    {
                        if(NOT FWDConfig_t.Pvt_VLAN_Settings.Port[PortCount].PortNumber)
                        {
                            fprintf(stderr, "\nERROR: Duplicate <CPU> entry in <Port> in <Ports> in <Private-VLAN-Setting> in <Forwarding> in '%s'\n",
                            gOptConfigFile);
                            return FALSE;                               
                        }
                        fprintf(stderr, "\nERROR: Duplicate <PortNumber> '%d' in <Port> in <Ports> in <Private-VLAN-Setting> in <Forwarding> in '%s'\n",
                        FWDConfig_t.Pvt_VLAN_Settings.Port[PortCount].PortNumber, gOptConfigFile);
                        return FALSE;                               
                    }
                }

                PortCount++;
            }
            FWDConfig_t.Pvt_VLAN_Settings.Ports = PortCount;
        }
        else
        {
            fprintf(stderr, "\nERROR: Unable to find <Ports> in <Private-VLAN-Setting> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }
    }


    /*FWD-Search-Config*/
    if ((FirstNode = mxmlFindElement(ForwardingEngine, ForwardingEngine, "FWD-Search-Config", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
    {
        /*QueuePriority*/
        if(NOT SetConfig_BinaryText(FirstNode, "QueuePriority", &FWDConfig_t.Search_Config.Queue_Priority,
                    "PCP", "IPV", 3, 3))
        {
            fprintf(stderr, "\nERROR: Unable to find <QueuePriority> in <FWDSearchConfig> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }
        /*QCI-MAC*/
        if(NOT SetConfig_BinaryText(FirstNode, "QCI-MAC", &FWDConfig_t.Search_Config.QCIMAC,
                    "Include", "Exclude", 7, 7))
        {
            fprintf(stderr, "\nERROR: Unable to find <QCI-MAC> in <FWDSearchConfig> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }
        /*QCI-VLAN*/
        if(NOT SetConfig_BinaryText(FirstNode, "QCI-VLAN", &FWDConfig_t.Search_Config.QCIVLAN,
                    "Include", "Exclude", 7, 7))
        {
            fprintf(stderr, "\nERROR: Unable to find <QCI-VLAN> in <FWDSearchConfig> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }
        /*QCI-PCP*/
        if(NOT SetConfig_BinaryText(FirstNode, "QCI-PCP", &FWDConfig_t.Search_Config.QCIPCP,
                    "Include", "Exclude", 7, 7))
        {
            fprintf(stderr, "\nERROR: Unable to find <QCI-PCP> in <FWDSearchConfig> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }
        /*QCI-DEI*/
        if(NOT SetConfig_BinaryText(FirstNode, "QCI-DEI", &FWDConfig_t.Search_Config.QCIDEI,
                    "Include", "Exclude", 7, 7))
        {
            fprintf(stderr, "\nERROR: Unable to find <QCI-DEI> in <FWDSearchConfig> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }
        /*QCI-SPN*/
        if(NOT SetConfig_BinaryText(FirstNode, "QCI-SPN", &FWDConfig_t.Search_Config.QCISPN,
                    "Include", "Exclude", 7, 7))
        {
            fprintf(stderr, "\nERROR: Unable to find <QCI-SPN> in <FWDSearchConfig> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }
        /*QCI-MAC-Select*/
        if(NOT SetConfig_BinaryText(FirstNode, "QCI-MAC-Select", &FWDConfig_t.Search_Config.QCI_MAC_Select,
                    "Dest", "Source", 4, 6))
        {
            fprintf(stderr, "\nERROR: Unable to find <QCI-MAC-Select> in <FWDSearchConfig> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }
        /*QCI-Filtering*/
        if(NOT SetConfig_BinaryText(FirstNode, "QCI-Filtering", &FWDConfig_t.Search_Config.QCI_Filtering,
                    "Enable", "Disable", 6, 7))
        {
            fprintf(stderr, "\nERROR: Unable to find <QCI-Filtering> in <FWDSearchConfig> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }
        /*Stream-ID-Unique-Number-Select*/
        if(NOT SetConfig_BinaryText(FirstNode, "Stream-ID-Unique-Number-Select", &FWDConfig_t.Search_Config.SID_UniqueNum_Select,
                    "MAC", "VLAN", 3, 4))
        {
            fprintf(stderr, "\nERROR: Unable to find <Stream-ID-Unique-Number-Select> in <FWDSearchConfig> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }
        
        /*Stream-ID-MAC-Select*/
        if(NOT SetConfig_BinaryText(FirstNode, "Stream-ID-MAC-Select", &FWDConfig_t.Search_Config.SID_MAC_Select,
                    "Dest", "Source", 4, 6))
        {
            fprintf(stderr, "\nERROR: Unable to find <Stream-ID-MAC-Select> in <FWDSearchConfig> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }
        /*Stream-ID-Filter-Select*/
        if(NOT SetConfig_BinaryText(FirstNode, "Stream-ID-Filter-Select", &FWDConfig_t.Search_Config.SID_Filter_Select,
                    "Black", "White", 5, 5))
        {
            fprintf(stderr, "\nERROR: Unable to find <Stream-ID-Filter-Select> in <FWDSearchConfig> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }
        /*Stream-ID-Filter*/
        if(NOT SetConfig_BinaryText(FirstNode, "Stream-ID-Filter", &FWDConfig_t.Search_Config.SID_Filter,
                    "Active", "Inactive", 6, 8))
        {
            fprintf(stderr, "\nERROR: Unable to find <Stream-ID-Filter> in <FWDSearchConfig> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }
        /*Stream-ID-Tbl*/
        if(NOT SetConfig_BinaryText(FirstNode, "Stream-ID-Tbl", &FWDConfig_t.Search_Config.SID_Tbl,
                    "Active", "Inactive", 6, 8))
        {
            fprintf(stderr, "\nERROR: Unable to find <Stream-ID-Tbl> in <FWDSearchConfig> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }
        /*SrcPortFilter*/
        if(NOT SetConfig_BinaryText(FirstNode, "SrcPortFilter", &FWDConfig_t.Search_Config.Src_Port_Filter,
                    "Active", "Inactive", 6, 8))
        {
            fprintf(stderr, "\nERROR: Unable to find <SrcPortFilter> in <FWDSearchConfig> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }

        /*MAC-Filter-Select*/
        if(NOT SetConfig_BinaryText(FirstNode, "MAC-Filter-Select", &FWDConfig_t.Search_Config.MAC_Filter_Select,
                    "Black", "White", 5, 5))
        {
            fprintf(stderr, "\nERROR: Unable to find <MAC-Filter-Select> in <FWDSearchConfig> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }

        /*MAC-Filter*/
        if(NOT SetConfig_BinaryText(FirstNode, "MAC-Filter", &FWDConfig_t.Search_Config.MAC_Filter,
                    "Active", "Inactive", 6, 8))
        {
            fprintf(stderr, "\nERROR: Unable to find <MAC-Filter> in <FWDSearchConfig> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }
        /*MAC-Tbl*/
        if(NOT SetConfig_BinaryText(FirstNode, "MAC-Tbl", &FWDConfig_t.Search_Config.MAC_Tbl,
                    "Active", "Inactive", 6, 8))
        {
            fprintf(stderr, "\nERROR: Unable to find <MAC-Tbl> in <FWDSearchConfig> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }
        /*VLAN-MAC-Priority*/
        if ((ch = SetConfig_GetText(FirstNode, FirstNode, "VLAN-MAC-Priority")) != NULL)
        {
            if (strncasecmp(ch, "MAC", 3) == 0)
            {
                FWDConfig_t.Search_Config.MAC_VLAN_Priority = rswitch_Config_Priority_MAC;
            }
            else if (strncasecmp(ch, "VLAN", 4) == 0)
            {
                FWDConfig_t.Search_Config.MAC_VLAN_Priority = rswitch_Config_Priority_VLAN;
            }
            else if (strncasecmp(ch, "Highest", 7) == 0)
            {
                FWDConfig_t.Search_Config.MAC_VLAN_Priority = rswitch_Config_Priority_Highest;
            }
            else if (strncasecmp(ch, "Lowest", 6) == 0)
            {
                FWDConfig_t.Search_Config.MAC_VLAN_Priority = rswitch_Config_Priority_Lowest;
            }
            else
            {
                fprintf(stderr, "\nERROR: Invalid <VLAN-MAC-Priority> in <FWDSearchConfig> in <Forwarding> in '%s'\n",
                gOptConfigFile);
                return FALSE;
            }
        }
        else
        {
            fprintf(stderr, "\nERROR: Unable to find <VLAN-MAC-Priority> in <FWDSearchConfig> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }
        /*VLAN-MAC-Fwding*/
        if ((ch = SetConfig_GetText(FirstNode, FirstNode, "VLAN-MAC-Fwding")) != NULL)
        {
            if (strncasecmp(ch, "VLAN_OR_MAC", 11) == 0)
            {
                FWDConfig_t.Search_Config.VLAN_MAC_Fwd = rswitch_Fwd_VLANorMAC;
            }
            else if (strncasecmp(ch, "VLAN_AND_MAC", 12) == 0)
            {
                FWDConfig_t.Search_Config.VLAN_MAC_Fwd = rswitch_Fwd_VLANandMAC;
            }
            else if (strncasecmp(ch, "VLAN", 4) == 0)
            {
                FWDConfig_t.Search_Config.VLAN_MAC_Fwd = rswitch_Fwd_VLAN;
            }
            else if (strncasecmp(ch, "MAC", 3) == 0)
            {
                FWDConfig_t.Search_Config.VLAN_MAC_Fwd = rswitch_Fwd_MAC;
            }
            else
            {
                fprintf(stderr, "\nERROR: Invalid <VLAN-MAC-Fwding> in <FWDSearchConfig> in <Forwarding> in '%s'\n",
                gOptConfigFile);
                return FALSE;
            }
        }
        else
        {
            fprintf(stderr, "\nERROR: Unable to find <VLAN-MAC-Fwding> in <FWDSearchConfig> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }
        /*VLAN-MAC-CSDN*/
        if(NOT SetConfig_BinaryText(FirstNode, "VLAN-MAC-CSDN", &FWDConfig_t.Search_Config.VLAN_MAC_CSDN,
                    "MAC", "VLAN", 3, 4))
        {
            fprintf(stderr, "\nERROR: Unable to find <VLAN-MAC-CSDN> in <FWDSearchConfig> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }
        /*VLAN-Filter-Select*/
        if(NOT SetConfig_BinaryText(FirstNode, "VLAN-Filter-Select", &FWDConfig_t.Search_Config.VLAN_Filter_Select,
                    "Black", "White", 5, 5))
        {
            fprintf(stderr, "\nERROR: Unable to find <VLAN-Filter-Select> in <FWDSearchConfig> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }
        /*VLAN-Filter*/
        if(NOT SetConfig_BinaryText(FirstNode, "VLAN-Filter", &FWDConfig_t.Search_Config.VLAN_Filter,
                    "Active", "Inactive", 6, 8))
        {
            fprintf(stderr, "\nERROR: Unable to find <VLAN-Filter> in <FWDSearchConfig> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }
        /*VLAN-Tbl*/
        if(NOT SetConfig_BinaryText(FirstNode, "VLAN-Tbl", &FWDConfig_t.Search_Config.VLAN_Tbl,
                    "Active", "Inactive", 6, 8))
        {
            fprintf(stderr, "\nERROR: Unable to find <VLAN-Tbl> in <FWDSearchConfig> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }
        FWDConfig_t.Search_Config.bEnable = TRUE;
    }
    else if (G_Reconfig != TRUE)
    {
        fprintf(stderr, "\nERROR : Unable to find <FWDSearchConfig> in <Forwarding> in '%s'\n",
        gOptConfigFile);
        return FALSE;
    }


    /*Insert-MAC-entries*/
    if ((FirstNode = mxmlFindElement(ForwardingEngine, ForwardingEngine, "Insert-MAC-entries", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
    {
        /*MAC-Entry*/
        if((SecondNode = mxmlFindElement(FirstNode, FirstNode, "MAC-Entry", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
        {
            fprintf(stderr, "\nERROR : No <MAC-Entry> in <Insert-MAC-entries> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }
        else
        {
            Count = 0;
            for(; SecondNode != NULL; SecondNode = mxmlWalkNext(SecondNode, FirstNode, MXML_NO_DESCEND))
            {
                if (SecondNode->type != MXML_ELEMENT)
                {
                    continue;
                }
                
                if(Count >= RENESAS_RSWITCH_L2_FWD_ENTRIES)
                {
                    fprintf(stderr, "ERROR : A maximum of %d <MAC-Entry> entries allowed in <Insert-MAC-entries> in <Forwarding> in %s\n",
                    RENESAS_RSWITCH_L2_FWD_ENTRIES, gOptConfigFile);
                    return FALSE;
                }

                /*MAC*/
                if (NOT SetConfig_MAC(SecondNode, FWDConfig_t.Insert_L2_Config.L2Fwd[Count].MAC, "MAC", Count))
                {
                    fprintf(stderr, "\nERROR : Unable to find <MAC> in <MAC-Entry> in <Insert-MAC-entries> in <Forwarding> in '%s'\n",
                    gOptConfigFile);
                    return FALSE;
                }
                /*PortNumbers*/
                if ((ThirdNode = mxmlFindElement(SecondNode, SecondNode, "PortNumbers", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
                {
                    if((PortNumber = mxmlFindElement(ThirdNode, ThirdNode, "PortNumber", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
                    {
                        
                    }

                    PortCount = 0;
                    for(; PortNumber != NULL; PortNumber = mxmlWalkNext(PortNumber, ThirdNode, MXML_NO_DESCEND))
                    {
                        if (PortNumber->type != MXML_ELEMENT)
                        {
                            continue;
                        }

                        if(NOT SetConfig_PortNumber(PortNumber, &FWDConfig_t.Insert_L2_Config.L2Fwd[Count].Destination.DestEth[PortCount].PortNumber, PortCount))
                        {
                            
                        }
                        PortCount++;
                    }
                    FWDConfig_t.Insert_L2_Config.L2Fwd[Count].Destination.DestEths = PortCount;
                }
                

                /*CPU*/
                /*CPU*/
                if(NOT SetConfig_BinaryText(SecondNode, "CPU", &FWDConfig_t.Insert_L2_Config.L2Fwd[Count].Destination.CPU_Enable,
                            "Yes", "No", 3, 2))
                {
                    if(NOT FWDConfig_t.Insert_L2_Config.L2Fwd[Count].Destination.DestEths)
                    {
                        fprintf(stderr, "\nERROR: Unable to find <CPU> or <PortNumber> in <MAC-Entry> in <Insert-MAC-entries> in <Forwarding> in '%s'\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                }
                /*Dynamic*/
                if(NOT SetConfig_BinaryText(SecondNode, "Dynamic", &FWDConfig_t.Insert_L2_Config.L2Fwd[Count].Destination.Dynamic,
                            "Enable", "Disable", 6, 7))
                {
                    
                }
                /*EthMirror*/
                if(NOT SetConfig_BinaryText(SecondNode, "EthMirror", &FWDConfig_t.Insert_L2_Config.L2Fwd[Count].Destination.EthMirror,
                            "Enable", "Disable", 6, 7))
                {
                    
                }

                /*CPUMirror*/
                if(NOT SetConfig_BinaryText(SecondNode, "CPUMirror", &FWDConfig_t.Insert_L2_Config.L2Fwd[Count].Destination.CPUMirror,
                            "Enable", "Disable", 6, 7))
                {
                    
                }

                /*PCPUpdate*/
                if(NOT SetConfig_BinaryText(SecondNode, "PCPUpdate", &FWDConfig_t.Insert_L2_Config.L2Fwd[Count].Destination.PCPUpdate,
                            "Enable", "Disable", 6, 7))
                {
                    
                }

                /*ListFilter*/
                if(NOT SetConfig_BinaryText(SecondNode, "ListFilter", &FWDConfig_t.Insert_L2_Config.L2Fwd[Count].Destination.ListFilter,
                            "Enable", "Disable", 6, 7))
                {
                    
                }

                /*PCP*/
                if(NOT SetConfig_Integer(SecondNode, "PCP",
                            &FWDConfig_t.Insert_L2_Config.L2Fwd[Count].Destination.PCP, RENESAS_RSWITCH_VLAN_PCP_VALUES))
                {
                    
                }

                /*CSDN*/
                if(NOT SetConfig_Integer(SecondNode, "CSDN",
                            &FWDConfig_t.Insert_L2_Config.L2Fwd[Count].Destination.CSDN, MAX_CSDN))
                {
                    
                }
                Count++;
            }
            FWDConfig_t.Insert_L2_Config.L2FwdEntries = Count;
        }
    }
    else if (G_Reconfig != TRUE)
    {
        fprintf(stderr, "\nERROR : Unable to find <Insert-MAC-entries> in <Forwarding> in '%s'\n",
        gOptConfigFile);
        return FALSE;
    }

    /*Insert-VLAN-entries*/
    if ((FirstNode = mxmlFindElement(ForwardingEngine, ForwardingEngine, "Insert-VLAN-entries", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
    {
        /*VLAN-Entry*/
        if((SecondNode = mxmlFindElement(FirstNode, FirstNode, "VLAN-Entry", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
        {
            fprintf(stderr, "\nERROR : No <VLAN-Entry> in <Insert-VLAN-entries> in <Forwarding> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }
        
        Count = 0;
        for(; SecondNode != NULL; SecondNode = mxmlWalkNext(SecondNode, FirstNode, MXML_NO_DESCEND))
        {
            if (SecondNode->type != MXML_ELEMENT)
            {
                continue;
            }
            if(Count >= RENESAS_RSWITCH_VLAN_FWD_ENTRIES)
            {
                fprintf(stderr, "ERROR : A maximum of %d <VLAN-Entry> entries allowed in <Insert-VLAN-entries> in <Forwarding> in %s\n",
                RENESAS_RSWITCH_VLAN_FWD_ENTRIES, gOptConfigFile);
                return FALSE;
            }
            
            /*VLAN*/
            if(NOT SetConfig_Integer(SecondNode, "VLAN",
                        &FWDConfig_t.Insert_VLAN_Config.VLANFwd[Count].VLAN_ID, RENESAS_RSWITCH_MAX_VLAN_ID))
            {
                fprintf(stderr, "\nERROR : Unable to find <VLAN> in <VLAN-Entry> in <Insert-VLAN-entries> in <Forwarding> in %s\n",
                gOptConfigFile);
                return FALSE;
            }
            for(int i = 0; i < Count; i++)
            {
                if(FWDConfig_t.Insert_VLAN_Config.VLANFwd[Count].VLAN_ID == FWDConfig_t.Insert_VLAN_Config.VLANFwd[i].VLAN_ID)
                {
                    fprintf(stderr, "\nERROR: Duplicate <VLAN> '%d' in <VLAN-Entry> in <Insert-VLAN-entries> in <Forwarding> in '%s'\n",
                    FWDConfig_t.Insert_VLAN_Config.VLANFwd[Count].VLAN_ID, gOptConfigFile);
                    return FALSE;                               
                }
            }
            

            /*VLAN-Filter*/
            if((ThirdNode = mxmlFindElement(SecondNode, SecondNode, "VLAN-Filter", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
            {
                /*CPU*/
                if(NOT SetConfig_BinaryText(ThirdNode, "CPU", &FWDConfig_t.Insert_VLAN_Config.VLANFwd[Count].VLAN_Filter.CPU_Enable,
                            "Yes", "No", 3, 2))
                {
                    
                }
                /*VLAN-Filter-Ports*/
                if ((FourthNode = mxmlFindElement(ThirdNode, ThirdNode, "VLAN-Filter-Ports", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
                {
                    if((PortNumber = mxmlFindElement(FourthNode, FourthNode, "PortNumber", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
                    {
                        fprintf(stderr, "\nERROR : No <PortNumber> in <VLAN-Filter-Ports> in <VLAN-Filter-Ports> in <VLAN-Filter> in <VLAN-Entry> in <Insert-VLAN-entries> in <Forwarding> in '%s'\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                    
                    PortCount = 0;
                    for(; PortNumber != NULL; PortNumber = mxmlWalkNext(PortNumber, FourthNode, MXML_NO_DESCEND))
                    {
                        if (PortNumber->type != MXML_ELEMENT)
                        {
                            continue;
                        }
                        
                        if(NOT SetConfig_PortNumber(PortNumber, &FWDConfig_t.Insert_VLAN_Config.VLANFwd[Count].VLAN_Filter.Port[PortCount].PortNumber, PortCount))
                        {
                            
                        }
                        for(int i = 0; i < PortCount; i++)
                        {
                            if(FWDConfig_t.Insert_VLAN_Config.VLANFwd[Count].VLAN_Filter.Port[PortCount].PortNumber == FWDConfig_t.Insert_VLAN_Config.VLANFwd[Count].VLAN_Filter.Port[i].PortNumber)
                            {
                                fprintf(stderr, "\nERROR: Duplicate <PortNumber> '%d' in <VLAN-Filter-Ports> in <VLAN-Filter-Ports> in <VLAN-Filter> in <VLAN-Entry> in <Insert-VLAN-entries> in <Forwarding> in '%s'\n",
                                FWDConfig_t.Insert_VLAN_Config.VLANFwd[Count].VLAN_Filter.Port[PortCount].PortNumber, gOptConfigFile);
                                return FALSE;                               
                            }
                        }
                        PortCount++;
                    }
                    FWDConfig_t.Insert_VLAN_Config.VLANFwd[Count].VLAN_Filter.Ports = PortCount;
                }
                else
                {
                    if(NOT FWDConfig_t.Insert_VLAN_Config.VLANFwd[Count].VLAN_Filter.CPU_Enable)
                    {
                        fprintf(stderr, "\nERROR: Unable to find <CPU> or <VLAN-Filter-Ports> in <VLAN-Filter-Ports> in <VLAN-Filter> in <VLAN-Entry> in <Insert-VLAN-entries> in <Forwarding> in '%s'\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                }
                
            }
            else
            {
                fprintf(stderr, "\nERROR: Unable to find <VLAN-Filter> in <VLAN-Filter-Ports> in <VLAN-Filter-Ports> in <VLAN-Filter> in <VLAN-Entry> in <Insert-VLAN-entries> in <Forwarding> in '%s'\n",
                gOptConfigFile);
                return FALSE;
            }
            
            /*VLAN-Routing*/
            if((ThirdNode = mxmlFindElement(SecondNode, SecondNode, "VLAN-Routing", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
            {
                
                if(NOT SetConfig_BinaryText(ThirdNode, "CPU", &FWDConfig_t.Insert_VLAN_Config.VLANFwd[Count].VLAN_Routing.CPU_Enable,
                            "Yes", "No", 3, 2))
                {
                    
                }
                /*VLAN-Routing-Ports*/
                if ((FourthNode = mxmlFindElement(ThirdNode, ThirdNode, "VLAN-Routing-Ports", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
                {
                    if((PortNumber = mxmlFindElement(FourthNode, FourthNode, "PortNumber", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
                    {
                        fprintf(stderr, "\nERROR : No <PortNumber> in <VLAN-Routing-Ports> in <VLAN-Routing-Ports> in <VLAN-Routing> in <VLAN-Entry> in <Insert-VLAN-entries> in <Forwarding> in '%s'\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                    
                    PortCount = 0;
                    for(; PortNumber != NULL; PortNumber = mxmlWalkNext(PortNumber, FourthNode, MXML_NO_DESCEND))
                    {
                        if (PortNumber->type != MXML_ELEMENT)
                        {
                            continue;
                        }
                        
                        if(NOT SetConfig_PortNumber(PortNumber, &FWDConfig_t.Insert_VLAN_Config.VLANFwd[Count].VLAN_Routing.Port[PortCount].PortNumber, PortCount))
                        {
                            fprintf(stderr, "\nERROR: Unable to find <PortNumber> in <VLAN-Routing-Ports> in <VLAN-Routing-Ports> in <VLAN-Routing> in <VLAN-Entry> in <Insert-VLAN-entries> in <Forwarding> in '%s'\n",
                            gOptConfigFile);
                            return FALSE;
                        }
                        for(int i = 0; i < PortCount; i++)
                        {
                            if(FWDConfig_t.Insert_VLAN_Config.VLANFwd[Count].VLAN_Routing.Port[PortCount].PortNumber == FWDConfig_t.Insert_VLAN_Config.VLANFwd[Count].VLAN_Routing.Port[i].PortNumber)
                            {
                                fprintf(stderr, "\nERROR: Duplicate <PortNumber> '%d' in <VLAN-Routing-Ports> in <VLAN-Routing-Ports> in <VLAN-Routing> in <VLAN-Entry> in <Insert-VLAN-entries> in <Forwarding> in '%s'\n",
                                FWDConfig_t.Insert_VLAN_Config.VLANFwd[Count].VLAN_Routing.Port[PortCount].PortNumber, gOptConfigFile);
                                return FALSE;                               
                            }
                        }
                        PortCount++;
                    }
                    FWDConfig_t.Insert_VLAN_Config.VLANFwd[Count].VLAN_Routing.Ports = PortCount;
                    
                }
                else
                {
                    if(NOT FWDConfig_t.Insert_VLAN_Config.VLANFwd[Count].VLAN_Routing.CPU_Enable)
                    {
                        fprintf(stderr, "\nERROR: Unable to find <CPU> or <VLAN-Routing-Ports> in <VLAN-Routing> in <VLAN-Entry> in <Insert-VLAN-entries> in <Forwarding> in '%s'\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                }
            }
            else
            {
                fprintf(stderr, "\nERROR: Unable to find <VLAN-Routing> in <VLAN-Entry> in <Insert-VLAN-entries> in <Forwarding> in '%s'\n",
                gOptConfigFile);
                return FALSE;
            }
            
            /*EthMirror*/
            if(NOT SetConfig_BinaryText(SecondNode, "EthMirror", &FWDConfig_t.Insert_VLAN_Config.VLANFwd[Count].EthMirror,
                        "Enable", "Disable", 6, 7))
            {
                
            }
            
            /*CPUMirror*/
            if(NOT SetConfig_BinaryText(SecondNode, "CPUMirror", &FWDConfig_t.Insert_VLAN_Config.VLANFwd[Count].CPUMirror,
                        "Enable", "Disable", 6, 7))
            {
                
            }
            
            /*PCPUpdate*/
            if(NOT SetConfig_BinaryText(SecondNode, "PCPUpdate", &FWDConfig_t.Insert_VLAN_Config.VLANFwd[Count].PCPUpdate,
                        "Enable", "Disable", 6, 7))
            {
                
            }
            
            /*PCP*/
            if(NOT SetConfig_Integer(SecondNode, "PCP",
                        &FWDConfig_t.Insert_VLAN_Config.VLANFwd[Count].PCP, RENESAS_RSWITCH_VLAN_PCP_VALUES))
            {
                
            }
            
            /*CSDN*/
            if(NOT SetConfig_Integer(SecondNode, "CSDN",
                        &FWDConfig_t.Insert_VLAN_Config.VLANFwd[Count].CSDN, MAX_CSDN))
            {
                
            }
            Count++;
        }
        FWDConfig_t.Insert_VLAN_Config.VLANFwdEntries = Count;
    }

    

    return TRUE;
}

/*
* FWD_Set_Config : Function called by rswitch tool to update the FWD Configuration
* Tree           : TSN XML Tree
*/
extern bool FWD_Set_Config(mxml_node_t * Tree)
{
    int ret = 0;
    if (gFwdFd != -1)
    {
        if ((ret = ioctl(gFwdFd, RSWITCH_FWD_GET_CONFIG, &FWDConfig_t)) != 0)   
        {
            fprintf(stderr, "\nERROR : RSWITCH_FWD_GET_CONFIG failed (%d) : %s\n", ret, strerror(errno));
            return FALSE;
        }
        G_Reconfig = FWDConfig_t.ConfigDone;
        printf("Value of G_Reconfig=%d \n", G_Reconfig);
    }
    else
    {
        fprintf(stderr, "WARNING, Omitting RSWITCH_FWD_GET_CONFIG as module not open\n");
    }
    if (NOT Set_Config(Tree, "Forwarding"))
    {
        return FALSE;
    }
    return TRUE;
}

/*
*   FWD_Open_Driver : Function to open the Forwarding Engine Driver File descriptor
*/

extern bool FWD_Open_Driver(void)
{
    gFwdFd = open(SWITCH_AVB_FWD_DEV_NAME, O_RDWR | O_SYNC); 

    if (gFwdFd < 0)
    {
        fprintf(stderr, "\n ERROR : FWD Open '%s' failed : %s \n", SWITCH_AVB_FWD_DEV_NAME, strerror(errno));
        return FALSE;
    }

    return TRUE;
}

/*
*   FWD_Close_Driver : Function to close the Forwarding Engine Driver File descriptor
*/
extern bool FWD_Close_Driver(void)
{
    if (gFwdFd != -1)
    {
        close(gFwdFd);
        gFwdFd = -1;
    }
    return TRUE;
}

/*
* Change History
* 2019-04-08   BT   R-Switch tool initial development and release
* 2019-12-02   AK  Updated Only for VLAN and L2 Update
/*
* Local variables:
* Mode: C
* tab-width: 4
* indent-tabs-mode: nil
* c-basic-offset: 4
* End:
* vim: ts=4 expandtab sw=4
*/
