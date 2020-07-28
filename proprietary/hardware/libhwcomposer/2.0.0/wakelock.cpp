#define DEBUG_LOG_TAG "WLT"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include "wakelock.h"

#include <hardware_legacy/power.h>

#include "utils/tools.h"

WakeLockTimer::WakeLockTimer()
    : m_need_pause(true)
    , m_need_stop(false)
    , m_time_base(0)
    , m_timeout(1000000000)
{
    sem_init(&m_job, 0, 0);
    sprintf(m_name, "%s", "HwcWakeLockTimer");
}

WakeLockTimer::~WakeLockTimer()
{
}

void WakeLockTimer::pause()
{
    Mutex::Autolock lock(m_mutex);
    m_need_pause = true;
    m_time_base = 0;
    HWC_LOGD("hold wakelock");
    int res = acquire_wake_lock(PARTIAL_WAKE_LOCK, m_name);
    if (res < 0)
    {
        HWC_LOGE("faile to hold wake lock: %d", res);
    }
    sem_post(&m_job);
}

void WakeLockTimer::count()
{
    Mutex::Autolock lock(m_mutex);
    m_need_pause = false;
    m_time_base = systemTime(SYSTEM_TIME_MONOTONIC);
    sem_post(&m_job);
}

void WakeLockTimer::kick()
{
    Mutex::Autolock lock(m_mutex);
    if (!m_need_pause)
    {
        m_time_base = systemTime(SYSTEM_TIME_MONOTONIC);
    }
}

void WakeLockTimer::stop()
{
    Mutex::Autolock lock(m_mutex);
    m_need_stop = true;
    sem_post(&m_job);
}

void WakeLockTimer::onFirstRef()
{
    sem_init(&m_job, 0, 0);
    run("WakeLockTimer");
}

bool WakeLockTimer::threadLoop()
{
    m_mutex.lock();
    while (true)
    {
        if (m_need_stop)
        {
            HWC_LOGD("stop wakelock timer");
            int res = release_wake_lock(m_name);
            if (res < 0)
            {
                HWC_LOGE("fail to release wake lock: %d", res);
            }
            m_mutex.unlock();
            return false;
        }

        if (m_need_pause)
        {
            HWC_LOGD("pause wakelock timer");
            m_mutex.unlock();
            sem_wait(&m_job);
            m_mutex.lock();
            HWC_LOGD("start wakelock timer");
            continue;
        }

        nsecs_t target_time = m_time_base + m_timeout;
        nsecs_t now = systemTime(SYSTEM_TIME_MONOTONIC);
        if (now < target_time)
        {
            m_mutex.unlock();
            usleep(m_timeout / 1000);
            m_mutex.lock();
        }

        if (m_need_pause)
        {
            HWC_LOGD("detect pause wakelock timer");
            continue;
        }
        now = systemTime(SYSTEM_TIME_MONOTONIC);
        target_time = m_time_base + m_timeout;
        if (now >= target_time)
        {
            HWC_LOGD("release wakelock");
            int res = release_wake_lock(m_name);
            m_need_pause = true;
            if (res < 0)
            {
                HWC_LOGE("fail to release wake lock: %d", res);
            }
        }
    }

    m_mutex.unlock();
    return false;
}
