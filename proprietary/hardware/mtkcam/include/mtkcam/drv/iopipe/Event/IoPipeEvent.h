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

/**
 * @mainpage IoPipe Event System
 * The core philosophy of IoPipe event system is trying to provide a platform-independent
 * event machanism, which archieves following goals:
 * @li Robustness: Thread-safe is basic requirement. It is also capable of compile-time type verification,
 *            automatic unsubscription on subscribers destroying(RAII), and dangling subscription invalidation.
 * @li Flexible: Adaptive to procedure-styled and object-based implementation.
 *            Events are extensible; broadcasting is also controllable.
 * @li Ease of use: One-step subscription & unsubscription, one-step event broadcasting.
 *
 * An event system NSCam::NSIoPipe::IoPipeEventSystem provides subscription and broadcasting operations.
 * Any module can create its own event system. There is also a global event system which is available
 * by NSCam::NSIoPipe::IoPipeEventSystem::getGlobal(). The global event system provides
 * a persist instance. System-level or hardware events can be deliverd via the global system.
 * The global event system provides a layer of platform segregation as well.
 * A sender and a receiver do not need to know each other through the event system. For example,
 * P1Node can subscribes ISP3-only events, but it receives only on ISP3. For ISP4 or higher
 * version, no ISP3-only event will be sent and no ISP3-only procedure will be triggered.
 *
 * Event IDs & event structures are defined in IoPipeEventDef.h. Event IDs are declared in
 * NSCam::NSIoPipe::IoPipeEventId for unified management. Event structures are all subclasses of
 * NSCam::NSIoPipe::IoPipeEvent.
 *
 * @section Usage
 * @code
 #include <mtkcam/drv/iopipe/Event/IoPipeEvent.h>

 using namespace NSCam::NSIoPipe;

 class FakeP1Node : public IoPipeEventSubscriber<IpRawP1ReleasedEvent>
 {
     static IoPipeEventCtrl onP1Acquiring(FakeP1Node *myself, IpRawP1AcquiringEvent &evt) {
         evt.setResult(IoPipeEvent::RESULT_OK);
         return IoPipeEventCtrl::OK;
     }

     static IoPipeEventCtrl onP1StatusChanged(FakeP1Node *myself, IoPipeEvent &evt) {
         return IoPipeEventCtrl::OK;
     }

     static IoPipeEventCtrl onP1Acquiring2(FakeP1Node *myself, IpRawP1AcquiringEvent &evt) {
         return IoPipeEventCtrl::OK;
     }

     IoPipeEventCtrl onIoPipeEvent(IpRawP1ReleasedEvent &evt) override {
         return IoPipeEventCtrl::OK;
     }

     void subscribeExample();
     void unsubscribeExample();

     // If the handle is destructed, the event will be unsubscribed automatically
     // Remember to hold it until really no longer needed
     sp<IoPipeEventHandle> acqHandle1, acqHandle2;
     sp<IoPipeEventHandle> acq2Handle;
     sp<IoPipeEventHandle> relHandle1, relHandle2;
 };

 static IoPipeEventCtrl resetFlowOnP1Acquiring2(Flow *flow, IpRawP1AcquiringEvent &evt)
 {
     flow->reset();
     return IoPipeEventCtrl::OK;
 }

 void FakeP1Node::subscribeExample()
 {
     IoPipeEventSystem &eventSystem = IoPipeEventSystem::getGlobal();

     // Users can use the same function to subscribe different events
     // This form also allows to accept procedure-styled function
     acqHandle1 = eventSystem.subscribe(
         EVT_IPRAW_P1_ACQUIRING, FakeP1Node::onP1Acquiring, this);
     acqHandle2 = eventSystem.subscribe(
         EVT_IPRAW_P1_ACQUIRING, FakeP1Node::onP1StatusChanged, this);
     relHandle1 = eventSystem.subscribe(
         EVT_IPRAW_P1_RELEASED, FakeP1Node::onP1StatusChanged, this);

     // IpRawP1AcquiringEvent is strongly associated to the event ID
     // We don't have to specify the ID for simplity
     acq2Handle = eventSystem.subscribe(FakeP1Node::onP1Acquiring2, this);

     // FakeP1Node implements IoPipeEventSubscriber<IpRawP1ReleasedEvent>
     // and overrides IoPipeEventSubscriber::onIoPipeEvent(), we can use another
     // simple subscription form.
     // Specify event ID to subscribe.
     relHandle2 = eventSystem.subscribe(EVT_IPRAW_P1_RELEASED, *this);

     // While IpRawP1ReleasedEvent is strongly associated to the event ID as well
     // We don't have to specify the event ID.
     // Although the event class can be derived from the template, stronly suggest to
     // describe it obviously. If 2 or more IoPipeEventSubscriber<> are implemented,
     // the event class should also be specified definitely.
     relHandle2 = eventSystem.subscribe<IpRawP1ReleasedEvent>(*this);
 }

 void FakeP1Node::unsubscribeExample()
 {
     // All of below are acceptible

     // invoke the unsubscribe() of handle
     acqHandle->unsubscribe();

     // ask to event system to unsubscribe
     IoPipeEventSystem::getGlobal().unsubscribe(acqHandle2);

     // All handles MUST be unsubscribe obviously, otherwise there will be warnings
     // See IoPipeEventCtrl::~IoPipeEventCtrl() for the reason
     acq2Handle->unsubscribe();
     relHandle1->unsubscribe();
     relHandle2->unsubscribe();
 }

 bool NormalStreamIsp3::acquireP1()
 {
     IoPipeEventSystem &eventSystem = IoPipeEventSystem::getGlobal();

     IpRawP1AcquiringEvent acquiringEvent;
     eventSystem.sendSyncEvent(acquiringEvent);
     if (acquiringEvent.getResult() == IoPipeEvent::RESULT_OK) {
          // continue operations ...
          return true;
     }

     return false;
 }

 void NormalStreamIsp4::convertPureRawToYuv()
 {
     // No P1 resource acquiring
     // Do something directly
 }
 * @endcode
 *
 * @see NSCam::NSIoPipe::IoPipeEventId, NSCam::NSIoPipe::IoPipeEventSystem
 */

