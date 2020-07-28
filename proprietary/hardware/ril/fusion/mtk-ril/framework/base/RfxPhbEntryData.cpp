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

#include "RfxPhbEntryData.h"
#include "RfxLog.h"
#include <string.h>

RFX_IMPLEMENT_DATA_CLASS(RfxPhbEntryData);

RfxPhbEntryData::RfxPhbEntryData(void *data, int length) : RfxBaseData(data, length) {
    if (data != NULL) {
        RIL_PhbEntryStructure *pInput = (RIL_PhbEntryStructure*)data;
        RIL_PhbEntryStructure *pData = (RIL_PhbEntryStructure*)calloc(1, sizeof(RIL_PhbEntryStructure));
        RFX_ASSERT(pData != NULL);
        pData->type = pInput->type;
        pData->index = pInput->index;
        if (pInput->number != NULL) {
            pData->number = strdup(pInput->number);
        }
        pData->ton = pInput->ton;
        if (pInput->alphaId != NULL) {
            pData->alphaId = strdup(pInput->alphaId);
        }

        m_data = (void*)pData;
        m_length = length;
    }
}

RfxPhbEntryData::~RfxPhbEntryData() {
    // free
    if (m_data != NULL) {
        RIL_PhbEntryStructure *pTmp = (RIL_PhbEntryStructure *) m_data;
        if (pTmp->number != NULL) {
            free(pTmp->number);
            pTmp->number = NULL;
        }
        if (pTmp->alphaId != NULL) {
            free(pTmp->alphaId);
            pTmp->alphaId = NULL;
        }
        free(m_data);
        m_data = NULL;
    }
}
