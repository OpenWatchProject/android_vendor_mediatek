
#define LOG_TAG "CLIENTAPI_HAL_MAIN"

#include <string.h>
#include <android/log.h>
#include <cutils/properties.h>

#include "debug.h"
#include "api_clientapi_msgq.h"
#include "clientapi_comm.h"
#include "clientapi_proxy.h"
#include "clientapi_proxy_adapter.h"
#include "clientapi_proxy_dispatcher.h"


static int main_CLIENTAPI_Proxy_Adapter_Init(void) {

    DbgMsg("UA start with HIDL Service");
    //set the ballback function
    //from: EventSource_IMS_DISPATCHER
    //to: EventSource_CLIENTAPI_PROXY_ADAPTATION
    CLIENTAPI_Proxy_adapter_Init_hal(EventSource_IMS_DISPATCHER);
    return 0;
}

void ClientapiEventDispatcher(CLIENTAPI_EventMsg_t *event) {

#if 0
    DbgMsg("ClientapiEventDispatcher, source: %d", event->source);
#endif


    switch (event->source) {
        case EventSource_CLIENTAPI_PROXY_ADAPTATION:
			ErrMsg("CLIENTAPI receive msg from EventSource_CLIENTAPI_PROXY_ADAPTATION");
            handleEventFromAdapter(event);
            break;
        case EventSource_IMS_DISPATCHER:
			ErrMsg("CLIENTAPI receive msg from EventSource_IMS_DISPATCHER");
            handleEventFromDispatcher(event);
            break;
        default:{
            ErrMsg("Invalid source : %d", event->source);
            break;
        }
    }
}

int main(int argc, char **argv) {

    void *mesgq = 0;
    int notused = argc;
    char **notused2 = argv;

    gVolteCoreHandler = NULL;

    DbgMsg("service started");
	ErrMsg("service started");

    /* initialize queue */
    mesgq = CLIENTAPI_CMSGQ_Init(32, "clientapiproxy_mesgq", 0);

    if (!mesgq) {
        ErrMsg("Can't create the message queue");
        return -1;
    }

    /* initialize the message queue */
    CLIENTAPI_COMM_Init(mesgq);

    if (0 == initVolteCoreAPIs()) {
        /* initilaize clientapi proxy which will manage the interaction with the Volte Dispatcher */
        CLIENTAPI_Proxy_Init(); 
    }

    /* initialize the clientapi proxy adapter */
    main_CLIENTAPI_Proxy_Adapter_Init();

    while (1) {

        CLIENTAPI_EventMsg_t *event = 0;

        event = (CLIENTAPI_EventMsg_t *)CLIENTAPI_CMSGQ_Get(mesgq, CLIENTAPI_CMSGQ_FLAG_WAIT);

        if (!event) {
            /* ErrMsg("Event Message is NULL"); */
            continue;
        }

        /* handle the clientapi event */
        ClientapiEventDispatcher(event);

        /* FREE THE EVENT */
        CLIENTAPI_CMSGQ_MsgFree(event);
    }

    return 0;
}
