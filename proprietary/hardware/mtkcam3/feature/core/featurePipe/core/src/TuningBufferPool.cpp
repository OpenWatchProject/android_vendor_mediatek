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

#include "../include/DebugControl.h"
#define PIPE_TRACE TRACE_TUNING_BUFFER_POOL
#define PIPE_CLASS_TAG "TuningBufferPool"

#include "../include/PipeLog.h"
#include "../include/TuningBufferPool.h"
#include <mtkcam3/feature/utils/PostRedZone.h>
#include <cutils/properties.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_COMMON);

using namespace NSCam::Utils;
using android::sp;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

TuningBufferHandle::TuningBufferHandle(const sp<BufferPool<TuningBufferHandle> > &pool)
: BufferHandle(pool), mpVA(0)
{
}



sp<TuningBufferPool> TuningBufferPool::create(const char *name, MUINT32 size, BufProtect bufProtect)
{
    TRACE_FUNC_ENTER();
    MINT32 prop = property_get_int32(KEY_TUNING_BUF_POOL_PROTECT, VAL_TUNING_BUF_PROTECT);
    MBOOL useBufProtect = (prop != -1) ? (prop > 0) : (bufProtect == BUF_PROTECT_RUN);
    sp<TuningBufferPool> pool = new TuningBufferPool(name, useBufProtect);
    if(pool == NULL)
    {
        MY_LOGE("OOM: Cannot create TuningBufferPool!");
    }
    else if(!pool->init(size))
    {
        MY_LOGE("Pool initialization failed! size(%u)", size);
        pool = NULL;
    }
    else
    {
        MY_LOGD("TuningBufPool(%s) created. size(%u), protect(%d)", name, size, useBufProtect);
    }
    TRACE_FUNC_EXIT();
    return pool;
}

MVOID TuningBufferPool::destroy(android::sp<TuningBufferPool> &pool)
{
    TRACE_FUNC_ENTER();
    if(pool != NULL)
    {
        pool->releaseAll();
        pool = NULL;
    }
    TRACE_FUNC_EXIT();
}

TuningBufferPool::TuningBufferPool(const char* name, MBOOL bufProtect)
    : BufferPool<TuningBufferHandle>(name)
    , miBufSize(0)
    , mIsBufProtect(bufProtect)

{
}

TuningBufferPool::~TuningBufferPool()
{
    uninit();
}

MBOOL TuningBufferPool::init(MUINT32 size)
{
    MBOOL ret = MTRUE;
    TRACE_FUNC_ENTER();

    android::Mutex::Autolock lock(mMutex);

    if(size <= 0)
    {
        ret = MFALSE;
    }
    miBufSize = size;

    TRACE_FUNC_EXIT();
    return ret;
}

MVOID TuningBufferPool::uninit()
{
    TRACE_FUNC_ENTER();
    android::Mutex::Autolock lock(mMutex);
    this->releaseAll();
    TRACE_FUNC_EXIT();
}

sp<TuningBufferHandle> TuningBufferPool::doAllocate()
{
    TRACE_FUNC_ENTER();

    android::Mutex::Autolock lock(mMutex);
    sp<TuningBufferHandle> bufferHandle;

    if((bufferHandle = new TuningBufferHandle(this)) == NULL)
    {
        MY_LOGE("TuningBufferHandle create failed!");
        return NULL;
    }

    if(mIsBufProtect)
        bufferHandle->mpVA = PostRedZone::mynew(miBufSize);
    else
        bufferHandle->mpVA = (MVOID*) malloc(miBufSize);

    if(bufferHandle->mpVA == NULL)
    {
        MY_LOGE("Out of memory!!");
        return NULL;
    }
    // clear memory
    memset((void*)bufferHandle->mpVA, 0, miBufSize);

    TRACE_FUNC_EXIT();

    return bufferHandle;
}

MBOOL TuningBufferPool::doRelease(TuningBufferHandle* handle)
{
    TRACE_FUNC_ENTER();

    if(mIsBufProtect)
        PostRedZone::mydelete(handle->mpVA);
    else
        free(handle->mpVA);

    TRACE_FUNC_EXIT();
    return MTRUE;
}

} //NSFeaturePipe
} //NSCamFeature
} //NSCam
