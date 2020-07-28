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

#define LOG_TAG "MtkCam/HwUtils/FVContainer"

#include <FVContainer/FVContainer.h>


// MTKCAM
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/utils/std/StlUtils.h> // NSCam::SpinLock
#include <mtkcam/utils/hw/FleetingQueue.h>
#include <mtkcam/def/common.h> //MBOOL
#include <cutils/compiler.h> //CC_LIKELY, CC_UNLIKELY

// STL
#include <unordered_set>
#include <vector>
#include <map>
#include <string>

#ifndef TIME_LIMIT_QUEUE_SIZE
#define TIME_LIMIT_QUEUE_SIZE           26
#endif
#ifndef TIME_LIMIT_QUEUE_BACKUP_SIZE
#define TIME_LIMIT_QUEUE_BACKUP_SIZE    4
#endif

// at least one block for query.
#if (TIME_LIMIT_QUEUE_SIZE <= 0)
#error "TIME_LIMIT_QUEUE_SIZE must > 0"
#endif

// ensure query operator will not get the writing block.
#if (TIME_LIMIT_QUEUE_BACKUP_SIZE <= 0)
#error "TIME_LIMIT_QUEUE_BACKUP_SIZE must > 0"
#endif


#define CHECK_FLEETINGQUEUE_USAGE_READ(x)     (x & IFVContainer::eFVContainer_Opt_Read)
#define CHECK_FLEETINGQUEUE_USAGE_WRITE(x)    (x & IFVContainer::eFVContainer_Opt_Write)




using namespace NSCam;
using android::sp;
CAM_ULOG_DECLARE_MODULE_ID(MOD_UTILITY);

// ----------------------------------------------------------------------------
// factory
// ----------------------------------------------------------------------------
sp<IFVContainer> IFVContainer::createInstance(char const* userId, IFVContainer::eFVContainer_Opt opt)
{
    return static_cast<IFVContainer*>(new FVContainer(userId, opt));
}


// ----------------------------------------------------------------------------
// Focus value queue, singleton, thread-safe
// ----------------------------------------------------------------------------
static NSCam::FleetingQueue<FV_DATATYPE, TIME_LIMIT_QUEUE_SIZE, TIME_LIMIT_QUEUE_BACKUP_SIZE> sFleetingQueue;
class NSCam::FVContainerImp {
// method
public:
        std::vector< FV_DATATYPE* > getInfo(char const* userId)
        {
            return sFleetingQueue.getInfo(userId);
        }

        std::vector< FV_DATATYPE* > getInfo(char const* userId, const int64_t& ts_start, const int64_t& ts_end)
        {
            return sFleetingQueue.getInfo(userId, ts_start, ts_end);
        }

        std::vector< FV_DATATYPE* > getInfo(char const* userId, const std::vector<int64_t>& vecTss)
        {
            return sFleetingQueue.getInfo(userId, vecTss);
        }

        MBOOL returnInfo(char const* userId, const std::vector< FV_DATATYPE* >& vecInfos)
        {

            return sFleetingQueue.returnInfo(userId, vecInfos);
        }

        FV_DATATYPE* editInfo(char const* userId, int64_t timestamp)
        {
            return sFleetingQueue.editInfo(userId, timestamp);
        }

        MBOOL publishInfo(char const* userId, FV_DATATYPE* info)
        {
            return sFleetingQueue.publishInfo(userId, info);
        }

        void clear()
        {
            sFleetingQueue.clear();
        }

        void dumpInfo()
        {
            sFleetingQueue.dumpInfo();
        }

public:
    static std::shared_ptr<FVContainerImp> getInstance()
    {
        std::lock_guard<std::mutex> __l(sLock);
        // create instance
        std::shared_ptr<FVContainerImp> inst = sInstance.lock();
        if (inst.get() == nullptr) {
            inst = std::shared_ptr<FVContainerImp>(new FVContainerImp);
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
        FVContainerImp()
        {

        }

        virtual ~FVContainerImp()
        {

        }

private:
        // singleton of weak pointer implementation
        static std::mutex                       sLock;
        static std::weak_ptr<FVContainerImp>    sInstance;

};
std::mutex                                      FVContainerImp::sLock;
std::weak_ptr<FVContainerImp>                   FVContainerImp::sInstance;


// ----------------------------------------------------------------------------
// FVContainer
// ----------------------------------------------------------------------------
vector<FV_DATATYPE> FVContainer::query(void)
{
    vector<FV_DATATYPE> ret;
    ret.clear();

    if (CHECK_FLEETINGQUEUE_USAGE_READ(mOpt)) {
        if (mFleepingQueueImpl.get()) {
            vector<FV_DATATYPE*> result = mFleepingQueueImpl->getInfo(mUserId);
            for (size_t i = 0 ; i < result.size() ; i++) {
                if (CC_LIKELY( result[i] != nullptr ))
                    ret.push_back(*result[i]);
                //else
                    //ret.push_back(FV_DATA_ERROR);
            }
            mFleepingQueueImpl->returnInfo(mUserId, result);
            return ret;
        }

        CAM_ULOGME("FVContainer instance is NULL");
    }

    CAM_ULOGME("Not allow to query FVContainer");
    return ret;
}

vector<FV_DATATYPE> FVContainer::query(const int32_t& mg_start, const int32_t& mg_end)
{
    vector<FV_DATATYPE> ret;
    ret.clear();

    if (CHECK_FLEETINGQUEUE_USAGE_READ(mOpt)) {
        if (mFleepingQueueImpl.get()) {
            vector<FV_DATATYPE*> result = mFleepingQueueImpl->getInfo(mUserId, static_cast<int64_t>(mg_start), static_cast<int64_t>(mg_end));
            for (size_t i = 0 ; i < result.size() ; i++) {
                if (CC_LIKELY( result[i] != nullptr ))
                    ret.push_back(*result[i]);
                //else
                    //ret.push_back(FV_DATA_ERROR);
            }
            mFleepingQueueImpl->returnInfo(mUserId, result);
            return ret;
        }

        CAM_ULOGME("FVContainer instance is NULL");
    }

    CAM_ULOGME("Not allow to query FVContainer");
    return ret;
}

vector<FV_DATATYPE> FVContainer::query(const vector<int32_t>& vecMgs)
{
    vector<FV_DATATYPE> ret;
    ret.clear();

    if (CHECK_FLEETINGQUEUE_USAGE_READ(mOpt)) {
        if (mFleepingQueueImpl.get()) {
            vector<int64_t> vecMgs64;
            vecMgs64.clear();
            for (size_t i = 0 ; i < vecMgs.size() ; i++)
                vecMgs64.push_back(static_cast<int64_t>(vecMgs[i]));

            vector<FV_DATATYPE*> result = mFleepingQueueImpl->getInfo(mUserId, vecMgs64);
            for (size_t i = 0 ; i < result.size() ; i++) {
                if (CC_LIKELY( result[i] != nullptr ))
                    ret.push_back(*result[i]);
                //else
                    //ret.push_back(FV_DATA_ERROR);
            }
            mFleepingQueueImpl->returnInfo(mUserId, result);
            return ret;
        }
        CAM_ULOGME("FVContainer instance is NULL");
    }

    CAM_ULOGME("Not allow to query FVContainer");
    return ret;
}

MBOOL FVContainer::push(int32_t magicNum, FV_DATATYPE fv)
{
    if (CHECK_FLEETINGQUEUE_USAGE_WRITE(mOpt)) {
        if (mFleepingQueueImpl.get()) {
            FV_DATATYPE* editor = mFleepingQueueImpl->editInfo(mUserId, static_cast<int64_t>(magicNum));

            if (CC_LIKELY( editor != nullptr )) {
                *editor = fv;
                mFleepingQueueImpl->publishInfo(mUserId, editor);
                return MTRUE;
            }
            return MFALSE;
        }
        CAM_ULOGME("FVContainer instance is NULL");
    }

    CAM_ULOGME("Not allow to edit FVContainer");
    return MFALSE;
}

void FVContainer::clear()
{
    if (mFleepingQueueImpl.get())
        mFleepingQueueImpl->clear();
}

void FVContainer::dumpInfo()
{
    if (mFleepingQueueImpl.get())
        mFleepingQueueImpl->dumpInfo();
}


// ----------------------------------------------------------------------------
// constructor
// ----------------------------------------------------------------------------
FVContainer::FVContainer(char const* userId, IFVContainer::eFVContainer_Opt opt)
    : mUserId(userId)
    , mOpt(opt)
{
    mFleepingQueueImpl = FVContainerImp::getInstance();
}

FVContainer::~FVContainer()
{
    mFleepingQueueImpl = nullptr;
}
