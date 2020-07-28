/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <linux/types.h>
typedef unsigned char           uint8_t;
typedef unsigned short          uint16_t;
typedef unsigned int            uint32_t;
#define PROP_VALUE_MAX  92
#define RF_TEST_MODE_SWITCH_IDLE 1000000
#include "libwifitest.h"
#include "lib.h"
//#include <cutils/properties.h>
//#include <sys/system_properties.h>
#include <errno.h>
#include "libwifitest.h"
#define PACKAGE     "WifiHwTestTool"
//#define EEPROM_PATH		"/lib/firmware/EEPROM_MT7668.bin"

//char EEPROM_PATH[64] = "/vendor/firmware/EEPROM_MT7668.bin";

//char EEPROM_PATH_TMP[64] = "/tmp/EEPROM_MT7668.bin"; // tmp buffer

//char EEPROM_PATH_WRITE[64] = "/tmp/EEPROM_MT7668.bin";
char EEPROM_PATH_TMP[64] = "/vendor/firmware/EEPROM_MT7668.bin"; // tmp buffer
char EEPROM_PATH_WRITE[64] = "/vendor/firmware/EEPROM_MT7668.bin";

char proc_name[256];

/**************************************************
                COMPILER FLAG
**************************************************/
#define WIFI_TEST_TOOL_VER101             1  // VER101 : New command for Freq. Offset and Rx count
#define WIFI_TEST_TOOL_VER102             1  // VER102 : New Flow for Auto Isolation Detection


typedef enum {
	WLAN_MODE_OFF,
	NORMAL_MODE_ON,
	TEST_MODE_ON
}WlanStatus;

typedef enum {
	OPER_NONE,
	TEST_TX,
	TEST_RX,
	READ_EFUSE,
	READ_EFUSE_FREE_BLOCK,
	WRITE_EFUSE,
	WRITE_EFUSE_MAC_WIFI,
	WRITE_EFUSE_MAC_BT,
	READ_MCR,
	WRITE_MCR,
	TEST_STOP,
	QUERY_RESULT,
	SET_NSS,
	READ_EEPROM,
	WRITE_EEPROM,
	I_CAL,
	WRITE_EEPROM_TO_EFUSE,
	SET_RX_PATH,
	SET_FREQ_OFFSET,
	SET_TX_POWER_COMPENSATION,
	SET_TX_PATH,
	DUMP_EFUSE_ALL
}Oper_Mode;

typedef enum _ENUM_RX_MATCH_RULE_T {
    RX_MATCH_RULE_DISABLE,
    RX_MATCH_RULE_RA,           /* RA only */
    RX_MATCH_RULE_TA,           /* TA only */
    RX_MATCH_RULE_RA_TA,        /* Both RA and TA */
    RX_MATCH_RULE_NUM
} ENUM_RX_MATCH_RULE_T, *P_ENUM_RX_MATCH_RULE_T;

typedef enum _ENUM_BUFFER_MODE_SOURCE {
    SOURCE_AUTO = 0,
    SOURCE_EEPROMBIN,
    SOURCE_EFUSE,
} ENUM_BUFFER_MODE_SOURCE;

char *bg_rate[] = {
"RATE_AUTO",         
"RATE_1MBPS",
"RATE_2MBPS",        
"RATE_5_5MBPS",      
"RATE_6MBPS",        
"RATE_9MBPS",        
"RATE_11MBPS",      
"RATE_12MBPS",     
"RATE_18MBPS",     
"RATE_24MBPS",     
"RATE_36MBPS",       
"RATE_48MBPS",       
"RATE_54MBPS",      
};
char *preamble[] = {
"LONG",         
"SHORT",
};
char *bandwidth[] = {
"BW20",         
"BW40",
"BW20U",         
"BW20L",
"BW80",
"BW160"
};

char *bandwidthV2[] = {
"BW20",         
"BW40",
"BW80",
"BW160"
};



static void wifi_sensitivity(int, int);
static void wifi_tx();
static WlanStatus wifiStatus();
void wifiTestStop();
void wifiGetResult();
int open_eeprom_file();
bool WIFI_TEST_EEPROM_Write(unsigned int offset, unsigned int val);
bool WIFI_TEST_EEPROM_Read(unsigned int offset, unsigned int *val);
void replaceEEPROM();
void saveEEPROM();


void signal_handler(int sig) 
{
    int retval = 0;

    retval = WIFI_TEST_CloseDUT();
    printf("\n(%d) aborted .., sig=%d\n", retval,sig);

    signal(SIGINT, SIG_DFL);
    exit(0);
}

//  "E:e:M:g:G:I:B:R:N:T:m:i:s:p:b:t:hVw:v:l:f    :c:rn:"


void print_help(int exval)
{
    printf("Usage: %s [options]\n", proc_name);
    printf("\n");

    printf("<Test mode control>\n");
    printf("    %s -O                       Start Wi-Fi test mode\n", proc_name);
    printf("    %s -C                       Open Default buffer bin\n", proc_name);  
    printf("\n");
    
    printf("<MCR read/write>\n");    
    printf("    %s [-M addr]                Read value from CR address\n", proc_name);
    printf("    %s [-w addr] [-v value]     Write value to CR address\n", proc_name);
    printf("\n");
    
    printf("<EFUSE read/write>\n"); 
    printf("    %s [-E offset]              Read value from EFUSE at offset\n", proc_name);
    printf("    %s [-e offset] [-v value]   Write value to EFUSE at offset\n", proc_name);
    printf("\n");

    printf("<EEPROM read/write>\n");
    printf("    %s [-U offset]              Read value from EEPROM at offset\n", proc_name);
    printf("    %s [-u offset] [-v value]   Write value to EEPROM at offset\n", proc_name);
    printf("\n");


    printf("<Tx test>\n");
    printf("A/B/G Mode:\n");
    printf("    %s [-t 0] [-R legacy rate] [-s preamble] [options]\n", proc_name); 

    printf("N Mode:\n");
    printf("    %s [-t 1] [-N MCS rate] [-g greenfield] [-G SGI] [options]\n", proc_name); 

    printf("AC Mode:\n");
    printf("    %s [-t 2] [-N MCS rate] [-G SGI] [options]\n", proc_name); 
    printf("\n");
    
    printf("<Rx test>\n"); 
    printf("    %s [-r] [-n time] [options]\n", proc_name);
    printf("\n");

    printf("Common for Tx/Rx:\n"); 
    printf("    -c #           Central channel number\n");
    printf("    -b [0~3]       RF bandwidth <0:20/1:40/2:80/3:160>Mhz <default 20Mhz>\n");
    printf("    -P [0~7]       Primary channel setting in unit of 20Mhz <default 0>\n");
    printf("    -B [0~3]       Bandwidth <0:20/1:40/2:20U/3:20L>Mhz (Legacy commaand, *deprecated)\n");
    printf("    -j [0~2]       J mode setting <0:disable/1:5Mhz/2:10Mhz>\n");
    printf("    -d [0/1]       Set Rx default antenna <0:main/1:AUX>\n");
    
    printf("    -S #           Test mode <0:non-blocking/others:blocking mode timeout in seconds>\n");    
    printf("    -T             Test terminate command for non-blocking test\n");
    printf("    -a #           Blocking mode test result query interval in seconds\n");
    printf("    -o #           Max Tx/Rx packet count in blocking mode test\n");
    
    printf("    -q             Query test result\n");

    printf("    -D             Enable debug mode(dump AT command sequence)\n");   
	
    printf("    -f             RX Filter type <0:default,Disalbe,1:filter RA>\n");
    printf("    -A             Set RA address on enabling RX Filter. ex:-A 123456789ABC is set mac 12:34:56:78:9A:BC to RA address\n");
    printf("\n");
    
    printf("Rx specific:\n");
    printf("    -n #           Test time in seconds.\n");    
    printf("    -Q [1/2/3]     Set Rx Path <1:Rx0 1/2:Rx1/3:Rx0+Rx1> \n");
    printf("\n");
    
    printf("Tx specific:\n");

	printf("    -n #           TX Packet number, 0 is meaning that TX Packet number = unlimited\n");
	
    printf("    -t [0/1/2]     Tx mode <0:11abg/1:11n/2:11ac>\n");
    printf("    -x [0~3]       Tx bandwidth <0:20/1:40/2:80/3:160>Mhz <default follow RF BW>\n");
    printf("    -p #           Tx gain in dBm\n");

	
    
    printf("    -n #           Frame count\n");
    printf("    -l #           Frame length in bytes\n");    
    printf("    -i #           Frame burst interval in TU\n"); 
    
    printf("    -R [1~12]      Legacy rate code\n"); 
    printf("                   <1M/2M/5.5M/6M/9M/11M/12M/18M/24M/36M/48M/54M>\n");
    printf("    -s [0/1]       <0:short/1:long> preamble\n");

    printf("    -N [0~15/32]   MCS rate index\n");
    printf("    -g [0/1]       <0:mixed mode/1:greenfield> \n");
    printf("    -G [0/1]       <0:normal/1:short> guard interval\n");
    printf("    -L             Enable LDPC <default BCC>\n");
    
    printf("    -m [0/3]       <0:disable/3:enable> continuous waveform mode\n");
    printf("    -# [1/2]       <1:NSS 1/2:NSS 2> \n");
    printf("    -y [1/2/3]     Set Tx Path <1:Tx0 1/2:Tx1/3:Tx0+Tx1> \n");
    printf("\n");

    printf("    -V #     Set Freq Offset \n");
    printf("    -J 1     I_Cal mode \n");
    printf("    -K 1     Write EEPROM bin file to efuse \n");
    printf("    -F [path for EEPROM_MT7668.bin]     change EEPROM bin file folder path, default is /lib/firmware/EEPROM_MT7668.bin\n");
    printf("    -X 1     Save bin file to EEPROM path\n");



    exit(exval);
}

