#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "debug.h"
#include "rcs_proxy_dispatcher.h"
#include "rcs_proxy_adapter.h"
#include "debug.h"
#include "api_timer.h"
#include "rcs_volte_core_interface.h"
#include <cutils/properties.h>

#define TIMER_LIST_NUM (20)

extern int gROISupported;
extern char gSimOptr[32];


static Timer_List_t g_timer_list[TIMER_LIST_NUM];
void *g_stack_channel_sip = 0;
void *g_roi_stack_channel_sip = 0;
static void *g_stack_channel_event = 0;
static void *g_roi_stack_channel_event = 0;
//bool to request that the rcs_proxy_request was sent or not
//in case of deregister itr will be set to 0
//static int rcs_proxy_request_sent = 0;

static int g_rcs_register_request_sent = 0;
static int g_rcs_deregister_request_sent = 0;
static int g_ims_registered_owner = OWNER_INVALID; // 0: VoLTE, 1:RCS
static int gIgnoreEvent = 0;
static pthread_mutex_t reg_mutex = PTHREAD_MUTEX_INITIALIZER;

static void print_bytes(const void *object, size_t size) {

    size_t i;

    for (i = 0; i < size; i++) {
        DbgMsg("%02x ", ((const unsigned char*)object)[i] & 0xff);
    }
}

static void read_sip_msg(VoLTE_Stack_Sip_Info_t *sip, void *user_data) {

    int i = 0;
    char print_buff[53];
    void* temp = user_data;
    if(!sip) {
        ErrMsg("incoming sip message is null & exit");
        exit(-1);
    }

    DbgMsg(
        "<<< SIP MSG RECEIVED :\n request_id = %d,conn_id = %d, len = %d, \n message = %s",
        sip->request_id,
        sip->conn_id,
        sip->message_len,
        sip->message);
    RCS_EventMsg_t *event = 0;
    //size of volte sip stack + message length
    int intsize = sizeof(int);
    event = RCS_COMM_GetEvent(3*intsize + sip->message_len);
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
    RCS_COMM_Recv(event);

    DbgMsg("SIP message sent to RCS");

    return;
}

void send_sip_message(int req_id, int conn_id, char *sip_message, int length) {

    sipmsgrsp *sipmsgStr;

    DbgMsg("send_sip_message for owner[%d]", g_ims_registered_owner);

    if(!sip_message) {
        ErrMsg("sip message is null");
        return;
    }

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

    DbgMsg(">>> send_msg details req_id[%d], conn_id[%d], SIP_msg length[%d]", sip_info->request_id, sip_info->conn_id, sip_info->message_len);

#if 0
    DbgMsg(">>> %s ",sip_info->message);
#endif

    if (OWNER_ROI_STACK == g_ims_registered_owner) {
        /* send SIP to rcs_volte_stack */

        roi_volte_sip_stack_send_sip(g_roi_stack_channel_sip, sip_info);
        roi_volte_sip_info_free(sip_info);
    } else if (OWNER_VOLTE_STACK == g_ims_registered_owner) {
        /* send SIP to volte_stack */

        volte_sip_stack_send_sip(g_stack_channel_sip, sip_info);
        volte_sip_info_free(sip_info);
    }
}

void handle_Cmd_Req_Reg_Info(void) {

    DbgMsg("send registartion state request to the dispatcher");

    //send request to get the registrtaion state
    volte_sip_stack_reg_state(g_stack_channel_event);
}

void handle_Cmd_Ims_Unpublish_Completed(RCS_EventMsg_t *msg) {
    volte_stack_obj_t *obj = (volte_stack_obj_t *)g_stack_channel_event;
    Channel_Data_t data = {};
    VoLTE_Event_Presence_Unpublish_t unpublish;
    int ret = 0;

    if (!obj) {
        return;
    } else if (obj->reg_id < 0) {
        return;
    }

    unpublish.acct_id = msg->data[0];
    data.type = VOLTE_CNF_SIPTX_PRESENCE_UNPUBLISH;
    data.len = sizeof(VoLTE_Event_Presence_Unpublish_t);
    data.flag = 0;
    data.data = &unpublish;

    ret = Channel_Send(obj->channel, &data);
    if (ret < 0) {
        DbgMsg("Can't send the sip message to volte sip stack");
        return;
    }
}

void handle_Cmd_Active_Rcs_Slot_Id(RCS_EventMsg_t *msg) {
    char property_value[92] = {0};
    int old_slot_id;
    int *raw_data = (int*)msg->data;
    int new_slot_id = *raw_data;
    int acct_id[2] = {0, 2};

    property_get("persist.vendor.active.rcs.slot.id", property_value, "0");
    old_slot_id = atoi(property_value);

    DbgMsg("handle_Cmd_Active_Rcs_Slot_Id, old_slot_id = %d, new_slot_id = %d",
        old_slot_id, new_slot_id);
    //get prop, return if no change
    if (new_slot_id == old_slot_id) {
        return;
    }

    //unbind old acct id
    volte_sip_stack_reg_unbind(g_stack_channel_event);
    if (g_stack_channel_sip) {
        volte_sip_stack_destroy(g_stack_channel_sip);
        g_stack_channel_sip = 0;
    }

    //bind new acct id
    volte_sip_stack_reg_bind(g_stack_channel_event, acct_id[new_slot_id]);

    //set property value for new slot id
    asprintf(&property_value, "%d", new_slot_id);
    property_set("persist.vendor.active.rcs.slot.id", property_value);
}

void handle_Cmd_Ims_Restoration(RCS_EventMsg_t *msg) {
    if (volte_sip_stack_restore != NULL && g_stack_channel_event != NULL) {
        DbgMsg("IMS restoration");
        volte_sip_stack_restore(g_stack_channel_event);
    }
}

int RCS_dispatcher_add_capability(char *capability_tag) {

    int status  = -1;
    DbgMsg("RCS_UA add capability : (%s) ",capability_tag);

    //if g_rcs_register_request_sent is sent, wait for the response
    if(g_rcs_register_request_sent == 1){
        DbgMsg("RCS_UA add capability already sent");
        return VoLTE_Stack_Result_Success;
    }


    //urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.session  [IM]
    //urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.filetransfer [FT]

    //add capability to the stack
    status = volte_sip_stack_reg_capability(g_stack_channel_event, 1, capability_tag);

    if(status == VoLTE_Stack_Result_Success){
        DbgMsg("Capability add status : SUCCESS ");
        //register request sent
        //wait for the register request result
        g_rcs_register_request_sent = 1;
    }else{
        DbgMsg("Capability add status : FAIL ; reason : %d",status);
        g_rcs_register_request_sent = 0;
    }
    return status;
}

void RCS_dispatcher_delete_capability(char *capability_tag) {
    /* send de-registartion request for rcs by deleting rcs capability
     * and disable boolean rcs_proxy_request_sent to 0 for nettype
     */

    //if g_rcs_register_request_sent is sent, wait for the response
    if(g_rcs_deregister_request_sent == 1) {
        DbgMsg("RCS_UA delete capability already sent");
        return ;
    }

    int status = volte_sip_stack_reg_capability(g_stack_channel_event, 0, capability_tag);

    if(status == VoLTE_Stack_Result_Success) {
        DbgMsg("Capability delete status : SUCCESS . Wait for REG notify");

        //de-register request sent
        //wait for the de-register request result
        g_rcs_deregister_request_sent = 1;
    } else {
        DbgMsg("Capability delete status : FAIL ; reason : %d",status);
        g_rcs_deregister_request_sent = 0;
    }

    if (g_stack_channel_sip)
    {
        DbgMsg("Disconnect to stack");
        volte_sip_stack_destroy(g_stack_channel_sip);
        DbgMsg("g_stack_channel_sip destroyed");
        g_stack_channel_sip = 0;
    }
}

