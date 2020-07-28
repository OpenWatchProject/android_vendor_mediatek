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
 *      TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

//! \file FeatureProfileHelper.cpp
#define LOG_TAG "FeatureProfileHelper"
#include <mtkcam3/feature/utils/FeatureProfileHelper.h>
#include <mtkcam/utils/std/ULog.h>

#include <isp_tuning/isp_tuning.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
using namespace NSCam;
using namespace NSIspTuning;


#define FPHELP_LOG(fmt, arg...)    CAM_ULOGMD("[%s]" fmt, __func__, ##arg)
#define FPHELP_INF(fmt, arg...)    CAM_ULOGMI("[%s]" fmt, __func__, ##arg)
#define FPHELP_WRN(fmt, arg...)    CAM_ULOGMW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define FPHELP_ERR(fmt, arg...)    CAM_ULOGME("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#define MY_LOGD_IF(cond, ...)       do { if ( (cond) >= (1) ) { FPHELP_LOG(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) >= (1) ) { FPHELP_INF(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) >= (1) ) { FPHELP_WRN(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) >= (0) ) { FPHELP_ERR(__VA_ARGS__); } }while(0)

CAM_ULOG_DECLARE_MODULE_ID(MOD_P2N_COMMON);

#define PROF_HELP_4K2K_LENG (3840*2160)

#define SET_VHDR_PROFILE(outProf, scen, mode)   \
{                                               \
    switch(mode){                               \
        case MTK_HDR_FEATURE_HDR_HAL_MODE_MVHDR:\
            outProf = EIspProfile_MHDR_##scen;  \
            break;                              \
        default:                                \
            FPHELP_ERR("Unknown vhdr mode(%d), set output prof to (%d)", mode, EIspProfile_Preview); \
            outProf = EIspProfile_Preview;      \
            break;                              \
    }                                           \
}

// IVHDR has no capture profile enum
#define SET_VHDR_PROFILE_CAP(outProf, scen, mode)   \
{                                               \
    switch(mode){                               \
        case MTK_HDR_FEATURE_HDR_HAL_MODE_MVHDR:\
            outProf = EIspProfile_MHDR_##scen;  \
            break;                              \
        default:                                \
            FPHELP_ERR("Unknown vhdr mode(%d), set output prof to (%d)", mode, EIspProfile_Preview); \
            outProf = EIspProfile_Preview;      \
            break;                              \
    }                                           \
}



/**************************************************
 *
 ***************************************************/

MVOID
FeatureProfileHelper::setFeatureProfile(MUINT8& outProfile __unused, ProfileScne scen __unused, MUINT32 hdrHalMode __unused, MBOOL isAuto __unused, MBOOL isEISOn __unused)
{
    FPHELP_ERR("No implemenation!");
}

/**************************************************
 *
 ***************************************************/

MBOOL
FeatureProfileHelper::getStreamingProf(MUINT8& outputProfile, const ProfileParam& param)
{
    MBOOL isProfileSet = MFALSE;
    if(param.hdrHalMode == MTK_HDR_FEATURE_HDR_HAL_MODE_OFF)
    {
        // No VHDR, currently no profile need to be set
        isProfileSet = MFALSE;
    }
    else
    {
        isProfileSet = MTRUE;
        // Do VHDR profile setting
        if ( (param.streamSize.w * param.streamSize.h) < PROF_HELP_4K2K_LENG)
        {
            SET_VHDR_PROFILE(outputProfile, Preview, param.hdrHalMode);
        }
        else
        {
            SET_VHDR_PROFILE(outputProfile, Video, param.hdrHalMode);
        }

        if (param.flag & ProfileParam::FLAG_PURE_RAW_STREAM) // Use Pure raw as P2 input -> capture profile
        {
            SET_VHDR_PROFILE_CAP(outputProfile, Capture, param.hdrHalMode);
        }
    }
    MY_LOGD_IF(isDebugOpen(), "isProfileSet(%d), outputProfile(%d), param.flag(%d)", isProfileSet, outputProfile, param.flag);
    return isProfileSet;
}

/**************************************************
 *
 ***************************************************/

MBOOL
FeatureProfileHelper::getShotProf(MUINT8& outputProfile, const ProfileParam& param)
{
    MBOOL isProfileSet = MFALSE;
    if(param.hdrHalMode == MTK_HDR_FEATURE_HDR_HAL_MODE_OFF)
    {
        // No VHDR, currently no profile need to be set
        isProfileSet = MFALSE;
    }
    else
    {
        // Do VHDR profile setting
        isProfileSet = MTRUE;
        SET_VHDR_PROFILE_CAP(outputProfile, Capture, param.hdrHalMode);
    }
    MY_LOGD_IF(isDebugOpen(), "isProfileSet(%d), outputProfile(%d)", isProfileSet, outputProfile);
    return isProfileSet;
}

