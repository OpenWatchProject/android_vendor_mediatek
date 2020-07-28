#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "clientapi_proxy_dispatcher.h"
#include "clientapi_proxy_adapter.h"

#include "debug.h"

#include "volte_core_interface.h"

#define getVarName(var)  #var

void *g_stack_channel_sip = 0;
void *g_roi_stack_channel_sip = 0;

static void *g_stack_channel_event = 0;
static void *g_roi_stack_channel_event = 0;


//bool to request that the clientapi_proxy_request was sent or not
//in case of deregister itr will be set to 0
static int clientapi_proxy_request_sent = 0;

static int clientapi_register_request_sent = 0;
static int clientapi_deregister_request_sent = 0;

static int g_ims_registered_owner = OWNER_INVALID;
static VoLTE_Event_Reg_State_t *g_reg_state = 0;
static int gIgnoreEvent = 0;
static pthread_mutex_t reg_mutex = PTHREAD_MUTEX_INITIALIZER;

/* callback for the SIP message receiving from DISPatcher */
/*****************************************************************************
 * FUNCTION
 *  volte_read_sip_cb
 * DESCRIPTION
 *
 * PARAMETERS
 *  sip             [?]
 *  user_data       [?]
 * RETURNS
 *  void
 *****************************************************************************/
static void volte_read_sip_cb(VoLTE_Stack_Sip_Info_t *sip, void *user_data)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    char print_buff[53];
    void * notused = user_data;
    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (sip == NULL)
    {
        ErrMsg("SIP channel is disconnected & exit");
        exit(-1);
    }
    SysMsg(
        "SIP MSG RECEIVED :\n request_id = %d,conn_id = %d, len = %d, \n message = %s\n",
        sip->request_id,
        sip->conn_id,
        sip->message_len,
        sip->message);
    CLIENTAPI_EventMsg_t *event = 0;

    //size of volte sip stack + message length
    int intsize = sizeof(int);

    event = (CLIENTAPI_EventMsg_t *)CLIENTAPI_COMM_GetEvent(3*intsize + sip->message_len);
    event->source = EventSource_IMS_DISPATCHER;
    event->request_id = RSP_EVENT_SIP_MSG;
    event->data_len = 3*intsize + sip->message_len;
    DbgMsg("event length %d ",event->data_len);

    //assign memcpy
    memcpy(event->data, &(sip->request_id), intsize);
    memcpy(&(event->data[intsize]), &(sip->conn_id), intsize);
    memcpy(&(event->data[intsize + intsize]), &(sip->message_len), intsize);
    memcpy(&(event->data[intsize + intsize + intsize]), sip->message, sip->message_len);

    /* send the event to  queue, which will be read by the main thread */
    CLIENTAPI_COMM_Recv(event);

    DbgMsg("SIP message sent to ClientAPI");

    return;
}



int send_sip_message(int req_id, int conn_id, char *sip_message, int length) {

    sip_message_struct *resp_sip_msg = 0;
    VoLTE_Stack_Sip_Info_t *sip_info = 0;
    int ret = 0;
    int total_length = sizeof(VoLTE_Stack_Sip_Info_t) + length;

    sip_info = (VoLTE_Stack_Sip_Info_t*) malloc(total_length);
    memset(sip_info, 0, total_length);
    sip_info->request_id = req_id;   // request id is assigned by the UA
    sip_info->conn_id = conn_id; //conn_id
    sip_info->message_len = length;

    memcpy(sip_info->message, sip_message, sip_info->message_len);


    DbgMsg("send_sip_message: owner[%d], req_id[%d], conn_id[%d], SIP_msg length[%d] ",
            g_ims_registered_owner, sip_info->request_id,sip_info->conn_id, sip_info->message_len);

#if 0
    DbgMsg("Message: %s", sip_info->message);
#endif

    /* send SIP to volte_stack */
    volte_sip_stack_send_sip(g_stack_channel_sip, sip_info);
    volte_sip_info_free(sip_info);

    return 0;
}



