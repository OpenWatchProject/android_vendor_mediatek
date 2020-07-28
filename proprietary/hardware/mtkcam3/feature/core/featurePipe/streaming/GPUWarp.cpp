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

#include "GPUWarp.h"

#include "DebugControl.h"
#define PIPE_CLASS_TAG "GPUWarp"
#define PIPE_TRACE TRACE_GPU_WARP
#include <featurePipe/core/include/PipeLog.h>
#include <featurePipe/core/include/DebugUtil.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_STREAMING);

#define NUM_REGISTER_SLOT 50 // defined by MTKWarp

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

GPUWarp::GPUWarp()
    : mpGpuWarp(NULL)
    , mWorkBufSize(0)
    , mWorkBuf(NULL)
{
    TRACE_FUNC_ENTER();

    mForceUseRGBA = getPropertyValue(KEY_FORCE_GPU_RGBA, VAL_FORCE_GPU_RGBA);

    TRACE_FUNC_EXIT();
}

GPUWarp::~GPUWarp()
{
    TRACE_FUNC_ENTER();
    cleanUp();
    TRACE_FUNC_EXIT();
}

MBOOL GPUWarp::onInit()
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    ret = initWarp();
    if( !ret )
    {
        cleanUp();
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MVOID GPUWarp::onUninit()
{
    TRACE_FUNC_ENTER();
    cleanUp();
    TRACE_FUNC_EXIT();
}

MBOOL GPUWarp::onProcessWarp(const ImgBuffer &in, const ImgBuffer &out, const ImgBuffer &warpMap, const MSize &inSize, const MSize &outSize)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    struct WarpImageExtInfo warpInfo;
    GpuTuning gpuTuning;
    IImageBuffer *warpBuffer = NULL;
    NB_SPTR inGB = NULL, outGB = NULL;

    if( prepareGB(in, out, inGB, outGB) &&
        registerGB(inGB, outGB) &&
        prepareWarp(warpMap, warpBuffer) )
    {
        configBasicWarpInfo(&warpInfo, &gpuTuning);
        configWarpMapInfo(warpInfo, warpBuffer);

        warpInfo.Width = inSize.w;
        warpInfo.Height = inSize.h;
        warpInfo.ClipWidth = outSize.w;
        warpInfo.ClipHeight = outSize.h;
        warpInfo.WarpLevel = 0;

        warpInfo.SrcGraphicBuffer = (void*)inGB;
        warpInfo.DstGraphicBuffer = (void*)outGB;

        TRACE_FUNC("WarpInfoSize(%dx%d),Clip(%dx%d),Warp(%d),Src(@%p),Dst(@%p)",
                   warpInfo.Width, warpInfo.Height, warpInfo.ClipWidth, warpInfo.ClipHeight,
                   warpInfo.WarpLevel, warpInfo.SrcGraphicBuffer, warpInfo.DstGraphicBuffer);
        char *gfxBuf = NULL;
        unlockNativeBuffer(outGB);
        ret = doWarp(warpInfo);
        lockNativeBuffer(outGB, NativeBufferWrapper::USAGE_SW, (void**)(&gfxBuf));
    }

    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL GPUWarp::initWarp()
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    struct WarpImageExtInfo warpInfo;
    GpuTuning gpuTuning;

    configInitWarpInfo(&warpInfo, &gpuTuning);
    warpInfo.SrcGraphicBuffer = NULL;
    warpInfo.DstGraphicBuffer = NULL;
    warpInfo.InputGBNum = 0;
    warpInfo.OutputGBNum = 0;

    ret = createWarpObj() && initWarpObj(warpInfo) && initWorkBuffer(warpInfo);

    TRACE_FUNC_EXIT();
    return ret;
}

MVOID GPUWarp::configInitWarpInfo(struct WarpImageExtInfo *warpInfo, GpuTuning *gpuTuning)
{
    TRACE_FUNC_ENTER();
    MSize maxImageSize = getMaxImageSize();
    configBasicWarpInfo(warpInfo, gpuTuning);
    warpInfo->Width = maxImageSize.w;
    warpInfo->Height = maxImageSize.h;
    TRACE_FUNC_EXIT();
}

