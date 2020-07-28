#include <dlfcn.h>
#include "debug.h"
#include "api_uce_mem.h"
#include "api_uce_msgq.h"
#include "uce_comm.h"

static void             *g_message_queue = 0;
static UCE_COMM_Send_fn      g_send_callback[EventSource_MAX] = {0};

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

//initialize the UCE_COMM
int UCE_COMM_Init(void *mesgq) {
    if (!mesgq) {
        ErrMsg("The message queue is NULL");
        return -1;
    }
    g_message_queue = mesgq;
    return 0;
}

//set the send function callback
int UCE_COMM_SetSend(int source, UCE_COMM_Send_fn func) {
    if (source < 0 || source >= EventSource_MAX) {
        ErrMsg("Invalid source : %d", source);
        return -1;
    }
    DbgMsg("Update the send_callback[%d] = %p", source, func);
    g_send_callback[source] = func;

    return 0;
}

//send UCE_event_msg
int UCE_COMM_Send(UCE_EventMsg_t *event) {

    if (!event) {
        ErrMsg("event is NULL");
        UCE_COMM_SendError(EventSource_Unknown, CommErrNo_SendFail);
        return -1;
    }

    if (event->source < 0 || event->source >= EventSource_MAX) {
        ErrMsg("Invalid source id : %d", event->source);
        UCE_COMM_SendError(EventSource_Unknown, CommErrNo_SendFail);
        return -2;
    }

    if (event->request_id == CommErrorRequestID) {
        ErrMsg("Can't use the request_id 0");
        UCE_COMM_SendError(event->source, CommErrNo_SendFail);
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
           // UCE_COMM_SendError(event->source, CommErrNo_SendFail);
           //  return -4;
        }
        return ret;
    }
    return -100;
}

//RECEIVE THE RCS_EVENT
int UCE_COMM_Recv(UCE_EventMsg_t *event) {
    if (!event) {
        ErrMsg("event is NULL");
        return -1;
    }

    if (!g_message_queue) {
        ErrMsg("The message queue is NULL");
        return -1;
    }
    return  UCE_CMSGQ_Put(g_message_queue, event);
}

//send error message
int UCE_COMM_SendError(int source, int error_no) {
    UCE_EventMsg_t *event = 0;
    CommErrMsg_t *err_msg = 0;

    if (!g_message_queue) {
        ErrMsg("The message queue is NULL");
        return -1;
    }

    event = (UCE_EventMsg_t *)UCE_COMM_GetEvent(sizeof(CommErrMsg_t));
    if (!event) {
        ErrMsg("Can't allocat the event");
        return -1;
    }

    event->source = source;
    event->request_id = CommErrorRequestID;
    event->data_len = sizeof(CommErrMsg_t);
    err_msg = (CommErrMsg_t *)(event->data);
    err_msg->error_no = error_no;

    return  UCE_CMSGQ_Put(g_message_queue, event);
}

void *
#ifdef MEM_FREE_DEBUG
_UCE_COMM_GetEvent(int size, char *file, int line) {
    void *ptr = (void *)API_get_mem(size+sizeof(UCE_EventMsg_t), file, line);
#else
UCE_COMM_GetEvent(int size) {
    void *ptr = (void *)MEM_Allocate(size+sizeof(UCE_EventMsg_t));
#endif

    if (ptr) {
        memset(ptr, 0, size+sizeof(UCE_EventMsg_t));
        return ptr;
    }

    return 0;
}

//free event
int UCE_COMM_FreeEvent(UCE_EventMsg_t *event) {
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

int initRcsVolteCoreAPIs() {

    freeDlopenHandler(gRcsVolteCoreHandler);

    gRcsVolteCoreHandler = getDlopenHandler("librcs_volte_core.so");

    if (gRcsVolteCoreHandler == NULL) {
        DbgMsg("Dlopen ibrcs_volte_core.so fail");
        return 1;
    }

    roi_volte_sip_info_free = (int (*)(VoLTE_Stack_Sip_Info_t*))
        getAPIbyDlopenHandler(gRcsVolteCoreHandler, "volte_sip_info_free");

    roi_volte_sip_stack_init = (int (*)())
        getAPIbyDlopenHandler(gRcsVolteCoreHandler, "volte_sip_stack_init");

    _roi_volte_sip_stack_create =
        (void *(*)(VoLTE_Stack_Channel_Info_t*,
        VoLTE_Stack_Ua_Capability_t*,
        VoLTE_Stack_Callback_t*,
        char*,
        int))
        getAPIbyDlopenHandler(gRcsVolteCoreHandler, "_volte_sip_stack_create");

    roi_volte_sip_stack_destroy = (int (*)(void*))
        getAPIbyDlopenHandler(gRcsVolteCoreHandler, "volte_sip_stack_destroy");

    roi_volte_sip_stack_send_sip = (int (*)(void *, VoLTE_Stack_Sip_Info_t *))
        getAPIbyDlopenHandler(gRcsVolteCoreHandler, "volte_sip_stack_send_sip");

    roi_volte_sip_stack_send_msg = (int (*)(void *, VoLTE_Stack_Message_t *))
        getAPIbyDlopenHandler(gRcsVolteCoreHandler, "volte_sip_stack_send_msg");

    roi_volte_sip_stack_reg_state = (int (*)(void *))
        getAPIbyDlopenHandler(gRcsVolteCoreHandler, "volte_sip_stack_reg_state");

    roi_volte_sip_stack_reg_bind = (int (*)(void *, int ))
        getAPIbyDlopenHandler(gRcsVolteCoreHandler, "volte_sip_stack_reg_bind");

    roi_volte_sip_stack_reg_capability = (int (*)(void *, int , char *))
        getAPIbyDlopenHandler(gRcsVolteCoreHandler, "volte_sip_stack_reg_capability");

    ROI_Rule_Capability_Init = (int (*)(VoLTE_Stack_Ua_Capability_t*))
        getAPIbyDlopenHandler(gRcsVolteCoreHandler, "Rule_Capability_Init");

    ROI_Rule_Capability_Deinit = (int (*)(VoLTE_Stack_Ua_Capability_t*))
        getAPIbyDlopenHandler(gRcsVolteCoreHandler, "Rule_Capability_Deinit");

    ROI_Rule_Level0_Set = (int (*)(VoLTE_Stack_Ua_Capability_t*, char*, char*))
        getAPIbyDlopenHandler(gRcsVolteCoreHandler, "Rule_Level0_Set");

    ROI_Rule_Level1_Set = (int (*)(VoLTE_Stack_Ua_Capability_t*, char*))
        getAPIbyDlopenHandler(gRcsVolteCoreHandler, "Rule_Level1_Set");

    ROI_Rule_Level2_Set = (int (*)(VoLTE_Stack_Ua_Capability_t*, const int , const int , char*))
        getAPIbyDlopenHandler(gRcsVolteCoreHandler, "Rule_Level2_Set");

    ROI_Rule_Level3_Set = (int (*)(VoLTE_Stack_Ua_Capability_t*, char*))
        getAPIbyDlopenHandler(gRcsVolteCoreHandler, "Rule_Level3_Set");

    DbgMsg("init ROI Volte APIs successfully");
    return 0;
}

