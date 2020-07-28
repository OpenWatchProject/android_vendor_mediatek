#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>

#include "sys_def.h"
#include "os/api_btimer.h"
#include "os/api_log.h"

#ifdef VOLTE_UNIT_TEST
/* Valgrind reserves SIGRTMAX (SIGRT32) in unit test enivironment */
#define TIMER_SIGNAL (SIGRTMAX-1)
#else
#define TIMER_SIGNAL SIGRTMAX
#endif /* VOLTE_UNIT_TEST */

struct ctimer{
         int used;
     timer_t timerid;
        void (*expiration_func)(void *arg);
        void *arg;
       pid_t pid;
    unsigned initial_time;
    unsigned reschedule_time;
    unsigned active;
};

static pthread_once_t once_control = PTHREAD_ONCE_INIT;

static void timer_enable(ctimer_t* o, int enable);

unsigned int TIME_Size() {
    return sizeof(struct ctimer);
}


//handle suspend event
static void sig_handler(int signo, siginfo_t *siginfo, void *priv)
{
    (void)signo; /* unused */
    (void)priv;  /* unused */
    ctimer_t *o = (ctimer_t *)siginfo->si_value.sival_ptr;
    o->expiration_func(o->arg);
    o->active = 0;
}

static void time_init(){
        //init signal handler
        struct sigaction action;

    memset(&action, 0, sizeof(action));
        sigemptyset(&action.sa_mask);
        action.sa_flags = SA_SIGINFO | SA_RESTART;
        action.sa_sigaction = sig_handler;

        sigaction(TIMER_SIGNAL, &action, NULL);
}

/*
#include <stdio.h>
int TIME_Init2(ctimer_t* o,
        char* name,
        void (*expiration_func)(void* argument),
        void* arg,
        unsigned initial_time,
        unsigned reschedule_time, char *file, int line)
*/
int TIME_Init(ctimer_t* o,
        char* name,
        void (*expiration_func)(void* argument),
        void* arg,
        unsigned initial_time,
        unsigned reschedule_time)
{
    int ret = 0;

    (void)name; /* unused */

    //once initialization
    pthread_once(&once_control, time_init);

    o->expiration_func = expiration_func;
    o->arg = arg;
    o->initial_time = initial_time;
    o->reschedule_time = reschedule_time;
    o->active = 1;
    o->pid    = getpid();
    o->timerid = 0;
    o->used    = 0;

    struct sigevent evp;
    (void)memset(&evp, 0, sizeof(evp));
    evp.sigev_notify = SIGEV_SIGNAL;
    evp.sigev_signo = TIMER_SIGNAL;
    evp.sigev_value.sival_ptr = o;

    ret = timer_create(CLOCK_MONOTONIC, &evp, &o->timerid);
    if (ret < 0) {
        ErrMsg("o = %p, timerid = %d, ret = %d, errno = %d (%s)", o, (intptr_t)o->timerid, ret, errno, strerror(errno));
        return -1;
    }
    o->used = 1;
    timer_enable(o, 1);

    return 0;
}

static void timer_enable(ctimer_t* o, int enable){
    struct itimerspec timeout;

    if (!o || o->used == 0) {
        return;
    }

    memset(&timeout, 0, sizeof(timeout));

    if (enable) {
        timeout.it_value.tv_sec  = o->initial_time / HZ;
        timeout.it_value.tv_nsec = 1000000 * (o->initial_time % HZ);
        // timeout.it_value.tv_nsec = 1000000000/HZ*(o->initial_time % HZ);
        if (o->reschedule_time > 0) {
            timeout.it_interval.tv_sec = o->reschedule_time / HZ;
            timeout.it_interval.tv_nsec = 1000000 * (o->reschedule_time % HZ);
            // timeout.it_interval.tv_nsec = 1000000000/HZ*(o->reschedule_time % HZ);
        }
    }
    timer_settime(o->timerid, 0, &timeout, NULL);
}

//int TIME_Dest2(ctimer_t* o, char *file, int line)
int TIME_Dest(ctimer_t* o)
{
    if (o) {
        if (o->used) {
            timer_delete(o->timerid);
            o->timerid = 0;
            o->used = 0;
        } else {
            ErrMsg("o->timerid is NULL, o = %p", o);
        }
        o->active = 0;
    } else {
        ErrMsg("o is NULL");
    }
    return 0;
}

int TIME_Deactivate(ctimer_t* o)
{
    if (o) {
        timer_enable(o, 0);
        o->active = 0;
    }
    return 0;
}

int TIME_Ctrl(ctimer_t*o, int req, ...)
{
    switch (req) {
        case 1 :
            return (int)(o->active);
        case 2:
            return (int)(o->pid);
    }
    return -1;
}
