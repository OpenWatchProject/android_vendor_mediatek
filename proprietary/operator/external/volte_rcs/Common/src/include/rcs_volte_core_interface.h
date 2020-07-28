/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2017
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

/*******************************************************************************
*==============================================================================
*                 HISTORY
*------------------------------------------------------------------------------
* $Log: rcs_volte_core_interface.h,v $
* Modified from ims_interface_ap
*------------------------------------------------------------------------------
*==============================================================================
*******************************************************************************/
#ifndef _RCS_VOLTE_CORE_INTERFACE_AP_H
#define _RCS_VOLTE_CORE_INTERFACE_AP_H

#include "volte_core_interface.h"

// workaround
#define VOLTE_REQ_SIPTX_REG_DIGITLINE 91037
#define VOLTE_IND_ATCMD_RELAY 20008

// volte_def.h
#define VOLTE_MAX_CALL_ID_LENGTH                        (64)

// ims_interface_ap.h
#define MD_ATCMD_LEN (2048)
#define MD_ATCMD_PREFIX_LEN (20)
#define IMC_ATCMD_MAX_CMDLINE_LEN (MD_ATCMD_LEN - (MD_ATCMD_PREFIX_LEN))


/*****imc_general_sap.h START*****/
#define __IMCB_IMC_LOCAL_PARA_HDR_ON__ 0

#if defined(__IMCB_IMC_LOCAL_PARA_HDR_ON__)
#if __IMCB_IMC_LOCAL_PARA_HDR_ON__
#define IMCB_IMC_LOCAL_PARA_HDR LOCAL_PARA_HDR
#else
#define IMCB_IMC_LOCAL_PARA_HDR
#endif
#else
#error "please specified IMCB_IMC_LOCAL_PARA_HDR_OFF !"
#endif


typedef enum VoLTE_ATCmd_Id_e {
    VoLTE_ATCMD_NONE            = 0,
    VoLTE_ATCMD_EIMSXUI         = 1,  /* +EIMSXUI:<account_id>,<broadcast_flag>,<xui_info> */
    VoLTE_ATCMD_EIMSEVTPKG      = 2,  /* +EIMSEVTPKG: <call_id>,<type>,<urc_index>,<total_urc_count>,<data> */
    VoLTE_ATCMD_EIMSGEO         = 3,  /* AT+EIMSGEO=/+EIMSGEO:<account_id>,<broadcast_flag>,<latitude>,<longitude>,<accurate>,<method>,<city>,<state>,<zip>,<country> */
    VoLTE_ATCMD_EIMSPDIS        = 4,  /* +EIMSPDIS: <transaction_id>,<em_id>,<method>,<nw_if_name[]> ; AT+EIMSPDIS= <transaction_id>,<method>,<is_success> */
    VoLTE_ATCMD_EIMSPCSCF       = 5,  /* AT+EIMSPCSCF= <transaction_id>,<method>, <protocol_type>, <port_num>, <addr> */
    VoLTE_ATCMD_PRTTCALL        = 6,  /* +PRTTCALL: <call_id>, <op> ; AT+PRTTCALL: <call_id>, <result> */
    VoLTE_ATCMD_DIGITLINE       = 7,  /* AT+DIGITLINE=<account_id>,<serial_number>,<is_logout>,<has_next>,<is_native>,<msisdn>,"<token>"
                                      * +DIGITLINE=<account_id>,<serial_number>,<msisdn_num>,<msisdn>,<is_active> */
    VoLTE_ATCMD_MAX,
} VoLTE_ATCmd_Id_t;


#undef __IMCB_IMC_LOCAL_PARA_HDR_ON__
/*****imc_general_sap.h END*****/


// ims_interface_ap.h
typedef struct _Roi_Event_Notify {
    INT32                                   acct_id;
    INT32                                   data;
} Roi_Event_Notify_t;

// volte_event.h
#define VoLTE_Stack_Message_Set(m, t, l, f, d) {\
    (void)memset((m), 0, sizeof(VoLTE_Stack_Message_t));\
    (m)->type = (t); \
    (m)->len = (l); \
    (m)->flag = (f); \
    (m)->data = (d); \
    }

#define VOLTE_MAX_SSID_LENGTH 32

/* location info */
typedef struct _Roi_Event_LBS_Location_ {
    INT32               lat[2];       /* [0..8388607], 23 bits field */
    INT32               lng[2];       /* [-8388608..8388607], 24 bits field */
    INT32               acc;
    UINT8               timestamp[VOLTE_MAX_TIME_STAMP_LENGTH];   // VOLTE_MAX_TIME_STAMP_LENGTH =32
} Roi_Event_LBS_Location_t;

typedef struct _Roi_Event_Reg_Item_ {
    INT32                                   id;                                                 ///< the account id
    UINT8                                   cell_id[VOLTE_MAX_CELL_ID_LENGTH];                  ///< the current cell id from the modem PS
    UINT8                                   call_id[VOLTE_MAX_CALL_ID_LENGTH];                  ///< the call ua sip msg call-id
    Roi_Event_LBS_Location_t              loc_info;
    UINT8                                   next_pcscf;                                         ///< Try next pcscf for Register
} Roi_Event_Reg_Item_t;

typedef struct _Roi_Event_Setting_TLV_ {
    INT32                                   type;                                               ///< refer to ::VoLTE_Setting_Type_e
    INT32                                   len;                                                ///< the length of data
    INT32                                   value[0];                                           ///< data
} Roi_Event_Setting_TLV_t;

typedef struct _Roi_Event_Setting_ {
    INT32                                   id;                                                 ///< account id
    INT32                                   len;                                                ///< the length of data
    INT32                                   num;                                                ///< the number of settings
    Roi_Event_Setting_TLV_t               data[0];                                            ///< data
} Roi_Event_Setting_t;

typedef struct _Roi_Event_Reg_Capability {
    INT32                                   id;                                                 ///< the account id
    INT32                                   type;                                               ///< refer to ::Roi_Event_Capability_Type_e
    INT8                                    capability[VOLTE_MAX_REG_CAPABILITY_LENGTH];        ///< the data
    INT32                                   rat_type;                                           ///< RAT type, refer to ::Roi_Event_Network_Type_e
} Roi_Event_Reg_Capability_t;

typedef struct _Roi_Event_3gpp_Rat_Change_ {
    INT32                                   id;                                                 ///< the account id
    INT32                                   old_rat_type;                                       ///< refer to ::Roi_Event_Network_Type_e
    UINT8                                   old_cell_id[VOLTE_MAX_CELL_ID_LENGTH];              ///< the current cell id from the modem PS
    INT32                                   new_rat_type;                                       ///< refer to ::Roi_Event_Network_Type_e
    UINT8                                   new_cell_id[VOLTE_MAX_CELL_ID_LENGTH];              ///< the current cell id from the modem PS
    Roi_Event_LBS_Location_t              loc_info;
} Roi_Event_3gpp_Rat_Change_t;

typedef struct _Roi_Event_Rat_Change_ {
    INT32                                   id;                                                 ///< the account id
    INT32                                   type;                                               ///< refer to ::Roi_Event_Network_Type_e
    UINT8                                   cell_id[VOLTE_MAX_CELL_ID_LENGTH];                  ///< the current cell id from the modem PS
    Roi_Event_LBS_Location_t                loc_info;
    UINT8                                   ssid[VOLTE_MAX_SSID_LENGTH];
} Roi_Event_Rat_Change_t;

typedef struct _Roi_Event_Network_Change_ {
    INT32                                   id;                                                 ///< the account id
    INT32                                   state;                                              ///< refer to ::Roi_Event_Network_State_e
    INT32                                   type;                                               ///< refer to ::Roi_Event_Network_Type_e
    UINT8                                   cell_id[VOLTE_MAX_CELL_ID_LENGTH];                  ///< the current cell id from the modem PS
    Roi_Event_LBS_Location_t              loc_info;
} Roi_Event_Network_Change_t;

typedef Roi_Event_Reg_Item_t Roi_Event_Account_Item_t;

typedef struct _Roi_Event_Reg_Auth_Req {
    INT32                                   id;                                                 ///< the account id
    INT8                                    nonce[VOLTE_MAX_AUTH_NONCE];                        ///< the nonce
    INT32                                   encry_algo;                                         ///< refer to ::Roi_Event_IPSec_Encry_Algo_e
    INT32                                   integrity_algo;  /// < refer to ::Roi_Event_IPSec_Integrity_Algo_e
    UINT32                                  user_data;                                          ///< the user data for UA
} Roi_Event_Reg_Auth_Req_t;

typedef struct _Roi_Event_Reg_Auth_Resp {
    INT32                                   id;                                                 ///< the account id
    INT32                                   result;                                             ///< refer to ::Roi_Event_Auth_Result_e
    INT8                                    nc[VOLTE_MAX_AUTH_NC];                              ///< the nc
    INT8                                    response[VOLTE_MAX_AUTH_RESPONSE];                  ///< the response
    INT8                                    auts[VOLTE_MAX_AUTH_AUTS];                          ///< the auts
    INT8                                    ck_esp[VOLTE_MAX_AUTH_CK];                          ///< the ck_esp for ipsec encryption key
    INT8                                    ik_esp[VOLTE_MAX_AUTH_IK];                          ///< the ik_esp for ipsec intergrity key
    UINT32                                  user_data;                                          ///< the user data for UA
} Roi_Event_Reg_Auth_Resp_t;

