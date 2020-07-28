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

using android::sp;
using android::String8;
using namespace NSCam;
using namespace NSCam::NSCamFeature::NSFeaturePipe_DepthMap;

namespace NSCam{
namespace NSCamFeature{

#define BAYER_BAYER_TEST_TIME 0
#define BAYER_MONO_TEST_TIME 4

typedef IImageBufferAllocator::ImgParam ImgParam;
StereoSizeProvider* gpSizePrvder = StereoSizeProvider::getInstance();


MVOID loadFullResizRawsImgBuf(
    sp<IImageBuffer>& rpFSMain1ImgBuf,
    sp<IImageBuffer>& rpFSMain2ImgBuf,
    sp<IImageBuffer>& rpRSMain1ImgBuf,
    sp<IImageBuffer>& rpRSMain2ImgBuf
)
{
    const char * sMain1Filename = "/sdcard/vsdof/BID_P2A_IN_RSRAW1_2100x1180_3944.raw";
    const char * sMain2Filename = "/sdcard/vsdof/BID_P2A_IN_RSRAW2_1280x720_2400.raw";

    const char * sMain1FSFilename = "/sdcard/vsdof/BID_P2A_IN_FSRAW1_4208x3120_5260.raw";
    const char * sMain2FSFilename = "/sdcard/vsdof/BID_P2A_IN_FSRAW2_4208x3120_5260.raw";


    MUINT32 bufStridesInBytes[3] = {3944, 0, 0};
    MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
    IImageBufferAllocator::ImgParam imgParamRRZO_Main1 = IImageBufferAllocator::ImgParam((eImgFmt_FG_BAYER10), MSize(2100, 1180), bufStridesInBytes, bufBoundaryInBytes, 1);

    MUINT32 bufStridesInBytes2[3] = {2400, 0, 0};
    IImageBufferAllocator::ImgParam imgParamRRZO_Main2 = IImageBufferAllocator::ImgParam((eImgFmt_FG_BAYER10), MSize(1280, 720), bufStridesInBytes2, bufBoundaryInBytes, 1);

    MUINT32 bufStridesInBytesFS[3] = {5260, 0, 0};
    MINT32 bufBoundaryInBytesFS[3] = {0, 0, 0};
    IImageBufferAllocator::ImgParam imgParamIMGO = IImageBufferAllocator::ImgParam((eImgFmt_BAYER10), MSize(4208, 3120), bufStridesInBytesFS, bufBoundaryInBytesFS, 1);

    rpRSMain1ImgBuf = createImageBufferFromFile(imgParamRRZO_Main1, sMain1Filename, "RsRaw1",  eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
    rpRSMain2ImgBuf = createImageBufferFromFile(imgParamRRZO_Main2, sMain2Filename, "RsRaw2",  eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
    rpFSMain1ImgBuf = createImageBufferFromFile(imgParamIMGO, sMain1FSFilename, "FsRaw",  eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
    rpFSMain2ImgBuf = createImageBufferFromFile(imgParamIMGO, sMain2FSFilename, "FsRaw2",  eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
}

#define USE_FD 1

sp<IDepthMapEffectRequest> generateEffectRequest(int request_id, DepthMapPipeOpState eState)
{
    MY_LOGD("+: reqID = %d, request state=%d", request_id, eState);

    sp<IDepthMapEffectRequest> pEffectReq =  IDepthMapEffectRequest::createInstance(request_id, WaitingListener::CB);

    sp<IImageBuffer> buf_fs_Main1, buf_fs_Main2, buf_rs_Main1, buf_rs_Main2, buf;
    // load resize raw
    loadFullResizRawsImgBuf(buf_fs_Main1, buf_fs_Main2, buf_rs_Main1, buf_rs_Main2);


    // prepare input frame info: RSRAW1
    pEffectReq->pushRequestImageBuffer({BID_P2A_IN_RSRAW1, eBUFFER_IOTYPE_INPUT}, buf_rs_Main1);

    // prepare input frame info: RSRAW2
    pEffectReq->pushRequestImageBuffer({BID_P2A_IN_RSRAW2, eBUFFER_IOTYPE_INPUT}, buf_rs_Main2);

    // prepare input frame info: FSRAW1
    pEffectReq->pushRequestImageBuffer({BID_P2A_IN_FSRAW1, eBUFFER_IOTYPE_INPUT}, buf_fs_Main1);

    // prepare input frame info: FSRAW2
    pEffectReq->pushRequestImageBuffer({BID_P2A_IN_FSRAW2, eBUFFER_IOTYPE_INPUT}, buf_fs_Main2);

    IMetadata* pMetadata;
    // prepare the metadata frame info
    // InAppMeta
    pMetadata = new IMetadata();
    pEffectReq->pushRequestMetadata({BID_META_IN_APP, eBUFFER_IOTYPE_INPUT}, pMetadata);
    // InAppMeta - EIS on
    trySetMetadata<MUINT8>(pMetadata, MTK_CONTROL_VIDEO_STABILIZATION_MODE,
    MTK_CONTROL_VIDEO_STABILIZATION_MODE_ON);
    trySetMetadata<MINT32>(pMetadata, MTK_JPEG_ORIENTATION, 0);
    trySetMetadata<MUINT8>(pMetadata, MTK_CONTROL_AF_TRIGGER, 0);
    trySetMetadata<MINT32>(pMetadata, MTK_STEREO_FEATURE_DOF_LEVEL, 0);
    // InHalMeta Main1
    pMetadata = new IMetadata();
    pEffectReq->pushRequestMetadata({BID_META_IN_HAL_MAIN1, eBUFFER_IOTYPE_INPUT}, pMetadata);
    // InHalMeta-EIS region
    IMetadata::IEntry entry(MTK_EIS_REGION);
    entry.push_back(0, Type2Type< MINT32 >());
    entry.push_back(0, Type2Type< MINT32 >());
    entry.push_back(0, Type2Type< MINT32 >());
    entry.push_back(0, Type2Type< MINT32 >());
    entry.push_back(1920, Type2Type< MINT32 >());
    entry.push_back(1080, Type2Type< MINT32 >());
    // the following is  MVtoCenterX, MVtoCenterY, IsFromRRZ
    entry.push_back(0, Type2Type< MINT32 >());
    entry.push_back(0, Type2Type< MINT32 >());
    entry.push_back(MTRUE, Type2Type< MBOOL >());
    pMetadata->update(MTK_EIS_REGION, entry);
    // magic num
    trySetMetadata<MINT32>(pMetadata, MTK_P1NODE_PROCESSOR_MAGICNUM, 0);
    // sensor mode
    trySetMetadata<MINT32>(pMetadata, MTK_P1NODE_SENSOR_MODE, SENSOR_SCENARIO_ID_NORMAL_PREVIEW);
    // conv offset
    trySetMetadata<MFLOAT>(pMetadata, MTK_CONVERGENCE_DEPTH_OFFSET, 0);

    // InHalMain2
    pMetadata = new IMetadata();
    trySetMetadata<MINT32>(pMetadata, MTK_P1NODE_PROCESSOR_MAGICNUM, 0);
    pEffectReq->pushRequestMetadata({BID_META_IN_HAL_MAIN2, eBUFFER_IOTYPE_INPUT}, pMetadata);
    // OutAppMeta BID_META_OUT_APP
    pMetadata = new IMetadata();
    pEffectReq->pushRequestMetadata({BID_META_OUT_APP, eBUFFER_IOTYPE_OUTPUT}, pMetadata);
    // OutHalMeta
    pMetadata = new IMetadata();
    pEffectReq->pushRequestMetadata({BID_META_OUT_HAL, eBUFFER_IOTYPE_OUTPUT}, pMetadata);

    // prepare output frame: MV_F
    ImgParam imgParam_MainImageCAP = getImgParam_MV_F_CAP();
    ImgParam imgParam_MainImage = getImgParam_MV_F();

    if(eState == eSTATE_CAPTURE)
    {
        buf = createEmptyImageBuffer(imgParam_MainImageCAP, "MainImg",  eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
        pEffectReq->pushRequestImageBuffer({BID_P2A_OUT_MV_F_CAP, eBUFFER_IOTYPE_OUTPUT},buf);

        // prepare output frame: DepthMap
        ImgParam imgParam_DEPTHMAP = getImgParam_DEPTHMAP();
        buf = createEmptyImageBuffer(imgParam_DEPTHMAP, "DepthMap", eBUFFER_USAGE_HW_CAMERA_READWRITE|eBUFFER_USAGE_SW_READ_OFTEN|eBUFFER_USAGE_SW_WRITE_OFTEN);
        pEffectReq->pushRequestImageBuffer({BID_GF_OUT_DEPTHMAP, eBUFFER_IOTYPE_OUTPUT},buf);

        // prepare output frame: JPSMain1
        ImgParam imgParam_JPSMain1 = getImgParam_JPSMain();
        buf = createEmptyImageBuffer(imgParam_JPSMain1, "JPSMain1", eBUFFER_USAGE_HW_CAMERA_READWRITE|eBUFFER_USAGE_SW_READ_OFTEN|eBUFFER_USAGE_SW_WRITE_OFTEN);
        pEffectReq->pushRequestImageBuffer({BID_N3D_OUT_JPS_MAIN1, eBUFFER_IOTYPE_OUTPUT},buf);

        buf = createEmptyImageBuffer(imgParam_JPSMain1, "JPSMain2", eBUFFER_USAGE_HW_CAMERA_READWRITE|eBUFFER_USAGE_SW_READ_OFTEN|eBUFFER_USAGE_SW_WRITE_OFTEN);
        pEffectReq->pushRequestImageBuffer({BID_N3D_OUT_JPS_MAIN2, eBUFFER_IOTYPE_OUTPUT},buf);

        // LDC
        ImgParam imgParam_LDC = getImgParam_LDC();
        buf = createEmptyImageBuffer(imgParam_LDC, "LDCBuf", eBUFFER_USAGE_SW_READ_OFTEN|eBUFFER_USAGE_SW_WRITE_OFTEN);
        pEffectReq->pushRequestImageBuffer({BID_N3D_OUT_LDC, eBUFFER_IOTYPE_OUTPUT},buf);

    }
    else
    {
        buf = createEmptyImageBuffer(imgParam_MainImage, "MainImg",  eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
        pEffectReq->pushRequestImageBuffer({BID_P2A_OUT_MV_F, eBUFFER_IOTYPE_OUTPUT},buf);
    }

    // prepare output frame: FD
    if(USE_FD)
    {
        ImgParam imgParam_FD = getImgParam_FD();
        buf = createEmptyImageBuffer(imgParam_FD, "FD", eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE |  eBUFFER_USAGE_SW_WRITE_RARELY);
        pEffectReq->pushRequestImageBuffer({BID_P2A_OUT_FDIMG, eBUFFER_IOTYPE_OUTPUT},buf);
    }

    // prepare output frame: DMBG
    ImgParam imgParam_DMBG = getImgParam_DMBG();
    buf = createEmptyImageBuffer(imgParam_DMBG, "imgParam_DMBG", eBUFFER_USAGE_HW_CAMERA_READWRITE|eBUFFER_USAGE_SW_READ_OFTEN|eBUFFER_USAGE_SW_WRITE_OFTEN);
    pEffectReq->pushRequestImageBuffer({BID_GF_OUT_DMBG, eBUFFER_IOTYPE_OUTPUT},buf);


    MY_LOGD("-");
    return pEffectReq;
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
    pDepthMapEffectHal->setParameters(effParam);

    MY_LOGD("Prepare EffectRequests...target time:%d", BAYER_BAYER_TEST_TIME);
    std::vector<sp<IDepthMapEffectRequest>> vEffectReqVec;
    int targetTime  = BAYER_BAYER_TEST_TIME;
    for(int i=0;i<targetTime;i++)
    {
        sp<IDepthMapEffectRequest> pEffReq;
        if(i%2 == 0)
            pEffReq = generateEffectRequest(i, eSTATE_NORMAL);
        else
            pEffReq = generateEffectRequest(i, eSTATE_CAPTURE);
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

    int aaa = effParam->getInt(EFFECTKEY_SENSOR_TYPE);
    pDepthMapEffectHal->setParameters(effParam);

    MY_LOGD("Prepare EffectRequests...");
    std::vector<sp<IDepthMapEffectRequest>> vEffectReqVec;
    int targetTime  = BAYER_MONO_TEST_TIME;
    for(int i=0;i<targetTime;i++)
    {
        sp<IDepthMapEffectRequest> pEffReq;
        if(i%2 == 0)
            pEffReq = generateEffectRequest(i, eSTATE_NORMAL);
        else
            pEffReq = generateEffectRequest(i, eSTATE_NORMAL);
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
