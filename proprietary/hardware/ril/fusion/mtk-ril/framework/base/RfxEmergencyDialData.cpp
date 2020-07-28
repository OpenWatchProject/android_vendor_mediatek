/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
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

#include "RfxEmergencyDialData.h"

RFX_IMPLEMENT_DATA_CLASS(RfxEmergencyDialData);

RfxEmergencyDialData::RfxEmergencyDialData(void *data, int length) : RfxBaseData(data, length)  {
    m_length = length;
    RIL_Emergency_Dial *dupData;
    RIL_Emergency_Dial *srcData = (RIL_Emergency_Dial*)data;
    int strLength;

    dupData = (RIL_Emergency_Dial *)calloc(1, sizeof(RIL_Emergency_Dial));
    RFX_ASSERT(dupData != NULL);
    memset(dupData, 0x00, sizeof(RIL_Emergency_Dial));
    m_data = dupData;

    /* Copy address(dial string) */
    if (srcData->dialData->address == NULL) {
        return;
    }

    dupData->dialData = (RIL_Dial*)calloc(1, sizeof(RIL_Dial));
    RFX_ASSERT(dupData->dialData != NULL);

    strLength = strlen(srcData->dialData->address) + 1;
    dupData->dialData->address = (char *)calloc(strLength, sizeof(char));
    RFX_ASSERT(dupData->dialData->address != NULL);
    memset(dupData->dialData->address, 0, strLength);
    memcpy(dupData->dialData->address, srcData->dialData->address, strLength);

    /* CLIR setting */
    dupData->dialData->clir = srcData->dialData->clir;

    /* uusPresent */
    dupData->dialData->uusInfo = NULL;
    if (srcData->dialData->uusInfo != NULL) {
        dupData->dialData->uusInfo = (RIL_UUS_Info*)calloc(1, sizeof(RIL_UUS_Info));
        RFX_ASSERT(dupData->dialData->uusInfo != NULL);
        dupData->dialData->uusInfo->uusType = (RIL_UUS_Type)srcData->dialData->uusInfo->uusType;
        dupData->dialData->uusInfo->uusDcs = (RIL_UUS_DCS)srcData->dialData->uusInfo->uusDcs;
        dupData->dialData->uusInfo->uusLength = srcData->dialData->uusInfo->uusLength;
        if (dupData->dialData->uusInfo->uusLength == -1) {
            dupData->dialData->uusInfo->uusData = NULL;
        } else {
            dupData->dialData->uusInfo->uusData =
                    (char *)calloc(dupData->dialData->uusInfo->uusLength + 1, sizeof(char));
            RFX_ASSERT(dupData->dialData->uusInfo->uusData != NULL);
            memset(dupData->dialData->uusInfo->uusData, 0,
                    dupData->dialData->uusInfo->uusLength + 1);
            memcpy(dupData->dialData->uusInfo->uusData, srcData->dialData->uusInfo->uusData,
                    dupData->dialData->uusInfo->uusLength + 1);
        }
    }

    /* EmergencyServiceCategory */
    dupData->serviceCategory = srcData->serviceCategory;

    /* EmergencyCallRouting */
    dupData->routing = srcData->routing;

    /* isTesting */
    dupData->isTesting = srcData->isTesting;

 }

RfxEmergencyDialData::~RfxEmergencyDialData() {
    // free
    RIL_Emergency_Dial *data = (RIL_Emergency_Dial*)m_data;
    if (data != NULL) {
        if (data->dialData->address != NULL) {
            free(data->dialData->address);
        }
        if (data->dialData->uusInfo != NULL) {
            if (data->dialData->uusInfo->uusData != NULL) {
                free(data->dialData->uusInfo->uusData);
            }
            free(data->dialData->uusInfo);
        }
        free(m_data);
    }
}
