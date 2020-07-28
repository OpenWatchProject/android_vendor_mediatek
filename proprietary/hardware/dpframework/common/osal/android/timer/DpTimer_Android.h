#ifndef __DP_TIMER_ANDROID_H__
#define __DP_TIMER_ANDROID_H__

#include <stdio.h>
#include <sys/time.h>

typedef struct timeval DpTimeValue;

#define DP_TIMER_GET_CURRENT_TIME(current)                                  \
{                                                                           \
    gettimeofday(&current, NULL);                                           \
}

#define DP_TIMER_GET_DURATION_IN_US(start,                                  \
                                    end,                                    \
                                    duration)                               \
{                                                                           \
    uint64_t time1;                                                         \
    uint64_t time2;                                                         \
                                                                            \
    time1 = (uint64_t)(start.tv_sec) * 1000000 + (uint64_t)(start.tv_usec); \
    time2 = (uint64_t)(end.tv_sec) * 1000000   + (uint64_t)(end.tv_usec);   \
                                                                            \
    duration = (int32_t)(time2 - time1);                                    \
}

#define DP_TIMER_GET_DURATION_IN_MS(start,                                  \
                                    end,                                    \
                                    duration)                               \
{                                                                           \
    uint64_t time1;                                                         \
    uint64_t time2;                                                         \
                                                                            \
    time1 = (uint64_t)(start.tv_sec) * 1000000 + (uint64_t)(start.tv_usec); \
    time2 = (uint64_t)(end.tv_sec) * 1000000   + (uint64_t)(end.tv_usec);   \
                                                                            \
    duration = (int32_t)((time2 - time1) / 1000);                           \
}

#endif  // __DP_TIMER_ANDROID_H__
