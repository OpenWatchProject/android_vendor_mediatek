#ifndef __PQ_SERVICE_TIMER_H__
#define __PQ_SERVICE_TIMER_H__

#include <stdio.h>
#include <sys/time.h>

typedef struct timeval PQTimeValue;

#define PQSERVICE_TIMER_GET_CURRENT_TIME(current)                               \
{                                                                               \
    if (mDebugLevel & eDebugTimer)                                              \
        gettimeofday(&current, NULL);                                           \
}

#define PQSERVICE_TIMER_GET_DURATION_IN_US(start,                               \
                                    end,                                        \
                                    duration)                                   \
{                                                                               \
    if (mDebugLevel & eDebugTimer) {                                            \
        uint64_t time1;                                                         \
        uint64_t time2;                                                         \
                                                                                \
        time1 = (uint64_t)(start.tv_sec) * 1000000 + (uint64_t)(start.tv_usec); \
        time2 = (uint64_t)(end.tv_sec) * 1000000   + (uint64_t)(end.tv_usec);   \
                                                                                \
        duration = time2 - time1;                                               \
    }                                                                           \
}

#define PQSERVICE_TIMER_GET_DURATION_IN_MS(start,                               \
                                    end,                                        \
                                    duration)                                   \
{                                                                               \
    if (mDebugLevel & eDebugTimer) {                                            \
        uint64_t time1;                                                         \
        uint64_t time2;                                                         \
                                                                                \
        time1 = (uint64_t)(start.tv_sec) * 1000000 + (uint64_t)(start.tv_usec); \
        time2 = (uint64_t)(end.tv_sec) * 1000000   + (uint64_t)(end.tv_usec);   \
                                                                                \
        duration = (time2 - time1) / 1000;                                      \
    }                                                                           \
}

#endif  // __PQ_SERVICE_TIMER_H__