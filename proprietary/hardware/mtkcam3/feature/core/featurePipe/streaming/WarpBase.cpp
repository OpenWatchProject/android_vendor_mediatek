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

#include "WarpBase.h"

#include "DebugControl.h"
#define PIPE_CLASS_TAG "WarpBase"
#define PIPE_TRACE TRACE_WARP_BASE
#include <featurePipe/core/include/PipeLog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_STREAMING);


template <typename T>
const T WARP_MAX(const T& a, const T& b)
{
    return a > b ? a : b;
}

template <typename T>
const T WARP_MIN(const T& a, const T& b)
{
    return a < b ? a : b;
}

template <typename T>
const T WARP_FLOOR(const T& x)
{
    return (int)x - ((int)x>x);
}


namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

WarpBase::WarpBase()
    : mStage(STAGE_IDLE)
    , mMaxImageSize(0, 0)
    , mMaxWarpSize(0, 0)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

WarpBase::~WarpBase()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

MBOOL WarpBase::init(const MSize &maxImageSize, const MSize &maxWarpSize)
{
    TRACE_FUNC_ENTER();
    android::Mutex::Autolock lock(mMutex);

    if( mStage != STAGE_IDLE )
    {
        MY_LOGE("Invalid state(%d)", mStage);
    }
    else if( !(maxImageSize.w && maxImageSize.h &&
               maxWarpSize.w && maxWarpSize.h) )
    {
        MY_LOGE("Invalid setting: maxImageSize=(%d,%d), maxWarpSize=(%d,%d)", maxImageSize.w, maxImageSize.h, maxWarpSize.w, maxWarpSize.h);
    }
    else
    {
        mMaxImageSize = maxImageSize;
        mMaxWarpSize = maxWarpSize;
        this->onInit();
        mStage = STAGE_INIT;
    }
    TRACE_FUNC_EXIT();
    return (mStage == STAGE_INIT);
}

MVOID WarpBase::uninit()
{
    TRACE_FUNC_ENTER();
    android::Mutex::Autolock lock(mMutex);
    if( mStage == STAGE_INIT )
    {
        mMaxImageSize.w = mMaxImageSize.h = 0;
        mMaxWarpSize.w = mMaxWarpSize.h = 0;
        this->onUninit();
        mStage = STAGE_IDLE;
    }
    TRACE_FUNC_EXIT();
}

MBOOL WarpBase::processWarp(const ImgBuffer &in, const ImgBuffer &out, const ImgBuffer &warpMap, const MSize &inSize, const MSize &outSize)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    android::Mutex::Autolock lock(mMutex);
    if( mStage == STAGE_INIT )
    {
        ret = this->onProcessWarp(in, out, warpMap, inSize, outSize);
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MSize WarpBase::getMaxImageSize() const
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return mMaxImageSize;
}

MSize WarpBase::getMaxWarpSize() const
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return mMaxWarpSize;
}

MBOOL WarpBase::makePassThroughWarp(const ImgBuffer &warp, const MSize &inSize, const MSize &outSize)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    const MUINT32 PRECISION = 5;
    double e = (1<<PRECISION);
    if( warp == NULL )
    {
        MY_LOGW("Invalid buffer");
    }
    else
    {
        MSize grid = warp->getImageBufferPtr()->getImgSize();
        MUINT32 *ptrX = (MUINT32*)warp->getImageBufferPtr()->getBufVA(0);
        MUINT32 *ptrY = (MUINT32*)warp->getImageBufferPtr()->getBufVA(1);
        if( ptrX && ptrY && grid.w > 1 && grid.h > 1 )
        {
            double x_off = e * (inSize.w-outSize.w)/2;
            double y_off = e * (inSize.h-outSize.h)/2;
            double x_step = e * (outSize.w-1)/(grid.w-1);
            double y_step = e * (outSize.h-1)/(grid.h-1);

            for( MUINT32 h = 0; h < (MUINT32)grid.h; ++h )
            {
                for( MUINT32 w = 0; w < (MUINT32)grid.w; ++w )
                {
                    MUINT32 pos = grid.w * h + w;
                    ptrX[pos] = x_off + w*x_step;
                    ptrY[pos] = y_off + h*y_step;
                }
            }
            ret = MTRUE;
        }
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL WarpBase::makePassThroughWarp(const sp<IImageBuffer> &buffer, const MSize &grid, const MSize &size)
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
            double x_step = 32.0 * (size.w-1)/(grid.w-1);
            for( MINT32 h = 0; h < grid.h; ++h)
            {
                for( MINT32 w = 0; w < grid.w; ++w)
                {
                    ptr[w+grid.w*h] = w*x_step;
                }
            }
            va = (char*)buffer->getBufVA(1);
            ptr = (MUINT32*)va;
            // y
            double y_step = 32.0 * (size.h-1)/(grid.h-1);
            for( MINT32 h = 0; h < grid.h; ++h)
            {
                for( MINT32 w = 0; w < grid.w; ++w)
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

MBOOL WarpBase::applyDZWarp(const ImgBuffer &warp, const MSize &inSize, const MSize &outSize, const MRectF &zoom, const MSizeF &marginPixel)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;

    // Calculate DZwarp parameters
    MSize grid(warp->getImageBuffer()->getImgSize().w, warp->getImageBuffer()->getImgSize().h);
    MSizeF src(inSize.w - marginPixel.w*2, inSize.h - marginPixel.h*2);
    MSizeF offset = MSizeF(zoom.p.x, zoom.p.y) - marginPixel;
    MRectF zoomCrop(zoom);

    // Scale up if source size < out size
    if( src.w < outSize.w )
    {
        zoomCrop.s.w = zoomCrop.s.w * outSize.w / src.w;
    }
    if( src.h < outSize.h )
    {
        zoomCrop.s.h = zoomCrop.s.h * outSize.h / src.h;
    }

    // Simple checking
    if( marginPixel.w < 0 || marginPixel.h < 0 )
    {
        MY_LOGW("Invalid marginPixel=(%fx%f)!", marginPixel.w, marginPixel.h);
    }
    else if( grid.w < 1 || grid.h < 1 )
    {
        MY_LOGW("Invalid Grid Size! GridW=%d, GridH=%d", grid.w, grid.h);
    }
    else
    {
        // Re-calculate warp matrix
        MINT32 *GridX = (MINT32 *) (warp->getImageBuffer())->getBufVA(0);
        MINT32 *GridY = (MINT32 *) (warp->getImageBuffer())->getBufVA(1);

        MFLOAT dz_new_x[grid.w*grid.h];
        MFLOAT dz_new_y[grid.w*grid.h];
        memset(&dz_new_x, 0, sizeof(dz_new_x));
        memset(&dz_new_y, 0, sizeof(dz_new_y));

        float dz_x_step = (float)(zoomCrop.s.w-1)/WARP_MAX((MUINT32)(grid.w-1), (MUINT32)1);
        float dz_y_step = (float)(zoomCrop.s.h-1)/WARP_MAX((MUINT32)(grid.h-1), (MUINT32)1);
        float src_x_step = (float)(src.w-1)/WARP_MAX((MUINT32)(grid.w-1), (MUINT32)1);
        float src_y_step = (float)(src.h-1)/WARP_MAX((MUINT32)(grid.h-1), (MUINT32)1);

        TRACE_FUNC("GridW=%d, GridH=%d, src_w=%d, src_h=%d, offset_x=%f, offset_y=%f, cropDZ_w=%f, cropDZ_h=%f",
                    grid.w, grid.h, src.w, src.h, offset.w, offset.h, zoomCrop.s.w, zoomCrop.s.h);

        for( MINT32 y = 0; y < grid.h; ++y )
        {
            MUINT32 img_y          = offset.h + y*dz_y_step;
            MUINT32 src_idx_y      = WARP_FLOOR(img_y/src_y_step);
            float y_alpha          = img_y - src_idx_y*src_y_step;
            float y_alpha_norm     = y_alpha / src_y_step;
            float y_alpha_inv_norm = (src_y_step - y_alpha) / src_y_step;
            MUINT32 grid_y         = src_idx_y*grid.w;
            MUINT32 grid_y_next    = WARP_MIN((src_idx_y+1), (MUINT32)(grid.h-1))*grid.w;
            MUINT32 new_grid_y     = y*grid.w;
            for( MINT32 x = 0; x < grid.w; ++x )
            {
                MUINT32 img_x          = offset.w + x*dz_x_step;
                MUINT32 src_idx_x      = WARP_FLOOR(img_x/src_x_step) ;
                MUINT32 src_idx_x_next = WARP_MIN(src_idx_x + 1, (MUINT32)(grid.w-1));
                float x_alpha          = img_x - src_idx_x*src_x_step;
                float x_alpha_norm     = x_alpha / src_x_step;
                float x_alpha_inv_norm = (src_x_step - x_alpha) / src_x_step;
                float dz_tmp0_x = GridX[grid_y + src_idx_x] * x_alpha_inv_norm +
                                  GridX[grid_y + src_idx_x_next] * x_alpha_norm;
                float dz_tmp0_y = GridY[grid_y + src_idx_x] * x_alpha_inv_norm +
                                  GridY[grid_y + src_idx_x_next] * x_alpha_norm;
                float dz_tmp1_x = GridX[grid_y_next + src_idx_x] * x_alpha_inv_norm +
                                  GridX[grid_y_next + src_idx_x_next] * x_alpha_norm;
                float dz_tmp1_y = GridY[grid_y_next + src_idx_x] * x_alpha_inv_norm +
                                  GridY[grid_y_next + src_idx_x_next] * x_alpha_norm;
                dz_new_x[new_grid_y+x] = dz_tmp0_x * y_alpha_inv_norm + dz_tmp1_x * y_alpha_norm;
                dz_new_y[new_grid_y+x] = dz_tmp0_y * y_alpha_inv_norm + dz_tmp1_y * y_alpha_norm;
            }
        }

        // Update to warp matrix
        for (MINT32 i = 0; i<grid.w*grid.h; ++i)
        {
            GridX[i] = dz_new_x[i];
            GridY[i] = dz_new_y[i];
            TRACE_FUNC("DZWarp X[%u]=%d, Y[%u]=%d", i, GridX[i], i, GridY[i]);
            ret = MTRUE;
        }
    }
    TRACE_FUNC_EXIT();
    return ret;
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
