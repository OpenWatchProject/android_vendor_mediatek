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

#define LOG_TAG "mtkcam-pipeline"
//
#include <iomanip>
#include <sstream>
//
#include "MyUtils.h"
#include "PipelineBufferSetFrameControlImp.h"
#include <mtkcam3/pipeline/hwnode/NodeId.h>
// [Bg service]
#include <mtkcam3/pipeline/prerelease/IPreReleaseRequest.h>
#include <mtkcam/utils/std/ULog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_CONTEXT);
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v3::Utils;
using namespace NSCam::v3::pipeline;
using namespace NSCam::v3::pipeline::NSPipelineBufferSetFrameControlImp;
// [Bg service]
using namespace NSCam::v3::pipeline::prerelease;
using namespace NSCam::Utils::ULog;

#define MAIN_CLASS_NAME PipelineBufferSetFrameControlImp

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
//
#define LOGLEVEL                    0
#define MY_LOG1(...)                MY_LOGD_IF(CC_UNLIKELY(1<=LOGLEVEL), __VA_ARGS__)
#define MY_LOG2(...)                MY_LOGD_IF(CC_UNLIKELY(2<=LOGLEVEL), __VA_ARGS__)


/******************************************************************************
 *
 ******************************************************************************/
/**
 * The log level
 *
 * == 0:
 *  Do nothing.
 *
 * >= 1:
 *  TBD
 *
 * >= 2:
 *  Dump the configuration and topology of every pipeline frame (on finishConfiguration()).
 *
 */
static int32_t gLogLevel = ::property_get_int32("persist.vendor.debug.camera.pipelineframe", 0);


/******************************************************************************
 *
 ******************************************************************************/
static long getDurationInUS(struct timespec const& t1, struct timespec const& t2)
{
    struct timespec diff;
    if (t2.tv_nsec-t1.tv_nsec < 0) {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec - 1;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec + 1000000000;
    } else {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec;
    }
    return (diff.tv_sec * 1000000.0 + diff.tv_nsec / 1000.0);
}


/******************************************************************************
 *
 ******************************************************************************/
static std::string getFrameLifetimeLog(struct timespec const& start, struct timespec const& end)
{
    std::string os;
    auto pLogTool = NSCam::Utils::LogTool::get();
    if ( CC_LIKELY(pLogTool) ) {
        os += "{";
        os += pLogTool->convertToFormattedLogTime(&start);
        if ( 0 != end.tv_sec || 0 != end.tv_nsec ) {
            os += " -> ";
            os += pLogTool->convertToFormattedLogTime(&end);
            os += " (";
            os += std::to_string(getDurationInUS(start, end));
            os += "us)";
        }
        os += "}";
    }
    return os;
}


/******************************************************************************
 *
 ******************************************************************************/
static std::string toString(const IPipelineFrame::ImageInfoIOMapSet& o)
{
    std::ostringstream oss;
    oss << "{ ";
    for (size_t i = 0; i < o.size(); i++) {
        auto const& iomap = o[i];
        oss << "( ";
        for (size_t j = 0; j < iomap.vIn.size(); j++) {
            auto const& streamId = iomap.vIn.keyAt(j);
            oss << "0x" << std::setbase(16) << streamId << " ";
        }
        oss << "-> ";
        for (size_t j = 0; j < iomap.vOut.size(); j++) {
            auto const& streamId = iomap.vOut.keyAt(j);
            oss << "0x" << std::setbase(16) << streamId << " ";
        }
        oss << ")";
    }
    oss << " }";
    return oss.str();
}


/******************************************************************************
 *
 ******************************************************************************/
static std::string toString(const IPipelineFrame::MetaInfoIOMapSet& o)
{
    std::ostringstream oss;
    oss << "{ ";
    for (size_t i = 0; i < o.size(); i++) {
        auto const& iomap = o[i];
        oss << "( ";
        for (size_t j = 0; j < iomap.vIn.size(); j++) {
            auto const& streamId = iomap.vIn.keyAt(j);
            oss << "0x" << std::setbase(16) << streamId << " ";
        }
        oss << "-> ";
        for (size_t j = 0; j < iomap.vOut.size(); j++) {
            auto const& streamId = iomap.vOut.keyAt(j);
            oss << "0x" << std::setbase(16) << streamId << " ";
        }
        oss << ")";
    }
    oss << " }";
    return oss.str();
}


/******************************************************************************
 *
 ******************************************************************************/
static std::string toString(const std::set<StreamId_T>& o)
{
    std::ostringstream oss;
    oss << "[ ";
    for (auto it = o.cbegin(); it != o.cend(); ++it) {
        oss << "0x" << std::setbase(16) << (*it) << " ";
    }
    oss << " ]";
    return oss.str();
}


/******************************************************************************
 *
 ******************************************************************************/
namespace {


struct ToTitleMsg
{
    IPipelineFrame const*   pFrame = nullptr;
    StreamId_T              streamId = -1;
    NodeId_T                nodeId = eNODEID_UNKNOWN;
};
static std::string toTitleMsg(ToTitleMsg const& arg)
{
    std::ostringstream oss;
    oss << "[";

    if ( auto pFrame = arg.pFrame ) {
        oss << " requestNo:" << pFrame->getRequestNo();
        oss << " frameNo:" << pFrame->getFrameNo();
    }

    if ( arg.streamId >= 0 ) {
        oss << " streamId:" << toHexString(arg.streamId);
    }

    if ( arg.nodeId != eNODEID_UNKNOWN ) {
        oss << " nodeId:" << toHexString(arg.nodeId);
    }

    oss << " ]";
    return oss.str();
}

static std::string toTitleMsg(IPipelineFrame const* pFrame, StreamId_T streamId = -1L)
{
    return toTitleMsg(ToTitleMsg{.pFrame = pFrame, .streamId = streamId,});
}


} //namespace


/******************************************************************************
 *
 ******************************************************************************/
struct ReturnResult
{
    std::weak_ptr<IPipelineFrameHalResultProcessor>
                            pHalResultProcessor;
    uint32_t                requestNo = -1;
    uint32_t                frameNo = -1;
    int64_t                 sensorTimestamp = 0;
    bool                    isFrameDestroyed = false;
    HalImageSetT*           pHalImageSet = nullptr;
    HalMetaSetT*            pHalMetaSet = nullptr;
    AppMetaSetT*            pAppMetaSet = nullptr;
};
static void handleReturnResult(ReturnResult const& arg)
{
    auto pResultProcessor = arg.pHalResultProcessor.lock();
    if (CC_UNLIKELY( pResultProcessor == nullptr )) {
        return;
    }

    auto convert = [](auto& psrc, auto& dst){
        if ( psrc != nullptr ) {
            auto& src = *psrc;
            for (auto const& b : src) {
                dst.push_back(b);
            }
            src.clear();
        }
    };


    std::list<android::sp<IImageStreamBuffer>> aHalImageBuffers;
    if (auto p = arg.pHalImageSet) {
        aHalImageBuffers.splice(aHalImageBuffers.end(), *p);
    }

    std::list<android::sp<IMetaStreamBuffer>> aHalMetaBuffers;
    convert(arg.pHalMetaSet, aHalMetaBuffers);

    std::list<android::sp<IMetaStreamBuffer>> aAppMetaBuffers;
    convert(arg.pAppMetaSet, aAppMetaBuffers);


    pResultProcessor->returnResult(
    IPipelineFrameHalResultProcessor::ReturnResult{
        .requestNo = arg.requestNo,
        .frameNo = arg.frameNo,
        .sensorTimestamp = arg.sensorTimestamp,
        .isFrameDestroyed = arg.isFrameDestroyed,
        .pHalImageStreamBuffers = (aHalImageBuffers.empty() ? nullptr : &aHalImageBuffers),
        .pHalMetaStreamBuffers  = (aHalMetaBuffers.empty()  ? nullptr : &aHalMetaBuffers),
        .pAppMetaStreamBuffers  = (aAppMetaBuffers.empty()  ? nullptr : &aAppMetaBuffers),
    });

}


/******************************************************************************
 *
 ******************************************************************************/
auto
IPipelineBufferSetFrameControl::
castFrom(IPipelineFrame* pPipelineFrame) -> IPipelineBufferSetFrameControl*
{
    if (CC_UNLIKELY( pPipelineFrame == nullptr )) {
        return nullptr;
    }

    if (CC_UNLIKELY( 0 != ::strcmp(pPipelineFrame->getMagicName(), IPipelineBufferSetFrameControl::magicName()) )) {
        return nullptr;
    }

    return reinterpret_cast<MAIN_CLASS_NAME*>(pPipelineFrame->getMagicInstance());
}


/******************************************************************************
 *
 ******************************************************************************/
auto
IPipelineBufferSetFrameControl::
create(CreationParams const& arg) -> IPipelineBufferSetFrameControl*
{
    return new MAIN_CLASS_NAME(arg);
}


/******************************************************************************
 *
 ******************************************************************************/
MAIN_CLASS_NAME::
MAIN_CLASS_NAME(CreationParams const& arg)
    : mFrameNo(arg.frameNo)
    , mRequestNo(arg.requestNo)
    , mbReprocessFrame(arg.isReprocessFrame)
    , mGroupFrameType{arg.groupFrameType}
    , mRWLock()
    , mpAppCallback(arg.pAppCallback)
    , mListeners()
    //
    , mStreamBufferProvider(arg.pStreamBufferProvider)
    , mHalResultProcessor(arg.pHalResultProcessor)
    , mpPipelineCallback(arg.pNodeCallback)
    , mpStreamInfoSet(0)
    , mpNodeMap(0)
    , mpPipelineNodeMap(0)
    , mpPipelineDAG(0)
    //
    , mItemMapLock()
    , mNodeStatusMap()
    , mpReleasedCollector(new ReleasedCollector)
    , mpItemMap_AppImage(new ItemMap_AppImageT(mpReleasedCollector.get()))
    , mpItemMap_AppMeta (new ItemMap_AppMetaT (mpReleasedCollector.get()))
    , mpItemMap_HalImage(new ItemMap_HalImageT(mpReleasedCollector.get()))
    , mpItemMap_HalMeta (new ItemMap_HalMetaT (mpReleasedCollector.get()))
    //
{
    NSCam::Utils::LogTool::get()->getCurrentLogTime(&mTimestampFrameCreated);
    ::memset(&mTimestampFrameDone, 0, sizeof(mTimestampFrameDone));
}


