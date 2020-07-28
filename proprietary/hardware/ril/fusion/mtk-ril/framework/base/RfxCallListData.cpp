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

#include "RfxCallListData.h"

RFX_IMPLEMENT_DATA_CLASS(RfxCallListData);

RfxCallListData::RfxCallListData(void *data, int length) : RfxBaseData(data, length)  {
    if (data == NULL) {
        m_data = NULL;
        return;
    }

    m_length = length;
    m_data = NULL;
    int countOfCall = m_length / sizeof(RIL_Call *);
    RIL_Call ** pp_calls = (RIL_Call **)calloc(countOfCall, sizeof(RIL_Call *));
    RFX_ASSERT(pp_calls != NULL);

    if (countOfCall > 0) {
        RIL_Call ** tmp = (RIL_Call **)data;
        for (int i = 0; i < countOfCall; i++) {
            pp_calls[i] = (RIL_Call *)calloc(1, sizeof(RIL_Call));
            RFX_ASSERT(pp_calls[i] != NULL);
            memset(pp_calls[i], 0, sizeof(RIL_Call));
            memcpy(pp_calls[i], tmp[i], sizeof(RIL_Call));
            if (tmp[i]->number != NULL) {
                int len = strlen(tmp[i]->number);
                //Allocate and zero-initialize array
                pp_calls[i]->number = (char *)calloc(len + 1, sizeof(char));
                RFX_ASSERT(pp_calls[i]->number != NULL);
                strncpy(pp_calls[i]->number, tmp[i]->number, len);
            }
            if (tmp[i]->name != NULL) {
                int len = strlen(tmp[i]->name);
                //Allocate and zero-initialize array
                pp_calls[i]->name = (char *)calloc(len + 1, sizeof(char));
                RFX_ASSERT(pp_calls[i]->name != NULL);
                strncpy(pp_calls[i]->name, tmp[i]->name, len);
            }
        }
    }

    m_data = pp_calls;
}

RfxCallListData::~RfxCallListData() {
    // free
    int countOfCall = m_length / sizeof(RIL_Call *);
    RIL_Call ** pp_calls = (RIL_Call **)m_data;
    if (pp_calls != NULL) {
        for (int i = 0; i < countOfCall; i++) {
            if (pp_calls[i] != NULL) {
                if (pp_calls[i]->number != NULL) {
                    free(pp_calls[i]->number);
                }
                if (pp_calls[i]->name != NULL) {
                    free(pp_calls[i]->name);
                }
                free(pp_calls[i]);
            }
        }
        free(pp_calls);
    }
    m_data = NULL;
}
