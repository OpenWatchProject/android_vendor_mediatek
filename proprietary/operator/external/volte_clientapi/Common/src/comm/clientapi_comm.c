#include <dlfcn.h>
#include "debug.h"
#include "api_clientapi_mem.h"
#include "api_clientapi_msgq.h"
#include "clientapi_comm.h"

static void             *g_message_queue = 0;
static CLIENTAPI_COMM_Send_fn      g_send_callback[EventSource_MAX] = {0};

int (*volte_sip_info_free)(VoLTE_Stack_Sip_Info_t*);
int (*volte_sip_stack_init)();
void *(*_volte_sip_stack_create)(VoLTE_Stack_Channel_Info_t*, VoLTE_Stack_Ua_Capability_t*, VoLTE_Stack_Callback_t*, char*, int);
int (*volte_sip_stack_destroy)(void*);
int (*volte_sip_stack_send_sip)(void *,VoLTE_Stack_Sip_Info_t *);
int (*volte_sip_stack_send_msg)(void *,VoLTE_Stack_Message_t *);
int (*volte_sip_stack_reg_state)(void *);
int (*volte_sip_stack_reg_bind)(void *, int );
int (*volte_sip_stack_reg_capability)(void *, int , char *);
int (*volte_sip_stack_restore)(void *);
int (*Rule_Capability_Init)(VoLTE_Stack_Ua_Capability_t*);
int (*Rule_Capability_Deinit)(VoLTE_Stack_Ua_Capability_t*);
int (*Rule_Level0_Set)(VoLTE_Stack_Ua_Capability_t*, char*, char*);
int (*Rule_Level1_Set)(VoLTE_Stack_Ua_Capability_t*, char*);
int (*Rule_Level2_Set)(VoLTE_Stack_Ua_Capability_t*, const int , const int , char*);
int (*Rule_Level3_Set)(VoLTE_Stack_Ua_Capability_t*, char*);
int (*Channel_Send)(void *channel, Channel_Data_t *data);
int (*volte_sip_stack_ind_external_ua_state)(void *, int, int, int);

//initialize the CLIENTAPI_COMM
int CLIENTAPI_COMM_Init(void *mesgq) {
    if (!mesgq) {
        ErrMsg("The message queue is NULL");
        return -1;
    }
    g_message_queue = mesgq;
    return 0;
}

//set the send function callback
int CLIENTAPI_COMM_SetSend(int source, CLIENTAPI_COMM_Send_fn func) {
    if (source < 0 || source >= EventSource_MAX) {
        ErrMsg("Invalid source : %d", source);
        return -1;
    }
    DbgMsg("Update the send_callback[%d] = %p", source, func);
    g_send_callback[source] = func;

    return 0;
}

//send CLIENTAPI_event_msg
int CLIENTAPI_COMM_Send(CLIENTAPI_EventMsg_t *event) {

    if (!event) {
        ErrMsg("event is NULL");
        CLIENTAPI_COMM_SendError(EventSource_Unknown, CommErrNo_SendFail);
        return -1;
    }

    if (event->source < 0 || event->source >= EventSource_MAX) {
        ErrMsg("Invalid source id : %d", event->source);
        CLIENTAPI_COMM_SendError(EventSource_Unknown, CommErrNo_SendFail);
        return -2;
    }

    if (event->request_id == CommErrorRequestID) {
        ErrMsg("Can't use the request_id 0");
        CLIENTAPI_COMM_SendError(event->source, CommErrNo_SendFail);
        return -3;
    }

    if(!g_send_callback[event->source]){
        ErrMsg("g_send_callback[%d] is empty", event->source);
        return -1;
    }

#if 1
    DbgMsg("Event = %d send to adapter, source = %d (%p)", event->request_id, event->source, g_send_callback[event->source]);
#endif

    if (g_send_callback[event->source]) {
        int ret = 0;
        ret = g_send_callback[event->source](event);
        if (ret != 0) {
            ErrMsg("Can't send the message");
           // CLIENTAPI_COMM_SendError(event->source, CommErrNo_SendFail);
           //  return -4;
        }
        return ret;
    }
    return -100;
}

//RECEIVE THE CLIENTAPI_EVENT
int CLIENTAPI_COMM_Recv(CLIENTAPI_EventMsg_t *event) {
    if (!event) {
        ErrMsg("event is NULL");
        return -1;
    }

    if (!g_message_queue) {
        ErrMsg("The message queue is NULL");
        return -1;
    }
    return  CLIENTAPI_CMSGQ_Put(g_message_queue, event);
}

//send error message
int CLIENTAPI_COMM_SendError(int source, int error_no) {
    CLIENTAPI_EventMsg_t *event = 0;
    CommErrMsg_t *err_msg = 0;

    if (!g_message_queue) {
        ErrMsg("The message queue is NULL");
        return -1;
    }

    event = (CLIENTAPI_EventMsg_t *)CLIENTAPI_COMM_GetEvent(sizeof(CommErrMsg_t));
    if (!event) {
        ErrMsg("Can't allocat the event");
        return -1;
    }

    event->source = source;
    event->request_id = CommErrorRequestID;
    event->data_len = sizeof(CommErrMsg_t);
    err_msg = (CommErrMsg_t *)(event->data);
    err_msg->error_no = error_no;

    return  CLIENTAPI_CMSGQ_Put(g_message_queue, event);
}

