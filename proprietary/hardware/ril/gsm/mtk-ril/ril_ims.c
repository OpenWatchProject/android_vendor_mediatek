/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

#include <telephony/mtk_ril.h>
#include <libmtkrilutils.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <alloca.h>
#include "atchannels.h"
#include "at_tok.h"
#include "misc.h"
#include <getopt.h>
#include <sys/socket.h>
#include <cutils/sockets.h>
#include <cutils/properties.h>
#include <termios.h>
#include <signal.h>
#include <ctype.h>

#include <ril_callbacks.h>

#include "ril_ims.h"

#ifdef LOG_TAG
    #undef LOG_TAG
#endif
#define LOG_TAG "RIL-IMS"

#include <log/log.h>

#define OEM_CHANNEL_CTX getRILChannelCtxFromToken(t)

/**
 * ims initilization.
 * @param rid ril id.
 *
 */
void imsInit(RILSubSystemId subsystem, RIL_SOCKET_ID rid) {
    UNUSED(subsystem);
    /* Set Ims capability */

    RLOGD("imsInit rid =%d ",rid);
    int checkingBit = 0;

    if (getMultiImsSupportValue() == 1) {
        rid = getMainProtocolRid();
        RLOGD("imsInit - multi ims not support, rid change to %d ",rid);
        checkingBit = 0;
    } else {
        checkingBit = rid;
    }

    RILChannelCtx *p_channel = getChannelCtxbyProxy();
    char ctVolteSupport[PROPERTY_VALUE_MAX] = { 0 };
    property_get("persist.vendor.mtk_ct_volte_support", ctVolteSupport, "0");
    if (atoi(ctVolteSupport) == 1) {
        at_send_command("AT+ECFGSET=\"mtk_ct_volte_support\",\"1\"", NULL, p_channel);
        RLOGD("VoLTE over IMS is support and AT+ECFGSET=...,1 is sent");
    } else {
        at_send_command("AT+ECFGSET=\"mtk_ct_volte_support\",\"0\"", NULL, p_channel);
        RLOGD("VoLTE over IMS is supported and AT+ECFGSET=...,0 is sent");
    }

    if (isVilteSupport()) {
        char videoEnable[PROPERTY_VALUE_MAX] = { 0 };
        property_get("persist.vendor.mtk.vilte.enable", videoEnable, "0");

        if ((atoi(videoEnable) & (1 << checkingBit)) == 0) {
            at_send_command("AT+EIMSCCP=0", NULL, p_channel);
            RLOGD("Ims video setting is disable and AT+EIMSCCP=0 is sent");
        } else {
            at_send_command("AT+EIMSCCP=1", NULL, p_channel);
            RLOGD("Ims video setting is enable and AT+EIMSCCP=1 is sent");
        }
    } else {
        at_send_command("AT+EIMSCCP=0", NULL, p_channel);
        RLOGD("Ims video isn't supported and AT+EIMSCCP=0 is sent");
    }

    if (isImsSupport()) {
        at_send_command("AT+EIMSSMS=1", NULL, p_channel);
        RLOGD("SMS over IMS is support and AT+EIMSSMS=1 is sent");
        at_send_command("AT+EIMSESS=1", NULL, p_channel);
        RLOGD("Enable IMS ECC support capability report");
    } else {
        at_send_command("AT+EIMSSMS=0", NULL, p_channel);
        RLOGD("SMS over IMS isn't supported and AT+EIMSSMS=0 is sent");
    }

    if (isImsAndVolteSupport() || isWfcSupport()) {
        char volteEnable[PROPERTY_VALUE_MAX] = { 0 };
        char wfcEnable[PROPERTY_VALUE_MAX] = { 0 };

        if (isImsAndVolteSupport()) {
            property_get("persist.vendor.mtk.volte.enable", volteEnable, "0");
            RLOGD("Imsinit: volteEnable = %d", atoi(volteEnable));
        }

        if (isWfcSupport()) {
            property_get("persist.vendor.mtk.wfc.enable", wfcEnable, "0");
            RLOGD("Imsinit: wfcEnable = %d", atoi(wfcEnable));
        }

        if (((atoi(volteEnable) & (1 << checkingBit)) == 0) &&
                ((atoi(wfcEnable) & (1 << checkingBit)) == 0)) {
            at_send_command("AT+EIMSVOLTE=0", NULL, p_channel);
            at_send_command("AT+EIMSWFC=0", NULL, p_channel);
            at_send_command("AT+EIMSVOICE=0", NULL, p_channel);
            at_send_command("AT+EIMS=0", NULL, p_channel);
            at_send_command("AT+EVADSREP=0", NULL, p_channel);
            RLOGD("volte setting and wfc setting are both disable");
        } else {
            if ((atoi(volteEnable) & (1 << checkingBit)) > 0) {
                at_send_command("AT+EIMSVOLTE=1", NULL, p_channel);
                RLOGD("volte setting is enable and AT+EIMSVOLTE=1 is sent");
            } else {
                at_send_command("AT+EIMSVOLTE=0", NULL, p_channel);
                RLOGD("volte setting is disable and AT+EIMSVOLTE=0 is sent");
            }

            if ((atoi(wfcEnable) & (1 << checkingBit)) > 0) {
                at_send_command("AT+EIMSWFC=1", NULL, p_channel);
                RLOGD("wfc setting is enable and AT+EIMSWFC=1 is sent");
            } else {
                at_send_command("AT+EIMSWFC=0", NULL, p_channel);
                RLOGD("wfc setting is disable and AT+EIMSWFC=0 is sent");
            }

            at_send_command("AT+EIMSVOICE=1", NULL, p_channel);
            at_send_command("AT+EIMS=1", NULL, p_channel);
            at_send_command("AT+EVADSREP=1", NULL, p_channel);
            at_send_command("AT+CIREP=1", NULL, p_channel);
            RLOGD("volte or wfc setting is enable and AT+EIMSVOICE=1 is sent");
        }
    } else {
        at_send_command("AT+EIMSVOLTE=0", NULL, p_channel);
        at_send_command("AT+EIMSWFC=0", NULL, p_channel);
        at_send_command("AT+EIMSVOICE=0", NULL, p_channel);
        at_send_command("AT+EIMS=0", NULL, p_channel);
        at_send_command("AT+EVADSREP=0", NULL, p_channel);
        RLOGD("volte and wfc are not supported");
    }

    if (isRcsUaProxySupport()) {
        char tempStr[32] = { 0 };
        if (isRcsServiceEnabled()) {
            sprintf(tempStr, "AT+EIMSRCS=1,%d", getRcsSrvTag());
        } else {
            sprintf(tempStr, "AT+EIMSRCS=0,%d", getRcsSrvTag());
        }
        at_send_command(tempStr, NULL, getChannelCtxbyProxy());
    } else {
        /* MD will save RCS state once enabled, so always reset here
           for runtime switch RCS UA support case.
           Response of EIMSRCS not the issue here, just ignore it */
        at_send_command("AT+EIMSRCS=0,0", NULL, getChannelCtxbyProxy());
    }
}

