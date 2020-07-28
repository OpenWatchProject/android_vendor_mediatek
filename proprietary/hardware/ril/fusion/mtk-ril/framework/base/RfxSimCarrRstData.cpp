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

#include "RfxSimCarrRstData.h"
#include <telephony/mtk_ril.h>
#include "RfxLog.h"

#define RFX_LOG_TAG "RfxSimCarrRstData"

RFX_IMPLEMENT_DATA_CLASS(RfxSimCarrRstData);

RfxSimCarrRstData::RfxSimCarrRstData(void *_data, int _length) : RfxBaseData(_data, _length) {
    if (_data != NULL) {
        RIL_CarrierRestrictionsWithPriority *pCarrRst
                = (RIL_CarrierRestrictionsWithPriority*)_data;
        RIL_CarrierRestrictionsWithPriority *pData = (RIL_CarrierRestrictionsWithPriority *)calloc(
                1, sizeof(RIL_CarrierRestrictionsWithPriority));
        RFX_ASSERT(pData != NULL);

        RIL_Carrier *allowedCarriers = NULL;
        RIL_Carrier *excludedCarriers = NULL;

        pData->len_allowed_carriers = pCarrRst->len_allowed_carriers;
        allowedCarriers = (RIL_Carrier *)calloc(pData->len_allowed_carriers, sizeof(RIL_Carrier));
        RFX_ASSERT(allowedCarriers != NULL);
        pData->allowed_carriers = allowedCarriers;

        pData->len_excluded_carriers = pCarrRst->len_excluded_carriers;
        excludedCarriers = (RIL_Carrier *)calloc(pData->len_excluded_carriers, sizeof(RIL_Carrier));
        RFX_ASSERT(excludedCarriers != NULL);
        pData->excluded_carriers = excludedCarriers;

        for (int i = 0; i < pData->len_allowed_carriers; i++) {
            RIL_Carrier *item = pCarrRst->allowed_carriers + i;
            if (item->mcc != NULL) {
                asprintf(&allowedCarriers[i].mcc, "%s", item->mcc);
            }
            if (item->mnc != NULL) {
                asprintf(&allowedCarriers[i].mnc, "%s", item->mnc);
            }
            allowedCarriers[i].match_type = item->match_type;
            if (item->match_data != NULL) {
                asprintf(&allowedCarriers[i].match_data, "%s", item->match_data);
            }
            RFX_LOG_I(RFX_LOG_TAG, "allowedCarriers %s, %s, %d, %s", allowedCarriers[i].mcc,
                    allowedCarriers[i].mnc, allowedCarriers[i].match_type,
                    allowedCarriers[i].match_data);
        }

        for (int i = 0; i < pData->len_excluded_carriers; i++) {
            RIL_Carrier *item = pCarrRst->excluded_carriers + i;
            if (item->mcc != NULL) {
                asprintf(&excludedCarriers[i].mcc, "%s", item->mcc);
            }
            if (item->mnc != NULL) {
                asprintf(&excludedCarriers[i].mnc, "%s", item->mnc);
            }
            excludedCarriers[i].match_type = item->match_type;
            if (item->match_data != NULL) {
                asprintf(&excludedCarriers[i].match_data, "%s", item->match_data);
            }
            RFX_LOG_I(RFX_LOG_TAG, "excludedCarriers %s, %s, %d, %s", excludedCarriers[i].mcc,
                    excludedCarriers[i].mnc, excludedCarriers[i].match_type,
                    excludedCarriers[i].match_data);
        }

        pData->allowedCarriersPrioritized = pCarrRst->allowedCarriersPrioritized;
        pData->simLockMultiSimPolicy = pCarrRst->simLockMultiSimPolicy;
        RFX_LOG_E(RFX_LOG_TAG, "pData Allowed %d, Excluded %d, Prioritized %d, MultiSIMPolicy %d",
                pData->len_allowed_carriers, pData->len_excluded_carriers,
                pData->allowedCarriersPrioritized, pData->simLockMultiSimPolicy);
        m_data = (void*)pData;
        m_length = _length;
    }
}

RfxSimCarrRstData::~RfxSimCarrRstData() {
    // free memory
    if (m_data != NULL) {
        RIL_CarrierRestrictionsWithPriority *pData
                = (RIL_CarrierRestrictionsWithPriority*)m_data;
        if (pData->allowed_carriers != NULL) {
            for (int i = 0; i < pData->len_allowed_carriers; i++) {
                RIL_Carrier *item = pData->allowed_carriers + i;
                if (item->mcc != NULL) {
                    free((void*)item->mcc);
                    item->mcc = NULL;
                }
                if (item->mnc != NULL) {
                    free((void*)item->mnc);
                    item->mnc = NULL;
                }
                if (item->match_data != NULL) {
                    free((void*)item->match_data);
                    item->match_data = NULL;
                }
            }
            free(pData->allowed_carriers);
            pData->allowed_carriers = NULL;
        }
        if (pData->excluded_carriers != NULL) {
            for (int i = 0; i < pData->len_excluded_carriers; i++) {
                RIL_Carrier *item = pData->excluded_carriers + i;
                if (item->mcc != NULL) {
                    free((void*)item->mcc);
                    item->mcc = NULL;
                }
                if (item->mnc != NULL) {
                    free((void*)item->mnc);
                    item->mnc = NULL;
                }
                if (item->match_data != NULL) {
                    free((void*)item->match_data);
                    item->match_data = NULL;
                }
            }
            free(pData->excluded_carriers);
            pData->excluded_carriers = NULL;
        }
        free(m_data);
        m_data = NULL;
    }
}
