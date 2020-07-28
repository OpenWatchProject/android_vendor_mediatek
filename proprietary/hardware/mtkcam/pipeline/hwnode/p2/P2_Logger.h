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

#ifndef _MTKCAM_HWNODE_P2_LOGGER
#define _MTKCAM_HWNODE_P2_LOGGER

#include "P2_Header.h"

namespace P2
{

MBOOL isTrace1On();
MBOOL isTrace2On();

#define DEFAULT_LOGGER_STR ""
#define DEFAULT_LOGGER_ID     ((MUINT32)-1)

class ILogger
{
public:
    virtual ~ILogger() {}
    virtual const char* getLogStr() const = 0;
    virtual MUINT32 getLogLevel() const = 0;
    virtual MUINT32 getLogSensorID() const = 0;
    virtual MUINT32 getLogFrameID() const = 0;
    virtual MUINT32 getLogRequestID() const = 0;
};

class ISharedLogger : virtual public android::RefBase, public ILogger
{
public:
    virtual ~ISharedLogger() {}
};

typedef sp<ISharedLogger> Logger;

class DefaultLogger : virtual public ISharedLogger
{
public:
    DefaultLogger();
    DefaultLogger(const char *str, MUINT32 logLevel, MUINT32 sensor, MUINT32 frame, MUINT32 request);
    DefaultLogger(const std::string &str, MUINT32 logLevel, MUINT32 sensor, MUINT32 frame, MUINT32 request);
    virtual ~DefaultLogger();
    const char* getLogStr() const;
    MUINT32 getLogLevel() const;
    MUINT32 getLogSensorID() const;
    MUINT32 getLogFrameID() const;
    MUINT32 getLogRequestID() const;
private:
    const std::string mStr;
    MUINT32 mLogLevel;
    MUINT32 mSensorID;
    MUINT32 mFrameID;
    MUINT32 mRequestID;
};

class ILoggerProvider : public virtual ILogger
{
public:
    virtual ~ILoggerProvider() {}
    virtual sp<ISharedLogger> getLogger() const = 0;
    virtual const char* getLogStr() const;
    virtual MUINT32 getLogLevel() const;
    virtual MUINT32 getLogSensorID() const;
    virtual MUINT32 getLogFrameID() const;
    virtual MUINT32 getLogRequestID() const;
};

class LoggerHolder : public virtual ILoggerProvider
{
public:
    LoggerHolder(const sp<ISharedLogger> &logger);
    LoggerHolder(const ILoggerProvider &provider);
    template <typename T> LoggerHolder(const sp<T> &logger);
    virtual sp<ISharedLogger> getLogger() const;

protected:
    const sp<ISharedLogger> mLogger;
};

template <typename T>
LoggerHolder::LoggerHolder(const sp<T> &logger)
    : mLogger(logger != NULL ? logger->getLogger() : NULL)
{
}

Logger makeSensorLogger(MUINT32 logLevel, MUINT32 sensorID);
Logger makeFrameLogger(MUINT32 logLevel, MUINT32 sensorID, MUINT32 frameID);
Logger makeRequestLogger(MUINT32 logLevel, MUINT32 sensorID, MUINT32 frameID, MUINT32 subID);

template<typename T>
Logger getLogger(const android::sp<T> &logHolder)
{
    return (logHolder != NULL) ? logHolder->getLogger() : NULL;
}

const char* getLogStr(const char* str);
const char* getLogStr(const std::string &str);
const char* getLogStr(const ILogger &logger);
const char* getLogStr(const ILogger *logger);
const char* getLogStr(const ILoggerProvider &provider);
const char* getLogStr(const ILoggerProvider *provider);

template<typename T>
const char* getLogStr(const android::sp<T> &logger)
{
    return (logger != NULL) ? logger->getLogStr() : DEFAULT_LOGGER_STR;
}

} // namespace P2

#endif // _MTKCAM_HWNODE_P2_LOGGER
