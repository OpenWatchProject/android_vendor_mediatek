/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
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

#include "RfxEmergencyNumberListData.h"

#define RFX_LOG_TAG "RfxEccListData"

RFX_IMPLEMENT_DATA_CLASS(RfxEmergencyNumberListData);

RfxEmergencyNumberListData::RfxEmergencyNumberListData(void *data, int length) :
        RfxBaseData(data, length) {
    m_length = length;
    copyEmergencyNumberListData((RIL_EmergencyNumber*)data);
}

void RfxEmergencyNumberListData::copyEmergencyNumberListData(RIL_EmergencyNumber *data) {
    int num = m_length / sizeof(RIL_EmergencyNumber);
    if (num == 0 || data == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%s] The number of responses is %d", __FUNCTION__, num);
        return;
    }
    RIL_EmergencyNumber *pData = (RIL_EmergencyNumber *)calloc(1, m_length);
    RFX_ASSERT(pData != NULL);
    for (int i = 0; i < num; i++) {
        asprintf(&(pData[i].number), "%s", data[i].number);
        asprintf(&(pData[i].mcc), "%s", data[i].mcc);
        asprintf(&(pData[i].mnc), "%s", data[i].mnc);
        pData[i].categories = data[i].categories;
        pData[i].urns = NULL; // not used yet
        pData[i].sources = data[i].sources;
    }
    m_data = pData;
}

RfxEmergencyNumberListData::~RfxEmergencyNumberListData() {
    // free memory
    if (m_data != NULL) {
        RIL_EmergencyNumber *pData = (RIL_EmergencyNumber *)m_data;
        int num = m_length / sizeof(RIL_EmergencyNumber);
        for (int i = 0; i < num; i++) {
            FREEIF(pData[i].number);
            FREEIF(pData[i].mcc);
            FREEIF(pData[i].mnc);
        }
        free(m_data);
    }
    m_data = NULL;
    m_length = 0;
}