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
#define __DEBUG // enable debug

#define LOG_TAG "MtkCam/SmartShot"
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
#include "SmartShot.h"
//
#include <mtkcam/utils/hw/CamManager.h>
using namespace NSCam::Utils;
//
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/middleware/v1/LegacyPipeline/NodeId.h>
#include <mtkcam/middleware/v1/camshot/_params.h>
//
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/ILegacyPipeline.h>
#include <mtkcam/utils/hw/HwInfoHelper.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <utils/Vector.h>
#include <sys/stat.h>
//
#include <mtkcam/aaa/IDngInfo.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/LegacyPipelineUtils.h>
#include <mtkcam/middleware/v1/LegacyPipeline/LegacyPipelineBuilder.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProviderFactory.h>

#include <mtkcam/aaa/IHal3A.h>
#include <mtkcam/pipeline/extension/MFNR.h>
#include <mtkcam/feature/utils/FeatureProfileHelper.h> //ProfileParam

#include <errno.h>

#include <limits>

// postprocess
#include <mtkcam/middleware/v1/camutils/IDetachJobManager.h>
#include <mtkcam/middleware/v1/camutils/PostProcessJobQueue.h>
#include <mtkcam/feature/effectHalBase/ICallbackClientMgr.h>

using namespace android;
using namespace NSShot::NSSmartShot;
using namespace NSCam;
using namespace NSCam::v1;
using namespace NSCam::v1::NSLegacyPipeline;
using namespace NSCamHW;
using namespace NS3Av3;
using namespace NSCam::plugin;
using namespace NSCam::Utils;

