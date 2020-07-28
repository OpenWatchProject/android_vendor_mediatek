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

#include "P2_Logger.h"

#include "P2_DebugControl.h"
#define P2_CLASS_TAG    P2_Logger
#define P2_TRACE        TRACE_P2_LOGGER
#include "P2_LogHeader.h"

namespace P2
{

MBOOL isTrace1On()
{
    static int spVal = property_get_int32("persist.vendor." KEY_TRACE_P2, 0);
    static int sdVal = property_get_int32("vendor.debug." KEY_TRACE_P2, 0);
    static int sVal = sdVal ? sdVal : spVal;
    static bool sTrace = (sVal == 1);
    return sTrace;
}

MBOOL isTrace2On()
{
    static int spVal = property_get_int32("persist.vendor." KEY_TRACE_P2, 0);
    static int sdVal = property_get_int32("vendor.debug." KEY_TRACE_P2, 0);
    static int sVal = sdVal ? sdVal : spVal;
    static bool sTrace = (sVal >= 1 && sVal <= 2);
    return sTrace;
}

DefaultLogger::DefaultLogger()
    : mStr(DEFAULT_LOGGER_STR)
    , mLogLevel(0)
    , mSensorID(DEFAULT_LOGGER_ID)
    , mFrameID(DEFAULT_LOGGER_ID)
    , mRequestID(DEFAULT_LOGGER_ID)
{
}

DefaultLogger::DefaultLogger(const char *str, MUINT32 logLevel, MUINT32 sensor, MUINT32 frame, MUINT32 request)
    : mStr(str)
    , mLogLevel(logLevel)
    , mSensorID(sensor)
    , mFrameID(frame)
    , mRequestID(request)
{
}

DefaultLogger::DefaultLogger(const std::string &str, MUINT32 logLevel, MUINT32 sensor, MUINT32 frame, MUINT32 request)
    : mStr(str)
    , mLogLevel(logLevel)
    , mSensorID(sensor)
    , mFrameID(frame)
    , mRequestID(request)
{
}

DefaultLogger::~DefaultLogger()
{
}

const char* DefaultLogger::getLogStr() const
{
    return mStr.c_str();
}

MUINT32 DefaultLogger::getLogLevel() const
{
    return mLogLevel;
}

MUINT32 DefaultLogger::getLogSensorID() const
{
    return mSensorID;
}

MUINT32 DefaultLogger::getLogFrameID() const
{
    return mFrameID;
}

MUINT32 DefaultLogger::getLogRequestID() const
{
    return mRequestID;
}

const char* ILoggerProvider::getLogStr() const
{
    const Logger log = getLogger();
    return (log != NULL) ? log->getLogStr() : DEFAULT_LOGGER_STR;
}

MUINT32 ILoggerProvider::getLogLevel() const
{
    const Logger log = getLogger();
    return (log != NULL) ? log->getLogLevel() : 0;
}

MUINT32 ILoggerProvider::getLogSensorID() const
{
    const Logger log = getLogger();
    return (log != NULL) ? log->getLogSensorID() : DEFAULT_LOGGER_ID;
}

MUINT32 ILoggerProvider::getLogFrameID() const
{
    const Logger log = getLogger();
    return (log != NULL) ? log->getLogFrameID() : DEFAULT_LOGGER_ID;
}

MUINT32 ILoggerProvider::getLogRequestID() const
{
    const Logger log = getLogger();
    return (log != NULL) ? log->getLogRequestID() : DEFAULT_LOGGER_ID;
}

LoggerHolder::LoggerHolder(const sp<ISharedLogger> &logger)
    : mLogger(logger)
{
}

LoggerHolder::LoggerHolder(const ILoggerProvider &provider)
    : mLogger(provider.getLogger())
{
}

sp<ISharedLogger> LoggerHolder::getLogger() const
{
    return mLogger;
}

Logger makeLogger(const char* str, MUINT32 logLevel, MUINT32 sensor=DEFAULT_LOGGER_ID, MUINT32 frame=DEFAULT_LOGGER_ID, MUINT32 request=DEFAULT_LOGGER_ID)
{
    Logger logger = new DefaultLogger(str, logLevel, sensor, frame, request);
    if( logger == NULL )
    {
        MY_LOGW("OOM: cannot create DefaultLogger");
    }
    return logger;
}

Logger makeSensorLogger(MUINT32 logLevel, MUINT32 sensorID)
{
    char str[128];
    snprintf(str, sizeof(str), "cam %d", sensorID);
    Logger logger = makeLogger(str, logLevel, sensorID);
    return logger;
}

Logger makeFrameLogger(MUINT32 logLevel, MUINT32 sensorID, MUINT32 frameID)
{
    char str[128];
    snprintf(str, sizeof(str), "cam %d frame %d", sensorID, frameID);
    Logger logger = makeLogger(str, logLevel, sensorID, frameID);
    return logger;
}

Logger makeRequestLogger(MUINT32 logLevel, MUINT32 sensorID, MUINT32 frameID, MUINT32 subID)
{
    char str[128];
    snprintf(str, sizeof(str), "cam %d frame %d-%d", sensorID, frameID, subID);
    Logger logger = makeLogger(str, logLevel, sensorID, frameID, subID);
    return logger;
}

const char* getLogStr(const char* str)
{
    return str ? str : DEFAULT_LOGGER_STR;
}

const char* getLogStr(const std::string &str)
{
    return str.c_str();
}

const char* getLogStr(const ILogger &logger)
{
    return logger.getLogStr();
}

const char* getLogStr(const ILogger *logger)
{
    return logger ? logger->getLogStr() : DEFAULT_LOGGER_STR;
}

const char* getLogStr(const ILoggerProvider &provider)
{
    return provider.getLogStr();
}

const char* getLogStr(const ILoggerProvider *provider)
{
    return provider ? provider->getLogStr() : DEFAULT_LOGGER_STR;
}

} // namespace P2
