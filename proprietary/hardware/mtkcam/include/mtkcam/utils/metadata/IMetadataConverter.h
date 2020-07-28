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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_METADATA_IMETADATACONVERTER_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_METADATA_IMETADATACONVERTER_H_
//
#include <utils/RefBase.h>
#include "IMetadata.h"
#include "IMetadataTagSet.h"

struct camera_metadata;

namespace NSCam {

/******************************************************************************
 *
 ******************************************************************************/
class IMetadataConverter : public virtual android::RefBase
{
public:
    static  android::sp<IMetadataConverter>   createInstance(IMetadataTagSet const &pTagInfo);

    virtual MBOOL                   convertFromHidl(const void* src/*const ::android::hardware::hidl_vec<uint8_t>* */, IMetadata& dst) = 0;

    // processCaptureResult() does not use this api, since the newly-allocated
    // camera_metadata structure will not be freed automatically.
    virtual MBOOL                   convertToHidl(const IMetadata& src, void* dst/*::android::hardware::hidl_vec<uint8_t>* */) = 0;

    virtual MBOOL                   convert(const camera_metadata* src, IMetadata& dst) const = 0;
    virtual MBOOL                   convert(const IMetadata& src, camera_metadata*& dst, size_t* pDstSize = nullptr) const = 0;
#ifdef MTKCAM_METADATA_V1
    virtual MBOOL                   convertWithoutAllocate(const IMetadata& src, camera_metadata*& dst) const = 0;
    virtual android::status_t       get_data_count(IMetadata const &src, size_t &entryCount, size_t &dataCount) const = 0;
#endif
    virtual size_t                  getCameraMetadataSize(const camera_metadata *metadata) const = 0;
    virtual void                    freeCameraMetadata(camera_metadata *metadata) const = 0;

    virtual MVOID                   dump(const IMetadata &rMetadata, int frameNo = -1) const = 0;
    virtual MVOID                   dumpAll(const IMetadata &rMetadata, int frameNo = -1) const = 0;
};


/******************************************************************************
 *
 ******************************************************************************/
};  //  NSCam
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_METADATA_IMETADATACONVERTER_H_

