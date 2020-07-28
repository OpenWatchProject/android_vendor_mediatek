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

#define LOG_TAG "MtkCam/HwUtils/FDContainer"

#include <FDContainer/FDContainer.h>


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
#define TIME_LIMIT_QUEUE_SIZE           56
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


#define CHECK_FLEETINGQUEUE_USAGE_READ(x)     (x & IFDContainer::eFDContainer_Opt_Read)
#define CHECK_FLEETINGQUEUE_USAGE_WRITE(x)    (x & IFDContainer::eFDContainer_Opt_Write)




using namespace NSCam;
using android::sp;
CAM_ULOG_DECLARE_MODULE_ID(MOD_UTILITY);

// ----------------------------------------------------------------------------
// factory
// ----------------------------------------------------------------------------
sp<IFDContainer> IFDContainer::createInstance(char const* userId, IFDContainer::eFDContainer_Opt opt)
{
    return static_cast<IFDContainer*>(new FDContainer(userId, opt));
}


// ----------------------------------------------------------------------------
// FD info queue, singleton, thread-safe
// ----------------------------------------------------------------------------
#define CAMERA_QUEUE_INDEX (0)
#define ISPHAL_QUEUE_INDEX (1)
#define FLEETING_QUEUE_MAX (2)
static NSCam::FleetingQueue<FD_DATATYPE, TIME_LIMIT_QUEUE_SIZE, TIME_LIMIT_QUEUE_BACKUP_SIZE> sFleetingQueue[FLEETING_QUEUE_MAX];
class NSCam::FDContainerImp {
// method
public:
        FD_DATATYPE* getLatestData(void)
        {
            return sFleetingQueue[mIdx].getLatestData();
        }

        std::vector< FD_DATATYPE* > getInfo(char const* userId)
        {
            return sFleetingQueue[mIdx].getInfo(userId);
        }

        std::vector< FD_DATATYPE* > getInfo(char const* userId, const int64_t& ts_start, const int64_t& ts_end)
        {
            return sFleetingQueue[mIdx].getInfo(userId, ts_start, ts_end);
        }

        std::vector< FD_DATATYPE* > getInfo(char const* userId, const std::vector<int64_t>& vecTss)
        {
            return sFleetingQueue[mIdx].getInfo(userId, vecTss);
        }

        MBOOL returnInfo(char const* userId, const std::vector< FD_DATATYPE* >& vecInfos)
        {

            return sFleetingQueue[mIdx].returnInfo(userId, vecInfos);
        }

        FD_DATATYPE* editInfo(char const* userId, int64_t timestamp)
        {
            return sFleetingQueue[mIdx].editInfo(userId, timestamp);
        }

        MBOOL publishInfo(char const* userId, FD_DATATYPE* info)
        {
            return sFleetingQueue[mIdx].publishInfo(userId, info);
        }

        void dumpInfo()
        {
            sFleetingQueue[mIdx].dumpInfo();
        }

public:
    static std::shared_ptr<FDContainerImp> getInstance(IFDContainer::eFDContainer_Opt opt)
    {
        std::lock_guard<std::mutex> __l(sLock);
        // create instance
        std::weak_ptr<FDContainerImp> &wp_inst = (opt & IFDContainer::eFDContainer_ISP_Opt) ? sIspInstance : sInstance;
        std::shared_ptr<FDContainerImp> inst = wp_inst.lock();
        if (inst.get() == nullptr) {
            inst = std::shared_ptr<FDContainerImp>(new FDContainerImp(opt));
            wp_inst = inst;
        }
        return inst;
    }

    static bool hasInstance()
    {
        std::lock_guard<std::mutex> __l(sLock);

        return sInstance.use_count();
    }

    public:
        FDContainerImp()
        {

        }

        FDContainerImp(IFDContainer::eFDContainer_Opt opt)
        {
            mIdx = (opt & IFDContainer::eFDContainer_ISP_Opt) ? ISPHAL_QUEUE_INDEX : CAMERA_QUEUE_INDEX;
        }

        virtual ~FDContainerImp()
        {

        }

private:
        // singleton of weak pointer implementation
        static std::mutex                       sLock;
        static std::weak_ptr<FDContainerImp>    sInstance;
        static std::weak_ptr<FDContainerImp>    sIspInstance;
        int32_t                                 mIdx = CAMERA_QUEUE_INDEX;

};
std::mutex                                      FDContainerImp::sLock;
std::weak_ptr<FDContainerImp>                   FDContainerImp::sInstance;
std::weak_ptr<FDContainerImp>                   FDContainerImp::sIspInstance;


// ----------------------------------------------------------------------------
// FDContainer
// ----------------------------------------------------------------------------
MBOOL FDContainer::cloneLatestFD(FD_DATATYPE& cloneInfo)
{
    if (CHECK_FLEETINGQUEUE_USAGE_READ(mOpt)) {
        if (mFleepingQueueImpl.get()) {
            FD_DATATYPE* last = mFleepingQueueImpl->getLatestData();
            if (last) {
                last->clone(cloneInfo);
                return MTRUE;
            } else {
                return MFALSE;
            }
        } else {
            CAM_ULOGME("FDContainer instance is NULL");
        }
    }

    CAM_ULOGME("Not allow to query FDContainer");
    return MFALSE;
}

vector<FD_DATATYPE*> FDContainer::queryLock(void)
{
    if (CHECK_FLEETINGQUEUE_USAGE_READ(mOpt) && mFleepingQueueImpl.get())
        return mFleepingQueueImpl->getInfo(mUserId);
    else
        CAM_ULOGME("FDContainer instance is NULL");

    CAM_ULOGME("Not allow to query FDContainer");
    return vector<FD_DATATYPE*>();
}

vector<FD_DATATYPE*> FDContainer::queryLock(const int64_t& ts_start, const int64_t& ts_end)
{
    if (CHECK_FLEETINGQUEUE_USAGE_READ(mOpt) && mFleepingQueueImpl.get())
        return mFleepingQueueImpl->getInfo(mUserId, ts_start, ts_end);
    else
        CAM_ULOGME("FDContainer instance is NULL");

    CAM_ULOGME("Not allow to query FDContainer");
    return vector<FD_DATATYPE*>();
}

vector<FD_DATATYPE*> FDContainer::queryLock(const vector<int64_t>& vecTss)
{
    if (CHECK_FLEETINGQUEUE_USAGE_READ(mOpt)) {
        if (mFleepingQueueImpl.get())
            return mFleepingQueueImpl->getInfo(mUserId, vecTss);

        CAM_ULOGME("FDContainer instance is NULL");
    }

    CAM_ULOGME("Not allow to query FDContainer");
    return vector<FD_DATATYPE*>();
}


MBOOL FDContainer::queryUnlock(const vector<FD_DATATYPE*>& vecInfos)
{
    if (CHECK_FLEETINGQUEUE_USAGE_READ(mOpt)) {
        if (mFleepingQueueImpl.get())
            return mFleepingQueueImpl->returnInfo(mUserId, vecInfos);

        CAM_ULOGME("FDContainer instance is NULL");
    }

    CAM_ULOGME("Not allow to query FDContainer");
    return MFALSE;
}

FD_DATATYPE* FDContainer::editLock(int64_t timestamp)
{
    if (CHECK_FLEETINGQUEUE_USAGE_WRITE(mOpt)) {
        if (mFleepingQueueImpl.get())
            return mFleepingQueueImpl->editInfo(mUserId, timestamp);

        CAM_ULOGME("FDContainer instance is NULL");
    }

    CAM_ULOGME("Not allow to edit FDContainer");
    return nullptr;
}

MBOOL FDContainer::editUnlock(FD_DATATYPE* info)
{
    if (CHECK_FLEETINGQUEUE_USAGE_WRITE(mOpt)) {
        if (mFleepingQueueImpl.get())
            return mFleepingQueueImpl->publishInfo(mUserId, info);

        CAM_ULOGME("FDContainer instance is NULL");
    }

    CAM_ULOGME("Not allow to edit FDContainer");
    return MFALSE;
}

void FDContainer::dumpInfo()
{
    if (mFleepingQueueImpl.get())
        mFleepingQueueImpl->dumpInfo();
}


// ----------------------------------------------------------------------------
// constructor
// ----------------------------------------------------------------------------
FDContainer::FDContainer(char const* userId, IFDContainer::eFDContainer_Opt opt)
    : mUserId(userId)
    , mOpt(opt)
{
    mFleepingQueueImpl = FDContainerImp::getInstance(opt);
}

FDContainer::~FDContainer()
{
    mFleepingQueueImpl = nullptr;
}
