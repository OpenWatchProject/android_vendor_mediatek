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
#include "MediaTypes.h"
#include "mhal_jpeg.h" 
#include "jpeg_hal_dec.h"

#include "img_common_def.h"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS
#include "utils/Trace.h" 

extern "C" {
#include <pthread.h>
#include <semaphore.h>
}

#include <cutils/log.h>

//#undef ATRACE_TAG
//#define ATRACE_TAG ""


#define SW_PATH_SIZE (150*1024)
//#define SW_PATH_SIZE (150*1024*1024)
//#define CHECK_SW_SMALL

static JpgDecHal* jpgDecoder = NULL;

static pthread_mutex_t resMutex = PTHREAD_MUTEX_INITIALIZER;

static int return_false(const char msg[])
{
    if(jpgDecoder != NULL)
    {
        delete jpgDecoder;
        jpgDecoder = NULL;
    }

    JPG_ERR("%s", msg);
    
    return JPEG_ERROR_INVALID_DRIVER;
}

int mHalJpgDecParser(unsigned char* srcAddr, unsigned int srcSize, int srcFD, unsigned int img_type )
{
    // unused params
    (void)srcFD;

    unsigned int fg = 1;
    char acProcBuf[256];
    unsigned int check_sw_size = 0;
    {   
        char acBuf[256];
        sprintf(acBuf, "/proc/%d/cmdline", getpid());
        FILE *fp = fopen(acBuf, "r");
        if (fp)
        {
            fread(acBuf, 1, sizeof(acBuf), fp);
            fclose(fp);
    #ifdef CHECK_SW_SMALL        
            if(strncmp(acBuf, "com.android.browser", 19) == 0)
            {
               //JPG_WARN("browser check sw size!!\n");
               check_sw_size = 1;
            }
    #endif        
        }
    }   
   
   
    pthread_mutex_lock(&resMutex);
    
    if(jpgDecoder != NULL) 
    {
        JPG_WARN("hw decoder is busy");
        pthread_mutex_unlock(&resMutex);
        //return JPEG_ERROR_INVALID_DRIVER;
        return MHAL_INVALID_RESOURCE;
    }
    jpgDecoder = new JpgDecHal();

    pthread_mutex_unlock(&resMutex);

    if(img_type == MHAL_IMAGE_TYPE_WEBP){
      if(!jpgDecoder->setDecType(JpgDecHal::IMG_DEC_TYPE_WEBP))
         return return_false("decoder can't support webp image!!");
    }else
      jpgDecoder->setDecType(JpgDecHal::IMG_DEC_TYPE_JPEG);

#ifdef BACK_GROUND_LOW
{
    
    sprintf(acProcBuf, "/proc/%d/oom_adj", getpid());
    FILE *fp = fopen(acProcBuf, "r");
    if (fp)
    {
        fread(acProcBuf, 1, sizeof(acProcBuf), fp);
        fclose(fp);
        if(strncmp(acProcBuf, "0", 1) == 0){
          //fg = 1;
        }else{
          fg = 0;
          //JPG_WARN("mHalJpgDec:: fg(%d), L:%d!!\n",fg , __LINE__);          
        }
    }
}
#endif
    
    if(fg == 0 || !jpgDecoder->setScalerType(JpgDecHal::IMG_RESCALE_TYPE_LINK))
      jpgDecoder->setScalerType(JpgDecHal::IMG_RESCALE_TYPE_TILE);
         
    if(!jpgDecoder->lock())
    {
        return return_false("can't lock decoder resource");
    }

    //JPG_DBG("mHalJpgDec:: parser lock success,fg(%d),SWS 0x%x, L:%d!!\n",fg, SW_PATH_SIZE,__LINE__);
    jpgDecoder->setSrcAddr(srcAddr);
    jpgDecoder->setSrcSize(srcSize, srcSize);
    
    if(!jpgDecoder->parse()) { return return_false("no support file format"); }

    if(check_sw_size && fg && (jpgDecoder->getJpgWidth() * jpgDecoder->getJpgHeight() < SW_PATH_SIZE) )
    { 
      return return_false("default go sw path size");
    }

    if(!jpgDecoder->lockResizer()){ return return_false("can't lock resizer resource"); }

    //JPG_DBG("mHalJpgDec:: parse success, fg(%d), L:%d!!\n",fg ,__LINE__);
    return 0;
}

int mHalJpgDecGetInfo(MHAL_JPEG_DEC_INFO_OUT *decOutInfo)
{
    //JPG_DBG("mHalJpgDec:: getInfo L:%d!!\n", __LINE__);
    if(jpgDecoder == NULL) { return return_false("null decoder"); }
    
    decOutInfo->srcWidth = jpgDecoder->getJpgWidth();
    decOutInfo->srcHeight = jpgDecoder->getJpgHeight();

    return 0;
}
 
int mHalJpgDecStart(MHAL_JPEG_DEC_START_IN *decInParams, void* procHandler)
{
    if(jpgDecoder == NULL) { return return_false("null decoder"); }

    if(decInParams == NULL)
    {
        JPG_ERR("mHalJpgDec:: cancel Decoder L:%d!!\n", __LINE__);
        jpgDecoder->unlock();
        delete jpgDecoder;
        jpgDecoder = NULL;

        return 0;
    } 
    if(decInParams->srcBuffer != NULL)
    {
        jpgDecoder->setSrcAddr(decInParams->srcBuffer);
        jpgDecoder->setSrcSize(decInParams->srcBufSize, decInParams->srcLength);
    }
    ATRACE_CALL();
    //android::Tracer::traceBegin(ATRACE_TAG_GRAPHICS,"MHAL_JPEG_DEC_DTART");
    jpgDecoder->setOutWidth(decInParams->dstWidth);
    jpgDecoder->setOutHeight(decInParams->dstHeight);

    unsigned int dst_size = decInParams->dstWidth * decInParams->dstHeight;
    switch (decInParams->dstFormat)
    {
        case JPEG_OUT_FORMAT_RGB565:
            jpgDecoder->setOutFormat(JpgDecHal::kRGB_565_Format);
            dst_size *= 2;
            break;

        case JPEG_OUT_FORMAT_RGB888:
            jpgDecoder->setOutFormat(JpgDecHal::kRGB_888_Format);
            dst_size *= 3;
            break;

        case JPEG_OUT_FORMAT_ARGB8888:
            jpgDecoder->setOutFormat(JpgDecHal::kARGB_8888_Format);
            dst_size *= 4;
            break;

        case JPEG_OUT_FORMAT_YUY2:
            jpgDecoder->setOutFormat(JpgDecHal::kYUY2_Pack_Format);
            dst_size *= 2;
            break;

        case JPEG_OUT_FORMAT_UYVY:
            jpgDecoder->setOutFormat(JpgDecHal::kUYVY_Pack_Format);
            dst_size *= 2;
            break;
            
        default:
            jpgDecoder->setOutFormat(JpgDecHal::kRGB_565_Format);
            dst_size *= 2;
            break;
    }
    //JPG_DBG("mhalJpegDecoder: src %p, srcSize %x, dst %p, size %x, w %d, h %d, fmt %d, PQ %d, IDCT %d!!\n",
    //    (void*)decInParams->srcBuffer, decInParams->srcLength, (void*)decInParams->dstVirAddr,dst_size,decInParams->dstWidth,
    //    decInParams->dstHeight,decInParams->dstFormat, decInParams->doPostProcessing, decInParams->PreferQualityOverSpeed);
    jpgDecoder->setDstAddr(decInParams->dstVirAddr);
    jpgDecoder->setDstSize(dst_size);
    jpgDecoder->setProcHandler(procHandler);
    
    if(decInParams->doDithering == 0)
        jpgDecoder->setDither(false);
    else
        jpgDecoder->setDither(true);
    
    jpgDecoder->setPQlevel(decInParams->doPostProcessing); 
    jpgDecoder->setQualityLevel(decInParams->PreferQualityOverSpeed);
    jpgDecoder->setPreferDecodeDownSample(0);

    if(decInParams->doRangeDecode == 1)
    {
        jpgDecoder->setRangeDecode(decInParams->rangeLeft, decInParams->rangeTop, 
                                   decInParams->rangeRight, decInParams->rangeBottom);
    }

    if(!jpgDecoder->start())
    {
        JPG_ERR("mHalJpgDec:: trigger Decoder Fail L:%d!!\n", __LINE__);    
        jpgDecoder->unlock();
        delete jpgDecoder;
        jpgDecoder = NULL;
        //android::Tracer::traceEnd(ATRACE_TAG_GRAPHICS);    
        return return_false("decode failed~~~");
    }

    jpgDecoder->unlock();
    delete jpgDecoder;
    jpgDecoder = NULL;
    //android::Tracer::traceEnd(ATRACE_TAG_GRAPHICS);    
    return 0;
}

int mi_return_false(void** jDecHandle, const char msg[])
{
    JpgDecHal* jHalDecoder = (JpgDecHal *)*jDecHandle;
    if(jHalDecoder != NULL)
    {
        delete jHalDecoder;
        jHalDecoder = NULL;
    }

    JPG_ERR("%s", msg);
    
    return JPEG_ERROR_INVALID_DRIVER;
}


int mi_mHalJpgDecParser(MHAL_JPEG_DEC_SRC_IN *srcInParams, unsigned char* srcAddr, unsigned int srcSize, int srcFD, unsigned int img_type )
{
    // unused params
    (void)srcFD;

    unsigned int fg = 1;
    char acProcBuf[256];
    unsigned int check_sw_size = 0;

    {   
        char acBuf[256];
        sprintf(acBuf, "/proc/%d/cmdline", getpid());
        FILE *fp = fopen(acBuf, "r");
        if (fp)
        {
            fread(acBuf, 1, sizeof(acBuf), fp);
            fclose(fp);
    #ifdef CHECK_SW_SMALL
            if(strncmp(acBuf, "com.android.browser", 19) == 0)
            {
               //JPG_WARN("browser check sw size!!\n");
               check_sw_size = 1;
            }
    #endif
        }
    }

#ifdef JPEG_HW_DECODER_MODULE
    pthread_mutex_lock(&resMutex);

    if(jpgDecoder != NULL) 
    {
        JPG_WARN("hw decoder is busy");
        pthread_mutex_unlock(&resMutex);
        //return JPEG_ERROR_INVALID_DRIVER;
        return MHAL_INVALID_RESOURCE;
    }

    pthread_mutex_unlock(&resMutex);
#endif

    void **jDecHandle = (void **)&srcInParams->jpgDecHandle;
    JpgDecHal* jHalDecoder = new JpgDecHal();
    *jDecHandle = (void *)jHalDecoder;

    if(img_type == MHAL_IMAGE_TYPE_WEBP){
      if(!jHalDecoder->setDecType(JpgDecHal::IMG_DEC_TYPE_WEBP))
         return mi_return_false( jDecHandle, "decoder can't support webp image!!");
    }else
      jHalDecoder->setDecType(JpgDecHal::IMG_DEC_TYPE_JPEG);

#ifdef BACK_GROUND_LOW
{
    sprintf(acProcBuf, "/proc/%d/oom_adj", getpid());
    FILE *fp = fopen(acProcBuf, "r");
    if (fp)
    {
        fread(acProcBuf, 1, sizeof(acProcBuf), fp);
        fclose(fp);
        if(strncmp(acProcBuf, "0", 1) == 0){
          //fg = 1;
        }else{
          fg = 0;
          //JPG_WARN("mHalJpgDec:: fg(%d), L:%d!!\n",fg , __LINE__);
        }
    }
}
#endif

    if(fg == 0 || !jHalDecoder->setScalerType(JpgDecHal::IMG_RESCALE_TYPE_LINK))
      jHalDecoder->setScalerType(JpgDecHal::IMG_RESCALE_TYPE_TILE);

    if(!jHalDecoder->lock())
    {
        return mi_return_false( jDecHandle, "can't lock decoder resource");
    }

    //JPG_DBG("mHalJpgDec:: parser lock success,fg(%d),SWS 0x%x, L:%d!!\n",fg, SW_PATH_SIZE,__LINE__);
    jHalDecoder->setSrcAddr(srcAddr);
    jHalDecoder->setSrcSize(srcSize, srcSize);
    
    if(!jHalDecoder->parse()) { return mi_return_false( jDecHandle, "no support file format"); }

    if(check_sw_size && fg && (jHalDecoder->getJpgWidth() * jHalDecoder->getJpgHeight() < SW_PATH_SIZE) )
    {
      return mi_return_false( jDecHandle, "default go sw path size");
    }

    if(!jHalDecoder->lockResizer()){ return mi_return_false( jDecHandle, "can't lock resizer resource"); }

    //JPG_DBG("mHalJpgDec:: parse success, fg(%d), L:%d!!\n",fg ,__LINE__);
    return 0;
}

