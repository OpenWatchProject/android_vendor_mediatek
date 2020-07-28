/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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
#define __DEBUG // enable debug
// #define __SCOPE_TIMER // enable log of scope timer

#define LOG_TAG "MtkCam/BaseController"
static const char* __CALLERNAME__ = LOG_TAG;

#include "BaseController.h"
#include "VendorUtils.h"

// MTKCAM
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Trace.h>
#include <mtkcam/utils/hw/HwInfoHelper.h>
#include <mtkcam/utils/exif/DebugExifUtils.h>
#include <mtkcam/aaa/INvBufUtil.h>
#if MTK_CAM_NEW_NVRAM_SUPPORT
#include <mtkcam/utils/mapping_mgr/cam_idx_mgr.h>
#endif
// faces
#include <mtkcam/utils/hw/IFDContainer.h>
// dpframework
#include <DpDataType.h>

// CUSTOM
#include <camera_custom_nvram.h>

// AOSP
#include <cutils/compiler.h>
#include <cutils/properties.h>
#include <sys/stat.h>
// STL
#include <chrono>
#include <future>
#include <cassert> // assert
#include <tuple>

#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
// FUNCTION_SCOPE
#ifdef __DEBUG
#define FUNCTION_SCOPE(caller)      auto __scope_logger__ = create_scope_logger(caller, __FUNCTION__)
#define LOG_SCOPE(caller, log)      auto __scope_logger__ = create_scope_logger(caller, log)
#include <memory>
#include <functional>
static std::shared_ptr<char> create_scope_logger(const char* callerName, const char* functionName)
{
    char* pText = const_cast<char*>(functionName);
    CAM_LOGD("[%s] %s + ", callerName, pText);
    return std::shared_ptr<char>(pText, [callerName](char* p){ CAM_LOGD("[%s] %s -", callerName, p); });
}
#else
#define FUNCTION_SCOPE          do{}while(0)
#define LOG_SCOPE               do{}while(0)
#endif
// SCOPE_TIMER
#ifdef __SCOPE_TIMER
#define SCOPE_TIMER(VAR, TEXT)  auto VAR = create_scope_timer(TEXT)
#include <memory>
#include <functional>
#include <utils/SystemClock.h> // elapsedRealtime(), uptimeMillis()
static std::unique_ptr<const char, std::function<void(const char*)> >
create_scope_timer(const char* text)
{
    auto t1 = android::elapsedRealtimeNano();
    return std::unique_ptr<const char, std::function<void(const char*)> >
        (
            text,
            [t1](const char* t)->void
            {
                auto t2 = android::elapsedRealtimeNano();
                MY_LOGD("%s --> duration(ns): %" PRId64 "", t, (t2 -t1));
            }
        );
}
#else
#define SCOPE_TIMER(VAR, TEXT)  do{}while(0)
#endif

#define BASE_CONTROLLER_DUMP_KEY  "debug.basecontroller.dump"
#define BASE_CONTROLLER_DUMP_PATH "/sdcard/camera_dump/"
#define BASE_CONTROLLER_DUMP_FILE_NAME_PREFIX \
    "{UNIQUEKEY}-{FRAMENO}-{REQUESTNO}-basecontroller-{CALLER}-iso-{ISO}-exp-{EXP}-{STAGE}-{BUFNAME}__{WIDTH}x{HEIGHT}.{EXT}"

#if MTK_CAM_NEW_NVRAM_SUPPORT
template <NSIspTuning::EModule_T module>
inline void* _getNVRAMBuf(NVRAM_CAMERA_FEATURE_STRUCT* /*pNvram*/, size_t /*idx*/)
{
    MY_LOGE("_getNVRAMBuf: unsupport module(%d)", module);
    *(volatile uint32_t*)(0x00000000) = 0xDEADC0DE;
    return nullptr;
}

template<>
inline void* _getNVRAMBuf<NSIspTuning::EModule_CA_LTM>(
        NVRAM_CAMERA_FEATURE_STRUCT*    pNvram,
        size_t                          idx
        )
{
    return &(pNvram->CA_LTM[idx]);
}

template<>
inline void* _getNVRAMBuf<NSIspTuning::EModule_ClearZoom>(
        NVRAM_CAMERA_FEATURE_STRUCT*    pNvram,
        size_t                          idx
        )
{
    return &(pNvram->ClearZoom[idx]);
}

/* describes the tuple indexes of getTuningFromNvram */
inline constexpr size_t I_NVRAM_BUFFER() { return 0; }
inline constexpr size_t I_NVRAM_INDEX()  { return 1; }

// magicNo is from HAL metadata of MTK_P1NODE_PROCESSOR_MAGICNUM <MINT32>
template<NSIspTuning::EModule_T module>
inline std::tuple<void*, int>
getTuningFromNvram(MUINT32 openId, MUINT32 idx, MINT32 magicNo)
{
    CAM_TRACE_BEGIN("getNvram");

    NVRAM_CAMERA_FEATURE_STRUCT *pNvram;
    void* pNRNvram              = nullptr;
    std::tuple<void*, int>      emptyVal(nullptr, -1);

    if (__builtin_expect( idx >= EISO_NUM, false )) {
        MY_LOGE("wrong nvram idx %d", idx);
        return emptyVal;
    }

    // load some setting from nvram
    MUINT sensorDev = MAKE_HalSensorList()->querySensorDevIdx(openId);
    IdxMgr* pMgr = IdxMgr::createInstance(static_cast<ESensorDev_T>(sensorDev));
    CAM_IDX_QRY_COMB rMapping_Info;

    // query mapping info
    MY_LOGD("getMappingInfo with magicNo(%d)", magicNo);
    pMgr->getMappingInfo(static_cast<ESensorDev_T>(sensorDev), rMapping_Info, magicNo);

    // query NVRAM index by mapping info
    idx = pMgr->query(static_cast<ESensorDev_T>(sensorDev), module, rMapping_Info, __FUNCTION__);
    MY_LOGD("query nvram DRE mappingInfo index: %d", idx);

    auto pNvBufUtil = MAKE_NvBufUtil();
    if (__builtin_expect( pNvBufUtil == NULL, false )) {
        MY_LOGE("pNvBufUtil==0");
        return emptyVal;
    }

    auto result = pNvBufUtil->getBufAndRead(
        CAMERA_NVRAM_DATA_FEATURE,
        sensorDev, (void*&)pNvram);
    if (__builtin_expect( result != 0, false )) {
        MY_LOGE("read buffer chunk fail");
        return emptyVal;
    }

    pNRNvram = _getNVRAMBuf<module>(pNvram, static_cast<size_t>(idx));
    CAM_TRACE_END();
    return std::make_tuple( pNRNvram, idx );
}
#endif