#define CHECK_OBJECT(x)  do{                                        \
    if (x == nullptr) { MY_LOGE("Null %s Object", #x); return MFALSE;} \
} while(0)

#include <cutils/properties.h>
#define DUMP_KEY  "debug.smartshot.dump"
#define DUMP_PATH "/sdcard/smartshot"
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

#define MYU_LOGE(fmt, arg...)       CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)

//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)
//

#define MY_COND(v)  __builtin_expect( mDbgLevel >= v, false )
#define MY_LOGD1(...) do { if ( MY_COND(1) ) { MY_LOGD(__VA_ARGS__); } } while(0)
#define MY_LOGD2(...) do { if ( MY_COND(2) ) { MY_LOGD(__VA_ARGS__); } } while(0)
#define MY_LOGD3(...) do { if ( MY_COND(3) ) { MY_LOGD(__VA_ARGS__); } } while(0)
//
#define FUNC_START                  MY_LOGD("+")
#define FUNC_END                    MY_LOGD("-")

// FUNCTION_SCOPE
#ifdef __DEBUG
#define FUNCTION_SCOPE      auto __scope_logger__ = create_scope_logger(" ", __FUNCTION__)
#define LOG_SCOPE(log)      auto __scope_logger__ = create_scope_logger(log, __FUNCTION__)
#include <memory>
#include <functional>
static std::shared_ptr<char> create_scope_logger(const char* log, const char* functionName)
{
    char* pText = const_cast<char*>(functionName);
    CAM_LOGD("[%s] %s + ", pText, log);
    return std::shared_ptr<char>(pText, [log](char* p){ CAM_LOGD("[%s] %s -", p, log); });
}
#else
#define FUNCTION_SCOPE          do{}while(0)
#define LOG_SCOPE               do{}while(0)
#endif

#define DEFAULT_MAX_FRAME_NUM       6

/**
 *  ZSD selector timed out in MS.
 *  If timed out, we set no need to wait AF done frame.
 */
#define DEFAULT_MAX_SELECTOR_TIMED_OUT_MS   1000

/**
 *  onCmd_cancel timed out in MS.
 *  If timed out, we think the camera is deadlock, force to assert.
 */
#define SMART_SHOT_CANCEL_TIMEOUT_MS 5000
#define AEE_ASSERT(error_log) \
        do { \
            MY_LOGE("%s: %s", __FUNCTION__, error_log); \
            *(volatile uint32_t*)(0x00000000) = 0xdeadbeef; \
        } while(0)

/******************************************************************************
 *
 ******************************************************************************/
extern
sp<IShot>
createInstance_SmartShot(
    char const*const    pszShotName,
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId
)
{
    sp<IShot>      pShot        = NULL;
    sp<SmartShot>  pImpShot     = NULL;
    //
    //  (1.1) new Implementator.
    pImpShot = new SmartShot(pszShotName, u4ShotMode, i4OpenId, false);
    if  ( pImpShot == 0 ) {
        CAM_LOGE("[%s] new SmartShot", __FUNCTION__);
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
createInstance_ZsdSmartShot(
    char const*const    pszShotName,
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId
)
{
    sp<IShot>  pShot            = NULL;
    sp<SmartShot>  pImpShot = NULL;
    //
    //  (1.1) new Implementator.
    pImpShot = new SmartShot(pszShotName, u4ShotMode, i4OpenId, true);
    if  ( pImpShot == 0 ) {
        CAM_LOGE("[%s] new SmartShot", __FUNCTION__);
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
template <typename T>
inline MBOOL
tryGetMetadata(
    IMetadata const* const pMetadata,
    MUINT32 const tag,
    T & rVal
)
{
    if( pMetadata == NULL ) {
        //MY_LOGE("pMetadata == NULL");
        return MFALSE;
    }

    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if( !entry.isEmpty() ) {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }
    return MFALSE;
}


/******************************************************************************
 *  This function is invoked when this object is firstly created.
 *  All resources can be allocated here.
 ******************************************************************************/
bool
SmartShot::
onCreate()
{
    bool ret = true;
    return ret;
}


/******************************************************************************
 *  This function is invoked when this object is ready to destryoed in the
 *  destructor. All resources must be released before this returns.
 ******************************************************************************/
void
SmartShot::
onDestroy()
{

}

/******************************************************************************
 *
 ******************************************************************************/
SmartShot::
SmartShot(
    char const*const pszShotName,
    uint32_t const   u4ShotMode,
    int32_t const    i4OpenId,
    bool const       isZsd
)
    : ImpShot(pszShotName, u4ShotMode, i4OpenId)
    , mSensorFps(0)
    , mPixelMode(0)
    , mu4Scenario(SENSOR_SCENARIO_ID_UNNAMED_START)
    , mu4Bitdepth(0)
    , mbZsdFlow(isZsd)
    //
    , mpManager(NULL)
    //
    , mpJpegPool()
    , mCapReqNo(0)
    , mLastRotation(0)
    , mLastPicFmt(0)
    , mNeedReconstruct(MFALSE)
    , mSelectorUsage(ISelector::SelectorType_ZsdRequestSelector)
    , mSelectorRequested(MTRUE)
    , mShotState(SHOTSTATE_INIT)
    , mDbgLevel(-1)
    , mFlushP1Done(false)
{
    MY_LOGD("create SmartShot %p, uid=%" PRId64 "", this, getInstanceId());

    MY_LOGD3("mbZsdFlow %d %d", mbZsdFlow, isZsd);
    mDbgLevel = ::property_get_int32("persist.mtk.camera.log_level", 0);
    mDumpFlag = ::property_get_int32(DUMP_KEY, 0);
    if( mDumpFlag ) {
        MY_LOGD3("enable dump flag 0x%x", mDumpFlag);
        MINT32 err = mkdir(DUMP_PATH, S_IRWXU | S_IRWXG | S_IRWXO);
        if(err != 0)
        {
            MY_LOGE("mkdir failed");
        }
    }
    // set default maximum frame number
    setMaxCaptureFrameNum(DEFAULT_MAX_FRAME_NUM);

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
SmartShot::
~SmartShot()
{
    MY_LOGD("~SmartShot() [+] %p, uid=%" PRId64 "", this, getInstanceId());

    this->waitUntilDrained();

    MY_LOGD3("isZsdMode:%d", mbZsdFlow);
    if (!mbZsdFlow) {
#if MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT
        ICommonCapturePipeline::removeCommonCapturePipeline(getOpenId());
#endif
    }

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

    MY_LOGD("~SmartShot() [-] %p, uid=%" PRId64 "", this, getInstanceId());
}
/******************************************************************************
 *
 ******************************************************************************/
bool
SmartShot::
sendCommand(
    uint32_t const  cmd,
    MUINTPTR const  arg1,
    uint32_t const  arg2,
    uint32_t const  arg3
)
{
    bool ret = true;
    //
    switch  (cmd)
    {
    //  This command is to reset this class. After captures and then reset,
    //  performing a new capture should work well, no matter whether previous
    //  captures failed or not.
    //
    //  Arguments:
    //          N/A
    case eCmd_reset:
        ret = onCmd_reset();
        break;

    //  This command is to perform capture.
    //
    //  Arguments:
    //          N/A
    case eCmd_capture:
        ret = onCmd_capture();
        // update last info
        updateLastInfo();
        break;

    //  This command is to perform cancel capture.
    //
    //  Arguments:
    //          N/A
    case eCmd_cancel:
        onCmd_cancel();
        break;

    case eCmd_getIsNeedNewOne:
        {
            MBOOL* pResult = reinterpret_cast<MBOOL*>(arg1);
            *pResult = isNeedToReconstruct();
        }
        break;
    //
    default:
        ret = ImpShot::sendCommand(cmd, arg1, arg2, arg3);
    }
    //
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
SmartShot::
onMsgReceived(
    MINT32 cmd, /*NOTIFY_MSG*/
    MINT32 /*arg1*/, MINT32 /*arg2*/,
    void*  /*arg3*/
)
{
    switch  (cmd)
    {
        case plugin::MSG_ON_NEXT_CAPTURE_READY:
            if (mbZsdFlow || supportFastP1Done()) this->onP2done();
        break;
        case plugin::MSG_ON_SHUTTER_CALLBACK:
            mpShotCallback->onCB_Shutter(true,0,getShotMode());
        break;
    }

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
SmartShot::
onCmd_reset()
{
    bool ret = true;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
SmartShot::
createNewPipeline()
{
    MBOOL ret = MFALSE;
    auto pPipeline = this->getPipeline();
    //
    if ( !pPipeline.get() ) ret = MTRUE;
    if ( !mpManager.get() ) ret = MTRUE;
    //
    /*if( mLastPipelineMode != getLegacyPipelineMode() )
    {
        MY_LOGD("LegacyPipelineMode changed (%d)->(%d), need to create pipeline",mLastPipelineMode,getLegacyPipelineMode());
        return MTRUE;
    }
    //
#warning "Need to use run-time change Rotation method to avoid re-create pipeline"
    if( mLastRotation != getRotation() )
    {
        MY_LOGD("Rotation changed (%d)->(%d), need to create pipeline",mLastRotation,getRotation());
        return MTRUE;
    }
    //
#warning "Need to use run-time change JpegSize method to avoid re-create pipeline"
    if( mLastJpegsize != mJpegsize )
    {
        MY_LOGD("JpegSize changed (%dx%d)->(%dx%d), need to create pipeline",
            mLastJpegsize.w,mLastJpegsize.h,
            mJpegsize.w,mJpegsize.h);
        return MTRUE;
    }*/

    MY_LOGD3("createNewPipeline %d", ret);

    return ret;
}

MBOOL
SmartShot::
isNeedToReconstruct()
{
    sp<IParamsManager> pParamsMgr = mShotParam.mpParamsMgr.promote();
    if (CC_UNLIKELY( pParamsMgr.get() == nullptr )) {
        MY_LOGE("cannot get params manager, assume need to reconstruct");
        return MTRUE;
    }

    // query picture rotation
    MUINT32 transform = pParamsMgr->getInt(CameraParameters::KEY_ROTATION);
    switch (transform) {
    case 0:     transform = 0;                   break;
    case 90:    transform = eTransform_ROT_90;   break;
    case 180:   transform = eTransform_ROT_180;  break;
    case 270:   transform = eTransform_ROT_270;  break;
    default:
        break;
    }
    // query picture size
    int iPictureWidth = 0, iPictureHeight = 0;
    pParamsMgr->getPictureSize(&iPictureWidth, &iPictureHeight);
    MY_LOGD3("lastTransform=%u, transform=%u",mLastRotation, transform);
    MY_LOGD3("lastSize=%dx%d, size=%dx%d",
            mLastJpegsize.w, mLastJpegsize.h,
            iPictureWidth, iPictureHeight);

    MSize newSize = (transform == 0 || transform == eTransform_ROT_180)
        ? MSize(iPictureWidth, iPictureHeight)
        : MSize(iPictureHeight, iPictureWidth);
    // query picture format
    int newPicFmt = eImgFmt_JPEG;
    newPicFmt = MtkCameraParameters::queryImageFormat(pParamsMgr->getStr(CameraParameters::KEY_PICTURE_FORMAT));
    MY_LOGD3("lastPicFmt=0x%X, newPicFmt=0x%X",mLastPicFmt, newPicFmt);

    if ( mLastRotation != transform )             return MTRUE;
    if ( mLastJpegsize.size() != newSize.size() ) return MTRUE;
    if ( mLastPicFmt   != newPicFmt )             return MTRUE;
    if ( mNeedReconstruct )                            return MTRUE;

    return MFALSE;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
SmartShot::
onCmd_capture()
{
    CAM_TRACE_NAME("SmartShot onCmd_capture");

    // boost CPU for a second
    if (mCpuCtrl.get()) {
        MY_LOGD3("cpuPerformanceMode CPU for a second");
        mCpuCtrl->cpuPerformanceMode(1);
    }

    mu4Scenario = mShotParam.muSensorMode;

    HwInfoHelper helper(getOpenId());
    if (mu4Scenario == SENSOR_SCENARIO_ID_UNNAMED_START) {
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
        else {
            MY_LOGD3("it's general sensor");
            mu4Scenario = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
        }
    }
    MY_LOGD("scenario(%d), sensorMode(%d)", mu4Scenario, mShotParam.muSensorMode);

    /* monitor this scope, if exited, mark as capture done */
    std::shared_ptr<void> __capture_state = std::shared_ptr<void>(nullptr, [this](void*) mutable {
        MY_LOGD("prepare to change shot state...");
        std::unique_lock<std::mutex> lk(mShotLock);
        mShotState = SHOTSTATE_DONE;
        mShotCond.notify_all();
        MY_LOGD("ShotState: SHOTSTATE_DONE");
    });

    /* check cpature state */
    bool bCancel = [this]() -> bool {
                std::unique_lock<std::mutex> lk(mShotLock);
                if (mShotState == SHOTSTATE_CANCEL) {
                    MY_LOGD3("ShotState: SHOTSTATE_CANCEL");
                    return true;
                }
                else {
                    mShotState = SHOTSTATE_CAPTURE;
                    MY_LOGD3("ShotState: SHOTSTATE_CAPTURE");
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
    std::shared_ptr<void> _preview_controller = isUseZsdBuffers()
        ? nullptr
        : pausePreview();
#if (MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT == 0)
    if ( createNewPipeline())
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

    auto pPipeline = this->getPipeline();
#if MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT
    if (pPipeline.get()) {
        MINT32 frameNo = 0;
        pPipeline->getFrameNo(frameNo);
        MY_LOGD3("update pipeline frameNo:%d, mCapReqNo:%d", frameNo, mCapReqNo);
        mCapReqNo = frameNo;
    }
#endif
    //
    plugin::InputInfo inputInfo;
    MINT64 mode = [ this ](MINT32 const type) -> MINT64 {
        switch(type)
        {
            case eShotMode_EngShot:
            case eShotMode_MfllShot:
            case eShotMode_ZsdMfllShot: {
                return MTK_PLUGIN_MODE_MFNR;
            }
            case eShotMode_ZsdHdrShot:
            case eShotMode_HdrShot: {
                return MTK_PLUGIN_MODE_HDR;
            }
        }
        MY_LOGW("no mapped vendor for type %d", type);
        return MTK_PLUGIN_MODE_COMBINATION;
    }(getShotMode());

    // update selector usage by mode
    if (mode == MTK_PLUGIN_MODE_MFNR) {
        // always uses ZsdSelector for both MFNR and AIS
        mSelectorUsage.store(ISelector::SelectorType_ZsdSelector);
        MY_LOGD("set selector usage to ZsdSelector due to MFLL or AIS");

        // check mfb mode, if it's AIS, do not using ZSD buffers
        if (mShotParam.mi4MfbMode == MTK_MFB_MODE_AIS) {
            MY_LOGD("do not use ZSD buffers due to AIS");
            setUseZsdBuffers(MFALSE);
        }
        else {
            auto forceUseZsdBuffers = ::property_get_int32(
                    "debug.mfll.fastmode", -1);
            if (CC_UNLIKELY( forceUseZsdBuffers >= 0 )) {
                MY_LOGD("force use ZSD buffers follows by value %d", forceUseZsdBuffers);
                setUseZsdBuffers(forceUseZsdBuffers == 0 ? MFALSE : MTRUE);
            }
            else {
                MY_LOGD("use ZSD buffers due to MFNR");
                setUseZsdBuffers(MTRUE);
            }
        }
    }
    else {
        mSelectorUsage.store( ISelector::SelectorType_ZsdRequestSelector );
        MY_LOGD("set selector usage to ZsdRequestSelector");
    }

    // set Input
    if(!makeVendorCombination(inputInfo, mode)){
        MY_LOGE("make vendor combination failed!");
        return false;
    }

    // update post nr settings
    MINT64 pluginMode = 0;
    for (size_t i = 0; i < inputInfo.combination.size(); ++i) {
        MY_LOGD3("in.combination[%zu]:0x%" PRIx64 ", MTK_PLUGIN_MODE_NR(0x%X)",
                 i, inputInfo.combination[i], MTK_PLUGIN_MODE_NR);
        if (MTK_PLUGIN_MODE_NR == inputInfo.combination[i]) {
            updatePostNRSetting(MTK_NR_MODE_AUTO, (mode == MTK_PLUGIN_MODE_MFNR), &mShotParam.mHalSetting);
        }
        pluginMode |= inputInfo.combination[i];
    }
    // update MTK_PLUGIN_MODE
    IMetadata::setEntry<MINT64>(&mShotParam.mHalSetting, MTK_PLUGIN_MODE, pluginMode);

    if (CC_UNLIKELY( mShotParam.mbFlashOn )) {
        MY_LOGD3("do not use ZSD buffers due to Flash MFNR");
        setUseZsdBuffers(MFALSE);
    }

    inputInfo.appCtrl      = mShotParam.mAppSetting;
    inputInfo.halCtrl      = mShotParam.mHalSetting;
    //
    inputInfo.fullRaw      = mpInfo_FullRaw;
    inputInfo.resizedRaw   = mpInfo_ResizedRaw;
    inputInfo.lcsoRaw      = mpInfo_LcsoRaw;
    inputInfo.jpegYuv      = mpInfo_Yuv;
    inputInfo.thumbnailYuv = mpInfo_YuvThumbnail;
    inputInfo.postview     = mpInfo_YuvPostview;
    inputInfo.jpeg         = mpInfo_Jpeg;
    //
    inputInfo.isZsdMode    = mbZsdFlow;
    inputInfo.sensorMode   = mu4Scenario;

    MINT32 capturedCount = 0;
    MINT32 dummyCount = 0;
    MINT32 delayedCount = 0;
    MINT32 totalShotCount = 0;
    plugin::OutputInfo outputInfo;
    plugin::InputSetting inputSetting;
    plugin::VendorInfo vInfo;
    {
        Mutex::Autolock _l(MFNRVendor::sCtrlerAccessLock);
        mpManager->registerItems(plugin::CALLER_SHOT_SMART, inputInfo);
        if (mode == MTK_PLUGIN_MODE_MFNR) {
            MetaItem meta;
            meta.setting.appMeta = mShotParam.mAppSetting;
            meta.setting.halMeta = mShotParam.mHalSetting;
            MINT32 arg1 = 0, arg2 = 0;

            MFNRVendor::ConfigParams configParams;
            configParams.isZSDMode = mbZsdFlow;
            configParams.isZHDRMode = false;
            configParams.isAutoHDR = false;
            configParams.isFlashOn = mShotParam.mbFlashOn;
            configParams.usingBackgroundService = supportBackgroundService();

            /* set exposuretime, realIso = 0,
             * use the real time setting if no MFNR controller */
            configParams.exposureTime = 0;
            configParams.realIso = 0;

            sp<IParamsManager> pParamsMgr = mShotParam.mpParamsMgr.promote();
            configParams.mfbMode = pParamsMgr->getMultFrameBlending();

            IMetadata halSetting = mShotParam.mHalSetting;
            IMetadata::getEntry<MINT32>(
                    const_cast<IMetadata*>(&halSetting),
                    MTK_PLUGIN_CUSTOM_HINT,
                    configParams.customHint);
            MY_LOGD("configParams.customHint:%d", configParams.customHint);

            mpManager->sendCommand(
                    MFNRVendor::INFO_MFNR_CONFIG_PARAM, MTK_PLUGIN_MODE_MFNR, meta, arg1, arg2,
                    reinterpret_cast<void*>(&configParams)
                    );
        }
        mpManager->get(plugin::CALLER_SHOT_SMART, inputInfo, outputInfo);
        // User defined
        vInfo.vendorMode = mode;
        if (mode == MTK_PLUGIN_MODE_MFNR)
        {
            if (inputInfo.postview.get()) {
                vInfo.appOut.push_back( inputInfo.postview->getStreamId() );
            }
            if (inputInfo.thumbnailYuv.get()) {
                vInfo.appOut.push_back( inputInfo.thumbnailYuv->getStreamId() );
            }
        }
        //
        inputSetting.pMsgCb = this;
        for ( int i = 0; i < outputInfo.frameCount; ++i ) {
            plugin::FrameInfo info;
            info.frameNo = mCapReqNo + i;
            info.curAppControl = mShotParam.mAppSetting;
            info.curHalControl = mShotParam.mHalSetting;
            info.curAppControl += outputInfo.settings[i].setting.appMeta;
            info.curHalControl += outputInfo.settings[i].setting.halMeta;
            if(i!=0)
              info.curHalControl.remove(MTK_NR_MODE);
            info.vVendorInfo.add(mode, vInfo);
            inputSetting.vFrame.push_back(info);
            MY_LOGD3("request frameNo:%d", info.frameNo);
        }
        // update p2 control
        updateSetting(outputInfo, inputSetting);
        //
        mpManager->set(plugin::CALLER_SHOT_SMART, inputSetting);
    }
    //
    capturedCount = inputSetting.vFrame.size();
    dummyCount = outputInfo.dummyCount;
    delayedCount = outputInfo.delayedCount;
    totalShotCount = capturedCount + dummyCount + delayedCount;
    MY_LOGD("totalShotCount:%d (capturedCount:%d, dummyCount:%d, delayedCount:%d)",
             totalShotCount, capturedCount, dummyCount, delayedCount);

    Vector<MINT32> vRequestNo;
    // if using ZSD flow, and not use ZSD buffers, we need to applyRawBufferSettings
    if ( mbZsdFlow && !isUseZsdBuffers() ) {
        // submit to zsd preview pipeline
        Vector<SettingSet> vSettings;

        // front dummy frames (optional for 3A stable before capture, example: capture with flash light)
        for ( MINT32 i = 0; i < dummyCount; ++i ) {
            SettingSet s;
            s.appSetting = mShotParam.mAppSetting;
            s.halSetting = mShotParam.mHalSetting;
            s.appSetting += outputInfo.dummySettings[i].setting.appMeta;
            s.halSetting += outputInfo.dummySettings[i].setting.halMeta;
            vSettings.push_back(s);
        }

        // captured frames
        for ( MINT32 i = 0; i < capturedCount; ++i ) {
            SettingSet s;
            s.appSetting = inputSetting.vFrame[i].curAppControl;
            s.halSetting = inputSetting.vFrame[i].curHalControl;
            vSettings.push_back(s);
        }

        // back delayed frames (optional for 3A stable before resume preview)
        for ( MINT32 i = 0; i < delayedCount; ++i ) {
            SettingSet s;
            s.appSetting = mShotParam.mAppSetting;
            s.halSetting = mShotParam.mHalSetting;
            s.appSetting += outputInfo.delayedSettings[i].setting.appMeta;
            s.halSetting += outputInfo.delayedSettings[i].setting.halMeta;
            vSettings.push_back(s);
        }
        applyRawBufferSettings(vSettings, totalShotCount, vRequestNo, dummyCount, delayedCount);
    }

    // resume preview after all request has been sent
    _preview_controller = nullptr;

    // if using ZSD selector, and using ZSD buffer, we have to get all buffers
    // at the same time, makes sure them continuous.
    if ( mbZsdFlow
        && mSelectorUsage.load() == ISelector::SelectorType_ZsdSelector
        && isUseZsdBuffers())
    {
        MY_LOGD("use ZsdSelector+ZsdBuffer control flow");
        status_t status = OK;
        Vector<MINT32> freeRequests;
        freeRequests.resize(capturedCount);
        auto result = submitZslCaptureSetting(
                inputSetting,
                freeRequests);

        if (result != OK) {
            MY_LOGE("Submit capture setting fail.");
            mNeedReconstruct = MTRUE;
            goto lbExit;
        }
    }
    else
    {
        int okCount = 0; // indicates OK count
        int failedCount = 0; // indicates failed count
        std::vector<MINT32> failedIndexes; // records the index of failed request

        // get multiple raw buffer and send to capture pipeline
        for ( MINT32 i = 0; i < capturedCount; ++i ) {
            IMetadata halSetting = mShotParam.mHalSetting;
            halSetting += inputSetting.vFrame[i].curHalControl;

            if ( mbZsdFlow ) {
                // get Selector Data (buffer & metadata)
                status_t status = OK;
                android::sp<IImageBuffer> pBuf = NULL; // full raw buffer
                IMetadata selectorAppMetadata; // app setting for this raw buffer. Ex.3A infomation

                // if using ZsdSelector...
                if (mSelectorUsage.load() == ISelector::SelectorType_ZsdSelector
                    && !isUseZsdBuffers() )
                {
                    // using requested buffers
                    status = getZsdSelectorRequestedData(
                            selectorAppMetadata,
                            halSetting,
                            pBuf,
                            vRequestNo[i]
                            );
                } else {
                    status = getSelectorData(
                            selectorAppMetadata,
                            halSetting,
                            pBuf
                            );
                }

                // check get data result
                if( status != OK) {
                    // count failed.
                    failedCount++;
                    failedIndexes.push_back(i);
                    //
                    MY_LOGE("Get selector data failed(idx=%d,target=%d,failedCount=%d), try again if necessary", i, capturedCount,failedCount);
                    // if all the frame are failed... take any frame and try again
                    if (failedCount >= capturedCount) {
                        MY_LOGW("No any frame is OK, try the last times (any frame), or indicates NE");
                        status = getSelectorData(
                            selectorAppMetadata,
                            halSetting,
                            pBuf
                            );

                        // if still failed, ok good, assert it.
                        if (status != OK) {
                            MY_LOGF("Tried to get any frame still failed, assert!");
                            *(volatile uint32_t*)(0x0) = 0xDEADFEED;
                            return false;
                        }
                    }
                    else {
                        continue;
                    }
                }

                // update App metadata from selector to the current request
                {
                    Mutex::Autolock _l(mResultMetadataSetLock);
                    mResultMetadataSetMap.editValueFor(mCapReqNo).selectorAppMetadata = selectorAppMetadata;
                }
            } else {
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
                        okCount == 0,
                        inputSetting.vFrame[i].curAppControl,
                        halSetting) ) {
                MY_LOGE("Submit capture setting fail.");
                mNeedReconstruct = MTRUE;
                goto lbExit;
            }
            //
            mCapReqNo++;
            okCount++;
        }

        // submit dummy frame if using Zsd mode
        if ( mbZsdFlow ) {
            for (int i = okCount, index = 0; i < capturedCount; i++, index++) {
                if (__builtin_expect( index >= failedIndexes.size(), false)) {
                    MY_LOGW("index(%d) is greater than failedIndexes.size(%zu), ignore dummy frame", index, failedIndexes.size());
                    break;
                }
                MINT32 failedIdx = failedIndexes[index]; // get failed index
                MY_LOGI("submit dummy frame (idx:%d,target:%d,failedIdx:%d)", i, capturedCount, failedIdx);
                submitZsdDummyCaptureSetting(
                        inputSetting.vFrame[failedIdx].curAppControl,
                        inputSetting.vFrame[failedIdx].curHalControl
                        );
                mCapReqNo++;
            }
        }
    }

    // if ZSD mode, check selector type.
    if ( mbZsdFlow ) {
        // check if using ZSD selector, and sent request, make sure to clear them
        if ( mSelectorUsage.load() == ISelector::SelectorType_ZsdSelector &&
             vRequestNo.size() > 0)
        {
            mpConsumer = IResourceContainer::getInstance(getOpenId())->queryConsumer( eSTREAMID_IMAGE_PIPE_RAW_OPAQUE );
            sp<StreamBufferProvider> pProvider = mpConsumer.promote();
            sp<ISelector> s = pProvider->querySelector();
            if (s.get() && s->getSelectorType() == ISelector::SelectorType_ZsdSelector) {
                MY_LOGD("clear wait request");
                static_cast<ZsdSelector*>(s.get())->clearWaitRequestNo(vRequestNo);
            }
        }
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
 ******************************************************************************/
void
SmartShot::
onCmd_cancel()
{
    FUNC_START;

    /* checking current state and update state to cancel */
    {
        std::unique_lock<std::mutex> lk(mShotLock);
        if (mShotState == SHOTSTATE_CAPTURE) {
            /* wait catpure done */
            MY_LOGW("%s: wait for shot condition signal", __FUNCTION__);
            int timeout_ms = SMART_SHOT_CANCEL_TIMEOUT_MS;
            auto status = mShotCond.wait_for(lk, std::chrono::milliseconds(timeout_ms));
            if (CC_UNLIKELY( status == std::cv_status::timeout )) {
                // timeout ASSERT
                std::string _log = std::string("FATAL: wait timeout of shot cancel: ") + \
                                   std::to_string(timeout_ms) + std::string("ms");
                AEE_ASSERT(_log.c_str());
            }
            MY_LOGW("%s: shot condition signal ready", __FUNCTION__);
        }
        mShotState = SHOTSTATE_CANCEL;
        MY_LOGD("ShotState: SHOTSTATE_CANCEL, isZsd(%d)", mbZsdFlow);
    }


    /* if using BackgroundService, do nothing */
    if (supportBackgroundService()) {
    }
    else {
        this->waitUntilDrained();
    }

    if (!mbZsdFlow) {
#if MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT
        ICommonCapturePipeline::removeCommonCapturePipeline(getOpenId());
#endif
    }

    /* Reconstuct shot instance after capture cancel */
    mNeedReconstruct = MTRUE;

    FUNC_END;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
SmartShot::
handlePostViewData(IImageBuffer *pImgBuffer)
{
    CAM_TRACE_CALL();

    MY_LOGD3("+ (pImgBuffer) = (%p)", pImgBuffer);
    class scopedVar
    {
    public:
                    scopedVar(IImageBuffer* pBuf)
                    : mBuffer(pBuf) {
                        if( mBuffer )
                            mBuffer->lockBuf(LOG_TAG, eBUFFER_USAGE_SW_READ_MASK);
                    }
                    ~scopedVar() {
                        if( mBuffer )
                            mBuffer->unlockBuf(LOG_TAG);
                    }
    private:
        IImageBuffer* mBuffer;
    } _local(pImgBuffer);

    // check current state
    mShotLock.lock();
    auto _state = mShotState;
    mShotLock.unlock();

    if (_state != SHOTSTATE_CANCEL) {
        mpShotCallback->onCB_PostviewClient(
                getInstanceId(),
                pImgBuffer,
                0, // never ask for offset
                supportBackgroundService()
                );
    }
    else {
        MY_LOGD("disable sending Postview Cb due to state is cancel");
    }

    MY_LOGD3("-");

    return  MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
SmartShot::
handleRaw16CB(IImageBuffer const *pImgBuffer)
{
    CAM_TRACE_CALL();
    MY_LOGD3("+ (pImgBuffer) = (%p)", pImgBuffer);
    mpShotCallback->onCB_Raw16Image(reinterpret_cast<IImageBuffer const*>(pImgBuffer));
    return  MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
SmartShot::
handleDNGMetaCB(MUINT32 const requestNo)
{
    MY_LOGD3("handleDNGMetaCB(%d)",requestNo);
    CAM_TRACE_CALL();
    //
    int idx = -1;
    {
        Mutex::Autolock _l(mResultMetadataSetLock);
        idx = mResultMetadataSetMap.indexOfKey(requestNo);
    }
    //
    if(idx < 0 )
    {
        MY_LOGE("mResultMetadataSetMap can't find requestNo(%d)",requestNo);
        return MFALSE;
    }
    else
    {
        IMetadata rDngMeta;
        IMetadata rHalMeta;
        IMetadata rAppMeta;
        {
            Mutex::Autolock _l(mResultMetadataSetLock);
            rHalMeta = mResultMetadataSetMap.editValueFor(requestNo).halResultMetadata;
            rAppMeta = mResultMetadataSetMap.editValueFor(requestNo).appResultMetadata;
            rDngMeta = MAKE_DngInfo(LOG_TAG, getOpenId())->getShadingMapFromHal(rHalMeta, rAppMeta);
        }
        //
        MY_LOGD3("rDngMeta(%d) size(%d)",requestNo,rDngMeta.count());
        MY_LOGD3("before append: mResultMetadataSetMap(%d) size(%d)",requestNo,rAppMeta.count());
        rAppMeta+=rDngMeta;
        MY_LOGD3("after append: mResultMetadataSetMap(%d) size(%d)",requestNo,rAppMeta.count());
        //
        if( rAppMeta.count() > 0 )
        {
            MY_LOGD3("handleDNGMetaCB onCB_DNGMetaData(%d) size(%d)",requestNo,rAppMeta.count());
            mpShotCallback->onCB_DNGMetaData((MUINTPTR)&rAppMeta);
        }
    }
    return  MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
SmartShot::
handleJpegData(MUINT32 const requestNo __attribute__((unused)), IImageBuffer* pJpeg)
{
    CAM_TRACE_CALL();
    //
    class scopedVar
    {
    public:
                    scopedVar(IImageBuffer* pBuf)
                    : mBuffer(pBuf) {
                        if( mBuffer )
                            mBuffer->lockBuf(LOG_TAG, eBUFFER_USAGE_SW_READ_MASK);
                    }
                    ~scopedVar() {
                        if( mBuffer )
                            mBuffer->unlockBuf(LOG_TAG);
                    }
    private:
        IImageBuffer* mBuffer;
    } _local(pJpeg);
    //
    uint8_t const* puJpegBuf = (uint8_t const*)pJpeg->getBufVA(0);
    MUINT32 u4JpegSize = pJpeg->getBitstreamSize();

    MY_LOGD3("+ (puJpgBuf, jpgSize) = (%p, %d)",
            puJpegBuf, u4JpegSize);

    // check current state
    mShotLock.lock();
    auto state = mShotState;
    mShotLock.unlock();

    // dummy raw callback
    if (state != SHOTSTATE_CANCEL)
        mpShotCallback->onCB_RawImage(0, 0, NULL);

    // jpeg callback
    mpShotCallback->onCB_CompressedImage_packed(getInstanceId(),
                                         u4JpegSize,
                                         puJpegBuf,
                                         0,                       //callback index
                                         true,                     //final image
                                         MTK_CAMERA_MSG_EXT_DATA_COMPRESSED_IMAGE,
                                         getShotMode(),
                                         supportBackgroundService()
                                         );
    MY_LOGD3("-");

    return MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MERROR
SmartShot::
updateSetting(
    plugin::OutputInfo& param,
    /*out*/
    plugin::InputSetting& setting
)
{

    {
        MINT64 p2_setting = MTK_P2_ISP_PROCESSOR|MTK_P2_MDP_PROCESSOR|MTK_P2_CAPTURE_REQUEST;
        if (param.inCategory == plugin::FORMAT_RAW && param.outCategory == plugin::FORMAT_YUV)
            p2_setting = MTK_P2_RAW_PROCESSOR|MTK_P2_CAPTURE_REQUEST|MTK_P2_PREVIEW_REQUEST;
        else if (param.inCategory == plugin::FORMAT_YUV && param.outCategory == plugin::FORMAT_YUV)
            p2_setting = MTK_P2_ISP_PROCESSOR|MTK_P2_YUV_PROCESSOR|MTK_P2_MDP_PROCESSOR|MTK_P2_CAPTURE_REQUEST;
        else if (param.inCategory == plugin::FORMAT_RAW && param.outCategory == plugin::FORMAT_RAW)
            p2_setting = MTK_P2_RAW_PROCESSOR|MTK_P2_ISP_PROCESSOR|MTK_P2_MDP_PROCESSOR|MTK_P2_CAPTURE_REQUEST;;

        for( size_t i = 0; i < setting.vFrame.size(); ++i ) {
            IMetadata::IEntry entry(MTK_PLUGIN_P2_COMBINATION);
            entry.push_back(p2_setting, Type2Type< MINT64 >());
            setting.vFrame.editItemAt(i).curHalControl.update(entry.tag(), entry);
        }
        MY_LOGD3("p2 setting: %" PRId64 , p2_setting);
    }

    return OK;
    // check flash needed
    // yes --> apply raw
    // no --> normal
}


void
SmartShot::
setMaxCaptureFrameNum(MINT32 num)
{
    mMaxCaptureFrameNum.store(num);
}


MINT32
SmartShot::
getMaxCaptureFrameNum()
{
    return mMaxCaptureFrameNum.load();
}


/******************************************************************************
*
*******************************************************************************/
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

MBOOL
SmartShot::
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
    BufferList vDstStreams_disp;
    MBOOL bRrzo = MFALSE;
    for( size_t i = 0; i < rawInputInfos.size() ; i++) {
        PREPARE_STREAM(vDstStreams, rawInputInfos[i]->getStreamId(),  true);
        if (rawInputInfos[i]->getStreamId() == eSTREAMID_IMAGE_PIPE_RAW_RESIZER) bRrzo = MTRUE;
    }
    // prepare RRZO, however we dont need it but LMV does.
    if (!bRrzo)
        PREPARE_STREAM(vDstStreams, eSTREAMID_IMAGE_PIPE_RAW_RESIZER, false);

    // PREPARE_STREAM(vDstStreams, eSTREAMID_IMAGE_PIPE_RAW_OPAQUE,  true);

#if defined(MFLL_DISPLAY_ALL_AIS_FRAMES) && (MFLL_DISPLAY_ALL_AIS_FRAMES)
    // if using AIS, these frames are necessary to be displayed
    auto bDisplayPreview = ::property_get_int32("persist.mfll.showaispreview", 1);
    if (mShotParam.mi4MfbMode == MTK_MFB_MODE_AIS && bDisplayPreview) {
        PREPARE_STREAM(vDstStreams, eSTREAMID_IMAGE_PIPE_YUV_00, false);
    }
#endif

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

    // print all request number
    if (__builtin_expect( mDbgLevel >= 3, false )) {
        std::string szDbgMsg;
        for (const auto& i : rqNum)
            szDbgMsg += std::to_string(i) + std::string(" ");
        MY_LOGD("all req: %s", szDbgMsg.c_str());
    }

    MUINT32 capturedCount = shotCount - dummyCount - delayedCount;
    // shotCount:     all request frames count (front dummy frames + captured frames + back delayed frames)
    // capturedCount: captured frames count
    // dummyCount:    front dummy frames count
    // delayedCount:  back delayed frames count
    //
    // We separate settings and real frame count
    rvRequestNo.clear();
    if ( static_cast<MUINT32>(capturedCount) < rqNum.size() ) {
        for ( MINT32 i = 0; i < capturedCount; ++i ) {
            MINT32 capIndex;
            if (dummyCount > 0) {
                capIndex = i + dummyCount;
            }
            else {
                capIndex = i;
            }

            if (capIndex > rqNum.size()) {
                MY_LOGW("capIndex:%d out of rqNum[%zu]",
                        capIndex, rqNum.size());
            }
            else {
                rvRequestNo.push_back(rqNum[capIndex]);
                MY_LOGD("rqNum[%d]:%d", capIndex, rqNum[capIndex]);
            }
        }
    } else {
        rvRequestNo = rqNum;
    }

    // print wait request captured frames number (without dummy, delayed frames)
    if (__builtin_expect( mDbgLevel >= 3, false )) {
        std::string szDbgMsg;
        for (const auto& i : rvRequestNo)
            szDbgMsg += std::to_string(i) + std::string(" ");
        MY_LOGD("wait req: %s", szDbgMsg.c_str());
    }

    //
    status_t status = OK;

    if (mSelectorUsage.load() == ISelector::SelectorType_ZsdRequestSelector) {
        mspOriSelector = pProvider->querySelector();
        sp<ZsdRequestSelector> pSelector = new ZsdRequestSelector();
        pSelector->setWaitRequestNo(rvRequestNo);
        status = pProvider->switchSelector(pSelector);
        MY_LOGD("ZsdRequestSelector");
    }
    else if (mSelectorUsage.load() == ISelector::SelectorType_ZsdSelector) {
        sp<ISelector> pSelector = pProvider->querySelector();
        if (CC_UNLIKELY( pSelector->getSelectorType() != ISelector::SelectorType_ZsdSelector )) {
            MY_LOGD("The current selector is not ZsdSelector, we intent to "\
                    "use ZsdSelector, hence to create one and switch it");
            pSelector = new ZsdSelector();
            status = pProvider->switchSelector(pSelector);
            if (status != OK) {
                MY_LOGE("change to ZsdSelector failed");
                return MFALSE;
            }
        }
        ZsdSelector* pZsdSelector = static_cast<ZsdSelector*>(pSelector.get());
        pZsdSelector->setWaitRequestNo(rvRequestNo);
        MY_LOGD("using ZsdSelector");
    }
    else {
        MY_LOGE("mSelectorUsage(%d) is not support", mSelectorUsage.load());
        return MFALSE;
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
SmartShot::
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
    auto pPipeline = this->getPipeline();
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

        MY_LOGD3("%s:=========================", __FUNCTION__);
        MY_LOGD3("%s: App metadata", __FUNCTION__);
        MY_LOGD3("%s: Get Iso -> %d, iso->: %d", __FUNCTION__, isoResult, isoVal);
        MY_LOGD3("%s: Get Exp -> %d, exp->: %" PRId64 "", __FUNCTION__, expResult, expVal);
        MY_LOGD3("%s:=========================", __FUNCTION__);
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
            PREPARE_STREAM(vDstStreams, eSTREAMID_IMAGE_PIPE_RAW_RESIZER, false);
            if ( mShotParam.mbEnableLtm )
                PREPARE_STREAM(vDstStreams, eSTREAMID_IMAGE_PIPE_RAW_LCSO, false);
        } else {
            Vector<sp<IImageStreamInfo>> rawInputInfos;
            mpConsumer = IResourceContainer::getInstance(getOpenId())->queryConsumer( eSTREAMID_IMAGE_PIPE_RAW_OPAQUE );
            sp<StreamBufferProvider> pConsumer = mpConsumer.promote();
            if( !pConsumer.get() ) return UNKNOWN_ERROR;
            pConsumer->querySelector()->queryCollectImageStreamInfo( rawInputInfos, MTRUE );
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
MERROR
SmartShot::
submitZsdDummyCaptureSetting(
        IMetadata appSetting,
        IMetadata halSetting
        )
{

    MY_LOGD("current submit requestNo:%d (dummy request)", mCapReqNo);
    auto pPipeline = this->getPipeline();
    if( !pPipeline.get() )
    {
        MY_LOGW("get pPipeline fail");
        return UNKNOWN_ERROR;
    }

    BufferList vDstStreams;
    ILegacyPipeline::ResultSet resultSet;

    // prepare a source (pick up the smallest one)
    const StreamId_T sid = eSTREAMID_IMAGE_PIPE_RAW_RESIZER;
    PREPARE_STREAM(vDstStreams, sid, false);

    // add a null image buffer to full RAW pool
#if MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT
    sp<CallbackBufferPool> pPool = pPipeline->queryBufferPool(sid);
#else
    sp<CallbackBufferPool> pPool = mpCallbackHandler->queryBufferPool(sid);
#endif

    if( pPool == NULL) {
        MY_LOGE("query Pool Fail!");
        return UNKNOWN_ERROR;
    }

    // create a dummy image buffer for this request
    pPool->addAllocateBuffer(1);

    // update error frame metadata
    IMetadata::setEntry<MUINT8>(&halSetting, MTK_HAL_REQUEST_ERROR_FRAME, 1);

    // add to local container
    {
        Vector<ISelector::BufferItemSet> vBufferSet;
        vBufferSet.add(ISelector::BufferItemSet());
        vBufferSet.editItemAt(0).id = sid;

        Mutex::Autolock _l(mResultMetadataSetLock);
        IMetadata resultAppMetadata;
        IMetadata resultHalMetadata;
        mResultMetadataSetMap.add(
                static_cast<MUINT32>(mCapReqNo),
                ResultSet_T{static_cast<MUINT32>(mCapReqNo), resultAppMetadata, resultHalMetadata, appSetting, vBufferSet, NONE_CB_DONE }
                );
    }

    // Go
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

    return OK;
}

/******************************************************************************
*
*******************************************************************************/
std::shared_ptr<void>
SmartShot::
pausePreview()
{
    // non-zsd, no need pause preview
    CAM_TRACE_NAME("pause_preview");
    if (CC_UNLIKELY( !mbZsdFlow )) {
        MY_LOGD3("no need to pause preview due to non-zsd mode");
        return nullptr;
    }

    sp<IFeatureFlowControl> _pf = IResourceContainer::getInstance(getOpenId())->queryFeatureFlowControl();
    if (CC_UNLIKELY( _pf == nullptr )) {
        MY_LOGW("IFeatureFlowControl is nullptr, cannot pause preview, ignored");
        return nullptr;
    }

    auto _result = _pf->pausePreview(true);
    if (CC_UNLIKELY( _result )) {
        MY_LOGW("pause preview failed");
        return nullptr;
    }
    //
    MY_LOGD("paused preview");
    //
    return std::shared_ptr<void>(nullptr, [_pf](void*) mutable {
        CAM_TRACE_NAME("resume_preview");
        auto _result = _pf->resumePreview();
        if (CC_UNLIKELY( _result )) {
            CAM_LOGW("resume preview failed");
        }
        else {
            CAM_LOGD("resumed preview");
        }
        _pf = nullptr;
    });
}

/******************************************************************************
*
*******************************************************************************/
MVOID
SmartShot::
updateLastInfo()
{
    mLastRotation = mShotParam.mu4Transform;
    mLastPicFmt   = mShotParam.miPictureFormat;
    mLastJpegsize = mJpegsize;
}

/******************************************************************************
*
*******************************************************************************/
MVOID
SmartShot::
endCapture()
{
    FUNCTION_SCOPE;

    /* declare a job to wait until pipeline drain */
    auto _jobWaitUntilDrain = [this](sp<ImpShot> shot)
    {
        const char* FUNCTION = "_jobWaitUntilDrain";
        MY_LOGD("%s: [+], uid=%" PRId64 "", FUNCTION, getInstanceId());
        this->waitUntilDrained();
        MY_LOGD("%s: [-], uid=%" PRId64 "", FUNCTION, getInstanceId());
    };

    /* ZSD flow */
    if ( mbZsdFlow) {
        if (mSelectorUsage.load () != ISelector::SelectorType_ZsdSelector){
            // change to ZSD selector
            mpConsumer = IResourceContainer::getInstance(getOpenId())->queryConsumer( eSTREAMID_IMAGE_PIPE_RAW_OPAQUE );
            sp<StreamBufferProvider> pConsumer = mpConsumer.promote();
            if( pConsumer != NULL) {
                if(mspOriSelector != nullptr) {
                    MY_LOGD3("restore original selector");
                    pConsumer->setSelector(mspOriSelector);
                    mspOriSelector = nullptr;
                }else{
                    MY_LOGD3("create new selector");
                    sp<ZsdSelector> pSelector = new ZsdSelector();
                    pConsumer->switchSelector(pSelector);
                }
            }
        }

        /* if using BackgroundService, wait pipeline drain in a detach thread */
        if (supportBackgroundService()) {
            sp<ImpShot> self = this;
            MY_LOGI("adding _jobWaitUntilDrain to PostProcessJobQueue, uid=%" PRId64 "", getInstanceId());
            NSCam::PostProcessJobQueue::addJob( std::bind(
                        std::move(_jobWaitUntilDrain), self
                        ));
        }
    }
    /* non-ZSD */
    else {
        /* if supportBackgroundService and fast P1 node flush, flush p1 but wait P2 drain in detached thread */
        if (supportBackgroundService() && supportFastP1Done()) {
            fastP1DoneDrain();
            sp<ImpShot> self = this;
            MY_LOGI("adding _jobWaitUntilDrain to PostProcessJobQueue, uid=%" PRId64 "", getInstanceId());
            NSCam::PostProcessJobQueue::addJob( std::bind(
                        std::move(_jobWaitUntilDrain), self
                        ));
        }
        else {
            this->waitUntilDrained();
        }
    }
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
SmartShot::
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
            MY_LOGD3("rawInputInfos[%d]:%d", i, rawInputInfos[i]->getStreamId());
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
            MBOOL useUFOfmt = MFALSE;
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

            MY_LOGD3("create ImageStreamInfo ResizedRaw done");
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
                    numPostviewYuv , 0,
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

/******************************************************************************
*
*******************************************************************************/
#if MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT
MBOOL
SmartShot::
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
        mpConsumer = IResourceContainer::getInstance(getOpenId())->queryConsumer( eSTREAMID_IMAGE_PIPE_RAW_OPAQUE );
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

        //  --- set 3A init config ----
        // HDRMode
        MUINT8 hdrModeVal = 0;
        IMetadata::getEntry<MUINT8>(&mShotParam.mHalSetting, MTK_3A_HDR_MODE, hdrModeVal);
        pipelineConfig.LPBConfigParams.hal3AParams.setHdrMode(hdrModeVal);

        // init frame's ISP Profile
        HDRMode hdrMode = static_cast<HDRMode>(hdrModeVal);
        ProfileParam profileParam
        {
            mSensorSize,
            vhdrMode, /*VHDR mode*/
            mu4Scenario,
            ProfileParam::FLAG_NONE,
            ((hdrMode == HDRMode::AUTO) || (hdrMode == HDRMode::VIDEO_AUTO))
                ? ProfileParam::FMASK_AUTO_HDR_ON
                : ProfileParam::FMASK_NONE
        };
        MUINT8 profile = 0;
        if (FeatureProfileHelper::getShotProf(profile, profileParam))
            pipelineConfig.LPBConfigParams.hal3AParams.setIspProfile(profile);


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
    sp<Pipeline_T> pPipeline = ICommonCapturePipeline::queryCommonCapturePipeline(pipelineConfig);
    CHECK_OBJECT(pPipeline);
    this->setPipeline(pPipeline);

    return MTRUE;
}

#else // not MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT

MBOOL
SmartShot::
createPipeline()
{
    CAM_TRACE_CALL();
    FUNC_START;
    LegacyPipelineBuilder::ConfigParams LPBConfigParams;
    //
    sp<IResourceContainer> pResourceContainer = IResourceContainer::getInstance(getOpenId());
    LPBConfigParams.p1ConfigConcurrency = pResourceContainer->queryResourceConcurrency(IResourceContainer::RES_CONCUR_P1);
    LPBConfigParams.p2ConfigConcurrency = pResourceContainer->queryResourceConcurrency(IResourceContainer::RES_CONCUR_P2);
    //
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
        MY_LOGD3("createPipeline for smart shot");
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
                MY_LOGD3("src config for streamId: %#" PRIx64"", pStreamInfo->getStreamId());

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

            MY_LOGD3("vhdrMode(0x%X)", vhdrMode);

            PipelineSensorParam sensorParam(
            /*.mode      = */mu4Scenario,
            /*.rawType   = */0x0000,
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

            //  --- set 3A init config ----
            // HDRMode
            MUINT8 hdrModeVal = 0;
            IMetadata::getEntry<MUINT8>(&mShotParam.mHalSetting, MTK_3A_HDR_MODE, hdrModeVal);
            LPBConfigParams.hal3AParams.setHdrMode(hdrModeVal);

            // init frame's ISP Profile
            HDRMode hdrMode = static_cast<HDRMode>(hdrModeVal);
            ProfileParam profileParam
            {
                mSensorSize,
                vhdrMode, /*VHDR mode*/
                mu4Scenario,
                ProfileParam::FLAG_NONE,
                ((hdrMode == HDRMode::AUTO) || (hdrMode == HDRMode::VIDEO_AUTO))
                    ? ProfileParam::FMASK_AUTO_HDR_ON
                    : ProfileParam::FMASK_NONE
            };
            MUINT8 profile = 0;
            if (FeatureProfileHelper::getShotProf(profile, profileParam))
                LPBConfigParams.hal3AParams.setIspProfile(profile);
            //
            pBuilder->updateHal3AConfig( LPBConfigParams.hal3AParams );
            // --------
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

    sp<Pipeline_T> pPipeline = pBuilder->create();
    this->setPipeline(pPipeline);
    //
    FUNC_END;
    return MTRUE;

}
#endif // MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT

/******************************************************************************
*
*******************************************************************************/
MINT
SmartShot::
getLegacyPipelineMode(void)
{
    int shotMode = getShotMode();
    EPipelineMode pipelineMode = getPipelineMode();
    int legacyPipeLineMode = LegacyPipelineMode_T::PipelineMode_Capture;
    switch(shotMode)
    {
        default:
            legacyPipeLineMode = (pipelineMode == ePipelineMode_Feature) ?
                LegacyPipelineMode_T::PipelineMode_Feature_Capture :
                LegacyPipelineMode_T::PipelineMode_Capture;
            break;
    }
    return legacyPipeLineMode;
}

/******************************************************************************
*
*******************************************************************************/
status_t
SmartShot::
getSelectorData(
    IMetadata& rAppSetting,
    IMetadata& rHalSetting,
    android::sp<IImageBuffer>& pBuffer
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
    auto pPipeline = this->getPipeline();
    if(pPipeline == NULL) {
        MY_LOGE("pPipeline is nullptr");
        return UNKNOWN_ERROR;
    }
    //
    status_t status = OK;
    MINT32 rRequestNo;
    Vector<ISelector::MetaItemSet> rvResultMeta;
    Vector<ISelector::BufferItemSet> rvBufferSet;
    pSelector->sendCommand(ISelector::eCmd_setAllBuffersTransferring, MTRUE, 0, 0);
    status = pSelector->getResult(rRequestNo, rvResultMeta, rvBufferSet);
    pSelector->sendCommand(ISelector::eCmd_setAllBuffersTransferring, MFALSE, 0, 0);

    // check
    MBOOL foundFullRaw = MFALSE;
    MBOOL foundResizedRaw = MFALSE;
    for ( size_t i = 0; i < rvBufferSet.size(); ++i ) {
        if (rvBufferSet[i].id == eSTREAMID_IMAGE_PIPE_RAW_OPAQUE) {
            foundFullRaw = MTRUE;
        }
        if (rvBufferSet[i].id == eSTREAMID_IMAGE_PIPE_RAW_RESIZER) {
            foundResizedRaw = MTRUE;
        }
        if ( foundFullRaw && foundResizedRaw ) {
            MY_LOGD3("found full raw and resized raw");
            break;
        }
    }

    if ( !foundFullRaw || !foundResizedRaw || rvBufferSet.isEmpty() ) {
        MY_LOGE("Selector get input raw buffer failed! bufferSetSize(%zu). foundFullRaw(%d), foundResizedRaw(%d)",
                rvBufferSet.size(), foundFullRaw, foundResizedRaw);
        return UNKNOWN_ERROR;
    }

    if(status != OK) {
        MY_LOGE("Selector getResult Fail!");
        return UNKNOWN_ERROR;
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

/******************************************************************************
*
*******************************************************************************/
status_t
SmartShot::
getZsdSelectorRequestedData(
    IMetadata&                  rAppSetting,
    IMetadata&                  rHalSetting,
    android::sp<IImageBuffer>&  pBuffer,
    MINT32                      requestNo
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

    auto pPipeline = this->getPipeline();
    if(pPipeline == NULL) {
        MY_LOGE("pPipeline is nullptr");
        return UNKNOWN_ERROR;
    }

    if (CC_UNLIKELY( pSelector->getSelectorType() != ISelector::SelectorType_ZsdSelector )) {
        MY_LOGW("using getSelectorData but the selector is not ZsdSelect");
        return getSelectorData(rAppSetting, rHalSetting, pBuffer);
    }

    ZsdSelector* pZsdSelector = static_cast<ZsdSelector*>(pSelector.get());

    //
    status_t status = OK;
    Vector<ISelector::MetaItemSet> rvResultMeta;
    Vector<ISelector::BufferItemSet> rvBufferSet;
    // get "the" result
    status = pZsdSelector->getRequestedResult(
            requestNo, rvResultMeta, rvBufferSet, 1500);

    // return selec buffers lambda helper
    auto __releaseSelectorResource = [&]()
    {
        for (auto& el : rvBufferSet) {
            pZsdSelector->returnBuffer(el);
        }
    };

    // check
    MBOOL foundFullRaw = MFALSE;
    MBOOL foundResizedRaw = MFALSE;
    for ( size_t i = 0; i < rvBufferSet.size(); ++i ) {
        if (rvBufferSet[i].id == eSTREAMID_IMAGE_PIPE_RAW_OPAQUE) {
            foundFullRaw = MTRUE;
        }
        if (rvBufferSet[i].id == eSTREAMID_IMAGE_PIPE_RAW_RESIZER) {
            foundResizedRaw = MTRUE;
        }
        if ( foundFullRaw && foundResizedRaw ) {
            MY_LOGD3("found full raw and resized raw");
            break;
        }
    }
    if ( !foundFullRaw || !foundResizedRaw || rvBufferSet.isEmpty() ) {
        MY_LOGE("Selector get input raw buffer failed! bufferSetSize(%zu). foundFullRaw(%d), foundResizedRaw(%d)",
                rvBufferSet.size(), foundFullRaw, foundResizedRaw);

        __releaseSelectorResource();
        return UNKNOWN_ERROR;
    }

    if(status != OK) {
        MY_LOGE("Selector getResult Fail!");
        __releaseSelectorResource();
        return UNKNOWN_ERROR;
    }

    if(rvResultMeta.size() < 2) {
        MY_LOGE("ZsdSelect getResult rvResultMeta(%zu)", rvResultMeta.size());
        __releaseSelectorResource();
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
        __releaseSelectorResource();
        return UNKNOWN_ERROR;
    }

    for ( size_t i = 0; i < rvBufferSet.size(); ++i ) {
        pBuffer = rvBufferSet[i].heap->createImageBuffer();
        //
        if(pBuffer == NULL) {
            MY_LOGE("get buffer is NULL!");
            __releaseSelectorResource();
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
            __releaseSelectorResource();
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

/******************************************************************************
*
*******************************************************************************/
status_t
SmartShot::
submitZslCaptureSetting(
    const plugin::InputSetting  inputSetting,
    Vector<MINT32>              &vRequestNo
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

    if (CC_UNLIKELY( pSelector->getSelectorType() != ISelector::SelectorType_ZsdSelector )) {
        MY_LOGE("wrong selector type(%#x) for this method", pSelector->getSelectorType());
        return UNKNOWN_ERROR;
    }

    ZsdSelector* pZsdSelector = static_cast<ZsdSelector*>(pSelector.get());

    //
    status_t status = OK;
    Vector< Vector<ISelector::MetaItemSet> >    vvResultMeta;
    Vector< Vector<ISelector::BufferItemSet> >  vvBufferSet;

    // get the continuous results for N sceonds (do not take AF unstable one)
    static const MINT32 timeoutMs = ::property_get_int32(
            "debug.smartshot.zsdtimeout", DEFAULT_MAX_SELECTOR_TIMED_OUT_MS);
    MY_LOGD3("getContinuousResults in %d ms", timeoutMs);
    pZsdSelector->sendCommand(ISelector::eCmd_setAllBuffersTransferring, MTRUE, 0, 0);
    status = pZsdSelector->getContinuousResults(
            vRequestNo,
            vvResultMeta,
            vvBufferSet,
            timeoutMs // ms
            );
    pZsdSelector->sendCommand(ISelector::eCmd_setAllBuffersTransferring, MFALSE, 0, 0);

    // if timed out, try again, take any frame (w/o AF OK)
    if (status == TIMED_OUT) {
        pZsdSelector->sendCommand(ISelector::eCmd_setNoWaitAfDone, 0, 0, 0, 0);
        MY_LOGD3("sendCommand(eCmd_setNoWaitAfDone,0,0,0)");
        MY_LOGW("wait continuous result timed out (%d), try again for any buffer", timeoutMs);
        pZsdSelector->sendCommand(ISelector::eCmd_setAllBuffersTransferring, MTRUE, 0, 0);
        status = pZsdSelector->getContinuousResults(
                vRequestNo,
                vvResultMeta,
                vvBufferSet,
                timeoutMs // ms
                );
        pZsdSelector->sendCommand(ISelector::eCmd_setAllBuffersTransferring, MFALSE, 0, 0);
        pZsdSelector->sendCommand(ISelector::eCmd_setNeedWaitAfDone, 0, 0, 0, 0); // set back
        MY_LOGD3("sendCommand(eCmd_setNeedWaitAfDone,0,0,0)");
    }

    if (CC_UNLIKELY( status != OK )) {
        MY_LOGE("Selector getResult failed(%#x)", status);
        return status;
    }

    auto checkAndProcessResult =
        [   this,
            pSelector,
            &inputSetting
        ](
            Vector< ISelector::MetaItemSet >&   m,
            Vector< ISelector::BufferItemSet >& b,
            IMetadata&                          rAppSetting,
            IMetadata&                          rHalSetting,
            MINT32                              index,
            MINT32                              capNo
        )
            -> status_t
    { // {{{ checkAndProcessResult +++

        status_t r = OK;

        std::vector< sp<IImageBuffer> > _buffers(b.size());

        // check if there's full size RAW
        MBOOL foundFullRaw = MFALSE;
        MBOOL foundResizedRaw = MFALSE;
        for (auto& el : b) {
            if (el.id == eSTREAMID_IMAGE_PIPE_RAW_OPAQUE) {
                foundFullRaw = MTRUE;
            }
            if (el.id == eSTREAMID_IMAGE_PIPE_RAW_RESIZER) {
                foundResizedRaw = MTRUE;
            }
            if (foundFullRaw && foundResizedRaw) {
                MY_LOGD3("found full raw and resized raw");
                break;
            }
        }

        // if not found, return this BufferItemSet back to selector.
        if ( !foundFullRaw || !foundResizedRaw || b.isEmpty() ) {
            MY_LOGE("Selector get input raw buffer failed! bufferSetSize(%zu). foundFullRaw(%d), foundResizedRaw(%d)",
                    b.size(), foundFullRaw, foundResizedRaw);
            r = UNKNOWN_ERROR;
            goto lbReturnToSelector;
        }


        if(m.size() < 2) {
            MY_LOGE("MetaItemSet size(%zu) < 2, we need both APP and HAL meta", m.size());
            r = UNKNOWN_ERROR;
            goto lbReturnToSelector;
        }


        // get app & hal metadata
        if( m.editItemAt(0).id == eSTREAMID_META_APP_DYNAMIC_P1
            &&
            m.editItemAt(1).id == eSTREAMID_META_HAL_DYNAMIC_P1 )
        {
            rAppSetting  = m.editItemAt(0).meta;
            rHalSetting += m.editItemAt(1).meta;
        }
        else
        {
            if( m.editItemAt(1).id == eSTREAMID_META_APP_DYNAMIC_P1
                &&
                m.editItemAt(0).id == eSTREAMID_META_HAL_DYNAMIC_P1 )
            {
                rAppSetting  = m.editItemAt(1).meta;
                rHalSetting += m.editItemAt(0).meta;
            }
            else {
                MY_LOGE("Something wrong for selector metadata.");
                r = UNKNOWN_ERROR;
                goto lbReturnToSelector;
            }
        }

        // create all buffers from heap and check if all buffers are OK
        {
            bool _buffersOK = true;
            for (size_t i = 0; i < b.size(); i++) {
                sp<IImageBuffer> _buf = b[i].heap->createImageBuffer();
                if (CC_UNLIKELY( _buf.get() == nullptr )) {
                    MY_LOGE("get heap but create image failed, streamID=%#" PRIx64"", b[i].id);
                    _buffersOK = false;
                    break; // for-loop
                }
                else {
                    _buffers[i] = _buf;
                    MY_LOGD3("get heap and create image buffer(%p), streamID=%#" PRIx64", heap addr=(%p,%p)",
                    _buffers[i].get(), b[i].id, b[i].heap.get(), _buffers[i]->getImageBufferHeap());
                }
            }
            // create buffer failed, ignore.
            if (CC_UNLIKELY( ! _buffersOK )) {
                MY_LOGE("create image buffer failed");
                r = UNKNOWN_ERROR;
                goto lbReturnToSelector;
            }

            auto pPipeline = this->getPipeline();
            if(pPipeline == NULL) {
                MY_LOGE("pPipeline is nullptr");
                r = UNKNOWN_ERROR;
                goto lbReturnToSelector;
            }

            //update p1 buffer to pipeline pool
            for (size_t i = 0; i < b.size(); i++) {
#if MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT
                sp<CallbackBufferPool> pPool = pPipeline->queryBufferPool( b[i].id );
#else
                sp<CallbackBufferPool> pPool = mpCallbackHandler->queryBufferPool( b[i].id );
#endif
                if (CC_UNLIKELY( pPool == nullptr )) {
                    MY_LOGE("query pool fail");
                    r = UNKNOWN_ERROR;
                    goto lbReturnToSelector;
                }
                pPool->addBuffer( _buffers[i] );
            }
        }

        // reprocess app & hal metadata
        {
            IMetadata myAppMeta = inputSetting.vFrame[index].curAppControl;
            // do not update  MTK_SENSOR_SENSITIVITY and MTK_SENSOR_EXPOSURE_TIME
            myAppMeta.remove(MTK_SENSOR_SENSITIVITY);
            myAppMeta.remove(MTK_SENSOR_EXPOSURE_TIME);
            // app
            rAppSetting += myAppMeta;
            // hal
            rHalSetting += inputSetting.vFrame[index].curHalControl;
            // update sensor size
            IMetadata::IEntry entry(MTK_HAL_REQUEST_SENSOR_SIZE);
            entry.push_back(mSensorSize, Type2Type< MSize >());
            rHalSetting.update(entry.tag(), entry);
        }

        // saves metadata to map
        do {
            Mutex::Autolock _l(mResultMetadataSetLock);
            IMetadata resultAppMetadata;
            IMetadata resultHalMetadata;
            mResultMetadataSetMap.add(
                    static_cast<MUINT32>(capNo),
                    ResultSet_T{static_cast<MUINT32>(capNo), resultAppMetadata, resultHalMetadata, rAppSetting, b, NONE_CB_DONE }
                    );

        } while (0);

        return r; // OK!!!

lbReturnToSelector:
        do {
            for (auto& el : b) {
                pSelector->returnBuffer(el);
            }
        }
        while(0);
        return r;
    }; // }}} checkAndProcessResult ---

    // check all results from selector and add to pool
    MY_LOGD3("size of(vRequestNo,vvResultMeta,vvBufferSet)=(%zu,%zu,%zu)",
            vRequestNo.size(), vvResultMeta.size(), vvBufferSet.size());
    if (CC_UNLIKELY(
        vRequestNo.size() != vvResultMeta.size() || vRequestNo.size() != vvBufferSet.size()))
    {
        // return all BufferItemSet back to selector
        for (auto& _c : vvBufferSet)
            for (auto& el : _c) {
                pSelector->returnBuffer( el );
            }
        return UNKNOWN_ERROR;
    }

    // for all results ...
    for (size_t i = 0; i < vRequestNo.size(); i++) {
        IMetadata rHalSetting;
        IMetadata rAppSetting;
        status = checkAndProcessResult(
                vvResultMeta.editItemAt(i),
                vvBufferSet.editItemAt(i),
                rAppSetting,
                rHalSetting,
                i,
                mCapReqNo);

        if (CC_UNLIKELY( status != OK )) {
            MY_LOGE("checkAndProcessResult returns %#x, ignore this buffer", status);
            continue;
        }

        // submit setting to capture pipeline
        if (OK != submitCaptureSetting(
                    i == 0,
                    rAppSetting,
                    rHalSetting
           ))
        {
            MY_LOGE("Submit capture setting fail.");
            mNeedReconstruct = MTRUE;
            return UNKNOWN_ERROR;
        }

        mCapReqNo++;
    }

    //
    return OK;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
SmartShot::
makeVendorCombination(
    plugin::InputInfo&  inputInfo,
    MINT64              const mode
)
{
    MBOOL ret = MTRUE;

    switch(mode){
        case MTK_PLUGIN_MODE_MFNR:
        case MTK_PLUGIN_MODE_HDR:
            inputInfo.combination.push_back(mode);
            inputInfo.combination.push_back(MTK_PLUGIN_MODE_NR);
            inputInfo.combination.push_back(MTK_PLUGIN_MODE_COPY);
            break;
        case MTK_PLUGIN_MODE_MFHR:
        case MTK_PLUGIN_MODE_BMDN:
        case MTK_PLUGIN_MODE_FUSION_3rdParty:
        case MTK_PLUGIN_MODE_DCMF_3RD_PARTY:
            inputInfo.combination.push_back(mode);
            break;
        default:
            MY_LOGE("can't support this vendor mode! %" PRId64 "", mode);
            ret = MFALSE;
    }
    return ret;
}
/*******************************************************************************
*
********************************************************************************/
MVOID
SmartShot::
onMetaReceived(
    MUINT32         const requestNo,
    StreamId_T      const streamId,
    MBOOL           const errorResult,
    IMetadata       const result
)
{
    CAM_TRACE_FMT_BEGIN("onMetaReceived No%d,StreamID %#" PRIx64, requestNo,streamId);
    MY_LOGD3("requestNo %d, stream %#" PRIx64", errResult:%d", requestNo, streamId, errorResult);
    //
    {
        Mutex::Autolock _l(mResultMetadataSetLock);
        int idx = mResultMetadataSetMap.indexOfKey(requestNo);
        if(idx < 0 )
        {
            MY_LOGE("mResultMetadataSetMap can't find requestNo(%d)",requestNo);
            for ( size_t i=0; i<mResultMetadataSetMap.size(); i++) {
                MY_LOGD3( "mResultMetadataSetMap(%zu/%zu)  requestNo(%d) buf(%p)",
                         i, mResultMetadataSetMap.size(), mResultMetadataSetMap[i].requestNo,
                         mResultMetadataSetMap[i].selectorGetBufs[0].heap.get() );
            }
            return;
        }
    }
    //
    MINT32 callbackState = NONE_CB_DONE;
//#warning "Need to modify if pipeline change"
    switch (streamId)
    {
        case eSTREAMID_META_APP_FULL:
            {
                Mutex::Autolock _l(mResultMetadataSetLock);
                mResultMetadataSetMap.editValueFor(requestNo).appResultMetadata = result;
                mResultMetadataSetMap.editValueFor(requestNo).appResultMetadata += mResultMetadataSetMap.editValueFor(requestNo).selectorAppMetadata;
                mResultMetadataSetMap.editValueFor(requestNo).callbackState |= FULL_APP_META_CB_DONE;
                callbackState = mResultMetadataSetMap.editValueFor(requestNo).callbackState;
            }

            if( (callbackState & FULL_ALL_META_CB_DONE) == FULL_ALL_META_CB_DONE )
            {
                if(isDataMsgEnabled(EIShot_DATA_MSG_RAW))
                {
                    handleDNGMetaCB(requestNo);//dng metadata callback
                }
            }

            // re-check callbackState again, if state is DNG_CB_DONE, removes it
            do {
                Mutex::Autolock _l(mResultMetadataSetLock);
                if (mResultMetadataSetMap.indexOfKey(requestNo) < 0)
                {
                    MY_LOGW("try to re-check callbackState for requestNo %d, but not exists",
                            requestNo);
                    break;
                }
                //
                callbackState = mResultMetadataSetMap.editValueFor(requestNo).callbackState;
                if( (callbackState & DNG_CB_DONE) == DNG_CB_DONE )
                {
                    //clear result metadata
                    mResultMetadataSetMap.removeItem(requestNo);
                    MY_LOGD3("clear mResultMetadataSetMap(%d)",requestNo);
                }
            } while(0);

            break;
        case eSTREAMID_META_HAL_FULL:
            {
                Mutex::Autolock _l(mResultMetadataSetLock);
                mResultMetadataSetMap.editValueFor(requestNo).halResultMetadata = result;
                mResultMetadataSetMap.editValueFor(requestNo).callbackState |= FULL_HAL_META_CB_DONE;
                callbackState = mResultMetadataSetMap.editValueFor(requestNo).callbackState;
            }
            //
            if( (callbackState & FULL_ALL_META_CB_DONE) == FULL_ALL_META_CB_DONE )
            {
                if(isDataMsgEnabled(EIShot_DATA_MSG_RAW))
                {
                    handleDNGMetaCB(requestNo);//dng metadata callback
                }
            }
            //
            // re-check callbackState again, if state is DNG_CB_DONE, removes it
            do {
                Mutex::Autolock _l(mResultMetadataSetLock);
                if (mResultMetadataSetMap.indexOfKey(requestNo) < 0)
                {
                    MY_LOGW("try to re-check callbackState for requestNo %d, but not exists",
                            requestNo);
                    break;
                }
                //
                callbackState = mResultMetadataSetMap.editValueFor(requestNo).callbackState;
                if( (callbackState & DNG_CB_DONE) == DNG_CB_DONE )
                {
                    //clear result metadata
                    mResultMetadataSetMap.removeItem(requestNo);
                    MY_LOGD3("clear mResultMetadataSetMap(%d)",requestNo);
                }
            } while(0);
            break;
        case eSTREAMID_META_HAL_DYNAMIC_P2:
            {
                Mutex::Autolock _l(mResultMetadataSetLock);
                mResultMetadataSetMap.editValueFor(requestNo).halResultMetadata = result;
            }
            break;
        case eSTREAMID_META_APP_DYNAMIC_P2:
            {
                Mutex::Autolock _l(mResultMetadataSetLock);
                mResultMetadataSetMap.editValueFor(requestNo).appResultMetadata += result;
            }
            break;
        default:
            MY_LOGD("unsupported streamID(%#" PRIx64")", streamId);
    }
    //
    CAM_TRACE_FMT_END();
}

/*******************************************************************************
*
********************************************************************************/
MVOID
SmartShot::
onDataReceived(
    MUINT32 const               requestNo,
    StreamId_T const            streamId,
    MBOOL   const               errorBuffer,
    android::sp<IImageBuffer>&  pBuffer
)
{
    CAM_TRACE_FMT_BEGIN("onDataReceived No%d,streamId %#" PRIx64"",requestNo,streamId);
    MY_LOGD3("requestNo %d, streamId %#" PRIx64", buffer %p, error %d", requestNo, streamId, pBuffer.get(), errorBuffer);
    //
    HwInfoHelper helper(getOpenId());
    if( ! helper.updateInfos() ) {
        MY_LOGE("cannot properly update infos");
    }
    MUINT32 u4RawFmtType;
    helper.getSensorRawFmtType(u4RawFmtType);
    //
    if( pBuffer != nullptr )
    {
        MINT32 data = NSCamShot::ECamShot_DATA_MSG_NONE;
        //
        switch (streamId)
        {
            case eSTREAMID_IMAGE_PIPE_RAW_OPAQUE:
                data = NSCamShot::ECamShot_DATA_MSG_RAW;
                if(isDataMsgEnabled(EIShot_DATA_MSG_RAW))
                {
                    pBuffer->lockBuf(LOG_TAG, eBUFFER_USAGE_SW_READ_MASK);
                    handleRaw16CB(pBuffer.get());
                    pBuffer->unlockBuf(LOG_TAG);
                }
                //
                if ( mbZsdFlow ) {
                    sp<StreamBufferProvider> pProvider = IResourceContainer::getInstance(getOpenId())->queryConsumer( eSTREAMID_IMAGE_PIPE_RAW_OPAQUE );
                    MY_LOGD3("Query Consumer OpenID(%d) StreamID(%d)", getOpenId(), eSTREAMID_IMAGE_PIPE_RAW_OPAQUE);
                    if( pProvider == NULL) {
                        MY_LOGE("can't find StreamBufferProvider in ConsumerContainer");
                    }
                    else {
                        Vector<ISelector::BufferItemSet> bufferSet;
                        {
                            Mutex::Autolock _l(mResultMetadataSetLock);
                            bufferSet = mResultMetadataSetMap.editValueFor(requestNo).selectorGetBufs;
                        };

                        sp< ISelector > pSelector = pProvider->querySelector();
                        if(pSelector == NULL) {
                            MY_LOGE("can't find Selector in Consumer when p2done");
                        } else {
                            for(size_t i = 0; i < bufferSet.size() ; i++)
                                pSelector->returnBuffer(bufferSet.editItemAt(i));
                        }
                    }
                    //
                    {
                        Mutex::Autolock _l(mResultMetadataSetLock);
                        for (auto& itr : mResultMetadataSetMap.editValueFor(requestNo).selectorGetBufs) {
                            itr = ISelector::BufferItemSet();
                        }
                    }
                }
                //
                {
                    Mutex::Autolock _l(mResultMetadataSetLock);
                    mResultMetadataSetMap.editValueFor(requestNo).callbackState |= RAW_CB_DONE;
                    if( (mResultMetadataSetMap.editValueFor(requestNo).callbackState & DNG_CB_DONE) == DNG_CB_DONE )
                    {
                        //clear result metadata
                        mResultMetadataSetMap.removeItem(requestNo);
                        MY_LOGD3("clear mResultMetadataSetMap(%d)",requestNo);
                    }
                }
                break;
            case eSTREAMID_IMAGE_PIPE_YUV_JPEG:
                data = NSCamShot::ECamShot_DATA_MSG_YUV;
                break;
            case eSTREAMID_IMAGE_PIPE_YUV_00:
                data = NSCamShot::ECamShot_DATA_MSG_POSTVIEW;
                handlePostViewData(pBuffer.get());
                break;
            case eSTREAMID_IMAGE_JPEG:
                data = NSCamShot::ECamShot_DATA_MSG_JPEG;
                handleJpegData(requestNo, pBuffer.get());
                break;
            default:
                data = NSCamShot::ECamShot_DATA_MSG_NONE;
                break;
        }
        //
        if( mDumpFlag & data )
        {
            String8 filename = String8::format("%s/SmartShot_%dx%d",
                    DUMP_PATH, pBuffer->getImgSize().w, pBuffer->getImgSize().h);
            switch( data )
            {
                case NSCamShot::ECamShot_DATA_MSG_RAW:
                    filename += String8::format("_%zd.raw", pBuffer->getBufStridesInBytes(0));
                    break;
                case NSCamShot::ECamShot_DATA_MSG_YUV:
                case NSCamShot::ECamShot_DATA_MSG_POSTVIEW:
                    filename += String8(".yuv");
                    break;
                case NSCamShot::ECamShot_DATA_MSG_JPEG:
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
    }
    CAM_TRACE_FMT_END();
}

/*******************************************************************************
*
********************************************************************************/
MERROR
SmartShot::
checkStreamAndEncodeLocked( MUINT32 const /*requestNo*/)
{
    return OK;
}

/*******************************************************************************
*
********************************************************************************/
MVOID
SmartShot::
beginCapture(
    MINT32 rAllocateCount
)
{
    CAM_TRACE_CALL();
    //
     HwInfoHelper helper(getOpenId());
     if( ! helper.updateInfos() ) {
         MY_LOGE("cannot properly update infos");
     }
     if( ! helper.getSensorSize( mu4Scenario, mSensorSize) ) {
         MY_LOGE("cannot get params about sensor");
     }
    //
    // TODO add other consumer
    if ( mbZsdFlow ) {
        mpConsumer = IResourceContainer::getInstance(getOpenId())->queryConsumer( eSTREAMID_IMAGE_PIPE_RAW_OPAQUE );
        MY_LOGD3("Query Consumer OpenID(%d) StreamID(%d)", getOpenId(), eSTREAMID_IMAGE_PIPE_RAW_OPAQUE);
        sp<StreamBufferProvider> pProvider = mpConsumer.promote();
        if( pProvider == NULL) {
            MY_LOGE("can't find StreamBufferProvider in ConsumerContainer");
            return;
        }
        sp<IImageStreamInfo> pInfo = pProvider->queryImageStreamInfo();
        pProvider->updateBufferCount("SmartShot", pInfo->getMaxBufNum() + rAllocateCount);
    }
    mJpegsize      = (mShotParam.mu4Transform & eTransform_ROT_90) ?
         MSize(mShotParam.mi4PictureHeight, mShotParam.mi4PictureWidth):
         MSize(mShotParam.mi4PictureWidth, mShotParam.mi4PictureHeight);
    //
    createStreams();
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
SmartShot::
constructCapturePipeline()
{
    CAM_TRACE_CALL();

    //
    MY_LOGD3("zsd raw stream %#" PRIx64 ", (%s) size(%dx%d), fmt 0x%x",
            mpInfo_FullRaw->getStreamId(),
            mpInfo_FullRaw->getStreamName(),
            mpInfo_FullRaw->getImgSize().w,
            mpInfo_FullRaw->getImgSize().h,
            mpInfo_FullRaw->getImgFormat()
           );
    // create new pipeline
    if ( ! createPipeline() ) {
        MY_LOGE("createPipeline failed");
        return MFALSE;
    }
    //
    auto pPipeline = this->getPipeline();
    CHECK_OBJECT(pPipeline);
    //
#if (MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT == 0)
    if( !pPipeline.get() )
    {
        MY_LOGW("get pPipeline fail");
        return MFALSE;
    }
    sp<ResultProcessor> pResultProcessor = pPipeline->getResultProcessor().promote();
    CHECK_OBJECT(pResultProcessor);

    // metadata
    mpMetadataListener = new MetadataListener(this);
    pResultProcessor->registerListener( std::numeric_limits<MUINT32>::min(), std::numeric_limits<MUINT32>::max(), true, mpMetadataListener);

    mpMetadataListenerFull = new MetadataListener(this);
    pResultProcessor->registerListener( std::numeric_limits<MUINT32>::min(), std::numeric_limits<MUINT32>::max(), false, mpMetadataListenerFull);

#endif
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
SmartShotUtil*
SmartShotUtil::
getInstance()
{
    static SmartShotUtil ssUtil;
    return &ssUtil;
}
/*******************************************************************************
*
********************************************************************************/
SmartShotUtil::
SmartShotUtil()
{
  mpSem = new Semaphore(0);
  if (mpSem == NULL)
  {
      MYU_LOGE("alloc Semaphore fail!!");
  }
}
/*******************************************************************************
*
********************************************************************************/
SmartShotUtil::
~SmartShotUtil()
{
    if (mpSem)
        delete mpSem;
}
/*******************************************************************************
*
********************************************************************************/
void
SmartShotUtil::
SyncDualCam(
    int openId
)
{
    MBOOL bSync;
    {
        Mutex::Autolock _l(mOpenIdMutex);
        mvOpenIds.push_back(openId);
        if ((bSync = (mvOpenIds.size() >= 2)))
            mvOpenIds.clear();
    }

    if (bSync)
    {
        mpSem->post();
    }
    else
    {
        bool bStatus = mpSem->timedWait(10000);  // 10 secs
        if (bStatus == false)
        {
            MYU_LOGE("sem_timedwait(), %s, errno = %d", strerror(errno), errno);
        }
    }
}


void
SmartShot::
waitUntilDrained()
{
    MY_LOGD("%s: this=%p [+]", __FUNCTION__, this);

    auto pPipeline = this->getPipeline();;

    if( pPipeline.get() )
    {
        /* if using fast P1 done mode, we have to wait P1 & other nodes drain */
        if (supportFastP1Done()) {
            fastP1DoneDrain();
            pPipeline->waitUntilP2JpegDrainedAndFlush();
        }
        else {
            pPipeline->waitUntilDrained();
        }

        pPipeline->flush();
    }

    // clear SmartShot's pipeline first.
    {
        sp<Pipeline_T> emptyPipeline;
        this->setPipeline(emptyPipeline);
    }

    MY_LOGD("%s: this=%p [-]", __FUNCTION__, this);
}


void
SmartShot::
fastP1DoneDrain()
{
    std::lock_guard<std::mutex> l(mFlushP1Lock);
    auto pPipeline = getPipeline();
    if (pPipeline.get() == nullptr) {
        return;
    }

    /* check if flushed */
    if (mFlushP1Done) {
        return;
    }

    FUNC_START;

    // 1. wait P1 drain
    pPipeline->waitNodeUntilDrained(eNODEID_P1Node);

    // 2. send p1done callback which tells MW to resume preview.
    mpShotCallback->onCB_P1done();

    // 3. flush P1 node
    pPipeline->flushNode(eNODEID_P1Node);

    // mark finished.
    mFlushP1Done = true;

    FUNC_END;
}


bool
SmartShot::
supportFastP1Done()
{
    HwInfoHelper helper(getOpenId());
    if( ! helper.updateInfos() ) {
        MY_LOGE("cannot properly update infos");
        return false;
    }

    MUINT32 u4RawFmtType;
    helper.getSensorRawFmtType(u4RawFmtType);

    if (mShotParam.mbNeedP1DoneCb)
        return true;

        // fast resume preview
    if (!mbZsdFlow && helper.get4CellSensorSupported() && u4RawFmtType==SENSOR_RAW_4CELL_HW_BAYER)
        return true;

    return false;
}
