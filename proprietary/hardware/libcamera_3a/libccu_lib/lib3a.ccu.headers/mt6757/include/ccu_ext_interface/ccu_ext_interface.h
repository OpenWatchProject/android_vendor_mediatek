#ifndef __CCU_INTERFACE__
#define __CCU_INTERFACE__

#include "ccu_ext_interface/ccu_types.h"
#include "algorithm/ae_algo_ctrl.h"

//--fpgaep
extern MBOOL sec_vsync_pushed;

/******************************************************************************
* Task definition
******************************************************************************/
typedef enum
{
    /*Receive by CCU*/
    //CCU Task
    MSG_TO_CCU_IDLE = 0x00000000,
    MSG_TO_CCU_SENSOR_INIT,
    MSG_TO_CCU_AE_INIT,
    MSG_TO_CCU_AE_START,
    MSG_TO_CCU_SET_SKIP_ALGO,
    MSG_TO_CCU_SET_AP_AE_READY,
    MSG_TO_CCU_SET_AP_AE_FRAME_SYNC_DATA,
    MSG_TO_CCU_SET_AP_AE_ONCHANGE_DATA,
    MSG_TO_CCU_GET_CCU_OUTPUT,
    MSG_TO_CCU_SET_AP_AE_OUTPUT,
    MSG_TO_CCU_SET_AE_ROI,
    MSG_TO_CCU_SET_AE_EV,
    MSG_TO_CCU_SET_MAX_FRAMERATE,
    MSG_TO_CCU_SHUTDOWN,
    /*todo-add timer sync task*/
    //APMCU Ack
    //CCU internal task
    MSG_CCU_INTERNAL_BANK_AE_ALGO,
    MSG_CCU_INTERNAL_FRAME_AE_ALGO,
    MSG_CCU_INTERNAL_VSYNC_AE_SYNC,

    /*Send to APMCU*/
    //CCU Ack/Done
    MSG_CCU_DONE_SHUTDOWN,
    MSG_CCU_DONE_SENSOR_INIT,
    MSG_CCU_DONE_AE_INIT,
    MSG_CCU_ACK_AE_START,
    MSG_CCU_ACK_SET_SKIP_ALGO,
    MSG_CCU_ACK_SET_AP_AE_READY,
    MSG_CCU_ACK_SET_AP_AE_FRAME_SYNC_DATA,
    MSG_CCU_ACK_SET_AP_AE_ONCHANGE_DATA,
    MSG_CCU_ACK_GET_CCU_OUTPUT,
    MSG_CCU_ACK_SET_AP_AE_OUTPUT,
    MSG_CCU_ACK_SET_AE_ROI,
    MSG_CCU_ACK_SET_AE_EV,
    MSG_CCU_ACK_SET_MAX_FRAMERATE,

    //APMCU Task
    MSG_TO_APMCU_FLUSH_LOG,
    MSG_TO_APMCU_CCU_ASSERT,
    MSG_TO_APMCU_CCU_WARNING
} ccu_msg_id;

typedef struct _ccu_msg_t //12bytes
{
    volatile ccu_msg_id msg_id;
    volatile MUINT32 in_data_ptr;
    volatile MUINT32 out_data_ptr;
} ccu_msg_t;

/******************************************************************************
* Special isr task (execute in isr)
******************************************************************************/
#define ISR_SP_TASK_SHUTDOWN 0x000000FF
#define ISR_SP_TASK_TRG_I2C_DONE 0x000000C1
#define ISR_SP_TASK_RST_I2C_DONE 0x000000B9

/******************************************************************************
* Special ap isr task (execute in apmcu_isr)
******************************************************************************/
#define APISR_SP_TASK_TRIGGER_I2C 0x000000A1
#define APISR_SP_TASK_RESET_I2C   0x000000A2

/******************************************************************************
* Error code definition
******************************************************************************/
#define CCU_ERROR_NO               (0)
#define CCU_ERROR_QUEUE_FULL       (1)

