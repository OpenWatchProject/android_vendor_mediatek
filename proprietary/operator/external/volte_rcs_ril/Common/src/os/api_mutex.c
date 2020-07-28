#include <pthread.h>
#include "os/api_mutex.h"

struct cmutex{
    pthread_mutex_t mutex;
};

unsigned MUTX_Size(){
    return sizeof(struct cmutex);
}

int MUTX_Init(cmutex_t* o, char* name)
{
    pthread_mutex_init(&o->mutex, NULL);
    return 0;
}

int MUTX_Dest(cmutex_t* o)
{
    pthread_mutex_destroy(&o->mutex);
    return 0;
}

int MUTX_Get(cmutex_t* o, cwait_t wait)
{
    switch(wait){
    case WAIT_NOWAIT:
        if(pthread_mutex_trylock(&o->mutex)){
            return -1;
        }
        break;
    case WAIT_FOREVER:
        pthread_mutex_lock(&o->mutex);
        break;
    default:
        return -1;
    }
    return 0;
}

int MUTX_Put(cmutex_t* o)
{
    pthread_mutex_unlock(&o->mutex);
    return 0;
}

int MUTX_Ctrl(cmutex_t* o)
{
    return 0;
}

#if defined _UNIT_

#include <base/utest.h>

int main(void)
{
    UTST_Add(SYSS_UTReset);
    UTST_Run(0);
    return 0;
}

#endif

