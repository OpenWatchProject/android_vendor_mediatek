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

#define LOG_TAG "MtkCam/TPNode"
#define DEBUG_LOG_TAG LOG_TAG

#include "BaseNode.h"
#include "hwnode_utilities.h"
#include "TPNodePlugin.h"

#include <mtkcam/pipeline/hwnode/TPNode.h>
#include <MFCNodeImp.h>

#include <utils/RWLock.h>
#include <utils/Thread.h>

#include <sys/prctl.h>
#include <sys/resource.h>

#include <mtkcam/drv/iopipe/SImager/IImageTransform.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

#include <mtkcam/utils/std/common.h>
#include <mtkcam/utils/exif/DebugExifUtils.h>
#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>

#include <mtkcam/custom/ExifFactory.h>

#include <cutils/properties.h>

#include <mtkcam/feature/utils/ImageBufferUtils.h>
#include <cassert>
#include <thread>
#include <future>
#include <queue>
#include <cmath>


#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#define TP_THREAD_NAME_NODE         "Cam@TPNode"
#define TP_THREAD_NAME_COPY         "Cam@TPCopy"
#define TP_THREAD_POLICY            SCHED_OTHER
#define TP_THREAD_PRIORITY          ANDROID_PRIORITY_NORMAL
#define TP_DUMP_PATH                "/sdcard/camera_dump/"
#define TP_DEBUG                    (0)

#define GROUP_OFFSET                (32)
#define GROUP_NAMED_ID(grp, nmd)    ((static_cast<uint64_t>(grp) << GROUP_OFFSET) | nmd)

using namespace std;
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::Utils;
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSSImager;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class TPNodeImp
    : public BaseNode
    , public TPNode
{
public:
                            TPNodeImp();

    MERROR                  init(const InitParams& rParams);

    MERROR                  config(const ConfigParams& rParams);

    MERROR                  queue(sp<IPipelineFrame> pFrame);

    using                   BaseNode::flush;
    MERROR                  flush();

    MERROR                  uninit();

    virtual                 ~TPNodeImp();

protected:
    MERROR                  threadSetting();


private:
    typedef List<sp<IPipelineFrame>> FrameQueueT;

    mutable RWLock          mConfigRWLock;
    sp<IMetaStreamInfo>     mpInAppMeta;
    sp<IMetaStreamInfo>     mpInHalMeta;
    sp<IMetaStreamInfo>     mpOutAppMeta;
    sp<IMetaStreamInfo>     mpOutHalMeta;
    sp<IImageStreamInfo>    mpInFullYuv;
    sp<IImageStreamInfo>    mpInResizedYuv;
    sp<IImageStreamInfo>    mpInBinningYuv;
    sp<IImageStreamInfo>    mpOutYuvJpeg;
    sp<IImageStreamInfo>    mpOutYuvThumbnail;
    sp<IImageStreamInfo>    mpOutYuv00;
    sp<IImageStreamInfo>    mpOutYuv01;
    sp<IImageStreamInfo>    mpOutDepth;
    sp<IImageStreamInfo>    mpOutClean;

    mutable Mutex           mFrameQueueLock;
    mutable Condition       mFrameQueueCond;
    FrameQueueT             mFrameQueue;
    MBOOL                   mbFrameDrained;
    mutable Condition       mbFrameDrainedCond;
    MBOOL                   mbNodeExit;

    MINT32                  mRequestIndex;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Frame Structure.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    enum BufferState {
        STATE_ACQUIRED          = 1 << 0,
        STATE_ACQUIRED_ERROR    = 1 << 1,
        STATE_USED              = 1 << 2,
        STATE_RELEASED          = 1 << 3,
        STATE_RELEASED_ERROR    = 1 << 4,
    };

    enum GroupState {
        GROUP_ERROR             = 1 << 4,
        GROUP_BUFFER_ERROR,
        GROUP_PLUGIN_ERROR,
        GROUP_COPY_ERROR,
        GROUP_FLUSH_ERROR,
        GROUP_OK                = 1 << 5,
    };

    enum DataTags {
        INPUT                   = 1 << 2,
        OUTPUT                  = 1 << 3,
        TEMP                    = 1 << 4,
        APP                     = 1 << 5,
        HAL                     = 1 << 6,
        METADATA                = 1 << 7,
        BUFFER                  = 1 << 8,
        FULL_SIZE               = 1 << 9,
        RESIZED                 = 1 << 10,
        BINNING                 = 1 << 11,
        JPEG                    = 1 << 12,
        THUMBNAIL               = 1 << 13,
        DEPTH                   = 1 << 14,
        CLEAN                   = 1 << 15,
        YUV                     = 1 << 16,
        GROUP                   = 1 << GROUP_OFFSET,
    };

    enum NamedData {
        STRM_META_IN_APP        = INPUT   | APP    | METADATA,
        STRM_META_IN_HAL        = INPUT   | HAL    | METADATA,
        STRM_META_OUT_APP       = OUTPUT  | APP    | METADATA,
        STRM_META_OUT_HAL       = OUTPUT  | HAL    | METADATA,
        STRM_BUF_IN_FULL        = INPUT   | BUFFER | FULL_SIZE,
        STRM_BUF_IN_RSZ         = INPUT   | BUFFER | RESIZED,
        STRM_BUF_IN_BIN         = INPUT   | BUFFER | BINNING,
        STRM_BUF_OUT_JPEG       = OUTPUT  | BUFFER | JPEG,
        STRM_BUF_OUT_THN        = OUTPUT  | BUFFER | THUMBNAIL,
        STRM_BUF_OUT_YUV00      = OUTPUT  | BUFFER | YUV        | 0x00,
        STRM_BUF_OUT_YUV01      = OUTPUT  | BUFFER | YUV        | 0x01,
        STRM_BUF_OUT_DEPTH      = OUTPUT  | BUFFER | DEPTH,
        STRM_BUF_OUT_CLEAN      = OUTPUT  | BUFFER | CLEAN,
        TEMP_BUF_FULL00         = TEMP    | BUFFER | FULL_SIZE  | 0x00,
        TEMP_BUF_FULL01         = TEMP    | BUFFER | FULL_SIZE  | 0x01,
    };

    typedef MUINT64 NamedId_T;
    typedef MUINT32 GroupId_T;

    class FrameRequest;
    friend class FrameRequest;

    class FrameBuffer : public VirtualLightRefBase {
    public:

                                        FrameBuffer(
                                                FrameRequest *pRequest,
                                                StreamId_T const streamId,
                                                NamedId_T const namedId);

                                        FrameBuffer(
                                                MINT32 const format,
                                                MSize const size,
                                                NamedId_T const namedId);

        virtual                         ~FrameBuffer();

        IImageBuffer*                   native();

        inline NamedId_T                getNamedId() { return mNamedId; }
        inline MUINT32                  getTransform() { return muTransform; }

    private:

        enum BufferType {
            STREAM_BUFFER,
            NODE_BUFFER
        };

        inline NodeId_T                 getNodeId() { return mpOwner->getNodeId(); }
        inline char const*              getNodeName() { return mpOwner->getNodeName(); }

        Mutex                           mLock;
        FrameRequest*                   mpOwner;
        StreamId_T const                mStreamId;
        NamedId_T const                 mNamedId;
        MUINT32 const                   mType;
        MUINT32                         muState;
        MUINT32                         muTransform;
        sp<IImageStreamBuffer>          mpStreamBuffer;
        sp<IImageBufferHeap>            mpImageBufferHeap;
        sp<IImageBuffer>                mpImageBuffer;
        std::future<void>               mJob;
    };

    class FrameMetadata : public VirtualLightRefBase {
    public:
                                        FrameMetadata(
                                                FrameRequest *pRequest,
                                                StreamId_T const streamId,
                                                NamedId_T const namedId);

        virtual                         ~FrameMetadata();

        IMetadata*                      native();

    private:

        inline NodeId_T                 getNodeId() { return mpOwner->getNodeId(); }
        inline char const*              getNodeName() { return mpOwner->getNodeName(); }

        Mutex                           mLock;
        FrameRequest*                   mpOwner;
        StreamId_T const                mStreamId;
        MUINT32                         muState;
        NamedId_T const                 mNamedId;
        sp<IMetaStreamBuffer>           mpStreamBuffer;
        IMetadata*                      mpMetadata;
    };

    class FrameRequest : public VirtualLightRefBase {
    public:

                                        FrameRequest(
                                                TPNodeImp* pNodeImp,
                                                sp<IPipelineFrame> const &pFrame);

        MERROR                          makeBuffer(StreamId_T, NamedId_T);

        MERROR                          makeMetadata(StreamId_T, NamedId_T);

        sp<FrameMetadata>               getMetadata(NamedId_T namedId)
                                        {
                                            if (mvpMetadata.indexOfKey(namedId) < 0)
                                                return NULL;

                                            return mvpMetadata.editValueFor(namedId);
                                        }

        sp<FrameBuffer>                 getBuffer(NamedId_T namedId)
                                        {
                                            if (mvpBuffer.indexOfKey(namedId) < 0)
                                                return NULL;

                                            return mvpBuffer.editValueFor(namedId);
                                        }

        MERROR                          remove(NamedId_T const& namedId)
                                        {
                                            if (!contains(namedId))
                                                    return BAD_VALUE;

                                            if (namedId & METADATA)
                                                mvpMetadata.removeItem(namedId);
                                            else
                                                mvpBuffer.removeItem(namedId);

                                            return OK;
                                        }

        inline MBOOL                    contains(NamedId_T const& namedId) const
                                        {
                                            return ((namedId & METADATA && mvpMetadata.indexOfKey(namedId) >= 0)
                                                    || mvpBuffer.indexOfKey(namedId) >= 0);
                                        }

        inline MUINT32                  getFrameNo() const
                                        {
                                            return mpFrame->getFrameNo();
                                        }

        inline MUINT32                  getRequestNo() const
                                        {
                                            return mpFrame->getRequestNo();
                                        }

        MVOID                           releaseBuffers();

        virtual                         ~FrameRequest();

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // inherited via BaseNode
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    public:
        inline NodeId_T                 getNodeId() {
                                            return mpNodeImp->getNodeId();
                                        }

        inline char const*              getNodeName() {
                                            return mpNodeImp->getNodeName();
                                        }

        MERROR                          getMetaStreamBuffer(
                                                StreamId_T streamId,
                                                sp<IMetaStreamBuffer>& rpStreamBuffer)
                                        {
                                            return mpNodeImp->ensureMetaBufferAvailable_(
                                                mpFrame->getFrameNo(),
                                                streamId,
                                                mpFrame->getStreamBufferSet(),
                                                rpStreamBuffer);
                                        }

        MERROR                          getImageStreamBuffer(
                                                StreamId_T streamId,
                                                sp<IImageStreamBuffer>& rpStreamBuffer)
                                        {
                                            return mpNodeImp->ensureImageBufferAvailable_(
                                                mpFrame->getFrameNo(),
                                                streamId,
                                                mpFrame->getStreamBufferSet(),
                                                rpStreamBuffer);
                                        }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // inherited via PipelineNode
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    public:
        inline IStreamBufferSet&        getStreamBufferSet() {
                                            return mpFrame->getStreamBufferSet();
                                        }

    private:
        TPNodeImp*                                  mpNodeImp;
        sp<IPipelineFrame> const                    mpFrame;
        KeyedVector<MUINT64, sp<FrameBuffer>>       mvpBuffer;
        KeyedVector<MUINT64, sp<FrameMetadata>>     mvpMetadata;
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Multi-frame Collector
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    class GroupedFrame;
    class MultiFrameCollector {

    public:
                                        MultiFrameCollector(TPNodeImp* pNodeImp)
                                                            : mpNodeImp(pNodeImp)
                                                            , mpGrouping(new GroupedFrame())
                                        { }

        MVOID                           begin(sp<FrameRequest>);

        MVOID                           push(GroupId_T uGroup, NamedId_T namedId);

        MVOID                           push(GroupId_T uGroup, sp<FrameBuffer>);

        MVOID                           end();

        MVOID                           markError() {
                                            mpGrouping->markState(GROUP_BUFFER_ERROR);
                                        }

        MVOID                           enque();

        MVOID                           flush();

        MVOID                           onFinish(sp<GroupedFrame> pGrpFrame);

        sp<GroupedFrame>                getGrouping() {
                                            return mpGrouping;
                                        }

    private:
        mutable Mutex                   mGroupQueueLock;
        mutable Condition               mGroupQueueCond;
        List<sp<GroupedFrame>>          mvpGroupQueue;
        TPNodeImp*                      mpNodeImp;
        sp<FrameRequest>                mpScopedRequest;
        sp<GroupedFrame>                mpGrouping;
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Frame Grouping
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:

    class GroupedFrame : public VirtualLightRefBase {
    friend class MultiFrameCollector;
    public:

                                        GroupedFrame()
                                            : muRequestNo(0)
                                            , mpToken(0)
                                            , muState(0)
                                        {}

        virtual                         ~GroupedFrame() {
                                            if (!isCorrect())
                                                dumpLog();

                                            mvpGrpBuffer.clear();
                                            mvpGrpMetadata.clear();
                                            mvpRequests.clear();
                                        }

        sp<FrameBuffer>&                getGroupedBuffer(GroupId_T, NamedId_T);

        sp<FrameMetadata>&              getGroupedMetadata(GroupId_T, NamedId_T);

        MVOID                           removeByTags(MUINT64);

        MVOID                           removeByTagsExceptGroup(MUINT64, GroupId_T);

        MBOOL                           contains(GroupId_T, NamedId_T);

        inline MUINT32                  getRequestNo() {
                                            return muRequestNo;
                                        }

        size_t                          getRequestCount() {
                                            return mvpRequests.size();
                                        }

        sp<FrameRequest> const&         getRequestAt(size_t index) {
                                            return mvpRequests.editItemAt(index);
                                        }

        MVOID                           releaseRequestAt(size_t index) {
                                            mvpRequests.editItemAt(index).clear();
                                        }

        MVOID                           markState(MUINT32 s) {
                                            muState |= s;
                                        }

        MBOOL                           isCorrect() {
                                            return !(muState & GROUP_ERROR);
                                        }

        MVOID                           finished();

        static MVOID                    onCallback(MVOID*, MERROR);

        MVOID                           dumpLog();


    private:
        Mutex                           mLock;
        MUINT32                         muRequestNo;
        MVOID*                          mpToken;
        MUINT32                         muState;
        Vector<sp<FrameRequest>>        mvpRequests;

        KeyedVector<MUINT64, sp<FrameBuffer>>       mvpGrpBuffer;
        KeyedVector<MUINT64, sp<FrameMetadata>>     mvpGrpMetadata;
    };


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Plugin Entry
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:

    MERROR                  initPlugin(PluginId_T);

    TPNodePlugin*           getPlugin(PluginId_T);

    MVOID                   doPlugin(sp<GroupedFrame> pGrpFrame);

    MVOID                   removePlugins();

private:
    typedef std::future<void>                       PluginJobT;
    typedef std::map<PluginId_T, PluginJobT>        PluginJobMapT;
    typedef KeyedVector<PluginId_T, TPNodePlugin*>  PluginMapT;

    mutable Mutex           mPluginLock;
    PluginJobMapT           mPluginJobMap;
    PluginMapT              mPluginMap;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Thread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    class NodeProcessor : public Thread {
        public:
                                        NodeProcessor(TPNodeImp* pNodeImp)
                                            : mpNodeImp(pNodeImp)
                                        {};

                                        ~NodeProcessor()
                                        {}

            // good place to do one-time initializations
            status_t                    readyToRun()
                                        {
                                            return mpNodeImp->threadSetting();
                                        }

            bool                        threadLoop();

            // Ask this object's thread to exit. This function is asynchronous, when the
            // function returns the thread might still be running. Of course, this
            // function can be called from a different thread.
            void                        requestExit();

        private:
            TPNodeImp*                  mpNodeImp;
    };

    class CopyProcessor : public Thread {
        public:
                                        CopyProcessor(TPNodeImp* pNodeImp)
                                            : mpNodeImp(pNodeImp)
                                            , mbCopyExit(MFALSE)
                                            , mbCopyDrained(MTRUE)
                                        {};

                                        ~CopyProcessor()
                                        {}

            // good place to do one-time initializations
            status_t                    readyToRun()
                                        {
                                            return mpNodeImp->threadSetting();
                                        }

            bool                        threadLoop();

            // Ask this object's thread to exit. This function is asynchronous, when the
            // function returns the thread might still be running. Of course, this
            // function can be called from a different thread.
            void                        requestExit();

            MVOID                       waitUntilDrained() const;

            MERROR                      queueCopyFrame(sp<GroupedFrame> pGrpFrame);

        private:
            typedef std::queue<std::function<void(void)>> JobQueueT;

            TPNodeImp*                  mpNodeImp;
            MBOOL                       mbCopyExit;
            MBOOL                       mbCopyDrained;
            mutable Condition           mCopyDrainedCond;
            JobQueueT                   mCopyQueue;
            mutable Mutex               mCopyLock;
            mutable Condition           mCopyCond;


    };


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    sp<NodeProcessor>                   mpNodeProcessor;
    sp<CopyProcessor>                   mpCopyProcessor;
    MultiFrameCollector*                mpCollector;
    MRect                               mActiveArray;
    MINT32                              mLogLevel;
    // debug dump (0: no dump; 1: dump raw)
    MINT32                              mDebugDump;

    // waitUntilDrained() is used to wait untill
    // 1. The frame queue is empty
    // 2. Node thread is idle
    MVOID                               waitUntilDrained() const;

    MERROR                              getInfoIOMapSet(
                                                sp<IPipelineFrame> const &pFrame,
                                                IPipelineFrame::InfoIOMapSet &rIOMapSet) const;

    MBOOL                               isMetaStreamAtIOMap(
                                                IPipelineFrame::InfoIOMapSet const &ioMapSet,
                                                StreamId_T const streamId) const;

    MBOOL                               isImageStreamAtIOMap(
                                                IPipelineFrame::InfoIOMapSet const &ioMapSet,
                                                StreamId_T const streamId) const;

    MERROR                              onDequeFrame(sp<IPipelineFrame> &rpFrame);

    MVOID                               onProcessFrame(sp<IPipelineFrame> const &pFrame);

    MVOID                               onPartialFrameDone(const sp<IPipelineFrame>& frame);

    MERROR                              verifyConfigParams(ConfigParams const &rParams) const;

    MERROR                              mapToRequest(
                                                sp<IPipelineFrame> const &pFrame,
                                                sp<FrameRequest> &rpRequest);

    MERROR                              copyBuffer(sp<GroupedFrame> pGrpFrame);

    MBOOL                               makeDebugInfo(IMetadata* metadata);
};

/******************************************************************************
 *
 ******************************************************************************/
static MBOOL isSameRatio(
        MSize const &rSrc, MSize const &rDst)
{
    #define THRESHOLD (0.03)

    float ratioSrc = (float)rSrc.w/rSrc.h;
    float ratioDst = (float)rDst.w/rDst.h;
    float ratioDiff = std::abs(ratioDst - ratioSrc);

    MBOOL isSame = (ratioDiff < THRESHOLD);

    MY_LOGD("src ratio(%f), dst ratio(%f), diff(%f) thres(%f) isSame(%d)",
        ratioSrc, ratioDst, ratioDiff,
        THRESHOLD, isSame
    );

    #undef THRESHOLD
    return isSame;
}

static MRect calCrop(
        MRect const &rSrc, MRect const &rDst,
        MBOOL isCentered = MTRUE, uint32_t ratio = 100)
{
    #define ROUND_TO_2X(x) ((x) & (~0x1))

    MRect rCrop;

    // srcW/srcH < dstW/dstH
    if (rSrc.s.w * rDst.s.h < rDst.s.w * rSrc.s.h)
    {
        rCrop.s.w = rSrc.s.w;
        rCrop.s.h = rSrc.s.w * rDst.s.h / rDst.s.w;
    }
    // srcW/srcH > dstW/dstH
    else if (rSrc.s.w * rDst.s.h > rDst.s.w * rSrc.s.h)
    {
        rCrop.s.w = rSrc.s.h * rDst.s.w / rDst.s.h;
        rCrop.s.h = rSrc.s.h;
    }
    // srcW/srcH == dstW/dstH
    else
    {
        rCrop.s.w = rSrc.s.w;
        rCrop.s.h = rSrc.s.h;
    }

    rCrop.s.w =  ROUND_TO_2X(rCrop.s.w * 100 / ratio);
    rCrop.s.h =  ROUND_TO_2X(rCrop.s.h * 100 / ratio);

    if (isCentered)
    {
        rCrop.p.x = (rSrc.s.w - rCrop.s.w) / 2;
        rCrop.p.y = (rSrc.s.h - rCrop.s.h) / 2;
    }

    #undef ROUND_TO_2X
    return rCrop;
}

static auto getDebugExif()
{
    static auto const inst = MAKE_DebugExif();
    return inst;
}

static inline void dumpStreamIfExist(
        const char* str,
        const sp<IStreamInfo>& streamInfo)
{
    MY_LOGD_IF(streamInfo.get(), "%s: streamId(%#" PRIx64 ") %s",
            str, streamInfo->getStreamId(), streamInfo->getStreamName());
}


/******************************************************************************
 *
 ******************************************************************************/
TPNodeImp::FrameBuffer::
FrameBuffer(
    FrameRequest *pRequest,
    StreamId_T const streamId,
    NamedId_T const namedId)
    : mpOwner(pRequest)
    , mStreamId(streamId)
    , mNamedId(namedId)
    , mType(STREAM_BUFFER)
    , muState(0)
    , muTransform(0)
{

    if (OK != mpOwner->getImageStreamBuffer(
            mStreamId,
            mpStreamBuffer))
    {
        muState = STATE_ACQUIRED_ERROR;
        MY_LOGE("fail to get image, stream:%#" PRIx64, mStreamId);
    } else {
        muState = STATE_ACQUIRED;
        muTransform = mpStreamBuffer->getStreamInfo()->getTransform();
    }
    MY_LOGD_IF(TP_DEBUG, "create buffer, stream:%#" PRIx64 , mStreamId);
}

/******************************************************************************
 *
 ******************************************************************************/
TPNodeImp::FrameBuffer::
FrameBuffer(
    MINT32 const format,
    MSize const size,
    NamedId_T const namedId)
    : mpOwner(NULL)
    , mStreamId(0)
    , mNamedId(namedId)
    , mType(NODE_BUFFER)
    , muState(0)
    , muTransform(0)
{
    mJob = std::async([&, format, size]() {
        CAM_TRACE_NAME("TP::TempBuf::Creation");

        String8 bufferName = String8::format("tp:s%dx%d:f%d", size.w, size.h, format);

        MUINT32 plane = NSCam::Utils::Format::queryPlaneCount(format);
        size_t boundaryInBytes[3] = {0};
        size_t stridesInBytes[3] = {0};
        for (MUINT32 i = 0; i < plane; i++) {
            stridesInBytes[i] = NSCam::Utils::Format::queryPlaneWidthInPixels(format, i, size.w) *
                                   NSCam::Utils::Format::queryPlaneBitsPerPixel(format, i) / 8;
        }

        IImageBufferAllocator::ImgParam imgParam(
                        (EImageFormat) format,
                        size, stridesInBytes,
                        boundaryInBytes, plane);

        mpImageBufferHeap = IIonImageBufferHeap::create(bufferName.string(), imgParam);
        if (!mpImageBufferHeap.get()) {
            muState = STATE_ACQUIRED_ERROR;
            MY_LOGE("node buffer(%s): create heap failed", bufferName.string());
            return;
        }

        mpImageBuffer = mpImageBufferHeap->createImageBuffer();
        if (!mpImageBuffer.get()) {
            muState = STATE_ACQUIRED_ERROR;
            MY_LOGE("node buffer(%s): create image buffer failed", bufferName.string());
            return;
        }

        MUINT const usage = (GRALLOC_USAGE_SW_READ_OFTEN  | GRALLOC_USAGE_SW_WRITE_OFTEN |
                             GRALLOC_USAGE_HW_CAMERA_READ | GRALLOC_USAGE_HW_CAMERA_WRITE);

        if (!(mpImageBuffer->lockBuf("TPNode", usage))) {
            muState = STATE_ACQUIRED_ERROR;
            MY_LOGE("node buffer(%s): lock image buffer failed", bufferName.string());
            return;
        }

        muState = STATE_ACQUIRED;
    });
}

/******************************************************************************
 *
 ******************************************************************************/
IImageBuffer*
TPNodeImp::FrameBuffer::
native()
{
    Mutex::Autolock _l(mLock);

    if (muState & STATE_ACQUIRED_ERROR)
        return NULL;

    if (mpImageBuffer.get())
        return mpImageBuffer.get();


    if (mType == STREAM_BUFFER) {
        mpImageBufferHeap = (mNamedId & INPUT) ?
                            mpStreamBuffer->tryReadLock(getNodeName()) :
                            mpStreamBuffer->tryWriteLock(getNodeName());

        if (mpImageBufferHeap == NULL) {
            MY_LOGE("[node:%#" PRIxPTR "][stream buffer:%s] cannot get ImageBufferHeap",
                    getNodeId(), mpStreamBuffer->getName());
            muState |= STATE_ACQUIRED_ERROR;
            return NULL;
        }

        mpImageBuffer = mpImageBufferHeap->createImageBuffer();
        if (mpImageBuffer == NULL) {
            MY_LOGE("[node:%#" PRIxPTR "][stream buffer:%s] cannot create ImageBuffer",
                    getNodeId(), mpStreamBuffer->getName());
            muState |= STATE_ACQUIRED_ERROR;
            return NULL;
        }

        MUINT groupUsage = mpStreamBuffer->queryGroupUsage(getNodeId());
        mpImageBuffer->lockBuf(getNodeName(), groupUsage);
        MY_LOGD_IF(TP_DEBUG, "stream %#" PRIx64 ": buffer: %p, usage: %x",
                   mStreamId, mpImageBuffer.get(), groupUsage);

        muState |= (STATE_ACQUIRED | STATE_USED);
    } else if (mType == NODE_BUFFER) {
        // wait for job done if the buffer doesen't be allocated yet
        mJob.wait();
    }

    return mpImageBuffer.get();
}

/******************************************************************************
 *
 ******************************************************************************/
TPNodeImp::FrameBuffer::
~FrameBuffer()
{
    if (mType == STREAM_BUFFER) {
        if (mpImageBuffer.get())
            mpImageBuffer->unlockBuf(getNodeName());

        if (mpImageBufferHeap.get())
            mpStreamBuffer->unlock(getNodeName(), mpImageBufferHeap.get());

        if (mNamedId & OUTPUT) {
            mpStreamBuffer->markStatus((muState & STATE_USED) ?
                                  STREAM_BUFFER_STATUS::WRITE_OK :
                                  STREAM_BUFFER_STATUS::WRITE_ERROR);
        }

        IStreamBufferSet &streamBufferSet = mpOwner->getStreamBufferSet();
        streamBufferSet.markUserStatus(mStreamId, getNodeId(),
                IUsersManager::UserStatus::RELEASE |
                ((muState & STATE_USED) ? IUsersManager::UserStatus::USED :0 ));

    } else if (mType == NODE_BUFFER) {
        mJob.wait();
        if (mpImageBuffer.get())
            mpImageBuffer->unlockBuf("TPNode");
    }

    MY_LOGD_IF(TP_DEBUG, "release buffer, stream:%#" PRIx64 " state:%#" PRIx32 , mStreamId, muState);
}

/******************************************************************************
 *
 ******************************************************************************/
TPNodeImp::FrameMetadata::
FrameMetadata(
    FrameRequest *pRequest,
    StreamId_T const streamId,
    NamedId_T const namedId)
    : mpOwner(pRequest)
    , mStreamId(streamId)
    , muState(0)
    , mNamedId(namedId)
    , mpMetadata(NULL)
{
    if (OK != mpOwner->getMetaStreamBuffer(mStreamId, mpStreamBuffer)) {
        muState = STATE_ACQUIRED_ERROR;
        MY_LOGE("failed to get metadata, stream:%#" PRIx64, mStreamId);
    }

    MY_LOGD_IF(TP_DEBUG, "create metadata, stream:%#" PRIx64, mStreamId);
}

/******************************************************************************
 *
 ******************************************************************************/
IMetadata*
TPNodeImp::FrameMetadata::
native()
{
    Mutex::Autolock _l(mLock);

    if (muState & STATE_ACQUIRED_ERROR)
        return NULL;

    if (!mpMetadata) {
        mpMetadata = (mNamedId & INPUT) ?
                            mpStreamBuffer->tryReadLock(getNodeName()) :
                            mpStreamBuffer->tryWriteLock(getNodeName());

        if (mpMetadata == NULL) {
            MY_LOGE("[node:%#" PRIxPTR "][stream buffer:%s] cannot get metadata",
                    getNodeId(), mpStreamBuffer->getName());
            muState |= STATE_ACQUIRED_ERROR;
        } else {
            muState |= (STATE_ACQUIRED | STATE_USED);
        }
    }

    return mpMetadata;
}

/******************************************************************************
 *
 ******************************************************************************/
TPNodeImp::FrameMetadata::
~FrameMetadata()
{
    if (mpMetadata)
        mpStreamBuffer->unlock(getNodeName(), mpMetadata);

    if (mNamedId & OUTPUT) {
        mpStreamBuffer->markStatus((muState & (STATE_USED)) ?
                              STREAM_BUFFER_STATUS::WRITE_OK :
                              STREAM_BUFFER_STATUS::WRITE_ERROR);
    }

    IStreamBufferSet &streamBufferSet = mpOwner->getStreamBufferSet();

    streamBufferSet.markUserStatus(mStreamId, getNodeId(),
                IUsersManager::UserStatus::RELEASE |
                ((muState & STATE_USED) ? IUsersManager::UserStatus::USED :0 ));

    MY_LOGD_IF(TP_DEBUG, "release metadata, stream:%#" PRIx64 " state:%#" PRIx32 , mStreamId, muState);
}


/******************************************************************************
 *
 ******************************************************************************/
TPNodeImp::FrameRequest::
FrameRequest(
    TPNodeImp* pNodeImp,
    sp<IPipelineFrame> const &pFrame)
    : mpNodeImp(pNodeImp)
    , mpFrame(pFrame)
{
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
TPNodeImp::FrameRequest::
makeBuffer(StreamId_T streamId, NamedId_T namedId) {
    mvpBuffer.add(namedId, new FrameBuffer(this, streamId, namedId));
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
TPNodeImp::FrameRequest::
makeMetadata(StreamId_T streamId, NamedId_T namedId) {
    mvpMetadata.add(namedId, new FrameMetadata(this, streamId, namedId));
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
TPNodeImp::FrameRequest::
releaseBuffers()
{
    mpFrame->getStreamBufferSet().applyRelease(getNodeId());
}

/******************************************************************************
 *
 ******************************************************************************/
TPNodeImp::FrameRequest::
~FrameRequest()
{
    mvpBuffer.clear();
    mvpMetadata.clear();

    releaseBuffers();

    mpNodeImp->onDispatchFrame(mpFrame);

    MY_LOGD_IF(TP_DEBUG, "destroy frame request, frame:%d", mpFrame->getFrameNo());
}

/******************************************************************************
 *
 ******************************************************************************/
sp<TPNodeImp::FrameBuffer>&
TPNodeImp::GroupedFrame::
getGroupedBuffer(GroupId_T groupId, NamedId_T namedId)
{
    MUINT64 const uGnId = GROUP_NAMED_ID(groupId, namedId);
    return mvpGrpBuffer.editValueFor(uGnId);
}

/******************************************************************************
 *
 ******************************************************************************/
sp<TPNodeImp::FrameMetadata>&
TPNodeImp::GroupedFrame::
getGroupedMetadata(GroupId_T groupId, NamedId_T namedId)
{
    MUINT64 const uGnId = GROUP_NAMED_ID(groupId, namedId);
    return mvpGrpMetadata.editValueFor(uGnId);
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
TPNodeImp::GroupedFrame::
removeByTags(MUINT64 tags)
{
    size_t cnt;
    ssize_t i;

    cnt = mvpGrpBuffer.size();
    for (i = cnt - 1; i >= 0; i--)
        if ((mvpGrpBuffer.keyAt(i) & tags) == tags)
            mvpGrpBuffer.removeItemsAt(i);

    cnt = mvpGrpMetadata.size();
    for (i = cnt - 1; i >= 0; i--)
        if ((mvpGrpMetadata.keyAt(i) & tags) == tags)
            mvpGrpMetadata.removeItemsAt(i);
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
TPNodeImp::GroupedFrame::
removeByTagsExceptGroup(MUINT64 tags, GroupId_T group)
{
    size_t cnt;
    ssize_t i;
    MUINT64 uGnId;

    cnt = mvpGrpBuffer.size();
    for (i = cnt - 1; i >= 0; i--) {
        uGnId = mvpGrpBuffer.keyAt(i);
        if ((uGnId & tags) == tags && (uGnId >> GROUP_OFFSET != group))
            mvpGrpBuffer.removeItemsAt(i);
    }

    cnt = mvpGrpMetadata.size();
    for (i = cnt - 1; i >= 0; i--) {
        uGnId = mvpGrpMetadata.keyAt(i);
        if ((uGnId & tags) == tags && (uGnId >> GROUP_OFFSET != group))
            mvpGrpMetadata.removeItemsAt(i);
    }
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
TPNodeImp::GroupedFrame::
contains(GroupId_T groupId, NamedId_T namedId)
{
    MUINT64 const uGnId = GROUP_NAMED_ID(groupId, namedId);
    return (namedId & METADATA && mvpGrpMetadata.indexOfKey(uGnId) >= 0)
            || (mvpGrpBuffer.indexOfKey(uGnId) >= 0);
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
TPNodeImp::GroupedFrame::
onCallback(MVOID* token, MERROR result)
{
    sp<GroupedFrame> pGrpFrame = (GroupedFrame*)token;
    if (result != OK)
        pGrpFrame->markState(GROUP_PLUGIN_ERROR);

    pGrpFrame->finished();
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
TPNodeImp::GroupedFrame::
finished()
{
    MultiFrameCollector *pCollector = reinterpret_cast<MultiFrameCollector *>(mpToken);
    pCollector->onFinish(this);
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
TPNodeImp::GroupedFrame::
dumpLog()
{
    size_t cnt;
    size_t i;
    MUINT64 uGnId;

    MY_LOGD("request(%d), state:%#" PRIx32, muRequestNo, muState);

    cnt = mvpGrpBuffer.size();
    for (i = 0; i < cnt; i++) {
        uGnId = mvpGrpBuffer.keyAt(i);
        MY_LOGD("  group(%" PRIu32 ") buffer, named id:%#" PRIx32,
            (MUINT32)(uGnId >> GROUP_OFFSET), (MUINT32)(uGnId));
    }

    cnt = mvpGrpMetadata.size();
    for (i = 0; i < cnt; i++) {
        uGnId = mvpGrpMetadata.keyAt(i);
        MY_LOGD("  group(%" PRIu32 ") metadata, named id:%#" PRIx32,
            (MUINT32)(uGnId >> GROUP_OFFSET), (MUINT32)(uGnId));
    }
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
TPNodeImp::MultiFrameCollector::
begin(sp<FrameRequest> pRequest)
{
    mpScopedRequest = pRequest;

    if (mpGrouping->mvpRequests.empty()) {
        mpGrouping->muRequestNo = pRequest->getRequestNo();
    } else if (mpGrouping->muRequestNo != pRequest->getRequestNo()) {
        MY_LOGW("there is different request number(%u) in a group(%u)",
                pRequest->getRequestNo(), mpGrouping->muRequestNo);
    }
    mpGrouping->mvpRequests.push_back(pRequest);

}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
TPNodeImp::MultiFrameCollector::
end()
{
    mpScopedRequest.clear();
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
TPNodeImp::MultiFrameCollector::
push(GroupId_T groupId, NamedId_T namedId)
{
    MUINT64 const uGnId = GROUP_NAMED_ID(groupId, namedId);

    if (!mpScopedRequest->contains(namedId))
        return;

    if (namedId & METADATA)
        mpGrouping->mvpGrpMetadata.add(uGnId, mpScopedRequest->getMetadata(namedId));
    else
        mpGrouping->mvpGrpBuffer.add(uGnId, mpScopedRequest->getBuffer(namedId));

    mpScopedRequest->remove(namedId);
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
TPNodeImp::MultiFrameCollector::
push(GroupId_T groupId, sp<FrameBuffer> pBuffer)
{
    MUINT64 const uGnId = GROUP_NAMED_ID(groupId, pBuffer->getNamedId());
    mpGrouping->mvpGrpBuffer.add(uGnId, pBuffer);
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
TPNodeImp::MultiFrameCollector::
enque()
{
    sp<GroupedFrame> pGrpFrame = mpGrouping;
    mpGrouping = new GroupedFrame();

    MY_LOGD("queue grouped frame: requestNo(%u) count(%zu) correct(%d)",
            pGrpFrame->getRequestNo(), pGrpFrame->getRequestCount(), pGrpFrame->isCorrect());

    pGrpFrame->mpToken = this;

    {
        Mutex::Autolock _l(mGroupQueueLock);
        mvpGroupQueue.push_back(pGrpFrame);
    }

    if (pGrpFrame->isCorrect()) {
        mpNodeImp->doPlugin(pGrpFrame);
    } else
        pGrpFrame->finished();
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
TPNodeImp::MultiFrameCollector::
flush()
{
    if (mpScopedRequest.get())
        MY_LOGE("the scoped request should be NULL");

    {
        Mutex::Autolock _l(mGroupQueueLock);

        if (mpGrouping->getRequestCount() > 0) {
            mpGrouping->markState(GROUP_FLUSH_ERROR);
            mpGrouping = new GroupedFrame();
        }

        if (mvpGroupQueue.size() != 0) {
            mGroupQueueCond.wait(mGroupQueueLock);
        }
    }
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
TPNodeImp::MultiFrameCollector::
onFinish(sp<GroupedFrame> pGrpFrame)
{
    MUINT32 const uFrameCount = pGrpFrame->getRequestCount();
    MUINT32 const uLastIndex = uFrameCount - 1;

    IMetadata* pInHalMetaFirst = pGrpFrame->getGroupedMetadata(0, STRM_META_IN_HAL)->native();
    IMetadata* pInHalMetaLast = pGrpFrame->getGroupedMetadata(uLastIndex, STRM_META_IN_HAL)->native();

    IMetadata* pInAppMetaFirst = pGrpFrame->getGroupedMetadata(0, STRM_META_IN_APP)->native();
    IMetadata* pInAppMetaLast = pGrpFrame->getGroupedMetadata(uLastIndex, STRM_META_IN_APP)->native();

    // 1. use the first frame to output jpeg exif and crop region
    *pInHalMetaLast = *pInHalMetaFirst;
    *pInAppMetaLast = *pInAppMetaFirst;

    // 2. release all input buffers, and output buffers excepts the last frame
    pGrpFrame->removeByTagsExceptGroup(INPUT, uLastIndex);

    // 3. releaes frames earily
    for (size_t i = 0 ; i < uFrameCount - 1; i++) {
        pGrpFrame->releaseRequestAt(i);
    }

    // 4. copy the blended image to destination buffers
    if (pGrpFrame->isCorrect())
        mpNodeImp->mpCopyProcessor->queueCopyFrame(pGrpFrame);
    else
        MY_LOGE("there is error occurred on request:%u", pGrpFrame->getRequestNo());

    // 5. remove from running queue
    {
        Mutex::Autolock _l(mGroupQueueLock);
        auto it = mvpGroupQueue.begin();
        for (; it != mvpGroupQueue.end(); it++) {
            if ((*it).get() == pGrpFrame.get()) {
                mvpGroupQueue.erase(it);
                mGroupQueueCond.broadcast();
                return;
            }
        }
    }

    // 5. should not be here
    MY_LOGE("can not match the grouped frame!");
}



/******************************************************************************
 *
 ******************************************************************************/
sp<TPNode>
TPNode::
createInstance()
{
    if(!getDebugExif()) {
        MY_LOGE("bad getDebugExif()");
        return NULL;
    }

    return new TPNodeImp();
}

/******************************************************************************
 *
 ******************************************************************************/
TPNodeImp::
TPNodeImp()
    : mFrameQueue()
    , mbFrameDrained(MTRUE)
    , mbNodeExit(MFALSE)
    , mRequestIndex(0)
    , mpCollector(NULL)
{
    mLogLevel = property_get_int32("debug.camera.log", 0);
    if (mLogLevel == 0)
        mLogLevel = property_get_int32("debug.camera.log.tpnode", 0);

    mDebugDump = property_get_int32("debug.camera.dump.tpnode", 0);

    // make debug dump path
    if (mDebugDump && !NSCam::Utils::makePath(TP_DUMP_PATH, 0660)) {
        MY_LOGW("make debug dump path %s failed", TP_DUMP_PATH);
    }
}

/******************************************************************************
 *
 ******************************************************************************/
TPNodeImp::
~TPNodeImp()
{
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
TPNodeImp::
init(const InitParams& rParams)
{
    RWLock::AutoWLock _l(mConfigRWLock);

    mOpenId   = rParams.openId;
    mNodeId   = rParams.nodeId;
    mNodeName = rParams.nodeName;

    // create worker threads
    mpNodeProcessor = new NodeProcessor(this);
    if (OK != mpNodeProcessor->run(TP_THREAD_NAME_NODE)) {
        MY_LOGE("create node thread failed");
        return INVALID_OPERATION;
    }

    mpCopyProcessor = new CopyProcessor(this);
    if (OK != mpCopyProcessor->run(TP_THREAD_NAME_COPY)) {
        MY_LOGE("create copy thread failed");
        return INVALID_OPERATION;
    }

    mpCollector = new MultiFrameCollector(this);

    // query active array size
    sp<IMetadataProvider> pMetadataProvider = NSMetadataProviderManager::valueFor(getOpenId());
    if (pMetadataProvider.get()) {

        IMetadata const& staticMetadata = pMetadataProvider->getMtkStaticCharacteristics();
        if (!IMetadata::getEntry<MRect>(&staticMetadata, MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION, mActiveArray)) {
            MY_LOGE("no static info: MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION");
            return BAD_VALUE;
        } else {
            MY_LOGD("active array(%d, %d, %dx%d)",
                    mActiveArray.p.x, mActiveArray.p.y,
                    mActiveArray.s.w, mActiveArray.s.h);
        }
    } else {
        MY_LOGE("there is no metadata provider!");
        return INVALID_OPERATION;
    }

    MY_LOGD("OpenId(%d) NodeId(%#08" PRIxPTR ") NodeName(%s)",
            getOpenId(), getNodeId(), getNodeName());

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
TPNodeImp::
uninit()
{
    if (OK != flush())
        MY_LOGE("flush failed");

    // TODO: check if pending requests are handled properly before exiting threads
    // destroy worker threads
    mpNodeProcessor->requestExit();
    mpNodeProcessor->join();
    mpNodeProcessor = NULL;

    mpCopyProcessor->requestExit();
    mpCopyProcessor->join();
    mpCopyProcessor = NULL;

    delete mpCollector;

    removePlugins();

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
TPNodeImp::
config(const ConfigParams& rParams)
{
    if (verifyConfigParams(rParams) != OK)
        return BAD_VALUE;

    flush();

    {
        RWLock::AutoWLock _l(mConfigRWLock);
        // meta
        mpInAppMeta         = rParams.pInAppMeta;
        mpInHalMeta         = rParams.pInHalMeta;
        mpOutAppMeta        = rParams.pOutAppMeta;
        mpOutHalMeta        = rParams.pOutHalMeta;
        // image
        mpInFullYuv         = rParams.pInFullYuv;
        mpInResizedYuv      = rParams.pInResizedYuv;
        mpInBinningYuv      = rParams.pInBinningYuv;
        mpOutYuvJpeg        = rParams.pOutYuvJpeg;
        mpOutYuvThumbnail   = rParams.pOutYuvThumbnail;
        mpOutYuv00          = rParams.pOutYuv00;
        mpOutYuv01          = rParams.pOutYuv01;
        mpOutDepth          = rParams.pOutDepth;
        mpOutClean          = rParams.pOutClean;
    }

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
TPNodeImp::
flush()
{
    {
        Mutex::Autolock _l(mFrameQueueLock);

        FrameQueueT::iterator it = mFrameQueue.begin();
        while (it != mFrameQueue.end()) {
            BaseNode::flush(*it);
            it = mFrameQueue.erase(it);
        }
    }

    waitUntilDrained();

    mpCollector->flush();

    mpCopyProcessor->waitUntilDrained();

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
TPNodeImp::
queue(sp<IPipelineFrame> pFrame)
{
    if (!pFrame.get()) {
        MY_LOGE("null frame");
        return BAD_VALUE;
    }

    Mutex::Autolock _l(mFrameQueueLock);

    // make sure the request with a smaller frame number has a higher priority
    FrameQueueT::iterator it = mFrameQueue.end();
    for (; it != mFrameQueue.begin();) {
        --it;
        if (0 <= (MINT32)(pFrame->getFrameNo() - (*it)->getFrameNo())) {
            ++it;
            break;
        }
    }

    // insert before the current frame
    mFrameQueue.insert(it, pFrame);

    mFrameQueueCond.broadcast();

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
TPNodeImp::
mapToRequest(sp<IPipelineFrame> const &pFrame, sp<FrameRequest> &rpRequest)
{
    // 1. get IOMap
    IPipelineFrame::InfoIOMapSet ioMapSet;
    if (OK != getInfoIOMapSet(pFrame, ioMapSet)) {
        MY_LOGE("fail to get frame's IOMap");
        return BAD_VALUE;
    }

    // 2. create a request
    rpRequest = new FrameRequest(this, pFrame);

    // 3. create metadata (based on IOMap)
    if (mpInAppMeta.get() && isMetaStreamAtIOMap(ioMapSet, mpInAppMeta->getStreamId()))
        rpRequest->makeMetadata(mpInAppMeta->getStreamId(), STRM_META_IN_APP);

    if (mpInHalMeta.get() && isMetaStreamAtIOMap(ioMapSet, mpInHalMeta->getStreamId()))
        rpRequest->makeMetadata(mpInHalMeta->getStreamId(), STRM_META_IN_HAL);

    if (mpOutAppMeta.get() && isMetaStreamAtIOMap(ioMapSet, mpOutAppMeta->getStreamId()))
        rpRequest->makeMetadata(mpOutAppMeta->getStreamId(), STRM_META_OUT_APP);

    if (mpOutHalMeta.get() && isMetaStreamAtIOMap(ioMapSet, mpOutHalMeta->getStreamId()))
        rpRequest->makeMetadata(mpOutHalMeta->getStreamId(), STRM_META_OUT_HAL);

    // 4. create image (based on IOMap)
    if (mpInFullYuv.get() && isImageStreamAtIOMap(ioMapSet, mpInFullYuv->getStreamId()))
        rpRequest->makeBuffer(mpInFullYuv->getStreamId(), STRM_BUF_IN_FULL);

    if (mpInResizedYuv.get() && isImageStreamAtIOMap(ioMapSet, mpInResizedYuv->getStreamId()))
        rpRequest->makeBuffer(mpInResizedYuv->getStreamId(), STRM_BUF_IN_RSZ);

    if (mpInBinningYuv.get() && isImageStreamAtIOMap(ioMapSet, mpInBinningYuv->getStreamId()))
        rpRequest->makeBuffer(mpInBinningYuv->getStreamId(), STRM_BUF_IN_BIN);

    if (mpOutYuvJpeg.get() && isImageStreamAtIOMap(ioMapSet, mpOutYuvJpeg->getStreamId()))
        rpRequest->makeBuffer(mpOutYuvJpeg->getStreamId(), STRM_BUF_OUT_JPEG);

    if (mpOutYuvThumbnail.get() && isImageStreamAtIOMap(ioMapSet, mpOutYuvThumbnail->getStreamId()))
        rpRequest->makeBuffer(mpOutYuvThumbnail->getStreamId(), STRM_BUF_OUT_THN);

    if (mpOutYuv00.get() && isImageStreamAtIOMap(ioMapSet, mpOutYuv00->getStreamId()))
        rpRequest->makeBuffer(mpOutYuv00->getStreamId(), STRM_BUF_OUT_YUV00);

    if (mpOutYuv01.get() && isImageStreamAtIOMap(ioMapSet, mpOutYuv01->getStreamId()))
        rpRequest->makeBuffer(mpOutYuv01->getStreamId(), STRM_BUF_OUT_YUV01);

    if (mpOutDepth.get() && isImageStreamAtIOMap(ioMapSet, mpOutDepth->getStreamId()))
        rpRequest->makeBuffer(mpOutDepth->getStreamId(), STRM_BUF_OUT_DEPTH);

    if (mpOutClean.get() && isImageStreamAtIOMap(ioMapSet, mpOutClean->getStreamId()))
        rpRequest->makeBuffer(mpOutClean->getStreamId(), STRM_BUF_OUT_CLEAN);

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
TPNodeImp::
onProcessFrame(sp<IPipelineFrame> const& pFrame)
{
    MY_LOGD("mRequestIndex(%d) frameNo(%u) requestNo(%u) nodeId(%#08" PRIxPTR ")",
            mRequestIndex, pFrame->getFrameNo(), pFrame->getRequestNo(), getNodeId());

    if (mRequestIndex >= MAX_NUM_OF_FRAME) {
        MY_LOGE("over the max number of frame, count:%d", mRequestIndex + 1);
        BaseNode::flush(pFrame);
        mpCollector->flush();
        return;
    }

    // map pipeline frame to request
    sp<FrameRequest> pRequest;
    if (OK != mapToRequest(pFrame, pRequest)) {
        MY_LOGE("fail to map pipeline frame to frame request");
        pRequest.clear();
        BaseNode::flush(pFrame);
        return;
    }

    GroupId_T const uGroup = mRequestIndex;
    MBOOL const bFinal = pRequest->contains(STRM_BUF_OUT_JPEG);

    // !! request begin
    mpCollector->begin(pRequest);

    if (uGroup == 0) {

        // [*] 1.allocate a working buffer in background, using async job
        if (pRequest->contains(STRM_BUF_IN_FULL)) {

            IImageBuffer* pFullBuffer = pRequest->getBuffer(STRM_BUF_IN_FULL)->native();

            sp<FrameBuffer> pTempImage0 =(new FrameBuffer(
                    pFullBuffer->getImgFormat(),
                    pFullBuffer->getImgSize(),
                    TEMP_BUF_FULL00));

            mpCollector->push(0, pTempImage0);

            // for Clean Image
            if (mpOutClean.get()) {
                sp<FrameBuffer> pTempImage1 =(new FrameBuffer(
                        pFullBuffer->getImgFormat(),
                        pFullBuffer->getImgSize(),
                        TEMP_BUF_FULL01));

                mpCollector->push(0, pTempImage1);
            }
        } else {
            mpCollector->markError();
            MY_LOGE("there is no input full-size yuv! It's marked error!");
        }

        // [*] 2.confirm the execution plugin
        PluginId_T pluginId = 0;
        if (pRequest->contains(STRM_META_IN_HAL)) {
            IMetadata* pHalMeta = pRequest->getMetadata(STRM_META_IN_HAL)->native();
            IMetadata::getEntry<MINT32>(pHalMeta, MTK_PLUGIN_MODE, pluginId);
        } else {
            mpCollector->markError();
            MY_LOGE("there is no input metadata!");
        }

        // [*] 3,init the specific plugin. (it's only default plugin for now)
        if (pluginId) {
            if (initPlugin(pluginId) != OK) {
                mpCollector->markError();
                MY_LOGE("fail to init plugin(%d)", pluginId);
            }
        } else {
            mpCollector->markError();
            MY_LOGE("no input hal metadata: MTK_PLUGIN_MODE");
        }
    }

    mpCollector->push(uGroup, STRM_META_IN_APP);
    mpCollector->push(uGroup, STRM_META_IN_HAL);
    mpCollector->push(uGroup, STRM_BUF_IN_FULL);
    mpCollector->push(uGroup, STRM_BUF_IN_RSZ);
    mpCollector->push(uGroup, STRM_BUF_IN_BIN);

    if (bFinal) {
        mpCollector->push(0, STRM_META_OUT_APP);
        mpCollector->push(0, STRM_META_OUT_HAL);
        mpCollector->push(0, STRM_BUF_OUT_JPEG);
        mpCollector->push(0, STRM_BUF_OUT_THN);
        mpCollector->push(0, STRM_BUF_OUT_DEPTH);
        mpCollector->push(0, STRM_BUF_OUT_CLEAN);
    }

    mpCollector->end();
    // !! request end

    // notify plugin by frame
    {
        sp<GroupedFrame> pGrpFrame = mpCollector->getGrouping();

        TPNodePlugin::PartialParams params;
        memset(&params, 0, sizeof(params));

        params.uRequestNo = pGrpFrame->getRequestNo();

        params.in.pFullYuv = pGrpFrame->contains(uGroup, STRM_BUF_IN_FULL) ?
            pGrpFrame->getGroupedBuffer(uGroup, STRM_BUF_IN_FULL)->native() : NULL;

        params.in.pResizedYuv = pGrpFrame->contains(uGroup, STRM_BUF_IN_RSZ) ?
            pGrpFrame->getGroupedBuffer(uGroup, STRM_BUF_IN_RSZ)->native() : NULL;

        params.in.pBinningYuv = pGrpFrame->contains(uGroup, STRM_BUF_IN_BIN) ?
            pGrpFrame->getGroupedBuffer(uGroup, STRM_BUF_IN_BIN)->native() : NULL;

        params.in.pHalMeta = pGrpFrame->contains(uGroup, STRM_META_IN_HAL) ?
            pGrpFrame->getGroupedMetadata(uGroup, STRM_META_IN_HAL)->native() : NULL;

        params.in.pAppMeta = pGrpFrame->contains(uGroup, STRM_META_IN_APP) ?
            pGrpFrame->getGroupedMetadata(uGroup, STRM_META_IN_APP)->native() : NULL;

        if (bFinal) {
            params.out.pFullYuv = pGrpFrame->contains(0, TEMP_BUF_FULL00) ?
                pGrpFrame->getGroupedBuffer(0, TEMP_BUF_FULL00)->native() : NULL;

            params.out.pHalMeta = pGrpFrame->contains(0, STRM_META_OUT_HAL) ?
                pGrpFrame->getGroupedMetadata(0, STRM_META_OUT_HAL)->native() : NULL;

            params.out.pAppMeta = pGrpFrame->contains(0, STRM_META_OUT_APP) ?
                pGrpFrame->getGroupedMetadata(0, STRM_META_OUT_APP)->native() : NULL;

            params.out.pDepth = pGrpFrame->contains(0, STRM_BUF_OUT_DEPTH) ?
                pGrpFrame->getGroupedBuffer(0, STRM_BUF_OUT_DEPTH)->native() : NULL;

            params.out.pClean = pGrpFrame->contains(0, TEMP_BUF_FULL01) ?
                pGrpFrame->getGroupedBuffer(0, TEMP_BUF_FULL01)->native() : NULL;

            params.bIsLastPartial = MTRUE;
        }

        PluginId_T pid = 0;
        TPNodePlugin* pPlugin = NULL;
        IMetadata* pHalMeta = params.in.pHalMeta;
        if (pHalMeta) {
            IMetadata::getEntry<MINT32>(pHalMeta, MTK_PLUGIN_MODE, pid);
            IMetadata::getEntry<MINT32>(pHalMeta, MTK_STEREO_FEATURE_OPEN_ID, params.in.nSensorId);
        }

        if (pid)
            pPlugin = getPlugin(pid);

        if (pPlugin) {
            pPlugin->onFrameQueue(params);
        } else {
            MY_LOGE("no tag [MTK_PLUGIN_MODE]!");
        }
    }

    if (bFinal) {
        mpCollector->enque();
        mRequestIndex = 0;
    } else
        mRequestIndex += 1;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
TPNodeImp::CopyProcessor::
queueCopyFrame(sp<GroupedFrame> pGrpFrame)
{
    {
        Mutex::Autolock _l(mCopyLock);
        std::function<void(void)> func = std::bind(
                &TPNodeImp::copyBuffer,
                mpNodeImp,
                pGrpFrame);

        mCopyQueue.emplace(std::move(func));
        mbCopyDrained = MFALSE;
    }
    mCopyCond.signal();

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
TPNodeImp::
copyBuffer(sp<GroupedFrame> pGrpFrame)
{
    CAM_TRACE_NAME("TP::MDP");

    // At this time, should remain the last request in grouped request
    MUINT32 uLastIndex = pGrpFrame->getRequestCount() - 1;

    if (!(pGrpFrame->contains(uLastIndex, STRM_META_IN_HAL) &&
        pGrpFrame->contains(uLastIndex, STRM_META_IN_APP) &&
        pGrpFrame->contains(0, STRM_META_OUT_HAL) &&
        pGrpFrame->contains(0, TEMP_BUF_FULL00) &&
        pGrpFrame->contains(0, STRM_BUF_OUT_JPEG) &&
        pGrpFrame->contains(0, STRM_BUF_OUT_THN)))
    {
        pGrpFrame->markState(GROUP_COPY_ERROR);
        MY_LOGE("there are buffers missing or be released!");
        return BAD_VALUE;
    }

    // 1. get required buffer & metadata
    IMetadata* pInHalMeta = pGrpFrame->getGroupedMetadata(uLastIndex, STRM_META_IN_HAL)->native();
    IMetadata* pInAppMeta = pGrpFrame->getGroupedMetadata(uLastIndex, STRM_META_IN_APP)->native();
    IMetadata* pOutHalMeta = pGrpFrame->getGroupedMetadata(0, STRM_META_OUT_HAL)->native();
    IImageBuffer* pTmpBuf0 = pGrpFrame->getGroupedBuffer(0, TEMP_BUF_FULL00)->native();
    IImageBuffer* pOutYuvJpeg = pGrpFrame->getGroupedBuffer(0, STRM_BUF_OUT_JPEG)->native();
    IImageBuffer* pOutYuvThumbnail = pGrpFrame->getGroupedBuffer(0, STRM_BUF_OUT_THN)->native();

    if (!pTmpBuf0 || !pOutYuvJpeg || !pOutYuvThumbnail) {
        pGrpFrame->markState(GROUP_COPY_ERROR);
        MY_LOGE("copy(%p) -> yuv jpeg(%p) yuv thumbnail(%p)",
                pTmpBuf0, pOutYuvJpeg, pOutYuvThumbnail);
        return BAD_VALUE;
    }

    // 2. generate output metadata & debug information
    if (pInHalMeta && pOutHalMeta) {
        MUINT8 uRequiredExif = 0;
        IMetadata::getEntry<MUINT8>(pInHalMeta, MTK_HAL_REQUEST_REQUIRE_EXIF, uRequiredExif);

        if (uRequiredExif && makeDebugInfo(pOutHalMeta)) {
            MY_LOGW("can not make debug information");
        }

        // The OutHalMeta should contain all values of InHalMeta
        *pOutHalMeta = *pInHalMeta + *pOutHalMeta;
    }

    // 3. calculate crop region
    MINT32 openId = 0;
    MRect activeArray;
    if (IMetadata::getEntry<MINT32>(pInHalMeta, MTK_STEREO_FEATURE_OPEN_ID, openId)) {
        sp<IMetadataProvider> pMetadataProvider = NSMetadataProviderManager::valueFor(openId);
        if (pMetadataProvider.get()) {
            IMetadata const& rStaticMetadata = pMetadataProvider->getMtkStaticCharacteristics();
            if (!tryGetMetadata<MRect>(&rStaticMetadata, MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION, activeArray)) {
                MY_LOGE("no static info: MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION");
                return BAD_VALUE;
            }
        } else {
            MY_LOGW("there is no metadata provider!");
            return BAD_VALUE;
        }
    } else {
        MY_LOGW("cannot find open id, and use the init active array region");
        activeArray = mActiveArray;
   }

    MRect cropRegion;
    if (!IMetadata::getEntry<MRect>(pInAppMeta, MTK_SCALER_CROP_REGION, cropRegion)) {
        cropRegion.p = MPoint(0, 0);
        cropRegion.s = activeArray.s;
        MY_LOGW("no MTK_SCALER_CROP_REGION, set crop region to full size %dx%d", cropRegion.s.w, cropRegion.s.h);
    }

    MY_LOGD("active array(%dx%d) crop region(%d, %d, %d, %d)",
            activeArray.s.w, activeArray.s.h,
            cropRegion.p.x, cropRegion.p.y, cropRegion.s.w, cropRegion.s.h);

    const MSize& tmpSize0 = pTmpBuf0->getImgSize();
    simpleTransform tranActive2Sensor = simpleTransform(MPoint(0, 0), activeArray.s, tmpSize0);

    cropRegion = transform(tranActive2Sensor, cropRegion);

    // write result to jpeg and thumbnail buffer
    MUINT32 trans = pGrpFrame->getGroupedBuffer(0, STRM_BUF_OUT_JPEG)->getTransform();

    MY_LOGD("full(%dx%d) -> jpeg(%dx%d) thumbnail(%dx%d) crop(%d, %d, %dx%d) transform(%d)",
            tmpSize0.w, tmpSize0.h,
            pOutYuvJpeg->getImgSize().w, pOutYuvJpeg->getImgSize().h,
            pOutYuvThumbnail->getImgSize().w, pOutYuvThumbnail->getImgSize().h,
            cropRegion.p.x, cropRegion.p.y, cropRegion.s.w, cropRegion.s.h, trans);

    // the aspect ratios among sensor, jpeg and thumbnail may be different,
    // so adjust the aspect ratio of the source crop region to be the same
    // as the destination one
    std::function<MRect(const MRect&, const MRect&)> getFinalCropRegion =
        [&](const MRect& srcRect, const MRect& dstRect) {
            // recalculate the final crop
            MRect finalCrop = calCrop(srcRect, dstRect, MFALSE);

            // let the final crop to be centered within the tp result's buffer
            finalCrop.p.x = (tmpSize0.w - finalCrop.s.w) / 2;
            finalCrop.p.y = (tmpSize0.h - finalCrop.s.h) / 2;

            MY_LOGD("finalCrop(%d, %d, %dx%d) dst(%d, %d, %dx%d)",
                    finalCrop.p.x, finalCrop.p.y,
                    finalCrop.s.w, finalCrop.s.h,
                    dstRect.p.x, dstRect.p.y, dstRect.s.w, dstRect.s.h);

            return finalCrop;
        };

    MRect jpegCropRegion = MRect(tmpSize0.w, tmpSize0.h);
    MRect thumbnailCropRegion = MRect(tmpSize0.w, tmpSize0.h);

    const MSize& transJpegSize = (trans & eTransform_ROT_90) ?
        MSize(pOutYuvJpeg->getImgSize().h, pOutYuvJpeg->getImgSize().w):
        MSize(pOutYuvJpeg->getImgSize().w, pOutYuvJpeg->getImgSize().h);

    if(isSameRatio(tmpSize0, transJpegSize)){
        MY_LOGD("[jpeg] in/out with same aspect ratio, no need to crop");
    }else{
        // due to the dimension of tp output frame has been swapped,
        // in order to calculate the aspect ratio, we need to restore it
        jpegCropRegion = getFinalCropRegion(cropRegion,
            MRect(transJpegSize.w, transJpegSize.h));
    }

    if(isSameRatio(tmpSize0, MSize(pOutYuvThumbnail->getImgSize().w, pOutYuvThumbnail->getImgSize().h))){
        MY_LOGD("[thumb] in/out with same aspect ratio, no need to crop");
    }else{
        // thumbnail's orientation is always 0, so we set it directly
        thumbnailCropRegion = getFinalCropRegion(cropRegion,
            MRect(pOutYuvThumbnail->getImgSize().w, pOutYuvThumbnail->getImgSize().h));
    }

    // handle digital zoom
    MFCNodeImp::generateOutputYuv(
        mOpenId,
        pTmpBuf0,
        pOutYuvJpeg,
        pOutYuvThumbnail,
        jpegCropRegion,
        thumbnailCropRegion,
        trans,
        pInAppMeta,
        pInHalMeta);


    if (pGrpFrame->contains(0, TEMP_BUF_FULL01)) {
        IImageBuffer* pTmpBuf1 = pGrpFrame->getGroupedBuffer(0, TEMP_BUF_FULL01)->native();
        IImageBuffer* pOutClean = pGrpFrame->getGroupedBuffer(0, STRM_BUF_OUT_CLEAN)->native();
        MFCNodeImp::generateOutputYuv(
            mOpenId,
            pTmpBuf1,
            pOutClean,
            NULL,
            jpegCropRegion,
            MRect(0, 0),
            trans,
            pInAppMeta,
            pInHalMeta);
    }
    // dump output buffer if necessary
    if (mDebugDump) {

        MINT32 iKey = 0;
        MUINT32 uStrides[3] = {0};
        char szFilename[100];

        IMetadata::getEntry<MINT32>(pInHalMeta, MTK_PIPELINE_UNIQUE_KEY, iKey);
        sp<FrameRequest> const& rpRequest = pGrpFrame->getRequestAt(uLastIndex);

        for (size_t i = 0; i < pTmpBuf0->getPlaneCount(); i++)
            uStrides[i] = pTmpBuf0->getBufStridesInBytes(i);

#define IMAGE_FORMAT_TO_STRING(e)             \
            (e == eImgFmt_YV12) ? "yv12" :    \
            (e == eImgFmt_NV21) ? "nv21" :    \
            (e == eImgFmt_YUY2) ? "yuy2" :    \
            (e == eImgFmt_I420) ? "i420" :    \
            "undef"

        // jpeg
        sprintf(szFilename,
                TP_DUMP_PATH "%09d-%04d-%04d-tp-temp-%dx%d-%u_%u_%u.%s",
                iKey,rpRequest->getFrameNo() ,rpRequest->getRequestNo(),
                tmpSize0.w, tmpSize0.h,
                uStrides[0], uStrides[1], uStrides[2],
                IMAGE_FORMAT_TO_STRING(pTmpBuf0->getImgFormat()));
        pTmpBuf0->saveToFile(szFilename);

#undef IMAGE_FORMAT_TO_STRING
    }

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
TPNodeImp::
makeDebugInfo(IMetadata* metadata)
{
    CAM_TRACE_CALL();

    std::map<MUINT32, MUINT32> debugInfoList;
    debugInfoList[getDebugExif()->getTagId_MF_TAG_IMAGE_HDR()] = 1;

    MBOOL needUpdateVHDR = MFALSE;
    MINT32 vhdrMode = SENSOR_VHDR_MODE_NONE;
    if( IMetadata::getEntry<MINT32>(metadata, MTK_P1NODE_SENSOR_VHDR_MODE, vhdrMode) &&
        vhdrMode != SENSOR_VHDR_MODE_NONE ){
        needUpdateVHDR = MTRUE;
    }

    MY_LOGD("needUpdateVHDR(%d) into DEBUG_EXIF_MF", needUpdateVHDR);

    if(needUpdateVHDR){
        // get debug Exif metadata
        IMetadata exif;
        if(!IMetadata::getEntry<IMetadata>(metadata, MTK_3A_EXIF_METADATA, exif)) {
            MY_LOGW("fail to get metadata MTK_3A_EXIF_METADATA");
            return MFALSE;
        }

        // set debug information into debug Exif metadata
        if (DebugExifUtils::setDebugExif(
                DebugExifUtils::DebugExifType::DEBUG_EXIF_MF,
                static_cast<MUINT32>(MTK_MF_EXIF_DBGINFO_MF_KEY),
                static_cast<MUINT32>(MTK_MF_EXIF_DBGINFO_MF_DATA),
                debugInfoList, &exif) == nullptr) {
            return MFALSE;
        }

        // update debug Exif metadata
        IMetadata::setEntry<IMetadata>(metadata, MTK_3A_EXIF_METADATA, exif);
    }
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
TPNodeImp::
onPartialFrameDone(const sp<IPipelineFrame>& frame)
{
    CAM_TRACE_CALL();
    frame->getStreamBufferSet().applyRelease(getNodeId());
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
TPNodeImp::
verifyConfigParams(ConfigParams const & rParams) const
{
    if (!rParams.pInAppMeta.get()) {
        MY_LOGE("out in app metadata");
        return BAD_VALUE;
    }

    if (!rParams.pInHalMeta.get()) {
        MY_LOGE("no in hal metadata");
        return BAD_VALUE;
    }

    if (!rParams.pInFullYuv.get()) {
        MY_LOGE("no in image FullYuv");
        return BAD_VALUE;
    }

    if (!rParams.pInResizedYuv.get()) {
        MY_LOGD("no in resized yuv");
    }

    if (!rParams.pOutYuvJpeg.get() || !rParams.pOutYuvThumbnail.get()) {
        MY_LOGE("no out yuvJpeg(%p) yuvThumbnail(%p)",
                rParams.pOutYuvJpeg.get(), rParams.pOutYuvThumbnail.get());
        return BAD_VALUE;
    }

    dumpStreamIfExist("[meta] in app", rParams.pInAppMeta);
    dumpStreamIfExist("[meta] in hal", rParams.pInHalMeta);
    dumpStreamIfExist("[meta] out app", rParams.pOutAppMeta);
    dumpStreamIfExist("[meta] out hal", rParams.pOutHalMeta);

    dumpStreamIfExist("[image] in full", rParams.pInFullYuv);
    dumpStreamIfExist("[image] in resized", rParams.pInResizedYuv);
    dumpStreamIfExist("[image] in binning", rParams.pInBinningYuv);
    dumpStreamIfExist("[image] out yuv jpeg", rParams.pOutYuvJpeg);
    dumpStreamIfExist("[image] out yvu thumbnail", rParams.pOutYuvThumbnail);
    dumpStreamIfExist("[image] out yuv 00", rParams.pOutYuv00);
    dumpStreamIfExist("[image] out yuv 01", rParams.pOutYuv01);
    dumpStreamIfExist("[image] out depth", rParams.pOutDepth);
    dumpStreamIfExist("[image] out clean", rParams.pOutClean);

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
TPNodeImp::
getInfoIOMapSet(
    sp<IPipelineFrame> const &pFrame,
    IPipelineFrame::InfoIOMapSet &rIOMapSet) const
{
    if (OK != pFrame->queryInfoIOMapSet(getNodeId(), rIOMapSet)) {
        MY_LOGE("queryInfoIOMap failed");
        return NAME_NOT_FOUND;
    }

    IPipelineFrame::ImageInfoIOMapSet &imageIOMapSet = rIOMapSet.mImageInfoIOMapSet;
    if (!imageIOMapSet.size()) {
        MY_LOGW("no imageIOMap in frame");
        return BAD_VALUE;
    }

    for (size_t i = 0; i < imageIOMapSet.size(); i++) {
        IPipelineFrame::ImageInfoIOMap const &imageIOMap = imageIOMapSet[i];
        if (imageIOMap.vIn.size() == 0) {
            MY_LOGE("[img] #%zu wrong size vIn %zu, vOut %zu",
                    i, imageIOMap.vIn.size(), imageIOMap.vOut.size());
            return BAD_VALUE;
        }
        MY_LOGD_IF(mLogLevel >= 1, "frame %d:[img] %zu, in %zu, out %zu",
                   pFrame->getFrameNo(), i, imageIOMap.vIn.size(), imageIOMap.vOut.size());
    }

    IPipelineFrame::MetaInfoIOMapSet &metaIOMapSet = rIOMapSet.mMetaInfoIOMapSet;
    if (!metaIOMapSet.size()) {
        MY_LOGW("no metaIOMap in frame");
        return BAD_VALUE;
    }

    for (size_t i = 0; i < metaIOMapSet.size(); i++) {
        IPipelineFrame::MetaInfoIOMap const &metaIOMap = metaIOMapSet[i];
        if (!mpInAppMeta.get() ||
            0 > metaIOMap.vIn.indexOfKey(mpInAppMeta->getStreamId())) {
            MY_LOGE("[meta] no in app");
            return BAD_VALUE;
        }
        if (!mpInHalMeta.get() ||
            0 > metaIOMap.vIn.indexOfKey(mpInHalMeta->getStreamId())) {
            MY_LOGE("[meta] no in hal");
            return BAD_VALUE;
        }
        //
        MY_LOGD_IF(mLogLevel >= 2, "frame %d:[meta] %zu: in %zu, out %zu",
                   pFrame->getFrameNo(), i, metaIOMap.vIn.size(), metaIOMap.vOut.size());
    }
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
TPNodeImp::
isMetaStreamAtIOMap(
    IPipelineFrame::InfoIOMapSet const &ioMapSet,
    StreamId_T const streamId
) const
{
    IPipelineFrame::MetaInfoIOMapSet const &metaIOMapSet = ioMapSet.mMetaInfoIOMapSet;

    for (size_t i = 0; i < metaIOMapSet.size(); i++) {
        IPipelineFrame::MetaInfoIOMap const &metaIOMap = metaIOMapSet[i];
        if ( 0 <= metaIOMap.vIn.indexOfKey(streamId) || 0 <= metaIOMap.vOut.indexOfKey(streamId) ) {
            return MTRUE;
        }
    }
    return MFALSE;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
TPNodeImp::
isImageStreamAtIOMap(
    IPipelineFrame::InfoIOMapSet const &ioMapSet,
    StreamId_T const streamId
) const
{
    IPipelineFrame::ImageInfoIOMapSet const &imageIOMapSet = ioMapSet.mImageInfoIOMapSet;

    for (size_t i = 0; i < imageIOMapSet.size(); i++) {
        IPipelineFrame::ImageInfoIOMap const &imageIOMap = imageIOMapSet[i];
        if ( 0 <= imageIOMap.vIn.indexOfKey(streamId) || 0 <= imageIOMap.vOut.indexOfKey(streamId) ) {
            return MTRUE;
        }
    }
    return MFALSE;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
TPNodeImp::
onDequeFrame(android::sp<IPipelineFrame>& rpFrame)
{
    Mutex::Autolock _l(mFrameQueueLock);
    //  Wait until the queue is not empty or this thread will exit
    while (mFrameQueue.empty() && !mbNodeExit) {
        // enable drained flag
        mbFrameDrained = MTRUE;
        mbFrameDrainedCond.signal();

        status_t status = mFrameQueueCond.wait(mFrameQueueLock);
        if (OK != status) {
            MY_LOGW("wait status(%d):%s, mFrameQueue.size(%zu)",
                    status, ::strerror(-status), mFrameQueue.size());
        }
    }

    // warn if request queue is not empty
    if (mbNodeExit) {
        MY_LOGE_IF(!mFrameQueue.empty(), "[flush] mFrameQueue.size(%zu)",
                mFrameQueue.size());
        return DEAD_OBJECT;
    }

    // request queue is not empty, take the first request from the queue
    mbFrameDrained = MFALSE;
    rpFrame = *mFrameQueue.begin();
    mFrameQueue.erase(mFrameQueue.begin());

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
TPNodeImp::
waitUntilDrained() const
{
    Mutex::Autolock _l(mFrameQueueLock);
    if (!mbFrameDrained) {
        MY_LOGD("wait for request drained");
        mbFrameDrainedCond.wait(mFrameQueueLock);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
TPNodeImp::
threadSetting()
{
    //  set thread policy & priority
    //  NOTE:
    //  Even if pthread_create() with SCHED_OTHER policy, a newly-created thread
    //  may inherit the non-SCHED_OTHER policy & priority of the thread creator.
    //  And thus, we must set the expected policy & priority after a thread creation.

    struct sched_param schedParam;
    ::sched_getparam(0, &schedParam);

    if (setpriority(PRIO_PROCESS, 0, TP_THREAD_PRIORITY)) {
        MY_LOGW("set priority failed(%s)", strerror(errno));
    }

    schedParam.sched_priority = 0;
    sched_setscheduler(0, TP_THREAD_POLICY, &schedParam);

    MY_LOGD("tid(%d) policy(%d) priority(%d)",
            gettid(), TP_THREAD_POLICY, TP_THREAD_PRIORITY);

    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
TPNodeImp::
initPlugin(PluginId_T pluginId)
{
    Mutex::Autolock _l(mPluginLock);
    if (mPluginMap.indexOfKey(pluginId) >= 0)
        return OK;

    if (!PluginRegistry::exists(pluginId))
        return BAD_VALUE;

    // create a async job to create the instance if it's not existed
    auto job = std::async([&, pluginId] () {
        if (mPluginMap.indexOfKey(pluginId) < 0) {

            TPNodePlugin* pPlugin =  PluginRegistry::createInstance(pluginId);
            if (pPlugin) {
                pPlugin->init();
                mPluginMap.add(pluginId, pPlugin);
            }
        }
    });

    mPluginJobMap[pluginId] = std::move(job);
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
TPNodePlugin*
TPNodeImp::
getPlugin(PluginId_T pluginId)
{
    Mutex::Autolock _l(mPluginLock);
    if (mPluginMap.indexOfKey(pluginId) < 0) {
        // It'll be creating if not exited in map
        if (mPluginJobMap.count(pluginId) > 0) {
            mPluginJobMap[pluginId].get();
            return mPluginMap.editValueFor(pluginId);
        }
    } else {
        return mPluginMap.editValueFor(pluginId);
    }

    return NULL;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
TPNodeImp::
removePlugins()
{
    Mutex::Autolock _l(mPluginLock);

    for (size_t i = 0; i < mPluginMap.size(); i++) {

        TPNodePlugin* pPlugin = mPluginMap.editValueAt(i);
        pPlugin->uninit();
        delete pPlugin;
    }

    mPluginJobMap.clear();
    mPluginMap.clear();
}

/******************************************************************************
 *
 ******************************************************************************/
void
TPNodeImp::NodeProcessor::
requestExit()
{
    Mutex::Autolock _l(mpNodeImp->mFrameQueueLock);
    mpNodeImp->mbNodeExit = MTRUE;
    mpNodeImp->mFrameQueueCond.signal();
}

/******************************************************************************
 *
 ******************************************************************************/
bool
TPNodeImp::NodeProcessor::
threadLoop()
{
    sp<IPipelineFrame> pFrame;
    if (!exitPending() && OK == mpNodeImp->onDequeFrame(pFrame)) {
        mpNodeImp->onProcessFrame(pFrame);

        return true;
    }
    return false;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
TPNodeImp::CopyProcessor::
threadLoop()
{
    {
        Mutex::Autolock _l(mCopyLock);

        if (!mbCopyExit && mCopyQueue.empty()) {
            mCopyDrainedCond.signal();
            mbCopyDrained = MTRUE;
            mCopyCond.wait(mCopyLock);
        }

        if (mCopyQueue.empty()) {
            mbCopyDrained = MTRUE;
            MY_LOGD("exit tp copy thread");
            return false;
        }

        mCopyQueue.front()();
        mCopyQueue.pop();
    }

    return true;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
TPNodeImp::CopyProcessor::
waitUntilDrained() const
{
    Mutex::Autolock _l(mCopyLock);
    if (!mbCopyDrained) {
        MY_LOGD("wait for request drained");
        mCopyDrainedCond.wait(mCopyLock);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
void
TPNodeImp::CopyProcessor::
requestExit()
{
    Mutex::Autolock _l(mCopyLock);
    mbCopyExit = MTRUE;
    mCopyCond.signal();
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
TPNodeImp::
doPlugin(sp<GroupedFrame> pGrpFrame)
{
    CAM_TRACE_NAME("TP::Plugin");

    const MUINT32 uFrameCount = pGrpFrame->getRequestCount();
    TPNodePlugin::RequestParams params;
    memset(&params, 0, sizeof(params));

    params.uInCount = uFrameCount;
    params.uRequestNo = pGrpFrame->getRequestNo();
    params.fnCallback = &GroupedFrame::onCallback;
    params.pToken = pGrpFrame.get();

    for (size_t i = 0; i < uFrameCount; i++) {
        params.in[i].pFullYuv = pGrpFrame->contains(i, STRM_BUF_IN_FULL) ?
               pGrpFrame->getGroupedBuffer(i, STRM_BUF_IN_FULL)->native() : NULL;

        params.in[i].pResizedYuv = pGrpFrame->contains(i, STRM_BUF_IN_RSZ) ?
               pGrpFrame->getGroupedBuffer(i, STRM_BUF_IN_RSZ)->native() : NULL;

        params.in[i].pBinningYuv = pGrpFrame->contains(i, STRM_BUF_IN_BIN) ?
               pGrpFrame->getGroupedBuffer(i, STRM_BUF_IN_BIN)->native() : NULL;

        params.in[i].pHalMeta = pGrpFrame->contains(i, STRM_META_IN_HAL) ?
               pGrpFrame->getGroupedMetadata(i, STRM_META_IN_HAL)->native() : NULL;

        params.in[i].pAppMeta = pGrpFrame->contains(i, STRM_META_IN_APP) ?
               pGrpFrame->getGroupedMetadata(i, STRM_META_IN_APP)->native() : NULL;

        IMetadata* pHalMeta = params.in[i].pHalMeta;
        if (pHalMeta)
            IMetadata::getEntry<MINT32>(pHalMeta, MTK_STEREO_FEATURE_OPEN_ID, params.in[i].nSensorId);
    }

    params.out.pFullYuv = pGrpFrame->contains(0, TEMP_BUF_FULL00) ?
               pGrpFrame->getGroupedBuffer(0, TEMP_BUF_FULL00)->native() : NULL;

    params.out.pHalMeta = pGrpFrame->contains(0, STRM_META_OUT_HAL) ?
            pGrpFrame->getGroupedMetadata(0, STRM_META_OUT_HAL)->native() : NULL;

    params.out.pAppMeta = pGrpFrame->contains(0, STRM_META_OUT_APP) ?
            pGrpFrame->getGroupedMetadata(0, STRM_META_OUT_APP)->native() : NULL;

    params.out.pDepth = pGrpFrame->contains(0, STRM_BUF_OUT_DEPTH) ?
               pGrpFrame->getGroupedBuffer(0, STRM_BUF_OUT_DEPTH)->native() : NULL;

    params.out.pClean = pGrpFrame->contains(0, TEMP_BUF_FULL01) ?
               pGrpFrame->getGroupedBuffer(0, TEMP_BUF_FULL01)->native() : NULL;

    // get plugin id
    PluginId_T pid = 0;

    TPNodePlugin* pPlugin = NULL;
    IMetadata* pHalMeta = params.in[uFrameCount -1].pHalMeta;
    if (pHalMeta)
        IMetadata::getEntry<MINT32>(pHalMeta, MTK_PLUGIN_MODE, pid);

    if (pid)
        pPlugin = getPlugin(pid);

    if (pPlugin) {
        pPlugin->process(params);
    } else {
        MY_LOGE("plugin(%d) is not existed!", pid);
        pGrpFrame->markState(GROUP_PLUGIN_ERROR);
        pGrpFrame->finished();
    }
}
