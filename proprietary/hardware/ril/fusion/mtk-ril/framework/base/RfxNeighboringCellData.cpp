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

#include "RfxNeighboringCellData.h"

RFX_IMPLEMENT_DATA_CLASS(RfxNeighboringCellData);

RfxNeighboringCellData::RfxNeighboringCellData(void *data, int length) : RfxBaseData(data, length)  {
    if (data != NULL) {
        m_length = length;
        int countStrings = length/sizeof(RIL_NeighboringCell *);
        copyCells((RIL_NeighboringCell **)data, countStrings);
    }
}

RfxNeighboringCellData::RfxNeighboringCellData(RIL_NeighboringCell **data, int countStrings) :
        RfxBaseData(data, countStrings * sizeof(RIL_NeighboringCell *))  {
    if (data != NULL) {
        m_length = countStrings * sizeof(RIL_NeighboringCell *);
        copyCells(data, countStrings);
    }
}

void RfxNeighboringCellData::copyCells(RIL_NeighboringCell **data, int countCell) {
    RIL_NeighboringCell **pCell = (RIL_NeighboringCell **) calloc(countCell, sizeof(RIL_NeighboringCell *));
    if (pCell == NULL) goto error;
    for (int i = 0; i < countCell; i++) {
        pCell[i] = (RIL_NeighboringCell *) calloc(1, sizeof(RIL_NeighboringCell));
        if (pCell[i] == NULL) goto error;
        asprintf(&pCell[i]->cid, "%s", data[i]->cid);
        pCell[i]->rssi = data[i]->rssi;
    }
    m_data = pCell;
    return;
error:
    if (pCell) free(pCell);
    m_data = NULL;
}

RfxNeighboringCellData::~RfxNeighboringCellData() {
    if (m_data) {
        RIL_NeighboringCell **pTmp = (RIL_NeighboringCell **) m_data;
        int countCell = m_length/sizeof(RIL_NeighboringCell *);
        for (int i = 0; i < countCell; i++) {
            if (pTmp[i]) {
                if (pTmp[i]->cid) {
                    free(pTmp[i]->cid);
                }
                free(pTmp[i]);
            }
        }
        free(m_data);
    }
}
