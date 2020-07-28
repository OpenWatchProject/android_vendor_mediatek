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

#ifndef __MTKCAM_UTILS_STD_ULOG_CLASS_H__
#define __MTKCAM_UTILS_STD_ULOG_CLASS_H__

#include <cstddef>
#include <cstdarg>
#include <cinttypes>
#include <cstdio>
#include <utility>
#include <type_traits>
#include <vector>
#include <atomic>
#include <string>
#include <time.h>
#include <utils/RefBase.h>
#include <mtkcam/utils/std/ULogDef.h>


namespace NSCam {
namespace Utils {
namespace ULog {

/**
 * A very light-weighted & size-fixed vector, efficient for log.
 * The interface is std::vector<>-like so that user can migrate painlessly.
 */
template <typename _T>
class ULogVector final
{
public:
    typedef _T *iterator;
    typedef const _T *const_iterator;
    typedef _T &reference;
    typedef const _T &const_reference;

    /** Maximum number of elements */
    static constexpr size_t MAX_ELEMENTS_NUM = 10;

    ULogVector() : mArrayEnd(0) { }

    template <typename _U>
    ULogVector(const std::initializer_list<_U> &list) {
        mArrayEnd = 0;
        for (auto id : list) {
            if (mArrayEnd >= MAX_ELEMENTS_NUM)
                break;
            mArray[mArrayEnd] = id;
            mArrayEnd++;
        }
    }

    template <typename _InputIt>
    ULogVector(_InputIt first, _InputIt last) {
        mArrayEnd = 0;
        for (_InputIt it = first; it != last; it++) {
            if (mArrayEnd >= MAX_ELEMENTS_NUM)
                break;
            mArray[mArrayEnd] = *it;
            mArrayEnd++;
        }
    }

    void push_back(const _T &v) {
        if (mArrayEnd < MAX_ELEMENTS_NUM) {
            mArray[mArrayEnd] = v;
            mArrayEnd++;
        } else {
            // Ignore remaining
        }
    }

    iterator begin() {
        return mArray;
    }

    iterator end() {
        return &mArray[mArrayEnd];
    }

    const_iterator begin() const {
        return cbegin();
    }

    const_iterator end() const {
        return cend();
    }

    const_iterator cbegin() const {
        return mArray;
    }

    const_iterator cend() const {
        return &mArray[mArrayEnd];
    }

    size_t capability() const {
        return MAX_ELEMENTS_NUM;
    }

    size_t max_size() const {
        return MAX_ELEMENTS_NUM;
    }

    bool empty() const {
        return (mArrayEnd == 0);
    }

    size_t size() const {
        return mArrayEnd;
    }

    void clear() {
        mArrayEnd = 0;
    }

    _T* ptr() {
        return mArray;
    }

    const _T* ptr() const {
        return mArray;
    }

    // Other function are unnecessary, we don't provide

private:

    _T mArray[MAX_ELEMENTS_NUM];
    size_t mArrayEnd;
};


typedef ULogVector<ModuleId> ModuleList;


enum FuncLifeTag
{
    API_ENTER           = 0x0,
    FUNCTION_ENTER      = 0x1,
    SUBROUTINE_ENTER    = 0x2,
    EXIT_BIT            = 0x4,
    API_EXIT            = (EXIT_BIT | API_ENTER),
    FUNCTION_EXIT       = (EXIT_BIT | FUNCTION_ENTER),
    SUBROUTINE_EXIT     = (EXIT_BIT | SUBROUTINE_ENTER)
};

enum DetailsType
{
    DETAILS_INVALID      = 0,
    DETAILS_ERROR        = 1,
    DETAILS_WARNING      = 2,
    DETAILS_INFO         = 3,
    DETAILS_DECISION_KEY = 4,
    DETAILS_DEBUG        = 5,
    DETAILS_VERBOSE      = 6
};


inline static bool _isPassFilter(ModuleId moduleId, unsigned int mask)
{
    return (moduleId & ULOG_LAYER_MASK & mask) != 0 &&
           (moduleId & ULOG_GROUP_MASK & mask) != 0;
}


class ULogRuntimeDiag
{
    friend class ULogInitializerImpl;

private:
    static ULogRuntimeDiag *get() {
        return sDiag;
    }

public:
    static bool isEnabled() {
        return (sDiag != nullptr);
    }

