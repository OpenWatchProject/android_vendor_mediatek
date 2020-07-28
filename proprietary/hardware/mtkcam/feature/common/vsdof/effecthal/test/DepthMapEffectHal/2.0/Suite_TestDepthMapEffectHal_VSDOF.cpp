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
#include <mtkcam/feature/stereo/effecthal/DepthMapEffectHal.h>
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
#include <mtkcam/feature/stereo/StereoCamEnum.h>

#include "TestDepthMap_Common.h"
#include "ImgParamSetting.h"

using android::sp;
using android::String8;
using namespace NSCam;
using namespace NSCam::NSCamFeature::NSFeaturePipe_DepthMap;
using namespace NSCam::v1::Stereo;

namespace NSCam{
namespace NSCamFeature{

#define BAYER_BAYER_TEST_TIME 0
#define BAYER_MONO_TEST_TIME 1
#define TEST_FLOW_TYPE eDEPTH_FLOW_TYPE_STANDARD

#define TEST_SINGLE_MODE_REQ 1
#define TEST_REQ_TYPE eREQTYPE_CAP

typedef IImageBufferAllocator::ImgParam ImgParam;
StereoSizeProvider* gpSizePrvder = StereoSizeProvider::getInstance();

sp<IDepthMapEffectRequest> generateEffectRequest(int request_id, TestReqType eState)
{
    MY_LOGD("+: reqID = %d, request state=%d", request_id, eState);

    sp<IDepthMapEffectRequest> pRequest =  IDepthMapEffectRequest::createInstance(request_id, WaitingListener::CB);

    prepareReqInputBuffer(pRequest);
    prepareReqOutputBuffer(eState, pRequest);
    MBOOL bEISOn = (eState == eREQTYPE_VR) ? MTRUE : MFALSE;
    setupReqMetadata(bEISOn, pRequest);

    MY_LOGD("-");
    return pRequest;
}

MVOID setupDumpProperties()
{
    #define SET_ID2NAME_PROB(name, ID) \
        snprintf(prop_name, 256, "%s.%s", name, #ID); \
        property_set(prop_name, "1");

    char prop_name[256];
    // P2AFM Node section
    char* name = "vendor.debug.P2AFM";
    property_set(name, "1");
    SET_ID2NAME_PROB(name, P2AFM_TO_N3D_FEFM_CCin);
    SET_ID2NAME_PROB(name, P2AFM_TO_FD_IMG);
    SET_ID2NAME_PROB(name, P2AFM_TO_WMF_MY_SL);
    SET_ID2NAME_PROB(name, P2AFM_OUT_MV_F);
    SET_ID2NAME_PROB(name, P2AFM_OUT_FD);
    SET_ID2NAME_PROB(name, P2AFM_OUT_MV_F_CAP);
    SET_ID2NAME_PROB(name, UT_OUT_FE);
    //N3D Node section
    name = "vendor.debug.N3DNode";
    property_set(name, "1");
    SET_ID2NAME_PROB(name, N3D_TO_DPE_MVSV_MASK);
    SET_ID2NAME_PROB(name, N3D_TO_OCC_LDC);
    SET_ID2NAME_PROB(name, N3D_TO_FD_EXTRADATA_MASK);
    SET_ID2NAME_PROB(name, N3D_OUT_JPS_WARPING_MATRIX);
    // DPE Node section
    name = "vendor.debug.DPENode";
    property_set(name, "1");
    SET_ID2NAME_PROB(name, DPE_TO_OCC_MVSV_DMP_CFM);
    // OCC Node Section
    name = "vendor.debug.OCCNode";
    property_set(name, "1");
    SET_ID2NAME_PROB(name, OCC_TO_WMF_DMH_MY_S);
    // WMF Node section
    name = "vendor.debug.WMFNode";
    property_set(name, "1");
    SET_ID2NAME_PROB(name, WMF_TO_GF_DMW_MY_S);
    // GF Node section
    name = "vendor.debug.GFNode";
    property_set(name, "1");
    SET_ID2NAME_PROB(name, GF_OUT_DMBG);
    SET_ID2NAME_PROB(name, GF_OUT_DEPTHMAP);
    // FD Node section
    name = "vendor.debug.FDNode";
    property_set(name, "1");
    SET_ID2NAME_PROB(name, FD_OUT_EXTRADATA);

    //======== Power on sensor ==========
    NSCam::IHalSensorList* pHalSensorList = MAKE_HalSensorList();
    pHalSensorList->searchSensors();
    int sensorCount = pHalSensorList->queryNumberOfSensors();

    int main1Idx, main2Idx;
    StereoSettingProvider::getStereoSensorIndex(main1Idx, main2Idx);
    MUINT pIndex[2] = { (MUINT)main1Idx, (MUINT)main2Idx };
    MUINT const main1Index = pIndex[0];
    MUINT const main2Index = pIndex[1];
    if(!pHalSensorList)
    {
        MY_LOGE("pHalSensorList == NULL");
    }
    //
    #define USER_NAME "N3DNodeUT"
    IHalSensor *pHalSensor = pHalSensorList->createSensor(
                                            USER_NAME,
                                            2,
                                            pIndex);
    if(pHalSensor == NULL)
    {
       MY_LOGE("pHalSensor is NULL");
    }
    // In stereo mode, Main1 needs power on first.
    // Power on main1 and main2 successively one after another.
    if( !pHalSensor->powerOn(USER_NAME, 1, &main1Index) )
    {
        MY_LOGE("sensor power on failed: %d", main1Index);
    }
    if( !pHalSensor->powerOn(USER_NAME, 1, &main2Index) )
    {
        MY_LOGE("sensor power on failed: %d", main2Index);
    }
}


TEST(DepthMapEffectHalTest, Standard)
{
    if(BAYER_BAYER_TEST_TIME==0)
        return;
    // setup dump buffer properties
    setupDumpProperties();
    // config providers
    StereoSettingProvider::setStereoFeatureMode(E_STEREO_FEATURE_VSDOF);
    StereoSettingProvider::setImageRatio(eRatio_16_9);
    StereoSettingProvider::setStereoProfile(STEREO_SENSOR_PROFILE_REAR_REAR);
    StereoSettingProvider::setStereoModuleType(BAYER_AND_BAYER);

    MY_LOGD("Create DepthMapEffectHal...");
    DepthMapEffectHal* pDepthMapEffectHal = new DepthMapEffectHal();
    pDepthMapEffectHal->init();
    // main1/main2 sensor index
    int main1Idx, main2Idx;
    StereoSettingProvider::getStereoSensorIndex(main1Idx, main2Idx);
    // prepare sensor idx parameters
    sp<EffectParameter> effParam = new EffectParameter();
    effParam->set(EFFECTKEY_SENSOR_IDX_MAIN1, main1Idx);
    effParam->set(EFFECTKEY_SENSOR_IDX_MAIN2, main2Idx);
    effParam->set(EFFECTKEY_SENSOR_TYPE, (int)BAYER_AND_BAYER);
    effParam->set(EFFECTKEY_FEATURE_MODE, (int)eDEPTHNODE_MODE_VSDOF);
    effParam->set(EFFECTKEY_FLOW_TYPE, (int)TEST_FLOW_TYPE);

    pDepthMapEffectHal->setParameters(effParam);

    MY_LOGD("Prepare EffectRequests...target time:%d", BAYER_BAYER_TEST_TIME);
    std::vector<sp<IDepthMapEffectRequest>> vEffectReqVec;
    int targetTime  = BAYER_BAYER_TEST_TIME;
    for(int i=0;i<targetTime;i++)
    {
        sp<IDepthMapEffectRequest> pEffReq;
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
    pDepthMapEffectHal->configure();
    pDepthMapEffectHal->start();


    // test normal request
    for(int i=0;i<vEffectReqVec.size();i++)
    {
        sp<IDepthMapEffectRequest> pReq = vEffectReqVec[i];
        MY_LOGD("DepthMapEffectHal updateEffectRequest: %d", i);
        pDepthMapEffectHal->updateEffectRequest(pReq.get());
    }
    MY_LOGD("DepthMapEffectHal start to wait!!....");
    bool bRet = WaitingListener::waitRequestAtLeast(targetTime,5, 1);
    MY_LOGD("Wait done!!....");
    EXPECT_TRUE(bRet);

    WaitingListener::resetCounter();
}

TEST(DepthMapEffectHalTest_BM, Standard)
{
    if(BAYER_MONO_TEST_TIME == 0)
        return;
    // setup dump buffer properties
    setupDumpProperties();
    // config providers
    StereoSettingProvider::setStereoFeatureMode(E_STEREO_FEATURE_VSDOF);
    StereoSettingProvider::setImageRatio(eRatio_16_9);
    StereoSettingProvider::setStereoProfile(STEREO_SENSOR_PROFILE_REAR_FRONT);
    StereoSettingProvider::setStereoModuleType(BAYER_AND_BAYER);

    MY_LOGD("Create DepthMapEffectHal Bayer+Mono... time:%d", BAYER_MONO_TEST_TIME);
    DepthMapEffectHal* pDepthMapEffectHal = new DepthMapEffectHal();
    pDepthMapEffectHal->init();
    // main1/main2 sensor index
    int main1Idx, main2Idx;
    StereoSettingProvider::getStereoSensorIndex(main1Idx, main2Idx);
    // prepare sensor idx parameters
    sp<EffectParameter> effParam = new EffectParameter();
    effParam->set(EFFECTKEY_SENSOR_IDX_MAIN1, main1Idx);
    effParam->set(EFFECTKEY_SENSOR_IDX_MAIN2, main2Idx);
    effParam->set(EFFECTKEY_SENSOR_TYPE, (int)BAYER_AND_MONO);
    effParam->set(EFFECTKEY_FEATURE_MODE, (int)eDEPTHNODE_MODE_VSDOF);
    effParam->set(EFFECTKEY_FLOW_TYPE, (int)TEST_FLOW_TYPE);

    int aaa = effParam->getInt(EFFECTKEY_SENSOR_TYPE);
    pDepthMapEffectHal->setParameters(effParam);

    MY_LOGD("Prepare EffectRequests...");
    std::vector<sp<IDepthMapEffectRequest>> vEffectReqVec;
    int targetTime  = BAYER_MONO_TEST_TIME;
    for(int i=0;i<targetTime;i++)
    {
        sp<IDepthMapEffectRequest> pEffReq;

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
    pDepthMapEffectHal->configure();
    pDepthMapEffectHal->start();
    // test normal request
    for(int i=0;i<vEffectReqVec.size();i++)
    {
        sp<IDepthMapEffectRequest> pReq = vEffectReqVec[i];
        MY_LOGD("DepthMapEffectHal updateEffectRequest: %d", i);
        pDepthMapEffectHal->updateEffectRequest(pReq.get());
    }
    MY_LOGD("DepthMapEffectHal start to wait!!....");
    bool bRet = WaitingListener::waitRequestAtLeast(targetTime,5, 1);
    MY_LOGD("Wait done!!....");
    EXPECT_TRUE(bRet);

    WaitingListener::resetCounter();
}

} //NSCamFeature
} // NSCam
