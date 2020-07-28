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

#include "RfxBaseHandler.h"
#include "RfxSimIoRspData.h"
#include "RfxSimIoData.h"
#include "RfxSimAuthData.h"
#include "RfxSimGenAuthData.h"
#include "RfxSimOpenChannelData.h"
#include "RfxSimApduData.h"
#include "RfxVoidData.h"
#include "RfxStringData.h"
#include "RfxStringsData.h"
#include "RfxIntsData.h"
#include "RfxSimStatusData.h"
#include "RfxRawData.h"
#include "RmcSimBaseHandler.h"
#include "RmcPhbSimIoRequestHandler.h"
#include <telephony/mtk_ril.h>
#include "rfx_properties.h"
#include "RfxChannel.h"
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <cstring>
#include <pthread.h>
#include "RfxMessageId.h"
#include "RfxRilUtils.h"
#include "RfxLog.h"

#ifdef __cplusplus
extern "C"
{
#endif
#include <hardware_legacy/power.h>
#include "base64.h"
#include "usim_fcp_parser.h"
#include <vendor/mediatek/proprietary/hardware/ril/platformlib/common/libmtkrilutils/proto/sap-api.pb.h>
#include "pb_decode.h"
#include "pb_encode.h"
#ifdef __cplusplus
}
#endif

#define PHBIO_LOG_TAG "RmcPhbSimIo"
#define DlogD(x...) if (mIsEngLoad == 1) logD( x )

// register handler to channel
RFX_IMPLEMENT_HANDLER_CLASS(RmcPhbSimIoRequestHandler, RIL_CMD_PROXY_7);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxSimIoData, RfxSimIoRspData, RFX_MSG_REQUEST_PHB_SIM_IO);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxSimIoData, RfxSimIoRspData, RFX_MSG_REQUEST_PHB_PBR_SIM_IO);

RmcPhbSimIoRequestHandler::RmcPhbSimIoRequestHandler(int slot_id, int channel_id):RmcSimBaseHandler(slot_id, channel_id) {
    const int request[] = {
        RFX_MSG_REQUEST_PHB_SIM_IO,
        RFX_MSG_REQUEST_PHB_PBR_SIM_IO,
    };

    registerToHandleRequest(request, sizeof(request)/sizeof(int));
    mIsEngLoad = RfxRilUtils::isEngLoad();
}

RmcPhbSimIoRequestHandler::~RmcPhbSimIoRequestHandler() {
}

void RmcPhbSimIoRequestHandler::onHandleRequest(const sp<RfxMclMessage>& msg) {
    DlogD(PHBIO_LOG_TAG, "onHandleRequest: %d", msg->getId());
    int request = msg->getId();
    switch (request) {
        case RFX_MSG_REQUEST_PHB_SIM_IO:
        case RFX_MSG_REQUEST_PHB_PBR_SIM_IO:
            requestHandlePhbSimIo(msg);
            break;
        default:
            assert(0);
            break;
    }
}

void RmcPhbSimIoRequestHandler::makePhbSimRspFromUsimFcp(unsigned char ** simResponse) {
    int format_wrong = 0;
    unsigned char * fcpByte = NULL;
    unsigned short  fcpLen = 0;
    usim_file_descriptor_struct fDescriptor = {0,0,0,0};
    usim_file_size_struct fSize  = {0};
    unsigned char simRspByte[GET_RESPONSE_EF_SIZE_BYTES] = {0};

    fcpLen = hexStringToByteArray(*simResponse, &fcpByte);

    if (FALSE == usim_fcp_query_tag(fcpByte, fcpLen, FCP_FILE_DES_T, &fDescriptor)) {
        logE(PHBIO_LOG_TAG, "USIM FD Parse fail:%s", *simResponse);
        format_wrong = 1;
        goto done;
    }

    if ((!IS_DF_ADF(fDescriptor.fd)) && (FALSE == usim_fcp_query_tag(fcpByte, fcpLen, FCP_FILE_SIZE_T,&fSize))) {
        logE(PHBIO_LOG_TAG, "USIM File Size fail:%s", *simResponse);
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
    free(*simResponse);
    free(fcpByte);
    if (format_wrong != 1) {
        *simResponse = byteArrayToHexString(simRspByte, GET_RESPONSE_EF_SIZE_BYTES);
        if (RfxRilUtils::isEngLoad()) {
            logD(PHBIO_LOG_TAG, "simRsp done:%s", *simResponse);
        }
    } else {
        *simResponse = NULL;
        logE(PHBIO_LOG_TAG, "simRsp done, but simRsp is null because command format may be wrong");
    }

}

void RmcPhbSimIoRequestHandler::requestHandlePhbSimIo(const sp<RfxMclMessage>& msg) {
    int err = 0, intPara = 0;
    RIL_SIM_IO_v6 *pData = (RIL_SIM_IO_v6*)(msg->getData()->getData());
    String8 cmd("");
    String8 path("");
    String8 aid((pData->aidPtr != NULL)? pData->aidPtr : "");
    String8 data((pData->data != NULL)? pData->data : "");
    String8 simResponse("");
    int appTypeId = queryAppTypeId(aid), channelId = 0;
    int remain = pData->p3;
    RIL_Errno result = RIL_E_SIM_ERR;
    RfxAtLine *line = NULL;
    char *tmpStr = NULL;
    sp<RfxAtResponse> p_response = NULL;
    sp<RfxMclMessage> response;
    RIL_SIM_IO_Response sr;
    int64_t tSimSwitch = 0;
    int len = 0;
    int offset = pData->p1 * (1 << 8) + pData->p2;

    memset(&sr, 0, sizeof(RIL_SIM_IO_Response));

    //check timestampe between the request msg and the capability switch begin
    tSimSwitch = getNonSlotMclStatusManager()->getInt64Value(
            RFX_STATUS_KEY_SIM_SWITCH_RADIO_UNAVAIL_TIME);
    if (msg->getTimeStamp() <= tSimSwitch) {
        //check timestampe between the request msg and the capability switch begin
        result = RIL_E_INVALID_STATE;
        goto error;
    }

    if (pData->pin2 != NULL && strlen(pData->pin2) > 0) {
        logD(PHBIO_LOG_TAG, "requestHandlePhbSimIo pin2 %s", pData->pin2);
        cmd.append(String8::format("AT+EPIN2=\"%s\"", pData->pin2));
        p_response = atSendCommand(cmd.string());
        err = p_response->getError();

        if (p_response != NULL) {
            if (p_response->getSuccess() == 0) {
                switch (p_response->atGetCmeError()) {
                    case CME_SIM_PIN2_REQUIRED:
                        result = RIL_E_SIM_PIN2;
                        break;
                    case CME_SIM_PUK2_REQUIRED:
                        result = RIL_E_SIM_PUK2;
                        break;
                    default:
                        result = RIL_E_SIM_ERR;
                        break;
                }
            } else {
                // success case
                result = RIL_E_SUCCESS;
            }
        }

        p_response = NULL;
        cmd.clear();

        if (result != RIL_E_SUCCESS) {
            logE(PHBIO_LOG_TAG, "Fail to check PIN2");
            goto error;
        }
    }

    // SIM access
    // 1. Remove 3F00
    if (strstr(pData->path, "3F00") || strstr(pData->path, "3f00")) {
        if (strlen(pData->path) > 4) {
            path.append(String8::format("%s", pData->path + 4));
        }
    } else {
        path.append(String8::format("%s", pData->path));
    }

    // 2. Query channel id
    cmd.append(String8::format("AT+ESIMAPP=%d,0", appTypeId));
    p_response = atSendCommandSingleline(cmd.string(), "+ESIMAPP:");

    if (p_response != NULL && p_response->getSuccess() > 0) {
        // Success
        line = p_response->getIntermediates();
        line->atTokStart(&err);

        // application id
        intPara = line->atTokNextint(&err);

        // channel id
        channelId = line->atTokNextint(&err);
        if (channelId == 255) {
            logE(PHBIO_LOG_TAG, "channel id is 255!");
            result = RIL_E_SIM_BUSY;
            goto error;
        }

        p_response = NULL;
        cmd.clear();
        line = NULL;
    } else {
        logE(PHBIO_LOG_TAG, "Fail to get app type %d channel id!", appTypeId);
        result = RIL_E_SIM_ERR;
        goto error;
    }

    DlogD(PHBIO_LOG_TAG, "SIM_IO(%d, %d, %d, %d, %d, %d, %s)", channelId,
            pData->command, pData->fileid, pData->p1, pData->p2, pData->p3, pData->path);

    while (remain > 0) {
        // 3. Send AT+ECRLA
        result = RIL_E_SIM_ERR;
        cmd.clear();

        if (pData->command == 192) {
            // GET RESPONSE
            cmd.append(String8::format("AT+ECRLA=%d,%d,%d,%d,%d,%d,%d", appTypeId, channelId,
                    pData->command, pData->fileid, pData->p1, pData->p2, 0 /*P3*/));
        } else if (data.isEmpty()) {
            // READ COMMAND, P3: Max value is 256 and use 0 to represent 256
            len = ((remain < 256) ? remain : 256);
            cmd.append(String8::format("AT+ECRLA=%d,%d,%d,%d,%d,%d,%d", appTypeId, channelId,
                    pData->command, pData->fileid, (0xFF & (offset >> 8))/*P1*/,
                    (0xFF & offset)/*P2*/,
                    ((remain < 256) ? remain : 0)/*P3*/));
        } else {
            // WRITE COMMAND, P3: Max value is 255
            len = (remain > 255) ? 255 : remain;

            cmd.append(String8::format("AT+ECRLA=%d,%d,%d,%d,%d,%d,%d", appTypeId, channelId,
                    pData->command, pData->fileid, (0xFF & (offset >> 8))/*P1*/,
                    (0xFF & offset)/*P2*/,
                    len/*P3*/));
        }

        if ((!data.isEmpty()) && (pData->data != NULL)) {
            // Command type WRITE
            // Append <data>
            tmpStr = (char*)calloc(1, sizeof(char) * 512);
            RFX_ASSERT(tmpStr != NULL);
            strncpy(tmpStr, pData->data + 2 * (pData->p3 - remain), 2 * len);
            cmd.append(String8::format(",\"%s\"", tmpStr));
            free(tmpStr);
            tmpStr = NULL;

            offset += len;
            remain -= len;
        } else if (!path.isEmpty()) {
            // append comma only if there is path, this is for data field
            cmd.append(",");
        }

        if (!path.isEmpty()) {
            // Append <path>
            cmd.append(String8::format(",\"%s\"", path.string()));
        }

        p_response = atSendCommandSingleline(cmd.string(), "+ECRLA:");

        if (p_response != NULL && p_response->getSuccess() == 1) {
            // Success
            line = p_response->getIntermediates();

            line->atTokStart(&err);
            if (err < 0) {
                goto error;
            }

            sr.sw1 = line->atTokNextint(&err);
            if (err < 0) {
                goto error;
            }

            sr.sw2 = line->atTokNextint(&err);
            if (err < 0) {
                goto error;
            }

            if (line->atTokHasmore()) {
                char *pRes = NULL;
                pRes = line->atTokNextstr(&err);
                if (err < 0) {
                    goto error;
                }
                simResponse.append(String8::format("%s", pRes));

                if (pData->command == 192) {
                    // In the case of GET_RESPONSE, we use 0 instead of pData->p3 so we set reamin as 0
                    // directly.
                    remain = 0;
                } else if (pData->data == NULL) {
                    // Case of command type READ
                    offset += len;
                    remain -= len;
                    len = ((remain < 256) ? remain : 256);
                }

                if (remain == 0) {
                    // no more SIM acess
                    asprintf(&sr.simResponse, "%s", simResponse.string());
                }
                // Run FCP parser for USIM and CSIM when the command is GET_RESPONSE
                if ((appTypeId == UICC_APP_USIM || appTypeId == UICC_APP_CSIM ||
                        appTypeId == UICC_APP_ISIM || isSimIoFcp(sr.simResponse)) &&
                        pData->command == 192 /* GET_RESPONSE */) {
                    makePhbSimRspFromUsimFcp((unsigned char**)&sr.simResponse);
                }

            } else if (pData->data == NULL) {
                // The command type is READ, but can't read data
                remain = 0;
            }
            result = RIL_E_SUCCESS;
        } else {
            // Fail to do SIM_IO
            remain = 0;
        }
    }

    if (RfxRilUtils::isEngLoad()) {
        logI(PHBIO_LOG_TAG, "SIM_IO(result %d, sw1 %d, sw2 %d, response %s)", result, sr.sw1, sr.sw2,
                sr.simResponse);
    }

    response = RfxMclMessage::obtainResponse(msg->getId(), result,
            RfxSimIoRspData((void*)&sr, sizeof(sr)), msg, false);

    responseToTelCore(response);
    p_response = NULL;

    if (sr.simResponse != NULL) {
        free(sr.simResponse);
    }
    return;
error:
    response = RfxMclMessage::obtainResponse(msg->getId(), result,
            RfxSimIoRspData((void*)&sr, sizeof(sr)), msg, false);

    responseToTelCore(response);
    p_response = NULL;

    if (sr.simResponse != NULL) {
        free(sr.simResponse);
    }

}

