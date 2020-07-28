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

#define LOG_TAG "MtkCam/AdvCamSettingMgrEIS"

#include <cutils/properties.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/std/TypeTraits.h>
#include <mtkcam/utils/hw/HwTransform.h>

#include "AdvCamSettingMgr_Imp.h"

#include <mtkcam/feature/eis/eis_ext.h>
#include <camera_custom_eis.h>

using namespace NSCam;
using namespace NSCamHW;
using namespace android;

#undef __func__
#define __func__ __FUNCTION__

#define ACSM_LOG(fmt, arg...)    CAM_LOGD("[%s]" fmt, __func__, ##arg)
#define ACSM_INF(fmt, arg...)    CAM_LOGI("[%s]" fmt, __func__, ##arg)
#define ACSM_WRN(fmt, arg...)    CAM_LOGW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define ACSM_ERR(fmt, arg...)    CAM_LOGE("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#define MY_LOGD_IF(cond, ...)       do { if ( (cond) >= (1) ) { ACSM_LOG(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) >= (1) ) { ACSM_INF(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) >= (1) ) { ACSM_WRN(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) >= (1) ) { ACSM_ERR(__VA_ARGS__); } }while(0)

/*******************************************************************************
* EIS Defines
*******************************************************************************/

/*******************************************************************************
* EIS function
*******************************************************************************/
MBOOL isEISOn(const IMetadata* appMetadata)
{
    MUINT8 appEisMode = 0;
    if( IMetadata::getEntry<MUINT8>(appMetadata, MTK_CONTROL_VIDEO_STABILIZATION_MODE, appEisMode) &&
        appEisMode == MTK_CONTROL_VIDEO_STABILIZATION_MODE_ON )
        return MTRUE;
    else
        return MFALSE;
}

MVOID AdvCamSettingMgr_Imp::calEisSettings(
                         const IMetadata*          appMetadata,
                         const AdvCamInputParam&   inputPara,
                               MUINT32             vhdrMode,
                               MBOOL               isTkApp,
                               MBOOL               useAdvP2,
                               MUINT32&            outEisMode,
                               MUINT32&            outEisFactor,
                               MUINT32&            outEisExtraBufNum)
{
    outEisMode = EIS_MODE_OFF;
    outEisFactor = 100;
    outEisExtraBufNum = 0;

    if( ! inputPara.isRecordPipe)
        return;

    MBOOL isDual = MFALSE;  // TODO: set isDual
    MBOOL isVHDR = (vhdrMode != SENSOR_VHDR_MODE_NONE);
    MUINT32 videoCfg = inputPara.is4K2K ? EISCustom::VIDEO_CFG_4K2K : EISCustom::VIDEO_CFG_FHD;
    if( !mEnableTSQ )
    {
        // Always set max buffer
        outEisExtraBufNum = EISCustom::getForwardFrames(videoCfg);
    }

    if( isEISOn(appMetadata) )
    {
        MUINT32 eisMask = EISCustom::USAGE_MASK_NONE;
        MUINT32 videoCfg = EISCustom::VIDEO_CFG_FHD;

        if( isVHDR )
        {
            eisMask |= EISCustom::USAGE_MASK_VHDR;
        }
        if( inputPara.is4K2K )
        {
            eisMask |= EISCustom::USAGE_MASK_4K2K;
            videoCfg = EISCustom::VIDEO_CFG_4K2K;
        }
        if( isDual )
        {
            eisMask |= EISCustom::USAGE_MASK_DUAL_ZOOM;
        }

        if( useAdvP2 &&
            EISCustom::isSupportAdvEIS_HAL3() )
        { // Turnkey App
            outEisMode = EISCustom::getEISMode(eisMask);
            if(! isTkApp)
                outEisMode &= ~(1<<EIS_MODE_EIS_QUEUE); // disable eis queue
            outEisFactor = EIS_MODE_IS_EIS_12_ENABLED(outEisMode) ? EISCustom::getEIS12Factor() : EISCustom::getEISFactor(videoCfg);
        }
        else
        {
            outEisMode = (1<<EIS_MODE_EIS_12);
            outEisFactor = EISCustom::getEIS12Factor();
        }
    }
}

MBOOL AdvCamSettingMgr_Imp::needReconfigByEIS(
                            const IMetadata*          appMetadata,
                            const AdvCamInputParam&   inputPara,
                            const sp<AdvCamSetting>&  curSet)
{
    // 1. Only adv P2 need EIS reconfig
    if(curSet == NULL || ! curSet->useAdvP2)
    {
        return MFALSE;
    }

    // 2. Currently only eis on/off switch need to check reconfig or not
    MINT32 eisOn = isEISOn(appMetadata) ? 1 : 0;
    if(eisOn == mLastInfo.eisOn)
    {
        return MFALSE;
    }
    mLastInfo.eisOn = eisOn;

    // 3. Check eis mode/factor change
    MUINT32 eisMode = EIS_MODE_OFF;
    MUINT32 eisFactor = 100;
    MUINT32 eisExtraBuf = 0;
    calEisSettings(appMetadata, inputPara, curSet->vhdrMode, curSet->isTkApp,
            curSet->useAdvP2, eisMode, eisFactor, eisExtraBuf);
    if(eisMode != curSet->eisMode || eisFactor != curSet->eisFactor)
    {
        ACSM_LOG("Need Reconfig by eis changed. curEis(0x%x/%u), futureEis(0x%x/%u)", curSet->eisMode, curSet->eisFactor,
                    eisMode, eisFactor);
        return MTRUE;
    }
    return MFALSE;
}

MBOOL AdvCamSettingMgr_Imp::updateEISRequestMeta(
                              const IMetadata*          appMetadata,
                              const PipelineParam&      pipelineParam,
                                    IMetadata*          halMetadata)
{
    // 1. Video Size

    if( isAdvEisEnabled(pipelineParam.currentAdvSetting) && isEISOn(appMetadata) )
    {
        IMetadata::setEntry<MSize>(halMetadata, MTK_EIS_VIDEO_SIZE, pipelineParam.mVideoSize);
    }

    // 2. Decide override timestamp mechanism or not
    if( mEnableTSQ )
    {
        MBOOL needOverrideTime = isEisQEnabled(pipelineParam.currentAdvSetting);
        IMetadata::setEntry<MBOOL>(halMetadata, MTK_EIS_NEED_OVERRIDE_TIMESTAMP, needOverrideTime);
        MY_LOGD_IF(mLogLevel,
            "videoSize(%dx%d), needTSQ(%d), eisOn(%d)",
            pipelineParam.mVideoSize.w, pipelineParam.mVideoSize.h,
            needOverrideTime, isEISOn(appMetadata));
    }
    else
    {
        MY_LOGD_IF(mLogLevel,
            "videoSize(%dx%d), eisOn(%d)",
            pipelineParam.mVideoSize.w, pipelineParam.mVideoSize.h,
            isEISOn(appMetadata));
    }
    return MTRUE;
}


MBOOL AdvCamSettingMgr_Imp::isEisEnabled(const android::sp<AdvCamSetting> &curSet)
{
    MBOOL ret = MFALSE;
    if( curSet != NULL )
    {
        ret = ( EIS_MODE_IS_EIS_12_ENABLED(curSet->eisMode) ||
                EIS_MODE_IS_EIS_22_ENABLED(curSet->eisMode) ||
                EIS_MODE_IS_EIS_25_ENABLED(curSet->eisMode) ||
                EIS_MODE_IS_EIS_30_ENABLED(curSet->eisMode) );
    }
    return ret;
}


MBOOL AdvCamSettingMgr_Imp::isAdvEisEnabled(const android::sp<AdvCamSetting> &curSet)
{
    MBOOL ret = MFALSE;
    if( curSet != NULL )
    {
        ret = ( EIS_MODE_IS_EIS_22_ENABLED(curSet->eisMode) ||
                EIS_MODE_IS_EIS_25_ENABLED(curSet->eisMode) ||
                EIS_MODE_IS_EIS_30_ENABLED(curSet->eisMode) );
    }
    return ret;
}

MBOOL AdvCamSettingMgr_Imp::isEisQEnabled(const android::sp<AdvCamSetting> &curSet)
{
    MBOOL ret = MFALSE;
    if( curSet != NULL )
    {
        ret = EIS_MODE_IS_EIS_QUEUE_ENABLED(curSet->eisMode);
    }
    return ret;
}
