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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#include "DualCamMFPipe_Common.h"

// Module header file
#define PIPE_MODULE_TAG "DualCamMF"
#define PIPE_CLASS_TAG "Common"
#define PIPE_LOG_TAG PIPE_MODULE_TAG PIPE_CLASS_TAG
#include <featurePipe/core/include/PipeLog.h>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace NSDCMF {

/*******************************************************************************
* ImageBufInfoMap
********************************************************************************/
ImageBufInfoMap::
ImageBufInfoMap(sp<PipeRequest> ptr)
:mpReqPtr(ptr)
{

}

ssize_t
ImageBufInfoMap::
addIImageBuffer(const DualCamMFBufferID& key, const sp<IImageBuffer>& value)
{
    sp<IImageBuffer> spBuf = value;
    mvIImageBufData[key] = spBuf;
    return mvIImageBufData.size();
}

ssize_t
ImageBufInfoMap::
addSmartBuffer(const DualCamMFBufferID& key, const SmartImageBuffer& value)
{
    return mvSmartImgBufData.add(key, value);
}

ssize_t
ImageBufInfoMap::
addGraphicBuffer(const DualCamMFBufferID& key, const SmartGraphicBuffer& value)
{
    return mvGraImgBufData.add(key, value);
}

ssize_t
ImageBufInfoMap::
addMfllBuffer(const DualCamMFBufferID& key, const sp<mfll::IMfllImageBuffer>& value)
{
    sp<mfll::IMfllImageBuffer> spBuf = value;
    mvMfllImgBufData[key] = spBuf;
    return mvMfllImgBufData.size();
}

ssize_t
ImageBufInfoMap::
addMetadata(const DualCamMFBufferID& key, IMetadata* const value)
{
    return mvMetaPtrData.add(key, value);
}

ssize_t
ImageBufInfoMap::
addSolidMetadata(const DualCamMFBufferID& key, IMetadata const value)
{
    return mvMetaData.add(key, value);
}

ssize_t
ImageBufInfoMap::
addExtBufName(const DualCamMFBufferID& key, const string& value)
{
    return mvExtNameSet.add(key, value);
}

ssize_t
ImageBufInfoMap::
delIImageBuffer(const DualCamMFBufferID& key)
{
    return mvIImageBufData.erase(key);
}

ssize_t
ImageBufInfoMap::
delSmartBuffer(const DualCamMFBufferID& key)
{
    return mvSmartImgBufData.removeItem(key);
}

ssize_t
ImageBufInfoMap::
delGraphicBuffer(const DualCamMFBufferID& key)
{
    return mvGraImgBufData.removeItem(key);
}

ssize_t
ImageBufInfoMap::
delMfllBuffer(const DualCamMFBufferID& key)
{
    return mvMfllImgBufData.erase(key);
}

ssize_t
ImageBufInfoMap::
delMetadata(const DualCamMFBufferID& key)
{
    return mvMetaPtrData.removeItem(key);
}

ssize_t
ImageBufInfoMap::
delSolidMetadata(const DualCamMFBufferID& key)
{
    return mvMetaData.removeItem(key);
}

const sp<IImageBuffer>
ImageBufInfoMap::
getIImageBuffer(const DualCamMFBufferID& key)
{
    CAM_LOGW_IF(mvIImageBufData.count(key) <= 0, "%d: key not found", key);
    return mvIImageBufData.count(key)>0? mvIImageBufData[key] : nullptr;
}

const SmartImageBuffer
ImageBufInfoMap::
getSmartBuffer(const DualCamMFBufferID& key)
{
    CAM_LOGW_IF(mvSmartImgBufData.indexOfKey(key) < 0, "%d: key not found", key);
    return (mvSmartImgBufData.indexOfKey(key) >= 0) ? mvSmartImgBufData.valueFor(key) : nullptr;
}

const SmartGraphicBuffer
ImageBufInfoMap::
getGraphicBuffer(const DualCamMFBufferID& key)
{
    CAM_LOGW_IF(mvGraImgBufData.indexOfKey(key) < 0, "%d: key not found", key);
    return (mvGraImgBufData.indexOfKey(key) >= 0) ? mvGraImgBufData.valueFor(key) : nullptr;
}

const sp<mfll::IMfllImageBuffer>
ImageBufInfoMap::
getMfllBuffer(const DualCamMFBufferID& key)
{
    CAM_LOGW_IF(mvMfllImgBufData.count(key) <= 0, "%d: key not found", key);
    return mvMfllImgBufData.count(key)>0? mvMfllImgBufData[key] : nullptr;
}

IMetadata*
ImageBufInfoMap::
getMetadata(const DualCamMFBufferID& key)
{
    CAM_LOGW_IF(mvMetaPtrData.indexOfKey(key) < 0, "%d: key not found", key);
    return (mvMetaPtrData.indexOfKey(key) >= 0) ? mvMetaPtrData.valueFor(key) : nullptr;
}

IMetadata
ImageBufInfoMap::
getSolidMetadata(const DualCamMFBufferID& key)
{
    CAM_LOGW_IF(mvMetaData.indexOfKey(key) < 0, "%d: key not found", key);
    return mvMetaData.valueFor(key);
}

