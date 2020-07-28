#ifndef __DP_LOGGER_ANDROID_H__
#define __DP_LOGGER_ANDROID_H__

#include <cutils/log.h>

#undef LOG_TAG
#define LOG_TAG "MDP"

#if defined(USING_MTK_LDVT)     // LDVT

#define DPLOGI(fmt, ...) (printf("MDP I " fmt,  ##__VA_ARGS__))
#define DPLOGW(fmt, ...) (printf("MDP W " fmt,  ##__VA_ARGS__))
#define DPLOGD(fmt, ...) (printf("MDP D " fmt,  ##__VA_ARGS__))
#define DPLOGE(fmt, ...) (printf("MDP E " fmt,  ##__VA_ARGS__))

#else

#define DPLOGI(...) // (ALOGI(__VA_ARGS__))

#define DPLOGW(...) (ALOGW(__VA_ARGS__))

#define DPLOGD(...) (ALOGD(__VA_ARGS__))

#define DPLOGE(...) (ALOGE(__VA_ARGS__))


#endif  // LDVT

#endif  // __DP_LOGGER_ANDROID_H__
