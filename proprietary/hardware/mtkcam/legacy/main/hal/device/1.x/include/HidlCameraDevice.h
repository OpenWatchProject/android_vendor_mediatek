/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

#ifndef _MTK_HARDWARE_MTKCAM_MAIN_HAL_DEVICE_1_X_INCLUDE_HIDLCAMERADEVICE_H_
#define _MTK_HARDWARE_MTKCAM_MAIN_HAL_DEVICE_1_X_INCLUDE_HIDLCAMERADEVICE_H_
//
#include "HidlCameraCommon.h"
//#include "utils/Mutex.h"

#include <android/hardware/camera/device/1.0/types.h>
#include <android/hardware/camera/device/1.0/ICameraDevicePreviewCallback.h>
#include <android/hardware/camera/device/1.0/ICameraDeviceCallback.h>
#include <android/hardware/camera/device/1.0/ICameraDevice.h>
//
#include <android/hidl/allocator/1.0/IAllocator.h>
#include <android/hidl/memory/1.0/IMemory.h>
//
#include <vendor/mediatek/hardware/camera/device/1.1/types.h>
#include <vendor/mediatek/hardware/camera/device/1.1/IMtkCameraDevice.h>
#include <vendor/mediatek/hardware/camera/device/1.1/IMtkCameraDeviceCallback.h>

//[TODO] Copy the files to local path
#include "HandleImporter.h"
#include "CameraModule.h"



using ::android::hardware::Return;
using namespace ::android::hardware::camera::device::V1_0;
using ::android::hardware::Void;
using ::android::hardware::hidl_handle;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_string;
using ::android::hidl::base::V1_0::IBase;
//
using ::android::hardware::camera::common::V1_0::helper::HandleImporter;
using ::android::hardware::graphics::common::V1_0::BufferUsage;
using ::android::hardware::graphics::common::V1_0::PixelFormat;
using ::android::hidl::allocator::V1_0::IAllocator;
using ::android::hidl::memory::V1_0::IMemory;
using ::android::hardware::hidl_memory;
//
using namespace ::vendor::mediatek::hardware::camera::device::V1_1;

/*
using ::android::sp;
using ::android::status_t;
using ::std::shared_ptr;
using ::android::hardware::camera::common::V1_0::helper::HandleImporter;
using ::android::hardware::graphics::common::V1_0::BufferUsage;
using ::android::hardware::graphics::common::V1_0::PixelFormat;
using ::android::hidl::allocator::V1_0::IAllocator;
using ::android::hidl::memory::V1_0::IMemory;
using ::android::hardware::hidl_memory;*/


static uint32_t const   kMajorDeviceVersion = 1;            // major version
static uint32_t const   kMinorDeviceVersion = 0;            // minor version

#endif  //_MTK_HARDWARE_MTKCAM_MAIN_HAL_DEVICE_1_X_INCLUDE_CAMERADEVICE1_H_

