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

#ifndef VENDOR_MEDIATEK_HARDWARE_CAMERA3_MAIN_HAL_SECURITY_TESTS_TESTDEFS_H
#define VENDOR_MEDIATEK_HARDWARE_CAMERA3_MAIN_HAL_SECURITY_TESTS_TESTDEFS_H

#include <android/hardware_buffer.h>
#include <media/NdkImage.h>

#include <array>

// ------------------------------------------------------------------------

namespace NSCam {
namespace tests {
namespace TestDefs {

static constexpr int32_t kAImageWidth = 640;
static constexpr int32_t kAImageHeight = 480;
using ReaderUsageAndAFormat = std::pair<AHardwareBuffer_UsageFlags, int32_t>;
static constexpr std::array<ReaderUsageAndAFormat, 2> kReaderUsageAndFormat =
{
    std::make_pair(AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT, AIMAGE_FORMAT_PRIVATE),
    std::make_pair(AHARDWAREBUFFER_USAGE_CPU_READ_NEVER, AIMAGE_FORMAT_PRIVATE),
};
// The maximum number of images the user will want to access simultaneously.
// This should be as small as possible to limit memory use.
// Once maxImages Images are obtained by the user, one of them has to be
// released before a new AImage will become available for access through
// AImageReader_acquireLatestImage or AImageReader_acquireNextImage.
// Must be greater than 0.
static constexpr int32_t kMaxAImagesStreaming = 3;
static constexpr uint32_t kRequestCount = 10;

static constexpr bool kEnableSecureImageDataTest = ENABLE_SECURE_IMAGE_DATA_TEST;

} // namespace TestDefs
} // namespace tests
} // namespace NSCam

#endif // VENDOR_MEDIATEK_HARDWARE_CAMERA3_MAIN_HAL_SECURITY_TESTS_TESTDEFS_H
