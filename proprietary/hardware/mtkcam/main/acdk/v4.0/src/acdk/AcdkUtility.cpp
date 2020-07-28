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

///////////////////////////////////////////////////////////////////////////////
// No Warranty
// Except as may be otherwise agreed to in writing, no warranties of any
// kind, whether express or implied, are given by MTK with respect to any MTK
// Deliverables or any use thereof, and MTK Deliverables are provided on an
// "AS IS" basis.  MTK hereby expressly disclaims all such warranties,
// including any implied warranties of merchantability, non-infringement and
// fitness for a particular purpose and any warranties arising out of course
// of performance, course of dealing or usage of trade.  Parties further
// acknowledge that Company may, either presently and/or in the future,
// instruct MTK to assist it in the development and the implementation, in
// accordance with Company's designs, of certain softwares relating to
// Company's product(s) (the "Services").  Except as may be otherwise agreed
// to in writing, no warranties of any kind, whether express or implied, are
// given by MTK with respect to the Services provided, and the Services are
// provided on an "AS IS" basis.  Company further acknowledges that the
// Services may contain errors, that testing is important and Company is
// solely responsible for fully testing the Services and/or derivatives
// thereof before they are used, sublicensed or distributed.  Should there be
// any third party action brought against MTK, arising out of or relating to
// the Services, Company agree to fully indemnify and hold MTK harmless.
// If the parties mutually agree to enter into or continue a business
// relationship or other arrangement, the terms and conditions set forth
// hereunder shall remain effective and, unless explicitly stated otherwise,
// shall prevail in the event of a conflict in the terms in any agreements
// entered into between the parties.
////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008, MediaTek Inc.
// All rights reserved.
//
// Unauthorized use, practice, perform, copy, distribution, reproduction,
// or disclosure of this information in whole or in part is prohibited.
////////////////////////////////////////////////////////////////////////////////

//! \file  AcdkUtility.cpp

#define LOG_TAG "AcdkUtility"

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#if defined (USING_MTK_ION)
#include <sys/mman.h>
#include <ion.h>
#include <ion/ion.h>
#include <linux/ion_drv.h>
#include <linux/mtk_ion.h>
#if defined(MTK_M4U_SUPPORT)
#include "m4u_lib.h"
#elif defined(MTK_ION_SUPPORT)
#include "mt_iommu_port.h"
#endif
#endif

#include <mtkcam/def/common.h>
//using namespace NSCam;
#include <mtkcam/main/acdk/AcdkTypes.h>

#include <mtkcam/drv/iopipe/SImager/ISImager.h> //To do: temp removed for MT6595_DEV

using namespace NSCam;
using namespace NSCam::NSIoPipe::NSSImager;

#include "AcdkLog.h"
#include "AcdkErrCode.h"

#include "AcdkUtility.h"
using namespace NSACDK;

#include <utils/RefBase.h>
#include <system/camera.h>
#include <hardware/camera.h>
#include <dlfcn.h>
using namespace android;

#include <mtkcam/utils/imgbuf/ImageBufferHeap.h>
#include <mtkcam/utils/std/Format.h>
using namespace NSCam::Utils::Format;

#include <cutils/properties.h>
#define GET_PROP(prop, init, val)\
{\
    char value[PROPERTY_VALUE_MAX] = {'\0'};\
    property_get(prop, value, (init));\
    (val) = atoi(value);\
}
#define UNUSED(x) (void)(x)

/*******************************************************************************
*
********************************************************************************/
#define PIXEL_BYTE_FP 2
#define ROUND_TO_2X(x) ((x) & (~0x1))

#include <imem_buffer.h>
#ifndef CAM_M4U_PORT_DMA_MODULEID
#define CAM_M4U_PORT_DMA_MODULEID       M4U_PORT_CAM_IMGO
#endif
/*******************************************************************************
*
********************************************************************************/
static const MUINT32 g_capRange  = 8;
static MUINT32 g_capWidthRange[g_capRange]  = {320,640,1024,1280,1600,2048,2560,3264};
static MUINT32 g_capHeightRange[g_capRange] = {240,480,768,960,1200,1536,1920,2448};

static const MUINT32 g_prvRange = 5;
static MUINT32 g_prvWidthRange[g_prvRange]  = {320,640,800,960,1280};
static MUINT32 g_prvHeightRange[g_prvRange] = {240,480,600,540, 720};
typedef struct
{
    IImageBuffer* pImgBuf;
    AMEM_BUF_INFO memBuf;
} ImageBufferMap;
Vector<ImageBufferMap> mvImgBufMap;
MBOOL mDebugEnable;

/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkUtility::ImageallocBuffer(IImageBuffer*& ppBuf, MUINT32 w, MUINT32 h, MUINT32 fmt, AMEM_BUF_INFO Imem)
{
    IImageBuffer* pBuf = NULL;

    ACDK_LOGD_IF(mDebugEnable, "ImageallocBuffer addr(%p), width(%d), height(%d), format(0x%x)", pBuf, w, h, fmt);

    /* To avoid non-continuous multi-plane memory, allocate ION memory and map it to ImageBuffer */
    ImageBufferMap bufMap;

    MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
    MUINT32 bufStridesInBytes[3] = {0};

    MUINT32 qvStride[3] = {0, 0, 0};
    queryImageStride(fmt,w,0,&qvStride[0]);
    queryImageStride(fmt,w,1,&qvStride[1]);
    queryImageStride(fmt,w,2,&qvStride[2]);
    MUINT32 plane = queryPlaneCount(fmt);
    for (MUINT32 i = 0; i < plane; i++)
    {
        queryImageStride(fmt,w,i,&qvStride[i]);
        bufStridesInBytes[i] = ( qvStride[i] * queryPlaneBitsPerPixel(fmt,i) ) >> 3;
    }


    //Initializtion Buffer Map
    bufMap.memBuf.size = Imem.size;
    bufMap.memBuf.memID    = Imem.memID;
    bufMap.memBuf.virtAddr = Imem.virtAddr;
    bufMap.memBuf.useNoncache = 1;
    bufMap.memBuf.bufSecu = 0;
    bufMap.memBuf.bufCohe = 0;
    //
    IImageBufferAllocator::ImgParam imgParam(fmt
                                            , MSize(w,h)
                                            , bufStridesInBytes
                                            , bufBoundaryInBytes
                                            , plane
                                            );

    PortBufInfo_v1 portBufInfo = PortBufInfo_v1(
                                    bufMap.memBuf.memID,
                                    bufMap.memBuf.virtAddr,
                                    bufMap.memBuf.useNoncache,
                                    bufMap.memBuf.bufSecu,
                                    bufMap.memBuf.bufCohe);

    sp<ImageBufferHeap> pHeap = ImageBufferHeap::create(
                                                    LOG_TAG,
                                                    imgParam,
                                                    portBufInfo);
    if(pHeap == 0) {
        ACDK_LOGD("pHeap is NULL");
        return MFALSE;
    }

    //
    pBuf = pHeap->createImageBuffer();
    if (!pBuf) {
        ACDK_LOGD("Null allocated failed\n");
        return  0;
    }
    pBuf->incStrong(pBuf);

    bufMap.pImgBuf = pBuf;
    mvImgBufMap.push_back(bufMap);

    if (!pBuf->lockBuf( LOG_TAG, eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_OFTEN ) )
    {
        ACDK_LOGD("lock Buffer failed\n");
        return  0;
    }

    ppBuf = pBuf;

    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AcdkUtility::deallocBuffer(IImageBuffer* &pBuf)
{
    if(!pBuf) {
        ACDK_LOGD_IF(mDebugEnable, "free a null buffer");
        return MFALSE;
    }

    pBuf->unlockBuf(LOG_TAG);

    pBuf->decStrong(pBuf);

    pBuf = NULL;

    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
AcdkUtility *AcdkUtility::createInstance()
{
    ACDK_LOGD("createInstance");
    GET_PROP("vendor.debug.acdk.util.log", "0", mDebugEnable);
    return new AcdkUtility;
}

/*******************************************************************************
*
********************************************************************************/
MVOID AcdkUtility::destroyInstance()
{
    delete this;
}

/*******************************************************************************
*
********************************************************************************/
MVOID AcdkUtility::queryPrvSize(MUINT32 &oriW, MUINT32 &oriH)
{
    ACDK_LOGD_IF(mDebugEnable, "oriW(%u),oriH(%u)",oriW,oriH);

    MUINT32 tempW = oriW, tempH = oriH;
    MBOOL isFound = MFALSE;

    for(MUINT32 i = 0; i < (g_prvRange - 1); ++i)
    {
        if((tempW == g_prvWidthRange[i] && tempH == g_prvHeightRange[i]) || (tempW >= g_prvWidthRange[i] && tempW < g_prvWidthRange[i + 1]))
        {
            oriW = g_prvWidthRange[i];
            oriH = g_prvHeightRange[i];
            isFound = MTRUE;
            break;
        }
    }

    if(isFound == MFALSE)
    {
        oriW = g_prvWidthRange[g_prvRange - 1];
        oriH = g_prvHeightRange[g_prvRange - 1];
    }

    ACDK_LOGD_IF(mDebugEnable, "before ROUND_TO_2X - oriW(%u),oriH(%u)",oriW,oriH);

    oriW = ROUND_TO_2X(oriW);
    oriH = ROUND_TO_2X(oriH);

    ACDK_LOGD_IF(mDebugEnable, "X - oriW(%u),oriH(%u)",oriW,oriH);
}

/*******************************************************************************
*
********************************************************************************/
MVOID AcdkUtility::queryCapSize(MUINT32 &oriW, MUINT32 &oriH)
{
    ACDK_LOGD_IF(mDebugEnable, "oriW(%u),oriH(%u)",oriW,oriH);

    MUINT32 tempW = oriW, tempH = oriH;
    MBOOL isFound = MFALSE;

    for(MUINT32 i = 0; i < (g_capRange - 1); ++i)
    {
        if(tempW == g_capWidthRange[i] && tempH == g_capHeightRange[i])
        {
            oriW = g_capWidthRange[i];
            oriH = g_capHeightRange[i];
            isFound = MTRUE;
            break;
        }
        else if(tempW >= g_capWidthRange[i] && tempW < g_capWidthRange[i + 1])
        {
            oriW = g_capWidthRange[i + 1];
            oriH = g_capHeightRange[i + 1];
            isFound = MTRUE;
            break;
        }
    }

    if(isFound == MFALSE)
    {
        oriW = g_capWidthRange[g_capRange - 1];
        oriH = g_capHeightRange[g_capRange - 1];
    }

    ACDK_LOGD_IF(mDebugEnable, "before align 16x - oriW(%u),oriH(%u)",oriW,oriH);

    oriW = oriW & ~(0xf);
    oriH = oriH & ~(0xf);

    ACDK_LOGD_IF(mDebugEnable, "X - oriW(%u),oriH(%u)",oriW,oriH);
}

/*******************************************************************************
*
*******************************************************************************/
MINT32 AcdkUtility::queryRAWImgFormatInfo(MUINT32 const imgFmt, MUINT32 u4ImgWidth, MUINT32 &u4Stride, MUINT32 &pixel_byte)
{
    MUINT32 stride = u4Stride;

    ACDK_LOGD_IF(mDebugEnable, "imgFmt(0x%x),u4ImgWidth(%u),u4Stride(%u),pixel_byte(%u)",imgFmt,u4ImgWidth,u4Stride,pixel_byte);

    if(u4ImgWidth % 2 || u4Stride % 2)
    {
        ACDK_LOGE("width and stride should be even number");
    }

    switch(imgFmt)
    {
        case eImgFmt_BAYER8 :   //= 0x0001,   //Bayer format, 8-bit
            pixel_byte = 1 << PIXEL_BYTE_FP;
            break;
        case eImgFmt_BAYER10 :  //= 0x0002,   //Bayer format, 10-bit
            pixel_byte = (5 << PIXEL_BYTE_FP) >> 2; // 4 pixels-> 5 bytes, 1.25

            if(stride % 8)
            {
                stride = stride + 8 - (stride % 8);
            }

            if(u4Stride < stride)
            {
                ACDK_LOGE("RAW10 STRIDE SHOULD BE MULTIPLE OF 8(%u)->(%u)",u4Stride,stride);
            }
            break;
        case eImgFmt_BAYER12 :   //= 0x0004,   //Bayer format, 12-bit
            pixel_byte = (3 << PIXEL_BYTE_FP) >> 1; // 2 pixels-> 3 bytes, 1.5

            if(stride % 4)
            {
                stride = stride + 4 - (stride % 4);
            }

            if(u4Stride < stride)
            {
                ACDK_LOGD_IF(mDebugEnable, "RAW12 STRIDE SHOULD BE MULTIPLE OF 4(%u)->(%u)",u4Stride,stride);
            }
            break;
        default:
            ACDK_LOGE("NOT SUPPORT imgFmt(%u)",imgFmt);
            return ACDK_RETURN_INVALID_PARA;
    }

    u4Stride = stride;
    ACDK_LOGD_IF(mDebugEnable, "X:imgFmt(0x%x),u4ImgWidth(%u),u4Stride(%u),pixel_byte(%u)",imgFmt,u4ImgWidth,u4Stride,pixel_byte);
    return ACDK_RETURN_NO_ERROR;
}

/*******************************************************************************
*
*******************************************************************************/
MINT32 AcdkUtility::queryRAWImgFullGInfo(MUINT32 const imgFmt, MUINT32 u4ImgWidth, MUINT32 &u4Stride, MUINT32 &pixel_byte)
{
    MUINT32 stride = u4Stride;

    ACDK_LOGD_IF(mDebugEnable, "imgFmt(0x%x),u4ImgWidth(%u),u4Stride(%u),pixel_byte(%u)",imgFmt,u4ImgWidth,u4Stride,pixel_byte);

    if(u4ImgWidth % 2 || u4Stride % 2)
    {
        ACDK_LOGE("width and stride should be even number");
    }
    //
    /*
        - mod(w*1.5*1.00,8) = 0; //8bit, w=16n
        - mod(w*1.5*1.25,8) = 0; //10bit w=64n
        - mod(w*1.5*1.50,8) = 0; //12bit w=32n
      */
    switch(imgFmt)
    {
        case eImgFmt_FG_BAYER8 :   //= 0x0001,   //Bayer format, 8-bit
            stride = (u4ImgWidth>>4)<<4;;
            break;
        case eImgFmt_FG_BAYER10 :  //= 0x0002,   //Bayer format, 10-bit
            stride = (u4ImgWidth>>6)<<6;
            break;
        case eImgFmt_FG_BAYER12 :   //= 0x0004,   //Bayer format, 12-bit
            stride = (u4ImgWidth>>5)<<5;
            break;
        default:
            ACDK_LOGE("NOT SUPPORT imgFmt(%u)",imgFmt);
            return ACDK_RETURN_INVALID_PARA;
    }

    u4Stride = stride;
    ACDK_LOGD_IF(mDebugEnable, "X:imgFmt(0x%x),u4ImgWidth(%u),u4Stride(%u),pixel_byte(%u)",imgFmt,u4ImgWidth,u4Stride,pixel_byte);
    return ACDK_RETURN_NO_ERROR;
}


/******************************************************************************
*
*******************************************************************************/
MINT32 AcdkUtility::queryImageSize(MUINT32 imgFormat, MUINT32 imgW, MUINT32 imgH, MUINT32 &imgSize)
{
    ACDK_LOGD_IF(mDebugEnable, "imgFormat(0x%x)",imgFormat);

    MINT32 err = ACDK_RETURN_NO_ERROR;

    switch(imgFormat)
    {
        // YUV 420 format
        case eImgFmt_YV12:
        case eImgFmt_NV21:
        case eImgFmt_NV21_BLK:
        case eImgFmt_NV12:
        case eImgFmt_NV12_BLK:
        case eImgFmt_I420:
            imgSize = imgW * imgH * 3 / 2;
            break;
        // YUV 422 format , RGB565
        case eImgFmt_YUY2:
        case eImgFmt_UYVY:
        case eImgFmt_YVYU:
        case eImgFmt_VYUY:
        case eImgFmt_YV16:
        case eImgFmt_NV16:
        case eImgFmt_NV61:
        case eImgFmt_RGB565:
            imgSize = imgW * imgH * 2;
            break;
        case eImgFmt_RGB888:
            imgSize = imgW * imgH * 3;
            break;
        case eImgFmt_ARGB888:
            imgSize = imgW * imgH * 3;
            break;
        case eImgFmt_JPEG:
            imgSize = imgW * imgH / 4;    //? assume the JPEG ratio is 1/4
            break;
        case eImgFmt_Y800:
            imgSize = imgW * imgH;
            break;
        default:
            imgSize = 0;
            ACDK_LOGE("cannot calculate image size");
            err = ACDK_RETURN_INVALID_PARA;
            break;
    }

    ACDK_LOGD_IF(mDebugEnable, "-");
    return err;
}

/******************************************************************************
*
*******************************************************************************/
MINT32 AcdkUtility::queryImageStride(MUINT32 imgFormat, MUINT32 imgW, MUINT32 planeIndex, MUINT32 *imgStride)
{
    ACDK_LOGD_IF(mDebugEnable, "imgFormat(0x%x)",imgFormat);

    switch(imgFormat)
    {
        // YUV 420 format
        case eImgFmt_NV21:
        case eImgFmt_NV21_BLK:
        case eImgFmt_NV12:
        case eImgFmt_NV12_BLK:
            *imgStride = (planeIndex == 2) ? (0) : (imgW);
            break;
        case eImgFmt_YV12:
        case eImgFmt_I420:
            *imgStride = (planeIndex == 0) ? (imgW) : (imgW / 2);
            break;
        // YUV 422 format , RGB565
        case eImgFmt_YUY2:
        case eImgFmt_UYVY:
        case eImgFmt_YVYU:
        case eImgFmt_VYUY:
        case eImgFmt_RGB565:
            *imgStride = (planeIndex == 0) ? (imgW) : 0;
            break;
        case eImgFmt_YV16:
        case eImgFmt_NV16:
        case eImgFmt_NV61:
            *imgStride = (planeIndex == 0) ? (imgW) : (imgW / 2);
            break;
        case eImgFmt_RGB888:
            *imgStride = imgW;
            break;
        case eImgFmt_ARGB888:
            *imgStride = imgW;
            break;
        case eImgFmt_JPEG:
            *imgStride = imgW ;
            break;
        case eImgFmt_Y800:
            *imgStride = (planeIndex == 0) ? (imgW) : (0);
            break;
        default:
            *imgStride = imgW;
            break;
    }

    ACDK_LOGD_IF(mDebugEnable, "-");
    return ACDK_RETURN_NO_ERROR;
}



/******************************************************************************
*
*******************************************************************************/
MINT32 AcdkUtility::imageProcess( MUINT32 imgInFormat,
                                       MUINT32 imgOutFormat,
                                       MUINT32 srcImgW,
                                       MUINT32 srcImgH,
                                       MUINT32 orientaion,
                                       MUINT32 flip,
                                       AMEM_BUF_INFO srcImem,
                                       AMEM_BUF_INFO dstImem,
                                       MUINT32 dstImgW,
                                       MUINT32 dstImgH)
{
    IImageBuffer* mpImageBuffer[2];
    MUINT32 u4Transform;
    UNUSED(dstImgW);
    UNUSED(dstImgH);
    ACDK_LOGD_IF(mDebugEnable, "Informat(0x%x), Outformat(0x%x)",imgInFormat,imgOutFormat);
    ACDK_LOGD_IF(mDebugEnable, "srcImgW(%u),srcImgH(%u)",srcImgW,srcImgH);
    ACDK_LOGD_IF(mDebugEnable, "orientaion(%u),flip(%u)",orientaion,flip);
    ACDK_LOGD_IF(mDebugEnable, "srcImem : size(%u),vir(0x%p),phy(0x%p),mimID(%d)",srcImem.size,(void*)srcImem.virtAddr,(void*)srcImem.phyAddr,srcImem.memID);
    ACDK_LOGD_IF(mDebugEnable, "dstImem : size(%u),vir(0x%p),phy(0x%p),mimID(%d)",dstImem.size,(void*)dstImem.virtAddr,(void*)dstImem.phyAddr,dstImem.memID);

    ImageallocBuffer(mpImageBuffer[0],srcImgW,srcImgH,imgInFormat,srcImem);
    //
    if(orientaion == 90 || orientaion == 270)
    {
        ImageallocBuffer(mpImageBuffer[1],srcImgH,srcImgW,imgOutFormat,dstImem);
    }
    else
    {
        ImageallocBuffer(mpImageBuffer[1],srcImgW,srcImgH,imgOutFormat,dstImem);
    }

    //
    switch(orientaion)
    {
        case 0:
            if( flip == true )
            {
                u4Transform = eTransform_FLIP_H;
            }
            else
            {
                u4Transform = 0;
            }
            break;
        case 90:
            if( flip == true )
            {
                u4Transform = eTransform_ROT_90 | eTransform_FLIP_V;
            }
            else
            {
                u4Transform = eTransform_ROT_90;
            }
            break;
        case 180:
            if( flip == true )
            {
                u4Transform = eTransform_FLIP_V;
            }
            else
            {
                u4Transform = eTransform_ROT_180;
            }
            break;
        case 270:
            if( flip == true )
            {
                u4Transform = eTransform_ROT_90 | eTransform_FLIP_H;
            }
            else
            {
                u4Transform = eTransform_ROT_270;
            }
            break;
        default:
            u4Transform = 0;
            break;
    }

    //create SImage object
    ISImager *sImager;
    #ifndef USING_MTK_LDVT
    sImager = ISImager::createInstance(mpImageBuffer[0]);
    #endif
    //====== SImage Process ======

    if( MTRUE != sImager->setTargetImgBuffer(mpImageBuffer[1]) )
        ACDK_LOGD_IF(mDebugEnable, "setTargetImgBuffer return fail");

    if( MTRUE != sImager->setTransform(u4Transform) )
        ACDK_LOGD_IF(mDebugEnable, "setTransform return fail");

    if( MTRUE != sImager->execute() )
        ACDK_LOGD_IF(mDebugEnable, "execute return fail");

    //====== Destory Instance ======
    sImager->destroyInstance();

    deallocBuffer(mpImageBuffer[0]);
    deallocBuffer(mpImageBuffer[1]);

    ACDK_LOGD_IF(mDebugEnable, "-");
    return ACDK_RETURN_NO_ERROR;
}


/******************************************************************************
*
*******************************************************************************/
MINT32 AcdkUtility::rawImgUnpack(AMEM_BUF_INFO srcImem,
                                       AMEM_BUF_INFO dstImem,
                                       MUINT32 a_imgW,
                                       MUINT32 a_imgH,
                                       MUINT32 a_bitDepth,
                                       MUINT32 a_Stride)
{

    ACDK_LOGD_IF(mDebugEnable, "srcImem : VA(0x%p),PA(0x%p),ID(%d),SZ(%u)",(void*)srcImem.virtAddr,
                                                           (void*)srcImem.phyAddr,
                                                           srcImem.memID,
                                                           srcImem.size);

    ACDK_LOGD_IF(mDebugEnable, "dstImem : VA(0x%p),PA(0x%p),ID(%d),SZ(%u)",(void*)dstImem.virtAddr,
                                                           (void*)dstImem.phyAddr,
                                                           dstImem.memID,
                                                           dstImem.size);

    ACDK_LOGD_IF(mDebugEnable, "imgW(%u),imgH(%u),bitDepth(%u),stride(%u)",a_imgW,
                                                           a_imgH,
                                                           a_bitDepth,
                                                           a_Stride);

    //====== Unpack ======

    MUINT8 *pSrcBuf = (MUINT8 *)srcImem.virtAddr;
    MUINT16 *pDstBuf = (MUINT16 *)dstImem.virtAddr;

    if(a_bitDepth == 8)
    {
        MUINT8 pixelValue;
        for(MUINT32 i = 0; i < (a_imgW * a_imgH); ++i)
        {
            pixelValue = *(pSrcBuf++);
            *(pDstBuf) = pixelValue;
        }
    }
    else if(a_bitDepth == 10)
    {
        MUINT8 *lineBuf;

        for(MUINT32 i = 0; i < a_imgH; ++i)
        {
            lineBuf = pSrcBuf + i * a_Stride;

            for(MUINT32 j = 0; j < (a_imgW / 4); ++j)
            {
                MUINT8 byte0 = (MUINT8)(*(lineBuf++));
                MUINT8 byte1 = (MUINT8)(*(lineBuf++));
                MUINT8 byte2 = (MUINT8)(*(lineBuf++));
                MUINT8 byte3 = (MUINT8)(*(lineBuf++));
                MUINT8 byte4 = (MUINT8)(*(lineBuf++));

                *(pDstBuf++) = (MUINT16)(byte0 + ((byte1 & 0x3) << 8));
                *(pDstBuf++) = (MUINT16)(((byte1 & 0xFC) >> 2) + ((byte2 & 0xF) << 6));
                *(pDstBuf++) = (MUINT16)(((byte2 & 0xF0) >> 4) + ((byte3 & 0x3F) << 4));
                *(pDstBuf++) = (MUINT16)(((byte3 & 0xC0) >> 6) + (byte4 << 2));
            }

            //process last pixel in the width
            if((a_imgW % 4) != 0)
            {
                MUINT8 byte0 = (MUINT8)(*(lineBuf++));
                MUINT8 byte1 = (MUINT8)(*(lineBuf++));
                MUINT8 byte2 = (MUINT8)(*(lineBuf++));
                MUINT8 byte3 = (MUINT8)(*(lineBuf++));
                MUINT8 byte4 = (MUINT8)(*(lineBuf++));

                for(MUINT32 j = 0; j < (a_imgW % 4); ++j)
                {
                    switch(j)
                    {
                        case 0 : *(pDstBuf++) = (MUINT16)(byte0 + ((byte1 & 0x3) << 8));
                            break;
                        case 1 : *(pDstBuf++) = (MUINT16)(((byte1 & 0x3F) >> 2) + ((byte2 & 0xF) << 6));
                            break;
                        case 2 : *(pDstBuf++) = (MUINT16)(((byte2 & 0xF0) >> 4) + ((byte3 & 0x3F) << 6));
                            break;
                        case 3 : *(pDstBuf++) = (MUINT16)(((byte3 & 0xC0) >> 6) + (byte4 << 2));
                            break;
                    }
                }
            }
        }
    }
    else if(a_bitDepth == 12)
    {
        MUINT8 *lineBuf;

        for(MUINT32 i = 0; i < a_imgH; ++i)
        {
            lineBuf = pSrcBuf + i * a_Stride;

            for(MUINT32 j = 0; j < (a_imgW / 4); ++j)
            {
                MUINT8 byte0 = (MUINT8)(*(lineBuf++));
                MUINT8 byte1 = (MUINT8)(*(lineBuf++));
                MUINT8 byte2 = (MUINT8)(*(lineBuf++));
                MUINT8 byte3 = (MUINT8)(*(lineBuf++));
                MUINT8 byte4 = (MUINT8)(*(lineBuf++));
                MUINT8 byte5 = (MUINT8)(*(lineBuf++));

                *(pDstBuf++) = (MUINT16)(byte0 + ((byte1 & 0xF) << 8));
                *(pDstBuf++) = (MUINT16)((byte1 >> 4) + (byte2 << 4));
                *(pDstBuf++) = (MUINT16)(byte3 + ((byte4 & 0xF) << 8));
                *(pDstBuf++) = (MUINT16)((byte4 >> 4) + (byte5 << 4));
            }

             //process last pixel in the width
            if((a_imgW % 4) != 0)
            {
                MUINT8 byte0 = (MUINT8)(*(lineBuf++));
                MUINT8 byte1 = (MUINT8)(*(lineBuf++));
                MUINT8 byte2 = (MUINT8)(*(lineBuf++));
                MUINT8 byte3 = (MUINT8)(*(lineBuf++));
                MUINT8 byte4 = (MUINT8)(*(lineBuf++));
                MUINT8 byte5 = (MUINT8)(*(lineBuf++));

                for(MUINT32 j = 0; j < (a_imgW % 4); ++j)
                {
                    switch(j)
                    {
                        case 0 : *(pDstBuf++) = (MUINT16)(byte0 + ((byte1 & 0xF) << 8));
                            break;
                        case 1 : *(pDstBuf++) = (MUINT16)((byte1 >> 4) + (byte2 << 4));
                            break;
                        case 2 : *(pDstBuf++) = (MUINT16)(byte3 + ((byte4 & 0xF) << 8));
                            break;
                        case 3 : *(pDstBuf++) = (MUINT16)((byte4 >> 4) + (byte5 << 4));
                            break;
                    }
                }
            }
        }
    }

    ACDK_LOGD_IF(mDebugEnable, "-");
    return ACDK_RETURN_NO_ERROR;
}


MBOOL AcdkUtility::init(void)
{
#if defined (USING_MTK_ION)
    mIonDrv = mt_ion_open("AcdkUtility");

    if (mIonDrv < 0) {
        ACDK_LOGE("ion device open FAIL ");
        return MFALSE;
    }
    ACDK_LOGD("Open ion id(%d)", mIonDrv);

    return MTRUE;
#else
    ACDK_LOGE("############ Fault: no ion lib exist #############");

    return MFALSE;
#endif
}

MBOOL AcdkUtility::uninit(void)
{
#if defined (USING_MTK_ION)
    if (mIonDrv) {
        ACDK_LOGD("close ion id(%d)", mIonDrv);
        ion_close(mIonDrv);
    }

    return MTRUE;
#else
    ACDK_LOGE("############ Fault: no ion lib exist #############");

    return MFALSE;
#endif
}

MINT32 AcdkUtility::allocVirtBuf(AMEM_BUF_INFO* pInfo)
{
#if defined (USING_MTK_ION)
    ion_user_handle_t pIonHandle;
    MINT32 IonBufFd;
    int ion_prot_flags = pInfo->useNoncache ? 0 : (ION_FLAG_CACHED | ION_FLAG_CACHED_NEEDS_SYNC);

    if (pInfo->size <= 0)
    {
        ACDK_LOGE("alloc size 0!");
        return -1;
    }

    #if 1 //ndef USING_MTK_LDVT   // Not using LDVT.
    if(ion_alloc(
        mIonDrv,
        pInfo->size,
        0, //32 //alignment
        ION_HEAP_MULTIMEDIA_MASK,
        ion_prot_flags,
        &pIonHandle))
    #else
    if(ion_alloc(
        mIonDrv,
        pInfo->size,
        0, //32 //alignment
        ION_HEAP_CARVEOUT_MASK,
		ion_prot_flags,
        &pIonHandle))
    #endif
    {
        ACDK_LOGE("ion_alloc fail, size(0x%x)",pInfo->size);
        return -1;
    }

    if(ion_share(
        mIonDrv,
        pIonHandle,
        &IonBufFd))
    {
        ACDK_LOGE("ion_share fail");
        return -1;
    }

    pInfo->memID = (MINT32)IonBufFd; // Tianshu suggest to keep this fd

    pInfo->virtAddr = (MUINTPTR)ion_mmap(mIonDrv,NULL, pInfo->size, PROT_READ|PROT_WRITE,
                                            MAP_SHARED, IonBufFd, 0);

    if (!pInfo->virtAddr)
    {
        ACDK_LOGE("Cannot map ion buffer. memID(0x%x)/size(0x%x)/va(0x%p)",
            pInfo->memID,pInfo->size,(void*)pInfo->virtAddr);
        return -1;
    }

	ACDK_LOGD("alloc ion flag(%d) memid(%d) va(%p) pa(%p) size(%d)",
        ion_prot_flags, pInfo->memID, (void*)pInfo->virtAddr, (void*)pInfo->phyAddr, pInfo->size);

    return 0;
#else
    ACDK_LOGE("############ Fault: no ion lib exist #############");

    return -1;
#endif
}

MINT32 AcdkUtility::freeVirtBuf(AMEM_BUF_INFO* pInfo)
{
#if defined (USING_MTK_ION)
    ion_user_handle_t pIonHandle;
    MINT32 IonBufFd;

    IonBufFd = pInfo->memID;

    if (ion_import(
        mIonDrv,
        IonBufFd,
        &pIonHandle))
    {
        ACDK_LOGE("ion_import fail, memID(0x%x)",IonBufFd);
        return -1;
    }

    if (ion_free(
        mIonDrv,
        pIonHandle))
    {
        ACDK_LOGE("ion_free fail");
        return -1;
    }

    ion_munmap(mIonDrv,(char*)pInfo->virtAddr, pInfo->size);
    ion_share_close(mIonDrv,pInfo->memID);

    if (ion_free(
        mIonDrv,
        pIonHandle))
    {
        ACDK_LOGE("ion_free fail");
        return -1;
    }

    return 0;
#else
    ACDK_LOGE("############ Fault: no ion lib exist #############");

    return -1;
#endif
}

MINT32 AcdkUtility::mapPhyAddr(AMEM_BUF_INFO* pInfo)
{
#if defined (USING_MTK_ION)
    ion_user_handle_t pIonHandle;
    MINT32 IonBufFd;
    MINT32 err;
    struct ion_mm_data mm_data;
    struct ion_sys_data sys_data;

    IonBufFd = pInfo->memID;

    if (ion_import(
        mIonDrv,
        IonBufFd,
        &pIonHandle))
    {
        ACDK_LOGE("ion_import fail, memID(0x%x)", IonBufFd);
        return -1;
    }

    mm_data.mm_cmd = ION_MM_CONFIG_BUFFER;
    mm_data.config_buffer_param.handle = (ion_user_handle_t)pIonHandle;
    mm_data.config_buffer_param.eModuleID = CAM_M4U_PORT_DMA_MODULEID;
    mm_data.config_buffer_param.security = pInfo->bufSecu;
    mm_data.config_buffer_param.coherent = pInfo->bufCohe;

    err = ion_custom_ioctl(mIonDrv, ION_CMD_MULTIMEDIA, &mm_data);
    if (err == (-ION_ERROR_CONFIG_LOCKED))
    {
        ACDK_LOGD("IOCTL[ION_IOC_CUSTOM] Double config after map phy address");
    }
    else if(err != 0)
    {
        ACDK_LOGE("IOCTL[ION_IOC_CUSTOM] ION_CMD_MULTIMEDIA Config Buffer failed!");
        return -1;
    }

    sys_data.sys_cmd = ION_SYS_GET_PHYS;
    sys_data.get_phys_param.handle = (ion_user_handle_t)pIonHandle;
    if (ion_custom_ioctl(mIonDrv, ION_CMD_SYSTEM, &sys_data))
    {
        ACDK_LOGE("IOCTL[ION_IOC_CUSTOM] Config Buffer failed!");
        return -1;
    }

    pInfo->phyAddr = (MUINTPTR)(sys_data.get_phys_param.phy_addr);

    if(ion_free(
        mIonDrv,
        pIonHandle))
    {
        ACDK_LOGE("ion_free fail");
        return -1;
    }

    //ACDK_LOGD("Physical address=0x%08X len=0x%X", sys_data.get_phys_param.phy_addr, sys_data.get_phys_param.len);

    ACDK_LOGD("mID(0x%x),size(0x%x),VA(x%p),PA(0x%p),S/C(%d/%d)",
        pInfo->memID,pInfo->size,(void*)pInfo->virtAddr,(void*)pInfo->phyAddr,pInfo->bufSecu,pInfo->bufCohe);

    return 0;
#else
    ACDK_LOGE("############ Fault: no ion lib exist #############");

    return -1;
#endif
}

MINT32  AcdkUtility::unmapPhyAddr(AMEM_BUF_INFO* pInfo)
{
    UNUSED(pInfo);
    return 0;
}

MINT32  AcdkUtility::cacheSyncbyRange(AMEM_CACHECTRL_ENUM ctrl, AMEM_BUF_INFO* pInfo)
{
    MINT32  err = 0;
#if defined (USING_MTK_ION)
    MINT32  IonBufFd;
    ion_user_handle_t pIonHandle;
    struct ion_sys_data sys_data;

    ACDK_LOGD("+, c/m/va/sz/pa(%d/0x%x/0x%p/0x%x/0x%p)",
                ctrl,pInfo->memID,(void*)pInfo->virtAddr, pInfo->size,(void*)pInfo->phyAddr);

    if ((ctrl == AMEM_CACHECTRL_ENUM_FLUSH) && (pInfo->size >= 0x3200000)) {
        //flush all if the buffer size is larger than 50M (suggested by K)
        //TBD......
    }

    //a. get handle of ION_IOC_SHARE from IonBufFd and increase handle ref count
    IonBufFd = pInfo->memID;
    if (ion_import(mIonDrv, IonBufFd, &pIonHandle)) {
        ACDK_LOGE("ion_import fail, memId(0x%x)", IonBufFd);
        err = -1;
        goto SYNC_EXIT;
    }

    //b. cache sync by range
    sys_data.sys_cmd = ION_SYS_CACHE_SYNC;
    sys_data.cache_sync_param.va = (void*)pInfo->virtAddr;
    sys_data.cache_sync_param.size = pInfo->size;
    sys_data.cache_sync_param.handle = (ion_user_handle_t)pIonHandle;

    switch(ctrl) {
        case AMEM_CACHECTRL_ENUM_FLUSH:
            sys_data.cache_sync_param.sync_type = ION_CACHE_FLUSH_BY_RANGE;
            break;
        case AMEM_CACHECTRL_ENUM_INVALID:
            sys_data.cache_sync_param.sync_type = ION_CACHE_INVALID_BY_RANGE;
            break;
        default:
            ACDK_LOGE("ERR cmd(%d)", ctrl);
            err = -1;
            goto SYNC_EXIT;
    }

    if (ion_custom_ioctl(mIonDrv, ION_CMD_SYSTEM, &sys_data)) {
        ACDK_LOGE("CAN NOT SYNC memID/(0x%x)", pInfo->memID);

        //decrease handle ref count if cache fail
        if (ion_free(mIonDrv, pIonHandle)) {
            ACDK_LOGE("ion_free fail");
        }

        err = -1;
        goto SYNC_EXIT;
    }

    //c. decrease handle ref count
    if (ion_free(mIonDrv, pIonHandle)) {
        ACDK_LOGE("ion_free fail");
        err = -1;
        goto SYNC_EXIT;
    }

SYNC_EXIT:
#else
    ACDK_LOGE("############ Fault: no ion lib exist #############");
    err = -1;
#endif
    return err;
}

