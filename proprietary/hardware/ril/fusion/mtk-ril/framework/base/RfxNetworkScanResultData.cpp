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

#include "RfxNetworkScanResultData.h"

RFX_IMPLEMENT_DATA_CLASS(RfxNetworkScanResultData);

RfxNetworkScanResultData::RfxNetworkScanResultData(void *data, int length) : RfxBaseData(data, length)  {
    if (data != NULL) {
        RIL_NetworkScanResult *pResult = (RIL_NetworkScanResult *) data;
        RIL_NetworkScanResult *pData = (RIL_NetworkScanResult *) calloc(1, sizeof(RIL_NetworkScanResult));
        if (pData == NULL) goto error;
        pData->status = pResult->status;
        pData->network_infos_length = pResult->network_infos_length;
        if (pData->network_infos_length > 0) {
            int countCell = pResult->network_infos_length;
            pData->network_infos = (RIL_CellInfo_v12 *) calloc(countCell, sizeof(RIL_CellInfo_v12));
            if (pData->network_infos == NULL) goto error;
            memcpy(pData->network_infos, pResult->network_infos,
                    pData->network_infos_length * sizeof(RIL_CellInfo_v12));
            for (int i = 0; i < countCell; i++) {
                switch(pData->network_infos[i].cellInfoType) {
                    case RIL_CELL_INFO_TYPE_GSM:
                        asprintf(&(pData->network_infos[i].CellInfo.gsm.cellIdentityGsm.operName.long_name),
                                "%s", pResult->network_infos[i].CellInfo.gsm.cellIdentityGsm.operName.long_name);
                        asprintf(&(pData->network_infos[i].CellInfo.gsm.cellIdentityGsm.operName.short_name),
                                "%s", pResult->network_infos[i].CellInfo.gsm.cellIdentityGsm.operName.short_name);
                        break;
                    case RIL_CELL_INFO_TYPE_WCDMA:
                        asprintf(&(pData->network_infos[i].CellInfo.wcdma.cellIdentityWcdma.operName.long_name),
                                "%s", pResult->network_infos[i].CellInfo.wcdma.cellIdentityWcdma.operName.long_name);
                        asprintf(&(pData->network_infos[i].CellInfo.wcdma.cellIdentityWcdma.operName.short_name),
                                "%s", pResult->network_infos[i].CellInfo.wcdma.cellIdentityWcdma.operName.short_name);
                        break;
                    case RIL_CELL_INFO_TYPE_LTE:
                        asprintf(&(pData->network_infos[i].CellInfo.lte.cellIdentityLte.operName.long_name),
                                "%s", pResult->network_infos[i].CellInfo.lte.cellIdentityLte.operName.long_name);
                        asprintf(&(pData->network_infos[i].CellInfo.lte.cellIdentityLte.operName.short_name),
                                "%s", pResult->network_infos[i].CellInfo.lte.cellIdentityLte.operName.short_name);
                        break;
                    case RIL_CELL_INFO_TYPE_NR:
                        // TODO
                    case RIL_CELL_INFO_TYPE_TD_SCDMA:
                    case RIL_CELL_INFO_TYPE_CDMA:
                    case RIL_CELL_INFO_TYPE_NONE:
                        break;
                }
            }
        }
        m_data = pData;
        m_length = length;
        return;
error:
        if (pData) free(pData);
        m_data = NULL;
        m_length = 0;
    }
}

RfxNetworkScanResultData::~RfxNetworkScanResultData() {
    RIL_NetworkScanResult * pData = (RIL_NetworkScanResult *) m_data;
    if (pData) {
        if (pData->network_infos_length > 0 && pData->network_infos != NULL) {
            int countCell = pData->network_infos_length;
            for (int i = 0; i < countCell; i++) {
                switch(pData->network_infos[i].cellInfoType) {
                    case RIL_CELL_INFO_TYPE_GSM:
                        if (pData->network_infos[i].CellInfo.gsm.cellIdentityGsm.operName.long_name)
                            free(pData->network_infos[i].CellInfo.gsm.cellIdentityGsm.operName.long_name);
                        if (pData->network_infos[i].CellInfo.gsm.cellIdentityGsm.operName.short_name)
                            free(pData->network_infos[i].CellInfo.gsm.cellIdentityGsm.operName.short_name);
                        break;
                    case RIL_CELL_INFO_TYPE_WCDMA:
                        if (pData->network_infos[i].CellInfo.wcdma.cellIdentityWcdma.operName.long_name)
                            free(pData->network_infos[i].CellInfo.wcdma.cellIdentityWcdma.operName.long_name);
                        if(pData->network_infos[i].CellInfo.wcdma.cellIdentityWcdma.operName.short_name)
                            free(pData->network_infos[i].CellInfo.wcdma.cellIdentityWcdma.operName.short_name);
                        break;
                    case RIL_CELL_INFO_TYPE_LTE:
                        if (pData->network_infos[i].CellInfo.lte.cellIdentityLte.operName.long_name)
                            free(pData->network_infos[i].CellInfo.lte.cellIdentityLte.operName.long_name);
                        if (pData->network_infos[i].CellInfo.lte.cellIdentityLte.operName.short_name)
                            free(pData->network_infos[i].CellInfo.lte.cellIdentityLte.operName.short_name);
                        break;
                    case RIL_CELL_INFO_TYPE_NR:
                        // TODO
                    case RIL_CELL_INFO_TYPE_TD_SCDMA:
                    case RIL_CELL_INFO_TYPE_CDMA:
                    case RIL_CELL_INFO_TYPE_NONE:
                        break;
                }
            }
            free(pData->network_infos);
        }
        free(pData);
    }
    m_data = NULL;
    m_length = 0;
}

