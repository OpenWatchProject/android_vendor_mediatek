/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#include <stddef.h>
#include "ccuif_compat.h"
#include <debug_exif/aaa/dbg_aaa_param.h>
#include "ccu_cmd_param_info.h"
#include "ccu_ext_interface/af_cxu_param.h"

#define PTRDESCS_TO_COUNT(DESCS) ((sizeof(DESCS)/sizeof(struct CcuPtrDescriptor))-1)

#define DESC_GEN(struct_name, field_name, field_type)   \
{                                                       \
    true,                                               \
    offsetof(struct_name, field_name),                  \
    offsetof(COMPAT_##struct_name, field_name),         \
    sizeof(field_type)                                  \
}

#define DESC_END                                        \
{                                                       \
    false,                                              \
    0,                                                  \
    0,                                                  \
    0                                                   \
}

namespace NSCcuIf {

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_SENSOR_INIT
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_SENSOR_INIT_PARAM_DESC =
{
    true, //need_input
    true, //need_output
    offsetof(SENSOR_INFO_IN_T, dma_buf_mva), //input_non_ptr_total_size
    sizeof(SENSOR_INFO_OUT_T), //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_SENSOR_INIT"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_AE_INIT
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_AE_INIT_PARAM_DESC =
{
    true, //need_input
    false, //need_output
    sizeof(CCU_AE_INITI_PARAM_T), //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_AE_INIT"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_AE_ALGO_INIT
//**********************************************************************************
static const struct CcuPtrDescriptor MSG_TO_CCU_AE_ALGO_INIT_PTR_DESCS[] = {
    {
        true,
        offsetof(CCU_AE_ALGO_INITI_PARAM_T, algo_init_param) + offsetof(AE_CORE_INIT, m_LumLog2x1000),
        offsetof(COMPAT_CCU_AE_ALGO_INITI_PARAM_T, algo_init_param) + offsetof(COMPAT_AE_CORE_INIT, m_LumLog2x1000),
        LumLog2x1000_TABLE_SIZE * sizeof(MUINT32)
    },
    DESC_GEN(AE_CORE_INIT, pCurrentTableF, CCU_strFinerEvPline),
    DESC_GEN(AE_CORE_INIT, pCurrentTable, CCU_strEvPline),
    DESC_GEN(AE_CORE_INIT, pCurrentGainList, CCU_strAEPLineGainList),
    {
        true,
        offsetof(CCU_AE_ALGO_INITI_PARAM_T, algo_init_param) + offsetof(AE_CORE_INIT, pEVValueArray),
        offsetof(COMPAT_CCU_AE_ALGO_INITI_PARAM_T, algo_init_param) + offsetof(COMPAT_AE_CORE_INIT, pEVValueArray),
        CCU_LIB3A_AE_EV_COMP_MAX * sizeof(MUINT32)
    },
    DESC_GEN(AE_CORE_INIT, pAETouchMovingRatio, CCU_strAEMovingRatio),
    DESC_GEN(AE_CORE_INIT, pAeNVRAM, CCU_AE_NVRAM_T),
    DESC_GEN(AE_CORE_INIT, prAeSyncNvram, CCU_AESYNC_NVRAM_T),
    DESC_END
};

static const struct CcuCtrlParamDescriptor MSG_TO_CCU_AE_ALGO_INIT_PARAM_DESC =
{
    true, //need_input
    false, //need_output
    offsetof(CCU_AE_ALGO_INITI_PARAM_T, algo_init_param) + offsetof(AE_CORE_INIT, m_LumLog2x1000), //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    &(MSG_TO_CCU_AE_ALGO_INIT_PTR_DESCS[0]), //intput_ptr_descs
    1024 * 100, //buffer size for data of ptr
    PTRDESCS_TO_COUNT(MSG_TO_CCU_AE_ALGO_INIT_PTR_DESCS), //ptr_count
    1, //ring count
    "MSG_TO_CCU_AE_ALGO_INIT"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_AE_START
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_AE_START_PARAM_DESC =
{
    false, //need_input
    false, //need_output
    0, //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_AE_START"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_AE_STOP
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_AE_STOP_PARAM_DESC =
{
    false, //need_input
    false, //need_output
    0, //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_AE_STOP"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_AE_STAT_START
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_AE_STAT_START_PARAM_DESC =
{
    false, //need_input
    false, //need_output
    0, //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_AE_STAT_START"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_AE_STAT_STOP
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_AE_STAT_STOP_PARAM_DESC =
{
    false, //need_input
    false, //need_output
    0, //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_AE_STAT_STOP"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_SWITCH_HIGH_FPS
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_SWITCH_HIGH_FPS_PARAM_DESC =
{
    false, //need_input
    false, //need_output
    0, //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_SWITCH_HIGH_FPS"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_MANUAL_EXP_CTRL
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_MANUAL_EXP_CTRL_PARAM_DESC =
{
    true, //need_input
    true, //need_output
    sizeof(struct ccu_manual_exp_info), //input_non_ptr_total_size
    sizeof(struct ccu_manual_exp_info_output), //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_MANUAL_EXP_CTRL"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_MANUAL_EXP_CTRL_DISABLE
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_MANUAL_EXP_CTRL_DISABLE_PARAM_DESC =
{
    true, //need_input
    false, //need_output
    sizeof(struct ccu_manual_exp_disable_info), //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_MANUAL_EXP_CTRL_DISABLE"
};

//**********************************************************************************

// CcuCtrlParamDescriptor for MSG_TO_CCU_SET_SUPER_NIGHT_MODE
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_SET_SUPER_NIGHT_MODE_PARAM_DESC =
{
    true, //need_input
    false, //need_output
    sizeof(struct ccu_super_night_mode_info), //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_SET_SUPER_NIGHT_MODE"
};

//**********************************************************************************

// CcuCtrlParamDescriptor for MSG_TO_CCU_AE_SET_FD_INFO
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_AE_SET_FD_INFO_PARAM_DESC =
{
    true, //need_input
    false, //need_output
    sizeof(AE_CORE_CTRL_FD_INFO), //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_AE_SET_FD_INFO"
};

// CcuCtrlParamDescriptor for MSG_TO_CCU_AE_BIND_INSTANCE
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_AE_BIND_INSTANCE_PARAM_DESC =
{
    false, //need_input
    true, //need_output
    0, //input_non_ptr_total_size
    sizeof(struct ccu_ae_bind_instance_output), //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_AE_BIND_INSTANCE"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_SET_AP_AE_CTRL_DATA_PERFRAME
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_SET_AP_AE_CTRL_DATA_PERFRAME_PARAM_DESC =
{
    true, //need_input
    false, //need_output
    sizeof(ccu_ae_ctrldata_perframe), //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_SET_AP_AE_CTRL_DATA_PERFRAME"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_SET_AP_AE_CTRL_DATA_ONCHANGE
//**********************************************************************************
static const struct CcuPtrDescriptor MSG_TO_CCU_SET_AP_AE_CTRL_DATA_ONCHANGE_PTR_DESCS[] = {
    DESC_GEN(AE_CORE_CTRL_RUN_TIME_INFO, pCurrentTable, CCU_strEvPline),
    DESC_GEN(AE_CORE_CTRL_RUN_TIME_INFO, pCurrentTableF, CCU_strFinerEvPline),
    DESC_GEN(AE_CORE_CTRL_RUN_TIME_INFO, pAeNVRAM, CCU_AE_NVRAM_T),
    DESC_END
};

static const struct CcuCtrlParamDescriptor MSG_TO_CCU_SET_AP_AE_CTRL_DATA_ONCHANGE_PARAM_DESC =
{
    true, //need_input
    false, //need_output
    offsetof(AE_CORE_CTRL_RUN_TIME_INFO, pCurrentTable), //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    &(MSG_TO_CCU_SET_AP_AE_CTRL_DATA_ONCHANGE_PTR_DESCS[0]), //intput_ptr_descs
    1024 * 100, //buffer size for data of ptr
    PTRDESCS_TO_COUNT(MSG_TO_CCU_SET_AP_AE_CTRL_DATA_ONCHANGE_PTR_DESCS), //ptr_count
    2, //ring count
    "MSG_TO_CCU_SET_AP_AE_CTRL_DATA_ONCHANGE"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_SET_MAX_FRAMERATE
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_SET_MAX_FRAMERATE_PARAM_DESC =
{
    true, //need_input
    false, //need_output
    sizeof(ccu_max_framerate_data), //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_SET_MAX_FRAMERATE"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_GET_AE_DEBUG_INFO
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_GET_AE_DEBUG_INFO_PARAM_DESC =
{
    false, //need_input
    true, //need_output
    0, //input_non_ptr_total_size
    sizeof(AE_DEBUG_INFO_T), //output_non_ptr_total_size
    0, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_GET_AE_DEBUG_INFO"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_HDR_AE_INPUT_READY
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_HDR_AE_INPUT_READY_PARAM_DESC =
{
    true, //need_input
    false, //need_output
    sizeof(struct ccu_hdr_ae_input_data), //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_HDR_AE_INPUT_READY"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_SET_AUTO_FLK
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_SET_AUTO_FLK_PARAM_DESC =
{
    true, //need_input
    false, //need_output
    sizeof(struct ccu_ae_auto_flk_data), //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_SET_AUTO_FLK"
};


//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_SET_3A_SYNC_STATE_PARAM_DESC
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_SET_3A_SYNC_STATE_PARAM_DESC =
{
    true, //need_input
    false, //need_output
    sizeof(struct ccu_3a_sync_state), //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_SET_3A_SYNC_STATE"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_SYNC_AE_SETTING_PARAM_DESC
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_SYNC_AE_SETTING_PARAM_DESC =
{
    true, //need_input
    false, //need_output
    sizeof(struct ccu_sync_ae_settings_data), //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_SYNC_AE_SETTING"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_START_FRAME_SYNC_PARAM_DESC
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_START_FRAME_SYNC_PARAM_DESC =
{
    false, //need_input
    false, //need_output
    0, //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_START_FRAME_SYNC"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_STOP_FRAME_SYNC_PARAM_DESC
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_STOP_FRAME_SYNC_PARAM_DESC =
{
    false, //need_input
    false, //need_output
    0, //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_STOP_FRAME_SYNC"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_START_FRAME_SYNC_R_PARAM_DESC
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_START_FRAME_SYNC_R_PARAM_DESC =
{
    false, //need_input
    false, //need_output
    0, //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_START_FRAME_SYNC_R"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_STOP_FRAME_SYNC_R_PARAM_DESC
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_STOP_FRAME_SYNC_R_PARAM_DESC =
{
    false, //need_input
    false, //need_output
    0, //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_STOP_FRAME_SYNC_R"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_QUERY_TG_SWITCH_STAT
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_QUERY_TG_SWITCH_STAT_PARAM_DESC =
{
    false, //need_input
    true, //need_output
    0, //input_non_ptr_total_size
    sizeof(struct ccu_tg_switch_stat), //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_QUERY_TG_SWITCH_STAT"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_QUERY_FRAME_SYNC_DONE_PARAM_DESC
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_QUERY_FRAME_SYNC_DONE_PARAM_DESC =
{
    false, //need_input
    true, //need_output
    0, //input_non_ptr_total_size
    sizeof(MBOOL), //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_QUERY_FRAME_SYNC_DONE"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_SET_FRAME_SUBSMPL_INFO
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_SET_FRAME_SUBSMPL_INFO_PARAM_DESC =
{
    true, //need_input
    false, //need_output
    sizeof(struct ccu_frame_subsmpl_info), //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_SET_FRAME_SUBSMPL_INFO"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_TG_DYNAMIC_SWITCH
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_TG_DYNAMIC_SWITCH_PARAM_DESC =
{
    false, //need_input
    false, //need_output
    0, //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_TG_DYNAMIC_SWITCH"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_QUERY_FRAME_SYNC_STAT
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_QUERY_FRAME_SYNC_STAT_PARAM_DESC =
{
    false, //need_input
    true, //need_output
    0, //input_non_ptr_total_size
    sizeof(struct ccu_frame_sync_stat), //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_QUERY_FRAME_SYNC_STAT"
};

//**********************************************************************************

// CcuCtrlParamDescriptor for MSG_TO_CCU_SUSPEND_PARAM_DESC
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_SUSPEND_PARAM_DESC =
{
    false, //need_input
    false, //need_output
    0, //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_SUSPEND"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_RESUME_PARAM_DESC
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_RESUME_PARAM_DESC =
{
    false, //need_input
    false, //need_output
    0, //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_RESUME"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_SHUTDOWN
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_SHUTDOWN_PARAM_DESC =
{
    false, //need_input
    false, //need_output
    0, //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_SHUTDOWN"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_UPDATE_TG_SENSOR_MAP
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_UPDATE_TG_SENSOR_MAP_DESC =
{
    true, //need_input
    false, //need_output
    sizeof(struct ccu_tg2sensor_pair), //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_UPDATE_TG_SENSOR_MAP"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_INVALIDATE_TG_SENSOR_MAP
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_INVALIDATE_TG_SENSOR_MAP_DESC =
{
    true, //need_input
    false, //need_output
    sizeof(uint32_t), //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_INVALIDATE_TG_SENSOR_MAP"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_AF_START
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_AF_START_PARAM_DESC =
{
    false, //need_input
    false, //need_output
    0, //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    NULL, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_AF_START"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_AF_INIT
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_AF_INIT_PARAM_DESC =
{
    true, //need_input
    false, //need_output
    offsetof(AFOINIT_INFO_IN_T, lens_info) + offsetof(LENS_INFO_IN_T, dma_buf_mva), //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    NULL, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_AF_INIT"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_AF_ALGO_DONE
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_AF_ALGO_DONE_PARAM_DESC =
{
    false, //need_input
    false, //need_output
    0, //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    NULL, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_AF_ALGO_DONE"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_AF_ACQUIRE_AFO_BUFFER
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_AF_ACQUIRE_AFO_BUFFER_PARAM_DESC =
{
    false, //need_input
    true, //need_output
    0, //input_non_ptr_total_size
    sizeof(AFAcquireBuf_Reg_INFO_OUT_AP), //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_AF_ACQUIRE_AFO_BUFFER"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_AF_SET_HW_REG
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_AF_SET_HW_REG_PARAM_DESC =
{
    false,//true, //need_input
    false, //need_output
    0,//sizeof(CAM_REG_AF_FMT), //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    0,//&(MSG_TO_CCU_AF_SET_HW_REG_PARAM_DESC[0]), //intput_ptr_descs
    NULL, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_AF_SET_HW_REG"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_AF_STOP
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_AF_STOP_PARAM_DESC =
{
    false, //need_input
    false, //need_output
    0, //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_AF_STOP"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_AF_ABORT
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_AF_ABORT_PARAM_DESC =
{
    false, //need_input
    false, //need_output
    0, //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_AF_ABORT"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_AF_CONTROL_INIT
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_AF_CONTROL_INIT_PARAM_DESC =
{
    false, //need_input
    false, //need_output
    0, //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_AF_CONTROL_INIT"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_AF_HW_INTI
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_AF_HW_INTI_PARAM_DESC =
{
    true, //need_input
    true, //need_output
    sizeof(AF_HW_INIT_INPUT_T), //input_non_ptr_total_size
    sizeof(AF_HW_INIT_OUTPUT_T), //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count, //ptr_count
    1, //ring count
    "MSG_TO_CCU_AF_HW_INTI"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_AF_START_ALGO_INIT
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_AF_START_ALGO_INIT_PARAM_DESC =
{
    true, //need_input
    true, //need_output
    sizeof(CCUStartAFInput_T), //input_non_ptr_total_size
    sizeof(StartAFOutput_T), //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_AF_START_ALGO_INIT"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_AF_DOAF
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_AF_DOAF_PARAM_DESC =
{
    true, //need_input
    false, //need_output
    sizeof(DoAFInput_T), //offsetof(DoAFInput_T, afInput) + offsetof(AF_INPUT_T, sStatProfile) + offsetof(AF_STAT_PROFILE_T, ptrStat), //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_AF_DOAF"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_AF_GET_DOAF_OUTPUT
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_AF_GET_DOAF_OUTPUT_PARAM_DESC =
{
    false, //need_input
    true, //need_output
    0, //input_non_ptr_total_size
    sizeof(AF_OUTPUT_T), //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_AF_GET_DOAF_OUTPUT"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_AF_GET_CURPOSTION
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_AF_GET_CURPOSTION_PARAM_DESC =
{
    false, //need_input
    false, //need_output
    0, //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_AF_GET_CURPOSTION"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_AF_GET_PDBLOCK
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_AF_GET_PDBLOCK_PARAM_DESC =
{
    true, //need_input
    true, //need_output
    sizeof(GetPdBlockInput_T), //input_non_ptr_total_size
    sizeof(GetPdBlockOutput_T), //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_AF_GET_PDBLOCK"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_AF_GET_DEBUGINFO
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_AF_GET_DEBUGINFO_PARAM_DESC =
{
    false, //need_input
    true, //need_output
    0, //input_non_ptr_total_size
    sizeof(EXT_AF_DEBUG_INFO_T), //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_AF_GET_DEBUGINFO"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_AF_SYNC
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_AF_SYNC_PARAM_DESC =
{
    true, //need_input
    false, //need_output
    sizeof(CCUAFsync_info_T), //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_AF_SYNC"
};

static const struct CcuCtrlParamDescriptor MSG_TO_CCU_AF_BIND_INSTANCE_PARAM_DESC =
{
    false, //need_input
    true, //need_output
    0, //input_non_ptr_total_size
    sizeof(struct ccu_af_bind_instance_output), //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_AF_BIND_INSTANCE"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_LTM_INIT
//**********************************************************************************
static const struct CcuPtrDescriptor MSG_TO_CCU_LTM_INIT_PTR_DESCS[] = {
    {
        true,
        offsetof(struct ccu_ltm_init_data_in, ltm_nvram_addr),
        offsetof(struct COMPAT_ccu_ltm_init_data_in, ltm_nvram_addr),
        CCU_LTM_NVRAM_SIZE * CCU_LTM_NVRAM_NUM_MAX
    },
    {
        true,
        offsetof(struct ccu_ltm_init_data_in, hlr_nvram_addr),
        offsetof(struct COMPAT_ccu_ltm_init_data_in, hlr_nvram_addr),
        CCU_HLR_NVRAM_SIZE * CCU_HLR_NVRAM_NUM_MAX
    },
    DESC_END
};
//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_LTM_INIT
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_LTM_INIT_PARAM_DESC =
{
    true, //need_input
    false, //need_output
    offsetof(struct ccu_ltm_init_data_in, ltm_nvram_addr), //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    MSG_TO_CCU_LTM_INIT_PTR_DESCS, //intput_ptr_descs
    1024 * 512, //buffer size for data of ptr
    PTRDESCS_TO_COUNT(MSG_TO_CCU_LTM_INIT_PTR_DESCS), //ptr_count
    1, //ring count
    "MSG_TO_CCU_LTM_INIT"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_LTM_PERFRAME_CTRL
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_LTM_PERFRAME_CTRL_PARAM_DESC =
{
    true, //need_input
    false, //need_output
    sizeof(struct ccu_ltm_perframe_data_in), //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_LTM_PERFRAME_CTRL"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_LTM_START
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_LTM_START_PARAM_DESC =
{
    false, //need_input
    false, //need_output
    0, //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_LTM_START"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_LTM_STOP
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_LTM_STOP_PARAM_DESC =
{
    false, //need_input
    false, //need_output
    0, //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_LTM_STOP"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_LTM_STOP
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_LTM_GET_INT_IDX_PARAM_DESC =
{
    false, //need_input
    true, //need_output
    0, //input_non_ptr_total_size
    sizeof(struct ccu_ltm_instance_mapping_handle), //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0, //buffer size for data of ptr
    0, //ptr_count
    1, //ring count
    "MSG_TO_CCU_LTM_GET_INT_IDX_PARAM_DESC"
};

//**********************************************************************************
// List of CcuCtrlParamDescriptor, mapped with uint32_t in ccu_control_extif.h
//**********************************************************************************
static const struct CcuCtrlParamDescriptor *CCU_CTRL_PARAM_DESCRIPTORS_SYSCTRL[DELIMITER_SYSCTRL_MSG_1] =
{
    NULL, //CCU Idle
    &MSG_TO_CCU_SUSPEND_PARAM_DESC,
    &MSG_TO_CCU_RESUME_PARAM_DESC,
    &MSG_TO_CCU_SHUTDOWN_PARAM_DESC,
    &MSG_TO_CCU_UPDATE_TG_SENSOR_MAP_DESC,
    &MSG_TO_CCU_INVALIDATE_TG_SENSOR_MAP_DESC
};

static const struct CcuCtrlParamDescriptor *CCU_CTRL_PARAM_DESCRIPTORS_AE[DELIMITER_AE_MSG_1] =
{
    &MSG_TO_CCU_SENSOR_INIT_PARAM_DESC,
    &MSG_TO_CCU_AE_INIT_PARAM_DESC,
    &MSG_TO_CCU_AE_ALGO_INIT_PARAM_DESC,
    &MSG_TO_CCU_AE_START_PARAM_DESC,
    &MSG_TO_CCU_AE_STOP_PARAM_DESC,
    &MSG_TO_CCU_SET_AP_AE_CTRL_DATA_PERFRAME_PARAM_DESC,
    &MSG_TO_CCU_SET_AP_AE_CTRL_DATA_ONCHANGE_PARAM_DESC,
    &MSG_TO_CCU_SET_MAX_FRAMERATE_PARAM_DESC,
    &MSG_TO_CCU_GET_AE_DEBUG_INFO_PARAM_DESC,
    &MSG_TO_CCU_HDR_AE_INPUT_READY_PARAM_DESC,
    &MSG_TO_CCU_SET_AUTO_FLK_PARAM_DESC,
    &MSG_TO_CCU_AE_STAT_START_PARAM_DESC,
    &MSG_TO_CCU_AE_STAT_STOP_PARAM_DESC,
    &MSG_TO_CCU_SWITCH_HIGH_FPS_PARAM_DESC,
    &MSG_TO_CCU_MANUAL_EXP_CTRL_PARAM_DESC,
    &MSG_TO_CCU_MANUAL_EXP_CTRL_DISABLE_PARAM_DESC,
    &MSG_TO_CCU_SET_SUPER_NIGHT_MODE_PARAM_DESC,
    &MSG_TO_CCU_AE_SET_FD_INFO_PARAM_DESC,
    &MSG_TO_CCU_AE_BIND_INSTANCE_PARAM_DESC
};

static const struct CcuCtrlParamDescriptor *CCU_CTRL_PARAM_DESCRIPTORS_3ACTRL[DELIMITER_3ACTRL_MSG_1] =
{
    &MSG_TO_CCU_SET_3A_SYNC_STATE_PARAM_DESC,
    &MSG_TO_CCU_SYNC_AE_SETTING_PARAM_DESC,
    &MSG_TO_CCU_START_FRAME_SYNC_PARAM_DESC,
    &MSG_TO_CCU_STOP_FRAME_SYNC_PARAM_DESC,
    &MSG_TO_CCU_QUERY_FRAME_SYNC_DONE_PARAM_DESC,
    &MSG_TO_CCU_SET_FRAME_SUBSMPL_INFO_PARAM_DESC,
    &MSG_TO_CCU_TG_DYNAMIC_SWITCH_PARAM_DESC,
    &MSG_TO_CCU_QUERY_FRAME_SYNC_STAT_PARAM_DESC,
    &MSG_TO_CCU_START_FRAME_SYNC_R_PARAM_DESC,
    &MSG_TO_CCU_STOP_FRAME_SYNC_R_PARAM_DESC,
    &MSG_TO_CCU_QUERY_TG_SWITCH_STAT_PARAM_DESC
};

static const CcuCtrlParamDescriptor *CCU_CTRL_PARAM_DESCRIPTORS_AF[DELIMITER_AF_MSG_1] =
{
    &MSG_TO_CCU_AF_INIT_PARAM_DESC,
    &MSG_TO_CCU_AF_START_PARAM_DESC,
    &MSG_TO_CCU_AF_ALGO_DONE_PARAM_DESC,
    &MSG_TO_CCU_AF_ACQUIRE_AFO_BUFFER_PARAM_DESC,
    &MSG_TO_CCU_AF_SET_HW_REG_PARAM_DESC,
    &MSG_TO_CCU_AF_STOP_PARAM_DESC,
    &MSG_TO_CCU_AF_ABORT_PARAM_DESC,
    &MSG_TO_CCU_AF_CONTROL_INIT_PARAM_DESC,
    &MSG_TO_CCU_AF_HW_INTI_PARAM_DESC,
    &MSG_TO_CCU_AF_START_ALGO_INIT_PARAM_DESC,
    &MSG_TO_CCU_AF_DOAF_PARAM_DESC,
    &MSG_TO_CCU_AF_GET_DOAF_OUTPUT_PARAM_DESC,
    &MSG_TO_CCU_AF_GET_CURPOSTION_PARAM_DESC,
    &MSG_TO_CCU_AF_GET_PDBLOCK_PARAM_DESC,
    &MSG_TO_CCU_AF_GET_DEBUGINFO_PARAM_DESC,
    &MSG_TO_CCU_AF_SYNC_PARAM_DESC,
    &MSG_TO_CCU_AF_BIND_INSTANCE_PARAM_DESC
};

static const CcuCtrlParamDescriptor *CCU_CTRL_PARAM_DESCRIPTORS_LTM[DELIMITER_LTM_MSG_1] =
{
    &MSG_TO_CCU_LTM_INIT_PARAM_DESC,
    &MSG_TO_CCU_LTM_PERFRAME_CTRL_PARAM_DESC,
    &MSG_TO_CCU_LTM_START_PARAM_DESC,
    &MSG_TO_CCU_LTM_STOP_PARAM_DESC,
    &MSG_TO_CCU_LTM_GET_INT_IDX_PARAM_DESC
};

static const void *CCU_CTRL_PARAM_DESCRIPTORS_LIST[CCU_FEATURE_MAX] =
{
    NULL, //undefined
    &CCU_CTRL_PARAM_DESCRIPTORS_AE,
    &CCU_CTRL_PARAM_DESCRIPTORS_AF,
    &CCU_CTRL_PARAM_DESCRIPTORS_LTM,
    &CCU_CTRL_PARAM_DESCRIPTORS_3ACTRL,
    &CCU_CTRL_PARAM_DESCRIPTORS_SYSCTRL
};

static const struct CcuCtrlCmdSetInfo CCU_CMD_SET_INFO_AE = {DELIMITER_AE_MSG_MIN, DELIMITER_AE_MSG_1};
static const struct CcuCtrlCmdSetInfo CCU_CMD_SET_INFO_AF = {DELIMITER_AF_MSG_MIN, DELIMITER_AF_MSG_1};
static const struct CcuCtrlCmdSetInfo CCU_CMD_SET_INFO_LTM = {DELIMITER_LTM_MSG_MIN, DELIMITER_LTM_MSG_1};
static const struct CcuCtrlCmdSetInfo CCU_CMD_SET_INFO_3ACTRL = {DELIMITER_3ACTRL_MSG_MIN, DELIMITER_3ACTRL_MSG_1};
static const struct CcuCtrlCmdSetInfo CCU_CMD_SET_INFO_SYSCTRL = {DELIMITER_SYSCTRL_MSG_MIN, DELIMITER_SYSCTRL_MSG_1};
static const struct CcuCtrlCmdSetInfo *CCU_CMD_SET_INFO_LIST[CCU_FEATURE_MAX] =
{
    NULL, //undefined
    &CCU_CMD_SET_INFO_AE,
    &CCU_CMD_SET_INFO_AF,
    &CCU_CMD_SET_INFO_LTM,
    &CCU_CMD_SET_INFO_3ACTRL,
    &CCU_CMD_SET_INFO_SYSCTRL
};

CcuCtrlParamDescriptor *CcuCmdParamInfo::getParamDescriptor(enum ccu_feature_type featureType, MUINT32 msgId)
{
    CcuCtrlParamDescriptor **descriptors = (CcuCtrlParamDescriptor **) CCU_CTRL_PARAM_DESCRIPTORS_LIST[featureType];
    return (CcuCtrlParamDescriptor *)descriptors[msgId];
}

CcuCtrlCmdSetInfo *CcuCmdParamInfo::getCmdSetInfo(enum ccu_feature_type featureType)
{
    return (CcuCtrlCmdSetInfo *) CCU_CMD_SET_INFO_LIST[featureType];
}

}; //namespace NSCcuIf