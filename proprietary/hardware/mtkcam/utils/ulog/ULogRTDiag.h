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


#include <mutex>
#include <mtkcam/utils/debug/debug.h>
#include <mtkcam/utils/std/ULog.h>
#include "ULogInt.h"


namespace NSCam {
namespace Utils {
namespace ULog {


class ULogRTDiagImpl
{
public:
    static ULogRTDiagImpl& get() {
        return *reinterpret_cast<ULogRTDiagImpl*>(&sSingleton);
    }

    void onLogEnter(ModuleId intoModuleId, RequestTypeId requestTypeId, RequestSerial requestSerial);
    void onLogExit(ModuleId outFromModuleId, RequestTypeId requestTypeId, RequestSerial requestSerial);
    void onLogSubreqs(ModuleId byModuleId, RequestTypeId requestTypeId, RequestSerial requestSerial,
        RequestTypeId subrequestTypeId, const RequestSerial *subrequestSerialList, size_t n);

    void debugDump(android::Printer& printer);
    void dumpToLog();
    ModuleId timeoutPickSuspect(RequestSerial timeoutAppReqSerial, int timeoutMs);
    void dumpToFile(int fd, const char *prefix = "");
    static void initSingleton();
    static void uninitSingleton();

private:
    template <int MaxEnterStates>
    struct RequestState {
        static constexpr int MAX_ENTER_STATES = MaxEnterStates;

        RequestSerial appReqSerial;
        RequestSerial serial;
        int enterCount;
        // Use separate arrays to reduce the memory padding waste
        ModuleId enterModuleId[MAX_ENTER_STATES];
        timespec enterTimeStamp[MAX_ENTER_STATES];

        RequestState() : appReqSerial(0), serial(0), enterCount(0),
            enterModuleId{0}, enterTimeStamp{{0,0}}
        { }
    };

    std::mutex mMutex;

    static constexpr int MAX_APP_REQS = 32; // Power of 2, so that modulo can be effiency
    RequestState<2> mAppRequests[MAX_APP_REQS];
    static constexpr int MAX_PIPELINE_FRAMES = 32;
    RequestState<4> mPipelineFrames[MAX_PIPELINE_FRAMES];
    static constexpr int MAX_P2_REQS = 64;
    static constexpr int MAX_FPIPE_REQS = 64;
    RequestState<2> mP2StrRequests[MAX_P2_REQS];
    RequestState<4> mFPipeStrRequests[MAX_FPIPE_REQS];
    RequestState<2> mP2CapRequests[MAX_P2_REQS];
    RequestState<4> mFPipeCapRequests[MAX_FPIPE_REQS];

    // Ugly, but can avoid the destructor to be called when somebody calls exit()
    static char sSingleton[];

    template <int MaxEnterStates>
    bool isActive(RequestState<MaxEnterStates> &state);

    template <int MaxEnterStates>
    void onLogEnter(ModuleId intoModuleId, RequestState<MaxEnterStates> &state, RequestSerial requestSerial,
        const timespec &timeStamp);

    template <int MaxEnterStates>
    void onLogExit(ModuleId outFromModuleId, RequestState<MaxEnterStates> &state, RequestSerial requestSerial);

    template <int MaxEnterStates1, int MaxEnterStates2>
    void onLogSubreqs(ModuleId byModuleId, RequestState<MaxEnterStates1> &state,
        RequestState<MaxEnterStates2> &subReqState, RequestSerial subReqSerial);

    template <int MaxEnterStates1>
    void onLogSubreqs(ModuleId byModuleId, RequestState<MaxEnterStates1> &state, RequestSerial requestSerial,
        RequestTypeId subrequestTypeId, const RequestSerial *subrequestSerialList, size_t n);

    template <typename _Print, int MaxEnterStates>
    void dumpActives(_Print &print, RequestTypeId requestTypeId,
        RequestState<MaxEnterStates> *stateArray, int nElements);

    template <int MaxEnterStates, int N>
    auto &requestSlot(RequestState<MaxEnterStates> (&reqArray)[N], int serial) {
        return reqArray[serial % N];
    }
};


}
}
}

