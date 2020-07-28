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
 * MediaTek Inc. (C) 2015. All rights reserved.
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
#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_MFCNODEIMP_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_MFCNODEIMP_H_

#include <utils/String8.h>
#include <utils/RWLock.h>
#include <utils/Mutex.h>
#include <utils/Condition.h>

#include <mtkcam/pipeline/stream/IStreamBuffer.h>
#include <mtkcam/pipeline/utils/streambuf/IStreamBufferPool.h>
#include <mtkcam/aaa/IHal3A.h>
#include <mtkcam/drv/iopipe/Port.h> // NSCam::NSIoPipe::PortID

#include <mtkcam/pipeline/hwnode/MFCNode.h>

#include "BaseNode.h"

/* STL */
#include <memory>
#include <functional>
#include <future>
#include <string>
#include <map>
#include <mutex>
#include <condition_variable>

/**
 *  The confidence threshold of GMV, which means if the confidence is not
 *  enough then the GMV won't be applied.
 */
#define MFC_GMV_CONFX_TH 25
#define MFC_GMV_CONFY_TH 25

using namespace android;
using namespace std;
using NSCam::NSIoPipe::PortID;
using NS3Av3::CaptureParam_T;

//-----------------------------------------------------------------------------
namespace NSCam {
namespace v3 {

//-----------------------------------------------------------------------------
// MFCNodeImp
//-----------------------------------------------------------------------------
/**
 *  Multi-Frame Capture Node Implementation (MFCNodeImp) is a super class for the
 *  implementations of HDRN and MFNR hence lots of common parts can be re-used
 *  for multi-frame capture node.
 *
 *  Derived classes are supposed to be necessary to re-implement some methods and
 *  invoke MFCNodeImp::method to use the common part benifits (or says it MUST be).
 *
 *  If derived class has no need to do anything for it, there's no need to re-Implement
 *  the methods.
 *
 *  e.g.:
 *  DerivedClass : public MFCNodeImp {
 *  public:
 *      virtual MERROR init(InitParams const &rParams)
 *      {
 *          MERROR err = MFCNodeImp::init();
 *          // do somthing here
 *          ...
 *          return err;
 *      }
 *  }
 */
class MFCNodeImp;

//-----------------------------------------------------------------------------
// CQueue
// @brief A thread-safe queue implementation
// @note  CQueue::deque will invoke twice copy constructors due to make thread-safe,
//        however, to avoid wasting time of copying elements CQueue provides a
//        method named "deque_by_move" to use std::move to move element.
//-----------------------------------------------------------------------------
template<class T = void*>
class CQueue {
/* std::chrono::seconds: duration, signed integer type of at least 35 bits */
#define TIMEOUT_INFINITE 0x7FFFFFFF
public:
    CQueue(std::function<T(void)> default_constructor = [](void)->T{
        return T();
    })
    {
        m_constructor = default_constructor;
    };

public:
    T deque(const int &timeoutsec = TIMEOUT_INFINITE)
    {
        std::unique_lock<std::mutex> _l(m_mutex);
        if (m_queue.size() <= 0) {
            auto s = m_condition.wait_for(_l, std::chrono::seconds(timeoutsec));
            if (s == std::cv_status::timeout)  {
                return m_constructor();
            }
        }
        T r = m_queue.front();
        m_queue.erase(m_queue.begin());
        return r;
    };

    T deque_by_move(const int &timeoutsec = TIMEOUT_INFINITE)
    {
        std::unique_lock<std::mutex> _l(m_mutex);
        if (m_queue.size() <= 0) {
            auto s = m_condition.wait_for(_l, std::chrono::seconds(timeoutsec));
            if (s == std::cv_status::timeout)
                return m_constructor();
        }
        T r = std::move(m_queue.front());
        m_queue.erase(m_queue.begin());
        return std::move(r);
    };

    void enque(const T &e)
    {
        std::unique_lock<std::mutex> _l(m_mutex);
        m_queue.push_back(e);
        m_condition.notify_one();
    };

    void push_back(const T &e)
    {
        enque(e);
    };

    void clear()
    {
        std::unique_lock<std::mutex> _l(m_mutex);
        m_queue.clear();
    };

    size_t size()
    {
        std::unique_lock<std::mutex> _l(m_mutex);
        return m_queue.size();
    };

    void iterative_do(std::function<void(T& element, void*)> func, void *arg)
    {
        std::unique_lock<std::mutex> _l(m_mutex);
        for (size_t i = 0; i < m_queue.size(); i++)
            func(m_queue[i], arg);
    };

    T& at(size_t index)
    {
        std::unique_lock<std::mutex> _l(m_mutex);
        return m_queue[index];
    };

    T& operator[](size_t index)
    {
        return this->at(index);
    };

    /* advance methods */
    std::mutex& mutex()
    {
        return m_mutex;
    };

    std::vector<T>& queue()
    {
        return m_queue;
    };

private:
    std::mutex  m_mutex;
    std::condition_variable m_condition;
    std::vector<T> m_queue;
    std::function<T(void)> m_constructor;
};

//-----------------------------------------------------------------------------
// CRequestFrame: A wrapper of IPipelineFrame, provides easy ways to retrieve,
// use and release IImageBuffers and IMetadata in an IPipelineFrame
//-----------------------------------------------------------------------------
class CRequestFrame final
{
public:
    /* Image buffers we care about */
    typedef enum {
        eRequestImg_FullRaw = 0, // full size RAW
        eRequestImg_ResizedRaw, // resized raw,
        eRequestImg_LcsoRaw, // LCS output histogram buffer
        eRequestImg_YuvJpeg, // output YUV buffer for JPEG node
        eRequestImg_YuvThumbnail, // output YUV thumbnail buffer for JPEG node
        eRequestImg_YuvPostview, // output YUV postview buffer (if exists)
        // all & size
        eRequestImg_All,
        eRequestImg_Size = eRequestImg_All,
    } eRequestImg;

    /* Metadata we care about */
    typedef enum {
        eRequestMeta_InAppRequest = 0,
        eRequestMeta_InHalP1,
        eRequestMeta_InFullRaw,
        eRequestMeta_InResizedRaw,
        eRequestMeta_OutAppResult,
        eRequestMeta_OutHalResult,
        // all & size
        eRequestMeta_All,
        eRequestMeta_Size = eRequestMeta_All,
    } eRequestMeta;

    /**
     *  Base class for a Image frame of a IPipelineFrame, this means that there're
     *  several FrameImage objects within a IPipelineFrame.
     */
    class FrameImage final{
    public:
        std::string             callerName; // saves MFCNodeImp::getNodeName()
        StreamId_T              streamId;
        MUINT32                 nodeId; // saves MFCNodeImp::getNodeId()
        sp<IImageBuffer>        imageBuffer;
        sp<IImageStreamBuffer>  streamBuffer;
        sp<IImageBufferHeap>    imageBufferHeap;
        sp<IPipelineFrame>      pipelineFrame; // the original IPipelineFrame
        Mutex                   mx;
        bool                    bError;
        /* StreamId_T linked to CRequestFrame::m_streamIdImages */
        std::map<eRequestImg, StreamId_T>& STREAMIDs; // matching stream id
    public:
        FrameImage(std::map<eRequestImg, StreamId_T> &STREAMIDs);
        ~FrameImage();
    };

    /* For metadata */
    class FrameMetadata final {
    public:
        std::string             callerName;
        StreamId_T              streamId;
        MUINT32                 nodeId;
        IMetadata               *pMetadata;
        sp<IMetaStreamBuffer>   streamBuffer;
        sp<IPipelineFrame>      pipelineFrame; // reference
        Mutex                   mx;
        bool                    bError;
        /* StreamId_T linked to CRequestFrame::m_streamIdMetadatas */
        std::map<eRequestMeta, StreamId_T>& STREAMIDs;
    public:
        FrameMetadata(std::map<eRequestMeta, StreamId_T> &STREAMIDs);
        ~FrameMetadata();
    };

public:
    /**
     *  Create a CRequestFrame
     *  @param caller                       Caller should be MFCNodeImp
     *  @param f                            IPipelineFrame to parse
     *  @param imageStreamInfoProvider      A stream id set for matching by FrameImage
     *  @param metadataStreamInfoProvider   A stream id set for matching by FrameMetadata
     *  @param index                        Debug info, priniting log with this index
     *  @note   imageStreamInfoProvider and metadataStreamInfoProvider is a std::function
     *          which should returns a std::map object contains stream ID that
     *          FrameImage & FrameMetadata will reference. Notice that, CRequestFrame
     *          invokes std::move to move the elements from providers.
     */
    CRequestFrame(
            MFCNodeImp *caller,
            sp<IPipelineFrame> &f,
            std::function<std::map<eRequestImg, StreamId_T>(void)> imageStreamInfoProvider,
            std::function<std::map<eRequestMeta, StreamId_T>(void)> metadataStreamInfoProvider,
            int index = -1 // not defined
    );
    ~CRequestFrame();

/* INTERFACEs */
public:
    /* To retrieve IImageBuffer pointer, it may be NULL if not exists */
    IImageBuffer*   imageBuffer(const eRequestImg &req);
    /* To retrieve IMetadata pointer, it may be NULL if not exists */
    IMetadata*      metadata(const eRequestMeta &req);
    /* To retrieve IPipelineFrame pointer, it may be NULL if not exists */
    sp<IPipelineFrame>& pipelineFrame();
    int             markError(const eRequestImg &req); // mark the buffer as error buffer
    int             releaseImage(const eRequestImg &req); // release FrameImage partially
    int             releaseMetadata(const eRequestMeta &req); // release FrameMetadata partially
    /**
     *  To check CRequestFrame status
     *  @return         If returns not 0, it means the frame has errors and can
     *                  not be used, should be dropped
     */
    int             status();

    /**
     *  To make sure parsing IPipelineFrame is on another thread due to it may
     *  happend deadlock if using the same thread
     */
    std::future<void> m_processFrame;

    //
    // inline functions
    //
    inline int      index() const { return m_index; }

public:
    /* a helper function to invoke BaseNode::flush to flush a IPipelineFrame in CRequestFrame */
    static void flush(std::shared_ptr<CRequestFrame> f, BaseNode *node)
    {
        class BaseNodeWrapper : public BaseNode {
        public:
            MERROR flush(sp<IPipelineFrame> const &pFrame) { return BaseNode::flush(pFrame); }
        } *nodeWrapper = reinterpret_cast<BaseNodeWrapper*>(node);
        if (f.get() == NULL || node == NULL)
            return;
        nodeWrapper->flush(f->pipelineFrame());
    };
    /* a helper function to invoke BaseNode::flush to flush a CRequestFrame in CQueue */
    static void flushClearAll(CQueue<std::shared_ptr<CRequestFrame>> &queue, BaseNode *node)
    {
        std::unique_lock<std::mutex> _l(queue.mutex());
        while (!queue.queue().empty()) {
            CRequestFrame::flush(queue.queue().front(), node);
            queue.queue().erase(queue.queue().begin());
        }
    };

/* ATTRIBUTEs */
private:
    MFCNodeImp *m_node; // reference of caller
    sp<IPipelineFrame> m_pipelineFrame;  // holds IPipelineFrame
    /* frame status represents if this frame can be used or not. 0 for OK */
    int m_frameStatus;
    int m_index; // information for debug

