#ifndef _META_PUB_H_
#define _META_PUB_H_

#include <stdbool.h>
#include "PortHandle.h"
#define	MAX_PATH				1024

typedef enum
{
	META_SUCCESS = 0,
	META_FAILED
} META_RESULT;


typedef enum
{
	META_STATUS_FAILED = 0,
    META_STATUS_SUCCESS
} META_STATUS;


//////////////////////////////////////////////////////////////////////////
//define the MD frame
#define MD_FRAME_TREACE_OFFSITE 3
#define MD_FRAME_HREADER_LENGTH 4
#define MAX_TST_RECEIVE_BUFFER_LENGTH       (4096*16)//2048
#define TST_CHECKSUM_SIZE					(1)
#define MD_FRAME_TST_INJECT_PRIMITIVE_LENGTH 10
#define MD_FRAME_FAILED_TST_LOG_PRIMITIVE_LENGTH  20
#define MD_FRAME_SUCCESS_TST_LOG_PRIMITIVE_LENGTH 102
#define MD_FRAME_REF_LENGTH 2
#define MD_FRAME_MSG_LEN_LENGTH 2
#define MD_FRAME_MAX_LENGTH 256
#define MD_FRAME_FAILED_CHECEK_SIM_OFFISTE 76
#define MD_FRAME_SUCCESS_CHECEK_SIM_OFFISTE 116
#define MD_FRAME_DS269_OFFSITE 8

//the define of the type of meta frame
#define  RS232_LOGGED_PRIMITIVE_TYPE   		0x60
#define  RS232_PS_TRACE_TYPE           		0x61
#define  RS232_PS_PROMPT_TRACE_TYPE    		0x62
#define  RS232_COMMAND_TYPE_OCTET      		0x63
#define  RS232_INJECT_PRIMITIVE_OCTET  		0x64
#define  RS232_INJECT_UT_PRIMITIVE     		0x65
#define  RS232_INJECT_APPRIMITIVE_OCTET     0x66

#define  RS232_INJECT_PRIMITIVE_OCTETMODEM2  		0xA0
#define  RS232_INJECT_PRIMITIVE_OCTETMODEM2_END		0xA7
#define  RS232_COMMAND_TYPE_MD2_MEMORY_DUMP      	0xC0
#define  RS232_COMMAND_TYPE_MD2_MEMORY_DUMP_END     0xC7

//TST tunneling
#define  RS232_COMMAND_TYPE_MD_DATA_TUNNEL_START   0xD0
#define  RS232_COMMAND_TYPE_MD_DATA_TUNNEL_END 0xD7
#define  RS232_RESPONSE_MD_DATA_TUNNEL_START   0xD8
#define  RS232_RESPONSE_MD_DATA_TUNNEL_END  0xDF
//TST tunneling + compression
#define  RS232_COMMAND_TYPE_MD_DATA_TUNNEL_COMP_START 0xF0
#define  RS232_COMMAND_TYPE_MD_DATA_TUNNEL_COMP_END   0xF7
#define  RS232_RESPONSE_MD_DATA_TUNNEL_COMP_START     0xF8   
#define  RS232_RESPONSE_MD_DATA_TUNNEL_COMP_END       0xFF


//the maximum size of frame
#define FRAME_MAX_LEN 1024*64
//the size of peer buf header
#define PEER_HEADER_LEN 8
// the maximum size of peer buf
#define PEER_BUF_MAX_LEN 1024*60
// the maximum size of peer buf + local buf
#define FT_MAX_LEN (FRAME_MAX_LEN -PEER_HEADER_LEN - 9)

/* teh define of escape key */
#define   STX_OCTET            	0x55
#define   MUX_KEY_WORD		    0x5A
#define   SOFT_FLOW_CTRL_BYTE   0x77
#define   STX_L1HEADER         	0xA5


