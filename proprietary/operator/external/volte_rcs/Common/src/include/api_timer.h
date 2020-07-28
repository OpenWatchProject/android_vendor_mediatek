#ifndef __ROI_API_TIMER_H__
#define __ROI_API_TIMER_H__

#include <time.h>
#include "rcs_volte_core_interface.h"


#define TIMER_1_SEC        (1000)

struct ctimer{
         int used;
     timer_t timerid;
       pid_t pid;
    unsigned initial_time;
    unsigned reschedule_time;
    unsigned active;
};

typedef struct ctimer   ctimer_t;
typedef struct _timer_list_ {
    VoLTE_Event_Timer_t *timer;
    ctimer_t *ctimer;
} Timer_List_t;

void * TIMER_Create(void (*expiration_func)(sigval_t sigval), void* event, unsigned timeout);

int TIMER_Free(void *timer);

#endif /* __ROI_API_TIMER_H__ */