/******************************************************************************
 *
 ******************************************************************************/
void
MAIN_CLASS_NAME::
onLastStrongRef(const void* /*id*/)
{
    //  Force to release all pending buffers.
    {
        HalImageSetT aHalImageSet;
        HalMetaSetT  aHalMetaSet;
        {
            android::Mutex::Autolock _l(mInformationKeeperLock);

            aHalImageSet = mPendingRelease_HalImage.set;
            mPendingRelease_HalImage.set.clear();

            aHalMetaSet = mPendingRelease_HalMeta.set;
            mPendingRelease_HalMeta.set.clear();
        }

        if ( ! aHalImageSet.empty() || ! aHalMetaSet.empty() ) {
            MY_LOGD("%s Force to release all pending buffers. "
                    "HalImage:#%zu HalImage:#%zu",
                    toTitleMsg(this).c_str(), aHalImageSet.size(), aHalMetaSet.size());

            handleReturnResult(ReturnResult{
                .pHalResultProcessor = mHalResultProcessor,
                .requestNo = getRequestNo(),
                .frameNo = getFrameNo(),
                .sensorTimestamp = mASensorTimestamp.load(),
                .isFrameDestroyed = false,
                .pHalImageSet = &aHalImageSet,
                .pHalMetaSet = &aHalMetaSet,
                .pAppMetaSet = nullptr,
            });
        }
    }

    if  (CC_UNLIKELY(
            (0 != mpItemMap_AppImage->mNonReleasedNum)
        ||  (0 != mpItemMap_AppMeta->mNonReleasedNum)
        ||  (0 != mpItemMap_HalImage->mNonReleasedNum)
        ||  (0 != mpItemMap_HalMeta->mNonReleasedNum)
        ))
    {
        MY_LOGW(
            "%s buffers are not completely released: #(AppImage, AppMeta, HalImage, HalMeta)=(%zd %zd %zd %zd)",
            toTitleMsg(this).c_str(),
            mpItemMap_AppImage->mNonReleasedNum,
            mpItemMap_AppMeta->mNonReleasedNum,
            mpItemMap_HalImage->mNonReleasedNum,
            mpItemMap_HalMeta->mNonReleasedNum
        );

        auto printMap = [](android::Printer& printer, auto const& map){
            for (size_t i = 0; i < map.size(); i++) {
                auto const& pItem = map.itemAt(i);
                if (CC_LIKELY( pItem != nullptr && pItem->getUsersManager() )) {
                    pItem->getUsersManager()->dumpState(printer);
                }
            }
        };

        ULogPrinter logPrinter(__ULOG_MODULE_ID, LOG_TAG, DetailsType::DETAILS_WARNING, "[onLastStrongRef] ");
        printMap(logPrinter, *mpItemMap_AppMeta);
        printMap(logPrinter, *mpItemMap_HalMeta);
        printMap(logPrinter, *mpItemMap_AppImage);
        printMap(logPrinter, *mpItemMap_HalImage);
    }

    {
        sp<IAppCallback> pAppCallback = mpAppCallback.promote();
        if  (CC_UNLIKELY( pAppCallback == 0 )) {
            MY_LOGW("%s Cannot promote AppCallback", toTitleMsg(this).c_str());
        }
        else {
            MY_LOGD("%s frame end", toTitleMsg(this).c_str());
            IAppCallback::Result result =
            {
                .frameNo         = getFrameNo(),
                .sensorTimestamp = mASensorTimestamp.load(),
                .nAppOutMetaLeft = 0,
                .vAppOutMeta     = Vector< sp<IMetaStreamBuffer> >(),
                .nHalOutMetaLeft = 0,
                .vHalOutMeta     = Vector< sp<IMetaStreamBuffer> >(),
                .bFrameEnd       = true,
                .vPhysicalOutMeta = android::KeyedVector<int, android::sp<IMetaStreamBuffer>>(),
                .vPhysicalAppStreamIds = std::unordered_map<uint32_t, std::vector<StreamId_T> >(),
                .vAppOutMetaByNodeId = DefaultKeyedVector<NodeId_T, std::vector<sp<IMetaStreamBuffer> >>(),
                .vHalOutMetaByNodeId = DefaultKeyedVector<NodeId_T, std::vector<sp<IMetaStreamBuffer> >>(),
                .vRootNode       = NodeSet(),
                .nPhysicalID     = mActivePhysicalID,
            };
            pAppCallback->updateFrame(getRequestNo(), 0, result);
            pAppCallback->onPipelineFrameDestroy(IAppCallback::PipelineFrameDestroy{
                    .requestNo      = getRequestNo(),
                    .frameNo        = getFrameNo(),
                    .groupFrameType = getGroupFrameType(),
                });
            CAM_ULOG_EXIT(MOD_DEFAULT_PIPELINE_MODEL, REQ_APP_REQUEST, getRequestNo());
        }
    }

    for (auto const& listener : mListeners)
    {
        sp<MyListener::IListener> p = listener.mpListener.promote();
        if  (CC_UNLIKELY( p == 0 ))
        {
            continue;
        }
        MY_LOG2("%s destroyed, remove it from listners", toTitleMsg(this).c_str());
        p->onPipelineFrame(
            getFrameNo(),
            IPipelineFrameListener::eMSG_FRAME_RELEASED,
            listener.mpCookie
        );
    }

    handleReturnResult(ReturnResult{
        .pHalResultProcessor = mHalResultProcessor,
        .requestNo = getRequestNo(),
        .frameNo = getFrameNo(),
        .sensorTimestamp = mASensorTimestamp.load(),
        .isFrameDestroyed = true,
    });

}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
MAIN_CLASS_NAME::
attachListener(
    wp<IPipelineFrameListener>const& pListener,
    MVOID* pCookie
)
{
    RWLock::AutoWLock _l(mRWLock);
    //
    mListeners.push_back(MyListener(pListener, pCookie));
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
sp<IPipelineNodeMap const>
MAIN_CLASS_NAME::
getPipelineNodeMap() const
{
    RWLock::AutoRLock _l(mRWLock);
    //
    sp<IPipelineNodeMap const> p = mpPipelineNodeMap.promote();
    //
    MY_LOGE_IF(
        p==0,
        "%s Bad PipelineNodeMap: wp:%p promote:%p - %s",
        toTitleMsg(this).c_str(), mpPipelineNodeMap.unsafe_get(), p.get(),
        getFrameLifetimeLog(mTimestampFrameCreated, mTimestampFrameDone).c_str()
    );
    //
    return p;
}


/******************************************************************************
 *
 ******************************************************************************/
IPipelineDAG const&
MAIN_CLASS_NAME::
getPipelineDAG() const
{
    RWLock::AutoRLock _l(mRWLock);
    //
    MY_LOGE_IF(
        mpPipelineDAG==0,
        "%s NULL PipelineDAG - %s",
        toTitleMsg(this).c_str(),
        getFrameLifetimeLog(mTimestampFrameCreated, mTimestampFrameDone).c_str()
    );
    return *mpPipelineDAG;
}


/******************************************************************************
 *
 ******************************************************************************/
android::sp<IPipelineDAG>
MAIN_CLASS_NAME::
getPipelineDAGSp()
{
    RWLock::AutoRLock _l(mRWLock);
    //
    MY_LOGE_IF(
        mpPipelineDAG==0,
        "%s NULL PipelineDAG - %s",
        toTitleMsg(this).c_str(),
        getFrameLifetimeLog(mTimestampFrameCreated, mTimestampFrameDone).c_str()
    );
    return mpPipelineDAG;
}


/******************************************************************************
 *
 ******************************************************************************/
IStreamInfoSet const&
MAIN_CLASS_NAME::
getStreamInfoSet() const
{
    RWLock::AutoRLock _l(mRWLock);
    //
    MY_LOGE_IF(
        mpStreamInfoSet==0,
        "%s NULL StreamInfoSet - %s",
        toTitleMsg(this).c_str(),
        getFrameLifetimeLog(mTimestampFrameCreated, mTimestampFrameDone).c_str()
    );
    return *mpStreamInfoSet;
}


/******************************************************************************
 *
 ******************************************************************************/
IStreamBufferSet&
MAIN_CLASS_NAME::
getStreamBufferSet() const
{
    RWLock::AutoRLock _l(mRWLock);
    return *const_cast<MAIN_CLASS_NAME*>(this);
}


/******************************************************************************
 *
 ******************************************************************************/
sp<IPipelineNodeCallback>
MAIN_CLASS_NAME::
getPipelineNodeCallback() const
{
    RWLock::AutoRLock _l(mRWLock);
    //
    sp<IPipelineNodeCallback> p = mpPipelineCallback.promote();
    //
    MY_LOGE_IF(
        p==0,
        "%s Bad PipelineNodeCallback: wp:%p promote:%p - %s",
        toTitleMsg(this).c_str(), mpPipelineCallback.unsafe_get(), p.get(),
        getFrameLifetimeLog(mTimestampFrameCreated, mTimestampFrameDone).c_str()
    );
    return p;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
MAIN_CLASS_NAME::
setNodeMap(
    android::sp<IPipelineFrameNodeMapControl> value
)
{
    if  (CC_UNLIKELY( value == 0)) {
        MY_LOGE("%s - NULL value", toTitleMsg(this).c_str());
        return BAD_VALUE;
    }
    //
    if  (CC_UNLIKELY( value->isEmpty() )) {
        MY_LOGE("%s - Empty value", toTitleMsg(this).c_str());
        return BAD_VALUE;
    }
    //
    RWLock::AutoWLock _l(mRWLock);
    mpNodeMap = value;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
MAIN_CLASS_NAME::
setPipelineNodeMap(
    android::sp<IPipelineNodeMap const> value
)
{
    if  (CC_UNLIKELY( value == 0)) {
        MY_LOGE("%s - NULL value", toTitleMsg(this).c_str());
        return BAD_VALUE;
    }
    //
    if  (CC_UNLIKELY( value->isEmpty() )) {
        MY_LOGE("%s - Empty value", toTitleMsg(this).c_str());
        return BAD_VALUE;
    }
    //
    RWLock::AutoWLock _l(mRWLock);
    mpPipelineNodeMap = value;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
MAIN_CLASS_NAME::
setPipelineDAG(android::sp<IPipelineDAG> value)
{
    if  (CC_UNLIKELY( value == 0)) {
        MY_LOGE("%s - NULL value", toTitleMsg(this).c_str());
        return BAD_VALUE;
    }
    //
    RWLock::AutoWLock _l(mRWLock);
    mpPipelineDAG = value;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
MAIN_CLASS_NAME::
setStreamInfoSet(android::sp<IStreamInfoSet const> value)
{
    if  (CC_UNLIKELY( value == 0)) {
        MY_LOGE("%s - NULL value", toTitleMsg(this).c_str());
        return BAD_VALUE;
    }
    //
    RWLock::AutoWLock _l(mRWLock);
    mpStreamInfoSet = value;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
MAIN_CLASS_NAME::
configureInformationKeeping(
    ConfigureInformationKeepingParams const& arg
) -> void
{
    android::Mutex::Autolock _l(mInformationKeeperLock);

    mPendingRelease_HalImage.isPendingToRelease = arg.keepHalImage;
    if ( arg.keepHalImage && ! arg.targetToKeep_HalImage.empty() ) {
        mPendingRelease_HalImage.targetStreamId = arg.targetToKeep_HalImage;
    }

    mPendingRelease_HalMeta.isPendingToRelease = arg.keepHalMeta;
    if ( arg.keepHalMeta && ! arg.targetToKeep_HalMeta.empty() ) {
        mPendingRelease_HalMeta.targetStreamId = arg.targetToKeep_HalMeta;
    }

    mPendingRelease_AppMeta.isPendingToRelease = arg.keepAppMeta || arg.keepTimestamp;

    MY_LOGD_IF((arg.keepTimestamp || arg.keepHalImage || arg.keepHalMeta || arg.keepAppMeta),
        "%s pending release configuration: "
        "keepTimestamp:%d keepHalImage:%d%s keepHalMeta:%d%s keepAppMeta:%d",
        toTitleMsg(this).c_str(), arg.keepTimestamp,
        arg.keepHalImage,
        (arg.targetToKeep_HalImage.empty() ? "" : ::toString(arg.targetToKeep_HalImage).c_str()),
        arg.keepHalMeta,
        (arg.targetToKeep_HalMeta.empty() ? "" : ::toString(arg.targetToKeep_HalMeta).c_str()),
        arg.keepAppMeta);
}


/******************************************************************************
 *
 ******************************************************************************/
auto
MAIN_CLASS_NAME::
transferPendingReleaseBuffers(
    android::Vector<android::sp<IImageStreamBuffer>>& out
) -> void
{
    {
        android::Mutex::Autolock _l(mInformationKeeperLock);
        for (auto const& b : mPendingRelease_HalImage.set) {
            out.push_back(b);
        }
        mPendingRelease_HalImage.set.clear();
    }

    for (size_t i = 0; i < out.size(); i++) {
        auto const& pStreamBuf = out[i];
        if (CC_LIKELY( pStreamBuf != nullptr )) {
#if 1
            if (CC_UNLIKELY( pStreamBuf->hasStatus(STREAM_BUFFER_STATUS::ERROR) )) {
                MY_LOGW("%s error-stauts: %s", toTitleMsg(this).c_str(), pStreamBuf->toString().c_str());
            }
#endif
            //reset UsersManager before transfering ownership.
            pStreamBuf->reset();
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
MAIN_CLASS_NAME::
transferPendingReleaseBuffers(
    android::Vector<android::sp<IMetaStreamBuffer>>& out
) -> void
{
    {
        android::Mutex::Autolock _l(mInformationKeeperLock);
        for (auto const& b : mPendingRelease_HalMeta.set) {
            out.push_back(b);
        }
        mPendingRelease_HalMeta.set.clear();
    }

    for (size_t i = 0; i < out.size(); i++) {
        auto const& pStreamBuf = out[i];
        if (CC_LIKELY( pStreamBuf != nullptr )) {
#if 1
            if (CC_UNLIKELY( pStreamBuf->hasStatus(STREAM_BUFFER_STATUS::ERROR) )) {
                MY_LOGW("%s error-stauts: %s", toTitleMsg(this).c_str(), pStreamBuf->toString().c_str());
            }
#endif
            //reset UsersManager before transfering ownership.
            pStreamBuf->reset();
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
MAIN_CLASS_NAME::
setSensorTimestamp(int64_t timestamp, char const* callerName) -> int
{
    int err = 0;

    if (CC_UNLIKELY( timestamp == 0 )) {
        MY_LOGW("%s set timestamp to 0 (Caller=%s)", toTitleMsg(this).c_str(), callerName);
    }

    int64_t expected = 0;
    bool ret = mASensorTimestamp.compare_exchange_strong(expected/*in/out*/, timestamp, std::memory_order_relaxed);
    if (CC_UNLIKELY(
            // fail to set the desired timestamp
            ! ret
            // the desired timestamp != the current one (i.e. expected)
        &&  timestamp != expected
    ))
    {
        MY_LOGE("%s Not allowed to overwrite existed timestamp: %" PRIu64 " -> %" PRIu64 " (Caller=%s)",
            toTitleMsg(this).c_str(), expected, timestamp, callerName);
        err = -EEXIST;
    }

    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
MAIN_CLASS_NAME::
tryGetSensorTimestamp() const -> int64_t
{
    android::Mutex::Autolock _l(mInformationKeeperLock);

    if ( 0 != mSensorTimestamp ) {
        return mSensorTimestamp;
    }

    if ( 0 == mSensorTimestamp ) {
        // try to get from App Meta output stream buffers.
        auto const& set = mPendingRelease_AppMeta.set;
        for (auto const& pStreamBuffer : set) {
            if (CC_LIKELY( pStreamBuffer != nullptr )) {
                bool hit = false;

                auto pMetadata = pStreamBuffer->tryReadLock(LOG_TAG);
                if (CC_LIKELY( pMetadata != nullptr )) {
                    hit = IMetadata::getEntry(pMetadata, MTK_SENSOR_TIMESTAMP, mSensorTimestamp);
                    pStreamBuffer->unlock(LOG_TAG, pMetadata);
                }

                if (hit)
                    break;
            }
        }
    }

    MY_LOGW_IF(0==mSensorTimestamp,
        "%s Timestamp=0(not ready?) keepAppMeta:%d",
        toTitleMsg(this).c_str(), mPendingRelease_AppMeta.isPendingToRelease);

    return mSensorTimestamp;
}

/******************************************************************************
 *
 ******************************************************************************/
auto
MAIN_CLASS_NAME::
setActivePhysicalID(int32_t physicalID) -> int
{
    int err = 0;

    if(physicalID >= 0) {
        mActivePhysicalID = physicalID;
    }

    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
MAIN_CLASS_NAME::
setPhysicalAppMetaStreamIds(
    std::unordered_map<uint32_t, std::vector<StreamId_T> > const&
        physicalAppMetaStreamIds
)
{
    RWLock::AutoWLock _l(mRWLock);
    mvPhysicalAppStreamIds = physicalAppMetaStreamIds;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
MAIN_CLASS_NAME::
setTrackFrameResultParams(
    std::shared_ptr<TrackFrameResultParamsT const>const& arg
) -> void
{
    android::RWLock::AutoWLock _l(mRWLock);
    mTrackFrameResultParams = arg;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
MAIN_CLASS_NAME::
getTrackFrameResultParams(
) const -> std::shared_ptr<TrackFrameResultParamsT const>
{
    android::RWLock::AutoRLock _l(mRWLock);
    return mTrackFrameResultParams;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
MAIN_CLASS_NAME::
queryIOStreamInfoSet(
    NodeId_T const& nodeId,
    sp<IStreamInfoSet const>& rIn,
    sp<IStreamInfoSet const>& rOut
) const
{
    auto toTitleMsg = [this, nodeId]() { return ::toTitleMsg(ToTitleMsg{.pFrame = this, .nodeId = nodeId,}); };

    RWLock::AutoRLock _l(mRWLock);
    //
    if  (CC_UNLIKELY( mpNodeMap == 0 )) {
        MY_LOGE("%s NULL node map", toTitleMsg().c_str());
        rIn = 0;
        rOut = 0;
        return NO_INIT;
    }
    //
    sp<IPipelineFrameNodeMapControl::INode> pNode = mpNodeMap->getNodeFor(nodeId);
    if  (CC_UNLIKELY( pNode == 0 )) {
        MY_LOGE("%s not found", toTitleMsg().c_str());
        rIn = 0;
        rOut = 0;
        return NAME_NOT_FOUND;
    }
    //
    if(pNode.get())
    {
        rIn = pNode->getIStreams();
        rOut= pNode->getOStreams();
    }
    //
    if  (CC_UNLIKELY( rIn == 0 || rOut == 0 )) {
        MY_LOGE("%s IStreams:%p OStreams:%p", toTitleMsg().c_str(), rIn.get(), rOut.get());
        return NO_INIT;
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
MAIN_CLASS_NAME::
queryInfoIOMapSet(
    NodeId_T const& nodeId,
    InfoIOMapSet& rIOMapSet
) const
{
    auto toTitleMsg = [this, nodeId]() { return ::toTitleMsg(ToTitleMsg{.pFrame = this, .nodeId = nodeId,}); };

    RWLock::AutoRLock _l(mRWLock);
    //
    if  (CC_UNLIKELY( mpNodeMap == 0 )) {
        MY_LOGE("%s Bad node map", toTitleMsg().c_str());
        return NO_INIT;
    }
    //
    sp<IPipelineFrameNodeMapControl::INode> pNode = mpNodeMap->getNodeFor(nodeId);
    if  (CC_UNLIKELY( pNode == 0 )) {
        MY_LOGE("%s not found", toTitleMsg().c_str());
        return NAME_NOT_FOUND;
    }
    //
    rIOMapSet = pNode->getInfoIOMapSet();
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
MAIN_CLASS_NAME::
startConfiguration()
{
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
MAIN_CLASS_NAME::
finishConfigurationLocked()
{
    //
    if  (CC_UNLIKELY( mpNodeMap == 0 || mpNodeMap->isEmpty() )) {
        MY_LOGE("Empty NodeMap: %p", mpNodeMap.get());
        return NO_INIT;
    }
    //
    if  (CC_UNLIKELY( mpStreamInfoSet == 0 ))
    {
        MY_LOGE("StreamInfoSet:%p", mpStreamInfoSet.get());
        return NO_INIT;
    }
    //
    if  (CC_UNLIKELY( mpPipelineDAG == 0 || mpPipelineNodeMap == 0 ))
    {
        MY_LOGE("PipelineDAG:%p PipelineNodeMap:%p", mpPipelineDAG.get(), mpPipelineNodeMap.unsafe_get());
        return NO_INIT;
    }
    //
    mpReleasedCollector->finishConfiguration(
            *mpItemMap_AppImage,
            *mpItemMap_AppMeta,
            *mpItemMap_HalImage,
            *mpItemMap_HalMeta
            );
    //
    mNodeStatusMap.setCapacity(mpNodeMap->size());
    for (size_t i = 0; i < mpNodeMap->size(); i++)
    {
        sp<NodeStatus> pNodeStatus = new NodeStatus;
        //
        IPipelineFrameNodeMapControl::INode* pNode = mpNodeMap->getNodeAt(i);
        NodeId_T const nodeId = pNode->getNodeId();
        {
            sp<IStreamInfoSet const> pStreams = pNode->getIStreams();
            //I:Meta
            for (size_t j = 0; j < pStreams->getMetaInfoNum(); j++)
            {
                sp<IStreamInfo> pStreamInfo = pStreams->getMetaInfoAt(j);
                StreamId_T const streamId = pStreamInfo->getStreamId();
                //
                sp<NodeStatus::IO> pIO = new NodeStatus::IO;
                pNodeStatus->mISetMeta.push_back(pIO);
                pIO->mMapItem = getMetaMapItemLocked(streamId);
                MY_LOGF_IF(pIO->mMapItem==0, "No I meta item for streamId:%s", toHexString(streamId).c_str());
            }
            //I:Image
            for (size_t j = 0; j < pStreams->getImageInfoNum(); j++)
            {
                sp<IStreamInfo> pStreamInfo = pStreams->getImageInfoAt(j);
                StreamId_T const streamId = pStreamInfo->getStreamId();
                //
                sp<NodeStatus::IO> pIO = new NodeStatus::IO;
                pNodeStatus->mISetImage.push_back(pIO);
                pIO->mMapItem = getImageMapItemLocked(streamId);
                MY_LOGF_IF(pIO->mMapItem==0, "No I image item for streamId:%s", toHexString(streamId).c_str());
            }
        }
        {
            sp<IStreamInfoSet const> pStreams = pNode->getOStreams();
            //O:Meta
            for (size_t j = 0; j < pStreams->getMetaInfoNum(); j++)
            {
                sp<IStreamInfo> pStreamInfo = pStreams->getMetaInfoAt(j);
                StreamId_T const streamId = pStreamInfo->getStreamId();
                //
                sp<NodeStatus::IO> pIO = new NodeStatus::IO;
                pNodeStatus->mOSetMeta.push_back(pIO);
                pIO->mMapItem = getMetaMapItemLocked(streamId);
                MY_LOGF_IF(pIO->mMapItem==0, "No O meta item for streamId:%s", toHexString(streamId).c_str());
            }
            //O:Image
            for (size_t j = 0; j < pStreams->getImageInfoNum(); j++)
            {
                sp<IStreamInfo> pStreamInfo = pStreams->getImageInfoAt(j);
                StreamId_T const streamId = pStreamInfo->getStreamId();
                //
                sp<NodeStatus::IO> pIO = new NodeStatus::IO;
                pNodeStatus->mOSetImage.push_back(pIO);
                pIO->mMapItem = getImageMapItemLocked(streamId);
                MY_LOGF_IF(pIO->mMapItem==0, "No O image item for streamId:%s", toHexString(streamId).c_str());
            }
        }
        //
        if  (CC_LIKELY(
                ! pNodeStatus->mISetMeta.empty()
            ||  ! pNodeStatus->mOSetMeta.empty()
            ||  ! pNodeStatus->mISetImage.empty()
            ||  ! pNodeStatus->mOSetImage.empty()
            ))
        {
            mNodeStatusMap.add(nodeId, pNodeStatus);
            mNodeStatusMap.mInFlightNodeCount++;
            //
            MY_LOG1(
                "nodeId:%s Image:I/O#=%zu/%zu Meta:I/O#=%zu/%zu",
                toHexString(nodeId).c_str(),
                pNodeStatus->mISetImage.size(), pNodeStatus->mOSetImage.size(),
                pNodeStatus->mISetMeta.size(), pNodeStatus->mOSetMeta.size()
            );
        }
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
MAIN_CLASS_NAME::
finishConfiguration()
{
    MERROR err = OK;
    {
        RWLock::AutoWLock _lRWLock(mRWLock);
        Mutex::Autolock _lItemMapLock(mItemMapLock);
        err = finishConfigurationLocked();
    }
    //
    if  ( CC_UNLIKELY(gLogLevel >= 2) )
    {
        ULogPrinter logPrinter(__ULOG_MODULE_ID, LOG_TAG, DetailsType::DETAILS_INFO, "[finishConfiguration] ");
        dumpState(logPrinter, {});
    }
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
sp<IUsersManager>
MAIN_CLASS_NAME::
findSubjectUsersLocked(
    StreamId_T streamId
)   const
{
#define _IMPLEMENT_(_map_) \
    { \
        ssize_t const index = _map_->indexOfKey(streamId); \
        if  ( 0 <= index ) { \
            return _map_->usersManagerAt(index); \
        } \
    }

    _IMPLEMENT_(mpItemMap_AppImage);
    _IMPLEMENT_(mpItemMap_AppMeta);
    _IMPLEMENT_(mpItemMap_HalImage);
    _IMPLEMENT_(mpItemMap_HalMeta);

#undef  _IMPLEMENT_

    MY_LOGW("%s not found", toTitleMsg(this, streamId).c_str());
    return NULL;
}


/******************************************************************************
 *
 ******************************************************************************/
template <class ItemMapT>
sp<typename ItemMapT::IStreamBufferT>
MAIN_CLASS_NAME::
getBufferLockedImp(
    StreamId_T streamId,
    UserId_T userId,
    ItemMapT const& rMap
)   const
{
    if  ( 0 == rMap.mNonReleasedNum ) {
        MY_LOGW_IF(1, "%s mNonReleasedNum==0", toTitleMsg(this, streamId).c_str());
        return NULL;
    }
    //
    sp<typename ItemMapT::ItemT> pItem = rMap.getItemFor(streamId);
    if  ( pItem == 0 ) {
        MY_LOGW_IF(0, "%s cannot find from map", toTitleMsg(this, streamId).c_str());
        return NULL;
    }
    //
    if  ( ! pItem->mBitStatus.hasBit(eBUF_STATUS_ACQUIRE) ) {
        if  (CC_UNLIKELY( pItem->mBitStatus.hasBit(eBUF_STATUS_ACQUIRE_FAILED) )) {
            pItem->mUsersManager->markUserStatus(userId, IUsersManager::UserStatus::RELEASE);
            MY_LOGW("%s Failure in previous acquiring buffer", toTitleMsg(this, streamId).c_str());
            return NULL;
        }
        MY_LOGF_IF(pItem->mBuffer!=0, "%s Non-null buffer but non-acquired status:%#x", toTitleMsg(this, streamId).c_str(), pItem->mBitStatus.value);
        //
        if constexpr
            ( std::is_same<ItemMapT, ItemMap_HalImageT>::value
           || std::is_same<ItemMapT, ItemMap_AppImageT>::value )
        {
            int err = [&, this]() -> int
            {
                int err = OK;

                auto pStreamBufferProvider = mStreamBufferProvider.lock();
                if (CC_UNLIKELY( pStreamBufferProvider == nullptr )) {
                    MY_LOGE(
                        "%s Failed on mStreamBufferProvider.lock() - expired:%d",
                        toTitleMsg(this, streamId).c_str(), mStreamBufferProvider.expired()
                    );
                    return NO_INIT;
                }
                NSCam::Utils::CamProfile profile(__FUNCTION__, "requestStreamBuffer");
                err = pStreamBufferProvider->requestStreamBuffer(
                        pItem->mBuffer,
                        IPipelineFrameStreamBufferProvider::RequestStreamBuffer{
                            .requestNo = getRequestNo(),
                            .frameNo = getFrameNo(),
                            .streamInfo = pItem->mStreamInfo.get(),
                        });
                profile.print_overtime(10, "%s", toTitleMsg(this, streamId).c_str());
                if  ( OK == err && pItem->mBuffer != 0 ) {
                    pItem->mBuffer->setUsersManager(pItem->mUsersManager);
                    pItem->mBitStatus.markBit(eBUF_STATUS_ACQUIRE);
                }
                else {
                    pItem->mBitStatus.markBit(eBUF_STATUS_ACQUIRE_FAILED);
                    pItem->mUsersManager->markUserStatus(
                            userId,
                            IUsersManager::UserStatus::RELEASE
                            );
                }
                return err;
            }();

            if  ( OK != err || pItem->mBuffer == 0 ) {
                #if 0
                MY_LOGE(
                    "%s mBuffer:%p err:%d(%s)",
                    toTitleMsg(this, streamId).c_str(), pItem->mBuffer.get(), err, ::strerror(-err));
                #endif
                pItem->mBuffer = NULL;
                return NULL;
            }
        }
    }
    //
    if  (CC_UNLIKELY( pItem->mBuffer == 0 )) {
        MY_LOGW(
            "%s mBitStatus(%#x) pValue->mBuffer == 0",
            toTitleMsg(this, streamId).c_str(), pItem->mBitStatus.value);
        return NULL;
    }
    //
    return pItem->mBuffer;
}


/******************************************************************************
 *
 ******************************************************************************/
template <class ItemMapT>
sp<typename ItemMapT::IStreamBufferT>
MAIN_CLASS_NAME::
getBufferLocked(
    StreamId_T streamId,
    UserId_T userId,
    ItemMapT const& rMap
)   const
{
    auto toTitleMsg = [this, streamId, nodeId=userId]() {
        return ::toTitleMsg(ToTitleMsg{.pFrame = this, .streamId = streamId, .nodeId = nodeId,});
    };

    sp<typename ItemMapT::IStreamBufferT>
    pBuffer = getBufferLockedImp(streamId, userId, rMap);
    //
    if  ( pBuffer == 0 ) {
        MY_LOGW_IF(0, "%s NULL buffer", toTitleMsg().c_str());
        return NULL;
    }

    /**
     * The buffer is NOT available if all users have released this buffer
     * (so as to be marked as released).
     */
    if  (CC_UNLIKELY( OK == pBuffer->haveAllUsersReleased() )) {
        MY_LOGW_IF(1, "%s all users released this buffer", toTitleMsg().c_str());
        return NULL;
    }

    /**
     * For a specific stream buffer (associated with a stream Id), a user (with
     * a unique user Id) could successfully acquire the buffer from this buffer
     * set only if all users ahead of this user have pre-released or released
     * the buffer.
     */
    if  (CC_UNLIKELY( OK != pBuffer->haveAllUsersReleasedOrPreReleased(userId) )) {
        MY_LOGW_IF(1, "%s not all of prior users release or pre-release this buffer", toTitleMsg().c_str());
        return NULL;
    }

    return pBuffer;
}


/******************************************************************************
 *
 ******************************************************************************/
sp<IMetaStreamBuffer>
MAIN_CLASS_NAME::
getMetaBuffer(StreamId_T streamId, UserId_T userId) const
{
    sp<IMetaStreamBuffer> p;
    //
    Mutex::Autolock _lItemMapLock(mItemMapLock);
    //
    p = getBufferLocked(streamId, userId, *mpItemMap_HalMeta);
    if  ( p != 0 ) {
        return p;
    }
    //
    p = getBufferLocked(streamId, userId, *mpItemMap_AppMeta);
    if  ( p != 0 ) {
        return p;
    }
    //
    return NULL;
}


/******************************************************************************
 *
 ******************************************************************************/
sp<IImageStreamBuffer>
MAIN_CLASS_NAME::
getImageBuffer(StreamId_T streamId, UserId_T userId) const
{
    sp<IImageStreamBuffer> p;
    //
    Mutex::Autolock _lItemMapLock(mItemMapLock);
    //
    p = getBufferLocked(streamId, userId, *mpItemMap_HalImage);
    if  ( p != 0 ) {
        return p;
    }
    //
    p = getBufferLocked(streamId, userId, *mpItemMap_AppImage);
    if  ( p != 0 ) {
        return p;
    }
    //
    return NULL;
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT32
MAIN_CLASS_NAME::
markUserStatus(
    StreamId_T const streamId,
    UserId_T userId,
    MUINT32 eStatus
)
{
    android::Mutex::Autolock _l(mItemMapLock);
    //
    sp<IUsersManager> pSubjectUsers = findSubjectUsersLocked(streamId);
    if  (CC_UNLIKELY( pSubjectUsers == 0 )) {
        return NAME_NOT_FOUND;
    }
    //
    return pSubjectUsers->markUserStatus(userId, eStatus);
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
MAIN_CLASS_NAME::
setUserReleaseFence(
    StreamId_T const streamId,
    UserId_T userId,
    MINT releaseFence
)
{
    android::Mutex::Autolock _l(mItemMapLock);
    //
    sp<IUsersManager> pSubjectUsers = findSubjectUsersLocked(streamId);
    if  (CC_UNLIKELY( pSubjectUsers == 0 )) {
        return NAME_NOT_FOUND;
    }
    //
    return pSubjectUsers->setUserReleaseFence(userId, releaseFence);
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT
MAIN_CLASS_NAME::
queryGroupUsage(
    StreamId_T const streamId,
    UserId_T userId
)   const
{
    android::Mutex::Autolock _l(mItemMapLock);
    //
    sp<IUsersManager> pSubjectUsers = findSubjectUsersLocked(streamId);
    if  (CC_UNLIKELY( pSubjectUsers == 0 )) {
        return 0;
    }
    //
    return pSubjectUsers->queryGroupUsage(userId);
}


/******************************************************************************
 *
 ******************************************************************************/
MINT
MAIN_CLASS_NAME::
createAcquireFence(
    StreamId_T const streamId,
    UserId_T userId
)   const
{
    android::Mutex::Autolock _l(mItemMapLock);
    //
    sp<IUsersManager> pSubjectUsers = findSubjectUsersLocked(streamId);
    if  (CC_UNLIKELY( pSubjectUsers == 0 )) {
        return -1;
    }
    //
    return pSubjectUsers->createAcquireFence(userId);
}


/******************************************************************************
 *
 ******************************************************************************/
sp<IMyMap::IItem>
MAIN_CLASS_NAME::
getMapItemLocked(
    StreamId_T streamId,
    IMyMap const& rItemMap
)   const
{
    android::sp<IMyMap::IItem>const& pItem = rItemMap.itemFor(streamId);
    if  ( pItem == 0 ) {
        MY_LOGW_IF(0, "%s cannot find from map", toTitleMsg(this, streamId).c_str());
        return NULL;
    }
    return pItem;
}


/******************************************************************************
 *
 ******************************************************************************/
sp<IMyMap::IItem>
MAIN_CLASS_NAME::
getMetaMapItemLocked(StreamId_T streamId) const
{
    sp<IMyMap::IItem> p;
    //
    p = getMapItemLocked(streamId, *mpItemMap_HalMeta);
    if  ( p != 0 ) {
        return p;
    }
    //
    p = getMapItemLocked(streamId, *mpItemMap_AppMeta);
    if  ( p != 0 ) {
        return p;
    }
    //
    return NULL;
}


/******************************************************************************
 *
 ******************************************************************************/
sp<IMyMap::IItem>
MAIN_CLASS_NAME::
getImageMapItemLocked(StreamId_T streamId) const
{
    sp<IMyMap::IItem> p;
    //
    p = getMapItemLocked(streamId, *mpItemMap_HalImage);
    if  ( p != 0 ) {
        return p;
    }
    //
    p = getMapItemLocked(streamId, *mpItemMap_AppImage);
    if  ( p != 0 ) {
        return p;
    }
    //
    return NULL;
}


/******************************************************************************
 *
 ******************************************************************************/
struct MAIN_CLASS_NAME::NodeStatusUpdater
{
public:     ////                    Definitions.
    typedef NodeStatus::IOSet       IOSet;

public:     ////                    Data Members.
    MUINT32 const                   mFrameNo;
    MUINT32                         mBufferStatus;
    NodeSet                         mRootNodeSet;

public:
    NodeStatusUpdater(MUINT32 frameNo, NodeSet rootNode)
        : mFrameNo(frameNo),
         mRootNodeSet(rootNode)
    {
        mBufferStatus = 0;
    }

    MUINT32 getBufferStatus()
    {
        return mBufferStatus;
    }

    MBOOL
    run(
        NodeId_T const nodeId,
        NodeStatusMap& rNodeStatusMap,
        android::BitSet32& rNodeStatusUpdated
    )
    {
        auto toTitleMsg = [this, nodeId]() {
                std::string os;
                os += "[";
                os += " frameNo:" + std::to_string(mFrameNo);
                os += " nodeId:" + toHexString(nodeId);
                os += " ]";
                return os;
            };

        MBOOL isAnyUpdate = MFALSE;

        // buffer status check
        MBOOL bufStatusCheck = MFALSE;
        if  (CC_UNLIKELY( rNodeStatusMap.indexOfKey(eNODEID_JpegNode) >= 0 && nodeId == eNODEID_JpegNode )) {
            bufStatusCheck = MTRUE;
        }
        //
        ssize_t const index = rNodeStatusMap.indexOfKey(nodeId);
        if  (CC_UNLIKELY( index < 0 )) {
            MY_LOGE("%s not found", toTitleMsg().c_str());
            return MFALSE;
        }
        //
        sp<NodeStatus> pNodeStatus = rNodeStatusMap.valueAt(index);
        if  (CC_UNLIKELY( pNodeStatus == 0 )) {
            MY_LOGE("%s NULL buffer", toTitleMsg().c_str());
            return MFALSE;
        }
        //
        // O Image
        if  ( updateNodeStatus(nodeId, pNodeStatus->mOSetImage, bufStatusCheck) ) {
            isAnyUpdate = MTRUE;
            rNodeStatusUpdated.markBit(IPipelineFrameListener::eMSG_ALL_OUT_IMAGE_BUFFERS_RELEASED);
            MY_LOG2("%s O Image Buffers Released", toTitleMsg().c_str());
        }
        // I Image
        if  ( updateNodeStatus(nodeId, pNodeStatus->mISetImage, MFALSE) ) {
            isAnyUpdate = MTRUE;
            MY_LOG2("%s I Image Buffers Released", toTitleMsg().c_str());
        }
        // O Meta
        if  ( updateNodeStatus(nodeId, pNodeStatus->mOSetMeta, MFALSE) ) {
            isAnyUpdate = MTRUE;
            rNodeStatusUpdated.markBit(IPipelineFrameListener::eMSG_ALL_OUT_META_BUFFERS_RELEASED);
            MY_LOG2("%s O Meta Buffers Released", toTitleMsg().c_str());
        }
        // I Meta
        if  ( updateNodeStatus(nodeId, pNodeStatus->mISetMeta, MFALSE) ) {
            isAnyUpdate = MTRUE;
            MY_LOG2("%s I Meta Buffers Released", toTitleMsg().c_str());
        }

        //
        // Is it a new node with all buffers released?
        if  (
                isAnyUpdate
            &&  pNodeStatus->mOSetImage.empty()
            &&  pNodeStatus->mISetImage.empty()
            &&  pNodeStatus->mOSetMeta.empty()
            &&  pNodeStatus->mISetMeta.empty()
            )
        {
            rNodeStatusMap.mInFlightNodeCount--;
        }
        //
        return isAnyUpdate;
    }

protected:

    MBOOL
    updateNodeStatus(NodeId_T const nodeId, IOSet& rIOSet, MBOOL bufStatusCheck)
    {
        if  ( rIOSet.mNotified ) {
            return MFALSE;
        }
        //
        IOSet::iterator it = rIOSet.begin();
        for (; it != rIOSet.end();) {
            if((*it) == NULL)
            {
                MY_LOGW("iterator (*it) is NULL! rIOSet.size:%zu", rIOSet.size());
                ++it;
                continue;
            }
            //
            sp<IMyMap::IItem> pMapItem = (*it)->mMapItem;
            //
            if ( ! pMapItem->getUsersManager()->isActive() ) {
                /**
                 * Remove if it's not active.
                 * We can remove this item from this user since an inactive item
                 * must be associated with no users.
                 *
                 * It happens if this item has been reset before (since all users have released it).
                 */
                it = rIOSet.erase(it);
                continue;
            }
            //
            MY_LOG1("haveAllProducerUsersReleased(%d) haveAllProducerUsersReleasedOrPreReleased(%d)",
            (OK == pMapItem->getUsersManager()->haveAllProducerUsersReleased()),
            (OK == pMapItem->getUsersManager()->haveAllProducerUsersReleasedOrPreReleased()));
            if  ( (OK == pMapItem->getUsersManager()->haveAllProducerUsersReleased()) ||
                  (OK == pMapItem->getUsersManager()->haveAllProducerUsersReleasedOrPreReleased())) {
                // store app/hal by nodeid
                // this function needs called before handleProducersReleased.
                // because handleProducersReleased will set eBUF_STATUS_PRODUCERS_RELEASE.
                pMapItem->handleProducersReleasedByNodeId(nodeId);
                pMapItem->handleProducersReleased();
            }
            //
            //  Check to see if this user "nodeId" has released.
            MUINT32 const status = pMapItem->getUsersManager()->getUserStatus(nodeId);
            MY_LOG1("nodeId(%s) status(%d) release(%d) UserStatus::RELEASE(%d) pre_release(%d) UserStatus::PRE_RELEASE(%d)",
                toHexString(nodeId).c_str(),
            status,
            IUsersManager::UserStatus::RELEASE,
            ((status & IUsersManager::UserStatus::RELEASE)),
            IUsersManager::UserStatus::PRE_RELEASE,
            ((status & IUsersManager::UserStatus::PRE_RELEASE)));
            if  ( (status & IUsersManager::UserStatus::RELEASE) ||
                  (status & IUsersManager::UserStatus::PRE_RELEASE)) {
                //
                it = rIOSet.erase(it);   //remove if released
                //
                if  ( (OK == pMapItem->getUsersManager()->haveAllUsersReleased()) ||
                      (OK == pMapItem->getUsersManager()->haveAllUsersReleasedOrPreReleased())) {
                    if ( bufStatusCheck ){
                        if (pMapItem->hasBufferStatus(STREAM_BUFFER_STATUS::ERROR)){
                            MY_LOGI("pMapItem buffer status error, streamId:%s, bufStatusCheck=%d, mBufferStatus=%d ",
                            toHexString(pMapItem->getStreamInfo()->getStreamId()).c_str(), bufStatusCheck, mBufferStatus );
                            mBufferStatus = STREAM_BUFFER_STATUS::ERROR;
                        }
                        MY_LOG1("pMapItem will be released, streamId:%s, bufStatusCheck=%d, mBufferStatus=%d ",
                            toHexString(pMapItem->getStreamInfo()->getStreamId()).c_str(), bufStatusCheck, mBufferStatus );
                    }
                    pMapItem->handleAllUsersReleased();

                }
            }
            else
            {
                ++it;
                continue;
            }
        }
        //
        if  ( rIOSet.empty() ) {
            rIOSet.mNotified = MTRUE;
            return MTRUE;
        }
        //
        return MFALSE;
    }

}; // end struct MAIN_CLASS_NAME::NodeStatusUpdater


/******************************************************************************
 *
 ******************************************************************************/
template <class BufSetT, class PendingReleaseSetT>
static void
tryKeepPendingReleaseBuffers(
    BufSetT& rBufSetToTest,
    PendingReleaseSetT& rPendingReleaseSet,
    android::Mutex& rInformationKeeperLock
)
{
    if ( ! rPendingReleaseSet.isPendingToRelease )
        return;

    if ( rPendingReleaseSet.targetStreamId.empty() ) {
        // target streams: not specified => keep all buffers
        android::Mutex::Autolock _l(rInformationKeeperLock);
        for (auto const& b : rBufSetToTest) {
            rPendingReleaseSet.set.push_back(b);
        }
        rBufSetToTest.clear();
        return;
    }
    else {
        // target streams: specified => just keep target buffers
        auto it = rBufSetToTest.begin();
        for (; it != rBufSetToTest.end(); it++) {
            if  (CC_LIKELY( (*it) != 0 )) {
                auto found = rPendingReleaseSet.targetStreamId.find((*it)->getStreamInfo()->getStreamId());
                if ( found != rPendingReleaseSet.targetStreamId.end() ) {
                    android::Mutex::Autolock _l(rInformationKeeperLock);
                    rPendingReleaseSet.set.push_back(*it);
                    (*it) = nullptr;//rBufSetToTest.erase(it);
                    continue;
                }
            }
        }
        return;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
MAIN_CLASS_NAME::
handleReleasedBuffers(UserId_T userId, sp<IAppCallback> pAppCallback)
{
    auto toTitleMsg = [this, nodeId=userId]() { return ::toTitleMsg(ToTitleMsg{.pFrame = this, .nodeId = nodeId,}); };

    AppImageSetT aAppImageSet;
    HalImageSetT aHalImageSet;
    HalMetaSetT  aHalMetaSet;
    AppMetaSetT  aAppMetaSetO;
    AppMetaSetT  aHalMetaSetO; // note: use AppMetaSetT in purpose.
    ssize_t      aAppMetaNumO;
    ssize_t      aHalMetaNumO;
    AppMetaMapT  aAppMetaSetOByNodeId;
    AppMetaMapT  aHalMetaSetOByNodeId;
    NodeSet rootNodeSet;
    if(mpPipelineDAG != nullptr)
    {
        rootNodeSet = mpPipelineDAG->getRootNodeSet();
    }
    {
        android::Mutex::Autolock _l(mpReleasedCollector->mLock);
        //
        aAppImageSet.swap(mpReleasedCollector->mAppImageSet_AllUsersReleased);
        aHalImageSet.splice(aHalImageSet.end(), mpReleasedCollector->mHalImageSet_AllUsersReleased);
        mpReleasedCollector->mHalImageSet_AllUsersReleased.clear();
        aHalMetaSet.splice(aHalMetaSet.end(), mpReleasedCollector->mHalMetaSet_AllUsersReleased);
        mpReleasedCollector->mHalMetaSet_AllUsersReleased.clear();
        //
        aAppMetaSetO = mpReleasedCollector->mAppMetaSetO_ProducersReleased;
        mpReleasedCollector->mAppMetaSetO_ProducersReleased.clear();
        aAppMetaNumO = mpReleasedCollector->mAppMetaNumO_ProducersInFlight;
        //
        aHalMetaSetO = mpReleasedCollector->mHalMetaSetO_ProducersReleased;
        mpReleasedCollector->mHalMetaSetO_ProducersReleased.clear();
        aHalMetaNumO = mpReleasedCollector->mHalMetaNumO_ProducersInFlight;
        //
        aAppMetaSetOByNodeId = mpReleasedCollector->mAppMetaSetOByNodeId_ProducersReleased;
        mpReleasedCollector->mAppMetaSetOByNodeId_ProducersReleased.clear();
        aHalMetaSetOByNodeId = mpReleasedCollector->mHalMetaSetOByNodeId_ProducersReleased;
        mpReleasedCollector->mHalMetaSetOByNodeId_ProducersReleased.clear();
    }
    //
    //  Callback to App.
    {
        if  (CC_UNLIKELY( pAppCallback == 0 )) {
            MY_LOGW("%s Cannot promote AppCallback", toTitleMsg().c_str());
        }
        else {
            if ( ! aAppImageSet.empty() ) {
                pAppCallback->onImageBufferReleased(IAppCallback::ImageBufferReleased{
                        .buffers = std::move(aAppImageSet),
                        .frame = this,
                    });
            }
            MY_LOG1("%s OAppMeta#(left:%zd this:%zu)", toTitleMsg().c_str(), aAppMetaNumO, aAppMetaSetO.size());
            IAppCallback::Result result =
            {
                getFrameNo(),
                mASensorTimestamp.load(),
                aAppMetaNumO,
                aAppMetaSetO,
                aHalMetaNumO,
                aHalMetaSetO,
                false,
                android::KeyedVector<int, android::sp<IMetaStreamBuffer>>(),
                mvPhysicalAppStreamIds,
                aAppMetaSetOByNodeId,
                aHalMetaSetOByNodeId,
                rootNodeSet,
                mActivePhysicalID,
            };
            pAppCallback->updateFrame(getRequestNo(), userId, result);
        }

        tryKeepPendingReleaseBuffers(aAppMetaSetO, mPendingRelease_AppMeta, mInformationKeeperLock);

        //aAppMetaSetO.clear();
        aHalMetaSetO.clear();
    }
    //
    //  Release to Hal? Right now or pending?
    {
        tryKeepPendingReleaseBuffers(aHalImageSet, mPendingRelease_HalImage, mInformationKeeperLock);
    }
    {
        tryKeepPendingReleaseBuffers(aHalMetaSet, mPendingRelease_HalMeta, mInformationKeeperLock);
    }

    handleReturnResult(ReturnResult{
        .pHalResultProcessor = mHalResultProcessor,
        .requestNo = getRequestNo(),
        .frameNo = getFrameNo(),
        .sensorTimestamp = mASensorTimestamp.load(),
        .isFrameDestroyed = false,
        .pHalImageSet = &aHalImageSet,
        .pHalMetaSet = &aHalMetaSet,
        .pAppMetaSet = &aAppMetaSetO,
    });
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
MAIN_CLASS_NAME::
applyRelease(UserId_T userId)
{
    auto toTitleMsg = [this, nodeId=userId]() { return ::toTitleMsg(ToTitleMsg{.pFrame = this, .nodeId = nodeId,}); };

    NodeId_T const nodeId = userId;
    sp<IAppCallback> pAppCallback;
    List<MyListener> listeners;
    BitSet32 nodeStatusUpdated;
    NodeSet rootNodeSet;
    if(mpPipelineDAG != nullptr)
    {
        rootNodeSet = mpPipelineDAG->getRootNodeSet();
    }
    NodeStatusUpdater updater(getFrameNo(), rootNodeSet);
    //
    {
        android::Mutex::Autolock _l(mApplyReleaseLock);
        MY_LOG1("%s +", toTitleMsg().c_str());
        //
        {
            RWLock::AutoWLock _lRWLock(mRWLock);
            Mutex::Autolock _lMapLock(mItemMapLock);
            //
            //  Update
            MBOOL isAnyUpdate = updater.run(nodeId, mNodeStatusMap, nodeStatusUpdated);
            mNodeStatusMap.mBufferStatus |= updater.getBufferStatus();
            //
            // Is the entire frame released?
            if  ( isAnyUpdate && 0 == mNodeStatusMap.mInFlightNodeCount )
            {
                nodeStatusUpdated.markBit(IPipelineFrameListener::eMSG_FRAME_RELEASED);
                //
                NSCam::Utils::LogTool::get()->getCurrentLogTime(&mTimestampFrameDone);
                //
#if 1
//              mpPipelineNodeMap = 0;
//              mpPipelineDAG = 0;
                mpStreamInfoSet = 0;
#endif
                MY_LOG1("%s Done - %s",
                    toTitleMsg().c_str(),
                    getFrameLifetimeLog(mTimestampFrameCreated, mTimestampFrameDone).c_str()
                );
                // [Bg service] send to PreRleaseRequestMgr to notify this request is end to AP.            //check frame has jpegNode
                ssize_t const index = mNodeStatusMap.indexOfKey(eNODEID_JpegNode);
                if  (CC_UNLIKELY( index >= 0 )) {
                    IPreReleaseRequestMgr::getInstance()->notifyCaptureCompleted(this, mNodeStatusMap.mBufferStatus);
                }

            }
            //
            if  ( ! nodeStatusUpdated.isEmpty() ) {
                listeners = mListeners;
            }
            //
            pAppCallback = mpAppCallback.promote();
        }
        //
        //
        handleReleasedBuffers(userId, pAppCallback);
    }
    //
    //  Callback to listeners if needed.
    if  ( ! nodeStatusUpdated.isEmpty() )
    {
        NSCam::Utils::CamProfile profile(__FUNCTION__, "IPipelineBufferSetFrameControl");
        //
        List<MyListener>::iterator it = listeners.begin();
        for (; it != listeners.end(); ++it) {
            sp<MyListener::IListener> p = it->mpListener.promote();
            if  (CC_UNLIKELY( p == 0 )) {
                continue;
            }
            //
            if  ( nodeStatusUpdated.hasBit(IPipelineFrameListener::eMSG_ALL_OUT_META_BUFFERS_RELEASED) ) {
                MY_LOG2("%s O Meta Buffers Released", toTitleMsg().c_str());
                p->onPipelineFrame(
                    getFrameNo(),
                    nodeId,
                    IPipelineFrameListener::eMSG_ALL_OUT_META_BUFFERS_RELEASED,
                    it->mpCookie
                );
            }
            //
            if  ( nodeStatusUpdated.hasBit(IPipelineFrameListener::eMSG_ALL_OUT_IMAGE_BUFFERS_RELEASED) ) {
                MY_LOG2("%s O Image Buffers Released", toTitleMsg().c_str());
                p->onPipelineFrame(
                    getFrameNo(),
                    nodeId,
                    IPipelineFrameListener::eMSG_ALL_OUT_IMAGE_BUFFERS_RELEASED,
                    it->mpCookie
                );
            }
            //
            if  ( nodeStatusUpdated.hasBit(IPipelineFrameListener::eMSG_FRAME_RELEASED) ) {
                MY_LOG2("%s Frame Done", toTitleMsg().c_str());
                MY_LOG2("Remove this from listener while pipeline frame destroyed");
            }
        }
        //
        profile.print_overtime(3, "notify listeners (nodeStatusUpdated:%#x)", nodeStatusUpdated.value);
    }
    //
    MY_LOG1("%s -", toTitleMsg().c_str());
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
MAIN_CLASS_NAME::
applyPreRelease(UserId_T userId)
{
    auto toTitleMsg = [this, nodeId=userId]() { return ::toTitleMsg(ToTitleMsg{.pFrame = this, .nodeId = nodeId,}); };

    NodeId_T const nodeId = userId;
    sp<IAppCallback> pAppCallback;
    List<MyListener> listeners;
    BitSet32 nodeStatusUpdated;
    NodeSet rootNodeSet;
    if(mpPipelineDAG != nullptr)
    {
        rootNodeSet = mpPipelineDAG->getRootNodeSet();
    }
    NodeStatusUpdater updater(getFrameNo(), rootNodeSet);
    MY_LOGD("%s +", toTitleMsg().c_str());
    {
        RWLock::AutoWLock _lRWLock(mRWLock);
        Mutex::Autolock _lMapLock(mItemMapLock);
        //
        //  Update
        MBOOL isAnyUpdate = updater.run(nodeId, mNodeStatusMap, nodeStatusUpdated);
        MY_LOGD("isAnyUpdate:%d mInFlightNodeCount:%zu", isAnyUpdate, mNodeStatusMap.mInFlightNodeCount);

        if  ( isAnyUpdate && 0 == mNodeStatusMap.mInFlightNodeCount )
        {
            nodeStatusUpdated.markBit(IPipelineFrameListener::eMSG_FRAME_RELEASED);
            //
            NSCam::Utils::LogTool::get()->getCurrentLogTime(&mTimestampFrameDone);
            //
            MY_LOG1("%s Done - %s",
                toTitleMsg().c_str(),
                getFrameLifetimeLog(mTimestampFrameCreated, mTimestampFrameDone).c_str()
            );
        }

        //
        if  ( ! nodeStatusUpdated.isEmpty() ) {
            listeners = mListeners;
        }
        //
        pAppCallback = mpAppCallback.promote();
    }

    AppImageSetT aAppImageSet;
    HalImageSetT aHalImageSet;
    HalMetaSetT  aHalMetaSet;
    AppMetaSetT  aAppMetaSetO;
    AppMetaSetT  aHalMetaSetO; // note: use AppMetaSetT in purpose.
    ssize_t      aAppMetaNumO;
    ssize_t      aHalMetaNumO;
    AppMetaMapT  aAppMetaSetOByNodeId;
    AppMetaMapT  aHalMetaSetOByNodeId;
    {
        android::Mutex::Autolock _l(mpReleasedCollector->mLock);
        //
        aAppImageSet.swap(mpReleasedCollector->mAppImageSet_AllUsersReleased);
        aHalImageSet.splice(aHalImageSet.end(), mpReleasedCollector->mHalImageSet_AllUsersReleased);
        mpReleasedCollector->mHalImageSet_AllUsersReleased.clear();
        aHalMetaSet.splice(aHalMetaSet.end(), mpReleasedCollector->mHalMetaSet_AllUsersReleased);
        mpReleasedCollector->mHalMetaSet_AllUsersReleased.clear();
        //
        aAppMetaSetO = mpReleasedCollector->mAppMetaSetO_ProducersReleased;
        mpReleasedCollector->mAppMetaSetO_ProducersReleased.clear();
        aAppMetaNumO = mpReleasedCollector->mAppMetaNumO_ProducersInFlight;
        //
        aHalMetaSetO = mpReleasedCollector->mHalMetaSetO_ProducersReleased;
        mpReleasedCollector->mHalMetaSetO_ProducersReleased.clear();
        aHalMetaNumO = mpReleasedCollector->mHalMetaNumO_ProducersInFlight;
        //
        aAppMetaSetOByNodeId = mpReleasedCollector->mAppMetaSetOByNodeId_ProducersReleased;
        mpReleasedCollector->mAppMetaSetOByNodeId_ProducersReleased.clear();
        aHalMetaSetOByNodeId = mpReleasedCollector->mHalMetaSetOByNodeId_ProducersReleased;
        mpReleasedCollector->mHalMetaSetOByNodeId_ProducersReleased.clear();
    }
    //  Callback to App.
    {
        if  (CC_UNLIKELY( pAppCallback == 0 )) {
            MY_LOGW("%s Cannot promote AppCallback", toTitleMsg().c_str());
        }
        else {
            if ( ! aAppImageSet.empty() ) {
                pAppCallback->onImageBufferReleased(IAppCallback::ImageBufferReleased{
                        .buffers = std::move(aAppImageSet),
                        .frame = this,
                    });
            }
            MY_LOG1("%s OAppMeta#(left:%zd this:%zu)", toTitleMsg().c_str(), aAppMetaNumO, aAppMetaSetO.size());
            IAppCallback::Result result =
            {
                getFrameNo(),
                mASensorTimestamp.load(),
                aAppMetaNumO,
                aAppMetaSetO,
                aHalMetaNumO,
                aHalMetaSetO,
                false,
                android::KeyedVector<int, android::sp<IMetaStreamBuffer>>(),
                mvPhysicalAppStreamIds,
                aAppMetaSetOByNodeId,
                aHalMetaSetOByNodeId,
                rootNodeSet,
                mActivePhysicalID,
            };
            pAppCallback->updateFrame(getRequestNo(), userId, result);
        }
        aAppMetaSetO.clear();
        aHalMetaSetO.clear();
    }
    //  Callback to listeners if needed.
    if  ( ! nodeStatusUpdated.isEmpty() )
    {
        NSCam::Utils::CamProfile profile(__FUNCTION__, "IPipelineBufferSetFrameControl");
        //
        List<MyListener>::iterator it = listeners.begin();
        for (; it != listeners.end(); ++it) {
            sp<MyListener::IListener> p = it->mpListener.promote();
            if  (CC_UNLIKELY( p == 0 )) {
                continue;
            }
            //
            if  ( nodeStatusUpdated.hasBit(IPipelineFrameListener::eMSG_ALL_OUT_META_BUFFERS_RELEASED) ) {
                MY_LOG1("%s O Meta Buffers Released", toTitleMsg().c_str());
                p->onPipelineFrame(
                    getFrameNo(),
                    nodeId,
                    IPipelineFrameListener::eMSG_ALL_OUT_META_BUFFERS_RELEASED,
                    it->mpCookie
                );
            }
            //
            if  ( nodeStatusUpdated.hasBit(IPipelineFrameListener::eMSG_ALL_OUT_IMAGE_BUFFERS_RELEASED) ) {
                MY_LOG1("%s O Image Buffers Released", toTitleMsg().c_str());
                p->onPipelineFrame(
                    getFrameNo(),
                    nodeId,
                    IPipelineFrameListener::eMSG_ALL_OUT_IMAGE_BUFFERS_RELEASED,
                    it->mpCookie
                );
            }
            //
            if  ( nodeStatusUpdated.hasBit(IPipelineFrameListener::eMSG_FRAME_RELEASED) ) {
                MY_LOG1("%s Frame Done", toTitleMsg().c_str());
                p->onPipelineFrame(
                    getFrameNo(),
                    IPipelineFrameListener::eMSG_FRAME_RELEASED,
                    it->mpCookie
                );
            }
        }
        //
        profile.print_overtime(3, "notify listeners (nodeStatusUpdated:%#x)", nodeStatusUpdated.value);
    }
    //
    MY_LOGD("%s -", toTitleMsg().c_str());
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
MAIN_CLASS_NAME::
logDebugInfo(std::string&& str)
{
    DebugLog x;
    x.str = std::move(str);
    NSCam::Utils::LogTool::get()->getCurrentLogTime(&x.timestamp);

    RWLock::AutoWLock _l(mDebugLogListRWLock);
    mDebugLogList.push_back(std::move(x));
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
MAIN_CLASS_NAME::
dumpState(android::Printer& printer __unused, const std::vector<std::string>& options __unused) const
{
    auto pLogTool = NSCam::Utils::LogTool::get();
    {
        /**
         *  frame:2208(r2446) 08-03 05:49:16.659 -> 08-03 05:49:16.700 (33ms) reprocess
         */

        android::String8 os;

        os += android::String8::format("frame:%u(r%u) ", mFrameNo, mRequestNo);

        if ( pLogTool ) {

            os += pLogTool->convertToFormattedLogTime(&mTimestampFrameCreated).c_str();

            if (mRWLock.tryReadLock() == OK) {
                if ( 0 != mTimestampFrameDone.tv_sec || 0 != mTimestampFrameDone.tv_nsec ) {
                    os += " -> ";
                    os += pLogTool->convertToFormattedLogTime(&mTimestampFrameDone).c_str();
                    os += " (";
                    os += std::to_string(getDurationInUS(mTimestampFrameCreated, mTimestampFrameDone)).c_str();
                    os += "us)";
                }
                mRWLock.unlock();
            }
        }

        if ( 0 != mASensorTimestamp.load() )
        {
            os += android::String8::format(" SensorTimestamp:%" PRIu64 "", mASensorTimestamp.load());
        }

        {
            android::Mutex::Autolock _l(mInformationKeeperLock);
            if (0 != mSensorTimestamp) {
                os += android::String8::format(" shutter:%" PRIu64 "", mSensorTimestamp);
            }
        }

        if (mbReprocessFrame) {
            os += " reprocess";
        }

        os += toString(mGroupFrameType).c_str();

        if (mIsAborted.load()) {
            os += " aborted";
        }

        if (isUnexpectedToAbort()) {
            os += " unexpected-to-abort";
        }

        printer.printLine(os.c_str());
    }
    const std::string prefix{"    "};
    android::PrefixPrinter prefixPrinter(printer, prefix.c_str());
    {
        /**
         * .root={ 0x1 } .edges={ 0x1->0x4 0x1->0x5 0x4->0x13 0x5->0x6 }
         */
        std::string os;

        auto pPipelineDAG = mpPipelineDAG;
        android::Vector<IPipelineDAG::NodeObj_T> roots = pPipelineDAG->getRootNode();
        if ( roots.size() ) {
            os += ".root={ ";
            for (size_t i = 0; i < roots.size(); i++) {
                auto const& v = roots[i];
                os += toHexString(v.id) + " ";
            }
            os += "}";
        }
        android::Vector<IPipelineDAG::Edge> edges;
        if ( OK == pPipelineDAG->getEdges(edges) ) {
            os += " .edges={ ";
            for (size_t i = 0; i < edges.size(); i++) {
                auto const& v = edges[i];
                os += toHexString(v.src) + "->" + toHexString(v.dst) + " ";
            }
            os += "}";
        }

        prefixPrinter.printLine(os.c_str());
    }
    {
        /**
         * .iomap(image)
         *   <nodeId 0x1>={ ( -> 0x100000001 0x100000002 0x100000003 ) }
         *   <nodeId 0x4>={ ( 0x100000001 0x100000002 0x100000003 -> 0x2 0x100000009 0x10000000a ) }
         *   <nodeId 0x5>={ ( 0x100000002 0x100000003 -> 0x10000000b ) }
         *   <nodeId 0x6>={ ( 0x10000000b -> ) }
         *   <nodeId 0x13>={ ( 0x100000009 0x10000000a -> 0x1 ) }
         * .iomap(meta)
         *   <nodeId 0x1>={ ( 0x80000000 0x10000000d -> 0x10000000f 0x100000015 ) }
         *   <nodeId 0x4>={ ( 0x80000000 0x10000000f 0x100000015 -> 0x100000012 0x100000018 ) }
         *   <nodeId 0x5>={ ( 0x80000000 0x10000000f 0x100000015 -> 0x100000011 ) }
         *   <nodeId 0x6>={ ( 0x80000000 0x100000011 -> 0x100000019 ) }
         *   <nodeId 0x13>={ ( 0x80000000 0x100000012 -> 0x10000001a ) }
         */
        auto pNodeMap = mpNodeMap;
        std::vector<std::string> os1; os1.push_back(".iomap(image)");
        std::vector<std::string> os2; os2.push_back(".iomap(meta)");
        for (size_t i = 0; i < pNodeMap->size(); i++) {
            if ( auto pNode = pNodeMap->getNodeAt(i) ) {
                auto nodeId = pNode->getNodeId();
                auto const& infoIomapSet = pNode->getInfoIOMapSet();
                if ( infoIomapSet.mImageInfoIOMapSet.size() ) {
                    std::ostringstream oss;
                    oss << "  <nodeId 0x" << std::setbase(16) << nodeId << ">="
                        << ::toString(infoIomapSet.mImageInfoIOMapSet);
                    os1.push_back(oss.str());
                }
                if ( infoIomapSet.mMetaInfoIOMapSet.size() ) {
                    std::ostringstream oss;
                    oss << "  <nodeId 0x" << std::setbase(16) << nodeId << ">="
                        << ::toString(infoIomapSet.mMetaInfoIOMapSet);
                    os2.push_back(oss.str());
                }
            }
        }
        for (auto const& v : os1) { prefixPrinter.printLine(v.c_str()); }
        for (auto const& v : os2) { prefixPrinter.printLine(v.c_str()); }
    }
    {
        /**
         * { 09-04 13:30:19.190 (X) -> 0x1 }
         * { 09-04 13:30:19.192 0x1 -> 0x4 }
         * { 09-04 13:30:19.201 0x1 -> 0x5 }
         * { 09-04 13:30:19.252 0x5 -> 0x6 }
         * { 09-04 13:30:19.260 0x6 -> (X) }
         */
        RWLock::AutoRLock _l(mDebugLogListRWLock);
        for (auto&& v : mDebugLogList) {
            std::string os;
            os += "{ ";
            if ( pLogTool ) {
                os += pLogTool->convertToFormattedLogTime(&v.timestamp);
                os += " ";
            }
            os += v.str;
            os += " }";
            prefixPrinter.printLine(os.c_str());
        }
    }

    if (mItemMapLock.timedLock(100000000 /* 100ms */) == OK) {

        auto printMap = [](android::Printer& printer, auto const& map){
            for (size_t i = 0; i < map.size(); i++) {
                auto const& pItem = map.mMap.valueAt(i);
                if (CC_LIKELY( pItem != nullptr )) {
                    if ( pItem->mBuffer != nullptr ) {
                        pItem->mBuffer->dumpState(printer, {});
                    }
                    else if ( pItem->getUsersManager() != nullptr ) {
                        pItem->getUsersManager()->dumpState(printer);
                    }
                }
            }
        };

        printMap(prefixPrinter, *mpItemMap_AppMeta);
        printMap(prefixPrinter, *mpItemMap_HalMeta);
        printMap(prefixPrinter, *mpItemMap_AppImage);
        printMap(prefixPrinter, *mpItemMap_HalImage);

        mItemMapLock.unlock();
    }

    // Pending Release
    {
        auto printSet = [](android::Printer& printer, auto const& set){
            for (auto const& pStreamBuf : set) {
                if (CC_LIKELY( pStreamBuf != nullptr )) {
                    printer.printFormatLine("    %s", pStreamBuf->toString().c_str());
                }
            }
        };

        android::Mutex::Autolock _l(mInformationKeeperLock);
        if ( ! mPendingRelease_HalImage.set.empty()
          || ! mPendingRelease_HalMeta.set.empty() )
        {
            prefixPrinter.printLine("Pending Release");
            printSet(prefixPrinter, mPendingRelease_HalImage.set);
            printSet(prefixPrinter, mPendingRelease_HalMeta.set);
        }
    }
}

