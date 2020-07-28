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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_LEGACYPIPELINE_BUFFER_BUFFERCALLBACKHANDLER_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_LEGACYPIPELINE_BUFFER_BUFFERCALLBACKHANDLER_H_
//
#include <utils/RefBase.h>
#include <utils/Vector.h>
//
#include <mtkcam/pipeline/stream/IStreamInfo.h>
#include <mtkcam/pipeline/utils/streaminfo/MetaStreamInfo.h>
#include <mtkcam/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam/pipeline/utils/streambuf/StreamBuffers.h>
#include <mtkcam/pipeline/utils/streambuf/StreamBufferProvider.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProvider.h>



typedef NSCam::v3::Utils::HalImageStreamBuffer      HalImageStreamBuffer;
typedef NSCam::v3::IImageStreamInfo                 IImageStreamInfo;
typedef NSCam::v3::Utils::IStreamBufferProvider     IStreamBufferProvider;

using namespace android::MtkCamUtils;

/******************************************************************************
 *
 ******************************************************************************/
//
namespace NSCam {
namespace v1 {

/******************************************************************************
 *
 ******************************************************************************/
class IImageCallback
    : public virtual android::RefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual                             ~IImageCallback() {};

    /**
     *
     * Received result buffer.
     *
     * @param[in] RequestNo : request number.
     *
     * @param[in] pBuffer : IImageBuffer.
     *
     */
    virtual MERROR                      onResultReceived(
                                            MUINT32    const           RequestNo,
                                            StreamId_T const           streamId,
                                            MBOOL      const           errorBuffer,
                                            android::sp<IImageBuffer>& pBuffer
                                        )                                                       = 0;
};

/******************************************************************************
 *
 ******************************************************************************/
class Notifier
    : public virtual android::RefBase
{
public:

    virtual                             ~Notifier() {};

    virtual MVOID                       onBufferAcquired(
                                            MINT32           rRequestNo,
                                            StreamId_T       rStreamId
                                        )                                                       = 0;

    virtual bool                        onBufferReturned(
                                            MINT32                         rRequestNo,
                                            StreamId_T                     rStreamId,
                                            bool                           bErrorBuffer,
                                            android::sp<IImageBufferHeap>& rpBuffer
                                        )                                                       = 0;
};

/******************************************************************************
 *
 ******************************************************************************/
class CallbackBufferPool
    : public IBufferPool
{
public:
                                        CallbackBufferPool(android::sp<IImageStreamInfo> pStreamInfo, MBOOL isSecurity = MFALSE, SecType secType = SecType::mem_normal);

                                        ~CallbackBufferPool();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IBufferPool Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:   //// operations.

    virtual MERROR                      acquireFromPool(
                                            char const*                    szCallerName,
                                            MINT32                         rRequestNo,
                                            android::sp<IImageBufferHeap>& rpBuffer,
                                            MUINT32&                       rTransform
                                        );

    virtual MERROR                      releaseToPool(
                                            char const*                   szCallerName,
                                            MINT32                        rRequestNo,
                                            android::sp<IImageBufferHeap> pBuffer,
                                            MUINT64                       rTimeStamp,
                                            bool                          rErrorResult
                                        );

public:    //// debug

    virtual char const*                 poolName() const;

    virtual MVOID                       dumpPool() const;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    MVOID                               setNotifier( android::sp<Notifier> pNotify );

    virtual MERROR                      allocateBuffer(
                                            char const* szCallerName,
                                            size_t maxNumberOfBuffers,
                                            size_t minNumberOfInitialCommittedBuffers
                                        );
    virtual MERROR                      addAllocateBuffer(
                                            size_t const addNumberOfBuffers
                                        );
    virtual MERROR                      addBuffer( android::sp<IImageBuffer>& pBuffer );

    virtual MERROR                      returnBufferToPool(
                                            char const* szCallerName,
                                            android::sp<IImageBufferHeap> rpBuffer
                                        );

    virtual android::sp<IImageStreamInfo> getStreamInfo();

    virtual MERROR                        updateStreamInfo(
                                            const sp<IImageStreamInfo>& pStreamInfo
                                        );

    virtual MVOID                       flush();

    virtual MVOID                       setForceNoNeedReturnBuffer( MBOOL bFlag );

    virtual MVOID                       setExtraParam( MBOOL contiguousPlanes);

protected:
    class CallbackBufferPoolImp;
    android::sp<CallbackBufferPoolImp>  mImp;
    bool                                mbNoNeedReturnBuffer;
};


/******************************************************************************
 *
 ******************************************************************************/
class BufferCallbackHandler
    : public Notifier
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
                                        BufferCallbackHandler( MINT32 const rOpenId );

                                        ~BufferCallbackHandler() {};

    MVOID                               setImageCallback(
                                            android::wp<IImageCallback> pCb
                                        ) {
                                            mpCallback = pCb;
                                        }

    android::sp<CallbackBufferPool>     queryBufferPool( StreamId_T id ) {
                                            return mvBufferPool.valueFor(id).pool;
                                        }

    MVOID                               setBufferPool(
                                            android::sp<CallbackBufferPool> pPool
                                        );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Notifier Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual MVOID                       onBufferAcquired(
                                            MINT32           rRequestNo,
                                            StreamId_T       rStreamId
                                        );

    virtual bool                        onBufferReturned(
                                            MINT32                         rRequestNo,
                                            StreamId_T                     rStreamId,
                                            bool                           bErrorBuffer,
                                            android::sp<IImageBufferHeap>& rpBuffer
                                        );
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  RefBase Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    void                                onLastStrongRef(const void* /*id*/);

protected:
    MINT32                              mOpenId;
    mutable Mutex                       mLock;

    struct Buffer_T
    {
        MINT32                    requestNo;
        StreamId_T                streamId;
        bool                      isReturned;
        bool                      isError;
        android::sp<IImageBuffer> buffer;
    };

    struct Info_T
    {
        StreamId_T                streamId;
        android::sp<CallbackBufferPool> pool;
    };

    KeyedVector<StreamId_T, Info_T>       mvBufferPool;
    android::wp<IImageCallback>           mpCallback;
    KeyedVector<MINT32, Vector<Buffer_T>> mvOrder;

};

/******************************************************************************
 *
 ******************************************************************************/

};  //namespace v1
};  //namespace NSCam
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_LEGACYPIPELINE_BUFFER_BUFFERCALLBACKHANDLER_H_

