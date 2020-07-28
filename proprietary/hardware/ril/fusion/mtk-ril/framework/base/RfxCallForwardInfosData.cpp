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
#include "RfxBaseData.h"
#include <telephony/ril.h>
#include "RfxCallForwardInfosData.h"

RFX_IMPLEMENT_DATA_CLASS(RfxCallForwardInfosData);

RfxCallForwardInfosData::RfxCallForwardInfosData(void *data, int length) : RfxBaseData(data, length)  {
    m_length = length;
    m_data = NULL;
    if (data != NULL) {
        int num = length / sizeof(RIL_CallForwardInfo *);

        if (num > 0) {
            RIL_CallForwardInfo **pTmp = (RIL_CallForwardInfo **) calloc(num, sizeof(RIL_CallForwardInfo *));
            RIL_CallForwardInfo **pCur = (RIL_CallForwardInfo **) data;

            if (pTmp != NULL) {
                m_length = length;
                for (int i = 0; i < num; i++) {
                    pTmp[i] = (RIL_CallForwardInfo *) calloc(1, sizeof(RIL_CallForwardInfo));

                    if (pTmp[i] != NULL) {
                        pTmp[i] -> status       = pCur[i] -> status;
                        pTmp[i] -> reason       = pCur[i] -> reason;
                        pTmp[i] -> serviceClass = pCur[i] -> serviceClass;
                        pTmp[i] -> toa          = pCur[i] -> toa;

                        if (pCur[i]->number != NULL) {
                            int len = strlen(pCur[i]->number);
                            pTmp[i]->number = (char *) calloc(len + 1, sizeof(char));
                            if (pTmp[i]->number != NULL) {
                                strncpy(pTmp[i]->number, pCur[i]->number, len);
                            }
                        }

                        pTmp[i] -> timeSeconds  = pCur[i] -> timeSeconds;
                    }
                }
                m_data = pTmp;
            }
        }
    }
}

RfxCallForwardInfosData::~RfxCallForwardInfosData() {
    int num = m_length / sizeof(RIL_CallForwardInfo *);
    RIL_CallForwardInfo **pCur = (RIL_CallForwardInfo **) m_data;

    if (m_data != NULL) {
        for (int i = 0; i < num; i++) {
            if (pCur[i]->number != NULL) {
                free(pCur[i] -> number);
            }
            free(pCur[i]);
        }
        free(pCur);
        m_data = NULL;
    }
}