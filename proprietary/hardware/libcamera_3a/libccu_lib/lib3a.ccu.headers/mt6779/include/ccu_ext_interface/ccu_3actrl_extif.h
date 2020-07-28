#ifndef __CCU_3ACTRL_INTERFACE__
#define __CCU_3ACTRL_INTERFACE__

#include "ccu_ext_interface/ccu_types.h"
#include "algorithm/ccu_n3d_sync2a_tuning_param.h"
#include "algorithm/ccu_MTKSyncAe.h"

/******************************************************************************
* 3A control Command ID definition
******************************************************************************/
enum ccu_msg_id_3actrl
{
    /*To identify ccu control msg count*/
    DELIMITER_3ACTRL_MSG_MIN = 0,

    /*Receive by CCU*/
    MSG_TO_CCU_SET_3A_SYNC_STATE = DELIMITER_3ACTRL_MSG_MIN,
    MSG_TO_CCU_SYNC_AE_SETTING,
    MSG_TO_CCU_START_FRAME_SYNC,
    MSG_TO_CCU_STOP_FRAME_SYNC,
    MSG_TO_CCU_QUERY_FRAME_SYNC_DONE,
    MSG_TO_CCU_SET_FRAME_SUBSMPL_INFO,
    MSG_TO_CCU_TG_DYNAMIC_SWITCH,
    MSG_TO_CCU_QUERY_FRAME_SYNC_STAT,
    MSG_TO_CCU_START_FRAME_SYNC_R,
    MSG_TO_CCU_STOP_FRAME_SYNC_R,
    MSG_TO_CCU_QUERY_TG_SWITCH_STAT,

    /*To identify ccu control msg count*/
    DELIMITER_3ACTRL_MSG_1,
    MSG_CCU_INTERNAL_FRAME_SYNC = DELIMITER_3ACTRL_MSG_1,
    MSG_CCU_INTERNAL_3ACTRL_VSYNC,
    MSG_CCU_INTERNAL_3ACTRL_CQ_DONE,
    
    /*To identify ccu control msg count*/
    DELIMITER_3ACTRL_MSG_MAX
};

/******************************************************************************
* 3A control related interface
******************************************************************************/
enum Ccu3ASyncCtrlMode
{
    SYNC,
    FREE_RUN
};

enum CcuDualCamFeatureType
{
    D_NONE,
    WIDE_TELE,
    BAYER_MONO,
    BAYER_BAYER
};

struct ccu_3a_sync_state
{
    enum Ccu3ASyncCtrlMode mode;
    enum CcuDualCamFeatureType feature;
    MUINT32 master_sensor_dev;
    MUINT32 master_sensor_idx;
    MUINT32 master_sensor_tg;
    CCU_SYNCAE_CAMERA_TYPE_ENUM master_cam_type;
    CCU_SYNCAE_CAMERA_TYPE_ENUM slave_cam_type;
};

struct ccu_sync_ae_settings_data
{
    MUINT32 master_sensor_dev;
    MUINT32 master_sensor_idx;
    MUINT32 master_sensor_tg;
};

enum camsys_work_type
{
    CAM_WORK_TYPE_NONE = 0,
    CAM_WORK_TYPE_DL,
    CAM_WORK_TYPE_DC
};

struct ccu_frame_subsmpl_info
{
    enum camsys_work_type work_type;
    MUINT32 subsmpl_ratio;
};

struct ccu_frame_sync_stat
{
    bool is_synced;
};

struct ccu_tg_switch_stat
{
    bool is_ready;
};

#endif
