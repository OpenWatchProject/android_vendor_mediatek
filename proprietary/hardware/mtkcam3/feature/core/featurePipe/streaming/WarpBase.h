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

#ifndef _MTK_CAMERA_STREAMING_FEATURE_PIPE_WARP_BASE_H_
#define _MTK_CAMERA_STREAMING_FEATURE_PIPE_WARP_BASE_H_

#include "MtkHeader.h"

#include <utils/Mutex.h>

#include "StreamingFeatureData.h"


namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

class WarpBase
{
public:
    WarpBase();
    virtual ~WarpBase();
    MBOOL init(const MSize &maxImageSize, const MSize &maxWarpSize);
    MVOID uninit();
    MBOOL processWarp(const ImgBuffer &in, const ImgBuffer &out, const ImgBuffer &warpMap, const MSize &inSize, const MSize &outSize);

public:
    static MBOOL makePassThroughWarp(const ImgBuffer &warp, const MSize &inSize, const MSize &outSize);
    static MBOOL makePassThroughWarp(const android::sp<IImageBuffer> &buffer, const MSize &grid, const MSize &size);
    static MBOOL applyDZWarp(const ImgBuffer &warp, const MSize &inSize, const MSize &outSize, const MRectF &zoom, const MSizeF &marginPixel);

protected:
    virtual MBOOL onInit() = 0;
    virtual MVOID onUninit() = 0;
    virtual MBOOL onProcessWarp(const ImgBuffer &in, const ImgBuffer &out, const ImgBuffer &warpMap, const MSize &inSize, const MSize &outSize) = 0;

protected:
    MSize getMaxImageSize() const;
    MSize getMaxWarpSize() const;

private:
    enum Stage { STAGE_IDLE, STAGE_INIT };
    Stage mStage;
    android::Mutex mMutex;
    MSize mMaxImageSize;
    MSize mMaxWarpSize;
};

}; // namespace NSFeaturePipe
}; // namespace NSCamFeature
}; // namespace NSCam

#endif // _MTK_CAMERA_STREAMING_FEATURE_PIPE_WARP_BASE_H_
