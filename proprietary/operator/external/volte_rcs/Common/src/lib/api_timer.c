#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <errno.h>

#include "api_timer.h"
#include "api_rcs_mem.h"
#include "debug.h"

#define HZ (1000)

static int boottime_alarm_support = 1; /*assume supported */
static void CTIME_cap_set (int support);

unsigned int CTIME_Size() {
    return sizeof(struct ctimer);
}

static void CTIME_cap_set (int support)
{
    SysMsg("set boottime_alarm_support = %d", support);
    boottime_alarm_support = support;
}

static void ctimer_enable(ctimer_t* o, int enable){
    struct itimerspec timeout;
    /* CAUTION: timeout=0 will cause cancel event when using CLOCK_READL_TIME */
    int at_least = 0;


    if (!o || o->used == 0) {
        return;
    }

    memset(&timeout, 0, sizeof(timeout));


    if (!o->initial_time) {
        at_least = 1;
    }

    if (enable) {
        timeout.it_value.tv_sec = o->initial_time / HZ;
        timeout.it_value.tv_nsec = 1000000 * (o->initial_time % HZ) + at_least;
        if (o->reschedule_time) {
            timeout.it_interval.tv_sec = o->reschedule_time / HZ;
            timeout.it_interval.tv_nsec = 1000000 * (o->reschedule_time % HZ);
        }
    }
    timer_settime(o->timerid, 0, &timeout, NULL);
}

static int CTIME_Init(ctimer_t* o, void (*expiration_func)(sigval_t sigval), void* arg,
    unsigned initial_time, unsigned reschedule_time) {
    struct sigevent evp;
    int ret = 0;

    memset(&evp, 0, sizeof(evp));

    o->initial_time = initial_time;
    o->reschedule_time = reschedule_time;
    o->timerid = 0;
    o->used    = 0;

    evp.sigev_notify = SIGEV_THREAD;
    evp.sigev_notify_function = expiration_func;
    evp.sigev_value.sival_ptr = arg;

    if (boottime_alarm_support) {
        ret = timer_create(CLOCK_BOOTTIME_ALARM, &evp, &o->timerid);
        if ( ret < 0 ) {
            DbgMsg("try CLOCK_BOOTTIME_ALARM: o = %p, timerid = %d, ret = %d, errno = %d (%s)",
                    o, (int)(intptr_t)o->timerid, ret, errno, strerror(errno));
            SysMsg("disable boottime_alarm_support");
            CTIME_cap_set (0);
        }
    }

    if ( !boottime_alarm_support) {
        ret = timer_create(CLOCK_BOOTTIME, &evp, &o->timerid);
    }

    if (ret < 0) {
        ErrMsg("try CLOCK_BOOTTIME: o = %p, timerid = %d, ret = %d, errno = %d (%s)",
                o, (int)(intptr_t)o->timerid, ret, errno, strerror(errno));
        return -1;
    }
    o->used = 1;
    ctimer_enable(o, 1);

    return 0;
}


void *TIMER_Create(void (*expiration_func)(sigval_t sigval), void* event, unsigned timeout) {
    ctimer_t* o = 0;
    int ret = 0;

    o = MEM_Allocate(CTIME_Size());
    if (!o) {
        return 0;
    }

    ret =  CTIME_Init(o, expiration_func, event, timeout, 0);
    if (ret != 0) {
        MEM_Free(o);
        return 0;
    }

    return o;
}

int TIMER_Free(void *timer) {
    ctimer_t* o = (ctimer_t *)timer;
    if (o) {
        if (o->used) {
            ctimer_enable(o, 0);
            timer_delete(o->timerid);
        }
        o->used    = 0;

        MEM_Free(o);
    }
    return 0;
}

