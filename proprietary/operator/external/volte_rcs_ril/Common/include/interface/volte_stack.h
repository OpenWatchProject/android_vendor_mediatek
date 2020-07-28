#ifndef __VOLTE_STACK_H__
#define __VOLTE_STACK_H__

#include "interface/volte_cap.h"
#include "interface/volte_event.h"

enum VoLTE_Stack_Protocol_Type_e {
    VoLTE_Stack_Protocol_Type_TCP = 1,
    VoLTE_Stack_Protocol_Type_UDP = 2,
    VoLTE_Stack_Protocol_Type_TCPUDP = 3
};


enum VoLTE_Stack_Protocol_Version_e {
    VoLTE_Stack_Protocol_Version_IPv4   = 1,
    VoLTE_Stack_Protocol_Version_IPv6   = 2
};

enum VoLTE_Stack_Result_e {
    VoLTE_Stack_Result_Disconnected     = -3,
    VoLTE_Stack_Result_Timeout          = -2,
    VoLTE_Stack_Result_Error            = -1,
    VoLTE_Stack_Result_Success          = 0
};

enum VoLTE_Stack_UA_Type_e {
    VoLTE_Stack_UA_Reg                  = 1,
    VoLTE_Stack_UA_Normal               = 2,
    VoLTE_Stack_UA_Call                 = 3,
    VoLTE_Stack_UA_SMS                  = 4,
    VoLTE_Stack_UA_Service              = 5,
    VoLTE_Stack_UA_RCS_Proxy            = 6,
    VoLTE_Stack_UA_USSD                 = 7,
    VoLTE_Stack_UA_Presence             = 8,
    VoLTE_Stack_UA_ClientAPI_Proxy      = 9,
    VoLTE_Stack_UA_RCS_Proxy_Sip        = 10,/* add for ims Submarine */
    VoLTE_Stack_UA_Presence_Sip         = 11,/* add for ims Submarine */

    VoLTE_Stack_UA_Count
};

typedef struct _VoLTE_Stack_Ua_Info {
    int                             local_port;
    char                            local_address[VOLTE_MAX_ADDRESS_LENGTH];
    int                             protocol_version;   /* VoLTE_Stack_Protocol_Version_e */

    unsigned int                    keep_alive;
    unsigned int                    keep_alive_mode;
    unsigned int                    tcp_connect_max_time_invite;
    unsigned int                    reuse_transport_methods;

    char                            tcp_on_demand;      /* 1 : on demand, 0 : always setup */
    char                            force_use_udp;      /* 1 : send the sip request by udp */
    char                            use_udp_on_tcp_fail;
    char                            backup_init_sa;

    unsigned int                    tcp_mtu_size;

    int                             ipsec;              /* 1 : enable ipsec, 0 : disable ipsec */
    unsigned int                    ipsec_port_start;
    unsigned int                    ipsec_port_end;
    unsigned int                    ipsec_port_curr;

    char                            support_sip_block;
    char                            reg_over_tcp;
    char                            connect_tcp_before_init_reg;
    char                            unused;

    char                            if_name[VOLTE_MAX_IF_NAME_LENGTH];
    int                             network_id;
    int                             sip_dscp;
    unsigned int                    sip_soc_priority;
    unsigned int                    sip_soc_tcp_mss;
    int                             sip_hdr_config;
    int                             acct_id;
} VoLTE_Stack_Ua_Info_t;

typedef struct _VoLTE_Stack_Channel_Info {
    int                             type;               /* VoLTE_Stack_UA_Type_e */
    unsigned int                    conn_info;
} VoLTE_Stack_Channel_Info_t;

typedef struct _VoLTE_Stack_Sip_Info {
    int                             conn_id;
    int                             request_id;
    int                             message_len;
    char                            message[0];
} VoLTE_Stack_Sip_Info_t;


typedef struct _VoLTE_Stack_Result {
    int                             result;             /* VoLTE_Stack_Result_e */
    unsigned int                    user_data[4];
} VoLTE_Stack_Result_t;

typedef struct _VoLTE_Stack_Message {
    unsigned int                    type;
    unsigned int                    len;
    unsigned int                    flag;
    void                           *data;
} VoLTE_Stack_Message_t;

typedef void (*VoLTE_Stack_Ua_Read_Sip_Fn) (VoLTE_Stack_Sip_Info_t *sip, void *user_data);

typedef void (*VoLTE_Stack_Ua_Read_Msg_Fn) (VoLTE_Stack_Message_t *msg, void *user_data);

typedef void (*VoLTE_Stack_Read_Msg_Fn) (VoLTE_Stack_Message_t *msg, void *user_data, void *channel);

typedef struct _VoLTE_Stack_Callback {
    VoLTE_Stack_Ua_Read_Sip_Fn      read_sip;
    VoLTE_Stack_Ua_Read_Msg_Fn      read_msg;
    void                           *user_data;
} VoLTE_Stack_Callback_t;

/* client api */
#define volte_sip_stack_create(i, c, cb)    _volte_sip_stack_create((i),(c),(cb),__FILE__,__LINE__)
void *_volte_sip_stack_create(VoLTE_Stack_Channel_Info_t *info, VoLTE_Stack_Ua_Capability_t *cap, VoLTE_Stack_Callback_t *callback, char *file, int line);

int volte_sip_stack_destroy(void *stack);

int volte_sip_stack_disconnect(void *stack);

int volte_sip_stack_send_sip(void *stack, VoLTE_Stack_Sip_Info_t *sip);

int volte_sip_stack_send_msg(void *stack, VoLTE_Stack_Message_t *msg);

/* client-reg api */
int volte_sip_stack_reg_bind(void *stack, int id);

int volte_sip_stack_reg_unbind(void *stack);

int volte_sip_stack_reg_capability(void *stack, int set, char *capability);

int volte_sip_stack_reg_state(void *stack);

int volte_sip_stack_restore(void *stack);

int volte_sip_stack_ind_external_ua_state(void *stack, int id, int ua_type, int ua_state);


/* sipmsg api */
int volte_sip_info_free(VoLTE_Stack_Sip_Info_t *sip);
#endif /* __VOLTE_STACK_H__ */
