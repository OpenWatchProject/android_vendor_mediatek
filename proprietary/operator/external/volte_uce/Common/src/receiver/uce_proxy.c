#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "uce_comm.h"
#include "uce_proxy.h"
#include "uce_proxy_adapter.h"
#include "uce_proxy_dispatcher.h"
#include "debug.h"


#define LOGGER_ENTRY_MAX_LEN        (4*4096)


char* getUceEvent(int requestId) {

    switch (requestId) {
        case CMD_REQ_REG_INFO:          return "CMD_REQ_REG_INFO";
        case CMD_IMS_REGISTER:          return "CMD_IMS_REGISTER";
        case CMD_IMS_DEREGISTER:        return "CMD_IMS_DEREGISTER";
        case CMD_SEND_SIP_MSG:          return "CMD_SEND_SIP_MSG";
        case CMD_UNPUBLISH_COMPLETED:   return "CMD_UNPUBLISH_COMPLETED";
        case CMD_IMS_RECOVER_REGISTER_IND:   return "CMD_IMS_RECOVER_REGISTER_IND";
        case RSP_REQ_REG_INFO:          return "RSP_REQ_REG_INFO";
        case RSP_IMS_REGISTER:          return "RSP_IMS_REGISTER";
        case RSP_EVENT_SIP_MSG:         return "RSP_EVENT_SIP_MSG";
        case VOLTE_IND_SIPTX_REG_STATE: return "VOLTE_IND_SIPTX_REG_STATE";
        default:                        return "unknown_uce_event";
    }
}

/* hanld events from the dispatcher */
int handleEventFromDispatcher(UCE_EventMsg_t *event) {

    DbgMsg("send uce event to adapter:  %s", getUceEvent(event->request_id));

    switch (event->request_id) {

        case RSP_REQ_REG_INFO:
            UCE_COMM_Send(event);
            break;
        case RSP_IMS_REGISTER:
            break;
        case RSP_EVENT_SIP_MSG:
            UCE_COMM_Send(event);
            break;
        default:
            break;
    }

    return 0;

}

/* handle events from the uce adapter */
int handleEventFromAdapter(UCE_EventMsg_t *event) {

    DbgMsg("handle uce event from adapter: %s", (char *)getUceEvent(event->request_id));

    UCE_EventMsg_t *resp_event;
    char *uce_data = 0;

    switch (event->request_id) {

        case CMD_REQ_REG_INFO:
            // [TBD], send to Rcs Volte stack or Volte stack?
            UCE_dispatcher_Send_Req_Reg_Info();
            break;

        case CMD_SEND_SIP_MSG: {
            SipmMsgRsp* sipmsg;
            int request_id = 0;
            int conn_id = 0;
            int sip_message_len;

            sipmsg = (SipmMsgRsp*) event->data;
            request_id = sipmsg->req_id;
            conn_id = sipmsg->con_id;
            sip_message_len = sipmsg->data_len;

#if 0
            DbgMsg("req_id %d ", request_id) ;
            DbgMsg("conn_id %d ", conn_id) ;
            DbgMsg("sip message length %d ", sip_message_len) ;
#endif

            send_sip_message(request_id, conn_id, sipmsg->sip_data, sip_message_len);
            break;
        }
        case CMD_UNPUBLISH_COMPLETED:
            handle_ims_dereg_unpublish_done(event);
            break;
	case CMD_IMS_RECOVER_REGISTER_IND:
		handle_ims_recover_reg();
		break;
        default:
            break;
    }
    return 0;
}
