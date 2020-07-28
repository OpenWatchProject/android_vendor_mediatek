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

#include "nw/RmcNetworkHandler.h"
#include "RfxDataRegStateData.h"

RFX_IMPLEMENT_DATA_CLASS(RfxDataRegStateData);

RfxDataRegStateData::RfxDataRegStateData(void *data, int length) : RfxBaseData(data, length)  {
    if (data != NULL) {
        RIL_DataRegistrationStateResponse *tmp = (RIL_DataRegistrationStateResponse *) data;
        RIL_DataRegistrationStateResponse *pData =
                (RIL_DataRegistrationStateResponse *) calloc(1, length);
        RFX_ASSERT(pData != NULL);
        memcpy(pData, tmp, length);

        // alloc memory for string type
        switch (pData->cellIdentity.cellInfoType) {
            case RIL_CELL_INFO_TYPE_GSM: {
                asprintf(&(pData->cellIdentity.cellIdentityGsm.operName.long_name), "%s",
                    tmp->cellIdentity.cellIdentityGsm.operName.long_name);
                asprintf(&(pData->cellIdentity.cellIdentityGsm.operName.short_name), "%s",
                    tmp->cellIdentity.cellIdentityGsm.operName.short_name);
                break;
            }

            case RIL_CELL_INFO_TYPE_WCDMA: {
                asprintf(&(pData->cellIdentity.cellIdentityWcdma.operName.long_name), "%s",
                    tmp->cellIdentity.cellIdentityWcdma.operName.long_name);
                asprintf(&(pData->cellIdentity.cellIdentityWcdma.operName.short_name), "%s",
                    tmp->cellIdentity.cellIdentityWcdma.operName.short_name);
                break;
            }

            case RIL_CELL_INFO_TYPE_CDMA: {
                asprintf(&(pData->cellIdentity.cellIdentityCdma.operName.long_name), "%s",
                    tmp->cellIdentity.cellIdentityCdma.operName.long_name);
                asprintf(&(pData->cellIdentity.cellIdentityCdma.operName.short_name), "%s",
                    tmp->cellIdentity.cellIdentityCdma.operName.short_name);
                break;
            }

            case RIL_CELL_INFO_TYPE_LTE: {
                asprintf(&(pData->cellIdentity.cellIdentityLte.operName.long_name), "%s",
                    tmp->cellIdentity.cellIdentityLte.operName.long_name);
                asprintf(&(pData->cellIdentity.cellIdentityLte.operName.short_name), "%s",
                    tmp->cellIdentity.cellIdentityLte.operName.short_name);
                break;
            }

            case RIL_CELL_INFO_TYPE_TD_SCDMA: {
                asprintf(&(pData->cellIdentity.cellIdentityTdscdma.operName.long_name), "%s",
                    tmp->cellIdentity.cellIdentityTdscdma.operName.long_name);
                asprintf(&(pData->cellIdentity.cellIdentityTdscdma.operName.short_name), "%s",
                    tmp->cellIdentity.cellIdentityTdscdma.operName.short_name);
                break;
            }
            case RIL_CELL_INFO_TYPE_NR: {
                asprintf(&(pData->cellIdentity.cellIdentityNr.operName.long_name), "%s",
                    tmp->cellIdentity.cellIdentityNr.operName.long_name);
                asprintf(&(pData->cellIdentity.cellIdentityNr.operName.short_name), "%s",
                    tmp->cellIdentity.cellIdentityNr.operName.short_name);
                break;
            }
            case RIL_CELL_INFO_TYPE_NONE: {
                pData->cellIdentity.cellIdentityTdscdma.operName.long_name = NULL;
                pData->cellIdentity.cellIdentityTdscdma.operName.short_name = NULL;
                break;
            }
        }

        m_data = pData;
        m_length = length;
    }
}

RfxDataRegStateData::~RfxDataRegStateData() {
    if (m_data) {
        RIL_DataRegistrationStateResponse *tmp = (RIL_DataRegistrationStateResponse *) m_data;
        // free string if it is not null.
        switch (tmp->cellIdentity.cellInfoType) {
            case RIL_CELL_INFO_TYPE_GSM: {
                if (tmp->cellIdentity.cellIdentityGsm.operName.long_name)
                    free(tmp->cellIdentity.cellIdentityGsm.operName.long_name);
                if (tmp->cellIdentity.cellIdentityGsm.operName.short_name)
                    free(tmp->cellIdentity.cellIdentityGsm.operName.short_name);
                break;
            }

            case RIL_CELL_INFO_TYPE_WCDMA: {
                if (tmp->cellIdentity.cellIdentityWcdma.operName.long_name)
                    free(tmp->cellIdentity.cellIdentityWcdma.operName.long_name);
                if (tmp->cellIdentity.cellIdentityWcdma.operName.short_name)
                    free(tmp->cellIdentity.cellIdentityWcdma.operName.short_name);
                break;
            }

            case RIL_CELL_INFO_TYPE_CDMA: {
                if (tmp->cellIdentity.cellIdentityCdma.operName.long_name)
                    free(tmp->cellIdentity.cellIdentityCdma.operName.long_name);
                if (tmp->cellIdentity.cellIdentityCdma.operName.short_name)
                    free(tmp->cellIdentity.cellIdentityCdma.operName.short_name);
                break;
            }

            case RIL_CELL_INFO_TYPE_LTE: {
                if (tmp->cellIdentity.cellIdentityLte.operName.long_name)
                    free(tmp->cellIdentity.cellIdentityLte.operName.long_name);
                if (tmp->cellIdentity.cellIdentityLte.operName.short_name)
                    free(tmp->cellIdentity.cellIdentityLte.operName.short_name);
                break;
            }

            case RIL_CELL_INFO_TYPE_TD_SCDMA: {
                if (tmp->cellIdentity.cellIdentityTdscdma.operName.long_name)
                    free(tmp->cellIdentity.cellIdentityTdscdma.operName.long_name);
                if (tmp->cellIdentity.cellIdentityTdscdma.operName.short_name)
                    free(tmp->cellIdentity.cellIdentityTdscdma.operName.short_name);
                break;
            }
            case RIL_CELL_INFO_TYPE_NR: {
                if (tmp->cellIdentity.cellIdentityNr.operName.long_name)
                    free(tmp->cellIdentity.cellIdentityNr.operName.long_name);
                if (tmp->cellIdentity.cellIdentityNr.operName.short_name)
                    free(tmp->cellIdentity.cellIdentityNr.operName.short_name);
                break;
            }
            case RIL_CELL_INFO_TYPE_NONE:
                break;
        }
        free(m_data);
    }
    m_data = NULL;
    m_length = 0;
}
