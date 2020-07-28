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
#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__
//
#define LOG_VRB(fmt, arg...)        CAM_LOGV("[%s] " fmt, __func__, ##arg)
#define LOG_DBG(fmt, arg...)        CAM_LOGD("[%s] " fmt, __func__, ##arg)
#define LOG_INF(fmt, arg...)        CAM_LOGI("[%s] " fmt, __func__, ##arg)
#define LOG_WRN(fmt, arg...)        CAM_LOGW("[%s] " fmt, __func__, ##arg)
#define LOG_ERR(fmt, arg...)        CAM_LOGE("[ERROR][%s] " fmt, __func__, ##arg)
#define LOG_AST(cond, fmt, arg...)  do{ if(!cond) CAM_LOGA("[%s] " fmt, __func__, ##arg); } while(0)
//
#define LOG_TAG "RecordFrameGenerator"
//
#include <utils/RefBase.h>
#include <mtkcam/feature/effectHalBase/IEffectHal.h>
#include <mtkcam/feature/effectHalBase/EffectHalBase.h>
#include <mtkcam/feature/effectHalBase/EffectRequest.h>
#include "RecordFrameGenerator.h"
//
#include <DebugUtil.h>
#include <string>
//
#include <mtkcam/feature/stereo/pipe/vsdof_common.h>
#include <mtkcam/feature/stereo/pipe/vsdof_data_define.h>
//
using namespace std;
using namespace NSCam;
using namespace NSCam::NSCamFeature::NSFeaturePipe;
using namespace VSDOF::Bokeh::UT;
/*******************************************************************************
 *
 ********************************************************************************/
RecordFrameGenerator::
RecordFrameGenerator(
    MINT32 scenario) : IFrameGeneratorBase(scenario)
{
}
/*******************************************************************************
 *
 ********************************************************************************/
RecordFrameGenerator::
~RecordFrameGenerator()
{
    LOG_DBG("dcot(0x%x)", this);
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
RecordFrameGenerator::
init(
    GeneratorParam param)
{
    mParams=param;
    //
    prepareFrame();
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
RecordFrameGenerator::
uninit()
{
    mvMainImageBuffer.clear();
    mvDepthMapImageBuffer.clear();
    mvMYSImageBuffer.clear();
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
EffectRequestPtr
RecordFrameGenerator::
generate()
{
    LOG_DBG("+");
    if(miIndex == mvDepthMapImageBuffer.size())
    {
        return nullptr;
    }
    // build request parameter for this request.
    sp<EffectParameter> reqst_para = new EffectParameter();
    reqst_para->set(VSDOF_FRAME_SCENARIO, mParams.scenario);
    reqst_para->set(VSDOF_FRAME_3DNR_FLAG, mParams.is3DNREnable);
    reqst_para->set(VSDOF_FRAME_GMV_X, mParams.gmv_x);
    reqst_para->set(VSDOF_FRAME_GMV_Y, mParams.gmv_y);
    reqst_para->set(VSDOF_FRAME_AF_TRIGGER_FLAG, mParams.isAFTrigger);
    reqst_para->set(VSDOF_FRAME_AF_POINT_X, mParams.ptAF_x);
    reqst_para->set(VSDOF_FRAME_AF_POINT_Y, mParams.ptAF_y);
    reqst_para->set(VSDOF_FRAME_BOKEH_LEVEL, mParams.bokehLevel);
    reqst_para->set(VSDOF_FRAME_G_SENSOR_ORIENTATION, mParams.gSensor);
    // set input frame
    sp<EffectFrameInfo> mainImgFrame = new EffectFrameInfo(
                                                miIndex,
                                                BOKEH_ER_BUF_MAIN1,
                                                nullptr,
                                                nullptr);
    mainImgFrame->setFrameBuffer(mvMainImageBuffer[miIndex]);
    sp<EffectFrameInfo> dmgFrame = new EffectFrameInfo(
                                                miIndex,
                                                BOKEH_ER_BUF_DMG,
                                                nullptr,
                                                nullptr);
    dmgFrame->setFrameBuffer(mvDepthMapImageBuffer[miIndex]);
    /*sp<EffectFrameInfo> mysFrame = new EffectFrameInfo(
                                                miIndex,
                                                BOKEH_ER_BUF_MYS,
                                                nullptr,
                                                nullptr);
    mysFrame->setFrameBuffer(mvMYSImageBuffer[miIndex]);*/
    // set output frame
    sp<EffectFrameInfo> previewFrame = new EffectFrameInfo(
                                                miIndex,
                                                BOKEH_ER_BUF_DISPLAY,
                                                nullptr,
                                                nullptr);
    previewFrame->setFrameBuffer(mvPreviewImageBuffer[miIndex]);
    sp<EffectFrameInfo> recordFrame = new EffectFrameInfo(
                                                miIndex,
                                                BOKEH_ER_BUF_RECORD,
                                                nullptr,
                                                nullptr);
    recordFrame->setFrameBuffer(mvRecordImageBuffer[miIndex]);
    //
    EffectRequestPtr pRequest = new EffectRequest(miIndex, NULL, NULL);
    pRequest->setRequestParameter(reqst_para);
    pRequest->vInputFrameInfo.add(BOKEH_ER_BUF_DMW, dmgFrame);
    //pRequest->vInputFrameInfo.add(BOKEH_ER_BUF_MYS, mysFrame);
    pRequest->vInputFrameInfo.add(BOKEH_ER_BUF_MAIN1, mainImgFrame);
    pRequest->vOutputFrameInfo.add(BOKEH_ER_BUF_DISPLAY, previewFrame);
    pRequest->vOutputFrameInfo.add(BOKEH_ER_BUF_RECORD, recordFrame);
    //
    miIndex++;
    LOG_DBG("-");
    return pRequest;
}
/*******************************************************************************
 *
 ********************************************************************************/
void
RecordFrameGenerator::
prepareFrame()
{
    sp<IImageBuffer> pImgBuffer = nullptr;
    FILE *pFile;
    long lSize;
    // depth map params (Y8, 1 plane)
    const MSize DEPTHMAP_SIZE(240,136);
    MUINT32 DEPTHMAP_STRIDES[3] = {240, 0, 0};
    MUINT32 FULL_RAW_BOUNDARY[3] = {0, 0, 0};
    const IImageBufferAllocator::ImgParam imgParam_DepthMap(
    eImgFmt_STA_BYTE, DEPTHMAP_SIZE, DEPTHMAP_STRIDES, FULL_RAW_BOUNDARY, 1);
    // MY_S
    const MSize MYS_SIZE(240,136);
    MUINT32 MYS_STRIDES[3] = {240, 120, 120};
    const IImageBufferAllocator::ImgParam imgParam_MYS(
    eImgFmt_YV12, MYS_SIZE, MYS_STRIDES, FULL_RAW_BOUNDARY, 3);
    // main image
    const MSize MAIN_IMG_SIZE(1920, 1080);
    MUINT32 MAIN_IMT_STRIDES[3] = {1920, 960, 960};
    const IImageBufferAllocator::ImgParam imgParam_MainImg(
    eImgFmt_YV12, MAIN_IMG_SIZE, MAIN_IMT_STRIDES, FULL_RAW_BOUNDARY, 3);
    // preview image
    const MSize MAIN_PREVIEW_SIZE(1920, 1080);
    MUINT32 PREVIEW_IMG_STRIDES[3] = {1920, 960, 960};
    const IImageBufferAllocator::ImgParam imgParam_PreviewImg(
    eImgFmt_YV12, MAIN_PREVIEW_SIZE, PREVIEW_IMG_STRIDES, FULL_RAW_BOUNDARY, 3);
#define LOAD_MAIN_IMAGE(NAME, PATH)\
    do{\
        pImgBuffer = loadImgFromFile(\
                        imgParam_MainImg,\
                        PATH,\
                        NAME,\
                        eBUFFER_USAGE_HW_CAMERA_WRITE|eBUFFER_USAGE_SW_READ_OFTEN|eBUFFER_USAGE_SW_WRITE_RARELY);\
        mvMainImageBuffer.push_back(pImgBuffer);\
    }while(0)
    LOAD_MAIN_IMAGE("MainImg_0", "/sdcard/vsdof/bokeh_ut/data/mtk_scene2_00001Orig_M_1920x1080.yuv");
    LOAD_MAIN_IMAGE("MainImg_1", "/sdcard/vsdof/bokeh_ut/data/mtk_scene2_00002Orig_M_1920x1080.yuv");
    LOAD_MAIN_IMAGE("MainImg_2", "/sdcard/vsdof/bokeh_ut/data/mtk_scene2_00003Orig_M_1920x1080.yuv");
    LOAD_MAIN_IMAGE("MainImg_3", "/sdcard/vsdof/bokeh_ut/data/mtk_scene2_00004Orig_M_1920x1080.yuv");
    LOAD_MAIN_IMAGE("MainImg_4", "/sdcard/vsdof/bokeh_ut/data/mtk_scene2_00005Orig_M_1920x1080.yuv");
    LOAD_MAIN_IMAGE("MainImg_5", "/sdcard/vsdof/bokeh_ut/data/mtk_scene2_00006Orig_M_1920x1080.yuv");
    LOAD_MAIN_IMAGE("MainImg_6", "/sdcard/vsdof/bokeh_ut/data/mtk_scene2_00007Orig_M_1920x1080.yuv");
    LOAD_MAIN_IMAGE("MainImg_7", "/sdcard/vsdof/bokeh_ut/data/mtk_scene2_00008Orig_M_1920x1080.yuv");
    LOAD_MAIN_IMAGE("MainImg_8", "/sdcard/vsdof/bokeh_ut/data/mtk_scene2_00009Orig_M_1920x1080.yuv");
    LOAD_MAIN_IMAGE("MainImg_9", "/sdcard/vsdof/bokeh_ut/data/mtk_scene2_00010Orig_M_1920x1080.yuv");
#undef LOAD_MAIN_IMAGE
    // dump image buffer
    /*makePath("/sdcard/vsdof/bokeh_ut/result/", 0660);
    string saveFileName = "";
    for(MINT32 i=0;i<mvMainImageBuffer.size();++i)
    {
        saveFileName = string("/sdcard/vsdof/bokeh_ut/result/MainImg_")+
                       to_string(i)+
                       string(".yuv");
        mvMainImageBuffer[i]->saveToFile(saveFileName.c_str());
    }*/
#define LOAD_DEPTH_MAP(NAME, PATH)\
    do{\
        pImgBuffer = createEmptyImageBuffer(\
                        imgParam_DepthMap,\
                        NAME,\
                        eBUFFER_USAGE_HW_CAMERA_WRITE|eBUFFER_USAGE_SW_READ_OFTEN|eBUFFER_USAGE_SW_WRITE_RARELY);\
        pFile = fopen(PATH, "rb");\
        if(pFile == nullptr)\
        {\
            LOG_ERR("Open fail: %s", PATH);\
            return;\
        }\
        fseek(pFile, 0, SEEK_END);\
        lSize = ftell(pFile);\
        rewind(pFile);\
        fread((void*)pImgBuffer->getBufVA(0), lSize, 1, pFile);\
        fclose(pFile);\
        mvDepthMapImageBuffer.push_back(pImgBuffer);\
    }while(0)
    //
    LOAD_DEPTH_MAP("DepthMap_0", "/sdcard/vsdof/bokeh_ut/data/Bmap_GF_00000.raw");
    LOAD_DEPTH_MAP("DepthMap_1", "/sdcard/vsdof/bokeh_ut/data/Bmap_GF_00001.raw");
    LOAD_DEPTH_MAP("DepthMap_2", "/sdcard/vsdof/bokeh_ut/data/Bmap_GF_00002.raw");
    LOAD_DEPTH_MAP("DepthMap_3", "/sdcard/vsdof/bokeh_ut/data/Bmap_GF_00003.raw");
    LOAD_DEPTH_MAP("DepthMap_4", "/sdcard/vsdof/bokeh_ut/data/Bmap_GF_00004.raw");
    LOAD_DEPTH_MAP("DepthMap_5", "/sdcard/vsdof/bokeh_ut/data/Bmap_GF_00005.raw");
    LOAD_DEPTH_MAP("DepthMap_6", "/sdcard/vsdof/bokeh_ut/data/Bmap_GF_00006.raw");
    LOAD_DEPTH_MAP("DepthMap_7", "/sdcard/vsdof/bokeh_ut/data/Bmap_GF_00007.raw");
    LOAD_DEPTH_MAP("DepthMap_8", "/sdcard/vsdof/bokeh_ut/data/Bmap_GF_00008.raw");
    LOAD_DEPTH_MAP("DepthMap_9", "/sdcard/vsdof/bokeh_ut/data/Bmap_GF_00009.raw");
#undef LOAD_DEPTH_MAP
    // dump image buffer
    /*makePath("/sdcard/vsdof/bokeh_ut/result/", 0660);
    string saveFileName = "";
    for(MINT32 i=0;i<mvDepthMapImageBuffer.size();++i)
    {
        saveFileName = string("/sdcard/vsdof/bokeh_ut/result/DepthMap_")+
                       to_string(i)+
                       string(".yuv");
        mvDepthMapImageBuffer[i]->saveToFile(saveFileName.c_str());
    }*/
/*#define LOAD_MYS(NAME, PATH_Y, PATH_U, PATH_V)\
    do{\
        pImgBuffer = createEmptyImageBuffer(\
                        imgParam_MYS,\
                        NAME,\
                        eBUFFER_USAGE_HW_CAMERA_WRITE|eBUFFER_USAGE_SW_READ_OFTEN|eBUFFER_USAGE_SW_WRITE_RARELY);\
        pFile = fopen(PATH_Y, "rb");\
        if(pFile == nullptr)\
        {\
            LOG_ERR("Open fail: %s", PATH_Y);\
            return;\
        }\
        fseek(pFile, 0, SEEK_END);\
        lSize = ftell(pFile);\
        rewind(pFile);\
        fread((void*)pImgBuffer->getBufVA(0), lSize, 1, pFile);\
        fclose(pFile);\
        pFile = fopen(PATH_U, "rb");\
        if(pFile == nullptr)\
        {\
            LOG_ERR("Open fail: %s", PATH_U);\
            return;\
        }\
        fseek(pFile, 0, SEEK_END);\
        lSize = ftell(pFile);\
        rewind(pFile);\
        fread((void*)pImgBuffer->getBufVA(1), lSize, 1, pFile);\
        fclose(pFile);\
        pFile = fopen(PATH_V, "rb");\
        if(pFile == nullptr)\
        {\
            LOG_ERR("Open fail: %s", PATH_V);\
            return;\
        }\
        fseek(pFile, 0, SEEK_END);\
        lSize = ftell(pFile);\
        rewind(pFile);\
        fread((void*)pImgBuffer->getBufVA(2), lSize, 1, pFile);\
        fclose(pFile);\
        mvMYSImageBuffer.push_back(pImgBuffer);\
    }while(0)
    LOAD_MYS(
            "MYS_0",
            "/sdcard/vsdof/bokeh_ut/data/m_pDownY8_V_00000.raw",
            "/sdcard/vsdof/bokeh_ut/data/m_pDownU8420_V_00000.raw",
            "/sdcard/vsdof/bokeh_ut/data/m_pDownV8420_V_00000.raw");
    LOAD_MYS(
            "MYS_1",
            "/sdcard/vsdof/bokeh_ut/data/m_pDownY8_V_00001.raw",
            "/sdcard/vsdof/bokeh_ut/data/m_pDownU8420_V_00001.raw",
            "/sdcard/vsdof/bokeh_ut/data/m_pDownV8420_V_00001.raw");
    LOAD_MYS(
            "MYS_2",
            "/sdcard/vsdof/bokeh_ut/data/m_pDownY8_V_00002.raw",
            "/sdcard/vsdof/bokeh_ut/data/m_pDownU8420_V_00002.raw",
            "/sdcard/vsdof/bokeh_ut/data/m_pDownV8420_V_00002.raw");
    LOAD_MYS(
            "MYS_3",
            "/sdcard/vsdof/bokeh_ut/data/m_pDownY8_V_00003.raw",
            "/sdcard/vsdof/bokeh_ut/data/m_pDownU8420_V_00003.raw",
            "/sdcard/vsdof/bokeh_ut/data/m_pDownV8420_V_00003.raw");
    LOAD_MYS(
            "MYS_4",
            "/sdcard/vsdof/bokeh_ut/data/m_pDownY8_V_00004.raw",
            "/sdcard/vsdof/bokeh_ut/data/m_pDownU8420_V_00004.raw",
            "/sdcard/vsdof/bokeh_ut/data/m_pDownV8420_V_00004.raw");
    LOAD_MYS(
            "MYS_5",
            "/sdcard/vsdof/bokeh_ut/data/m_pDownY8_V_00005.raw",
            "/sdcard/vsdof/bokeh_ut/data/m_pDownU8420_V_00005.raw",
            "/sdcard/vsdof/bokeh_ut/data/m_pDownV8420_V_00005.raw");
    LOAD_MYS(
            "MYS_6",
            "/sdcard/vsdof/bokeh_ut/data/m_pDownY8_V_00006.raw",
            "/sdcard/vsdof/bokeh_ut/data/m_pDownU8420_V_00006.raw",
            "/sdcard/vsdof/bokeh_ut/data/m_pDownV8420_V_00006.raw");
    LOAD_MYS(
            "MYS_7",
            "/sdcard/vsdof/bokeh_ut/data/m_pDownY8_V_00007.raw",
            "/sdcard/vsdof/bokeh_ut/data/m_pDownU8420_V_00007.raw",
            "/sdcard/vsdof/bokeh_ut/data/m_pDownV8420_V_00007.raw");
    LOAD_MYS(
            "MYS_8",
            "/sdcard/vsdof/bokeh_ut/data/m_pDownY8_V_00008.raw",
            "/sdcard/vsdof/bokeh_ut/data/m_pDownU8420_V_00008.raw",
            "/sdcard/vsdof/bokeh_ut/data/m_pDownV8420_V_00008.raw");
    LOAD_MYS(
            "MYS_9",
            "/sdcard/vsdof/bokeh_ut/data/m_pDownY8_V_00009.raw",
            "/sdcard/vsdof/bokeh_ut/data/m_pDownU8420_V_00009.raw",
            "/sdcard/vsdof/bokeh_ut/data/m_pDownV8420_V_00009.raw");
#undef LOAD_MYS*/
    /*makePath("/sdcard/vsdof/bokeh_ut/result/", 0660);
    saveFileName = "";
    for(MINT32 i=0;i<mvMYSImageBuffer.size();++i)
    {
        saveFileName = string("/sdcard/vsdof/bokeh_ut/result/MYS_")+
                       to_string(i)+
                       string(".yuv");
        mvMYSImageBuffer[i]->saveToFile(saveFileName.c_str());
    }*/
#define CREATE_EMPTY_IMAGE_BUFFER(NAME, TYPE)\
    do{\
        pImgBuffer = createEmptyImageBuffer(\
                        imgParam_PreviewImg,\
                        NAME,\
                        eBUFFER_USAGE_HW_CAMERA_WRITE|eBUFFER_USAGE_SW_READ_OFTEN|eBUFFER_USAGE_SW_WRITE_RARELY);\
        mv##TYPE##ImageBuffer.push_back(pImgBuffer);\
    }while(0)
    CREATE_EMPTY_IMAGE_BUFFER("PRV_0", Preview);
    CREATE_EMPTY_IMAGE_BUFFER("PRV_1", Preview);
    CREATE_EMPTY_IMAGE_BUFFER("PRV_2", Preview);
    CREATE_EMPTY_IMAGE_BUFFER("PRV_3", Preview);
    CREATE_EMPTY_IMAGE_BUFFER("PRV_4", Preview);
    CREATE_EMPTY_IMAGE_BUFFER("PRV_5", Preview);
    CREATE_EMPTY_IMAGE_BUFFER("PRV_6", Preview);
    CREATE_EMPTY_IMAGE_BUFFER("PRV_7", Preview);
    CREATE_EMPTY_IMAGE_BUFFER("PRV_8", Preview);
    CREATE_EMPTY_IMAGE_BUFFER("PRV_9", Preview);
    //
    CREATE_EMPTY_IMAGE_BUFFER("REC_0", Record);
    CREATE_EMPTY_IMAGE_BUFFER("REC_1", Record);
    CREATE_EMPTY_IMAGE_BUFFER("REC_2", Record);
    CREATE_EMPTY_IMAGE_BUFFER("REC_3", Record);
    CREATE_EMPTY_IMAGE_BUFFER("REC_4", Record);
    CREATE_EMPTY_IMAGE_BUFFER("REC_5", Record);
    CREATE_EMPTY_IMAGE_BUFFER("REC_6", Record);
    CREATE_EMPTY_IMAGE_BUFFER("REC_7", Record);
    CREATE_EMPTY_IMAGE_BUFFER("REC_8", Record);
    CREATE_EMPTY_IMAGE_BUFFER("REC_9", Record);
#undef CREATE_EMPTY_IMAGE_BUFFER
}
