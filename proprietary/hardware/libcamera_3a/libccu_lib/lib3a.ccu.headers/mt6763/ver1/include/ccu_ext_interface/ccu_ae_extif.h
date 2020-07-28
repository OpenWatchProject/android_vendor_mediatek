#ifndef __CCU_AE_INTERFACE__
#define __CCU_AE_INTERFACE__

#include "ccu_ext_interface/ccu_types.h"
#include "algorithm/ae_algo_ctrl.h"

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

typedef struct
{
    MBOOL do_skip;
} ccu_skip_algo_data;

typedef struct
{
    MUINT16 framerate;
    kal_bool min_framelength_en;
} ccu_max_framerate_data;

#endif
