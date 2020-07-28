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

#define LOG_TAG "ACameraMetadataCache"

#include "ACameraMetadataCache.h"

#include <mtkcam/utils/std/ULog.h>

// ------------------------------------------------------------------------

namespace NSCam {

CAM_ULOG_DECLARE_MODULE_ID(MOD_CAMERA_HAL_SERVER);

// ------------------------------------------------------------------------

ACameraMetadataCache::~ACameraMetadataCache()
{
    for (auto &&cameraMetadata : mACameraMetadataMap)
    {
        ACameraMetadata_free(const_cast<ACameraMetadata*>(cameraMetadata.second));
        CAM_ULOGMD("free ACameraMetadata of %s", cameraMetadata.first.c_str());
    }
    mACameraMetadataMap.clear();
}

const ACameraMetadata* ACameraMetadataCache::getACameraMetadata(
    ACameraManager* manager, const char* cameraId)
{
    if (!manager || !cameraId)
    {
        CAM_ULOGME("invalid manager or camera ID, do nothing");
        return nullptr;
    }

    auto search = mACameraMetadataMap.find(cameraId);
    if (search == mACameraMetadataMap.end())
    {
        ACameraMetadata* staticMetadata = nullptr;

        // query the capabilities of a camera device.
        // These capabilities are immutable for a given camera.
        //
        // The caller must call ACameraMetadata_free to free the memory of
        // the output characteristics
        camera_status_t ret = ACameraManager_getCameraCharacteristics(
                manager, cameraId, &staticMetadata);
        CAM_ULOGM_ASSERT(ret == ACAMERA_OK,
                "get camera characteristics failed(%d)", ret);
        CAM_ULOGM_ASSERT(staticMetadata != nullptr, "invalid static metadata");

        mACameraMetadataMap.emplace(cameraId, staticMetadata);
        CAM_ULOGMD("cache ACameraMetadata of %s", cameraId);

        return staticMetadata;
    }

    return search->second;
}

} // namespace NSCam