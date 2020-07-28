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

#define LOG_TAG "MtkCam/DualCamBokehPipe"

#include <cutils/properties.h>

#include "DualCamBokehFeaturePipe.h"
#include <mtkcam/feature/featurePipe/IStreamingFeaturePipe.h>
#include <mtkcam/utils/metadata/IMetadataConverter.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>


#define PIPE_CLASS_TAG LOG_TAG
#define PIPE_TRACE TRACE_STREAMING_FEATURE_PIPE

#undef MY_LOGV
#undef MY_LOGD
#undef MY_LOGI
#undef MY_LOGW
#undef MY_LOGE
#undef MY_LOGA
#undef MY_LOGF

#undef MY_LOGV_IF
#undef MY_LOGD_IF
#undef MY_LOGI_IF
#undef MY_LOGW_IF
#undef MY_LOGE_IF
#undef MY_LOGA_IF
#undef MY_LOGF_IF

#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%d]id:%d[%s] " fmt, mRefCount, mSensorId, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%d]id:%d[%s] " fmt, mRefCount, mSensorId, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%d]id:%d[%s] " fmt, mRefCount, mSensorId, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%d]id:%d[%s] " fmt, mRefCount, mSensorId, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%d]id:%d[%s] " fmt, mRefCount, mSensorId, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%d]id:%d[%s] " fmt, mRefCount, mSensorId, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%d]id:%d[%s] " fmt, mRefCount, mSensorId, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#define FUNCTION_IN(id)             CAM_LOGI("[%d]id:%d[%s]+", mRefCount, id, __FUNCTION__)
#define FUNCTION_OUT(id)            CAM_LOGI("[%d]id:%d[%s]-", mRefCount, id, __FUNCTION__)

#define IGNORE_MORE_THAN_ONE_OPENED_ID \
    if(mSyncManager->getOpenedIdCount()>1) {MY_LOGD("ignored");return true;}

using namespace android;

