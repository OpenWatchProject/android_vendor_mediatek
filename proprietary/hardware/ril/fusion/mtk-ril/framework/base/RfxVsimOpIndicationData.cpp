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

// External SIM - START
#include "RfxVsimOpIndicationData.h"
#include <telephony/mtk_ril.h>
#include "RfxRilUtils.h"

RFX_IMPLEMENT_DATA_CLASS(RfxVsimOpIndicationData);

RfxVsimOpIndicationData::RfxVsimOpIndicationData(void *data, int length) :
    RfxBaseData(data, length) {
    if (data != NULL) {
        RIL_VsimOperationEvent* pVsim = (RIL_VsimOperationEvent*)data;
        RIL_VsimOperationEvent* pData = (RIL_VsimOperationEvent*)calloc(1,
                sizeof(RIL_VsimOperationEvent));
        if (pData == NULL) {
            RfxRilUtils::printLog(
                    ERROR,
                    String8::format("[RfxVsimOpIndicationData]"),
                    String8::format("fail to allocate pData!"),
                    0);
            return;
        }
        pData->data_length = pVsim->data_length;
        if (pVsim->data != NULL) {
            pData->data = (char*)calloc(1, pVsim->data_length + 1);

            if (pData->data != NULL) {
                memset(pData->data, 0, pVsim->data_length + 1);
                memcpy(pData->data, pVsim->data, pVsim->data_length);
            } else {
                RfxRilUtils::printLog(ERROR,
                        String8::format("[RfxVsimOpIndicationData]"),
                        String8::format("fail to allocate memory, size = %d", pData->data_length),
                        0);
                pData->data_length = 0;
            }
        }
        pData->eventId = pVsim->eventId;
        pData->result = pVsim->result;
        pData->transaction_id = pVsim->transaction_id;

        m_data = pData;
        m_length = length;
    }
}

RfxVsimOpIndicationData::~RfxVsimOpIndicationData() {
    if (m_data != NULL) {
        RIL_VsimOperationEvent* pData = (RIL_VsimOperationEvent*)m_data;
        if (pData->data != NULL) {
            free(pData->data);
        }
        free(m_data);
    }
}
// External SIM - END
