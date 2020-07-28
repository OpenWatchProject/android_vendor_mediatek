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

#ifndef _MTKCAM_FEATURE_UTILS_P2_PACK_H_
#define _MTKCAM_FEATURE_UTILS_P2_PACK_H_

#include <map>

#include <utils/RefBase.h>

#include <mtkcam3/feature/utils/p2/P2PlatInfo.h>
#include <mtkcam3/feature/utils/p2/P2Data.h>

namespace NSCam {
namespace Feature {
namespace P2Util {

class P2InfoObj : virtual public android::RefBase
{
public:
    P2InfoObj(const ILog &log);
    virtual ~P2InfoObj();
    void addSensorInfo(const ILog &log, unsigned sensorID);

    android::sp<P2InfoObj> clone() const;
    const P2ConfigInfo& getConfigInfo() const;
    const P2SensorInfo& getSensorInfo(unsigned sensorID) const;

public:
    ILog mLog;
    P2ConfigInfo mConfigInfo;
    std::map<unsigned,P2SensorInfo> mSensorInfoMap;
};

class P2Info
{
public:
    P2Info();
    P2Info(const P2Info &info, const ILog &log, unsigned sensorID);
    P2Info(const android::sp<P2InfoObj> &infoObj, const ILog &log, unsigned sensorID);
    const P2ConfigInfo& getConfigInfo() const;
    const P2SensorInfo& getSensorInfo() const;
    const P2SensorInfo& getSensorInfo(unsigned sensorID) const;
    const P2PlatInfo* getPlatInfo() const;

public:
    ILog mLog;

private:
    android::sp<P2InfoObj> mInfoObj;
    const P2ConfigInfo* mConfigInfoPtr = &P2ConfigInfo::Dummy;
    const P2SensorInfo* mSensorInfoPtr = &P2SensorInfo::Dummy;
    const P2PlatInfo* mPlatInfoPtr = NULL;
};

class P2DataObj : virtual public android::RefBase
{
public:
    P2DataObj(const ILog &log);
    virtual ~P2DataObj();
    const P2FrameData& getFrameData() const;
    const P2SensorData& getSensorData(unsigned sensorID) const;

public:
    ILog mLog;
    P2FrameData mFrameData;
    std::map<unsigned,P2SensorData> mSensorDataMap;
};

class P2Data
{
public:
    P2Data();
    P2Data(const P2Data &data, const ILog &log, unsigned sensorID);
    P2Data(const android::sp<P2DataObj> &dataObj, const ILog &log, unsigned sensorID);
    const P2FrameData& getFrameData() const;
    const P2SensorData& getSensorData() const;
    const P2SensorData& getSensorData(unsigned sensorID) const;

public:
    ILog mLog;

private:
    android::sp<const P2DataObj> mDataObj;
    const P2FrameData* mFrameDataPtr = &P2FrameData::Dummy;
    const P2SensorData* mSensorDataPtr = &P2SensorData::Dummy;
};

class P2Pack
{
public:
    P2Pack();
    P2Pack(const ILog &log, const android::sp<P2InfoObj> &info, const android::sp<P2DataObj> &data);
    P2Pack(const P2Pack &src, const ILog &log, unsigned sensorID);

    P2Pack getP2Pack(const ILog &log, unsigned sensorID) const;

    const P2PlatInfo* getPlatInfo() const;
    const P2ConfigInfo& getConfigInfo() const;
    const P2SensorInfo& getSensorInfo() const;
    const P2SensorInfo& getSensorInfo(unsigned sensorID) const;
    const P2FrameData& getFrameData() const;
    const P2SensorData& getSensorData() const;
    const P2SensorData& getSensorData(unsigned sensorID) const;
    MBOOL isValid() const {return mIsValid;}

public:
    ILog mLog;

private:
    bool mIsValid = false;
    P2Info mInfo;
    P2Data mData;
};

} // namespace P2Util
} // namespace Feature
} // namespace NSCam

#endif // _MTKCAM_FEATURE_UTILS_P2_PACK_H_
