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

#define LOG_TAG "MtkCam/CollectShot"
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/Trace.h>
//
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/aaa/IIspMgr.h>
//
#include <mtkcam/middleware/v1/IShot.h>
//
#include "ImpShot.h"
#include "CollectShot.h"
//
#include <mtkcam/utils/hw/CamManager.h>
using namespace NSCam::Utils;
//
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/middleware/v1/camshot/_params.h>
//
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/ILegacyPipeline.h>
#include <mtkcam/utils/hw/HwInfoHelper.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <utils/Vector.h>
//
#include <mtkcam/aaa/IDngInfo.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/LegacyPipelineUtils.h>
#include <mtkcam/middleware/v1/LegacyPipeline/LegacyPipelineBuilder.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProviderFactory.h>

#include <mtkcam/aaa/IHal3A.h>

// IVendor
#include <mtkcam/pipeline/extension/Collect.h>

// postprocess
#include <mtkcam/middleware/v1/camutils/IDetachJobManager.h>
#include <mtkcam/middleware/v1/camutils/PostProcessJobQueue.h>
#include <mtkcam/feature/effectHalBase/ICallbackClientMgr.h>

//Std
#include <chrono>
#include <thread>

using namespace android;
using namespace NSShot::NSSmartShot;
using namespace NSCam;
using namespace NSCam::v1;
using namespace NSCam::v1::NSLegacyPipeline;
using namespace NSCamHW;
using namespace NS3Av3;
using namespace NSCam::plugin;

