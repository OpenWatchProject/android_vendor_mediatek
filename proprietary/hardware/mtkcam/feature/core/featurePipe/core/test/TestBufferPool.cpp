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

#include "TestBufferPool.h"

#define PIPE_TRACE 0
#define PIPE_MODULE_TAG "FeaturePipeTest"
#define PIPE_CLASS_TAG "TestBufferPool"
#include <featurePipe/core/include/PipeLog.h>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

TestBufferHandle::TestBufferHandle(const android::sp<BufferPool<TestBufferHandle> > &pool)
    : BufferHandle(pool)
    , mType(TestBufferHandle::ALLOCATE)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

android::sp<TestBufferPool> TestBufferPool::create(unsigned size)
{
    TRACE_FUNC_ENTER();
    android::sp<TestBufferPool> pool = new TestBufferPool();
    if( pool == NULL )
    {
        MY_LOGE("OOM: Cannot create GraphicBufferPool");
    }
    else if( !pool->init(size) )
    {
        pool = NULL;
    }
    TRACE_FUNC_EXIT();
    return pool;
}

MVOID TestBufferPool::destroy(android::sp<TestBufferPool> &pool)
{
    TRACE_FUNC_ENTER();
    if( pool != NULL )
    {
        pool->releaseAll();
        pool = NULL;
    }
    TRACE_FUNC_EXIT();
}

TestBufferPool::TestBufferPool()
    : BufferPool<TestBufferHandle>("testpool")
    , mReady(false)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

TestBufferPool::~TestBufferPool()
{
    TRACE_FUNC_ENTER();
    uninit();
    TRACE_FUNC_EXIT();
}

MBOOL TestBufferPool::init(unsigned size)
{
    TRACE_FUNC_ENTER();

    android::Mutex::Autolock lock(mMutex);
    MBOOL ret = MFALSE;

    if( mReady )
    {
        MY_LOGE("Already init");
    }
    else
    {
        mSize = size;
        mReady = MTRUE;
        ret = MTRUE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MVOID TestBufferPool::uninit()
{
    TRACE_FUNC_ENTER();
    android::Mutex::Autolock lock(mMutex);
    if( mReady )
    {
        this->releaseAll();
        mReady = MFALSE;
    }
    TRACE_FUNC_EXIT();
}

MBOOL TestBufferPool::add(char *buffer)
{
    TRACE_FUNC_ENTER();

    MBOOL ret = MFALSE;
    android::Mutex::Autolock lock(mMutex);
    android::sp<TestBufferHandle> handle;

    if( !mReady )
    {
        MY_LOGE("pool need init first");
    }
    else if( (handle = new TestBufferHandle(this)) == NULL )
    {
        MY_LOGE("OOM: create bufferHandle failed");
    }
    else if( buffer == NULL )
    {
        MY_LOGE("invalid buffer handle");
    }
    else
    {
        handle->mBuffer = buffer;
        handle->mType = TestBufferHandle::REGISTER;
        addToPool(handle);
        ret = MTRUE;
    }

    TRACE_FUNC_EXIT();
    return ret;
}

android::sp<TestBufferHandle> TestBufferPool::doAllocate()
{
    TRACE_FUNC_ENTER();

    android::Mutex::Autolock lock(mMutex);

    android::sp<TestBufferHandle> bufferHandle;

    if( !mReady )
    {
        MY_LOGE("pool need init first");
        return NULL;
    }

    if( (bufferHandle = new TestBufferHandle(this)) == NULL )
    {
        MY_LOGE("OOM: create bufferHandle failed");
        return NULL;
    }
    bufferHandle->mBuffer = new char[mSize];
    if( bufferHandle->mBuffer == NULL )
    {
        MY_LOGE("OOM: create test buffer failed");
        return NULL;
    }
    bufferHandle->mType = TestBufferHandle::ALLOCATE;

    TRACE_FUNC_EXIT();
    return bufferHandle;
}

MBOOL TestBufferPool::doRelease(TestBufferHandle *handle)
{
    TRACE_FUNC_ENTER();

    // release should not need lock(mMutex)
    // becuare only BufferPool::releaseAll and
    // BufferPool::recycle calls release for handles for the pool,
    // and no handle can exist when IMemDrv is invalid

    MBOOL ret = MTRUE;

    if( !handle )
    {
        MY_LOGE("TestBufferHandle missing");
        ret = MFALSE;
    }
    else if( handle->mType == TestBufferHandle::ALLOCATE )
    {
        if( handle->mBuffer == NULL )
        {
            MY_LOGE("TestBufferHandle::mTestBuffer missing");
            ret = MFALSE;
        }
        else
        {
            delete [] handle->mBuffer;
            handle->mBuffer = NULL;
        }
    }
    else
    {
        handle->mBuffer = NULL;
    }

    TRACE_FUNC_EXIT();
    return ret;
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
