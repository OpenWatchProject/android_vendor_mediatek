/****************************************************************************
Copyright (c) 2016 Wi-Fi Alliance.  All Rights Reserved

Permission to use, copy, modify, and/or distribute this software for any purpose with or
without fee is hereby granted, provided that the above copyright notice and this permission
notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

******************************************************************************/

/*
 *   File: wfa_cs.c -- configuration and setup
 *   This file contains all implementation for the dut setup and control
 *   functions, such as network interfaces, ip address and wireless specific
 *   setup with its supplicant.
 *
 *   The current implementation is to show how these functions
 *   should be defined in order to support the Agent Control/Test Manager
 *   control commands. To simplify the current work and avoid any GPL licenses,
 *   the functions mostly invoke shell commands by calling linux system call,
 *   system("<commands>").
 *
 *   It depends on the differnt device and platform, vendors can choice their
 *   own ways to interact its systems, supplicants and process these commands
 *   such as using the native APIs.
 *
 *
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/types.h>
#include <linux/socket.h>
#include <linux/if.h>
#include <linux/if_bridge.h>
#include <poll.h>
#include <ctype.h>

#include "wfa_portall.h"
#include "wfa_debug.h"
#include "wfa_ver.h"
#include "wfa_main.h"
#include "wfa_types.h"
#include "wfa_ca.h"
#include "wfa_tlv.h"
#include "wfa_sock.h"
#include "wfa_tg.h"
#include "wfa_cmds.h"
#include "wfa_rsp.h"
#include "wfa_utils.h"
#ifdef WFA_WMM_PS_EXT
#include "wfa_wmmps.h"
#endif

/**
 * Forward declaration for internal utilities
 */
#include "wpa_helpers.h"

#define AP_BRIDGE_INF_NAME "br0"
#define USB_INF_NAME "rndis0"

#define UNSET_VALUE -1

typedef struct sta_params {
    caStaSetIpConfig_t ipconfig;
    /* use array below this line. Must sync with set_params and get_params */
    char intf[WFA_IF_NAME_LEN];
    int type[1];
    char ssid[WFA_SSID_NAME_LEN];
    char keyMgmtType[16];
    char encpType[16];
    int pmf[1];
    int akmSuiteType[1];
    char clientCertificate[128];
    char certType[16];
    char ecGroupID[64];
    char groupCipher[16];
    char groupMgntCipher[16];
    char innerEAP[16];
    char invalidSAEElement[64];
    char networkMode[16];
    int owe[1];
    char pacFile[32];
    char pairwiseCipher[16];
    char passphrase[64];
    char password[96];
    int pmksaCaching[1];
    int profile[1];
    char prog[16];
    char program[16];
    char trustedRootCA[128];
    char tlsCipher[64];
    char username[64];
    int peapVersion[1];
    int prefer[1];
    char micAlg[16];
} sta_params_t;

#ifdef CONFIG_MTK_AP
typedef struct ap_params {
    char hostapd_bin[64];
    char hostapd_conf[64];
    char driver_conf[64];
    char inf_name[WFA_IF_NAME_LEN];
    char ctrl_inf[64];

    char ip_addr[WFA_IP_ADDR_STR_LEN];
    char ip_netmask[WFA_IP_MASK_STR_LEN];

    char device_name[WFA_IF_NAME_LEN];
    char ssid[WFA_SSID_NAME_LEN];
    enum ENUM_AP_MODE mode;
    int channel;
    int rts;
    int frgmnt;
    int bcnint;
    int dtim_period;
    enum ENUM_CHANNEL_WIDTH ch_width;
    enum ENUM_CHANNEL_OFFSET ch_offset;
    enum wfa_state pmf;
    char passphrase[101];
    char wepkey[27];
    enum wfa_state wme;
    enum wfa_state wmmps;
    enum wfa_state p2p_mgmt;
    char country_code[3];
    enum wfa_state sgi20;
    enum wfa_state sig_rts;
    enum wfa_state dynamic_bw_signaling;
    enum wfa_state preauthentication;
    enum wfa_state sha256ad;
    enum ENUM_KEY_MGNT_TYPE keyMgmtType;
    enum ENUM_ENCP_TYPE encpType;
    enum ENUM_PROGRAME_TYPE program;
    char ap_ecGroupID[64];
    int antiCloggingThreshold;
    int ap_reflection;
    char ap_sae_commit_override[1024];
} ap_params_t;
#endif

#ifdef CONFIG_MTK_P2P
typedef struct p2p_params {
    char current_ssid[WFA_SSID_NAME_LEN];
    enum wfa_state persistent;
    enum ENUM_P2P_MODE p2p_mode;
    enum WPS_METHOD wps_method;
    char wpsPin[WFA_WPS_PIN_LEN];
    int stop_event_thread;
} p2p_params_t;
#endif

typedef struct mtk_dut
{
    char shellResult[WFA_CMD_STR_SZ * 4];

    int ipconfigDeferred;
    int networkId;
    int sta_pmf;
    int program;
    char ssid[WFA_SSID_NAME_LEN];

    enum ENUM_DRIVER_GEN driver_gen;
    sta_params_t sta_params; // params from ucc
#ifdef CONFIG_MTK_AP
    ap_params_t ap_params;
#endif
#ifdef CONFIG_MTK_P2P
    p2p_params_t p2p_params;
#endif
} mtk_dut_t;

mtk_dut_t gDut;
const char *sigma_mode_tbl[] = {
    MODE_WMM_PS,        /* 0 */
    MODE_WMM_AC,        /* 1 */
    MODE_VOE,           /* 2 */
    MODE_P2P,           /* 3 */
    MODE_AP,            /* 4 */
    MODE_TDLS,          /* 5 */
    MODE_TG_N,          /* 6 */
    MODE_TG_AC,         /* 7 */
    MODE_WPA3,          /* 8 */
};
enum {
    SIGMA_MODE_WMM_PS,  /* 0 */
    SIGMA_MODE_WMM_AC,  /* 1 */
    SIGMA_MODE_VOE,     /* 2 */
    SIGMA_MODE_P2P,     /* 3 */
    SIGMA_MODE_AP,      /* 4 */
    SIGMA_MODE_TDLS,    /* 5 */
    SIGMA_MODE_TG_N,    /* 6 */
    SIGMA_MODE_TG_AC,   /* 7 */
    SIGMA_MODE_WPA3,    /* 8 */
};
int sigma_mode = SIGMA_MODE_WMM_AC;

int ascii2hexstr(const char *str, char *hex);
int channel2freq(char channel);
const char* prog2str(int prog);
int str2prog(const char *str);
const char* encpType2str(int encpType);
int str2encpType(const char *str);
void wfa_cs_init(void);
int shell(const char *fmt, ...);
int check_connection(const char *ifname);
void update_ip_config(caStaSetIpConfig_t *ipconfig);
void remove_wpa_networks(const char *ifname);
int find_network(const char *ifname, const char *ssid);
int add_network_common(const char *ifname);
int str_same(const char* str1, const char* str2);
int set_wpa_common(const char* intf, const int id);
int set_eap_common(const char* intf, const int id);
void reset_param();
int *get_param_val(const char* name);
char *get_param_str(const char* name);
void set_param_val(const char* name, int value);
void set_param_str(const char* name, char *value);
int sta_set_open(int *respLen, BYTE *respBuf);
int sta_set_owe(int *respLen, BYTE *respBuf);
int sta_mon_conn(const char* intf);

int get_ip_config(const char* intf, caStaGetIpConfigResp_t *ifinfo);
int get_mac(const char* intf, unsigned char* addr);
static void init_driver_gen(void);

// AP functions
#ifdef CONFIG_MTK_AP
static void init_ap_params(void);
static void config_hostapd_htcap(FILE *f, ap_params_t *p_ap_params);
static void config_driver_wmm_params(FILE *f, ap_params_t *p_ap_params);
static void write_hostapd_conf();
static void write_driver_conf();
static int run_hostapd_cli(char *buf);
static void setup_ap_bridge_n_address();
static void mtk_add_ap_bridge();
static void mtk_del_ap_bridge();
static int is_interface_up(char *inf);
static int mtk_bridge_setup(const char *brname, enum ENUM_BRIDGE_OP_TYPE type);
static int mtk_bridge_intf_setup(const char *bridge, const char *dev,
        enum ENUM_BRIDGE_OP_TYPE type);
#endif

// P2P functions
#ifdef CONFIG_MTK_P2P
static char *get_main_intf(void);
static void init_p2p_params(void);
static int p2p_is_peer_known(const char *ifname, const char *peer, int discovered);
static int p2p_find_peer(const char *ifname, const char *peer, int discovered);
static void enable_dhcp_client(char *ifname);
static void enable_dhcp(char *ifname, int enable, int go);
static void *p2p_event_receive(void* data);
static void remove_p2p_persistent_networks(const char *ifname);
// P2P dhcp functions
extern ifc_init(void);
extern void ifc_cifc_initlose(void);
extern int do_dhcp(char *ifname);
#endif

// HS20 functions
#ifdef CONFIG_MTK_PPR2
static int sta_ap_exsit(const char *ifname, const char *bssid);
static int sta_find_ap(const char *ifname, const char *bssid);
static int sta_run_osu_client(const char *params);
static int sta_download_ppsmo(const char *ifname, const char *filename, const char *filepath,
        const char *managementtreeuri);
static int sta_download_cert(const char *ifname, const char *filename, const char *filepath);
static int sta_set_priority(void);
#endif

#define CERTIFICATES_PATH    "/etc/wpa_supplicant"

/* Some device may only support UDP ECHO, activate this line */
//#define WFA_PING_UDP_ECHO_ONLY 1

#define WFA_ENABLED 1

extern unsigned short wfa_defined_debug;
int wfaExecuteCLI(char *CLI);

/* Since the two definitions are used all over the CA function */
char gCmdStr[WFA_CMD_STR_SZ];
dutCmdResponse_t gGenericResp;
int wfaTGSetPrio(int sockfd, int tgClass);
void create_apts_msg(int msg, unsigned int txbuf[],int id);

int sret = 0;

extern char e2eResults[];

FILE *e2efp = NULL;
int chk_ret_status()
{
    char *ret = getenv(WFA_RET_ENV);

    if(*ret == '1')
        return WFA_SUCCESS;
    else
        return WFA_FAILURE;
}

/*
 * agtCmdProcGetVersion(): response "ca_get_version" command to controller
 *  input:  cmd --- not used
 *          valLen -- not used
 *  output: parms -- a buffer to store the version info response.
 */
int agtCmdProcGetVersion(int len, BYTE *parms, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t *getVerResp = &gGenericResp;

    DPRINT_INFO(WFA_OUT, "\nEntering agtCmdProcGetVersion ...\n");

    getVerResp->status = STATUS_COMPLETE;
    wSTRNCPY(getVerResp->cmdru.version, MTK_SYSTEM_VER, sizeof(getVerResp->cmdru.version));
    getVerResp->cmdru.version[sizeof(getVerResp->cmdru.version) - 1] = '\0';

    wfaEncodeTLV(WFA_GET_VERSION_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)getVerResp, respBuf);

    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

    return WFA_SUCCESS;
}

/*
 * wfaStaAssociate():
 *    The function is to force the station wireless I/F to re/associate
 *    with the AP.
 */
int wfaStaAssociate(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCommand_t *assoc = (dutCommand_t *)caCmdBuf;
    char *ifname = assoc->intf;
    dutCmdResponse_t *staAssocResp = &gGenericResp;
    char extra[50], buf[1024];

    DPRINT_INFO(WFA_OUT, "\nEntering wfaStaAssociate ...\n");

    if (strcmp(assoc->cmdsu.assoc.ssid, gDut.ssid) == 0) {
        DPRINT_INFO(WFA_OUT, "Sta associate for the most recently added network\n");
    } else if (find_network(ifname, assoc->cmdsu.assoc.ssid) < 0) {
        DPRINT_ERR(WFA_ERR, "Sta associate for a previously stored network profile but not found\n");
        staAssocResp->status = STATUS_ERROR;
        goto done;
    }

    if(assoc->cmdsu.assoc.bssid[0] != '\0') {
        if (set_network(ifname, gDut.networkId, "bssid", assoc->cmdsu.assoc.bssid) < 0) {
            staAssocResp->status = STATUS_ERROR;
            goto done;
        }
    }

    extra[0] = '\0';
    if (assoc->cmdsu.assoc.channel)
        wSPRINTF(extra, " freq=%u", channel2freq(assoc->cmdsu.assoc.channel));
    wSPRINTF(buf, "SELECT_NETWORK %d%s", gDut.networkId, extra);
    if (wpa_command(ifname, buf) < 0) {
        DPRINT_ERR(WFA_ERR, "Failed to select network id %d on %s",
                        gDut.networkId, ifname);
        staAssocResp->status = STATUS_ERROR;
        goto done;
    }

    sta_mon_conn(ifname);
    update_ip_config(NULL);

    staAssocResp->status = STATUS_COMPLETE;
done:
    wfaEncodeTLV(WFA_STA_ASSOCIATE_RESP_TLV, 4, (BYTE *)staAssocResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + 4;

    return WFA_SUCCESS;
}

/*
 * wfaStaReAssociate():
 *    The function is to force the station wireless I/F to re/associate
 *    with the AP.
 */
int wfaStaReAssociate(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCommand_t *assoc = (dutCommand_t *)caCmdBuf;
    char *ifname = assoc->intf;
    dutCmdResponse_t *staAssocResp = &gGenericResp;
    char buf[1024];

    DPRINT_INFO(WFA_OUT, "\nEntering wfaStaReAssociate ...\n");

    if(assoc->cmdsu.assoc.bssid[0] != '\0') {
        if (set_network(ifname, gDut.networkId, "bssid", assoc->cmdsu.assoc.bssid) < 0) {
            staAssocResp->status = STATUS_ERROR;
            goto done;
        }
    } else {
        staAssocResp->status = STATUS_INVALID;
        goto done;
    }

    if (wpa_command(ifname, "REASSOCIATE") < 0) {
        DPRINT_ERR(WFA_ERR, "Failed to reassociate network id %d on %s",
                        gDut.networkId, ifname);
        staAssocResp->status = STATUS_ERROR;
        goto done;
    }

    sta_mon_conn(ifname);
    update_ip_config(NULL);

    /*
     * Then report back to control PC for completion.
     * This does not have failed/error status. The result only tells
     * a completion.
     */
    staAssocResp->status = STATUS_COMPLETE;
done:
    wfaEncodeTLV(WFA_STA_ASSOCIATE_RESP_TLV, 4, (BYTE *)staAssocResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + 4;

    return WFA_SUCCESS;
}

/*
 * wfaStaIsConnected():
 *    The function is to check whether the station's wireless I/F has
 *    already connected to an AP.
 */
int wfaStaIsConnected(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCommand_t *connStat = (dutCommand_t *)caCmdBuf;
    dutCmdResponse_t *staConnectResp = &gGenericResp;

    DPRINT_INFO(WFA_OUT, "\nEntering wfaStaIsConnected ...\n");

    staConnectResp->cmdru.connected = check_connection(connStat->intf);

    /*
     * Report back the status: Complete or Failed.
     */
    staConnectResp->status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_STA_IS_CONNECTED_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)staConnectResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

    return WFA_SUCCESS;
}

/*
 * wfaStaGetIpConfig():
 * This function is to retriev the ip info including
 *     1. dhcp enable
 *     2. ip address
 *     3. mask
 *     4. primary-dns
 *     5. secondary-dns
 *
 *     The current implementation is to use a script to find these information
 *     and store them in a file.
 */
int wfaStaGetIpConfig(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCommand_t *getIpConf = (dutCommand_t *)caCmdBuf;
    dutCmdResponse_t *ipconfigResp = &gGenericResp;
    char *ifname = getIpConf->intf;
    caStaGetIpConfigResp_t *ifinfo = &ipconfigResp->cmdru.getIfconfig;
    char *buf = NULL;
    char *buf_save = NULL;

    DPRINT_INFO(WFA_OUT, "\nEntering wfaStaGetIpConfig ...\n");

    if (get_ip_config(getIpConf->intf, ifinfo) == WFA_SUCCESS) {
        ipconfigResp->status = STATUS_COMPLETE;
    } else {
        ipconfigResp->status = STATUS_ERROR;
    }
    ifinfo->isDhcp = gDut.sta_params.ipconfig.isDhcp;

    /*
     * report status
     */
    wfaEncodeTLV(WFA_STA_GET_IP_CONFIG_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)ipconfigResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

    return WFA_SUCCESS;
}

/*
 * wfaStaSetIpConfig():
 *   The function is to set the ip configuration to a wireless I/F.
 *   1. IP address
 *   2. Mac address
 *   3. default gateway
 *   4. dns nameserver (pri and sec).
 */
int wfaStaSetIpConfig(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCommand_t *setIpConf = (dutCommand_t *)caCmdBuf;
    caStaSetIpConfig_t *ipconfig = &setIpConf->cmdsu.ipconfig;
    dutCmdResponse_t *staSetIpResp = &gGenericResp;
    int connected = 0;
    char result[32];

    DPRINT_INFO(WFA_OUT, "\nEntering wfaStaSetIpConfig ... dhcp=%d\n", ipconfig->isDhcp);

    update_ip_config(ipconfig);

    /*
     * report status
     */
    staSetIpResp->status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_STA_SET_IP_CONFIG_RESP_TLV, 4, (BYTE *)staSetIpResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + 4;

    return WFA_SUCCESS;
}

/*
 * wfaStaVerifyIpConnection():
 * The function is to verify if the station has IP connection with an AP by
 * send ICMP/pings to the AP.
 */
int wfaStaVerifyIpConnection(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCommand_t *verip = (dutCommand_t *)caCmdBuf;
    dutCmdResponse_t *verifyIpResp = &gGenericResp;

#ifndef WFA_PING_UDP_ECHO_ONLY
    char strout[64], *pcnt;
    FILE *tmpfile;

    DPRINT_INFO(WFA_OUT, "\nEntering wfaStaVerifyIpConnection ...\n");

    /* set timeout value in case not set */
    if(verip->cmdsu.verifyIp.timeout <= 0)
    {
        verip->cmdsu.verifyIp.timeout = 10;
    }

    /* execute the ping command  and pipe the result to a tmp file */
    sprintf(gCmdStr, "ping %s -c 3 -W %u | grep loss | cut -f3 -d, 1>& /tmp/pingout.txt", verip->cmdsu.verifyIp.dipaddr, verip->cmdsu.verifyIp.timeout);
    sret = system(gCmdStr);

    /* scan/check the output */
    tmpfile = fopen("/tmp/pingout.txt", "r+");
    if(tmpfile == NULL)
    {
        verifyIpResp->status = STATUS_ERROR;
        wfaEncodeTLV(WFA_STA_VERIFY_IP_CONNECTION_RESP_TLV, 4, (BYTE *)verifyIpResp, respBuf);
        *respLen = WFA_TLV_HDR_LEN + 4;

        DPRINT_ERR(WFA_ERR, "file open failed\n");
        return WFA_FAILURE;
    }

    verifyIpResp->status = STATUS_COMPLETE;
    if(fscanf(tmpfile, "%63s", strout) == EOF)
        verifyIpResp->cmdru.connected = 0;
    else
    {
        pcnt = strtok(strout, "%");

        /* if the loss rate is 100%, not able to connect */
        if(atoi(pcnt) == 100)
            verifyIpResp->cmdru.connected = 0;
        else
            verifyIpResp->cmdru.connected = 1;
    }

    fclose(tmpfile);
#else
    int btSockfd;
    struct pollfd fds[2];
    int timeout = 2000;
    char anyBuf[64];
    struct sockaddr_in toAddr;
    int done = 1, cnt = 0, ret, nbytes;

    verifyIpResp->status = STATUS_COMPLETE;
    verifyIpResp->cmdru.connected = 0;

    btSockfd = wfaCreateUDPSock("127.0.0.1", WFA_UDP_ECHO_PORT);

    if(btSockfd == -1)
    {
        verifyIpResp->status = STATUS_ERROR;
        wfaEncodeTLV(WFA_STA_VERIFY_IP_CONNECTION_RESP_TLV, 4, (BYTE *)verifyIpResp, respBuf);
        *respLen = WFA_TLV_HDR_LEN + 4;
        return WFA_FAILURE;;
    }

    toAddr.sin_family = AF_INET;
    toAddr.sin_addr.s_addr = inet_addr(verip->cmdsu.verifyIp.dipaddr);
    toAddr.sin_port = htons(WFA_UDP_ECHO_PORT);

    while(done)
    {
        wfaTrafficSendTo(btSockfd, (char *)anyBuf, 64, (struct sockaddr *)&toAddr);
        cnt++;

        fds[0].fd = btSockfd;
        fds[0].events = POLLIN | POLLOUT;

        ret = poll(fds, 1, timeout);
        switch(ret)
        {
        case 0:
            /* it is time out, count a packet lost*/
            break;
        case -1:
        /* it is an error */
        default:
        {
            switch(fds[0].revents)
            {
            case POLLIN:
            case POLLPRI:
            case POLLOUT:
                nbytes = wfaTrafficRecv(btSockfd, (char *)anyBuf, (struct sockaddr *)&toAddr);
                if(nbytes != 0)
                    verifyIpResp->cmdru.connected = 1;
                done = 0;
                break;
            default:
                /* errors but not care */
                ;
            }
        }
        }
        if(cnt == 3)
        {
            done = 0;
        }
    }

#endif

    wfaEncodeTLV(WFA_STA_VERIFY_IP_CONNECTION_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)verifyIpResp, respBuf);

    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

    return WFA_SUCCESS;
}

/*
 * wfaStaGetMacAddress()
 *    This function is to retrieve the MAC address of a wireless I/F.
 */
int wfaStaGetMacAddress(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    unsigned char addr[6];
    dutCommand_t *getMac = (dutCommand_t *)caCmdBuf;
    dutCmdResponse_t *getmacResp = &gGenericResp;
    char *ifname = getMac->intf;

    DPRINT_INFO(WFA_OUT, "\nEntering wfaStaGetMacAddress ...\n");

    getmacResp->status = get_mac(ifname, addr);
    wSNPRINTF(getmacResp->cmdru.mac, sizeof(getmacResp->cmdru.mac),
            "%02x:%02x:%02x:%02x:%02x:%02x",
            addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

    wfaEncodeTLV(WFA_STA_GET_MAC_ADDRESS_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)getmacResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

    return WFA_SUCCESS;
}

/*
 * wfaStaGetStats():
 * The function is to retrieve the statistics of the I/F's layer 2 txFrames,
 * rxFrames, txMulticast, rxMulticast, fcsErrors/crc, and txRetries.
 * Currently there is not definition how to use these info.
 */
int wfaStaGetStats(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t *statsResp = &gGenericResp;

    /* this is never used, you can skip this call */

    statsResp->status = STATUS_ERROR;
    wfaEncodeTLV(WFA_STA_GET_STATS_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)statsResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);


    return WFA_SUCCESS;
}

