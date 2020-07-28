/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
#define LOG_TAG "MtkCam/vmgr_Wpool"

//
#include <sys/prctl.h>
#include <sys/resource.h>
#include <system/thread_defs.h>
//
#include "WorkPool.h"
#include "../MyUtils.h"
#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>
#include <mtkcam/utils/imgbuf/ImageBufferHeap.h>


#define TIMEOUT  (1000000000L)

// ----------------------------------------------------------------------------
// macros
// ----------------------------------------------------------------------------
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

using namespace android;
using namespace NSCam;
using namespace NSCam::plugin;

/******************************************************************************
 *
 ******************************************************************************/
WorkPool::
WorkPool(bool allocateImageBuffer)
    : mImgBuf(allocateImageBuffer)
{}

/******************************************************************************
 *
 ******************************************************************************/
WorkPool::
~WorkPool()
{
    clear();
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
WorkPool::
clear()
{
    for( auto &fut : mvFutures ) {
        if(fut.valid())
            MERROR result = fut.get();
    }
    mvFutures.clear();
    //
    {
        Mutex::Autolock _l(mLock);
        if ( !mInUseBuf.empty() )  mClearCond.wait(mLock);
        //
        mpStreamInfo = nullptr;
        mAvailableBuf.clear();
        mInUseBuf.clear();
        mResource.clear();
    }

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
void
WorkPool::
addHeap(sp<IImageStreamInfo> pInfo, sp<IImageBufferHeap> pHeap)
{
    Mutex::Autolock _l(mLock);
    mpStreamInfo = pInfo;
    mAvailableBuf.push_back(pHeap);
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
WorkPool::
allocate(sp<IImageStreamInfo> pInfo)
{
    if ( mpStreamInfo.get() ) {
        MY_LOGE("allocate before.");
        return -ENODEV;
    }
    //
    {
        Mutex::Autolock _l(mLock);
        mpStreamInfo = pInfo;
        mAvailableBuf.clear();
        mInUseBuf.clear();
    }
    //
    for (size_t i = 0; i < pInfo->getMinInitBufNum(); ++i)
    {
        if(!mImgBuf) {
            sp<IImageBufferHeap> pHeap;
            if( do_construct(pHeap, false) == NO_MEMORY ) {
                MY_LOGE("do_construct allocate buffer failed");
                return true;
            }
            {
                Mutex::Autolock _l(mLock);
                mAvailableBuf.push_back(pHeap);
                mCond.signal();
            }
        } else {
            sp<IImageBuffer> pBuf;
            if( do_construct(pBuf, false) == NO_MEMORY ) {
                MY_LOGE("do_construct allocate buffer failed");
                return true;
            }
            {
                Mutex::Autolock _l(mLock);
                mAvailableImgBuf.push_back(pBuf);
                mCond.signal();
            }
        }
    }

    for( auto &fut : mvFutures ) {
        MERROR result = fut.get();
    }
    mvFutures.clear();
    //
    {
        Mutex::Autolock _l(mLock);
        size_t current = mAvailableBuf.size() + mInUseBuf.size();
        if ( current < pInfo->getMaxBufNum() )
        {
            mvFutures.push_back(
                std::async(std::launch::async,
                    [ this ](size_t rCurrent, size_t rMaxBufferNumber) {
                        ::prctl(PR_SET_NAME, (unsigned long)"allocate", 0, 0, 0);
                        //
                        MERROR err = OK;
                        while( rCurrent != rMaxBufferNumber ) {
                            if(!mImgBuf) {
                                sp<IImageBufferHeap> pHeap;
                                if( do_construct(pHeap, false) == NO_MEMORY ) {
                                    MY_LOGE("do_construct allocate buffer failed");
                                    continue;
                                }
                                {
                                    Mutex::Autolock _l(mLock);
                                    mAvailableBuf.push_back(pHeap);
                                    rCurrent++;
                                    mCond.signal();
                                }
                            } else {
                                sp<IImageBuffer> pBuf;
                                if( do_construct(pBuf, false) == NO_MEMORY ) {
                                    MY_LOGE("do_construct allocate buffer failed");
                                    continue;
                                }
                                {
                                    Mutex::Autolock _l(mLock);
                                    mAvailableImgBuf.push_back(pBuf);
                                    rCurrent++;
                                    mCond.signal();
                                }
                            }
                        }
                        //
                        return err;
                    }, current, pInfo->getMaxBufNum()
                )
            );
        }
    }

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
sp<IImageBufferHeap>
WorkPool::
allocateOne(sp<IImageStreamInfo> pInfo)
{
    mpStreamInfo = pInfo;
    sp<IImageBufferHeap> pHeap;
    if( do_construct(pHeap, false) == NO_MEMORY ) {
        MY_LOGE("do_construct allocate buffer failed");
    }

    return pHeap;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
WorkPool::
allocateFromBlob(sp<IImageStreamInfo> pInfo)
{
    if ( mpStreamInfo.get() ) {
        MY_LOGE("allocate before.");
        return -ENODEV;
    }
    //
    {
        Mutex::Autolock _l(mLock);
        mpStreamInfo = pInfo;
        mAvailableBuf.clear();
        mInUseBuf.clear();
        //
        mAvailableImgBuf.clear();
        mInUseImgBuf.clear();
    }
    //
    for (size_t i = 0; i < pInfo->getMinInitBufNum(); ++i)
    {
        if(!mImgBuf) {
            sp<IImageBufferHeap> pHeap;
            if( do_construct(pHeap, true) == NO_MEMORY ) {
                MY_LOGE("do_construct allocate buffer failed");
                return true;
            }
            {
                Mutex::Autolock _l(mLock);
                mAvailableBuf.push_back(pHeap);
                mCond.signal();
            }
        } else {
            sp<IImageBuffer> pBuf;
            if( do_construct(pBuf, true) == NO_MEMORY ) {
                MY_LOGE("do_construct allocate buffer failed");
                return true;
            }
            {
                Mutex::Autolock _l(mLock);
                mAvailableImgBuf.push_back(pBuf);
                mCond.signal();
            }
        }
    }

    for( auto &fut : mvFutures ) {
        MERROR result = fut.get();
    }
    mvFutures.clear();
    //
    {
        Mutex::Autolock _l(mLock);
        size_t current = mAvailableBuf.size() + mInUseBuf.size();
        if ( current < pInfo->getMaxBufNum() )
        {
            mvFutures.push_back(
                std::async(std::launch::async,
                    [ this ](size_t rCurrent, size_t rMaxBufferNumber) {
                        ::prctl(PR_SET_NAME, (unsigned long)"allocate", 0, 0, 0);
                        //
                        MERROR err = OK;
                        while( rCurrent != rMaxBufferNumber ) {
                            if(!mImgBuf) {
                                sp<IImageBufferHeap> pHeap;
                                if( do_construct(pHeap, true) == NO_MEMORY ) {
                                    MY_LOGE("do_construct allocate buffer failed");
                                    continue;
                                }
                                {
                                    Mutex::Autolock _l(mLock);
                                    mAvailableBuf.push_back(pHeap);
                                    rCurrent++;
                                    mCond.signal();
                                }
                            } else {
                                sp<IImageBuffer> pBuf;
                                if( do_construct(pBuf, true) == NO_MEMORY ) {
                                    MY_LOGE("do_construct allocate buffer failed");
                                    continue;
                                }
                                {
                                    Mutex::Autolock _l(mLock);
                                    mAvailableImgBuf.push_back(pBuf);
                                    rCurrent++;
                                    mCond.signal();
                                }
                            }
                        }
                        //
                        return err;
                    }, current, pInfo->getMaxBufNum()
                )
            );
        }
    }
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
sp<IImageBufferHeap>
WorkPool::
allocateFromBlobOne(sp<IImageStreamInfo> pInfo)
{
    mpStreamInfo = pInfo;
    sp<IImageBufferHeap> pHeap;
    if( do_construct(pHeap, true) == NO_MEMORY ) {
        MY_LOGE("do_construct allocate buffer failed");
    }

    return pHeap;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
WorkPool::
do_construct( sp<IImageBufferHeap>& pImageBufferHeap, MBOOL continuous )
{
    IImageStreamInfo::BufPlanes_t const& bufPlanes = mpStreamInfo->getBufPlanes();
    size_t bufStridesInBytes[3] = {0};
    size_t bufBoundaryInBytes[3]= {0};
    for (size_t i = 0; i < bufPlanes.size(); i++) {
        bufStridesInBytes[i] = bufPlanes[i].rowStrideInBytes;
    }

    if ( eImgFmt_JPEG == mpStreamInfo->getImgFormat() ||
         eImgFmt_BLOB == mpStreamInfo->getImgFormat() )
    {
        IImageBufferAllocator::ImgParam imgParam(
                mpStreamInfo->getImgSize(),
                (*bufStridesInBytes),
                (*bufBoundaryInBytes));
        imgParam.imgFormat = eImgFmt_BLOB;
        MY_LOGD("eImgFmt_JPEG -> eImgFmt_BLOB");
        pImageBufferHeap = IIonImageBufferHeap::create(
                                mpStreamInfo->getStreamName(),
                                imgParam,
                                IIonImageBufferHeap::AllocExtraParam(),
                                MFALSE
                            );
    }
    else if ( !continuous ) {
        IImageBufferAllocator::ImgParam imgParam(
            mpStreamInfo->getImgFormat(),
            mpStreamInfo->getImgSize(),
            bufStridesInBytes, bufBoundaryInBytes,
            bufPlanes.size()
            );
        MY_LOGD("format:%x, size:(%d,%d), stride:%zu, boundary:%zu, planes:%zu",
            mpStreamInfo->getImgFormat(), mpStreamInfo->getImgSize().w, mpStreamInfo->getImgSize().h,
            bufStridesInBytes[0], bufBoundaryInBytes[0], bufPlanes.size());
        pImageBufferHeap = IIonImageBufferHeap::create(
                                mpStreamInfo->getStreamName(),
                                imgParam,
                                IIonImageBufferHeap::AllocExtraParam(),
                                MFALSE
                            );
    } else {
        /* To avoid non-continuous multi-plane memory, allocate ION memory and map it to ImageBuffer */
        MUINT32 fmt          = mpStreamInfo->getImgFormat();
        MUINT32 plane        = NSCam::Utils::Format::queryPlaneCount(fmt);
        MUINT32 allPlaneSize = 0;
        for (size_t i = 0; i < plane; i++)
        {
            allPlaneSize += ((Utils::Format::queryPlaneWidthInPixels(fmt,i, mpStreamInfo->getImgSize().w)
                * Utils::Format::queryPlaneBitsPerPixel(fmt,i)) / 8)
                * Utils::Format::queryPlaneHeightInPixels(fmt, i, mpStreamInfo->getImgSize().h);
        }
        MY_LOGD("allocBuffer size(%d) (%dx%d)", allPlaneSize, mpStreamInfo->getImgSize().w, mpStreamInfo->getImgSize().h);

        // allocate blob buffer
        IImageBufferAllocator::ImgParam blobParam = IImageBufferAllocator::ImgParam(allPlaneSize, 0);

#if 1
        pImageBufferHeap = IIonImageBufferHeap::create(LOG_TAG, blobParam);
        if (pImageBufferHeap == NULL)
        {
            MY_LOGE("pHeap is NULL");
            return NO_MEMORY;
        }
#else
        IImageBufferAllocator *allocator = IImageBufferAllocator::getInstance();
        sp<IImageBuffer> tmpImageBuffer = allocator->alloc(LOG_TAG, blobParam, MFALSE);
        if (tmpImageBuffer == NULL)
        {
            MY_LOGE("tmpImageBuffer is NULL");
            return NO_MEMORY;
        }

        // NOTE: after sp holds the allocated buffer, free can be called anywhere
        allocator->free(tmpImageBuffer.get());

        MUINT const usage = (eBUFFER_USAGE_SW_READ_OFTEN |
                             eBUFFER_USAGE_SW_WRITE_OFTEN |
                             eBUFFER_USAGE_HW_CAMERA_READWRITE);
        if (!tmpImageBuffer->lockBuf(LOG_TAG, usage))
        {
            MY_LOGE("lock Buffer failed");
            return NO_MEMORY;
        }

        // encapsulate tmpImageBuffer into external ImageBuffer
        IImageBufferAllocator::ImgParam extParam(
                fmt, mpStreamInfo->getImgSize(), bufStridesInBytes, bufBoundaryInBytes, plane);
        PortBufInfo_v1 portBufInfo =
            PortBufInfo_v1(
                    tmpImageBuffer->getFD(),
                    tmpImageBuffer->getBufVA(0),
                    0, 0, 0);
        tmpImageBuffer->unlockBuf(LOG_TAG);

        pImageBufferHeap = ImageBufferHeap::create(LOG_TAG, extParam, portBufInfo, MFALSE);

        if (pImageBufferHeap == NULL)
        {
            MY_LOGE("pHeap is NULL");
            return NO_MEMORY;
        }

        mResource.push_back(tmpImageBuffer);
#endif
    }
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
WorkPool::
do_construct( sp<IImageBuffer>& pImageBuffer, MBOOL continuous )
{
    status_t err = OK;
    sp<IImageBufferHeap> pImageBufferHeap;
    err = do_construct(pImageBufferHeap, continuous);
    if ( pImageBufferHeap.get() )
    {
        if (continuous)
        {
            size_t strideInBytes[3] = {0};
            MSize size    = mpStreamInfo->getImgSize();
            MUINT32 fmt   = mpStreamInfo->getImgFormat();
            MUINT32 plane = NSCam::Utils::Format::queryPlaneCount(fmt);
            for (MUINT32 i = 0; i < plane; i++)
            {
                strideInBytes[i] = (Utils::Format::queryPlaneWidthInPixels(fmt, i, size.w) *
                    Utils::Format::queryPlaneBitsPerPixel(fmt, i)) / 8;
#ifdef __DEBUG
                MY_LOGD("strideInBytes[%d]:%d", i, strideInBytes[i]);
#endif
            }

            pImageBuffer = pImageBufferHeap->createImageBuffer_FromBlobHeap((size_t)0,
                (MINT32)mpStreamInfo->getImgFormat(), size, strideInBytes);
        }
        else
        {
            pImageBuffer = pImageBufferHeap->createImageBuffer();
        }
    }
    return err;
}

/******************************************************************************
 *
 ******************************************************************************/
sp<IImageBufferHeap>
WorkPool::
acquireHeap()
{
    Mutex::Autolock _l(mLock);
    sp<IImageBufferHeap> heap;
    if( !mAvailableBuf.empty() ) {
        typename List< sp<IImageBufferHeap> >::iterator iter = mAvailableBuf.begin();
        heap = (*iter);
        mInUseBuf.push_back(*iter);
        mAvailableBuf.erase(iter);
        //
        return heap;
    }

    mCond.waitRelative(mLock, TIMEOUT);

    if( !mAvailableBuf.empty() ) {
        typename List< sp<IImageBufferHeap> >::iterator iter = mAvailableBuf.begin();
        heap = (*iter);
        mInUseBuf.push_back(*iter);
        mAvailableBuf.erase(iter);
        //
        return heap;
    }

    return nullptr;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
WorkPool::
releaseHeap(sp<IImageBufferHeap> pHeap)
{
    Mutex::Autolock _l(mLock);
    typename List< sp<IImageBufferHeap> >::iterator iter = mInUseBuf.begin();
    while( iter != mInUseBuf.end() )
    {
        if( (*iter).get() == pHeap.get() ) {
            mAvailableBuf.push_back(*iter);
            mInUseBuf.erase(iter);
            if ( mInUseBuf.empty() ) mClearCond.signal();
            //
            break;
        }
        iter++;
    }
    mCond.signal();

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
sp<IImageBuffer>
WorkPool::
acquireImageBuffer()
{
    if (!mImgBuf) return nullptr;
    //
    Mutex::Autolock _l(mLock);
    sp<IImageBuffer> buf;
    if( !mAvailableImgBuf.empty() ) {
        typename List< sp<IImageBuffer> >::iterator iter = mAvailableImgBuf.begin();
        buf = (*iter);
        mInUseImgBuf.push_back(*iter);
        mAvailableImgBuf.erase(iter);
        //
        return buf;
    }

    mCond.waitRelative(mLock, TIMEOUT);

    if( !mAvailableImgBuf.empty() ) {
        typename List< sp<IImageBuffer> >::iterator iter = mAvailableImgBuf.begin();
        buf = (*iter);
        mInUseImgBuf.push_back(*iter);
        mAvailableImgBuf.erase(iter);
        //
        return buf;
    }

    return nullptr;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
WorkPool::
releaseImageBuffer(sp<IImageBuffer> pBuf)
{
    Mutex::Autolock _l(mLock);
    typename List< sp<IImageBuffer> >::iterator iter = mInUseImgBuf.begin();
    while( iter != mInUseImgBuf.end() )
    {
        if( (*iter).get() == pBuf.get() ) {
            mAvailableImgBuf.push_back(*iter);
            mInUseImgBuf.erase(iter);
            if ( mInUseImgBuf.empty() ) mClearCond.signal();
            //
            break;
        }
        iter++;
    }
    mCond.signal();

    return OK;
}