void create_SIP_Stack_connection(VoLTE_Event_Reg_State_t *reg_state) {

    int rule_init_status =  -1;

    DbgMsg("create_SIP_Stack_connection");

    VoLTE_Stack_Channel_Info_t info;
    VoLTE_Stack_Callback_t callback;
    VoLTE_Stack_Ua_Capability_t cap;

    if (isImsSubmarineSupported()) {
        info.type = VoLTE_Stack_UA_RCS_Proxy_Sip | VoLTE_UA_MAGIC_NUMBER;
    } else {
        info.type = VoLTE_Stack_UA_RCS_Proxy | VoLTE_UA_MAGIC_NUMBER;
    }
    info.conn_info = reg_state->conn_info;

    callback.read_sip = read_sip_msg;
    callback.read_msg = 0;
    callback.user_data = 0;

    Rule_Capability_Init(&cap);
    /* DbgMsg("RCS[%d] public_uid = %s, associated_uri = %s", reg_state->id, reg_state->public_uid, reg_state->associated_uri); */
    Rule_Level0_Set(&cap, reg_state->public_uid, reg_state->associated_uri);


    Rule_Level2_Set(&cap, SIP_MESSAGE_METHOD, SIP_HDR_CONTENT_TYPE, "application/rcs");
    Rule_Level2_Set(&cap, SIP_NOTIFY_METHOD, SIP_HDR_EVENT, "presence");
    rule_init_status = Rule_Level3_Set(&cap, "cpm");
    DbgMsg("RCS[%d] Connect to stack, create sip channel", reg_state->id);
    g_stack_channel_sip = volte_sip_stack_create(&info, &cap, &callback);
    Rule_Capability_Deinit(&cap);

    if (!g_stack_channel_sip)
    {
        ErrMsg("Can't create the channel to sip channel stack");
        return;
    }else{
        DbgMsg("RCS[%d] Connect to stack, create sip channel : SUCCESS ",reg_state->id);
    }
}

void create_Roi_SIP_Stack_connection(VoLTE_Event_Reg_State_t *reg_state) {

    int rule_init_status =  -1;

    if (rcsVolteApiHandler == NULL) {
        return;
    }
    DbgMsg("create_Roi_SIP_Stack_connection");

    VoLTE_Stack_Channel_Info_t info;
    VoLTE_Stack_Callback_t callback;
    VoLTE_Stack_Ua_Capability_t cap;

    info.type = VoLTE_Stack_UA_RCS_Proxy | VoLTE_UA_MAGIC_NUMBER;
    info.conn_info = reg_state->conn_info;

    callback.read_sip = read_sip_msg;
    callback.read_msg = 0;
    callback.user_data = 0;

    ROI_Rule_Capability_Init(&cap);
    /* DbgMsg("RCS[%d] public_uid = %s, associated_uri = %s", reg_state->id, reg_state->public_uid, reg_state->associated_uri); */
    ROI_Rule_Level0_Set(&cap, reg_state->public_uid, reg_state->associated_uri);



    ROI_Rule_Level2_Set(&cap, SIP_MESSAGE_METHOD, SIP_HDR_CONTENT_TYPE, "application/rcs");
    ROI_Rule_Level2_Set(&cap, SIP_NOTIFY_METHOD, SIP_HDR_EVENT, "presence");
    rule_init_status = ROI_Rule_Level3_Set(&cap, "cpm");
    DbgMsg("RCS[%d] Connect to stack, create sip channel", reg_state->id);
    g_roi_stack_channel_sip = roi_volte_sip_stack_create(&info, &cap, &callback);
    ROI_Rule_Capability_Deinit(&cap);

    if (!g_roi_stack_channel_sip) {
        ErrMsg("Can't create the channel to roi sip channel stack");
        return;
    } else {
        DbgMsg("RCS[%d] Connect to stack, create roi sip channel : SUCCESS ",reg_state->id);
    }
}

static void volte_read_msg_cb(VoLTE_Stack_Message_t *msg, void *user_data) {

    void* temp = user_data;//not used

    if (!msg) {
        ErrMsg("Stack channel is disconnected & exit");
        exit(-1);
    }

    switch (msg->type)
    {
        case VOLTE_IND_SIPTX_REG_STATE:
            DbgMsg("<<< volte_read_msg_cb, type: VOLTE_IND_SIPTX_REG_STATE");
            handle_ims_reg_state(msg, OWNER_VOLTE_STACK);
            break;

        case VOLTE_IND_SIPTX_REG_DEREG_START:
            DbgMsg("<< volte_read_msg_cb, type: VOLTE_IND_SIPTX_REG_DEREG_START");
            handle_ims_dereg_start(msg, OWNER_VOLTE_STACK);
            break;

        default:
#if 0
            DbgMsg("un-supported message: %d", msg->type);
#endif
            break;
    }
}


char* getVolteEvent(int type) {

    switch (type) {
        case VOLTE_IND_SIPTX_REG_STATE:             return "VOLTE_IND_SIPTX_REG_STATE";
        case VOLTE_IND_SIPTX_REG_DEREG_START:       return "VOLTE_IND_SIPTX_REG_DEREG_START";
        case VOLTE_IND_SIPTX_REG_AUTH_REQ:          return "VOLTE_IND_SIPTX_REG_AUTH_REQ";
        case VOLTE_IND_SIPTX_GEOLOCATION_REQ:       return "VOLTE_IND_SIPTX_GEOLOCATION_REQ";
        case VOLTE_IND_SIPTX_REG_QUERY_STATE:       return "VOLTE_IND_SIPTX_REG_QUERY_STATE";
        case VOLTE_IND_SIPTX_EMS_MODE:              return "VOLTE_IND_SIPTX_EMS_MODE";
        case VOLTE_IND_SIPTX_REG_TIMER_START:       return "VOLTE_IND_SIPTX_REG_TIMER_START";
        case VOLTE_IND_SIPTX_REG_TIMER_CANCEL:      return "VOLTE_IND_SIPTX_REG_TIMER_CANCEL";
        case VOLTE_IND_IPSEC_POLICY_FLUSH:          return "VOLTE_IND_IPSEC_POLICY_FLUSH";
        case VOLTE_IND_IPSEC_POLICY_ADD:            return "VOLTE_IND_IPSEC_POLICY_ADD";
        case VOLTE_IND_IPSEC_POLICY_DELETE:         return "VOLTE_IND_IPSEC_POLICY_DELETE";
        default:                                    return "unknown_volte_event";
    }
}


static void roi_read_msg_cb(VoLTE_Stack_Message_t *msg, void *user_data) {

    RCS_UNUSED(user_data); //current not used

    if (!msg) {
        ErrMsg("ROI Stack channel is disconnected");
        return;
    }

    DbgMsg("<<< roi_read_msg_cb, type: %s", getVolteEvent(msg->type));

    switch (msg->type) {
        case VOLTE_IND_SIPTX_REG_STATE:
            handle_ims_reg_state(msg, OWNER_ROI_STACK);
            break;

        case VOLTE_IND_SIPTX_REG_DEREG_START:
            handle_ims_dereg_start(msg, OWNER_ROI_STACK);
            break;

        // RCS for 3g volte
        case VOLTE_IND_SIPTX_REG_AUTH_REQ:
            handle_Volte_Ind_Siptx_Reg_Auth_Req(msg);
            break;

        case VOLTE_IND_SIPTX_GEOLOCATION_REQ:
            handle_Volte_Ind_Siptx_Geolocation_Req(msg);
            break;
        case VOLTE_IND_SIPTX_REG_QUERY_STATE:
            handle_Volte_Ind_Siptx_Reg_Query_State(msg);
            break;
        case VOLTE_IND_SIPTX_EMS_MODE:
            handle_Volte_Ind_Siptx_Ems_Mode(msg);
            break;

        case VOLTE_IND_SIPTX_REG_TIMER_START:
        case VOLTE_IND_SIPTX_REG_TIMER_CANCEL:
            handle_Volte_Ind_Siptx_Reg_Timer(msg);
            break;

        case VOLTE_IND_IPSEC_POLICY_FLUSH:
        case VOLTE_IND_IPSEC_POLICY_ADD:
        case VOLTE_IND_IPSEC_POLICY_DELETE:
            break;

        case VOLTE_IND_ATCMD_RELAY:
            handle_Volte_Ind_Atcmd_Relay(msg);
            break;
        default:
            //ErrMsg("un-supported message: %d", msg->type);
            break;
    }
}

void handle_Volte_Ind_Siptx_Reg_Auth_Req(VoLTE_Stack_Message_t *msg) {

    // VoLTE_Event_Reg_Auth_Req_t
    // RCS_PROXY_EVENT_REG_AUTH_REQ
    Roi_Event_Reg_Auth_Req_t *input_data = (Roi_Event_Reg_Auth_Req_t*) msg->data;
    RCS_EventMsg_t *event = 0;
    DbgMsg("%s", __FUNCTION__);

    if (msg->len != sizeof(Roi_Event_Reg_Auth_Req_t))
    {
        ErrMsg("Data length error! Stack msg VOLTE_IND_SIPTX_REG_AUTH_REQ data len:%d, expected:%d",
            msg->len, sizeof(Roi_Event_Reg_Auth_Req_t) );
        return;
    }
    /**
     * SEND RESPONSE TO THE UA
     */
    Roi_Event_Reg_Auth_Req_t resp ={0,{0},0,0,0};

    /* notify the event and put it in the main thread */
    event = RCS_COMM_GetEvent(sizeof(Roi_Event_Reg_Auth_Req_t));
    event->source = EventSource_IMS_DISPATCHER;
    event->request_id = EVENT_IMS_AUTH_REQ;
    event->data_len = sizeof(Roi_Event_Reg_Auth_Req_t);

    resp.id = input_data->id;
    memcpy(resp.nonce, input_data->nonce, VOLTE_MAX_AUTH_NONCE);
    resp.encry_algo = input_data->encry_algo;
    resp.integrity_algo = input_data->integrity_algo;
    resp.user_data = input_data->user_data;

#if 0
    DbgMsg("id:%d, encry_algo:%d, nonce:[%s], integrity_algo:%d, user_data:%d",
        resp.id, resp.encry_algo, resp.nonce ,resp.integrity_algo, resp.user_data);
#endif

    memcpy(event->data, &resp, event->data_len);

    /* add event to queue */
    RCS_COMM_Recv(event);
}

void handle_Volte_Ind_Siptx_Geolocation_Req(VoLTE_Stack_Message_t *msg) {

    // VoLTE_Event_Geolocation_Info_t
    // RCS_PROXY_EVENT_GEOLOCATION_REQ
    Roi_Event_Geolocation_Info_t *input_data = (Roi_Event_Geolocation_Info_t*) msg->data;
    RCS_EventMsg_t *event = 0;
    DbgMsg("%s", __FUNCTION__);

    if (msg->len != sizeof(Roi_Event_Geolocation_Info_t))
    {
        ErrMsg("Data length error! Stack msg VOLTE_IND_SIPTX_REG_AUTH_REQ data len:%d, expected:%d",
            msg->len, sizeof(Roi_Event_Geolocation_Info_t) );
        return;
    }
    /**
     * SEND RESPONSE TO THE UA
     */

#if 0
    //Dbg only
    Roi_Event_UA_Msg_Hdr_t* ua_msg_hdr = &(input_data->ua_msg_hdr);
    DbgMsg("ua_msg_hdr id:%d, hdsk:%u, resv:[%02X][%02X][%02X], data[%02X]",
        ua_msg_hdr->id, ua_msg_hdr->handshake,
        ua_msg_hdr->resv[0], ua_msg_hdr->resv[1], ua_msg_hdr->resv[2],
        ua_msg_hdr->data[0]);
    DbgMsg("type_id:%u, lat:[%g], lon:[%g]",
        input_data->type_id, *(double *)&input_data->latitude ,
        *(double *)&input_data->longitude);
#endif

    /* notify the event and put it in the main thread */
    event = RCS_COMM_GetEvent(sizeof(Roi_Event_Geolocation_Info_t));
    event->source = EventSource_IMS_DISPATCHER;
    event->request_id = EVENT_IMS_GEOLOCATION_REQ;
    event->data_len = sizeof(Roi_Event_Geolocation_Info_t);
    memcpy(event->data, input_data, event->data_len);

    /* add event to queue */
    RCS_COMM_Recv(event);
}

void handle_Volte_Ind_Siptx_Reg_Query_State(VoLTE_Stack_Message_t *msg) {

    // VoLTE_Event_Query_State_t
    // RCS_PROXY_EVENT_REG_QUERY_STATE
    Roi_Event_Query_State_t *input_data = (Roi_Event_Query_State_t*) msg->data;
    RCS_EventMsg_t *event = 0;
    DbgMsg("%s", __FUNCTION__);

    if (msg->len != sizeof(Roi_Event_Query_State_t))
    {
        ErrMsg("Data length error! Stack msg VOLTE_IND_SIPTX_REG_AUTH_REQ data len:%d, expected:%d",
            msg->len, sizeof(Roi_Event_Query_State_t) );
        return;
    }
    /**
     * SEND RESPONSE TO THE UA
     */

#if 0
    //Dbg only
    DbgMsg("id:%d, type:%d", input_data->id, input_data->type);
#endif

    /* notify the event and put it in the main thread */
    event = RCS_COMM_GetEvent(sizeof(Roi_Event_Query_State_t));
    event->source = EventSource_IMS_DISPATCHER;
    event->request_id = EVENT_IMS_QUERY_STATE;
    event->data_len = sizeof(Roi_Event_Query_State_t);
    memcpy(event->data, input_data, event->data_len);

    /* add event to queue */
    RCS_COMM_Recv(event);
}

