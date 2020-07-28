/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2018. All rights reserved.
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

#include "RfxSimSlotStatusData.h"
#include <telephony/mtk_ril.h>

RFX_IMPLEMENT_DATA_CLASS(RfxSimSlotStatusData);

RfxSimSlotStatusData::RfxSimSlotStatusData(void *_data, int _length) : RfxBaseData(_data, _length) {
    if (_data == NULL) {
        m_data = NULL;
        return;
    }

    m_data = NULL;
    m_length = _length;
    int countOfSlot = m_length / sizeof(RIL_SimSlotStatus *);
    RIL_SimSlotStatus ** pp_slot_status = (RIL_SimSlotStatus **)calloc(countOfSlot,
            sizeof(RIL_SimSlotStatus *));
    RFX_ASSERT(pp_slot_status != NULL);
    if (countOfSlot > 0) {
        RIL_SimSlotStatus ** tmp = (RIL_SimSlotStatus **)_data;
        for (int i = 0; i < countOfSlot; i++) {
            pp_slot_status[i] = (RIL_SimSlotStatus *)calloc(1, sizeof(RIL_SimSlotStatus));
            RFX_ASSERT(pp_slot_status[i] != NULL);
            memset(pp_slot_status[i], 0, sizeof(RIL_SimSlotStatus));
            memcpy(pp_slot_status[i], tmp[i], sizeof(RIL_SimSlotStatus));

            pp_slot_status[i]->card_state = tmp[i]->card_state;
            pp_slot_status[i]->slotState = tmp[i]->slotState;
            pp_slot_status[i]->logicalSlotId = tmp[i]->logicalSlotId;
            if (tmp[i]->atr != NULL) {
                int len = strlen(tmp[i]->atr);
                pp_slot_status[i]->atr = (char *)calloc(len + 1, sizeof(char));
                RFX_ASSERT(pp_slot_status[i]->atr != NULL);
                strncpy(pp_slot_status[i]->atr, tmp[i]->atr, len);
            }
            if (tmp[i]->iccId != NULL) {
                int len = strlen(tmp[i]->iccId);
                pp_slot_status[i]->iccId = (char *)calloc(len + 1, sizeof(char));
                RFX_ASSERT(pp_slot_status[i]->iccId != NULL);
                strncpy(pp_slot_status[i]->iccId, tmp[i]->iccId, len);
            }
            if (tmp[i]->eid != NULL) {
                int len = strlen(tmp[i]->eid);
                pp_slot_status[i]->eid = (char *)calloc(len + 1, sizeof(char));
                RFX_ASSERT(pp_slot_status[i]->eid != NULL);
                strncpy(pp_slot_status[i]->eid, tmp[i]->eid, len);
            }
        }
    }
    m_data = (void*)pp_slot_status;
    m_length = _length;
}

RfxSimSlotStatusData::~RfxSimSlotStatusData() {
    int countOfSlot = m_length / sizeof(RIL_SimSlotStatus *);
    RIL_SimSlotStatus ** pp_slot_status = (RIL_SimSlotStatus **)m_data;
    if (pp_slot_status != NULL) {
        for (int i = 0; i < countOfSlot; i++) {
            if (pp_slot_status[i] != NULL) {
                if (pp_slot_status[i]->atr != NULL) {
                    free(pp_slot_status[i]->atr);
                }
                if (pp_slot_status[i]->iccId != NULL) {
                    free(pp_slot_status[i]->iccId);
                }
                if (pp_slot_status[i]->eid != NULL) {
                    free(pp_slot_status[i]->eid);
                }
                free(pp_slot_status[i]);
            }
        }
        free(pp_slot_status);
    }
    m_data = NULL;
}
