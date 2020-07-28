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

#include "ImgBufferStore.h"

#include "DebugControl.h"
#define PIPE_CLASS_TAG "ImgBufferStore"
#define PIPE_TRACE TRACE_IMG_BUFFER_STORE
#include <featurePipe/core/include/PipeLog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_STREAMING);

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

ImgBufferStore::ImgBufferStore()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

ImgBufferStore::~ImgBufferStore()
{
    TRACE_FUNC_ENTER();
    uninit();
    TRACE_FUNC_EXIT();
}

MBOOL ImgBufferStore::init(const android::sp<IBufferPool> &pool)
{
    TRACE_FUNC_ENTER();
    android::Mutex::Autolock poolLock(mPoolMutex);
    android::Mutex::Autolock recordLock(mRecordMutex);
    MBOOL ret = MFALSE;
    if( mBufferPool != NULL )
    {
        MY_LOGE("Invalid init: init already called");
    }
    else if( pool == NULL )
    {
        MY_LOGE("Invalid init: pool = NULL");
    }
    else
    {
        mBufferPool = pool;
        ret = MTRUE;
    }

    TRACE_FUNC_EXIT();
    return ret;
}

MVOID ImgBufferStore::uninit()
{
    TRACE_FUNC_ENTER();
    android::Mutex::Autolock poolLock(mPoolMutex);
    android::Mutex::Autolock recordLock(mRecordMutex);
    if( mRecordMap.size() )
    {
        MY_LOGE("Buffer(%zu) not returned before uninit, force free", mRecordMap.size());
        mRecordMap.clear();
    }
    if( mBufferPool != NULL )
    {
        mBufferPool = NULL;
    }
    TRACE_FUNC_EXIT();
}

IImageBuffer* ImgBufferStore::requestBuffer()
{
    TRACE_FUNC_ENTER();
    IImageBuffer *buffer = NULL;
    android::sp<IIBuffer> imgBuffer;

    mPoolMutex.lock();
    if( mBufferPool == NULL )
    {
        MY_LOGE("Invalid state: pool is NULL");
    }
    else
    {
        imgBuffer = mBufferPool->requestIIBuffer();
        if( imgBuffer != NULL )
        {
            buffer = imgBuffer->getImageBufferPtr();
        }
    }

    // 1. Lock record map mutex after requestIIBuffer()
    //    to prevent deadlock with returnBuffer()
    // 2. Lock record map mutex before unlock pool
    //    to prevent illegal uninit() called during requestBuffer()
    mRecordMutex.lock();
    mPoolMutex.unlock();

    if( imgBuffer == NULL || buffer == NULL )
    {
        MY_LOGE("Failed to get buffer: imgBuffer=%p", imgBuffer.get());
    }
    else
    {
        RecordMap::iterator it = mRecordMap.find(buffer);
        if( it != mRecordMap.end() )
        {
            MY_LOGE("Buffer(%p) already in record(size=%zu), old=%p, new=%p", buffer, mRecordMap.size(), it->second.get(), imgBuffer.get());
            buffer = NULL;
        }
        else
        {
            mRecordMap[buffer] = imgBuffer;
        }
    }

    mRecordMutex.unlock();
    TRACE_FUNC_EXIT();
    return buffer;
}

MBOOL ImgBufferStore::returnBuffer(IImageBuffer *buffer)
{
    TRACE_FUNC_ENTER();
    android::Mutex::Autolock recordLock(mRecordMutex);
    MBOOL ret = MFALSE;
    if( !buffer )
    {
        MY_LOGE("Buffer is NULL");
    }
    else
    {
        RecordMap::iterator it = mRecordMap.find(buffer);
        if( it == mRecordMap.end() )
        {
            MY_LOGE("Cannot find buffer(%p) record(size=%zu)", buffer, mRecordMap.size());
        }
        else
        {
            mRecordMap.erase(it);
            ret = MTRUE;
        }
    }
    TRACE_FUNC_EXIT();
    return ret;
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
