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

#include <gtest/gtest.h>

#include <cstdlib>
#include <iostream>

#include "DebugControl.h"
#define PIPE_TRACE TRACE_PURE_NATIVEBUFFERWRAPPER
#define PIPE_CLASS_TAG "Suite_NativeBufferWrapper_Test"
#include <featurePipe/core/include/PipeLog.h>

#include <featurePipe/core/include/GraphicBufferPool.h>
#include <featurePipe/core/include/FatImageBufferPool.h>
#include "../StreamingFeature_Common.h"
#include "../GPUWarp.h"
#include "TestUtil.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_STREAMING);

#define NUM_POOL      8

namespace NSCam{
namespace NSCamFeature{
namespace NSFeaturePipe{

android::sp<GraphicBufferPool> createInputPool(const MSize &size, unsigned count)
{
    android::sp<GraphicBufferPool> pool;
    pool = GraphicBufferPool::create("input", size.w, size.h, HAL_PIXEL_FORMAT_YV12, GraphicBufferPool::USAGE_HW_TEXTURE);
    pool->allocate(count);
    return pool;
}

android::sp<GraphicBufferPool> createRecordPool(const MSize &size, unsigned count)
{
    android::sp<GraphicBufferPool> pool;
    pool = GraphicBufferPool::create("record", size.w, size.h, HAL_PIXEL_FORMAT_YV12, GraphicBufferPool::USAGE_HW_TEXTURE);
    pool->allocate(count);
    return pool;
}

android::sp<FatImageBufferPool> createWarpPool(const MSize &size, unsigned count)
{
    android::sp<FatImageBufferPool> pool;
    pool = FatImageBufferPool::create("warp", size, eImgFmt_WARP_2PLANE, FatImageBufferPool::USAGE_HW_AND_SW);
    pool->allocate(count);

    return pool;
}

void DestoryGraphicBufferPool(android::sp<GraphicBufferPool>& pool)
{
    GraphicBufferPool::destroy(pool);
    pool = NULL;
}

void DestoryFatImageBufferPool(android::sp<FatImageBufferPool>& pool)
{
    FatImageBufferPool::destroy(pool);
    pool = NULL;
}

TEST(PureBufferWrapper, Simple)
{
    TRACE_FUNC_ENTER();
    MSize streamSize = multiply(FHD_SIZE, 1.25);
    MSize recordSize = FHD_SIZE;

    android::sp<NativeBufferWrapper> mFullImage[NUM_POOL];
    android::sp<NativeBufferWrapper> mWarpOut[NUM_POOL];
    // Allocate
    for( MUINT32 i = 0; i < NUM_POOL; i++ )
    {
        mFullImage[i] = new NativeBufferWrapper(streamSize.w, streamSize.h,
                                                HAL_PIXEL_FORMAT_YV12, NativeBufferWrapper::USAGE_HW_TEXTURE,
                                                "fullImg" + std::to_string(i));
    }
    for( MUINT32 i = 0; i < NUM_POOL; i++ )
    {
        mWarpOut[i] = new NativeBufferWrapper(recordSize.w, recordSize.h,
                                              HAL_PIXEL_FORMAT_YV12, NativeBufferWrapper::USAGE_HW_RENDER,
                                              "warpout" + std::to_string(i));
    }

    for( MUINT32 i = 0; i < NUM_POOL; i++ )
    {
        MUINT32* virAddr;
        NB_SPTR sptr = mFullImage[i]->getBuffer_SPTR();
        MY_LOGD("Lock and unlock NativeBuffer(%p)", sptr);
        ASSERT_TRUE( mFullImage[i]->lock(NativeBufferWrapper::USAGE_SW, (void**)(&virAddr)) );
        ASSERT_TRUE( mFullImage[i]->unlock() );
    }
    for( MUINT32 i = 0; i < NUM_POOL; i++ )
    {
        MUINT32* virAddr;
        NB_SPTR sptr = mWarpOut[i]->getBuffer_SPTR();
        MY_LOGD("Lock and unlock NativeBuffer(%p)", sptr);
        ASSERT_TRUE( mWarpOut[i]->lock(NativeBufferWrapper::USAGE_SW, (void**)(&virAddr)) );
        ASSERT_TRUE( mWarpOut[i]->unlock() );
    }

    for( MUINT32 i = 0; i < NUM_POOL; i++ )
    {
        mFullImage[i] = NULL;
    }
    for( MUINT32 i = 0; i < NUM_POOL; i++ )
    {
        mWarpOut[i] = NULL;
    }

    TRACE_FUNC_EXIT();
}

TEST(PureBufferWrapper, GraphicBufferPool)
{
    TRACE_FUNC_ENTER();
    MSize streamSize = multiply(FHD_SIZE, 1.25);
    MSize warpSize = MSize(2, 2);

    // Create buffer pool
    android::sp<GraphicBufferPool> inputPool;
    android::sp<GraphicBufferPool> recordPool;
    android::sp<FatImageBufferPool> warpPool;
    inputPool = createInputPool(streamSize, NUM_POOL);
    recordPool = createRecordPool(streamSize, NUM_POOL);
    warpPool = createWarpPool(warpSize, NUM_POOL);

    // Warp
    GPUWarp gpuWarp;
    ASSERT_TRUE( gpuWarp.init(inputPool->getImageSize(), MAX_WARP_SIZE) );

    for( int i = 0; i < 10; ++i )
    {
        gpuWarp.processWarp(inputPool->requestIIBuffer(), recordPool->requestIIBuffer(), warpPool->requestIIBuffer(),
                            inputPool->getImageSize(), recordPool->getImageSize());
    }

    gpuWarp.uninit();

    // Destory buffer pool
    GraphicBufferPool::destroy(inputPool);
    GraphicBufferPool::destroy(recordPool);
    FatImageBufferPool::destroy(warpPool);
    inputPool = NULL;
    recordPool = NULL;
    warpPool = NULL;

    TRACE_FUNC_EXIT();
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
