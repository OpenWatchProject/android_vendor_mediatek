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

#ifndef _MTK_CAMERA_THIRDPARTY_POST_PROCESSOR_H_
#define _MTK_CAMERA_THIRDPARTY_POST_PROCESSOR_H_

// Standard C header file
#include <map>
#include <tuple>
// Android system/core header file

// mtkcam custom header file

// mtkcam global header file

// Module header file
#include <mtkcam/middleware/v1/PostProc/IPostProcCB.h>
#include <mtkcam/middleware/v1/PostProc/IPostProc.h>
// Local header file
#include "../inc/IPostProcRequestBuilder.h"

/*******************************************************************************
* Namespace start.
*******************************************************************************/
namespace android {
namespace NSPostProc {
class TPPPListener;
/******************************************************************************
 *
 ******************************************************************************/
class ThirdPartyPostProcessor :
        public ImagePostProcessorBase
{
public:
    ThirdPartyPostProcessor() = delete;
    ThirdPartyPostProcessor(PostProcessorType type);
    virtual ~ThirdPartyPostProcessor();
public:
    MVOID                   onLastStrongRef( const void* /*id*/) override;
public:
    android::status_t
                            doPostProc(
                                    sp<PostProcRequestSetting> setting) override;
public:
    status_t                onPostProcDone();
    status_t                onEvent(
                                    MINT32 capNo,
                                    PostProcCB callbackType,
                                    MUINT32 streamId,
                                    MBOOL bError,
                                    void* params1 = nullptr,
                                    void* params2 = nullptr);
private:
    status_t buildPipeline();
    sp<NodeConfigDataManager> buildNodeConfigDataManager(
                                    FrameDataTableSet& data);
private:
    //
    MINT32                            mSensorId = -1;
    MINT32                            mSensorIdMain2 = -1;
    //
    sp<NSCam::v1::NSLegacyPipeline::ILegacyPipeline>
                                      mpPipeline = nullptr;
    //
    std::map<MINT32, IPostProcRequestBuilder::CallbackBufferPoolItem> mInputBufferPoolTable;
    //
    sp<ImageStreamManager>            mpImageStreamManager = nullptr;
    //
    using ProcessingBuilder = std::tuple<MINT32, sp<IPostProcRequestBuilder>>;
    mutable Mutex                                   mPostProcRequestTableLock;
    std::map<MINT64, ProcessingBuilder>   mPostProcRequestTable;
    //
    sp<TPPPListener>                  mpListener = nullptr;
    //
    MUINT32                           miRequestNo;
};
/******************************************************************************
 *
 ******************************************************************************/
class TPPPListener : public IPostProcCB
{
public:
    TPPPListener(wp<ThirdPartyPostProcessor> p) : mpImp(p)
    {
    }
    virtual ~TPPPListener(){}
    status_t onEvent(
                        MINT32  capNo,
                        PostProcCB callbackType,
                        MUINT32 streamId,
                        MBOOL bError,
                        void* params1 = nullptr,
                        void* params2 = nullptr) override
    {
        // onEvent called by PostProcessing and StereoFlowControl is destroyed by stopPreview() at the same time, so must check if existing
        sp<ThirdPartyPostProcessor> pCBImp= mpImp.promote();
        return (pCBImp != nullptr) ? pCBImp->onEvent(capNo, callbackType, streamId, bError, params1, params2) : OK;
    }
private:
    wp<ThirdPartyPostProcessor> mpImp;
};
};
};
#endif  //  _MTK_CAMERA_THIRDPARTY_POST_PROCESSOR_H_
