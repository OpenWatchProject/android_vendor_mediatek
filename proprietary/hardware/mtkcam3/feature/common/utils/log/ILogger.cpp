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

#include <mtkcam3/feature/utils/log/ILogger.h>

namespace NSCam {
namespace Feature {

ILog::ILog()
{
}

ILog::ILog(const android::sp<ILogObj> &log)
    : mLog(log)
{
}

ILog::~ILog()
{
}

class DefaultLogger : public ILogObj
{
public:
    DefaultLogger(const char *str, const char *name, unsigned level, unsigned sensor, unsigned mwFrame, unsigned mwRequest, unsigned frame, unsigned request);
    virtual ~DefaultLogger();
    const char* getLogStr() const;
    const char* getUserName() const;
    unsigned getLogLevel() const;
    unsigned getLogSensorID() const;
    unsigned getLogFrameID() const;
    unsigned getLogMWFrameID() const;
    unsigned getLogRequestID() const;
    unsigned getLogMWRequestID() const;
private:
    std::string mStr;
    std::string mUserName;
    unsigned mLogLevel;
    unsigned mSensorID;
    unsigned mMWFrameID;
    unsigned mMWRequestID;
    unsigned mFrameID;
    unsigned mRequestID;
};

DefaultLogger::DefaultLogger(const char *str, const char *name, unsigned level, unsigned sensor, unsigned mwFrame, unsigned mwRequest, unsigned frame, unsigned request)
    : mStr(str)
    , mUserName(name)
    , mLogLevel(level)
    , mSensorID(sensor)
    , mMWFrameID(mwFrame)
    , mMWRequestID(mwRequest)
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

const char* DefaultLogger::getUserName() const
{
    return mUserName.c_str();
}

unsigned DefaultLogger::getLogLevel() const
{
    return mLogLevel;
}

unsigned DefaultLogger::getLogSensorID() const
{
    return mSensorID;
}

unsigned DefaultLogger::getLogFrameID() const
{
    return mFrameID;
}

unsigned DefaultLogger::getLogRequestID() const
{
    return mRequestID;
}

unsigned DefaultLogger::getLogMWFrameID() const
{
    return mMWFrameID;
}

unsigned DefaultLogger::getLogMWRequestID() const
{
    return mMWRequestID;
}

ILog makeLogger(const char *str, const char *name, unsigned logLevel, unsigned sensorID, unsigned mwFrameID, unsigned mwRequestID, unsigned frameID, unsigned requestID)
{
    return ILog(new DefaultLogger(str, name, logLevel, sensorID, mwFrameID, mwRequestID, frameID, requestID));
}

ILog makeSensorLogger(const char *name, unsigned logLevel, unsigned sensorID)
{
    char str[128];
    snprintf(str, sizeof(str), "%s cam %d", name, sensorID);
    return makeLogger(str, name, logLevel, sensorID);
}

ILog makeFrameLogger(const char *name, unsigned logLevel, unsigned sensorID, unsigned mwFrameID, unsigned mwRequestID, unsigned frameID)
{
    char str[128];
    snprintf(str, sizeof(str), "%s cam %d MWFrame:#%d MWReq:#%d, frame %d ", name, sensorID, mwFrameID, mwRequestID, frameID);
    return makeLogger(str, name, logLevel, sensorID, mwFrameID, mwRequestID, frameID);
}

ILog makeRequestLogger(const char *name, unsigned logLevel, unsigned sensorID, unsigned mwFrameID, unsigned mwRequestID, unsigned frameID, unsigned requestID)
{
    char str[128];
    snprintf(str, sizeof(str), "%s cam %d MWFrame:#%d MWReq:#%d, frame %d-%d ", name, sensorID, mwFrameID, mwRequestID, frameID, requestID);
    return makeLogger(str, name, logLevel, sensorID, mwFrameID, mwRequestID, frameID, requestID);
}

ILog makeSensorLogger(const ILog &log, unsigned sensorID)
{
    return makeSensorLogger(log.getUserName(), log.getLogLevel(), sensorID);
}

ILog makeFrameLogger(const ILog &log, unsigned mwFrameID, unsigned mwRequestID, unsigned frameID)
{
    return makeFrameLogger(log.getUserName(), log.getLogLevel(), log.getLogSensorID(), mwFrameID, mwRequestID, frameID);
}

ILog makeRequestLogger(const ILog &log, unsigned requestID)
{
    return makeRequestLogger(log.getUserName(), log.getLogLevel(), log.getLogSensorID(), log.getLogMWFrameID(), log.getLogMWRequestID(), log.getLogFrameID(), requestID);
}

ILog makeRequestLogger(const ILog &log, unsigned logLevel, unsigned requestID)
{
    return makeRequestLogger(log.getUserName(), logLevel, log.getLogSensorID(), log.getLogMWFrameID(), log.getLogMWRequestID(), log.getLogFrameID(), requestID);
}

ILog makeSubSensorLogger(const ILog &log, unsigned sensorID)
{
    return makeRequestLogger(log.getUserName(), log.getLogLevel(), sensorID, log.getLogMWFrameID(), log.getLogMWRequestID(), log.getLogFrameID(), log.getLogRequestID());
}

} // namespace Feature
} // namespace NSCam
