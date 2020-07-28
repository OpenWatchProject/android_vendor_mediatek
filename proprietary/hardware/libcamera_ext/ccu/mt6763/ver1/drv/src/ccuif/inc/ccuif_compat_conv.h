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
#ifndef _CCUIF_COMPAT_CONV_H_
#define _CCUIF_COMPAT_CONV_H_

#include <stddef.h>
#include "ccuif_compat.h"

/*******************************************************************************
* ARM linux kernel 64 -> CCU 32 compatible
* Make all pointers as 32bit MVA,
* avoiding pointer type compatible problem in kernel driver
********************************************************************************/
namespace NSCcuIf {

#define CCU_EXT_MSG_COUNT (DELIMITER_CCU_CONTROL_CNT_1)

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

//**********************************************************************************
// CcuPtrDescriptor: describes pointed data info. of ptr member in original struct
//**********************************************************************************
struct CcuPtrDescriptor
{
    const bool valid; //true: valid entry, false: means end of ptr description list
    const size_t offset_in_orig_struct; //ptr. member byte size offset in original struct
    const size_t offset_in_compat_struct; //ptr. member byte size offset in compat. struct
    const size_t size; //data chunk size of datas to which ptr member points
};

//**********************************************************************************
// CcuCtrlParamDescriptor: describes ccu_io_control param info.
//**********************************************************************************
struct CcuCtrlParamDescriptor
{
    const bool need_input; //if need input data
    const bool need_output; //if need output data
    const size_t input_non_ptr_total_size; //input data size of non-ptr member part
    const size_t output_non_ptr_total_size; //output data size, have only non-ptr member
    const struct CcuPtrDescriptor *intput_ptr_descs; //list of CcuPtrDescriptor to describe info needed to handle ptr members
    const size_t buffer_size; //buffer size need for content of ptr member
    const char *msg_literal;
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_SENSOR_INIT
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_SENSOR_INIT_PARAM_DESC =
{
    true, //need_input
    true, //need_output
    offsetof(SENSOR_INFO_IN_T, pu8BufferVAddr), //input_non_ptr_total_size
    sizeof(SENSOR_INFO_OUT_T), //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0,
    "MSG_TO_CCU_SENSOR_INIT"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_AE_INIT
//**********************************************************************************
static const struct CcuPtrDescriptor MSG_TO_CCU_AE_INIT_PTR_DESCS[] = {
    {
        true,
        offsetof(CCU_AE_INITI_PARAM_T, algo_init_param) + offsetof(AE_CORE_INIT, pCurrentTable),
        offsetof(COMPAT_CCU_AE_INITI_PARAM_T, algo_init_param) + offsetof(COMPAT_AE_CORE_INIT, pCurrentTable),
        sizeof(strEvPline)
    },
    {
        true,
        offsetof(CCU_AE_INITI_PARAM_T, algo_init_param) + offsetof(AE_CORE_INIT, pTgEVCompJumpRatioOverExp),
        offsetof(COMPAT_CCU_AE_INITI_PARAM_T, algo_init_param) + offsetof(COMPAT_AE_CORE_INIT, pTgEVCompJumpRatioOverExp),
        LIB3A_AE_EV_COMP_MAX * sizeof(MINT32)
    },
    {
        true,
        offsetof(CCU_AE_INITI_PARAM_T, algo_init_param) + offsetof(AE_CORE_INIT, pTgEVCompJumpRatioUnderExp),
        offsetof(COMPAT_CCU_AE_INITI_PARAM_T, algo_init_param) + offsetof(COMPAT_AE_CORE_INIT, pTgEVCompJumpRatioUnderExp),
        LIB3A_AE_EV_COMP_MAX * sizeof(MINT32)
    },
    {
        true,
        offsetof(CCU_AE_INITI_PARAM_T, algo_init_param) + offsetof(AE_CORE_INIT, LumLog2x1000),
        offsetof(COMPAT_CCU_AE_INITI_PARAM_T, algo_init_param) + offsetof(COMPAT_AE_CORE_INIT, LumLog2x1000),
        LumLog2x1000_TABLE_SIZE * sizeof(MUINT32)
    },
    DESC_END
};

static const struct CcuCtrlParamDescriptor MSG_TO_CCU_AE_INIT_PARAM_DESC =
{
    true, //need_input
    false, //need_output
    offsetof(CCU_AE_INITI_PARAM_T, algo_init_param) + offsetof(AE_CORE_INIT, pCurrentTable), //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    &(MSG_TO_CCU_AE_INIT_PTR_DESCS[0]), //intput_ptr_descs
    1024 * 50,
    "MSG_TO_CCU_AE_INIT"
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
    0,
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
    0,
    "MSG_TO_CCU_AE_STOP"
};

// CcuCtrlParamDescriptor for MSG_TO_CCU_SET_SKIP_ALGO
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_SET_SKIP_ALGO_PARAM_DESC =
{
    true, //need_input
    false, //need_output
    sizeof(ccu_skip_algo_data), //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    NULL, //buffer for data of ptr
    "MSG_TO_CCU_SET_SKIP_ALGO"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_SET_AP_AE_FRAME_SYNC_DATA
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_SET_AP_AE_FRAME_SYNC_DATA_PARAM_DESC =
{
    true, //need_input
    false, //need_output
    sizeof(ccu_ae_frame_sync_data), //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0,
    "MSG_TO_CCU_SET_AP_AE_FRAME_SYNC_DATA"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_SET_AP_AE_ONCHANGE_DATA
//**********************************************************************************
static const struct CcuPtrDescriptor MSG_TO_CCU_SET_AP_AE_ONCHANGE_DATA_PTR_DESCS[] = {
    DESC_GEN(ccu_ae_onchange_data, pCurrentTable, strEvPline),
    DESC_END
};

static const struct CcuCtrlParamDescriptor MSG_TO_CCU_SET_AP_AE_ONCHANGE_DATA_PARAM_DESC =
{
    true, //need_input
    false, //need_output
    offsetof(ccu_ae_onchange_data, pCurrentTable), //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    &(MSG_TO_CCU_SET_AP_AE_ONCHANGE_DATA_PTR_DESCS[0]), //intput_ptr_descs
    1024 * 50,
    "MSG_TO_CCU_SET_AP_AE_ONCHANGE_DATA"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_GET_CCU_OUTPUT
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_GET_CCU_OUTPUT_PARAM_DESC =
{
    false, //need_input
    true, //need_output
    0, //input_non_ptr_total_size
    sizeof(AE_CORE_MAIN_OUT), //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0,
    "MSG_TO_CCU_GET_CCU_OUTPUT"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_SET_AP_AE_OUTPUT
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_SET_AP_AE_OUTPUT_PARAM_DESC =
{
    true, //need_input
    false, //need_output
    sizeof(AE_CORE_CTRL_CPU_EXP_INFO), //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0,
    "MSG_TO_CCU_SET_AP_AE_OUTPUT"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_SET_AE_ROI
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_SET_AE_ROI_PARAM_DESC =
{
    true, //need_input
    false, //need_output
    sizeof(ccu_ae_roi), //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0,
    "MSG_TO_CCU_SET_AE_ROI"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_SET_AE_EV
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_SET_AE_EV_PARAM_DESC =
{
    true, //need_input
    false, //need_output
    sizeof(LIB3A_AE_EVCOMP_T), //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    0,
    "MSG_TO_CCU_SET_AE_EV"
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
    0,
    "MSG_TO_CCU_SET_MAX_FRAMERATE"
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
    0,
    "MSG_TO_CCU_SHUTDOWN"
};
//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_AF_START
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_AF_START_PARAM_DESC =
{
    false/*true*/, //need_input
    false, //need_output
    0/*sizeof(unsigned int)*/, //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    NULL, //buffer for data of ptr
    "MSG_TO_CCU_AF_START"
};
//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_AF_INIT
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_AF_INIT_PARAM_DESC =
{
    true, //need_input
    true, //need_output
    sizeof(AFOINIT_INFO_IN_T), //input_non_ptr_total_size
    //offsetof(AFOINIT_INFO_IN_T, AFOBufsAddr[3]) + offsetof(AFOINIT_INFO_IN_T, AFORegAddr[3]), //input_non_ptr_total_size
    sizeof(AFOINIT_INFO_OUT_T), //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    NULL,
    "MSG_TO_CCU_AF_INIT"
};

//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_AF_ALGO_DONE
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_AF_ALGO_DONE_PARAM_DESC =
{
    true, //need_input
    false, //need_output
    sizeof(AFSetHWReg_INFO_IN_T), //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    NULL, //intput_ptr_descs
    NULL, //buffer for data of ptr
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
    0, //buffer for data of ptr
    "MSG_TO_CCU_AF_ACQUIRE_AFO_BUFFER"
};
//**********************************************************************************
// CcuCtrlParamDescriptor for MSG_TO_CCU_AF_SET_HW_REG
//**********************************************************************************
static const struct CcuCtrlParamDescriptor MSG_TO_CCU_AF_SET_HW_REG_PARAM_DESC =
{
    true, //need_input
    false, //need_output
    sizeof(CAM_REG_AF_T), //input_non_ptr_total_size
    0, //output_non_ptr_total_size
    0,//&(MSG_TO_CCU_AF_SET_HW_REG_PARAM_DESC[0]), //intput_ptr_descs
    NULL, //buffer for data of ptr
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
    0,
    "MSG_TO_CCU_AF_STOP"
};

//**********************************************************************************
// List of CcuCtrlParamDescriptor, mapped with ccu_msg_id in ccu_control_extif.h
//**********************************************************************************
static const struct CcuCtrlParamDescriptor *CCU_CTRL_PARAM_DESCRIPTOR_LIST[CCU_EXT_MSG_COUNT] =
{
    NULL, //CCU Idle
    &MSG_TO_CCU_SENSOR_INIT_PARAM_DESC,
    &MSG_TO_CCU_AE_INIT_PARAM_DESC,
    &MSG_TO_CCU_AE_START_PARAM_DESC,
    &MSG_TO_CCU_AE_STOP_PARAM_DESC,
    &MSG_TO_CCU_SET_SKIP_ALGO_PARAM_DESC,
    &MSG_TO_CCU_SET_AP_AE_FRAME_SYNC_DATA_PARAM_DESC,
    &MSG_TO_CCU_SET_AP_AE_ONCHANGE_DATA_PARAM_DESC,
    &MSG_TO_CCU_GET_CCU_OUTPUT_PARAM_DESC,
    &MSG_TO_CCU_SET_AP_AE_OUTPUT_PARAM_DESC,
    &MSG_TO_CCU_SET_AE_ROI_PARAM_DESC,
    &MSG_TO_CCU_SET_AE_EV_PARAM_DESC,
    &MSG_TO_CCU_SET_MAX_FRAMERATE_PARAM_DESC,
    &MSG_TO_CCU_SHUTDOWN_PARAM_DESC,
    &MSG_TO_CCU_AF_INIT_PARAM_DESC,
    &MSG_TO_CCU_AF_START_PARAM_DESC,
    &MSG_TO_CCU_AF_ALGO_DONE_PARAM_DESC,
    &MSG_TO_CCU_AF_ACQUIRE_AFO_BUFFER_PARAM_DESC,
    &MSG_TO_CCU_AF_SET_HW_REG_PARAM_DESC,
    &MSG_TO_CCU_AF_STOP_PARAM_DESC,
};

};  //namespace NSCcuIf
#endif  //  _CCUIF_H_

