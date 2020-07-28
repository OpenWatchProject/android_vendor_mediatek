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

#include "RfxImsConfData.h"

RFX_IMPLEMENT_DATA_CLASS(RfxImsConfData);

RfxImsConfData::RfxImsConfData(void *data, int length) : RfxBaseData(data, length)  {
    if (data == NULL) {
        m_length = 0;
        m_data = NULL;
        return;
    }

    m_length = length;
    m_data = NULL;
    RIL_Conference_Participants* srcData = (RIL_Conference_Participants*)data;
    int count = m_length/sizeof(RIL_Conference_Participants);
    RIL_Conference_Participants* dupData = (RIL_Conference_Participants*)
            calloc(count, sizeof(RIL_Conference_Participants));
    RFX_ASSERT(dupData != NULL);
    for(int i = 0; i < count; i++) {
        int len = strlen(srcData[i].useraddr);
        dupData[i].useraddr =
                (char*)calloc(len + 1, sizeof(char));
        RFX_ASSERT(dupData[i].useraddr != NULL);
        strncpy(dupData[i].useraddr, srcData[i].useraddr, len);
        len = strlen(srcData[i].end_point);
        dupData[i].end_point =
                (char*)calloc(strlen(srcData[i].end_point) + 1, sizeof(char));
        RFX_ASSERT(dupData[i].end_point != NULL);
        strncpy(dupData[i].end_point, srcData[i].end_point, len);
        len = strlen(srcData[i].entity);
        dupData[i].entity =
                (char*)calloc(strlen(srcData[i].entity) + 1, sizeof(char));
        RFX_ASSERT(dupData[i].entity != NULL);
        strncpy(dupData[i].entity, srcData[i].entity, len);
        len = strlen(srcData[i].display_text);
        dupData[i].display_text = (char*)calloc(strlen(srcData[i].display_text) + 1, sizeof(char));
        RFX_ASSERT(dupData[i].display_text != NULL);
        strncpy(dupData[i].display_text, srcData[i].display_text, len);
        len = strlen(srcData[i].status);
        dupData[i].status = (char*)calloc(strlen(srcData[i].status) + 1, sizeof(char));
        RFX_ASSERT(dupData[i].status != NULL);
        strncpy(dupData[i].status, srcData[i].status, len);
    }
    m_data = dupData;
}

RfxImsConfData::~RfxImsConfData() {
    // free
    int count = m_length/sizeof(RIL_Conference_Participants);
    RIL_Conference_Participants* dupData = (RIL_Conference_Participants*) m_data;
    if (dupData != NULL) {
        for (int i = 0; i < count; i++) {
            free(dupData[i].useraddr);
            free(dupData[i].end_point);
            free(dupData[i].display_text);
            free(dupData[i].status);
            free(dupData[i].entity);
        }
        free(dupData);
    }
    m_length = 0;
    m_data = NULL;
}
