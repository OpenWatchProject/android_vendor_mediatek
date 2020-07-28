/*
 * Copyright (c) [2017], MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws.
 * The information contained herein is confidential and proprietary to
 * MediaTek Inc. and/or its licensors.
 * Except as otherwise provided in the applicable licensing terms with
 * MediaTek Inc. and/or its licensors, any reproduction, modification, use or
 * disclosure of MediaTek Software, and information contained herein, in whole
 * or in part, shall be strictly prohibited.
*/

#include "includes.h"
#include "common.h"
#include "wpa_supplicant_i.h"
#include "eloop.h"
#include "bss.h"
#include "scan.h"
#include "driver_i.h"
#include "wpa_debug.h"
#include "config.h"
#include "wapi.h"
#include <dlfcn.h>

#ifndef ETH_P_WAI
#define ETH_P_WAI 0x88B4
#endif

#define LIB_FULL_NAME "libwapi.so"

static void *libwapi_handle = NULL;

/* Begin: wrapper definition for wapi interface*/
static int wapi_tx_wai(void *ctx, const u8* pbuf, int length)
{
    struct wpa_supplicant *wpa_s = (struct wpa_supplicant *)ctx;

    int ret;
    ret = l2_packet_send(wpa_s->wapi->l2_wai,
            wpa_s->bssid, ETH_P_WAI,
            (const u8*)pbuf,
            (unsigned  int)length);

    return ret;
}

static int wapi_get_state_helper(void *ctx)
{
    struct wpa_supplicant *wpa_s = (struct wpa_supplicant *)ctx;

    return wpa_s->wpa_state;
}

static void wapi_deauthenticate_helper(void *ctx, int reason_code)
{
    struct wpa_supplicant *wpa_s = (struct wpa_supplicant *)ctx;

    wpa_supplicant_deauthenticate(wpa_s, reason_code);
}

static void wapi_set_state_helper(void *ctx, int state)
{
    struct wpa_supplicant *wpa_s = (struct wpa_supplicant *)ctx;

    wpa_supplicant_set_state(wpa_s,(enum wpa_states) state);
}

static int wapi_set_key_helper(void *_wpa_s, int alg,
                  const u8 *addr, int key_idx, int set_tx,
                  const u8 *seq, size_t seq_len,
                  const u8 *key, size_t key_len)
{
    struct {
        const u8 *addr;
        int key_idx;
        int set_tx;
        const u8 *seq;
        size_t seq_len;
        const u8 *key;
        size_t key_len;
    } wapi_key_param;
    wapi_key_param.addr = addr;
    wapi_key_param.key_idx = key_idx;
    wapi_key_param.set_tx = set_tx;
    wapi_key_param.seq = seq;
    wapi_key_param.seq_len = seq_len;
    wapi_key_param.key = key;
    wapi_key_param.key_len = key_len;

    return wpa_drv_driver_cmd((struct wpa_supplicant*)_wpa_s,
                "set-wapi-key",
                (char*)&wapi_key_param,
                sizeof(wapi_key_param));
}

static int wapi_msg_send_helper(void *priv,
            const u8 *msg_in,
            int msg_in_len, u8 *msg_out,
            int *msg_out_len)
{
    struct {
        const u8 *msg_in;
        int msg_in_len;
        u8 *msg_out;
        int *msg_out_len;
    } wapi_msg_send_param;

    wapi_msg_send_param.msg_in = msg_in;
    wapi_msg_send_param.msg_in_len = msg_in_len;
    wapi_msg_send_param.msg_out = msg_out;
    wapi_msg_send_param.msg_out_len = msg_out_len;
    return wpa_drv_driver_cmd((struct wpa_supplicant*)priv,
                "wapi-msg-send",
                (char*)&wapi_msg_send_param,
                sizeof(wapi_msg_send_param));
}
/* End: wrapper definition for wapi interface*/

static void wpa_supplicant_rx_wai(void *ctx, const u8 *src_addr, const u8 *buf, size_t len)
{
    if (libwapi_handle == NULL) {
        wpa_printf(MSG_ERROR, "Do NOT support WAPI");
        return;
    }

    wapi_set_rx_wai(buf, len);
}

int wapi_init_l2(struct wpa_supplicant *wpa_s)
{
    struct wapi_context *wapi = wpa_s->wapi;

    if (!wapi) {
        wpa_printf(MSG_ERROR, "[WAPI] wapi context is NULL");
        return -1;
    }
    wapi->l2_wai = l2_packet_init(wpa_s->ifname,
                    wpa_drv_get_mac_addr(wpa_s),
                    ETH_P_WAI,
                    wpa_supplicant_rx_wai, wpa_s, 0);

    if (wapi->l2_wai &&
        l2_packet_get_own_addr(wapi->l2_wai, wpa_s->own_addr)) {
        wpa_printf(MSG_ERROR, "[WAPI] Failed to get own L2 address for WAPI");
        return -1;
    }
    return 0;
}

int wapi_init(struct wpa_supplicant *wpa_s)
{
    libwapi_handle = dlopen(LIB_FULL_NAME, RTLD_NOW);
    if (libwapi_handle == NULL) {
        wpa_printf(MSG_ERROR, "[WAPI] %s dlopen failed in %s: %s",
                __FUNCTION__, LIB_FULL_NAME, dlerror());
        return -1;
    }

    // reset errors
    dlerror();

    init_wapi_api();

    wpa_s->wapi = os_zalloc(sizeof(struct wapi_context));
    struct wapi_cb_ctx *ctx;
    ctx = os_zalloc(sizeof(*ctx));
    if (ctx == NULL) {
        wpa_printf(MSG_ERROR, "[WAPI] Failed to allocate WAPI context.");
        return -1;
    }

    /*
	* prepare the resource of wpa_supplicant
	*/
    ctx->ctx = wpa_s;
    ctx->mtu_len = 1500;
    ctx->msg_send = wapi_msg_send_helper;
    ctx->wpa_msg = wpa_msg;
    ctx->get_state = wapi_get_state_helper;
    /*
	* ctx->deauthenticate = wpa_supplicant_deauthenticate;
	*/
    ctx->deauthenticate = wapi_deauthenticate_helper;
    ctx->ether_send = wapi_tx_wai;
    ctx->set_key = wapi_set_key_helper;
    ctx->set_state = wapi_set_state_helper;
    ctx->cancel_timer = eloop_cancel_timeout;
    ctx->set_timer = eloop_register_timeout;
    /*
	* allocate cb resource to wapi module
	*/
#ifdef CONFIG_MEDIATEK_REDUCE_LOG
    wpa_printf(MSG_MSGDUMP, "[WAPI] wapi_init");
#else
    wpa_printf(MSG_ERROR, "[WAPI] wapi_init");
#endif

    if (wapi_lib_init(ctx) < 0) {
        os_free(ctx);
        wpa_printf(MSG_ERROR, "[WAPI] Failed to initialize WAPI library");
        return -1;
    }
    return wapi_init_l2(wpa_s);
}

int wapi_deinit(struct wpa_supplicant *wpa_s)
{
    if (libwapi_handle == NULL) {
        wpa_printf(MSG_ERROR, "Do NOT support WAPI");
        return -1;
    }
    /*free cb resource to wapi module*/
    wpa_printf(MSG_DEBUG, "[WAPI] wapi_deinit");

    if (wapi_lib_exit() < 0) {
        wpa_printf(MSG_ERROR, "[WAPI] Failed to initialize WAPI library");
        return -1;
    }

    dlclose(libwapi_handle);
    libwapi_handle = NULL;
    return 0;
}

int wapi_set_suites(struct wpa_supplicant *wpa_s, struct wpa_ssid *ssid,
                  struct wpa_bss *bss, u8 *wpa_ie, size_t *wpa_ie_len)
{
    if (libwapi_handle == NULL) {
        wpa_printf(MSG_ERROR, "Do NOT support WAPI");
        return -1;
    }

    CNTAP_PARA  wapi_param;
    int wapi_set_param_result = 0;

