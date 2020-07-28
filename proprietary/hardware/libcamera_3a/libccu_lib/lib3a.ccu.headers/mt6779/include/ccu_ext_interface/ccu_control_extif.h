/*! \file ccu_control_extif.h
    \brief External interfaces for CCU Control
*/
#ifndef __CCU_CTRL_INTERFACE__
#define __CCU_CTRL_INTERFACE__

#include "ccu_ext_interface/ccu_types.h"
#include "ccu_ext_interface/ccu_ae_extif.h"
#include "ccu_ext_interface/ccu_ltm_extif.h"
#include "ccu_ext_interface/ccu_af_extif.h"
#include "ccu_ext_interface/af_cxu_param.h"

/******************************************************************************
* Task definition
******************************************************************************/
/**
* Enum for function NSCcuIf::ICcuCtrl::ccuControl()
*/
typedef enum
{
    DELIMITER_SYSCTRL_MSG_MIN = 0, /*To identify ccu control msg count*/
    /*Receive by CCU*/
    MSG_TO_CCU_IDLE = DELIMITER_SYSCTRL_MSG_MIN,
    MSG_TO_CCU_SUSPEND, //Request CCU to suspend on corresponding TG
    MSG_TO_CCU_RESUME, //Request CCU to resume on corresponding TG
    MSG_TO_CCU_SHUTDOWN, //Request CCU to shutdown
    MSG_TO_CCU_UPDATE_TG_SENSOR_MAP, //Request CCU to update tg to sensor_idx mapping
    MSG_TO_CCU_INVALIDATE_TG_SENSOR_MAP, //Request CCU to invalidate tg to sensor_idx mapping

    DELIMITER_SYSCTRL_MSG_1, /*To identify ccu control msg count*/
    /*CCU internal task generated in HW isr*/
    MSG_CCU_INTERNAL_VSYNC_SYS = DELIMITER_SYSCTRL_MSG_1,
    MSG_CCU_INTERNAL_P1_DONE_SYS,
    MSG_CCU_INTERNAL_PR_LOG,
    MSG_CCU_INTERNAL_FORCE_SHUTDOWN, //Request CCU to shutdown
    MSG_CCU_INTERNAL_NS_VSYNC, //non-subsampled vsync

    DELIMITER_SYSCTRL_MSG_MAX /*To identify ccu control msg count*/
} ccu_msg_id;

typedef enum
{
    MSG_TO_APMCU_FLUSH_LOG, //CCU Request APMCU to print out CCU logs
    MSG_TO_APMCU_CCU_ASSERT, //CCU inform APMCU that CCU ASSERT occurs
    MSG_TO_APMCU_CCU_WARNING, //CCU inform APMCU that CCU WARNING occurs
    MSG_TO_APMCU_CAM_AFO_i,
    MSG_TO_APMCU_CAM_AFO_ABORT = 255
} ccu_to_ap_msg_id;

enum ccu_feature_type
{
    CCU_FEATURE_UNDEF = 0x0,
    CCU_FEATURE_MIN = 0x1,
    CCU_FEATURE_AE = CCU_FEATURE_MIN,
    CCU_FEATURE_AF,
    CCU_FEATURE_LTM,
    CCU_FEATURE_3ACTRL,
    CCU_FEATURE_SYSCTRL,
    CCU_FEATURE_MAX,
};
#define CCU_FEATURE_COUNT (CCU_FEATURE_MAX - CCU_FEATURE_MIN)

//CCU_MSG_CNT_MAX is used to evaluate number of reserved slots for resources(like buffers)
//must larger than the most msg id count of all features
#define CCU_MSG_CNT_MAX 22

//this should be synced with enum IMGSENSOR_SENSOR_IDX in kd_camera_feature.h (device/mediatek/common)
enum CCU_IMGSENSOR_SENSOR_IDX {
    CCU_IMGSENSOR_SENSOR_IDX_MIN_NUM = 0,
    CCU_IMGSENSOR_SENSOR_IDX_MAIN = CCU_IMGSENSOR_SENSOR_IDX_MIN_NUM,
    CCU_IMGSENSOR_SENSOR_IDX_SUB,
    CCU_IMGSENSOR_SENSOR_IDX_MAIN2,
    CCU_IMGSENSOR_SENSOR_IDX_SUB2,
    CCU_IMGSENSOR_SENSOR_IDX_MAIN3,
    CCU_IMGSENSOR_SENSOR_IDX_SUB3,
    CCU_IMGSENSOR_SENSOR_IDX_MAIN4,
    CCU_IMGSENSOR_SENSOR_IDX_SUB4,
    CCU_IMGSENSOR_SENSOR_IDX_MAIN5,
    CCU_IMGSENSOR_SENSOR_IDX_SUB5,
    CCU_IMGSENSOR_SENSOR_IDX_MAIN6,
    CCU_IMGSENSOR_SENSOR_IDX_SUB6,
    CCU_IMGSENSOR_SENSOR_IDX_MAX_NUM,
    CCU_IMGSENSOR_SENSOR_IDX_NONE,
};

