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
 * MediaTek Inc. (C) 2014. All rights reserved.
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

#ifndef __DRM_CTA_UTIL_H__
#define __DRM_CTA_UTIL_H__

#include <drm/drm_framework_common.h>
#include <DrmEngineBase.h>
#include <utils/RefBase.h>
#include <utils/String8.h>
#include <utils/threads.h>
#include <stdio.h>

namespace android {

class DrmCtaUtil {
public:
    DrmCtaUtil();

public:
    /*
     * Get the file full path from fd
     */
    static String8 getPathFromFd(int fd);
    /**
     * Generate a Uintvar format value from a normal value
     */
    static String8 getUintVarFromNormal(uint32_t normal);
    /**
     * Get normal value from a Uintvar format value
     */
    static uint32_t getNormalFromUintVar(String8 uvar);

    static String8 getErrorCallbackMsg(String8 path, String8 flag);
private:
    String8 mCtaKey;
    //static vector<String8> sCtaWhiteList;

public:
    class Listener {
        public:
            Listener() : mListener(NULL), mUniqueId(-1) {}

            Listener(IDrmEngine::OnInfoListener *listener, int uniqueId)
                : mListener(listener), mUniqueId(uniqueId) {};

            IDrmEngine::OnInfoListener *GetListener() const {return mListener;}
            int GetUniqueId() const {return mUniqueId;}

        private:
            IDrmEngine::OnInfoListener *mListener;
            int mUniqueId;
    };

public:
    static bool notify(const Vector<DrmCtaUtil::Listener> *infoListener, String8 progress);
    static bool isTrustCtaClient(pid_t pid);
    static bool isTrustCtaClient(String8 &processName);
    static bool IsCtaTrustedCheckTokenClient(String8 &processName);
    static bool IsCtaTrustedGetTokenClient(String8 &processName);
};

}

#endif // __DRM_CTA_UTIL_H__
