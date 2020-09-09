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
#include "rswitch_TSN.h"
#include <math.h>
#include <inttypes.h>

#include <drivers/net/ethernet/renesas/rswitch_eth/rswitch_eth.h> 
#define CBS_DEBUG
#define  DEFAULT_REPORT_PRINT '-'

extern char          * gOptConfigFile;
long double     SystemFreq = 0.0;
struct rswitch_Config   TSNConfig_t;


uint32_t        ListTotal = 0;
static uint32_t        gTSN_XML_Port;
static int             gEthFd          = -1;
static uint32_t        gXML_Txqueue    =  0;
static uint32_t        gXML_Txstream    =  0;
static uint32_t        gXML_Stream     =  0;

static uint32_t        maxInterferenceSize[RENESAS_RSWITCH_TX_QUEUES]  = {0,0,0,0,0,0,0,0};
uint8_t                gMAC[6];
char                   gMACChar0[32];
char                   gMACChar1[32];
char                   gMACChar2[32];
char                   gMACChar3[32];
char                   gMACChar4[32];
char                   gMACChar5[32];
uint32_t               gXML_GateControl = 0;
uint32_t               gXML_GateQueue = 0;
uint32_t               gXML_NullStreamEntryCount = 0;
uint32_t               gXML_ActiveStreamIngEntryCount = 0;
uint32_t               gXML_ActiveStreamEgEntryCount = 0;
uint32_t               gNullStreamEntryCountPerPort = 0;
uint64_t               CycleTime[RENESAS_RSWITCH_MAX_PORTS][RENESAS_RSWITCH_TX_QUEUES];
uint32_t               PortTransmitRate[RENESAS_RSWITCH_MAX_PORTS];
/*
* TSN_Print_Configuration() : Prints the configuration report of TSN
*
*/
void TSN_Print_Configuration()
{
    int count     = 0;
    int Count2  = 0;
    int Count3  = 0;
    int queuenum  = 0;
    int Streamnum = 0;
    

    printf("\n======================================= FRER Global Configuration ======================================\n");

    
    printf("\n======================================= ACTIVE TSN PORTS ======================================\n");
    printf("SYSTEM CLOCK FREQUENCY IN MHZ : %LF\n", SystemFreq);
    for (count = 0; count < TSNConfig_t.Ports; count++)
    {
        printf("TSN Port Number : %32d\n", TSNConfig_t.Port[count].PortNumber);

        
        if(TSNConfig_t.Port[count].TxParam.TxStreams)
        {
            printf("Tx  QueueNumber                  BandwidthFraction                   portTransmitRate\n");
            for(Streamnum = 0; Streamnum < TSNConfig_t.Port[count].TxParam.TxStreams; Streamnum++)
            {
                printf("    %9d %34d %29d Mbps\n", TSNConfig_t.Port[count].TxParam.TxStream[Streamnum].QueueNum, TSNConfig_t.Port[count].TxParam.TxStream[Streamnum].BandwidthFraction, TSNConfig_t.Port[count].TxParam.TxStream[Streamnum].portTransmitRate/1000000);
            }
        }

        if(TSNConfig_t.Port[count].TxParam.TxQueues)
        {
            printf("Tx  Queue   MinimumFragSize  MACSelect  \n");
            for (queuenum = 0; queuenum < TSNConfig_t.Port[count].TxParam.TxQueues; queuenum++)
            {
                printf("    %5d %14d %10s \n", TSNConfig_t.Port[count].TxParam.TxQueue[queuenum].QueueNumber,
                (64*(1+TSNConfig_t.Port[count].PminSize))-4,
                (TSNConfig_t.Port[count].TxParam.TxQueue[queuenum].Pre_Empt_MAC ? "pMAC" : "eMAC"));
            }


        }

        if (TSNConfig_t.Port[count].TxParam.TAS.bEnable == TRUE)
        {
            printf("\n--------------------------------  TAS  -----------------------------\n");
            printf("AdminBaseTime      AdminCycleTimeExtension      SoftwareDelay    TimerDomain      Jitter      Latency     QueueNumber      QueueState\n");
            printf("--------------------------------------------------------------------\n");
            
            printf("%-19"PRIu64"", TSNConfig_t.Port[count].TxParam.TAS.AdminBaseTime.nseconds);
            printf("%-29"PRIu64"", TSNConfig_t.Port[count].TxParam.TAS.AdminCycleTimeExtension.nseconds);
            printf("%-17"PRIu64"", TSNConfig_t.Port[count].TxParam.TAS.SWTimeMultiplier);
            
            printf("%-17d", TSNConfig_t.Port[count].TxParam.TAS.timer_domain);
            printf("%-12d", TSNConfig_t.Port[count].TxParam.TAS.jitter_time);
            printf("%-12d", TSNConfig_t.Port[count].TxParam.TAS.latency_time);

            printf("%-17d", TSNConfig_t.Port[count].TxParam.TAS.AdminGateState[0].QueueNumber);
            printf("%-16s\n", TSNConfig_t.Port[count].TxParam.TAS.AdminGateState[0].State ? "OPEN":"CLOSE");

            for(Count2 = 1; Count2 < TSNConfig_t.Port[count].TxParam.TAS.AdminGateStates; Count2++)
            {
                printf("%107d%16s", TSNConfig_t.Port[count].TxParam.TAS.AdminGateState[Count2].QueueNumber, " ");
                printf("%s\n", TSNConfig_t.Port[count].TxParam.TAS.AdminGateState[Count2].State ? "OPEN":"CLOSE");
            }
            printf("--------------------------------------------------------------------\n");
            printf("AdminCycleTime[µs]  Queue  TimeInterval[µs]  QueueState\n");
            printf("--------------------------------------------------------------------\n");
            for (Count2 = 0; Count2 < TSNConfig_t.Port[count].TxParam.TAS.Queues; Count2++)
            {
                
                for(Count3 = 0; Count3 < TSNConfig_t.Port[count].TxParam.TAS.GateControl[Count2].GateControls; Count3++) {
                    if (Count3 == 0) {
                        printf("%13lu.%03lu  ", CycleTime[count][Count2]/1000, CycleTime[count][Count2]%1000);
                        printf(" %3d  ", TSNConfig_t.Port[count].TxParam.TAS.GateControl[Count2].QueueNumber);
                    }
                    else
                    printf("%25s", " ");
                    printf("%13u.%03u  ", TSNConfig_t.Port[count].TxParam.TAS.GateControl[Count2].QueueGateConfig[Count3].TimeInterval/1000, TSNConfig_t.Port[count].TxParam.TAS.GateControl[Count2].QueueGateConfig[Count3].TimeInterval%1000);
                    printf(" %s\n", TSNConfig_t.Port[count].TxParam.TAS.GateControl[Count2].QueueGateConfig[Count3].State ? "OPEN":"CLOSE");
                }
            }
        }
        else
        {
            printf("Tx TAS  :  Disabled\n");
        }

        

        printf("\n-------------------------------------------------------------------------------------------\n");
    }
}



