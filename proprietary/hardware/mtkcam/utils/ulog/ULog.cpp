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
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#define LOG_TAG "ULog"

#include <cstdint>
#include <atomic>
#include <memory>
#include <string>
#include <cutils/properties.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/utils/debug/debug.h>
#include <camera_custom_ulog.h> // in custom/<platform>/hal/inc/
#include "ULogInt.h"
#include "ULogTable.h"
#include "ULogFile.h"
#include "ULogTrace.h"
#include "ULogGuard.h"
#include "ULogMulti.h"
#include "ULogRTDiag.h"


// Default values
// See ULogInitializerImpl::ULogInitializerImpl() how they are applied
static const int ULOG_MODE_DEFAULT = static_cast<int>(NSCam::Utils::ULog::ULogger::ANDROID_LOG);

// All PROJECT_FILTERs are defined in camera_custom_ulog.h, which configured by project
#ifndef ULOG_PROJECT_REQ_FILTER
#ifdef MTKCAM_USER_LOAD // Disable all trace logs in user load
#define ULOG_PROJECT_REQ_FILTER 0
#else
#define ULOG_PROJECT_REQ_FILTER 0x1ff000
#endif
#endif

#ifndef ULOG_PROJECT_FUNC_FILTER
#ifdef MTKCAM_USER_LOAD
#define ULOG_PROJECT_FUNC_FILTER 0
#else
#define ULOG_PROJECT_FUNC_FILTER 0x0c080000
#endif
#endif

#ifndef ULOG_PROJECT_DETAILS_FILTER
#define ULOG_PROJECT_DETAILS_FILTER 0xfffff000
#endif

static const unsigned int ULOG_REQ_FILTER_DEFAULT = ULOG_PROJECT_REQ_FILTER;
static const unsigned int ULOG_FUNC_FILTER_DEFAULT = ULOG_PROJECT_FUNC_FILTER;
static const unsigned int ULOG_DETAILS_FILTER_DEFAULT = ULOG_PROJECT_DETAILS_FILTER;

static const int ULOG_DETAILS_LEVEL_DEFAULT = static_cast<int>(NSCam::Utils::ULog::DETAILS_INFO);


// Property names
static const char * const ULOG_MODE_PROP_NAME = "vendor.debug.camera.ulog.mode";
static const char * const ULOG_FILTER_PROP_NAME = "vendor.debug.camera.ulog.filter";
static const char * const ULOG_FUNCLIFE_FILTER_PROP_NAME = "vendor.debug.camera.ulog.func";
static const char * const ULOG_FUNCLIFE2_FILTER_PROP_NAME = "vendor.debug.camera.ulog.func2";
static const char * const ULOG_DETAILS_LEVEL_PROP_NAME = "vendor.debug.camera.ulog.level";
static const char * const ULOG_DETAILS_FILTER_PROP_NAME = "vendor.debug.camera.ulog.details"; // normal details
static const char * const ULOG_RUNTIME_DIAG_PROP_NAME = "vendor.debug.camera.ulog.rtdiag";


// ================================================================================================

namespace NSCam {
namespace Utils {
namespace ULog {

ULogger* getDefaultULogger();


// Consider the spacial locality of cache/page
// zero init so that all of them can be together in .bss
alignas(1024)
unsigned int ULogger::sModFilterMask = 0;
unsigned int ULogFuncLife::sModFilterMask = 0;
ULogger* ULogger::sULogger = getDefaultULogger(); // never be NULL
ULogRuntimeDiag *ULogRuntimeDiag::sDiag = nullptr;
unsigned int ULogger::sDetailsLevel = 0;
unsigned int ULogger::sNormalDetailsMask = 0;
unsigned int ULogger::sMode = 0;

// Put the singletons here so that the initialization order is under control
// and in the same page
ULogTable ULogTable::sSingleton; // 512 bytes
ULogGuardMonitor ULogGuardMonitor::sSingleton;


// ------------------------------------------------------------------------------------------------

size_t printIntArray(char *buffer, size_t bufferSize, const unsigned int *value, size_t lenOfValue)
{
    static constexpr size_t MAX_DIGITS = 13;

    size_t n = lenOfValue;
    size_t charPrinted = 0;
    while (n > 0) {
        if (n >= 4 && bufferSize > MAX_DIGITS * 4) {
            charPrinted = snprintf(buffer, bufferSize, ",%u,%u,%u,%u", value[0], value[1], value[2], value[3]);
            n -= 4;
            value += 4;
        } else if (n >= 3 && bufferSize > MAX_DIGITS * 3) {
            charPrinted = snprintf(buffer, bufferSize, ",%u,%u,%u", value[0], value[1], value[2]);
            n -= 3;
            value += 3;
        } else if (n >= 2 && bufferSize > MAX_DIGITS * 2) {
            charPrinted = snprintf(buffer, bufferSize, ",%u,%u", value[0], value[1]);
            n -= 2;
            value += 2;
        } else if (n >= 1 && bufferSize > MAX_DIGITS) {
            charPrinted = snprintf(buffer, bufferSize, ",%u", value[0]);
            n--;
            value++;
        } else {
            break;
        }

        buffer += charPrinted;
        bufferSize -= charPrinted;
    }

    return (lenOfValue - n);
}


ULogger::~ULogger()
{
}


void ULogger::addModFilterMask(unsigned int mask)
{
    sModFilterMask |= (mask & (ULOG_LAYER_MASK | ULOG_GROUP_MASK));
    std::atomic_thread_fence(std::memory_order_release);
}


void ULogger::onLogPathDiv(ModuleId , const char *, RequestTypeId , RequestSerial , const ModuleId *, size_t )
{
}


void ULogger::onLogPathJoin(ModuleId , const char *, RequestTypeId , RequestSerial , const ModuleId *, size_t )
{
}


void ULogger::onLogSubreqs(ModuleId , const char *, RequestTypeId , RequestSerial , RequestTypeId , const RequestSerial *, size_t )
{
}


void ULogger::onLogDetails(ModuleId , const char *, DetailsType , const char *, size_t )
{
}


void ULogger::onLogValue(ModuleId , const char *, const char *, std::int32_t )
{
}


void ULogger::onFlush(int)
{
}


class AndroidULogger : public ULogger
{
private:
    std::atomic_uint mLogSerial;

    inline unsigned int getLogSerial() {
        return mLogSerial.fetch_add(1, std::memory_order_relaxed);
    }

public:
    AndroidULogger() : mLogSerial(0) { }

    virtual ~AndroidULogger() { }

    virtual void onLogEnter(ModuleId moduleId, const char *tag, RequestTypeId requestTypeId, RequestSerial requestSerial) override {
        ULOG_IMP_LOGD("R %s:%u M[%s:%x] +  :%s #%u", getRequestTypeName(requestTypeId), requestSerial,
            getModuleName(moduleId), moduleId, tag, getLogSerial());
    }

    virtual void onLogExit(ModuleId moduleId, const char *tag, RequestTypeId requestTypeId, RequestSerial requestSerial) override {
        ULOG_IMP_LOGD("R %s:%u M[%s:%x] -  :%s #%u", getRequestTypeName(requestTypeId), requestSerial,
            getModuleName(moduleId), moduleId, tag, getLogSerial());
    }

    virtual void onLogDiscard(ModuleId moduleId, const char *tag, RequestTypeId requestTypeId, const RequestSerial *requestSerialList, size_t n) override {
        if (n == 1) {
            ULOG_IMP_LOGD("R %s:%u M[%s:%x] - (discard)  :%s #%u", getRequestTypeName(requestTypeId), requestSerialList[0],
                getModuleName(moduleId), moduleId, tag, getLogSerial());
        } else {
            static const size_t BUFFER_SIZE = 128;
            char buffer[BUFFER_SIZE];

            while (n > 0) {
                int nPrinted = printIntArray(buffer, BUFFER_SIZE, requestSerialList, n);
                n -= nPrinted;
                requestSerialList += nPrinted;
                ULOG_IMP_LOGD("R %s:%s M[%s:%x] - (discard) cont:%zu  :%s #%u", getRequestTypeName(requestTypeId),
                    buffer + 1, getModuleName(moduleId), moduleId, n, tag, getLogSerial());
            }
        }
    }

    virtual void onLogSubreqs(ModuleId moduleId, const char *tag, RequestTypeId requestTypeId, RequestSerial requestSerial,
        RequestTypeId subrequestTypeId, const RequestSerial *subrequestSerialList, size_t n) override
    {
        if (n == 1) {
            ULOG_IMP_LOGD("R %s:%u -> R %s:%u genOn M[%s:%x]  :%s #%u", getRequestTypeName(requestTypeId), requestSerial,
                getRequestTypeName(subrequestTypeId), subrequestSerialList[0], getModuleName(moduleId), moduleId, tag, getLogSerial());
        } else {
            static const size_t BUFFER_SIZE = 128;
            char buffer[BUFFER_SIZE];

            while (n > 0) {
                size_t nPrinted = printIntArray(buffer, BUFFER_SIZE, subrequestSerialList, n);
                n -= nPrinted;
                subrequestSerialList += nPrinted;
                ULOG_IMP_LOGD("R %s:%u -> R %s:%s genOn M[%s:%x] cont:%zu  :%s #%u", getRequestTypeName(requestTypeId), requestSerial,
                    getRequestTypeName(subrequestTypeId), buffer + 1, getModuleName(moduleId), moduleId, n, tag, getLogSerial());
            }
        }
    }

    virtual void onLogPathDiv(ModuleId moduleId, const char *tag, RequestTypeId requestTypeId, RequestSerial requestSerial,
        const ModuleId *toModuleIdList, size_t n) override
    {
        switch (n) {
        case 1:
            ULOG_IMP_LOGD("R %s:%u  M[%s] divTo M[%s]  :%s", getRequestTypeName(requestTypeId), requestSerial,
                getModuleName(moduleId), getModuleName(toModuleIdList[0]), tag);
            break;
        case 2:
            ULOG_IMP_LOGD("R %s:%u  M[%s] divTo M[%s] M[%s]  :%s", getRequestTypeName(requestTypeId), requestSerial,
                getModuleName(moduleId), getModuleName(toModuleIdList[0]), getModuleName(toModuleIdList[1]), tag);
            break;
        case 3:
            ULOG_IMP_LOGD("R %s:%u  M[%s] divTo M[%s] M[%s] M[%s]  :%s", getRequestTypeName(requestTypeId), requestSerial,
                getModuleName(moduleId),
                getModuleName(toModuleIdList[0]), getModuleName(toModuleIdList[1]), getModuleName(toModuleIdList[2]), tag);
            break;
        case 4:
            ULOG_IMP_LOGD("R %s:%u  M[%s] divTo M[%s] M[%s] M[%s] M[%s]  :%s",
                getRequestTypeName(requestTypeId), requestSerial,
                getModuleName(moduleId), getModuleName(toModuleIdList[0]), getModuleName(toModuleIdList[1]),
                getModuleName(toModuleIdList[2]), getModuleName(toModuleIdList[3]), tag);
            break;
        default:
            if (n > 4) {
                ULOG_IMP_LOGD("R %s:%u  M[%s] divTo M[%s] M[%s] M[%s] M[%s] ... (%zu)  :%s",
                    getRequestTypeName(requestTypeId), requestSerial,
                    getModuleName(moduleId), getModuleName(toModuleIdList[0]), getModuleName(toModuleIdList[1]),
                    getModuleName(toModuleIdList[2]), getModuleName(toModuleIdList[3]), n, tag);
            }
            break;
        }
    }

    virtual void onLogPathJoin(ModuleId moduleId, const char *tag, RequestTypeId requestTypeId, RequestSerial requestSerial,
        const ModuleId *fromModuleIdList, size_t n) override
    {
        switch (n) {
        case 1:
            ULOG_IMP_LOGD("R %s:%u  M[%s] joinFrom M[%s]  :%s", getRequestTypeName(requestTypeId), requestSerial,
                getModuleName(moduleId), getModuleName(fromModuleIdList[0]), tag);
            break;
        case 2:
            ULOG_IMP_LOGD("R %s:%u  M[%s] joinFrom M[%s] M[%s]  :%s", getRequestTypeName(requestTypeId), requestSerial,
                getModuleName(moduleId), getModuleName(fromModuleIdList[0]), getModuleName(fromModuleIdList[1]), tag);
            break;
        case 3:
            ULOG_IMP_LOGD("R %s:%u  M[%s] joinFrom M[%s] M[%s] M[%s]  :%s",
                getRequestTypeName(requestTypeId), requestSerial,
                getModuleName(moduleId),
                getModuleName(fromModuleIdList[0]), getModuleName(fromModuleIdList[1]), getModuleName(fromModuleIdList[2]), tag);
            break;
        case 4:
            ULOG_IMP_LOGD("R %s:%u  M[%s] joinFrom M[%s] M[%s] M[%s] M[%s]  :%s",
                getRequestTypeName(requestTypeId), requestSerial,
                getModuleName(moduleId), getModuleName(fromModuleIdList[0]), getModuleName(fromModuleIdList[1]),
                getModuleName(fromModuleIdList[2]), getModuleName(fromModuleIdList[3]), tag);
            break;
        default:
            if (n > 4) {
                ULOG_IMP_LOGD("R %s:%u  M[%s] joinFrom M[%s] M[%s] M[%s] M[%s] ... (%zu)  :%s",
                    getRequestTypeName(requestTypeId), requestSerial,
                    getModuleName(moduleId), getModuleName(fromModuleIdList[0]), getModuleName(fromModuleIdList[1]),
                    getModuleName(fromModuleIdList[2]), getModuleName(fromModuleIdList[3]), n, tag);
            }
            break;
        }
    }

    virtual void onLogFuncLife(ModuleId moduleId, const char *tag, const char *funcName, FuncLifeTag lifeTag) override {
        if ((lifeTag & ~EXIT_BIT) == API_ENTER) {
            ULOG_FW_LOGD(tag, "[%s/%s] %c  ULog#%u", getModuleName(moduleId), funcName, ((lifeTag & EXIT_BIT) ? '-' : '+'), getLogSerial());
        } else {
            ULOG_FW_LOGD(tag, "[%s] %c  ULog#%u", funcName, ((lifeTag & EXIT_BIT) ? '-' : '+'), getLogSerial());
        }
    }

    virtual void onLogFuncLifeExt(ModuleId moduleId, const char *tag, const char *funcName, FuncLifeTag lifeTag,
        std::intptr_t v1, std::intptr_t v2, std::intptr_t v3) override
    {
        if ((lifeTag & ~EXIT_BIT) == API_ENTER) {
            ULOG_FW_LOGD(tag, "[%s/%s] %c (0x%" PRIxPTR ",0x%" PRIxPTR ",0x%" PRIxPTR ") ULog#%u", getModuleName(moduleId), funcName,
                ((lifeTag & EXIT_BIT) ? '-' : '+'), v1, v2, v3, getLogSerial());
        } else {
            ULOG_FW_LOGD(tag, "[%s] %c (0x%" PRIxPTR ",0x%" PRIxPTR ",0x%" PRIxPTR ") ULog#%u", funcName,
                ((lifeTag & EXIT_BIT) ? '-' : '+'), v1, v2, v3, getLogSerial());
        }
    }

    virtual void onLogDetails(ModuleId , const char *tag, DetailsType type, const char *content, size_t) override
    {
        switch (type) {
        case DETAILS_INFO:
            ULOG_FW_LOGI(tag, "%s", content);
            break;
        case DETAILS_DECISION_KEY:
        case DETAILS_DEBUG:
            ULOG_FW_LOGD(tag, "%s", content);
            break;
        case DETAILS_VERBOSE:
            ULOG_FW_LOGV(tag, "%s", content);
            break;
        default:
            break;
        }
    }
};


class ULogMtkcamDebuggee : public IDebuggee
{
private:
    std::string mDebuggeeName;

public:
    ULogMtkcamDebuggee() : mDebuggeeName("NSCam::ULog") { }

    virtual std::string debuggeeName() const override {
        return mDebuggeeName;
    }

    virtual void debug(android::Printer& printer,
        const std::vector<std::string>& ) override
    {
        if (ULogRuntimeDiag::isEnabled())
            ULogRTDiagImpl::get().debugDump(printer);

        ULogger::flush(3);
        printer.printLine("Flushed!");
    }
};


class ULogInitializerImpl
{
    friend ULogger* getDefaultULogger();

private:
    static int sInited;
    static AndroidULogger sAndroidULogger;
    static DetailsType sDefaultAndroidLogType;

    static DetailsType getAndroidDefaultDetailsType() {
        if (sDefaultAndroidLogType == DETAILS_INVALID) {
            // for backward compatible
            int32_t level = ::property_get_int32("persist.vendor.mtk.camera.log_level", -1);
            if (level == -1) {
#ifdef ULOG_PROJECT_DETAILS_LEVEL
                sDefaultAndroidLogType = ULOG_PROJECT_DETAILS_LEVEL;
                return;
#else
                level = MTKCAM_LOG_LEVEL_DEFAULT;
#endif
            }

            switch (level) {
            case 4:
                sDefaultAndroidLogType = DETAILS_VERBOSE;
                break;
            case 3:
                sDefaultAndroidLogType = DETAILS_DEBUG;
                break;
            case 2:
                sDefaultAndroidLogType = DETAILS_INFO;
                break;
            case 1:
            default:
                sDefaultAndroidLogType = DETAILS_WARNING;
                break;
            }
        }

        return sDefaultAndroidLogType;
    }

public:
    static void setDefaultULoggerInInit() {
        ULogger::setDetailsLevelAndMask(getAndroidDefaultDetailsType(), 0xffffffff);
        ULogger::setModFilterMask(0);
        ULogFuncLife::setModFilterMask(0);
        ULogger::sULogger = &sAndroidULogger;
        ULogger::setMode(ULogger::ANDROID_LOG);
        std::atomic_thread_fence(std::memory_order_release);
    }

    ULogInitializerImpl();
    ~ULogInitializerImpl();

private:
    std::unique_ptr<ULogger> mpULogger;
    std::shared_ptr<IDebuggee> mDebuggee;
    std::shared_ptr<IDebuggeeCookie> mDebuggeeCookie;
};


AndroidULogger ULogInitializerImpl::sAndroidULogger;
int ULogInitializerImpl::sInited = 0;
DetailsType ULogInitializerImpl::sDefaultAndroidLogType = DETAILS_INVALID;


ULogger* getDefaultULogger()
{
    return &ULogInitializerImpl::sAndroidULogger;
}


inline ULogInitializerImpl::ULogInitializerImpl()
{
    // We don't have any sub thread yet, so all global writes are safe here
    sInited++;
    if (sInited > 1)
        return;

    ULogTable::getSingleton().init();

    int mode = property_get_int32(ULOG_MODE_PROP_NAME, ULOG_MODE_DEFAULT);

    // ---- Default filter/level ----

    int detailsLevelDefault = ULOG_DETAILS_LEVEL_DEFAULT;
    std::int64_t reqFilterDefault = ULOG_REQ_FILTER_DEFAULT;
    std::int64_t funcLifeFilterDefault = ULOG_FUNC_FILTER_DEFAULT;
    std::int64_t normalDetailsDefault = ULOG_DETAILS_FILTER_DEFAULT;
    int runtimeDiagDefault = 0;

    if (mode & ULogger::ANDROID_LOG) {
        // Normal details log works as normal Android log
        // Others requires filter
        detailsLevelDefault = getAndroidDefaultDetailsType();
    }

    if (mode & (ULogger::TEXT_FILE_LOG | ULogger::PASSIVE_LOG)) {
        if (mode & ULogger::TEXT_FILE_LOG) {
            detailsLevelDefault = DETAILS_DEBUG;
        } else { // passive mode
            detailsLevelDefault = DETAILS_DECISION_KEY;
        }
    }

#ifdef ULOG_RUNTIME_DIAGNOSTIC
    runtimeDiagDefault = 0x1;
#endif

    if (mode & ULogger::TRACE_LOG) {
#if defined(MTKCAM_SYSTRACE_LEVEL_DEFAULT) && MTKCAM_SYSTRACE_LEVEL_DEFAULT == 0
        funcLifeFilterDefault = 0x007ff000; // HwNode
#else
        funcLifeFilterDefault = 0xfffff000;
#endif
    }

    if (mode == ULogger::TRACE_LOG) {
#if defined(MTKCAM_SYSTRACE_LEVEL_DEFAULT) && MTKCAM_SYSTRACE_LEVEL_DEFAULT == 0
        reqFilterDefault = 0;
#else
        reqFilterDefault = 0x00904000;
#endif
        detailsLevelDefault = DETAILS_DECISION_KEY;
    }

    // ---- Get filter & level from property

    unsigned int reqFilter = static_cast<unsigned int>(
        property_get_int64(ULOG_FILTER_PROP_NAME, reqFilterDefault));
    if ((reqFilter & ULOG_LAYER_MASK) == 0)
        reqFilter <<= 12;

    unsigned int funcLifeFilter = static_cast<unsigned int>(
        property_get_int64(ULOG_FUNCLIFE_FILTER_PROP_NAME, funcLifeFilterDefault));
    if (funcLifeFilter != 0) {
        if (funcLifeFilter == 1)
            funcLifeFilter = reqFilter;
        if ((funcLifeFilter & ULOG_LAYER_MASK) == 0)
            funcLifeFilter <<= 12;
    }

    unsigned int func2LifeFilter = 0;
    if ((mode & ULogger::TRACE_LOG) && (mode != ULogger::TRACE_LOG)) {
        // Has second logger besides trace
        func2LifeFilter = static_cast<unsigned int>(
            property_get_int64(ULOG_FUNCLIFE2_FILTER_PROP_NAME, 0));
        if (func2LifeFilter != 0) {
            if (func2LifeFilter == 1)
                func2LifeFilter = funcLifeFilter;
            if ((func2LifeFilter & ULOG_LAYER_MASK) == 0)
                func2LifeFilter <<= 12;
        }
    }

    unsigned int normalDetailsFilter = static_cast<unsigned int>(
        property_get_int64(ULOG_DETAILS_FILTER_PROP_NAME, normalDetailsDefault));
    if ((normalDetailsFilter & ULOG_LAYER_MASK) == 0)
        normalDetailsFilter <<= 12;

    int detailsLevel = property_get_int32(ULOG_DETAILS_LEVEL_PROP_NAME, detailsLevelDefault);
    if (detailsLevel < DETAILS_ERROR)
        detailsLevel = DETAILS_ERROR;

    unsigned int runtimeDiagEnabled = static_cast<unsigned int>(
        property_get_int32(ULOG_RUNTIME_DIAG_PROP_NAME, runtimeDiagDefault));
    if (runtimeDiagEnabled != 0) {
        ULogRuntimeDiag::onInit();
    }

    // ---- Instantiate ULogger ----

    bool attachMtkcamDebug = false;
    switch (mode) {
    case ULogger::TEXT_FILE_LOG:
        mpULogger = std::make_unique<File::FileULogger>();
        ULogger::sULogger = mpULogger.get();
        attachMtkcamDebug = true;
        break;
    case ULogger::PASSIVE_LOG:
        mpULogger = std::make_unique<File::PassiveULogger>();
        ULogger::sULogger = mpULogger.get();
        attachMtkcamDebug = true;
        break;
    case ULogger::TRACE_LOG:
        mpULogger = std::make_unique<TraceULogger>();
        ULogger::sULogger = mpULogger.get();
        break;
    case (ULogger::TRACE_LOG | ULogger::ANDROID_LOG):
        mpULogger = std::make_unique<DualULogger<TraceULogger, AndroidULogger>>(func2LifeFilter);
        ULogger::sULogger = mpULogger.get();
        break;
    case (ULogger::TRACE_LOG | ULogger::TEXT_FILE_LOG):
        mpULogger = std::make_unique<DualULogger<TraceULogger, File::FileULogger>>(func2LifeFilter);
        ULogger::sULogger = mpULogger.get();
        attachMtkcamDebug = true;
        break;
    case (ULogger::TRACE_LOG | ULogger::PASSIVE_LOG):
        mpULogger = std::make_unique<DualULogger<TraceULogger, File::PassiveULogger>>(func2LifeFilter);
        ULogger::sULogger = mpULogger.get();
        attachMtkcamDebug = true;
        break;
    default:
        if (mode == 0) {
            reqFilter = 0; // All off
            funcLifeFilter = 0;
            normalDetailsFilter = 0;
            detailsLevel = DETAILS_ERROR;
        }
        mode = ULogger::ANDROID_LOG;
        ULogger::sULogger = &sAndroidULogger;
        break;
    }

    if (ULogRuntimeDiag::isEnabled())
        attachMtkcamDebug = true;

    ULogger::setMode(mode);
    if (ULogger::sULogger != nullptr) {
        ULogger::sULogger->onInit();
    }

    std::atomic_thread_fence(std::memory_order_release);

    // ---- Init mask & others ----

    ULogger::setDetailsLevelAndMask(static_cast<DetailsType>(detailsLevel), normalDetailsFilter);
    ULogger::setModFilterMask(reqFilter);
    ULogFuncLife::setModFilterMask(funcLifeFilter);

    ULOG_IMP_LOGI("ULog initialized: mode=0x%x  filters: req=0x%x func=0x%x/0x%x details=0x%x level=%d",
        mode, reqFilter, funcLifeFilter, func2LifeFilter, normalDetailsFilter, detailsLevel);

    std::atomic_thread_fence(std::memory_order_release);

    ULogGuard::onInit();

    if (attachMtkcamDebug && IDebuggeeManager::get() != nullptr) {
        mDebuggee = std::make_shared<ULogMtkcamDebuggee>();
        mDebuggeeCookie = IDebuggeeManager::get()->attach(mDebuggee);
    }
}


inline ULogInitializerImpl::~ULogInitializerImpl()
{
    sInited--;
    if (sInited > 0)
        return;

    ULogRuntimeDiag::onUninit();
    ULogGuard::onUninit();

    if (ULogger::sULogger != nullptr) {
        ULogger::sULogger->onUninit();
        ULogger::sULogger = &sAndroidULogger;
    }
}


// User must put ULogInitializer in the main()
struct SetULogToAndroidByDefault
{
    SetULogToAndroidByDefault() {
        ULogInitializerImpl::setDefaultULoggerInInit();
    }
    ~SetULogToAndroidByDefault() {
        // Flush for manual exit() call
        ULogger::flush(30);
    }
} __setULogToAndroidByDefault;


ULogInitializer::ULogInitializer()
{
    mpImpl = new ULogInitializerImpl;
}


ULogInitializer::~ULogInitializer()
{
    delete mpImpl;
}


}
}
}


