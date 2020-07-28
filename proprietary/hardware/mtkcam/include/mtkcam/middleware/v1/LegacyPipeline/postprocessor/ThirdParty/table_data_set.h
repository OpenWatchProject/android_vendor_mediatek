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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_LAGACYPIPELINE_PP_TABLE_DATA_SET_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_LAGACYPIPELINE_PP_TABLE_DATA_SET_H_
#include <string>
#include <vector>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/ContextBuilder/ImageStreamManager.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/ContextBuilder/MetaStreamManager.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/ContextBuilder/NodeConfigDataManager.h>

namespace android {
namespace NSPostProc {
enum class ThirdPartyType
{
    NONE,
    FULL,
    //SINGLE_3RD_FR_W_JPEG,
    //SINGLE_3RD_FR_WO_JPEG,
    SINGLE_3RD_F_W_JPEG,
    SINGLE_3RD_F_WO_JPEG,
    DUAL_3RD_FR_WO_JPEG,
    DUAL_3RD_R_W_JPEG,
    DUAL_3RD_FR_W_JPEG,
    //DUAL_3RD_F_W_JPEG,
    //DUAL_3RD_F_WO_JPEG,
};
class FrameDataTableSet
{
public:
    FrameDataTableSet() = delete;
    FrameDataTableSet(
        std::string name,
        ThirdPartyType type,
        MetaStreamManager::metadata_info_setting* metadata,
        ImageStreamManager::image_stream_info_pool_setting* image,
        MINT32* node
    )
    {
        this->name = name;
        this->type = type;
        this->metadata = metadata;
        this->image = image;
        this->node = node;
    }
    std::string name;
    ThirdPartyType type = ThirdPartyType::NONE;
    MetaStreamManager::metadata_info_setting* metadata = nullptr;
    ImageStreamManager::image_stream_info_pool_setting* image = nullptr;
    MINT32* node = nullptr;
};
extern std::vector<FrameDataTableSet> gFrameDataTableSet;
};
};
#endif // _MTK_PLATFORM_HARDWARE_INCLUDE_LAGACYPIPELINE_PP_TABLE_DATA_SET_H_