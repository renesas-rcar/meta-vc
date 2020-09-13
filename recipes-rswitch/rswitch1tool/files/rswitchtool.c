/*
* @brief r-switch configuration and report tool
*
* rswitchtool
*  - Configure TSN ports
*  - Report active configuration
*
* exit codes
*  - 0 success
*  - 1 command-line error (usage error)
*  - 2 (unused)
*  - 3 execution error
*
* @author
* -
*
*/



#include "rswitchtool.h"
#include "rswitch_TSN.h"
#include "rswitch_FWD.h"

#ifdef BUILD_OPTION_DEBUG_FPGA
#include <drivers/platform/rswitch/rswitch_debug.h> //-tbern
#define RSWITCH_DEBUG_DEB_NAME "/dev/"RSWITCH_DEBUG_DEVICE_NAME

int             gFDdebug = -1;
uint32_t        IPRAM[256 * 1024];
#endif /* BUILD_OPTION_DEBUG_FPGA */

bool   gCmdConfigure    = FALSE;
bool   gCmdReport       = FALSE;
bool   gOptConfigureFwd = FALSE;
bool   gOptReportFwd    = FALSE;
bool   gOptFwd          = FALSE;
bool   gOptConfigureEth = FALSE;
bool   gOptConfigurePhy = FALSE;
bool   gOptReportPhy    = FALSE;
bool   gOptReportEth    = FALSE;
bool   gOptEth          = FALSE;
bool   gOptPhy          = FALSE;
bool   gCmd_SetFPGA     = FALSE;
bool   gCmd_ShowFPGA    = FALSE;
bool   gOpt_OpenDebug   = FALSE;

bool   configbitfile    = FALSE;
bool   page1_adr_is_set = FALSE;
bool   erasebitfile     = FALSE;

char * gOptConfigFile   = "";
char * gOptBitFile      = "";
char * gProgName        = NULL;

unsigned int         gOpt_FPGA_Register      = 0;
unsigned int         gOpt_FPGA_RegisterEnd   = 0;
unsigned int         gOpt_FPGA_Value         = 0;
unsigned int         gOpt_FPGA_Address        = 0;
const unsigned int   Fpga_Flash_Memory_Offset = 0x04000000;
unsigned int         Fpga_Page1_Start_Address = 0x01580000;
unsigned int         Fpga_Update_Address      = 0;
static unsigned int  PhyCount = 0;
static uint32_t      PhyPort[RENESAS_RSWITCH_MAX_ETHERNET_PORTS];

enum rswitch_phy_mode
{
    rswitch_phy_mode_slave,
    rswitch_phy_mode_master
};

static enum rswitch_phy_mode phy_mode[RENESAS_RSWITCH_MAX_ETHERNET_PORTS];

/*Getopt options */
static struct option LongOptions[] =
{
    { "help",       no_argument,       NULL, 'h' },
    { "configure",  required_argument, NULL, 'c' },
    { "report",     required_argument, NULL, 'r' },
    { "file",       required_argument, NULL, 'f' },
    { "version",    no_argument,       NULL, 'v' },
    { "erase",      no_argument,       NULL, 'e' },
    {  NULL,        0,                 NULL      },
};

/*
* Usage() : rswitch tool help print
*/
void usage(void)
{
    printf("\nRSwitch Configure & Report \n"
    "usage : %s {Action} \n"
    "{Action} is \n"
    "  Configure the TSN, PHY or Forwarding or all three. Must be used together with '-f', '--f=' or '--file='\n"
    "   -c {eth | fwd | phy | all}, --c={eth | fwd | phy | all}, --configure={eth | fwd | phy | all}\n"
    "  Config File. Must be used together with '-c', '--c=' or '--configure=' \n"
    "   -f {file}, --f={file}, --file={file}\n"
    "  Remote Configuration. To be used when board is accessed through remote connection\n"
    "   cat {file} | rswitchtool -c {eth | fwd | phy | all}\n"
    "  Report the TSN, PHY or Forwarding or all three\n"
    "   -r {eth | fwd | phy | all}, --r={eth | fwd | phy | all}, --report={eth | fwd | phy | all}\n"
    "  Bit File\n"
    "   -b {file}, --b={file}, --bitfile={file}  Store a new Bitfile in the Flash Memory at StartAddress\n"
    "  Page 1 Start Address in Flash Memory\n"
    "   -p 0x{StartAddess},                      Replace the default Start Address value 0x0158_0000 by a new one\n"
    "                                            StartAddress needs to be 64 kBytes aligned\n"
    "                                            0x0001_0000 <= StartAddress <= 0x7FFF_0000\n"
    "  Erase Flash\n"
    "   -e, --e, --erase                         Erase the Flash Memory starting on the StartAddress\n"
    "  RSwitch tool Version number\n"
    "   -v, --v, --version                       Display the version\n"
#ifdef BUILD_OPTION_DEBUG_FPGA
    "  Debug Actions\n"
    "   -x 0x{reg}                               Display FPGA register/memory hex {reg} \n"
    "   -X 0x{reg}                               As -x but forces memory dump\n"
    "   -x 0x{reg} -y 0x{reg}                    Display a range of FPGA registers/memory \n"
#ifdef BUILD_OPTION_DEBUG_FPGA_ALLOW_UPDATE
    "   -x 0x{reg}=0x{value}}                    Set FPGA register/memory hex {reg} to hex {value} \n"
#endif /* BUILD_OPTION_DEBUG_FPGA_ALLOW_UPDATE */
#endif /* BUILD_OPTION_DEBUG_FPGA */
    "  Manual\n"
    "   -h, --h, --help                          Displays help information\n"
    "\n\n"
    , gProgName);
}

