/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef _MTK_CAMERA_CORE_CAMSHOT_INC_CAMSHOT_MULTISHOT_H_
#define _MTK_CAMERA_CORE_CAMSHOT_INC_CAMSHOT_MULTISHOT_H_
//
#include <mtkcam/middleware/v1/LegacyPipeline/ILegacyPipeline.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProvider.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/VirtualBufferPool.h>
#include <utils/Vector.h>

#include <semaphore.h>
using namespace std;
//
#include <utils/Mutex.h>
using namespace android;

#include <mtkcam/middleware/v1/camshot/BufferCallbackHandler.h>
#include <mtkcam/middleware/v1/LegacyPipeline/processor/ResultProcessor.h>
#include "../MultiShot/MShotDispatcher.h"
using namespace NSCam::v1;
using namespace NSCam::v1::NSLegacyPipeline;

/*******************************************************************************
*
********************************************************************************/
namespace NSCamShot {
////////////////////////////////////////////////////////////////////////////////
class CamShotImp;
/*******************************************************************************
*
********************************************************************************/
class MultiShot
    : public virtual RefBase
    , public CamShotImp
    , public IImageCallback
    , public ResultProcessor::IListener
    , public VirtualBufferPool::IVirtualPoolCallback
    , public MShotDispatcher::IDispatcherCallback
{
    enum{
        LCSO_FORCE_OFF = 0,
        LCSO_FORCE_ON,
        LCSO_RUNTIME_DECIDE,
        LCSO_GET_PROPERTY,
    };
    enum{
        eMAX = 0,
        eMIN,
    };
    struct Request
    {
        MINT32 requestNo;
        Vector<ISelector::MetaItemSet> meta;
        Vector<ISelector::BufferItemSet> bufferSet;
        //
        Request()
            : requestNo(-1)
            , meta()
            , bufferSet()
            {}
        //
        Request(
            MINT32 _requestNo,
            Vector<ISelector::MetaItemSet> _meta,
            Vector<ISelector::BufferItemSet> _bufferSet
            )
            : requestNo(_requestNo)
            , meta(_meta)
            , bufferSet(_bufferSet)
            {}
    };


public:     ////    Constructor/Destructor.
                    MultiShot(
                        EShotMode const eShotMode,
                        char const*const szCamShotName,
                        EPipelineMode const ePipelineMode
                    );
    virtual         ~MultiShot();

public:     ////    Instantiation.
    virtual MBOOL   init();
    virtual MBOOL   uninit();

public:     ////    Operations.
    virtual MBOOL   start(SensorParam const & rSensorParam, MUINT32 count);
    virtual MBOOL   startAsync(SensorParam const & rSensorParam);
    virtual MBOOL   stop();

public:     ////    Settings.
    virtual MBOOL   setShotParam(ShotParam const & rParam);
    virtual MBOOL   setJpegParam(JpegParam const & rParam);

public:     ////    buffer setting.
    virtual MBOOL   registerImageBuffer(ECamShotImgBufType const eBufType, IImageBuffer const *pImgBuffer);

public:     ////    Old style commnad.
    virtual MBOOL   sendCommand(MINT32 cmd, MINT32 arg1, MINT32 arg2, MINT32 arg3 , MVOID* arg4 = NULL);

public:     ////    implement ResultProcessor::IListener
    virtual void                onResultReceived(
                                        MUINT32         const requestNo,
                                        StreamId_T      const streamId,
                                        MBOOL           const errorResult,
                                        IMetadata       const result
                                    );

    virtual void                onFrameEnd(
                                    MUINT32         const requestNo
                                ){};

    virtual String8             getUserName() { return String8("MultiShot"); }

public:     ////    implement IImageCallback
    virtual MERROR              onResultReceived(
                                        MUINT32    const           RequestNo,
                                        StreamId_T const           streamId,
                                        MBOOL      const           errorBuffer,
                                        android::sp<IImageBuffer>& pBuffer
                                    );

public:     ////    implement IVirtualPoolCallback
    virtual MVOID               onReturnBufferHeap(
                                        StreamId_T                          id,
                                        android::sp<IImageBufferHeap>       spBufferHeap
                                    );

public:     ////    implement IDispatcherCallback
    virtual MVOID               onFrameNotify(
                                        MINT32 const frameNo,
                                        MINT32 const nodeId
                                    );

protected:  ////
    MUINT32                             getRotation() const;
    MVOID                               updateFinishDataMsg(MUINT32 datamsg);
    MBOOL                               getRequest(sp<NSCam::v1::ISelector> spSelector, Vector<Request>& vRequest, MINT32 maxCount);
    MVOID                               selectTodoRequest(Vector<Request>& vCandidateRequest, Vector<Request>& vTodoRequest);
    MVOID                               clearRequest(sp<NSCam::v1::ISelector> spSelector, Vector<Request>& vRequest);

protected:  ////    for capture pipeline
    MINT32                              getCapBufCount(StreamId_T streamId, MINT32 type);
    MBOOL                               createCaptureStreams(StreamBufferProvider* pProvider = NULL);
    MBOOL                               createCapturePipeline(StreamBufferProvider* pProvider = NULL);

protected:  ////    for preview pipeline
    MERROR                              createPreviewPipeline();

    MERROR                                      decideRrzoImage(
                                                    NSCamHW::HwInfoHelper& helper,
                                                    MUINT32 const bitDepth,
                                                    MSize referenceSize,
                                                    MUINT const usage,
                                                    MINT32 const minBuffer,
                                                    MINT32 const maxBuffer,
                                                    sp<IImageStreamInfo>& rpImageStreamInfo,
                                                    MBOOL useUFO = MFALSE
                                                );

    MERROR                              decideImgoImage(
                                                    NSCamHW::HwInfoHelper& helper,
                                                    MUINT32 const bitDepth,
                                                    MSize referenceSize,
                                                    MUINT const usage,
                                                    MINT32 const minBuffer,
                                                    MINT32 const maxBuffer,
                                                    sp<IImageStreamInfo>& rpImageStreamInfo,
                                                    MBOOL useUFO = MFALSE
                                                );

    MERROR                              decideLcsoImage(
                                                    NSCamHW::HwInfoHelper& helper,
                                                    MUINT32 const bitDepth,
                                                    MSize referenceSize,
                                                    MUINT const usage,
                                                    MINT32 const minBuffer,
                                                    MINT32 const maxBuffer,
                                                    sp<IImageStreamInfo>& rpImageStreamInfo
                                                );

public:
    class MShotPreviewThread
        : public virtual android::RefBase
        , public Thread
    {
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  Thread Interface.
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        public:
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
        //  RefBase Interface.
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        public:
            virtual MVOID                           onLastStrongRef( const void* /*id*/);

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  Implementations.
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        public:
                                                    MShotPreviewThread();

            MERROR                                  start(
                                                        android::wp<ILegacyPipeline>    apPipeline,
                                                        MINT32                          aStartRequestNumber,
                                                        MINT32                          aEndRequestNumber,
                                                        IMetadata&                      aAppMeta,
                                                        IMetadata&                      aHalMeta
                                                    );

            MVOID                                   stop();

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  Data Members.
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        protected:
            mutable Mutex                                   mLock;
            android::wp< ILegacyPipeline >                  mpPipeline;

            IMetadata                                       mAppMeta;
            IMetadata                                       mHalMeta;
            MBOOL                                           mbNeedPrecap;

            MINT32                                          mRequestNumber;
            MINT32                                          mRequestNumberMin;
            MINT32                                          mRequestNumberMax;

    }; // class RequestThread

private:    //// data members

    ////      parameters
    SensorParam                 mSensorParam;
    ShotParam                   mShotParam;
    JpegParam                   mJpegParam;

    PipelineSensorParam         mPreviewSensorParam;

    MUINT32                     mShotCount;
    MUINT32                     mCurCount;

    MSize                       mSensorSize;
    MINT32                      mSensorFps;
    MUINT32                     mPixelMode;

    sp<IImageStreamInfo>        mpInfo_FullRaw;
    sp<IImageStreamInfo>        mpInfo_LcsoRaw;
    sp<IImageStreamInfo>        mpInfo_Yuv;
    sp<IImageStreamInfo>        mpInfo_YuvThumbnail;
    sp<IImageStreamInfo>        mpInfo_Jpeg;

    sp<NSCam::v1::NSLegacyPipeline::ILegacyPipeline>
                                mpPreviewPipeline;
    sp<NSCam::v1::NSLegacyPipeline::ILegacyPipeline>
                                mpCapturePipeline;

    sp<NSCam::v1::ISelector>    mpSelector;
    sp<BufferCallbackHandler>   mpCallbackHandler;
    sp<VirtualBufferPool>       mpImgoPool;
    sp<VirtualBufferPool>       mpLcsoPool;

    sp<MShotPreviewThread>      mpPreviewThread;

    MINT32                      mRequestNumber;
    MINT32                      mRequestNumberMin;
    MINT32                      mRequestNumberMax;

    //        shot related control
    Mutex                       mLock;
    Condition                   mNextFrameCond;
    MBOOL                       mbNeedPreview;
    MBOOL                       mbTorchFlash;
    MBOOL                       mbToStop;
    MBOOL                       mbOnStop;
    MBOOL                       mbDoShutterCb;
    MUINT32                     mFinishedData;
    MINT32                      mCurSubmitCount;

    IMetadata                   mPreviewAppMetadata;
    IMetadata                   mPreviewHalMetadata;

    KeyedVector< MINT32, sp<IImageBuffer> > mvRegBuf;

    MUINT32                     muNRType;
    MBOOL                       mbEnGPU;

    //        optimize
    MBOOL                       mbMemOpt;//memory optimization
                                         //true : use less memory, can have better performance only when jpeg node is faster than p2node
                                         //false : use more memory, always has better performance
    MBOOL                       mbP2Opt;//p2node optimization
                                        //true : has better performance and use more memory, keep two requests in p2node
                                        //false : has worse performance and use less memory, keep only one request in p2node
    MINT64                      mTargetDurationMs;//decide frame duration of first two capture frame when p2node optimization on
    //        speed control
    MUINT32                     muCaptureFps;
    MUINT32                     muPreviewFps;
    //        LCSO control
    MINT32                      mPrvPipeLCS;
    MINT32                      mCapPipeLCS;
    MBOOL                       mbPrvEnableLCS;
    MBOOL                       mbCapEnableLCS;

private:    //// debug
    MUINT32                     mDumpFlag;
    sp<MShotDispatcher>         mpDispatcher;
};


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamShot
#endif  //  _MTK_CAMERA_CORE_CAMSHOT_INC_CAMSHOT_MULTISHOT_H_

