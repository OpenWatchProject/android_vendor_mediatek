
#ifndef _FT_DRIVERINTERFACE_H_
#define _FT_DRIVERINTERFACE_H_

#ifdef __cplusplus
extern "C"{
#endif

//WCN

#ifdef FT_WIFI_FEATURE
#include "meta_wifi_para.h"
#endif

#ifdef FT_GPS_FEATURE
#include "meta_gps_para.h"
#endif

#ifdef FT_NFC_FEATURE
#include "meta_nfc_para.h"
#endif

#ifdef FT_BT_FEATURE
#include "meta_bt_para.h"
#endif
#ifdef FT_FM_FEATURE
#include "meta_fm_para.h"
#endif

#ifdef FT_RAT_FEATURE
#include "meta_rat_para.h"
#endif

#ifdef FT_MSIM_FEATURE
#include "meta_msim_para.h"
#endif

//MM
#ifdef FT_AUDIO_FEATURE
#include "meta_audio_para.h"
#endif

#ifdef FT_MATV_FEATURE
#include "meta_matv_para.h"
#endif

#ifdef FT_DRM_KEY_MNG_FEATURE
#include "meta_drmkey_install_para.h"
#endif

#ifdef FT_GAMMA_FEATURE
#include "meta_gamma_para.h"
#endif

#ifdef FT_ATTESTATION_KEY_FEATURE
#include "kmsetkey.h"
#endif

//NVRAM
#include "libfile_op.h"

#ifdef FT_NVRAM_FEATURE
#include "Meta_APEditor_Para.h"
#endif

	
typedef struct
{
	unsigned int	file_size;
	unsigned char	file_ID;
	unsigned char	stage;
} FT_STREAM_BLOCK;

typedef struct
{
    FT_H    header;
    char    buffer[1024];
    int     count;
    int     mode;
} FT_NVRAM_BACKUP_REQ;

typedef struct
{
    FT_H            header;
    FT_STREAM_BLOCK block;
    unsigned char   status;
} FT_NVRAM_BACKUP_CNF;

typedef struct
{
    FT_H            header;
    FT_STREAM_BLOCK block;
} FT_NVRAM_RESTORE_REQ;

typedef struct
{
	FT_H		header;
	unsigned char	status;
} FT_NVRAM_RESTORE_CNF;


//Basic
typedef struct
{
    FT_H			header;					
    unsigned int	dummy;						
} FT_IS_ALIVE_REQ;

typedef struct
{
    FT_H			header;						
    unsigned int	dummy;						
} FT_IS_ALIVE_CNF;

typedef struct
{
    FT_H    header;
} FT_VER_INFO_REQ;

typedef struct
{
    FT_H    header;			
    unsigned char   bb_chip[64];
    unsigned char   eco_ver[4];
    unsigned char   sw_time[64];
    unsigned char   dsp_fw[64];
    unsigned char   rsc_ver[64];
    unsigned char   sw_ver[64];
    unsigned char   hw_ver[64];
    unsigned char   melody_ver[64];
    unsigned char	status;	
} FT_VER_INFO_CNF;

typedef struct
{
    FT_H		 header;		//ft header
} FT_VER_INFO_V2_REQ;

typedef struct
{
    FT_H		header;			//ft header
    unsigned char	bb_chip[64];
    unsigned char	eco_ver[4];
    unsigned char	sw_time[64];
    unsigned char	dsp_fw[64];
    unsigned char	dsp_patch[64];
    unsigned char	sw_ver[64];
    unsigned char	hw_ver[64];
    unsigned char	melody_ver[64];
    unsigned char    build_disp_id[64];
    unsigned char	 status;	//ft status: 0 is success
} FT_VER_INFO_V2_CNF;


typedef enum {
    FT_SHUTDOWN_OP_POWEROFF = 0,
    FT_SHUTDOWN_OP_REBOOT,
    FT_SHUTDOWN_OP_ATMDISCONNECT,
    FT_SHUTDOWN_OP_WAITUSB,
    FT_SHUTDOWN_OP_END = 0x0fffffff
} FT_SHUTDOWN_OP;


typedef struct
{
	FT_H header;
	unsigned int dummy; 
}FT_POWER_OFF_REQ;

typedef struct
{
    FT_H            header;
    unsigned int    delay;
    unsigned int    dummy;
} FT_META_REBOOT_REQ;

typedef struct
{
	FT_H header;
	unsigned char tag[64];
}FT_BUILD_PROP_REQ;

typedef struct
{
	FT_H header;
	unsigned char content[128];
	int		status;
}FT_BUILD_PROP_CNF;

typedef struct
{
    FT_H            header;
    unsigned int    dummy;               
} FT_GET_CHIPID_REQ;

typedef struct
{
    FT_H            header;
    unsigned char   chipId[17];
    unsigned char   status;
} FT_GET_CHIPID_CNF;

//Sensor
#ifdef FT_GSENSOR_FEATURE
#include "meta_gsensor_para.h"
#endif
#ifdef FT_MSENSOR_FEATURE
#include "meta_msensor_para.h"
#endif
#ifdef FT_ALSPS_FEATURE
#include "meta_alsps_para.h"
#endif
#ifdef FT_GYROSCOPE_FEATURE
#include "meta_gyroscope_para.h"
#endif


typedef struct
{
    FT_H            header;
    unsigned int    dummy;               
} FT_MSENSOR_REQ;

typedef struct
{
    FT_H            header;
    unsigned char   status;
} FT_MSENSOR_CNF;

typedef struct
{
    FT_H            header;
    unsigned int    dummy;               
} FT_ALSPS_REQ;

typedef struct
{
    FT_H            header;
    unsigned char   status;
} FT_ALSPS_CNF;

//Modem
typedef enum 
{
	FT_MODEM_OP_QUERY_INFO = 0,
	FT_MODEM_OP_CAPABILITY_LIST = 1,
	FT_MODEM_OP_SET_MODEMTYPE = 2,
	FT_MODEM_OP_GET_CURENTMODEMTYPE = 3,
	FT_MODEM_OP_QUERY_MDIMGTYPE = 4,
	FT_MODEM_OP_QUERY_MDDOWNLOADSTATUS = 5,
	FT_MODEM_OP_TRIGGER_NATIVE_DOWNLOAD = 6,
	FT_MODEM_OP_REBOOT_MODEM = 7,
	FT_MODEM_OP_GET_MODEMMODE = 8,
	FT_MODEM_OP_QUERY_MDDBPATH = 9,
	FT_MODEM_OP_SUPPORT_COMPRESS = 10,
	FT_MODEM_END = 0x0fffffff
}FT_MODEM_OP;

typedef struct
{
    unsigned int modem_id;  //no use
}MODEM_QUERY_MDDBPATH_REQ;

typedef struct
{
    unsigned char mddb_path[64];
}MODEM_QUERY_MDDBPATH_CNF;

typedef struct
{
	unsigned char modem_index;
	unsigned char mode;
}MODEM_REBOOT_REQ;

typedef struct
{
	unsigned int reserved;
}MODEM_REBOOT_CNF;


typedef struct
{
	unsigned char modem_index;
}MODEM_GET_MODEMMODE_REQ;

typedef struct
{
	unsigned char mode;
}MODEM_GET_MODEMMODE_CNF;

typedef struct 
{
	unsigned char reserved;
}MODEM_QUERY_DOWNLOAD_STATUS_REQ;

typedef struct 
{
	unsigned int percentage;
	unsigned int status_code;
}MODEM_QUERY_DOWNLOAD_STATUS_CNF;

typedef struct 
{
	unsigned char reserved;
}MODEM_QUERY_INFO_REQ;
	
typedef struct 
{
	unsigned int modem_number;
	unsigned int modem_id;
}MODEM_QUERY_INFO_CNF;

typedef enum 
{
	FT_MODEM_SRV_INVALID = 0,
	FT_MODEM_SRV_TST = 1,
	FT_MODEM_SRV_DHL = 2,
	FT_MODEM_SRV_ETS = 3,
	FT_MODEM_SRV_END = 0x0fffffff
}FT_MODEM_SRV;
	
typedef enum 
{
	FT_MODEM_CH_NATIVE_INVALID = 0,
	FT_MODEM_CH_NATIVE_TST = 1,
	FT_MODEM_CH_TUNNELING = 2,
	FT_MODEM_CH_TUNNELING_IGNORE_CKSM = 3,
	FT_MODEM_CH_NATIVE_ETS = 4,
	FT_MODEM_CH_END = 0x0fffffff
}FT_MODEM_CH_TYPE;
	
typedef struct 
{
	FT_MODEM_SRV md_service;
	FT_MODEM_CH_TYPE ch_type;
	unsigned char reserved;
}MODEM_CAPABILITY; 

typedef struct 
{
    unsigned char reserved;
}MODEM_CAPABILITY_LIST_REQ;

typedef struct 
{
	MODEM_CAPABILITY modem_cap[8];
}MODEM_CAPABILITY_LIST_CNF; 

typedef struct 
{
	unsigned int modem_id;
	unsigned int modem_type;
}MODEM_SET_MODEMTYPE_REQ;

typedef struct 
{
	unsigned char reserved;	
}MODEM_SET_MODEMTYPE_CNF;

typedef struct 
{
	unsigned int modem_id;
}MODEM_GET_CURRENTMODEMTYPE_REQ;

typedef struct 
{
	unsigned int current_modem_type;
}MODEM_GET_CURENTMODEMTYPE_CNF;

typedef struct 
{
	unsigned int modem_id;	
}MODEM_QUERY_MDIMGTYPE_REQ;

typedef struct 
{
	unsigned int mdimg_type[16];
}MODEM_QUERY_MDIMGTYPE_CNF;
	
typedef struct
{       
   unsigned int action;   //0:disable; 1:enable 
} MODEM_SUPPORT_COMPRESS_REQ;

typedef struct 
{
   unsigned int result;  //0:fail; 1:success
} MODEM_SUPPORT_COMPRESS_CNF;

typedef union 
{
	MODEM_QUERY_INFO_REQ query_modem_info_req; 
	MODEM_CAPABILITY_LIST_REQ query_modem_cap_req;
	MODEM_SET_MODEMTYPE_REQ set_modem_type_req;
	MODEM_GET_CURRENTMODEMTYPE_REQ get_currentmodem_type_req; 
	MODEM_QUERY_MDIMGTYPE_REQ query_modem_imgtype_req;
    MODEM_QUERY_DOWNLOAD_STATUS_REQ query_modem_download_status_req;
	MODEM_REBOOT_REQ reboot_modem_req;
	MODEM_GET_MODEMMODE_REQ get_modem_mode_req;
	MODEM_QUERY_MDDBPATH_REQ query_mddbpath_req;
	MODEM_SUPPORT_COMPRESS_REQ set_compress_req;
}FT_MODEM_CMD;
	
typedef union 
{
	MODEM_QUERY_INFO_CNF query_modem_info_cnf;
	MODEM_CAPABILITY_LIST_CNF query_modem_cap_cnf;
	MODEM_SET_MODEMTYPE_CNF set_modem_type_cnf;
	MODEM_GET_CURENTMODEMTYPE_CNF get_currentmodem_type_cnf;
	MODEM_QUERY_MDIMGTYPE_CNF query_modem_imgtype_cnf;
	MODEM_QUERY_DOWNLOAD_STATUS_CNF query_modem_download_status_cnf;
	MODEM_REBOOT_CNF reboot_modem_cnf;
	MODEM_GET_MODEMMODE_CNF get_modem_mode_cnf;
	MODEM_QUERY_MDDBPATH_CNF query_mddbpath_cnf;
	MODEM_SUPPORT_COMPRESS_CNF set_compress_cnf;
}FT_MODEM_RESULT;
		
typedef struct 
{
	FT_H header;
	FT_MODEM_OP	type;
	FT_MODEM_CMD cmd;
}FT_MODEM_REQ;
		
typedef struct 
{
	FT_H header;
	FT_MODEM_OP type;
	unsigned char status;
	FT_MODEM_RESULT result;
}FT_MODEM_CNF;

typedef enum 
{
     FT_GET_SIM_NUM = 0,
     FT_MISC_WCN_END = 0x0fffffff
}FT_GET_SIM_OP;


typedef struct 
{
    FT_H    header;
    FT_GET_SIM_OP    type;   
}FT_GET_SIM_REQ;

typedef struct 
{
    FT_H                  header;
    FT_GET_SIM_OP             type;	
   	unsigned char  status;
	unsigned int   number;
}FT_GET_SIM_CNF;

//Baseband
#ifdef FT_LCDBK_FEATURE
#include "meta_lcdbk_para.h"
#endif

#ifdef FT_KEYPADBK_FEATURE
#include "meta_keypadbk_para.h"
#endif

#ifdef FT_LCD_FEATURE
#include "meta_lcdft_para.h"
#endif

#ifdef FT_VIBRATOR_FEATURE
#include "meta_vibrator_para.h"
#endif

typedef enum
{
	 FT_UTILCMD_CHECK_IF_FUNC_EXIST = 0
	,FT_UTILCMD_CHECK_IF_ISP_SUPPORT
	,FT_UTILCMD_QUERY_BT_MODULE_ID
	,FT_UTILCMD_ENABLE_WATCHDOG_TIMER
	,FT_UTILCMD_CHECK_IF_ACOUSTIC16_SUPPORT
	,FT_UTILCMD_CHECK_IF_AUDIOPARAM45_SUPPORT
	,FT_UTILCMD_CHECK_IF_LOW_COST_SINGLE_BANK_FLASH
	,FT_UTILCMD_QUERY_PMIC_ID
	,FT_UTILCMD_BT_POWER_ON
	,FT_UTILCMD_KEYPAD_LED_ONOFF
	,FT_UTILCMD_VIBRATOR_ONOFF					//10
	,FT_UTILCMD_QUERY_LOCAL_TIME
	,FT_UTILCMD_CHECK_IF_WIFI_ALC_SUPPORT
	,FT_UTILCMD_RF_ITC_PCL
	,FT_UTILCMD_CHECK_IF_DRC_SUPPORT
	,FT_UTILCMD_CHECK_IF_BT_POWERON
	,FT_UTILCMD_MAIN_SUB_LCD_LIGHT_LEVEL
	,FT_UTILCMD_SIGNAL_INDICATOR_ONOFF
	,FT_UTILCMD_SET_CLEAN_BOOT_FLAG
	,FT_UTILCMD_LCD_COLOR_TEST
	,FT_UTILCMD_SAVE_MOBILE_LOG					//20
	,FT_UTILCMD_GET_EXTERNAL_SD_CARD_PATH
	,FT_UTILCMD_SET_LOG_LEVEL
	,FT_UTILCMD_SDIO_AUTO_CALIBRATION
	,FT_UTILCMD_QUERY_WCNDRIVER_READY
	,FT_UTILCMD_SDIO_CHECK_CALIBRATION
	,FT_UTILCMD_SWITCH_WIFI_USB
	,FT_UTILCMD_SET_ATM_FLAG
	,FT_UTILCMD_SET_PRODUCT_INFO
	,FT_UTILCMD_PRINTF_CUSLOG 
	,FT_UTILCMD_END								//30
} FtUtilCmdType;

typedef struct
{
	unsigned int		query_ft_msg_id;
	unsigned int		query_op_code;
}FtUtilCheckIfFuncExist;

typedef struct
{
    int		Notused;
    unsigned char BackupTime[64];
} SetCleanBootFlag_REQ;

typedef struct
{
    BOOL	drv_statsu;							//inidicate the result of setting clean boot
} SetCleanBootFlag_CNF;

typedef struct
{
    int		reserved;
} SAVE_MOBILE_LOG_REQ;

typedef struct
{
    BOOL	drv_status;							
} SAVE_MOBILE_LOG_CNF;

typedef struct
{
    unsigned int level;
}SET_LOG_LEVEL_REQ;

typedef struct
{
    int reserved;
}SET_LOG_LEVEL_CNF;

typedef struct
{
    unsigned int result;
}QUERY_WCNDRIVER_READY_CNF;

typedef struct
{
	unsigned short	   interval;
} WatchDog_REQ;

typedef struct
{
	unsigned short	   rtc_sec;
	unsigned short	   rtc_min;
	unsigned short	   rtc_hour;
	unsigned short	   rtc_day;
	unsigned short	   rtc_mon;
	unsigned short	   rtc_wday;
	unsigned short	   rtc_year;
	unsigned short	   status;
} WatchDog_CNF;

typedef struct
{
    unsigned int flag; //0: wifi to usb, 1: usb to wifi
}SWITCH_WIFI_USB_REQ;

typedef struct
{
    unsigned int result;
}SWITCH_WIFI_USB_CNF;

typedef struct
{
	unsigned int flag;	
}SET_ATM_FLAG_REQ;

typedef struct 
{
	int	reserved;	
}SET_ATM_FLAG_CNF;

typedef struct
{
	unsigned int type; //0: ATM flag  1: meta log flag
	unsigned int flag;	
}SET_PRODUCT_INFO_REQ;

typedef struct 
{
	int	reserved;	
}SET_PRODUCT_INFO_CNF;

typedef struct
{	 
    unsigned char log[256];
}PRINTF_LOG_REQ;

typedef struct
{	
    int reserved;
}PRINTF_LOG_CNF;


typedef union
{
    FtUtilCheckIfFuncExist	CheckIfFuncExist;
    WatchDog_REQ			m_WatchDogReq;   // whether use
    KeypadBK_REQ			m_KeypadBKReq;
    LCDLevel_REQ			m_LCDReq;
    NLED_REQ				m_NLEDReq;
    SetCleanBootFlag_REQ	m_SetCleanBootFlagReq;
    LCDFt_REQ         m_LCDColorTestReq;
    SAVE_MOBILE_LOG_REQ     m_SaveMobileLogReq;
	SET_LOG_LEVEL_REQ       m_SetLogLevelReq;
	SWITCH_WIFI_USB_REQ     m_SwitchWiFiUSBReq;
	SET_ATM_FLAG_REQ		m_SetATMFlagReq;
	SET_PRODUCT_INFO_REQ	m_SetProductInfo;
	PRINTF_LOG_REQ			m_PrintCusLogReq;
    unsigned int			dummy;
} FtUtilCmdReq_U;

typedef union
{
    FtUtilCheckIfFuncExist	CheckIfFuncExist;
    WatchDog_CNF			m_WatchDogCnf;
    KeypadBK_CNF			m_KeypadBKCnf;
    LCDLevel_CNF			m_LCDCnf;
    NLED_CNF     			m_NLEDCnf;
    SetCleanBootFlag_CNF	m_SetCleanBootFlagCnf;
    LCDFt_CNF         m_LCDColorTestCNF;
    SAVE_MOBILE_LOG_CNF     m_SaveMobileLogCnf;
	SET_LOG_LEVEL_CNF		m_SetLogLevelCnf;
	QUERY_WCNDRIVER_READY_CNF m_QueryWCNDriverReadyCnf;
	SWITCH_WIFI_USB_CNF     m_SwitchWiFiUSBCnf;
	SET_ATM_FLAG_CNF        m_SetATMFlagCnf;
	SET_PRODUCT_INFO_CNF    m_SetProductInfoCnf;
	PRINTF_LOG_CNF			m_PrintCusLogCnf;
    unsigned int			dummy;
} FtUtilCmdCnf_U;


typedef struct
{
    FT_H            header;	//ft header
    FtUtilCmdType   type;	//cmd type
    FtUtilCmdReq_U  cmd;	//cmd parameter
} FT_UTILITY_COMMAND_REQ;

typedef struct
{
    FT_H            header;	//ft header
    FtUtilCmdType   type;	//cmd type
    FtUtilCmdCnf_U  result;	//module cmd result
    unsigned char   status;	//ft status: 0 is success
} FT_UTILITY_COMMAND_CNF;

//File system
#ifdef FT_EMMC_FEATURE
#include "meta_clr_emmc_para.h"
#endif
#ifdef FT_NAND_FEATURE
#include "meta_clr_emmc_para.h"
#endif
#ifdef FT_CRYPTFS_FEATURE
#include "meta_cryptfs_para.h"
#endif

//Misc
#ifdef FT_ADC_FEATURE
#include "meta_adc_para.h"
#endif


#ifdef FT_TOUCH_FEATURE
#include "meta_touch_para.h"
#endif


#ifdef FT_GPIO_FEATURE
#include "Meta_GPIO_Para.h"
#endif



typedef enum
{
    FT_CUSTOMER_OP_BASIC = 0
    ,FT_CUSTOMER_OP_END

} META_CUSTOMER_CMD_TYPE;

typedef union
{
    unsigned char	m_u1Dummy;
} META_CUSTOMER_CMD_U;

typedef union
{
    unsigned char  m_u1Dummy; 
} META_CUSTOMER_CNF_U;

typedef struct
{
    FT_H                    header;
    META_CUSTOMER_CMD_TYPE  type;
    META_CUSTOMER_CMD_U     cmd;	
} FT_CUSTOMER_REQ;

typedef struct
{
    FT_H                    header;
    META_CUSTOMER_CMD_TYPE  type;
    unsigned char           status;
    META_CUSTOMER_CNF_U     result;			
} FT_CUSTOMER_CNF;

typedef enum 
{
     FT_SPECIALTEST_OP_HUGEDATA = 0,
     FT_SPECIALTEST_END = 0x0fffffff
}SPECIALTEST_OP;

typedef struct
{
    int reserved;
}SPECIALTEST_HUGEDATA_REQ;

typedef struct 
{
    unsigned char result;
}SPECIALTEST_HUGEDATA_CNF;


typedef union 
{
    SPECIALTEST_HUGEDATA_REQ  specialtest_hugedata_req; 
}FT_SPECIALTEST_CMD;

typedef union 
{
    SPECIALTEST_HUGEDATA_CNF  specialtest_hugedata_cnf;
}FT_SPECIALTEST_RESULT;

typedef struct 
{
    FT_H                  header;
    SPECIALTEST_OP       type;
    FT_SPECIALTEST_CMD     cmd;
}FT_SPECIALTEST_REQ;


typedef struct 
{
    FT_H                         header;
    SPECIALTEST_OP            type;
    unsigned char                status;
    FT_SPECIALTEST_RESULT        result;
}FT_SPECIALTEST_CNF;


struct FT_RAWDATATEST_REQ
{
      FT_H                  header;
};

struct FT_RAWDATATEST_CNF
{
      FT_H                        header;
      unsigned char                      status;
};

typedef struct
{
    FT_H    header;
} FT_CHIP_INFO_REQ;

typedef struct
{
    FT_H    header;			
    unsigned char   code_func[64];
    unsigned char   code_proj[64];
	unsigned char   code_date[64];
	unsigned char   code_fab[64];
    unsigned char	status;
} FT_CHIP_INFO_CNF;

//////////////////////////////////////////////////////////////////////

typedef enum{
	FT_SIM_DETECT_OP_EXTMOD = 0,
	FT_SIM_DETECT_OP_PRJTYPE,
	FT_SIM_DETECT_OP_MDIDXSET,
	FT_SIM_DETECT_OP_SWITCH,	
	FT_SIM_DETECT_OP_GETSSW,	
	FT_SIM_DETECT_END = 0x0fffffff
}FT_SIM_DETECT_OP;

typedef struct  
{
	unsigned int reserved;
}SIM_QUERY_MDTYPE_REQ;

typedef struct  
{
	unsigned int md_type;
}SIM_QUERY_MDTYPE_CNF;

typedef struct  
{
	unsigned int reserved;
}SIM_QUERY_PRJTYPE_REQ;

typedef struct  
{
	unsigned int prj_type;
}SIM_QUERY_PRJTYPE_CNF;

typedef struct  
{
	unsigned int mode_cmd;
}SIM_SET_SWITCHER_REQ;

typedef struct  
{
	unsigned int reserved;
}SIM_SET_SWITCHER_CNF;

typedef struct  
{
	unsigned int reserved;
}SIM_QUERY_MDIDXSET_REQ;

typedef struct  
{
	unsigned int md_idxset;
}SIM_QUERY_MDIDXSET_CNF;

typedef struct  
{
	unsigned int reserved;
}SIM_QUERY_SSW_REQ;

typedef struct  
{
	unsigned int ssw_val;
}SIM_QUERY_SSW_CNF;

typedef union 
{
	SIM_QUERY_MDTYPE_CNF   sim_query_mdtype_cnf;
	SIM_QUERY_PRJTYPE_CNF  sim_query_prjtype_cnf;
	SIM_QUERY_MDIDXSET_CNF sim_query_mdidxset_cnf;
	SIM_SET_SWITCHER_CNF   sim_set_switcher_cnf;
	SIM_QUERY_SSW_CNF	   sim_query_ssw_cnf;

} FT_SIM_DETECT_RESULT;


typedef union
{
	SIM_QUERY_MDTYPE_REQ   sim_query_mdtype_req;
	SIM_QUERY_PRJTYPE_REQ  sim_query_prjtype_req;
	SIM_QUERY_MDIDXSET_REQ sim_query_mdidxset_req;
	SIM_SET_SWITCHER_REQ   sim_set_switcher_req;
	SIM_QUERY_SSW_REQ	   sim_query_ssw_req;

} FT_SIM_DETECT_CMD;


typedef struct 
{
	FT_H header;
	FT_SIM_DETECT_OP	 type;
	FT_SIM_DETECT_CMD   cmd;
}FT_SIM_DETECT_REQ;
		
typedef struct 
{
	FT_H header;
	FT_SIM_DETECT_OP  type;
	FT_SIM_DETECT_RESULT result;
	unsigned char status;
}FT_SIM_DETECT_CNF;


typedef enum{
	FT_FILE_OP_PARSE = 0,
	FT_FILE_OP_GETFILEINFO,
	FT_FILE_OP_SENDFILE,
	FT_FILE_OP_RECEIVEFILE,	
	FT_FILE_OP_END = 0x0fffffff
}FT_FILE_OPERATION_OP;

typedef struct  
{
	unsigned char path_name[256];
	unsigned char filename_substr[256];
}FILE_OPERATION_PARSE_REQ;

typedef enum 
{
	FT_FILE_TYPE_INVALID = 0,
	FT_FILE_TYPE_FILE = 1,
	FT_FILE_TYPE_FOLDER = 2,
	FT_FILE_TYPE_END = 0x0fffffff
}FT_FILE_TYPE;

typedef struct 
{
	FT_FILE_TYPE file_type;
	unsigned int file_size;
	unsigned char file_name[256];
}FT_FILE_INFO;  

typedef struct  
{
	unsigned int file_count;
}FILE_OPERATION_PARSE_CNF;

typedef struct  
{
	unsigned int index;   // [ 0, file_count )
}FILE_OPERATION_GETFILEINFO_REQ;


typedef struct  
{
	FT_FILE_INFO file_info;
}FILE_OPERATION_GETFILEINFO_CNF;

typedef struct
{
	unsigned int   file_size;	// Total size of the current file. Only available on EOF
	unsigned char	stage;		// CREATE, WRITE, EOF
}FILEOPERATION_STREAM_BLOCK;

typedef struct
{
    unsigned char dest_file_name[256];
	FILEOPERATION_STREAM_BLOCK stream_block;
}FILE_OPERATION_SENDFILE_REQ;

typedef struct  
{
	unsigned int send_result;
}FILE_OPERATION_SENDFILE_CNF;

typedef struct  
{
    unsigned char source_file_name[256];
}FILE_OPERATION_RECEIVEFILE_REQ;

typedef struct  
{
    FILEOPERATION_STREAM_BLOCK stream_block;
	unsigned int receive_result;
}FILE_OPERATION_RECEIVEFILE_CNF;

typedef union 
{
	FILE_OPERATION_PARSE_CNF   parse_cnf;
	FILE_OPERATION_GETFILEINFO_CNF  getfileinfo_cnf;
	FILE_OPERATION_SENDFILE_CNF  sendfile_cnf;
	FILE_OPERATION_RECEIVEFILE_CNF receivefile_cnf;

}FT_FILE_OPERATION_RESULT;


typedef union
{
	FILE_OPERATION_PARSE_REQ   parse_req;
	FILE_OPERATION_GETFILEINFO_REQ  getfileinfo_req;
	FILE_OPERATION_SENDFILE_REQ  sendfile_req;
	FILE_OPERATION_RECEIVEFILE_REQ receivefile_req;

}FT_FILE_OPERATION_CMD;


typedef struct
{
	FT_H header;
	FT_FILE_OPERATION_OP	 type;
	FT_FILE_OPERATION_CMD   cmd;
}FT_FILE_OPERATION_REQ;
		
typedef struct 
{
	FT_H header;
	FT_FILE_OPERATION_OP  type;
	FT_FILE_OPERATION_RESULT result;
	unsigned char status;
}FT_FILE_OPERATION_CNF;


#ifdef FT_SDCARD_FEATURE
#include "meta_sdcard_para.h"
#endif

////////////////////Target log ctrl///////////////////////////
typedef enum{
	FT_MDLOGGER_OP_SWITCH_TYPE           = 0,
	FT_MDLOGGER_OP_QUERY_STATUS          = 1,
	FT_MDLOGGER_OP_QUERY_NORMALLOG_PATH  = 2,
	FT_MDLOGGER_OP_QUERY_EELOG_PATH		 = 3,
	FT_MOBILELOG_OP_SWITCH_TYPE			 = 4,
	FT_MOBILELOG_OP_QUERY_LOG_PATH		 = 5,
	FT_TARGETLOG_OP_PULL                 = 6,
	FT_TARGETLOG_OP_PULLING_STATUS       = 7,
	FT_MDLOGGER_OP_SET_FILTER		     = 8,
	FT_CONNSYSLOG_OP_SWITCH_TYPE	     = 9,
	FT_TARGETLOG_CTRL_OP_END = 0x0fffffff
}FT_TARGETLOG_CTRL_OP;

typedef struct
{
	unsigned int mode;     //1: usb, 2:SD
	unsigned int action;   //0: stop, 1: start
}MDLOGGER_CTRL_REQ;

typedef struct
{
	unsigned int reserved;
}MDLOGGER_CTRL_CNF;

typedef struct
{
	unsigned int reserved;
}MDLOGGER_QUERY_STATUS_REQ;

typedef struct
{
	unsigned int status;  //0: stop 1:logging
}MDLOGGER_QUERY_STATUS_CNF;

typedef struct
{
	unsigned int reserved;
}MDLOGGER_QUERY_LOGPATH_REQ;

typedef struct
{
	unsigned char path[256];
}MDLOGGER_QUERY_LOGPATH_CNF;

typedef struct 
{
	unsigned int type;      //0:default filter 1:customization filter
}MDLOGGER_SET_FILTER_REQ;


// for mobilelog ctrl
typedef struct
{
	unsigned int mode;     //reserved, in the future maybe->1: usb, 2:SD
	unsigned int action;   //0: stop, 1: start
}MOBILELOG_SWITCH_MODE_REQ;

typedef struct
{
	unsigned int reserved;
}MOBILELOG_SWITCH_MODE_CNF;

typedef struct
{
	unsigned int reserved;
}MOBILELOG_QUERY_LOGPATH_REQ;

typedef struct
{
	unsigned char path[256];
}MOBILELOG_QUERY_LOGPATH_CNF;

typedef struct
{
	unsigned int type;      //0:modemlog 1:mobilelog 2:connsysLog 3:mddb
	unsigned int action;    //0:stop  1:start
}TARGET_LOG_PULL_REQ;

typedef struct
{
	unsigned int reserved;
}TARGET_LOG_PULL_CNF;

typedef struct
{
	unsigned int type;	//0:modemlog 1:mobilelog 2:modem EE log 3:connsyslog 4:mddb
}TARGET_LOG_PULL_STATUS_REQ;

typedef struct
{
	unsigned int status;  //0:pulling 1:done
}TARGET_LOG_PULL_STATUS_CNF;

// for connsyslog ctrl
typedef struct
{
	unsigned int mode;     //reserved, in the future maybe->1: usb, 2:SD
	unsigned int action;   //0: stop, 1: start
}CONNSYSLOG_SWITCH_MODE_REQ;

typedef struct
{
	unsigned int reserved;
}CONNSYSLOG_SWITCH_MODE_CNF;


typedef union
{
	MDLOGGER_CTRL_CNF           mdlogger_ctrl_cnf;
	MDLOGGER_QUERY_STATUS_CNF   mdlogger_status_cnf;
	MDLOGGER_QUERY_LOGPATH_CNF  mdlogger_logpath_cnf;
	MOBILELOG_SWITCH_MODE_CNF   mobilelog_ctrl_cnf;
	MOBILELOG_QUERY_LOGPATH_CNF mobilelog_logpath_cnf;
	CONNSYSLOG_SWITCH_MODE_CNF  connsyslog_ctrl_cnf;
	TARGET_LOG_PULL_CNF         targetlog_pull_cnf;
	TARGET_LOG_PULL_STATUS_CNF  targetlog_pulling_status_cnf;
} FT_TARGETLOG_CTRL_RESULT;


typedef union
{
	MDLOGGER_CTRL_REQ           mdlogger_ctrl_req;
	MDLOGGER_QUERY_STATUS_REQ   mdlogger_status_req;
	MDLOGGER_QUERY_LOGPATH_REQ  mdlogger_logpath_req;
	MDLOGGER_SET_FILTER_REQ		mdlogger_setfilter_req;
	MOBILELOG_SWITCH_MODE_REQ   mobilelog_ctrl_req;
	MOBILELOG_QUERY_LOGPATH_REQ mobilelog_logpath_req;
	CONNSYSLOG_SWITCH_MODE_REQ  connsyslog_ctrl_req;
	TARGET_LOG_PULL_REQ         targetlog_pull_req;
	TARGET_LOG_PULL_STATUS_REQ  targetlog_pulling_status_req;
} FT_TARGETLOG_CTRL_CMD;

typedef struct
{
	FT_H header;
	FT_TARGETLOG_CTRL_OP	type;
	FT_TARGETLOG_CTRL_CMD   cmd;
}FT_TARGETLOG_CTRL_REQ;

typedef struct
{
	FT_H header;
	FT_TARGETLOG_CTRL_OP     type;
	FT_TARGETLOG_CTRL_RESULT result;
	unsigned char status;
}FT_TARGETLOG_CTRL_CNF;


////////////////////APDB function///////////////////////////
typedef enum
{
    FT_APDB_OP_QUERYPATH = 0,
    FT_APDB_OP_END = 0x0fffffff
}FT_APDB_OP;

typedef struct
{
    unsigned int reserved; //reserved, no use
}QUERY_APDBPATH_REQ;

typedef struct
{
    unsigned char apdb_path[128];
}QUERY_APDBPATH_CNF;

typedef union
{
    QUERY_APDBPATH_CNF   query_apdbpath_cnf;
} FT_APDB_RESULT;


typedef union
{
    QUERY_APDBPATH_REQ   query_apdbpath_req;
} FT_APDB_CMD;

typedef struct
{
    FT_H header;
    FT_APDB_OP  type;
    FT_APDB_CMD   cmd;
}FT_APDB_REQ;

typedef struct
{
    FT_H header;
    FT_APDB_OP     type;
    FT_APDB_RESULT result;
    unsigned int status;
}FT_APDB_CNF;


////////////////////Attestation Key///////////////////////////
typedef enum 
{
    FT_ATTESTATIONKEY_INSTALL_SET = 0,
	FT_ATTESTATIONKEY_INSTALL_END = 0x0fffffff
}FT_ATTESTATIONKEY_INSTALL_OP;

typedef struct
{
    unsigned int    file_size;
    unsigned char   stage;
}ATTESTATIONKEY_INSTALL_SET_REQ;

typedef struct
{
    unsigned int result;
}ATTESTATIONKEY_INSTALL_SET_CNF;

typedef union
{
    ATTESTATIONKEY_INSTALL_SET_REQ   set_req;
}FT_ATTESTATIONKEY_INSTALL_CMD;

typedef union
{
    ATTESTATIONKEY_INSTALL_SET_CNF    set_cnf;
}FT_ATTESTATIONKEY_INSTALL_RESULT;

typedef struct
{
	FT_H						   header;
	FT_ATTESTATIONKEY_INSTALL_OP   type;
	FT_ATTESTATIONKEY_INSTALL_CMD  cmd;
}FT_ATTESTATIONKEY_INSTALL_REQ;

typedef struct
{
	FT_H							 header;
	FT_ATTESTATIONKEY_INSTALL_OP     type;
	FT_ATTESTATIONKEY_INSTALL_RESULT result;
	unsigned char					 status;
}FT_ATTESTATIONKEY_INSTALL_CNF;

////////////////////SysEnv///////////////////////////
typedef enum{
	FT_SYSENV_SET = 0,
	FT_SYSENV_GET = 1,
	FT_SYSENV_END = 0x0fffffff
}FT_SYSENV_OP;

typedef struct 
{
	unsigned char name[256];
	unsigned char value[256];	
}SYS_ENV_SET_REQ;

typedef struct 
{
	unsigned int reserved;
}SYS_ENV_SET_CNF;

typedef struct 
{
	unsigned char name[256];
}SYS_ENV_GET_REQ;

typedef struct 
{
	unsigned char value[256];
}SYS_ENV_GET_CNF;


typedef union 
{
	SYS_ENV_SET_CNF     sysenv_set_cnf;
	SYS_ENV_GET_CNF     sysenv_get_cnf;	
} FT_SYS_ENV_RESULT;


typedef union
{
	SYS_ENV_SET_REQ     sysenv_set_req;
	SYS_ENV_GET_REQ     sysenv_get_req;	
} FT_SYS_ENV_CMD;

typedef struct
{
	FT_H			header;
	FT_SYSENV_OP    type;
	FT_SYS_ENV_CMD  cmd;
}FT_SYS_ENV_REQ;

typedef struct
{
	FT_H		  		header;
	FT_SYSENV_OP  		type;
	FT_SYS_ENV_RESULT 	result;
	unsigned int		status;
}FT_SYS_ENV_CNF;


typedef enum
{
	FT_CLOCK_SET = 0,
	FT_CLOCK_GET = 1,
	FT_CLOCK_END = 0x0fffffff
}FT_TARGETCLOCK_OP;

typedef struct
{
	unsigned int year;
	unsigned int mon;
	unsigned int day;
	unsigned int hour;
	unsigned int min;
	unsigned int sec;
	unsigned int ms;
}SET_TARGET_CLOCK_REQ;

typedef struct
{
	unsigned int reserved;
}SET_TARGET_CLOCK_CNF;

typedef union
{
	SET_TARGET_CLOCK_REQ     set_clock_req;
}FT_TARGET_CLOCK_CMD;

typedef union
{
	SET_TARGET_CLOCK_CNF     set_clock_cnf;	
}FT_TARGET_CLOCK_RESULT;

typedef struct
{
	FT_H				header;
	FT_TARGETCLOCK_OP	type;
	FT_TARGET_CLOCK_CMD	cmd;
}FT_TARGETCLOCK_REQ;
		
typedef struct
{
	FT_H					header;
	FT_TARGETCLOCK_OP		type;
	FT_TARGET_CLOCK_RESULT	result;
	unsigned int			status;
}FT_TARGETCLOCK_CNF;

typedef enum
{
	FT_CTRL_POWEROFF,
	FT_CTRL_REBOOT,
	FT_CTRL_REBOOT_RECOVERY,
	FT_CTRL_REBOOT_BYDELAY,
	FT_CTRL_DISCONN_ATM,
	FT_CTRL_CHECKUSB_POWEROFF,
	FT_CTRL_DONOTHING,
	FT_CTRL_TARGET_OP_END = 0x0fffffff
}FT_DISCONN_TARGET_OP;

typedef struct
{
	unsigned int    delay;
	unsigned int    reserved; 
}FT_TARGET_DISCONNECT_REQ;

typedef struct
{
	unsigned int    reserved; 
}FT_TARGET_DISCONNECT_CNF;

typedef union
{
	FT_TARGET_DISCONNECT_REQ   disconnect_req;
}FT_DISCONNECT_CMD;

typedef union
{
	FT_TARGET_DISCONNECT_CNF   disconnect_cnf;	
}FT_DISCONNECT_RESULT;

typedef struct
{
	FT_H				    header;
	FT_DISCONN_TARGET_OP	type;
	FT_DISCONNECT_CMD	    cmd;
}FT_DISCONNECT_REQ;

typedef struct
{
	FT_H					header;
	FT_DISCONN_TARGET_OP	type;
	FT_DISCONNECT_RESULT	result;
	unsigned int			status;
}FT_DISCONNECT_CNF;

#ifdef __cplusplus
}
#endif

#endif




