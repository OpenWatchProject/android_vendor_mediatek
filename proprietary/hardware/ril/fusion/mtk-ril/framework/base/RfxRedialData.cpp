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

#include "RfxLog.h"
#include "RfxRedialData.h"
#include "RfxRilUtils.h"

RFX_IMPLEMENT_DATA_CLASS(RfxRedialData);

RfxRedialData::RfxRedialData(void *data, int length) : RfxBaseData(data, length) {
    RFX_Redial *dupData;
    RFX_Redial *srcData = (RFX_Redial *)data;

    dupData = (RFX_Redial *)calloc(1, sizeof(RFX_Redial));
    RFX_ASSERT(dupData != NULL);
    mDialData = new RfxDialData(srcData->dial_data, sizeof(RIL_Dial));

    dupData->dial_data = (RIL_Dial *)(mDialData->getData());
    dupData->call_id = srcData->call_id;

    m_length = sizeof(RFX_Redial);
    m_data = dupData;

    mCallId = srcData->call_id;
}

RfxRedialData::RfxRedialData(RIL_Dial *data, int callId) : RfxBaseData(NULL, 0) {
    RFX_Redial *dupData;

    dupData = (RFX_Redial *)calloc(1, sizeof(RFX_Redial));
    RFX_ASSERT(dupData != NULL);
    mDialData = new RfxDialData(data, sizeof(RIL_Dial));

    dupData->dial_data = (RIL_Dial *)(mDialData->getData());
    dupData->call_id = callId;

    m_length = sizeof(RFX_Redial);
    m_data = dupData;

    mCallId = callId;
}

RfxRedialData::~RfxRedialData() {
    if (mDialData != NULL) {
        delete(mDialData);
    }

    if (m_data != NULL) {
        free(m_data);
    }
}

void RfxRedialData::dump() {
    RFX_LOG_D("RfxRedialData",
            "Dump mCallId:%d, number:%s",
            mCallId,
            (RfxRilUtils::isUserLoad() ? "[hidden]" : (((RFX_Redial *)m_data)->dial_data)->address));
}