#ifndef __IO_PIPE_EVENT_H__
#define __IO_PIPE_EVENT_H__

#include <atomic>
#include <memory>
#include <map>
#include <vector>
#include <queue>
#include <utils/Thread.h>
#include <utils/RefBase.h>
#include <utils/Mutex.h>
#include <utils/Condition.h>

#ifndef LOG_TAG
#define LOG_TAG "IoPipeEvent"
#define __IO_PIPE_EVENT_LOG_TAG__
#endif

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include "IoPipeEventDef.h"


namespace NSCam {
namespace NSIoPipe {


/**
 *Result value of event handlers.
 */
enum class IoPipeEventCtrl
{
    /** Done or ignored, continue broadcasting */
    OK,
    /** Stop event broadcasting. If subscriber intercepts the event.
     * Usually the event is already transferred to correct owner
     * or a valid result is given into IoPipeEvent::setResult() */
    STOP_BROADCASTING
};


class IoPipeEventSystem;


/** All subscribers will get a android::sp<> to the handle.
 * If the handle is released, the event is also unsubscribed automatically.
 *
 * If the associated event system is destroyed, the handle will be invalidate automatically as well.
 * @warning But no behavior can be guaranteed if user destroys system and operates handle simutaneously.
 * Racing conditions are users' resposibility.
 * A event system lives longer than its subscribers are better design. Unsubscribing actively
 * before the system dies is preferred.
 */
class IoPipeEventHandle : virtual public android::RefBase
{
    friend class IoPipeEventSystem;

public:

    /** Will unsubscribe automatically but will print warning if did not unsubscribe before.
     * @warning
     * Do NOT rely on the destructor of sp<> to unsubscribe but explcitly call unsubscribe()
     * at proper position.
     * Because after the user state becomes invalid before the sp<> destructs, the subscription
     * is still valid and the event may be sent to an invaid user instance.
     * Handle is valid does not imply user is also valid.
     * Why not simply ask user as an sp<>? 2 reaons:
     * @li User is not deleted due to the sp<> is present does not imply the system is still in
     *     valid state.
     * @li It restricts the user design. User may be an embedded instance, utility module or
     *     customer module, inherits from android::RefBase is not always proper.
     */
    virtual ~IoPipeEventHandle();

    /** Unsubscribe the event */
    bool unsubscribe();

    /** After the event is unsubscribed, the handle will become invalid. */
    bool isValid() {
        android::Mutex::Autolock lock(mMutex);
        return (mEventSystem != NULL);
    }

    /** No copy constructor, user shall always operate by sp<> */
    IoPipeEventHandle(const IoPipeEventHandle &handle) = delete;

    /** No assignment, user shall always operate by sp<> */
    IoPipeEventHandle& operator=(const IoPipeEventHandle &handle) = delete;

private:
    IoPipeEventHandle(IoPipeEventSystem *eventSystem, IoPipeEventId eventId, unsigned int handleId) :
        mEventSystem(eventSystem), mEventId(eventId), mHandleId(handleId)
    {
    }

    // We assume the life of event system is longer than the handle
    // We are unable to use weak pointer here:
    // 1. An instance shall not hold a smart pointer to itself otherwise it will easily form a cyclic reference.
    // 2. sp<>/wp<> is template and requires the definition of class before declaration
    // However, when the event system destroys, this handle will be invalidated automatically
    // by the system
    IoPipeEventSystem *mEventSystem;
    const IoPipeEventId mEventId;
    const unsigned int mHandleId;
    android::Mutex mMutex;

    struct Admission { };
};


/** Transform static function invocation to member function
 * User must override IoPipeEventSubscriber::onIoPipeEvent() to receive the event.
 * @code
class FakeP1Node : public IoPipeEventSubscriber<IpRawP1ReleasedEvent>
{
private:
    IoPipeEventCtrl onIoPipeEvent(IpRawP1ReleasedEvent &evt) override {
        return IoPipeEventCtrl::OK;
    }
};

// The template parameter IpRawP1ReleasedEvent can be omitted if no overloading function.
// The event ID can be omitted as IpRawP1ReleasedEvent is strongly ID-associated.
// However, do not omit both.
sp<IoPipeEventHandle> relHandle = eventSystem.subscribe<IpRawP1ReleasedEvent>(EVT_IPRAW_P1_RELEASED, p1);
 * @endcode
 */
template <typename _EventT>
class IoPipeEventSubscriber
{
    friend class IoPipeEventSystem;

public:
    virtual ~IoPipeEventSubscriber() { }

private:
    static IoPipeEventCtrl onIoPipeEventStub(IoPipeEventSubscriber *self, _EventT &evt) {
        return self->onIoPipeEvent(evt);
    }

    /** @protected
     * Override me. The event will be sent to here.
     */
    virtual IoPipeEventCtrl onIoPipeEvent(_EventT &evt) = 0;
};


/** The event system of IoPipe. An event system is a broker between event sender & receivers.
 * Every module can have it own event system. For global events or system-level events,
 * IoPipeEventSystem::getGlobal() provides a global event system.
 * A sender and a receiver do not need to know each other through event system. For example,
 * P1Node can subscribes ISP3-only events, but it receives only on ISP3. For ISP4 or higher
  * version, no ISP3-only event will be sent and no ISP3-only procedure will be triggered.
 * This design provides a layer of platform abstraction.
 */
class IoPipeEventSystem final
{
public:

    /** Any user created the event must give a name for debug log */
    explicit IoPipeEventSystem(const char *name);

    ~IoPipeEventSystem();

    /** The most flexible version of subscribe().
     * It can adapted to procedure-styled implementation where
     * handler is a function pointer and user can be pointer of any type or NULL.
     * For object-based implementation, handler can be a static function
     * and the user can be an instance.
     * (handler, user) can not subscribe the same event twice.
     * @param eventId The ID of event to be subscribed
     * @param handler Event handler. The callback function
     * @param user The pointer to be passed into the handler
     * @param cookie will be passed the handler via IoPipeEvent::getCookie().
     *               Cookie can be used as an ID if subscribe to multiple event systems
     * @return  An sp<> to the event handle. If the handle is destructed, the event will be
     *          unsubscribed automatically. Remember to hold it until no longer needed.
     * @warning Do not operate event system or handle in the handler function. Otherwise,
     *          it may cause deadlock or undefined behavior.
     */
    template <typename _EventT, typename _UserT>
    android::sp<IoPipeEventHandle> subscribe(
        IoPipeEventId eventId,
        IoPipeEventCtrl (*handler)(_UserT *user, _EventT &evt),
        _UserT *user,
        IoPipeEvent::CookieType cookie = 0)
    {
        android::Mutex::Autolock lockMap(mSubscrMapMutex);
        android::Mutex::Autolock lockList(getEventMutex(eventId));

        android::sp<IoPipeEventHandle> handle;
        SubscriberList *subscrList = NULL;

        if (!preCheckSubscribe(handle, subscrList, eventId, reinterpret_cast<FuncPtr>(handler), user)) {
            // If already registered, return the handle direcrly
            // If error, handle will still be NULL
            return handle;
        }

        if (subscrList == NULL)
            return NULL;

        unsigned int handleId = getNextHandleId();
        handle = new IoPipeEventHandle(this, eventId, handleId);
        subscrList->emplace_back(
            std::unique_ptr<SubscriberProxy>(
                new SubscriberSpecific<_EventT, _UserT>(handle, handleId, handler, user, cookie))
        );

        if (mDebugFlags & DEBUG_OPERATIONS) {
            CAM_ULOGD(Utils::ULog::MOD_IOPIPE_EVENT, "[IoPipeEventSystem::subscribe][%s] event = %d(#=%zu), handler = %p, user = %p",
                mName, eventId, subscrList->size(), handler, user);
        }

        return handle;
    }