bool
ImageBufInfoMap::
getExtBufName(const DualCamMFBufferID& key, string& rString)
{
    CAM_LOGW_IF(mvExtNameSet.indexOfKey(key) < 0, "%d: key not found", key);

    if(mvExtNameSet.indexOfKey(key) >= 0){
        rString = mvExtNameSet.valueFor(key);
        return true;
    }else{
        return false;
    }
}
/*******************************************************************************
* ScopeLogger
********************************************************************************/
ScopeLogger::
ScopeLogger(const char* text1, const char* text2, int idx)
  : mText1(text1)
  , mText2(text2)
  , mIdx(idx)
{
    if(mText2 == nullptr){
        CAM_LOGD("[%s] +", mText1);
    }else{
        if(mIdx == -1){
            CAM_LOGD("[%s][%s] +", mText1, mText2);
        }else{
            CAM_LOGD("[%s][%s]idx:[%d] +", mText1, mText2, mIdx);
        }
    }
}

ScopeLogger::
~ScopeLogger()
{
    if(mText2 == nullptr){
        CAM_LOGD("[%s] -", mText1);
    }else{
        if(mIdx == -1){
            CAM_LOGD("[%s][%s] -", mText1, mText2);
        }else{
            CAM_LOGD("[%s][%s]idx:[%d] -", mText1, mText2, mIdx);
        }
    }
}
/*******************************************************************************
* SimpleTimer
********************************************************************************/
SimpleTimer::
SimpleTimer()
{}

SimpleTimer::
SimpleTimer(bool bStartTimer)
{
    if(bStartTimer)
        start = std::chrono::system_clock::now();
}

MBOOL
SimpleTimer::
startTimer()
{
    start = std::chrono::system_clock::now();
    return MTRUE;
}

float
SimpleTimer::
countTimer()
{
    std::chrono::time_point<std::chrono::system_clock> cur = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = cur-start;
    return elapsed_seconds.count()*1000;
}

/*******************************************************************************
* StopWatchCollection
********************************************************************************/
android::sp<IStopWatch>
StopWatchCollection::
GetStopWatch(eStopWatchType type, const std::string name)
{
    android::sp<IStopWatch> ret = nullptr;
    android::sp<ProfileUtil> pProfileUtil = new ProfileUtil("StopWatchCollection", name.c_str());

    {
        std::lock_guard<std::mutex> guard(mMutex);

        switch(type)
        {
            case eStopWatchType::eSTOPWATCHTYPE_AUTO:
                mStopWatches.push_back(pProfileUtil);
                ret = new AutoStopWatch(pProfileUtil);
                break;
            case eStopWatchType::eSTOPWATCHTYPE_MANUAL:
                mStopWatches.push_back(pProfileUtil);
                ret = new ManualStopWatch(pProfileUtil);
                break;
            default:
                CAM_LOGE("unknown stop watch type: %d", type);
                break;
        }
    }
    return ret;
}

void
StopWatchCollection
::PrintReport()
{
    if (!mIsEnableLog) return;


    std::lock_guard<std::mutex> guard(mMutex);

    std::ostringstream stringStream;
    stringStream << "************************************" << std::endl;
    stringStream << "Name: " << mName << " Req#: " << mRequestID << std::endl;
    stringStream << "************************************" << std::endl;
    for(auto w : mStopWatches)
    {
        stringStream <<  w->getMessage() << " " << w->getDurationInMillisecond() << " ms" << std::endl;
    }
    stringStream << "************************************";

    __android_log_print(ANDROID_LOG_DEBUG, "STPWR", "%s", stringStream.str().c_str());
}

void
StopWatchCollection
::BeginStopWatch(const std::string name, const void* cookie)
{
    if (!mIsEnableLog) return;


    std::lock_guard<std::mutex> guard(mMutex);

    auto iter = mProfileUtilTable.find(cookie);
    if(iter != mProfileUtilTable.end())
    {
        CAM_LOGE("key exist, name: %s, cookie:0x%d", name.c_str(), cookie);
    }
    else
    {
        android::sp<ProfileUtil> pProfileUtil = new ProfileUtil("StopWatchCollection", name.c_str());
        mProfileUtilTable.insert(pair<const void*, android::sp<ProfileUtil>>(cookie, pProfileUtil));
        pProfileUtil->beginProfile();
    }
}

void
StopWatchCollection
::EndStopWatch(const void* cookie)
{
    if (!mIsEnableLog) return;


    std::lock_guard<std::mutex> guard(mMutex);

    auto iter = mProfileUtilTable.find(cookie);
    if(iter == mProfileUtilTable.end())
    {
        CAM_LOGD("key not found, cookie:0x%d", cookie);
    }
    else
    {
        iter->second->endProfile(false);
        mStopWatches.push_back(iter->second);
        mProfileUtilTable.erase(iter);
    }
}

StopWatchCollection
::~StopWatchCollection()
{
    if (!mIsEnableLog) return;


    for(auto&  pair : mProfileUtilTable)
    {
        CAM_LOGE("not end stop watch, name: %s, cookie: 0x%d", pair.second->getMessage().c_str(), pair.first);
    }
}

} //NSDCMF
} //NSFeaturePipe
} //NSCamFeature
} //NSCam