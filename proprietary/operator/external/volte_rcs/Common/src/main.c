#include <string.h>
#include <dlfcn.h>
#include "debug.h"
#include "api_rcs_msgq.h"
#include "rcs_comm.h"
#include "rcs_proxy.h"
#include "rcs_proxy_adapter.h"
#include "rcs_proxy_dispatcher.h"
#include "rcs_proxy_session_handler.h"

#include <android/log.h>
#include <cutils/properties.h>


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

/** Dlopen handling function **/
void *getDlopenHandler(const char *libPath);
void *getAPIbyDlopenHandler(void *dlHandler, const char *apiName);
void freeDlopenHandler(void *dlHandler);

void *dlopenHandlerForVolteApi;
void *dlopenHandlerForMalApi;
int initVolteApi();
void initMalApi();

void *rcsVolteApiHandler;
int initRcsVolteCoreApi();

int imsSubmarineSupported = 0;
int gROISupported = 0;
char gSimOptr[92] = {0};

int isROISupported() {
    /* Rcs Over Internet only supported in T-Mobile Network */
    memset(gSimOptr, 0, sizeof(gSimOptr));
    property_get("persist.vendor.operator.optr", gSimOptr, "");
    return (strcmp("OP08", gSimOptr) == 0) ? 1 : 0;
}

int isImsSubmarineSupported() {
    DbgMsg("isImsSubmarineSupported, %d", imsSubmarineSupported);
    return imsSubmarineSupported;
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


void *getAPIbyDlopenHandler(void *dlHandler, const char *apiName) {
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

void freeDlopenHandler(void *dlHandler) {
    if (dlHandler != NULL) {
        DbgMsg("Free dlopen handler");
        dlclose(dlHandler);
    }
}

int initVolteApi() {
    freeDlopenHandler(dlopenHandlerForVolteApi);
    dlopenHandlerForVolteApi = getDlopenHandler("libvolte_core_shr.so");

    if (dlopenHandlerForVolteApi == NULL) {
        DbgMsg("Dlopen Volte APIs fail");
        dlopenHandlerForVolteApi = getDlopenHandler("librcs_interface.so");
        if (dlopenHandlerForVolteApi == NULL) {
            DbgMsg("Dlopen rcs client APIs fail");
            return 1;
        }
        imsSubmarineSupported = 1;
    }
    volte_sip_info_free = (int (*)(VoLTE_Stack_Sip_Info_t*))
        getAPIbyDlopenHandler(dlopenHandlerForVolteApi, "volte_sip_info_free");

    volte_sip_stack_init = (int (*)())
        getAPIbyDlopenHandler(dlopenHandlerForVolteApi, "volte_sip_stack_init");

    _volte_sip_stack_create =
        (void *(*)(VoLTE_Stack_Channel_Info_t*,
            VoLTE_Stack_Ua_Capability_t*,
            VoLTE_Stack_Callback_t*,
            char*,
            int))
        getAPIbyDlopenHandler(dlopenHandlerForVolteApi, "_volte_sip_stack_create");

    volte_sip_stack_destroy = (int (*)(void*))
        getAPIbyDlopenHandler(dlopenHandlerForVolteApi, "volte_sip_stack_destroy");

    volte_sip_stack_send_sip = (int (*)(void *, VoLTE_Stack_Sip_Info_t *))
        getAPIbyDlopenHandler(dlopenHandlerForVolteApi, "volte_sip_stack_send_sip");

    volte_sip_stack_send_msg = (int (*)(void *, VoLTE_Stack_Message_t *))
        getAPIbyDlopenHandler(dlopenHandlerForVolteApi, "volte_sip_stack_send_msg");

    volte_sip_stack_reg_state = (int (*)(void *))
        getAPIbyDlopenHandler(dlopenHandlerForVolteApi, "volte_sip_stack_reg_state");

    volte_sip_stack_reg_bind = (int (*)(void *, int ))
        getAPIbyDlopenHandler(dlopenHandlerForVolteApi, "volte_sip_stack_reg_bind");

    volte_sip_stack_reg_unbind = (int (*)(void *))
            getAPIbyDlopenHandler(dlopenHandlerForVolteApi, "volte_sip_stack_reg_unbind");

    volte_sip_stack_reg_capability = (int (*)(void *, int , char *))
        getAPIbyDlopenHandler(dlopenHandlerForVolteApi, "volte_sip_stack_reg_capability");

    Rule_Capability_Init = (int (*)(VoLTE_Stack_Ua_Capability_t*))
        getAPIbyDlopenHandler(dlopenHandlerForVolteApi, "Rule_Capability_Init");

    Rule_Capability_Deinit = (int (*)(VoLTE_Stack_Ua_Capability_t*))
        getAPIbyDlopenHandler(dlopenHandlerForVolteApi, "Rule_Capability_Deinit");

    Rule_Level0_Set = (int (*)(VoLTE_Stack_Ua_Capability_t*, char*, char*))
        getAPIbyDlopenHandler(dlopenHandlerForVolteApi, "Rule_Level0_Set");

    Rule_Level1_Set = (int (*)(VoLTE_Stack_Ua_Capability_t*, char*))
        getAPIbyDlopenHandler(dlopenHandlerForVolteApi, "Rule_Level1_Set");

    Rule_Level2_Set = (int (*)(VoLTE_Stack_Ua_Capability_t*, const int , const int , char*))
        getAPIbyDlopenHandler(dlopenHandlerForVolteApi, "Rule_Level2_Set");

    Rule_Level3_Set = (int (*)(VoLTE_Stack_Ua_Capability_t*, char*))
        getAPIbyDlopenHandler(dlopenHandlerForVolteApi, "Rule_Level3_Set");

    volte_sip_stack_restore = (int (*)(void *))
            getCompatibleAPIbyDlopenHandler(dlopenHandlerForVolteApi, "volte_sip_stack_restore");

    volte_sip_stack_ind_external_ua_state = (int (*)(void *, int, int, int))
            getCompatibleAPIbyDlopenHandler(
            dlopenHandlerForVolteApi, "volte_sip_stack_ind_external_ua_state");

    DbgMsg("init Volte APIs successfully");
    return 0;
}

int initRcsVolteCoreApi() {
    freeDlopenHandler(rcsVolteApiHandler);
    rcsVolteApiHandler = getDlopenHandler("librcs_volte_core.so");
    if (rcsVolteApiHandler == NULL) {
        DbgMsg("Dlopen ibrcs_volte_core.so fail");
        return -1;
    }
    roi_volte_sip_info_free = (int (*)(VoLTE_Stack_Sip_Info_t*))
        getAPIbyDlopenHandler(rcsVolteApiHandler, "volte_sip_info_free");

    roi_volte_sip_stack_init = (int (*)())
        getAPIbyDlopenHandler(rcsVolteApiHandler, "volte_sip_stack_init");

    _roi_volte_sip_stack_create =
        (void *(*)(VoLTE_Stack_Channel_Info_t*,
        VoLTE_Stack_Ua_Capability_t*,
        VoLTE_Stack_Callback_t*,
        char*,
        int))
        getAPIbyDlopenHandler(rcsVolteApiHandler, "_volte_sip_stack_create");

    roi_volte_sip_stack_destroy = (int (*)(void*))
        getAPIbyDlopenHandler(rcsVolteApiHandler, "volte_sip_stack_destroy");

    roi_volte_sip_stack_send_sip = (int (*)(void *, VoLTE_Stack_Sip_Info_t *))
        getAPIbyDlopenHandler(rcsVolteApiHandler, "volte_sip_stack_send_sip");

    roi_volte_sip_stack_send_msg = (int (*)(void *, VoLTE_Stack_Message_t *))
        getAPIbyDlopenHandler(rcsVolteApiHandler, "volte_sip_stack_send_msg");

    roi_volte_sip_stack_reg_state = (int (*)(void *))
        getAPIbyDlopenHandler(rcsVolteApiHandler, "volte_sip_stack_reg_state");

    roi_volte_sip_stack_reg_bind = (int (*)(void *, int ))
        getAPIbyDlopenHandler(rcsVolteApiHandler, "volte_sip_stack_reg_bind");

    roi_volte_sip_stack_reg_unbind = (int (*)(void *))
        getAPIbyDlopenHandler(rcsVolteApiHandler, "volte_sip_stack_reg_unbind");

    roi_volte_sip_stack_reg_capability = (int (*)(void *, int , char *))
        getAPIbyDlopenHandler(rcsVolteApiHandler, "volte_sip_stack_reg_capability");

    ROI_Rule_Capability_Init = (int (*)(VoLTE_Stack_Ua_Capability_t*))
        getAPIbyDlopenHandler(rcsVolteApiHandler, "Rule_Capability_Init");

    ROI_Rule_Capability_Deinit = (int (*)(VoLTE_Stack_Ua_Capability_t*))
        getAPIbyDlopenHandler(rcsVolteApiHandler, "Rule_Capability_Deinit");

    ROI_Rule_Level0_Set = (int (*)(VoLTE_Stack_Ua_Capability_t*, char*, char*))
        getAPIbyDlopenHandler(rcsVolteApiHandler, "Rule_Level0_Set");

    ROI_Rule_Level1_Set = (int (*)(VoLTE_Stack_Ua_Capability_t*, char*))
        getAPIbyDlopenHandler(rcsVolteApiHandler, "Rule_Level1_Set");

    ROI_Rule_Level2_Set = (int (*)(VoLTE_Stack_Ua_Capability_t*, const int , const int , char*))
        getAPIbyDlopenHandler(rcsVolteApiHandler, "Rule_Level2_Set");

    ROI_Rule_Level3_Set = (int (*)(VoLTE_Stack_Ua_Capability_t*, char*))
        getAPIbyDlopenHandler(rcsVolteApiHandler, "Rule_Level3_Set");

    roi_sip_stack_update_setting = (int (*)(void *, void * , int))
        getAPIbyDlopenHandler(rcsVolteApiHandler, "volte_sip_stack_update_setting");

    roi_sip_stack_reg_operation = (int (*)(void *, int ))
        getAPIbyDlopenHandler(rcsVolteApiHandler, "volte_sip_stack_reg_operation");

    DbgMsg("init ROI Volte APIs successfully");
    return 0;
}

void initMalApi() {
    freeDlopenHandler(dlopenHandlerForMalApi);
    dlopenHandlerForMalApi = getDlopenHandler("libmal.so");
    if (dlopenHandlerForMalApi == NULL) {
        return;
    }

    rds_set_rcsstat = (int (*)(rds_ru_set_callstat_req_t *))
        getAPIbyDlopenHandler(dlopenHandlerForMalApi, "rds_set_rcsstat");

    DbgMsg("Dlopen Mal APIs success");
}

/*****************************************************************************
 * FUNCTION
 *  main_RCS_Proxy_Adapter_Init
 * DESCRIPTION
 *
 * PARAMETERS
 *  void
 * RETURNS
 *
 *****************************************************************************/
static int main_RCS_Proxy_Adapter_Init(void)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    DbgMsg("UA start with HIDL Service");
    //set the ballback function
    //from: EventSource_IMS_DISPATCHER
    //to: EventSource_UCE_PROXY_ADAPTATION
    RCS_Proxy_adapter_Init_hal(EventSource_IMS_DISPATCHER);
    return 0;
}

/* HANDLE THE EVENT AFTER READING FROM THE INTERNAL QUEUE */


/*****************************************************************************
 * FUNCTION
 *  RCS_Proxy_Process_Events
 * DESCRIPTION
 *
 * PARAMETERS
 *  event       [?]
 * RETURNS
 *  void
 *****************************************************************************/
void RCS_Proxy_Process_Events(RCS_EventMsg_t *event)
{
    DbgMsg("RCS proxy internal event, source : %d ", event->source);
    switch (event->source)
    {
        case EventSource_RCS_PROXY_ADAPTATION:
            handleEventFromAdapter(event);
            break;
        case EventSource_IMS_DISPATCHER:
            handleEventFromDispatcher(event);
            break;
        default:
            ErrMsg("Invalid source : %d", event->source);
            break;
    }
}


/*****************************************************************************
 * FUNCTION
 *  main
 * DESCRIPTION
 *
 * PARAMETERS
 *  argc        [IN]
 *  argv        [IN]
 * RETURNS
 *
 *****************************************************************************/
int main(int argc, char **argv)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    void *mesgq = 0;
    int tempC = argc;
    char **tempV = argv;

    gROISupported = isROISupported();

    DbgMsg("service started, ROI: %d", gROISupported);

    /* initialize queue */
    mesgq = RCS_CMSGQ_Init(32, "rcsproxy_mesgq", 0);
    if (!mesgq)
    {
        ErrMsg("Can't create the message queue");
        return -1;
    }

    /* initialize the message queue */
    RCS_COMM_Init(mesgq);

    /* volte APIs */
    if (0 == initVolteApi()) {
        /* initilaize rcs proxy which will manage the interaction with the Volte Dispatcher */
        RCS_Proxy_Init();
    }
    if (0 == initRcsVolteCoreApi()) {
        /* initilaize rcs proxy for ROI */
        RCS_Roi_Proxy_Init();
    }
    initMalApi();

    /* initialize the rcs proxy adapter */
    main_RCS_Proxy_Adapter_Init();

    while (1)
    {
        RCS_EventMsg_t *event = 0;

        event = RCS_CMSGQ_Get(mesgq, RCS_CMSGQ_FLAG_WAIT);
        if (!event) {
            continue;
        }
        RCS_Proxy_Process_Events(event);
        RCS_CMSGQ_MsgFree(event);
    }

    return 0;
}
