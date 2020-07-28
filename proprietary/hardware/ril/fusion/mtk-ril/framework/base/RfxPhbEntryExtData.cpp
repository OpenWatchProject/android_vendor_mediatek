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

#include "RfxPhbEntryExtData.h"
#include "RfxLog.h"
#include <string.h>

RFX_IMPLEMENT_DATA_CLASS(RfxPhbEntryExtData);

RfxPhbEntryExtData::RfxPhbEntryExtData(void *data, int length) : RfxBaseData(data, length)  {
    if (data != NULL) {
        RIL_PHB_ENTRY *pInput = (RIL_PHB_ENTRY*)data;
        RIL_PHB_ENTRY *pData = (RIL_PHB_ENTRY*)calloc(1, sizeof(RIL_PHB_ENTRY));
        RFX_ASSERT(pData != NULL);
        pData->index = pInput->index;
        if (pInput->number != NULL) {
            pData->number = strdup(pInput->number);
        }
        pData->type = pInput->type;
        if (pInput->text != NULL) {
            pData->text = strdup(pInput->text);
        }
        pData->hidden = pInput->hidden;
        if (pInput->group != NULL) {
            pData->group = strdup(pInput->group);
        }
        if (pInput->adnumber != NULL) {
            pData->adnumber = strdup(pInput->adnumber);
        }
        pData->adtype = pInput->adtype;
        if (pInput->secondtext != NULL) {
            pData->secondtext = strdup(pInput->secondtext);
        }
        if (pInput->email != NULL) {
            pData->email = strdup(pInput->email);
        }

        m_data = (void*)pData;
        m_length = length;
    }
}

RfxPhbEntryExtData::~RfxPhbEntryExtData() {
    // free
    if (m_data != NULL) {
        RIL_PHB_ENTRY *pTmp = (RIL_PHB_ENTRY *) m_data;
        if (pTmp != NULL) {
            if (pTmp->number != NULL) {
                free(pTmp->number);
                pTmp->number = NULL;
            }
            if (pTmp->text != NULL) {
                free(pTmp->text);
                pTmp->text = NULL;
            }
            if (pTmp->group != NULL) {
                free(pTmp->group);
                pTmp->group = NULL;
            }
            if (pTmp->adnumber != NULL) {
                free(pTmp->adnumber);
                pTmp->adnumber = NULL;
            }
            if (pTmp->secondtext != NULL) {
                free(pTmp->secondtext);
                pTmp->secondtext = NULL;
            }
            if (pTmp->email != NULL) {
                free(pTmp->email);
                pTmp->email = NULL;
            }
        }
        free(m_data);
        m_data = NULL;
    }
}
