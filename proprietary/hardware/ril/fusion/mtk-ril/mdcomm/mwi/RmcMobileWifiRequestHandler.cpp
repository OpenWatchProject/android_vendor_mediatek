/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

// MTK fusion include
#include "RfxVoidData.h"
#include "RfxStringData.h"
#include "RfxStringsData.h"
#include "RfxIntsData.h"

// MWI Local include
#include "RmcMobileWifiRequestHandler.h"
#include "RmcMobileWifiInterface.h"

#include <sys/socket.h>
#include <errno.h>

#define RFX_LOG_TAG "RmcMwi"

 // register handler to channel
RFX_IMPLEMENT_HANDLER_CLASS(RmcMobileWifiRequestHandler, RIL_CMD_PROXY_1);

// register request to RfxData
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringsData, RfxVoidData, RFX_MSG_REQUEST_SET_WIFI_ENABLED);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringsData, RfxVoidData, RFX_MSG_REQUEST_SET_WIFI_ASSOCIATED);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringsData, RfxVoidData, RFX_MSG_REQUEST_SET_WFC_CONFIG);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringsData, RfxVoidData, RFX_MSG_REQUEST_SET_WIFI_SIGNAL_LEVEL);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringsData, RfxVoidData, RFX_MSG_REQUEST_SET_GEO_LOCATION);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringsData, RfxVoidData, RFX_MSG_REQUEST_SET_WIFI_IP_ADDRESS);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringsData, RfxVoidData, RFX_MSG_REQUEST_SET_EMERGENCY_ADDRESS_ID);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringsData, RfxVoidData, RFX_MSG_REQUEST_SET_NATT_KEEP_ALIVE_STATUS);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_SET_WIFI_PING_RESULT);
///M: Notify ePDG screen state
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_NOTIFY_EPDG_SCREEN_STATE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxIntsData, RFX_MSG_REQUEST_QUERY_SSAC_STATUS);


static const int requests[] = {
    RFX_MSG_REQUEST_SET_WIFI_ENABLED,
    RFX_MSG_REQUEST_SET_WIFI_ASSOCIATED,
    RFX_MSG_REQUEST_SET_WFC_CONFIG,
    RFX_MSG_REQUEST_SET_WIFI_SIGNAL_LEVEL,
    RFX_MSG_REQUEST_SET_GEO_LOCATION,
    RFX_MSG_REQUEST_SET_WIFI_IP_ADDRESS,
    RFX_MSG_REQUEST_SET_EMERGENCY_ADDRESS_ID,
    RFX_MSG_REQUEST_SET_NATT_KEEP_ALIVE_STATUS,
    RFX_MSG_REQUEST_SET_WIFI_PING_RESULT,
    ///M: Notify ePDG screen state
    RFX_MSG_REQUEST_NOTIFY_EPDG_SCREEN_STATE,
    RFX_MSG_REQUEST_QUERY_SSAC_STATUS,
};

typedef enum{
    WFC_SETTING_WIFI_UEMAC = 0,
    WFC_SETTING_LOCATION_SETTING = 1,
}WfcConfigType;

RmcMobileWifiRequestHandler::RmcMobileWifiRequestHandler(
    int slot_id, int channel_id) : RfxBaseHandler(slot_id, channel_id) {
    // register to handle request
    registerToHandleRequest(requests, sizeof(requests) / sizeof(int));
}

RmcMobileWifiRequestHandler::~RmcMobileWifiRequestHandler() {
}

void RmcMobileWifiRequestHandler::onHandleTimer() {
}

void RmcMobileWifiRequestHandler::onHandleRequest(const sp<RfxMclMessage>& msg) {
    int requestId = msg->getId();

    switch (requestId) {
        case RFX_MSG_REQUEST_SET_WIFI_ENABLED:
            setWifiEnabled(msg);
            break;
        case RFX_MSG_REQUEST_SET_WIFI_ASSOCIATED:
            setWifiAssociated(msg);
            break;
        case RFX_MSG_REQUEST_SET_WFC_CONFIG:
            setWfcConfig(msg);
            break;
        case RFX_MSG_REQUEST_SET_WIFI_SIGNAL_LEVEL:
            setWifiSignal(msg);
            break;
        case RFX_MSG_REQUEST_SET_GEO_LOCATION:
            setGeoLocation(msg);
            break;
        case RFX_MSG_REQUEST_SET_WIFI_IP_ADDRESS:
            setWifiIpAddress(msg);
            break;
        case RFX_MSG_REQUEST_SET_EMERGENCY_ADDRESS_ID:
            setEmergencyAddressId(msg);
            break;
        case RFX_MSG_REQUEST_SET_NATT_KEEP_ALIVE_STATUS:
            setNattKeepAliveStatus(msg);
            break;
        case RFX_MSG_REQUEST_SET_WIFI_PING_RESULT:
            setWifiPingResult(msg);
            break;
        ///M: Notify ePDG screen state
        case RFX_MSG_REQUEST_NOTIFY_EPDG_SCREEN_STATE:
            notifyEPDGScreenState(msg);
            break;
        case RFX_MSG_REQUEST_QUERY_SSAC_STATUS:
            querySsacStatus(msg);
            break;
        default:
            break;
    }
}

