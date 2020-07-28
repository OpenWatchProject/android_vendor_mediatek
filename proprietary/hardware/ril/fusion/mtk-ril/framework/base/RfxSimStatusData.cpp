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

#include "RfxSimStatusData.h"
#include <telephony/mtk_ril.h>

RFX_IMPLEMENT_DATA_CLASS(RfxSimStatusData);

RfxSimStatusData::RfxSimStatusData(void *_data, int _length) : RfxBaseData(_data, _length) {
    if (_data != NULL) {
        RIL_CardStatus_v8 *pCard = (RIL_CardStatus_v8*)_data;
        RIL_CardStatus_v8 *pData = (RIL_CardStatus_v8*)calloc(1, sizeof(RIL_CardStatus_v8));
        RFX_ASSERT(pData != NULL);
        pData->card_state = pCard->card_state;
        pData->universal_pin_state = pCard->universal_pin_state;
        pData->gsm_umts_subscription_app_index = pCard->gsm_umts_subscription_app_index;
        pData->ims_subscription_app_index = pCard->ims_subscription_app_index;
        pData->cdma_subscription_app_index = pCard->cdma_subscription_app_index;
        pData->num_applications = pCard->num_applications;

        for (int i = 0; i < pData->num_applications; i++) {
            pData->applications[i].app_type = pCard->applications[i].app_type;
            pData->applications[i].app_state = pCard->applications[i].app_state;
            pData->applications[i].perso_substate = pCard->applications[i].perso_substate;
            if (pCard->applications[i].aid_ptr != NULL) {
                asprintf(&pData->applications[i].aid_ptr, "%s", pCard->applications[i].aid_ptr);
            }
            if (pCard->applications[i].app_label_ptr != NULL) {
                asprintf(&pData->applications[i].app_label_ptr,
                        "%s", pCard->applications[i].app_label_ptr);
            }
            pData->applications[i].pin1 = pCard->applications[i].pin1;
            pData->applications[i].pin1_replaced = pCard->applications[i].pin1_replaced;
            pData->applications[i].pin2 = pCard->applications[i].pin2;
        }

        // Parameter add from android radio hidl v1.2
        pData->physicalSlotId = pCard->physicalSlotId;
        if (pCard->atr != NULL) {
            asprintf(&pData->atr, "%s", pCard->atr);
        }
        if (pCard->iccId != NULL) {
            asprintf(&pData->iccId, "%s", pCard->iccId);
        }

        // Parameter add from android radio hidl v1.4
        if (pCard->eid!= NULL) {
            asprintf(&pData->eid, "%s", pCard->eid);
        }

        m_data = (void*)pData;
        m_length = _length;
    }
}

RfxSimStatusData::~RfxSimStatusData() {
    if (m_data != NULL) {
        RIL_CardStatus_v8 *pData = (RIL_CardStatus_v8*)m_data;

        for (int i = 0; i < pData->num_applications; i++) {
            if (pData->applications[i].aid_ptr != NULL) {
                free(pData->applications[i].aid_ptr);
                pData->applications[i].aid_ptr = NULL;
            }
            if (pData->applications[i].app_label_ptr != NULL) {
                free(pData->applications[i].app_label_ptr);
                pData->applications[i].app_label_ptr = NULL;
            }
        }

        // Parameter add from android radio hidl v1.2
        if (pData->atr != NULL) {
            free(pData->atr);
            pData->atr = NULL;
        }
        if (pData->iccId != NULL) {
            free(pData->iccId);
            pData->iccId = NULL;
        }

        // Parameter add from android radio hidl v1.4
        if (pData->eid != NULL) {
            free(pData->eid);
            pData->eid = NULL;
        }

        // free memory
        free(m_data);
        m_data = NULL;
    }
}
