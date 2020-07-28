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

#define LOG_TAG "MtkCam/ISPModule"
#include "ISPModule.h"
#include "ISPDevice.h"
#include <vendor/mediatek/hardware/camera/isphal/1.0/types.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include <log/log.h>
#include <utils/Errors.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_ISP_HAL_SERVER);
using namespace NSCam::ISPHal;

namespace vendor {
namespace mediatek {
namespace hardware {
namespace camera {
namespace isphal {
namespace V1_0 {
namespace implementation {

using vendor::mediatek::hardware::camera::isphal::V1_0::IISPModule;
using vendor::mediatek::hardware::camera::isphal::V1_0::Status;

#define MY_LOGV(fmt, arg...)        CAM_ULOGMV(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)

#if 1
#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")
#else
#define FUNC_START
#define FUNC_END
#endif

using namespace std;
using namespace android;

// Methods from ::vendor::mediatek::hardware::camera::isphal::V1_0::IISPModule follow.
Return<void> ISPModule::openISPDevice(openISPDevice_cb _hidl_cb) {
    FUNC_START;
    ::android::Mutex::Autolock _l(mLock);
    android::sp<IDevice> pDevice = IISPManager::getInstance()->createISPDevice();
    if (pDevice != nullptr)
    {
        _hidl_cb(Status::OK, pDevice);
    }
    else
    {
        _hidl_cb(Status::INTERNAL_ERROR, nullptr);
    }
    FUNC_END;

    return Void();
}

/******************************************************************************
 *
 ******************************************************************************/
void ISPModule::serviceDied(uint64_t cookie __unused, const wp<hidl::base::V1_0::IBase>& who __unused)
{
    if (cookie != (uint64_t)this) {
        MY_LOGE("Unexpected ISPModule serviceDied cookie 0x%" PRIx64 ", expected %p", cookie, this);
    }
    CAM_LOGE("%s: ISPHal ISPModule serviceDied!!",__FUNCTION__);

}

// Methods from ::android::hidl::base::V1_0::IBase follow.

IISPModule* HIDL_FETCH_IISPModule(const char* /* name */) {
    CAM_LOGI("ISPHal  into HIDL_FETCH_IISPModule");
    return new ISPModule();
}


}  // namespace implementation
}  // namespace V1_0
}  // namespace isphal
}  // namespace camera
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

/******************************************************************************
 *
 ******************************************************************************/
IISPManager* IISPManager::getInstance()
{
    static ISPManager singleton;
    return &singleton;
}

/******************************************************************************
 *
 ******************************************************************************/
auto ISPManager::removeISPDevice(int32_t id) -> void
{
    ::android::Mutex::Autolock _l(mLock);
    if(mDeviceMap.indexOfKey(id)>=0)
    {
        mDeviceMap.removeItem(id);
        return;
    }
    MY_LOGW("Cannnot find ISP device(%d)", id);
}

/******************************************************************************
 *
 ******************************************************************************/
auto ISPManager::createISPDevice() -> android::sp<IDevice>
{
    ::android::Mutex::Autolock _l(mLock);
    android::sp<IDevice> pDevice = new vendor::mediatek::hardware::camera::isphal::V1_0::implementation::ISPDevice(mDeviceCount);
    if (pDevice != nullptr)
    {
        mDeviceMap.add(mDeviceCount, pDevice);
        MY_LOGI("Add ISP device(%d)", mDeviceCount);
        mDeviceCount++;
    }
    return pDevice;
}