    /* WAPI IE id is equal with AC_ACCESS_DELAY */
    if (!bss || !wpa_bss_get_ie(bss, WLAN_EID_WAPI)) {
        wpa_printf(MSG_ERROR, "[WAPI] No WAPI IE found, bss is NULL %d", !!bss);
        return -1;
    }
    /* No need to check cipher suite due to only SMS4 is available in WAPI Spec */
    if (!(ssid->key_mgmt & (WPA_KEY_MGMT_WAPI_CERT | WPA_KEY_MGMT_WAPI_PSK))) {
        wpa_printf(MSG_ERROR, "[WAPI] param error, key_mgmt 0x%x", ssid->key_mgmt);
        return -1;
    }
    wpa_s->wpa_proto = ssid->proto;
    wpa_s->key_mgmt = ssid->key_mgmt;
    /* Always use SMS4 as pairwise and group cipher suite */
    wpa_s->pairwise_cipher = WPA_CIPHER_SMS4;
    wpa_s->group_cipher = WPA_CIPHER_SMS4;
    os_memset(&wapi_param, 0, sizeof(wapi_param));
    if (wpa_s->key_mgmt == WPA_KEY_MGMT_WAPI_PSK) {
        /* it's not PMK actually but string */
        wapi_param.authType = AUTH_TYPE_WAPI_PSK;

        if (!ssid->passphrase) {
            wpa_printf(MSG_ERROR, "[WAPI] ssid->passphrase is NULL");
            return -1;
        }
        /* Currently, if passphrase is enclosed with "", we consider it is a hex string */
        if (ssid->passphrase[0] == '\"') {
            wapi_param.kt = KEY_TYPE_HEX;
            wapi_param.kl = strlen((char*)ssid->passphrase) - 2;
            memcpy(wapi_param.kv, &ssid->passphrase[1], wapi_param.kl);
            wpa_printf(MSG_INFO, "[WAPI] Using Key mgmt WAPI Hex PSK");
        } else {
            wapi_param.kt = KEY_TYPE_ASCII;
            wapi_param.kl = strlen((char*)ssid->passphrase);
            memcpy(wapi_param.kv, ssid->passphrase, wapi_param.kl);
            wpa_printf(MSG_INFO, "[WAPI] Using Key mgmt WAPI Ascii PSK");
        }
        wapi_param.kv[wapi_param.kl] = '\0';
    } else if (wpa_s->key_mgmt == WPA_KEY_MGMT_WAPI_CERT) {
        if (!ssid->wapi_cert_alias || !os_strcmp((char *)ssid->wapi_cert_alias, "NULL"))
            wapi_param.cert_list = NULL;
        else
            wapi_param.cert_list = (char *)ssid->wapi_cert_alias;
        wapi_param.authType = AUTH_TYPE_WAPI_CERT;
        wpa_printf(MSG_INFO, "[WAPI] Using Key mgmt WAPI Cert");
    }

    wapi_set_param_result = wapi_set_user(&wapi_param);
    if (wapi_set_param_result < 0) {
        wpa_printf(MSG_WARNING, "[WAPI] wapi_set_user result %d", wapi_set_param_result);
        return -1;
    }

    /*
	* fill wapi ie for associate request
	*/
    *wpa_ie_len = wapi_get_assoc_ie(wpa_ie);

    if (*wpa_ie_len <= 0) {
        wpa_printf(MSG_DEBUG, "[WAPI] Failed to generate WAPI IE.");
        return -1;
    }

    return 0;
}