//CCB: Mux Header size
/***************************************************
Mux Header Format
   0                   1                 2                      3          
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
  |                        mux header magic  (4B)               |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
  |           len           (2B)         |               
  +-+-+-+-+-+-+-+-+-+-+-+-+-
  
  mux header magic:
   0                   1                   2                   3
   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |     0xAC      |     0xCA      |     0x00      |     0xFF      |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

****************************************************/
#define META_CCB_MUX_HEADER_LEN 			6
#define META_CCB_TX_MAX_RETRY				10
#define META_CCB_INIT_MAX_RETRY				10
#define META_CCB_POOL_BITMASK   			1    //For Meta, the bitmask is fixed
#define META_CCB_BUFFER_ID   				0    //For Meta, the buffer id is fixed.

/* Define the rs232 frame phase states */
#define  RS232_FRAME_STX               				0
#define  RS232_FRAME_LENHI             				1
#define  RS232_FRAME_LENLO             				2
#define  RS232_FRAME_TYPE              				3
#define  RS232_FRAME_LOCAL_LENHI       				4
#define  RS232_FRAME_LOCAL_LENLO       				5
#define  RS232_FRAME_PEER_LENHI        				6
#define  RS232_FRAME_PEER_LENLO        				7
#define  RS232_FRAME_COMMAND_DATA      				8
#define  RS232_FRAME_COMMAND_HEADER    				9
#define  RS232_FRAME_UT_DATA		   				10
#define  RS232_FRAME_MD_DATA		   				11
#define  RS232_FRAME_AP_INJECT_PIRIMITIVE_HEADER 	12
#define  RS232_FRAME_AP_PRIM_LOCAL_PARA_DATA     	13
#define  RS232_FRAME_AP_PRIM_PEER_DATA           	14
#define  RS232_FRAME_CHECKSUM          				15
#define  RS232_FRAME_KEYWORD		   				16
#define  RS232_FRAME_SOFT_CTRL         				17
#define  RS232_FRAME_MD_CONFIRM_DATA				18
#define  RS232_FRAME_MD_TUNNELING_DATA 				19
#define  RS232_FRAME_MD_TUNNELING_CHECKSUM 			20
#define  RS232_FRAME_MD_TUNNELING_COMPRESS_DATA                 21


#define  UART1_PATH_TTYS	"/dev/ttyS0"
#define  UART2_PATH_TTYS	"/dev/ttyS1"
#define  UART3_PATH_TTYS	"/dev/ttyS2"
#define  UART4_PATH_TTYS	"/dev/ttyS3"

#define  UART1_PATH_TTYMT	"/dev/ttyMT0"
#define  UART2_PATH_TTYMT	"/dev/ttyMT1"
#define  UART3_PATH_TTYMT	"/dev/ttyMT2"
#define  UART4_PATH_TTYMT	"/dev/ttyMT3"


////////////////////////////log ctrl///////////////////////////////////

#define MDLOG_SOCKET_NAME		     "ModemLogHidlServer"
#define MBLOG_SOCKET_NAME            "MobileLogHidlServer"
#define CONNSYSLOG_SOCKET_NAME       "ConnsysFWHidlServer"

#define MDLOG_START                  "meta_start,%d"
#define MDLOG_STOP			      	 "meta_pause"	
#define MDLOG_PULL_START	         "pull_mdlog_start,1"
#define MDLOG_PULL_STOP  	         "pull_mdlog_stop"
#define MDLOG_QUERY_STATUS           "get_running_state"
#define MDLOG_QUERY_NORMALLOG_PATH   "get_run_folder" 
#define MDLOG_QUERY_EELOG_PATH       "get_ee_folder"
#define MDLOG_PULL_STATUS            "get_pullmdlog_state"
#define MDLOG_EE_DONE_STATUS         "get_ee_done_state"
#define MDLOG_SET_FILTER		     "meta_mdfilter,%d"

#define MBLOG_START                  "meta_start" 
#define MBLOG_STOP                   "meta_pause"
#define MBLOG_PULL_START             "ext_autopull"
#define MBLOG_PULL_STOP              "ext_autostop"
#define MBLOG_QUERY_NORMALLOG_PATH   "get_run_mbfolder"
#define MBLOG_PULL_STATUS            "get_pullmblog_state"

