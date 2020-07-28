/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

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
#ifndef _ISP_TUNING_H_
#define _ISP_TUNING_H_

#include "isp_tuning_sensor.h"
#include "tuning_mapping/cam_idx_struct_ext.h"

namespace NSIspTuning
{


/*******************************************************************************
*
*******************************************************************************/
typedef enum MERROR_ENUM
{
    MERR_OK         = 0,
    MERR_UNKNOWN    = 0x80000000, // Unknown error
    MERR_UNSUPPORT,
    MERR_BAD_PARAM,
    MERR_BAD_CTRL_CODE,
    MERR_BAD_FORMAT,
    MERR_BAD_ISP_DRV,
    MERR_BAD_NVRAM_DRV,
    MERR_BAD_SENSOR_DRV,
    MERR_BAD_SYSRAM_DRV,
    MERR_SET_ISP_REG,
    MERR_NO_MEM,
    MERR_NO_SYSRAM_MEM,
    MERR_NO_RESOURCE,
    MERR_CUSTOM_DEFAULT_INDEX_NOT_FOUND,
    MERR_CUSTOM_NOT_READY,
    MERR_PREPARE_HW,
    MERR_APPLY_TO_HW,
    MERR_CUSTOM_ISO_ENV_ERR,
    MERR_CUSTOM_CT_ENV_ERR
} MERROR_ENUM_T;

#if !MTK_CAM_NEW_NVRAM_SUPPORT
typedef enum
{
    EIspProfile_Preview = 0,
    EIspProfile_Video,
    EIspProfile_Capture,
    EIspProfile_iHDR_Preview,
    EIspProfile_zHDR_Preview,
    EIspProfile_mHDR_Preview,
    EIspProfile_iHDR_Video,
    EIspProfile_zHDR_Video,
    EIspProfile_mHDR_Video,
    EIspProfile_iHDR_Preview_VSS,
    EIspProfile_zHDR_Preview_VSS,  //10
    EIspProfile_mHDR_Preview_VSS,
    EIspProfile_iHDR_Video_VSS,
    EIspProfile_zHDR_Video_VSS,
    EIspProfile_mHDR_Video_VSS,
    EIspProfile_zHDR_Capture,
    EIspProfile_mHDR_Capture,
    EIspProfile_Auto_iHDR_Preview,
    EIspProfile_Auto_zHDR_Preview,
    EIspProfile_Auto_mHDR_Preview,
    EIspProfile_Auto_iHDR_Video,  //20
    EIspProfile_Auto_zHDR_Video,
    EIspProfile_Auto_mHDR_Video,
    EIspProfile_Auto_iHDR_Preview_VSS,
    EIspProfile_Auto_zHDR_Preview_VSS,
    EIspProfile_Auto_mHDR_Preview_VSS,
    EIspProfile_Auto_iHDR_Video_VSS,
    EIspProfile_Auto_zHDR_Video_VSS,
    EIspProfile_Auto_mHDR_Video_VSS,
    EIspProfile_Auto_zHDR_Capture,
    EIspProfile_Auto_mHDR_Capture,  //30
    EIspProfile_MFNR_Before_Blend,
    EIspProfile_MFNR_Single,
    EIspProfile_MFNR_MFB,
    EIspProfile_MFNR_After_Blend,
    EIspProfile_zHDR_Capture_MFNR_Before_Blend,
    EIspProfile_zHDR_Capture_MFNR_Single,
    EIspProfile_zHDR_Capture_MFNR_MFB,
    EIspProfile_zHDR_Capture_MFNR_After_Blend,
    EIspProfile_EIS_Preview,
    EIspProfile_EIS_Video,  //40
    EIspProfile_EIS_iHDR_Preview,
    EIspProfile_EIS_zHDR_Preview,
    EIspProfile_EIS_mHDR_Preview,
    EIspProfile_EIS_iHDR_Video,
    EIspProfile_EIS_zHDR_Video,
    EIspProfile_EIS_mHDR_Video,
    EIspProfile_EIS_Auto_iHDR_Preview,
    EIspProfile_EIS_Auto_zHDR_Preview,
    EIspProfile_EIS_Auto_mHDR_Preview,
    EIspProfile_EIS_Auto_iHDR_Video,  //50
    EIspProfile_EIS_Auto_zHDR_Video,
    EIspProfile_EIS_Auto_mHDR_Video,
    EIspProfile_Capture_MultiPass_HWNR,
    EIspProfile_YUV_Reprocess,
    EIspProfile_Flash_Capture,
    EIspProfile_Preview_Capture_ZOOM1,
    EIspProfile_Preview_Capture_ZOOM2,
    EIspProfile_Video_Preview_ZOOM1,
    EIspProfile_Video_Preview_ZOOM2,
    EIspProfile_Video_Video_ZOOM1,  //60
    EIspProfile_Video_Video_ZOOM2,
    EIspProfile_Capture_Capture_ZOOM1,
    EIspProfile_Capture_Capture_ZOOM2,
    EIspProfile_MFNR_Before_ZOOM1,
    EIspProfile_MFNR_Before_ZOOM2,
    EIspProfile_MFNR_Single_ZOOM1,
    EIspProfile_MFNR_Single_ZOOM2,
    EIspProfile_MFNR_MFB_ZOOM1,
    EIspProfile_MFNR_MFB_ZOOM2,
    EIspProfile_MFNR_After_ZOOM1,  //70
    EIspProfile_MFNR_After_ZOOM2,
    EIspProfile_Flash_Capture_ZOOM1,
    EIspProfile_Flash_Capture_ZOOM2,
    // N3D
    EIspProfile_N3D_Preview,          // N3D Preview
    EIspProfile_N3D_Video,            // N3D Video
    EIspProfile_N3D_Capture,          // N3D Capture
    //EIspProfile_N3D_Denoise,
    // N3D
    EIspProfile_N3D_Preview_toW,
    EIspProfile_N3D_Video_toW,
    EIspProfile_N3D_Capture_toW,
    EIspProfile_N3D_Capture_Depth, //80
    EIspProfile_N3D_Capture_Depth_toW,
    //EIspProfile_N3D_Denoise_toGGM,
    //EIspProfile_N3D_Denoise_toYUV,
    //EIspProfile_N3D_Denoise_toW,
    //EIspProfile_N3D_MFHR_Before_Blend,
    //EIspProfile_N3D_MFHR_Single,
    //EIspProfile_N3D_MFHR_MFB,
    //EIspProfile_N3D_MFHR_After_Blend,
    // SWHDR
    EIspProfile_SWHDR_Phase1,
    EIspProfile_SWHDR_Phase2,
    EIspProfile_MFNR_Before_ZOOM0,
    EIspProfile_MFNR_Single_ZOOM0,
    EIspProfile_MFNR_MFB_ZOOM0,
    EIspProfile_MFNR_After_ZOOM0,

    EIspProfile_Preview_Preview_ZOOM1,
    EIspProfile_Preview_Preview_ZOOM2,
    EIspProfile_Preview_Video_ZOOM1,     //90
    EIspProfile_Preview_Video_ZOOM2,
    //EIspProfile_zHDR_Flash_Capture,
    //EIspProfile_mHDR_Flash_Capture,
    //EIspProfile_Auto_zHDR_Flash_Capture,
    //EIspProfile_Auto_mHDR_Flash_Capture,
    EIspProfile_SWHDR_R_SE_Before_Blend                ,
    EIspProfile_SWHDR_R_SE_Single                      ,
    EIspProfile_SWHDR_R_SE_MFB                         ,
    EIspProfile_SWHDR_R_SE_After_Blend                 ,
    EIspProfile_SWHDR_R_SE_Capture                     ,
    EIspProfile_SWHDR_R_VLE_Before_Blend               ,
    EIspProfile_SWHDR_R_VLE_Single                     ,
    EIspProfile_SWHDR_R_VLE_MFB                        ,
    EIspProfile_SWHDR_R_VLE_After_Blend                ,//100
    EIspProfile_SWHDR_R_VLE_Capture                    ,
    EIspProfile_SWHDR_R_RLE_Capture                    ,
    EIspProfile_SWHDR_R_SLE_MFB                        ,
    EIspProfile_SWHDR_V_SE_Before_Blend                ,
    EIspProfile_SWHDR_V_SE_Single                      ,
    EIspProfile_SWHDR_V_SE_MFB                         ,
    EIspProfile_SWHDR_V_SE_After_Blend                 ,
    EIspProfile_SWHDR_V_VLE_Before_Blend               ,
    EIspProfile_SWHDR_V_VLE_Single                     ,
    EIspProfile_SWHDR_V_VLE_MFB                        ,//110
    EIspProfile_SWHDR_V_VLE_After_Blend                ,
    EIspProfile_iHDR_Preview_LINE                      ,
    EIspProfile_zHDR_Preview_LINE                      ,
    EIspProfile_mHDR_Preview_LINE                      ,
    EIspProfile_Preview_Capture_Facebook               ,
    EIspProfile_Preview_Video_Facebook                 ,
    EIspProfile_Video_Video_Facebook                   ,
    EIspProfile_iHDR_Video_Messenger                   ,
    EIspProfile_zHDR_Video_Messenger                   ,
    EIspProfile_mHDR_Video_Messenger                   ,//120
    EIspProfile_iHDR_Video_WhatsApp                    ,
    EIspProfile_zHDR_Video_WhatsApp                    ,
    EIspProfile_mHDR_Video_WhatsApp                    ,
    EIspProfile_Capture_Capture_WhatsApp               ,
    EIspProfile_N3D_Video_WhatsApp                     ,
    EIspProfile_N3D_Preview_QQ                         ,
    EIspProfile_Capture_Capture_QQ                     ,
    EIspProfile_Bokeh                                  ,
    EIspProfile_NUM
} EIspProfile_T;
#endif
/*******************************************************************************
* Operation Mode
*******************************************************************************/
typedef enum
{
    EOperMode_Normal    = 0,
    EOperMode_PureRaw,
    EOperMode_Meta,
    EOperMode_EM,
    EOperMpde_Factory
} EOperMode_T;




/*******************************************************************************
* PCA Mode
*******************************************************************************/
typedef enum
{
    EPCAMode_180BIN    = 0,
    EPCAMode_360BIN,
    EPCAMode_NUM
} EPCAMode_T;

typedef enum
{
    ESensorTG_None = 0,
    ESensorTG_1,
    ESensorTG_2,
}   ESensorTG_T;

typedef enum
{
    ERawType_Proc = 0,
    ERawType_Pure = 1
}   ERawType_T;


};  //  NSIspTuning

#endif //  _ISP_TUNING_H_

