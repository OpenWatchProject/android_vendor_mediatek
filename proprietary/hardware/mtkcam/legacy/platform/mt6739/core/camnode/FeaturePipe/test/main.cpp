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
 * MediaTek Inc. (C) 2017. All rights reserved.
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


#include <mtkcam/common.h>
#include <mtkcam/utils/common.h>

#include <mtkcam/ImageFormat.h>
#include <mtkcam/utils/Format.h>

#include <utils/StrongPointer.h>
using namespace android;

#include "../NativeBufferWrapper.h"
#include "MTKWarp.h"
#include "MTKFeatureSet.h"
using namespace NSCam::NSCamFeature::NSFeaturePipe;

#define FULLIMG_BUFFER_NUM 3
#define WARPOUT_BUFFER_NUM 3

#define WARPMAP_MAX_W      320
#define WARPMAP_MAX_H      320

#define FULLIMG_BUFFER_WIDTH  2752
#define FULLIMG_BUFFER_HEIGHT 2752

#define WARPOUT_BUFFER_WIDTH   2304
#define WARPOUT_BUFFER_HEIGHT  1306


//
#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__
//
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt,  __func__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt,  __func__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt,  __func__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt,  __func__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt,  __func__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt,  __func__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt,  __func__, ##arg)


#if 0
#define FUNC_START()     MY_LOGD("+")
#define FUNC_END()       MY_LOGD("-")
#define FUNC_NAME()      MY_LOGD("")
#else
#define FUNC_START()
#define FUNC_END()
#define FUNC_NAME()
#endif


#define MODULE_NAME "NativeBufferWrapper_UT"

#undef LOG_TAG
#define LOG_TAG "NativeBufferWrapper_UT"


MVOID Test_SimpleNativeBuffer()
{
    FUNC_START();

    sp<NativeBufferWrapper> mFullImage[3];
    sp<NativeBufferWrapper> mWarpOut[3];

    // Allocate
    for( MUINT32 i = 0; i < FULLIMG_BUFFER_NUM; i++ )
    {
        mFullImage[i] = new NativeBufferWrapper(FULLIMG_BUFFER_WIDTH, FULLIMG_BUFFER_HEIGHT,
                                                HAL_PIXEL_FORMAT_YV12, NativeBufferWrapper::USAGE_HW_TEXTURE,
                                                "fullImg" + std::to_string(i));
    }
    for( MUINT32 i = 0; i < WARPOUT_BUFFER_NUM; i++ )
    {
        mWarpOut[i] = new NativeBufferWrapper(WARPOUT_BUFFER_WIDTH, WARPOUT_BUFFER_HEIGHT,
                                              HAL_PIXEL_FORMAT_YV12, NativeBufferWrapper::USAGE_HW_RENDER,
                                              "warpout" + std::to_string(i));
    }

    Warp_SPTR srcGBArray[FULLIMG_BUFFER_NUM];
    Warp_SPTR dstGBArray[WARPOUT_BUFFER_NUM];

    // Do some operation
    for( MUINT32 i = 0; i < FULLIMG_BUFFER_NUM; i++ )
    {
        MUINT32* virAddr;
        srcGBArray[i] = mFullImage[i]->getWarp_SPTR();
        mFullImage[i]->lock(NativeBufferWrapper::USAGE_SW, (void**)(&virAddr));
        mFullImage[i]->unlock();
    }
    for( MUINT32 i = 0; i < WARPOUT_BUFFER_NUM; i++ )
    {
        MUINT32* virAddr;
        dstGBArray[i] = mWarpOut[i]->getWarp_SPTR();
        mWarpOut[i]->lock(NativeBufferWrapper::USAGE_SW, (void**)(&virAddr));
        mWarpOut[i]->unlock();
    }

    // Destroy
    MY_LOGD("Destroy mFullImage");
    for( MUINT32 i = 0; i < FULLIMG_BUFFER_NUM; i++ )
    {
        mFullImage[i]->unlock();
        mFullImage[i] = NULL;
    }
    MY_LOGD("Destroy mWarpOut");
    for( MUINT32 i = 0; i < WARPOUT_BUFFER_NUM; i++ )
    {
        mWarpOut[i]->unlock();
        mWarpOut[i] = NULL;
    }

    FUNC_END();
}


MVOID Test_NativeBufferGPUWarp()
{
    FUNC_START();

    sp<NativeBufferWrapper> mFullImage[3];
    sp<NativeBufferWrapper> mWarpOut[3];

    MTKWarp* mpGpuWarp = MTKWarp::createInstance(DRV_WARP_OBJ_GLES);

    for( MUINT32 i = 0; i < FULLIMG_BUFFER_NUM; i++ )
    {
        mFullImage[i] = new NativeBufferWrapper(FULLIMG_BUFFER_WIDTH, FULLIMG_BUFFER_HEIGHT,
                                                HAL_PIXEL_FORMAT_YV12, NativeBufferWrapper::USAGE_HW_TEXTURE,
                                                "fullImg" + std::to_string(i));
    }
    for( MUINT32 i = 0; i < WARPOUT_BUFFER_NUM; i++ )
    {
        mWarpOut[i] = new NativeBufferWrapper(WARPOUT_BUFFER_WIDTH, WARPOUT_BUFFER_HEIGHT,
                                              HAL_PIXEL_FORMAT_YV12, NativeBufferWrapper::USAGE_HW_RENDER,
                                              "warpout" + std::to_string(i));
    }

    MUINT32 passWarp[2][4];
    MY_LOGD("Init warpMap");
    passWarp[0][0] = 0;
    passWarp[0][1] = (FULLIMG_BUFFER_WIDTH-1) * 16.0;
    passWarp[0][2] = 0;
    passWarp[0][3] = (FULLIMG_BUFFER_WIDTH-1) * 16.0;
    passWarp[1][0] = 0;
    passWarp[1][1] = 0;
    passWarp[1][2] = (FULLIMG_BUFFER_HEIGHT-1) * 16.0;
    passWarp[1][3] = (FULLIMG_BUFFER_HEIGHT-1) * 16.0;

    WarpImageExtInfo init_warp_info;
    GpuTuning gpu_tuning;
    Warp_SPTR srcGBArray[FULLIMG_BUFFER_NUM];
    Warp_SPTR dstGBArray[WARPOUT_BUFFER_NUM];

    MUINT8* warpWorkingBuffer = NULL;

    for( MUINT32 i = 0; i < FULLIMG_BUFFER_NUM; i++ )
    {
        srcGBArray[i] = mFullImage[i]->getWarp_SPTR();
    }
    for( MUINT32 i = 0; i < WARPOUT_BUFFER_NUM; i++ )
    {
        dstGBArray[i] = mWarpOut[i]->getWarp_SPTR();
    }
    gpu_tuning.GLESVersion = 3;
    gpu_tuning.Demo = 0;

    init_warp_info.Features = 0;
    ADD_FEATURE(init_warp_info.Features, MTK_EIS2_BIT);
    init_warp_info.ImgFmt = WARP_IMAGE_YV12;
    init_warp_info.OutImgFmt = WARP_IMAGE_YV12;

    init_warp_info.SrcGraphicBuffer = (void*) &srcGBArray;
    init_warp_info.DstGraphicBuffer = (void*) &dstGBArray;

    init_warp_info.InputGBNum = FULLIMG_BUFFER_NUM;
    init_warp_info.OutputGBNum = WARPOUT_BUFFER_NUM;

    init_warp_info.Width = FULLIMG_BUFFER_WIDTH;
    init_warp_info.Height = FULLIMG_BUFFER_HEIGHT;
    init_warp_info.pTuningPara = &gpu_tuning;

    init_warp_info.WarpMapNum = 1;
    init_warp_info.WarpMatrixNum = 0;

    init_warp_info.MaxWarpMapSize[0] = WARPMAP_MAX_W;
    init_warp_info.MaxWarpMapSize[1] = WARPMAP_MAX_H;

    // Init
    MRESULT mret = mpGpuWarp->WarpInit((MUINT32*) &init_warp_info, (MUINT32*) NULL);
    if( mret != S_WARP_OK )
    {
        MY_LOGE("MTKWarp init failed! (%d)", mret);
        // TODO: error handling
    }

    unsigned int warpWorkBufSize;

    mret = mpGpuWarp->WarpFeatureCtrl(WARP_FEATURE_GET_WORKBUF_SIZE, NULL, &warpWorkBufSize);
    if( mret != S_WARP_OK )
    {
        MY_LOGE("MTKWarp WARP_FEATURE_GET_WORKBUF_ADDR failed! (%d)", mret);
        // TODO: error handling
    }

    if( warpWorkingBuffer == NULL )
    {
        warpWorkingBuffer = (MUINT8*) malloc(warpWorkBufSize);
        if( warpWorkingBuffer == NULL )
        {
            MY_LOGE("Warp working buffer allocation failed!");
        }
    }

    mret = mpGpuWarp->WarpFeatureCtrl(WARP_FEATURE_SET_WORKBUF_ADDR, &warpWorkingBuffer, NULL);
    if (mret != S_WARP_OK)
    {
        MY_LOGE("MTKWarp WARP_FEATURE_SET_WORKBUF_ADDR failed! (%d)", mret);
    }

    // Perframe
    for( MUINT32 i = 0; i < 100; i++ )
    {
        MY_LOGD("Warp Test # %d iteration", i);
        WarpImageExtInfo warp_info;
        GpuTuning gpu_tuning;

        MUINT32 cur_src = std::rand() % FULLIMG_BUFFER_NUM;
        MUINT32 cur_dst = std::rand() % WARPOUT_BUFFER_NUM;
        MY_LOGD("Select cur_src(%d), cur_dst(%d)", cur_src, cur_dst);
        Warp_SPTR cur_srcGB = srcGBArray[cur_src];
        Warp_SPTR cur_dstGB = srcGBArray[cur_dst];

        warp_info.pTuningPara = &gpu_tuning;
        warp_info.ClipWidth   = FULLIMG_BUFFER_WIDTH;// IP Base should set warpOut Size
        warp_info.ClipHeight  = FULLIMG_BUFFER_HEIGHT;// IP Base should set warpOut Size
        warp_info.Width       = FULLIMG_BUFFER_WIDTH;
        warp_info.Height      = FULLIMG_BUFFER_HEIGHT;
        warp_info.WarpMapSize[0][0] = 2;
        warp_info.WarpMapSize[0][1] = 2;
        warp_info.WarpMapAddr[0][0] = passWarp[0];
        warp_info.WarpMapAddr[0][1] = passWarp[1];
        warp_info.SrcGraphicBuffer  = (void*) cur_srcGB;
        warp_info.DstGraphicBuffer  = (void*) cur_dstGB;

        mret = mpGpuWarp->WarpFeatureCtrl(WARP_FEATURE_ADD_IMAGE, &warp_info, NULL);
        if( mret != S_WARP_OK )
        {
            MY_LOGE("MTKWarp WARP_FEATURE_ADD_IMAGE failed! (%d)", mret);
        }

        MY_LOGD("WarpMain: feature(0x%08x),input(%d,%d),output(%d,%d),warp map(%d,%d),addr(%p,%p)",warp_info.Features,warp_info.Width,warp_info.Height, warp_info.ClipWidth, warp_info.ClipHeight, warp_info.WarpMapSize[0][0], warp_info.WarpMapSize[0][1], warp_info.WarpMapAddr[0][0], warp_info.WarpMapAddr[0][1]);

        MUINT32* virAddr;
        NativeBufferWrapper::unlock((Warp_SPTR)(warp_info.DstGraphicBuffer));
        mret = mpGpuWarp->WarpMain();
        NativeBufferWrapper::lock((Warp_SPTR)(warp_info.DstGraphicBuffer), NativeBufferWrapper::USAGE_SW, (void**)(&virAddr));

        if( mret != S_WARP_OK )
        {
            MY_LOGE("MTKWarp WarpMain failed! (%d)", mret);
        }
    }

    if( mpGpuWarp != NULL )
    {
        MY_LOGD("Destroy mpGpuWarp");
        mpGpuWarp->WarpReset();
        mpGpuWarp->destroyInstance(mpGpuWarp);
    }

    MY_LOGD("Destroy mFullImage");
    for( MUINT32 i = 0; i < FULLIMG_BUFFER_NUM; i++ )
    {
        mFullImage[i]->unlock();
        mFullImage[i] = NULL;
    }
    MY_LOGD("Destroy mWarpOut");
    for( MUINT32 i = 0; i < WARPOUT_BUFFER_NUM; i++ )
    {
        mWarpOut[i]->unlock();
        mWarpOut[i] = NULL;
    }

    if( warpWorkingBuffer != NULL )
    {
        MY_LOGD("Destroy warpWorkingBuffer");
        free(warpWorkingBuffer);
    }

    FUNC_END();
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    Test_SimpleNativeBuffer();
    Test_NativeBufferGPUWarp();

    return 0;
}


