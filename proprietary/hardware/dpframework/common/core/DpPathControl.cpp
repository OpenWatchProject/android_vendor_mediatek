#include "DpPathControl.h"
#include "DpPathBase.h"

DpPathControl::DpPathControl(DpStream *stream)
    : m_composer(stream)
{
}


DpPathControl::~DpPathControl()
{
}


DP_STATUS_ENUM DpPathControl::onInitStream()
{
    DP_TRACE_CALL();
    DP_STATUS_ENUM           status;
    DpPathComposer::iterator iterator;
    DP_STATUS_ENUM           overall;
    DpTimeValue begin;
    DpTimeValue end;
    int32_t     diff;

    DPLOGI("DpPathControl: init stream begin\n");

    DP_TIMER_GET_CURRENT_TIME(begin);

    DP_PROFILER_COMPOSE_PATH_BEGIN();
    status = m_composer.compose();
    DP_PROFILER_COMPOSE_PATH_END();

    if (status != DP_STATUS_RETURN_SUCCESS)
    {
        DPLOGE("DpPathControl: compose path for stream failed\n");
        return status;
    }

    DP_TIMER_GET_CURRENT_TIME(end);
    DP_TIMER_GET_DURATION_IN_MS(begin,
                                end,
                                diff);
    if (diff > 10)
    {
        DPLOGW("DpPathControl: compose stream %d ms\n", diff);
    }

    overall = DP_STATUS_RETURN_SUCCESS;
    for (iterator = m_composer.begin(); iterator != m_composer.end(); iterator++)
    {
        status = iterator->initPath();
        if (status != DP_STATUS_RETURN_SUCCESS)
        {
            DPLOGE("DpPathControl: init stream path (%p) failed (%d)\n", &(*iterator), status);
            overall = (DP_STATUS_ENUM)(overall | status);
        }
    }

    DPLOGI("DpPathControl: init stream end\n");

    return overall;
}


DP_STATUS_ENUM DpPathControl::onStartStream()
{
    DP_STATUS_ENUM           status;
    DpPathComposer::iterator iterator;
    DP_STATUS_ENUM           overall;

    DPLOGI("DpPathControl: start stream begin\n");

    // Default value for status
    overall = DP_STATUS_RETURN_SUCCESS;
    for (iterator = m_composer.begin(); iterator != m_composer.end(); iterator++)
    {
        status = iterator->startPath();
        if (status != DP_STATUS_RETURN_SUCCESS)
        {
            DPLOGE("DpPathControl: start stream path (%p) failed (%d)\n", &(*iterator), status);
            overall = (DP_STATUS_ENUM)(overall | status);
        }
    }

    DPLOGI("DpPathControl: start stream end\n");

    return overall;
}


DP_STATUS_ENUM DpPathControl::onWaitStream()
{
    DP_STATUS_ENUM           status;
    DpPathComposer::iterator iterator;
    DP_STATUS_ENUM           overall;

    DPLOGI("DpPathControl: wait stream begin\n");

    // Default value for status
    overall = DP_STATUS_RETURN_SUCCESS;
    for (iterator = m_composer.begin(); iterator != m_composer.end(); iterator++)
    {
        status = iterator->waitPath();
        if (status != DP_STATUS_RETURN_SUCCESS)
        {
            DPLOGE("DpPathControl: wait stream path (%p) failed (%d)\n", &(*iterator), status);
            overall = (DP_STATUS_ENUM)(overall | status);
        }
    }

    DPLOGI("DpPathControl: wait stream end\n");

    return overall;
}


DP_STATUS_ENUM DpPathControl::onStopStream()
{
    DP_STATUS_ENUM           status;
    DpPathComposer::iterator iterator;
    DP_STATUS_ENUM           overall;

    DPLOGI("DpPathControl: stop stream begin\n");

    // Default value for status
    overall = DP_STATUS_RETURN_SUCCESS;

    for (iterator = m_composer.begin(); iterator != m_composer.end(); iterator++)
    {
        status = iterator->stopPath();
        if (status != DP_STATUS_RETURN_SUCCESS)
        {
            DPLOGE("DpPathControl: stop stream path (%p) failed (%d)\n", &(*iterator), status);
            overall = (DP_STATUS_ENUM)(overall | status);
        }
    }

    for (iterator = m_composer.begin(); iterator != m_composer.end(); iterator++)
    {
        status = iterator->abortPath();
        if (status != DP_STATUS_RETURN_SUCCESS)
        {
            DPLOGE("DpPathControl: abort stream path (%p) failed (%d)\n", &(*iterator), status);
            overall = (DP_STATUS_ENUM)(overall | status);
        }
    }

    for (iterator = m_composer.begin(); iterator != m_composer.end(); iterator++)
    {
        status = iterator->destroyPath();
        if (status != DP_STATUS_RETURN_SUCCESS)
        {
            DPLOGE("DpPathControl: abort stream path (%p) failed (%d)\n", &(*iterator), status);
            overall = (DP_STATUS_ENUM)(overall | status);
        }
    }

    DPLOGI("DpPathControl: stop stream end\n");

    return overall;
}


DP_STATUS_ENUM DpPathControl::onDumpDebugStream()
{
    DP_STATUS_ENUM           status;
    DpPathComposer::iterator iterator;
    DP_STATUS_ENUM           overall;

    DPLOGI("DpPathControl: dump debug stream begin\n");

    // Default value for status
    overall = DP_STATUS_RETURN_SUCCESS;
    for (iterator = m_composer.begin(); iterator != m_composer.end(); iterator++)
    {
        status = iterator->dumpDebugPath();
        if (status != DP_STATUS_RETURN_SUCCESS)
        {
            DPLOGE("DpPathControl: dump debug stream path (%p) failed (%d)\n", &(*iterator), status);
            overall = (DP_STATUS_ENUM)(overall | status);
        }
    }

    DPLOGI("DpPathControl: dump debug stream end\n");

    return overall;
}

