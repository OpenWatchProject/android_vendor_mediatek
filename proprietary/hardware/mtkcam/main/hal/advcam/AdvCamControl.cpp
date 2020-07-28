/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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

#define LOG_TAG "AdvCamControl_impl"
#include <dlfcn.h>
#include <mtkcam/feature/advCamSetting/CamCfgSetter.h>
#include <mtkcam/utils/std/Log.h>
#include <android/hardware/camera/device/3.4/types.h>
#include <system/camera_metadata.h>
#include "AdvCamControl.h"

namespace vendor {
namespace mediatek {
namespace hardware {
namespace camera {
namespace advcam {
namespace V1_0 {
namespace implementation {

typedef android::hardware::camera::device::V3_2::CameraMetadata CameraMetadata;

/* refer to hardware/interface/camera/device/3.4/default */
bool convertFromHidl(const CameraMetadata &src, const camera_metadata_t** dst) {
    if (src.size() == 0) {
        *dst = NULL;
        return true;
    }

    camera_metadata_t* srcData = ((camera_metadata_t*)src.data());
    // CameraMetadata size must match underlying camera_metadata_t
    if (get_camera_metadata_size(srcData) != src.size()) {
        CAM_LOGE("CameraMetadata input is corrupt");
        return false;
    }
    if (dst != NULL)
        *dst = srcData;
    return true;
}


// Methods from ::vendor::mediatek::hardware::camera::advcam::V1_0::IAdvCamControl follow.
Return<Status> AdvCamControl::setConfigureParam(uint32_t id, const CameraMetadata& configParam) {

    const camera_metadata_t* metadata = NULL;
    bool converted = convertFromHidl(configParam, &metadata);

    CAM_LOGD("id(%d), converted(%d)", id, converted);
    AdvCamControlWrap::getInstance()->setConfigureParam(id, metadata);
    return ::android::hardware::camera::common::V1_0::Status {};
}


// Methods from ::android::hidl::base::V1_0::IBase follow.

IAdvCamControl* HIDL_FETCH_IAdvCamControl(const char* /* name */) {
    CAM_LOGI("IAdvCamControl  into HIDL_FETCH_IAdvCamControl");
    return new AdvCamControl();
}

/* AdvCamControlWrap */

AdvCamControlWrap*
AdvCamControlWrap::getInstance()
{
    static AdvCamControlWrap inst;
    return &inst;
}

void
AdvCamControlWrap::setConfigureParam(uint32_t openId, const camera_metadata_t* metadata)
{
    char const szModulePath[] = "libmtkcam.adv_setting.so";
    char const szEntrySymbol[] = "getSetterInstance";
    void* pfnEntry = NULL;
    void *gLib = NULL;
    NSCam::CamCfgSetter *pCfgSetter = NULL;
    //
    gLib = ::dlopen(szModulePath, RTLD_NOW);
    if  ( ! gLib )
    {
        char const *err_str = ::dlerror();
        CAM_LOGE("dlopen: %s error=%s", szModulePath, (err_str ? err_str : "unknown"));
        goto lbExit;
    }
    //
    pfnEntry = ::dlsym(gLib, szEntrySymbol);
    if  ( ! pfnEntry )
    {
        char const *err_str = ::dlerror();
        CAM_LOGE("dlsym: %s error=%s", szEntrySymbol, (err_str ? err_str : "unknown"));
        goto lbExit;
    }
    //
    pCfgSetter = reinterpret_cast<NSCam::CamCfgSetter*(*)(uint32_t)>(pfnEntry)(openId);
    if  ( ! pCfgSetter )
    {
        ALOGE("No CamCfgSetter instance");
        goto lbExit;
    }

    if(metadata != NULL)
        pCfgSetter->setConfigureParam(metadata);
    else
        ALOGE("Config Metadata is NULL!!");

lbExit:

    if(gLib != NULL){
        ::dlclose(gLib);
        gLib = NULL;
    }

}

}  // namespace implementation
}  // namespace V1_0
}  // namespace advcam
}  // namespace camera
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor
