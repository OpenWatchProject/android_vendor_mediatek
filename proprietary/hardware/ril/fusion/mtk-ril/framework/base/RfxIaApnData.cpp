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

#include <string.h>
#include "RfxIaApnData.h"
#include "RfxLog.h"

RFX_IMPLEMENT_DATA_CLASS(RfxIaApnData);

#define RFX_LOG_TAG "RfxIaApnData"

RfxIaApnData::RfxIaApnData(void *data, int length) : RfxBaseData(data, length) {
    m_length = length;
    RFX_ASSERT(m_length == sizeof(RIL_InitialAttachApn_v15));

    int result;

    RIL_InitialAttachApn_v15 *pTmp =
            (RIL_InitialAttachApn_v15 *) calloc(1, sizeof(RIL_InitialAttachApn_v15));
    if (pTmp == NULL) {
        m_data = NULL;
        return;
    }
    RIL_InitialAttachApn_v15 *pCur = (RIL_InitialAttachApn_v15 *) data;

    copyString(&(pTmp->apn), pCur->apn);
    copyString(&(pTmp->protocol), pCur->protocol);
    copyString(&(pTmp->roamingProtocol), pCur->roamingProtocol);
    copyString(&(pTmp->username), pCur->username);
    copyString(&(pTmp->password), pCur->password);

    pTmp->authtype = pCur->authtype;
    m_data = pTmp;
}

RfxIaApnData::~RfxIaApnData() {
    RIL_InitialAttachApn_v15* pTmp = (RIL_InitialAttachApn_v15 *)m_data;
    // free memory
    if (pTmp != NULL) {
        if(pTmp->apn != NULL) {
            free(pTmp->apn);
        }
        if(pTmp->protocol != NULL) {
            free(pTmp->protocol);
        }
        if(pTmp->roamingProtocol != NULL) {
            free(pTmp->roamingProtocol);
        }
        if(pTmp->username != NULL) {
            free(pTmp->username);
        }
        if(pTmp->password != NULL) {
            free(pTmp->password);
        }
        free(pTmp);
    }
}

void RfxIaApnData::copyString(char **dst, char *src) {
    if (src != NULL) {
        *dst = (char *) calloc(strlen(src) + 1, sizeof(char));
        if (*dst == NULL) {
            RFX_LOG_E(RFX_LOG_TAG, "OOM");
            return;
        }
        strncpy(*dst, src, strlen(src));
    } else {
        *dst = (char *) calloc(1, sizeof(char));
        if (*dst == NULL) {
            RFX_LOG_E(RFX_LOG_TAG, "OOM");
            return;
        }
        (*dst)[0] = 0;
    }
}