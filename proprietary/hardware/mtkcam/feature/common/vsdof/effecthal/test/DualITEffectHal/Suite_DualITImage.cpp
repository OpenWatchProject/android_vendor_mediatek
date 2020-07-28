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

#define LOG_TAG "DualITEffectHalUTTest"

#include <string>
#include <sys/resource.h>
#include <utils/RefBase.h>

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>

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
#include <gtest/gtest.h>
#include <mtkcam/feature/effectHalBase/IEffectHal.h>
#include <mtkcam/feature/effectHalBase/EffectHalBase.h>
#include <mtkcam/feature/stereo/pipe/IBokehPipe.h>
#include <mtkcam/feature/stereo/pipe/vsdof_common.h>
#include <mtkcam/feature/stereo/pipe/vsdof_data_define.h>
//
#include <mtkcam/feature/stereo/effecthal/DualITEffectHal.h>
#include <DebugUtil.h>

using namespace std;
using namespace NSCam;
//************************************************************************
// utility function
//************************************************************************
#define NS_PER_SEC  1000000000
#define NS_PER_MS   1000000
#define NS_PER_US   1000
static void Wait(int ms)
{
  long waitSec;
  long waitNSec;
  waitSec = (ms * NS_PER_MS) / NS_PER_SEC;
  waitNSec = (ms * NS_PER_MS) % NS_PER_SEC;
  struct timespec t;
  t.tv_sec = waitSec;
  t.tv_nsec = waitNSec;
  if( nanosleep(&t, NULL) != 0 )
  {

  }
}
// image
const MSize IMG_SIZE(1920, 1080);
MUINT32 IMG_STRIDES[3] = {1920, 960, 960};
MUINT32 FULL_RAW_BOUNDARY[3] = {0, 0, 0};
const IImageBufferAllocator::ImgParam imgParam_Img(
eImgFmt_YV12, IMG_SIZE, IMG_STRIDES, FULL_RAW_BOUNDARY, 3);
// out image
const MSize OUT_IMG_SIZE(4352, 1152);
MUINT32 OUT_IMG_STRIDES[3] = {4352, 2176, 2176};
const IImageBufferAllocator::ImgParam imgParam_OutImg(
eImgFmt_YV12, OUT_IMG_SIZE, OUT_IMG_STRIDES, FULL_RAW_BOUNDARY, 3);
//
static MVOID
effectHalCb(
    MVOID* tag,
    String8 status,
    sp<NSCam::EffectRequest>& request)
{
    LOG_DBG("1");
    printf("DIT process done");

    LOG_DBG("2");
    NSCam::NSCamFeature::NSFeaturePipe::makePath("/sdcard/dit", 0660);
    sp<EffectFrameInfo> frame = nullptr;
    sp<IImageBuffer> buffer = nullptr;
    string saveFileName;

    LOG_DBG("3");
    for(int i=0;i<request->vOutputFrameInfo.size();++i)
    {
        frame = request->vOutputFrameInfo.valueAt(i);
        frame->getFrameBuffer(buffer);
        saveFileName = "/sdcard/dit/result.yuv";
        buffer->saveToFile(saveFileName.c_str());
    }

    LOG_DBG("4");
}
//
//************************************************************************
//
//************************************************************************
sp<IImageBuffer>
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
//
TEST(BokehEffectHalUT, GF_UT)
{
    NSCam::NSCamFeature::NSFeaturePipe::makePath("/sdcard/dit", 0660);
    sp<IImageBuffer> pLeftImgBuffer =
                loadImgFromFile(
                    imgParam_Img,
                    "/sdcard/vsdof/bokeh_ut/data/mtk_scene2_00001Orig_M_1920x1080.yuv",
                    "MainImg_0",
                    eBUFFER_USAGE_HW_MASK|eBUFFER_USAGE_HW_CAMERA_READWRITE|eBUFFER_USAGE_SW_READ_OFTEN|eBUFFER_USAGE_SW_WRITE_RARELY);
    pLeftImgBuffer->saveToFile("/sdcard/dit/left.yuv");
    printf("0x%x 0x%x 0x%x\n", pLeftImgBuffer->getBufPA(0), pLeftImgBuffer->getBufPA(1), pLeftImgBuffer->getBufPA(2));
    sp<IImageBuffer> pRightImgBuffer =
                loadImgFromFile(
                    imgParam_Img,
                    "/sdcard/vsdof/bokeh_ut/data/mtk_scene2_00010Orig_M_1920x1080.yuv",
                    "MainImg_1",
                    eBUFFER_USAGE_HW_MASK|eBUFFER_USAGE_HW_CAMERA_READWRITE|eBUFFER_USAGE_SW_READ_OFTEN|eBUFFER_USAGE_SW_WRITE_RARELY);
    pRightImgBuffer->saveToFile("/sdcard/dit/right.yuv");
    printf("0x%x 0x%x 0x%x\n", pRightImgBuffer->getBufPA(0), pRightImgBuffer->getBufPA(1), pRightImgBuffer->getBufPA(2));
    sp<IImageBuffer> pOutputImgBuffer =
                createEmptyImageBuffer(
                    imgParam_OutImg,
                    "JPS_Out",
                    eBUFFER_USAGE_SW_READ_MASK | eBUFFER_USAGE_HW_CAMERA_READ | eBUFFER_USAGE_HW_TEXTURE);
    MINT const readUsage = eBUFFER_USAGE_SW_READ_MASK | eBUFFER_USAGE_HW_CAMERA_READ | eBUFFER_USAGE_HW_TEXTURE;
    MINT const usage = eBUFFER_USAGE_SW_READ_MASK | eBUFFER_USAGE_HW_CAMERA_READ | eBUFFER_USAGE_HW_TEXTURE;
    printf("test 0x%x\n", !(usage & ~readUsage));
    pOutputImgBuffer->saveToFile("/sdcard/dit/out.yuv");
    printf("0x%x 0x%x 0x%x\n", pOutputImgBuffer->getBufPA(0), pOutputImgBuffer->getBufPA(1), pOutputImgBuffer->getBufPA(2));
    DualITEffectHal *ditHal = new DualITEffectHal();
    ditHal->init();
    ditHal->configure();
    ditHal->prepare();
    ditHal->start();
    // prepate request
    sp<EffectFrameInfo> leftImgFrame = new EffectFrameInfo(
                                                0,
                                                DualITEffectHal::DIT_BUF_JPS_LEFT,
                                                nullptr,
                                                nullptr);
    leftImgFrame->setFrameBuffer(pLeftImgBuffer);
    sp<EffectFrameInfo> rightImgFrame = new EffectFrameInfo(
                                                0,
                                                DualITEffectHal::DIT_BUF_JPS_RIGHT,
                                                nullptr,
                                                nullptr);
    rightImgFrame->setFrameBuffer(pRightImgBuffer);
    sp<EffectFrameInfo> outImgFrame = new EffectFrameInfo(
                                                0,
                                                DualITEffectHal::DIR_BUF_JPS_OUTPUT,
                                                nullptr,
                                                nullptr);
    outImgFrame->setFrameBuffer(pOutputImgBuffer);
    EffectRequestPtr pRequest = new EffectRequest(0, effectHalCb, NULL);
    pRequest->vInputFrameInfo.add(DualITEffectHal::DIT_BUF_JPS_LEFT, leftImgFrame);
    pRequest->vInputFrameInfo.add(DualITEffectHal::DIT_BUF_JPS_RIGHT, rightImgFrame);
    pRequest->vOutputFrameInfo.add(DualITEffectHal::DIR_BUF_JPS_OUTPUT, outImgFrame);
    ditHal->updateEffectRequest(pRequest);
    //
    ditHal->abort();
    ditHal->release();
    ditHal->unconfigure();
    ditHal->uninit();
    delete ditHal;
    ditHal = NULL;
}