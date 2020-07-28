#define LOG_TAG "SWWDT-Test"

#include <inttypes.h>
#include <utils/Log.h>
#include <utils/String8.h>
#include <utils/threads.h>
#include <utils/Timers.h>

#include "ui_ext/SWWatchDog.h"

using namespace android;

class Task : public Thread {
public:
    explicit Task(SWWatchDog::msecs_t wdtThreshold = 1000) :
        mWDTThreshold(wdtThreshold) {

        class MyNotify : public SWWatchDog::Recipient {
            void onSetAnchor(const SWWatchDog::anchor_id_t& id, const pid_t& tid,
                             const nsecs_t& anchorTime, const String8& msg) {
                ALOGI("[SWWDT-Test] Add an anchor(%#" PRIxPTR ") in TID=%4d time=%" PRId64 " <<%s>>",
                    id, tid, anchorTime, msg.string());
            }

            void onDelAnchor(const SWWatchDog::anchor_id_t& id, const pid_t& tid,
                             const nsecs_t& anchorTime, const String8& msg, const nsecs_t& now) {
                ALOGI("[SWWDT-Test] Del an anchor(%#" PRIxPTR ") in TID=%4d SpendTime=%" PRId64 "ms <<%s>>",
                    id, tid, ns2ms(now - anchorTime), msg.string());
            }

            void onTimeout(  const SWWatchDog::anchor_id_t& id, const pid_t& tid,
                             const nsecs_t& anchorTime, const String8& msg, const nsecs_t& now) {
                ALOGW("[SWWDT-Test] Timeout--> id=%#" PRIxPTR " in TID=%4d SpendTime=%" PRId64 "ms <<%s>>",
                    id, tid, ns2ms(now - anchorTime), msg.string());
            }
        };

        mWDT.setWDTNotify(new MyNotify());
        mWDT.setThreshold(mWDTThreshold);
    }
    ~Task() {}

private:
    SWWatchDog::msecs_t mWDTThreshold;
    SWWatchDog mWDT;

    bool threadLoop() {
        // Using default notify, message(__func__:__LINE__), threshold(800)
        AUTO_WDT_DEF;

        foo();
        return false;
    }

    void foo() {
        // Using default notify, message(__func__:__LINE__)
        AUTO_WDT(mWDTThreshold);

        bar();
    }

    void bar() {
        // Using notify, threshold in mWDT, and set a message to send to notify.
        SWWatchDog::AutoWDT _wdt(mWDT, String8::format("WDT monitor sleep in bar():%d", __LINE__));

        sleep(10);
    }
};


int main(int /*argc*/, char** /*argv*/) {
    AUTO_WDT_DEF;

    class TickNotify : public SWWatchDog::Recipient {
        void onTick(const nsecs_t& now) {
            ALOGI("[SWWDT-Test] Tick now = %" PRId64 , now);
        }
    };
    SWWatchDog::setTickNotify(new TickNotify());


    sp<Task> thread[3];

    for (int i = 0; i < 3; ++i) {
        thread[i] = new Task((i+1) * 3000);
        thread[i]->run("SWWDT Test", PRIORITY_NORMAL);
    }

    for (int i = 0; i < 11; ++i) {
        String8 strDump;
        SWWatchDog::dump(strDump);
        printf("%s\n", strDump.string());

        if (i == 3) {
            SWWatchDog::suspend();
        }
        if (i == 8) {
            SWWatchDog::resume();
        }

        sleep(1);
    }

    return 0;
}
