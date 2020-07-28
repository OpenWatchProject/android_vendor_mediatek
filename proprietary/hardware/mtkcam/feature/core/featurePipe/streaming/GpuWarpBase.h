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

#ifndef _GPU_WARP_BASE_H_
#define _GPU_WARP_BASE_H_

#include "MtkHeader.h"
//#include <mtkcam/common.h>

#include <ui/GraphicBuffer.h>

#include <utils/RefBase.h>
#include <utils/Mutex.h>

#include <vector>
#include <set>

#include <featurePipe/core/include/Timer.h>

#define TRACE_GPU_WARP_TIME 0

#if TRACE_GPU_WARP_TIME
#define TRACE_GPU_START() NSCam::NSCamFeature::NSFeaturePipe::Timer gpuTraceTimer(true);
#define TRACE_GPU_STOP(str) MY_LOGD("GPUTIMER: %s used %d ms", str, gpuTraceTimer.getNow());
#else
#define TRACE_GPU_START()
#define TRACE_GPU_STOP(str)
#endif

namespace NSCam {
namespace NSCamFeature {

class GpuWarpBase : public virtual android::RefBase
{
public:
    enum RegisterMode { REGISTER_MODE_INIT, REGISTER_MODE_RUNTIME };
    enum NBDir { NB_DIR_IN, NB_DIR_OUT };
    enum Feature {
                    USE_CPU = (1<<0),
                    FEATURE_VFB = (1<<1),
                    FEATURE_EIS2 = (1<<2),
                    DEFAULT_FEATURE = 0,
                 };
    typedef std::vector<NB_SPTR> NB_SPTR_ARRAY;
    typedef std::set<NativeBufferWrapper::NativeBuffer*> NB_SET;

    GpuWarpBase();
    virtual ~GpuWarpBase();

    MBOOL init(RegisterMode mode, const MSize &maxImageSize, const MSize &maxWarpSize, MUINT32 feature = 0);
    MBOOL uninit();
    MBOOL config(const NB_SPTR_ARRAY &in, const NB_SPTR_ARRAY &out, MUINT32 feature = DEFAULT_FEATURE);
    MBOOL reset();
    MBOOL process(NB_SPTR in, NB_SPTR out, IImageBuffer *warpMap, const MSize &inSize, const MSize &outSize, MBOOL passThrough = MFALSE);
    MBOOL isGBRegister(NB_SPTR in, NB_SPTR out);

    static MUINT32 getDefaultFeature();
    static MUINT32 toggleVFB(MUINT32 ori, MBOOL value);
    static MBOOL hasVFB(MUINT32 feature);
    static MUINT32 toggleEIS(MUINT32 ori, MBOOL value);
    static MBOOL hasEIS(MUINT32 feature);

    static MBOOL makePassThroughWarp(const android::sp<IImageBuffer> &buffer, const MSize &grid, MSize size);
    virtual MUINT32 getNumRegisterSlot() const = 0;

protected:
    virtual MBOOL onConfig(MUINT32 feature, const NB_SPTR_ARRAY &in, const NB_SPTR_ARRAY &out, const MSize &maxImage, const MSize &maxWarp, RegisterMode registerMode) = 0;
    virtual MBOOL onReset() = 0;
    virtual MBOOL onProcess(NB_SPTR in, NB_SPTR out, IImageBuffer *warpMap, const MSize &inSize, const MSize &outSize, MBOOL passThrough) = 0;
    virtual MBOOL onRegisterGB(NB_SPTR gb, NBDir dir) = 0;

private:
    MBOOL needConfig(const NB_SPTR_ARRAY &in, const NB_SPTR_ARRAY &out, MUINT32 feature);
    MVOID updateConfig(MUINT32 feature);
    MBOOL isSubset(const NB_SET &whole, const NB_SPTR_ARRAY &part);
    MBOOL isSubset(const NB_SET &whole, const NB_SPTR part);
    MBOOL checkProcessParam(NB_SPTR in, NB_SPTR out, IImageBuffer *warpMap, const MSize &inputSize, const MSize &outputSize);
    MBOOL registerGB(NB_SPTR gb, NB_SET &map, NBDir dir);
    MVOID registerGB(const NB_SPTR_ARRAY &gbs, NB_SET &map, NBDir dir);

private:
    enum Stage { STAGE_IDLE, STAGE_INIT, STAGE_CONFIG };
    Stage mStage;
    android::Mutex mMutex;
    android::Condition mCondition;

    RegisterMode mRegisterMode;
    MUINT32 mFeature;
    MSize mMaxImage;
    MSize mMaxWarp;
    NB_SET mInGBSet;
    NB_SET mOutGBSet;
};

}; // namespace NSCamFeature
}; // namespace NSCam

#endif // _GPU_WARP_BASE_H_
