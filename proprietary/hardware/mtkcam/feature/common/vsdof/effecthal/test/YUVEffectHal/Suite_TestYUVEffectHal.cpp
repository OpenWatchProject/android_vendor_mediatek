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
#define LOG_TAG "TestYUVEffectHal"

#include <time.h>
#include <gtest/gtest.h>

#include <vector>
#include <utils/Vector.h>
#include <utils/String8.h>
#include <cutils/properties.h>
//mtkcam module header
#include <mtkcam/aaa/IHal3A.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/imgbuf/ImageBufferHeap.h>
#include <mtkcam/feature/effectHalBase/EffectRequest.h>
#include <mtkcam/feature/stereo/effecthal/YUVEffectHal.h>
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/feature/effectHalBase/IEffectHal.h>
#include <mtkcam/feature/effectHalBase/EffectHalBase.h>
#include "TestYUV_Common.h"

#include <mtkcam/utils/std/Log.h>


using android::sp;
using android::String8;
using namespace NSCam;

namespace NSCam{
namespace NSCamFeature{
namespace NSFeaturePipe{

typedef IImageBufferAllocator::ImgParam ImgParam;
StereoSizeProvider* gpSizePrvder = StereoSizeProvider::getInstance();


MVOID loadFullResizRawsImgBuf(sp<IImageBuffer>& rpFSMain1ImgBuf, sp<IImageBuffer>& rpRSMain1ImgBuf, sp<IImageBuffer>& rpFSMain2ImgBuf)
{
    const char * sMain1RSFilename = "/sdcard/denoise/BID_DUALYUV_IN_RSRAW1_2100x1180_3944.raw";
    //const char * sMain2Filename = "/sdcard/vsdof/PASS1_RESIZEDRAW_2_1920x1080_3600_7.raw";
    const char * sMain1FSFilename = "/sdcard/denoise/BID_DUALYUV_IN_FSRAW1_4208x3120_5260.raw";
    const char * sMain2FSFilename = "/sdcard/denoise/BID_DUALYUV_IN_FSRAW2_4208x3120_5260.raw";


    MUINT32 bufStridesInBytes[3] = {3944, 0, 0};
    MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
    IImageBufferAllocator::ImgParam imgParamRRZO = IImageBufferAllocator::ImgParam((eImgFmt_FG_BAYER10), MSize(2100, 1180), bufStridesInBytes, bufBoundaryInBytes, 1);

    MUINT32 bufStridesInBytesFS[3] = {5260, 0, 0};
    MINT32 bufBoundaryInBytesFS[3] = {0, 0, 0};
    IImageBufferAllocator::ImgParam imgParamIMGO = IImageBufferAllocator::ImgParam((eImgFmt_BAYER10), MSize(4208, 3120), bufStridesInBytesFS, bufBoundaryInBytesFS, 1);

    rpRSMain1ImgBuf = createImageBufferFromFile(imgParamRRZO, sMain1RSFilename, "RsRaw1",  eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
    //rpRSMain2ImgBuf = createImageBufferFromFile(imgParamRRZO, sMain2Filename, "RsRaw2",  eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
    rpFSMain1ImgBuf = createImageBufferFromFile(imgParamIMGO, sMain1FSFilename, "FsRaw",  eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
    rpFSMain2ImgBuf = createImageBufferFromFile(imgParamIMGO, sMain2FSFilename, "FsRaw",  eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
}


#define DECLARE_MACRO()\
    sp<EffectFrameInfo> effectFrame;\
    MUINT32 reqIdx;

#define ADD_METABUF_FRAME(effectReq, frameMapName, frameId, rpMetaBuf) \
    rpMetaBuf = new IMetadata();\
    reqIdx = effectReq->getRequestNo();\
    effectFrame = new EffectFrameInfo(reqIdx, frameId); \
    {\
        sp<EffectParameter> effParam = new EffectParameter(); \
        effParam->setPtr(EFFECT_PARAMS_KEY, (void*)rpMetaBuf); \
        effectFrame->setFrameParameter(effParam); \
        effectReq->frameMapName.add(frameId, effectFrame); \
    }

#define ADD_METABUF_INPUT_FRAME(effectReq, frameId, rpMetaBuf)  \
    ADD_METABUF_FRAME(effectReq, vInputFrameInfo, frameId, rpMetaBuf)

#define ADD_METABUF_OUTPUT_FRAME(effectReq, frameId, rpMetaBuf)  \
    ADD_METABUF_FRAME(effectReq, vOutputFrameInfo, frameId, rpMetaBuf)


#define USE_FD 1

sp<EffectRequest> generateEffectRequest(int request_id, YUVEffectHal::DualYUVNodeOpState eState)
{
    MY_LOGD("+: reqID = %d, request state=%d", request_id, eState);
    DECLARE_MACRO();

    sp<EffectRequest> pEffectReq = new EffectRequest(request_id, NULL, NULL);
    sp<EffectParameter> pEffectParam = new EffectParameter();
    // ------------- test VR scenario ----------------
    pEffectParam->set(DUAL_YUV_REQUEST_STATE_KEY, (int)eState);
    pEffectReq->setRequestParameter(pEffectParam);

    sp<IImageBuffer> buf_fs_Main1, buf_rs_Main1, buf_fs_Main2, buf;
    // load resize raw
    loadFullResizRawsImgBuf(buf_fs_Main1, buf_rs_Main1, buf_fs_Main2);
    MY_LOGD("FS_RAW1=%x FS_RAW2=%x RS_RAW1=%x", buf_fs_Main1.get(), buf_fs_Main2.get(), buf_rs_Main1.get());

    if(eState == YUVEffectHal::STATE_CAPTURE)
    {
        // prepare input frame info: FSRAW2
        sp<EffectFrameInfo> frameInfo = new EffectFrameInfo(request_id, YUVEffectHal::BID_DualYUV_IN_FSRAW2);
        frameInfo->setFrameBuffer(buf_fs_Main2);
        pEffectReq->vInputFrameInfo.add(YUVEffectHal::BID_DualYUV_IN_FSRAW2, frameInfo);

        // prepare input frame info: FSRAW1
        frameInfo = new EffectFrameInfo(request_id, YUVEffectHal::BID_DualYUV_IN_FSRAW1);
        frameInfo->setFrameBuffer(buf_fs_Main1);
        pEffectReq->vInputFrameInfo.add(YUVEffectHal::BID_DualYUV_IN_FSRAW1, frameInfo);
    }
    else // preview
    {
        // prepare input frame info: RSRAW1
        sp<EffectFrameInfo> frameInfo = new EffectFrameInfo(request_id, YUVEffectHal::BID_DualYUV_IN_RSRAW1);
        frameInfo->setFrameBuffer(buf_rs_Main1);
        pEffectReq->vInputFrameInfo.add(YUVEffectHal::BID_DualYUV_IN_RSRAW1, frameInfo);
    }


    IMetadata* pMetadata = NULL;
    // prepare the metadata frame info


    if(eState == YUVEffectHal::STATE_CAPTURE)
    {
        // InAppMeta
        ADD_METABUF_INPUT_FRAME(pEffectReq, YUVEffectHal::BID_META_IN_APP, pMetadata);
        // InHalMeta
        ADD_METABUF_INPUT_FRAME(pEffectReq, YUVEffectHal::BID_META_IN_HAL, pMetadata);
        ADD_METABUF_INPUT_FRAME(pEffectReq, YUVEffectHal::BID_META_IN_HAL_MAIN2, pMetadata);
    }
    else // preview
    {
        // InAppMeta
        ADD_METABUF_INPUT_FRAME(pEffectReq, YUVEffectHal::BID_META_IN_APP, pMetadata);
        // // InAppMeta - EIS on
        trySetMetadata<MUINT8>(pMetadata, MTK_CONTROL_VIDEO_STABILIZATION_MODE, MTK_CONTROL_VIDEO_STABILIZATION_MODE_OFF);
        // // InHalMeta
        ADD_METABUF_INPUT_FRAME(pEffectReq, YUVEffectHal::BID_META_IN_HAL, pMetadata);
        // // InHalMeta-EIS region
        // IMetadata::IEntry entry(MTK_EIS_REGION);
        // entry.push_back(0, Type2Type< MINT32 >());
        // entry.push_back(0, Type2Type< MINT32 >());
        // entry.push_back(0, Type2Type< MINT32 >());
        // entry.push_back(0, Type2Type< MINT32 >());
        // entry.push_back(0, Type2Type< MINT32 >());
        // entry.push_back(0, Type2Type< MINT32 >());
        // // the following is  MVtoCenterX, MVtoCenterY, IsFromRRZ
        // entry.push_back(0, Type2Type< MINT32 >());
        // entry.push_back(0, Type2Type< MINT32 >());
        // entry.push_back(MTRUE, Type2Type< MBOOL >());
        // pMetadata->update(MTK_EIS_REGION, entry);
    }

    // prepare output frame: MV_F
    ImgParam imgParam_MainImageCAP = getImgParam_MV_F_CAP();
    ImgParam imgParam_MainImageCAP_Main2 = getImgParam_MV_F_CAP();
    ImgParam imgParam_MainImage = getImgParam_MV_F();

    if(eState == YUVEffectHal::STATE_CAPTURE)
    {
        // main1
        sp<EffectFrameInfo> frameInfo = new EffectFrameInfo(request_id, YUVEffectHal::BID_OUT_MV_F_CAP_MAIN1);
        buf = createEmptyImageBuffer(imgParam_MainImageCAP, "MainImg_Cap",  eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
        frameInfo->setFrameBuffer(buf);
        pEffectReq->vOutputFrameInfo.add(YUVEffectHal::BID_OUT_MV_F_CAP_MAIN1, frameInfo);

        // main2
        frameInfo = new EffectFrameInfo(request_id, YUVEffectHal::BID_OUT_MV_F_CAP_MAIN2);
        buf = createEmptyImageBuffer(imgParam_MainImageCAP_Main2, "MainImg_Cap_Main2",  eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
        frameInfo->setFrameBuffer(buf);
        pEffectReq->vOutputFrameInfo.add(YUVEffectHal::BID_OUT_MV_F_CAP_MAIN2, frameInfo);
    }
    else
    {
        sp<EffectFrameInfo> frameInfo = new EffectFrameInfo(request_id, YUVEffectHal::BID_OUT_MV_F);
        buf = createEmptyImageBuffer(imgParam_MainImage, "MainImg_MV_F",  eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
        frameInfo->setFrameBuffer(buf);
        pEffectReq->vOutputFrameInfo.add(YUVEffectHal::BID_OUT_MV_F, frameInfo);

        // prepare output frame: FD
        if(USE_FD)
        {
            frameInfo = new EffectFrameInfo(request_id, YUVEffectHal::BID_OUT_FD);
            ImgParam imgParam_FD = getImgParam_FD();
            buf = createEmptyImageBuffer(imgParam_FD, "FD", eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE |  eBUFFER_USAGE_SW_WRITE_RARELY);
            frameInfo->setFrameBuffer(buf);
            pEffectReq->vOutputFrameInfo.add(YUVEffectHal::BID_OUT_FD, frameInfo);
        }
    }

    MY_LOGD("-");
    return pEffectReq;
}

MVOID setupDumpProperties()
{
    property_set("vendor.debug.yuveffect.dump", "1");
    property_set("vendor.debug.camera.log", "1");
    property_set("vendor.debug.camera.log.yuveffecthal", "1");


    //======== Power on sensor ==========
    NSCam::IHalSensorList* pHalSensorList = MAKE_HalSensorList();
    pHalSensorList->searchSensors();
    int sensorCount = pHalSensorList->queryNumberOfSensors();

    int main1Idx, main2Idx;
    StereoSettingProvider::setStereoProfile(STEREO_SENSOR_PROFILE_REAR_FRONT);
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


TEST(YUVEffectHalTest, Standard)
{
    // setup dump buffer properties
    setupDumpProperties();

    MY_LOGD("Create YUVEffectHal...");
    // configure and start effectHal
    YUVEffectHal* pYUVEffectHal = new YUVEffectHal();
    pYUVEffectHal->init();
    pYUVEffectHal->configure();
    pYUVEffectHal->prepare();
    pYUVEffectHal->start();

    MY_LOGD("Prepare EffectRequests...");
    std::vector<sp<EffectRequest>> vEffectReqVec;
    int targetTime  = 1;
    for(int i=0;i<targetTime;i++)
    {
        sp<EffectRequest> pEffReq = generateEffectRequest(i, YUVEffectHal::STATE_CAPTURE);
        vEffectReqVec.push_back(pEffReq);
    }
    MY_LOGD("YUVEffectHal configure and start...");

    // test normal request
    bool bRet = MTRUE;
    for(int i=0;i<vEffectReqVec.size();i++)
    {
        sp<EffectRequest> pReq = vEffectReqVec[i];
        MY_LOGD("YUVEffectHal updateEffectRequest: %d", i);
        bRet = pYUVEffectHal->updateEffectRequest(pReq);
        if(!bRet)
            MY_LOGD("YUVEffectHal return failed: requestindex:%d", i);
    }
    MY_LOGD("YUVEffectHal start to wait!!....");

}

}
}
}



