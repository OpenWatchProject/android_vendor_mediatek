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

#ifndef _MTK_HARDWARE_PIPELINE_EXTENSION_UTIL_WORKPOOL_H_
#define _MTK_HARDWARE_PIPELINE_EXTENSION_UTIL_WORKPOOL_H_

#include <utils/StrongPointer.h>
#include <utils/Condition.h>
#include <utils/Mutex.h>
#include <utils/List.h>

// MTKCAM
#include <mtkcam/utils/std/common.h>
#include <mtkcam/pipeline/extension/IVendorManager.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

// STL
#include <vector>
#include <future>

using namespace android;

namespace NSCam {
namespace plugin {

class WorkPool : public RefBase
{
public:     ////
    /**
     * @param allocateImageBuffer : true indicate pool will handle IImageBuffer
     *                              instead of IImageBufferHeap. use image buffer
     *                              related function.
     */
                                WorkPool(bool allocateImageBuffer = false);
    virtual                     ~WorkPool();

public:     ////
    /**
     * allocate non-continuous memory
     */
    status_t                    allocate(sp<IImageStreamInfo> pInfo);
    sp<IImageBufferHeap>        allocateOne(sp<IImageStreamInfo> pInfo);
    /**
     * allocate continuous memory
     */
    status_t                    allocateFromBlob(sp<IImageStreamInfo> pInfo);
    sp<IImageBufferHeap>        allocateFromBlobOne(sp<IImageStreamInfo> pInfo);
    /**
     * clear pool
     */
    status_t                    clear();
    /**
     * buffe heap related function
     */
    sp<IImageBufferHeap>        acquireHeap();
    status_t                    releaseHeap(sp<IImageBufferHeap> pHeap);
    void                        addHeap(sp<IImageStreamInfo> pInfo, sp<IImageBufferHeap> pHeap);
    /**
     * image buffer related function
     */
    sp<IImageBuffer>            acquireImageBuffer();
    status_t                    releaseImageBuffer(sp<IImageBuffer> pBuf);
    //
    sp<IImageStreamInfo>        getStreamInfo() { return mpStreamInfo; }

protected:
    status_t                    do_construct(
                                    sp<IImageBufferHeap>& pImageBufferHeap,
                                    MBOOL continuous
                                );
    status_t                    do_construct(
                                    sp<IImageBuffer>&      pImageBuffer,
                                    MBOOL continuous
                                );

protected:
    bool                                mImgBuf;
    //
    mutable Mutex                       mLock;
    Condition                           mCond;
    Condition                           mClearCond;
    sp<IImageStreamInfo>                mpStreamInfo;
    List< sp<IImageBufferHeap> >        mAvailableBuf;
    List< sp<IImageBufferHeap> >        mInUseBuf;
    //
    List< sp<IImageBuffer> >            mAvailableImgBuf;
    List< sp<IImageBuffer> >            mInUseImgBuf;
    //
    std::vector<sp<IImageBuffer>>       mResource;

protected:
    std::vector< std::future<MERROR> >  mvFutures;

}; // class WorkPool
}; // namespace NSCam
}; // namespace plugin
#endif//_MTK_HARDWARE_PIPELINE_EXTENSION_UTIL_WORKPOOL_H_
