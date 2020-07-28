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

#include "VNRHal.h"

#include "DebugControl.h"
#define PIPE_CLASS_TAG "VNRHal"
#define PIPE_TRACE TRACE_VNR_HAL
#include <featurePipe/core/include/PipeLog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_STREAMING);

#define KEY_VNR_HAL_PRINT_IO "vendor.debug.fpipe.vnrhal.printio"

#if SUPPORT_VNR_DSDN
#define DSDN_CREATE(arg...)             MTKDSDN::createInstance(arg)
#define DSDN_DESTROY(obj, arg...)       obj->destroyInstance(obj, ##arg)
#define DSDN_INIT(obj, arg...)          obj->Init(arg)
#define DSDN_FEATURE_CTRL(obj, arg...)  obj->FeatureCtrl(arg)
#define DSDN_MAIN(obj, arg...)          obj->Main(arg)
#define DSDN_RESET(obj, arg...)         obj->Reset(arg)
#else
#define DSDN_CREATE(arg...)             NULL
#define DSDN_DESTROY(obj)
#define DSDN_INIT(obj, arg...)          E_DSDN_ERR
#define DSDN_FEATURE_CTRL(obj, arg...)  E_DSDN_ERR
#define DSDN_MAIN(obj, arg...)          E_DSDN_ERR
#define DSDN_RESET(obj, arg...)         E_DSDN_ERR
#endif // SUPPORT_VNR_DSDN

extern bool vnr_isDummyDSDN() __attribute__((weak));

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

static inline const char* toErrStr(MRESULT err)
{
    switch(err)
    {
    case S_DSDN_OK:                 return "S_DSDN_OK";
    case E_DSDN_NEED_OVER_WRITE:    return "E_DSDN_NEED_OVER_WRITE";
    case E_DSDN_NULL_OBJECT:        return "E_DSDN_NULL_OBJECT";
    case E_DSDN_WRONG_STATE:        return "E_DSDN_WRONG_STATE";
    case E_DSDN_WRONG_CMD_ID:       return "E_DSDN_WRONG_CMD_ID";
    case E_DSDN_WRONG_CMD_PARAM:    return "E_DSDN_WRONG_CMD_PARAM";
    case E_DSDN_INSUFF_WORK_BUF:    return "E_DSDN_INSUFF_WORK_BUF";
    case E_DSDN_WRONG_IMAGE_FMT:    return "E_DSDN_WRONG_IMAGE_FMT";
    case E_DSDN_ILLEGAL_CONFIG:     return "E_DSDN_ILLEGAL_CONFIG";
    case E_DSDN_RECTIFY_FAIL:       return "E_DSDN_RECTIFY_FAIL";
    case E_DSDN_ILLEGAL_CORE_NUM:   return "E_DSDN_ILLEGAL_CORE_NUM";
    case E_DSDN_NULL_FILE_POINTER:  return "E_DSDN_NULL_FILE_POINTER";
    case E_DSDN_MDP_FAIL:           return "E_DSDN_MDP_FAIL";
    case E_DSDN_INVALID_PARAM:      return "E_DSDN_INVALID_PARAM";
    case E_DSDN_VPU_INIT_ERROR:     return "E_DSDN_VPU_INIT_ERROR";
    case E_DSDN_VPU_GET_ALGO_ERROR: return "E_DSDN_VPU_GET_ALGO_ERROR";
    case E_DSDN_VPU_GET_REQ_ERROR:  return "E_DSDN_VPU_GET_REQ_ERROR";
    case E_DSDN_VPU_ENQ_FAIL:       return "E_DSDN_VPU_ENQ_FAIL";
    case E_DSDN_VPU_SET_PROP_FAIL:  return "E_DSDN_VPU_SET_PROP_FAIL";
    case E_DSDN_ERR:                return "E_DSDN_ERR";
    default:                        return "unknown";
    }
}

MBOOL VNRHal::isSimulate()
{
  return ( vnr_isDummyDSDN != (bool(*)(void))0 );
}

MBOOL VNRHal::isSupport(EImageFormat format)
{
    switch(format)
    {
    case eImgFmt_YV12:  return MFALSE;
    case eImgFmt_NV12:  return MTRUE;
    case eImgFmt_NV21:  return MFALSE;
    case eImgFmt_YUY2:  return MFALSE;
    default:            return MFALSE;
    }
}

VNRHal::VNRHal()
{
    TRACE_FUNC_ENTER();
    mPrintIO = property_get_int32(KEY_VNR_HAL_PRINT_IO, 0);
    TRACE_FUNC_EXIT();
}

VNRHal::~VNRHal()
{
    TRACE_FUNC_ENTER();
    cleanUp();
    TRACE_FUNC_EXIT();
}

MBOOL VNRHal::init(const MSize &maxSize)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;

    if( mDSDN != NULL )
    {
        MY_LOGE("Invalid state, VNRHal already inited");
    }
    else
    {
        mMaxSize = maxSize;
        ret = createDSDN() &&
              //initDSDNBuffer() &&
              initDSDN();
        if( !ret )
        {
            cleanUp();
        }
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL VNRHal::uninit()
{
    TRACE_FUNC_ENTER();
    cleanUp();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL VNRHal::process(MUINT32 fps, const ImgBuffer &inFullImg, const ImgBuffer &inDS1Img, const ImgBuffer &inDS2Img, const ImgBuffer &outFullImg)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    struct DSDNProcInfo procInfo;
    IImageBuffer *inFull = getBuffer(inFullImg);
    IImageBuffer *inDS1 = getBuffer(inDS1Img);
    IImageBuffer *inDS2 = getBuffer(inDS2Img);
    IImageBuffer *outFull = getBuffer(outFullImg);

    if( checkState() && checkInput(inFull, inDS1, inDS2, outFull) )
    {
        prepareProcInfo(procInfo, fps, inFull, inDS1, inDS2, outFull);
        printProcInfo(procInfo);
        ret = doDSDN(procInfo);
    }

    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL VNRHal::checkState() const
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;
    if( mDSDN == NULL )
    {
        MY_LOGE("Invalid DSDN = NULL");
        ret = MFALSE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL VNRHal::checkInput(const IImageBuffer *full, const IImageBuffer *ds1, const IImageBuffer *ds2, const IImageBuffer *out) const
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;
    if( !full || !ds1 || !ds2 || !out )
    {
        MY_LOGE("Invalid VNR input: full=%p ds1=%p ds2=%p out=%p", full, ds1, ds2, out);
        ret = MFALSE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL VNRHal::cleanUp()
{
    TRACE_FUNC_ENTER();
    uninitDSDN();
    uninitDSDNBuffer();
    mMaxSize = MSize(0,0);
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL VNRHal::createDSDN()
{
    TRACE_FUNC_ENTER();
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "MTKDSDN create");
    if( (mDSDN = DSDN_CREATE(DRV_DSDN_OBJ_SW)) == NULL )
    {
        MY_LOGE("MTKDSDN::createInstance failed");
    }
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    TRACE_FUNC_EXIT();
    return mDSDN != NULL;
}

DSDNTuningParam gDSDNTuning[] =
{
  { "vendor.DSDN.debugLevel",   0},
  { "vendor.DSDN.lgoLevel",     0},
};

MBOOL VNRHal::initDSDN()
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;
    MRESULT mret;
    DSDNTuningInfo tuningInfo;
    DSDNInitInfo initInfo;

    tuningInfo.NumOfParam = sizeof(gDSDNTuning)/sizeof(gDSDNTuning[0]);
    tuningInfo.params = gDSDNTuning;

    initInfo.dsdnMode = 0;
    initInfo.maxWidth = mMaxSize.w;
    initInfo.maxHeight = mMaxSize.h;
    initInfo.workingBuffAddr = NULL;
    initInfo.workingBuffSize = 0;
    initInfo.pTuningInfo = &tuningInfo;

    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "MTKDSDN init");
    if( !mDSDN )
    {
        MY_LOGE("Invalid VNRHal object state");
        ret = MFALSE;
    }
    else if( (mret = DSDN_INIT(mDSDN, &initInfo, NULL)) != S_DSDN_OK )
    {
        MY_LOGE("MTKDSDN init failed(%08X:%s)", mret, toErrStr(mret));
        ret = MFALSE;
    }
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL VNRHal::initDSDNBuffer()
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    MRESULT mret;
    if( !mDSDN )
    {
        MY_LOGE("Invalid VNRHal object state");
    }
    else if( (mret = DSDN_FEATURE_CTRL(mDSDN, DSDN_FEATURE_GET_WORKBUF_SIZE, NULL, &mWorkBufSize)) != S_DSDN_OK )
    {
        MY_LOGE("MTKDSDN DSDN_FEATURE_GET_WORKBUF_ADDR failed(%08x:%s)",
                mret, toErrStr(mret));
    }
    else if( (mWorkBuf = (MUINT8*)malloc(mWorkBufSize*sizeof(unsigned char))) == NULL )
    {
        MY_LOGE("OOM: DSDN working buffer allocation failed!");
    }
    else
    {
        DSDNInitInfo initInfo;
        initInfo.workingBuffSize = mWorkBufSize;
        initInfo.workingBuffAddr = (MUINT8*)mWorkBuf;
        if( (mret = DSDN_FEATURE_CTRL(mDSDN, DSDN_FEATURE_SET_WORKBUF_ADDR, &initInfo, NULL)) != S_DSDN_OK )
        {
            MY_LOGE("MTKDSDN DSDN_FEATURE_SET_WORKBUF_ADDR failed(%08x:%s)",
                    mret, toErrStr(mret));
        }
        else
        {
            ret = MTRUE;
        }
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL VNRHal::uninitDSDN()
{
    TRACE_FUNC_ENTER();
    if( mDSDN )
    {
        P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "MTKDSDN destroy");
        DSDN_RESET(mDSDN);
        DSDN_DESTROY(mDSDN);
        mDSDN = NULL;
        P2_CAM_TRACE_END(TRACE_ADVANCED);
    }
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL VNRHal::uninitDSDNBuffer()
{
    TRACE_FUNC_ENTER();
    mWorkBufSize = 0;
    free(mWorkBuf);
    mWorkBuf = NULL;
    TRACE_FUNC_EXIT();
    return MTRUE;
}

IImageBuffer* VNRHal::getBuffer(const ImgBuffer &img) const
{
    return img != NULL ? img->getImageBufferPtr() : NULL;
}

MBOOL VNRHal::prepareProcInfo(struct DSDNProcInfo &info, MUINT32 fps, const IImageBuffer *full, const IImageBuffer *ds1, const IImageBuffer *ds2, const IImageBuffer *out) const
{
    TRACE_FUNC_ENTER();
    info.fps = fps;
    info.numOfBuffer = 0;
    addBufferInfo(info, DSDN_BUFFER_TYPE_IN1, full);
    addBufferInfo(info, DSDN_BUFFER_TYPE_IN2, ds1);
    addBufferInfo(info, DSDN_BUFFER_TYPE_IN3, ds2);
    addBufferInfo(info, DSDN_BUFFER_TYPE_OUT1, out);
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MVOID VNRHal::printProcInfo(const struct DSDNProcInfo &info) const
{
    if( TRACE_VNR_HAL || mPrintIO )
    {
        MY_LOGD("fps=%d buf=%d", info.fps, info.numOfBuffer);
        for( MUINT32 i = 0; i < info.numOfBuffer; ++i )
        {
            const DSDNBufferInfo &buf = info.bufferInfo[i];
            MY_LOGD("buf[%d]: type=%d fmt=%d(%s) size=(%dx%d) plane=%d",
                    i, buf.type, buf.format, toString(buf.format), buf.width, buf.height, buf.planeCount);
            for( MUINT32 j = 0; j < buf.planeCount; ++j )
            {
                const DSDNPlaneInfo &plane = buf.planes[j];
                MY_LOGD("plane[%d]: fd=%d offset=%d stride=%d length=%d va=%p",
                        j, plane.fd, plane.offset, plane.stride, plane.length, plane.va);
            }
        }
    }
}

MBOOL VNRHal::addBufferInfo(struct DSDNProcInfo &info, DSDN_BUFFER_TYPE_ENUM type, const IImageBuffer *img) const
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    if( img )
    {
        size_t heapCount = img->getFDCount();
        size_t planeCount = img->getPlaneCount();
        if( planeCount > 3 ||
            (heapCount != 1 && heapCount != planeCount) )
        {
            MY_LOGE("invalid heap/plane count = %zu/%zu",
                     heapCount, planeCount);
        }
        else
        {
            MUINT32 idx = info.numOfBuffer++;
            MINT32 baseFD = img->getFD(0);
            MUINT8 *baseVA = (MUINT8*)img->getBufVA(0) - img->getBufOffsetInBytes(0);

            DSDNBufferInfo *bufferInfo = &info.bufferInfo[idx];

            MSize size = img->getImgSize();
            bufferInfo->width = size.w;
            bufferInfo->height = size.h;
            bufferInfo->type = type;
            bufferInfo->format = toDSDNFormat((EImageFormat)img->getImgFormat());
            bufferInfo->planeCount = planeCount;
            for( unsigned i = 0; i < planeCount; ++i )
            {
                DSDNPlaneInfo *planeInfo = &bufferInfo->planes[i];

                planeInfo->stride = img->getBufStridesInBytes(i);
                planeInfo->length = img->getBufSizeInBytes(i);
                planeInfo->va = (MUINT8*)img->getBufVA(i);

                if( heapCount != 1 )
                {
                    planeInfo->fd = img->getFD(i);
                    planeInfo->offset = img->getBufOffsetInBytes(i);
                }
                if( heapCount == 1 || planeInfo->fd == baseFD )
                {
                    planeInfo->fd = baseFD;
                    planeInfo->offset = planeInfo->va - baseVA;
                }
            }
            ret = MTRUE;
        }
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL VNRHal::doDSDN(const struct DSDNProcInfo &info)
{
    (void)info;
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    MRESULT mret;
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "MTKDSDN run");
    if( (mret = DSDN_FEATURE_CTRL(mDSDN, DSDN_FEATURE_SET_PROC_INFO, (void*)&info, NULL)) != S_DSDN_OK )
    {
        MY_LOGE("MTKDSDN DSDN_FEATURE_SET_PROC_INFO failed(%08x:%s)",
                mret, toErrStr(mret));
    }
    else if( (mret = DSDN_MAIN(mDSDN)) != S_DSDN_OK )
    {
        MY_LOGE("MTKDSDN Main failed(%08x:%s)", mret, toErrStr(mret));
    }
    else
    {
        ret = MTRUE;
    }
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    TRACE_FUNC_EXIT();
    return ret;
}

DSDN_BUFFER_FMT_ENUM VNRHal::toDSDNFormat(EImageFormat fmt) const
{
    DSDN_BUFFER_FMT_ENUM dsdnFmt = DSDNFormatData;
    switch(fmt)
    {
    case eImgFmt_YV12:  dsdnFmt = DSDNFormatImageYV12;  break;
    case eImgFmt_NV12:  dsdnFmt = DSDNFormatImageNV12;  break;
    case eImgFmt_NV21:  dsdnFmt = DSDNFormatImageNV21;  break;
    case eImgFmt_YUY2:  dsdnFmt = DSDNFormatImageYUY2;  break;
    default:            dsdnFmt = DSDNFormatData;       break;
    }
    return dsdnFmt;
}

const char* VNRHal::toString(DSDN_BUFFER_FMT_ENUM fmt) const
{
    const char *str = "unknown";
    switch(fmt)
    {
    case DSDNFormatImageY8:   str = "y8";       break;
    case DSDNFormatImageYV12: str = "yv12";     break;
    case DSDNFormatImageNV12: str = "nv12";     break;
    case DSDNFormatImageNV21: str = "nv21";     break;
    case DSDNFormatImageYUY2: str = "yuy2";     break;
    default:                  str = "unknown";  break;
    }
    return str;
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
