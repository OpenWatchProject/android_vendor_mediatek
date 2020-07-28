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

#include "RfxDataCallResponseData.h"

#define RFX_LOG_TAG "RfxDataCallResponseData"

RFX_IMPLEMENT_DATA_CLASS(RfxDataCallResponseData);

RfxDataCallResponseData::RfxDataCallResponseData(void *data, int length)
    : RfxBaseData(data, length) {
    m_length = length;
    copyDataCallResponseData((MTK_RIL_Data_Call_Response_v11*)data);
}

RfxDataCallResponseData::RfxDataCallResponseData(MTK_RIL_Data_Call_Response_v11 *data, int num)
    : RfxBaseData(data, num * sizeof(MTK_RIL_Data_Call_Response_v11)) {
    m_length = num * sizeof(MTK_RIL_Data_Call_Response_v11);
    copyDataCallResponseData(data);
}

void RfxDataCallResponseData::copyDataCallResponseData(MTK_RIL_Data_Call_Response_v11 *data) {
    int num = m_length / sizeof(MTK_RIL_Data_Call_Response_v11);
    if (num == 0 || data == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%s] The number of responses is %d (if num != 0, means data is null)",
                __FUNCTION__, num);
        return;
    }

    MTK_RIL_Data_Call_Response_v11 *tmpPtr = (MTK_RIL_Data_Call_Response_v11 *)calloc(1, m_length);
    RFX_ASSERT(tmpPtr != NULL);
    MTK_RIL_Data_Call_Response_v11 *curPtr = data;

    for (int i = 0; i < num; i++) {
        tmpPtr[i].status = curPtr[i].status;
        tmpPtr[i].suggestedRetryTime = curPtr[i].suggestedRetryTime;
        tmpPtr[i].cid = curPtr[i].cid;
        tmpPtr[i].active = curPtr[i].active;
        tmpPtr[i].mtu = curPtr[i].mtu;
        tmpPtr[i].rat = curPtr[i].rat;

        if ((&curPtr[i])->type != NULL) {
            (&tmpPtr[i])->type = (char *)calloc(1, (strlen((&curPtr[i])->type) * sizeof(char)) + 1);
            RFX_ASSERT((&tmpPtr[i])->type != NULL);
            strncpy((&tmpPtr[i])->type, (&curPtr[i])->type, strlen((&curPtr[i])->type));
        }

        if ((&curPtr[i])->ifname != NULL) {
            (&tmpPtr[i])->ifname = (char *)calloc(1, (strlen((&curPtr[i])->ifname) * sizeof(char)) + 1);
            RFX_ASSERT((&tmpPtr[i])->ifname != NULL);
            strncpy((&tmpPtr[i])->ifname, (&curPtr[i])->ifname, strlen((&curPtr[i])->ifname));
        }

        if ((&curPtr[i])->addresses != NULL) {
            (&tmpPtr[i])->addresses = (char *)calloc(1, (strlen((&curPtr[i])->addresses) * sizeof(char)) + 1);
            RFX_ASSERT((&tmpPtr[i])->addresses != NULL);
            strncpy((&tmpPtr[i])->addresses, (&curPtr[i])->addresses, strlen((&curPtr[i])->addresses));
        }

        if ((&curPtr[i])->dnses != NULL) {
            (&tmpPtr[i])->dnses = (char *)calloc(1, (strlen((&curPtr[i])->dnses) * sizeof(char)) + 1);
            RFX_ASSERT((&tmpPtr[i])->dnses != NULL);
            strncpy((&tmpPtr[i])->dnses, (&curPtr[i])->dnses, strlen((&curPtr[i])->dnses));
        }

        if ((&curPtr[i])->gateways != NULL) {
            (&tmpPtr[i])->gateways = (char *)calloc(1, (strlen((&curPtr[i])->gateways) * sizeof(char)) + 1);
            RFX_ASSERT((&tmpPtr[i])->gateways != NULL);
            strncpy((&tmpPtr[i])->gateways, (&curPtr[i])->gateways, strlen((&curPtr[i])->gateways));
        }

        if ((&curPtr[i])->pcscf != NULL) {
            (&tmpPtr[i])->pcscf = (char *)calloc(1, (strlen((&curPtr[i])->pcscf) * sizeof(char)) + 1);
            RFX_ASSERT((&tmpPtr[i])->pcscf != NULL);
            strncpy((&tmpPtr[i])->pcscf, (&curPtr[i])->pcscf, strlen((&curPtr[i])->pcscf));
        }
    }
    m_data = tmpPtr;
}

RfxDataCallResponseData::~RfxDataCallResponseData() {
    MTK_RIL_Data_Call_Response_v11 *tmpPtr = (MTK_RIL_Data_Call_Response_v11 *)m_data;
    int num = m_length / sizeof(MTK_RIL_Data_Call_Response_v11);

    if (tmpPtr != NULL) {
        for (int i = 0; i < num; i++) {
            FREEIF((&tmpPtr[i])->type);
            FREEIF((&tmpPtr[i])->ifname);
            FREEIF((&tmpPtr[i])->addresses);
            FREEIF((&tmpPtr[i])->gateways);
            FREEIF((&tmpPtr[i])->dnses);
            FREEIF((&tmpPtr[i])->pcscf);
        }
        free(tmpPtr);
        tmpPtr = NULL;
    }
}
