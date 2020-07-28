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
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#include <mtkcam3/feature/utils/SecureBufferControlUtils.h>
#define ILOG_MODULE_TAG SecureUtil
#include <mtkcam3/feature/utils/log/ILogHeader.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_P2_PROC_COMMON);

namespace NSCam {
namespace Feature {

SecureBufferControl::SecureBufferControl()
{
    mLogLevel = ::property_get_int32("vendor.debug.securecamera.log", 0);
}

SecureBufferControl::~SecureBufferControl()
{
    std::lock_guard<std::mutex> _m(mMapLock);
    mMap.clear();
}

MVOID SecureBufferControl::init(NSCam::SecType secureType)
{
    mSecureType = secureType;
    mSecureEnum = toSecEnum(secureType);
}

android::sp<IImageBuffer> SecureBufferControl::allocSecureBuffer(size_t size)
{
    IImageBufferAllocator::ImgParam imgParam(size, 0);

    android::sp<ISecureImageBufferHeap> blobHeap =
        ISecureImageBufferHeap::create(LOG_TAG,
                imgParam,
                ISecureImageBufferHeap::AllocExtraParam(0, 1, 0, MTRUE, mSecureType));
    if (blobHeap == NULL) {
        MY_LOGE("Allocate secure tuning buffer failed!");
        return NULL;
    }

    return blobHeap->createImageBuffer();
}

SecureBufferControl::HANDLE_T SecureBufferControl::registerAndGetSecHandle(IImageBuffer *buf)
{
    HANDLE_T sechandle = NULL;
    if (buf == NULL) return NULL;
    std::lock_guard<std::mutex> _m(mMapLock);
    KEY_T key = buf->getFD(0);
    auto search = mMap.find(key);
    if (search != mMap.end()) {
        sechandle = std::get<0>(search->second);
    } else {
        android::sp<IImageBuffer> secbuf =
            allocSecureBuffer(buf->getBufSizeInBytes(0));
        if (secbuf == NULL) return NULL;
        MUINT const groupUsage =
            (eBUFFER_USAGE_HW_CAMERA_READWRITE |
             eBUFFER_USAGE_SW_READ_OFTEN |
             eBUFFER_USAGE_SW_WRITE_OFTEN);
        secbuf->lockBuf(LOG_TAG, groupUsage);
        sechandle = (HANDLE_T)secbuf->getBufVA(0);
        mMap.emplace(key, std::make_tuple(sechandle, secbuf));
        secbuf->unlockBuf(LOG_TAG);
    }
    return sechandle;
}

NSIoPipe::EDIPSecureEnum SecureBufferControl::getSecureEnum() const
{
    return mSecureEnum;
}

NSIoPipe::EDIPSecureEnum SecureBufferControl::toSecEnum(NSCam::SecType secureType) const
{
    NSIoPipe::EDIPSecureEnum secureEnum = NSIoPipe::EDIPSecureEnum::EDIPSecure_NONE;
    switch (secureType) {
        case SecType::mem_normal :
            secureEnum = NSIoPipe::EDIPSecureEnum::EDIPSecure_NONE;
            break;
        case SecType::mem_secure :
            secureEnum = NSIoPipe::EDIPSecureEnum::EDIPSecure_SECURE;
            break;
        case SecType::mem_protected :
            secureEnum = NSIoPipe::EDIPSecureEnum::EDIPSecure_PROTECT;
            break;
        default:
            MY_LOGW("Not support secure type (%d), reset secure enum to normal type!", secureType);
            secureEnum = NSIoPipe::EDIPSecureEnum::EDIPSecure_NONE;
            break;
    }
    return secureEnum;
}

} // namespace Feature
} // namespace NSCam