/*
* Close_Drivers() : Close the driver file descriptors
*/
static bool Close_Drivers(void)
{
    if (gOptEth)
    {
        TSN_Close_Driver();
    }

    if(gOptFwd)
    {
        FWD_Close_Driver();
    }

#ifdef BUILD_OPTION_DEBUG_FPGA
    if (gFDdebug != -1)
    {
        close(gFDdebug);
        gFDdebug = -1;
    }
#endif /* BUILD_OPTION_DEBUG_FPGA */

    return TRUE;
}

/*
* Configure_Devices() : Perform the device driver configuration based on the user input
*/
static bool Configure_Devices(void)
{
    bool bReturn = TRUE;

    if (gOptConfigureEth)
    {
        if (NOT TSN_Configure_Device())
        {
            bReturn = FALSE;
        }
    }

    if (gOptConfigureFwd)
    {
        if(NOT FWD_Configure_Device())
        {
            bReturn = FALSE;
        }
    }
    return bReturn;
}

/*
* PHY_Report_Device() : Report the device PHY configuration to the user
*/
static void PHY_Report_Device()
{
    //~ int ret = 0;

    printf("\n=================================== PHY =======================================\n");
    printf("PortNumber      Mode\n");
    for(int Count = 0; Count < PhyCount; Count++)
    {
        printf("%-16u", PhyPort[Count]);
        printf("%s\n", phy_mode[Count] ? "Master" : "Slave");
    }
}

/*
* Report_Configurations() : Report the device driver configuration to the user
*/
static bool Report_Configurations(void)
{
    bool bReturn = TRUE;

    if (gOptReportEth)
    {
        if (NOT TSN_Report_Device())
        {
            bReturn = FALSE;
        }
    }

    if (gOptReportFwd)
    {
        if (NOT FWD_Report_Device())
        {
            bReturn = FALSE;
        }
    }

    return bReturn;
}

