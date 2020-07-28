#ifndef __LIB_UCE_COMM_H__
#define __LIB_UCE_COMM_H__

#include "rcs_volte_core_interface.h"

/* event coming from which source */
enum UCE_EventSource_e
{
    EventSource_Unknown = 0,
    EventSource_UCE_PROXY_ADAPTATION,
    EventSource_IMS_DISPATCHER,
    EventSource_MAX,
};

//structure for sending uce_event message.
//its an internal format of message
typedef struct _UCE_EventMsg_t
{
    int source;
    int request_id;
    int data_len;
    char data[0];
} UCE_EventMsg_t;

#define CommErrorRequestID      (0)

enum CommErrNo_e
{
    CommErrNo_Connected = 0,
    CommErrNo_Disconnected = 1,
    CommErrNo_SendFail = 2,
};

typedef struct _CommErrMsg_t
{
    int error_no;
} CommErrMsg_t;

//initilaize the UCE_communctaion
//initlizes the internal queue
int UCE_COMM_Init(void *mesgq);

/* UCE Comm send function pointer */
typedef int (*UCE_COMM_Send_fn) (UCE_EventMsg_t *event);

/* set the callback fro sending message to uce proxy adapter */
int UCE_COMM_SetSend(int source, UCE_COMM_Send_fn func);

/**
  send the event to the UCE_PROXY_ADAPTER VIA SOCKET
  */
int UCE_COMM_Send(UCE_EventMsg_t *event);

/* SEND ERROR */
int UCE_COMM_SendError(int source, int error_no);

/**
  RECEIVE MESSAGE FROM UCE_PROXY_ADAPTER
  */

int UCE_COMM_Recv(UCE_EventMsg_t *event);

#ifdef MEM_FREE_DEBUG
#define UCE_COMM_GetEvent(s)    _UCE_COMM_GetEvent((s), __FILE__, __LINE__)
void *_UCE_COMM_GetEvent(int size, char *file, int line);
#else /* MEM_FREE_DEBUG */
void *UCE_COMM_GetEvent(int size);
#endif /* MEM_FREE_DEBUG */

/* free the UCE_comm_event */
int UCE_COMM_FreeEvent(UCE_EventMsg_t *event);

/* void RCS_COMM_TimerExpiry(void *data); */

/** Dlopen handling function **/
void *gVolteCoreHandler;
void *gRcsVolteCoreHandler;
void *getDlopenHandler(const char *libPath);
void *getAPIbyDlopenHandler(void *dlHandler, const char *apiName);
void *getCompatibleAPIbyDlopenHandler(void *dlHandler, const char *apiName);
void freeDlopenHandler(void *dlHandler);
int initVolteCoreAPIs();
int initRcsVolteCoreAPIs();

#endif /* __LIB_UCE_COMM_H__ */