    /* stream Id for representation the stream IDs node is using*/
    std::map<eRequestImg, StreamId_T>                       m_streamIdImages;
    std::map<eRequestMeta, StreamId_T>                      m_streamIdMetadatas;
    /* image */
    std::map<eRequestImg, std::shared_ptr<FrameImage>>      m_frameImages;
    CQueue<std::shared_ptr<FrameImage>>                     m_frameImageOthers;
    /* metadata */
    std::map<eRequestMeta, std::shared_ptr<FrameMetadata>>  m_frameMetadatas;
    CQueue<std::shared_ptr<FrameMetadata>>                  m_frameMetadataOthers;
};

//-----------------------------------------------------------------------------
// MFCNodeImp
//-----------------------------------------------------------------------------
class MFCNodeImp
    : public MFCNode    /* Implement this */
    , public BaseNode   /* Using common parts */
{
protected:
    MFCNodeImp() : m_index(0) {};
    virtual ~MFCNodeImp();

public:
    /* deque a CRequestFrame */
    virtual std::shared_ptr<CRequestFrame> deque(const int &timeoutsec = TIMEOUT_INFINITE);

/**
 *  Attributes which may be inherited
 */
protected:
    /* Node topology info */
    sp<IMetaStreamInfo>     mpInAppMeta_Request;
    sp<IMetaStreamInfo>     mpInHalMeta_P1;
    sp<IMetaStreamInfo>     mpOutAppMeta_Result;
    sp<IMetaStreamInfo>     mpOutHalMeta_Result;
    sp<IImageStreamInfo>    mpInFullRaw;
    sp<IImageStreamInfo>    mpInResizedRaw;
    sp<IImageStreamInfo>    mpInLcsoRaw;
    ImageStreamInfoSetT     mvOutImages; // output image port
    /* Request frames queue*/
    CQueue<std::shared_ptr<CRequestFrame>> mRequestFrameQueue;
    /* Saves main IPipelineFrame (the first IPipelineFrame) */
    sp<IPipelineFrame>              mPipelineFrame;
    int m_index; // for debug, represent the index of frame enqued

/* IPipelineNode interfaces, MFCNodeImp Implementations, common parts. */
public:
    /**
     *  Derived class should set InitParams::nodeId...etc to BaseNode::mNodeId.
     *
     *  @params rParams     - IPipelineFrame::InitParams for initialization.
     *  @return             - Returns OK if no error.
     */
    virtual MERROR init(InitParams const& rParams);

    /**
     *  Middleware invokes this method while the node is going to be released.
     *  To release resource which is no need here.
     *
     *  @return             - Returns OK if no error.
     */
    virtual MERROR uninit(void);

    /**
     *  This is not interface which defined in IPipelineNode but PipelineContex
     *  which will be invoked while the node is being configured.
     *
     *  @param rParams      - MFCNode::ConfigParam for configuring node.
     *  @return             - Returns OK if no error.
     */
    virtual MERROR config(const ConfigParams& rParams);

    /**
     *  While an IPipelineFrame is queued, this mthod will be invoked.
     *  @param pFrame       - IPipelineFrame instance.
     *  @notice             - Middleware invokes IPipelineNode::queue with synchronized
     *                        call which means that you shouldn't block or execute
     *                        complicated operations here or performance will goes low.
     */
    virtual MERROR queue(sp<IPipelineFrame> pFrame);

    /**
     *  Middleware will invoke IPipelineNode::flush to flush all frames.
     *  Node has responsibility to invoke Base::flush to flush all queued
     *  CRequestFrame instances.
     *
     *  @return             - Returns OK if no error.
     */
    virtual MERROR flush(void); // Implement for IPipelineFrame
    /**
     *  To be determined.
     *
     *  @return             - Returns OK if no error.
     */
    virtual MERROR flush(sp<IPipelineFrame> const &pFrame); // Implement for IPipelineFrame

    /**
     *  When everything is ok and it's time to pass frames to the next node,
     *  invoke this method.
     */
    virtual MVOID onDispatchFrame();

    /**
     *  To be determined.
     *
     *  @return             - Returns OK if no error.
     */
	virtual MVOID onDispatchFrame(sp<IPipelineFrame> const& pFrame);

    /**
     *  Release all resoures, this method will be invoked while invoking onDispatchFrame,
     *  hence please make sure derived class has released all CRequestFrame that
     *  makes onDispatchFrame successfully
     */
    virtual MVOID releaseResource(void);

/**
 *  Useful utils
 */
public:
    /**
     *  This is a helper function that print out stream information
     *
     *  @param str              - Prefix string of log.
     *  @param streamInfo       - IStreamInfo object that to dump
     */
    static void dumpStreamIfExist(const char* str, const sp<IStreamInfo>& streamInfo);

    /**
     *  This is a helper function that print out information of NS3Av3::CaptureParam_T
     *
     *  @param captureParam     - Parameter to be dumped.
     *  @param msg              - const char* to a prfix message, can be NULL.
     */
    static void dumpCaptureParam(const CaptureParam_T &captureParam, const char* msg = NULL);

    /**
     *  This is a helper function to print out the exposure/iso using CAM_LOGD
     *  @param halMeta          - P1 hal metadata contains 3A exifs
     */
    static void dumpExifInfo(const IMetadata &halMeta);

    /**
     *  A helper function to retrieve string of image format. E.g.: return ".yuy2"
     *
     *  @param f                - eImgFmt enumeration to query
     *  @return                 - Address of const char*.
     */
    static const char* getFileFormatName(const MINT &f);

    /**
     *  To verify the MFCNode::ConfigParams is valid or not. This method will be
     *  invoked in MFCNodeImp::config which means that when PipelineContext is
     *  configuring MFCNode, this method will be used as checking if the configuration
     *  is ok or not. Derived class maybe have to re-implement this method for
     *  it's proposal.
     *
     *  @param rParams          - Parameter to check.
     *  @return                 - Returns OK if no error.
     *  @note                   - Derived class maybe have to re-implement this method
     *                            for it's proposal.
     */
    static MERROR verifyConfigParams(const ConfigParams &rParams);

    /**
     *  A helper function to generate output YUV and thumbnail YUV using
     *  1. No NR apply: IImageTransform
     *  2. MNR:         pass 2 driver
     *  3. SWNR:        TODO: not finished yet
     *
     *  @param sensorId         - Sensor ID now using
     *  @param imgSrc           - IImageBuffer contains source image (should be full raw size)
     *  @param imgDst1          - YUV JPEG buffer should be the first destination
     *  @param imgDst2          - YUV thumbnail should be the second destination.
     *                            This argument can be NULL
     *  @param crop1            - A cropping region of source for destination 1
     *  @param crop2            - A cropping region of source for destination 2
     *  @param orientation1     - The orientation for destination 1, it should be
     *                            0, eTransform_ROT_90, eTransform_ROT_180 or
     *                            eTransform_ROT_270
     *  @param metaApp          - IMetadata instance of the source frame (App)
     *  @param metaHal          - IMetadata instance of the source frame (Hal)
     *  @param nrType           - No NR apply, set this to 0. TODO: To use global enumeration to
     *                            describe HWNR/SWNR enumeration
     *  @param isMfll           - For SWNR, the tuning parameter is different between
     *                            normal usage and MFLL.
     */
    static MERROR generateOutputYuv(
            int             sensorId,
            IImageBuffer    *imgSrc,
            IImageBuffer    *imgDst1,
            IImageBuffer    *imgDst2,
            const MRect     &crop1,
            const MRect     &crop2,
            int             orientation1,
            IMetadata       *metaApp,
            IMetadata       *metaHal,
            int             isMfll = 0  // Mfll set to 0
            );

    static MINT32 nrStrategy(MINT32 iso, IMetadata *metaHal);

/* inline & template functions */
public:
    /**
     *  function to retrieve the region with zoom in
     *  @param pMetadata        IMetadata contains scaler crop region info
     *  @param [out] outRgn     Retrieved region base on the size of full size RAW
     *  @return                 If ok retruns MTRUE
     */
    static MBOOL retrieveScalerCropRgn(IMetadata* pMetadata, MRect& outRgn);

    /**
     *  function to retrieve GMV from IMetadata of HAL
     *  @param pMetadata        IMetadata
     *  @param [out] x          Retrieved value of x
     *  @param [out] y          Retrieved value of y
     *  @param [out] size       The resized image resolution that for GMV calculating
     *  @return                 If ok returns MTRUE
     *  @note                   The unit of GMV is based on the image resolution
     *                          from size, you need to map the unit from resized
     *                          domain to your domain
     */
    static MBOOL retrieveGmvInfo(IMetadata *pMetadata, int &x, int &y, MSize &size);

    /**
     *  Function to calculate ROI based on source rect and destination rect.
     *  The rule that won't make distortion between source and destination rects,
     *  but a cropping will be applied.
     *
     *  e.g.:
     *
     *  <--- src width -->
     *  +----------------+   ^
     *  |                |   |
     *  |================|   |            ^
     *  |      ROI       | src height     |  dst height
     *  |================|   |            V
     *  |                |   |
     *  +----------------+   V
     *  <--- dst width -->
     *
     *  and the return rectangle will be with a offset (x,y) and resolution of
     *  this ROI.
     *
     *  @param rSrc             Rectangle describes size of source rect
     *  @param rDst             Rectangle describes size of destination rect
     *  @return                 ROI
     */
    static MRect calCrop(MRect const &rSrc, MRect const &rDst);

}; /* class MFCNodeImp */
}; /* namespace v3 */
}; /* namespace NSCam */
#endif/*_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_MFCNODEIMP_H_*/
