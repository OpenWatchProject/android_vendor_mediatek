#ifndef __UCE_PROXY_DISPATCHER_H__
#define __UCE_PROXY_DISPATCHER_H__

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "uce_comm.h"

#define VoLTE_UA_MAGIC_NUMBER               (0x12140500)


#define OWNER_INVALID      -1
#define OWNER_VOLTE_STACK   0
#define OWNER_ROI_STACK     1


/* handle events from the dispatcher */
void UCE_dispatcher_Send_Req_Reg_Info(void);

/* initialize the connection with volte disapatcher and initlize the sip and event callbacl for UCE */
int UCE_Proxy_Init(void);

/* initialize the connection with volte disapatcher and initlize the sip and event callbacl for UCE */
/* this is for rcs service over Internet(RoI) feature */
int UCE_Roi_Proxy_Init(void);


/* send SIP message to the Dispatcher */
int uce_send_sip_msg(char *sip_message_string, int length);
int send_sip_message(int req_id, int conn_id, char *sip_message, int length);
void handle_ims_reg_state(VoLTE_Stack_Message_t *msg, int owner);
void handle_ims_dereg_start(VoLTE_Stack_Message_t *msg, void *user_data);
void handle_ims_dereg_unpublish_done(UCE_EventMsg_t *event);
void handle_ims_recover_reg();


/* void create_SIP_Stack_connection(VoLTE_Event_Reg_State_t*  reg_state); */


typedef struct  _sipmsgrsp {
int req_id;
int con_id;
int data_len;
char sip_data[0];
} SipmMsgRsp;


#endif /* __UCE_PROXY_DISPATCHER_H__ */

