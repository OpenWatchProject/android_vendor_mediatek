#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "rcs_comm.h"
#include "rcs_proxy.h"
#include "rcs_proxy_adapter.h"
#include "rcs_proxy_dispatcher.h"
#include "rcs_proxy_session_handler.h"
#include "debug.h"


#define LOGGER_ENTRY_MAX_LEN        (4*4096)

/* hanld events from the dispatcher */


/*****************************************************************************
 * FUNCTION
 *  handleEventFromDispatcher
 * DESCRIPTION
 *
 * PARAMETERS
 *  event       [?]
 * RETURNS
 *
 *****************************************************************************/
int handleEventFromDispatcher(RCS_EventMsg_t *event) {

    DbgMsg("send uce event to adapter:  %s", idToString(event->request_id));

    switch (event->request_id) {
        case RSP_REQ_REG_INFO:
            RCS_COMM_Send(event);
            break;
        case RSP_IMS_REGISTER:
            break;

        case RSP_EVENT_SIP_MSG:
            RCS_COMM_Send(event);
            break;

        // RCS for 3g volte
        case EVENT_IMS_AUTH_REQ:
        case EVENT_IMS_GEOLOCATION_REQ:
        case EVENT_IMS_QUERY_STATE:
        case EVENT_IMS_EMS_MODE_INFO:
        case EVENT_IMS_DIGITLING_REG_IND:
            RCS_COMM_Send(event);
            break;
        case EVENT_IMS_DEREGISTER_IND:
            RCS_COMM_Send(event);
            break;

        default:
            break;

    }
    return 0;
}


/* handle events from the rcs adapter */
int handleEventFromAdapter(RCS_EventMsg_t *event) {

    DbgMsg("handle event from adapter: %s", (char *)idToString(event->request_id));

    RCS_EventMsg_t *resp_event;
    char *rcs_data = 0;

    switch (event->request_id) {
        case CMD_REQ_REG_INFO:
            /* rcs proxy needs current registration information */
            handle_Cmd_Req_Reg_Info();
            break;
        case CMD_IMS_REGISTER:
            /* register rcse to the volte */
            DbgMsg("CMD_IMS_REGISTER received.");

            /* get the capability string from event */
            int status = -1;
            rcs_data = (void*)event->data;

            DbgMsg("CMD_IMS_REGISTER data = [%s] ", rcs_data);
            //RCS_dispatcher_add_capability(rcs_data); */

            //send request to volte dispatcher
            //if (rcs_data) {
            //    status = RCS_dispatcher_add_capability(rcs_data);
            //}
            char state[2] = {'0', '\0'};
            // get the resp_event
            resp_event = RCS_COMM_GetEvent(sizeof(state));
            resp_event->request_id = RSP_IMS_REGISTERING;
            resp_event->source = EventSource_RCS_PROXY_ADAPTATION;
            resp_event->data_len = sizeof(state);

            handle_Cmd_Ims_Register(resp_event);

            if (status == VoLTE_Stack_Result_Success) {
                // notify RCS
                DbgMsg("RCS_UA ==> RCS_ADAPTER : RSP_IMS_REGISTERING [success]");
                // send data to RCS_proxy_adapater

                // success
                state[0] = '1';
                memcpy(resp_event->data, &state, sizeof(state));
                RCS_COMM_Send(resp_event);

            } else {
                //notify RCS
                DbgMsg("RCS_UA ==> RCS_ADAPTER : RSP_IMS_REGISTERING [failure]");

                //failure
                state[0] = '0';
                memcpy(resp_event->data, &state, sizeof(state));
                //send data to RCS_proxy_adapater
                RCS_COMM_Send(resp_event);
            }
            break;

        case CMD_IMS_DEREGISTER:
            /* de-register rcse to the rcs_volte_stack */
            DbgMsg("CMD_IMS_DEREGISTER received.");
            handle_Cmd_Ims_Deregister(event);
            break;
        case CMD_SEND_SIP_MSG:
            DbgMsg("CMD_SEND_SIP_MSG received. send SIP message");
            sipmsgrsp *sipmsgStr;
            int request_id = 0;
            int conn_id = 0;
            int sip_message_len ;

            sipmsgStr = (sipmsgrsp*) event->data;
            request_id = sipmsgStr->req_id;
            conn_id = sipmsgStr->con_id;
            sip_message_len = sipmsgStr->data_len;

            send_sip_message(request_id, conn_id, sipmsgStr->sip_data, sip_message_len);
            break;
        // RCS for 3g volte
        case CMD_IMS_NOTIFY_VOPS_INFO:
            handle_Cmd_Ims_Notify_Vops_Info(event);
            break;
        case CMD_IMS_UPDATE_SETTING:
            handle_Cmd_Ims_Update_Settings(event);
            break;
        case CMD_IMS_UPDATE_RT_SETTING:
            handle_Cmd_Ims_Update_Rt_Settings(event);
            break;
        case CMD_IMS_ADD_CAPABILITY:
            handle_Cmd_Ims_Add_Capability(event);
            break;
        case CMD_IMS_NOTIFY_RAT_CHANGE:
            handle_Cmd_Ims_Notify_Rat_Change(event);
            break;
        case CMD_IMS_NOTIFY_3GPP_RAT_CHANGE:
            handle_Cmd_Ims_Notify_3gpp_Rat_Change(event);
            break;
        case CMD_IMS_NOTIFY_NETWORK_CHANGE:
            handle_Cmd_Ims_Notify_Network_Change(event);
            break;
        case CMD_IMS_AUTH_REQ_RESULT:
            handle_Cmd_Ims_Auth_Reg_Result(event);
            break;
        case CMD_IMS_GEOLOCATION_REQ_RESULT:
            handle_Cmd_Ims_Geolocation_Req_Result(event);
            break;
        case CMD_RDS_NOTIFY_RCS_CONN_INIT:
            rcse_rds_conn_init();
            break;
        case CMD_RDS_NOTIFY_RCS_CONN_ACTIVE:
            rcse_rds_conn_active();
            break;
        case CMD_RDS_NOTIFY_RCS_CONN_INACTIVE:
            rcse_rds_conn_inactive();
            break;
        case CMD_IMS_REG_DIGITLINE:
            handle_Cmd_Ims_Reg_Digitline(event);
            break;
        case CMD_RCS_ACTIVATION:
            handle_Cmd_Rcs_Activation();
            break;
        case CMD_RCS_DEACTIVATION:
            handle_Cmd_Rcs_Deactivation();
            break;
        case CMD_SIM_OPERATOR:
            handle_Cmd_Sim_Operator(event);
            break;
        case CMD_SERVICE_ACTIVATION_STATE:
            handle_Cmd_Service_Activation_State(event);
            break;
        case CMD_UPDATE_SERVICE_TAGS:
            handle_Cmd_Update_Service_Tags(event);
            break;
        case CMD_UNPUBLISH_COMPLETED:
            handle_Cmd_Ims_Unpublish_Completed(event);
            break;
        case CMD_ACTIVE_RCS_SLOT_ID:
            handle_Cmd_Active_Rcs_Slot_Id(event);
            break;
        case CMD_IMS_RESTORATION:
            handle_Cmd_Ims_Restoration(event);
            break;
        case CMD_ENABLE_IMS_DEREG_SUSPEND:
            handle_Cmd_EnableImsDeregSuspend(event);
            break;
        default:
            break;
    }
    return 0;
}
