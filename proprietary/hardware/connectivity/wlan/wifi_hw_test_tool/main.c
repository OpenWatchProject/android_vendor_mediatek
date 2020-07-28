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
#include "libwifitest.h"
#include "lib.h"
#include <cutils/properties.h>
#include <sys/system_properties.h>
#include <errno.h>

#define PACKAGE     "WifiHwTestTool"

char proc_name[256];

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
	WRITE_EFUSE,
	READ_MCR,
	WRITE_MCR,
	TEST_STOP,
	QUERY_RESULT
}Oper_Mode;

typedef enum _ENUM_RX_MATCH_RULE_T {
    RX_MATCH_RULE_DISABLE,
    RX_MATCH_RULE_RA,           /* RA only */
    RX_MATCH_RULE_TA,           /* TA only */
    RX_MATCH_RULE_RA_TA,        /* Both RA and TA */
    RX_MATCH_RULE_NUM
} ENUM_RX_MATCH_RULE_T, *P_ENUM_RX_MATCH_RULE_T;

typedef enum _ENUM_FUNCTION_MODE {
    MODE_DISABLE = 0,
    MODE_ENABLE
};

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

void signal_handler(int sig) 
{
    int retval = 0;

    retval = WIFI_TEST_CloseDUT();
    printf("\n(%d) aborted .., sig=%d\n", retval,sig);

    signal(SIGINT, SIG_DFL);
    exit(0);
}

//  "E:e:M:g:G:I:B:R:N:T:m:s:p:b:t:hVw:v:l:f    :c:rn:"


void print_help(int exval)
{
    printf("Usage: %s [options]\n", proc_name);
    printf("\n");

    printf("<Test mode control>\n");
    printf("    %s -O                       Enable Wi-Fi test mode\n", proc_name);
    printf("    %s -C                       Disable Wi-Fi test mode\n", proc_name);  
    printf("\n");
    
    printf("<MCR read/write>\n");    
    printf("    %s [-M addr]                Read value from CR address\n", proc_name);
    printf("    %s [-w addr] [-v value]     Write value to CR address\n", proc_name);
    printf("\n");
    
    printf("<EFUSE read/write>\n"); 
    printf("    %s [-E offset]              Read value from EFUSE at offset\n", proc_name);
    printf("    %s [-e offset] [-v value]   Write value to EFUSE at offset\n", proc_name);
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
    printf("\n");
    
    printf("Tx specific:\n");

	printf("    -n #           TX Packet number, 0 is meaning that TX Packet number = unlimited\n");
	
    printf("    -t [0/1/2]     Tx mode <0:11abg/1:11n/2:11ac>\n");
    printf("    -x [0~3]       Tx bandwidth <0:20/1:40/2:80/3:160>Mhz <default follow RF BW>\n");
    printf("    -p #           Tx gain in dBm\n");

	
    
    printf("    -n #           Frame count\n");
    printf("    -l #           Frame length in bytes\n");    
    
    printf("    -R [1~12]      Legacy rate code\n"); 
    printf("                   <1M/2M/5.5M/6M/9M/11M/12M/18M/24M/36M/48M/54M>\n");
    printf("    -s [0/1]       <0:short/1:long> preamble\n");

    printf("    -N [0~15/32]   MCS rate index\n");
    printf("    -g [0/1]       <0:mixed mode/1:greenfield> \n");
    printf("    -G [0/1]       <0:normal/1:short> guard interval\n");
    printf("    -L             Enable LDPC <default BCC>\n");
    
    printf("    -m [0/3]       <0:disable/3:enable> continuous waveform mode\n");
    printf("\n"); 
    
    exit(exval);
}

