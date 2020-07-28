#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "atch.h"
#include "atapi.h"
#include "netagent_io.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <linux/in.h>
#include <linux/in6.h>
#include <linux/ipsec.h>
#include <linux/xfrm.h>

#include <mtk_log.h>
#include <sys/socket.h>


#ifdef __ANDROID__
    //#include <android/log.h>
    //#define DbgMsg(message, ...)       __android_log_print(ANDROID_LOG_DEBUG,   "NETAGENT", message " (%s:%d)",  ##__VA_ARGS__, __FILE__, __LINE__)
    //#define ErrMsg(message, ...)       __android_log_print(ANDROID_LOG_ERROR,   "NETAGENT", message " (%s:%d)",  ##__VA_ARGS__, __FILE__, __LINE__)
    //#define SysMsg(message, ...)       __android_log_print(ANDROID_LOG_INFO,    "NETAGENT", message " (%s:%d)",  ##__VA_ARGS__, __FILE__, __LINE__)
    //#define DbgMsg(f, a...)    printf("D " f " (%s:%d)\n", ## a, __FILE__, __LINE__)
    //#define ErrMsg(f, a...)    printf("E " f " (%s:%d)\n", ## a, __FILE__, __LINE__)

    #define NA_LOG_TAG "NetAgent_IO"
    #define DbgMsg(...) ((void)mtkLogD(NA_LOG_TAG, __VA_ARGS__))
    #define ErrMsg(...) ((void)mtkLogE(NA_LOG_TAG, __VA_ARGS__))

#else
    #define DbgMsg(message, ...)
    #define ErrMsg(message, ...)
    #define SysMsg(message, ...)
#endif

#define MAX_AT_CMD_BUFFER           (1024)
#define MAX_AT_LINE_BUFFER          (1024)
#define MAX_AT_RESPONSE_BUFFER      (4096)

#define TIMEOUT_REALTIME_CMD        (5)
#define TIMEOUT_SHORT_QUERY         (30)
#define TIMEOUT_LONG_QUERY          (300)
#define TIMEOUT_SHORT_EXECUTE       (30)
#define TIMEOUT_NORMAL_EXECUTE     (120)
#define TIMEOUT_LONG_EXECUTE        (300)

#define INVALID_IPV6_PREFIX_LENGTH -1

#define FREEIF(data)    \
if (data != NULL) {     \
    free(data);         \
    data = NULL;        \
}

typedef struct {
    unsigned int            addr_v4;
    unsigned int            addr_v6[4];
} netagent_io_addr_t;

typedef struct {
    unsigned int            state;      /* netagent_io_ifst_e */
    unsigned int            addr_type;  /* netagent_io_addr_type_e */
} netagent_io_ifst_parameter_t;

typedef struct {
    unsigned int            flag;       /* netagent_io_ra_e */
} netagent_io_ra_parameter_t;

typedef struct {
    int                     ipv6_prefix_length;
    unsigned int            addr_type;
    netagent_io_addr_t      addr;
} netagent_io_ip_parameter_t;

typedef struct {
    unsigned int            mtu_size;
} netagent_io_mtu_parameter_t;

typedef struct {
    netagent_io_ho_info_t   info;
    netagent_io_addr_t      addr;
} netagent_io_ho_parameter_t;

typedef struct {
    int                     ipv6_prefix_length;
    unsigned int            addr_type;
    netagent_io_addr_t      addr;
    int                     result;
} netagent_io_ip_add_del_parameter_t;

typedef struct {
    unsigned int trans_id;
    char mod_id[MAX_MOD_NAME_LENGTH];
    int result;
    unsigned int order;
    unsigned int pref;
    char *flags;
    char *service;
    char *regexp;
    char fqdn[MAX_FQDN_LENGTH];
} netagent_io_NAPTR_parameter_t; //for NAPTR AT command

typedef struct {
    int                     interfaceId;
    unsigned int            action;
    unsigned int            addr_type;
    netagent_io_addr_t      addr;
    int                     port;
    unsigned int            response;
} netagent_io_tcp_parameter_t;

typedef struct {
    unsigned int            action;
    unsigned int            addr_type;
    netagent_io_addr_t      addrSrc;
    netagent_io_addr_t      addrDst;
    unsigned int            protocol;
    unsigned int            mode;
    unsigned int            min;
    unsigned int            max;
    unsigned int            response;
} netagent_io_spi_parameter_t;

typedef struct {
    // this is transaction_id and interface_id combination,
    // i.e, trans_intf_id = (transaction_id * 100) + interface_id
    unsigned int    trans_intf_id;
    unsigned int    cmd;
    char*           reason;
    union {
        netagent_io_ifst_parameter_t        ifst;   // cmd = NETAGENT_IO_CMD_IFST
        netagent_io_mtu_parameter_t         mtu;    // cmd = NETAGENT_IO_CMD_SETMTU
        netagent_io_ra_parameter_t          ra;     // cmd = NETAGENT_IO_CMD_RA
        netagent_io_ip_parameter_t          ip;     // cmd = NETAGENT_IO_CMD_IPUPDATE / NETAGENT_IO_CMD_IFUP / NETAGENT_IO_CMD_IFCHG
        netagent_io_ho_parameter_t          ho;     // cmd = NETAGENT_IO_CMD_PDNHO
        netagent_io_ip_add_del_parameter_t  ipadd;  // cmd = NETAGENT_IO_CMD_IPADD
        netagent_io_ip_add_del_parameter_t  ipdel;  // cmd = NETAGENT_IO_CMD_IPDEL
        netagent_io_NAPTR_parameter_t       naptr;  // cmd = NETAGENT_IO_CMD_NAPTR
        netagent_io_arp_parameter_t         arp;    // cmd = NETAGENT_IO_CMD_ARP_QUERY
        netagent_io_tcp_parameter_t         tcp;    // cmd = NETAGENT_IO_CMD_TCP_RSVN
        netagent_io_spi_parameter_t         spi;    // cmd = NETAGENT_IO_CMD_SPI_RSVN
    } parameter;
} netagent_io_cmd_obj_t;


typedef struct {
    void        *cmd_ch;
    void        *urc_ch;
} netagent_io_t;

extern int Get_Mac_Addr(const char *ipStr, const char *ifname, struct sockaddr *WlanSocketAddr);
extern char *SocketAddr_ntop(const struct sockaddr *SockAddr, char *bufp);

void *
netagent_io_init() {
    netagent_io_t   *o = 0;
    char path_buf[128] = {0};

    o = (netagent_io_t *)malloc(sizeof(netagent_io_t));
    if (!o) {
        ErrMsg("Can't allocate memory");
        return 0;
    }
    memset(o, 0, sizeof(netagent_io_t));

    snprintf(path_buf, sizeof(path_buf), "/dev/radio/pttynwcmd");
    o->cmd_ch = atapi_init(ATCH_Type_RILD, path_buf);
    if (!(o->cmd_ch)) {
        ErrMsg("Can't open the channel (%s)", path_buf);
        goto init_fail;
    }

    snprintf(path_buf, sizeof(path_buf), "/dev/radio/pttynwurc");
    o->urc_ch = atapi_init(ATCH_Type_RILD, path_buf);
    if (!(o->urc_ch)) {
        ErrMsg("Can't open the channel (%s)", path_buf);
        goto init_fail;
    }

    atapi_urc_init(o->urc_ch);

    return o;

init_fail :
    if (o->cmd_ch) {
        atapi_deinit(o->cmd_ch);
        o->cmd_ch = 0;
    }

    if (o->urc_ch) {
        atapi_deinit(o->urc_ch);
        o->urc_ch = 0;
    }
    free(o);
    return 0;
}


int
netagent_io_test(void *o, char *cmd) {
    char *tmp_resp_buf = NULL;
    char *resp_buf = NULL;
    netagent_io_t *io = o;

    resp_buf = (char*)calloc(1, MAX_AT_RESPONSE_BUFFER * sizeof(char));

    DbgMsg("AT> %s", cmd);
    atapi_execute_command(io->cmd_ch, cmd, resp_buf, MAX_AT_RESPONSE_BUFFER, 0, 30);
    tmp_resp_buf = resp_buf;
    while (*tmp_resp_buf == '\r' || *tmp_resp_buf == '\n') {
        tmp_resp_buf++;
    }
    {
        int index = 0;
        index = strcspn(tmp_resp_buf, "\r\n");
        tmp_resp_buf[index] = 0;
    }
    DbgMsg("AT< %s", tmp_resp_buf);

    FREEIF(resp_buf);
    return 0;
}

int
netagent_io_deinit(void *o) {
    netagent_io_t *io = o;
    if (!o) {
        ErrMsg("parameter is NULL");
        return NETAGENT_IO_RET_GENERIC_FAILURE;
    }
    atapi_deinit(io->urc_ch);
    atapi_deinit(io->cmd_ch);
    free(o);
    return NETAGENT_IO_RET_SUCCESS;
}

static void *
netagent_io_cmd_alloc(unsigned int if_id, netagent_io_cmd_e cmd) {
    netagent_io_cmd_obj_t   *cmd_obj = 0;

    cmd_obj = (netagent_io_cmd_obj_t *)calloc(1, sizeof(netagent_io_cmd_obj_t));
    if (!cmd_obj) {
        ErrMsg("Can't allocate io obj");
        return 0;
    }
    cmd_obj->trans_intf_id = if_id;
    cmd_obj->cmd = cmd;

    return cmd_obj;
}

void *
netagent_io_cmd_ifst_alloc(unsigned int if_id, netagent_io_ifst_e state, netagent_io_addr_type_e addr_type) {
    netagent_io_cmd_obj_t   *cmd_obj = 0;

    cmd_obj = (netagent_io_cmd_obj_t *)calloc(1, sizeof(netagent_io_cmd_obj_t));
    if (!cmd_obj) {
        ErrMsg("Can't allocate io obj");
        return 0;
    }

    cmd_obj->trans_intf_id = if_id;
    cmd_obj->cmd = NETAGENT_IO_CMD_IFST;
    cmd_obj->parameter.ifst.state = state;
    cmd_obj->parameter.ifst.addr_type = addr_type;

    return cmd_obj;
}

