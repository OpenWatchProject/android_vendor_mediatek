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

#define LOG_TAG "mtkcam-ppl_builder"
//
#include "RequestBuilderImpl.h"
#include "PipelineContextImpl.h"
#include <mtkcam/utils/std/ULog.h>
#include "MyUtils.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_CONTEXT);
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v3::pipeline;
using namespace NSCam::v3::pipeline::NSPipelineContext;
using namespace NSCam::Utils::ULog;

#define ThisNamespace   NSCam::v3::pipeline::NSPipelineContext::FrameBuilderImpl

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
static auto getDebugLogLevel() -> int32_t
{
    return ::property_get_int32("persist.vendor.debug.camera.RequestBuilder", 0);
}
static int32_t gDebugLevel = getDebugLogLevel();


/******************************************************************************
 *
 ******************************************************************************/
static void print(android::Printer& printer, FrameBuilderImpl const& o)
{
    printer.printFormatLine(".requestNo=%u", o.mRequestNo);

    {
        android::String8 os;
        os += ".root=";
        os += toString(o.mRootNodes);
        printer.printLine(os.c_str());
    }

    {
        std::string os;
        os += ".isReprocessFrame=";
        os += std::to_string(o.mbReprocessFrame);
        printer.printLine(os.c_str());
    }

    {
        std::string os;
        os += ".reprocessSensorTimestamp=";
        os += std::to_string(o.mReprocessSensorTimestamp);
        printer.printLine(os.c_str());
    }

    {
        auto const& x = o.mNodeEdges;
        android::String8 os;
        os += ".edges=";
        os += toString(x);
        printer.printLine(os.c_str());
    }

    {
        auto const& map = o.mImageNodeIOMaps;
        android::String8 os;
        os += "IOMap(image)";
        for (size_t i = 0; i < map.size(); i++) {
            auto const& nodeId  = map.keyAt(i);
            auto const& iomapSet= map.valueAt(i);
            os += android::String8::format("\n    <nodeId %#" PRIxPTR ">=", nodeId);
            os += toString(iomapSet);
        }
        printer.printLine(os.c_str());
    }

    {
        auto const& map = o.mMetaNodeIOMaps;
        android::String8 os;
        os += "IOMap(meta)";
        for (size_t i = 0; i < map.size(); i++) {
            auto const& nodeId  = map.keyAt(i);
            auto const& iomapSet= map.valueAt(i);
            os += android::String8::format("\n    <nodeId %#" PRIxPTR ">=", nodeId);
            os += toString(iomapSet);
        }
        printer.printLine(os.c_str());
    }

    {
        auto const& map = o.mReplacingInfos;
        android::String8 os;
        os += "[Overridden image stream info]";
        for (size_t i = 0; i < map.size(); i++) {
            auto const& v = map[i];
            os += "\n    ";
            os += v->toString();
        }
        printer.printLine(os.c_str());
    }

    {
        auto const& map = o.mStreamBuffers_AppImage;
        android::String8 os;
        os += "[App image stream buffers]";
        for (auto const& v : map) {
            auto const& streaminfo = v.second.first;
            auto const& streambuffer = v.second.second;
            os += "\n    ";
            os += streaminfo->toString();
            if ( streambuffer == nullptr ) {
                os += " (NO BUFFER)";
            }
        }
        printer.printLine(os.c_str());
    }

    {
        auto const& map = o.mStreamBuffers_HalImage;
        android::String8 os;
        os += "[Hal image stream buffers]";
        for (size_t i = 0; i < map.size(); i++) {
            auto const& v = map[i];
            os += "\n    ";
            os += v->toString();
        }
        printer.printLine(os.c_str());
    }

    {
        auto const& map = o.mStreamBuffers_AppMeta;
        android::String8 os;
        os += "[App meta stream buffers]";
        for (size_t i = 0; i < map.size(); i++) {
            auto const& v = map[i];
            os += "\n    ";
            os += v->toString();
        }
        printer.printLine(os.c_str());
    }

    {
        auto const& map = o.mStreamBuffers_HalMeta;
        android::String8 os;
        os += "[Hal meta stream buffers]";
        for (size_t i = 0; i < map.size(); i++) {
            auto const& v = map[i];
            os += "\n    ";
            os += v->toString();
        }
        printer.printLine(os.c_str());
    }

    if  (auto p = o.mpCallback.unsafe_get()) {
        android::String8 os;
        os += android::String8::format(".pCallback=%p", p);
        printer.printLine(os.c_str());
    }

    {
        auto const& x = o.mvPhysicalCameraSetting;
        std::string os;
        os += ".PhysicalCameraSetting=";
        os += "{ ";
        for (auto const& v : x) {
            os += std::to_string(v) + " ";
        }
        os += "}";
        printer.printLine(os.c_str());
    }

    if  (auto p = o.mTrackFrameResultParams.get()) {
        printer.printFormatLine(".TrackFrameResultParams=%s", toString(*p).c_str());
    }

    if ( o.mEnabledFlags.test(IFrameBuilder::ENABLED_FLAG_UNEXPECTED_TO_ABORT) ) {
        printer.printLine(".UNEXPECTED_TO_ABORT=1");
    }

    {
        printer.printFormatLine(".GroupFrameType=%s", toString(o.mGroupFrameType).c_str());
    }
}


