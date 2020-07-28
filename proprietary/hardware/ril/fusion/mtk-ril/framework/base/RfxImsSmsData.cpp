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

#include "RfxImsSmsData.h"
#include "RfxLog.h"
#include <telephony/mtk_ril.h>

RFX_IMPLEMENT_DATA_CLASS(RfxImsSmsData);

RfxImsSmsData::RfxImsSmsData(void *_data, int _length) : RfxBaseData(_data, _length) {
    if (_data != NULL) {
        RIL_IMS_SMS_Message *pIms = (RIL_IMS_SMS_Message*)_data;

        if (pIms->tech == RADIO_TECH_3GPP) {
            copyImsGsmSmsData(_data, _length);
        } else {
            copyImsCdmaSmsData(_data, _length);
        }
    }
}

RfxImsSmsData::~RfxImsSmsData() {
    // free memory
    if (m_data != NULL) {
        RIL_IMS_SMS_Message *pIms = (RIL_IMS_SMS_Message*)m_data;
        if (pIms->tech == RADIO_TECH_3GPP) {
            releaseImsGsmSmsData();
        } else {
            releaseImsCdmaSmsData();
        }
    }
}

void RfxImsSmsData::copyImsGsmSmsData(void *data, int length) {
    RIL_IMS_SMS_Message *pIms = (RIL_IMS_SMS_Message*)data;
    RIL_IMS_SMS_Message *pData = (RIL_IMS_SMS_Message *)calloc(1, sizeof(RIL_IMS_SMS_Message));
    char** pStrs = pIms->message.gsmMessage;
    char** pDstStrs = NULL;
    int count = GSM_SMS_MESSAGE_STRS_COUNT;

    RFX_ASSERT(pData != NULL);
    pData->tech = pIms->tech;
    pData->messageRef = pIms->messageRef;

    if (pStrs != NULL) {
        pDstStrs = (char**)calloc(count, sizeof(char *));
        RFX_ASSERT(pDstStrs != NULL);
        for (int i = 0; i < count; i++) {
            if (pStrs[i] != NULL) {
                asprintf(&pDstStrs[i], "%s", pStrs[i]);
            }
        }
        pData->message.gsmMessage = pDstStrs;
    }

    m_data = (void*)pData;
    m_length = length;
}

void RfxImsSmsData::releaseImsGsmSmsData() {
    if (m_data != NULL) {
        RIL_IMS_SMS_Message *pIms = (RIL_IMS_SMS_Message*)m_data;
        char** pStrs = pIms->message.gsmMessage;
        int count = GSM_SMS_MESSAGE_STRS_COUNT;

        if (pStrs != NULL) {
            for (int i = 0; i < count; i++) {
                if (pStrs[i] != NULL) {
                    free(pStrs[i]);
                }
            }

            free(pStrs);
        }

        free(m_data);
        m_data = NULL;
    }
}

void RfxImsSmsData::copyImsCdmaSmsData(void *data, int length) {
    RIL_IMS_SMS_Message *pIms = (RIL_IMS_SMS_Message*)data;
    RIL_IMS_SMS_Message *pData = (RIL_IMS_SMS_Message *)calloc(1, sizeof(RIL_IMS_SMS_Message));
    RFX_ASSERT(pData != NULL);
    pData->tech = pIms->tech;
    pData->messageRef = pIms->messageRef;
    if (pIms->message.cdmaMessage != NULL) {
        pData->message.cdmaMessage =
            (RIL_CDMA_SMS_Message *)malloc(sizeof(RIL_CDMA_SMS_Message));
        RFX_ASSERT(pData->message.cdmaMessage != NULL);
        *(pData->message.cdmaMessage) = *(pIms->message.cdmaMessage);
    }
    m_data = (void*)pData;
    m_length = length;
}

void RfxImsSmsData::releaseImsCdmaSmsData() {
    if (m_data != NULL) {
        RIL_IMS_SMS_Message *pIms = (RIL_IMS_SMS_Message*)m_data;
        free(pIms->message.cdmaMessage);
        pIms->message.cdmaMessage = NULL;
        free(m_data);
        m_data = NULL;
    }
}
