#ifndef __PQ_TIMER_H__
#define __PQ_TIMER_H__

#include <stdio.h>
#include <sys/time.h>

typedef struct timeval PQTimeValue;

#define PQ_TIMER_GET_CURRENT_TIME(current)          \
{                                                   \
    gettimeofday(&current, NULL);                   \
}

#define PQ_TIMER_GET_DURATION_IN_US(start,          \
                                    end,            \
                                    duration)       \
{                                                   \
    int32_t time1;                                  \
    int32_t time2;                                  \
                                                    \
    time1 = start.tv_sec * 1000000 + start.tv_usec; \
    time2 = end.tv_sec * 1000000   + end.tv_usec;   \
                                                    \
    duration = time2 - time1;                       \
}

#define PQ_TIMER_GET_DURATION_IN_MS(start,          \
                                    end,            \
                                    duration)       \
{                                                   \
    int32_t time1;                                  \
    int32_t time2;                                  \
                                                    \
    time1 = start.tv_sec * 1000000 + start.tv_usec; \
    time2 = end.tv_sec * 1000000   + end.tv_usec;   \
                                                    \
    duration = (time2 - time1) / 1000;              \
}

#endif  // __PQ_TIMER_H__
