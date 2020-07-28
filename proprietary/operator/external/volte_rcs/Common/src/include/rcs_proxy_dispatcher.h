#ifndef __RCS_PROXY_DISPATCHER_H__
#define __RCS_PROXY_DISPATCHER_H__

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "rcs_comm.h"

#define RCS_UNUSED(x)                  ((void)(x))
#define VoLTE_UA_MAGIC_NUMBER               (0x12140500)


#define OWNER_INVALID      -1
#define OWNER_VOLTE_STACK   0
#define OWNER_ROI_STACK     1


extern void *rcsVolteApiHandler;

/* handle events from the dispatcher */
void handle_Cmd_Req_Reg_Info(void);

/* initialize the connection with volte disapatcher and initlize the sip and event callbacl for RCS */
int RCS_Proxy_Init(void);

/* initialize the connection with volte disapatcher and initlize the sip and event callbacl for RCS */
/* this is for rcs service over Internet(RoI) feature*/
int RCS_Roi_Proxy_Init(void);

/* send SIP message to the Dispatcher */
int rcse_send_sip_msg(char *sip_message_string, int length);

void send_sip_message(int req_id, int conn_id, char *sip_message, int length);

int RCS_dispatcher_add_capability(char *capability_tag);

void RCS_dispatcher_delete_capability(char *capability_tag);

/* void create_SIP_Stack_connection(VoLTE_Event_Reg_State_t*  reg_state); */

#define VoLTE_Stack_Message_Set_rcs(m, t, l, f, d)  do { (m)->type = (t); (m)->len = (l); (m)->flag = (f); (m)->data = (d) ; } while (0)
enum VoLTE_Message_Attr_rcs_e {
    VoLTE_Message_Attr_Default_rcs          = 0x0002,
};

typedef struct _sipmsgrsp
{
    int req_id;
    int con_id;
    int data_len;
    char sip_data[0];
} sipmsgrsp;

void handle_ims_reg_state(VoLTE_Stack_Message_t *msg, int from);
void handle_ims_dereg_start(VoLTE_Stack_Message_t *msg, int from);

// RCS for 3g volte
void handle_Cmd_Ims_Notify_Vops_Info(RCS_EventMsg_t *event);
void handle_Cmd_Ims_Update_Settings(RCS_EventMsg_t *event);
void handle_Cmd_Ims_Update_Rt_Settings(RCS_EventMsg_t *event);
void handle_Cmd_Ims_Add_Capability(RCS_EventMsg_t *event);
void handle_Cmd_Ims_Notify_Rat_Change(RCS_EventMsg_t *event);
void handle_Cmd_Ims_Notify_3gpp_Rat_Change(RCS_EventMsg_t *event);
void handle_Cmd_Ims_Notify_Network_Change(RCS_EventMsg_t *event);
void handle_Cmd_Ims_Auth_Reg_Result(RCS_EventMsg_t *event);
void handle_Cmd_Ims_Geolocation_Req_Result(RCS_EventMsg_t *event);
void handle_Cmd_Ims_Unpublish_Completed(RCS_EventMsg_t *event);
void handle_Volte_Ind_Siptx_Reg_Auth_Req(VoLTE_Stack_Message_t *msg);
void handle_Volte_Ind_Siptx_Geolocation_Req(VoLTE_Stack_Message_t *msg);
void handle_Volte_Ind_Siptx_Reg_Query_State(VoLTE_Stack_Message_t *msg);
void handle_Volte_Ind_Siptx_Ems_Mode(VoLTE_Stack_Message_t *msg);
void handle_Cmd_Ims_Register(RCS_EventMsg_t *event);
void handle_Cmd_Ims_Deregister(RCS_EventMsg_t *event);
void handle_Volte_Ind_Siptx_Reg_Timer(VoLTE_Stack_Message_t *msg);
void handle_Volte_Ind_Atcmd_Relay(VoLTE_Stack_Message_t *msg);
void handle_Cmd_Ims_Reg_Digitline(RCS_EventMsg_t *event);
void handle_Cmd_Rcs_Activation();
void handle_Cmd_Rcs_Deactivation();
void handle_Cmd_Sim_Operator();
void handle_Cmd_Active_Rcs_Slot_Id(RCS_EventMsg_t *msg);
void handle_Cmd_Update_Service_Tags(RCS_EventMsg_t *event);
void handle_Cmd_Service_Activation_State(RCS_EventMsg_t *event);
void handle_Cmd_Ims_Restoration(RCS_EventMsg_t *msg);
void handle_Cmd_EnableImsDeregSuspend(RCS_EventMsg_t *msg);


#endif /* __RCS_PROXY_DISPATCHER_H__ */ /** __RCS_PROXY_DISPATCHER_H__  **/

