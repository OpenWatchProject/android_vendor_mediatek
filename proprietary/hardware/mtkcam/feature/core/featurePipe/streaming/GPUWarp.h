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

#ifndef _MTK_CAMERA_STREAMING_FEATURE_PIPE_GPU_WARP_H_
#define _MTK_CAMERA_STREAMING_FEATURE_PIPE_GPU_WARP_H_

#include "WarpBase.h"

#include <set>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

class GPUWarp : public virtual WarpBase
{
public:
    GPUWarp();
    virtual ~GPUWarp();

protected:
    virtual MBOOL onInit();
    virtual MVOID onUninit();
    virtual MBOOL onProcessWarp(const ImgBuffer &in, const ImgBuffer &out, const ImgBuffer &warpMap, const MSize &inSize, const MSize &outSize);

private:
    enum NB_DIR { NB_DIR_IN, NB_DIR_OUT };
    typedef std::set<NativeBuffer*> NB_SET;

    MBOOL initWarp();
    MVOID configInitWarpInfo(struct WarpImageExtInfo *warpInfo, GpuTuning *gpuTuning);
    MVOID configBasicWarpInfo(struct WarpImageExtInfo *warpInfo, GpuTuning *gpuTuning);
    MBOOL createWarpObj();
    MBOOL initWarpObj(struct WarpImageExtInfo &warpInfo);
    MBOOL initWorkBuffer(struct WarpImageExtInfo &warpInfo);
    MVOID cleanUp();
    MVOID uninitWarp();
    MVOID uninitWorkBuffer();
    MBOOL prepareGB(const ImgBuffer &in, const ImgBuffer &out, NB_SPTR &inGB, NB_SPTR &outGB);
    MBOOL registerGB(NB_SPTR in, NB_SPTR out);
    MBOOL registerGB(NB_SPTR gb, NB_SET &set, NB_DIR dir);
    MBOOL prepareWarp(const ImgBuffer &warpMap, IImageBuffer* &warpBuffer);
    MVOID configWarpMapInfo(struct WarpImageExtInfo &warpInfo, IImageBuffer *warpBuffer);
    MBOOL doWarp(struct WarpImageExtInfo &warpInfo);

private:
    MTKWarpExt *mpGpuWarp;
    MUINT32 mWorkBufSize;
    MUINT8 *mWorkBuf;

    NB_SET mInSet;
    NB_SET mOutSet;

    MBOOL  mForceUseRGBA;
};

}; // namespace NSFeaturePipe
}; // namespace NSCamFeature
}; // namespace NSCam

#endif // _MTK_CAMERA_STREAMING_FEATURE_PIPE_GPU_WARP_H_
