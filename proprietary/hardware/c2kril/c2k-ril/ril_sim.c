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
#include <stdlib.h>
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
#include "icc.h"
#include "csim_fcp_parser.h"
#include "ril_sim.h"
// BT SIM Access Profile - START
#include <hardware/ril/librilutils/proto/sap-api.pb.h>
#include "pb_decode.h"
#include "pb_encode.h"
// BT SIM Access Profile - END
#include <libmtkrilutils.h>
#include "ril_nw.h"
#include "ril_radio.h"
#include "c2k_ril_data.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "C2K_RIL-SIM"

#define SIM_CHANNEL_CTX getRILChannelCtxFromToken(t)
#define SIM_UNUSED(x) ((void)(x))
// BT SIM Access Profile - START
#define TRUE  1
#define FALSE 0
// BT SIM Access Profile - END

extern RIL_UNSOL_Msg_Cache s_unsol_msg_cache;
extern int s_ps_on_desired;
extern int uim_modual_initialized;
extern int cardtype;
extern int uimInsertedStatus;
extern int unlock_pin_outtimes;
extern int no_service_times;
extern int inemergency;

static const struct timeval TIMEVAL_0 = {0, 0};

// BT SIM Access Profile - START
extern int at_tok_start(char **p_cur);
extern int at_tok_nextint(char **p_cur, int *p_out);
extern int at_tok_nextstr(char **p_cur, char **out);
extern int at_tok_hasmore(char **p_cur);
extern int switchStkUtkMode(int mode, RIL_Token t);
extern char ConvertNum(unsigned char* s);
extern RUIM_Status getRUIMStatus(RADIO_STATUS_UPDATE isReportRadioStatus);
extern RIL_RadioState getRadioState(void);
extern int strStartsWith(const char *line, const char *prefix);
extern int waitIccidReadable(void);
extern int waitUimModualInitialized(void);
extern void setIccidProperty(VIA_ICCID_TYPE type, char* pIccid);
extern int getCdmaModemSlot(void);
extern int isRadioOn();
// BT SIM Access Profile - END

static int isFirstReport = 1;
static PS_PARAM psParamStorePinCode = {1, 1};

void turnPSEnable(void *param);
void reportRuimStatus(void* param);
void reportCardType (void);
void reportCardTypeForURC (int CSlot);

static int isSimPinEnable(RIL_Token  t);
static void requestEnterSimPin(void*  data, size_t  datalen, RIL_Token  t);
static void requestEnterSimPuk(void*  data, size_t  datalen, RIL_Token  t);
static void requestEnterSimPin2(void*  data, size_t  datalen, RIL_Token  t);
static void requestEnterSimPuk2(void*  data, size_t  datalen, RIL_Token  t);
static void requestSetFacilityLock(void *data, size_t datalen, RIL_Token t);
static void requestGetIMSI(void *data, size_t datalen, RIL_Token t);
static void requestSIM_IO(void *data, size_t datalen, RIL_Token t);
static void sleepMsec(long long msec);
static void requestQueryFacilityLock(void *data, size_t datalen, RIL_Token t);
static void requestSubscription(void *data, size_t datalen, RIL_Token t);
static void requestSendUTKURCREG(void* param);
static int getCardStatus(VIA_RIL_CardStatus **pp_card_status, RIL_Token t);
static void freeCardStatus(VIA_RIL_CardStatus *p_card_status);
static void getChvCount(int * pin1Count, int * pin2Count, int * puk1Count, int * puk2Count);
static void storePinCode(const char*  string );
static void requestSimOpenNFCChannel(void *data, size_t datalen, RIL_Token t);
// static void requestSimOpenNFCChannelWithSw(void *data, size_t datalen, RIL_Token t);
static void requestSimCloseNFCChannel(void *data, size_t datalen, RIL_Token t);
static void requestSimGetATRFORNFC(void *data, size_t datalen, RIL_Token t);
static void requestSimTransmitBasic(void *data, size_t datalen, RIL_Token t);
static void requestSimTransmitChannel(void *data, size_t datalen, RIL_Token t);
static int requestCGLA_OpenChannelWithSw(RIL_SIM_IO_Response *sr, int *len,
        int NFCchannel, int length, int cla, int ins, int p1, int p2, int p3,
        char **data, RIL_Token t);
static int requestSimChannelAccess(int sessionid, char *senddata, RIL_SIM_IO_Response *output);
static void requestSwitchCardType(void *data, size_t datalen, RIL_Token t);
static void requestCdmaGetSubscriptionSource(void *data, size_t datalen, RIL_Token t);

// BT SIM Access Profile - START
static BtSapStatus s_bt_sap_status[SIM_COUNT] = {BT_SAP_INIT};
static unsigned char* s_bt_sap_atr[SIM_COUNT] = {NULL};
static int s_bt_sap_current_protocol[SIM_COUNT] = {0};
static int s_bt_sap_support_protocol[SIM_COUNT] = {0};

static LocalBtSapMsgHeader* allocateLocalBtSapMsgHeader(void *param,
        RIL_Token t, RIL_SOCKET_ID rid);
static void releaseLocalBtSapMsgHeader(LocalBtSapMsgHeader *localMsgHdr);
static void decodeBtSapPayload(MsgId msgId, void *src, size_t srclen, void *dst);
static void requestBtSapConnect(void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID rid);
static void requestLocalSapConnect(void *param);
static void requestBtSapDisconnect(void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID rid);
static void requestLocalBtSapDisconnect(void *param);
static void requestBtSapTransferApdu(void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID rid);
static void requestLocalBtSapTransferApdu(void *param);
static void requestBtSapTransferAtr(void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID rid);
static void requestBtSapPower(void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID rid);
static void requestLocalBtSapPower(void *param);
static void requestBtSapResetSim(void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID rid);
static void requestLocalBtSapResetSim(void *param);
static void requestBtSapSetTransferProtocol(void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID rid);
static void sendBtSapResponseComplete(RIL_Token t, RIL_Errno ret, MsgId msgId,
        void *data);
static char* btSapMsgIdToString(MsgId msgId);
static void resetBtSapContext(RIL_SOCKET_ID rid);
static void notifyBtSapStatusInd(RIL_SIM_SAP_STATUS_IND_Status message, RIL_SOCKET_ID rid);
// BT SIM Access Profile - END

/* ALPS01949738, Send CPOF->CPON after SIM PIN, 2015/02/16 {*/
static bool isNeedToSetRadio(VIA_CARD_STATE card_type);
static void TriggerMD3BootPower();
/* ALPS01949738, Send CPOF->CPON after SIM PIN, 2015/02/16 }*/
static void requestGetAllowedCarriers(void *data, size_t datalen, RIL_Token t);
static void requestSetAllowedCarriers(void *data, size_t datalen, RIL_Token t);

/* ALPS02217042: Trigger to power off/on MD only under SIM PIN mode {*/
static SimPowerController s_sim_power_ctrl = {
        CARD_NOT_INSERTED,
        _FALSE,
        updateCardState,
        setSimPower,
        refreshSimPower,
        triggerSetSimPower};
/* ALPS02217042: Trigger to power off/on MD only under SIM PIN mode }*/

// The prefix of AID which support on our platform
// Support USIM, SIM, ISIM currently
const static char* aid_support_list[AID_MAX] = {
    // {"A0000000871002"}, // RUIM no aid
    "a0000003431002",  // CSIM
};

static int read_ef_dir_complete = 0;
static AidInfo aid_info_array[AID_MAX];

void resetAidInfo() {
    int i = 0;
    for (i = 0; i < AID_MAX; i++) {
        memset(&aid_info_array[i], 0, sizeof(AidInfo));
    }
    read_ef_dir_complete = 0;
}

AidInfo* getAidInfo(AidIndex index) {
    RLOGD("getAidInfo, aid_len %d, applabel_len %d", aid_info_array[index].aid_len,
            aid_info_array[index].app_label_len);
    return &aid_info_array[index];
}

int queryAppType(char* pAid) {
    int match = -1, i = 0;
    for (i = 0; i < AID_MAX; i++) {
        if (pAid != NULL && (memcmp(aid_support_list[i], pAid, AID_PREFIX_LEN) == 0)) {
            match = i;
        }
    }
    RLOGD("matchAppAid, match %d", match);
    return match;
}

static int toByte(char c) {
    if (c >= '0' && c <= '9') return (c - '0');
    if (c >= 'A' && c <= 'F') return (c - 'A' + 10);
    if (c >= 'a' && c <= 'f') return (c - 'a' + 10);

    RLOGE("toByte Error: %c", c);
    return 0;
}

int hexStringToByteArray(unsigned char* hexString, unsigned char ** byte) {
    int length = strlen((char*)hexString);
    int i = 0;
    unsigned char* buffer = calloc(1, length / 2);
    assert(buffer != NULL);

    for (i = 0 ; i < length ; i += 2) {
        buffer[i / 2] = (unsigned char)((toByte(hexString[i]) << 4) | toByte(hexString[i+1]));
    }

    *byte = buffer;

    return (length/2);
}

/*****************************************************************************
 * FUNCTION
 *  fcp_tlv_search_tag
 * DESCRIPTION
 *  Search for the tag in the input ptr and return the length and value ptr of the tag.
 * PARAMETERS
 *  in_ptr      [IN]        Kal_uint8 * input buffer pointer (from the value part of fcp template)
 *  len         [IN]        Kal_int16  input buffer length
 *  tag         [IN]        Usim_fcp_tag_enum tag to be found
 *  out_ptr     [OUT]       Kal_uint8 ** the address of found data value.
 * RETURNS
 *  the length of the tag searched
 * GLOBALS AFFECTED
 *  void
 *****************************************************************************/
unsigned char fcp_tlv_search_tag(unsigned char *in_ptr, unsigned short len,
    csim_fcp_tag_enum tag, unsigned char **out_ptr) {
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    unsigned char tag_len = 0;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    for (*out_ptr = in_ptr; len > 0; *out_ptr += tag_len) {
        tag_len = (*(*out_ptr + 1) + 2);
        if (**out_ptr == (unsigned char) tag) {
            *out_ptr += 2;
            return *(*out_ptr - 1);
        }

        if (len <= tag_len) {
            LOGE("fcp_tlv_search_tag, len: %d, tag_len: %d", len, tag_len);
            len = 0;
        } else {
            len -= tag_len;
        }
    }
    if (len != 0) {
        assert(0);
    }
    *out_ptr = NULL;
    return 0;
}

static int readEfDirResponseParser(unsigned char ** simResponse, int *record_length) {
    int record_num = -1;
    unsigned char * fcpByte = NULL;
    unsigned short  fcpLen = 0;
    unsigned char *out_ptr = NULL;
    int value_len = 0;
    // csim_file_descriptor_struct fDescriptor = {0,0,0,0};

    fcpLen = hexStringToByteArray(*simResponse, &fcpByte);

    if ((fcpByte[1] == 0) || (fcpByte[0] != 0x62)) {
        assert(0);
    }
    value_len = fcp_tlv_search_tag(&fcpByte[2], fcpByte[1], FCP_FILE_DES_T, &out_ptr);
    /* File descriptor is mandatory for all FCP template */
    if (!((NULL != out_ptr) && ((value_len == 2) || (value_len == 5)))) {
        // assert(0);
        RLOGE("fcp_file_descriptor_query, no valid file descriptor! value_len: %d", value_len);
    } else {
        if (out_ptr != NULL) {
            if (value_len == 5) {
                (*record_length) = (short) ((out_ptr[2] << 8) | out_ptr[3]);
                record_num = out_ptr[4];
            } else {
                (*record_length) = 0;
                record_num = 0;
            }
        } else {
            RLOGE("USIM FD Parse fail");
        }
    }
    free(fcpByte);
    RLOGE("readEfDirResponseParser, num: %d, length: %d", record_num, (*record_length));
    return record_num;
}

static char* queryEfDirReadLine(char* line) {
    int err;
    RIL_SIM_IO_Response sr;

    memset(&sr, 0, sizeof(RIL_SIM_IO_Response));

    do {
        err = at_tok_start(&line);
        if (err < 0) {
            break;
        }

        err = at_tok_nextint(&line, &(sr.sw1));
        if (err < 0) {
            break;
        }

        err = at_tok_nextint(&line, &(sr.sw2));
        if (err < 0) {
            break;
        }

        if (at_tok_hasmore(&line)) {
            err = at_tok_nextstr(&line, &(sr.simResponse));
            if (err < 0) {
                break;
            }
        }
    } while (0);

    RLOGD("queryEfDirReadLine, result: %d", ((sr.simResponse != NULL)? 1 : 0));
    return sr.simResponse;
}

int hexStringToByteArrayEx(unsigned char* hexString, int hexStringLen, unsigned char ** byte) {
    int length = hexStringLen/2;
    int i = 0;
    unsigned char* buffer = malloc((length + 1)*sizeof(char));
    assert(buffer != NULL);

    memset(buffer, 0, ((length + 1)*sizeof(char)));
    for (i = 0 ; i < hexStringLen ; i += 2) {
        buffer[i / 2] = (unsigned char)((toByte(hexString[i]) << 4) | toByte(hexString[i+1]));
    }

    *byte = buffer;

    return (hexStringLen/2);
}


void queryEfDir(RIL_Token t) {
    int err;
    int record_num = -1, i = 0, j = 0, record_length = 0;
    int app_info_len = 0, aid_len = 0, app_label_len = 0;
    int fcpLen = 0;
    char * line = NULL;
    char * cmd = NULL;
    unsigned char * fcpByte = NULL;
    ATResponse * p_response = NULL;
    ATResponse * p_response2 = NULL;

    // RLOGD("queryEfDir, rid: %d, USIM: %d, flag: %d", rid, isUsimDetect[rid], read_ef_dir_complete[rid]);
    if (read_ef_dir_complete != 0) {
        return;
    }
    read_ef_dir_complete = 1;

    asprintf(&cmd, "AT+CRSM=%d,%d,%d,%d,%d,%d", 0xa4, 0x2f00, 0, 0 , 0, 0);

    do {
        err = at_send_command_singleline(cmd, "+CRSM:", &p_response, SIM_CHANNEL_CTX);
        if (err < 0 || p_response == NULL || p_response->success == 0) {
            RLOGE("Fail to GET_RESPONSE EfDir");
        } else {
            if ((line = queryEfDirReadLine(p_response->p_intermediates->line)) != NULL) {
                if ((record_num = readEfDirResponseParser((unsigned char **)&line, &record_length)) >= 0) {
                    // To read applicaion records
                    int matched = 0;
                    for (i = 0; i < record_num; i++) {
                        free(cmd);
                        cmd = NULL;
                        asprintf(&cmd, "AT+CRSM=%d,%d,%d,%d,%d,%d", 0xb2, 0x2f00, 0, (i+1) , 4, record_length);
                        err = at_send_command_singleline(cmd, "+CRSM:", &p_response2,
                                SIM_CHANNEL_CTX);
                        if (err < 0 || p_response2->success == 0) {
                            RLOGE("Fail to READ_RECORD EfDir");
                        } else {
                            if ((line = queryEfDirReadLine(p_response2->p_intermediates->line)) != NULL) {
                                fcpLen = hexStringToByteArrayEx((unsigned char *)line, 8, &fcpByte);
                                // Refer to in TS 102.221 13.1, the 2nd byte denotes the length of application
                                app_info_len = (int)fcpByte[1];
                                // Refer to in TS 102.221 13.1, the 4th byte denotes the length of AID
                                aid_len = (int)fcpByte[3];
                                free(fcpByte);
                                fcpByte = NULL;
                                // The range of AID len and application len are defined in TS 102.221 13.1
                                RLOGD("app len %d, AID len %d", app_info_len, aid_len);
                                if ((app_info_len < 3 || app_info_len > 127) ||
                                    (aid_len < 1 || aid_len > 16)) {
                                    RLOGE("Parameter are wrong!");
                                    break;
                                }
                                for (j = 0; j < AID_MAX; j++) {
                                    if (memcmp(aid_support_list[j], &line[8], AID_PREFIX_LEN) == 0) {
                                        memcpy(aid_info_array[j].aid, &line[8], (aid_len*2));
                                        aid_info_array[j].aid_len = aid_len;
                                        matched++;
                                        if ((app_info_len-(aid_len+1)) > 2) {
                                            // There is application label
                                            fcpLen = hexStringToByteArrayEx((unsigned char *)&line[
                                                    8+(aid_len*2)], 4, &fcpByte);
                                            app_label_len = (int)fcpByte[1];
                                            aid_info_array[j].app_label_len = app_label_len;
                                            free(fcpByte);
                                            fcpByte = NULL;
                                            RLOGD("app_label_len %d", app_label_len);
                                            memcpy(aid_info_array[j].appLabel, &line[12+(aid_len*2)],
                                                    (app_label_len*2));
                                        }
                                        RLOGD("aid: %s, app_label: %s", aid_info_array[j].aid,
                                                aid_info_array[j].appLabel);
                                        break;
                                    }
                                };
                                if (matched == AID_MAX) {
                                    break;
                                }
                            }
                        }
                        at_response_free(p_response2);
                        p_response2 = NULL;
                    }
                }
            }
        }
    } while (0);

    at_response_free(p_response);
    if (p_response2 != NULL) {
        at_response_free(p_response2);
    }
    if (cmd != NULL) {
        free(cmd);
    }
}

int rilSimMain(int request, void *data, size_t datalen, RIL_Token t)
{
    //LOGD("rilSimMain enter");
    switch (request) {
        case RIL_REQUEST_GET_SIM_STATUS: {
            VIA_RIL_CardStatus *p_card_status = NULL;
            RIL_CardStatus_v8 *p_oem_card_status = NULL;
            char *p_buffer;
            int buffer_size;
            char tmp[PROPERTY_VALUE_MAX] = {0};
            char tmp2[PROPERTY_VALUE_MAX] = {0};

            property_get(PROPERTY_RIL_CT3G[getCdmaSocketSlotId()-1], tmp, "");
            property_get(PROPERTY_RIL_UICC_TYPE[getCdmaSocketSlotId()-1], tmp2, "");

            if ((strcmp("1", tmp) == 0) && ((strcmp("CSIM", tmp2) != 0) && (strcmp("RUIM", tmp2) != 0))) {
                RLOGD("ct3g: %s, uicc type: %s", tmp, tmp2);
                RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
                break;
            }

            int result = getCardStatus(&p_card_status, t);
            if (result == RIL_E_SUCCESS) {
                dispatchCardStatus(p_card_status, &p_oem_card_status);
                p_buffer = (char *) p_oem_card_status;
                buffer_size = sizeof(*p_oem_card_status);
            } else {
                p_buffer = NULL;
                buffer_size = 0;
            }
            RIL_onRequestComplete(t, result, p_buffer, buffer_size);
            if (p_card_status != NULL) {
                freeCardStatus(p_card_status);
            }
            if (p_oem_card_status != NULL) {
                free(p_oem_card_status);
                p_oem_card_status = NULL;
            }
            break;
        }
        case RIL_REQUEST_ENTER_SIM_PIN:
        case RIL_REQUEST_CHANGE_SIM_PIN:
            requestEnterSimPin(data, datalen, t);
            break;
        case RIL_REQUEST_ENTER_SIM_PUK:
            requestEnterSimPuk(data, datalen, t);
            break;
       case RIL_REQUEST_ENTER_SIM_PIN2:
       case RIL_REQUEST_CHANGE_SIM_PIN2:
            requestEnterSimPin2(data, datalen, t);
            break;
       case RIL_REQUEST_ENTER_SIM_PUK2:
            requestEnterSimPuk2(data, datalen, t);
            break;
        case RIL_REQUEST_GET_IMSI:
            requestGetIMSI(data, datalen, t);
            break;
        case RIL_REQUEST_SIM_IO:
            requestSIM_IO(data, datalen, t);
            break;
        case RIL_REQUEST_QUERY_FACILITY_LOCK:
            requestQueryFacilityLock(data, datalen, t);
            break;
        case RIL_REQUEST_SET_FACILITY_LOCK:
            requestSetFacilityLock(data, datalen, t);
           break;
        case RIL_REQUEST_CDMA_SUBSCRIPTION:
            requestSubscription(data, datalen, t);
            break;
        case RIL_REQUEST_SIM_TRANSMIT_APDU_BASIC:
            requestSimTransmitBasic(data, datalen, t);
            break;
        case RIL_REQUEST_SIM_TRANSMIT_APDU_CHANNEL:
            requestSimTransmitChannel(data, datalen, t);
            break;
        case RIL_REQUEST_SIM_OPEN_CHANNEL:
            requestSimOpenNFCChannel(data, datalen, t);
            break;
        case RIL_REQUEST_SIM_CLOSE_CHANNEL:
            requestSimCloseNFCChannel(data, datalen, t);
            break;
        case RIL_REQUEST_SIM_GET_ATR:
            requestSimGetATRFORNFC(data, datalen, t);
            break;
        case RIL_LOCAL_C2K_REQUEST_SWITCH_CARD_TYPE:
            requestSwitchCardType(data, datalen, t);
            break;
        case RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE :
            requestCdmaGetSubscriptionSource(data, datalen, t);
            break;
        case RIL_REQUEST_SET_ALLOWED_CARRIERS:
            requestSetAllowedCarriers(data, datalen, t);
            break;
        case RIL_REQUEST_GET_ALLOWED_CARRIERS:
            requestGetAllowedCarriers(data, datalen, t);
            break;
        default:
            return 0; /* no matched request */
    }
    return 1;
}

    int rilSimUnsolicited(const char *s, const char *sms_pdu) {
        SIM_UNUSED(sms_pdu);
        char *line = NULL;
        int err;
        int cdmaSlot = getActiveSvlteModeSlotId();
        int currentprotocol = -1;
        int currentCSlot = -1;
        if (strStartsWith(s, "+UIMST:")) {
            char *dup = strdup(s);
            if (!dup) {
                return 1;
            }

            int cardState = 0;
            line = dup;
            err = at_tok_start(&line);
            if (err < 0) {
                free(dup);
                return 1;
            }
            err = at_tok_nextint(&line, &cardState);
            if (err < 0) {
                free(dup);
                return 1;
            }
            if (isSvlteSupport()) {
                //SVLTE project, MD3 UIMST URC has no the second parameter, so do nothing
                LOGD("SVLTE, no the second param for UIMST");
            } else {
                //Other case, SRLTE project, add the second parameter for check
                int urcprotocol = 0;
                err = at_tok_nextint(&line, &urcprotocol);
                if (err < 0) {
                    free(dup);
                    return 1;
                }
                // Check if the urc is for current slot. During dynamic switch urc for the slot may
                // be sent to another. Here is to ignore unmatched uimst urc
                char protocolInfo[PROPERTY_VALUE_MAX] = { 0 };
                property_get("vendor.ril.cdma.emdstatus.protocol", protocolInfo, "");
                LOGD("protocolInfo: %s, urcprotocol: %d, cdmaSlot: %d", protocolInfo, urcprotocol, cdmaSlot);
                char *dupprotocolInfo = strdup(protocolInfo);
                if (!dupprotocolInfo) {
                    free(dup);
                    LOGD("dupprotocolInfo == null");
                    return 1;
                }
                char *dupline = dupprotocolInfo;
                err = at_tok_nextint(&dupline, &currentprotocol);
                if (err < 0) {
                    free(dup);
                    free(dupprotocolInfo);
                    LOGD("currentprotocol: %d", currentprotocol);
                    return 1;
                }
                err = at_tok_nextint(&dupline, &currentCSlot);
                if (err < 0) {
                    free(dup);
                    free(dupprotocolInfo);
                    LOGD("currentCSlot: %d", currentCSlot);
                    return 1;
                }
                free(dupprotocolInfo);
                if ((currentprotocol != urcprotocol) && strStartsWith(s, "+UIMST:255")) {
                    free(dup);
                    LOGD("currentprotocol-1: %d", currentprotocol);
                    return 1;
                }
            }

            // Check the no card urc.
            if (strStartsWith(s, "+UIMST:255")) {
                char tmp[PROPERTY_VALUE_MAX] = { 0 };
                property_get(PROPERTY_RIL_FULL_UICC_TYPE[getCdmaSocketSlotId()-1], tmp, "");
                if ((strcmp("SIM", tmp) == 0) || (strcmp("USIM", tmp) == 0)) {
                    LOGD("%s: %s  Ignore +UIMST:255 for gsm only card",
                            PROPERTY_RIL_FULL_UICC_TYPE[getCdmaSocketSlotId()-1], tmp);
                    free(dup);
                    return 1;
                }
                if (isSrlteSupport()) {
                    if (cdmaSlot != currentCSlot) {
                        free(dup);
                        LOGD("currentCSlot-1: %d", currentCSlot);
                        return 1;
                    }
                }
                LOGD("resetUtkStatus");
                resetUtkStatus();
            }
            // Reset aid info and it will read during get_card_status.
            resetAidInfo();
            // add for utk end

            if (!uim_modual_initialized)
                uim_modual_initialized = 1;

            /*add for hot plug:handle misinformation*/
            /*use "0" to inform framework this is misinformation*/
            cardtype = cardState;
            s_sim_power_ctrl.update(&s_sim_power_ctrl, cardState);
            LOGD("cardtype is %d", cardtype);

            /* ALPS01949738, Send CPOF->CPON after SIM PIN, 2015/02/16 {*/
            if (isNeedToSetRadio(cardtype)) {
                RIL_requestProxyTimedCallback(TriggerMD3BootPower, NULL, NULL, NW_CHANNEL);
            }
            /* ALPS01949738, Send CPOF->CPON after SIM PIN, 2015/02/16 }*/

            int mSlotId = cdmaSlot;//getActiveSvlteModeSlotId();
            if ((CARD_NOT_INSERTED == cardState) || (UNKOWN_CARD == cardState)
                    || (LOCKED_CARD == cardState)) {
                LOGD("No Card Inserted!");
                uimInsertedStatus = UIM_STATUS_NO_CARD_INSERTED;
                // setIccidProperty(SET_TO_NA, NULL);
                // resetSIMProperties("gsm.ril.fulluicctype");
                if (mSlotId != -1) {
                    char ct3gRoaming2[PROPERTY_VALUE_MAX] = { 0 };
                    property_get(PROPERTY_RIL_CT3G_ROAMING2[mSlotId - 1], ct3gRoaming2, "0");
                    RLOGD("ct3gRoaming2: %s", ct3gRoaming2);
                    if (strcmp("1", ct3gRoaming2) == 0) {
                        RLOGD("Now is 3G CT card and In IR, NOT set iccid to N/A");
                    } else {
                        property_set(PROPERTY_ICCID_SIM[mSlotId - 1], "N/A");
                        RLOGD("Reset %s to N/A", PROPERTY_ICCID_SIM[mSlotId - 1]);
                    }
                } else {
                    RLOGD("Invalid svlte slotid: %d", mSlotId);
                }
                RIL_onUnsolicitedResponse(RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, NULL, 0);
            } else {
                uimInsertedStatus = UIM_STATUS_CARD_INSERTED;
                char svlteIccidStatus[PROPERTY_VALUE_MAX] = { 0 };
                char maskIccid[PROPERTY_VALUE_MAX] = {0};
                if (mSlotId != -1) {
                    property_get(PROPERTY_ICCID_SIM[mSlotId-1], svlteIccidStatus, "");
                }
                givePrintableIccid(svlteIccidStatus, maskIccid);
                RLOGD("svlteIccidStatus:%s", maskIccid);
                if (isCdmaLteDcSupport() && (!strcmp(svlteIccidStatus, "N/A"))) {
                    setIccidProperty(CLEAN_ICCID, NULL);
                    if (mSlotId != -1) {
                        property_set(PROPERTY_ICCID_SIM[mSlotId-1], "");
                        RLOGD("Reset %s to NULL", PROPERTY_ICCID_SIM[mSlotId-1]);
                    } else {
                        RLOGD("Invalid svlte slotid: %d", mSlotId);
                    }
                }
            }

            //add by bin, send card type to framework in startup
            //if the value of UIMST: is valid, then we send it to framework
            reportCardTypeForURC(cdmaSlot);

            if (isFirstReport) {
                if (UIM_STATUS_CARD_INSERTED == uimInsertedStatus) {
                    RIL_requestProxyTimedCallback(requestSendUTKURCREG, NULL, NULL, SIM_CHANNEL);
                }
                reportRuimStatus(&uimInsertedStatus);
                isFirstReport = 0;
            }
            free(dup);
            return 1;
        } else if (strStartsWith(s, "+ECSIMP:")) {
            int activate = 0;
            char *dup = strdup(s);

            line = dup;
            err = at_tok_start(&line);
            if (err < 0) {
                RLOGE("Unknown command!");
                free(dup);
                return 1;
            }

            err = at_tok_nextint(&line, &activate);
            if (err < 0) {
                RLOGE("Unknown parameter!");
                free(dup);
                return 1;
            }
            RLOGD("CSIM activate: %d", activate);
            RIL_onUnsolicitedResponse(RIL_UNSOL_UICC_SUBSCRIPTION_STATUS_CHANGED, &activate,
                    sizeof(int));
            free(dup);
            return 1;
        }
        return 0;
    }

