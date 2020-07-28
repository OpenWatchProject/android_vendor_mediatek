#ifndef __LIB_API_RCS_CMSGQ_H__
#define __LIB_API_RCS_CMSGQ_H__

enum
{
    RCS_CMSGQ_FLAG_NOWAIT = 0,
    RCS_CMSGQ_FLAG_WAIT
};

enum
{
    RCS_CMSGQ_CHECK_CONTINUE = 1,
    RCS_CMSGQ_CHECK_DROP,
    RCS_CMSGQ_CHECK_RETURN,
};

#define MSG_DEFAULT_PRIORITY        (10)

typedef int (*RCS_CMSGQ_callback) (void *new_msg, void *msg);

void *RCS_CMSGQ_Init(int size, char *name, RCS_CMSGQ_callback func);

int RCS_CMSGQ_Put(void *msgq, void *msg);

int RCS_CMSGQ_Put2(void *msgq, void *msg, int priority);

#ifdef DELAY_TIMER_ENABLED
int RCS_CMSGQ_DelayPut(void *msgq, void *msg, int delay, int priority);
#endif 

void *RCS_CMSGQ_Get(void *msgq, int flag);

void *RCS_CMSGQ_GetMsg(void *msgq, RCS_CMSGQ_callback func, void *data);

int RCS_CMSGQ_Free(void *msgq);

void *RCS_CMSGQ_MsgAllocate(int size);

int RCS_CMSGQ_MsgFree(void *ptr);

#endif /* __LIB_API_RCS_CMSGQ_H__ */ /* __LIB_API_RCS_RCS_CMSGQ_H__ */

