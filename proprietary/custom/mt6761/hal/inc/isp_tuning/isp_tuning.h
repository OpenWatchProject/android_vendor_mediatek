/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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
    MERR_DUMMY,
    MERR_CUSTOM_ISO_ENV_ERR = MERR_DUMMY,
    MERR_CUSTOM_CT_ENV_ERR = MERR_DUMMY
} MERROR_ENUM_T;


/*******************************************************************************
* Operation Mode
*******************************************************************************/
typedef enum
{
    EOperMode_Normal    = 0,
    EOperMode_PureRaw,
    EOperMode_Meta,
    EOperMode_EM
} EOperMode_T;

/*******************************************************************************
* Sensor Mode
*******************************************************************************/
typedef enum
{
    ESensorMode_Preview    = 0,
    ESensorMode_Capture,
    ESensorMode_Video,
    ESensorMode_SlimVideo1,
    ESensorMode_SlimVideo2,
    ESensorMode_Custom1,
    ESensorMode_Custom2,
    ESensorMode_Custom3,
    ESensorMode_Custom4,
    ESensorMode_Custom5,
    ESensorMode_NUM
} ESensorMode_T;

/*******************************************************************************
* PCA Mode
*******************************************************************************/
typedef enum
{
    EPCAMode_180BIN    = 0,
    EPCAMode_360BIN,
    EPCAMode_NUM
} EPCAMode_T;

