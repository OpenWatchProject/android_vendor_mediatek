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
 * MediaTek Inc. (C) 2018. All rights reserved.
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
#ifndef _MTK_CAMERA_FEATURE_PIPE_WORKING_BUFFER_POOL_H_
#define _MTK_CAMERA_FEATURE_PIPE_WORKING_BUFFER_POOL_H_

#include <core/BufferPool.h>
#include <tuple>
#include <functional>

#include <core/GraphicBufferPool.h>

using namespace com::mediatek::campostalgo::NSBufferPool;

namespace com {
namespace mediatek {
namespace campostalgo {
namespace NSFeaturePipe {

struct BufferConfig
{
    const char* name;
    MUINT32 width;
    MUINT32 height;
    EImageFormat format;
    MUINT32 usage;
    MUINT32 minCount;
    MUINT32 maxCount;
};

android_pixel_format_t toPixelFormat(NSCam::EImageFormat fmt);

class WorkingBufferPool : public virtual android::RefBase
{
    public:
        WorkingBufferPool(const char* name);
        virtual ~WorkingBufferPool();

        /**
         * @brief to setup buffer config.
         * @return
         *-true indicates ok; otherwise some error happened
         */
        MBOOL init(Vector<BufferConfig> vBufConfig);

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
        MBOOL allocate();

        /**
         * @brief to get the bufPool
         * @return
         *-the pointer indicates success; nullptr indidcated failed
         */
        android::sp<IIBuffer> getImageBuffer(
            const MSize& size, MUINT32 format, MSize align = MSize(0,0), MBOOL bGraphicBuffer = MFALSE);


    private:
        typedef std::tuple<MUINT32, MUINT32, MUINT32> PoolKey_T;
        mutable Mutex       mPoolLock;
        Condition           mCondPoolLock;
        const char*         mName;
        MBOOL               mbInit;
        MINT32              muTuningBufSize;

        // Image buffers
        KeyedVector<PoolKey_T, android::sp<IBufferPool>> mvImagePools;
};

/*******************************************************************************
* Namespace end.
********************************************************************************/

}
}
}
}



#endif // _MTK_CAMERA_FEATURE_PIPE_WORKING_BUFFER_POOL_H_
