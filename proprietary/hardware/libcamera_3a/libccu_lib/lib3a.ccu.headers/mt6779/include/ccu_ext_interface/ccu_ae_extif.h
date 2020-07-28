#ifndef __CCU_AE_INTERFACE__
#define __CCU_AE_INTERFACE__

#include "ccu_ext_interface/ccu_types.h"
#include "algorithm/ae_algo_ctrl.h"

#define CCU_AE_INSTANCE_CAPACITY 2
#define CCU_AE_INSTANCE_IDX_VALIDATE(IDX) ((IDX >= 0) && (IDX < CCU_AE_INSTANCE_CAPACITY))

/******************************************************************************
* AE Command ID definition
******************************************************************************/
enum ccu_msg_id_ae
{
    /*To identify ccu control msg count*/
    DELIMITER_AE_MSG_MIN = 0,
    /*Receive by CCU*/
    MSG_TO_CCU_SENSOR_INIT = DELIMITER_AE_MSG_MIN,
    MSG_TO_CCU_AE_INIT,
    MSG_TO_CCU_AE_ALGO_INIT,
    MSG_TO_CCU_AE_START,
    MSG_TO_CCU_AE_STOP,
    MSG_TO_CCU_SET_AP_AE_CTRL_DATA_PERFRAME,
    MSG_TO_CCU_SET_AP_AE_CTRL_DATA_ONCHANGE,
    MSG_TO_CCU_SET_MAX_FRAMERATE,
    MSG_TO_CCU_GET_AE_DEBUG_INFO,
    MSG_TO_CCU_HDR_AE_INPUT_READY,
    MSG_TO_CCU_SET_AUTO_FLK,
    MSG_TO_CCU_AE_STAT_START,
    MSG_TO_CCU_AE_STAT_STOP,
    MSG_TO_CCU_SWITCH_HIGH_FPS,
    MSG_TO_CCU_MANUAL_EXP_CTRL,
    MSG_TO_CCU_MANUAL_EXP_CTRL_DISABLE,
    MSG_TO_CCU_SET_SUPER_NIGHT_MODE,
    MSG_TO_CCU_AE_SET_FD_INFO,
    MSG_TO_CCU_AE_BIND_INSTANCE,

    /*To identify ccu control msg count*/
    DELIMITER_AE_MSG_1,
    /*CCU internal task generated in HW isr*/
    MSG_CCU_INTERNAL_BANK_AE_ALGO = DELIMITER_AE_MSG_1,
    MSG_CCU_INTERNAL_AAO_DONE,
    MSG_CCU_INTERNAL_VSYNC_AE,
    MSG_CCU_INTERNAL_HIST_DONE_AE,
    MSG_CCU_INTERNAL_SENSOR_TIMER,
    MSG_CCU_INTERNAL_NS_VSYNC_AE,

    /*To identify ccu control msg count*/
    DELIMITER_AE_MSG_MAX
};

/******************************************************************************
* AE algo related interface
******************************************************************************/
struct ccu_ae_bind_instance_output
{
    uint32_t instance_idx;
};

typedef struct
{
    //From CPU pre-setted
    int hdr_stat_en;    //needed on AAO only
    int ae_overcnt_en;  //needed on AAO only
    int tsf_stat_en;    //needed on AAO only
    int bits_num;       //needed on AAO only
    int ae_footprint_hum;
    int ae_footprint_vum;
    int ae_cust_stat_enable;
    int ae_cust_stat_width;
    int ae_cust_stat_height;
    int aao_line_width;
    int pso_line_width;
    strAERealSetting exp_on_start;
    CCU_AE_TargetMODE ae_target_mode;
    uint32_t bin_sum_ratio;
} CCU_AE_CONTROL_CFG_T;

typedef struct
{
    CCU_AE_CONTROL_CFG_T control_cfg;
} CCU_AE_INITI_PARAM_T;

typedef struct
{
    AE_CORE_INIT algo_init_param;
    //CCU_strSyncAEInitInfo *sync_algo_init_param;
} CCU_AE_ALGO_INITI_PARAM_T;

typedef struct
{
    MUINT32 AEWinBlock_u4XHi;
    MUINT32 AEWinBlock_u4XLow;
    MUINT32 AEWinBlock_u4YHi;
    MUINT32 AEWinBlock_u4YLow;
} ccu_ae_roi;

typedef struct
{
    //AE control status
    MINT32 magic_num;
    MINT32 req_num;
    MBOOL force_reset_ae_status;

    // Algo frame sync data
    AE_CORE_CTRL_CCU_VSYNC_INFO algo_frame_data;
} ccu_ae_ctrldata_perframe;

struct ccu_dynamic_high_fps_info
{
    strAERealSetting manual_exp;
    uint32_t target_fps;
};

enum ccu_manual_timing
{
    CCU_MANUAL_EXP_TIMING_NONE = 0,
    CCU_MANUAL_EXP_TIMING_PRESET,
    CCU_MANUAL_EXP_TIMING_SET
};

struct ccu_manual_exp_info
{
    enum ccu_manual_timing timing;
    strAERealSetting real_setting;
    strAERealSetting binsum_conv_setting;
};

struct ccu_manual_exp_info_output
{
    bool is_supported;
};

struct ccu_manual_exp_disable_info
{
    enum ccu_manual_timing timing;
};

enum ccu_iso_shutter_priority_mode
{
    CCU_PRIORITY_OFF = 0,
    CCU_SHUTTER_PRIORITY,
    CCU_ISO_PRIORITY,
};

struct ccu_iso_shutter_priority_info
{
    enum ccu_iso_shutter_priority_mode mode;
    MUINT32  shutter;
    MUINT32  iso;
};

struct ccu_super_night_mode_info
{
    MBOOL enable;
    CCU_AE_CUST_Super_Night_Param_T ae_cust_param;
};

struct ccu_ae_auto_flk_data
{
    MBOOL auto_flicker_en; /**< Specifies if need to turn on auto anti flicker function*/
};

typedef struct
{
    MUINT16 framerate;
    kal_bool min_framelength_en;
} ccu_max_framerate_data;

struct ccu_ae_output
{
    AE_CORE_MAIN_OUT algo_output;
    strAERealSetting real_setting;
    MBOOL is_ae_output_valid;
    MUINT32 ae_line_cnt;
};

#define CCU_HDR_DATA_BUF_CNT 2
struct ccu_hdr_ae_input_data
{
    MUINT32 u4CurHDRRatio;
	MUINT32 u4BufSizeX;
	MUINT32 u4BufSizeY;
    MUINT32 hdr_data_buffer_mva;
};

#define AE_ACTOUT_RING_SIZE 2
struct ccu_ae_active_output_handle
{
    uint32_t ae_output_ddr_mva[AE_ACTOUT_RING_SIZE];
    uint32_t ae_output_ridx;
    uint32_t ae_algo_data_ddr_mva[AE_ACTOUT_RING_SIZE];
    uint32_t ae_algo_data_ridx;
    uint32_t ae_stat_ddr_mva[AE_ACTOUT_RING_SIZE];
    uint32_t ae_stat_ridx;
};

struct exif_data_addrs_s
{
    MUINT32 ae_algo_data_addr;
    MUINT32 ae_init_data_addr;
    MUINT32 ae_vsync_info_addr;
    MUINT32 aesync_algo_in_addr;
    MUINT32 aesync_algo_out_addr;
};

#endif