enum ccu_tg_info
{
    CCU_CAM_TG_NONE = 0x0,
    CCU_CAM_TG_MIN  = 0x1,
    CCU_CAM_TG_1    = 0x1,
    CCU_CAM_TG_2    = 0x2,
    CCU_CAM_TG_3    = 0x3,
    CCU_CAM_TG_MAX
};
#define CCU_TG2IDX(TG) (TG-1)
#define CCU_CAM_TG_MIN_IDX (CCU_CAM_TG_MIN - 1)
#define CCU_CAM_TG_MAX_IDX (CCU_CAM_TG_MAX - 1)
#define CCU_CAM_TG_CNT (CCU_CAM_TG_MAX - CCU_CAM_TG_MIN)

enum ccu_raw_info
{
    CCU_CAM_RAW_NONE = 0x0,
    CCU_CAM_RAW_MIN  = 0x1,
    CCU_CAM_RAW_A    = 0x1,
    CCU_CAM_RAW_B    = 0x2,
    CCU_CAM_RAW_C    = 0x3,
    CCU_CAM_RAW_MAX
};
#define CCU_RAW2IDX(TG) (TG-1)
#define CCU_CAM_RAW_MIN_IDX (CCU_CAM_RAW_MIN - 1)
#define CCU_CAM_RAW_MAX_IDX (CCU_CAM_RAW_MAX - 1)
#define CCU_CAM_RAW_CNT (CCU_CAM_RAW_MAX - CCU_CAM_RAW_MIN)

//Make sure struct ccu_msg definition is synchronized with the one located in kernel header (ccu_ext_interface.h)
struct ccu_msg {
    volatile enum ccu_feature_type feature_type;
    volatile uint32_t msg_id;
    volatile MUINT32 in_data_ptr;
    volatile MUINT32 out_data_ptr;
    volatile enum ccu_tg_info tg_info;
    volatile uint32_t sensor_idx; //new
};

struct ccu2ap_msg
{
    volatile uint32_t msg_id;
    volatile MUINT32 in_data_ptr;
    volatile MUINT32 out_data_ptr;
};

struct ap2ccu_ipc_t
{
    volatile MUINT32 write_cnt;
    volatile MUINT32 read_cnt;
    struct ccu_msg msg;
    volatile MBOOL ack;
};

struct ccu_tg2sensor_pair
{
    uint32_t tg_info;
    uint32_t sensor_idx;
};

struct __attribute__ ((aligned (8))) shared_buf_map
{
    /*** from CCU->APMCU ***/
    MUINT32 ipc_in_data_addr_ccu;
    MUINT32 ipc_out_data_addr_ccu;
    MUINT32 ipc_in_data_base_offset;
    MUINT32 ipc_out_data_base_offset;
    MUINT32 ipc_base_offset;

    //>>>>>>> AE datas
    struct exif_data_addrs_s exif_data_addrs[CCU_AE_INSTANCE_CAPACITY];
    struct ccu_ae_active_output_handle ae_actout_handle[CCU_AE_INSTANCE_CAPACITY];
    //since CCU SRAM bus protocal(APB) do not support byte access, use uint32_t instead of bool here
    uint32_t is_ae_output_ready[CCU_AE_INSTANCE_CAPACITY];
    //since CCU SRAM bus protocal(APB) do not support byte access, use uint32_t instead of bool here
    uint32_t is_ae_stat_ready[CCU_AE_INSTANCE_CAPACITY];
    //since CCU SRAM bus protocal(APB) do not support byte access, use uint32_t instead of bool here
    uint32_t is_ae_started[CCU_AE_INSTANCE_CAPACITY];
    //since CCU SRAM bus protocal(APB) do not support byte access, use uint32_t instead of bool here
    uint32_t is_ae_instant_mode[CCU_AE_INSTANCE_CAPACITY];

    //>>>>>>> LTM datas
    struct ccu_ltm_debug_info_handle ltm_debug_info_handle[CCU_LTM_INSTANCE_MAX];

