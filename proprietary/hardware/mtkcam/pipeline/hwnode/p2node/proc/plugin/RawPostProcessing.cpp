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

#define LOG_TAG "MtkCam/P2Node_PLUGRaw"

#include "RawPostProcessing.h"
#include "../PluginProcedure.h"
#include <mtkcam/utils/std/DebugScanLine.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/utils/hw/IPlugProcessing.h>
#include <MTKDngOp.h>
// test mode: 0=> in-place processing, 1=> in-out processing
//  => in-place: do a software NR and draw a thick line in the middle of image
//  => in-out: memcpy the source image and draw a thick line in the middle of image
#define TEST_MODE (1)
static MTKDngOp *MyDngop;
static DngOpResultInfo MyDngopResultInfo;
static DngOpImageInfo MyDngopImgInfo;

#define INPK_TO_UPK   (0)
#define UPK_OUT (1)

// auto-mount raw processing
REGISTER_POSTPROCESSING(Raw, RawPostProcessing);

/******************************************************************************
 *
 ******************************************************************************/
RawPostProcessing::
RawPostProcessing(MUINT32 const openId)
        : PostProcessing(openId)
{
}


/******************************************************************************
 *
 ******************************************************************************/
RawPostProcessing::
~RawPostProcessing() {
}


/******************************************************************************
 *
 ******************************************************************************/
