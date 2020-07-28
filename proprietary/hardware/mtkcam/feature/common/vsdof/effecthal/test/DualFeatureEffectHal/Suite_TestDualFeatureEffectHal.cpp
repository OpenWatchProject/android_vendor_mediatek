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

#define LOG_TAG "TestDepthMapPipe"

#include <time.h>
#include <gtest/gtest.h>

#include <vector>

#include <cutils/properties.h>
#include <utils/Vector.h>
#include <utils/String8.h>

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/imgbuf/ImageBufferHeap.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/aaa/IHal3A.h>
#include <mtkcam/feature/effectHalBase/EffectRequest.h>
#include <mtkcam/feature/stereo/effecthal/DualFeatureEffectHal.h>
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
#include <mtkcam/feature/stereo/StereoCamEnum.h>

#include "TestDepthMap_Common.h"
#include "ImgParamSetting.h"

using android::sp;
using android::String8;
using namespace NSCam;
using namespace NSCam::NSCamFeature::NSDualFeature;
using namespace NSCam::v1::Stereo;

namespace NSCam{
namespace NSCamFeature{

#define BAYER_BAYER_TEST_TIME 1
#define BAYER_MONO_TEST_TIME 0
#define TEST_FLOW_TYPE eDEPTH_FLOW_TYPE_STANDARD

#define TEST_SINGLE_MODE_REQ 1
#define TEST_REQ_TYPE eREQTYPE_PV

typedef IImageBufferAllocator::ImgParam ImgParam;
StereoSizeProvider* gpSizePrvder = StereoSizeProvider::getInstance();

sp<DualFeatureRequest> generateEffectRequest(int request_id, TestReqType eState)
{
    MY_LOGD("+: reqID = %d, request state=%d", request_id, eState);

    sp<DualFeatureRequest> pRequest =  DualFeatureRequest::createInstance(request_id, WaitingListener::CB);

    prepareReqInputBuffer(pRequest);
    prepareReqOutputBuffer(eState, pRequest);
    MBOOL bEISOn = (eState == eREQTYPE_VR) ? MTRUE : MFALSE;
    setupReqMetadata(bEISOn, pRequest);

    MY_LOGD("-");
    return pRequest;
}

TEST(DualFeatureEffectHalTest, Standard)
{
    if(BAYER_BAYER_TEST_TIME==0)
        return;

    UTEnvironmenSetup::Profile profile = {eRatio_4_3, STEREO_SENSOR_PROFILE_REAR_REAR,
                                            E_STEREO_FEATURE_VSDOF, BAYER_AND_BAYER};
    UTEnvironmenSetup setup("DualFeatureHal", profile);

    if(!setup.isReadyToUT())
    {
        EXPECT_TRUE(0);
        return;
    }

    MY_LOGD("Create DualFeatureEffectHalTest...");
    DualFeatureSettings setting;
    // main1/main2 sensor index
    int32_t main1Idx, main2Idx;
    StereoSettingProvider::getStereoSensorIndex(main1Idx, main2Idx);
    setting.miSensorIdx_Main1 = main1Idx;
    setting.miSensorIdx_Main2 = main2Idx;
    setting.mSensorType = v1::Stereo::BAYER_AND_BAYER;
    setting.mMode = E_STEREO_FEATURE_VSDOF;
    setting.mszRRZO_Main1 = getRRZOMain1Size();
    // init hal
    DualFeatureEffectHal *pEffectHal = new DualFeatureEffectHal(setting);
    pEffectHal->init();


    MY_LOGD("Prepare EffectRequests...target time:%d", BAYER_BAYER_TEST_TIME);
    std::vector<sp<DualFeatureRequest>> vEffectReqVec;
    int targetTime  = BAYER_BAYER_TEST_TIME;
    for(int i=0;i<targetTime;i++)
    {
        sp<DualFeatureRequest> pEffReq;
        if(TEST_SINGLE_MODE_REQ)
            pEffReq = generateEffectRequest(i, TEST_REQ_TYPE);
        else
        {
            if(i%3 == 0)
                pEffReq = generateEffectRequest(i, eREQTYPE_PV);
            else if(i%3 == 1)
                pEffReq = generateEffectRequest(i, eREQTYPE_VR);
            else
                pEffReq = generateEffectRequest(i, eREQTYPE_CAP);

        }
        vEffectReqVec.push_back(pEffReq);
    }
    MY_LOGD("EffectHal configure and start...");
    // configure and start effectHal
    pEffectHal->configure();
    pEffectHal->start();
    // test normal request
    for(int i=0;i<vEffectReqVec.size();i++)
    {
        sp<DualFeatureRequest> pReq = vEffectReqVec[i];
        MY_LOGD("DepthMapEffectHal updateEffectRequest: %d", i);
        pEffectHal->updateEffectRequest(pReq.get());
    }
    MY_LOGD("EffectHal start to wait!!....");
    bool bRet = WaitingListener::waitRequestAtLeast(targetTime,5, 1);
    MY_LOGD("Wait done!!....");
    EXPECT_TRUE(bRet);

    WaitingListener::resetCounter();
}

//TEST(DepthMapEffectHalTest_BM, Standard)
void test()
{
    if(BAYER_MONO_TEST_TIME == 0)
        return;
    UTEnvironmenSetup::Profile profile = {eRatio_4_3, STEREO_SENSOR_PROFILE_REAR_REAR,
                                            E_STEREO_FEATURE_VSDOF, BAYER_AND_MONO};
    UTEnvironmenSetup setup("DualFeatureHal", profile);

    if(!setup.isReadyToUT())
    {
        EXPECT_TRUE(0);
        return;
    }

    MY_LOGD("Create DualFeatureEffectHalTest...");
    DualFeatureSettings setting;
    // main1/main2 sensor index
    int32_t main1Idx, main2Idx;
    StereoSettingProvider::getStereoSensorIndex(main1Idx, main2Idx);
    setting.miSensorIdx_Main1 = main1Idx;
    setting.miSensorIdx_Main2 = main2Idx;
    setting.mSensorType = v1::Stereo::BAYER_AND_BAYER;
    setting.mMode = E_STEREO_FEATURE_VSDOF;
    setting.mszRRZO_Main1 = getRRZOMain1Size();
    // init hal
    DualFeatureEffectHal *pEffectHal = new DualFeatureEffectHal(setting);
    pEffectHal->init();
    pEffectHal->configure();
    pEffectHal->start();

    MY_LOGD("Prepare EffectRequests...");
    std::vector<sp<DualFeatureRequest>> vEffectReqVec;
    int targetTime  = BAYER_MONO_TEST_TIME;
    for(int i=0;i<targetTime;i++)
    {
        sp<DualFeatureRequest> pEffReq;

        if(TEST_SINGLE_MODE_REQ)
            pEffReq = generateEffectRequest(i, TEST_REQ_TYPE);
        else
        {
            if(i%3 == 0)
                pEffReq = generateEffectRequest(i, eREQTYPE_PV);
            else if(i%3 == 1)
                pEffReq = generateEffectRequest(i, eREQTYPE_VR);
            else
                pEffReq = generateEffectRequest(i, eREQTYPE_CAP);

        }
        vEffectReqVec.push_back(pEffReq);
    }
    MY_LOGD("DepthMapEffectHal configure and start...");
    // configure and start effectHal
    pEffectHal->configure();
    pEffectHal->start();
    // test normal request
    for(int i=0;i<vEffectReqVec.size();i++)
    {
        sp<DualFeatureRequest> pReq = vEffectReqVec[i];
        MY_LOGD("DepthMapEffectHal updateEffectRequest: %d", i);
        pEffectHal->updateEffectRequest(pReq.get());
    }
    MY_LOGD("DepthMapEffectHal start to wait!!....");
    bool bRet = WaitingListener::waitRequestAtLeast(targetTime,5, 1);
    MY_LOGD("Wait done!!....");
    EXPECT_TRUE(bRet);

    WaitingListener::resetCounter();
}

} //NSCamFeature
} // NSCam
