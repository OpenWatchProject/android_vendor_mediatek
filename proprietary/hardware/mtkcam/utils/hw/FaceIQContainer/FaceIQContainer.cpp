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
 * MediaTek Inc. (C) 2019. All rights reserved.
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

#define LOG_TAG "MtkCam/HwUtils/FaceIQContainer"

#include <FaceIQContainer/FaceIQContainer.h>


// MTKCAM
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/utils/std/StlUtils.h> // NSCam::SpinLock
#include <mtkcam/utils/hw/FleetingQueue.h>
#include <mtkcam/def/common.h> //MBOOL
#include <cutils/compiler.h> //CC_LIKELY, CC_UNLIKELY

// STL
#include <condition_variable>
#include <unordered_set>
#include <vector>
#include <map>
#include <string>

#ifndef TIME_LIMIT_QUEUE_SIZE
#define TIME_LIMIT_QUEUE_SIZE           18
#endif
#ifndef TIME_LIMIT_QUEUE_BACKUP_SIZE
#define TIME_LIMIT_QUEUE_BACKUP_SIZE    2
#endif

// at least one block for query.
#if (TIME_LIMIT_QUEUE_SIZE <= 0)
#error "TIME_LIMIT_QUEUE_SIZE must > 0"
#endif

// ensure query operator will not get the writing block.
#if (TIME_LIMIT_QUEUE_BACKUP_SIZE <= 0)
#error "TIME_LIMIT_QUEUE_BACKUP_SIZE must > 0"
#endif


#define CHECK_FLEETINGQUEUE_USAGE_READ(x)     (x & IFaceIQContainer::eFaceIQContainer_Opt_Read)
#define CHECK_FLEETINGQUEUE_USAGE_WRITE(x)    (x & IFaceIQContainer::eFaceIQContainer_Opt_Write)




using namespace NSCam;
using android::sp;
CAM_ULOG_DECLARE_MODULE_ID(MOD_UTILITY);

// ----------------------------------------------------------------------------
// factory
// ----------------------------------------------------------------------------
sp<IFaceIQContainer> IFaceIQContainer::createInstance(char const* userId, IFaceIQContainer::eFaceIQContainer_Opt opt)
{
    return static_cast<IFaceIQContainer*>(new FaceIQContainer(userId, opt));
}


// ----------------------------------------------------------------------------
// FD info queue, singleton, thread-safe
// ----------------------------------------------------------------------------
static NSCam::FleetingQueue<FD_DATATYPE, TIME_LIMIT_QUEUE_SIZE, TIME_LIMIT_QUEUE_BACKUP_SIZE> sFleetingQueue;
class NSCam::FaceIQContainerImp {
// method
public:
    FD_DATATYPE* getInfo(char const* userId, int64_t timestamp, int64_t timeout)
    {
        auto time_cur   = std::chrono::system_clock::now();
        auto time_start = time_cur;
        auto time_end   = time_start + std::chrono::milliseconds(timeout);
        auto status     = std::cv_status::no_timeout;
        auto vecInfos   = sFleetingQueue.getInfo(userId, timestamp, timestamp);

        std::unique_lock<std::mutex> lk;

        while (time_cur >= time_start && time_cur < time_end) {
            CAM_ULOGMD("%s: + wait timestamp(%" PRIi64 ") timeout(%" PRIi64 ")", __FUNCTION__, timestamp, timeout);
            status = sCond.wait_until(lk, time_end);
            CAM_ULOGMD("%s: - wait timestamp(%" PRIi64 ") timeout(%" PRIi64 ")", __FUNCTION__, timestamp, timeout);
            // timeout
            if (status == std::cv_status::timeout)
                break;
            // check again
            vecInfos = sFleetingQueue.getInfo(userId, timestamp, timestamp);
            if (!vecInfos.empty())
                break;

            time_cur = std::chrono::system_clock::now();
        }

        return vecInfos.empty()?nullptr:vecInfos.back();
    }

    MBOOL returnInfo(char const* userId, const std::vector< FD_DATATYPE* >& vecInfos)
    {
        return sFleetingQueue.returnInfo(userId, vecInfos);
    }

    FD_DATATYPE* editInfo(char const* userId, int64_t timestamp)
    {
        return sFleetingQueue.editInfo(userId, timestamp);
    }