using namespace NSCam;
using namespace NSCam::plugin;
using namespace NSCamHW;

wp<ScenarioCtrl> BaseController::m_wpScenarioCtrl = nullptr;

// ----------------------------------------------------------------------------
// Constructor(s)/Destructor
// ----------------------------------------------------------------------------
BaseController::BaseController(const char* callerName)
    : BaseController(callerName, SENSOR_SCENARIO_ID_NORMAL_CAPTURE)
{
}

BaseController::BaseController(const char* callerName, const MUINT32 sensorMode)
    : m_enquedTimes(0)
    , m_callerName(__CALLERNAME__)
    , m_sensorMode(sensorMode)
    , m_bInvalidated(false)
    , m_frameNumber(0)
    , m_spScenarioCtrl(nullptr)
    , m_dumpFlag(0)
{
    if (callerName) m_callerName = callerName;

    m_dumpFlag = ::property_get_int32(BASE_CONTROLLER_DUMP_KEY, 0);
    if( m_dumpFlag ) {
        MY_LOGD("enable dump flag 0x%X", m_dumpFlag);
        MINT32 err = mkdir(BASE_CONTROLLER_DUMP_PATH, S_IRWXU | S_IRWXG | S_IRWXO);
        if(err != 0 && err != EEXIST) {
            MY_LOGE("mkdir failed: %d", err);
        }
    }
}


BaseController::~BaseController()
{
    FUNCTION_SCOPE(m_callerName);
    try {
        exitCaptureScenario();
    }
    catch (std::exception&) {
        MY_LOGE("exitCaptureScenario failed");
#if (MTKCAM_LOG_LEVEL_DEFAULT >= 3)
        // exitCaptureScenario may throw exception, however, if the exception has been thrown, it's
        // okay indeed in userload. Hence we invoke assert while log level >= 3
        assert(0);
#endif
    }
}


// ----------------------------------------------------------------------------
// Assignment operators
// ----------------------------------------------------------------------------
BaseController& BaseController::operator = (BaseController&& other)
{
    if (CC_UNLIKELY(this == &other))
        return *this;

    //
    // Step 1: using temporary variables to save the variables of other
    //         and clear or reset other's variables.
    //
    other.m_opAssignmentMx.lock();
    // {{{
    auto __callername = other.m_callerName;
    other.m_callerName = __CALLERNAME__;
    // StreamID Maps
    other.m_streamIdMapMx.lock();
    auto __streamIdMap_img  = std::move(other.m_streamIdMap_img);
    auto __streamIdMap_meta = std::move(other.m_streamIdMap_meta);
    other.m_streamIdMapMx.unlock();
    // Message callback
    other.m_messageCbMx.lock();
    wp<INotifyCallback> __messageCb[NOTIFY_CALLBACK_MAX_SIZE];
    for (MUINT i = 0; i < NOTIFY_CALLBACK_MAX_SIZE; i++)
    {
       __messageCb[i] = other.m_messageCb[i];
       other.m_messageCb[i] = nullptr;
    }
    other.m_messageCbMx.unlock();
    // RequestFrame
    other.m_incomingFramesMx.lock();
    other.m_invalidateMx.lock();
    auto __requestFrames = std::move(m_incomingFrames);
    auto __bInvalidated  = other.m_bInvalidated;
    auto __enquedTimes   = other.m_enquedTimes;
    other.m_bInvalidated = false;
    other.m_enquedTimes = 0;
    other.m_invalidateMx.unlock();
    other.m_incomingFramesMx.unlock();
    // Request numbers
    other.m_requestNoStackMx.lock();
    auto __requestNoStack = std::move(other.m_requestNoStack);
    other.m_requestNoStackMx.unlock();
    // Future execution
    other.m_futureExeMx.lock();
    auto __futureExe = other.m_futureExe;
    other.m_futureExeMx.unlock();
    // }}}
    other.m_opAssignmentMx.unlock();

    //
    // Step 2: move temporary variable to this
    //
    this->m_opAssignmentMx.lock();
    // {{{
    this->m_callerName = __callername;
    // StreamID Map
    this->m_streamIdMapMx.lock();
    this->m_streamIdMap_img  = std::move(__streamIdMap_img);
    this->m_streamIdMap_meta = std::move(__streamIdMap_meta);
    this->m_streamIdMapMx.unlock();
    // Message callback
    this->m_messageCbMx.lock();
    for (MUINT i = 0; i < NOTIFY_CALLBACK_MAX_SIZE; i++)
    {
        this->m_messageCb[i] = __messageCb[i];
    }
    this->m_messageCbMx.unlock();
    // RequestFrame
    this->m_incomingFramesMx.lock();
    this->m_invalidateMx.lock();
    auto __trashRequestFrames = std::move(this->m_incomingFrames); // reduce duration of mutex locking
    this->m_incomingFrames = std::move(__requestFrames);
    this->m_bInvalidated = __bInvalidated;
    this->m_enquedTimes = __enquedTimes;
    this->m_invalidateMx.unlock();
    this->m_incomingFramesMx.unlock();
    // Request numbers
    this->m_requestNoStackMx.lock();
    this->m_requestNoStack = std::move(__requestNoStack);
    this->m_requestNoStackMx.unlock();
    // Future execution
    this->m_futureExeMx.lock();
    auto __trashFutureExe = this->m_futureExe;
    this->m_futureExe = __futureExe;
    this->m_futureExeMx.unlock();
    // }}}
    this->m_opAssignmentMx.unlock();

    // clear trashes: we don't need to clear trashes but invoke the destructors
    // while exiting this scope.
    // __trashRequestFrames.clear();
    // __trashFutureExe = nullptr;

    return *this;
}


bool BaseController::operator == (const BaseController& other)
{
    return (this == &other);
}


bool BaseController::operator != (const BaseController& other)
{
    return (this != &other);
}


// ----------------------------------------------------------------------------
// Methods
// ----------------------------------------------------------------------------

bool BaseController::isBelong(MUINT32 requestNo) const
{
    std::lock_guard<T_MUTEX> __l(m_requestNoStackMx);
    auto itr = std::find(m_requestNoStack.begin(), m_requestNoStack.end(), requestNo);
    return (itr != m_requestNoStack.end());
}

bool BaseController::isEmptyBelong() const
{
    std::lock_guard<T_MUTEX> __l(m_requestNoStackMx);
    return m_requestNoStack.empty();
}

bool BaseController::isInvalidated() const
{
    std::lock_guard<T_MUTEX> __l(m_invalidateMx);
    return m_bInvalidated;
}


void BaseController::setBelong(MUINT32 requestNo)
{
    std::lock_guard<T_MUTEX> __l(m_requestNoStackMx);
    m_requestNoStack.push_back(requestNo);
}


void BaseController::setMessageCallback(wp<INotifyCallback> cb, MUINT idx /* = 0 */)
{
    std::lock_guard<T_MUTEX> __l(m_messageCbMx);
    if (idx >= NOTIFY_CALLBACK_MAX_SIZE)
    {
        return;
    }
    m_messageCb[idx] = cb;
}

void BaseController::clearBelong(MUINT32 requestNo)
{
    std::lock_guard<T_MUTEX> __l(m_requestNoStackMx);
    auto itr = std::find(m_requestNoStack.begin(), m_requestNoStack.end(), requestNo);
    m_requestNoStack.erase(itr);
}

void BaseController::clearBelong()
{
    std::deque<MUINT32> lRequestNo;
    {
        std::lock_guard<T_MUTEX> __l(m_requestNoStackMx);
        lRequestNo = std::move(m_requestNoStack);
    }
}


int BaseController::enqueFrame(
        const MINT32 openId,
        MUINT32 requestNo,
        const IVendor::BufferParam& bufParam,
        const IVendor::MetaParam& metaParam,
        wp<IVendor::IDataCallback> cb)
{
    std::lock_guard<std::mutex> __oplocker(m_funcEnqueFrameMx);
    std::lock_guard<std::mutex> __ll(m_incomingFramesMx);
    std::lock_guard<T_MUTEX>    __l(m_invalidateMx);
    //
    std::lock_guard<std::mutex> __L(m_streamIdMapMx);

    if (m_bInvalidated) // invalidated, return failed
        return -1;

    std::shared_ptr<RequestFrame> pFrame(
            new RequestFrame(
                openId,
                m_callerName,
                bufParam,
                metaParam,
                cb,
                m_streamIdMap_img,
                m_streamIdMap_meta,
                requestNo,
                m_enquedTimes
                )
            );

    // saves frame
    m_incomingFrames.push_back(pFrame);
    ++m_enquedTimes;
    m_incomingFramesCond.notify_one();

    return 0;
}


int BaseController::enqueFrame(std::shared_ptr<RequestFrame> frame)
{
    std::lock_guard<std::mutex> __oplocker(m_funcEnqueFrameMx);
    std::lock_guard<std::mutex> __ll(m_incomingFramesMx);
    std::lock_guard<T_MUTEX>    __l(m_invalidateMx);
    if (CC_LIKELY(!m_bInvalidated)) {
        // saves frame
        m_incomingFrames.push_back(frame);
        ++m_enquedTimes;
        m_incomingFramesCond.notify_one();
    }
    else {
        return -1;
    }
    return 0;
}


std::shared_ptr<RequestFrame> BaseController::dequeFrame()
{
    std::unique_lock<std::mutex> locker(m_incomingFramesMx);

    {
        std::lock_guard<T_MUTEX> __l(m_invalidateMx);
        if (CC_UNLIKELY(m_bInvalidated))
            return nullptr;
    }

    if (m_incomingFrames.size() <= 0) {
        m_incomingFramesCond.wait(locker);
    }

    if (m_incomingFrames.empty())
        return nullptr;

    auto rVal = m_incomingFrames.front();
    m_incomingFrames.pop_front();

    return rVal;
}


std::cv_status BaseController::dequeFrame(
        std::shared_ptr<RequestFrame>& frame,
        int timeoutMs)
{
    std::cv_status status = std::cv_status::no_timeout;

    std::unique_lock<std::mutex> locker(m_incomingFramesMx);
    {
        // if controller has been
        std::lock_guard<T_MUTEX> __l(m_invalidateMx);
        if (m_bInvalidated)
            return std::cv_status::no_timeout;
    }


    if (m_incomingFrames.size() <= 0) {
        status = m_incomingFramesCond.wait_for(locker, std::chrono::milliseconds(timeoutMs));
    }

    if (status == std::cv_status::no_timeout) {
        if (m_incomingFrames.empty())
            return status;

        frame = m_incomingFrames.front();
        m_incomingFrames.pop_front();
    }
    else {
        frame = nullptr;
    }

    return status;
}


void BaseController::clearFrames()
{
    std::lock_guard<std::mutex> locker(m_incomingFramesMx);
    m_incomingFrames.clear();
}


void BaseController::invalidate()
{
    std::deque< std::shared_ptr<RequestFrame> > lFrames;
    {
        std::lock_guard<std::mutex> __oplocker(m_funcEnqueFrameMx);
        std::lock_guard<std::mutex> __l1(m_incomingFramesMx);
        std::lock_guard<T_MUTEX> __l2(m_invalidateMx);
        m_bInvalidated = true;
        lFrames = std::move(m_incomingFrames);
        m_incomingFramesCond.notify_all();
    }
}


void BaseController::validate()
{
    std::lock_guard<std::mutex> __oplocker(m_funcEnqueFrameMx);
    std::lock_guard<std::mutex> __l1(m_incomingFramesMx);
    std::lock_guard<T_MUTEX> __l2(m_invalidateMx);
    m_bInvalidated = false;
}


void BaseController::reset()
{
    BaseController __new;
    *this = std::move(__new);
}

bool BaseController::execute(
        intptr_t    arg1    /* = 0 */       __attribute__((unused)),
        intptr_t    arg2    /* = 0 */       __attribute__((unused)))
{
    FUNCTION_SCOPE(m_callerName);
    SCOPE_TIMER(__tmr1, "BaseController::execute");

    std::lock_guard<std::mutex> __l(m_futureExeMx);

    bool bStart = false; // flag represents if it's need to start a job

    // std::shared_future::valid
    if (m_futureExe.valid()) {
        switch (m_futureExe.wait_for(std::chrono::seconds(0))) {
        case std::future_status::deferred: // Job hasn't been executed yet.
        case std::future_status::ready: // Job has finished.
            bStart = true; // It's ok to start a new job.
            break;
        case std::future_status::timeout: // Job is still in executing, return false.
            return false;
        default:;
        }
    }
    else {
        bStart = true;
    }

    if (bStart) {
        // start a new job
        auto fu = std::async(std::launch::async, [this, arg1, arg2]() mutable {
            auto r = this->job(arg1, arg2);
            arg1 = 0;
            arg2 = 0;
            return r;
        });

        m_futureExe = std::shared_future<intptr_t>(std::move(fu));

        return true;
    }
    return false;
}


bool BaseController::waitExecution(intptr_t* result /* = nullptr */)
{
    FUNCTION_SCOPE(m_callerName);
    SCOPE_TIMER(__tmr1, "BaseController::waitExecution");

    std::shared_future<intptr_t> t1;
    {
        std::lock_guard<std::mutex> __l(m_futureExeMx);
        t1 = m_futureExe;
    }

    if (t1.valid()) {
        if (result) (*result) = t1.get();
        else        t1.wait();
        return true;
    }
    else {
        return false;
    }

    return false;
}


BaseController::ExecutionStatus
BaseController::getExecutionStatus() const
{
    FUNCTION_SCOPE(m_callerName);
    SCOPE_TIMER(__tmr1, "BaseController::getExecutionStatus");

    std::lock_guard<std::mutex> __l(m_futureExeMx);
    if (CC_LIKELY(m_futureExe.valid())) {
        switch (m_futureExe.wait_for(std::chrono::seconds(0))) {
        case std::future_status::deferred:
            return ES_NOT_STARTED_YET;
        case std::future_status::timeout:
            return ES_RUNNING;
        case std::future_status::ready:
            return ES_READY;
        }
    }
    return ES_NOT_STARTED_YET;
}


intptr_t BaseController::job(
        intptr_t    arg1 /* = 0 */      __attribute__((unused)),
        intptr_t    arg2 /* = 0 */      __attribute__((unused)))
{
    return 0;
}

void BaseController::doCancel()
{
    invalidate();
}

void BaseController::onShutter()
{
    FUNCTION_SCOPE(m_callerName);

    sp<INotifyCallback> spCb[NOTIFY_CALLBACK_MAX_SIZE];
    {
        std::lock_guard<T_MUTEX> __l(m_messageCbMx);
        for (MUINT i = 0; i < NOTIFY_CALLBACK_MAX_SIZE; i++)
        {
            spCb[i] = m_messageCb[i].promote();
        }
    }

    for (MUINT i = 0; i < NOTIFY_CALLBACK_MAX_SIZE; i++)
    {
        if (CC_LIKELY(spCb[i].get()))
            spCb[i]->onMsgReceived(MSG_ON_SHUTTER_CALLBACK);
    }
}


void BaseController::onNextCaptureReady()
{
    FUNCTION_SCOPE(m_callerName);

    sp<INotifyCallback> spCb[NOTIFY_CALLBACK_MAX_SIZE];
    {
        std::lock_guard<T_MUTEX> __l(m_messageCbMx);
        for (MUINT i = 0; i < NOTIFY_CALLBACK_MAX_SIZE; i++)
        {
            spCb[i] = m_messageCb[i].promote();
        }
    }

    for (MUINT i = 0; i < NOTIFY_CALLBACK_MAX_SIZE; i++)
    {
        if (CC_LIKELY(spCb[i].get()))
            spCb[i]->onMsgReceived(MSG_ON_NEXT_CAPTURE_READY);
    }
}


bool BaseController::setMetadtaToResultFrame(
        const std::shared_ptr<RequestFrame> pMainFrame,
        std::shared_ptr<RequestFrame> pResultFrame)
{
    FUNCTION_SCOPE(__CALLERNAME__);
    SCOPE_TIMER(__tmr1, "BaseController::setMetadtaToResultFrame");

    if (CC_UNLIKELY(pMainFrame.get() == nullptr)) {
        MY_LOGE("main frame is NULL, cannot set result frame");
        return false;
    }
    if (CC_UNLIKELY(pResultFrame.get() == nullptr)) {
        MY_LOGE("result frame is NULL, cannot set result frame");
        return false;
    }

    auto pMainApp = pMainFrame->getMetadata(RequestFrame::eRequestMeta_InAppRequest);
    auto pMainHal = pMainFrame->getMetadata(RequestFrame::eRequestMeta_InHalP1);
    auto pResultApp = pResultFrame->getMetadata(RequestFrame::eRequestMeta_OutAppResult);
    auto pResultHal = pResultFrame->getMetadata(RequestFrame::eRequestMeta_OutHalResult);

    if (CC_UNLIKELY(pMainApp == nullptr)) {
        MY_LOGE("IMetadata of main app is NULL");
        return false;
    }
    if (CC_UNLIKELY(pMainHal == nullptr)) {
        MY_LOGE("IMetadata of main hal is NULL");
        return false;
    }
    if (CC_UNLIKELY(pResultApp == nullptr)) {
        MY_LOGE("IMetadata of result app is NULL");
        return false;
    }
    if (CC_UNLIKELY(pResultHal == nullptr)) {
        MY_LOGE("IMetadata of result hal is NULL");
        return false;
    }

    *pResultApp = *pMainApp;
    *pResultHal = *pMainHal;

    return true;
}

bool BaseController::handleYuvOutput(
        std::string msg,
        IImageBuffer*   pResult,
        IImageBuffer*   pJpegYuv,
        IImageBuffer*   pThumnYuv,
        IMetadata*      pAppIn,
        IMetadata*      pHalIn,
        int openId,
        int outOrientaion,
        int requestNo,
        bool needClearZoom,
        bool needCrop1,
        bool needCrop2,
        IMetadata* pMetadataExif
        )
{
    if (pJpegYuv == nullptr) {
        MY_LOGE("output YUV is NULL");
        return false;
    }

    if (pThumnYuv == nullptr) {
        MY_LOGD("thumbnail YUV is NULL, no need to generated");
    }

    if (pResult == nullptr) {
        MY_LOGE("not src.");
        return false;
    }

    static bool needDre   = !! ::property_get_int32("camera.mdp.dre.enable", 0);
    static bool supportCZ = !! ::property_get_int32("camera.mdp.cz.enable",  0);

    needClearZoom = needClearZoom && supportCZ;

    MY_LOGD("%s", msg.c_str());

    MRect rectSrc = [&](){
        return MRect(MPoint(0, 0), pResult->getImgSize());
    }();

    MRect rectDst1 = [&](){
        return (outOrientaion & eTransform_ROT_90)
            ? MRect(MPoint(0, 0), MSize(pJpegYuv->getImgSize().h, pJpegYuv->getImgSize().w))
            : MRect(MPoint(0, 0), pJpegYuv->getImgSize())
            ;
    }();

    MRect rectDst2 = [&](){
        if (pThumnYuv == nullptr)
            return MRect();
        return MRect(MPoint(0, 0), pThumnYuv->getImgSize());
    }();

    // calculate the main yuv output region
    if (needCrop1)
        rectDst1 = calCropRegin(pAppIn, pHalIn, rectSrc.s, rectDst1.s, openId);

    if (CC_UNLIKELY(rectDst1.s.w > rectSrc.s.w)) {
        MY_LOGW("Crop width is out of image source range! rectSrc(%dx%d), rectDst1(%dx%d)",
                rectSrc.s.w ,rectSrc.s.h, rectDst1.s.w, rectDst1.s.h);
        rectDst1.s.w = rectSrc.s.w;
    }
    if (CC_UNLIKELY(rectDst1.s.h > rectSrc.s.h)) {
        MY_LOGW("Crop height is out of image source range! rectSrc(%dx%d), rectDst1(%dx%d)",
                rectSrc.s.w ,rectSrc.s.h, rectDst1.s.w, rectDst1.s.h);
        rectDst1.s.h = rectSrc.s.h;
    }

    // if destination 2 exists, calcaulte pillarboxing or letterboxing crop window
    // based on output1.
    if(pThumnYuv && needCrop2) {
        MRect rectDst1_for_thumb = rectDst1;
        if (!needCrop1) {
            MY_LOGD("generate thumbnail's view angle in this stage, even though main yuv do not crop yet");
            rectDst1_for_thumb = calCropRegin(pAppIn, pHalIn, rectSrc.s, rectDst1.s, openId);
            MY_LOGD("rectDst1_for_thumb=(%d,%d, %dx%d)",
                    rectDst1_for_thumb.p.x, rectDst1_for_thumb.p.y,
                    rectDst1_for_thumb.s.w, rectDst1_for_thumb.s.h);
        }
        rectDst2 = calCrop(rectDst1_for_thumb, rectDst2);
    }

    MY_LOGD("rectSrc =(%d,%d, %dx%d)", rectSrc.p.x,  rectSrc.p.y,  rectSrc.s.w,  rectSrc.s.h);
    MY_LOGD("rectDst1=(%d,%d, %dx%d)", rectDst1.p.x, rectDst1.p.y, rectDst1.s.w, rectDst1.s.h);
    MY_LOGD("rectDst2=(%d,%d, %dx%d)", rectDst2.p.x, rectDst2.p.y, rectDst2.s.w, rectDst2.s.h);

    // create JPEG YUV and thumbnail YUV using MDP (IImageTransform)
    std::unique_ptr<IImageTransform, std::function<void(IImageTransform*)>> transform(
            IImageTransform::createInstance(), // constructor
            [](IImageTransform *p){ if (p) p->destroyInstance(); } // deleter
            );

    if (transform.get() == nullptr) {
        MY_LOGE("IImageTransform is NULL, cannot generate output");
        return false;
    }

    /* describes EXIF buffer, updated or not */
    MDPSetting                  myMdpSetting;
    std::unique_ptr<char[]>     pMdpExifBuffer;
    // declare fdData for unlock
    vector<FD_DATATYPE*> fdData;
    if (needClearZoom || needDre) {
        CAM_TRACE_NAME("ClearZoom set PQParam");
        LOG_SCOPE(m_callerName, "prepare/set clear zoom PQParam config");

        // query info for MDP
        MINT32 uniqueKey    = 0; // pipeline uniqueKey, it's a system timestamp
        MINT32 frameNum     = 0; // pipeline frame number
        MINT32 iso          = 0; // the ISO of the frame
        MINT32 magicNum     = 0; // P1 magic number, for tuning mapping usage
        MINT32 frameCount   = 0; // indicates multi frame feature or single
        MINT32 lv_value     = 0; // indicates brightness level
        MINT64 p1timestamp  = 0;

        if (CC_LIKELY(pHalIn != nullptr)) {
            IMetadata exifMeta;
            if (!IMetadata::getEntry<IMetadata>(const_cast<IMetadata*>(pHalIn), MTK_3A_EXIF_METADATA, exifMeta)) {
                MY_LOGW("no MTK_3A_EXIF_METADATA from HalMetaData");
            }
            else {
                if (!IMetadata::getEntry<MINT32>(&exifMeta, MTK_3A_EXIF_AE_ISO_SPEED, iso)) {
                    MY_LOGW("no MTK_3A_EXIF_AE_ISO_SPEED from HalMetaData");
                }
            }

            if (!IMetadata::getEntry<MINT32>(const_cast<IMetadata*>(pHalIn), MTK_PIPELINE_UNIQUE_KEY, uniqueKey))
            {
                MY_LOGW("no MTK_PIPELINE_UNIQUE_KEY from HalMetaData");
            }

            if (!IMetadata::getEntry<MINT32>(const_cast<IMetadata*>(pHalIn), MTK_PIPELINE_FRAME_NUMBER, frameNum))
            {
                MY_LOGW("no MTK_PIPELINE_FRAME_NUMBER from HalMetaData");
            }

            if (!IMetadata::getEntry<MINT32>(const_cast<IMetadata*>(pHalIn), MTK_PLUGIN_PROCESSED_FRAME_COUNT, frameCount))
            {
                MY_LOGW("no MTK_CLEARZOOM_FRAME_COUNT from HalMetaData");
            }
            if (!IMetadata::getEntry<MINT32>(const_cast<IMetadata*>(pHalIn), MTK_P1NODE_PROCESSOR_MAGICNUM, magicNum))
            {
                MY_LOGW("no MTK_P1NODE_PROCESSOR_MAGICNUM from HalMetaData");
            }
            if (!IMetadata::getEntry<MINT32>(const_cast<IMetadata*>(pHalIn), MTK_REAL_LV, lv_value))
            {
                MY_LOGW("no MTK_REAL_LV from HalMetaData, assume to 0");
            }
            if (!IMetadata::getEntry<MINT64>(const_cast<IMetadata*>(pHalIn), MTK_P1NODE_FRAME_START_TIMESTAMP, p1timestamp))
            {
                MY_LOGW("no MTK_P1NODE_FRAME_START_TIMESTAMP from HalMetaData");
            }
        }

        //
        // Configure IImageTransform
        //

        IImageTransform::PQParam config;

        // apply PQ: ClearZoom | DRE
        config.type =
            (needClearZoom ? IImageTransform::PQType::ClearZoom : 0)
            |
            (needDre ? IImageTransform::PQType::DRE : 0)
            ;

        config.enable       = MTRUE;
        config.portIdx      = 0; // DST_BUF_0
        config.sensorId     = openId;
        config.iso          = iso;
        config.timestamp    = uniqueKey;
        config.frameNo      = frameNum;
        config.requestNo    = requestNo;
        config.lv_value     = lv_value;
        // fd info
        int64_t dumpFd = ::property_get_int64("camera.debug.fd.dump",  0);
        {
            auto fdReader = IFDContainer::createInstance(LOG_TAG,  IFDContainer::eFDContainer_Opt_Read);
            MINT64 tolerence = dumpFd>100 ? dumpFd : 600000000; // 600ms
            fdData = fdReader->queryLock(p1timestamp-tolerence, p1timestamp);
            for(int i=((int)fdData.size()-1);i>=0;i--){
                if(fdData[i]!=nullptr) {
                  config.p_faceInfor = (void*)(&fdData[i]->facedata);
                  if(dumpFd==1)
                    fdReader->dumpInfo();
                  break;
                }
            }
            if(config.p_faceInfor == nullptr) {
                 MY_LOGW("can't query facedata ts:%" PRId64 "", p1timestamp);
                 fdReader->dumpInfo();
            }
        }
        if (frameCount > 1) {
            config.mode = IImageTransform::Mode::Capture_Multi;
        }
        else {
            config.mode = IImageTransform::Mode::Capture_Single;
        }
        MY_LOGD("sensorId:%d mode:%d(frameCount:%d) ISO:%d Timestamp:%d FrameNo:%d RequestNo:%d LV:%d p1ts:%" PRId64 "",
                    config.sensorId,
                    config.mode, frameCount,
                    config.iso,
                    config.timestamp,
                    config.frameNo,
                    config.requestNo,
                    config.lv_value,
                    p1timestamp);

        /* create MDP EXIF buffer */
        pMdpExifBuffer      = std::make_unique<char[]>(MDPSETTING_MAX_SIZE); // allocate buffer
        myMdpSetting.size   = MDPSETTING_MAX_SIZE;
        myMdpSetting.buffer = static_cast<void*>( pMdpExifBuffer.get() );

        /* attach to MDP setting */
        config.p_mdpSetting = static_cast<void*>( &myMdpSetting );

        if (needClearZoom) {
#if MTK_CAM_NEW_NVRAM_SUPPORT
            std::tuple<void*, int> nvramData = getTuningFromNvram<NSIspTuning::EModule_ClearZoom>(
                    openId,
                    0, // no need index
                    magicNum
                    );

            // attach tuning buffer (read-only, no need to free)
            config.cz.p_customSetting = std::get<I_NVRAM_BUFFER()>(nvramData);
            MY_LOGD("{ClearZoom(%s)}",
                    msg.c_str());
#endif
        }
        else {
            MY_LOGD("{ClearZoom is disabled(%s)}", msg.c_str());
        }

        if (needDre) {
            config.dre.cmd      = IImageTransform::DREParam::CMD::DRE_Apply;
            config.dre.type     = IImageTransform::DREParam::HisType::His_One_Time;
            config.dre.userId   = uniqueKey;
            config.dre.pBuffer  = nullptr;

#if MTK_CAM_NEW_NVRAM_SUPPORT
            std::tuple<void*, int> nvramData = getTuningFromNvram<NSIspTuning::EModule_CA_LTM>(
                    openId,
                    0, // no need index
                    magicNum
                    );
            // attach tuning buffer (read-only, no need to free)
            config.dre.p_customSetting = std::get<I_NVRAM_BUFFER()>(nvramData);
            config.dre.customIdx       = std::get<I_NVRAM_INDEX()>(nvramData);
            MY_LOGD("{Try to apply DRE(%s) sensorId:%d ISO:%d userId:%d customIdx:%d}",
                    msg.c_str(),
                    config.sensorId,
                    config.iso,
                    uniqueKey,
                    config.dre.customIdx
                   );
#endif
        }
        else {
            MY_LOGD("{DRE is disabled(%s)}", msg.c_str());
        }

        transform->setPQParameter(config);
    }

    MBOOL ret = MTRUE;
    {
        CAM_TRACE_NAME("ImageTransform execute");
        LOG_SCOPE(m_callerName, "ImageTransform execute");

        if(__builtin_expect( m_dumpFlag, false )) {
            dumpImage(
                pResult,
                pHalIn,
                BASE_CONTROLLER_DUMP_FILE_NAME_PREFIX,
                "before_handleyuv",
                "jpegyuv"
            );
        }

        if (pThumnYuv) {
            ret = transform->execute(
                    pResult,
                    pJpegYuv,
                    pThumnYuv,
                    rectDst1,
                    rectDst2,
                    outOrientaion,
                    0,
                    3000);
        }
        else {
            ret = transform->execute(
                    pResult,
                    pJpegYuv,
                    nullptr,
                    rectDst1,
                    outOrientaion,
                    3000);
        }

        if( m_dumpFlag ) {
            dumpImage(
                pJpegYuv,
                pHalIn,
                BASE_CONTROLLER_DUMP_FILE_NAME_PREFIX,
                "after_handleyuv",
                "jpegyuv"
            );
            dumpImage(
                pThumnYuv,
                pHalIn,
                BASE_CONTROLLER_DUMP_FILE_NAME_PREFIX,
                "after_handleyuv",
                "thumbyuv"
            );
        }
    }

    if (ret != MTRUE)
        MY_LOGE("execute IImageTransform::execute returns fail");
    // fd info
    {
        auto fdReader = IFDContainer::createInstance(LOG_TAG,  IFDContainer::eFDContainer_Opt_Read);
        fdReader->queryUnlock(fdData);
    }
    // update MDP debug EXIF
    if (pMdpExifBuffer.get() && pMetadataExif) {
        IMetadata exifMeta;
        if ( ! IMetadata::getEntry(pMetadataExif, MTK_3A_EXIF_METADATA, exifMeta) ) {
            MY_LOGW("cannot get 3A_EXIF_METADATA, cannot set MDP info to EXIF");
        }
        else {
            using NSCam::DebugExifUtils;

            // update only DEBUG_EXIF_RESERVE3
            auto _pResult = DebugExifUtils::setDebugExif(
                    DebugExifUtils::DebugExifType::DEBUG_EXIF_RESERVE3,
                    static_cast<MUINT32>(MTK_RESVC_EXIF_DBGINFO_KEY),
                    static_cast<MUINT32>(MTK_RESVC_EXIF_DBGINFO_DATA),
                    MDPSETTING_MAX_SIZE,
                    pMdpExifBuffer.get(),
                    &exifMeta
                    );

            if (_pResult == nullptr) {
                MY_LOGW("setDebugExif failed, may be not have MDP debug EXIF");
            }
            else {
                // update EXIF data back to the target
                IMetadata::setEntry(pMetadataExif, MTK_3A_EXIF_METADATA, exifMeta);
                MY_LOGD("setDebugExif (MDP) OK.");
            }
        }
    }
    else {
        MY_LOGD("no apply MDP EXIF info");
    }

    return ret;
}

uint32_t BaseController::getSensorMode() const
{
    return m_sensorMode;
}

bool BaseController::enterCaptureScenario(
        const MINT32 openId,
        const MUINT32 sensorMode
        )
{
    std::lock_guard<std::mutex> __oplocker(m_opScenarioCtrlMx);
    FUNCTION_SCOPE(m_callerName);

    m_spScenarioCtrl = BaseController::m_wpScenarioCtrl.promote();
    if( m_spScenarioCtrl.get() == nullptr )
    {
        MY_LOGW("no scenario Control exist, create a new one");
        BaseController::m_wpScenarioCtrl
            = new ScenarioCtrl(openId, sensorMode, IScenarioControl::Scenario_ContinuousShot);
        m_spScenarioCtrl = BaseController::m_wpScenarioCtrl.promote();
    }
    else{
        MY_LOGD("keep ScenarioCtrl, ref count:%u", m_spScenarioCtrl->getStrongCount());
    }

    return MTRUE;
}

bool BaseController::exitCaptureScenario()
{
    std::lock_guard<std::mutex> __oplocker(m_opScenarioCtrlMx);
    FUNCTION_SCOPE(m_callerName);

    if( m_spScenarioCtrl.get() != nullptr ) {
        MY_LOGD("release ScenarioCtrl, ref count:%u", m_spScenarioCtrl->getStrongCount());
        m_spScenarioCtrl = nullptr;
    }

    return MTRUE;
}

bool BaseController::dumpImage(
        IImageBuffer* pImg,
        IMetadata*    pHalIn,
        const char*   prefix,
        const char*   stageName,
        const char*   bufferName)
{
    using namespace std;
    using std::string;

    bool ret = true;

    if (pImg == nullptr) {
        MY_LOGE("pImag is invalid nullptr, no need to dump");
        return false;
    }

    // prefix, folder + file pattern
    string _f = BASE_CONTROLLER_DUMP_PATH;
    _f += prefix;

    auto rp = [&_f](string token, string value)
    {
        auto pos = _f.find(token);
        if ( pos != std::string::npos ) {
            _f.replace(pos, token.length(), value);
        }
    };

    auto get_img_format_string = [](MINT imageFormat) {
        MY_LOGD("image format:0x%X", imageFormat);
        switch (imageFormat) {
            case eImgFmt_I420:
                return "i420";
            case eImgFmt_I422:
                return "i422";
            case eImgFmt_NV12:
                return "nv12";
            case eImgFmt_NV21:
                return "nv21";
            case eImgFmt_NV16:
                return "nv16";
            case eImgFmt_NV61:
                return "nv61";
            case eImgFmt_Y8:
                return "y8";
            case eImgFmt_YUY2:
                return "yuy2";
            case eImgFmt_YV12:
                return "yv12";
            case eImgFmt_YVYU:
                return "yvyu";
            default:
                MY_LOGE("unknow image format:0x%X", imageFormat);
        }
        return "unknow";
    };

    char strbuf[256] = {0};

    MINT32 uniqueKey    = 0; // pipeline uniqueKey, it's a system timestamp
    MINT32 frameKey     = 0; // pipeline frame number
    MINT32 requestKey   = 0; // pipeline request number
    MINT32 iso          = 0; // the ISO of the frame
    MINT32 exp          = 0; // the exposure time of the frame

    if (CC_LIKELY(pHalIn != nullptr)) {
        IMetadata exifMeta;
        if (!IMetadata::getEntry<IMetadata>(const_cast<IMetadata*>(pHalIn), MTK_3A_EXIF_METADATA, exifMeta)) {
            MY_LOGW("no MTK_3A_EXIF_METADATA from HalMetaData");
        }
        else {
            if (!IMetadata::getEntry<MINT32>(&exifMeta, MTK_3A_EXIF_AE_ISO_SPEED, iso)) {
                MY_LOGW("no MTK_3A_EXIF_AE_ISO_SPEED from HalMetaData");
            }
            if (!IMetadata::getEntry<MINT32>(&exifMeta, MTK_3A_EXIF_CAP_EXPOSURE_TIME, exp)) {
                MY_LOGW("no MTK_3A_EXIF_CAP_EXPOSURE_TIME from HalMetaData");
            }
        }

        if (!IMetadata::getEntry<MINT32>(const_cast<IMetadata*>(pHalIn), MTK_PIPELINE_UNIQUE_KEY, uniqueKey))
        {
            MY_LOGW("no MTK_PIPELINE_UNIQUE_KEY from HalMetaData");
        }

        if (!IMetadata::getEntry<MINT32>(const_cast<IMetadata*>(pHalIn), MTK_PIPELINE_FRAME_NUMBER, frameKey))
        {
            MY_LOGW("no MTK_PIPELINE_FRAME_NUMBER from HalMetaData");
        }
    }

    // width, height
    rp("{WIDTH}",  to_string(pImg->getImgSize().w));
    rp("{HEIGHT}", to_string(pImg->getImgSize().h));

    // iso, shutterUs
    rp("{ISO}", to_string(iso));
    rp("{EXP}", to_string(exp));

    // uniqueKey
    ::sprintf(strbuf, "%09d", uniqueKey);
    rp("{UNIQUEKEY}", strbuf);

    // frame numbuer
    ::sprintf(strbuf, "%04d", frameKey);
    rp("{FRAMENO}", strbuf);

    // request number
    ::sprintf(strbuf, "%04d", requestKey);
    rp("{REQUESTNO}", strbuf);

    // caller name, stage name, buffer name, extension name
    rp("{CALLER}", m_callerName);
    rp("{STAGE}", stageName);
    rp("{BUFNAME}", bufferName);
    rp("{EXT}", get_img_format_string(pImg->getImgFormat()));

    MY_LOGD("save image %s", _f.c_str());
    ret = pImg->saveToFile(_f.c_str());

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
ScenarioCtrl::ScenarioCtrl(
    const MINT32 openId,
    const MUINT32 sensorMode,
    const MINT32 scenario
)
    : m_openId(openId)
    , m_sensorMode(sensorMode)
    , m_scenario(scenario)
    , m_enterResult(MFALSE)
    , m_scenarioControl(nullptr)
{
    FUNCTION_SCOPE(__CALLERNAME__);

    if( m_scenarioControl.get() == nullptr )
    {
        MY_LOGD("no scenario Control exist, create a new one");
        m_scenarioControl = IScenarioControl::create(m_openId);

        if( CC_UNLIKELY(m_scenarioControl.get() == nullptr) ) {
            MY_LOGE("get Scenario Control fail");
            return;
        }
        else {
            HwInfoHelper helper(m_openId);
            if( ! helper.updateInfos() ) {
                MY_LOGE("cannot properly update infos");
                return;
            }
            MSize sensorSize;
            MINT32 sensorFps;

            // check sensor mode, for development stage, assert it if it's undefined
            if (CC_UNLIKELY( m_sensorMode == SENSOR_SCENARIO_ID_UNNAMED_START )) {
#if (MTKCAM_LOG_LEVEL_DEFAULT >= 3)
                MY_LOGE("invalid sensor mode, assert");
                assert(0);
#else
                MY_LOGW("invalid sensor mode, force set to capture sensor mode");
                m_sensorMode = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
#endif
            }

            if( ! helper.getSensorSize( m_sensorMode, sensorSize) ||
                ! helper.getSensorFps( m_sensorMode, sensorFps) ) {
                MY_LOGE("cannot get params about sensor");
                return;
            }
            //
            IScenarioControl::ControlParam param;
            param.scenario   = m_scenario;
            param.sensorSize = sensorSize;
            param.sensorFps  = sensorFps;

            if (helper.getDualPDAFSupported(m_sensorMode)) {
                FEATURE_CFG_ENABLE_MASK(param.featureFlag,IScenarioControl::FEATURE_DUAL_PD);
            }

            m_scenarioControl->enterScenario(param);
            m_enterResult = MTRUE;
            MY_LOGD("enter Scenario Control: %u, ref count:%u", param.scenario, m_scenarioControl->getStrongCount());
        }
    }
    else {
        MY_LOGD("keep Scenario Control, ref count:%u", m_scenarioControl->getStrongCount());
    }
}


ScenarioCtrl::~ScenarioCtrl()
{
    FUNCTION_SCOPE(__CALLERNAME__);

    if( m_scenarioControl.get() != nullptr ) {
        if (m_enterResult) {
            m_scenarioControl->enterScenario(IScenarioControl::Scenario_ZsdPreview);
            MY_LOGD("resume Scenario Control: Scenario_ZsdPreview");
        }

        MY_LOGD("release Scenario Control, ref count:%u", m_scenarioControl->getStrongCount());
        m_scenarioControl = nullptr;
        m_enterResult = MFALSE;
    }
}
