/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2019. All rights reserved.
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

#define LOG_TAG "MtkCam/PostDevice"

#include "PostDevice.h"


#include <cutils/log.h>
//#include <system/graphics-base-v1.0.h>
#include <hardware/gralloc.h>

// AOSP
#include <log/log.h>
#include <utils/Errors.h>

#include <mtkcam/utils/LogicalCam/IHalLogicalDeviceList.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam3/pipeline/hwnode/StreamId.h>

#include <mtkcam3/feature/chromaNR/IChromaNR.h>

// STD
#include <memory>


// using ::android::hardware::graphics::common::V1_0::Dataspace;
// using ::android::hardware::graphics::common::V1_0::PixelFormat;

namespace vendor {
namespace mediatek {
namespace hardware {
namespace camera {
namespace postproc {
namespace V1_0 {
namespace implementation {

#define MY_LOGV(fmt, arg...)        CAM_LOGV(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)

using namespace std;
using namespace NSCam;
using namespace chromaNR;
using namespace android;
using namespace android::hardware;
using namespace vendor::mediatek::hardware::camera::postproc::V1_0;

IPostDevice* HIDL_FETCH_IPostDevice(const char* /* name */)
{
    CAM_LOGI("IPostDevice into %s", __FUNCTION__);
    return new PostDevice();
}

/******************************************************************************
 *
 ******************************************************************************/

PostDevice::
~PostDevice()
{
    MY_LOGI("destroy Post device");
}

PostDevice::
PostDevice()
{
    MY_LOGI("create Post device");
}

auto
PostDevice::
doChromaNR(
    const ::vendor::mediatek::hardware::camera::postproc::V1_0::ChromaNRParam& param
) -> Return<int32_t>
{

    int32_t status = 0;

    auto pHalLogicalDeviceList = MAKE_HalLogicalDeviceList();
    if(pHalLogicalDeviceList == nullptr) {
        MY_LOGF("Create HalLogicalDeviceList fail");
        return -1;
    }

    int deviceId;
    deviceId = pHalLogicalDeviceList->getDeviceIdByVID(param.open_id);
    MY_LOGD("Virtual id(%d), openId(%d)", param.open_id, deviceId);

    chromaNR::ChromaNRParam nrParam;
    nrParam.open_id = deviceId;
    nrParam.current_iso = param.current_iso;
    nrParam.src_width = param.src_width;
    nrParam.src_height = param.src_height;
    nrParam.src_yPitch = param.src_yPitch;
    nrParam.src_uvPitch = param.src_uvPitch;
    nrParam.src_format = param.src_format;
    for(auto i = 0; i < sizeof(param.src_sizeList); i++) {
        nrParam.src_sizeList[i] = param.src_sizeList[i];
    }
    nrParam.inputHandle = param.inputHandle;

    std::shared_ptr<IChromaNR> core = IChromaNR::createInstance();
    core->doChromaNR(nrParam);

    return status;
}



}  // namespace implementation
}  // namespace V1_0
}  // namespace postproc
}  // namespace camera
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor
