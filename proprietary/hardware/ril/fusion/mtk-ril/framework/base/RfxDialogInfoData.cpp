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

#include "RfxDialogInfoData.h"

#include "RfxLog.h"
#define RFX_LOG_TAG "RfxDialogInfoData"

RFX_IMPLEMENT_DATA_CLASS(RfxDialogInfoData);

RfxDialogInfoData::RfxDialogInfoData(void *data, int length) : RfxBaseData(data, length) {
    if (data == NULL) {
        m_data = NULL;
        return;
    }

    RFX_LOG_D(RFX_LOG_TAG, "RfxDialogInfoData start length = %d", length);
    m_length = length;
    m_data = NULL;
    int countOfDialog = m_length / sizeof(RIL_DialogInfo *);
    RIL_DialogInfo** dialogInfo = (RIL_DialogInfo **)calloc(countOfDialog, sizeof(RIL_DialogInfo*));
    RFX_ASSERT(dialogInfo != NULL);
    if (countOfDialog > 0) {
        RIL_DialogInfo ** tmp = (RIL_DialogInfo **)data;
        for (int i = 0; i < countOfDialog; i++) {
            RFX_LOG_D(RFX_LOG_TAG, "RfxDialogInfoData memcpy i = %d", i);
            dialogInfo[i] = (RIL_DialogInfo *)calloc(1, sizeof(RIL_DialogInfo));
            RFX_ASSERT(dialogInfo[i] != NULL);
            memset(dialogInfo[i], 0, sizeof(RIL_DialogInfo));
            //memcpy(dialogInfo[i], tmp[i], sizeof(RIL_DialogInfo));

            dialogInfo[i]->dialogId = tmp[i]->dialogId;
            dialogInfo[i]->callState = tmp[i]->callState;
            dialogInfo[i]->callType = tmp[i]->callType;
            dialogInfo[i]->isCallHeld = tmp[i]->isCallHeld;
            dialogInfo[i]->isPullable = tmp[i]->isPullable;
            dialogInfo[i]->isMt = tmp[i]->isMt;

            if (tmp[i]->address != NULL) {
                int len = strlen(tmp[i]->address);
                //Allocate and zero-initialize array
                dialogInfo[i]->address = (char *)calloc(len + 1, sizeof(char));
                strncpy(dialogInfo[i]->address, tmp[i]->address, len);
            }
            if (tmp[i]->remoteAddress != NULL) {
                int len = strlen(tmp[i]->remoteAddress);
                //Allocate and zero-initialize array
                dialogInfo[i]->remoteAddress = (char *)calloc(len + 1, sizeof(char));
                strncpy(dialogInfo[i]->remoteAddress, tmp[i]->remoteAddress, len);
            }
            /*RFX_LOG_V(RFX_LOG_TAG,
                    "RfxDialogInfoData set dialogId:%d, address: %s, remoteAddress: %s",
                    dialogInfo[i]->dialogId, dialogInfo[i]->address, dialogInfo[i]->remoteAddress);*/
        }
    }
    RFX_LOG_D(RFX_LOG_TAG, "RfxDialogInfoData end");
    m_data = dialogInfo;
}

RfxDialogInfoData::~RfxDialogInfoData() {
    // free
    int countOfCall = m_length / sizeof(RIL_DialogInfo *);
    RIL_DialogInfo ** dialog_info = (RIL_DialogInfo **)m_data;

    if (dialog_info != NULL) {
        for (int i = 0; i < countOfCall; i++) {
            if (dialog_info[i] != NULL) {
                if (dialog_info[i]->address != NULL) {
                    free(dialog_info[i]->address);
                }
            }
        }
        free(dialog_info);
    }
    m_data = NULL;
    RFX_LOG_D(RFX_LOG_TAG, "~RfxDialogInfoData");
}