static int isSimPinEnable(RIL_Token  t)
{
    ATResponse   *p_response = NULL;
    char*         cmd = NULL;
    char*         line = NULL;
    int           err;
    int           isSimPinEnabled = 0;
    int           result = -1;

    /*Check PIN is enabled or not*/
    asprintf(&cmd, "AT+CLCK=\"SC\",2");
    err = at_send_command_singleline(cmd, "+CLCK:", &p_response, SIM_CHANNEL_CTX);
    free(cmd);
    if (err < 0) {
        LOGE("query SIM PIN lock:%d",err);
        goto done;
    }

    if (p_response == NULL || p_response->success == 0) {
        LOGE("query SIM PIN error p_response->success = 0");
        goto done;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) {
        LOGE("query SIM PIN lock, get token error");
        goto done;
    }

    err = at_tok_nextint(&line, &(isSimPinEnabled));
    if (err < 0) {
        LOGE("query SIM PIN lock, get result fail");
        goto done;
    }

    LOGD("query SIM PIN lock, isSimPinEnabled: %d", isSimPinEnabled);

    if (isSimPinEnabled == 0) {
        result = 0;
    } else if (isSimPinEnabled == 1) {
        result = 1;
    }
done:
    at_response_free(p_response);
    return result;
}

static void requestEnterSimPin(void*  data, size_t  datalen, RIL_Token  t)
{
    ATResponse   *p_response = NULL;
    int           err;
    char*         cmd = NULL;
    const char**  strings = (const char**)data;;
    int index = 0;
    int simPinEnable = -1;
    PS_PARAM psParam;
    memset(&psParam, 0, sizeof(psParam));
    int chvCount[4];

    if ( datalen == 2*sizeof(char*) ) {
        psParam.enable = 0;
        psParam.force = 1;
        turnPSEnable((void *)&psParam);
        asprintf(&cmd, "AT+CPIN=\"%s\"", strings[0]);
    } else if ( datalen == 3*sizeof(char*) ) {
        /*Add for ALPS02252331 start*/
        simPinEnable = isSimPinEnable(t);
        if (simPinEnable == 0) {
            RIL_onRequestComplete(t, RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0);
            return;
        } else if (simPinEnable == -1) {
            goto error;
        }
        /*Add for ALPS02252331 end*/
        asprintf(&cmd, "AT+CPIN=\"%s\",\"%s\"", strings[0], strings[1]);
    } else
        goto error;

    err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
    free(cmd);

    if (err < 0 || p_response == NULL || p_response->success == 0) {
        getChvCount(chvCount, &chvCount[1], &chvCount[2], &chvCount[3]);
        if (chvCount[0] <= 0) {
            RIL_onUnsolicitedResponse (RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, NULL, 0);
        }
        /* ALPS02148729, APP(Settings) needs left count to show message */
        RIL_onRequestComplete(t, RIL_E_PASSWORD_INCORRECT, &chvCount[0], sizeof(chvCount[0]));
    } else {
        getChvCount(chvCount, &chvCount[1], &chvCount[2], &chvCount[3]);
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);

        /*use datalen to distinguish RIL_REQUEST_ENTER_SIM_PIN and RIL_REQUEST_CHANGE_SIM_PIN,
           RIL_REQUEST_ENTER_SIM_PIN use first parameter and RIL_REQUEST_CHANGE_SIM_PIN use the second one*/
        if(datalen == 2*sizeof(char*) ) {
            index = 0;
            storePinCode(strings[index]);
            //fix CR2155, after unlock pin, change service state to searching
            //avoid to trigger RIL_REQUEST_RADIO_POWER_CARD_SWITCH(0) flow
            s_unsol_msg_cache.service_state = 2;
        } else if(datalen == 3*sizeof(char*) ) {
            index = 1;
            storePinCode(strings[index]);
        }
    }
    at_response_free(p_response);

    return;
error:
    getChvCount(chvCount, &chvCount[1], &chvCount[2], &chvCount[3]);
    if (chvCount[0] <= 0) {
        RIL_onUnsolicitedResponse (RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, NULL, 0);
    }
    /* ALPS02148729, APP(Settings) needs left count to show message */
    RIL_onRequestComplete(t, RIL_E_PASSWORD_INCORRECT, &chvCount[0], sizeof(chvCount[0]));
}

static void requestEnterSimPuk(void*  data, size_t  datalen, RIL_Token  t)
{
    ATResponse   *p_response = NULL;
    int           err;
    char*         cmd = NULL;
    const char**  strings = (const char**)data;
    int chvCount[4];

    if ( datalen == 2*sizeof(char*) ) {
        asprintf(&cmd, "AT+CPIN=\"%s\"", strings[0]);
    } else if ( datalen == 3*sizeof(char*) ) {
        s_sim_power_ctrl.execute(&s_sim_power_ctrl);
        asprintf(&cmd, "AT+CPUK=\"%s\",\"%s\"", strings[0], strings[1]);
    } else
        goto error;

    err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
    free(cmd);

    if (err < 0 || p_response == NULL || p_response->success == 0) {
error:
        getChvCount(chvCount, &chvCount[1], &chvCount[2], &chvCount[3]);
        /* ALPS02148729, APP(Settings) needs left count to show message */
        RIL_onRequestComplete(t, RIL_E_PASSWORD_INCORRECT, &chvCount[2], sizeof(chvCount[2]));
    } else {
        getChvCount(chvCount, &chvCount[1], &chvCount[2], &chvCount[3]);
        //add to fix HANDROID#2225
        if(unlock_pin_outtimes == 1)
            unlock_pin_outtimes = 0;

        //fix2255
        s_unsol_msg_cache.service_state = 2;

        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
    at_response_free(p_response);
}

static void requestEnterSimPin2(void*  data, size_t  datalen, RIL_Token  t) {

    ATResponse *p_response = NULL;
    int err = 0;
    char *cmd = NULL;
    const char **strings = (const char**)data;;
    int chvCount[4] = {0};
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;

    assert(NULL != strings);
    LOGD("%s(): datalen: %lu.", __FUNCTION__, datalen);

    do {
        if ( datalen == 2*sizeof(char*) ) {
            asprintf(&cmd, "AT+CPIN2=\"%s\"", strings[0]);
            LOGD("%s(): pin2: %s.", __FUNCTION__, strings[0]);
        } else if ( datalen == 3*sizeof(char*) ) {
            asprintf(&cmd, "AT+CPIN2=\"%s\",\"%s\"", strings[0], strings[1]);
            LOGD("%s(): pin2: %s, new pin2: %s.", __FUNCTION__, strings[0], strings[1]);
        } else {
            LOGE("%s(): invalid datalen.", __FUNCTION__);
            break;
        }

        err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
        free(cmd);

        getChvCount(chvCount, &chvCount[1], &chvCount[2], &chvCount[3]);

        if (p_response != NULL && p_response->success == 0) {
            switch (at_get_cme_error(p_response)) {
                case CME_INCORRECT_PASSWORD:
                    ret = RIL_E_PASSWORD_INCORRECT;
                    break;
                case CME_SIM_PUK2_REQUIRED:
                    ret = RIL_E_SIM_PUK2;
                    break;
                default:
                    ret = RIL_E_REQUEST_NOT_SUPPORTED;
                    break;
            }
        }

        if (err < 0 || p_response == NULL || p_response->success == 0) {
            if (chvCount[1] <= 0) {
                RIL_onUnsolicitedResponse (RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, NULL, 0);
            }
            RIL_onRequestComplete(t, ret, &chvCount[1], sizeof(chvCount[1]));
        } else {
            RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        }
        at_response_free(p_response);
    } while(0);

}

static void requestEnterSimPuk2(void*  data, size_t  datalen, RIL_Token  t) {
    ATResponse *p_response = NULL;
    int err = 0;
    char *cmd = NULL;
    const char **strings = (const char**)data;
    int chvCount[4] = {0};

    assert(NULL != strings);
    assert(datalen == 3*sizeof(char*));

    LOGD("%s(): datalen: %lu.", __FUNCTION__, datalen);
    asprintf(&cmd, "AT+CPUK2=\"%s\",\"%s\"", strings[0], strings[1]);
    LOGD("%s(): puk2: %s, pin2: %s.", __FUNCTION__, strings[0], strings[1]);

    err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
    free(cmd);
    getChvCount(chvCount, &chvCount[1], &chvCount[2], &chvCount[3]);
    if (err < 0 || p_response->success == 0) {
        RIL_onRequestComplete(t, RIL_E_PASSWORD_INCORRECT, &chvCount[3], sizeof(chvCount[3]));
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
    at_response_free(p_response);
}

static void turnOffPSCallBack(void * param)
{
    SIM_UNUSED(param);
    // setRadioState(RADIO_STATE_RUIM_LOCKED_OR_ABSENT);
    PS_PARAM psParam;
    memset(&psParam, 0, sizeof(psParam));
    psParam.enable = 0;
    psParam.force = 1;
    turnPSEnable((void *) &psParam);
}

// copy it ril_callback.c to here
static char *myitoa(int num, char *str, int radix)
{
    char index[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    unsigned unum;
    int i = 0, j, k;
    if (radix == 10 && num < 0) {
        unum = (unsigned) -num;
        str[i++] = '-';
    } else
        unum = (unsigned) num;
    do {
        str[i++] = index[unum % (unsigned) radix];
        unum /= radix;
    } while (unum);
    str[i] = '\0';
    if (str[0] == '-')
        k = 1;
    else
        k = 0;
    for (j = k; j < (i - 1) / 2.0 + k; j++)
    {
        num = str[j];
        str[j] = str[i - j - 1 + k];
        str[i - j - 1 + k] = num;
    }
    return str;
}



/* Fix PIN LOCK NE, 2015/01/14{*/
// TODO: To figure out what is "vendor.net.cdma.pc" for
static void updateNetCdmaPC(const char* p_str) {
#define PIN_SIZE (9)
    char str_array[PIN_SIZE] = {0};
    char pin_code[PROPERTY_VALUE_MAX] = {0};
    char * p_pw_str = NULL;
    int pin_num = 0;
    *str_array = '\0';
    strncat(str_array, p_str, PIN_SIZE-1);
    pin_num = atoi(p_str);
    pin_num = pin_num * 57 + 13;
    p_pw_str = myitoa(pin_num , str_array, 10);
    property_set("vendor.net.cdma.pc", p_pw_str);
    sleep(1);
    property_get("vendor.net.cdma.pc", pin_code, "none");
    LOGD("%s: pin_code = %s", __FUNCTION__, pin_code);
#undef PIN_SIZE
}
/* Fix PIN LOCK NE, 2015/01/14}*/

static void requestSetFacilityLock(void *data, size_t datalen, RIL_Token t)
{
    SIM_UNUSED(datalen);
    /* It must be tested if the Lock for a particular class can be set without
      * modifing the values of the other class. If not, first must call
      * requestQueryFacilityLock to obtain the previus value
     */
    ATResponse   *p_response = NULL;
    int err = 0;
    const char** STRS = (const char**)data;
    char *cmd = NULL;
    const char *fac = STRS[0];
    const char *mode = STRS[1];
    const char *password = STRS[2];
    const char *classx = STRS[3];
    // struct timeval TIMEVAL_TURN_OFF_PS = {1, 0};
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;

    int chvCount[4];

    assert (datalen >=  (4 * sizeof(char **)));

    asprintf(&cmd, "AT+CLCK=\"%s\",%s,\"%s\",%s", fac, mode, password, classx);
    err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
    free(cmd);
    getChvCount(chvCount, &chvCount[1], &chvCount[2], &chvCount[3]);
    if (chvCount[0] <= 0) {
        //fix HANDROID#2225 to cancle pinlock failed,set service_state = 0
        unlock_pin_outtimes = 1;
        //add URC to fix fix HANDROID#2225, trigger framework send voice_registration request as soon as possible
        RIL_onUnsolicitedResponse (
            RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED,
            NULL, 0);

        RIL_onUnsolicitedResponse (RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, NULL, 0);
        #ifdef OPEN_PS_ON_CARD_ABSENT
        // setRadioState(RADIO_STATE_RUIM_LOCKED_OR_ABSENT);
        #else
        // Turn off radio after 1s, for we should not block the ril here.
        RIL_requestProxyTimedCallback(turnOffPSCallBack, NULL, &TIMEVAL_TURN_OFF_PS,
                SIM_CHANNEL);
        #endif
    }
    if (err < 0 || p_response == NULL || p_response->success == 0) {
        if (0 == strcmp(STRS[0],"SC")) {
            RIL_onRequestComplete(t, RIL_E_PASSWORD_INCORRECT, &chvCount[0], sizeof(chvCount[0]));
        } else if (0 == strcmp(STRS[0],"FD")) {
            ret = ((chvCount[1] == 0) ? RIL_E_SIM_PUK2 : RIL_E_PASSWORD_INCORRECT);
            RIL_onRequestComplete(t, ret, &chvCount[1], sizeof(chvCount[1]));
        }
    } else {
        if(!strncmp(mode, "1", 1) && (NULL != password)) {
            updateNetCdmaPC(password);
        }
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
    at_response_free(p_response);
}

void reportRuimStatus(void* param)
{
    int status = *((int*)(param));
    LOGD("reportRuimStatus status is %d", status);

}

void reportCardType (void) {
    int notifyfw = 1;
    char tmp[PROPERTY_VALUE_MAX] = {0};
    int mSlotId = getActiveSvlteModeSlotId();
    if (mSlotId != -1) {
        property_get(PROPERTY_RIL_CT3G_ROAMING2[mSlotId - 1], tmp, "0");
        LOGD("report cardtype: %d to framework, roaming: %s", cardtype, tmp);
        if (strcmp("1", tmp) == 0) {
            if (cardtype == CARD_NOT_INSERTED) {
                notifyfw = 0;
            }
            property_set(PROPERTY_RIL_CT3G_ROAMING2[mSlotId - 1], "0");
        }
    }
    if (notifyfw != 0) {
        sprintf(tmp, "%d", cardtype);
        if (mSlotId != -1) {
            property_set(PROPERTY_RIL_CDMA_CARD_TYPE[mSlotId - 1], tmp);
            LOGD("set %s to: %s", PROPERTY_RIL_CDMA_CARD_TYPE[mSlotId - 1], tmp);
        } else {
            LOGD("Invalid svlte slotid: %d", mSlotId);
        }
        RIL_onUnsolicitedResponse(RIL_LOCAL_C2K_UNSOL_CDMA_CARD_TYPE_NOTIFY, &cardtype, sizeof(cardtype));
    }
}

void reportCardTypeForURC (int CSlot) {
    int notifyfw = 1;
    char tmp[PROPERTY_VALUE_MAX] = {0};
    int mSlotId = CSlot;
    if (mSlotId != -1) {
        property_get(PROPERTY_RIL_CT3G_ROAMING2[mSlotId - 1], tmp, "0");
        LOGD("report cardtype: %d to framework, roaming: %s", cardtype, tmp);
        if (strcmp("1", tmp) == 0) {
            if (cardtype == CARD_NOT_INSERTED) {
                notifyfw = 0;
            }
            property_set(PROPERTY_RIL_CT3G_ROAMING2[mSlotId - 1], "0");
        }
    }
    if (notifyfw != 0) {
        sprintf(tmp, "%d", cardtype);
        if ((mSlotId != -1) && (cardtype != NEED_TO_INPUT_PIN)) {
            property_set(PROPERTY_RIL_CDMA_CARD_TYPE[mSlotId - 1], tmp);
            LOGD("set %s to: %s", PROPERTY_RIL_CDMA_CARD_TYPE[mSlotId - 1], tmp);
        } else {
            LOGD("Invalid svlte slotid: %d or cardtype == NEED_TO_INPUT_PIN", mSlotId);
        }
        RIL_onUnsolicitedResponse(RIL_LOCAL_C2K_UNSOL_CDMA_CARD_TYPE_NOTIFY, &cardtype, sizeof(cardtype));
    }
}

static void requestGetIMSI(void *data, size_t datalen, RIL_Token t)
{
    SIM_UNUSED(data);
    SIM_UNUSED(datalen);
    int err = 0;
    ATResponse *p_response = NULL;
    char *line = NULL;
    char * responseStr = NULL;

    err = at_send_command_singleline ("AT+VCIMI", "+VCIMI", &p_response, SIM_CHANNEL_CTX);
    if ((err < 0) || (p_response == NULL) || (p_response->success == 0))
    {
        LOGE("GetIMSI error!");
        goto error;
    }

    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err < 0)
    {
        goto error;
    }
    err = at_tok_nextstr(&line, &responseStr);
    if (err < 0)
    {
        goto error;
    }
    RIL_onRequestComplete(t, RIL_E_SUCCESS, responseStr, strlen(responseStr));
    at_response_free(p_response);
    return;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

int iccGetDfFromPath(char *path)
{
    if (path == 0)
        return -1;

    int len = strlen(path);
    if (len < 4) {
        LOGE("%s: invalid path size!", __FUNCTION__);
        return -1; // invalid path
    }

    char MF[5] = { 0 };
    memcpy(MF, path, 4); // get the MF
    if (convertDfId(MF) != 0) {
        LOGE("%s: invalid MF!", __FUNCTION__);
        return -1; // invalid MF
    }
    if (len == 4)
        return 0; // only MF
    if (len % 4 != 0) {
        LOGE("%s: invalid path pattern!", __FUNCTION__);
        return -1; // invalid path
    }

    char DF[5] = { 0 };
    memcpy(DF, path + len - 4, 4); // get the last DF
    int dfid = convertDfId(DF); // convert the DF
    if (dfid < 0) {
        LOGE("%s: invalid DF!", __FUNCTION__);
        return -1; // invalid DF
    }

    return dfid;

}

void makeSimRspFromUsimFcp(unsigned char ** simResponse) {
    int format_wrong = 0;
    unsigned char * fcpByte = NULL;
    unsigned short fcpLen = 0;
    csim_file_descriptor_struct fDescriptor = { 0, 0, 0, 0 };
    csim_file_size_struct fSize = { 0 };
    unsigned char simRspByte[GET_RESPONSE_EF_SIZE_BYTES] = { 0 };
    fcpLen = hexStringToByteArrays(*simResponse, &fcpByte);

    if (FALSE == csim_fcp_query_tag(fcpByte, fcpLen, FCP_FILE_DES_T,
            &fDescriptor)) {
        LOGE("USIM FD Parse fail:%s, fcpByte:%s, fcpLen:%d", *simResponse,
                fcpByte, fcpLen);
        format_wrong = 1;
        goto done;
    }
    if ((!IS_DF_ADF(fDescriptor.fd)) && (FALSE == csim_fcp_query_tag(fcpByte,
            fcpLen, FCP_FILE_SIZE_T, &fSize))) {
        LOGW("USIM File Size fail:%s", *simResponse);
        format_wrong = 1;
        goto done;
    }
    if (IS_DF_ADF(fDescriptor.fd)) {
        simRspByte[RESPONSE_DATA_FILE_TYPE] = TYPE_DF;
        goto done;
    } else {
        simRspByte[RESPONSE_DATA_FILE_TYPE] = TYPE_EF;
    }
    simRspByte[RESPONSE_DATA_FILE_SIZE_1] = (fSize.file_size & 0xFF00) >> 8;
    simRspByte[RESPONSE_DATA_FILE_SIZE_2] = fSize.file_size & 0xFF;
    if (IS_LINEAR_FIXED_EF(fDescriptor.fd)) {
        simRspByte[RESPONSE_DATA_STRUCTURE] = EF_TYPE_LINEAR_FIXED;
        simRspByte[RESPONSE_DATA_RECORD_LENGTH] = fDescriptor.rec_len;
    } else if (IS_TRANSPARENT_EF(fDescriptor.fd)) {
        simRspByte[RESPONSE_DATA_STRUCTURE] = EF_TYPE_TRANSPARENT;

    } else if (IS_CYCLIC_EF(fDescriptor.fd)) {
        simRspByte[RESPONSE_DATA_STRUCTURE] = EF_TYPE_CYCLIC;
        simRspByte[RESPONSE_DATA_RECORD_LENGTH] = fDescriptor.rec_len;
    }

done:
    if (format_wrong != 1) {
        *simResponse = byteArrayToHexString(simRspByte,
                GET_RESPONSE_EF_SIZE_BYTES);
    } else {
        *simResponse = NULL;
        RLOGD("simRsp done, but simRsp is null because command format may be wrong");
    }
}

//Return the cardtype address that firstly matched or null for no match.
static bool isCardTypeExist(const char *cardtype) {
    char tmp[PROPERTY_VALUE_MAX] = { 0 };
    char *value;
    property_get(PROPERTY_RIL_FULL_UICC_TYPE[getCdmaSocketSlotId()-1], tmp, "");
    value = strstr(tmp, cardtype);
    LOGD("%s: %s isExist: %s", PROPERTY_RIL_FULL_UICC_TYPE[getCdmaSocketSlotId()-1], tmp,
            value);
    return (value != NULL);
}

/* Adapt MTK Multi-Application, 2015/01/10 {*/
static void handleSimIo(RIL_SIM_IO_v6 *p_args, RIL_Token t) {
    ATResponse *p_response = NULL;
    RIL_SIM_IO_Response sr;
    int err = -1;
    char* cmd = NULL;
    char *line = NULL;
    int DF = 0;
    bool need_free = false;

    memset(&sr, 0, sizeof(sr));
    if (checkUserLoad() == 0) {
        if (p_args) {
            LOGD("%s: path:%s, fileid:0x%x", __FUNCTION__, p_args->path, p_args->fileid);
        }
    }
    DF = iccGetDfFromPath(p_args->path);
    if (DF < 0) {
        // For FCP test. COMMAND_STATUS doesn't care DF in MD, so set DF to 0 if no path.
        if (p_args->command == COMMAND_STATUS) {
            DF = 0;
        } else {
            goto error;
        }
    }

    // at present, CP needs to use COMMAND_SELECT instead of COMMAND_GET_RESPONSE
    if (p_args->command==COMMAND_GET_RESPONSE) p_args->command=COMMAND_SELECT;
    if ((p_args->fileid == 20283) && (p_args->command == COMMAND_UPDATE_RECORD)) {
        // modify index value.
        p_args->p1 = (((p_args->p1 - 1) % 250) + 1);
    }
    if (p_args->data == NULL) {
        asprintf(&cmd, "AT+CRSM=%d,%d,%d,%d,%d,%d",
                    p_args->command, p_args->fileid, DF,
                    p_args->p1, p_args->p2, p_args->p3);
    } else {
        asprintf(&cmd, "AT+CRSM=%d,%d,%d,%d,%d,%d,\"%s\"",
                    p_args->command, p_args->fileid, DF,
                    p_args->p1, p_args->p2, p_args->p3, p_args->data);
    }

    err = at_send_command_singleline(cmd, "+CRSM:", &p_response, SIM_CHANNEL_CTX);
    free(cmd);

    if (err < 0 || p_response == NULL || p_response->success == 0) goto error;

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &(sr.sw1));
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &(sr.sw2));
    if (err < 0) goto error;

    if (at_tok_hasmore(&line)) {
        err = at_tok_nextstr(&line, &(sr.simResponse));
        if (err < 0) goto error;
    }
    if (p_args->command == COMMAND_SELECT)
    {

        if  (sr.simResponse != NULL && isCardTypeExist("CSIM"))
        {
           makeSimRspFromUsimFcp((unsigned char **) &(sr.simResponse));
           LOGD("[handleSimIo] sr.simResponse2:%s", sr.simResponse);
           need_free = true;
        }

     }
    RIL_onRequestComplete(t, RIL_E_SUCCESS, &sr, sizeof(sr));
     if (p_args->command == COMMAND_SELECT)
     {
        if (sr.simResponse && need_free)
        {
           free(sr.simResponse);
           sr.simResponse = NULL;
        }
     }
    at_response_free(p_response);
    return;

error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}
/* Adapt MTK Multi-Application, 2015/01/10 }*/

static void requestSIM_IO(void *data, size_t datalen, RIL_Token t)
{
    SIM_UNUSED(datalen);
    RIL_SIM_IO_v6 *p_args = (RIL_SIM_IO_v6 *)data;
    if (p_args) {
        handleSimIo(p_args, t);
    } else {
        LOGE("%s: data error!", __FUNCTION__);
    }
}

static void requestQueryFacilityLock(void *data, size_t datalen, RIL_Token t)
{
    SIM_UNUSED(datalen);
    int err = 0, response = 0;
    int fdnServiceResult = -1;
    ATResponse *p_response = NULL;
    char * cmd = NULL;
    char * line = NULL;
    char * facility_string = NULL;
    int ret = RIL_E_GENERIC_FAILURE;
    int count = 0;

    facility_string   = ((char **)data)[0];

    if (0 == strcmp(facility_string, "FD")) {
        // Use AT+ECSTQ=<app>,<service> to query service table
        // 0:  Service not supported
        // 1:  Service supported
        // 2:  Service allocated but not activated
        if (cardtype== 8 || cardtype== 9) {
            // CSIM
            asprintf(&cmd, "AT+ECSTQ=%d,%d", 1, 2);
        } else {
            // UIM
            asprintf(&cmd, "AT+ECSTQ=%d,%d", 0, 3);
        }
        err = at_send_command_singleline(cmd, "+ECSTQ:", &p_response, SIM_CHANNEL_CTX);

        free(cmd);
        cmd = NULL;

        // The same as AOSP. 0 - Available & Disabled, 1-Available & Enabled, 2-Unavailable.
        if (err < 0 || NULL == p_response) {
            RLOGE("Fail to query service table");
        } else if (p_response->success == 0) {
            RLOGE("Fail to query service table");
        } else {
            line = p_response->p_intermediates->line;

            err = at_tok_start(&line);
            if (err < 0) {
                goto error;
            }
            err = at_tok_nextint(&line, &fdnServiceResult);
            if (err < 0) {
                goto error;
            }

            if (fdnServiceResult == 0) {
                fdnServiceResult = 2;
            } else if (fdnServiceResult == 2) {
                fdnServiceResult = 0;
            }

            RLOGD("FDN available: %d", fdnServiceResult);
        }
        at_response_free(p_response);
        p_response = NULL;
    }

    asprintf(&cmd, "AT+CLCK=\"%s\",2", facility_string);
    do {
        err = at_send_command_singleline(cmd,"+CLCK:", &p_response, SIM_CHANNEL_CTX);
        if (err < 0 || p_response == NULL){
            if (cmd != NULL) {
                free(cmd);
                cmd = NULL;
            }
            goto error;
        }

        if (p_response->success == 0) {
            switch (at_get_cme_error(p_response)) {
                case CME_SIM_BUSY:
                    ret = RUIM_BUSY;
                    sleepMsec(200);
                    count++;
                    // To avoid block; if the busy time is too long; need to check modem.
                    if (p_response != NULL) {
                        at_response_free(p_response);
                        p_response = NULL;
                    }
                    if (count == 30) {
                        LOGE("Retry 30 times and still error in requestQueryFacilityLock!");
                        if (cmd != NULL) {
                            free(cmd);
                            cmd = NULL;
                        }
                        goto error;
                    }
                    break;
                default:
                    if (cmd != NULL) {
                        free(cmd);
                        cmd = NULL;
                    }
                    LOGE("p_response->success:%d", p_response->success);
                    goto error;
            }
        } else {
            if (cmd != NULL) {
                free(cmd);
                cmd = NULL;
            }
            LOGD("p_response->success:%d", p_response->success);
            break;
        }
    } while (RUIM_BUSY == ret);

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);

    if (err < 0) {
        goto error;
    }

    err = at_tok_nextint(&line, &response);

    if (err < 0) {
        goto error;
    }

    if (fdnServiceResult == 1 && response == 0) {
        fdnServiceResult = 0;
    }

    if (fdnServiceResult != -1) {
        response = fdnServiceResult;
        RLOGD("final FDN result: %d", response);
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(int));
    at_response_free(p_response);
    return;

error:
    at_response_free(p_response);
    LOGE("ERROR: requestQueryFacilityLock() failed\n");
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static void requestSubscription(void *data, size_t datalen, RIL_Token t)
{
    SIM_UNUSED(data);
    SIM_UNUSED(datalen);
    RIL_Subscription response;
    int ret = RIL_E_SUCCESS;
    int err = 0;
    int skip = 0;
    int sidnidCount = 0;
    int sidnidMaxLen = 20;
    ATLine *p_cur = NULL;
    char *line_mdn = NULL;
    char *line_sidnid = NULL;
    char *line_min = NULL;
    char *line_prl = NULL;
    char *err_str = "-1";
    ATResponse *p_response_mdn = NULL;
    ATResponse *p_response_sidnid = NULL;
    ATResponse *p_response_min = NULL;
    ATResponse *p_response_prl = NULL;

    memset(&response,0,sizeof(response));

    err = at_send_command_multiline ("AT+CNUM?", "+CNUM:", &p_response_mdn, SIM_CHANNEL_CTX);
    if ((err != 0) || (p_response_mdn == NULL) || (p_response_mdn->success == 0))
    {
        switch (at_get_cme_error(p_response_mdn)) {
        case CME_SIM_NOT_INSERTED:
            ret = RIL_E_SIM_ABSENT;
            break;
        default:
            ret = RIL_E_GENERIC_FAILURE;
            break;
        }
        goto error;
    }
    /*Although +CNUM? may response multiline groups when read a UICC card,
    we only need to report the first group to FW.*/
    line_mdn = p_response_mdn->p_intermediates->line;
    err = at_tok_start(&line_mdn);
    if (err < 0) goto error;
        err = at_tok_nextint(&line_mdn, &skip);
    if (err < 0) goto error;
    err = at_tok_nextstr(&line_mdn, &response.mdn);
    if (err < 0) goto error;

    err = at_send_command_multiline ("AT+CSNID?", "+CSNID:", &p_response_sidnid, SIM_CHANNEL_CTX);
    if ((err != 0) || (p_response_sidnid->success == 0))
    {
        switch (at_get_cme_error(p_response_sidnid)) {
        case CME_SIM_NOT_INSERTED:
            ret = RIL_E_SIM_ABSENT;
            break;
        default:
            ret = RIL_E_GENERIC_FAILURE;
            break;
        }
        goto error;
    }
    for (sidnidCount = 0, p_cur = p_response_sidnid->p_intermediates
            ; p_cur != NULL
            ; p_cur = p_cur->p_next
    ) {
        sidnidCount++;
    }
    LOGD("sidnidCount:%d", sidnidCount);
    if (sidnidCount > 0) {
        char **sid = (char **)alloca(sidnidCount * sizeof(char *));
        if (sid == NULL) { goto error; }
        char **nid = (char **)alloca(sidnidCount * sizeof(char *));
        if (nid == NULL) { goto error; }
        int i = 0;
        for (p_cur = p_response_sidnid->p_intermediates
                ; p_cur != NULL
                ; p_cur = p_cur->p_next, i++
        ) {
            line_sidnid = p_cur->line;
            err = at_tok_start(&line_sidnid);
            if (err < 0) goto error;
            err = at_tok_nextstr(&line_sidnid, &sid[i]);
            if (err < 0) goto error;
            err = at_tok_nextstr(&line_sidnid, &nid[i]);
            if (err < 0) goto error;
        }
        char *finalSid = (char *)alloca((sidnidCount * sidnidMaxLen + sidnidCount) * sizeof(char));
        if (finalSid == NULL) { goto error; }
        memset(finalSid, 0, (sidnidCount * sidnidMaxLen + sidnidCount) * sizeof(char));
        char *finalNid = (char *)alloca((sidnidCount * sidnidMaxLen + sidnidCount) * sizeof(char));
        if (finalNid == NULL) { goto error; }
        memset(finalNid, 0, (sidnidCount * sidnidMaxLen + sidnidCount) * sizeof(char));
        for (i = 0; i < sidnidCount; i++) {
            strncat(finalSid, sid[i], sidnidMaxLen);
            strncat(finalSid, ",", 1);
            strncat(finalNid, nid[i], sidnidMaxLen);
            strncat(finalNid, ",", 1);
        }
        if (strlen(finalSid) > 0) {
            finalSid[strlen(finalSid) - 1] = '\0';
        }
        if (strlen(finalNid) > 0) {
            finalNid[strlen(finalNid) - 1] = '\0';
        }
        LOGD("finalSid:%s, finalNid:%s", finalSid, finalNid);
        response.homesid = finalSid;
        response.homenid = finalNid;
    }

    err = at_send_command_singleline ("AT+VMIN?", "+VMIN:", &p_response_min, SIM_CHANNEL_CTX);
    if ((err != 0) || (p_response_min->success == 0))
    {
        switch (at_get_cme_error(p_response_min)) {
        case CME_SIM_NOT_INSERTED:
            ret = RIL_E_SIM_ABSENT;
            break;
        default:
            ret = RIL_E_GENERIC_FAILURE;
            break;
        }
        goto error;
    }
    line_min = p_response_min->p_intermediates->line;
    err = at_tok_start(&line_min);
    if (err < 0) goto error;
    err = at_tok_nextstr(&line_min, &response.min);
    if (err < 0) goto error;

    err = at_send_command_singleline ("AT+VPRLID?", "+VPRLID:", &p_response_prl, SIM_CHANNEL_CTX);
    if ((err != 0) || (p_response_prl->success == 0))
    {
        switch (at_get_cme_error(p_response_prl)) {
        case CME_SIM_NOT_INSERTED:
            ret = RIL_E_SIM_ABSENT;
            break;
        default:
            ret = RIL_E_GENERIC_FAILURE;
            break;
        }
        goto error;
    }
    line_prl = p_response_prl->p_intermediates->line;
    err = at_tok_start(&line_prl);
    if (err < 0) goto error;
    err = at_tok_nextstr(&line_prl, &response.prl_id);
    if (err < 0) goto error;

    RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(response));
    at_response_free(p_response_mdn);
    at_response_free(p_response_sidnid);
    at_response_free(p_response_min);
    at_response_free(p_response_prl);
    return;
error:
    if (ret == RIL_E_SIM_ABSENT) {
        response.mdn = err_str;
        response.homesid = err_str;
        response.homenid = err_str;
        response.min = err_str;
        response.prl_id = err_str;
        RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(response));
    } else {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    }
    at_response_free(p_response_mdn);
    at_response_free(p_response_sidnid);
    at_response_free(p_response_min);
    at_response_free(p_response_prl);
}

