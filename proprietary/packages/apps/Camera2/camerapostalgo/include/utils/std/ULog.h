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

#ifndef __MTKCAM_UTILS_STD_ULOG_H__
#define __MTKCAM_UTILS_STD_ULOG_H__

#include <cstddef>
#include <cstdarg>
#include <cinttypes>
#include <cstdio>
#include <type_traits>
#include <atomic>
// If not defined LOG_TAG before include this .h, LOG_TAG will be defined to NULL
#ifdef LOG_TAG
#include <log/log.h>
#endif
// If not defined LOG_TAG before include this .h, it will build error
// We rely on user to include Log.h by theirselves
// #include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULogDef.h>
#include <mtkcam/utils/std/ULogClass.h>


namespace NSCam {
namespace Utils {
namespace ULog {


template <RequestTypeId _TypeId>
class ULogRequest
{
public:
    ULogRequest() : mSerial(sNextSerial.fetch_add(1, std::memory_order_relaxed)) { }

    RequestTypeId getULogRequestType() {
        return _TypeId;
    }

    RequestSerial getULogSerial() {
        return mSerial;
    }

private:
    static std::atomic_uint sNextSerial;
    const RequestSerial mSerial;
};


template <RequestTypeId _TypeId>
std::atomic_uint ULogRequest<_TypeId>::sNextSerial(1);


template <typename _M>
inline bool isULogReqEnabled(_M &&module)
{
    return ULogger::isLogEnabled(getULogModuleId(std::forward<_M>(module)));
}


template <typename _M>
inline bool isULogFuncTraceEnabled(_M &&module)
{
    return ULogFuncLife::isLogEnabled(getULogModuleId(std::forward<_M>(module)));
}


inline bool isULogDetailsEnabled(DetailsType type)
{
    return ULogger::isDetailsEnabled(type);
}


const char *getULogReqNameById(RequestTypeId req);

template <typename _R>
inline const char *getULogReqName(_R &&r)
{
    return getULogReqNameById(getULogRequestTypeId(std::forward<_R>(r)));
}


}
}
}


#ifndef __CAM_ULOG_DETAILS_BUFFER_SIZE__
#define __CAM_ULOG_DETAILS_BUFFER_SIZE__ 1024 /* The same as Android log */
#endif

#ifndef __CAM_ULOG_TAG__
#define __CAM_ULOG_TAG__ LOG_TAG
#endif

#define CAM_ULOGT_ENTER(_logTag, _intoModule, _requestType, _requestSerial) \
    NSCam::Utils::ULog::ULogger::logEnter(NSCam::Utils::ULog::getULogModuleId(_intoModule), \
    _logTag, NSCam::Utils::ULog::getULogRequestTypeId(_requestType), _requestSerial)

#define CAM_ULOGT_EXIT(_logTag, _outFromModule, _requestType, _requestSerial) \
    NSCam::Utils::ULog::ULogger::logExit(NSCam::Utils::ULog::getULogModuleId(_outFromModule), \
    _logTag, NSCam::Utils::ULog::getULogRequestTypeId(_requestType), _requestSerial)

#define CAM_ULOG_ENTER(_intoModule, _requestType, _requestSerial) \
    CAM_ULOGT_ENTER(__CAM_ULOG_TAG__, _intoModule, _requestType, _requestSerial)

#define CAM_ULOG_EXIT(_outFromModule, _requestType, _requestSerial) \
    CAM_ULOGT_EXIT(__CAM_ULOG_TAG__, _outFromModule, _requestType, _requestSerial)

#define CAM_ULOG_DISCARD(_byModule, _requestType, _requestSerial) \
    NSCam::Utils::ULog::ULogger::logDiscard(NSCam::Utils::ULog::getULogModuleId(_byModule), \
    __CAM_ULOG_TAG__, NSCam::Utils::ULog::getULogRequestTypeId(_requestType), _requestSerial)

#define CAM_ULOG_SUBREQS(_byModule, _requestType, _requestSerial, _subrequestType, _subrequestSerial) \
    NSCam::Utils::ULog::ULogger::logSubreqs(NSCam::Utils::ULog::getULogModuleId(_byModule), \
    __CAM_ULOG_TAG__, NSCam::Utils::ULog::getULogRequestTypeId(_requestType), _requestSerial, \
    NSCam::Utils::ULog::getULogRequestTypeId(_subrequestType), _subrequestSerial)

#define CAM_ULOG_PATHDIV(_fromModule, _requestType, _requestSerial, _toModuleId) \
    NSCam::Utils::ULog::ULogger::logPathDiv(NSCam::Utils::ULog::getULogModuleId(_fromModule), \
    __CAM_ULOG_TAG__, NSCam::Utils::ULog::getULogRequestTypeId(_requestType), _requestSerial, _toModuleId)

#define CAM_ULOG_PATHJOIN(_toModule, _requestType, _requestSerial, _fromModuleId) \
    NSCam::Utils::ULog::ULogger::logPathJoin(NSCam::Utils::ULog::getULogModuleId(_toModule), \
    __CAM_ULOG_TAG__, NSCam::Utils::ULog::getULogRequestTypeId(_requestType), _requestSerial, _fromModuleId)

#define CAM_ULOG_ENTER_GUARD(_intoModule, _requestType, _requestSerial, _warnMs...) do { \
        CAM_ULOG_ENTER(_intoModule, _requestType, _requestSerial); \
        NSCam::Utils::ULog::ULogGuard::registerReqGuard(NSCam::Utils::ULog::getULogModuleId(_intoModule), \
        NSCam::Utils::ULog::getULogRequestTypeId(_requestType), _requestSerial, ##_warnMs); \
    } while (0)

#define CAM_ULOG_EXIT_GUARD(_outFromModule, _requestType, _requestSerial) do { \
        CAM_ULOG_EXIT(_outFromModule, _requestType, _requestSerial); \
        NSCam::Utils::ULog::ULogGuard::unregisterReqGuard(NSCam::Utils::ULog::getULogModuleId(_outFromModule), \
        NSCam::Utils::ULog::getULogRequestTypeId(_requestType), _requestSerial); \
    } while (0)

#define CAM_ULOG_DISCARD_GUARD(_outFromModule, _requestType, _requestSerial) do { \
        CAM_ULOG_DISCARD(_outFromModule, _requestType, _requestSerial); \
        NSCam::Utils::ULog::ULogGuard::unregisterReqGuard(NSCam::Utils::ULog::getULogModuleId(_outFromModule), \
        NSCam::Utils::ULog::getULogRequestTypeId(_requestType), _requestSerial); \
    } while (0)

#define CAM_ULOG_MODIFY_GUARD(_modifyModule, _requestType, _requestSerial, _warnMs) do { \
        NSCam::Utils::ULog::ULogGuard::modifyReqGuard(NSCam::Utils::ULog::getULogModuleId(_modifyModule), \
        NSCam::Utils::ULog::getULogRequestTypeId(_requestType), _requestSerial, _warnMs); \
    } while (0)

#if 0

// Clang can not inline variable arguments well, we inline it manually
#define CAM_ULOG_DETAILS_R(_module, _detailsType, _requestType, _requestSerial, _fmt, _args...) \
    do { \
        NSCam::Utils::ULog::ModuleId _uModuleId = NSCam::Utils::ULog::getULogModuleId(_module); \
        if (NSCam::Utils::ULog::ULogger::isReqDetailsEnabled(_uModuleId, _detailsType)) { \
            char _buffer[__CAM_ULOG_DETAILS_BUFFER_SIZE__]; \
            size_t _contentLen = ::snprintf(_buffer, sizeof(_buffer), _fmt, ##_args); \
            _buffer[__CAM_ULOG_DETAILS_BUFFER_SIZE__ - 1] = '\0'; \
            NSCam::Utils::ULog::ULogger::logReqDetails(_uModuleId, __CAM_ULOG_TAG__, \
                _detailsType, NSCam::Utils::ULog::getULogRequestTypeId(_requestType), _requestSerial, _buffer, _contentLen); \
        } \
    } while (0)


/*
 * !! Larger overhead than Android log !!
 * Only for control flow related debug. Do NOT use for general purpose, such as image quality.
 */
#define CAM_ULOGE_R(_module, _requestType, _requestSerial, _fmt, _args...) \
    CAM_ULOG_DETAILS_R(_module, NSCam::Utils::ULog::DETAILS_ERROR, _requestType, _requestSerial, _fmt, ##_args)

#define CAM_ULOGW_R(_module, _requestType, _requestSerial, _fmt, _args...) \
    CAM_ULOG_DETAILS_R(_module, NSCam::Utils::ULog::DETAILS_WARNING, _requestType, _requestSerial, _fmt, ##_args)

#define CAM_ULOGI_R(_module, _requestType, _requestSerial, _fmt, _args...) \
    CAM_ULOG_DETAILS_R(_module, NSCam::Utils::ULog::DETAILS_INFO, _requestType, _requestSerial, _fmt, ##_args)

#define CAM_ULOG_DECKEY_R(_module, _requestType, _requestSerial, _fmt, _args...) \
    CAM_ULOG_DETAILS_R(_module, NSCam::Utils::ULog::DETAILS_DECISION_KEY, _requestType, _requestSerial, _fmt, ##_args)

#define CAM_ULOGD_R(_module, _requestType, _requestSerial, _fmt, _args...) \
    CAM_ULOG_DETAILS_R(_module, NSCam::Utils::ULog::DETAILS_DEBUG, _requestType, _requestSerial, _fmt, ##_args)

#endif

// For systrace only. Adapt to CAM_TRACE_INT()
#define CAM_ULOG_IVALUE(_module, _name, _value) \
    NSCam::Utils::ULog::ULogger::logValue(_module, __CAM_ULOG_TAG__, _name, _value)

#define CAM_ULOG_NAME(_module, _name) \
    NSCam::Utils::ULog::ULogger::logName(_module, __CAM_ULOG_TAG__, _name)


#define __ulogMustBeStringLiteral(_str) ( "" _str ) // If compilation failed here, means _str is not a literal


// Used by callee, API functions
#define CAM_ULOG_APILIFE(_module) NSCam::Utils::ULog::ULogFuncLife __ulog_func_life__( \
    NSCam::Utils::ULog::getULogModuleId(_module), LOG_TAG, NSCam::Utils::ULog::API_ENTER, __func__)

#define CAM_ULOG_APILIFE_ARGS(_module, _args...) NSCam::Utils::ULog::ULogFuncLife __ulog_func_life__( \
    NSCam::Utils::ULog::getULogModuleId(_module), LOG_TAG, NSCam::Utils::ULog::API_ENTER, __func__, ##_args)

// Always print function trace
// Only if seldom invocation or very important API
#define CAM_ULOG_APILIFE_ALWAYS(_module) NSCam::Utils::ULog::ULogFuncLife __ulog_func_life__( \
    NSCam::Utils::ULog::getULogModuleId(_module), NSCam::Utils::ULog::ULogFuncLife::Always(), LOG_TAG, \
    NSCam::Utils::ULog::API_ENTER, __func__)

// _warnMs is optional, default is 10000 ms
#define CAM_ULOG_APILIFE_GUARD(_module, _warnMs...) NSCam::Utils::ULog::ULogFuncLifeGuard __ulog_func_life__( \
    NSCam::Utils::ULog::getULogModuleId(_module), LOG_TAG, NSCam::Utils::ULog::API_ENTER, __func__, \
    __PRETTY_FUNCTION__, ##_warnMs)

#define CAM_ULOG_APILIFE_CUSTGUARD(_module, _warnMs, _abortMs, _dispatchKey) \
    NSCam::Utils::ULog::ULogFuncLifeGuard __ulog_func_life__( \
    NSCam::Utils::ULog::getULogModuleId(_module), LOG_TAG, NSCam::Utils::ULog::API_ENTER, __func__, \
    __PRETTY_FUNCTION__, _warnMs, _abortMs, __ulogMustBeStringLiteral(_dispatchKey))

// Used by callee, module internal functions
#define CAM_ULOG_FUNCLIFE(_module) NSCam::Utils::ULog::ULogFuncLife __ulog_func_life__( \
    NSCam::Utils::ULog::getULogModuleId(_module), LOG_TAG, NSCam::Utils::ULog::FUNCTION_ENTER, __func__)

#define CAM_ULOG_FUNCLIFE_ARGS(_module, _args...) NSCam::Utils::ULog::ULogFuncLife __ulog_func_life__( \
    NSCam::Utils::ULog::getULogModuleId(_module), LOG_TAG, NSCam::Utils::ULog::FUNCTION_ENTER, __func__, ##_args)

#define CAM_ULOG_FUNCLIFE_IF(_module, _cond) NSCam::Utils::ULog::ULogFuncLife __ulog_func_life__( \
    NSCam::Utils::ULog::getULogModuleId(_module), (_cond), LOG_TAG, NSCam::Utils::ULog::FUNCTION_ENTER, __func__)

// Always print function trace
// Only if seldom invocation or very important function
#define CAM_ULOG_FUNCLIFE_ALWAYS(_module) NSCam::Utils::ULog::ULogFuncLife __ulog_func_life__( \
    NSCam::Utils::ULog::getULogModuleId(_module), NSCam::Utils::ULog::ULogFuncLife::Always(), LOG_TAG, \
    NSCam::Utils::ULog::FUNCTION_ENTER, __func__)

// _warnMs is optional, default is 10000 ms
#define CAM_ULOG_FUNCLIFE_GUARD(_module, _warnMs...) NSCam::Utils::ULog::ULogFuncLifeGuard __ulog_func_life__( \
    NSCam::Utils::ULog::getULogModuleId(_module), LOG_TAG, NSCam::Utils::ULog::FUNCTION_ENTER, \
    __func__, __PRETTY_FUNCTION__, ##_warnMs)

#define CAM_ULOG_FUNCLIFE_CUSTGUARD(_module, _warnMs, _abortMs, _dispatchKey) \
    NSCam::Utils::ULog::ULogFuncLifeGuard __ulog_func_life__( \
    NSCam::Utils::ULog::getULogModuleId(_module), LOG_TAG, NSCam::Utils::ULog::FUNCTION_ENTER, \
    __func__, __PRETTY_FUNCTION__, _warnMs, _abortMs, __ulogMustBeStringLiteral(_dispatchKey))

// Used by caller, but not restricted
#define CAM_ULOG_TAGLIFE(_module, _literalTag) NSCam::Utils::ULog::ULogFuncLife __ulog_tag_life__( \
    NSCam::Utils::ULog::getULogModuleId(_module), LOG_TAG, NSCam::Utils::ULog::SUBROUTINE_ENTER, \
    __ulogMustBeStringLiteral(_literalTag))

#define CAM_ULOG_TAGLIFE_VARS(_module, _literalTag, _vars...) NSCam::Utils::ULog::ULogFuncLife __ulog_func_life__( \
    NSCam::Utils::ULog::getULogModuleId(_module), LOG_TAG, NSCam::Utils::ULog::SUBROUTINE_ENTER, \
    __ulogMustBeStringLiteral(_literalTag), ##_vars)

#define CAM_ULOG_TAGLIFE_IF(_module, _literalTag, _cond) NSCam::Utils::ULog::ULogFuncLife __ulog_tag_life__( \
    NSCam::Utils::ULog::getULogModuleId(_module), (_cond), LOG_TAG, NSCam::Utils::ULog::SUBROUTINE_ENTER, \
    __ulogMustBeStringLiteral(_literalTag))

// Always print tag trace
// Only if seldom invocation or very important routine
#define CAM_ULOG_TAGLIFE_ALWAYS(_module, _literalTag) NSCam::Utils::ULog::ULogFuncLife __ulog_func_life__( \
    NSCam::Utils::ULog::getULogModuleId(_module), NSCam::Utils::ULog::ULogFuncLife::Always(), LOG_TAG, \
    NSCam::Utils::ULog::SUBROUTINE_ENTER, __ulogMustBeStringLiteral(_literalTag))

#define CAM_ULOG_TAGLIFE_GUARD(_module, _literalTag, _warnMs...) NSCam::Utils::ULog::ULogFuncLifeGuard __ulog_tag_life__( \
    NSCam::Utils::ULog::getULogModuleId(_module), LOG_TAG, NSCam::Utils::ULog::SUBROUTINE_ENTER, \
    __ulogMustBeStringLiteral(_literalTag), ##_warnMs)

#define CAM_ULOG_TAGLIFE_CUSTGUARD(_module, _literalTag, _warnMs, _abortMs, _dispatchKey) \
    NSCam::Utils::ULog::ULogFuncLifeGuard __ulog_tag_life__( \
    NSCam::Utils::ULog::getULogModuleId(_module), LOG_TAG, NSCam::Utils::ULog::SUBROUTINE_ENTER, \
    __ulogMustBeStringLiteral(_literalTag), _warnMs, _abortMs, __ulogMustBeStringLiteral(_dispatchKey))


#define CAM_ULOG_TAG_BEGIN(_module, _literalTag) { CAM_ULOG_TAGLIFE(_module, _literalTag);
#define CAM_ULOG_TAG_END() }

#define CAM_ULOG_DTAG_BEGIN(_module, _condEval, _fmt, _args...) { \
    char _dtagBuffer[256]; \
    bool _cond = (_condEval); \
    if (_cond) { ::snprintf(_dtagBuffer, sizeof(_dtagBuffer), _fmt, ##_args); } else { _dtagBuffer[0] = '\0'; } \
    NSCam::Utils::ULog::ULogFuncLife __ulog_func_life__( \
        NSCam::Utils::ULog::getULogModuleId(_module), (_cond), LOG_TAG, NSCam::Utils::ULog::SUBROUTINE_ENTER, _dtagBuffer);

#define CAM_ULOG_DTAG_END() }

#define CAM_ULOG_TAG_CUSTGUARD_BEGIN(_module, _literalTag, _warnMs, _abortMs, _dispatchKey) { \
    CAM_ULOG_TAGLIFE_CUSTGUARD(_module, _literalTag, _warnMs, _abortMs, _dispatchKey);

#define CAM_ULOG_TAG_CUSTGUARD_END() }


// Clang can not inline variable arguments well, we inline it manually.
#define CAM_ULOG_DETAILS_FILE(_uModuleId, _detailsType, _fmt, _args...) \
    do { \
        char _buffer[__CAM_ULOG_DETAILS_BUFFER_SIZE__]; \
        size_t _contentLen = ::snprintf(_buffer, sizeof(_buffer), _fmt, ##_args); \
        _buffer[__CAM_ULOG_DETAILS_BUFFER_SIZE__ - 1] = '\0'; \
        NSCam::Utils::ULog::ULogger::logNormalDetails(_uModuleId, LOG_TAG, _detailsType, _buffer, _contentLen); \
    } while (0)


#define CAM_ULOG_TIMEBOMB_CREATE(_module, _serial, _dispatchKey, _warnMs, _abortMs, _maxAbortBound...) \
    NSCam::Utils::ULog::ULogTimeBomb::create(NSCam::Utils::ULog::ULogTimeBomb::please_use_CAM_ULOG_TIMEBOMB_CREATE(), \
        NSCam::Utils::ULog::getULogModuleId(_module), _serial, \
        _dispatchKey, _warnMs, _abortMs, ##_maxAbortBound)


#if !defined(USING_MTK_LDVT)

// Why we don't invoke CAM_LOGx() here?
// 1. We don't wait to include Log.h
// 2. The implementation is inefficient

// We don't redirect to LOG_FATAL immediately, because when NDEBUG is defined it will be no effect
// However, module can add
// LOCAL_CFLAGS += -DULOG_FATAL_DEFAULT_BEHAVIOR
// in Android.mk to apply the default behavior
#if defined(ULOG_FATAL_DEFAULT_BEHAVIOR)
#define _ULOG_FATAL_ACTION(_fmt, _args...) LOG_FATAL(_fmt " (%s){#%d:%s}", ##_args, __func__, __LINE__, __FILE__)
#else
#define _ULOG_FATAL_ACTION(_fmt, _args...) LOG_ALWAYS_FATAL(_fmt " (%s){#%d:%s}", ##_args, __func__, __LINE__, __FILE__)
#endif

#define CAM_ULOG_FATAL(_module, _fmt, _args...) \
    do { \
        if (NSCam::Utils::ULog::ULogger::isFileModeEnabled()) { \
            CAM_ULOG_DETAILS_FILE(NSCam::Utils::ULog::getULogModuleId(_module), NSCam::Utils::ULog::DETAILS_ERROR, _fmt, ##_args); \
            NSCam::Utils::ULog::ULogger::flush(); \
        } \
        _ULOG_FATAL_ACTION(_fmt, ##_args); \
    } while (0)

#if defined(__clang__) || defined(__GNUC__)
#define CAM_ULOG_ASSERT(_module, _cond, _fmt, _args...) \
    do { if (__builtin_expect((!(_cond)), false)) { CAM_ULOG_FATAL(_module, _fmt, ##_args); } } while (0)
#else
#define CAM_ULOG_ASSERT(_module, _cond, _fmt, _args...) \
    do { if (!(_cond)) { CAM_ULOG_FATAL(_module, _fmt, ##_args); } } while (0)
#endif

#define CAM_ULOGE(_module, _fmt, _args...) \
    do { \
        if (NSCam::Utils::ULog::ULogger::isAndroidModeEnabled()) { \
            ALOGE(_fmt " (%s){#%d:%s}", ##_args, __func__, __LINE__, __FILE__); \
        } \
        if (NSCam::Utils::ULog::ULogger::isFileModeEnabled()) { \
            CAM_ULOG_DETAILS_FILE(NSCam::Utils::ULog::getULogModuleId(_module), NSCam::Utils::ULog::DETAILS_ERROR, _fmt, ##_args); \
        } \
    } while (0)

#define CAM_ULOGW(_module, _fmt, _args...) \
    do { \
        NSCam::Utils::ULog::ModuleId _uModuleId = NSCam::Utils::ULog::getULogModuleId(_module); \
        if (NSCam::Utils::ULog::ULogger::isNormalDetailsEnabled(_uModuleId, NSCam::Utils::ULog::DETAILS_WARNING)) { \
            if (NSCam::Utils::ULog::ULogger::isAndroidModeEnabled()) { ALOGW(_fmt, ##_args); } \
            if (NSCam::Utils::ULog::ULogger::isFileModeEnabled()) { \
                CAM_ULOG_DETAILS_FILE(_uModuleId, NSCam::Utils::ULog::DETAILS_WARNING, _fmt, ##_args); \
            } \
        } \
    } while (0)

#define CAM_ULOGI(_module, _fmt, _args...) \
    do { \
        NSCam::Utils::ULog::ModuleId _uModuleId = NSCam::Utils::ULog::getULogModuleId(_module); \
        if (NSCam::Utils::ULog::ULogger::isNormalDetailsEnabled(_uModuleId, NSCam::Utils::ULog::DETAILS_INFO)) { \
            if (NSCam::Utils::ULog::ULogger::isAndroidModeEnabled()) { ALOGI(_fmt, ##_args); } \
            if (NSCam::Utils::ULog::ULogger::isFileModeEnabled()) { \
                CAM_ULOG_DETAILS_FILE(_uModuleId, NSCam::Utils::ULog::DETAILS_INFO, _fmt, ##_args); \
            } \
        } \
    } while (0)

// Only cross-module information can be tagged with DECKEY
#define CAM_ULOG_DECKEY(_module, _fmt, _args...) \
    do { \
        NSCam::Utils::ULog::ModuleId _uModuleId = NSCam::Utils::ULog::getULogModuleId(_module); \
        if (NSCam::Utils::ULog::ULogger::isNormalDetailsEnabled(_uModuleId, NSCam::Utils::ULog::DETAILS_DECISION_KEY)) { \
            if (NSCam::Utils::ULog::ULogger::isAndroidModeEnabled()) { ALOGD(_fmt "  #DECKEY", ##_args); } \
            if (NSCam::Utils::ULog::ULogger::isFileModeEnabled()) { \
                CAM_ULOG_DETAILS_FILE(_uModuleId, NSCam::Utils::ULog::DETAILS_DECISION_KEY, _fmt "  #DECKEY", ##_args); \
            } \
        } \
    } while (0)

#define CAM_ULOGD(_module, _fmt, _args...) \
    do { \
        NSCam::Utils::ULog::ModuleId _uModuleId = NSCam::Utils::ULog::getULogModuleId(_module); \
        if (NSCam::Utils::ULog::ULogger::isNormalDetailsEnabled(_uModuleId, NSCam::Utils::ULog::DETAILS_DEBUG)) { \
            if (NSCam::Utils::ULog::ULogger::isAndroidModeEnabled()) { ALOGD(_fmt, ##_args); } \
            if (NSCam::Utils::ULog::ULogger::isFileModeEnabled()) { \
                CAM_ULOG_DETAILS_FILE(_uModuleId, NSCam::Utils::ULog::DETAILS_DEBUG, _fmt, ##_args); \
            } \
        } \
    } while (0)

#define CAM_ULOGV(_module, _fmt, _args...) \
    do { \
        NSCam::Utils::ULog::ModuleId _uModuleId = NSCam::Utils::ULog::getULogModuleId(_module); \
        if (NSCam::Utils::ULog::ULogger::isNormalDetailsEnabled(_uModuleId, NSCam::Utils::ULog::DETAILS_VERBOSE)) { \
            if (NSCam::Utils::ULog::ULogger::isAndroidModeEnabled()) { ALOGV(_fmt, ##_args); } \
            if (NSCam::Utils::ULog::ULogger::isFileModeEnabled()) { \
                CAM_ULOG_DETAILS_FILE(_uModuleId, NSCam::Utils::ULog::DETAILS_VERBOSE, _fmt, ##_args); \
            } \
        } \
    } while (0)


#else // USING_MTK_LDVT

#include <mtkcam/utils/std/Log.h>

#define CAM_ULOG_FATAL(_module, _fmt, _args...) CAM_LOGF(_fmt, ##_args)
#define CAM_ULOG_ASSERT(_module, _cond, _fmt, _args...) do { if (!(_cond)) { CAM_LOGA(_fmt, _args); } } while (0)
#define CAM_ULOGE(_module, _fmt, _args...) CAM_LOGE(_fmt, ##_args)
#define CAM_ULOGW(_module, _fmt, _args...) CAM_LOGW(_fmt, ##_args)
#define CAM_ULOG_DECKEY(_module, _fmt, _args...) CAM_LOGI(_fmt, ##_args)
#define CAM_ULOGI(_module, _fmt, _args...) CAM_LOGI(_fmt, ##_args)
#define CAM_ULOGD(_module, _fmt, _args...) CAM_LOGD(_fmt, ##_args)
#define CAM_ULOGV(_module, _fmt, _args...) CAM_LOGV(_fmt, ##_args)

#endif // USING_MTK_LDVT

// -------------------------------------------------------------------------

#define CAM_ULOG_DECLARE_MODULE_ID(_moduleId) \
    static constexpr NSCam::Utils::ULog::ModuleId __ULOG_MODULE_ID = NSCam::Utils::ULog::_moduleId ;

#define CAM_ULOGM_SUBREQS(_requestType, _requestSerial, _subrequestType, _subrequestSerial) \
    CAM_ULOG_SUBREQS(__ULOG_MODULE_ID, _requestType, _requestSerial, _subrequestType, _subrequestSerial)

#define CAM_ULOGME_R(_requestType, _requestSerial, _fmt, _args...) \
    CAM_ULOGE_R(__ULOG_MODULE_ID, _requestType, _requestSerial, _fmt, ##_args)

#define CAM_ULOGMW_R(_requestType, _requestSerial, _fmt, _args...) \
    CAM_ULOGW_R(__ULOG_MODULE_ID, _requestType, _requestSerial, _fmt, ##_args)

#define CAM_ULOGMI_R(_requestType, _requestSerial, _fmt, _args...) \
    CAM_ULOGI_R(__ULOG_MODULE_ID, _requestType, _requestSerial, _fmt, ##_args)

#define CAM_ULOGM_DECKEY_R(_requestType, _requestSerial, _fmt, _args...) \
    CAM_ULOG_DECKEY_R(__ULOG_MODULE_ID, _requestType, _requestSerial, _fmt, ##_args)

#define CAM_ULOGMD_R(_requestType, _requestSerial, _fmt, _args...) \
    CAM_ULOGD_R(__ULOG_MODULE_ID, _requestType, _requestSerial, _fmt, ##_args)

// For systrace only. Adapt to CAM_TRACE_INT()
#define CAM_ULOGM_IVALUE(_name, value) \
    CAM_ULOG_IVALUE(__ULOG_MODULE_ID, _name, value)

#define CAM_ULOGM_NAME(_name) CAM_ULOG_NAME(__ULOG_MODULE_ID, _name)


#define CAM_ULOGM_APILIFE() CAM_ULOG_APILIFE(__ULOG_MODULE_ID)
#define CAM_ULOGM_APILIFE_ARGS(_args...) CAM_ULOG_APILIFE_ARGS(__ULOG_MODULE_ID, ##_args)
#define CAM_ULOGM_APILIFE_ALWAYS() CAM_ULOG_APILIFE_ALWAYS(__ULOG_MODULE_ID)
#define CAM_ULOGM_APILIFE_GUARD(_warnMs...) CAM_ULOG_APILIFE_GUARD(__ULOG_MODULE_ID, ##_warnMs)
#define CAM_ULOGM_APILIFE_CUSTGUARD(_params...) CAM_ULOG_APILIFE_CUSTGUARD(__ULOG_MODULE_ID, ##_params)
#define CAM_ULOGM_FUNCLIFE() CAM_ULOG_FUNCLIFE(__ULOG_MODULE_ID)
#define CAM_ULOGM_FUNCLIFE_ARGS(_args...) CAM_ULOG_FUNCLIFE_ARGS(__ULOG_MODULE_ID, ##_args)
#define CAM_ULOGM_FUNCLIFE_IF(_cond) CAM_ULOG_FUNCLIFE_IF(__ULOG_MODULE_ID, _cond)
#define CAM_ULOGM_FUNCLIFE_ALWAYS() CAM_ULOG_FUNCLIFE_ALWAYS(__ULOG_MODULE_ID)
#define CAM_ULOGM_FUNCLIFE_GUARD(_warnMs...) CAM_ULOG_FUNCLIFE_GUARD(__ULOG_MODULE_ID, ##_warnMs)
#define CAM_ULOGM_FUNCLIFE_CUSTGUARD(_params...) CAM_ULOG_FUNCLIFE_GUARD(__ULOG_MODULE_ID, ##_params)
#define CAM_ULOGM_TAGLIFE(_literalTag) CAM_ULOG_TAGLIFE(__ULOG_MODULE_ID, _literalTag)
#define CAM_ULOGM_TAGLIFE_VARS(_literalTag, _vars...) CAM_ULOG_TAGLIFE_VARS(__ULOG_MODULE_ID, _literalTag, ##_vars)
#define CAM_ULOGM_TAGLIFE_IF(_literalTag, _cond) CAM_ULOG_TAGLIFE_IF(__ULOG_MODULE_ID, _literalTag, _cond)
#define CAM_ULOGM_TAGLIFE_ALWAYS(_literalTag) CAM_ULOG_TAGLIFE_ALWAYS(__ULOG_MODULE_ID, _literalTag)
#define CAM_ULOGM_TAGLIFE_GUARD(_literalTag, _warnMs...) CAM_ULOG_TAGLIFE_GUARD(__ULOG_MODULE_ID, _literalTag, ##_warnMs)
#define CAM_ULOGM_TAGLIFE_CUSTGUARD(_literalTag, _params...) CAM_ULOG_TAGLIFE_CUSTGUARD(__ULOG_MODULE_ID, _literalTag, ##_params)

#define CAM_ULOGM_TAG_BEGIN(_literalTag) CAM_ULOG_TAG_BEGIN(__ULOG_MODULE_ID, _literalTag)
#define CAM_ULOGM_TAG_END() CAM_ULOG_TAG_END()

#define CAM_ULOGM_DTAG_BEGIN(_cond, _fmt, _args...) CAM_ULOG_DTAG_BEGIN(__ULOG_MODULE_ID, _cond, _fmt, ##_args)
#define CAM_ULOGM_DTAG_END() CAM_ULOG_DTAG_END()

#define CAM_ULOGM_TAG_CUSTGUARD_BEGIN(_literalTag, _params...) CAM_ULOG_TAG_CUSTGUARD_BEGIN(__ULOG_MODULE_ID, _literalTag, ##_params)
#define CAM_ULOGM_TAG_CUSTGUARD_END() CAM_ULOG_TAG_CUSTGUARD_END()

#define CAM_ULOGM_FATAL(_fmt, _args...) CAM_ULOG_FATAL(__ULOG_MODULE_ID, _fmt, ##_args)
#define CAM_ULOGM_ASSERT(_cond, _fmt, _args...) CAM_ULOG_ASSERT(__ULOG_MODULE_ID, _cond, _fmt, ##_args)
#define CAM_ULOGME(_fmt, _args...) CAM_ULOGE(__ULOG_MODULE_ID, _fmt, ##_args)
#define CAM_ULOGMW(_fmt, _args...) CAM_ULOGW(__ULOG_MODULE_ID, _fmt, ##_args)
#define CAM_ULOGMI(_fmt, _args...) CAM_ULOGI(__ULOG_MODULE_ID, _fmt, ##_args)
#define CAM_ULOGM_DECKEY(_fmt, _args...) CAM_ULOG_DECKEY(__ULOG_MODULE_ID, _fmt, ##_args)
#define CAM_ULOGMD(_fmt, _args...) CAM_ULOGD(__ULOG_MODULE_ID, _fmt, ##_args)
#define CAM_ULOGMV(_fmt, _args...) CAM_ULOGV(__ULOG_MODULE_ID, _fmt, ##_args)


#endif