extern int rilImsMain(int request, void *data, size_t datalen, RIL_Token t)
{
    switch (request)
    {
        case RIL_REQUEST_SET_IMS_ENABLE:
            requestSetImsEnabled(data, datalen, t);
            break;
        case RIL_REQUEST_SET_VOLTE_ENABLE:
            requestSetVolteEnabled(data, datalen, t);
            break;
        case RIL_REQUEST_SET_WFC_ENABLE:
            requestSetWfcEnabled(data, datalen, t);
            break;
        case RIL_REQUEST_SET_IMS_VIDEO_ENABLE:
            requestSetImsVideoEnabled(data, datalen, t);
            break;
        case RIL_REQUEST_SET_WFC_PROFILE:
            requestSetWfcProfile(data, datalen, t);
            break;
        case RIL_REQUEST_IMS_DEREG_NOTIFICATION:
            requestDeregisterIms(data, datalen, t);
            break;
        case RIL_REQUEST_GET_PROVISION_VALUE:
            LOGD("Ril_ims.c  RIL_REQUEST_GET_PROVISION_VALUE");
            requestGetProvisionValue(data, datalen, t);
            break;
        case RIL_REQUEST_SET_PROVISION_VALUE:
            LOGD("Ril_ims.c  RIL_REQUEST_SET_PROVISION_VALUE");
            requestSetProvisionValue(data, datalen, t);
            break;
        case RIL_REQUEST_IMS_REGISTRATION_STATE:
            requestImsRegistrationState(data, datalen, t);
            break;
        case RIL_REQUEST_SET_IMS_REGISTRATION_REPORT:
            enableImsRegistrationReport(data, datalen, t);
            break;
        case RIL_REQUEST_SET_IMS_RTP_REPORT:
            LOGD("Ril_ims.c  RIL_REQUEST_SET_IMS_RTP_REPORT");
            requestSetImsRtpReport(data, datalen, t);
            break;
        case RIL_REQUEST_SET_MD_IMSCFG:
            requestSetModemImsConfig(data, datalen, t);
            break;
        case RIL_REQUEST_QUERY_VOPS_STATUS:
            queryVopsStatus(data, datalen, t);
            break;
        default:
            return 0;  /* no matched request */
            break;
    }
    return 1; /* request found and handled */
}

extern int rilImsUnsolicited(const char *s, const char *sms_pdu, RILChannelCtx* p_channel)
{
    UNUSED(sms_pdu);
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);
    if (isImsSupport()) {
       if (strStartsWith(s, "+EIMS: 0")) {
           LOGD("EIMS URC:%s",s);
           onImsEnabling(0, rid);
           return 1;
       } else if (strStartsWith(s, "+EIMS: 1")) {
           LOGD("EIMS URC:%s",s);
           onImsEnabling(1, rid);
           return 1;
       } else if (strStartsWith(s, "+EIMCFLAG: 0")) {
           LOGD("EIMCFLAG URC:%s",s);
           onImsEnabled(0, rid);
           return 1;
       } else if (strStartsWith(s, "+EIMCFLAG: 1")) {
           LOGD("EIMCFLAG URC:%s",s);
           onImsEnabled(1, rid);
           return 1;
       } else if (strStartsWith(s, "+CIREGU")) {
           LOGD("CIREG URC:%s",s);
           onImsRegistrationInfo((char *)s, rid);
           return 1;
       } else if (strStartsWith(s, "+EIMSDEREG")) {
           LOGD("EIMSDEREG URC:%s",s);
           onImsDeregistered(rid);
           return 1;
       } else if (strStartsWith(s, "+EPVSGET")) {
           LOGD("EPVSGET URC:%s",s);
           onGetProvisionValue((char *)s, rid);
           return 1;
       } else if (strStartsWith(s, "+EIMSRTPRPT")) {
           LOGD("EIMSRTPRPT URC:%s", s);
           onImsRtpReport((char *)s, rid);
           return 1;
       } else if (strStartsWith(s, "+CIREPI")) {
           onImsVoPSIndication((char *)s, rid);
           return 1;
       }
   }
   return 0;
}

void requestSetAdsReportEnabled(int enable, size_t datalen, RIL_Token t)
{
    UNUSED(datalen);
    char * cmd;
    ATResponse *p_response = NULL;
    int err;

    LOGD("IMS: requestSetAdsReportEnabled enable:%d", enable);
    asprintf(&cmd, "AT+EVADSREP=%d", enable);
    err = at_send_command(cmd, &p_response, OEM_CHANNEL_CTX);
    free(cmd);
    if (err < 0 || p_response->success == 0) {
        LOGD("IMS: requestSetAdsReportEnabled Failed !!");
    }
    at_response_free(p_response);
}

void requestSetModemImsConfig(void * data, size_t datalen, RIL_Token t)
{
    UNUSED(data);
    UNUSED(datalen);

    LOGE("[%s] not support currently", __FUNCTION__);
    RIL_onRequestComplete(t, RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0);
}

