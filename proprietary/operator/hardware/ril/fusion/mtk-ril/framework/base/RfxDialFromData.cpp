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

#include "RfxDialFromData.h"

RFX_IMPLEMENT_DATA_CLASS(RfxDialFromData);

RfxDialFromData::RfxDialFromData(void *data, int length) : RfxBaseData(data, length)  {
    m_length = length;
    RIL_DialFrom *dupData;
    RIL_DialFrom *srcData = (RIL_DialFrom*)data;
    int strLength;

    dupData = (RIL_DialFrom *)calloc(1, sizeof(RIL_DialFrom));
    RFX_ASSERT(dupData != NULL);
    memset(dupData, 0x00, sizeof(RIL_DialFrom));
    m_data = dupData;

    /* Copy address(dial string) */
    if (srcData->address == NULL) {
        return;
    }
    strLength = strlen(srcData->address) + 1;
    dupData->address = (char *)calloc(strLength, sizeof(char));
    RFX_ASSERT(dupData->address != NULL);
    memset(dupData->address, 0, strLength);
    memcpy(dupData->address, srcData->address, strLength);

    /* Copy fromAddress(dial from string) */
    if (srcData->fromAddress == NULL) {
        return;
    }
    strLength = strlen(srcData->fromAddress) + 1;
    dupData->fromAddress = (char *)calloc(strLength, sizeof(char));
    RFX_ASSERT(dupData->fromAddress != NULL);
    memset(dupData->fromAddress, 0, strLength);
    memcpy(dupData->fromAddress, srcData->fromAddress, strLength);

    /* CLIR setting */
    dupData->clir = srcData->clir;

    /* is video call */
    dupData->isVideoCall = srcData->isVideoCall;
}

RfxDialFromData::~RfxDialFromData() {
    // free
    RIL_DialFrom *data = (RIL_DialFrom*)m_data;
    if (data != NULL) {
        if (data->address != NULL) {
            free(data->address);
        }
        if (data->fromAddress != NULL) {
            free(data->fromAddress);
        }
        free(m_data);
    }
}
