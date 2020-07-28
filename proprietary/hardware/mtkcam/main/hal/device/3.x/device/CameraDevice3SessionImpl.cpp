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

#include "CameraDevice3SessionImpl.h"
#include "MyUtils.h"
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
#if PIPELINE_NEW_ARCH
using namespace NSCam::v3::pipeline::model;
#endif

#define ThisNamespace   CameraDevice3SessionImpl

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s::%s] " fmt, getLogPrefix().c_str(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s::%s] " fmt, getLogPrefix().c_str(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s::%s] " fmt, getLogPrefix().c_str(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s::%s] " fmt, getLogPrefix().c_str(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s::%s] " fmt, getLogPrefix().c_str(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s::%s] " fmt, getLogPrefix().c_str(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s::%s] " fmt, getLogPrefix().c_str(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
namespace {
struct TimeoutCaculator
{
    nsecs_t const mStartTime;
    nsecs_t const mTimeout;

            TimeoutCaculator(nsecs_t const t)
                : mStartTime(::systemTime())
                , mTimeout(t)
            {}

    nsecs_t timeoutToWait() const
            {
                nsecs_t const elapsedInterval = (::systemTime() - mStartTime);
                nsecs_t const timeoutToWait = (mTimeout > elapsedInterval)
                                            ? (mTimeout - elapsedInterval)
                                            :   0
                                            ;
                return timeoutToWait;
            }
};
};


/******************************************************************************
 *
 ******************************************************************************/
