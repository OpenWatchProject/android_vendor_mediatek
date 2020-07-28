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



#define MTK_LOG_ENABLE 1
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include <MediaHal.h>
#include "MediaTypes.h"

#include "img_cdp_pipe.h"

extern "C" {
#include <pthread.h>
#include <semaphore.h>
}

#include <cutils/log.h>

#define xlog(...) \
        do { \
            ALOGW(__VA_ARGS__); \
        } while (0)

#undef LOG_TAG
#define LOG_TAG "mHalImgScaler"

static ImgCdpPipe* imgScaler = NULL;

static pthread_mutex_t resScalerMutex = PTHREAD_MUTEX_INITIALIZER;

static int return_false(const char msg[])
{
    if(imgScaler != NULL)
    {
        delete imgScaler;
        imgScaler = NULL;
    }

    xlog("%s", msg);
    
    return false;//JPEG_ERROR_INVALID_DRIVER;
}


 
int mHalScaler_BitBlt( mHalBltParam_t* bltParam )
{
   
   unsigned int inFmt = 0, pSrcSize = 1;
   unsigned int outFmt = 0, pDstSize = 1;;
   
    pthread_mutex_lock(&resScalerMutex);
    
    if(imgScaler != NULL) 
    {
        xlog("hw decoder is busy");
        pthread_mutex_unlock(&resScalerMutex);
        //return JPEG_ERROR_INVALID_DRIVER;
        return MHAL_INVALID_RESOURCE;
    }
    imgScaler = new ImgCdpPipe();
    
    pthread_mutex_unlock(&resScalerMutex);

    if(!imgScaler->lock(ImgCdpPipe::SCALER_MODE_FRAME))
    {
        return return_false("can't lock resource");
    }
   
   
    if(imgScaler == NULL) { return return_false("null scaler"); }

    if(bltParam == NULL)
    {
        imgScaler->unlock();
        delete imgScaler;
        imgScaler = NULL;

        return true;
    }

    switch(bltParam->srcFormat){
     case MHAL_FORMAT_BGR_888:
       inFmt = ImgCdpPipe::SCALER_IN_1P_RGB888 ;
       pSrcSize = 3;
         break;         
     case MHAL_FORMAT_ABGR_8888:
       inFmt = ImgCdpPipe::SCALER_IN_1P_ARGB8888 ;
       pSrcSize = 4;
         break;         
     case MHAL_FORMAT_RGB_565:
       inFmt = ImgCdpPipe::SCALER_IN_1P_RGB565 ;      
       pSrcSize = 2 ;
         break;         
     case MHAL_FORMAT_Y800:
       inFmt = ImgCdpPipe::SCALER_IN_1P_GRAY ;        
       pSrcSize = 1;
         break;
     default: 
            ALOGW("MDPResizer : unvalid src bitmap config %d!!\n", bltParam->srcFormat);
            return false;
    }
    
    switch(bltParam->dstFormat)
    {
        case MHAL_FORMAT_ABGR_8888:
         outFmt = ImgCdpPipe::SCALER_OUT_1P_ARGB8888 ;
         pDstSize = 4;
            break;

        case MHAL_FORMAT_RGB_565:
         outFmt = ImgCdpPipe::SCALER_OUT_1P_RGB565 ;
         pDstSize = 2;
            break;            
        default :
            ALOGW("MDPResizer : unvalid dst bitmap config %d!!\n", bltParam->dstFormat);
            return false;
    }    
    



    imgScaler->setSrcImageColor(inFmt);
    //set tile only configure
    //imgScaler->setTileBufRowNum(fTileBufRowNum);
    //imgScaler->setSrcRowHeight(fImgMcuHeight[0],fImgMcuHeight[1]);
        
    imgScaler->setSrcImageSize( bltParam->srcW,  bltParam->srcH, 0, 0);    
    imgScaler->setSrcBufSize( bltParam->srcW * bltParam->srcH * pSrcSize, 0);    
    imgScaler->setSrcBufStride( bltParam->srcWStride, 0);
    imgScaler->setSrcAddr0((unsigned int) bltParam->srcAddr, 0, 0);



    imgScaler->setDstImageColor(outFmt);
    imgScaler->setDstImageSize(bltParam->dstW, bltParam->dstH);    
    imgScaler->setDstBufSize(bltParam->dstW * bltParam->dstH * pDstSize);
    imgScaler->setDstAddr(bltParam->dstAddr);    
    //imgScaler->setDstHandler((unsigned long)fProcHandler);
    
//    if(fIonEn && fDstFD >= 0)
//      imgScaler->setDstFD(fDstFD);

#if 0
    unsigned int dst_size = decInParams->dstWidth * decInParams->dstHeight;
    switch (decInParams->dstFormat)
    {
        case JPEG_OUT_FORMAT_RGB565:
            imgScaler->setOutFormat(JpgDecHal::kRGB_565_Format);
            dst_size *= 2;
            break;

        case JPEG_OUT_FORMAT_RGB888:
            imgScaler->setOutFormat(JpgDecHal::kRGB_888_Format);
            dst_size *= 3;
            break;

        case JPEG_OUT_FORMAT_ARGB8888:
            imgScaler->setOutFormat(JpgDecHal::kARGB_8888_Format);
            dst_size *= 4;
            break;

        case JPEG_OUT_FORMAT_YUY2:
            imgScaler->setOutFormat(JpgDecHal::kYUY2_Pack_Format);
            dst_size *= 2;
            break;

        case JPEG_OUT_FORMAT_UYVY:
            imgScaler->setOutFormat(JpgDecHal::kUYVY_Pack_Format);
            dst_size *= 2;
            break;
            
        default:
            imgScaler->setOutFormat(JpgDecHal::kRGB_565_Format);
            dst_size *= 2;
            break;
    }
    xlog("mhalJpegDecoder: src %x, dst %x, size %x, w %d, h %d, fmt %d!!\n",decInParams->srcBuffer,decInParams->dstVirAddr,dst_size,decInParams->dstWidth, decInParams->dstHeight,decInParams->dstFormat);
    imgScaler->setDstAddr(decInParams->dstVirAddr);
    imgScaler->setDstSize(dst_size);
    imgScaler->setProcHandler(procHandler);

    
    if(decInParams->doDithering == 0)
        imgScaler->setDither(false);
    else
        imgScaler->setDither(true);
#endif

    
    if(!imgScaler->Start())
    {
        imgScaler->unlock();
        delete imgScaler;
        imgScaler = NULL;
        return return_false("decode failed~~~");
    }

    imgScaler->unlock();
    delete imgScaler;
    imgScaler = NULL;
    
    return true;
}

 