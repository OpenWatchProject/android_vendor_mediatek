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

#define LOG_TAG "mtkcam-PipelineFrameBuilder"

#include <impl/PipelineFrameBuilder.h>
#include <mtkcam/utils/std/ULog.h>

//
#include "MyUtils.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_MODEL);

/******************************************************************************
 *
 ******************************************************************************/
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v3::pipeline::model;
using namespace NSCam::v3::pipeline::NSPipelineContext;
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


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace model {


/******************************************************************************
 *
 ******************************************************************************/
static auto getDebugLogLevel() -> int32_t
{
    return ::property_get_int32("persist.vendor.debug.camera.log", 0);
}
static int32_t gLogLevel = getDebugLogLevel();


/******************************************************************************
 *
 ******************************************************************************/
static void print(android::Printer& printer, BuildPipelineFrameInputParams const& o)
{
    printer.printFormatLine(".requestNo=%u", o.requestNo);

    if  (auto p = o.pAppImageStreamBuffers) {
        android::String8 os;
        os += "App image stream buffers=";
        os += toString(*p);
        printer.printLine(os.c_str());
    }
    if  (auto p = o.pAppMetaStreamBuffers) {
        android::String8 os;
        os += "App meta stream buffers=";
        for (auto const& v : *p) {
            os += "\n    ";
            os += v->toString();
        }
        printer.printLine(os.c_str());
    }
    if  (auto p = o.pHalImageStreamBuffers) {
        android::String8 os;
        os += "Hal image stream buffers=";
        for (auto const& v : *p) {
            os += "\n    ";
            os += v->toString();
        }
        printer.printLine(os.c_str());
    }
    if  (auto p = o.pHalMetaStreamBuffers) {
        android::String8 os;
        os += "Hal meta stream buffers=";
        for (auto const& v : *p) {
            os += "\n    ";
            os += v->toString();
        }
        printer.printLine(os.c_str());
    }
    if  (auto p = o.physicalMetaStreamIds) {
        if(p->size() > 0) {
            android::String8 os;
            os += "Physical metadata app stream id:";
            for (auto const& v : *p) {
                os += "\n    ";
                os += android::String8::format("sensor id: %d\n", v.first);
                for (auto const& s:v.second)
                    os += android::String8::format("        (stream id: %#" PRIx64 ")", s);
            }
            printer.printLine(os.c_str());
        }
    }

    if  (auto p = o.pvUpdatedImageStreamInfo) {
        android::String8 os;
        os += "Updated image stream info=";
        for (auto const& v : *p) {
            os += "\n    ";
            os += v.second->toString();
        }
        printer.printLine(os.c_str());
    }

    if  (auto p = o.pnodeSet) {
        android::String8 os;
        os += ".nodes={ ";
        for (auto const& v : *p) {
            os += android::String8::format("%#" PRIxPTR " ", v);
        }
        os += "}";
        printer.printLine(os.c_str());
    }

    if  (auto p = o.pRootNodes) {
        android::String8 os;
        os += ".root=";
        os += NSCam::v3::pipeline::NSPipelineContext::toString(*p);
        printer.printLine(os.c_str());
    }

    if  (auto p = o.pEdges) {
        android::String8 os;
        os += ".edges=";
        os += toString(*p);
        printer.printLine(os.c_str());
    }

    if  (auto p = o.pnodeIOMapImage) {
        android::String8 os;
        os += "IOMap(image)";
        for (auto const& v : *p) {
            os += android::String8::format("\n    <nodeId %#" PRIxPTR ">=", v.first);
            os += toString(v.second);
        }
        printer.printLine(os.c_str());
    }

    if  (auto p = o.pnodeIOMapMeta) {
        android::String8 os;
        os += "IOMap(meta)";
        for (auto const& v : *p) {
            os += android::String8::format("\n    <nodeId %#" PRIxPTR ">=", v.first);
            os += toString(v.second);
        }
        printer.printLine(os.c_str());
    }

    if  (auto p = o.pCallback.unsafe_get()) {
        android::String8 os;
        os += android::String8::format(".pCallback=%p", p);
        printer.printLine(os.c_str());
    }
}


/******************************************************************************
 *
 ******************************************************************************/
static void dumpToLog(
    BuildPipelineFrameInputParams const& o,
    DetailsType logPriority,
    NSPipelineContext::IPipelineContext* pPipelineContext = nullptr
)
{
    ULogPrinter logPrinter(__ULOG_MODULE_ID, LOG_TAG, logPriority, "");
    logPrinter.printLine("BuildPipelineFrameInputParams=");
    print(logPrinter, o);

    if ( pPipelineContext != nullptr ) {
        logPrinter.printLine(".pPipelineContext=");
        pPipelineContext->dumpState(logPrinter, {});
    }
}


/******************************************************************************
 *
 ******************************************************************************/
static inline void dumpToLogIfNeeded(
    BuildPipelineFrameInputParams const& o
)
{
    if  ( CC_UNLIKELY(gLogLevel >= 2) )
    {
        dumpToLog(o, DetailsType::DETAILS_INFO, nullptr);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
static inline void dumpToLogIfNeeded(
    NSPipelineContext::IPipelineContext* pPipelineContext
)
{
    if  ( CC_UNLIKELY(gLogLevel >= 3) )
    {
        if ( pPipelineContext != nullptr ) {
            ULogPrinter logPrinter(__ULOG_MODULE_ID, LOG_TAG, DetailsType::DETAILS_INFO, "");
            logPrinter.printLine("PipelineContext=");
            pPipelineContext->dumpState(logPrinter, {});
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto makeFrameBuilder(
    BuildPipelineFrameInputParams const& in __unused
) -> std::shared_ptr<IFrameBuilder>
{
    CAM_TRACE_NAME(__FUNCTION__);

    bool isAppReprocessFrame = [&in](){
            if  (auto&& pAppImageStreamInfo = in.pAppImageStreamInfo) {
                return
                        ( pAppImageStreamInfo->pAppImage_Input_Yuv.get() )
                    ||  ( pAppImageStreamInfo->pAppImage_Input_Priv.get() )
                    ||  ( pAppImageStreamInfo->pAppImage_Input_RAW16.get() )
                        ;
            }
            return false;
        }();
    bool isReprocessFrame = isAppReprocessFrame || in.bReprocessFrame;
    int64_t reprocessSensorTimestamp = in.reprocessSensorTimestamp;

    auto builder = IFrameBuilder::make();
    builder->setRequestNo(in.requestNo);
    builder->setReprocessFrame(isReprocessFrame);
    builder->setRootNode( *in.pRootNodes );
    builder->setNodeEdges( *in.pEdges );
    builder->setPhysicalAppMetaStreamIds( in.physicalMetaStreamIds );

    if  (auto&& p = in.pvUpdatedImageStreamInfo) {
        for ( auto&& s : *p ) {
            builder->replaceStreamInfo( s.first, s.second );
        }
    }

    // IOMap of Image/Meta
    for ( auto key : *(in.pnodeSet) ) {
        auto const& it_image = in.pnodeIOMapImage->find(key);
        auto const& it_meta  = in.pnodeIOMapMeta->find(key);
        builder->setIOMap(
                key,
                (it_image !=in.pnodeIOMapImage->end() ) ? it_image->second : IOMapSet::empty(),
                (it_meta !=in.pnodeIOMapMeta->end() )   ? it_meta->second  : IOMapSet::empty()
            );
    }

    // set StreamBuffers of Image/Meta
    // app images and check this image stream is physical or not.
    if  ( in.pAppImageStreamInfo && in.pAppImageStreamBuffers ) {
        auto&& pAppImageStreamInfo = in.pAppImageStreamInfo;
        auto&& pAppImageStreamBuffers = in.pAppImageStreamBuffers;

    auto setAppImage = [&builder](auto const& bufferMap, auto const& pStreamInfo){
            if ( pStreamInfo ) {
                auto streamId = pStreamInfo->getStreamId();
                auto found = bufferMap.find(streamId);
                auto pStreamBuffer = (found != bufferMap.end() ? found->second : nullptr);
                builder->setAppImageStreamBuffer(streamId, pStreamInfo, pStreamBuffer);
                MY_LOGD("setAppImageStreamBuffer: %#" PRIx64 " buffer:%p", streamId, pStreamBuffer.get());
            }
        };
#define SET_APP_IMAGE(_BufferMap_, _Arg_...) setAppImage(pAppImageStreamBuffers->_BufferMap_, _Arg_)

        // logical stream
        for ( auto&& v : pAppImageStreamInfo->vAppImage_Output_Proc ) {
            SET_APP_IMAGE( vOImageBuffers, v.second );
        }
        for ( auto&& v : pAppImageStreamInfo->vAppImage_Output_RAW16 ) {
            SET_APP_IMAGE( vOImageBuffers, v.second );
        }
        // physical stream
        for ( auto&& s : pAppImageStreamInfo->vAppImage_Output_Proc_Physical ) {
            for ( auto&& v : s.second ) {
                SET_APP_IMAGE( vOImageBuffers, v );
            }
        }
        for ( auto&& s : pAppImageStreamInfo->vAppImage_Output_RAW16_Physical ) {
            for ( auto&& v : s.second ) {
                SET_APP_IMAGE( vOImageBuffers, v );
            }
        }
        SET_APP_IMAGE( vIImageBuffers, pAppImageStreamInfo->pAppImage_Input_Yuv );
        SET_APP_IMAGE( vIImageBuffers, pAppImageStreamInfo->pAppImage_Input_RAW16 );
        SET_APP_IMAGE( vIImageBuffers, pAppImageStreamInfo->pAppImage_Input_Priv );
        SET_APP_IMAGE( vOImageBuffers, pAppImageStreamInfo->pAppImage_Output_Priv );
        SET_APP_IMAGE( vOImageBuffers, pAppImageStreamInfo->pAppImage_Jpeg );
        SET_APP_IMAGE( vOImageBuffers, pAppImageStreamInfo->pAppImage_Output_IspTuningData_Raw );
        SET_APP_IMAGE( vOImageBuffers, pAppImageStreamInfo->pAppImage_Output_IspTuningData_Yuv );

        // physical isp meta stream
        for ( auto&& s : pAppImageStreamInfo->vAppImage_Output_IspTuningData_Raw_Physical ) {
            SET_APP_IMAGE(vOImageBuffers, s.second);
        }
        for ( auto&& s : pAppImageStreamInfo->vAppImage_Output_IspTuningData_Yuv_Physical ) {
            SET_APP_IMAGE(vOImageBuffers, s.second);
        }

#undef  SET_APP_IMAGE
    }

    // app meta
    if  (auto&& pSBuffers = in.pAppMetaStreamBuffers) {
        for ( auto&& pSBuffer : *pSBuffers ) {
            auto streamId = pSBuffer->getStreamInfo()->getStreamId();
            builder->setAppMetaStreamBuffer(streamId, pSBuffer);

            if ( isReprocessFrame && 0 == in.reprocessSensorTimestamp ) {
                // For a reprocess frame, the caller does not specify the sensor timestamp,
                // so we try to find one from app meta buffers.
                int64_t timestamp = 0;
                if (auto pMetadata = pSBuffer->tryReadLock(LOG_TAG)) {
                    IMetadata::getEntry(pMetadata, MTK_SENSOR_TIMESTAMP, timestamp);
                    pSBuffer->unlock(LOG_TAG, pMetadata);
                }
                if ( 0 != timestamp ) {
                    MY_LOGI(
                        "[requestNo:%u] reprocess timestamp:%" PRIu64 " from streamId:%#" PRIx64 "",
                        in.requestNo, timestamp, streamId
                    );
                    if (CC_UNLIKELY( reprocessSensorTimestamp != 0 && reprocessSensorTimestamp != timestamp ))
                    {
                        MY_LOGE(
                            "[requestNo:%u] overwrite existed reprocess timestamp: "
                            "%" PRIu64 " -> %" PRIu64 "(from streamId:%#" PRIx64 ")",
                            in.requestNo, reprocessSensorTimestamp, timestamp, streamId
                        );
                    }
                    reprocessSensorTimestamp = timestamp;
                }
            }
        }
    }

    // hal image
    if  (auto&& pSBuffers = in.pHalImageStreamBuffers) {
        for ( auto&& pSBuffer : *pSBuffers ) {
            builder->setHalImageStreamBuffer(pSBuffer->getStreamInfo()->getStreamId(), pSBuffer);
        }
    }

    // hal meta
    if  (auto&& pSBuffers = in.pHalMetaStreamBuffers) {
        for ( auto&& pSBuffer : *pSBuffers ) {
            builder->setHalMetaStreamBuffer(pSBuffer->getStreamInfo()->getStreamId(), pSBuffer);
        }
    }

    // reprocess frame
    if ( isReprocessFrame ) {
        builder->setReprocessSensorTimestamp(reprocessSensorTimestamp);
        MY_LOGI_IF((0!=in.reprocessSensorTimestamp),
            "[requestNo:%u] reprocess timestamp:%" PRIu64 " from users",
            in.requestNo, in.reprocessSensorTimestamp);
        if (CC_UNLIKELY( 0 == reprocessSensorTimestamp )) {
            std::ostringstream oss;
            oss << "[requestNo:" << in.requestNo << "] "
                << "this reprocess frame has no reprocess timestamp given."
                    ;
            auto errMsg = oss.str();

            MY_LOGE("%s", errMsg.c_str());
            dumpToLog(in, DetailsType::DETAILS_ERROR);
            NSCam::Utils::triggerAeeWarning(LOG_TAG, NULL, errMsg.c_str());
        }
    }

    builder->updateFrameCallback(in.pCallback);

    dumpToLogIfNeeded(in);
    return builder;
}


/******************************************************************************
 *
 ******************************************************************************/
auto makePipelineFrame(
    android::sp<IPipelineFrame>& out,
    std::shared_ptr<NSPipelineContext::IFrameBuilder> const& in,
    android::sp<NSPipelineContext::IPipelineContext> pPipelineContext
) -> int
{
    CAM_TRACE_NAME(__FUNCTION__);

    auto requestBuilder = IRequestBuilder::make();
    if (CC_UNLIKELY( requestBuilder == nullptr )) {
        MY_LOGE("Failed on IRequestBuilder::make()");
        return -ENOMEM;
    }

    out = requestBuilder->build(pPipelineContext, in);
    if (CC_UNLIKELY( out == nullptr )) {
        MY_LOGE("Failed to make one IPipelineFrame");
        return -EINVAL;
    }

    dumpToLogIfNeeded(pPipelineContext.get());
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto makePipelineFrame(
    std::vector<android::sp<IPipelineFrame>>& out,
    std::vector<std::shared_ptr<NSPipelineContext::IFrameBuilder>>const& in,
    android::sp<NSPipelineContext::IPipelineContext> pPipelineContext
) -> int
{
    CAM_TRACE_NAME(__FUNCTION__);

    auto requestBuilder = IRequestBuilder::make();
    if (CC_UNLIKELY( requestBuilder == nullptr )) {
        MY_LOGE("Failed on IRequestBuilder::make()");
        return -ENOMEM;
    }

    out = requestBuilder->build(pPipelineContext, in);
    if (CC_UNLIKELY( out.empty() )) {
        MY_LOGE("Failed to make IPipelineFrame - %zu frames in total", in.size());
        return -EINVAL;
    }

    dumpToLogIfNeeded(pPipelineContext.get());
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto buildPipelineFrame(
    android::sp<IPipelineFrame>& out __unused,
    BuildPipelineFrameInputParams const& in __unused,
    android::sp<NSPipelineContext::IPipelineContext> pPipelineContext __unused
) -> int
{
    CAM_TRACE_NAME(__FUNCTION__);

    auto pPPLCtx = ( pPipelineContext == nullptr ) ? in.pPipelineContext/*deprecated*/ : pPipelineContext;

    auto frameBuilder = makeFrameBuilder(in);
    if (CC_UNLIKELY( frameBuilder == nullptr )) {
        MY_LOGE("[requestNo:%u] Failed on makeFrameBuilder", in.requestNo);
        dumpToLog(in, DetailsType::DETAILS_ERROR, pPPLCtx.get());
        return -EINVAL;
    }

    int err = makePipelineFrame(out, frameBuilder, pPPLCtx);
    if (CC_UNLIKELY( out == nullptr || err != OK )) {
        MY_LOGE("[requestNo:%u] Failed to build IPipelineFrame err:%d", in.requestNo, err);
        dumpToLog(in, DetailsType::DETAILS_ERROR, pPPLCtx.get());
        return -EINVAL;
    }

    return err;
}


};  //namespace model
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam

