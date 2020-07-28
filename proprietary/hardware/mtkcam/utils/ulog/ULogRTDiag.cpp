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

#define LOG_TAG "ULogDiag"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include "ULogRTDiag.h"


CAM_ULOG_DECLARE_MODULE_ID(MOD_UNIFIED_LOG);

namespace NSCam {
namespace Utils {
namespace ULog {

const clockid_t ULOG_RT_DIAG_CLOCK_ID = CLOCK_MONOTONIC;

alignas(sizeof(void*))
char ULogRTDiagImpl::sSingleton[sizeof(ULogRTDiagImpl)];


inline void ULogRTDiagImpl::initSingleton()
{
    new (ULogRTDiagImpl::sSingleton) ULogRTDiagImpl;
}


inline void ULogRTDiagImpl::uninitSingleton()
{
    // We don't destruct the singleton, but Coverity may warn
    // get().~ULogRTDiagImpl();
}


template <int MaxEnterStates>
inline bool ULogRTDiagImpl::isActive(RequestState<MaxEnterStates> &state)
{
    if (state.enterCount == 0)
        return false;

    auto &appReqState = requestSlot(mAppRequests, state.appReqSerial);
    if (appReqState.serial != state.appReqSerial ||
        appReqState.enterCount == 0)
    {
        return false;
    }

    return true;
}


template <int MaxEnterStates>
inline void ULogRTDiagImpl::onLogEnter(ModuleId intoModuleId, RequestState<MaxEnterStates> &state, RequestSerial requestSerial,
    const timespec &timeStamp)
{
    if (__unlikely(state.serial != requestSerial))
        return;

    for (int i = 0; i < state.MAX_ENTER_STATES; i++) // unrolled
        if (state.enterModuleId[i] == intoModuleId)
            return;

    for (int i = 0; i < state.MAX_ENTER_STATES; i++)
        if (state.enterModuleId[i] == 0) {
            state.enterModuleId[i] = intoModuleId;
            state.enterTimeStamp[i] = timeStamp;
            state.enterCount++;
            break;
        }
}


template <int MaxEnterStates>
inline void ULogRTDiagImpl::onLogExit(ModuleId outFromModuleId, RequestState<MaxEnterStates> &state, RequestSerial requestSerial)
{
    if (__unlikely(state.serial != requestSerial))
        return;

    for (int i = 0; i < state.MAX_ENTER_STATES; i++)
        if (state.enterModuleId[i] == outFromModuleId) {
            state.enterModuleId[i] = 0;
            state.enterCount--;
            break;
        }
}


template <int MaxEnterStates1, int MaxEnterStates2>
inline void ULogRTDiagImpl::onLogSubreqs(ModuleId /* byModuleId */, RequestState<MaxEnterStates1> &state,
    RequestState<MaxEnterStates2> &subReqState, RequestSerial subReqSerial)
{
    subReqState.appReqSerial = state.appReqSerial;
    subReqState.serial = subReqSerial;
    subReqState.enterCount = 0;

    for (int i = 0; i < subReqState.MAX_ENTER_STATES; i++)
        subReqState.enterModuleId[i] = 0;
}


inline void ULogRTDiagImpl::onLogEnter(ModuleId intoModuleId, RequestTypeId requestTypeId, RequestSerial requestSerial)
{
    // Get time outside the lock
    timespec timeStamp;
    clock_gettime(ULOG_RT_DIAG_CLOCK_ID, &timeStamp);

    std::lock_guard<std::mutex> lock(mMutex);

    switch (requestTypeId) {
    case REQ_APP_REQUEST:
        {
            auto &appRequest = requestSlot(mAppRequests, requestSerial);
            if (appRequest.appReqSerial != requestSerial) {
                if (__unlikely(appRequest.enterCount != 0)) {
                    CAM_ULOGMW("Tracking of %s:%u conflict-missed. Not exit yet: %d",
                        getRequestTypeName(REQ_APP_REQUEST), appRequest.appReqSerial, appRequest.enterCount);
                }

                appRequest.appReqSerial = requestSerial;
                appRequest.serial = requestSerial;
                appRequest.enterCount = 0;
                for (int i = 0; i < appRequest.MAX_ENTER_STATES; i++)
                    appRequest.enterModuleId[i] = 0;
            }
            onLogEnter(intoModuleId, appRequest, requestSerial, timeStamp);
        }
        break;
    case REQ_PIPELINE_FRAME:
        onLogEnter(intoModuleId, requestSlot(mPipelineFrames, requestSerial), requestSerial, timeStamp);
        break;
    case REQ_P2_CAP_REQUEST:
        onLogEnter(intoModuleId, requestSlot(mP2CapRequests, requestSerial), requestSerial, timeStamp);
        break;
    case REQ_P2_STR_REQUEST:
        onLogEnter(intoModuleId, requestSlot(mP2StrRequests, requestSerial), requestSerial, timeStamp);
        break;
    case REQ_CAP_FPIPE_REQUEST:
        onLogEnter(intoModuleId, requestSlot(mFPipeCapRequests, requestSerial), requestSerial, timeStamp);
        break;
    case REQ_STR_FPIPE_REQUEST:
        onLogEnter(intoModuleId, requestSlot(mFPipeStrRequests, requestSerial), requestSerial, timeStamp);
        break;
    default:
        break;
    }
}


inline void ULogRTDiagImpl::onLogExit(ModuleId outFromModuleId, RequestTypeId requestTypeId, RequestSerial requestSerial)
{
    std::lock_guard<std::mutex> lock(mMutex);

    switch (requestTypeId) {
    case REQ_APP_REQUEST:
        onLogExit(outFromModuleId, requestSlot(mAppRequests, requestSerial), requestSerial);
        break;
    case REQ_PIPELINE_FRAME:
        onLogExit(outFromModuleId, requestSlot(mPipelineFrames, requestSerial), requestSerial);
        break;
    case REQ_P2_CAP_REQUEST:
        onLogExit(outFromModuleId, requestSlot(mP2CapRequests, requestSerial), requestSerial);
        break;
    case REQ_P2_STR_REQUEST:
        onLogExit(outFromModuleId, requestSlot(mP2StrRequests, requestSerial), requestSerial);
        break;
    case REQ_CAP_FPIPE_REQUEST:
        onLogExit(outFromModuleId, requestSlot(mFPipeCapRequests, requestSerial), requestSerial);
        break;
    case REQ_STR_FPIPE_REQUEST:
        onLogExit(outFromModuleId, requestSlot(mFPipeStrRequests, requestSerial), requestSerial);
        break;
    default:
        break;
    }
}


template <int MaxEnterStates1>
inline void ULogRTDiagImpl::onLogSubreqs(ModuleId byModuleId, RequestState<MaxEnterStates1> &state, RequestSerial requestSerial,
    RequestTypeId subrequestTypeId, const RequestSerial *subrequestSerialList, size_t n)
{
    std::lock_guard<std::mutex> lock(mMutex);

    if (__unlikely(!isActive(state)))
        return;

    if (__unlikely(state.serial != requestSerial))
        return;

    switch (subrequestTypeId) {
    case REQ_PIPELINE_FRAME:
        for (size_t i = 0; i < n; i++) {
            RequestSerial subReqSerial = subrequestSerialList[i];
            onLogSubreqs(byModuleId, state, requestSlot(mPipelineFrames, subReqSerial), subReqSerial);
        }
        break;
    case REQ_P2_CAP_REQUEST:
        for (size_t i = 0; i < n; i++) {
            RequestSerial subReqSerial = subrequestSerialList[i];
            onLogSubreqs(byModuleId, state, requestSlot(mP2CapRequests, subReqSerial), subReqSerial);
        }
        break;
    case REQ_P2_STR_REQUEST:
        for (size_t i = 0; i < n; i++) {
            RequestSerial subReqSerial = subrequestSerialList[i];
            onLogSubreqs(byModuleId, state, requestSlot(mP2StrRequests, subReqSerial), subReqSerial);
        }
        break;
    case REQ_CAP_FPIPE_REQUEST:
        for (size_t i = 0; i < n; i++) {
            RequestSerial subReqSerial = subrequestSerialList[i];
            onLogSubreqs(byModuleId, state, requestSlot(mFPipeCapRequests, subReqSerial), subReqSerial);
        }
        break;
    case REQ_STR_FPIPE_REQUEST:
        for (size_t i = 0; i < n; i++) {
            RequestSerial subReqSerial = subrequestSerialList[i];
            onLogSubreqs(byModuleId, state, requestSlot(mFPipeStrRequests, subReqSerial), subReqSerial);
        }
        break;
    default:
        break;
    }
}


inline void ULogRTDiagImpl::onLogSubreqs(ModuleId byModuleId, RequestTypeId requestTypeId, RequestSerial requestSerial,
    RequestTypeId subrequestTypeId, const RequestSerial *subrequestSerialList, size_t n)
{
    switch (requestTypeId) {
    case REQ_APP_REQUEST:
        onLogSubreqs(byModuleId, requestSlot(mAppRequests, requestSerial), requestSerial,
            subrequestTypeId, subrequestSerialList, n);
        break;
    case REQ_PIPELINE_FRAME:
        onLogSubreqs(byModuleId, requestSlot(mPipelineFrames, requestSerial), requestSerial,
            subrequestTypeId, subrequestSerialList, n);
        break;
    case REQ_P2_CAP_REQUEST:
        onLogSubreqs(byModuleId, requestSlot(mP2CapRequests, requestSerial), requestSerial,
            subrequestTypeId, subrequestSerialList, n);
        break;
    case REQ_P2_STR_REQUEST:
        onLogSubreqs(byModuleId, requestSlot(mP2StrRequests, requestSerial), requestSerial,
            subrequestTypeId, subrequestSerialList, n);
        break;
    case REQ_CAP_FPIPE_REQUEST:
        onLogSubreqs(byModuleId, requestSlot(mFPipeCapRequests, requestSerial), requestSerial,
            subrequestTypeId, subrequestSerialList, n);
        break;
    case REQ_STR_FPIPE_REQUEST:
        onLogSubreqs(byModuleId, requestSlot(mFPipeStrRequests, requestSerial), requestSerial,
            subrequestTypeId, subrequestSerialList, n);
        break;
    default:
        break;
    }
}


template <typename _Print, int MaxEnterStates>
void ULogRTDiagImpl::dumpActives(_Print &print, RequestTypeId requestTypeId,
    RequestState<MaxEnterStates> *stateArray, int nElements)
{
    ModuleId enterModuleId[MaxEnterStates];
    timespec enterTimeStamp[MaxEnterStates];
    const char *requestTypeName = nullptr;
    const char *appReqTypeName = getRequestTypeName(REQ_APP_REQUEST);

    std::lock_guard<std::mutex> lock(mMutex);

    for (int recordIndex = 0; recordIndex < nElements; recordIndex++) {
        RequestState<MaxEnterStates> &state = stateArray[recordIndex];
        if (isActive(state)) {
            int enterStateTop = 0;
            for (int stateIndex = 0; stateIndex < state.MAX_ENTER_STATES; stateIndex++) {
                if (state.enterModuleId[stateIndex] != 0) {
                    enterModuleId[enterStateTop] = state.enterModuleId[stateIndex];
                    enterTimeStamp[enterStateTop] = state.enterTimeStamp[stateIndex];
                    enterStateTop++;
                }
            }

            if (requestTypeName == nullptr)
                requestTypeName = getRequestTypeName(requestTypeId);

            print(requestTypeName, state.serial, appReqTypeName, state.appReqSerial,
                enterModuleId, enterTimeStamp, enterStateTop);
        }
    }
}


void ULogRTDiagImpl::debugDump(android::Printer& printer)
{
    timespec nowReal;
    clock_gettime(CLOCK_REALTIME, &nowReal);
    char realTimeBuffer[40];
    struct tm logTm;
    strftime(realTimeBuffer, sizeof(realTimeBuffer), ULOG_TIME_FORMAT, ulocaltime_r(&nowReal.tv_sec, &logTm));
    printer.printFormatLine("Dump time: %s.%06ld", realTimeBuffer, (nowReal.tv_nsec / 1000));

    timespec nowMono;
    clock_gettime(ULOG_RT_DIAG_CLOCK_ID, &nowMono);

    auto printToPrinter = [&printer, &nowMono] (
        const char *requestTypeName, RequestSerial requestSerial, const char *appReqTypeName, RequestSerial appReqSerial,
        const ModuleId *enterModuleId, const timespec *enterTimeStamp, int enterStateTop)
    {
        switch (enterStateTop) {
        case 0:
            break;
        case 1:
            printer.printFormatLine("R %s:%u(%s:%u) in M[%s:%x] %d ms",
                requestTypeName, requestSerial, appReqTypeName, appReqSerial,
                getModuleName(enterModuleId[0]), enterModuleId[0], timeDiffMs(enterTimeStamp[0], nowMono));
            break;
        case 2:
            printer.printFormatLine("R %s:%u(%s:%u) in M[%s:%x] %d ms, M[%s:%x] %d ms",
                requestTypeName, requestSerial, appReqTypeName, appReqSerial,
                getModuleName(enterModuleId[0]), enterModuleId[0], timeDiffMs(enterTimeStamp[0], nowMono),
                getModuleName(enterModuleId[1]), enterModuleId[1], timeDiffMs(enterTimeStamp[1], nowMono));
            break;
        case 3:
            printer.printFormatLine("R %s:%u(%s:%u) in M[%s:%x] %d ms, M[%s:%x] %d ms, M[%s:%x] %d ms",
                requestTypeName, requestSerial, appReqTypeName, appReqSerial,
                getModuleName(enterModuleId[0]), enterModuleId[0], timeDiffMs(enterTimeStamp[0], nowMono),
                getModuleName(enterModuleId[1]), enterModuleId[1], timeDiffMs(enterTimeStamp[1], nowMono),
                getModuleName(enterModuleId[2]), enterModuleId[2], timeDiffMs(enterTimeStamp[2], nowMono));
            break;
        case 4:
        default:
            printer.printFormatLine("R %s:%u(%s:%u) in M[%s:%x] %d ms, M[%s:%x] %d ms, M[%s:%x] %d ms, M[%s:%x] %d ms",
                requestTypeName, requestSerial, appReqTypeName, appReqSerial,
                getModuleName(enterModuleId[0]), enterModuleId[0], timeDiffMs(enterTimeStamp[0], nowMono),
                getModuleName(enterModuleId[1]), enterModuleId[1], timeDiffMs(enterTimeStamp[1], nowMono),
                getModuleName(enterModuleId[2]), enterModuleId[2], timeDiffMs(enterTimeStamp[2], nowMono),
                getModuleName(enterModuleId[3]), enterModuleId[3], timeDiffMs(enterTimeStamp[3], nowMono));
            break;
        }
    };

    dumpActives(printToPrinter, REQ_APP_REQUEST, mAppRequests, MAX_APP_REQS);
    dumpActives(printToPrinter, REQ_PIPELINE_FRAME, mPipelineFrames, MAX_PIPELINE_FRAMES);
    dumpActives(printToPrinter, REQ_P2_CAP_REQUEST, mP2CapRequests, MAX_P2_REQS);
    dumpActives(printToPrinter, REQ_P2_STR_REQUEST, mP2StrRequests, MAX_P2_REQS);
    dumpActives(printToPrinter, REQ_CAP_FPIPE_REQUEST, mFPipeCapRequests, MAX_FPIPE_REQS);
    dumpActives(printToPrinter, REQ_STR_FPIPE_REQUEST, mFPipeStrRequests, MAX_FPIPE_REQS);
}


void ULogRTDiagImpl::dumpToLog()
{
    timespec nowMono;
    clock_gettime(ULOG_RT_DIAG_CLOCK_ID, &nowMono);

    auto printToLog = [&nowMono] (
        const char *requestTypeName, RequestSerial requestSerial, const char *appReqTypeName, RequestSerial appReqSerial,
        const ModuleId *enterModuleId, const timespec *enterTimeStamp, int enterStateTop)
    {
        switch (enterStateTop) {
        case 0:
            break;
        case 1:
            CAM_ULOGMI_ALWAYS("R %s:%u(%s:%u) in M[%s:%x] %d ms",
                requestTypeName, requestSerial, appReqTypeName, appReqSerial,
                getModuleName(enterModuleId[0]), enterModuleId[0], timeDiffMs(enterTimeStamp[0], nowMono));
            break;
        case 2:
            CAM_ULOGMI_ALWAYS("R %s:%u(%s:%u) in M[%s:%x] %d ms, M[%s:%x] %d ms",
                requestTypeName, requestSerial, appReqTypeName, appReqSerial,
                getModuleName(enterModuleId[0]), enterModuleId[0], timeDiffMs(enterTimeStamp[0], nowMono),
                getModuleName(enterModuleId[1]), enterModuleId[1], timeDiffMs(enterTimeStamp[1], nowMono));
            break;
        case 3:
            CAM_ULOGMI_ALWAYS("R %s:%u(%s:%u) in M[%s:%x] %d ms, M[%s:%x] %d ms, M[%s:%x] %d ms",
                requestTypeName, requestSerial, appReqTypeName, appReqSerial,
                getModuleName(enterModuleId[0]), enterModuleId[0], timeDiffMs(enterTimeStamp[0], nowMono),
                getModuleName(enterModuleId[1]), enterModuleId[1], timeDiffMs(enterTimeStamp[1], nowMono),
                getModuleName(enterModuleId[2]), enterModuleId[2], timeDiffMs(enterTimeStamp[2], nowMono));
            break;
        case 4:
        default:
            CAM_ULOGMI_ALWAYS("R %s:%u(%s:%u) in M[%s:%x] %d ms, M[%s:%x] %d ms, M[%s:%x] %d ms, M[%s:%x] %d ms",
                requestTypeName, requestSerial, appReqTypeName, appReqSerial,
                getModuleName(enterModuleId[0]), enterModuleId[0], timeDiffMs(enterTimeStamp[0], nowMono),
                getModuleName(enterModuleId[1]), enterModuleId[1], timeDiffMs(enterTimeStamp[1], nowMono),
                getModuleName(enterModuleId[2]), enterModuleId[2], timeDiffMs(enterTimeStamp[2], nowMono),
                getModuleName(enterModuleId[3]), enterModuleId[3], timeDiffMs(enterTimeStamp[3], nowMono));
            break;
        }
    };

    dumpActives(printToLog, REQ_APP_REQUEST, mAppRequests, MAX_APP_REQS);
    dumpActives(printToLog, REQ_PIPELINE_FRAME, mPipelineFrames, MAX_PIPELINE_FRAMES);
    dumpActives(printToLog, REQ_P2_CAP_REQUEST, mP2CapRequests, MAX_P2_REQS);
    dumpActives(printToLog, REQ_P2_STR_REQUEST, mP2StrRequests, MAX_P2_REQS);
    dumpActives(printToLog, REQ_CAP_FPIPE_REQUEST, mFPipeCapRequests, MAX_FPIPE_REQS);
    dumpActives(printToLog, REQ_STR_FPIPE_REQUEST, mFPipeStrRequests, MAX_FPIPE_REQS);
}


ModuleId ULogRTDiagImpl::timeoutPickSuspect(RequestSerial timeoutAppReqSerial, int timeoutMs)
{
    ModuleId suspect = 0;
    int maxElapsedOfLayer = 0;

    timespec nowMono;
    clock_gettime(ULOG_RT_DIAG_CLOCK_ID, &nowMono);

    auto printToLog = [&nowMono, &suspect, timeoutAppReqSerial, timeoutMs, &maxElapsedOfLayer] (
        const char *requestTypeName, RequestSerial requestSerial, const char *appReqTypeName, RequestSerial appReqSerial,
        const ModuleId *enterModuleId, const timespec *enterTimeStamp, int enterStateTop)
    {
        if (appReqSerial != timeoutAppReqSerial)
            return;

        bool pickedSuspectFromHere = false;
        for (int i = 0; i < enterStateTop; i++) {
            int elapsed = timeDiffMs(enterTimeStamp[i], nowMono);
            if (elapsed * 2 > timeoutMs) { // elapsed > timeoutMs / 2
                unsigned int enterLayer = (enterModuleId[i] & ULOG_LAYER_MASK);
                if (enterLayer > (suspect & ULOG_LAYER_MASK)) {
                    suspect = enterModuleId[i];
                    maxElapsedOfLayer = elapsed;
                    pickedSuspectFromHere = true;
                } else if (enterLayer == (suspect & ULOG_LAYER_MASK) &&
                           elapsed > maxElapsedOfLayer)
                {
                    suspect = enterModuleId[i];
                    maxElapsedOfLayer = elapsed;
                    pickedSuspectFromHere = true;
                }
            }
        }

        switch (enterStateTop) {
        case 0:
            break;
        case 1:
            if (pickedSuspectFromHere) {
                CAM_ULOGMW("R %s:%u(%s:%u) in M[%s:%x] %d ms",
                    requestTypeName, requestSerial, appReqTypeName, appReqSerial,
                    getModuleName(enterModuleId[0]), enterModuleId[0], timeDiffMs(enterTimeStamp[0], nowMono));
            } else {
                CAM_ULOGMI_ALWAYS("R %s:%u(%s:%u) in M[%s:%x] %d ms",
                    requestTypeName, requestSerial, appReqTypeName, appReqSerial,
                    getModuleName(enterModuleId[0]), enterModuleId[0], timeDiffMs(enterTimeStamp[0], nowMono));
            }
            break;
        case 2:
            if (pickedSuspectFromHere) {
                CAM_ULOGMW("R %s:%u(%s:%u) in M[%s:%x] %d ms, M[%s:%x] %d ms",
                    requestTypeName, requestSerial, appReqTypeName, appReqSerial,
                    getModuleName(enterModuleId[0]), enterModuleId[0], timeDiffMs(enterTimeStamp[0], nowMono),
                    getModuleName(enterModuleId[1]), enterModuleId[1], timeDiffMs(enterTimeStamp[1], nowMono));
            } else {
                CAM_ULOGMI_ALWAYS("R %s:%u(%s:%u) in M[%s:%x] %d ms, M[%s:%x] %d ms",
                    requestTypeName, requestSerial, appReqTypeName, appReqSerial,
                    getModuleName(enterModuleId[0]), enterModuleId[0], timeDiffMs(enterTimeStamp[0], nowMono),
                    getModuleName(enterModuleId[1]), enterModuleId[1], timeDiffMs(enterTimeStamp[1], nowMono));
            }
            break;
        case 3:
            CAM_ULOGMI_ALWAYS("R %s:%u(%s:%u) in M[%s:%x] %d ms, M[%s:%x] %d ms, M[%s:%x] %d ms",
                requestTypeName, requestSerial, appReqTypeName, appReqSerial,
                getModuleName(enterModuleId[0]), enterModuleId[0], timeDiffMs(enterTimeStamp[0], nowMono),
                getModuleName(enterModuleId[1]), enterModuleId[1], timeDiffMs(enterTimeStamp[1], nowMono),
                getModuleName(enterModuleId[2]), enterModuleId[2], timeDiffMs(enterTimeStamp[2], nowMono));
            break;
        case 4:
        default:
            CAM_ULOGMI_ALWAYS("R %s:%u(%s:%u) in M[%s:%x] %d ms, M[%s:%x] %d ms, M[%s:%x] %d ms, M[%s:%x] %d ms",
                requestTypeName, requestSerial, appReqTypeName, appReqSerial,
                getModuleName(enterModuleId[0]), enterModuleId[0], timeDiffMs(enterTimeStamp[0], nowMono),
                getModuleName(enterModuleId[1]), enterModuleId[1], timeDiffMs(enterTimeStamp[1], nowMono),
                getModuleName(enterModuleId[2]), enterModuleId[2], timeDiffMs(enterTimeStamp[2], nowMono),
                getModuleName(enterModuleId[3]), enterModuleId[3], timeDiffMs(enterTimeStamp[3], nowMono));
            break;
        }
    };

    dumpActives(printToLog, REQ_APP_REQUEST, mAppRequests, MAX_APP_REQS);
    dumpActives(printToLog, REQ_PIPELINE_FRAME, mPipelineFrames, MAX_PIPELINE_FRAMES);
    dumpActives(printToLog, REQ_P2_CAP_REQUEST, mP2CapRequests, MAX_P2_REQS);
    dumpActives(printToLog, REQ_P2_STR_REQUEST, mP2StrRequests, MAX_P2_REQS);
    dumpActives(printToLog, REQ_CAP_FPIPE_REQUEST, mFPipeCapRequests, MAX_FPIPE_REQS);
    dumpActives(printToLog, REQ_STR_FPIPE_REQUEST, mFPipeStrRequests, MAX_FPIPE_REQS);

    return suspect;
}


void ULogRTDiagImpl::dumpToFile(int fd, const char *prefix)
{
    timespec nowMono;
    clock_gettime(ULOG_RT_DIAG_CLOCK_ID, &nowMono);

    auto printToFile = [fd, prefix, &nowMono] (
        const char *requestTypeName, RequestSerial requestSerial, const char *appReqTypeName, RequestSerial appReqSerial,
        const ModuleId *enterModuleId, const timespec *enterTimeStamp, int enterStateTop)
    {
        char buffer[512];
        size_t n = 0;

        switch (enterStateTop) {
        case 0:
            break;
        case 1:
            n = snprintf(buffer, sizeof(buffer), "%sR %s:%u(%s:%u) in M[%s:%x] %d ms\n",
                prefix, requestTypeName, requestSerial, appReqTypeName, appReqSerial,
                getModuleName(enterModuleId[0]), enterModuleId[0], timeDiffMs(enterTimeStamp[0], nowMono));
            break;
        case 2:
            n = snprintf(buffer, sizeof(buffer), "%sR %s:%u(%s:%u) in M[%s:%x] %d ms, M[%s:%x] %d ms\n",
                prefix, requestTypeName, requestSerial, appReqTypeName, appReqSerial,
                getModuleName(enterModuleId[0]), enterModuleId[0], timeDiffMs(enterTimeStamp[0], nowMono),
                getModuleName(enterModuleId[1]), enterModuleId[1], timeDiffMs(enterTimeStamp[1], nowMono));
            break;
        case 3:
            n = snprintf(buffer, sizeof(buffer), "%sR %s:%u(%s:%u) in M[%s:%x] %d ms, M[%s:%x] %d ms, M[%s:%x] %d ms\n",
                prefix, requestTypeName, requestSerial, appReqTypeName, appReqSerial,
                getModuleName(enterModuleId[0]), enterModuleId[0], timeDiffMs(enterTimeStamp[0], nowMono),
                getModuleName(enterModuleId[1]), enterModuleId[1], timeDiffMs(enterTimeStamp[1], nowMono),
                getModuleName(enterModuleId[2]), enterModuleId[2], timeDiffMs(enterTimeStamp[2], nowMono));
            break;
        case 4:
        default:
            n = snprintf(buffer, sizeof(buffer), "%sR %s:%u(%s:%u) in M[%s:%x] %d ms, M[%s:%x] %d ms, M[%s:%x] %d ms, M[%s:%x] %d ms\n",
                prefix, requestTypeName, requestSerial, appReqTypeName, appReqSerial,
                getModuleName(enterModuleId[0]), enterModuleId[0], timeDiffMs(enterTimeStamp[0], nowMono),
                getModuleName(enterModuleId[1]), enterModuleId[1], timeDiffMs(enterTimeStamp[1], nowMono),
                getModuleName(enterModuleId[2]), enterModuleId[2], timeDiffMs(enterTimeStamp[2], nowMono),
                getModuleName(enterModuleId[3]), enterModuleId[3], timeDiffMs(enterTimeStamp[3], nowMono));
            break;
        }

        if (n > 0)
            write(fd, buffer, n);
    };

    dumpActives(printToFile, REQ_APP_REQUEST, mAppRequests, MAX_APP_REQS);
    dumpActives(printToFile, REQ_PIPELINE_FRAME, mPipelineFrames, MAX_PIPELINE_FRAMES);
    dumpActives(printToFile, REQ_P2_CAP_REQUEST, mP2CapRequests, MAX_P2_REQS);
    dumpActives(printToFile, REQ_P2_STR_REQUEST, mP2StrRequests, MAX_P2_REQS);
    dumpActives(printToFile, REQ_CAP_FPIPE_REQUEST, mFPipeCapRequests, MAX_FPIPE_REQS);
    dumpActives(printToFile, REQ_STR_FPIPE_REQUEST, mFPipeStrRequests, MAX_FPIPE_REQS);
}


void ULogRuntimeDiag::onInit()
{
    ULogRTDiagImpl::initSingleton();
    if (sDiag == nullptr)
        sDiag = new ULogRuntimeDiag;
}


void ULogRuntimeDiag::onUninit()
{
    if (sDiag != nullptr) {
        delete sDiag;
        sDiag = nullptr;
    }
    ULogRTDiagImpl::uninitSingleton();
}


void ULogRuntimeDiag::onLogEnter(ModuleId intoModuleId, RequestTypeId requestTypeId, RequestSerial requestSerial)
{
    ULogRTDiagImpl::get().onLogEnter(intoModuleId, requestTypeId, requestSerial);
}


void ULogRuntimeDiag::onLogExit(ModuleId outFromModuleId, RequestTypeId requestTypeId, RequestSerial requestSerial)
{
    ULogRTDiagImpl::get().onLogExit(outFromModuleId, requestTypeId, requestSerial);
}


void ULogRuntimeDiag::onLogSubreqs(ModuleId byModuleId, RequestTypeId requestTypeId, RequestSerial requestSerial,
    RequestTypeId subrequestTypeId, const RequestSerial *subrequestSerialList, size_t n)
{
    ULogRTDiagImpl::get().onLogSubreqs(byModuleId, requestTypeId, requestSerial, subrequestTypeId, subrequestSerialList, n);
}


void ULogRuntimeDiag::onLogDiscard(ModuleId byModuleId, RequestTypeId requestTypeId, const RequestSerial *requestSerialList, size_t n)
{
    for (size_t i = 0; i < n; i++)
        ULogRTDiagImpl::get().onLogExit(byModuleId, requestTypeId, requestSerialList[i]);
}


void ULogRuntimeDiag::dumpToLog()
{
    if (isEnabled())
        ULogRTDiagImpl::get().dumpToLog();
}


}
}
}

