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

#include "PipelineModelImpl.h"
#include "MyUtils.h"

//
#include <mtkcam/utils/hw/HwInfoHelper.h>
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/utils/LogicalCam/IHalLogicalDeviceList.h>
#include <mtkcam3/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/std/ULog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_MODEL);

/******************************************************************************
 *
 ******************************************************************************/
using namespace android;
using namespace NSCam;
using namespace NSCam::v3::pipeline::model;
using namespace NSCamHW;
/******************************************************************************
 *
 ******************************************************************************/
#define MY_DEBUG(level, fmt, arg...) \
    do { \
        CAM_LOG##level("[%u:%s] " fmt, getOpenId(), __FUNCTION__, ##arg); \
        mDebugPrinter->printFormatLine(#level" [%u:%s] " fmt, getOpenId(), __FUNCTION__, ##arg); \
    } while(0)

#define MY_WARN(level, fmt, arg...) \
    do { \
        CAM_LOG##level("[%u:%s] " fmt, getOpenId(), __FUNCTION__, ##arg); \
        mWarningPrinter->printFormatLine(#level" [%u:%s] " fmt, getOpenId(), __FUNCTION__, ##arg); \
    } while(0)

#define MY_ERROR(level, fmt, arg...) \
    do { \
        CAM_LOG##level("[%u:%s] " fmt, getOpenId(), __FUNCTION__, ##arg); \
        mErrorPrinter->printFormatLine(#level" [%u:%s] " fmt, getOpenId(), __FUNCTION__, ##arg); \
    } while(0)

#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(...)                MY_WARN (W, __VA_ARGS__)
#define MY_LOGE(...)                MY_ERROR(E, __VA_ARGS__)
#define MY_LOGA(...)                MY_ERROR(A, __VA_ARGS__)
#define MY_LOGF(...)                MY_ERROR(F, __VA_ARGS__)
//
#define MY_LOGV_IF(cond, ...)       do { if (            (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if (            (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if (            (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#define MY_LOGD1(...)               MY_LOGD_IF(           (mLogLevel>=1), __VA_ARGS__)
#define MY_LOGD2(...)               MY_LOGD_IF(CC_UNLIKELY(mLogLevel>=2), __VA_ARGS__)
#define MY_LOGD3(...)               MY_LOGD_IF(CC_UNLIKELY(mLogLevel>=3), __VA_ARGS__)

/******************************************************************************
 *
 ******************************************************************************/
PipelineModelImpl::
PipelineModelImpl(CreationParams const& creationParams __unused)
    : mPipelineStaticInfo(std::make_shared<PipelineStaticInfo>())
    , mErrorPrinter(creationParams.errorPrinter)
    , mWarningPrinter(creationParams.warningPrinter)
    , mDebugPrinter(creationParams.debugPrinter)
    //
    , mOpenId(creationParams.openId)
    , mLogLevel(0)
    , mHalDeviceAdapter(IHalDeviceAdapter::create(mOpenId))
    //
    , mUserName()
    , mSession(nullptr)
    //
{
    mLogLevel = ::property_get_int32("vendor.debug.camera.log", 0);
    if ( mLogLevel == 0 ) {
        mLogLevel = ::property_get_int32("vendor.debug.camera.log.pipelinemodel", 0);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineModelImpl::
createInstance(CreationParams const& creationParams) -> android::sp<PipelineModelImpl>
{
    sp<PipelineModelImpl> pPipeline = new PipelineModelImpl(creationParams);
    if ( CC_UNLIKELY( ! pPipeline.get() ) ) {
        CAM_ULOGME("create pipelinemodel instance fail");
        return nullptr;
    }

    if ( CC_UNLIKELY( ! pPipeline->init() ) ) {
        CAM_ULOGME("pipelinemodel instance init fail");
        return nullptr;
    }

    return pPipeline;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineModelImpl::
init() -> bool
{
    MY_LOGD1("+");

    if ( CC_UNLIKELY( mPipelineStaticInfo==nullptr ) ) {
        MY_LOGE("mPipelineStaticInfo==nullptr");
        return false;
    }

    bool ret = initPipelineStaticInfo();
    if ( CC_UNLIKELY( ! ret ) ) {
        MY_LOGE("Fail on initPipelineStaticInfo");
        return false;
    }

    MY_LOGD1("-");
    return true;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineModelImpl::
initPipelineStaticInfo() -> bool
{
    bool ret = false;
    MY_LOGD1("+");

    if ( CC_UNLIKELY( mHalDeviceAdapter==nullptr ) ) {
        MY_LOGE("Fail on IHalDeviceAdapter::create()");
        return false;
    }

    ret = mHalDeviceAdapter->getPhysicalSensorId(mPipelineStaticInfo->sensorId);
    if ( CC_UNLIKELY(!ret) ) {
        MY_LOGE("Fail on getPhysicalSensorId");
        return false;
    }

    mPipelineStaticInfo->openId = mOpenId;

    mPipelineStaticInfo->sensorRawType.resize(mPipelineStaticInfo->sensorId.size());
    for (size_t i = 0; i < mPipelineStaticInfo->sensorId.size(); i++) {
        HwInfoHelper helper(mPipelineStaticInfo->sensorId[i]);
        if ( CC_UNLIKELY(!helper.updateInfos()) ) {
            MY_LOGE("cannot properly update infos");
        }

        if ( CC_UNLIKELY(!helper.getSensorRawFmtType(mPipelineStaticInfo->sensorRawType[i])) ) {
            MY_LOGW("sensorId[%zu]:%d fail on getSensorRawFmtType", i, mPipelineStaticInfo->sensorId[i]);
        }

        if ( helper.get4CellSensorSupported() ) {
            mPipelineStaticInfo->is4CellSensor = true;
        }
        if ( helper.isType3PDSensorWithoutPDE(0, true) ) {
            mPipelineStaticInfo->isType3PDSensorWithoutPDE = true;
        }
    }

    MINT32 DebugDirectFDYUV = ::property_get_int32("persist.vendor.debug.camera.directfdyuv", -1);
    if (DebugDirectFDYUV == -1)
    {
        mPipelineStaticInfo->isP1DirectFDYUV = ::property_get_int32("ro.vendor.camera.directfdyuv.support", 0) == 1;
    }
    else
    {
        mPipelineStaticInfo->isP1DirectFDYUV = DebugDirectFDYUV == 1;
        MY_LOGI("debug direct YUV : %d", mPipelineStaticInfo->isP1DirectFDYUV);
    }
    mPipelineStaticInfo->isP1DirectScaledYUV = StereoSettingProvider::isP1YUVSupported();

    //A logical device will have only one feature
    mPipelineStaticInfo->isP1DirectScaledYUV = false;
    MINT32 dualFeature = MAKE_HalLogicalDeviceList()->getSupportedFeature(mOpenId);
    if(dualFeature & DEVICE_FEATURE_VSDOF)
    {
        mPipelineStaticInfo->isP1DirectScaledYUV = StereoSettingProvider::isP1YUVSupported();
    }

    sp<IMetadataProvider> pMetadataProvider = NSMetadataProviderManager::valueFor(getOpenId());
    if( pMetadataProvider.get() )
    {
        IMetadata static_meta = pMetadataProvider->getMtkStaticCharacteristics();
        IMetadata::IEntry capbilities = static_meta.entryFor(MTK_REQUEST_AVAILABLE_CAPABILITIES);
        if( !capbilities.isEmpty() )
        {
            for (MUINT32 i = 0; i < capbilities.count(); i++)
            {
                if ( capbilities.itemAt(i, Type2Type<MUINT8>()) == MTK_REQUEST_AVAILABLE_CAPABILITIES_BURST_CAPTURE )
                {
                    mPipelineStaticInfo->isSupportBurstCap = true;
                    break;
                }
            }
        }
        // isp tuning data stream size
        IMetadata::IEntry rawIspTuningDataStreamSize = static_meta.entryFor(MTK_CONTROL_CAPTURE_ISP_TUNING_DATA_SIZE_FOR_RAW);
        if( !rawIspTuningDataStreamSize.isEmpty() )
        {
            mPipelineStaticInfo->rawIspTuningDataStreamSize = MSize(rawIspTuningDataStreamSize.itemAt(0,Type2Type< MINT32 >()),rawIspTuningDataStreamSize.itemAt(1,Type2Type< MINT32 >()));
            MY_LOGD("MTK_CONTROL_CAPTURE_ISP_TUNING_DATA_STREAM_SIZE_FOR_RAW(%d,%d)",mPipelineStaticInfo->rawIspTuningDataStreamSize.w,mPipelineStaticInfo->rawIspTuningDataStreamSize.h);
        }
        IMetadata::IEntry yuvIspTuningDataStreamSize = static_meta.entryFor(MTK_CONTROL_CAPTURE_ISP_TUNING_DATA_SIZE_FOR_YUV);
        if( !yuvIspTuningDataStreamSize.isEmpty() )
        {
            mPipelineStaticInfo->yuvIspTuningDataStreamSize = MSize(yuvIspTuningDataStreamSize.itemAt(0,Type2Type< MINT32 >()),yuvIspTuningDataStreamSize.itemAt(1,Type2Type< MINT32 >()));
            MY_LOGD("MTK_CONTROL_CAPTURE_ISP_TUNING_DATA_STREAM_SIZE_FOR_YUV(%d,%d)",mPipelineStaticInfo->yuvIspTuningDataStreamSize.w,mPipelineStaticInfo->yuvIspTuningDataStreamSize.h);
        }
    }

    if ( mPipelineStaticInfo->sensorId.size() > 1 ) {
        mPipelineStaticInfo->isDualDevice = true;
        // multicam path does not support 4cell flow.
        mPipelineStaticInfo->is4CellSensor = false;
        // multicam disable dirFD yuv
        mPipelineStaticInfo->isP1DirectFDYUV = false;

        if ( CC_UNLIKELY(mPipelineStaticInfo->isType3PDSensorWithoutPDE) ) {
            MY_LOGE("Conflict: DualCam v.s. ISP3 PD Sensor");
        }
    }

    mPipelineStaticInfo->isNeedP2ProcessRaw = MTK_CAM_MUST_USE_P2_PROCESS_RAW;
    MY_LOGD("Must use P2 output process raw : %d", mPipelineStaticInfo->isNeedP2ProcessRaw);

    NSCam::IHalSensorList *pSensorHalList = NULL;
    NSCam::SensorStaticInfo sensorStaticInfo;

    pSensorHalList = MAKE_HalSensorList();
    if(pSensorHalList == NULL)
    {
        MY_LOGE("pSensorHalList::get fail");
        pSensorHalList = NULL;
        return false;
    }

    MUINT32 sensorDev = (MUINT32)pSensorHalList->querySensorDevIdx(mPipelineStaticInfo->sensorId[0]);

    pSensorHalList->querySensorStaticInfo(sensorDev,&sensorStaticInfo);

    {
    //video HDR
        //query from sensor driver
        //sensorStaticInfo.HDR_Support  0: NO HDR, 1: iHDR, 2:mvHDR, 3:zHDR
        MBOOL bVhdrSensor = (sensorStaticInfo.HDR_Support > 0);
        if(bVhdrSensor &&
           (dualFeature & DEVICE_FEATURE_VSDOF))
        {
            MY_LOGD("Disable VHDR for VSDoF, open id %d", mOpenId);
            bVhdrSensor = false;
        }

        if(bVhdrSensor)
        {
            MY_LOGD("Sensor driver support VHDR (HDR_Support : %d)", sensorStaticInfo.HDR_Support);
            mPipelineStaticInfo->isVhdrSensor = true;
        }
        else
        {
            MY_LOGD("Sensor driver not support VHDR");
            mPipelineStaticInfo->isVhdrSensor = false;
        }
    }

    MY_LOGD1("-");
    return true;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineModelImpl::
dumpState(
    android::Printer& printer __unused,
    const std::vector<std::string>& options __unused
) -> void
{
    //  Instantiation data
    {
        auto const& o = *mPipelineStaticInfo;
        std::string os;
        os += "{";
        os += ".openId=";
        os += std::to_string(o.openId);

        os += ", .sensorId={";
        for (auto const id : o.sensorId) {
            os += std::to_string(id);
            os += " ";
        }
        os += "}";

        os += ", .sensorRawType={";
        for (auto const v : o.sensorRawType) {
            os += std::to_string(v);
            os += " ";
        }
        os += "}";

        if  (o.isDualDevice) {
            os += ", .isDualDevice=true";
        }
        if  (o.isType3PDSensorWithoutPDE) {
            os += ", .isType3PDSensorWithoutPDE=true";
        }
        if  (o.is4CellSensor) {
            os += ", .is4CellSensor=true";
        }
        if  (o.isVhdrSensor) {
            os += ", .isVhdrSensor=true";
        }
        if  (o.isP1DirectFDYUV) {
            os += ", .isP1DirectFDYUV=true";
        }
        if  (o.isP1DirectScaledYUV) {
            os += ", .isP1DirectScaledYUV=true";
        }
        os += "}";
        printer.printLine(os.c_str());
    }
    printer.printFormatLine("{.mLogLevel=%d, .mHalDeviceAdapter=%p}", mLogLevel, mHalDeviceAdapter.get());

    auto timeout = std::chrono::milliseconds(100);

    //  Open data & Configuration data
    if  ( mLock.try_lock_for(timeout) ) {
        printer.printFormatLine(
            "{.mUserName=%s, .mCallback=%p, .mvOpenFutures#=%zu, .mSession=%p}",
            mUserName.c_str(), mCallback.unsafe_get(), mvOpenFutures.size(), mSession.get()
        );
        mLock.unlock();
    }
    else {
        printer.printLine("timeout: try_lock_for()");
    }

    //  Session
    if  ( mLock.try_lock_for(timeout) ) {
        if  ( mSession != nullptr ) {
            mSession->dumpState(printer, options);
        }
        mLock.unlock();
    }
    else {
        printer.printLine("timeout: try_lock_for()");
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineModelImpl::
open(
    std::string const& userName,
    android::wp<IPipelineModelCallback> const& callback
) -> int
{
    MY_LOGD1("+");
    //
    {
        std::lock_guard<std::timed_mutex> _l(mLock);

        mUserName = userName;
        mCallback = callback;

        mvOpenFutures.push_back(
            std::async(std::launch::async,
                [this]() {
                    return CC_LIKELY( mHalDeviceAdapter!=nullptr )
                        && CC_LIKELY( mHalDeviceAdapter->open() )
                        && CC_LIKELY( mHalDeviceAdapter->powerOn() );
                }
            )
        );
    }
    //
    MY_LOGD1("-");
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineModelImpl::
waitUntilOpenDone() -> bool
{
    MY_LOGD1("+");
    std::lock_guard<std::timed_mutex> _l(mLock);
    bool ret = waitUntilOpenDoneLocked();
    MY_LOGD1("- ret:%d", ret);
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineModelImpl::
waitUntilOpenDoneLocked() -> bool
{
    MY_LOGD1("+");
    //
    bool ret = true;
    for( auto &fut : mvOpenFutures ) {
        bool result = fut.get();
        if  ( CC_UNLIKELY( !result ) ) {
            MY_LOGE("Fail to init");
            ret = false;
        }
    }
    //
    mvOpenFutures.clear();
    //
    MY_LOGD1("-");
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineModelImpl::
close() -> void
{
    MY_LOGD1("+");
    {
        std::lock_guard<std::timed_mutex> _l(mLock);

        NSCam::Utils::CamProfile profile(__FUNCTION__, mUserName.c_str());

        waitUntilOpenDoneLocked();
        profile.print("waitUntilInitDone -");

        MY_LOGD1("destroying mSession");
        mCallback = nullptr;
        mSession = nullptr;//created at configure; destroyed at close.

        if  ( CC_LIKELY(mHalDeviceAdapter != nullptr) ) {
            mHalDeviceAdapter->powerOff();
            profile.print("Device powerOff -");
            mHalDeviceAdapter->close();
            profile.print("Device close -");
        }
        mUserName.clear();
    }
    MY_LOGD1("-");
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineModelImpl::
configure(
    std::shared_ptr<UserConfigurationParams>const& params
) -> int
{
    int err = OK;
    MY_LOGD1("+");
    {
        std::lock_guard<std::timed_mutex> _l(mLock);

        IPipelineModelSessionFactory::CreationParams sessionCfgParams;
        sessionCfgParams.pPipelineStaticInfo      = mPipelineStaticInfo;
        sessionCfgParams.pUserConfigurationParams = params;
        sessionCfgParams.pErrorPrinter            = mErrorPrinter;
        sessionCfgParams.pWarningPrinter          = mWarningPrinter;
        sessionCfgParams.pDebugPrinter            = mDebugPrinter;
        sessionCfgParams.pPipelineModelCallback   = mCallback.promote();
        //
        mSession = nullptr;
        mSession = IPipelineModelSessionFactory::createPipelineModelSession(sessionCfgParams);
        if ( CC_UNLIKELY( mSession==nullptr ) ) {
            MY_LOGE("null session");
            err = NO_INIT;
        }
    }
    MY_LOGD1("- err:%d", err);
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineModelImpl::
submitRequest(
    std::vector<std::shared_ptr<UserRequestParams>>const& requests,
    uint32_t& numRequestProcessed
) -> int
{
    int err = OK;
    android::sp<IPipelineModelSession> session;

    MY_LOGD2("+");
    {
        std::lock_guard<std::timed_mutex> _l(mLock);
        session = mSession;
    }
    {
        if ( CC_UNLIKELY( session==nullptr ) ) {
            MY_LOGE("null session");
            err = DEAD_OBJECT;
        }
        else {
            err = session->submitRequest(requests, numRequestProcessed);
        }
        session = nullptr;
    }
    MY_LOGD2("- err:%d", err);
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineModelImpl::
beginFlush() -> int
{
    int err = OK;
    android::sp<IPipelineModelSession> session;
    MY_LOGD1("+");

    {
        std::lock_guard<std::timed_mutex> _l(mLock);
        session = mSession;
    }
    if ( session != nullptr )
    {
        err = session->beginFlush();
        session = nullptr;
    }

    MY_LOGD1("-");
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineModelImpl::
endFlush() -> void
{
    android::sp<IPipelineModelSession> session;
    MY_LOGD1("+");

    {
        std::lock_guard<std::timed_mutex> _l(mLock);
        session = mSession;
    }
    if ( session != nullptr )
    {
        session->endFlush();
        session = nullptr;
    }

    MY_LOGD1("-");
}

