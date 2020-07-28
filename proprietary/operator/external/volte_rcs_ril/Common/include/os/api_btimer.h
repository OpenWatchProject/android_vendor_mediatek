#ifndef __VOLTE_OS_API_TIMER_H__
#define __VOLTE_OS_API_TIMER_H__

typedef struct ctimer   ctimer_t;

int TIME_Init(ctimer_t* o,
        char* name,
        void (*expiration_func)(void* argument),
        void* arg,
        unsigned initial_time,
        unsigned reschedule_time);

int TIME_Dest(ctimer_t* o);

int TIME_Deactivate(ctimer_t* o);

unsigned int TIME_Size();

int TIME_Ctrl(ctimer_t*o, int req, ...);

#endif /* __VOLTE_OS_API_TIMER_H__ */