void wapi_event_assoc(struct wpa_supplicant *wpa_s)
{
    if (libwapi_handle == NULL) {
        wpa_printf(MSG_ERROR, "Do NOT support WAPI");
        return;
    }

    static int loop = 0;
    MAC_ADDRESS bssid_s;
    MAC_ADDRESS own_s;

    /*
	* To avoiding state LOOP case,
	* otherwise HAL will disable interface
	*/

    /*
	* ALPS00127420
	* Can not auto-reconnect WAPI-PSK AP after
	* power off--> power on WAPI AP
	*/
    if ((wpa_s->wpa_state == WPA_4WAY_HANDSHAKE ||
        wpa_s->wpa_state == WPA_GROUP_HANDSHAKE) &&
        (loop >= 1 && loop <= 20)) {
        /*dont set state*/
        wpa_printf(MSG_INFO, "[WAPI] %s: [Loop = %d] "
            "dont set_state", __FUNCTION__, loop);
        loop++;
    } else {
        wpa_printf(MSG_INFO, "[WAPI] %s: [Loop = %d] "
            "set_state", __FUNCTION__, loop);
        loop=1;
        wpa_supplicant_set_state(wpa_s, WPA_ASSOCIATED);
    }

    /*
	* stop WPA and other time out use WAPI time only
	*/
    wpa_supplicant_cancel_auth_timeout(wpa_s);
    wpa_supplicant_cancel_scan(wpa_s);

    wpa_printf(MSG_DEBUG, "[WAPI] Associated, Bssid:"MACSTR" Own:"MACSTR,
        MAC2STR(wpa_s->bssid), MAC2STR(wpa_s->own_addr));

    if (is_zero_ether_addr(wpa_s->bssid)){
        wpa_printf(MSG_DEBUG, "[WAPI] Zero bssid, Not to set msg to WAPI SM\n");
        /*
		* Have been disassociated with the WAPI AP
		*/
        return;
    }

    memcpy(bssid_s.v, wpa_s->bssid, sizeof(bssid_s.v));
    memcpy(own_s.v, wpa_s->own_addr, sizeof(own_s.v));

    wapi_set_msg(CONN_ASSOC, &bssid_s, &own_s, wpa_s->wapi->bss_wapi_ie,
        wpa_s->wapi->bss_wapi_ie_len);
}

void wapi_event_disassoc(struct wpa_supplicant *wpa_s, const u8 *bssid)
{
    if (libwapi_handle == NULL) {
        wpa_printf(MSG_ERROR, "Do NOT support WAPI");
        return;
    }

    MAC_ADDRESS bssid_s;
    MAC_ADDRESS own_s;

    if (!bssid) {
        wpa_printf(MSG_DEBUG, "[WAPI] wapi_event_disassoc, bssid is NULL!");
        return;
    }

    memcpy(bssid_s.v, bssid, sizeof(bssid_s.v));
    memcpy(own_s.v, wpa_s->own_addr, sizeof(own_s.v));

    wapi_set_msg(CONN_DISASSOC, &bssid_s, &own_s, wpa_s->wapi->bss_wapi_ie , wpa_s->wapi->bss_wapi_ie_len);
}

void wapi_handle_cert_list_changed(struct wpa_supplicant *wpa_s)
{
    if (libwapi_handle == NULL) {
        wpa_printf(MSG_ERROR, "Do NOT support WAPI");
        return;
    }

    CNTAP_PARA  wapi_param;
    os_memset(&wapi_param, 0, sizeof(wapi_param));
    wapi_param.reinit_cert_list = 1;
    wapi_param.cert_list = wpa_s->conf->wapi_cert_alias_list;
    wapi_param.authType = AUTH_TYPE_WAPI_CERT;
    wapi_set_user(&wapi_param);
}

void init_wapi_api() {
    if (libwapi_handle == NULL) {
        wpa_printf(MSG_ERROR, "Do NOT support WAPI");
        return;
    }

    wpa_printf(MSG_DEBUG, "[WAPI] start init APIs");
    wapi_set_rx_wai = dlsym(libwapi_handle, "wapi_set_rx_wai");
    wapi_lib_init = dlsym(libwapi_handle, "wapi_lib_init");
    wapi_lib_exit = dlsym(libwapi_handle, "wapi_lib_exit");
    wapi_set_user = dlsym(libwapi_handle, "wapi_set_user");
    wapi_get_assoc_ie = dlsym(libwapi_handle, "wapi_get_assoc_ie");
    wapi_set_msg = dlsym(libwapi_handle, "wapi_set_msg");
    wpa_printf(MSG_DEBUG, "[WAPI] Dlopen wapi APIs success");
}