static int channel = 1;
static int times = 10;
//static int numBurst = 0;
static int txMode = 0;
static unsigned char macAddr[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
static float txGain = 10;
static int payloadLength = 1024;
static int SIFS = 20;
static int g_rate = 6;
static ENUM_WIFI_TEST_MCS_RATE gMCSrate = WIFI_TEST_MCS_RATE_0;
static int g_bandwidth = WIFI_TEST_BW_20MHZ;
static ENUM_WIFI_TEST_PREAMBLE_TYPE gMode = WIFI_TEST_PREAMBLE_TYPE_MIXED_MODE;
static ENUM_WIFI_TEST_GI_TYPE giType = WIFI_TEST_GI_TYPE_NORMAL_GI;
extern char WIFI_IF_NAME[256];
static WIFI_PreambleType_t pType = WIFI_TEST_PREAMBLE_SHORT;
static unsigned int mcr_addr = 0;
static unsigned int mcr_value = 0;
static unsigned int efuse_addr = 0;
static unsigned int eeprom_addr = 0;
static unsigned int rx_path = 0x1;
static unsigned int freq_offset = 0;
static unsigned int tx_power_compensation = 0;
/* we use bit data to tranfser,
   In detail => [31:24]:Reserved
                [23:20]: Tx0/Tx1
                [19:18]: 2G/5G
                [17:8]: Channel#
                [7:0]: channelpower compensataion */
static unsigned int tx_power_compensation_data = 0;
static int cw_mode = -1;
static int sleep_time = 10;
static bool sleepModeSet = false;
static int NSS = 1;

static int priSetting = 0;
static bool isNewBwSet = false;
static ENUM_WIFI_CHANNEL_BANDWIDTH rfBw = WIFI_TEST_CH_BW_20MHZ;
static bool isTxBwSet = false;
static ENUM_WIFI_CHANNEL_BANDWIDTH txBw = WIFI_TEST_CH_BW_20MHZ;
static int coding = 0; /* BCC */
static int rxDefaultAnt = 0;
static int jModeSetting = 0;
static int printInterval = 1;
static uint32_t maxPktCount = 0;
static int user_expect = 0;
static int current_mode = 0;
static int buffer_mode_content_source = 0;
static ENUM_RX_MATCH_RULE_T eRxOkMatchRule = RX_MATCH_RULE_DISABLE;
static bool bRxFilterMacAddrLegalFg = false;
static unsigned char aucRxFilterMacAddr[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; 

static uint32_t u4DefaultRxPath = 0x00030000;
static uint32_t u4DefaultTxPath = 0x1;
static uint32_t u4Dbdc = 0x0;
static uint32_t u4DbdcBandIndex = 0x0;
static uint32_t u4Cbw = 0x0;
static uint32_t u4Dbw = 0x0;
static uint32_t u4PriCh = 0x0;
static uint32_t u4Band = 0;
static uint32_t u4MacHeader = 0x1;
static uint32_t u4Payload = 65706;
static uint32_t u4Ta = 0x0;
static uint32_t u4Stbc = 0x0;
static uint32_t u4Ibf = 0x0;
static uint32_t u4Ebf = 0x0;

static uint8_t efuse_write_mac_bytes[6] = {};

#if WIFI_TEST_TOOL_VER101
static uint32_t u4DefaultCalFreqOffset = 0x0;
#endif
#if WIFI_TEST_TOOL_VER102
/* Auto Isolation Feature */
static unsigned int Isolation_val = 0;
static float BTTxPower = 0;
static bool SetBTTxFlag = false;
static bool SetIsoFlag = false;
#endif

#define WIFI_TEST_BW_MAX 5

extern bool fgDebugMode;
unsigned char uacEEPROMImage[MAX_EEPROM_BUFFER_SIZE];

int main(int argc, char *argv[]) 
{
    int opt = 0;
    int result = 0;
    uint16_t index=0, loop_index=0;
    Oper_Mode operation = OPER_NONE;
    WlanStatus wlan_status = 0;
    char input[20];
	int band_width[] = {WIFI_TEST_BW_20MHZ, WIFI_TEST_BW_40MHZ, 
        WIFI_TEST_BW_U20MHZ, WIFI_TEST_BW_D20MHZ, WIFI_TEST_BW_80MHZ,
        WIFI_TEST_BW_160MHZ};
    char *pch;
    int count=0,tmp=0;

#if 1
    char *Input_CMD[40];

    for (loop_index; loop_index<40; loop_index++)
    {
        Input_CMD[loop_index] = calloc(15,sizeof(char));
    }
#endif

    //open_eeprom_file();

//    while(1){

        /* Initial value*/
#if 0
        count = 0;
        opt = 0;
        operation = OPER_NONE;
        tx_power_compensation_data = 0;
        optind = 1; /* We need to reset this value, for the pointer next time to the next beginning */

        for (loop_index; loop_index<40; loop_index++)
        {
            Input_CMD[loop_index] = memset(Input_CMD[loop_index] , 0, sizeof(char)*15);
        }
#endif


#if 0
        printf("channel = %d, u4DefaultTxPath = %d, u4Cbw=%d", channel, u4DefaultTxPath, u4Cbw);
        printf("\nPlease input CMD \n");
        gets(input);
        //printf("input is %s\n", input);

        pch = strtok(input, " ");

        while (pch != NULL)
        {
            strcpy(Input_CMD[count],pch);
            //printf ("count=%d, string=%s\n",count,Input_CMD[count]);
            pch = strtok (NULL, " ");
            count++;
        }
#endif

        strncpy(proc_name, argv[0], 255);
        proc_name[255] = '\0';




        if (argc == 1){
        //if (count == 1){
            fprintf(stderr, "Needs arguments....\n\n");
            print_help(1);
        }

    	// set up the Ctrl + C handler
    	signal(SIGINT, signal_handler);
        while ((opt = getopt(argc, argv, "A:f:E:e:U:u:M:g:G:F:J:K:Q:V:W:X:#:$:y:Y:I:B:R:N:Tm:i:S:s:p:b:t:hVw:v:k:l:f:c:rOCn:DP:x:Ld:j:qa:oz:ZH:")) != -1) {
        //while ((tmp = opt = getopt(count, Input_CMD, "A:f:E:e:U:u:M:g:G:F:J:K:Q:V:W:X:#:y:Y:I:B:R:N:Tm:i:S:s:p:b:t:hVw:v:k:l:f:c:rOCn:DP:x:Ld:j:qa:o")) != -1) {
    	    switch(opt) {
    	        case 'e':
    	        	if (operation == OPER_NONE){
    		            operation = WRITE_EFUSE;
    		            xtoi(optarg, &efuse_addr);
                        printf("efuse_addr = %x\n", efuse_addr);
    		        }
    	            break;
    	        case 'E':
    	        	if (operation == OPER_NONE){
    		            operation = READ_EFUSE;
    		            xtoi(optarg, &efuse_addr);
    	            }
    	            break;
    	        case 'w':
    	        	if (operation == OPER_NONE){
    		            operation = WRITE_MCR;
    		            xtoi(optarg, &mcr_addr);
    	            }
    	            break;
    	        case 'M':
    	        	if (operation == OPER_NONE){
    		            operation = READ_MCR;
    		            xtoi(optarg, &mcr_addr);
    	            }
    	            break;
    	        case 'r':
    	        	if (operation == OPER_NONE)
    	            	operation = TEST_RX;
    	            break;
    			case 't':
    				if (operation == OPER_NONE)
    					operation = TEST_TX;
    				txMode = atoi(optarg);
    				break;

    	        case 'q':
    				if (operation == OPER_NONE)
    					operation = QUERY_RESULT;
    	            break;

    	        case 'u':
    	        	if (operation == OPER_NONE){
    		            operation = WRITE_EEPROM;
    		            xtoi(optarg, &eeprom_addr);
                        printf("stage 111111 \n");
    		        }
    	            break;
    	        case 'U':
    	        	if (operation == OPER_NONE){
    		            operation = READ_EEPROM;
    		            xtoi(optarg, &eeprom_addr);
    	            }
    	            break;

    	        case 'g':
    	        	gMode = !atoi(optarg) ? WIFI_TEST_PREAMBLE_TYPE_MIXED_MODE:WIFI_TEST_PREAMBLE_TYPE_GREENFIELD;
    	            break;
    	        case 'G':
    	        	giType = !atoi(optarg) ? WIFI_TEST_GI_TYPE_NORMAL_GI:WIFI_TEST_GI_TYPE_SHORT_GI;
    	            break;
    	        case 'I':
    	            strcpy(WIFI_IF_NAME, optarg);
    	            break;
    	        case 'F':
                    memset(EEPROM_PATH_WRITE, 0, sizeof(EEPROM_PATH_WRITE));
                    //printf("path name is 11 %s\n", EEPROM_PATH);
    	            strcpy(EEPROM_PATH_WRITE, optarg);
                    //printf("path name is 22 %s\n", EEPROM_PATH);
                    //open_eeprom_file();
    	            break;
    	        case 'J':
                    printf("i-cal mode \n");
    	        	if (operation == OPER_NONE){
    		            operation = I_CAL;
                        buffer_mode_content_source = atoi(optarg);
    		        }
    	            break;

                case 'K':
                    if (operation == OPER_NONE)
                        operation = WRITE_EEPROM_TO_EFUSE;
                    break;

                case 'W':
                    if (operation == OPER_NONE) {
                        operation = SET_TX_POWER_COMPENSATION;
    		            xtoi(optarg, &tx_power_compensation);
                    }
                    break;

                case 'Q':
    	        	//if (operation == OPER_NONE){
    		            //operation = SET_RX_PATH;
    		            xtoi(optarg, &rx_path);
    	            //}
    	            break;

                case 'y':
    	        	//if (operation == OPER_NONE){
    		            //operation = SET_TX_PATH;
    		            xtoi(optarg, &u4DefaultTxPath);
    	            //}
    	            break;

                case 'Y':
                    u4Cbw = atoi(optarg);
    	            break;

    	        case 'X':
                    open_eeprom_file(); //read from tmp buffer
                    saveEEPROM();
    	            break;


    	        case 'B':
    	        	{
    	        		int index = atoi(optarg);
    	        		if (index > WIFI_TEST_BW_MAX){
    	        			printf("not support this band");
    	        			return -1;
    	        		}
    					g_bandwidth = band_width[index];
    		            break;
    				}
    	        case 'N':
                    gMCSrate = atoi(optarg);
    	            break;
    	        case 'R':
    	            g_rate = atoi(optarg);
    	            break;
    	        case 'i':
    	            SIFS = atoi(optarg);
    	            break;
    	        case 'p':
    	            txGain = atof(optarg);
#if WIFI_TEST_TOOL_VER102 
                    SetBTTxFlag = true;
                    BTTxPower = atof(optarg);
#endif    	            
    	            break;
    	        case 'l':
    	            payloadLength = atoi(optarg);
    	            break;

    	        case 'b':
                    rfBw = atoi(optarg);
                    isNewBwSet = true;
    	            break;

    	        case 'j':
                    jModeSetting = atoi(optarg);
    	            break;

    	        case 'P':
                    priSetting = atoi(optarg);
    	            break;

    	        case 'x':
                    txBw = atoi(optarg);
                    isTxBwSet = true;
    	            break;

    	        case 'L':
                    coding = 1;
    	            break;

    	        case 'd':
                    rxDefaultAnt = atoi(optarg);
    	            break;

    	        case 'a':
                    printInterval = atoi(optarg);
    	            break;

    	        case 'o':
                    maxPktCount = atoi(optarg);
    	            break;

    	        case 'h':
    	        case ':':
    	            print_help(0);
    	            break;
    	        case 'n':
    	            times = atoi(optarg);
    	            break;
    	        case 'c':
    	            channel = atoi(optarg);
    	            break;
    	        case 'V':
    	        	if (operation == OPER_NONE){
    		            operation = SET_FREQ_OFFSET;
    		            xtoi(optarg, &freq_offset);
    	            }
    	            break;
    	        case 'v':
    	            xtoi(optarg, &mcr_value);
    	            break;
    	        case 's':
    	            pType = !atoi(optarg) ? WIFI_TEST_PREAMBLE_SHORT:WIFI_TEST_PREAMBLE_LONG;
    				break;
    	    	case 'm':
    	            cw_mode = atoi(optarg);
    	            break;
    	        case '?':
    	            fprintf(stderr, "%s: Error - No such option: `%c`\r", proc_name, optopt);
    	            print_help(1);
    	            break;
    			case 'S':
    				sleep_time = atoi(optarg);
                    sleepModeSet = true;
    				break;
    			case 'O':
				#if 0
    				if (access(EEPROM_PATH, R_OK)==0)
    				{
    					int source = open(EEPROM_PATH, O_RDONLY, 0);
    					int dest = open(EEPROM_PATH_TMP, O_RDWR | O_CREAT, 0777);
    					struct stat stat_source;
    					fstat(source, &stat_source);

    					sendfile(dest, source, 0, stat_source.st_size);

    					close(source);
    					close(dest);
    					fprintf(stderr, "copy from %s to %s\n", EEPROM_PATH, EEPROM_PATH_TMP);
    					if (access(EEPROM_PATH_TMP, R_OK|W_OK)!=0) {
    						fprintf(stderr, "%s is not ready\n", EEPROM_PATH_TMP);
    						exit(1);
    					}
    					fprintf(stderr, "%s is ready\n", EEPROM_PATH_TMP);
    				}
    				else{
    					fprintf(stderr, "%s is not valid\n", EEPROM_PATH);
    					exit(1);
    				}    			
				#else
    				if (access(EEPROM_PATH_TMP, R_OK)==0)
    				{
    					fprintf(stderr, "%s is ready\n", EEPROM_PATH_TMP);
    				}
    				else{
    					fprintf(stderr, "%s is not valid\n", EEPROM_PATH_TMP);
    					exit(1);
    				} 				
				#endif
    				user_expect = 1;
    				break;
    			case 'C':
    				open_eeprom_file();
    				break;
    			case 'T':
    				//operation = TEST_STOP;
    				SetIsoFlag = true;
    				break;
            	case 'D':
    	            fgDebugMode = true;
    	            break;
    	    	case 'f':
    	            eRxOkMatchRule = atoi(optarg);
    	            break;

                case '#':
                    //if (operation == OPER_NONE){
    		        //    operation = SET_NSS;  // add one more operaton?
    		            //xtoi(optarg, &NSS);
                        NSS = atoi(optarg);
    	        	    printf("NSS = %d, %x\n", NSS, NSS);
    		        //}
                    break;

    	    	case 'A':
    	            result = xtoAddrptr(optarg, aucRxFilterMacAddr);
    	            if (!result)
    	            {
    	        	    printf("Address format doesn't support\n");
    	        	    return -1;
    	            }
    	            else
    	            {
    	        	    bRxFilterMacAddrLegalFg = true;
    	            }
    	            break;
                case 'k':
                    result = xtoAddrptr(optarg, macAddr);
                    if (!result)
                    {
                        printf("Address format doesn't support\n");
                        return -1;
                    }
                    break;
                case 'z':
                    {
                    	int values[6];
                    	int idx;
                    	char mac_type;

                    	if (strlen(optarg)!=sizeof("B 00:11:22:33:44:55")-1) {
                    		printf("Invalid input format (%s)\n", optarg);
                    		return -1;
                    	}

                    	mac_type = optarg[0];
                    	if (mac_type != 'W' && mac_type != 'B') {
                    		printf("Invalid target type (%c)\n", mac_type);
                    		return -1;
                    	}

                    	if (mac_type == 'W')operation = WRITE_EFUSE_MAC_WIFI;
                    	if (mac_type == 'B')operation = WRITE_EFUSE_MAC_BT;

                    	/* parse MAC address string */
                    	if( 6 != sscanf( optarg+2, "%x:%x:%x:%x:%x:%x",
                    	    &values[0], &values[1], &values[2],
                    	    &values[3], &values[4], &values[5] ) ) {
                    		printf("Cannot parse mac address (%s)\n", optarg+2);
                    		return -1;
                    	}
                    	/* convert to uint8_t */
                        for( idx = 0; idx < 6; ++idx ) {
                            efuse_write_mac_bytes[idx] = (uint8_t) values[idx];
                    	}
                    }
        			break;
                case 'Z':
                    operation = READ_EFUSE_FREE_BLOCK;
        			break;

                case 'H':
                    {
                        //operation = DUMP_EFUSE_ALL;
                        current_mode = atoi(optarg);
                        printf("current_mode is %d\n", current_mode);

                        WIFI_TEST_init();

                        if (current_mode == 1) //dump from efuse
                        {
                            /* 1. Set to eeprom efuse mode */
                            WIFI_TEST_set_Eeprom_Mode(EFUSE_MODE);

                            /* 2. Read back all the efuse contents */
                            /* 1024/16 = 64*/
                            for (index=0; index<64 ; index++)
                            {
                                WIFI_TEST_get_HQA(index*EFUSE_BYTES_PER_LINE, index, uacEEPROMImage+index*EFUSE_BYTES_PER_LINE);
                            }
                            printf("Dump from efuse:\n");
                        }
                        else if (current_mode == 2) //dump from tmp eeprom
                        {
                            open_eeprom_file();
                            printf("Dump from eeprom:\n");
                        }
                        else
                        {
                            printf("Not Support this mode!!\n");
                            break;
                        }
                        printf("       0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F\n");
                        for (index=0; index<MAX_EEPROM_BUFFER_SIZE; index+=16)
                        {
                            printf("%3x    %02x  %02x  %02x  %02x  %02x  %02x  %02x  %02x  %02x  %02x  %02x  %02x  %02x  %02x  %02x  %02x\n", index,
                             uacEEPROMImage[index]  ,  uacEEPROMImage[index+1],  uacEEPROMImage[index+2],   uacEEPROMImage[index+3],
                             uacEEPROMImage[index+4],  uacEEPROMImage[index+5],  uacEEPROMImage[index+6],   uacEEPROMImage[index+7],
                             uacEEPROMImage[index+8],  uacEEPROMImage[index+9],  uacEEPROMImage[index+10], uacEEPROMImage[index+11],
                            uacEEPROMImage[index+12], uacEEPROMImage[index+13],  uacEEPROMImage[index+14], uacEEPROMImage[index+15]);
                        }
                    }
                    break;
#if WIFI_TEST_TOOL_VER101                    
                case '$':
                    u4DefaultCalFreqOffset = atoi(optarg);
                    break;
#endif     
                default:
        			printf("No case meet !!!\n");
        			break;
    	    }
    	}


        /* Decide RF/Tx bandwidth */
        if(isNewBwSet) {
            if(!isTxBwSet) {
                txBw = rfBw;
            }
            else if(txBw > rfBw){
                txBw = rfBw;
            }
        }
        else if(isTxBwSet) {
            isNewBwSet = true;
            rfBw = txBw;
        }

        /* BW coding check */
        if((txBw >= WIFI_TEST_CH_BW_NUM) || (rfBw >= WIFI_TEST_CH_BW_NUM)) {
            printf("Invalid bandwidth setting RF[%u] Tx[%u]", rfBw, txBw);
            return -1;
        }

    	wlan_status = wifiStatus();
    	switch (wlan_status){
    		case WLAN_MODE_OFF:
    			if ((user_expect & 0x1) == 1){
    				bool ret = false;
    				if ((user_expect & 0x2)==0x2 && operation == OPER_NONE){
    					return 0;
    				}
    				ret = WIFI_TEST_OpenDUT();
    				printf("[%s] Enable Wi-Fi test mode %s\n",
                        WIFI_IF_NAME, ret==true ? "success":"fail");
    				if (ret == true){
    					wlan_status = TEST_MODE_ON;
    					break;
    				}
    			}
    			printf("[%s] Not in test mode, use -O to enable.\n",
                    WIFI_IF_NAME);
    			return 0;

    		case TEST_MODE_ON:
    			if ((user_expect & 0x1) == 1)
    			{
    				unsigned int free_block;
    				bool retval;
    				unsigned char counter = 10;

    				do {
    					if (WIFI_TEST_OpenDUT() == false) {
    						usleep(100*1000);
    						continue;
    					}

    					retval = WIFI_TEST_GetFreeEfuseBlock(&free_block);
    					if (free_block==0 || !retval) { // result is something wrong. Retry again
    						usleep(100*1000);
    						continue;
    					}
    					// Step here means init is fully ready. Stop retry process
    					break;
    				} while (counter--);
    				if (counter<=0) {
    					printf("[%s] Test mode init fail\n", WIFI_IF_NAME);
    					exit(1);
    				}
    				printf("[%s] Already in test mode\n", WIFI_IF_NAME);
    			}
    			break;

    		case NORMAL_MODE_ON:
    			printf("Please turn off normal mode wlan first!\n");
    			return 0;

            default:
    			printf("No case meet !!!\n");
    			break;
    	}


    	WIFI_TEST_init();

        /* J mode setting */
        if(jModeSetting) {
            bool retval = false;
            retval = WIFI_TEST_SetJMode(jModeSetting);
            printf("(%s) Set J mode to %d\n", retval ? "success":"fail",
                jModeSetting);
        	if (retval == 0) return -1;
        }


    	switch (operation){
    		case READ_EFUSE_FREE_BLOCK:
    		{
    			unsigned int free_block;
    			bool retval = WIFI_TEST_GetFreeEfuseBlock(&free_block);
    			printf("(%s) Read EFUSE Free block 0x%02X\n", retval ? "success":"fail", free_block);
    			break;
    		}
    		case WRITE_EFUSE_MAC_WIFI:
    		{
    			bool retval = WIFI_TEST_MAC_Write(0x4, efuse_write_mac_bytes, sizeof(efuse_write_mac_bytes));
    			printf("(%s) Write EFUSE WIFI MAC\n", retval ? "success":"fail");
    			break;
    		}
    		case WRITE_EFUSE_MAC_BT:
    		{
    			bool retval = WIFI_TEST_MAC_Write(0x384, efuse_write_mac_bytes, sizeof(efuse_write_mac_bytes));
    			printf("(%s) Write EFUSE BT MAC\n", retval ? "success":"fail");
    			break;
    		}
    		case WRITE_EFUSE:
    		{
    			bool retval = WIFI_TEST_EFUSE_Write(efuse_addr, mcr_value);
    	        printf("(%s) Wirte EFUSE addr 0x%x value 0x%x\n", retval ? "success":"fail", efuse_addr, mcr_value);
    	        break;
    		}
    		case READ_EFUSE:
    		{
    			unsigned int val = 0;
    	        bool retval = WIFI_TEST_EFUSE_Read(efuse_addr, &val);
    	        printf("(%s) EFUSE addr 0x%x value 0x%02x\n", retval ? "success":"fail", efuse_addr, val);
    	        break;
    	    }
    		case WRITE_EEPROM:
    		{
                open_eeprom_file();
    			bool retval = WIFI_TEST_EEPROM_Write(eeprom_addr, mcr_value);
    	        printf("(%s) Wirte EEPROM addr 0x%x value 0x%02x\n", retval ? "success":"fail", eeprom_addr, mcr_value);
    	        break;
    		}
    		case READ_EEPROM:
    		{
    			unsigned int val = 0;
                open_eeprom_file();
    	        bool retval = WIFI_TEST_EEPROM_Read(eeprom_addr, &val);
    	        printf("(%s) EEPROM addr 0x%x value 0x%02x\n", retval ? "success":"fail", eeprom_addr, val);
    	        break;
    	    }
            case I_CAL:
            {
    			unsigned int val = 0;
                printf("buffer_mode_content_source = %d\n", buffer_mode_content_source);
                if (buffer_mode_content_source == SOURCE_AUTO)
                {
                    /* Read address 0x00 to check it is a re-cal ic or not */
           	        bool retval = WIFI_TEST_EFUSE_Read(0x00, &val);
                    printf("0x00 is %x\n", val);

                    if(val != 0x68)  // non-cal I-cal IC
                        buffer_mode_content_source = SOURCE_EEPROMBIN;
                    else  //re-cal IC
                        buffer_mode_content_source = SOURCE_EFUSE;
                }

                if (buffer_mode_content_source == SOURCE_EEPROMBIN)
                {
        			printf("Source from eeprom bin\n");
                    open_eeprom_file();

                    // 2017.03.16 We don't need to merget the i-cal fields anymore, due to we use patch to protect them.
                    //replaceEEPROM();

                    /* 1. Set to eeprom bin mode */
                    WIFI_TEST_set_Eeprom_Mode(BUFFER_BIN_MODE);

                    /* 2. Update the contents in driver's eeprom table from EEPROM path */
                    /* 1024/16 = 64*/
                    for (index=0; index<64 ; index++)
                    {
                        WIFI_TEST_set_HQA(index*EFUSE_BYTES_PER_LINE, index, uacEEPROMImage+index*EFUSE_BYTES_PER_LINE);
                    }

                    /* 3. Send buffer mode CMD through driver to FW */
                    WIFI_TEST_set_Efuse_Buffer_Mode(1);  //1 // 1: buffer mode CMD source from eeprom.bin
                }
                else if (buffer_mode_content_source == SOURCE_EFUSE)
                {
        			printf("Source from efuse\n");
                    WIFI_TEST_set_Efuse_Buffer_Mode(0);  //1 // 0: buffer mode CMD source from EFUSE
                }
                else
                {
                    printf("Not Support!!!!\n");
                    break;
                }

                break;
            }
            case WRITE_EEPROM_TO_EFUSE:
            {
    			unsigned int val = 0;

                /* Set to efuse mode */
                WIFI_TEST_set_Eeprom_Mode(EFUSE_MODE);

                for (index=0; index<64 ; index++)  /* 1024/16 = 64*/
                {
                    WIFI_TEST_set_HQA(index*EFUSE_BYTES_PER_LINE, index, uacEEPROMImage+index*EFUSE_BYTES_PER_LINE);
                }
                break;
            }
    		case SET_RX_PATH:
    		{
                if (rx_path == 0x1) /*WF0*/
                {
                    u4DefaultRxPath = 0x00010000;
                }
                else if (rx_path == 0x2) /*WF1*/
                {
                    u4DefaultRxPath = 0x00020000;
                }
                else if (rx_path == 0x3) /*WF0 + WF1*/
                {
                    u4DefaultRxPath = 0x00030000;
                }
                else
                {
                    printf("RX path not support!!!\n");
                    break;
                }

    			bool retval = WIFI_TEST_SetRxPath(u4DefaultRxPath);
    	        printf("(%s) RX path is set to value 0x%x\n", retval ? "success":"fail", rx_path);
    	        break;
    		}

    		case SET_TX_PATH:
    		{
    			bool retval = WIFI_TEST_SetTxPath(u4DefaultTxPath);
    	        printf("(%s) TX path is set to value 0x%x\n", retval ? "success":"fail", u4DefaultTxPath);
    	        break;
    		}

    		case SET_FREQ_OFFSET:
    		{    	
                bool retval = WIFI_TEST_SetFreqOffset(freq_offset);
    	        printf("(%s) Freq Offset is set to value 0x%x\n", retval ? "success":"fail", freq_offset);
#if WIFI_TEST_TOOL_VER101                
                if(u4DefaultCalFreqOffset == 0)
                {
                    /* Do Nothing */
                }
                else if(u4DefaultCalFreqOffset == 1) 
                {
                    /* Code Compenstaion Enable bit */
                    if(freq_offset & BIT(7))
                    {
                        uint32_t offset_value, default_offset_value;
                        unsigned int compare_addr = eeprom_addr;
                        /* Get the Freq. Offset Default Absolute value */
                        open_eeprom_file();
    	                retval = WIFI_TEST_GetCalFreqOffsetEepromAddr(0x0, &eeprom_addr);
                        printf("(%s) Get Freq Offset EEPROM addr 0x%x \n", retval ? "success":"fail", eeprom_addr);
                        retval = (compare_addr == eeprom_addr) ? 1 : 0;
                        if(retval)
                        {
                            printf("(%s) Not compatible with firmware, Please update the newset firmware!!\n", retval?"fail":"success");
                            break;
                        }
                        retval = WIFI_TEST_EEPROM_Read(eeprom_addr, &default_offset_value);
                        printf("(%s) Get Freq Offset 0x%x value 0x%02x\n", retval ? "success":"fail", eeprom_addr, default_offset_value);

                        /* Set the Freq. Offset Compenstation value */
                        offset_value = freq_offset & BITS(0,6);
                        default_offset_value &= BITS(0,6);                        
                        if(default_offset_value >= offset_value)
                        {
                            /* - compensation */
                            freq_offset = default_offset_value - offset_value;
                            if(freq_offset > 0x3F)
                            {
                                printf("(fail) Invalid Compensation value\n");
                                break;
                            }
                            freq_offset |= BIT(6);
                        }
                        else
                        {
                            /* + compensation */
                            freq_offset = offset_value - default_offset_value;
                            if(freq_offset > 0x3F)
                            {
                                printf("(fail) Invalid Compensation value\n");                                
                                break;
                            }
                            freq_offset &= ~BIT(6);
                        }
                        /* Set default freq_offset enable bit */
                        freq_offset |= BIT(7);                        
                    }
                    else
                    {
                        printf("(fail) Invalid Freq Offset value 0x%02x\n", freq_offset); 
                        break;
                    }
                }
                else
                {
                    printf("(fail) Set Freq Offset not support!!!\n");
                    break;
                }
                
                unsigned int cal_freq_addr = 0;
                unsigned int last_cal_freq_addr = cal_freq_addr;
                /* Get eeprom value */
                retval = WIFI_TEST_GetCalFreqOffsetEepromAddr(u4DefaultCalFreqOffset ,&cal_freq_addr);
      	        printf("(%s) Freq Offset Calibration get eeprom address 0x%x\n", retval ? "success":"fail", cal_freq_addr);
                retval = (cal_freq_addr == last_cal_freq_addr) ? 1 : 0;
                if(retval)
                {
                    printf("(%s) Not compatible with firmware, Please update the newset firmware!!\n", retval?"fail":"success");
                    break;
                }
                /* Write eeprom value */
      			retval = WIFI_TEST_EEPROM_Write(cal_freq_addr, freq_offset);
       	        printf("(%s) Freq Offset Calibration Write EEPROM addr 0x%x value 0x%x\n", retval? "success":"fail", cal_freq_addr, freq_offset);
#endif                
    	        break;
    		}

    		case SET_NSS:
    		{
    			bool retval = WIFI_TEST_SetNss(NSS);
    	        printf("(%s) Nss is set to value 0x%x\n", retval ? "success":"fail", NSS);
    	        break;
    		}

    		case SET_TX_POWER_COMPENSATION:
            {
                /* we use bit data to tranfser,
                   [31:24]:Reserved
                   [23:20]: Tx0/Tx1
                   [19:18]: 2G/5G
                   [17:8]: Channel#
                   [7:0]: channelpower compensataion */

            /* For Get EEPROM address subtype*/
            tx_power_compensation_data |= BIT(24);
                tx_power_compensation_data |= tx_power_compensation;
                tx_power_compensation_data |= channel << 8;

                if (channel > 14) //5G
                    tx_power_compensation_data |= BIT(18);


    			bool retval = 0;

                if (u4DefaultTxPath == 0x3) // Tx0 + Tx1
                {
                    /* Tx0 */
                    /* Set Tx0 Channel Compensation */
                    retval = WIFI_TEST_SetTxPowerCompensation(tx_power_compensation_data);
                    printf("(%s) Tx0 power compensation is set to value 0x%x\n", retval ? "success":"fail", tx_power_compensation_data);

                    /* Tx1 */
                    /* Set Tx1 Channel Compensation */
                    tx_power_compensation_data |= BIT(20);
                    retval = WIFI_TEST_SetTxPowerCompensation(tx_power_compensation_data);
        	        printf("(%s) Tx1 power compensation is set to value 0x%x\n", retval ? "success":"fail", tx_power_compensation_data);
                }
                else
                {
                    if (u4DefaultTxPath == 0x2) //Tx1
                        tx_power_compensation_data |= BIT(20);
                    else if (u4DefaultTxPath == 0x1) //Tx0
                        tx_power_compensation_data &= ~BIT(20);

                    /* Set Tx Channel Compensation */
                    retval = WIFI_TEST_SetTxPowerCompensation(tx_power_compensation_data);
        	        printf("(%s) Tx power compensation is set to value 0x%x\n", retval ? "success":"fail", tx_power_compensation_data);
                }


    			unsigned int val = 0;
                bool retval3=0, retval2=0;

                if (u4DefaultTxPath == 0x3) // Tx0 + Tx1
                {
                    /* Tx0 */
                    /* Get eeprom address */
                    tx_power_compensation_data &= ~BIT(20);
                    retval2 = WIFI_TEST_GetTxPowerCompensationEepromAddr(tx_power_compensation_data ,&val);
        	        printf("(%s) Tx0 power get eeprom address 0x%x\n", retval2 ? "success":"fail", val);

                    /* Write eeprom valuse */
        			retval3 = WIFI_TEST_EEPROM_Write(val, tx_power_compensation);
        	        printf("(%s) Tx0 Wirte EEPROM addr 0x%x value 0x%x\n", retval3 ? "success":"fail", val, tx_power_compensation);

                    /* Tx1 */
                    /* Get eeprom address */
                    tx_power_compensation_data |= BIT(20);
                    retval2 = WIFI_TEST_GetTxPowerCompensationEepromAddr(tx_power_compensation_data ,&val);
        	        printf("(%s) Tx1 power get eeprom address 0x%x\n", retval2 ? "success":"fail", val);

                    /* Write eeprom valuse */
        			retval3 = WIFI_TEST_EEPROM_Write(val, tx_power_compensation);
        	        printf("(%s) Tx1 Wirte EEPROM addr 0x%x value 0x%x\n", retval3 ? "success":"fail", val, tx_power_compensation);
                }
                else
                {
                    if (u4DefaultTxPath == 0x2) //Tx1
                        tx_power_compensation_data |= BIT(20);
                    else if (u4DefaultTxPath == 0x1) //Tx0
                        tx_power_compensation_data &= ~BIT(20);

                    /* Get eeprom address */
                    retval2 = WIFI_TEST_GetTxPowerCompensationEepromAddr(tx_power_compensation_data ,&val);
        	        printf("(%s) power get eeprom address 0x%x\n", retval2 ? "success":"fail", val);

                    /* Write eeprom valuse */
        			retval3 = WIFI_TEST_EEPROM_Write(val, tx_power_compensation);
        	        printf("(%s) Wirte EEPROM addr 0x%x value 0x%x\n", retval3 ? "success":"fail", val, tx_power_compensation);
                }
    	        break;
            }

    		case WRITE_MCR:
    		{
    			bool retval = WIFI_TEST_MCR_Write(mcr_addr, mcr_value);
    	        printf("(%s) MCR addr 0x%x is set to value 0x%x\n", retval ? "success":"fail", mcr_addr, mcr_value);
    	        break;
    		}
    		case READ_MCR:
    		{
    			unsigned int val = 0;
    	        bool retval = WIFI_TEST_MCR_Read(mcr_addr, &val);
    	        printf("(%s) MCR addr 0x%x value 0x%x\n", retval ? "success":"fail", mcr_addr, val);
    	        break;
    		}
    		case TEST_RX:
            {
                int testDuration;

                if(sleepModeSet) {
                    testDuration = sleep_time;
                }
                else {
                    testDuration = times;
                }

    			wifi_sensitivity(testDuration, channel);
    			break;
            }
    		case TEST_TX:
    			wifi_tx();
    			break;
    		case TEST_STOP:
    			wifiTestStop();
    			break;

            case QUERY_RESULT:
                wifiGetResult();
                break;

            case DUMP_EFUSE_ALL:
            {
                /* 1. Set to eeprom efuse mode */
                WIFI_TEST_set_Eeprom_Mode(EFUSE_MODE);

                /* 2. Read back all the efuse contents */
                /* 1024/16 = 64*/
                for (index=0; index<64 ; index++)
                {
                    WIFI_TEST_get_HQA(index*EFUSE_BYTES_PER_LINE, index, uacEEPROMImage+index*EFUSE_BYTES_PER_LINE);
                }

                printf("       0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F\n");
                for (index=0; index<MAX_EEPROM_BUFFER_SIZE; index+=16)
                {
                    printf("%3x    %02x  %02x  %02x  %02x  %02x  %02x  %02x  %02x  %02x  %02x  %02x  %02x  %02x  %02x  %02x  %02x\n", index,
                     uacEEPROMImage[index]  ,  uacEEPROMImage[index+1],  uacEEPROMImage[index+2],   uacEEPROMImage[index+3],
                     uacEEPROMImage[index+4],  uacEEPROMImage[index+5],  uacEEPROMImage[index+6],   uacEEPROMImage[index+7],
                     uacEEPROMImage[index+8],  uacEEPROMImage[index+9],  uacEEPROMImage[index+10], uacEEPROMImage[index+11],
                    uacEEPROMImage[index+12], uacEEPROMImage[index+13],  uacEEPROMImage[index+14], uacEEPROMImage[index+15]);
                }
            }

            default:
    		case OPER_NONE:
    			//printf("not give any operation\n");
    			break;
    	}
    	WIFI_TEST_deinit();

    	if ((user_expect & 0x2) == 0x2 && wlan_status == TEST_MODE_ON){
    		int ret = WIFI_TEST_CloseDUT();
    		printf("[%s] Disable Wi-Fi test mode %s\n", WIFI_IF_NAME,
                ret==true ? "success":"fail");
    	}
            printf("still in while loop!!!!!\n");

//    }  //while 1 end
#if 0
    for(loop_index=0; loop_index<40; loop_index++)
    {
        free(Input_CMD[loop_index]);
    }
    free(Input_CMD);
#endif
    return 0;
}

void wifi_sensitivity(int times, int channel)
{
    int i, nextInterval;
    int rxOk, rxErr;
    int rxRssisFinal;
    bool retval;
    bool finalResult = false;
    bool ret[3];

    retval = WIFI_TEST_Channel(channel);
    printf("(%s) Set central channel number to %d\n", retval ? "success":"fail", 
        channel);
	if (retval == 0) return;

#if 0
    retval = WIFI_TEST_SetRxDefaultAnt(rxDefaultAnt);
    printf("(%s) Set Rx default antenna to %s\n", retval ? "success":"fail", 
        rxDefaultAnt?"AUX":"main");
	if (retval == 0) return;
#endif
    
    if(isNewBwSet) {
        retval = WIFI_TEST_SetBandwidthV2(rfBw);
        printf("(%s) Set RF bandwidth to %s\n", retval ? "success":"fail", 
            bandwidthV2[rfBw]);
    	if (retval == 0) return;
       
        retval = WIFI_TEST_SetPriChannelSetting(priSetting);
        printf("(%s) Set primary channel index to %u\n", 
            retval ? "success":"fail", priSetting);
    	if (retval == 0) return;        
    }
    else {
        retval = WIFI_TEST_SetBandwidth(g_bandwidth);
        printf("(%s) Set bandwidth to %s\n", retval ? "success":"fail", 
            bandwidth[g_bandwidth]);
    	if (retval == 0) return;
    }
    if (eRxOkMatchRule == RX_MATCH_RULE_DISABLE) {
        retval = WIFI_TEST_SetRX(false, NULL, NULL);	
        printf("(%s) Disable RX filter\n", retval ? "success":"fail");
    } else {
        if (bRxFilterMacAddrLegalFg) {
            retval = WIFI_TEST_SetRX(true, NULL, (char *)aucRxFilterMacAddr);	
            printf("(%s) Enable RX filter, Set RA Address to %02x:%02x:%02x:%02x:%02x:%02x\n", retval ? "success":"fail", 
                aucRxFilterMacAddr[0],
            	aucRxFilterMacAddr[1],
            	aucRxFilterMacAddr[2],
            	aucRxFilterMacAddr[3],
            	aucRxFilterMacAddr[4],
            	aucRxFilterMacAddr[5]
            	);
            if (retval == 0) return;
        } else {
            printf("Enalbe RX filter, need to set RA address\n");
            return;
        }
    }

    /* 71 */
    retval = WIFI_TEST_SetCbw(u4Cbw);
    printf("(%s) Band is set to value 0x%d\n", retval ? "success":"fail", u4Cbw);

    /* todo: not support yet, temp code.
     * Ted201213: force RX patch WF0+WF1 band => 0x00030000
     * bit[15:0] band index, bit[31:16] mask of RX path.
     */
    if (rx_path == 0x1) /*WF0*/
    {
        u4DefaultRxPath = 0x00010000;
    }
    else if (rx_path == 0x2) /*WF1*/
    {
        u4DefaultRxPath = 0x00020000;
    }
    else if (rx_path == 0x3) /*WF0 + WF1*/
    {
        u4DefaultRxPath = 0x00030000;
    }
    else
    {
        printf("RX path not support!!!\n");

    }


    WIFI_TEST_SetRxPath(u4DefaultRxPath);

    retval = WIFI_TEST_RxStart();
    printf("(%s) RX test started\n", retval ? "success":"fail");
	if (retval == 0) return;
#if WIFI_TEST_TOOL_VER102 
		if(SetIsoFlag)
    {
	    retval = WIFI_TEST_AutoIso();   
	    /* Auto Isolation Value */
	    uint32_t defaultBTTxPower, SetBTTxPower, PowerOffset = 0;
	    retval = WIFI_TEST_GetCalIsolationValue(&Isolation_val);
	    defaultBTTxPower = Isolation_val >> 24;
	    defaultBTTxPower |= ((defaultBTTxPower >> 7) & 1)? 0xffffff00 : 0x00000000;
	    SetBTTxPower = BTTxPower*2;
	    Isolation_val = ((Isolation_val << 8) >> 8);      
	    if(SetBTTxFlag){
	        PowerOffset = SetBTTxPower - defaultBTTxPower*2;
	        printf("BT Tx Power : %.1f dbm\n", BTTxPower);
	    }else{
	        printf("BT Tx Power : %d.0 dBm\n", defaultBTTxPower);
	    }
	    Isolation_val += PowerOffset;
	    printf("(%s)Auto Isolation Value %.1f \n", retval ? "success":"fail",(float)Isolation_val/2);
	    SetIsoFlag = false;	
	   } 	
#endif
    nextInterval = printInterval;
    
    for(i = 0; (i < times) || !finalResult; i += nextInterval) {

        if(i >= times) {
            finalResult = true;
        }
        
        ret[0] = WIFI_TEST_FRGood(&rxOk);
        ret[1] = WIFI_TEST_FRError(&rxErr);
        ret[2] = WIFI_TEST_RSSI(&rxRssisFinal);
		if ((rxOk + rxErr) == 0){
			fprintf(stdout, "[%3d] (%d)RX OK: %4d / (%d)RX ERR: %4d\n", 
                i, ret[0], rxOk, ret[1], rxErr);
		}
        else{
            fprintf(stdout, "[%3d] (%d)RX OK: %4d / (%d)RX ERR: %4d / PER: %2d .. /"
                " (%d)RSSI: %i\r\n", i, ret[0], rxOk, ret[1], rxErr, 
                (100 * rxErr)/(rxOk + rxErr), ret[2], (signed char)rxRssisFinal);
		}
        fflush(stdout);

        if((times - i) < printInterval) {
            nextInterval = times - i;
        }

        if((rxOk + rxErr >= (int)maxPktCount) && maxPktCount) {
            printf("Rx packet count[%u] >= max count[%u], break!\n", 
                rxOk + rxErr, maxPktCount);
            break;
        }
        
        sleep(nextInterval);
    }

	if (times == 0) {
        printf("Rx test is running! use -T to stop Rx test...\n");
    }
    else {
        retval = WIFI_TEST_RxStop();
    }
}

void wifi_tx(void)
{
    bool retval;
    bool finalResult = false;
    
    WIFI_TEST_TxDestAddress(macAddr);

    retval = WIFI_TEST_Channel(channel);
    printf("(%s) Set central channel number to %d\n", retval ? "success":"fail", 
        channel);
    if (retval == 0) return;

#if 0
    retval = WIFI_TEST_SetRxDefaultAnt(rxDefaultAnt);
    printf("(%s) Set Rx default antenna to %s\n", retval ? "success":"fail", 
        rxDefaultAnt?"AUX":"main");
	if (retval == 0) return;
#endif

    if(isNewBwSet) {
        retval = WIFI_TEST_SetBandwidthV2(rfBw);
        printf("(%s) Set RF bandwidth to %s\n", retval ? "success":"fail", 
            bandwidthV2[rfBw]);
    	if (retval == 0) return;

        retval = WIFI_TEST_SetTxBandwidth(txBw);
        printf("(%s) Set Tx bandwidth to %s\n", retval ? "success":"fail", 
            bandwidthV2[txBw]);
    	if (retval == 0) return;
        
        retval = WIFI_TEST_SetPriChannelSetting(priSetting);
        printf("(%s) Set primary channel index to %u\n", 
            retval ? "success":"fail", priSetting);
    	if (retval == 0) return;        
    }
    else {
        retval = WIFI_TEST_SetBandwidth(g_bandwidth);
        printf("(%s) Set bandwidth to %s\n", retval ? "success":"fail", 
            bandwidth[g_bandwidth]);
    	if (retval == 0) return;
    }

    retval = WIFI_TEST_TxGain(txGain);
    printf("(%s) Set Tx power gain to %f dBm\n", retval ? "success":"fail",
        txGain);
	if (retval == 0) return;

    retval = WIFI_TEST_TxPayloadLength(payloadLength);
    printf("(%s) Set Tx payload to %d bytes..\n", retval ? "success":"fail", payloadLength);
	if (retval == 0) return;
	
	retval = WIFI_TEST_TxBurstInterval(SIFS);
    printf("(%s) Set frame interval to %d TU\n", retval ? "success":"fail", SIFS);
	if (retval == 0) return;

    retval = WIFI_TEST_TxBurstFrames(times);
    printf("(%s) Set frame count to %d \n", retval ? "success":"fail", times);    
    if (retval == 0) return;

    switch(txMode) {
    case 0: /* A/B/G mode */
        retval = WIFI_TEST_SetPreamble(pType);
        printf("(%s) Set %s preamble\n", retval ? "success":"fail", preamble[pType]);
    	if (retval == 0) return;
        
        retval = WIFI_TEST_TxDataRate(g_rate);
        printf("(%s) Set Tx mode to 11a/b/g, tx rate %s\n", retval ? "success":"fail", bg_rate[g_rate]);
        if (retval == 0) return;
        break;

    case 1: /* N mode */
        retval = WIFI_TEST_TxDataRate11n(gMCSrate, gMode, giType);
        printf("(%s) Set Tx mode to 11n, MCS%u, %s, %s GI, %s\n", retval ? "success":"fail", 
            gMCSrate, gMode?"greenfield":"mixed-mode", giType?"Short":"Normal", 
            coding?"LDPC":"BCC");
        if (retval == 0) return;

        retval = WIFI_TEST_SetTxCodingMode(coding);
        if (retval == 0) return;
        
        break;
        
    case 2: /* AC mode */
        retval = WIFI_TEST_TxDataRate11ac(gMCSrate, giType);
        printf("(%s) Set Tx mode to 11ac MCS%u, %s GI, %s\n", retval ? "success":"fail", 
            gMCSrate, giType?"Short":"Normal", coding?"LDPC":"BCC");
        if (retval == 0) return;
        
        retval = WIFI_TEST_SetTxCodingMode(coding);
        if (retval == 0) return;
        
        break;

    default:
        printf("Unsupported Tx mode[%u]!\n", txMode);
        return;
    }
    
    retval = WIFI_TEST_SetTxPath(u4DefaultTxPath);
    printf("(%s) TX path is set to value 0x%x\n", retval ? "success":"fail", u4DefaultTxPath);

        retval = WIFI_TEST_SetBand(u4Band);
        printf("(%s) Band is set to value 0x%d\n", retval ? "success":"fail", u4Band);

        /* 110 */
        retval = WIFI_TEST_SetDBDC(u4Dbdc);
        printf("(%s) u4Dbdc is set to value 0x%d\n", retval ? "success":"fail", u4Dbdc);

        /* 104 */
        retval = WIFI_TEST_SetDBDCBand(u4DbdcBandIndex);
        printf("(%s) u4DbdcBandIndex is set to value 0x%d\n", retval ? "success":"fail", u4DbdcBandIndex);


        /* 71 */
        retval = WIFI_TEST_SetCbw(u4Cbw);
        printf("(%s) Band is set to value 0x%d\n", retval ? "success":"fail", u4Cbw);

        /* 72 */
        //retval = WIFI_TEST_SetDbw(u4Dbw);
        //printf("(%s) Band is set to value 0x%d\n", retval ? "success":"fail", u4Dbw);

        retval = WIFI_TEST_SetDbw(u4Cbw);
        printf("(%s) Band is set to value 0x%d\n", retval ? "success":"fail", u4Cbw);

        /* 73 */
        retval = WIFI_TEST_SetPriCh(u4PriCh);
        printf("(%s) Band is set to value 0x%d\n", retval ? "success":"fail", u4PriCh);


        /* 101 */
        retval = WIFI_TEST_SetMacHeader(u4MacHeader);
        printf("(%s) u4MacHeader is set to value 0x%d\n", retval ? "success":"fail", u4MacHeader);

        /* 103 */
        retval = WIFI_TEST_SetPayLoad(u4Payload);
        printf("(%s) u4Payload is set to value 0x%d\n", retval ? "success":"fail", u4Payload);

        /* 69 */
        retval = WIFI_TEST_SetTA(u4Ta);
        printf("(%s) u4Ta is set to value 0x%d\n", retval ? "success":"fail", u4Ta);

        /* 17 */
        retval = WIFI_TEST_SetStbc(u4Stbc);
        printf("(%s) u4Stbc is set to value 0x%d\n", retval ? "success":"fail", u4Stbc);

        /* 126*/
        retval = WIFI_TEST_SetIbf(u4Ibf);
        printf("(%s) u4Ibf is set to value 0x%d\n", retval ? "success":"fail", u4Ibf);

        /* 127*/
        retval = WIFI_TEST_SetEbf(u4Ebf);
        printf("(%s) u4Ebf is set to value 0x%d\n", retval ? "success":"fail", u4Ebf);

        /* 114*/
        retval = WIFI_TEST_SetNss(NSS);
        printf("(%s) Nss is set to value 0x%x\n", retval ? "success":"fail", NSS);


    //for CW mode
    if(-1 != cw_mode){
		retval = WIFI_TEST_CW_MODE(cw_mode);
		printf("(%s) cw mode set to %d\n", retval ? "success":"fail", cw_mode);
		if (retval == 0) return;
		
		retval = WIFI_TEST_CW_MODE_START();
		printf("(%s) cw mode start.\n", retval ? "success":"fail");
		if (retval == 0) return;
    }else{   
    	printf("no cw mode configuration.\n");
    	retval = WIFI_TEST_TxStart();
    	printf("(%s) TX test started..\n", retval ? "success":"fail");
    	if (retval == 0) return;     
    }

	if (sleep_time == 0) {
        printf("Tx test is running! use -T to stop Tx test...\n");
        return;
    }
    else {
        int i, nextInterval;
        uint32_t u4TxOk, u4Tx;

        nextInterval = printInterval;
        
        printf("Tx test is running! wait for %us...\n", sleep_time);

        for(i = 0; (i < sleep_time) || !finalResult; i += nextInterval) {
            if(i >= sleep_time) {
                finalResult = true;
            }
            
            retval = WIFI_TEST_TxCount(&u4Tx);
            retval = WIFI_TEST_TxGoodCount(&u4TxOk);
            
            if(retval == 0) {
                printf("(%s) Cannot get test result!\n", retval ?"success":"fail");
            }
            else {
                printf("[%u] Tx total/good count: %u/%u\n", i, u4Tx, u4TxOk);       
            }
            
            if(i >= sleep_time) {
                break;
            }

            if((u4Tx >= maxPktCount) && maxPktCount) {
                printf("Tx packet count[%u] >= max count[%u], break!\n", 
                    u4Tx, maxPktCount);
                break;
            }

            if((sleep_time - i) < printInterval) {
                nextInterval = (sleep_time - i);
            }
            
            sleep(nextInterval);
        }           

        printf("Stop Tx test!\n");
    }
    
    retval = WIFI_TEST_TxStop();
}

void wifiGetResult(void) {
    uint32_t u4RxOk, u4RxFailed, u4Rssi;
    uint32_t u4TxOk, u4Tx;
    bool retval;

    do {

        retval = WIFI_TEST_FRGood((int*)&u4RxOk);
    	if (retval == 0) break;

        retval = WIFI_TEST_FRError((int*)&u4RxFailed);
    	if (retval == 0) break;
        
        retval = WIFI_TEST_RSSI((int*)&u4Rssi);
    	if (retval == 0) break;

        retval = WIFI_TEST_TxCount(&u4Tx);
    	if (retval == 0) break;
        
        retval = WIFI_TEST_TxGoodCount(&u4TxOk);
    	if (retval == 0) break;

    } while(false);

    if(retval == 0) {
        printf("(%s) Cannot get test result!\n", retval ?"success":"fail");
    }
    else {
        printf("Tx total/good count: %u/%u\n", u4Tx, u4TxOk);
        printf("Rx good/err count: %u/%u PER: %u RSSI:%i\n", u4RxOk, u4RxFailed,
            (100 * u4RxFailed)/(u4RxOk + u4RxFailed), (signed char)u4Rssi);        
    }
}

void wifiTestStop(void){
	bool retval = WIFI_TEST_TxStop();
	printf("(%s) stop Tx\n", retval ? "success":"fail");
    
	retval = WIFI_TEST_RxStop();
	printf("(%s) stop Rx\n", retval ? "success":"fail");    
}

/* if wlan.driver.status is ok, then wlan normal mode is on
	if /sys/class/net/wlan0 is not exist, then wlan is off 
	otherwise, we think the wlan may be turned on by us */
static WlanStatus wifiStatus(void){
	char driver_status[PROP_VALUE_MAX];
	bool normal_mode_on = false;
	char netdevPath[256];
	static bool isTestMode = false;
    
	struct stat buf;
	//property_get("wlan.driver.status", driver_status, "unloaded");
	//if (strncmp(driver_status, "ok", 2) == 0){
	//	normal_mode_on = true;
//	}


	return TEST_MODE_ON;
    snprintf(netdevPath, 255, "/sys/class/net/%s", WIFI_IF_NAME);
	if (stat(netdevPath, &buf) < 0 && errno==ENOENT)
		return WLAN_MODE_OFF;
	return normal_mode_on ? NORMAL_MODE_ON:TEST_MODE_ON;
}


/*----------------------------------------------------------------------------*/
/*!
 * @brief This function is to open the eeprom.bin file in /lib/firmware
 *
 * @param
 *
 * @return
 */
/*----------------------------------------------------------------------------*/
int
open_eeprom_file(
    void
    )
{
    FILE *fh;
    char  buff[16];
    int   ret = -1;
    int index = 0, index1 = 0;
    int counter = 0;

    fh = fopen(EEPROM_PATH_TMP, "rb");

    if(fh != NULL) {
            /* Success : use eeprom.bin file from /lib/firmware */
            //fgets(buff, sizeof(buff), fh);
            //fgets(buff, sizeof(buff), fh);

            /* Read each device line */
            //while(fgets(buff, sizeof(buff), fh)) {

            while(!feof(fh)) {
                fread(uacEEPROMImage, sizeof(uacEEPROMImage), 1, fh);
            }

        fclose(fh);
    }
    else {
        printf("Open file error!!! Please modify the file path for reading source\n");
    }

#if 0
    for (index=0; index<MAX_EEPROM_BUFFER_SIZE; index+=16)
    {
        printf("offset=%2x, %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x", index, 
         uacEEPROMImage[index]  ,  uacEEPROMImage[index+1],  uacEEPROMImage[index+2],   uacEEPROMImage[index+3],
         uacEEPROMImage[index+4],  uacEEPROMImage[index+5],  uacEEPROMImage[index+6],   uacEEPROMImage[index+7],
         uacEEPROMImage[index+8],  uacEEPROMImage[index+9],  uacEEPROMImage[index+10], uacEEPROMImage[index+11],
        uacEEPROMImage[index+12], uacEEPROMImage[index+13],  uacEEPROMImage[index+14], uacEEPROMImage[index+15]);
        

        printf("\n");
    }
#endif

    return ret;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This API provided a service for EEPROM query commands
 *
 * @param
 *
 * @return
 */
/*----------------------------------------------------------------------------*/

bool WIFI_TEST_EEPROM_Read(unsigned int offset, unsigned int *val)
{
    if (!val) {
        return false;
    }

    *val = uacEEPROMImage[offset];


    return true;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This API provided a service for EEPROM write commands
 *
 * @param
 *
 * @return
 */
/*----------------------------------------------------------------------------*/

bool WIFI_TEST_EEPROM_Write(unsigned int offset, unsigned int val)
{
    FILE *fh;
    unsigned char test;
    uacEEPROMImage[offset] = val;
    test = uacEEPROMImage[offset];

#if 1
    fh = fopen(EEPROM_PATH_TMP, "rb+");
    if(fh != NULL) {

   	    //printf("111  of WIFI_TEST_EEPROM_Write\n");

#if 0
           while(!feof(fh)) {
   	        printf("1.5  of WIFI_TEST_EEPROM_Write\n");
                fwrite(uacEEPROMImage, sizeof(uacEEPROMImage), 1200, fh);
      	        printf("1.6  of WIFI_TEST_EEPROM_Write\n");
           }
#endif
           fseek(fh, offset, SEEK_SET);
           fwrite(&test, sizeof(unsigned char), 1, fh);
        //printf("222  of WIFI_TEST_EEPROM_Write\n");

        fclose(fh);
    }
#endif

    //printf("end of WIFI_TEST_EEPROM_Write\n");

    return true;
}


/*----------------------------------------------------------------------------*/
/*!
 * @brief This API provided a service for i-cal mode (intelligent calibration)
 *
 * @param
 *
 * @return
 */
/*----------------------------------------------------------------------------*/
void replaceEEPROM()
{
    int i, index;
    unsigned int val = 0;
    bool retval;

    unsigned int replace_positions[] =
            { 0x53, 0x54, 0x55, 0xF1, 0xF4, 0xF7,0x178,0x179,0x17A,
             0x17B,0x1B3,0x1B4,0x1B5,0x1B6,0x1B7,0x1E2,0x1E3,0x300,0x305,
             0x306,0x307,0x308,0x309,0x366,0x367,0x368,0x369,0x36A,0x36B,
             0x36C,0x38C,0x38D,0x38E};

    printf("Inside Replace: Checking Address\n");

#if 0
    for(i=0;i<80;i++)
    {
#endif
        // Replace all the contents in the FT fields
        for(i=0;i< sizeof(replace_positions)/sizeof(unsigned int) ;i++)  //34
        {
            retval = WIFI_TEST_EFUSE_Read(replace_positions[i], &val);

            //printf("(%s) EFUSE addr 0x%x value 0x%x\n", retval ? "success":"fail", replace_positions[i], val);

          	uacEEPROMImage[replace_positions[i]] = val;
        }
#if 0
    }
#endif
#if 0
    printf("After merge\n");

    for (index=0; index<MAX_EEPROM_BUFFER_SIZE; index+=16)
    {
        printf("offset=%2x, %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x", index,
         uacEEPROMImage[index]  ,  uacEEPROMImage[index+1],  uacEEPROMImage[index+2],   uacEEPROMImage[index+3],
         uacEEPROMImage[index+4],  uacEEPROMImage[index+5],  uacEEPROMImage[index+6],   uacEEPROMImage[index+7],
         uacEEPROMImage[index+8],  uacEEPROMImage[index+9],  uacEEPROMImage[index+10], uacEEPROMImage[index+11],
        uacEEPROMImage[index+12], uacEEPROMImage[index+13],  uacEEPROMImage[index+14], uacEEPROMImage[index+15]);


        printf("\n");
    }
#endif
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This API provided a service for saving array to bin file
 *
 * @param
 *
 * @return
 */
/*----------------------------------------------------------------------------*/
void saveEEPROM()
{
   	FILE *file=fopen(EEPROM_PATH_WRITE,"wb");

    fwrite(uacEEPROMImage,sizeof(unsigned char),MAX_EEPROM_BUFFER_SIZE,file);

	fclose(file);
}
