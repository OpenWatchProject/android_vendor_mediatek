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

#include "RfxSipRegInfoData.h"
#include "telephony/mtk_ril.h"

RFX_IMPLEMENT_DATA_CLASS(RfxSipRegInfoData);

RfxSipRegInfoData::RfxSipRegInfoData(void *data, int length)
    : RfxBaseData(data, length) {
    if (NULL != data) {
        RIL_SipRegInfo* pIn =
                (RIL_SipRegInfo*)data;
        RIL_SipRegInfo *pOut =
                (RIL_SipRegInfo*)calloc(1, sizeof(RIL_SipRegInfo));

        pOut->account_id = pIn->account_id;
        pOut->response_code = pIn->response_code;
        if (pIn->direction != NULL) {
            pOut->direction = strdup(pIn->direction);
        }
        if (pIn->sip_msg_type != NULL) {
            pOut->sip_msg_type = strdup(pIn->sip_msg_type);
        }
        if (pIn->method != NULL) {
            pOut->method = strdup(pIn->method);
        }
        if (pIn->reason_phrase != NULL) {
            pOut->reason_phrase = strdup(pIn->reason_phrase);
        }
        if (pIn->warn_text != NULL) {
            pOut->warn_text = strdup(pIn->warn_text);
        }
        m_data = (void*)pOut;
        m_length = length;
    }
}

RfxSipRegInfoData::~RfxSipRegInfoData() {
    if (m_data) {
        RIL_SipRegInfo *p = (RIL_SipRegInfo*)m_data;
        if (p->direction != NULL) {
            free(p->direction);
        }
        if (p->sip_msg_type != NULL) {
            free(p->sip_msg_type);
        }
        if (p->method != NULL) {
            free(p->method);
        }
        if (p->reason_phrase != NULL) {
            free(p->reason_phrase);
        }
        if (p->warn_text != NULL) {
            free(p->warn_text);
        }
        free(m_data);
        m_data = NULL;
    }
}
