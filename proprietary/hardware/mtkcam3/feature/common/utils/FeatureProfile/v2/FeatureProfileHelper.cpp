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

#define _SET_FEATURE_PROFILE_HDR(outProf, scen, vMode)                     \
do{                                                                        \
    if( scen == ProfileScen_Preview ){                                     \
        outProf = EIspProfile_##vMode##HDR_Preview;                        \
    }else if( scen == ProfileScen_Video ){                                 \
        outProf = EIspProfile_##vMode##HDR_Video;                          \
    }else if( scen == ProfileScen_Capture ){                               \
        outProf = EIspProfile_##vMode##HDR_Capture;                        \
    }else{                                                                 \
        outProf = EIspProfile_##vMode##HDR_Preview;                        \
    }                                                                      \
}while(0)                                                                  \

#define _SET_FEATURE_PROFILE_HDR_AUTO(outProf, scen, vMode, isAuto)        \
do{                                                                        \
    if( isAuto ){                                                          \
        if( scen == ProfileScen_Preview ){                                 \
            outProf = EIspProfile_Auto_##vMode##HDR_Preview;               \
        }else if( scen == ProfileScen_Video ){                             \
            outProf = EIspProfile_Auto_##vMode##HDR_Video;                 \
        }else if( scen == ProfileScen_Capture ){                           \
            outProf = EIspProfile_Auto_##vMode##HDR_Capture;               \
        }else{                                                             \
            outProf = EIspProfile_Auto_##vMode##HDR_Preview;               \
        }                                                                  \
    }else{                                                                 \
        _SET_FEATURE_PROFILE_HDR(outProf, scen, vMode);                    \
    }                                                                      \
}while(0)                                                                  \

#define _SET_FEATURE_PROFILE_HDR_PREVIEW(outProf, scen, vMode)             \
do{                                                                        \
    if( scen == ProfileScen_Preview ){                                     \
        outProf = EIspProfile_##vMode##HDR_Preview;                        \
    }else if( scen == ProfileScen_Video ){                                 \
        outProf = EIspProfile_##vMode##HDR_Video;                          \
    }else{                                                                 \
        outProf = EIspProfile_##vMode##HDR_Preview;                        \
    }                                                                      \
}while(0)                                                                  \


/**************************************************
 *
 ***************************************************/

MVOID
FeatureProfileHelper::setFeatureProfile(MUINT8& outProfile, ProfileScne scen, MUINT32 hdrHalMode, MBOOL isAuto, MBOOL isEISOn)
{
    if ( hdrHalMode == MTK_HDR_FEATURE_HDR_HAL_MODE_MVHDR ) {
        _SET_FEATURE_PROFILE_HDR_AUTO(outProfile, scen, m, isAuto);
    }
#ifdef SUPPORT_MSTREAM
    else if ( hdrHalMode & MTK_HDR_FEATURE_HDR_HAL_MODE_MSTREAM_PREVIEW ) {
        _SET_FEATURE_PROFILE_HDR_PREVIEW(outProfile, scen, ms2);
    }
#endif
    else if ( isEISOn ){
        if ( scen == ProfileScen_Preview ) {
                outProfile = EIspProfile_EIS_Preview;
        }
        else if ( scen == ProfileScen_Video) {
                outProfile = EIspProfile_EIS_Video;
        }
        else {
                FPHELP_ERR("Unknown profile mode(0x%x), set output profile to (%d)", hdrHalMode, EIspProfile_Preview);
                outProfile = EIspProfile_EIS_Preview;
        }
    }
    else {
        FPHELP_ERR("Unknown profile mode(0x%x), set output profile to (%d)", hdrHalMode, EIspProfile_Preview);
        outProfile = EIspProfile_Preview;
    }
}

/**************************************************
 *
 ***************************************************/

MBOOL
FeatureProfileHelper::getStreamingProf(MUINT8& outputProfile, const ProfileParam& param)
{
    MBOOL isProfileSet = MFALSE;
    MBOOL isAutoHDR = (param.featureMask & ProfileParam::FMASK_AUTO_HDR_ON);
    MBOOL isEISOn   = (param.featureMask & ProfileParam::FMASK_EIS_ON);

    if(param.hdrHalMode == MTK_HDR_FEATURE_HDR_HAL_MODE_OFF && !isEISOn)
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
            setFeatureProfile(outputProfile, ProfileScen_Preview, param.hdrHalMode, isAutoHDR, isEISOn);
        }
        else
        {
            setFeatureProfile(outputProfile, ProfileScen_Video, param.hdrHalMode, isAutoHDR, isEISOn);
        }

        if (param.flag & ProfileParam::FLAG_PURE_RAW_STREAM) // Use Pure raw as P2 input -> capture profile
        {
            setFeatureProfile(outputProfile, ProfileScen_Capture, param.hdrHalMode, isAutoHDR, MFALSE);
        }
    }
    MY_LOGD_IF(isDebugOpen(), "isProfileSet(%d), outputProfile(%d), isAuto(%d), isEISOn(%d),  param.flag(%d)", isProfileSet, outputProfile, isAutoHDR, isEISOn, param.flag);
    return isProfileSet;
}

/**************************************************
 *
 ***************************************************/

MBOOL
FeatureProfileHelper::getShotProf(MUINT8& outputProfile, const ProfileParam& param)
{
    MBOOL isProfileSet = MFALSE;
    MBOOL isAutoHDR = (param.featureMask & ProfileParam::FMASK_AUTO_HDR_ON);

    if(param.hdrHalMode == MTK_HDR_FEATURE_HDR_HAL_MODE_OFF )
    {
        // No VHDR, currently no profile need to be set
        isProfileSet = MFALSE;
    }
    else
    {
        // Do VHDR profile setting
        isProfileSet = MTRUE;

        switch(param.sensorMode){
            case ESensorMode_Preview: // Should the same as VSS 1080p
            {
                setFeatureProfile(outputProfile, ProfileScen_Preview_VSS, param.hdrHalMode, isAutoHDR, MFALSE);
                break;
            }
            case ESensorMode_Video: // Should the same as VSS 4k2k
            {
                setFeatureProfile(outputProfile, ProfileScen_Video_VSS, param.hdrHalMode, isAutoHDR, MFALSE);
                break;
            }
            case ESensorMode_Capture: // Should the same as VHDR Shot
            {
                setFeatureProfile(outputProfile, ProfileScen_Capture, param.hdrHalMode, isAutoHDR, MFALSE);
                break;
            }
            default:
            {
                FPHELP_WRN("SensorMode(%d) can not be recognized!", param.sensorMode);
                isProfileSet = MFALSE;
                break;
            }
        }
    }
    MY_LOGD_IF(isDebugOpen(), "isProfileSet(%d), outputProfile(%d), isAuto(%d), isEISOn(0)", isProfileSet, outputProfile, isAutoHDR);
    return isProfileSet;
}

