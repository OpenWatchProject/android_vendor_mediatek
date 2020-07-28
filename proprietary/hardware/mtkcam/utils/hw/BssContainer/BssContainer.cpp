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

#define LOG_TAG "MtkCam/HwUtils/BssContainer"

#include <BssContainer/BssContainer.h>


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
#define TIME_LIMIT_QUEUE_SIZE           3
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


#define CHECK_FLEETINGQUEUE_USAGE_READ(x)     (x & IBssContainer::eBssContainer_Opt_Read)
#define CHECK_FLEETINGQUEUE_USAGE_WRITE(x)    (x & IBssContainer::eBssContainer_Opt_Write)




using namespace NSCam;
using android::sp;
CAM_ULOG_DECLARE_MODULE_ID(MOD_UTILITY);

// ----------------------------------------------------------------------------
// factory
// ----------------------------------------------------------------------------
sp<IBssContainer> IBssContainer::createInstance(char const* userId, IBssContainer::eBssContainer_Opt opt)
{
    return static_cast<IBssContainer*>(new BssContainer(userId, opt));
}


// ----------------------------------------------------------------------------
// BSS info queue, singleton, thread-safe
// ----------------------------------------------------------------------------
static NSCam::FleetingQueue<BSS_DATATYPE, TIME_LIMIT_QUEUE_SIZE, TIME_LIMIT_QUEUE_BACKUP_SIZE> sFleetingQueue;
class NSCam::BssContainerImp {
// method
public:
    BSS_DATATYPE* getInfo(char const* userId, int64_t uniqueKey, int64_t timeout)
    {
        auto time_cur   = std::chrono::system_clock::now();
        auto time_start = time_cur;
        auto time_end   = time_start + std::chrono::milliseconds(timeout);
        auto status     = std::cv_status::no_timeout;
        auto vecInfos   = sFleetingQueue.getInfo(userId, uniqueKey, uniqueKey);

        std::unique_lock<std::mutex> lk;

        while (time_cur >= time_start && time_cur < time_end) {
            CAM_ULOGMD("%s: + wait uniqueKey(%" PRIi64 ") timeout(%" PRIi64 ")", __FUNCTION__, uniqueKey, timeout);
            status = sCond.wait_until(lk, time_end);
            CAM_ULOGMD("%s: - wait uniqueKey(%" PRIi64 ") timeout(%" PRIi64 ")", __FUNCTION__, uniqueKey, timeout);
            // timeout
            if (status == std::cv_status::timeout)
                break;
            // check again
            vecInfos = sFleetingQueue.getInfo(userId, uniqueKey, uniqueKey);
            if (!vecInfos.empty())
                break;

            time_cur = std::chrono::system_clock::now();
        }

        return vecInfos.empty()?nullptr:vecInfos.back();
    }

    MBOOL returnInfo(char const* userId, const std::vector< BSS_DATATYPE* >& vecInfos)
    {
        return sFleetingQueue.returnInfo(userId, vecInfos);
    }

    BSS_DATATYPE* editInfo(char const* userId, int64_t uniqueKey)
    {
        return sFleetingQueue.editInfo(userId, uniqueKey);
    }

    MBOOL publishInfo(char const* userId, BSS_DATATYPE* info)
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
    static std::shared_ptr<BssContainerImp> getInstance()
    {
        std::lock_guard<std::mutex> __l(sLock);
        // create instance
        std::shared_ptr<BssContainerImp> inst = sInstance.lock();
        if (inst.get() == nullptr) {
            inst = std::shared_ptr<BssContainerImp>(new BssContainerImp);
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
    BssContainerImp()
    {

    }

    virtual ~BssContainerImp()
    {

    }

private:
    // singleton of weak pointer implementation
    static std::mutex                           sLock;
    static std::condition_variable              sCond;
    static std::weak_ptr<BssContainerImp>    sInstance;

};
std::mutex                                      BssContainerImp::sLock;
std::condition_variable                         BssContainerImp::sCond;
std::weak_ptr<BssContainerImp>                  BssContainerImp::sInstance;


// ----------------------------------------------------------------------------
// BssContainer
// ----------------------------------------------------------------------------
BSS_DATATYPE* BssContainer::queryLock(int64_t uniqueKey, int64_t timeout)
{
    if (CHECK_FLEETINGQUEUE_USAGE_READ(mOpt)) {
        if (mFleepingQueueImpl.get())
            return mFleepingQueueImpl->getInfo(mUserId, uniqueKey, timeout);

        CAM_ULOGME("BssContainer instance is NULL");
    }

    CAM_ULOGME("Not allow to query BssContainer");
    return nullptr;
}

MBOOL BssContainer::queryUnlock(BSS_DATATYPE* info)
{
    std::vector< BSS_DATATYPE* > vecInfos;

    vecInfos.clear();
    vecInfos.push_back(info);

    if (CHECK_FLEETINGQUEUE_USAGE_READ(mOpt)) {
        if (mFleepingQueueImpl.get())
            return mFleepingQueueImpl->returnInfo(mUserId, vecInfos);

        CAM_ULOGME("BssContainer instance is NULL");
    }

    CAM_ULOGME("Not allow to query BssContainer");
    return MFALSE;
}

BSS_DATATYPE* BssContainer::editLock(int64_t uniqueKey)
{
    if (CHECK_FLEETINGQUEUE_USAGE_WRITE(mOpt)) {
        if (mFleepingQueueImpl.get())
            return mFleepingQueueImpl->editInfo(mUserId, uniqueKey);

        CAM_ULOGME("BssContainer instance is NULL");
    }

    CAM_ULOGME("Not allow to edit BssContainer");
    return nullptr;
}

MBOOL BssContainer::editUnlock(BSS_DATATYPE* info)
{
    if (CHECK_FLEETINGQUEUE_USAGE_WRITE(mOpt)) {
        if (mFleepingQueueImpl.get())
            return mFleepingQueueImpl->publishInfo(mUserId, info);

        CAM_ULOGME("BssContainer instance is NULL");
    }

    CAM_ULOGME("Not allow to edit BssContainer");
    return MFALSE;
}

void BssContainer::dumpInfo()
{
    if (mFleepingQueueImpl.get())
        mFleepingQueueImpl->dumpInfo();
}


// ----------------------------------------------------------------------------
// constructor
// ----------------------------------------------------------------------------
BssContainer::BssContainer(char const* userId, IBssContainer::eBssContainer_Opt opt)
    : mUserId(userId)
    , mOpt(opt)
{
    mFleepingQueueImpl = BssContainerImp::getInstance();
}

BssContainer::~BssContainer()
{
    mFleepingQueueImpl = nullptr;
}
