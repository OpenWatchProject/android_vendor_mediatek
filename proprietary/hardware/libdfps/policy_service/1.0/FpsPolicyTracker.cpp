#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include "dfps/FpsPolicyTracker.h"

#include <utils/Trace.h>
#include <utils/Log.h>

#include "dfps/string_def.h"

#define FPT_ATRACE_NAME(name) android::ScopedTrace ___tracer(ATRACE_TAG, name)

#define FPT_ATRACE_BUFFER(x, ...)                                               \
    if (ATRACE_ENABLED()) {                                                     \
        char ___traceBuf[256];                                                  \
        snprintf(___traceBuf, sizeof(___traceBuf), x, ##__VA_ARGS__);           \
        android::ScopedTrace ___bufTracer(ATRACE_TAG, ___traceBuf);             \
    }

namespace android {

FpsPolicyTracker::FpsPolicyTracker()
    : mStop(false)
    , mKeepTracking(false)
    , mPeriod(1000000000)
    , mLastDumpTime(0)
{
    mRequestSet.num = 0;
    mRequestSet.policy = NULL;

    memset(&mRequest, 0, sizeof(mRequest));
}

FpsPolicyTracker::~FpsPolicyTracker()
{
}

void FpsPolicyTracker::stop()
{
    Mutex::Autolock l(mLock);
    mStop = true;
}

void FpsPolicyTracker::setSuspend()
{
    Mutex::Autolock l(mLock);
    mKeepTracking = false;
    mCond.signal();
}

void FpsPolicyTracker::setResume()
{
    Mutex::Autolock l(mLock);
    mKeepTracking = true;
    mCond.signal();
}

bool FpsPolicyTracker::threadLoop()
{
    status_t err = NO_ERROR;

    while (true) {
        FPT_ATRACE_NAME("dump_request_info");
        Mutex::Autolock l(mLock);

        if (mStop) {
            return false;
        }

        if (mKeepTracking) {
            nsecs_t now = systemTime(SYSTEM_TIME_MONOTONIC);
            nsecs_t targetTime = computeNextDumpTimeLocked();

            int needDump = true;
            if (targetTime > now) {
                err = mCond.waitRelative(mLock, targetTime - now);
                if (err != TIMED_OUT) {
                    needDump = false;
                }
            }
            if (needDump) {
                dumpFpsPolicyInfoLocked();
                mLastDumpTime = systemTime(SYSTEM_TIME_MONOTONIC);
            }
        } else {
            mCond.wait(mLock);
        }
    }
}

void FpsPolicyTracker::onFirstRef()
{
}

nsecs_t FpsPolicyTracker::computeNextDumpTimeLocked()
{
    return mLastDumpTime + mPeriod;
}

void FpsPolicyTracker::dumpFpsPolicyInfoLocked()
{
    FPT_ATRACE_BUFFER("fps[%d] mode[%s]  swFps[%d]  swMode[%d]  validInfo[%d]  transState[%d] forbidVsync[%d]",
                      mRequest.fps, getModeString(mRequest.mode),
                      mRequest.sw_fps, mRequest.sw_mode,
                      mRequest.valid_info, mRequest.transient_state, mRequest.forbid_vsync);
    FPT_ATRACE_BUFFER("numPolicy[%d]  numRequestSet[%d]", mRequest.num_policy, mRequestSet.num);
    for (int i = 0; i < mRequestSet.num; i++) {
        DFRC_DRV_POLICY *policy = &mRequestSet.policy[i];
        FPT_ATRACE_BUFFER("seq[%llu]  api[%s]  pid[%d]  fps[%d]  mode[%s]  tPid[%d]  glId[%llu]",
                          policy->sequence, getApiString(policy->api), policy->pid,
                          policy->fps, getModeString(policy->mode),
                          policy->target_pid, policy->gl_context_id);
    }
}

void FpsPolicyTracker::saveRequestInfo(const DFRC_DRV_VSYNC_REQUEST& request,
                                        const DFRC_DRC_REQUEST_SET& requestSet)
{
    Mutex::Autolock l(mLock);
    mRequest = request;
    if (mRequestSet.policy != NULL) {
        delete [] mRequestSet.policy;
        mRequestSet.policy = NULL;
    }
    mRequestSet.num = requestSet.num;
    if (requestSet.num) {
        mRequestSet.policy = new DFRC_DRV_POLICY[requestSet.num];
        for (int i = 0; i < requestSet.num; i++) {
            mRequestSet.policy[i] = requestSet.policy[i];
        }
    }
}

};
