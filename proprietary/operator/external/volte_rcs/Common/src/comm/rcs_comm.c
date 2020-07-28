#include <string.h>
#include "debug.h"
#include "api_rcs_mem.h"
#include "api_rcs_msgq.h"
#include "rcs_comm.h"

static void             *g_message_queue = 0;
static RCS_COMM_Send_fn      g_send_callback[EventSource_MAX] = {0};


// initialize the RCS_COMM
int RCS_COMM_Init(void *mesgq) {
    if (!mesgq) {
        ErrMsg("The message queue is NULL");
        return -1;
    }
    g_message_queue = mesgq;
    return 0;
}

// set the send function callback
int RCS_COMM_SetSend(int source, RCS_COMM_Send_fn func) {
    if (source < 0 || source >= EventSource_MAX) {
        ErrMsg("Invalid source : %d", source);
        return -1;
    }

    DbgMsg("Update the send_callback[%d] = %p", source, func);
    g_send_callback[source] = func;

    return 0;
}


// send RCS_event_msg
int RCS_COMM_Send(RCS_EventMsg_t *event) {

    if (!event) {
        ErrMsg("event is NULL");
        RCS_COMM_SendError(EventSource_Unknown, CommErrNo_SendFail);
        return -1;
    }

    if (event->source < 0 || event->source >= EventSource_MAX) {
        ErrMsg("Invalid source id : %d", event->source);
        RCS_COMM_SendError(EventSource_Unknown, CommErrNo_SendFail);
        return -2;
    }

    if (event->request_id == CommErrorRequestID) {
        ErrMsg("Can't use the request_id 0");
        RCS_COMM_SendError(event->source, CommErrNo_SendFail);
        return -3;
    }

    if(!g_send_callback[event->source]){
        ErrMsg("g_send_callback[%d] is empty", event->source);
        return -1;
    }

    DbgMsg("Event = %d send to adapter, source = %d (%p)", event->request_id, event->source, g_send_callback[event->source]);
    if (g_send_callback[event->source]) {
        int ret = 0;
        ret = g_send_callback[event->source](event);
        if (ret != 0) {
            ErrMsg("Can't send the message");
           // RCS_COMM_SendError(event->source, CommErrNo_SendFail);
           //  return -4;
        }
        return ret;
    }

    return -100;
}


// RECEIVE THE RCS_EVENT
int RCS_COMM_Recv(RCS_EventMsg_t *event) {
    if (!event) {
        ErrMsg("event is NULL");
        return -1;
    }

    if (!g_message_queue) {
        ErrMsg("The message queue is NULL");
        return -1;
    }
    return  RCS_CMSGQ_Put(g_message_queue, event);
}


// send error message
int RCS_COMM_SendError(int source, int error_no) {
    RCS_EventMsg_t *event = 0;
    CommErrMsg_t *err_msg = 0;

    if (!g_message_queue) {
        ErrMsg("The message queue is NULL");
        return -1;
    }

    event = RCS_COMM_GetEvent(sizeof(CommErrMsg_t));
    if (!event) {
        ErrMsg("Can't allocat the event");
        return -1;
    }

    event->source = source;
    event->request_id = CommErrorRequestID;
    event->data_len = sizeof(CommErrMsg_t);
    err_msg = (void *)(event->data);
    err_msg->error_no = error_no;

    return  RCS_CMSGQ_Put(g_message_queue, event);
}


void *
#ifdef MEM_FREE_DEBUG
_RCS_COMM_GetEvent(int size, char *file, int line) {
    void *ptr = (void *)API_get_mem(size+sizeof(RCS_EventMsg_t), file, line);
#else
RCS_COMM_GetEvent(int size) {
    void *ptr = (void *)MEM_Allocate(size+sizeof(RCS_EventMsg_t));
#endif

    if (ptr) {
        memset(ptr, 0, size+sizeof(RCS_EventMsg_t));
        return ptr;
    }

    return 0;
}

// free event
int RCS_COMM_FreeEvent(RCS_EventMsg_t *event) {
    MEM_Free(event);
    return 0;
}