int wfaSetEncryption(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    caStaSetEncryption_t *setEncryp = (caStaSetEncryption_t *)caCmdBuf;
    char *ifname = setEncryp->intf;
    dutCmdResponse_t *setEncrypResp = &gGenericResp;
    char buf[200];
    int id, i;

    DPRINT_INFO(WFA_OUT, "\nEntering wfaSetEncryption ...\n");

    // must set ssid before add_network_common
    set_param_str("ssid", setEncryp->ssid);

    id = add_network_common(ifname);
    if (id < 0) {
        setEncrypResp->status = STATUS_ERROR;
        goto done;
    }

    if (set_network(ifname, id, "key_mgmt", "NONE") < 0) {
        setEncrypResp->status = STATUS_ERROR;
        goto done;
    }

    if (setEncryp->encpType == ENCRYPT_WEP) {
        const char *val;

        if (setEncryp->activeKeyIdx < 1 || setEncryp->activeKeyIdx > 4) {
            setEncrypResp->status = STATUS_INVALID;
            goto done;
        }

        wSPRINTF(buf, "%d", setEncryp->activeKeyIdx - 1);
        if (set_network(ifname, id, "wep_tx_keyidx", buf) < 0) {
            setEncrypResp->status = STATUS_ERROR;
            goto done;
        }

        for( i = 0; i < 4; i++) {
            if(setEncryp->keys[i][0] == '\0')
                continue;
            wSPRINTF(buf, "wep_key%d", i);
            if (set_network(ifname, id, buf, setEncryp->keys[i]) < 0) {
                setEncrypResp->status = STATUS_ERROR;
                goto done;
            }
        }
    } else {
        for(i = 0; i < 4; i++) {
            wSPRINTF(buf, "wep_key%d", i);
            set_network_quoted(ifname, id, buf, "");
        }
    }

    setEncrypResp->status = STATUS_COMPLETE;
done:
    wfaEncodeTLV(WFA_STA_SET_ENCRYPTION_RESP_TLV, 4, (BYTE *)setEncrypResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + 4;

    return WFA_SUCCESS;
}

int wfaStaSetSecurity(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCommand_t *cmd = (dutCommand_t *)caCmdBuf;
    char *intf = cmd->intf;
    caStaSetSecurity_t *ssec = &cmd->cmdsu.setsec;

    DPRINT_INFO(WFA_OUT, "\nEntering wfaStaSetSecurity ...intf=%s, type=%d, pmf=%d\n", intf, ssec->type, ssec->pmf);
    if (intf[0] == '\0')
        return WFA_FAILURE;

    set_param_str("intf", intf);
    set_param_val("type", ssec->type);
    set_param_str("ssid", ssec->ssid);
    set_param_str("keyMgmtType", ssec->keyMgmtType);
    set_param_str("encpType", ssec->encpType);
    set_param_val("pmf", ssec->pmf);
    set_param_val("akmSuiteType", ssec->akmSuiteType);
    set_param_str("clientCertificate", ssec->clientCertificate);
    set_param_str("certType", ssec->certType);
    set_param_str("ecGroupID", ssec->ecGroupID);
    set_param_str("groupCipher", ssec->groupCipher);
    set_param_str("groupMgntCipher", ssec->groupMgntCipher);
    set_param_str("innerEAP", ssec->innerEAP);
    set_param_str("invalidSAEElement", ssec->invalidSAEElement);
    set_param_str("networkMode", ssec->networkMode);
    set_param_val("owe", ssec->owe);
    set_param_str("pacFile", ssec->pacFile);
    set_param_str("pairwiseCipher", ssec->pairwiseCipher);
    set_param_str("passphrase", ssec->passphrase);
    set_param_str("password", ssec->passphrase);
    set_param_val("pmksaCaching", ssec->pmksaCaching);
    set_param_val("profile", ssec->profile);
    set_param_str("prog", ssec->prog);
    set_param_str("trustedRootCA", ssec->trustedRootCA);
    set_param_str("tlsCipher", ssec->tlsCipher);
    set_param_str("username", ssec->username);
    set_param_val("peapVersion", ssec->peapVersion);

    if (ssec->type == SEC_TYPE_PSK ||
        ssec->type == SEC_TYPE_PSK_SAE ||
        ssec->type == SEC_TYPE_SAE)
        return wfaStaSetPSK(0, NULL, respLen, respBuf);
    if (ssec->type == SEC_TYPE_EAPTLS)
        return wfaStaSetEapTLS(0, NULL, respLen, respBuf);
    if (ssec->type == SEC_TYPE_EAPTTLS)
        return wfaStaSetEapTTLS(0, NULL, respLen, respBuf);
    if (ssec->type == SEC_TYPE_EAPPEAP)
        return wfaStaSetPEAP(0, NULL, respLen, respBuf);
    if (ssec->type == SEC_TYPE_EAPSIM)
        return wfaStaSetEapSIM(0, NULL, respLen, respBuf);
    if (ssec->type == SEC_TYPE_EAPAKA)
        return wfaStaSetEapAKA(0, NULL, respLen, respBuf);
    if (ssec->type == SEC_TYPE_OPEN)
        return sta_set_open(respLen, respBuf);
    if (ssec->type == SEC_TYPE_OWE)
        return sta_set_owe(respLen, respBuf);
    return WFA_FAILURE;
}

/*
 * wfaStaSetEapTLS():
 *   This is to set
 *   1. ssid
 *   2. encrypType - tkip or aes-ccmp
 *   3. keyManagementType - wpa or wpa2
 *   4. trustedRootCA
 *   5. clientCertificate
 */
int wfaStaSetEapTLS(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    caStaSetEapTLS_t *setTLS = (caStaSetEapTLS_t *)caCmdBuf;
    char *intf = get_param_str("intf");
    dutCmdResponse_t *setEapTlsResp = &gGenericResp;
    int id;

    if (setTLS) {
        DPRINT_INFO(WFA_OUT, "\nEntering wfaStaSetEapTLS ...\n");
        set_param_str("ssid", setTLS->ssid);
        set_param_str("username", setTLS->username);
        set_param_str("keyMgmtType", setTLS->keyMgmtType);
        set_param_str("encptype", setTLS->encrptype);
        set_param_str("trustedRootCA", setTLS->trustedRootCA);
        set_param_str("clientCertificate", setTLS->clientCertificate);
        set_param_val("pmf", setTLS->pmf);
        set_param_str("micAlg", setTLS->micAlg);
        intf = setTLS->intf;
    }

    id = add_network_common(intf);
    if (id < 0) {
        setEapTlsResp->status = STATUS_ERROR;
        goto done;
    }

    if (set_eap_common(intf, id) != STATUS_COMPLETE) {
        setEapTlsResp->status = STATUS_ERROR;
        goto done;
    }

    if (set_network(intf, id, "eap", "TLS") < 0) {
        setEapTlsResp->status = STATUS_ERROR;
        goto done;
    }

    if (!get_param_str("username") &&
        set_network_quoted(intf, id, "identity", "wifi-user@wifilabs.local") < 0) {
            setEapTlsResp->status = STATUS_ERROR;
            goto done;
    }

    if (set_network_quoted(intf, id, "private_key_passwd", "wifi") < 0) {
        setEapTlsResp->status = STATUS_ERROR;
        goto done;
    }

    setEapTlsResp->status = STATUS_COMPLETE;
done:
    wfaEncodeTLV(WFA_STA_SET_EAPTLS_RESP_TLV, 4, (BYTE *)setEapTlsResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + 4;

    return WFA_SUCCESS;
}

/*
 * The function is to set
 *   1. ssid
 *   2. passPhrase
 *   3. keyMangementType - wpa/wpa2
 *   4. encrypType - tkip or aes-ccmp
 */
int wfaStaSetPSK(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    caStaSetPSK_t *setPSK = (caStaSetPSK_t *)caCmdBuf;
    char *intf = get_param_str("intf");
    dutCmdResponse_t *setPskResp = &gGenericResp;
    const char *val, *alg;
    const int *type, *pmf;
    char buf[256];
    int id;

    if (setPSK) {
        DPRINT_INFO(WFA_OUT, "\nEntering wfaStaSetPSK ...ssid=%s,psk=%s\n", setPSK->ssid, setPSK->passphrase);
        set_param_str("ssid", setPSK->ssid);
        set_param_str("passphrase", setPSK->passphrase);
        set_param_str("keyMgmtType", setPSK->keyMgmtType);
        set_param_str("micAlg", setPSK->micAlg);
        set_param_str("prog", setPSK->prog);
        set_param_val("prefer", setPSK->prefer);
        set_param_str("encpType", encpType2str(setPSK->encpType));
        set_param_val("pmf", setPSK->pmf);
        intf = setPSK->intf;
    }

    id = add_network_common(intf);
    if (id < 0)
        return STATUS_ERROR;

    if (set_wpa_common(intf, id) != STATUS_COMPLETE)
        return STATUS_ERROR;

    buf[0] = '\0';

    type = get_param_val("type");
    alg = get_param_str("micAlg");
    pmf = get_param_val("pmf");
    val = get_param_str("keyMgmtType");
    // key_mgmt
    if (type && *type == SEC_TYPE_SAE) {
        if (str_same(val, "wpa2-ft"))
            wSPRINTF(buf, "FT-SAE");
        else
            wSPRINTF(buf, "SAE");
        if (wpa_command(intf, "SET sae_groups ") < 0) {
            setPskResp->status = STATUS_ERROR;
            goto done;
        }
        if (!pmf) {
            gDut.sta_pmf = WFA_REQUIRED;
            if (set_network(intf, id, "ieee80211w", "2") < 0) {
                setPskResp->status = STATUS_ERROR;
                goto done;
            }
        }
    } else if (type && *type == SEC_TYPE_PSK_SAE) {
        if (str_same(val, "wpa2-ft"))
            wSPRINTF(buf, "FT-SAE FT-PSK");
        else
            wSPRINTF(buf, "SAE WPA-PSK");
        if (wpa_command(intf, "SET sae_groups ") < 0) {
            setPskResp->status = STATUS_ERROR;
            goto done;
        }
        if (!pmf) {
            gDut.sta_pmf = WFA_OPTIONAL;
            if (set_network(intf, id, "ieee80211w", "1") < 0) {
                setPskResp->status = STATUS_ERROR;
                goto done;
            }
        }
    } else if (str_same(alg, "SHA-256")) {
        wSPRINTF(buf, "WPA-PSK-SHA256");
    } else if (str_same(alg, "SHA-1")) {
        wSPRINTF(buf, "WPA-PSK");
    } else if (str_same(val, "wpa2-ft")) {
        wSPRINTF(buf, "FT-PSK");
    } else if (str_same(val, "wpa2-sha256") ||
        gDut.sta_pmf == WFA_REQUIRED ||
        gDut.sta_pmf == WFA_OPTIONAL) {
        wSPRINTF(buf, "WPA-PSK WPA-PSK-SHA256");
    } else {
        wSPRINTF(buf, "WPA-PSK");
    }

    if (set_network(intf, id, "key_mgmt", buf) < 0) {
        setPskResp->status = STATUS_ERROR;
        goto done;
    }

    // passphrase
    val = get_param_str("passPhrase");
    if (type && *type == SEC_TYPE_SAE) {
        if (set_network_quoted(intf, id, "sae_password", val) < 0) {
            setPskResp->status = STATUS_ERROR;
            goto done;
        }
    } else {
        if (set_network_quoted(intf, id, "psk", val) < 0) {
            setPskResp->status = STATUS_ERROR;
            goto done;
        }
    }

    // sae_groups
    val = get_param_str("ECGroupID");
    if (val) {
        wSPRINTF(buf, "SET sae_groups %s", val);
        if (wpa_command(intf, buf) < 0) {
            setPskResp->status = STATUS_ERROR;
            goto done;
        }
    }

    val = get_param_str( "InvalidSAEElement");
    if (val) {
        wSPRINTF(buf, "SET sae_commit_override %s", val);
        if (wpa_command(intf, buf) < 0) {
            setPskResp->status = STATUS_ERROR;
            goto done;
        }
    }

    setPskResp->status = STATUS_COMPLETE;
done:
    wfaEncodeTLV(WFA_STA_SET_PSK_RESP_TLV, 4, (BYTE *)setPskResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + 4;

    return WFA_SUCCESS;
}

/*
 * wfaStaGetInfo():
 * Get vendor specific information in name/value pair by a wireless I/F.
 */
int wfaStaGetInfo(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t infoResp;
    dutCommand_t *getInfo = (dutCommand_t *)caCmdBuf;

    DPRINT_INFO(WFA_OUT, "\nEntering wfaStaGetInfo ...\n");

    /*
     * Normally this is called to retrieve the vendor information
     * from a interface, no implement yet
     */
    wSPRINTF(infoResp.cmdru.info, "interface,%s,vendor,Mediatek Inc.,cardtype,802.11a/b/g/n/ac", getInfo->intf);

    infoResp.status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_STA_GET_INFO_RESP_TLV, sizeof(infoResp), (BYTE *)&infoResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(infoResp);

    return WFA_SUCCESS;
}

/*
 * wfaStaSetEapTTLS():
 *   This is to set
 *   1. ssid
 *   2. username
 *   3. passwd
 *   4. encrypType - tkip or aes-ccmp
 *   5. keyManagementType - wpa or wpa2
 *   6. trustedRootCA
 */
int wfaStaSetEapTTLS(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    caStaSetEapTTLS_t *setTTLS = (caStaSetEapTTLS_t *)caCmdBuf;
    char *intf = get_param_str("intf");
    dutCmdResponse_t *setEapTtlsResp = &gGenericResp;
    int id;

    if (setTTLS) {
        DPRINT_INFO(WFA_OUT, "\nEntering wfaStaSetEapTTLS ...\n");
        set_param_str("ssid", setTTLS->ssid);
        set_param_str("username", setTTLS->username);
        set_param_str("password", setTTLS->passwd);
        set_param_str("keyMgmtType", setTTLS->keyMgmtType);
        set_param_str("encptype", setTTLS->encrptype);
        set_param_str("trustedRootCA", setTTLS->trustedRootCA);
        set_param_str("clientCertificate", setTTLS->clientCertificate);
        set_param_val("pmf", setTTLS->pmf);
        set_param_str("micAlg", setTTLS->micAlg);
        set_param_str("prog", setTTLS->prog);
        set_param_val("prefer", setTTLS->prefer);
        intf = setTTLS->intf;
    }

    id = add_network_common(intf);
    if (id < 0) {
        setEapTtlsResp->status = STATUS_ERROR;
        goto done;
    }

    if (set_eap_common(intf, id) != STATUS_COMPLETE) {
        setEapTtlsResp->status = STATUS_ERROR;
        goto done;
    }

    if (set_network(intf, id, "eap", "TTLS") < 0) {
        setEapTtlsResp->status = STATUS_ERROR;
        goto done;
    }

    if (set_network_quoted(intf, id, "phase2", "auth=MSCHAPV2") < 0) {
        setEapTtlsResp->status = STATUS_ERROR;
        goto done;
    }

    setEapTtlsResp->status = STATUS_COMPLETE;
done:
    wfaEncodeTLV(WFA_STA_SET_EAPTTLS_RESP_TLV, 4, (BYTE *)setEapTtlsResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + 4;

    return WFA_SUCCESS;
}

/*
 * wfaStaSetEapSIM():
 *   This is to set
 *   1. ssid
 *   2. user name
 *   3. passwd
 *   4. encrypType - tkip or aes-ccmp
 *   5. keyMangementType - wpa or wpa2
 */
int wfaStaSetEapSIM(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    caStaSetEapSIM_t *setSIM = (caStaSetEapSIM_t *)caCmdBuf;
    char *intf = get_param_str("intf");
    dutCmdResponse_t *setEapSimResp = &gGenericResp;
    int id;

    if (setSIM) {
        DPRINT_INFO(WFA_OUT, "\nEntering wfaStaSetEapSIM ...\n");

        set_param_str("ssid", setSIM->ssid);
        set_param_str("username", setSIM->username);
        set_param_str("password", setSIM->passwd);
        set_param_str("keyMgmtType", setSIM->keyMgmtType);
        set_param_str("encptype", setSIM->encrptype);
        set_param_val("pmf", setSIM->pmf);
        intf = setSIM->intf;
    }

    id = add_network_common(intf);
    if (id < 0) {
        setEapSimResp->status = STATUS_ERROR;
        goto done;
    }

    if (set_eap_common(intf, id) != STATUS_COMPLETE) {
        setEapSimResp->status = STATUS_ERROR;
        goto done;
    }

    if (set_network(intf, id, "eap", "SIM") < 0) {
        setEapSimResp->status = STATUS_ERROR;
        goto done;

    }

    setEapSimResp->status = STATUS_COMPLETE;
done:
    wfaEncodeTLV(WFA_STA_SET_EAPSIM_RESP_TLV, 4, (BYTE *)setEapSimResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + 4;

    return WFA_SUCCESS;
}

/*
 * wfaStaSetPEAP()
 *   This is to set
 *   1. ssid
 *   2. user name
 *   3. passwd
 *   4. encryType - tkip or aes-ccmp
 *   5. keyMgmtType - wpa or wpa2
 *   6. trustedRootCA
 *   7. innerEAP
 *   8. peapVersion
 */
int wfaStaSetPEAP(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    caStaSetEapPEAP_t *setPEAP = (caStaSetEapPEAP_t *)caCmdBuf;
    char *intf = get_param_str("intf");
    dutCmdResponse_t *setPeapResp = &gGenericResp;
    int id;
    char buf[256];
    const char *val;

    if (setPEAP) {
        DPRINT_INFO(WFA_OUT, "\nEntering wfaStaSetPEAP ...\n");
        set_param_str("ssid", setPEAP->ssid);
        set_param_str("username", setPEAP->username);
        set_param_str("password", setPEAP->passwd);
        set_param_str("keyMgmtType", setPEAP->keyMgmtType);
        set_param_str("encptype", setPEAP->encrptype);
        set_param_str("trustedRootCA", setPEAP->trustedRootCA);
        set_param_str("innerEAP", setPEAP->innerEAP);
        set_param_val("peapVersion", setPEAP->peapVersion);
        set_param_val("pmf", setPEAP->pmf);
        intf = setPEAP->intf;
    }

    id = add_network_common(intf);
    if (id < 0) {
        setPeapResp->status = STATUS_ERROR;
        goto done;
    }

    if (set_eap_common(intf, id) != STATUS_COMPLETE) {
        setPeapResp->status = STATUS_ERROR;
        goto done;
    }

    if (set_network(intf, id, "eap", "PEAP") < 0) {
        setPeapResp->status = STATUS_ERROR;
        goto done;
    }

    val = get_param_str("innerEAP");
    if (val) {
        if (str_same(val, "MSCHAPv2")) {
            wSPRINTF(buf, "auth=MSCHAPV2");
        } else if (str_same(val, "GTC")) {
            wSPRINTF(buf, "auth=GTC");
        } else {
            setPeapResp->status = STATUS_INVALID;
            goto done;
        }
        if (set_network_quoted(intf, id, "phase2", buf) < 0) {
            setPeapResp->status = STATUS_ERROR;
            goto done;
        }
    }

    val = get_param_val("peapVersion");
    if (val) {
        if (*val < 0 || *val > 1) {
            setPeapResp->status = STATUS_INVALID;
            goto done;
        }
        wSPRINTF(buf, "peapver=%d", *val);
        if (set_network_quoted(intf, id, "phase1", buf) < 0) {
            setPeapResp->status = STATUS_ERROR;
            goto done;
        }
    }

    setPeapResp->status = STATUS_COMPLETE;
done:
    wfaEncodeTLV(WFA_STA_SET_PEAP_RESP_TLV, 4, (BYTE *)setPeapResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + 4;

    return WFA_SUCCESS;
}

/*
 * wfaStaSetUAPSD()
 *    This is to set
 *    1. acBE
 *    2. acBK
 *    3. acVI
 *    4. acVO
 */
int wfaStaSetUAPSD(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t *setUAPSDResp = &gGenericResp;
    caStaSetUAPSD_t *setuapsd = (caStaSetUAPSD_t *)caCmdBuf;
    char buf[256];
    unsigned int uapsd = 0;

    if (setuapsd->acBE)
            uapsd |= 1;
    if (setuapsd->acBK)
        uapsd |= 1<<1;
    if (setuapsd->acVI)
        uapsd |= 1<<2;
    if (setuapsd->acVO)
        uapsd |= 1<<3;
    if (wSTRNCMP(setuapsd->intf, "p2p", 3) == 0) {
        uapsd |= 0x10000;
    }

    shell(IWPRIV" %s set_sw_ctrl 0x10010003 0x%x", setuapsd->intf, uapsd);

    wSPRINTF(buf, "set uapsd %d,%d,%d,%d,%d", setuapsd->acBE, setuapsd->acBK,
        setuapsd->acVI, setuapsd->acVO, setuapsd->maxSPLength);
    wpa_command(setuapsd->intf, buf);

    setUAPSDResp->status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_STA_SET_UAPSD_RESP_TLV, 4, (BYTE *)setUAPSDResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + 4;
    return WFA_SUCCESS;
}

int wfaDeviceGetInfo(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCommand_t *dutCmd = (dutCommand_t *)caCmdBuf;
    caDevInfo_t *devInfo = &dutCmd->cmdsu.dev;
    dutCmdResponse_t *infoResp = &gGenericResp;
    caDeviceGetInfoResp_t dinfo = {"MediaTek Inc.", "MT66xx", MTK_SYSTEM_VER};

    DPRINT_INFO(WFA_OUT, "\nEntering wfaDeviceGetInfo ... mode=%d\n", devInfo->fw);

    if(devInfo->fw == 0) {
        if (shell("getprop ro.build.flavor") == WFA_SUCCESS)
            wSTRNCPY(dinfo.model, gDut.shellResult, sizeof(dinfo.model) - 1);
        if (shell("getprop ro.mediatek.version.release") == WFA_SUCCESS)
            wSTRNCPY(dinfo.version, gDut.shellResult, sizeof(dinfo.version) - 1);
    } else {
        int size = sizeof(infoResp->cmdru.devInfo.firmware) - 1;
        if (shell("getprop persist.vendor.connsys.patch.version") == WFA_SUCCESS)
            wSTRNCPY(dinfo.firmware, gDut.shellResult, size);
        else
            wSTRNCPY(dinfo.firmware, "NOVERSION", size);

        /* TODO: should we parse fw version from iwpriv ?
        wSTRNCPY(infoResp->cmdru.devInfo.firmware, "NOVERSION", size);
        // get firmware version
        if (shell(IWPRIV" wlan0 driver ver") == WFA_SUCCESS) {
            char *outer = NULL, *outer_save, *inner = NULL, *inner_save = NULL;

            outer = strtok_r(gDut.shellResult, "\n\r", &outer_save);
            while (outer) {
                inner = strtok_r(outer, " ", &inner_save);
                if (inner && !wSTRNCMP(inner, "Tailer", 6)) {
                    wSTRNCPY(infoResp->cmdru.devInfo.firmware, inner_save, size);
                    break;
                }
                outer = strtok_r(NULL, "\n\r", &outer_save);
            }
        }
        */
    }

    wMEMCPY(&infoResp->cmdru.devInfo, &dinfo, sizeof(caDeviceGetInfoResp_t));

    /*
     * report status
     */
    infoResp->status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_DEVICE_GET_INFO_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)infoResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

    return WFA_SUCCESS;
}

/*
 * This funciton is to retrieve a list of interfaces and return
 * the list back to Agent control.
 * ********************************************************************
 * Note: We intend to make this WLAN interface name as a hardcode name.
 * Therefore, for a particular device, you should know and change the name
 * for that device while doing porting. The MACRO "WFA_STAUT_IF" is defined in
 * the file "inc/wfa_ca.h". If the device OS is not linux-like, this most
 * likely is hardcoded just for CAPI command responses.
 * *******************************************************************
 *
 */
int wfaDeviceListIF(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t *infoResp = &gGenericResp;
    dutCommand_t *ifList = (dutCommand_t *)caCmdBuf;
    caDeviceListIFResp_t *ifListResp = &infoResp->cmdru.ifList;

    DPRINT_INFO(WFA_OUT, "\nEntering wfaDeviceListIF ...\n");

    switch(ifList->cmdsu.iftype)
    {
    case IF_80211:
        infoResp->status = STATUS_COMPLETE;
        ifListResp->iftype = IF_80211;
        strcpy(ifListResp->ifs[0], get_main_intf());
        strcpy(ifListResp->ifs[1], "NULL");
        strcpy(ifListResp->ifs[2], "NULL");
        break;
    case IF_ETH:
        infoResp->status = STATUS_COMPLETE;
        ifListResp->iftype = IF_ETH;
        strcpy(ifListResp->ifs[0], "eth0");
        strcpy(ifListResp->ifs[1], "NULL");
        strcpy(ifListResp->ifs[2], "NULL");
        break;
    default:
    {
        infoResp->status = STATUS_ERROR;
        wfaEncodeTLV(WFA_DEVICE_LIST_IF_RESP_TLV, 4, (BYTE *)infoResp, respBuf);
        *respLen = WFA_TLV_HDR_LEN + 4;

        return WFA_SUCCESS;
    }
    }

    wfaEncodeTLV(WFA_DEVICE_LIST_IF_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)infoResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

    return WFA_SUCCESS;
}

int wfaStaDebugSet(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t *debugResp = &gGenericResp;
    dutCommand_t *debugSet = (dutCommand_t *)caCmdBuf;

    DPRINT_INFO(WFA_OUT, "\nEntering wfaStaDebugSet ...\n");

    if(debugSet->cmdsu.dbg.state == 1) /* enable */
        wfa_defined_debug |= debugSet->cmdsu.dbg.level;
    else
        wfa_defined_debug = (~debugSet->cmdsu.dbg.level & wfa_defined_debug);

    debugResp->status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_STA_GET_INFO_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)debugResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);


    return WFA_SUCCESS;
}


/*
 *   wfaStaGetBSSID():
 *     This function is to retrieve BSSID of a specific wireless I/F.
 */
int wfaStaGetBSSID(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCommand_t *dutCmd = (dutCommand_t *)caCmdBuf;
    dutCmdResponse_t *bssidResp = &gGenericResp;
    char bssid[20];
    int ret;

    DPRINT_INFO(WFA_OUT, "\nEntering wfaStaGetBSSID ...\n");

    ret = get_wpa_status(dutCmd->intf, "bssid", bssid, sizeof(bssid));
    if (ret >= 0) {
        DPRINT_INFO(WFA_OUT, "get_wpa_status bssid=%s\n", bssid);
        wSTRNCPY(bssidResp->cmdru.bssid, bssid, WFA_MAC_ADDR_STR_LEN);
    } else {
        DPRINT_INFO(WFA_OUT, "get_wpa_status field=\"bssid\" ret=%d\n", ret);
        wSTRNCPY(bssidResp->cmdru.bssid, "00:00:00:00:00:00", WFA_MAC_ADDR_STR_LEN);
    }

    bssidResp->status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_STA_GET_BSSID_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)bssidResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

    return WFA_SUCCESS;
}

/*
 * wfaStaSetIBSS()
 *    This is to set
 *    1. ssid
 *    2. channel
 *    3. encrypType - none or wep
 *    optional
 *    4. key1
 *    5. key2
 *    6. key3
 *    7. key4
 *    8. activeIndex - 1, 2, 3, or 4
 */
int wfaStaSetIBSS(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    caStaSetIBSS_t *setIBSS = (caStaSetIBSS_t *)caCmdBuf;
    dutCmdResponse_t *setIbssResp = &gGenericResp;
    int i;

    /*
     * disable the network first
     */
    sprintf(gCmdStr, "wpa_cli -i %s disable_network 0", setIBSS->intf);
    sret = system(gCmdStr);

    /*
     * set SSID
     */
    sprintf(gCmdStr, "wpa_cli -i %s set_network 0 ssid '\"%s\"'", setIBSS->intf, setIBSS->ssid);
    sret = system(gCmdStr);

    /*
     * Set channel for IBSS
     */
    sprintf(gCmdStr, "iwconfig %s channel %i", setIBSS->intf, setIBSS->channel);
    sret = system(gCmdStr);

    /*
     * Tell the supplicant for IBSS mode (1)
     */
    sprintf(gCmdStr, "wpa_cli -i %s set_network 0 mode 1", setIBSS->intf);
    sret = system(gCmdStr);

    /*
     * set Key management to NONE (NO WPA) for plaintext or WEP
     */
    sprintf(gCmdStr, "wpa_cli -i %s set_network 0 key_mgmt NONE", setIBSS->intf);
    sret = system(gCmdStr);

    if(setIBSS->encpType == 1)
    {
        for(i=0; i<4; i++)
        {
            if(strlen(setIBSS->keys[i]) ==5 || strlen(setIBSS->keys[i]) == 13)
            {
                sprintf(gCmdStr, "wpa_cli -i %s set_network 0 wep_key%i \"%s\"",
                        setIBSS->intf, i, setIBSS->keys[i]);
                sret = system(gCmdStr);
            }
        }

        i = setIBSS->activeKeyIdx;
        if(strlen(setIBSS->keys[i]) ==5 || strlen(setIBSS->keys[i]) == 13)
        {
            sprintf(gCmdStr, "wpa_cli -i %s set_network 0 wep_tx_keyidx %i",
                    setIBSS->intf, setIBSS->activeKeyIdx);
            sret = system(gCmdStr);
        }
    }

    sprintf(gCmdStr, "wpa_cli -i %s enable_network 0", setIBSS->intf);
    sret = system(gCmdStr);

    setIbssResp->status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_STA_SET_IBSS_RESP_TLV, 4, (BYTE *)setIbssResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + 4;

    return WFA_SUCCESS;
}

/*
 *  wfaSetMode():
 *  The function is to set the wireless interface with a given mode (possible
 *  adhoc)
 *  Input parameters:
 *    1. I/F
 *    2. ssid
 *    3. mode adhoc or managed
 *    4. encType
 *    5. channel
 *    6. key(s)
 *    7. active  key
 */
int wfaStaSetMode(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    caStaSetMode_t *setmode = (caStaSetMode_t *)caCmdBuf;
    dutCmdResponse_t *SetModeResp = &gGenericResp;
    int i;

    /*
     * bring down the interface
     */
    sprintf(gCmdStr, "ifconfig %s down",setmode->intf);
    sret = system(gCmdStr);

    /*
     * distroy the interface
     */
    sprintf(gCmdStr, "wlanconfig %s destroy",setmode->intf);
    sret = system(gCmdStr);


    /*
     * re-create the interface with the given mode
     */
    if(setmode->mode == 1)
        sprintf(gCmdStr, "wlanconfig %s create wlandev wifi0 wlanmode adhoc",setmode->intf);
    else
        sprintf(gCmdStr, "wlanconfig %s create wlandev wifi0 wlanmode managed",setmode->intf);

    sret = system(gCmdStr);
    if(setmode->encpType == ENCRYPT_WEP)
    {
        int j = setmode->activeKeyIdx;
        for(i=0; i<4; i++)
        {
            if(setmode->keys[i][0] != '\0')
            {
                sprintf(gCmdStr, "iwconfig  %s key  s:%s",
                        setmode->intf, setmode->keys[i]);
                sret = system(gCmdStr);
            }
            /* set active key */
            if(setmode->keys[j][0] != '\0')
                sprintf(gCmdStr, "iwconfig  %s key  s:%s",
                        setmode->intf, setmode->keys[j]);
            sret = system(gCmdStr);
        }

    }
    /*
     * Set channel for IBSS
     */
    if(setmode->channel)
    {
        sprintf(gCmdStr, "iwconfig %s channel %i", setmode->intf, setmode->channel);
        sret = system(gCmdStr);
    }


    /*
     * set SSID
     */
    sprintf(gCmdStr, "iwconfig %s essid %s", setmode->intf, setmode->ssid);
    sret = system(gCmdStr);

    /*
     * bring up the interface
     */
    sprintf(gCmdStr, "ifconfig %s up",setmode->intf);
    sret = system(gCmdStr);

    SetModeResp->status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_STA_SET_MODE_RESP_TLV, 4, (BYTE *)SetModeResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + 4;

    return WFA_SUCCESS;
}

int wfaStaSetPwrSave(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    caStaSetPwrSave_t *setps = (caStaSetPwrSave_t *)caCmdBuf;
    dutCmdResponse_t *SetPSResp = &gGenericResp;

    if (str_same(setps->mode, "off"))
        shell(IWPRIV" %s set_power_mode 0", setps->intf);
    else
        shell(IWPRIV" %s set_power_mode 1", setps->intf);

    SetPSResp->status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_STA_SET_PWRSAVE_RESP_TLV, 4, (BYTE *)SetPSResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + 4;

    return WFA_SUCCESS;
}

int wfaStaUpload(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    caStaUpload_t *upload = &((dutCommand_t *)caCmdBuf)->cmdsu.upload;
    dutCmdResponse_t *upLoadResp = &gGenericResp;
    caStaUploadResp_t *upld = &upLoadResp->cmdru.uld;

    if(upload->type == WFA_UPLOAD_VHSO_RPT)
    {
        int rbytes;
        /*
         * if asked for the first packet, always to open the file
         */
        if(upload->next == 1)
        {
            if(e2efp != NULL)
            {
                fclose(e2efp);
                e2efp = NULL;
            }

            e2efp = fopen(e2eResults, "r");
        }

        if(e2efp == NULL)
        {
            upLoadResp->status = STATUS_ERROR;
            wfaEncodeTLV(WFA_STA_UPLOAD_RESP_TLV, 4, (BYTE *)upLoadResp, respBuf);
            *respLen = WFA_TLV_HDR_LEN + 4;
            return WFA_FAILURE;
        }

        rbytes = fread(upld->bytes, 1, 256, e2efp);

        if(rbytes < 256)
        {
            /*
             * this means no more bytes after this read
             */
            upld->seqnum = 0;
            fclose(e2efp);
            e2efp=NULL;
        }
        else
        {
            upld->seqnum = upload->next;
        }

        upld->nbytes = rbytes;

        upLoadResp->status = STATUS_COMPLETE;
        wfaEncodeTLV(WFA_STA_UPLOAD_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)upLoadResp, respBuf);
        *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);
    }
    else
    {
        upLoadResp->status = STATUS_ERROR;
        wfaEncodeTLV(WFA_STA_UPLOAD_RESP_TLV, 4, (BYTE *)upLoadResp, respBuf);
        *respLen = WFA_TLV_HDR_LEN + 4;
    }

    return WFA_SUCCESS;
}
/*
 * wfaStaSetWMM()
 *  TO be ported on a specific plaform for the DUT
 *  This is to set the WMM related parameters at the DUT.
 *  Currently the function is used for GROUPS WMM-AC and WMM general configuration for setting RTS Threshhold, Fragmentation threshold and wmm (ON/OFF)
 *  It is expected that this function will set all the WMM related parametrs for a particular GROUP .
 */
int wfaStaSetWMM(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
#ifdef WFA_WMM_AC
    caStaSetWMM_t *setwmm = (caStaSetWMM_t *)caCmdBuf;
    char *ifname = setwmm->intf;
    dutCmdResponse_t *setwmmResp = &gGenericResp;
    char buf[1024];

    DPRINT_INFO(WFA_OUT, "\nEntering wfaStaSetWMM ...\n");

    switch(setwmm->group)
    {
    case GROUP_WMMAC:
        if (setwmm->send_trig)
        {
            int Sockfd, r;
            struct sockaddr_in psToAddr;
            unsigned int TxMsg[512];

            Sockfd = wfaCreateUDPSock(setwmm->dipaddr, 12346);
            memset(&psToAddr, 0, sizeof(psToAddr));
            psToAddr.sin_family = AF_INET;
            psToAddr.sin_addr.s_addr = inet_addr(setwmm->dipaddr);
            psToAddr.sin_port = htons(12346);


            switch (setwmm->trig_ac)
            {
            case WMMAC_AC_VO:
                wfaTGSetPrio(Sockfd, 7);
                create_apts_msg(APTS_CK_VO, TxMsg, 0);
                printf("\r\nSending AC_VO trigger packet\n");
                break;

            case WMMAC_AC_VI:
                wfaTGSetPrio(Sockfd, 5);
                create_apts_msg(APTS_CK_VI, TxMsg, 0);
                printf("\r\nSending AC_VI trigger packet\n");
                break;

            case WMMAC_AC_BK:
                wfaTGSetPrio(Sockfd, 2);
                create_apts_msg(APTS_CK_BK, TxMsg, 0);
                printf("\r\nSending AC_BK trigger packet\n");
                break;

            default:
            case WMMAC_AC_BE:
                wfaTGSetPrio(Sockfd, 0);
                create_apts_msg(APTS_CK_BE, TxMsg, 0);
                printf("\r\nSending AC_BE trigger packet\n");
                break;
            }

            r = sendto(Sockfd, TxMsg, 256, 0, (struct sockaddr *)&psToAddr,
                   sizeof(struct sockaddr));
            if(r < 0)
                DPRINT_INFO(WFA_OUT, "WMMAC sendto error %d\n", r);
            close(Sockfd);
            usleep(1000000);
        }
        else if (setwmm->action == WMMAC_ADDTS)
        {
            printf("ADDTS AC PARAMS: dialog id: %d, TID: %d, "
                   "DIRECTION: %d, PSB: %d, UP: %d, INFOACK: %d BURST SIZE DEF: %d"
                   "Fixed %d, MSDU Size: %d, Max MSDU Size %d, "
                   "MIN SERVICE INTERVAL: %d, MAX SERVICE INTERVAL: %d, "
                   "INACTIVITY: %d, SUSPENSION %d, SERVICE START TIME: %d, "
                   "MIN DATARATE: %d, MEAN DATA RATE: %d, PEAK DATA RATE: %d, "
                   "BURSTSIZE or MSDU Aggreg: %d, DELAY BOUND: %d, PHYRATE: %d, SPLUSBW: %f, "
                   "MEDIUM TIME: %d, ACCESSCAT: %d\n",
                   setwmm->actions.addts.dialog_token,
                   setwmm->actions.addts.tspec.tsinfo.TID,
                   setwmm->actions.addts.tspec.tsinfo.direction,
                   setwmm->actions.addts.tspec.tsinfo.PSB,
                   setwmm->actions.addts.tspec.tsinfo.UP,
                   setwmm->actions.addts.tspec.tsinfo.infoAck,
                   setwmm->actions.addts.tspec.tsinfo.bstSzDef,
                   setwmm->actions.addts.tspec.Fixed,
                   setwmm->actions.addts.tspec.size,
                   setwmm->actions.addts.tspec.maxsize,
                   setwmm->actions.addts.tspec.min_srvc,
                   setwmm->actions.addts.tspec.max_srvc,
                   setwmm->actions.addts.tspec.inactivity,
                   setwmm->actions.addts.tspec.suspension,
                   setwmm->actions.addts.tspec.srvc_strt_tim,
                   setwmm->actions.addts.tspec.mindatarate,
                   setwmm->actions.addts.tspec.meandatarate,
                   setwmm->actions.addts.tspec.peakdatarate,
                   setwmm->actions.addts.tspec.burstsize,
                   setwmm->actions.addts.tspec.delaybound,
                   setwmm->actions.addts.tspec.PHYrate,
                   setwmm->actions.addts.tspec.sba,
                   setwmm->actions.addts.tspec.medium_time,
                   setwmm->actions.addts.accesscat);

            wmmtspec_t *addts = &setwmm->actions.addts.tspec;

            //tspec should be set here.
            if (setwmm->actions.addts.psb_flag) {
                wSPRINTF(buf, "addts token %d,"
                    "tid %d,dir %d,psb %d,up %d,fixed %d,size %d,maxsize %d,maxsrvint %d,"
                    "minsrvint %d,inact %d,suspension %d,srvstarttime %d,minrate %d,meanrate %d,"
                    "peakrate %d,burst %d,delaybound %d,phyrate %d,sba %f,mediumtime %d",
                    setwmm->actions.addts.dialog_token, addts->tsinfo.TID, addts->tsinfo.direction,
                    addts->tsinfo.PSB, addts->tsinfo.UP, addts->Fixed, addts->size, addts->maxsize,
                    addts->max_srvc, addts->min_srvc, addts->inactivity, addts->suspension,
                    addts->srvc_strt_tim, addts->mindatarate, addts->meandatarate, addts->peakdatarate,
                    addts->burstsize, addts->delaybound, addts->PHYrate, addts->sba, addts->medium_time);
            } else {
                wSPRINTF(buf, "addts token %d,"
                    "tid %d,dir %d,up %d,fixed %d,size %d,maxsize %d,maxsrvint %d,"
                    "minsrvint %d,inact %d,suspension %d,srvstarttime %d,minrate %d,meanrate %d,"
                    "peakrate %d,burst %d,delaybound %d,phyrate %d,sba %f,mediumtime %d",
                    setwmm->actions.addts.dialog_token, addts->tsinfo.TID, addts->tsinfo.direction,
                    addts->tsinfo.UP, addts->Fixed, addts->size, addts->maxsize,
                    addts->max_srvc, addts->min_srvc, addts->inactivity, addts->suspension,
                    addts->srvc_strt_tim, addts->mindatarate, addts->meandatarate, addts->peakdatarate,
                    addts->burstsize, addts->delaybound, addts->PHYrate, addts->sba, addts->medium_time);
            }

            shell(IWPRIV" %s driver \"%s\"", setwmm->intf, buf);
        }
        else if (setwmm->action == WMMAC_DELTS)
        {
            // send del tspec
            wSPRINTF(buf, "delts tid %d", setwmm->actions.delts);

            shell(IWPRIV" %s driver \"%s\"", setwmm->intf, buf);
        }

        setwmmResp->status = STATUS_COMPLETE;
        break;

    case GROUP_WMMCONF:
        sprintf(gCmdStr, "iwconfig %s rts %d",
                ifname,setwmm->actions.config.rts_thr);

        sret = system(gCmdStr);
        sprintf(gCmdStr, "iwconfig %s frag %d",
                ifname,setwmm->actions.config.frag_thr);

        sret = system(gCmdStr);
        sprintf(gCmdStr, "iwpriv %s wmmcfg %d",
                ifname, setwmm->actions.config.wmm);

        sret = system(gCmdStr);
        setwmmResp->status = STATUS_COMPLETE;
        break;

    default:
        DPRINT_ERR(WFA_ERR, "The group %d is not supported\n",setwmm->group);
        setwmmResp->status = STATUS_ERROR;
        break;

    }

    wfaEncodeTLV(WFA_STA_SET_WMM_RESP_TLV, 4, (BYTE *)setwmmResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + 4;
#endif

    return WFA_SUCCESS;
}

int wfaStaSendNeigReq(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t *sendNeigReqResp = &gGenericResp;

    /*
     *  run your device to send NEIGREQ
     */

    sendNeigReqResp->status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_STA_SEND_NEIGREQ_RESP_TLV, 4, (BYTE *)sendNeigReqResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + 4;

    return WFA_SUCCESS;
}

int wfaStaSetEapFAST(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    caStaSetEapFAST_t *setFAST= (caStaSetEapFAST_t *)caCmdBuf;
    char *ifname = setFAST->intf;
    dutCmdResponse_t *setEapFastResp = &gGenericResp;

    DPRINT_INFO(WFA_OUT, "\nEntering wfaStaSetEapFAST ...\n");

    setEapFastResp->status = STATUS_INVALID;
    wfaEncodeTLV(WFA_STA_SET_EAPFAST_RESP_TLV, 4, (BYTE *)setEapFastResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + 4;

    return WFA_SUCCESS;
}

int wfaStaSetEapAKA(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    caStaSetEapAKA_t *setAKA = (caStaSetEapAKA_t *)caCmdBuf;
    char *intf = get_param_str("intf");
    dutCmdResponse_t *setEapAkaResp = &gGenericResp;
    int id;

    if (setAKA) {
        DPRINT_INFO(WFA_OUT, "\nEntering wfaStaSetEapAKA ...\n");
        set_param_str("ssid", setAKA->ssid);
        set_param_str("username", setAKA->username);
        set_param_str("password", setAKA->passwd);
        set_param_str("keyMgmtType", setAKA->keyMgmtType);
        set_param_str("encptype", setAKA->encrptype);
        set_param_val("pmf", setAKA->pmf);
        intf = setAKA->intf;
    }

    id = add_network_common(intf);
    if (id < 0) {
        setEapAkaResp->status = STATUS_ERROR;
        goto done;
    }

    if (set_eap_common(intf, id) != STATUS_COMPLETE) {
        setEapAkaResp->status = STATUS_ERROR;
        goto done;
    }

    const char *eapMethod = get_param_str("username") &&
            get_param_str("username")[0] == '6' ? "AKA'" : "AKA";
    if (set_network(intf, id, "eap", eapMethod) < 0) {
        setEapAkaResp->status = STATUS_ERROR;
        goto done;
    }

    setEapAkaResp->status = STATUS_COMPLETE;
done:
    wfaEncodeTLV(WFA_STA_SET_EAPAKA_RESP_TLV, 4, (BYTE *)setEapAkaResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + 4;

    return WFA_SUCCESS;
}

int wfaStaSetSystime(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    caStaSetSystime_t *systime = (caStaSetSystime_t *)caCmdBuf;
    dutCmdResponse_t *setSystimeResp = &gGenericResp;

    DPRINT_INFO(WFA_OUT, "\nEntering wfaStaSetSystime ... time=%d-%d-%d %d:%d:%d\n",
        systime->year, systime->month, systime->date,
        systime->hours, systime->minutes, systime->seconds);

    sprintf(gCmdStr, "date \"%d-%d-%d %d:%d:%d\"",
        systime->year, systime->month, systime->date,
        systime->hours, systime->minutes, systime->seconds);
    sret = system(gCmdStr);

    wpa_command("wlan0", "PMKSA_FLUSH");

    setSystimeResp->status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_STA_SET_SYSTIME_RESP_TLV, 4, (BYTE *)setSystimeResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + 4;

    return WFA_SUCCESS;
}

#ifdef WFA_STA_TB
int wfaStaPresetParams(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t *PresetParamsResp = &gGenericResp;
    caStaPresetParameters_t *presetParams = (caStaPresetParameters_t *)caCmdBuf;
    enum _response_staus status = STATUS_COMPLETE;

    DPRINT_INFO(WFA_OUT, "\nEntering wfaStaPresetParameters function ... intf=%s,supp=%d\n",
        presetParams->intf, presetParams->supplicant);

    // disable scan random mac for 11n
    wpa_command(presetParams->intf, "MAC_RAND_SCAN all enable=0");
#ifdef CONFIG_MTK_PPR2
    if (presetParams->program == PROG_TYPE_HS2_R2 && presetParams->fileType[0] != '\0') {
        if (strcasecmp(presetParams->fileType, "PPSMO") == 0) {
            if (sta_download_ppsmo(presetParams->intf, presetParams->fileName,
                    presetParams->filePath, presetParams->managementTreeURI))
                goto exit;
        } else if(strcasecmp(presetParams->fileType, "CERT") == 0) {
            if (sta_download_cert(presetParams->intf, presetParams->fileName,
                    presetParams->filePath))
                goto exit;
        } else {
            DPRINT_ERR(WFA_ERR, "Unsupported FileType\n");
        }
        status = STATUS_ERROR;
        goto exit;
    }
#endif

exit:
    PresetParamsResp->status = status;
    wfaEncodeTLV(WFA_STA_PRESET_PARAMETERS_RESP_TLV, 4, (BYTE *)PresetParamsResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + 4;

    return WFA_SUCCESS;
}

int wfaStaSet11n(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t *v11nParamsResp = &gGenericResp;

    v11nParamsResp->status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_STA_SET_11N_RESP_TLV, 4, (BYTE *)v11nParamsResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + 4;
    return WFA_SUCCESS;
}
int wfaStaSetWireless(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t *staWirelessResp = &gGenericResp;

    staWirelessResp->status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_STA_SET_WIRELESS_RESP_TLV, 4, (BYTE *)staWirelessResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + 4;
    return WFA_SUCCESS;
}

int wfaStaSendADDBA(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t *staSendADDBAResp = &gGenericResp;

    staSendADDBAResp->status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_STA_SET_SEND_ADDBA_RESP_TLV, 4, (BYTE *)staSendADDBAResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + 4;
    return WFA_SUCCESS;
}

int wfaStaSetRIFS(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t *staSetRIFSResp = &gGenericResp;

    staSetRIFSResp->status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_STA_SET_RIFS_TEST_RESP_TLV, 4, (BYTE *)staSetRIFSResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + 4;

    return WFA_SUCCESS;
}

int wfaStaSendCoExistMGMT(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t *staSendMGMTResp = &gGenericResp;

    staSendMGMTResp->status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_STA_SEND_COEXIST_MGMT_RESP_TLV, 4, (BYTE *)staSendMGMTResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + 4;

    return WFA_SUCCESS;

}

int wfaStaResetDefault(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    caStaResetDefault_t *reset = (caStaResetDefault_t *)caCmdBuf;
    dutCmdResponse_t *ResetResp = &gGenericResp;

    DPRINT_INFO(WFA_OUT, "\nEntering wfaStaResetDefault ... prog=%s\n", reset->prog);

    remove_wpa_networks(reset->intf);

    reset_param();
    set_param_str("prog", reset->prog);
    gDut.program = str2prog(reset->prog);

    // disable scan random mac
    wpa_command(reset->intf, "MAC_RAND_SCAN all enable=0");

#ifdef CONFIG_MTK_PPR2
    if (gDut.program == PROG_TYPE_HS2 || gDut.program == PROG_TYPE_HS2_R2) {
        wpa_command(reset->intf, "SET interworking 1");
        wpa_command(reset->intf, "REMOVE_CRED all");
    }
#endif

    ResetResp->status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_STA_RESET_DEFAULT_RESP_TLV, 4, (BYTE *)ResetResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + 4;

    return WFA_SUCCESS;
}

#else

int wfaStaTestBedCmd(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t *staCmdResp = &gGenericResp;

    wfaEncodeTLV(WFA_STA_DISCONNECT_RESP_TLV, 4, (BYTE *)staCmdResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + 4;

    return WFA_SUCCESS;
}
#endif

/*
 * This is used to send a frame or action frame
 */
int wfaStaDevSendFrame(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCommand_t *cmd = (dutCommand_t *)caCmdBuf;
    /* uncomment it if needed */
    // char *ifname = cmd->intf;
    dutCmdResponse_t *devSendResp = &gGenericResp;
    caStaDevSendFrame_t *sf = &cmd->cmdsu.sf;
    char buf[256];
    enum _response_staus status = STATUS_COMPLETE;

    DPRINT_INFO(WFA_OUT, "\nEntering wfaStaDevSendFrame function ...prog=%d\n", sf->program);
    /* processing the frame */

    switch(sf->program)
    {
    case PROG_TYPE_PMF:
    {
        pmfFrame_t *pmf = &sf->frameType.pmf;
        switch(pmf->eFrameName)
        {
        case PMF_TYPE_DISASSOC:
        {
            /* use the protected to set what type of key to send */

        }
        break;
        case PMF_TYPE_DEAUTH:
        {

        }
        break;
        case PMF_TYPE_SAQUERY:
        {

        }
        break;
        case PMF_TYPE_AUTH:
        {
        }
        break;
        case PMF_TYPE_ASSOCREQ:
        {
        }
        break;
        case PMF_TYPE_REASSOCREQ:
        {
        }
        break;
        }
    }
    break;
    case PROG_TYPE_TDLS:
    {
        tdlsFrame_t *tdls = &sf->frameType.tdls;
        DPRINT_INFO(WFA_OUT, "tdls frame=%d\n", tdls->eFrameName);
        /* use the peer mac address to send the frame */
        switch(tdls->eFrameName)
        {
            case TDLS_TYPE_DISCOVERY:
                wSPRINTF(buf, "TDLS_DISCOVER %s", tdls->peer);
                break;
            case TDLS_TYPE_SETUP:
                wSPRINTF(buf, "TDLS_SETUP %s", tdls->peer);
                break;
            case TDLS_TYPE_TEARDOWN:
                wSPRINTF(buf, "TDLS_TEARDOWN %s", tdls->peer);
                break;
            case TDLS_TYPE_CHANNELSWITCH:
                break;
            case TDLS_TYPE_NULLFRAME:
                break;
        }
        wpa_command(cmd->intf, buf);
    }
    break;
    case PROG_TYPE_VENT:
    {
        ventFrame_t *vent = &sf->frameType.vent;

        DPRINT_INFO(WFA_OUT, "Vent type %d, ssid %s\n", vent->type, vent->ssid);
        switch(vent->type)
        {
        case VENT_TYPE_NEIGREQ:
            wSPRINTF(buf, "DRIVER NEIGHBOR-REQUEST SSID=%s", vent->ssid);
            break;
        case VENT_TYPE_TRANSMGMT:
            wSPRINTF(buf, "DRIVER BSS-TRANSITION-QUERY reason=6");
            break;
        }
        wpa_command(cmd->intf, buf);
    }
    break;
    case PROG_TYPE_WFD:
    {
        wfdFrame_t *wfd = &sf->frameType.wfd;
        switch(wfd->eframe)
        {
        case WFD_FRAME_PRBREQ:
        {
            /* send probe req */
        }
        break;

        case WFD_FRAME_PRBREQ_TDLS_REQ:
        {
            /* send tunneled tdls probe req  */
        }
        break;

        case WFD_FRAME_11V_TIMING_MSR_REQ:
        {
            /* send 11v timing mearurement request */
        }
        break;

        case WFD_FRAME_RTSP:
        {
            /* send WFD RTSP messages*/
            // fetch the type of RTSP message and send it.
            switch(wfd->eRtspMsgType)
            {
            case WFD_RTSP_PAUSE:
                break;
            case WFD_RTSP_PLAY:
                //send RTSP PLAY
                break;
            case WFD_RTSP_TEARDOWN:
                //send RTSP TEARDOWN
                break;
            case WFD_RTSP_TRIG_PAUSE:
                //send RTSP TRIGGER PAUSE
                break;
            case WFD_RTSP_TRIG_PLAY:
                //send RTSP TRIGGER PLAY
                break;
            case WFD_RTSP_TRIG_TEARDOWN:
                //send RTSP TRIGGER TEARDOWN
                break;
            case WFD_RTSP_SET_PARAMETER:
                //send RTSP SET PARAMETER
                if (wfd->eSetParams == WFD_CAP_UIBC_KEYBOARD)
                {
                    //send RTSP SET PARAMETER message for UIBC keyboard
                }
                if (wfd->eSetParams == WFD_CAP_UIBC_MOUSE)
                {
                    //send RTSP SET PARAMETER message for UIBC Mouse
                }
                else if (wfd->eSetParams == WFD_CAP_RE_NEGO)
                {
                    //send RTSP SET PARAMETER message Capability re-negotiation
                }
                else if (wfd->eSetParams == WFD_STANDBY)
                {
                    //send RTSP SET PARAMETER message for standby
                }
                else if (wfd->eSetParams == WFD_UIBC_SETTINGS_ENABLE)
                {
                    //send RTSP SET PARAMETER message for UIBC settings enable
                }
                else if (wfd->eSetParams == WFD_UIBC_SETTINGS_DISABLE)
                {
                    //send RTSP SET PARAMETER message for UIBC settings disable
                }
                else if (wfd->eSetParams == WFD_ROUTE_AUDIO)
                {
                    //send RTSP SET PARAMETER message for route audio
                }
                else if (wfd->eSetParams == WFD_3D_VIDEOPARAM)
                {
                    //send RTSP SET PARAMETER message for 3D video parameters
                }
                else if (wfd->eSetParams == WFD_2D_VIDEOPARAM)
                {
                    //send RTSP SET PARAMETER message for 2D video parameters
                }
                break;
            }
        }
        break;
        }
    }
    break;
    /* not need to support HS2 release 1, due to very short time period  */
    case PROG_TYPE_HS2_R2:
    {
        /* type of frames */
        hs2Frame_t *hs2 = &sf->frameType.hs2_r2;
        if (hs2->eframe != HS2_FRAME_ANQPQuery) {
            DPRINT_ERR(WFA_ERR, "HS2 doesn't support frameType:%d\n", hs2->eframe);
            status = STATUS_ERROR;
            goto exit;
        }

        if (sta_find_ap(cmd->intf, hs2->sDestMac) < 0) {
            DPRINT_ERR(WFA_ERR, "Cloud not find the requested AP\n");
            status = STATUS_ERROR;
            goto exit;
        }

        char buf[200], *pos, *end, *val;
        int count, count2;

        pos = buf;
        end = buf + sizeof(buf);
        count = 0;
        pos += wSNPRINTF(pos, end - pos, "ANQP_GET %s ", hs2->sDestMac);

        if (hs2->bAnqpCapList) {
            pos += wSNPRINTF(pos, end - pos, "%s257", count > 0 ? "," : "");
            count++;
        }

        if (hs2->bVenueName) {
            pos += wSNPRINTF(pos, end - pos, "%s258", count > 0 ? "," : "");
            count++;
        }

        if (hs2->bNetAuthType) {
            pos += wSNPRINTF(pos, end - pos, "%s260", count > 0 ? "," : "");
            count++;
        }

        if (hs2->bRoamingCons) {
            pos += wSNPRINTF(pos, end - pos, "%s261", count > 0 ? "," : "");
            count++;
        }

        if (hs2->bNaiRealmList) {
            pos += wSNPRINTF(pos, end - pos, "%s263", count > 0 ? "," : "");
            count++;
        }

        if (hs2->b3gppInfo) {
            pos += wSNPRINTF(pos, end - pos, "%s264", count > 0 ? "," : "");
            count++;
        }

        if (hs2->bDomainList) {
            pos += wSNPRINTF(pos, end - pos, "%s268", count > 0 ? "," : "");
            count++;
        }

        if (count && wpa_command(cmd->intf, buf) < 0) {
            DPRINT_ERR(WFA_ERR, "ANQP_GET failed\n");
            status = STATUS_ERROR;
            goto exit;
        }

        pos = buf;
        end = buf + sizeof(buf);
        count2 = 0;
        pos += wSNPRINTF(pos, end - pos, "HS20_ANQP_GET %s ", hs2->sDestMac);

        if (hs2->bHsCapList) {
            pos += wSNPRINTF(pos, end - pos, "%s2", count2 > 0 ? "," : "");
            count2++;
        }

        if (hs2->bOperName) {
            pos += wSNPRINTF(pos, end - pos, "%s3", count2 > 0 ? "," : "");
            count2++;
        }

        if (hs2->bWanMat) {
            pos += wSNPRINTF(pos, end - pos, "%s4", count2 > 0 ? "," : "");
            count2++;
        }

        if (hs2->bOpClass) {
            pos += wSNPRINTF(pos, end - pos, "%s7", count2 > 0 ? "," : "");
            count2++;
        }

        if (hs2->bOsuProviderList) {
            pos += wSNPRINTF(pos, end - pos, "%s8", count2 > 0 ? "," : "");
            count2++;
        }

        if (count && count2) {
            DPRINT_INFO(WFA_OUT, "Wait before sending out next query\n");
            wSLEEP(1);
        }

        if (count2 && wpa_command(cmd->intf, buf) < 0) {
            DPRINT_ERR(WFA_ERR, "HS20_ANQP_GET failed\n");
            status = STATUS_ERROR;
            goto exit;
        }

        if(hs2->bNaiHomeRealmList) {
            if (count && count2) {
                DPRINT_INFO(WFA_OUT, "Wait before sending out next query\n");
                wSLEEP(1);
            }
            if (hs2->bNaiHomeRealmList == 1)
                val = "mail.example.com";
            else
                val = "invalid.realm.com";
            wSNPRINTF(buf, end - pos,
                "HS20_GET_NAI_HOME_REALM_LIST %s realm=%s",
                hs2->sDestMac, val);
            if (wpa_command(cmd->intf, buf) < 0) {
                DPRINT_ERR(WFA_ERR, "HS20_GET_NAI_HOME_REALM_LIST failed\n");
                status = STATUS_ERROR;
                goto exit;
            }
        }

        if (hs2->sIconRequest[0] != '\0') {
            if (count || count2) {
                DPRINT_INFO(WFA_OUT, "Wait before sending out next query\n");
                wSLEEP(1);
            }

            wSNPRINTF(buf, end - pos, "HS20_ICON_REQUEST %s %s", hs2->sDestMac, hs2->sIconRequest);
            if (wpa_command(cmd->intf, buf) < 0) {
                DPRINT_ERR(WFA_ERR, "HS20_ICON_REQUEST failed\n");
                status = STATUS_ERROR;
                goto exit;
            }
        }

        break;
    }/*  PROG_TYPE_HS2-R2  */
    case PROG_TYPE_GEN:
    {
        /* General frames */
    }
    }
exit:
    devSendResp->status = status;
    wfaEncodeTLV(WFA_STA_DEV_SEND_FRAME_RESP_TLV, 4, (BYTE *)devSendResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + 4;

    return WFA_SUCCESS;
}

/*
 * This is used to set a temporary MAC address of an interface
 */
int wfaStaSetMacAddr(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    // Uncomment it if needed
    //dutCommand_t *cmd = (dutCommand_t *)caCmdBuf;
    // char *ifname = cmd->intf;
    dutCmdResponse_t *staCmdResp = &gGenericResp;
    // Uncomment it if needed
    //char *macaddr = &cmd->cmdsu.macaddr[0];

    wfaEncodeTLV(WFA_STA_SET_MAC_ADDRESS_RESP_TLV, 4, (BYTE *)staCmdResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + 4;

    return WFA_SUCCESS;
}


int wfaStaDisconnect(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCommand_t *disc = (dutCommand_t *)caCmdBuf;
    char *intf = disc->intf;
    dutCmdResponse_t *staDiscResp = &gGenericResp;

    DPRINT_INFO(WFA_OUT, "\nEntering wfaStaDisconnect ...\n");

    wpa_command(intf, "DISCONNECT");

    /*
     * remove this due to UCC, e.g. 11n 5.2.53, won't guantee that it will
     * set network parameters after issue disconnect
     *
     * remove_wpa_networks(intf);
     */

    staDiscResp->status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_STA_DISCONNECT_RESP_TLV, 4, (BYTE *)staDiscResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + 4;

    return WFA_SUCCESS;
}

/* Execute CLI, read the status from Environment variable */
int wfaExecuteCLI(char *CLI)
{
    char *retstr;

    sret = system(CLI);

    retstr = getenv("WFA_CLI_STATUS");
    printf("cli status %s\n", retstr);
    return atoi(retstr);
}

/* Supporting Functions */

void wfaSendPing(tgPingStart_t *staPing, float *interval, int streamid)
{
    int totalpkts, tos=-1;
    char cmdStr[256];
    char bflag[] = "  ";

    totalpkts = (int)(staPing->duration * staPing->frameRate);

    if (staPing->dscp >= 0) {
        tos= convertDscpToTos(staPing->dscp);
        if (tos < 0)
            DPRINT_WARNING(WFA_WNG, "invalid tos converted, dscp=%d\n",
                    staPing->dscp);
    }
    DPRINT_INFO(WFA_OUT, "Entering %s(), streamid:%d iptype:%d tos:%d\n",
            __func__, streamid, staPing->iptype, tos);

    if (staPing->iptype == 2) {
        if (tos > 0)
            wSNPRINTF(cmdStr, sizeof(cmdStr), "echo streamid=%i > /data/local/spout_%d.txt;" WFAPING6 " %s -i %f -c %i -Q %d -s %i -q %s >> /data/local/spout_%d.txt 2>/dev/null",
                    streamid, streamid, bflag, *interval, totalpkts, tos,  staPing->frameSize, staPing->dipaddr, streamid);
        else
            wSNPRINTF(cmdStr, sizeof(cmdStr), "echo streamid=%i > /data/local/spout_%d.txt;" WFAPING6 " %s -i %f -c %i -s %i -q %s >> /data/local/spout_%d.txt 2>/dev/null",
                    streamid, streamid, bflag, *interval, totalpkts, staPing->frameSize, staPing->dipaddr, streamid);
    } else {
        if (tos > 0)
            wSNPRINTF(cmdStr, sizeof(cmdStr), "echo streamid=%i > /data/local/spout_%d.txt;" WFAPING " %s -i %f -c %i -Q %d -s %i -q %s >> /data/local/spout_%d.txt 2>/dev/null",
                    streamid, streamid, bflag, *interval, totalpkts, tos, staPing->frameSize, staPing->dipaddr,streamid);
        else
            wSNPRINTF(cmdStr, sizeof(cmdStr), "echo streamid=%i > /data/local/spout_%d.txt;" WFAPING " %s -i %f -c %i -s %i -q %s >> /data/local/spout_%d.txt 2>/dev/null",
                    streamid, streamid, bflag, *interval, totalpkts, staPing->frameSize, staPing->dipaddr, streamid);
    }
    DPRINT_INFO(WFA_OUT, "cmdStr: %s\n", cmdStr);
    system(cmdStr);

    sprintf(cmdStr, UPDATEPID " /data/local/spout_%d.txt", streamid);
    DPRINT_INFO(WFA_OUT, "cmdStr: %s\n", cmdStr);
    system(cmdStr);
}

int wfaStopPing(dutCmdResponse_t *stpResp, int streamid)
{
    char strout[256];
    FILE *tmpfile = NULL;
    char cmdStr[256];

    DPRINT_INFO(WFA_OUT, "Entering %s(), streamid:%d\n",
            __func__, streamid);

    sprintf(cmdStr, GETPID " /data/local/spout_%d.txt /data/local/pid.txt",
            streamid);
    DPRINT_INFO(WFA_OUT, "cmdStr: %s\n", cmdStr);
    system(cmdStr);

    sprintf(cmdStr, STOPPING " /data/local/pid.txt ; sleep 2");
    DPRINT_INFO(WFA_OUT, "cmdStr: %s\n", cmdStr);
    system(cmdStr);

    sprintf(cmdStr, GETPSTATS " /data/local/spout_%d.txt", streamid);
    DPRINT_INFO(WFA_OUT, "cmdStr: %s\n", cmdStr);
    system(cmdStr);

    tmpfile = fopen("/data/local/stpsta.txt", "r+");
    if (tmpfile == NULL)
        return WFA_FAILURE;
    if (fscanf(tmpfile, "%255s", strout) != EOF) {
        if (*strout == '\0')
            stpResp->cmdru.pingStp.sendCnt = 0;
        else
            stpResp->cmdru.pingStp.sendCnt = atoi(strout);
    }
    DPRINT_INFO(WFA_OUT, "sent count:%d\n", stpResp->cmdru.pingStp.sendCnt);
    if (fscanf(tmpfile, "%255s", strout) != EOF) {
        if (*strout == '\0')
            stpResp->cmdru.pingStp.repliedCnt = 0;
        else
            stpResp->cmdru.pingStp.repliedCnt = atoi(strout);
    }
    DPRINT_INFO(WFA_OUT, "replied count:%d\n",
            stpResp->cmdru.pingStp.repliedCnt);
    fclose(tmpfile);

    return WFA_SUCCESS;
}

/*
 * wfaStaGetP2pDevAddress():
 */
int wfaStaGetP2pDevAddress(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t *infoResp = &gGenericResp;
    dutCommand_t *getInfo = (dutCommand_t *)caCmdBuf;
    char buf[100];
    enum _response_staus status = STATUS_COMPLETE;

    DPRINT_INFO(WFA_OUT, "Entering %s()\n", __func__);

    if (get_wpa_status(getInfo->intf, "p2p_device_address", buf,
            sizeof(buf)) < 0) {
        status = STATUS_ERROR;
    } else {
        status = STATUS_COMPLETE;
        wSNPRINTF(infoResp->cmdru.devid, sizeof(infoResp->cmdru.devid),
                "%s", buf);
    }

    infoResp->status = status;
    wfaEncodeTLV(WFA_STA_P2P_GET_DEV_ADDRESS_RESP_TLV, sizeof(dutCmdResponse_t),
            (BYTE *)infoResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

    return WFA_SUCCESS;
}

/*
 * wfaStaSetP2p():
 */
int wfaStaSetP2p(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t *infoResp = &gGenericResp;
    caStaSetP2p_t *getStaSetP2p = (caStaSetP2p_t *)caCmdBuf;
    char buf[256];
    enum _response_staus status = STATUS_COMPLETE;

    DPRINT_INFO(WFA_OUT, "Entering %s()\n", __func__);

    if (getStaSetP2p->listen_chn_flag) {
        wSNPRINTF(buf, sizeof(buf), "P2P_SET listen_channel %d",
                getStaSetP2p->listen_chn);
        if (wpa_command(getStaSetP2p->intf, buf) != 0) {
            status = STATUS_ERROR;
            goto exit;
        }
    }

    if (getStaSetP2p->noa_duration_flag && getStaSetP2p->noa_count_flag &&
            getStaSetP2p->noa_interval_flag) {
        wSNPRINTF(buf, sizeof(buf), "P2P_SET noa %d,%d,%d",
                getStaSetP2p->noa_count,
                getStaSetP2p->noa_interval,
                getStaSetP2p->noa_duration);
        if (wpa_command(getStaSetP2p->intf, buf) != 0) {
            status = STATUS_ERROR;
            goto exit;
        }
    }

    if (getStaSetP2p->discoverability_flag) {
        wSNPRINTF(buf, sizeof(buf), "P2P_SET discoverability %d",
                getStaSetP2p->discoverability);
        if (wpa_command(getStaSetP2p->intf, buf) != 0) {
            status = STATUS_ERROR;
            goto exit;
        }
    }

    if (getStaSetP2p->presistent_flag) {
        gDut.p2p_params.persistent = getStaSetP2p->presistent;
    }

    if (getStaSetP2p->p2pmanaged_flag) {
        wSNPRINTF(buf, sizeof(buf), "P2P_SET managed %d",
                getStaSetP2p->p2pmanaged);
        if (wpa_command(getStaSetP2p->intf, buf) != 0) {
            status = STATUS_ERROR;
            goto exit;
        }
    }

    if (getStaSetP2p->go_apsd_flag) {
        wSNPRINTF(buf, sizeof(buf), "P2P_SET go_apsd %d",
                getStaSetP2p->go_apsd);
        if (wpa_command(getStaSetP2p->intf, buf) != 0) {
            status = STATUS_ERROR;
            goto exit;
        }
    }

    if (getStaSetP2p->crossconnection_flag) {
        wSNPRINTF(buf, sizeof(buf), "P2P_SET cross_connect %d",
                getStaSetP2p->crossconnection);
        if (wpa_command(getStaSetP2p->intf, buf) != 0) {
            status = STATUS_ERROR;
            goto exit;
        }
    }

    if (getStaSetP2p->p2p_mode_flag) {
        if (strcasecmp(getStaSetP2p->p2p_mode, "listen") == 0) {
            if (wpa_command(getStaSetP2p->intf, "P2P_SET disabled 0") != 0) {
                status = STATUS_ERROR;
                goto exit;
            }
            if (wpa_command(getStaSetP2p->intf, "P2P_LISTEN") != 0) {
                status = STATUS_ERROR;
                goto exit;
            }
            gDut.p2p_params.p2p_mode = P2P_MODE_LISTEN;
        } else if (strcasecmp(getStaSetP2p->p2p_mode, "Discover") == 0) {
            if (wpa_command(getStaSetP2p->intf, "P2P_SET disabled 0") != 0) {
                status = STATUS_ERROR;
                goto exit;
            }
            if (wpa_command(getStaSetP2p->intf, "P2P_FIND") != 0) {
                status = STATUS_ERROR;
                goto exit;
            }
            gDut.p2p_params.p2p_mode = P2P_MODE_DISCOVER;
        } else if (strcasecmp(getStaSetP2p->p2p_mode, "Idle") == 0) {
            if (wpa_command(getStaSetP2p->intf, "P2P_SET disabled 0") != 0) {
                status = STATUS_ERROR;
                goto exit;
            }
            if (wpa_command(getStaSetP2p->intf, "P2P_STOP_FIND") != 0) {
                status = STATUS_ERROR;
                goto exit;
            }
            gDut.p2p_params.p2p_mode = P2P_MODE_IDLE;
        } else if (strcasecmp(getStaSetP2p->p2p_mode, "Disable") == 0) {
            if (wpa_command(getStaSetP2p->intf, "P2P_SET disabled 1") != 0) {
                status = STATUS_ERROR;
                goto exit;
            }
            gDut.p2p_params.p2p_mode = P2P_MODE_DISABLE;
        } else {
            status = STATUS_ERROR;
            goto exit;
        }
    }

    if (getStaSetP2p->ext_listen_time_int_flag &&
            getStaSetP2p->ext_listen_time_period_flag) {
        wSNPRINTF(buf, sizeof(buf), "P2P_EXT_LISTEN %d %d",
                getStaSetP2p->ext_listen_time_period,
                getStaSetP2p->ext_listen_time_int);
        if (wpa_command(getStaSetP2p->intf, buf) != 0) {
            status = STATUS_ERROR;
            goto exit;
        }
    }

exit:
    infoResp->status = status;
    wfaEncodeTLV(WFA_STA_P2P_SETP2P_RESP_TLV, sizeof(dutCmdResponse_t),
            (BYTE *)infoResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

    return status;
}

/*
 * wfaStaP2pConnect():
 */
int wfaStaP2pConnect(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t *infoResp = &gGenericResp;
    caStaP2pConnect_t *getStaP2pConnect = (caStaP2pConnect_t *)caCmdBuf;
    enum _response_staus status = STATUS_COMPLETE;
    char buf[256];
    struct wpa_ctrl *ctrl;
    int res;

    DPRINT_INFO(WFA_OUT, "Entering %s()\n", __func__);

    if (p2p_find_peer(getStaP2pConnect->intf, getStaP2pConnect->devId, 1) <= 0) {
        status = STATUS_ERROR;
        goto exit;
    }

    switch (gDut.p2p_params.wps_method) {
    case WPS_METHOD_PBC:
        wSNPRINTF(buf, sizeof(buf), "P2P_CONNECT %s pbc join",
                getStaP2pConnect->devId);
        break;
    case WPS_METHOD_PIN_DISPLAY:
        wSNPRINTF(buf, sizeof(buf), "P2P_CONNECT %s %s display join",
                getStaP2pConnect->devId,
                gDut.p2p_params.wpsPin);
        break;
    case WPS_METHOD_KEYPAD:
        wSNPRINTF(buf, sizeof(buf), "P2P_CONNECT %s %s keypad join",
                getStaP2pConnect->devId,
                gDut.p2p_params.wpsPin);
        break;
    default:
        status = STATUS_ERROR;
        goto exit;
    }

    ctrl = open_wpa_mon(getStaP2pConnect->intf);
    if (ctrl == NULL) {
        DPRINT_ERR(WFA_ERR, "open wpa mon fail.\n");
        status = STATUS_ERROR;
        goto exit;
    }

    if (wpa_command(getStaP2pConnect->intf, buf) != 0) {
        status = STATUS_ERROR;
        goto exit;
    }

    res = get_wpa_cli_event(ctrl, "P2P-GROUP-STARTED", buf, sizeof(buf));

    wpa_ctrl_detach(ctrl);
    wpa_ctrl_close(ctrl);

    if (res < 0) {
        DPRINT_ERR(WFA_ERR, "p2p connect fail.\n");
        status = STATUS_ERROR;
        goto exit;
    }

exit:
    infoResp->status = status;
    wfaEncodeTLV(WFA_STA_P2P_CONNECT_RESP_TLV, sizeof(dutCmdResponse_t),
            (BYTE *)infoResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

    return status;
}

/*
 * wfaStaStartAutoGo():
 */
int wfaStaStartAutoGo(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t *infoResp = &gGenericResp;
    caStaStartAutoGo_t *getStaStartAutoGo = (caStaStartAutoGo_t *)caCmdBuf;
    int freq;
    enum _response_staus status = STATUS_COMPLETE;
    char buf[256];
    struct wpa_ctrl *ctrl;
    int res;
    char *pos, *ssid, *go_dev_addr;

    DPRINT_INFO(WFA_OUT, "Entering %s()\n", __func__);

    if (getStaStartAutoGo->oper_chn >= 1 && getStaStartAutoGo->oper_chn <= 13) {
        freq = 2412 + (getStaStartAutoGo->oper_chn - 1) * 5;
    } else if (getStaStartAutoGo->oper_chn == 14) {
        freq = 2484;
    } else if (getStaStartAutoGo->oper_chn >= 36 &&
            getStaStartAutoGo->oper_chn <= 165) {
        freq = 5000 + getStaStartAutoGo->oper_chn * 5;
    } else {
        status = STATUS_ERROR;
        goto exit;
    }

    if (getStaStartAutoGo->ssid_flag) {
        wSNPRINTF(buf, sizeof(buf), "P2P_SET ssid_postfix %s",
                getStaStartAutoGo->ssid);
    } else {
        wSNPRINTF(buf, sizeof(buf), "P2P_SET ssid_postfix ");
    }
    if (wpa_command(getStaStartAutoGo->intf, buf) != 0) {
        status = STATUS_ERROR;
        goto exit;
    }

    if (wpa_command(getStaStartAutoGo->intf, "P2P_STOP_FIND") != 0) {
        status = STATUS_ERROR;
        goto exit;
    }

    ctrl = open_wpa_mon(getStaStartAutoGo->intf);
    if (ctrl == NULL) {
        DPRINT_ERR(WFA_ERR, "open wpa mon fail.\n");
        status = STATUS_ERROR;
        goto exit;
    }

    wSNPRINTF(buf, sizeof(buf), "P2P_GROUP_ADD %sfreq=%d",
            gDut.p2p_params.persistent ? "persistent " : "",
            freq);
    if (wpa_command(getStaStartAutoGo->intf, buf) != 0) {
        status = STATUS_ERROR;
        goto exit;
    }

    res = get_wpa_cli_event(ctrl, "P2P-GROUP-STARTED", buf, sizeof(buf));

    wpa_ctrl_detach(ctrl);
    wpa_ctrl_close(ctrl);

    if (res < 0) {
        DPRINT_ERR(WFA_ERR, "start auto GO fail.\n");
        status = STATUS_ERROR;
        goto exit;
    }

    DPRINT_INFO(WFA_OUT, "buf: %s\n", buf);

    ssid = strstr(buf, "ssid=\"");
    if (ssid == NULL) {
        DPRINT_ERR(WFA_ERR, "get ssid NULL.\n");
        status = STATUS_ERROR;
        goto exit;
    }
    ssid += 6;
    pos = strchr(ssid, '"');
    if (pos == NULL) {
        DPRINT_ERR(WFA_ERR, "get ssid terminator fail.\n");
        status = STATUS_ERROR;
        goto exit;
    }
    *pos++ = '\0';
    DPRINT_INFO(WFA_OUT, "ssid: %s\n", ssid);

    go_dev_addr = strstr(pos, "go_dev_addr=");
    if (go_dev_addr == NULL) {
        DPRINT_ERR(WFA_ERR, "get go_dev_addr NULL.\n");
        status = STATUS_ERROR;
        goto exit;
    }
    go_dev_addr += 12;
    go_dev_addr[WFA_P2P_DEVID_LEN - 1] = '\0';
    DPRINT_INFO(WFA_OUT, "go_dev_addr: %s\n", go_dev_addr);

    wSNPRINTF(infoResp->cmdru.grpid, sizeof(infoResp->cmdru.grpid), "%s %s",
            go_dev_addr, ssid);

exit:
    infoResp->status = status;
    wfaEncodeTLV(WFA_STA_P2P_START_AUTO_GO_RESP_TLV, sizeof(dutCmdResponse_t),
            (BYTE *)infoResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

    return status;
}

/*
 * wfaStaP2pStartGrpFormation():
 */
int wfaStaP2pStartGrpFormation(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t *infoResp = &gGenericResp;
    enum _response_staus status = STATUS_COMPLETE;
    caStaP2pStartGrpForm_t *getStaP2pStartGrpForm = (caStaP2pStartGrpForm_t *)caCmdBuf;
    int freq = 0;
    char buf[256];
    struct wpa_ctrl *ctrl = NULL;

    DPRINT_INFO(WFA_OUT, "Entering %s()\n", __func__);

    if (getStaP2pStartGrpForm->ssid_flag)
        wSNPRINTF(buf, sizeof(buf), "P2P_SET ssid_postfix %s",
                getStaP2pStartGrpForm->ssid);
    else
        wSNPRINTF(buf, sizeof(buf), "P2P_SET ssid_postfix ");
    if (wpa_command(getStaP2pStartGrpForm->intf, buf) < 0) {
        status = STATUS_ERROR;
        goto exit;
    }

    if (getStaP2pStartGrpForm->oper_chn_flag) {
        if (getStaP2pStartGrpForm->oper_chn >= 1 && getStaP2pStartGrpForm->oper_chn <= 13) {
            freq = 2412 + (getStaP2pStartGrpForm->oper_chn - 1) * 5;
        } else if (getStaP2pStartGrpForm->oper_chn == 14) {
            freq = 2484;
        } else if (getStaP2pStartGrpForm->oper_chn >= 36 &&
                getStaP2pStartGrpForm->oper_chn <= 165) {
            freq = 5000 + getStaP2pStartGrpForm->oper_chn * 5;
        } else {
            status = STATUS_ERROR;
            goto exit;
        }
    }

    if (p2p_find_peer(getStaP2pStartGrpForm->intf, getStaP2pStartGrpForm->devId,
            getStaP2pStartGrpForm->init_go_neg) <= 0) {
        status = STATUS_ERROR;
        goto exit;
    }

    if (getStaP2pStartGrpForm->init_go_neg) {
        ctrl = open_wpa_mon(getStaP2pStartGrpForm->intf);
        if (ctrl == NULL) {
            DPRINT_WARNING(WFA_WNG, "open wpa mon fail.\n");
            status = STATUS_ERROR;
            goto exit;
        }
    }

    wSNPRINTF(buf, sizeof(buf), "P2P_CONNECT %s %s%s%s%s go_intent=%d",
            getStaP2pStartGrpForm->devId,
            gDut.p2p_params.wps_method == WPS_METHOD_PBC ?
                    "pbc" : gDut.p2p_params.wpsPin,
            gDut.p2p_params.wps_method == WPS_METHOD_PBC ?
                    "" : (gDut.p2p_params.wps_method == WPS_METHOD_PIN_DISPLAY ?
                            " display" :
                            (gDut.p2p_params.wps_method == WPS_METHOD_PIN_LABEL ?
                                    " label" : " keypad")),
            gDut.p2p_params.persistent ? " persistent" : "",
            getStaP2pStartGrpForm->init_go_neg ? "" : " auth",
            getStaP2pStartGrpForm->intent_val);
    if (freq > 0)
        wSNPRINTF(buf + strlen(buf), sizeof(buf) - strlen(buf), " freq=%d",
                freq);

    if (wpa_command(getStaP2pStartGrpForm->intf, buf) < 0) {
        DPRINT_WARNING(WFA_WNG, "p2p connect fail\n");
        status = STATUS_ERROR;
        goto exit;
    }

    if (!getStaP2pStartGrpForm->init_go_neg) {
        goto exit;
    } else {
        int res;
        const char *events[] = {
                "P2P-GROUP-STARTED",
                "P2P-GO-NEG-FAILURE",
                "P2P-GROUP-FORMATION-FAILURE",
                NULL
        };

        res = get_wpa_cli_events(ctrl, events, buf, sizeof(buf));
        wpa_ctrl_detach(ctrl);
        wpa_ctrl_close(ctrl);
        ctrl = NULL;
        if (res < 0) {
            DPRINT_WARNING(WFA_WNG, "wait for p2p formation events fail\n");
            status = STATUS_ERROR;
            goto exit;
        }
        DPRINT_INFO(WFA_OUT, "formation event: %s\n", buf);
        if (strstr(buf, "P2P-GO-NEG-FAILURE")) {
            status = STATUS_ERROR;
            goto exit;
        } else if (strstr(buf, "P2P-GROUP-FORMATION-FAILURE")) {
            status = STATUS_ERROR;
            goto exit;
        } else {
            char *pos, *group_type, *ssid, *go_dev_addr;

            pos = strchr(buf, ' ');
            if (pos == NULL) {
                status = STATUS_ERROR;
                goto exit;
            }
            pos++;
            pos = strchr(pos, ' ');
            if (pos == NULL) {
                status = STATUS_ERROR;
                goto exit;
            }
            pos++;
            group_type = pos;
            pos = strchr(group_type, ' ');
            if (pos == NULL) {
                status = STATUS_ERROR;
                goto exit;
            }
            *pos++ = '\0';
            DPRINT_INFO(WFA_OUT, "group_type: %s\n", group_type);

            ssid = strstr(pos, "ssid=\"");
            if (ssid == NULL) {
                status = STATUS_ERROR;
                goto exit;
            }
            ssid += 6;
            pos = strchr(ssid, '"');
            if (pos == NULL) {
                status = STATUS_ERROR;
                goto exit;
            }
            *pos++ = '\0';
            DPRINT_INFO(WFA_OUT, "ssid: %s\n", ssid);

            go_dev_addr = strstr(pos, "go_dev_addr=");
            if (go_dev_addr == NULL) {
                status = STATUS_ERROR;
                goto exit;
            }
            go_dev_addr += 12;
            go_dev_addr[WFA_P2P_DEVID_LEN - 1] = '\0';
            DPRINT_INFO(WFA_OUT, "go_dev_addr: %s\n", go_dev_addr);

            wSNPRINTF(infoResp->cmdru.grpFormInfo.result,
                    sizeof(infoResp->cmdru.grpFormInfo.result),
                    "%s",
                    strcmp(group_type, "GO") == 0 ? "GO" : "CLIENT");

            wSNPRINTF(infoResp->cmdru.grpFormInfo.grpId,
                    sizeof(infoResp->cmdru.grpFormInfo.grpId),
                    "%s %s",
                    go_dev_addr, ssid);
        }
    }

exit:
    if (ctrl) {
        wpa_ctrl_detach(ctrl);
        wpa_ctrl_close(ctrl);
    }
    infoResp->status = status;
    wfaEncodeTLV(WFA_STA_P2P_START_GRP_FORMATION_RESP_TLV,
            sizeof(dutCmdResponse_t), (BYTE *)infoResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

    return status;
}

/*
 * wfaStaP2pDissolve():
 */
int wfaStaP2pDissolve(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t *infoResp = &gGenericResp;
    caStaP2pDissolve_t *staP2pDissolve = (caStaP2pDissolve_t *)caCmdBuf;
    enum _response_staus status = STATUS_COMPLETE;

    DPRINT_INFO(WFA_OUT, "Entering %s()\n", __func__);

    if (wpa_command(staP2pDissolve->intf, "P2P_GROUP_REMOVE *") < 0) {
        status = STATUS_ERROR;
        goto exit;
    }

exit:
    infoResp->status = status;
    wfaEncodeTLV(WFA_STA_P2P_DISSOLVE_RESP_TLV, sizeof(dutCmdResponse_t),
            (BYTE *)infoResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

    return WFA_SUCCESS;
}

/*
 * wfaStaSendP2pInvReq():
 */
int wfaStaSendP2pInvReq(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t infoResp;
    /* caStaSendP2pInvReq_t *getStaP2pInvReq= (caStaSendP2pInvReq_t *)caCmdBuf; */

    DPRINT_INFO(WFA_OUT, "Entering %s()\n", __func__);

    // Implement the function and this does not return any thing back.

    infoResp.status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_STA_P2P_SEND_INV_REQ_RESP_TLV, sizeof(dutCmdResponse_t),
            (BYTE *)&infoResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

    return WFA_SUCCESS;
}


/*
 * wfaStaAcceptP2pInvReq():
 */
int wfaStaAcceptP2pInvReq(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t infoResp;
    /* uncomment and use it
     * caStaAcceptP2pInvReq_t *getStaP2pInvReq= (caStaAcceptP2pInvReq_t *)caCmdBuf;
     */

    DPRINT_INFO(WFA_OUT, "Entering %s()\n", __func__);

    // Implement the function and this does not return any thing back.

    infoResp.status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_STA_P2P_ACCEPT_INV_REQ_RESP_TLV, sizeof(dutCmdResponse_t),
            (BYTE *)&infoResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

    return WFA_SUCCESS;
}


/*
 * wfaStaSendP2pProvDisReq():
 */
int wfaStaSendP2pProvDisReq(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t *infoResp = &gGenericResp;
    caStaSendP2pProvDisReq_t *getStaP2pProvDisReq= (caStaSendP2pProvDisReq_t *)caCmdBuf;
    char buf[256];
    char *config_method;
    enum _response_staus status = STATUS_COMPLETE;

    DPRINT_INFO(WFA_OUT, "Entering %s()\n", __func__);

    if (strcasecmp(getStaP2pProvDisReq->confMethod, "Display") == 0) {
        config_method = "display";
    } else if (strcasecmp(getStaP2pProvDisReq->confMethod, "Keypad") == 0) {
        config_method = "keypad";
    } else if (strcasecmp(getStaP2pProvDisReq->confMethod, "Label") == 0) {
        config_method = "label";
    } else if (strcasecmp(getStaP2pProvDisReq->confMethod, "PushButton") == 0) {
        config_method = "pbc";
    } else {
        status = STATUS_ERROR;
        goto exit;
    }

    if (p2p_find_peer(getStaP2pProvDisReq->intf, getStaP2pProvDisReq->devId, 0) <= 0) {
        status = STATUS_ERROR;
        goto exit;
    }

    wSNPRINTF(buf, sizeof(buf), "P2P_PROV_DISC %s %s",
            getStaP2pProvDisReq->devId, config_method);
    if (wpa_command(getStaP2pProvDisReq->intf, buf) != 0) {
        status = STATUS_ERROR;
        goto exit;
    }

exit:
    infoResp->status = status;
    wfaEncodeTLV(WFA_STA_P2P_SEND_PROV_DIS_REQ_RESP_TLV, sizeof(dutCmdResponse_t),
            (BYTE *)infoResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

    return WFA_SUCCESS;
}

/*
 * wfaStaSetWpsPbc():
 */
int wfaStaSetWpsPbc(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t *infoResp = &gGenericResp;
    caStaSetWpsPbc_t *getStaSetWpsPbc= (caStaSetWpsPbc_t *)caCmdBuf;
    enum _response_staus status = STATUS_COMPLETE;

    DPRINT_INFO(WFA_OUT, "Entering %s()\n", __func__);

    if (getStaSetWpsPbc->grpid_flag) {
        if (wpa_command(getStaSetWpsPbc->intf, "WPS_PBC") < 0) {
            status = STATUS_ERROR;
            goto exit;
        }
    }
    gDut.p2p_params.wps_method = WPS_METHOD_PBC;

exit:
    infoResp->status = status;
    wfaEncodeTLV(WFA_STA_WPS_SETWPS_PBC_RESP_TLV, sizeof(dutCmdResponse_t),
            (BYTE *)infoResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

    return WFA_SUCCESS;
}

/*
 * wfaStaWpsReadPin():
 */
int wfaStaWpsReadPin(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t *infoResp = &gGenericResp;
    caStaWpsReadPin_t *getStaWpsReadPin= (caStaWpsReadPin_t *)caCmdBuf;
    enum _response_staus status = STATUS_COMPLETE;
    char buf[256];

    DPRINT_INFO(WFA_OUT, "Entering %s()\n", __func__);

    wSTRNCPY(gDut.p2p_params.wpsPin, "48120203", WFA_WPS_PIN_LEN);
    wSTRNCPY(infoResp->cmdru.wpsPin, gDut.p2p_params.wpsPin,
            WFA_WPS_PIN_LEN);

    if (getStaWpsReadPin->grpid_flag) {
        wSNPRINTF(buf, sizeof(buf), "WPS_PIN any %s", gDut.p2p_params.wpsPin);
        if (wpa_command(getStaWpsReadPin->intf, buf) < 0) {
            status = STATUS_ERROR;
            goto exit;
        }
    }
    gDut.p2p_params.wps_method = WPS_METHOD_PIN_DISPLAY;

exit:
    infoResp->status = status;
    wfaEncodeTLV(WFA_STA_WPS_READ_PIN_RESP_TLV, sizeof(dutCmdResponse_t),
            (BYTE *)infoResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

    return WFA_SUCCESS;
}

/*
 * wfaStaWpsReadLabel():
 */
int wfaStaWpsReadLabel(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t *infoResp = &gGenericResp;
    caStaWpsReadLabel_t *getStaWpsReadLabel= (caStaWpsReadLabel_t *)caCmdBuf;
    enum _response_staus status = STATUS_COMPLETE;
    char buf[256];

    DPRINT_INFO(WFA_OUT, "Entering %s()\n", __func__);

    wSTRNCPY(gDut.p2p_params.wpsPin, "48120203", WFA_WPS_PIN_LEN);
    wSTRNCPY(infoResp->cmdru.wpsPin, gDut.p2p_params.wpsPin,
            WFA_WPS_PIN_LEN);

    if (getStaWpsReadLabel->grpid_flag) {
        wSNPRINTF(buf, sizeof(buf), "WPS_PIN any %s", gDut.p2p_params.wpsPin);
        if (wpa_command(getStaWpsReadLabel->intf, buf) < 0) {
            status = STATUS_ERROR;
            goto exit;
        }
    }
    gDut.p2p_params.wps_method = WPS_METHOD_PIN_LABEL;

exit:
    infoResp->status = status;
    wfaEncodeTLV(WFA_STA_WPS_READ_PIN_RESP_TLV, sizeof(dutCmdResponse_t),
            (BYTE *)infoResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

    return WFA_SUCCESS;
}

/*
 * wfaStaWpsEnterPin():
 */
int wfaStaWpsEnterPin(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t *infoResp = &gGenericResp;
    caStaWpsEnterPin_t *getStaWpsEnterPin= (caStaWpsEnterPin_t *)caCmdBuf;
    enum _response_staus status = STATUS_COMPLETE;
    char buf[256];

    DPRINT_INFO(WFA_OUT, "Entering %s()\n", __func__);

    wSTRNCPY(gDut.p2p_params.wpsPin, getStaWpsEnterPin->wpsPin,
            WFA_WPS_PIN_LEN - 1);
    if (getStaWpsEnterPin->grpid_flag) {
        wSNPRINTF(buf, sizeof(buf), "WPS_PIN any %s", getStaWpsEnterPin->wpsPin);
        if (wpa_command(getStaWpsEnterPin->intf, buf) < 0) {
            status = STATUS_ERROR;
            goto exit;
        }
    }
    gDut.p2p_params.wps_method = WPS_METHOD_KEYPAD;

exit:
    infoResp->status = status;
    wfaEncodeTLV(WFA_STA_WPS_ENTER_PIN_RESP_TLV, sizeof(dutCmdResponse_t),
            (BYTE *)infoResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

    return WFA_SUCCESS;
}

/*
 * wfaStaGetPsk():
 */
int wfaStaGetPsk(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t *infoResp = &gGenericResp;
    caStaGetPsk_t *getStaGetPsk= (caStaGetPsk_t *)caCmdBuf;
    char passphrase[64];
    enum _response_staus status = STATUS_COMPLETE;

    DPRINT_INFO(WFA_OUT, "Entering %s()\n", __func__);

    if (wpa_command_resp(getStaGetPsk->intf, "P2P_GET_PASSPHRASE",
            passphrase, sizeof(passphrase)) < 0) {
        status = STATUS_ERROR;
        goto exit;
    }

    wSTRNCPY(infoResp->cmdru.pskInfo.ssid, gDut.p2p_params.current_ssid,
            WFA_SSID_NAME_LEN);
    wSTRNCPY(infoResp->cmdru.pskInfo.passPhrase, passphrase, WFA_PSK_PP_LEN);

exit:
    infoResp->status = status;
    wfaEncodeTLV(WFA_STA_P2P_GET_PSK_RESP_TLV, sizeof(dutCmdResponse_t),
            (BYTE *)infoResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

    return WFA_SUCCESS;
}

/*
 * wfaStaP2pReset():
 */
int wfaStaP2pReset(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t *infoResp = &gGenericResp;
    caStaWpsEnterPin_t *getStaP2pReset= (caStaWpsEnterPin_t *)caCmdBuf;
    enum _response_staus status = STATUS_COMPLETE;

    DPRINT_INFO(WFA_OUT, "Entering %s()\n", __func__);

    gDut.p2p_params.persistent = WFA_DISABLED;
    gDut.p2p_params.p2p_mode = P2P_MODE_IDLE;
    gDut.p2p_params.wps_method = WPS_METHOD_NONE;

    if (wpa_command(getStaP2pReset->intf, "P2P_GROUP_REMOVE *") < 0) {
        status = STATUS_ERROR;
        goto exit;
    }
    if (wpa_command(getStaP2pReset->intf, "P2P_STOP_FIND") < 0) {
        status = STATUS_ERROR;
        goto exit;
    }
    if (wpa_command(getStaP2pReset->intf, "P2P_FLUSH") < 0) {
        status = STATUS_ERROR;
        goto exit;
    }
    if (wpa_command(getStaP2pReset->intf, "P2P_SERVICE_FLUSH") < 0) {
        status = STATUS_ERROR;
        goto exit;
    }
    if (wpa_command(getStaP2pReset->intf, "P2P_SET disabled 0") < 0) {
        status = STATUS_ERROR;
        goto exit;
    }
    if (wpa_command(getStaP2pReset->intf, "P2P_SET ssid_postfix ") < 0) {
        status = STATUS_ERROR;
        goto exit;
    }
    if (wpa_command(getStaP2pReset->intf, "P2P_EXT_LISTEN") < 0) {
        status = STATUS_ERROR;
        goto exit;
    }
    if (wpa_command(getStaP2pReset->intf, "SET p2p_go_intent 7") < 0) {
        status = STATUS_ERROR;
        goto exit;
    }
    if (wpa_command(getStaP2pReset->intf, "P2P_SET go_apsd disable") < 0) {
        status = STATUS_ERROR;
        goto exit;
    }
    if (wpa_command(getStaP2pReset->intf, "SAVE_CONFIG") < 0) {
        status = STATUS_ERROR;
        goto exit;
    }

    if (shell("ifconfig %s 0.0.0.0", getStaP2pReset->intf) != WFA_SUCCESS) {
        DPRINT_WARNING(WFA_WNG, "set p2p interface down fail\n");
        status = STATUS_ERROR;
        goto exit;
    }

    remove_p2p_persistent_networks(getStaP2pReset->intf);

exit:
    infoResp->status = status;
    wfaEncodeTLV(WFA_STA_P2P_RESET_RESP_TLV, sizeof(dutCmdResponse_t),
            (BYTE *)infoResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

    return WFA_SUCCESS;
}

/*
 * wfaStaGetP2pIpConfig():
 */
int wfaStaGetP2pIpConfig(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t *infoResp = &gGenericResp;
    caStaGetIpConfigResp_t *ifinfo = &(infoResp->cmdru.getIfconfig);
    caStaGetP2pIpConfig_t *staGetP2pIpConfig= (caStaGetP2pIpConfig_t *)caCmdBuf;
    enum _response_staus status = STATUS_COMPLETE;
    char ip[30];
    int count;

    DPRINT_INFO(WFA_OUT, "Entering %s()\n", __func__);

    if (strlen(staGetP2pIpConfig->grpId) == 0) {
        DPRINT_WARNING(WFA_WNG, "group id is NULL\n");
        status = STATUS_ERROR;
        goto exit;
    }

    // default timeout to get ip: 120s
    count = 120;
    while (count > 0) {
        count--;
        if (get_wpa_status(staGetP2pIpConfig->intf, "ip_address", ip,
                sizeof(ip)) == 0 && strlen(ip) > 0) {
            DPRINT_INFO(WFA_OUT, "IP address %s\n", ip);
            break;
        }
        wSLEEP(1);
    }
    if (count == 0) {
        DPRINT_WARNING(WFA_WNG, "can NOT get ip address\n");
        status = STATUS_ERROR;
        goto exit;
    }

    if (get_ip_config(staGetP2pIpConfig->intf, ifinfo) == WFA_SUCCESS) {
        status = STATUS_COMPLETE;
    } else {
        status = STATUS_ERROR;
        goto exit;
    }
    // TODO: assume p2p always uses DHCP
    ifinfo->isDhcp = 1;

    DPRINT_INFO(WFA_OUT,
            "mac_addr: %s, ip: %s, mask: %s, dns1: %s, dns2: %s, isDhcp: %d\n",
            ifinfo->mac,
            ifinfo->ipaddr,
            ifinfo->mask,
            ifinfo->dns[0],
            ifinfo->dns[1],
            ifinfo->isDhcp);

exit:
    infoResp->status = status;
    wfaEncodeTLV(WFA_STA_P2P_GET_IP_CONFIG_RESP_TLV, sizeof(dutCmdResponse_t),
            (BYTE *)infoResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

    return WFA_SUCCESS;
}

/*
 * wfaStaSendServiceDiscoveryReq():
 */
int wfaStaSendServiceDiscoveryReq(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t infoResp;

    printf("\n Entry wfaStaSendServiceDiscoveryReq... ");
    // Implement the function and this does not return any thing back.


    infoResp.status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_STA_P2P_SEND_SERVICE_DISCOVERY_REQ_RESP_TLV, sizeof(infoResp), (BYTE *)&infoResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(infoResp);

    return WFA_SUCCESS;
}

/*
 * wfaStaSendP2pPresenceReq():
 */
int wfaStaSendP2pPresenceReq(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t infoResp;

    infoResp.status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_STA_P2P_SEND_PRESENCE_REQ_RESP_TLV, sizeof(infoResp), (BYTE *)&infoResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(infoResp);

    return WFA_SUCCESS;
}

/*
 * wfaStaSetSleepReq():
 */
int wfaStaSetSleepReq(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t *resp = &gGenericResp;
    caStaSetSleep_t *staSetSleepReq= (caStaSetSleep_t *) caCmdBuf;
    enum _response_staus status = STATUS_COMPLETE;

    DPRINT_INFO(WFA_OUT, "Entering %s()\n", __func__);

    if (wpa_command(staSetSleepReq->intf, "DRIVER p2p_set_sleep") != 0) {
        DPRINT_WARNING(WFA_WNG, "supplicant driver cmd (p2p_set_sleep) fail\n");
        status = STATUS_ERROR;
    }

    resp->status = status;
    wfaEncodeTLV(WFA_STA_P2P_SET_SLEEP_RESP_TLV, sizeof(dutCmdResponse_t),
            (BYTE *)resp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

    return WFA_SUCCESS;
}

/*
 * wfaStaSetOpportunisticPsReq():
 */
int wfaStaSetOpportunisticPsReq(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t infoResp;

    printf("\n Entry wfaStaSetOpportunisticPsReq... ");
    // Implement the function and this does not return any thing back.


    infoResp.status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_STA_P2P_SET_OPPORTUNISTIC_PS_RESP_TLV, sizeof(infoResp), (BYTE *)&infoResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + 4;

    return WFA_SUCCESS;
}
#ifndef WFA_STA_TB
/*
 * wfaStaPresetParams():
 */

int wfaStaPresetParams(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t infoResp;

    DPRINT_INFO(WFA_OUT, "Inside wfaStaPresetParameters function ...\n");

    // Implement the function and its sub commands
    infoResp.status = STATUS_COMPLETE;

    wfaEncodeTLV(WFA_STA_PRESET_PARAMETERS_RESP_TLV, 4, (BYTE *)&infoResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + 4;

    return WFA_SUCCESS;
}
int wfaStaSet11n(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{

    dutCmdResponse_t infoResp;
    dutCmdResponse_t *v11nParamsResp = &infoResp;

#ifdef WFA_11N_SUPPORT_ONLY

    caSta11n_t * v11nParams = (caSta11n_t *)caCmdBuf;

    int st =0; // SUCCESS

    DPRINT_INFO(WFA_OUT, "Inside wfaStaSet11n function....\n");

    if(v11nParams->addba_reject != 0xFF && v11nParams->addba_reject < 2)
    {
        // implement the funciton
        if(st != 0)
        {
            v11nParamsResp->status = STATUS_ERROR;
            strcpy(v11nParamsResp->cmdru.info, "set_addba_reject failed");
            wfaEncodeTLV(WFA_STA_SET_11N_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)v11nParamsResp, respBuf);
            *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);
            return FALSE;
        }
    }

    if(v11nParams->ampdu != 0xFF && v11nParams->ampdu < 2)
    {
        // implement the funciton

        if(st != 0)
        {
            v11nParamsResp->status = STATUS_ERROR;
            strcpy(v11nParamsResp->cmdru.info, "set_ampdu failed");
            wfaEncodeTLV(WFA_STA_SET_11N_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)v11nParamsResp, respBuf);
            *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);
            return FALSE;
        }
    }

    if(v11nParams->amsdu != 0xFF && v11nParams->amsdu < 2)
    {
        // implement the funciton
        if(st != 0)
        {
            v11nParamsResp->status = STATUS_ERROR;
            strcpy(v11nParamsResp->cmdru.info, "set_amsdu failed");
            wfaEncodeTLV(WFA_STA_SET_11N_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)v11nParamsResp, respBuf);
            *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);
            return FALSE;
        }
    }

    if(v11nParams->greenfield != 0xFF && v11nParams->greenfield < 2)
    {
        // implement the funciton
        if(st != 0)
        {
            v11nParamsResp->status = STATUS_ERROR;
            strcpy(v11nParamsResp->cmdru.info, "_set_greenfield failed");
            wfaEncodeTLV(WFA_STA_SET_11N_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)v11nParamsResp, respBuf);
            *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);
            return FALSE;
        }
    }

    if(v11nParams->mcs32!= 0xFF && v11nParams->mcs32 < 2 && v11nParams->mcs_fixedrate[0] != '\0')
    {
        // implement the funciton
        //st = wfaExecuteCLI(gCmdStr);
        if(st != 0)
        {
            v11nParamsResp->status = STATUS_ERROR;
            strcpy(v11nParamsResp->cmdru.info, "set_mcs failed");
            wfaEncodeTLV(WFA_STA_SET_11N_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)v11nParamsResp, respBuf);
            *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);
            return FALSE;
        }
    }
    else if (v11nParams->mcs32!= 0xFF && v11nParams->mcs32 < 2 && v11nParams->mcs_fixedrate[0] == '\0')
    {
        // implement the funciton
        //st = wfaExecuteCLI(gCmdStr);
        if(st != 0)
        {
            v11nParamsResp->status = STATUS_ERROR;
            strcpy(v11nParamsResp->cmdru.info, "set_mcs32 failed");
            wfaEncodeTLV(WFA_STA_SET_11N_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)v11nParamsResp, respBuf);
            *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);
            return FALSE;
        }
    }
    else if (v11nParams->mcs32 == 0xFF && v11nParams->mcs_fixedrate[0] != '\0')
    {
        // implement the funciton
        //st = wfaExecuteCLI(gCmdStr);
        if(st != 0)
        {
            v11nParamsResp->status = STATUS_ERROR;
            strcpy(v11nParamsResp->cmdru.info, "set_mcs32 failed");
            wfaEncodeTLV(WFA_STA_SET_11N_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)v11nParamsResp, respBuf);
            *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);
            return FALSE;
        }
    }

    if(v11nParams->rifs_test != 0xFF && v11nParams->rifs_test < 2)
    {
        // implement the funciton
        //st = wfaExecuteCLI(gCmdStr);
        if(st != 0)
        {
            v11nParamsResp->status = STATUS_ERROR;
            strcpy(v11nParamsResp->cmdru.info, "set_rifs_test failed");
            wfaEncodeTLV(WFA_STA_SET_11N_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)v11nParamsResp, respBuf);
            *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);
            return FALSE;
        }
    }

    if(v11nParams->sgi20 != 0xFF && v11nParams->sgi20 < 2)
    {
        // implement the funciton
        //st = wfaExecuteCLI(gCmdStr);
        if(st != 0)
        {
            v11nParamsResp->status = STATUS_ERROR;
            strcpy(v11nParamsResp->cmdru.info, "set_sgi20 failed");
            wfaEncodeTLV(WFA_STA_SET_11N_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)v11nParamsResp, respBuf);
            *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);
            return FALSE;
        }
    }

    if(v11nParams->smps != 0xFFFF)
    {
        if(v11nParams->smps == 0)
        {
            // implement the funciton
            //st = wfaExecuteCLI(gCmdStr);
        }
        else if(v11nParams->smps == 1)
        {
            // implement the funciton
            //st = wfaExecuteCLI(gCmdStr);
            ;
        }
        else if(v11nParams->smps == 2)
        {
            // implement the funciton
            //st = wfaExecuteCLI(gCmdStr);
            ;
        }
        if(st != 0)
        {
            v11nParamsResp->status = STATUS_ERROR;
            strcpy(v11nParamsResp->cmdru.info, "set_smps failed");
            wfaEncodeTLV(WFA_STA_SET_11N_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)v11nParamsResp, respBuf);
            *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);
            return FALSE;
        }
    }

    if(v11nParams->stbc_rx != 0xFFFF)
    {
        // implement the funciton
        //st = wfaExecuteCLI(gCmdStr);
        if(st != 0)
        {
            v11nParamsResp->status = STATUS_ERROR;
            strcpy(v11nParamsResp->cmdru.info, "set_stbc_rx failed");
            wfaEncodeTLV(WFA_STA_SET_11N_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)v11nParamsResp, respBuf);
            *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);
            return FALSE;
        }
    }

    if(v11nParams->width[0] != '\0')
    {
        // implement the funciton
        //st = wfaExecuteCLI(gCmdStr);
        if(st != 0)
        {
            v11nParamsResp->status = STATUS_ERROR;
            strcpy(v11nParamsResp->cmdru.info, "set_11n_channel_width failed");
            wfaEncodeTLV(WFA_STA_SET_11N_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)v11nParamsResp, respBuf);
            *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);
            return FALSE;
        }
    }

    if(v11nParams->_40_intolerant != 0xFF && v11nParams->_40_intolerant < 2)
    {
        // implement the funciton
        //st = wfaExecuteCLI(gCmdStr);
        if(st != 0)
        {
            v11nParamsResp->status = STATUS_ERROR;
            strcpy(v11nParamsResp->cmdru.info, "set_40_intolerant failed");
            wfaEncodeTLV(WFA_STA_SET_11N_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)v11nParamsResp, respBuf);
            *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);
            return FALSE;
        }
    }

    if(v11nParams->txsp_stream != 0 && v11nParams->txsp_stream <4)
    {
        // implement the funciton
        //st = wfaExecuteCLI(gCmdStr);
        if(st != 0)
        {
            v11nParamsResp->status = STATUS_ERROR;
            strcpy(v11nParamsResp->cmdru.info, "set_txsp_stream failed");
            wfaEncodeTLV(WFA_STA_SET_11N_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)v11nParamsResp, respBuf);
            *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);
            return FALSE;
        }

    }

    if(v11nParams->rxsp_stream != 0 && v11nParams->rxsp_stream < 4)
    {
        // implement the funciton
        //st = wfaExecuteCLI(gCmdStr);
        if(st != 0)
        {
            v11nParamsResp->status = STATUS_ERROR;
            strcpy(v11nParamsResp->cmdru.info, "set_rxsp_stream failed");
            wfaEncodeTLV(WFA_STA_SET_11N_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)v11nParamsResp, respBuf);
            *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);
            return FALSE;
        }
    }

#endif

    v11nParamsResp->status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_STA_SET_11N_RESP_TLV, 4, (BYTE *)v11nParamsResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + 4;
    return WFA_SUCCESS;
}
#endif
/*
 * wfaStaAddArpTableEntry():
 */
int wfaStaAddArpTableEntry(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t infoResp;
    /* caStaAddARPTableEntry_t *staAddARPTableEntry= (caStaAddARPTableEntry_t *)caCmdBuf; uncomment and use it */

    printf("\n Entry wfastaAddARPTableEntry... ");
    // Implement the function and this does not return any thing back.

    infoResp.status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_STA_P2P_ADD_ARP_TABLE_ENTRY_RESP_TLV, sizeof(infoResp), (BYTE *)&infoResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(infoResp);

    return WFA_SUCCESS;
}

/*
 * wfaStaBlockICMPResponse():
 */
int wfaStaBlockICMPResponse(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t infoResp;
    /* caStaBlockICMPResponse_t *staAddARPTableEntry= (caStaBlockICMPResponse_t *)caCmdBuf; uncomment and use it */

    printf("\n Entry wfaStaBlockICMPResponse... ");
    // Implement the function and this does not return any thing back.

    infoResp.status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_STA_P2P_BLOCK_ICMP_RESPONSE_RESP_TLV, sizeof(infoResp), (BYTE *)&infoResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(infoResp);

    return WFA_SUCCESS;
}

