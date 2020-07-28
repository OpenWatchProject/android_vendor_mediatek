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

#ifndef __MTKCAM_UTILS_STD_ULOG_MULTI_H__
#define __MTKCAM_UTILS_STD_ULOG_MULTI_H__

#include <mtkcam/utils/std/ULog.h>


namespace NSCam {
namespace Utils {
namespace ULog {

// We use template instead of dynamic dispatching for overhead minimization
// All function call will be inlined

template <typename LogT1, typename LogT2>
class DualULogger final : public ULogger
{
private:
    LogT1 log1;
    LogT2 log2;
    const unsigned int mFunc2Filter;

    // New function filter for log2 to avoid verbose tracing log
    inline bool isFunc2Enabled(ModuleId moduleId) {
        // Function log is default off, the sModFilterMask != 0 will help fast check
        return (mFunc2Filter != 0 &&
                _isPassFilter(moduleId, mFunc2Filter));
    }

public:
    DualULogger(unsigned int func2Filter) : log1(), log2(), mFunc2Filter(func2Filter) { }

    virtual void onLogEnter(ModuleId intoModuleId, const char *tag, RequestTypeId requestTypeId, RequestSerial requestSerial) override {
        log1.onLogEnter(intoModuleId, tag, requestTypeId, requestSerial);
        log2.onLogEnter(intoModuleId, tag, requestTypeId, requestSerial);
    }

    virtual void onLogExit(ModuleId outFromModuleId, const char *tag, RequestTypeId requestTypeId, RequestSerial requestSerial) override {
        log2.onLogExit(outFromModuleId, tag, requestTypeId, requestSerial);
        log1.onLogExit(outFromModuleId, tag, requestTypeId, requestSerial);
    }

    virtual void onLogSubreqs(ModuleId byModuleId, const char *tag, RequestTypeId requestTypeId, RequestSerial requestSerial,
        RequestTypeId subrequestTypeId, const RequestSerial *subrequestSerialList, size_t n) override
    {
        log1.onLogSubreqs(byModuleId, tag, requestTypeId, requestSerial, subrequestTypeId, subrequestSerialList, n);
        log2.onLogSubreqs(byModuleId, tag, requestTypeId, requestSerial, subrequestTypeId, subrequestSerialList, n);
    }

    virtual void onLogDetails(ModuleId moduleId, const char *tag, DetailsType type,
        const char *content, size_t contentLen) override
    {
        log1.onLogDetails(moduleId, tag, type, content, contentLen);
        log2.onLogDetails(moduleId, tag, type, content, contentLen);
    }

    virtual void onLogFuncLife(ModuleId moduleId, const char *tag, const char *funcName, FuncLifeTag lifeTag) override
    {
        if (!(lifeTag & EXIT_BIT)) {
            log1.onLogFuncLife(moduleId, tag, funcName, lifeTag);
            if (isFunc2Enabled(moduleId))
                log2.onLogFuncLife(moduleId, tag, funcName, lifeTag);
        } else {
            if (isFunc2Enabled(moduleId))
                log2.onLogFuncLife(moduleId, tag, funcName, lifeTag);
            log1.onLogFuncLife(moduleId, tag, funcName, lifeTag);
        }
    }

    virtual void onLogFuncLifeExt(ModuleId moduleId, const char *tag, const char *funcName, FuncLifeTag lifeTag,
        std::intptr_t v1, std::intptr_t v2, std::intptr_t v3) override
    {
        if (!(lifeTag & EXIT_BIT)) {
            log1.onLogFuncLifeExt(moduleId, tag, funcName, lifeTag, v1, v2, v3);
            if (isFunc2Enabled(moduleId))
                log2.onLogFuncLifeExt(moduleId, tag, funcName, lifeTag, v1, v2, v3);
        } else {
            if (isFunc2Enabled(moduleId))
                log2.onLogFuncLifeExt(moduleId, tag, funcName, lifeTag, v1, v2, v3);
            log1.onLogFuncLifeExt(moduleId, tag, funcName, lifeTag, v1, v2, v3);
        }
    }

    virtual void onLogDiscard(ModuleId byModuleId, const char *tag, RequestTypeId requestTypeId, const RequestSerial *requestSerialList, size_t n) override
    {
        log1.onLogDiscard(byModuleId, tag, requestTypeId, requestSerialList, n);
        log2.onLogDiscard(byModuleId, tag, requestTypeId, requestSerialList, n);
    }

    virtual void onLogPathDiv(ModuleId fromModuleId, const char *tag, RequestTypeId requestTypeId, RequestSerial requestSerial,
        const ModuleId *toModuleIdList, size_t n) override
    {
        log1.onLogPathDiv(fromModuleId, tag, requestTypeId, requestSerial, toModuleIdList, n);
        log2.onLogPathDiv(fromModuleId, tag, requestTypeId, requestSerial, toModuleIdList, n);
    }

    virtual void onLogPathJoin(ModuleId toModuleId, const char *tag, RequestTypeId requestTypeId, RequestSerial requestSerial,
        const ModuleId *fromModuleIdList, size_t n) override
    {
        log1.onLogPathJoin(toModuleId, tag, requestTypeId, requestSerial, fromModuleIdList, n);
        log2.onLogPathJoin(toModuleId, tag, requestTypeId, requestSerial, fromModuleIdList, n);
    }

    virtual void onLogValue(ModuleId moduleId, const char *tag, const char *name, std::int32_t value) override
    {
        log1.onLogValue(moduleId, tag, name, value);
        log2.onLogValue(moduleId, tag, name, value);
    }

    virtual void onFlush(int waitDoneSec) override
    {
        log1.onFlush(waitDoneSec);
        log2.onFlush(waitDoneSec);
    }

    virtual void onInit() override
    {
        log1.onInit();
        log2.onInit();
    }

    virtual void onUninit() override
    {
        log2.onUninit();
        log1.onUninit();
    }
};

}
}
}


#endif