static bool SetConfig_Integer(mxml_node_t * ParentNode, char const * const Tag, uint32_t *structure, uint32_t ulimit)
{
    uint64_t Data = 0;
    char const * ch = NULL, *c;
    char *s, ss[100];
    if ((ch = SetConfig_GetText(ParentNode, ParentNode, Tag)) != NULL)
    {
        for (s=ss, c=ch; *c; c++) {
            if (*c != '_')
            *(s++) = *c;
        }
        s[0] = 0;

        if (sscanf(ss, "%"PRIu64, &Data) != 1)
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

static bool SetConfig_Integer64(mxml_node_t * ParentNode, char const * const Tag, uint64_t *structure, uint64_t ulimit)
{
    uint64_t Data = 0;
    char const * ch = NULL, *c;
    char *s, ss[100];
    if ((ch = SetConfig_GetText(ParentNode, ParentNode, Tag)) != NULL)
    {
        for (s=ss, c=ch; *c; c++) {
            if (*c != '_')
            *(s++) = *c;
        }
        s[0] = 0;

        if (sscanf(ss, "%"PRIu64, &Data) != 1)
        {
            fprintf(stderr, "\nInvalid <%s>\n", Tag);
            return FALSE;
        }
        if(Data > ulimit)
        {
            printf("%s = %"PRIu64"\n", Tag, Data);
            fprintf(stderr, "Invalid <%s> - cannot be larger than %"PRIu64"", Tag, ulimit);
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

uint32_t CalcmaxInterfaceSize(int queue)
{

    return (((( RENESAS_RSWITCH_TX_QUEUES - 1 - queue) * PKT_BUF_SZ) + PKT_BUF_SZ)* 8);


}

static void Calc_CBS(long double BandwidthFraction, uint32_t queue, uint32_t portTransmitRate_mhz, uint32_t TAS_Enable, uint32_t Cycletime, uint32_t OpenTime)
{
    long double CIV  = 0;
    uint32_t CUL  = 0;
    long double requestDelay = 0;
#ifdef CBS_DEBUG
    printf("BWF = %LF \n", BandwidthFraction);
#endif

    CIV =TAS_Enable? (( ( (portTransmitRate_mhz) * (BandwidthFraction)) / (SystemFreq)) / 8) * (Cycletime / OpenTime):
    (( ( (portTransmitRate_mhz) * (BandwidthFraction)) / (SystemFreq)) / 8) ;
    requestDelay = queue?8.75*(8-queue):8.75*7;
    

    CUL = (((((maxInterferenceSize[gXML_Txqueue] * 8) + requestDelay + 8) * SystemFreq * CIV ) / portTransmitRate_mhz)) ;
#ifdef CBS_DEBUG
    printf("request delay = %Lf \n", requestDelay);
    printf("MAX IF SIZe = %d \n", maxInterferenceSize[gXML_Txqueue]);
    printf("CUL = %d \n", CUL);
#endif
    TSNConfig_t.Port[gTSN_XML_Port].TxParam.TxStream[gXML_Txstream].CUL = CUL;
    TSNConfig_t.Port[gTSN_XML_Port].TxParam.TxStream[gXML_Txstream].CIVman = floor(CIV);
    TSNConfig_t.Port[gTSN_XML_Port].TxParam.TxStream[gXML_Txstream].CIVexp = (long double)(CIV - floor(CIV)) * pow(2, 16);
#ifdef CBS_DEBUG
    printf("CIV = %LF \n", CIV);
#endif
}





/*
* Set_Config_Transmit_Queue() : Update the Transmit queue structure from the input XML configuration
* arg1 -     Node             : Configuration XML node for Tx Queue
* arg2 -     Parent           : Configuration Parent XML node for Tx Queue
*/
static bool Set_Config_Transmit_Queue(mxml_node_t * Node ,mxml_node_t * Parent)
{
    char const * XMLResult = NULL;
    uint32_t     QueueInp  = 0;
    uint32_t     CBSInp    = 0;
    uint32_t     maxFrameSize  = 0;
    uint32_t Cycletime  = 0;
    uint32_t GateControl  = 0;
    uint32_t TAS_Enable  = 0;
    uint32_t OpenTime = 0;
    uint32_t q = 0;
    long double  BWFInp    = 0;
    /*Get Queue XML Node and update the structure*/
    if ((XMLResult = SetConfig_GetText(Node, Parent, "Queue")) == NULL)
    {
        fprintf(stderr, "\nERROR: No <Queue> in <Txqueue> '%u' in <Transmit> in <Port> '%u' in <TSNSSW> in '%s'\n",
        gXML_Txqueue, TSNConfig_t.Port[gTSN_XML_Port].PortNumber, gOptConfigFile);
        return FALSE;
    }
    if (sscanf(XMLResult, "%u", &QueueInp) != 1)
    {
        fprintf(stderr, "\nERROR: Invalid <Queue> '%.31s' in <Txqueue> '%u' in <Transmit> in <Port> '%u' in <TSNSSW> in '%s'\n",
        XMLResult, gXML_Txqueue, TSNConfig_t.Port[gTSN_XML_Port].PortNumber, gOptConfigFile);
        return FALSE;
    }
    if (QueueInp >= RENESAS_RSWITCH_TX_QUEUES)
    {
        fprintf(stderr, "\nERROR: Invalid <Queue> '%.31s' in <Txqueue> '%u' in <Transmit> in <Port> '%u' in <TSNSSW> in '%s'\n",
        XMLResult, gXML_Txqueue, TSNConfig_t.Port[gTSN_XML_Port].PortNumber, gOptConfigFile);
        return FALSE;
    }
    else
    {
        TSNConfig_t.Port[gTSN_XML_Port].TxParam.TxQueue[gXML_Txqueue].QueueNumber = QueueInp;

    }

    /*Get Queue XML Node and update the structure*/
    if ((XMLResult = SetConfig_GetText(Node, Parent, "maxFrameSize")) == NULL)
    {
        fprintf(stderr, "\nERROR: No <maxFrameSize> in <Txqueues> in <TxQueue> '%u' in <Transmit> in <Port> '%u' in <TSNSSW> in '%s'\n",
        gXML_Txqueue, TSNConfig_t.Port[gTSN_XML_Port].PortNumber, gOptConfigFile);
        return FALSE;
    }
    if (sscanf(XMLResult, "%u", &maxFrameSize) != 1)
    {
        fprintf(stderr, "\nERROR: Invalid <maxFrameSize> '%.31s' in <Txqueues> in <TxQueue> '%u' in <Transmit> in <Port> '%u' in <TSNSSW> in '%s'\n",
        XMLResult, gXML_Txqueue, TSNConfig_t.Port[gTSN_XML_Port].PortNumber, gOptConfigFile);
        return FALSE;
    }

    else
    {
        TSNConfig_t.Port[gTSN_XML_Port].TxParam.TxQueue[gXML_Txqueue].MAX_Frame_Sz = maxFrameSize;

    }


    /*Get Queue XML Node and update the structure*/
    if ((XMLResult = SetConfig_GetText(Node, Parent, "maxInterferenceSize")) != NULL)
    {
        if (sscanf(XMLResult, "%u", &maxInterferenceSize[gXML_Txqueue]) != 1)
        {
            fprintf(stderr, "\nERROR: Invalid <maxInterferenceSize> '%.31s' in <Txqueue> '%u' in <Transmit> in <Port> '%u' in <TSNSSW> in '%s'\n",
            XMLResult, gXML_Txqueue, TSNConfig_t.Port[gTSN_XML_Port].PortNumber, gOptConfigFile);
            return FALSE;
        }
    }



    /*Get PreEmptMAC XML Node and update the structure*/
    if ((XMLResult = SetConfig_GetText(Node, Parent, "PremptMAC")) != NULL)
    {
        if (strncasecmp(XMLResult, "true", 4) == 0)
        {
            TSNConfig_t.Port[gTSN_XML_Port].TxParam.TxQueue[gXML_Txqueue].Pre_Empt_MAC = TRUE;
        }
        else if (strncasecmp(XMLResult, "false", 5) == 0)
        {
            TSNConfig_t.Port[gTSN_XML_Port].TxParam.TxQueue[gXML_Txqueue].Pre_Empt_MAC = FALSE;
        }
        else
        {
            fprintf(stderr, "\nERROR: Invalid <PreEmptMAC> '%.31s' in <Txqueue> '%u' in <Transmit> in <Port> '%u' in <TSNSSW> in '%s'\n",
            XMLResult, gXML_Txqueue, TSNConfig_t.Port[gTSN_XML_Port].PortNumber, gOptConfigFile);
            return FALSE;
        }
    }

    /*Get the BWF XML Node and update the structure*/
    if ((XMLResult = SetConfig_GetText(Node, Parent, "bandwidthFraction")) == NULL)
    {
        fprintf(stderr, "\Warning: No <bandwidthFraction> in <Txqueue> '%u'  in <Transmit> in <Port> '%u' in <TSNSSW> in '%s'\n",
        gXML_Txqueue, TSNConfig_t.Port[gTSN_XML_Port].PortNumber, gOptConfigFile);
        return TRUE;
    }
    if (sscanf(XMLResult, "%LF", &BWFInp) != 1)
    {
        fprintf(stderr, "\nERROR: Invalid <bandwidthFraction> '%.31s' in <Txqueue> '%u' in <Transmit> in <Port> '%u' in <TSNSSW> in '%s'\n",
        XMLResult, gXML_Txqueue, TSNConfig_t.Port[gTSN_XML_Port].PortNumber, gOptConfigFile);
        return FALSE;
    }
    else
    {
        TSNConfig_t.Port[gTSN_XML_Port].TxParam.TxStream[gXML_Txstream].QueueNum = QueueInp;
        
        TSNConfig_t.Port[gTSN_XML_Port].TxParam.TxStream[gXML_Txstream].BandwidthFraction = BWFInp*100;
        TSNConfig_t.Port[gTSN_XML_Port].TxParam.TxStream[gXML_Txstream].portTransmitRate = PortTransmitRate[gTSN_XML_Port]*1000000;
        TAS_Enable = TSNConfig_t.Port[gTSN_XML_Port].TxParam.TAS.bEnable;
        if(TAS_Enable)
        {
            Cycletime =  TSNConfig_t.Port[gTSN_XML_Port].TxParam.TAS.AdminCycleTime;
            for(q  =0; q < TSNConfig_t.Port[gTSN_XML_Port].TxParam.TAS.Queues; q++)
            {
                if(TSNConfig_t.Port[gTSN_XML_Port].TxParam.TAS.GateControl[q].QueueNumber == QueueInp)
                {
                    for(GateControl = 0; GateControl < TSNConfig_t.Port[gTSN_XML_Port].TxParam.TAS.GateControl[q].GateControls; GateControl++)
                    {

                        if(TSNConfig_t.Port[gTSN_XML_Port].TxParam.TAS.GateControl[q].QueueGateConfig[GateControl].State == rswitch_GateState_Open )
                        {

                            OpenTime +=TSNConfig_t.Port[gTSN_XML_Port].TxParam.TAS.GateControl[q].QueueGateConfig[GateControl].TimeInterval;
                        }
                    }
                    break;
                }

            }
        }
        Calc_CBS(BWFInp,QueueInp,PortTransmitRate[gTSN_XML_Port],TAS_Enable, Cycletime, OpenTime );
        TSNConfig_t.Port[gTSN_XML_Port].TxParam.TxStreams++;
#ifdef CBS_DEBUG
        printf("BWF (%%)  MAxframeSz(%d) MaxInterferenceSz(%d) for Queue %d = %d CUL=%x CIVexp= %x CIVman = %x \n",
        TSNConfig_t.Port[gTSN_XML_Port].TxParam.TxQueue[gXML_Txqueue].MAX_Frame_Sz,
        maxInterferenceSize[gXML_Txqueue],
        QueueInp,
        (uint32_t)(BWFInp * 100),
        TSNConfig_t.Port[gTSN_XML_Port].TxParam.TxStream[gXML_Txstream].CUL,
        TSNConfig_t.Port[gTSN_XML_Port].TxParam.TxStream[gXML_Txstream].CIVexp,
        TSNConfig_t.Port[gTSN_XML_Port].TxParam.TxStream[gXML_Txstream].CIVman );
#endif
        gXML_Txstream++;
    }




    return TRUE;
}



static bool Set_Config_Transmit_TxQueues(mxml_node_t * Transmit)
{
    mxml_node_t   * TxQueues  = NULL;
    mxml_node_t   * TxQueue   = NULL;
    uint32_t        queue = 0;
    for(queue = 0; queue < RENESAS_RSWITCH_TX_QUEUES; queue++)
    {
        TSNConfig_t.Port[gTSN_XML_Port].TxParam.TxQueue[queue].MAX_Frame_Sz = -1;

    }
    /*Get TxQueues XML Node*/
    if ((TxQueues = mxmlFindElement(Transmit, Transmit, "TxQueues", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
    {
        fprintf(stderr, "\n WARNING: Unable to find <TxQueues> in <Transmit> in <Port> '%u' in <TSNSSW> in '%s'\n",
        TSNConfig_t.Port[gTSN_XML_Port].PortNumber, gOptConfigFile);
        return TRUE;
    }
    TSNConfig_t.Port[gTSN_XML_Port].TxParam.TxStreams = 0;
    /*Get TxQueue XML Node and update the structure*/
    if ((TxQueue = mxmlFindElement(TxQueues, TxQueues, "TxQueue", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
    {
        for (;TxQueue != NULL; TxQueue = mxmlWalkNext(TxQueue, Transmit, MXML_NO_DESCEND))
        {
            if (TxQueue->type != MXML_ELEMENT)
            {
                continue;
            }
            if (strcasecmp("TxQueue", TxQueue->value.element.name) == 0)
            {
                if (NOT Set_Config_Transmit_Queue(TxQueue, Transmit))
                {
                    fprintf(stderr, "\nERROR: Unable to find <TxQueue> in <Port> '%u' in <TSNSSW> in '%s'",
                    TSNConfig_t.Port[gTSN_XML_Port].PortNumber, gOptConfigFile);
                    return FALSE;
                }
                gXML_Txqueue++;
            }
        }

        if (gXML_Txqueue > RENESAS_RSWITCH_TX_QUEUES)
        {
            fprintf(stderr, "\nERROR: <TxQueue> '%u' in <Transmit> in <Port> '%u' in <TSNSSW> in '%s' is out of range (0 - to %u only)",
            gXML_Txqueue, TSNConfig_t.Port[gTSN_XML_Port].PortNumber, gOptConfigFile, RENESAS_RSWITCH_TX_QUEUES);
            return FALSE;
        }
        TSNConfig_t.Port[gTSN_XML_Port].TxParam.TxQueues = gXML_Txqueue;

    }
    return TRUE;

}

uint64_t lcm_func(uint64_t n1, uint64_t n2)
{
    uint64_t  minMultiple;


    minMultiple = (n1>n2) ? n1 : n2;
    // Always true
    while(1)
    {
        if( minMultiple%n1==0 && minMultiple%n2==0 )
        {
            //printf("The LCM of %d and %d is %d.", n1, n2,minMultiple);
            return minMultiple;

        }
        ++minMultiple;
    }
    return 0;



}



/*
* Set_Config_Transmit()      : Update the Transmit structure from the input XML configuration
* arg1 -    TopNode          : Configuration XML node for Rx Stream
*/
static bool Set_Config_Transmit(mxml_node_t * TopNode)
{
    mxml_node_t * Transmit  = NULL;
    mxml_node_t * Tas       = NULL;
    
    mxml_node_t * GateControlLists = NULL;
    mxml_node_t * GateControlList = NULL;
    mxml_node_t * AdminGateStates = NULL;
    mxml_node_t * Queues = NULL;
    mxml_node_t * Queue = NULL;
    mxml_node_t * Tn = NULL;
    
    uint8_t     Count = 0;
    uint8_t     Count2 = 0;

    /*Get Transmit XML Node*/
    if ((Transmit = mxmlFindElement(TopNode, TopNode, "Transmit", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
    {
        if (NOT Set_Config_Transmit_TxQueues(Transmit))
        {
            return FALSE;
        }




        

        /*
            Zero or One <TAS> definition under <Transmit>
            */
        if ((Tas = mxmlFindElement(Transmit, Transmit, "TAS", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
        {
            gXML_GateControl = 0;
            
            TSNConfig_t.Port[gTSN_XML_Port].TxParam.TAS.bEnable=TRUE;
            

            /*ieee8021STAdminBaseTime*/
            if(NOT SetConfig_Integer64(Tas, "AdminBaseTime",
                        &TSNConfig_t.Port[gTSN_XML_Port].TxParam.TAS.AdminBaseTime.nseconds, 100000000000))
            {
                fprintf(stderr, "\nERROR: Unable to find <AdminBaseTime> in <TAS> in <Transmit> in <Port> in <Ethernet-Ports> in '%s'\n",
                gOptConfigFile);
                return FALSE;
            }

            /*ieee8021STAdminCycleTimeExtension*/
            if(NOT SetConfig_Integer64(Tas, "AdminCycleTimeExtension",
                        &TSNConfig_t.Port[gTSN_XML_Port].TxParam.TAS.AdminCycleTimeExtension.nseconds, UINT64_MAX-1))
            {
                fprintf(stderr, "\nERROR: Unable to find <AdminCycleTimeExtension> in <TAS> in <Transmit> in <Port> in <Ethernet-Ports> in '%s'\n",
                gOptConfigFile);
                return FALSE;
            }

            /*SoftwareMultiplier*/
            if(NOT SetConfig_Integer64(Tas, "SoftwareDelay",
                        &TSNConfig_t.Port[gTSN_XML_Port].TxParam.TAS.SWTimeMultiplier, 100000000000))
            {
                fprintf(stderr, "\nERROR: Unable to find <SoftwareDelay> in <TAS> in <Transmit> in <Port> in <Ethernet-Ports> in '%s'\n",
                gOptConfigFile);
                return FALSE;
            }
            


            /*TimerDomain*/
            if(NOT SetConfig_BinaryText(Tas, "TimerDomain", &TSNConfig_t.Port[gTSN_XML_Port].TxParam.TAS.timer_domain,
                        "1", "0", 1, 1))
            {
                fprintf(stderr, "\nERROR: Unable to find <TimerDomain> in <TAS> in <Transmit> in <Port> in <Ethernet-Ports> in '%s'\n",
                gOptConfigFile);
                return FALSE;
            }

            /*Jitter*/
            if(NOT SetConfig_Integer(Tas, "Jitter",
                        &TSNConfig_t.Port[gTSN_XML_Port].TxParam.TAS.jitter_time, ULONG_MAX))
            {
                fprintf(stderr, "\nERROR: Unable to find <Jitter> in <TAS> in <Transmit> in <Port> in <Ethernet-Ports> in '%s'\n",
                gOptConfigFile);
                return FALSE;
            }
            /*AdminGateStates*/
            if ((AdminGateStates = mxmlFindElement(Tas, Tas, "AdminGateStates", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
            {
                if ((Queues = mxmlFindElement(AdminGateStates, AdminGateStates, "Queues", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
                {

                    /*Queue*/
                    if((Queue = mxmlFindElement(Queues, Queues, "Queue", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
                    {
                        fprintf(stderr, "\nERROR : Unable to find <Queue> in <Queues> in <AdminGateStates> in <TAS> in <Transmit> in <Port> in <Ethernet-Ports> in '%s'\n",
                        gOptConfigFile);
                        return FALSE;
                    }
                    else
                    {
                        Count = 0;
                        for(; Queue!= NULL; Queue = mxmlWalkNext(Queue, Queues, MXML_NO_DESCEND))
                        {

                            if (Queue->type != MXML_ELEMENT)
                            {
                                continue;
                            }
                            /*QueueNumber*/
                            if(NOT SetConfig_Integer(Queue, "QueueNumber",
                                        &TSNConfig_t.Port[gTSN_XML_Port].TxParam.TAS.AdminGateState[Count].QueueNumber, ULONG_MAX))
                            {
                                fprintf(stderr, "\nERROR: Unable to find <QueueNumber> in <Queue> in <AdminGateStates> in <TAS> in <Transmit> in <Port> in <Ethernet-Ports> in '%s'\n",
                                gOptConfigFile);
                                return FALSE;
                            }
                            /*State*/
                            if(NOT SetConfig_BinaryText(Queue, "State", &TSNConfig_t.Port[gTSN_XML_Port].TxParam.TAS.AdminGateState[Count].State,
                                        "Open", "Close", 4, 6))
                            {
                                fprintf(stderr, "\nERROR: Unable to find <State> in <Queue> in <AdminGateStates> in <TAS> in <Transmit> in <Port> in <Ethernet-Ports> in '%s'\n",
                                gOptConfigFile);
                                return FALSE;
                            }
                            Count++;
                        }
                        TSNConfig_t.Port[gTSN_XML_Port].TxParam.TAS.AdminGateStates = Count;
                    }
                }
                else
                {
                    fprintf(stderr, "\nERROR : Unable to find <Queues> in <AdminGateStates> in <TAS> in <Transmit> in <Port> in <Ethernet-Ports> in '%s'\n",
                    gOptConfigFile);
                    return FALSE;
                }
            }
            else
            {
                fprintf(stderr, "\nERROR : Unable to find <AdminGateStates> in <TAS> in <Transmit> in <Port> in <Ethernet-Ports> in '%s'\n",
                gOptConfigFile);
                return FALSE;
            }

            /*Latency*/
            if(NOT SetConfig_Integer(Tas, "Latency",
                        &TSNConfig_t.Port[gTSN_XML_Port].TxParam.TAS.latency_time, ULONG_MAX))
            {
                fprintf(stderr, "\nERROR: Unable to find <Latency> in <TAS> in <Transmit> in <Port> in <Ethernet-Ports> in '%s'\n",
                gOptConfigFile);
                return FALSE;
            }

            /*AdminControlLists*/
            if ((GateControlLists = mxmlFindElement(Tas, Tas, "AdminControlLists", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
            {
                if ((GateControlList = mxmlFindElement(GateControlLists, GateControlLists, "AdminControlList", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
                {
                    fprintf(stderr, "\nERROR : Unable to find <AdminControlList> in <AdminControlLists> in <TAS> in <Transmit> in <Port> in <Ethernet-Ports> in '%s'\n",
                    gOptConfigFile);
                    return FALSE;
                }
                else
                {
                    Count = 0;
                    /*AdminControlList*/
                    for(; GateControlList!= NULL; GateControlList = mxmlWalkNext(GateControlList, GateControlLists, MXML_NO_DESCEND))
                    {
                        if (GateControlList->type != MXML_ELEMENT)
                        {
                            continue;
                        }

                        /*QueueNumber*/
                        if(NOT SetConfig_Integer(GateControlList, "QueueNumber",
                                    &TSNConfig_t.Port[gTSN_XML_Port].TxParam.TAS.GateControl[Count].QueueNumber, ULONG_MAX))
                        {
                            fprintf(stderr, "\nERROR: Unable to find <QueueNumber> in <AdminControlList> in <AdminControlLists> in <TAS> in <Transmit> in <Port> in <Ethernet-Ports> in '%s'\n",
                            gOptConfigFile);
                            return FALSE;
                        }
                        if ((Tn = mxmlFindElement(GateControlList, GateControlList, "Tn", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
                        {
                            fprintf(stderr, "\nERROR : Unable to find <Tn> in <AdminControlList> in <AdminControlLists> in <TAS> in <Transmit> in <Port> in <Ethernet-Ports> in '%s'\n",
                            gOptConfigFile);
                            return FALSE;
                        }
                        else
                        {
                            /*Tn*/
                            Count2 = 0;
                            for(; Tn != NULL; Tn = mxmlWalkNext(Tn, GateControlList, MXML_NO_DESCEND))
                            {

                                if (Tn->type != MXML_ELEMENT)
                                {
                                    continue;
                                }
                                /*TimeInterval*/
                                if(NOT SetConfig_Integer(Tn, "TimeInterval",
                                            &TSNConfig_t.Port[gTSN_XML_Port].TxParam.TAS.GateControl[Count].QueueGateConfig[Count2].TimeInterval, ULONG_MAX))
                                {
                                    fprintf(stderr, "\nERROR: Unable to find <TimeInterval> in <Tn> in <AdminControlList> in <AdminControlLists> in <TAS> in <Transmit> in <Port> in <Ethernet-Ports> in '%s'\n",
                                    gOptConfigFile);
                                    return FALSE;
                                }

                                
                                {
                                    CycleTime[gTSN_XML_Port][Count] = CycleTime[gTSN_XML_Port][Count] + TSNConfig_t.Port[gTSN_XML_Port].TxParam.TAS.GateControl[Count].QueueGateConfig[Count2].TimeInterval;
                                    
                                }
                                /*QueueState*/
                                if(NOT SetConfig_BinaryText(Tn, "QueueState", &TSNConfig_t.Port[gTSN_XML_Port].TxParam.TAS.GateControl[Count].QueueGateConfig[Count2].State,
                                            "Open", "Close", 4, 5))
                                {
                                    fprintf(stderr, "\nERROR: Unable to find <QueueState> in <Tn> in <AdminControlList> in <AdminControlLists> in <TAS> in <Transmit> in <Port> in <Ethernet-Ports> in '%s'\n",
                                    gOptConfigFile);
                                    return FALSE;
                                }
                                Count2++;
                            }
                            if(!Count)
                            {
                                TSNConfig_t.Port[gTSN_XML_Port].TxParam.TAS.AdminCycleTime = CycleTime[gTSN_XML_Port][Count];
                            }
                            else
                            {
                                TSNConfig_t.Port[gTSN_XML_Port].TxParam.TAS.AdminCycleTime   =
                                lcm_func(CycleTime[gTSN_XML_Port][Count], TSNConfig_t.Port[gTSN_XML_Port].TxParam.TAS.AdminCycleTime);
                            }
                            TSNConfig_t.Port[gTSN_XML_Port].TxParam.TAS.GateControl[Count].GateControls = Count2;
                            printf("Admin Cycle Time queue %d = %d \n",
                            TSNConfig_t.Port[gTSN_XML_Port].TxParam.TAS.AdminGateState[Count].QueueNumber, CycleTime[gTSN_XML_Port][Count]);
                        }
                        Count++;
                        ListTotal++;
                        if(ListTotal > RENESAS_RSWITCH_ETH_MAX_GATE_CONTROLS)
                        {
                            fprintf(stderr, "\nERROR: Total GateControls %d exceeds maximum value %d in  in <AdminControlLists> in <TAS> in <Transmit> in <Port> in <Ethernet-Ports> in '%s'\n",
                            ListTotal, RENESAS_RSWITCH_ETH_MAX_GATE_CONTROLS, gOptConfigFile);
                            return FALSE;

                        }
                    }

                    TSNConfig_t.Port[gTSN_XML_Port].TxParam.TAS.Queues = Count;
                    TSNConfig_t.Port[gTSN_XML_Port].TxParam.TAS.SWTimeMultiplier +=
                    ( TSNConfig_t.Port[gTSN_XML_Port].TxParam.TAS.AdminCycleTime
                    - (TSNConfig_t.Port[gTSN_XML_Port].TxParam.TAS.SWTimeMultiplier
                    % TSNConfig_t.Port[gTSN_XML_Port].TxParam.TAS.AdminCycleTime));
                    printf("Final Admin Cycle Time = %d \n",TSNConfig_t.Port[gTSN_XML_Port].TxParam.TAS.AdminCycleTime);
                }
            }
            else
            {
                fprintf(stderr, "\nERROR: Unable to find <QueueState> in <Tn> in <AdminControlList> in <AdminControlLists> in <TAS> in <Transmit> in <Port> in <Ethernet-Ports> in '%s'\n",
                gOptConfigFile);
                return FALSE;
            }
        }

    }
    return TRUE;
}









/*
* Set_Config()                : Update the TSN Port Configuration from the input XML configuration
* arg1 -     Tree             : Configuration XML Tree for TSN Port
* arg2 -     PortNode         : Port Node Name for TSN Port
*/
static bool Set_Config(mxml_node_t * Tree, const char * const PortNode)
{
    mxml_node_t   * TopNode    = NULL;
    mxml_node_t   * Port       = NULL;
    mxml_node_t   * PortName   = NULL;
    mxml_node_t   * FrerConfig = NULL;
    char const    * XMLResult  = NULL;
    unsigned int    PortNumber = 0;
    unsigned int    PminSize = 0;
    unsigned int    RateInp = 0;
    long double SysFreq = 0.0;
    

    if ((TopNode = mxmlFindElement(Tree, Tree, PortNode, NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
    {
        fprintf(stderr, "\nERROR: Unable to find <%s> in '%s'\n", PortNode, gOptConfigFile);
        return FALSE;
    }
    if ((XMLResult = SetConfig_GetText(TopNode, TopNode, "SystemClockFrequency")) == NULL)
    {
        fprintf(stderr, "\nERROR: No <SystemClockFrequency>   in  <TSNSSW> in '%s'\n",
        gOptConfigFile);
        return FALSE;
    }
    if (sscanf(XMLResult, "%LF", &SysFreq) != 1)
    {
        fprintf(stderr, "\nERROR: Invalid <SystemClockFrequency> '%.31s'  in <TSNSSW> in '%s'\n",
        XMLResult,  gOptConfigFile);
        return FALSE;
    }
    else
    {
        SystemFreq = SysFreq;

    }




    /*Get the TSN Port XML Node from configuration file */
    if ((Port = mxmlFindElement(TopNode, TopNode, "Port", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
    {
        fprintf(stderr, "\nERROR: Unable to find <Port> in '%s'\n", gOptConfigFile);
        return FALSE;
    }
    for (; Port!= NULL; Port = mxmlWalkNext(Port, TopNode, MXML_NO_DESCEND))
    {
        if (Port->type != MXML_ELEMENT)
        {
            continue;
        }

        /*Get Port*/
        if (strcasecmp("Port", Port->value.element.name) == 0)
        {
            /*Get PortNumber XML Node and update the strucutre*/
            if  ((XMLResult = SetConfig_GetText(Port, Port, "Portnumber")) == NULL)
            {
                fprintf(stderr, "\n ERROR: No <Portnumber> in <Port> %u in <%s> in '%s' \n",
                gTSN_XML_Port, PortNode, gOptConfigFile);
                return FALSE;
            }
            if (sscanf(XMLResult, "%u", &PortNumber) != 1)
            {
                fprintf(stderr, "\n ERROR: No <Portnumber> '%.31s' in <Port> %u in <%s> in '%s' \n",
                XMLResult, gTSN_XML_Port, PortNode, gOptConfigFile);
                return FALSE;
            }
            TSNConfig_t.Port[gTSN_XML_Port].PortNumber = PortNumber;
            

            /*Get PortNam from XML Node and update the structure*/
            if ((PortName = mxmlFindElement(Port, TopNode, "PortName", NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
            {
                if ((XMLResult = SetConfig_GetText(Port, TopNode, "PortName")) != NULL)
                {
                    strncpy(TSNConfig_t.Port[gTSN_XML_Port].PortName, XMLResult, sizeof(TSNConfig_t.Port[gTSN_XML_Port].PortName)-1);
                }
            }

            /*Get MAC Address from XML Node and update the structure*/
            if ((XMLResult = SetConfig_GetText(Port, TopNode, "MAC")) != NULL)
            {
                if (strlen(XMLResult) != 17)
                {
                    fprintf(stderr, "\nERROR: Invalid <MAC> (length) '%.31s' in <Port> in '%s'\n", XMLResult, gOptConfigFile);
                    return FALSE;
                }
                if (sscanf(XMLResult, "%2s:%2s:%2s:%2s:%2s:%2s", gMACChar0, gMACChar1, gMACChar2, gMACChar3, gMACChar4, gMACChar5) != 6)
                {
                    fprintf(stderr, "\nERROR: Invalid <MAC> (hex) '%.31s' in <port> %u  in '%s'\n",
                    XMLResult, TSNConfig_t.Port[gTSN_XML_Port].PortNumber, gOptConfigFile);

                }
                gMAC[0] = strtoul(gMACChar0, 0, 16);
                gMAC[1] = strtoul(gMACChar1, 0, 16);
                gMAC[2] = strtoul(gMACChar2, 0, 16);
                gMAC[3] = strtoul(gMACChar3, 0, 16);
                gMAC[4] = strtoul(gMACChar4, 0, 16);
                gMAC[5] = strtoul(gMACChar5, 0, 16);
                memcpy(TSNConfig_t.Port[gTSN_XML_Port].MAC, gMAC, sizeof(gMAC));
                TSNConfig_t.Port[gTSN_XML_Port].bEnableMAC = TRUE;
            }

            if ( gTSN_XML_Port >= RENESAS_RSWITCH_MAX_ETHERNET_PORTS)
            {
                fprintf(stderr, "\nERROR: Invalid <Port> '%u' in <TSNSSW> in '%s' - out of range (0 to %lu)\n",
                gTSN_XML_Port, gOptConfigFile, ARRAY_SIZE(TSNConfig_t.Port) );
                return FALSE;
            }

            /*Get addFragSize XML Node and update the strucutre*/
            if  ((XMLResult = SetConfig_GetText(Port, Port, "addFragSize")) != NULL)
            {

                if (sscanf(XMLResult, "%u", &PminSize) != 1)
                {
                    fprintf(stderr, "\n ERROR: No <addFragSize> '%.31s' in <Port> %u in <%s> in '%s' \n",
                    XMLResult, gTSN_XML_Port, PortNode, gOptConfigFile);
                    return FALSE;
                }

                if((PminSize == 0) || (PminSize == 1) || (PminSize == 2) || (PminSize == 3))
                {
                    TSNConfig_t.Port[gTSN_XML_Port].PminSize = PminSize;
                }
                else
                {
                    fprintf(stderr, "\n ERROR: Invalid <addFragSize> '%.31s' in <Port> %u in <%s> in '%s' Valid values 60,124,188,252\n",
                    XMLResult, gTSN_XML_Port, PortNode, gOptConfigFile);
                    return FALSE;


                }
            }
            else
            {
                fprintf(stderr, "\n WARNING: No <addFragSize> in <Port> %u in <%s> in '%s' \n",
                gTSN_XML_Port, PortNode, gOptConfigFile);
                
            }
            /*Get the portTransmitRate XML Node and update the structure*/
            if ((XMLResult = SetConfig_GetText(Port, Port, "portTransmitRate")) == NULL)
            {
                fprintf(stderr, "\nERROR: No <portTransmitRate>   in <Port> '%u' in <TSNSSW> in '%s'\n",
                TSNConfig_t.Port[gTSN_XML_Port].PortNumber, gOptConfigFile);
                return FALSE;
            }
            if (sscanf(XMLResult, "%u", &RateInp) != 1)
            {
                fprintf(stderr, "\nERROR: Invalid <protTransmitRate> '%.31s'  in <Port> '%u' in <TSNSSW> in '%s'\n",
                XMLResult, TSNConfig_t.Port[gTSN_XML_Port].PortNumber, gOptConfigFile);
                return FALSE;
            }
            else
            {
                PortTransmitRate[gTSN_XML_Port] = RateInp;

            }

            
            if(NOT Set_Config_Transmit(Port))
            {
                return FALSE;
            }
            
            gTSN_XML_Port++;
            
        }
    }
    TSNConfig_t.Ports = gTSN_XML_Port;
    

    TSN_Print_Configuration();
    return TRUE;
}

/*
* TSN_Set_Config : Function called by rswitch tool to update the TSN Configuration
* Tree           : TSN XML Tree
*/
extern bool TSN_Set_Config(mxml_node_t * Tree)
{
    if (NOT Set_Config(Tree, "Ethernet-Ports"))
    {
        return FALSE;
    }
    return TRUE;
}


/*
* TSN_Reset_Configuration : Function called by rswitch tool to reset the TSN Configuration
*/
void TSN_Reset_Configuration(void)
{
    memset(&TSNConfig_t, 0, sizeof(struct rswitch_Config));
}

/*
* TSN_Report_Device : Function called by rswitch tool to report full system TSN configuration
*/
extern bool TSN_Report_Device(void)
{
    int ret = 0;


    if (gEthFd != -1)   //tbern
    {
        if ((ret = ioctl(gEthFd, RSWITCH_GET_CONFIG , &TSNConfig_t)) != 0)  //disable for testing with no driver -tbern
        {
            fprintf(stderr, "\nERROR: RSW_GET_CONFIG failed due to %s\n", strerror(errno));
            return FALSE;
        }
        TSN_Print_Configuration();
    }
    else
    {
        fprintf(stderr, "WARNING: Omiting SW_ETH_GET_CONFIG as module not open\n");
    }
    return TRUE;
}

/*
* TSN_Configure_Device : Function called by rswitch tool to configure full system TSN configuration
*/
extern bool TSN_Configure_Device(void)
{
    int ret = 0;

    if (gEthFd != -1)
    {
        if ((ret = ioctl(gEthFd, RSWITCH_SET_CONFIG, &TSNConfig_t)) != 0)   //disable for testing with no driver -tbern
        {
            fprintf(stderr, "\nERROR: R_SW_SET_CONFIG  Failed due to %s\n", strerror(errno));
            return FALSE; //tbern
        }
    }
    else
    {
        fprintf(stderr, "WARNING: Omiting SW_ETH_SET_CONFIG as module not open\n");
    }
    return TRUE;
}

/*
* TSN_Open_Driver : Function to open the TSN Driver File descriptor
*/
extern bool TSN_Open_Driver(void)   //disable for testing when no driver present -tbern
{
    gEthFd = open(SWITCH_AVB_TSN_DEV_NAME, O_RDWR | O_SYNC);

    if (gEthFd < 0)
    {
        fprintf(stderr, "\n ERROR : TSN Open '%s' failed : %s \n", SWITCH_AVB_TSN_DEV_NAME, strerror(errno));
        return FALSE;
    }
    return TRUE;
}

/*
* TSN_Close_Driver : Function to close the TSN Driver File descriptor
*/
extern bool TSN_Close_Driver(void) 
{
    if (gEthFd != -1)
    {
        close(gEthFd);
        gEthFd = -1;
    }
    return TRUE;
}


/*
* Change History
* 2019-04-08   BT   R-Switch tool initial development and release
* 2019-04-26   AK   Added CBS & Preemption Support
* 2019-09-10   AK   Updated CBS as per new xml
* 2019-09-13   AK   Updated for Max frame size and interference size
* 2019-09-13   AK   Updated for Flexible Gate Control list
* 2019-09-27   AK   Updated for optional BWF, SoftwareDelay, MaxFramesize can be 0, Removed ConfigChange Time, To be calculated by driver
                    Removed 1eee8021ST Prefix
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