/*
* SetConfig_GetText() : Get the Data from the XML Node based on the input Stream, tree, Tag
* arg1 - Stream       : Stream node of XML where Tag needs to be searched
* arg2 - Parent       : Parent node of XML where stream is present
* arg3 - Tag          : string need to be searched in the stream
*/
const char * const  SetConfig_GetText(mxml_node_t * Stream, mxml_node_t * tree, char const * const Tag)
{
    mxml_node_t        * node  = NULL;
    mxml_node_t        * value = NULL;

    if ((node = mxmlFindElement(Stream, tree, Tag, NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
    {
        return NULL;
    }
    if ((value = mxmlGetFirstChild(node)) == NULL)
    {
        return NULL;
    }
    if (value->type != MXML_TEXT)
    {
        return NULL;
    }
    return value->value.text.string;
}

static int erase_flash(void)
{
    struct rswitch_debug_memory  Register_New;
    int x = 0;
    int ret = 0;
    int sector_count = 0;
    gFDdebug = open(RSWITCH_DEBUG_DEB_NAME, O_RDWR | O_SYNC);

    /* For debugging of one access only
    Register_New.Address = 0x100500C;
    Register_New.Value = (Fpga_Update_Address - 0x04000000) / 0x100 + 0x02;
    Register_New.file_write_enable = 0;
    if ((ret = ioctl(gFDdebug, RSWITCH_DEBUG_IOCTL_WRITEBITFILE, &Register_New)) != 0)
        fprintf(stderr, "\nERROR: RSWITCH_DEBUG_IOCTL_WRITEMEMORY failed(%d) on address %08x: %s \n", ret, Register_New.Address, strerror(errno));

    return 0;
    */

    Fpga_Update_Address = Fpga_Flash_Memory_Offset + Fpga_Page1_Start_Address;
    printf("\tStarting Flash Memory Page 1 erasing from address 0x%08x to 0x%08x\n",Fpga_Page1_Start_Address, Fpga_Page1_Start_Address * 2);
    printf("\tErasing of the flash will take around %3d minutes\n", (((2 * (Fpga_Page1_Start_Address / 0x10000)) / 60) + 1));
    for(x = (Fpga_Update_Address - 0x04000000) / 0x100 + 0x02; x < (Fpga_Update_Address - 0x04000000) / 0x100 * 2 + 0x02; x=x+0x100)
    {
        printf("\t\tErasing Sector 0x%08x",Fpga_Page1_Start_Address + sector_count * 0x10000);
        Register_New.Address = 0x100500C;
        Register_New.Value = x;
        Register_New.file_write_enable = 0;
        if ((ret = ioctl(gFDdebug, RSWITCH_DEBUG_IOCTL_WRITEBITFILE, &Register_New)) != 0)
        {
            fprintf(stderr, "\nERROR: RSWITCH_DEBUG_IOCTL_WRITEMEMORY failed(%d): %s \n", ret, strerror(errno));
            Close_Drivers();
            return 3;
        }
        sleep(2);
        sector_count++;
        //printf(" - Sector Count %4d",sector_count);
        //printf(" - erased part so far %9d (0x%08x) bytes", sector_count * 0x10000, sector_count * 0x10000);
        printf(" -> %3.2f %% of the requested area is erased    \r",(float) sector_count * 0x10000 / Fpga_Page1_Start_Address * 100);
        fflush(stdout);
    }
    printf("\nErasing Flash finished\n\n");
    return 0;

}
/*
* Set_XML_type_cb() : Callback to load the XML file
*/
static mxml_type_t XML_type_cb(mxml_node_t * node)
{
    const char    * type;

    if ((type = mxmlElementGetAttr(node, "type")) == NULL)
    {
        type = node->value.element.name;
    }

    if (strcmp(type, "integer") == 0)
    {
        return (MXML_INTEGER);
    }
    else if ( (strcmp(type, "opaque") == 0) OR (strcmp(type, "pre") == 0) )
    {
        return (MXML_OPAQUE);
    }
    else if (strcmp(type, "real") == 0)
    {
        return (MXML_REAL);
    }
    else
    {
        return (MXML_TEXT);
    }
}

static bool Set_RUP(void)
{
    FILE    *bitfilefp = NULL;
    unsigned char *buffer;
    uint32_t filesz = 0;
    int i = 0;
    int ret = 0;
    uint32_t write_word = 0;
    bitfilefp = fopen(gOptBitFile, "rb");

    printf("\tConfiguring bitfile\n");

    struct rswitch_debug_memory  Register_New;
    if (bitfilefp == NULL)
    {
        fprintf(stderr, "\nERROR: unable to open BitFile '%s'-%s\n", gOptBitFile, strerror(errno));
        return FALSE;
    }
    fseek(bitfilefp, 0L, SEEK_END);
    filesz = ftell(bitfilefp);
    rewind(bitfilefp);
    buffer = (char*)malloc(filesz * sizeof(char));
    fread(buffer,filesz,1,bitfilefp);

    gFDdebug = open(RSWITCH_DEBUG_DEB_NAME, O_RDWR | O_SYNC);
    if (gFDdebug < 0)
    {
        fprintf(stderr, "\nERROR: Open '%s' failed: %d(%s) \n", RSWITCH_DEBUG_DEB_NAME, errno, strerror(errno));
        return FALSE;
    }

    printf("\tBitfile will be now stored at Flash Memory Page 1 on address 0x%08x\n",Fpga_Page1_Start_Address);
    for(i = 0; i < ((filesz+3)/4); i ++)
    {
        write_word =  buffer[(i*4)] | buffer[(i*4) + 1] << 8 | buffer[(i*4) + 2] << 16 | buffer[(i*4) + 3] << 24;
        Register_New.Address = Fpga_Update_Address + i*4;
        Register_New.Value = write_word;
        Register_New.file_write_enable = 0;
        if ((ret = ioctl(gFDdebug, RSWITCH_DEBUG_IOCTL_WRITEBITFILE, &Register_New)) != 0)
        {
            fprintf(stderr, "\nERROR: RSWITCH_DEBUG_IOCTL_WRITEMEMORY failed(%d): %s \n", ret, strerror(errno));
            Close_Drivers();
            return 3;
        }
    }
    printf("Bitfile stored\n\n");

    Close_Drivers();
    return 0;
}

/*
* PHY_Set_Config() : Load static configuration from user input XML configuration file
* arg1 - PortNode  : Port Node Name for TSN Port
*/
static bool PHY_Set_Config(const char * const PortNode)
{
    FILE        * XMLFp   = NULL;
    mxml_node_t * Tree    = NULL;
    mxml_node_t * GFTNode = NULL;
    mxml_node_t   * TopNode    = NULL;
    mxml_node_t   * FirstNode    = NULL;
    mxml_node_t   * PortNumber    = NULL;
    char const * ch = NULL;
    mxml_node_t *value = NULL;
    unsigned char  * PortChar[RENESAS_RSWITCH_MAX_ETHERNET_PORTS];
    char str1[40];
    char str2[40];


    /*Load the XML configuration file */
    XMLFp = fopen(gOptConfigFile, "r");
    if (XMLFp == NULL)
    {
        fprintf(stderr, "\nERROR: unable to open Configuration File '%s'-%s\n", gOptConfigFile, strerror(errno));
        return FALSE;
    }

    Tree = mxmlLoadFile(NULL, XMLFp, XML_type_cb);
    if (Tree == NULL)
    {
        fprintf(stderr, "\nERROR: Could not load XML from Configuration file '%s'\n", gOptConfigFile);
        return FALSE;
    }

    fclose(XMLFp);
    /* Get R-switch node*/
    if ((GFTNode = mxmlFindElement(Tree, Tree, "R-SWITCH", NULL, NULL, MXML_DESCEND)) == NULL)
    {
        fprintf(stderr, "\nERROR: Unable to find node <R-SWITCH> in '%s'\n", gOptConfigFile);
        return FALSE;
    }


    if ((TopNode = mxmlFindElement(GFTNode, GFTNode, PortNode, NULL, NULL, MXML_DESCEND_FIRST)) != NULL)
    {
        if((FirstNode = mxmlFindElement(TopNode, TopNode, "Port", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
        {
            fprintf(stderr, "\nERROR : No <Port> in <PHY-Config> in '%s'\n",
            gOptConfigFile);
            return FALSE;
        }

        for(; FirstNode != NULL; FirstNode = mxmlWalkNext(FirstNode, TopNode, MXML_NO_DESCEND))
        {
            if (FirstNode->type != MXML_ELEMENT)
            {
                continue;
            }

            if((PortNumber = mxmlFindElement(FirstNode, FirstNode, "PortNumber", NULL, NULL, MXML_DESCEND_FIRST)) == NULL)
            {
                fprintf(stderr, "\nERROR : No <PortNumber> in <Port> in <PHY-Config> in '%s'\n",
                gOptConfigFile);
                return FALSE;
            }

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

                PortChar[PhyCount] = value->value.text.string;

                if (sscanf(PortChar[PhyCount], "%u", &PhyPort[PhyCount]) != 1)
                {
                    fprintf(stderr, "\n Invalid  <PortNumber>");
                    return FALSE;
                }

                if (PhyPort[PhyCount] >= RENESAS_RSWITCH_MAX_ETHERNET_PORTS)
                {
                    fprintf(stderr, "\nInvalid  <PortNumber>, Range (0-%d)\n", (RENESAS_RSWITCH_MAX_ETHERNET_PORTS-1));
                    return FALSE;
                }
            }
            for(int i = 0; i < PhyCount; i++)
            {
                if(PhyPort[PhyCount] == PhyPort[i])
                {
                    fprintf(stderr, "\nERROR: Duplicate <PortNumber> '%d' in <Port> in <PHY-Config> in '%s'\n",
                    PhyPort[PhyCount], gOptConfigFile);
                    return FALSE;
                }
            }

            switch(PhyPort[PhyCount])
            {
            case 1 :
                strcpy(str1,"phytool write eth0/0");
                system("echo 0 > /sys/class/gpio/gpio497/value");
                system("echo 1 > /sys/class/gpio/gpio498/value");
                break;
            case 2 :
                strcpy(str1,"phytool write eth0/0");
                system("echo 1 > /sys/class/gpio/gpio497/value");
                system("echo 0 > /sys/class/gpio/gpio498/value");
                break;
            case 3 :
                strcpy(str1,"phytool write eth0/1");
                break;
            case 4 :
                strcpy(str1,"phytool write eth0/2");
                break;
            case 5 :
                strcpy(str1,"phytool write eth0/3");
                break;
            }
            strcpy(str2, str1);

            if((ch = SetConfig_GetText(FirstNode, FirstNode, "Mode")) != NULL)
            {
                if(strncasecmp(ch, "Master", 6) == 0)
                {
                    phy_mode[PhyCount] = rswitch_phy_mode_master;
                    strcat(str1, "/0x1F 0x0");
                    strcat(str2, "/0x9 0x800");
                }
                else if (strncasecmp(ch, "Slave", 5) == 0)
                {
                    phy_mode[PhyCount] = rswitch_phy_mode_slave;
                    strcat(str1, "/0x1F 0x0");
                    strcat(str2, "/0x9 0x0");
                }
                else
                {
                    fprintf(stderr, "\nInvalid <%s> - can only be \"%s\" or \"%s\"\n",
                    "Mode", "Master", "Slave");
                    return FALSE;
                }
            }
            system(str1);
            system (str2);
            printf("\n%s\n", str1);
            printf("%s\n", str2);
            PhyCount++;
        }
    }

    PHY_Report_Device();
    return TRUE;
}

/*
* Set_Config_From_XML() : Load static configuration from user input XML configuration file
*/
static bool Set_Config_From_XML(void)
{
    FILE        * XMLFp   = NULL;
    mxml_node_t * Tree    = NULL;
    mxml_node_t * GFTNode = NULL;

    /*Load the XML configuration file */
    XMLFp = fopen(gOptConfigFile, "r");
    if (XMLFp == NULL)
    {
        fprintf(stderr, "\nERROR: unable to open Configuration File '%s'-%s\n", gOptConfigFile, strerror(errno));
        return FALSE;
    }

    Tree = mxmlLoadFile(NULL, XMLFp, XML_type_cb);
    if (Tree == NULL)
    {
        fprintf(stderr, "\nERROR: Could not load XML from Configuration file '%s'\n", gOptConfigFile);
        return FALSE;
    }

    fclose(XMLFp);
    /* Get R-switch node*/
    if ((GFTNode = mxmlFindElement(Tree, Tree, "R-SWITCH", NULL, NULL, MXML_DESCEND)) == NULL)
    {
        fprintf(stderr, "\nERROR: Unable to find node <R-SWITCH> in '%s'\n", gOptConfigFile);
        return FALSE;
    }


    /*set TSN configuration*/
    if (gOptConfigureEth)
    {
        if (NOT TSN_Set_Config(GFTNode))
        {
            mxmlDelete(Tree);
            return FALSE;
        }
    }

    if (gOptConfigureFwd)
    {
        if (NOT FWD_Set_Config(GFTNode))
        {
            mxmlDelete(Tree);
            return FALSE;
        }
    }
    mxmlDelete(Tree);
    return TRUE;
}

/*
* Reset_Configurations() : Reset the module structures
*/
void Reset_Configurations(void)
{
    TSN_Reset_Configuration();
}



#ifdef BUILD_OPTION_DEBUG_FPGA

#define _REGISTERS_PER_LINE     (8)

static void _Debug_Print_HexLine(uint32_t StartAddress, uint32_t Number, uint32_t * Register)
{
    int         a = 0;

    printf("0x%08X  ", StartAddress);
    for (a = 0; a < Number; a++)
    {
        printf("%08X ", Register[a]);
        if (a && a%4==3)
        printf(" ");
    }
    printf("\n");
}
#endif /* BUILD_OPTION_DEBUG_FPGA */


/*
* Open_Drivers() : Open the driver file descriptors
*/
static bool Open_Drivers(void)
{
    bool Result = FALSE;

    if (gOptEth)
    {
        if (TSN_Open_Driver())
        {
            Result = TRUE;
        }
    }

    if (gOptFwd)
    {
        if (FWD_Open_Driver())
        {
            Result = TRUE;
        }
    }


#ifdef BUILD_OPTION_DEBUG_FPGA
    if (gOpt_OpenDebug)
    {
        gFDdebug = open(RSWITCH_DEBUG_DEB_NAME, O_RDWR | O_SYNC);
        if (gFDdebug < 0)
        {
            fprintf(stderr, "\nERROR: Open '%s' failed: %d(%s) \n", RSWITCH_DEBUG_DEB_NAME, errno, strerror(errno));
            Result = FALSE;
        }
        else
        {
            Result = TRUE;
        }
    }
#endif /* BUILD_OPTION_DEBUG_FPGA */

    return Result;
}




void eliminate_(char *ss)
{
    char *s;
    while ((s=strchr(optarg, '_')) != NULL) {
        strcpy(s, s+1);
    }
}



/*
* main()        : Initial main function to the r switch tool
* arg1 - argc   : Count for the user input arguements
* arg2 - argv   : pointer to user input arguements
*/
int main(int argc, char **argv)
{
    int OptCase = 0;
    int OptionIndex = 0;
    int  ret          = 0;
    char *OptArg = NULL;
#ifdef BUILD_OPTION_DEBUG_FPGA
    char  *fpga_heading = "FPGA Memory";
#endif /* BUILD_OPTION_DEBUG_FPGA */

    Reset_Configurations();    //Puts TSNConfig_t to zero

    gProgName = strchr(argv[0], '/');               //searches user input arguments for the character /
    gProgName = gProgName ? 1+gProgName : argv[0];  //if / is found, then?

    FILE *output = fopen("/etc/temp/temp.xml", "w");
    int length = 0;
    char ch[100000];
    int xml_end = 0;
    char check_for_end [12] = "</R-SWITCH>";

    while (EOF != (OptCase = getopt_long(argc, argv, "-hf:c:r:vx:y:X:b:p:e", LongOptions, &OptionIndex)))    //Reads user option input until it runs out
    {
        OptArg = optarg;
        switch (OptCase)    //determines which command was received from user
        {
        case 1:
            fprintf(stderr, "\nERROR: Unknown Parameter - %s\n", OptArg);
            usage();
            return 1;
        case 'b':
            printf("Storing new bitfile to Flash Memory Page 1 at 0x%x\n", Fpga_Page1_Start_Address);
            gOptBitFile = optarg;

            printf("\tInput Bit file : %s\n", gOptBitFile);    //after receiving the file name, outputs it to console
            erase_flash();
            Set_RUP();
            configbitfile = 1;
            break;
        case 'p':
            //eliminate_(optarg);
            printf("Setting new Flash Memory Page 1 Start Address\n");
            if (sscanf(optarg, "0x%x", &gOpt_FPGA_Address) != 1)
            {
                fprintf(stderr,  "\n\nERROR: command-line format is '-p 0xnnnn to set Flash Memory Page 1 Start Address' \n");
                usage();
                return 1;
            }
            Fpga_Page1_Start_Address = gOpt_FPGA_Address;
            if ((Fpga_Page1_Start_Address * 2) < 0x00010000)
            {
                fprintf(stderr,  "\n\nERROR: Start Address is too small; smallest Start Address is 0x00010000' \n");
                usage();
                return 1;
            }
            if ((Fpga_Page1_Start_Address * 2) > 0x7FFF0000)
            {
                fprintf(stderr,  "\n\nERROR: Start Address is too big; because of this the erasing would go out of memory' \n");
                usage();
                return 1;
            }
            printf("\tNew Flash Memory Page 1 Start Address is now 0x%08x\n\n", Fpga_Page1_Start_Address);
            page1_adr_is_set = 1;
            break;
        case 'e':
            printf("Erasing Flash Memory starting at 0x%x\n", Fpga_Page1_Start_Address);
            erase_flash();
            return 0;
            break;
        case 'c':
            gCmdConfigure = TRUE;

            if (strcmp("all", OptArg) == 0)
            {
                gOptConfigureEth = TRUE;    //Flag shows whether to configure ethernet or not
                gOptConfigureFwd = TRUE;    //Flag shows whether to configure forwarding engine or not
                gOptConfigurePhy = TRUE;
            }
            else if (strcmp("fwd", OptArg) == 0)
            {
                gOptConfigureFwd = TRUE;
            }
            else if (strcmp("eth", OptArg) == 0)
            {
                gOptConfigureEth = TRUE;
            }
            else if (strcmp("phy", OptArg) == 0)
            {
                gOptConfigurePhy = TRUE;
            }
            else
            {
                fprintf(stderr, "\nERROR: '%s' invalid Configure type \n", OptArg);
                usage();
                return 1;
            }

            if(EOF == (OptCase = getopt_long(argc, argv, "f:", LongOptions, &OptionIndex)))
            {
                //int u=0;
                while (!xml_end)
                {
                    ch[length++] = getchar();
                    if (*(ch+length-1) == '<')
                    {
                        for (int i = 0; i < 11; i++)
                        {
                            if (*(ch+length-1) != *(check_for_end+i))
                            break;
                            ch[length++] = getchar();
                            if (i == 10)
                            xml_end = TRUE;
                        }
                    }
                }
                ch[length-1] = '\0';
                fprintf(output, ch);
                fclose(output);
                gOptConfigFile = "/etc/temp/temp.xml";
            }
            else
            {
                gOptConfigFile = optarg;
                printf("Input configuration file : %s\n", gOptConfigFile);    //after receiving the file name, outputs it to console
            }
            break;
        case 'r':
            gCmdReport = TRUE;

            if (OptArg != NULL)
            {
                if (strcmp("all", OptArg) == 0)
                {
                    gOptReportEth = TRUE;    //Flag shows whether to report (question: report = display status or settings?) ethernet
                    gOptReportFwd = TRUE;    //Flag shows whether to configue forwarding engine or not (question: same as above)
                    gOptReportPhy = TRUE;
                }
                else if (strcmp("fwd", OptArg) == 0)
                {
                    gOptReportFwd = TRUE;
                }
                else if (strcmp("eth", OptArg) == 0)
                {
                    gOptReportEth = TRUE;
                }
                else if (strcmp("phy", OptArg) == 0)
                {
                    gOptReportPhy = TRUE;
                }
                else
                {
                    fprintf(stderr, "\nERROR: '%s' invalid Report type \n", OptArg);
                    usage();
                    return 1;
                }
            }
            else
            {
                gOptReportEth = TRUE;
                gOptReportFwd = TRUE;
            }

            break;
        case 'f':    //receives the file name put in by user
            gOptConfigFile = optarg;

            printf("Input configuration file : %s\n", gOptConfigFile);    //after receiving the file name, outputs it to console
            break;
        case 'v':
            printf("%s  Version : %s\n", gProgName, RSWITCHTOOL_VERSION);    //Displays the version
            return 0;
#ifdef BUILD_OPTION_DEBUG_FPGA
        case 'x':
        case 'X':
#ifdef BUILD_OPTION_DEBUG_FPGA_ALLOW_UPDATE
            eliminate_(optarg);
            if(sscanf(optarg, "0x%x=0x%x", &gOpt_FPGA_Register, &gOpt_FPGA_Value) != 2)    //reads register address and value from user input. sscanf returns the number of items successfuly scanned
            {
#endif /* BUILD_OPTION_DEBUG_FPGA_ALLOW_UPDATE */
                if(sscanf(optarg, "0x%x", &gOpt_FPGA_Register) != 1)
                {
                    fprintf(stderr, "\n\nERROR: format is\n");
                    fprintf(stderr, "  '-x 0xnnnn         to display current register value' \n");
#ifdef BUILD_OPTION_DEBUG_FPGA_ALLOW_UPDATE
                    fprintf(stderr, "  '-x 0xnnnn=0xnnnn' to set a new register value \n");
#endif /* BUILD_OPTION_DEBUG_FPGA_ALLOW_UPDATE */
                    usage();
                    return 1;
                }
                gCmd_ShowFPGA = TRUE;    //flag to show the specified register and its value
#ifdef BUILD_OPTION_DEBUG_FPGA_ALLOW_UPDATE
            }
            else
            {
                gCmd_SetFPGA = TRUE;    //flag to set the specified register by the specified value
            }
            if (OptCase == 'X') {
                gOpt_FPGA_RegisterEnd = gOpt_FPGA_Register+4;    //question: how does this cause memory dump?
            }
#endif /* BUILD_OPTION_DEBUG_FPGA_ALLOW_UPDATE */
            break;

        case 'y':
            eliminate_(optarg);
            if (sscanf(optarg, "0x%x", &gOpt_FPGA_RegisterEnd) != 1)
            {
                fprintf(stderr,  "\n\nERROR: command-line format is '-x 0xnnnn -y 0xnnnn to display current register values' \n"); //question: how does this work exactly?
                usage();
                return 1;
            }
            break;
#endif /* BUILD_OPTION_DEBUG_FPGA */
        case 'h':
            usage();
            return 0;
        default:
            fprintf(stderr, "\nERROR: Command-line error - unknown parameter\n");
            usage();
            return 1;
        }
    }

#ifdef BUILD_OPTION_DEBUG_FPGA
    if ( (gCmd_SetFPGA) OR (gCmd_ShowFPGA) )
    {
        gOpt_OpenDebug = TRUE;
    }
#endif /* BUILD_OPTION_DEBUG_FPGA */


    if ((gOptConfigureEth) OR (gOptReportEth))
    {
        gOptEth = TRUE;
    }

    if ((gOptConfigureFwd) OR (gOptReportFwd))
    {
        gOptFwd = TRUE;
    }

    if ((NOT gOptEth) AND (NOT gOptConfigurePhy) AND (NOT gOptReportPhy) AND (NOT gOpt_OpenDebug) AND (NOT gOptFwd) AND
            (NOT erasebitfile) AND (NOT configbitfile) AND (NOT page1_adr_is_set))    //checks whether anything with ethernet, forwarding engine or registers is being done
    {
        fprintf(stderr, "\n ERROR : At least '--c' or '--r' are required \n\n");    //have to select at least one of the three
        usage();
        return 1;
    }

    /* Set PHY configuration*/
    if (gOptConfigurePhy)
    {
        if (NOT PHY_Set_Config("PHY-Config"))
        {
            return FALSE;
        }
    }

    if (gOptReportPhy)
    {
        PHY_Report_Device();
    }

    /*Open Files for Device Driver IOCTLs*/
    if(NOT Open_Drivers())     //If any of the selected driver file descriptors do not open, output error
    {
        return 3;
    }

#ifdef BUILD_OPTION_DEBUG_FPGA
    if (gCmd_ShowFPGA)
    {
        struct rswitch_debug_memory Register;     //question: where to find rsw_debug_memory?

        if( gOpt_FPGA_RegisterEnd == 0)
        {
            Register.Address = gOpt_FPGA_Register;

            /*  Get a single FPGA register/memory word */
            if ((ret = ioctl(gFDdebug, RSWITCH_DEBUG_IOCTL_READMEMORY, &Register)) != 0)
            {
                fprintf(stderr, "\nERROR: RSWITCH_DEBUG_IOCTL_READMEMORY failed(%d): %s \n", ret, strerror(errno));
                Close_Drivers();
                return 3;
            }
            printf("FPGA Address(0x%04X) has value(0x%08X) \n", Register.Address, Register.Value);
        }
        else
        {
            uint32_t        x;
            unsigned int    Values = (gOpt_FPGA_RegisterEnd - gOpt_FPGA_Register) / 4;
            if (Values > sizeof(IPRAM))
            {
                fprintf(stderr, "\n\nERROR: Too many values. You can read up to %lu words\n", sizeof(IPRAM));
                Close_Drivers();
                return 1;
            }
            if (gOpt_FPGA_RegisterEnd < gOpt_FPGA_Register)
            {
                fprintf(stderr, "\n\nERROR: format is\n");
                fprintf(stderr, "  '-x 0x{low} -y 0x{high} to display current register/memory values' \n");
                usage();
                Close_Drivers();
                return 1;
            }
            for (x = 0; x < Values; x++)
            {
                Register.Address = gOpt_FPGA_Register + (x * 4);
                if ((ret = ioctl(gFDdebug, RSWITCH_DEBUG_IOCTL_READMEMORY, &Register)) != 0)
                {
                    fprintf(stderr, "\nERROR: RACE_DEBUG_IOCTL_READMEMORY failed(%d): %s \n", ret, strerror(errno));
                    Close_Drivers();
                    return 3;
                }
                IPRAM[x] = Register.Value;
            }
            if (fpga_heading != NULL && fpga_heading[0]) {
                printf("\n");
                printf("------------------------------------ %s ---------------------------------- \n", fpga_heading);
            }
            for (x = 0; x < Values; x += _REGISTERS_PER_LINE)
            {
                _Debug_Print_HexLine(gOpt_FPGA_Register + (x * 4),
                MIN(Values - x, _REGISTERS_PER_LINE),
                &IPRAM[x]);
            }
        }
        Close_Drivers();
        return 0;
    }
    if (gCmd_SetFPGA)
    {
        struct rswitch_debug_memory  Register_Old;
        struct rswitch_debug_memory  Register_New;
        Register_Old.Address = gOpt_FPGA_Register;
        Register_New.Address = gOpt_FPGA_Register;
        Register_New.Value   = gOpt_FPGA_Value;

        /* Set a single FPGA register  */
        ioctl(gFDdebug, RSWITCH_DEBUG_IOCTL_READMEMORY, &Register_Old);
        if ((ret = ioctl(gFDdebug, RSWITCH_DEBUG_IOCTL_WRITEMEMORY, &Register_New)) != 0)
        {
            fprintf(stderr, "\nERROR: RSWITCH_DEBUG_IOCTL_WRITEMEMORY failed(%d): %s \n", ret, strerror(errno));
            Close_Drivers();
            return 3;
        }
        printf("FPGA Address(0x%04X) value(0x%08X) set to (0x%08X)\n",
        Register_Old.Address, Register_Old.Value, Register_New.Value);
        Close_Drivers();
        return 0;
    }

#endif /* BUILD_OPTION_DEBUG_FPGA */

    if((gOptReportEth) OR (gOptReportFwd))
    {
        Report_Configurations();
    }

    if (gCmdConfigure)
    {
        if (gOptConfigFile != NULL)
        {
            if (NOT Set_Config_From_XML())
            {
                Close_Drivers();
                return 3;
            }
            else
            {

                if (NOT Configure_Devices())
                {
                    ret = -3;
                }
                Report_Configurations();
                if (ret < 0)
                {
                    Close_Drivers();
                    return ret;
                }
            }
        }

    }
    Close_Drivers();

    return 0;
}


/*
* Change History
* 21-Aug-2019 rswitchtool version 3.0.0
add command
            set new Start Address for bit file downloading to the flash and for erasing the flash (-p)
            erase the flash starting from start address (-e)
* 10-Oct-2019 rswitchtool version 3.0.1
make maxInterferenceSize optional
* 09-Aug-2020 rswitchtool version 3.0.2
exchange <Preempt-MinSize> by <addFragSize>
new value for this is now "0", "1", "2" or "3"
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
