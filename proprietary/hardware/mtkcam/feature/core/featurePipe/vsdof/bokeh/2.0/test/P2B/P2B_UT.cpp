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
#define LOG_TAG "P2B_UT"
//
#include <mtkcam/feature/stereo/pipe/vsdof_common.h>
#include <mtkcam/feature/stereo/pipe/vsdof_data_define.h>
#include <mtkcam/feature/stereo/hal/stereo_common.h>
//
#include <sys/resource.h>
//
#include "P2B_UT.h"
//
using namespace android;
using namespace NSCam;
using namespace VSDOF::Bokeh::UT;
using namespace StereoHAL;
/*******************************************************************************
 *
 ********************************************************************************/
P2B_UT::
~P2B_UT()
{
    LOG_DBG("CKH: dctor(0x%x)", this);
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
P2B_UT::
init(P2B_UT_Params params)
{
    mParams = params;
    preareInputData();
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
P2B_UT::
uninit()
{
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
P2B_UT::
generate(
    sp<EffectRequest>& effect/*,
    SmartImageBuffer& dmg,
    SmartImageBuffer& dmbg*/)
{
    LOG_DBG("+");
    if(mvWDMAImageBuffer.size() == 0)
    {
        LOG_ERR("Preview buffer is not create");
        return MFALSE;
    }
    if(mvMainImageBuffer.size() == 0)
    {
        LOG_ERR("Please check main img is upload.");
        return MFALSE;
    }
    if(mvWROTImageBuffer.size() == 0)
    {
        LOG_ERR("Preview buffer is not create");
        return MFALSE;
    }
    if(miIndex == mvMainImageBuffer.size())
    {
        return MFALSE;
    }
    //
    effect = new EffectRequest(miIndex, NULL, NULL);
    //
    //dmg = mpDMGBufPool->request();
    //dmbg = mpDMBGBufPool ->request();
    // build request
    sp<EffectParameter> reqst_para = new EffectParameter();
    reqst_para->set(VSDOF_FRAME_SCENARIO, mParams.scenario);
    reqst_para->set(VSDOF_FRAME_3DNR_FLAG, mParams.is3DNREnable);
    reqst_para->set(VSDOF_FRAME_GMV_X, mParams.gmv_x);
    reqst_para->set(VSDOF_FRAME_GMV_Y, mParams.gmv_y);
    reqst_para->set(VSDOF_FRAME_AF_TRIGGER_FLAG, mParams.isAFTrigger);
    reqst_para->set(VSDOF_FRAME_AF_POINT_X, mParams.ptAF_x);
    reqst_para->set(VSDOF_FRAME_AF_POINT_Y, mParams.ptAF_y);
    reqst_para->set(VSDOF_FRAME_BOKEH_LEVEL, mParams.bokehLevel);
    reqst_para->set(VSDOF_FRAME_G_SENSOR_ORIENTATION, mParams.iGSensor);
    effect->setRequestParameter(reqst_para);
    if(mParams.scenario == ENUM_STEREO_SCENARIO::eSTEREO_SCENARIO_PREVIEW ||
        mParams.scenario == ENUM_STEREO_SCENARIO::eSTEREO_SCENARIO_RECORD)
    {
        // set input frame
        printf("Genereate Preview/Capture frame\n");
        sp<EffectFrameInfo> mainImgFrame = new EffectFrameInfo(
                                                    miIndex,
                                                    BOKEH_ER_BUF_MAIN1,
                                                    nullptr,
                                                    nullptr);
        mainImgFrame->setFrameBuffer(mvMainImageBuffer[miIndex]);
        sp<EffectFrameInfo> dmbgImgFrame = new EffectFrameInfo(
                                                    miIndex,
                                                    BOKEH_ER_BUF_DMG,
                                                    nullptr,
                                                    nullptr);
        dmbgImgFrame->setFrameBuffer(mvDMBGImageBuffer[miIndex]);
        // set output frame
        sp<EffectFrameInfo> previewFrame = new EffectFrameInfo(
                                                    miIndex,
                                                    BOKEH_ER_BUF_DISPLAY,
                                                    nullptr,
                                                    nullptr);
        previewFrame->setFrameBuffer(mvWDMAImageBuffer[miIndex]);
        effect->vInputFrameInfo.add(BOKEH_ER_BUF_MAIN1, mainImgFrame);
        effect->vInputFrameInfo.add(BOKEH_ER_BUF_DMG, dmbgImgFrame);
        effect->vOutputFrameInfo.add(BOKEH_ER_BUF_DISPLAY, previewFrame);
        // set Rec frame
        sp<EffectFrameInfo> recordFrame = nullptr;
        if(mParams.scenario == ENUM_STEREO_SCENARIO::eSTEREO_SCENARIO_RECORD)
        {
            recordFrame = new EffectFrameInfo(
                                                        miIndex,
                                                        BOKEH_ER_BUF_RECORD,
                                                        nullptr,
                                                        nullptr);
            recordFrame->setFrameBuffer(mvWROTImageBuffer[miIndex]);
            effect->vOutputFrameInfo.add(BOKEH_ER_BUF_RECORD, recordFrame);
        }
    }
    else if(mParams.scenario == ENUM_STEREO_SCENARIO::eSTEREO_SCENARIO_CAPTURE)
    {
        // set input frame
        printf("Genereate Capture frame\n");
        sp<EffectFrameInfo> mainImgFrame = new EffectFrameInfo(
                                                    miIndex,
                                                    BOKEH_ER_BUF_MAIN1,
                                                    nullptr,
                                                    nullptr);
        mainImgFrame->setFrameBuffer(mvMainImageBuffer_Cap[miIndex]);
        //
        sp<EffectFrameInfo> pFrame = nullptr;
        effect->vInputFrameInfo.add(BOKEH_ER_BUF_MAIN1, mainImgFrame);
        pFrame = new EffectFrameInfo(
                                                    miIndex,
                                                    BOKEH_ER_BUF_CLEAN_IMG,
                                                    nullptr,
                                                    nullptr);
        pFrame->setFrameBuffer(mvWDMAImageBuffer[miIndex]);
        effect->vOutputFrameInfo.add(BOKEH_ER_BUF_CLEAN_IMG, pFrame);
        //
        pFrame = new EffectFrameInfo(
                                                    miIndex,
                                                    BOKEH_ER_BUF_VSDOF_IMG,
                                                    nullptr,
                                                    nullptr);
        pFrame->setFrameBuffer(mvWROTImageBuffer[miIndex]);
        effect->vOutputFrameInfo.add(BOKEH_ER_BUF_VSDOF_IMG, pFrame);
    }
    else
    {
        LOG_ERR("Unsupported scenario");
    }
    //
    miIndex++;
    LOG_DBG("-");
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
P2B_UT::
preareInputData()
{
    LOG_DBG("+");
    sp<IImageBuffer> pImgBuffer = nullptr;
    FILE *pFile;
    long lSize;
    // depth map params (Y8, 1 plane)
    const MSize DMBG_SIZE(240,136);
    MUINT32 DMBG_STRIDES[3] = {240, 0, 0};
    MUINT32 FULL_RAW_BOUNDARY[3] = {0, 0, 0};
    const IImageBufferAllocator::ImgParam imgParam_DMBG(
    eImgFmt_STA_BYTE, DMBG_SIZE, DMBG_STRIDES, FULL_RAW_BOUNDARY, 1);
    // main image
    const MSize MAIN_IMG_SIZE(1920, 1080);
    MUINT32 MAIN_IMT_STRIDES[3] = {1920, 960, 960};
    const IImageBufferAllocator::ImgParam imgParam_MainImg(
    eImgFmt_YV12, MAIN_IMG_SIZE, MAIN_IMT_STRIDES, FULL_RAW_BOUNDARY, 3);
    // main image cap
    const MSize MAIN_IMG_SIZE_CAP(3072, 1728);
    MUINT32 MAIN_IMT_STRIDES_CAP[3] = {3072, 1536, 1536};
    const IImageBufferAllocator::ImgParam imgParam_MainImg_Cap(
    eImgFmt_YV12, MAIN_IMG_SIZE_CAP, MAIN_IMT_STRIDES_CAP, FULL_RAW_BOUNDARY, 3);
    // preview image
    const MSize MAIN_PREVIEW_SIZE(1920, 1080);
    MUINT32 PREVIEW_IMG_STRIDES[3] = {1920, 960, 960};
    const IImageBufferAllocator::ImgParam imgParam_PreviewImg(
    eImgFmt_YV12, MAIN_PREVIEW_SIZE, PREVIEW_IMG_STRIDES, FULL_RAW_BOUNDARY, 3);
    //
    // create DMG buffer pool
    /*createBufferPool(
                    mpDMGBufPool,
                    240,
                    136,
                    eImgFmt_STA_BYTE,
                    10,
                    "mpDMGBufPool",
                    ImageBufferPool::USAGE_SW|ImageBufferPool::USAGE_HW);
    // init DMG
    SmartImageBuffer sbDMGBuffer = nullptr;
    for(int i=0;i<10;++i)
    {
        sbDMGBuffer = mpDMGBufPool->request();
        setImageBufferValue(
                    sbDMGBuffer,
                    240,
                    136,
                    128);
        sbDMGBuffer = nullptr; // return to pool.
    }*/
    // create DMBG buffer pool
    /*createBufferPool(
                    mpDMBGBufPool,
                    240,
                    136,
                    eImgFmt_STA_BYTE,
                    10,
                    "mpDMBGBufPool",
                    ImageBufferPool::USAGE_SW|ImageBufferPool::USAGE_HW);*/
    //
#define LOAD_MAIN_IMAGE(NAME, PATH)\
    do{\
        pImgBuffer = loadImgFromFile(\
                        imgParam_MainImg,\
                        PATH,\
                        NAME,\
                        eBUFFER_USAGE_HW_CAMERA_WRITE|eBUFFER_USAGE_SW_READ_OFTEN|eBUFFER_USAGE_SW_WRITE_RARELY|eBUFFER_USAGE_HW_MASK);\
        mvMainImageBuffer.push_back(pImgBuffer);\
    }while(0)
    LOG_DBG("Load main image");
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
    //
#define LOAD_MAIN_IMAGE_CAP(NAME, PATH)\
    do{\
        pImgBuffer = loadImgFromFile(\
                        imgParam_MainImg_Cap,\
                        PATH,\
                        NAME,\
                        eBUFFER_USAGE_HW_CAMERA_WRITE|eBUFFER_USAGE_SW_READ_OFTEN|eBUFFER_USAGE_SW_WRITE_RARELY|eBUFFER_USAGE_HW_MASK);\
        mvMainImageBuffer_Cap.push_back(pImgBuffer);\
    }while(0)
    LOG_DBG("Load capture main image");
    LOAD_MAIN_IMAGE_CAP("MainImg_0_CAP", "/sdcard/vsdof/bokeh_ut/data/mtk_scene2_00001Orig_M_3072x1728.yuv");
    LOAD_MAIN_IMAGE_CAP("MainImg_1_CAP", "/sdcard/vsdof/bokeh_ut/data/mtk_scene2_00002Orig_M_3072x1728.yuv");
    LOAD_MAIN_IMAGE_CAP("MainImg_2_CAP", "/sdcard/vsdof/bokeh_ut/data/mtk_scene2_00003Orig_M_3072x1728.yuv");
    LOAD_MAIN_IMAGE_CAP("MainImg_3_CAP", "/sdcard/vsdof/bokeh_ut/data/mtk_scene2_00004Orig_M_3072x1728.yuv");
    LOAD_MAIN_IMAGE_CAP("MainImg_4_CAP", "/sdcard/vsdof/bokeh_ut/data/mtk_scene2_00005Orig_M_3072x1728.yuv");
    LOAD_MAIN_IMAGE_CAP("MainImg_5_CAP", "/sdcard/vsdof/bokeh_ut/data/mtk_scene2_00006Orig_M_3072x1728.yuv");
    LOAD_MAIN_IMAGE_CAP("MainImg_6_CAP", "/sdcard/vsdof/bokeh_ut/data/mtk_scene2_00007Orig_M_3072x1728.yuv");
    LOAD_MAIN_IMAGE_CAP("MainImg_7_CAP", "/sdcard/vsdof/bokeh_ut/data/mtk_scene2_00008Orig_M_3072x1728.yuv");
    LOAD_MAIN_IMAGE_CAP("MainImg_8_CAP", "/sdcard/vsdof/bokeh_ut/data/mtk_scene2_00009Orig_M_3072x1728.yuv");
    LOAD_MAIN_IMAGE_CAP("MainImg_9_CAP", "/sdcard/vsdof/bokeh_ut/data/mtk_scene2_00010Orig_M_3072x1728.yuv");
#undef LOAD_MAIN_IMAGE_CAP
    //
#define LOAD_DMBG_MAP(NAME, PATH)\
    do{\
        pImgBuffer = createEmptyImageBuffer(\
                        imgParam_DMBG,\
                        NAME,\
                        eBUFFER_USAGE_HW_CAMERA_WRITE|eBUFFER_USAGE_SW_READ_OFTEN|eBUFFER_USAGE_SW_WRITE_RARELY);\
        pFile = fopen(PATH, "rb");\
        if(pFile == nullptr)\
        {\
            LOG_ERR("Open fail: %s", PATH);\
            return MFALSE;\
        }\
        fseek(pFile, 0, SEEK_END);\
        lSize = ftell(pFile);\
        rewind(pFile);\
        fread((void*)pImgBuffer->getBufVA(0), lSize, 1, pFile);\
        fclose(pFile);\
        LOG_DBG("CKH: " #NAME ": fmt(0x%x)", pImgBuffer->getImgFormat());\
        mvDMBGImageBuffer.push_back(pImgBuffer);\
    }while(0)
    LOG_DBG("Load DMBG");
    LOAD_DMBG_MAP("Dmbg_0", "/sdcard/vsdof/bokeh_ut/data/Bmap_GF_00000.raw");
    LOAD_DMBG_MAP("Dmbg_1", "/sdcard/vsdof/bokeh_ut/data/Bmap_GF_00001.raw");
    LOAD_DMBG_MAP("Dmbg_2", "/sdcard/vsdof/bokeh_ut/data/Bmap_GF_00002.raw");
    LOAD_DMBG_MAP("Dmbg_3", "/sdcard/vsdof/bokeh_ut/data/Bmap_GF_00003.raw");
    LOAD_DMBG_MAP("Dmbg_4", "/sdcard/vsdof/bokeh_ut/data/Bmap_GF_00004.raw");
    LOAD_DMBG_MAP("Dmbg_5", "/sdcard/vsdof/bokeh_ut/data/Bmap_GF_00005.raw");
    LOAD_DMBG_MAP("Dmbg_6", "/sdcard/vsdof/bokeh_ut/data/Bmap_GF_00006.raw");
    LOAD_DMBG_MAP("Dmbg_7", "/sdcard/vsdof/bokeh_ut/data/Bmap_GF_00007.raw");
    LOAD_DMBG_MAP("Dmbg_8", "/sdcard/vsdof/bokeh_ut/data/Bmap_GF_00008.raw");
    LOAD_DMBG_MAP("Dmbg_9", "/sdcard/vsdof/bokeh_ut/data/Bmap_GF_00009.raw");
#undef LOAD_DMBG_MAP
    //
/*#define LOAD_DMBG_MAP(NAME, PATH, BUFFER)\
    do{\
        pImgBuffer = BUFFER->mImageBuffer;\
        pFile = fopen(PATH, "rb");\
        if(pFile == nullptr)\
        {\
            LOG_ERR("Open fail: %s", PATH);\
            return MFALSE;\
        }\
        fseek(pFile, 0, SEEK_END);\
        lSize = ftell(pFile);\
        rewind(pFile);\
        fread((void*)pImgBuffer->getBufVA(0), lSize, 1, pFile);\
        fclose(pFile);\
        mvDMBGImageBuffer.push_back(pImgBuffer);\
    }while(0)
    //
    SmartImageBuffer temp1 = mpDMBGBufPool->request();
    LOAD_DMBG_MAP("Dmbg_0", "/sdcard/vsdof/bokeh_ut/data/Bmap_GF_00000.raw", temp1);
    SmartImageBuffer temp2 = mpDMBGBufPool->request();
    LOAD_DMBG_MAP("Dmbg_1", "/sdcard/vsdof/bokeh_ut/data/Bmap_GF_00001.raw", temp2);
    SmartImageBuffer temp3 = mpDMBGBufPool->request();
    LOAD_DMBG_MAP("Dmbg_2", "/sdcard/vsdof/bokeh_ut/data/Bmap_GF_00002.raw", temp3);
    SmartImageBuffer temp4 = mpDMBGBufPool->request();
    LOAD_DMBG_MAP("Dmbg_3", "/sdcard/vsdof/bokeh_ut/data/Bmap_GF_00003.raw", temp4);
    SmartImageBuffer temp5 = mpDMBGBufPool->request();
    LOAD_DMBG_MAP("Dmbg_4", "/sdcard/vsdof/bokeh_ut/data/Bmap_GF_00004.raw", temp5);
    SmartImageBuffer temp6 = mpDMBGBufPool->request();
    LOAD_DMBG_MAP("Dmbg_5", "/sdcard/vsdof/bokeh_ut/data/Bmap_GF_00005.raw", temp6);
    SmartImageBuffer temp7 = mpDMBGBufPool->request();
    LOAD_DMBG_MAP("Dmbg_6", "/sdcard/vsdof/bokeh_ut/data/Bmap_GF_00006.raw", temp7);
    SmartImageBuffer temp8 = mpDMBGBufPool->request();
    LOAD_DMBG_MAP("Dmbg_7", "/sdcard/vsdof/bokeh_ut/data/Bmap_GF_00007.raw", temp8);
    SmartImageBuffer temp9= mpDMBGBufPool->request();
    LOAD_DMBG_MAP("Dmbg_8", "/sdcard/vsdof/bokeh_ut/data/Bmap_GF_00008.raw", temp9);
    SmartImageBuffer temp10 = mpDMBGBufPool->request();
    LOAD_DMBG_MAP("Dmbg_9", "/sdcard/vsdof/bokeh_ut/data/Bmap_GF_00009.raw", temp10);
    // release
    temp1 = nullptr;
    temp2 = nullptr;
    temp3 = nullptr;
    temp4 = nullptr;
    temp5 = nullptr;
    temp6 = nullptr;
    temp7 = nullptr;
    temp8 = nullptr;
    temp9 = nullptr;
    temp10 = nullptr;
#undef LOAD_DMBG_MAP*/
#define CREATE_EMPTY_IMAGE_BUFFER(NAME, TYPE)\
    do{\
        pImgBuffer = createEmptyImageBuffer(\
                        imgParam_PreviewImg,\
                        NAME,\
                        eBUFFER_USAGE_HW_CAMERA_WRITE|eBUFFER_USAGE_SW_READ_OFTEN|eBUFFER_USAGE_SW_WRITE_RARELY);\
        mv##TYPE##ImageBuffer.push_back(pImgBuffer);\
    }while(0)
    LOG_DBG("Load empty preview buffer");
    CREATE_EMPTY_IMAGE_BUFFER("PRV_0", WDMA);
    CREATE_EMPTY_IMAGE_BUFFER("PRV_1", WDMA);
    CREATE_EMPTY_IMAGE_BUFFER("PRV_2", WDMA);
    CREATE_EMPTY_IMAGE_BUFFER("PRV_3", WDMA);
    CREATE_EMPTY_IMAGE_BUFFER("PRV_4", WDMA);
    CREATE_EMPTY_IMAGE_BUFFER("PRV_5", WDMA);
    CREATE_EMPTY_IMAGE_BUFFER("PRV_6", WDMA);
    CREATE_EMPTY_IMAGE_BUFFER("PRV_7", WDMA);
    CREATE_EMPTY_IMAGE_BUFFER("PRV_8", WDMA);
    CREATE_EMPTY_IMAGE_BUFFER("PRV_9", WDMA);
    //
    LOG_DBG("Load empty record buffer");
    CREATE_EMPTY_IMAGE_BUFFER("REC_0", WROT);
    CREATE_EMPTY_IMAGE_BUFFER("REC_1", WROT);
    CREATE_EMPTY_IMAGE_BUFFER("REC_2", WROT);
    CREATE_EMPTY_IMAGE_BUFFER("REC_3", WROT);
    CREATE_EMPTY_IMAGE_BUFFER("REC_4", WROT);
    CREATE_EMPTY_IMAGE_BUFFER("REC_5", WROT);
    CREATE_EMPTY_IMAGE_BUFFER("REC_6", WROT);
    CREATE_EMPTY_IMAGE_BUFFER("REC_7", WROT);
    CREATE_EMPTY_IMAGE_BUFFER("REC_8", WROT);
    CREATE_EMPTY_IMAGE_BUFFER("REC_9", WROT);
#undef CREATE_EMPTY_IMAGE_BUFFER
    LOG_DBG("-");
    return MTRUE;
}//************************************************************************
//
//************************************************************************
MVOID
P2B_UT::
setImageBufferValue(SmartImageBuffer& buffer,
                MINT32 width,
                MINT32 height,
                MINT32 value)
{
    MUINT8* data = (MUINT8*)buffer->mImageBuffer->getBufVA(0);
    memset(data, value, sizeof(MUINT8) * width * height);
}
//************************************************************************
//
//************************************************************************
MBOOL
P2B_UT::
createBufferPool(
    android::sp<ImageBufferPool> &pPool,
    MUINT32 width,
    MUINT32 height,
    NSCam::EImageFormat format,
    MUINT32 bufCount,
    const char* caller,
    MUINT32 bufUsage)
{
    MBOOL ret = MFALSE;
    pPool = ImageBufferPool::create(caller, width, height, format, bufUsage);
    if(pPool == nullptr)
    {
        ret = MFALSE;
        LOG_AST(MTRUE, "Create [%s] failed.", caller);
        goto lbExit;
    }
    for(MUINT32 i=0;i<bufCount;++i)
    {
        if(!pPool->allocate())
        {
            ret = MFALSE;
            LOG_AST(MTRUE, "Allocate [%s] working buffer failed.", caller);
            goto lbExit;
        }
    }
    ret = MTRUE;
lbExit:
    return ret;
}
//************************************************************************
//
//************************************************************************
sp<IImageBuffer>
P2B_UT::
loadImgFromFile(
    const IImageBufferAllocator::ImgParam imgParam,
    const char* path,
    const char* name,
    MINT usage)
{
    IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
    sp<IImageBuffer> pImgBuf = allocator->alloc(name, imgParam);
    pImgBuf->loadFromFile(path);
    pImgBuf->lockBuf(name, usage);
    return pImgBuf;
}
//************************************************************************
//
//************************************************************************
sp<IImageBuffer>
P2B_UT::
createEmptyImageBuffer(
    const IImageBufferAllocator::ImgParam imgParam,
    const char* name,
    MINT usage)
{
    IImageBufferAllocator *allocator = IImageBufferAllocator::getInstance();
    sp<IImageBuffer> pImgBuf = allocator->alloc(name, imgParam);
    pImgBuf->lockBuf(name, usage);
    return pImgBuf;
}