int mi_mHalJpgDecGetInfo( MHAL_JPEG_DEC_INFO_OUT *decOutInfo)
{
   void **jDecHandle = (void **)&decOutInfo->jpgDecHandle ;
   JpgDecHal* jHalDecoder = (JpgDecHal*) *jDecHandle;
    //JPG_DBG("mHalJpgDec:: getInfo L:%d!!\n", __LINE__);
    if(jHalDecoder == NULL) { return mi_return_false( jDecHandle, "null decoder"); }

    decOutInfo->srcWidth = jHalDecoder->getJpgWidth();
    decOutInfo->srcHeight = jHalDecoder->getJpgHeight();

    return 0;
}


int mi_mHalJpgDecStart( MHAL_JPEG_DEC_START_IN *decInParams, void* procHandler)
{
   void **jDecHandle = (void **)&decInParams->jpgDecHandle;
   JpgDecHal* jHalDecoder = (JpgDecHal*) *jDecHandle;
    if(jHalDecoder == NULL) { return mi_return_false( (void**)jDecHandle, "null decoder"); }

    if(decInParams->fCancelDecoderFlag)
    {
        JPG_ERR("mHalJpgDec:: cancel Decoder L:%d!!\n", __LINE__);
        jHalDecoder->unlock();
        delete jHalDecoder;
        jHalDecoder = NULL;

        return 0;
    } 
    if(decInParams->srcBuffer != NULL)
    {
        jHalDecoder->setSrcFD(decInParams->srcFD);
        jHalDecoder->setSrcAddr(decInParams->srcBuffer);
        jHalDecoder->setSrcSize(decInParams->srcBufSize, decInParams->srcLength);
    }
    ATRACE_CALL();
    //android::Tracer::traceBegin(ATRACE_TAG_GRAPHICS,"MHAL_JPEG_DEC_DTART");
    jHalDecoder->setOutWidth(decInParams->dstWidth);
    jHalDecoder->setOutHeight(decInParams->dstHeight);

    unsigned int dst_size = (decInParams->dstStride != 0)? decInParams->dstStride * decInParams->dstHeight:
                                                           decInParams->dstWidth * decInParams->dstHeight;
    unsigned int dst_csize = 0;
    switch (decInParams->dstFormat)
    {
        case JPEG_OUT_FORMAT_RGB565:
            jHalDecoder->setOutFormat(JpgDecHal::kRGB_565_Format);
            if (decInParams->dstStride != 0)
            {
                if (decInParams->dstStride >= (decInParams->dstWidth << 1))
                    jHalDecoder->setOutStride(decInParams->dstStride, decInParams->dstStride);
                else
                {
                    JPG_ERR("mHalJpgDec:: stride error! expected %d but only %d\n", (decInParams->dstWidth << 1), decInParams->dstStride);
                    goto ERR_EXIT;
                }
            }
            else
                dst_size = dst_size << 1;
            break;

        case JPEG_OUT_FORMAT_RGB888:
            jHalDecoder->setOutFormat(JpgDecHal::kRGB_888_Format);
            if (decInParams->dstStride != 0)
            {
                if (decInParams->dstStride >= (decInParams->dstWidth * 3))
                    jHalDecoder->setOutStride(decInParams->dstStride, decInParams->dstStride);
                else
                {
                    JPG_ERR("mHalJpgDec:: stride error! expected %d but only %d\n", (decInParams->dstWidth * 3), decInParams->dstStride);
                    goto ERR_EXIT;
                }
            }
            else
                dst_size = dst_size * 3;
            break;

        case JPEG_OUT_FORMAT_ARGB8888:
            jHalDecoder->setOutFormat(JpgDecHal::kARGB_8888_Format);
            if (decInParams->dstStride != 0)
            {
                if (decInParams->dstStride >= (decInParams->dstWidth << 2))
                    jHalDecoder->setOutStride(decInParams->dstStride, decInParams->dstStride);
                else
                {
                    JPG_ERR("mHalJpgDec:: stride error! expected %d but only %d\n", (decInParams->dstWidth << 2), decInParams->dstStride);
                    goto ERR_EXIT;
                }
            }
            else
                dst_size = dst_size << 2;
            break;

        case JPEG_OUT_FORMAT_YUY2:
            jHalDecoder->setOutFormat(JpgDecHal::kYUY2_Pack_Format);
            if (decInParams->dstStride != 0)
            {
                if (decInParams->dstStride >= (decInParams->dstWidth << 1))
                    jHalDecoder->setOutStride(decInParams->dstStride, decInParams->dstStride);
                else
                {
                    JPG_ERR("mHalJpgDec:: stride error! expected %d but only %d\n", (decInParams->dstWidth << 1), decInParams->dstStride);
                    goto ERR_EXIT;
                }
            }
            else
                dst_size = dst_size << 1;
            break;

        case JPEG_OUT_FORMAT_UYVY:
            jHalDecoder->setOutFormat(JpgDecHal::kUYVY_Pack_Format);
            if (decInParams->dstStride != 0)
            {
                if (decInParams->dstStride >= (decInParams->dstWidth << 1))
                    jHalDecoder->setOutStride(decInParams->dstStride, decInParams->dstStride);
                else
                {
                    JPG_ERR("mHalJpgDec:: stride error! expected %d but only %d\n", (decInParams->dstWidth << 1), decInParams->dstStride);
                    goto ERR_EXIT;
                }
            }
            else
                dst_size = dst_size << 1;
            break;

        case JPEG_OUT_FORMAT_YV12:
            jHalDecoder->setOutFormat(JpgDecHal::kYVU420_YV12_Format);
            if (decInParams->dstStride != 0)
            {
                if (decInParams->dstStride >= TO_CEIL( decInParams->dstWidth, 16 ))
                {
                    jHalDecoder->setOutStride(decInParams->dstStride, (decInParams->dstStride >> 1));
                    dst_csize = (dst_size >> 2);
                }
                else
                {
                    JPG_ERR("mHalJpgDec:: stride error! expected %d but only %d\n", TO_CEIL( decInParams->dstStride, 16 ), decInParams->dstStride);
                    goto ERR_EXIT;
                }
            }
            else
            {
                dst_size = TO_CEIL( decInParams->dstWidth, 16 ) * decInParams->dstHeight;
                dst_csize = (dst_size >> 2);
            }
            break;

        case JPEG_OUT_FORMAT_I420:
            jHalDecoder->setOutFormat(JpgDecHal::kYVU420_3PLANE_Format);
            if (decInParams->dstStride != 0)
            {
                if (decInParams->dstStride >= TO_CEIL( decInParams->dstWidth, 16 ))
                {
                    jHalDecoder->setOutStride(decInParams->dstStride, (decInParams->dstStride >> 1));
                    dst_csize = (dst_size >> 2);
                }
                else
                {
                    JPG_ERR("mHalJpgDec:: stride error! expected %d but only %d\n", TO_CEIL( decInParams->dstStride, 16 ), decInParams->dstStride);
                    goto ERR_EXIT;
                }
            }
            else
            {
                dst_size = TO_CEIL( decInParams->dstWidth, 16 ) * decInParams->dstHeight;
                dst_csize = (dst_size >> 2);
            }
            break;

        case JPEG_OUT_FORMAT_YV12_ANDROID:
            jHalDecoder->setOutFormat(JpgDecHal::kYVU420_YV12_ANDROID_Format);
            if (decInParams->dstStride != 0)
            {
                if (decInParams->dstStride >= TO_CEIL( decInParams->dstWidth, 16 ) && (decInParams->dstStride >> 1) >= TO_CEIL( (decInParams->dstWidth >> 1) , 16 ))
                {
                    jHalDecoder->setOutStride(decInParams->dstStride, decInParams->dstStride >> 1);
                    dst_csize = (dst_size >> 2);
                }
                else
                {
                    JPG_ERR("mHalJpgDec:: stride error! expected %d but only %d\n", TO_CEIL( decInParams->dstStride, 16 ), decInParams->dstStride);
                    goto ERR_EXIT;
                }
            }
            else
            {
                dst_size = TO_CEIL( decInParams->dstWidth, 16 ) * decInParams->dstHeight;
                dst_csize = TO_CEIL( decInParams->dstWidth/2, 16 ) * decInParams->dstHeight/2;
            }
            break;

        default:
            jHalDecoder->setOutFormat(JpgDecHal::kRGB_565_Format);
            dst_size *= 2;
            break;
    }
    //JPG_DBG("mhalJpegDecoder: src %p, srcSize %x, dst %p, size %x %x, w %d, h %d, fmt %d, PQ %d, IDCT %d!!\n",
    //    (void*)decInParams->srcBuffer, decInParams->srcLength, (void*)decInParams->dstVirAddr,dst_size, dst_csize,decInParams->dstWidth,
    //    decInParams->dstHeight,decInParams->dstFormat, decInParams->doPostProcessing, decInParams->PreferQualityOverSpeed);
    // There is no need to initial dst buffer
    //memset((void*)(decInParams->dstVirAddr),0x80,dst_size);
    jHalDecoder->setDstFD(decInParams->dstFD);
    jHalDecoder->setDstAddr(decInParams->dstVirAddr);
    jHalDecoder->setDstSize(dst_size);
    if(decInParams->dstFormat == JPEG_OUT_FORMAT_YV12 || decInParams->dstFormat == JPEG_OUT_FORMAT_I420) {
      jHalDecoder->setDstChromaAddr((unsigned char*)(decInParams->dstVirAddr+dst_size), (unsigned char*)(decInParams->dstVirAddr+dst_size+dst_csize));
      jHalDecoder->setDstChromaSize(dst_csize, dst_csize);
    }
    jHalDecoder->setProcHandler(procHandler);

    if(decInParams->doDithering == 0)
        jHalDecoder->setDither(false);
    else
        jHalDecoder->setDither(true);

    jHalDecoder->setPQlevel(decInParams->doPostProcessing);
    if (decInParams->postProcessingParam)
        jHalDecoder->setPostProcParam(decInParams->postProcessingParam->imageDCParam,
                                      decInParams->postProcessingParam->ISOSpeedRatings);

    jHalDecoder->setQualityLevel(decInParams->PreferQualityOverSpeed);
    jHalDecoder->setPreferDecodeDownSample(1);

    if(decInParams->doRangeDecode == 1)
    {
        jHalDecoder->setRangeDecode(decInParams->rangeLeft, decInParams->rangeTop,
                                   decInParams->rangeRight, decInParams->rangeBottom);
    }

    if(!jHalDecoder->start())
    {
ERR_EXIT:
        JPG_ERR("mHalJpgDec:: trigger Decoder Fail L:%d!!\n", __LINE__);    
        jHalDecoder->unlock();
        return mi_return_false( (void**)jDecHandle, "decode failed~~~");
    }

    jHalDecoder->unlock();
    delete jHalDecoder;
    jHalDecoder = NULL;
    decInParams->jpgDecHandle = NULL;
    //android::Tracer::traceEnd(ATRACE_TAG_GRAPHICS);
    return JPEG_ERROR_NONE;
}

