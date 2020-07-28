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

#include "TuningBufferPool.h"

#include <ion/ion.h>
#include <sys/mman.h>
#include <linux/ion_drv.h>
#include <linux/mman-proprietary.h>
#include <mtkcam/def/common.h>

#define PIPE_TRACE 0
#define PIPE_MODULE_TAG "VSDOFUtil"
#define PIPE_CLASS_TAG "TuningBufferPool"
#include <featurePipe/core/include/PipeLog.h>

using namespace NSCam::Utils;
using android::sp;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

TuningBufferHandle::TuningBufferHandle(const sp<BufferPool<TuningBufferHandle> > &pool)
: BufferHandle(pool), mpVA(0)
{
}



sp<TuningBufferPool> TuningBufferPool::create(const char *name, MUINT32 size)
{
    TRACE_FUNC_ENTER();
    sp<TuningBufferPool> pool = new TuningBufferPool(name);
    if(pool == NULL)
    {
        MY_LOGE("OOM: Cannot create TuningBufferPool!");
    }
    else if(!pool->init(size))
    {
        MY_LOGE("Pool initialization failed!");
        pool = NULL;
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

TuningBufferPool::TuningBufferPool(const char* name)
: BufferPool<TuningBufferHandle>(name),
  miBufSize(0)
{
}

TuningBufferPool::~TuningBufferPool()
{
    uninit();
}

MBOOL TuningBufferPool::init(MUINT32 size)
{
    TRACE_FUNC_ENTER();

    android::Mutex::Autolock lock(mMutex);

    if(size <= 0)
        return MFALSE;

    miBufSize = size;

    TRACE_FUNC_EXIT();
    return MTRUE;
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

    free(handle->mpVA);

    TRACE_FUNC_EXIT();
    return MTRUE;
}

#if 0
MERROR TuningBufferPool::ionAllocate(android::sp<TuningBufferHandle>& buffHandle)
{
    int ion_flags = mbCachable ? 0 : (ION_FLAG_CACHED | ION_FLAG_CACHED_NEEDS_SYNC);
    int align = 0;

    // store ion device
    buffHandle->mIonDevice = mIonDevice;

    // ion_alloc_mm: create ion handle
    int err = ::ion_alloc_mm(mIonDevice, miBufSize, align, ion_flags, &buffHandle->mpIonHandle);
    if(err != 0)
    {
        MY_LOGE("ion_alloc_mm failed! err=%d", err);
        return BAD_VALUE;
    }

    // ion_share: ion handle -> buf fd
    err = ::ion_share(mIonDevice, buffHandle->mpIonHandle, &buffHandle->miSharedFD);
    if(0 != err || -1 == buffHandle->miSharedFD)
    {
        MY_LOGE("ion_share failed! err=%d, miSharedFD:%d", err, buffHandle->miSharedFD);
        return BAD_VALUE;
    }
    // ion_mmap: map FD to virtual space
    buffHandle->mpVA = (MVOID*) ::ion_mmap(mIonDevice, NULL, miBufSize, PROT_READ|PROT_WRITE, MAP_SHARED, buffHandle->miSharedFD, 0);
    if(buffHandle->mpVA <= 0)
    {
        MY_LOGE("ion_mmap failed! shardFD=%d va=%x, size=%d", buffHandle->miSharedFD, buffHandle->mpVA, miBufSize);
        return BAD_VALUE;
    }
    // clear memory
    memset((void*)buffHandle->mpVA, 0, miBufSize);

    return OK;
}


MBOOL TuningBufferPool::doRelease_OLD(TuningBufferHandle* handle)
{
    TRACE_FUNC_ENTER();

    // ion_munmap: ummap virtual address
    if( handle->mpVA != 0)
    {
        ::ion_munmap(mIonDevice, (void*) handle->mpVA, miBufSize);
        handle->mpVA = 0;
    }

    // ion_share_close: close buf fd
    if( handle->miSharedFD >= 0)
    {
        ::ion_share_close(mIonDevice, handle->miSharedFD);
        handle->miSharedFD = -1;
    }

    // ion_free: buf handle
    if(handle->mpIonHandle != 0)
    {
        ::ion_free(mIonDevice, handle->mpIonHandle);
        handle->mpIonHandle = 0;
    }

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL TuningBufferHandle::syncCache(TuningBufSyncCtrl eCtrl)
{
    if(ion_import(mIonDevice, miSharedFD, &mpIonHandle))
    {
        MY_LOGE("ion_import fail, memId(0x%x)", miSharedFD);
    }
    // cache sync by range
    struct ion_sys_data sys_data;
    sys_data.sys_cmd=ION_SYS_CACHE_SYNC;
    sys_data.cache_sync_param.handle=mpIonHandle;
    switch(eCtrl)
    {
        case eTUNINGBUF_CACHECTRL_FLUSH:
            sys_data.cache_sync_param.sync_type=ION_CACHE_FLUSH_BY_RANGE;
            break;
        case eTUNINGBUF_CACHECTRL_SYNC:
            sys_data.cache_sync_param.sync_type=ION_CACHE_INVALID_BY_RANGE;
            break;
        default:
            MY_LOGE("ERR cmd(%d)", eCtrl);
            break;
    }
    if(ion_custom_ioctl(mIonDevice, ION_CMD_SYSTEM, &sys_data))
    {
        MY_LOGE("CAN NOT DO SYNC, memID/(0x%x)", miSharedFD);
        // decrease handle ref count if cache fail
        if(ion_free(mIonDevice, mpIonHandle))
        {
            MY_LOGE("ion_free fail");
        }
        return MFALSE;
    }

    // decrease handle ref count
    if(ion_free(mIonDevice, mpIonHandle))
    {
        MY_LOGE("ion_free fail");
        return MFALSE;
    }
    return MTRUE;
}
#endif

} //NSFeaturePipe
} //NSCamFeature
} //NSCam