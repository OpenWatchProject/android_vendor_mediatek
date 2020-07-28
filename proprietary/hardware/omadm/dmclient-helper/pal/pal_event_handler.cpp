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

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <utils/Errors.h>

#include "pal_event_handler.h"
#include "pal_internal.h"


namespace android {
#undef LOG_TAG
#define LOG_TAG "pal_event_handler"

#define WAP_PUSH_HDR_SZ 24
#define SU_CANCEL_DATA_SZ 13
#define SU_CANCEL_TRIGGER_STR "1010101010101"
#define SU_CANCEL_TRIGGER_STR_UTF16 u"1010101010101"

int OmadmEventHandler::WapPushSysUpdate(int trans_id, String8* pdu)
{
    UNUSED(trans_id);
    int ret = RESULT_MEMORY_ERROR;

    int pkg0_size = pdu->size();
    pdu_t * pkg0 = (pdu_t*)malloc(sizeof(pdu_t));
    pkg0->length = pkg0_size;
    pkg0->data = (char*)malloc(pkg0_size);
    const char* data = (*pdu).string();

    if (pkg0->data) {
        for (int i = 0; i < pkg0_size; i++) {
            pkg0->data[i] = data[i];
            ALOGI("ptr[%d] = %x\n", i, pkg0->data[i] );
        }
        int srv_id_length = pkg0->data[WAP_PUSH_HDR_SZ-1];
        int vnd_data_length = pkg0_size - srv_id_length - WAP_PUSH_HDR_SZ;
        if (vnd_data_length == SU_CANCEL_DATA_SZ) {
            /* got SU CANCEL command in vendor specific field */
            int vnd_data_offset = WAP_PUSH_HDR_SZ + srv_id_length;
            if (!strncmp(&pkg0->data[vnd_data_offset], SU_CANCEL_TRIGGER_STR,
                    vnd_data_length)) {
                ret = mEventNotifyCallback(SU_CANCEL, pkg0);
            }
        } else if (vnd_data_length != 0) {
            ALOGE("OmadmEventHandler::WapPushSysUpdate() msg format fault,"
                " pkg0 size %d, srv_id_length %d, vendor data size %d, ",
                pkg0_size, srv_id_length, vnd_data_length);
            ret = RESULT_PARSE_ERROR;
        } else if (mEventNotifyCallback) {
            ALOGI("OmadmEventHandler::WapPushSysUpdate() bytes = %d, server name length = %d",
                pkg0_size, srv_id_length);
            ret = mEventNotifyCallback(PACKAGE0, pkg0);
            ALOGI("OmadmEventHandler::WapPushSysUpdate() package0 sent");
        }
    }

    if (ret != RESULT_SUCCESS) {
        if (pkg0->data != NULL) free(pkg0->data);
        if (pkg0 != NULL) free(pkg0);
    }
    
    return ret;
};


int OmadmEventHandler::SmsCancelSysUpdate(String16* sms_user_data)
{
    int cbytes = sms_user_data->size();
    int ret = RESULT_MEMORY_ERROR;
    ALOGI("OmadmEventHandler::SmsCancelSysUpdate");
    if (!memcmp(sms_user_data->string(),SU_CANCEL_TRIGGER_STR_UTF16, cbytes << 1)) {
        if (mEventNotifyCallback) {
            ret = mEventNotifyCallback(SU_CANCEL, NULL);
            ALOGI("EH:SU cancel event sent");
        }
    } else {
        ALOGI("EH:SU cancel trigger didn't match");
        ret = RESULT_PARSE_ERROR;
    }
    return ret;
};

}