PostProcessing::ProcessingProfile&
RawPostProcessing::
profile() {
#if TEST_MODE == 0
    static PostProcessing::ProcessingProfile profile(
            eIN_PLACE_PROCESSING,
            eIMG_FMT_DEFAULT);
#elif TEST_MODE == 1
    static PostProcessing::ProcessingProfile profile(
            eIN_OUT_PROCESSING,
            eIMG_FMT_DEFAULT,
            #if UPK_OUT
            eIMG_FMT_BAYER10_UNPAK
            #else
            eIMG_FMT_DEFAULT
            #endif
            );

#else
#error unsupported this test mode
#endif
    return profile;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
RawPostProcessing::
doOrNot(PreConditionParams const& params) {
    if (!params.bResized) {
        android::sp<IPlugProcessing> pPlugProcessing = IPlugProcessing::createInstance((MUINT32)IPlugProcessing::PLUG_ID_FCELL, (NSCam::IPlugProcessing::DEV_ID)muOpenId);
        MBOOL ret = false;
        pPlugProcessing->sendCommand(NSCam::NSCamPlug::ACK, (MINTPTR)&ret);
        return ret;
    }

    return MFALSE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
RawPostProcessing::
process(ProcessingParams const& param) {

    MINT32 logLevel = ::property_get_int32("debug.camera.p2plug.log", 0);
    static int photo_count = 0;
    IImageBuffer *pInBuf = param.pInBuffer;
    void *pInVa    = (void *) (pInBuf->getBufVA(0));
    int nImgWidth  = pInBuf->getImgSize().w;
    int nImgHeight = pInBuf->getImgSize().h;
    int nBufSize   = pInBuf->getBufSizeInBytes(0);
    int nImgStride = pInBuf->getBufStridesInBytes(0);

#if TEST_MODE == 0
    DebugScanLine *mpDebugScanLine = DebugScanLine::createInstance();
    MY_LOGD("in-place processing. va:0x%x", pInVa);
    for (int i = 0; i < 5; i++)
        mpDebugScanLine->drawScanLine(nImgWidth, nImgHeight, pInVa, nBufSize, nImgStride);
    if (mpDebugScanLine != NULL) {
        mpDebugScanLine->destroyInstance();
        mpDebugScanLine = NULL;
    }
#elif TEST_MODE == 1
    IImageBuffer *pUpkOutBuf = nullptr;
    void *pUpkOutVa = nullptr;
    sp<BufferHandle> pUpkBufferHandle = nullptr;
    if(pInBuf->getImgFormat() == eImgFmt_BAYER10_UNPAK) {
        pUpkOutBuf = pInBuf;
        pUpkOutVa = pInVa;
    } else // pack in
    {
        pUpkBufferHandle = WorkingBufferHandle::create("Fcell", eImgFmt_BAYER10_UNPAK, pInBuf->getImgSize());
        pUpkOutBuf = pUpkBufferHandle->getBuffer();
        pUpkOutVa = (void *) (pUpkOutBuf->getBufVA(0));
        // unpack algorithm
        MY_LOGD("Unpack +");
        MyDngop = MyDngop->createInstance(DRV_DNGOP_UNPACK_OBJ_SW);
        MyDngopImgInfo.Width = nImgWidth;
        MyDngopImgInfo.Height = nImgHeight;
        MyDngopImgInfo.Stride_src = nImgStride;
        MyDngopImgInfo.Stride_dst = pUpkOutBuf->getBufStridesInBytes(0);
        MyDngopImgInfo.BIT_NUM = 10;
        MyDngopImgInfo.BIT_NUM_DST = 10;
        MUINT32 buf_size = DNGOP_BUFFER_SIZE(nImgWidth * 2, nImgHeight);
        MyDngopImgInfo.Buff_size = buf_size;
        MyDngopImgInfo.srcAddr = pInVa;
        MyDngopResultInfo.ResultAddr = pUpkOutVa;
        MyDngop->DngOpMain((void*)&MyDngopImgInfo, (void*)&MyDngopResultInfo);
        MyDngop->destroyInstance(MyDngop);
        MY_LOGD("Unpack -");
        MY_LOGD("unpack processing. va[in]:%p, va[out]:%p", MyDngopImgInfo.srcAddr, MyDngopResultInfo.ResultAddr);
        MY_LOGD("img size(%dx%d) src stride(%d) bufSize(%d) -> dst stride(%d) bufSize(%zu)", nImgWidth, nImgHeight,
                  MyDngopImgInfo.Stride_src,nBufSize, MyDngopImgInfo.Stride_dst , pUpkOutBuf->getBufSizeInBytes(0));
    }

    //Step 1: Init Fcell Library
    android::sp<IPlugProcessing> pPlugProcessing = IPlugProcessing::createInstance((MUINT32)IPlugProcessing::PLUG_ID_FCELL, (NSCam::IPlugProcessing::DEV_ID)muOpenId);
    PlugInitParam initParam;
    initParam.openId = param.iOpenId;
    initParam.img_w = nImgWidth;
    initParam.img_h = nImgHeight;
    pPlugProcessing->sendCommand(NSCam::NSCamPlug::SET_PARAM, NSCam::IPlugProcessing::PARAM_INIT, (MINTPTR)&initParam);
    MERROR res = OK;
    res = pPlugProcessing->init(IPlugProcessing::OP_MODE_SYNC);
    if(res == OK)
        MY_LOGD("REMOSAIC - Libinit");
    pPlugProcessing->waitInitDone();

    //Step 2: Prepare parameters to call Fcell library
#if !UPK_OUT
    sp<BufferHandle> pFcellUpkBufferHandle = WorkingBufferHandle::create("Fcell", eImgFmt_BAYER10_UNPAK, pInBuf->getImgSize());
    IImageBuffer *pFcellUpkOutBuf = pFcellUpkBufferHandle->getBuffer();
    void *pFcellUpkOutVa = (void *) (pFcellUpkOutBuf->getBufVA(0));
    MY_LOGD("fcell processing: src buffer size(%zu) dst buffer size(%zu)",pUpkOutBuf->getBufSizeInBytes(0), pFcellUpkOutBuf->getBufSizeInBytes(0));
#elif UPK_OUT
    IImageBuffer *pFcellUpkOutBuf = param.pOutBuffer;
    void *pOutVa = (void *) (pFcellUpkOutBuf->getBufVA(0));
    void *pFcellUpkOutVa = pOutVa;
#endif

    const char * fcellImgFile = "fcell_src.raw";
    const char * outImgFile = "sdcard/fcellfull.raw";

    int16_t *pFcellImage = static_cast<int16_t*>(pUpkOutVa);
    if(pFcellImage == NULL)
    {
        MY_LOGE("pFcellImage allocate fail.");
    }
    int16_t *pOutImage = static_cast<int16_t*>(pFcellUpkOutVa);
    if(pOutImage == NULL)
    {
        MY_LOGE("pOutImage allocate fail.");
    }

    /*FILE *fp0 = fopen(fcellImgFile,"rb");
    if(fp0)
    {
        int imgsize = pUpkOutBuf->getBufSizeInBytes(0);
        fread(pFcellImage, imgsize, 1,fp0);
        fclose(fp0);
    }
    else
    {
        MY_LOGE("Cannot open fcell image file %s\n", fcellImgFile);
        delete[] pFcellImage;
        delete[] pOutImage;
        return -1;
    }*/
    if(logLevel >1 ){
        MY_LOGD("DEBUG: fcellprocess dump");
        char filename[256] = {0};
        sprintf(filename, "sdcard/fcell_src_%dx%d_%zu_%d.raw",
                pUpkOutBuf->getImgSize().w, pUpkOutBuf->getImgSize().h, pUpkOutBuf->getBufStridesInBytes(0),photo_count);
        pUpkOutBuf->saveToFile(filename);
        MY_LOGD("[raw][in] fcell src(%dx%d) fmt(0x%x) count(%d)",
                pUpkOutBuf->getImgSize().w, pUpkOutBuf->getImgSize().h,
                pUpkOutBuf->getImgFormat(),photo_count);
    }
    //Step 3: Process Fcell.
    struct timeval start, end;
    gettimeofday( &start, NULL );
    MY_LOGD("fcellprocess begin");
    PlugProcessingParam ProcessingParam;
    ProcessingParam.src_buf_fd = pUpkOutBuf->getFD(0);
    ProcessingParam.dst_buf_fd = pFcellUpkOutBuf->getFD(0);
    ProcessingParam.img_w = nImgWidth;
    ProcessingParam.img_h = nImgHeight;
    ProcessingParam.src_buf_size = pUpkOutBuf->getBufSizeInBytes(0);
    ProcessingParam.dst_buf_size = pFcellUpkOutBuf->getBufSizeInBytes(0);
    ProcessingParam.src_buf = (unsigned short* )pFcellImage;
    ProcessingParam.dst_buf = (unsigned short* )pOutImage;
    {
        int analog_gain = 0, awb_rgain = 0, awb_ggain = 0, awb_bgain = 0;
        bool ret = 1;
        ret &= tryGetMetadata<MINT32>(param.pInHalMeta, MTK_ANALOG_GAIN, analog_gain);
        ret &= tryGetMetadata<MINT32>(param.pInHalMeta, MTK_AWB_RGAIN, awb_rgain);
        ret &= tryGetMetadata<MINT32>(param.pInHalMeta, MTK_AWB_GGAIN, awb_ggain);
        ret &= tryGetMetadata<MINT32>(param.pInHalMeta, MTK_AWB_BGAIN, awb_bgain);
        MY_LOGD("ret = %d, analog_gain = %d, awb_rgain = %d, awb_ggain = %d, awb_bgain = %d", ret,  analog_gain, awb_rgain, awb_ggain, awb_bgain);
        ProcessingParam.gain_awb_r = awb_rgain;
        ProcessingParam.gain_awb_gr = awb_ggain;
        ProcessingParam.gain_awb_gb = awb_ggain;
        ProcessingParam.gain_awb_b = awb_bgain;
        ProcessingParam.gain_analog = analog_gain;
    }

    if(pPlugProcessing->sendCommand(NSCam::NSCamPlug::PROCESS, (MINTPTR)(&ProcessingParam), (MINTPTR)0, (MINTPTR)0, (MINTPTR)0) != OK)
    {
        MY_LOGE("fcellprocess fails!!!\n");
        return -1;
    }
    gettimeofday( &end, NULL );
    MY_LOGD("fcell process finish, time: %ld ms.\n", 1000 * ( end.tv_sec - start.tv_sec ) + (end.tv_usec - start.tv_usec)/1000);

    //Step 4: Save buffer
    if(logLevel >1 ){
        char filename[256] = {0};
        sprintf(filename, "sdcard/fcell_dst_%dx%d_%zu_%d.raw",
                pFcellUpkOutBuf->getImgSize().w, pFcellUpkOutBuf->getImgSize().h, pFcellUpkOutBuf->getBufStridesInBytes(0),photo_count);
        pFcellUpkOutBuf->saveToFile(filename);
        MY_LOGDO("[raw][out] fcell dst(%dx%d) fmt(0x%x) count(%d)",
                pFcellUpkOutBuf->getImgSize().w, pFcellUpkOutBuf->getImgSize().h,
                pFcellUpkOutBuf->getImgFormat(),photo_count);
        photo_count++;
    }

#if !UPK_OUT
    // pack algorithm
    IImageBuffer *pOutBuf = param.pOutBuffer;
    void *pOutVa = (void *) (pOutBuf->getBufVA(0));
    MY_LOGD("Pack +");
    MyDngop = MyDngop->createInstance(DRV_DNGOP_PACK_OBJ_SW);
    MyDngopImgInfo.Width = nImgWidth;
    MyDngopImgInfo.Height = nImgHeight;
    MyDngopImgInfo.Stride_src = nImgWidth * 2;
    MyDngopImgInfo.Stride_dst = pOutBuf->getBufStridesInBytes(0);
    int bit_depth = 10;
    MyDngopImgInfo.BIT_NUM = bit_depth;
    MyDngopImgInfo.Bit_Depth = bit_depth;
    buf_size = DNGOP_BUFFER_SIZE(pOutBuf->getBufStridesInBytes(0), nImgHeight);
    MyDngopImgInfo.Buff_size = buf_size;
    MyDngopImgInfo.srcAddr = pFcellUpkOutVa;
    MyDngopResultInfo.ResultAddr = pOutVa;
    MyDngop->DngOpMain((void*)&MyDngopImgInfo, (void*)&MyDngopResultInfo);
    MyDngop->destroyInstance(MyDngop);
    MY_LOGD("Pack -");

    MY_LOGD("pack processing. va[in]:%p, va[out]:%p", MyDngopImgInfo.srcAddr, MyDngopResultInfo.ResultAddr);
    MY_LOGD("img size(%dx%d) src stride(%d) bufSize(%d) -> dst stride(%d) bufSize(%zu)", nImgWidth, nImgHeight,
              MyDngopImgInfo.Stride_src,MyDngopImgInfo.Buff_size, MyDngopImgInfo.Stride_dst , pOutBuf->getBufSizeInBytes(0));
#endif
    param.pOutBuffer->syncCache(eCACHECTRL_FLUSH);

    if(res != ALREADY_EXISTS) {
        MY_LOGD("REMOSAIC - LibUninit");
        CAM_TRACE_BEGIN("REMOSAIC - LibDeinit");
        pPlugProcessing->uninit(); //shot
        CAM_TRACE_END();
    }
#endif

    return MTRUE;
}