void *
#ifdef MEM_FREE_DEBUG
_CLIENTAPI_COMM_GetEvent(int size, char *file, int line) {
    void *ptr = (void *)API_get_mem(size+sizeof(CLIENTAPI_EventMsg_t), file, line);
#else
CLIENTAPI_COMM_GetEvent(int size) {
    void *ptr = (void *)MEM_Allocate(size+sizeof(CLIENTAPI_EventMsg_t));
#endif

    if (ptr) {
        memset(ptr, 0, size+sizeof(CLIENTAPI_EventMsg_t));
        return ptr;
    }

    return 0;
}

//free event
int CLIENTAPI_COMM_FreeEvent(CLIENTAPI_EventMsg_t *event) {
    MEM_Free(event);
    return 0;
}

void *getDlopenHandler(const char *libPath) {
    void *dlHandler;
    dlHandler = dlopen(libPath, RTLD_NOW);
    if (dlHandler == NULL) {
        ErrMsg("dlopen failed: %s", dlerror());
        return NULL;
    }
    return dlHandler;
}

void *getAPIbyDlopenHandler(void *dlHandler, const char *apiName){
    if (dlHandler == NULL) {
        ErrMsg("dlopen Handler is null");
        exit(EXIT_FAILURE);
    }
    void *outputFun;
    outputFun = dlsym(dlHandler, apiName);
    if (outputFun == NULL) {
        ErrMsg("%s is not defined or exported: %s", apiName, dlerror());
        exit(EXIT_FAILURE);
    }
    return outputFun;
}

void *getCompatibleAPIbyDlopenHandler(
        void *dlHandler, const char *apiName){
    if (dlHandler == NULL) {
        ErrMsg("dlopen Handler is null");
    }
    void *outputFun;
    outputFun = dlsym(dlHandler, apiName);
    if (outputFun == NULL) {
        ErrMsg("%s is not defined or exported: %s", apiName, dlerror());
    }
    return outputFun;
}


void freeDlopenHandler (void *dlHandler) {
    if (dlHandler != NULL) {
        DbgMsg("[%s] Free dlopen handler",__FUNCTION__);
        dlclose(dlHandler);
        dlHandler = NULL;
    }
}

int initVolteCoreAPIs() {

    freeDlopenHandler(gVolteCoreHandler);

    gVolteCoreHandler = getDlopenHandler("libvolte_core_shr.so");

    if (gVolteCoreHandler == NULL) {
        DbgMsg("Dlopen libvolte_core_shr.so fail");
        return 1;
    }

    _volte_sip_stack_create = (void *(*)
            (VoLTE_Stack_Channel_Info_t *, VoLTE_Stack_Ua_Capability_t *, VoLTE_Stack_Callback_t *, char *, int))
            getAPIbyDlopenHandler(gVolteCoreHandler, "_volte_sip_stack_create");

    volte_sip_stack_send_sip = (int (*)(void *, VoLTE_Stack_Sip_Info_t *))
            getAPIbyDlopenHandler(gVolteCoreHandler, "volte_sip_stack_send_sip");

    volte_sip_info_free = (int (*)(VoLTE_Stack_Sip_Info_t *))
            getAPIbyDlopenHandler(gVolteCoreHandler, "volte_sip_info_free");

    volte_sip_stack_reg_state = (int (*)(void *))
            getAPIbyDlopenHandler(gVolteCoreHandler, "volte_sip_stack_reg_state");

    volte_sip_stack_reg_bind = (int (*)(void *, int))
            getAPIbyDlopenHandler(gVolteCoreHandler, "volte_sip_stack_reg_bind");

    volte_sip_stack_reg_capability = (int(*)(void *, int, char *))
            getAPIbyDlopenHandler(gVolteCoreHandler, "volte_sip_stack_reg_capability");

    volte_sip_stack_destroy = (int(*)(void *))
            getAPIbyDlopenHandler(gVolteCoreHandler, "volte_sip_stack_destroy");

    volte_sip_stack_init = (int(*)(void))
            getAPIbyDlopenHandler(gVolteCoreHandler, "volte_sip_stack_init");

    Rule_Capability_Init = (int(*)(VoLTE_Stack_Ua_Capability_t *))
            getAPIbyDlopenHandler(gVolteCoreHandler, "Rule_Capability_Init");

    Rule_Level0_Set = (int(*)(VoLTE_Stack_Ua_Capability_t *, char *, char *))
            getAPIbyDlopenHandler(gVolteCoreHandler, "Rule_Level0_Set");

    Rule_Level2_Set = (int(*)(VoLTE_Stack_Ua_Capability_t *, const int, const int, char *))
            getAPIbyDlopenHandler(gVolteCoreHandler, "Rule_Level2_Set");

    Rule_Level3_Set = (int(*)(VoLTE_Stack_Ua_Capability_t *, char *))
            getAPIbyDlopenHandler(gVolteCoreHandler, "Rule_Level3_Set");

    Rule_Capability_Deinit = (int(*)(VoLTE_Stack_Ua_Capability_t *))
            getAPIbyDlopenHandler(gVolteCoreHandler, "Rule_Capability_Deinit");

    Channel_Send = (int(*)(void *, Channel_Data_t *))
            getAPIbyDlopenHandler(gVolteCoreHandler, "Channel_Send");

    volte_sip_stack_restore = (int (*)(void *))
            getCompatibleAPIbyDlopenHandler(gVolteCoreHandler, "volte_sip_stack_restore");

    DbgMsg("init VoLTE Stack APIs successfully");
    return 0;


}

