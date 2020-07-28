
#include <string.h>
#include <android/log.h>
#include <cutils/properties.h>

#include "debug.h"
#include "api_uce_msgq.h"
#include "uce_comm.h"
#include "uce_proxy.h"
#include "uce_proxy_adapter.h"
#include "uce_proxy_dispatcher.h"


static int main_UCE_Proxy_Adapter_Init(void) {

    DbgMsg("UA start with HIDL Service");
    //set the ballback function
    //from: EventSource_IMS_DISPATCHER
    //to: EventSource_UCE_PROXY_ADAPTATION
    UCE_Proxy_adapter_Init_hal(EventSource_IMS_DISPATCHER);
    return 0;
}

void UceEventDispatcher(UCE_EventMsg_t *event) {

#if 0
    DbgMsg("UceEventDispatcher, source: %d", event->source);
#endif

    switch (event->source) {
        case EventSource_UCE_PROXY_ADAPTATION:
            handleEventFromAdapter(event);
            break;
        case EventSource_IMS_DISPATCHER:
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
    gRcsVolteCoreHandler = NULL;

    DbgMsg("service started");

    /* initialize queue */
    mesgq = UCE_CMSGQ_Init(32, "uceproxy_mesgq", 0);

    if (!mesgq) {
        ErrMsg("Can't create the message queue");
        return -1;
    }

    /* initialize the message queue */
    UCE_COMM_Init(mesgq);

    if (0 == initVolteCoreAPIs()) {
        /* initilaize uce proxy which will manage the interaction with the Volte Dispatcher */
        UCE_Proxy_Init(); 
    }

    if (0 == initRcsVolteCoreAPIs()) {
        /* Establish channel between UA Proxy and Rcs VoLTE stack */
        UCE_Roi_Proxy_Init();
    }

    /* initialize the uce proxy adapter */
    main_UCE_Proxy_Adapter_Init();

    while (1) {

        UCE_EventMsg_t *event = 0;

        event = (UCE_EventMsg_t *)UCE_CMSGQ_Get(mesgq, UCE_CMSGQ_FLAG_WAIT);

        if (!event) {
            /* ErrMsg("Event Message is NULL"); */
            continue;
        }

        /* handle the uce event */
        UceEventDispatcher(event);

        /* FREE THE EVENT */
        UCE_CMSGQ_MsgFree(event);
    }

    return 0;
}
