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

#ifndef _MTK_CAMERA_DUMMY_POST_PROCESSOR_FACTORY_H_
#define _MTK_CAMERA_DUMMY_POST_PROCESSOR_FACTORY_H_

// Standard C header file
#include <list>
// Android system/core header file

// mtkcam custom header file

// mtkcam global header file

// Module header file
#include <mtkcam/pipeline/stream/IStreamInfo.h>
#include <mtkcam/middleware/v1/PostProc/IPostProc.h>
#include <mtkcam/middleware/v1/camshot/BufferCallbackHandler.h>
// Local header file

/*******************************************************************************
* Namespace start.
********************************************************************************/

namespace android {
namespace NSPostProc {

/*******************************************************************************
* Class Define
********************************************************************************/
/**
 * @class DummyPostProcessor
 * @brief Processing post image
 */
class DummyPostProcessor:
        public ImagePostProcessorBase, public NSCam::v1::IImageCallback, public Thread
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    DummyPostProcessor(PostProcessorType type);
    virtual ~DummyPostProcessor();
    /**
     * @brief submit setting to post processor and execute PostProc
     * @return The process result
     */
    virtual android::status_t                  doPostProc(android::sp<PostProcRequestSetting> setting) override;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IImageCallback interface
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    MERROR                  onResultReceived(
                                        MUINT32    const requestNo,
                                        StreamId_T const streamId,
                                        MBOOL      const errorBuffer,
                                        android::sp<IImageBuffer>& pBuffer) override;

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
  std::list<android::sp<PostProcRequestSetting>> mQueue;
  mutable Mutex                                  mQueueLock;
  mutable Condition                              mQueueCond;
  MBOOL                                          mbExit = MFALSE;
};
};
};
#endif  //  _MTK_CAMERA_DUMMY_POST_PROCESSOR_FACTORY_H_
