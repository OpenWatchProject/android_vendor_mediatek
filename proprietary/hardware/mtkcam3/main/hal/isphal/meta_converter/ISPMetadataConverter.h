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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#ifndef _MTK_HARDWARE_CAMERA_ISPHAL_ISPMETADATA_CONVERTER_H_
#define _MTK_HARDWARE_CAMERA_ISPHAL_ISPMETADATA_CONVERTER_H_
//
#include <utils/RefBase.h>
#include <mtkcam/utils/metadata/IMetadata.h>
//#include <isp_metadata/mtkisp_metadata.h>
#include <mtkisp_metadata.h>
#include <unordered_map>
#include <vendor/mediatek/hardware/camera/isphal/1.0/types.h>


namespace NSCam {
namespace ISPHal {

using namespace vendor::mediatek::hardware::camera::isphal::V1_0;


/******************************************************************************
 *
 ******************************************************************************/
class ISPMetadataConverter : public virtual android::RefBase
{
public:

                           ~ISPMetadataConverter();
                           ISPMetadataConverter();
    auto                   convertToIMetadata(const void* src, IMetadata& dst) -> MBOOL;
    auto                   convertToISPMetadata(const IMetadata& src, void*& dst, size_t* pDstSize = nullptr) -> MBOOL;
    auto                   freeISPMetaBuffer(void* buf) -> MBOOL;
private:
    std::unordered_map<uint32_t, uint32_t> mMaptoIMetaType;
    std::unordered_map<uint32_t, uint32_t> mMaptoISPType;
};


/******************************************************************************
 *
 ******************************************************************************/
};  // ISPHal
};  //  NSCam

#endif  //_MTK_HARDWARE_CAMERA_ISPHAL_ISPMETADATA_CONVERTER_H_

