//#define LOG_TAG "FpsPolicy"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include "dfps/FpsPolicyService.h"

#include <dlfcn.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>
#include <inttypes.h>
#include <log/log.h>
#include <cutils/properties.h>
#include <utils/SortedVector.h>

#include <cutils/memory.h>
#include <utils/Trace.h>

#include <vsync_enhance/DispSyncEnhancementDef.h>

#include "dfps/string_def.h"

#if !defined(__STDC_FORMAT_MACROS)
#define __STDC_FORMAT_MACROS 1
#endif

#define FPS_ATRACE_NAME(name) android::ScopedTrace ___tracer(ATRACE_TAG, name)

#define FPS_ATRACE_BUFFER(x, ...)                                               \
    if (ATRACE_ENABLED()) {                                                     \
        char ___traceBuf[256];                                                  \
        snprintf(___traceBuf, sizeof(___traceBuf), x, ##__VA_ARGS__);           \
        android::ScopedTrace ___bufTracer(ATRACE_TAG, ___traceBuf);             \
    }

namespace android {

#define DFRC_NODE_PATH "/dev/mtk_dfrc"

FpsPolicyService::FpsPolicyService()
    : mStop(false)
    , mCount(0)
    , mSingleLayer(0)
    , mNumDisplay(0)
    , mFgPid(0)
    , mWindowFlag(0)
    , mNeedInitTracker(true)
    , mForbidVsync(false)
    , mCallbackSwVsyncChange(NULL)
    , mCallbackDataSwVsyncChange(NULL)
{
    mDfrcFd = open(DFRC_NODE_PATH, O_RDWR, 0);
    if (mDfrcFd < 0) {
        ALOGE("failed to open %s", DFRC_NODE_PATH);
    }
    int res = ioctl(mDfrcFd, DFRC_IOCTL_CMD_RESET_STATE);
    if (res < 0) {
        ALOGE("failed to reset the state of dfrc");
    }
    res = ioctl(mDfrcFd, DFRC_IOCTL_CMD_GET_PANEL_INFO, &mPanelInfo);
    if (res < 0) {
        ALOGE("failed to get panel info from dfrc");
    } else {
        ALOGI("try to new %d refresh ranges", mPanelInfo.num);
        mRange.resize(mPanelInfo.num);
        for (int i = 0; i < mPanelInfo.num; i++) {
            mRange[i].index = i;
            res = ioctl(mDfrcFd, DFRC_IOCTL_CMD_GET_REFRESH_RANGE, &mRange[i]);
            if (res < 0) {
                ALOGE("failed to get refresh range %d: %d", i, res);
                mRange[i].max_fps = 60;
                mRange[i].min_fps = 60;
            }
        }
    }

    mForceInputWindowInfo.pid = 0;
    memset(&mRequest, 0, sizeof(mRequest));
}

FpsPolicyService::~FpsPolicyService()
{
    if (mDfrcFd >= 0) {
        close(mDfrcFd);
    }
}

status_t FpsPolicyService::regPolicy(const char* name, size_t size,
                                     const dfps_fps_policy_info_t& info, int32_t* /*receiveFd*/)
{
    String8 policyName(name, size);
    ALOGD("regPolicy: %llx (%s)", info.sequence, policyName.string());

    Mutex::Autolock l(mInfoLock);
    bool isNew = true;
    for (size_t i = 0; i < mList.size(); i++) {
        if (mList[i]->mInfo.sequence == info.sequence) {
            isNew = false;
            ALOGW("try to register a existed FPS policy(%s)", policyName.string());
            return ALREADY_EXISTS;
        }
    }
    if (isNew) {
        sp<FpsPolicyInfoPack> pack = new FpsPolicyInfoPack(policyName, info);
        mList.add(info.sequence, pack);
        int res = ioctl(mDfrcFd, DFRC_IOCTL_CMD_REG_POLICY, &info);
        if (res < 0) {
            ALOGE("failed to register FPS policy(%s) to dfrc: %d", policyName.string(), res);
        }
    }

    return NO_ERROR;
}

status_t FpsPolicyService::unregPolicy(uint64_t sequence, bool binderDied)
{
    Mutex::Autolock l(mInfoLock);
    ssize_t index = mList.indexOfKey(sequence);
    if (index >= 0) {
        sp<FpsPolicyInfoPack> pack = mList[index];
        ALOGD("unregPolicy: %" PRIx64 "(%s) binderDied:%d", sequence, pack->mName.string(),
                                                            binderDied);
        mList.removeItem(sequence);
        int res = ioctl(mDfrcFd, DFRC_IOCTL_CMD_UNREG_POLICY, &pack->mInfo.sequence);
        if (res < 0) {
            ALOGE("failed to unregister FPS policy(%s) from dfrc", pack->mName.string());
        }
        tryGetRequestLocked();
    }
    else if (!binderDied) {
        ALOGW("try to unregister a nonexistent FPS policy: %" PRIx64, sequence);
    }

    return NO_ERROR;
}

status_t FpsPolicyService::setFps(const dfps_fps_policy_info_t& info)
{
    Mutex::Autolock l(mInfoLock);
    ssize_t index = mList.indexOfKey(info.sequence);
    if (index >= 0) {
        sp<FpsPolicyInfoPack> pack = mList.editValueAt(index);
        pack->mInfo.fps = info.fps;
        pack->mInfo.mode = info.mode;
        pack->mInfo.target_pid = info.target_pid;
        pack->mInfo.gl_context_id = info.gl_context_id;
        pack->mInfo.flag = info.flag;
        ALOGD("setFps: %llx (%s)  fps:%d  mode:%d  targetPid:%d  glContextId:%llx  flag:%04x",
                info.sequence, pack->mName.string(), info.fps, info.mode,
                info.target_pid, info.gl_context_id, info.flag);
        int res = ioctl(mDfrcFd, DFRC_IOCTL_CMD_SET_POLICY, &info);
        if (res < 0) {
            ALOGE("failed to set FPS policy(%s:%d:%d:%x) to dfrc: %d",
                  pack->mName.string(), info.fps, info.mode, info.flag, res);
        }
        tryGetRequestLocked();
    } else {
        ALOGW("try to setFps for a nonexistent FPS policy: %llx fps=%d mode=%d flag=%x",
              info.sequence, info.fps, info.mode, info.flag);
    }
    return NO_ERROR;
}

status_t FpsPolicyService::setSingleLayer(int32_t single)
{
    Mutex::Autolock l(mInfoLock);

    mSingleLayer = single;
    tryGetRequestLocked();

    return NO_ERROR;
}

status_t FpsPolicyService::setNumberDisplay(int32_t number)
{
    Mutex::Autolock l(mInfoLock);

    mNumDisplay = number;
    tryGetRequestLocked();

    return NO_ERROR;
}

status_t FpsPolicyService::setHwcInfo(const dfps_hwc_info_t& info)
{
    Mutex::Autolock l(mInfoLock);

    mSingleLayer = info.single_layer;
    mNumDisplay = info.num_display;

    int res = ioctl(mDfrcFd, DFRC_IOCTL_CMD_SET_HWC_INFO, &info);
    if (res < 0) {
        ALOGE("failed to set HWC Info(sl:%d|nd:%d) to dfrc: %d", mSingleLayer, mNumDisplay, res);
    }
    tryGetRequestLocked();

    return NO_ERROR;
}

status_t FpsPolicyService::setInputWindows(const char* name, size_t size,
                                           const dfps_simple_input_window_info_t& info)
{
    String8 inputWindowName(name, size);
    Mutex::Autolock l(mInfoLock);

    mForceInputWindowName = inputWindowName;
    mForceInputWindowInfo = info;

    int res = ioctl(mDfrcFd, DFRC_IOCTL_CMD_SET_INPUT_WINDOW, &info);
    if (res < 0) {
        ALOGE("failed to set input window info(%d) to dfrc: %d", info.pid, res);
    }
    tryGetRequestLocked();

    return NO_ERROR;
}

status_t FpsPolicyService::regInfo(uint64_t id)
{
    ALOGD("regInfo: %" PRIx64, id);

    Mutex::Autolock l(mInfoLock);
    bool isNew = true;
    for (size_t i = 0; i < mCbList.size(); i++) {
        if (mCbList[i]->mId == id) {
            isNew = false;
            ALOGW("try to register a existed FPS Info(%" PRIx64 ")", id);
            break;
        }
    }
    if (isNew) {
        sp<FpsInfoPack> pack = new FpsInfoPack(id);
        mCbList.add(id, pack);
    }

    return NO_ERROR;
}

status_t FpsPolicyService::unregInfo(uint64_t id, bool binderDied)
{
    Mutex::Autolock l(mInfoLock);
    ssize_t index = mCbList.indexOfKey(id);
    if (index >= 0) {
        sp<FpsInfoPack> pack = mCbList[index];
        ALOGD("unregInfo: %" PRIx64, id);
        mCbList.removeItem(id);
    }
    else if (!binderDied) {
        ALOGW("try to unregister a nonexistent FPS info: %" PRIx64, id);
    }
    return NO_ERROR;
}

status_t FpsPolicyService::getPanelInfo(dfps_panel_info_t *info)
{
    Mutex::Autolock l(mInfoLock);

    info->support_90 = mPanelInfo.support_90;
    info->support_120 = mPanelInfo.support_120;
    info->num = mPanelInfo.num;

    return NO_ERROR;
}

status_t FpsPolicyService::setForegroundInfo(int32_t pid, const char *name, size_t size)
{
    String8 appName(name, size);
    Mutex::Autolock l(mInfoLock);

    mFgPid = pid;
    mFgPackageName = appName;

    DFRC_DRV_FOREGROUND_WINDOW_INFO fgInfo;
    fgInfo.pid = pid;
    int res = ioctl(mDfrcFd, DFRC_IOCTL_CMD_SET_FOREGROUND_WINDOW, &fgInfo);
    if (res < 0) {
        ALOGE("failed to set fg window info(%d) to dfrc: %d", pid, res);
    }
    tryGetRequestLocked();

    return NO_ERROR;
}

status_t FpsPolicyService::getFpsRange(dfps_fps_range_t *range)
{
    Mutex::Autolock l(mInfoLock);

    if (mPanelInfo.num == 0) {
        ALOGW("try to get fps range, but data does not init");
        return NO_INIT;
    } else if (range->index < 0 || range->index >= mPanelInfo.num) {
        ALOGW("try to get fps range with a invalid value: %d", range->index);
        return BAD_VALUE;
    }

    for (int i = 0; i < mPanelInfo.num; i++) {
        if (mRange[i].index == range->index) {
            range->min_fps = mRange[i].min_fps;
            range->max_fps = mRange[i].max_fps;
            break;
        }
    }

    return NO_ERROR;
}

status_t FpsPolicyService::setWindowFlag(int32_t flag, int32_t mask)
{
    Mutex::Autolock l(mInfoLock);

    int32_t temp = (mWindowFlag & ~(mask)) | (flag & mask);
    if (mWindowFlag != temp) {
        mWindowFlag = temp;

        DFRC_DRV_WINDOW_STATE state;
        state.window_flag = mWindowFlag;
        int res = ioctl(mDfrcFd, DFRC_IOCTL_CMD_SET_WINDOW_STATE, &state);
        if (res < 0) {
            ALOGE("failed to set window state(%08x|%08x) to dfrc: %d", flag, mask, res);
        }
        tryGetRequestLocked();
    }

    return NO_ERROR;
}

status_t FpsPolicyService::enableTracker(int32_t enable)
{
    Mutex::Autolock l(mInfoLock);

    if (mNeedInitTracker) {
        mNeedInitTracker = false;
        mTracker = new FpsPolicyTracker;
        mTracker->run("FpsPolicyTracker");
    }

    if (enable) {
        mTracker->setResume();
    } else {
        mTracker->setSuspend();
    }

    return NO_ERROR;
}

status_t FpsPolicyService::forbidAdjustingVsync(int32_t forbid)
{
    Mutex::Autolock l(mInfoLock);

    if (mForbidVsync != forbid) {
        mForbidVsync = forbid;
        int res = ioctl(mDfrcFd, DFRC_IOCTL_CMD_FORBID_ADJUSTING_VSYNC, &forbid);
        if (res < 0) {
            ALOGE("failed to forbid adjusting vsync(%d): %d", forbid, res);
        }
        tryGetRequestLocked();
    }

    return NO_ERROR;
}

status_t FpsPolicyService::dump(uint32_t *outSize, char *outBuffer)
{
    String8 tmp;

    if (outBuffer != NULL) {
        uint32_t min = *outSize > mDumpString.size() ? mDumpString.size() : *outSize;
        memcpy(outBuffer, mDumpString.string(), min);
    } else {
        Mutex::Autolock l(mInfoLock);
        printListFpsRangeLocked(tmp);
        printSystemInfoLocked(tmp);
        printListPolicyInfoLocked(tmp);
        printListInfoLocked(tmp);
        mDumpString = tmp;
        *outSize = tmp.size();
    }

    return NO_ERROR;
}

status_t FpsPolicyService::registerCallback(int32_t descriptor, dfps_callback_data_t callbackData, dfps_function_pointer_t pointer)
{
    ALOGI("%s: registerrr cb:%d  target:%d", __func__, descriptor, DFPS_CALLBACK_SW_VSYNC_CHANGE);
    switch(descriptor) {
        case DFPS_CALLBACK_SW_VSYNC_CHANGE:
            mCallbackSwVsyncChange = reinterpret_cast<DFPS_PFN_SW_VSYNC_CHANGE>(pointer);
            mCallbackDataSwVsyncChange = callbackData;
            break;

        default:
            ALOGE("%s: uknownnn descriptor(%d)", __func__, descriptor);
            return NO_ERROR;
    }

    return NO_ERROR;
}

void FpsPolicyService::printSystemInfoLocked(String8& result)
{
    result.appendFormat("Request SW VSync Mode: %d  FPS: %d\n", mRequest.mode, mRequest.fps);
    result.appendFormat("Number of display[%d]  Single Layer[%d]\n", mNumDisplay, mSingleLayer);
    result.appendFormat("Foreground pid[%d]  Foreground Name[%s]\n", mFgPid, mFgPackageName.string());
    result.appendFormat("Force window pid[%d]  Force window name[%s]\n",
            mForceInputWindowInfo.pid, mForceInputWindowName.string());
    result.appendFormat("Window flags[%08x]\n", mWindowFlag);
    result.appendFormat("Forbid adjusting VSync[%d]\n", mForbidVsync);
    result.appendFormat("\n");
}

void FpsPolicyService::printListFpsRangeLocked(String8& result)
{
    result.appendFormat("Panel info: 90Hz[%d] 120Hz[%d]\n", mPanelInfo.support_90, mPanelInfo.support_120);
    result.appendFormat("Total number of fps index: %d\n", mPanelInfo.num);
    for (int i = 0; i < mPanelInfo.num; i++) {
        result.appendFormat("  + index_%d: %d~%d\n", mRange[i].index, mRange[i].min_fps, mRange[i].max_fps);
    }
    result.appendFormat("\n");
}

void FpsPolicyService::printListPolicyInfoLocked(String8& result)
{
    result.appendFormat("All Fps Policy(count = %zu)\n", mList.size());
    for (size_t i = 0; i < mList.size(); i++) {
        sp<FpsPolicyInfoPack> pack = mList[i];
        result.appendFormat("  + FpsPolicy %llx (%s)\n",
                pack->mInfo.sequence, pack->mName.string());
        result.appendFormat("    API: %s   FPS: %d   MODE: %d\n",
                getApiString(pack->mInfo.api), pack->mInfo.fps, pack->mInfo.mode);
        result.appendFormat("    TARGET: %d   GlContextId: %llx   FLAG: %04x\n",
                pack->mInfo.target_pid, pack->mInfo.gl_context_id, pack->mInfo.flag);
    }
    result.appendFormat("\n");
}

void FpsPolicyService::printListInfoLocked(String8& result)
{
    result.appendFormat("All Fps Info(count = %zu)\n", mCbList.size());
    for (size_t i = 0; i < mCbList.size(); i++) {
        sp<FpsInfoPack> info = mCbList[i];
        result.appendFormat("  + FpsInfo %" PRIx64 "\n", info->mId);
    }
    result.appendFormat("\n");
}

status_t FpsPolicyService::readyToRun()
{
    mStop = false;

    return 0;
}

bool FpsPolicyService::threadLoop()
{
    while (true) {

        if (mStop) {
            return false;
        }

        DFRC_DRV_VSYNC_REQUEST request;
        DFRC_DRC_REQUEST_SET requestSet;
        {
            Mutex::Autolock _l(mInfoLock);
            request = mRequest;
        }
        int res = ioctl(mDfrcFd, DFRC_IOCTL_CMD_GET_VSYNC_REQUEST, &request);
        int ret = 0;
        requestSet.num = 0;
        requestSet.policy = NULL;
        if (res < 0) {
            ALOGW("failed to get vsync request from dfrc: %d\n", res);
        } else if (request.num_policy != 0) {
            requestSet.num = request.num_policy;
            requestSet.policy = new DFRC_DRV_POLICY[requestSet.num];
            ret = ioctl(mDfrcFd, DFRC_IOCTL_CMD_GET_REQUEST_SET, &requestSet);
            if (ret < 0) {
                ALOGW("failed to get request set from dfrc: %d\n", res);
                ret = 0;
            } else {
                ret = request.num_policy;
            }
        }

        bool needAdjustVSync = false;
        {
            Mutex::Autolock _l(mInfoLock);
            if (res < 0) {
                mCondition.wait(mInfoLock);
            } else {
                if (!mNeedInitTracker) {
                    mTracker->saveRequestInfo(request, requestSet);
                }
                dumpVsyncRequestLocked(request, requestSet);

                //TODO report info to FpsPolicy

                if (mRequest.sw_fps != request.sw_fps || mRequest.sw_mode != request.sw_mode) {
                    needAdjustVSync = true;
                }
                mRequest = request;
            }
        }
        if (needAdjustVSync && mCallbackSwVsyncChange != NULL) {
            mCallbackSwVsyncChange(mCallbackDataSwVsyncChange, mRequest.sw_mode, mRequest.sw_fps);
        }
        if (requestSet.policy) {
            delete [] requestSet.policy;
            requestSet.policy = NULL;
        }
    }

    return false;
}

void FpsPolicyService::tryGetRequestLocked()
{
    mCount++;
    mCondition.signal();
}

void FpsPolicyService::dumpVsyncRequestLocked(const DFRC_DRV_VSYNC_REQUEST& request,
                                              const DFRC_DRC_REQUEST_SET& requestSet)
{
    ALOGI("oldRequest: fps[%d] mode[%d] --> newRequest: fps[%d] mode[%d] num[r:%d|rs:%d]",
          mRequest.fps, mRequest.mode, request.fps, request.mode,
          request.num_policy, requestSet.num);

    FPS_ATRACE_BUFFER("oldRequest: fps[%d] mode[%s] --> newRequest: fps[%d] mode[%s]",
                      mRequest.fps, getModeString(mRequest.mode),
                      request.fps, getModeString(request.mode));
    FPS_ATRACE_BUFFER("detail: fps[%d] mode[%d] swFps[%d] swMode[%d] validInfo[%d] transState[%d] forbidVsync[%d]",
                      request.fps, request.mode, request.sw_fps, request.sw_mode,
                      request.valid_info, request.transient_state, request.forbid_vsync);
    FPS_ATRACE_BUFFER("numPolicy[%d]  numRequestSet[%d]", request.num_policy, requestSet.num);
    for (int i = 0; i < requestSet.num; i++) {
        DFRC_DRV_POLICY *policy = &requestSet.policy[i];
        FPS_ATRACE_BUFFER("seq[%llu]  api[%s]  pid[%d]  fps[%d]  mode[%s]  tPid[%d]  glId[%llu]",
                          policy->sequence, getApiString(policy->api), policy->pid,
                          policy->fps, getModeString(policy->mode),
                          policy->target_pid, policy->gl_context_id);
        ALOGI("seq[%llu]  api[%s]  pid[%d]  fps[%d]  mode[%s]  tPid[%d]  glId[%llu]",
                          policy->sequence, getApiString(policy->api), policy->pid,
                          policy->fps, getModeString(policy->mode),
                          policy->target_pid, policy->gl_context_id);
    }
}
};
