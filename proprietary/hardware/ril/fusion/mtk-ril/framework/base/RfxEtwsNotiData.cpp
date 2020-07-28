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

#include "RfxEtwsNotiData.h"
#include <telephony/mtk_ril.h>

#define LOG_TAG "ETWS"

RFX_IMPLEMENT_DATA_CLASS(RfxEtwsNotiData);

RfxEtwsNotiData::RfxEtwsNotiData(void *_data, int _length) : RfxBaseData(_data, _length) {
    if (_data != NULL) {
        RIL_CBEtwsNotification *pEtwsNoti = (RIL_CBEtwsNotification*)_data;
        RIL_CBEtwsNotification *pData = (RIL_CBEtwsNotification *)calloc(1,
                sizeof(RIL_CBEtwsNotification));

        if (pData != NULL) {
            pData->warningType = pEtwsNoti->warningType;
            pData->serialNumber = pEtwsNoti->serialNumber;
            pData->messageId = pEtwsNoti->messageId;

            if (pEtwsNoti->plmnId != NULL) {
                asprintf(&pData->plmnId, "%s", pEtwsNoti->plmnId);
            }
            if (pEtwsNoti->securityInfo != NULL) {
                asprintf(&pData->securityInfo, "%s", pEtwsNoti->securityInfo);
            }
        }

        m_data = (void*)pData;
        m_length = _length;
    }
}

RfxEtwsNotiData::~RfxEtwsNotiData() {
    // free memory
    if (m_data != NULL) {
        RIL_CBEtwsNotification *pData = (RIL_CBEtwsNotification*)m_data;
        if (pData->plmnId != NULL) {
            free(pData->plmnId);
            pData->plmnId = NULL;
        }
        if (pData->securityInfo != NULL) {
            free(pData->securityInfo);
            pData->securityInfo = NULL;
        }
        free(m_data);
        m_data = NULL;
    }
}