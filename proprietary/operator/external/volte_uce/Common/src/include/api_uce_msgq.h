#ifndef __LIB_API_UCE_CMSGQ_H__
#define __LIB_API_UCE_CMSGQ_H__

enum
{
    UCE_CMSGQ_FLAG_NOWAIT = 0,
    UCE_CMSGQ_FLAG_WAIT
};

enum
{
    UCE_CMSGQ_CHECK_CONTINUE = 1,
    UCE_CMSGQ_CHECK_DROP,
    UCE_CMSGQ_CHECK_RETURN,
};

#define MSG_DEFAULT_PRIORITY        (10)

typedef int (*RCS_CMSGQ_callback) (void *new_msg, void *msg);

void *UCE_CMSGQ_Init(int size, char *name, RCS_CMSGQ_callback func);

int UCE_CMSGQ_Put(void *msgq, void *msg);

int UCE_CMSGQ_Put2(void *msgq, void *msg, int priority);

#ifdef DELAY_TIMER_ENABLED
int UCE_CMSGQ_DelayPut(void *msgq, void *msg, int delay, int priority);
#endif

void *UCE_CMSGQ_Get(void *msgq, int flag);

void *UCE_CMSGQ_GetMsg(void *msgq, RCS_CMSGQ_callback func, void *data);

int UCE_CMSGQ_Free(void *msgq);

void *UCE_CMSGQ_MsgAllocate(int size);

int UCE_CMSGQ_MsgFree(void *ptr);

#endif /* __LIB_API_UCE_CMSGQ_H__ */