static void requestSimTransmitBasic(void *data, size_t datalen, RIL_Token t)
{
    SIM_UNUSED(datalen);
    ATResponse *p_response = NULL;
    RIL_SIM_IO_Response sr;
    RIL_SIM_APDU *p_args;
    int err = -1;
    char* cmd = NULL;
    char *line = NULL;
    int len;

    // AOSP has RIL_SIM_APDU, but c2k uses RIL_SIM_IO_v6
    // requestSimTransmitBasic is used also by MTK request RIL_REQUEST_SIM_TRANSMIT_BASIC
    // but RIL_REQUEST_SIM_TRANSMIT_BASIC phase out
    // RIL_SIM_APDU and RIL_SIM_IO_v6 have a big diff, so if refector it, will be a big effort
    // so keep it first.
    memset(&sr, 0, sizeof(sr));
    p_args = (RIL_SIM_APDU *)data;

    if((p_args->data == NULL) ||(strlen(p_args->data) == 0)) {
        LOGD("%s p3:%d", __FUNCTION__, p_args->p3);

        if(p_args->p3 < 0) {
            asprintf(&cmd, "AT+CGLA=0,%d,\"%02x%02x%02x%02x\"",
                            8, p_args->cla, p_args->instruction,
                            p_args->p1, p_args->p2);
        } else {
            asprintf(&cmd, "AT+CGLA=0,%d,\"%02x%02x%02x%02x%02x\"",
                            10, p_args->cla, p_args->instruction,
                            p_args->p1, p_args->p2, p_args->p3);
        }
    } else {
        asprintf(&cmd, "AT+CGLA=0,%lu,\"%02x%02x%02x%02x%02x%s\"",
        10 + strlen(p_args->data), p_args->cla, p_args->instruction,
        p_args->p1, p_args->p2, p_args->p3,
        p_args->data);
    }

    LOGD("%s command:%s",__FUNCTION__, cmd);
    err = at_send_command_singleline(cmd, "+CGLA:", &p_response, SIM_CHANNEL_CTX);
    free(cmd);

    if (err < 0 || p_response->success == 0) {
        LOGD("CGLA Send Error");
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &len);
    if (err < 0) goto error;

    err = at_tok_nextstr(&line, &(sr.simResponse));
    if (err < 0) goto error;

    sscanf(&(sr.simResponse[len - 4]), "%02x%02x", &(sr.sw1), &(sr.sw2));
    sr.simResponse[len - 4] = '\0';

    LOGD("%s:sr.sw1:%02x, sr.sw2:%02x, sr.simResponse:%s", __FUNCTION__, sr.sw1, sr.sw2, sr.simResponse);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, &sr, sizeof(sr));
    at_response_free(p_response);
    p_response = NULL;

    // end sim toolkit session if 90 00 on TERMINAL RESPONSE
    if((p_args->instruction == 20) && (sr.sw1 == 0x90))
        RIL_onUnsolicitedResponse(RIL_UNSOL_STK_SESSION_END, NULL, 0);

    // return if no sim toolkit proactive command is ready
    if(sr.sw1 != 0x91)
        return;

    asprintf(&cmd, "AT+CGLA=0,10,\"a0120000%02x\"", sr.sw2);
    LOGD("%s fetch cmd = %s", __FUNCTION__, cmd);
    err = at_send_command_singleline(cmd, "+CGLA:", &p_response, SIM_CHANNEL_CTX);
    free(cmd);

    if (err < 0 || p_response->success == 0) {
        goto fetch_error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &len);
    if (err < 0) goto error;

    err = at_tok_nextstr(&line, &(sr.simResponse));
    if (err < 0) goto error;

    sscanf(&(sr.simResponse[len - 4]), "%02x%02x", &(sr.sw1), &(sr.sw2));
    sr.simResponse[len - 4] = '\0';

    LOGD("%s, fetch len = %d %02x, %02x", __FUNCTION__, len, sr.sw1, sr.sw2);

    if(strlen(sr.simResponse) > 0) {
        RIL_onUnsolicitedResponse(RIL_UNSOL_STK_PROACTIVE_COMMAND, sr.simResponse, strlen(sr.simResponse));
        goto fetch_error;
    }

error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
fetch_error:
    at_response_free(p_response);
}

// Hex To Int
unsigned int htoi(char* szHex) {
    unsigned int hex = 0;
    int nibble;
    while (*szHex) {
        hex <<= 4;

        if (*szHex >= '0' && *szHex <= '9') {
            nibble = *szHex - '0';
        } else if (*szHex >= 'a' && *szHex <= 'f') {
            nibble = *szHex - 'a' + 10;
        } else if (*szHex >= 'A' && *szHex <= 'F') {
            nibble = *szHex - 'A' + 10;
        } else {
            nibble = 0;
        }

        hex |= nibble;

        szHex++;
    }

    return hex;
}


static void requestSimOpenNFCChannel(void *data, size_t datalen, RIL_Token t)
{
    SIM_UNUSED(datalen);
    int err = 0;
    int len = 0;
    ATResponse *p_response = NULL;
    char *cmd = NULL;
    int sessionid, channel = -1;
    int status[3] = {0};
    RIL_SIM_IO_Response sr;

    RIL_OpenChannelParams *openChannelParams = NULL;
    char *pAid = NULL;
    int p2 = 0x00;

    openChannelParams = (RIL_OpenChannelParams *)data;

    pAid = openChannelParams->aidPtr;
    if (openChannelParams->p2 > 0) {
        p2 = openChannelParams->p2;
    }

    LOGD("%s, aid is %s, p2:%d", __FUNCTION__, pAid, openChannelParams->p2);

    // If p2 is invalid as TS 102 221 Table 11.2 defined.
    if ((pAid == NULL) && (p2 != 0) && ((p2 & 0x80) > 0 || ((p2 & 0x40) > 0 && (p2 & 0x20) > 0)
            || ((p2 & 0x40) == 0 && (p2 & 0x20) > 0) || (p2 & 0x10) > 0 || (p2 & 0x04) == 0)) {
        LOGI("requestSimOpenNFCChannel, invalid p2");
        sr.sw1 = 0xff;
        sr.sw2 = 0xff;
        sr.simResponse = NULL;
        err = RIL_E_GENERIC_FAILURE;
        goto error;
    }

    memset(&sr, 0, sizeof(RIL_SIM_IO_Response));
    // Open logical Channel - START
    // AT+CGLA=<sessionid>,<length>,<command>
    // <sessionid> = 0
    // <command> = 00 70 00 00 01
    err = requestCGLA_OpenChannelWithSw(&sr, &len, 0, 10, 0x00, 0x70, 0x00, 0x00, 0x01, NULL, t);
    LOGD("requestSimOpenNFCChannel: Open logical Channel: err = %d, len = %d %02x, %02x", err, len, sr.sw1, sr.sw2);

    // if sw1 == sw2 == 0xff, it means that open logical channel fail
    if (err == RIL_E_SUCCESS) {
        // Support Extended Channel - 4 to 19 (0x40 ~ 0x4F)
        if (sr.simResponse != NULL) {
            channel = htoi(sr.simResponse);
        } else {
            channel = 0;
            status[0] = 0;
            sr.sw1 = 0xff;
            sr.sw2 = 0xff;
            goto error;
        }
        LOGD("requestSimOpenNFCChannel - channel = %x", channel);
        status[0] = channel;
        if (channel < 1 || channel > 19) {
            channel = 0;
            status[0] = 0;
            sr.sw1 = 0xff;
            sr.sw2 = 0xff;
            goto error;
        }
    } else if (err == RIL_E_NO_SUCH_ELEMENT || err == RIL_E_MISSING_RESOURCE) {
        status[1] = sr.sw1;
        status[2] = sr.sw2;
        goto error;
    } else {
        sr.sw1 = 0xff;
        sr.sw2 = 0xff;
        goto error;
    }

    sessionid = channel + 1;
    // Open logical Channel - END

    // Select AID
    // AT+CGLA=<sessionid>,<length>,<command>
    // <command> = 0X A4 04 00 length DATA
    // X = channel
    if (sr.simResponse != NULL) {
        free(sr.simResponse);
        sr.simResponse = NULL;
    }

    at_response_free(p_response);
    p_response = NULL;

    if (pAid != NULL && (int)(strlen((char *)pAid)/2) > 0) { // To check NULL AID
        if(channel > 3) {
            LOGD("channel > 3");
            err = requestCGLA_OpenChannelWithSw(&sr, &len,
                sessionid, 10 + strlen(pAid), 0x40 |(channel-4), 0xA4, 0x04, 0x04,
                (int)(strlen(pAid)/2), (char **)(&pAid), t);
        } else {
            LOGD("channel <= 3");
            err = requestCGLA_OpenChannelWithSw(&sr, &len,
                sessionid, 10 + strlen(pAid), channel, 0xA4, 0x04, 0x04,
                (int)(strlen(pAid)/2), (char **)(&pAid), t);
        }
    } else {
        LOGD("requestCGLA_OpenChannelWithSw - channel = %x with NULL AID", channel);
        err = RIL_E_SUCCESS;
    }


    if (err != RIL_E_SUCCESS) {
        LOGD("Select AID AT-CMD failed, and then close logical session %d", sessionid);
        status[0] = 0;
        status[1] = sr.sw1;
        status[2] = sr.sw2;
        goto error_select_aid;
    } else if (err == RIL_E_SUCCESS) {
        if ((sr.sw1 == 0x6A && sr.sw2 == 0x82) ||
            (sr.sw1 == 0x69 && sr.sw2 == 0x85) ||
            (sr.sw1 == 0x69 && sr.sw2 == 0x99)) {
            LOGE("Select AID, File not found");
            if (sr.simResponse != NULL) {
                free(sr.simResponse);
                sr.simResponse = NULL;
            }
            RIL_onRequestComplete(t, RIL_E_NO_SUCH_ELEMENT, NULL, 0);
            return;
        } else if ((sr.sw1 == 0x6A && sr.sw2 == 0x84) ||
                   (sr.sw1 == 0x6A && sr.sw2 == 0x81) ||
                   (sr.sw1 == 0x68 && sr.sw2 == 0x81)) {
            LOGE("Select AID, Not enough memory space in the file");
            if (sr.simResponse != NULL) {
                free(sr.simResponse);
                sr.simResponse = NULL;
            }
            RIL_onRequestComplete(t, RIL_E_MISSING_RESOURCE, NULL, 0);
            return;
        }
    }

    if (sr.simResponse != NULL) {
        free(sr.simResponse);
        sr.simResponse = NULL;
    }

    status[0] = sessionid - 1;
    status[1] = sr.sw1;
    status[2] = sr.sw2;

    RIL_onRequestComplete(t, RIL_E_SUCCESS,  status, 3*sizeof(int));
    return;

error_select_aid:
    // Close logical Channel if selecting AID is failed
    asprintf(&cmd, "AT+CCHC=%d", sessionid);
    at_send_command(cmd, NULL, SIM_CHANNEL_CTX);
    free(cmd);
    goto error;

error:
    at_response_free(p_response);
    if (sr.simResponse != NULL) {
        free(sr.simResponse);
        sr.simResponse = NULL;
    }
    if (err == RIL_E_NO_SUCH_ELEMENT || err == RIL_E_MISSING_RESOURCE) {
        RIL_onRequestComplete(t, err, NULL, 0);
    } else {
        RIL_onRequestComplete(t, err, status, 3*sizeof(int));
    }
}

/*
*function:Close UICC NFC
*argument from framework: sessionid(get from CCHO )
*return to framework:SUCCESS or GENERIC_FAILURE
*/
static void requestSimCloseNFCChannel(void *data, size_t datalen, RIL_Token t)
{
    SIM_UNUSED(datalen);
    int channelid = ((int *)data)[0];
    char *cmd = NULL;
    int err = 0;
    ATResponse *p_response = NULL;

    LOGD("%s, channelid is %d", __FUNCTION__, channelid);
    asprintf(&cmd, "AT+CCHC=%d", channelid + 1);    //CCHC using sessionid
    err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
    free(cmd);
    if (err < 0) {
        goto error;
    }
    if (p_response != NULL && p_response->success == 0) {
        err = RIL_E_GENERIC_FAILURE;
        if (p_response->finalResponse != NULL) {
            if (!strcmp(p_response->finalResponse,
                    "+CME ERROR: INCORRECT PARAMETERS")) {
                err = RIL_E_GENERIC_FAILURE;
            }
            if (*(int *)data == 0) {
                err = RIL_E_INVALID_ARGUMENTS;
            }
        }
        goto error;
    }
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);
    return;
error:
    RIL_onRequestComplete(t, err, NULL, 0);
    at_response_free(p_response);
}

static void requestSimTransmitChannel(void *data, size_t datalen, RIL_Token t)
{
    SIM_UNUSED(datalen);
    ATResponse *p_response = NULL;
    RIL_SIM_IO_Response sr;
    RIL_SIM_APDU *p_args;
    int err = -1;
    char* cmd = NULL;
    char *line = NULL;
    int len = 0;
    int channel = -1;
    int cla = -1;

    memset(&sr, 0, sizeof(sr));
    p_args = (RIL_SIM_APDU *)data;
    channel = p_args->sessionid;
    cla = p_args->cla;
    p_args->sessionid++;    // We return channelid to upper layer
    LOGD("%s, sessionid:%d, cla:%02x, ins:%02x, p1:%02x, p2:%02x, p3:%02x, data:%s", __FUNCTION__,
            p_args->sessionid, p_args->cla, p_args->instruction, p_args->p1, p_args->p2, p_args->p3, p_args->data);

    if (channel < 4) {
        // b7 = 0 indicates the first interindustry class byte coding
        cla = ((cla & 0xBC) | channel);

    } else if (channel < 20) {
        // b7 = 1 indicates the further interindustry class byte coding
        bool isSM = (cla & 0x0C) != 0;
        cla = ((cla & 0xB0) | 0x40 | (channel - 4));
        if (isSM) {
            cla |= 0x20;
        }
    }

    if ((p_args->data == NULL) || (strlen(p_args->data) == 0)) {
        if (p_args->p3 < 0) {
            asprintf(&cmd, "AT+CGLA=%d,%d,\"%02x%02x%02x%02x\"",
                            p_args->sessionid,
                            8, cla, p_args->instruction,
                            p_args->p1, p_args->p2);
            LOGD("%s, command:%s", __FUNCTION__, cmd);
            err = at_send_command_singleline(cmd, "+CGLA:", &p_response, SIM_CHANNEL_CTX);
            free(cmd);

            if (err < 0 || p_response == NULL || p_response->success == 0) {
                LOGD("CGLA Send Error");
                goto error;
            }

            line = p_response->p_intermediates->line;

            err = at_tok_start(&line);
            if (err < 0) goto error;

            err = at_tok_nextint(&line, &len);
            if (err < 0) goto error;

            err = at_tok_nextstr(&line, &(sr.simResponse));
            if (err < 0) goto error;

            sscanf(&(sr.simResponse[len - 4]), "%02x%02x", &(sr.sw1), &(sr.sw2));
            sr.simResponse[len - 4] = '\0';
        } else {
            err = requestCGLA_OpenChannelWithSw(&sr, &len,
                p_args->sessionid, 10, cla, p_args->instruction,
                p_args->p1, p_args->p2, p_args->p3, NULL, t);
            LOGD("requestCGLA_OpenChannelWithSw for data is null and p3 >= 0, err = %d", err);
        }
    } else {
        err = requestCGLA_OpenChannelWithSw(&sr, &len,
            p_args->sessionid, 10 + ((p_args->p3)*2), cla,
            p_args->instruction, p_args->p1, p_args->p2,
            p_args->p3, &(p_args->data), t);
        LOGD("requestCGLA_OpenChannelWithSw for data is not null, err = %d", err);
        if (err != RIL_E_SUCCESS) {
            // FIXME: Align GSM, not deal with RIL_E_MISSING_RESOURCE & RIL_E_NO_SUCH_ELEMENT
            LOGD("%s, sr.sw1:%02x, sr.sw2:%02x, sr.simResponse:%s", __FUNCTION__, sr.sw1, sr.sw2, sr.simResponse);
            if ((err == RIL_E_NO_SUCH_ELEMENT) && (sr.sw1 == 0x6A && sr.sw2 == 0x82)) {
                // Do nothing, for VTS
            } else {
                goto error;
            }
        }
    }

    LOGD("%s, sr.sw1:%02x, sr.sw2:%02x, sr.simResponse:%s", __FUNCTION__, sr.sw1, sr.sw2, sr.simResponse);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, &sr, sizeof(sr));
    if (p_args->p3 < 0) {
        at_response_free(p_response);
        p_response = NULL;
    } else {
        if (sr.simResponse != NULL) {
            free(sr.simResponse);
            sr.simResponse = NULL;
        }
    }

    // end sim toolkit session if 90 00 on TERMINAL RESPONSE
    if ((p_args->instruction == 20) && (sr.sw1 == 0x90)) {
        RIL_onUnsolicitedResponse(RIL_UNSOL_STK_SESSION_END, NULL, 0);
    }

    // return if no sim toolkit proactive command is ready
    if (sr.sw1 != 0x91) {
        return;
    }

    asprintf(&cmd, "a0120000%02x", sr.sw2);
    LOGD("%s fetch cmd = %s", __FUNCTION__, cmd);
    err = requestSimChannelAccess(0, cmd, &sr);
    free(cmd);

    if (err != RIL_E_SUCCESS) {
        LOGD("%s, requestSimChannelAccess return %d", __FUNCTION__, err);
        goto error;
    }

    if (strlen(sr.simResponse) > 0) {
        LOGD("%s fetch success", __FUNCTION__);
        RIL_onUnsolicitedResponse(RIL_UNSOL_STK_PROACTIVE_COMMAND, sr.simResponse, strlen(sr.simResponse));
        return;
    }

error:
    LOGD("%s, TransmitChannel error", __FUNCTION__);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

/*
*function:get ATR for NFC function
*argument from framework: 0
*return to framework:SUCCESS or GENERIC_FAILURE
*/
static void requestSimGetATRFORNFC(void *data, size_t datalen, RIL_Token t)
{
    SIM_UNUSED(data);
    SIM_UNUSED(datalen);
    int err = 0;

    ATResponse *p_response = NULL;
    char *line = NULL;
    char * responseStr = NULL;

    err = at_send_command_singleline ("AT+ESIMINFO=0", "+ESIMINFO", &p_response, SIM_CHANNEL_CTX);
    if (err < 0 || p_response == NULL || p_response->success == 0)
    {
        LOGE("GetATR error!");
        goto error;
    }

    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err < 0)
    {
        goto error;
    }
    err = at_tok_nextstr(&line, &responseStr);
    if (err < 0)
    {
        goto error;
    }
    RIL_onRequestComplete(t, RIL_E_SUCCESS, responseStr, sizeof(char *));
    at_response_free(p_response);
    return;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

/*
 *function:open NFC Channel With SW
 *argument from framework: 0
 *return to framework:SUCCESS or GENERIC_FAILURE
 */
/*
static void requestSimOpenNFCChannelWithSw(void *data, size_t datalen, RIL_Token t) {
    SIM_UNUSED(datalen);
    int err = 0;
    ATResponse *p_response = NULL;
    char* cmd = NULL;
    RIL_SIM_IO_Response sr;
    int len = 0;
    int NFCchannel = 0;
    int srResponseMaxLen = 600 - 1;
    char srResponse[600] = { '\0' };
    int nfcp2 = 0;
    int status[3] = {0};

    memset(&sr, 0, sizeof(sr));

    LOGD("%s, data is %s", __FUNCTION__, (char*) data);

    // open logical Channel
    // AT+CGLA=<sessionid>,<length>,<command>
    // <sessionid> = 0
    // <command> = 00 70 00 00 01
    err = requestCGLA_OpenChannelWithSw(&sr, &len, 0, 10, 0x00, 0x70, 0x00,
            0x00, 0x01, NULL, t);

    LOGD("%s, open logical channel, len:%d, sw1:%02x, sw2:%02x, str:%s",
            __FUNCTION__, len, sr.sw1, sr.sw2, sr.simResponse);

    //if get NFC channel failed, set sr.sw1 = 0xff, sr.sw2 = 0xff
    if (err == RIL_E_SUCCESS) {
        NFCchannel = atoi(sr.simResponse);
        LOGD("%s, NFCchannel:%d", __FUNCTION__, NFCchannel);
        if (NFCchannel < 1 || NFCchannel > 3) {
            LOGD("%s:open channel failed", __FUNCTION__);
            NFCchannel = 0;
            status[0] = 0;
            sr.sw1 = 0xff;
            sr.sw2 = 0xff;
            goto error;
        } else {
            NFCchannel++;
            status[0] = NFCchannel;
        }
    } else if (err == RIL_E_NO_SUCH_ELEMENT || err == RIL_E_MISSING_RESOURCE) {
        status[1] = sr.sw1;
        status[2] = sr.sw2;
        goto error;
    } else {
        LOGD("%s, open channel failed", __FUNCTION__);
        sr.sw1 = 0xff;
        sr.sw2 = 0xff;
        goto error;
    }

    // Select AID
    // AT+CGLA=<sessionid>,<length>,<command>
    // <command> = 0X A4 04 00 length DATA
    // X = channel
    if (sr.simResponse != NULL) {
        free(sr.simResponse);
        sr.simResponse = NULL;
    }
    //some application need p2 = 0x04, others need p2=0x00
    if (!strcmp(data, "a000000063504b43532d3135")) {
        nfcp2 = 0x04;
    } else {
        nfcp2 = 0x00;
    }
    err = requestCGLA_OpenChannelWithSw(&sr, &len, NFCchannel,
            10 + strlen((char *) data), NFCchannel, 0xA4, 0x04, nfcp2,
            (int) (strlen((char *) data) / 2), (char **) (&data), t);

    LOGD("%s, select AID, len:%d, sw1:%02x, sw2:%02x, str:%s",
            __FUNCTION__, len, sr.sw1, sr.sw2, sr.simResponse);

    //Select AID failed, close channel
    if (err != RIL_E_SUCCESS) {
        LOGD("%s, select AID failed, close channel:%d", __FUNCTION__,
                NFCchannel);
        goto select_error;
    } else if (err == RIL_E_SUCCESS) {
        LOGD("%s, select AID success", __FUNCTION__);
        if ((sr.sw1 == 0x6A && sr.sw2 == 0x82) ||
            (sr.sw1 == 0x69 && sr.sw2 == 0x85) ||
            (sr.sw1 == 0x69 && sr.sw2 == 0x99)) {
            RLOGE("Select AID, file not found");
            if (sr.simResponse != NULL) {
                free(sr.simResponse);
                sr.simResponse = NULL;
            }
            RIL_onRequestComplete(t, RIL_E_NO_SUCH_ELEMENT, NULL, 0);
            return;
        } else if ((sr.sw1 == 0x6A && sr.sw2 == 0x84) ||
                   (sr.sw1 == 0x6A && sr.sw2 == 0x81) ||
                   (sr.sw1 == 0x68 && sr.sw2 == 0x81)) {
            RLOGE("Select AID, not enough memory space in the file");
            if (sr.simResponse != NULL) {
                free(sr.simResponse);
                sr.simResponse = NULL;
            }
            RIL_onRequestComplete(t, RIL_E_MISSING_RESOURCE, NULL, 0);
            return;
        }
        goto no_error;
    }

select_error:
//asprintf(&cmd, "AT+CGLA=0,%d,\"%02x%02x%02x%02x\"",
//                               8, 0x00, 0x70, 0x80, NFCchannel);
     //   p1 = 0x00 open channel
     //   p2 = 0x80 close channel
    asprintf(&cmd, "AT+CGLA=0,%d,\"%02x%02x%02x%02x%02x\"", 10, 0x00, 0x70,
            0x80, NFCchannel, 0x00);
    err = at_send_command_singleline(cmd, "+CGLA:", &p_response, SIM_CHANNEL_CTX);
    LOGD("Select AID failed, close channel:%d", NFCchannel);

    if (err < 0 || p_response == NULL || p_response->success == 0) {
        LOGD("%s, close channel failed", __FUNCTION__);
        err = RIL_E_GENERIC_FAILURE;
    } else {
        NFCchannel = 0;
        status[0] = 0;
    }
    goto error;

no_error:
    srResponse[0] = (char) '0';
    srResponse[1] = (char) (NFCchannel + ((int) '0'));
    srResponse[2] = (char) '\0';

    if ((sr.simResponse != NULL) && (strlen(sr.simResponse) > 0)) {
        strncat(srResponse, sr.simResponse, srResponseMaxLen - strlen(srResponse));
    }

    if (sr.simResponse != NULL) {
        free(sr.simResponse);
        sr.simResponse = NULL;
    }

    sr.simResponse = srResponse;
    status[1] = sr.sw1;
    status[2] = sr.sw2;
    RLOGD("%s succeed, channel:%d, sw1:%02x, sw2:%02x", __FUNCTION__, status[0], status[1], status[2]);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, status, 3 * sizeof(int));
    return;

error:
    srResponse[0] = (char) '0';
    srResponse[1] = (char) '0';
    srResponse[2] = (char) '\0';

    if (sr.sw1 != 0xff && sr.sw2 != 0xff && sr.simResponse != NULL
            && strlen(sr.simResponse) > 0) {
        strncat(srResponse, sr.simResponse, srResponseMaxLen - strlen(srResponse));
    }

    if (sr.simResponse != NULL) {
        free(sr.simResponse);
        sr.simResponse = NULL;
    }

    sr.simResponse = srResponse;
    status[1] = sr.sw1;
    status[2] = sr.sw2;
    RLOGD("%s fail, channel:%d, sw1:%02x, sw2:%02x", __FUNCTION__, status[0], status[1], status[2]);
    if ((status[1] == 0x6A && status[2] == 0x82) ||
        (status[1] == 0x69 && status[2] == 0x85) ||
        (status[1] == 0x69 && status[2] == 0x99)) {
        RIL_onRequestComplete(t, RIL_E_NO_SUCH_ELEMENT, NULL, 0);
    } else if ((status[1] == 0x6A && status[2] == 0x84) ||
               (status[1] == 0x6A && status[2] == 0x81) ||
               (status[1] == 0x68 && status[2] == 0x81)) {
        RIL_onRequestComplete(t, RIL_E_MISSING_RESOURCE, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, status, 3 * sizeof(int)); // assume RIL_E_SUCCESS in order to pass parameter
    }

    if (p_response != NULL) {
        at_response_free(p_response);
    }

    if (cmd != NULL) {
        free(cmd);
    }
    return;
} */

static int checkRetryFCI(int sw1, int sw2) {
    int retry = 0;
    if (sw1 != 0x61 && sw1 != 0x91 && (sw1 != 0x63 && sw2 != 0x10) && (sw1 != 0x92 && sw2 != 0x40)
            && sw1 != 0x9F && (sw1 != 0x90 && sw2 != 0x00)) {
        retry = 1;
    }
    RLOGD("%s, sw1:%02x, sw2:%02x, retry:%d", __FUNCTION__, sw1, sw2, retry);
    return retry;
}

static int requestCGLA_OpenChannelWithSw(RIL_SIM_IO_Response *sr, int *len, int NFCchannel,
                      int length, int cla, int ins, int p1, int p2, int p3, char **data, RIL_Token t)
{
    LOGD("%s:channel:%d, length:%d, cla:%02x, ins:%02x, p1:%02x, p2:%02x, p3:%02x",
            __FUNCTION__, NFCchannel, length, cla, ins, p1, p2, p3);
    int err = 0;
    ATResponse *p_response = NULL;
    char *line = NULL;
    char *cmd = NULL;
    int srResponseBufferSize = 600;
    char *srResponse = (char *)malloc(sizeof(char) * srResponseBufferSize);
    assert(srResponse != NULL);

redo:
    *len = 0;
    memset(srResponse, 0, srResponseBufferSize);
    memset(sr, 0, sizeof(*sr));

    if (data == NULL) {
        if (ins == 0xA4 && p2 == 0x04) {
            asprintf(&cmd, "AT+CGLA=%d,%d,\"%02x%02x%02x%02x%02x00\"",
                                  NFCchannel, length+2, cla, ins, p1, p2, p3);
        } else {
            asprintf(&cmd, "AT+CGLA=%d,%d,\"%02x%02x%02x%02x%02x\"",
                                  NFCchannel, length, cla, ins, p1, p2, p3);
        }
    } else {
        if (ins == 0xA4 && p2 == 0x04) {
            asprintf(&cmd, "AT+CGLA=%d,%d,\"%02x%02x%02x%02x%02x%s00\"",
                                  NFCchannel, length+2, cla, ins, p1, p2, p3, *data);
        } else {
            asprintf(&cmd, "AT+CGLA=%d,%d,\"%02x%02x%02x%02x%02x%s\"",
                                  NFCchannel, length, cla, ins, p1, p2, p3, *data);
        }
    }

    err = at_send_command_singleline (cmd, "+CGLA:", &p_response, SIM_CHANNEL_CTX);

    if (err < 0 || p_response == NULL || p_response->success == 0) {
        LOGE("%s, err = %d", __FUNCTION__, err);
        err = RIL_E_GENERIC_FAILURE;
        if (p_response != NULL && p_response->finalResponse != NULL) {
            if (!strcmp(p_response->finalResponse, "+CME ERROR: MEMORY FULL")) {
                err = RIL_E_MISSING_RESOURCE;
            }
            if (!strcmp(p_response->finalResponse, "+CME ERROR: NOT FOUND")) {
                err = RIL_E_NO_SUCH_ELEMENT;
            }
        }
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) {
        LOGD("%s: failed to at_tok_start", __FUNCTION__);
        goto error;
    }

    err = at_tok_nextint(&line, len);
    if (err < 0) {
        LOGD("%s: failed to at_tok_nextint", __FUNCTION__);
        goto error;
    }

    err = at_tok_nextstr(&line, &(sr->simResponse));
    if (err < 0) {
        LOGD("%s: failed to at_tok_nextstr", __FUNCTION__);
        goto error;
    }

    sscanf(&(sr->simResponse[*len - 4]), "%02x%02x", &(sr->sw1), &(sr->sw2));
    sr->simResponse[*len - 4] = '\0';
    strncpy(srResponse, sr->simResponse, srResponseBufferSize - 1);
    sr->simResponse = srResponse;

    at_response_free(p_response);
    p_response = NULL;
    free(cmd);
    cmd = NULL;

    if (sr->sw1 == 0x61) {
        length = 10;
        ins = 0xC0;
        p1 = 0x00;
        p2 = 0x00;
        p3 = sr->sw2;
        data = NULL;
        goto redo;
    } else if (sr->sw1 == 0x6c) {
        p3 = sr->sw2;
        goto redo;
    } else if ((sr->sw1 == 0x6A && sr->sw2 == 0x82) ||
                (sr->sw1 == 0x69 && sr->sw2 == 0x85) ||
                (sr->sw1 == 0x69 && sr->sw2 == 0x99)) {
        RLOGE("File not found");
        return RIL_E_NO_SUCH_ELEMENT;
    } else if ((sr->sw1 == 0x6A && sr->sw2 == 0x84) || (sr->sw1 == 0x6A && sr->sw2 == 0x81)) {
        RLOGE("Not enough memory space in the file");
        return RIL_E_MISSING_RESOURCE;
    } else if (sr->sw1 != 0x90 && sr->sw1 != 0x91) { // wrong sw1, sw2
        if (ins == 0xA4 && data != NULL && p2 == 0x04) {
            // Retry only for "Select AID"
            if (checkRetryFCI(sr->sw1, sr->sw2) == 1) {
                p2 = 0x00;
                goto redo;
            }
        }
        // GSMA NFC test: We should treat warning code 0x62 or 0x63 as
        // success.
        if (sr->sw1 == 0x62 || sr->sw1 == 0x63) {
            LOGD("%s, requestCGLA_OpenChannelWithSw: 0x62 || 0x63 warning code.", __FUNCTION__);
        } else {
            return RIL_E_GENERIC_FAILURE;
        }
    }

    return RIL_E_SUCCESS;

error:
    sr->sw1 = 0x6f;
    sr->sw2 = 0x00;
    sr->simResponse = NULL;
    at_response_free(p_response);
    free(cmd);
    if (srResponse != NULL) {
        free(srResponse);
        srResponse = NULL;
    }
    return err;
}

static int requestSimChannelAccess(int sessionid, char * senddata, RIL_SIM_IO_Response * output)
{
    ATResponse *p_response = NULL;
    char *cmd = NULL;
    char *line = NULL;
    int len = 0;
    int err = 0;

    LOGD("%s sessionid:%d, senddata:%s", __FUNCTION__, sessionid, senddata);

    if(senddata == NULL) {
        err = AT_ERROR_GENERIC;
        return err;
    }

    len = strlen(senddata);
    asprintf(&cmd,  "AT+CGLA=%d,%d,\"%s\"", sessionid, len, senddata);
    err = at_send_command_singleline(cmd, "+CGLA:", &p_response, getChannelCtxbyProxy());
    free(cmd);

    if (err < 0 || p_response == NULL || p_response->success == 0) {
        LOGD("%s, AT+CGLA send failed", __FUNCTION__);
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &len);
    if (err < 0) goto error;

    err = at_tok_nextstr(&line, &(output->simResponse));
    if (err < 0) goto error;

    sscanf(&(output->simResponse[len - 4]), "%02x%02x", &(output->sw1), &(output->sw2));
    output->simResponse[len - 4] = '\0';
    at_response_free(p_response);
    return RIL_E_SUCCESS;

error:
    at_response_free(p_response);
    return RIL_E_GENERIC_FAILURE;
}

static void requestSendUTKURCREG(void* param)
{
    SIM_UNUSED(param);
    /*
     * !beacause we send UTKURCREG in unsoilicited thread
     * !the result of this command can not returned to framework
     * !so we IGNOR the result of this command
     *
     **/
    at_send_command("AT+UTKURCREG", NULL, getChannelCtxbyProxy());
}