void requestSetImsEnabled(void * data, size_t datalen, RIL_Token t)
{
    char * cmd;
    ATResponse *p_response = NULL;
    int err;
    int enable = ((int *)data)[0];
    LOGD("requestSetImsEnabled enable:%d", enable);
    asprintf(&cmd, "AT+EIMS=%d", enable);
    err = at_send_command(cmd, &p_response, OEM_CHANNEL_CTX);
    // Notify modem to enable or disable ADS report
    requestSetAdsReportEnabled(enable, datalen, t);

    free(cmd);
    if (err < 0 || p_response->success == 0) {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
        //IMS_RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        //IMS_RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
    at_response_free(p_response);
}

void requestSetVolteEnabled(void * data, size_t datalen, RIL_Token t)
{
    UNUSED(datalen);
    char * cmd;
    ATResponse *p_response = NULL;
    int err;
    int enable = ((int *)data)[0];
    LOGD(" requestSetVolteEnabled enable:%d", enable);
    asprintf(&cmd, "AT+EIMSVOLTE=%d", enable);
    err = at_send_command(cmd, &p_response, OEM_CHANNEL_CTX);

    free(cmd);
    if (err < 0 || p_response->success == 0) {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
        //IMS_RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        //IMS_RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
    at_response_free(p_response);
}

void requestSetWfcEnabled(void * data, size_t datalen, RIL_Token t)
{
    UNUSED(datalen);
    char * cmd;
    ATResponse *p_response = NULL;
    int err;
    int enable = ((int *)data)[0];
    LOGD(" requestSetWfcEnabled enable:%d", enable);
    asprintf(&cmd, "AT+EIMSWFC=%d", enable);
    err = at_send_command(cmd, &p_response, OEM_CHANNEL_CTX);

    free(cmd);
    if (err < 0 || p_response->success == 0) {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
        //IMS_RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        //IMS_RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
    at_response_free(p_response);
}

void requestSetImsVideoEnabled(void * data, size_t datalen, RIL_Token t)
{
    UNUSED(datalen);
    char * cmd;
    ATResponse *p_response = NULL;
    int err;
    int enable = ((int *)data)[0];
    LOGD(" requestSetImsVideoEnabled enable:%d", enable);
    asprintf(&cmd, "AT+EIMSCCP=%d", enable);
    err = at_send_command(cmd, &p_response, OEM_CHANNEL_CTX);

    free(cmd);
    if (err < 0 || p_response->success == 0) {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
        //IMS_RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        //IMS_RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
    at_response_free(p_response);
}

void requestSetWfcProfile(void * data, size_t datalen, RIL_Token t)
{
    UNUSED(datalen);
    char * cmd;
    ATResponse *p_response = NULL;
    int err;
    int preference = ((int *)data)[0];
    LOGD(" requestSetWfcProfile preference:%d", preference);
    asprintf(&cmd, "AT+EWFCP=%d", preference);
    err = at_send_command(cmd, &p_response, OEM_CHANNEL_CTX);

    free(cmd);
    if (err < 0 || p_response->success == 0) {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
        //IMS_RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        //IMS_RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
    at_response_free(p_response);
}

void requestDeregisterIms(void * data, size_t datalen, RIL_Token t)
{
    UNUSED(datalen);
    char * cmd;
    ATResponse *p_response = NULL;
    int err;
    int reason = ((int *)data)[0];
    LOGD(" requestDeregisterIms reason:%d", reason);
    asprintf(&cmd, "AT+EIMSDEREG=%d", reason);
    err = at_send_command(cmd, &p_response, OEM_CHANNEL_CTX);

    free(cmd);
    if (err < 0 || p_response->success == 0) {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
        //IMS_RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        //IMS_RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
    at_response_free(p_response);
}

/// M: IMS Provisioning. @{
void requestGetProvisionValue(void * data, size_t datalen, RIL_Token t) {
    UNUSED(datalen);
    char *string = (char *)data;
    int err;
    ATResponse *p_response = NULL;
    char *line;
    RIL_Errno errNo = RIL_E_GENERIC_FAILURE;

    LOGD("[requestGetProvisionValue] %s", string);
    asprintf(&line, "AT+EPVSGET=\"%s\"", string);
    LOGD("[requestGetProvisionValue]AT command = %s", line);

    err = at_send_command(line, &p_response, OEM_CHANNEL_CTX);
    free(line);

    if (err < 0 || p_response->success == 0) {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
        if (NULL != p_response && p_response->success == 0) {
            switch (at_get_cme_error(p_response)) {
                case CME_PROVISION_NO_DEFAULT_VALUE:
                    // +CME ERROR: 604
                    LOGD("requestGetProvisionValue: CME_PROVISION_NO_DEFAULT_VALUE");
                    errNo = RIL_E_OEM_ERROR_24; // 524
                    break;
                default:
                    LOGD("requestGetProvisionValue: errno is %d\n", at_get_cme_error(p_response));
                    errNo = RIL_E_GENERIC_FAILURE;
                    break;
            }
        }
        //IMS_RIL_onRequestComplete(t, errNo, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        //IMS_RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
    at_response_free(p_response);
}

void requestSetProvisionValue(void * data, size_t datalen, RIL_Token t) {
    UNUSED(datalen);
    const char**  strings = (const char**)data;
    int err;
    ATResponse *p_response = NULL;
    char *line;

    LOGD("[requestSetProvisionValue] %s, %s", strings[0], strings[1]);
    asprintf(&line, "AT+EPVSSET=\"%s\",\"%s\"", strings[0], strings[1]);
    LOGD("[requestSetProvisionValue]AT command = %s", line);

    err = at_send_command(line, &p_response, OEM_CHANNEL_CTX);
    free(line);

    if (err < 0 || p_response->success == 0) {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
        //IMS_RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        //IMS_RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
    at_response_free(p_response);
}
/// @}

void requestImsRegistrationState(void * data, size_t datalen, RIL_Token t)
{
    UNUSED(data);
    UNUSED(datalen);
    int err;
    int skip;
    int response[2] = { 0 };
    char *line;
    ATResponse *p_response = NULL;
    char* tokenStr = NULL;

    memset(response, 0, sizeof(response));

    err = at_send_command_singleline(
              "AT+CIREG?",
              "+CIREG:", &p_response, OEM_CHANNEL_CTX);

    /* +CIREG: <n>,<reg_info>[,<ext_info>] */
    if (err != 0 || p_response->success == 0 ||
            p_response->p_intermediates == NULL) goto error;

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    /* <mode> */
    err = at_tok_nextint(&line, &skip);
    if (err < 0 || skip < 0 )
    {
        LOGE("The <mode> is an invalid value!!!");
        goto error;
    } else {
        /* <reg_info> value 0: not registered , 1: registered */
        err = at_tok_nextint(&line, &response[0]);
        if (err < 0 ) goto error;

        response[1] = RADIO_TECH_3GPP;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(response));
    at_response_free(p_response);

    return;
error:
    LOGE("requestImsRegistrationState must not return error when radio is on");
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, response, sizeof(response));
    at_response_free(p_response);
}

void requestSetImsRtpReport(void * data, size_t datalen, RIL_Token t) {
    UNUSED(datalen);
    int *c = (int *)data;
    char *cmd;
    int err;
    ATResponse *p_response = NULL;

    // AT+EIMSRTPTS=<default_ebi>,<network_id>,<timer>
    LOGD(" requestSetImsRtpReport pdn_id:%d, network_id:%d, timer:%d", c[0], c[1], c[2]);
    asprintf(&cmd, "AT+EIMSRTPTS=%d,%d,%d", c[0], c[1], c[2]);

    err = at_send_command(cmd, &p_response, OEM_CHANNEL_CTX);
    free(cmd);

    if (err < 0 || p_response->success == 0) {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
    at_response_free(p_response);
}

void enableImsRegistrationReport(void * data, size_t datalen, RIL_Token t)
{
    UNUSED(data);
    UNUSED(datalen);
    char * cmd;
    ATResponse *p_response = NULL;
    int err;
    LOGD("Enable +CIREGU report");
    asprintf(&cmd, "AT+CIREG=2");
    err = at_send_command(cmd, &p_response, OEM_CHANNEL_CTX);

    free(cmd);
    if (err < 0 || p_response->success == 0) {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
        //IMS_RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        //IMS_RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
    at_response_free(p_response);
}

void onImsEnabling(int enable, RIL_SOCKET_ID rid)
{
    LOGD("onImsEnabling enable = %d, RIL_SOCKET_ID = %d", enable, rid);

    if(enable == 1) {
        RIL_UNSOL_RESPONSE(
                RIL_UNSOL_IMS_ENABLE_START,
                NULL, 0, rid);
    } else {
        RIL_UNSOL_RESPONSE(
                RIL_UNSOL_IMS_DISABLE_START,
                NULL, 0, rid);
    }
}

void onImsEnabled(int enable, RIL_SOCKET_ID rid)
{
    LOGD("onImsEnabled enable = %d, RIL_SOCKET_ID = %d", enable, rid);

    if(enable == 1) {
        RIL_UNSOL_RESPONSE(
                RIL_UNSOL_IMS_ENABLE_DONE,
                NULL, 0, rid);
    } else {
        RIL_UNSOL_RESPONSE(
                RIL_UNSOL_IMS_DISABLE_DONE,
                NULL, 0, rid);
    }
}

void onImsRegistrationInfo(char *urc, const RIL_SOCKET_ID rid)
{
    int ret;
    int response[3] = { 0 };
    char* tokenStr = NULL;

    LOGD("onImsRegistrationInfo:%s",urc);

    ret = at_tok_start(&urc);
    if (ret < 0) goto error;

    // get reg_info
    ret = at_tok_nextint(&urc, &response[0]);
    if (ret < 0) goto error;

    // get ext_info , value range is 1~FFFFFFFF
    ret = at_tok_nextstr(&urc, &tokenStr); //hex string
    if (ret < 0) {
        // report mode is 1 , no ext_info available
        response[1] = 0; // set invalid value 0 for upper layer to distinguish if ext_info is availble or not
    } else if (strlen(tokenStr) > 0) {
        response[1] = (int)strtol(tokenStr, NULL, 16);
    }

    // Fix DSDS bug : transfer rid socket information to IMS Service to judgement for valide sim/phone id
    response[2] = rid;

    LOGD("onImsRegistrationInfo reg_info = %d, ext_info = %d, RIL_SOCKET_ID = %d", response[0], response[1], response[2]);
    // Todo: delete this old VoLTE URC
    RIL_UNSOL_RESPONSE(RIL_UNSOL_IMS_REGISTRATION_INFO, response, sizeof(response), rid); // MTK defined UNSOL EVENT (with IMS data info attached)
    // IMS VoLTE refactoring
    //LOGD("IMS: dispatch IMS registration info to IMS phone");
    //IMS_RIL_onUnsolicitedResponse(RIL_UNSOL_IMS_REGISTRATION_INFO, response, sizeof(response), rid); // MTK defined UNSOL EVENT (with IMS data info attached)
    RIL_UNSOL_RESPONSE(RIL_UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED, NULL,0, rid); // for AOSP defined UNSOL EVENT (no data)
    return;

error:
    LOGE("There is something wrong with the +CIREG");
}

void onImsDeregistered(RIL_SOCKET_ID rid)
{
    LOGD("onImsDeregistered RIL_SOCKET_ID = %d", rid);

    RIL_UNSOL_RESPONSE(
        RIL_UNSOL_IMS_DEREG_DONE,
        NULL, 0, rid);
}

void onGetProvisionValue(char *urc, const RIL_SOCKET_ID rid)
{
    int ret, i;
    char *data[3];
    ret = at_tok_start(&urc);
    if (ret < 0) return;

    for (i = 0; i < 2; i++){
        ret = at_tok_nextstr(&urc, &data[i]);
        if (ret < 0) return;
    }

    asprintf(&data[2], "%d", rid);

    RIL_onUnsolicitedResponse(
        RIL_UNSOL_GET_PROVISION_DONE,
        data, 3 * sizeof(char *), rid);
}

void onImsRtpReport(char *urc, const RIL_SOCKET_ID rid) {
    /**
     * +EIMSRTPRPT: <default_ebi>, <network_id>, <timer>, <send pkt lost>, <recv pkt lost>
     * <default_ebi>: pdn_id (0: wifi, 1~15: lte)
     * <network_id>: network_id
     * <timer>: response/sample frequency (ms)
     * <send pkt lost>: from the last received RTCP fraction lost
     * <recv pkt lost>: the statistic received RTP lost rate (0~100)
     */
    int ret, i;
    char *data[7];

    ret = at_tok_start(&urc);
    if (ret < 0) goto error;

    for (i = 0; i < 5; i++) {
        ret = at_tok_nextstr(&urc, &data[i]);
        if (ret < 0) goto error;
    }
    data[5] = (char*)""; //jitter
    data[6] = (char*)""; //delay
    RIL_UNSOL_RESPONSE(RIL_UNSOL_IMS_RTP_INFO, data, 7 * sizeof(char *), rid);
    return;
error:
    LOGE("error in onImsRtpReport");
}

void onImsEccUpdated(char *urc, const RIL_SOCKET_ID rid)
{
    /**
     * +EIMSESS: <rat>, <support_emc>
     *
     */
    int ret, i, ratType = 0, supportEmc = 0;
    int response[1] = { 0 };

    LOGD("onImsEccUpdated:%s", urc);

    ret = at_tok_start(&urc);
    if (ret < 0) goto error;

    ret = at_tok_nextint(&urc, &ratType);
    if (ret < 0) goto error;

    ret = at_tok_nextint(&urc, &supportEmc);
    if (ret < 0) goto error;
    LOGD("onImsEccUpdated: rat :%d, support_emc : %d", ratType, supportEmc);

    if ((ratType == 3 || ratType == 4) && supportEmc == 1) {
        response[0] = 1;
    } else {
        response[0] = 0;
    }
    RIL_UNSOL_RESPONSE(RIL_UNSOL_IMS_SUPPORT_ECC, response,
            sizeof(response), rid);
    return;

error:
    LOGE("error on onImsInfoUpdated");
}

void onImsVoPSIndication(char *urc, const RIL_SOCKET_ID rid) {
    // +CIREPI: <nwimsvops>
    // IMS Voice Over PS sessions (IMSVOPS) supported indication from the network.
    int ret, nwimsvops = 0;
    int response[1] = { 0 };

    LOGD("onImsVoPSIndication:%s", urc);

    ret = at_tok_start(&urc);
    if (ret < 0) goto error;

    ret = at_tok_nextint(&urc, &nwimsvops);
    if (ret < 0) goto error;

    LOGD("onImsVoPSIndication: nwimsvops :%d", nwimsvops);

    response[0] = nwimsvops;

    RIL_UNSOL_RESPONSE(RIL_UNSOL_VOPS_INDICATION, response,
            sizeof(response), rid);
    return;

error:
    LOGE("error on onImsVoPSIndication");

}

void queryVopsStatus(void * data, size_t datalen, RIL_Token t) {
    char *line;
    int err;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    int responses[2] = {0};
    ATResponse *p_response = NULL;

    UNUSED(data);
    UNUSED(datalen);

    LOGD("queryVopsStatus");

    /**
     * Query IMS network reporting
     * AT+CIREP?
     * +CIREP: <reporting>,<nwimsvops>
     */
    err = at_send_command_singleline("AT+CIREP?", "+CIREP:", &p_response, OEM_CHANNEL_CTX);

    if (err < 0 || p_response->success == 0) {
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    /**
         * <reporting> : integer
         * Enables or disables reporting of changes in the IMSVOPS supported
         * indication received from the network and reporting of PS to CS SRVCC,
         * PS to CS vSRVCC and CS to PS SRVCC handover information.
         *
         * 0   Disable reporting
         * 1   Enable reporting
         */
    err = at_tok_nextint(&line, &responses[0]);
    if (err < 0) goto error;

    /**
         * <nwimsvops> : integer
         * Gives the last IMS Voice Over PS sessions (IMSVOPS) supported
         * indication received from network.
         *
         * 0   IMSVOPS support indication is not received from network, or is negative
         * 1   IMSVOPS support indication as received from network is positive
         */
    err = at_tok_nextint(&line, &responses[1]);
    if (err < 0) goto error;

    ret = RIL_E_SUCCESS;

    RIL_onRequestComplete(t, ret, responses, sizeof(responses));
    at_response_free(p_response);
    return;

error:
    RIL_onRequestComplete(t, ret, NULL, 0);
    at_response_free(p_response);
}

