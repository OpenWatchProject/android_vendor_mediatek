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
#ifndef __IMFLLIMAGEBUFFERQUEUE_H__
#define __IMFLLIMAGEBUFFERQUEUE_H__

#include <mtkcam3/feature/mfnr/IMfllImageBuffer.h>

// STL
#include <memory>
#include <functional>

namespace mfll
{
// IMfllImageBufferQueue is an interface for control the image buffer queue of
// IMfllImageBuffer for MFNR Core only. And due to pure virtual interface here,
// it means that the MFNR module MfllImageBuffer must to implement the
// interface of IMfllImageBufferQueue.
//
// The aim of IMfllImageBufferQueue is to re-use all MfllImageBuffer in MFNR
// Core for Dual Phase case, and it makes memory footprints much lower.
//
// Note: all methods are thread-safe.
class IMfllImageBufferQueue
{
// method
public:
    // To get (or create if not exist) the IMfllImageBufferQueue of the certain
    // type of IMfllImageBuffer.
    //  @param f            Type of IMfllImageBuffer.
    //  @return             Returns the shared pointer contains the instance
    //                      of IMfllImageBufferQueue. If it's nullptr, it means
    //                      the buffer queue mechanism hasn't be implemented.
    //  @note               The IMfllImageBufferQueue is a shared object when
    //                      it's been created, the life time of the instance is
    //                      controlled by std::shared_ptr, which means if the
    //                      reference count has counted to 0, the instance will
    //                      be released and all the buffers will be released too.
    static std::shared_ptr<IMfllImageBufferQueue>
        getInstance(IMfllImageBuffer_Flag_t f);

    // Try to get the IMfllImageBufferQueue. This method returns a weak pointer
    // of the instance IMfllImageBufferQueue.
    //  @param f            Type of IMfllImageBuffer.
    //  @return             Returns the weak pointer contains the instance of
    //                      IMfllImageBufferQueue. Caller has to promote it
    //                      (using std::weak_ptr::lock), if the instance is
    //                      nullptr, which means the IMfllImageBufferQueue is
    //                      destroyed or not support.
    static std::weak_ptr<IMfllImageBufferQueue>
        tryGetInstance(IMfllImageBuffer_Flag_t f);


// method
public:
    // To accquire the certain type buffer. If buffer count is greater than
    // maximum limit, this method block the current thread and wait until
    // returnBuffer has been invoked.
    //  @return             An IMfllImageBuffer instance, it may be inited or not.
    virtual IMfllImageBuffer* accquireBuffer() = 0;

    // To return a buffer to the IMfllImageBufferQueue. If no one accquireBuffer,
    // the buffer(s) will be kept until the IMfllImageBufferQueue has destroyed.
    //  @param pBuffer      Pointer of IMfllImageBuffer.
    virtual void returnBuffer(IMfllImageBuffer* pBuffer) = 0;

    // To update the maximum value that the instance queue can create.
    // if the original limit is greater than l, it won't be updated.
    //  @param l            Value of maximum buffer count limit.
    virtual void updateInstanceLimit(int l) = 0;

    // To get the value of instance limit
    //  @return             Value of maximum buffer count limit.
    virtual int getInstanceLimit() = 0;


public:
    virtual ~IMfllImageBufferQueue(){}
}; // class IMfllImageBufferQueue
}; // namespace mfll
#endif//__IMFLLIMAGEBUFFERQUEUE_H__
