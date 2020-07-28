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
#define LOG_TAG "MtkCam/ZslProc"

#include "ZslProcessor.h"
#include "MyUtils.h"
#include "mtkcam/drv/iopipe/CamIO/IHalCamIO.h"
#include "mtkcam/utils/metadata/client/mtk_metadata_tag.h"
#include "mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h"
#include "mtkcam/utils/metastore/IMetadataProvider.h"
#include "mtkcam3/pipeline/utils/streaminfo/MetaStreamInfo.h"
#include "mtkcam3/pipeline/hwnode/StreamId.h"
#include <algorithm>
#include <mtkcam/utils/std/ULog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_ZSL);

using namespace NSCam::v3::pipeline::model;
using namespace NSCam::Utils::ULog;
/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if (            (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if (            (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if (            (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#define MY_LOGD1(...)               MY_LOGD_IF((mLogLevel>=1),__VA_ARGS__)
#define MY_LOGD2(...)               MY_LOGD_IF((mLogLevel>=2),__VA_ARGS__)
#define MY_LOGD3(...)               MY_LOGD_IF((mLogLevel>=3),__VA_ARGS__)

// public
#if 0       // performance check
#define FUNC_START_PUBLIC                                           \
        NSCam::Utils::CamProfile profile(__FUNCTION__, LOG_TAG);    \
        profile.print("+ %s", __FUNCTION__);
#define FUNC_END_PUBLIC       profile.print("-")
#elif 1     // functional check
#define FUNC_START_PUBLIC   MY_LOGD("+")
#define FUNC_END_PUBLIC     MY_LOGD("-")
#else
#define FUNC_START_PUBLIC
#define FUNC_END_PUBLIC
#endif

// protected/private
#if 1
#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")
#else
#define FUNC_START
#define FUNC_END
#endif

#define abs(a) (((a) < 0) ? -(a) : (a))

#define CHECK_ERROR(_err_)                                \
    do {                                                  \
        MERROR const err = (_err_);                       \
        if( err != OK ) {                                 \
            MY_LOGE("err:%d(%s)", err, ::strerror(-err)); \
            return err;                                   \
        }                                                 \
    } while(0)

#ifndef RETURN_IF_NOT_OK
#define RETURN_IF_NOT_OK(_expr_, fmt, arg...)                     \
    do {                                                                \
        int const err = (_expr_);                                       \
        if( CC_UNLIKELY(err != 0) ) {                                   \
            MY_LOGE("err:%d(%s) - " fmt, err, ::strerror(-err), ##arg); \
            return;                                                 \
        }                                                               \
    } while(0)
#endif

// TO-DO: move to custom folder
constexpr int64_t ZSLShutterDelayTimeDiffMs = 0;
constexpr int64_t MAX_TOLERANCE_SHUTTERDELAY_MS = 50L;
constexpr int32_t ZSLForcePolicyDisable = 0xefffffff;
constexpr int64_t ZSLForceTimeoutDisable = -1;

/******************************************************************************
 *
 ******************************************************************************/
static inline bool isDummyFrame(IFrameBuilder const& o)
{
    return ((o.getGroupFrameType() == GroupFrameType::PREDUMMY) ||
            (o.getGroupFrameType() == GroupFrameType::POSTDUMMY));
};

inline bool ZslProcessor::isAppMeta(StreamId_T const& streamId)
{
    return (mMetaDynamicP1.vApp.find(streamId) != mMetaDynamicP1.vApp.end());
};

inline bool ZslProcessor::isHalMeta(StreamId_T const& streamId)
{
    return (mMetaDynamicP1.vHal.find(streamId) != mMetaDynamicP1.vHal.end());
};

static inline int64_t getDurationMs(chrono::system_clock::time_point const& tp)
{
    return chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - tp).count();
}

inline bool ZslProcessor::hasExceedMaxRequest()  // lock
{
    Mutex::Autolock _l(mLock);
    if (mvPendingRequest.size() + mvInflightZslRequestNo.size() >= mMaxRequestSize) {
        MY_LOGW("pendingSz(%zu) + inflightSz(%zu) >= maxReqSz(%d), 1st-pending: R%u",
                mvPendingRequest.size(), mvInflightZslRequestNo.size(), mMaxRequestSize,
                (mvPendingRequest.empty()? 0 : mvPendingRequest[0].params.requestNo));
        return true;
    }
    return false;
}

template <typename T> inline
std::string toString_StreamIds(const T& items, char const* name)
{
    std::ostringstream oss;
    oss << name << "{ " << std::showbase << std::hex;
    for (auto const& streamId : items) {
        oss << streamId << " ";
    }
    oss <<  "}" << std::dec;
    return oss.str();
}

template <typename T> inline
void updateTag(const uint32_t tag, const IMetadata* inMeta, IMetadata* outMeta)
{
    IMetadata::IEntry inEntry = inMeta->entryFor(tag);
    IMetadata::IEntry outEntry(tag);
    const size_t entryCnt = inEntry.count();
    if (entryCnt == 0) {
        MY_LOGD("can not get tag(%#x) in AppCtrl", tag);
        return;
    }

    if constexpr (std::is_same_v<T, IMetadata> || std::is_same_v<T, IMetadata::Memory>)
    {
        for (size_t i = 0; i < entryCnt; i++) {
            T data = inEntry.itemAt(i, Type2Type<T>());
            outEntry.push_back(data, Type2Type<T>());
        }
    }
    else // IMETADATA_IENTRY_ARRAY_OPS supported (memcpy)
    {
        T datas[entryCnt];
        inEntry.itemAt(0, datas, entryCnt, Type2Type<T>());
        outEntry.push_back(datas, entryCnt, Type2Type<T>());
    }

    outMeta->update(tag, outEntry);
    MY_LOGD("update tag(%#x) #%zu", tag, entryCnt);
}

/******************************************************************************
 *
 ******************************************************************************/
void releaseBuffer(android::sp<NSCam::v3::IImageStreamBuffer>& rpBuffer)
{
    //MY_LOGD("release imageSB: (%#" PRIx64 ":%s)", rpBuffer->getStreamInfo()->getStreamId(), rpBuffer->getStreamInfo()->getStreamName());
    rpBuffer->releaseBuffer();  // return to the pool
    rpBuffer = nullptr;
}

void releaseBuffer(android::sp<NSCam::v3::IMetaStreamBuffer>& rpBuffer)
{
    //MY_LOGD("metaSB release automatically: (%#" PRIx64 ":%s)", rpBuffer->getStreamInfo()->getStreamId(), rpBuffer->getStreamInfo()->getStreamName());
    rpBuffer = nullptr;
}


/******************************************************************************
 *
 ******************************************************************************/
const std::string ZslProcessor::MyDebuggee::mName{"NSCam::v3::pipeline::model::IZslProcessor"};
auto ZslProcessor::MyDebuggee::debug(android::Printer& printer, const std::vector<std::string>& options) -> void
{
    auto p = mZslProcessor.promote();
    if ( CC_LIKELY(p != nullptr) ) {
        p->debug(printer, options);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
ZslProcessor::
ZslProcessor(int32_t openId, std::string const& name)
    : mOpenId(openId)
    , mUserName(name)
    , mLogLevel(::property_get_int32("vendor.debug.camera.log.pipelinemodel.zsl", 0))
    , mMaxRequestSize(0)
    , mFakeShutterNs(0)
    , mZSLTimeDiffMs(::property_get_int64("vendor.debug.camera.zsl.timestamp", ZSLShutterDelayTimeDiffMs))
    , mZSLForcePolicy(::property_get_int32("vendor.debug.camera.zsl.policy", ZSLForcePolicyDisable))
    , mZSLForceTimeOutMs(::property_get_int64("vendor.debug.camera.zsl.timeout", ZSLForceTimeoutDisable))
{
    int64_t default_zsl_ts_diff = ::property_get_int64("ro.vendor.camera3.zsl.default", 0);
    int64_t project_zsl_ts_diff = ::property_get_int64("ro.vendor.camera3.zsl.project", 0);
    int64_t user_zsl_ts_diff = ::property_get_int64("vendor.debug.camera3.zsl.timestamp", ZSLShutterDelayTimeDiffMs);
    mZSLTimeDiffMs = default_zsl_ts_diff + project_zsl_ts_diff + user_zsl_ts_diff;
    MY_LOGI("zsl timestamp diff(%" PRId64 ") = default(%" PRId64 ")+project(%" PRId64 ")+user(%" PRId64 ")",
            mZSLTimeDiffMs, default_zsl_ts_diff, project_zsl_ts_diff, user_zsl_ts_diff);
}


/******************************************************************************
 *
 ******************************************************************************/
android::sp<IZslProcessor>
IZslProcessor::
createInstance(
    int32_t openId,
    string const& name
)
{
    sp<ZslProcessor> pZslProcessor = new ZslProcessor(openId, name);
    if (CC_UNLIKELY(!pZslProcessor.get())) {
        MY_LOGE("create zsl processor instance fail");
        return nullptr;
    }

    return pZslProcessor;
}


/******************************************************************************
 *
 ******************************************************************************/
void
ZslProcessor::
onFirstRef()
{
    mDebuggee = std::make_shared<MyDebuggee>(this);
    if (CC_LIKELY(mDebuggee != nullptr)) {
        if (auto pDbgMgr = IDebuggeeManager::get()) {
            mDebuggee->mCookie = pDbgMgr->attach(mDebuggee, 0);
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
void
ZslProcessor::
onLastStrongRef(const void* id __unused)
{
    if (CC_LIKELY(mDebuggee != nullptr)) {
        if (auto pDbgMgr = IDebuggeeManager::get()) {
            pDbgMgr->detach(mDebuggee->mCookie);
        }
        mDebuggee = nullptr;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ZslProcessor::
configure(ZslConfigParams const& in) -> int
{
    CAM_TRACE_CALL();
    {
        Mutex::Autolock _l(mLock);
        // check if there is pending request
        if (mvPendingRequest.size() > in.maxRequestSize) {
            MY_LOGE("%zu ZSL pending requests not finish yet, maxReqSz(%d), 1st-reqNo: %u",
                    mvPendingRequest.size(), in.maxRequestSize,
                    (mvPendingRequest.empty()? 0 : mvPendingRequest[0].params.requestNo));
            return INVALID_OPERATION;
        }

        // save config params
        mvParsedStreamInfo_P1 = *(in.pParsedStreamInfo_P1);
        mPipelineModelCallback = in.pCallback;
        mMaxRequestSize = in.maxRequestSize;

        // get timeStampSrc from MetadataProvider, and init mFakeShutterNs
        MUINT8 timestampSrc = 1;
        sp<IMetadataProvider> pMetadataProvider = NSMetadataProviderManager::valueFor(mOpenId);
        if (pMetadataProvider.get()) {
            IMetadata static_meta = pMetadataProvider->getMtkStaticCharacteristics();
            if (IMetadata::getEntry<MUINT8>(&static_meta,
                MTK_SENSOR_INFO_TIMESTAMP_SOURCE, timestampSrc)) {
                MY_LOGD1("set TimestampSrc: %u(%s)", timestampSrc, timestampSrc ? "REALTIME" : "UNKNOWN");
            }
        } else {
            MY_LOGI("can not get MTK_SENSOR_INFO_TIMESTAMP_SOURCE, default TimestampSrc: %u(%s)",
                    timestampSrc, timestampSrc ? "REALTIME" : "UNKNOWN");
        }
        mFakeShutterNs = (timestampSrc == MTK_SENSOR_INFO_TIMESTAMP_SOURCE_UNKNOWN) ?
                    ::systemTime(SYSTEM_TIME_MONOTONIC) : ::systemTime(SYSTEM_TIME_BOOTTIME);
        // 0 = MTK_SENSOR_INFO_TIMESTAMP_SOURCE_UNKNOWN  (impl w/ SYSTEM_TIME_MONOTONIC)
        // 1 = MTK_SENSOR_INFO_TIMESTAMP_SOURCE_REALTIME (impl w/ SYSTEM_TIME_BOOTTIME)

        std::ostringstream oss;
        oss << "maxReqSize(" << mMaxRequestSize << ") " << "timestampSrc(" << (int)timestampSrc
            << ") ts(" << mFakeShutterNs << "ns) " << std::showbase << std::hex;

        // set DynamicP1 (AppMeta & HalMeta) & (default tracked streams) from ParsedStreamInfo_P1
        // FIXME: hard coded ...
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        auto addStreams = [](auto& dest, auto const& spSrc){
            if (spSrc.get()) { dest.emplace(spSrc->getStreamId()); }
        };
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        mDefaultTrackedParams = std::make_shared<TrackFrameResultParams>();
        for (size_t i = 0; i < mvParsedStreamInfo_P1.size(); i++) {
            addStreams(mMetaDynamicP1.vApp, mvParsedStreamInfo_P1[i].pAppMeta_DynamicP1);
            addStreams(mMetaDynamicP1.vHal, mvParsedStreamInfo_P1[i].pHalMeta_DynamicP1);
            //
            addStreams(mDefaultTrackedParams->imageStreams, mvParsedStreamInfo_P1[i].pHalImage_P1_Imgo);
            addStreams(mDefaultTrackedParams->imageStreams, mvParsedStreamInfo_P1[i].pHalImage_P1_Rrzo);
            addStreams(mDefaultTrackedParams->imageStreams, mvParsedStreamInfo_P1[i].pHalImage_P1_Lcso);
            addStreams(mDefaultTrackedParams->metaStreams, mvParsedStreamInfo_P1[i].pAppMeta_DynamicP1);
            addStreams(mDefaultTrackedParams->metaStreams, mvParsedStreamInfo_P1[i].pHalMeta_DynamicP1);
        }
        MY_LOGI("%s %s %s %s %s", oss.str().c_str(),
                toString_StreamIds(mMetaDynamicP1.vApp, "AppMeta_DynamicP1").c_str(),
                toString_StreamIds(mMetaDynamicP1.vHal, "HalMeta_DynamicP1").c_str(),
                toString_StreamIds(mDefaultTrackedParams->imageStreams, "DefaultTrackedImages").c_str(),
                toString_StreamIds(mDefaultTrackedParams->metaStreams, "DefaultTrackedMetas").c_str());
    }
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ZslProcessor::
debug(
    android::Printer& printer __unused,
    const std::vector<std::string>& options __unused
) -> void
{
    //@TODO: complete
    if (mLock.timedLock(100000000 /* 100ms */) == OK) {
        printer.printLine(" *Pending Request*");
        android::PrefixPrinter prefixPrinter1(printer, "  ");

        for (auto const& v : mvPendingRequest) {
            prefixPrinter1.printFormatLine("requestNo:%u", v.params.requestNo);
            android::PrefixPrinter prefixPrinter2(prefixPrinter1, "  ");
            print(prefixPrinter2, v.params);
        }

        mLock.unlock();
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ZslProcessor::
print(
    android::Printer& printer,
    ZslRequestParams const& o
) -> void
{
    if (CC_LIKELY(o.pCapParams != nullptr)) {
        auto const& zslPolicyParams = o.pCapParams->zslPolicyParams;
        printer.printFormatLine(
            "requestNo(%u) isZoom(%d) ZslPolicyParams:{policy=%#x timestamp=%" PRId64 " timeout=%" PRId64 "}",
            o.requestNo, o.isZoom,
            zslPolicyParams.mPolicy, zslPolicyParams.mTimestamp, zslPolicyParams.mTimeouts);
    }

    for (auto const& p : o.vFrameBuilder) {
        if (CC_LIKELY(p != nullptr)) {
            print(printer, *p);
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ZslProcessor::
print(
    android::Printer& printer,
    IFrameBuilder const& o
) -> void
{
    //@TODO: dump IFrameBuilder params
    printer.printFormatLine("(%s)", toString(o.getGroupFrameType()).c_str());
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ZslProcessor::
submitZslRequest(
    ZslRequestParams&& in
) -> std::vector<FrameBuildersOfOneRequest>
{
    CAM_TRACE_CALL();
    FUNC_START_PUBLIC;
    auto const reqNo = in.requestNo;

    //(2) [INVALID_OPERATION]: exceed mMaxRequestSize => ignore request
    if (hasExceedMaxRequest()) { return {}; }

    // ===== Process previous pending request =====
    auto vOut = processZslPendingRequest(reqNo);    // std::vector<FrameBuildersOfOneRequest>

    Mutex::Autolock _l(mLock);
    // ===== Process zsl capture request =====
    // fail if previous pending request fail, still need to put this capture request into pending list.
    if (auto vFrameBuilder = processZslRequest(in, chrono::system_clock::now()); vFrameBuilder.empty() )  // [NOT_ENOUGH_DATA]
    {
        // ===== 1. callback fake shutter =====
        callbackFakeShutter(reqNo);  // not to block following requests

        // ===== 2. add to pending list =====
        mvPendingRequest.emplace_back(
            ZslPendingParams{
                .params = std::move(in),
                .timePoint = chrono::system_clock::now()
            });

        MY_LOGD("[requestNo:%u]: fail to process this zsl capture request [expected pplFrameSz(%zu)], add to pending list [fakeShutter(%" PRId64 "), pendingSz(%zu)] => pending",
                reqNo, mvPendingRequest.back().params.vFrameBuilder.size(), mvFakeShutter[reqNo], mvPendingRequest.size());
    }
    else  // [OK]
    {
        vOut.push_back(std::move(vFrameBuilder));
    }

    FUNC_END_PUBLIC;
    return vOut;
}

/******************************************************************************
 *
 ******************************************************************************/
auto
ZslProcessor::
processZslPendingRequest(
    uint32_t const reqNo
) -> std::vector<FrameBuildersOfOneRequest>
{
    CAM_TRACE_CALL();
    std::vector<FrameBuildersOfOneRequest> vOut;

    Mutex::Autolock _l(mLock);
    // ===== Process pending request =====
    while (mvPendingRequest.size())
    {
        auto const& pendingReq = mvPendingRequest.front();
        if (auto vFrameBuilder = processZslRequest(pendingReq.params, pendingReq.timePoint); vFrameBuilder.empty() )  // [NOT_ENOUGH_DATA]
        {
            MY_LOGD("[requestNo:%u]: fail to process pending request (reqNo:%u) [expected pplFrameSz(%zu); pendingSz(%zu)] => keep pending",
                    reqNo, pendingReq.params.requestNo, pendingReq.params.vFrameBuilder.size(), mvPendingRequest.size());
            break;
        }
        else  // [OK]
        {
            vOut.emplace_back(std::move(vFrameBuilder));
            MY_LOGD("[requestNo:%u]: successfully process pending request (reqNo:%u), remove from pending list [pendingSz(%zu)]",
                    reqNo, pendingReq.params.requestNo, mvPendingRequest.size());

            mvPendingRequest.pop_front();  // remove from pending list
        }
    }

    return vOut;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ZslProcessor::
hasZslPendingRequest() const -> bool
{
    Mutex::Autolock _l(mLock);
    return mvPendingRequest.empty() ? false : true;
}

/******************************************************************************
 *
 ******************************************************************************/
auto
ZslProcessor::
onFrameUpdated(
    uint32_t const requestNo,
    IPipelineBufferSetFrameControl::IAppCallback::Result const& result
) -> void
{
    CAM_TRACE_CALL();
    Mutex::Autolock _l(mLock);

    auto iter_Inflight = find(mvInflightZslRequestNo.begin(), mvInflightZslRequestNo.end(), requestNo);
    if (iter_Inflight != mvInflightZslRequestNo.end() &&
        (result.vAppOutMeta.size() || ((result.nAppOutMetaLeft == 0) && (result.nHalOutMetaLeft == 0))))
    {
        auto fakeIt = mvFakeShutter.find(requestNo);

        // remove MTK_SENSOR_TIMESTAMP to avoid cb shutter twice
        for (auto const& pMetaStream : result.vAppOutMeta)
        {
            if (fakeIt != mvFakeShutter.end() && isAppMeta(pMetaStream->getStreamInfo()->getStreamId()))
            {
                IMetadata* pP1OutAppMeta = pMetaStream->tryWriteLock(LOG_TAG);
                if (CC_UNLIKELY(pP1OutAppMeta == nullptr)) {
                    MY_LOGW("cannot lock metadata");
                } else {
                    if (pP1OutAppMeta->remove(MTK_SENSOR_TIMESTAMP) == OK) {
                        MY_LOGD("(%#" PRIx64 ":%s) remove MTK_SENSOR_TIMESTAMP tag(%#x) [requestNo:%u, fake shutter:%" PRId64 "]",
                                pMetaStream->getStreamInfo()->getStreamId(),
                                pMetaStream->getStreamInfo()->getStreamName(),
                                MTK_SENSOR_TIMESTAMP, fakeIt->first, fakeIt->second);
                    } else {
                        MY_LOGW("(%#" PRIx64 ":%s) can not find MTK_SENSOR_TIMESTAMP tag(%#x)",
                                pMetaStream->getStreamInfo()->getStreamId(),
                                pMetaStream->getStreamInfo()->getStreamName(),
                                MTK_SENSOR_TIMESTAMP);
                    }
                }
                pMetaStream->unlock(LOG_TAG, pP1OutAppMeta);
            }
        }

        // get last meta (only main frame called onFrameUpdated)
        if ((result.nAppOutMetaLeft == 0) && (result.nHalOutMetaLeft == 0))
        {
            if (fakeIt != mvFakeShutter.end())
            {
                mvFakeShutter.erase(fakeIt);
            }
            mvInflightZslRequestNo.erase(iter_Inflight);
        }
    }
}

/******************************************************************************
 *
 ******************************************************************************/
auto
ZslProcessor::
flush() -> std::vector<FrameBuildersOfOneRequest>
{
    CAM_TRACE_CALL();
    FUNC_START_PUBLIC;
    Mutex::Autolock _l(mLock);
    std::vector<FrameBuildersOfOneRequest> vOut;

    for (auto const& pendingReq : mvPendingRequest)
    {
        auto const& rZslReqParam = pendingReq.params;
        FrameBuildersOfOneRequest vFrameBuilder;

        // [1][2] get IFrameBuilder from ZslRequestParams (skip dummy) -> a vector of IFrameBuilder
        for (auto const& pFrameBuilder : rZslReqParam.vFrameBuilder)
        {
            if (isDummyFrame(*pFrameBuilder))  continue;

            if (CC_UNLIKELY(pFrameBuilder == nullptr)) {
                MY_LOGE("bad pFrameBuilder - requestNo:%u", rZslReqParam.requestNo);
                return {};
            }

            auto fakeIt = mvFakeShutter.find(rZslReqParam.requestNo);
            int64_t reprocessTS = (fakeIt == mvFakeShutter.end()) ? 0 : fakeIt->second;
            pFrameBuilder->setReprocessSensorTimestamp(reprocessTS);
            pFrameBuilder->setReprocessFrame(false);

            vFrameBuilder.push_back(pFrameBuilder);
        }

        vOut.push_back(vFrameBuilder);  //@ rZslReqParam.vFrameBuilder w/out dummies
    }
    mvPendingRequest.clear();

    FUNC_END_PUBLIC;
    return vOut;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ZslProcessor::
registerTrackFrameParams(  // pass to setTrackFrameResultParams
    android::sp<IPipelineFrame>const& pPipelineFrame,
    policy::pipelinesetting::RequestResultParams const* pReqResult
) -> void
{
    IPipelineBufferSetFrameControl::castFrom(pPipelineFrame.get())->setTrackFrameResultParams(
        (pReqResult != nullptr) ?
        std::make_shared<TrackFrameResultParams>(
            TrackFrameResultParams{
                .imageStreams = getRootsStreamsForZoom(pReqResult->roots, pReqResult->nodeIOMapImage),
                .metaStreams  = getRootsStreamsForZoom(pReqResult->roots, pReqResult->nodeIOMapMeta)
            }) : mDefaultTrackedParams
    );
}


/******************************************************************************
 *      Implementation
 ******************************************************************************/
auto
ZslProcessor::
processZslRequest(
    ZslRequestParams const& procIn,
    std::chrono::system_clock::time_point const& procReqTime
) -> FrameBuildersOfOneRequest
{
    SelectResult_T selectResult;

    if (selectBuf_Locked(procIn, procReqTime, selectResult) == OK)  // [OK] => vOut
    {
        mvInflightZslRequestNo.push_back(procIn.requestNo);

        // dump selected HBS[F:]
        std::ostringstream oss;
        oss << "zsl requestNo:" << procIn.requestNo << " selected history frames: ";
        for (auto const& hbs : selectResult.vZSLBufSets)
            oss << "[F" << hbs.mRefFrameNo << "] ";
        MY_LOGI("%s", oss.str().c_str());

        return buildFrame_Locked(procIn, selectResult);
    }
    else  // [NOT_ENOUGH_DATA] => pending
    {
        ULogPrinter logPrinter(__ULOG_MODULE_ID, LOG_TAG, DetailsType::DETAILS_DEBUG);
        procIn.pPipelineContext->getHistoryBufferProvider()->dumpBufferState(logPrinter);
        return {};
    }
}


auto
ZslProcessor::
callbackFakeShutter(uint32_t const reqNo) -> void
{
    CAM_TRACE_CALL();
    auto const pCallback = mPipelineModelCallback.promote();
    if (CC_UNLIKELY(!pCallback.get())) {
        MY_LOGE("can not promote pCallback for shutter");
        return;
    }

    // generate sp<IMetaStreamBuffer> with only MTK_SENSOR_TIMESTAMP (w/ the value of mFakeShutterNs)
    sp<IMetaStreamBuffer> pShutterMetaBuffer = HalMetaStreamBufferAllocatorT(mvParsedStreamInfo_P1[0].pHalMeta_DynamicP1.get())();  // take any IMetaStreamInfo to allocate
    IMetadata* meta = pShutterMetaBuffer->tryWriteLock(LOG_TAG);
    IMetadata::setEntry<MINT64>(meta, MTK_SENSOR_TIMESTAMP, (mFakeShutterNs += 1000));  // add 1ms to ensure unique fakeShutter
    pShutterMetaBuffer->unlock(LOG_TAG, meta);
    pShutterMetaBuffer->finishUserSetup();

    // cb fake shutter
    pCallback->onFrameUpdated(
        UserOnFrameUpdated{
            .requestNo = reqNo,
            .nOutMetaLeft = 1,
            .vOutMeta = { pShutterMetaBuffer }
        });
    MY_LOGD("call back fake shutter (requestNo:%u, timestamp:%" PRId64 ")", reqNo, mFakeShutterNs);

    // record fakeShutter  (setReprocessSensorTimestamp @buildFrame_Locked ; remove from vAppOutMeta @onFrameUpdated)
    mvFakeShutter.emplace(reqNo, mFakeShutterNs);
}

auto
ZslProcessor::
buildFrame_Locked(
    ZslRequestParams const& rZslReqParam,  // [in] pre request
    SelectResult_T& rSelectResult          // [in] selected history buffers
) -> FrameBuildersOfOneRequest
{
    CAM_TRACE_CALL();
    FrameBuildersOfOneRequest vFrameBuilder;
    auto& vZSLBufSets    = rSelectResult.vZSLBufSets;
    auto& vUseZslBuffers = rSelectResult.vUseZslBuffers;
    size_t nonDummyCnt = 0;
    size_t nonHistoryCnt = 0;

    // [1] get IFrameBuilder from ZslRequestParams (skip dummy)
    // [2] Zsl processing -> a vector of IFrameBuilder
    for (auto const& pFrameBuilder : rZslReqParam.vFrameBuilder)
    {
        if (isDummyFrame(*pFrameBuilder))  continue;

        bool bUseHistoryBuffer = (vUseZslBuffers.empty() || vUseZslBuffers[nonDummyCnt]);  //Noraml Flow  or  UserSelect using history buffer
        std::vector<android::sp<IImageStreamBuffer>> vpHalImg;
        std::vector<android::sp<IMetaStreamBuffer>> vpAppMetaZsl;
        std::vector<android::sp<IMetaStreamBuffer>> vpHalMetaZsl;

        if (bUseHistoryBuffer) {
            // prepare hal image buffer for each frame
            prepareHalImg(vZSLBufSets[nonDummyCnt - nonHistoryCnt].vImageSet, vpHalImg);

            std::vector<android::sp<IMetaStreamBuffer>> vAppMetaStreamBuffers;
            std::vector<android::sp<IMetaStreamBuffer>> vHalMetaStreamBuffers;
            pFrameBuilder->getAppMetaStreamBuffers(vAppMetaStreamBuffers);
            pFrameBuilder->getHalMetaStreamBuffers(vHalMetaStreamBuffers);

            // prepare app/hal meta buffer for each frame
            prepareMeta(vAppMetaStreamBuffers,
                        vHalMetaStreamBuffers,
                        vZSLBufSets[nonDummyCnt - nonHistoryCnt].vMetaSet,
                        vpAppMetaZsl,
                        vpHalMetaZsl);

        } else {
            nonHistoryCnt++;
        }

        auto fakeIt = mvFakeShutter.find(rZslReqParam.requestNo);
        int64_t reprocessTS = (fakeIt == mvFakeShutter.end()) ?
                getTimeStamp(vZSLBufSets[nonDummyCnt - nonHistoryCnt].vMetaSet) : fakeIt->second;

        pFrameBuilder->setReprocessFrame(bUseHistoryBuffer);
        pFrameBuilder->setReprocessSensorTimestamp(reprocessTS);
        for (auto const& sb : vpHalImg) {
            pFrameBuilder->setHalImageStreamBuffer(sb->getStreamInfo()->getStreamId(), sb);
        }
        for (auto const& sb : vpHalMetaZsl) {
            pFrameBuilder->setHalMetaStreamBuffer(sb->getStreamInfo()->getStreamId(), sb);
        }
        for (auto const& sb : vpAppMetaZsl) {
            pFrameBuilder->setAppMetaStreamBuffer(sb->getStreamInfo()->getStreamId(), sb);
        }

        vFrameBuilder.push_back(pFrameBuilder);

        nonDummyCnt++;
    }

    mFakeShutterNs = getTimeStamp(vZSLBufSets[vZSLBufSets.size() - 1].vMetaSet);  //update fakeShutter
    return vFrameBuilder;
}

void
ZslProcessor::
prepareHalImg(
    const ImageSet_T&  vImageSet,               // [in]
    vector<sp<IImageStreamBuffer>>& vpHalImg    // [out]
)
{
    CAM_TRACE_CALL();
    for (size_t i = 0; i < vImageSet.size(); i++)
    {
        android::sp<IImageStreamBuffer> pHalImg = vImageSet.valueAt(i);
        if (pHalImg == nullptr)
        {
            continue;
        }
        auto pStreamInfo = pHalImg->getStreamInfo();
        if (pStreamInfo == nullptr)
        {
            continue;
        }
        // reset hal image status and user manager
        pHalImg->clearStatus();
        sp<IUsersManager> pUsersManager = new UsersManager(pStreamInfo->getStreamId(),
                                                           pStreamInfo->getStreamName());
        pHalImg->setUsersManager(pUsersManager);
        MY_LOGD("reset hal image usersmanager of (%#" PRIx64 ":%s)->UsersManager(%p)",
                pStreamInfo->getStreamId(), pStreamInfo->getStreamName(), pUsersManager.get());
        vpHalImg.push_back(pHalImg);
    }
}

void
ZslProcessor::
prepareMeta(
    const vector<sp<IMetaStreamBuffer>>& vpAppMeta,    // [in] AppMetas of this frame (zsl req. input)
    const vector<sp<IMetaStreamBuffer>>& vpHalMeta,    // [in] HalMetas of this frame (zsl req. input)
    const MetaSet_T& vMetaSet,                         // [in] metaSet from HBC (history buffers)
    vector<sp<IMetaStreamBuffer>>& vpAppMetaZsl,       // [out] result AppMetas (zsl req. + history)
    vector<sp<IMetaStreamBuffer>>& vpHalMetaZsl        // [out] result HalMetas (zsl req. + history)
)
{
    CAM_TRACE_CALL();
#if 0
    // *** move to IFrameBuilder ***
    // 1. prepare ZSL app control meta from ZSL request input
    for (size_t i=0; i<vpAppMeta.size(); i++) {
        // MY_LOGI("[%zu/%zu]:%s", i, vpAppMeta.size(), vpAppMeta[i]->getStreamInfo()->getStreamName());
        vpAppMetaZsl.push_back(vpAppMeta[i]);
    }

    // 2. prepare ZSL hal control meta from ZSL request input
    for (size_t i=0; i<vpHalMeta.size(); i++) {
        // MY_LOGI("[%zu/%zu]:%s", i, vpHalMeta.size(), vpHalMeta[i]->getStreamInfo()->getStreamName());
        vpHalMetaZsl.push_back(vpHalMeta[i]);
    }
#endif

    // 3. prepare ZSL app/hal result meta from HBC meta
    for (size_t i = 0; i < vMetaSet.size(); i++)
    {
        auto pStreamBuffer = vMetaSet.valueAt(i);                       // meta from HBC (history buffer)
        auto const streamId = pStreamBuffer->getStreamInfo()->getStreamId();  // streamId of that meta
        sp<IMetaStreamBuffer> pMetaBuffer;

        // append P1 control meta data to P1 outpout Hal Meta data
        appendToP1OutHalMeta(pStreamBuffer, vpHalMeta, pMetaBuffer);
        // append app meta from Meta:App:Control to App:Meta:DynamicP1_mainX
        appendToP1OutAppMeta(vpAppMeta, pMetaBuffer);

        if (isAppMeta(streamId)) {
            MY_LOGD("MetaStreamBuffer Push to AppMeta: (%#" PRIx64 ")", streamId);
            vpAppMetaZsl.push_back(pMetaBuffer);
        } else if (isHalMeta(streamId)) {
            MY_LOGD("MetaStreamBuffer Push to HalMeta: (%#" PRIx64 ")", streamId);
            vpHalMetaZsl.push_back(pMetaBuffer);
        } else {
            MY_LOGW("Get unknown IMetaStreamBuffer, (%#" PRIx64 ")", streamId);
        }
    }
}

auto
ZslProcessor::
appendToP1OutAppMeta(
    const vector<sp<IMetaStreamBuffer>>& vpAppMeta,  // [in] AppMetas of this frame (zsl req. input)
    sp<IMetaStreamBuffer>& pMetaBuffer               // [in/out]
) -> void
{
    CAM_TRACE_CALL();
    if (auto const streamId = pMetaBuffer->getStreamInfo()->getStreamId();
        isAppMeta(streamId))
    {
        static constexpr int tags_UINT8[] = {
        // ========== necessary post processing tag ==========
            MTK_CONTROL_CAPTURE_INTENT,
            MTK_NOISE_REDUCTION_MODE,
            MTK_COLOR_CORRECTION_ABERRATION_MODE,
            MTK_COLOR_CORRECTION_MODE,
            MTK_TONEMAP_MODE,
            MTK_SHADING_MODE,
            MTK_HOT_PIXEL_MODE,
            MTK_EDGE_MODE,
        // ========== customize tag ==========
        };
        static constexpr int tags_INT32[] = {
            MTK_JPEG_ORIENTATION
        };
        static constexpr int tags_FLOAT[] = {
        };
        static constexpr int tags_INT64[] = {
        };
        static constexpr int tags_DOUBLE[] = {
        };
        static constexpr int tags_Rational[] = {
        };
        static constexpr int tags_Point[] = {
        };
        static constexpr int tags_Size[] = {
        };
        static constexpr int tags_Rect[] = {
        };
        static constexpr int tags_Metadata[] = {
        };
        static constexpr int tags_Memory[] = {
        };

        if (auto it = find_if(vpAppMeta.begin(), vpAppMeta.end(),
                [](auto const& pAppMeta){ return pAppMeta->getStreamInfo()->getStreamId() == eSTREAMID_META_APP_CONTROL; });
            it != vpAppMeta.end())
        {
            MY_LOGD("(%#" PRIx64 ":%s) update metadata from (%#" PRIx64 ":%s)" ,
                    streamId, pMetaBuffer->getStreamInfo()->getStreamName(),
                    eSTREAMID_META_APP_CONTROL, (*it)->getStreamInfo()->getStreamName());
            IMetadata* pAppInMeta = (*it)->tryReadLock(LOG_TAG);
            IMetadata* pAppOutMeta = pMetaBuffer->tryWriteLock(LOG_TAG);
            for (uint32_t tag : tags_UINT8)  updateTag<MUINT8>(tag, pAppInMeta, pAppOutMeta);
            for (uint32_t tag : tags_INT32)  updateTag<MINT32>(tag, pAppInMeta, pAppOutMeta);
            for (uint32_t tag : tags_FLOAT)  updateTag<MFLOAT>(tag, pAppInMeta, pAppOutMeta);
            for (uint32_t tag : tags_INT64)  updateTag<MINT64>(tag, pAppInMeta, pAppOutMeta);
            for (uint32_t tag : tags_DOUBLE)  updateTag<MDOUBLE>(tag, pAppInMeta, pAppOutMeta);
            for (uint32_t tag : tags_Rational)  updateTag<MRational>(tag, pAppInMeta, pAppOutMeta);
            for (uint32_t tag : tags_Point)  updateTag<MPoint>(tag, pAppInMeta, pAppOutMeta);
            for (uint32_t tag : tags_Size)  updateTag<MSize>(tag, pAppInMeta, pAppOutMeta);
            for (uint32_t tag : tags_Rect)  updateTag<MRect>(tag, pAppInMeta, pAppOutMeta);
            for (uint32_t tag : tags_Metadata)  updateTag<IMetadata>(tag, pAppInMeta, pAppOutMeta);
            for (uint32_t tag : tags_Memory)  updateTag<IMetadata::Memory>(tag, pAppInMeta, pAppOutMeta);
            (*it)->unlock(LOG_TAG, pAppInMeta);
            pMetaBuffer->unlock(LOG_TAG, pAppOutMeta);
        }
    }
}

auto
ZslProcessor::
appendToP1OutHalMeta(
    android::sp<IMetaStreamBuffer>& pStreamBuffer,     // [in] meta from HBC (history buffer)
    const vector<sp<IMetaStreamBuffer>>& vpHalMeta,    // [in] HalMetas of this frame (zsl req. input)
    sp<IMetaStreamBuffer>& pMetaBuffer                 // [out]
) -> void
{
    CAM_TRACE_CALL();
    IMetadata* pMeta = pStreamBuffer->tryReadLock(LOG_TAG);
    auto pStreamInfo = const_cast<IMetaStreamInfo*>(pStreamBuffer->getStreamInfo());
    auto const streamId = pStreamInfo->getStreamId();
    bool bAppend = false;

    // append P1 control meta data to P1 outpout Hal Meta data
    for (auto const& p1_mainX : mvParsedStreamInfo_P1) {
        if (streamId == p1_mainX.pHalMeta_DynamicP1->getStreamId())
        {
            // ===== find Hal_DynamicP1 in this HBC meta =====
            IMetadata p1OutHalMeta(*pMeta);
            //
            for (auto const& pHalMeta : vpHalMeta) {
                if (pHalMeta->getStreamInfo()->getStreamId() == p1_mainX.pHalMeta_Control->getStreamId()) {
                    // ===== find Hal_Control in (zsl req. input) =====
                    IMetadata* pP1CtrlMeta = pHalMeta->tryReadLock(LOG_TAG);
                    p1OutHalMeta += *pP1CtrlMeta;
                    pHalMeta->unlock(LOG_TAG, pP1CtrlMeta);
                    MY_LOGD("append P1 IN control meta data (%#" PRIx64 ":%s) to P1 Out hal dynamic meta data (%#" PRIx64 ":%s)",
                            p1_mainX.pHalMeta_Control->getStreamId(), p1_mainX.pHalMeta_Control->getStreamName(),
                            streamId, p1_mainX.pHalMeta_DynamicP1->getStreamName());
                    break;
                }
            }
            pMetaBuffer = HalMetaStreamBufferAllocatorT(pStreamInfo)(p1OutHalMeta);
            bAppend = true;
        }
    }

    // other meta data do not append, just copy from HBC buffer record
    if (bAppend == false) {
        pMetaBuffer = HalMetaStreamBufferAllocatorT(pStreamInfo)(*pMeta);
    }
    pStreamBuffer->unlock(LOG_TAG,pMeta);
}

auto
ZslProcessor::
selectBuf_Locked(  //@TODO: refactor
    ZslRequestParams const& rZslReqParam,     // [in]
    std::chrono::system_clock::time_point const& ZslReqTime,  // [in]
    SelectResult_T& rSelectResult                             // [out]
) -> int32_t
{
    CAM_TRACE_CALL();
    Vector<BufferSet_T> vHBC_Buf_Sets;  //@[AS-IS] hbc
    std::list<NSPipelineContext::IFrameBufferManager::HistoryBufferSet> candidateList;  //@[TO-BE] fbm
    auto& vZSLBufSets = rSelectResult.vZSLBufSets;
    //auto& vUseZslBuffers = rSelectResult.vUseZslBuffers;  //unused
    //
    auto const reqNo = rZslReqParam.requestNo;
    auto const& pHBP = rZslReqParam.pPipelineContext->getHistoryBufferProvider();
    std::unique_ptr<void, std::function<void(void*)>> autoEndSelect;
    //
    auto const& pCapParams = rZslReqParam.pCapParams;
    auto const zslPolicy = (mZSLForcePolicy == ZSLForcePolicyDisable)?
                            pCapParams->zslPolicyParams.mPolicy : mZSLForcePolicy;
    auto const reqTimeOut = (mZSLForceTimeOutMs == ZSLForceTimeoutDisable)?
                            pCapParams->zslPolicyParams.mTimeouts : mZSLForceTimeOutMs;
    auto const reqFrameSz = pCapParams->subFrames.size() + 1;  // main + sub  //@TODO: pre-sub
    MY_LOGI("+ : [reqNo:%u, reqFrameSz:%zu, policy:%#x]", reqNo, reqFrameSz, zslPolicy);

    // 1. get history buffers
    std::unordered_set<StreamId_T> requiredImages, requiredMetas;
    if (rZslReqParam.isZoom) {  // Zoom specify required Streams
        requiredImages = getRootsStreamsForZoom(pCapParams->mainFrame->roots, pCapParams->mainFrame->nodeIOMapImage);
        requiredMetas = getRootsStreamsForZoom(pCapParams->mainFrame->roots, pCapParams->mainFrame->nodeIOMapMeta);
    } else {
        requiredImages = mDefaultTrackedParams->imageStreams;
        requiredMetas = mDefaultTrackedParams->metaStreams;
    }
    MY_LOGI("zsl required streams: %s %s",
            toString_StreamIds(requiredImages, "Image").c_str(),
            toString_StreamIds(requiredMetas, "Meta").c_str());
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Lambda Functions
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    bool lambdaLog = mLogLevel;  //@ remove?
    auto acquired_from_fbm = [
        &pHBP, &candidateList, &autoEndSelect, &zslPolicy, this,
        &imageStreams = requiredImages, &metaStreams = requiredMetas
    ](auto& destVec) -> int
    {
        auto opt = pHBP->beginSelect();
        autoEndSelect = decltype(autoEndSelect)(pHBP.get(), [pHBP](void*){ pHBP->endSelect(); });
        if (opt)
        {
            auto& tmpList = *opt;
            for (auto it = tmpList.begin(); it != tmpList.end();)
            {
                //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                // Lambda Functions
                //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                auto isError = [](auto const& vImage, bool& rbKeep) -> bool {
                    for (auto const& sb : vImage) {
                        if (sb->getStatus() & STREAM_BUFFER_STATUS::ERROR) {
                            rbKeep = false;
                            MY_LOGI("(%#" PRIx64 ":%s): error image - return and release",
                                    sb->getStreamInfo()->getStreamId(), sb->getStreamInfo()->getStreamName());
                            return true;
                        }
                    }
                    return false;
                };
                //
                auto isPDPureRaw = [this](auto const& vHalMeta, bool& rbKeep) -> bool {
                    if (!isPDProcessRawBuffer(vHalMeta)) {
                        rbKeep = false;
                        MY_LOGD("PD_pure_raw - return and release");
                        return true;
                    }
                    return false;
                };
                //
                auto containsRequired = [](auto const& requiredStreams, auto const& vSb) -> bool
                {
                    for (auto const& streamId : requiredStreams) {
                        if (find_if(vSb.begin(), vSb.end(), [streamId](const auto& sb)
                            { return streamId == sb->getStreamInfo()->getStreamId(); }) == vSb.end())
                            return false;
                    }
                    return true;
                };
                //
                auto convertRequiredAndReleaseOthers = []
                    (auto& vSb, auto& vSet, const auto& requiredStreams, const auto frameNo/*DEBUG*/)
                {
                    std::ostringstream oss;
                    oss << std::showbase << std::hex;
                    for (auto it = vSb.begin(); it != vSb.end();) {
                        auto& sb = *it;
                        auto streamId = sb->getStreamInfo()->getStreamId();
                        if (requiredStreams.find(streamId) != requiredStreams.end())
                        {
                            vSet.add(streamId, sb);
                            ++it;
                        }
                        else
                        {
                            oss << streamId << " ";
                            releaseBuffer(sb);
                            it = vSb.erase(it);
                        }
                    }
                    MY_LOGD_IF(oss.str().size(), "[F:%u] not required { %s} release sb", frameNo, oss.str().c_str());
                };
                //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                auto& hbs = *it;
                //FIXME: concatenate HalMeta + AppMeta
                auto vMetaSb = hbs.vpHalMetaStreamBuffers;
                vMetaSb.insert(vMetaSb.end(), hbs.vpAppMetaStreamBuffers.begin(), hbs.vpAppMetaStreamBuffers.end());
                //
                if (bool bKeep = true;
                    !isError(hbs.vpHalImageStreamBuffers, bKeep) &&
                    ((zslPolicy & eZslPolicy_PD_ProcessedRaw) ? !isPDPureRaw(hbs.vpHalMetaStreamBuffers, bKeep) : true) &&
                    containsRequired(imageStreams, hbs.vpHalImageStreamBuffers) &&
                    containsRequired(metaStreams, vMetaSb))
                {
                    auto& destSet = destVec.editItemAt(destVec.add());
                    destSet.mRefFrameNo = hbs.frameNo;
                    // if only need partial: take required & release others
                    convertRequiredAndReleaseOthers(hbs.vpHalImageStreamBuffers, destSet.vImageSet, imageStreams, hbs.frameNo);
                    convertRequiredAndReleaseOthers(hbs.vpHalMetaStreamBuffers, destSet.vMetaSet, metaStreams, hbs.frameNo);
                    convertRequiredAndReleaseOthers(hbs.vpAppMetaStreamBuffers, destSet.vMetaSet, metaStreams, hbs.frameNo);
                    ++it;
                }
                else
                {
                    MY_LOGI("[F:%u] does not contain all required buffers, return back", hbs.frameNo);
                    pHBP->returnUnselectedSet(
                        IFrameBufferManager::ReturnUnselectedSet{
                            .hbs = std::move(hbs),
                            .keep = bKeep
                        });
                    it = tmpList.erase(it);
                }
            }
            candidateList = *std::move(opt);
            return OK;
        }
        return NOT_ENOUGH_DATA;
    };
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    auto cancel_to_fbm = [&pHBP, &candidateList, lambdaLog](auto& rBufSet)
    {
        auto it = find_if(candidateList.begin(), candidateList.end(),
                          [frameNo = rBufSet.mRefFrameNo](const auto& hbs)
                          { return frameNo == hbs.frameNo; });

        if (it != candidateList.end()) {
            pHBP->returnUnselectedSet(
                NSPipelineContext::IFrameBufferManager::ReturnUnselectedSet{
                    .hbs = std::move(*it),
                    .keep = rBufSet.bKeep
                });
            candidateList.erase(it);
        } else if (lambdaLog) {  //@TODO: modify
            MY_LOGD("can't find BufSet[F:%u] in candidateList", rBufSet.mRefFrameNo);
            std::ostringstream oss;
            oss << "dump candidateList: ";
            for (auto const& hbs : candidateList)
                oss << "[R" << hbs.requestNo << " F" << hbs.frameNo << "] ";
            MY_LOGD("%s", oss.str().c_str());
        }

        // clear
        rBufSet.vImageSet.clear();
        rBufSet.vMetaSet.clear();
    };
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    if (acquired_from_fbm(vHBC_Buf_Sets) != OK) {
        MY_LOGW("- : can not get any buffer from fbm");
        return NOT_ENOUGH_DATA;  //@TODO: timeout (duration)
    }

    if (mLogLevel) {  //@ remove?
        std::ostringstream oss;
        oss << "dump vHBC_Buf_Sets: ";
        for (auto const& rBufSet : vHBC_Buf_Sets)
            oss << rBufSet.mRefFrameNo << " ";
        MY_LOGD("%s", oss.str().c_str());
    }

    // 1-a. check available history buffers size
    if (vHBC_Buf_Sets.size() < reqFrameSz) {
        MY_LOGW("- : availableSz(%zu) < reqFrameSz(%zu), cancel back, pending", vHBC_Buf_Sets.size(), reqFrameSz);
        for (auto& rBufSet : vHBC_Buf_Sets)    cancel_to_fbm(rBufSet);
        return NOT_ENOUGH_DATA;  //@TODO: timeout (duration)
    }

    if (zslPolicy & eZslPolicy_UserSelect)
    {
        MY_LOGW("do not support userSelect");
    }
    else
    {
        size_t startIndex = 0;

        // 2. calculate ZSD index to begin w/
        if (zslPolicy & eZslPolicy_ZeroShutterDelay)
        {
            int64_t ReqTimeStampDiff = (pCapParams->zslPolicyParams.mTimestamp == -1) ?
                                       mZSLTimeDiffMs : pCapParams->zslPolicyParams.mTimestamp;
            MY_LOGI_IF((pCapParams->zslPolicyParams.mTimestamp != -1),
                       "use ZslPolicyParams timestamp diff(%" PRId64 ")", ReqTimeStampDiff);
            updateStartIndexForZSD(vHBC_Buf_Sets, ZslReqTime, ReqTimeStampDiff, startIndex);
            // subtract subframes size for multiframe feature (eg. mfnr, ainr), ZSD buffer could still get in the first round.
            if (startIndex >= pCapParams->subFrames.size())  //@TODO: pre-sub??
                startIndex -= pCapParams->subFrames.size();
            MY_LOGD("ZSD Capture: start with buffer[%zu/%zu][F:%u] (reqFrameSz: %zu)",
                startIndex, vHBC_Buf_Sets.size(), vHBC_Buf_Sets[startIndex].mRefFrameNo, reqFrameSz);
        }

        // 3. check quality for policies
        bool bContinuousFrame = (zslPolicy & eZslPolicy_ContinuousFrame);
        bool bPD_ProcessedRaw = (zslPolicy & eZslPolicy_PD_ProcessedRaw);
        std::set<size_t> selectedIndexes;

        for (size_t i = startIndex; i < vHBC_Buf_Sets.size(); i++)
        {
            if (checkQuality(vHBC_Buf_Sets[i].vMetaSet, zslPolicy, vHBC_Buf_Sets.editItemAt(i).bKeep)) {
                selectedIndexes.emplace_hint(selectedIndexes.end(), i);
                MY_LOGI("buffer[%zu/%zu][F:%u]: selected", i, vHBC_Buf_Sets.size(), vHBC_Buf_Sets[i].mRefFrameNo);
            } else if (bContinuousFrame && !bPD_ProcessedRaw) {
                // PD_ProcessedRaw (isp3.0) does not follow ContinuousFrame policy
                selectedIndexes.clear();
                MY_LOGD("buffer[%zu/%zu][F:%u]: clear all selected record [bContinuousFrame(%d), bPD_ProcessedRaw(%d)]",
                        i, vHBC_Buf_Sets.size(), vHBC_Buf_Sets[i].mRefFrameNo, bContinuousFrame, bPD_ProcessedRaw);
            }

            // 4-a. get enough buffers [Success]
            if (selectedIndexes.size() == reqFrameSz)
            {
                // 4-a-1. add selected buffers to output
                for (auto const& j : selectedIndexes) {
                    vZSLBufSets.push_back(vHBC_Buf_Sets[j]);
                }

                // 4-a-2. cancel unused buffers back to source
                for (size_t j = 0; j < vHBC_Buf_Sets.size(); j++) {
                    if (selectedIndexes.find(j) == selectedIndexes.end()) {
                        MY_LOGD("buffer[%zu/%zu][F:%u]: cancel back", j, vHBC_Buf_Sets.size(), vHBC_Buf_Sets[j].mRefFrameNo);
                        cancel_to_fbm(vHBC_Buf_Sets.editItemAt(j));
                    } else {
                        MY_LOGD("buffer[%zu/%zu][F:%u]: selected, do not cancel", j, vHBC_Buf_Sets.size(), vHBC_Buf_Sets[j].mRefFrameNo);
                    }
                }

                MY_LOGI("- : get enough qualified history buffers [reqNo:%u, reqFrameSz:%zu, policy:%#x]", reqNo, reqFrameSz, zslPolicy);
                return OK;
            }
        }
    }

    // 4-b/c. not success, check if timeout
    if (auto durationMs = getDurationMs(ZslReqTime); durationMs >= reqTimeOut)
    {
        // 4-b. timeout, select buffers w/out checking their quality
        bool bEnough = false;
        for (size_t i = 0; i < vHBC_Buf_Sets.size(); i++)
        {
            if (!bEnough && vHBC_Buf_Sets[i].bKeep) {
                MY_LOGD("buffer[%zu/%zu][F:%u]: selected, do not cancel - timeout flow", i, vHBC_Buf_Sets.size(), vHBC_Buf_Sets[i].mRefFrameNo);
                vZSLBufSets.push_back(vHBC_Buf_Sets[i]);
                if (vZSLBufSets.size() == reqFrameSz)   bEnough = true;
            } else {
                MY_LOGD("buffer[%zu/%zu][F:%u]: cancel back - timeout flow [bEnough(%d), bKeep(%d)]",
                        i, vHBC_Buf_Sets.size(), vHBC_Buf_Sets[i].mRefFrameNo, bEnough, vHBC_Buf_Sets[i].bKeep);
                cancel_to_fbm(vHBC_Buf_Sets.editItemAt(i));
            }
        }

        // 4-b-a. get enough buffers, successfully timeout [Timeout]
        if (vZSLBufSets.size() == reqFrameSz) {
            MY_LOGI("- : timeout(%" PRId64 " >= %" PRId64 "ms), get enough history buffers (w/out checking quality) [reqNo:%u, reqFrameSz:%zu, policy:%#x]",
                    durationMs, reqTimeOut, reqNo, reqFrameSz, zslPolicy);
            return OK;
        }

        // 4-b-b. fail to get enough buffers, keep pending [Pending]
        MY_LOGW("- : timeout(%" PRId64 " >= %" PRId64 "ms), still cannot get enough history buffers (w/out checking quality), pending [reqNo:%u, reqFrameSz:%zu (> availableSz:%zu), policy:%#x]",
                durationMs, reqTimeOut, reqNo, reqFrameSz, vZSLBufSets.size(), zslPolicy);
        for (auto& rBufSet : vZSLBufSets)   cancel_to_fbm(rBufSet);
    }
    else  // 4-c. haven't timeout [Pending]
    {
        for (auto& rBufSet : vHBC_Buf_Sets)    cancel_to_fbm(rBufSet);
        MY_LOGI("- : cannot get enough qualified history buffers, haven't timeout(%" PRId64 " < %" PRId64 "ms), pending [reqNo:%u, reqFrameSz:%zu, policy:%#x]",
                durationMs, reqTimeOut, reqNo, reqFrameSz, zslPolicy);
    }

    vZSLBufSets.clear();
    return NOT_ENOUGH_DATA;  //[Pending]
}


auto
ZslProcessor::
updateStartIndexForZSD(
    const Vector<BufferSet_T>& vHBC_Buf_Sets,            // [in]
    const chrono::system_clock::time_point& ZslReqTime,  // [in]
    int64_t ReqTimeStampDiff,                            // [in] timeStampDiff tuned by platform (!?)
    size_t& startIndex                                   // [out]
) -> void
{
    CAM_TRACE_CALL();
    int64_t BufTimeStamp = 0, LastBufTimeStamp = 0;  //(ms)

    // convert ZslReqTime (chrono) to ReqTimeStamp (MTK_SENSOR_TIMESTAMP)    //TODO: fix?
    // now - ZslReqTime = getTimeStamp(lastAcquiredBuf) - ReqTimeStamp  [unit: ms]
    // => ReqTimeStamp = getTimeStamp(lastAcquiredBuf) - (now - ZslReqTime)
    int64_t durationMs = getDurationMs(ZslReqTime);
    int64_t newestBufTimeMs = getTimeStamp(vHBC_Buf_Sets[vHBC_Buf_Sets.size() - 1].vMetaSet) / 1000000;  //(ms)
    int64_t ReqTimeStamp = newestBufTimeMs - durationMs;
    int64_t SelTimeStamp = ReqTimeStamp - ReqTimeStampDiff;

    MY_LOGD1("ReqTimeStamp(%" PRId64 ") = newestBufTimeMs(%" PRId64 ") - durationMs(%" PRId64 ")",
            ReqTimeStamp, newestBufTimeMs, durationMs);

    bool bFind = false;
    for (size_t i = 0; i < vHBC_Buf_Sets.size(); i++) {
        BufTimeStamp = getTimeStamp(vHBC_Buf_Sets[i].vMetaSet) / 1000000;  //(ms)
        MY_LOGD("buffer[%zu/%zu][F:%u]: bufTS(%" PRId64 ") >= SelTS(%" PRId64 ")=reqTS(%" PRId64 ")-Diff(%" PRId64 ")",
                i, vHBC_Buf_Sets.size(), vHBC_Buf_Sets[i].mRefFrameNo,
                BufTimeStamp, SelTimeStamp, ReqTimeStamp, ReqTimeStampDiff);

        int64_t shutter_delay = BufTimeStamp - SelTimeStamp;
        if (shutter_delay >= 0 && !bFind) {
            startIndex = i;
            if (i == 0 && shutter_delay > MAX_TOLERANCE_SHUTTERDELAY_MS) {
                MY_LOGW("not enough depth of history buffer: delay(%" PRId64 ") vs. tolerance(%" PRId64 ")",
                        shutter_delay, MAX_TOLERANCE_SHUTTERDELAY_MS);
            } else if (i > 0) {
                LastBufTimeStamp = getTimeStamp(vHBC_Buf_Sets[i - 1].vMetaSet) / 1000000;  //(ms)
                if (shutter_delay > abs(LastBufTimeStamp - SelTimeStamp))
                    startIndex = (i - 1);
                MY_LOGI("check prev: lastTS(%" PRId64 ")-delay(%" PRId64 ") vs. bufTS(%" PRId64 ")-delay(%" PRId64 ")",
                        LastBufTimeStamp, LastBufTimeStamp - SelTimeStamp,
                        BufTimeStamp, shutter_delay);
            }
            bFind = true;
        }
    }

    if (!bFind) {
        MY_LOGW("future timestamp! use newest HBS [timestamp(ms):%" PRId64 "]", BufTimeStamp);
        startIndex = vHBC_Buf_Sets.size() - 1;
    }

    auto timestamp = getTimeStamp(vHBC_Buf_Sets[startIndex].vMetaSet) / 1000000;  //(ms)
    MY_LOGI("zsl select buffer[%zu/%zu][F:%u]: timestamp(ms):%" PRId64,
            startIndex, vHBC_Buf_Sets.size(), vHBC_Buf_Sets[startIndex].mRefFrameNo, timestamp);
    String8 const str = String8::format("%s|buffer[%zu/%zu][F:%u]: timestamp(ms):%" PRId64,
                                        __FUNCTION__, startIndex, vHBC_Buf_Sets.size(), vHBC_Buf_Sets[startIndex].mRefFrameNo, timestamp);
    CAM_TRACE_BEGIN(str.string());
    CAM_TRACE_END();
}

/******************************************************************************
 *  Check Quality
 ******************************************************************************/
auto
ZslProcessor::
checkQuality(
    const MetaSet_T &rvResult,  // [in]
    const int32_t zslPolicy,    // [in]
    bool& rbKeep  __unused      // [out]
) -> bool
{
    CAM_TRACE_CALL();
    MBOOL ret = true;

    if (zslPolicy & eZslPolicy_AfState) {
        ret = isAfOkBuffer(rvResult);
        if (ret == false) return ret;
    }

    if (zslPolicy & eZslPolicy_AeState) {
        ret = isAeOkBuffer(rvResult);
        if (ret == false) return ret;
    }

    if (zslPolicy & eZslPolicy_DualFrameSync) {
        ret = isFameSyncOkBuffer(rvResult);
        if (ret == false) return ret;
    }

    return ret;
}

bool
ZslProcessor::
isAfOkBuffer(const MetaSet_T &rvResult)
{
    CAM_TRACE_CALL();
    struct AfItem_T {
        MUINT8 afMode, afState, lensState;
        StreamId_T streamId;  //DEBUG
        bool bResult = true;
        // ctor
        AfItem_T(MUINT8 mode = 0, MUINT8 state = 0, MUINT8 lens = 0, StreamId_T stream = -1)
            : afMode(std::move(mode)), afState(std::move(state)), lensState(std::move(lens)), streamId(std::move(stream)) {}
    };
    std::vector<AfItem_T> vAfItem;  // size = no. of AppMetas in rvResult  [results: OR]
    bool bFind = false;

    // ===== get af info. =====
    for (size_t i = 0; i < rvResult.size(); i++)
    {
        if (auto const streamId = rvResult[i]->getStreamInfo()->getStreamId();
            isAppMeta(streamId))
        {
            // get AF & LENS state from MTK_CONTROL_AF_STATE & MTK_LENS_STATE
            IMetadata* meta = rvResult[i]->tryReadLock(LOG_TAG);
            if (CC_UNLIKELY(meta == nullptr)) {
                MY_LOGW("cannot lock metadata");
                continue;
            }
            IMetadata::IEntry const eAfMode = meta->entryFor(MTK_CONTROL_AF_MODE);
            IMetadata::IEntry const eAfState = meta->entryFor(MTK_CONTROL_AF_STATE);
            IMetadata::IEntry const eLensState = meta->entryFor(MTK_LENS_STATE);
            if (!eAfMode.isEmpty() && !eAfState.isEmpty() && !eLensState.isEmpty()) {
                vAfItem.emplace_back(eAfMode.itemAt(0, Type2Type<MUINT8>()),
                                     eAfState.itemAt(0, Type2Type<MUINT8>()),
                                     eLensState.itemAt(0, Type2Type<MUINT8>()),
                                     streamId);
                bFind = true;
                MY_LOGD1("(%#" PRIx64 ":%s): find AF meta [afMode(%d) afState(%d) lensState(%d)]",
                        streamId, rvResult[i]->getStreamInfo()->getStreamName(),
                        vAfItem.back().afMode, vAfItem.back().afState, vAfItem.back().lensState);
            } else {
                MY_LOGW("(%#" PRIx64 ":%s): cannot find AF meta [tag.count(%u), afMode.isEmpty(%d) afState.isEmpty(%d) lensState.isEmpty(%d)]",
                        streamId, rvResult[i]->getStreamInfo()->getStreamName(),
                        meta->count(), eAfMode.isEmpty(), eAfState.isEmpty(), eLensState.isEmpty());
            }
            rvResult[i]->unlock(LOG_TAG, meta);
        }
    }

    if (!bFind) {
        MY_LOGW("Can't Find MTK_CONTROL_AF_STATE or MTK_LENS_STATE");
        return false;
    }

    // ===== check af tags. =====
    for (auto& afItem : vAfItem)  // all metas should be ok to return true
    {
        // *** Reserve this condition (AP vs. HAL) to align w/ hal1. ***
        if (afItem.afMode == MTK_CONTROL_AF_MODE_AUTO ||
            afItem.afMode == MTK_CONTROL_AF_MODE_MACRO)
        {  //(1) By AP
            if (afItem.afState == MTK_CONTROL_AF_STATE_INACTIVE ||
                afItem.afState == MTK_CONTROL_AF_STATE_PASSIVE_SCAN ||
                afItem.afState == MTK_CONTROL_AF_STATE_ACTIVE_SCAN ||
                afItem.lensState == MTK_LENS_STATE_MOVING)
            {  //(1.1)
                if (afItem.afState == MTK_CONTROL_AF_STATE_INACTIVE &&
                    afItem.lensState == MTK_LENS_STATE_STATIONARY)
                {  //(1.1.1)
                    // modification1: no statinary count in hal3 version.
                    // this information would be passed by af/lens module if needed.
                    MY_LOGD("(%#" PRIx64 "): AF ok [afMode(%d) afState(%d) lensState(%d)]",
                            afItem.streamId, afItem.afMode, afItem.afState, afItem.lensState);
                } else {
                    MY_LOGW("(%#" PRIx64 "): AF fail [afMode(%d) afState(%d) lensState(%d)]",
                            afItem.streamId, afItem.afMode, afItem.afState, afItem.lensState);
                    afItem.bResult = false;
                }
            }

        } else if (afItem.afMode == MTK_CONTROL_AF_MODE_CONTINUOUS_VIDEO ||
                   afItem.afMode == MTK_CONTROL_AF_MODE_CONTINUOUS_PICTURE)
        {  //(2) By HAL
            if (afItem.afState == MTK_CONTROL_AF_STATE_INACTIVE ||
                afItem.afState == MTK_CONTROL_AF_STATE_PASSIVE_SCAN ||
                afItem.afState == MTK_CONTROL_AF_STATE_ACTIVE_SCAN ||
                afItem.lensState == MTK_LENS_STATE_MOVING)
            {  //(2.1)
                // modification2: no inactive count in hal3 version.
                // we do not need to count inactive count  default afstate is NOT_FOCUS
                if (afItem.afState == MTK_CONTROL_AF_STATE_INACTIVE &&
                    afItem.lensState == MTK_LENS_STATE_STATIONARY)
                {  //(2.1.1)
                    // modification1: no statinary count in hal3 version.
                    // this information would be passed by af/lens module if needed.
                    MY_LOGD("(%#" PRIx64 "): AF ok [afMode(%d) afState(%d) lensState(%d)]",
                            afItem.streamId, afItem.afMode, afItem.afState, afItem.lensState);
                } else {
                    MY_LOGW("(%#" PRIx64 "): AF fail [afMode(%d) afState(%d) lensState(%d)]",
                            afItem.streamId, afItem.afMode, afItem.afState, afItem.lensState);
                    afItem.bResult = false;
                }
            }
        }
    }

    // ===== final result =====
    bool result = false;
    for (auto const& afItem : vAfItem) {
        result |= afItem.bResult;  // [OR]
    }
    MY_LOGD("=> AF %s! [AfItemSz(%zu)]", (result ? "ok" : "fail"), vAfItem.size());
    return result;
}

bool
ZslProcessor::
isAeOkBuffer(const MetaSet_T &rvResult)
{
    CAM_TRACE_CALL();
    std::vector< std::pair<MUINT8, StreamId_T> > vAeState;  // size = no. of AppMetas in rvResult  [results: AND]
    bool bFind = false;

    // ===== get ae info. =====
    for (size_t i = 0; i < rvResult.size(); i++)
    {
        if (auto const streamId = rvResult[i]->getStreamInfo()->getStreamId();
            isAppMeta(streamId))
        {
            IMetadata* meta = rvResult[i]->tryReadLock(LOG_TAG);
            if (CC_UNLIKELY(meta == nullptr)) {
                MY_LOGW("cannot lock metadata");
                continue;
            }

            IMetadata::IEntry const entry = meta->entryFor(MTK_CONTROL_AE_STATE);
            if (!entry.isEmpty()) {
                vAeState.emplace_back(entry.itemAt(0, Type2Type<MUINT8>()), streamId);
                bFind = true;
                MY_LOGD1("(%#" PRIx64 ":%s): find AE meta [aeState(%d)]",
                        streamId, rvResult[i]->getStreamInfo()->getStreamName(), vAeState.back().first);
            } else {
                MY_LOGW("(%#" PRIx64 ":%s): cannot find AE meta [tag.count(%u), aeState.isEmpty(%d)]",
                        streamId, rvResult[i]->getStreamInfo()->getStreamName(), meta->count(), entry.isEmpty());
            }
            rvResult[i]->unlock(LOG_TAG, meta);
        }
    }

    if (!bFind) {
        MY_LOGW("Can't Find MTK_CONTROL_AE_STATE");
        return false;
    }

    // ===== check ae tag. =====
    for (auto const& aeState : vAeState) {  // all metas should be ok to return true
        // if AE CONVERGED or locked, it is ae ok buffer
        if ((aeState.first == MTK_CONTROL_AE_STATE_CONVERGED) ||
            (aeState.first == MTK_CONTROL_AE_STATE_LOCKED)) {
            MY_LOGD("(%#" PRIx64 "): AE ok [aeState(%d)]", aeState.second, aeState.first);
        } else {
            MY_LOGW("(%#" PRIx64 "): AE failed [aeState(%d)]", aeState.second, aeState.first);
            return false;
        }
    }

    MY_LOGD("=> AE ok! [AeStateSz(%zu)]", vAeState.size());
    return true;
}

bool
ZslProcessor::
isFameSyncOkBuffer(const MetaSet_T &rvResult)
{
    CAM_TRACE_CALL();
    int64_t frameSyncState = 0;  // only check once (suppose main1 & main2 should have the same result)  [results: shortcut OR]
    bool bFind = false;
    StreamId_T streamId = -1;

    // ===== get frameSync info. =====
    for (size_t i = 0; i < rvResult.size(); i++)
    {
        if (streamId = rvResult[i]->getStreamInfo()->getStreamId();
            isHalMeta(streamId))
        {
            IMetadata* meta = rvResult[i]->tryReadLock(LOG_TAG);
            if (CC_UNLIKELY(meta == nullptr)) {
                MY_LOGW("cannot lock metadata");
                continue;
            }

            IMetadata::IEntry const entry = meta->entryFor(MTK_FRAMESYNC_RESULT);
            if (!entry.isEmpty()) {
                frameSyncState = entry.itemAt(0, Type2Type<MINT64>());
                bFind = true;
                rvResult[i]->unlock(LOG_TAG, meta);
                MY_LOGD1("(%#" PRIx64 ":%s): find frameSync meta [frameSyncState(%" PRId64 ")]",
                        streamId, rvResult[i]->getStreamInfo()->getStreamName(), frameSyncState);
                break;  // only check once
            } else {
                MY_LOGW("(%#" PRIx64 ":%s): cannot find frameSync meta [tag.count(%u), frameSyncState.isEmpty(%d)]",
                        streamId, rvResult[i]->getStreamInfo()->getStreamName(), meta->count(), entry.isEmpty());
            }
            rvResult[i]->unlock(LOG_TAG, meta);
        }
    }

    if (!bFind) {
        MY_LOGW("Can't Find MTK_FRAMESYNC_RESULT");
        return false;
    }

    // ===== check frameSync tag. =====
    if (frameSyncState == MTK_FRAMESYNC_RESULT_PASS) {
        MY_LOGD("(%#" PRIx64 "): framesync ok [frameSyncState(%" PRId64 ")]", streamId, frameSyncState);
        return true;
    }
    MY_LOGD("(%#" PRIx64 "): framesync failed [frameSyncState(%" PRId64 ")]", streamId, frameSyncState);
    return false;
}


bool
ZslProcessor::
isPDProcessRawBuffer(std::vector<android::sp<IMetaStreamBuffer>>const& rvResult)
{
    CAM_TRACE_CALL();
    std::vector< std::pair<MINT32, StreamId_T> > vRawType;  // either main1 or main2 is processedRaw/PD (?)  [results: OR]
    bool bFind = false;

    // ===== get processedRaw info. =====
    for (size_t i = 0; i < rvResult.size(); i++)
    {
        if (auto const streamId = rvResult[i]->getStreamInfo()->getStreamId();
            isHalMeta(streamId))
        {
            IMetadata* meta = rvResult[i]->tryReadLock(LOG_TAG);
            if (CC_UNLIKELY(meta == nullptr)) {
                MY_LOGW("cannot lock metadata");
                continue;
            }

            IMetadata::IEntry const entry = meta->entryFor(MTK_P1NODE_RAW_TYPE);
            if (!entry.isEmpty()) {
                vRawType.emplace_back(entry.itemAt(0, Type2Type<MINT32>()), streamId);
                bFind = true;
                MY_LOGD1("(%#" PRIx64 ":%s): find PD meta [p1node_raw_type(%d)]",
                        streamId, rvResult[i]->getStreamInfo()->getStreamName(), vRawType.back().first);
            } else {
                MY_LOGW("(%#" PRIx64 ":%s): cannot find PD meta [tag.count(%u), p1node_raw_type.isEmpty(%d)]",
                        streamId, rvResult[i]->getStreamInfo()->getStreamName(), meta->count(), entry.isEmpty());
            }
            rvResult[i]->unlock(LOG_TAG, meta);
        }
    }

    if (!bFind) {
        MY_LOGW("Can't Find MTK_P1NODE_RAW_TYPE");
        return false;
    }

    // ===== check processedRaw tag. =====
    for (auto const& iRawType : vRawType) {  // either one is PD, return true
        if (iRawType.first == NSIoPipe::NSCamIOPipe::EPipe_PROCESSED_RAW) {
            MY_LOGD("(%#" PRIx64 "): iRawType ok [iRawType(%d)]", iRawType.second, iRawType.first);
            return true;
        } else {
            MY_LOGW("(%#" PRIx64 "): iRawType skip [iRawType(%d)", iRawType.second, iRawType.first);
        }
    }

    MY_LOGD("=> not PDProcessedRaw [vRawTypeSz(%zu)]", vRawType.size());
    return false;
}


/******************************************************************************
 *  Utils.
 ******************************************************************************/
int64_t
ZslProcessor::
getTimeStamp(const MetaSet_T &rvResult)
{
    int64_t SensorTimestamp = 0;  // use either main1/main2's SensortimeStamp

    // ===== get sensorTimeStamp =====
    for (size_t i = 0; i < rvResult.size(); i++)
    {
        if (auto const streamId = rvResult[i]->getStreamInfo()->getStreamId();
            isAppMeta(streamId))
        {
            IMetadata* meta = rvResult[i]->tryReadLock(LOG_TAG);
            if (CC_UNLIKELY(meta == nullptr)) {
                MY_LOGW("cannot lock metadata");
                continue;
            }
            IMetadata::IEntry const entry = meta->entryFor(MTK_SENSOR_TIMESTAMP);
            if (!entry.isEmpty()) {
                SensorTimestamp = entry.itemAt(0, Type2Type<MINT64>());
                rvResult[i]->unlock(LOG_TAG, meta);
                MY_LOGD1("(%#" PRIx64 ":%s): find meta [SensorTimestamp(%" PRId64 ")]",
                        streamId, rvResult[i]->getStreamInfo()->getStreamName(), SensorTimestamp);
                break;  // get SensorTimestamp
            } else {
                MY_LOGW("(%#" PRIx64 ":%s): cannot find meta [tag.count(%u), SensorTimestamp.isEmpty(%d)]",
                        streamId, rvResult[i]->getStreamInfo()->getStreamName(), meta->count(), entry.isEmpty());
            }
            rvResult[i]->unlock(LOG_TAG, meta);
        }
    }
    return SensorTimestamp;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ZslProcessor::
getRootsStreamsForZoom(
    NodeSet const& roots,  // Set<NodeId_T>   /* NSPipelineContext:: */
    std::unordered_map<IPipelineFrame::NodeId_T, IOMapSet> const& table  /* IPipelineFrame:: , NSPipelineContext:: */
) -> std::unordered_set<StreamId_T>
{
    std::unordered_set<StreamId_T> out;
    for (auto const root : roots)
    {
        if (auto it = table.find(root); it != table.end()) {
            for (auto const& iomap : it->second /*IOMapSet*/) {
                for (auto const& streamId : iomap.vOut)
                    out.emplace(streamId);
            }
        }
    }
    return out;
}