void handle_Volte_Ind_Siptx_Ems_Mode(VoLTE_Stack_Message_t *msg) {

    // VoLTE_Event_Reg_Ems_Mode_t
    // RCS_PROXY_EVENT_EMS_MODE
    Roi_Event_Reg_Ems_Mode_t *input_data = (Roi_Event_Reg_Ems_Mode_t*) msg->data;
    RCS_EventMsg_t *event = 0;
    DbgMsg("%s", __FUNCTION__);

    if (msg->len != sizeof(Roi_Event_Reg_Ems_Mode_t)) {
        ErrMsg("Data length error! Stack msg VOLTE_IND_SIPTX_REG_AUTH_REQ data len:%d, expected:%d",
            msg->len, sizeof(Roi_Event_Reg_Ems_Mode_t) );
        return;
    }
    /**
     * SEND RESPONSE TO THE UA
     */

#if 0
    //Dbg only
    DbgMsg("id:%d, ems_mode:%d, rat_type:%d, reg_service:%d",
        input_data->id, input_data->ems_mode, input_data->rat_type, input_data->reg_service);
#endif

    /* notify the event and put it in the main thread */
    event = RCS_COMM_GetEvent(sizeof(Roi_Event_Reg_Ems_Mode_t));
    event->source = EventSource_IMS_DISPATCHER;
    event->request_id = EVENT_IMS_EMS_MODE_INFO;
    event->data_len = sizeof(Roi_Event_Reg_Ems_Mode_t);
    memcpy(event->data, input_data, event->data_len);

    /* add event to queue */
    RCS_COMM_Recv(event);
}

void volte_reg_timer_expiry(sigval_t sigval) {

    DbgMsg("%s", __func__);

    VoLTE_Event_Timer_t *timer_t = (VoLTE_Event_Timer_t*) sigval.sival_ptr;
    VoLTE_Stack_Message_t new_msg;
    DbgMsg("timer_t acct_id:%d, timer_id:%d, timeout:%d, data:%d", timer_t->id, timer_t->timer_id, timer_t->timeout, timer_t->user_data);
    VoLTE_Stack_Message_Set(&new_msg, VOLTE_CNF_SIPTX_REG_TIMER_EXPIRY, sizeof(VoLTE_Event_Timer_t), 0, timer_t);
    roi_volte_sip_stack_send_msg(g_roi_stack_channel_event, &new_msg);
}

void start_volte_reg_timer(VoLTE_Event_Timer_t *timer) {

    int i = 0;
    unsigned ms = timer->timeout * 1000;
    VoLTE_Event_Timer_t *timer_t = (VoLTE_Event_Timer_t *) malloc(sizeof(VoLTE_Event_Timer_t));
    memset(timer_t, 0, sizeof(VoLTE_Event_Timer_t));
    memcpy(timer_t, timer, sizeof(VoLTE_Event_Timer_t));
    void *tid = TIMER_Create(volte_reg_timer_expiry, timer_t, ms);
    if (!tid) {
        DbgMsg("timer create failed!");
        return;
    }

    int stored = 0;
    for (i=0 ; i<=TIMER_LIST_NUM-1 ; i++) {
        if (!g_timer_list[i].timer) {
            g_timer_list[i].timer = timer_t;
            g_timer_list[i].ctimer = tid;

            ctimer_t *t = g_timer_list[i].ctimer;
            DbgMsg("id=%d, timer_id=%d, ctimer->timerid=%d", timer->id, timer->timer_id, (int)t->timerid);
            stored = 1;
            break;
        }
    }

    if (!stored) {
        DbgMsg("timer fails to store!");
    }
}

void stop_volte_reg_timer(VoLTE_Event_Timer_t *timer) {

    DbgMsg("%s", __func__);

    int i = 0;
    for (i=0 ; i<TIMER_LIST_NUM-1 ; i++) {
        if (g_timer_list[i].timer) {
            if (g_timer_list[i].timer->timer_id == timer->timer_id
                    && g_timer_list[i].timer->id == timer->id
                    && g_timer_list[i].timer->user_data == timer->user_data) {
                free(g_timer_list[i].timer);
                g_timer_list[i].timer = 0;
                TIMER_Free(g_timer_list[i].ctimer);
                g_timer_list[i].ctimer = 0;
                break;
            }
        }
    }
}

void handle_Volte_Ind_Siptx_Reg_Timer(VoLTE_Stack_Message_t *msg) {

    if (msg->type == VOLTE_IND_SIPTX_REG_TIMER_START) {
        VoLTE_Event_Timer_t *timer = (VoLTE_Event_Timer_t*) msg->data;
        start_volte_reg_timer(timer);
    } else if (msg->type == VOLTE_IND_SIPTX_REG_TIMER_CANCEL) {
        VoLTE_Event_Timer_t *timer = (VoLTE_Event_Timer_t*) msg->data;
        stop_volte_reg_timer(timer);
    }
}

void handle_Volte_Ind_Atcmd_Relay(VoLTE_Stack_Message_t *msg) {
        // VoLTE_Event_ATCmd_Relay_t
        // VOLTE_IND_ATCMD_RELAY
        Roi_Event_ATCmd_Relay_t *input_data = (Roi_Event_ATCmd_Relay_t*) msg->data;
        RCS_EventMsg_t *event = 0;
        DbgMsg("%s", __FUNCTION__);

        if (msg->len != sizeof(Roi_Event_ATCmd_Relay_t)) {
            ErrMsg("Data length error! Stack msg Roi_Event_ATCmd_Relay_t data len:%d, expected:%d",
                msg->len, sizeof(Roi_Event_ATCmd_Relay_t) );
            return;
        }

#if 1
        //Dbg only
        DbgMsg("account_id:%d, atcmd_id:%d, cmdline:%s",
            input_data->account_id, input_data->atcmd_id, input_data->cmdline);
#endif


        if (input_data->atcmd_id == VoLTE_ATCMD_DIGITLINE) {

            /* notify the event and put it in the main thread */
            event = RCS_COMM_GetEvent(sizeof(Roi_Event_ATCmd_Relay_t));
            event->source = EventSource_IMS_DISPATCHER;
            event->request_id = EVENT_IMS_DIGITLING_REG_IND;
            event->data_len = sizeof(Roi_Event_ATCmd_Relay_t);
            memcpy(event->data, input_data, event->data_len);

            /* add event to queue */
            RCS_COMM_Recv(event);
        } else {
            DbgMsg("un-supported AT command");
            return;
        }
}

int RCS_Proxy_Init(void) {

    VoLTE_Stack_Channel_Info_t info;
    VoLTE_Stack_Callback_t callback;

    volte_sip_stack_init();

    /* configure the SIP channel information */
    memset(&info, 0, sizeof(VoLTE_Stack_Channel_Info_t));

    info.type = VoLTE_Stack_UA_RCS_Proxy | VoLTE_UA_MAGIC_NUMBER;

    /* configure the callback function information */
    memset(&callback, 0, sizeof(VoLTE_Stack_Callback_t));
    callback.read_msg = volte_read_msg_cb;
    callback.user_data = 0;

    g_stack_channel_event = volte_sip_stack_create(&info, 0, &callback);
    if (!g_stack_channel_event) {
        ErrMsg("Can't create the channel to stack");
        return -1;
    }

    char property_value[92] = {0};
    int slot_id;
    int acct_id[2] = {0, 2};

    property_get("persist.vendor.active.rcs.slot.id", property_value, "0");
    slot_id = atoi(property_value);

    volte_sip_stack_reg_bind(g_stack_channel_event, acct_id[slot_id]);

    //set presence enable state to ims stack
    //0: disable, 1: enable
    if (volte_sip_stack_ind_external_ua_state != NULL) {
        DbgMsg("RCS[%d] volte_sip_stack_ind_external_ua_state", 0);
        volte_sip_stack_ind_external_ua_state(
                g_stack_channel_event, acct_id[slot_id], info.type, 1);
    }

    DbgMsg("RCS[%d] RCS_Proxy_Init successful", acct_id[slot_id]);
    return 0;
}