ThisNamespace::CommandHandler::
CommandHandler(int32_t instanceId)
    : Thread(false/*canCallJava*/)
    , mLogPrefix(std::to_string(instanceId)+"-session-cmd")
    , mCommand(nullptr)
    , mCommandName()
{
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::CommandHandler::
print(::android::Printer& printer) const -> void
{
    printer.printFormatLine("\n== CommandHandler (tid:%d) isRunning:%d exitPending:%d ==", getTid(), isRunning(), exitPending());

    std::string commandName;
    if ( OK == mLock.timedLock(kDumpTryLockTimeout) ) {
        commandName = mCommandName;
        mLock.unlock();
    }
    else {
        printer.printLine("CommandHandler: lock failed");
        commandName = mCommandName;
    }

    if (commandName.empty()) {
        printer.printLine("   No pending command");
    }
    else {
        printer.printFormatLine("   Command \"%s\" is in flight", commandName.c_str());
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::CommandHandler::
threadLoop() -> bool
{
    CommandT command = nullptr;
    {
        Mutex::Autolock _l(mLock);
        while ( ! exitPending() && ! mCommand ) {
            int err = mCond.wait(mLock);
            if (OK != err) {
                MY_LOGW("exitPending:%d err:%d(%s)", exitPending(), err, ::strerror(-err));
                MY_LOGW("%s command(%s)", (!mCommand ? "invalid" : "valid"), (!mCommandName.empty() ? mCommandName.c_str() : "empty"));
            }
        }
        command = mCommand;
    }

    if ( command != nullptr ) {
        command();
    }

    {
        Mutex::Autolock _l(mLock);

        mCommandName.clear();
        mCommand = nullptr;
        mCond.broadcast();
    }

    return true;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::CommandHandler::
requestExitAndWait(nsecs_t const timeout) -> int
{
    auto h = this->add("requestExitAndWait", [this](){
        Thread::requestExit();
        MY_LOGD("requestExitAndWait done");
    });
    if ( h == nullptr ) {
        Thread::requestExit();
        MY_LOGW("requestExitAndWait cannot run");
        return NO_INIT;
    }
    return this->waitDone(h, timeout);
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::CommandHandler::
waitDone(void* handle, nsecs_t const timeout) -> int
{
    if (this != handle) {
        MY_LOGE("this:%p != handle:%p", this, handle);
        return NO_INIT;
    }
    //
    TimeoutCaculator toc(timeout);
    //
    Mutex::Autolock _l(mLock);
    int err = OK;
    while ( ! exitPending() && mCommand != nullptr )
    {
        err = mCond.waitRelative(mLock, toc.timeoutToWait());
        if  ( OK != err ) {
            break;
        }
    }
    //
    if  ( mCommand != nullptr ) {
        MY_LOGE(
            "%s command(%s) isRunning:%d exitPending:%d timeout(ns):%" PRId64 " elapsed(ns):%" PRId64 " err:%d(%s)",
            (!mCommand ? "invalid" : "valid"),
            (!mCommandName.empty() ? mCommandName.c_str() : "empty"),
            isRunning(), exitPending(), timeout, toc.timeoutToWait(), err, ::strerror(-err)
        );
        return exitPending() ? DEAD_OBJECT : err;
    }

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::CommandHandler::
add(char const* name, CommandT cmd) -> void*
{
    Mutex::Autolock _l(mLock);

    if ( mCommand != nullptr ) {
        MY_LOGE("fail to add a new command \"%s\" since the previous command \"%s\" is still in flight", name, (!mCommandName.empty() ? mCommandName.c_str() : "empty"));
        return nullptr;
    }

    mCommandName = name;
    mCommand = cmd;
    mCond.broadcast();
    return this;
}


/******************************************************************************
 *
 ******************************************************************************/
ThisNamespace::
~CameraDevice3SessionImpl()
{
}


/******************************************************************************
 *
 ******************************************************************************/
ThisNamespace::
CameraDevice3SessionImpl(CreationInfo const& info)
    : ICameraDevice3Session()
    , mStaticInfo(info)
    , mLogPrefix(std::to_string(info.mStaticDeviceInfo->mInstanceId)+"-session")
    , mLogLevel(0)
    , mStateLog()
    , mAppStreamManagerErrorState(std::make_shared<EventLogPrinter>(15, 25))
    , mAppStreamManagerWarningState(std::make_shared<EventLogPrinter>(25, 15))
    , mAppStreamManagerDebugState(std::make_shared<EventLogPrinter>())
{
    MY_LOGE_IF(mAppStreamManagerErrorState==nullptr, "Bad mAppStreamManagerErrorState");
    MY_LOGE_IF(mAppStreamManagerWarningState==nullptr, "Bad mAppStreamManagerWarningState");
    MY_LOGE_IF(mAppStreamManagerDebugState==nullptr, "Bad mAppStreamManagerDebugState");
    MY_LOGD("%p", this);
    mStateLog.add("-> initialized");
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
getSafeAppStreamManager() const -> ::android::sp<IAppStreamManager>
{
    //  Although mAppStreamManager is setup during opening camera,
    //  we're not sure any callback to this class will arrive
    //  between open and close calls.
    Mutex::Autolock _l(mAppStreamManagerLock);
    return mAppStreamManager;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
#if PIPELINE_NEW_ARCH
getSafePipelineModel() const -> ::android::sp<IPipelineModel>
#else
getSafePipelineModel() const -> ::android::sp<IPipelineModelManager>
#endif
{
    Mutex::Autolock _l(mPipelineModelLock);
    return mPipelineModel;
}


/******************************************************************************
 *
 ******************************************************************************/
/**
 * flush() must only return when there are no more outstanding buffers or
 * requests left in the HAL. The framework may call configure_streams (as
 * the HAL state is now quiesced) or may issue new requests.
 *
 * Performance requirements:
 *
 * The HAL should return from this call in 100ms, and must return from this
 * call in 1000ms. And this call must not be blocked longer than pipeline
 * latency (see S7 for definition).
 */
auto
ThisNamespace::
flushAndWait() -> ::android::status_t
{
    NSCam::Utils::CamProfile profile(__FUNCTION__, LOG_TAG);
    profile.print("+ %s", getInstanceName().c_str());
    //
    TimeoutCaculator toc(kFlushAndWaitTimeout);
    int err = OK;
    //
    auto pPipelineModel = getSafePipelineModel();
    if  ( pPipelineModel != 0 ) {
        err = pPipelineModel->beginFlush();
        MY_LOGW_IF(OK!=err, "pPipelineModel->beginFlush err:%d(%s)", -err, ::strerror(-err));
    }
    //
    auto pAppStreamManager = getSafeAppStreamManager();
    if  ( pAppStreamManager != 0 ) {
        profile.print("waitUntilDrained +");
        err = pAppStreamManager->waitUntilDrained(toc.timeoutToWait());
        profile.print("waitUntilDrained -");
        MY_LOGW_IF(OK!=err, "AppStreamManager::waitUntilDrained err:%d(%s)", -err, ::strerror(-err));
    }
    //
    if  ( pPipelineModel != 0 ) {
        pPipelineModel->endFlush();
    }
    //
    profile.print("-");
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
enableRequesting() -> void
{
    Mutex::Autolock _lRequesting(mRequestingLock);
    ::android_atomic_release_store(1, &mRequestingAllowed);
    m1stRequestNotSent = true;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
disableRequesting() -> void
{
    Mutex::Autolock _lRequesting(mRequestingLock);
    ::android_atomic_release_store(0, &mRequestingAllowed);
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
waitUntilOpenDoneLocked() -> bool
{
    auto pPipelineModel = getSafePipelineModel();
    if  ( pPipelineModel != nullptr ) {
#if PIPELINE_NEW_ARCH
        return pPipelineModel->waitUntilOpenDone();
#else
        return pPipelineModel->waitUntilInitDone();
#endif
    }
    return true;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
tryRunCommandLocked(
    nsecs_t const timeout,
    char const* commandName,
    CommandHandler::CommandT command
) -> int
{
    if ( mCommandHandler == nullptr ) {
        MY_LOGE("Bad mCommandHandler");
        return NO_INIT;
    }

    auto h = mCommandHandler->add(commandName, command);
    if ( ! h ) {
        return NO_INIT;
    }

    auto err = mCommandHandler->waitDone(h, timeout);
    if ( err != OK ) {
        handleCommandFailureLocked(err);
        //
        while( (err = mCommandHandler->waitDone(h, timeout)) != OK ) {
            handleCommandFailureLocked(err);
        }
    }

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
handleCommandFailureLocked(int status) -> void
{
    MY_LOGE("err:%d(%s)", status, ::strerror(-status));

    LogPrinter printer(LOG_TAG);

    // dump the callstack of command handler thread
    auto pCommandHandler = mCommandHandler;
    if ( pCommandHandler != nullptr ) {
        // CallStack cs;
        // cs.update(1, pCommandHandler->getTid());
        // cs.print(printer);

        //This might not work since UnwindCurThreadBT only supports current thread
        std::string callstack;
        UnwindCurThreadBT(&callstack);
        MY_LOGD("%s", callstack.c_str());
    }

    // Async to dump debugging information with a timeout of 1 second.
    std::async(std::launch::async,
        [](auto pDeviceManager)
        {
            LogPrinter printer(LOG_TAG);

            // dump the callstack of this process
            {
            #if 1
                std::string callstack;
                UnwindCurProcessBT(&callstack);
                __android_log_write(ANDROID_LOG_INFO, LOG_TAG, callstack.c_str());
            #endif
            }

            if ( pDeviceManager ) {
                pDeviceManager->debug(std::make_shared<LogPrinter>(LOG_TAG), {});
            }
        },
        mStaticInfo.mDeviceManager
    ).wait_for(std::chrono::seconds(1));

#if MTKCAM_TARGET_BUILD_VARIANT==0
    // Suicide only in user build.
    // Async to commit suicide (kill camerahalserver) after 1 second.
    std::async(std::launch::async, [](){
        std::this_thread::sleep_for(std::chrono::seconds(1));
        CAM_LOGI("commit suicide: kill camerahalserver - raise(SIGINT)");
        ::raise(SIGINT);
        CAM_LOGI("commit suicide: kill camerahalserver - raise(SIGTERM)");
        ::raise(SIGTERM);
        CAM_LOGI("commit suicide: kill camerahalserver - raise(SIGKILL)");
        ::raise(SIGKILL);
    }).wait_for(std::chrono::seconds(0));
#endif
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
onOpenLocked(
    const ::android::sp<V3_4::ICameraDeviceCallback>& callback
) -> ::android::status_t
{
    mLogLevel = getCameraDevice3DebugLogLevel();

    mCameraDeviceCallback = callback;

    //--------------------------------------------------------------------------
    {
        Mutex::Autolock _l(mAppStreamManagerLock);

        if  ( mAppStreamManager != nullptr ) {
            MY_LOGE("mAppStreamManager:%p != 0 while opening", mAppStreamManager.get());
            mAppStreamManager->destroy();
            mAppStreamManager = nullptr;
        }
        mAppStreamManager = IAppStreamManager::create(
            IAppStreamManager::CreationInfo{
                .mInstanceId            = getInstanceId(),
                .mCameraDeviceCallback  = callback,
                .mMetadataProvider      = mStaticInfo.mMetadataProvider,
                .mMetadataConverter     = mStaticInfo.mMetadataConverter,
                .mErrorPrinter          = std::static_pointer_cast<android::Printer>(mAppStreamManagerErrorState),
                .mWarningPrinter        = std::static_pointer_cast<android::Printer>(mAppStreamManagerWarningState),
                .mDebugPrinter          = std::static_pointer_cast<android::Printer>(mAppStreamManagerDebugState),
            }
        );
        if  ( mAppStreamManager == nullptr ) {
            MY_LOGE("IAppStreamManager::create");
            return NO_INIT;
        }
    }
    //--------------------------------------------------------------------------
#if PIPELINE_NEW_ARCH
    {
        Mutex::Autolock _l(mPipelineModelLock);
        auto pPipelineModelMgr = IPipelineModelManager::get();
        if  ( pPipelineModelMgr == nullptr ) {
            MY_LOGE("IPipelineModelManager::get() is null object!");
            return NO_INIT;
        }
        //
        auto pPipelineModel = pPipelineModelMgr->getPipelineModel( getInstanceId() );
        if ( pPipelineModel == nullptr ) {
            MY_LOGE("IPipelineModelManager::getPipelineModel(%d) is null object!", getInstanceId());
            return NO_INIT;
        }
        //
        ::android::status_t err = OK;
        err = pPipelineModel->open(getInstanceName().c_str(), this);
        if  ( OK != err ) {
            MY_LOGE( "fail to IPipelinemodel->open() status:%d(%s)", -err, ::strerror(-err) );
            return NO_INIT;
        }
        mPipelineModel = pPipelineModel;
    }
#else
    {
        Mutex::Autolock _l(mPipelineModelLock);
        if  ( mPipelineModel != nullptr ) {
            MY_LOGE("mPipelineModel:%p != 0 while opening", mPipelineModel.get());
            mPipelineModel = nullptr;
        }
        IPipelineModelManager::AppCreation params;
        params.openId = getInstanceId();
        params.userName = getInstanceName().c_str();
        params.appCallback = this;
        mPipelineModel = IPipelineModelManager::create(params);
        if  ( mPipelineModel == nullptr ) {
            MY_LOGE("IPipelineModelManager::create");
            return NO_INIT;
        }
    }
#endif
    //--------------------------------------------------------------------------
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
onCloseLocked() -> void
{
    ::android::status_t err = OK;

    NSCam::Utils::CamProfile profile(__FUNCTION__, getInstanceName().c_str());

    if  ( ! waitUntilOpenDoneLocked() ) {
        MY_LOGE("open fail");
    }
    //
    {
        disableRequesting();
        flushAndWait();
        profile.print("flush -");
    }
    //
    mCameraDeviceCallback = nullptr;
    //
    {
        Mutex::Autolock _l(mPipelineModelLock);
        if  ( mPipelineModel != nullptr ) {
#if PIPELINE_NEW_ARCH
            mPipelineModel->close();
#endif
            mPipelineModel = nullptr;
            profile.print("PipelineModel -");
        }
    }
    //
    {
        Mutex::Autolock _l(mAppStreamManagerLock);
        if  ( mAppStreamManager != nullptr ) {
            mAppStreamManager->destroy();
            mAppStreamManager = nullptr;
            profile.print("AppStreamManager -");
        }
    }
    //--------------------------------------------------------------------------
    profile.print("");
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
onConfigureStreamsLocked(
    const WrappedStreamConfiguration& requestedConfiguration,
    WrappedHalStreamConfiguration& halConfiguration
) -> ::android::status_t
{
    MY_LOGD("+");

    ::android::status_t err = OK;

    if  ( ! waitUntilOpenDoneLocked() ) {
        return NO_INIT;
    }
    //
    {
        disableRequesting();
        flushAndWait();
    }
    //
    //
    IAppStreamManager::ConfigAppStreams appStreams;
    auto pAppStreamManager = getSafeAppStreamManager();
    if  ( pAppStreamManager == 0 ) {
        MY_LOGE("Bad AppStreamManager");
        return NO_INIT;
    }
    //
    err = pAppStreamManager->beginConfigureStreams(requestedConfiguration, halConfiguration, appStreams);
    if  ( OK != err ) {
        MY_LOGE("fail to beginConfigureStreams");
        return err;
    }
    //
    //
    {
        auto pPipelineModel = getSafePipelineModel();
        if  ( pPipelineModel == 0 ) {
            MY_LOGE("Bad PipelineModel");
            return NO_INIT;
        }
        //
#if PIPELINE_NEW_ARCH
        auto pParams = std::make_shared<UserConfigurationParams>();
        if ( ! pParams ) {
            MY_LOGE("Bad UserConfigurationParams");
            return NO_INIT;
        }
        pParams->operationMode      = static_cast<uint32_t>(((V3_4::StreamConfiguration&)requestedConfiguration).operationMode);
#warning "P Migration..."
        // pParams->sessionParams      =

#define _CLONE_(dst, src) \
            do { \
                dst.clear(); \
                for ( size_t j=0; j<src.size(); ++j) { \
                    dst.emplace( std::make_pair(src.keyAt(j), src.valueAt(j) ) ); \
                } \
            } while (0) \

        _CLONE_(pParams->vImageStreams,         appStreams.vImageStreams);
        _CLONE_(pParams->vMetaStreams,          appStreams.vMetaStreams);
        _CLONE_(pParams->vMinFrameDuration,     appStreams.vMinFrameDuration);
        _CLONE_(pParams->vStallFrameDuration,   appStreams.vStallFrameDuration);

#undef _CLONE_
        //
        err = pPipelineModel->configure(pParams);
#else
        IPipelineModelManager::AppConfiguration params;
        params.operationMode        = static_cast<uint32_t>(((V3_4::StreamConfiguration&)requestedConfiguration).operationMode);
        params.vImageStreams        = appStreams.vImageStreams;
        params.vMetaStreams         = appStreams.vMetaStreams;
        params.vMinFrameDuration    = appStreams.vMinFrameDuration;
        params.vStallFrameDuration  = appStreams.vStallFrameDuration;
        //
        err = pPipelineModel->configure(params);
#endif
        if  ( OK != err ) {
            MY_LOGE("fail to configure pipeline");
            return err;
        }
    }
    //
    //
    err = pAppStreamManager->endConfigureStreams(halConfiguration);
    if  ( OK != err ) {
        MY_LOGE("fail to endConfigureStreams");
        return err;
    }
    //
    //
    enableRequesting();
    MY_LOGD("-");
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
onFlushLocked() -> ::android::status_t
{
    MY_LOGD("");

    ::android::status_t status = OK;

    if  ( ! waitUntilOpenDoneLocked() ) {
        return NO_INIT;
    }
    //
    disableRequesting();
    //
    status = flushAndWait();
    //
    enableRequesting();
    //
    return status;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
onProcessCaptureRequest(
    const hidl_vec<V3_4::CaptureRequest>& requests,
    const hidl_vec<BufferCache>& cachesToRemove,
    uint32_t& numRequestProcessed
) -> ::android::status_t
{
    size_t const requestNum = requests.size();
    uint32_t const requestNo_1st = requests[0].v3_2.frameNumber;

    ::android::status_t err = OK;
#if ! PIPELINE_NEW_ARCH
    ::android::Vector<IPipelineModelManager::AppRequest> pipelineRequests;
#endif
    auto pPipelineModel = getSafePipelineModel();

    ::android::Vector<IAppStreamManager::Request> appRequests;
    auto pAppStreamManager = getSafeAppStreamManager();
    if  ( pAppStreamManager == 0 ) {
        MY_LOGE("Bad AppStreamManager");
        return DEAD_OBJECT;
    }
    //
    {
        Mutex::Autolock _lRequesting(mRequestingLock);
        //
        pAppStreamManager->removeBufferCache(cachesToRemove);
        //
        if  ( 0 == ::android_atomic_acquire_load(&mRequestingAllowed) ) {
            MY_LOGW("submitting requests during flushing - requestNo_1st:%u #:%zu", requestNo_1st, requestNum);
            pAppStreamManager->flushRequest(requests);
            numRequestProcessed = requests.size();
            return OK;
        }
        //
        if  ( pPipelineModel == 0 ) {
            MY_LOGE("Bad PipelineModel");
            return NO_INIT;
        }
        //
        err = pAppStreamManager->submitRequest(requests, appRequests);
        if  ( OK != err ) {
            return err;
        }
    }
    //
#if PIPELINE_NEW_ARCH
#define _CLONE_(dst, src) \
            do  { \
                dst.clear(); \
                for (size_t j = 0; j < src.size(); j++) { \
                    dst.emplace( std::make_pair(src.keyAt(j), src.valueAt(j) ) ); \
                } \
            } while (0) \

    std::vector<std::shared_ptr<UserRequestParams>> vPipelineRequests(appRequests.size());
    for ( size_t i=0; i<appRequests.size(); ++i ) {
        auto& pItem = vPipelineRequests[i];
        pItem = std::make_shared<UserRequestParams>();
        if ( !pItem ) {
            MY_LOGE("Bad UserRequestParams");
            return NO_INIT;
        }
        pItem->requestNo = appRequests[i].frameNo;
        _CLONE_(pItem->vIImageBuffers,    appRequests[i].vInputImageBuffers);
        _CLONE_(pItem->vOImageBuffers,    appRequests[i].vOutputImageBuffers);
        _CLONE_(pItem->vIMetaBuffers,     appRequests[i].vInputMetaBuffers);
    }
#undef  _CLONE_
    //
    //  Since this call may block, it should be performed out of locking.
    err = pPipelineModel->submitRequest(vPipelineRequests, numRequestProcessed);
    if  ( OK != err || requests.size() != numRequestProcessed ) {
        MY_LOGE("%u/%zu requests submitted sucessfully - err:%d(%s)",
            numRequestProcessed, vPipelineRequests.size(), -err, ::strerror(-err));
        return err;
    }
#else
    pipelineRequests.resize(appRequests.size());
    for (size_t i = 0; i < appRequests.size(); i++) {
#define _CLONE_(dst, src) \
            do  { \
                dst.setCapacity(src.size()); \
                dst.clear(); \
                for (size_t j = 0; j < src.size(); j++) { \
                    dst.add(src.keyAt(j), src.valueAt(j)); \
                } \
            } while (0) \

        auto& item = pipelineRequests.editItemAt(i);
                item.requestNo = appRequests[i].frameNo;
        _CLONE_(item.vIImageBuffers, appRequests[i].vInputImageBuffers);
        _CLONE_(item.vOImageBuffers, appRequests[i].vOutputImageBuffers);
        _CLONE_(item.vIMetaBuffers, appRequests[i].vInputMetaBuffers);

#undef  _CLONE_
    }
    //
    //  Since this call may block, it should be performed out of locking.
    err = pPipelineModel->submitRequest(pipelineRequests, numRequestProcessed);
    if  ( OK != err || requests.size() != numRequestProcessed ) {
        MY_LOGE("%u/%zu requests submitted sucessfully - err:%d(%s)",
            numRequestProcessed, pipelineRequests.size(), -err, ::strerror(-err));
        return err;
    }
#endif
    //
    return OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  NSCam::ICameraDevice3Session Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
dumpState(::android::Printer& printer, const std::vector<std::string>& options __unused) -> void
{
    printer.printLine("\n== state transition (most recent at bottom): Camera device ==");
    mStateLog.print(printer);

    printer.printLine("\n== error state (most recent at bottom): App Stream Manager ==");
    mAppStreamManagerErrorState->print(printer);

    printer.printLine("\n== warning state (most recent at bottom): App Stream Manager ==");
    mAppStreamManagerWarningState->print(printer);

    printer.printLine("\n== debug state (most recent at bottom): App Stream Manager ==");
    mAppStreamManagerDebugState->print(printer);

    auto pCommandHandler = mCommandHandler;
    if ( pCommandHandler != nullptr ) {
        pCommandHandler->print(printer);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
open(
    const ::android::sp<V3_4::ICameraDeviceCallback>& callback
) -> ::android::status_t
{
    ::android::status_t status = OK;
    String8 const stateTag("-> open");
    mStateLog.add(stateTag + " +");

    do {

        if (callback == nullptr) {
            MY_LOGE("cannot open camera. callback is null!");
            status = BAD_VALUE;
            break;
        }

        auto pDeviceManager = mStaticInfo.mDeviceManager;
        auto const& instanceName = mStaticInfo.mStaticDeviceInfo->mInstanceName;

        status = pDeviceManager->startOpenDevice(instanceName);
        if  ( OK != status ) {
            break;
        }

        do {
            ::android::Mutex::Autolock _lOpsLock(mOpsLock);

            //------------------------------------------------------------------
            mCommandHandler = new CommandHandler(getInstanceId());
            if  ( mCommandHandler == nullptr ) {
                MY_LOGE("Bad mCommandHandler");
                status = NO_INIT;
                break;
            }
            else {
                const std::string threadName{std::to_string(getInstanceId())+":dev3-cmd"};
                status = mCommandHandler->run(threadName.c_str());
                if  ( OK != status ) {
                    MY_LOGE("Fail to run the thread %s - status:%d(%s)", threadName.c_str(), status, ::strerror(-status));
                    mCommandHandler = nullptr;
                    status = NO_INIT;
                    break;
                }
            }
            //------------------------------------------------------------------
            int err = NO_INIT;
            status = tryRunCommandLocked(kWaitCommandTimeout, "onOpenLocked", [&, this](){
                err = onOpenLocked(callback);
            });
            if ( status == OK ) {
                status = err;
            }
        } while (0);

        pDeviceManager->updatePowerOnDone();

        if  ( OK != status ) {
            pDeviceManager->finishOpenDevice(instanceName, true/*cancel*/);
            break;
        }

        status = pDeviceManager->finishOpenDevice(instanceName, false/*cancel*/);
        if  ( OK != status ) {
            break;
        }

    } while (0);

    mStateLog.add(stateTag + " - " + (0==status ? "OK" : ::strerror(-status)));
    return status;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ICameraDeviceSession Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/******************************************************************************
 *
 ******************************************************************************/
Return<void>
ThisNamespace::
constructDefaultRequestSettings(RequestTemplate type, constructDefaultRequestSettings_cb _hidl_cb)
{
    String8 const stateTag(String8::format("-> constructDefaultRequestSettings (type:%#x)", type));
    mStateLog.add(stateTag + " +");
    MY_LOGD_IF(1, "%s", stateTag.c_str());

    ITemplateRequest* obj = NSTemplateRequestManager::valueFor(getInstanceId());
    if  (obj == nullptr) {
        obj = ITemplateRequest::getInstance(getInstanceId());
        NSTemplateRequestManager::add(getInstanceId(), obj);
    }

    CameraMetadata metadata;
    mStaticInfo.mMetadataConverter->convertToHidl(obj->getMtkData(static_cast<int>(type)), &metadata);

    _hidl_cb(Status::OK, metadata);

    mStateLog.add(stateTag + " -");
    return Void();
}


/******************************************************************************
 *
 ******************************************************************************/
Return<void>
ThisNamespace::
configureStreams(const V3_2::StreamConfiguration& requestedConfiguration, configureStreams_cb _hidl_cb)
{
    CAM_TRACE_CALL();

    ::android::status_t status = OK;
    String8 const stateTag(String8::format("-> configure (operationMode:%#x)", requestedConfiguration.operationMode));
    mStateLog.add(stateTag + " +");

    WrappedHalStreamConfiguration halStreamConfiguration;
    {
        ::android::Mutex::Autolock _lOpsLock(mOpsLock);

        int err = NO_INIT;
        status = tryRunCommandLocked(kWaitCommandTimeout, "onConfigureStreamsLocked", [&, this](){
            err = onConfigureStreamsLocked(requestedConfiguration, halStreamConfiguration);
        });
        if ( status == OK ) {
            status = err;
        }
    }

    _hidl_cb(mapToHidlCameraStatus(status), halStreamConfiguration);

    mStateLog.add(stateTag + " - " + (0==status ? "OK" : ::strerror(-status)));
    return Void();
}

//V3_4
Return<void>
ThisNamespace::
configureStreams_3_3(const V3_2::StreamConfiguration& requestedConfiguration, configureStreams_3_3_cb _hidl_cb)
{
    CAM_TRACE_CALL();

    ::android::status_t status = OK;
    String8 const stateTag(String8::format("-> configure (operationMode:%#x)", requestedConfiguration.operationMode));
    mStateLog.add(stateTag + " +");

    WrappedHalStreamConfiguration halStreamConfiguration;
    {
        ::android::Mutex::Autolock _lOpsLock(mOpsLock);

        int err = NO_INIT;
        status = tryRunCommandLocked(kWaitCommandTimeout, "onConfigureStreamsLocked", [&, this](){
            err = onConfigureStreamsLocked(requestedConfiguration, halStreamConfiguration);
        });
        if ( status == OK ) {
            status = err;
        }
    }

    _hidl_cb(mapToHidlCameraStatus(status), halStreamConfiguration);

    mStateLog.add(stateTag + " - " + (0==status ? "OK" : ::strerror(-status)));
    return Void();
}

Return<void>
ThisNamespace::
configureStreams_3_4(const V3_4::StreamConfiguration& requestedConfiguration, configureStreams_3_4_cb _hidl_cb)
{
    CAM_TRACE_CALL();

    ::android::status_t status = OK;
    String8 const stateTag(String8::format("-> configure (operationMode:%#x)", requestedConfiguration.operationMode));
    mStateLog.add(stateTag + " +");

    WrappedHalStreamConfiguration halStreamConfiguration;
    {
        ::android::Mutex::Autolock _lOpsLock(mOpsLock);

        int err = NO_INIT;
        status = tryRunCommandLocked(kWaitCommandTimeout, "onConfigureStreamsLocked", [&, this](){
            err = onConfigureStreamsLocked(requestedConfiguration, halStreamConfiguration);
        });
        if ( status == OK ) {
            status = err;
        }
    }

    _hidl_cb(mapToHidlCameraStatus(status), halStreamConfiguration);

    mStateLog.add(stateTag + " - " + (0==status ? "OK" : ::strerror(-status)));
    return Void();
}

/******************************************************************************
 *
 ******************************************************************************/
Return<void>
ThisNamespace::
getCaptureRequestMetadataQueue(getCaptureRequestMetadataQueue_cb _hidl_cb)
{
    auto pAppStreamManager = getSafeAppStreamManager();
    if  ( pAppStreamManager != nullptr ) {
        _hidl_cb(pAppStreamManager->getCaptureRequestMetadataQueue());
    }
    else {
        MY_LOGE("Bad AppStreamManager");
        //_hidl_cb();
    }
    return Void();
}


/******************************************************************************
 *
 ******************************************************************************/
Return<void>
ThisNamespace::
getCaptureResultMetadataQueue(getCaptureResultMetadataQueue_cb _hidl_cb)
{
    auto pAppStreamManager = getSafeAppStreamManager();
    if  ( pAppStreamManager != nullptr ) {
        _hidl_cb(pAppStreamManager->getCaptureResultMetadataQueue());
    }
    else {
        MY_LOGE("Bad AppStreamManager");
        //_hidl_cb();
    }
    return Void();
}


/******************************************************************************
 *
 ******************************************************************************/
Return<void>
ThisNamespace::
processCaptureRequest(const hidl_vec<V3_2::CaptureRequest>& requests, const hidl_vec<BufferCache>& cachesToRemove, processCaptureRequest_cb _hidl_cb)
{
    uint32_t numRequestProcessed = 0;
    hidl_vec<V3_4::CaptureRequest> v34Requests(requests.size());
    int i = 0;
    for(auto &r : requests) {
        v34Requests[i++] = (V3_4::CaptureRequest &)WrappedCaptureRequest(r);
    }
    auto status = onProcessCaptureRequest(v34Requests, cachesToRemove, numRequestProcessed);

    MY_LOGD_IF(getLogLevel() >= 2, "- requestNo_1st:%u #:%zu numRequestProcessed:%u", requests[0].frameNumber, requests.size(), numRequestProcessed);

    if  ( m1stRequestNotSent ) {
        if  (OK == status) {
            m1stRequestNotSent = false;
            mStateLog.add("-> 1st request - OK");
            MY_LOGD("-> 1st request - OK");
        }
        else {
            mStateLog.add("-> 1st request - failure");
            MY_LOGE("-> 1st request - failure");
        }
    }

    _hidl_cb(mapToHidlCameraStatus(status), numRequestProcessed);
    return Void();
}

/******************************************************************************
 *
 ******************************************************************************/
Return<void>
ThisNamespace::
processCaptureRequest_3_4(const hidl_vec<V3_4::CaptureRequest>& requests, const hidl_vec<BufferCache>& cachesToRemove, processCaptureRequest_3_4_cb _hidl_cb)
{
    uint32_t numRequestProcessed = 0;
    auto status = onProcessCaptureRequest(requests, cachesToRemove, numRequestProcessed);

    MY_LOGD_IF(getLogLevel() >= 2, "- requestNo_1st:%u #:%zu numRequestProcessed:%u", requests[0].v3_2.frameNumber, requests.size(), numRequestProcessed);

    if  ( m1stRequestNotSent ) {
        if  (OK == status) {
            m1stRequestNotSent = false;
            mStateLog.add("-> 1st request - OK");
            MY_LOGD("-> 1st request - OK");
        }
        else {
            mStateLog.add("-> 1st request - failure");
            MY_LOGE("-> 1st request - failure");
        }
    }

    _hidl_cb(mapToHidlCameraStatus(status), numRequestProcessed);
    return Void();
}

/******************************************************************************
 *
 ******************************************************************************/
Return<Status>
ThisNamespace::
flush()
{
    ::android::status_t status = OK;
    String8 const stateTag("-> flush");
    mStateLog.add(stateTag + " +");
    {
        ::android::Mutex::Autolock _lOpsLock(mOpsLock);

        int err = NO_INIT;
        status = tryRunCommandLocked(kWaitCommandTimeout, "onFlushLocked", [&, this](){
            err = onFlushLocked();
        });
        if ( status == OK ) {
            status = err;
        }
    }
    mStateLog.add(stateTag + " - " + (0==status ? "OK" : ::strerror(-status)));
    return mapToHidlCameraStatus(status);
}


/******************************************************************************
 *
 ******************************************************************************/
Return<void>
ThisNamespace::
close()
{
    ::android::status_t status = OK;
    String8 const stateTag("-> close");
    mStateLog.add(stateTag + " +");

    auto pDeviceManager = mStaticInfo.mDeviceManager;
    auto const& instanceName = mStaticInfo.mStaticDeviceInfo->mInstanceName;

    status = pDeviceManager->startCloseDevice(instanceName);
    if  ( OK != status ) {
        MY_LOGW("finishCloseDevice [%d %s]", -status, ::strerror(-status));
    }

    {
        ::android::Mutex::Autolock _lOpsLock(mOpsLock);

        status = tryRunCommandLocked(kWaitCommandTimeout, "onCloseLocked", [&, this](){
            onCloseLocked();
        });
        mCommandHandler->requestExitAndWait(kExitCommandTimeout);
        mCommandHandler = nullptr;
    }

    status = pDeviceManager->finishCloseDevice(instanceName);
    if  ( OK != status ) {
        MY_LOGW("finishCloseDevice [%d %s]", -status, ::strerror(-status));
    }

    mStateLog.add(stateTag + " - " + (0==status ? "OK" : ::strerror(-status)));
    return Void();
}


#if PIPELINE_NEW_ARCH
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineModelCallback Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
onFrameUpdated(
    pipeline::model::UserOnFrameUpdated const& params
) -> void
{
    CAM_TRACE_CALL();

    String8 const postfix = String8::format("frameNo:%u userId:%#" PRIxPTR " OAppMeta#(left:%zd this:%zu)",
        params.requestNo, params.userId, params.nOutMetaLeft, params.vOutMeta.size());

    MY_LOGD_IF(getLogLevel() >= 2, "+ %s", postfix.string());
    NSCam::Utils::CamProfile profile(__FUNCTION__, "CameraDevice3SessionImpl");

    auto pAppStreamManager = getSafeAppStreamManager();
    if  ( pAppStreamManager == 0 ) {
        MY_LOGE("Bad AppStreamManager");
        return;
    }
    profile.print_overtime(1, "getSafeAppStreamManager: %s", postfix.string());

    Vector<sp<IMetaStreamBuffer>> vResultMeta;
    vResultMeta.setCapacity(params.vOutMeta.size());
    for ( size_t i=0; i<params.vOutMeta.size(); ++i ) {
        vResultMeta.add(params.vOutMeta[i]);
    }

    pAppStreamManager->updateResult( params.requestNo, params.userId,
                                     vResultMeta, params.nOutMetaLeft<=0);

    profile.print_overtime(1, "updateResult: %s", postfix.string());
    MY_LOGD_IF(getLogLevel() >= 2, "- %s", postfix.string());
}
#else
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineModelManager::IAppCallback Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
onFrameUpdated(
    uint32_t const requestNo,
    intptr_t const userId,
    ssize_t const nOutMetaLeft,
    android::Vector<android::sp<IMetaStreamBuffer> > vOutMeta
) -> void
{
    CAM_TRACE_CALL();

    String8 const postfix = String8::format("frameNo:%u userId:%#" PRIxPTR " OAppMeta#(left:%zd this:%zu)", requestNo, userId, nOutMetaLeft, vOutMeta.size());

    MY_LOGD_IF(getLogLevel() >= 2, "+ %s", postfix.string());
    NSCam::Utils::CamProfile profile(__FUNCTION__, "CameraDevice3SessionImpl");

    auto pAppStreamManager = getSafeAppStreamManager();
    if  ( pAppStreamManager == 0 ) {
        MY_LOGE("Bad AppStreamManager");
        return;
    }
    profile.print_overtime(1, "getSafeAppStreamManager: %s", postfix.string());

    pAppStreamManager->updateResult(requestNo, userId, vOutMeta, nOutMetaLeft <= 0);

    profile.print_overtime(1, "updateResult: %s", postfix.string());
    MY_LOGD_IF(getLogLevel() >= 2, "- %s", postfix.string());
}
#endif