#define CHECK_OBJECT(x)  do{                                        \
    if (x == nullptr) { MY_LOGE("Null %s Object", #x); return MFALSE;} \
} while(0)

#include <cutils/properties.h>
/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getShotName(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


#define FUNC_START                  MY_LOGD("+")
#define FUNC_END                    MY_LOGD("-")

#define FUNCTION_SCOPE          auto __scope_logger__ = create_scope_logger(__FUNCTION__)
#include <memory>
static std::shared_ptr<char> create_scope_logger(const char* functionName)
{
    char* pText = const_cast<char*>(functionName);
    CAM_LOGD("[%s] + ", pText);
    return std::shared_ptr<char>(pText, [](char* p){ CAM_LOGD("[%s] -", p); });
}


#define DEFAULT_MAX_FRAME_NUM       3

/******************************************************************************
 *
 ******************************************************************************/
extern
sp<IShot>
createInstance_CollectShot(
    char const*const    pszShotName,
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId
)
{
    sp<IShot>      pShot        = NULL;
    sp<SmartShot>  pImpShot     = NULL;
    //
    //  (1.1) new Implementator.
    pImpShot = new CollectShot(pszShotName, u4ShotMode, i4OpenId, false);
    if  ( pImpShot == 0 ) {
        CAM_LOGE("[%s] new CollectShot", __FUNCTION__);
        goto lbExit;
    }
    //
    //  (1.2) initialize Implementator if needed.
    if  ( ! pImpShot->onCreate() ) {
        CAM_LOGE("[%s] onCreate()", __FUNCTION__);
        goto lbExit;
    }
    //
    //  (2)   new Interface.
    pShot = new IShot(pImpShot);
    if  ( pShot == 0 ) {
        CAM_LOGE("[%s] new IShot", __FUNCTION__);
        goto lbExit;
    }
    //
lbExit:
    //
    //  Free all resources if this function fails.
    if  ( pShot == 0 && pImpShot != 0 ) {
        pImpShot->onDestroy();
        pImpShot = NULL;
    }
    //
    return  pShot;
}

/******************************************************************************
 *
 ******************************************************************************/
extern
sp<IShot>
createInstance_ZsdCollectShot(
    char const*const    pszShotName,
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId
)
{
    sp<IShot>       pShot       = NULL;
    sp<CollectShot>    pImpShot    = NULL;
    //
    //  (1.1) new Implementator.
    pImpShot = new CollectShot(pszShotName, u4ShotMode, i4OpenId, true);
    if  ( pImpShot == 0 ) {
        CAM_LOGE("[%s] new CollectShot", __FUNCTION__);
        goto lbExit;
    }
    //
    //  (1.2) initialize Implementator if needed.
    if  ( ! pImpShot->onCreate() ) {
        CAM_LOGE("[%s] onCreate()", __FUNCTION__);
        goto lbExit;
    }
    //
    //  (2)   new Interface.
    pShot = new IShot(pImpShot);
    if  ( pShot == 0 ) {
        CAM_LOGE("[%s] new IShot", __FUNCTION__);
        goto lbExit;
    }
    //
lbExit:
    //
    //  Free all resources if this function fails.
    if  ( pShot == 0 && pImpShot != 0 ) {
        pImpShot->onDestroy();
        pImpShot = NULL;
    }
    //
    return  pShot;
}

static
MVOID
PREPARE_STREAM(BufferList& vDstStreams, StreamId id, MBOOL criticalBuffer)
{
    vDstStreams.push_back(
        BufferSet{
            .streamId       = id,
            .criticalBuffer = criticalBuffer,
        }
    );
}

static void onFakeJpegCallback(const sp<IShotCallback>& callback)
{
    if (callback == NULL)
    {
        CAM_LOGE("[onFakeJpegCallback] shot callback is null");
        return;
    }

    CAM_LOGW("[onFakeJpegCallback] send dummy jpeg callback");

    const uint32_t jpegSize = 512;
    const uint8_t  jpegBuffer[jpegSize] {};
    const uint32_t exifHeaderSize = 512;
    const uint8_t  exifHeaderBuffer[exifHeaderSize] {};

    callback->onCB_CompressedImage(
            0, jpegSize, jpegBuffer, exifHeaderSize, exifHeaderBuffer, 0, true);
}

/******************************************************************************
 *
 ******************************************************************************/
CollectShot::
CollectShot(
    char const*const pszShotName,
    uint32_t const   u4ShotMode,
    int32_t const    i4OpenId,
    bool const       isZsd
)
    : SmartShot(pszShotName, u4ShotMode, i4OpenId, isZsd)
{
    m_frameNum = property_get_int32("debug.collect.num", 3);
    mCurrentReqNum = 0;
    mbUsedLatest = false;
    MY_LOGD("Data collect frame number(%d)", m_frameNum);

    // lambda to create a CpuCtrl instance, and initialize it
    auto createCputCtrl = []()->CpuCtrl*
    {
        auto c = CpuCtrl::createInstance();
        if ( __builtin_expect( c == nullptr, false )) {
            CAM_LOGW("create CpuCtrl failed");
            return nullptr;
        }

        // init CpuCtrl
        if ( ! c->init() ) {
            c->destroyInstance();
            CAM_LOGW("init CpuCtrl failed");
            return nullptr;
        }

        return c;
    };

    // lambda to delete the CpuCtrl instance
    auto cpuCtrlDestuctor = [](CpuCtrl* c)->void
    {
        if ( __builtin_expect( c != nullptr, true )) {
            c->uninit();
            c->destroyInstance();
        }
    };

    // create CpuCtrl
    mCpuCtrl = decltype(mCpuCtrl)(
            createCputCtrl(),
            cpuCtrlDestuctor
            );
}


/******************************************************************************
 *
 ******************************************************************************/
CollectShot::
~CollectShot()
{
    MY_LOGD("~CollectShot() +");

    this->waitUntilDrained();


    MY_LOGD("isZsdMode:%d", mbZsdFlow);

    if( mResultMetadataSetMap.size() > 0 )
    {
        int n = mResultMetadataSetMap.size();
        for(int i=0; i<n; i++)
        {
            MY_LOGW("requestNo(%d) doesn't clear before SmartShot destroyed",mResultMetadataSetMap.keyAt(i));
            mResultMetadataSetMap.editValueAt(i).selectorGetBufs.clear();
        }
    }
    mResultMetadataSetMap.clear();

    MY_LOGD("~CollectShot() -");
}


/******************************************************************************
 *
 ******************************************************************************/

bool
CollectShot::
onCmd_capture()
{
    CAM_TRACE_NAME("SmartShot onCmd_capture");

    // boost CPU for a second
    if (mCpuCtrl.get()) {
        //MY_LOGD3("cpuPerformanceMode CPU for a second");
        mCpuCtrl->cpuPerformanceMode(4);
    }

    mu4Scenario = mShotParam.muSensorMode;

    HwInfoHelper helper(getOpenId());
    //if (mu4Scenario == SENSOR_SCENARIO_ID_UNNAMED_START)
    {
        if (helper.get4CellSensorSupported()) {
            MUINT32 u4RawFmtType = 0;
            if(helper.getSensorRawFmtType(u4RawFmtType)) {
                MY_LOGD("4 cell sensor raw format: %d", u4RawFmtType);
                if (u4RawFmtType == SENSOR_RAW_4CELL ||
                    u4RawFmtType == SENSOR_RAW_4CELL_BAYER) {
                    // sw 4 cell remosaic sensor, not support full capture sensor mode (mosaic pattern raw)
                    // use preview sensor mode raw (bayer pattern)
                    mu4Scenario = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
                }
                else if (u4RawFmtType == SENSOR_RAW_4CELL_HW_BAYER) {
                    // hw 4 cell remosaic sensor, support full capture sensor mode (bayer pattern raw)
                    mu4Scenario = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
                }
                else {
                    MY_LOGW("not 4 cell raw format: %d", u4RawFmtType);
                    mu4Scenario = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
                }
            }
            else {
                MY_LOGW("unknow raw format, getSensorRawFmtType() fail!");
                mu4Scenario = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
            }
        }
    }
    MY_LOGD("scenario(%d), sensorMode(%d)", mu4Scenario, mShotParam.muSensorMode);

    /* monitor this scope, if exited, mark as capture done */
    std::shared_ptr<void> __capture_state = std::shared_ptr<void>(nullptr, [this](void*) mutable {
        std::unique_lock<std::mutex> lk(mShotLock);
        mShotState = SHOTSTATE_DONE;
        mShotCond.notify_all();
    });

    /* check cpature state */
    bool bCancel = [this]() -> bool {
                std::unique_lock<std::mutex> lk(mShotLock);
                if (mShotState == SHOTSTATE_CANCEL) {
                    return true;
                }
                else {
                    mShotState = SHOTSTATE_CAPTURE;
                    return false;
                }
            }();

    if (bCancel) {
        MY_LOGD("canceled capture");
        return true;
    }

    if (supportBackgroundService()) {
        /* add a sync token if using BackgroundService */
        IMetadata::setEntry<MINT64>(
                &mShotParam.mHalSetting,
                MTK_PLUGIN_DETACT_JOB_SYNC_TOKEN,
                getInstanceId() );

        /* start capturing, add token to job manager */
        IDetachJobManager::getInstance()->registerToken( getInstanceId(), LOG_TAG );
    }

     // pause preview, if using ZSD selector
    std::shared_ptr<void> _preview_controller = pausePreview();

#if (MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT == 0)
    if ( createNewPipeline() )
#endif
    {
        MINT32 runtimeAllocateCount = 0;
        beginCapture( runtimeAllocateCount );
        /* if usingBackgroundService, create an unique VendorManager */
        if (supportBackgroundService()) {
            genVendorManagerId(); // generate VendorManager ID.
            mpManager = IVendorManager::createInstance(LOG_TAG, getOpenId());
            NSVendorManager::add(getVendorManagerId(), mpManager.get());
        }
        else {
            mpManager = NSVendorManager::get(getOpenId());
        }
        CHECK_OBJECT(mpManager);
        //
        if (!constructCapturePipeline()) {
            MY_LOGW("construct capture pipeline fail");
            return false;
        }
    }

    auto pPipeline = mpPipeline;
#if MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT
    if (pPipeline.get()) {
        MINT32 frameNo = 0;
        pPipeline->getFrameNo(frameNo);
        MY_LOGD("update pipeline frameNo:%d, mCapReqNo:%d", frameNo, mCapReqNo);
        mCapReqNo = frameNo;
    }
#endif

    //
    plugin::InputInfo inputInfo;
    MINT64 mode = [ this ](MINT32 const type) -> MINT64 {
        switch(type)
        {
            case eShotMode_ZsdHdrShot:
            case eShotMode_HdrShot: {
                return MTK_PLUGIN_MODE_HDR;
            }
        }
        MY_LOGW("no mapped vendor for type %d", type);
        return MTK_PLUGIN_MODE_COMBINATION;
    }(getShotMode());

    // set Input
    if(!makeVendorCombination(inputInfo, mode)){
        MY_LOGE("make vendor combination failed!");
        return false;
    }

    inputInfo.appCtrl      = mShotParam.mAppSetting;
    inputInfo.halCtrl      = mShotParam.mHalSetting;
    //
    inputInfo.fullRaw      = mpInfo_FullRaw;
    inputInfo.resizedRaw   = mpInfo_ResizedRaw;
    inputInfo.lcsoRaw      = mpInfo_LcsoRaw;
    //
    inputInfo.fullRaw      = mpInfo_FullRaw;
    inputInfo.resizedRaw   = mpInfo_ResizedRaw;
    inputInfo.lcsoRaw      = mpInfo_LcsoRaw;
    inputInfo.jpegYuv      = mpInfo_Yuv;
    inputInfo.thumbnailYuv = mpInfo_YuvThumbnail;
    inputInfo.postview     = mpInfo_YuvPostview;
    inputInfo.jpeg         = mpInfo_Jpeg;

    plugin::OutputInfo outputInfo;
    mpManager->get(plugin::CALLER_SHOT_SMART, inputInfo, outputInfo);
    // if not ZSD, removes delay frame if necessary
    if ( ! mbZsdFlow && outputInfo.frameCount > m_frameNum) {
        int _removesCnt = outputInfo.frameCount - m_frameNum;
        // removes delay frame
        for ( int i = 0; i < _removesCnt; i++)
            outputInfo.settings.pop();
        outputInfo.frameCount = m_frameNum;
    }
    //
    plugin::VendorInfo vInfo;
    vInfo.vendorMode = mode;
    //
    plugin::InputSetting inputSetting;
    inputSetting.pMsgCb = this;
    for ( int i = 0; i < outputInfo.frameCount; ++i ) {
        plugin::FrameInfo info;
        info.frameNo = mCapReqNo + i;
        info.curAppControl = mShotParam.mAppSetting;
        info.curHalControl = mShotParam.mHalSetting;
        info.curAppControl += outputInfo.settings[i].setting.appMeta;
        info.curHalControl += outputInfo.settings[i].setting.halMeta;
        info.vVendorInfo.add(mode, vInfo);
        inputSetting.vFrame.push_back(info);
        MY_LOGD("request frameNo:%d", info.frameNo);
    }
    // update p2 control
    updateSetting(outputInfo, inputSetting);
    //
    mpManager->set(plugin::CALLER_SHOT_SMART, inputSetting);
    //
    MINT32 shotCount = inputSetting.vFrame.size();
    Vector<MINT32> vRequestNo;
    if ( mbZsdFlow ) {
        // submit to zsd preview pipeline
        Vector<SettingSet> vSettings;
        for ( size_t i = 0; i < shotCount; ++i ) {
            SettingSet s;
            s.appSetting = inputSetting.vFrame[i].curAppControl;
            s.halSetting = inputSetting.vFrame[i].curHalControl;
            vSettings.push_back(s);
        }
        applyRawBufferSettings(vSettings, inputSetting.vFrame.size(), vRequestNo);
    }

    _preview_controller = nullptr;

    // get multiple raw buffer and send to capture pipeline
    for ( MINT32 i = 0; i < shotCount; ++i ) {
        IMetadata halSetting = mShotParam.mHalSetting;
        if ( mbZsdFlow ) {
            // get Selector Data (buffer & metadata)
            status_t status = OK;
            android::sp<IImageBuffer> pBuf = NULL; // full raw buffer
            IMetadata selectorAppMetadata; // app setting for this raw buffer. Ex.3A infomation
            status = getSelectorData(
                        selectorAppMetadata,
                        halSetting,
                        pBuf,
                        false
                    );

            //
            if(CC_UNLIKELY(mCurrentReqNum != vRequestNo[i])) {
                MY_LOGW("CurrentReq not match(%d, %d)", mCurrentReqNum, vRequestNo[i]);
                //Clear capture pipeline raw buffers
                {
                    Mutex::Autolock _l(mResultMetadataSetLock);
                    auto rvBufferSet = mResultMetadataSetMap.editValueFor(mCapReqNo).selectorGetBufs;

                    for ( size_t i = 0; i < rvBufferSet.size(); ++i ) {
                        sp<CallbackBufferPool> pPool = mpCallbackHandler->queryBufferPool(rvBufferSet[i].id);
                        pPool->flush();
                    }
                }

                //Return previous selector buffer
                returnSelectedBuffers();

                //Get the latest one
                status = getSelectorData(
                        selectorAppMetadata,
                        halSetting,
                        pBuf,
                        true //Use the latest
                    );

                // indicate that this request is used for error handling
                IMetadata::setEntry<MUINT8>(
                        &halSetting, MTK_HAL_REQUEST_ERROR_FRAME, 1);
            }
            else {
                MY_LOGD("We get buffer(%d)", mCurrentReqNum);
            }
            //
            {
                Mutex::Autolock _l(mResultMetadataSetLock);
                mResultMetadataSetMap.editValueFor(mCapReqNo).selectorAppMetadata = selectorAppMetadata;
            }
        } else {
            halSetting += inputSetting.vFrame[i].curHalControl;
            IMetadata::IEntry entry(MTK_HAL_REQUEST_SENSOR_SIZE);
            entry.push_back(mSensorSize, Type2Type< MSize >());
            halSetting.update(entry.tag(), entry);
            {
                Mutex::Autolock _l(mResultMetadataSetLock);
                IMetadata resultAppMetadata;
                IMetadata resultHalMetadata;
                IMetadata selectorAppMetadata;
                Vector<ISelector::BufferItemSet> bufferSet;
                mResultMetadataSetMap.add(
                        static_cast<MUINT32>(mCapReqNo),
                        ResultSet_T{static_cast<MUINT32>(mCapReqNo), resultAppMetadata, resultHalMetadata, selectorAppMetadata, bufferSet, NONE_CB_DONE}
                        );
            }
        }
        // submit setting to capture pipeline
        if (OK != submitCaptureSetting(
                    i == 0,
                    inputSetting.vFrame[i].curAppControl,
                    halSetting) ) {
            MY_LOGE("Submit capture setting fail.");
            goto lbExit;
        }
        //
        mCapReqNo++;
    }
lbExit:

    // 1. wait pipeline done
    // 2. set selector back to default zsd selector
    // 3. set full raw buffer count back to normal
    endCapture();
    //
    return MTRUE;

}

/******************************************************************************
*
*******************************************************************************/
MBOOL
CollectShot::
createStreams()
{
    CAM_TRACE_CALL();
    //
    mu4Bitdepth = getShotRawBitDepth(); // ImpShot
    //
    MUINT32 const openId        = getOpenId();
    MUINT32 const sensorMode    = mu4Scenario;
    MUINT32 const bitDepth      = mu4Bitdepth;
    //
    MBOOL const bEnablePostview = (getShotMode() == eShotMode_ZsdHdrShot || getShotMode() == eShotMode_HdrShot) ? MFALSE :
                                    (mShotParam.miPostviewClientFormat != eImgFmt_UNKNOWN) ? MTRUE : MFALSE;
    MSize const postviewSize    = MSize(mShotParam.mi4PostviewWidth, mShotParam.mi4PostviewHeight);
    MINT const  postviewFmt     = static_cast<EImageFormat>(mShotParam.miPostviewClientFormat);
    MINT const yuvfmt_main      = eImgFmt_NV21;
    MINT const yuvfmt_thumbnail = eImgFmt_YUY2;
    //
    MSize const thumbnailsize = MSize(mJpegParam.mi4JpegThumbWidth, mJpegParam.mi4JpegThumbHeight);
    //
    //

    if(mbZsdFlow) {
        mpInfo_ResizedRaw = nullptr;
        Vector<sp<IImageStreamInfo>> rawInputInfos;
        mpConsumer = IResourceContainer::getInstance(getOpenId())->queryConsumer( eSTREAMID_IMAGE_PIPE_RAW_OPAQUE );
        sp<StreamBufferProvider> pConsumer = mpConsumer.promote();
        if( !pConsumer.get() ) return UNKNOWN_ERROR;
        pConsumer->querySelector()->queryCollectImageStreamInfo( rawInputInfos, MTRUE );
        for(size_t i = 0; i < rawInputInfos.size() ; i++)
        {
            MY_LOGD("rawInputInfos[%d]:%d", i, rawInputInfos[i]->getStreamId());
            if(rawInputInfos[i]->getStreamId() == eSTREAMID_IMAGE_PIPE_RAW_LCSO)
                mpInfo_LcsoRaw = rawInputInfos[i];
            else if(rawInputInfos[i]->getStreamId() == eSTREAMID_IMAGE_PIPE_RAW_OPAQUE)
                mpInfo_FullRaw = rawInputInfos[i];
            else if(rawInputInfos[i]->getStreamId() == eSTREAMID_IMAGE_PIPE_RAW_RESIZER)
                mpInfo_ResizedRaw = rawInputInfos[i];
        }
    }
    else {
        HwInfoHelper helper(openId);
        // Fullsize Raw
        {
            if( ! helper.updateInfos() ) {
                MY_LOGE("cannot properly update infos");
                return MFALSE;
            }
            //
            if( ! helper.getSensorSize( sensorMode, mSensorSize) ||
                ! helper.getSensorFps( sensorMode, mSensorFps)   ||
                ! helper.queryPixelMode( sensorMode, mSensorFps, mPixelMode) ){
                MY_LOGE("cannot get params about sensor");
                return MFALSE;
            }
            //
            MSize size = mSensorSize;
            MINT format;
            size_t stride;
            MBOOL useUFOfmt = MTRUE;
            if (getShotMode() == eShotMode_EngShot)
            {
                useUFOfmt = MFALSE;
            }
            MUINT const usage = 0; //not necessary here
            if( ! helper.getImgoFmt(bitDepth, format, useUFOfmt) ||
                    ! helper.alignPass1HwLimitation(mPixelMode, format, true, size, stride) )
            {
                MY_LOGE("wrong params about imgo");
                return MFALSE;
            }
            //
            sp<IImageStreamInfo>
                pStreamInfo = createRawImageStreamInfo(
                        "SmartShot:Fullraw",
                        eSTREAMID_IMAGE_PIPE_RAW_OPAQUE,
                        eSTREAMTYPE_IMAGE_INOUT,
                        getMaxCaptureFrameNum(), 1,
                        usage, format, size, stride
                        );
            if( pStreamInfo == nullptr ) {
                return MFALSE;
            }
            mpInfo_FullRaw = pStreamInfo;
        }
        // Resized raw (for disable frontal binning use-case)
        if (helper.isRaw() && !!postviewSize)
        {
            MSize size;
            MINT format;
            size_t stride;
            MBOOL useUFOfmt = MTRUE;
            if (getShotMode() == eShotMode_EngShot)
            {
                useUFOfmt = MFALSE;
            }
            MUINT const usage =
                eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE;

            if(!helper.getRrzoFmt(bitDepth, format, useUFOfmt) ||
               !helper.alignRrzoHwLimitation(postviewSize, mSensorSize, size) ||
               !helper.alignPass1HwLimitation(mPixelMode, format, false, size, stride))
            {
                MY_LOGE("wrong parameter for RRZO");
                return MFALSE;
            }

            sp<IImageStreamInfo>
                pStreamInfo = createRawImageStreamInfo(
                    "SmartShot:ResizedRaw",
                    eSTREAMID_IMAGE_PIPE_RAW_RESIZER,
                    eSTREAMTYPE_IMAGE_INOUT,
                    getMaxCaptureFrameNum(), 1,
                    usage, format, size, stride);

            if( pStreamInfo == nullptr ) {
                MY_LOGE("create ImageStreamInfo ResizedRaw failed");
                return MFALSE;
            }

            MY_LOGD("create ImageStreamInfo ResizedRaw done");
            mpInfo_ResizedRaw = pStreamInfo;
        }
        // Lcso Raw
        if( mShotParam.mbEnableLtm )
        {
            NS3Av3::LCSO_Param lcsoParam;
            if ( auto pIspMgr = MAKE_IspMgr() ) {
                pIspMgr->queryLCSOParams(lcsoParam);
            }

            MUINT const usage = eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE; //not necessary here
            //
            sp<IImageStreamInfo>
                pStreamInfo = createRawImageStreamInfo(
                        "SmartShot:LCSraw",
                        eSTREAMID_IMAGE_PIPE_RAW_LCSO,
                        eSTREAMTYPE_IMAGE_INOUT,
                        getMaxCaptureFrameNum(), 1,
                        usage, lcsoParam.format, lcsoParam.size, lcsoParam.stride
                        );
            if( pStreamInfo == nullptr ) {
                return MFALSE;
            }
            mpInfo_LcsoRaw = pStreamInfo;
        }
    }

    /* Gives the image buffer count for DualPhase support. In DualPhase case, the shot instance
     * will be re-used hence we must need double (or triple) image buffers */
    int numPostviewYuv  = 2;
    int numYuvJpeg      = 2;
    int numThumbnailYuv = 3;
    int numJpeg         = 1;

    /* However, for BackgroundService case, we don't need duplicated image buffers because
     * we always create new shot instance every taking picture. It means the pipeline
     * will be independent hence we don't need duplicated image buffers here */
    if (supportBackgroundService()) {
        numPostviewYuv  = 1;
        numYuvJpeg      = 1;
        numThumbnailYuv = 1;
        numJpeg         = 1;
    }

    //
    // postview YUV
    if (bEnablePostview)
    {
        MSize size        = postviewSize;
        MINT format       = postviewFmt;
        MUINT const usage = 0; //not necessary here
        MUINT32 transform = 0; //not support
        sp<IImageStreamInfo>
            pStreamInfo = createImageStreamInfo(
                    "SmartShot:Postview",
                    eSTREAMID_IMAGE_PIPE_YUV_00,
                    eSTREAMTYPE_IMAGE_INOUT,
                    numPostviewYuv, 0,
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
    {
        MSize size        = mJpegsize;
        MINT format       = yuvfmt_main;
        MUINT const usage = 0; //not necessary here
        MUINT32 transform = mShotParam.mu4Transform;
        sp<IImageStreamInfo>
            pStreamInfo = createImageStreamInfo(
                    "SmartShot:MainYuv",
                    eSTREAMID_IMAGE_PIPE_YUV_JPEG,
                    eSTREAMTYPE_IMAGE_INOUT,
                    numYuvJpeg, 0,
                    usage, format, size, transform,
                    MSize(16, 16)
                    );
        if( pStreamInfo == nullptr ) {
            return BAD_VALUE;
        }
        //
        mpInfo_Yuv = pStreamInfo;
    }
    //
    // Thumbnail Yuv
    {
        MSize size        = thumbnailsize;
        MINT format       = yuvfmt_thumbnail;
        MUINT const usage = 0; //not necessary here
        MUINT32 transform = 0;
        sp<IImageStreamInfo>
            pStreamInfo = createImageStreamInfo(
                    "SmartShot:ThumbnailYuv",
                    eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL,
                    eSTREAMTYPE_IMAGE_INOUT,
                    numThumbnailYuv, 0,
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
    {
        MSize size        = mJpegsize;
        MINT format       = eImgFmt_BLOB;
        MUINT const usage = 0; //not necessary here
        MUINT32 transform = 0;
        sp<IImageStreamInfo>
            pStreamInfo = createImageStreamInfo(
                    "SmartShot:Jpeg",
                    eSTREAMID_IMAGE_JPEG,
                    eSTREAMTYPE_IMAGE_INOUT,
                    numJpeg, 0,
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

// ----------------------------------------------------------------------------


/******************************************************************************
*
*******************************************************************************/
status_t
CollectShot::
getSelectorData(
    IMetadata& rAppSetting,
    IMetadata& rHalSetting,
    android::sp<IImageBuffer>& pBuffer,
    bool  bResetReqRule
)
{
    CAM_TRACE_CALL();
    //
    mpConsumer = IResourceContainer::getInstance(getOpenId())->queryConsumer( eSTREAMID_IMAGE_PIPE_RAW_OPAQUE );
    sp<StreamBufferProvider> pConsumer = mpConsumer.promote();
    if(pConsumer == NULL) {
        MY_LOGE("mpConsumer is NULL!");
        return UNKNOWN_ERROR;
    }
    //
    sp< ISelector > pSelector = pConsumer->querySelector();
    if(pSelector == NULL) {
        MY_LOGE("can't find Selector in Consumer");
        return UNKNOWN_ERROR;
    }
    //
    auto pPipeline = mpPipeline;
    if(pPipeline == NULL) {
        MY_LOGE("pPipeline is nullptr");
        return UNKNOWN_ERROR;
    }

    if(bResetReqRule || mbUsedLatest) {
        Vector< MINT32 > vReqNew;
        MINT32 reqNew;

        if(!mbUsedLatest) {
            //After capture frames, it would be 0 ev
            MY_LOGD("Capture frameNum(%d)", m_frameNum);
            reqNew = mCurrentReqNum + m_frameNum;
        } else {
            reqNew = REQUEST_LATEST;
        }
        vReqNew.push_back(reqNew);
        ZsdRequestSelector *pZsdSelector = reinterpret_cast<ZsdRequestSelector*>(pSelector.get());
        pZsdSelector->setWaitRequestNo(vReqNew);
        mbUsedLatest = true;
    }

    //
    status_t status = OK;
    MINT32 rRequestNo;
    Vector<ISelector::MetaItemSet> rvResultMeta;
    Vector<ISelector::BufferItemSet> rvBufferSet;

    status = pSelector->getResult(rRequestNo, rvResultMeta, rvBufferSet);

    mCurrentReqNum = rRequestNo;

    // check
    MBOOL foundFullRaw = MFALSE;
    for ( size_t i = 0; i < rvBufferSet.size(); ++i ) {
        if (rvBufferSet[i].id == eSTREAMID_IMAGE_PIPE_RAW_OPAQUE) {
            foundFullRaw = MTRUE;
        }

        if ( foundFullRaw  ) {
            MY_LOGD("found full raw");
            break;
        }
    }

    if ( !foundFullRaw || rvBufferSet.isEmpty() ) {
        MY_LOGE("Selector get input raw buffer failed! bufferSetSize(%zu). foundFullRaw(%d)",
                rvBufferSet.size(), foundFullRaw);
        return UNKNOWN_ERROR;
    }

    if(status != OK) {
        MY_LOGW("Selector getResult Fail!,We use the latest result");
        //return UNKNOWN_ERROR;
    }

    if(rvResultMeta.size() < 2) {
        MY_LOGE("ZsdSelect getResult rvResultMeta(%zu)", rvResultMeta.size());
        return UNKNOWN_ERROR;
    }

    // get app & hal metadata
    if(    rvResultMeta.editItemAt(0).id == eSTREAMID_META_APP_DYNAMIC_P1
        && rvResultMeta.editItemAt(1).id == eSTREAMID_META_HAL_DYNAMIC_P1 )
    {
        rAppSetting  = rvResultMeta.editItemAt(0).meta;
        rHalSetting += rvResultMeta.editItemAt(1).meta;
    }
    else
    if(    rvResultMeta.editItemAt(1).id == eSTREAMID_META_APP_DYNAMIC_P1
        && rvResultMeta.editItemAt(0).id == eSTREAMID_META_HAL_DYNAMIC_P1 )
    {
        rAppSetting  = rvResultMeta.editItemAt(1).meta;
        rHalSetting += rvResultMeta.editItemAt(0).meta;
    }
    else {
        MY_LOGE("Something wrong for selector metadata.");
        return UNKNOWN_ERROR;
    }

    for ( size_t i = 0; i < rvBufferSet.size(); ++i ) {
        pBuffer = rvBufferSet[i].heap->createImageBuffer();
        //
        if(pBuffer == NULL) {
            MY_LOGE("get buffer is NULL!");
            return UNKNOWN_ERROR;
        }

       //update p1 buffer to pipeline pool
#if MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT
        sp<CallbackBufferPool> pPool = pPipeline->queryBufferPool( rvBufferSet[i].id );
#else
        sp<CallbackBufferPool> pPool = mpCallbackHandler->queryBufferPool( rvBufferSet[i].id );
#endif
        if( pPool == NULL) {
            MY_LOGE("query Pool Fail!");
            return UNKNOWN_ERROR;
        }
        pPool->addBuffer(pBuffer);
    }
    //
    {
        Mutex::Autolock _l(mResultMetadataSetLock);

        IMetadata resultAppMetadata;
        IMetadata resultHalMetadata;
        mResultMetadataSetMap.add(
                static_cast<MUINT32>(mCapReqNo),
                ResultSet_T{static_cast<MUINT32>(mCapReqNo), resultAppMetadata, resultHalMetadata, rAppSetting, rvBufferSet, NONE_CB_DONE }
                );
    }

    return OK;
}

MBOOL
CollectShot::
applyRawBufferSettings(
    Vector< SettingSet >    vSettings,
    MINT32                  shotCount,
    Vector< MINT32 >&       rvRequestNo,
    MINT32                  dummyCount,
    MINT32                  delayedCount
)
{
    MY_LOGD("Apply user's setting.");
    //
    mpConsumer = IResourceContainer::getInstance(getOpenId())->queryConsumer( eSTREAMID_IMAGE_PIPE_RAW_OPAQUE );
    sp<StreamBufferProvider> pProvider = mpConsumer.promote();
    if(pProvider == NULL) {
        MY_LOGE("pProvider is NULL!");
        return MFALSE;
    }
    //
    sp<IFeatureFlowControl> pFeatureFlowControl = IResourceContainer::getInstance(getOpenId())->queryFeatureFlowControl();
    if(pFeatureFlowControl == NULL) {
        MY_LOGE("IFeatureFlowControl is NULL!");
        return MFALSE;
    }
    //
    HwInfoHelper helper(getOpenId());
    MSize sensorSize;
    if( ! helper.updateInfos() ) {
        MY_LOGE("cannot properly update infos!");
        return MFALSE;
    }
    //
    if( ! helper.getSensorSize( mu4Scenario, sensorSize) ) {
        MY_LOGE("cannot get params about sensor!");
        return MFALSE;
    }
    MY_LOGD("sensorMode(%d), sensorSize(%d,%d)", mu4Scenario, sensorSize.w, sensorSize.h);
    //
    for ( size_t i = 0; i < vSettings.size(); ++i ) {
        IMetadata::IEntry entry(MTK_HAL_REQUEST_SENSOR_SIZE);
        entry.push_back(sensorSize, Type2Type< MSize >());
        vSettings.editItemAt(i).halSetting.update(entry.tag(), entry);
    }
    //
    BufferList           vDstStreams;

    Vector<sp<IImageStreamInfo>> rawInputInfos;
    pProvider->querySelector()->queryCollectImageStreamInfo( rawInputInfos, MTRUE );
    for( size_t i = 0; i < rawInputInfos.size() ; i++) {
        PREPARE_STREAM(vDstStreams, rawInputInfos[i]->getStreamId(),  true);
    }

    Vector<MINT32> rqNum;
    if( OK != pFeatureFlowControl->submitRequest(
                vSettings,
                vDstStreams,
                rqNum
                )
      )
    {
        MY_LOGE("submitRequest failed");
        return MFALSE;
    }

    // We separate settings and real frame count
    rvRequestNo.clear();
    if ( static_cast<MUINT32>(shotCount) < rqNum.size() ) {
        for ( MINT32 i = 0; i < shotCount; ++i )
            rvRequestNo.push_back(rqNum[i]);
    } else {
        rvRequestNo = rqNum;
    }
    //
    status_t status = OK;
    {
        mspOriSelector = pProvider->querySelector();
        sp<ZsdRequestSelector> pSelector = new ZsdRequestSelector();
        pSelector->setWaitRequestNo(rvRequestNo);
        status = pProvider->switchSelector(pSelector);
        MY_LOGD("ZsdRequestSelector");
        mSelectorUsage.store( ISelector::SelectorType_ZsdRequestSelector );
    }

    // TODO add LCSO consumer set the same Selector
    if(status != OK)
    {
        MY_LOGE("change selector Fail!");
        return MFALSE;
    }
    return MTRUE;
}

/******************************************************************************
*
*******************************************************************************/

MERROR
CollectShot::
submitCaptureSetting(
    MBOOL mainFrame,
    IMetadata appSetting,
    IMetadata halSetting
)
{
    MY_LOGD("current submit requestNo:%d", mCapReqNo);
    ILegacyPipeline::ResultSet resultSet;
    if ( mbZsdFlow ){
        Mutex::Autolock _l(mResultMetadataSetLock);
        resultSet.add(eSTREAMID_META_APP_DYNAMIC_P1, mResultMetadataSetMap.editValueFor(mCapReqNo).selectorAppMetadata);
    }
    auto pPipeline = mpPipeline;
    if( !pPipeline.get() )
    {
        MY_LOGW("get pPipeline fail");
        return UNKNOWN_ERROR;
    }
    //
    {
        MBOOL expResult, isoResult;
        MINT64 expVal;
        MINT32 isoVal;

        isoResult = IMetadata::getEntry<MINT32>(const_cast<IMetadata*>(&appSetting), MTK_SENSOR_SENSITIVITY, isoVal);
        expResult = IMetadata::getEntry<MINT64>(const_cast<IMetadata*>(&appSetting), MTK_SENSOR_EXPOSURE_TIME, expVal); // ms->us

        MY_LOGD("%s:=========================", __FUNCTION__);
        MY_LOGD("%s: App metadata", __FUNCTION__);
        MY_LOGD("%s: Get Iso -> %d, iso->: %d", __FUNCTION__, isoResult, isoVal);
        MY_LOGD("%s: Get Exp -> %d, exp->: %" PRId64 "", __FUNCTION__, expResult, expVal);
        MY_LOGD("%s:=========================", __FUNCTION__);
    }

    {
        BufferList vDstStreams;
        if ( mainFrame ) {
            if( mpInfo_Jpeg.get() ) {
                PREPARE_STREAM(vDstStreams, eSTREAMID_IMAGE_JPEG, false);
            }
            if( mpInfo_Yuv.get() ) {
                PREPARE_STREAM(vDstStreams, eSTREAMID_IMAGE_PIPE_YUV_JPEG, false);
            }
            if( mpInfo_YuvThumbnail.get() ) {
                PREPARE_STREAM(vDstStreams, eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL, false);
            }
            if( mpInfo_YuvPostview.get() ) {
                PREPARE_STREAM(vDstStreams, eSTREAMID_IMAGE_PIPE_YUV_00, false);
            }
        }

        if ( !mbZsdFlow ) {
            PREPARE_STREAM(vDstStreams, eSTREAMID_IMAGE_PIPE_RAW_OPAQUE, false);
            if ( mShotParam.mbEnableLtm )
                PREPARE_STREAM(vDstStreams, eSTREAMID_IMAGE_PIPE_RAW_LCSO, false);
        } else {
            Vector<sp<IImageStreamInfo>> rawInputInfos;
            mpConsumer = IResourceContainer::getInstance(getOpenId())->queryConsumer( eSTREAMID_IMAGE_PIPE_RAW_OPAQUE );
            sp<StreamBufferProvider> pConsumer = mpConsumer.promote();
            if( !pConsumer.get() ) return UNKNOWN_ERROR;
            pConsumer->querySelector()->queryCollectImageStreamInfo( rawInputInfos );
            for( size_t i = 0; i < rawInputInfos.size() ; i++) {
                PREPARE_STREAM(vDstStreams, rawInputInfos[i]->getStreamId(),  false);
            }
        }
        //
        if( OK != pPipeline->submitRequest(
                    mCapReqNo,
                    appSetting,
                    halSetting,
                    vDstStreams,
                    &resultSet
                    )
          )
        {
            MY_LOGE("submitRequest failed");
            return UNKNOWN_ERROR;
        }
    }

    return OK;
}

/******************************************************************************
*
*******************************************************************************/
#if MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT
MBOOL
CollectShot::
createPipeline()
{
    CAM_TRACE_CALL();
    FUNCTION_SCOPE;

    ICommonCapturePipeline::PipelineConfig pipelineConfig;
    pipelineConfig.userName = LOG_TAG;
    pipelineConfig.openId = getOpenId();
    pipelineConfig.isZsdMode = mbZsdFlow;

    // LegacyPipelineBuilder::ConfigParams
    pipelineConfig.LPBConfigParams.mode = getLegacyPipelineMode();
    pipelineConfig.LPBConfigParams.enableEIS = MFALSE;
    pipelineConfig.LPBConfigParams.enableLCS = mShotParam.mbEnableLtm;
    pipelineConfig.LPBConfigParams.pluginUser = plugin::CALLER_SHOT_SMART;

    if ( mbZsdFlow ) {
        Vector<sp<IImageStreamInfo>> rawInputInfos;
        sp<StreamBufferProvider> pConsumer = mpConsumer.promote();
        if( !pConsumer.get() ) return UNKNOWN_ERROR;
        pConsumer->querySelector()->queryCollectImageStreamInfo( rawInputInfos, MTRUE );
        for(size_t i = 0; i < rawInputInfos.size() ; i++)
        {
            if(rawInputInfos[i]->getStreamId() == eSTREAMID_IMAGE_PIPE_RAW_LCSO)
                pipelineConfig.LPBConfigParams.enableLCS = MTRUE;
        }
    }
    //
    HwInfoHelper helper(getOpenId());
    if( ! helper.updateInfos() ) {
        MY_LOGE("cannot properly update infos");
        return BAD_VALUE;
    }
    //
    if (helper.getDualPDAFSupported(mu4Scenario))
    {
        pipelineConfig.LPBConfigParams.enableDualPD = MTRUE;
    }
    //
    MUINT32 pipeBit;
    MINT ImgFmt = eImgFmt_BAYER12;
    if( helper.getLpModeSupportBitDepthFormat(ImgFmt, pipeBit) )
    {
        using namespace NSCam::NSIoPipe::NSCamIOPipe;
        if(pipeBit & CAM_Pipeline_14BITS)
        {
            pipelineConfig.LPBConfigParams.pipeBit = CAM_Pipeline_14BITS;
        }
        else
        {
            pipelineConfig.LPBConfigParams.pipeBit = CAM_Pipeline_12BITS;
        }
    }
    //
    // provide pCallbackListener
    pipelineConfig.pCallbackListener = this;

    if (!mbZsdFlow) {
        // query VHDR sensor mode
        MUINT32 vhdrMode = SENSOR_VHDR_MODE_NONE;
        sp<IParamsManager> pParamsMgr = mShotParam.mpParamsMgr.promote();
        if (CC_UNLIKELY( pParamsMgr.get() == nullptr )) {
            MY_LOGE("cannot promote IParamsManager, use SENSOR_VHDR_MODE_NONE as default");
        }
        else {
            // query from feature table to decide whether
            // single-frame HDR or multi-frame HDR capture is supported
            bool isSingleFrameCaptureHDR = pParamsMgr->getSingleFrameCaptureHDR();
            // query current VHDR mode
            uint32_t __vhdrMode = pParamsMgr->getVHdr();
            if (isSingleFrameCaptureHDR) {
                switch (__vhdrMode) {
                case SENSOR_VHDR_MODE_ZVHDR:
                    vhdrMode = SENSOR_VHDR_MODE_ZVHDR;
                    break;
                case SENSOR_VHDR_MODE_IVHDR:
                    vhdrMode = SENSOR_VHDR_MODE_IVHDR;
                    break;
                default:;
                }
            }
        }

        // provide IScenarioControl::ControlParam
        pipelineConfig.sensorParam.mode = mu4Scenario;
        pipelineConfig.sensorParam.rawType = 0x0000;
        pipelineConfig.sensorParam.size = mSensorSize;
        pipelineConfig.sensorParam.fps = (MUINT)mSensorFps;
        pipelineConfig.sensorParam.pixelMode = mPixelMode;
        pipelineConfig.sensorParam.vhdrMode = vhdrMode;

        // provide IScenarioControl::ControlParam
        pipelineConfig.scenarioControlParam.scenario   = IScenarioControl::Scenario_Capture;
        pipelineConfig.scenarioControlParam.sensorSize = mSensorSize;
        pipelineConfig.scenarioControlParam.sensorFps  = mSensorFps;
        if(pipelineConfig.LPBConfigParams.enableDualPD) {
            FEATURE_CFG_ENABLE_MASK(pipelineConfig.scenarioControlParam.featureFlag,IScenarioControl::FEATURE_DUAL_PD);
        }
    }

    // provide sp<IImageStreamInfo>
    // fullraw
    if( mpInfo_FullRaw.get() )
    {
        pipelineConfig.pInfo_FullRaw = mpInfo_FullRaw;
    }
    // resized raw
    if( mpInfo_ResizedRaw.get() )
    {
        pipelineConfig.pInfo_ResizedRaw = mpInfo_ResizedRaw;
    }
    // resized raw
    if( mpInfo_LcsoRaw.get() )
    {
        pipelineConfig.pInfo_LcsoRaw = mpInfo_LcsoRaw;
    }
    // jpeg main yuv
    if( mpInfo_Yuv.get() )
    {
        pipelineConfig.pInfo_Yuv = mpInfo_Yuv;
    }
    // postview yuv
    if( mpInfo_YuvPostview.get() )
    {
        pipelineConfig.pInfo_YuvPostview = mpInfo_YuvPostview;
    }
    // jpeg thumbnail yuv
    if( mpInfo_YuvThumbnail.get() )
    {
        pipelineConfig.pInfo_YuvThumbnail = mpInfo_YuvThumbnail;
    }
    // jpeg
    if( mpInfo_Jpeg.get() )
    {
        pipelineConfig.pInfo_Jpeg = mpInfo_Jpeg;
    }

    // creat and query common capture pipeline
    mpPipeline = ICommonCapturePipeline::queryCommonCapturePipeline(pipelineConfig);
    CHECK_OBJECT(mpPipeline);

    return MTRUE;
}

#else // not MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT
MBOOL
CollectShot::
createPipeline()
{
    CAM_TRACE_CALL();
    FUNC_START;
    LegacyPipelineBuilder::ConfigParams LPBConfigParams;
    LPBConfigParams.mode = getLegacyPipelineMode();
    LPBConfigParams.enableEIS = MFALSE;
    LPBConfigParams.enableLCS = mShotParam.mbEnableLtm;
    LPBConfigParams.pluginUser = plugin::CALLER_SHOT_SMART;
    LPBConfigParams.vendorManagerId = getVendorManagerId();
    if ( mbZsdFlow ) {
        Vector<sp<IImageStreamInfo>> rawInputInfos;
        mpConsumer = IResourceContainer::getInstance(getOpenId())->queryConsumer( eSTREAMID_IMAGE_PIPE_RAW_OPAQUE );
        sp<StreamBufferProvider> pConsumer = mpConsumer.promote();
        if( !pConsumer.get() ) return UNKNOWN_ERROR;
        pConsumer->querySelector()->queryCollectImageStreamInfo( rawInputInfos, MTRUE );
        for(size_t i = 0; i < rawInputInfos.size() ; i++)
        {
            if(rawInputInfos[i]->getStreamId() == eSTREAMID_IMAGE_PIPE_RAW_LCSO)
                LPBConfigParams.enableLCS = MTRUE;
        }
    }
    //
    HwInfoHelper helper(getOpenId());
    if( ! helper.updateInfos() ) {
        MY_LOGE("cannot properly update infos");
        return BAD_VALUE;
    }
    //
    if (helper.getDualPDAFSupported(mu4Scenario))
    {
        LPBConfigParams.enableDualPD = MTRUE;
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
    sp<LegacyPipelineBuilder> pBuilder = LegacyPipelineBuilder::createInstance(
                                            getOpenId(),
                                            "SmartShot",
                                            LPBConfigParams);
    CHECK_OBJECT(pBuilder);

    mpImageCallback = new ImageCallback(this, 0);
    CHECK_OBJECT(mpImageCallback);

    sp<BufferCallbackHandler> pCallbackHandler = new BufferCallbackHandler(getOpenId());
    CHECK_OBJECT(pCallbackHandler);
    pCallbackHandler->setImageCallback(mpImageCallback);
    mpCallbackHandler = pCallbackHandler;

    sp<StreamBufferProviderFactory> pFactory =
                StreamBufferProviderFactory::createInstance();
    //
    Vector<PipelineImageParam> vImageParam;
    //
    {
        MY_LOGD("createPipeline for smart shot");
        if ( mbZsdFlow ) {
            Vector<sp<IImageStreamInfo>> rawInputInfos;
            mpConsumer = IResourceContainer::getInstance(getOpenId())->queryConsumer( eSTREAMID_IMAGE_PIPE_RAW_OPAQUE );
            sp<StreamBufferProvider> pConsumer = mpConsumer.promote();
            if( !pConsumer.get() ) return UNKNOWN_ERROR;
            pConsumer->querySelector()->queryCollectImageStreamInfo( rawInputInfos, MTRUE );
            //
            Vector<PipelineImageParam> vImgSrcParams;
            sp<IImageStreamInfo> pStreamInfo = NULL;
            for(size_t i = 0; i < rawInputInfos.size() ; i++) {
                pStreamInfo = rawInputInfos[i];
                MY_LOGD("src config for streamId: %#" PRIx64"", pStreamInfo->getStreamId());

                sp<CallbackBufferPool> pPool = new CallbackBufferPool(pStreamInfo);
                //pPool->addBuffer(pBuf);
                mpCallbackHandler->setBufferPool(pPool);
                //
                pFactory->setImageStreamInfo(pStreamInfo);
                pFactory->setUsersPool(
                        mpCallbackHandler->queryBufferPool(pStreamInfo->getStreamId())
                    );
                PipelineImageParam imgParam = {
                    .pInfo     = pStreamInfo,
                    .pProvider = pFactory->create(false),
                    .usage     = 0
                };
                vImgSrcParams.push_back(imgParam);
            }
            //
            if( OK != pBuilder->setSrc(vImgSrcParams) ) {
                MY_LOGE("setSrc failed");
                return MFALSE;
            //
            }
        } else {

            // query VHDR sensor mode
            MUINT32 vhdrMode = SENSOR_VHDR_MODE_NONE;
            sp<IParamsManager> pParamsMgr = mShotParam.mpParamsMgr.promote();
            if (CC_UNLIKELY( pParamsMgr.get() == nullptr )) {
                MY_LOGE("cannot promote IParamsManager, use SENSOR_VHDR_MODE_NONE as default");
            }
            else {
                // query from feature table to decide whether
                // single-frame HDR or multi-frame HDR capture is supported
                bool isSingleFrameCaptureHDR = pParamsMgr->getSingleFrameCaptureHDR();
                // query current VHDR mode
                uint32_t __vhdrMode = pParamsMgr->getVHdr();
                if (isSingleFrameCaptureHDR) {
                    switch (__vhdrMode) {
                    case SENSOR_VHDR_MODE_ZVHDR:
                        vhdrMode = SENSOR_VHDR_MODE_ZVHDR;
                        break;
                    case SENSOR_VHDR_MODE_IVHDR:
                        vhdrMode = SENSOR_VHDR_MODE_IVHDR;
                        break;
                    default:;
                    }
                }
            }

            MY_LOGD("vhdrMode(0x%X)", vhdrMode);

            PipelineSensorParam sensorParam(
            /*.mode      = */mu4Scenario,
            /*.rawType   = */0x0001,
            /*.size      = */mSensorSize,
            /*.fps       = */(MUINT)mSensorFps,
            /*.pixelMode = */mPixelMode,
            /*.vhdrMode  = */vhdrMode
            );
            //
            if( OK != pBuilder->setSrc(sensorParam) ) {
                MY_LOGE("setSrc failed");
                return MFALSE;
            }
            //
            sp<IScenarioControl> pScenarioCtrl = IScenarioControl::create(getOpenId());
            if( pScenarioCtrl.get() == NULL )
            {
                MY_LOGE("get Scenario Control fail");
                return MFALSE;
            }
            IScenarioControl::ControlParam param;
            param.scenario   = IScenarioControl::Scenario_Capture;
            param.sensorSize = mSensorSize;
            param.sensorFps  = mSensorFps;
            if(LPBConfigParams.enableDualPD)
                FEATURE_CFG_ENABLE_MASK(param.featureFlag,IScenarioControl::FEATURE_DUAL_PD);

            pScenarioCtrl->enterScenario(param);
            pBuilder->setScenarioControl(pScenarioCtrl);
            //
            // fullraw
            if( mpInfo_FullRaw.get() )
            {
                sp<IImageStreamInfo> pStreamInfo = mpInfo_FullRaw;
                //
                sp<CallbackBufferPool> pPool = new CallbackBufferPool(pStreamInfo);
                pPool->allocateBuffer(
                                  pStreamInfo->getStreamName(),
                                  pStreamInfo->getMaxBufNum(),
                                  pStreamInfo->getMinInitBufNum()
                                  );
                mpCallbackHandler->setBufferPool(pPool);
                //
                pFactory->setImageStreamInfo(pStreamInfo);
                pFactory->setUsersPool(
                            mpCallbackHandler->queryBufferPool(pStreamInfo->getStreamId())
                        );
                //
                PipelineImageParam imgParam = {
                    .pInfo     = pStreamInfo,
                    .pProvider = pFactory->create(false),
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
            // resized raw
            if( mpInfo_LcsoRaw.get() )
            {
                sp<IImageStreamInfo> pStreamInfo = mpInfo_LcsoRaw;
                //
                PipelineImageParam imgParam = {
                    .pInfo     = pStreamInfo,
                    .pProvider = nullptr,
                    .usage     = 0
                };
                vImageParam.push_back(imgParam);
            }
        }

    }
    //
    {
        if( mpInfo_Yuv.get() )
        {
            CAM_TRACE_NAME("set mpInfo_Yuv");
            sp<IImageStreamInfo> pStreamInfo = mpInfo_Yuv;
            //
            sp<CallbackBufferPool> pPool = new CallbackBufferPool(pStreamInfo);
            {
                CAM_TRACE_NAME("alloc_buf");
                pPool->allocateBuffer(
                                  pStreamInfo->getStreamName(),
                                  pStreamInfo->getMaxBufNum(),
                                  pStreamInfo->getMinInitBufNum()
                                  );
                mpCallbackHandler->setBufferPool(pPool);
            }
            //
            pFactory->setImageStreamInfo(pStreamInfo);
            pFactory->setUsersPool(
                        mpCallbackHandler->queryBufferPool(pStreamInfo->getStreamId())
                    );
            //
            PipelineImageParam imgParam = {
                .pInfo     = pStreamInfo,
                .pProvider = pFactory->create(false),
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
            sp<CallbackBufferPool> pPool = new CallbackBufferPool(pStreamInfo);
            {
                CAM_TRACE_NAME("alloc_buf");
                pPool->allocateBuffer(
                                  pStreamInfo->getStreamName(),
                                  pStreamInfo->getMaxBufNum(),
                                  pStreamInfo->getMinInitBufNum()
                                  );
            }
            mpCallbackHandler->setBufferPool(pPool);
            //
            pFactory->setImageStreamInfo(pStreamInfo);
            pFactory->setUsersPool(
                        mpCallbackHandler->queryBufferPool(pStreamInfo->getStreamId())
                    );
            //
            PipelineImageParam imgParam = {
                .pInfo     = pStreamInfo,
                .pProvider = pFactory->create(false),
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
            sp<CallbackBufferPool> pPool = new CallbackBufferPool(pStreamInfo);
            {
                CAM_TRACE_NAME("alloc_buf");
                pPool->allocateBuffer(
                                  pStreamInfo->getStreamName(),
                                  pStreamInfo->getMaxBufNum(),
                                  pStreamInfo->getMinInitBufNum()
                                  );
            }
            mpCallbackHandler->setBufferPool(pPool);
            //
            pFactory->setImageStreamInfo(pStreamInfo);
            pFactory->setUsersPool(
                        mpCallbackHandler->queryBufferPool(pStreamInfo->getStreamId())
                    );
            //
            PipelineImageParam imgParam = {
                .pInfo     = pStreamInfo,
                .pProvider = pFactory->create(false),
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
            sp<CallbackBufferPool> pPool = new CallbackBufferPool(pStreamInfo);
            {
                CAM_TRACE_NAME("allocateBuffer");
                pPool->allocateBuffer(
                                  pStreamInfo->getStreamName(),
                                  pStreamInfo->getMaxBufNum(),
                                  pStreamInfo->getMinInitBufNum()
                                  );
            }
            mpCallbackHandler->setBufferPool(pPool);
            //
            pFactory->setImageStreamInfo(pStreamInfo);
            pFactory->setUsersPool(
                        mpCallbackHandler->queryBufferPool(pStreamInfo->getStreamId())
                    );
            //
            PipelineImageParam imgParam = {
                .pInfo     = pStreamInfo,
                .pProvider = pFactory->create(false),
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
    mpPipeline = pBuilder->create();
    //
    FUNC_END;
    return MTRUE;

}
#endif

MVOID
CollectShot::
onMetaReceived(
        MUINT32         const requestNo,
        StreamId_T      const streamId,
        MBOOL           const errorResult,
        IMetadata       const result
        )
{
    SmartShot::onMetaReceived(requestNo, streamId, errorResult, result);
}

status_t CollectShot::
returnSelectedBuffers()
{

    // selected buffers are only available in ZSD mode
    if (!mbZsdFlow)
        return OK;

    mpConsumer = IResourceContainer::getInstance(getOpenId())->queryConsumer( eSTREAMID_IMAGE_PIPE_RAW_OPAQUE );

    sp<StreamBufferProvider> pConsumer = mpConsumer.promote();

    // return selected buffers to selector and then restore selector
    if (pConsumer.get())
    {
        // return selected buffers to selector
        sp<ISelector> selector = pConsumer->querySelector();
        if (selector.get())
        {
            Mutex::Autolock _l(mResultMetadataSetLock);
            Vector<ISelector::BufferItemSet> bufferSet;
            if(mResultMetadataSetMap.indexOfKey(mCapReqNo) >= 0) {
                bufferSet = mResultMetadataSetMap.editValueFor(mCapReqNo).selectorGetBufs;

                for(size_t i = 0; i < bufferSet.size() ; i++)
                    selector->returnBuffer(bufferSet.editItemAt(i));

                {
                    for (auto& itr : mResultMetadataSetMap.editValueFor(mCapReqNo).selectorGetBufs) {
                        itr = ISelector::BufferItemSet();
                    }
                }

                //clear result metadata
                mResultMetadataSetMap.removeItem(mCapReqNo);
           }

        }
    }

    return OK;
}