void *
netagent_io_cmd_arp_alloc(char * ifname, char * ip) {
    netagent_io_cmd_obj_t   *cmd_obj = 0;

    cmd_obj = (netagent_io_cmd_obj_t *)calloc(1, sizeof(netagent_io_cmd_obj_t));
    if (!cmd_obj) {
        ErrMsg("Can't allocate io obj");
        return 0;
    }

    //cmd_obj->trans_intf_id = if_id;
    cmd_obj->cmd = NETAGENT_IO_CMD_ARP_QUERY;
    strncpy (cmd_obj->parameter.arp.ifname , ifname, 63);
    strncpy (cmd_obj->parameter.arp.ip , ip, 63);

    return cmd_obj;
}

void *

netagent_io_cmd_arp_result_alloc( netagent_io_arp_parameter_t  * arp) {
    netagent_io_cmd_obj_t   *cmd_obj = 0;
    struct sockaddr * wlanSocketAddr = 0;
    int fail_code = 0;
    cmd_obj = (netagent_io_cmd_obj_t *)calloc(1, sizeof(netagent_io_cmd_obj_t));
    if (!cmd_obj) {
        ErrMsg("Can't allocate io obj");
        return 0;
    }
    cmd_obj->cmd = NETAGENT_IO_CMD_ARP_RESULT;
    wlanSocketAddr =
            (struct sockaddr *) calloc(1, sizeof(struct sockaddr));
    if (!wlanSocketAddr) {
        ErrMsg("Can't allocate wlanSocketAddr");
        free(cmd_obj);
        return 0;
    }
    fail_code= Get_Mac_Addr((const char*)arp->ip,
            (const char*)arp->ifname, wlanSocketAddr);
    if (!fail_code) {
        SocketAddr_ntop((const struct sockaddr * )wlanSocketAddr, arp->mac);
        DbgMsg("get mac success, ip = %s, mac = %s", arp->ip, arp->mac);
    } else {
        ErrMsg("get mac fail, ip = %s, err = %s", arp->ip, strerror(fail_code));
    }
    arp->fail_code = fail_code;
    memcpy (&(cmd_obj->parameter.arp), arp, sizeof(netagent_io_arp_parameter_t));
    free (wlanSocketAddr);
    return cmd_obj;
}

void *
netagent_io_cmd_ra_alloc(unsigned int if_id, netagent_io_ra_e flag) {
    netagent_io_cmd_obj_t   *cmd_obj = 0;

    cmd_obj = (netagent_io_cmd_obj_t *)calloc(1, sizeof(netagent_io_cmd_obj_t));
    if (!cmd_obj) {
        ErrMsg("Can't allocate io obj");
        return 0;
    }

    cmd_obj->trans_intf_id = if_id;
    cmd_obj->cmd = NETAGENT_IO_CMD_RA;
    cmd_obj->parameter.ra.flag = flag;

    return cmd_obj;
}

static void *
netagent_io_cmd_ip_alloc(unsigned int if_id, netagent_io_cmd_e cmd, netagent_io_addr_type_e addr_type, unsigned int *addr, int ipv6PrefixLength) {
    netagent_io_cmd_obj_t   *cmd_obj = 0;

    cmd_obj = (netagent_io_cmd_obj_t *)calloc(1, sizeof(netagent_io_cmd_obj_t));
    if (!cmd_obj) {
        ErrMsg("Can't allocate io obj");
        return 0;
    }

    cmd_obj->trans_intf_id = if_id;
    cmd_obj->cmd = cmd;
    cmd_obj->parameter.ip.addr_type = addr_type;
    cmd_obj->parameter.ip.ipv6_prefix_length = ipv6PrefixLength;
    if (cmd_obj->parameter.ip.addr_type == NETAGENT_IO_ADDR_TYPE_IPv4) {
        memcpy(&(cmd_obj->parameter.ip.addr.addr_v4), addr, sizeof(cmd_obj->parameter.ip.addr.addr_v4));
    } else if (cmd_obj->parameter.ip.addr_type == NETAGENT_IO_ADDR_TYPE_IPv6) {
        memcpy(cmd_obj->parameter.ip.addr.addr_v6, addr, sizeof(cmd_obj->parameter.ip.addr.addr_v6));
    } else {
        ErrMsg("error addr_type");
        free(cmd_obj);
        return 0;
    }

    return cmd_obj;
}

void *
netagent_io_cmd_ipupdate_alloc(unsigned int if_id, netagent_io_addr_type_e addr_type, unsigned int *addr, int ipv6PrefixLength) {
    return netagent_io_cmd_ip_alloc(if_id, NETAGENT_IO_CMD_IPUPDATE, addr_type, addr, ipv6PrefixLength);
}

void *
netagent_io_cmd_sync_capability_alloc() {
    netagent_io_cmd_obj_t   *cmd_obj = 0;

    cmd_obj = (netagent_io_cmd_obj_t *)calloc(1, sizeof(netagent_io_cmd_obj_t));
    if (!cmd_obj) {
        ErrMsg("Can't allocate io obj");
        return 0;
    }
    cmd_obj->cmd = NETAGENT_IO_CMD_SYNC_CAPABILITY;

    return cmd_obj;
}

void *
netagent_io_cmd_naptr_init_alloc(unsigned int trans_id, netagent_io_cmd_e cmd) {
    netagent_io_cmd_obj_t   *cmd_obj = 0;

    cmd_obj = (netagent_io_cmd_obj_t *)calloc(1, sizeof(netagent_io_cmd_obj_t));
    if (!cmd_obj) {
        ErrMsg("Can't allocate io obj");
        return 0;
    }
    cmd_obj->parameter.naptr.trans_id = trans_id;
    cmd_obj->cmd = cmd;
    DbgMsg("cmd_obj->parameter.naptr.trans_id %d", cmd_obj->parameter.naptr.trans_id);


    return cmd_obj;
}

void *
netagent_io_cmd_naptr_capability_alloc() {
    netagent_io_cmd_obj_t   *cmd_obj = 0;

    cmd_obj = (netagent_io_cmd_obj_t *)calloc(1, sizeof(netagent_io_cmd_obj_t));
    if (!cmd_obj) {
        ErrMsg("Can't allocate io obj");
        return 0;
    }

    cmd_obj->cmd = NETAGENT_IO_CMD_NAPTR_CAPABILITY;

    return cmd_obj;
}

void *
netagent_io_cmd_naptr_alloc(netagent_io_cmd_e cmd, struct result_naptr_in_netagent* result_list) {

    netagent_io_cmd_obj_t   *cmd_obj = 0;

    cmd_obj = (netagent_io_cmd_obj_t *)calloc(1, sizeof(netagent_io_cmd_obj_t));
    if (!cmd_obj) {
        ErrMsg("Can't allocate io obj");
        return 0;
    }
    /*AT+ENAPTR=<trans_id>,<mod_id>,<result>,<order>,<pref>,<flags>,<service>,<regexp>,<replacement>*/
    cmd_obj->cmd = cmd;
    cmd_obj->parameter.naptr.trans_id = result_list->trans_id;
    memcpy(cmd_obj->parameter.naptr.mod_id, result_list->mod_id, sizeof(cmd_obj->parameter.naptr.mod_id));
    cmd_obj->parameter.naptr.result = result_list->result;
    cmd_obj->parameter.naptr.order = result_list->order;
    cmd_obj->parameter.naptr.pref = result_list->pref;
    cmd_obj->parameter.naptr.flags = result_list->flags;
    cmd_obj->parameter.naptr.service = result_list->service;
    cmd_obj->parameter.naptr.regexp = result_list->regexp;
    memcpy(cmd_obj->parameter.naptr.fqdn, result_list->fqdn, sizeof(cmd_obj->parameter.naptr.fqdn));

    return cmd_obj;
}

void *
netagent_io_cmd_pdnho_alloc(unsigned int if_id) {
    netagent_io_cmd_obj_t   *cmd_obj = 0;

    cmd_obj = (netagent_io_cmd_obj_t *)calloc(1, sizeof(netagent_io_cmd_obj_t));
    if (!cmd_obj) {
        ErrMsg("Can't allocate io obj");
        return 0;
    }
    cmd_obj->trans_intf_id = if_id;
    cmd_obj->cmd = NETAGENT_IO_CMD_PDNHO;

    return cmd_obj;
}

void *
netagent_io_cmd_ip_add_del_alloc(unsigned int if_id, netagent_io_cmd_e cmd, int result, netagent_io_addr_type_e addr_type, unsigned int *addr, int ipv6PrefixLength) {
    netagent_io_cmd_obj_t   *cmd_obj = 0;

    cmd_obj = (netagent_io_cmd_obj_t *)calloc(1, sizeof(netagent_io_cmd_obj_t));
    if (!cmd_obj) {
        ErrMsg("Can't allocate io obj");
        return 0;
    }

    cmd_obj->trans_intf_id = if_id;
    cmd_obj->cmd = cmd;
    cmd_obj->parameter.ip.ipv6_prefix_length = ipv6PrefixLength;
    if (addr_type == NETAGENT_IO_ADDR_TYPE_IPv6 ) {
        if (cmd_obj->cmd == NETAGENT_IO_CMD_IPADD) {
            cmd_obj->parameter.ipadd.result = result;
            cmd_obj->parameter.ipadd.addr_type = addr_type;
            memcpy(cmd_obj->parameter.ipadd.addr.addr_v6, addr, sizeof(cmd_obj->parameter.ipadd.addr.addr_v6));
        } else if (cmd_obj->cmd == NETAGENT_IO_CMD_IPDEL) {
            cmd_obj->parameter.ipdel.result = result;
            cmd_obj->parameter.ipdel.addr_type = addr_type;
            memcpy(cmd_obj->parameter.ipdel.addr.addr_v6, addr, sizeof(cmd_obj->parameter.ipdel.addr.addr_v6));
        } else {
            ErrMsg("error cmd");
            free(cmd_obj);
            return 0;
        }
    } else {
        ErrMsg("error addr_type");
        free(cmd_obj);
        return 0;
    }

    return cmd_obj;
}

