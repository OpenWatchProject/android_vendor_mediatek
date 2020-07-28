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

#include <CamPostAlgoService.h>
#include <binder/IServiceManager.h>
#include <CamPostAlgoProviderMgr.h>
#include <utils/String8.h>
#include <log/Log.h>

using namespace com::mediatek::campostalgo;
using namespace android;

#define LOG_TAG "CamPostAlgoService"

#define STATUS_ERROR_FMT(errorCode, errorString, ...) \
    Status::fromServiceSpecificError(errorCode, \
            String8::format("%s:%d: " errorString, __FUNCTION__, __LINE__, \
                    __VA_ARGS__))

CamPostAlgoService::CamPostAlgoService() :
        mInitialized(false) {

}

CamPostAlgoService::~CamPostAlgoService() {

}

status_t CamPostAlgoService::enumerateInterfaces() {
    CamPostAlgoProviderMgr::getInstance();
    ALOGD("enumerateInterfaces");
    return OK;
}

Status CamPostAlgoService::connect(const ::android::String16& opPackageName,
        int32_t clientUid,
        ::android::sp<::com::mediatek::campostalgo::ICamPostAlgoInterface>* interface) {
    ALOGD("connect");
    if (mInitialized) {
        *interface = CamPostAlgoProviderMgr::getInstance()->getInterface();
        return Status::ok();
    } else {
        ALOGE("service has not initialized");
        *interface = nullptr;
        return Status::fromServiceSpecificError(Status::EX_SERVICE_SPECIFIC,
                String8::format("%s:%d: service hasn't been inited",
                        __FUNCTION__, __LINE__));
    }
}

void CamPostAlgoService::onFirstRef() {
    status_t res;
    res = enumerateInterfaces();
    if (res == OK) {
        mInitialized = true;
    }
}
