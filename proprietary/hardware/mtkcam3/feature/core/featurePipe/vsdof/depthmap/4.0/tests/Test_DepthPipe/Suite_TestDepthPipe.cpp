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

// Standard C header file
#include <time.h>
#include <iostream>
#include <chrono>
#include <thread>
// Android system/core header file
#include <gtest/gtest.h>
#include <vector>
// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/drv/IHalSensor.h>
// Module header file
#include <mtkcam3/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam3/feature/stereo/StereoCamEnum.h>
#include <mtkcam3/feature/stereo/pipe/IDepthMapPipe.h>
#include <mtkcam3/feature/stereo/pipe/IDepthMapEffectRequest.h>
#include <mtkcam3/feature/stereo/pipe/DepthPipeHolder.h>
// Local header file
#include "../common/CallbackUTNode.h"
#include "../common/TestDepthMap_Common.h"

#define LOG_TAG "MtkCam/DepthPipeUT"
using namespace NSCam::NSCamFeature::NSFeaturePipe;
using namespace NSCam::NSCamFeature::NSFeaturePipe_DepthMap;
using namespace NSCam::v1::Stereo;

#define TEST_REQ_TYPE eSTATE_NORMAL
#define EIS_ON 0
#define TEST_FLOW_TYPE eDEPTH_FLOW_TYPE_QUEUED_DEPTH //eDEPTH_FLOW_TYPE_STANDARD
#define REQUEST_NEED_FEFM 0
#define TEST_REQ_SIZE 4

sp<IDepthMapEffectRequest>
prepareEnqueRequest(
    int request_id,
    DepthMapPipeOpState eState)
{
    MY_LOGD("+: reqID = %d, request state=%d", request_id, eState);
    // new request
    sp<IDepthMapEffectRequest> pRequest =  IDepthMapEffectRequest::createInstance(request_id, WaitingListener::CB);
    // load raws
    if(eState == eSTATE_CAPTURE)
        prepareReqInputBuffer_Capture(pRequest);
    else
        prepareReqInputBuffer(pRequest);
    // prepare output
    prepareReqOutputBuffer(eState, pRequest);
    if(eState == eSTATE_CAPTURE)
        // prepare metadata
        setupReqMetadata(MFALSE, pRequest);
    else
        setupReqMetadata(EIS_ON, pRequest);

    MY_LOGD("-");
    return pRequest;
}

TEST(DepthMapPipe_Test, Standard)
{
    MY_LOGD("Test_DepthMapPipe");
    NSCam::v1::Stereo::SeneorModuleType moduleType = BAYER_AND_BAYER;
    ENUM_STEREO_RATIO imageRatio = eRatio_16_9;
    MINT32 featureMode = E_STEREO_FEATURE_VSDOF;
    
    UTEnvironmenSetup::Profile profile = {imageRatio, featureMode, moduleType};
    UTEnvironmenSetup setup("Test_DepthMapPipe", profile);
    if(!setup.isReadyToUT())
    {
        EXPECT_TRUE(0);
        return;
    }
    MY_LOGD("Create DepthMapPipe...");
    // main1/main2 sensor index
    int32_t main1Idx, main2Idx;
    StereoSettingProvider::getStereoSensorIndex(main1Idx, main2Idx);
    //
    MY_LOGD("Init 3A HAL main1...");
    IHal3A* p3AHal_Main1 = MAKE_Hal3A(main1Idx, "p3AHal_Main1");
    MY_LOGD("Init 3A HAL main2...");
    IHal3A* p3AHal_Main2 = MAKE_Hal3A(main2Idx, "p3AHal_Main2");
    p3AHal_Main1->destroyInstance("p3AHal_Main1");
    p3AHal_Main2->destroyInstance("p3AHal_Main2");
    MY_LOGD("main1Idx=%d main2Idx=%d", main1Idx, main2Idx);
    //
    sp<DepthMapPipeOption> pPipeOption = new DepthMapPipeOption(moduleType, eDEPTHNODE_MODE_VSDOF, eDEPTH_FLOW_TYPE_QUEUED_DEPTH);
    sp<DepthMapPipeSetting> pPipeSetting = new DepthMapPipeSetting();
    pPipeSetting->miSensorIdx_Main1 = main1Idx;
    pPipeSetting->miSensorIdx_Main2 = main2Idx;
    pPipeSetting->mszRRZO_Main1 = getRRZOMain1Size();
    //
    sp<SmartDepthMapPipe> pDepthPipe = DepthPipeHolder::createPipe(pPipeSetting, pPipeOption);
    MY_LOGD("Prepare EffectRequests...target time:%d", TEST_REQ_SIZE);
    std::vector<sp<IDepthMapEffectRequest>> vEffectReqVec;
    int targetTime  = TEST_REQ_SIZE;
    for(int i=0;i<targetTime;i++)
    {
        sp<IDepthMapEffectRequest> pEffReq = prepareEnqueRequest(i, TEST_REQ_TYPE);
        vEffectReqVec.push_back(pEffReq);
    }
    MY_LOGD("Init the depthmap pipe...");
    pDepthPipe->init();
    // test normal request
    for(int i=0;i<vEffectReqVec.size();i++)
    {
        sp<IDepthMapEffectRequest> pReq = vEffectReqVec[i];
        MY_LOGD("DepthMapEffectHal updateEffectRequest: %d", i);
        pDepthPipe->enque(pReq);
        std::this_thread::sleep_for(33ms);
    }
    MY_LOGD("start to wait!!....");
    bool bRet = WaitingListener::waitRequestAtLeast(targetTime, 8, 4);
    MY_LOGD("Wait done!!....");
    EXPECT_TRUE(bRet);

    WaitingListener::resetCounter();
    return;
}