#define CONNLOG_START                "meta_connsys_start" 
#define CONNLOG_STOP                 "meta_connsys_stop"
#define CONNLOG_PULL_START	         "pull_FWlog_start"
#define CONNLOG_PULL_STOP  	         "pull_FWlog_stop"
#define CONNLOG_PULL_STATUS          "pull_FWlog_status"
#define CONNLOG_QUERY_STATUS         "log_running_status"

#define MDDB_PULL_START				 "pull_MDDB"
//////////////////////////////////////////////////////////////////////////

//-------------------------------------
// define com mask parameter
//-------------------------------------
#define DEFAULT_COM_MASK    (EV_RXCHAR | EV_RLSD | EV_ERR | EV_BREAK | EV_RING)

//////////////////////////////////////////////////////////////////////////

typedef signed short	int16;
typedef signed int		int32;
typedef unsigned char	uint8;
typedef unsigned short	uint16;
typedef unsigned int	uint32;



//*****************************************************************************
//
//                          META Driver data structure def
//
//*****************************************************************************



// defie the type of frame.
typedef enum
{
	AP_FRAME =0,	//ap side
	MD_FRAME		//modem side
} META_FRAME_TYPE;


// the data pass between FT and TST
typedef struct
{
	META_FRAME_TYPE eFrameType;	//frame type
	unsigned char *pData;
	unsigned short LocalLen;			//local len
	unsigned short PeerLen;			//peer len
} META_RX_DATA;

typedef enum
{
	META_UNKNOWN_COM=0,
	META_UART_COM,
	META_USB_COM,
	META_SOCKET,
}META_COM_TYPE;

typedef struct
{
	unsigned short	token;
	unsigned short	id;
}FT_H;

typedef struct
{	
    char* ip_addr;
	unsigned int port;
}WIFI_PARA;