RUIM_Status getUIMStatus(RIL_Token t) {
    ATResponse *p_response = NULL;
    int err;
    int ret;
    char *cpinLine;
    char *cpinResult;
    PS_PARAM psParam;
    memset(&psParam, 0, sizeof(psParam));
    char boottimes[PROPERTY_VALUE_MAX] = { 0 };
    LOGD("getUIMStatus sState = %d", getRadioState());
    property_get("vendor.net.cdma.boottimes", boottimes, "0");
    if ((getRadioState() == RADIO_STATE_UNAVAILABLE) && (strcmp(boottimes, "2") != 0)) {
        ret = RUIM_NOT_READY;
        goto done;
    }

    LOGD("getUIMStatus1");
    err = at_send_command_singleline("AT+CPIN?", "+CPIN:", &p_response, SIM_CHANNEL_CTX);
    LOGD("getUIMStatus2");
    if (err != 0) {
        ret = RUIM_NOT_READY;
        goto done;
    }
    switch (at_get_cme_error(p_response)) {
        case CME_SUCCESS:
            break;

        case CME_SIM_NOT_INSERTED:
            ret = RUIM_ABSENT;
#ifndef OPEN_PS_ON_CARD_ABSENT
            // psParam.enable = 0;
            // turnPSEnable((void *)&psParam);
#endif /* OPEN_PS_ON_CARD_ABSENT */
            goto done;
        case CME_SIM_FAILURE:
            ret = RUIM_NOT_READY;
            goto done;
        case CME_SIM_BUSY:
            ret = RUIM_BUSY;
            goto done;
        default:
            ret = RUIM_NOT_READY;
            goto done;
    }

    /* CPIN? has succeeded, now look at the result */

    cpinLine = p_response->p_intermediates->line;
    err = at_tok_start(&cpinLine);

    if (err < 0) {
        ret = RUIM_NOT_READY;
        goto done;
    }

    err = at_tok_nextstr(&cpinLine, &cpinResult);

    if (err < 0) {
        ret = RUIM_NOT_READY;
        goto done;
    }

    if (0 == strcmp(cpinResult, "SIM PIN")) {
        ret = RUIM_PIN;
        // LOGD("sState = %d",sState);
        // setRadioState(RADIO_STATE_RUIM_LOCKED_OR_ABSENT);
#ifndef OPEN_PS_ON_CARD_ABSENT
        // if (getRadioState() == RADIO_STATE_RUIM_LOCKED_OR_ABSENT) {
        //    psParam.enable = 0;
        //    turnPSEnable((void *)&psParam);
        // }
#endif /* OPEN_PS_ON_CARD_ABSENT */
        goto done;
    }
    else if (0 == strcmp(cpinResult, "SIM PUK")) {
#ifndef OPEN_PS_ON_CARD_ABSENT
        // if (getRadioState() == RADIO_STATE_RUIM_LOCKED_OR_ABSENT) {
        //     psParam.enable = 0;
        //     turnPSEnable((void *)&psParam);
        // }
#endif /* OPEN_PS_ON_CARD_ABSENT */
        ret = RUIM_PUK;
        goto done;
    }
    else if (0 == strcmp(cpinResult, "UIM Error")) {
        LOGD("UIM Error");
        ret = RUIM_NOT_READY;  // cwen add uim error notification that UI not display no card
        goto done;
    }
    else if (0 != strcmp(cpinResult, "READY")) {
        /* we're treating unsupported lock types as "sim absent" */
        ret = RUIM_ABSENT;
        goto done;
    }

    at_response_free(p_response);
    p_response = NULL;
    cpinResult = NULL;

    /* ALPS01977096: To align with GSM to set READY if Radio is NOT UNAVAILABLE, 2015/03/12 {*/
    // TODO: To complete align with GSM to remove Radio ON check
    LOGD("%s: Radio state: %d", __FUNCTION__, getRadioState());
    if (RADIO_STATE_UNAVAILABLE == getRadioState()) {
        LOGD("%s: RADIO_STATE_UNAVAILABLE", __FUNCTION__);
        ret = RUIM_NOT_READY;
    } else {
        LOGD("%s:UIM_READY", __FUNCTION__);
        ret = RUIM_READY;
    }
    /*  ALPS01977096: To align with GSM to set READY if Radio is NOT UNAVAILABLE, 2015/03/12 }*/

    queryEfDir(t);

done:
    at_response_free(p_response);
    LOGD("%s: ret = %d", __FUNCTION__, ret);
    return ret;
}

/// M: SVLTE solution2 modification. @{
void setPinPukCountLeftProperty(int pin1, int pin2, int puk1, int puk2) {
    int slotid = 0;
    char str[16] = { 0 };

    slotid = getCdmaModemSlot();

    LOGD("%s: slotid:%d, pin1:%d, pin2:%d, puk1:%d, puk2:%d.",
            __FUNCTION__, slotid, pin1, pin2, puk1, puk2);

    // pin1
    sprintf(str, "%d", pin1);
    property_set(PROPERTY_RIL_SIM_PIN1[slotid - 1], str);
    memset(str, 0, 16);
    // pin2
    sprintf(str, "%d", pin2);
    property_set(PROPERTY_RIL_SIM_PIN2[slotid - 1], str);
    memset(str, 0, 16);
    // puk1
    sprintf(str, "%d", puk1);
    property_set(PROPERTY_RIL_SIM_PUK1[slotid - 1], str);
    memset(str, 0, 16);
    // puk2
    sprintf(str, "%d", puk2);
    property_set(PROPERTY_RIL_SIM_PUK2[slotid - 1], str);
}
/// @}

int correctPinStatus(int sim_state, char *fac, RIL_Token t) {
    ATResponse*     p_response = NULL;
    int             err = -1;
    int             status = -1, result = RIL_PINSTATE_UNKNOWN;
    char*           cmd = NULL;
    char*           line = NULL;

    asprintf(&cmd, "AT+CLCK=\"%s\",2", fac);
    err = at_send_command_singleline(cmd, "+CLCK:", &p_response, SIM_CHANNEL_CTX);

    free(cmd);

    if (err < 0 || p_response == NULL || (p_response != NULL && p_response->success == 0)) {
        RLOGE("[correctPinStatus] Fail to get facility lock");
    } else {
        line = p_response->p_intermediates->line;
        err = at_tok_start(&line);
        if (err < 0) goto error;
        err = at_tok_nextint(&line, &status);
        if (err < 0) goto error;
    }

    if (status == 0) {
        result = RIL_PINSTATE_DISABLED;
    } else if (status == 1 && (sim_state == RUIM_READY)) {
        if (strcmp(fac, "SC") == 0) {
            result = RIL_PINSTATE_ENABLED_VERIFIED;
        } else {
            result = RIL_PINSTATE_ENABLED_NOT_VERIFIED;
        }
    }
    RLOGD("[correctPinStatus] sim_state %d result %d", sim_state, result);
    at_response_free(p_response);
    return result;
error:
    RLOGE("[correctPinStatus] CLCK error!");
    at_response_free(p_response);
    return RIL_PINSTATE_UNKNOWN;
}

static void sleepMsec(long long msec) {
    struct timespec ts;
    int err;

    ts.tv_sec = (msec / 1000);
    ts.tv_nsec = (msec % 1000) * 1000 * 1000;

    do {
        err = nanosleep(&ts, &ts);
    } while (err < 0 && errno == EINTR);
}

bool getIccIdFromProperty(char *iccid) {
    bool isIccIdReady = false;
    char iccIdProp[PROPERTY_VALUE_MAX] = {0};
    property_get(PROPERTY_ICCID_SIM[getCdmaSocketSlotId()-1], iccIdProp, "");

    if (strlen(iccIdProp) > 0) {
        isIccIdReady = true;

        if (strcmp(iccIdProp, "N/A") != 0) {
            property_get(PROPERTY_ICCID_SIM[getCdmaSocketSlotId()-1], iccid, "");
        }
    }

    return isIccIdReady;
}

static RUIM_Status getSIMStatusWithRetry() {
    RUIM_Status sim_status = RUIM_ABSENT;
    int count = 0;
    do {
        sim_status = getRUIMStatus(DO_NOT_UPDATE);
       if (RUIM_BUSY == sim_status) {
           sleepMsec(200);
           count++;     // to avoid block; if the busy time is too long; need to check modem.
           if (count == 30) break;
       }
    } while (RUIM_BUSY == sim_status);
    return sim_status;
}

/**
 * Get the current card status.
 *
 * This must be freed using freeCardStatus.
 * @return: On success returns RIL_E_SUCCESS
 */
