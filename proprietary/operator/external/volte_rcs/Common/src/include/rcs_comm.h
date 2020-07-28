#ifndef __LIB_RCS_COMM_H__
#define __LIB_RCS_COMM_H__

#include "rcs_volte_core_interface.h"

/* event coming from which source */
enum RCS_EventSource_e
{
    EventSource_Unknown = 0,
    EventSource_RCS_PROXY_ADAPTATION,
    EventSource_IMS_DISPATCHER,
    EventSource_MAX,
};

//structure for sending rcs_event message.
//its an internal format of message
typedef struct _RCS_EventMsg_t
{
    int source;
    int request_id;
    int data_len;
    char data[0];
} RCS_EventMsg_t;

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

//initilaize the RCS_communctaion
//initlizes the internal queue
int RCS_COMM_Init(void *mesgq);

/* RCS Comm send function pointer */
typedef int (*RCS_COMM_Send_fn) (RCS_EventMsg_t *event);

/* set the callback fro sending message to rcs proxy adapter */
int RCS_COMM_SetSend(int source, RCS_COMM_Send_fn func);

/**
  send the event to the RCS_PROXY_ADAPTER VIA SOCKET
  */
int RCS_COMM_Send(RCS_EventMsg_t *event);

/* SEND ERROR */
int RCS_COMM_SendError(int source, int error_no);

/**
  RECEIVE MESSAGE FROM RCS_PROXY_ADAPTER
  */
int RCS_COMM_Recv(RCS_EventMsg_t *event);

#ifdef MEM_FREE_DEBUG
#define RCS_COMM_GetEvent(s)    _RCS_COMM_GetEvent((s), __FILE__, __LINE__)
void *_RCS_COMM_GetEvent(int size, char *file, int line);
#else /* MEM_FREE_DEBUG */
void *RCS_COMM_GetEvent(int size);
#endif /* MEM_FREE_DEBUG */

/* free the RCS_comm_event */
int RCS_COMM_FreeEvent(RCS_EventMsg_t *event);

/* void RCS_COMM_TimerExpiry(void *data); */

#endif /* __LIB_RCS_COMM_H__ */

