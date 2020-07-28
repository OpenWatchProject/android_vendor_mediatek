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

#define LOG_TAG "mtkcam-InitFrameHandler"

#include "PipelineContextImpl.h"
#include "InitFrameHandler.h"
//
#include <sys/prctl.h>
//
#include <cstring>
#include <sstream>
#include <mtkcam/utils/std/ULog.h>
//
#include "MyUtils.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_CONTEXT);

using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v3::pipeline;
using namespace NSCam::v3::pipeline::NSPipelineContext;
using namespace NSCam::Utils::ULog;
#define ThisNamespace   PipelineContext::PipelineContextImpl::InitFrameHandler

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
auto
ThisNamespace::
make(
    uint32_t initFrameCount,
    android::sp<PipelineContextT> pPipelineContext
) -> std::shared_ptr<InitFrameHandler>
{
    auto pInst = std::make_shared<InitFrameHandler>(initFrameCount, pPipelineContext);
    MY_LOGF_IF(pInst==nullptr, "Fail on new InitFrameHandler");

    pInst->mThread = std::make_shared<std::thread>(
        &ThisNamespace::_threadLoop,
        pInst, pInst, "InitFrameHandler");
    MY_LOGF_IF(pInst->mThread==nullptr, "Fail on std::make_shared<std::thread>");
    pInst->mThread->detach();

    return pInst;
}


/******************************************************************************
 *
 ******************************************************************************/
ThisNamespace::
~InitFrameHandler()
{
    MY_LOGD("%p", this);
}


/******************************************************************************
 *
 ******************************************************************************/
ThisNamespace::
InitFrameHandler(
    uint32_t initFrameCount,
    android::sp<PipelineContextT> pPipelineContext
)
    : mConfigInitFrameCount(initFrameCount)
    , mpPipelineContext(pPipelineContext)
{
    char const* keyBuildType = "ro.build.type";
    char valueBuildType[PROPERTY_VALUE_MAX] = {0};
    property_get(keyBuildType, valueBuildType, "?");
    if ( 0 == strncmp("userdebug", valueBuildType, PROPERTY_VALUE_MAX) ) { mBuildType = 'd'; }
    else if ( 0 == strncmp("user", valueBuildType, PROPERTY_VALUE_MAX) ) { mBuildType = 'u'; }
    else if ( 0 == strncmp("eng",  valueBuildType, PROPERTY_VALUE_MAX) ) { mBuildType = 'e'; }

    MY_LOGD("%p - %s=%s", this, keyBuildType, valueBuildType);
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
dumpStateLocked(android::Printer* printer, bool locked) -> void
{
    printer->printLine("*Init frames*");

    printer->printFormatLine(
        " InitFrameCount:%u-1 FrameCountSent:%u"
        " Active:%d ThreadRunning:%d ThreadExitPending:%d"
        " PipelineContext:%p",
        mConfigInitFrameCount,
        mFrameCountSent.load(),
        mActive.load(),
        mThreadRunning.load(),
        mThreadExitPending.load(),
        mpPipelineContext.unsafe_get()
    );

    if (locked) {
        if (mSendingFrame != nullptr) {
            printer->printFormatLine(" R%u F%u : being sent", mSendingFrame->getRequestNo(), mSendingFrame->getFrameNo());
        }

        for (auto pFrame : mPendingQueue) {
            printer->printFormatLine(" R%u F%u : pending", pFrame->getRequestNo(), pFrame->getFrameNo());
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
dumpState(android::Printer& printer) -> void
{
    if (mLock.try_lock_for(std::chrono::milliseconds(50))) {
        dumpStateLocked(&printer);
        mLock.unlock();
    }
    else {
        dumpStateLocked(&printer, false);
        printer.printLine("mLock.try_lock_for");
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
getWaitNextInitFrame() const -> int64_t
{
    return ('e' == mBuildType)
        ? kWaitNextInitFrame_EngBuild
        : kWaitNextInitFrame_NonEngBuild
        ;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
getWaitDrainedDuration() const -> int64_t
{
    return ('e' == mBuildType)
        ? kWaitDrainedDuration_EngBuild
        : kWaitDrainedDuration_NonEngBuild
        ;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
getWaitDrainedTimeoutAfterConfig() const -> int64_t
{
    return ('e' == mBuildType)
        ? kWaitDrainedTimeoutAfterConfig_EngBuild
        : kWaitDrainedTimeoutAfterConfig_NonEngBuild
        ;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
enqueue(android::sp<IPipelineFrame>const& pFrame) -> int
{
    {
        std::lock_guard<std::timed_mutex> _lg(mLock);

        if (CC_LIKELY( mActive )) {
            mPendingQueue.push_back(pFrame);
            mCond.notify_all();
            MY_LOGI("RequestNo:%u FrameNo:%u", pFrame->getRequestNo(), pFrame->getFrameNo());
            return OK;
        }
    }

    // enqueue while non-active
    {
        std::ostringstream oss;
        oss << "RequestNo:" << pFrame->getRequestNo()
            << " FrameNo:"  << pFrame->getFrameNo()
            << " is being enqueued while non-active"
                ;
        auto errMsg = oss.str();

        MY_LOGE("%s", errMsg.c_str());

        ULogPrinter logPrinter(__ULOG_MODULE_ID, LOG_TAG, DetailsType::DETAILS_ERROR, "[enqueue] ");
        dumpState(logPrinter);

        NSCam::Utils::triggerAeeException(LOG_TAG, NULL, errMsg.c_str());
    }

    return NO_INIT;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
deactivateAndWaitUntilDrained() -> void
{
    // Ensure enqueue() is disabled before waiting until drained.
    {
        std::lock_guard<std::timed_mutex> _lg(mLock);
        mActive = false; //set it while locking to ensure enqueue() is atomic.
    }

    waitUntilDrained();

    // Request to exit the thread.
    requestExit();
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
waitUntilDrained() -> void
{
    auto toString = [this](){
        return  std::string("\'")
              + "#Init:" + std::to_string(mConfigInitFrameCount) + "-1 > "
              + "#Sent:" + std::to_string(mFrameCountSent.load())
              + std::string("\'?")
              + std::string(" PendingQueue#:") + std::to_string(mPendingQueue.size())
              + std::string(" SendingFrame#:") + std::to_string(mSendingFrame!=nullptr)
                ;
    };

    auto kWaitDrainedDuration = getWaitDrainedDuration();
    auto kWaitDrainedTimeoutAfterConfig = getWaitDrainedTimeoutAfterConfig();

    bool hasTriggerAeeForTimeout = false;
    nsecs_t startTimeAfterConfigured = 0;
    while (1)
    {
        bool isTimedout = false;
        {
            std::unique_lock<std::timed_mutex> _ulock(mLock);

            bool exitLoop =
                            mPendingQueue.empty()       // empty queue
                        &&  mSendingFrame == nullptr    // not sending any frame
                            ;
            if ( exitLoop ) {
                break;
            }

            MY_LOGD("+ %s", toString().c_str());
            auto status = mCond.wait_for(_ulock, std::chrono::nanoseconds(kWaitDrainedDuration));
            isTimedout = ( status == std::cv_status::timeout );
            MY_LOGD("- %s", toString().c_str());
        }

        if ( isTimedout ) {
            // Do nothing if the configuration is on-going.
            if ( 0 == startTimeAfterConfigured && ! isConfigDone() ) {
                continue;
            }

            // Here the configuration is done.
            if ( 0 == startTimeAfterConfigured ) {
                startTimeAfterConfigured = systemTime();
                MY_LOGD("configure done:%d", isConfigDone());
                continue;
            }

            // Do something if it timed out after configuring.
            auto elapsedTime = (systemTime() - startTimeAfterConfigured);
            if ( elapsedTime >= kWaitDrainedTimeoutAfterConfig ) {
                std::string errMsg = (std::ostringstream()
                    << "waitUntilDrained" << " timed out (" << (elapsedTime/1000000) << "ms)"
                    << " after configuration done"
                ).str();
                ULogPrinter logPrinter(__ULOG_MODULE_ID, LOG_TAG, DetailsType::DETAILS_ERROR, "[waitUntilDrained] ");
                logPrinter.printLine(errMsg.c_str());
                dumpState(logPrinter);

                if ( ! hasTriggerAeeForTimeout ) {
                    hasTriggerAeeForTimeout = true;
                    NSCam::Utils::triggerAeeException(LOG_TAG, NULL, errMsg.c_str());
                }
                continue; // Still try waiting until drained.
            }
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
requestExit() -> void
{
    {
        mThreadExitPending = true;
    }
    {
        std::lock_guard<std::timed_mutex> _lg(mLock);
        mCond.notify_all();

        MY_LOGF_IF(!mPendingQueue.empty(), "PendingQueue#:%zu", mPendingQueue.size());
    }

    //join();
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
isConfigDone() const -> bool
{
    return mIsConfigDone.load();
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
setConfigDone(int err) -> void
{
    MY_LOGD("err:%d", err);
    if ( 0 == err ) {
        mIsConfigDone.store(true);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
_threadLoop(std::shared_ptr<ThisT> pThis, std::string threadName) -> void
{
    mThreadRunning = true;

    ::prctl(PR_SET_NAME, (unsigned long)threadName.c_str(), 0, 0, 0);

    std::shared_ptr<ThisT>  strong(pThis);
    std::weak_ptr<ThisT>    weak(strong);
    pThis.reset();

    do {
        bool result = threadLoop();
        {
            if ( ! result || mThreadExitPending.load() ) {
                mThreadExitPending = true;
                break;
            }
        }

        // Release our strong reference, to let a chance to the thread to die a peaceful death.
        // And immediately, re-acquire a strong reference for the next loop
        strong.reset();
        strong = weak.lock();
    } while (strong != 0);

    mThreadRunning = false;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
threadLoop() -> bool
{
    android::sp<IPipelineFrame> pFrame;
    auto kWaitNextInitFrame = getWaitNextInitFrame();
    bool isTimeout = false;

    //  Try to dequeue a frame.
    {
        std::unique_lock<std::timed_mutex> _ulock(mLock);

        if  ( mPendingQueue.empty() ) {
            auto status = mCond.wait_for(_ulock, std::chrono::nanoseconds(kWaitNextInitFrame));
            if ( status == std::cv_status::timeout ) {
                isTimeout = true;// do timeout handling outside locking.
            }
        }

        if  ( ! mPendingQueue.empty() ) {
            // Dequeue a frame since not empty.
            mSendingFrame = mPendingQueue.front();
            pFrame = mSendingFrame;
            mPendingQueue.erase(mPendingQueue.begin());
        }
    }

    //  Push the frame to PipelineContext if possible.
    if (CC_LIKELY( pFrame != nullptr )) {
        auto pPipelineContext = mpPipelineContext.promote();
        MY_LOGF_IF(pPipelineContext==nullptr,
            "Fail on mpPipelineContext.promote() - requestNo:%u frameNo:%u",
            pFrame->getRequestNo(), pFrame->getFrameNo());

        int err = pPipelineContext->sendFrame(pFrame);
        MY_LOGE_IF(err!=0, "sendFrame err:%d(%s)", err, ::strerror(-err));

        // Don't hold PipelineContext before waitUntilDrained() returns.
        pPipelineContext.clear();
        {
            std::lock_guard<std::timed_mutex> _lg(mLock);

            mSendingFrame = nullptr;
            pFrame = nullptr;

            mFrameCountSent++;
            mCond.notify_all();
        }
        return true;
    }

    // Try to handle timeout...
    if (CC_UNLIKELY( isTimeout && isConfigDone() )) {
        {
            std::unique_lock<std::timed_mutex> _ulock(mLock);
            MY_LOGW(
                "Timeout(%" PRIu64 "ms) while waiting for next init frame - exitPending:%d PendingQueue#:%zu",
                kWaitNextInitFrame/1000000, mThreadExitPending.load(), mPendingQueue.size()
            );
        }
        ULogPrinter logPrinter(__ULOG_MODULE_ID, LOG_TAG, DetailsType::DETAILS_WARNING, "[threadLoop] ");
        dumpState(logPrinter);
        return true;
    }

    return  true;
}