/******************************************************************************
* Status definition
******************************************************************************/
#define CCU_STATUS_INIT_DONE              0xffff0000
#define CCU_STATUS_INIT_DONE_2            0xffff00a5

/******************************************************************************
* Direct-link source definition
******************************************************************************/
#define AE_STREAM_DL_SRC_AAO  0x00000000
#define AE_STREAM_DL_SRC_PSO  0x00000001

/******************************************************************************
* AE algo related interface
******************************************************************************/
typedef struct
{
    //From CPU pre-setted
    int hdr_stat_en;    //needed on AAO only
    int ae_overcnt_en;  //needed on AAO only
    int tsf_stat_en;    //needed on AAO only
    int bits_num;       //needed on AAO only
    int ae_footprint_hum;
    int ae_footprint_vum;
    int ae_dl_source;   //0=AAO, 1=PSO
    int ae_footprint_lines_per_bank;
} CCU_AE_CONTROL_CFG_T;

typedef struct
{
    CCU_AE_CONTROL_CFG_T control_cfg;
    AE_CORE_INIT algo_init_param;
} CCU_AE_INITI_PARAM_T;

typedef struct
{
    MUINT32 AEWinBlock_u4XHi;
    MUINT32 AEWinBlock_u4XLow;
    MUINT32 AEWinBlock_u4YHi;
    MUINT32 AEWinBlock_u4YLow;
} ccu_ae_roi;

typedef struct
{
    MBOOL is_zooming;
    MINT32 magic_num;
} ccu_ae_frame_sync_data;

typedef struct
{
    MBOOL auto_flicker_en;
    MUINT32 m_u4IndexMax;
    MUINT32 m_u4IndexMin;
    strEvPline *pCurrentTable;
} ccu_ae_onchange_data;


/******************************************************************************
******************************************************************************/
typedef enum SENSOR_SCENARIO
{
    SENSOR_SCENARIO_PREVIEW             = 0x00,
    SENSOR_SCENARIO_CAPTURE             = 0x01,
    SENSOR_SCENARIO_NORMAL_VIDEO        = 0x02,     // a.k.a. Video Preview on CPU
    SENSOR_SCENARIO_HIGH_SPEED_VIDEO    = 0x03,
    SENSOR_SCENARIO_SLIM_VIDEO          = 0x04,
    SENSOR_SCENARIO_MAX                 = 0xFF      // Not a scenario value!
} SENSOR_SCENARIO_T;



/******************************************************************************
* To save code size, set WHICH_SENSOR_MAIN and WHICH_SENSOR_SUB to 0 and 1
* respectively, and remove the switch-case block that converts them to
* SENSOR_IDX_MAIN and SENSOR_IDX_SUB.
******************************************************************************/
typedef enum WHICH_SENSOR
{
    WHICH_SENSOR_NONE       = 0x00,
    WHICH_SENSOR_MAIN       = 0x01,     // Main sensor: rear camera
    WHICH_SENSOR_MAIN_2     = 0x02,     // Not used for now
    WHICH_SENSOR_SUB        = 0x10,     // Sub sensor: front camera
    WHICH_SENSOR_MAX        = 0xFF
} WHICH_SENSOR_T;


typedef struct SENSOR_INFO_IN
{
    //U32 u32SensorId;
    WHICH_SENSOR_T eWhichSensor;    // Main/Sub sensor
    U16 u16FPS;                     // Current FPS used by set_mode()
    SENSOR_SCENARIO_T eScenario;    // Preview, Video, HS Video, and etc.
    U8 *pu8BufferVAddr;             // virtual address of I2C DMA buffer
    U16 u16BufferLen;               // length of I2C DMA buffer
    U32 sensorI2cSlaveAddr;         // slave addr. got from kdsensorlist
} SENSOR_INFO_IN_T;

typedef struct SENSOR_INFO_OUT
{
    U16 u16TransferLen;             // I2C one-shot transfer length
    //U8 u8SlaveAddr;                 // I2C slave address of the image sensor
    U8 u8SupportedByCCU;            // 0: not supported, 1: supported
} SENSOR_INFO_OUT_T;

#endif