    /**
     * If user inherits from IoPipeEventSubscriber and overrides its onIoPipeEvent(),
     * can use this varity to subscribe.
     * @param eventId The ID of event to be subscribed
     * @param user should overrides IoPipeEventSubscriber::onIoPipeEvent(), and can not subscribe
     *             the same event twice.
     * @param cookie will be passed the handler via IoPipeEvent::getCookie().
     *               Cookie can be used as an ID if subscribe to multiple event systems
     * @return  An sp<> to the event handle. If the handle is destructed, the event will be
     *          unsubscribed automatically. Remember to hold it until no longer needed.
     */
    template <typename _EventT>
    android::sp<IoPipeEventHandle> subscribe(
        IoPipeEventId eventId,
        IoPipeEventSubscriber<_EventT> &user,
        IoPipeEvent::CookieType cookie = 0)
    {
        return subscribe(
                eventId,
                &(IoPipeEventSubscriber<_EventT>::onIoPipeEventStub),
                &user,
                cookie);
    }

    /**
     * If the event is strongly ID-associated(exactly one ID one event type).
     * The event ID is not needed but already implicitly included in the event type.
     * To use this API, _EventT::__EVENT_ID__ must be defined.
     * @see StrictIoPipeEvent
     */
    template <typename _EventT, typename _UserT>
    android::sp<IoPipeEventHandle> subscribe(
        IoPipeEventCtrl (*handler)(_UserT *user, _EventT &evt),
        _UserT *user,
        IoPipeEvent::CookieType cookie = 0)
    {
        // If not strongly ID-associated event, should not use this API
        return subscribe(_EventT::__EVENT_ID__, handler, user, cookie);
    }

    /**
     * The simplest subscribe function for user who implements IoPipeEventSubscriber
     * for strongly ID-associated event.
     * To use this API, _EventT::__EVENT_ID__ must be defined.
     * Recommend to specify the event type in the template parameter for clarity.
     * @see StrictIoPipeEvent
     */
    template <typename _EventT>
    android::sp<IoPipeEventHandle> subscribe(
        IoPipeEventSubscriber<_EventT> &user,
        IoPipeEvent::CookieType cookie = 0)
    {
        // If not strongly ID-associated event, should not use this API
        return subscribe(_EventT::__EVENT_ID__, user, cookie);
    }

    /** Unsubscribe the event */
    bool unsubscribe(android::sp<IoPipeEventHandle> &handle) {
        if (handle->unsubscribe())
            handle = NULL;
        return (handle == NULL);
    }

    /** Send event to subscribers.
     * The function will be blocked until all subscribers are invoked,
     * or somebody returns IoPipeEventCtrl::STOP_BROADCASTING.
     */
    IoPipeEventCtrl sendSyncEvent(IoPipeEvent &evt);

    /** Get the global system of IoPipe event.
     * The global event system is used to transfer system-level event (e.g. hardware).
     * It provides longest life time, than any sub-systems.
     * @warning Senders or subscribers on the global system can be arbitrary.
     * DO NOT use to communicate between 2 specific instances.
     */
    static IoPipeEventSystem &getGlobal() {
        return sGlobalSystem;
    }

    /** Unsubscribe the event by IoPipeEventHandle.
     * Use unsubscribe(android::sp<IoPipeEventHandle> &) instead as user
     * usually holds an sp<> of the handle.
     */
    bool unsubscribe(IoPipeEventHandle &handle, IoPipeEventHandle::Admission &&);

    /** No copy constructor */
    IoPipeEventSystem(const IoPipeEventSystem &) = delete;

    /** No assignment */
    IoPipeEventSystem& operator=(const IoPipeEventSystem &) = delete;

    /** No other pointer type, no NULL */
    IoPipeEventSystem(void *) = delete;

private:

    typedef void (*FuncPtr)();

    class SubscriberProxy {
    public:
        unsigned int mHandleId;
        IoPipeEvent::CookieType mCookie;

        SubscriberProxy(const android::sp<IoPipeEventHandle> &handle, unsigned int handleId, IoPipeEvent::CookieType cookie) :
            mHandleId(handleId), mCookie(cookie), mHandle(handle)
        {
        }

        virtual ~SubscriberProxy() { }

        android::sp<IoPipeEventHandle> getHandle() {
            return mHandle.promote();
        }

        void resetHandle(const android::sp<IoPipeEventHandle> &handle) {
            mHandle = handle;
        }

        virtual IoPipeEventCtrl invoke(IoPipeEvent &evt) = 0;
        virtual bool match(FuncPtr handler, void *user) = 0;

    private:
        android::wp<IoPipeEventHandle> mHandle;
    };

    template <typename _EventT, typename _UserT>
    class SubscriberSpecific : public SubscriberProxy {
    public:
        typedef IoPipeEventCtrl (*HandlerT)(_UserT *user, _EventT &evt);

        virtual IoPipeEventCtrl invoke(IoPipeEvent &evt) override {
            IoPipeEvent::CookieType cookie = evt.getCookie();
            evt.setCookie(mCookie);
            IoPipeEventCtrl ctrl = (*mHandler)(mUser, static_cast<_EventT&>(evt));
            evt.setCookie(cookie); // nobody can see other's cookie

            return ctrl;
        }

        virtual bool match(FuncPtr handler, void *user) override {
            return (reinterpret_cast<FuncPtr>(mHandler) == handler) && (mUser == user);
        }

        SubscriberSpecific(const android::sp<IoPipeEventHandle> &handle, unsigned int handleId,
                HandlerT handler, _UserT *user, IoPipeEvent::CookieType cookie) :
            SubscriberProxy(handle, handleId, cookie), mHandler(handler), mUser(user)
        {
        }

    private:
        HandlerT mHandler;
        _UserT *mUser;
    };

    enum {
        DEBUG_OPERATIONS = 0x1
    };

    static constexpr int MUTEX_BUCKET_SIZE = 2;

    static IoPipeEventSystem sGlobalSystem;
    const char *mName;
    std::atomic_uint mNextHandleId;
    unsigned int mDebugFlags;

    // Lock order: handler mutex -> mSubscrMapMutex -> getEventMutex()
    android::Mutex mSubscrMapMutex;
    android::Mutex mDedicatedMutexBucket[MUTEX_BUCKET_SIZE];
    android::Mutex mSharedMutexBucket[MUTEX_BUCKET_SIZE];
    // std::vector is more efficient since subscribe()/unsubscribe() are
    // rarelly called relatively. And one event only has one or few subscribers.
    typedef std::vector<std::unique_ptr<SubscriberProxy>> SubscriberList;
    std::map<IoPipeEventId, SubscriberList> mSubscribers;

    bool preCheckSubscribe(android::sp<IoPipeEventHandle> &existHandle, SubscriberList *&subscrList,
        IoPipeEventId eventId, FuncPtr handler, void *user);

    SubscriberList *getSubscriberList(IoPipeEventId eventId);

    unsigned int getNextHandleId() {
        return mNextHandleId.fetch_add(1);
    }

    android::Mutex &getEventMutex(IoPipeEventId id) {
        switch (id) {
        case EVT_IPRAW_P1_ACQUIRING:
        case EVT_IPRAW_P1_RELEASED:
            // Long time & complex operation, use dedicated mutex
            return mDedicatedMutexBucket[static_cast<int>(id) % MUTEX_BUCKET_SIZE];
        default:
            break;
        }

        return mSharedMutexBucket[static_cast<int>(id) % MUTEX_BUCKET_SIZE];
    }

    // ----------------- Post machenism -----------------------

private:
    /** NOT TESTED. DO NOT USE
     * After a event posted, the event system owns the ownership and the sender shall not operate
     * it anymore.
     */
    void postAsyncEvent(android::sp<IoPipeEvent> evt);

private:
    class PostThread : public android::Thread {
    public:
        PostThread(IoPipeEventSystem &eventSystem) :
            android::Thread(), mEventSystem(eventSystem)
        {
        }
    private:
        IoPipeEventSystem &mEventSystem;
        virtual bool threadLoop() override;
    };
    friend class PostThread;

    static constexpr int THREAD_POOL_SIZE = 1;

    android::Mutex mThreadPoolMutex;
    android::sp<PostThread> mThreadPool[THREAD_POOL_SIZE];
    android::Mutex mPostMutex;
    android::Condition mPostCond;
    std::queue<android::sp<IoPipeEvent>> mPostEventQueue;
};


}
}

#ifdef __IO_PIPE_EVENT_LOG_TAG__
#undef LOG_TAG
#endif

#endif

