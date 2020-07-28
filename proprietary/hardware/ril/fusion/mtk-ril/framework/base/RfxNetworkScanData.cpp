/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
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

#include "RfxNetworkScanData.h"

RFX_IMPLEMENT_DATA_CLASS(RfxNetworkScanData);

RfxNetworkScanData::RfxNetworkScanData(void *data, int length) : RfxBaseData(data, length)  {
    RIL_NetworkScanRequest* pTmp = (RIL_NetworkScanRequest*) data;
    RIL_NetworkScanRequest *pData = (RIL_NetworkScanRequest *) calloc(1, sizeof(RIL_NetworkScanRequest));

    if (pData != NULL) {
        pData->type = pTmp->type;
        pData->interval = pTmp->interval;
        pData->specifiers_length = pTmp->specifiers_length;

        for (int i = 0 ; i < (int)pData->specifiers_length ; i++) {
            pData->specifiers[i].radio_access_network = pTmp->specifiers[i].radio_access_network;
            pData->specifiers[i].bands_length = pTmp->specifiers[i].bands_length;

            if (pData->specifiers[i].radio_access_network == GERAN) {
                memcpy(&pData->specifiers[i].bands.geran_bands, &pTmp->
                        specifiers[i].bands.geran_bands, MAX_BANDS*sizeof(RIL_GeranBands));
            } else if (pData->specifiers[i].radio_access_network == UTRAN) {
                memcpy(pData->specifiers[i].bands.utran_bands, pTmp->
                        specifiers[i].bands.utran_bands, MAX_BANDS*sizeof(RIL_UtranBands));
            } else {
                memcpy(pData->specifiers[i].bands.eutran_bands, pTmp->
                        specifiers[i].bands.eutran_bands, MAX_BANDS*sizeof(RIL_EutranBands));
            }

            pData->specifiers[i].channels_length = pTmp->specifiers[i].channels_length;
            memcpy(pData->specifiers[i].channels, pTmp->specifiers[i].channels, MAX_CHANNELS*sizeof(uint32_t));
        }
        pData->maxSearchTime = pTmp->maxSearchTime;
        pData->incrementalResults = pTmp->incrementalResults;
        pData->incrementalResultsPeriodicity = pTmp->incrementalResultsPeriodicity;
        pData->mccMncs_length = pTmp->mccMncs_length;
        pData->mccMncs = (char**) calloc(pData->mccMncs_length, sizeof(char*));
        RFX_ASSERT(pData->mccMncs != NULL);
        memset(pData->mccMncs, 0, pData->mccMncs_length);
        for (size_t i = 0; i < pData->mccMncs_length; i++) {
            asprintf(&(pData->mccMncs[i]), "%s", pTmp->mccMncs[i]);
        }

        m_data = pData;
        m_length = length;
    }
}

RfxNetworkScanData::~RfxNetworkScanData() {
    RIL_NetworkScanRequest *pData = (RIL_NetworkScanRequest *) m_data;
    if (pData) {
        if (pData->mccMncs_length > 0 && pData->mccMncs != NULL) {
            for (size_t i = 0; i < pData->mccMncs_length; i++) {
                if (pData->mccMncs[i] != NULL) free(pData->mccMncs[i]);
                pData->mccMncs[i] = NULL;
            }
            pData->mccMncs = NULL;
        }
    }
    free(m_data);
    m_data = NULL;
}