int RCS_Roi_Proxy_Init(void) {

    VoLTE_Stack_Channel_Info_t info;
    VoLTE_Stack_Callback_t callback;
    int acct_id = 0;


    roi_volte_sip_stack_init();

    /* configure the SIP channel information */
    memset(&info, 0, sizeof(VoLTE_Stack_Channel_Info_t));

    info.type = VoLTE_Stack_UA_RCS_Proxy | VoLTE_UA_MAGIC_NUMBER;

    /* configure the callback function information */
    memset(g_timer_list, 0, sizeof(Timer_List_t) * TIMER_LIST_NUM);
    memset(&callback, 0, sizeof(VoLTE_Stack_Callback_t));
    callback.read_msg = roi_read_msg_cb;
    callback.user_data = 0;

    g_roi_stack_channel_event = roi_volte_sip_stack_create(&info, 0, &callback);

    if (!g_roi_stack_channel_event) {
        ErrMsg("Can't create the channel to stack");
        return -1;
    }

    roi_volte_sip_stack_reg_bind(g_roi_stack_channel_event, acct_id);

    DbgMsg("RCS[%d] RCS_Roi_Proxy_Init successful", 0);
    return 0;
}

void handle_Cmd_Ims_Register(RCS_EventMsg_t *event) {

    void *nouse = event;
    int result; // Success:0, Error:-1, Timeout:-2, Disconnected: -3

    result = roi_sip_stack_reg_operation(g_roi_stack_channel_event,
            REG_OP_REGISTER);
    DbgMsg("handle_Cmd_Ims_Register result = %d", result);
}

void handle_Cmd_Ims_Deregister(RCS_EventMsg_t *event) {

    void *nouse = event;
    int result; // Success:0, Error:-1, Timeout:-2, Disconnected: -3

    if (g_ims_registered_owner == OWNER_ROI_STACK) {
        result = roi_sip_stack_reg_operation(g_roi_stack_channel_event,
                REG_OP_DEREGISTER);
        DbgMsg("handle_Cmd_Ims_Deregister result = %d", result);
    } else if (g_ims_registered_owner == OWNER_VOLTE_STACK) {
        DbgMsg("handle_Cmd_Ims_Deregister => ignored!");
    }
}

// RCS for 3g volte
void handle_Cmd_Ims_Notify_Vops_Info(RCS_EventMsg_t *event) {

    //VOLTE_REQ_SIPTX_REG_NOTIFY_IMSVOPS
    //VoLTE_Event_Notify_t
    //volte_sip_stack_send_msg

    VoLTE_Stack_Message_t new_msg;
    Roi_Event_Notify_t* rcs_data;
    rcs_data = (Roi_Event_Notify_t*)event->data;

#if 0
    DbgMsg("acct_id:%d, data:%d", rcs_data->acct_id, rcs_data->data);
#endif

    VoLTE_Stack_Message_Set(&new_msg, VOLTE_REQ_SIPTX_REG_NOTIFY_IMSVOPS, sizeof(Roi_Event_Notify_t), 0, rcs_data);
    roi_volte_sip_stack_send_msg(g_roi_stack_channel_event, &new_msg);
};

void handle_Cmd_Ims_Update_Rt_Settings(RCS_EventMsg_t *event) {

    //VOLTE_REQ_SIPTX_REG_UPDATE_SETTING
    //VoLTE_Event_Setting_t
    //volte_sip_stack_send_msg

    int data_len = 0, i;
    VoLTE_Stack_Message_t new_msg;
    Roi_Event_Setting_t *stack_setting;
    Roi_Event_Setting_TLV_t *stack_setting_tlv = NULL;
    char* ptr;
    ptr = (char *) event->data;
    stack_setting = (Roi_Event_Setting_t*)event->data;
    ptr = (char *) stack_setting->data;

#if 0
    DbgMsg("settings id:%d, num:%d", stack_setting->id, stack_setting->num);
#endif

    for (i = 0; i < stack_setting->num; i++) {
        stack_setting_tlv = (Roi_Event_Setting_TLV_t*) ptr;
        data_len = data_len + sizeof(Roi_Event_Setting_TLV_t) + stack_setting_tlv->len;

#if 0
        if(stack_setting_tlv->len == 4 || stack_setting_tlv->len == 1) {
            //DbgMsg("tlv[%d] type:%d, len:%d, val:%d", i, stack_setting_tlv->type,
            //    stack_setting_tlv->len, *(int*)stack_setting_tlv->value);
            DbgMsg("tlv[%d] type:%d, len:%d, val:%d, ntohl_cal:%d, asString:%s", i, stack_setting_tlv->type,
                stack_setting_tlv->len, *(int*)stack_setting_tlv->value,
                ntohl(*(int*)stack_setting_tlv->value), (char*)stack_setting_tlv->value);
        } else {
            DbgMsg("tlv[%d] type:%d, len:%d, val:%s", i, stack_setting_tlv->type,
                stack_setting_tlv->len, (char*)stack_setting_tlv->value);
        }
#endif

        ptr += (sizeof(Roi_Event_Setting_TLV_t) + stack_setting_tlv->len);
    }
    stack_setting->len = data_len;

#if 0
    DbgMsg("settings len:%d", stack_setting->len);
#endif

    VoLTE_Stack_Message_Set(&new_msg, VOLTE_REQ_SIPTX_REG_UPDATE_SETTING,
        sizeof(Roi_Event_Setting_t) + stack_setting->len, 0, stack_setting);
    roi_volte_sip_stack_send_msg(g_roi_stack_channel_event, &new_msg);
}


