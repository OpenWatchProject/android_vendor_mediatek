/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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
#include <termios.h>
#include <cutils/properties.h>
#include <ril_callbacks.h>

#include <ril_sim.h>
#include "usim_fcp_parser.h"
#include <hardware/ril/librilutils/proto/sap-api.pb.h>
#include "pb_decode.h"
#include "pb_encode.h"
#include "base64.h"

#ifdef MTK_RIL_MD1
#define LOG_TAG "RIL-SIMLOCK"
#else
#define LOG_TAG "RILMD2-SIMLOCK"
#endif

#include <utils/Log.h>


#define PROPERTY_DATA_NETWORK_TYPE "gsm.network.type"

//RJIL subsidy lock start.
void requestSubsidyLockGetModemStatus(void *data, size_t datalen, RIL_Token t) {
    LOGD("[Subsidy lock] requestSubsidyLockGetModemStatus");

    ATResponse *p_response = NULL;
    int err = -1;
    char *cmd, *line = NULL;
    RIL_Errno ril_error = RIL_E_GENERIC_FAILURE;
    //char result[4];
    int result[2] = {0};
    result[0] = 2;
    result[1] = RIL_E_GENERIC_FAILURE;
    int response = 0;
    char str[16] = {0};

    RIL_SIM_UNUSED_PARM(data);
    RIL_SIM_UNUSED_PARM(datalen);

    asprintf(&cmd, "AT+ESLBLOB=2");
    err = at_send_command_singleline(cmd, "+ESLBLOB:", &p_response, getRILChannelCtxFromToken(t));
    free(cmd);

    if (err < 0 || NULL == p_response || NULL == p_response->p_intermediates || 0 == p_response->success) {
        LOGE("[Subsidy Lock] requestSubsidyLockGetModemStatus Fail with err: %d", err);
        if (p_response == NULL) {
            LOGE("[Subsidy Lock] requestSubsidyLockGetModemStatus response is NULL");
        } else {
            LOGE("[Subsidy Lock] requestSubsidyLockGetModemStatus response = %s",p_response);
        }
        ril_error = RIL_E_GENERIC_FAILURE;
        goto done;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start (&line);
    if (err < 0) {
        ril_error = RIL_E_GENERIC_FAILURE;
        LOGE("[Subsidy Lock] FAIL1: err = %d", err);
        goto done;
    }

    // Get modem status len
    // Return status meanings. 0 : PERMANENT_UNLOCK, 1: SUBSIDYLOCK
    err = at_tok_nextint(&line, &result[1]);
    if (err < 0) {
        ril_error = RIL_E_GENERIC_FAILURE;
         LOGE("[Subsidy Lock] FAIL2: err = %d", err);
        goto done;
    }

    ril_error = RIL_E_SUCCESS;
    LOGD("modem status ril_error = %d, result =%d", ril_error, result[1]);
    sprintf(str, "%d", result[1]);
    property_set("persist.vendor.subsidylock.lockstatus_ril", str);

done:
    LOGD("send response for getModeStatus ril_error = %d, status =%d", ril_error, result[1]);
    RIL_onRequestComplete(t, ril_error, result, sizeof(result));
    at_response_free(p_response);
}

void requestSubsidyLockUpdateSettings(void *data, size_t datalen, RIL_Token t) {
    char property_value_status[PROPERTY_VALUE_MAX] = {0};
    ATResponse *p_response = NULL;
    ATResponse *p_response1 = NULL;
    ATResponse *p_response2 = NULL;
    int result[2] = {0};
    result[0] = 1;
    int err;
    //const char** strings = (const char**)data;
    char *cmd;
    char *line = NULL;
    RIL_Errno ret = RIL_E_SUCCESS;

    // parse blob data to get lock status bit. 0 : SUBSIDYLOCK, 1: PERMANENT_UNLOCK
    int blob_status = ((char*)data)[7] - '0';
    LOGD("[Subsidy Lock] requestSubsidyLockUpdateSettings: blob_status = %d, data=%s",
            blob_status, ((char*)data));

    if(blob_status == 0) {
        LOGD("[SUBSIDY_LOCK]blob update request to SUBSIDYLOCK");
    } else {
        LOGD("[SUBSIDY_LOCK]blob update request to PERMANENT_UNLOCK");
    }

    asprintf(&cmd, "AT+ESLBLOB=1,%d,\"%s\"", strlen(data) + 1, ((char*)data));
    err = at_send_command(cmd, &p_response, getRILChannelCtxFromToken(t));
    free(cmd);

    if (err < 0 || NULL == p_response) {
        LOGE("[Subsidy Lock] requestSubsidyLockUpdateSettings Fail");
        ret = RIL_E_GENERIC_FAILURE;
        goto done;
    }

    if (0 == p_response->success) {
        switch (at_get_cme_error(p_response)) {
            LOGD("p_response = %d /n", at_get_cme_error(p_response) );
            case CME_BLOB_VALIDATION_FAILED:
                ret = RIL_E_VALIDATION_FAILED;
                LOGD("p_response: CME_BLOB_VALIDATION_FAILED");
                break;
            case CME_BLOB_CONFIG_DATA_INVALID:
                ret = RIL_E_CONFIG_DATA_INVALID;
                LOGD("p_response: CME_SIM_LOCK_BLOB_VERIFICATION_FAILED");
                break;
            case CME_BLOB_IMEI_MISMATCH:
                ret = RIL_E_IMEI_MISMATCH;
                LOGD("p_response: CME_SIM_LOCK_BLOB_VERIFICATION_FAILED");
                break;
            case CME_BLOB_LENGTH_MISMATCH:
                ret = RIL_E_LENGTH_MISMATCH;
                LOGD("p_response: CME_SIM_LOCK_BLOB_VERIFICATION_FAILED");
                break;
            case CME_UNKNOWN_ERROR:
                ret = RIL_E_UNKNOWN_ERROR;
                LOGD("p_response: CME_SIM_LOCK_BLOB_VERIFICATION_FAILED");
                break;
            case CME_REBOOT_REQUIRED:
                ret = RIL_E_SUCCESS;
                LOGD("p_response: CME_REBOOT_REQUEST");
                asprintf(&cmd, "AT+EBTSAP=0");
                err = at_send_command_singleline(cmd, "+EBTSAP:", &p_response1,
                                            getRILChannelCtxFromToken(t));
                if (err < 0 || NULL == p_response1) {
                    LOGE("AT+EBTSAP=0 Fail, err: %d", err);
                }

                free(cmd);
                at_response_free(p_response1);
                asprintf(&cmd, "AT+EBTSAP=1");
                err = at_send_command(cmd, &p_response2,
                                            getRILChannelCtxFromToken(t));
                if (err < 0 || NULL == p_response2) {
                    LOGE("AT+EBTSAP=1 Fail, err: %d", err);
                }
                free(cmd);
                at_response_free(p_response2);
                break;
            default:
                ret = RIL_E_GENERIC_FAILURE;
                goto done;
        }
    } else {
        LOGE("handleReqSubsidyLockUpdateSettings success, E_REBOOT_NOT_REQUIRED\n");
        ret = RIL_E_SUCCESS;
    }

    if(ret == RIL_E_SUCCESS) {
        LOGE("update blob_status property");
        if (blob_status == 1) {
            // PERMANENT UNLOCK CASE:
            property_set("persist.vendor.subsidylock.lockstatus_ril", "0");
        } else {
            // SUBSIDYLOCK CASE:
            property_set("persist.vendor.subsidylock.lockstatus_ril", "1");
        }
    }

done:
    result[1] = (int)ret;
    LOGD("send response for update blob: %d", result[1]);
    RIL_onRequestComplete(t, ret, result, sizeof(result));
    at_response_free(p_response);
}

//RJIL subsidy lock API end.

extern int rilSimLockMain(int request, void *data, size_t datalen, RIL_Token t)
{
    switch (request)
    {
        // Remote SIM ME Lock end
        //RJIL sublock start
        case RIL_REQUEST_GET_SUBLOCK_MODEM_STATUS:
            requestSubsidyLockGetModemStatus(data, datalen, t);
            break;
        case RIL_REQUEST_UPDATE_SUBLOCK_SETTINGS:
            requestSubsidyLockUpdateSettings(data, datalen, t);
            break;
        default:
            return 0; /* no match */
        break;
    }
    return 1; /* request find */
}