/*
 * wfaStaSetRadio():
 */

int wfaStaSetRadio(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCommand_t *setRadio = (dutCommand_t *)caCmdBuf;
    dutCmdResponse_t *staCmdResp = &gGenericResp;
    caStaSetRadio_t *sr = &setRadio->cmdsu.sr;

    DPRINT_INFO(WFA_OUT, "\nEntering wfaStaSetRadio ...\n");

    if(sr->mode == WFA_OFF)
    {
        // turn radio off
    }
    else
    {
        // always turn the radio on
    }

    staCmdResp->status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_STA_SET_RADIO_RESP_TLV, 4, (BYTE *)staCmdResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + 4;

    return WFA_SUCCESS;
}

/*
 * wfaStaSetRFeature():
 */

int wfaStaSetRFeature(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCommand_t *dutCmd = (dutCommand_t *)caCmdBuf;
    caStaRFeat_t *rfeat = &dutCmd->cmdsu.rfeat;
    dutCmdResponse_t *caResp = &gGenericResp;

    DPRINT_INFO(WFA_OUT, "\nEntering wfaStaSetRFeature ... prog=%s,cs=%s,uapsd=%d\n",
        rfeat->prog, rfeat->chswitchmode, rfeat->uapsd);

    if(str_same(rfeat->prog, "tdls")) {
        int offset = 0;

        if (str_same(rfeat->secchoffset, "20"))
            offset = 0;
        else if (str_same(rfeat->secchoffset, "40above"))
            offset = 1;
        else if (str_same(rfeat->secchoffset, "40below"))
            offset = 3;

        if (rfeat->uapsd == eEnable || rfeat->uapsd == eDisable) {
            shell(IWPRIV" %s set_power_mode %d", dutCmd->intf, (int)rfeat->uapsd);
        }
        if (str_same(rfeat->chswitchmode, "Initiate")) {
            /* for gen3/gen4 */
            shell(IWPRIV" %s driver \"set_chip tdls 1 %s %d 0 %d 0 0\"",
                dutCmd->intf, rfeat->peer, rfeat->offchnum, offset);

            /* for others */
            shell(IWPRIV" %s set_str_cmd 0_9_%s_0_1_1_12_%d_%d_1",
                dutCmd->intf, rfeat->peer, rfeat->offchnum, offset);
        } else {
            /* for gen3/gen4 */
            shell(IWPRIV" %s driver \"set_chip tdls 0 %s %d 0 %d 0 0\"",
                 dutCmd->intf, rfeat->peer, rfeat->offchnum, offset);

            /* for others */
           shell(IWPRIV" %s set_str_cmd 0_9_%s_0_1_0_0_%d_%d_0",
               dutCmd->intf, rfeat->peer, rfeat->offchnum, offset);
       }
    }

    caResp->status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_STA_SET_RFEATURE_RESP_TLV, 4, (BYTE *)caResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + 4;

    return WFA_SUCCESS;
}

/*
 * wfaStaStartWfdConnection():
 */
int wfaStaStartWfdConnection(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t infoResp;
    //caStaStartWfdConn_t *staStartWfdConn= (caStaStartWfdConn_t *)caCmdBuf; //uncomment and use it

    printf("\n Entry wfaStaStartWfdConnection... ");


    // Fetch the GrpId and WFD session and return
    strcpy(&infoResp.cmdru.wfdConnInfo.wfdSessionId[0], "1234567890");
    strcpy(&infoResp.cmdru.wfdConnInfo.p2pGrpId[0], "WIFI_DISPLAY");
    strcpy(&infoResp.cmdru.wfdConnInfo.result[0], "GO");

    infoResp.status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_STA_START_WFD_CONNECTION_RESP_TLV, sizeof(infoResp), (BYTE *)&infoResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(infoResp);

    return WFA_SUCCESS;
}
/*
 * wfaStaCliCommand():
 */