static int channel = 1;
static int times = 10;
//static int numBurst = 0;
static int txMode = 0;
static unsigned char macAddr[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
#define Default_TxGain -1
static int txGain = Default_TxGain;
static int payloadLength = 1024;
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
static int cw_mode = -1;
static int sleep_time = 10;
static bool sleepModeSet = false;

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

static ENUM_RX_MATCH_RULE_T eRxOkMatchRule = RX_MATCH_RULE_DISABLE;
static bool bRxFilterMacAddrLegalFg = false;
static unsigned char aucRxFilterMacAddr[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; 

#define WIFI_TEST_BW_MAX 5

extern bool fgDebugMode;

int main(int argc, char *argv[]) 
{
    int opt = 0;
    int result = 0;
    Oper_Mode operation = OPER_NONE;
    WlanStatus wlan_status = 0;
	int band_width[] = {WIFI_TEST_BW_20MHZ, WIFI_TEST_BW_40MHZ, 
        WIFI_TEST_BW_U20MHZ, WIFI_TEST_BW_D20MHZ, WIFI_TEST_BW_80MHZ,
        WIFI_TEST_BW_160MHZ};

    strncpy(proc_name, argv[0], 255);
    proc_name[255] = '\0';
    
    if (argc == 1){
        fprintf(stderr, "Needs arguments....\n\n");
        print_help(1);
    }
	// set up the Ctrl + C handler
	signal(SIGINT, signal_handler);
    while ((opt = getopt(argc, argv, "A:f:E:e:M:g:G:I:B:R:N:Tm:S:s:p:b:t:hVw:v:k:l:f:c:rOCn:DP:x:Ld:j:qa:o")) != -1) {
	    switch(opt) {
	        case 'e':
	        	if (operation == OPER_NONE){
		            operation = WRITE_EFUSE;
		            xtoi(optarg, &efuse_addr);
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
                
	        case 'g':
	        	gMode = !atoi(optarg) ? WIFI_TEST_PREAMBLE_TYPE_MIXED_MODE:WIFI_TEST_PREAMBLE_TYPE_GREENFIELD;
	            break;
	        case 'G':
	        	giType = !atoi(optarg) ? WIFI_TEST_GI_TYPE_NORMAL_GI:WIFI_TEST_GI_TYPE_SHORT_GI;
	            break;
	        case 'I':
	            strcpy(WIFI_IF_NAME, optarg);
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
	        case 'p':
	            txGain = atoi(optarg);
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
				user_expect = 1;
				break;
			case 'C':
				user_expect |= 2;
				break;
			case 'T':
				operation = TEST_STOP;
				break;
        	case 'D':
	            fgDebugMode = true;
	            break;
	    	case 'f':
	            eRxOkMatchRule = atoi(optarg);
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
				printf("[%s] Already in test mode\n", WIFI_IF_NAME);
			break;
            
		case NORMAL_MODE_ON:
			printf("Please turn off normal mode wlan first!\n");
			return 0;
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
	        printf("(%s) EFUSE addr 0x%x value 0x%x\n", retval ? "success":"fail", efuse_addr, val);
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

    retval = WIFI_TEST_SetRxDefaultAnt(rxDefaultAnt);
    printf("(%s) Set Rx default antenna to %s\n", retval ? "success":"fail", 
        rxDefaultAnt?"AUX":"main");
	if (retval == 0) return;
    
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

    retval = WIFI_TEST_RxStart();
    printf("(%s) RX test started\n", retval ? "success":"fail");
	if (retval == 0) return;

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

    retval = WIFI_TEST_SetRxDefaultAnt(rxDefaultAnt);
    printf("(%s) Set Rx default antenna to %s\n", retval ? "success":"fail", 
        rxDefaultAnt?"AUX":"main");
	if (retval == 0) return;

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

    // if txGain = Default_TxGain, then use NVRAM tx pwr setting
    if(txGain != Default_TxGain){
        retval = WIFI_TEST_TxGain(txGain*2);
        printf("(%s) Set Tx power gain to %d dBm\n", retval ? "success":"fail",
            txGain);
        if (retval == 0) return;
    }
    else{
        printf("(success) Set Tx power gain by NVRAM Setting\n");
    }

    retval = WIFI_TEST_TxPayloadLength(payloadLength);
    printf("(%s) Set Tx payload to %d bytes..\n", retval ? "success":"fail", payloadLength);
	if (retval == 0) return;
	
    retval = WIFI_TEST_DPD_MODE(MODE_DISABLE);
    printf("(%s) Disable DPD\n", retval ? "success":"fail");
    if (retval == 0) return;

    retval = WIFI_TEST_TEMP_COMPEN_MODE(MODE_DISABLE);
    printf("(%s) Disable TEMP_COMP\n", retval ? "success":"fail");
    if (retval == 0) return;

    retval = WIFI_TEST_TX_POWER_MODE(MODE_DISABLE);
    printf("(%s) TX power mode 0\n", retval ? "success":"fail");
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
    
	struct stat buf;
	property_get("wlan.driver.status", driver_status, "unloaded");
	if (strncmp(driver_status, "ok", 2) == 0){
		normal_mode_on = true;
	}

    snprintf(netdevPath, 255, "/sys/class/net/%s", WIFI_IF_NAME);
    
	if (stat(netdevPath, &buf) < 0 && errno==ENOENT)
		return WLAN_MODE_OFF;
	return normal_mode_on ? NORMAL_MODE_ON:TEST_MODE_ON;
}
