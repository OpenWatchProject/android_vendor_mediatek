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

#ifndef _MTKCAM_FEATURE_UTILS_ILOGGER_H_
#define _MTKCAM_FEATURE_UTILS_ILOGGER_H_

#include <string>

#include <utils/RefBase.h>

namespace NSCam {
namespace Feature {

class ILogBase
{
public:
    virtual ~ILogBase() {}
    virtual const char* getLogStr() const = 0;
    virtual const char* getUserName() const = 0;
    virtual unsigned getLogLevel() const = 0;
    virtual unsigned getLogSensorID() const = 0;
    virtual unsigned getLogMWFrameID() const = 0;
    virtual unsigned getLogMWRequestID() const = 0;
    virtual unsigned getLogFrameID() const = 0;
    virtual unsigned getLogRequestID() const = 0;
};

class ILogObj : virtual public android::RefBase, public ILogBase
{
public:
    virtual ~ILogObj() {}
};

#define DECL_ILOG(type, func, def)                    \
    virtual inline type func() const                  \
    {                                                 \
        return (mLog != NULL) ? mLog->func() : (def); \
    }

class ILog : public ILogBase
{
public:
    ILog();
    ILog(const android::sp<ILogObj> &log);
    virtual ~ILog();
    DECL_ILOG(const char*, getLogStr, "");
    DECL_ILOG(const char*, getUserName, "unknwon");
    DECL_ILOG(unsigned, getLogLevel, 0);
    DECL_ILOG(unsigned, getLogSensorID, -1);
    DECL_ILOG(unsigned, getLogMWFrameID, 0);
    DECL_ILOG(unsigned, getLogMWRequestID, 0);
    DECL_ILOG(unsigned, getLogFrameID, 0);
    DECL_ILOG(unsigned, getLogRequestID, 0);

private:
    android::sp<ILogObj> mLog;
};
#undef DECL_ILOG

inline const char* getLogStr(const char* str) { return str; }
inline const char* getLogStr(const std::string &str) { return str.c_str(); }
inline const char* getLogStr(const ILog &log) { return log.getLogStr(); }

ILog makeLogger(const char *str, const char *name = "unknwon", unsigned logLevel = 0, unsigned sensorID = -1, unsigned mwFrameID = 0, unsigned mwRequestID = 0, unsigned frameID = 0, unsigned requestID = 0);
ILog makeSensorLogger(const char *name, unsigned logLevel, unsigned sensorID);
ILog makeFrameLogger(const char *name, unsigned logLevel, unsigned sensorID, unsigned mwFrameID, unsigned mwRequestID, unsigned frameID);
ILog makeRequestLogger(const char *name, unsigned logLevel, unsigned sensorID, unsigned mwFrameID, unsigned mwRequestID, unsigned frameID, unsigned requestID);
ILog makeSensorLogger(const ILog &log, unsigned sensorID);
ILog makeFrameLogger(const ILog &log, unsigned mwFrameID, unsigned mwRequestID, unsigned frameID);
ILog makeRequestLogger(const ILog &log, unsigned requestID);
ILog makeRequestLogger(const ILog &log, unsigned logLevel, unsigned requestID);
ILog makeSubSensorLogger(const ILog &log, unsigned sensorID);

template <typename T>
inline ILog spToILog(T &p)
{
    return p != NULL ? p->mLog : ILog();
}

} // namespace Feature
} // namespace NSCam

#endif // _MTKCAM_FEATURE_UTILS_ILOGGER_H_