/******************************************************************************
 *
 ******************************************************************************/
static inline void dumpToLog(
    FrameBuilderImpl const* builder,
    DetailsType logPriority,
    const char* prefix = 0
)
{
    if (CC_LIKELY( builder != nullptr )) {
        ULogPrinter logPrinter(__ULOG_MODULE_ID, LOG_TAG, logPriority, prefix);
        logPrinter.printLine("FrameBuilder=");
        print(logPrinter, *builder);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
static inline void dumpToLogIfNeeded(
    FrameBuilderImpl const* builder
)
{
    if  ( CC_UNLIKELY(gDebugLevel >= 2) )
    {
        dumpToLog(builder, DetailsType::DETAILS_INFO);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
static inline void dumpToLogIfNeeded(
    std::vector<FrameBuilderImpl*>const& builders
)
{
    if  ( CC_UNLIKELY(gDebugLevel >= 2) )
    {
        for (size_t i = 0; i < builders.size(); i++) {
            auto const& builder = builders[i];
            dumpToLog(
                builder, DetailsType::DETAILS_INFO,
                (std::to_string(i) + "|" + std::to_string(builders.size())).c_str()
            );
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
static inline auto convertToFrameBuilderImpl(IFrameBuilder* builder) -> FrameBuilderImpl*
{
    return static_cast<FrameBuilderImpl*>(builder);
}


/******************************************************************************
 *
 ******************************************************************************/
auto
IFrameBuilder::
make() -> std::shared_ptr<IFrameBuilder>
{
    return std::make_shared<FrameBuilderImpl>();
}



/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
dumpState(
    android::Printer& printer
) -> void
{
    print(printer, *convertToFrameBuilderImpl(this));
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
onRequestConstructed()
-> void
{
    // clear one-shot data
    mStreamBuffers_AppImage.clear();
    mStreamBuffers_HalImage.clear();
    mStreamBuffers_AppMeta.clear();
    mStreamBuffers_HalMeta.clear();
    //
    mFlag = FLAG_NO_CHANGE;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
setRequestNo(uint32_t requestNo) -> void
{
    mRequestNo = requestNo;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
setReprocessFrame(MBOOL const bReprocessFrame) -> void
{
    mbReprocessFrame = bReprocessFrame;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
setReprocessSensorTimestamp(int64_t reprocessSensorTimestamp) -> void
{
    mReprocessSensorTimestamp = reprocessSensorTimestamp;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
setIOMap(
    NodeId_T const nodeId,
    IOMapSet const& imageIOMap,
    IOMapSet const& metaIOMap
) -> void
{
    if( ! getFlag(FLAG_IOMAP_CHANGED) )
    {
        mImageNodeIOMaps.clear();
        mMetaNodeIOMaps.clear();
        //
        setFlag(FLAG_IOMAP_CHANGED);
    }
    mImageNodeIOMaps.add(nodeId, imageIOMap);
    mMetaNodeIOMaps.add(nodeId, metaIOMap);
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
setRootNode(
    NodeSet const& roots
) -> void
{
    setFlag(FLAG_NODEEDGE_CHANGED);
    mRootNodes = roots;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
setNodeEdges(
    NodeEdgeSet const& edges
) -> void
{
    setFlag(FLAG_NODEEDGE_CHANGED);
    mNodeEdges = edges;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
replaceStreamInfo(
    StreamId_T const streamId,
    android::sp<IImageStreamInfo> pStreamInfo
) -> void
{
    setFlag(FLAG_REPLACE_STREAMINFO);
    mReplacingInfos.add(streamId, pStreamInfo);
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
setAppImageStreamBuffer(
    StreamId_T const streamId,
    android::sp<IImageStreamInfo> pStreamInfo,
    android::sp<IImageStreamBuffer> pStreamBuffer
) -> void
{
    mStreamBuffers_AppImage.try_emplace(streamId, pStreamInfo, pStreamBuffer);
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
setAppImageStreamBuffer(
    StreamId_T const streamId,
    android::sp<IImageStreamBuffer> pStreamBuffer
) -> void
{
    if ( pStreamBuffer != nullptr ) {
        auto pStreamInfo = const_cast<IImageStreamInfo*>(pStreamBuffer->getStreamInfo());
        mStreamBuffers_AppImage.try_emplace(streamId, pStreamInfo, pStreamBuffer);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
setHalImageStreamBuffer(
    StreamId_T const streamId,
    android::sp<IImageStreamBuffer> buffer
) -> void
{
    mStreamBuffers_HalImage.add(streamId, buffer);
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
getAppMetaStreamBuffers(
    std::vector<android::sp<IMetaStreamBuffer>>& out
) const -> void
{
    auto add = [&out](auto const& src){
        for (size_t i = 0; i < src.size(); i++) {
            out.push_back(src[i]);
        }
    };

    out.reserve( mStreamBuffers_AppMeta.size() );
    add(mStreamBuffers_AppMeta);

}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
getHalMetaStreamBuffers(
    std::vector<android::sp<IMetaStreamBuffer>>& out
) const -> void
{
    auto add = [&out](auto const& src){
        for (size_t i = 0; i < src.size(); i++) {
            out.push_back(src[i]);
        }
    };

    out.reserve( mStreamBuffers_HalMeta.size() );
    add(mStreamBuffers_HalMeta);
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
setAppMetaStreamBuffer(
    StreamId_T const streamId,
    android::sp<IMetaStreamBuffer> buffer
) -> void
{
    mStreamBuffers_AppMeta.add(streamId, buffer);
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
setHalMetaStreamBuffer(
    StreamId_T const streamId,
    android::sp<IMetaStreamBuffer> buffer
) -> void
{
    mStreamBuffers_HalMeta.add(streamId, buffer);
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
updateFrameCallback(
    wp<AppCallbackT> pCallback
) -> void
{
    setFlag(FLAG_CALLBACK_CHANGED);
    mpCallback = pCallback;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
setPhysicalCameraSetting(
    std::vector<int32_t> const& physicalCameraSetting
) -> void
{
    mvPhysicalCameraSetting = physicalCameraSetting;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
setPhysicalAppMetaStreamIds(
    std::unordered_map<uint32_t, std::vector<StreamId_T> > const*
                                                physicalAppMetaStreamIds
) -> void
{
    if(physicalAppMetaStreamIds != nullptr)
    {
        mvPhysicalAppStreamIds = *physicalAppMetaStreamIds;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
setTrackFrameResultParams(
    std::shared_ptr<TrackFrameResultParams const>const& arg
) -> void
{
    mTrackFrameResultParams = arg;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
IRequestBuilder::
make() -> std::shared_ptr<IRequestBuilder>
{
    return std::make_shared<RequestBuilderImpl>();
}


/******************************************************************************
 *
 ******************************************************************************/
auto
RequestBuilderImpl::
build(
    android::sp<IPipelineContext> pPipelineContext,
    std::vector<FrameBuilderT>const& builders
) -> std::vector<PipelineFrameT>
{
    std::vector<PipelineFrameT> out;

    auto pContext = PipelineContext::cast(pPipelineContext.get());
    if (CC_UNLIKELY( pContext == nullptr )) {
        MY_LOGE("Bad context %p", pPipelineContext.get());
        return out;
    }
    //
    auto pContextImpl = pContext->getImpl();
    if (CC_UNLIKELY( pContextImpl == nullptr )) {
        MY_LOGE("cannot get context");
        return out;
    }
    //
    std::vector<FrameBuilderImpl*> temp;
    temp.reserve(builders.size());
    for (auto& builder : builders) {
        temp.push_back(convertToFrameBuilderImpl(builder.get()));
    }
    //
    dumpToLogIfNeeded(temp);
    out = pContextImpl->buildFrame(temp);
    if (CC_UNLIKELY( out.empty() )) {
        MY_LOGE("Failed on build multi frames (%zu frames expected)", builders.size());
        for (auto const& pFrameBuilderImpl : temp) {
            dumpToLog(pFrameBuilderImpl, DetailsType::DETAILS_ERROR, __FUNCTION__);
        }
    }
    return out;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
RequestBuilderImpl::
build(
    android::sp<IPipelineContext> pPipelineContext,
    FrameBuilderT const& builder
) -> PipelineFrameT
{
    auto pContext = PipelineContext::cast(pPipelineContext.get());
    if (CC_UNLIKELY( pContext == nullptr )) {
        MY_LOGE("Bad context %p", pPipelineContext.get());
        return nullptr;
    }
    //
    auto pContextImpl = pContext->getImpl();
    if (CC_UNLIKELY( pContextImpl == nullptr )) {
        MY_LOGE("cannot get context");
        return nullptr;
    }
    //
    auto const& pFrameBuilderImpl = convertToFrameBuilderImpl(builder.get());
    dumpToLogIfNeeded(pFrameBuilderImpl);
    auto pFrame = pContextImpl->buildFrame(pFrameBuilderImpl);
    if (CC_UNLIKELY( pFrame == nullptr )) {
        MY_LOGE("Failed on build one frame");
        dumpToLog(pFrameBuilderImpl, DetailsType::DETAILS_ERROR, __FUNCTION__);
    }
    //
    return pFrame;
}

