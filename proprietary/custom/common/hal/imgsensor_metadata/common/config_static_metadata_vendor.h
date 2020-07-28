/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */


STATIC_METADATA_BEGIN(COMMON, VENDOR, COMMON)

//------------------------------------------------------------------------------
//  vendor: ASD
//------------------------------------------------------------------------------
//==========================================================================
#if MTKCAM_HAVE_ASD_SUPPORT
    CONFIG_METADATA_BEGIN(MTK_FACE_FEATURE_AVAILABLE_ASD_MODES)
        CONFIG_ENTRY_VALUE(MTK_FACE_FEATURE_ASD_MODE_OFF, MINT32)
        CONFIG_ENTRY_VALUE(MTK_FACE_FEATURE_ASD_MODE_SIMPLE, MINT32)
    CONFIG_METADATA_END()
#endif
//==========================================================================

//------------------------------------------------------------------------------
//  vendor: face3A
//------------------------------------------------------------------------------
//==========================================================================
#if MTKCAM_HAVE_FD_SUPPORT
    CONFIG_METADATA_BEGIN(MTK_FACE_FEATURE_AVAILABLE_FORCE_FACE_3A)
        CONFIG_ENTRY_VALUE(0, MINT32)
        CONFIG_ENTRY_VALUE(1, MINT32)
    CONFIG_METADATA_END()
#endif
//==========================================================================

//------------------------------------------------------------------------------
//  vendor: 3DNR
//------------------------------------------------------------------------------
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_NR_FEATURE_AVAILABLE_3DNR_MODES)
        CONFIG_ENTRY_VALUE(MTK_NR_FEATURE_3DNR_MODE_OFF,  MINT32)
#if MTKCAM_HAVE_NR3D_SUPPORT
        CONFIG_ENTRY_VALUE(MTK_NR_FEATURE_3DNR_MODE_ON,  MINT32)
#endif
    CONFIG_METADATA_END()
    //==========================================================================

//------------------------------------------------------------------------------
//  vendor: VHDR
//------------------------------------------------------------------------------
//     //==========================================================================
// #if (1 == MTKCAM_HAVE_VHDR_SUPPORT)
//     CONFIG_METADATA_BEGIN(MTK_HDR_FEATURE_AVAILABLE_VHDR_MODES)
//         CONFIG_ENTRY_VALUE(MTK_HDR_FEATURE_VHDR_MODE_OFF,  MINT32) // MUST Add this mode
//         CONFIG_ENTRY_VALUE(MTK_HDR_FEATURE_VHDR_MODE_MVHDR,  MINT32)
//     CONFIG_METADATA_END()
// #endif
//     //==========================================================================

//------------------------------------------------------------------------------
//  vendor: MFNR
//------------------------------------------------------------------------------
    //==========================================================================
#if MTKCAM_HAVE_MFB_SUPPORT
    CONFIG_METADATA_BEGIN(MTK_MFNR_FEATURE_AVAILABLE_AIS_MODES)
        CONFIG_ENTRY_VALUE(MTK_MFNR_FEATURE_AIS_OFF,  MINT32) // MUST Add this mode
#if (MTKCAM_HAVE_MFB_SUPPORT >= 2)
        CONFIG_ENTRY_VALUE(MTK_MFNR_FEATURE_AIS_ON,  MINT32)
#endif
    CONFIG_METADATA_END()
    //==========================================================================
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_MFNR_FEATURE_AVAILABLE_MFB_MODES)
        CONFIG_ENTRY_VALUE(MTK_MFNR_FEATURE_MFB_OFF,  MINT32) // MUST Add this mode
#if (MTKCAM_HAVE_MFB_SUPPORT == 1)
        CONFIG_ENTRY_VALUE(MTK_MFNR_FEATURE_MFB_MFLL,  MINT32)
        CONFIG_ENTRY_VALUE(MTK_MFNR_FEATURE_MFB_AUTO,  MINT32)
#elif (MTKCAM_HAVE_MFB_SUPPORT == 2)
        CONFIG_ENTRY_VALUE(MTK_MFNR_FEATURE_MFB_AIS,  MINT32)
        CONFIG_ENTRY_VALUE(MTK_MFNR_FEATURE_MFB_AUTO,  MINT32)
#elif (MTKCAM_HAVE_MFB_SUPPORT == 3)
        CONFIG_ENTRY_VALUE(MTK_MFNR_FEATURE_MFB_MFLL,  MINT32)
        CONFIG_ENTRY_VALUE(MTK_MFNR_FEATURE_MFB_AIS,  MINT32)
        CONFIG_ENTRY_VALUE(MTK_MFNR_FEATURE_MFB_AUTO,  MINT32)
#endif
    CONFIG_METADATA_END()
#endif //MTKCAM_HAVE_MFB_SUPPORT

//------------------------------------------------------------------------------
//  vendor: CSHOT
//------------------------------------------------------------------------------
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_CSHOT_FEATURE_AVAILABLE_MODES)
        CONFIG_ENTRY_VALUE(MTK_CSHOT_FEATURE_AVAILABLE_MODE_OFF, MINT32)
        CONFIG_ENTRY_VALUE(MTK_CSHOT_FEATURE_AVAILABLE_MODE_ON, MINT32)
    CONFIG_METADATA_END()
    //==========================================================================

//------------------------------------------------------------------------------
//  vendor: fast s2s
//------------------------------------------------------------------------------
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_CONTROL_CAPTURE_EARLY_NOTIFICATION_SUPPORT)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_CAPTURE_EARLY_NOTIFICATION_SUPPORT_OFF, MINT32)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_CAPTURE_EARLY_NOTIFICATION_SUPPORT_ON, MINT32)
    CONFIG_METADATA_END()
    //==========================================================================

//------------------------------------------------------------------------------
//  vendor: postview
//------------------------------------------------------------------------------
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_CONTROL_CAPTURE_AVAILABLE_POSTVIEW_MODES)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_CAPTURE_POSTVIEW_MODE_OFF, MINT32)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_CAPTURE_POSTVIEW_MODE_ON, MINT32)
    CONFIG_METADATA_END()
    //==========================================================================

//------------------------------------------------------------------------------
//  vendor: bg service
//------------------------------------------------------------------------------
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_BGSERVICE_FEATURE_PRERELEASE_AVAILABLE_MODES)
        CONFIG_ENTRY_VALUE(MTK_BGSERVICE_FEATURE_PRERELEASE_MODE_OFF, MINT32)
#if MTKCAM_HAVE_BGSERVICE_SUPPORT
        CONFIG_ENTRY_VALUE(MTK_BGSERVICE_FEATURE_PRERELEASE_MODE_ON, MINT32)
#endif
    CONFIG_METADATA_END()
    //==========================================================================

//------------------------------------------------------------------------------
//  vendor: flash feature
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//  vendor: flash feature - calibration available
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//  vendor: flash feature - customization available
//------------------------------------------------------------------------------
    if ( rInfo.isBackSide() ) {
        //==========================================================================
        CONFIG_METADATA_BEGIN(MTK_FLASH_FEATURE_CALIBRATION_AVAILABLE)
            CONFIG_ENTRY_VALUE(1, MINT32)
        CONFIG_METADATA_END()
        //==========================================================================
        //==========================================================================
        CONFIG_METADATA_BEGIN(MTK_FLASH_FEATURE_CUSTOMIZATION_AVAILABLE)
            CONFIG_ENTRY_VALUE(0, MUINT8)
        CONFIG_METADATA_END()
        //==========================================================================
    } else {
        //==========================================================================
        CONFIG_METADATA_BEGIN(MTK_FLASH_FEATURE_CALIBRATION_AVAILABLE)
            CONFIG_ENTRY_VALUE(0, MINT32)
        CONFIG_METADATA_END()
        //==========================================================================
        //==========================================================================
        CONFIG_METADATA_BEGIN(MTK_FLASH_FEATURE_CUSTOMIZATION_AVAILABLE)
            CONFIG_ENTRY_VALUE(1, MUINT8)
        CONFIG_METADATA_END()
        //==========================================================================
    }

//------------------------------------------------------------------------------
STATIC_METADATA_END()