void *
netagent_io_cmd_rsvn_alloc(unsigned int transaction_id, netagent_io_cmd_e cmd,
        unsigned int action, unsigned int response) {
    netagent_io_cmd_obj_t   *cmd_obj = 0;

    cmd_obj = (netagent_io_cmd_obj_t *)calloc(1, sizeof(netagent_io_cmd_obj_t));
    if (!cmd_obj) {
        ErrMsg("Can't allocate io obj");
        return 0;
    }

    cmd_obj->trans_intf_id = transaction_id;
    cmd_obj->cmd = cmd;
    cmd_obj->parameter.tcp.action = action;
    cmd_obj->parameter.tcp.response= response;

    return cmd_obj;
}

void *
netagent_io_cmd_spi_alloc(unsigned int transaction_id,
        unsigned int action, unsigned int response) {
    netagent_io_cmd_obj_t   *cmd_obj = 0;

    cmd_obj = (netagent_io_cmd_obj_t *)calloc(1, sizeof(netagent_io_cmd_obj_t));
    if (!cmd_obj) {
        ErrMsg("Can't allocate io obj");
        return 0;
    }

    cmd_obj->trans_intf_id = transaction_id;
    cmd_obj->cmd = NETAGENT_IO_CMD_SPI_RSVN;
    cmd_obj->parameter.spi.action = action;
    cmd_obj->parameter.spi.response= response;

    return cmd_obj;
}

int
netagent_io_get_if_id(void *co, unsigned int *if_id) {
    netagent_io_cmd_obj_t   *cmd_obj = co;

    if (!cmd_obj) {
        ErrMsg("obj is NULL");
        return NETAGENT_IO_RET_GENERIC_FAILURE;
    }

    *if_id = cmd_obj->trans_intf_id;
    return NETAGENT_IO_RET_SUCCESS;
}


int
netagent_io_get_ip_change_reason(void *co, char **reason) {
    netagent_io_cmd_obj_t   *cmd_obj = co;

    if (!cmd_obj) {
        ErrMsg("obj is NULL");
        return NETAGENT_IO_RET_GENERIC_FAILURE;
    }

    *reason = cmd_obj->reason;
    return NETAGENT_IO_RET_SUCCESS;
}

int
netagent_io_get_cmd_type(void *co, netagent_io_cmd_e *cmd) {
    netagent_io_cmd_obj_t   *cmd_obj = co;

    if (!cmd_obj) {
        ErrMsg("obj is NULL");
        return NETAGENT_IO_RET_GENERIC_FAILURE;
    }

    *cmd = cmd_obj->cmd;
    return NETAGENT_IO_RET_SUCCESS;
}

int
netagent_io_get_addr_type(void *co, netagent_io_addr_type_e *addr_type) {
    netagent_io_cmd_obj_t   *cmd_obj = co;

    if (!cmd_obj) {
        ErrMsg("obj is NULL");
        return NETAGENT_IO_RET_GENERIC_FAILURE;
    }

    switch (cmd_obj->cmd) {
        case NETAGENT_IO_CMD_IFUP :
        case NETAGENT_IO_CMD_IFCHG :
        case NETAGENT_IO_CMD_IPCHG:
            *addr_type = cmd_obj->parameter.ip.addr_type;
            return NETAGENT_IO_RET_SUCCESS;
        case NETAGENT_IO_CMD_IFDOWN :
            *addr_type = NETAGENT_IO_ADDR_TYPE_UNKNOWN;
            return NETAGENT_IO_RET_SUCCESS;
        case NETAGENT_IO_CMD_IPADD :
            *addr_type = cmd_obj->parameter.ipadd.addr_type;
            return NETAGENT_IO_RET_SUCCESS;
        case NETAGENT_IO_CMD_IPDEL :
            *addr_type = cmd_obj->parameter.ipdel.addr_type;
            return NETAGENT_IO_RET_SUCCESS;
        case NETAGENT_IO_CMD_TCP_RSVN:
        case NETAGENT_IO_CMD_UDP_RSVN :
            *addr_type = cmd_obj->parameter.tcp.addr_type;
            return NETAGENT_IO_RET_SUCCESS;
        case NETAGENT_IO_CMD_SPI_RSVN:
            *addr_type = cmd_obj->parameter.spi.addr_type;
            return NETAGENT_IO_RET_SUCCESS;
    }
    ErrMsg("invalid cmd = %d", cmd_obj->cmd);
    return NETAGENT_IO_RET_GENERIC_FAILURE;
}

int
netagent_io_get_mtu_size(void *co, unsigned int *mtu_size) {
    netagent_io_cmd_obj_t   *cmd_obj = co;

    if (!cmd_obj) {
        ErrMsg("obj is NULL");
        return NETAGENT_IO_RET_GENERIC_FAILURE;
    }

    if((cmd_obj->parameter.mtu.mtu_size)==0) {
        return NETAGENT_IO_RET_GENERIC_FAILURE;
    }

    else if(cmd_obj->parameter.mtu.mtu_size) {
        memcpy(mtu_size, &(cmd_obj->parameter.mtu.mtu_size), sizeof(cmd_obj->parameter.mtu.mtu_size));
        return NETAGENT_IO_RET_SUCCESS;
    }

    ErrMsg("invalid cmd = %d", cmd_obj->cmd);
    return NETAGENT_IO_RET_GENERIC_FAILURE;
}

int
netagent_io_get_naptr(void *co, netagent_io_naptr_info_t *naptr) {
    netagent_io_cmd_obj_t   *cmd_obj = co;
    if (!cmd_obj) {
        ErrMsg("obj is NULL");
        return NETAGENT_IO_RET_GENERIC_FAILURE;
    }

    DbgMsg("cmd_obj->parameter.naptr.trans_id : %d", cmd_obj->parameter.naptr.trans_id);
    DbgMsg("cmd_obj->parameter.naptr.mod_id : %s", cmd_obj->parameter.naptr.mod_id);
    DbgMsg("cmd_obj->parameter.naptr.fqdn : %s", cmd_obj->parameter.naptr.fqdn);

    if(cmd_obj) {
        naptr->trans_id = cmd_obj->parameter.naptr.trans_id;
        memcpy(naptr->mod_id, cmd_obj->parameter.naptr.mod_id, sizeof(cmd_obj->parameter.naptr.mod_id));
        memcpy(naptr->fqdn, cmd_obj->parameter.naptr.fqdn, sizeof(cmd_obj->parameter.naptr.fqdn));
        DbgMsg("naptr->trans_id: %d", naptr->trans_id);
        DbgMsg("naptr->mod_id : %s", naptr->mod_id);
        DbgMsg("naptr->fqdn : %s", naptr->fqdn);

        return NETAGENT_IO_RET_SUCCESS;
    }

    ErrMsg("invalid cmd = %d", cmd_obj->cmd);
    return NETAGENT_IO_RET_GENERIC_FAILURE;
}

int netagent_io_get_arp(void *co, netagent_io_arp_parameter_t *arp){
    netagent_io_cmd_obj_t   *cmd_obj = co;
    if (!cmd_obj) {
        ErrMsg("obj is NULL");
        return NETAGENT_IO_RET_GENERIC_FAILURE;
    }

    if(cmd_obj) {
        memcpy (arp, &(cmd_obj->parameter.arp), sizeof(netagent_io_arp_parameter_t));
        DbgMsg("arp->ifname: %s", arp->ifname);
        DbgMsg("arp->ip: %s", arp->ip);
        return NETAGENT_IO_RET_SUCCESS;
    }
    ErrMsg("invalid cmd = %d", cmd_obj->cmd);
    return NETAGENT_IO_RET_GENERIC_FAILURE;
}

int
netagent_io_get_addr_v4(void *co, unsigned int *addr) {
    netagent_io_cmd_obj_t   *cmd_obj = co;

    if (!cmd_obj) {
        ErrMsg("obj is NULL");
        return NETAGENT_IO_RET_GENERIC_FAILURE;
    }

    switch (cmd_obj->cmd) {
        case NETAGENT_IO_CMD_IFUP :
        case NETAGENT_IO_CMD_IFCHG :
        case NETAGENT_IO_CMD_IPCHG:
            if (cmd_obj->parameter.ip.addr_type & NETAGENT_IO_ADDR_TYPE_IPv4) {
                memcpy(addr, &(cmd_obj->parameter.ip.addr.addr_v4), sizeof(cmd_obj->parameter.ip.addr.addr_v4));
                return NETAGENT_IO_RET_SUCCESS;
            }
            ErrMsg("invalid addr_type = %d", cmd_obj->parameter.ip.addr_type);
            return NETAGENT_IO_RET_GENERIC_FAILURE;
        case NETAGENT_IO_CMD_PDNHO :
            if (cmd_obj->parameter.ho.info.addr_type & NETAGENT_IO_ADDR_TYPE_IPv4) {
                memcpy(addr, &(cmd_obj->parameter.ho.addr.addr_v4), sizeof(cmd_obj->parameter.ho.addr.addr_v4));
                return NETAGENT_IO_RET_SUCCESS;
            }
            ErrMsg("invalid addr_type = %d", cmd_obj->parameter.ip.addr_type);
            return NETAGENT_IO_RET_GENERIC_FAILURE;
        case NETAGENT_IO_CMD_TCP_RSVN:
        case NETAGENT_IO_CMD_UDP_RSVN :
            if (cmd_obj->parameter.tcp.addr_type & NETAGENT_IO_ADDR_TYPE_IPv4) {
                memcpy(addr, &(cmd_obj->parameter.tcp.addr.addr_v4), sizeof(cmd_obj->parameter.tcp.addr.addr_v4));
                return NETAGENT_IO_RET_SUCCESS;
            }
            ErrMsg("invalid addr_type = %d", cmd_obj->parameter.tcp.addr_type);
            return NETAGENT_IO_RET_GENERIC_FAILURE;
    }
    ErrMsg("invalid cmd = %d", cmd_obj->cmd);
    return NETAGENT_IO_RET_GENERIC_FAILURE;
}

