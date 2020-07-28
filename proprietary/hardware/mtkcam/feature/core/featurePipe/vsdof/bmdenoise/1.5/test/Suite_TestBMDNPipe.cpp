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
#include "TestBMDeNoise_Common.h"

using android::sp;
using android::String8;
using namespace NSCam;

namespace NSCam{
namespace NSCamFeature{
namespace NSFeaturePipe{

#define USER_NAME "BMDNPipeUT"

typedef IImageBufferAllocator::ImgParam ImgParam;
StereoSizeProvider* gpSizePrvder = StereoSizeProvider::getInstance();

#define DECLARE_MACRO()\
    sp<EffectFrameInfo> effectFrame;\
    MUINT32 reqIdx;

#define ADD_METABUF_FRAME(effectReq, frameMapName, frameId, rpMetaBuf) \
    rpMetaBuf = new IMetadata();\
    reqIdx = effectReq->getRequestNo();\
    effectFrame = new EffectFrameInfo(reqIdx, frameId); \
    {\
        sp<EffectParameter> effParam = new EffectParameter(); \
        effParam->setPtr(BMDENOISE_EFFECT_PARAMS_KEY, (void*)rpMetaBuf); \
        effectFrame->setFrameParameter(effParam); \
        effectReq->frameMapName.add(frameId, effectFrame); \
    }

#define ADD_METABUF_INPUT_FRAME(effectReq, frameId, rpMetaBuf)  \
    ADD_METABUF_FRAME(effectReq, vInputFrameInfo, frameId, rpMetaBuf)

#define ADD_METABUF_OUTPUT_FRAME(effectReq, frameId, rpMetaBuf)  \
    ADD_METABUF_FRAME(effectReq, vOutputFrameInfo, frameId, rpMetaBuf)

#define MY_LOGGER(fmt, arg...) \
    CAM_LOGD("[%s][%s]" fmt, USER_NAME, __func__, ##arg); \
    printf("[D][%s][%s]" fmt"\n", USER_NAME, __func__, ##arg);

MVOID loadImgBuf(
    sp<IImageBuffer>& rpFSMain1ImgBuf,
    sp<IImageBuffer>& rpFSMain2ImgBuf,
    sp<IImageBuffer>& rpRSMain1ImgBuf,
    sp<IImageBuffer>& rpRSMain2ImgBuf
    )
{
    const char * sMain1FSFilename = "/sdcard/bmdn/test/BID_P2A_IN_FSRAW1_4208x3120_5260.raw";
    const char * sMain2FSFilename = "/sdcard/bmdn/test/BID_P2A_IN_FSRAW1_4208x3120_5260.raw";
    const char * sMain1RSFilename = "/sdcard/bmdn/test/BID_P2A_IN_RSRAW1_2104x1560_3952.raw";
    const char * sMain2RSFilename = "/sdcard/bmdn/test/BID_P2A_IN_RSRAW1_2104x1560_3952.raw";

    {
        MUINT32 bufStridesInBytes[3] = {5260, 0, 0};
        MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
        IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam((eImgFmt_BAYER10), MSize(4208, 3120), bufStridesInBytes, bufBoundaryInBytes, 1);

        rpFSMain1ImgBuf = createImageBufferFromFile(imgParam, sMain1FSFilename, "FSRaw1",  eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
        EXPECT_TRUE(rpFSMain1ImgBuf != nullptr)<< "rpFSMain1ImgBuf != nullptr";
    }

    {
        MUINT32 bufStridesInBytes[3] = {5260, 0, 0};
        MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
        IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam((eImgFmt_BAYER10), MSize(4208, 3120), bufStridesInBytes, bufBoundaryInBytes, 1);

        rpFSMain2ImgBuf = createImageBufferFromFile(imgParam, sMain2FSFilename, "FSRaw2",  eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
        EXPECT_TRUE(rpFSMain2ImgBuf != nullptr)<< "rpFSMain2ImgBuf != nullptr";
    }

    {
        MUINT32 bufStridesInBytes[3] = {3952, 0, 0};
        MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
        IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam((eImgFmt_BAYER10), MSize(2104, 1560), bufStridesInBytes, bufBoundaryInBytes, 1);

        rpRSMain1ImgBuf = createImageBufferFromFile(imgParam, sMain1RSFilename, "RSRaw1",  eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
        EXPECT_TRUE(rpRSMain1ImgBuf != nullptr)<< "rpRSMain1ImgBuf != nullptr";
    }

    {
        MUINT32 bufStridesInBytes[3] = {3952, 0, 0};
        MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
        IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam((eImgFmt_BAYER10), MSize(2104, 1560), bufStridesInBytes, bufBoundaryInBytes, 1);

        rpRSMain2ImgBuf = createImageBufferFromFile(imgParam, sMain2RSFilename, "RSRaw2",  eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
        EXPECT_TRUE(rpRSMain2ImgBuf != nullptr)<< "rpRSMain2ImgBuf != nullptr";
    }

    // create a fake SceneInfo
#if 0
    10-26 17:01:31.122417   446  3414 D BMDeNoise [RootNode:: getSceneInfo]scene_info[0]:0
    10-26 17:01:31.122905   446  3414 D BMDeNoise [RootNode:: getSceneInfo]scene_info[1]:1
    10-26 17:01:31.122951   446  3414 D BMDeNoise [RootNode:: getSceneInfo]scene_info[2]:1
    10-26 17:01:31.122989   446  3414 D BMDeNoise [RootNode:: getSceneInfo]scene_info[3]:23
    10-26 17:01:31.123028   446  3414 D BMDeNoise [RootNode:: getSceneInfo]scene_info[4]:42395
    10-26 17:01:31.123067   446  3414 D BMDeNoise [RootNode:: getSceneInfo]scene_info[5]:42263
    10-26 17:01:31.123106   446  3414 D BMDeNoise [RootNode:: getSceneInfo]scene_info[6]:36448
    10-26 17:01:31.123146   446  3414 D BMDeNoise [RootNode:: getSceneInfo]scene_info[7]:64
    10-26 17:01:31.123224   446  3414 D BMDeNoise [RootNode:: getSceneInfo]scene_info[8]:62
    10-26 17:01:31.123265   446  3414 D BMDeNoise [RootNode:: getSceneInfo]scene_info[9]:56
    10-26 17:01:31.123303   446  3414 D BMDeNoise [RootNode:: getSceneInfo]scene_info[10]:0
    10-26 17:01:31.123344   446  3414 D BMDeNoise [RootNode:: getSceneInfo]scene_info[11]:100
    10-26 17:01:31.123381   446  3414 D BMDeNoise [RootNode:: getSceneInfo]scene_info[12]:100
    10-26 17:01:31.123421   446  3414 D BMDeNoise [RootNode:: getSceneInfo]scene_info[13]:0
    10-26 17:01:31.123460   446  3414 D BMDeNoise [RootNode:: getSceneInfo]scene_info[14]:293
    10-26 17:01:31.123705   446  3414 W BMDeNoise [RootNode:: getSceneInfoType]scene_info[0], decision confidence[1], use new decision:1
    {
        // 15 int elements => 15X4  = 60
        MSize size(60, 1);
        int bufStridesInBytes[1] = {size.w};
        MINT32 bufBoundaryInBytes[1] = {0};
        MUINT32 usage = eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN;
        IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam((eImgFmt_Y8), size, bufStridesInBytes, bufBoundaryInBytes, 1);

        rpSceneInfoImgBuf = createEmptyImageBuffer(imgParam, "sceneInfo",  usage);
        EXPECT_TRUE(rpSceneInfoImgBuf != nullptr)<< "rpSceneInfoImgBuf != nullptr";

        rpSceneInfoImgBuf->lockBuf(LOG_TAG, usage);
        int* sceneInfo = (int*)rpSceneInfoImgBuf->getBufVA(0);
        sceneInfo[0] = 0;
        sceneInfo[1] = 1;
        sceneInfo[2] = 1;
        sceneInfo[3] = 23;
        sceneInfo[4] = 42395;
        sceneInfo[5] = 42263;
        sceneInfo[6] = 36448;
        sceneInfo[7] = 64;
        sceneInfo[8] = 62;
        sceneInfo[9] = 56;
        sceneInfo[10] = 0;
        sceneInfo[11] = 100;
        sceneInfo[12] = 100;
        sceneInfo[13] = 0;
        sceneInfo[14] = 293;
        rpSceneInfoImgBuf->unlockBuf(LOG_TAG);
    }
#endif
}

sp<PipeRequest> generatePipeRequest(int request_id)
{
    MY_LOGD("+: reqID = %d", request_id);

    sp<PipeRequest> spPipeReq = new PipeRequest(request_id, WaitingListener::CB);

    sp<IImageBuffer> buf_fs_Main1;
    sp<IImageBuffer> buf_fs_Main2;
    sp<IImageBuffer> buf_rs_Main1;
    sp<IImageBuffer> buf_rs_Main2;

    // load input image buffers
    loadImgBuf(buf_fs_Main1, buf_fs_Main2, buf_rs_Main1, buf_rs_Main2);
    MY_LOGD("buf_fs_Main1=%p, buf_fs_Main2=%p, buf_rs_Main1=%p, buf_rs_Main2=%p",
        buf_fs_Main1.get(),
        buf_fs_Main2.get(),
        buf_rs_Main1.get(),
        buf_rs_Main2.get()
    );

    EXPECT_EQ(true, buf_fs_Main1 != nullptr) << "buf_fs_Main1";
    EXPECT_EQ(true, buf_fs_Main2 != nullptr) << "buf_fs_Main2";
    EXPECT_EQ(true, buf_rs_Main1 != nullptr) << "buf_rs_Main1";
    EXPECT_EQ(true, buf_rs_Main2 != nullptr) << "buf_rs_Main2";

    spPipeReq->addImageBuffer(BID_INPUT_FSRAW_1, buf_fs_Main1);
    spPipeReq->addImageBuffer(BID_INPUT_FSRAW_2, buf_fs_Main2);
    spPipeReq->addImageBuffer(BID_INPUT_RSRAW_1, buf_rs_Main1);
    spPipeReq->addImageBuffer(BID_INPUT_RSRAW_2, buf_rs_Main2);

    // prepare metadatas
    IMetadata* pMetadata = new IMetadata();
    // update iso meta
    {
        IMetadata exifMeta;
        IMetadata::IEntry entry_iso(MTK_3A_EXIF_AE_ISO_SPEED);
        entry_iso.push_back(1600, Type2Type< MINT32 >());
        exifMeta.update(entry_iso.tag(), entry_iso);

        trySetMetadata<IMetadata>(pMetadata, MTK_3A_EXIF_METADATA, exifMeta);
    }
    // update jpeg rotation
    trySetMetadata<MINT32>(pMetadata, MTK_JPEG_ORIENTATION, 270);

    // magic num
    trySetMetadata<MINT32>(pMetadata, MTK_P1NODE_PROCESSOR_MAGICNUM, 0);


    spPipeReq->addMetadata(BID_META_IN_APP, pMetadata);
    spPipeReq->addMetadata(BID_META_IN_HAL, pMetadata);
    spPipeReq->addMetadata(BID_META_IN_HAL_MAIN2, pMetadata);

    // prepare output buffer
    {
        ImgParam imgParam= getImgParam_out_YUV();
        sp<IImageBuffer> pBuf = createEmptyImageBuffer(imgParam, "BID_JPEG_YUV",  eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
        spPipeReq->addImageBuffer(BID_JPEG_YUV, pBuf);
    }
    {
        ImgParam imgParam = getImgParam_out_Thumb();
        sp<IImageBuffer> pBuf = createEmptyImageBuffer(imgParam, "BID_THUMB_YUV",  eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
        spPipeReq->addImageBuffer(BID_THUMB_YUV, pBuf);
    }

    MY_LOGD("-: reqID = %d", request_id);
    return spPipeReq;
}

sp<PipeRequest> generatePipeRequest_HR(int request_id, int MFHRShot_req_id, bool isMain)
{
    MY_LOGD("+: reqID = %d,%d,%d", request_id, MFHRShot_req_id, isMain);
    // generate basic id
    sp<PipeRequest> pReq = generatePipeRequest(request_id);

    // add MFHR params
    pReq->addParam(BMDeNoiseParamID::PID_MFHR_REQ_ID, MFHRShot_req_id);

    // main req or not
    if(isMain){
        pReq->addParam(BMDeNoiseParamID::PID_MFHR_IS_MAIN, 1);
    }else{
        // remove output buffers
        pReq->removeImageBuffer(BID_JPEG_YUV);
        pReq->removeImageBuffer(BID_THUMB_YUV);
    }

    MY_LOGD("-: reqID = %d,%d,%d", request_id, MFHRShot_req_id, isMain);
    return pReq;
}

IHalSensor* HalSensorInit(){
    NSCam::IHalSensorList* pHalSensorList = NSCam::IHalSensorList::get();
    pHalSensorList->searchSensors();
    int sensorCount = pHalSensorList->queryNumberOfSensors();

    int main1Idx, main2Idx;
    StereoSettingProvider::getStereoSensorIndex(main1Idx, main2Idx);
    MUINT pIndex[2] = { (MUINT)main1Idx, (MUINT)main2Idx };
    MUINT const main1Index = pIndex[0];
    MUINT const main2Index = pIndex[1];
    if(!pHalSensorList)
    {
        MY_LOGGER("pHalSensorList == NULL");
    }
    IHalSensor *pHalSensor = pHalSensorList->createSensor(
                                            USER_NAME,
                                            2,
                                            pIndex);
    if(pHalSensor == NULL)
    {
       MY_LOGGER("pHalSensor is NULL");
    }
    // In stereo mode, Main1 needs power on first.
    // Power on main1 and main2 successively one after another.
    if( !pHalSensor->powerOn(USER_NAME, 1, &main1Index) )
    {
        MY_LOGGER("sensor power on failed: %d", main1Index);
    }
    if( !pHalSensor->powerOn(USER_NAME, 1, &main2Index) )
    {
        MY_LOGGER("sensor power on failed: %d", main2Index);
    }

    MY_LOGGER("sensorCount:%d", sensorCount);
    return pHalSensor;
}

void HalSensorUninit(IHalSensor* pHalSensor){

    int main1Idx, main2Idx;
    StereoSettingProvider::getStereoSensorIndex(main1Idx, main2Idx);
    MUINT pIndex[2] = { (MUINT)main1Idx, (MUINT)main2Idx };
    MUINT const main1Index = pIndex[0];
    MUINT const main2Index = pIndex[1];
    //

    MY_LOGGER("PowerOff Main1");
    MUINT pIndex_main1[1] = { (MUINT)main1Index};
    pHalSensor->powerOff(USER_NAME, 1, pIndex_main1);

    MY_LOGGER("PowerOff Main2");
    MUINT pIndex_main2[1] = { (MUINT)main2Index};
    pHalSensor->powerOff(USER_NAME, 1, pIndex_main2);

    pHalSensor->destroyInstance(USER_NAME);
}

void cam1DeviceInit(){
    MY_LOGGER("StereoSettingProvider::setStereoProfile(STEREO_SENSOR_PROFILE_REAR_REAR);")
    StereoSettingProvider::setStereoProfile(STEREO_SENSOR_PROFILE_REAR_REAR);
}

void flowControlInit(){

    StereoSettingProvider::setImageRatio(eRatio_Default);

    StereoSettingProvider::setStereoFeatureMode(v1::Stereo::E_STEREO_FEATURE_DENOISE);

    StereoSettingProvider::setStereoModuleType(v1::Stereo::BAYER_AND_MONO);
}

TEST(BMDN, BasicTest)
{
    MY_LOGGER("BMDNBasicTest +");

    IHalSensor* pHalSensor = HalSensorInit();

    cam1DeviceInit();

    flowControlInit();

    int main1Idx, main2Idx;
    StereoSettingProvider::getStereoSensorIndex(main1Idx, main2Idx);

    MY_LOGGER("getStereoSensorIndex: (%d, %d)", main1Idx, main2Idx);

    MY_LOGGER("Create BMDeNoisePipe");
    sp<IBMDeNoisePipe> spPipe = IBMDeNoisePipe::createInstance(
        main1Idx, main2Idx,
        BMDeNoiseFeatureType::TYPE_BMDN
    );
    spPipe->setFlushOnStop(MTRUE);
    spPipe->init();
    spPipe->sync();

    int waitForSec = 3;
    for(int i = 0 ; i < waitForSec ; i ++){
        MY_LOGGER("after create sleep %d/%d", i, waitForSec);
        usleep(1000*1000);
    }

    MY_LOGGER("Prepare PipeRequests...");
    MINT32 reqIdx = 0;
    sp<PipeRequest> pReq = generatePipeRequest(reqIdx);
    EXPECT_TRUE(pReq != nullptr)<< "sp<PipeRequest> pReq = generatePipeRequest";

    MY_LOGGER("Submit PipeRequests...");
    spPipe->enque(pReq);

    MY_LOGGER("BMDeNoisePipe start to wait +....");
    bool bRet = WaitingListener::waitRequest(1,30);
    MY_LOGGER("Wait done - ....");
    EXPECT_TRUE(bRet);

    WaitingListener::resetCounter();

    MY_LOGGER("uninit pipe ....");
    spPipe->uninit();
    spPipe = nullptr;

    MY_LOGGER("HalSensorUninit....");
    HalSensorUninit(pHalSensor);

    MY_LOGGER("BMDNBasicTest -");
}


TEST(MFHR, BasicTest)
{
    MY_LOGGER("MFHRBasicTest +");

    ScopeLogger logger("PIPE_LOG_TAG", "MFHR");

#if 0
    IHalSensor* pHalSensor = HalSensorInit();

    cam1DeviceInit();

    flowControlInit();

    int main1Idx, main2Idx;
    StereoSettingProvider::getStereoSensorIndex(main1Idx, main2Idx);

    MY_LOGGER("getStereoSensorIndex: (%d, %d)", main1Idx, main2Idx);

    MY_LOGGER("Create BMDeNoisePipe");
    sp<IBMDeNoisePipe> spPipe = IBMDeNoisePipe::createInstance(
        main1Idx, main2Idx,
        BMDeNoiseFeatureType::TYPE_MFHR
    );
    spPipe->setFlushOnStop(MTRUE);
    spPipe->init();
    spPipe->sync();

    int waitForSec = 3;
    for(int i = 0 ; i < waitForSec ; i ++){
        MY_LOGGER("after create sleep %d/%d", i, waitForSec);
        usleep(1000*1000);
    }

    MY_LOGGER("Prepare PipeRequests...");
    static const int COSTOM_MFHR_REQ_NUM = 3;
    int MFHRReq_id = 0;
    for(int reqIdx = 0 ; reqIdx < COSTOM_MFHR_REQ_NUM ; reqIdx ++){
        bool isMain = (reqIdx == 0) ? true : false;
        sp<PipeRequest> pReq = generatePipeRequest_HR(reqIdx, MFHRReq_id, isMain);
        EXPECT_TRUE(pReq != nullptr)<< "sp<PipeRequest> pReq = generatePipeRequest_HR";

        MY_LOGGER("Submit PipeRequests...%d,%d,%d", reqIdx, MFHRReq_id, isMain);
        spPipe->enque(pReq);
    }

    MY_LOGGER("BMDeNoisePipe start to wait +....");
    bool bRet = WaitingListener::waitRequest(COSTOM_MFHR_REQ_NUM, 30);
    MY_LOGGER("Wait done - ....");
    EXPECT_TRUE(bRet);

    WaitingListener::resetCounter();

    MY_LOGGER("uninit pipe ....");
    spPipe->uninit();
    spPipe = nullptr;

    MY_LOGGER("HalSensorUninit....");
    HalSensorUninit(pHalSensor);
#endif
    MY_LOGGER("MFHRBasicTest -");
}

}
}
}



