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

#ifndef _MTK_HARDWARE_MTKCAM3_PIPELINE_PIPELINE_INITFRAMEHANDLER_H_
#define _MTK_HARDWARE_MTKCAM3_PIPELINE_PIPELINE_INITFRAMEHANDLER_H_
//
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <thread>
#include <vector>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam::v3::pipeline::NSPipelineContext {


/******************************************************************************
 *
 ******************************************************************************/
class PipelineContext::PipelineContextImpl::InitFrameHandler
{
public:     ////
    using ThisT = InitFrameHandler;
    using InitFrameQueueT = std::vector<android::sp<IPipelineFrame>>;
    using PipelineContextT = PipelineContextImpl;

    static constexpr int64_t kWaitNextInitFrame_EngBuild = 1000000000; // 1000 ms
    static constexpr int64_t kWaitDrainedDuration_EngBuild = 1000000000; // 1000 ms
    static constexpr int64_t kWaitDrainedTimeoutAfterConfig_EngBuild = INT64_MAX; // no timeout

    static constexpr int64_t kWaitNextInitFrame_NonEngBuild = 200000000; // 200 ms
    static constexpr int64_t kWaitDrainedDuration_NonEngBuild = 300000000; // 300 ms
    static constexpr int64_t kWaitDrainedTimeoutAfterConfig_NonEngBuild = 5000000000; // 5000 ms

protected:  ////                        Data Members (const)
    /**
     * 'u' : user build
     * 'd' : userdebug build
     * 'e' : eng build
     * '?' : unknown
     */
    char                                mBuildType{'?'};

    std::shared_ptr<std::thread>        mThread;//detach thread
    std::atomic_bool                    mThreadRunning{false};
    std::atomic_bool                    mThreadExitPending{false};

    /**
     * Initial frame count.
     */
    uint32_t const                      mConfigInitFrameCount = 0;

    /**
     * Unsed to send every frame to PipelineContext via mpPipelineContext->sendFrame().
     */
    android::wp<PipelineContextT> const mpPipelineContext;

protected:  ////                        Data Members (per-frame)
    std::timed_mutex                    mLock;
    std::condition_variable_any         mCond;

    /**
     * Avoid to promote PipelineContext on isConfigDone() every time.
     */
    mutable std::atomic_bool            mIsConfigDone{false};

    /**
     * enqueue() is allowed only while active (i.e. mActive==true).
     */
    std::atomic_bool                    mActive{true};

    /**
     * Init. frame queue keeps the frames sent via enqueue().
     *
     * Data flow:
     *           pFrame
     *      --> (InitFrameHandler::enqueue) --> mPendingQueue
     *      --> [InitFrameHandler::threadLoop] --> mSendingFrame
     *      --> (PipelineContextImpl::sendFrame) --> IPipelineNode
     */
    InitFrameQueueT                     mPendingQueue;

    /**
     * It's the frame which is "being" sent to PipelineContext.
     *
     * That is, we have dequeued the frame from mInitFrameQueue but have not
     * finished to send it to PipelineContext.
     */
    android::sp<IPipelineFrame>         mSendingFrame;

    std::atomic_uint32_t                mFrameCountSent{0}; // Count up when every sendFrame() succeeds.

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation (Operations)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Operations.
                    ~InitFrameHandler();
                    InitFrameHandler(
                        uint32_t initFrameCount,
                        android::sp<PipelineContextT> pPipelineContext
                    );

protected:  ////    Attributes.
    auto            getWaitNextInitFrame() const -> int64_t;
    auto            getWaitDrainedDuration() const -> int64_t;
    auto            getWaitDrainedTimeoutAfterConfig() const -> int64_t;

protected:  ////    Operations.
    auto            _threadLoop(std::shared_ptr<ThisT> pThis, std::string threadName) -> void;
    auto            threadLoop() -> bool;

    auto            requestExit() -> void;
    auto            isConfigDone() const -> bool;
    auto            dumpStateLocked(android::Printer* printer, bool locked = true) -> void;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Operations.
    static auto     make(
                        uint32_t initFrameCount,
                        android::sp<PipelineContextT> pPipelineContext
                    ) -> std::shared_ptr<InitFrameHandler>;

    auto            dumpState(android::Printer& printer) -> void;
    auto            setConfigDone(int err) -> void;
    auto            enqueue(android::sp<IPipelineFrame>const& pFrame) -> int;
    auto            waitUntilDrained() -> void;
    auto            deactivateAndWaitUntilDrained() -> void;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSCam::v3::pipeline::NSPipelineContext
#endif  //_MTK_HARDWARE_MTKCAM3_PIPELINE_PIPELINE_INITFRAMEHANDLER_H_