//the ID define of ft req and cnf, it is used to ananlyze the different module.
typedef enum
{
	/* RF */
	FT_RF_TEST_REQ_ID = 0					   ,/*0*/
	FT_RF_TEST_CNF_ID = 1					   ,
	/* BaseBand */
	FT_REG_READ_ID = 2						   ,
	FT_REG_READ_CNF_ID = 3					   ,
	FT_REG_WRITE_ID	= 4						   ,
	FT_REG_WRITE_CNF_ID	= 5					   ,/*5*/
	FT_ADC_GETMEADATA_ID = 6				   ,
	FT_ADC_GETMEADATA_CNF_ID = 7			   ,
	/* test alive */
	FT_IS_ALIVE_REQ_ID = 8						   ,
	FT_IS_ALIVE_CNF_ID = 9						   ,
	/* power off */
	FT_POWER_OFF_REQ_ID = 10						   ,/*10*/
	/* unused */
	FT_RESERVED04_ID = 11 						   ,
	/* required META_DLL version */
	FT_CHECK_META_VER_REQ_ID = 12 				   ,
	FT_CHECK_META_VER_CNF_ID = 13 				   ,
	/* utility command */
	FT_UTILITY_COMMAND_REQ_ID = 14				   ,
	FT_UTILITY_COMMAND_CNF_ID = 15				   ,/*15*/
	/* for NVRAM */
	FT_NVRAM_GET_DISK_INFO_REQ_ID = 16			   ,
	FT_NVRAM_GET_DISK_INFO_CNF_ID = 17			   ,
	FT_NVRAM_RESET_REQ_ID = 18					   ,
	FT_NVRAM_RESET_CNF_ID = 19					   ,
	FT_NVRAM_LOCK_CNF_ID = 20					   ,/*20*/
	FT_NVRAM_LOCK_REQ_ID = 21 					   ,
	FT_NVRAM_READ_REQ_ID = 22 					   ,
	FT_NVRAM_READ_CNF_ID = 23 					   ,
	FT_NVRAM_WRITE_REQ_ID = 24					   ,
	FT_NVRAM_WRITE_CNF_ID = 25					   ,/*25*/
	/* FAT */
	FT_FAT_OPERATION_ID = 26 				   ,/* 26 ~ 40 */
	/* L4 Audio */
	FT_L4AUD_REQ_ID = 41 					   ,/* 41 ~ 50 */
	FT_L4AUD_CNF_ID							   ,
	/* Version Info */
	FT_VER_INFO_REQ_ID = 51					   ,/* 51 */
	FT_VER_INFO_CNF_ID						   ,
	/* CCT */
	FT_CCT_REQ_ID = 53						   ,/* 53 */
	FT_CCT_CNF_ID							   ,
	/* WiFi */
	FT_WIFI_WNDRV_SET_REQ_ID = 55			   ,/* 55 */
	FT_WIFI_WNDRV_SET_CNF_ID 				   ,
	FT_WIFI_WNDRV_QUERY_REQ_ID = 57			   ,/* 57 */
	FT_WIFI_WNDRV_QUERY_CNF_ID				   ,
	FT_WIFI_REQ_ID = 59						   ,/* 59 */
	FT_WIFI_CNF_ID							   ,  
	FT_BT_REQ_ID = 61						   ,
	FT_BT_CNF_ID 							   ,
	FT_PMIC_REG_READ_ID = 63 		   , 
	FT_PMIC_REG_READ_CNF_ID			   ,
	FT_PMIC_REG_WRITE_ID = 65		   , 
	FT_PMIC_REG_WRITE_CNF_ID 			   ,
	FT_URF_TEST_REQ_ID = 67					 ,	 /* 67 */
	FT_URF_TEST_CNF_ID				   ,
	FT_FM_REQ_ID = 69						  ,   /* 69 */
	FT_FM_CNF_ID = 70						  ,
	FT_TDMB_REQ_ID = 71				  , /* 71 */
	FT_TDMB_CNF_ID = 72				  , /* 72 */
	/* This is a special message defined to handle L1 report. */
	FT_DISPATCH_REPORT_ID					   ,
	FT_WM_METATEST_REQ_ID						,  	/* 74 */
	FT_WM_METATEST_CNF_ID						,
	// for battery dfi
	FT_WM_BAT_REQ_ID								,	/* 76 */
	FT_WM_BAT_CNF_ID								,
	//for dvbt test
	FT_WM_DVB_REQ_ID								,	/* 78 */
	FT_WM_DVB_CNF_ID								,
	FT_BATT_READ_INFO_REQ_ID=80    ,
	FT_BATT_READ_INFO_CNF_ID,
	FT_GPS_REQ_ID = 82							,
	FT_GPS_CNF_ID 							    ,
	FT_BAT_CHIPUPDATE_REQ_ID = 84	,
	FT_BAT_CHIPUPDATE_CNF_ID 			,
	FT_SDCARD_REQ_ID = 86 ,
	FT_SDCARD_CNF_ID 	 ,
	FT_LOW_POWER_REQ_ID = 88,
	FT_LOW_POWER_CNF_ID ,
	FT_GPIO_REQ_ID = 90,
	FT_GPIO_CNF_ID ,
	// For NVRAM backup & restore
	FT_NVRAM_BACKUP_REQ_ID = 94,
	FT_NVRAM_BACKUP_CNF_ID,
	FT_NVRAM_RESTORE_REQ_ID = 96,
	FT_NVRAM_RESTORE_CNF_ID,
	// For G-Sensor
	FT_GSENSOR_REQ_ID = 114,
	FT_GSENSOR_CNF_ID ,
	FT_META_MODE_LOCK_REQ_ID = 116,
	FT_META_MODE_LOCK_CNF_ID,
	// Reboot
	FT_REBOOT_REQ_ID = 118,
	// For MATV
	FT_MATV_CMD_REQ_ID = 119,
	FT_MATV_CMD_CNF_ID,
	// Customer API
	FT_CUSTOMER_REQ_ID = 121,
	FT_CUSTOMER_CNF_ID = 122,
	// Get chip ID
	FT_GET_CHIPID_REQ_ID = 123,
	FT_GET_CHIPID_CNF_ID = 124,
	// M-Sensor
	FT_MSENSOR_REQ_ID = 125,
	FT_MSENSOR_CNF_ID = 126,
	// Touch panel
	FT_CTP_REQ_ID = 127,
	FT_CTP_CNF_ID = 128,
	// ALS_PS
	FT_ALSPS_REQ_ID = 129,
	FT_ALSPS_CNF_ID = 130,
	//Gyroscope	
	FT_GYROSCOPE_REQ_ID = 131,
	FT_GYROSCOPE_CNF_ID = 132,
	// Get version info V2
	FT_VER_INFO_V2_REQ_ID = 133,
	FT_VER_INFO_V2_CNF_ID = 134,
	//CMMB
	FT_CMMB_REQ_ID = 135,
	FT_CMMB_CNF_ID = 136,

	FT_BUILD_PROP_REQ_ID = 137,
	FT_BUILD_PROP_CNF_ID = 138,

	// NFC
	FT_NFC_REQ_ID = 139,
	FT_NFC_CNF_ID = 140,

	FT_ADC_REQ_ID = 141,
   	FT_ADC_CNF_ID = 142,

   	FT_EMMC_REQ_ID = 143,
   	FT_EMMC_CNF_ID = 144,

   	FT_CRYPTFS_REQ_ID = 145,
   	FT_CRYPTFS_CNF_ID = 146,

   	FT_MODEM_REQ_ID = 147,
   	FT_MODEM_CNF_ID = 148,

   	FT_SIM_NUM_REQ_ID = 149,
   	FT_SIM_NUM_CNF_ID = 150,
   
   	// DFO
   	FT_DFO_REQ_ID = 151,
   	FT_DFO_CNF_ID = 152,

   	//DRMKey
   	FT_DRMKEY_REQ_ID = 153,
   	FT_DRMKEY_CNF_ID = 154,

   	//FT_HDCP_REQ_ID = 155,
   	//FT_HDCP_CNF_ID = 156,

	//SPECIALTEST
   	FT_SPECIALTEST_REQ_ID = 157,
   	FT_SPECIALTEST_CNF_ID = 158,

	FT_CHIP_INFO_REQ_ID = 159,
	FT_CHIP_INFO_CNF_ID = 160,

	FT_SIM_DETECT_REQ_ID = 161,
	FT_SIM_DETECT_CNF_ID = 162,

    FT_FILE_OPERATION_REQ_ID = 163,
    FT_FILE_OPERATION_CNF_ID = 164,
	FT_GAMMA_REQ_ID = 165,
	FT_GAMMA_CNF_ID = 166,

	FT_RATCONFIG_REQ_ID = 167,
	FT_RATCONFIG_CNF_ID = 168,

	FT_MSIM_REQ_ID = 169,
	FT_MSIM_CNF_ID = 170,

    //MD logger ctrl
    FT_TARGETLOG_CTRL_REQ_ID = 171,
    FT_TARGETLOG_CTRL_CNF_ID = 172,

    FT_APDB_REQ_ID = 173,
    FT_APDB_CNF_ID = 174,

    FT_ATTESTATIONKEY_REQ_ID = 175,
	FT_ATTESTATIONKEY_CNF_ID = 176,

	FT_SYSENV_REQ_ID = 177,
	FT_SYSENV_CNF_ID = 178,

	FT_TARGETCLOCK_REQ_ID = 179,
	FT_TARGETCLOCK_CNF_ID = 180,	

	FT_DISCONNECT_REQ_ID = 181,
	FT_DISCONNECT_CNF_ID = 182,

	FT_MSG_LAST_ID	
} FT_MESSAGE_ID;



#ifndef FALSE
#define FALSE 0
#define TRUE 1
#endif

#ifndef BOOL
#define BOOL bool
#endif

#ifndef META_BOOL
#define META_BOOL bool
#endif

typedef unsigned char BYTE;

#endif	// _META_PUB_H_
