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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_LEGACYPIPELINE_STEREOSELECTOR_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_LEGACYPIPELINE_STEREOSELECTOR_H_
//
#include <utils/RefBase.h>
#include <utils/Vector.h>
//
#include <mtkcam/pipeline/utils/streaminfo/MetaStreamInfo.h>
#include <mtkcam/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam/pipeline/utils/streambuf/StreamBuffers.h>
#include <mtkcam/pipeline/utils/streambuf/StreamBufferProvider.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProvider.h>
#include <mtkcam/middleware/v1/LegacyPipeline/processor/ResultProcessor.h>
#include <mtkcam/middleware/v1/camutils/IBuffer.h>


typedef NSCam::v3::Utils::HalImageStreamBuffer      HalImageStreamBuffer;
typedef NSCam::v3::IImageStreamInfo                 IImageStreamInfo;
typedef NSCam::v3::Utils::IStreamBufferProvider     IStreamBufferProvider;

using namespace android::MtkCamUtils;
using namespace NSCam::v1;

/******************************************************************************
 *
 ******************************************************************************/
//
namespace NSCam {
namespace v1 {
namespace NSLegacyPipeline{

class StereoBufferSynchronizer;

class StereoSelector
    : public BaseSelector
{
public:
                                        StereoSelector(char const* szCallerName);

                                        ~StereoSelector() {};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ISelector Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    virtual android::status_t           selectResult(
                                            MINT32                          rRequestNo,
                                            Vector<MetaItemSet>             rvResult,
                                            Vector<BufferItemSet>           rvBuffers,
                                            MBOOL                           errorResult
                                        );

    virtual android::status_t           getResult(
                                            MINT32&                          rRequestNo,
                                            Vector<MetaItemSet>&             rvResultMeta,
                                            Vector<BufferItemSet>&           rvBuffers
                                        );

    virtual android::status_t           getResultPreview(
                                            MINT32&                          rRequestNo,
                                            Vector<MetaItemSet>&             rvResultMeta,
                                            android::sp<IImageBufferHeap>&   rpHeap
                                        );

    virtual android::status_t           getResultCapture(
                                            MINT32&                          rRequestNo,
                                            Vector<MetaItemSet>&             rvResultMeta,
                                            android::sp<IImageBufferHeap>&   rpHeap
                                        );

    virtual android::status_t           returnBuffer(
                                            BufferItemSet&    rBuffer
                                        );

    virtual android::status_t           flush();

    virtual android::status_t           setSynchronizer(
                                            android::wp<StereoBufferSynchronizer>  rpSynchronizer, StreamId_T  streamId
                                        );

    char const*                         getUserName() {return mName;};

    virtual android::status_t           sendCommand( MINT32 cmd,
                                                        MINT32 arg1,
                                                        MINT32 arg2,
                                                        MINT32 arg3,
                                                        MVOID* arg4 = NULL
                                                        ) { return OK; }

    virtual StreamId_T                  getStreamId();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  RefBase Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual void                        onLastStrongRef( const void* /*id*/);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    // struct ResultSet_T {
    //     MUINT32                       requestNo;
    //     android::sp<IImageBufferHeap> heap;
    //     Vector<MetaItemSet>           resultMeta;
    // };

    // KeyedVector<MUINT32, ResultSet_T>   mResultSetMap;
    mutable Mutex                       mResultSetLock;

protected:
    android::wp<IConsumerPool>         mpPool;
    const char*                        mName;
    StreamId_T                         mStreamId;
    wp<StereoBufferSynchronizer>       mpSyncronizer;
    int                                mLogLevel;
};

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSLegacyPipeline
};  //namespace v1
};  //namespace NSCam
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_LEGACYPIPELINE_STEREOSELECTOR_H_

