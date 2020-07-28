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

#include <mtkcam3/feature/utils/p2/P2Pack.h>

namespace NSCam {
namespace Feature {
namespace P2Util {

P2InfoObj::P2InfoObj(const ILog &log)
    : mLog(log)
{
}

P2InfoObj::~P2InfoObj()
{
}

android::sp<P2InfoObj> P2InfoObj::clone() const
{
    android::sp<P2InfoObj> child = new P2InfoObj(this->mLog);
    child->mConfigInfo = this->mConfigInfo;
    child->mSensorInfoMap = this->mSensorInfoMap;
    return child;
}

const P2ConfigInfo& P2InfoObj::getConfigInfo() const
{
    return mConfigInfo;
}

void P2InfoObj::addSensorInfo(const ILog &log, unsigned sensorID)
{
    mConfigInfo.mAllSensorID.push_back(sensorID);
    mSensorInfoMap[sensorID] = P2SensorInfo(log, sensorID);
}

const P2SensorInfo& P2InfoObj::getSensorInfo(unsigned sensorID) const
{
    auto it = mSensorInfoMap.find(sensorID);
    return (it != mSensorInfoMap.end()) ? it->second : P2SensorInfo::Dummy;
}

P2Info::P2Info()
{
}

P2Info::P2Info(const P2Info &info, const ILog &log, unsigned sensorID)
    : mLog(log)
    , mInfoObj(info.mInfoObj)
    , mConfigInfoPtr(info.mConfigInfoPtr)
    , mSensorInfoPtr(&info.getSensorInfo(sensorID))
    , mPlatInfoPtr(P2PlatInfo::getInstance(sensorID))
{
}

P2Info::P2Info(const android::sp<P2InfoObj> &infoObj, const ILog &log, unsigned sensorID)
    : mLog(log)
    , mInfoObj(infoObj)
    , mPlatInfoPtr(P2PlatInfo::getInstance(sensorID))
{
    if( mInfoObj != NULL )
    {
        mConfigInfoPtr = &mInfoObj->getConfigInfo();
        mSensorInfoPtr = &mInfoObj->getSensorInfo(sensorID);
    }
}

const P2ConfigInfo& P2Info::getConfigInfo() const
{
    return *mConfigInfoPtr;
}

const P2SensorInfo& P2Info::getSensorInfo() const
{
    return *mSensorInfoPtr;
}

const P2SensorInfo& P2Info::getSensorInfo(unsigned sensorID) const
{
    return mInfoObj->getSensorInfo(sensorID);
}

const P2PlatInfo* P2Info::getPlatInfo() const
{
    return mPlatInfoPtr;
}

P2DataObj::P2DataObj(const ILog &log)
    : mLog(log)
{
}

P2DataObj::~P2DataObj()
{
}

const P2FrameData& P2DataObj::getFrameData() const
{
    return mFrameData;
}

const P2SensorData& P2DataObj::getSensorData(unsigned sensorID) const
{
    auto it = mSensorDataMap.find(sensorID);
    return (it != mSensorDataMap.end()) ? it->second : P2SensorData::Dummy;
}

P2Data::P2Data()
{
}

P2Data::P2Data(const P2Data &data, const ILog &log, unsigned sensorID)
    : mLog(log)
    , mDataObj(data.mDataObj)
    , mFrameDataPtr(data.mFrameDataPtr)
    , mSensorDataPtr(&data.getSensorData(sensorID))
{
}

P2Data::P2Data(const android::sp<P2DataObj> &dataObj, const ILog &log, unsigned sensorID)
    : mLog(log)
    , mDataObj(dataObj)
{
    if( mDataObj != NULL )
    {
        mFrameDataPtr = &mDataObj->getFrameData();
        if(dataObj->mSensorDataMap.count(sensorID) == 0)
        {
            dataObj->mSensorDataMap[sensorID] = P2SensorData();
        }
        mSensorDataPtr = &mDataObj->getSensorData(sensorID);
    }
}

const P2FrameData& P2Data::getFrameData() const
{
    return *mFrameDataPtr;
}

const P2SensorData& P2Data::getSensorData() const
{
    return *mSensorDataPtr;
}

const P2SensorData& P2Data::getSensorData(unsigned sensorID) const
{
    return mDataObj->getSensorData(sensorID);
}

P2Pack::P2Pack()
{
}

P2Pack::P2Pack(const ILog &log, const android::sp<P2InfoObj> &info, const android::sp<P2DataObj> &data)
    : mLog(log)
{
    if( info != NULL )
    {
        mIsValid = true;
        unsigned mainID = info->getConfigInfo().mMainSensorID;
        mInfo = P2Info(info, log, mainID);
        mData = P2Data(data, log, mainID);
    }
}

P2Pack::P2Pack(const P2Pack &src, const ILog &log, unsigned sensorID)
    : mLog(log)
    , mIsValid(src.mIsValid)
    , mInfo(src.mInfo, log, sensorID)
    , mData(src.mData, log, sensorID)
{
}

P2Pack P2Pack::getP2Pack(const ILog &log, unsigned sensorID) const
{
    return P2Pack(*this, log, sensorID);
}

const P2PlatInfo* P2Pack::getPlatInfo() const
{
    return mInfo.getPlatInfo();
}

const P2ConfigInfo& P2Pack::getConfigInfo() const
{
    return mInfo.getConfigInfo();
}

const P2SensorInfo& P2Pack::getSensorInfo() const
{
    return mInfo.getSensorInfo();
}

const P2SensorInfo& P2Pack::getSensorInfo(unsigned sensorID) const
{
    return mInfo.getSensorInfo(sensorID);
}

const P2FrameData& P2Pack::getFrameData() const
{
    return mData.getFrameData();
}

const P2SensorData& P2Pack::getSensorData() const
{
    return mData.getSensorData();
}

const P2SensorData& P2Pack::getSensorData(unsigned sensorID) const
{
    return mData.getSensorData(sensorID);
}

} // namespace P2Util
} // namespace Feature
} // namespace NSCam
