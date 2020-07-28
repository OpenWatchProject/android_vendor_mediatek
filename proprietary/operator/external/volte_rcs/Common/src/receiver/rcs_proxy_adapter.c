#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <pthread.h>

#ifdef __ANDROID__
#include <cutils/sockets.h>
#endif

#include "debug.h"
#include "rcs_proxy_adapter.h"
#include "HidlService.h"

static pthread_t g_recv_thread = 0;

static void *RCS_Init_Hal_Thread(void *arg)
{
    int notused = (int)arg;
    /* the thread would blocking in here */
    startHidlService();
    return 0;
}

/*****************************************************************************
 * FUNCTION
 *  print_bytes
 * DESCRIPTION
 *
 * PARAMETERS
 *  object      [IN]
 *  size        [IN]
 * RETURNS
 *  void
 *****************************************************************************/
void print_bytes(const void *object, size_t size)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    size_t i;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    /* DbgMsg("[ "); */
    for (i = 0; i < size; i++)
    {
        DbgMsg("%02x ", ((const unsigned char*)object)[i] & 0xff);
    }
    /* DbgMsg("]\n"); */
}

//callback registerd in init function.
//used to send the data to the rcs_ua_adpater layer via socket
// RCS_proxy ==>  RCS_proxy_adapatation layer
/*****************************************************************************
 * FUNCTION
 *  RCS_Proxy_adapter_SEND
 * DESCRIPTION
 *
 * PARAMETERS
 *  event       [?]
 * RETURNS
 *
 *****************************************************************************/
static int RCS_Proxy_adapter_SEND(RCS_EventMsg_t *event)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (!event || event->source != EventSource_IMS_DISPATCHER) {
        ErrMsg("RCS_Proxy_adapter_SEND error");
        return -1;
    }
    eventNotify((int8_t *)(event->data), event->request_id, event->data_len);
    return 0;
}

/*****************************************************************************
 * FUNCTION
 *  send_test_event_to_dispatcher
 * DESCRIPTION
 *
 * PARAMETERS
 *  void
 * RETURNS
 *  void
 *****************************************************************************/
void send_test_event_to_dispatcher()
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/

    RCS_EventMsg_t *event = 0;

    unsigned int data_len = 0;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    DbgMsg("send_test_event_to_dispatcher");
    /* assign a event with the size of data length */
    event = RCS_COMM_GetEvent(data_len);
    if (!event)
    {
        ErrMsg("Can't allocat the event");
        return;
    }

    /* read request id */

    event->source = EventSource_RCS_PROXY_ADAPTATION;
    event->request_id = CMD_REQ_REG_INFO;
    event->data_len = data_len;

    /* send the event to  queue, which will be read by the main thread */
    RCS_COMM_Recv(event);

    DbgMsg("send_test_event_to_dispatcher sucessfully sent");

}

//initialize RCS proxy adapater
//it startes the read and write thread for handling with RCS_proxy adapter


/*****************************************************************************
 * FUNCTION
 *  RCS_Proxy_adapter_Init_hal
 * DESCRIPTION
 *
 * PARAMETERS
 *  source_id       [IN]
 * RETURNS
 *
 *****************************************************************************/
void RCS_Proxy_adapter_Init_hal(int source_id)
{
    DbgMsg("RCS_Proxy_adapter_Init_hal");

    pthread_attr_t attr;

    /* create rcs hal service thread */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (pthread_create(&g_recv_thread, &(attr), RCS_Init_Hal_Thread, (void*)0))
    {
        ErrMsg("Can't create RCS_Init_Hal_Thread");
        return;
    }

    /* set callback for sending event directly to rcs_adapter */
    RCS_COMM_SetSend(source_id, RCS_Proxy_adapter_SEND);
}

/* send test event response */
/*****************************************************************************
 * FUNCTION
 *  send_test_event_response
 * DESCRIPTION
 *
 * PARAMETERS
 *  void
 * RETURNS
 *  void
 *****************************************************************************/
void send_test_event_response()
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
}

void playbackTest(RCS_EventMsg_t *event) {
    //playback for testing
    RCS_EventMsg_t *event_back = 0;
    event_back = (RCS_EventMsg_t *)RCS_COMM_GetEvent(event->data_len);
    if (!event_back)
    {
        ErrMsg("Can't allocat the event_back");
        return;
    }

        event_back->source = EventSource_IMS_DISPATCHER;
        event_back->request_id = event->request_id;
        event_back->data_len = event->data_len;

        memcpy((char*)(event_back->data), event->data, event->data_len);

        RCS_Proxy_adapter_SEND(event_back);
}

/*****************************************************************************
 * FUNCTION
 *  writeEventHal
 * DESCRIPTION
 *  hidl function for Rcs adapter writing event
 * PARAMETERS
 *  void
 * RETURNS
 *  void
 *****************************************************************************/

