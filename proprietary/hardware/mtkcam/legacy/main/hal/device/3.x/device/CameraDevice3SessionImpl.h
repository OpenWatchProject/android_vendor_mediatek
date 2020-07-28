/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#ifndef _MTK_HARDWARE_MTKCAM_MAIN_HAL_DEVICE_3_X_DEVICE_DEPEND_CAMERADEVICE3SESSIONIMPL_H_
#define _MTK_HARDWARE_MTKCAM_MAIN_HAL_DEVICE_3_X_DEVICE_DEPEND_CAMERADEVICE3SESSIONIMPL_H_
//
#include <mtkcam/v3/pipeline/IPipelineModelManager.h>
#include <ICameraDevice3Session.h>
#include <IAppStreamManager.h>
//
#include <functional>
//
#include <utils/Mutex.h>
#include <utils/Thread.h>
//
#include "EventLog.h"

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {


/******************************************************************************
 *
 ******************************************************************************/
class CameraDevice3SessionImpl
    : public ICameraDevice3Session
    , public IPipelineModelManager::IAppCallback
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////

    static const size_t                     kDumpTryLockTimeout     = 300000000;    // 300 ms
    static const nsecs_t                    kExitCommandTimeout     = 10000000000;  // 10 sec
    static const nsecs_t                    kWaitCommandTimeout     = 10000000000;  // 10 sec
    static const nsecs_t                    kFlushAndWaitTimeout    = 3000000000;   //  3 sec

    class   CommandHandler : public android::Thread
    {
    public:
        using CommandT = std::function<void()>;

    protected:
        std::string const                   mLogPrefix;
        mutable ::android::Mutex            mLock;
        ::android::Condition                mCond;
        CommandT                            mCommand;
        std::string                         mCommandName;

    private:
        virtual auto    threadLoop() -> bool;
        auto const&     getLogPrefix() const            { return mLogPrefix; }

    public:
                        CommandHandler(int32_t instanceId);
        auto            print(::android::Printer& printer) const -> void;
        auto            requestExitAndWait(nsecs_t const timeout) -> int;
        auto            waitDone(void* handle, nsecs_t const timeout) -> int;
        auto            add(char const* name, CommandT cmd) -> void*;
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////

    //  setup during constructor
    CreationInfo                            mStaticInfo;
    std::string const                       mLogPrefix;
    mutable ::android::Mutex                mOpsLock;

    //  setup during opening camera
    int32_t                                 mLogLevel = 0;
    EventLog                                mStateLog;
    ::android::sp<ICameraDeviceCallback>    mCameraDeviceCallback = nullptr;
    ::android::sp<CommandHandler>           mCommandHandler = nullptr;

    mutable android::Mutex                  mAppStreamManagerLock;
    ::android::sp<IAppStreamManager>        mAppStreamManager = nullptr;
    std::shared_ptr<EventLogPrinter>        mAppStreamManagerErrorState;
    std::shared_ptr<EventLogPrinter>        mAppStreamManagerWarningState;
    std::shared_ptr<EventLogPrinter>        mAppStreamManagerDebugState;

    mutable android::Mutex                  mPipelineModelLock;
    ::android::sp<IPipelineModelManager>    mPipelineModel = nullptr;

    //  setup during configuring streams

    //  setup during submitting requests.
    mutable android::Mutex                  mRequestingLock;
    MINT32                                  mRequestingAllowed = 0;
    bool                                    m1stRequestNotSent = true;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////        Instantiation.
    virtual             ~CameraDevice3SessionImpl();
                        CameraDevice3SessionImpl(CreationInfo const& info);

public:     ////        Operations.
    auto const&         getDeviceInfo() const           { return *mStaticInfo.mStaticDeviceInfo; }
    auto const&         getInstanceName() const         { return getDeviceInfo().mInstanceName; }
    auto                getInstanceId() const           { return getDeviceInfo().mInstanceId; }
    auto                getLogLevel() const             { return mLogLevel; }
    auto const&         getLogPrefix() const            { return mLogPrefix; }

protected:  ////        Operations.
    auto                getSafeAppStreamManager() const -> ::android::sp<IAppStreamManager>;
    auto                getSafePipelineModel() const -> ::android::sp<IPipelineModelManager>;
    auto                flushAndWait() -> ::android::status_t;

protected:  ////        Operations.
    auto                enableRequesting() -> void;
    auto                disableRequesting() -> void;

protected:  ////        Operations.
    virtual auto        waitUntilOpenDoneLocked() -> bool;
    virtual auto        tryRunCommandLocked(nsecs_t const timeout, char const* commandName, CommandHandler::CommandT command) -> int;
    virtual auto        handleCommandFailureLocked(int status) -> void;

protected:  ////        [Template method] Operations.
    virtual auto        onOpenLocked(
                            const ::android::sp<ICameraDeviceCallback>& callback
                            ) -> ::android::status_t;

    virtual auto        onCloseLocked() -> void;

    virtual auto        onConfigureStreamsLocked(
                            const StreamConfiguration& requestedConfiguration,
                            HalStreamConfiguration& halConfiguration
                            ) -> ::android::status_t;

    virtual auto        onFlushLocked() -> ::android::status_t;

    virtual auto        onProcessCaptureRequest(
                            const hidl_vec<CaptureRequest>& requests,
                            const ::android::hardware::hidl_vec<BufferCache>& cachesToRemove,
                            uint32_t& numRequestProcessed
                            ) -> ::android::status_t;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  NSCam::ICameraDevice3Session Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    virtual auto        dumpState(
                            ::android::Printer& printer,
                            const std::vector<std::string>& options
                            ) -> void override;

    virtual auto        open(
                            const ::android::sp<ICameraDeviceCallback>& callback
                            ) -> ::android::status_t override;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ICameraDeviceSession Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual Return<void> constructDefaultRequestSettings(RequestTemplate type, constructDefaultRequestSettings_cb _hidl_cb) override;
    virtual Return<void> configureStreams(const StreamConfiguration& requestedConfiguration, configureStreams_cb _hidl_cb) override;
    virtual Return<void> getCaptureRequestMetadataQueue(getCaptureRequestMetadataQueue_cb _hidl_cb) override;
    virtual Return<void> getCaptureResultMetadataQueue(getCaptureResultMetadataQueue_cb _hidl_cb) override;
    virtual Return<void> processCaptureRequest(const hidl_vec<CaptureRequest>& requests, const ::android::hardware::hidl_vec<BufferCache>& cachesToRemove, processCaptureRequest_cb _hidl_cb) override;
    virtual Return<Status> flush() override;
    virtual Return<void> close() override;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineModelManager::IAppCallback Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////        Operations.
    virtual auto        onFrameUpdated(
                            uint32_t const requestNo,
                            intptr_t const userId,
                            ssize_t const nOutMetaLeft,
                            android::Vector<android::sp<IMetaStreamBuffer> > vOutMeta
                            ) -> void;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_MAIN_HAL_DEVICE_3_X_DEVICE_DEPEND_CAMERADEVICE3SESSIONIMPL_H_

