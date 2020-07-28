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
 * MediaTek Inc. (C) 2019. All rights reserved.
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

#include <PipelineConfigure.h>

#define LOG_TAG "PipelineConfigure"

namespace NSPA {

PipelineConfigure::PipelineConfigure() {
    ALOGD("PipelineConfigure created!");

}

void PipelineConfigure::destroy() {
    android::Mutex::Autolock _l(mPipeConfigLock);
    mStreamAlgos.clear();
    mAppStreamMap.clear();
    sp<android::Surface> sf;

    for (int i = 0; i < mAppStreamInfoMap.size(); i++) {
        sf = mAppStreamInfoMap[i]->mStream;
        if (android::Surface::isValid(sf)) {
            sp<ANativeWindow> aNativeWindow(sf);
            int err = native_window_api_disconnect(aNativeWindow.get(),
                    NATIVE_WINDOW_API_CPU);
            if (err != android::NO_ERROR) {
                ALOGE("[%s] disconnect App Stream %s failed: %s, return",
                        __func__, sf->getConsumerName().string(),
                        strerror(-err));
            } else {
                ALOGD("disconnect App Stream name is %s",
                        sf->getConsumerName().string());
            }
        }
    }
    mAppStreamInfoMap.clear();
    mHalStreamMap.clear();
    for (int i = 0; i < mHalStreams.size(); i++) {
        sf = mHalStreams[i];
        if (android::Surface::isValid(sf)) {
            sp<ANativeWindow> aNativeWindow(sf);
            int err = native_window_api_disconnect(aNativeWindow.get(),
                    NATIVE_WINDOW_API_CAMERA);
            if (err != android::NO_ERROR) {
                ALOGE("[%s] disconnect Hal Stream failed: %s, return", __func__,
                        strerror(-err));
            } else {
                ALOGD("disconnect Hal Stream name is %s",
                        sf->getConsumerName().string());
            }
        }
    }
    mHalStreams.clear();
    mPABuffers.clear();

}

PipelineConfigure::~PipelineConfigure() {
    ALOGD("PipelineConfigure destroyed %p!", this);
}

} /* namespace NSPA */
