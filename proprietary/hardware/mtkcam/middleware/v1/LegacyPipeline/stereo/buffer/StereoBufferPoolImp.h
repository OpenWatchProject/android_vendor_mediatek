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

#ifndef _MTK_HARDWARE_MTKCAM_LEGACYPIPELINE_BUFFER_STEREO_BUFFER_POOL_IMP_H_
#define _MTK_HARDWARE_MTKCAM_LEGACYPIPELINE_BUFFER_STEREO_BUFFER_POOL_IMP_H_
//
#include <utils/RefBase.h>
#include <utils/Vector.h>
#include <utils/Thread.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProvider.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/buffer/StereoBufferPool.h>

/******************************************************************************
 *
 ******************************************************************************/
//
namespace NSCam {
namespace v1 {

class StereoBufferPoolImp
    : public StereoBufferPool
    , public Thread
{
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  IbufferPool Interface.
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
    //// debug
    /**
     * Pool name.
     */
    virtual char const*                 poolName() const;

    /**
     * Dump information for debug.
     */
    virtual MVOID                       dumpPool() const;

public:
                                        StereoBufferPoolImp( const char* name, sp<IImageStreamInfo> _streamInfo);

                                        ~StereoBufferPoolImp() {};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    // set selector, which will be used to return buffer
    virtual MERROR                      setSelector(
                                            sp<ISelector> pSelector
                                        );

    // set an available buffer to this pool
    virtual MERROR                      setBuffer(
                                            char const* szCallerName,
                                            android::sp<IImageBufferHeap> pBuffer
                                        );
    // set synchronizer, which will be used to return buffer for ZSL capture
    virtual MERROR                      setSynchronizer(
                                            sp<StereoBufferSynchronizer> pSynchronizer
                                        );
    /**
    *    decide whether to return to synchronizer(for ZSL capture)
    *    or selector(retrun directly to Pass1 usable pool)
    */
    virtual MERROR                      setReturnMode(
                                            MINT32 retrunMode
                                        );
    virtual MUINT32                     getCurrentAvailableBufSize();
    virtual MUINT32                     getInUseBufSize();
    virtual MERROR                      returnAllAvailableBuffer();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  RefBase Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    void                                onLastStrongRef(const void* /*id*/);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Thread Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    // Ask this object's thread to exit. This function is asynchronous, when the
    // function returns the thread might still be running. Of course, this
    // function can be called from a different thread.
    virtual void        requestExit();

    // Good place to do one-time initializations
    virtual status_t    readyToRun();

private:
    // Derived class must implement threadLoop(). The thread starts its life
    // here. There are two ways of using the Thread object:
    // 1) loop: if threadLoop() returns true, it will be called again if
    //          requestExit() wasn't called.
    // 2) once: if threadLoop() returns false, the thread will exit upon return.
    virtual bool        threadLoop();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MERROR                              returnBuffer(android::sp<IImageBufferHeap> pBuffer);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    MINT32                              mLogLevel            = 0;
    MINT32                              mReturnMode          = RETURN_MODE::TO_SELECTOR;

    const char*                         mName;
    sp<IImageStreamInfo>                mStreamInfo;

    mutable Mutex                       mPoolLock;
    Condition                           mCondPoolLock;
    List< sp<IImageBufferHeap> >        mAvailableBuf;
    List< sp<IImageBufferHeap> >        mInUseBuf;

    sp<ISelector>                       mspSelector          = nullptr;
    sp<StereoBufferSynchronizer>        mspSynchronizer      = nullptr;
};

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace v1
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_LEGACYPIPELINE_BUFFER_STEREO_BUFFER_POOL_IMP_H_

