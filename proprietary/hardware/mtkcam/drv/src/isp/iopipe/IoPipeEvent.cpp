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

#define LOG_TAG "IoPipeEvent"

#include <cutils/properties.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/drv/iopipe/Event/IoPipeEvent.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_IOPIPE_EVENT);


using namespace android;

#define EVT_DBG_IF(cond, fmt, arg...) do { if(cond) { CAM_ULOGMD("[%s]" fmt, __func__, ##arg); } } while (0)
#define EVT_WRN(fmt, arg...)  CAM_ULOGMW("[%s]" fmt, __func__, ##arg)
#define EVT_ERR(fmt, arg...)  CAM_ULOGME("[%s]" fmt, __func__, ##arg)


namespace NSCam {
namespace NSIoPipe {



IoPipeEventHandle::~IoPipeEventHandle()
{
    Mutex::Autolock lock(mMutex);

    if (mEventSystem != NULL) {
        EVT_WRN("Did not unsubscribe before handle release! eventId = %d, handleId = %u", mEventId, mHandleId);
        mEventSystem->unsubscribe(*this, Admission());
    }
}


bool IoPipeEventHandle::unsubscribe()
{
    Mutex::Autolock lock(mMutex);
    bool result = false;

    if (mEventSystem != NULL) {
        result = mEventSystem->unsubscribe(*this, Admission());
        mEventSystem = NULL;
    }

    return result;
}


IoPipeEventSystem IoPipeEventSystem::sGlobalSystem("Global");


IoPipeEventSystem::IoPipeEventSystem(const char *name) :
    mName(name), mNextHandleId(1), mDebugFlags(0)
{
    mDebugFlags = property_get_int32("vendor.debug.iopipe.event", 0);
}


IoPipeEventSystem::~IoPipeEventSystem()
{
    std::vector<sp<IoPipeEventHandle>> invalidList;

    {
        android::Mutex::Autolock lockMap(mSubscrMapMutex);

        // Try to invalidate all handles
        for (auto mapIt = mSubscribers.begin(); mapIt != mSubscribers.end(); mapIt++) {
            IoPipeEventId eventId = mapIt->first;
            android::Mutex::Autolock lockList(getEventMutex(eventId));
            for (auto subscrIt = mapIt->second.begin(); subscrIt != mapIt->second.end(); subscrIt++) {
                sp<IoPipeEventHandle> handle = (*subscrIt)->getHandle();
                if (handle != NULL) {
                    // Do NOT invalidate the handle here, notice the lock order
                    invalidList.push_back(handle);
                }
            }
        }
        mSubscribers.clear();
    }

    // Invalidate handles out of mSubscrMapMutex
    for (auto &handle : invalidList) {
        Mutex::Autolock lock(handle->mMutex);
        EVT_WRN("[%s] Handle invalidated automatically: eventId = %d, handleId = %u", mName, handle->mEventId, handle->mHandleId);
        handle->mEventSystem = NULL;
    }
    invalidList.clear();

    Mutex::Autolock lock(mThreadPoolMutex);

    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        if (mThreadPool[i] != NULL)
            mThreadPool[i]->requestExit();
    }

    {
        Mutex::Autolock lock(mPostMutex);
        mPostCond.broadcast();
    }

    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        if (mThreadPool[i] != NULL) {
            mThreadPool[i]->join();
        }
    }
}


bool IoPipeEventSystem::preCheckSubscribe(
    android::sp<IoPipeEventHandle> &existHandle, SubscriberList *&subscrList,
    IoPipeEventId eventId, FuncPtr handler, void *user)
{
    // mSubscrMapMutex & getEventMutex(eventId) MUST already locked

    auto it = mSubscribers.find(eventId);
    if (it == mSubscribers.end()) {
        auto pair = mSubscribers.emplace(eventId, SubscriberList());
        if (pair.first == mSubscribers.end()) {
            EVT_ERR("Unable to build event map. event = %d, handler = %p, user = %p",
                eventId, handler, user);
            subscrList = NULL;
            existHandle = NULL;
            return false; // stop processing
        }

        subscrList = &(pair.first->second);
        existHandle = NULL;

        return true; // new subscription
    }

    subscrList = &(it->second);
    for (auto subscrIt = subscrList->begin(); subscrIt != subscrList->end(); subscrIt++) {
        if ((*subscrIt)->match(handler, user)) {
            existHandle = (*subscrIt)->getHandle();
            if (existHandle == NULL) {
                EVT_ERR("[%s] Handle lost: eventId = %d, handler = %p, user = %p", mName, eventId, handler, user);
                existHandle = new IoPipeEventHandle(this, eventId, (*subscrIt)->mHandleId);
                (*subscrIt)->resetHandle(existHandle);
            } else {
                EVT_WRN("[%s] Double subscription. event = %d, handler = %p, user = %p",
                    mName, eventId, handler, user);
            }
            return false; // handle already exists
        }
    }

    return true;// new subscription
}


IoPipeEventSystem::SubscriberList *IoPipeEventSystem::getSubscriberList(IoPipeEventId eventId)
{
    Mutex::Autolock lock(mSubscrMapMutex);

    SubscriberList *subscrList = NULL;

    auto it = mSubscribers.find(eventId);
    if (it != mSubscribers.end()) {
        subscrList = &(it->second);
    }

    return subscrList;
}


bool IoPipeEventSystem::unsubscribe(IoPipeEventHandle &handle, IoPipeEventHandle::Admission &&)
{
    SubscriberList *subscrList = getSubscriberList(handle.mEventId);
    if (subscrList == NULL) {
        EVT_ERR("[%s] No subscriber list eventId = %d, handleId = %u", mName, handle.mEventId, handle.mHandleId);
        return false;
    }

    Mutex::Autolock lock(getEventMutex(handle.mEventId));
    for (auto it = subscrList->begin(); it != subscrList->end(); it++) {
        if ((*it)->mHandleId == handle.mHandleId) {
            subscrList->erase(it);
            break;
        }
    }

    EVT_DBG_IF(mDebugFlags & DEBUG_OPERATIONS,
        "[%s] Un-subscription success: eventId = %d, handleId = %u", mName, handle.mEventId, handle.mHandleId);

    return true;
}


IoPipeEventCtrl IoPipeEventSystem::sendSyncEvent(IoPipeEvent &evt)
{
    IoPipeEventId eventId = evt.getEventId();

    SubscriberList *subscrList = getSubscriberList(eventId);
    if (subscrList == NULL) {
        EVT_DBG_IF(mDebugFlags & DEBUG_OPERATIONS,
            "[%s] No subscriber. eventId = %d", mName, eventId);
        return IoPipeEventCtrl::OK;
    }

    IoPipeEventCtrl ctrl = IoPipeEventCtrl::OK;
    Mutex::Autolock lock(getEventMutex(eventId));
    for (auto it = subscrList->begin(); it != subscrList->end(); it++) {
        unsigned int handleId = (*it)->mHandleId;
        EVT_DBG_IF(mDebugFlags & DEBUG_OPERATIONS,
            "[%s] invoke START: eventId = %d, handleId = %u", mName, eventId, handleId);

        ctrl = (*it)->invoke(evt);

        EVT_DBG_IF(mDebugFlags & DEBUG_OPERATIONS,
            "[%s] invoke END: eventId = %d, handleId = %u, ctrl = %d",
            mName, eventId, handleId, static_cast<int>(ctrl));

        if (ctrl != IoPipeEventCtrl::OK)
            break;
    }

    return ctrl;
}


void IoPipeEventSystem::postAsyncEvent(sp<IoPipeEvent> evt)
{
    // NOT TESTED YET

    {
        Mutex::Autolock lock(mThreadPoolMutex);
        if (mThreadPool[0] == NULL) {
            for (int i = 0; i < THREAD_POOL_SIZE; i++) {
                mThreadPool[i] = new PostThread(*this);
                mThreadPool[i]->run("IoPipeEventSystem::PostThread");
            }
        }
    }

    Mutex::Autolock lock(mPostMutex);
    mPostEventQueue.push(evt);
    mPostCond.signal();
}


bool IoPipeEventSystem::PostThread::threadLoop()
{
    // NOT TESTED YET

    sp<IoPipeEvent> evt = NULL;

    {
        Mutex::Autolock lock(mEventSystem.mPostMutex);

        while (!exitPending() &&
            mEventSystem.mPostEventQueue.empty())
        {
            mEventSystem.mPostCond.wait(mEventSystem.mPostMutex);
        }

        if (exitPending())
            return false;

        evt = mEventSystem.mPostEventQueue.front();
        mEventSystem.mPostEventQueue.pop();
    }

    IoPipeEventId eventId = evt->getEventId();
    SubscriberList *subscrList = mEventSystem.getSubscriberList(eventId);
    if (subscrList == NULL)
        return true;

    IoPipeEventCtrl ctrl = IoPipeEventCtrl::OK;
    Mutex::Autolock lock(mEventSystem.getEventMutex(eventId));
    for (auto it = subscrList->begin(); it != subscrList->end(); it++) {
        unsigned int handleId = (*it)->mHandleId;
        EVT_DBG_IF(mEventSystem.mDebugFlags & DEBUG_OPERATIONS,
            "[%s] invoke START: eventId = %d, handleId = %u", mEventSystem.mName, eventId, handleId);

        ctrl = (*it)->invoke(*evt);

        EVT_DBG_IF(mEventSystem.mDebugFlags & DEBUG_OPERATIONS,
            "[%s] invoke END: eventId = %d, handleId = %u, ctrl = %d",
            mEventSystem.mName, eventId, handleId, static_cast<int>(ctrl));

        if (ctrl != IoPipeEventCtrl::OK)
            break;
    }

    return true;
}


IoPipeEvent& IoPipeEvent::operator=(const IoPipeEvent &evt)
{
    mEventId = evt.mEventId;
    mCookie = evt.mCookie;
    mResult = evt.mResult;

    return *this;
}


}
}

