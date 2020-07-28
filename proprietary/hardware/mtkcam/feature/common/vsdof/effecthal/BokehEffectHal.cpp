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
 * MediaTek Inc. (C) 2010. All rights reserved.
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
 
#define LOG_TAG "BokehEffectHal"
#define EFFECT_NAME "BokehEffect"
#define MAJOR_VERSION 0
#define MINOR_VERSION 1

#define UNUSED(x) (void)x

#include <cutils/log.h>
#include <mtkcam/utils/std/Log.h>
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#include <mtkcam/feature/effectHalBase/IEffectHal.h>
#include <mtkcam/feature/effectHalBase/EffectHalBase.h>
//
#include <mtkcam/feature/stereo/pipe/IBokehPipe.h>
//
#include <mtkcam/feature/stereo/effecthal/BokehEffectHal.h>
//
#include <mtkcam/feature/stereo/pipe/vsdof_data_define.h>
/******************************************************************************
 *
 ******************************************************************************/
#define FUNCTION_LOG_START
#define FUNCTION_LOG_END
//
using namespace NSCam;
using namespace android;
using namespace NSCam::NSCamFeature::NSFeaturePipe;
//************************************************************************
//
//************************************************************************
BokehEffectHal::
BokehEffectHal()
{
}
//************************************************************************
//
//************************************************************************
BokehEffectHal::
~BokehEffectHal()
{
}
//************************************************************************
//
//************************************************************************
bool
BokehEffectHal::
allParameterConfigured()
{
    FUNCTION_LOG_START;
    FUNCTION_LOG_END;
    return true;
}
//************************************************************************
//
//************************************************************************
status_t
BokehEffectHal::
initImpl()
{
    FUNCTION_LOG_START;
    //
    mvEffectParams = new EffectParameter();
    FUNCTION_LOG_END;
    return OK;
}
//************************************************************************
//
//************************************************************************
status_t
BokehEffectHal::
uninitImpl()
{
    FUNCTION_LOG_START;
    if(mvEffectParams!=nullptr)
        mvEffectParams = nullptr;
    FUNCTION_LOG_END;
    return OK;
}
//************************************************************************
//
//************************************************************************
status_t
BokehEffectHal::
prepareImpl()
{
    FUNCTION_LOG_START;
    //
    if(mpBokehPipe.get())
    {
        MY_LOGE("[%s] already init.", __FUNCTION__);
        return MFALSE;
    }
    //
    if(mvEffectParams.get() == nullptr)
    {
        MY_LOGE("Effect parameter is null");
        return MFALSE;
    }
    //
    // get open id.
    MINT32 openID = mvEffectParams->getInt(BOKEH_EFFECT_REQUEST_OPEN_ID);
    MINT32 runPath = mvEffectParams->getInt(BOKEH_EFFECT_REQUEST_RUN_PATH);
    MINT32 stopMode = mvEffectParams->getInt(VSDOF_FEAME_STOP_MODE);
    mpBokehPipe = IBokehPipe::createInstance(openID, runPath);
    mpBokehPipe->init();
    if(stopMode == VSDOF_STOP_MODE::SYNC)
    {
        mpBokehPipe->setStopMode(MFALSE);
    }
    else
    {
        mpBokehPipe->setStopMode(MTRUE);
    }
    FUNCTION_LOG_END;
    return OK;
}
//************************************************************************
//
//************************************************************************
status_t
BokehEffectHal::
releaseImpl()
{
    FUNCTION_LOG_START;
    if(!mpBokehPipe.get())
    {
        mpBokehPipe->destroyInstance();
    }
    FUNCTION_LOG_END;
    return OK;
}
//************************************************************************
//
//************************************************************************
status_t
BokehEffectHal::
getNameVersionImpl(
    EffectHalVersion &nameVersion) const
{
    FUNCTION_LOG_START;
    nameVersion.effectName = EFFECT_NAME;
    nameVersion.major = MAJOR_VERSION;
    nameVersion.minor = MINOR_VERSION;
    FUNCTION_LOG_END;
    return OK;
}
//************************************************************************
//
//************************************************************************
status_t
BokehEffectHal::
getCaptureRequirementImpl(
    EffectParameter *inputParam,
    Vector<EffectCaptureRequirement> &requirements) const
{
    FUNCTION_LOG_START;
    FUNCTION_LOG_END;
    return OK;
}
//************************************************************************
//
//************************************************************************
status_t
BokehEffectHal::
setParameterImpl(
    String8 &key,
    String8 &object)
{
    FUNCTION_LOG_START;
    UNUSED(key);
    UNUSED(object);
    FUNCTION_LOG_END;
    return OK;
}
//************************************************************************
//
//************************************************************************
status_t
BokehEffectHal::
setParametersImpl(
    sp<EffectParameter> parameter)
{
    FUNCTION_LOG_START;
    mvEffectParams = parameter;
    FUNCTION_LOG_END;
    return OK;
}
//************************************************************************
//
//************************************************************************
status_t
BokehEffectHal::
startImpl(
    uint64_t *uid)
{
    FUNCTION_LOG_START;
    UNUSED(uid);
    FUNCTION_LOG_END;
    return OK;
}
//************************************************************************
//
//************************************************************************
status_t
BokehEffectHal::
abortImpl(
    EffectResult &result,
    EffectParameter const *parameter)
{
    FUNCTION_LOG_START;
    UNUSED(result);
    UNUSED(parameter);
    //
    onSyncRequestFinish();
    //
    FUNCTION_LOG_END;
    return OK;
}
//************************************************************************
//
//************************************************************************
status_t
BokehEffectHal::
updateEffectRequestImpl(
    const EffectRequestPtr request)
{
    FUNCTION_LOG_START;
    if(mpBokehPipe.get() == nullptr)
    {
        MY_LOGE("mpBokehPipe is null.");
        return BAD_VALUE;
    }
    //
    EffectRequestPtr effectRefquest = (EffectRequestPtr) request;
    MY_LOGD("Enque to bokeh effect hal.");
    MBOOL ret = mpBokehPipe->enque(effectRefquest);
    if(ret != MTRUE)
    {
        MY_LOGE("Enque to bokeh effect hal fail.");
        return UNKNOWN_ERROR;
    }
    //
    FUNCTION_LOG_END;
    return OK;
}
//************************************************************************
//
//************************************************************************
MBOOL
BokehEffectHal::
onSyncRequestFinish()
{
    FUNCTION_LOG_START;
    if(mpBokehPipe.get() == nullptr)
    {
        MY_LOGE("mpBokehPipe is null.");
        return MFALSE;
    }
    mpBokehPipe->sync();
    mvEffectParams = nullptr;
    FUNCTION_LOG_END;
    return true;
}
//************************************************************************
//
//************************************************************************
MBOOL
BokehEffectHal::
flush()
{
    if(mpBokehPipe != NULL)
    {
        mpBokehPipe->flush();
        return MTRUE;
    }
    else
    {
        return MFALSE;
    }
}