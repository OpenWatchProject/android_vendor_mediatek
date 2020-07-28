#ifndef __LIB_API_CLIENTAPI_CMSGQ_H__
#define __LIB_API_CLIENTAPI_CMSGQ_H__

enum
{
    CLIENTAPI_CMSGQ_FLAG_NOWAIT = 0,
    CLIENTAPI_CMSGQ_FLAG_WAIT
};

enum
{
    CLIENTAPI_CMSGQ_CHECK_CONTINUE = 1,
    CLIENTAPI_CMSGQ_CHECK_DROP,
    CLIENTAPI_CMSGQ_CHECK_RETURN,
};

#define MSG_DEFAULT_PRIORITY        (10)

typedef int (*CLIENTAPI_CMSGQ_callback) (void *new_msg, void *msg);

void *CLIENTAPI_CMSGQ_Init(int size, char *name, CLIENTAPI_CMSGQ_callback func);

int CLIENTAPI_CMSGQ_Put(void *msgq, void *msg);

int CLIENTAPI_CMSGQ_Put2(void *msgq, void *msg, int priority);

#ifdef DELAY_TIMER_ENABLED
int CLIENTAPI_CMSGQ_DelayPut(void *msgq, void *msg, int delay, int priority);
#endif

void *CLIENTAPI_CMSGQ_Get(void *msgq, int flag);

void *CLIENTAPI_CMSGQ_GetMsg(void *msgq, CLIENTAPI_CMSGQ_callback func, void *data);

int CLIENTAPI_CMSGQ_Free(void *msgq);

void *CLIENTAPI_CMSGQ_MsgAllocate(int size);

int CLIENTAPI_CMSGQ_MsgFree(void *ptr);

#endif /* __LIB_API_CLIENTAPI_CMSGQ_H__ */

