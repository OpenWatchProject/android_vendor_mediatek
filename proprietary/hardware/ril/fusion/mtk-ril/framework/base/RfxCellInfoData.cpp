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

#include "RfxCellInfoData.h"

RFX_IMPLEMENT_DATA_CLASS(RfxCellInfoData);

RfxCellInfoData::RfxCellInfoData(void *data, int length) : RfxBaseData(data, length)  {
    if (data != NULL) {
        m_length = length;
        int countCell = m_length / sizeof(RIL_CellInfo_v12);

        RIL_CellInfo_v12 *pCell = (RIL_CellInfo_v12 *) calloc(countCell, sizeof(RIL_CellInfo_v12));
        if (pCell == NULL) goto error;
        memcpy(pCell, (RIL_CellInfo_v12 *)data, m_length);
        // alloc memory for string
        RIL_CellInfo_v12 *tmp = (RIL_CellInfo_v12 *)data;
        for (int i = 0; i < countCell; i++) {
            switch (pCell[i].cellInfoType) {
                case RIL_CELL_INFO_TYPE_GSM: {
                    asprintf(&(pCell[i].CellInfo.gsm.cellIdentityGsm.operName.long_name), "%s",
                        tmp[i].CellInfo.gsm.cellIdentityGsm.operName.long_name);
                    asprintf(&(pCell[i].CellInfo.gsm.cellIdentityGsm.operName.short_name), "%s",
                        tmp[i].CellInfo.gsm.cellIdentityGsm.operName.short_name);
                    break;
                }

                case RIL_CELL_INFO_TYPE_WCDMA: {
                    asprintf(&(pCell[i].CellInfo.wcdma.cellIdentityWcdma.operName.long_name), "%s",
                        tmp[i].CellInfo.wcdma.cellIdentityWcdma.operName.long_name);
                    asprintf(&(pCell[i].CellInfo.wcdma.cellIdentityWcdma.operName.short_name), "%s",
                        tmp[i].CellInfo.wcdma.cellIdentityWcdma.operName.short_name);
                    break;
                }

                case RIL_CELL_INFO_TYPE_CDMA: {
                    // It's NULL now.
                    break;
                }

                case RIL_CELL_INFO_TYPE_LTE: {
                    asprintf(&(pCell[i].CellInfo.lte.cellIdentityLte.operName.long_name), "%s",
                        tmp[i].CellInfo.lte.cellIdentityLte.operName.long_name);
                    asprintf(&(pCell[i].CellInfo.lte.cellIdentityLte.operName.short_name), "%s",
                        tmp[i].CellInfo.lte.cellIdentityLte.operName.short_name);
                    break;
                }

                case RIL_CELL_INFO_TYPE_TD_SCDMA: {
                    asprintf(&(pCell[i].CellInfo.tdscdma.cellIdentityTdscdma.operName.long_name), "%s",
                        tmp[i].CellInfo.tdscdma.cellIdentityTdscdma.operName.long_name);
                    asprintf(&(pCell[i].CellInfo.tdscdma.cellIdentityTdscdma.operName.short_name), "%s",
                        tmp[i].CellInfo.tdscdma.cellIdentityTdscdma.operName.short_name);
                    break;
                }
                case RIL_CELL_INFO_TYPE_NR: {
                    asprintf(&(pCell[i].CellInfo.nr.cellidentity.operName.long_name), "%s",
                        tmp[i].CellInfo.nr.cellidentity.operName.long_name);
                    asprintf(&(pCell[i].CellInfo.nr.cellidentity.operName.short_name), "%s",
                        tmp[i].CellInfo.nr.cellidentity.operName.short_name);
                    break;
                }
                case RIL_CELL_INFO_TYPE_NONE:
                    break;
            }
        }

        m_data = pCell;
    }
    return;
error:
    m_data = NULL;
}

RfxCellInfoData::~RfxCellInfoData() {
    if (m_data) {
        int countCell = m_length / sizeof(RIL_CellInfo_v12);
        RIL_CellInfo_v12 *pCell = (RIL_CellInfo_v12 *) m_data;
        for (int i = 0; i < countCell; i++) {
            switch (pCell[i].cellInfoType) {
                case RIL_CELL_INFO_TYPE_GSM: {
                    if (pCell[i].CellInfo.gsm.cellIdentityGsm.operName.long_name)
                        free(pCell[i].CellInfo.gsm.cellIdentityGsm.operName.long_name);
                    if (pCell[i].CellInfo.gsm.cellIdentityGsm.operName.short_name)
                        free(pCell[i].CellInfo.gsm.cellIdentityGsm.operName.short_name);
                    break;
                }

                case RIL_CELL_INFO_TYPE_WCDMA: {
                    if (pCell[i].CellInfo.wcdma.cellIdentityWcdma.operName.long_name)
                        free(pCell[i].CellInfo.wcdma.cellIdentityWcdma.operName.long_name);
                    if (pCell[i].CellInfo.wcdma.cellIdentityWcdma.operName.short_name)
                        free(pCell[i].CellInfo.wcdma.cellIdentityWcdma.operName.short_name);
                    break;
                }

                case RIL_CELL_INFO_TYPE_CDMA: {
                    // It's NULL now.
                    break;
                }

                case RIL_CELL_INFO_TYPE_LTE: {
                    if (pCell[i].CellInfo.lte.cellIdentityLte.operName.long_name)
                        free(pCell[i].CellInfo.lte.cellIdentityLte.operName.long_name);
                    if (pCell[i].CellInfo.lte.cellIdentityLte.operName.short_name)
                        free(pCell[i].CellInfo.lte.cellIdentityLte.operName.short_name);
                    break;
                }

                case RIL_CELL_INFO_TYPE_TD_SCDMA: {
                    if (pCell[i].CellInfo.tdscdma.cellIdentityTdscdma.operName.long_name)
                        free(pCell[i].CellInfo.tdscdma.cellIdentityTdscdma.operName.long_name);
                    if (pCell[i].CellInfo.tdscdma.cellIdentityTdscdma.operName.short_name)
                        free(pCell[i].CellInfo.tdscdma.cellIdentityTdscdma.operName.short_name);
                    break;
                }
                case RIL_CELL_INFO_TYPE_NR:{
                    if (pCell[i].CellInfo.nr.cellidentity.operName.long_name)
                        free(pCell[i].CellInfo.nr.cellidentity.operName.long_name);
                    if (pCell[i].CellInfo.nr.cellidentity.operName.short_name)
                        free(pCell[i].CellInfo.nr.cellidentity.operName.short_name);
                    break;
                }
                case RIL_CELL_INFO_TYPE_NONE:
                    break;
            }
        }

        free(m_data);
    }
}
