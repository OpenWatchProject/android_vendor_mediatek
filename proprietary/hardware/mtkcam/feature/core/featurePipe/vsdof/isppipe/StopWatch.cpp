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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

 /**
 * @file PipeBufferHandler.cpp
 * @brief Buffer handler for isp pipe
 */

 // Standard C header file
#include <chrono>
#include <list>
#include <map>
#include <mutex>
#include <sstream>
// Android system/core header file
#include <cutils/properties.h>
// mtkcam custom header file

// mtkcam global header file

// Module header file

// Local header file
#include <mtkcam/feature/stereo/pipe/IStopWatch.h>
// Logging header file

#undef PIPE_CLASS_TAG
#define PIPE_MODULE_TAG "IspPipe"
#define PIPE_CLASS_TAG "StopWatch"
#include <featurePipe/core/include/PipeLog.h>

class scoped_tracer
{
public:
    scoped_tracer(const char* functionName)
    : mFunctionName(functionName)
    {
        CAM_LOGD("[%s] +", mFunctionName);
    }
    ~scoped_tracer()
    {
        CAM_LOGD("[%s] -", mFunctionName);
    }
private:
    const char* const mFunctionName;
};
#define SCOPED_TRACER() scoped_tracer ___scoped_tracer(__FUNCTION__ );
/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace DualCamThirdParty {

using android::sp;

/*******************************************************************************
* Class Definition
********************************************************************************/
/**
 * @class Chrono
 * @brief time elapsed measurement class
 */
class Chrono : public IStopWatch
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    Chrono(std::string name);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IStopWatch Public Function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    const std::string&  getName()       const override;
    MVOID               begin()         override;
    MVOID               end()           override;
    MFLOAT              getElapsed()    const override;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// IspPipe Private member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    using Timing        = std::chrono::time_point<std::chrono::high_resolution_clock>;
    using DuationTime   = std::chrono::duration<MFLOAT, std::micro>;

    const std::string   mName;
    DuationTime         mDuration;
    Timing              mBeginTiming;
    Timing              mEndTiming;
};

/**
 * @class StopWatch
 * @brief base(abstract) class of IStopWatch
 */
class StopWatch : public IStopWatch
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IStopWatch Public Function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    const std::string&  getName()       const;
    MVOID               begin()         override;
    MVOID               end()           override;
    MFLOAT              getElapsed()    const override;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StopWatch Potected Function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    StopWatch(const std::string& name);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StopWatch non-virtual interface (NVI)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    inline virtual MVOID   onBegin();
    inline virtual MVOID   onEnd();
    inline virtual MFLOAT  onGetElapsed() const;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// StopWatch Private member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    const std::string mName;
};

/**
 * @class EmptyStopWatch
 * @brief empty implementation of StopWatch
 */
class EmptyStopWatch : public StopWatch
{
public:
    EmptyStopWatch(const std::string& name);
};

/**
 * @class ThirdPartyBokehUtility
 * @brief auto time elapsed measurement from object create to destory
 */
class AutoStopWatch final: public StopWatch
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  AutoStopWatch Public Function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    AutoStopWatch(sp<IStopWatch> realStopWatch, MBOOL printLogWhenDestory = MTRUE);
    ~AutoStopWatch() override;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AutoStopWatch Private member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    sp<IStopWatch> mRealStopWatch;
    const MBOOL mPrintLogWhenDestory;
};

/**
 * @class ManualStopWatch
 * @brief manual time elapsed measurement
 */
class ManualStopWatch final: public StopWatch
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ManualStopWatch Public Function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    ManualStopWatch(sp<IStopWatch> realStopWatch);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StopWatch non-virtual interface (NVI)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MVOID onBegin() override;
    MVOID onEnd() override;
    MFLOAT onGetElapsed() const override;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ManualStopWatch Private member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    sp<IStopWatch> mRealStopWatch;
};

/**
 * @class EmptyStopWatchCollection
 * @brief empty implementation of IStopWatchCollection
 */