void RmcMobileWifiRequestHandler::setWifiEnabled(const sp<RfxMclMessage>& msg) {
    /* AT+EWIFIEN=<ifname>,<enabled>
     * <ifname>: interface name, such as wlan0
     * <enabled>: 0 = disable; 1 = enable
     * <flightModeOn>: 0 = disable; 1 = enable
     */
    char** params = (char**)msg->getData()->getData();
    int dataLen =  msg->getData()->getDataLength() / sizeof(char*);
    logD(RFX_LOG_TAG, "setWifiEnabled dataLen: %d", dataLen);

    char* atWifiEnCmd = AT_SET_WIFI_ENABLE;
    char* atAPMCmd = AT_SET_AIRPLANE_MODE;
    char* ifname = params[0];
    int enabled = atoi(params[1]);

    sp<RfxAtResponse> p_response;

    if (dataLen == 2) {
        String8 cmd = String8::format("%s=\"%s\",%d", atWifiEnCmd, ifname, enabled);
        handleCmdWithVoidResponse(msg, cmd);
    } else if (dataLen == 3) {

        int allCause = atoi(params[2]);

        if ((allCause & EWIFIEN_NEED_SEND_WIFI_ENABLED) == EWIFIEN_NEED_SEND_WIFI_ENABLED) {
            int cause = (allCause & (EWIFIEN_CAUSE)) == EWIFIEN_CAUSE ? 1 : 0;
            int wifiEnabled = (allCause & (EWIFIEN_WIFI_ENABLED_STATE)) == EWIFIEN_WIFI_ENABLED_STATE ? 1 : 0;
            String8 cmd = String8::format("%s=\"%s\",%d,%d",
                    atWifiEnCmd, ifname, wifiEnabled, cause);
            handleCmdWithVoidResponse(msg, cmd);
        }

        if ((allCause & EWIFIEN_NEED_SEND_AP_MODE) == EWIFIEN_NEED_SEND_AP_MODE) {
            int flightModeOn = (allCause & (EWIFIEN_AP_MODE_STATE)) == EWIFIEN_AP_MODE_STATE ? 1 : 0;
            String8 apCmd = String8::format("%s=%d", atAPMCmd, flightModeOn);
            p_response = atSendCommand(apCmd);
            if (p_response->getError() != 0 || p_response->getSuccess() != 1) {
                logD(RFX_LOG_TAG, "Not support EAPMODE command.");
            }
        }
    }
}