int wfaStaCliCommand(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    char cmdName[32];
    char *pcmdStr=NULL, *str;
    int  st = 1;
    char CmdStr[WFA_CMD_STR_SZ];
    FILE *wfaCliFd;
    char wfaCliBuff[64];
    char retstr[256];
    int CmdReturnFlag =0;
    char tmp[256];
    FILE * sh_pipe;
    caStaCliCmdResp_t infoResp;

    printf("\nEntry wfaStaCliCommand; command Received: %s\n",caCmdBuf);
    memcpy(cmdName, strtok_r((char *)caCmdBuf, ",", (char **)&pcmdStr), 32);
    sprintf(CmdStr, "%s",cmdName);

    for(;;)
    {
        // construct CLI standard cmd string
        str = strtok_r(NULL, ",", &pcmdStr);
        if(str == NULL || str[0] == '\0')
            break;
        else
        {
            sprintf(CmdStr, "%s /%s",CmdStr,str);
            str = strtok_r(NULL, ",", &pcmdStr);
            sprintf(CmdStr, "%s %s",CmdStr,str);
        }
    }
    // check the return process
    wfaCliFd=fopen("/etc/WfaEndpoint/wfa_cli.txt","r");
    if(wfaCliFd!= NULL)
    {
        while(fgets(wfaCliBuff, 64, wfaCliFd) != NULL)
        {
            //printf("\nLine read from CLI file : %s",wfaCliBuff);
            if(ferror(wfaCliFd))
                break;

            str=strtok(wfaCliBuff,"-");
            if(strcmp(str,cmdName) == 0)
            {
                str=strtok(NULL,",");
                if (str != NULL)
                {
                    if(strcmp(str,"TRUE") == 0)
                        CmdReturnFlag =1;
                }
                else
                    printf("ERR wfa_cli.txt, inside line format not end with , or missing TRUE/FALSE\n");
                break;
            }
        }
        fclose(wfaCliFd);
    }
    else
    {
        printf("/etc/WfaEndpoint/wfa_cli.txt is not exist\n");
        goto cleanup;
    }

    //printf("\n Command Return Flag : %d",CmdReturnFlag);
    memset(&retstr[0],'\0',255);
    memset(&tmp[0],'\0',255);
    sprintf(gCmdStr, "%s",  CmdStr);
    printf("\nCLI Command -- %s\n", gCmdStr);

    sh_pipe = popen(gCmdStr,"r");
    if(!sh_pipe)
    {
        printf ("Error in opening pipe\n");
        goto cleanup;
    }

    sleep(5);
    //tmp_val=getdelim(&retstr,255,"\n",sh_pipe);
    if (fgets(&retstr[0], 255, sh_pipe) == NULL)
    {
        printf("Getting NULL string in popen return\n");
        goto cleanup;
    }
    else
        printf("popen return str=%s\n",retstr);

    sleep(2);
    if(pclose(sh_pipe) == -1)
    {
        printf("Error in closing shell cmd pipe\n");
        goto cleanup;
    }
    sleep(2);

    // find status first in output
    str = strtok_r((char *)retstr, "-", (char **)&pcmdStr);
    if (str != NULL)
    {
        memset(tmp, 0, 10);
        memcpy(tmp, str,  2);
        printf("cli status=%s\n",tmp);
        if(strlen(tmp) > 0)
            st = atoi(tmp);
        else printf("Missing status code\n");
    }
    else
    {
        printf("wfaStaCliCommand no return code found\n");
    }
    infoResp.resFlag=CmdReturnFlag;

cleanup:

    switch(st)
    {
    case 0:
        infoResp.status = STATUS_COMPLETE;
        if (CmdReturnFlag)
        {
            if((pcmdStr != NULL) && (strlen(pcmdStr) > 0) )
            {
                memset(&(infoResp.result[0]),'\0',WFA_CLI_CMD_RESP_LEN-1);
                strncpy(&infoResp.result[0], pcmdStr ,(strlen(pcmdStr) < WFA_CLI_CMD_RESP_LEN ) ? strlen(pcmdStr) : (WFA_CLI_CMD_RESP_LEN-2) );
                printf("Return CLI result string to CA=%s\n", &(infoResp.result[0]));
            }
            else
            {
                strcpy(&infoResp.result[0], "No return string found\n");
            }
        }
        break;
    case 1:
        infoResp.status = STATUS_ERROR;
        break;
    case 2:
        infoResp.status = STATUS_INVALID;
        break;
    }

    wfaEncodeTLV(WFA_STA_CLI_CMD_RESP_TLV, sizeof(infoResp), (BYTE *)&infoResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(infoResp);

    printf("Exit from wfaStaCliCommand\n");
    return TRUE;

}
/*
 * wfaStaConnectGoStartWfd():
 */

int wfaStaConnectGoStartWfd(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t infoResp;
//  caStaConnectGoStartWfd_t *staConnecGoStartWfd= (caStaConnectGoStartWfd_t *)caCmdBuf; //uncomment and use it

    printf("\n Entry wfaStaConnectGoStartWfd... ");

    // connect the specified GO and then establish the wfd session

    // Fetch WFD session and return
    strcpy(&infoResp.cmdru.wfdConnInfo.wfdSessionId[0], "1234567890");

    infoResp.status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_STA_CONNECT_GO_START_WFD_RESP_TLV, sizeof(infoResp), (BYTE *)&infoResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(infoResp);

    return WFA_SUCCESS;
}

/*
 * wfaStaGenerateEvent():
 */

int wfaStaGenerateEvent(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t infoResp;
    caStaGenEvent_t *staGenerateEvent= (caStaGenEvent_t *)caCmdBuf; //uncomment and use it
    caWfdStaGenEvent_t *wfdGenEvent;

    printf("\n Entry wfaStaGenerateEvent... ");


    // Geneate the specified action and return with complete/error.
    if(staGenerateEvent->program == PROG_TYPE_WFD)
    {
        wfdGenEvent = &staGenerateEvent->wfdEvent;
        if(wfdGenEvent ->type == eUibcGen)
        {
        }
        else if(wfdGenEvent ->type == eUibcHid)
        {
        }
        else if(wfdGenEvent ->type == eFrameSkip)
        {

        }
        else if(wfdGenEvent ->type == eI2cRead)
        {
        }
        else if(wfdGenEvent ->type == eI2cWrite)
        {
        }
        else if(wfdGenEvent ->type == eInputContent)
        {
        }
        else if(wfdGenEvent ->type == eIdrReq)
        {
        }
    }

    infoResp.status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_STA_GENERATE_EVENT_RESP_TLV, sizeof(infoResp), (BYTE *)&infoResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(infoResp);

    return WFA_SUCCESS;
}




/*
 * wfaStaReinvokeWfdSession():
 */

int wfaStaReinvokeWfdSession(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t infoResp;
//  caStaReinvokeWfdSession_t *staReinvokeSession= (caStaReinvokeWfdSession_t *)caCmdBuf; //uncomment and use it

    printf("\n Entry wfaStaReinvokeWfdSession... ");

    // Reinvoke the WFD session by accepting the p2p invitation   or sending p2p invitation


    infoResp.status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_STA_REINVOKE_WFD_SESSION_RESP_TLV, sizeof(infoResp), (BYTE *)&infoResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(infoResp);

    return WFA_SUCCESS;
}


int wfaStaGetParameter(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t infoResp;
    caStaGetParameter_t *staGetParam= (caStaGetParameter_t *)caCmdBuf; //uncomment and use it


    caStaGetParameterResp_t *paramList = &infoResp.cmdru.getParamValue;

    printf("\n Entry wfaStaGetParameter... ");

    // Check the program type
    if(staGetParam->program == PROG_TYPE_WFD)
    {
        if(staGetParam->getParamValue == eDiscoveredDevList )
        {
            // Get the discovered devices, make space seperated list and return, check list is not bigger than 128 bytes.
            paramList->getParamType = eDiscoveredDevList;
            strcpy((char *)&paramList->devList, "11:22:33:44:55:66 22:33:44:55:66:77 33:44:55:66:77:88");
        }
    }

    if(staGetParam->program == PROG_TYPE_WFDS)
    {

        if(staGetParam->getParamValue == eDiscoveredDevList )
        {
            // Get the discovered devices, make space seperated list and return, check list is not bigger than 128 bytes.
            paramList->getParamType = eDiscoveredDevList;
            strcpy((char *)&paramList->devList, "11:22:33:44:55:66 22:33:44:55:66:77 33:44:55:66:77:88");

        }
        if(staGetParam->getParamValue == eOpenPorts)
        {
            // Run the port checker tool
            // Get all the open ports and make space seperated list and return, check list is not bigger than 128 bytes.
            paramList->getParamType = eOpenPorts;
            strcpy((char *)&paramList->devList, "22 139 445 68 9700");

        }

    }
    if(staGetParam->program == PROG_TYPE_NAN)
    {
      if(staGetParam->getParamValue == eMasterPref )
      {
          // Get the master preference of the device and return the value
          paramList->getParamType = eMasterPref;
          strcpy((char *)&paramList->masterPref, "0xff");
      }
    }

    infoResp.status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_STA_GET_PARAMETER_RESP_TLV, sizeof(infoResp), (BYTE *)&infoResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(infoResp);

   return WFA_SUCCESS;
}


int wfaStaNfcAction(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t *resp = &gGenericResp;

    DPRINT_INFO(WFA_OUT, "Entering %s(), Not support now, return error.\n", __func__);

    resp->status = STATUS_ERROR;
    wfaEncodeTLV(WFA_STA_NFC_ACTION_RESP_TLV, sizeof(dutCmdResponse_t),
            (BYTE *)resp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

    return WFA_SUCCESS;
}

int wfaStaExecAction(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{

    dutCmdResponse_t infoResp;
    caStaExecAction_t *staExecAction = (caStaExecAction_t *)caCmdBuf;  //comment if not used

     printf("\n Entry wfaStaExecAction... ");

    if(staExecAction->prog == PROG_TYPE_NAN)
    {
        // Perform necessary configurations and actions
        // return the MAC address conditionally as per CAPI specification
    }

    infoResp.status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_STA_EXEC_ACTION_RESP_TLV, sizeof(infoResp), (BYTE *)&infoResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(infoResp);

   return WFA_SUCCESS;
}

int wfaStaInvokeCommand(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{

    dutCmdResponse_t infoResp;
    caStaInvokeCmd_t *staInvokeCmd = (caStaInvokeCmd_t *)caCmdBuf;  //uncomment and use it

     printf("\n Entry wfaStaInvokeCommand... ");


     // based on the command type , invoke API or complete the required procedures
     // return the  defined parameters based on the command that is received ( example response below)

    if(staInvokeCmd->cmdType == ePrimitiveCmdType && staInvokeCmd->InvokeCmds.primtiveType.PrimType == eCmdPrimTypeAdvt )
    {
         infoResp.cmdru.staInvokeCmd.invokeCmdRspType = eCmdPrimTypeAdvt;
         infoResp.cmdru.staInvokeCmd.invokeCmdResp.advRsp.numServInfo = 1;
         strcpy(infoResp.cmdru.staInvokeCmd.invokeCmdResp.advRsp.servAdvInfo[0].servName,"org.wi-fi.wfds.send.rx");
         infoResp.cmdru.staInvokeCmd.invokeCmdResp.advRsp.servAdvInfo[0].advtID = 0x0000f;
         strcpy(infoResp.cmdru.staInvokeCmd.invokeCmdResp.advRsp.servAdvInfo[0].serviceMac,"ab:cd:ef:gh:ij:kl");
    }
    else if (staInvokeCmd->cmdType == ePrimitiveCmdType && staInvokeCmd->InvokeCmds.primtiveType.PrimType == eCmdPrimTypeSeek)
    {
        infoResp.cmdru.staInvokeCmd.invokeCmdRspType = eCmdPrimTypeSeek;
        infoResp.cmdru.staInvokeCmd.invokeCmdResp.seekRsp.searchID = 0x000ff;
    }
    else if (staInvokeCmd->cmdType == ePrimitiveCmdType && staInvokeCmd->InvokeCmds.primtiveType.PrimType == eCmdPrimTypeConnSession)
    {
        infoResp.cmdru.staInvokeCmd.invokeCmdRspType = eCmdPrimTypeConnSession;
        infoResp.cmdru.staInvokeCmd.invokeCmdResp.connSessResp.sessionID = 0x000ff;
        strcpy(infoResp.cmdru.staInvokeCmd.invokeCmdResp.connSessResp.result,"GO");
        strcpy(infoResp.cmdru.staInvokeCmd.invokeCmdResp.connSessResp.grpId,"DIRECT-AB WFADUT");

    }
    infoResp.status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_STA_INVOKE_CMD_RESP_TLV, sizeof(infoResp), (BYTE *)&infoResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(infoResp);

   return WFA_SUCCESS;
}


int wfaStaManageService(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{

    dutCmdResponse_t infoResp;
    //caStaMngServ_t *staMngServ = (caStaMngServ_t *)caCmdBuf;  //uncomment and use it

     printf("\n Entry wfaStaManageService... ");

    // based on the manage service type , invoke API's or complete the required procedures
    // return the  defined parameters based on the command that is received ( example response below)
    strcpy(infoResp.cmdru.staManageServ.result, "CLIENT");
    strcpy(infoResp.cmdru.staManageServ.grpId, "AA:BB:CC:DD:EE:FF_DIRECT-SSID");
    infoResp.cmdru.staManageServ.sessionID = 0x000ff;

    infoResp.status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_STA_MANAGE_SERVICE_RESP_TLV, sizeof(infoResp), (BYTE *)&infoResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(infoResp);

   return WFA_SUCCESS;
}



int wfaStaGetEvents(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{

    dutCmdResponse_t infoResp;
    caStaGetEvents_t *staGetEvents = (caStaGetEvents_t *)caCmdBuf;  //uncomment and use it

     printf("\n Entry wfaStaGetEvents... ");

     if(staGetEvents->program == PROG_TYPE_NAN)
    {
        // Get all the events from the Log file or stored events
        // return the  received/recorded event details - eventName, remoteInstanceID, localInstanceID, mac
    }

    // Get all the event from the Log file or stored events
    // return the  received/recorded events as space seperated list   ( example response below)
    strcpy(infoResp.cmdru.staGetEvents.result, "SearchResult SearchTerminated AdvertiseStatus SessionRequest ConnectStatus SessionStatus PortStatus");

    infoResp.status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_STA_GET_EVENTS_RESP_TLV, sizeof(infoResp), (BYTE *)&infoResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(infoResp);

   return WFA_SUCCESS;
}

int wfaStaGetEventDetails(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{

    dutCmdResponse_t infoResp;
    caStaGetEventDetails_t *getStaGetEventDetails = (caStaMngServ_t *)caCmdBuf;  //uncomment and use it

     printf("\n Entry wfaStaGetEventDetails... ");


     // based on the Requested Event type
     // return the latest corresponding evnet detailed parameters  ( example response below)

    if(getStaGetEventDetails->eventId== eSearchResult )
    {
        // fetch from log file or event history for the search result event and return the parameters
        infoResp.cmdru.staGetEventDetails.eventID= eSearchResult;

        infoResp.cmdru.staGetEventDetails.getEventDetails.searchResult.searchID = 0x00abcd;
        strcpy(infoResp.cmdru.staGetEventDetails.getEventDetails.searchResult.serviceMac,"ab:cd:ef:gh:ij:kl");
        infoResp.cmdru.staGetEventDetails.getEventDetails.searchResult.advID = 0x00dcba;
        strcpy(infoResp.cmdru.staGetEventDetails.getEventDetails.searchResult.serviceName,"org.wi-fi.wfds.send.rx");

        infoResp.cmdru.staGetEventDetails.getEventDetails.searchResult.serviceStatus = eServiceAvilable;
    }
    else if (getStaGetEventDetails->eventId == eSearchTerminated)
    {       // fetch from log file or event history for the search terminated event and return the parameters
        infoResp.cmdru.staGetEventDetails.eventID= eSearchTerminated;
        infoResp.cmdru.staGetEventDetails.getEventDetails.searchTerminated.searchID = 0x00abcd;
    }
    else if (getStaGetEventDetails->eventId == eAdvertiseStatus)
    {// fetch from log file or event history for the Advertise Status event and return the parameters
        infoResp.cmdru.staGetEventDetails.eventID= eAdvertiseStatus;
        infoResp.cmdru.staGetEventDetails.getEventDetails.advStatus.advID = 0x00dcba;

        infoResp.cmdru.staGetEventDetails.getEventDetails.advStatus.status = eAdvertised;
    }
    else if (getStaGetEventDetails->eventId == eSessionRequest)
    {// fetch from log file or event history for the session request event and return the parameters
        infoResp.cmdru.staGetEventDetails.eventID= eSessionRequest;
        infoResp.cmdru.staGetEventDetails.getEventDetails.sessionReq.advID = 0x00dcba;
        strcpy(infoResp.cmdru.staGetEventDetails.getEventDetails.sessionReq.sessionMac,"ab:cd:ef:gh:ij:kl");
        infoResp.cmdru.staGetEventDetails.getEventDetails.sessionReq.sessionID = 0x00baba;
    }
    else if (getStaGetEventDetails->eventId ==eSessionStatus )
    {// fetch from log file or event history for the session status event and return the parameters
        infoResp.cmdru.staGetEventDetails.eventID= eSessionStatus;
        infoResp.cmdru.staGetEventDetails.getEventDetails.sessionStatus.sessionID = 0x00baba;
        strcpy(infoResp.cmdru.staGetEventDetails.getEventDetails.sessionStatus.sessionMac,"ab:cd:ef:gh:ij:kl");
        infoResp.cmdru.staGetEventDetails.getEventDetails.sessionStatus.state = eSessionStateOpen;
    }
    else if (getStaGetEventDetails->eventId == eConnectStatus)
    {
        infoResp.cmdru.staGetEventDetails.eventID= eConnectStatus;
        infoResp.cmdru.staGetEventDetails.getEventDetails.connStatus.sessionID = 0x00baba;
        strcpy(infoResp.cmdru.staGetEventDetails.getEventDetails.connStatus.sessionMac,"ab:cd:ef:gh:ij:kl");
        infoResp.cmdru.staGetEventDetails.getEventDetails.connStatus.status = eGroupFormationComplete;

    }
    else if (getStaGetEventDetails->eventId == ePortStatus)
    {
        infoResp.cmdru.staGetEventDetails.eventID= ePortStatus;
        infoResp.cmdru.staGetEventDetails.getEventDetails.portStatus.sessionID = 0x00baba;
        strcpy(infoResp.cmdru.staGetEventDetails.getEventDetails.portStatus.sessionMac,"ab:cd:ef:gh:ij:kl");
        infoResp.cmdru.staGetEventDetails.getEventDetails.portStatus.port = 1009;
        infoResp.cmdru.staGetEventDetails.getEventDetails.portStatus.status = eLocalPortAllowed;
    }



    infoResp.status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_STA_GET_EVENT_DETAILS_RESP_TLV, sizeof(infoResp), (BYTE *)&infoResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(infoResp);

   return WFA_SUCCESS;
}

int wfaStaScan(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    caStaScan_t *staScan= (caStaScan_t *)caCmdBuf;
    dutCmdResponse_t *staScanResp = &gGenericResp;
    char buf[256];
    char *bssid = NULL, *ssid = NULL;
    char ssid_hex[65];

    DPRINT_INFO(WFA_OUT, "\nEntering wfaStaScan ...\n");

    if((staScan->hessid[0] != '\0')){
    wSNPRINTF(buf, sizeof(buf), "SET hessid %s", staScan->hessid);
        if (wpa_command(staScan->intf, buf) < 0) {
            staScanResp->status = STATUS_ERROR;
            goto done;
        }
    }

    if((staScan->accs_net_type[0] != '\0')){
        wSNPRINTF(buf, sizeof(buf), "SET access_network_type %s", staScan->accs_net_type);
        if (wpa_command(staScan->intf, buf) < 0) {
            staScanResp->status = STATUS_ERROR;
            goto done;
        }
    }

    if((staScan->ssid[0] != '\0')){
        if (2 * strlen(staScan->ssid) >= sizeof(ssid_hex)) {
            staScanResp->status = STATUS_INVALID;
            goto done;
        }
        ssid = staScan->ssid;
        ascii2hexstr(ssid, ssid_hex);
    }

    if((staScan->bssid[0] != '\0')){
        bssid = staScan->bssid;
    }

    wSNPRINTF(buf, sizeof(buf), "SCAN%s%s%s%s",
        bssid ? " bssid=": "", bssid ? bssid : "",
        ssid ? " ssid " : "",  ssid ? ssid_hex : "");

    if (wpa_command(staScan->intf, buf) < 0) {
        staScanResp->status = STATUS_ERROR;
        goto done;
    }

    staScanResp->status = STATUS_COMPLETE;
done:
    wfaEncodeTLV(WFA_STA_SCAN_RESP_TLV, 4, (BYTE *)staScanResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + 4;

    return WFA_SUCCESS;
}

/**
 * Internal utilities
 */

int ascii2hexstr(const char *str, char *hex)
{
    int i, length;

    length = strlen(str);

    for (i = 0; i < length; i++)
        snprintf(hex + i * 2, 3, "%X", str[i]);

    hex[length * 2] = '\0';
    return 1;
}

int channel2freq(char channel)
{
    if (channel > 0 && channel <= 14)
        return channel * 5 + 2407;
    if (channel >= 36 && channel <= 161)
        return 5000 + 5 * channel;
    return 0;
}

const char* prog2str(int prog) {
    switch (prog) {
    case PROG_TYPE_PMF: return "PMF"; break;
    case PROG_TYPE_TDLS: return "TDLS"; break;
    case PROG_TYPE_VENT: return "VOE"; break;
    case PROG_TYPE_WFD: return "WFD"; break;
    case PROG_TYPE_WFDS: return "WFDS"; break;
    case PROG_TYPE_HS2: return "HS2"; break;
    case PROG_TYPE_HS2_R2: return "HS2-R2"; break;
    case PROG_TYPE_NAN: return "NAN"; break;
    case PROG_TYPE_LOC: return "LOC"; break;
    default: return "General"; break;
    }
}

int str2prog(const char *str) {
    if (str_same(str, "PMF") == 0)
        return PROG_TYPE_PMF;
    else if (str_same(str, "TDLS") == 0)
        return PROG_TYPE_TDLS;
    else if (str_same(str, "VOE") == 0)
        return PROG_TYPE_VENT;
    else if (str_same(str, "WFD") == 0)
        return PROG_TYPE_WFD;
    else if (str_same(str, "WFDS") == 0)
        return PROG_TYPE_WFDS;
    else if (str_same(str, "HS2") == 0)
        return PROG_TYPE_HS2;
    else if (str_same(str, "HS2-R2") == 0)
        return PROG_TYPE_HS2_R2;
    else if (str_same(str, "NAN") == 0)
        return PROG_TYPE_NAN;
    else if (str_same(str, "LOC") == 0)
        return PROG_TYPE_LOC;
    else
        return PROG_TYPE_GEN;
}

const char* encpType2str(int encpType) {
    if (encpType == ENCRYPT_TKIP)
        return "tkip";
    else if (encpType == ENCRYPT_AESCCMP)
        return "aes-ccmp";
    else if (encpType == ENCRYPT_AESCCMP_TKIP)
        return "aes-ccmp-tkip";

    DPRINT_ERR(WFA_ERR, "Unknown encpType=%d\n", encpType);
    return "UNKNOWN";
}

int str2encpType(const char *str) {

    if(str_same(str, "tkip"))
        return ENCRYPT_TKIP;
    else if(str_same(str, "aes-ccmp"))
        return ENCRYPT_AESCCMP;
    else if (str_same(str, "aes-ccmp-tkip"))
        return ENCRYPT_AESCCMP_TKIP;

    return ENCRYPT_NONE;
}

int chk_exist(const char *path)
{
    if (path && access(path, F_OK) != -1) {
        DPRINT_INFO(WFA_OUT, "%s exist.\n", path);
        return 1;
    } else {
        DPRINT_ERR(WFA_ERR, "%s doesn't exist!!\n", path);
        return 0;
    }
}

void wfa_cs_init(void)
{
    const char *path[] = {IWPRIV, DHCPCD, SETIPCONFIG, GETIPCONFIG,
            GETPID, GETPSTATS, STOPPING, UPDATEPID, WFAPING, WFAPING6};
    int count = sizeof(path) / sizeof(*path);
    int i;
    caStaSetIpConfig_t *ipconfig = &gDut.sta_params.ipconfig;

    DPRINT_INFO(WFA_OUT, "\nEntering wfa_cs_init ...\n");

    wMEMSET(&gDut, 0, sizeof(gDut));
    ipconfig->isDhcp = 1;
    wSPRINTF(ipconfig->intf, WFA_STAUT_IF);

    /*
     * check toolkits before use them
     */
    for (i = 0; i < count; i++) {
        chk_exist(path[i]);
    }

    if (shell("getprop vendor.wifi.sigma.mode") == WFA_SUCCESS) {
        DPRINT_INFO(WFA_OUT, "vendor.wifi.sigma.mode=%s\n", gDut.shellResult);
        count = sizeof(sigma_mode_tbl) / sizeof(*sigma_mode_tbl);
        for (i = 0; i < count; i++) {
            if (str_same(gDut.shellResult, sigma_mode_tbl[i])) {
                sigma_mode = i;
                break;
            }
        }
    }

    init_driver_gen();
#ifdef CONFIG_MTK_AP
    init_ap_params();
#endif
#ifdef CONFIG_MTK_P2P
    init_p2p_params();
#endif
}

int shell(const char *fmt, ...)
{
    va_list params;
    FILE *filep;
    size_t len = 0;
    size_t remain_len = sizeof(gDut.shellResult);
    char *result = gDut.shellResult;

    va_start(params, fmt);
    vsnprintf(gCmdStr, sizeof(gCmdStr), fmt, params);
    va_end(params);

    DPRINT_INFO(WFA_OUT, "shell: %s\n", gCmdStr);

    if((filep=popen(gCmdStr, "r")) != NULL)
    {
        wMEMSET(gDut.shellResult, 0, remain_len);
        while (fgets(result, remain_len, filep) != NULL)
        {
            len = strlen(result);
            result += len;
            remain_len -= len;
        }
        pclose(filep);
        while (*(--result) == '\n' || *result == '\r');
        *(result + 1) = 0;
    } else {
        DPRINT_ERR(WFA_ERR, "shell popen error");
        return WFA_ERROR;
    }
    return WFA_SUCCESS;
}

int check_connection(const char *ifname)
{
    char result[32];
    int ret;

    ret = get_wpa_status(ifname, "wpa_state", result, sizeof(result));
    if (ret >= 0) {
        DPRINT_INFO(WFA_OUT, "wpa_state=%s\n", result);
        if (wSTRNCMP(result, "COMPLETED", 9) == 0)
            return 1;
    } else {
        DPRINT_ERR(WFA_ERR, "get_wpa_status field=\"wpa_state\" ret=%d\n", ret);
    }
    return 0;
}

void update_ip_config(caStaSetIpConfig_t *ipconfig)
{
    caStaSetIpConfig_t *cfg = &gDut.sta_params.ipconfig;
    int connected = 0;

    DPRINT_INFO(WFA_OUT, "inside update_ip_config ...\n");

    // copy if new ipconfig
    if (ipconfig)
        wMEMCPY(cfg, ipconfig, sizeof(caStaSetIpConfig_t));

    // get connection status
    connected = check_connection(cfg->intf);

    // Set ipconfig after association
    if(cfg->isDhcp) {
        if (connected) {
            shell(DHCPCD" -t5 %s", cfg->intf);
            DPRINT_INFO(WFA_OUT, "dhcpcd Request DHCP IP DONE!\n");
        }
    } else {
        if (connected) {
            gDut.ipconfigDeferred = 0;
            shell(SETIPCONFIG" %s %s %s %s %s", cfg->intf,
                    cfg->ipaddr, cfg->mask, cfg->pri_dns, cfg->sec_dns);
            if(cfg->defGateway[0] != '\0') {
                shell("ip route add default gw %s", cfg->defGateway);
            }
        } else {
            // Deffered ipconfig before association
            gDut.ipconfigDeferred = 1;
            DPRINT_INFO(WFA_OUT, "The preset ip[%s] netmask[%s]\n", cfg->ipaddr, cfg->mask);
        }
    }
}

void remove_wpa_networks(const char *ifname)
{
    char buf[4096];
    char cmd[256];
    char *pos;

    DPRINT_INFO(WFA_OUT, "inside remove_wpa_networks ...\n");

    if (wpa_command_resp(ifname, "LIST_NETWORKS", buf, sizeof(buf)) < 0)
        return;

    /* Skip the first line (header) */
    pos = strchr(buf, '\n');
    if (pos == NULL)
        return;
    pos++;
    while (pos && pos[0]) {
        int id = atoi(pos);
        snprintf(cmd, sizeof(cmd), "REMOVE_NETWORK %d", id);
        wpa_command(ifname, cmd);
        pos = strchr(pos, '\n');
        if (pos)
            pos++;
    }
}

int find_network(const char *ifname, const char *ssid)
{
    char list[4096];
    char *pos;

    DPRINT_INFO(WFA_OUT, "inside find_network ... ssid=%s\n", ssid);

    if (wpa_command_resp(ifname, "LIST_NETWORKS", list, sizeof(list)) < 0)
        return -1;
    DPRINT_INFO(WFA_OUT, "%s", list);
    pos = strstr(list, ssid);
    if (!pos || pos == list || pos[strlen(ssid)] != '\t')
        return -1;

    while (pos > list && pos[-1] != '\n')
        pos--;
    gDut.networkId = atoi(pos);
    wSPRINTF(gDut.ssid, "%s", ssid);
    return 0;
}

void reset_param() {
    wMEMSET(&gDut.sta_params, 0, sizeof(gDut.sta_params));
    gDut.sta_params.type[0] = UNSET_VALUE;
    gDut.sta_params.pmf[0] = WFA_INVALID_BOOL;
    gDut.sta_params.akmSuiteType[0] = UNSET_VALUE;
    gDut.sta_params.owe[0] = UNSET_VALUE;
    gDut.sta_params.pmksaCaching[0] = UNSET_VALUE;
    gDut.sta_params.profile[0] = UNSET_VALUE;
    gDut.sta_params.peapVersion[0] = UNSET_VALUE;
    gDut.sta_params.prefer[0] = UNSET_VALUE;
}

void *get_param(const char* name)
{
    sta_params_t *params = &gDut.sta_params;

#define GET_PARAM_V(x, v) \
    if (str_same(name, #x)) { \
        if (params->x[0] != v) \
            return params->x; \
        else \
            return NULL; \
    }

#define GET_PARAM(x) GET_PARAM_V(x, 0)

    GET_PARAM(intf);
    GET_PARAM_V(type, UNSET_VALUE);
    GET_PARAM(ssid);
    GET_PARAM(keyMgmtType);
    GET_PARAM(encpType);
    GET_PARAM_V(pmf, WFA_INVALID_BOOL);
    GET_PARAM_V(peapVersion, UNSET_VALUE);
    GET_PARAM_V(akmSuiteType, UNSET_VALUE);
    GET_PARAM(clientCertificate);
    GET_PARAM(certType);
    GET_PARAM(ecGroupID);
    GET_PARAM(groupCipher);
    GET_PARAM(groupMgntCipher);
    GET_PARAM(innerEAP);
    GET_PARAM(invalidSAEElement);
    GET_PARAM(networkMode);
    GET_PARAM_V(owe, UNSET_VALUE);
    GET_PARAM(pacFile);
    GET_PARAM(pairwiseCipher);
    GET_PARAM(passphrase);
    GET_PARAM(password);
    GET_PARAM_V(pmksaCaching, UNSET_VALUE);
    GET_PARAM_V(profile, UNSET_VALUE);
    GET_PARAM(prog);
    GET_PARAM(trustedRootCA);
    GET_PARAM(tlsCipher);
    GET_PARAM(username);
    GET_PARAM_V(peapVersion, UNSET_VALUE);
    GET_PARAM_V(prefer, UNSET_VALUE);
    GET_PARAM(micAlg);

    //DPRINT_ERR(WFA_ERR, "Can't get_param %s\n", name);

#undef GET_PARAM
    return NULL;
}

char *get_param_str(const char* name) {
        return (char*) get_param(name);
}

int *get_param_val(const char* name) {
        return (int*) get_param(name);
}

void set_param(const char* name, void *value, unsigned int size)
{
    sta_params_t *params = &gDut.sta_params;

#define SET_PARAM(x) \
        if (str_same(name, #x) && size <= sizeof(params->x)) { \
            wMEMCPY(params->x, value, size); \
            return; \
        }

    SET_PARAM(intf);
    SET_PARAM(type);
    SET_PARAM(ssid);
    SET_PARAM(keyMgmtType);
    SET_PARAM(encpType);
    SET_PARAM(pmf);
    SET_PARAM(peapVersion);
    SET_PARAM(akmSuiteType);
    SET_PARAM(clientCertificate);
    SET_PARAM(certType);
    SET_PARAM(ecGroupID);
    SET_PARAM(groupCipher);
    SET_PARAM(groupMgntCipher);
    SET_PARAM(innerEAP);
    SET_PARAM(invalidSAEElement);
    SET_PARAM(networkMode);
    SET_PARAM(owe);
    SET_PARAM(pacFile);
    SET_PARAM(pairwiseCipher);
    SET_PARAM(passphrase);
    SET_PARAM(password);
    SET_PARAM(pmksaCaching);
    SET_PARAM(profile);
    SET_PARAM(prog);
    SET_PARAM(trustedRootCA);
    SET_PARAM(tlsCipher);
    SET_PARAM(username);
    SET_PARAM(peapVersion);
    SET_PARAM(prefer);
    SET_PARAM(micAlg);

    DPRINT_ERR(WFA_ERR, "Can't set_param %s\n", name);

#undef GET_PARAM
}

void set_param_val(const char* name, int value) {
    set_param(name, &value, sizeof(int));
}

void set_param_str(const char* name, char *value) {
    //    DPRINT_INFO(WFA_OUT, "set_param_str %s=[%s] %d\n", name, value, strlen(value));
    set_param(name, value, strlen(value) + 1);
}

int add_network_common(const char *ifname)
{
    const char *ssid = get_param_str("ssid");
    int id;
    const char *val;

    if (ssid == NULL) {
        DPRINT_ERR(WFA_ERR, "add_network error!! ifname=%s,ssid=%s...\n", ifname, ssid);
        return -2;
    }

    // remove all networks first
    //remove_wpa_networks(ifname);

    id = add_network(ifname);
    if (id < 0) {
        DPRINT_ERR(WFA_ERR, "add_network error!! ifname=%s ...\n", ifname);
        return -2;
    }

    if (set_network_quoted(ifname, id, "ssid", ssid) < 0) {
        DPRINT_ERR(WFA_ERR, "add_network error!! ssid=%s ...\n", ssid);
        return -2;
    }

    gDut.networkId = id;
    wSPRINTF(gDut.ssid, "%s", ssid);

    val = get_param_str("prog");
    if (str_same(val, "hs2")) {
        char buf[100];

        wSPRINTF(buf,  "ENABLE_NETWORK %d no-connect", id);
        wpa_command(ifname, buf);

        val = get_param_val("prefer");
        if (val && *val > 0)
            set_network(ifname, id, "priority", "1");
    }

    DPRINT_INFO(WFA_OUT, "add_network_common id=%d ssid=%s ...\n", id, ssid);
    return id;
}

int str_same(const char* str1, const char* str2)
{
    if (str1 && str2) {
        return strcasecmp(str1, str2) == 0;
    }
    return 0;
}

int set_wpa_common(const char* intf, const int id)
{
    const char *val;
    int *type;
    int owe, suite_b = 0;
    int ret = STATUS_COMPLETE;
    char buf[256];

    type = get_param_val("type");
    owe = type && *type == SEC_TYPE_OWE;

    val = get_param_str("keyMgmtType");
    if (!val && owe)
        val = "OWE";
    if (val == NULL) {
        return STATUS_INVALID;
    }

    // keyMgmtType to proto
    if (!str_same(val, "OWE")) {
        if (str_same(val, "wpa") || str_same(val, "wpa-psk"))
            wSPRINTF(buf, "WPA");
        else if (str_same(val, "wpa2-wpa-psk") || str_same(val, "wpa2-wpa-ent"))
            wSPRINTF(buf, "WPA WPA2");
        else
            wSPRINTF(buf, "WPA2");
        if (str_same(val, "SuiteB"))
            suite_b = 1;
        if (set_network(intf, id, "proto", buf) < 0)
            ret = STATUS_ERROR;
    }

    val = get_param_str("encpType");
    // encrptype to pairwise
    if (str_same(val, "tkip")) {
        wSPRINTF(buf, "TKIP");
    } else if (str_same(val, "aes-ccmp")) {
        wSPRINTF(buf, "CCMP");
    } else if (str_same(val, "aes-ccmp-tkip")) {
        wSPRINTF(buf, "CCMP TKIP");
    } else if (str_same(val, "aes-gcmp") == 0) {
        wSPRINTF(buf, "GCMP");
        if (set_network(intf, id, "group", buf) < 0)
            ret = STATUS_ERROR;
    }
    if (set_network(intf, id, "pairwise", buf) < 0)
        ret = STATUS_ERROR;

    val = get_param_str("PairwiseCipher");
    if (val) {
        if (str_same(val, "AES-GCMP-256") == 0)
            wSPRINTF(buf, "GCMP-256");
        else if (str_same(val, "AES-CCMP-256") == 0)
            wSPRINTF(buf, "CCMP-256");
        else if (str_same(val, "AES-GCMP-128") == 0)
            wSPRINTF(buf, "GCMP");
        else if (str_same(val, "AES-CCMP-128") == 0)
            wSPRINTF(buf, "CCMP");
        if (set_network(intf, id, "pairwise", buf) < 0)
            ret = STATUS_ERROR;
    }

    val = get_param_str("GroupCipher");
    if (val) {
        if (str_same(val, "AES-GCMP-256") == 0)
            wSPRINTF(buf, "GCMP-256");
        else if (str_same(val, "AES-CCMP-256") == 0)
            wSPRINTF(buf, "CCMP-256");
        else if (str_same(val, "AES-GCMP-128") == 0)
            wSPRINTF(buf, "GCMP");
        else if (str_same(val, "AES-CCMP-128") == 0)
            wSPRINTF(buf, "CCMP");
        if (set_network(intf, id, "group", buf) < 0)
            ret = STATUS_ERROR;
    }

    val = get_param_str("GroupMgntCipher");
    if (val) {
        if (str_same(val, "BIP-GMAC-256") == 0)
            wSPRINTF(buf, "BIP-GMAC-256");
        else if (str_same(val, "BIP-CMAC-256") == 0)
            wSPRINTF(buf, "BIP-CMAC-256");
        else if (str_same(val, "BIP-GMAC-128") == 0)
            wSPRINTF(buf, "BIP-GMAC-128");
        else if (str_same(val, "BIP-CMAC-128") == 0)
            wSPRINTF(buf, "AES-128-CMAC");
        if (set_network(intf, id, "group_mgmt", buf) < 0)
            ret = STATUS_ERROR;
    }

    gDut.sta_pmf = WFA_DISABLED;

    val = get_param_val("pmf");
    if (val) {
        int pmf = *val;

        if (pmf == WFA_DISABLED || pmf == WFA_F_DISABLED)
            gDut.sta_pmf = WFA_DISABLED;
        else if (pmf == WFA_REQUIRED || pmf == WFA_F_REQUIRED || owe || suite_b)
            gDut.sta_pmf = WFA_REQUIRED;
        else
            gDut.sta_pmf = WFA_OPTIONAL;

        wSPRINTF(buf, "%d", gDut.sta_pmf);
        if (set_network(intf, id, "ieee80211w", buf) < 0)
            ret = STATUS_ERROR;
    }
    return ret;
}

int set_eap_common(const char* intf, const int id)
{
    char buf[256];
    char *ext = NULL;
    const char *val, *alg;
    const int *akm;

    if (set_wpa_common(intf, id) != STATUS_COMPLETE)
        return STATUS_ERROR;

    val = get_param_str("keyMgmtType");
    alg = get_param_str("micAlg");
    akm = get_param_val("AKMSuiteType");

    buf[0] = '\0';

    // key_mgmt
    if (str_same(val, "SuiteB")) {
        wSPRINTF(buf, "WPA-EAP-SUITE-B-192");
    } else if (str_same(alg, "SHA-256")) {
        wSPRINTF(buf, "WPA-EAP-SHA256");
    } else if (str_same(alg, "SHA-1")) {
        wSPRINTF(buf, "WPA-EAP");
    } else if (str_same(val, "wpa2-ft")) {
        wSPRINTF(buf, "FT-EAP");
    } else if (str_same(val, "wpa2-sha256") ||
           gDut.sta_pmf == WFA_REQUIRED || gDut.sta_pmf == WFA_OPTIONAL) {
        wSPRINTF(buf, "WPA-EAP WPA-EAP-SHA256");
    } else if (akm && *akm == 14) {
        if (gDut.sta_pmf == WFA_REQUIRED || gDut.sta_pmf == WFA_OPTIONAL)
            wSPRINTF(buf, "WPA-EAP-SHA256 FILS-SHA256");
        else
            wSPRINTF(buf, "WPA-EAP FILS-SHA256");
        if (set_network(intf, id, "erp", "1") < 0)
             return STATUS_ERROR;
    } else if (akm && *akm == 15) {
        if (gDut.sta_pmf == WFA_REQUIRED || gDut.sta_pmf == WFA_OPTIONAL)
            wSPRINTF(buf, "WPA-EAP-SHA256 FILS-SHA384");
        else
            wSPRINTF(buf, "WPA-EAP FILS-SHA384");
        if (set_network(intf, id, "erp", "1") < 0)
             return STATUS_ERROR;
    } else if (gDut.sta_pmf == WFA_OPTIONAL) {
        wSPRINTF(buf, "WPA-EAP WPA-EAP-SHA256");
    } else {
        wSPRINTF(buf, "WPA-EAP");
    }
    if (set_network(intf, id, "key_mgmt", buf) < 0)
        return STATUS_ERROR;


    val = get_param_str("trustedRootCA");
    if (val) {
        ext = strstr(val, ".pem");
        if (ext)
            *ext = 0;
        wSPRINTF(buf, EAP_CERT_PATH"/%s.pem", val);
        chk_exist(buf);
        if (set_network_quoted(intf, id, "ca_cert", buf) < 0)
            return STATUS_ERROR;
    }

    val = get_param_str("clientCertificate");
    if (val) {
        ext = strstr(val, ".pem");
        if (ext)
            *ext = 0;

        wSPRINTF(buf, EAP_CERT_PATH"/%s.pem", val);
        chk_exist(buf);
        if (set_network_quoted(intf, id, "client_cert", buf) < 0)
            return STATUS_ERROR;

        wSPRINTF(buf, EAP_CERT_PATH"/%s.key", val);
        chk_exist(buf);
        if (set_network_quoted(intf, id, "private_key", buf) < 0)
            return STATUS_ERROR;
    }

    val = get_param_str("username");
    if (val) {
        if (set_network_quoted(intf, id, "identity", val) < 0)
            return STATUS_ERROR;
    }

    val = get_param_str("password");
    if (val) {
        if (set_network_quoted(intf, id, "password", val) < 0)
            return STATUS_ERROR;
    }

    // default enable okc
    if (set_network(intf, id, "proactive_key_caching", "1") < 0)
        return STATUS_ERROR;

    return STATUS_COMPLETE;
}

int sta_set_open(int *respLen, BYTE *respBuf) {
    char *intf = get_param_str("intf");
    dutCmdResponse_t *setOpenResp = &gGenericResp;

    char buf[256];
    int id;
    const char *val, *alg, *pmf;

    id = add_network_common(intf);
    if (id < 0) {
        setOpenResp->status = STATUS_ERROR;
        goto done;
    }

    if (set_network(intf, id, "key_mgmt", "NONE") < 0) {
        setOpenResp->status = STATUS_ERROR;
        goto done;
    }

    setOpenResp->status = STATUS_COMPLETE;
done:
    wfaEncodeTLV(WFA_STA_SET_SECURITY_RESP_TLV, 4, (BYTE *)setOpenResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + 4;

    return WFA_SUCCESS;
}


int sta_set_owe(int *respLen, BYTE *respBuf)
{
    char *intf = get_param_str("intf");
    dutCmdResponse_t *setOweResp = &gGenericResp;
    const char *val;
    int id;

    id = add_network_common(intf);
    if (id < 0) {
        setOweResp->status = STATUS_ERROR;
        goto done;
    }

    if (set_wpa_common(intf, id) != STATUS_COMPLETE) {
        setOweResp->status = STATUS_ERROR;
        goto done;
    }

    if (set_network(intf, id, "key_mgmt", "OWE") < 0) {
        setOweResp->status = STATUS_ERROR;
        goto done;
    }

    val = get_param_str("ECGroupID");
    if (val) {
        if (set_network(intf, id, "owe_group", val) < 0) {
            setOweResp->status = STATUS_ERROR;
            goto done;
        }
    }

    setOweResp->status = STATUS_COMPLETE;
done:
    wfaEncodeTLV(WFA_STA_SET_SECURITY_RESP_TLV, 4, (BYTE *)setOweResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + 4;

    return WFA_SUCCESS;
}

int sta_mon_conn(const char* ifname) {
    struct wpa_ctrl *ctrl;
    int res;
    char buf[1024];

    // wait for connected event
    ctrl = open_wpa_mon(ifname);
    if (ctrl == NULL) {
        DPRINT_ERR(WFA_ERR, "Failed to open wpa_supplicant monitor connection\n");
        return -1;
    }

    res = get_wpa_cli_event(ctrl, "CTRL-EVENT-CONNECTED", buf, sizeof(buf));

    wpa_ctrl_detach(ctrl);
    wpa_ctrl_close(ctrl);

    if (res < 0)
        DPRINT_WARNING(WFA_WNG, "Failed to connect to network %d on %s\n", gDut.networkId, ifname);

    return res;
}

static void init_driver_gen()
{
    if (shell("getprop ro.vendor.wlan.gen") != WFA_SUCCESS)
        return;
    if (strstr(gDut.shellResult, "gen2"))
        gDut.driver_gen = GEN_2;
    else if (strstr(gDut.shellResult, "gen3"))
        gDut.driver_gen = GEN_3;
    else if (strstr(gDut.shellResult, "gen4m"))
        gDut.driver_gen = GEN_4m;
    else if (strstr(gDut.shellResult, "gen4"))
        gDut.driver_gen = GEN_4;
    else
        gDut.driver_gen = GEN_UNKNOWN;
    DPRINT_INFO(WFA_OUT, "propertyValue=%s, driver gen=%d.\n",
            gDut.shellResult,
            gDut.driver_gen);
}

#ifdef CONFIG_MTK_AP
int wfaApGetVersion(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t *getVerResp = &gGenericResp;

    DPRINT_INFO(WFA_OUT, "Entering %s()\n", __func__);

    getVerResp->status = STATUS_COMPLETE;
    wSTRNCPY(getVerResp->cmdru.version, MTK_SYSTEM_VER, sizeof(getVerResp->cmdru.version));
    getVerResp->cmdru.version[sizeof(getVerResp->cmdru.version) - 1] = '\0';

    wfaEncodeTLV(WFA_AP_CA_VERSION_RESP_TLV, sizeof(dutCmdResponse_t),
            (BYTE *)getVerResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

    return WFA_SUCCESS;
}

int wfaApConfigCommit(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t *resp = &gGenericResp;
    char command[1000];

    DPRINT_INFO(WFA_OUT, "Entering %s()\n", __func__);

    write_hostapd_conf();
    write_driver_conf();

    if (shell("killall hostapd") != WFA_SUCCESS)
        DPRINT_WARNING(WFA_WNG, "killall hostapd fail\n");
    if (shell("echo 0 > /dev/wmtWifi") != WFA_SUCCESS)
        DPRINT_WARNING(WFA_WNG, "turn off wifi driver fail\n");
    wSLEEP(1);

    if (shell("echo 1 > /dev/wmtWifi") != WFA_SUCCESS)
        DPRINT_WARNING(WFA_WNG, "turn on wifi driver fail\n");
    if (shell("echo AP > /dev/wmtWifi") != WFA_SUCCESS)
        DPRINT_WARNING(WFA_WNG, "change wifi driver to AP mode fail\n");
    wMEMSET(&command, 0, sizeof(command));
    wSNPRINTF(command, sizeof(command), "%s -ddd %s &",
            gDut.ap_params.hostapd_bin,
            gDut.ap_params.hostapd_conf);
    if (system(command) != 0)
        DPRINT_WARNING(WFA_WNG, "turn on hostapd fail\n");

    setup_ap_bridge_n_address();

    wUSLEEP(500000);

    if (run_hostapd_cli("ping") == WFA_SUCCESS)
        DPRINT_INFO(WFA_OUT, "ping resp: %s\n", gDut.shellResult);

    wSLEEP(5);

    /* notify FW sigma test mode to disable fast-tx */
    if (shell(IWPRIV " %s set_sw_ctrl 0xa0400000 0xE",
            gDut.ap_params.inf_name) != WFA_SUCCESS)
        DPRINT_WARNING(WFA_WNG, "setup ip/netmask fail\n");

    resp->status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_AP_CONFIG_COMMIT_RESP_TLV, sizeof(dutCmdResponse_t),
            (BYTE *)resp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

    return WFA_SUCCESS;
}

int wfaApDeauthSta(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t *resp = &gGenericResp;
    dutCommand_t *deauthSta = (dutCommand_t *) caCmdBuf;
    char buf[100];

    DPRINT_INFO(WFA_OUT, "Entering %s(), sta mac: %s\n", __func__,
            deauthSta->cmdsu.macaddr);

    wSNPRINTF(buf, sizeof(buf), "deauth %s", deauthSta->cmdsu.macaddr);
    run_hostapd_cli(buf);

    resp->status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_AP_DEAUTH_STA_RESP_TLV, sizeof(dutCmdResponse_t),
            (BYTE *)resp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

    return WFA_SUCCESS;
}

int wfaApGetMacAddress(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    unsigned char addr[6];
    dutCmdResponse_t *getMacResp = &gGenericResp;

    DPRINT_INFO(WFA_OUT, "Entering %s()\n", __func__);

    getMacResp->status = get_mac(gDut.ap_params.inf_name, addr);
    wSNPRINTF(getMacResp->cmdru.mac, sizeof(getMacResp->cmdru.mac),
            "%02x:%02x:%02x:%02x:%02x:%02x",
            addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

    DPRINT_INFO(WFA_OUT, "mac=%s\n", getMacResp->cmdru.mac);
exit:
    wfaEncodeTLV(WFA_AP_GET_MAC_ADDRESS_RESP_TLV, sizeof(dutCmdResponse_t),
            (BYTE *)getMacResp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);
    return WFA_SUCCESS;
}

int wfaApResetDefault(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t *resp = &gGenericResp;
    caApResetDefault_t *apResetDefault_t = (caApResetDefault_t *) caCmdBuf;

    DPRINT_INFO(WFA_OUT, "Entering %s(), program: %d\n", __func__,
            apResetDefault_t->program);

    gDut.ap_params.program = apResetDefault_t->program;

    wMEMSET(&gDut.ap_params, 0, sizeof(ap_params_t));

    init_ap_params();

    if (shell("killall hostapd") != WFA_SUCCESS)
        DPRINT_WARNING(WFA_WNG, "killall hostapd fail\n");
    if (shell("echo 0 > /dev/wmtWifi") != WFA_SUCCESS)
        DPRINT_WARNING(WFA_WNG, "turn off wifi driver fail\n");
    wSLEEP(1);

    resp->status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_AP_RESET_DEFAULT_RESP_TLV, sizeof(dutCmdResponse_t),
            (BYTE *)resp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

    return WFA_SUCCESS;
}

int wfaApSendAddBaReq(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t *resp = &gGenericResp;

    DPRINT_INFO(WFA_OUT, "Entering %s(), Not support now, skip.\n", __func__);

    resp->status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_AP_SEND_ADDBA_REQ_RESP_TLV, sizeof(dutCmdResponse_t),
            (BYTE *)resp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

    return WFA_SUCCESS;
}

int wfaApSetApQos(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t *resp = &gGenericResp;

    DPRINT_INFO(WFA_OUT, "Entering %s(), Not support now, skip.\n", __func__);

    resp->status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_AP_SET_APQOS_RESP_TLV, sizeof(dutCmdResponse_t),
            (BYTE *)resp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

    return WFA_SUCCESS;
}

int wfaApSetPmf(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t *resp = &gGenericResp;

    DPRINT_INFO(WFA_OUT, "Entering %s()\n", __func__);

    /*
     * Ignore this command since the parameters are already
     * handled by xcCmdProcApSetSecurity */

    resp->status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_AP_SET_PMF_RESP_TLV, sizeof(dutCmdResponse_t),
            (BYTE *)resp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

    return WFA_SUCCESS;
}

int wfaApSetSecurity(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t *resp = &gGenericResp;
    caApSetSecurity_t *apSetSecurity = (caApSetSecurity_t *) caCmdBuf;
    ap_params_t *p_ap_params = &gDut.ap_params;

    DPRINT_INFO(WFA_OUT, "Entering %s()\n", __func__);

    if (apSetSecurity->keyMgmtType)
        p_ap_params->keyMgmtType = apSetSecurity->keyMgmtType;

    if (apSetSecurity->ap_wepkey[0])
        wSTRNCPY(p_ap_params->wepkey, apSetSecurity->ap_wepkey,
                sizeof(p_ap_params->wepkey) - 1);

    if (apSetSecurity->encpType)
        p_ap_params->encpType = apSetSecurity->encpType;

    if (apSetSecurity->ap_passphrase[0])
        wSTRNCPY(p_ap_params->passphrase, apSetSecurity->ap_passphrase,
                sizeof(p_ap_params->passphrase) - 1);

    if (apSetSecurity->pmf)
        p_ap_params->pmf = apSetSecurity->pmf;

    if (apSetSecurity->sha256ad)
        p_ap_params->sha256ad = apSetSecurity->sha256ad;

    if (apSetSecurity->preauthentication)
        p_ap_params->preauthentication = apSetSecurity->preauthentication;

    if (apSetSecurity->antiCloggingThreshold)
        p_ap_params->antiCloggingThreshold = apSetSecurity->antiCloggingThreshold;

    if (apSetSecurity->ap_reflection)
        p_ap_params->ap_reflection = apSetSecurity->ap_reflection;

    if (apSetSecurity->ap_ecGroupID[0])
        wSTRNCPY(p_ap_params->ap_ecGroupID, apSetSecurity->ap_ecGroupID,
                sizeof(p_ap_params->ap_ecGroupID) - 1);

    if (apSetSecurity->ap_sae_commit_override[0])
        wSTRNCPY(p_ap_params->ap_sae_commit_override, apSetSecurity->ap_sae_commit_override,
                sizeof(p_ap_params->ap_sae_commit_override) - 1);

    resp->status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_AP_SET_SECURITY_RESP_TLV, sizeof(dutCmdResponse_t),
            (BYTE *)resp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

    return WFA_SUCCESS;
}

int wfaApSetStaQos(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t *resp = &gGenericResp;

    DPRINT_INFO(WFA_OUT, "Entering %s(), Not support now, skip.\n", __func__);

    resp->status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_AP_SET_STAQOS_RESP_TLV, sizeof(dutCmdResponse_t),
            (BYTE *)resp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

    return WFA_SUCCESS;
}

int wfaApSetWireless(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t *resp = &gGenericResp;
    caApSetWireless_t *apSetWireless = (caApSetWireless_t *) caCmdBuf;
    ap_params_t *p_ap_params = &gDut.ap_params;

    DPRINT_INFO(WFA_OUT, "Entering %s()\n", __func__);

    if (apSetWireless->program)
        p_ap_params->program = apSetWireless->program;

    if (strlen(apSetWireless->ssid) > 0)
        wSTRNCPY(p_ap_params->ssid, apSetWireless->ssid,
                sizeof(p_ap_params->ssid) - 1);

    if (strlen(apSetWireless->dutName) > 0)
        wSTRNCPY(p_ap_params->device_name, apSetWireless->dutName,
                sizeof(p_ap_params->device_name) - 1);

    if (apSetWireless->channel)
        p_ap_params->channel = apSetWireless->channel;

    if (apSetWireless->rts)
        p_ap_params->rts = apSetWireless->rts;

    if (apSetWireless->fragment)
        p_ap_params->frgmnt = apSetWireless->fragment;

    if (apSetWireless->beaconInterval)
        p_ap_params->bcnint = apSetWireless->beaconInterval;

    if (apSetWireless->dtim)
        p_ap_params->dtim_period = apSetWireless->dtim;

    if (apSetWireless->wme)
        p_ap_params->wme = apSetWireless->wme;

    if (apSetWireless->wmmps)
        p_ap_params->wmmps = apSetWireless->wmmps;

    if (apSetWireless->p2p_mgmt)
        p_ap_params->p2p_mgmt = apSetWireless->p2p_mgmt;

    if (apSetWireless->mode)
        p_ap_params->mode = apSetWireless->mode;

    if (apSetWireless->offset)
        p_ap_params->ch_offset = apSetWireless->offset;

    if (apSetWireless->sgi20)
        p_ap_params->sgi20 = apSetWireless->sgi20;

    resp->status = STATUS_COMPLETE;

    wfaEncodeTLV(WFA_AP_SET_WIRELESS_RESP_TLV, sizeof(dutCmdResponse_t),
            (BYTE *)resp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

    return WFA_SUCCESS;
}

int wfaApAccessPoint(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t *resp = &gGenericResp;

    DPRINT_INFO(WFA_OUT, "Entering %s(), do nothing.\n", __func__);

    resp->status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_AP_ACCESS_POINT_RESP_TLV, sizeof(dutCmdResponse_t),
            (BYTE *)resp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

    return WFA_SUCCESS;
}

static void init_ap_params()
{
    int sdk;
    char propertyValue[128] = {'\0'};

    DPRINT_INFO(WFA_OUT, "Entering %s()\n", __func__);

    gDut.ap_params.keyMgmtType = KEY_MGNT_TYPE_OPEN;

    wSNPRINTF(gDut.ap_params.inf_name, sizeof(gDut.ap_params.inf_name), "%s",
            "ap0");
    wSNPRINTF(gDut.ap_params.hostapd_conf, sizeof(gDut.ap_params.hostapd_conf),
            "%s", "/data/local/sigma_hostapd.conf");
    wSNPRINTF(gDut.ap_params.driver_conf, sizeof(gDut.ap_params.driver_conf),
            "%s", "/data/misc/wifi/wifi.cfg");

    wMEMSET(&propertyValue, 0, sizeof(propertyValue));
    if (shell("getprop vendor.wifi.sigma.ip.addr") == WFA_SUCCESS)
        wSTRNCPY(propertyValue, gDut.shellResult, sizeof(propertyValue) - 1);
    wSNPRINTF(gDut.ap_params.ip_addr, sizeof(gDut.ap_params.ip_addr), "%s",
            propertyValue);

    wMEMSET(&propertyValue, 0, sizeof(propertyValue));
    if (shell("getprop vendor.wifi.sigma.ip.netmask") == WFA_SUCCESS)
        wSTRNCPY(propertyValue, gDut.shellResult, sizeof(propertyValue) - 1);
    wSNPRINTF(gDut.ap_params.ip_netmask, sizeof(gDut.ap_params.ip_netmask), "%s",
            propertyValue);

    wMEMSET(&propertyValue, 0, sizeof(propertyValue));
    if (shell("getprop ro.build.version.sdk") == WFA_SUCCESS)
        wSTRNCPY(propertyValue, gDut.shellResult, sizeof(propertyValue) - 1);
    sdk = atoi(propertyValue);
    if (sdk >= 19 && sdk < 26) { // KK ~ N
        wSNPRINTF(gDut.ap_params.ctrl_inf, sizeof(gDut.ap_params.ctrl_inf), "%s",
                "data/misc/wifi/hostapd");
        wSNPRINTF(gDut.ap_params.hostapd_bin, sizeof(gDut.ap_params.hostapd_bin),
                "%s", "/system/bin/hostapd");
    } else if (sdk >= 26 && sdk < 28) { // O
        wSNPRINTF(gDut.ap_params.ctrl_inf, sizeof(gDut.ap_params.ctrl_inf), "%s",
                "data/misc/wifi/hostapd/ctrl");
        wSNPRINTF(gDut.ap_params.hostapd_bin, sizeof(gDut.ap_params.hostapd_bin),
                "%s", "/vendor/bin/hostapd");
    } else { // P ~
        wSNPRINTF(gDut.ap_params.ctrl_inf, sizeof(gDut.ap_params.ctrl_inf), "%s",
                "data/vendor/wifi/hostapd/ctrl");
        wSNPRINTF(gDut.ap_params.hostapd_bin, sizeof(gDut.ap_params.hostapd_bin),
                "%s", "/vendor/bin/hw/hostapd");
    }
}

static void config_hostapd_htcap(FILE *f, ap_params_t *p_ap_params)
{
    int ht40plus = 0, ht40minus = 0;

    DPRINT_INFO(WFA_OUT, "Entering %s()\n", __func__);

    if (f == NULL || p_ap_params == NULL) {
        DPRINT_WARNING(WFA_WNG, "invalid args\n");
        return;
    }

    if (p_ap_params->mode != AP_MODE_11ng &&
            p_ap_params->mode != AP_MODE_11na &&
            p_ap_params->mode != AP_MODE_11ac)
        return;

    if (p_ap_params->ch_width == CHANNEL_WIDTH_40 &&
            p_ap_params->ch_offset == CHANNEL_OFFSET_ABOVE) {
        ht40plus = 1;
        ht40minus = 0;
    } else if (p_ap_params->ch_width == CHANNEL_WIDTH_40 &&
            p_ap_params->ch_offset == CHANNEL_OFFSET_BELOW) {
        ht40plus = 0;
        ht40minus = 1;
    } else if (p_ap_params->mode == AP_MODE_11ac) {
        /* Force secondary channel offset to 40+ in 11ac mode */
        ht40plus = 1;
        ht40minus = 0;
    } else {
        ht40plus = 0;
        ht40minus = 0;
        DPRINT_INFO(WFA_OUT, "no channel offset setting.\n");
    }
    if (!ht40plus && !ht40minus && p_ap_params->sgi20 != WFA_ENABLED)
        return;
    fprintf(f, "ht_capab=%s%s%s\n",
        ht40plus ? "[HT40+]" : "",
        ht40minus ? "[HT40-]" : "",
        p_ap_params->sgi20 == WFA_ENABLED ? "[SHORT-GI-20]" : "");
}

static void config_driver_wmm_params(FILE *f, ap_params_t *p_ap_params)
{
    DPRINT_INFO(WFA_OUT, "Entering %s()\n", __func__);

    if (f == NULL || p_ap_params == NULL)
        return;

    if (p_ap_params->mode == AP_MODE_11ng) {
        switch (gDut.driver_gen) {
        case GEN_2:
            fprintf(f, "BeAifsN 3");
            fprintf(f, "BeCwMin 3");
            fprintf(f, "BeCwMax 6");
            fprintf(f, "BkAifsN 8");
            fprintf(f, "BkCwMin 4");
            fprintf(f, "BkCwMax 10");
            break;
        case GEN_3:
            fprintf(f, "BeAifsN 3");
            fprintf(f, "BeCwMin 8");
            fprintf(f, "BeCwMax 63");
            fprintf(f, "BkAifsN 8");
            fprintf(f, "BkCwMin 15");
            fprintf(f, "BkCwMax 1024");
            break;
        default:
            break;
        }
    }
}

static void write_hostapd_conf()
{
    FILE *f;
    ap_params_t *p_ap_params = &gDut.ap_params;
    char buf[500];

    DPRINT_INFO(WFA_OUT, "Entering %s()\n", __func__);

    f = fopen("/data/local/sigma_hostapd.conf", "w");
    if (f == NULL) {
        DPRINT_ERR(WFA_ERR, "file open fail\n");
        goto exit;
    }

    fprintf(f, "interface=%s\n", p_ap_params->inf_name);
    fprintf(f, "ctrl_interface=%s\n", p_ap_params->ctrl_inf);

    if (strlen(p_ap_params->ssid) > 0)
        fprintf(f, "ssid=%s\n", p_ap_params->ssid);

    if (p_ap_params->device_name[0])
        fprintf(f, "device_name=%s\n", p_ap_params->device_name);

    switch (p_ap_params->mode) {
    case AP_MODE_11b:
        fprintf(f, "hw_mode=b\n");
        break;
    case AP_MODE_11g:
    case AP_MODE_11ng:
        fprintf(f, "hw_mode=g\n");
        if (p_ap_params->mode == AP_MODE_11ng) {
            fprintf(f, "ieee80211n=1\n");
        }
        break;
    case AP_MODE_11a:
    case AP_MODE_11na:
    case AP_MODE_11ac:
        fprintf(f, "hw_mode=a\n");
        if (p_ap_params->mode == AP_MODE_11na ||
                p_ap_params->mode == AP_MODE_11ac) {
            fprintf(f, "ieee80211n=1\n");
        }
        if (p_ap_params->mode == AP_MODE_11ac) {
            fprintf(f, "ieee80211ac=1\n");
            if (p_ap_params->country_code[0])
                fprintf(f, "country_code=%s\n", p_ap_params->country_code);
        }
        break;
    default:
        DPRINT_WARNING(WFA_WNG, "unsupport mode: %d\n", p_ap_params->mode);
        goto exit;
    }
    config_hostapd_htcap(f, p_ap_params);
    if (p_ap_params->channel)
        fprintf(f, "channel=%d\n", p_ap_params->channel);
    if (p_ap_params->bcnint)
        fprintf(f, "beacon_int=%d\n", p_ap_params->bcnint);
    if (p_ap_params->rts)
        fprintf(f, "rts_threshold=%d\n", p_ap_params->rts);
    if (p_ap_params->frgmnt)
        fprintf(f, "fragm_threshold=%d\n", p_ap_params->frgmnt);
    if (p_ap_params->dtim_period)
        fprintf(f, "dtim_period=%d\n", p_ap_params->dtim_period);
    if (p_ap_params->wme)
        fprintf(f, "wmm_enabled=1\n");
    if (p_ap_params->wmmps)
        fprintf(f, "uapsd_advertisement_enabled=1\n");
    if (p_ap_params->p2p_mgmt)
        fprintf(f, "manage_p2p=1\n");

    switch (p_ap_params->keyMgmtType) {
    case KEY_MGNT_TYPE_OPEN:
        if (p_ap_params->wepkey[0])
            fprintf(f, "wep_key0=%s\n", p_ap_params->wepkey);
        break;
    case KEY_MGNT_TYPE_WPA_PSK:
    case KEY_MGNT_TYPE_WPA2_PSK:
    case KEY_MGNT_TYPE_WPA2_PSK_MIXED:
        /* config wpa */
        if (p_ap_params->keyMgmtType == KEY_MGNT_TYPE_WPA2_PSK) {
            fprintf(f, "rsn_pairwise=CCMP\n");
            fprintf(f, "wpa=2\n");
        } else if (p_ap_params->keyMgmtType == KEY_MGNT_TYPE_WPA2_PSK_MIXED) {
            fprintf(f, "wpa=3\n");
        } else {
            fprintf(f, "wpa=1\n");
        }

        switch (p_ap_params->encpType) {
        case ENCP_TYPE_NONE:
            /* do nothing*/
            break;
        case ENCP_TYPE_TKIP:
            fprintf(f, "wpa_pairwise=TKIP\n");
            break;
        case ENCP_TYPE_CCMP:
            fprintf(f, "wpa_pairwise=CCMP\n");
            break;
        case ENCP_TYPE_GCMP_128:
            fprintf(f, "wpa_pairwise=GCMP\n");
            break;
        case ENCP_TYPE_CCMP_TKIP:
            fprintf(f, "wpa_pairwise=CCMP TKIP\n");
            break;
        default:
            DPRINT_WARNING(WFA_WNG, "unknown encpType: %d",
                    p_ap_params->encpType);
            break;
        }

        if (p_ap_params->passphrase[0])
            fprintf(f, "wpa_passphrase=%s\n", p_ap_params->passphrase);

        switch (p_ap_params->pmf) {
        case WFA_DISABLED:
        case WFA_OPTIONAL:
            if (p_ap_params->sha256ad)
                fprintf(f, "wpa_key_mgmt=WPA-PSK WPA-PSK-SHA256\n");
            else
                fprintf(f, "wpa_key_mgmt=WPA-PSK\n");
            break;
        case WFA_REQUIRED:
            fprintf(f, "WPA-PSK-SHA256\n");
            break;
        default:
            break;
        }
        break;

    case KEY_MGNT_TYPE_WPA2_PSK_SAE:
    case KEY_MGNT_TYPE_WPA2_SAE:
    {
        const char *key_mgmt;

        fprintf(f, "wpa=2\n");

        switch (p_ap_params->pmf) {
        case WFA_DISABLED:
        case WFA_OPTIONAL:
            if (p_ap_params->keyMgmtType == KEY_MGNT_TYPE_WPA2_SAE) {
                key_mgmt = "SAE";
            } else {
                key_mgmt = "WPA-PSK SAE";
            }
            fprintf(f, "wpa_key_mgmt=%s%s\n", key_mgmt, p_ap_params->sha256ad ? " WPA-PSK-SHA256" : "");
            break;
        case WFA_REQUIRED:
            if (p_ap_params->keyMgmtType == KEY_MGNT_TYPE_WPA2_SAE) {
                key_mgmt = "SAE";
            } else {
                key_mgmt = "WPA-PSK-SHA256 SAE";
            }
            fprintf(f, "wpa_key_mgmt=%s\n", key_mgmt);
            break;
        default:
            break;
        }

        switch (p_ap_params->encpType) {
        case ENCP_TYPE_NONE:
            /* do nothing*/
            break;
        case ENCP_TYPE_TKIP:
            fprintf(f, "wpa_pairwise=TKIP\n");
            break;
        case ENCP_TYPE_CCMP:
            fprintf(f, "wpa_pairwise=CCMP\n");
            break;
        case ENCP_TYPE_GCMP_128:
            fprintf(f, "wpa_pairwise=GCMP\n");
            break;
        case ENCP_TYPE_CCMP_TKIP:
            fprintf(f, "wpa_pairwise=CCMP TKIP\n");
            break;
        default:
            DPRINT_WARNING(WFA_WNG, "unknown encpType: %d",
                    p_ap_params->encpType);
            break;
        }

        if (p_ap_params->keyMgmtType == KEY_MGNT_TYPE_WPA2_SAE) {
            ///fprintf(f, "sae_require_mfp=2\n");
            if (p_ap_params->passphrase[0])
                fprintf(f, "sae_password=%s\n", p_ap_params->passphrase);
        } else {
            if (p_ap_params->passphrase[0]) {
                fprintf(f, "wpa_passphrase=%s\n", p_ap_params->passphrase);
                fprintf(f, "sae_password=%s\n", p_ap_params->passphrase);
            }
        }
        break;
    }
    case KEY_MGNT_TYPE_WPA_EAP:
    case KEY_MGNT_TYPE_WPA2_EAP:
    case KEY_MGNT_TYPE_WPA2_EAP_MIXED:
    case KEY_MGNT_TYPE_SUITEB:
        DPRINT_WARNING(WFA_WNG, "not support keyMgmtType: %d",
                p_ap_params->keyMgmtType);
        break;
    default:
        DPRINT_WARNING(WFA_WNG, "unknown keyMgmtType: %d",
                p_ap_params->keyMgmtType);
        break;
    }

    switch (p_ap_params->pmf) {
    case WFA_DISABLED:
        /* do nothing */
        break;
    case WFA_OPTIONAL:
        fprintf(f, "ieee80211w=1\n");
        if (p_ap_params->keyMgmtType == KEY_MGNT_TYPE_WPA2_PSK_SAE) {
            fprintf(f, "sae_require_mfp=1\n");
        }
        break;
    case WFA_REQUIRED:
        fprintf(f, "ieee80211w=2\n");
        break;
    default:
        break;
    }

    if (p_ap_params->preauthentication)
        fprintf(f, "rsn_preauth=1\n");

    if (p_ap_params->ap_ecGroupID[0])
        fprintf(f, "sae_groups=%s\n", p_ap_params->ap_ecGroupID);

    if (p_ap_params->ap_sae_commit_override[0])
        fprintf(f, "sae_commit_override=%s\n", p_ap_params->ap_sae_commit_override);

    if (p_ap_params->antiCloggingThreshold >= 0)
        fprintf(f, "sae_anti_clogging_threshold=%d\n", p_ap_params->antiCloggingThreshold);

    if (p_ap_params->ap_reflection)
        fprintf(f, "sae_reflection_attack=1\n");

exit:
    if (f != NULL)
        fclose(f);
    if (shell("chmod 0777 /data/local/sigma_hostapd.conf") != WFA_SUCCESS)
        DPRINT_WARNING(WFA_WNG, "chmod for hostapd.conf fail\n");
    if (shell("chown wifi:system /data/local/sigma_hostapd.conf") !=
            WFA_SUCCESS)
        DPRINT_WARNING(WFA_WNG, "chown for hostapd.conf fail\n");
}

static void write_driver_conf()
{
    FILE *f;
    ap_params_t *p_ap_params = &gDut.ap_params;
    char buf[500];

    DPRINT_INFO(WFA_OUT, "Entering %s()\n", __func__);

    f = fopen(gDut.ap_params.driver_conf, "w");
    if (f == NULL) {
        DPRINT_ERR(WFA_ERR, "file open fail\n");
        goto exit;
    }
    if (p_ap_params->wmmps && p_ap_params->wme)
        fprintf(f, "ApUapsd 1\n");
    if (p_ap_params->sig_rts)
        fprintf(f, "SigTaRts 1\n");
    if (p_ap_params->dynamic_bw_signaling)
        fprintf(f, "DynBwRts 1\n");
    config_driver_wmm_params(f, p_ap_params);
exit:
    if (f != NULL)
        fclose(f);
    if (shell("chmod 0777 %s", gDut.ap_params.driver_conf) != WFA_SUCCESS)
        DPRINT_WARNING(WFA_WNG, "chmod for driver config fail\n");
}

static int run_hostapd_cli(char *buf)
{
    DPRINT_INFO(WFA_OUT, "Entering %s(), buf: %s\n", __func__, buf);

    return shell("hostapd_cli -i%s -p %s %s",
            gDut.ap_params.inf_name,
            gDut.ap_params.ctrl_inf,
            buf);
}

static int is_interface_up(char *inf)
{
    struct ifreq ifr;
    int sock = wSOCKET(PF_INET6, SOCK_DGRAM, IPPROTO_IP);

    if (sock < 0)
        return 0;

    wMEMSET(&ifr, 0, sizeof(ifr));
    wSTRNCPY(ifr.ifr_name, inf, sizeof(ifr.ifr_name) - 1);
    if (wIOCTL(sock, SIOCGIFFLAGS, &ifr) < 0) {
        DPRINT_WARNING(WFA_WNG, "Get interface flags fail: %s\n",
                AP_BRIDGE_INF_NAME);
        wCLOSE(sock);
        return 0;
    }
    wCLOSE(sock);
    return !!(ifr.ifr_flags & IFF_UP);
}

static void mtk_del_ap_bridge()
{
    int ret;

    DPRINT_INFO(WFA_OUT, "Entering %s()\n", __func__);

    if (shell("ifconfig " AP_BRIDGE_INF_NAME " down") != WFA_SUCCESS)
        DPRINT_WARNING(WFA_WNG, "Set %s interface down fail\n",
                AP_BRIDGE_INF_NAME);
    ret = mtk_bridge_intf_setup(AP_BRIDGE_INF_NAME, gDut.ap_params.inf_name,
            BRIDGE_OP_TYPE_DEL_INTF);
    ret = mtk_bridge_intf_setup(AP_BRIDGE_INF_NAME, USB_INF_NAME,
            BRIDGE_OP_TYPE_DEL_INTF);
    ret = mtk_bridge_setup(AP_BRIDGE_INF_NAME, BRIDGE_OP_TYPE_DEL_BR);
}

static void mtk_add_ap_bridge()
{
    int ret;

    DPRINT_INFO(WFA_OUT, "Entering %s()\n", __func__);

    if (is_interface_up(AP_BRIDGE_INF_NAME)) {
        DPRINT_WARNING(WFA_WNG, "Interface %s already exists.\n",
                AP_BRIDGE_INF_NAME);
        return;
    }

    /* Clear rndis0's ip address */
    if (shell("ifconfig " USB_INF_NAME " 0.0.0.0") != WFA_SUCCESS)
        DPRINT_WARNING(WFA_WNG, "Clear interface %s's address fail\n",
                USB_INF_NAME);

    /* Create bridge br0 */
    ret = mtk_bridge_setup(AP_BRIDGE_INF_NAME, BRIDGE_OP_TYPE_ADD_BR);

    /* Bind br0 with ap0 & rndis0 */
    ret = mtk_bridge_intf_setup(AP_BRIDGE_INF_NAME, gDut.ap_params.inf_name,
            BRIDGE_OP_TYPE_ADD_INTF);
    ret = mtk_bridge_intf_setup(AP_BRIDGE_INF_NAME, USB_INF_NAME,
            BRIDGE_OP_TYPE_ADD_INTF);

    /* Set br0's ip & netmask' */
    if (shell("ifconfig %s %s netmask %s up",
            AP_BRIDGE_INF_NAME,
            gDut.ap_params.ip_addr,
            gDut.ap_params.ip_netmask) != WFA_SUCCESS)
        DPRINT_WARNING(WFA_WNG, "Setup %s ip/netmask fail\n",
                AP_BRIDGE_INF_NAME);
}

static void setup_ap_bridge_n_address()
{
    mtk_del_ap_bridge();

    if (!is_interface_up(USB_INF_NAME)) {
        DPRINT_INFO(WFA_OUT, "Setup %s's address directly since %s not existed.\n",
                gDut.ap_params.inf_name, USB_INF_NAME);
        if (shell("ifconfig %s %s netmask %s up",
                gDut.ap_params.inf_name,
                gDut.ap_params.ip_addr,
                gDut.ap_params.ip_netmask) != WFA_SUCCESS)
            DPRINT_WARNING(WFA_WNG, "Setup %s ip/netmask fail\n",
                    gDut.ap_params.inf_name);
        return;
    }

    mtk_add_ap_bridge();

    /* Disable iptables on br0 */
    shell("echo 0 > /proc/sys/net/bridge/bridge-nf-call-ip6tables");
    shell("echo 0 > /proc/sys/net/bridge/bridge-nf-call-iptables");
    shell("echo 0 > /proc/sys/net/bridge/bridge-nf-call-arptables");
}

static int mtk_bridge_setup(const char *brname, enum ENUM_BRIDGE_OP_TYPE type)
{
    int ret = 0;
    int br_ctrl_fd = -1;

    if (type != BRIDGE_OP_TYPE_ADD_BR && type != BRIDGE_OP_TYPE_DEL_BR) {
        ret = -EINVAL;
        goto exit;
    }

    br_ctrl_fd = wSOCKET(AF_LOCAL, SOCK_STREAM, 0);
    if (br_ctrl_fd < 0) {
        ret = -ENOENT;
        goto exit;
    }

    ret = wIOCTL(br_ctrl_fd, type == BRIDGE_OP_TYPE_ADD_BR ? SIOCBRADDBR : SIOCBRDELBR, brname);
    if (ret < 0) {
        char _br[IFNAMSIZ];
        unsigned long arg[3] = { type == BRIDGE_OP_TYPE_ADD_BR ? BRCTL_ADD_BRIDGE : BRCTL_DEL_BRIDGE,
                (unsigned long) _br };

        wSTRNCPY(_br, brname, IFNAMSIZ);
        ret = wIOCTL(br_ctrl_fd, SIOCSIFBR, arg);
    }

    if (ret < 0)
        ret = errno;

exit:
    DPRINT_INFO(WFA_ERR, "%s(), brname: %s, type: %d, br_ctrl_fd: %d, ret: %d (%s)\n",
            __func__, brname, type, br_ctrl_fd, ret, ret == 0 ? "NULL" : strerror(errno));
    if (br_ctrl_fd >= 0)
        wCLOSE(br_ctrl_fd);

    return ret;
}

static int mtk_bridge_intf_setup(const char *bridge, const char *dev,
        enum ENUM_BRIDGE_OP_TYPE type)
{
    struct ifreq ifr;
    int ret = 0;
    int ifindex = if_nametoindex(dev);
    int br_ctrl_fd = -1;

    if (type != BRIDGE_OP_TYPE_ADD_INTF && type != BRIDGE_OP_TYPE_DEL_INTF) {
        ret = -EINVAL;
        goto exit;
    }

    br_ctrl_fd = wSOCKET(AF_LOCAL, SOCK_STREAM, 0);
    if (br_ctrl_fd < 0) {
        ret = -ENOENT;
        goto exit;
    }

    if (ifindex == 0) {
        ret = -ENODEV;
        goto exit;
    }

    wSTRNCPY(ifr.ifr_name, bridge, IFNAMSIZ);
    ifr.ifr_ifindex = ifindex;
    ret = wIOCTL(br_ctrl_fd, type == BRIDGE_OP_TYPE_ADD_INTF ? SIOCBRADDIF : SIOCBRDELIF, &ifr);
    if (ret < 0) {
        unsigned long args[4] = { type == BRIDGE_OP_TYPE_ADD_INTF ? BRCTL_ADD_IF : BRCTL_DEL_IF,
                ifindex, 0, 0 };

        ifr.ifr_data = (char *) args;
        ret = wIOCTL(br_ctrl_fd, SIOCDEVPRIVATE, &ifr);
    }

    if (ret < 0)
        ret = errno;

exit:
    DPRINT_INFO(WFA_ERR, "%s(), bridge: %s, dev: %s, type: %d, br_ctrl_fd: %d, ret: %d (%s)\n",
            __func__, bridge, dev, type, br_ctrl_fd, ret, ret == 0 ? "NULL" : strerror(errno));
    if (br_ctrl_fd >= 0)
        wCLOSE(br_ctrl_fd);

    return ret;
}

char *getApMainInterface()
{
    if (!is_interface_up(USB_INF_NAME)) {
        return gDut.ap_params.inf_name;
    } else {
        return AP_BRIDGE_INF_NAME;
    }
}
#endif

#ifdef CONFIG_MTK_PPR2
int wfaStaAddCredential(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf) {
    dutCmdResponse_t *resp = &gGenericResp;
    caStaAddCred_t *credInfo = (caStaAddCred_t *) caCmdBuf;
    int id;
    char *ifname = credInfo->intf;

    id = add_cred(ifname);
    if (id < 0) {
        DPRINT_WARNING(WFA_WNG, "Failed to add credential!\n");
        resp->status = STATUS_ERROR;
        goto exit;
    }

    if (credInfo->prefer > 0)
        set_cred(ifname, id, "priority", "1");

    if (credInfo->realm[0] != '\0' && set_cred_quoted(ifname, id, "realm", credInfo->realm) < 0) {
        DPRINT_WARNING(WFA_WNG, "Failed to set realm!\n");
        resp->status = STATUS_ERROR;
        goto exit;
    }

    if (credInfo->username[0] != '\0' &&
            set_cred_quoted(ifname, id, "username", credInfo->username) < 0) {
        DPRINT_WARNING(WFA_WNG, "Failed to set username!\n");
        resp->status = STATUS_ERROR;
        goto exit;
    }

    if (credInfo->pwd[0] != '\0' && set_cred_quoted(ifname, id, "password", credInfo->pwd) < 0) {
        DPRINT_WARNING(WFA_WNG, "Failed to set password!\n");
        resp->status = STATUS_ERROR;
        goto exit;
    }

    if (credInfo->rootCA[0] != '\0') {
        char fname[200];
        wSNPRINTF(fname, sizeof(fname), HS20_PATH"/%s", credInfo->rootCA);

        if (chk_exist(fname) == 0) {
            DPRINT_WARNING(WFA_WNG, "ROOT_CA file (%s) not found!\n", fname);
            resp->status = STATUS_ERROR;
            goto exit;
        }

        if (set_cred_quoted(ifname, id, "ca_cert", fname) < 0) {
            DPRINT_WARNING(WFA_WNG, "Colud not set root CA!\n");
            resp->status = STATUS_ERROR;
            goto exit;
        }
    }

    resp->status = STATUS_COMPLETE;
exit:
    wfaEncodeTLV(WFA_STA_ADD_CREDENTIAL_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)resp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

    return WFA_SUCCESS;
}

int wfaStaBssidPool(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf) {
    dutCmdResponse_t *resp = &gGenericResp;
    caStaBssidPool_t *staBssidPool = (caStaBssidPool_t *) caCmdBuf;
    char bssidList[200];
    int res, bssidListLen;

    if (staBssidPool->bssidFilter == 0 || staBssidPool->bssidList[0] == '\0') {
        DPRINT_WARNING(WFA_WNG, "Unexpected UCC params!\n");
        strcpy(resp->cmdru.info, "Unexpected UCC params!");
        resp->status = STATUS_ERROR;
        goto exit;
    }

    wSNPRINTF(bssidList, sizeof(bssidList), "SET bssid_filter %s", staBssidPool->bssidList);
    DPRINT_INFO(WFA_OUT, "bssid list from UCC:%s\n", staBssidPool->bssidList);
    // For PPR3 5.3-A, UCC command have many trailing space, and lead command fail
    // remove trailing space
    bssidListLen = strlen(bssidList);
    while (bssidListLen > 0 && isspace(bssidList[bssidListLen - 1])) {
        bssidListLen--;
    }
    bssidList[bssidListLen] = '\0';
    DPRINT_INFO(WFA_OUT, "bssid list after removing spaces:%s\n", bssidList);
    if (wpa_command(staBssidPool->intf, bssidList) < 0) {
        DPRINT_WARNING(WFA_WNG, "Fail to enable BSSID filter!\n");
        strcpy(resp->cmdru.info, "Fail to enable BSSID filter");
        resp->status = STATUS_ERROR;
        goto exit;
    }

    resp->status = STATUS_COMPLETE;

exit:
    wfaEncodeTLV(WFA_STA_BSSID_POOL_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)resp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

    return WFA_SUCCESS;
}

int wfaStaHs2Associate(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf) {
    dutCmdResponse_t *resp = &gGenericResp;
    caStaHs2Associate_t *staHs2Associate = (caStaHs2Associate_t *) caCmdBuf;
    char *ifname = staHs2Associate->intf;
    int ignoreBlackList = staHs2Associate->ignoreBlackList;
    int tries = 0;
    int res;
    char bssid[100], ssid[100], buf[100], blackList[100], wpa_state[100], arping[100];
    struct wpa_ctrl *ctrl = NULL;
    const char *events[] = {
        "CTRL-EVENT-CONNECTED",
        "INTERWORKING-BLACKLISTED",
        "INTERWORKING-NO-MATCH",
        NULL
    };

    DPRINT_INFO(WFA_OUT, "Entering %s()\n", __func__);
    blackList[0] = '\0';
    arping[0]='\0';

try_again:
    ctrl = open_wpa_mon(ifname);
    if (ctrl == NULL) {
        DPRINT_WARNING(WFA_WNG, "open wpa mon fail.\n");
        strcpy(resp->cmdru.info, "Failed to open wpa monitor");
        resp->status = STATUS_ERROR;
        goto exit;
    }

    tries++;
    if (wpa_command(ifname, "INTERWORKING_SELECT auto") < 0) {
        DPRINT_WARNING(WFA_WNG, "wpa cmd fail, failed to start interworking connection\n");
        resp->status = STATUS_ERROR;
        strcpy(resp->cmdru.info, "Failed to start interworking connection");
        wpa_ctrl_detach(ctrl);
        wpa_ctrl_close(ctrl);
        goto exit;
    }

    buf[0] = '\0';
    while(1) {
        char *pos;
        res = get_wpa_cli_events(ctrl, events, buf, sizeof(buf));
        pos = strstr(buf, "INTERWORKING-BLACKLISTED");
        if (!pos)
            break;
        pos += 25;
        DPRINT_INFO(WFA_OUT, "Found blacklisted AP: %s\n", pos);
        if (!blackList[0])
            memcpy(blackList, pos, strlen(pos) + 1);
    }

    if (ignoreBlackList && blackList[0]) {
        char *end;
        int r;
        end = strchr(blackList, ' ');
        if (end)
            *end = '\0';
        DPRINT_INFO(WFA_OUT, "Try to connect to a blacklisted network: %s\n", blackList);
        r = wSNPRINTF(buf, sizeof(buf), "INTERWORKING_CONNECT %s", blackList);
        if (r < 0 || r >= sizeof(buf) || wpa_command(ifname, buf)) {
            resp->status = STATUS_ERROR;
            strcpy(resp->cmdru.info,
                    "Failed to start Interworking connection to blacklisted network");
            wpa_ctrl_detach(ctrl);
            wpa_ctrl_close(ctrl);
            goto exit;
        }
        res = get_wpa_cli_event(ctrl, "CTRL-EVENT-CONNECTED", buf, sizeof(buf));
    }

    wpa_ctrl_detach(ctrl);
    wpa_ctrl_close(ctrl);

    if (res < 0) {
        resp->status = STATUS_ERROR;
        strcpy(resp->cmdru.info, "Could not connect to network");
        goto exit;
    }

    if (strstr(buf, "INTERWORKING-NO-MATCH") ||
        strstr(buf, "INTERWORKING-BLACKLISTED")) {
        if (tries < 2) {
            DPRINT_INFO(WFA_OUT,
                    "No match found - try again to verify no APs were missed in the scan\n");
            goto try_again;
        }
        resp->status = STATUS_ERROR;
        strcpy(resp->cmdru.info, "No network with matching credentials found");
        goto exit;
    }

    if (get_wpa_status(ifname, "bssid", bssid, sizeof(bssid)) < 0 ||
        get_wpa_status(ifname, "ssid", ssid, sizeof(ssid)) < 0) {
        resp->status = STATUS_ERROR;
        strcpy(resp->cmdru.info, "Could not get current BSSID/SSID");
        DPRINT_INFO(WFA_OUT, "Could not get current BSSID/SSID\n");
        goto exit;
    }
    DPRINT_INFO(WFA_OUT, "bssid=%s ssid=%s\n", bssid, ssid);

    int ret = get_wpa_status(ifname, "wpa_state", wpa_state, sizeof(wpa_state));
    DPRINT_INFO(WFA_OUT, "wpa_state=%s\n", wpa_state);

    if (ret < 0) {
        resp->status = STATUS_ERROR;
        strcpy(resp->cmdru.info, "Get wpa_state failed");
        DPRINT_INFO(WFA_OUT, "Get wpa_state failed\n");
        goto exit;
    } else {
        if (wSTRNCMP(wpa_state, "COMPLETED", 9) == 0) {
            if (gDut.sta_params.ipconfig.isDhcp) {
                enable_dhcp_client(ifname);
            } else {
                int res = wSNPRINTF(arping, sizeof(arping), "arping -c 4 -D -I %s -s 0.0.0.0 %s",
                        gDut.sta_params.ipconfig.intf, gDut.sta_params.ipconfig.ipaddr);
                if (res < 0 || res >= (int) sizeof(arping)) {
                    resp->status = STATUS_ERROR;
                    DPRINT_WARNING(WFA_WNG, "Failed to copy strings to buf\n");
                    goto exit;
                }

                DPRINT_INFO(WFA_OUT, "Check duplicate IPv4 address: %s\n", arping);
                if(system(arping) != 0) {
                    DPRINT_WARNING(WFA_WNG, "Failed run arping: %s\n", arping);
                    resp->status = STATUS_ERROR;
                    goto exit;
                }
            }
            update_ip_config(NULL);
        } else {
            DPRINT_INFO(WFA_OUT, "DUT haven't connected yet\n");
        }
    }

    wSNPRINTF(resp->cmdru.info, sizeof(resp->cmdru.info), "SSID,%s,BSSID,%s", ssid, bssid);
    resp->status = STATUS_COMPLETE;
    sta_set_priority();

exit:
    wfaEncodeTLV(WFA_STA_HS2_ASSOCIATE_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)resp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

    return WFA_SUCCESS;
}

int wfaStaOsu(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf) {
    dutCommand_t *params = (dutCommand_t *)caCmdBuf;
    char *ifname = params->intf;
    char cmd[300], bssid[100], ssid[100], buf[200];
    dutCmdResponse_t *resp = &gGenericResp;
    int res;

    DPRINT_INFO(WFA_OUT, "Entering %s()\n", __func__);
    /* kill dhcpcd */
    system("killall dhcpcd");
    /* start DHCP */
    DPRINT_INFO(WFA_OUT, "Start DHCP client\n");
    system(DHCPCD" -b wlan0");
    system(DHCPCD" -t 0 wlan0 &");

    mkdir(HS20_PATH"/Logs", S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
    res = wSNPRINTF(cmd, sizeof(cmd), "signup " HS20_PATH "/Certs/osu-ca.pem");
    DPRINT_INFO(WFA_OUT, "Trigger OSU: %s()\n", cmd);
    sta_set_priority();

    if (sta_run_osu_client(cmd) == 0) {
        FILE *f;

        DPRINT_WARNING(WFA_WNG, "Failed to complete OSU!\n");

        f = fopen(HS20_PATH"/hs20-osu-client.res", "r");
        if (f) {
            char res[300], *pos;
            if (!fgets(res, sizeof(res), f))
                res[0] = '\0';
            pos = strchr(res, '\n');
            if (pos)
                *pos = '\0';
            fclose(f);
            DPRINT_WARNING(WFA_WNG, "hs20-osu-client provisioning failed: %s\n", res);

            resp->status = STATUS_ERROR;
            wSTRNCPY(resp->cmdru.staOsuInfo.failReason, res,
                    sizeof(resp->cmdru.staOsuInfo.failReason));
            resp->cmdru.staOsuInfo.failReason[sizeof(
                    resp->cmdru.staOsuInfo.failReason) - 1] = '\0';
        } else {
            DPRINT_WARNING(WFA_WNG, "Failed to open: " HS20_PATH "/hs20-osu-client.res\n");
        }
    }

    if (sta_mon_conn(ifname) < 0) {
        DPRINT_WARNING(WFA_WNG, "hs20-osu-client connect failed\n");
        resp->status = STATUS_ERROR;
        strcpy(resp->cmdru.staOsuInfo.failReason, "hs20-osu-client connect failed");
        resp->cmdru.staOsuInfo.failReason[sizeof(
                resp->cmdru.staOsuInfo.failReason) - 1] = '\0';
    } else {
        /* DUT connected */
        if (get_wpa_status(ifname, "bssid", bssid, sizeof(bssid)) < 0 ||
                get_wpa_status(ifname, "ssid", ssid, sizeof(ssid)) < 0) {
            resp->status = STATUS_ERROR;
            strcpy(resp->cmdru.staOsuInfo.failReason, "hs20-osu-client can't find bssid or ssid");
            resp->cmdru.staOsuInfo.failReason[sizeof(
                    resp->cmdru.staOsuInfo.failReason) - 1] = '\0';
        } else {
            update_ip_config(NULL);
            resp->status = STATUS_COMPLETE;
            strcpy(resp->cmdru.staOsuInfo.ssid, ssid);
            resp->cmdru.staOsuInfo.ssid[sizeof(
                    resp->cmdru.staOsuInfo.ssid) - 1] = '\0';
            strcpy(resp->cmdru.staOsuInfo.bssid, bssid);
            resp->cmdru.staOsuInfo.bssid[sizeof(
                    resp->cmdru.staOsuInfo.bssid) - 1] = '\0';
        }
    }

    wfaEncodeTLV(WFA_STA_OSU_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)resp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

    return WFA_SUCCESS;
}

int wfaStaPolicyUpdate(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf) {
    dutCmdResponse_t *resp = &gGenericResp;
    // Currently we don't care about the parameters from UCC
    // so ignore caCmdBuf!

    DPRINT_INFO(WFA_OUT, "Entering %s()\n", __func__);

    mkdir(HS20_PATH"/Logs", S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
    if (sta_run_osu_client("pol_upd fqdn=wi-fi.org") == 0) {
        resp->status = STATUS_ERROR;
    } else {
        resp->status = STATUS_COMPLETE;
    }
    DPRINT_WARNING(WFA_WNG, "Policy update %s\n", resp->status == STATUS_COMPLETE ?
            "successful" : "failed");

    wfaEncodeTLV(WFA_STA_POLICY_UPDATE_RESP_TLV, sizeof(dutCmdResponse_t), (BYTE *)resp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

    return WFA_SUCCESS;
}

int wfaStaResetParm(int len, BYTE *caCmdBuf, int *respLen, BYTE *respBuf)
{
    dutCmdResponse_t *resp = &gGenericResp;

    DPRINT_INFO(WFA_OUT, "Entering %s(), do nothing.\n", __func__);

    resp->status = STATUS_COMPLETE;
    wfaEncodeTLV(WFA_STA_RESET_PARM_RESP_TLV, sizeof(dutCmdResponse_t),
            (BYTE *)resp, respBuf);
    *respLen = WFA_TLV_HDR_LEN + sizeof(dutCmdResponse_t);

    return WFA_SUCCESS;
}
#endif


static char *get_main_intf()
{
#ifdef CONFIG_MTK_P2P
    char buf[4096];

    if (wpa_command_resp("wlan0", "INTERFACES", buf, sizeof(buf)) < 0) {
        DPRINT_WARNING(WFA_WNG, "wpa cmd fail, get interface\n");
        return "";
    }
    if (strstr(buf, "p2p0"))
        return "p2p0";
    else
#endif
        return "wlan0";
}

#ifdef CONFIG_MTK_P2P
static void init_p2p_params()
{
    static pthread_t event_thread;
    char *ifname = "p2p0"; // TODO: hardcode p2p0 interface name

    if (strcmp(get_main_intf(), ifname) != 0)
        return;

    pthread_create(&event_thread, NULL, &p2p_event_receive, ifname);
}

static int p2p_is_peer_known(const char *ifname, const char *peer, int discovered)
{
    char buf[4096];

    wSNPRINTF(buf, sizeof(buf), "P2P_PEER %s", peer);
    if (wpa_command_resp(ifname, buf, buf, sizeof(buf)) < 0) {
        DPRINT_WARNING(WFA_WNG, "wpa cmd fail, %s\n", buf);
        return 0;
    }
    DPRINT_INFO(WFA_OUT, "p2p_is_peer_known buf: %s\n", buf);
    if (strncasecmp(buf, peer, strlen(peer)) != 0)
        return 0;
    if (!discovered)
        return 1;
    return strstr(buf, "[PROBE_REQ_ONLY]") == NULL ? 1 : 0;
}

/**
 * p2p_find_peer - Find p2p peer
 * @ifname: interface name for p2p control interface
 * @peer: peer's mac address
 * @discovered: Fully discovered, i.e. which we have only seen in a received
 *              Probe Request frame.
 * Returns: 1 if peer is found, 0 if peer can NOT be found, or -1 on failure
 */
static int p2p_find_peer(const char *ifname, const char *peer, int discovered)
{
    int count = 0;

    DPRINT_INFO(WFA_OUT, "ifname: %s, peer: %s, discovered: %d\n",
            ifname, peer, discovered);

    if (p2p_is_peer_known(ifname, peer, discovered))
        return 1;

    if (wpa_command(ifname, "P2P_FIND type=progressive") < 0) {
        DPRINT_WARNING(WFA_WNG, "wpa cmd fail, P2P_FIND type=progressive\n");
        return -1;
    }

    while (count < 120) {
        count++;
        wSLEEP(1);
        if (p2p_is_peer_known(ifname, peer, discovered)) {
            DPRINT_INFO(WFA_OUT, "restore to previous state\n");
            switch (gDut.p2p_params.p2p_mode) {
            case P2P_MODE_IDLE:
                wpa_command(ifname, "P2P_STOP_FIND");
                break;
            case P2P_MODE_LISTEN:
                wpa_command(ifname, "P2P_LISTEN");
                break;
            }
            return 1;
        }
    }
    DPRINT_WARNING(WFA_WNG, "p2p find timeout, can not find %s\n", peer);
    return 0;
}

static void enable_dhcp_client(char *ifname)
{
    int rc = -1;
    uint32_t ipaddr;
    uint32_t gateway;
    uint32_t prefixLength;
    uint32_t dns1;
    uint32_t dns2;
    uint32_t server;
    uint32_t lease;

    DPRINT_INFO(WFA_OUT, "Entering %s()\n", __func__);

    if (ifc_init())
        DPRINT_WARNING(WFA_WNG, "init dhcp client fail\n");

    rc = do_dhcp(ifname);

    if (rc)
        DPRINT_WARNING(WFA_WNG, "do dhcp client fail\n");

    ifc_close();
}

static void enable_dhcp(char *ifname, int enable, int go)
{
    DPRINT_INFO(WFA_OUT, "Entering %s(), ifname: %s, enable: %d, go: %d\n",
            __func__, ifname, enable, go);

    if (enable) {
        if (go) {
            if (shell("ifconfig %s %s up", ifname, "192.168.49.1") !=
                    WFA_SUCCESS)
                DPRINT_WARNING(WFA_WNG, "setup p2p GO ip fail\n");
            if (shell("ip rule add pref 9999 from all fwmark 0x0/0xffff table main") !=
                    WFA_SUCCESS)
                DPRINT_WARNING(WFA_WNG, "setup p2p ip table fail\n");
            if (shell("ndc tether start 192.168.49.100 192.168.49.200") != WFA_SUCCESS)
                DPRINT_WARNING(WFA_WNG, "start p2p dhcp server fail\n");
        } else {
            enable_dhcp_client(ifname);
        }
    } else {
        if (go) {
            if (shell("ndc tether stop") != WFA_SUCCESS)
                DPRINT_WARNING(WFA_WNG, "stop p2p dhcp server fail\n");
        } else {
            if (shell("ifconfig %s 0.0.0.0", ifname) != WFA_SUCCESS)
                DPRINT_WARNING(WFA_WNG, "clear p2p ip addr fail\n");
        }
    }
}

static void *p2p_event_receive(void* data)
{
    struct wpa_ctrl *ctrl;
    int i;
    struct timeval tv;
    char buf[256];
    int go = 0;
    int fd, ret;
    fd_set rfd;
    size_t len;
    char *pos;
    char *ifname = (char*) data;
    const char *events[] = {
            "P2P-GROUP-STARTED",
            "P2P-GROUP-REMOVED",
            NULL
    };

    DPRINT_INFO(WFA_OUT, "Entering %s()\n", __func__);

    ctrl = open_wpa_mon(ifname);
    if (!ctrl) {
        DPRINT_WARNING(WFA_WNG, "open wpa mon for p2p0 fail\n");
        goto exit;
    }

    fd = wpa_ctrl_get_fd(ctrl);
    if (fd < 0) {
        DPRINT_WARNING(WFA_WNG, "get fd fail for wpa ctrl\n");
        goto exit;
    }

    while (!gDut.p2p_params.stop_event_thread) {
        FD_ZERO(&rfd);
        FD_SET(fd, &rfd);
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        ret = select(fd + 1, &rfd, NULL, NULL, &tv);
        if (ret == 0)
            continue;
        if (ret < 0) {
            DPRINT_WARNING(WFA_WNG, "select fail, %s\n", strerror(errno));
            wUSLEEP(100000);
            continue;
        }

        len = sizeof(buf);
        if (wpa_ctrl_recv(ctrl, buf, &len) < 0) {
            DPRINT_WARNING(WFA_WNG, "fail waiting for events\n");
            continue;
        }
        if (len >= sizeof(buf))
            len = sizeof(buf) - 1;
        buf[len] = '\0';

        ret = 0;
        pos = strchr(buf, '>');
        if (pos) {
            for (i = 0; events[i]; i++) {
                if (strncmp(pos + 1, events[i], strlen(events[i])) == 0) {
                    ret = 1;
                    break; /* Event found */
                }
            }
        }
        if (!ret)
            continue;

        /*
         * output format:
         * <3>P2P-GROUP-STARTED p2p0 GO ssid="DIRECT-lF6.1.11" freq=2462 passphrase="ViGQq96f" go_dev_addr=02:08:22:5e:8d:55 [PERSISTENT]
         */

        DPRINT_INFO(WFA_OUT, "%s(), receive event: %s\n", __func__, buf);
        if (strstr(buf, "P2P-GROUP-")) {
            char *group_type, *pos, *ssid;

            pos = strchr(buf, ' ');
            if (!pos)
                continue;
            pos++;
            pos = strchr(pos, ' ');
            if (!pos)
                continue;
            *pos++ = '\0';
            group_type = pos;
            pos = strchr(group_type, ' ');
            if (!pos)
                continue;
            *pos++ = '\0';
            go = strcmp(group_type, "GO") == 0;
            DPRINT_INFO(WFA_OUT, "%s(), group_type: %s\n", __func__,
                    group_type);

            /* save ssid for getpsk */
            ssid = strstr(pos, "ssid=\"");
            if (ssid != NULL && go) {
                ssid += 6;
                pos = strchr(ssid, '"');
                if (pos != NULL) {
                    *pos++ = '\0';
                    wSTRNCPY(gDut.p2p_params.current_ssid, ssid,
                            WFA_SSID_NAME_LEN);
                    DPRINT_INFO(WFA_OUT, "%s(), current_ssid: %s\n",
                            __func__,
                            gDut.p2p_params.current_ssid);
                }
            }
        }

        if (strstr(buf, "P2P-GROUP-STARTED")) {
            enable_dhcp(ifname, 1, go);
        } else if (strstr(buf, "P2P-GROUP-REMOVED")) {
            enable_dhcp(ifname, 0, go);
            go = 0;
            wMEMSET(&gDut.p2p_params.current_ssid, 0, WFA_SSID_NAME_LEN);
        }
    }

exit:
    enable_dhcp(ifname, 0, go);
    if (ctrl) {
        wpa_ctrl_detach(ctrl);
        wpa_ctrl_close(ctrl);
    }
    pthread_exit(0);
    return NULL;
}

static void remove_p2p_persistent_networks(const char *ifname)
{
    char buf[4096];
    char cmd[256];
    char *pos;

    DPRINT_INFO(WFA_OUT, "Entering %s()\n", __func__);

    if (wpa_command_resp(ifname, "LIST_NETWORKS", buf, sizeof(buf)) < 0)
        return;

    /*
     * output format:
     * network id / ssid / bssid / flags
     * 1       DIRECT-rB7.1.2  02:08:22:fe:8b:51       [DISABLED][P2P-PERSISTENT]
     */

    pos = strchr(buf, '\n');
    if (pos == NULL)
        return;
    pos++;
    while (pos && pos[0]) {
        int id = atoi(pos);
        wSNPRINTF(cmd, sizeof(cmd), "REMOVE_NETWORK %d", id);
        if (wpa_command(ifname, cmd) < 0) {
            DPRINT_ERR(WFA_ERR, "wpa cmd fail: %s\n", cmd);
            continue;
        }
        pos = strchr(pos, '\n');
        if (pos)
            pos++;
    }
}
#endif

void dump_wmm_paraters()
{
    struct wmm_param {
        char *name;
        char *key;
    };
    struct wmm_param wmm_params[] = {
        {"BK", "0x820f31a0"},
        {"BE", "0x820f31a4"},
        {"VI", "0x820f31a8"},
        {"VO", "0x820f31ac"},
        {"TXOP[VO|VI]", "0x820f4010"},
        {"TXOP[BE|BK]", "0x820f4014"},
    };
    int i = 0;
    int length = sizeof(wmm_params) / sizeof(struct wmm_param);

    switch (gDut.driver_gen) {
    case GEN_4m:
        /* Enable driver dump */
        shell(IWPRIV" wlan0 driver 'set_mcr 2011 2011'");
        for (i = 0; i < length; i++) {
            if (shell(IWPRIV" wlan0 driver 'get_mcr %s'", wmm_params[i].key) ==
                    WFA_SUCCESS) {
                DPRINT_INFO(WFA_OUT, "%s: %s\n", wmm_params[i].name,
                        gDut.shellResult);
            } else {
                DPRINT_WARNING(WFA_WNG, "get wmm parameter fail for %s\n",
                        wmm_params[i].name);
            }
        }
        break;
    default:
        break;
    }
}

int get_ip_config(const char* intf, caStaGetIpConfigResp_t *ifinfo)
{
    char *buf = NULL;
    char *buf_save = NULL;

    if (shell(GETIPCONFIG" %s", intf) != WFA_SUCCESS)
        return WFA_FAILURE;

    wMEMSET(ifinfo, 0, sizeof(*ifinfo));
    buf = strtok_r(gDut.shellResult, "\r\n", &buf_save);
    while (buf) {
        if (!wSTRNCMP(buf, "mac_addr=", 9)) {
            wSTRNCPY(ifinfo->mac, buf + 9, sizeof(ifinfo->mac) - 1);
        } else if (!wSTRNCMP(buf, "ip=", 3)) {
            wSTRNCPY(ifinfo->ipaddr, buf + 3, sizeof(ifinfo->ipaddr) - 1);
        } else if (!wSTRNCMP(buf, "mask=", 5)) {
            wSTRNCPY(ifinfo->mask, buf + 5, sizeof(ifinfo->mask) - 1);
        } else if (!wSTRNCMP(buf, "dns1=", 5)) {
            wSTRNCPY(ifinfo->dns[0], buf + 5, sizeof(ifinfo->dns[0]) - 1);
        } else if (!wSTRNCMP(buf, "dns2=", 5)) {
            wSTRNCPY(ifinfo->dns[1], buf + 5, sizeof(ifinfo->dns[1]) - 1);
        }
        buf = strtok_r(NULL, "\r\n", &buf_save);
    }
    return WFA_SUCCESS;
}

int get_mac(const char* intf, unsigned char* addr)
{
    int s;
    struct ifreq ifr;

    DPRINT_INFO(WFA_OUT, "Entering %s()\n", __func__);

    wMEMSET(addr, 0, 6);
    s = wSOCKET(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        DPRINT_ERR(WFA_ERR, "socket create fail.\n");
        return STATUS_ERROR;
    }
    wMEMSET(&ifr, 0, sizeof(ifr));
    wSTRNCPY(ifr.ifr_name, intf, sizeof(ifr.ifr_name) - 1);
    if (wIOCTL(s, SIOCGIFHWADDR, &ifr) < 0) {
        wCLOSE(s);
        DPRINT_ERR(WFA_ERR, "ioctl get addr fail.\n");
        return STATUS_ERROR;
    }
    wMEMCPY(addr, ifr.ifr_hwaddr.sa_data, 6);
    wCLOSE(s);

    return STATUS_COMPLETE;
}

#ifdef CONFIG_MTK_PPR2
static int sta_ap_exsit(const char *ifname, const char *bssid)
{
    char buf[4096];

    wSNPRINTF(buf, sizeof(buf), "BSS %s MASK=1", bssid);
    if (wpa_command_resp(ifname, buf, buf, sizeof(buf)) < 0)
        return 0;
    if (strncmp(buf, "id=", 3) != 0)
        return 0;
    return 1;
}

static int sta_find_ap(const char *ifname, const char *bssid)
{
    int res;
    struct wpa_ctrl *ctrl;
    char buf[256];

    if (sta_ap_exsit(ifname, bssid))
        return 0;
    DPRINT_INFO(WFA_OUT, "AP not in BSS table, try to find it\n");

    ctrl = open_wpa_mon(ifname);
    if (!ctrl) {
        DPRINT_WARNING(WFA_WNG, "Failed to open wpa_supplicant monitor\n");
        return -1;
    }

    if (wpa_command(ifname, "SCAN") < 0) {
        DPRINT_WARNING(WFA_WNG, "Failed to start scan\n");
        wpa_ctrl_detach(ctrl);
        wpa_ctrl_close(ctrl);
        return -1;
    }

    res = get_wpa_cli_event(ctrl, "CTRL-EVENT-SCAN-RESULTS", buf, sizeof(buf));

    wpa_ctrl_detach(ctrl);
    wpa_ctrl_close(ctrl);

    if (res < 0) {
        DPRINT_WARNING(WFA_WNG, "Scan did not complete\n");
        return -1;
    }

    if (sta_ap_exsit(ifname, bssid))
        return 0;
    DPRINT_INFO(WFA_OUT, "AP not in BSS table\n");

    return -1;
}

static int sta_run_osu_client(const char *params)
{
    char buf[300], cmd[200];
    int res;

    res = wSNPRINTF(cmd, sizeof(cmd),
            HS20_PATH"/hs20-osu-client -w " CONFIG_CTRL_IFACE_CLIENT_DIR "/ "
            "-r " HS20_PATH "/hs20-osu-client.res -dddKt -f "
            HS20_PATH "/Logs/hs20-osu-client.txt");
    if (res < 0 || res >= (int) sizeof(cmd)) {
        DPRINT_WARNING(WFA_WNG, "Failed to copy strings to cmd\n");
        return 0;
    }

    res = wSNPRINTF(buf, sizeof(buf), "%s %s", cmd, params);
    if (res < 0 || res >= (int) sizeof(buf)) {
        DPRINT_WARNING(WFA_WNG, "Failed to copy strings to buf\n");
        return 0;
    }

    DPRINT_INFO(WFA_OUT, "Start hs20 osu client: %s\n", buf);
    if(system(buf) != 0) {
        DPRINT_WARNING(WFA_WNG, "Failed to run osu client: %s\n", buf);
        return 0;
    }

    DPRINT_INFO(WFA_OUT, "Run hs20-osu-client successfully\n", buf);
    return 1;
}

static int sta_download_ppsmo(const char *ifname, const char *filename, const char *filepath,
        const char *managementtreeuri)
{
    char path[500], buf[600], fbuf[100];
    char *fqdn = NULL;
    path[0] = '\0';

    if (filename == NULL || filepath == NULL) {
        DPRINT_WARNING(WFA_WNG, "Invalid filename or filepath\n");
        return 0;
    }

    if (strcasecmp(filepath, "VendorSpecific") == 0) {
        wSNPRINTF(path, sizeof(path), HS20_PATH"/PPS/%s", filename);
        DPRINT_INFO(WFA_OUT, "Use pre-configured PPS MO: %s\n", path);
        if (chk_exist(path) == 0) {
            DPRINT_INFO(WFA_OUT, "Requested PPS MO file doesn't exist!\n");
            return 0;
        }

        wSNPRINTF(buf, sizeof(buf), "cp %s " HS20_PATH "/pps-tnds.xml", path);
        if (system(buf) != 0) {
            DPRINT_WARNING(WFA_WNG, "Failed to copy PPS MO file\n");
            return 0;
        }
    } else {
        DPRINT_WARNING(WFA_WNG, "Unsupported filepath:%s, we ONLY support pre-configed PPS MO!\n",
                filepath);
        return 0;
    }

    if (sta_run_osu_client("from_tnds " HS20_PATH "/pps-tnds.xml " HS20_PATH "/pps.xml") == 0) {
        DPRINT_WARNING(WFA_WNG, "Failed to parse download PPSMO!\n");
        return 0;
    }
    unlink(HS20_PATH"/pps-tnds.xml");

    if (managementtreeuri) {
        const char *pos, *end;
        DPRINT_INFO(WFA_OUT, "managementTreeURI: %s\n", managementtreeuri);

        if (strncmp(managementtreeuri, "./Wi-Fi/", 8) != 0) {
            DPRINT_WARNING(WFA_WNG, "Invalid managementTreeURI prefiex!\n");
            return 0;
        }
        pos = managementtreeuri + 8;
        end = strchr(pos, '/');
        if (end == NULL || strcmp(end, "/PerProviderSubscription") != 0) {
            DPRINT_WARNING(WFA_WNG, "Invalid managementTreeURI postfix!\n");
            return 0;
        }
        if (end - pos >= (int) sizeof(fbuf)) {
            DPRINT_WARNING(WFA_WNG, "Too long FQDN in managementTreeURI!\n");
            return 0;
        }
        memcpy(fbuf, pos, end - pos);
        fbuf[end - pos] = '\0';
        fqdn = fbuf;
        DPRINT_INFO(WFA_WNG, "FQDN from managementTreeURI: %s\n", fqdn);
    } else if (sta_run_osu_client("from_tnds " HS20_PATH "/pps.xml") == 1) {
        FILE *f = fopen(HS20_PATH"/pps-fqdn", "r");
        if (f) {
            if (fgets(fbuf, sizeof(fbuf), f)) {
                fbuf[sizeof(fbuf) - 1] = '\0';
                fqdn = fbuf;
                DPRINT_INFO(WFA_WNG, "Use FQDN: %s\n", fqdn);
            }
            fclose(f);
        } else {
            DPRINT_WARNING(WFA_WNG, "Cannot find pps-fqdn: %s!\n", HS20_PATH"/pps-fqdn");
        }
    }

    if (fqdn == NULL) {
        DPRINT_WARNING(WFA_WNG, "No FQDN found or specified!\n");
    }

    mkdir(HS20_PATH"/SP", S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
    wSNPRINTF(buf, sizeof(buf), HS20_PATH"/SP/%s", fqdn);
    mkdir(buf, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);

    wSNPRINTF(buf, sizeof(buf),
        "mv " HS20_PATH "/pps.xml " HS20_PATH "/SP/%s/pps.xml", fqdn);
    if (system(buf)) {
        DPRINT_WARNING(WFA_WNG, "Could not move PPS MO!\n");
        return 0;
    }

    // Suppose filePath must be VendorSpecific
    wSNPRINTF(buf, sizeof(buf), "cp " HS20_PATH "/Certs/ca.pem " HS20_PATH "/SP/%s/ca.pem", fqdn);
    if (system(buf)) {
        DPRINT_WARNING(WFA_WNG, "Failed to copy OSU CA cert(ca.pem)!\n");
        return 0;
    }

    wSNPRINTF(buf, sizeof(buf), "cp " HS20_PATH "/Certs/aaa-ca.pem " HS20_PATH "/SP/%s/aaa-ca.pem",
            fqdn);
    if (system(buf)) {
        DPRINT_WARNING(WFA_WNG, "Failed to copy AAA CA cert(aaa-ca.pem)!\n");
        return 0;
    }

    if (chk_exist(HS20_PATH"/next-client-cert.pem")) {
        wSNPRINTF(buf, sizeof(buf),
            "mv " HS20_PATH "/next-client-cert.pem " HS20_PATH "/SP/%s/client-cert.pem", fqdn);
        if (system(buf)) {
            DPRINT_WARNING(WFA_WNG, "Could not move client certificate!\n");
            return 0;
        }
    }

    if (chk_exist(HS20_PATH"/next-client-key.pem")) {
        wSNPRINTF(buf, sizeof(buf),
            "mv " HS20_PATH "/next-client-key.pem " HS20_PATH "/SP/%s/client-key.pem", fqdn);
        if (system(buf)) {
            DPRINT_WARNING(WFA_WNG, "Could not move client key!\n");
            return 0;
        }
    }

    wSNPRINTF(buf, sizeof(buf), "set_pps " HS20_PATH "/SP/%s/pps.xml", fqdn);
    if (sta_run_osu_client(buf) == 0) {
        DPRINT_WARNING(WFA_WNG, "Failed to configure credential from PPSMO!\n");
        return 0;
    }

    return 1;
}

static int sta_download_cert(const char *ifname, const char *filename, const char *filepath)
{
    char path[500], buf[600];
    if (filename == NULL || filepath == NULL) {
        DPRINT_WARNING(WFA_WNG, "Invalid filename or filepath\n");
        return 0;
    }

    if (strcasecmp(filepath, "VendorSpecific") == 0) {
        wSNPRINTF(path, sizeof(path), HS20_PATH"/Certs/%s-cert.pem", filename);
        DPRINT_INFO(WFA_OUT, "Use pre-configured client certificate from the device: %s\n", path);


        if (!chk_exist(path)) {
            DPRINT_WARNING(WFA_WNG, "certificate file does not exist\n");
            return 0;
        }
        wSNPRINTF(buf, sizeof(buf), "cp %s " HS20_PATH "/next-client-cert.pem", path);
        if (system(buf) != 0) {
            DPRINT_WARNING(WFA_WNG, "Failed to copy client certificate\n");
            return 0;
        }

        wSNPRINTF(path, sizeof(path), HS20_PATH"/Certs/%s-key.pem", filename);
        DPRINT_INFO(WFA_OUT, "Use pre-configured client private key from the device: %s\n", path);
        if (!chk_exist(path)) {
            DPRINT_WARNING(WFA_WNG, "Requested private key file does not exist\n");
            return 0;
        }
        wSNPRINTF(buf, sizeof(buf), "cp %s " HS20_PATH "/next-client-key.pem", path);
        if (system(buf) != 0) {
            DPRINT_WARNING(WFA_WNG, "Failed to copy client key\n");
            return 0;
        }
    } else {
        DPRINT_WARNING(WFA_WNG, "Unsupported filepath:%s, we ONLY support pre-configed PPS MO!\n",
                filepath);
        return 0;
    }
    return 1;
}

static int sta_set_priority()
{
    system("ip rule del prio 23000");
    if (system("ip rule add from all lookup main prio 23000") != 0) {
        DPRINT_WARNING(WFA_WNG, "Failed to run:ip rule add from all lookup main prio!\n");
        return 0;
    }
    if (system("ip route flush cache") != 0) {
        DPRINT_WARNING(WFA_WNG, "Failed to run ip route flush cache!\n");
        return 0;
    }
    return 1;
}

#endif

