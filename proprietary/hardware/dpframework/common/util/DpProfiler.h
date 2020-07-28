#ifndef __DP_PROFILER_H__
#define __DP_PROFILER_H__

#include "DpConfig.h"
#include "DpDataType.h"
#include "DpTimer.h"

#if CONFIG_FOR_OS_ANDROID
#include <utils/Trace.h>
#endif // CONFIG_FOR_OS_ANDROID


#ifdef MDP_VERSION_2_0

#include "DpDriver.h"

#undef ATRACE_TAG
#define ATRACE_TAG ATRACE_TAG_ALWAYS
#define DP_TRACE_CALL(...)                  MDPTrace ___tracer(ATRACE_TAG, __FUNCTION__, ##__VA_ARGS__)
#define DP_TRACE_BEGIN(name)                ATRACE_BEGIN(name)
#define DP_TRACE_END()                      ATRACE_END()
#define DP_TRACE_NAME(name)

class MDPTrace
{

public:

    MDPTrace(uint64_t, const char* name, int32_t label = 1)
    {
        Tracelabel = label;

        if (DpDriver::getInstance()->getEnableSystrace() >= Tracelabel)
        {
            ATRACE_BEGIN(name);
        }

    };

    ~MDPTrace()
    {

        if (DpDriver::getInstance()->getEnableSystrace() >= Tracelabel)
        {
            ATRACE_END();
        }

    };

    int32_t Tracelabel = 0;
};

#else

#if CONFIG_FOR_SYSTRACE

#undef ATRACE_TAG
#define ATRACE_TAG ATRACE_TAG_ALWAYS
#define DP_TRACE_CALL()                     ATRACE_CALL()
#define DP_TRACE_BEGIN(name)                ATRACE_BEGIN(name)
#define DP_TRACE_END()                      ATRACE_END()

#else

#define DP_TRACE_CALL()
#define DP_TRACE_BEGIN(name)
#define DP_TRACE_END()

#endif  // CONFIG_FOR_SYSTRACE

#endif

#if CONFIG_FOR_PROFILE_INFO

// Reset profile info
#define DP_PROFILER_RESET_PROFILE_INFO()                                                    \
{                                                                                           \
    dpProfilerResetInfo();                                                                  \
}

/*
 * For overall all
 */
#define DP_PROFILER_PERFORMANCE_BEGIN()                                                     \
{                                                                                           \
    dpProfilerMarkBegin<0>();                                                               \
}

#define DP_PROFILER_PERFORMANCE_END()                                                       \
{                                                                                           \
    dpProfilerMarkEnd<0>();                                                                 \
}

/*
 * For config frame
 */
#define DP_PROFILER_COMPOSE_PATH_BEGIN()                                                    \
{                                                                                           \
    dpProfilerMarkBegin<1>();                                                               \
}

#define DP_PROFILER_COMPOSE_PATH_END()                                                      \
{                                                                                           \
    dpProfilerMarkEnd<1>();                                                                 \
}

/*
 * For config frame
 */
#define DP_PROFILER_CONFIG_FRAME_BEGIN()                                                    \
{                                                                                           \
    dpProfilerMarkBegin<2>();                                                               \
}

#define DP_PROFILER_CONFIG_FRAME_END()                                                      \
{                                                                                           \
    dpProfilerMarkEnd<2>();                                                                 \
}


/*
 * For config tile
 */
#define DP_PROFILER_CONFIG_TILE_BEGIN()                                                     \
{                                                                                           \
    dpProfilerMarkBegin<3>();                                                               \
}


#define DP_PROFILER_CONFIG_TILE_END()                                                       \
{                                                                                           \
    dpProfilerMarkEnd<3>();                                                                 \
}

/*
 * For execution
 */
#define DP_PROFILER_HW_EXECUTION_BEGIN()                                                    \
{                                                                                           \
    dpProfilerMarkBegin<4>();                                                               \
}


#define DP_PROFILER_HW_EXECUTION_END()                                                      \
{                                                                                           \
    dpProfilerMarkEnd<4>();                                                                 \
}

/*
 * For execution
 */
#define DP_PROFILER_MAP_HW_ADDR_BEGIN()                                                     \
{                                                                                           \
    dpProfilerMarkBegin<5>();                                                               \
}


#define DP_PROFILER_MAP_HW_ADDR_END()                                                       \
{                                                                                           \
    dpProfilerMarkEnd<5>();                                                                 \
}

/*
 * For execution
 */
#define DP_PROFILER_FLUSH_BUFFER_BEGIN()                                                    \
{                                                                                           \
    dpProfilerMarkBegin<6>();                                                               \
}


#define DP_PROFILER_FLUSH_BUFFER_END()                                                      \
{                                                                                           \
    dpProfilerMarkEnd<6>();                                                                 \
}

/*
 * For dump result
 */
#define DP_PROFILER_DUMP_PROFILE_INFO()                                                     \
{                                                                                           \
    dpProfilerDumpInfo();                                                                   \
}

extern DpTimeValue g_profileTemp[7];
extern int32_t     g_profileData[7];

// Functions
void dpProfilerResetInfo();

template <int32_t ID>
void dpProfilerMarkBegin()
{
    DP_TIMER_GET_CURRENT_TIME(g_profileTemp[ID]);
}

template <int32_t ID>
void dpProfilerMarkEnd()
{
    DpTimeValue endTime;
    int32_t     duration;

    duration = 0;

    DP_TIMER_GET_CURRENT_TIME(endTime);

    DP_TIMER_GET_DURATION_IN_MS(g_profileTemp[ID],
                                endTime,
                                duration);


    g_profileData[ID] += duration;
}

void dpProfilerDumpInfo();

#else

#define DP_PROFILER_RESET_PROFILE_INFO()
#define DP_PROFILER_PERFORMANCE_BEGIN()
#define DP_PROFILER_PERFORMANCE_END()
#define DP_PROFILER_COMPOSE_PATH_BEGIN()
#define DP_PROFILER_COMPOSE_PATH_END()
#define DP_PROFILER_CONFIG_FRAME_BEGIN()
#define DP_PROFILER_CONFIG_FRAME_END()
#define DP_PROFILER_CONFIG_TILE_BEGIN()
#define DP_PROFILER_CONFIG_TILE_END()
#define DP_PROFILER_HW_EXECUTION_BEGIN()
#define DP_PROFILER_HW_EXECUTION_END()
#define DP_PROFILER_MAP_HW_ADDR_BEGIN()
#define DP_PROFILER_MAP_HW_ADDR_END()
#define DP_PROFILER_FLUSH_BUFFER_BEGIN()
#define DP_PROFILER_FLUSH_BUFFER_END()
#define DP_PROFILER_DUMP_PROFILE_INFO()

#endif  // CONFIG_FOR_PROFILE_INFO

#endif  // __DP_PROFILER_H__
