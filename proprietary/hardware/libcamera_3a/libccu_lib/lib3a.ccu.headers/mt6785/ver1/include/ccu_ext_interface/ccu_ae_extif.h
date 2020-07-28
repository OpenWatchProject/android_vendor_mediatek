#ifndef __CCU_AE_INTERFACE__
#define __CCU_AE_INTERFACE__

#include "ccu_ext_interface/ccu_types.h"
#include "algorithm/ae_algo_ctrl.h"
#include "algorithm/ccu_n3d_sync2a_tuning_param.h"
#include "algorithm/ccu_MTKSyncAe.h"

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
    int aao_line_width;
    int pso_line_width;
    AE_CORE_CTRL_CPU_EXP_INFO exp_on_start;
} CCU_AE_CONTROL_CFG_T;

typedef struct
{
    CCU_AE_CONTROL_CFG_T control_cfg;
    AE_CORE_INIT algo_init_param;
    CCU_strSyncAEInitInfo *sync_algo_init_param;
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
    //Frame Status
    MBOOL is_zooming;
    MINT32 magic_num;
    //Skip Algo
    MBOOL do_skip; /**< \deprecated No used after CCU v1.1 (full AE) flow */
    MBOOL do_manual; /**< Request CCU to bypass AE algo computation, set manual exposure settings directly*/
    AE_CORE_CTRL_CPU_EXP_INFO manual_exp;
    MBOOL force_reset_ae_status;
    //Algo frame data
    AE_CORE_CTRL_CCU_VSYNC_INFO algo_frame_data;
} ccu_ae_frame_sync_data;

typedef struct
{
    MBOOL auto_flicker_en; /**< Specifies if need to turn on auto anti flicker function*/
    MUINT32 m_u4IndexMax;
    MUINT32 m_u4IndexMin;
    MUINT32 m_u4IndexFMax; /**< Specifies max AE pline index of current scenario*/
    MUINT32 m_u4IndexFMin; /**< Specifies minimum AE pline index of current scenario*/
    MUINT32 m_u4IndexF;
    MUINT32 m_u4Index;
    MINT32 i4MaxBV;
    MINT32 i4MinBV;
    /* OBC version2 */
    // MUINT16 u2LengthV2;
    // MINT16 IDX_PartitionV2[NVRAM_ISP_REGS_ISO_SUPPORT_NUM];
    CCU_ISP_NVRAM_OBC_T OBC_TableV2[NVRAM_ISP_REGS_ISO_SUPPORT_NUM];
    CCU_ISP_NVRAM_OBC_T OBC_Table[4];
    MUINT32 m_u4FinerEVIdxBase;
    MUINT32 m_u4Prvflare;
    MUINT32 u4UpdateLockIndex;
    CCU_strEvPline *pCurrentTable;
    CCU_strFinerEvPline *pCurrentTableF; /**< Specifies AE pline table of current scenario*/
    CCU_AE_NVRAM_T* pAeNVRAM;
} ccu_ae_onchange_data;

typedef struct
{
    MUINT16 framerate;
    kal_bool min_framelength_en;
} ccu_max_framerate_data;

enum Ccu3ASyncCtrlMode
{
    SYNC,
    FREE_RUN
};

enum CcuDualCamFeatureType
{
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

enum ccu_exp_set_stat
{
    CCU_EXP_SETSTAT_NONE,
    CCU_EXP_SETSTAT_SUCCESS,
    CCU_EXP_SETSTAT_MISS,
    CCU_EXP_SETSTAT_EXCEED
};

struct ccu_ae_output
{
    AE_CORE_MAIN_OUT algo_output;
    enum ccu_exp_set_stat exp_set_stat;
    MUINT32 ae_line_cnt;
};

#endif