int
netagent_io_get_addr_v6(void *co, unsigned int *addr) {
    netagent_io_cmd_obj_t   *cmd_obj = co;

    if (!cmd_obj) {
        ErrMsg("obj is NULL");
        return NETAGENT_IO_RET_GENERIC_FAILURE;
    }

    switch (cmd_obj->cmd) {
        case NETAGENT_IO_CMD_IFUP :
        case NETAGENT_IO_CMD_IFCHG :
        case NETAGENT_IO_CMD_IPCHG:
            if (cmd_obj->parameter.ip.addr_type & NETAGENT_IO_ADDR_TYPE_IPv6) {
                memcpy(addr, cmd_obj->parameter.ip.addr.addr_v6, sizeof(cmd_obj->parameter.ip.addr.addr_v6));
                return NETAGENT_IO_RET_SUCCESS;
            }
            ErrMsg("invalid addr_type = %d", cmd_obj->parameter.ip.addr_type);
            return NETAGENT_IO_RET_GENERIC_FAILURE;
        case NETAGENT_IO_CMD_PDNHO :
            if (cmd_obj->parameter.ho.info.addr_type & NETAGENT_IO_ADDR_TYPE_IPv6) {
                memcpy(addr, cmd_obj->parameter.ho.addr.addr_v6, sizeof(cmd_obj->parameter.ho.addr.addr_v6));
                return NETAGENT_IO_RET_SUCCESS;
            }
            ErrMsg("invalid addr_type = %d", cmd_obj->parameter.ip.addr_type);
            return NETAGENT_IO_RET_GENERIC_FAILURE;
        case NETAGENT_IO_CMD_IPADD :
            if (cmd_obj->parameter.ipadd.addr_type & NETAGENT_IO_ADDR_TYPE_IPv6) {
                memcpy(addr, cmd_obj->parameter.ipadd.addr.addr_v6, sizeof(cmd_obj->parameter.ipadd.addr.addr_v6));
                return NETAGENT_IO_RET_SUCCESS;
            }
            ErrMsg("invalid addr_type = %d", cmd_obj->parameter.ipadd.addr_type);
            return NETAGENT_IO_RET_GENERIC_FAILURE;
        case NETAGENT_IO_CMD_IPDEL :
            if (cmd_obj->parameter.ipdel.addr_type & NETAGENT_IO_ADDR_TYPE_IPv6) {
                memcpy(addr, cmd_obj->parameter.ipdel.addr.addr_v6, sizeof(cmd_obj->parameter.ipdel.addr.addr_v6));
                return NETAGENT_IO_RET_SUCCESS;
            }
            ErrMsg("invalid addr_type = %d", cmd_obj->parameter.ipdel.addr_type);
            return NETAGENT_IO_RET_GENERIC_FAILURE;
        case NETAGENT_IO_CMD_TCP_RSVN:
        case NETAGENT_IO_CMD_UDP_RSVN :
            if (cmd_obj->parameter.tcp.addr_type & NETAGENT_IO_ADDR_TYPE_IPv6) {
                memcpy(addr, cmd_obj->parameter.tcp.addr.addr_v6, sizeof(cmd_obj->parameter.tcp.addr.addr_v6));
                return NETAGENT_IO_RET_SUCCESS;
            }
            ErrMsg("invalid addr_type = %d", cmd_obj->parameter.tcp.addr_type);
            return NETAGENT_IO_RET_GENERIC_FAILURE;
    }
    ErrMsg("invalid cmd = %d", cmd_obj->cmd);
    return NETAGENT_IO_RET_GENERIC_FAILURE;
}

int
netagent_io_get_pdnho_info(void *co, netagent_io_ho_info_t *ho_info) {
    netagent_io_cmd_obj_t   *cmd_obj = co;

    if (!cmd_obj) {
        ErrMsg("obj is NULL");
        return NETAGENT_IO_RET_GENERIC_FAILURE;
    }

    if (cmd_obj->cmd != NETAGENT_IO_CMD_PDNHO) {
        ErrMsg("invalid cmd = %d", cmd_obj->cmd);
        return NETAGENT_IO_RET_GENERIC_FAILURE;
    }

    memcpy(ho_info, &(cmd_obj->parameter.ho.info), sizeof(netagent_io_ho_info_t));

    return NETAGENT_IO_RET_SUCCESS;
}

int
netagent_io_get_rsvn_action(void *co, unsigned int *action) {
    netagent_io_cmd_obj_t *cmd_obj = co;

    if (!cmd_obj) {
        ErrMsg("obj is NULL");
        return NETAGENT_IO_RET_GENERIC_FAILURE;
    }

    switch (cmd_obj->cmd) {
        case NETAGENT_IO_CMD_TCP_RSVN:
        case NETAGENT_IO_CMD_UDP_RSVN :
            *action = cmd_obj->parameter.tcp.action;
            return NETAGENT_IO_RET_SUCCESS;
        case NETAGENT_IO_CMD_SPI_RSVN :
            *action = cmd_obj->parameter.spi.action;
            return NETAGENT_IO_RET_SUCCESS;
    }

    ErrMsg("invalid cmd = %d", cmd_obj->cmd);
    return NETAGENT_IO_RET_GENERIC_FAILURE;
}

int
netagent_io_get_rsvn_if_id(void *co, int *interfaceId) {
    netagent_io_cmd_obj_t *cmd_obj = co;

    if (!cmd_obj) {
        ErrMsg("obj is NULL");
        return NETAGENT_IO_RET_GENERIC_FAILURE;
    }

    switch (cmd_obj->cmd) {
        case NETAGENT_IO_CMD_TCP_RSVN:
        case NETAGENT_IO_CMD_UDP_RSVN :
            *interfaceId = cmd_obj->parameter.tcp.interfaceId;
            return NETAGENT_IO_RET_SUCCESS;
    }

    ErrMsg("invalid cmd = %d", cmd_obj->cmd);
    return NETAGENT_IO_RET_GENERIC_FAILURE;
}

int
netagent_io_get_rsvn_port(void *co, int *port) {
    netagent_io_cmd_obj_t *cmd_obj = co;

    if (!cmd_obj) {
        ErrMsg("obj is NULL");
        return NETAGENT_IO_RET_GENERIC_FAILURE;
    }

    switch (cmd_obj->cmd) {
        case NETAGENT_IO_CMD_TCP_RSVN:
        case NETAGENT_IO_CMD_UDP_RSVN :
            *port = cmd_obj->parameter.tcp.port;
            return NETAGENT_IO_RET_SUCCESS;
    }

    ErrMsg("invalid cmd = %d", cmd_obj->cmd);
    return NETAGENT_IO_RET_GENERIC_FAILURE;
}

int
netagent_io_get_spi_protocol(void *co, unsigned int *protocol) {
    netagent_io_cmd_obj_t *cmd_obj = co;

    if (!cmd_obj) {
        ErrMsg("obj is NULL");
        return NETAGENT_IO_RET_GENERIC_FAILURE;
    }

    switch (cmd_obj->cmd) {
        case NETAGENT_IO_CMD_SPI_RSVN:
            *protocol = cmd_obj->parameter.spi.protocol;
            return NETAGENT_IO_RET_SUCCESS;
    }

    ErrMsg("invalid cmd = %d", cmd_obj->cmd);
    return NETAGENT_IO_RET_GENERIC_FAILURE;
}

int
netagent_io_get_spi_mode(void *co, unsigned int *mode) {
    netagent_io_cmd_obj_t *cmd_obj = co;

    if (!cmd_obj) {
        ErrMsg("obj is NULL");
        return NETAGENT_IO_RET_GENERIC_FAILURE;
    }

    switch (cmd_obj->cmd) {
        case NETAGENT_IO_CMD_SPI_RSVN:
            *mode = cmd_obj->parameter.spi.mode;
            return NETAGENT_IO_RET_SUCCESS;
    }

    ErrMsg("invalid cmd = %d", cmd_obj->cmd);
    return NETAGENT_IO_RET_GENERIC_FAILURE;
}

int
netagent_io_get_spi_min_max(void *co, unsigned int *min, unsigned int *max) {
    netagent_io_cmd_obj_t *cmd_obj = co;

    if (!cmd_obj) {
        ErrMsg("obj is NULL");
        return NETAGENT_IO_RET_GENERIC_FAILURE;
    }

    switch (cmd_obj->cmd) {
        case NETAGENT_IO_CMD_SPI_RSVN:
            *min = cmd_obj->parameter.spi.min;
            *max = cmd_obj->parameter.spi.max;
            return NETAGENT_IO_RET_SUCCESS;
    }

    ErrMsg("invalid cmd = %d", cmd_obj->cmd);
    return NETAGENT_IO_RET_GENERIC_FAILURE;
}

int
netagent_io_get_spi_src_addr(void *co, unsigned int *addr) {
    netagent_io_cmd_obj_t   *cmd_obj = co;

    if (!cmd_obj) {
        ErrMsg("obj is NULL");
        return NETAGENT_IO_RET_GENERIC_FAILURE;
    }

    switch (cmd_obj->cmd) {
        case NETAGENT_IO_CMD_SPI_RSVN :
            if (cmd_obj->parameter.spi.addr_type & NETAGENT_IO_ADDR_TYPE_IPv4) {
                memcpy(addr, &(cmd_obj->parameter.spi.addrSrc.addr_v4),
                        sizeof(cmd_obj->parameter.spi.addrSrc.addr_v4));
                return NETAGENT_IO_RET_SUCCESS;
            }
            memcpy(addr, cmd_obj->parameter.spi.addrSrc.addr_v6,
                    sizeof(cmd_obj->parameter.spi.addrSrc.addr_v6));
            return NETAGENT_IO_RET_SUCCESS;
    }
    ErrMsg("invalid cmd = %d", cmd_obj->cmd);
    return NETAGENT_IO_RET_GENERIC_FAILURE;
}

