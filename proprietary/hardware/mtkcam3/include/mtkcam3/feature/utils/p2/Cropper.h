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

#ifndef _MTKCAM_FEATURE_UTILS_P2_CROPPER_H_
#define _MTKCAM_FEATURE_UTILS_P2_CROPPER_H_

#include <mtkcam/drv/def/IPostProcDef.h>

#include <mtkcam3/feature/utils/log/ILogger.h>
#include <mtkcam3/feature/utils/p2/LMVInfo.h>

namespace NSCam {
namespace Feature {
namespace P2Util {

class Cropper : virtual public android::RefBase
{
public:
    enum CropMask
    {
        USE_RESIZED     = 0x01,
        USE_EIS_12      = 0x02,
        USE_CROP_RATIO  = 0x04,
    };

    Cropper() {}
    virtual ~Cropper() {}

    virtual MBOOL isValid() const = 0;
    virtual MBOOL isEISAppOn() const = 0;
    virtual MSize getSensorSize() const = 0;
    virtual MRect getResizedCrop() const = 0;
    virtual MRect getP1Crop() const = 0;
    virtual MSize getP1OutSize() const = 0;
    virtual MRect getP1BinCrop() const = 0;
    virtual MSize getP1BinSize() const = 0;
    virtual const LMVInfo& getLMVInfo() const = 0;

    virtual NSIoPipe::MCropRect calcViewAngle(const ILog &log, const MSize &size, MUINT32 cropFlag) const = 0;
    virtual NSIoPipe::MCropRect calcViewAngle(const ILog &log, const MSize &size, MUINT32 cropFlag, const float cropRatio) const = 0;
    virtual MRectF calcViewAngleF(const ILog &log, const MSize &size, MUINT32 cropFlag, const float widthCropRatio, const float heightCropRatio, const MUINT32 dmaConstrainFlag) const = 0;
    virtual MBOOL refineBoundary(const ILog &log, const MSize &imgSize, NSIoPipe::MCropRect &crop) const = 0;

    virtual MRect getCropRegion() const = 0;
    virtual MRect getActiveCrop() const = 0;
    virtual MRect toActive(const NSIoPipe::MCropRect &cropRect, MBOOL resize) const = 0;
    virtual MRect toActive(const MRectF &cropF, MBOOL resize) const = 0;
    virtual MRect toActive(const MRect &crop, MBOOL resize) const = 0;
    virtual MVOID dump(const ILog &log) const = 0;
};

} // namespace P2Util
} // namespace Feature
} // namespace NSCam

#endif // _MTKCAM_FEATURE_UTILS_P2_CROPPER_H_