class EmptyStopWatchCollection final: public IStopWatchCollection
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  EmptyStopWatchCollection Public Function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    EmptyStopWatchCollection(const std::string& name, MUINT32 requestID);
    ~EmptyStopWatchCollection() override;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IStopWatchCollection Public Function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    const std::string&  getName()                                                           const;
    sp<IStopWatch>      getStopWatch(StopWatchType stopWatchType, const std::string& name)  override;
    MVOID               begin(const std::string& name, MVOID* cookie)                       override;
    MVOID               end(MVOID* cookie)                                                  override;
    MVOID               printResult()                                                       const override;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// EmptyStopWatchCollection Private member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    const std::string mName;
    const MUINT32     mRequestID;
};

/**
 * @class StopWatchCollection
 * @brief implementation of IStopWatchCollection
 */
class StopWatchCollection final: public IStopWatchCollection
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StopWatchCollection Public Function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    StopWatchCollection(const std::string& name, MUINT32 requestID);
    ~StopWatchCollection() override;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IStopWatchCollection Public Function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    const std::string&  getName()                                                           const override;
    sp<IStopWatch>      getStopWatch(StopWatchType stopWatchType, const std::string& name)  override;
    MVOID               begin(const std::string& name, MVOID* cookie)                       override;
    MVOID               end(MVOID* cookie)                                                  override;
    MVOID               printResult()                                                       const override;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// StopWatchCollection Private member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    const std::string mName;
    const MUINT32     mRequestID;

    std::mutex                         mLocker;
    std::list<sp<IStopWatch>>          mRealStopWatches;
    std::map<MVOID*, sp<IStopWatch>>   mRealStopWatchTable;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Chrono Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Chrono::
Chrono(std::string name)
: mName(name)
, mDuration(DuationTime::zero())
{

}

const
std::string&
Chrono::
getName() const
{
    return mName;
}

MVOID
Chrono::
begin()
{
    mBeginTiming = std::chrono::high_resolution_clock::now();
}

MVOID
Chrono::
end()
{
    mEndTiming = std::chrono::high_resolution_clock::now();
    mDuration = mEndTiming - mBeginTiming;
}

MFLOAT
Chrono::
getElapsed() const
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(mDuration).count();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// StopWatch Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
const
std::string&
StopWatch::
getName() const
{
    return mName;
}

MVOID
StopWatch::
begin()
{
    onBegin();
}

MVOID
StopWatch::
end()
{
    onEnd();
}

MFLOAT
StopWatch::
getElapsed() const
{
    return onGetElapsed();
}

StopWatch::
StopWatch(const std::string& name)
: mName(name)
{

}

MVOID
StopWatch::
onBegin()
{

}

MVOID
StopWatch::
onEnd()
{

}

