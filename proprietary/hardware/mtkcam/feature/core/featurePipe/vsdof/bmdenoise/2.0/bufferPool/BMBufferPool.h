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
#ifndef _MTK_CAMERA_VSDOF_BMDENOISE_FEATURE_PIPE_BMBUFFER_POOL_H_
#define _MTK_CAMERA_VSDOF_BMDENOISE_FEATURE_PIPE_BMBUFFER_POOL_H_

#include "../BMDeNoisePipe_Common.h"
#include "../BMDeNoisePipeNode.h"
#include <featurePipe/core/include/GraphicBufferPool.h>

namespace NSCam{
namespace NSCamFeature{
namespace NSFeaturePipe{
namespace NSBMDN{

struct BufferConfig{
    const char* name;
    BMDeNoiseBufferID bufID;
    MUINT32 width;
    MUINT32 height;
    EImageFormat format;
    MUINT32 usage;
    MBOOL isSingle;
    MBOOL isGraphic;
    MUINT32 MaximumCount;
};

struct ImageBufferSet
{
    BufferConfig mBufConfig;
    android::sp<ImageBufferPool> mImagePool = nullptr;
    MUINT32 currentAllocateCount = 0;
};

struct GraphicBufferSet
{
    BufferConfig mBufConfig;
    android::sp<GraphicBufferPool> mGraphicPool = nullptr;
    MUINT32 currentAllocateCount = 0;
};

class BMBufferPool
{
    public:
        BMBufferPool(const char* name);
        virtual ~BMBufferPool();

        /**
         * @brief to setup buffer config.
         * @return
         *-true indicates ok; otherwise some error happened
         */
        MBOOL init(Vector<BufferConfig> bufConfig, MUINT32 tuningBufferCount = 0);

        /**
         * @brief to release buffer pools
         * @return
         *-true indicates ok; otherwise some error happened
         */
        MBOOL uninit();

        /**
         * @brief to do buffer allocation. Must call after init
         * @return
         *-true indicates the allocated buffer have not reach maximun yet; false indicates it has finished all buffer allocation
         */
        MBOOL doAllocate(MINT32 count = 1);

        /**
         * @brief to get the bufPool
         * @return
         *-the pointer indicates success; nullptr indidcated failed
         */
        android::sp<ImageBufferPool> getBufPool(BMDeNoiseBufferID bufID);

        /**
         * @brief to get the Graphic bufPool
         * @return
         *-the pointer indicates success; nullptr indidcated failed
         */
        android::sp<GraphicBufferPool> getGraphicBufPool(BMDeNoiseBufferID bufID);

        /**
         * @brief to get the tunning buffer pool
         * @return
         *-the pointer indicates success; nullptr indidcated failed
         */
        android::sp<TuningBufferPool> getTuningBufPool();

        // to check whether the buffers are ready to use;
        // it's a blocking call which returns after buffer ready
        MVOID checkReady(){/*TODO*/};

        /**
         * @brief to get the corresonding android_pixel_format from EImageFormat
         * @return
         *-the pointer indicates success; nullptr indidcated failed
         */
        static android::PixelFormat getPixelFormat(EImageFormat eFmt);

    private:
        mutable Mutex       mPoolLock;
        Condition           mCondPoolLock;
        const char*         mName;

        MBOOL               mbInint = MFALSE;
        MBOOL               mbReady = MFALSE;

        // Image buffers
        KeyedVector< BMDeNoiseBufferID, ImageBufferSet > mvImageBufSets;

        // Graphic buffers
        KeyedVector< BMDeNoiseBufferID, GraphicBufferSet > mvGraphicBufSets;

        // tuning buffers
        android::sp<TuningBufferPool>   mpTuningBufferPool;
};
};
};
};
};
#endif // _MTK_CAMERA_VSDOF_BMDENOISE_FEATURE_PIPE_BMBUFFER_POOL_H_