    //AF datas
    af_shared_buf_map_T af_shared_buf[CCU_AF_INSTANCE_CAPACITY];

    /*** from APMCU->CCU ***/
    MUINT32 bkdata_ddr_buf_mva;
};

/*notify : tag number cannot over 16 now*/
typedef enum
{
    CCU_FLOW_BASIC_LOGTAG = 0,
    CCU_MUST_LOGTAG,
    CCU_SENSOR_LOGTAG,
    CCU_LENS_LOGTAG,
    CCU_AE_FLOW_LOGTAG,
    CCU_AE_ALGO_LOGTAG,
    CCU_AE_SYNC_ALGO_LOGTAG,
    CCU_AF_FLOW_LOGTAG,
    CCU_AF_ALGO_LOGTAG,
    CCU_AF_SYNC_ALGO_LOGTAG,
    CCU_LTM_FLOW_LOGTAG,
    CCU_LTM_ALGO_LOGTAG,
    CCU_I2C_LOGTAG,
    CCU_VERBOSE_LOGTAG, //placeholder
    CCU_RESERVE2_LOGTAG, //placeholder
    CCU_RESERVE1_LOGTAG, //placeholder
} ccu_log_tag;
/*notify : tag number cannot over 16 now*/

#define PRINT_S "1"
#define IGNORE_S "2"
#define DSIABLE_S "0"

#define PRINT 0x1
#define IGNORE 0x2
#define DSIABLE 0x0

#define CCU_LOG_MASK(TAG_NAME) 0x3 << CCU_LOG_MASK_OFFSET(TAG_NAME)
#define CCU_LOG_MASK_OFFSET(TAG_NAME) TAG_NAME*2

/******************************************************************************
* struct size matching detection
******************************************************************************/
#define GEN_STRUCT_ENTRY_CCU(NAME_APMCU, NAME_CCU) sizeof(NAME_CCU)
#define GEN_STRUCT_ENTRY_APMCU(NAME_APMCU, NAME_CCU) sizeof(NAME_APMCU)
#define GEN_STRUCT_STRING_CCU(NAME_APMCU, NAME_CCU) #NAME_CCU
#define GEN_STRUCT_STRING_APMCU(NAME_APMCU, NAME_CCU) #NAME_APMCU

#define GEN_CCU_STRUCT_SIZE_MAP(GEN_ENTRY) \
    GEN_ENTRY(COMPAT_CCU_AeAlgo, CCU_AeAlgo), \
    GEN_ENTRY(COMPAT_AE_CORE_INIT, AE_CORE_INIT), \
    GEN_ENTRY(AE_CORE_MAIN_OUT, AE_CORE_MAIN_OUT), \
    GEN_ENTRY(COMPAT_AE_CORE_CTRL_RUN_TIME_INFO, AE_CORE_CTRL_RUN_TIME_INFO), \
    GEN_ENTRY(AE_CORE_CTRL_CCU_VSYNC_INFO, AE_CORE_CTRL_CCU_VSYNC_INFO), \
    GEN_ENTRY(COMPAT_CCU_SYNC_AE_CAM_INPUT_T, CCU_SYNC_AE_CAM_INPUT_T), \
    GEN_ENTRY(COMPAT_CCU_SYNC_AE_INPUT_T, CCU_SYNC_AE_INPUT_T), \
    GEN_ENTRY(AF_HW_INIT_INPUT_T, AF_HW_INIT_INPUT_T), \
    GEN_ENTRY(AF_INPUT_T, AF_INPUT_T), \
    GEN_ENTRY(struct shared_buf_map, struct shared_buf_map),\
    GEN_ENTRY(AFAcquireBuf_Reg_INFO_OUT_AP_T, AFAcquireBuf_Reg_INFO_OUT_AP_T),\
    GEN_ENTRY(struct exif_data_addrs_s, struct exif_data_addrs_s), \
    GEN_ENTRY(struct ccu_ae_active_output_handle, struct ccu_ae_active_output_handle),\
    GEN_ENTRY(struct ccu_ltm_debug_info_handle, struct ccu_ltm_debug_info_handle),\
    GEN_ENTRY(CCUAFsync_info_T, CCUAFsync_info_T), \
    GEN_ENTRY(AF_SyncInfo_T, AF_SyncInfo_T), \
    

/******************************************************************************
* Error code definition
******************************************************************************/
#define CCU_NO_ERROR               (0)

/******************************************************************************
* Status definition
******************************************************************************/
#define CCU_STATUS_INIT_DONE              0xffff0000
#define CCU_STATUS_INIT_DONE_2            0xffff00a5

#endif
