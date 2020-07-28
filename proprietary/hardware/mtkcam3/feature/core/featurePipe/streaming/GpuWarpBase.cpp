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

#include "GpuWarpBase.h"

#include "DebugControl.h"
#define PIPE_CLASS_TAG "GpuWarpBase"
#define PIPE_TRACE TRACE_GPU_WARP
#include <featurePipe/core/include/PipeLog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_STREAMING);

using android::Mutex;
using android::sp;

namespace NSCam {
namespace NSCamFeature {

GpuWarpBase::GpuWarpBase()
    : mStage(STAGE_IDLE)
    , mRegisterMode(REGISTER_MODE_RUNTIME)
    , mFeature(0)
    , mMaxImage(0, 0)
    , mMaxWarp(0, 0)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

GpuWarpBase::~GpuWarpBase()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

MBOOL GpuWarpBase::init(RegisterMode registerMode, const MSize &maxImageSize, const MSize &maxWarpSize, MUINT32 feature)
{
    TRACE_FUNC_ENTER();
    Mutex::Autolock lock(mMutex);

    if( mStage != STAGE_IDLE )
    {
        MY_LOGE("Invalid state(%d)", mStage);
    }
    else if( registerMode != REGISTER_MODE_INIT &&
             registerMode != REGISTER_MODE_RUNTIME )
    {
        MY_LOGE("Invalid register mode = %d", registerMode);
    }
    else if( !(maxImageSize.w && maxImageSize.h &&
               maxWarpSize.w && maxWarpSize.h) )
    {
        MY_LOGE("Invalid config: feature=0x%x maxImage=(%d,%d), maxWarp=(%d,%d)", feature, maxImageSize.w, maxImageSize.h, maxWarpSize.w, maxWarpSize.h);
    }
    else
    {
        mRegisterMode = registerMode;
        mMaxImage = maxImageSize;
        mMaxWarp = maxWarpSize;
        mStage = STAGE_INIT;
    }
    TRACE_FUNC_EXIT();
    return (mStage == STAGE_INIT);
}

MBOOL GpuWarpBase::uninit()
{
    TRACE_FUNC_ENTER();
    Mutex::Autolock lock(mMutex);
    if( mStage == STAGE_CONFIG )
    {
        updateConfig(DEFAULT_FEATURE);
        this->onReset();
    }
    if( mStage == STAGE_INIT || mStage == STAGE_CONFIG )
    {
        mFeature = 0;
        mMaxImage.w = mMaxImage.h = 0;
        mMaxWarp.w = mMaxWarp.h = 0;
        mStage = STAGE_IDLE;
    }
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL GpuWarpBase::config(const NB_SPTR_ARRAY &in, const NB_SPTR_ARRAY &out, MUINT32 feature)
{
    TRACE_FUNC_ENTER();
    Mutex::Autolock lock(mMutex);
    if( mStage != STAGE_INIT && mStage != STAGE_CONFIG )
    {
        MY_LOGW("Invalid state(%d)", mStage);
    }
    else if( needConfig(in, out, feature) )
    {
        this->updateConfig(feature);
        if( this->onConfig(mFeature, in, out, mMaxImage, mMaxWarp, mRegisterMode) )
        {
            if( mRegisterMode == REGISTER_MODE_RUNTIME )
            {
                this->registerGB(in, mInGBSet, NB_DIR_IN);
                this->registerGB(out, mOutGBSet, NB_DIR_OUT);
            }
            mStage = STAGE_CONFIG;
        }
        else
        {
            updateConfig(DEFAULT_FEATURE);
            mStage = STAGE_INIT;
        }
    }
    TRACE_FUNC_EXIT();
    return (mStage == STAGE_CONFIG);
}

MBOOL GpuWarpBase::reset()
{
    TRACE_FUNC_ENTER();
    Mutex::Autolock lock(mMutex);
    if( mStage == STAGE_CONFIG )
    {
        updateConfig(DEFAULT_FEATURE);
        this->onReset();
        mStage = STAGE_INIT;
    }
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL GpuWarpBase::needConfig(const NB_SPTR_ARRAY &in, const NB_SPTR_ARRAY &out, MUINT32 feature)
{
    TRACE_FUNC_ENTER();
    MBOOL changed = MFALSE;
    changed = changed || (mFeature != feature);
    changed = changed || !isSubset(mInGBSet, in);
    changed = changed || !isSubset(mOutGBSet, out);
    TRACE_FUNC_EXIT();
    return changed;
}

MVOID GpuWarpBase::updateConfig(MUINT32 feature)
{
    TRACE_FUNC_ENTER();
    mFeature = feature;
    mInGBSet.clear();
    mOutGBSet.clear();
    TRACE_FUNC_EXIT();
}

MBOOL GpuWarpBase::isSubset(const NB_SET &whole, const NB_SPTR_ARRAY &part)
{
    TRACE_FUNC_ENTER();
    MBOOL subset = MTRUE;
    for( NB_SPTR_ARRAY::const_iterator it = part.begin(), end = part.end(); it != end; ++it )
    {
        if( getNativeBuffer(it) )
        {
            if( whole.count(getNativeBuffer(it)) == 0 )
            {
                subset = MFALSE;
                break;
            }
        }
    }
    TRACE_FUNC_EXIT();
    return subset;
}

MBOOL GpuWarpBase::isSubset(const NB_SET &whole, const NB_SPTR part)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    if( part && whole.count(getNativeBuffer(part)) )
    {
        ret = MTRUE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL GpuWarpBase::process(NB_SPTR in, NB_SPTR out, IImageBuffer *warpMap, const MSize &inSize, const MSize &outSize, MBOOL passThrough)
{
    TRACE_FUNC_ENTER();
    Mutex::Autolock lock(mMutex);
    MBOOL ret = MFALSE;
    if( mStage == STAGE_CONFIG )
    {
        if( checkProcessParam(in, out, warpMap, inSize, outSize) )
        {
            ret = this->onProcess(in, out, warpMap, inSize, outSize, passThrough);
        }
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL GpuWarpBase::isGBRegister(NB_SPTR in, NB_SPTR out)
{
    TRACE_FUNC_ENTER();
    Mutex::Autolock lock(mMutex);
    MBOOL ret = MFALSE;

    if( mStage == STAGE_CONFIG )
    {
        ret = registerGB(in, mInGBSet, NB_DIR_IN) &&
              registerGB(out, mOutGBSet, NB_DIR_OUT);
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MUINT32 GpuWarpBase::getDefaultFeature()
{
    TRACE_FUNC_ENTER();
    MUINT32 feature = DEFAULT_FEATURE;
    TRACE_FUNC_EXIT();
    return feature;
}

MUINT32 GpuWarpBase::toggleVFB(MUINT32 ori, MBOOL value)
{
    TRACE_FUNC_ENTER();
    if( value )
    {
        ori |= FEATURE_VFB;
    }
    else
    {
        ori &= ~FEATURE_VFB;
    }
    TRACE_FUNC_EXIT();
    return ori;
}

MBOOL GpuWarpBase::hasVFB(MUINT32 feature)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = feature & FEATURE_VFB;
    TRACE_FUNC_EXIT();
    return ret;
}

MUINT32 GpuWarpBase::toggleEIS(MUINT32 ori, MBOOL value)
{
    TRACE_FUNC_ENTER();
    if( value )
    {
        ori |= FEATURE_EIS2;
    }
    else
    {
        ori &= ~FEATURE_EIS2;
    }
    TRACE_FUNC_EXIT();
    return ori;
}

MBOOL GpuWarpBase::hasEIS(MUINT32 feature)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = feature & FEATURE_EIS2;
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL GpuWarpBase::makePassThroughWarp(const sp<IImageBuffer> &buffer, const MSize &grid, MSize size)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    if( buffer == NULL )
    {
        MY_LOGW("Invalid buffer!");
    }
    else if( grid.w < 1 || grid.h < 1 ||
             buffer->getImgSize().w < grid.w || buffer->getImgSize().h < grid.h )
    {
        MY_LOGW("Invalid Grid Size W(%d)xH(%d), Buffer Size W(%d)xH(%d)",
                grid.w, grid.h, buffer->getImgSize().w, buffer->getImgSize().h);
    }
    else
    {
        char *va = NULL;
        MUINT32 *ptr = NULL;
        va = (char*)buffer->getBufVA(0);
        ptr = (MUINT32*)va;
        if( va && ptr )
        {
            // x
            double x_step = 16.0 * (size.w-1)/(grid.w-1);
            for( MUINT32 h = 0; h < grid.h; ++h)
            {
                for( MUINT32 w = 0; w < grid.w; ++w)
                {
                    ptr[w+grid.w*h] = w*x_step;
                }
            }
            va = (char*)buffer->getBufVA(1);
            ptr = (MUINT32*)va;
            // y
            double y_step = 16.0 * (size.h-1)/(grid.h-1);
            for( MUINT32 h = 0; h < grid.h; ++h)
            {
                for( MUINT32 w = 0; w < grid.w; ++w)
                {
                    ptr[w+grid.w*h] = h*y_step;
                }
            }
            ret = MTRUE;
        }
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL GpuWarpBase::checkProcessParam(NB_SPTR in, NB_SPTR out, IImageBuffer *warpMap, const MSize &inSize, const MSize &outSize)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    if( in == NULL )
    {
        MY_LOGE("invalid input buffer handle addr");
    }
    else if( getNativeBuffer(in) == NULL )
    {
        MY_LOGE("invalid input buffer handle value");
    }
    else if( out == NULL )
    {
        MY_LOGE("invalid output buffer handle addr");
    }
    else if( getNativeBuffer(out) == NULL )
    {
        MY_LOGE("invalid output buffer handle value");
    }
    else if( warpMap == NULL )
    {
        MY_LOGE("invalid warpMap handle");
    }
    else if( inSize.w <= 0 || inSize.h <= 0 ||
             inSize.w > mMaxImage.w || inSize.h > mMaxImage.h )
    {
        MY_LOGE("invalid input size");
    }
    else if( outSize.w <= 0 || outSize.h <= 0 ||
             outSize.w > mMaxImage.w || outSize.h > mMaxImage.h )
    {
        MY_LOGE("invalid output size");
    }
    else
    {
        ret = MTRUE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL GpuWarpBase::registerGB(NB_SPTR gb, NB_SET &set, NBDir dir)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;

    if( gb && getNativeBufferPtr(gb) && !isSubset(set, gb) )
    {
        if( set.size() >= this->getNumRegisterSlot() )
        {
            ret = MFALSE;
        }
        else
        {
            ret = this->onRegisterGB(gb, dir);
            if( ret )
            {
                set.insert(getNativeBuffer(gb));
            }
        }
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MVOID GpuWarpBase::registerGB(const NB_SPTR_ARRAY &gbs, NB_SET &set, NBDir dir)
{
    TRACE_FUNC_ENTER();
    for( NB_SPTR_ARRAY::const_iterator it = gbs.begin(), end = gbs.end(); it != end; ++it )
    {
        registerGB(*it, set, dir);
    }
    TRACE_FUNC_EXIT();
}

} // namespace NSCamFeature
} // namespace NSCam