/*******************************************************************************
*
*******************************************************************************/
typedef enum
{
    // Camera1.0/Camera3.0
    EIspProfile_Preview = 0,          // Preview
    EIspProfile_Video,                // Video
    EIspProfile_Capture,              // Capture
    EIspProfile_ZSD_Capture,          // ZSD Capture
    EIspProfile_VSS_Capture,          // VSS Capture
    // Camera1.0
    EIspProfile_PureRAW_Capture,      // Pure RAW Capture
    // N3D
    EIspProfile_N3D_Preview,          // N3D Preview
    EIspProfile_N3D_Video,            // N3D Video
    EIspProfile_N3D_Capture,          // N3D Capture
    // MFB
    EIspProfile_MFB_Capture_EE_Off,   // MFB capture: EE off
    EIspProfile_MFB_Capture_EE_Off_SWNR, // MFB capture with SW NR: EE off
    EIspProfile_MFB_Blending_All_Off, // MFB blending: all off
    EIspProfile_MFB_Blending_All_Off_SWNR, // MFB blending with SW NR: all off
    EIspProfile_MFB_PostProc_EE_Off,  // MFB post process: capture + EE off
    EIspProfile_MFB_PostProc_ANR_EE,  // MFB post process: capture + ANR + EE
    EIspProfile_MFB_PostProc_ANR_EE_SWNR,  // MFB post process with SW NR: capture + ANR + EE
    EIspProfile_MFB_PostProc_Mixing,  // MFB post process: mixing + all off
    EIspProfile_MFB_PostProc_Mixing_SWNR,  // MFB post process with SW NR: mixing + all off
    // vFB
    EIspProfile_VFB_PostProc,         // VFB post process: all off + ANR + CCR + PCA
    // iHDR
    EIspProfile_IHDR_Preview,         // IHDR preview
    EIspProfile_IHDR_Video,           // IHDR video
    // Multi-pass ANR
    EIspProfile_Capture_MultiPass_ANR_1,     // Capture multi pass ANR 1
    EIspProfile_Capture_MultiPass_ANR_2,     // Capture multi pass ANR 2
    EIspProfile_VSS_Capture_MultiPass_ANR_1, // VSS capture multi Pass ANR 1
    EIspProfile_VSS_Capture_MultiPass_ANR_2, // VSS capture multi Pass ANR 2
    EIspProfile_MFB_MultiPass_ANR_1, // MFB multi Pass ANR 1
    EIspProfile_MFB_MultiPass_ANR_2, // MFB multi Pass ANR 2
    EIspProfile_Capture_SWNR, // Capture with SW NR
    EIspProfile_VSS_Capture_SWNR, // VSS capture with SW NR
    EIspProfile_PureRAW_Capture_SWNR, // Pure RAW capture with SW NR
    // mHDR
    EIspProfile_MHDR_Preview,         // MHDR preview
    EIspProfile_MHDR_Video,           // MHDR video
    EIspProfile_MHDR_Capture, // TODO: reserve dedicated NVRAM
    // VSS_MFB (ZSD+MFLL)
    EIspProfile_VSS_MFB_Capture_EE_Off,   // MFB capture: EE off
    EIspProfile_VSS_MFB_Capture_EE_Off_SWNR, // MFB capture with SW NR: EE off
    EIspProfile_VSS_MFB_Blending_All_Off, // MFB blending: all off
    EIspProfile_VSS_MFB_Blending_All_Off_SWNR, // MFB blending with SW NR: all off
    EIspProfile_VSS_MFB_PostProc_EE_Off,  // MFB post process: capture + EE off
    EIspProfile_VSS_MFB_PostProc_ANR_EE,  // MFB post process: capture + ANR + EE
    EIspProfile_VSS_MFB_PostProc_ANR_EE_SWNR,  // MFB post process with SW NR: capture + ANR + EE
    EIspProfile_VSS_MFB_PostProc_Mixing,  // MFB post process: mixing + all off
    EIspProfile_VSS_MFB_PostProc_Mixing_SWNR,  // MFB post process with SW NR: mixing + all off
    EIspProfile_VSS_MFB_MultiPass_ANR_1, // MFB multi Pass ANR 1
    EIspProfile_VSS_MFB_MultiPass_ANR_2, // MFB multi Pass ANR 2
    EIspProfile_FLASH_Capture,
    //IP_Raw_TPipe (tile mode of pass2 raw path)
    EIspProfile_PureRAW_TPipe_Capture,
    EIspProfile_PureRAW_TPipe_Capture_SWNR,
    //Super night mode
    EIspProfile_3rd_Party_Processed_Raw_Reprocess,
    EIspProfile_NUM,
    EIspProfile_DUMMY,
    EIspProfile_iHDR_Preview = EIspProfile_DUMMY,
    EIspProfile_iHDR_Video = EIspProfile_DUMMY,
    EIspProfile_mHDR_Preview = EIspProfile_DUMMY,
    EIspProfile_mHDR_Video = EIspProfile_DUMMY,
    EIspProfile_mHDR_Capture = EIspProfile_DUMMY,
    EIspProfile_zHDR_Preview = EIspProfile_DUMMY,
    EIspProfile_zHDR_Video = EIspProfile_DUMMY,
    EIspProfile_iHDR_Preview_VSS = EIspProfile_DUMMY,
    EIspProfile_zHDR_Preview_VSS = EIspProfile_DUMMY,  //10
    EIspProfile_mHDR_Preview_VSS = EIspProfile_DUMMY,
    EIspProfile_iHDR_Video_VSS = EIspProfile_DUMMY,
    EIspProfile_zHDR_Video_VSS = EIspProfile_DUMMY,
    EIspProfile_mHDR_Video_VSS = EIspProfile_DUMMY,
    EIspProfile_zHDR_Capture = EIspProfile_DUMMY,
    EIspProfile_Auto_iHDR_Preview = EIspProfile_DUMMY,
    EIspProfile_Auto_zHDR_Preview = EIspProfile_DUMMY,
    EIspProfile_Auto_mHDR_Preview = EIspProfile_DUMMY,
    EIspProfile_Auto_iHDR_Video = EIspProfile_DUMMY,//20
    EIspProfile_Auto_zHDR_Video = EIspProfile_DUMMY,
    EIspProfile_Auto_mHDR_Video = EIspProfile_DUMMY,
    EIspProfile_Auto_iHDR_Preview_VSS = EIspProfile_DUMMY,
    EIspProfile_Auto_zHDR_Preview_VSS = EIspProfile_DUMMY,
    EIspProfile_Auto_mHDR_Preview_VSS = EIspProfile_DUMMY,
    EIspProfile_Auto_iHDR_Video_VSS = EIspProfile_DUMMY,
    EIspProfile_Auto_zHDR_Video_VSS = EIspProfile_DUMMY,
    EIspProfile_Auto_mHDR_Video_VSS = EIspProfile_DUMMY,
    EIspProfile_Auto_zHDR_Capture = EIspProfile_DUMMY,
    EIspProfile_Auto_mHDR_Capture = EIspProfile_DUMMY,  //30
    EIspProfile_MFNR_Before_Blend = EIspProfile_MFB_PostProc_EE_Off,
    EIspProfile_MFNR_Single = EIspProfile_MFB_PostProc_ANR_EE,
    EIspProfile_MFNR_MFB = EIspProfile_MFB_Blending_All_Off,
    EIspProfile_MFNR_After_Blend = EIspProfile_MFB_PostProc_Mixing,
    EIspProfile_zHDR_Capture_MFNR_Before_Blend = EIspProfile_DUMMY,
    EIspProfile_zHDR_Capture_MFNR_Single = EIspProfile_DUMMY,
    EIspProfile_zHDR_Capture_MFNR_MFB = EIspProfile_DUMMY,
    EIspProfile_zHDR_Capture_MFNR_After_Blend = EIspProfile_DUMMY,
    EIspProfile_EIS_Preview = EIspProfile_Preview,
    EIspProfile_EIS_Video = EIspProfile_Video,  //40
    EIspProfile_EIS_iHDR_Preview = EIspProfile_DUMMY,
    EIspProfile_EIS_zHDR_Preview = EIspProfile_DUMMY,
    EIspProfile_EIS_mHDR_Preview = EIspProfile_DUMMY,
    EIspProfile_EIS_iHDR_Video = EIspProfile_DUMMY,
    EIspProfile_EIS_zHDR_Video = EIspProfile_DUMMY,
    EIspProfile_EIS_mHDR_Video = EIspProfile_DUMMY,
    EIspProfile_EIS_Auto_iHDR_Preview = EIspProfile_DUMMY,
    EIspProfile_EIS_Auto_zHDR_Preview = EIspProfile_DUMMY,
    EIspProfile_EIS_Auto_mHDR_Preview = EIspProfile_DUMMY,
    EIspProfile_EIS_Auto_iHDR_Video = EIspProfile_DUMMY,  //50
    EIspProfile_EIS_Auto_zHDR_Video = EIspProfile_DUMMY,
    EIspProfile_EIS_Auto_mHDR_Video = EIspProfile_DUMMY,
    EIspProfile_Capture_MultiPass_HWNR = EIspProfile_DUMMY,
    EIspProfile_YUV_Reprocess = EIspProfile_DUMMY,
    EIspProfile_Preview_Capture_ZOOM1 = EIspProfile_DUMMY,
    EIspProfile_Preview_Capture_ZOOM2 = EIspProfile_DUMMY,
    EIspProfile_Video_Preview_ZOOM1 = EIspProfile_DUMMY,
    EIspProfile_Video_Preview_ZOOM2 = EIspProfile_DUMMY,
    EIspProfile_Video_Video_ZOOM1 = EIspProfile_DUMMY,
    EIspProfile_Video_Video_ZOOM2 = EIspProfile_DUMMY,
    EIspProfile_Capture_Capture_ZOOM1 = EIspProfile_DUMMY,
    EIspProfile_Capture_Capture_ZOOM2 = EIspProfile_DUMMY,
    EIspProfile_MFNR_Before_ZOOM1 = EIspProfile_DUMMY,
    EIspProfile_MFNR_Before_ZOOM2 = EIspProfile_DUMMY,
    EIspProfile_MFNR_Single_ZOOM1 = EIspProfile_DUMMY,
    EIspProfile_MFNR_Single_ZOOM2 = EIspProfile_DUMMY,
    EIspProfile_MFNR_MFB_ZOOM1 = EIspProfile_DUMMY,
    EIspProfile_MFNR_MFB_ZOOM2 = EIspProfile_DUMMY,
    EIspProfile_MFNR_After_ZOOM1 = EIspProfile_DUMMY,
    EIspProfile_MFNR_After_ZOOM2 = EIspProfile_DUMMY,
    EIspProfile_Flash_Capture_ZOOM1 = EIspProfile_DUMMY,
    EIspProfile_Flash_Capture_ZOOM2 = EIspProfile_DUMMY,
    EIspProfile_N3D_Denoise = EIspProfile_DUMMY,
    EIspProfile_N3D_HighRes = EIspProfile_DUMMY,
    EIspProfile_N3D_Preview_toW = EIspProfile_DUMMY,
    EIspProfile_N3D_Video_toW = EIspProfile_DUMMY,
    EIspProfile_N3D_Capture_toW = EIspProfile_DUMMY,
    EIspProfile_N3D_Denoise_toGGM = EIspProfile_DUMMY,
    EIspProfile_N3D_Denoise_toYUV = EIspProfile_DUMMY,
    EIspProfile_N3D_Denoise_toW = EIspProfile_DUMMY,
    EIspProfile_N3D_HighRes_toYUV = EIspProfile_DUMMY,
    EIspProfile_N3D_MFHR_Before_Blend = EIspProfile_DUMMY,
    EIspProfile_N3D_MFHR_Single = EIspProfile_DUMMY,
    EIspProfile_N3D_MFHR_MFB = EIspProfile_DUMMY,
    EIspProfile_N3D_MFHR_After_Blend = EIspProfile_DUMMY,
    EIspProfile_SWHDR_Phase1 = EIspProfile_DUMMY,
    EIspProfile_SWHDR_Phase2 = EIspProfile_DUMMY,
    EIspProfile_MFNR_Before_ZOOM0 = EIspProfile_DUMMY,
    EIspProfile_MFNR_Single_ZOOM0 = EIspProfile_DUMMY,
    EIspProfile_MFNR_MFB_ZOOM0 = EIspProfile_DUMMY,
    EIspProfile_MFNR_After_ZOOM0 = EIspProfile_DUMMY,
    EIspProfile_zHDR_Flash_Capture = EIspProfile_DUMMY
} EIspProfile_T;

/*******************************************************************************
*
*******************************************************************************/
typedef enum
{
    ESensorDev_None         = 0x00,
    ESensorDev_Main         = 0x01,
    ESensorDev_Sub          = 0x02,
	ESensorDev_MainSecond   = 0x04,
    ESensorDev_Main3D       = 0x05,
	ESensorDev_SubSecond    = 0x08,
    ESensorDev_MainThird    = 0x10
}   ESensorDev_T;

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

