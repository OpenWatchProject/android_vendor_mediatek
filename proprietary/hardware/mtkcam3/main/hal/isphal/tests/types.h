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

#ifndef VENDOR_MEDIATEK_HARDWARE_CAMERA3_MAIN_HAL_ISPHAL_TESTS_TYPES_H
#define VENDOR_MEDIATEK_HARDWARE_CAMERA3_MAIN_HAL_ISPHAL_TESTS_TYPES_H

#include <regex>
#include <string>
#include <vector>
#include <utility>

#include <json/json.h>

#include <android/hardware/graphics/common/1.0/types.h>

namespace isphal_test {

    enum class ISP_META_TAG : uint32_t {
        MTK_ISP_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING = 0x800b000b,
        MTK_ISP_CONTROL_CAPTURE_HINT_FOR_ISP_FRAME_COUNT,
        MTK_CONTROL_CAPTURE_HINT_FOR_ISP_FRAME_INDEX
    };

    struct File
    {
        std::string rootDirectory;
        std::string fileName;

        Json::UInt width;
        Json::UInt height;
        Json::Int  format;
        std::vector<Json::UInt> rowStride;

        Json::UInt size;

        explicit File(const std::string& _rootDirectory, const std::string& _fileName,
                Json::UInt _width, Json::UInt _height, Json::Int _format,
                std::vector<Json::UInt>&& _rowStride, Json::UInt _size)
            : rootDirectory(_rootDirectory), fileName(_fileName), width(_width),
              height(_height), format(_format), size(_size)
        {
            rowStride = std::move(_rowStride);
        }

        std::string getFullyQualifiedName() const
        {
            return std::string (rootDirectory + "/" + fileName);
        };

        bool isJPEG() const
        {
            std::regex rx(".*\\.(jpeg|jpg)$", std::regex::icase);
            return std::regex_match(fileName, rx);
        };

        ::android::hardware::graphics::common::V1_0::PixelFormat getPixelFormat() const
        {
            return ::android::hardware::graphics::common::V1_0::PixelFormat(format);
        };
    };
} // namespace isphal_test

#endif // VENDOR_MEDIATEK_HARDWARE_CAMERA3_MAIN_HAL_ISPHAL_TESTS_TYPES_H
