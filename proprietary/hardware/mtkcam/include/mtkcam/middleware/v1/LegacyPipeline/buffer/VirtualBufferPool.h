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

#ifndef _MTK_HARDWARE_MTKCAM_LEGACYPIPELINE_BUFFER_VIRTUALBUFFERPOOL_H_
#define _MTK_HARDWARE_MTKCAM_LEGACYPIPELINE_BUFFER_VIRTUALBUFFERPOOL_H_
//
#include <utils/RefBase.h>
#include <utils/Vector.h>
//
#include "StreamBufferProvider.h"

/******************************************************************************
 *
 ******************************************************************************/
//
namespace NSCam {
namespace v1 {

class VirtualBufferPool
    : public virtual android::RefBase
    , public IBufferPool
{
public:
    class IVirtualPoolCallback : public virtual android::RefBase
    {
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  Interface.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    public:   //// operations.

        /**
         * Return buffer heap to provider.
         *
         * @param[in] spBufferHeap: a buffer heap want to return.
         *
         */
        virtual MVOID                       onReturnBufferHeap(
                                                StreamId_T                      streamID,
                                                android::sp<IImageBufferHeap>   spBufferHeap
                                            )                                               = 0;
    };

public:
                                        VirtualBufferPool( android::sp<IVirtualPoolCallback> pCallback, StreamId_T id );

                                        ~VirtualBufferPool() {};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:   //// operations.

    /**
     * Try to acquire a buffer from the pool.
     *
     * @param[in] szCallerName: a null-terminated string for a caller name.
     *
     * @param[out] rpBuffer: a reference to a newly acquired buffer.
     *
     * @return 0 indicates success; non-zero indicates an error code.
     */
    virtual MERROR                      acquireFromPool(
                                            char const*                    szCallerName,
                                            MINT32                         rRequestNo,
                                            android::sp<IImageBufferHeap>& rpBuffer,
                                            MUINT32&                       rTransform
                                        );

    /**
     * Release a buffer to the pool.
     *
     * @param[in] szCallerName: a null-terminated string for a caller name.
     *
     * @param[in] pBuffer: a buffer to release.
     *
     * @return
     *      0 indicates success; non-zero indicates an error code.
     */
    virtual MERROR                      releaseToPool(
                                            char const*                   szCallerName,
                                            MINT32                        rRequestNo,
                                            android::sp<IImageBufferHeap> pBuffer,
                                            MUINT64                       rTimeStamp,
                                            bool                          rErrorResult
                                        );

    /**
     * Receive a buffer heap from provider.
     *
     * @param[in] spBufferHeap: a buffer heap.
     *
     */
    virtual MVOID                       onReceiveBufferHeap(
                                            android::sp<IImageBufferHeap>   spBufferHeap
                                        );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:    //// debug
    /**
     * Pool name.
     */
    virtual char const*                 poolName() const;

    /**
     * Dump information for debug.
     */
    virtual MVOID                       dumpPool() const;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  RefBase Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    void                                onLastStrongRef(const void* /*id*/);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    MINT32                              mLogLevel;

protected:
    mutable Mutex                       mPoolLock;
    List< sp<IImageBufferHeap> >        mAvailableBuf;
    List< sp<IImageBufferHeap> >        mInUseBuf;
    android::sp<IVirtualPoolCallback>   mpCallback;
    StreamId_T                          streamID;
};

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace v1
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_LEGACYPIPELINE_BUFFER_VIRTUALBUFFERPOOL_H_

