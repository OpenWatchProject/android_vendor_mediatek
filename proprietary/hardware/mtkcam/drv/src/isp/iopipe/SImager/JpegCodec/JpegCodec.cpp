/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "MtkCam/JpegCodec"
//
#include <mtkcam/utils/std/ULog.h>
#include "MyUtils.h"
//
CAM_ULOG_DECLARE_MODULE_ID(MOD_SIMAGER);
#define MY_LOGV(fmt, arg...)    CAM_ULOGMV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    CAM_ULOGMD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_ULOGMI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_ULOGMW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_ULOGME("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define FUNCTION_LOG_START      MY_LOGD("+");
#define FUNCTION_LOG_END        MY_LOGD("-");

//
#include <cutils/properties.h>
#include <stdlib.h>
//
#include <mtkcam/utils/std/common.h>
#include <mtkcam/drv/iopipe/SImager/ISImagerDataTypes.h>
#include <mtkcam/drv/iopipe/SImager/IImageTransform.h>
//
#if 1==MTKCAM_USE_LEGACY_JPEGHW
// jpeg encoder
#include <enc/jpeg_hal.h>
#define JPEG_HAL_SUPPORT
#endif
//
// image transform
#include <mtkcam/utils/std/Format.h>
using namespace NSCam::Utils::Format;
//
#include "./inc/JpegCodec.h"

#define MEDIA_PATH "/sdcard/"

#include <assert.h>


#define CHECK_OBJECT(x)  { if (x == NULL) { MY_LOGE("Null %s Object", #x); return MFALSE;}}
/*******************************************************************************
*
********************************************************************************/
using namespace android;
using namespace NSCam;
using namespace NSCam::NSIoPipe::NSSImager;

////////////////////////////////////////////////////////////////////////////////


/*******************************************************************************
*
********************************************************************************/
JpegCodec::
JpegCodec(
)
    : mi4ErrorCode(0)
{
    mu4DumpFlag = ::property_get_int32("vendor.debug.camera.dump", 0);
    mEnableLog = ::property_get_int32("vendor.debug.camera.log.simager", 0);
}