MVOID GPUWarp::configBasicWarpInfo(struct WarpImageExtInfo *warpInfo, GpuTuning *gpuTuning)
{
    TRACE_FUNC_ENTER();
    MSize maxWarpSize = getMaxWarpSize();

    warpInfo->pTuningPara = gpuTuning;
    warpInfo->pTuningPara->GLESVersion = 3;
    warpInfo->pTuningPara->Demo = 0;

    warpInfo->Features = 0;
    //ADD_FEATURE(warpInfo->Features, MTK_EIS2_BIT);

    warpInfo->ImgFmt = WARP_IMAGE_YV12;
    warpInfo->OutImgFmt = mForceUseRGBA ? WARP_IMAGE_RGBA8888 : WARP_IMAGE_YV12;
    warpInfo->WarpMatrixNum = 0;
    warpInfo->WarpMapNum = 1;
    warpInfo->MaxWarpMapSize[0] = maxWarpSize.w;
    warpInfo->MaxWarpMapSize[1] = maxWarpSize.h;
    warpInfo->WorkingBuffAddr = (MUINT8*)mWorkBuf;
    warpInfo->DisableEGLImageInit = 1; // RUN_TIME_GB_REGISTER
    TRACE_FUNC_EXIT();
}

MBOOL GPUWarp::createWarpObj()
{
    TRACE_FUNC_ENTER();
    if( (mpGpuWarp = MTKWarpExt::createInstance(DRV_WARP_OBJ_GLES)) == NULL )
    {
        MY_LOGE("MTKWarp::createInstance failed");
    }
    TRACE_FUNC_EXIT();
    return mpGpuWarp != NULL;
}

