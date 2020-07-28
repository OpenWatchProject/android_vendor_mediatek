#include "LogUtils.h"
#include <sys/time.h>

#define LOG_TAG "FeatureUtils"

void LogUtils::fps(const char* tag) {
    static long lastPrint = getCurrentMillSeconds();
    static long times = 0;
    long now = getCurrentMillSeconds();
    if (now - lastPrint < 1000) {
        times ++;
    } else {
        int fps = times/((now-lastPrint)/1000);
        times = 0;
        lastPrint = now;
        if (gLogFps > 0) {
            ALOGD("[%s] fps = %d", tag, fps);
        }
    }
}

long LogUtils::getCurrentMillSeconds() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}