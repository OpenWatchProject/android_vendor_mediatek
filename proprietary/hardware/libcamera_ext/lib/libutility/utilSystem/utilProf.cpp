#define LOG_TAG "utilProf"
#include <stdio.h>
#include <string.h>
#include "utilProf.h"

#define BILLION (1000000000)

#define MTK_LOG_ENABLE 1
#if defined(__ANDROID__) || defined(ANDROID)
#include <android/log.h>
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#else
#define LOGD(fmt, ...) printf("[%s] " fmt, LOG_TAG, ##__VA_ARGS__)
#define LOGE(fmt, ...) printf("[%s] Error(%d) " fmt, LOG_TAG, __LINE__, ##__VA_ARGS__)
#endif

static MUINT64 utilCaptureTime()
{
    MUINT64 cap = 0;
#if defined(__linux__) || defined(__ANDROID__) || defined(__QNX__) || defined(__CYGWIN__)
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    cap = (MUINT64)((MUINT64)t.tv_nsec + (MUINT64)t.tv_sec*BILLION);
#elif defined(_WIN32) || defined(UNDER_CE)
    LARGE_INTEGER t;
    QueryPerformanceCounter(&t);
    cap = (MUINT64)t.QuadPart;
#endif
    return cap;
}

void utilStartCapture(utilPerf *perf)
{
    perf->beg = utilCaptureTime();
}

MFLOAT utilTimeToMS(MUINT64 c) {
#define NS_PER_MSEC  (1000000.0f)
    return (MFLOAT)c/NS_PER_MSEC;
}

void utilStopCapture(utilPerf *perf)
{
    perf->end = utilCaptureTime();
    perf->tmp = perf->end - perf->beg;
    perf->sum += perf->tmp;
    perf->num++;
    perf->avg = perf->sum / perf->num;
    perf->min = (perf->min < perf->tmp ? perf->min : perf->tmp);
}

void utilPrintPerf(utilPerf *perf, const char *name)
{
    LOGD( "[%s] sum:" UTIL_FMT_TIME "ms, avg:" UTIL_FMT_TIME "ms\n",
        name,
        utilTimeToMS(perf->sum),
        utilTimeToMS(perf->avg));
}

void utilInitPerf(utilPerf *perf)
{
    memset(perf, 0, sizeof(utilPerf));
    perf->min = 0xFFFFFFFFFFFFFFFF;
}

utilPerf utilPerf::operator+(const utilPerf &in)
{
    utilPerf result;
    result.sum = this->sum + in.sum;
    result.avg = this->avg + in.avg;
    return result;
}