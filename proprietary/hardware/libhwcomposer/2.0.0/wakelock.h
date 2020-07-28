#ifndef HWC_WAKELOCK_H_
#define HWC_WAKELOCK_H_

#include <semaphore.h>
#include <utils/threads.h>
#include <utils/Mutex.h>

using namespace android;

class WakeLockTimer : public Thread
{
public:
    WakeLockTimer();
    ~WakeLockTimer();

    // pause timer
    void pause();

    // start timer
    void count();

    // extend timer
    void kick();

    // stop timer
    void stop();

private:
    virtual void onFirstRef();

    virtual bool threadLoop();

    char m_name[64];

    Mutex m_mutex;

    sem_t m_job;

    bool m_need_pause;

    bool m_need_stop;

    nsecs_t m_time_base;

    nsecs_t m_timeout;
};

#endif