MFLOAT
StopWatch::
onGetElapsed() const
{
    return -1.0f;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// EmptyStopWatch Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
EmptyStopWatch::
EmptyStopWatch(const std::string& name)
: StopWatch(name)
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AutoStopWatch Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AutoStopWatch::
AutoStopWatch(sp<IStopWatch> realStopWatch, MBOOL printLogWhenDestory)
: StopWatch(realStopWatch->getName())
, mRealStopWatch(realStopWatch)
, mPrintLogWhenDestory(printLogWhenDestory)
{
    mRealStopWatch->begin();
}

AutoStopWatch::
~AutoStopWatch()
{
    mRealStopWatch->end();

    if(mPrintLogWhenDestory)
    {
        MY_LOGD("end time measure, name:%s, duration time: %.3f");
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ManualStopWatch Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ManualStopWatch::
ManualStopWatch(sp<IStopWatch> realStopWatch)
: StopWatch(realStopWatch->getName())
, mRealStopWatch(realStopWatch)
{

}

MVOID
ManualStopWatch::
onBegin()
{
    mRealStopWatch->begin();
}

MVOID
ManualStopWatch::
onEnd()
{
    mRealStopWatch->end();
}

MFLOAT
ManualStopWatch::
onGetElapsed() const
{
    return mRealStopWatch->getElapsed();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// EmptyStopWatchCollection Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
EmptyStopWatchCollection::
EmptyStopWatchCollection(const std::string& name, MUINT32 requestID)
: mName(name + "_emptyStopwatch")
, mRequestID(requestID)
{

}

EmptyStopWatchCollection::
~EmptyStopWatchCollection()
{
    // empty
}

const std::string&
EmptyStopWatchCollection::
getName() const
{
    return mName;
}

sp<IStopWatch>
EmptyStopWatchCollection::
getStopWatch(StopWatchType stopWatchType, const std::string& name)
{
    return new EmptyStopWatch(name);
}

MVOID
EmptyStopWatchCollection::
begin(const std::string& name, MVOID* cookie)
{
    // empty
}

MVOID
EmptyStopWatchCollection::
end(MVOID* cookie)
{
    // empty
}

MVOID
EmptyStopWatchCollection::
printResult() const
{
    // empty
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// StopWatchCollection Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
sp<IStopWatchCollection>
IStopWatchCollection::
createInstance(const std::string& name, MUINT32 requestID)
{
    const MBOOL isEnablePerformanceMeasurement = ::property_get_int32("vendor.debug.isp.pipe.perfmea.on", 0);

    sp<IStopWatchCollection> ret = nullptr;
    if(isEnablePerformanceMeasurement)
    {
        ret = new StopWatchCollection(name, requestID);
    }
    else
    {
        ret = new EmptyStopWatchCollection(name, requestID);
    }
    return ret;
}

StopWatchCollection::
StopWatchCollection(const std::string& name, MUINT32 requestID)
: mName(name)
, mRequestID(requestID)
{

}

StopWatchCollection::
~StopWatchCollection()
{
    if(mRealStopWatchTable.size() > 0)
    {
        MY_LOGW("there are some items does not call end, collectionName:%s", getName().c_str());

        for(auto& item : mRealStopWatchTable)
        {
            MY_LOGW("wrong item, cookie:%d, name:%s",item.first, item.second->getName().c_str());
        }
    }
}

const std::string&
StopWatchCollection::
getName() const
{
    return mName;
}

sp<IStopWatch>
StopWatchCollection::
getStopWatch(StopWatchType stopWatchType, const std::string& name)
{
    sp<IStopWatch> realStopWatch = new Chrono(name);
    sp<IStopWatch> ret = nullptr;
    switch(stopWatchType)
    {
        case StopWatchType::Auto:
            ret = new AutoStopWatch(realStopWatch, MFALSE);
            break;
        case StopWatchType::Manual:
            ret = new ManualStopWatch(realStopWatch);
            break;
        default:
            // error handle
            MY_LOGW("unknown stop watch type: %d", static_cast<MINT32>(stopWatchType));
            break;
    }

    if(ret != nullptr)
    {
        std::lock_guard<std::mutex> guard(mLocker);
        mRealStopWatches.push_back(realStopWatch);
    }
    return ret;
}

MVOID
StopWatchCollection
::begin(const std::string& name, MVOID* cookie)
{
    std::lock_guard<std::mutex> guard(mLocker);

    auto iter = mRealStopWatchTable.find(cookie);
    if(iter != mRealStopWatchTable.end())
    {
        MY_LOGW("key is existing, key:%d", cookie);
    }
    else
    {
        sp<Chrono> chrono = new Chrono(name);
        mRealStopWatchTable.insert(std::pair<MVOID*, sp<Chrono>>(cookie, chrono));
        chrono->begin();
    }
}

MVOID
StopWatchCollection
::end(MVOID* cookie)
{
    std::lock_guard<std::mutex> guard(mLocker);

    auto iter = mRealStopWatchTable.find(cookie);
    if(iter != mRealStopWatchTable.end())
    {
        iter->second->end();
        mRealStopWatches.push_back(iter->second);
        mRealStopWatchTable.erase(iter);
    }
    else
    {
        MY_LOGW("key is not found, key:%d", cookie);
    }
}

MVOID
StopWatchCollection
::printResult() const
{
    std::ostringstream stringStream;
    stringStream << "************************************" << std::endl;
    stringStream << "Name: " << mName << " Req#: " << mRequestID << std::endl;
    stringStream << "************************************" << std::endl;
    for(auto item : mRealStopWatches)
    {
        stringStream <<  item->getName() << " " << item->getElapsed() << " ms" << std::endl;
    }
    stringStream << "************************************";

    __android_log_print(ANDROID_LOG_DEBUG, "STWRP", "%s", stringStream.str().c_str());
}


}; // DualCamThirdParty
}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam
