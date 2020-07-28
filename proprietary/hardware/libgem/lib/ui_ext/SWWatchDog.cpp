#define LOG_TAG "SWWatchDog"

#include <cmath>
#include <inttypes.h>
#include <cutils/log.h>
#include <utils/KeyedVector.h>
#include <utils/Timers.h>

#include "ui_ext/SWWatchDog.h"

namespace android {

const SWWatchDog::anchor_id_t SWWatchDog::NO_ANCHOR     = 0;
const SWWatchDog::msecs_t SWWatchDog::DEFAULT_TIMER     = 1000;
const SWWatchDog::msecs_t SWWatchDog::DEFAULT_THRESHOLD = 800;

//==================================================================================================
// WDTAnchor
//
class WDTAnchor : public LightRefBase<WDTAnchor> {
public:
    WDTAnchor() :
        mAnchorTime(0),
        mThreshold(ms2ns(SWWatchDog::DEFAULT_THRESHOLD)),
        mTid(gettid()) {}

    WDTAnchor(const WDTAnchor& src) {
        mAnchorTime = src.mAnchorTime;
        mThreshold  = src.mThreshold;
        mMsg        = src.mMsg;
        mTid        = src.mTid;
        mNotify     = src.mNotify;
    }

    explicit WDTAnchor(const sp<WDTAnchor>& src) {
        mAnchorTime = src->mAnchorTime;
        mThreshold  = src->mThreshold;
        mMsg        = src->mMsg;
        mTid        = src->mTid;
        mNotify     = src->mNotify;
    }

    ~WDTAnchor() {}

    WDTAnchor& operator= (const WDTAnchor& src) {
        if (&src != this) {
            mAnchorTime = src.mAnchorTime;
            mThreshold  = src.mThreshold;
            mMsg        = src.mMsg;
            mTid        = src.mTid;
            mNotify     = src.mNotify;
        }
        return *this;
    }

    WDTAnchor& operator= (const sp<WDTAnchor>& src) {
        if (src == this) {
            return *this;
        }
        if (src != NULL) {
            mAnchorTime = src->mAnchorTime;
            mThreshold  = src->mThreshold;
            mMsg        = src->mMsg;
            mTid        = src->mTid;
            mNotify     = src->mNotify;
        } else {
            mAnchorTime = 0;
            mThreshold  = ms2ns(SWWatchDog::DEFAULT_THRESHOLD);
            mMsg        = "";
            mTid        = gettid();
            mNotify.clear();
        }
        return *this;
    }

    SWWatchDog::anchor_id_t getID() const {
        return reinterpret_cast<SWWatchDog::anchor_id_t>(this);
    }

    bool isTimeout(nsecs_t now = 0) const {
        if (now == 0) {
            now = systemTime();
        }
        return (mAnchorTime != 0 && mThreshold < now - mAnchorTime);
    }

private:
    friend class SWWatchDogTimer;
    nsecs_t mAnchorTime;
    nsecs_t mThreshold;
    String8 mMsg;
    pid_t mTid;
    sp<SWWatchDog::Recipient> mNotify;
    mutable Mutex mDataLock;
};

//==================================================================================================
// SWWatchDogTimer
//
class SWWatchDogTimer : public Thread {
public:
    SWWatchDogTimer() :
        mAnchorTable(NULL),
        mDuringOnTimeout(false),
        mSuspend(false),
        mTimer(ms2ns(SWWatchDog::DEFAULT_TIMER)),
        mTimeoutTimes(0)
    {}

    ~SWWatchDogTimer() {
        requestExit();
        resume();
    }

    static sp<SWWatchDogTimer> getInstance() {
        Mutex::Autolock _l(sLock);
        static sp<SWWatchDogTimer> instance = sInstance;
        if (instance.get() == 0) {
            instance = new SWWatchDogTimer();
            sInstance = instance;
        }
        return instance;
    }

    void dump(String8& result) const {
        // if during onTimeout, the mTableLock was already locked.
        if (mDuringOnTimeout) {
            dumpLocked(result);
        } else {
            Mutex::Autolock _l(mTableLock);
            dumpLocked(result);
        }
    }

    void suspend() {
        Mutex::Autolock _l(mLock);
        mSuspend = true;
    }

    void resume() {
        Mutex::Autolock _l(mLock);
        mSuspend = false;
        mCondition.signal();
    }

    bool setTickNotify(const sp<SWWatchDog::Recipient>& notify) {
        Mutex::Autolock _l(mTickLock);
        mTickNotify = notify;
        return true;
    }

    SWWatchDog::anchor_id_t setAnchor(const sp<SWWatchDog::Recipient>& notify,
                                      const nsecs_t& threshold, const String8& msg) {

        if (notify == NULL) {
            ALOGE("[SW_WDT] Set an anchor w/o notify. msg=%s", msg.string());
            return SWWatchDog::NO_ANCHOR;
        }

        sp<WDTAnchor> anchor = new WDTAnchor();
        if (anchor == NULL) {
            ALOGE("[SW_WDT] Not enough of memory, could not set anchor! <<%s>>", msg.string());
            return SWWatchDog::NO_ANCHOR;
        }
        SWWatchDog::anchor_id_t id = anchor->getID();

        anchor->mAnchorTime   = systemTime();
        anchor->mThreshold    = threshold;
        anchor->mMsg          = msg;
        anchor->mNotify       = notify;
        notify->onSetAnchor(id, anchor->mTid, anchor->mAnchorTime, anchor->mMsg);

        {
            Mutex::Autolock _l(mTableLock);
            mAnchorTable.add(id, anchor);
        }
        return id;
    }

    bool delAnchor(const SWWatchDog::anchor_id_t& id) {

        sp<WDTAnchor> anchor;
        {
            Mutex::Autolock _l(mTableLock);
            ssize_t index = mAnchorTable.indexOfKey(id);
            if (index == NAME_NOT_FOUND) {
                ALOGE("[SW_WDT] delAnchor: the anchor(%#" PRIxPTR ") is not in the pool", id);
                return false;
            }
            anchor = mAnchorTable.editValueFor(id);
            if (anchor == NULL) {
                ALOGE("[SW_WDT] delAnchor: the anchor(%#" PRIxPTR ") is NULL", id);
                return false;
            }
            if (anchor->mTid != gettid()) {
                ALOGW("[SW_WDT] delAnchor: the anchor(%#" PRIxPTR ") can not been deleted in different thread", id);
                return false;
            }

            mAnchorTable.removeItem(id);
        }

        anchor->mNotify->onDelAnchor(id, anchor->mTid, anchor->mAnchorTime,
                                     anchor->mMsg, systemTime());

        return true;
    }

    void setTimer(const SWWatchDog::msecs_t& timer) {
        Mutex::Autolock _l(mLock);
        mTimer = ms2ns(timer);
    }

private:
    static sp<SWWatchDogTimer> sInstance;
    static Mutex sLock;

    // Should be protected by mTableLock
    DefaultKeyedVector<SWWatchDog::anchor_id_t, sp<WDTAnchor> > mAnchorTable;
    mutable Mutex mTableLock;
    volatile bool mDuringOnTimeout;

    // Should be protected by mTickLock
    sp<SWWatchDog::Recipient> mTickNotify;
    mutable Mutex mTickLock;

    // Should be protected by mLock
    volatile bool mSuspend;
    nsecs_t mTimer;
    mutable Mutex mLock;
    mutable Condition mCondition;
    uint32_t mTimeoutTimes;


    void onFirstRef() {
#ifndef DISABLE_SWWDT
        run("SWWatchDog", PRIORITY_URGENT_DISPLAY);
#endif
    }

    bool threadLoop() {
#ifndef DISABLE_SWWDT
        {
            Mutex::Autolock _l(mLock);
            if (mSuspend) {
                mCondition.wait(mLock);
            }
        }

        if (exitPending()) {
            return false;
        }

        bool isTimeout = false;
        nsecs_t now = systemTime();
        {
            Mutex::Autolock _l(mTableLock);

            // check Watchdog timeout for each thread
            size_t anchorNumber = mAnchorTable.size();
            for (size_t idx = 0; idx < anchorNumber; ++idx) {
                sp<WDTAnchor> anchor = mAnchorTable.valueAt(idx);
                if (anchor == NULL) {
                    ALOGE("[SW_WDT] There is an empty anchor(%zu) in the pool.", idx);
                    continue;
                }

                // notify anchor if timeout
                if (anchor->mNotify != NULL) {
                    if (anchor->isTimeout(now)) {
                        mDuringOnTimeout = true;
                        isTimeout = true;
                        anchor->mNotify->onTimeout(anchor->getID(), anchor->mTid,
                                                   anchor->mAnchorTime, anchor->mMsg, now);
                        mDuringOnTimeout = false;
                    }
                } else {
                    ALOGE("[SW_WDT] There is an anchor(%#" PRIXPTR ") w/o notify.", anchor->getID());
                    continue;
                }
            }
        }

        if (isTimeout) {
            mTimeoutTimes = mTimeoutTimes < 2 ? mTimeoutTimes + 1 : mTimeoutTimes;
        } else {
            mTimeoutTimes = 0;
        }

        // notify onTick if needed.
        {
            Mutex::Autolock _l(mTickLock);
            if (mTickNotify != NULL) {
                mTickNotify->onTick(now);
            }
        }


        // sleep a period time, but wakeup at thread resume or terminate.
        if (!exitPending()) {
            Mutex::Autolock _l(mLock);
            const nsecs_t timer = mTimer * static_cast<int>(pow(2, mTimeoutTimes));
            mCondition.waitRelative(mLock, timer);

            return true;
        }
#endif  // #ifndef DISABLE_SWWDT
        return false;
    }

    void dumpLocked(String8& result) const {
        nsecs_t now = systemTime();
        size_t anchorNumber = mAnchorTable.size();
        {
            Mutex::Autolock _l(mLock);
            result.appendFormat("WDT Anchor Num: %3zu\tNow=%" PRId64 "\tWDT Suspend=%s\n",
                anchorNumber, now, mSuspend ? "Yes" : "No");
        }
        result.append("--------------------------------------------------\n");
        for (size_t idx = 0; idx < anchorNumber; ++idx) {
            sp<WDTAnchor> anchor = mAnchorTable.valueAt(idx);
            result.appendFormat("    [%2zu]  ", idx);
            if (anchor == NULL) {
                result.append(" No anchor\n");
            } else {
                if (anchor->isTimeout(now)){
                    result.append("*");
                } else {
                    result.append(" ");
                }
                result.appendFormat("id=%#" PRIxPTR " TID=%6d\tthreshold=%6" PRId64
                                    "ms\tSpendTime=%6" PRId64 "ms   <<%s>>\n",
                    anchor->getID(), anchor->mTid, ns2ms(anchor->mThreshold),
                    ns2ms(now - anchor->mAnchorTime), anchor->mMsg.string());
            }
        }
        result.append("--------------------------------------------------\n");
    }

};
Mutex SWWatchDogTimer::sLock(Mutex::PRIVATE);
sp<SWWatchDogTimer> SWWatchDogTimer::sInstance(0);

//==================================================================================================
// SWWatchDog
//
SWWatchDog::SWWatchDog(const msecs_t& threshold) :
    mThreshold(ms2ns(threshold)),
    mNotify(new Recipient()) {}

SWWatchDog::~SWWatchDog() {}

void SWWatchDog::dump(String8& result) {
#ifndef DISABLE_SWWDT
    SWWatchDogTimer::getInstance()->dump(result);
#else
    (void)(result);
#endif
}

void SWWatchDog::suspend() {
#ifndef DISABLE_SWWDT
    SWWatchDogTimer::getInstance()->suspend();
#endif
}

void SWWatchDog::resume() {
#ifndef DISABLE_SWWDT
    SWWatchDogTimer::getInstance()->resume();
#endif
}

SWWatchDog::anchor_id_t SWWatchDog::setAnchor(const String8& msg, const msecs_t& threshold) {
#ifndef DISABLE_SWWDT
    Mutex::Autolock _l(mDataLock);
    if (threshold >= 0) {
        return SWWatchDogTimer::getInstance()->setAnchor(mNotify, ms2ns(threshold), msg);
    } else {
        return SWWatchDogTimer::getInstance()->setAnchor(mNotify, mThreshold, msg);
    }
#else
    (void)(msg);
    (void)(threshold);
    return NO_ANCHOR;
#endif
}

bool SWWatchDog::delAnchor(const anchor_id_t& id) {
#ifndef DISABLE_SWWDT
    return SWWatchDogTimer::getInstance()->delAnchor(id);
#else
    (void)(id);
    return false;
#endif
}

bool SWWatchDog::setTickNotify(const sp<Recipient>& notify) {
    return SWWatchDogTimer::getInstance()->setTickNotify(notify);
}

bool SWWatchDog::setWDTNotify(const sp<Recipient>& notify) {
    Mutex::Autolock _l(mDataLock);
    mNotify = notify;
    return true;
}

void SWWatchDog::setTimer(const msecs_t& timer) {
    SWWatchDogTimer::getInstance()->setTimer(timer);
}

void SWWatchDog::setThreshold(const msecs_t& threshold) {
    Mutex::Autolock _l(mDataLock);
    mThreshold = ms2ns(threshold);
}

SWWatchDog::msecs_t SWWatchDog::getThreshold() const {
    Mutex::Autolock _l(mDataLock);
    return ns2ms(mThreshold);
}

SWWatchDog SWWatchDog::DEFAULT_WDT;

//==================================================================================================
// SWWatchDog::Recipient
//
void SWWatchDog::Recipient::onTimeout(const anchor_id_t& id, const pid_t& tid,
                                      const nsecs_t& anchorTime, const String8& msg,
                                      const nsecs_t& now) {
    ALOGW("[SW_WDT] Thread(%d) timeout. id=%#" PRIXPTR " <<%s>> spend %" PRId64 " ms", tid, id,
        msg.string(), ns2ms(now - anchorTime));
}

};  // namespace android
