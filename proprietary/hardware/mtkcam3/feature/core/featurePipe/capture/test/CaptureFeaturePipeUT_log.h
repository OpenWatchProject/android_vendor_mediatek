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
 * MediaTek Inc. (C) 2018. All rights reserved.
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

#ifndef _MTK_CAMERA_CAPTURE_FEATURE_PIPE_UT_LOG_H_
#define _MTK_CAMERA_CAPTURE_FEATURE_PIPE_UT_LOG_H_

// Standard C header file
#include <cstring>
#include <functional>
#include <memory>
#include <cstdio>
#include <cstdarg>
#include <iostream>
// Android system/core header file

// mtkcam custom header file

// mtkcam global header file

// Module header file
#include <mtkcam/utils/std/Log.h>
// Local header file

/*******************************************************************************
* Utilities Define.
********************************************************************************/
namespace { // TODO: what do us need to use anonymous namespace?
using AutoObject = std::unique_ptr<const char, std::function<void(const char*)>>;

std::unique_ptr<char>
getPreFix(const char* text)
{
    static const MINT32 LENGTH = 256;
    auto ret = std::unique_ptr<char>(new char[LENGTH]);
    ::strcpy(ret.get(), "[");
    ::strcat(ret.get(), text);
    ::strcat(ret.get(), "] ");
    return ret;
}
//
void printLog(const char* text, const char* fmt, ...)
{
    static const MINT32 LENGTH = 1024;
    auto buf = std::unique_ptr<char>(new char[LENGTH]);

    ::strcpy(buf.get(), getPreFix(text).get());
    ::strcat(buf.get(), fmt);

    const MINT32 newLength = LENGTH - ::strlen(buf.get());

    va_list ap;
    ::va_start(ap, fmt);
    ::vsnprintf(buf.get(), newLength, fmt, ap);
    ::va_end(ap);

    std::cout << buf.get() << std::endl;
}
//
auto
createAutoScoper(const char* funcName) -> AutoObject
{
    CAM_LOGD("[%s] +", funcName);
    return AutoObject(funcName, [](const char* p)
    {
        CAM_LOGD("[%s] -", p);
    });
}


} // anonymous namespace


/*******************************************************************************
* MACRO Define.
********************************************************************************/
#define UT_LOGD(FMT, arg...) printLog(__FUNCTION__, FMT, ##arg)
#define SCOPED_TRACER_NAME(TEXT) auto scoped_tracer = ::createAutoScoper(TEXT)
#define SCOPED_TRACER() auto scoped_tracer = ::createAutoScoper(__FUNCTION__)


#endif // _MTK_CAMERA_CAPTURE_FEATURE_PIPE_UT_LOG_H_