int
netagent_io_get_spi_dst_addr(void *co, unsigned int *addr) {
    netagent_io_cmd_obj_t   *cmd_obj = co;

    if (!cmd_obj) {
        ErrMsg("obj is NULL");
        return NETAGENT_IO_RET_GENERIC_FAILURE;
    }

    switch (cmd_obj->cmd) {
        case NETAGENT_IO_CMD_SPI_RSVN :
            if (cmd_obj->parameter.spi.addr_type & NETAGENT_IO_ADDR_TYPE_IPv4) {
                memcpy(addr, &(cmd_obj->parameter.spi.addrDst.addr_v4),
                        sizeof(cmd_obj->parameter.spi.addrDst.addr_v4));
                return NETAGENT_IO_RET_SUCCESS;
            }
            memcpy(addr, cmd_obj->parameter.spi.addrDst.addr_v6,
                    sizeof(cmd_obj->parameter.spi.addrDst.addr_v6));
            return NETAGENT_IO_RET_SUCCESS;
    }
    ErrMsg("invalid cmd = %d", cmd_obj->cmd);
    return NETAGENT_IO_RET_GENERIC_FAILURE;
}

int
netagent_io_cmd_free(void *co) {
    netagent_io_cmd_obj_t   *cmd_obj = co;

    if (!cmd_obj) {
        ErrMsg("obj is NULL");
        return NETAGENT_IO_RET_GENERIC_FAILURE;
    }
    FREEIF(cmd_obj->reason);
    free(cmd_obj);

    return NETAGENT_IO_RET_SUCCESS;
}


#define IGNORE_CHAR(s,c) {while ((*(s)) == (c) && *(s)) (s)++;}
#define FIND_CHAR(s,c) {while ((*(s)) != (c) && *(s)) (s)++;}


static int
_netagent_io_at_parser(char *line, int argc, char **argv) {
    char *str = line;
    int num = 0;

    while (num < argc && *str) {
        IGNORE_CHAR(str, ' ');
        argv[num++] = ((*str == '"') ? (str+1) : str);
        while (*str) {
            if (*str == '"') {
                str++;
                FIND_CHAR(str, '"');
                if (*str == '"') {
                    *str = 0;
                }
                str++;
                FIND_CHAR(str, ',');
                if (*str == ',') {
                    str++;
                }
                break;
            } else {
                FIND_CHAR(str, ',');
                if (*str == ',') {
                    char *ptr = str-1;

                    *str = 0;
                    str++;

                    while (*ptr == ' ') {
                        *ptr = 0;
                        ptr--;
                    }

                    break;
                }
            }
        }
    }

    return num;
}


static int
netagent_io_ipv4_str2bin(char *ip_str, unsigned int *ip_bin) {
    int i = 0;
    char *ptr = 0;
    unsigned char ip_buf[4] = {0};

    if (!ip_str || !ip_bin) {
        return -1;
    }

    ptr = ip_str;
    for (i=0 ; i<4 && ptr && *ptr ; ++i) {
        ip_buf[i] =  (unsigned char)strtoul(ptr, 0, 10);
        ptr = strchr(ptr, '.');
        if (ptr) {
            ptr++;
        }
    }

    if (i == 4) {
        memcpy(ip_bin, ip_buf, 4);
        return 0;
    }
    return -1;
}

static int
netagent_io_ipv6_str2bin(char *ip_str, unsigned int *ip_bin) {
    int i = 0;
    char *ptr = 0;
    unsigned char ip_buf[16] = {0};

    if (!ip_str || !ip_bin) {
        return -1;
    }

    ptr = ip_str;
    for (i=0 ; i<16 && ptr && *ptr ; ++i) {
        ip_buf[i] =  (unsigned char)strtoul(ptr, 0, 16);
        ptr = strchr(ptr, ':');
        if (ptr) {
            ptr++;
        }
    }

    if (i == 16) {
        memcpy(ip_bin, ip_buf, 16);
        return 0;
    }
    return -1;
}

static int
netagent_io_parser_addr_str(netagent_io_cmd_obj_t *cmd_obj, char *arg1, char *arg2, char *arg3) {
    if (!cmd_obj) {
        return -1;
    }

    switch (cmd_obj->parameter.ip.addr_type) {
        case NETAGENT_IO_ADDR_TYPE_IPv4 : {
            netagent_io_ipv4_str2bin(arg1, &(cmd_obj->parameter.ip.addr.addr_v4));
            cmd_obj->reason = strdup(arg2);
            break;
        }
        case NETAGENT_IO_ADDR_TYPE_IPv6 : {
            netagent_io_ipv6_str2bin(arg1, cmd_obj->parameter.ip.addr.addr_v6);
            cmd_obj->reason = strdup(arg2);
            break;
        }
        case NETAGENT_IO_ADDR_TYPE_IPv4v6 : {
            netagent_io_ipv4_str2bin(arg1, &(cmd_obj->parameter.ip.addr.addr_v4));
            netagent_io_ipv6_str2bin(arg2, cmd_obj->parameter.ip.addr.addr_v6);
            cmd_obj->reason = strdup(arg3);
            break;
        }
    }

    return 0;
}

static int
netagent_io_parser_tcp_addr_str(netagent_io_cmd_obj_t *cmd_obj, char *ip_str) {
    int i = 0;
    char *ptr = 0;
    unsigned char ip_buf[16] = {0};

    if (!cmd_obj || !ip_str) {
        return -1;
    }

    ptr = ip_str;
    for (i = 0; ptr && *ptr ; ++i) {
        ip_buf[i] =  (unsigned char)strtoul(ptr, 0, 10);
        ptr = strchr(ptr, '.');
        if (ptr) {
            ptr++;
        }
    }

    if (i == 4) {
        cmd_obj->parameter.tcp.addr_type = NETAGENT_IO_ADDR_TYPE_IPv4;
        memcpy(&(cmd_obj->parameter.tcp.addr.addr_v4), ip_buf, 4);
        return 0;
    } else if (i == 16) {
        cmd_obj->parameter.tcp.addr_type = NETAGENT_IO_ADDR_TYPE_IPv6;
        memcpy(cmd_obj->parameter.tcp.addr.addr_v6, ip_buf, 16);
        return 0;
    }

    return -1;
}

static int
netagent_io_parser_spi_addr_str(netagent_io_cmd_obj_t *cmd_obj, char *ip_src_str, char *ip_dst_str) {
    int i = 0;
    char *ptr = 0;
    unsigned char ip_buf[16] = {0};

    if (!cmd_obj || !ip_src_str || !ip_dst_str) {
        return -1;
    }

    ptr = ip_src_str;
    for (i = 0; ptr && *ptr ; ++i) {
        ip_buf[i] =  (unsigned char)strtoul(ptr, 0, 10);
        ptr = strchr(ptr, '.');
        if (ptr) {
            ptr++;
        }
    }

    if (i == 4) {
        cmd_obj->parameter.spi.addr_type = NETAGENT_IO_ADDR_TYPE_IPv4;
        memcpy(&(cmd_obj->parameter.spi.addrSrc.addr_v4), ip_buf, 4);
    } else if (i == 16) {
        cmd_obj->parameter.spi.addr_type = NETAGENT_IO_ADDR_TYPE_IPv6;
        memcpy(cmd_obj->parameter.spi.addrSrc.addr_v6, ip_buf, 16);
    }

    memset(ip_buf, 0, sizeof(ip_buf));
    ptr = ip_dst_str;
    for (i = 0; ptr && *ptr ; ++i) {
        ip_buf[i] =  (unsigned char)strtoul(ptr, 0, 10);
        ptr = strchr(ptr, '.');
        if (ptr) {
            ptr++;
        }
    }

    if (i == 4) {
        memcpy(&(cmd_obj->parameter.spi.addrDst.addr_v4), ip_buf, 4);
        return 0;
    } else if (i == 16) {
        memcpy(cmd_obj->parameter.spi.addrDst.addr_v6, ip_buf, 16);
        return 0;
    }

    return -1;
}

static int
netagent_io_parser_ho_addr_str(netagent_io_cmd_obj_t *cmd_obj, char *arg1, char *arg2) {
    if (!cmd_obj) {
        return -1;
    }

    switch (cmd_obj->parameter.ho.info.addr_type) {
        case NETAGENT_IO_ADDR_TYPE_UNKNOWN : {
            // No address information
            break;
        }
        case NETAGENT_IO_ADDR_TYPE_IPv4 : {
            netagent_io_ipv4_str2bin(arg1, &(cmd_obj->parameter.ho.addr.addr_v4));
            break;
        }
        case NETAGENT_IO_ADDR_TYPE_IPv6 : {
            netagent_io_ipv6_str2bin(arg1, cmd_obj->parameter.ho.addr.addr_v6);
            break;
        }
        case NETAGENT_IO_ADDR_TYPE_IPv4v6 : {
            netagent_io_ipv4_str2bin(arg1, &(cmd_obj->parameter.ho.addr.addr_v4));
            netagent_io_ipv6_str2bin(arg2, cmd_obj->parameter.ho.addr.addr_v6);
            break;
        }
        default :
            ErrMsg("Unexpect address type %d", cmd_obj->parameter.ho.info.addr_type);
            break;
    }

    return 0;
}

