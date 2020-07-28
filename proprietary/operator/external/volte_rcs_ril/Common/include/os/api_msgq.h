#ifndef __SYSSERV_MSGQ_H__
#define __SYSSERV_MSGQ_H__

enum {
    MSGQ_FLAG_NOWAIT = 0,
    MSGQ_FLAG_WAIT
};

enum {
    MSGQ_CHECK_CONTINUE = 1,
    MSGQ_CHECK_DROP,
    MSGQ_CHECK_RETURN,
};

#define MSG_DEFAULT_PRIORITY        (10)

typedef int (*MSGQ_callback) (void *new_msg, void *msg);

void *MSGQ_Init(int size, char *name, MSGQ_callback func);

int MSGQ_Put(void *msgq, void *msg);

int MSGQ_Put2(void *msgq, void *msg, int priority);

//#define DELAY_TIMER_ENABLED
#ifdef DELAY_TIMER_ENABLED
int MSGQ_DelayPut(void *msgq, void *msg, int delay, int priority);
void * MSGQ_DelayCancel(void *msg_obj);
#endif

void *MSGQ_Get(void *msgq, int flag);

void *MSGQ_GetMsg(void *msgq, MSGQ_callback func, void *data);

int MSGQ_Free(void *msgq);

#endif /* __SYSSERV_MSGQ_H__ */
