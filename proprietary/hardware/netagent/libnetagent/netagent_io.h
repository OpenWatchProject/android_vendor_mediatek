#ifndef __LIB_NETAGENT_IO_H__
#define __LIB_NETAGENT_IO_H__

#define MAX_MOD_NAME_LENGTH 16
#define MAX_FQDN_LENGTH 256

typedef enum {
    NETAGENT_IO_RET_SUCCESS     = 0,
    NETAGENT_IO_RET_GENERIC_FAILURE = 1,
    NETAGENT_IO_RET_AT_FAIL     = 2,
} netagent_io_ret_e;

typedef enum {
    NETAGENT_IO_CMD_IFST        = 1,
    NETAGENT_IO_CMD_RA          = 2,
    NETAGENT_IO_CMD_IPUPDATE    = 3,

    NETAGENT_IO_CMD_IFUP        = 101,
    NETAGENT_IO_CMD_IFDOWN      = 102,
    NETAGENT_IO_CMD_IFCHG       = 103,
    NETAGENT_IO_CMD_IFSTATE     = 104,
    NETAGENT_IO_CMD_SETMTU      = 105,
    NETAGENT_IO_CMD_IPADD       = 106,
    NETAGENT_IO_CMD_IPDEL       = 107,

    NETAGENT_IO_CMD_SYNC_CAPABILITY = 201,
    NETAGENT_IO_CMD_PDNHO           = 202,
    NETAGENT_IO_CMD_IPCHG           = 203,
    NETAGENT_IO_CMD_NAPTR_QUERY     = 204,
    NETAGENT_IO_CMD_NAPTR_SEND      = 205,
    NETAGENT_IO_CMD_NAPTR_CAPABILITY = 206,
    NETAGENT_IO_CMD_TCP_RSVN        = 207,
    NETAGENT_IO_CMD_UDP_RSVN        = 208,
    NETAGENT_IO_CMD_SPI_RSVN        = 209,

    NETAGENT_IO_CMD_ARP_QUERY       = 301,
    NETAGENT_IO_CMD_ARP_RESULT      = 302,
    NETAGENT_IO_CMD_MAX             = 0xFFFFFFFF
} netagent_io_cmd_e;

typedef enum {
    NETAGENT_IO_IFST_DOWN       = 0,
    NETAGENT_IO_IFST_UP         = 1,
    NETAGENT_IO_IFST_MAX        = 0xFFFFFFFF
} netagent_io_ifst_e;

typedef enum {
    NETAGENT_IO_MTU             = 0,
    NETAGENT_IO_MTU_MAX         = 0xFFFFFFFF
} netagent_io_mtu_e;

typedef enum {
    NETAGENT_IO_ADDR_TYPE_UNKNOWN = 0,
    NETAGENT_IO_ADDR_TYPE_IPv4  = 0x1,
    NETAGENT_IO_ADDR_TYPE_IPv6  = 0x2,
    NETAGENT_IO_ADDR_TYPE_IPv4v6= 0x3,
    NETAGENT_IO_ADDR_TYPE_Any   = 0x4,
    NETAGENT_IO_ADDR_TYPE_MAX   = 0xFFFFFFFF
} netagent_io_addr_type_e;

typedef enum {
    NETAGENT_IO_NO_RA_REFRESH       = 0,
    NETAGENT_IO_NO_RA_INITIAL       = 1,
    NETAGENT_IO_RA_MAX              = 0xFFFFFFFF
} netagent_io_ra_e;

// Handover definition @{
// Sync capability to modem.
// 4bytes
// 0 bit: clear ipsec.
// 1 bit: interface ip changed after handover.
// 2 ~ 31 bit: not define.
#define NETAGENT_CAPABILITY_CLEAR_IPSEC 1
#define NETAGENT_CAPABILITY_HO_IP_CHANGED 2

typedef enum {
    NETAGENT_IO_HO_RAN_UNKNOWN      = 0,
    NETAGENT_IO_HO_RAN_MOBILE       = 1,
    NETAGENT_IO_HO_RAN_WIFI         = 2,
    NETAGENT_IO_HO_RAN_MAX          = 0xFFFFFFFF
} netagent_io_ho_ran_e;

typedef enum {
    NETAGENT_IO_HO_STATE_START      = 0,
    NETAGENT_IO_HO_STATE_STOP       = 1,
    NETAGENT_IO_HO_STATE_MAX        = 0xFFFFFFFF
} netagent_io_ho_state_e;

typedef enum {
    NETAGENT_IO_HO_RESULT_FAIL      = 0,
    NETAGENT_IO_HO_RESULT_SUCCESS   = 1,
    NETAGENT_IO_HO_RESULT_MAX       = 0xFFFFFFFF
} netagent_io_ho_result_e;

typedef struct {
    int                             cid;
    netagent_io_ho_state_e          hostate;
    netagent_io_ho_result_e         is_succ;
    netagent_io_ho_ran_e            src_ran;
    netagent_io_ho_ran_e            tgt_ran;
    netagent_io_addr_type_e         addr_type;
} netagent_io_ho_info_t;

typedef struct {
    unsigned int trans_id;
    char mod_id[MAX_MOD_NAME_LENGTH];
    char fqdn[MAX_FQDN_LENGTH];
} netagent_io_naptr_info_t; //for urc

typedef struct {
    int                     fail_code;
    char                    ifname[64];
    char                    ip[64];
    char                    mac[64];
} netagent_io_arp_parameter_t;

struct result_naptr_in_netagent {
  unsigned int trans_id;
  char mod_id[MAX_MOD_NAME_LENGTH];
  int result;
  unsigned int order;
  unsigned int pref;
  char * flags;
  char * service;
  char * regexp;
  char fqdn[MAX_FQDN_LENGTH];
}; //for struct in netagent
// @}

void *netagent_io_init();
int netagent_io_deinit(void *o);

void *netagent_io_cmd_arp_result_alloc( netagent_io_arp_parameter_t  * arp) ;
int netagent_io_get_arp(void *co, netagent_io_arp_parameter_t *arp);


void *netagent_io_cmd_ifst_alloc(unsigned int if_id, netagent_io_ifst_e state, netagent_io_addr_type_e addr_type);
void *netagent_io_cmd_ra_alloc(unsigned int if_id, netagent_io_ra_e flag);
void *netagent_io_cmd_ipupdate_alloc(unsigned int if_id, netagent_io_addr_type_e addr_type, unsigned int *addr, int ipv6PrefixLength);
void *netagent_io_cmd_sync_capability_alloc();
void *netagent_io_cmd_pdnho_alloc(unsigned int if_id);
void *netagent_io_cmd_ip_add_del_alloc(unsigned int if_id, netagent_io_cmd_e cmd, int result, netagent_io_addr_type_e addr_type, unsigned int *addr, int ipv6PrefixLength);
void *netagent_io_cmd_naptr_capability_alloc();
void *netagent_io_cmd_naptr_init_alloc(unsigned int trans_id, netagent_io_cmd_e cmd);
void *netagent_io_cmd_naptr_alloc(netagent_io_cmd_e cmd, struct result_naptr_in_netagent* result_list);
void *netagent_io_cmd_rsvn_alloc(unsigned int transaction_id, netagent_io_cmd_e cmd, unsigned int action, unsigned int response);
void *netagent_io_cmd_spi_alloc(unsigned int transaction_id, unsigned int action, unsigned int response);
int netagent_io_cmd_free(void *co);

int netagent_io_get_if_id(void *co, unsigned int *if_id);
int netagent_io_get_ip_change_reason(void *co, char **reason);
int netagent_io_get_cmd_type(void *co, netagent_io_cmd_e *cmd);
int netagent_io_get_addr_type(void *co, netagent_io_addr_type_e *addr_type);
int netagent_io_get_mtu_size(void *co, unsigned int *mtu_size);
int netagent_io_get_addr_v4(void *co, unsigned int *addr);
int netagent_io_get_addr_v6(void *co, unsigned int *addr);
int netagent_io_get_pdnho_info(void *co, netagent_io_ho_info_t *ho_info);
int netagent_io_get_naptr(void *co, netagent_io_naptr_info_t *naptr);
int netagent_io_get_rsvn_action(void *co, unsigned int *action);
int netagent_io_get_rsvn_if_id(void *co, int *interfaceId);
int netagent_io_get_rsvn_port(void *co, int *port);
int netagent_io_get_spi_protocol(void *co, unsigned int *protocol);
int netagent_io_get_spi_mode(void *co, unsigned int *mode);
int netagent_io_get_spi_min_max(void *co, unsigned int *min, unsigned int *max);
int netagent_io_get_spi_src_addr(void *co, unsigned int *addr);
int netagent_io_get_spi_dst_addr(void *co, unsigned int *addr);