static int
netagent_io_parser_ip_add_del_addr_str(netagent_io_cmd_obj_t *cmd_obj, char *arg1) {
    if (!cmd_obj) {
        return -1;
    }

    if (cmd_obj->cmd == NETAGENT_IO_CMD_IPADD) {
        if (cmd_obj->parameter.ipadd.addr_type == NETAGENT_IO_ADDR_TYPE_IPv6) {
            netagent_io_ipv6_str2bin(arg1, cmd_obj->parameter.ipadd.addr.addr_v6);
        } else {
            ErrMsg("Unexpect addr type: %d", cmd_obj->parameter.ipadd.addr_type);
        }
    } else if (cmd_obj->cmd == NETAGENT_IO_CMD_IPDEL) {
        if (cmd_obj->parameter.ipdel.addr_type == NETAGENT_IO_ADDR_TYPE_IPv6) {
            netagent_io_ipv6_str2bin(arg1, cmd_obj->parameter.ipdel.addr.addr_v6);
        } else {
            ErrMsg("Unexpect addr type: %d", cmd_obj->parameter.ipdel.addr_type);
        }
    } else {
        ErrMsg("Unexpect cmd: %d", cmd_obj->cmd);
    }

    return 0;
}

void *
netagent_io_recv(void *o) {
    netagent_io_t *io = o;
    netagent_io_cmd_obj_t *cmd_obj = 0;;

    if (!o) {
        ErrMsg("parameter is NULL");
        return 0;
    }

    while (1) {
        char *line = atapi_urc_get(io->urc_ch);
        if (line) {
            char *argv[16] = {0};
            int num = 0;

            if (strncmp(line, "+EIF: ", 6) == 0) {
                {
                    int index = 0;
                    index = strcspn(line, "\r\n");
                    line[index] = 0;
                }
                DbgMsg("recv urc : %s", line);

                num = _netagent_io_at_parser(line+6, 16, argv);
                if (num < 2) {
                    ErrMsg("URC format error");
                    break;
                }

                cmd_obj = netagent_io_cmd_alloc(strtoul(argv[0], 0, 10), NETAGENT_IO_CMD_MAX);
                if (!cmd_obj) {
                    ErrMsg("can't allocate obj");
                    break;
                }

                if (strcmp(argv[1], "ifup") == 0) {
                    if (num < 3) {
                        netagent_io_cmd_free(cmd_obj);
                        cmd_obj = 0;
                        ErrMsg("URC format error");
                        break;
                    }
                    cmd_obj->cmd = NETAGENT_IO_CMD_IFUP;
                    cmd_obj->parameter.ip.addr_type = strtoul(argv[2], 0, 16);
                    netagent_io_parser_addr_str(cmd_obj, argv[3], argv[4], argv[5]);
                } else if (strcmp(argv[1], "ifdown") == 0) {
                    cmd_obj->cmd = NETAGENT_IO_CMD_IFDOWN;
                    cmd_obj->reason = strdup(argv[2]);
                } else if (strcmp(argv[1], "mtu") == 0) {
                    cmd_obj->cmd = NETAGENT_IO_CMD_SETMTU;
                    cmd_obj->parameter.mtu.mtu_size = strtoul(argv[2], 0, 10);
                    //set mtu size
                } else if (strcmp(argv[1], "ifchg") == 0) {
                    if (num < 3) {
                        netagent_io_cmd_free(cmd_obj);
                        cmd_obj = 0;
                        ErrMsg("URC format error");
                        break;
                    }
                    cmd_obj->cmd = NETAGENT_IO_CMD_IFCHG;
                    cmd_obj->parameter.ip.addr_type = strtoul(argv[2], 0, 16);
                    netagent_io_parser_addr_str(cmd_obj, argv[3], argv[4], argv[5]);
                } else if (strcmp(argv[1], "ho") == 0) {
                    if (num < 7) {
                        netagent_io_cmd_free(cmd_obj);
                        cmd_obj = 0;
                        ErrMsg("URC format error");
                        break;
                    }
                    cmd_obj->cmd = NETAGENT_IO_CMD_PDNHO;
                    cmd_obj->parameter.ho.info.cid = strtol(argv[2], 0, 10);
                    cmd_obj->parameter.ho.info.hostate = strtoul(argv[3], 0, 10);
                    cmd_obj->parameter.ho.info.is_succ = strtoul(argv[4], 0, 10);
                    cmd_obj->parameter.ho.info.src_ran = strtoul(argv[5], 0, 10);
                    cmd_obj->parameter.ho.info.tgt_ran = strtoul(argv[6], 0, 10);
                    cmd_obj->parameter.ho.info.addr_type = strtoul(argv[7], 0, 16);
                    netagent_io_parser_ho_addr_str(cmd_obj, argv[8], argv[9]);

                    dumpPdnHoInfo(&(cmd_obj->parameter.ho.info));

                } else if (strcmp(argv[1], "ipchg") == 0) {
                    if (num < 3) {
                        netagent_io_cmd_free(cmd_obj);
                        cmd_obj = 0;
                        ErrMsg("URC format error");
                        break;
                    }
                    cmd_obj->cmd = NETAGENT_IO_CMD_IPCHG;
                    cmd_obj->parameter.ip.addr_type = strtoul(argv[2], 0, 16);
                    netagent_io_parser_addr_str(cmd_obj, argv[3], argv[4], argv[5]);
                } else if (strcmp(argv[1], "ipadd") == 0) {
                    if (num < 3) {
                        netagent_io_cmd_free(cmd_obj);
                        cmd_obj = 0;
                        ErrMsg("URC format error");
                        break;
                    }
                    cmd_obj->cmd = NETAGENT_IO_CMD_IPADD;
                    cmd_obj->parameter.ipadd.addr_type = strtoul(argv[2], 0, 16);
                    netagent_io_parser_ip_add_del_addr_str(cmd_obj, argv[3]);
                } else if (strcmp(argv[1], "ipdel") == 0) {
                    if (num < 3) {
                        netagent_io_cmd_free(cmd_obj);
                        cmd_obj = 0;
                        ErrMsg("URC format error");
                        break;
                    }
                    cmd_obj->cmd = NETAGENT_IO_CMD_IPDEL;
                    cmd_obj->parameter.ipdel.addr_type = strtoul(argv[2], 0, 16);
                    netagent_io_parser_ip_add_del_addr_str(cmd_obj, argv[3]);
                } else {
                    netagent_io_cmd_free(cmd_obj);
                    cmd_obj = 0;
                    ErrMsg("invalid cmd str = (%s)", argv[1]);
                    break;
                }
            } else if(strncmp(line, "+ENAPTR: ", 9) == 0) {
                {
                /*+ENAPTR: <trans_id>,<mod_id>,<fqdn>*/
                //ENAPTR: 100001,"WO","epdg.epc.mcc345.visited-country.pub.3gppnetwork.org"
                    int index = 0;
                    index = strcspn(line, "\r\n");
                    line[index] = 0;
                }
                DbgMsg("recv urc : %s", line);

                num = _netagent_io_at_parser(line + 9, 16, argv);

                if (num < 3) {
                    cmd_obj = 0;
                    ErrMsg("URC format error");
                    break;
                }

                cmd_obj = netagent_io_cmd_naptr_init_alloc(strtoul(argv[0], 0, 10), NETAGENT_IO_CMD_NAPTR_QUERY);
                if (!cmd_obj) {
                    cmd_obj = 0;
                    ErrMsg("can't allocate obj");
                    break;
                }
                strncpy(cmd_obj->parameter.naptr.mod_id, argv[1], MAX_MOD_NAME_LENGTH-1);
                strncpy(cmd_obj->parameter.naptr.fqdn, argv[2], MAX_FQDN_LENGTH-1);
            } else if(strncmp(line, "+EWIFIMAC: ", 11) == 0) {
                {
                    int index = 0;
                    index = strcspn(line, "\r\n");
                    line[index] = 0;
                }
                DbgMsg("recv urc : %s", line);
                num = _netagent_io_at_parser(line+11, 16, argv);
                if (num < 2) {
                    ErrMsg("URC format error");
                    break;
                }
                cmd_obj = netagent_io_cmd_arp_alloc(argv[0], argv[1]);
                if (!cmd_obj) {
                    ErrMsg("can't allocate obj");
                    break;
                }
            } else if (strncmp(line, "+EIPPORT: ", 10) == 0) {
                /*
                    URC which MD uses to request a port:
                    +EIPPORT: <transaction id>, <0: alloc, 1: free>, <interface ID, range -1-99>,
                        <local IP address>, <IANA protocol numbers, 6: tcp, 17: udp>,
                        <port to be allocated or freed>
                    <interface ID>:
                        0-98: PDN connection identifier
                        99: WiFi interface
                        -1: No interface (reservation isn't interface specific)
                    e.x. +EIPPORT: 123, 0, 1, "192.168.1.1", 6, 2000
                */
                {
                    int index = 0;
                    index = strcspn(line, "\r\n");
                    line[index] = 0;
                }
                DbgMsg("recv urc : %s", line);

                num = _netagent_io_at_parser(line+10, 16, argv);
                if (num < 6) {
                    ErrMsg("URC format error");
                    break;
                }

                // using 'trans_intf_id' to represent <transaction ID>
                cmd_obj = netagent_io_cmd_alloc(strtoul(argv[0], 0, 10), NETAGENT_IO_CMD_MAX);
                if (!cmd_obj) {
                    ErrMsg("can't allocate obj");
                    break;
                }

                cmd_obj->parameter.tcp.action = strtoul(argv[1], 0, 10);
                cmd_obj->parameter.tcp.interfaceId = strtol(argv[2], 0, 10);
                netagent_io_parser_tcp_addr_str(cmd_obj, argv[3]);
                cmd_obj->cmd = (strcmp(argv[4], "6") == 0) ? NETAGENT_IO_CMD_TCP_RSVN :
                        NETAGENT_IO_CMD_UDP_RSVN;
                cmd_obj->parameter.tcp.port = strtoul(argv[5], 0, 10);
            } else if (strncmp(line, "+EIPSPI: ", 9) == 0) {
                /*
                    URC which MD uses to request an SPI (similar as "ip xfrm state allocspi"):
                    +EIPSPI: <transaction id>, <0: alloc>, <source IP address>, <destination IP address>,
                        <"esp" | "ah" | "comp" | "route2" | "hao">, <mode>, <min spi>, <max spi>
                    e.x. EIPSPI: 456, 0, "192.168.1.1", "192.168.1.2", "esp", "transport", 0, 4294967295

                    URC which MD uses to free an SPI (similar as "ip xfrm state delete"):
                    +EIPSPI: <transaction id>, <1: free>, <source IP address>, <destination IP address>,
                        <"esp" | "ah" | "comp" | "route2" | "hao">, <spi to be freed>
                    e.x. EIPSPI: 789, 1, "192.168.1.1", "192.168.1.2", "esp", 11223344
                */
                {
                    int index = 0;
                    index = strcspn(line, "\r\n");
                    line[index] = 0;
                }
                DbgMsg("recv urc : %s", line);

                num = _netagent_io_at_parser(line+9, 16, argv);
                if (num < 6) {
                    ErrMsg("URC format error");
                    break;
                }

                // using 'trans_intf_id' to represent <transaction ID>
                cmd_obj = netagent_io_cmd_alloc(strtoul(argv[0], 0, 10), NETAGENT_IO_CMD_MAX);
                if (!cmd_obj) {
                    ErrMsg("can't allocate obj");
                    break;
                }

                cmd_obj->cmd = NETAGENT_IO_CMD_SPI_RSVN;
                cmd_obj->parameter.spi.action = strtoul(argv[1], 0, 10);
                netagent_io_parser_spi_addr_str(cmd_obj, argv[2], argv[3]);
                if (strcmp(argv[4], "esp") == 0) {
                    cmd_obj->parameter.spi.protocol = IPPROTO_ESP;
                } else if (strcmp(argv[4], "ah") == 0) {
                    cmd_obj->parameter.spi.protocol = IPPROTO_AH;
                } else if (strcmp(argv[4], "comp") == 0) {
                    cmd_obj->parameter.spi.protocol = IPPROTO_COMP;
                } else if (strcmp(argv[4], "route2") == 0) {
                    cmd_obj->parameter.spi.protocol = IPPROTO_ROUTING;
                } else if (strcmp(argv[4], "hao") == 0) {
                    cmd_obj->parameter.spi.protocol = IPPROTO_DSTOPTS;
                } else if (strcmp(argv[4], "ipsec-any") == 0) {
                    cmd_obj->parameter.spi.protocol = IPSEC_PROTO_ANY;
                }
                if (cmd_obj->parameter.spi.action == 0) {
                    if (strcmp(argv[5], "transport") == 0) {
                        cmd_obj->parameter.spi.mode = XFRM_MODE_TRANSPORT;
                    } else if (strcmp(argv[5], "tunnel") == 0) {
                        cmd_obj->parameter.spi.mode = XFRM_MODE_TUNNEL;
                    } else if (strcmp(argv[5], "ro") == 0) {
                        cmd_obj->parameter.spi.mode = XFRM_MODE_ROUTEOPTIMIZATION;
                    } else if (strcmp(argv[5], "in_trigger") == 0) {
                        cmd_obj->parameter.spi.mode = XFRM_MODE_IN_TRIGGER;
                    } else if (strcmp(argv[5], "beet") == 0) {
                        cmd_obj->parameter.spi.mode = XFRM_MODE_BEET;
                    }
                    cmd_obj->parameter.spi.min = strtoul(argv[6], 0, 10);
                    cmd_obj->parameter.spi.max = strtoul(argv[7], 0, 10);
                } else {
                    // using 'mode' to remember the spi to be freed
                    cmd_obj->parameter.spi.mode = strtoul(argv[5], 0, 10);
                }
            }

            free(line);

            if (cmd_obj) {
                return cmd_obj;
            }
        }
    }

    return 0;
}

