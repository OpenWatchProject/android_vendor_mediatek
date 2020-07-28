#ifndef __FPSPOLICYSERVICE_H__
#define __FPSPOLICYSERVICE_H__

#include <vector>

#include <semaphore.h>

#include <utils/Thread.h>
#include <utils/String8.h>
#include <utils/KeyedVector.h>

#include <linux/dfrc_drv.h>

#include <dfps/dynamicfps_defs.h>

#include "FpsPolicyTracker.h"

namespace android
{
class FpsPolicyService : public Thread
{
public:
    FpsPolicyService();
    ~FpsPolicyService();

    virtual status_t regPolicy(const char* name, size_t size, const dfps_fps_policy_info_t& info, int32_t *receiveFd);

    virtual status_t unregPolicy(uint64_t sequence, bool binderDied = false);

    virtual status_t setFps(const dfps_fps_policy_info_t& info);

    virtual status_t setSingleLayer(int32_t single);

    virtual status_t setNumberDisplay(int32_t number);

    virtual status_t setHwcInfo(const dfps_hwc_info_t& info);

    virtual status_t setInputWindows(const char* name, size_t size, const dfps_simple_input_window_info_t& info);

    virtual status_t regInfo(uint64_t id);

    virtual status_t unregInfo(uint64_t id, bool binderDied = false);

    virtual status_t getPanelInfo(dfps_panel_info_t *info);

    virtual status_t setForegroundInfo(int32_t pid, const char *name, size_t size);

    virtual status_t getFpsRange(dfps_fps_range_t *range);

    virtual status_t setWindowFlag(int32_t flag, int32_t mask);

    virtual status_t enableTracker(int32_t enable);

    virtual status_t forbidAdjustingVsync(int32_t forbid);

    virtual status_t dump(uint32_t *outSize, char *outBuffer);

    virtual status_t registerCallback(int32_t descriptor, dfps_callback_data_t callbackData, dfps_function_pointer_t pointer);

    virtual status_t readyToRun();

private:
    class FpsPolicyInfoPack : public RefBase {
    public:
        FpsPolicyInfoPack(const String8& name, const dfps_fps_policy_info_t& info)
            : mName(name)
            , mInfo(info)
        {
        }
        virtual ~FpsPolicyInfoPack()
        {
        }
        String8 mName;
        dfps_fps_policy_info_t mInfo;
    };

    class InputWindowPack : public RefBase {
    public:
        InputWindowPack(const String8& name, const dfps_simple_input_window_info_t& info)
            : mName(name)
            , mInfo(info)
        {
        }
        virtual ~InputWindowPack() {};
        String8 mName;
        dfps_simple_input_window_info_t mInfo;
    };

    class FpsInfoPack : public RefBase {
    public:
        FpsInfoPack(const uint64_t id)
            : mId(id)
        {
        }
        uint64_t mId;
    };

    void tryGetRequestLocked();

    void printListPolicyInfoLocked(String8& result);

    void printListInfoLocked(String8& result);

    void printListFpsRangeLocked(String8& result);

    void printSystemInfoLocked(String8& result);

    virtual bool threadLoop();

    void dumpVsyncRequestLocked(const DFRC_DRV_VSYNC_REQUEST& request,
                                const DFRC_DRC_REQUEST_SET& requestSet);

    mutable Mutex mInfoLock;
    mutable KeyedVector<uint64_t, sp<FpsPolicyInfoPack> > mList;
    mutable KeyedVector<uint64_t, sp<FpsInfoPack> > mCbList;

    Condition mCondition;
    bool mStop;
    uint64_t mCount;

    int32_t mSingleLayer;
    int32_t mNumDisplay;

    String8 mForceInputWindowName;
    dfps_simple_input_window_info_t mForceInputWindowInfo;

    int32_t mDfrcFd;

    DFRC_DRV_VSYNC_REQUEST mRequest;
    DFRC_DRV_PANEL_INFO mPanelInfo;
    std::vector<DFRC_DRV_REFRESH_RANGE> mRange;

    int32_t mFgPid;
    String8 mFgPackageName;

    int32_t mWindowFlag;

    bool mNeedInitTracker;
    sp<FpsPolicyTracker> mTracker;

    int32_t mForbidVsync;

    DFPS_PFN_SW_VSYNC_CHANGE mCallbackSwVsyncChange;
    dfps_callback_data_t mCallbackDataSwVsyncChange;

    String8 mDumpString;
};

};

#endif
