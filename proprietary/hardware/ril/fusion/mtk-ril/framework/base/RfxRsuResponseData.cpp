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

#include "RfxRsuResponseData.h"

RFX_IMPLEMENT_DATA_CLASS(RfxRsuResponseData);

RfxRsuResponseData::RfxRsuResponseData(void *data, int length) : RfxBaseData(data, length)  {
    m_length = length;
    if ((m_length / sizeof(RIL_RsuResponse)) == 0 || data == NULL) {
        return;
    }

    RIL_RsuResponse *dupData;
    RIL_RsuResponse *srcData = (RIL_RsuResponse*)data;
    int strLength = -1;

    dupData = (RIL_RsuResponse*)calloc(1, sizeof(RIL_RsuResponse));
    RFX_ASSERT(dupData != NULL);
    memcpy(dupData, srcData, sizeof(RIL_RsuResponse));

    if (srcData->data != NULL) {
        strLength = strlen(srcData->data) + 1;
        dupData->data = (char *)calloc(strLength, sizeof(char));
        RFX_ASSERT(dupData->data != NULL);
        memset(dupData->data, 0, strLength);
        memcpy(dupData->data, srcData->data, strLength);
    }
    if (srcData->reserveString1 != NULL) {
        strLength = strlen(srcData->reserveString1) + 1;
        dupData->reserveString1 = (char *)calloc(strLength, sizeof(char));
        RFX_ASSERT(dupData->reserveString1 != NULL);
        memset(dupData->reserveString1, 0, strLength);
        memcpy(dupData->reserveString1, srcData->reserveString1, strLength);
    }

    m_data = dupData;
}

RfxRsuResponseData::~RfxRsuResponseData() {
    RIL_RsuResponse *data = (RIL_RsuResponse*)m_data;
    if (data != NULL) {
        if (data->data != NULL) {
            free(data->data);
        }
        if (data->reserveString1 != NULL) {
            free(data->reserveString1);
        }
        free(m_data);
    }
}