    MBOOL publishInfo(char const* userId, FD_DATATYPE* info)
    {
        MBOOL ret = sFleetingQueue.publishInfo(userId, info);
        if (ret)
            sCond.notify_all();
        return ret;
    }

    void dumpInfo()
    {
        sFleetingQueue.dumpInfo();
    }

public:
    static std::shared_ptr<FaceIQContainerImp> getInstance()
    {
        std::lock_guard<std::mutex> __l(sLock);
        // create instance
        std::shared_ptr<FaceIQContainerImp> inst = sInstance.lock();
        if (inst.get() == nullptr) {
            inst = std::shared_ptr<FaceIQContainerImp>(new FaceIQContainerImp);
            sInstance = inst;
        }
        return inst;
    }

    static bool hasInstance()
    {
        std::lock_guard<std::mutex> __l(sLock);

        return sInstance.use_count();
    }

public:
    FaceIQContainerImp()
    {

    }

    virtual ~FaceIQContainerImp()
    {

    }

private:
    // singleton of weak pointer implementation
    static std::mutex                           sLock;
    static std::condition_variable              sCond;
    static std::weak_ptr<FaceIQContainerImp>    sInstance;

};
std::mutex                                      FaceIQContainerImp::sLock;
std::condition_variable                         FaceIQContainerImp::sCond;
std::weak_ptr<FaceIQContainerImp>               FaceIQContainerImp::sInstance;


// ----------------------------------------------------------------------------
// FaceIQContainer
// ----------------------------------------------------------------------------
FD_DATATYPE* FaceIQContainer::queryLock(int64_t timestamp, int64_t timeout)
{
    if (CHECK_FLEETINGQUEUE_USAGE_READ(mOpt)) {
        if (mFleepingQueueImpl.get())
            return mFleepingQueueImpl->getInfo(mUserId, timestamp, timeout);

        CAM_ULOGME("FaceIQContainer instance is NULL");
    }

    CAM_ULOGME("Not allow to query FaceIQContainer");
    return nullptr;
}

MBOOL FaceIQContainer::queryUnlock(FD_DATATYPE* info)
{
    std::vector< FD_DATATYPE* > vecInfos;

    vecInfos.clear();
    vecInfos.push_back(info);

    if (CHECK_FLEETINGQUEUE_USAGE_READ(mOpt)) {
        if (mFleepingQueueImpl.get())
            return mFleepingQueueImpl->returnInfo(mUserId, vecInfos);

        CAM_ULOGME("FaceIQContainer instance is NULL");
    }

    CAM_ULOGME("Not allow to query FaceIQContainer");
    return MFALSE;
}

FD_DATATYPE* FaceIQContainer::editLock(int64_t timestamp)
{
    if (CHECK_FLEETINGQUEUE_USAGE_WRITE(mOpt)) {
        if (mFleepingQueueImpl.get())
            return mFleepingQueueImpl->editInfo(mUserId, timestamp);

        CAM_ULOGME("FaceIQContainer instance is NULL");
    }

    CAM_ULOGME("Not allow to edit FaceIQContainer");
    return nullptr;
}

MBOOL FaceIQContainer::editUnlock(FD_DATATYPE* info)
{
    if (CHECK_FLEETINGQUEUE_USAGE_WRITE(mOpt)) {
        if (mFleepingQueueImpl.get())
            return mFleepingQueueImpl->publishInfo(mUserId, info);

        CAM_ULOGME("FaceIQContainer instance is NULL");
    }

    CAM_ULOGME("Not allow to edit FaceIQContainer");
    return MFALSE;
}

void FaceIQContainer::dumpInfo()
{
    if (mFleepingQueueImpl.get())
        mFleepingQueueImpl->dumpInfo();
}


// ----------------------------------------------------------------------------
// constructor
// ----------------------------------------------------------------------------
FaceIQContainer::FaceIQContainer(char const* userId, IFaceIQContainer::eFaceIQContainer_Opt opt)
    : mUserId(userId)
    , mOpt(opt)
{
    mFleepingQueueImpl = FaceIQContainerImp::getInstance();
}

FaceIQContainer::~FaceIQContainer()
{
    mFleepingQueueImpl = nullptr;
}
