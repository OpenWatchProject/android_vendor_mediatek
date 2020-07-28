#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include "disp_dejitter/DispDeJitter.h"

#include <vector>

#include <cutils/compiler.h>
#include <cutils/properties.h>

#include <utils/Log.h>
#include <utils/Trace.h>
#include <utils/String8.h>

#include <ui/GraphicBuffer.h>
#include <ui/gralloc_extra.h>

#ifdef ATRACE_TAG_PERF
#define DJ_ATRACE_INT(name, value) if(mEnableLog){atrace_int(ATRACE_TAG|ATRACE_TAG_PERF, name, value);}
#else
#define DJ_ATRACE_INT(name, value) if(mEnableLog){atrace_int(ATRACE_TAG, name, value);}
#endif
#define DJ_ATRACE_BUFFER(x, ...)                                                \
    if (ATRACE_ENABLED() && mEnableLog) {                                       \
        char ___traceBuf[256];                                                  \
        snprintf(___traceBuf, sizeof(___traceBuf), x, ##__VA_ARGS__);           \
        android::ScopedTrace ___bufTracer(ATRACE_TAG, ___traceBuf);             \
    }

// Kalman Filter parameters
const float kfQk = 15000.0f;    // covariance of the process noise
const float kfRk = 20000000.0f; // covariance of the observation noise
// Display DeJitter parameters
const int64_t gInitialProcessTime = 1000000;    // Initial process time = 1ms
const uint64_t gResetThreashold = 200000000;    // source timestamp gap > 200ms, reset filter
const int64_t gCushionTime = (int64_t)(1000000000.0f/60.0f + 0.5f); // 1 vsync cycle
// Display DeJitter Blacklist
static std::vector<const char*> gBlacklist = {
    "com.tencent.mm",
    "com.sina.weibo",
    "com.mediatek.camera"
};

namespace android {

DispDeJitter::DispDeJitter()
: mEnableListChecked(false)
, mFinalEnable(true)
, mLastSourceTimestamp(0) {
    char value[PROPERTY_VALUE_MAX] = {};
    property_get("vendor.debug.sf.display_dejitter", value, "1");
    mEnable = (atoi(value) != 0);
    property_get("vendor.debug.sf.display_dejitter_log", value, "1");
    mEnableLog = (atoi(value) != 0);
    resetFilter();
}

DispDeJitter::~DispDeJitter() {}

bool DispDeJitter::shouldDelayPresent(const String8& name, const sp<GraphicBuffer>& gb,
                                      const nsecs_t& expectedPresent) {
    if (CC_UNLIKELY(!mEnable)) {
        DJ_ATRACE_BUFFER("[dejitter] disabled");
        return false;
    }

    if (CC_UNLIKELY(!mEnableListChecked)) {
        mFinalEnable = inquiryEnableList(name);
        mEnableListChecked = true;
    }

    if (CC_UNLIKELY(!mFinalEnable)) {
        DJ_ATRACE_BUFFER("[dejitter] disabled_");
        return false;
    }

    if (gb == NULL) {
        DJ_ATRACE_BUFFER("[dejitter] gb is null");
        return false;
    }

    ge_timestamp_info_t timestampInfo;
    memset(&timestampInfo, 0, sizeof(timestampInfo));
    gralloc_extra_query(gb->handle, GRALLOC_EXTRA_GET_TIMESTAMP_INFO, &timestampInfo);
    if (timestampInfo.timestamp == 0) {
        DJ_ATRACE_BUFFER("[dejitter] no source timestamp");
        return false;
    }
    if (CC_UNLIKELY(timestampInfo.timestamp - mLastSourceTimestamp > gResetThreashold)) {
        resetFilter();
    }
    mLastSourceTimestamp = timestampInfo.timestamp;

    int64_t processTime = kalmanFilter((int64_t)(timestampInfo.timestamp_queued - timestampInfo.timestamp));
    int64_t showTime = timestampInfo.timestamp + processTime + gCushionTime;
    DJ_ATRACE_INT("processTime", processTime);
    if (showTime > expectedPresent) {
        DJ_ATRACE_INT("LAYER_DELAY", 1);
        DJ_ATRACE_BUFFER("[dejitter] src: %" PRIu64 "  show: %" PRId64 "  exped: %" PRId64,
            timestampInfo.timestamp, showTime, expectedPresent);
        return true;
    }

    DJ_ATRACE_INT("LAYER_DELAY", 0);
    DJ_ATRACE_BUFFER("[dejitter] src: %" PRIu64 "  show: %" PRId64 "  exped: %" PRId64,
        timestampInfo.timestamp, showTime, expectedPresent);
    return false;
}

inline bool DispDeJitter::inquiryEnableList(const String8& name) {
    for (const auto& blackName : gBlacklist) {
        if (name.find(blackName) >= 0)
            return false;
    }
    return true;
}

inline int64_t DispDeJitter::kalmanFilter(int64_t measuredProcessTime) {
    mLastCovariance += kfQk;
    float kalmanGain = mLastCovariance / (mLastCovariance + kfRk);
    mLastCovariance *= (1.0f - kalmanGain);
    mLastEstimateProcessTime += (int64_t)(kalmanGain * (float)(measuredProcessTime - mLastEstimateProcessTime));

    return mLastEstimateProcessTime;
}

inline void DispDeJitter::resetFilter() {
    mLastEstimateProcessTime = gInitialProcessTime;
    mLastCovariance = kfRk;
    DJ_ATRACE_BUFFER("[dejitter] reset filter");
}

DispDeJitter* createDispDeJitter() {
    return new DispDeJitter();
}

void destroyDispDeJitter(DispDeJitter* dispDeJitter) {
    if (dispDeJitter != nullptr) {
        delete dispDeJitter;
    }
}

bool shouldDelayPresent(DispDeJitter* dispDeJitter, const String8& name, const sp<GraphicBuffer>& gb,
                        const nsecs_t& expectedPresent) {
    if (dispDeJitter != nullptr) {
        return dispDeJitter->shouldDelayPresent(name, gb, expectedPresent);
    }
    return false;
}

void markTimestamp(const sp<GraphicBuffer>& gb) {
    if (gb != nullptr) {
        ge_timestamp_info_t timestampInfo;
        memset(&timestampInfo, 0, sizeof(timestampInfo));
        gralloc_extra_query(gb->handle, GRALLOC_EXTRA_GET_TIMESTAMP_INFO, &timestampInfo);
        if (timestampInfo.timestamp != 0) {
            timestampInfo.timestamp_queued = systemTime(SYSTEM_TIME_MONOTONIC);
            gralloc_extra_perform(gb->handle, GRALLOC_EXTRA_SET_TIMESTAMP_INFO, &timestampInfo);
        }
    }
}

}   // namespace android