/* send request to stack for registration information */
void CLIENTAPI_dispatcher_Send_Req_Reg_Info(void) {

    DbgMsg("send registartion state request to the dispatcher");

    //send request to get the registrtaion state
    volte_sip_stack_reg_state(g_stack_channel_event);
}

/* create SIP stack connection */
void create_SIP_Stack_connection(VoLTE_Event_Reg_State_t *reg_state) {

    int rule_init_status =  -1;
    VoLTE_Stack_Channel_Info_t info;
    VoLTE_Stack_Callback_t callback;
    VoLTE_Stack_Ua_Capability_t cap;

    info.type = VoLTE_Stack_UA_ClientAPI_Proxy | VoLTE_UA_MAGIC_NUMBER;
    info.conn_info = reg_state->conn_info;

    callback.read_sip = volte_read_sip_cb;
    callback.read_msg = 0;
    callback.user_data = 0;

    Rule_Capability_Init(&cap);
    /* DbgMsg("CLIENTAPI[%d] public_uid = %s, associated_uri = %s", reg_state->id, reg_state->public_uid, reg_state->associated_uri); */
    Rule_Level0_Set(&cap, reg_state->public_uid, reg_state->associated_uri);

    Rule_Level2_Set(&cap, SIP_MESSAGE_METHOD, SIP_HDR_CONTENT_TYPE, "application/clientapi");
    Rule_Level2_Set(&cap, SIP_NOTIFY_METHOD, SIP_HDR_EVENT, "clientapi");
    rule_init_status = Rule_Level3_Set(&cap, "cpm");
    g_stack_channel_sip = volte_sip_stack_create(&info, &cap, &callback);
    Rule_Capability_Deinit(&cap);

    if (!g_stack_channel_sip) {
        ErrMsg("Can't create the channel to sip channel stack");
    } else {
        DbgMsg("CLIENTAPI[%d] create sip channel successfully ",reg_state->id);
    }
}



/* read event stack message : related to Registration */
/*****************************************************************************
 * FUNCTION
 *  volte_read_msg_cb
 * DESCRIPTION
 *
 * PARAMETERS
 *  msg             [?]
 *  user_data       [?]
 * RETURNS
 *  void
 *****************************************************************************/
static void volte_read_msg_cb(VoLTE_Stack_Message_t *msg, void *user_data) {

    int i = 0;

    if (!msg) {
        ErrMsg("stack channel is disconnected & exit");
        exit(-1);
    }

    switch (msg->type) {
        case VOLTE_IND_SIPTX_REG_STATE:
            SysMsg("<<< volte_read_msg_cb, type: VOLTE_IND_SIPTX_REG_STATE");
            handle_ims_reg_state(msg, OWNER_VOLTE_STACK);
            break;
        case VOLTE_IND_SIPTX_REG_DEREG_START:
            SysMsg("<<< volte_read_msg_cb, type: VOLTE_IND_SIPTX_REG_DEREG_START");
            handle_ims_dereg_start(msg, user_data);
            break;
        default:
            break;
    }
}


/* initialize the CLIENTAPI_proxy and dispatcher connection */
/*****************************************************************************
 * FUNCTION
 *  CLIENTAPI_Proxy_Init
 * DESCRIPTION
 *
 * PARAMETERS
 *  void
 * RETURNS
 *
 *****************************************************************************/
int CLIENTAPI_Proxy_Init(void)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    VoLTE_Stack_Channel_Info_t info;
    VoLTE_Stack_Callback_t callback;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    DbgMsg("CLIENTAPI[%d] volte_sip_stack init", 0);
    volte_sip_stack_init();

    DbgMsg("CLIENTAPI[%d] Connect to stack, create registration event channel", 0);

    /* configure the SIP channel information */
    memset(&info, 0, sizeof(VoLTE_Stack_Channel_Info_t));

    info.type = VoLTE_Stack_UA_ClientAPI_Proxy | VoLTE_UA_MAGIC_NUMBER;

    /* configure the callback function information */
    memset(&callback, 0, sizeof(VoLTE_Stack_Callback_t));
    callback.read_msg = volte_read_msg_cb;
    callback.user_data = 0;

    g_stack_channel_event = volte_sip_stack_create(&info, 0, &callback);
    if (!g_stack_channel_event)
    {
        ErrMsg("Can't create the channel to stack");
        return 1;
    }

    DbgMsg("CLIENTAPI[%d] event channel connected successfully", 0);

    int acct_id = 0;

    volte_sip_stack_reg_bind(g_stack_channel_event, acct_id);

    return 0;

}

