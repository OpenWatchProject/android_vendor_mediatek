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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

#ifndef _MTK_CAMERA_STEREO_FLOW_CONTROL_BOKEH_POST_PROCESSOR_FACTORY_H_
#define _MTK_CAMERA_STEREO_FLOW_CONTROL_BOKEH_POST_PROCESSOR_FACTORY_H_

#include <list>
#include <vector>
// Module header file
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/PostProcessor/ImagePostProcessor.h>
#include <mtkcam/middleware/v1/LegacyPipeline/ILegacyPipeline.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/buffer/StereoSelector.h>
#include <mtkcam/middleware/v1/LegacyPipeline/stereo/buffer/StereoBufferPool.h>
#include <mtkcam/middleware/v1/LegacyPipeline/IResourceContainer.h>
#include <mtkcam/middleware/v1/IParamsManager.h>
#include <mtkcam/middleware/v1/IParamsManagerV3.h>
//
#include <mtkcam/utils/hw/IScenarioControl.h>
// Local header file
#include "../builder/IPostProcessRequestBuilder.h"
#include "../../../../../../common/CbImgBufMgr/CallbackImageBufferManager.h"
#include "../ICaptureRequestCB.h"

/*******************************************************************************
* Namespace start.
********************************************************************************/

namespace NSCam {
namespace v1 {
namespace NSLegacyPipeline {

struct PostProcRequestInfo
{
  MINT32                         cbCountDown;
  MINT64                         timeStamp;
  sp<IPostProcessRequestBuilder> pRequest;
  wp<ICaptureRequestCB>          pCb;
  MINT32                         captureNo;
};
/*******************************************************************************
* Class Define
********************************************************************************/
/**
 * @class ImagePostProcessor
 * @brief Processing post image
 */
class BokehPostProcessor:
        public ImagePostProcessor, public NSCam::v1::IImageCallback, public NSCam::v1::Notifier
{
public:
    static const int MAX_POSTPROC_REQUSET_CNT = 3;
    static const int STEREO_FLOW_POSTPROC_REQUSET_NUM_START  = 4000;
    static const int STEREO_FLOW_POSTPROC_REQUSET_NUM_END    = 5000;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    BokehPostProcessor();
    virtual ~BokehPostProcessor();
    /**
     * @brief enque to post processor.
     * @return The process result
     */
    android::status_t                  enque(android::sp<ImagePostProcessorData> data) override;
    /**
     * @brief flush post processor queue.
     * @return The process result
     */
    android::status_t                  flush() override;
    /**
     * @brief wait all processing done.
     * @return The process result
     */
    android::status_t                  waitUntilDrain() override;
    /**
     * @brief destroy all member field.
     * @return The process result
     */
    android::status_t                  destroy() override;
    /**
     * @brief get queue size
     * @return return size
     */
    MINT32                             size() override;
    /**
     * @brief set camera mode
     * @return the process result
     */
    android::status_t                  setCameraStatus(CameraMode mode) override;
    /**
     * @brief set callback
     * @return the process result
     */
    android::status_t                  setCallback(android::sp<ICaptureRequestCB> pCb) override;
public:
    MVOID                              onLastStrongRef( const void* /*id*/) override;
public:
    static void destroyPostProcessor();
public:
    android::status_t                  onDequeData(android::sp<ImagePostProcessorData>& data);
    android::status_t                  onProcessData(android::sp<ImagePostProcessorData> const& data);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IImageCallback interface
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    MERROR                  onResultReceived(
                                        MUINT32    const requestNo,
                                        StreamId_T const streamId,
                                        MBOOL      const errorBuffer,
                                        android::sp<IImageBuffer>& pBuffer) override;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Notifier interface
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    virtual MVOID                       onBufferAcquired(
                                            MINT32           rRequestNo,
                                            StreamId_T       rStreamId
                                        ) override;

    virtual bool                        onBufferReturned(
                                            MINT32                         rRequestNo,
                                            StreamId_T                     rStreamId,
                                            bool                           bErrorBuffer,
                                            android::sp<IImageBufferHeap>& rpBuffer
                                        )override;

private:
    android::status_t                 buildPipeline();
    android::status_t                 clearPipeline();
    MVOID                             init();
    MVOID                             unInit();
    MVOID                             enterPerformanceScenario();
    MVOID                             exitPerformanceScenario();
    MBOOL                             mbInit = MFALSE;
    mutable Mutex                     mInitLock;
    //
    MINT32                            mSensorId = -1;
    MINT32                            mSensorIdMain2 = -1;
    //
    mutable Mutex                     mQueueLock;
    mutable Condition                 mQueueCond;
    android::List<android::sp<ImagePostProcessorData> > mQueue;
    MBOOL                                               mbOnProcess;    // to avoid timing issue
    //
    android::sp<ILegacyPipeline>      mpPipeline = nullptr;
    MINT32                            miPostProcCounter = STEREO_FLOW_POSTPROC_REQUSET_NUM_START;
    //
    CameraMode                        mCameraMode = CameraMode::CloseCamera;
    MBOOL                             mbExit = MFALSE;
    // buffer pool for pipeline
    sp<CallbackBufferPool>            mpPool_Main = nullptr;
    sp<CallbackBufferPool>            mpPool_DepthWrapper = nullptr;
    sp<CallbackBufferPool>            mpPool_Dbg = nullptr;
    //
    sp<IResourceContainer>            mpResourceContainier = NULL;
    sp<ImageStreamManager>            mpImageStreamManager = nullptr;
    // check if request done
    mutable Mutex                     mRequestLock;
    DefaultKeyedVector<MUINT32, PostProcRequestInfo> mvPostProcRequests;
    DefaultKeyedVector<MUINT32, PostProcRequestInfo> mvRemoveRequests;

    //
    mutable Mutex                     mCBLock;
    wp<ICaptureRequestCB>             mpCb;
    //
    sp<IScenarioControl>              mpScenarioCtrl = NULL;
/*******************************************************************************
* Class Define
********************************************************************************/
protected:
    class ProcessThread : public Thread
    {
        public:

                                        ProcessThread(BokehPostProcessor* pProcessImp)
                                            : mpProcessImp(pProcessImp)
                                        {}

                                        ~ProcessThread()
                                        {}

        public:

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  Thread Interface.
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

        public:
                        // Ask this object's thread to exit. This function is asynchronous, when the
                        // function returns the thread might still be running. Of course, this
                        // function can be called from a different thread.
                        void        requestExit() override;

                        // Good place to do one-time initializations
                        status_t    readyToRun() override;

        private:
                        // Derived class must implement threadLoop(). The thread starts its life
                        // here. There are two ways of using the Thread object:
                        // 1) loop: if threadLoop() returns true, it will be called again if
                        //          requestExit() wasn't called.
                        // 2) once: if threadLoop() returns false, the thread will exit upon return.
                        bool        threadLoop() override;

        private:

                        BokehPostProcessor*       mpProcessImp = nullptr;
    };
    sp<ProcessThread>             mpProcessThread = nullptr;
};
};
};
};
#endif  //  _MTK_CAMERA_STEREO_FLOW_CONTROL_BOKEH_POST_PROCESSOR_FACTORY_H_
