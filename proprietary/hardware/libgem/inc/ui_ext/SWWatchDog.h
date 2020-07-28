#ifndef ANDROID_MTK_WATCHDOG
#define ANDROID_MTK_WATCHDOG

#ifdef FPGA_EARLY_PORTING
#define DISABLE_SWWDT   1
#endif

#include <unistd.h>
#include <utils/threads.h>
#include <utils/String8.h>

#define AUTO_WDT(threshold) \
    SWWatchDog::AutoWDT _wdt(String8::format("%s:%d", __func__, __LINE__), threshold);

#define AUTO_WDT_DEF        \
    SWWatchDog::AutoWDT _wdt(String8::format("%s:%d", __func__, __LINE__));

namespace android {

class SWWatchDog : public RefBase {
public:
    typedef nsecs_t usecs_t;
    typedef nsecs_t msecs_t;
    typedef uintptr_t anchor_id_t;
    static const anchor_id_t NO_ANCHOR;
    static const msecs_t DEFAULT_TIMER;
    static const msecs_t DEFAULT_THRESHOLD;

    /**
     * c'tor of SWWatchDog with default threshold.
     * @param  threshold
     */
    explicit SWWatchDog(const msecs_t& threshold = DEFAULT_THRESHOLD);
    virtual ~SWWatchDog();

    /**
     * Dump all of the anchors' information in current process.
     * @param result Dump data
     */
    static void dump(String8& result);

    /**
     * Suspend/Resume the SWWatchDog, it will effect all of the anchors in this process.
     */
    static void suspend();
    static void resume();

    /**
     * Set/Delete an anchor in the Watchdog.
     * If the anchor live longer than threshold, the Watchdog will call mNotify->onTimeout().
     * The anchor could be only set/deleted by the same thread.
     * @param   msg         Some message want to pass to onTimeout().
     * @param   threshold   The lifetime threshold for current anchor.
     * @        anchor_id_t The unique ID of anchor, or 0 = fail.
     * @return  bool        Delete anchor Success or Fail.
     */
    anchor_id_t setAnchor(const String8& msg, const msecs_t& threshold = -1);
    bool delAnchor(const anchor_id_t& id);

    /**
     * Auto monitor an scope life time with SWWatchDog settings and notifications.
     */
    class AutoWDT {
    public:
        inline AutoWDT(const String8& msg, const msecs_t& threshold = DEFAULT_THRESHOLD) :
            mWDT(DEFAULT_WDT) {
            mID = mWDT.setAnchor(msg, threshold);
        }
        inline AutoWDT(SWWatchDog& wdt, const String8& msg) : mWDT(wdt)  { mID = mWDT.setAnchor(msg); }
        inline AutoWDT(SWWatchDog* wdt, const String8& msg) : mWDT(*wdt) { mID = mWDT.setAnchor(msg); }
        inline ~AutoWDT() { mWDT.delAnchor(mID); }
    private:
        SWWatchDog& mWDT;
        anchor_id_t mID;
    };

    /**
     * Inherite SWWatchDog::Recipient can receive the anchor notify from SWWatchDog.
     * NOTE: Be careful!! DO NOT set/delete any anchors in the onTimeout() callback function.
     * It will cause a DEADLOCK in SWWatchDog.
     */
    class Recipient : public LightRefBase<Recipient> {
    public:
        Recipient() {}
        virtual ~Recipient() {}
        virtual void onSetAnchor(const anchor_id_t& /*id*/, const pid_t& /*tid*/,
                                 const nsecs_t& /*anchorTime*/, const String8& /*msg*/) {}
        virtual void onDelAnchor(const anchor_id_t& /*id*/, const pid_t& /*tid*/,
                                 const nsecs_t& /*anchorTime*/, const String8& /*msg*/,
                                 const nsecs_t& /*now*/) {}
        virtual void onTimeout(  const anchor_id_t& /*id*/, const pid_t& /*tid*/,
                                 const nsecs_t& /*anchorTime*/, const String8& /*msg*/,
                                 const nsecs_t& /*now*/);
        virtual void onTick(const nsecs_t& /*now*/) {}
    };

    /**
     * Set WatchDog onTick notifier for current process.
     * @param notify The recipient of onTick()
     * @return Success or Fail
     */
    static bool setTickNotify(const sp<Recipient>& notify);

    /**
     * Set the notifier for this WatchDog in the future.
     * It will NOT effect the anchor which already set.
     * @param notify The recipient of WatchDog
     * @return Success or Fail
     */
    bool setWDTNotify(const sp<Recipient>& notify);

    /**
     * Set the SW WatchDog detect period, it will effect all of the anchors in current process.
     * The better way is set the minimum timer to serve each anchor.
     * @param timer WDT detect period, unit: ms
     */
    static void setTimer(const msecs_t& timer);

    /**
     * Set/Get the WatchDog timeout threshold.
     * It will only effect the anchor which create by this SWWatchDog in the future.
     * @param threshold The anchor timeout threshold, unit: ms
     */
    void setThreshold(const msecs_t& threshold);
    msecs_t getThreshold() const;

private:
    friend class AutoWDT;
    static SWWatchDog DEFAULT_WDT;
    nsecs_t mThreshold;
    sp<Recipient> mNotify;
    mutable Mutex mDataLock;
};

};  // namedpace android

#endif  //ANDROID_MTK_WATCHDOG
