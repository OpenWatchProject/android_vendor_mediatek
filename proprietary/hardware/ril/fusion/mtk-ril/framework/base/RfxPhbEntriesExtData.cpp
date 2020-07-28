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

#include "RfxPhbEntriesExtData.h"
#include "RfxLog.h"
#include <string.h>

RFX_IMPLEMENT_DATA_CLASS(RfxPhbEntriesExtData);

RfxPhbEntriesExtData::RfxPhbEntriesExtData() : RfxBaseData(NULL, 0) {
}

RfxPhbEntriesExtData::RfxPhbEntriesExtData(void *data, int length) : RfxBaseData(data, length)  {
    m_length = length;
    int countEntries = length/sizeof(RIL_PHB_ENTRY*);
    copyPhbEntries((RIL_PHB_ENTRY **)data, countEntries);
}

RfxPhbEntriesExtData::RfxPhbEntriesExtData(RIL_PHB_ENTRY **data, int countEntries) :
        RfxBaseData(data, countEntries * sizeof(RIL_PHB_ENTRY*)) {
    m_length = countEntries * sizeof(RIL_PHB_ENTRY*);
    copyPhbEntries(data, countEntries);
}

void RfxPhbEntriesExtData::copyPhbEntries(RIL_PHB_ENTRY **data, int countEntries) {
    if (data != NULL) {
        RIL_PHB_ENTRY **pInput = (RIL_PHB_ENTRY**)data;
        RIL_PHB_ENTRY **pData = (RIL_PHB_ENTRY**)calloc(1, sizeof(RIL_PHB_ENTRY*)*countEntries);
        RFX_ASSERT(pData != NULL);
        for (int i = 0; i < countEntries; i++) {
            pData[i] = (RIL_PHB_ENTRY*)calloc(1, sizeof(RIL_PHB_ENTRY));
            RFX_ASSERT(pData[i] != NULL);
            pData[i]->index = pInput[i]->index;
            if (pInput[i]->number != NULL) {
                pData[i]->number = strdup(pInput[i]->number);
            }
            pData[i]->type = pInput[i]->type;
            if (pInput[i]->text != NULL) {
                pData[i]->text = strdup(pInput[i]->text);
            }
            pData[i]->hidden = pInput[i]->hidden;
            if (pInput[i]->group != NULL) {
                pData[i]->group = strdup(pInput[i]->group);
            }
            if (pInput[i]->adnumber != NULL) {
                pData[i]->adnumber = strdup(pInput[i]->adnumber);
            }
            pData[i]->adtype = pInput[i]->adtype;
            if (pInput[i]->secondtext != NULL) {
                pData[i]->secondtext = strdup(pInput[i]->secondtext);
            }
            if (pInput[i]->email != NULL) {
                pData[i]->email = strdup(pInput[i]->email);
            }
        }

        m_data = (void*)pData;
    }
}

RfxPhbEntriesExtData::~RfxPhbEntriesExtData() {
    // free
    if (m_data != NULL) {
        RIL_PHB_ENTRY **pTmp = (RIL_PHB_ENTRY **) m_data;
        int countEntries = m_length/sizeof(RIL_PHB_ENTRY*);
        for (int i = 0; i < countEntries; i++) {
            if (pTmp[i] != NULL) {
                if (pTmp[i]->number != NULL) {
                    free(pTmp[i]->number);
                }
                if (pTmp[i]->text != NULL) {
                    free(pTmp[i]->text);
                }
                if (pTmp[i]->group != NULL) {
                    free(pTmp[i]->group);
                }
                if (pTmp[i]->adnumber != NULL) {
                    free(pTmp[i]->adnumber);
                }
                if (pTmp[i]->secondtext != NULL) {
                    free(pTmp[i]->secondtext);
                }
                if (pTmp[i]->email != NULL) {
                    free(pTmp[i]->email);
                }
                free(pTmp[i]);
            }
        }
        free(m_data);
    }
}
