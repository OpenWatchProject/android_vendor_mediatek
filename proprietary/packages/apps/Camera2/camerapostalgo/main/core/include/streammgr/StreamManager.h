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
 * MediaTek Inc. (C) 2019. All rights reserved.
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

#ifndef MAIN_CORE_INCLUDE_STREAMMGR_STREAMMANAGER_H_
#define MAIN_CORE_INCLUDE_STREAMMGR_STREAMMANAGER_H_

#include <IStreamManager.h>
#include <list>
#include <utils/Mutex.h>
#include <cutils/atomic.h>
#include <utils/Vector.h>
#include <utils/Errors.h>
#include <utils/Condition.h>
#include <core/Timer.h>
#include <utils/KeyedVector.h>
#include <campostalgo/StreamInfo.h>
#include <campostalgo/FeatureConfig.h>
#include <processor/PA_Request.h>
#include <vendor/mediatek/hardware/mms/1.0/IMms.h>

using android::Vector;
using android::NOT_ENOUGH_DATA;
using android::DEAD_OBJECT;
using android::Condition;

using com::mediatek::campostalgo::StreamInfo;
using com::mediatek::campostalgo::StreamType;
using com::mediatek::campostalgo::FeatureConfig;

using ::vendor::mediatek::hardware::mms::V1_0::IMms;

namespace NSPA {

extern float getBytesPerPixel(int32_t hal_format);

typedef struct camera3_jpeg_blob {
    uint16_t jpeg_blob_id;
    uint32_t jpeg_size;
} camera3_jpeg_blob_t;

enum {
    CAMERA3_JPEG_BLOB_ID = 0x00FF,
    CAMERA3_JPEG_APP_SEGMENTS_BLOB_ID = 0x0100,
};

class StreamManager: public IStreamManager {

public:
    class ResultHandler;
    class CallbackHandler;

    struct ResultItem: public android::LightRefBase<ResultItem> {
        uint32_t frameNo = 0;
        sp<PipelineConfigure> pipeConfigure = nullptr;
        sp<PA_Request> request = nullptr;
    };
    using ResultQueueT = android::KeyedVector<uint32_t, android::sp<ResultItem>>;

    virtual ~StreamManager();

    virtual Status configure(const FeatureConfig& config,
            FeatureResult* result);

    virtual auto configureStreams(const StreamType type,
            const Vector<sp<android::Surface>>& appSurfaces) -> status_t override;
    virtual void addInterfaceCallback(
            const ::android::sp<ICamPostAlgoCallback>& callback) override;

    /*    virtual void configureAlgos(StreamType type,
     const std::vector<AlgoType>& algos) override;*/

    virtual auto configureHalStreams(const StreamType type,
            const Vector<sp<StreamInfo>> & infos) -> status_t;

    virtual auto getConfigure(
            const StreamType type) -> sp<PipelineConfigure> override;

    int32_t createStreamUniqueId();

    virtual auto updateResult(sp<PA_Request> pr) -> status_t;
    virtual auto getCommonMetadata(const StreamType& type) const -> const IMetadata*;

    virtual auto destroy() ->void;


protected:
    StreamManager(std::string name): mInstanceName(name) {
        }
    virtual auto initialize() -> status_t;
    auto getConfigureLocked(const StreamType type) -> sp<PipelineConfigure>;

private:

    std::string const mInstanceName;
    KeyedVector<StreamType, sp<PipelineConfigure>> mPipeConfigs;
    mutable android::Mutex mOpsLock;
    volatile int32_t mStreamCounter = 0;
    sp<ResultHandler> mResultHandler = nullptr;
    sp<CallbackHandler> mCallbackHandler = nullptr;
    IMetadata mCommonCaptureParams;
    IMetadata mCommonPreviewParams;

    friend class IStreamManager;
};

/**
 * Result Handler
 */
class StreamManager::ResultHandler: public android::Thread {
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  Implementation.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

protected:
    ////
    std::string const mInstanceName;
    mutable android::Mutex mResultQueueLock;
    android::Condition mResultQueueCond;
    ResultQueueT mResultQueue;
    // add mdp service instance
    sp<IMms> mMdpService = nullptr;
    sp<FPSCounter> mFpsCounter = new FPSCounter();
    Timer *mTimer = new Timer(false);
protected:
    ////

    auto dequeResult(ResultQueueT& rvResult) -> status_t;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  Thread Interfaces.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    ResultHandler(std::string name) :
            mInstanceName(name) {

    }

    const char* getInstanceName() {
        return mInstanceName.c_str();
    }
    ////
    // Ask this object's thread to exit. This function is asynchronous, when the
    // function returns the thread might still be running. Of course, this
    // function can be called from a different thread.
    virtual auto requestExit() -> void;

    // Good place to do one-time initializations
    virtual auto readyToRun() -> android::status_t;

private:
    // Derived class must implement threadLoop(). The thread starts its life
    // here. There are two ways of using the Thread object:
    // 1) loop: if threadLoop() returns true, it will be called again if
    //          requestExit() wasn't called.
    // 2) once: if threadLoop() returns false, the thread will exit upon return.
    virtual auto threadLoop() -> bool;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  Interfaces.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    ////    Operations.
    ResultHandler();

    virtual auto destroy() -> void;

    auto enqueResult(sp<ResultItem> const& params) -> status_t;
    auto handleBuffer(sp<android::Surface> sf,
            sp<android::GraphicBuffer> src_gb,
            android::PixelFormat real_format = android::PIXEL_FORMAT_UNKNOWN) ->status_t;

};

/**
 * Callback Handler
 */
class StreamManager::CallbackHandler: public android::Thread {
public:

    typedef std::list<sp<FeatureParam>> CallbackQueue;

    CallbackHandler(std::string name) :
            mInstanceName(name) {
    }

    virtual ~CallbackHandler();
    const char* getInstanceName() {
            return mInstanceName.c_str();
        }
    void addInterfaceCallback(const sp<ICamPostAlgoCallback>& cb);
protected:
    std::string const mInstanceName;
    mutable android::Mutex mQueue1Lock;
    android::Condition mQueue1Cond;
    CallbackQueue mQueue1;

    mutable android::Mutex mQueue2Lock;
    android::Condition mQueue2Cond;
    CallbackQueue mQueue2;
    sp<ICamPostAlgoCallback> mInterfaceCb;

    auto waitUntilQueue1NotEmpty() -> bool;

    auto performCallback() -> void;

private:
    // Derived class must implement threadLoop(). The thread starts its life
    // here. There are two ways of using the Thread object:
    // 1) loop: if threadLoop() returns true, it will be called again if
    //          requestExit() wasn't called.
    // 2) once: if threadLoop() returns false, the thread will exit upon return.
    virtual auto threadLoop() -> bool;
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  Thread Interfaces.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    ////
    // Ask this object's thread to exit. This function is asynchronous, when the
    // function returns the thread might still be running. Of course, this
    // function can be called from a different thread.
    virtual auto requestExit() -> void;

    // Good place to do one-time initializations
    virtual auto readyToRun() -> android::status_t;

    virtual auto destroy() -> void;
    virtual auto waitUntilDrained(nsecs_t const timeout) -> int;

    virtual auto enqueResult(sp<FeatureParam>& item) -> status_t;
};

}

#endif /* MAIN_CORE_INCLUDE_STREAMMGR_STREAMMANAGER_H_ */
