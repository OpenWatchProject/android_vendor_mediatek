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

#define LOG_TAG "MtkCam/VirtualBufferPool"
//
#include "MyUtils.h"
//
#include <utils/RWLock.h>
//
#include <sys/prctl.h>
#include <sys/resource.h>
//
#include <mtkcam/utils/imgbuf/ImageBufferHeap.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProvider.h>

#include <mtkcam/middleware/v1/LegacyPipeline/buffer/VirtualBufferPool.h>

using namespace android;
using namespace NSCam;
using namespace NSCam::v1;
using namespace NSCam::Utils;

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

#if 1
#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")
#else
#define FUNC_START
#define FUNC_END
#endif

/******************************************************************************
 *
 ******************************************************************************/
VirtualBufferPool::
VirtualBufferPool(
    sp<IVirtualPoolCallback> pCallback,
    StreamId_T               id
)
    : mLogLevel(1)
    , mpCallback(pCallback)
    , streamID(id)
{}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
VirtualBufferPool::
acquireFromPool(
    char const*           szCallerName,
    MINT32                /*rRequestNo*/,
    sp<IImageBufferHeap>& rpBuffer,
    MUINT32&              /*rTransform*/
)
{
    Mutex::Autolock _l(mPoolLock);
    MY_LOGD("[%s] mAvailableBuf:%d mInUseBuf:%d",
            szCallerName, mAvailableBuf.size(), mInUseBuf.size());
    if( !mAvailableBuf.empty() )
    {
        typename List< sp<IImageBufferHeap> >::iterator iter = mAvailableBuf.begin();
        mInUseBuf.push_back(*iter);
        rpBuffer = *iter;
        mAvailableBuf.erase(iter);
        return OK;
    }
    MY_LOGE("[%s] buffer pool is empty", szCallerName);
    return TIMED_OUT;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
VirtualBufferPool::
releaseToPool(
    char const*          szCallerName,
    MINT32               /*rRequestNo*/,
    sp<IImageBufferHeap> rpBuffer,
    MUINT64              /*rTimeStamp*/,
    bool                 /*rErrorResult*/
)
{
    Mutex::Autolock _l(mPoolLock);
    typename List< sp<IImageBufferHeap> >::iterator iter = mInUseBuf.begin();
    while( iter != mInUseBuf.end() ) {
        if ( rpBuffer == (*iter) ) {
            mInUseBuf.erase(iter);

            MY_LOGD(
                "[%s] mAvailableBuf:%d mInUseBuf:%d", szCallerName, mAvailableBuf.size(), mInUseBuf.size()
            );
            if( mpCallback != NULL )
            {
                mpCallback->onReturnBufferHeap(streamID, rpBuffer);
            }
            else
            {
                MY_LOGE("mpCallback == NULL");
                return UNKNOWN_ERROR;
            }
            FUNC_END;
            return OK;
        }
        iter++;
    }
    MY_LOGE("[%s] Cannot find buffer %p.", szCallerName, rpBuffer.get());
    return UNKNOWN_ERROR;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
VirtualBufferPool::
onReceiveBufferHeap(
    android::sp<IImageBufferHeap>   spBufferHeap
)
{
    Mutex::Autolock _l(mPoolLock);
    MY_LOGD("receive buffer heap:%p.", spBufferHeap.get());
    mAvailableBuf.push_back(spBufferHeap);
    return;
}

/******************************************************************************
 *
 ******************************************************************************/
char const*
VirtualBufferPool::
poolName() const
{
    return "VirtualBufferPool";
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
VirtualBufferPool::
dumpPool() const
{
    MY_LOGI("[%#" PRIx64 "] logLevel:%d inUse:%d available:%d callback:%p",
        streamID, mLogLevel, mInUseBuf.size(), mAvailableBuf.size(), mpCallback.get()
    );
    //
    {
        String8 str = String8::format("Available Buffer: ");
        typename List< sp<IImageBufferHeap> >::const_iterator iter = mAvailableBuf.begin();
        while( iter != mAvailableBuf.end() ) {
            str += String8::format("%p", (*iter).get());
            iter++;
        }
        MY_LOGI("%s", str.string());
    }
    //
    {
        String8 str = String8::format("Inuse Buffer: ");
        typename List< sp<IImageBufferHeap> >::const_iterator iter = mInUseBuf.begin();
        while( iter != mInUseBuf.end() ) {
            str += String8::format("%p", (*iter).get());
            iter++;
        }
        MY_LOGI("%s", str.string());
    }
}

/******************************************************************************
 *
 ******************************************************************************/
void
VirtualBufferPool::
onLastStrongRef(const void* /*id*/)
{
    FUNC_START;
    if ( mAvailableBuf.size() > 0 ) {
        typename List< sp<IImageBufferHeap> >::iterator iter = mAvailableBuf.begin();
        while( iter != mAvailableBuf.end() ) {
            mpCallback->onReturnBufferHeap(streamID, *iter);
            iter++;
        }
    }
    mAvailableBuf.clear();

    if ( mInUseBuf.size() > 0 ) {
        typename List< sp<IImageBufferHeap> >::iterator iter = mInUseBuf.begin();
        while( iter != mInUseBuf.end() ) {
            MY_LOGW("[%s] buffer %p not return to pool.", poolName(), (*iter).get());
            iter++;
        }
    }
    mInUseBuf.clear();
    FUNC_END;
}