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
#include "uce_comm.h"
#include "uce_proxy_adapter.h"
#include "HidlService.h"

static pthread_t g_recv_thread = 0;

static void *UCE_Init_Hal_Thread(void *arg)
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
 *  UCE_Proxy_adapter_SEND
 * DESCRIPTION
 *
 * PARAMETERS
 *  event       [?]
 * RETURNS
 *
 *****************************************************************************/
static int UCE_Proxy_adapter_SEND(UCE_EventMsg_t *event)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (!event || event->source != EventSource_IMS_DISPATCHER) {
        ErrMsg("UCE_Proxy_adapter_SEND error");
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

    UCE_EventMsg_t *event = 0;

    unsigned int data_len = 0;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    DbgMsg("send_test_event_to_dispatcher");
    /* assign a event with the size of data length */
    event = (UCE_EventMsg_t *)UCE_COMM_GetEvent(data_len);
    if (!event)
    {
        ErrMsg("Can't allocat the event");
        return;
    }

    /* read request id */

    event->source = EventSource_UCE_PROXY_ADAPTATION;
    event->request_id = CMD_REQ_REG_INFO;
    event->data_len = data_len;

    /* send the event to  queue, which will be read by the main thread */
    UCE_COMM_Recv(event);

    DbgMsg("send_test_event_to_dispatcher sucessfully sent");

}

//initialize UCE proxy adapater
//it startes the read and write thread for handling with UCE_proxy adapter

/*****************************************************************************
 * FUNCTION
 *  UCE_Proxy_adapter_Init_hal
 * DESCRIPTION
 *
 * PARAMETERS
 *  source_id       [IN]
 * RETURNS
 *
 *****************************************************************************/
void UCE_Proxy_adapter_Init_hal(int source_id)
{
    DbgMsg("UCE_Proxy_adapter_Init_hal");

    pthread_attr_t attr;

    /* create presence hal service thread */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (pthread_create(&g_recv_thread, &(attr), UCE_Init_Hal_Thread, (void*)0))
    {
        ErrMsg("Can't create UCE_Init_Hal_Thread");
        return;
    }

    /* set callback for sending event directly to uce_adapter */
    UCE_COMM_SetSend(source_id, UCE_Proxy_adapter_SEND);
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

void playbackTest(UCE_EventMsg_t *event) {
    //playback for testing
    UCE_EventMsg_t *event_back = 0;
    event_back = (UCE_EventMsg_t *)UCE_COMM_GetEvent(event->data_len);
    if (!event_back)
    {
        ErrMsg("Can't allocat the event_back");
        return;
    }

    event_back->source = EventSource_IMS_DISPATCHER;
    event_back->request_id = event->request_id;
    event_back->data_len = event->data_len;

    memcpy((char*)(event_back->data), event->data, event->data_len);

    UCE_Proxy_adapter_SEND(event_back);
}

/*****************************************************************************
 * FUNCTION
 *  writeEventHal
 * DESCRIPTION
 *  hidl function for Presence adapter writing event
 * PARAMETERS
 *  void
 * RETURNS
 *  void
 *****************************************************************************/

void writeEventHal(int32_t request_id, int32_t length, char* data) {
    UCE_EventMsg_t *event = 0;

    /* assign a event with the size of data length */
    event = (UCE_EventMsg_t *)UCE_COMM_GetEvent(length);
    if (!event)
    {
        ErrMsg("Can't allocat the event");
        return;
    }

    event->source = EventSource_UCE_PROXY_ADAPTATION;
    event->request_id = request_id;
    event->data_len = length;

    if (length > 0) {
        memcpy((char*)(event->data), data, length);
    } else {
        //TODO: need to clear data?
        //event->data = NULL;
    }
    /* send the event to  queue, which will be read by the main thread */
    UCE_COMM_Recv(event);
    //playbackTest(event);
}