/*******************************************************************************
*
********************************************************************************/
MBOOL
JpegCodec::
encode(
    IImageBuffer const *pSrcBufInfo,
    IImageBuffer const *pDstBufInfo,
    MRect const rROI,
    MUINT32 const u4Transform,
    MUINT32 const u4Quality,
    MUINT32 const u4IsSOI,
    MUINT32 const eCodecType,
    MUINT32 const u4TimeoutMs,
    MBOOL const srcBufNeedFlush//,
//    MUINT32 &u4EncSize
)
{
    MBOOL ret = MTRUE;
    if (checkIfNeedImgTransform(eCodecType, pSrcBufInfo, pDstBufInfo, rROI, u4Transform))
    {
        //
        IImageBuffer *pImageBuff = nullptr;
        int format = pSrcBufInfo->getImgFormat();
        MUINT32 u4AlignedWidth = pDstBufInfo->getImgSize().w;
        MUINT32 u4AlignedHeight = pDstBufInfo->getImgSize().h;

        if( !isSupportedFormat((NSCam::EImageFormat)format) )
        {
            MY_LOGW("use yuy2 for jpeg encode");
            // default format
            format = eImgFmt_YUY2;
        }

        {
            // Jpeg code width/height should be aligned
            MUINT32 w_align(0), h_align(0), w_stride_align(0), h_stride_align(0);
            getAlignment(eCodecType, format, &w_align, &h_align, &w_stride_align, &h_stride_align);

            u4AlignedWidth = (~(w_align-1)) & ((w_align-1) + u4AlignedWidth);
            u4AlignedHeight = (~(h_align-1)) & ((h_align-1) + u4AlignedHeight);
            MY_LOGW("[encode] Ori (width, height) = (%d, %d), Aligned (width, height) = (%d, %d) u4Transform(%d)",
                    pSrcBufInfo->getImgSize().w, pSrcBufInfo->getImgSize().h,
                    u4AlignedWidth, u4AlignedHeight, u4Transform);
        }

        if( !allocYuvMem(&pImageBuff,
                         MSize(u4AlignedWidth, u4AlignedHeight), format) )
        {
            return MFALSE;
        }

        //
        IImageTransform *pImgTransform = IImageTransform::createInstance();

        if (pImgTransform == nullptr) {
            MY_LOGE("IImageTransform createInstance failed!");
            return MFALSE;
        }

        ret = pImageBuff->lockBuf(LOG_TAG, eBUFFER_USAGE_HW_CAMERA_WRITE|eBUFFER_USAGE_SW_MASK);
        if (!ret) {
            MY_LOGE("LockBuffer failes, please check VA usage!");
            assert(1);
        }

        ret = pImageBuff->syncCache(eCACHECTRL_INVALID);

        ret = ret && pImgTransform->execute(pSrcBufInfo, pImageBuff, NULL, rROI, u4Transform, u4TimeoutMs);  //10s timeout

        if (ret && mu4DumpFlag)
        {
            char fileName[256] = {0};
            sprintf(fileName, "/%s/trans_%dx%d.yuv", MEDIA_PATH, u4AlignedWidth, u4AlignedHeight);
            pImageBuff->saveToFile(fileName);
        }

        pImgTransform->destroyInstance();
        // (2). Encode
        MSize srcImgSize = pImageBuff->getImgSize();
        if((pDstBufInfo->getImgSize().w != u4AlignedWidth) || (pDstBufInfo->getImgSize().h != u4AlignedHeight))
        {
            srcImgSize = pSrcBufInfo->getImgSize();
            if((u4Transform == eTransform_ROT_90) ||
               (u4Transform == eTransform_ROT_270)
              )
                srcImgSize = MSize(srcImgSize.h, srcImgSize.w);
            else
                srcImgSize = pSrcBufInfo->getImgSize();
        }
        ret = ret && encode(pImageBuff, pDstBufInfo,
                            u4Quality, u4IsSOI, eCodecType, u4TimeoutMs, srcBufNeedFlush, srcImgSize);
        ret = ret && pImageBuff->unlockBuf(LOG_TAG);

        ret = ret && freeYuvMem(&pImageBuff);
        //
    }
    else
    {
        ret = encode(pSrcBufInfo, pDstBufInfo, u4Quality, u4IsSOI, eCodecType, u4TimeoutMs, srcBufNeedFlush, pSrcBufInfo->getImgSize());
    }
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
JpegCodec::
encode(
    IImageBuffer const *pSrcBufInfo,
    IImageBuffer const *pDstBufInfo,
    MUINT32 const u4Quality,
    MUINT32 const u4IsSOI,
    MUINT32 const eCodecType,
    MUINT32 const u4TimeoutMs __attribute__((unused)),
    MBOOL const srcBufNeedFlush,
    MSize const imgSize
)
{
    FUNCTION_LOG_START;
    //MtkCamUtils::CamProfile profile("encode", "JpegCodec");
    MBOOL ret = MTRUE;
#ifdef JPEG_HAL_SUPPORT
    JpgEncHal* pJpgEncoder = new JpgEncHal();
    CHECK_OBJECT(pJpgEncoder);

    MUINT32 planenum = 0;
    MUINT32 encfmt = JpgEncHal:: kENC_YUY2_Format;
    JpgEncHal::EncLockType lockType = JpgEncHal::JPEG_ENC_LOCK_HW_FIRST;
    switch(eCodecType)
    {
        case JPEGENC_HW_FIRST:
            lockType = JpgEncHal::JPEG_ENC_LOCK_HW_FIRST;
            break;
        case JPEGENC_SW:
            lockType = JpgEncHal::JPEG_ENC_LOCK_SW_ONLY;
            break;
        case JPEGENC_HW_ONLY:
            lockType = JpgEncHal::JPEG_ENC_LOCK_HW_ONLY;
            MY_LOGD("Force using hw to encode jpeg");
            break;
        default:
            break;
    }

    // (1). Lock
    if(!pJpgEncoder->LevelLock(lockType))
    {
        MY_LOGE("can't lock jpeg resource");
        goto EXIT;
    }

    if( !mapToJpgFmt(pSrcBufInfo->getImgFormat(), &encfmt, &planenum) )
    {
        MY_LOGE("map to jpeg fmt failed: 0x%x", pSrcBufInfo->getImgFormat());
        goto EXIT;
    }

    // (2). size, format, addr
    if ( planenum == 1 )
    {
        if(eCodecType == JPEGENC_SW) {
            ret = pJpgEncoder->setEncSize(imgSize.w,
                                      imgSize.h,
                                      static_cast<JpgEncHal::EncFormat>(encfmt),
                                      srcBufNeedFlush)
            && pJpgEncoder->setSrcAddr((void *)pSrcBufInfo->getBufVA(0), (void *)NULL)
            && pJpgEncoder->setSrcBufSize(pSrcBufInfo->getBufStridesInBytes(0),
                                          pSrcBufInfo->getBufSizeInBytes(0),
                                          0);
            MY_LOGD("SW encode, encode stride : %d", (int)pSrcBufInfo->getBufStridesInBytes(0));
        }
        else {
            ret = pJpgEncoder->setEncSize(imgSize.w,
                                      imgSize.h,
                                      static_cast<JpgEncHal::EncFormat>(encfmt),
                                      srcBufNeedFlush)
            && pJpgEncoder->setSrcAddr((void *)pSrcBufInfo->getBufVA(0), (void *)NULL)
            && pJpgEncoder->setSrcBufSize(pJpgEncoder->getSrcBufMinStride(),
                                          pSrcBufInfo->getBufSizeInBytes(0),
                                          0);
            MY_LOGD("HW encode, encode stride : %d", (int)pJpgEncoder->getSrcBufMinStride());
        }
    }
    else if ( planenum == 2 )
    {
        if(eCodecType == JPEGENC_SW)
        {
            ret = pJpgEncoder->setEncSize(imgSize.w,
                    imgSize.h,
                    static_cast<JpgEncHal::EncFormat>(encfmt),
                    srcBufNeedFlush)
                && pJpgEncoder->setSrcAddr((void *)pSrcBufInfo->getBufVA(0),
                        (void *)pSrcBufInfo->getBufVA(1))
                && pJpgEncoder->setSrcBufSize(pSrcBufInfo->getBufStridesInBytes(0),
                        pSrcBufInfo->getBufSizeInBytes(0),
                        pSrcBufInfo->getBufSizeInBytes(1));
            MY_LOGD("SW encode, encode stride : %d", (int)pSrcBufInfo->getBufStridesInBytes(0));
        }
        else
        {
            ret = pJpgEncoder->setEncSize(imgSize.w,
                    imgSize.h,
                    static_cast<JpgEncHal::EncFormat>(encfmt),
                    srcBufNeedFlush)
                && pJpgEncoder->setSrcAddr((void *)pSrcBufInfo->getBufVA(0),
                        (void *)pSrcBufInfo->getBufVA(1))
                && pJpgEncoder->setSrcBufSize(pJpgEncoder->getSrcBufMinStride(),
                        pSrcBufInfo->getBufSizeInBytes(0),
                        pSrcBufInfo->getBufSizeInBytes(1));
            MY_LOGD("HW encode, encode stride : %d", (int)pJpgEncoder->getSrcBufMinStride());
        }
    }
    else if ( planenum == 3 )
    {
        ret = pJpgEncoder->setEncSize(imgSize.w,
                                      imgSize.h,
                                      static_cast<JpgEncHal::EncFormat>(encfmt),
                                      srcBufNeedFlush)
            // workaround for YV12
            && pJpgEncoder->setSrcAddr((void *)pSrcBufInfo->getBufVA(0),
                                       (void *)pSrcBufInfo->getBufVA(2),
                                       (void *)pSrcBufInfo->getBufVA(1))
            && pJpgEncoder->setSrcBufSize(pSrcBufInfo->getBufStridesInBytes(0),
                                          pSrcBufInfo->getBufSizeInBytes(0),
                                          pSrcBufInfo->getBufSizeInBytes(1),
                                          pSrcBufInfo->getBufSizeInBytes(2),
                                          pSrcBufInfo->getBufStridesInBytes(1));
    }
    else
    {
        MY_LOGE("Not support image format:0x%x", (NSCam::EImageFormat)pSrcBufInfo->getImgFormat());
        goto EXIT;
    }

    // (3). set quality
    ret = ret && pJpgEncoder->setQuality(u4Quality)
    // (4). dst addr, size
          && pJpgEncoder->setDstPAddr((void *)pDstBufInfo->getBufPA(0))
          && pJpgEncoder->setDstAddr((void *)pDstBufInfo->getBufVA(0))
          && pJpgEncoder->setDstSize(pDstBufInfo->getBufSizeInBytes(0));

    // (5). ion mode
    {
        MINT32 srcfd_count = pSrcBufInfo->getFDCount();
        MINT32 dstfd_count = pDstBufInfo->getFDCount();
        // set Ion mode
        // jpeg encoder doesn't support 3 plane yet
        if( planenum < 3 && srcfd_count && dstfd_count )
        {
            MINT srcfd0 = -1, srcfd1 = -1;

            //MY_LOGD("ion mode(1)");
            pJpgEncoder->setIonMode(1);

            if ( planenum == 2 )
            {
                if( srcfd_count == 1 )
                    srcfd0 = srcfd1 = pSrcBufInfo->getFD(0);
                else
                {
                    srcfd0 = pSrcBufInfo->getFD(0);
                    srcfd1 = pSrcBufInfo->getFD(1);
                }
            }
            else
            {
                srcfd0 = pSrcBufInfo->getFD(0);
            }

            MY_LOGD("set src fd %d, %d", srcfd0, srcfd1);
            pJpgEncoder->setSrcFD(srcfd0, srcfd1);
            pJpgEncoder->setDstFD(pDstBufInfo->getFD(0));
        }
        else if ( planenum == 3 || (!srcfd_count && !dstfd_count) )
        {
            //MY_LOGD("ion mode(0)");
            pJpgEncoder->setIonMode(0);
        }
        else
        {
            ret = MFALSE;
            MY_LOGE("not support yet src fd count %d, dst fd count %d", srcfd_count, dstfd_count);
        }
    }

    // (6). set SOI
    pJpgEncoder->enableSOI((u4IsSOI > 0) ? 1 : 0);

    // (7).  Start

    MUINT32 u4EncSize;
    if (pJpgEncoder->start(&u4EncSize))
    {
        MY_LOGD("Jpeg encode done, size = %u", u4EncSize);
        ret = (const_cast<IImageBuffer*>(pDstBufInfo))->setBitstreamSize(u4EncSize);
    }
    else
    {
        MY_LOGE("encode start failed");
        ret = MFALSE;
        pJpgEncoder->unlock();
        goto EXIT;
    }

    pJpgEncoder->unlock();
    //profile.print();

EXIT:

    if(!ret)
    {
        //dump settings
        MY_LOGE("lock type 0x%x", eCodecType);
        MUINT32 plane = queryPlaneCount(pSrcBufInfo->getImgFormat());
        MY_LOGE(" fmt 0x%x, wxh %dx%d, va 0x%zx/0x%zx, pa 0x%zx/0x%zx fd %d/%d, size %zu/%zu",
                pSrcBufInfo->getImgFormat(),
                pSrcBufInfo->getImgSize().w       , pSrcBufInfo->getImgSize().h                       ,
                pSrcBufInfo->getBufVA(0)          , plane>1 ? pSrcBufInfo->getBufVA(1) : 0            ,
                pSrcBufInfo->getBufPA(0)          , plane>1 ? pSrcBufInfo->getBufPA(1) : 0            ,
                pSrcBufInfo->getFD(0)             , plane>1 ? pSrcBufInfo->getFD(1) : 0               ,
                pSrcBufInfo->getBufSizeInBytes(0) , plane>1 ? pSrcBufInfo->getBufSizeInBytes(1) : 0);
        MY_LOGE("quality %u, dstV/P 0x%zx/0x%zx, size %zu, soi %d",
                 u4Quality,
                 pDstBufInfo->getBufVA(0), pDstBufInfo->getBufPA(0),
                 pDstBufInfo->getBufSizeInBytes(0),
                 u4IsSOI );
    }
    delete pJpgEncoder;

    FUNCTION_LOG_END;
#else
#warning [jpeg hal not ready yet]
#endif
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
JpegCodec::
isSupportedFormat(EImageFormat const eFmt)
{
    MY_LOGD("Format:0x%x", eFmt);

    switch(eFmt)
    {
        case eImgFmt_YUY2:
        //case eImgFmt_UYVY:
        case eImgFmt_NV12:
        case eImgFmt_NV21:
        case eImgFmt_YV12:
            return MTRUE;
        default:
            break;
    }
    MY_LOGD("not supported fmt 0x%x", eFmt);
    return MFALSE;
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
JpegCodec::
getAlignment(MUINT32 const eCodecType, MINT32 const eFmt, MUINT32* width, MUINT32* height, MUINT32* stride_width, MUINT32* stride_height)
{
    if( eCodecType == JPEGENC_SW )
    {
        *width = 1;
        *height = 1;
        *stride_width = 1;
        *stride_height = 1;
        return MTRUE;
    }
    // HW alignment
    // width/heigh alignment should in 2^k form
    switch( eFmt )
    {
        case eImgFmt_YUY2:
            *width = 16;
            *height = 8;
            *stride_width = 32;
            *stride_height = 8;
            return MTRUE;
#if 0
        case eImgFmt_UYVY: //sw
#endif
        case eImgFmt_NV12:
        case eImgFmt_NV21:
            *width = 16;
            *height = 16;
            *stride_width = 16;
            *stride_height = 16;
            return MTRUE;
        case eImgFmt_YV12: //sw, no limitation
            *width = 1;
            *height = 1;
            *stride_width = 1;
            *stride_height = 1;
            return MTRUE;
        default:
            //jpeg not support
            break;
    }
    return MFALSE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
JpegCodec::
mapToJpgFmt(MINT32 imgfmt, MUINT32* jpgfmt, MUINT32* planenum)
{
    switch(imgfmt)
    {
        case eImgFmt_YUY2:
            *planenum = 1;
#ifdef JPEG_HAL_SUPPORT
            *jpgfmt = JpgEncHal::kENC_YUY2_Format;
#endif
            break;
#if 0
        case eImgFmt_UYVY:
            *planenum = 1;
            *jpgfmt = JpgEncHal::kENC_UYVY_Format;
            break;
#endif
        case eImgFmt_NV12:
            *planenum = 2;
#ifdef JPEG_HAL_SUPPORT
            *jpgfmt = JpgEncHal::kENC_NV12_Format;
#endif
            break;
        case eImgFmt_NV21:
            *planenum = 2;
#ifdef JPEG_HAL_SUPPORT
            *jpgfmt = JpgEncHal::kENC_NV21_Format;
#endif
            break;
        case eImgFmt_YV12:
            *planenum = 3;
            *jpgfmt = JpgEncHal::kENC_YV12_Format;
            break;
        default:
            return MFALSE;
    }
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
JpegCodec::
checkIfNeedImgTransform(
    MUINT32 const eCodecType,
    IImageBuffer const *pSrcBufInfo,
    IImageBuffer const * pDstBufInfo,
    MRect const rROI,
    MUINT32 const u4Transform

)
{
    // resize
    if (pDstBufInfo->getImgSize() != pSrcBufInfo->getImgSize() )
    {
        MY_LOGD("Resize src =(%d,%d), dst=(%d,%d)",
                 pSrcBufInfo->getImgSize().w, pSrcBufInfo->getImgSize().h,
                 pDstBufInfo->getImgSize().w, pDstBufInfo->getImgSize().h);
        return MTRUE;
    }

    // check if src meets alignment
    {
        MUINT32 w_align(0), h_align(0), w_stride_align(0), h_stride_align(0);
        if( getAlignment(eCodecType, pSrcBufInfo->getImgFormat(), &w_align, &h_align, &w_stride_align, &h_stride_align) )
        {
            if (
                (((pSrcBufInfo->getBufStridesInBytes(0)& (w_stride_align-1) ) != 0) ||
                 (((pSrcBufInfo->getBufSizeInBytes(0)/pSrcBufInfo->getBufStridesInBytes(0)) & (h_stride_align-1) ) != 0))
               )
            {
                MY_LOGD("encType 0x%x, src fmt 0x%x, stride width/stride height not aligh to %dx/%dx, src =(%d, %d), stride = (%d, %d)",
                        eCodecType,
                        pSrcBufInfo->getImgFormat(),
                        w_stride_align,
                        h_stride_align,
                        pSrcBufInfo->getImgSize().w,
                        pSrcBufInfo->getImgSize().h,
                        (int)pSrcBufInfo->getBufStridesInBytes(0),
                        (int)(pSrcBufInfo->getBufSizeInBytes(0)/pSrcBufInfo->getBufStridesInBytes(0))
                        );
                return MTRUE;
            }
        }
    }

    // roi
    if (rROI.p.x != 0 || rROI.p.y != 0
        || rROI.s.w != pSrcBufInfo->getImgSize().w
        || rROI.s.h != pSrcBufInfo->getImgSize().h)
    {
        MY_LOGD("Crop , roi = (%d, %d, %d, %d)", rROI.p.x, rROI.p.y,
                                                 rROI.s.w, rROI.s.h);
        return MTRUE;
    }
    // image transform
    if (0 != u4Transform)
    {
        MY_LOGD("u4Transform: %d", u4Transform);
        return MTRUE;
    }
    // JPEG format but source format not support
    if (!isSupportedFormat((NSCam::EImageFormat)pSrcBufInfo->getImgFormat()))
    {
        MY_LOGD("Not JPEG codec support fmt:0x%x", (NSCam::EImageFormat)pSrcBufInfo->getImgFormat());
        return MTRUE;
    }
    //
    MY_LOGV("No need to do image transform");

    return MFALSE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
JpegCodec::
allocYuvMem(IImageBuffer **pBuf, MSize const & align_imgSize, int const format)
{
    MUINT32 bufStridesInBytes[3] = {0};

using namespace NSCam::Utils::Format;
    for (int i = 0; i < (int)queryPlaneCount(format); i++)
    {
        bufStridesInBytes[i] =
            (queryPlaneWidthInPixels(format,i, align_imgSize.w) * queryPlaneBitsPerPixel(format,i)) >> 3;
    }

    MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
    //
    IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
    if (Utils::Format::queryPlaneCount(format) <= 0)
    {
        CAM_LOGE("Error Format %d!\n", format);
        return  MFALSE;
    }
    IImageBufferAllocator::ImgParam imgParam(
        format, align_imgSize, bufStridesInBytes, bufBoundaryInBytes, Utils::Format::queryPlaneCount(format)
    );

    *pBuf = allocator->alloc_ion("allocYuvBuf", imgParam);
    if  ( *pBuf == 0 )
    {
        CAM_LOGE("NULL YUV Buffer\n");
        return  MFALSE;
    }
    //
    MY_LOGD("<YUV> ImgBitsPerPixel:%zu BufSizeInBytes:%zu", (*pBuf)->getImgBitsPerPixel(), (*pBuf)->getBufSizeInBytes(0));
    return  MTRUE;
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
JpegCodec::
freeYuvMem(IImageBuffer **pBuf)
{
    IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
    if (allocator == nullptr) {
        MY_LOGE("IImageBufferAllocator getInstance failed!");
        return MFALSE;
    }
    allocator->free(*pBuf);
    *pBuf = NULL;
    return  MTRUE;
}


////////////////////////////////////////////////////////////////////////////////