    static void logEnter(ModuleId intoModuleId, RequestTypeId requestTypeId, RequestSerial requestSerial) {
        if (isEnabled())
            get()->onLogEnter(intoModuleId, requestTypeId, requestSerial);
    }

    static void logExit(ModuleId outFromModuleId, RequestTypeId requestTypeId, RequestSerial requestSerial) {
        if (isEnabled())
            get()->onLogExit(outFromModuleId, requestTypeId, requestSerial);
    }

    static void logDiscard(ModuleId byModuleId, RequestTypeId requestTypeId, RequestSerial requestSerial) {
        if (isEnabled())
            get()->onLogDiscard(byModuleId, requestTypeId, &requestSerial, 1);
    }

    static void logDiscard(ModuleId byModuleId, RequestTypeId requestTypeId,
        const std::vector<RequestSerial> &requestSerialList)
    {
        if (isEnabled() && requestSerialList.size() > 0)
            get()->onLogDiscard(byModuleId, requestTypeId, &*requestSerialList.begin(), requestSerialList.size());
    }

    static void logSubreqs(ModuleId byModuleId, RequestTypeId requestTypeId, RequestSerial requestSerial,
        RequestTypeId subrequestTypeId, RequestSerial subrequestSerial)
    {
        if (isEnabled())
            get()->onLogSubreqs(byModuleId, requestTypeId, requestSerial, subrequestTypeId, &subrequestSerial, 1);
    }

    static void logSubreqs(ModuleId byModuleId, RequestTypeId requestTypeId, RequestSerial requestSerial,
        RequestTypeId subrequestTypeId, const ULogVector<RequestSerial> &subrequestSerialList)
    {
        if (isEnabled() && subrequestSerialList.size() > 0) {
            get()->onLogSubreqs(byModuleId, requestTypeId, requestSerial, subrequestTypeId,
                subrequestSerialList.ptr(), subrequestSerialList.size());
        }
    }

    static void logSubreqs(ModuleId byModuleId, RequestTypeId requestTypeId, RequestSerial requestSerial,
        RequestTypeId subrequestTypeId, const std::vector<RequestSerial> &subrequestSerialList)
    {
        if (isEnabled() && subrequestSerialList.size() > 0) {
            get()->onLogSubreqs(byModuleId, requestTypeId, requestSerial, subrequestTypeId,
                &*subrequestSerialList.begin(), subrequestSerialList.size());
        }
    }

    static void dumpToLog();

private:
    static ULogRuntimeDiag *sDiag;

    static void onLogEnter(ModuleId intoModuleId, RequestTypeId requestTypeId, RequestSerial requestSerial);
    static void onLogExit(ModuleId outFromModuleId, RequestTypeId requestTypeId, RequestSerial requestSerial);
    static void onLogSubreqs(ModuleId byModuleId, RequestTypeId requestTypeId, RequestSerial requestSerial,
        RequestTypeId subrequestTypeId, const RequestSerial *subrequestSerialList, size_t n);
    static void onLogDiscard(ModuleId byModuleId, RequestTypeId requestTypeId, const RequestSerial *requestSerialList, size_t n);

    static void onInit();
    static void onUninit();
};


class ULogger
{
private:
    static ULogger *sULogger;
    static unsigned int sMode;
    static unsigned int sModFilterMask; // not atomic, but only set in program init
    static unsigned int sDetailsLevel;
    static unsigned int sNormalDetailsMask;

    // We must declare performance-sensitive functions here so that they can be inlined
    // To implement following APIs, we can note that moduleId & detailsType are maybe
    // constants, we should maximize the effectiveness of compile-time optimization
    // EVERY CHANGE MUST BE DISASSEMBLED TO REVIEW

    inline static bool isAboveFilterLayers(ModuleId moduleId) {
        return ((moduleId & ULOG_LAYER_MASK) <= (sModFilterMask & ULOG_LAYER_MASK));
    }

    static ULogger *get() {
        // Never be null, we can eliminate the check overhead
        return sULogger;
    }

    inline static const char *noNull(const char *s) {
        return (s != nullptr) ? s : "";
    }

public:
    enum ULogMode
    {
        ANDROID_LOG     = 0x01,
        TEXT_FILE_LOG   = 0x02,
        BIN_FILE_LOG    = 0x04,
        PASSIVE_LOG     = 0x08,
        TRACE_LOG       = 0x10,
        DIAGNOSTIC      = 0x20
    };

    inline static bool isLogEnabled(ModuleId moduleId) {
        return _isPassFilter(moduleId, sModFilterMask);
    }

    static bool isModeEnabled(unsigned int mode) {
        return (sMode & mode) != 0;
    }

    static bool isAndroidModeOnly() {
        return (sMode == ANDROID_LOG);
    }

    static bool isAndroidModeEnabled() {
        return isModeEnabled(ANDROID_LOG);
    }

    static bool isFileModeEnabled() {
        return isModeEnabled(TEXT_FILE_LOG | BIN_FILE_LOG | PASSIVE_LOG);
    }

    static bool isDetailsEnabled(DetailsType detailsType) {
        return (static_cast<unsigned int>(detailsType) <= sDetailsLevel);
    }

    inline static bool isLogEnabled(ModuleId moduleId, DetailsType detailsType) {
        return isLogEnabled(moduleId) && isDetailsEnabled(detailsType);
    }

    static void logEnter(ModuleId intoModuleId, const char *tag, RequestTypeId requestTypeId, RequestSerial requestSerial) {
        if (isLogEnabled(intoModuleId))
            get()->onLogEnter(intoModuleId, noNull(tag), requestTypeId, requestSerial);
        ULogRuntimeDiag::logEnter(intoModuleId, requestTypeId, requestSerial);
    }

    static void logExit(ModuleId outFromModuleId, const char *tag, RequestTypeId requestTypeId, RequestSerial requestSerial) {
        if (isLogEnabled(outFromModuleId))
            get()->onLogExit(outFromModuleId, noNull(tag), requestTypeId, requestSerial);
        ULogRuntimeDiag::logExit(outFromModuleId, requestTypeId, requestSerial);
    }

    static void logDiscard(ModuleId byModuleId, const char *tag, RequestTypeId requestTypeId, RequestSerial requestSerial) {
        if (isLogEnabled(byModuleId))
            get()->onLogDiscard(byModuleId, noNull(tag), requestTypeId, &requestSerial, 1);
        ULogRuntimeDiag::logDiscard(byModuleId, requestTypeId, requestSerial);
    }

    static void logDiscard(ModuleId byModuleId, const char *tag, RequestTypeId requestTypeId,
        const std::vector<RequestSerial> &requestSerialList)
    {
        if (isLogEnabled(byModuleId) && requestSerialList.size() > 0)
            get()->onLogDiscard(byModuleId, noNull(tag), requestTypeId, &*requestSerialList.begin(), requestSerialList.size());
        ULogRuntimeDiag::logDiscard(byModuleId, requestTypeId, requestSerialList);
    }

    static void logSubreqs(ModuleId byModuleId, const char *tag, RequestTypeId requestTypeId, RequestSerial requestSerial,
        RequestTypeId subrequestTypeId, RequestSerial subrequestSerial)
    {
        if (isAboveFilterLayers(byModuleId))
            get()->onLogSubreqs(byModuleId, noNull(tag), requestTypeId, requestSerial, subrequestTypeId, &subrequestSerial, 1);
        ULogRuntimeDiag::logSubreqs(byModuleId, requestTypeId, requestSerial, subrequestTypeId, subrequestSerial);
    }

    static void logSubreqs(ModuleId byModuleId, const char *tag, RequestTypeId requestTypeId, RequestSerial requestSerial,
        RequestTypeId subrequestTypeId, const ULogVector<RequestSerial> &subrequestSerialList)
    {
        if (isAboveFilterLayers(byModuleId) && subrequestSerialList.size() > 0) {
            get()->onLogSubreqs(byModuleId, noNull(tag), requestTypeId, requestSerial, subrequestTypeId,
                subrequestSerialList.ptr(), subrequestSerialList.size());
        }
        ULogRuntimeDiag::logSubreqs(byModuleId, requestTypeId, requestSerial, subrequestTypeId, subrequestSerialList);
    }

    static void logSubreqs(ModuleId byModuleId, const char *tag, RequestTypeId requestTypeId, RequestSerial requestSerial,
        RequestTypeId subrequestTypeId, const std::vector<RequestSerial> &subrequestSerialList)
    {
        if (isAboveFilterLayers(byModuleId) && subrequestSerialList.size() > 0) {
            get()->onLogSubreqs(byModuleId, noNull(tag), requestTypeId, requestSerial, subrequestTypeId,
                &*subrequestSerialList.begin(), subrequestSerialList.size());
        }
        ULogRuntimeDiag::logSubreqs(byModuleId, requestTypeId, requestSerial, subrequestTypeId, subrequestSerialList);
    }

    static void logPathDiv(ModuleId fromModuleId, const char *tag, RequestTypeId requestTypeId, RequestSerial requestSerial,
        ModuleId toModuleId)
    {
        if (isLogEnabled(fromModuleId, DETAILS_DECISION_KEY))
            get()->onLogPathDiv(fromModuleId, noNull(tag), requestTypeId, requestSerial, &toModuleId, 1);
    }

    static void logPathDiv(ModuleId fromModuleId, const char *tag, RequestTypeId requestTypeId, RequestSerial requestSerial,
        const ULogVector<ModuleId> &toModuleList)
    {
        if (isLogEnabled(fromModuleId, DETAILS_DECISION_KEY) && toModuleList.size() > 0)
            get()->onLogPathDiv(fromModuleId, noNull(tag), requestTypeId, requestSerial, toModuleList.ptr(), toModuleList.size());
    }

    static void logPathDiv(ModuleId fromModuleId, const char *tag, RequestTypeId requestTypeId, RequestSerial requestSerial,
        const std::vector<ModuleId> &toModuleList)
    {
        if (isLogEnabled(fromModuleId, DETAILS_DECISION_KEY) && toModuleList.size() > 0)
            get()->onLogPathDiv(fromModuleId, noNull(tag), requestTypeId, requestSerial, &*toModuleList.begin(), toModuleList.size());
    }

    static void logPathJoin(ModuleId toModuleId, const char *tag, RequestTypeId requestTypeId, RequestSerial requestSerial,
        ModuleId fromModuleId)
    {
        if (isLogEnabled(toModuleId, DETAILS_DECISION_KEY))
            get()->onLogPathJoin(toModuleId, noNull(tag), requestTypeId, requestSerial, &fromModuleId, 1);
    }

    static void logPathJoin(ModuleId toModuleId, const char *tag, RequestTypeId requestTypeId, RequestSerial requestSerial,
        const ULogVector<ModuleId> &fromModuleList)
    {
        if (isLogEnabled(toModuleId, DETAILS_DECISION_KEY) && fromModuleList.size() > 0)
            get()->onLogPathJoin(toModuleId, noNull(tag), requestTypeId, requestSerial, fromModuleList.ptr(), fromModuleList.size());
    }

    static void logPathJoin(ModuleId toModuleId, const char *tag, RequestTypeId requestTypeId, RequestSerial requestSerial,
        const std::vector<ModuleId> &fromModuleList)
    {
        if (isLogEnabled(toModuleId, DETAILS_DECISION_KEY) && fromModuleList.size() > 0)
            get()->onLogPathJoin(toModuleId, noNull(tag), requestTypeId, requestSerial, &*fromModuleList.begin(), fromModuleList.size());
    }

    static bool isReqDetailsEnabled(ModuleId moduleId, DetailsType type) {
        return isDetailsEnabled(type) && (isLogEnabled(moduleId) || _isPassFilter(moduleId, sNormalDetailsMask));
    }

    static void logReqDetails(ModuleId moduleId, const char *tag, DetailsType type,
        RequestTypeId requestTypeId, RequestSerial requestSerial, const char *content, size_t contentLen)
    {
        get()->onLogDetails(moduleId, noNull(tag), type, requestTypeId, requestSerial, content, contentLen);
    }

    inline static bool isNormalDetailsEnabled(ModuleId moduleId, DetailsType type) {
        // Warning can be disabled
        // If warning is enabled, all warnings of all modules can be printed
        return isDetailsEnabled(type) && (type <= DETAILS_WARNING || _isPassFilter(moduleId, sNormalDetailsMask));
    }

    static void logNormalDetails(ModuleId moduleId, const char *tag, DetailsType type, const char *content, size_t contentLen) {
        get()->onLogDetails(moduleId, noNull(tag), type, REQ_INVALID_ID, 0, content, contentLen);
    }

    static void logValue(ModuleId moduleId, const char *tag, const char *name, std::int32_t value) {
        if (isLogEnabled(moduleId))
            get()->onLogValue(moduleId, noNull(tag), name, value);
    }

    static void logName(ModuleId moduleId, const char *tag, const char *name) {
        logValue(moduleId, tag, name, 0);
    }

    static void addModFilterMask(unsigned int mask);

    static unsigned int getModFilterMask() {
        return sModFilterMask;
    }

    static DetailsType getDetailsLevel() {
        return static_cast<DetailsType>(sDetailsLevel);
    }

    static void flush(int waitDoneSec = 1) {
        get()->onFlush(waitDoneSec);
    }

public:
    // Following order are sorted by calling frequency for locality
    virtual void onLogEnter(ModuleId intoModuleId, const char *tag, RequestTypeId requestTypeId, RequestSerial requestSerial) = 0;
    virtual void onLogExit(ModuleId outFromModuleId, const char *tag, RequestTypeId requestTypeId, RequestSerial requestSerial) = 0;
    virtual void onLogSubreqs(ModuleId byModuleId, const char *tag, RequestTypeId requestTypeId, RequestSerial requestSerial,
        RequestTypeId subrequestTypeId, const RequestSerial *subrequestSerialList, size_t n);
    virtual void onLogDetails(ModuleId moduleId, const char *tag, DetailsType type,
        RequestTypeId requestTypeId, RequestSerial requestSerial, const char *content, size_t contentLen);
    virtual void onLogFuncLife(ModuleId moduleId, const char *tag, const char *funcName, FuncLifeTag lifeTag) = 0;
    virtual void onLogFuncLifeExt(ModuleId moduleId, const char *tag, const char *funcName, FuncLifeTag lifeTag,
        std::intptr_t v1, std::intptr_t v2, std::intptr_t v3) = 0;
    virtual void onLogDiscard(ModuleId byModuleId, const char *tag, RequestTypeId requestTypeId, const RequestSerial *requestSerialList, size_t n) = 0;
    virtual void onLogPathDiv(ModuleId fromModuleId, const char *tag, RequestTypeId requestTypeId, RequestSerial requestSerial,
        const ModuleId *toModuleIdList, size_t n);
    virtual void onLogPathJoin(ModuleId toModuleId, const char *tag, RequestTypeId requestTypeId, RequestSerial requestSerial,
        const ModuleId *fromModuleIdList, size_t n);
    virtual void onLogValue(ModuleId moduleId, const char *tag, const char *name, std::int32_t value);
    virtual void onFlush(int waitDoneSec);

    virtual void onInit() { }
    virtual void onUninit() { }
    virtual ~ULogger();

protected:
    ULogger() { }

private:
    friend class ULogInitializerImpl;
    friend class ULogFuncLife;

    static void onLogDetailsAndroid(ModuleId moduleId, const char *tag, DetailsType type,
        RequestTypeId requestTypeId, RequestSerial requestSerial, const char *content);

    static void setMode(unsigned int mode) {
        sMode = mode;
    }

    static void setModFilterMask(unsigned int mask) {
        sModFilterMask = mask;
    }

    static void setDetailsLevelAndMask(DetailsType level, unsigned int mask) {
        sDetailsLevel = static_cast<decltype(sDetailsLevel)>(level);
        sNormalDetailsMask = mask;
    }

    static void logFuncLife(ModuleId moduleId, const char *tag, const char *funcName, FuncLifeTag lifeTag) {
        get()->onLogFuncLife(moduleId, tag, funcName, lifeTag);
    }

    static void logFuncLifeEnter(ModuleId moduleId, const char *tag, const char *funcName, FuncLifeTag lifeTag,
        std::intptr_t v1, std::intptr_t v2 = 0, std::intptr_t v3 = 0)
    {
        get()->onLogFuncLifeExt(moduleId, tag, funcName, lifeTag, v1, v2, v3);
    }
};


template <typename _M>
inline ModuleId getULogModuleIdIntegral(_M &&ptr, std::false_type &&)
{
    // ptr may be a smart pointer
    return ptr->getULogModuleId();
}


template <typename _M>
inline ModuleId getULogModuleIdIntegral(_M &&id, std::true_type &&)
{
    return static_cast<ModuleId>(id);
}


template <typename _M>
inline ModuleId getULogModuleId(_M &&m)
{
    return getULogModuleIdIntegral(
        std::forward<_M>(m),
        std::is_convertible<typename std::remove_reference<_M>::type, unsigned int>());
}


template <>
inline ModuleId getULogModuleId<ModuleId>(ModuleId &&m)
{
    return m;
}


template <>
inline ModuleId getULogModuleId<ModuleId&>(ModuleId &m)
{
    return m;
}


template <>
inline ModuleId getULogModuleId<const ModuleId&>(const ModuleId &m)
{
    return m;
}


template <typename _R>
inline RequestTypeId getULogRequestTypeIdIntegral(_R &&ptr, std::false_type &&)
{
    // ptr may be a smart pointer
    return ptr->getULogRequestTypeId();
}


template <typename _R>
inline RequestTypeId getULogReferenceTypeIdIntegral(_R &&id, std::true_type &&)
{
    return static_cast<NSCam::Utils::ULog::RequestTypeId>(id);
}


template <typename _R>
inline RequestTypeId getULogRequestTypeId(_R &&r)
{
    return NSCam::Utils::ULog::getULogRequestTypeIdIntegral(
        std::forward<_R>(r),
        std::is_integral<typename std::remove_reference<_R>::type>());
}


template <>
inline RequestTypeId getULogRequestTypeId<RequestTypeId>(RequestTypeId &&r)
{
    return r;
}


template <>
inline RequestTypeId getULogRequestTypeId<RequestTypeId&>(RequestTypeId &r)
{
    return r;
}


template <>
inline RequestTypeId getULogRequestTypeId<const RequestTypeId&>(const RequestTypeId &r)
{
    return r;
}


template <typename _T>
inline std::intptr_t asIntPtr(_T &&ptr, std::true_type &&)
{
    return reinterpret_cast<std::intptr_t>(ptr);
}


template <typename _T>
inline std::intptr_t asIntPtr(_T &&value, std::false_type &&)
{
    return static_cast<std::intptr_t>(value);
}


template <typename _T>
inline std::intptr_t asIntPtr(_T &&value)
{
    return asIntPtr(
        std::forward<_T>(value),
        std::is_pointer<typename std::remove_reference<_T>::type>());
}


class ULogFuncLife
{
public:
    struct Always { };

    // All following APIs are implicitly inlined by C++ standard

    static inline bool isLogEnabled(ModuleId moduleId) {
        // Function log is default off, the sModFilterMask != 0 will help fast check
        return (sModFilterMask != 0 &&
                _isPassFilter(moduleId, sModFilterMask));
    }

    inline ULogFuncLife(ModuleId moduleId, bool cond, const char *tag, FuncLifeTag lifeTag, const char *funcName) :
        mModuleId(moduleId), mExitTag(0), mTag(tag), mFuncName(funcName)
    {
        if (cond) {
            mExitTag = (EXIT_BIT | lifeTag);
            ULogger::logFuncLife(mModuleId, mTag, mFuncName, lifeTag);
        }
    }

    inline ULogFuncLife(ModuleId moduleId, Always&&, const char *tag, FuncLifeTag lifeTag, const char *funcName) :
        mModuleId(moduleId), mExitTag(0), mTag(tag), mFuncName(funcName)
    {
        mExitTag = (EXIT_BIT | lifeTag);
        ULogger::logFuncLife(mModuleId, mTag, mFuncName, lifeTag);
    }

    inline ULogFuncLife(ModuleId moduleId, const char *tag, FuncLifeTag lifeTag, const char *funcName) :
        mModuleId(moduleId), mExitTag(0), mTag(tag), mFuncName(funcName)
    {
        if (isLogEnabled(moduleId)) {
            mExitTag = (EXIT_BIT | lifeTag);
            ULogger::logFuncLife(mModuleId, mTag, mFuncName, lifeTag);
        }
    }

    template <typename ... _T>
    inline ULogFuncLife(ModuleId moduleId, const char *tag, FuncLifeTag lifeTag, const char *funcName, _T&& ... v) :
        mModuleId(moduleId), mExitTag(0), mTag(tag), mFuncName(funcName)
    {
        if (isLogEnabled(moduleId)) {
            mExitTag = (EXIT_BIT | lifeTag);
            ULogger::logFuncLifeEnter(mModuleId, mTag, mFuncName, lifeTag,
                asIntPtr(std::forward<_T>(v)) ...); // Only accept 3 arguments at most
        }
    }

    inline ~ULogFuncLife() {
        if (mExitTag != 0) // 0 also means API_ENTER, it is meaningless for exit
            ULogger::logFuncLife(mModuleId, mTag, mFuncName, static_cast<FuncLifeTag>(mExitTag));
    }

private:
    friend class ULogInitializerImpl;

    static unsigned int sModFilterMask; // not atomic, but only set in program init

    // Too many variables? No, they will be inlined
    const ModuleId mModuleId;
    unsigned int mExitTag;
    const char* const mTag;
    const char* const mFuncName;

    static void setModFilterMask(unsigned int mask) {
        sModFilterMask = mask;
    }
};


class ULogTimeBomb final : public android::RefBase
{
public:
    struct please_use_CAM_ULOG_TIMEBOMB_CREATE { };

    // Please use CAM_ULOG_TIMEBOMB_CREATE() to create a time bomb.
    // Do NOT call create() directly.
    static android::sp<ULogTimeBomb> create(
        const please_use_CAM_ULOG_TIMEBOMB_CREATE &,
        ModuleId moduleId, unsigned int serial, const char *dispatchKey, int warnMs, int abortMs, int abortMaxBound = 0);

    ModuleId getModuleId() const { return mModuleId; }
    const char *getDispatchKey() const { return mDispatchKey; }
    bool isValid() const { return mValidGuardPattern == VALID_PATTERN; }
    unsigned int getSerial() const { return mSerial; }
    int getWarnMs() const { return mWarnMs.load(std::memory_order_relaxed); }
    int getAbortMs() const { return mAbortMs.load(std::memory_order_relaxed); }
    const struct timespec& getTimeStamp() const { return mTimeStamp; }
    int getElapsedMs() const;

    // Extend the time of warn & abort, will limited by abortMaxBound
    void extendTime(int plusWarnMs, int plusAbortMs);

    ULogTimeBomb &operator=(const ULogTimeBomb &) = delete; // NO COPY

protected:
    virtual void onLastStrongRef(const void *id);

private:
    ULogTimeBomb(
        ModuleId moduleId, unsigned int serial, const char *dispatchKey, int warnMs, int abortMs, int abortMaxBound);

    ~ULogTimeBomb() {
        mValidGuardPattern = 0xdeaddead;
    }

    static constexpr unsigned int VALID_PATTERN = 0xabcd0987;
    volatile unsigned int mValidGuardPattern;

    ModuleId mModuleId;
    unsigned int mSerial;
    int mAbortMaxBound;
    std::atomic_int mWarnMs;
    std::atomic_int mAbortMs;
    struct timespec mTimeStamp;
    static constexpr size_t DISPATCH_KEY_LEN_MAX = 64;
    char mDispatchKey[DISPATCH_KEY_LEN_MAX + 1]; // Fixed string container to minimize the overhead
};


class ULogGuard
{
    friend class ULogFuncLifeGuard;
    friend class ULogInitializerImpl;

public:
    static void registerReqGuard(ModuleId moduleId, RequestTypeId requestType, RequestSerial requestSerial, int warnMs = 0);
    static void modifyReqGuard(ModuleId moduleId, RequestTypeId requestType, RequestSerial requestSerial, int warnMs);
    static void unregisterReqGuard(ModuleId moduleId, RequestTypeId requestType, RequestSerial requestSerial);

    struct Status { }; // Reserved

    class IFinalizer : virtual public android::RefBase {
    public:
        virtual void onTimeout(const Status &) = 0;
    };

    static void registerFinalizer(android::sp<IFinalizer> finalizer);

private:
    typedef std::int32_t GuardId;

    static void onInit();
    static void onUninit();
    static void registerFuncGuard(ModuleId moduleId, const char *tag, const char *funcName, const char *longFuncName,
        int warnMs, int abortMs, const char *dispatchKey, GuardId *outGuardId);
    static void registerFuncGuard(ModuleId moduleId, const char *tag, const char *funcName, const char *longFuncName,
        int warnMs, GuardId *outGuardId);
    static void unregisterFuncGuard(int guardId);
};


class ULogFuncLifeGuard : public ULogFuncLife
{
public:
    inline ULogFuncLifeGuard(ModuleId moduleId, const char *tag, FuncLifeTag lifeTag,
            const char *funcName, int warnMs = 0) :
        ULogFuncLife(moduleId, tag, lifeTag, funcName)
    {
        ULogGuard::registerFuncGuard(moduleId, tag, funcName, funcName, warnMs, &mGuardId);
    }

    inline ULogFuncLifeGuard(ModuleId moduleId, const char *tag, FuncLifeTag lifeTag,
            const char *funcName, int warnMs, int abortMs, const char *dispatchKey) :
        ULogFuncLife(moduleId, tag, lifeTag, funcName)
    {
        ULogGuard::registerFuncGuard(moduleId, tag, funcName, funcName, warnMs, abortMs, dispatchKey, &mGuardId);
    }

    inline ULogFuncLifeGuard(ModuleId moduleId, const char *tag, FuncLifeTag lifeTag,
            const char *funcName, const char *longFuncName, int warnMs = 0) :
        ULogFuncLife(moduleId, tag, lifeTag, funcName)
    {
        ULogGuard::registerFuncGuard(moduleId, tag, funcName, longFuncName, warnMs, &mGuardId);
    }

    inline ULogFuncLifeGuard(ModuleId moduleId, const char *tag, FuncLifeTag lifeTag,
            const char *funcName, const char *longFuncName, int warnMs, int abortMs, const char *dispatchKey) :
        ULogFuncLife(moduleId, tag, lifeTag, funcName)
    {
        ULogGuard::registerFuncGuard(moduleId, tag, funcName, longFuncName, warnMs, abortMs, dispatchKey, &mGuardId);
    }

    inline ~ULogFuncLifeGuard() {
        ULogGuard::unregisterFuncGuard(mGuardId);
    }

private:
    ULogGuard::GuardId mGuardId;
};


class ULogInitializerImpl;

class ULogInitializer
{
public:
    // Can be only used by main()
    ULogInitializer();
    ~ULogInitializer();

private:
    ULogInitializerImpl *mpImpl;
};


}
}
}

#endif