int netagent_io_send(void *o, void *co) {
    netagent_io_t *io = o;
    netagent_io_cmd_obj_t *cmd_obj = co;
    int ret = 0;
    char cmd_buf[512] = {0};
    char *tmp_resp_buf = NULL;
    char *resp_buf = NULL;

    if (!o) {
        ErrMsg("parameter is NULL");
        return NETAGENT_IO_RET_GENERIC_FAILURE;
    }

    if (!cmd_obj) {
        ErrMsg("obj is NULL");
        return NETAGENT_IO_RET_GENERIC_FAILURE;
    }

    switch (cmd_obj->cmd) {
        case NETAGENT_IO_CMD_IFST :
            snprintf(cmd_buf, sizeof(cmd_buf), "AT+EIF=%d, \"ifst\", %s, %d",
                cmd_obj->trans_intf_id,
                cmd_obj->parameter.ifst.state == NETAGENT_IO_IFST_UP ? "\"up\"" : "\"down\"",
                cmd_obj->parameter.ifst.addr_type);
            break;
        case NETAGENT_IO_CMD_ARP_RESULT :
            snprintf(cmd_buf, sizeof(cmd_buf), "AT+EWIFIMAC=%d, \"%s\" , \"%s\", \"%s\"",
                cmd_obj->parameter.arp.fail_code,
                cmd_obj->parameter.arp.ifname,
                cmd_obj->parameter.arp.ip,
                cmd_obj->parameter.arp.mac);
            break;
        case NETAGENT_IO_CMD_IPUPDATE : {
            unsigned char *addr_ptr = 0;
            int i = 0;
            int offset = 0;

            offset = snprintf(cmd_buf, sizeof(cmd_buf), "AT+EIF=%d, \"ipupdate\", %d, \"",
                cmd_obj->trans_intf_id,
                cmd_obj->parameter.ip.addr_type);

            if (cmd_obj->parameter.ip.addr_type == NETAGENT_IO_ADDR_TYPE_IPv4) {
                addr_ptr = (unsigned char *)&(cmd_obj->parameter.ip.addr.addr_v4);
                for (i = 0 ; i<4 ; ++i) {
                    if (i == 0) {
                        offset += snprintf(cmd_buf+offset, sizeof(cmd_buf)-offset, "%d", *addr_ptr);
                    } else {
                        offset += snprintf(cmd_buf+offset, sizeof(cmd_buf)-offset, ".%d", *addr_ptr);
                    }
                    addr_ptr++;
                }
            } else if (cmd_obj->parameter.ip.addr_type == NETAGENT_IO_ADDR_TYPE_IPv6) {
                addr_ptr = (unsigned char *)cmd_obj->parameter.ip.addr.addr_v6;
                for (i = 0 ; i<16 ; ++i) {
                    if (i == 0) {
                        offset += snprintf(cmd_buf+offset, sizeof(cmd_buf)-offset, "%02X", *addr_ptr);
                    } else {
                        offset += snprintf(cmd_buf+offset, sizeof(cmd_buf)-offset, ":%02X", *addr_ptr);
                    }
                    addr_ptr++;
                }
                int ipv6_prefix_length = cmd_obj->parameter.ip.ipv6_prefix_length;
                if (ipv6_prefix_length != INVALID_IPV6_PREFIX_LENGTH) {
                    offset += snprintf(cmd_buf+offset, sizeof(cmd_buf)-offset, "/%d", ipv6_prefix_length);
                }
            } else {
                ErrMsg("invalid addr type = %d", cmd_obj->parameter.ip.addr_type);
                return NETAGENT_IO_RET_GENERIC_FAILURE;
            }
            offset += snprintf(cmd_buf+offset, sizeof(cmd_buf)-offset, "\"");

            break;
        }

        case NETAGENT_IO_CMD_IPADD : {
            unsigned char *addr_ptr = 0;
            int i = 0;
            int offset = 0;

            if (cmd_obj->parameter.ipadd.result == 99) {
                offset = snprintf(cmd_buf, sizeof(cmd_buf), "AT+EIF=%d, \"ipadd\", %d, \"",
                    cmd_obj->trans_intf_id,
                    cmd_obj->parameter.ipadd.addr_type);
            } else {
                offset = snprintf(cmd_buf, sizeof(cmd_buf), "AT+EIF=%d, \"ipadd\", %d, %d, \"",
                    cmd_obj->trans_intf_id,
                    cmd_obj->parameter.ipadd.result,
                    cmd_obj->parameter.ipadd.addr_type);
            }

            if (cmd_obj->parameter.ipadd.addr_type == NETAGENT_IO_ADDR_TYPE_IPv6) {
                addr_ptr = (unsigned char *)cmd_obj->parameter.ipadd.addr.addr_v6;
                for (i = 0 ; i<16 ; ++i) {
                    if (i == 0) {
                        offset += snprintf(cmd_buf+offset, sizeof(cmd_buf)-offset, "%02X", *addr_ptr);
                    } else {
                        offset += snprintf(cmd_buf+offset, sizeof(cmd_buf)-offset, ":%02X", *addr_ptr);
                    }
                    addr_ptr++;
                }
                int ipv6_prefix_length = cmd_obj->parameter.ip.ipv6_prefix_length;
                if (ipv6_prefix_length != INVALID_IPV6_PREFIX_LENGTH) {
                    offset += snprintf(cmd_buf+offset, sizeof(cmd_buf)-offset, "/%d", ipv6_prefix_length);
                }
            } else {
                ErrMsg("invalid addr type = %d", cmd_obj->parameter.ipadd.addr_type);
                return NETAGENT_IO_RET_GENERIC_FAILURE;
            }
            offset += snprintf(cmd_buf+offset, sizeof(cmd_buf)-offset, "\"");

            break;
        }

        case NETAGENT_IO_CMD_IPDEL : {
            unsigned char *addr_ptr = 0;
            int i = 0;
            int offset = 0;

            if (cmd_obj->parameter.ipdel.result == 99) {
                offset = snprintf(cmd_buf, sizeof(cmd_buf), "AT+EIF=%d, \"ipdel\", %d, \"",
                    cmd_obj->trans_intf_id,
                    cmd_obj->parameter.ipdel.addr_type);
            } else {
                offset = snprintf(cmd_buf, sizeof(cmd_buf), "AT+EIF=%d, \"ipdel\", %d, %d, \"",
                    cmd_obj->trans_intf_id,
                    cmd_obj->parameter.ipdel.result,
                    cmd_obj->parameter.ipdel.addr_type);
            }

            if (cmd_obj->parameter.ipdel.addr_type == NETAGENT_IO_ADDR_TYPE_IPv6) {
                addr_ptr = (unsigned char *)cmd_obj->parameter.ipdel.addr.addr_v6;
                for (i = 0 ; i<16 ; ++i) {
                    if (i == 0) {
                        offset += snprintf(cmd_buf+offset, sizeof(cmd_buf)-offset, "%02X", *addr_ptr);
                    } else {
                        offset += snprintf(cmd_buf+offset, sizeof(cmd_buf)-offset, ":%02X", *addr_ptr);
                    }
                    addr_ptr++;
                }
                int ipv6_prefix_length = cmd_obj->parameter.ip.ipv6_prefix_length;
                if (ipv6_prefix_length != INVALID_IPV6_PREFIX_LENGTH) {
                    offset += snprintf(cmd_buf+offset, sizeof(cmd_buf)-offset, "/%d", ipv6_prefix_length);
                }
            } else {
                ErrMsg("invalid addr type = %d", cmd_obj->parameter.ipdel.addr_type);
                return NETAGENT_IO_RET_GENERIC_FAILURE;
            }
            offset += snprintf(cmd_buf+offset, sizeof(cmd_buf)-offset, "\"");

            break;
        }

        case NETAGENT_IO_CMD_RA :
            snprintf(cmd_buf, sizeof(cmd_buf), "AT+EIF=%d, \"ra\", \"%s\"",
                    cmd_obj->trans_intf_id,
                    cmd_obj->parameter.ra.flag == NETAGENT_IO_NO_RA_INITIAL ? "no_ra_initial" : "no_ra_refresh");
            break;
        case NETAGENT_IO_CMD_PDNHO :
            snprintf(cmd_buf, sizeof(cmd_buf), "AT+EIF=%d, \"ho\", 1", cmd_obj->trans_intf_id);
            break;
        case NETAGENT_IO_CMD_SYNC_CAPABILITY: {
            //AT+EIFCONFIG=<support>,[support2]
            int capability = NETAGENT_CAPABILITY_CLEAR_IPSEC + NETAGENT_CAPABILITY_HO_IP_CHANGED;
            snprintf(cmd_buf, sizeof(cmd_buf), "AT+EIFCONFIG=%d", capability);
            break;
        }
        case NETAGENT_IO_CMD_NAPTR_CAPABILITY :
            //AT+ENAPTRCONFIG=<enable>
            //<enalbe>: 1 for support, 0 for not support
            snprintf(cmd_buf, sizeof(cmd_buf), "AT+ENAPTRCONFIG=1");
            break;
        case NETAGENT_IO_CMD_NAPTR_SEND :
            /* AT+ENAPTR=<trans_id>,<mod_id>,<result>,<order>,<pref>,<flags>,<service>,<regexp>,<replacement>*/
            snprintf(cmd_buf, sizeof(cmd_buf), "AT+ENAPTR=%d,\"%s\",%d,%d,%d,\"%s\",\"%s\",\"%s\",\"%s\"",
                    cmd_obj->parameter.naptr.trans_id,
                    cmd_obj->parameter.naptr.mod_id,
                    cmd_obj->parameter.naptr.result,
                    cmd_obj->parameter.naptr.order,
                    cmd_obj->parameter.naptr.pref,
                    cmd_obj->parameter.naptr.flags,
                    cmd_obj->parameter.naptr.service,
                    cmd_obj->parameter.naptr.regexp,
                    cmd_obj->parameter.naptr.fqdn);
             break;
         case NETAGENT_IO_CMD_TCP_RSVN:
         case NETAGENT_IO_CMD_UDP_RSVN:
             // AT+EIPPORT=<transaction id>,<0: alloc, 1: free>,<0: failure: 1: success>
             snprintf(cmd_buf, sizeof(cmd_buf), "AT+EIPPORT=%d, %d, %d",
                     cmd_obj->trans_intf_id,
                     cmd_obj->parameter.tcp.action,
                     cmd_obj->parameter.tcp.response);
             break;
         case NETAGENT_IO_CMD_SPI_RSVN:
             // AT+EIPSPI=<transaction id>, <0: alloc>, <0: failure, otherwise the allocated SPI>
             // AT+EIPSPI=<transaction id>, <1: free>, <0: failure, 1: success>
             snprintf(cmd_buf, sizeof(cmd_buf), "AT+EIPSPI=%d, %d, %d",
                     cmd_obj->trans_intf_id,
                     cmd_obj->parameter.spi.action,
                     cmd_obj->parameter.spi.response);
             break;
    }

    DbgMsg("AT> %s", cmd_buf);
    resp_buf = (char*)calloc(1, MAX_AT_RESPONSE_BUFFER * sizeof(char));
    ret = atapi_execute_command(io->cmd_ch, cmd_buf, resp_buf, MAX_AT_RESPONSE_BUFFER, 0, 30);
    if (ret != ATAPI_RET_SUCCESS) {
        ErrMsg("ret = %d", ret);
        FREEIF(resp_buf);
        return NETAGENT_IO_RET_AT_FAIL;
    }
    tmp_resp_buf = resp_buf;
    while (*tmp_resp_buf == '\r' || *tmp_resp_buf == '\n') {
        tmp_resp_buf++;
    }
    {
        int index = 0;
        index = strcspn(tmp_resp_buf, "\r\n");
        tmp_resp_buf[index] = 0;
    }
    DbgMsg("AT< %s", tmp_resp_buf);
    FREEIF(resp_buf);
    return NETAGENT_IO_RET_SUCCESS;
}