void writeEventHal(int32_t request_id, int32_t length, char* data) {
    RCS_EventMsg_t *event = 0;

    /* assign a event with the size of data length */
    event = (RCS_EventMsg_t *)RCS_COMM_GetEvent(length);
    if (!event)
    {
        ErrMsg("Can't allocat the event");
        return;
    }

    event->source = EventSource_RCS_PROXY_ADAPTATION;
    event->request_id = request_id;
    event->data_len = length;

    if (length > 0) {
        memcpy((char*)(event->data), data, length);
    } else {
        //TODO: need to clear data?
        //event->data = NULL;
    }
    /* send the event to  queue, which will be read by the main thread */
    RCS_COMM_Recv(event);
    //playbackTest(event);
}

// RCS for 3g volte
const char * idToString(int id) {
    switch(id) {
        case CMD_REQ_REG_INFO : return "CMD_REQ_REG_INFO";
        case CMD_IMS_REGISTER : return "CMD_IMS_REGISTER";
        case CMD_IMS_DEREGISTER : return "CMD_IMS_DEREGISTER";
        case CMD_SEND_SIP_MSG : return "CMD_SEND_SIP_MSG";
        case CMD_IMS_NOTIFY_VOPS_INFO : return "CMD_IMS_NOTIFY_VOPS_INFO";
        case CMD_IMS_UPDATE_SETTING : return "CMD_IMS_UPDATE_SETTING";
        case CMD_IMS_UPDATE_RT_SETTING : return "CMD_IMS_UPDATE_RT_SETTING";
        case CMD_IMS_ADD_CAPABILITY : return "CMD_IMS_ADD_CAPABILITY";
        case CMD_IMS_NOTIFY_RAT_CHANGE : return "CMD_IMS_NOTIFY_RAT_CHANGE";
        case CMD_IMS_NOTIFY_NETWORK_CHANGE : return "CMD_IMS_NOTIFY_NETWORK_CHANGE";
        case CMD_IMS_AUTH_REQ_RESULT : return "CMD_IMS_AUTH_REQ_RESULT";
        case CMD_IMS_GEOLOCATION_REQ_RESULT : return "CMD_IMS_GEOLOCATION_REQ_RESULT";
        case CMD_IMS_QUERY_STATE_RESULT : return "CMD_IMS_QUERY_STATE_RESULT";
        case CMD_IMS_REG_DIGITLINE : return "CMD_IMS_REG_DIGITLINE";
        case CMD_SIM_OPERATOR : return "CMD_SIM_OPERATOR";
        case CMD_SERVICE_ACTIVATION_STATE: return "CMD_SERVICE_ACTIVATION_STATE";
        case CMD_ENABLE_IMS_DEREG_SUSPEND: return "CMD_ENABLE_IMS_DEREG_SUSPEND";

        case RSP_REQ_REG_INFO : return "RSP_REQ_REG_INFO";
        case RSP_IMS_REGISTERING : return "RSP_IMS_REGISTERING";
        case RSP_IMS_REGISTER : return "RSP_IMS_REGISTER";
        case RSP_IMS_DEREGISTERING : return "RSP_IMS_DEREGISTERING";
        case RSP_IMS_DEREGISTER : return "RSP_IMS_DEREGISTER";
        case RSP_EVENT_SIP_MSG : return "RSP_EVENT_SIP_MSG";

        case EVENT_IMS_DEREGISTER_IND : return "EVENT_IMS_DEREGISTER_IND";
        case EVENT_IMS_AUTH_REQ : return "EVENT_IMS_AUTH_REQ";
        case EVENT_IMS_GEOLOCATION_REQ : return "EVENT_IMS_GEOLOCATION_REQ";
        case EVENT_IMS_QUERY_STATE : return "EVENT_IMS_QUERY_STATE";
        case EVENT_IMS_EMS_MODE_INFO : return "EVENT_IMS_EMS_MODE_INFO";
        case EVENT_IMS_DIGITLING_REG_IND : return "EVENT_IMS_DIGITLING_REG_IND";
        case CMD_ACTIVE_RCS_SLOT_ID : return "CMD_ACTIVE_RCS_SLOT_ID";
        case CMD_IMS_RESTORATION: return "CMD_IMS_RESTORATION";


        default: return "Unknown Rcs id";
    }
}

int RCS_getInt32(void** src, int* dst) {
    if(src == NULL || dst == NULL) {
        return 0;
    }
    int* tmp = *src;
    *dst = *tmp;
    *src = *src + sizeof(int);
    return 1;
}

// Copy to destination space and move source pointer.
int RCS_getStrn(void** src, char* dst, int len) {
    if(src == NULL || dst == NULL) {
        return 0;
    }
    char* tmp = *src;
    strncpy(dst, tmp, len);
    *src = *src + len;
    return 1;
}
