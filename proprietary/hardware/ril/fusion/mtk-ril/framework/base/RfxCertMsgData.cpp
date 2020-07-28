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

#include "RfxCertMsgData.h"
#include <telephony/mtk_ril.h>
#include <mtk_log.h>

RFX_IMPLEMENT_DATA_CLASS(RfxCertMsgData);

#define RFX_LOG_TAG "RfxCertMsgData"

RfxCertMsgData::RfxCertMsgData(void *data, int length) : RfxBaseData(data, length)  {
    if (data != NULL) {
        RIL_CertMsg* pOriginal = (RIL_CertMsg *) data;
        RIL_CertMsg* pData = (RIL_CertMsg *) calloc(1, sizeof(RIL_CertMsg));
        if (pData == NULL) {
            RFX_LOG_E(RFX_LOG_TAG, "OOM");
            return;
        }

        pData->uid = pOriginal->uid;
        pData->certLength = pOriginal->certLength;
        if (pOriginal->cert != NULL) {
            pData->cert = (char *)calloc(1, pData->certLength+1);
            if (pData->cert == NULL) {
                mtkLogD(RFX_LOG_TAG, "OOM");
                return;
            }
            memcpy(pData->cert, pOriginal->cert, pData->certLength);
            pData->cert[pData->certLength] = '\0';
        }
        pData->msgLength = pOriginal->msgLength;
        if (pOriginal->msg != NULL) {
            pData->msg = (char *)calloc(1, pData->msgLength+1);
            if (pData->msg == NULL) {
                mtkLogD(RFX_LOG_TAG, "OOM");
                return;
            }
            memcpy(pData->msg, pOriginal->msg, pData->msgLength);
            pData->msg[pData->msgLength] = '\0';
        }

        m_data = (void *) pData;
        m_length = length;
    }
}

RfxCertMsgData::~RfxCertMsgData() {
    // free memory
    if (m_data != NULL) {
        RIL_CertMsg* pData = (RIL_CertMsg *) m_data;
        if (pData->cert != NULL) {
            free(pData->cert);
        }
        if (pData->msg != NULL) {
            free(pData->msg);
        }
        free(pData);
    }
}
