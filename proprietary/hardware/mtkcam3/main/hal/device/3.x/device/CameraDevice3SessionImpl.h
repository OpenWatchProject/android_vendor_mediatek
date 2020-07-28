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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#include <mtkcam3/pipeline/model/IPipelineModelManager.h>
#pragma GCC diagnostic pop
//
#include <ICameraDevice3Session.h>
#include <IAppStreamManager.h>
//
#include <functional>
//
#include <utils/Mutex.h>
#include <utils/Thread.h>
#include <cutils/properties.h>
//
#include "EventLog.h"
#include "Cam3CPUCtrl.h"
#include "DisplayIdleDelayUtil.h"
#include <mtkcam/utils/std/CallStackLogger.h>
#include <future>

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
    , public pipeline::model::IPipelineModelCallback
    , public android::hardware::hidl_death_recipient
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////

    static const nsecs_t _ONE_MS = 1000000;

    static bool isLowMemoryDevice()
    {
        return ::property_get_bool("ro.config.low_ram", false);
    }

    nsecs_t getExitCommandTimeout() const
    {
        #if (MTKCAM_TARGET_BUILD_VARIANT==2)
        if(isLowMemoryDevice()) {
            return ::property_get_int32("vendor.cam3dev.exitcommandtimeout", 30000) * _ONE_MS;
        }
        #endif

        return 10000000000; //10s
    }

    nsecs_t getWaitCommandTimeout() const
    {
        #if (MTKCAM_TARGET_BUILD_VARIANT==2)
        if(isLowMemoryDevice()) {
            return ::property_get_int32("vendor.cam3dev.waitcommandtimeout", 30000) * _ONE_MS;
        }
        #endif

        return 10000000000; //10s
    }

    nsecs_t getFlushAndWaitTimeout() const
    {
        #if (MTKCAM_TARGET_BUILD_VARIANT==2)
        if(isLowMemoryDevice()) {
            return ::property_get_int32("vendor.cam3dev.flushandwaittimeout", 9000) * _ONE_MS;
        }
        #endif

        return 3000000000;  //3s
    }


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

    protected:
        nsecs_t getDumpTryLockTimeout() const
        {
            #if (MTKCAM_TARGET_BUILD_VARIANT==2)
            if(isLowMemoryDevice()) {
                return ::property_get_int32("vendor.cam3dev.dumptrylocktimeout", 900) * _ONE_MS;
            }
            #endif

            return 300000000;   //300ms
        }
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
    ::android::sp<V3_4::ICameraDeviceCallback>    mCameraDeviceCallback = nullptr;
    ::android::sp<CommandHandler>           mCommandHandler = nullptr;

    mutable android::Mutex                  mAppStreamManagerLock;
    ::android::sp<IAppStreamManager>        mAppStreamManager = nullptr;
    std::shared_ptr<EventLogPrinter>        mAppStreamManagerErrorState;
    std::shared_ptr<EventLogPrinter>        mAppStreamManagerWarningState;
    std::shared_ptr<EventLogPrinter>        mAppStreamManagerDebugState;

    mutable android::Mutex                  mPipelineModelLock;
    ::android::sp<pipeline::model::IPipelineModel>
                                            mPipelineModel = nullptr;
    //  setup during configuring streams

    //  setup during submitting requests.
    mutable android::Mutex                  mRequestingLock;
    MINT32                                  mRequestingAllowed = 0;
    bool                                    m1stRequestNotSent = true;

    //CPU Control
    Cam3CPUCtrl*                            mpCpuCtrl = nullptr;
    MUINT32                                 mCpuPerfTime = 1;   //1 sec
    DisplayIdleDelayUtil                    mDisplayIdleDelayUtil;
    //CPU preferidle, this instance does not have timeout
    Cam3CPUCtrl*                            mpCpuPrefIdleCtrl = nullptr;

    // linkToDeath
    ::android::hidl::base::V1_0::DebugInfo  mLinkToDeathDebugInfo;

    // Callstack
    NSCam::Utils::CallStackLogger           mCallstaskLogger;

    // Prevent dump thread blocking
    std::future<void>                       mDumpFut;

    // scenario timestamp
    uint64_t                                mConfigTimestamp;

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
    auto                getSafePipelineModel() const -> ::android::sp<pipeline::model::IPipelineModel>;
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
                            const ::android::sp<V3_4::ICameraDeviceCallback>& callback
                            ) -> ::android::status_t;

    virtual auto        onCloseLocked() -> void;

    virtual auto        onConfigureStreamsLocked(
                            const WrappedStreamConfiguration& requestedConfiguration,
                            WrappedHalStreamConfiguration& halConfiguration
                            ) -> ::android::status_t;

    virtual auto        onFlushLocked() -> ::android::status_t;

    virtual auto        onProcessCaptureRequest(
                            const hidl_vec<V3_4::CaptureRequest>& wrappedRequests,
                            const hidl_vec<BufferCache>& cachesToRemove,
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
                            const ::android::sp<V3_5::ICameraDeviceCallback>& callback
                            ) -> ::android::status_t override;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ICameraDeviceSession Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual Return<void> constructDefaultRequestSettings(RequestTemplate type, constructDefaultRequestSettings_cb _hidl_cb) override;
    virtual Return<void> configureStreams(const V3_2::StreamConfiguration& requestedConfiguration, configureStreams_cb _hidl_cb) override;
    virtual Return<void> getCaptureRequestMetadataQueue(getCaptureRequestMetadataQueue_cb _hidl_cb) override;
    virtual Return<void> getCaptureResultMetadataQueue(getCaptureResultMetadataQueue_cb _hidl_cb) override;
    virtual Return<void> processCaptureRequest(const hidl_vec<V3_2::CaptureRequest>& requests, const hidl_vec<BufferCache>& cachesToRemove, processCaptureRequest_cb _hidl_cb) override;
    virtual Return<Status> flush() override;
    virtual Return<void> close() override;

    //V3_4
    virtual Return<void> configureStreams_3_3(const V3_2::StreamConfiguration& requestedConfiguration, configureStreams_3_3_cb _hidl_cb) override;
    virtual Return<void> configureStreams_3_4(const V3_4::StreamConfiguration& requestedConfiguration, configureStreams_3_4_cb _hidl_cb) override;
    virtual Return<void> processCaptureRequest_3_4(const hidl_vec<V3_4::CaptureRequest>& requests, const hidl_vec<BufferCache>& cachesToRemove, processCaptureRequest_3_4_cb _hidl_cb) override;

    //V3_5
    virtual Return<void> configureStreams_3_5(const V3_5::StreamConfiguration& requestedConfiguration, configureStreams_3_5_cb _hidl_cb) override;
    virtual Return<void> signalStreamFlush(const hidl_vec<int32_t>& streamIds, uint32_t streamConfigCounter) override;
    virtual Return<void> isReconfigurationRequired(const hidl_vec<uint8_t>& oldSessionParams, const hidl_vec<uint8_t>& newSessionParams, isReconfigurationRequired_cb _hidl_cb) override;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineModelCallback Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////        Operations.
    virtual auto        onFrameUpdated(
                            pipeline::model::UserOnFrameUpdated const& params
                            ) -> void;

    virtual auto        onMetaResultAvailable(
                            pipeline::model::UserOnMetaResultAvailable&& arg
                        ) -> void;

    virtual auto        onImageBufferReleased(
                            pipeline::model::UserOnImageBufferReleased&& arg __unused
                        ) -> void {}

    virtual auto        onRequestCompleted(
                            pipeline::model::UserOnRequestCompleted&& arg __unused
                        ) -> void {}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ::android::hardware::hidl_death_recipient
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    virtual void       serviceDied(uint64_t cookie, const android::wp<android::hidl::base::V1_0::IBase>& who) override;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_MAIN_HAL_DEVICE_3_X_DEVICE_DEPEND_CAMERADEVICE3SESSIONIMPL_H_

