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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_PIPELINE_IPIPELINEBUFFERSETFRAMECONTROL_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_PIPELINE_IPIPELINEBUFFERSETFRAMECONTROL_H_
//
#include <list>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>
//
#include <utils/RefBase.h>
#include <utils/StrongPointer.h>
//
#include <mtkcam3/pipeline/stream/IStreamBufferSet.h>
#include <mtkcam3/pipeline/utils/streambuf/StreamBuffers.h>
#include "types.h"
#include "IPipelineNode.h"


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam::v3::pipeline {


/**
 * An interface to the provider of pipeline frame stream buffer.
 */
struct IPipelineFrameStreamBufferProvider
{
public:     ////    Operations.
    virtual         ~IPipelineFrameStreamBufferProvider() = default;

    struct RequestStreamBuffer
    {
        ////////////////////////////////////////////////////////////////////////
        // input arguments

        uint32_t    requestNo = -1;

        uint32_t    frameNo = -1;

        IImageStreamInfo const*
                    streamInfo = nullptr;

    };

    /**
     * Synchronous call for a pipeline frame to ask for an empty stream buffer from the provider.
     *
     */
    virtual auto    requestStreamBuffer(
                        android::sp<IImageStreamBuffer>& rpImageStreamBuffer,
                        RequestStreamBuffer const& arg
                    ) -> int                                                = 0;

};


/**
 * An interface to the processor of pipeline frame Hal result.
 */
struct IPipelineFrameHalResultProcessor
{
public:     ////    Operations.
    virtual         ~IPipelineFrameHalResultProcessor() = default;

    struct ReturnResult
    {
        ////////////////////////////////////////////////////////////////////////
        // input arguments

        uint32_t    requestNo = -1;

        uint32_t    frameNo = -1;

        int64_t     sensorTimestamp = 0;

        bool        isFrameDestroyed = false;

        std::list<android::sp<IImageStreamBuffer>>*
                    pHalImageStreamBuffers = nullptr;

        std::list<android::sp<IMetaStreamBuffer>>*
                    pHalMetaStreamBuffers = nullptr;

        std::list<android::sp<IMetaStreamBuffer>>*
                    pAppMetaStreamBuffers = nullptr;

    };

    /**
     * Synchronous call for a pipeline frame to return the Hal result to the processor.
     *
     */
    virtual auto    returnResult(
                        ReturnResult const& arg
                    ) -> void                                               = 0;

};


/**
 * An interface of pipeline frame node map.
 */
class IPipelineFrameNodeMapControl
    : public virtual android::RefBase
{
public:     ////
    typedef IPipelineFrameNodeMapControl        ThisT;
    typedef IPipelineFrame::NodeId_T            NodeId_T;
    typedef IPipelineFrame::InfoIOMapSet        InfoIOMapSet;
    typedef IStreamInfoSet                      IStreamInfoSetT;
    typedef android::sp<IStreamInfoSetT>        IStreamInfoSetPtr;
    typedef android::sp<IStreamInfoSetT const>  IStreamInfoSetPtr_CONST;

public:     ////                Definitions.
    /**
     *
     */
    struct  INode
        : public virtual android::RefBase
    {
        virtual NodeId_T        getNodeId() const                           = 0;

        virtual IStreamInfoSetPtr_CONST
                                getIStreams() const                         = 0;
        virtual MVOID           setIStreams(IStreamInfoSetPtr p)            = 0;

        virtual IStreamInfoSetPtr_CONST
                                getOStreams() const                         = 0;
        virtual MVOID           setOStreams(IStreamInfoSetPtr p)            = 0;

        virtual InfoIOMapSet const&
                                getInfoIOMapSet() const                     = 0;
        virtual InfoIOMapSet&   editInfoIOMapSet()                          = 0;
    };

public:     ////                Operations.
    static  ThisT*              create();

    virtual ssize_t             setCapacity(size_t size)                    = 0;
    virtual MVOID               clear()                                     = 0;
    virtual ssize_t             addNode(NodeId_T const nodeId)              = 0;

public:     ////                Operations.
    virtual MBOOL               isEmpty() const                             = 0;
    virtual size_t              size() const                                = 0;

    virtual INode*              getNodeFor(NodeId_T const nodeId) const     = 0;
    virtual INode*              getNodeAt(size_t index) const               = 0;

};


/**
 * An interface of pipeline buffer set control.
 */
class IPipelineBufferSetControl
    : public IStreamBufferSet
{
public:     ////                Definitions.
    /**
     * Stream Buffer Map Interface.
     */
    template <class _IStreamBuffer_>
    class IMap
        : public virtual android::RefBase
    {
    public:     ////            Definitions.
        using IStreamBufferT = _IStreamBuffer_;
        using IStreamInfoT = typename IStreamBufferT::IStreamInfoT;

    public:     ////            Operations.
        virtual ssize_t         add(
                                    android::sp<IStreamInfoT> pStreamInfo,
                                    android::sp<IUsersManager> pUsersManager
                                )                                           = 0;

        virtual ssize_t         add(
                                    android::sp<IStreamBufferT> value
                                )                                           = 0;

        virtual ssize_t         setCapacity(size_t size)                    = 0;

        virtual bool            isEmpty() const                             = 0;

        virtual size_t          size() const                                = 0;

        virtual ssize_t         indexOfKey(StreamId_T const key) const      = 0;

        virtual StreamId_T      keyAt(size_t index) const                   = 0;

        virtual IUsersManager*  usersManagerAt(size_t index) const          = 0;

        virtual IStreamInfoT*   streamInfoAt(size_t index) const            = 0;

    };

public:     ////                Operations.
    virtual android::sp<IMap<IImageStreamBuffer> >
                                editMap_AppImage()                          = 0;

    virtual android::sp<IMap<IMetaStreamBuffer> >
                                editMap_AppMeta()                           = 0;

    virtual android::sp<IMap<IImageStreamBuffer> >
                                editMap_HalImage()                          = 0;

    virtual android::sp<IMap<IMetaStreamBuffer> >
                                editMap_HalMeta()                           = 0;

};


/**
 * An interface of (in-flight) pipeline frame control.
 */
class IPipelineBufferSetFrameControl
    : public IPipelineFrame
    , public IPipelineBufferSetControl
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    typedef IPipelineBufferSetFrameControl          ThisT;

    using TrackFrameResultParamsT = NSCam::v3::pipeline::NSPipelineContext::TrackFrameResultParams;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                Callback.
    /**
     *
     */
    class IAppCallback
        : public virtual android::RefBase
    {
    public:

        /*
         * Result structure used in updateFrame(...)
         */
        struct Result
        {
            uint32_t                frameNo;

            /**
             * Definition:
             * https://developer.android.com/reference/android/hardware/camera2/CaptureResult#SENSOR_TIMESTAMP
             *
             * 0 indicates an invalid sensor timestampe (due to not ready or dropping frame)
             * non-zero indicates a valid sensor timestampe
             *
             */
            int64_t                 sensorTimestamp = 0;

            ssize_t                 nAppOutMetaLeft;
            android::Vector<
                android::sp<IMetaStreamBuffer>
                >                   vAppOutMeta;
            ssize_t                 nHalOutMetaLeft;
            android::Vector<
                android::sp<IMetaStreamBuffer>
                >                   vHalOutMeta;
            bool                    bFrameEnd;
            // [physical camerata]
            //     vPhysicalOutMeta will be update in multicam session.
            android::KeyedVector<int,
                                android::sp<IMetaStreamBuffer>>
                                    vPhysicalOutMeta;
            // store physical app metadata stream id.
            std::unordered_map<uint32_t, std::vector<StreamId_T> >
                                            vPhysicalAppStreamIds;
            // [multiple camera extension]
            android::DefaultKeyedVector<NodeId_T,
                std::vector<android::sp<IMetaStreamBuffer> >
                >                   vAppOutMetaByNodeId;
            android::DefaultKeyedVector<NodeId_T,
                std::vector<android::sp<IMetaStreamBuffer> >
                >                   vHalOutMetaByNodeId;
            NodeSet                 vRootNode;
            // Device 3.5: Active Physical ID(Before remapping)
            int32_t                 nPhysicalID = -1;
        };

    public:     ////            Operations.

        /*
         * Invoked when some node update the results.
         *
         * @param[in] requestNo: the request number.
         *
         * @param[in] userId: for debug only, SHOULD NOT be used.
         * This userId cannot be mapped to certain expected result metadata
         * streams. This is left to debug if each user(a.k.a, node) has done
         * callback.
         *
         * @param[in] result: the result metadata, including app/hal meta.
         */
        virtual MVOID           updateFrame(
                                    MUINT32 const requestNo,
                                    MINTPTR const userId,
                                    Result const& result
                                )                                           = 0;


        /**
         * This method is called when some (input or output) App image buffers
         * are released.
         */
        struct ImageBufferReleased
        {
            /**
             * @param App image stream buffers to release.
             */
            std::vector<android::sp<IImageStreamBuffer>>
                                        buffers;

            /**
             * @param The pipeline frame which has fully completed.
             */
            IPipelineFrame const*       frame = nullptr;

        };
        virtual auto    onImageBufferReleased(
                          ImageBufferReleased&& arg __unused
                        ) -> void {}


        /**
         * This method is called when a pipeline frame is being destroyed.
         */
        struct PipelineFrameDestroy
        {
            /**
             * @param The request number.
             */
            uint32_t                    requestNo = 0;

            /**
             * @param The frame number.
             */
            uint32_t                    frameNo = 0;

            /**
             * @param The group frame type.
             */
            GroupFrameTypeT             groupFrameType;

        };
        virtual auto    onPipelineFrameDestroy(
                          PipelineFrameDestroy&& arg __unused
                        ) -> void {}


    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Creation.
                    struct CreationParams
                    {
                        uint32_t                            requestNo = 0;
                        uint32_t                            frameNo = 0;
                        bool                                isReprocessFrame = false;
                        GroupFrameTypeT                     groupFrameType{GroupFrameTypeT::MAIN};
                        android::wp<IAppCallback>           pAppCallback = nullptr;
                        android::wp<IPipelineNodeCallback>  pNodeCallback = nullptr;
                        std::weak_ptr<IPipelineFrameStreamBufferProvider>
                                                            pStreamBufferProvider;
                        std::weak_ptr<IPipelineFrameHalResultProcessor>
                                                            pHalResultProcessor;
                    };
    static  auto    create(CreationParams const& arg) -> ThisT*;

public:     ////                Operations.
    static inline char const*   magicName() { return "IPBSFC";/*IPipelineBufferSetFrameControl*/ }
    static  auto                castFrom(IPipelineFrame* pPipelineFrame) -> IPipelineBufferSetFrameControl*;

public:     ////                Configuration.
    virtual MERROR              startConfiguration()                        = 0;
    virtual MERROR              finishConfiguration()                       = 0;

    virtual MERROR              setNodeMap(
                                    android::sp<IPipelineFrameNodeMapControl> value
                                )                                           = 0;

    virtual MERROR              setPipelineNodeMap(
                                    android::sp<IPipelineNodeMap const> value
                                )                                           = 0;

    virtual MERROR              setPipelineDAG(
                                    android::sp<IPipelineDAG> value
                                )                                           = 0;

    virtual MERROR              setStreamInfoSet(
                                    android::sp<IStreamInfoSet const> value
                                )                                           = 0;

    virtual auto    setTrackFrameResultParams(
                        std::shared_ptr<TrackFrameResultParamsT const>const& arg
                    ) -> void                                               = 0;

    virtual auto    getTrackFrameResultParams() const
                        -> std::shared_ptr<TrackFrameResultParamsT const>   = 0;

    virtual auto    setUnexpectedToAbort(bool enabled) -> void              = 0;

    /**
     * Configure which information will be kept in this frame.
     *
     * @param[in] keepTimestamp:
     *  true indicates the timestamp is kept in this frame when it's ready.
     *
     * @param[in] keepHalImage:
     *  ture indicates all Hal Image stream buffers will be kept in this frame when they're ready
     *  (pending release).
     *
     * @param[in] keepHalMeta:
     *  ture indicates all Hal Meta stream buffers will be kept in this frame when they're ready.
     *
     * @param[in] keepAppMeta:
     *  ture indicates all App Meta output stream buffers will be kept in this frame when they're ready.
     *  Notes: App Meta input stream buffers won't be kept.
     *
     * @param[in] targetToKeep_HalImage
     *  If keepHalImage == false, this argument is ignored.
     *  If keepHalImage == true, this argument indicates a set of target Hal Image stream id to keep.
     *  If keepHalImage == true and this target set is empty, then all Hal Image stream buffers will be kept.
     *
     * @param[in] targetToKeep_HalMeta
     *  If keepHalMeta == false, this argument is ignored.
     *  If keepHalMeta == true, this argument indicates a set of target Hal Meta stream id to keep.
     *  If keepHalMeta == true and this target set is empty, then all Hal Meta stream buffers will be kept.
     *
     */
    struct ConfigureInformationKeepingParams
    {
        bool                    keepTimestamp   = false;
        bool                    keepHalImage    = false;
        bool                    keepHalMeta     = false;
        bool                    keepAppMeta     = false;
        std::set<StreamId_T>    targetToKeep_HalImage;
        std::set<StreamId_T>    targetToKeep_HalMeta;
    };
    virtual auto    configureInformationKeeping(
                    ConfigureInformationKeepingParams const& arg
                    ) -> void                                               = 0;

public:     ////    Operations.
    virtual auto    transferPendingReleaseBuffers(
                        android::Vector<android::sp<IImageStreamBuffer>>& out
                    ) -> void                                               = 0;

    virtual auto    transferPendingReleaseBuffers(
                        android::Vector<android::sp<IMetaStreamBuffer>>& out
                    ) -> void                                               = 0;

    // update physical app metadata stream id list.
    virtual auto    setPhysicalAppMetaStreamIds(
                        std::unordered_map<uint32_t, std::vector<StreamId_T> > const&
                                                physicalAppMetaStreamIds
                    ) -> MERROR                                               = 0;

    /**
     * Try to get the sensor timestamp.
     *
     * Definition:
     * https://developer.android.com/reference/android/hardware/camera2/CaptureResult#SENSOR_TIMESTAMP
     *
     * This return value is valid only if the sensor timestampe is ready and
     * it is enabled via configureInformationKeeping() with keepTimestamp=true.
     */
    virtual auto    tryGetSensorTimestamp() const -> int64_t                = 0;

public:     ////    Operations.
    virtual auto    abort() -> void                                         = 0;

public:     ////                Debugging.
    virtual MVOID               logDebugInfo(std::string&& str)             = 0;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSCam::v3::pipeline
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_PIPELINE_IPIPELINEBUFFERSETFRAMECONTROL_H_

