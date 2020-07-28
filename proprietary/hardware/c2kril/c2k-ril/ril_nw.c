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

/* //hardware/viatelecom/ril/viatelecom-withuim-ril/viatelecom-withuim-ril.c
**
** Copyright 2009, Viatelecom Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#include <telephony/mtk_ril.h>
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
#include <getopt.h>
#include <sys/socket.h>
#include <cutils/sockets.h>
#include <termios.h>
#include "atchannel.h"
#include "ril_callbacks.h"
#include <utils/Log.h>

#include <cutils/properties.h>
#include <netdb.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <linux/route.h>

#include "at_tok.h"
#include "hardware/ccci_intf.h"
#include "ril_oem.h"
#include "ril_nw.h"
#include "ril_wp.h"
#include "ril_cc.h"
#include <libmtkrilutils.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "C2K_RIL-NW"

#ifndef UNUSED
#define UNUSED(x) ((void)(x))   // Eliminate "warning: unused parameter"
#endif
#define NW_CHANNEL_CTX getRILChannelCtxFromToken(t)

extern RIL_UNSOL_Msg_Cache s_unsol_msg_cache;
extern int no_service_times;
extern int unlock_pin_outtimes;
extern int inemergency;
extern int currentNetworkMode;
extern int cta_no_uim;
extern int iccid_readable;
extern int cardtype;
extern int s_md3_vpon;
extern int arsithrehd;

extern int uimInsertedStatus;
/*maintain the dbm value reported to framework last time*/
static int last_cdma_dbm = 0;

/*maintain the radio technology reported to framework last time*/
static int last_radio_technology = 0;

/*indicate there exits a repolling network status action in queue*/
static int repollinqueue = 0;

/*maintain the service state reported to framework last time*/
static int last_service_state = 0;

// add for VSER & MODE
static int NetworkMode = -1;

static const struct timeval TIMEVAL_0 = {0, 0};

/* indicate the daylight saving time reported by CTZV for CCLK*/
static int daylightSavingTime = 0;

static int ps_state = 0;

int currentNetworkMode = -1;

extern void trigger_update_cache();
extern void requestSignalStrength(void * data, size_t datalen, RIL_Token t);
extern void requestRegistrationState(int request, void *data, size_t datalen, RIL_Token t);
extern void requestOperator(void *data, size_t datalen, RIL_Token t);
extern void updatePrlInfo(int system, int mode);
/// M: [C2K][IR] Support CT 3g dual Mode card IR feature.
extern int switchStkUtkMode(int mode, RIL_Token t);
extern void setIccidProperty(VIA_ICCID_TYPE type, char* pIccid);
extern int getRUIMState();

static void repollNetworkState(void *param);
static void queryIccid(void *param);
static void requestQueryNetworkSelectionMode(void *data, size_t datalen, RIL_Token t);
static void requestGetCellInfoList(void * data, size_t datalen __unused, RIL_Token t);
static void requestSetPreferredNetworkType(void *data, size_t datalen, RIL_Token t);
static void requestGetPreferredNetworkType(void *data, size_t datalen, RIL_Token t);
static void requestSetRoamingPreference(void *data, size_t datalen, RIL_Token t);
static void requestQueryRoamingPreference(void *data, size_t datalen, RIL_Token t);
static void requestSetSvlteRatMode(void * data, size_t datalen, RIL_Token t);
static void onEngModeInfoUpdate(const char* urc);
static void onNetworkInfo(const char* urc);
/// M: [C2K][IR] Support SVLTE IR feature. @{
static void requestResumeCdmaRegistration(void *data, size_t datalen, RIL_Token t);
static void requestSetCdmaRegSuspendEnabled(void *data, size_t datalen, RIL_Token t);
static int isCdmaRegSuspendEnabled(void);
/// M: [C2K][IR] Support SVLTE IR feature. @}

void resetUnsolMsgCache(void);
void updateNetworkRegistrationStatus(RIL_Token t);

static void onNetworkExistence(const char * urc);

void requestModemPowerOff(void *data, size_t datalen, RIL_Token t);

extern int rilNwMain(int request, void *data, size_t datalen, RIL_Token t){
    switch (request) {
        case RIL_REQUEST_SIGNAL_STRENGTH:
            requestSignalStrength(data, datalen, t);
            break;
        case RIL_REQUEST_VOICE_REGISTRATION_STATE:
        case RIL_REQUEST_DATA_REGISTRATION_STATE:
            requestRegistrationState(request, data, datalen, t);
            break;
        case RIL_REQUEST_OPERATOR:
            requestOperator(data, datalen, t);
            break;
        case RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE:
            requestQueryNetworkSelectionMode(data, datalen, t);
            break;
        case RIL_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC: {
            ATResponse *p_response = NULL;
            int err = at_send_command("AT+COPS=0", &p_response, NW_CHANNEL_CTX);
            if (err < 0 || p_response == NULL || p_response->success == 0) {
                RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
            } else {
                RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
            }
            at_response_free(p_response);
        }
            break;
        case RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE:
            requestSetPreferredNetworkType(data, datalen, t);
            break;
        case RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE:
            requestGetPreferredNetworkType(data, datalen, t);
            break;
        case RIL_REQUEST_CDMA_SET_ROAMING_PREFERENCE:
            requestSetRoamingPreference(data, datalen, t);
            break;
        case RIL_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE:
            requestQueryRoamingPreference(data, datalen, t);
            break;
        /// M: [C2K][IR] Support SVLTE IR feature. @{
        case RIL_REQUEST_RESUME_REGISTRATION_CDMA:
            requestResumeCdmaRegistration(data, datalen, t);
            break;
        case RIL_REQUEST_SET_REG_SUSPEND_ENABLED_CDMA:
            requestSetCdmaRegSuspendEnabled(data, datalen, t);
            break;
        case RIL_LOCAL_C2K_REQUEST_SET_SVLTE_RAT_MODE:
            requestSetSvlteRatMode(data, datalen, t);
        break;
        /// M: [C2K][IR] Support SVLTE IR feature. @}
        /// M: [Network][C2K] Sprint roaming control @{
        case RIL_REQUEST_SET_ROAMING_ENABLE:
            setRoamingConfig(data, datalen, t);
            break;
        case RIL_REQUEST_GET_ROAMING_ENABLE:
            getRoamingConfig(data, datalen, t);
            break;
        case RIL_REQUEST_GET_CELL_INFO_LIST:
            requestGetCellInfoList(data, datalen, t);
            break;
        /// @}
        default:
            return 0; /* no matched request */
    }
    return 1;
}

extern int rilNwUnsolicited(const char *s, const char *sms_pdu) {
    UNUSED(sms_pdu);
    char *line = NULL;
    char * output_string = NULL;
    int err;

    if (strStartsWith(s, "^SYSINFO:")) {
        int sysinfo_service_mode = 0;
        int sysinfo_roaming = 0;
        int skip = 0;
        char *dup = strdup(s);

        if (!dup) {
            return 1;
        }
        line = dup;
        err = at_tok_start(&line);
        if (err < 0) {
            free(dup);
            return 1;
        }
        err = at_tok_nextint(&line, &skip);
        if (err < 0) {
            free(dup);
            return 1;
        }
        err = at_tok_nextint(&line, &skip);
        if (err < 0) {
            free(dup);
            return 1;
        }
        err = at_tok_nextint(&line, &sysinfo_roaming);
        if (err < 0) {
            free(dup);
            return 1;
        }
        err = at_tok_nextint(&line, &sysinfo_service_mode);
        if (err < 0) {
            free(dup);
            return 1;
        }

        // s_unsol_msg_cache.evdo_service_state:1--> Do in service
        // s_unsol_msg_cache.evdo_service_state:0--> Do no service
        // s_unsol_msg_cache.x1_service_state:2-->1x is searching
        // s_unsol_msg_cache.x1_service_state:1--> 1x in service
        // s_unsol_msg_cache.x1_service_state:0--> 1x no service
        // s_unsol_msg_cache.service_state:2-->framework searching
        // s_unsol_msg_cache.service_state:1-->framework in service
        // s_unsol_msg_cache.service_state:0-->framework no service
        LOGD("SYSINFO:NetworkMode is:%d, inemergency = %d", NetworkMode, inemergency);
        switch (NetworkMode) {
        case NET_TYPE_CDMA_ONLY:
            break;
        case NET_TYPE_EVDO_ONLY:
            if ((sysinfo_service_mode == 0)
                    || ((inemergency == 0)
                    && (RADIO_STATE_ON != getRadioState() || RUIM_READY != getRUIMState()))) {
                // ((Do is no service) ||(not RUIM_READY && not in inemergency), set no service
                LOGD("SYSINFO:Do only mode, Do no service");
                s_unsol_msg_cache.service_state = 0;
                s_unsol_msg_cache.evdo_service_state = 0;
            } else if (sysinfo_service_mode != 0) {
                // Do is in service, set in service
                LOGD("SYSINFO:Do only mode, Do in service");
                s_unsol_msg_cache.service_state = 1;
                s_unsol_msg_cache.evdo_service_state = 1;
            }
            break;
        case NET_TYPE_CDMA_EVDO_HYBRID:
            if (((sysinfo_service_mode == 0)
                    && (s_unsol_msg_cache.x1_service_state == 0))
                    || ((inemergency == 0)
                    && (RADIO_STATE_ON != getRadioState() || RUIM_READY != getRUIMState()))) {
                // ((Do is no service) && (1x is no service)) ||(not RUIM_READY  && not in inemergency), set no service
                LOGD("SYSINFO:hybrid mode, Do no service, 1x no service");
                s_unsol_msg_cache.service_state = 0;
                s_unsol_msg_cache.evdo_service_state = 0;
            } else if (sysinfo_service_mode != 0) {
                // Do or 1x is in service, set in service
                LOGD("SYSINFO:hybrid mode, Do/1x in service");
                s_unsol_msg_cache.service_state = 1;
                s_unsol_msg_cache.evdo_service_state = 1;
            } else if ((sysinfo_service_mode == 0)
                    && (s_unsol_msg_cache.x1_service_state == 2)) {
                // beacuse if Do is no service, but 1x in searching(VSER: 1, x1_service_state == 2), sysinfo_service_mode == 0
                LOGD(
                        "SYSINFO:hybrid mode, Do is no service, 1x is in searching");
                // in this situation, keep service_state value as before
                s_unsol_msg_cache.evdo_service_state = 0;
            } else {
                LOGD("SYSINFO:other case......sysinfo_service_mode:%d, s_unsol_msg_cache.x1_service_state:%d",
                        sysinfo_service_mode,
                        s_unsol_msg_cache.x1_service_state);
            }
            break;
        default:
            LOGD("SYSINFO:why come here? NetworkMode:%d", NetworkMode);
            break;
        }

        int radio_technology;
        if (sysinfo_service_mode == 8 || sysinfo_service_mode == 4) {
            radio_technology = 8;
        } else if (sysinfo_service_mode == 2) {
            radio_technology = 6;
        } else {
            radio_technology = 0;
        }
        if (!isCdmaLteDcSupport()) {
            s_unsol_msg_cache.radio_technology = radio_technology;
        } else {
            if (s_unsol_msg_cache.radio_technology < 13) {
                s_unsol_msg_cache.radio_technology = radio_technology;
            }
        }
        s_unsol_msg_cache.sysinfo_roam_status = sysinfo_roaming;

        RIL_onUnsolicitedResponse(
                RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, NULL, 0);

        free(dup);
        return 1;
    } else if (strStartsWith(s, "+CREG:")) {
        char *dup = strdup(s);

        int skip = 0;
        int commas_count = 0;
        char * p = NULL;
        if (!dup) {
            return 1;
        }
        line = dup;
        err = at_tok_start(&line);
        if (err < 0) {
            free(dup);
            return 1;
        }
        for (p = line; *p != '\0'; p++) {
            if (*p == ',') {
                commas_count++;
            }
        }
        switch (commas_count) {
        case 0: /* +CREG: <stat> */
        case 2: /* +CREG: <stat>, <lac>, <cid> */
            err = at_tok_nextint(&line, &(s_unsol_msg_cache.register_state));
            if (err < 0) {
                free(dup);
                return 1;
            }
            break;
        case 1: /* +CREG: <n>, <stat>*/
        case 3: /* +CREG: <n>, <stat>, <lac>, <cid> */
        case 4: /* +CREG: <n>, <stat>, <lac>, <cid>, <networkType> */
            err = at_tok_nextint(&line, &skip);
            if (err < 0) {
                free(dup);
                return 1;
            }
            err = at_tok_nextint(&line, &(s_unsol_msg_cache.register_state));
            if (err < 0) {
                free(dup);
                return 1;
            }
            break;
        default:
            LOGD("unsupport +CREG");
            break;
        }
        free(dup);
        RIL_onUnsolicitedResponse(
                RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, NULL, 0);
        return 1;
    } else if (strStartsWith(s, "+ECGREG:")) {
        if (isCdmaLteDcSupport()) {
            char *dup = strdup(s);
            int state = 0;
            int ps_type = 0;
            int response[1];
            int commas_count = 0;
            char * p = NULL;
            int skip = 0;
            if (!dup) {
                return 1;
            }
            line = dup;
            err = at_tok_start(&line);
            if (err < 0) {
                free(dup);
                return 1;
            }
            for (p = line; *p != '\0'; p++) {
                if (*p == ',') {
                    commas_count++;
                }
            }
            switch (commas_count) {
            case 0: /* +ECGREG: <state> */
                err = at_tok_nextint(&line, &state);
                if (err < 0) {
                    free(dup);
                    return 1;
                }
                break;
            case 1: /* +ECGREG: <state>, <ps_type> */
                err = at_tok_nextint(&line, &state);
                if (err < 0) {
                    free(dup);
                    return 1;
                }
                err = at_tok_nextint(&line, &ps_type);
                if (err < 0) {
                    free(dup);
                    return 1;
                }
                break;
            case 2:/* response: +ECGREG: <n>, <state>, <ps_type> */
                err = at_tok_nextint(&line, &skip);
                if (err < 0) {
                    free(dup);
                    return 1;
                }
                err = at_tok_nextint(&line, &state);
                if (err < 0) {
                    free(dup);
                    return 1;
                }
                err = at_tok_nextint(&line, &ps_type);
                if (err < 0) {
                    free(dup);
                    return 1;
                }
                break;
            default:
                LOGD("unsupport ECGREG");
                break;
            }
            free(dup);

            if (state == 0) {
                ps_state = 0;
                s_unsol_msg_cache.radio_technology = 0;
            } else if (state == 1) {
                ps_state = 1;
                if (ps_type == 2) {
                    s_unsol_msg_cache.radio_technology = 13;
                } else if (ps_type == 1) {
                    s_unsol_msg_cache.radio_technology = 8;
                } else if (ps_type == 0) {
                    s_unsol_msg_cache.radio_technology = 6;
                }
            }
            response[0] = s_unsol_msg_cache.radio_technology;
            RIL_onUnsolicitedResponse(
                    RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, NULL, 0);
        }
       return 1;
     } else if (strStartsWith(s, "+VMCCMNC:")) {
        char *dup = strdup(s);
        int skip = 0;
        int commas_count = 0;
        char * p = NULL;
        char *mccmnc = NULL;
        if (!dup) {
            return 1;
        }
        line = dup;
        err = at_tok_start(&line);
        if (err < 0) {
            free(dup);
            return 1;
        }
        for (p = line; *p != '\0'; p++) {
            if (*p == ',')
                commas_count++;
        }
        switch (commas_count) {
            case 1: /* +VMCCMNC:<MCC>,<MNC> */
                err = at_tok_nextstr(&line, &output_string);
                if (err < 0) {
                    free(dup);
                    return 1;
                }
                strncpy(s_unsol_msg_cache.mcc, output_string, 7);
                err = at_tok_nextstr(&line, &output_string);
                if (err < 0) {
                    free(dup);
                    return 1;
                }
                strncpy(s_unsol_msg_cache.mnc, output_string, 7);
                break;
            case 2: /* +VMCCMNC:<MccMnc>,<MCC>,<MNC> */
                err = at_tok_nextint(&line, &skip);
                if (err < 0) {
                    free(dup);
                    return 1;
                }
                err = at_tok_nextstr(&line, &output_string);
                if (err < 0) {
                    free(dup);
                    return 1;
                }
                strncpy(s_unsol_msg_cache.mcc, output_string, 7);
                err = at_tok_nextstr(&line, &output_string);
                if (err < 0) {
                    free(dup);
                    return 1;
                }
                strncpy(s_unsol_msg_cache.mnc, output_string, 7);
                break;
            default:
                LOGD("unsupport VMCCMNC");
                break;
            }

        // Modem might response invalid mcc/mnc ex:"N/A", mcc convert to "000" and nmc convert to "00"
        if (strcmp(s_unsol_msg_cache.mcc, "N/A") == 0 || strcmp(s_unsol_msg_cache.mnc, "N/A") == 0) {
            LOGD("URC+VMCCMNC, modem response invalid value:N/A");
            s_unsol_msg_cache.mcc[0] = '\0';
            s_unsol_msg_cache.mnc[0] = '\0';
        }

        /// Notify CC to refresh emergency list @{
        asprintf(&mccmnc, "%s%s", s_unsol_msg_cache.mcc, s_unsol_msg_cache.mnc);
        onMccMncIndication(mccmnc);
        free(mccmnc);
        /// @}

        free(dup);
        RIL_onUnsolicitedResponse(
                RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, NULL, 0);
        return 1;
     } else if (strStartsWith(s, "+VROM:")) {
        char *dup = strdup(s);
        int skip = 0;
        int commas_count = 0;
        char * p = NULL;
        if (!dup) {
            return 1;
        }
        line = dup;
        err = at_tok_start(&line);
        if (err < 0) {
            free(dup);
            return 1;
        }
        for (p = line; *p != '\0'; p++) {
            if (*p == ',') {
                commas_count++;
                }
        }
        switch (commas_count) {
            case 0: /* URC: +VROM <roaming_indicator>*/
                err = at_tok_nextint(&line, &(s_unsol_msg_cache.roaming_indicator));
                if (err < 0) {
                    free(dup);
                    return 1;
                }
                break;
            case 1:/* response: +VROM <n><roaming_indicator>*/
                err = at_tok_nextint(&line, &skip);
                if (err < 0) {
                    free(dup);
                    return 1;
                }
                err = at_tok_nextint(&line, &(s_unsol_msg_cache.roaming_indicator));
                if (err < 0) {
                    free(dup);
                    return 1;
                }
                break;
            default:
                break;
        }
        free(dup);
        if (s_unsol_msg_cache.roaming_indicator > 255) {
            s_unsol_msg_cache.roaming_indicator = 1;
        }
        RIL_requestProxyTimedCallback(repollNetworkState, NULL, &TIMEVAL_0, DEFAULT_CHANNEL);
        repollinqueue = 1;
        return 1;
    }  else if (strStartsWith(s, "+EIPRL:")) {
        // +EIPRL:<system1>,<mode> [,<system2>,<mode>]
        // <system> : integer type; system type
        // 0    1xRTT
        // 1    EvDO
        // <mode>:integer type
        // 0    The system is not in PRL
        // 1    The system is in PRL
        // 255  Invalid value(used in network lost)

        char *dup = strdup(s);
        int system = -1;
        int mode = 255;

        if (!dup) {
          return 1;
        }

        line = dup;
        err = at_tok_start(&line);
        if (err < 0) {
          free(dup);
          return 1;
        }

        err = at_tok_nextint(&line, &system);
        if (err < 0) {
          free(dup);
          return 1;
        }

        err = at_tok_nextint(&line, &mode);
        if (err < 0) {
          free(dup);
          return 1;
        }
        updatePrlInfo(system, mode);

        if(at_tok_hasmore(&line)){
          err = at_tok_nextint(&line, &system);
          if (err < 0) {
              free(dup);
              return 1;
          }

          err = at_tok_nextint(&line, &mode);
          if (err < 0) {
              free(dup);
              return 1;
          }
          updatePrlInfo(system, mode);
        }
        free(dup);

        RIL_onUnsolicitedResponse(
              RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, NULL, 0);
        return 1;
    } else if (strStartsWith(s, "+EDEFROAM:")) {
        char *dup = strdup(s);
        int skip = 0;
        int commas_count = 0;
        char * p = NULL;
        if (!dup) {
            return 1;
        }
        line = dup;
        err = at_tok_start(&line);
        if (err < 0) {
            free(dup);
            return 1;
        }
        for (p = line; *p != '\0'; p++) {
            if (*p == ',') {
                commas_count++;
                }
        }
        switch (commas_count) {
            case 0: // URC: +EDEFROAM <prl_roaming_indicator>
                err = at_tok_nextint(&line, &(s_unsol_msg_cache.prl_roaming_indicator));
                if (err < 0) {
                    free(dup);
                    return 1;
                }
                break;
            case 1:// response: +EDEFROAM <n><prl_roaming_indicator>
                err = at_tok_nextint(&line, &skip);
                if (err < 0) {
                    free(dup);
                    return 1;
                }
                err = at_tok_nextint(&line, &(s_unsol_msg_cache.prl_roaming_indicator));
                if (err < 0) {
                    free(dup);
                    return 1;
                }
                break;
            default:
                break;
        }
        free(dup);
        RIL_onUnsolicitedResponse(
                RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, NULL, 0);
        return 1;
    } else if (strStartsWith(s, "^MODE:")) {
        int service_mode = 0;
        char *dup = strdup(s);

        if (!dup) {
            return 1;
        }

        line = dup;
        err = at_tok_start(&line);
        if (err < 0) {
            free(dup);
            return 1;
        }
        err = at_tok_nextint(&line, &service_mode);
        if (err < 0) {
            free(dup);
            return 1;
        }
        free(dup);

        // s_unsol_msg_cache.evdo_service_state:1--> Do in service
        // s_unsol_msg_cache.evdo_service_state:0--> Do no service
        // s_unsol_msg_cache.x1_service_state:2-->1x is searching
        // s_unsol_msg_cache.x1_service_state:1--> 1x in service
        // s_unsol_msg_cache.x1_service_state:0--> 1x no service
        // s_unsol_msg_cache.service_state:2-->framework searching
        // s_unsol_msg_cache.service_state:1-->framework in service
        // s_unsol_msg_cache.service_state:0-->framework no service
        LOGD("MODE:NetworkMode is:%d, inemergency = %d", NetworkMode, inemergency);
        switch (NetworkMode) {
        case NET_TYPE_CDMA_ONLY:
            break;
        case NET_TYPE_EVDO_ONLY:
            if ((service_mode == 0)
                    || ((inemergency == 0)
                    && (RADIO_STATE_ON != getRadioState() || RUIM_READY != getRUIMState()))) {
                // ((Do is no service) ||(not RUIM_READY && not in inemergency), set no service
                LOGD("MODE:Do only mode, Do no service");
                s_unsol_msg_cache.service_state = 0;
                s_unsol_msg_cache.evdo_service_state = 0;
            } else {
                // Do is in service, set in service
                LOGD("MODE:Do only mode, Do in service");
                s_unsol_msg_cache.service_state = 1;
                s_unsol_msg_cache.evdo_service_state = 1;
            }
            break;
        case NET_TYPE_CDMA_EVDO_HYBRID:
            if (((service_mode == 0)
                    && (s_unsol_msg_cache.x1_service_state == 0))
                    || ((inemergency == 0)
                    && (RADIO_STATE_ON != getRadioState() || RUIM_READY != getRUIMState()))) {

                // ((Do is no service) && (1x is no service)) ||(not RUIM_READY  && not in inemergency), set no service
                LOGD("MODE:hybrid mode, Do no service, 1x no service");
                s_unsol_msg_cache.service_state = 0;
                s_unsol_msg_cache.evdo_service_state = 0;
            } else if (service_mode != 0) {
                // Do/1x is in service, set in service
                LOGD("MODE:hybrid mode, Do/1x in service");
                s_unsol_msg_cache.service_state = 1;
                s_unsol_msg_cache.evdo_service_state = 1;
            } else if ((service_mode == 0)
                    && (s_unsol_msg_cache.x1_service_state == 2)) {
                // Do is no service, but 1x may in searching
                LOGD("MODE:hybrid mode, DO is no service, 1x is searching");
                // in this situation, keep service_state value as before
                s_unsol_msg_cache.evdo_service_state = 0;
            } else {
                LOGD(
                        "MODE:hybrid mode, other case, service_mode:%d, s_unsol_msg_cache.evdo_service_state:%d",
                        service_mode, s_unsol_msg_cache.evdo_service_state);
            }
            break;
        default:
            LOGD("MODE:why come here? NetworkMode:%d", NetworkMode);
            break;
        }
        int radio_technology;
        if (service_mode == 8 || service_mode == 4) {
            radio_technology = 8;
        } else if (service_mode == 2) {
            radio_technology = 6;
        } else {
            radio_technology = 0;
        }
        if (!isCdmaLteDcSupport()) {
            s_unsol_msg_cache.radio_technology = radio_technology;
        } else {
            if (s_unsol_msg_cache.radio_technology < 13) {
                s_unsol_msg_cache.radio_technology = radio_technology;
            }
        }

        RIL_onUnsolicitedResponse(
                RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, NULL, 0);
        return 1;
     } else if (strStartsWith(s, "+VSER:")) {
        int skip = 0;
        char *dup = NULL;
        char *p = NULL;
        int commas_count = 0;
        int service_state_1x = 0;

        dup = strdup(s);
        if (!dup) {
            return 1;
        }

        line = dup;
        err = at_tok_start(&line);
        if (err < 0) {
            free(dup);
            return 1;
        }
        for (p = line; *p != '\0'; p++) {
            if (*p == ',')
                commas_count++;
        }
        switch (commas_count) {
        case 0:
            err = at_tok_nextint(&line, &service_state_1x);
            if (err < 0) {
                free(dup);
                return 1;
            }
            break;
        case 1:
            err = at_tok_nextint(&line, &skip);
            if (err < 0) {
                free(dup);
                return 1;
            }
            err = at_tok_nextint(&line, &service_state_1x);
            if (err < 0) {
                free(dup);
                return 1;
            }
            break;
        default:
            break;
        }

        /*
         the value of "+VSER:" could't match with framework's, do converting here.
         +VSER: 0 in service,
         1 out of service,but searching
         2 out of service, and stop searching
         3 limited service
         s_unsol_msg_cache.evdo_service_state:1--> Do in service
         s_unsol_msg_cache.evdo_service_state:0--> Do no service
         s_unsol_msg_cache.x1_service_state:2-->1x is searching
         s_unsol_msg_cache.x1_service_state:1--> 1x in service
         s_unsol_msg_cache.x1_service_state:0--> 1x no service
         s_unsol_msg_cache.service_state:2-->framework searching
         s_unsol_msg_cache.service_state:1-->framework in service
         s_unsol_msg_cache.service_state:0-->framework no service
         */
        LOGD("VSER:NetworkMode is:%d, service_state_1x is:%d", NetworkMode, service_state_1x);
        switch (NetworkMode) {
        case NET_TYPE_CDMA_ONLY:
            if (service_state_1x == 0) {
                LOGD("VSER:1X only mode, 1x in service");
                s_unsol_msg_cache.service_state = 1;
                s_unsol_msg_cache.x1_service_state = 1;
            } else if (service_state_1x == 1) {
                LOGD("VSER:1X only mode, 1x searching");
                s_unsol_msg_cache.service_state = 2;
                s_unsol_msg_cache.x1_service_state = 2;
            } else if (service_state_1x == 2) {
                LOGD("VSER:1X only mode, 1x no service");
                s_unsol_msg_cache.service_state = 0;
                s_unsol_msg_cache.x1_service_state = 0;
            } else if (service_state_1x == 3) {
                // [ALPS02039951]
                LOGD("VSER:1X only mode, 1x limited service");
                s_unsol_msg_cache.service_state = 0;
                s_unsol_msg_cache.x1_service_state = 0;
            }
            break;
        case NET_TYPE_EVDO_ONLY:
            LOGD("VSER:Do Only, set 1x to no service");
            s_unsol_msg_cache.service_state = 0;
            s_unsol_msg_cache.x1_service_state = 0;
            break;
        case NET_TYPE_CDMA_EVDO_HYBRID:
            if (service_state_1x == 0) {
                LOGD("VSER:hybrid mode, 1x in service");
                s_unsol_msg_cache.service_state = 1;
                s_unsol_msg_cache.x1_service_state = 1;
            } else if (service_state_1x == 1) {
                if (s_unsol_msg_cache.evdo_service_state == 1) {
                    // 1x searching, DO in service, set in service
                    LOGD("VSER:hybrid mode, 1x searching Do in service");
                    s_unsol_msg_cache.service_state = 1;
                } else {
                    // 1x searching, DO no service, set searching
                    LOGD("VSER:hybrid mode, 1x searching Do no service");
                    s_unsol_msg_cache.service_state = 2;
                }
                s_unsol_msg_cache.x1_service_state = 2;
            } else if (service_state_1x == 2) {
                if (s_unsol_msg_cache.evdo_service_state == 0) {
                    // 1x No service, DO no service, set no service
                    LOGD("VSER:hybrid mode, 1x no service, Do in service");
                    s_unsol_msg_cache.service_state = 0;
                }
                // otherwise keep service_state value as before
                LOGD("VSER:hybrid mode, 1x no service Do in service");
                s_unsol_msg_cache.x1_service_state = 0;
            } else if (service_state_1x == 3) {
                // [ALPS02039951]
                if (s_unsol_msg_cache.evdo_service_state == 0) {
                    // 1x limited service, DO no service, set limited service
                    LOGD("VSER:hybrid mode, 1x limited service, Do no service");
                    s_unsol_msg_cache.service_state = 0;
                } else {
                    // 1x limited service, DO in service, set in service
                    LOGD("VSER:hybrid mode, 1x limited service, Do in service");
                    s_unsol_msg_cache.service_state = 1;
                }
                // otherwise keep service_state value as before
                LOGD("VSER:hybrid mode, 1x limited service");
                s_unsol_msg_cache.x1_service_state = 0;
            }
            break;
        default:
            LOGD("VSER:why come here?currentNetworkMode:%d",
                    currentNetworkMode);
            break;
        }

        LOGD("VSER:%s, s_unsol_msd_cache.service_state:%d", __FUNCTION__,
                s_unsol_msg_cache.service_state);

        free(dup);
        RIL_onUnsolicitedResponse(
                RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, NULL, 0);
        return 1;
     } else if (strStartsWith(s, "+HDRCSQ")) {
         int evdo_dbm = 0;
         int evdo_ber = 0;
         int evdo_ecio = 0;
         int commas_count = 0;
         char *dup = strdup(s);
         char *p = NULL;

         if (!dup) {
             return 1;
         }

         line = dup;
         err = at_tok_start(&line);
         if (err < 0) {
             free(dup);
             return 1;
         }
         for (p = line; *p != '\0'; p++) {
             if (*p == ',') {
                 commas_count++;
             }
         }
         switch (commas_count) {
         case 0: /* +HDRCSQ: <rssi> */
         case 1:
             err = at_tok_nextint(&line, &evdo_dbm);
             if (err < 0) {
                 free(dup);
                 return 1;
             }
             break;
         case 2: /* +HDRCSQ: <rssi>,<ber>,<ecio> */
             err = at_tok_nextint(&line, &evdo_dbm);
             if (err < 0) {
                 free(dup);
                 return 1;
             }
             err = at_tok_nextint(&line, &evdo_ber);
             if (err < 0) {
                 free(dup);
                 return 1;
             }
             err = at_tok_nextint(&line, &evdo_ecio);
             if (err < 0) {
                 free(dup);
                 return 1;
             }
             break;
         default:
             LOGD("unsupported +HDRCSQ");
             break;
         }

        evdo_dbm = 113 - evdo_dbm * 2;
        if (evdo_dbm == 113) {
            evdo_dbm = 120;
        }
         s_unsol_msg_cache.signal_strength.EVDO_SignalStrength.dbm = evdo_dbm;

         if (commas_count == 2) {
             evdo_ecio = evdo_ecio / 8;
             s_unsol_msg_cache.signal_strength.EVDO_SignalStrength.ecio = evdo_ecio;
         }

         free(dup);

         if(s_unsol_msg_cache.signal_strength.CDMA_SignalStrength.dbm > 100)
         {
             // if no signal
             LOGD("1X signal is 0, poll network state");
             RIL_onUnsolicitedResponse(
                     RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, NULL, 0);
         } else {
             RIL_onUnsolicitedResponse(RIL_UNSOL_SIGNAL_STRENGTH,
                     &(s_unsol_msg_cache.signal_strength),
                     sizeof(RIL_SignalStrength_v5));
         }
         return 1;
     } else if (strStartsWith(s, "^HDRSINR:")) {
        int sinr = 0;
        int sinr_dB;
        int cvt_sinr_dB = 0;
        char *dup = strdup(s);

        if (!dup) {
            return 1;
        }

        line = dup;
        err = at_tok_start(&line);
        if (err < 0) {
            free(dup);
            return 1;
        }
        err = at_tok_nextint(&line, &sinr);
        if (err < 0) {
            free(dup);
            return 1;
        }
        err = at_tok_nextint(&line, &sinr_dB);
        if (err < 0) {
            free(dup);
            return 1;
        }
        LOGD("sinr:%d, sinr_dB:%d", sinr, sinr_dB);
        if (sinr_dB >= 100) {
            cvt_sinr_dB = 8;
        } else if (sinr_dB >= 70) {
            cvt_sinr_dB = 7;
        } else if (sinr_dB >= 50) {
            cvt_sinr_dB = 6;
        } else if (sinr_dB >= 30) {
            cvt_sinr_dB = 5;
        } else if (sinr_dB >= -20) {
            cvt_sinr_dB = 4;
        } else if (sinr_dB >= -45) {
            cvt_sinr_dB = 3;
        } else if (sinr_dB >= -90) {
            cvt_sinr_dB = 2;
        } else if (sinr_dB > -120) {
            cvt_sinr_dB = 1;
        } else {
            cvt_sinr_dB = 0;
        }
        s_unsol_msg_cache.signal_strength.EVDO_SignalStrength.signalNoiseRatio = cvt_sinr_dB;
        RIL_onUnsolicitedResponse(RIL_UNSOL_SIGNAL_STRENGTH,
                 &(s_unsol_msg_cache.signal_strength), sizeof(RIL_SignalStrength_v5));
        free(dup);
        return 1;
     } else if (strStartsWith(s, "+CSQ:")) {
         int cdma_dbm = 0;
         int cdma_ber = 0;
         int cdma_ecio = 0;
         int commas_count = 0;
         char *dup = strdup(s);
         char *p = NULL;

         if (!dup) {
             return 1;
         }

         line = dup;
         err = at_tok_start(&line);
         if (err < 0) {
             free(dup);
             return 1;
         }
         for (p = line; *p != '\0'; p++) {
             if (*p == ',') {
                 commas_count++;
             }
         }
         LOGD("CSQ:commas_count is %d", commas_count);
         switch (commas_count) {
         case 0: /* +CSQ: <rssi> */
         case 1:
             err = at_tok_nextint(&line, &cdma_dbm);
             if (err < 0) {
                 free(dup);
                 return 1;
             }
             break;
         case 2: /* +CSQ: <rssi>,<ber>,<ecio> */
             err = at_tok_nextint(&line, &cdma_dbm);
             if (err < 0) {
                 free(dup);
                 return 1;
             }
             err = at_tok_nextint(&line, &cdma_ber);
             if (err < 0) {
                 free(dup);
                 return 1;
             }
             err = at_tok_nextint(&line, &cdma_ecio);
             if (err < 0) {
                 free(dup);
                 return 1;
             }
             break;
         default:
             LOGD("unsupported +CSQ");
             break;
         }
         cdma_dbm = 113 - cdma_dbm * 2;  // 31 -->  -51dbm
         if (cdma_dbm == 113) {
             cdma_dbm = 120;
         }
         // -5 => -10 / 2 (-10 follow AOSP ril define, 2 is to normalize MD3 ecio data)
         cdma_ecio = cdma_ecio * -5;
         free(dup);
         if (cdma_dbm> 100)
         {
             // if no signal
             LOGD("1X signal is 0, poll network state");
             RIL_onUnsolicitedResponse(
                     RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, NULL, 0);
         } else {
             s_unsol_msg_cache.signal_strength.CDMA_SignalStrength.dbm = cdma_dbm;
             LOGD("onUnsolicited s_unsol_msg_cache.signal_strength.CDMA_SignalStrength.dbm = %d",
                     s_unsol_msg_cache.signal_strength.CDMA_SignalStrength.dbm);
             last_cdma_dbm = cdma_dbm;
             if (commas_count == 2) {
                 s_unsol_msg_cache.signal_strength.CDMA_SignalStrength.ecio = cdma_ecio;
                 LOGD("onUnsolicited s_unsol_msg_cache.signal_strength.CDMA_SignalStrength.ecio = %d",
                         s_unsol_msg_cache.signal_strength.CDMA_SignalStrength.ecio);
             }
             RIL_onUnsolicitedResponse(RIL_UNSOL_SIGNAL_STRENGTH,
                     &(s_unsol_msg_cache.signal_strength),
                     sizeof(RIL_SignalStrength_v6));
         }
         return 1;
     } else if (strStartsWith(s,"+CCLK:")) {
        char *dup = strdup(s);
        char yearstring[5];
        int year = 2011;

        // +CCLK:<time>
        // <time>: string type; format is "yy/MM/dd,hh:mm:ss zz"

        if (!dup) {
            return 1;
        }
        line = dup;
        err = at_tok_start(&line);
        if (err < 0) {
            free(dup);
            return 1;
        }

        memset(yearstring, 0, sizeof(yearstring));
        strncpy(yearstring, line, 4);
        year = atoi(yearstring);
        LOGD("line = %s  year = %d", line, year);
        if (year < 2012) {
            LOGE("wrong time, return!!");
            free(dup);
            return 1;
        }
        char * timeStringWithoutYear = NULL;
        timeStringWithoutYear = strchr(line, '/');
        char *formatedYear = NULL;
        asprintf(&formatedYear, "%d%s,%d", year - 2000, timeStringWithoutYear, daylightSavingTime);
        LOGD("After convert time format the dstTime = %s, formatedYear = %s ", timeStringWithoutYear, formatedYear);
        RIL_onUnsolicitedResponse(RIL_UNSOL_NITZ_TIME_RECEIVED, formatedYear,
                strlen(formatedYear));
        free(formatedYear);
        free(dup);
        return 1;
    } else if (strStartsWith(s,"+CTZV:")) {
        char *dup = strdup(s);
        /*
         *   +CTZV:<year>,<month>,<day>,<hour>,<minute>,<second>,<sign_flag>,<time_zone>,<day_lt>
         *   0: year, two last digits of year
         *   1: month
         *   2: day
         *   3: hour
         *   4: minute
         *   5: second
         */
        int times[6] = { 0 };
        int time_zone = 0;
        int dst = 0;
        int i = 0;
        char *sign_flag = NULL;
        char *timeString = NULL;

        if (!dup) {
            return 1;
        }
        line = dup;
        err = at_tok_start(&line);
        if (err < 0) {
            free(dup);
            return 1;
        }
        LOGD("line = %s", line);

        for (i = 0; i < 6; i++) {
            err = at_tok_nextint(&line, &times[i]);
            if (err < 0) {
                free(dup);
                return 1;
            }
        }
        err = at_tok_nextstr(&line, &sign_flag);
        if (err < 0) {
            free(dup);
            return 1;
        }
        err = at_tok_nextint(&line, &time_zone);
        if (err < 0) {
            free(dup);
            return 1;
        }
        err = at_tok_nextint(&line, &dst);
        if (err < 0) {
            LOGD("ERROR occurs when parsing <localTimeOffset> of URC+CTZV");
        } else {
            daylightSavingTime = dst;
        }

        asprintf(&timeString, "%d/%d/%d,%d:%d:%d%s%d,%d", times[0], times[1],
                times[2], times[3], times[4], times[5], sign_flag, time_zone, daylightSavingTime);
        LOGD("timeString is %s", timeString);
        RIL_onUnsolicitedResponse(RIL_UNSOL_NITZ_TIME_RECEIVED, timeString,
                strlen(timeString));
        free(timeString);
        free(dup);
        return 1;
    } else if (strStartsWith(s, "+WPRL: ")) {
        int version = -1;
        char *dup = NULL;
        dup = strdup(s);
        if (!dup) {
            LOGE("+WPRL: Unable to allocate memory");
            return 1;
        }
        line = dup;
        if (at_tok_start(&line) < 0) {
            LOGE("invalid +WPRL response: %s", s);
            free(dup);
            return 1;
        }
        if (at_tok_nextint(&line, &version) < 0) {
            LOGE("invalid +WPRL response: %s", s);
            free(dup);
            return 1;
        }
        free(dup);
        RIL_onUnsolicitedResponse(RIL_UNSOL_CDMA_PRL_CHANGED, &version,
                sizeof(version));
        return 1;
    } else if (strStartsWith(s, "+CIEV:")) {
        int ind;
        int val;
        int prl;

        char *dup = strdup(s);

        if (!dup) {
            return 1;
        }

        line = dup;
        err = at_tok_start(&line);
        if (err < 0) {
            free(dup);
            return 1;
        }
        err = at_tok_nextint(&line, &ind);
        if (err < 0) {
            free(dup);
            return 1;
        }
        err = at_tok_nextint(&line, &val);
        if (err < 0) {
            free(dup);
            return 1;
        }

        /* <ind> 102: E911 mode indication; <value> 0:disabled; 1:enabled */
        if (ind == 102 && val == 1) {
            s_unsol_msg_cache.service_state = 1;
            free(dup);
            return 0;
        } else if (ind == 102 && val == 0) {
            if (inemergency) {
                inemergency = 0;
                RIL_requestProxyTimedCallback(trigger_update_cache, NULL, &TIMEVAL_0,
                        DEFAULT_CHANNEL);
            }
            free(dup);
            return 0;
        } else if (ind == 12 && val == 0) {
            iccid_readable = 0;
            cta_no_uim = 1;
            RIL_onUnsolicitedResponse(RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED,
                    NULL, 0);
        } else if (ind == 12 && val == 1) {
            iccid_readable = 0;
            cta_no_uim = 0;
            RIL_onUnsolicitedResponse(RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED,
                    NULL, 0);
        } else if (ind == 13) {
            free(dup);
            return 0;
        /*iccid is unready when exec +VRESETUIM*/
        } else if (ind == 109 && val == 0) {
            if (iccid_readable)
                iccid_readable = 0;
        }
        /*iccid is readable after modem boot up or exec +VRESETUIM complete*/
        else if (ind == 109 && val == 1) {
            if (!iccid_readable) {
                iccid_readable = 1;
                if (UIM_STATUS_CARD_INSERTED == uimInsertedStatus) {
                    RIL_requestProxyTimedCallback(queryIccid, NULL, &TIMEVAL_0, NW_CHANNEL);
                }
            }
            RIL_onUnsolicitedResponse(RIL_LOCAL_C2K_UNSOL_CDMA_CARD_READY, NULL, 0);
        } else if (ind == 130) {
            NetworkMode = val;
            LOGD("CIEV:NetworkMode:%d", val);
        } else if (ind == 131) {
            prl = val;
            LOGD("PRL:%d", prl);
            RIL_onUnsolicitedResponse(RIL_UNSOL_CDMA_PRL_CHANGED, &prl,
                    sizeof(prl));
        } else if (ind == 107 && val == 1) {
            LOGD("rilNwUnsolicited: +CIEV:107,1");
            if (isDssNoResetSupport()) {
                simSwitchNotifySimInitDone();
            }
            RIL_onUnsolicitedResponse(RIL_LOCAL_C2K_UNSOL_CDMA_IMSI_READY, NULL, 0);
        } else if (ind == 110) {
           // world mode switch
           if (isWorldModeSwitch(val)) {
               // TDD/FDD mode switch
               // +CIEV:110,0 TDD/FDD mode switch start
               // +CIEV:110,1 TDD/FDD mode switch end
               LOGD("rilNwUnsolicited: +CIEV:110,%d", val);
               char worldmode[PROPERTY_VALUE_MAX] = {0};
               property_get("ro.vendor.mtk_md_world_mode_support", worldmode, "0");
               if (0 == strcmp(worldmode, "1")) {
                   if (val == 0 || val == 1) {
                       onWorldModeChanged(val);
                   }
               } else {
                   LOGD("rilNwUnsolicited: +CIEV:110,%d world mode not support", val);
               }
           }
        } else if (ind == 101) {
            // PHB/SMS ready URC, let PHB module handle
            LOGD("rilNwUnsolicited: Don't handle +CIEV:101");
            free(dup);
            return 0;
        }
        free(dup);
        return 1;
    }else if (strStartsWith(s, "^PREFMODE")) {
        char *dup = strdup(s);
        if (!dup) {
            return 1;
        }
        line = dup;
        err = at_tok_start(&line);
        if (err < 0) {
            free(dup);
            return 1;
        }
        err = at_tok_nextint(&line, &NetworkMode);
        if (err < 0) {
            LOGD("at_tok_nextstr error!");
            free(dup);
            return 1;
        }
        LOGD("NetworkMode:%d", NetworkMode);
        free(dup);
        return 1;
    }else if (strStartsWith(s, "+ENWKEXISTENCE:")) { //only for ECC Feature
        LOGD("ENWKEXISTENCE: %s", s);
        onNetworkExistence(s);
        return 1;
    } else if (strStartsWith(s, "^OTACMSG:")) {
       // URC ^OTACMSG:<status>
       // <status>: integer type
       //    1 programming started
       //    2 service programming lock unlocked
       //    3 NAM parameters downloaded successfully
       //    4 MDN downloaded successfully
       //    5 IMSI downloaded successfully
       //    6 PRL downloaded successfully
       //    7 commit successfully
       //    8 programming successfully
       //    9 programming unsuccessfully
       //    10 verify SPC failed
       //    11 a key exchanged
       //    12 SSD updated
       //    13 OTAPA started
       //    14 OTAPA stopped

       char *dup = strdup(s);
       int state = 0;
       int response[1] = {0};

       if (!dup) {
           return 1;
       }

       line = dup;
       err = at_tok_start(&line);
       if (err < 0) {
           free(dup);
           return 1;
       }
       err = at_tok_nextint(&line, &state);
       if (err < 0) {
           free(dup);
           return 1;
       }
       free(dup);
       response[0] = state;

       RIL_onUnsolicitedResponse(
               RIL_UNSOL_CDMA_OTA_PROVISION_STATUS, &response, sizeof(response));
       return 1;
    } else if (strStartsWith(s, "+EFCELL:")) {
        // +EFCELL:<n>,<system1>,<state>,<system2>,<state>
        // system: 0: 1xRTT, 1: EVDO
        // state:  0: not femtocell, 1: femtorcell
        int skip;
        int response[4];
        char *dup = strdup(s);
        if (!dup) {
            return 1;
        }
        line = dup;
        err = at_tok_start(&line);
        if (err < 0) {
            free(dup);
            return 1;
        }
        err = at_tok_nextint(&line, &skip);
        if (err < 0) {
            free(dup);
            return 1;
        }
        err = at_tok_nextint(&line, &response[0]);
        if (err < 0) {
            free(dup);
            return 1;
        }
        err = at_tok_nextint(&line, &response[1]);
        if (err < 0) {
            free(dup);
            return 1;
        }
        err = at_tok_nextint(&line, &response[2]);
        if (err < 0) {
            free(dup);
            return 1;
        }
        err = at_tok_nextint(&line, &response[3]);
        if (err < 0) {
            free(dup);
            return 1;
        }
        RIL_onUnsolicitedResponse(RIL_UNSOL_FEMTOCELL_INFO, &response, sizeof(response));
        free(dup);
        return 1;
    }  else if (strStartsWith(s,"+ECENGINFO:")) {
        LOGD("ECENGINFO: %s", s);
        onEngModeInfoUpdate(s);
        return 1;
    } else if (strStartsWith(s, "+ENWINFO:")) {
        onNetworkInfo(s);
        return 1;
    }
    /// M: [C2K][IR] Support SVLTE IR feature. @{
    else if (strStartsWith(s,"+REGPAUSEIND:")) {
        RIL_Mccmnc mcc_mnc;
        memset(&mcc_mnc, 0, sizeof(RIL_Mccmnc));
        char *dup = strdup(s);
        if (!dup) {
            return 1;
        }
        line = dup;
        err = at_tok_start(&line);
        if (err < 0) {
            free(dup);
            return 1;
        }
        LOGD("line = %s", line);

        err = at_tok_nextstr(&line, &mcc_mnc.mcc);
        if (err < 0) {
            free(dup);
            return 1;
        }
        err = at_tok_nextstr(&line, &mcc_mnc.mnc);
        if (err < 0) {
            free(dup);
            return 1;
        }
        RIL_onUnsolicitedResponse(RIL_UNSOL_CDMA_PLMN_CHANGED, &mcc_mnc,
                sizeof(RIL_Mccmnc));
        free(dup);
        return 1;
    }
    /// M: [C2K][IR] Support SVLTE IR feature. @}
    return 0;
}

///M: [Network][C2K] since HIDL is include in O version, should add MTK network existence state
/// into MTK URC(RIL_UNSOL_RESPONSE_CS_NETWORK_STATE_CHANGED) instead of use AOSP URC. @{
static void onNetworkExistence(const char* urc) {
    /* +ENWKEXISTENCE: <exist> */
    char* dup = NULL;
    char* line = NULL;
    int exsit = 0;
    int err;
    char* responseStr[6] = { NULL };
    int skip = 0;
    int commas_count = 0;
    char * p = NULL;
    int i = 0;
    for(i= 0; i < 6; i++) {
        asprintf(&responseStr[i], "-1");
    }
    dup = strdup(urc);
    if (dup == NULL) {
        LOGD("onNetworkExistence: memory is leak");
        return;
    }
    line = dup;
    err = at_tok_start(&line);
    if (err < 0) {
        free(dup);
        return;
    }
    for (p = line; *p != '\0'; p++) {
        if (*p == ',') {
            commas_count++;
        }
    }
    switch (commas_count) {
        case 0: // URC: +ENWKEXSITENCE   <exsit>
            err = at_tok_nextint(&line, &exsit);
            if (err < 0) {
                free(dup);
                return;
            }
            break;
        case 1:// response: +ENWKEXSITENCE <n><exsit>
            err = at_tok_nextint(&line, &skip);
            if (err < 0) {
                free(dup);
                return;
            }
            err = at_tok_nextint(&line, &exsit);
            if (err < 0) {
                free(dup);
                return;
            }
            break;
        default:
            break;
    }
    free(responseStr[5]);
    responseStr[5] = NULL;
    asprintf(&responseStr[5], "%d", exsit);
    s_unsol_msg_cache.network_exist = exsit;
    LOGD("onNetworkExistence, exsit: %d", exsit);
    //  Try to annouce this change to framework.
    RIL_onUnsolicitedResponse(
            RIL_UNSOL_RESPONSE_CS_NETWORK_STATE_CHANGED, responseStr, sizeof(responseStr));
    RIL_onUnsolicitedResponse(
            RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, NULL, 0);
    for (int n = 0; n < 6; n++) {
        if (responseStr[n] != NULL) {
            free(responseStr[n]);
            responseStr[n] = NULL;
        }
    }
    free(dup);
}
/// @}

void requestSignalStrength(void *data, size_t datalen, RIL_Token t)
{
    UNUSED(data);
    UNUSED(datalen);
    struct timeval TIMEVAL_SIGNAL_REPOLL = {4, 0};   /* 4 second */
    static int no_signal_times = 0;
    if ((s_unsol_msg_cache.signal_strength.CDMA_SignalStrength.dbm > 100)
            && (RADIO_STATE_ON == getRadioState() && RUIM_READY == getRUIMState())) {
        if(no_signal_times == 0){
            LOGD("no signal first time, return fake value");
            s_unsol_msg_cache.signal_strength.CDMA_SignalStrength.dbm = last_cdma_dbm;
        } else {
            LOGD("no signal %d time, return true value", no_signal_times);
            TIMEVAL_SIGNAL_REPOLL.tv_sec = 4*no_signal_times;
        }
        if(!repollinqueue) {
            LOGD("before registrater repollNetworkState!!!!");
            RIL_requestProxyTimedCallback(repollNetworkState, NULL, &TIMEVAL_SIGNAL_REPOLL,
                    DEFAULT_CHANNEL);
            repollinqueue = 1;
            no_signal_times++;
        } else {
            LOGD("There is a repoll signal action in queue");
            s_unsol_msg_cache.signal_strength.CDMA_SignalStrength.dbm = last_cdma_dbm;
        }
    }  else {
        no_signal_times = 0;
    }
    last_cdma_dbm = s_unsol_msg_cache.signal_strength.CDMA_SignalStrength.dbm;
    RIL_onRequestComplete(t, RIL_E_SUCCESS,
        &(s_unsol_msg_cache.signal_strength),
        sizeof(RIL_SignalStrength_v6));
}

void requestRegistrationState(int request, void *data,
                                        size_t datalen, RIL_Token t)
{
    UNUSED(data);
    UNUSED(datalen);
    RIL_Registration_state response;
    int err = 0;
    ATResponse *p_response = NULL;
    char *line = NULL;
    int skip = 0;
    struct timeval TIMEVAL_NETWORK_REPOLL = {4, 0};   /* 4 second */
    memset(&response, 0, sizeof(RIL_Registration_state));

#if 0
    /*Get System ID and Network ID*/
    err = at_send_command_singleline ("AT+CSNID?", "+CSNID:", &p_response, NW_CHANNEL_CTX);
    if((err != 0) || p_response == NULL || (p_response->success == 0))
    {
        goto error;
    }

    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err < 0) goto error;
    err = at_tok_nextstr(&line, &response.system_id);
    if (err < 0) goto error;
    err = at_tok_nextstr(&line, &response.network_id);
    if (err < 0) goto error;
#else
    /*Get System ID and Network ID*/
    err = at_send_command_singleline ("AT+VLOCINFO?", "+VLOCINFO:", &p_response, NW_CHANNEL_CTX);
    if((err != 0) || p_response == NULL || (p_response->success == 0))
    {
        goto error;
    }

    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err < 0) goto error;
    err = at_tok_nextint(&line, &skip);
    if (err < 0) goto error;
    err = at_tok_nextint(&line, &skip);
    if (err < 0) goto error;
    err = at_tok_nextint(&line, &skip);
    if (err < 0) goto error;
    err = at_tok_nextstr(&line, &response.system_id);
    if (err < 0) {
        goto error;
    } else {
        property_set("vendor.cdma.operator.sid", response.system_id);
    }
    err = at_tok_nextstr(&line, &response.network_id);
    if (err < 0) goto error;
    err = at_tok_nextstr(&line, &response.basestation_id);
    if (err < 0) goto error;
    err = at_tok_nextstr(&line, &response.basestation_latitude);
    if (err < 0) goto error;
    err = at_tok_nextstr(&line, &response.basestation_longitude);
    if (err < 0) goto error;
#endif
    LOGD("%s: s_unsol_msg_cache.service_state is %d, s_unsol_msg_cache.sysinfo_roam_status is %d"
        ,__func__, s_unsol_msg_cache.service_state, s_unsol_msg_cache.sysinfo_roam_status);
    if((1 == s_unsol_msg_cache.sysinfo_roam_status) && (1 == s_unsol_msg_cache.service_state)) {
        //LOGD("in roaming state");
        s_unsol_msg_cache.service_state = 5;
    } else if ((0 == s_unsol_msg_cache.sysinfo_roam_status) && (5 == s_unsol_msg_cache.service_state)) {
        //LOGD("in roaming off state");
        s_unsol_msg_cache.service_state = 1;
    }
    if((1 == s_unsol_msg_cache.sysinfo_roam_status) && (1 == s_unsol_msg_cache.x1_service_state)) {
        //LOGD("1x in roaming state");
        s_unsol_msg_cache.x1_service_state = 5;
    } else if ((0 == s_unsol_msg_cache.sysinfo_roam_status) && (5 == s_unsol_msg_cache.x1_service_state)) {
        //LOGD("1x in roaming off state");
        s_unsol_msg_cache.x1_service_state = 1;
    }
    if (s_unsol_msg_cache.radio_technology == 0
            && (RADIO_STATE_ON == getRadioState() && RUIM_READY == getRUIMState())) {
        if(no_service_times == 0){
            LOGD("no service first time, return fake value");
            asprintf(&(response.radio_technology), "%d", last_radio_technology);
            asprintf(&(response.register_state), "%d", last_service_state);

            no_service_times++;
            RIL_requestProxyTimedCallback(repollNetworkState, NULL, &TIMEVAL_NETWORK_REPOLL,
                    DEFAULT_CHANNEL);
            repollinqueue = 1;
        } else {
            if(!repollinqueue){
                LOGD("no service %d time, return true value", no_service_times);
                TIMEVAL_NETWORK_REPOLL.tv_sec = 4*no_service_times;
                asprintf(&(response.radio_technology), "%d", s_unsol_msg_cache.radio_technology);
                asprintf(&(response.register_state), "%d", s_unsol_msg_cache.service_state);
                last_service_state = s_unsol_msg_cache.service_state;
                last_radio_technology = s_unsol_msg_cache.radio_technology;
                LOGD("before registrater repollNetworkState!!!!");
                RIL_requestProxyTimedCallback(repollNetworkState, NULL, &TIMEVAL_NETWORK_REPOLL,
                        DEFAULT_CHANNEL);
                repollinqueue = 1;
                no_service_times++;
            } else {
                LOGD("There is a repoll action in queue, last_service_state:%d, last_radio_technology:%d",
                        last_service_state, s_unsol_msg_cache.service_state);
                last_service_state = s_unsol_msg_cache.service_state;
                last_radio_technology = s_unsol_msg_cache.radio_technology;
                asprintf(&(response.radio_technology), "%d", last_radio_technology);
                asprintf(&(response.register_state), "%d", last_service_state);
            }
        }

    } else {
        asprintf(&(response.radio_technology), "%d", s_unsol_msg_cache.radio_technology);
        asprintf(&(response.register_state), "%d", s_unsol_msg_cache.service_state);
        last_service_state = s_unsol_msg_cache.service_state;
        last_radio_technology = s_unsol_msg_cache.radio_technology;
        no_service_times = 0;
    }

    //fix HANDROID#2225, if unlock pin-code more than 3 time,we should set service_state to 0
    if(unlock_pin_outtimes == 1) {
        s_unsol_msg_cache.service_state = 0;
        last_service_state = s_unsol_msg_cache.service_state;
    }

    if (isCdmaLteDcSupport()) {
        if (request == RIL_REQUEST_DATA_REGISTRATION_STATE) {
            if (ps_state == 1 && s_unsol_msg_cache.sysinfo_roam_status == 1) {
                ps_state = 5;
            } else if (ps_state == 5 && s_unsol_msg_cache.sysinfo_roam_status == 0) {
                ps_state = 1;
            }
            asprintf(&(response.register_state), "%d", ps_state);
            asprintf(&(response.radio_technology), "%d", ps_state == 0 ? 0 : s_unsol_msg_cache.radio_technology);
        } else {
            asprintf(&(response.register_state), "%d", s_unsol_msg_cache.x1_service_state);
            asprintf(&(response.radio_technology), "%d",
                    (s_unsol_msg_cache.x1_service_state == 1
                            || s_unsol_msg_cache.x1_service_state == 5) ? 6 : 0);
        }
    }

    asprintf(&(response.roaming_indicator), "%d", s_unsol_msg_cache.roaming_indicator);
    asprintf(&(response.network_exist), "%d", s_unsol_msg_cache.network_exist);

    response.lac = REGISTRATION_DEFAULT_VALUE;
    response.cid = REGISTRATION_DEFAULT_VALUE;
    response.concurrent_service = REGISTRATION_DEFAULT_VALUE;
    asprintf(&(response.prl_state), "%d", s_unsol_msg_cache.prl_state);
    asprintf(&(response.prl_roaming_indicator), "%d", s_unsol_msg_cache.prl_roaming_indicator);
    response.deny_reason = REGISTRATION_DEFAULT_VALUE;
    RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(RIL_Registration_state));
    at_response_free(p_response);
    free(response.register_state);
    free(response.radio_technology);
    free(response.roaming_indicator);

    free(response.prl_state);
    free(response.prl_roaming_indicator);
    free(response.network_exist);
    return;
error:
    LOGE("requestRegistrationState must never return an error when radio is on");
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

bool isValidMCC(char* mcc) {
   if (mcc != NULL && strcmp(mcc, "000") != 0
       && strcmp(mcc, "N/A") != 0
       && strcmp(mcc, "") != 0) {
       return true;
   }
   return false;
}

void requestOperator(void *data, size_t datalen, RIL_Token t)
{
    UNUSED(data);
    UNUSED(datalen);
    RIL_Operationinfo response;
    ATResponse *p_response = NULL;
    char *line = NULL, *out = NULL;
    int commas_count = 0;
    char *p = NULL;
    int skip = 0;
    memset(&response, 0, sizeof(response));

    // LOGD("%s: uimInsertedStatus is %d, sState is %d",__FUNCTION__,uimInsertedStatus, getRadioState());
    asprintf(&(response.longons), "%s", "");
    asprintf(&(response.shortons), "%s", "");

    if ((UIM_STATUS_NO_CARD_INSERTED == uimInsertedStatus)
            || (RADIO_STATE_ON != getRadioState())
            || (RUIM_READY != getRUIMState())) {
        asprintf(&(response.mccmnc), "%s", "");
    } else {
        asprintf(&(response.mccmnc), "%s%s", s_unsol_msg_cache.mcc, s_unsol_msg_cache.mnc);
        LOGD("requestOperator, mccmnc is %s", response.mccmnc);
        if (!isValidMCC(s_unsol_msg_cache.mcc)) {
            int err = at_send_command_singleline("AT+VMCCMNC?", "+VMCCMNC:", &p_response, NW_CHANNEL_CTX);
            if ((err < 0) || p_response == NULL || (p_response->success == 0)) {
                LOGD("requestOperator, error when send at command is %d", err);
                goto error;
            }
            char *dup = strdup(p_response->p_intermediates->line);
            line = dup;
            err = at_tok_start(&line);
            if (err < 0) {
                free(dup);
                goto error;
            }
            for (p = line; *p != '\0'; p++) {
                if (*p == ',')
                    commas_count++;
            }
            switch (commas_count) {
            case 1: /* +VMCCMNC:<MCC>,<MNC> */
                err = at_tok_nextstr(&line, &out);
                if (err < 0) {
                   free(dup);
                   goto error;
                }
                strncpy(s_unsol_msg_cache.mcc, out, 7);
                err = at_tok_nextstr(&line, &out);
                if (err < 0) {
                    free(dup);
                    goto error;
                }
                strncpy(s_unsol_msg_cache.mnc, out, 7);
                break;
            case 2: /* +VMCCMNC:<MccMnc>,<MCC>,<MNC> */
                err = at_tok_nextint(&line, &skip);
                if (err < 0) {
                    free(dup);
                    goto error;
                }
                err = at_tok_nextstr(&line, &out);
                if (err < 0) {
                    free(dup);
                    goto error;
                }
                strncpy(s_unsol_msg_cache.mcc, out, 7);
                err = at_tok_nextstr(&line, &out);
                if (err < 0) {
                    free(dup);
                    goto error;
                }
                strncpy(s_unsol_msg_cache.mnc, out, 7);
                break;
            default:
                LOGD("requestOperator, unsupport VMCCMNC");
                break;
            }

            // Modem might response invalid mcc/mnc ex:"N/A", mcc convert to "000" and nmc convert to "00"
            if (strcmp(s_unsol_msg_cache.mcc, "N/A") == 0 || strcmp(s_unsol_msg_cache.mnc, "N/A") == 0) {
                // LOGD("requestOperator, modem response invalid value:N/A");
                s_unsol_msg_cache.mcc[0] = '\0';
                s_unsol_msg_cache.mnc[0] = '\0';
            }

            asprintf(&(response.mccmnc), "%s%s", s_unsol_msg_cache.mcc, s_unsol_msg_cache.mnc);
            LOGD("requestOperator, mccmnc is %s", response.mccmnc);

            /// Notify CC to refresh emergency list @{
            onMccMncIndication(response.mccmnc);
            /// @}
        }
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(RIL_Operationinfo));
    free(response.mccmnc);
    free(response.longons);
    free(response.shortons);
    at_response_free(p_response);
    return;

error:
    free(response.mccmnc);
    free(response.longons);
    free(response.shortons);
    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

/* repoll the registration state, while found no service */
static void repollNetworkState(void *param)
{
    UNUSED(param);
    LOGD("In %s, submit unsol network state change", __func__);
    repollinqueue = 0;
    trigger_update_cache();
    RIL_onUnsolicitedResponse(RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, \
            NULL, 0);
}

static void requestQueryNetworkSelectionMode(void *data, size_t datalen, RIL_Token t)
{
    UNUSED(data);
    UNUSED(datalen);
    int err = 0;
    ATResponse *p_response = NULL;
    int response = 0;
    char *line = NULL;

    err = at_send_command_singleline("AT+COPS?", "+COPS:", &p_response, NW_CHANNEL_CTX);

    if (err < 0 || p_response == NULL || p_response->success == 0) {
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);

    if (err < 0) {
        goto error;
    }

    err = at_tok_nextint(&line, &response);

    if (err < 0) {
        goto error;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(int));
    at_response_free(p_response);
    return;
error:
    at_response_free(p_response);
    LOGE("requestQueryNetworkSelectionMode must never return error when radio is on");
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static void requestSetPreferredNetworkType(void *data, size_t datalen, RIL_Token t)
{
    UNUSED(datalen);
    int err = 0;
    int mode = ((int*)data)[0];
    int value = 0;
    char *cmd = NULL;

    int cdmaslotid = getActiveSvlteModeSlotId();
    // get main capability id
    char tempstr[PROPERTY_VALUE_MAX];
    int currMajorSim = 1;

    memset(tempstr, 0, sizeof(tempstr));
    property_get("persist.vendor.radio.simswitch", tempstr, "1");
    currMajorSim = atoi(tempstr);
    LOGD("requestSetPreferredNetworkType cdmaslotid = %d, currMajorSim = %d",
            cdmaslotid, currMajorSim);

    if (cdmaslotid == currMajorSim) {
        switch (mode) {
            case NETWORK_MODE_CDMA_NO_EVDO:
                value = 2;
                break;
            case NETWORK_MODE_EVDO_NO_CDMA:
                value = 4;
                break;
            case NETWORK_MODE_HYBRID:
            case NETWORK_MODE_GLOBAL:
                value = 8;
                break;
            default:
                goto error;
        }
    } else {
        // special handle for limitation of MD3
        value = 2;
    }
    asprintf(&cmd, "AT^PREFMODE=%d", value);
    err = at_send_command(cmd, NULL, NW_CHANNEL_CTX);
    free(cmd);

    if (err != 0) goto error;

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);

    return;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}


static void requestGetPreferredNetworkType(void *data, size_t datalen, RIL_Token t)
{
    UNUSED(data);
    UNUSED(datalen);
    int err = 0;
    ATResponse *p_response = NULL;
    char *line = NULL;
    int mode = 0;
    int response = 0;
    err = at_send_command_singleline ("AT^PREFMODE?", "^PREFMODE:", &p_response, NW_CHANNEL_CTX);
    if ((err < 0) || p_response == NULL || (p_response->success == 0))
    {
        goto error;
    }

    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err < 0)
    {
        goto error;
    }
    err = at_tok_nextint(&line, &mode);
    if (err < 0)
    {
        goto error;
    }
    LOGD("mode = %d",mode);
    switch(mode){
        case 2:
            LOGD("CDMA mode");
            response = NETWORK_MODE_CDMA_NO_EVDO;
            break;
        case 4:
            LOGD("HDR mode");
            response = NETWORK_MODE_EVDO_NO_CDMA;
            break;
        case 8:
            LOGD("CDMA/HDR HYBRID mode");
            response = NETWORK_MODE_HYBRID;
            break;
    }
    RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(int));
    at_response_free(p_response);
    return;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

void updatePrlInfo(int system, int mode){
    LOGD("updatePrlInfo() system = %d, mode = %d", system, mode);
    if ((system == 0) && (mode < 255)) {
        s_unsol_msg_cache.prl_state = mode;
    } else {
        LOGD("updatePrlInfo() unsupport system = %d", system);
    }
}
void resetUnsolMsgCache()
{
    s_md3_vpon = 0;
    LOGD("%s: set s_md3_vpon = 0 and service state = %d.", __FUNCTION__,
                 s_unsol_msg_cache.service_state);
    // if service_state is still in service and then update to searching after cpof @{
    if (s_unsol_msg_cache.service_state == 1 || s_unsol_msg_cache.x1_service_state == 1) {
        s_unsol_msg_cache.service_state = 2;
        s_unsol_msg_cache.x1_service_state = 2;
        s_unsol_msg_cache.evdo_service_state = 2;
        s_unsol_msg_cache.radio_technology = 0;
        ps_state = 0;
        last_radio_technology = 0;
        last_service_state = 0;
    }
}

///M: [C2K][SIM] @{
static void queryIccid(void *param) {
    UNUSED(param);
    int err;
    char *iccid_line = NULL;
    char *iccidStr = NULL;
    ATResponse *p_response = NULL;

    err = at_send_command_singleline("AT+VICCID?", "+VICCID:",
            &p_response, getChannelCtxbyProxy());
    if ((err < 0) || p_response == NULL || (p_response->success == 0)) {
        LOGD("failed to get iccid from uim card!!!");
    } else {
        iccid_line = p_response->p_intermediates->line;

        err = at_tok_start(&iccid_line);
        if (err < 0) {
            LOGD("failed to at_tok_start!!");
            goto done;
        }

        err = at_tok_nextstr(&iccid_line, &iccidStr);
        if (err < 0) {
            LOGD("failed to at_tok_nextstr!!");
            goto done;
        }

        //add for hotplug, if GSM card, needn't to set iccid.
        if (cardtype == SIM_CARD) {
            LOGD("GSM card, abandon to set ICCID");
        } else {
            setIccidProperty(SET_VALID_ICCID, iccidStr);
            RIL_onUnsolicitedResponse(RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, NULL, 0);
        }
    }

    done: at_response_free(p_response);
    p_response = NULL;
}
/// @}

static void requestSetRoamingPreference(void *data, size_t datalen, RIL_Token t) {
    UNUSED(datalen);
    ATResponse *p_response = NULL;
    int requrstRoamingPreferencesType = -1;
    int roamingPreferencesType = -1;
    int err;
    char *cmd = NULL;
    RIL_Errno ril_errno = RIL_E_REQUEST_NOT_SUPPORTED;
    requrstRoamingPreferencesType = ((int *)data)[0];

    // AT$ROAM=<type>
    // <type>=0: set the device to Sprint only mode
    // <type>=1: set the device to automatic mode

    if (requrstRoamingPreferencesType == 0){
        // for Home Networks only
        roamingPreferencesType = 0;
    } else if (requrstRoamingPreferencesType == 2){
        // for Roaming on Any Network
        roamingPreferencesType = 1;
    } else {
        LOGE("[%s]Not support RoamingPreferencesType=%d", __func__, requrstRoamingPreferencesType);
    }

    if (roamingPreferencesType >= 0) {
        err = asprintf(&cmd,  "AT$ROAM=%d", roamingPreferencesType);
        if (err >= 0) {
            err = at_send_command(cmd, &p_response, getRILChannelCtxFromToken(t));
            if(err >= 0 && p_response != NULL && p_response->success != 0) {
                ril_errno = RIL_E_SUCCESS;
            }
        }
        free(cmd);
    }
    RIL_onRequestComplete(t, ril_errno, NULL, 0);
    at_response_free(p_response);
}

static void requestQueryRoamingPreference(void *data, size_t datalen, RIL_Token t) {
    UNUSED(data);
    UNUSED(datalen);
    ATResponse *p_response = NULL;
    int err;
    char *line = NULL;
    RIL_Errno ril_errno = RIL_E_REQUEST_NOT_SUPPORTED;

    int response[1] = { 0 };
    int *roamingPreferencesType = &response[0];

    *roamingPreferencesType = -1;

    // AT$ROAM=<type>
    // <type>=0: set the device to Sprint only mode
    // <type>=1: set the device to automatic mode

    err = at_send_command_singleline("AT$ROAM?", "$ROAM:", &p_response,
            getRILChannelCtxFromToken(t));

    if (err >= 0 && p_response != NULL && p_response->success != 0) {
        line = p_response->p_intermediates->line;
        err = at_tok_start(&line);
        if (err < 0) goto error;

        // <type>
        err = at_tok_nextint(&line, roamingPreferencesType);
        if (err < 0) goto error;

        if (*roamingPreferencesType == 0) {
            // for Home Networks only
            *roamingPreferencesType = 0;
            ril_errno = RIL_E_SUCCESS;
        } else if (*roamingPreferencesType == 1) {
            // for Roaming on Any Network
            *roamingPreferencesType = 2;
            ril_errno = RIL_E_SUCCESS;
        } else {
            LOGE("ERROR occurs <relative_phase> form antenna info request");
            goto error;
        }
    }
    RIL_onRequestComplete(t, ril_errno, response, sizeof(response));
    at_response_free(p_response);
    return;
error:
    RIL_onRequestComplete(t, ril_errno, response, sizeof(response));
    at_response_free(p_response);
}

int parseEngModeUrc(char *urc, char **cmd, char **info)
{
    char *colon = NULL;

    if ((NULL == urc) || (NULL == cmd) || (NULL == info))
    {
        return -1;
    }

    *cmd = NULL;
    *info = NULL;

    if ('\0' == urc[0])
    {
        return -1;
    }

    colon = strchr(urc, ':');
    if (NULL == colon)
    {
        return -1;
    }

    if (colon == urc)
    {
        return -1;
    }

    *colon = '\0';
    if ('\0' != *(colon + 1))
    {
        *info = colon + 1;
    }

    *cmd = urc;

    return 0;
}

static void onEngModeInfoUpdate(const char* urc)
{
    RIL_Eng_Mode_Info engModeInfo;
    char *tmp_urc = NULL;
    char *cmd = NULL;
    char *info = NULL;

    if (NULL == urc)
    {
        LOGE("ECENGINFO input urc is NULL\r\n");
        return;
    }

    LOGD("ECENGINFO input urc = \"%s\"\r\n", urc);

    memset(&engModeInfo, 0, sizeof(engModeInfo));

    tmp_urc = strdup(urc);
    if (NULL == tmp_urc)
    {
        LOGE("+ECENGINFO: Unable to allocate memory");
        return;
    }

    if (0 != parseEngModeUrc(tmp_urc, &cmd, &info))
    {
        /* processing error here, free tmp_urc first if return */
        LOGE("ECENGINFO invalid urc format\r\n");
        free(tmp_urc);
        return;
    }
    else
    {
        LOGD("ECENGINFO cmd = \"%s\", info = \"%s\"\r\n", cmd, info);
        engModeInfo.at_command = cmd;
        engModeInfo.info = info;
        RIL_onUnsolicitedResponse(RIL_LOCAL_C2K_UNSOL_ENG_MODE_NETWORK_INFO, &engModeInfo,
                sizeof(engModeInfo));
        free(tmp_urc);
    }
}

static void onNetworkInfo(const char* urc) {
    /* +ENWINFO: <type>,<nw_info> */
    int err;
    int type;
    char *responseStr[2];
    char *dup = NULL;
    char *line = NULL;

    dup = strdup(urc);
    line = dup;
    err = at_tok_start(&line);
    if (err < 0) {
        free(dup);
        return;
    }

    err = at_tok_nextint(&line, &type);
    if (err < 0) {
        free(dup);
        return;
    }
    asprintf(&responseStr[0], "%d", type);

    // get raw data of structure of NW info
    err = at_tok_nextstr(&line, &(responseStr[1]));
    if (err < 0) {
        free(dup);
        return;
    }

    RIL_onUnsolicitedResponse(RIL_UNSOL_NETWORK_INFO, responseStr,
            sizeof(responseStr));
    free(responseStr[0]);
    free(dup);
}

/// M: for get switching state. 0 is not switching. 1 is switching @{
int getRatSwitching() {
    char switching[PROPERTY_VALUE_MAX] = { 0 };
    int switchingVal = 0;

    property_get("vendor.ril.rat.switching", switching, "0");
    switchingVal = atoi(switching);
    LOGD("getRatSwitching, switchingVal is %d", switchingVal);
    return switchingVal;
}
/// M: for get switching state. 0 is not switching. 1 is switching @}

/// M: [C2K][IR] Support CT 3g dual Mode card IR feature. @{

static void requestResumeCdmaRegistration(void *data, size_t datalen, RIL_Token t)
{
    UNUSED(data);
    UNUSED(datalen);
    int err = 0;
    ATResponse *p_response = NULL;

    err = at_send_command("AT+REGRESUME", &p_response, NW_CHANNEL_CTX);
    if ((err < 0) || p_response == NULL || (p_response->success == 0))
    {
        LOGE("%s: send at error is %d", __FUNCTION__, err);
        goto error;
    }

    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;

error:
    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static void requestSetCdmaRegSuspendEnabled(void *data, size_t datalen, RIL_Token t)
{
    UNUSED(datalen);
    int value;
    int err = 0;
    ATResponse *p_response = NULL;

    value = ((int *)data)[0];

    LOGD("%s: value is %d", __FUNCTION__, value);

    if(1 == value)
    {
        if(!isCdmaRegSuspendEnabled())
        {
            err = at_send_command("AT+VREGCTR=1", &p_response, NW_CHANNEL_CTX);
            if ((err < 0) || p_response == NULL || (p_response->success == 0))
            {
                LOGE("%s: send at error", __FUNCTION__);
                goto error;
            }
        }
    }
    else if (0 == value)
    {
        if(1 == isCdmaRegSuspendEnabled())
        {
            err = at_send_command("AT+VREGCTR=0", &p_response, NW_CHANNEL_CTX);
            if ((err < 0) || p_response == NULL || (p_response->success == 0))
            {
                LOGE("%s: send at error", __FUNCTION__);
                goto error;
            }
        }
    }

    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;

error:
    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

/*
+VREGCTR=<enable>   OK
+VREGCTR?   + VREGCTR: < enable >
+VREGCTR=?  +VREGCTR: (0 - 1)

<enable>:
0   disable register pause function
1   enable register pause function
*/
static int isCdmaRegSuspendEnabled(void)
{
    int err = 0;
    ATResponse *p_response = NULL;
    char *line = NULL;
    int enable = -1;

    err = at_send_command_singleline("AT+VREGCTR?", "+VREGCTR:", &p_response,
            getChannelCtxbyProxy());
    if ((err < 0) || p_response == NULL || (p_response->success == 0))
    {
        LOGD("%s: failed to send +VREGCTR?, err = %d", __FUNCTION__, err);
        goto exit;
    }

    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err < 0)
    {
        LOGD("%s: failed to at_tok_start", __FUNCTION__);
        goto exit;
    }
    err = at_tok_nextint(&line, &enable);
    if (err < 0)
    {
        LOGD("%s: failed to at_tok_nextint", __FUNCTION__);
        goto exit;
    }
    LOGD("%s: enable = %d", __FUNCTION__, enable);

exit:
    at_response_free(p_response);
    return enable;
}

void requestSetSvlteRatMode(void * data, size_t datalen, RIL_Token t) {
    UNUSED(datalen);
    char* cmd;
    ATResponse *p_response = NULL;
    int err = 0;

    int preRoaming = ((int *) data)[3];
    int roaming = ((int *) data)[4];

    LOGD("c2k requestSetSvlteRatMode(), preRoaming =%d, roaming=%d.",preRoaming, roaming);

    // switch UTK/STK mode.
    int mode = (roaming == ROAMING_MODE_HOME || roaming == ROAMING_MODE_JPKR_CDMA) ? 1 : 0;
    err = switchStkUtkMode(mode, t);

    if(err >= 0) {
        if(preRoaming != roaming || roaming == 1) {
            //radio off
            err = asprintf(&cmd, "AT+CPOF");
            LOGD("c2k requestSetSvlteRatMode(), send command %s.", cmd);
            err = at_send_command(cmd, &p_response, NW_CHANNEL_CTX);
            setRadioState(RADIO_STATE_OFF);
        }
    }

    LOGD("c2k requestSetSvlteRatMode(), err=%d.", err);
    if (err < 0) {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
}
/// M: [C2K][IR] Support CT 3g dual Mode card IR feature. @}

/// M: [Network][C2K] Sprint roaming control @{
void setRoamingConfig(void* data, size_t datalen, RIL_Token t) {
    /*
        data[0] : phone id (0,1,2,3,...)
        data[1] : international_voice_text_roaming (0,1)
        data[2] : international_data_roaming (0,1)
        data[3] : domestic_voice_text_roaming (0,1)
        data[4] : domestic_data_roaming (0,1)
        data[5] : domestic_LTE_data_roaming (0,1)

    +EROAMBAR:<protocol_index>, (not ready now)
        <BAR_Dom_Voice_Roaming_Enabled>,
        <BAR_Dom_Data_Roaming_Enabled>,
        <Bar_Int_Voice_Roaming_Enabled>,
        <Bar_Int_Data_Roaming_Enabled>,
        <Bar_LTE_Data_Roaming_Enabled>

        NOTE: The order is different.
    */
    UNUSED(datalen);
    char *cmd = NULL;
    int err = 0;
    int (*roamingConfig)[6] = (int(*)[6])data;
    /* //DS
    asprintf(&cmd, "AT+EROAMBAR=%d,%d,%d,%d,%d,%d", (*roamingConfig)[0]
        , (*roamingConfig)[3]
        , (*roamingConfig)[4]
        , (*roamingConfig)[1]
        , (*roamingConfig)[2]
        , (*roamingConfig)[5]);
    */
    /* //SS */
    //rever the setting from enable(fwk) to bar(md)
    for (int i = 1; i < 6; i++) {
        (*roamingConfig)[i] = (*roamingConfig)[i] == 0 ? 1: 0;
    }

    asprintf(&cmd, "AT+EROAMBAR=%d,%d,%d,%d,%d"
        , (*roamingConfig)[3]  // BAR_Dom_Voice_Roaming_Enabled
        , (*roamingConfig)[4]  // BAR_Dom_Data_Roaming_Enabled
        , (*roamingConfig)[1]  // Bar_Int_Voice_Roaming_Enabled
        , (*roamingConfig)[2]  // Bar_Int_Data_Roaming_Enabled
        , (*roamingConfig)[5]);// Bar_LTE_Data_Roaming_Enabled
    LOGD("setRoamingConfig %s", cmd);
    err = at_send_command(cmd, NULL, NW_CHANNEL_CTX);
    if(err < 0) {
        free(cmd);
        goto error;
    }
    free(cmd);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

void getRoamingConfig(void* data, size_t datalen, RIL_Token t) {
    /*
    +EROAMBAR:<protocol_index>, (not ready now)
        <BAR_Dom_Voice_Roaming_Enabled>,
        <BAR_Dom_Data_Roaming_Enabled>,
        <Bar_Int_Voice_Roaming_Enabled>,
        <Bar_Int_Data_Roaming_Enabled>,
        <Bar_LTE_Data_Roaming_Enabled>
Expected Result:
    response[0]: phone id (0,1,2,3,...)
    response[1] : international_voice_text_roaming (0,1)
    response[2] : international_data_roaming (0,1)
    response[3] : domestic_voice_text_roaming (0,1)
    response[4] : domestic_data_roaming (0,1)
    response[5] : domestic_LTE_data_roaming (1)
    */
    UNUSED(data);
    UNUSED(datalen);
    int roamingConfig[6] = {1, 0, 1, 1, 1, 1}; //default value
    char *line = NULL;
    int err = 0;
    ATResponse *p_response = NULL;

    err = at_send_command_singleline("AT+EROAMBAR?", "+EROAMBAR:", &p_response, NW_CHANNEL_CTX);

    if (err < 0 || p_response == NULL || p_response->success == 0) {
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);

    if (err < 0) {
        goto error;
    }
    /* //DS
    err = at_tok_nextint(&line, &(roamingConfig[0]));
    if (err < 0) {
        goto error;
    }
    */
    roamingConfig[0] = 0;

    // <BAR_Dom_Voice_Roaming_Enabled>
    err = at_tok_nextint(&line, &(roamingConfig[3]));
    if (err < 0) {
        goto error;
    }

    // <BAR_Dom_Data_Roaming_Enabled>
    err = at_tok_nextint(&line, &(roamingConfig[4]));
    if (err < 0) {
        goto error;
    }

    // <Bar_Int_Voice_Roaming_Enabled>
    err = at_tok_nextint(&line, &(roamingConfig[1]));
    if (err < 0) {
        goto error;
    }

    // <Bar_Int_Data_Roaming_Enabled>
    err = at_tok_nextint(&line, &(roamingConfig[2]));
    if (err < 0) {
        goto error;
    }

    // <Bar_LTE_Data_Roaming_Enabled>
    err = at_tok_nextint(&line, &(roamingConfig[5]));
    if (err < 0) {
        goto error;
    }

    //rever the setting from enable(fwk) to bar(md)
    for (int i = 1; i < 6; i++) {
        roamingConfig[i] = roamingConfig[i] == 0 ? 1: 0;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, roamingConfig, sizeof(roamingConfig));
    at_response_free(p_response);
    return;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

/// M: [Network][C2K] for handle RIL_REQUEST_SET_UNSOLICITED_RESPONSE_FILTER. @{
void updateNetworkRegistrationStatus(RIL_Token t) {
    LOGD("updateNetworkRegistrationStatus");
    at_send_command("AT^SYSINFO", NULL, NW_CHANNEL_CTX);
    at_send_command("AT+VROM?", NULL, NW_CHANNEL_CTX);
    at_send_command("AT+ECGREG?", NULL, NW_CHANNEL_CTX);
    at_send_command("AT+CREG?", NULL, NW_CHANNEL_CTX);
    at_send_command("AT+VMCCMNC?", NULL, NW_CHANNEL_CTX);
    at_send_command("AT+VSER?", NULL, NW_CHANNEL_CTX);
    at_send_command("AT+EIPRL?", NULL, NW_CHANNEL_CTX);
    at_send_command("AT+EDEFROAM?", NULL, NW_CHANNEL_CTX);
    at_send_command("AT+EDOROM?", NULL, NW_CHANNEL_CTX);
    at_send_command("AT+ENWKEXISTENCE?", NULL, NW_CHANNEL_CTX);
    if (isFemtocellSupport()) {
        at_send_command("AT+EFCELL?", NULL, NW_CHANNEL_CTX);
    }
}

int setUnsolResponseFilterSignalStrength(bool enable, RIL_Token t) {
    int err;
    char *cmd = NULL;

    if (enable) {
        asprintf(&cmd, "AT+ARSI=1,%d", arsithrehd);
        err = at_send_command(cmd, NULL, NW_CHANNEL_CTX);
        if (err < 0) goto error;
        err = at_send_command("AT^HDRSINR=1", NULL, NW_CHANNEL_CTX);
        if (err < 0) {
            LOGD("setUnsolResponseFilterSignalStrength(), enable AT^HDRSINR fail");
        }
        // query signal strength
        at_send_command("AT+CSQ?", NULL, NW_CHANNEL_CTX);
        at_send_command("AT+HDRCSQ?", NULL, NW_CHANNEL_CTX);
        at_send_command("AT^HDRSINR?", NULL, NW_CHANNEL_CTX);
    } else {
        asprintf(&cmd, "AT+ARSI=0,%d", arsithrehd);
        err = at_send_command(cmd, NULL, NW_CHANNEL_CTX);
        if (err < 0) goto error;
        err = at_send_command("AT^HDRSINR=0", NULL, NW_CHANNEL_CTX);
        if (err < 0) {
            LOGD("setUnsolResponseFilterSignalStrength(), disable AT^HDRSINR fail");
        }
    }
    return 0;
error:
    LOGE("ERROR: setUnsolResponseFilterSignalStrength failed");
    return -1;
}

int setUnsolResponseFilterNetworkState(bool enable, RIL_Token t) {
    int err;

    if (enable) {
        LOGD("start unsolited network registration URC.");
        err = at_send_command("AT+VMCCMNC=1", NULL, NW_CHANNEL_CTX);
        if (err < 0) goto error;
        err = at_send_command("AT+CREG=1", NULL, NW_CHANNEL_CTX);
        if (err < 0) goto error;
        err = at_send_command("AT+VROM=1", NULL, NW_CHANNEL_CTX);
        if (err < 0) goto error;
        err = at_send_command("AT+EDEFROAM=1", NULL, NW_CHANNEL_CTX);
        if (err < 0) {
            LOGD("setUnsolResponseFilterNetworkState(), send AT+EDEFROAM fail");
        }
        err = at_send_command("AT+EDOROM=1", NULL, NW_CHANNEL_CTX);
        if (err < 0) {
            LOGD("setUnsolResponseFilterNetworkState(), send AT+EDOROM fail");
        }
        err = at_send_command("AT+ENWKEXISTENCE=1", NULL, NW_CHANNEL_CTX);
        if (err < 0) goto error;
        if (isFemtocellSupport()) {
            err = at_send_command("AT+EFCELL=1", NULL, NW_CHANNEL_CTX);
            if (err < 0) {
                LOGD("setUnsolResponseFilterNetworkState(), enable AT+EFCELL fail");
            }
        }
      updateNetworkRegistrationStatus(t);
    } else {
        LOGD("stop unsolted network registration URC.");
        err = at_send_command("AT+CREG=0", NULL, NW_CHANNEL_CTX);
        if (err < 0) goto error;
        err = at_send_command("AT+VMCCMNC=0", NULL, NW_CHANNEL_CTX);
        if (err < 0) goto error;
        if (isFemtocellSupport()) {
            err = at_send_command("AT+EFCELL=0", NULL, NW_CHANNEL_CTX);
            if (err < 0) {
                LOGD("setUnsolResponseFilterNetworkState(), disable AT+EFCELL fail");
            }
        }
        err = at_send_command("AT+VROM=0", NULL, NW_CHANNEL_CTX);
        if (err < 0) goto error;
        err = at_send_command("AT+EDEFROAM=0", NULL, NW_CHANNEL_CTX);
        if (err < 0) {
            LOGD("setUnsolResponseFilterNetworkState(), send AT+EDEFROAM fail");
        }
        err = at_send_command("AT+EDOROM=0", NULL, NW_CHANNEL_CTX);
        if (err < 0) {
            LOGD("setUnsolResponseFilterNetworkState(), send AT+EDOROM fail");
        }
        err = at_send_command("AT+ENWKEXISTENCE=0", NULL, NW_CHANNEL_CTX);
        if (err < 0) goto error;
    }
    return 0;
error:
    LOGE("ERROR: setUnsolResponseFilterNetworkState failed");
    return -1;
}

void requestGetCellInfoList(void * data, size_t datalen, RIL_Token t) {
    UNUSED(data);
    UNUSED(datalen);
    UNUSED(t);
    ATResponse *p_response = NULL;
    RIL_CellInfo_v12 *response = NULL;

    if (!isSvlteSupport()) {
        goto error;
    }

    if (s_unsol_msg_cache.x1_service_state == 5 || s_unsol_msg_cache.x1_service_state == 1) {
        response = (RIL_CellInfo_v12 *) malloc(1 * sizeof(RIL_CellInfo_v12));
        if (response == NULL) {
            LOGE("requestGetCellInfoList malloc fail");
            goto error;
        }
        memset(response, 0, 1 * sizeof(RIL_CellInfo_v12));
        /*Get System ID and Network ID*/
        response->registered = 1;
        response->connectionStatus = PRIMARY_SERVING;
        response->cellInfoType = RIL_CELL_INFO_TYPE_CDMA;

        int skip = 0;
        int err = 0;
        char *line = NULL;
        err = at_send_command_singleline("AT+VLOCINFO?", "+VLOCINFO:", &p_response, NW_CHANNEL_CTX);
        if ((err != 0) || p_response == NULL || (p_response->success == 0)) {
            goto error;
        }

        line = p_response->p_intermediates->line;
        err = at_tok_start(&line);
        if (err < 0) goto error;
        err = at_tok_nextint(&line, &skip);
        if (err < 0) goto error;
        err = at_tok_nextint(&line, &skip);
        if (err < 0) goto error;
        if (skip == 111) { // Here, if MCC is default value, ignore this CELL
            goto error;
        }
        err = at_tok_nextint(&line, &skip);
        if (err < 0) goto error;
        err = at_tok_nextint(&line, &response->CellInfo.cdma.cellIdentityCdma.systemId);
        if (err < 0) goto error;
        err = at_tok_nextint(&line, &response->CellInfo.cdma.cellIdentityCdma.networkId);
        if (err < 0) goto error;
        err = at_tok_nextint(&line, &response->CellInfo.cdma.cellIdentityCdma.basestationId);
        if (err < 0) goto error;
        err = at_tok_nextint(&line, &response->CellInfo.cdma.cellIdentityCdma.latitude);
        if (err < 0) goto error;
        err = at_tok_nextint(&line, &response->CellInfo.cdma.cellIdentityCdma.longitude);
        if (err < 0) goto error;
        response->CellInfo.cdma.cellIdentityCdma.operName.long_name = NULL;
        response->CellInfo.cdma.cellIdentityCdma.operName.short_name = NULL;
        response->CellInfo.cdma.signalStrengthCdma.dbm = s_unsol_msg_cache.signal_strength.CDMA_SignalStrength.dbm;
        response->CellInfo.cdma.signalStrengthCdma.ecio = s_unsol_msg_cache.signal_strength.CDMA_SignalStrength.ecio;
        response->CellInfo.cdma.signalStrengthEvdo.dbm = s_unsol_msg_cache.signal_strength.EVDO_SignalStrength.dbm;
        response->CellInfo.cdma.signalStrengthEvdo.ecio = s_unsol_msg_cache.signal_strength.EVDO_SignalStrength.ecio;
        response->CellInfo.cdma.signalStrengthEvdo.signalNoiseRatio =
               s_unsol_msg_cache.signal_strength.EVDO_SignalStrength.signalNoiseRatio;
        // Thirdly, we report the URC to AP level
        RIL_onRequestComplete(t, RIL_E_SUCCESS, response, 1 * sizeof(RIL_CellInfo_v12));
        at_response_free(p_response);
        p_response = NULL;
        free(response);
        response = NULL;
        return;
    }
error:
    LOGD("requestGetCellInfoList, doesn't have valide cellinfo.");
    RIL_onRequestComplete(t, RIL_E_NO_NETWORK_FOUND, NULL, 0);
    if (p_response != NULL) {
        at_response_free(p_response);
        p_response = NULL;
    }
    if (response != NULL) {
        free(response);
        response = NULL;
    }
}
/// @}
