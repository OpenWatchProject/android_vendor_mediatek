#ifndef __LIB_CLIENTAPI_COMM_H__
#define __LIB_CLIENTAPI_COMM_H__

#include "volte_core_interface.h"
/* event coming from which source */
enum CLIENTAPI_EventSource_e
{
    EventSource_Unknown = 0,
    EventSource_CLIENTAPI_PROXY_ADAPTATION,
    EventSource_IMS_DISPATCHER,
    EventSource_MAX,
};

//structure for sending CLIENTAPI_event message.
//its an internal format of message
typedef struct _CLIENTAPI_EventMsg_t
{
    int source;
    int request_id;
    int data_len;
    char data[0];
} CLIENTAPI_EventMsg_t;

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

//initilaize the CLIENTAPI_communctaion
//initlizes the internal queue
int CLIENTAPI_COMM_Init(void *mesgq);

/* CLIENTAPI Comm send function pointer */
typedef int (*CLIENTAPI_COMM_Send_fn) (CLIENTAPI_EventMsg_t *event);

/* set the callback fro sending message to CLIENTAPI proxy adapter */
int CLIENTAPI_COMM_SetSend(int source, CLIENTAPI_COMM_Send_fn func);

/**
  send the event to the CLIENTAPI_PROXY_ADAPTER VIA SOCKET
  */
int CLIENTAPI_COMM_Send(CLIENTAPI_EventMsg_t *event);

/* SEND ERROR */
int CLIENTAPI_COMM_SendError(int source, int error_no);

/**
  RECEIVE MESSAGE FROM CLIENTAPI_PROXY_ADAPTER
  */

int CLIENTAPI_COMM_Recv(CLIENTAPI_EventMsg_t *event);

#ifdef MEM_FREE_DEBUG
#define CLIENTAPI_COMM_GetEvent(s)    _CLIENTAPI_COMM_GetEvent((s), __FILE__, __LINE__)
void *_CLIENTAPI_COMM_GetEvent(int size, char *file, int line);
#else /* MEM_FREE_DEBUG */
void *CLIENTAPI_COMM_GetEvent(int size);
#endif /* MEM_FREE_DEBUG */

/* free the CLIENTAPI_comm_event */
int CLIENTAPI_COMM_FreeEvent(CLIENTAPI_EventMsg_t *event);

/* void clientapi_COMM_TimerExpiry(void *data); */

/** Dlopen handling function **/
void *gVolteCoreHandler;
void *getDlopenHandler(const char *libPath);
void *getAPIbyDlopenHandler(void *dlHandler, const char *apiName);
void *getCompatibleAPIbyDlopenHandler(void *dlHandler, const char *apiName);
void freeDlopenHandler(void *dlHandler);
int initVolteCoreAPIs();

#endif /* __LIB_CLIENTAPI_COMM_H__ */

