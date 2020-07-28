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

#ifndef _MTK_HARDWARE_MTKCAM_LEGACYPIELINE_STREAMBUFFERPROVIDERIMP_H_
#define _MTK_HARDWARE_MTKCAM_LEGACYPIELINE_STREAMBUFFERPROVIDERIMP_H_
//
#include <utils/RefBase.h>
#include <utils/Vector.h>
//
#include <mtkcam/pipeline/utils/streaminfo/MetaStreamInfo.h>
#include <mtkcam/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam/pipeline/utils/streambuf/StreamBuffers.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProvider.h>

typedef NSCam::v3::Utils::HalImageStreamBuffer      HalImageStreamBuffer;
typedef NSCam::v3::IImageStreamInfo                 IImageStreamInfo;
typedef NSCam::v3::Utils::IStreamBufferProvider     IStreamBufferProvider;

/******************************************************************************
 *
 ******************************************************************************/
//
namespace NSCam {
namespace v1 {

enum ProviderMsg_T {
    TIMESTAMP_INFO
};

/******************************************************************************
 *
 ******************************************************************************/
class PStreamBufferProviderImp
    : public StreamBufferProvider
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IConsumerPool Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    virtual android::status_t           returnBuffer(
                                            android::sp<IImageBufferHeap>    rpHeap
                                        );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ITimestampCallback Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    virtual void                        doTimestampCallback(
                                            MUINT32     requestNo,
                                            MBOOL       errorResult,
                                            MINT64      timestamp
                                        );

    virtual void                        onFrameEnd(
                                            MUINT32     requestNo
                                        );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IStreamBufferProvider Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    virtual MERROR                      dequeStreamBuffer(
                                            MUINT32 const                       iRequestNo,
                                            android::sp<IImageStreamInfo> const rpStreamInfo,
                                            android::sp<HalImageStreamBuffer>&  rpStreamBuffer
                                        );

    virtual MERROR                      enqueStreamBuffer(
                                            android::sp<IImageStreamInfo> const rpStreamInfo,
                                            android::sp<HalImageStreamBuffer>   rpStreamBuffer,
                                            MUINT32                             bBufStatus
                                        );


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
                                        PStreamBufferProviderImp();

                                        ~PStreamBufferProviderImp();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StreamBufferProvider Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    virtual MERROR                          dequeStreamBufferAndWait(
                                                MUINT32 const                       iRequestNo,
                                                android::sp<IImageStreamInfo> const rpStreamInfo,
                                                android::sp<HalImageStreamBuffer>&  rpStreamBuffer
                                            );

    virtual MVOID                           notifyNoBufDeque(
                                                MUINT32 const                       iRequestNo
                                            );

    virtual MERROR                          updateBufferCount(
                                                char const*                   szCallerName,
                                                MINT32                        rMaxBufferNumber
                                            );

    virtual android::status_t               setImageStreamInfo(
                                                android::sp<IImageStreamInfo> pStreamInfo
                                            );

    virtual android::sp<IImageStreamInfo>   queryImageStreamInfo();

    virtual android::status_t               setBufferPool(
                                                android::sp< IBufferPool >    pBufProvider
                                            );

    virtual android::status_t               setSelector(
                                                android::sp< ISelector > pRule
                                            );

    virtual android::status_t               switchSelector(
                                                android::sp< ISelector > pNewRule
                                            );

    virtual android::sp< ISelector >        querySelector();

    virtual MVOID                           onLastStrongRef( const void* /*id*/);

    virtual MVOID                           flush();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    MVOID                                   sendCommand(MINT32 cmd, MUINT32 arg1, MUINT32 arg2);

protected:
    MBOOL                                   hasTimeStampInfo;

protected:
    struct ResultSet_T {
        MUINT32                         requestNo;
        android::sp<IImageBufferHeap>   heap;
        MBOOL                           error;
        MINT64                          timestamp;
    };

public:
    MERROR                              setPairRule(
                                            android::sp< PairMetadata > pConsumer
                                        );

protected:

    void                                handleResult(
                                            Vector<ResultSet_T> const& rvResult
                                        );

    android::status_t                   dequeResult(
                                            Vector<ResultSet_T>& rvResult
                                        );

    android::status_t                   handleReturn();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    MINT32                                  mLogLevel;
    const char*                             mName;

protected:
    android::sp< IBufferPool >              mpBufferPool;
    mutable Mutex                           mPoolLock;
    Condition                               mBufferCond;

protected:
    android::sp< IImageStreamInfo >         mpImageStreamInfo;
    android::sp< PairMetadata >             mpConsumer;

protected:
    android::sp< ISelector >                mpRule;
    mutable Mutex                           mRuleLock;

protected:
    struct Buffer_T {
        MUINT32                             requestNo;
        android::sp<IImageBufferHeap>       heap;
    };

    KeyedVector<HalImageStreamBuffer*, Buffer_T>
                                            mBufferMap;
    mutable Mutex                           mBufferLock;

protected:
    mutable Mutex                           mResultSetLock;
    KeyedVector<MUINT32, ResultSet_T>       mResultSetMap;
    mutable Mutex                           mConsumerLock;
    KeyedVector<MUINT32, ResultSet_T>       mConsumerMap;

protected:
    mutable Mutex                           mLock;
    Vector< MUINT32 >                       mvFrameEnd;
};


};  //namespace v1
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_LEGACYPIELINE_STREAMBUFFERPROVIDERIMP_H_