void *netagent_io_recv(void *o) ;
int netagent_io_send(void *o, void *co) ;
int netagent_io_test(void *o, char *cmd);

#define SYSTEM_IP_WRAPPER_BIN_DIR  "/system/bin/ip-wrapper-1.0"
void dumpPdnHoInfo(netagent_io_ho_info_t *info);
int netagent_io_flush_ipsec_policy(const char *src_addr, netagent_io_addr_type_e addr_type);
int netagent_io_exec_fmt(int *status, const char *fmt, ...);
int netagent_io_fork_exec(const char *cmd, int *status);
int netagent_io_system_ext(char *argv[]);


/* ----

Example 1 :  for init

    void    *netagent_io_obj = 0;

    netagent_io_obj = netagent_io_init();
    if (!netagent_io_obj) {
        Err("init fail");
    } else {
        Dbg("init success");
    }

Example 2 : for deinit

    if (netagent_io_deinit(netagent_io_obj) == NETAGENT_IO_RET_SUCCESS) {
        Dbg("deinit success");
    } else {
        Err("deinit fail");
    }

Example 3 : Receive the URC

    void    *netagent_io_cmd_obj = 0;

    if (netagent_io_recv(netagent_io_obj, &netagent_io_cmd_obj) == NETAGENT_IO_RET_SUCCESS) {
        netagent_io_cmd_e cmd;
        netagent_io_get_addr_type type;
        unsigned int addr_v4;
        unsigned int addr_v6[4];

        if (netagent_io_get_cmd_type(netagent_io_cmd_obj, &cmd) == NETAGENT_IO_RET_SUCCESS) {
            switch (cmd) {
                case NETAGENT_IO_CMD_IFUP :
                    if (netagent_io_get_cmd_type(netagent_io_cmd_obj, &type) == NETAGENT_IO_RET_SUCCESS) {
                        switch (type) {
                            case NETAGENT_IO_ADDR_TYPE_IPv4 :
                                if (netagent_io_get_addr_v4(netagent_io_cmd_obj, &addr_v4) == NETAGENT_IO_RET_SUCCESS) {
                                    Dbg("get v4 address success");
                                } else {
                                    Err("get v4 address fail");
                                }
                                break;
                            case NETAGENT_IO_ADDR_TYPE_IPv6 :
                                if (netagent_io_get_addr_v6(netagent_io_cmd_obj, &addr_v6) == NETAGENT_IO_RET_SUCCESS) {
                                    Dbg("get v6 address success");
                                } else {
                                    Err("get v6 address fail");
                                }
                                break;
                            case NETAGENT_IO_ADDR_TYPE_IPv4v6;
                                if (netagent_io_get_addr_v4(netagent_io_cmd_obj, &addr_v4) == NETAGENT_IO_RET_SUCCESS) {
                                    Dbg("get v4 address success");
                                } else {
                                    Err("get v4 address fail");
                                }
                                if (netagent_io_get_addr_v6(netagent_io_cmd_obj, &addr_v6) == NETAGENT_IO_RET_SUCCESS) {
                                    Dbg("get v6 address success");
                                } else {
                                    Err("get v6 address fail");
                                }
                                break;
                        }

                    } else {
                        Err("get type fail");
                    }
                    break;
                case NETAGENT_IO_CMD_IFDOWN :
                case NETAGENT_IO_CMD_IFCHG :
            }
        } else {
            Err("get cmd fail");
        }

        netagent_io_cmd_free(netagent_io_cmd_obj);

    } else {
        Err("recv fail");
    }

Example 4 : send ifst (up with ipv4 address);

    void *netagent_io_cmd_obj = 0;
    unsigned int interface_id = 1;

    netagent_io_cmd_obj = netagent_io_cmd_ifst_alloc(interface_id, NETAGENT_IO_IFST_UP, NETAGENT_IO_ADDR_TYPE_IPv4);

    if (netagent_io_send(netagent_io_obj, &netagent_io_cmd_obj) == NETAGENT_IO_RET_SUCCESS) {
        Dbg("send cmd success");
    } else {
        Err("send cmd fail");
    }

    netagent_io_cmd_free(netagent_io_cmd_obj);

---- */


#endif /* __LIB_NETAGENT_IO_H__ */