static int getCardStatus(VIA_RIL_CardStatus **pp_card_status, RIL_Token t) {
    static RIL_AppStatus app_status_array[] = {
  // SIM_ABSENT = 0
        { RIL_APPTYPE_UNKNOWN, RIL_APPSTATE_UNKNOWN, RIL_PERSOSUBSTATE_READY,
          NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
        // SIM_NOT_READY = 1
        { RIL_APPTYPE_RUIM, RIL_APPSTATE_DETECTED, RIL_PERSOSUBSTATE_READY,
          NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
        // SIM_READY = 2
        { RIL_APPTYPE_RUIM, RIL_APPSTATE_READY, RIL_PERSOSUBSTATE_READY,
          NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
        // SIM_PIN = 3
        { RIL_APPTYPE_RUIM, RIL_APPSTATE_PIN, RIL_PERSOSUBSTATE_READY,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },
        // SIM_PUK = 4
        { RIL_APPTYPE_RUIM, RIL_APPSTATE_PUK, RIL_PERSOSUBSTATE_READY,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_BLOCKED, RIL_PINSTATE_UNKNOWN },
        // SIM_NETWORK_PERSONALIZATION = 5
        { RIL_APPTYPE_RUIM, RIL_APPSTATE_SUBSCRIPTION_PERSO, RIL_PERSOSUBSTATE_RUIM_NETWORK1,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN }
    };

    RIL_CardState card_state;
    int num_apps;
    int chvCount[4];
    int i;
    int appstatusarraylen = 0;
    int ratSwitching = 0;
    char tmp[PROPERTY_VALUE_MAX] = {0};
    char iccId[PROPERTY_VALUE_MAX] = {0};
    RUIM_Status sim_status = RUIM_ABSENT;
    int count = 0;

    ratSwitching = getRatSwitching();
    property_get(PROPERTY_RIL_CT3G[getCdmaSocketSlotId() - 1], tmp, "");

    //MTK6592 add for UICC card, if uicc card insert, we change RIL_APPTYPE_RUIM to RIL_APPTYPE_CSIM
    LOGD("cardtype is:%d before change:%d", cardtype, app_status_array[1].app_type);
    char tmp_csim[PROPERTY_VALUE_MAX] = { 0 };
    char *isexist;
    property_get(PROPERTY_RIL_FULL_UICC_TYPE[getCdmaSocketSlotId()-1], tmp_csim, "");
    // LOGD("%s: %s", PROPERTY_RIL_FULL_UICC_TYPE[getCdmaSocketSlotId()-1], tmp_csim);
    isexist = strstr(tmp_csim, "CSIM");

    appstatusarraylen = sizeof(app_status_array) / sizeof(RIL_AppStatus);
    // LOGD("appstatusarraylen is:%d", appstatusarraylen);
    if(isexist != NULL) {
        for (i = 1; i < appstatusarraylen; i++) {
            app_status_array[i].app_type = RIL_APPTYPE_CSIM;
        }
    } else {
        for (i = 1; i < appstatusarraylen; i++) {
            app_status_array[i].app_type = RIL_APPTYPE_RUIM;
        }
    }
    LOGD("isExist: %s, cardtype after change:%d", isexist, app_status_array[1].app_type);

    do {
        sim_status = getRUIMStatus(UPDATE_RADIO_STATUS);
        if (RUIM_BUSY == sim_status) {
            sleepMsec(200);
            count++;
            // To avoid block; if the busy time is too long; need to check modem.
            if (count == 30) {
                LOGE("Retry 30 times and still error in getRUIMStatus!");
                sim_status = RUIM_NOT_READY;
                break;
            }
        }
    } while (RUIM_BUSY == sim_status);

    if ((sim_status == RUIM_NOT_READY) || !getIccIdFromProperty(iccId)) {
        // RUIM not ready so do nothing!
        LOGE("Fail to get card status!");
        return RIL_E_GENERIC_FAILURE;
    }

    if (sim_status != RUIM_ABSENT) {
        queryEfDir(t);
    }

    if (sim_status == RUIM_ABSENT) {
        LOGD("ratSwitching: %d, ct3g: %s", ratSwitching, tmp);
        if ((strcmp("1", tmp) == 0) && ratSwitching == 1) {
            LOGD("ABSENT due to rat switch so change as PRESENT!");
            card_state = RIL_CARDSTATE_PRESENT;
        } else {
            card_state = RIL_CARDSTATE_ABSENT;
        }
        num_apps = 0;
    } else if (sim_status == RUIM_CARD_REBOOT) {
        card_state = RIL_CARDSTATE_PRESENT;
        num_apps = 0;
    } else if (sim_status == RUIM_RESTRICTED) {
        card_state = RIL_CARDSTATE_RESTRICTED;
        num_apps = 1;
    } else {
        card_state = RIL_CARDSTATE_PRESENT;
        num_apps = 1;
    }
    // Allocate and initialize base card status.
    VIA_RIL_CardStatus *p_card_status = malloc(sizeof(VIA_RIL_CardStatus));
    assert(p_card_status != NULL);
    p_card_status->card_state = card_state;
    p_card_status->universal_pin_state = RIL_PINSTATE_UNKNOWN;
    p_card_status->gsm_umts_subscription_app_index = -1;
    p_card_status->cdma_subscription_app_index = -1;
    p_card_status->num_applications = num_apps;

    // Initialize application status
    for (i = 0; i < RIL_CARD_MAX_APPS; i++) {
        p_card_status->applications[i] = app_status_array[RUIM_ABSENT];
    }

    // Pickup the appropriate application status
    // that reflects sim_status for gsm.
    if (num_apps != 0) {
        if (isCdmaLteDcSupport()) {
            int typeCount = 0;
            char cardType[PROPERTY_VALUE_MAX] = {0};
            property_get(PROPERTY_RIL_FULL_UICC_TYPE[getCdmaSocketSlotId()-1], cardType, "");
            LOGD("C2K getCardStatus cardType is:%s", cardType);

            char* type = cardType;
            char* out;
            int err = 0;
            while (at_tok_hasmore(&type)) {
                err = at_tok_nextstr(&type, &out);
                if (err < 0) {
                    continue;
                }
                // LOGD("C2K getCardStatus out is:%s", out);
                if (strcmp("USIM", out) == 0 || strcmp("SIM", out) == 0) {
                    typeCount|= 0x01;
                    continue;
                } else if (strcmp("CSIM", out) == 0 || strcmp("RUIM", out) == 0) {
                    typeCount|= 0x02;
                    continue;
                }
            }
            LOGD("C2K getCardStatus typeCount is:%d", typeCount);

            // Only support one app, ruim
            p_card_status->num_applications = 1;
            p_card_status->gsm_umts_subscription_app_index = 0;
            p_card_status->cdma_subscription_app_index = 0;
            // Get the correct app status
            p_card_status->applications[0] = app_status_array[sim_status];
            if (app_status_array[sim_status].app_type == RIL_APPTYPE_CSIM) {
                // Set CSIM AID and application label
                AidInfo* pAidInfo = getAidInfo(AID_CSIM);
                p_card_status->applications[0].aid_ptr = pAidInfo->aid;
                p_card_status->applications[0].app_label_ptr = pAidInfo->appLabel;
            }
            if (sim_status == RUIM_READY) {
                p_card_status->applications[0].pin1 = correctPinStatus(sim_status, "SC", t);
                // MD3 not support
                // p_card_status->applications[0].pin2 = correctPinStatus(sim_status, "FD", t);
            }
        } else {
            // Only support one app, gsm
            p_card_status->num_applications = 1;
            p_card_status->gsm_umts_subscription_app_index = 0;
            p_card_status->cdma_subscription_app_index = 0;
            // Get the correct app status
            p_card_status->applications[0] = app_status_array[sim_status];
        }
    }

    getChvCount(chvCount, &chvCount[1], &chvCount[2], &chvCount[3]);
    p_card_status->pin1Count = chvCount[0];
    p_card_status->pin2Count = chvCount[1];
    p_card_status->puk1Count = chvCount[2];
    p_card_status->puk2Count = chvCount[3];

    p_card_status->physicalSlotId = getCdmaSocketSlotId()-1;
    p_card_status->atr = NULL;
    p_card_status->eid = NULL;

    p_card_status->iccId = (char *)calloc(1, PROPERTY_VALUE_MAX);
    strncpy(p_card_status->iccId, iccId, strlen(iccId));
    (p_card_status->iccId)[strlen(iccId)] = '\0';

    *pp_card_status = p_card_status;
    return RIL_E_SUCCESS;
}

/**
 * Free the card status returned by getCardStatus
 */
static void freeCardStatus(VIA_RIL_CardStatus *p_card_status) {
    free(p_card_status);
}

/**
 * Get the CHV count
 *
 */
static void getChvCount(int * pin1Count, int * pin2Count, int * puk1Count, int * puk2Count) {
    ATResponse *p_response = NULL;
    int err;
    char *cpinLine;
    char *cpinResult = NULL;
    char full_uicc_type[PROPERTY_VALUE_MAX] = { 0 };

    *pin1Count = 0;
    *pin2Count = 0;
    *puk1Count = 0;
    *puk2Count = 0;

    // LOGD("getChvCount");
    /* remove radio check as MD3 supports AT+CPINC query at radio off state
    if (getRadioState() == RADIO_STATE_OFF || getRadioState() == RADIO_STATE_UNAVAILABLE) {
        LOGD("getChvCount error : RADIO_STATE OFF or UNAVAILABLE");
        goto error;
    }
    */
    err = at_send_command_singleline("AT+CPINC?", "+CPINC:", &p_response, getChannelCtxbyProxy());

    if (err != 0) {
        LOGD("getChvCount error : AT+CPINC failed");
        goto error;
    }

    switch (at_get_cme_error(p_response)) {
        case CME_SUCCESS:
            break;

        case CME_SIM_NOT_INSERTED:
        case CME_SIM_FAILURE:
            LOGD("getChvCount error : CME_SIM_NOT_INSERTED");
            goto error;

        default:
            LOGD("getChvCount error : not CME_SUCCESS");
            goto error;
    }

    /* CPIN? has succeeded, now look at the result */

    cpinLine = p_response->p_intermediates->line;
    err = at_tok_start (&cpinLine);

    if (err < 0) {
        LOGD("getChvCount error : at result error");
        goto error;
    }

    *pin1Count = atoi(cpinLine);
    err = at_tok_nextstr(&cpinLine, &cpinResult);
    if (err < 0 || !cpinResult) {
        LOGD("getChvCount error : puk1");
        goto error;
    }

    *puk1Count = atoi(cpinLine);
    err = at_tok_nextstr(&cpinLine, &cpinResult);
    if (err < 0 || !cpinResult) {
        LOGD("getChvCount error : pin2");
        goto error;
    }

    *pin2Count = atoi(cpinLine);
    err = at_tok_nextstr(&cpinLine, &cpinResult);
    if (err < 0 || !cpinLine) {
        LOGD("getChvCount error : puk2");
        goto error;
    }
    *puk2Count = atoi(cpinLine);
    LOGD("getChvCount : %d, %d, %d, %d", *pin1Count, *pin2Count, *puk1Count, *puk2Count);

    goto done;

error :
    *pin1Count = *pin2Count = *puk1Count = *puk2Count = -1;

done:
    property_get(PROPERTY_RIL_FULL_UICC_TYPE[getCdmaSocketSlotId()-1], full_uicc_type, "");
    LOGD("%s: %s", PROPERTY_RIL_FULL_UICC_TYPE[getCdmaSocketSlotId()-1], full_uicc_type);
    // C2K RIL only set property for CDMA 3G
    if ((strstr(full_uicc_type, "SIM") == NULL) && (strstr(full_uicc_type, "USIM") == NULL)) {
        setPinPukCountLeftProperty(*pin1Count, *pin2Count, *puk1Count, *puk2Count);
    }
    at_response_free(p_response);
    p_response = NULL;
    cpinResult = NULL;
}

static void storePinCode(const char*  string )
{
    updateNetCdmaPC(string);
    if (s_ps_on_desired) {
        RIL_requestProxyTimedCallback(turnPSEnable, &psParamStorePinCode, NULL,
                NW_CHANNEL);
    }
}

/* ALPS01949738, Send CPOF->CPON after SIM PIN, 2015/02/16 {*/
static bool isNeedToSetRadio(VIA_CARD_STATE card_type) {
    static VIA_CARD_STATE s_last_type = CARD_NOT_INSERTED;
    LOGD("%s: s_last_type(%d), card_type(%d).", __FUNCTION__, s_last_type,card_type);
    if ((NEED_TO_INPUT_PIN == s_last_type) &&
            ((CT_4G_UICC_CARD == card_type) || (NOT_CT_UICC_CARD == card_type))) {
        s_last_type = card_type;
        return true;
    } else {
        s_last_type = card_type;
        return false;
    }
}

static void TriggerMD3BootPower() {
    int err = 0;
    ATResponse *p_response = NULL;

    LOGD("%s: current radio state is %d", __FUNCTION__, getRadioState());
    err = at_send_command("AT+CPOF", &p_response, getChannelCtxbyProxy());
    if (err < 0 || (p_response && p_response->success == 0)) {
        LOGE("%s: failed to send CPOF, err is %d", __FUNCTION__, err);
    } else {
        combineDataAttach(getChannelCtxbyProxy());
        at_response_free(p_response);
        p_response = NULL;
        grabRadioPowerLock();
        err = at_send_command("AT+CPON", &p_response, getChannelCtxbyProxy());
        releaseRadioPowerLock();
        if (err < 0|| p_response->success == 0) {
            LOGE("%s: failed to send CPON, err is %d", __FUNCTION__, err);
        }
    }
    if (p_response) {
        at_response_free(p_response);
    }
}
/* ALPS01949738, Send CPOF->CPON after SIM PIN, 2015/02/16 }*/

void resetSIMProperties(const char *p_sim) {
    RLOGD("%s: Reset %s to null", __FUNCTION__, p_sim);
    property_set(p_sim, "");
}
/**
 * Switch RUIM card to SIM or switch SIM to RUIM.
 */
static void requestSwitchCardType(void *data, size_t datalen, RIL_Token t) {
    SIM_UNUSED(datalen);
    char* cmd = NULL;
    ATResponse *p_response = NULL;
    int err = 0;
    int cardType = ((int *) data)[0];
    LOGD("c2k requestSwitchCardType(), cardType = %d", cardType);
    // switch UTK/STK mode.
    err = switchStkUtkMode(cardType, t);
    if (err >= 0) {
        // radio off
        err = asprintf(&cmd, "AT+CPOF");
        LOGD("c2k requestSwitchCardType(), send command %s.", cmd);
        err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
    }
    LOGD("c2k requestSwitchCardType(), err=%d.", err);
    if (err < 0) {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
    if (cmd != NULL) {
        free(cmd);
    }
    at_response_free(p_response);
}


/* ALPS02217042: Trigger to power off/on MD only under SIM PIN mode {*/
//TODO: To merge code for ALPS01949738
static void updateCardState(void *controller, int card_state) {
    assert(NULL != controller);
    SimPowerController *p_ctrl = (SimPowerController*)controller;
    //Trigger to power on if (SIM PIN -> SIM READY) && (CPOF is sent)
    p_ctrl->refresh(p_ctrl, card_state);
    p_ctrl->card_state = card_state;
}

static void setSimPower(void *controller) {
    assert(NULL != controller);
    SimPowerController *p_ctrl = (SimPowerController*)controller;

    LOGD("%s: card_state:%d, is_poweroff:%d.",
            __FUNCTION__, p_ctrl->card_state, p_ctrl->is_poweroff);
    if ((NEED_TO_INPUT_PIN == p_ctrl->card_state)
            && (!p_ctrl->is_poweroff)) {
        p_ctrl->trigger(_FALSE);
        p_ctrl->is_poweroff = _TRUE;
    }
}

static void refreshSimPower(void *controller, int current_card_state) {
    assert(NULL != controller);
    SimPowerController *p_ctrl = (SimPowerController*)controller;

    LOGD("%s: current_card_state:%d, card_state:%d, is_poweroff:%d.",
            __FUNCTION__, current_card_state, p_ctrl->card_state, p_ctrl->is_poweroff);
    if ((NEED_TO_INPUT_PIN == p_ctrl->card_state)
            && ((UIM_CARD == current_card_state)
                    || (SIM_CARD == current_card_state)
                    || (UIM_SIM_CARD == current_card_state)
                    || (CT_3G_UIM_CARD == current_card_state)
                    || (CT_UIM_SIM_CARD == current_card_state)
                    || (CT_4G_UICC_CARD == current_card_state)
                    || (NOT_CT_UICC_CARD == current_card_state))
            && (p_ctrl->is_poweroff)) {
        RIL_requestProxyTimedCallback(
                requestSetSimPower, p_ctrl, NULL, SIM_CHANNEL);
    }
}

static void triggerSetSimPower(int power) {
    PS_PARAM ps_param = {0, 1};
    ps_param.enable = power;
    turnPSEnable(&ps_param);
}

static void requestSetSimPower(void *controller) {
    assert(NULL != controller);
    SimPowerController *p_ctrl = (SimPowerController*) controller;
    LOGD("%s: is_poweroff:%d.", __FUNCTION__, p_ctrl->is_poweroff);
    p_ctrl->trigger(p_ctrl->is_poweroff);
    p_ctrl->is_poweroff = !p_ctrl->is_poweroff;
}
/* ALPS02217042: Trigger to power off/on MD only under SIM PIN mode }*/

/**
  * Enable or disable MD3 Sleep
 */

_BOOL isSimInserted(RIL_SOCKET_ID rid) {
    SIM_UNUSED(rid);
    return (UIM_STATUS_CARD_INSERTED == uimInsertedStatus) ? _TRUE : _FALSE;
}

// BT SIM Access Profile - START
int rilBtSapMain(int request, void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID rid) {
    switch (request) {
        case MsgId_RIL_SIM_SAP_CONNECT:
            requestBtSapConnect(data, datalen, t, rid);
            break;
        case MsgId_RIL_SIM_SAP_DISCONNECT:
            requestBtSapDisconnect(data, datalen, t, rid);
            break;
        case MsgId_RIL_SIM_SAP_APDU:
            requestBtSapTransferApdu(data, datalen, t, rid);
            break;
        case MsgId_RIL_SIM_SAP_TRANSFER_ATR:
            requestBtSapTransferAtr(data, datalen, t, rid);
            break;
        case MsgId_RIL_SIM_SAP_POWER:
            requestBtSapPower(data, datalen, t, rid);
            break;
        case MsgId_RIL_SIM_SAP_RESET_SIM:
            requestBtSapResetSim(data, datalen, t, rid);
            break;
        case MsgId_RIL_SIM_SAP_SET_TRANSFER_PROTOCOL:
            requestBtSapSetTransferProtocol(data, datalen, t, rid);
            break;
        default:
            return 0;
            break;
    }

    return 1;
}

LocalBtSapMsgHeader* allocateLocalBtSapMsgHeader(void *param, RIL_Token t,
        RIL_SOCKET_ID rid) {
    LocalBtSapMsgHeader *localMsgHdr = (LocalBtSapMsgHeader*)malloc(sizeof(LocalBtSapMsgHeader));
    assert(localMsgHdr != NULL);
    memset(localMsgHdr, 0, sizeof(LocalBtSapMsgHeader));
    localMsgHdr->t = t;
    localMsgHdr->param = param;
    localMsgHdr->socket_id = rid;

    return localMsgHdr;
}

void releaseLocalBtSapMsgHeader(LocalBtSapMsgHeader *localMsgHdr) {
    if (localMsgHdr != NULL) {
        if (localMsgHdr->param != NULL) {
            free(localMsgHdr->param);
        }
        free(localMsgHdr);
    }
}

void decodeBtSapPayload(MsgId msgId, void *src, size_t srclen, void *dst) {
    pb_istream_t stream;
    const pb_field_t *fields = NULL;

    RLOGD("[BTSAP] decodeBtSapPayload start (%s)", btSapMsgIdToString(msgId));
    if (dst == NULL || src == NULL) {
        RLOGE("[BTSAP] decodeBtSapPayload, dst or src is NULL!!");
        return;
    }

    switch (msgId) {
        case MsgId_RIL_SIM_SAP_CONNECT:
            fields = RIL_SIM_SAP_CONNECT_REQ_fields;
            break;
        case MsgId_RIL_SIM_SAP_DISCONNECT:
            fields = RIL_SIM_SAP_DISCONNECT_REQ_fields;
            break;
        case MsgId_RIL_SIM_SAP_APDU:
            fields = RIL_SIM_SAP_APDU_REQ_fields;
            break;
        case MsgId_RIL_SIM_SAP_TRANSFER_ATR:
            fields = RIL_SIM_SAP_TRANSFER_ATR_REQ_fields;
            break;
        case MsgId_RIL_SIM_SAP_POWER:
            fields = RIL_SIM_SAP_POWER_REQ_fields;
            break;
        case MsgId_RIL_SIM_SAP_RESET_SIM:
            fields = RIL_SIM_SAP_RESET_SIM_REQ_fields;
            break;
        case MsgId_RIL_SIM_SAP_SET_TRANSFER_PROTOCOL:
            fields = RIL_SIM_SAP_SET_TRANSFER_PROTOCOL_REQ_fields;
            break;
        default:
            RLOGE("[BTSAP] decodeBtSapPayload, MsgId is mistake!");
            return;
    }

    stream = pb_istream_from_buffer((uint8_t *)src, srclen);
    if (!pb_decode(&stream, fields, dst) ) {
        RLOGE("[BTSAP] decodeBtSapPayload, Error decoding protobuf buffer : %s", PB_GET_ERROR(&stream));
    } else {
        RLOGD("[BTSAP] decodeBtSapPayload, Success!");
    }
}

void requestBtSapConnect(void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID rid) {
    RIL_SIM_SAP_CONNECT_REQ *req = NULL;
    RIL_SIM_SAP_CONNECT_RSP rsp;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    RILChannelCtx *p_cctx = SIM_CHANNEL_CTX;
    //FIXME: To get from system property
    int sysMaxSize = 32767;

    req = (RIL_SIM_SAP_CONNECT_REQ*)malloc(sizeof(RIL_SIM_SAP_CONNECT_REQ));
    assert(req != NULL);
    memset(req, 0, sizeof(RIL_SIM_SAP_CONNECT_REQ));
    decodeBtSapPayload(MsgId_RIL_SIM_SAP_CONNECT, data, datalen, req);

    RLOGD("[BTSAP] requestBtSapConnect (%d,%d,%d)", rid, req->max_message_size, sysMaxSize);
    do {
        if (req->max_message_size > sysMaxSize) {
            // Max message size from request > project setting
            rsp.response = RIL_SIM_SAP_CONNECT_RSP_Response_RIL_E_SAP_MSG_SIZE_TOO_LARGE;
            rsp.max_message_size = sysMaxSize;
            rsp.has_max_message_size = TRUE;
        //} else if (req->max_message_size < sysMaxSize) {
            // Max message size from request < project setting
        //    rsp.response = RIL_SIM_SAP_CONNECT_RSP_Response_RIL_E_SAP_MSG_SIZE_TOO_SMALL;
        //    rsp.max_message_size = sysMaxSize;
        //    rsp.has_max_message_size = TRUE;
        } else {
            // Send connect request
            RIL_SIM_SAP_CONNECT_REQ *local_req =
                (RIL_SIM_SAP_CONNECT_REQ*)malloc(sizeof(RIL_SIM_SAP_CONNECT_REQ));
            assert(local_req != NULL);
            memset(local_req, 0, sizeof(RIL_SIM_SAP_CONNECT_REQ));
            memcpy(local_req, req, sizeof(RIL_SIM_SAP_CONNECT_REQ));
            LocalBtSapMsgHeader *localMsgHeader = allocateLocalBtSapMsgHeader(local_req, t, rid);
            RIL_requestProxyTimedCallback(requestLocalSapConnect, localMsgHeader,
                    &TIMEVAL_0, p_cctx->id);
            break;
        }

        // Send CONNECTION_RESP directly because max message size from request is wrong
        //RIL_SAP_onRequestComplete(t, ret, &rsp, sizeof(rsp));
        sendBtSapResponseComplete(t, ret, MsgId_RIL_SIM_SAP_CONNECT, &rsp);
    } while(0);

    free(req);

    RLOGD("[BTSAP] requestBtSapConnect end");
}

void requestLocalSapConnect(void *param) {
    LocalBtSapMsgHeader *localMsgHdr = (LocalBtSapMsgHeader*)param;
    RIL_Token t = localMsgHdr->t;
    RIL_SOCKET_ID rid = localMsgHdr->socket_id;
    RIL_SIM_SAP_CONNECT_RSP rsp;
    ATResponse *p_response = NULL;
    int err, type = -1;
    char *line = NULL, *pAtr = NULL;
    RILChannelCtx *p_cctx = getChannelCtxbyProxy();
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    RIL_SIM_SAP_STATUS_IND_Status unsolMsg = RIL_SIM_SAP_STATUS_IND_Status_RIL_SIM_STATUS_UNKNOWN_ERROR;

    RLOGD("[BTSAP] requestLocalSapConnect start");
    err = at_send_command_singleline("AT+EBTSAP=0", "+EBTSAP:", &p_response, p_cctx);

    if (err < 0 || NULL == p_response) {
        rsp.response = RIL_SIM_SAP_CONNECT_RSP_Response_RIL_E_SAP_CONNECT_FAILURE;
        goto format_error;
    }

    if (p_response->success == 0) {
        RLOGE("[BTSAP] CME ERROR = %d", at_get_cme_error(p_response));
        rsp.response = RIL_SIM_SAP_CONNECT_RSP_Response_RIL_E_SAP_CONNECT_FAILURE;
        switch (at_get_cme_error(p_response)) {
            case CME_BT_SAP_UIM_ERROR_NO_REASON_DEFINED:
                ret = RIL_E_BT_SAP_ERROR_NO_REASON_DEFINED;
                break;
            case CME_BT_SAP_UIM_NOT_ACCESSIBLE:
                ret = RIL_E_BT_SAP_NOT_ACCESSIBLE;
                unsolMsg = RIL_SIM_SAP_STATUS_IND_Status_RIL_SIM_STATUS_CARD_NOT_ACCESSIBLE;
                break;
            case CME_BT_SAP_UIM_NOT_INSERTED:
                ret = RIL_E_BT_SAP_UIM_NOT_INSERTED;
                unsolMsg = RIL_SIM_SAP_STATUS_IND_Status_RIL_SIM_STATUS_CARD_REMOVED;
                break;
            default:
                break;
        }
        goto format_error;
    } else {
        line = p_response->p_intermediates->line;
        rsp.response = RIL_SIM_SAP_CONNECT_RSP_Response_RIL_E_SAP_CONNECT_FAILURE;

        err = at_tok_start(&line);
        if (err < 0) goto format_error;

        err = at_tok_nextint(&line, &type);
        if (err < 0) goto format_error;

        s_bt_sap_current_protocol[rid] = type;

        err = at_tok_nextint(&line, &type);
        if (err < 0) goto  format_error;

        s_bt_sap_support_protocol[rid] = type;

        err = at_tok_nextstr(&line, &pAtr);
        if (err < 0) goto  format_error;

        int atrLen = 0;
        if (pAtr != NULL) {
            atrLen = strlen(pAtr);
            s_bt_sap_atr[rid] = (unsigned char*)malloc((atrLen+1)*sizeof(unsigned char));
            assert(s_bt_sap_atr[rid] != NULL);
            memset(s_bt_sap_atr[rid], 0, atrLen);
            memcpy(s_bt_sap_atr[rid], pAtr, atrLen);
        }

        RLOGD("[BTSAP] requestLocalSapConnect, cur_type: %d, supp_type: %d, size: %d",
                s_bt_sap_current_protocol[rid], s_bt_sap_support_protocol[rid], atrLen);

        ret = RIL_E_SUCCESS;
        // For AOSP BT SAP UT case, to set the flag has_max_message_size as true.
        //rsp.has_max_message_size = false;
        rsp.has_max_message_size = true;
        rsp.max_message_size = 0;
        RLOGD("[BTSAP] Connection Success");
        rsp.response = RIL_SIM_SAP_CONNECT_RSP_Response_RIL_E_SUCCESS;
        unsolMsg = RIL_SIM_SAP_STATUS_IND_Status_RIL_SIM_STATUS_CARD_RESET;
        s_bt_sap_status[rid] = BT_SAP_CONNECTION_SETUP;
    }

    // Send response
    //RIL_SAP_onRequestComplete(t, ret, &rsp, sizeof(rsp));
    sendBtSapResponseComplete(t, ret, MsgId_RIL_SIM_SAP_CONNECT, &rsp);

    at_response_free(p_response);
    releaseLocalBtSapMsgHeader(localMsgHdr);

    if (unsolMsg != RIL_SIM_SAP_STATUS_IND_Status_RIL_SIM_STATUS_UNKNOWN_ERROR) {
        notifyBtSapStatusInd(unsolMsg, rid);
    }
    RLOGD("[BTSAP] requestLocalSapConnect end");
    return;
format_error:
    RLOGE("[BTSAP] Connection Fail");
    rsp.has_max_message_size = false;
    rsp.max_message_size = 0;

    // Send response
    //RIL_SAP_onRequestComplete(t, ret, &rsp, sizeof(rsp));
    sendBtSapResponseComplete(t, ret, MsgId_RIL_SIM_SAP_CONNECT, &rsp);

    at_response_free(p_response);
    releaseLocalBtSapMsgHeader(localMsgHdr);
    RLOGD("[BTSAP] requestLocalSapConnect end");
}

void requestBtSapDisconnect(void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID rid) {
    RIL_SIM_SAP_DISCONNECT_REQ *req = NULL;
    RIL_SIM_SAP_DISCONNECT_RSP rsp;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    RILChannelCtx *p_cctx = SIM_CHANNEL_CTX;

    RLOGD("[BTSAP] requestBtSapDisconnect start");
    req = (RIL_SIM_SAP_DISCONNECT_REQ*)malloc(sizeof(RIL_SIM_SAP_DISCONNECT_REQ));
    assert(req != NULL);
    memset(req, 0, sizeof(RIL_SIM_SAP_DISCONNECT_REQ));
    decodeBtSapPayload(MsgId_RIL_SIM_SAP_DISCONNECT, data, datalen, req);

    s_bt_sap_status[rid] = BT_SAP_DISCONNECT;
    if (isSimInserted(rid)) {
        // Send disconnect request
        RIL_SIM_SAP_DISCONNECT_REQ *local_req =
            (RIL_SIM_SAP_DISCONNECT_REQ*)malloc(sizeof(RIL_SIM_SAP_DISCONNECT_REQ));
        assert(local_req != NULL);
        memset(local_req, 0, sizeof(RIL_SIM_SAP_DISCONNECT_REQ));
        memcpy(local_req, req, sizeof(RIL_SIM_SAP_DISCONNECT_REQ));
        LocalBtSapMsgHeader *localMsgHeader = allocateLocalBtSapMsgHeader(local_req, t, rid);
        RIL_requestProxyTimedCallback(requestLocalBtSapDisconnect, localMsgHeader,
                &TIMEVAL_0, p_cctx->id);
    } else {
        RLOGD("[BTSAP] requestBtSapDisconnect but card was removed");
        rsp.dummy_field = 1;
        s_bt_sap_status[rid] = BT_SAP_INIT;
        ret = RIL_E_SUCCESS;
        // FIXME: Send response immediately
        //RIL_SAP_onRequestComplete(t, ret, &rsp, sizeof(rsp));
        sendBtSapResponseComplete(t, ret, MsgId_RIL_SIM_SAP_DISCONNECT, &rsp);
    }
    free(req);
    RLOGD("[BTSAP] requestBtSapDisconnect end");
}

void requestLocalBtSapDisconnect(void *param) {
    LocalBtSapMsgHeader *localMsgHdr = (LocalBtSapMsgHeader*)param;
    RIL_Token t = localMsgHdr->t;
    RIL_SOCKET_ID rid = localMsgHdr->socket_id;
    RIL_SIM_SAP_DISCONNECT_RSP rsp;
    ATResponse *p_response = NULL;
    int err;
    RILChannelCtx *p_cctx = getChannelCtxbyProxy();
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;

    RLOGD("[BTSAP] requestLocalBtSapDisconnect start");
    err = at_send_command("AT+EBTSAP=1", &p_response, p_cctx);

    if (err < 0 || NULL == p_response) {
        goto format_error;
    }

    if (p_response->success == 0) {
        RLOGE("[BTSAP] CME ERROR = %d", at_get_cme_error(p_response));
        switch (at_get_cme_error(p_response)) {
            case CME_BT_SAP_UIM_ERROR_NO_REASON_DEFINED:
                ret = RIL_E_BT_SAP_ERROR_NO_REASON_DEFINED;
                break;
            case CME_BT_SAP_UIM_NOT_ACCESSIBLE:
                ret = RIL_E_BT_SAP_NOT_ACCESSIBLE;
                break;
            case CME_BT_SAP_UIM_NOT_INSERTED:
                ret = RIL_E_BT_SAP_UIM_NOT_INSERTED;
                break;
            default:
                break;
        }
        goto format_error;
    } else {
        resetBtSapContext(rid);
        ret = RIL_E_SUCCESS;
        rsp.dummy_field = 1;
    }

    // Send response
    //RIL_SAP_onRequestComplete(t, ret, &rsp, sizeof(rsp));
    sendBtSapResponseComplete(t, ret, MsgId_RIL_SIM_SAP_DISCONNECT, &rsp);

    at_response_free(p_response);
    releaseLocalBtSapMsgHeader(localMsgHdr);

    RLOGD("[BTSAP] requestLocalBtSapDisconnect end");
    return;
format_error:
    RLOGE("[BTSAP] Fail to disconnect");
    s_bt_sap_status[rid] = BT_SAP_INIT;
    rsp.dummy_field = 0;

    // Send response
    //RIL_SAP_onRequestComplete(t, ret, &rsp, sizeof(rsp));
    sendBtSapResponseComplete(t, ret, MsgId_RIL_SIM_SAP_DISCONNECT, &rsp);

    at_response_free(p_response);
    releaseLocalBtSapMsgHeader(localMsgHdr);

    RLOGD("[BTSAP] requestLocalBtSapDisconnect end");
}

char* convertBtSapIntToHexString(uint8_t *data, size_t datalen) {
    char* output = NULL, *pOut = NULL;
    size_t i = 0;

    if (data == NULL || datalen <= 0) {
        return output;
    }

    output = (char*)calloc(1, (sizeof(char)*datalen+1)*2);
    assert(output != NULL);
    pOut = output;

    for (i = 0; i < datalen; i++) {
        pOut = &output[2*i];
        sprintf(pOut, "%02x", data[i]);
    }

    RLOGD("[BTSAP] convert string (%lu, %s)", datalen, output);
    return output;
}

void requestBtSapTransferApdu(void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID rid) {
    RIL_SIM_SAP_APDU_REQ *req = NULL;
    RIL_SIM_SAP_APDU_RSP rsp;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    RILChannelCtx *p_cctx = SIM_CHANNEL_CTX;

    req = (RIL_SIM_SAP_APDU_REQ*)malloc(sizeof(RIL_SIM_SAP_APDU_REQ));
    assert(req != NULL);
    memset(req, 0, sizeof(RIL_SIM_SAP_APDU_REQ));
    decodeBtSapPayload(MsgId_RIL_SIM_SAP_APDU, data, datalen, req);
    RLOGD("[BTSAP] requestBtSapTransferApdu start, (%d,%d)", rid, req->type);
    do {
        if (!isSimInserted(rid)) {
            RLOGD("[BTSAP] requestBtSapTransferApdu but card was removed");
            rsp.response = RIL_SIM_SAP_APDU_RSP_Response_RIL_E_SIM_ABSENT;
            ret = RIL_E_BT_SAP_UIM_NOT_INSERTED;
        } else if (queryBtSapStatus(rid) == BT_SAP_POWER_OFF) {
            RLOGD("[BTSAP] requestBtSapTransferApdu but card was already power off");
            rsp.response = RIL_SIM_SAP_APDU_RSP_Response_RIL_E_SIM_ALREADY_POWERED_OFF;
        } else {
            // Send disconnect request
            RIL_SIM_SAP_APDU_REQ *local_req =
                (RIL_SIM_SAP_APDU_REQ*)malloc(sizeof(RIL_SIM_SAP_APDU_REQ));
            assert(local_req != NULL);
            memset(local_req, 0, sizeof(RIL_SIM_SAP_APDU_REQ));
            pb_bytes_array_t *payload = (pb_bytes_array_t *)calloc(
                    1,sizeof(pb_bytes_array_t) + req->command->size);
            assert(payload != NULL);
            memcpy(payload->bytes, req->command->bytes, req->command->size);
            payload->size = req->command->size;
            local_req->command = payload;
            local_req->type = req->type;
            LocalBtSapMsgHeader *localMsgHeader = allocateLocalBtSapMsgHeader(local_req, t, rid);
            RIL_requestProxyTimedCallback(requestLocalBtSapTransferApdu, localMsgHeader,
                    &TIMEVAL_0, p_cctx->id);
            break;
        }

        // FIXME: Send response immediately
        //RIL_SAP_onRequestComplete(t, ret, &rsp, sizeof(rsp));
        sendBtSapResponseComplete(t, ret, MsgId_RIL_SIM_SAP_APDU, &rsp);
    } while(0);

    free(req);

    RLOGD("[BTSAP] requestBtSapTransferApdu end");
}

void requestLocalBtSapTransferApdu(void *param) {
    LocalBtSapMsgHeader *localMsgHdr = (LocalBtSapMsgHeader*)param;
    RIL_Token t = localMsgHdr->t;
    RIL_SOCKET_ID rid = localMsgHdr->socket_id;
    RIL_SIM_SAP_APDU_REQ *req = localMsgHdr->param;
    RIL_SIM_SAP_APDU_RSP rsp;
    ATResponse *p_response = NULL;
    int err;
    char* cmd = NULL;
    char *line = NULL;
    unsigned char *pApduResponse = NULL;
    char *hexStr = NULL;
    RILChannelCtx *p_cctx = getChannelCtxbyProxy();
    pb_bytes_array_t *apduResponse = NULL;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;

    hexStr = convertBtSapIntToHexString(req->command->bytes, req->command->size);
    RLOGD("[BTSAP] requestLocalBtSapTransferApdu start, (%d,%d,%lu,%s)", rid, req->type,
            req->command->size, hexStr);
    memset(&rsp, 0, sizeof(RIL_SIM_SAP_APDU_RSP));
    asprintf(&cmd, "AT+EBTSAP=5,%d,\"%s\"", req->type, hexStr);
    err = at_send_command_singleline(cmd, "+EBTSAP:", &p_response, p_cctx);

    if (hexStr != NULL) {
        free(hexStr);
    }
    free(cmd);

    if (err < 0 || NULL == p_response) {
        rsp.response = RIL_SIM_SAP_APDU_RSP_Response_RIL_E_GENERIC_FAILURE;
        goto format_error;
    }

    if (p_response->success == 0) {
        RLOGE("[BTSAP] CME ERROR = %d", at_get_cme_error(p_response));
        rsp.response = RIL_SIM_SAP_APDU_RSP_Response_RIL_E_GENERIC_FAILURE;
        switch (at_get_cme_error(p_response)) {
            case CME_BT_SAP_UIM_ERROR_NO_REASON_DEFINED:
                ret = RIL_E_BT_SAP_ERROR_NO_REASON_DEFINED;
                rsp.response = RIL_SIM_SAP_APDU_RSP_Response_RIL_E_GENERIC_FAILURE;
                break;
            case CME_BT_SAP_UIM_NOT_ACCESSIBLE:
                ret = RIL_E_BT_SAP_NOT_ACCESSIBLE;
                rsp.response = RIL_SIM_SAP_APDU_RSP_Response_RIL_E_SIM_NOT_READY;
                break;
            case CME_BT_SAP_UIM_NOT_INSERTED:
                ret = RIL_E_BT_SAP_UIM_NOT_INSERTED;
                rsp.response = RIL_SIM_SAP_APDU_RSP_Response_RIL_E_SIM_ABSENT;
                break;
            default:
                break;
        }
        goto format_error;
    } else {
        unsigned char *pRes = NULL;

        line = p_response->p_intermediates->line;

        err = at_tok_start(&line);
        if (err < 0) goto format_error;

        err = at_tok_nextstr(&line, (char**)&pApduResponse);
        if (err < 0) goto  format_error;

        int apduLen = strlen((char*)pApduResponse);
        int reslen = hexStringToByteArrayEx(pApduResponse, apduLen, &pRes);
        apduResponse = (pb_bytes_array_t *)calloc(1,sizeof(pb_bytes_array_t) + reslen);
        assert(apduResponse != NULL);
        memcpy(apduResponse->bytes, pRes, reslen);
        apduResponse->size = reslen;
        rsp.apduResponse = apduResponse;
        rsp.response = RIL_SIM_SAP_APDU_RSP_Response_RIL_E_SUCCESS;
        ret = RIL_E_SUCCESS;
        free(pRes);
    }

    // Send response
    //RIL_SAP_onRequestComplete(t, ret, &rsp, sizeof(rsp));
    sendBtSapResponseComplete(t, ret, MsgId_RIL_SIM_SAP_APDU, &rsp);

    if (apduResponse != NULL) {
        free(apduResponse);
    }
    at_response_free(p_response);
    if (req->command != NULL) {
        free(req->command);
    }
    releaseLocalBtSapMsgHeader(localMsgHdr);
    RLOGD("[BTSAP] requestLocalBtSapTransferApdu end");
    return;
format_error:
    RLOGE("[BTSAP] Fail to send APDU");

    // Send response
    //RIL_SAP_onRequestComplete(t, ret, &rsp, sizeof(rsp));
    sendBtSapResponseComplete(t, ret, MsgId_RIL_SIM_SAP_APDU, &rsp);

    at_response_free(p_response);
    if (req->command != NULL) {
        free(req->command);
    }
    releaseLocalBtSapMsgHeader(localMsgHdr);
    RLOGD("[BTSAP] requestLocalBtSapTransferApdu end");
}

void requestBtSapTransferAtr(void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID rid) {
    RIL_SIM_SAP_TRANSFER_ATR_REQ *req = NULL;
    RIL_SIM_SAP_TRANSFER_ATR_RSP rsp;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    BtSapStatus status = -1;
    pb_bytes_array_t *atrResponse = NULL;

    RLOGD("[BTSAP] requestBtSapTransferAtr start, (%d)", rid);
    req = (RIL_SIM_SAP_TRANSFER_ATR_REQ*)malloc(sizeof(RIL_SIM_SAP_TRANSFER_ATR_REQ));
    assert(req != NULL);
    memset(req, 0, sizeof(RIL_SIM_SAP_TRANSFER_ATR_REQ));
    decodeBtSapPayload(MsgId_RIL_SIM_SAP_TRANSFER_ATR, data, datalen, req);

    status = queryBtSapStatus(rid);
    if (!isSimInserted(rid)) {
        RLOGD("[BTSAP] requestBtSapTransferAtr but card was removed");
        rsp.response = RIL_SIM_SAP_TRANSFER_ATR_RSP_Response_RIL_E_SIM_ABSENT;
        rsp.atr = NULL;
        ret = RIL_E_BT_SAP_UIM_NOT_INSERTED;
    } else if (status == BT_SAP_POWER_OFF) {
        RLOGD("[BTSAP] requestBtSapTransferAtr but card was already power off");
        rsp.response = RIL_SIM_SAP_TRANSFER_ATR_RSP_Response_RIL_E_SIM_ALREADY_POWERED_OFF;
        rsp.atr = NULL;
    } else if (status == BT_SAP_POWER_ON ||
                status == BT_SAP_CONNECTION_SETUP ||
                status == BT_SAP_ONGOING_CONNECTION) {
        unsigned char *pRes = NULL;
        rsp.response = RIL_SIM_SAP_TRANSFER_ATR_RSP_Response_RIL_E_SUCCESS;
        int atrLen = strlen((char*)s_bt_sap_atr[rid]);
        int reslen = hexStringToByteArrayEx(s_bt_sap_atr[rid], atrLen, &pRes);
        atrResponse = (pb_bytes_array_t *)calloc(1,sizeof(pb_bytes_array_t) + reslen);
        assert(atrResponse != NULL);
        memcpy(atrResponse->bytes, pRes, reslen);
        atrResponse->size = reslen;
        rsp.atr = atrResponse;
        if (pRes != NULL) {
            free(pRes);
        }

        RLOGD("[BTSAP] requestBtSapTransferAtr, status: %d, size: %d", status, atrLen);

        if (status == BT_SAP_CONNECTION_SETUP) {
            s_bt_sap_status[rid] = BT_SAP_ONGOING_CONNECTION;
        }
    } else {
        rsp.response = RIL_SIM_SAP_TRANSFER_ATR_RSP_Response_RIL_E_GENERIC_FAILURE;
        rsp.atr = NULL;
    }
    // FIXME: Send response immediately
    //RIL_SAP_onRequestComplete(t, ret, &rsp, sizeof(rsp));
    sendBtSapResponseComplete(t, ret, MsgId_RIL_SIM_SAP_TRANSFER_ATR, &rsp);

    if (atrResponse != NULL) {
        free(atrResponse);
    }

    free(req);
    RLOGD("[BTSAP] requestBtSapTransferAtr end");
}

void requestBtSapPower(void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID rid) {
    RIL_SIM_SAP_POWER_REQ *req = NULL;
    RIL_SIM_SAP_POWER_RSP rsp;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    BtSapStatus status = -1;
    RILChannelCtx *p_cctx = SIM_CHANNEL_CTX;

    req = (RIL_SIM_SAP_POWER_REQ*)malloc(sizeof(RIL_SIM_SAP_POWER_REQ));
    assert(req != NULL);
    memset(req, 0, sizeof(RIL_SIM_SAP_POWER_REQ));
    decodeBtSapPayload(MsgId_RIL_SIM_SAP_POWER, data, datalen, req);

    RLOGD("[BTSAP] requestBtSapPower start, (%d,%d)", rid, req->state);
    status = queryBtSapStatus(rid);
    do {
        if (!isSimInserted(rid)) {
            RLOGD("[BTSAP] requestBtSapPower but card was removed");
            rsp.response = RIL_SIM_SAP_POWER_RSP_Response_RIL_E_SIM_ABSENT;
            ret = RIL_E_BT_SAP_UIM_NOT_INSERTED;
        } else if (req->state == TRUE && status == BT_SAP_POWER_ON) {
            RLOGD("[BTSAP] requestBtSapPower on but card was already power on");
            rsp.response = RIL_SIM_SAP_POWER_RSP_Response_RIL_E_SIM_ALREADY_POWERED_ON;
        } else if (req->state == FALSE && status == BT_SAP_POWER_OFF) {
            RLOGD("[BTSAP] requestBtSapPower off but card was already power off");
            rsp.response = RIL_SIM_SAP_POWER_RSP_Response_RIL_E_SIM_ALREADY_POWERED_OFF;
        } else {
            // Send disconnect request
            RIL_SIM_SAP_POWER_REQ *local_req =
                (RIL_SIM_SAP_POWER_REQ*)malloc(sizeof(RIL_SIM_SAP_POWER_REQ));
            assert(local_req != NULL);
            memset(local_req, 0, sizeof(RIL_SIM_SAP_POWER_REQ));
            memcpy(local_req, req, sizeof(RIL_SIM_SAP_POWER_REQ));
            LocalBtSapMsgHeader *localMsgHeader = allocateLocalBtSapMsgHeader(local_req, t, rid);
            RIL_requestProxyTimedCallback(requestLocalBtSapPower, localMsgHeader,
                    &TIMEVAL_0, p_cctx->id);
            break;
        }

        // FIXME: Send response immediately
        //RIL_SAP_onRequestComplete(t, ret, &rsp, sizeof(rsp));
        sendBtSapResponseComplete(t, ret, MsgId_RIL_SIM_SAP_POWER, &rsp);
    } while(0);

    free(req);

    RLOGD("[BTSAP] requestBtSapPower end");
}

void requestLocalBtSapPower(void *param) {
    LocalBtSapMsgHeader *localMsgHdr = (LocalBtSapMsgHeader*)param;
    RIL_Token t = localMsgHdr->t;
    RIL_SOCKET_ID rid = localMsgHdr->socket_id;
    RIL_SIM_SAP_POWER_REQ *req = localMsgHdr->param;
    RIL_SIM_SAP_POWER_RSP rsp;
    ATResponse *p_response = NULL;
    int err;
    int type = 0;
    char* cmd = NULL;
    char *line = NULL, *pAtr = NULL;
    RILChannelCtx *p_cctx = getChannelCtxbyProxy();
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;

    RLOGD("[BTSAP] requestLocalBtSapPower start, (%d,%d)", rid, req->state);
    memset(&rsp, 0, sizeof(RIL_SIM_SAP_POWER_RSP));
    if (req->state == TRUE) {
        asprintf(&cmd, "AT+EBTSAP=2,%d", s_bt_sap_current_protocol[rid]);
        err = at_send_command_singleline(cmd, "+EBTSAP:", &p_response, p_cctx);
        free(cmd);
    } else {
        err = at_send_command("AT+EBTSAP=3", &p_response, p_cctx);
    }

    if (err < 0 || NULL == p_response) {
        rsp.response = RIL_SIM_SAP_POWER_RSP_Response_RIL_E_GENERIC_FAILURE;
        goto format_error;
    }

    if (p_response->success == 0) {
        RLOGE("[BTSAP] CME ERROR = %d", at_get_cme_error(p_response));
        rsp.response = RIL_SIM_SAP_POWER_RSP_Response_RIL_E_GENERIC_FAILURE;
        switch (at_get_cme_error(p_response)) {
            case CME_BT_SAP_UIM_NOT_INSERTED:
                ret = RIL_E_BT_SAP_UIM_NOT_INSERTED;
                rsp.response = RIL_SIM_SAP_POWER_RSP_Response_RIL_E_SIM_ABSENT;
                break;
            default:
                break;
        }
        goto format_error;
    } else {
        if (req->state == TRUE) {
            line = p_response->p_intermediates->line;

            err = at_tok_start(&line);
            if (err < 0) goto format_error;

            err = at_tok_nextint(&line, &type);
            if (err < 0) goto  format_error;

            s_bt_sap_current_protocol[rid] = type;

            err = at_tok_nextstr(&line, &pAtr);
            if (err < 0) goto  format_error;

            int atrLen = 0;
            if (pAtr != NULL) {
                // Update ATR
                if (s_bt_sap_atr[rid] != NULL) {
                    free(s_bt_sap_atr[rid]);
                }
                atrLen = strlen(pAtr);
                s_bt_sap_atr[rid] = (unsigned char*)malloc((atrLen+1)*sizeof(unsigned char));
                assert(s_bt_sap_atr[rid] != NULL);
                memset(s_bt_sap_atr[rid], 0, atrLen);
                memcpy(s_bt_sap_atr[rid], pAtr, atrLen);
            }
            RLOGD("[BTSAP] requestLocalBtSapPower, cur_type: %d, size: %d",
                    s_bt_sap_current_protocol[rid], atrLen);
            s_bt_sap_status[rid] = BT_SAP_POWER_ON;
        } else {
            s_bt_sap_status[rid] = BT_SAP_POWER_OFF;
        }
        RLOGD("[BTSAP] requestLocalBtSapPower, success! status: %d", s_bt_sap_status[rid]);
        ret = RIL_E_SUCCESS;
        rsp.response = RIL_SIM_SAP_POWER_RSP_Response_RIL_E_SUCCESS;
    }

    // Send response
    //RIL_SAP_onRequestComplete(t, ret, &rsp, sizeof(rsp));
    sendBtSapResponseComplete(t, ret, MsgId_RIL_SIM_SAP_POWER, &rsp);

    at_response_free(p_response);
    releaseLocalBtSapMsgHeader(localMsgHdr);
    RLOGD("[BTSAP] requestLocalBtSapPower end");
    return;
format_error:
    RLOGE("[BTSAP] Fail to Set Power");

    // Send response
    //RIL_SAP_onRequestComplete(t, ret, &rsp, sizeof(rsp));
    sendBtSapResponseComplete(t, ret, MsgId_RIL_SIM_SAP_POWER, &rsp);

    at_response_free(p_response);
    releaseLocalBtSapMsgHeader(localMsgHdr);
    RLOGD("[BTSAP] requestLocalBtSapPower end");
}

void requestBtSapResetSim(void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID rid) {
    RIL_SIM_SAP_RESET_SIM_REQ *req = NULL;
    RIL_SIM_SAP_RESET_SIM_RSP rsp;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    BtSapStatus status = -1;
    RILChannelCtx *p_cctx = SIM_CHANNEL_CTX;

    RLOGD("[BTSAP] requestBtSapResetSim start, (%d)", rid);
    req = (RIL_SIM_SAP_RESET_SIM_REQ*)malloc(sizeof(RIL_SIM_SAP_RESET_SIM_REQ));
    assert(req != NULL);
    memset(req, 0, sizeof(RIL_SIM_SAP_RESET_SIM_REQ));
    decodeBtSapPayload(MsgId_RIL_SIM_SAP_RESET_SIM, data, datalen, req);

    status = queryBtSapStatus(rid);
    do {
        if (!isSimInserted(rid)) {
            RLOGD("[BTSAP] requestBtSapResetSim but card was removed");
            rsp.response = RIL_SIM_SAP_RESET_SIM_RSP_Response_RIL_E_SIM_ABSENT;
        } else if (status == BT_SAP_POWER_OFF) {
            RLOGD("[BTSAP] requestBtSapResetSim off but card was already power off");
            rsp.response = RIL_SIM_SAP_RESET_SIM_RSP_Response_RIL_E_SIM_ALREADY_POWERED_OFF;
        } else {
            // Send disconnect request
            RIL_SIM_SAP_RESET_SIM_REQ *local_req =
                (RIL_SIM_SAP_RESET_SIM_REQ*)malloc(sizeof(RIL_SIM_SAP_RESET_SIM_REQ));
            assert(local_req != NULL);
            memset(local_req, 0, sizeof(RIL_SIM_SAP_RESET_SIM_REQ));
            memcpy(local_req, req, sizeof(RIL_SIM_SAP_RESET_SIM_REQ));
            LocalBtSapMsgHeader *localMsgHeader = allocateLocalBtSapMsgHeader(local_req, t, rid);
            RIL_requestProxyTimedCallback(requestLocalBtSapResetSim, localMsgHeader,
                    &TIMEVAL_0, p_cctx->id);
            break;
        }

        // FIXME: Send response immediately
        //RIL_SAP_onRequestComplete(t, ret, &rsp, sizeof(rsp));
        sendBtSapResponseComplete(t, ret, MsgId_RIL_SIM_SAP_RESET_SIM, &rsp);
    } while(0);

    free(req);

    RLOGD("[BTSAP] requestBtSapResetSim end");
}

void requestLocalBtSapResetSim(void *param) {
    LocalBtSapMsgHeader *localMsgHdr = (LocalBtSapMsgHeader*)param;
    RIL_Token t = localMsgHdr->t;
    RIL_SOCKET_ID rid = localMsgHdr->socket_id;
    RIL_SIM_SAP_RESET_SIM_RSP rsp;
    ATResponse *p_response = NULL;
    int err;
    int type = 0;
    char* cmd = NULL;
    char *line = NULL, *pAtr = NULL;
    RILChannelCtx *p_cctx = getChannelCtxbyProxy();
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;

    RLOGD("[BTSAP] requestLocalBtSapResetSim start, (%d)", rid);
    memset(&rsp, 0, sizeof(RIL_SIM_SAP_RESET_SIM_RSP));
    asprintf(&cmd, "AT+EBTSAP=4,%d", s_bt_sap_current_protocol[rid]);
    err = at_send_command_singleline(cmd, "+EBTSAP:", &p_response, p_cctx);
    free(cmd);

    if (err < 0 || NULL == p_response) {
        rsp.response = RIL_SIM_SAP_RESET_SIM_RSP_Response_RIL_E_GENERIC_FAILURE;
        goto format_error;
    }

    if (p_response->success == 0) {
        RLOGE("[BTSAP] CME ERROR = %d", at_get_cme_error(p_response));
        rsp.response = RIL_SIM_SAP_RESET_SIM_RSP_Response_RIL_E_GENERIC_FAILURE;
        switch (at_get_cme_error(p_response)) {
            case CME_BT_SAP_UIM_NOT_INSERTED:
                ret = RIL_E_BT_SAP_UIM_NOT_INSERTED;
                rsp.response = RIL_SIM_SAP_RESET_SIM_RSP_Response_RIL_E_SIM_ABSENT;
                break;
            default:
                break;
        }
        goto format_error;
    } else {
        line = p_response->p_intermediates->line;

        err = at_tok_start(&line);
        if (err < 0) goto format_error;

        err = at_tok_nextint(&line, &type);
        if (err < 0) goto  format_error;

        s_bt_sap_current_protocol[rid] = type;

        err = at_tok_nextstr(&line, &pAtr);
        if (err < 0) goto  format_error;

        int atrLen = 0;

        if (pAtr != NULL) {
            // Update ATR
            if (s_bt_sap_atr[rid] != NULL) {
                free(s_bt_sap_atr[rid]);
            }
            atrLen = strlen(pAtr);
            s_bt_sap_atr[rid] = (unsigned char*)malloc((atrLen+1)*sizeof(unsigned char));
            assert(s_bt_sap_atr[rid] != NULL);
            memset(s_bt_sap_atr[rid], 0, atrLen);
            memcpy(s_bt_sap_atr[rid], pAtr, atrLen);
        }

        RLOGD("[BTSAP] requestLocalBtSapResetSim, cur_type: %d, size: %d",
                s_bt_sap_current_protocol[rid], atrLen);
        ret = RIL_E_SUCCESS;
        rsp.response = RIL_SIM_SAP_RESET_SIM_RSP_Response_RIL_E_SUCCESS;
    }

    // Send response
    //RIL_SAP_onRequestComplete(t, ret, &rsp, sizeof(rsp));
    sendBtSapResponseComplete(t, ret, MsgId_RIL_SIM_SAP_RESET_SIM, &rsp);

    at_response_free(p_response);
    releaseLocalBtSapMsgHeader(localMsgHdr);
    RLOGD("[BTSAP] requestLocalBtSapResetSim end");
    return;
format_error:
    RLOGE("[BTSAP] Fail to Reset SIM");

    // Send response
    //RIL_SAP_onRequestComplete(t, ret, &rsp, sizeof(rsp));
    sendBtSapResponseComplete(t, ret, MsgId_RIL_SIM_SAP_RESET_SIM, &rsp);

    at_response_free(p_response);
    releaseLocalBtSapMsgHeader(localMsgHdr);
    RLOGD("[BTSAP] requestLocalBtSapResetSim end");
}

void requestBtSapSetTransferProtocol(void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID rid) {
    SIM_UNUSED(data);
    SIM_UNUSED(datalen);

    RIL_SIM_SAP_ERROR_RSP rsp;
    RIL_Errno ret = RIL_E_BT_SAP_ERROR_NO_REASON_DEFINED;

    RLOGD("[BTSAP] requestBtSapSetTransferProtocol start, (%d)", rid);
    rsp.dummy_field = 1;

    // Send response
    //RIL_SAP_onRequestComplete(t, ret, &rsp, sizeof(rsp));
    sendBtSapResponseComplete(t, ret, MsgId_RIL_SIM_SAP_ERROR_RESP, &rsp);
}

void sendBtSapResponseComplete(RIL_Token t, RIL_Errno ret, MsgId msgId,
        void *data) {
    SIM_UNUSED(t);
    SIM_UNUSED(ret);

    const pb_field_t *fields = NULL;
    size_t encoded_size = 0;
    size_t buffer_size = 0;
    pb_ostream_t ostream;
    bool success = false;

    RLOGD("[BTSAP] sendBtSapResponseComplete, start (%s)", btSapMsgIdToString(msgId));

    switch (msgId) {
        case MsgId_RIL_SIM_SAP_CONNECT:
            fields = RIL_SIM_SAP_CONNECT_RSP_fields;
            break;
        case MsgId_RIL_SIM_SAP_DISCONNECT:
            fields = RIL_SIM_SAP_DISCONNECT_RSP_fields;
            break;
        case MsgId_RIL_SIM_SAP_APDU:
            fields = RIL_SIM_SAP_APDU_RSP_fields;
            break;
        case MsgId_RIL_SIM_SAP_TRANSFER_ATR:
            fields = RIL_SIM_SAP_TRANSFER_ATR_RSP_fields;
            break;
        case MsgId_RIL_SIM_SAP_POWER:
            fields = RIL_SIM_SAP_POWER_RSP_fields;
            break;
        case MsgId_RIL_SIM_SAP_RESET_SIM:
            fields = RIL_SIM_SAP_RESET_SIM_RSP_fields;
            break;
        case MsgId_RIL_SIM_SAP_SET_TRANSFER_PROTOCOL:
            fields = RIL_SIM_SAP_SET_TRANSFER_PROTOCOL_RSP_fields;
            break;
        case MsgId_RIL_SIM_SAP_ERROR_RESP:
            fields = RIL_SIM_SAP_ERROR_RSP_fields;
            break;
        default:
            RLOGE("[BTSAP] sendBtSapResponseComplete, MsgId is mistake!");
            return;
    }

    if ((success = pb_get_encoded_size(&encoded_size, fields, data)) &&
            encoded_size <= INT32_MAX) {
        //buffer_size = encoded_size + sizeof(uint32_t);
        buffer_size = encoded_size;
        uint8_t buffer[buffer_size];
        //written_size = htonl((uint32_t) encoded_size);
        ostream = pb_ostream_from_buffer(buffer, buffer_size);
        //pb_write(&ostream, (uint8_t *)&written_size, sizeof(written_size));
        success = pb_encode(&ostream, fields, data);

        if(success) {
            RLOGD("[BTSAP] sendBtSapResponseComplete, Size: %lu", encoded_size);
            //FIXME: Send response
            RIL_SAP_onRequestComplete(t, ret, buffer, buffer_size);
        } else {
            RLOGE("[BTSAP] sendBtSapResponseComplete, Encode failed!");
        }
    } else {
        RLOGE("Not sending response type %d: encoded_size: %lu. encoded size result: %d",
                msgId, encoded_size, success);
    }
}

char* btSapMsgIdToString(MsgId msgId) {
    switch (msgId) {
        case MsgId_RIL_SIM_SAP_CONNECT:
            return "BT_SAP_CONNECT";
        case MsgId_RIL_SIM_SAP_DISCONNECT:
            return "BT_SAP_DISCONNECT";
        case MsgId_RIL_SIM_SAP_APDU:
            return "BT_SAP_TRANSFER_APDU";
        case MsgId_RIL_SIM_SAP_TRANSFER_ATR:
            return "BT_SAP_TRANSFER_ATR";
        case MsgId_RIL_SIM_SAP_POWER:
            return "BT_SAP_POWER";
        case MsgId_RIL_SIM_SAP_RESET_SIM:
            return "BT_SAP_RESET_SIM";
        case MsgId_RIL_SIM_SAP_SET_TRANSFER_PROTOCOL:
            return "BT_SAP_TRANSFER_PROTOCOL";
        case MsgId_RIL_SIM_SAP_ERROR_RESP:
            return "BT_SAP_ERROR_RESP";
        case MsgId_RIL_SIM_SAP_STATUS:
            return "BT_SAP_STATUS_IND";
        default:
            return "BT_SAP_UNKNOWN_MSG_ID";
    }
}

BtSapStatus queryBtSapStatus(RIL_SOCKET_ID rid) {
    RLOGD("[BTSAP] mBtSapStatus (%d, %d)", rid, s_bt_sap_status[rid]);
    return s_bt_sap_status[rid];
}

void resetBtSapContext(RIL_SOCKET_ID rid) {
    RLOGD("[BTSAP] resetBtSapAtr (%d)", rid);
    s_bt_sap_status[rid] = BT_SAP_INIT;
    s_bt_sap_current_protocol[rid] = 0;
    s_bt_sap_support_protocol[rid] = 0;
    if (s_bt_sap_atr[rid] != NULL) {
        free(s_bt_sap_atr[rid]);
        s_bt_sap_atr[rid] = NULL;
    }
}

void notifyBtSapStatusInd(RIL_SIM_SAP_STATUS_IND_Status message, RIL_SOCKET_ID rid) {
    int msgId = MsgId_RIL_SIM_SAP_STATUS;
    RIL_SIM_SAP_STATUS_IND unsolMsg;
    size_t encoded_size = 0;
    uint32_t written_size = 0;
    size_t buffer_size = 0;
    pb_ostream_t ostream;
    bool success = false;

    RLOGD("[BTSAP] notifyBtSapStatusInd, (%d, %d)", rid, message);
    unsolMsg.statusChange = message;

    if (message == RIL_SIM_SAP_STATUS_IND_Status_RIL_SIM_STATUS_CARD_RESET &&
        queryBtSapStatus(rid) == BT_SAP_CONNECTION_SETUP) {
        s_bt_sap_status[rid] = BT_SAP_ONGOING_CONNECTION;
    }

    // send unsolicited message STATUS_IND
    if ((success = pb_get_encoded_size(&encoded_size, RIL_SIM_SAP_STATUS_IND_fields,
            &unsolMsg)) && encoded_size <= INT32_MAX) {
        //buffer_size = encoded_size + sizeof(uint32_t);
        buffer_size = encoded_size;
        uint8_t buffer[buffer_size];
        //written_size = htonl((uint32_t) encoded_size);
        ostream = pb_ostream_from_buffer(buffer, buffer_size);
        //pb_write(&ostream, (uint8_t *)&written_size, sizeof(written_size));
        success = pb_encode(&ostream, RIL_SIM_SAP_STATUS_IND_fields, &unsolMsg);

        if(success) {
            RLOGD("[BTSAP] notifyBtSapStatusInd, Size: %lu Size as written: 0x%x",
                encoded_size, written_size);
            //FIXME: Send response
            RIL_SAP_onUnsolicitedResponse(MsgId_RIL_SIM_SAP_STATUS, buffer, buffer_size);
        } else {
            RLOGE("[BTSAP] notifyBtSapStatusInd, Encode failed!");
        }
    } else {
        RLOGE("Not sending response type %d: encoded_size: %lu. encoded size result: %d",
                msgId, encoded_size, success);
    }

}
// BT SIM Access Profile - END

static void requestCdmaGetSubscriptionSource(void *data, size_t datalen, RIL_Token t) {
    SIM_UNUSED(data);
    SIM_UNUSED(datalen);
    int err = 0;
    ATResponse *p_response = NULL;
    char *line = NULL;
    int state = 255;
    int source;

    err = at_send_command_singleline("AT+GETUIMST?", "+GETUIMST:", &p_response, SIM_CHANNEL_CTX);
    if ((err < 0) || (p_response == NULL) || (p_response->success == 0)) {
        goto error;
    }

    line = p_response->p_intermediates->line;
    LOGD("requestCdmaGetSubscriptionSource:  before at_tok_start line = %s", line);
    err = at_tok_start(&line);
    if (err < 0) {
        goto error;
    }
    err = at_tok_nextint(&line, &state);
    if (err < 0) {
        goto error;
    }
    // State value
    // 255: No card
    // 240: CDMA_SUBSCRIPTION_SOURCE_NV
    // Other values: CDMA_SUBSCRIPTION_SOURCE_RUIM_SIM
    LOGD("requestCdmaGetSubscriptionSource state = %d", state);
    if (state == 240) {
        source = SUBSCRIPTION_FROM_NV;
    } else {
        source = SUBSCRIPTION_FROM_RUIM;
    }
    RIL_onRequestComplete(t, RIL_E_SUCCESS, &source, sizeof(int));
    at_response_free(p_response);
    return;
    error: RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

int checkUserLoad() {
    int isUserLoad = 0;
    char property_value_emulation[PROPERTY_VALUE_MAX] = { 0 };
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("vendor.ril.emulation.userload", property_value_emulation, "0");
    if(strcmp("1", property_value_emulation) == 0) {
        return 1;
    }
    property_get("ro.build.type", property_value, "");
    isUserLoad = (strcmp("user", property_value) == 0);
    return isUserLoad;
}

void turnPSEnable(void *param)
{
    ATResponse *p_response = NULL;
    int err = 0;
    char *cmd = NULL;
    int radioOn = 0;
    RIL_RadioState newRadioState = getRadioState();
    PS_PARAM* psParm = (PS_PARAM *) param;

    LOGD("%s: force is %d, enable is %d, inemergency is %d", __FUNCTION__,
            psParm->force, psParm->enable, inemergency);
    if (!psParm->force && !psParm->enable && inemergency)
    {
        LOGD("%s: in emergency mode, do not close ps!", __FUNCTION__);
        return;
    }

    if (psParm->enable)
    {
        asprintf(&cmd, "%s", "AT+CPON");
        newRadioState = RADIO_STATE_ON;
    }
    else
    {
        asprintf(&cmd, "%s", "AT+CPOF");
        //newRadioState = RADIO_STATE_SIM_LOCKED_OR_ABSENT;
    }

    if (cmd == NULL) {
        LOGD("%s: command is null!", __FUNCTION__);
        return;
    }

    radioOn = isRadioOn();
    LOGD("%s: isEnable = %d, radioOn = %d", __FUNCTION__, psParm->enable, radioOn);
    if((psParm->enable && !radioOn) || (!psParm->enable && radioOn)) {
        if (psParm->enable) {
            combineDataAttach(getChannelCtxbyProxy());
        }
        err = at_send_command(cmd, &p_response, getChannelCtxbyProxy());
        if((err != 0) || (p_response->success == 0)) {
            LOGD("%s: failed to %s", __FUNCTION__, cmd);
        } else {
            setRadioState(newRadioState);
        }
        at_response_free(p_response);
    }

    free(cmd);
}

int dispatchCardStatus(VIA_RIL_CardStatus * p_card_status,
        RIL_CardStatus_v8 ** pp_oem_card_status) {
    int i;

    if (p_card_status == NULL || pp_oem_card_status == NULL) {
        return -1;
    }

    RIL_CardStatus_v8 *p_oem_card_status = malloc(sizeof(RIL_CardStatus_v8));
    assert(p_oem_card_status);
    p_oem_card_status->card_state = p_card_status->card_state;
    p_oem_card_status->universal_pin_state = p_card_status->universal_pin_state;
    p_oem_card_status->gsm_umts_subscription_app_index
            = p_card_status->gsm_umts_subscription_app_index;
    p_oem_card_status->cdma_subscription_app_index = p_card_status->cdma_subscription_app_index;
    p_oem_card_status->ims_subscription_app_index = p_card_status->cdma_subscription_app_index;
    p_oem_card_status->num_applications = p_card_status->num_applications;

    for (i = 0; i < RIL_CARD_MAX_APPS; i++) {
        p_oem_card_status->applications[i].app_type = p_card_status->applications[i].app_type;
        p_oem_card_status->applications[i].app_state = p_card_status->applications[i].app_state;
        p_oem_card_status->applications[i].perso_substate
                = p_card_status->applications[i].perso_substate;
        p_oem_card_status->applications[i].aid_ptr = p_card_status->applications[i].aid_ptr;
        p_oem_card_status->applications[i].app_label_ptr
                = p_card_status->applications[i].app_label_ptr;
        p_oem_card_status->applications[i].pin1_replaced
                = p_card_status->applications[i].pin1_replaced;
        p_oem_card_status->applications[i].pin1 = p_card_status->applications[i].pin1;
        p_oem_card_status->applications[i].pin2 = p_card_status->applications[i].pin2;
    }

    // Parameters add from radio hidl v1.2.
    p_oem_card_status->physicalSlotId = p_card_status->physicalSlotId;
    p_oem_card_status->atr = p_card_status->atr;
    p_oem_card_status->iccId = p_card_status->iccId;

    // Parameters add from radio hidl v1.4.
    p_oem_card_status->eid = p_card_status->eid;

    *pp_oem_card_status = p_oem_card_status;

    return 0;
}

static void requestGetAllowedCarriers(void *data, size_t datalen, RIL_Token t) {
    // sp<RfxMclMessage> response;
    // sp<RfxAtResponse> p_response;
    // RfxAtLine *p_cur = NULL;
    ATResponse*     p_response = NULL;
    ATLine          *p_cur;
    int             err = -1;
    RIL_CarrierRestrictionsWithPriority *result = NULL;
    char            *line;
    int             lockstate = 0;  // 1: locked state, 2: unlocked state, 3: disable state
    int             listtype = -1;  // 0: white list, 1: black list
    int             category = -1;
    int             tmp_len = 0;
    int             len[RIL_MATCH_MAX] = {RIL_MATCH_MAX};
    char            *tmp;
    int             i, j;
    int             k = 0;
    int             total_allowed_len = 0;
    int             total_excluded_len = 0;
    RIL_Carrier *allowed = NULL;
    RIL_Carrier **allowed_tmp = (RIL_Carrier **)calloc(RIL_MATCH_MAX, sizeof(RIL_Carrier*));
    RIL_Carrier *excluded = NULL;
    RIL_Carrier **excluded_tmp = (RIL_Carrier **)calloc(RIL_MATCH_MAX, sizeof(RIL_Carrier*));

    SIM_UNUSED(datalen);
    SIM_UNUSED(data);

    LOGI("requestGetAllowedCarriers IN!");
    err = at_send_command_multiline("AT+ECRRST?", "+ECRRST:", &p_response, SIM_CHANNEL_CTX);
    if (err < 0 || p_response->success == 0) {
            goto error;
    }

    result = (RIL_CarrierRestrictionsWithPriority *)calloc(1, sizeof(
            RIL_CarrierRestrictionsWithPriority));
    assert(result != NULL);

    p_cur = p_response->p_intermediates;
    line = p_cur->line;
    err = at_tok_start(&line);
    if (err < 0) {
        goto error;
    }
    LOGI("First Line Start");

    err = at_tok_nextint(&line, &lockstate);
    if (err < 0) {
        goto error;
    }
    LOGI("lockstate: %d", lockstate);

    err = at_tok_nextint(&line, &(result->simLockMultiSimPolicy));
    if (err < 0) {
        goto error;
    }
    LOGI("simLockMultiSimPolicy: %d", result->simLockMultiSimPolicy);

    err = at_tok_nextint(&line, &(result->allowedCarriersPrioritized));
    if (err < 0) {
        goto error;
    }
    LOGI("allowedCarriersPrioritized: %d", result->allowedCarriersPrioritized);

    // White list
    p_cur = p_cur->p_next;
    line = p_cur->line;
    err = at_tok_start(&line);
    if (err < 0) {
        goto error;
    }
    LOGI("Second Line Start");

    err = at_tok_nextint(&line, &listtype);
    if (err < 0) {
        goto error;
    }
    if (listtype != 0 && listtype != 1) {
        goto error;
    }
    LOGI("listtype: %d", listtype);

    // result->len_allowed_carriers = total_allowed_len;
    // allowed = calloc(total_allowed_len, sizeof(RIL_Carrier));
    for (i = 0; i < RIL_MATCH_MAX; i++) {
        err = at_tok_nextint(&line, &category);
        if (err < 0) {
            goto error;
        }
        LOGI("category: %d", category);

        err = at_tok_nextint(&line, &tmp_len);
        if (err < 0) {
            goto error;
        }
        LOGI("tmp_len: %d", tmp_len);

        len[i] = tmp_len;
        allowed_tmp[i] = (RIL_Carrier *)calloc(tmp_len, sizeof(RIL_Carrier));
        for (j = 0; j < tmp_len; j++) {
            RIL_Carrier *p_cr = allowed_tmp[i] + j;

            err = at_tok_nextstr(&line, &tmp);
            if (err < 0) {
                goto error;
            }
            LOGI("mccmnc: %s", tmp);

            char* tmp_mcc = (char *)alloca(4);
            memset(tmp_mcc, '\0', 4);
            strncpy(tmp_mcc, tmp, 3);
            // strcat(tmp_mcc, "\0");
            LOGI("tmp_mcc: %s, %zu", tmp_mcc, strlen(tmp_mcc));
            asprintf(&p_cr->mcc, "%s", tmp_mcc);
            LOGI("mcc: %s", p_cr->mcc);

            char* tmp_mnc = (char *)alloca(4);
            memset(tmp_mnc, '\0', 4);
            strncpy(tmp_mnc, tmp + 3, strlen(tmp) - 3);
            LOGI("tmp_mnc: %s, %zu", tmp_mcc, strlen(tmp_mcc));
            asprintf(&p_cr->mnc, "%s", tmp_mnc);
            LOGI("mnc: %s", p_cr->mnc);
            p_cr->match_type = (RIL_CarrierMatchType) category;
            if (category != 0) {
                err = at_tok_nextstr(&line, &tmp);
                if (err < 0) {
                    goto error;
                }
                LOGI("match_data: %s", tmp);
                asprintf(&p_cr->match_data, "%s", tmp);
            }
        }
        total_allowed_len += tmp_len;
    }

    result->len_allowed_carriers = total_allowed_len;
    result->allowed_carriers = allowed = (RIL_Carrier *)calloc(total_allowed_len,
            sizeof(RIL_Carrier));
    for (i = 0; i < RIL_MATCH_MAX; i++) {
        for (j = 0; j < len[i]; j++) {
            LOGI("i:%d, j:%d", i, j);
            RIL_Carrier *p_cr = allowed_tmp[i] + j;
            LOGI("*p_cur OK");
            RIL_Carrier *p_cr2 = allowed + (k++);
            LOGI("*p_cur2 OK");
            asprintf(&p_cr2->mcc, "%s", p_cr->mcc);
            LOGI("copy mcc OK");
            asprintf(&p_cr2->mnc, "%s", p_cr->mnc);
            LOGI("copy mnc OK");
            p_cr2->match_type = p_cr->match_type;
            asprintf(&p_cr2->match_data, "%s", p_cr->match_data);
        }
    }

    if (allowed_tmp != NULL) {
        for (i = 0; i < RIL_MATCH_MAX; i++) {
           if (allowed_tmp[i] != NULL) {
               free(allowed_tmp[i]);
               allowed_tmp[i] = NULL;
           }
        }
        free(allowed_tmp);
        allowed_tmp = NULL;
    }
    LOGI("Free allowed_tmp OK");

    // Black list.
    p_cur = p_cur->p_next;
    line = p_cur->line;
    err = at_tok_start(&line);
    if (err < 0) {
        goto error;
    }
    LOGI("Third Line Start");

    err = at_tok_nextint(&line, &listtype);
    if (err < 0) {
        goto error;
    }
    if (listtype != 0 && listtype != 1) {
        goto error;
    }
    LOGI("listtype: %d", listtype);

    // result->len_allowed_carriers = total_allowed_len;
    // allowed = calloc(total_allowed_len, sizeof(RIL_Carrier));
    for (i = 0; i < RIL_MATCH_MAX; i++) {
        err = at_tok_nextint(&line, &category);
        if (err < 0) {
            goto error;
        }
        LOGI("category: %d", category);

        err = at_tok_nextint(&line, &tmp_len);
        if (err < 0) {
            goto error;
        }
        LOGI("tmp_len: %d", tmp_len);

        len[i] = tmp_len;
        excluded_tmp[i] = (RIL_Carrier *)calloc(tmp_len, sizeof(RIL_Carrier));
        for (j = 0; j < tmp_len; j++) {
            RIL_Carrier *p_cr = excluded_tmp[i] + j;

            err = at_tok_nextstr(&line, &tmp);
            if (err < 0) {
                goto error;
            }
            LOGI("mccmnc: %s", tmp);

            char* tmp_mcc = (char *)alloca(4);
            memset(tmp_mcc, '\0', 4);
            strncpy(tmp_mcc, tmp, 3);
            // strcat(tmp_mcc, "\0");
            LOGI("tmp_mcc: %s, %zu", tmp_mcc, strlen(tmp_mcc));
            asprintf(&p_cr->mcc, "%s", tmp_mcc);
            LOGI("mcc: %s", p_cr->mcc);

            char* tmp_mnc = (char *)alloca(4);
            memset(tmp_mnc, '\0', 4);
            strncpy(tmp_mnc, tmp + 3, strlen(tmp) - 3);
            // strcat(tmp_mnc, "\0");
            LOGI("tmp_mnc: %s, %zu", tmp_mcc, strlen(tmp_mcc));
            asprintf(&p_cr->mnc, "%s", tmp_mnc);
            LOGI("mnc: %s", p_cr->mnc);
            p_cr->match_type = (RIL_CarrierMatchType) category;
            if (category != 0) {
                err = at_tok_nextstr(&line, &tmp);
                if (err < 0) {
                    goto error;
                }
                LOGI("match_data: %s", tmp);
                asprintf(&p_cr->match_data, "%s", tmp);
            }
        }
        total_excluded_len += tmp_len;
    }

    result->len_excluded_carriers = total_excluded_len;
    result->excluded_carriers = excluded = (RIL_Carrier *) calloc(total_excluded_len,
            sizeof(RIL_Carrier));
    for (i = 0; i < RIL_MATCH_MAX; i++) {
        for (j = 0; j < len[i]; j++) {
            LOGI("i:%d, j:%d", i, j);
            RIL_Carrier *p_cr = excluded_tmp[i] + j;
            LOGI("*p_cur OK");
            RIL_Carrier *p_cr2 = excluded + (k++);
            LOGI("*p_cur2 OK");
            asprintf(&p_cr2->mcc, "%s", p_cr->mcc);
            LOGI("copy mcc OK");
            asprintf(&p_cr2->mnc, "%s", p_cr->mnc);
            LOGI("copy mnc OK");
            p_cr2->match_type = p_cr->match_type;
            asprintf(&p_cr2->match_data, "%s", p_cr->match_data);
        }
    }

    if (excluded_tmp != NULL) {
        for (i = 0; i < RIL_MATCH_MAX; i++) {
           if (excluded_tmp[i] != NULL) {
               free(excluded_tmp[i]);
               excluded_tmp[i] = NULL;
           }
        }
        free(excluded_tmp);
        excluded_tmp = NULL;
    }
    LOGI("Free excluded_tmp OK");
    LOGI("Black list OK");

    LOGI("requestGetAllowedCarriers: allowed_len %d, excluded len %d",
         result->len_allowed_carriers, result->len_excluded_carriers);
    for (int i = 0; i < result->len_allowed_carriers; i++) {
        allowed = result->allowed_carriers + i;
        LOGI("allowed mcc %s, mnc %s, match_type %d, match_data %s",
                allowed->mcc, allowed->mnc, allowed->match_type, allowed->match_data);
    }
    for (int i = 0; i < result->len_excluded_carriers; i++) {
        excluded = result->excluded_carriers + i;
        LOGI("excluded mcc %s, mnc %s, match_type %d, match_data %s",
                excluded->mcc, excluded->mnc, excluded->match_type, excluded->match_data);
    }

    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, result, sizeof(RIL_CarrierRestrictionsWithPriority));
    free(result);
    result = NULL;
    return;

error:
    if (allowed_tmp != NULL) {
        for (i = 0; i < RIL_MATCH_MAX; i++) {
           if (allowed_tmp[i] != NULL) {
               free(allowed_tmp[i]);
               allowed_tmp[i] = NULL;
           }
        }
        free(allowed_tmp);
        allowed_tmp = NULL;
    }
    if (excluded_tmp != NULL) {
        for (i = 0; i < RIL_MATCH_MAX; i++) {
           if (excluded_tmp[i] != NULL) {
               free(excluded_tmp[i]);
               excluded_tmp[i] = NULL;
           }
        }
        free(excluded_tmp);
        excluded_tmp = NULL;
    }
    if (result != NULL) {
        free(result);
        result = NULL;
    }

    LOGI("requestGetAllowedCarriers Fail!");
    RIL_onRequestComplete(t, RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0);
    at_response_free(p_response);
}

static void requestSetAllowedCarriers(void *data, size_t datalen, RIL_Token t) {
    // sp<RfxAtResponse> p_response;
    // sp<RfxMclMessage> response;
    ATResponse*     p_response = NULL;
    char*           line;
    char*           cmd = NULL;
    int err = -1;
    // String8 cmd("");
    RIL_CarrierRestrictionsWithPriority *crp = (RIL_CarrierRestrictionsWithPriority*)data;
    RIL_Carrier *allowed = NULL;
    RIL_Carrier *excluded = NULL;
    int allowedCount = 0;
    int excludedCount = 0;
    int totalCount = 0;
    RUIM_Status sim_status = RUIM_ABSENT;

    // 1.wait sim status
    sim_status = getSIMStatusWithRetry();

    // 2.unlock
    /* Comment as modem issue.
    err = at_send_command("AT+ECRRST=0,\"12345678\"", &p_response, SIM_CHANNEL_CTX);
    if (err < 0 || p_response->success == 0) {
        logE(mTag, "requestSetAllowedCarriers AT+ECRRST=0 Fail, e= %d", err);
        err = RIL_E_GENERIC_FAILURE;
        goto error;
    }
    at_response_free(p_response);
    p_response = NULL; */

    // 3.Clear all
    err = at_send_command("AT+ECRRST=6", &p_response, SIM_CHANNEL_CTX);
    if (err < 0 || p_response == NULL || p_response->success == 0) {
        LOGE("requestSetAllowedCarriers AT+ECRRST=6 Fail, e= %d", err);
        goto error;
    }
    at_response_free(p_response);
    p_response = NULL;

    // 4.set multi SIM policy and prioritized.
    LOGI("requestSetAllowedCarriers MultiSimPolicy %d,  Prioritized %d",
            crp->simLockMultiSimPolicy, crp->allowedCarriersPrioritized);
    asprintf(&cmd, "AT+ECRRST=9,\"12345678\",%d,%d",
            crp->simLockMultiSimPolicy, crp->allowedCarriersPrioritized);
    err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
    free(cmd);
    cmd = NULL;
    if (err < 0 || p_response->success == 0) {
        LOGE("requestSetAllowedCarriers AT+ECRRST=9 Fail, e= %d", err);
        goto error;
    }
    at_response_free(p_response);
    p_response = NULL;

    // 5.set allowed list
    for (int i = 0; i < crp->len_allowed_carriers; i++) {
        allowed = crp->allowed_carriers + i;

        if (strcmp(allowed->mcc, "") == 0 && strcmp(allowed->mnc, "") == 0) {
            asprintf(&cmd, "AT+ECRRST=7");
        } else if (allowed->match_type == RIL_MATCH_ALL) {
            LOGI("requestSetAllowedCarriers match_type: %d, mcc mnc %s %s",
                    allowed->match_type, allowed->mcc, allowed->mnc);
            asprintf(&cmd, "AT+ECRRST=2,,0,%d,\"%s%s\"",
                    allowed->match_type, allowed->mcc, allowed->mnc);
        } else {
            LOGI("requestSetAllowedCarriers match_type: %d, mcc mnc %s %s, match_data: %s",
                    allowed->match_type, allowed->mcc, allowed->mnc, allowed->match_data);
            asprintf(&cmd, "AT+ECRRST=2,,0,%d,\"%s%s\",\"%s\"",
                    allowed->match_type, allowed->mcc, allowed->mnc, allowed->match_data);
        }

        err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
        free(cmd);
        cmd = NULL;
        if (err < 0 || p_response->success == 0) {
            LOGE("requestSetAllowedCarriers set white list %d Fail, e= %d", i, err);
        } else {
            allowedCount++;
        }
        at_response_free(p_response);
        p_response = NULL;
    }

    // 6.set excluded list
    for (int i = 0; i < crp->len_excluded_carriers; i++) {
        excluded = crp->excluded_carriers + i;

        if (excluded->match_type == RIL_MATCH_ALL) {
            asprintf(&cmd, "AT+ECRRST=2,,1,%d,\"%s%s\"",
                    excluded->match_type, excluded->mcc, excluded->mnc);
        } else {
            asprintf(&cmd, "AT+ECRRST=2,,1,%d,\"%s%s\",\"%s\"",
                    excluded->match_type, excluded->mcc, excluded->mnc, excluded->match_data);
        }

        err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
        free(cmd);
        cmd = NULL;
        if (err < 0 || p_response->success == 0) {
            LOGE("requestSetAllowedCarriers set black list %d Fail, e= %d", i, err);
        } else {
            excludedCount++;
        }
        at_response_free(p_response);
        p_response = NULL;
    }

    /* 7.lock
    err = at_send_command("AT+ECRRST=1", &p_response, SIM_CHANNEL_CTX);
    if (err < 0 || p_response->success == 0) {
        logE(mTag, "requestSetAllowedCarriers AT+ECRRST=1 Fail, e= %d", err);
        err = RIL_E_GENERIC_FAILURE;
        goto error;
    }
    at_response_free(p_response);
    p_response = NULL; */

    // 8.wait reboot
    sim_status = getSIMStatusWithRetry();

    // 9.Rset sim
    if ((sim_status == RUIM_CARD_REBOOT) || (sim_status == RUIM_RESTRICTED)) {
        err = at_send_command_singleline("AT+EBTSAP=0", "+EBTSAP:", &p_response, SIM_CHANNEL_CTX);
        if (err < 0 || NULL == p_response || at_get_cme_error(p_response) != CME_SUCCESS) {
            LOGE("requestSetAllowedCarriers AT+EBTSAP=0 Fail, e= %d", err);
            goto error;
        }
        at_response_free(p_response);
        p_response = NULL;

        err = at_send_command("AT+EBTSAP=1", &p_response, SIM_CHANNEL_CTX);
        if (err < 0 || NULL == p_response || at_get_cme_error(p_response) != CME_SUCCESS) {
            LOGE("requestSetAllowedCarriers AT+EBTSAP=1 Fail, e= %d", err);
            goto error;
        }
        at_response_free(p_response);
        p_response = NULL;
    }

    // check if all sucess
    if (allowedCount < crp->len_allowed_carriers || excludedCount < crp->len_excluded_carriers) {
        goto error;
    }

    // AOSP justs request allowedCount when hidl version < 1.4.
    // totalCount = allowedCount + excludedCount;
    RIL_onRequestComplete(t, RIL_E_SUCCESS, &allowedCount, sizeof(int));
    return;

error:

    LOGE("requestSetAllowedCarriers Fail");
    RIL_onRequestComplete(t, RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0);
    at_response_free(p_response);
}