/* initialize the CLIENTAPI_proxy and dispatcher connection */
/*****************************************************************************
 * FUNCTION
 *  CLIENTAPI_Roi_Proxy_Init
 * DESCRIPTION
 *
 * PARAMETERS
 *  void
 * RETURNS
 *
 *****************************************************************************/

void handle_ims_reg_state(VoLTE_Stack_Message_t *msg, int from) {

    char owner[20];
    CLIENTAPI_EventMsg_t *event = 0;
    VoLTE_Event_Reg_State_t *reg_state = (VoLTE_Event_Reg_State_t*) msg->data;

    if (msg->len != sizeof(VoLTE_Event_Reg_State_t)) {
        //ErrMsg("Data length error");
    }
    DbgMsg("reg_mutex in");
    pthread_mutex_lock(&reg_mutex);
    /*
     *   0: SIM1 IMS
     *   1: SIM1 Emergency
     *   2: SIM2 IMS
     *   3: SIM2 Emergency
     */
    if (reg_state->id == 1 || reg_state->id == 3) {
        DbgMsg("ignore VOLTE_IND_SIPTX_REG_STATE event since it's for emergency account\n");
        goto free_mutex;
    }

    if (reg_state->cause == VoLTE_Event_Reg_Cause_Not_Bind) {
        DbgMsg("rebinding to REG UA \n");
        int acct_id = 0;
        volte_sip_stack_reg_bind(g_stack_channel_event, acct_id);
    }
    strcpy(owner, "VOLTE");

    DbgMsg("Reg-Info[%d] from %s state = %d", reg_state->id, owner, reg_state->state);

    if (gIgnoreEvent == 1 && from != g_ims_registered_owner) {
        DbgMsg("!!ignore reg_state event!!");
        goto free_mutex;
    }

    if (reg_state->state == VoLTE_Event_Reg_State_Registered) {
        g_ims_registered_owner = from;
        gIgnoreEvent = 1;
    } else if (reg_state->state != VoLTE_Event_Reg_State_Registered && from == g_ims_registered_owner) {
        g_ims_registered_owner = OWNER_INVALID;
        gIgnoreEvent = 0;
    }



#if 0
    DbgMsg("other info:"
        "\n cause = %d"
        "\n local_address = %s"
        "\n local_port = %d"
        "\n protocol_type = %d"
        "\n protocol_version = %d"
        "\n public_uid = %s"
        "\n private_uid = %s"
        "\n home_uri = %s"
        "\n pcscf_address = %s"
        "\n pcscf_port = %d"
        "\n imei = %s"
        "\n associated_uri = %s"
        "\n pub_gruu = %s"
        "\n temp_gruu = %s ,"
        "\n service_route = %s"
        "\n path = %s"
        "\n net_type  = %d"
        "\n emergency_type = %d"
        "\n reg_timestamp = %s"
        "\n security-verify = %s"
        "\n user-agent = %s",
         reg_state->cause,
         reg_state->local_address,
         reg_state->local_port,
         reg_state->protocol_type,
         reg_state->protocol_version,
         reg_state->public_uid,
         reg_state->private_uid,
         reg_state->home_uri,
         reg_state->pcscf_address,
         reg_state->pcscf_port,
         reg_state->imei,
         reg_state->associated_uri,
         reg_state->pub_gruu,
         reg_state->temp_gruu,
         reg_state->service_route,
         reg_state->path,
         reg_state->net_type,
         reg_state->emergency_type,
         reg_state->reg_timestamp,
         reg_state->security_verify,
         reg_state->user_agent);
#endif

    //IF IMS CURRENT STATE IS REGISTERED
    if (reg_state->state == VoLTE_Event_Reg_State_Registered) {
        if (!g_stack_channel_sip) {
            create_SIP_Stack_connection(reg_state);
        }
    } else if (reg_state->state == VoLTE_Event_Reg_State_Disconnected) {
        if (g_stack_channel_sip) {
            DbgMsg("CLIENTAPI[%d] Disconnect to stack", reg_state->id);
            volte_sip_stack_destroy(g_stack_channel_sip);
            g_stack_channel_sip = 0;
        }
    }

    //if CLIENTAPI REGISTER REQUEST IS SENT . THEN THIS RESPONSE IS FOR THAT REQUEST
    if (clientapi_register_request_sent) {

        //RESET THE VALUE
        clientapi_register_request_sent = 0 ;

        char reg_update_state[2] = {'0','\0'};
        event = (CLIENTAPI_EventMsg_t *)CLIENTAPI_COMM_GetEvent(sizeof(reg_update_state));
        event->request_id = RSP_IMS_REGISTER;
        event->source = EventSource_CLIENTAPI_PROXY_ADAPTATION;
        event->data_len = sizeof(reg_update_state);

        //IF CLIENTAPI GOT REGISTERED
        if (reg_state->state == VoLTE_Event_Reg_State_Registered) {
            reg_update_state[0] = '1';
            //notify CLIENTAPI
            DbgMsg("ClientAPI_UA ==> ClientAPI_ADAPTER : CLIENTAPI_PROXY_EVENT_RSP_CLIENTAPI_REGISTERED [Success]");
            memcpy(event->data, &reg_update_state, sizeof(reg_update_state));
            CLIENTAPI_COMM_Send(event);
            goto free_mutex;
        } else if ((reg_state->state == VoLTE_Event_Reg_State_Disconnected)
                ||(reg_state->state == VoLTE_Event_Reg_State_Unregistered)) {

            //IF CLIENTAPI FAILED

           //state = 0
            reg_update_state[0] = '0';

            DbgMsg("ClientAPI_UA ==> ClientAPI_ADAPTER : CLIENTAPI_PROXY_EVENT_RSP_CLIENTAPI_REGISTERED [failure]");
            memcpy(event->data, &reg_update_state, sizeof(reg_update_state));
            CLIENTAPI_COMM_Send(event);
            goto free_mutex;
        }
    }

    /**
     * SEND RESPONSE TO THE UA
     */
    CLIENTAPI_PROXY_Event_Reg_State2 *resp2 = 0;

    /* notify the event and put it in the main thread */
    event = (CLIENTAPI_EventMsg_t *)CLIENTAPI_COMM_GetEvent(sizeof(CLIENTAPI_PROXY_Event_Reg_State2));
    event->source = EventSource_IMS_DISPATCHER;
    event->request_id = RSP_REQ_REG_INFO;
    event->data_len = sizeof(CLIENTAPI_PROXY_Event_Reg_State2);

    g_reg_state = (VoLTE_Event_Reg_State_t*) msg->data;

    //copy content
    resp2 = (CLIENTAPI_PROXY_Event_Reg_State2*) malloc(sizeof(CLIENTAPI_PROXY_Event_Reg_State2));
    memset(resp2, 0, sizeof(CLIENTAPI_PROXY_Event_Reg_State2));

    resp2->id = from; // since account_id is meaningless for stack, reuse for "from" purpose
    resp2->state = reg_state->state;
    memcpy(resp2->local_address, reg_state->local_address, VOLTE_MAX_ADDRESS_LENGTH);
    resp2->local_port = reg_state->local_port;
    resp2->protocol_type = reg_state->protocol_type; //protocol type : 0 -> UDP  ; 1 -> TCP
    resp2->protocol_version = reg_state->protocol_version; //protocol version : 0 -> IPV4  ; 1 -> IPv6
    memcpy(resp2->public_uid, reg_state->public_uid, VOLTE_MAX_REG_UID_LENGTH);
    memcpy(resp2->pcscf_address, reg_state->pcscf_address,VOLTE_MAX_ADDRESS_LIST_LENGTH);
    resp2->pcscf_port = reg_state->pcscf_port;
    memcpy(resp2->user_agent, reg_state->user_agent,VOLTE_MAX_USER_AGENT_LENGTH);
    memcpy(resp2->associated_uri, reg_state->associated_uri, VOLTE_MAX_ASSOCIATED_URI);
    memcpy(resp2->instance_id, reg_state->instance_id, VOLTE_MAX_URI_LENGTH);
    memcpy(resp2->service_route, reg_state->service_route, VOLTE_MAX_URI_LENGTH);
    memcpy(resp2->home_uri, reg_state->home_uri, VOLTE_MAX_DOMAIN_NAME_LENGTH);
    if (reg_state->state == VoLTE_Event_Reg_State_Registered && reg_state->digit_number_of_vline > 0) {
        resp2->digit_number_of_vline = reg_state->digit_number_of_vline;
        memcpy(resp2->digit_pidentifier, reg_state->digit_pidentifier, VOLTE_MAX_DIGIT_PIDENTIFIER_LENGTH);
        memcpy(resp2->digit_ppa_header_string, reg_state->digit_ppa_header_string, VOLTE_MAX_DIGIT_PPA_LENGTH);
        DbgMsg("ClientAPI_UA ==> digit_number_of_vline : %d, pidentifier: %s, ppa: %s"
                "\n orig digit_number_of_vline : %d, pidentifier: %s, ppa: %s",
                reg_state->digit_number_of_vline, reg_state->digit_pidentifier, reg_state->digit_ppa_header_string,
                resp2->digit_number_of_vline, resp2->digit_pidentifier, resp2->digit_ppa_header_string);
    }
    memcpy(event->data, resp2, event->data_len);
    CLIENTAPI_COMM_Recv(event);
    free(resp2);

free_mutex:
    pthread_mutex_unlock(&reg_mutex);
    DbgMsg("reg_mutex out");
}

void handle_ims_dereg_start(VoLTE_Stack_Message_t *msg, void *user_data) {
    CLIENTAPI_EventMsg_t *event = 0;
    void *notused = user_data;

    event = (CLIENTAPI_EventMsg_t *)CLIENTAPI_COMM_GetEvent(sizeof(VoLTE_Event_Reg_Dereg_Start_t));
    event->request_id = EVENT_IMS_DEREGISTER_IND;
    event->source = EventSource_IMS_DISPATCHER;
    event->data_len = sizeof(VoLTE_Event_Reg_Dereg_Start_t);
    memcpy(event->data, msg->data, event->data_len);

    DbgMsg("ClientAPI_UA ==> ClientAPI_ADAPTER : CLIENTAPI_PROXY_EVENT_IND_IMS_DEREG_START");
    CLIENTAPI_COMM_Send(event);
}

void handle_ims_dereg_unpublish_done(CLIENTAPI_EventMsg_t *event) {
    volte_stack_obj_t *obj = (volte_stack_obj_t *)g_stack_channel_event;
    Channel_Data_t data = {};
    VoLTE_Event_Presence_Unpublish_t unpublish;
    int ret = 0;

    if (!obj) {
        ErrMsg("stack object is NULL");
        return;
    } else if (obj->reg_id < 0) {
        ErrMsg("stack object unbind");
        return;
    }

    unpublish.acct_id = event->data[0];
    data.type = VOLTE_CNF_SIPTX_PRESENCE_UNPUBLISH;
    data.len  = sizeof(VoLTE_Event_Presence_Unpublish_t);
    data.flag = 0;
    data.data = &unpublish;

    ret = Channel_Send(obj->channel, &data);
    if (ret < 0) {
        ErrMsg("Can't send the sip message to volte sip stack");
        return;
    }
}

void handle_ims_recover_reg() {
        if (!g_stack_channel_event) {
            ErrMsg("g_stack_channel_event is null");
            return;
        }
        if (volte_sip_stack_restore == NULL) {
            DbgMsg("Not support on the ap verion");
            return;
        }
        DbgMsg("ClientAPI_UA ==> notify ims for recover status of register");
        volte_sip_stack_restore(g_stack_channel_event);
        return;
}