namespace NSCam
{
namespace NSCamFeature
{
namespace NSFeaturePipe
{


/******************************************************************************
 * Initialize
 ******************************************************************************/
MINT32 DualCamBokehFeaturePipe::totalRefCount = 0;
std::weak_ptr<NSDualFeature::DualFeatureEffectHal>
DualCamBokehFeaturePipe::singletonDualFeatureEffectHal;
static Mutex& sDualFeaturePipeCreateLock = *new Mutex();
static Condition sDualFeaturePipeCondition;
static MBOOL gIsMain1Init = MFALSE;
/******************************************************************************
 *
 ******************************************************************************/
DualCamBokehFeaturePipe::
DualCamBokehFeaturePipe(
    MUINT32 sensorIndex,
    const UsageHint& usageHint)
    : DualCamStreamingFeaturePipe() // need using default implement
{
    Mutex::Autolock _l(sDualFeaturePipeCreateLock);

    mRefCount = DualCamBokehFeaturePipe::totalRefCount;

    DualCamBokehFeaturePipe::totalRefCount++;

    FUNCTION_IN(sensorIndex);

    MY_LOGD("new DualCamBokehFeaturePipe(%p): sensor:%d, usage:%d",
            this, sensorIndex, usageHint.mMode);

    mSensorId = sensorIndex;

    // this will be the same single instance, and add new opened id to list
    mSyncManager = SyncManager::createInstance(sensorIndex);
    // prepare setting for dual feature effect hal
    // get sensor index
    MINT32 main1id, main2id;
    if(!StereoSettingProvider::getStereoSensorIndex(main1id, main2id))
    {
        MY_LOGE("get sensor index fail, should not happened.");
        goto lbExit;
    }
    MY_LOGD("main1: %d main2: %d", main1id, main2id);
    if(main1id == mSensorId)
    {
        NSDualFeature::DualFeatureSettings setting;
        setting.miSensorIdx_Main1 = main1id;
        setting.miSensorIdx_Main2 = main2id;
        setting.mMode = (NSCam::v1::Stereo::StereoFeatureMode)StereoSettingProvider::getStereoFeatureMode();
        setting.mSensorType = (NSDualFeature::SeneorModuleType)StereoSettingProvider::getStereoModuleType();
        setting.mszRRZO_Main1 = usageHint.mStreamingSize;
        mDualFeatureEffectHal =
                            std::shared_ptr<NSDualFeature::DualFeatureEffectHal>(
                            new NSDualFeature::DualFeatureEffectHal(setting),
                            [&](NSDualFeature::DualFeatureEffectHal* p)
                            {
                                if(p!=nullptr)
                                {
                                    MY_LOGD("release dual feature pipe");
                                    delete p;
                                    p = nullptr;
                                    gIsMain1Init = MFALSE;
                                }
                            });
        DualCamBokehFeaturePipe::singletonDualFeatureEffectHal = mDualFeatureEffectHal;

        mDualFeatureEffectHal->init();
        mDualFeatureEffectHal->configure();
        mDualFeatureEffectHal->start();
        gIsMain1Init = MTRUE;
        sDualFeaturePipeCondition.signal();
    }
    else
    {
        if(!gIsMain1Init)
        {
            MY_LOGD("wait main1 init +");
            sDualFeaturePipeCondition.wait(sDualFeaturePipeCreateLock);
            MY_LOGD("wait main1 init -");
        }
        mDualFeatureEffectHal =
            DualCamBokehFeaturePipe::singletonDualFeatureEffectHal.lock();
        if(mDualFeatureEffectHal == nullptr)
        {
            MY_LOGE("should not happened!");
            goto lbExit;
        }
    }
lbExit:
    MY_LOGD("[%d]:[%p]", mSensorId, mDualFeatureEffectHal.get());
    FUNCTION_OUT(mSensorId);
}
/******************************************************************************
 *
 ******************************************************************************/
DualCamBokehFeaturePipe::
~DualCamBokehFeaturePipe()
{
    FUNCTION_IN(mSensorId);
    mDualFeatureEffectHal = nullptr;
    mSyncManager = nullptr;

    FUNCTION_OUT(mSensorId);
}
/******************************************************************************
 *
 ******************************************************************************/
void
DualCamBokehFeaturePipe::
onSynced(
    MUINT32 sensorId,
    MUINT32 frameNo,
    FeaturePipeParam& param
)
{
    //Mutex::Autolock _l(mRequsetLock);
    MY_LOGD("frameNo:%d BokehFeaturePipe->enque() +", frameNo);
    // enque
    mDualFeatureEffectHal->updateEffectRequest(param);
    //MY_LOGD("frameNo:%d -", requestNo);
}
/******************************************************************************
 *
 ******************************************************************************/
void
DualCamBokehFeaturePipe::
onCallback(
    MUINT32 sensorId,
    MUINT32 frameNo,
    FeaturePipeParam& param,
    FeaturePipeParam::MSG_TYPE type,
    MBOOL isDrop
)
{
    //Mutex::Autolock _l(mRequsetLock);

    MY_LOGD("frameNo:%d, type:%d, isDrop:%d, DequeSuc:%d +", frameNo, type, isDrop,
            param.mQParams.mDequeSuccess);

    FeaturePipeParam::CALLBACK_T callback =
        param.mVarMap.get<FeaturePipeParam::CALLBACK_T>(SYNC_CALLBACK, NULL);

    if (callback)
    {
        if (isDrop)
        {
            param.mQParams.mDequeSuccess = false;
        }
        else
        {
            int value = property_get_int32("vendor.debug.dualcam.dumpyuv", 0);
            if (value)
            {
                for (MUINT i = 0, n = param.mQParams.mvFrameParams[0].mvOut.size(); i < n; i++)
                {
                    if (param.mQParams.mvFrameParams[0].mvOut[i].mBuffer)
                    {
                        MSize size = param.mQParams.mvFrameParams[0].mvOut[i].mBuffer->getImgSize();
                        char filename[256];
                        snprintf(filename, sizeof(filename),
                                 "/sdcard/dump/out_cam%d_%05d_[%d]_%d+%dx%d_0x%x.raw",
                                 sensorId, frameNo, i,
                        (int)param.mQParams.mvFrameParams[0].mvOut[i].mBuffer->getBufOffsetInBytes(0),
                                 size.w, size.h,
                        param.mQParams.mvFrameParams[0].mvOut[i].mBuffer->getImgFormat());
                        param.mQParams.mvFrameParams[0].mvOut[i].mBuffer->saveToFile(filename);
                        MY_LOGI("write frameNo:%d, %s", frameNo, filename);
                    }
                }
            }
        }
        callback(type, param);
    }
    else
    {
        MY_LOGW("frameNo:%d, no call back function", frameNo);
        exit(1);
    }

    //MY_LOGD("frameNo:%d, type:%d -", requestNo, type);
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
DualCamBokehFeaturePipe::
init(
    const char* name
)
{
    FUNCTION_IN(mSensorId);
    IGNORE_MORE_THAN_ONE_OPENED_ID;

    FUNCTION_OUT(mSensorId);
    return MTRUE;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
DualCamBokehFeaturePipe::
uninit(
    const char* name
)
{
    FUNCTION_IN(mSensorId);

    mSyncManager->flushAndRemoveOpenId(mSensorId);

    MINT count = mSyncManager->getOpenedIdCount();

    if (count > 0)
    {
        MY_LOGD("ignored");
        return true;
    }

    if(mDualFeatureEffectHal != nullptr)
    {
        MBOOL result = mDualFeatureEffectHal->abort();
        result |= mDualFeatureEffectHal->unconfigure();
        result |= mDualFeatureEffectHal->uninit();
    }

    FUNCTION_OUT(mSensorId);
    return MTRUE;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
DualCamBokehFeaturePipe::
enque(
    const FeaturePipeParam& param
)
{
    //FUNCTION_IN(mSensorId);
    MBOOL result = true;

    MUINT32 frameNo = param.mVarMap.get<MUINT32>(VAR_DUALCAM_FRAME_NO, 0);
    MINT64 timestamp = param.mVarMap.get<MINT64>(VAR_DUALCAM_TIMESTAMP, -1000000);
    MINT32 timestamp_ms = timestamp / 1000000;

    MY_LOGW("frameNo:%d, time:%dms", frameNo, timestamp_ms);

    int value = property_get_int32("vendor.debug.dualcam.dumpraw", 0);
    if (value)
    {
        if (param.mQParams.mvFrameParams[0].mvIn.size() > 0
                && param.mQParams.mvFrameParams[0].mvIn[0].mBuffer)
        {
            MSize size = param.mQParams.mvFrameParams[0].mvIn[0].mBuffer->getImgSize();
            char filename[256];
            snprintf(filename, sizeof(filename),
                     "/sdcard/dump/in_cam%d_%05d_%dx%d_%d_%d.raw",
                     mSensorId, frameNo, size.w, size.h,
                     (int)param.mQParams.mvFrameParams[0].mvIn[0].mBuffer->getBufStridesInBytes(0),
                     timestamp_ms);
            param.mQParams.mvFrameParams[0].mvIn[0].mBuffer->saveToFile(filename);
            MY_LOGI("write frameNo:%d, %s", frameNo, filename);
        }
    }

    mSyncManager->queue(mSensorId, frameNo, param, (void*)this);

    //MY_LOGD("frameNo:%d -", frameNo);
    return result;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
DualCamBokehFeaturePipe::
flush()
{
    FUNCTION_IN(mSensorId);

    MBOOL ret = 0; 

    ret = mSyncManager->flushOnly(mSensorId);

    if (ret)
    {
        IGNORE_MORE_THAN_ONE_OPENED_ID;
    }

    if(mDualFeatureEffectHal != nullptr)
    {
        ret = mDualFeatureEffectHal->flush();
    }

    FUNCTION_OUT(mSensorId);
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
DualCamBokehFeaturePipe::
setJpegParam(
    NSCam::NSIoPipe::NSPostProc::EJpgCmd cmd,
    int arg1,
    int arg2
)
{
    FUNCTION_IN(mSensorId);

    IGNORE_MORE_THAN_ONE_OPENED_ID;

    MY_LOGD("no implement");

    FUNCTION_OUT(mSensorId);
    return MTRUE;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
DualCamBokehFeaturePipe::
setFps(
    MINT32 fps
)
{
    FUNCTION_IN(mSensorId);

    MY_LOGD("no implement");

    FUNCTION_OUT(mSensorId);
    return MTRUE;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
DualCamBokehFeaturePipe::
sendCommand(
    NSCam::NSIoPipe::NSPostProc::ESDCmd cmd,
    MINTPTR arg1,
    MINTPTR arg2,
    MINTPTR arg3
)
{
    FUNCTION_IN(mSensorId);

    MY_LOGD("no implement");

    FUNCTION_OUT(mSensorId);
    return MTRUE;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
DualCamBokehFeaturePipe::
addMultiSensorId(
    MUINT32 sensorId
)
{
    FUNCTION_IN(mSensorId);

    MY_LOGD("no implement");

    FUNCTION_OUT(mSensorId);
    return MTRUE;
}
/******************************************************************************
 *
 ******************************************************************************/
MUINT32
DualCamBokehFeaturePipe::
getRegTableSize()
{
    //FUNCTION_IN(mSensorId);

    MY_LOGD("no implement");

    //FUNCTION_OUT(mSensorId);
    return 0;
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DualCamBokehFeaturePipe::
sync()
{
    FUNCTION_IN(mSensorId);

    MY_LOGW("not implement!");

    FUNCTION_OUT(mSensorId);
}
/******************************************************************************
 *
 ******************************************************************************/
IImageBuffer*
DualCamBokehFeaturePipe::
requestBuffer()
{
    FUNCTION_IN(mSensorId);

    MY_LOGD("no implement");

    FUNCTION_OUT(mSensorId);
    return nullptr;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
DualCamBokehFeaturePipe::
returnBuffer(
    IImageBuffer* buffer
)
{
    FUNCTION_IN(mSensorId);

    MY_LOGD("no implement");

    FUNCTION_OUT(mSensorId);
    return MTRUE;
}
/******************************************************************************
 *
 ******************************************************************************/
}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam
