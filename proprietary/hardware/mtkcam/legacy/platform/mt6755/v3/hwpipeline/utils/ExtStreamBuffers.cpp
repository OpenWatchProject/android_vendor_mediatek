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

#include "ExtStreamBuffers.h"
// #include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>
// #include "MyUtils.h"
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v3::Utils;


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
ExtImageStreamBuffer::
Allocator::
Allocator(
    IStreamInfoT* pStreamInfo,
    IIonImageBufferHeap::AllocImgParam_t const& rAllocImgParam
)
    : mpStreamInfo(pStreamInfo)
    , mAllocImgParam(rAllocImgParam)
{
}


/******************************************************************************
 *
 ******************************************************************************/
ExtImageStreamBuffer::StreamBufferT*
ExtImageStreamBuffer::
Allocator::
operator()(IStreamBufferPoolT* pPool)
{
    sp<IIonImageBufferHeap>
    pImageBufferHeap =
    IIonImageBufferHeap::create(
        mpStreamInfo->getStreamName(),
        mAllocImgParam,
        IIonImageBufferHeap::AllocExtraParam(),
        MFALSE
    );
    if  ( pImageBufferHeap == 0 ) {
        MY_LOGE(
            "IIonImageBufferHeap::create: %s",
            mpStreamInfo->getStreamName()
        );
        return NULL;
    }
    //
    return new StreamBufferT(
        mpStreamInfo,
        pPool,
        pImageBufferHeap
    );
}


/******************************************************************************
 *
 ******************************************************************************/
// ExtImageStreamBuffer::StreamBufferT*
// ExtImageStreamBuffer::
// Allocator::
// operator()(sp<IImageBufferHeap> pImageBufferHeap)
// {

// }


/******************************************************************************
 *
 ******************************************************************************/
ExtImageStreamBuffer::
ExtImageStreamBuffer(
    sp<IStreamInfoT> pStreamInfo,
    wp<IStreamBufferPoolT> pStreamBufPool,
    sp<IImageBufferHeap> pImageBufferHeap
)
    : TStreamBufferT(pStreamInfo, *pImageBufferHeap.get() )
    , mBufPool(pStreamBufPool)
    , mImageBufferHeap(pImageBufferHeap)
    //
{
}


/******************************************************************************
 *
 ******************************************************************************/
ExtImageStreamBuffer::
~ExtImageStreamBuffer()
{
    MY_LOGD("%p +", this);
    MY_LOGD("%p -", this);
}

/******************************************************************************
 *
 ******************************************************************************/
sp<ExtImageStreamBuffer::IStreamBufferPoolT>
ExtImageStreamBuffer::
tryGetBufferPool() const
{
    Mutex::Autolock _l(mBufPoolMutex);
    //
    if  ( mBufPool == 0 ) {
        MY_LOGV("[%s:%p] no buffer pool", getName(), this);
        return NULL;
    }
    //
    sp<IStreamBufferPoolT> const pPool = mBufPool.promote();
    if  ( pPool == 0 ) {
        MY_LOGW(
            "[%s:%p] NULL promote of buffer pool:%p",
            getName(), this, mBufPool.unsafe_get()
        );
    }
    //
    return pPool;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
ExtImageStreamBuffer::
resetBuffer()
{
    reset();
    //
    {
        Mutex::Autolock _l(mBufMutex);
        mBufStatus = 0;
        mWriters.clear();
        mReaders.clear();
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
ExtImageStreamBuffer::
releaseBuffer()
{
    MY_LOGD_IF(0, "+ %s", getName());
    sp<IStreamBufferPoolT> pStreamBufferPool = tryGetBufferPool();
    if  ( pStreamBufferPool != 0 ) {
        //
        //  Reset buffer before returning to pool.
        resetBuffer();
        //
        //  Release to Pool
        MERROR err = pStreamBufferPool->releaseToPool(
            getName(),
            this
        );
        MY_LOGE_IF(OK!=err, "%s fail to release to pool", getName());
    } else {
        MY_LOGW("cannot promote StreamBufferPool");
    }
    MY_LOGD_IF(0, "- %s", getName());
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
ExtImageStreamBuffer::
setUsersManager(
    android::sp<IUsersManager> value
)
{
    mUsersManager = value;
}


/******************************************************************************
 *
 ******************************************************************************/
ExtMetaStreamBuffer::
Allocator::
Allocator(
    IStreamInfoT* pStreamInfo
)
    : mpStreamInfo(pStreamInfo)
{
}


/******************************************************************************
 *
 ******************************************************************************/
ExtMetaStreamBuffer::StreamBufferT*
ExtMetaStreamBuffer::
Allocator::
operator()()
{
    return new StreamBufferT(
        mpStreamInfo
    );
}


/******************************************************************************
 *
 ******************************************************************************/
// ExtMetaStreamBuffer::StreamBufferT*
// ExtMetaStreamBuffer::
// Allocator::
// operator()(NSCam::IMetadata const& metadata)
// {
//     return new StreamBufferT(
//         mpStreamInfo, metadata
//     );
// }


/******************************************************************************
 *
 ******************************************************************************/
ExtMetaStreamBuffer::
ExtMetaStreamBuffer(
    sp<IStreamInfoT> pStreamInfo/*,
    IUsersManager* pUsersManager*/
)
    : TStreamBufferT(pStreamInfo, mMetadata)
    , mMetadata()
    //
{
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
ExtMetaStreamBuffer::
resetBuffer()
{
    reset();
    //
    {
        Mutex::Autolock _l(mBufMutex);
        mBufStatus = 0;
        mWriters.clear();
        mReaders.clear();
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
ExtMetaStreamBuffer::
releaseBuffer()
{
    MY_LOGD_IF(0, "+ %s", getName());
    MY_LOGD_IF(0, "- %s", getName());
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
ExtMetaStreamBuffer::
setUsersManager(
    android::sp<IUsersManager> value
)
{
    mUsersManager = value;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
ExtMetaStreamBuffer::
isRepeating() const
{
    return false;
}