typedef struct _Roi_Event_UA_Msg_Hdr {
    INT32   id;         /* account id */
    UINT8   handshake;  /* 0: one-way, 1: two-way */
    UINT8   resv[3];
    UINT8   data[0];

} Roi_Event_UA_Msg_Hdr_t ;

typedef struct _Roi_Event_Geolocation_Info {
    Roi_Event_UA_Msg_Hdr_t     ua_msg_hdr;
    UINT32                       type_id;    /* same as msg id */
    INT32                        latitude[2];
    INT32                        longitude[2];
    INT32                        accuracy;
    INT8                         method[16];
    INT8                         city[32];
    INT8                         state[32];
    INT8                         zip[8];
    INT8                         country_code[8];

} Roi_Event_Geolocation_Info_t;

typedef struct _Roi_imcb_imc_atcmd_relay_req {
    INT32     account_id;     /* Account ID, noraml account of the SIM*/
    INT32     atcmd_id;       /* VoLTE_ATCmd_Id_t, volte_event.h */
    INT32     pad;            /* pad */
    INT32     pad2;           /* pad */
    INT8      cmdline[IMC_ATCMD_MAX_CMDLINE_LEN];
} Roi_Event_ATCmd_Relay_t;


typedef struct _Roi_Event_Query_State {
    INT32                                   id;                                                 ///< account id
    INT32                                   type;                                               ///< refer to ::VoLTE_Query_Type_e
} Roi_Event_Query_State_t;

typedef struct _Roi_Event_Reg_Ems_Mode_ {
    INT32                                   id;                                                 ///< the account id
    INT32                                   ems_mode;                                           ///< refer to ::Roi_Event_Ems_Mode_e
    INT32                                   rat_type;                                           ///< refer to ::Roi_Event_Network_Type_e
    INT32                                   reg_service;                                        ///< refer to ::ims_reg_service_enum
} Roi_Event_Reg_Ems_Mode_t;

enum VoLTE_Stack_Reg_Op_e {
    REG_OP_REGISTER         = 1,
    REG_OP_REREGISTER       = 2,
    REG_OP_DEREGISTER       = 3
};

typedef struct _VoLTE_Event_Timer {
    UINT32                                  id;
    UINT32                                  timer_id;
    UINT32                                  timeout;
    UINT32                                  user_data;
} VoLTE_Event_Timer_t;

// Roi_Event_Timer_t
// Roi_Event_IPSec_Policy_Flush_Ind_t
// Roi_Event_IPSec_Policy_Add_Ind_t
#define roi_volte_sip_stack_create(i, c, cb)    _roi_volte_sip_stack_create((i),(c),(cb),__FILE__,__LINE__)

int (*roi_volte_sip_info_free)(VoLTE_Stack_Sip_Info_t*);
int (*roi_volte_sip_stack_init)();
void *(*_roi_volte_sip_stack_create)(VoLTE_Stack_Channel_Info_t*, VoLTE_Stack_Ua_Capability_t*, VoLTE_Stack_Callback_t*, char*, int);
int (*roi_volte_sip_stack_destroy)(void*);
int (*roi_volte_sip_stack_send_sip)(void *,VoLTE_Stack_Sip_Info_t *);
int (*roi_volte_sip_stack_send_msg)(void *,VoLTE_Stack_Message_t *);
int (*roi_volte_sip_stack_reg_state)(void *);
int (*roi_volte_sip_stack_reg_bind)(void *, int );
int (*roi_volte_sip_stack_reg_unbind)(void *);
int (*roi_volte_sip_stack_reg_capability)(void *, int , char *);
int (*ROI_Rule_Capability_Init)(VoLTE_Stack_Ua_Capability_t*);
int (*ROI_Rule_Capability_Deinit)(VoLTE_Stack_Ua_Capability_t*);
int (*ROI_Rule_Level0_Set)(VoLTE_Stack_Ua_Capability_t*, char*, char*);
int (*ROI_Rule_Level1_Set)(VoLTE_Stack_Ua_Capability_t*, char*);
int (*ROI_Rule_Level2_Set)(VoLTE_Stack_Ua_Capability_t*, const int , const int , char*);
int (*ROI_Rule_Level3_Set)(VoLTE_Stack_Ua_Capability_t*, char*);
int (*roi_sip_stack_reg_operation)(void *, int );
int (*roi_sip_stack_update_setting)(void *stack, void *setting, int setting_len);

// for VoLTE
int (*volte_sip_stack_reg_unbind)(void *);

#endif //_RCS_VOLTE_CORE_INTERFACE_AP_H
