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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

#ifndef _ISECURE_CAMERA_TYPES_H_
#define _ISECURE_CAMERA_TYPES_H_

#include <mtkcam/def/Errors.h>
#include <mtkcam/def/UITypes.h>

#include <cstdint>
#include <utility>

namespace NSCam {
namespace security {

// ---------------------------------------------------------------------------

using Result = android::status_t;

/// secure camera operating mode
enum class Mode : uint32_t {
    NONE = 0,
    IRIS
};

/// secure camera path
enum Path
{
    INVALID = 0,
    IR      = 1 << 0,
    BAYER   = 1 << 1,
};

/// Iris configuration
struct Configuration {
    Mode mode;
    NSCam::MSize size;

    Configuration()
        : mode(Mode::NONE),
          size()
    {};
};

/// secure memory
struct SecureMemory {
    // NOTE:
    // SECHAND need to match struct ion_sys_get_phys_param in <linux/ion_drv.h>
    typedef uint32_t SECHAND;
    SECHAND handle;
    // file descriotpr
    int fd;
    size_t  size;
};

/// Payload structure
struct RawAttribute {
    std::pair<int64_t, int> identifier;  // opaque unique identifier
    size_t       length;             // raw data length in bytes
    NSCam::MSize size;               // image width and height
    size_t       stride;             // image data stride
    int          format;             // image format (the value is the same as EImageFormat)
    int       orientation;           // image orientation
};

struct Buffer {
    RawAttribute    attribute;
    union Addr
    {
        void*           virtualAddress;     // buffer address
        SecureMemory    secureHandle;

        Addr() : virtualAddress(nullptr) {};
    } addr;

    Path path;
    void* priv; // a private information for specific purpose
};

// ---------------------------------------------------------------------------

} // namespace security
} // namespace NSCam
#endif //_ISECURE_CAMERA_TYPES_H_
