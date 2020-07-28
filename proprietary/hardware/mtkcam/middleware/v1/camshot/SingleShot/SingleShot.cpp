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
#define LOG_TAG "MtkCam/SShot"
//
#include <mtkcam/utils/std/Log.h>
#include <sys/stat.h>
#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __func__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __func__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __func__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __func__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __func__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __func__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __func__, ##arg)
#define FUNC_START                  MY_LOGD("+")
#define FUNC_END                    MY_LOGD("-")
//
//
#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/Trace.h>
using namespace android;
//
#include <mtkcam/middleware/v1/camshot/_callbacks.h>
#include <mtkcam/middleware/v1/camshot/_params.h>
//
#include <mtkcam/utils/metadata/IMetadata.h>
using namespace NSCam;

#include <mtkcam/middleware/v1/LegacyPipeline/ILegacyPipeline.h>
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProviderFactory.h>
#include <mtkcam/middleware/v1/LegacyPipeline/LegacyPipelineBuilder.h>
using namespace NSCam::v1::NSLegacyPipeline;

#include "../inc/CamShotImp.h"
#include "../inc/SingleShot.h"
#include <mtkcam/middleware/v1/camshot/CamShotUtils.h>
//
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/aaa/IIspMgr.h>
//
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metadata/IMetadataConverter.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/IResourceContainer.h>
#include <mtkcam/middleware/v1/LegacyPipeline/NodeId.h>
//
#include <mtkcam/utils/hw/HwInfoHelper.h>
using namespace NSCamHW;
//
#include <mtkcam/middleware/v1/camshot/BufferCallbackHandler.h>
using namespace NSCam::v1;
//
#include <mtkcam/utils/hw/CamManager.h>
using namespace NSCam::Utils;
//
#include <mtkcam/middleware/v1/camutils/IDetachJobManager.h>
#include <mtkcam/middleware/v1/camutils/PostProcessJobQueue.h>

#include <cutils/properties.h>
#define DUMP_KEY  "vendor.debug.singleshot.dump"
#define DUMP_PATH "/sdcard/singleshot"

#define CHECK_OBJECT(x)  do{                                        \
    if (x == nullptr) { MY_LOGE("Null %s Object", #x); return MFALSE;} \
} while(0)

/*******************************************************************************
*
********************************************************************************/
namespace NSCamShot {

/*******************************************************************************
*
********************************************************************************/
class ImageCallback
    : public IImageCallback
{
public:
                                        ImageCallback(
                                            CallbackListener* pListener,
                                            MUINT32 const data
                                            )
                                            : mpListener(pListener)
                                            , mData(data)
                                        {}
public:   ////    interface of IImageCallback
    /**
     *
     * Received result buffer.
     *
     * @param[in] RequestNo : request number.
     *
     * @param[in] pBuffer : IImageBuffer.
     *
     */
    virtual MERROR                      onResultReceived(
                                            MUINT32 const              RequestNo,
                                            StreamId_T const           streamId,
                                            MBOOL   const           /*errorBuffer*/,
                                            android::sp<IImageBuffer>& pBuffer
                                        ) {
                                            if( mpListener )
                                                mpListener->onDataReceived(
                                                    RequestNo, streamId, pBuffer
                                                    );
                                            return OK;
                                        }
protected:
    CallbackListener*                   mpListener;
    MUINT32 const                       mData;
};


/*******************************************************************************
*
********************************************************************************/
class MetadataListener
    : public ResultProcessor::IListener
{
public:
                                        MetadataListener(
                                            CallbackListener* pListener
                                            )
                                            : mpListener(pListener)
                                        {}
public:   ////    interface of IListener
    virtual void                        onResultReceived(
                                            MUINT32         const requestNo,
                                            StreamId_T      const streamId,
                                            MBOOL           const errorResult,
                                            IMetadata       const result
                                        ) {
                                            if( mpListener )
                                                mpListener->onMetaReceived(
                                                    requestNo, streamId, errorResult, result
                                                    );
                                        }

    virtual void                       onFrameEnd(
                                            MUINT32         const /*requestNo*/
                                        ) {};

    virtual String8                     getUserName() { return String8(LOG_TAG); }

protected:
    CallbackListener*                   mpListener;
};


/*******************************************************************************
*
********************************************************************************/
class ResourceHolder
    : public virtual android::RefBase
{
public:
    MVOID                       add( sp<RefBase> pResource ) {
                                    mvResources.push_back(pResource);
                                }
    MVOID                       clear() {
                                    mvResources.clear();
                                }
protected:
    Vector< sp<RefBase> >       mvResources;
};

/*******************************************************************************
*
********************************************************************************/
SingleShot::
SingleShot(
    EShotMode const eShotMode,
    char const*const szCamShotName,
    EPipelineMode const ePipelineMode
)
    : CamShotImp(eShotMode, szCamShotName, ePipelineMode)
    , mpResource( new ResourceHolder() )
    , mbCBShutter(MTRUE)
    , mbP2Done(MTRUE)
    , mbRequestSubmitted(MFALSE)
    , mDataMsgEnableSet(mi4DataMsgSet)
    , mbIsLongExposureTime(MFALSE)
{
    char value[PROPERTY_VALUE_MAX];
    ::property_get(DUMP_KEY, value, "0");
    mDumpFlag = atoi(value);
    if( mDumpFlag ) {
        MY_LOGD("enable dump flag 0x%x", mDumpFlag);
        mkdir(DUMP_PATH, S_IRWXU | S_IRWXG | S_IRWXO);
    }
    MY_LOGD("mDataMsgEnableSet(0x%x), mi4DataMsgSet(0x%x)",
            mDataMsgEnableSet, mi4DataMsgSet);
}


/*******************************************************************************
*
********************************************************************************/
SingleShot::
~SingleShot()
{
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
SingleShot::
init()
{
    FUNC_START;
    MBOOL ret = MTRUE;
    FUNC_END;
    //
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
SingleShot::
uninit()
{
    MBOOL ret = MTRUE;
    FUNC_START;

    CAM_TRACE_NAME("mpPipeline->flush");
    if( mpPipeline.get() )
        mpPipeline->flush();

    FUNC_END;
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
SingleShot::
start(SensorParam const & rSensorParam)
{
    CAM_TRACE_CALL();
    FUNC_START;
    mSensorParam = rSensorParam;
    //
    dumpSensorParam(mSensorParam);

    FUNC_END;
    //
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
SingleShot::
getRawSize(SensorParam const & rSensorParam, MSize& size)
{
    CAM_TRACE_CALL();
    MUINT32 const openId     = rSensorParam.u4OpenID;
    MUINT32 const sensorMode = rSensorParam.u4Scenario;
    MUINT32 const bitDepth   = rSensorParam.u4Bitdepth;
    //
    HwInfoHelper helper(openId);
    if( ! helper.updateInfos() ) {
        MY_LOGE("cannot properly update infos");
        return MFALSE;
    }
    //
    if( ! helper.getSensorSize( sensorMode, mSensorSize) ||
            ! helper.getSensorFps( sensorMode, mSensorFps) ||
            ! helper.queryPixelMode( sensorMode, mSensorFps, mPixelMode)
      ) {
        MY_LOGE("cannot get params about sensor");
        return MFALSE;
    }
    //
    // Fullsize Raw
    MINT format;
    size_t stride;
    size = mSensorSize;
    if( ! helper.getImgoFmt(bitDepth, format) ||
            ! helper.alignPass1HwLimitation(mPixelMode, format, true, size, stride) )
    {
        MY_LOGE("wrong params about imgo");
        return MFALSE;
    }
    //
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
SingleShot::
startOne(SensorParam const & rSensorParam)
{
    CAM_TRACE_CALL();
    FUNC_START;
    mSensorParam = rSensorParam;
    dumpSensorParam(mSensorParam);
    //
    HwInfoHelper helper(mSensorParam.u4OpenID);
    if( ! helper.updateInfos() ) {
        MY_LOGE("cannot properly update infos");
        return BAD_VALUE;
    }
    //
#if 0
    if(!((helper.get4CellSensorSupported()) && ((!mShotParam.bEnableRrzoCapture))))//4cell do P2done_callback before last img data callback
    {
        Mutex::Autolock _l(mP2DoneLock);
        if (mbP2Done)
        {
            MY_LOGD("Ready to notify p2done");
            mbP2Done = MFALSE;
            handleNotifyCallback(ECamShot_NOTIFY_MSG_P2DONE, 0, 0);
        }
    }
#endif
    //
    if ( ! isDataMsgEnabled(ECamShot_DATA_MSG_ALL) && ! isNotifyMsgEnabled(ECamShot_DATA_MSG_ALL) )
    {
        MY_LOGE("no data/msg enabled");
        return MFALSE;
    }

    if ( ! createStreams() ) {
        MY_LOGE("createStreams failed");
        return MFALSE;
    }
    //
    mDataMsgEnableSet = mi4DataMsgSet;
    MY_LOGD("mDataMsgEnableSet(0x%x), mi4DataMsgSet(0x%x)", mDataMsgEnableSet, mi4DataMsgSet);
    //
    //
    sp<CamManager::UsingDeviceHelper> spDeviceHelper = new CamManager::UsingDeviceHelper(mSensorParam.u4OpenID);
    Vector<BufferItemSet> bufferSet;
    if ( ! createPipeline(bufferSet) ) {
        MY_LOGE("createPipeline failed");
        return MFALSE;
    }
    spDeviceHelper->configDone();
    //
    IMetadata appSetting = mShotParam.appSetting;
    IMetadata halSetting = mShotParam.halSetting;
    //
    if (getShotMode() == eShotMode_EngShot)
    {
            IMetadata::setEntry<MINT32>(&halSetting, MTK_HAL_REQUEST_IMG_IMGO_FORMAT, eImgFmt_BAYER10);
    }
    //
    {
        IMetadata::setEntry<MSize>(&halSetting, MTK_HAL_REQUEST_SENSOR_SIZE, mSensorSize);
    }
    // to check exposure time to identify is long exposure time or not
    {
        MINT64 exp = 0;
        IMetadata::getEntry<MINT64>(&appSetting, MTK_SENSOR_EXPOSURE_TIME, exp);
        if( exp > 500000000) //exp time unit is ns, 500000000 ns = 500 ms
        {
            Mutex::Autolock _l(mStopMtx);
            mbIsLongExposureTime = MTRUE;
            MY_LOGD("exposure time:%lld, mbIsLongExposureTime(%d)",exp,mbIsLongExposureTime);
        }
    }
    //
    if( OK != mpPipeline->submitSetting(
                0,
                appSetting,
                halSetting
                )
      )
    {
        MY_LOGE("submitRequest failed");
        return MFALSE;
    }
    //
    {
        Mutex::Autolock _l(mStopMtx);
        mbRequestSubmitted = MTRUE;
        mSubmittedCond.signal();
    }
    //
    //mpPipeline->waitUntilDrained();
    //
    if((helper.get4CellSensorSupported()&&(!mShotParam.bEnableRrzoCapture)) ||
        mShotParam.bNeedP1DoneCb)//need 4cell remosaic flow
    {
        MBOOL bBackgroundService = MFALSE;
        sp<android::RefBase> impShot; // ImpShot life extender
        // check if BackgroundService extended parameter has been set
        if (mShotParam.extParamBGserv.impShot) {
            bBackgroundService = MTRUE; // mark as using BackgroundService.
            impShot = static_cast<android::RefBase*>(mShotParam.extParamBGserv.impShot);
        }

        //
        //mpPipeline->waitUntilP1DrainedAndFlush();
        mpPipeline->waitNodeUntilDrained(eNODEID_P1Node);
        //do P1flushdone_callback to cam1device
        {
             MY_LOGD("handleNotifyCallback:ECamShot_NOTIFY_MSG_P1DONE");
             handleNotifyCallback(ECamShot_NOTIFY_MSG_P1DONE, 0, 0);
        }
        mpPipeline->flushNode(eNODEID_P1Node);
        //
        // if not using BackgroundService, wait pipeline drain here.
        if (!bBackgroundService) {
            //
            mpPipeline->waitUntilP2JpegDrainedAndFlush();
        }
        else {
            // if using BackgroundService, declare a job to wait P2 and JPEG node drain
            auto _jobWaitUntilDrain = [this](
                    sp<android::RefBase>                                lifeExtender, // extend life cycle of NormalShot instance
                    sp<NSCam::v1::NSLegacyPipeline::ILegacyPipeline>    pPipeline, // extend life cycle of SingleShot::mpPipeline
                    sp<ResourceHolder>                                  pResource, // extend life cycle of SingleShot::mpResource
                    int64_t                                             instanceId
                    )
            {
                const char* FUNCTION = "_jobWaitUntilDrain";
                MY_LOGD("%s: [+], uid=%" PRId64 "", FUNCTION, instanceId);
                pPipeline->waitUntilP2JpegDrainedAndFlush();
                MY_LOGD("%s: [-], uid=%" PRId64 "", FUNCTION, instanceId);
                pPipeline = nullptr;
                pResource = nullptr;
                lifeExtender = nullptr;
            };

            MY_LOGI("adding _jobWaitUntilDrain to PostProcessJobQueue, uid=%" PRId64 "", mShotParam.extParamBGserv.instanceId);
            NSCam::PostProcessJobQueue::addJob( std::bind(
                        std::move(_jobWaitUntilDrain),
                        impShot,
                        mpPipeline,
                        mpResource,
                        mShotParam.extParamBGserv.instanceId
                        ));
        }
    }
    else
    {
        mpPipeline->waitUntilDrainedAndFlush();
    }
    //
    {
        Mutex::Autolock _l(mStopMtx);
        mpPipeline = nullptr;
        mbIsLongExposureTime = MFALSE;
    }
    spDeviceHelper = NULL;
    mpResource.clear();
    //
    FUNC_END;
    //
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
SingleShot::
startOne(SensorParam const & rSensorParam, StreamBufferProvider* pProvider, IMetadata* rOverwrite)
{
    CAM_TRACE_CALL();
    FUNC_START;
    //
    CAM_TRACE_BEGIN("handleNotifyCallback");
    {
        Mutex::Autolock _l(mP2DoneLock);
        if (mbP2Done)
        {
            MY_LOGD("Ready to notify p2done");
            mbP2Done = MFALSE;
            handleNotifyCallback(ECamShot_NOTIFY_MSG_P2DONE, 0, 0);
        }
    }
    CAM_TRACE_END();
    //
    mSensorParam = rSensorParam;
    dumpSensorParam(mSensorParam);
    //
    sp< ISelector > pSelector = pProvider->querySelector();
    if(pSelector == NULL)
    {
        MY_LOGE("can't find Selector in Consumer");
        return MFALSE;
    }
    //
    //android::sp<IImageBuffer> pBuf = NULL;
    Vector<BufferItemSet> bufferSet;
    status_t status = OK;
    CAM_TRACE_BEGIN("getSelectorData");
    status = getSelectorData(pSelector.get(), mShotParam, bufferSet, rOverwrite);
    if(status != OK)
    {
        MY_LOGE("GetSelectorData Fail!");
        CAM_TRACE_END();
        return MFALSE;
    }
    CAM_TRACE_END();
    MY_LOGD("GetSelectorData bufferSet(%s)",ISelector::logBufferSets(bufferSet).string());
    //
    if(bufferSet.size() == 0)
    {
        MY_LOGE("Get selector buffer is NULL!");
        return MFALSE;
    }
    //
    if(isDataMsgEnabled(ECamShot_DATA_MSG_DNG))
    {
        for (size_t i = 0; i < bufferSet.size() ; i++)
        {
            if(bufferSet[i].id == eSTREAMID_IMAGE_PIPE_RAW_OPAQUE)
            {
                sp<IImageBuffer> pBuf = bufferSet[i].heap->createImageBuffer();
                if(pBuf != NULL)
                {
                    pBuf->lockBuf(LOG_TAG, eBUFFER_USAGE_SW_READ_MASK);
                    handleDataCallback(ECamShot_DATA_MSG_DNG, 0, 0, pBuf.get());
                    pBuf->unlockBuf(LOG_TAG);
                }
            }
        }
    }
    //
    if ( ! isDataMsgEnabled(ECamShot_DATA_MSG_ALL) && ! isNotifyMsgEnabled(ECamShot_DATA_MSG_ALL) )
    {
        MY_LOGE("no data/msg enabled");
        return MFALSE;
    }
    //
    CAM_TRACE_BEGIN("createStreams");
    if ( ! createStreams(pProvider) ) {
        MY_LOGE("createStreams failed");
        CAM_TRACE_END();
        return MFALSE;
    }
    CAM_TRACE_END();
    //
    CAM_TRACE_BEGIN("createPipeline");
    if ( ! createPipeline(bufferSet) ) {
        MY_LOGE("createPipeline failed");
        CAM_TRACE_END();
        return MFALSE;
    }
    CAM_TRACE_END();
    //
    IMetadata appSetting = mShotParam.appSetting;
    IMetadata halSetting = mShotParam.halSetting;
    //
    {
        IMetadata::IEntry entry(MTK_HAL_REQUEST_SENSOR_SIZE);
        entry.push_back(mSensorSize, Type2Type< MSize >());
        halSetting.update(entry.tag(), entry);
    }
    //
    ILegacyPipeline::ResultSet resultSet;
    resultSet.add(eSTREAMID_META_APP_DYNAMIC_P1, mSelectorAppMetadata);
    //
    CAM_TRACE_BEGIN("submitSetting");
    if( OK != mpPipeline->submitSetting(
                0,
                appSetting,
                halSetting,
                &resultSet
                )
      )
    {
        MY_LOGE("submitRequest failed");
        CAM_TRACE_END();
        return MFALSE;
    }
    CAM_TRACE_END();
    //
    CAM_TRACE_BEGIN("waitUntilDrained");
    mpPipeline->waitUntilDrained();
    CAM_TRACE_END();
    //
    CAM_TRACE_BEGIN("Release mpPipeline");
    mpPipeline = nullptr;
    CAM_TRACE_END();
    CAM_TRACE_BEGIN("Release mpResource");
    mpResource.clear();
    CAM_TRACE_END();
    //
    CAM_TRACE_BEGIN("return selector buffer");
    for(size_t i = 0; i < bufferSet.size() ; i++)
    {
        MY_LOGD("Ready to returnBuffer(0x%" PRIx64 ")(%p)", bufferSet[i].id, bufferSet[i].heap.get());
        pSelector->returnBuffer(bufferSet.editItemAt(i));
    }
    CAM_TRACE_END();
    //
    FUNC_END;
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
SingleShot::
startAsync(SensorParam const & /*rSensorParam*/)
{
    FUNC_START;
    //
    MY_LOGE("not supported yet");
    //
    FUNC_END;
    //
    return MFALSE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
SingleShot::
stop()
{
    CAM_TRACE_CALL();
    FUNC_START;
    //
    {
        //wait startOne() submit request then flush
        Mutex::Autolock _l(mStopMtx);
        if( !mbRequestSubmitted )
        {
            nsecs_t timeout = 2000000000LL; // wait for most 2 secs
            MY_LOGI("wait submit request +");
            mSubmittedCond.waitRelative(mStopMtx,timeout);
            MY_LOGI("wait submit request -");
        }
        if(mbIsLongExposureTime == MTRUE)
        {
            if( mpPipeline.get() )
            {
                MY_LOGD("disableNotifyMsg:EOF (Shutter)");
                disableNotifyMsg( NSCamShot::ECamShot_NOTIFY_MSG_EOF );
                MY_LOGD("flush+");
                mpPipeline->flush();
                MY_LOGD("flush-");
            }
            else
            {
                MY_LOGI("mpPipeline is NULL");
            }
        }
        else
        {
            MY_LOGD("mbIsLongExposureTime(%d), not need to flush when stop", mbIsLongExposureTime);
        }
    }
    //
    FUNC_END;
    //
    return MTRUE;
}



/*******************************************************************************
*
********************************************************************************/
MBOOL
SingleShot::
setShotParam(ShotParam const & rParam)
{
    FUNC_START;
    mShotParam = rParam;
    //
    dumpShotParam(mShotParam);

    FUNC_END;
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
SingleShot::
setJpegParam(JpegParam const & rParam)
{
    FUNC_START;
    mJpegParam = rParam;
    //
    dumpJpegParam(mJpegParam);

    FUNC_END;
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
SingleShot::
registerImageBuffer(ECamShotImgBufType const eBufType, IImageBuffer const *pImgBuffer)
{
    MY_LOGD("register bufType(%d) pImgBuffer(%p)", eBufType, pImgBuffer);
    if(pImgBuffer==NULL)
    {
        MY_LOGE("register NULL buffer!");
        return MFALSE;
    }
    IImageBuffer* pBuf = const_cast<IImageBuffer*>(pImgBuffer);
    if(mvRegBuf.indexOfKey(eBufType) >= 0)
    {

        mvRegBuf.replaceValueFor(eBufType, pBuf);
    }
    else
        mvRegBuf.add(eBufType, pBuf);
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
SingleShot::
sendCommand(MINT32 cmd, MINT32 /*arg1*/, MINT32 /*arg2*/, MINT32 /*arg3*/, MVOID* /*arg4*/)
{
    //FUNC_START;
    switch( cmd )
    {
#if 0
        case ECamShot_CMD_SET_CAPTURE_STYLE:
            muCapStyle = arg1;
            MY_LOGD("capture style 0x%x", muCapStyle);
            break;
        case ECamShot_CMD_SET_NRTYPE:
            muNRType = arg1;
            MY_LOGD("NR type 0x%x", muNRType);
            break;
        case ECamShot_CMD_SET_VHDR_PROFILE:
            mbVHdr = MTRUE; //enable vhdr mode
            mVhdrProfile = (EIspProfile_T)arg1;
            MY_LOGD("set vhdr profile 0x%x", arg1);
            break;
        case ECamShot_CMD_SET_RWB_PROC_TYPE:
            mbEnGPU = arg1; //enable GPU process
            MY_LOGD("GPU process 0x%x", mbEnGPU);
            break;
#endif
        default:
            MY_LOGE("not supported cmd 0x%x", cmd);
            return MFALSE;
    }
    //FUNC_END;
    //
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
SingleShot::
getResultMetadata(IMetadata &rAppResultMetadata, IMetadata &rHalResultMetadata)
{
    CAM_TRACE_CALL();
    Mutex::Autolock _l(mLock);
    if (!mAppDone || !mHalDone)
    {
      MY_LOGD("Wait Result of Metadata (APP: %d,HAL: %d)", mAppDone, mHalDone);
      mCond.wait(mLock);
    }

    rAppResultMetadata = mAppResultMetadata;
    rHalResultMetadata = mHalResultMetadata;
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MUINT32
SingleShot::
getRotation() const
{
    return mShotParam.u4PictureTransform;
}


/*******************************************************************************
*
********************************************************************************/
MERROR
SingleShot::
createStreams(StreamBufferProvider* pProvider)
{
    CAM_TRACE_CALL();
    MUINT32 const openId     = mSensorParam.u4OpenID;
    MUINT32 const sensorMode = mSensorParam.u4Scenario;
    MUINT32 const bitDepth   = mSensorParam.u4Bitdepth;
    //
    MSize const previewsize   = MSize(mShotParam.u4PostViewWidth, mShotParam.u4PostViewHeight);
    MINT const previewfmt     = mShotParam.ePostViewFmt;
    MINT const yuvfmt         = mShotParam.ePictureFmt;
    MBOOL const openLCS       = mShotParam.bOpenLCS;
    MSize const jpegsize      = (getRotation() & eTransform_ROT_90) ?
        MSize(mShotParam.u4PictureHeight, mShotParam.u4PictureWidth):
        MSize(mShotParam.u4PictureWidth, mShotParam.u4PictureHeight);
    MSize const thumbnailsize = MSize(mJpegParam.u4ThumbWidth, mJpegParam.u4ThumbHeight);
    //
    if( pProvider == nullptr )
    {
        HwInfoHelper helper(openId);
        if( ! helper.updateInfos() ) {
            MY_LOGE("cannot properly update infos");
            return MFALSE;
        }
        //
        if( ! helper.getSensorSize( sensorMode, mSensorSize) ||
                ! helper.getSensorFps( sensorMode, mSensorFps) ||
                ! helper.queryPixelMode( sensorMode, mSensorFps, mPixelMode)
          ) {
            MY_LOGE("cannot get params about sensor");
            return MFALSE;
        }
        //
        // Fullsize Raw
        {
            MSize size = mSensorSize;
            MINT format;
            size_t stride;
            MUINT const usage = 0; //not necessary here

            MBOOL useUnpakFmt = MFALSE;
            MBOOL useUFOfmt = MTRUE;
            // check if the sensor pattern is unpacked pattern (if it's a 4 cell sensor).
            if (helper.get4CellSensorPattern() == HwInfoHelper::e4CellSensorPattern_Unpacked)
            {
                useUnpakFmt = MTRUE;
            }

            if (getShotMode() == eShotMode_EngShot)
            {
                useUFOfmt = MFALSE;
            }
            if (isDataMsgEnabled(ECamShot_DATA_MSG_DNG))
            {
                useUFOfmt = MFALSE;
            }
            if( ! helper.getImgoFmt(bitDepth, format, useUFOfmt, useUnpakFmt) ||
                    ! helper.alignPass1HwLimitation(mPixelMode, format, true, size, stride) )
            {
                MY_LOGE("wrong params about imgo");
                return MFALSE;
            }
            //
            sp<IImageStreamInfo>
                pStreamInfo = createRawImageStreamInfo(
                        "SingleShot:Fullraw",
                        eSTREAMID_IMAGE_PIPE_RAW_OPAQUE,
                        eSTREAMTYPE_IMAGE_INOUT,
                        1, 1,
                        usage, format, size, stride
                        );
            if( pStreamInfo == nullptr ) {
                return MFALSE;
            }
            mpInfo_FullRaw = pStreamInfo;
        }
        //
        // Resize Raw
        if( helper.isRaw() && !! previewsize )
        {
            MSize size;
            MINT format;
            size_t stride;
            MBOOL useUFOfmt = MFALSE;
            MUINT const usage = 0; //not necessary here
            if (getShotMode() == eShotMode_EngShot)
            {
                useUFOfmt = MFALSE;
            }
            if (isDataMsgEnabled(ECamShot_DATA_MSG_DNG))
            {
                useUFOfmt = MFALSE;
            }
            if( ! helper.getRrzoFmt(bitDepth, format, useUFOfmt) ||
                    ! helper.alignRrzoHwLimitation(MSize(mSensorSize.w/2,mSensorSize.h/2), mSensorSize, size) ||
                    ! helper.alignPass1HwLimitation(mPixelMode, format, false, size, stride) )
            {
                MY_LOGE("wrong params about imgo");
                return MFALSE;
            }
            //
            sp<IImageStreamInfo>
                pStreamInfo = createRawImageStreamInfo(
                        "SingleShot:Resizedraw",
                        eSTREAMID_IMAGE_PIPE_RAW_RESIZER,
                        eSTREAMTYPE_IMAGE_INOUT,
                        1, 1,
                        usage, format, size, stride
                        );
            if( pStreamInfo == nullptr ) {
                return MFALSE;
            }
            mpInfo_ResizedRaw = pStreamInfo;
        }
        //
        // Lcso Raw
        if( openLCS )
        {
            NS3Av3::LCSO_Param lcsoParam;
            if ( auto pIspMgr = MAKE_IspMgr() ) {
                pIspMgr->queryLCSOParams(lcsoParam);
            }

            MUINT const usage = eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE; //not necessary here
            //
            sp<IImageStreamInfo>
                pStreamInfo = createRawImageStreamInfo(
                        "SingleShot:LCSraw",
                        eSTREAMID_IMAGE_PIPE_RAW_LCSO,
                        eSTREAMTYPE_IMAGE_INOUT,
                        2, 1,
                        usage, lcsoParam.format, lcsoParam.size, lcsoParam.stride
                        );
            if( pStreamInfo == nullptr ) {
                return MFALSE;
            }
            mpInfo_LcsoRaw = pStreamInfo;
        }
    }
    else
    {
        // zsd flow: query IImageStreamInfo of raw stream from provider
        Vector<sp<IImageStreamInfo>> rawInputInfos;
        pProvider->querySelector()->queryCollectImageStreamInfo( rawInputInfos );
        //
        for(size_t i = 0 ; i < rawInputInfos.size() ; i++)
        {
            MY_LOGD("zsd raw(%zu) stream %#" PRIx64 ", (%s) size(%dx%d), fmt 0x%x",
                i,
                rawInputInfos[i]->getStreamId(),
                rawInputInfos[i]->getStreamName(),
                rawInputInfos[i]->getImgSize().w,
                rawInputInfos[i]->getImgSize().h,
                rawInputInfos[i]->getImgFormat()
            );
        }
        //
        HwInfoHelper helper(openId);
        if( ! helper.updateInfos() ) {
            MY_LOGE("cannot properly update infos");
            return MFALSE;
        }
        if( ! helper.getSensorSize( sensorMode, mSensorSize) ||
                ! helper.getSensorFps( sensorMode, mSensorFps) ||
                ! helper.queryPixelMode( sensorMode, mSensorFps, mPixelMode)
          ) {
            MY_LOGE("cannot get params about sensor");
            return MFALSE;
        }
        //
    }
    //
    if( isDataMsgEnabled(ECamShot_DATA_MSG_POSTVIEW) )
    {
        MSize size        = previewsize;
        MINT format       = previewfmt;
        MUINT const usage = 0; //not necessary here
        MUINT32 transform = 0;
        sp<IImageStreamInfo>
            pStreamInfo = createImageStreamInfo(
                    "SingleShot:Postview",
                    eSTREAMID_IMAGE_PIPE_YUV_00,
                    eSTREAMTYPE_IMAGE_INOUT,
                    1, 1,
                    usage, format, size, transform
                    );
        if( pStreamInfo == nullptr ) {
            return BAD_VALUE;
        }
        //
        mpInfo_YuvPostview = pStreamInfo;
    }
    //
    // Yuv
    if( isDataMsgEnabled(ECamShot_DATA_MSG_YUV|ECamShot_DATA_MSG_JPEG) )
    {
        MSize size        = jpegsize;
        MINT format       = yuvfmt;
        MUINT const usage = 0; //not necessary here
        MUINT32 transform = getRotation();
        sp<IImageStreamInfo>
            pStreamInfo = createImageStreamInfo(
                    "SingleShot:Yuv",
                    eSTREAMID_IMAGE_PIPE_YUV_JPEG,
                    eSTREAMTYPE_IMAGE_INOUT,
                    1, 1,
                    usage, format, size, transform
                    );
        if( pStreamInfo == nullptr ) {
            return BAD_VALUE;
        }
        //
        mpInfo_Yuv = pStreamInfo;
    }
    //
    // Thumbnail Yuv
    if( isDataMsgEnabled(ECamShot_DATA_MSG_JPEG) && thumbnailsize != MSize(0,0))
    {
        MSize size        = thumbnailsize;
        MINT format       = eImgFmt_YUY2;
        MUINT const usage = 0; //not necessary here
        MUINT32 transform = 0;
        sp<IImageStreamInfo>
            pStreamInfo = createImageStreamInfo(
                    "SingleShot:ThumbnailYuv",
                    eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL,
                    eSTREAMTYPE_IMAGE_INOUT,
                    1, 1,
                    usage, format, size, transform
                    );
        if( pStreamInfo == nullptr ) {
            return BAD_VALUE;
        }
        //
        mpInfo_YuvThumbnail = pStreamInfo;
    }
    //
    // Jpeg
    if( isDataMsgEnabled(ECamShot_DATA_MSG_JPEG) )
    {
        MSize size        = jpegsize;
        MINT format       = eImgFmt_BLOB;
        MUINT const usage = 0; //not necessary here
        MUINT32 transform = 0;
        sp<IImageStreamInfo>
            pStreamInfo = createImageStreamInfo(
                    "SingleShot:Jpeg",
                    eSTREAMID_IMAGE_JPEG,
                    eSTREAMTYPE_IMAGE_INOUT,
                    1, 1,
                    usage, format, size, transform
                    );
        if( pStreamInfo == nullptr ) {
            return BAD_VALUE;
        }
        //
        mpInfo_Jpeg = pStreamInfo;
    }
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MERROR
SingleShot::
createPipeline(Vector<BufferItemSet>& rBufferSet)
{
    CAM_TRACE_CALL();
    FUNC_START;
    LegacyPipelineBuilder::ConfigParams LPBConfigParams;
    //
    sp<IResourceContainer> pResourceContainer = IResourceContainer::getInstance(mSensorParam.u4OpenID);
    LPBConfigParams.p1ConfigConcurrency = pResourceContainer->queryResourceConcurrency(IResourceContainer::RES_CONCUR_P1);
    LPBConfigParams.p2ConfigConcurrency = pResourceContainer->queryResourceConcurrency(IResourceContainer::RES_CONCUR_P2);
    //
    LPBConfigParams.mode                 = getLegacyPipelineMode();
    LPBConfigParams.enableEIS            = MFALSE;
    LPBConfigParams.enableLCS            = mShotParam.bOpenLCS;
    LPBConfigParams.vendorManagerId      = mShotParam.extParamBGserv.vendorManagerId;
#if 1
    LPBConfigParams.optimizedCaptureFlow = (getShotMode() == eShotMode_NormalShot ||
                                            getShotMode() == eShotMode_4CellRemosaicShot ||
                                            getShotMode() == eShotMode_EngShot) ? MTRUE : MFALSE;
#endif
    //
    HwInfoHelper helper(mSensorParam.u4OpenID);
    if( ! helper.updateInfos() ) {
        MY_LOGE("cannot properly update infos");
        return BAD_VALUE;
    }
    //
    if (helper.getPDAFSupported(SENSOR_SCENARIO_ID_NORMAL_CAPTURE) && !CamManager::getInstance()->isMultiDevice())
    {
        LPBConfigParams.disableFrontalBinning = MTRUE;
    }
    //
    if (helper.getDualPDAFSupported(mSensorParam.u4Scenario))
    {
        LPBConfigParams.enableDualPD = MTRUE;
    }
    //
    if (helper.get4CellSensorSupported())
    {
        LPBConfigParams.support4cellsensor= 1;
        LPBConfigParams.enableRrzoCapture = mShotParam.bEnableRrzoCapture;
        //
        MUINT32 u4RawFmtType = 0;
        if(!helper.getSensorRawFmtType(u4RawFmtType))
        {
            MY_LOGW("getSensorRawFmtType() fail!");
        }
        if(u4RawFmtType == SENSOR_RAW_4CELL)
        {
            LPBConfigParams.enableQuadCode = MTRUE;
        }
        if(u4RawFmtType == SENSOR_RAW_4CELL_HW_BAYER)
        {
            LPBConfigParams.support4cellsensor= 0;
        }
    }
    //
    MUINT32 pipeBit;
    MINT ImgFmt = eImgFmt_BAYER12;
    if( helper.getLpModeSupportBitDepthFormat(ImgFmt, pipeBit) )
    {
        using namespace NSCam::NSIoPipe::NSCamIOPipe;
        if(pipeBit & CAM_Pipeline_14BITS)
        {
            LPBConfigParams.pipeBit = CAM_Pipeline_14BITS;
        }
        else
        {
            LPBConfigParams.pipeBit = CAM_Pipeline_12BITS;
        }
    }
    //
    LPBConfigParams.needAddRrzoIntoP2IoMap = MFALSE;
    //
    CAM_TRACE_BEGIN("create builder");
    sp<LegacyPipelineBuilder> pBuilder = LegacyPipelineBuilder::createInstance(
                                            mSensorParam.u4OpenID,
                                            "SingleShot",
                                            LPBConfigParams);
    CAM_TRACE_END();

    CHECK_OBJECT(pBuilder);

    CAM_TRACE_BEGIN("create ImageCallback");
    sp<ImageCallback> pCallback = new ImageCallback(this, 0);
    mpResource->add(pCallback);
    CAM_TRACE_END();

    CAM_TRACE_BEGIN("create BufferCallbackHandler");
    sp<BufferCallbackHandler> pCallbackHandler = new BufferCallbackHandler(mSensorParam.u4OpenID);
    pCallbackHandler->setImageCallback(pCallback);
    mpResource->add(pCallbackHandler);
    CAM_TRACE_END();

    CAM_TRACE_BEGIN("create StreamBufferProviderFactory");
    sp<StreamBufferProviderFactory> pFactory =
                StreamBufferProviderFactory::createInstance();
    CAM_TRACE_END();
    //
    Vector<PipelineImageParam> vImageParam;

    //Set up mmdvfs
    IScenarioControl::ControlParam param;
    param.scenario = IScenarioControl::Scenario_ContinuousShot;
    param.sensorSize = mSensorSize;
    param.sensorFps  = mSensorFps;
    param.supportCustomOption= ((helper.get4CellSensorSupported())?1:0); //0: CUSTOM_OPTION_NOME ; 1:CUSTOM_OPTION_SENSOR_4CELL
    if(LPBConfigParams.enableDualPD)
        FEATURE_CFG_ENABLE_MASK(param.featureFlag,IScenarioControl::FEATURE_DUAL_PD);

    //
    m_scenarioControl = IScenarioControl::create(mSensorParam.u4OpenID);

    if( m_scenarioControl.get() == NULL )
    {
        MY_LOGE("get Scenario Control fail");
        return MFALSE;
    }

    m_scenarioControl->enterScenario(param);
    pBuilder->setScenarioControl(m_scenarioControl);

    //
    if( rBufferSet.size() == 0 )
    {
        MY_LOGD("createPipeline for Normal");
        // normal flow
        PipelineSensorParam sensorParam(
            /*.mode      = */mSensorParam.u4Scenario,
            /*.rawType   = */mSensorParam.u4RawType,
            /*.size      = */mSensorSize,
            /*.fps       = */(MUINT)mSensorFps,
            /*.pixelMode = */mPixelMode,
            /*.vhdrMode  = */mSensorParam.u4VHDRMode
            );
        //
        if( OK != pBuilder->setSrc(sensorParam) ) {
            MY_LOGE("setSrc failed");
            return MFALSE;
        }

        //
        m_scenarioControl = IScenarioControl::create(mSensorParam.u4OpenID);

        if( m_scenarioControl.get() == NULL )
        {
            MY_LOGE("get Scenario Control fail");
            return MFALSE;
        }

        IScenarioControl::ControlParam param;
        param.scenario = IScenarioControl::Scenario_ContinuousShot;
        param.sensorSize = mSensorSize;
        param.sensorFps  = mSensorFps;
        param.supportCustomOption= ((helper.get4CellSensorSupported())?1:0); //0: CUSTOM_OPTION_NOME ; 1:CUSTOM_OPTION_SENSOR_4CELL
        if(LPBConfigParams.enableDualPD)
            FEATURE_CFG_ENABLE_MASK(param.featureFlag,IScenarioControl::FEATURE_DUAL_PD);

        m_scenarioControl->enterScenario(param);
        pBuilder->setScenarioControl(m_scenarioControl);
        //

        // fullraw
        if( mpInfo_FullRaw.get() )
        {
            sp<IImageStreamInfo> pStreamInfo = mpInfo_FullRaw;
            //
            //pCallbackHandler->setImageStreamInfo(ECamShot_DATA_MSG_RAW, pStreamInfo,
            //    (mvRegBuf.indexOfKey(ECamShot_BUF_TYPE_RAW)>=0 ? mvRegBuf.editValueFor(ECamShot_BUF_TYPE_RAW) : NULL ));
            //
            sp<CallbackBufferPool> pPool = new CallbackBufferPool(pStreamInfo);
            if(mvRegBuf.indexOfKey(ECamShot_BUF_TYPE_RAW)>=0)
            {
                pPool->addBuffer(mvRegBuf.editValueFor(ECamShot_BUF_TYPE_RAW));
            }
            else
            {
                pPool->allocateBuffer(
                                  pStreamInfo->getStreamName(),
                                  pStreamInfo->getMaxBufNum(),
                                  pStreamInfo->getMinInitBufNum()
                                  );
            }
            pCallbackHandler->setBufferPool(pPool);
            //
            pFactory->setImageStreamInfo(pStreamInfo);
            pFactory->setUsersPool(
                        pCallbackHandler->queryBufferPool(pStreamInfo->getStreamId())
                    );
            //
            PipelineImageParam imgParam = {
                .pInfo     = pStreamInfo,
                .pProvider = pFactory->create(),
                .usage     = 0
            };
            vImageParam.push_back(imgParam);
        }
        // resized raw
        if( mpInfo_ResizedRaw.get() )
        {
            sp<IImageStreamInfo> pStreamInfo = mpInfo_ResizedRaw;
            //
            PipelineImageParam imgParam = {
                .pInfo     = pStreamInfo,
                .pProvider = nullptr,
                .usage     = 0
            };
            vImageParam.push_back(imgParam);
        }
        // Lcsoraw
        if( mpInfo_LcsoRaw.get() )
        {
            sp<IImageStreamInfo> pStreamInfo = mpInfo_LcsoRaw;

            PipelineImageParam imgParam = {
                .pInfo     = pStreamInfo,
                .pProvider = nullptr,
                .usage     = 0
            };
            vImageParam.push_back(imgParam);
        }
    }
    else
    {
        CAM_TRACE_NAME("pBuilder->setSrc");
        MY_LOGD("createPipeline for ZSD/VSS");
        Vector<PipelineImageParam> imgSrcParam;
        // zsd flow
        sp<IImageStreamInfo> pStreamInfo = NULL;
        for(size_t i = 0; i < rBufferSet.size() ; i++)
        {
            pStreamInfo = rBufferSet[i].streamInfo;
            //
            if(pStreamInfo == NULL)
            {
                MY_LOGE("Can not find stream info (0x%" PRIx64 "),(%p)", rBufferSet[i].id, rBufferSet[i].heap.get());
                return MFALSE;
            }
            sp<IImageBuffer> pBuf = rBufferSet[i].heap->createImageBuffer();
            if(pBuf == NULL)
            {
                MY_LOGE("Can not build image buffer (0x%" PRIx64 "),(%p)", rBufferSet[i].id, rBufferSet[i].heap.get());
                return MFALSE;
            }
            sp<CallbackBufferPool> pPool = new CallbackBufferPool(pStreamInfo);
            pPool->addBuffer(pBuf);
            pCallbackHandler->setBufferPool(pPool);
            //
            pFactory->setImageStreamInfo(pStreamInfo);
            pFactory->setUsersPool(
                    pCallbackHandler->queryBufferPool(pStreamInfo->getStreamId())
                );
            PipelineImageParam imgParam = {
                .pInfo     = pStreamInfo,
                .pProvider = pFactory->create(),
                .usage     = 0
            };
            imgSrcParam.push_back(imgParam);
        }
        //
        if( OK != pBuilder->setSrc(imgSrcParam) ) {
            MY_LOGE("setSrc failed");
            return MFALSE;
        //
        }
    }
    //
    {
        CAM_TRACE_NAME("pBuilder->setDst");
        if( mpInfo_Yuv.get() )
        {
            CAM_TRACE_NAME("set mpInfo_Yuv");
            sp<IImageStreamInfo> pStreamInfo = mpInfo_Yuv;
            //
            //pCallbackHandler->setImageStreamInfo(ECamShot_DATA_MSG_YUV, pStreamInfo,
            //    (mvRegBuf.indexOfKey(ECamShot_BUF_TYPE_YUV)>=0 ? mvRegBuf.editValueFor(ECamShot_BUF_TYPE_YUV) : NULL ));
            sp<CallbackBufferPool> pPool = new CallbackBufferPool(pStreamInfo);
            if(mvRegBuf.indexOfKey(ECamShot_BUF_TYPE_YUV)>=0)
            {
                CAM_TRACE_NAME("addBuffer");
                pPool->addBuffer(mvRegBuf.editValueFor(ECamShot_BUF_TYPE_YUV));
            }
            else
            {
                CAM_TRACE_NAME("allocateBuffer");
                pPool->allocateBuffer(
                                  pStreamInfo->getStreamName(),
                                  pStreamInfo->getMaxBufNum(),
                                  pStreamInfo->getMinInitBufNum()
                                  );
            }
            pCallbackHandler->setBufferPool(pPool);
            //
            pFactory->setImageStreamInfo(pStreamInfo);
            pFactory->setUsersPool(
                        pCallbackHandler->queryBufferPool(pStreamInfo->getStreamId())
                    );
            //
            PipelineImageParam imgParam = {
                .pInfo     = pStreamInfo,
                .pProvider = pFactory->create(),
                .usage     = 0
            };
            vImageParam.push_back(imgParam);
        }
        //
        if( mpInfo_YuvPostview.get() )
        {
            CAM_TRACE_NAME("set mpInfo_YuvPostview");
            sp<IImageStreamInfo> pStreamInfo = mpInfo_YuvPostview;
            //
            //pCallbackHandler->setImageStreamInfo(ECamShot_DATA_MSG_POSTVIEW, pStreamInfo,
            //    (mvRegBuf.indexOfKey(ECamShot_BUF_TYPE_POSTVIEW)>=0 ? mvRegBuf.editValueFor(ECamShot_BUF_TYPE_POSTVIEW) : NULL ));
            sp<CallbackBufferPool> pPool = new CallbackBufferPool(pStreamInfo);
            if(mvRegBuf.indexOfKey(ECamShot_BUF_TYPE_POSTVIEW)>=0)
            {
                CAM_TRACE_NAME("addBuffer");
                pPool->addBuffer(mvRegBuf.editValueFor(ECamShot_BUF_TYPE_POSTVIEW));
            }
            else
            {
                CAM_TRACE_NAME("allocateBuffer");
                pPool->allocateBuffer(
                                  pStreamInfo->getStreamName(),
                                  pStreamInfo->getMaxBufNum(),
                                  pStreamInfo->getMinInitBufNum()
                                  );
            }
            pCallbackHandler->setBufferPool(pPool);
            //
            pFactory->setImageStreamInfo(pStreamInfo);
            pFactory->setUsersPool(
                        pCallbackHandler->queryBufferPool(pStreamInfo->getStreamId())
                    );
            //
            PipelineImageParam imgParam = {
                .pInfo     = pStreamInfo,
                .pProvider = pFactory->create(),
                .usage     = 0
            };
            vImageParam.push_back(imgParam);
        }
        //
        if( mpInfo_YuvThumbnail.get() )
        {
            CAM_TRACE_NAME("set mpInfo_YuvThumbnail");
            sp<IImageStreamInfo> pStreamInfo = mpInfo_YuvThumbnail;
            //
            PipelineImageParam imgParam = {
                .pInfo     = pStreamInfo,
                .pProvider = nullptr,
                .usage     = 0
            };
            vImageParam.push_back(imgParam);
        }
        //
        if( mpInfo_Jpeg.get() )
        {
            CAM_TRACE_NAME("set mpInfo_Jpeg");
            sp<IImageStreamInfo> pStreamInfo = mpInfo_Jpeg;
            //
            //pCallbackHandler->setImageStreamInfo(ECamShot_DATA_MSG_JPEG, pStreamInfo,
            //    (mvRegBuf.indexOfKey(ECamShot_BUF_TYPE_JPEG)>=0 ? mvRegBuf.editValueFor(ECamShot_BUF_TYPE_JPEG) : NULL ));
            sp<CallbackBufferPool> pPool = new CallbackBufferPool(pStreamInfo);
            if(mvRegBuf.indexOfKey(ECamShot_BUF_TYPE_JPEG)>=0)
            {
                CAM_TRACE_NAME("addBuffer");
                pPool->addBuffer(mvRegBuf.editValueFor(ECamShot_BUF_TYPE_JPEG));
            }
            else
            {
                CAM_TRACE_NAME("allocateBuffer");
                pPool->allocateBuffer(
                                  pStreamInfo->getStreamName(),
                                  pStreamInfo->getMaxBufNum(),
                                  pStreamInfo->getMinInitBufNum()
                                  );
            }
            pCallbackHandler->setBufferPool(pPool);
            //
            pFactory->setImageStreamInfo(pStreamInfo);
            pFactory->setUsersPool(
                        pCallbackHandler->queryBufferPool(pStreamInfo->getStreamId())
                    );
            //
            PipelineImageParam imgParam = {
                .pInfo     = pStreamInfo,
                .pProvider = pFactory->create(),
                .usage     = 0
            };
            vImageParam.push_back(imgParam);
        }
        //
        if( OK != pBuilder->setDst(vImageParam) ) {
            MY_LOGE("setDst failed");
            return MFALSE;
        }
    }
    //
    CAM_TRACE_BEGIN("pBuilder->create");
    mpPipeline = pBuilder->create();
    CAM_TRACE_END();
    CHECK_OBJECT(mpPipeline);
    //
    {
        CAM_TRACE_NAME("setup ResultProcessor");
        sp<ResultProcessor> pResultProcessor = mpPipeline->getResultProcessor().promote();
        CHECK_OBJECT(pResultProcessor);

        //partial
        sp<MetadataListener> pListenerPartial = new MetadataListener(this);
        mpResource->add(pListenerPartial);
        //
        pResultProcessor->registerListener( 0, 0, true, pListenerPartial);
        //
        //full
        sp<MetadataListener> pListener = new MetadataListener(this);
        mpResource->add(pListener);
        //
        pResultProcessor->registerListener( 0, 0, false, pListener);
        //
        if( rBufferSet.size() != 0 )
        {
            sp<TimestampProcessor> pTimestampProcessor = mpPipeline->getTimestampProcessor().promote();
            pTimestampProcessor->onResultReceived(
                                    0,
                                    eSTREAMID_META_APP_DYNAMIC_P1,
                                    MFALSE,
                                    mSelectorAppMetadata);
        }
        mAppDone = MFALSE;
        mHalDone = MFALSE;

        mAppResultMetadata.clear();
        mHalResultMetadata.clear();
    }
    //
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
SingleShot::
onMetaReceived(
    MUINT32         const requestNo,
    StreamId_T      const streamId,
    MBOOL           const /*errorResult*/,
    IMetadata       const result
)
{
    CAM_TRACE_FMT_BEGIN("onMetaReceived No%d,StreamID  %#" PRIx64, requestNo,streamId);
    MY_LOGD("requestNo %d, stream %#" PRIx64, requestNo, streamId);

    Mutex::Autolock _l(mLock);

    if (streamId == eSTREAMID_META_APP_FULL)
    {
      mAppResultMetadata = result;
      mAppDone = MTRUE;
#if 0
      sp<IMetadataConverter> pMetadataConverter;
      pMetadataConverter = IMetadataConverter::createInstance(IDefaultMetadataTagSet::singleton()->getTagSet());
      pMetadataConverter->dump(mAppResultMetadata, 0);
#endif
    }
    else if (streamId == eSTREAMID_META_HAL_FULL)
    {
      mHalResultMetadata = result;
      mHalDone = MTRUE;
    }

    if ((streamId == eSTREAMID_META_APP_FULL || streamId == eSTREAMID_META_HAL_FULL)
      && mAppDone && mHalDone)
    {
      mCond.signal();
    }



    {
      Mutex::Autolock _l(mShutterLock);
      if (mbCBShutter)
      {
        mbCBShutter = MFALSE;
        handleNotifyCallback(ECamShot_NOTIFY_MSG_EOF, 0, 0);
      }
    }
    //
    mAppResultMetadata+=mSelectorAppMetadata;
    //
    if(streamId == eSTREAMID_META_APP_FULL)
        handleDataCallback(ECamShot_DATA_MSG_APPMETA, (MUINTPTR)&mAppResultMetadata, 0, 0);
    else if(streamId == eSTREAMID_META_HAL_FULL)
        handleDataCallback(ECamShot_DATA_MSG_HALMETA, (MUINTPTR)&mHalResultMetadata, 0, 0);
    //
    CAM_TRACE_FMT_END();
}


/*******************************************************************************
*
********************************************************************************/
MVOID
SingleShot::
onDataReceived(
    MUINT32 const               requestNo,
    StreamId_T const            streamId,
    android::sp<IImageBuffer>&  pBuffer
)
{
    CAM_TRACE_FMT_BEGIN("onDataReceived No%d,streamId%" PRIx64 ,requestNo,streamId);
    MY_LOGD("requestNo %d, streamId 0x%" PRIx64 ", buffer %p", requestNo, streamId, pBuffer.get());
    //
    ECamShotDataMsg data = ECamShot_DATA_MSG_NONE;
    switch (streamId)
    {
        case eSTREAMID_IMAGE_PIPE_RAW_OPAQUE:
            data = ECamShot_DATA_MSG_RAW;
            break;
        case eSTREAMID_IMAGE_PIPE_YUV_JPEG:
            data = ECamShot_DATA_MSG_YUV;
            break;
        case eSTREAMID_IMAGE_PIPE_YUV_00:
            data = ECamShot_DATA_MSG_POSTVIEW;
            break;
        case eSTREAMID_IMAGE_JPEG:
            data = ECamShot_DATA_MSG_JPEG;
            break;
        default:
            data = ECamShot_DATA_MSG_NONE;
            break;
    }
    //disable datamsgset
    MY_LOGD("start : mDataMsgEnableSet(0x%x), mi4DataMsgSet(0x%x), data(0x%x)", mDataMsgEnableSet, mi4DataMsgSet, data);
    ::android_atomic_and(~((MINT32)data), &mDataMsgEnableSet);
    MY_LOGD("mDataMsgEnableSet(0x%x), mi4DataMsgSet(0x%x), data(0x%x)", mDataMsgEnableSet, mi4DataMsgSet, data);
    //
    HwInfoHelper helper(mSensorParam.u4OpenID);
    if( ! helper.updateInfos() ) {
        MY_LOGE("cannot properly update infos");
    }
    else
    {
        //
        if(((helper.get4CellSensorSupported()) && ((!mShotParam.bEnableRrzoCapture))&& (mDataMsgEnableSet == ECamShot_DATA_MSG_NONE)) || ////4cell do P2done_callback before last img data callback
            mShotParam.bNeedP2DoneCb)
        {
            MY_LOGD("(mDataMsgEnableSet == ECamShot_DATA_MSG_NONE)");
            Mutex::Autolock _l(mP2DoneLock);
            if (mbP2Done)
            {
                MY_LOGD("4cell support or mShotParam.bNeedP2DoneCb==true, and last streamID: %d, Ready to notify p2done", streamId);
                mbP2Done = MFALSE;
                handleNotifyCallback(ECamShot_NOTIFY_MSG_P2DONE, 0, 0);
            }
        }
        //
    }
    //
    switch( data )
    {
        case ECamShot_DATA_MSG_RAW:
        case ECamShot_DATA_MSG_YUV:
        case ECamShot_DATA_MSG_POSTVIEW:
        case ECamShot_DATA_MSG_JPEG:
            break;
        default:
            MY_LOGW("not supported yet: 0x%x", data);
            break;
    }

    if( pBuffer != 0 )
    {
        CAM_TRACE_NAME("handleDataCallback");
        // debug
        if( mDumpFlag & data )
        {
            String8 filename = String8::format("%s/singleshot_%dx%d",
                    DUMP_PATH, pBuffer->getImgSize().w, pBuffer->getImgSize().h);
            switch( data )
            {
                case ECamShot_DATA_MSG_RAW:
                    filename += String8::format("_%zu.raw", pBuffer->getBufStridesInBytes(0));
                    break;
                case ECamShot_DATA_MSG_YUV:
                case ECamShot_DATA_MSG_POSTVIEW:
                    filename += String8(".yuv");
                    break;
                case ECamShot_DATA_MSG_JPEG:
                    filename += String8(".jpeg");
                    break;
                default:
                    break;
            }
            pBuffer->lockBuf(LOG_TAG, eBUFFER_USAGE_SW_READ_MASK);
            pBuffer->saveToFile(filename);
            pBuffer->unlockBuf(LOG_TAG);
            //
            MY_LOGD("dump buffer in %s", filename.string());
        }
        handleDataCallback(data, 0, 0, pBuffer.get());
    }
    CAM_TRACE_FMT_END();
}


/*******************************************************************************
*
********************************************************************************/

MINT
SingleShot::
getLegacyPipelineMode(void)
{
    int shotMode = getShotMode();
    EPipelineMode pipelineMode = getPipelineMode();
    int legacyPipeLineMode = LegacyPipelineMode_T::PipelineMode_Capture;
    switch(shotMode)
    {
        case eShotMode_VideoSnapShot:
            MY_LOGD("shotMode(%d)",shotMode);
            legacyPipeLineMode = (pipelineMode == ePipelineMode_Feature) ?
                LegacyPipelineMode_T::PipelineMode_Feature_Capture_VSS :
                LegacyPipelineMode_T::PipelineMode_Capture_VSS;
            break;

        default:
            legacyPipeLineMode = (pipelineMode == ePipelineMode_Feature) ?
                LegacyPipelineMode_T::PipelineMode_Feature_Capture :
                LegacyPipelineMode_T::PipelineMode_Capture;
            break;
    }
    return legacyPipeLineMode;
}

/*******************************************************************************
*
********************************************************************************/
status_t
SingleShot::
getSelectorData(
    ISelector*                  pSelector,
    ShotParam&                  rShotParam,
    Vector<BufferItemSet>&      rBufferSet,
    IMetadata*                  rOverwrite
    )
{
    CAM_TRACE_CALL();
    if(pSelector==NULL)
    {
        MY_LOGE("input Selector is NULL!");
        return UNKNOWN_ERROR;
    }


    status_t status = OK;
    MINT32 rRequestNo;
    Vector<ISelector::MetaItemSet> rvResultMeta;
    status = pSelector->getResult(rRequestNo, rvResultMeta, rBufferSet);

    CAM_TRACE_FMT_BEGIN("getSelectorData_%d",rRequestNo);

    if(status!=OK)
    {
        MY_LOGE("Selector getResult Fail!");
        CAM_TRACE_FMT_END();
        return UNKNOWN_ERROR;
    }

    if(rvResultMeta.size()==0)
    {
        MY_LOGE("getResult rvResultMeta == 0");
        CAM_TRACE_FMT_END();
        return UNKNOWN_ERROR;
    }

    MBOOL bAppSetting = MFALSE, bHalSetting = MFALSE;
    for(MUINT32 i=0; i<rvResultMeta.size(); i++)
    {
        if(rvResultMeta.editItemAt(i).id == eSTREAMID_META_APP_DYNAMIC_P1)
        {
            mSelectorAppMetadata = rvResultMeta.editItemAt(i).meta;
            bAppSetting = MTRUE;
        }
        else
        if(rvResultMeta.editItemAt(i).id == eSTREAMID_META_HAL_DYNAMIC_P1)
        {
            // overwrite some setting
            rShotParam.halSetting += rvResultMeta.editItemAt(i).meta;
            //
            if(rOverwrite != NULL)//VSS+VHDR change capture ISP profile
            {
                IMetadata overWrite = *rOverwrite;
                rShotParam.halSetting += overWrite;
            }
            //
            bHalSetting = MTRUE;
        }
    }
    //
    if( !bHalSetting ||
        !bAppSetting)
    {
        MY_LOGE("can't find App(%d)/Hal(%d) Setting from ZsdSelect getResult rvResultMeta",
                bAppSetting,
                bHalSetting);
        CAM_TRACE_FMT_END();
        return UNKNOWN_ERROR;
    }
#if 0    //
    pBuffer = rpHeap->createImageBuffer();
    //
    if(pBuffer == NULL)
    {
        MY_LOGE("get buffer is NULL!");
        CAM_TRACE_FMT_END();
        return UNKNOWN_ERROR;
    }
#endif

    CAM_TRACE_FMT_END();
    return OK;
}

////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamShot
