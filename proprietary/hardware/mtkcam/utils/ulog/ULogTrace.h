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

#ifndef __MTKCAM_UTILS_STD_ULOG_TRACE_H__
#define __MTKCAM_UTILS_STD_ULOG_TRACE_H__

#include <cstring>
#include <mtkcam/utils/std/Trace.h>
#include <mtkcam/utils/std/ULog.h>


namespace NSCam {
namespace Utils {
namespace ULog {

class TraceULogger final : public ULogger
{
private:
    static inline char *fastConcat(char *dest, size_t &maxChars, const char *str)
    {
        while (maxChars > 1 && *str != '\0') {
            *dest = *str;
            dest++;
            str++;
            maxChars--;
        }
        dest[0] = '\0';

        return dest;
    }

public:
    virtual void onLogEnter(ModuleId intoModuleId, const char *, RequestTypeId requestTypeId, RequestSerial requestSerial) override
    {
        char name[128];

        char *tail = name;
        size_t maxChars = sizeof(name);
        tail = fastConcat(tail, maxChars, getRequestTypeName(requestTypeId));
        tail = fastConcat(tail, maxChars, " ");
        tail = fastConcat(tail, maxChars, getModuleName(intoModuleId));

        CAM_TRACE_ASYNC_BEGIN(name, requestSerial);
    }

    virtual void onLogExit(ModuleId outFromModuleId, const char *, RequestTypeId requestTypeId, RequestSerial requestSerial) override
    {
        char name[128];

        char *tail = name;
        size_t maxChars = sizeof(name);
        tail = fastConcat(tail, maxChars, getRequestTypeName(requestTypeId));
        tail = fastConcat(tail, maxChars, " ");
        tail = fastConcat(tail, maxChars, getModuleName(outFromModuleId));

        CAM_TRACE_ASYNC_END(name, requestSerial);
    }

    virtual void onLogDiscard(ModuleId byModuleId, const char *tag, RequestTypeId requestTypeId, const RequestSerial *requestSerialList, size_t n) override
    {
        for (size_t i = 0; i < n; i++) {
            TraceULogger::onLogExit(byModuleId, tag, requestTypeId, requestSerialList[i]);
        }
    }

    virtual void onLogFuncLife(ModuleId moduleId, const char *, const char *funcName, FuncLifeTag lifeTag) override
    {
        switch (lifeTag) {
        case API_ENTER:
            {
                char name[256];

                char *tail = name;
                size_t maxChars = sizeof(name);
                tail = fastConcat(tail, maxChars, getModuleName(moduleId));
                tail = fastConcat(tail, maxChars, "/");
                tail = fastConcat(tail, maxChars, funcName);

                CAM_TRACE_BEGIN(name);
            }
            break;
        case FUNCTION_ENTER:
        case SUBROUTINE_ENTER:
            CAM_TRACE_BEGIN(funcName);
            break;
        default:
            if (lifeTag & EXIT_BIT) {
                CAM_TRACE_END();
            }
            break;
        }
    }

    virtual void onLogFuncLifeExt(ModuleId moduleId, const char *, const char *funcName, FuncLifeTag lifeTag,
        std::intptr_t v1, std::intptr_t , std::intptr_t ) override
    {
        char name[256];

        switch (lifeTag) {
        case API_ENTER:
            if (-100000 < v1 && v1 < 100000) { // a small value
                snprintf(name, sizeof(name), "%s/%s(%d)", getModuleName(moduleId), funcName, static_cast<int>(v1));
            } else {
                char *tail = name;
                size_t maxChars = sizeof(name);
                tail = fastConcat(tail, maxChars, getModuleName(moduleId));
                tail = fastConcat(tail, maxChars, "/");
                tail = fastConcat(tail, maxChars, funcName);
            }
            CAM_TRACE_BEGIN(name);
            break;
        case FUNCTION_ENTER:
            if (-100000 < v1 && v1 < 100000) { // a small value
                snprintf(name, sizeof(name), "%s(%d)", funcName, static_cast<int>(v1));
                CAM_TRACE_BEGIN(name);
            } else {
                CAM_TRACE_BEGIN(funcName);
            }
            break;
        case SUBROUTINE_ENTER:
            CAM_TRACE_BEGIN(funcName);
            break;
        default:
            if (lifeTag & EXIT_BIT) {
                CAM_TRACE_END();
            }
            break;
        }
    }

    virtual void onLogValue(ModuleId , const char *, const char *name, std::int32_t value) override
    {
        CAM_TRACE_INT(name, value);
    }
};

}
}
}

#endif

