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

#ifndef _MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_ZSL_INCLUDE_IMPL_ZSLPROCESSOR_H_
#define _MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_ZSL_INCLUDE_IMPL_ZSLPROCESSOR_H_

#include <chrono>
#include <deque>
#include <map>
#include <vector>

#include <utils/Timers.h>
#include "impl/IZslProcessor.h"
#include "impl/IHistoryBufferContainer.h"
#include "mtkcam3/pipeline/pipeline/IPipelineContext.h"
#include "mtkcam3/pipeline/pipeline/IFrameBufferManager.h"
#include "mtkcam3/pipeline/utils/streambuf/StreamBuffers.h"
#include "mtkcam3/pipeline/model/types.h"
#include "mtkcam/utils/metadata/IMetadata.h"
#include <mtkcam/utils/debug/debug.h>


/******************************************************************************
 *
 ******************************************************************************/
using namespace android;
using namespace std;
using namespace NSCam;
using namespace NSCam::v3::pipeline::NSPipelineContext;
using namespace NSCam::v3::Utils;
using namespace NSCam::v3::pipeline::policy;


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace model {


class ZslProcessor
    : public IZslProcessor
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    typedef Utils::HalMetaStreamBuffer::Allocator
                                    HalMetaStreamBufferAllocatorT;
    typedef IHistoryBufferContainer::BufferSet_T  BufferSet_T;
    typedef IHistoryBufferContainer::MetaSet_T    MetaSet_T;
    typedef IHistoryBufferContainer::ImageSet_T   ImageSet_T;

    using TrackFrameResultParams = NSCam::v3::pipeline::NSPipelineContext::TrackFrameResultParams;

public:
    struct  MyDebuggee : public IDebuggee
    {
        static const std::string            mName;
        std::shared_ptr<IDebuggeeCookie>    mCookie = nullptr;
        android::wp<ZslProcessor>           mZslProcessor = nullptr;

                        MyDebuggee(ZslProcessor* p) : mZslProcessor(p) {}
        virtual auto    debuggeeName() const -> std::string { return mName; }
        virtual auto    debug(
                            android::Printer& printer,
                            const std::vector<std::string>& options
                        ) -> void override;
    };

protected:
    struct SelectResult_T {
        std::vector<BufferSet_T> vZSLBufSets;
        std::vector<bool>        vUseZslBuffers;
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Instantiation data (initialized at the creation stage).
    const int32_t                           mOpenId;
    const std::string                       mUserName;
    const int32_t                           mLogLevel;
    //
    std::shared_ptr<MyDebuggee>             mDebuggee = nullptr;

protected:  ////    Configuration data (initialized at the configuration stage).
    std::vector<ParsedStreamInfo_P1>        mvParsedStreamInfo_P1;
    android::wp<IPipelineModelCallback>     mPipelineModelCallback;
    uint32_t                                mMaxRequestSize;
    //
    struct Meta_DynamicP1 {
        std::unordered_set<StreamId_T>      vApp;
        std::unordered_set<StreamId_T>      vHal;
    }                                       mMetaDynamicP1;
    std::shared_ptr<TrackFrameResultParams> mDefaultTrackedParams;


protected:  ////    Request data.
    std::deque<uint32_t>                    mvInflightZslRequestNo;

    struct ZslPendingParams
    {
        ZslRequestParams                    params;
        std::chrono::system_clock::time_point
                                            timePoint;
    };
    std::deque<ZslPendingParams>            mvPendingRequest;
    std::map<uint32_t, int64_t>             mvFakeShutter;  // key: reqNo, value: fakeShutter
    int64_t                                 mFakeShutterNs; // timestamp to cb as fakeshutter (for pending req)

    mutable Mutex                           mLock;
    int64_t                                 mZSLTimeDiffMs;
    const int32_t                           mZSLForcePolicy;
    const int64_t                           mZSLForceTimeOutMs;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:

    auto    print(
                android::Printer& printer,
                ZslRequestParams const& o
            ) -> void;

    auto    print(
                android::Printer& printer,
                IFrameBuilder const& o
            ) -> void;

    auto    callbackFakeShutter(uint32_t const reqNo) -> void;

    auto    buildFrame_Locked(
                ZslRequestParams const& rZslReqParam,  // [in]
                SelectResult_T& rSelectResult          // [in]
            ) -> FrameBuildersOfOneRequest;

    auto    prepareHalImg(
                const ImageSet_T& vImageSet,                          // [in]
                vector<sp<IImageStreamBuffer>>& vpHalImg              // [out]
            ) -> void;

    auto    prepareMeta(
                const vector<sp<IMetaStreamBuffer>>& vpAppMeta,       // [in]
                const vector<sp<IMetaStreamBuffer>>& vpHalMeta,       // [in]
                const MetaSet_T& vMetaSet,                            // [in]
                vector<sp<IMetaStreamBuffer>>& vpAppMetaZsl,          // [out]
                vector<sp<IMetaStreamBuffer>>& vpHalMetaZsl           // [out]
            ) -> void;

    auto    appendToP1OutAppMeta(
                const vector<sp<IMetaStreamBuffer>>& vpAppMeta,       // [in]
                sp<IMetaStreamBuffer>& pMetaBuffer                    // [in/out]
            ) -> void;

    auto    appendToP1OutHalMeta(
                android::sp<IMetaStreamBuffer>& pStreamBuffer,        // [in]
                const vector<sp<IMetaStreamBuffer>>& vpHalMeta,       // [in]
                sp<IMetaStreamBuffer>& pMetaBuffer                    // [out]
            ) -> void;

    auto    processZslRequest(
                ZslRequestParams const& procIn,
                std::chrono::system_clock::time_point const& procReqTime
            ) -> FrameBuildersOfOneRequest;

    auto    selectBuf_Locked(
                ZslRequestParams const& rZslReqParam,                  // [in]
                const chrono::system_clock::time_point& ZslReqTime,    // [in]
                SelectResult_T& rSelectResult                          // [out]
            ) -> int32_t;

    auto    updateStartIndexForZSD(
                const Vector<BufferSet_T>& vHBC_Buf_Sets,               // [in]
                const chrono::system_clock::time_point& ZslReqTime,     // [in]
                int64_t ReqTimeStampDiff,                               // [in]
                size_t& TimeSelBuf                                      // [out]
            ) -> void;

    auto    checkQuality(
                const MetaSet_T& rvResult,                              // [in]
                const int32_t ZslPolicy,                                // [in]
                bool& rbKeep                                            // [out]
            ) -> bool;

    auto    getRootsStreamsForZoom(
                NodeSet const& roots,
                std::unordered_map<NodeId_T, IOMapSet> const& table
            ) -> std::unordered_set<StreamId_T>;

    bool isAfOkBuffer(const MetaSet_T& rvResult);
    bool isAeOkBuffer(const MetaSet_T& rvResult);
    bool isFameSyncOkBuffer(const MetaSet_T& rvResult);
    bool isPDProcessRawBuffer(std::vector<android::sp<IMetaStreamBuffer>>const& rvResult);
    int64_t getTimeStamp(const MetaSet_T& rvResult);

    bool isAppMeta(StreamId_T const& streamId);
    bool isHalMeta(StreamId_T const& streamId);
    bool hasExceedMaxRequest();

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  Operations.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   public:  ////    Instantiation.
                    ZslProcessor(int32_t openId, std::string const& name);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IDebuggee Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Interfaces.

    virtual auto    debug(
                        android::Printer& printer,
                        const std::vector<std::string>& options
                    ) -> void;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  RefBase Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Interfaces.

    virtual void    onFirstRef();
    virtual void    onLastStrongRef(const void* id);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IZslProcessor Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Interfaces.

    virtual auto    configure(ZslConfigParams const& in) -> int override;

    /**
     * The IZslProcessor is in charge of evaluating zsl/non-zsl flow might be applied to this input
     * params from User(application) and Capture Setting Policy module.
     *
     * @param[in] in:
     *  Callers must promise its content. The callee is not allowed to modify it.
     *
     * @param[out] out:
     *  On this call, the callee must allocate and set up its content.
     *
     * @return
     *      OK indicates success; otherwise failure.
     */
    virtual auto    submitZslRequest(
                        ZslRequestParams&& in
                    ) -> std::vector<FrameBuildersOfOneRequest> override;


    virtual auto    processZslPendingRequest(uint32_t const requestNo)
                        -> std::vector<FrameBuildersOfOneRequest> override;


    /**
     * The IZslProcessor has pending zsl capture request. User should submitZslRequest even
     * though there exists no zsl capture request in current reqeust.
     *
     * @param[in] in:
     *
     * @param[out] out:
     *
     * @return
     *      true indicates has pending request;
     */
    virtual auto    hasZslPendingRequest() const -> bool override;


    /**
     * The IZslProcessor is in charge of update configured metadata to HistoryBufferContainer.
     *
     * @param[in] in:
     *  Callers must promise its content. The callee is not allowed to modify it.
     *
     * @param[out] out:
     *  On this call, the callee must allocate and set up its content.
     *
     * @return
     *
     */
    virtual auto    onFrameUpdated(
                        uint32_t const requestNo,
                        IPipelineBufferSetFrameControl::IAppCallback::Result const& result
                    ) -> void override;

    /**
     * The IZslProcessor is in charge of pipeline flush and mark error for StreamBuffer
     *
     * @param[in] in:
     *
     * @param[out] out:
     *
     * @return
     *
     */
    virtual auto    flush() -> std::vector<FrameBuildersOfOneRequest> override;


    /**
     * register specific StreamIds to keep as history buffers.
     *
     * registerTrackFrameParams:
     *
     * register specific StreamIds to keep in FBM by frameNo.
     * This should be called right after buildPipelineFrame(), where frameNo is generated.
     *
     * @param[in] pPipelineFrame: to setTrackFrameResultParams.
     *
     * @param[in] pReqResult: if not nullptr: getRootsStreamsForZoom ; else use mDefaultTrackedParams.
     */
    virtual auto    registerTrackFrameParams(
                        android::sp<IPipelineFrame>const& pPipelineFrame,
                        policy::pipelinesetting::RequestResultParams const* pReqResult
                    ) -> void override;
};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace model
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_ZSL_INCLUDE_IMPL_ZSLPROCESSOR_H_