void RmcMobileWifiRequestHandler::setWifiAssociated(const sp<RfxMclMessage>& msg) {
    /* AT+EWIFIASC=<ifname>,<assoc>,<ssid>,<ap_mac>,<wifi_type>,<mtu>
     * <ifname>: interface name, such as wlan0
     * <assoc>: 0 = not associated; 1 = associated
     * <ssid>: wifi ap ssid when associated, 0 if assoc = 0
     * <ap_mac>: wifi ap mac addr, 0 if assoc = 0
     * <wifi_type>: wifi type: such as 802.11b, 802.11n...
     * <mtu>: mtu size
     */
    char** params = (char**)msg->getData()->getData();

    char* atCmd = AT_SET_WIFI_ASSOCIATED;
    char* ifname = params[0];
    const char* assoc = params[1];
    char* ssid = (atoi(assoc) == 0)? (char*)"0": params[2];
    char* ap_mac = (atoi(assoc) == 0)? (char*)"0": params[3];
    char* mtu = (atoi(assoc) == 0)? (char*)"0": params[4];
    char* ue_mac = (params[5])? params[5]: (char*)"0";
    char defaultWifiType[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get("vendor.test.wifi.type", defaultWifiType, "802.11n");
    char* type = defaultWifiType;

    // TODO: Need to check modem generation before sync to trunk
    if (atoi(assoc) == 0) {
        type = (char*) "";
    } else {
        int ioctl_sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (ioctl_sock >= 0) {
            struct iwreq wrq;
            struct priv_driver_cmd_t priv_cmd;
            int ret;

            memset(&priv_cmd,0x00,sizeof(priv_cmd));
            priv_cmd.used_len = strlen(priv_cmd.buf);
            priv_cmd.total_len = PRIV_CMD_SIZE;

            strncpy(wrq.ifr_name, "wlan0", (IFNAMSIZ - 1));
            wrq.u.data.pointer = &priv_cmd;
            wrq.u.data.length = strlen(priv_cmd.buf);
            wrq.u.data.flags = PRIV_CMD_GET_WIFI_TYPE;

            ret = ioctl(ioctl_sock, IOCTL_SET_STRUCT_FOR_EM, &wrq);
            if ((ret == 0) && (wrq.u.data.length > 1)) {
                //ALOGI("get wifi type ok: [%s]", wrq.u.data.pointer);
                logD(RFX_LOG_TAG, "get wifi type ok: [%s]", (char*)wrq.u.data.pointer);
                type = (char*) wrq.u.data.pointer;
            } else {
                //ALOGI("get wifi type fail: %d\n", ret);
                logD(RFX_LOG_TAG, "get wifi type fail: %d, len = %d", ret, wrq.u.data.length);
            }
            close(ioctl_sock);
        } else {
            logE(RFX_LOG_TAG, "setWifiAssociatedWithMtu() ioctl_sock = %d %s",
                    ioctl_sock, strerror(errno));
        }
    }

    String8 cmd = String8::format("%s=\"%s\",%s,\"%s\",\"%s\",\"%s\",%s,\"%s\"",
                                   atCmd, ifname, assoc, ssid, ap_mac, type, mtu, ue_mac);
    handleCmdWithVoidResponse(msg, cmd);
}

void RmcMobileWifiRequestHandler::setWfcConfig(const sp<RfxMclMessage>& msg) {
    char** params = (char**)msg->getData()->getData();
    char* setting = params[0];
    char* param1 = params[1]? params[1]: (char*)"" ;
    char* param2 = params[2]? params[2]: (char*)"" ;
    logD(RFX_LOG_TAG, "setWfcConfig: %s, %s, %s", setting, param1, param2);
    switch (atoi(setting)) {
        case WFC_SETTING_WIFI_UEMAC:
            setWfcConfig_WifiUeMac(msg);
            break;
        case WFC_SETTING_LOCATION_SETTING:
            setWfcConfig_LocationSetting(msg);
            break;
        default:
            break;
    }
}

void RmcMobileWifiRequestHandler::setWfcConfig_WifiUeMac(const sp<RfxMclMessage>& msg) {
    RIL_Errno result = RIL_E_SUCCESS;
    // report to tcl
    sp<RfxMclMessage> mclResponse =
            RfxMclMessage::obtainResponse(msg->getId(), result, RfxVoidData(), msg);
    responseToTelCore(mclResponse);
}

void RmcMobileWifiRequestHandler::setWfcConfig_LocationSetting(const sp<RfxMclMessage>& msg) {
    char* atCmd = AT_SET_LOCATION_ENABLE;
    char** params = (char**)msg->getData()->getData();
    char* configType = params[0]; //WFC Config Type
    char* iFname = params[1]? params[1]: (char*)"" ; //ifName. e.g. ""locenable""
    char* setting = params[2]? params[2]: (char*)"" ; //setting value
    int enabled = atoi(setting);

    logD(RFX_LOG_TAG, "setWfcConfig_LocationSetting: %s, %s, %s", configType, iFname, setting);
    String8 cmd = String8::format("%s=%d", atCmd, enabled);
    handleCmdWithVoidResponse(msg, cmd);
}

void RmcMobileWifiRequestHandler::setWifiSignal(const sp<RfxMclMessage>& msg) {
    /* AT+EWIFISIGLVL=<ifname>,<rssi>,<snr>
     * <ifname>: interface name, such as wlan0
     * <rssi>: rssi value
     * <snr>: string value
     */
    char** params = (char**)msg->getData()->getData();

    char* atCmd = AT_SET_WIFI_SIGNAL_LEVEL;
    char* ifname = params[0];
    int rssi = atoi(params[1]);
    char* snr = params[2];

    String8 cmd = String8::format("%s=\"%s\",%d,\"%s\"", atCmd, ifname, rssi, snr);
    handleCmdWithVoidResponse(msg, cmd);
}

void RmcMobileWifiRequestHandler::setWifiIpAddress(const sp<RfxMclMessage>& msg) {
    /* AT+EWIFIADDR=<ifname>,<ipv4>,<ipv6>,<ipv4PrefixLen>,<ipv6PrefixLen>,
     *         <ipv4gateway>,<ipv6gateway>,<dns_count>, <dns_1>, <dns_2>,..., <dns_n>
     * <ifname>: interface name, such as wlan0
     * <ipv4>: IPV4 address
     * <ipv6>: IPV6 address
     * <ipv4PrefixLen>: IPV4 address prefix length
     * <ipv6PrefixLen>: IPV6 address prefix length
     * <ipv4gateway>: IPV4 gateway address
     * <ipv6gateway>: IPV6 gateway address
     * <dns_count>: dns server count
     * <dns_1>, <dns_2>,... <dns_n>: dns server addresses
     */
    char** params = (char**)msg->getData()->getData();

    char* atCmd = AT_SET_WIFI_IP_ADDRESS;
    char* ifname = params[0];

    // Google HIDL service changes "" in java as null in cpp
    char* ipv4 = (params[1] == NULL) ? (char*)"" : params[1];
    char* ipv6 = (params[2] == NULL) ? (char*)"" : params[2];
    char* ipv4PrefixLen = (params[3] == NULL) ? (char*)"-1" : params[3];
    if (strncmp(ipv4PrefixLen,"-1", 2) == 0) ipv4PrefixLen = (char*)"";
    char* ipv6PrefixLen = (params[4] == NULL) ? (char*)"-1" : params[4];
    if (strncmp(ipv6PrefixLen,"-1", 2) == 0) ipv6PrefixLen = (char*)"";
    char* ipv4gateway = (params[5] == NULL) ? (char*)"" : params[5];
    char* ipv6gateway = (params[6] == NULL) ? (char*)"" : params[6];
    char* dnsCount = (params[7] == NULL) ? (char*)"0" : params[7];
    char* dnsAddresses = (params[8] == NULL) ? (char*)"" : params[8];

    String8 cmd = String8::format("%s=\"%s\",\"%s\",\"%s\",%s,%s,\"%s\",\"%s\",%s,%s",
            atCmd, ifname, ipv4, ipv6,
            ipv4PrefixLen, ipv6PrefixLen,
            ipv4gateway, ipv6gateway,
            dnsCount, dnsAddresses);
    handleCmdWithVoidResponse(msg, cmd);
}

void RmcMobileWifiRequestHandler::setGeoLocation(const sp<RfxMclMessage>& msg) {
    /* AT+EIMSGEO=<account_id>,<broadcast_flag>,<latitude>,<longitude>,<accurate>,<method>,<city>,<state>,<zip>,<country>
     * <account_id>: request id, 0~7
     * <broadcast_flag>: 0, 1
     * <latitude>: latitude from GPS, 0 as failed
     * <longitude>: longitude from GPS, 0 as failed
     * <accurate>: accurate from GPS, 0 as failed
     * <method>: Location information from Fwk type, Network or GPS
     * <city>: City
     * <state>: State
     * <zip>: Zip
     * <country>: country
     * <ueWlanMac>: UE Wi-Fi interface mac address
     */
    char** params = (char**)msg->getData()->getData();
    char* atCmd = AT_SET_GEO_LOCATION;
    int dataLen =  msg->getData()->getDataLength() / sizeof(char*);

    logD(RFX_LOG_TAG, "setGeoLocation dataLen: %d", dataLen);

    // Google HIDL service changes "" in java as null in cpp
    char* method = (params[5] == NULL) ? (char*)"" : params[5];
    char* city = (params[6] == NULL) ? (char*)"" : params[6];
    char* state = (params[7] == NULL) ? (char*)"" : params[7];
    char* zip = (params[8] == NULL) ? (char*)"" : params[8];
    char* country = (params[9] == NULL) ? (char*)"" : params[9];

    if (dataLen == 10) {
        String8 cmd = String8::format("%s=%s,%s,%s,%s,%s,\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"", atCmd,
            params[0], params[1], params[2], params[3], params[4],
            method, city, state, zip, country);
        handleCmdWithVoidResponse(msg, cmd);
    } else if (dataLen == 11) {
        char* ueWlanMac = (params[10] == NULL) ? (char*)"" : params[10];
        String8 cmd = String8::format("%s=%s,%s,%s,%s,%s,\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"", atCmd,
            params[0], params[1], params[2], params[3], params[4],
            method, city, state, zip, country, ueWlanMac);
        handleCmdWithVoidResponse(msg, cmd);
    }
}

void RmcMobileWifiRequestHandler::setEmergencyAddressId(const sp<RfxMclMessage>& msg) {
    /* AT+EIMSAID = <aid>
     * <aid>: Access Id from Settings UI
     */
    char* atCmd = AT_SET_ECC_AID;
    char** params = (char**)msg->getData()->getData();

    logD(RFX_LOG_TAG, "setEmergencyAddressId aid: %s", params[0]);
    String8 cmd = String8::format("%s=\"%s\"", atCmd, params[0]);
    handleCmdWithVoidResponse(msg, cmd);
}

void RmcMobileWifiRequestHandler::setNattKeepAliveStatus(const sp<RfxMclMessage>& msg) {
    /* AT+EWIFINATT= <ifname>,<enable>,<src_ip>,<src_port>,<dst_ip>,<dst_port>
     * <ifname>: interface name, such as wlan0
     * <enable>: enabled, 0 = disable; 1 = enabled
     * <src_ip>: source IP
     * <src_port>: source port
     * <dst_ip>: destination IP
     * <dst_port>: destination port
     */
    char** params = (char**)msg->getData()->getData();
    char* atCmd = AT_SET_NATT_KEEP_ALIVE_STATUS;

    String8 cmd = String8::format("%s=\"%s\",%s,\"%s\",%s,\"%s\",%s", atCmd,
        params[0], params[1], params[2], params[3], params[4], params[5]);

    handleCmdWithVoidResponse(msg, cmd);
}


void RmcMobileWifiRequestHandler::setWifiPingResult(const sp<RfxMclMessage>& msg) {
    /* AT+EIWLPING= <rat_type>,<ave_latency>,<loss_rate>
     * <rat_type>: rat type for which ping result has been requested ie: RAT_WIFI
     * <ave_latency>: Average latency in MS of ping.
     * <loss_rate>: percentage packet loss rate of ping
     */
    char* atCmd = AT_SET_WIFI_PING_RESULT;
    int *params = (int *)msg->getData()->getData();

    logD(RFX_LOG_TAG, "setWifiPingResult: rat:%d, latency:%d, packetloss:%d",
            params[0], params[1], params[2]);
    String8 cmd = String8::format("%s = %d, %d, %d", atCmd, params[0], params[1], params[2]);
    handleCmdWithVoidResponse(msg, cmd);
}

///M: Notify ePDG screen state
void RmcMobileWifiRequestHandler::notifyEPDGScreenState(const sp<RfxMclMessage>& msg) {
    /* AT+ESCREENSTATE= <state>
     * <state>: Screen state
     */
    char* atCmd = AT_SET_EPDG_SCREEN_STATE;
    int *params = (int *)msg->getData()->getData();

    logD(RFX_LOG_TAG, "notifyEPDGScreenState: state:%d", params[0]);
    String8 cmd = String8::format("%s = %d", atCmd, params[0]);
    handleCmdWithVoidResponse(msg, cmd);
}

void RmcMobileWifiRequestHandler::querySsacStatus(const sp<RfxMclMessage>& msg) {
    /* AT+CSSAC=?
     * return: <BFVoice>,<BFVideo>,<BTVoice>,<BTVoice>
     */

    char* atCmd = AT_QUERY_SSAC;
    sp<RfxAtResponse> p_response;
    int err;
    RfxAtLine *line;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    int responses[4] = {0};

    String8 cmd = String8::format("%s=?", atCmd);
    p_response = atSendCommandSingleline(cmd, "+CSSAC:");

    err = p_response->getError();
    if (err < 0 || p_response == NULL) {
        logE(RFX_LOG_TAG, "querySsacStatus Fail");
        goto error;
    }

    if (p_response->getIntermediates() != NULL) {
        line = p_response->getIntermediates();
        line->atTokStart(&err);
        if (err < 0) {
            goto error;
        }

        // BFVoice
        responses[0] = line->atTokNextint(&err);
        if (err < 0) {
            goto error;
        }

        // BFVideo
        responses[1] = line->atTokNextint(&err);
        if (err < 0) {
            goto error;
        }

        // BTVoice
        responses[2] = line->atTokNextint(&err);
        if (err < 0) {
            goto error;
        }

        // BTVoice
        responses[3] = line->atTokNextint(&err);
        if (err < 0) {
            goto error;
        }
    }

    ret = RIL_E_SUCCESS;

error:
    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), ret,
            RfxIntsData(responses, sizeof(responses) / sizeof(int)), msg, false);

    // response to Telcore
    responseToTelCore(response);
}