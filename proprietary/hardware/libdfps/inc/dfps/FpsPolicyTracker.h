#ifndef __FPSPOLICYTRACKER_H__
#define __FPSPOLICYTRACKER_H__

#include <utils/String8.h>
#include <utils/Thread.h>

#include <linux/dfrc_drv.h>

namespace android {

class FpsPolicyTracker : public Thread {
public:
    FpsPolicyTracker();
    virtual ~FpsPolicyTracker();

    void stop();
    void setSuspend();
    void setResume();
    void saveRequestInfo(const DFRC_DRV_VSYNC_REQUEST& request,
                         const DFRC_DRC_REQUEST_SET& requestSet);

private:
    virtual bool threadLoop();
    virtual void onFirstRef();
    nsecs_t computeNextDumpTimeLocked();
    void dumpFpsPolicyInfoLocked();

    mutable Mutex mLock;
    Condition mCond;
    bool mStop;
    bool mKeepTracking;
    nsecs_t mPeriod;
    nsecs_t mLastDumpTime;

    DFRC_DRV_VSYNC_REQUEST mRequest;
    DFRC_DRC_REQUEST_SET mRequestSet;
};

};
#endif
