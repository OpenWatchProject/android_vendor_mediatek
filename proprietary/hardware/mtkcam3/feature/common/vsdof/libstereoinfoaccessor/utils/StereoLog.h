#ifndef STEREO_INFO_ACCESSOR_LOG_H
#define STEREO_INFO_ACCESSOR_LOG_H

#include <utils/Log.h>

#define STEREO_LOG_V 4
#define STEREO_LOG_D 3
#define STEREO_LOG_I 2
#define STEREO_LOG_W 1
#define STEREO_LOG_E 0

#ifndef STEREO_LOG_LEVEL //the macro can defined in Android.mk
#define STEREO_LOG_LEVEL STEREO_LOG_I
#endif

#define StereoLogV(...)                         \
    do {                                        \
        if(STEREO_LOG_LEVEL >= STEREO_LOG_V) {  \
            ALOGV(__VA_ARGS__);                 \
        }                                       \
    } while(0)                                  \

#define StereoLogD(...)                         \
    do {                                        \
        if(STEREO_LOG_LEVEL >= STEREO_LOG_D) {  \
            ALOGD(__VA_ARGS__);                 \
        }                                       \
    } while(0)                                  \

#define StereoLogI(...)                         \
    do {                                        \
        if(STEREO_LOG_LEVEL >= STEREO_LOG_I) {  \
            ALOGI(__VA_ARGS__);                 \
        }                                       \
    } while(0)                                  \

#define StereoLogW(...)                         \
    do {                                        \
        if(STEREO_LOG_LEVEL >= STEREO_LOG_W) {  \
            ALOGW(__VA_ARGS__);                 \
        }                                       \
    } while(0)                                  \

#define StereoLogE(...)                         \
    do {                                        \
        if(STEREO_LOG_LEVEL >= STEREO_LOG_E) {  \
            ALOGE(__VA_ARGS__);                 \
        }                                       \
    } while(0)                                  \

#endif