MBOOL GPUWarp::initWarpObj(struct WarpImageExtInfo &warpInfo)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;
    MRESULT mret;
    if( !mpGpuWarp )
    {
        MY_LOGE("Invalid GpuWarp object state");
        ret = MFALSE;
    }
    else if( (mret = mpGpuWarp->WarpInit((MUINT32*) &warpInfo, NULL)) != S_WARP_OK )
    {
        MY_LOGE("MTKWarp init failed(%d).", -mret);
        ret = MFALSE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL GPUWarp::initWorkBuffer(struct WarpImageExtInfo &warpInfo)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    MRESULT mret;
    if( !mpGpuWarp )
    {
        MY_LOGE("Invalid GpuWarp object state");
    }
    else if( (mret = mpGpuWarp->WarpFeatureCtrl(WARP_FEATURE_GET_WORKBUF_SIZE, NULL, &mWorkBufSize)) != S_WARP_OK )
    {
        MY_LOGE("MTKWarp WARP_FEATURE_GET_WORKBUF_ADDR failed! (%d)", -mret);
    }
    else if(mWorkBufSize > 0 && (mWorkBuf = (MUINT8*)malloc(mWorkBufSize*sizeof(unsigned char))) == NULL )
    {
        MY_LOGE("OOM: Warp working buffer allocation failed!");
    }
    else
    {
        warpInfo.WorkingBuffAddr = (MUINT8*)mWorkBuf;
        if( (mret = mpGpuWarp->WarpFeatureCtrl(WARP_FEATURE_SET_WORKBUF_ADDR, &warpInfo, NULL)) != S_WARP_OK )
        {
            MY_LOGE("MTKWarp WARP_FEATURE_SET_WORKBUF_ADDR failed! (%d)", -mret);
        }
        else
        {
            ret = MTRUE;
        }
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MVOID GPUWarp::cleanUp()
{
    TRACE_FUNC_ENTER();
    uninitWarp();
    uninitWorkBuffer();
    mInSet.clear();
    mOutSet.clear();
    TRACE_FUNC_EXIT();
}

MVOID GPUWarp::uninitWarp()
{
    TRACE_FUNC_ENTER();
    if( mpGpuWarp )
    {
        mpGpuWarp->WarpReset();
        mpGpuWarp->destroyInstance(mpGpuWarp);
        mpGpuWarp = NULL;
    }
    TRACE_FUNC_EXIT();
}

MVOID GPUWarp::uninitWorkBuffer()
{
    TRACE_FUNC_ENTER();
    mWorkBufSize = 0;
    free(mWorkBuf);
    mWorkBuf = NULL;
    TRACE_FUNC_EXIT();
}

MBOOL GPUWarp::prepareGB(const ImgBuffer &in, const ImgBuffer &out, NB_SPTR &inGB, NB_SPTR &outGB)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    NB_SPTR inGBPtr = NULL;
    NB_SPTR outGBPtr = NULL;
    NativeBuffer *inBuf = NULL;
    NativeBuffer *outBuf = NULL;
    if( in != NULL )
    {
        inGBPtr = in->getGraphicBufferAddr();
    }
    if( out != NULL )
    {
        outGBPtr = out->getGraphicBufferAddr();
    }
    if( inGBPtr != NULL )
    {
        inBuf = getNativeBufferPtr(inGBPtr);
    }
    if( outGBPtr != NULL )
    {
        outBuf = getNativeBufferPtr(outGBPtr);
    }
    if( inBuf && outBuf )
    {
        inGB = inGBPtr;
        outGB = outGBPtr;
        ret = MTRUE;
    }
    else
    {
        MY_LOGE("Missing buffer in(%p) out(%p) inGB(%p) outGB(%p) inGBget(%p) outGBget(%p)", in.get(), out.get(), inGBPtr, outGBPtr, inBuf, outBuf);
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL GPUWarp::registerGB(NB_SPTR in, NB_SPTR out)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;
    MBOOL inReady = MFALSE;
    MBOOL outReady = MFALSE;

    if( in && out )
    {
        inReady = mInSet.count(getNativeBufferPtr(in));
        outReady = mOutSet.count(getNativeBufferPtr(out));

        if( (mpGpuWarp == NULL) ||
            (!inReady && mInSet.size() >= NUM_REGISTER_SLOT) ||
            (!outReady && mOutSet.size() >= NUM_REGISTER_SLOT) )
        {
            MY_LOGW("Restarting GPUWarp: inSet=%zu/%d outSet=%zu/%d gpu=%p", mInSet.size(), NUM_REGISTER_SLOT, mOutSet.size(), NUM_REGISTER_SLOT, mpGpuWarp);
            onUninit();
            onInit();
            inReady = outReady = MFALSE;
            if( !mpGpuWarp )
            {
                MY_LOGE("Restarting GPUWarp failed");
            }
        }

        if( mpGpuWarp )
        {
            inReady = inReady || registerGB(in, mInSet, NB_DIR_IN);
            outReady = outReady || registerGB(out, mOutSet, NB_DIR_OUT);
        }
    }
    if( !inReady || !outReady )
    {
        MY_LOGE("GB not registered: in(%p)=%d out(%p)=%d", in, inReady, out, outReady);
        ret = MFALSE;
    }

    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL GPUWarp::registerGB(NB_SPTR gb, NB_SET &set, NB_DIR dir)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    struct WarpImageExtInfo warpInfo;
    GpuTuning gpuTuning;

    if( gb )
    {
        MRESULT mret;
        configBasicWarpInfo(&warpInfo, &gpuTuning);
        warpInfo.InitBufferCount = 1;
        warpInfo.SrcGraphicBuffer = (dir == NB_DIR_IN) ? (void*)&gb : NULL;
        warpInfo.DstGraphicBuffer = (dir == NB_DIR_IN) ? NULL : (void*)&gb;
        warpInfo.InitBufferType = (dir == NB_DIR_IN) ? 0 : 1;
        if( (mret = mpGpuWarp->WarpFeatureCtrl(WARP_FEATURE_INIT_EGLIMAGE, &warpInfo, NULL)) != S_WARP_OK )
        {
            MY_LOGE("MTKWarp init egl failed (%d)", -mret);
        }
        else
        {
            set.insert(getNativeBufferPtr(gb));
            ret = MTRUE;
        }
    }

    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL GPUWarp::prepareWarp(const ImgBuffer &warpMap, IImageBuffer* &warpBuffer)
{
    TRACE_FUNC_ENTER();
    warpBuffer = NULL;
    if( warpMap == NULL )
    {
        MY_LOGE("Missing warp map");
    }
    else if( (warpBuffer = warpMap->getImageBufferPtr()) == NULL )
    {
        MY_LOGE("Missing warp map(%p) buffer", warpMap.get());
    }
    TRACE_FUNC_EXIT();
    return warpBuffer != NULL;
}

MVOID GPUWarp::configWarpMapInfo(struct WarpImageExtInfo &warpInfo, IImageBuffer *warpBuffer)
{
    TRACE_FUNC_ENTER();
    MSize grid(warpBuffer->getImgSize().w, warpBuffer->getImgSize().h);
    warpInfo.WarpMapSize[0][0] = grid.w;
    warpInfo.WarpMapSize[0][1] = grid.h;
    warpInfo.WarpMapAddr[0][0] = (MUINT32*)(warpBuffer->getBufVA(0));
    warpInfo.WarpMapAddr[0][1] = (MUINT32*)(warpBuffer->getBufVA(1));
    TRACE_FUNC_EXIT();
}

MBOOL GPUWarp::doWarp(struct WarpImageExtInfo &warpInfo)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    MRESULT mret;
    if( (mret = mpGpuWarp->WarpFeatureCtrl(WARP_FEATURE_ADD_IMAGE, &warpInfo, NULL)) != S_WARP_OK )
    {
        MY_LOGE("MTKWarp WARP_FEATURE_ADD_IMAGE failed! (%d)", -mret);
    }
    else if( (mret = mpGpuWarp->WarpMain()) != S_WARP_OK )
    {
        MY_LOGE("MTKWarp WarpMain failed! (%d)", -mret);
    }
    else
    {
        ret = MTRUE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