void handle_Cmd_Ims_Update_Settings(RCS_EventMsg_t *event) {

    //VOLTE_REQ_SIPTX_SETTING
    //VoLTE_Event_Setting_t
    //volte_sip_stack_update_setting

    int data_len = 0, i;
    Roi_Event_Setting_t *stack_setting;
    Roi_Event_Setting_TLV_t *stack_setting_tlv = NULL;
    char* ptr;
    stack_setting = (Roi_Event_Setting_t*)event->data;
    ptr = (char *) stack_setting->data;
    DbgMsg("settings id:%d, num:%d", stack_setting->id, stack_setting->num);
    for (i = 0; i < stack_setting->num; i++) {
        stack_setting_tlv = (Roi_Event_Setting_TLV_t*) ptr;
        data_len = data_len + sizeof(Roi_Event_Setting_TLV_t) + stack_setting_tlv->len;

#if 0
        if(stack_setting_tlv->len == 4 || stack_setting_tlv->len == 1) {
            DbgMsg("tlv[%d] type:%d, len:%d, val:%d", i, stack_setting_tlv->type,
                stack_setting_tlv->len, *(int*)stack_setting_tlv->value);
        } else {
            DbgMsg("tlv[%d] type:%d, len:%d, val:%s", i, stack_setting_tlv->type,
                stack_setting_tlv->len, (char*)stack_setting_tlv->value);
        }
#endif

        ptr += (sizeof(Roi_Event_Setting_TLV_t) + stack_setting_tlv->len);
    }
    stack_setting->len = data_len;

#if 0
    DbgMsg("settings len:%d", stack_setting->len);
#endif

    roi_sip_stack_update_setting(g_roi_stack_channel_event, stack_setting,
        stack_setting->len + sizeof(Roi_Event_Setting_t));
}
void handle_Cmd_Ims_Add_Capability(RCS_EventMsg_t *event) {

    //VOLTE_REQ_SIPTX_REG_ADD_CAPABILITY
    //VoLTE_Event_Reg_Capability_t
    //volte_sip_stack_reg_capability

    int enable, id, type, rat;
    void* ptr = event->data;
    RCS_getInt32(&ptr, &enable);
    RCS_getInt32(&ptr, &id);
    RCS_getInt32(&ptr, &type);
    char rcs_data[VOLTE_MAX_REG_CAPABILITY_LENGTH] = {0};
    RCS_getStrn(&ptr, (char*)(&rcs_data), VOLTE_MAX_REG_CAPABILITY_LENGTH);
    RCS_getInt32(&ptr, &rat);

    DbgMsg("reg cap en:%d, id:%d, type:%d, rat:%d,", enable, id, type, rat);
    DbgMsg("reg_cap[%s]", rcs_data);
    roi_volte_sip_stack_reg_capability(g_roi_stack_channel_event, enable, rcs_data);
}
void handle_Cmd_Ims_Notify_Rat_Change(RCS_EventMsg_t *event) {

    //VOLTE_REQ_SIPTX_REG_RAT_CHANGE
    //VoLTE_Event_Rat_Change_t
    //volte_sip_stack_send_msg

    VoLTE_Stack_Message_t new_msg;
    Roi_Event_Rat_Change_t* rcs_data = (Roi_Event_Rat_Change_t*)event->data;
    Roi_Event_LBS_Location_t* loc_info = &rcs_data->loc_info;

#if 0
    DbgMsg("id:%d, type:%d, ci[%s], ssid[%s]", rcs_data->id, rcs_data->type, rcs_data->cell_id,
        rcs_data->ssid);
    DbgMsg("loc_info lat:[%g], lon:[%g], acc[%d], time[%s]", *(double *)&loc_info->lat,
        *(double *)&loc_info->lng,
        loc_info->acc, loc_info->timestamp);
#endif

    VoLTE_Stack_Message_Set(&new_msg, VOLTE_REQ_SIPTX_REG_RAT_CHANGE, sizeof(Roi_Event_Rat_Change_t), 0, rcs_data);
    roi_volte_sip_stack_send_msg(g_roi_stack_channel_event, &new_msg);
}
void handle_Cmd_Ims_Notify_3gpp_Rat_Change(RCS_EventMsg_t *event) {

    //VOLTE_REQ_SIPTX_REG_3GPP_RAT_CHANGE
    //VoLTE_Event_3gpp_Rat_Change_t
    //volte_sip_stack_update_setting

    VoLTE_Stack_Message_t new_msg;
    Roi_Event_3gpp_Rat_Change_t* rcs_data = (Roi_Event_3gpp_Rat_Change_t*)event->data;
    Roi_Event_LBS_Location_t* loc_info = &rcs_data->loc_info;

#if 0
    DbgMsg("id:%d, old_type:%d, old_ci[%s], new_type:%d, new_ci[%s]",
        rcs_data->id, rcs_data->old_rat_type, rcs_data->old_cell_id,
        rcs_data->new_rat_type, rcs_data->new_cell_id);
    DbgMsg("loc_info lat:[%g], lon:[%g], acc[%d], time[%s]", *(double *)&loc_info->lat,
        *(double *)&loc_info->lng,
        loc_info->acc, loc_info->timestamp);
#endif

    VoLTE_Stack_Message_Set(&new_msg, VOLTE_REQ_SIPTX_REG_3GPP_RAT_CHANGE, sizeof(Roi_Event_3gpp_Rat_Change_t), 0, rcs_data);
    roi_volte_sip_stack_send_msg(g_roi_stack_channel_event, &new_msg);
}
void handle_Cmd_Ims_Notify_Network_Change(RCS_EventMsg_t *event) {

    //VOLTE_REQ_SIPTX_REG_NETWORK_CHANGE
    //VoLTE_Event_Network_Change_t
    //volte_sip_stack_send_msg

    VoLTE_Stack_Message_t new_msg;
    Roi_Event_Network_Change_t* rcs_data = (Roi_Event_Network_Change_t*)event->data;
    Roi_Event_LBS_Location_t* loc_info = &rcs_data->loc_info;

#if 0
    DbgMsg("id:%d, state:%d, type:%d, ci[%s]", rcs_data->id, rcs_data->state, rcs_data->type,
        rcs_data->cell_id);
    DbgMsg("loc_info lat:[%g], lon:[%g], acc:%d, time[%s]", *(double *)&loc_info->lat,
        *(double *)&loc_info->lng,
        loc_info->acc, loc_info->timestamp);
#endif

    VoLTE_Stack_Message_Set(&new_msg, VOLTE_REQ_SIPTX_REG_NETWORK_CHANGE, sizeof(Roi_Event_Network_Change_t), 0, rcs_data);
    roi_volte_sip_stack_send_msg(g_roi_stack_channel_event, &new_msg);
}
void handle_Cmd_Ims_Auth_Reg_Result(RCS_EventMsg_t *event) {

    //VOLTE_CNF_SIPTX_REG_AUTH_REQ
    //VoLTE_Event_Reg_Auth_Resp_t
    //volte_sip_stack_send_msg

    VoLTE_Stack_Message_t new_msg;
    Roi_Event_Reg_Auth_Resp_t* rcs_data = (Roi_Event_Reg_Auth_Resp_t*)event->data;

#if 0
    DbgMsg("id:%d, result:%d, nc[%s], rsp[%s]", rcs_data->id, rcs_data->result, rcs_data->nc,
        rcs_data->response);
    DbgMsg("auts:[%s], ck_esp:[%s], ik_esp[%s], usr_data:%d", rcs_data->auts, rcs_data->ck_esp,
        rcs_data->ik_esp, rcs_data->user_data);
#endif

    VoLTE_Stack_Message_Set(&new_msg, VOLTE_CNF_SIPTX_REG_AUTH_REQ, sizeof(Roi_Event_Reg_Auth_Resp_t), 0, rcs_data);
    roi_volte_sip_stack_send_msg(g_roi_stack_channel_event, &new_msg);
}
void handle_Cmd_Ims_Geolocation_Req_Result(RCS_EventMsg_t *event) {

    //VOLTE_CNF_SIPTX_GEOLOCATION_REQ
    //VoLTE_Event_Geolocation_Info_t
    //volte_sip_stack_send_msg

    VoLTE_Stack_Message_t new_msg;
    Roi_Event_Geolocation_Info_t* rcs_data = (Roi_Event_Geolocation_Info_t*)event->data;
    Roi_Event_UA_Msg_Hdr_t* ua_msg_hdr = &rcs_data->ua_msg_hdr;

#if 0
    DbgMsg("ua_msg id:%d, hsk:%d", ua_msg_hdr->id, ua_msg_hdr->handshake);
    // TBD, unknown type of resv[3] and data[0]
    DbgMsg("resv:[%02X][%02X][%02X]", ua_msg_hdr->resv[0], ua_msg_hdr->resv[1], ua_msg_hdr->resv[2]);
    DbgMsg("type_id:%d, lat[%g], lon[%g], acc[%d]", rcs_data->type_id,
        *(double *)&rcs_data->latitude,
        *(double *)&rcs_data->longitude,
        rcs_data->accuracy);

    DbgMsg("method[%s], city[%s], state[%s], zip[%s], country_code[%s]", rcs_data->method,
        rcs_data->city, rcs_data->state,
        rcs_data->zip, rcs_data->country_code);
#endif

    VoLTE_Stack_Message_Set(&new_msg, VOLTE_CNF_SIPTX_GEOLOCATION_REQ, sizeof(Roi_Event_Geolocation_Info_t), 0, rcs_data);
    roi_volte_sip_stack_send_msg(g_roi_stack_channel_event, &new_msg);
}