void dumpPdnHoInfo(netagent_io_ho_info_t *info) {
    if (info != NULL) {
        DbgMsg("[PdnHoInfo] cid: %d, hostate: %d, is_succ: %d, src_ran: %d, tgt_ran: %d, addr_type: %d",
                info->cid, info->hostate, info->is_succ, info->src_ran, info->tgt_ran, info-> addr_type);
    }
}

int netagent_io_flush_ipsec_policy(const char *src_addr, netagent_io_addr_type_e addr_type) {
    int status = 0;
    char prefix[22] = "";

    // M: ALPS03937238 Process prefix only for IPV6 type address
    if(src_addr != NULL && strlen(src_addr)>= 20 && addr_type != NETAGENT_IO_ADDR_TYPE_IPv4){
        memset(&prefix, 0, sizeof(prefix));
        strncpy(prefix,src_addr,20);
        prefix[20]=':';
        prefix[21]='\0';
    }
    DbgMsg("flush IPSEC policy, addr %s, addr_type %d,prefix %s", src_addr, addr_type, prefix);

    switch (addr_type) {
        // Clear policy for 32 bit IPv4 address
        case NETAGENT_IO_ADDR_TYPE_IPv4 :
            netagent_io_exec_fmt(&status,
                                 SYSTEM_IP_WRAPPER_BIN_DIR " xfrm policy delete dir out src %s",
                                 src_addr);
            netagent_io_exec_fmt(&status,
                                 SYSTEM_IP_WRAPPER_BIN_DIR " xfrm policy delete dir in dst %s",
                                 src_addr);
            netagent_io_exec_fmt(&status,
                                 SYSTEM_IP_WRAPPER_BIN_DIR " xfrm policy delete dir fwd dst %s",
                                 src_addr);
            break;
        // Clear policy for IPv6 address with 64 bits prefix
        case NETAGENT_IO_ADDR_TYPE_IPv6 :
            netagent_io_exec_fmt(&status,
                                 SYSTEM_IP_WRAPPER_BIN_DIR " xfrm policy delete dir out src %s/64",
                                 prefix);
            netagent_io_exec_fmt(&status,
                                 SYSTEM_IP_WRAPPER_BIN_DIR " xfrm policy delete dir in dst %s/64",
                                 prefix);
            netagent_io_exec_fmt(&status,
                                 SYSTEM_IP_WRAPPER_BIN_DIR " xfrm policy delete dir fwd dst %s/64",
                                 prefix);
            break;
        default:
            DbgMsg("Invalid addr_type %d", addr_type);
    }
    return 0;
}

int netagent_io_exec_fmt(int *status, const char *fmt, ...) {
    char *s = NULL;
    va_list args;
    int ret = 0;
    va_start(args, fmt);
    vasprintf(&s, fmt, args);
    ret = netagent_io_fork_exec(s, status);
    free(s);
    va_end(args);
    return ret;
}

#define MAX_EXECV_ARG 24
int netagent_io_fork_exec(const char *cmd, int *status) {
    const char *DELIM = " ";
    char *clone;
    int argc;
    char *argv[MAX_EXECV_ARG] = {0};

    if (cmd == NULL) {
        return 0;
    }

    clone = strdup(cmd);
    argv[0] = strtok(clone, DELIM);

    for (argc = 1; argc < MAX_EXECV_ARG; argc++) {
        if ((argv[argc] = strtok(NULL, DELIM)) == NULL) {
            break;
        }
    }

    if (netagent_io_system_ext(argv) != 0) {
        free(clone);
        return -1;
    }

    free(clone);
    return 0;
}

int netagent_io_system_ext(char *argv[]) {
    int pid = fork();

    if (pid == 0) {
        execv(argv[0], argv);
        return -1;
    } else if (pid > 0) {
        int wstatus, wexit_status;
        waitpid(pid, &wstatus, 0);
        wexit_status = WEXITSTATUS(wstatus);
        DbgMsg("Child process exit status: %d", wexit_status);
    }  else {
        ErrMsg("Can't fork process");
        return -1;
    }
    return 0;
}

