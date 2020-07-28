/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

/*******************************************************************************
 *
 * Filename:
 * ---------
 * AudioCustEncryptClient.cpp
 *
 * Project:
 * --------
 *   Android
 *
 * Description:
 * ------------
 *   This file a client for AudioCustEncrypt
 *
 * Author:
 * -------
 *   Tina Tsai
 *
 *******************************************************************************/
#include "AudioCustEncryptClient.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioCustEncryptClient"

namespace android {

/*==============================================================================
 *                     Implementation
 *============================================================================*/

AudioCustEncryptClient *AudioCustEncryptClient::mAudioCustEncryptClient = NULL;
AudioCustEncryptClient *AudioCustEncryptClient::GetInstance() {
    static Mutex mGetInstanceLock;
    Mutex::Autolock _l(mGetInstanceLock);

    if (mAudioCustEncryptClient == NULL) {
        mAudioCustEncryptClient = new AudioCustEncryptClient();
    }
    ASSERT(mAudioCustEncryptClient != NULL);
    return mAudioCustEncryptClient;
}

AudioCustEncryptClient::AudioCustEncryptClient() {
    ALOGD("%s()", __FUNCTION__);
    acpOpsInited = 0;
    Init();
}

AudioCustEncryptClient::~AudioCustEncryptClient() {
    ALOGD("%s()", __FUNCTION__);
    Deinit();
}

void AudioCustEncryptClient::Init(void) {
    const char *error;
    const char *funName = NULL;
    ALOGD("%s(), acpOpsInited(%d)", __FUNCTION__, acpOpsInited);

    if (acpOpsInited == 0) {
        ALOGD("%s(), init AcpOps struct", __FUNCTION__);

        /* dlopen */
        handle = dlopen("libaudiocustencrypt.so", RTLD_LAZY);
        if (handle == NULL) {
            ALOGE("%s(), dlopen fail! (%s)\n", __FUNCTION__, dlerror());
        } else {
            dlerror();    /* Clear any existing error */
            /* dlsym */
            funName = "Initial";
            ALOGD("%s(), dlsym %s", __FUNCTION__, funName);
            Initial = (int (*)(void)) dlsym(handle, funName);
            error = dlerror();
            if (error != NULL) {
                ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
            }

            funName = "EncryptProcess";
            ALOGD("%s(), dlsym %s", __FUNCTION__, funName);
            EncryptProcess = (int (*)(char *TargetBuf, char *SourceBuf, uint16_t SourceByte)) dlsym(handle, funName);
            error = dlerror();
            if (error != NULL) {
                ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
            }

            funName = "DecryptProcess";
            ALOGD("%s(), dlsym %s", __FUNCTION__, funName);
            DecryptProcess = (int (*)(char *TargetBuf, char *SourceBuf, uint16_t SourceByte)) dlsym(handle, funName);
            error = dlerror();
            if (error != NULL) {
                ALOGE("%s(), dlsym %s fail. (%s)\n", __FUNCTION__, funName, error);
            }

            acpOpsInited = 1;
        }
    }

    ALOGD("-%s(), acpOpsInited(%d)", __FUNCTION__, acpOpsInited);
}

void AudioCustEncryptClient::Deinit() {
    ALOGD("+%s(), acpOpsInited (%d)\n", __FUNCTION__, acpOpsInited);
    if (acpOpsInited != 0) {
        dlclose(handle);
        acpOpsInited = 0;
    }
    ALOGD("-%s(), acpOpsInited (%d)\n", __FUNCTION__, acpOpsInited);
}


}