void handle_Cmd_Ims_Reg_Digitline(RCS_EventMsg_t *event) {

    // VOLTE_REQ_SIPTX_REG_DIGITLINE
    // VoLTE_Event_ATCmd_Relay_t
    // volte_sip_stack_send_msg

    VoLTE_Stack_Message_t new_msg;
    Roi_Event_ATCmd_Relay_t* rcs_data = (Roi_Event_ATCmd_Relay_t*)event->data;

    VoLTE_Stack_Message_Set(&new_msg, VOLTE_REQ_SIPTX_REG_DIGITLINE, sizeof(Roi_Event_ATCmd_Relay_t), 0, rcs_data);
    roi_volte_sip_stack_send_msg(g_roi_stack_channel_event, &new_msg);
}

void handle_Cmd_Rcs_Activation() {

    DbgMsg("handle_Cmd_Rcs_Activation");

    int acct_id = 0;
    if (1 == gROISupported) {
        roi_volte_sip_stack_reg_bind(g_roi_stack_channel_event, acct_id);
    }

    volte_sip_stack_reg_bind(g_stack_channel_event, acct_id);
}

void handle_Cmd_Rcs_Deactivation() {

    DbgMsg("handle_Cmd_Rcs_Deactivation");
    if (1 == gROISupported) {
        roi_volte_sip_stack_reg_unbind(g_roi_stack_channel_event);
    }

    volte_sip_stack_reg_unbind(g_stack_channel_event);
}

void handle_Cmd_Sim_Operator(RCS_EventMsg_t *event) {

    void *nouse = event;
    gROISupported = isROISupported();
    DbgMsg("handle_Cmd_Sim_Operator, operator is %s, ROI support? %d", gSimOptr, gROISupported);
}

void handle_Cmd_Service_Activation_State(RCS_EventMsg_t *event) {
    int *raw_data = (int*)event->data;
    int state = *raw_data;
    DbgMsg("handle_Cmd_Service_Activation_State, state = %d", state);
    if (state) {
        property_set("persist.vendor.service.rcs", "1");
    } else {
        property_set("persist.vendor.service.rcs", "0");
    }
}

void handle_Cmd_Update_Service_Tags(RCS_EventMsg_t *event) {
    int features = *((int *)event->data);
    char property_value[92] = {0};
    int slot_id;
    char tags[16];

    sprintf(tags, "%d", features);

    property_get("persist.vendor.active.rcs.slot.id", property_value, "0");
    slot_id = atoi(property_value);

    DbgMsg("handle_Cmd_Update_Service_Tags, tags = %d, slot id = %d", features, slot_id);

    if (slot_id == 0) {
        property_set("persist.vendor.service.tag.rcs", tags);
    } else {
        property_set("persist.vendor.service.tag.rcs.2", tags);
    }
}

void handle_Cmd_EnableImsDeregSuspend(RCS_EventMsg_t *event) {
    int suspend = *((int *)event->data);

    //0: disable, 1: enable
    if (volte_sip_stack_ind_external_ua_state != NULL) {
        DbgMsg("volte_sip_stack_ind_external_ua_state:%d", suspend);
        int author = VoLTE_Stack_UA_RCS_Proxy | VoLTE_UA_MAGIC_NUMBER;
        volte_sip_stack_ind_external_ua_state(
                g_stack_channel_event, 0, author, suspend > 0 ? 1:0);
    }
}

void handle_ims_reg_state(VoLTE_Stack_Message_t *msg, int from) {

    char owner[20];
    RCS_EventMsg_t *event = 0;
    VoLTE_Event_Reg_State_t *reg_state = (VoLTE_Event_Reg_State_t*) msg->data;

    if (msg->len != sizeof(VoLTE_Event_Reg_State_t)) {
        ErrMsg("Data length error");
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
        DbgMsg("ignore VOLTE_IND_SIPTX_REG_STATE event since it's for emergency account %d\n", reg_state->id);
        goto free_mutex;
    }

    if (reg_state->cause == VoLTE_Event_Reg_Cause_Not_Bind) {
        DbgMsg("rebinding to REG UA \n");
        char property_value[92] = {0};
        int slot_id;
        int acct_id[2] = {0, 2};

        property_get("persist.vendor.active.rcs.slot.id", property_value, "0");
        slot_id = atoi(property_value);
        volte_sip_stack_reg_bind(g_stack_channel_event, acct_id[slot_id]);
    }

    if (from == OWNER_VOLTE_STACK) {
        strcpy(owner, "VOLTE");
    } else if (from == OWNER_ROI_STACK) {
        strcpy(owner, "  ROI");
    }

    DbgMsg("<<< Reg-Info[%d] from %s, state = %d", reg_state->id, owner, reg_state->state);

    if (gIgnoreEvent == 1 && from != g_ims_registered_owner) {
        DbgMsg("!!ignore reg_state event!!");
        goto free_mutex;
    }

    if (reg_state->state == VoLTE_Event_Reg_State_Registered) {
        g_ims_registered_owner = from;
        gIgnoreEvent = 1;
    } else if (reg_state->state == VoLTE_Event_Reg_State_Disconnected && from == g_ims_registered_owner) {
        g_ims_registered_owner = OWNER_INVALID;
        gIgnoreEvent = 0;

        // we may miss some reg events, re-query to get the latest status
        if (from == OWNER_VOLTE_STACK && gROISupported == 1) {
            // query 3g_volte_stack
            roi_volte_sip_stack_reg_state(g_roi_stack_channel_event);
        } else {
            // query volte_stack
            volte_sip_stack_reg_state(g_stack_channel_event);
        }
    }


#if 0
    DbgMsg("other info:"
        "\n cause = %d"
        "\n public_uid = %s"
        "\n local address  = %s"
        "\n local port = %d"
        "\n protocol_type = %d"
        "\n protocol_version = %d"
        "\n instance_id = %s"
        "\n private_uid = %s"
        "\n home_uri = %s,"
        "\n pcscf_address = %s"
        "\n pcscf_port =%d"
        "\n associated_uri = %s"
        "\n pub_gruu = %s"
        "\n temp_gruu = %s"
        "\n service_route = %s"
        "\n path = %s"
        "\n net_type = %d"
        "\n emergency_type = %d"
        "\n reg_timestamp = %s"
        "\n associated_uri = %s"
        "\n security-verify = %s"
        // "\n rcs_state = %s,"
        // "\n rcs_feature = %s,"
        "\n user-agent = %s ",
        reg_state->cause,
        reg_state->public_uid,
        reg_state->local_address,
        reg_state->local_port,
        reg_state->protocol_type,
        reg_state->protocol_version,
        reg_state->instance_id,
        reg_state->private_uid,
        reg_state->home_uri,
        reg_state->pcscf_address,
        reg_state->pcscf_port,
        reg_state->associated_uri,
        reg_state->pub_gruu,
        reg_state->temp_gruu,
        reg_state->service_route,
        reg_state->path,
        reg_state->net_type,
        reg_state->emergency_type,
        reg_state->reg_timestamp,
        reg_state->associated_uri,
        reg_state->security_verify,
        // reg_state->rcs_state,
        // reg_state->rcs_feature,
        reg_state->user_agent);
    DbgMsg("-----------------------------------------------------------------------------");
#endif

    //IF IMS CURRENT STATE IS REGISTERED
    if (reg_state->state == VoLTE_Event_Reg_State_Registered) {

        if (!g_stack_channel_sip) {
            create_SIP_Stack_connection(reg_state);
        }
        if (!g_roi_stack_channel_sip) {
            create_Roi_SIP_Stack_connection(reg_state);
        }
    } else if (reg_state->state == VoLTE_Event_Reg_State_Disconnected) {

        if (g_stack_channel_sip) {
            volte_sip_stack_destroy(g_stack_channel_sip);
            g_stack_channel_sip = 0;
        }

        if (g_roi_stack_channel_sip) {
            roi_volte_sip_stack_destroy(g_roi_stack_channel_sip);
            g_roi_stack_channel_sip = 0;
        }
    }

    if (g_rcs_register_request_sent) {

        DbgMsg("<<< RESPOSNE OF REGISTER REQUEST RECEIVED");
        //RESET THE VALUE
        g_rcs_register_request_sent = 0 ;

        char reg_update_state[2] = {'0','\0'};
        event = RCS_COMM_GetEvent(sizeof(reg_update_state));
        event->request_id = RSP_IMS_REGISTER;
        event->source = EventSource_RCS_PROXY_ADAPTATION;
        event->data_len = sizeof(reg_update_state);

        //IF RCS GOT REGISTERED
        if (reg_state->state == VoLTE_Event_Reg_State_Registered) {
            reg_update_state[0] = '1';
            //notify RCS
            DbgMsg("<<< RCS_UA ==> RCS_ADAPTER : RCS_PROXY_EVENT_RSP_RCS_REGISTERED [Success]");
            memcpy(event->data, &reg_update_state, sizeof(reg_update_state));
            RCS_COMM_Send(event);
            goto free_mutex;
        } else if ((reg_state->state == VoLTE_Event_Reg_State_Disconnected)
               ||(reg_state->state == VoLTE_Event_Reg_State_Unregistered)) {
            // state = 0
            reg_update_state[0] = '0';
            // notify RCS
            DbgMsg("<<< RCS_UA ==> RCS_ADAPTER : RCS_PROXY_EVENT_RSP_RCS_REGISTERED [failure]");
            memcpy(event->data, &reg_update_state, sizeof(reg_update_state));
            RCS_COMM_Send(event);
            goto free_mutex;
        }
    }

    /**
     * SEND RESPONSE TO THE UA
     */
    RCS_PROXY_Event_Reg_State2 *resp2 = 0;

    /* notify the event and put it in the main thread */
    event = RCS_COMM_GetEvent(sizeof(RCS_PROXY_Event_Reg_State2));
    event->source = EventSource_IMS_DISPATCHER;
    event->request_id = RSP_REQ_REG_INFO;
    event->data_len = sizeof(RCS_PROXY_Event_Reg_State2);

    // copy content
    resp2 = (RCS_PROXY_Event_Reg_State2*) malloc(sizeof(RCS_PROXY_Event_Reg_State2));
    memset(resp2, 0, sizeof(RCS_PROXY_Event_Reg_State2));

    resp2->id = from; // since account_id is meaningless for stack, reuse for "from" purpose
    resp2->state = reg_state->state;
    resp2->rcs_state = reg_state->rcs_state;
    resp2->rcs_feature = reg_state->rcs_feature;
    memcpy(resp2->local_address, reg_state->local_address, VOLTE_MAX_ADDRESS_LENGTH);
    resp2->local_port = reg_state->local_port;
    resp2->protocol_type = reg_state->protocol_type; // protocol type: TCP=1, UDP=2, TCPUDP=3
    resp2->protocol_version = reg_state->protocol_version; // protocol version : 0 -> IPV4  ; 1 -> IPv6
    memcpy(resp2->public_uid, reg_state->public_uid, VOLTE_MAX_REG_UID_LENGTH);
    memcpy(resp2->private_uid, reg_state->private_uid, VOLTE_MAX_REG_UID_LENGTH);
    memcpy(resp2->home_uri, reg_state->home_uri, VOLTE_MAX_DOMAIN_NAME_LENGTH);
    memcpy(resp2->pcscf_address, reg_state->pcscf_address, VOLTE_MAX_ADDRESS_LIST_LENGTH);
    resp2->pcscf_port = reg_state->pcscf_port;
    memcpy(resp2->user_agent, reg_state->user_agent, VOLTE_MAX_USER_AGENT_LENGTH);
    memcpy(resp2->associated_uri, reg_state->associated_uri, VOLTE_MAX_ASSOCIATED_URI);
    memcpy(resp2->instance_id, reg_state->instance_id, VOLTE_MAX_URI_LENGTH);
    memcpy(resp2->service_route, reg_state->service_route, VOLTE_MAX_URI_LENGTH);
    memcpy(resp2->pani, reg_state->pani, VOLTE_MAX_P_ACCESS_NETWORK_INFO_LENGTH);
    resp2->digit_number_of_vline = reg_state->digit_number_of_vline;
    memcpy(resp2->digit_pidentifier, reg_state->digit_pidentifier, VOLTE_MAX_DIGIT_PIDENTIFIER_LENGTH);
    memcpy(resp2->digit_ppa_header_string, reg_state->digit_ppa_header_string, VOLTE_MAX_DIGIT_PPA_LENGTH);
    memcpy(event->data, resp2, event->data_len);

    RCS_COMM_Recv(event);
    free(resp2);

free_mutex:
    pthread_mutex_unlock(&reg_mutex);
    DbgMsg("reg_mutex out");
}


void handle_ims_dereg_start(VoLTE_Stack_Message_t *msg, int from) {

    char owner[20];
    RCS_EventMsg_t *event = 0;
    VoLTE_Event_Reg_Dereg_Start_t *dereg_ind = (VoLTE_Event_Reg_Dereg_Start_t *)msg->data;

    if (msg->len != sizeof(VoLTE_Event_Reg_Dereg_Start_t)) {
        ErrMsg("Data length error");
    }

    /**
     * SEND RESPONSE TO THE UA
     */
    /* notify the event and put it in the main thread */
    event = RCS_COMM_GetEvent(sizeof(VoLTE_Event_Reg_Dereg_Start_t));
    event->source = EventSource_IMS_DISPATCHER;
    event->request_id = EVENT_IMS_DEREGISTER_IND;
    event->data_len = sizeof(VoLTE_Event_Reg_Dereg_Start_t);
    memcpy(event->data, msg->data, event->data_len);

    RCS_COMM_Recv(event);
}
