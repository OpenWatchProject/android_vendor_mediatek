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

#ifndef _MTK_HARDWARE_MTKCAM_LEGACYPIELINE_BUFFER_PAIRMETADATA_H_
#define _MTK_HARDWARE_MTKCAM_LEGACYPIELINE_BUFFER_PAIRMETADATA_H_
//
#include <utils/RefBase.h>
#include <utils/Vector.h>
#include <utils/Thread.h>
//
#include <sys/prctl.h>
#include <sys/resource.h>
#include <system/thread_defs.h>
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


/******************************************************************************
 *
 ******************************************************************************/
class PairMetadataImp
    : public PairMetadata
    , public Thread
{
public:
                                        PairMetadataImp(const char*);

                                        ~PairMetadataImp() {};

public:    //// PairMetadata interface

    virtual android::status_t           getResultBufferFromProvider(
                                            MUINT32                       rRequestNo,
                                            android::sp<IImageStreamInfo> pStreamInfo,
                                            android::sp<IImageBufferHeap> rpHep,
                                            MUINT64                       rTimeStamp,
                                            MBOOL                         rbErrorBuffer
                                        );

    virtual android::status_t           setNumberOfMetadata( MINT32 num );

    virtual android::status_t           setSelector(
                                            android::sp< ISelector > pRule
                                        );

    virtual android::sp< ISelector >    querySelector() { Mutex::Autolock _l(mRuleLock); return mpRule; };

    virtual android::status_t           flush();

public:    //// ResultProcessor::IListener interface

    virtual void                        onResultReceived(
                                            MUINT32         const requestNo,
                                            StreamId_T      const streamId,
                                            MBOOL           const errorResult,
                                            IMetadata       const result
                                        );

    virtual void                        onFrameEnd(
                                            MUINT32         const requestNo
                                        );

    virtual android::String8            getUserName();

public:    //// RefBase interface

    virtual void                        onLastStrongRef( const void* /*id*/);

public:    //// thread interface
    // Ask this object's thread to exit. This function is asynchronous, when the
    // function returns the thread might still be running. Of course, this
    // function can be called from a different thread.
    //virtual void        requestExit();

    // Good place to do one-time initializations
    virtual status_t    readyToRun();

private:
    // Derived class must implement threadLoop(). The thread starts its life
    // here. There are two ways of using the Thread object:
    // 1) loop: if threadLoop() returns true, it will be called again if
    //          requestExit() wasn't called.
    // 2) once: if threadLoop() returns false, the thread will exit upon return.
    virtual bool        threadLoop();

protected:
    struct ResultSet_T {
        MUINT32                             requestNo;
        Vector<ISelector::BufferItemSet>    bufferSet;
        Vector<ISelector::MetaItemSet>      resultMeta;
        MBOOL                               error;
        MUINT64                             timestamp;
    };

protected: //// implementation
    void                                updateResult(
                                            Vector<ResultSet_T> const& rvResult
                                        );

    void                                handleResult(
                                            Vector<ResultSet_T> const& rvResult
                                        );

    android::status_t                   dequeResult(
                                            Vector<ResultSet_T>& rvResult
                                        );

    android::status_t                   handleReturn();

protected:
    MINT32                              mLogLevel;
    const char*                         mName;

protected:
    android::sp< ISelector >            mpRule;
    mutable Mutex                       mRuleLock;

protected:
    KeyedVector<MUINT32, ResultSet_T>     mResultSetMap;
    size_t                                mNumberOfWaitingMeta;
    size_t                                mNumberOfWaitingBuffer;

protected:
    Condition                             mTempQueueCond;
    Vector<ResultSet_T>                   mTempSetMap;
    mutable Mutex                         mTempSetLock;

protected:
    mutable Mutex                           mLock;
    Vector< MUINT32 >                       mvFrameEnd;

protected:
    bool                                  mbExitThread;

};

};  //namespace v1
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_LEGACYPIELINE_BUFFER_PAIRMETADATA_H_

