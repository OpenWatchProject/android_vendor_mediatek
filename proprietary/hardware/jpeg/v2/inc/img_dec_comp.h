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

#ifndef __IMG_DEC_COMP_H__
#define __IMG_DEC_COMP_H__

#ifdef MTK_LOG_ENABLE
#undef MTK_LOG_ENABLE
#endif
#define MTK_LOG_ENABLE 1
#include <cutils/log.h>

/*******************************************************************************
*
********************************************************************************/
#ifndef JUINT32
typedef unsigned int JUINT32;
#endif

#ifndef JINT32
typedef int JINT32;
#endif
 

//#define TO_CEIL(x,a) ( ((x) + ((a)-1)) & ~((a)-1) )
#ifdef LOG_TAG
 #undef LOG_TAG
#endif

//#define JPEG_PROFILING   

#define LOG_TAG "ImageDecoder"

#define IMGDEC_W(fmt, arg...)    ALOGW(fmt, ##arg)
#define IMGDEC_D(fmt, arg...)    ALOGD(fmt, ##arg)



/*******************************************************************************
* class ImageDecoder
********************************************************************************/
class ImageDecoder {
public:
    ImageDecoder(){
       //IMGDEC_D("ImageDecoder::ImageDecoder()\n");
    };
    virtual ~ImageDecoder(){
       //IMGDEC_D("ImageDecoder::~ImageDecoder()\n");
    };

    enum ColorFormat {
        IMGDEC_3PLANE_NONE,
        IMGDEC_3PLANE_YUV420,       
        IMGDEC_3PLANE_YUV422,      
        IMGDEC_3PLANE_GRAY,
        IMGDEC_3PLANE_YUV444
        
    }; 

    enum JpegColorFormat {
        IMG_COLOR_NONE,
        IMG_COLOR_YUV420,       
        IMG_COLOR_YUV422,  //2x1, 1x1, 1x1      
        IMG_COLOR_YUV444,       
        IMG_COLOR_YUV422V, //1x2, 1x1, 1x1      
        IMG_COLOR_GRAY,
        IMG_COLOR_NO_SUPPORT, 
        IMG_COLOR_INVALID
    }; 

    enum  {
        IMG_DEC_MODE_NONE ,        
        IMG_DEC_MODE_FRAME, // frame mode
        IMG_DEC_MODE_LINK ,  // LINK to GDMA
        IMG_DEC_MODE_TILE   // tile mode do row decode
    }  ;
    
    enum {
      IMG_MEM_TYPE_PHY,
      IMG_MEM_TYPE_M4U,
      IMG_MEM_TYPE_ION
      
    };

    enum IMG_DEC_RESULT {
      IMG_DEC_RST_CFG_ERR, //false
      IMG_DEC_RST_DONE,    // true
      IMG_DEC_RST_ROW_DONE,
      IMG_DEC_RST_HUFF_ERROR,
      IMG_DEC_RST_DECODE_FAIL,
      IMG_DEC_RST_FORMAT_FAIL,
      IMG_DEC_RST_BS_UNDERFLOW
      
    };

    virtual bool lock() {return true;}

    virtual bool unlock(){return true;}
    virtual bool parse(){return true;}

    //virtual bool Start(){return true;}
    //virtual bool ResumeDecRow( JUINT32 decRowNum){return true;}
    
    virtual JUINT32 StartConfig(){return true;}
    virtual JUINT32 Start(){return true;}
    virtual JUINT32 ResumeDecRow(JUINT32 decRowNum) {
      // unused params
      (void)decRowNum;

      return true;
    }
    
    virtual bool Config(JUINT32 mode) {
      // unused params
      (void)mode;

      return true;
    }
    
    
    virtual JUINT32 getOutFormat() { return 0; }

    virtual JUINT32 getJpgWidth() { return 0; }
    virtual JUINT32 getJpgHeight() { return 0; }
    virtual JUINT32 getMcuRow() { return 0; }
    virtual JUINT32 getMcuInRow() { return 0; }
    virtual JUINT32 getMcuHeightY() { return 0; }
    virtual JUINT32 getMcuHeightC() { return 0; }
    
    
    virtual JUINT32 getMinLumaBufSize() { return 0; }
    virtual JUINT32 getMinCbCrBufSize() { return 0; }

    virtual JUINT32 getMinLumaBufStride() { return 0; }
    virtual JUINT32 getMinCbCrBufStride() { return 0; }

    virtual JUINT32 getLumaImgStride() { return 0; }
    virtual JUINT32 getCbCrImgStride() { return 0; }
  
    virtual JUINT32 getLumaImgHeight() { return 0; }
    virtual JUINT32 getCbCrImgHeight() { return 0; }

    
    // in VA
    virtual void setSrcAddr(unsigned char * srcAddr) {
      // unused params
      (void)srcAddr;

      return;
    }

    virtual void setSrcSize(JUINT32 srcBufSize, JUINT32 srcSize) {
      // unused params
      (void)srcBufSize;
      (void)srcSize;

      return;
    }


    // out VA
    virtual void setDstAddr0(unsigned char *dstAddr, unsigned char *dstAddr1, unsigned char *dstAddr2) {
      // unused params
      (void)dstAddr;
      (void)dstAddr1;
      (void)dstAddr2;

      return;
    }
    
    virtual void setDstAddr1(unsigned char *dstAddr, unsigned char *dstAddr1, unsigned char *dstAddr2) {
      // unused params
      (void)dstAddr;
      (void)dstAddr1;
      (void)dstAddr2;

      return;
    }

    virtual void setIonMode(bool ionEn) {
      // unused params
      (void)ionEn;

      return;
    }

    virtual void setSrcFD( JINT32 srcFD ) {
      // unused params
      (void)srcFD;

      return;
    }

    virtual void setDstFD( JINT32 dstFD ) {
      // unused params
      (void)dstFD;

      return;
    }
    
    //current NOUSE
    virtual void setDstBufStride( JUINT32 dstStride, JUINT32 dstStride1) {
      // unused params
      (void)dstStride;
      (void)dstStride1;

      return;
    }

    /* make sure output valid buffer size */
    virtual void setDstBufSize(JUINT32 dstSize, JUINT32 dstSize1) {
      // unused params
      (void)dstSize;
      (void)dstSize1;

      return;
    }

    // set additional brz actor, should check if valid factor
    virtual void setBRZ(JUINT32 brz) {
      // unused params
      (void)brz;

      return;
    }
    virtual void setTileBufNum(JUINT32 ringBufRowNum) {
      // unused params
      (void)ringBufRowNum;

      return;
    }
    virtual bool setFirstDecRow(JUINT32 rowNum) {
      // unused params
      (void)rowNum;

      return true;
    }
    virtual bool setQuality(JUINT32 level) {
      // unused params
      (void)level;

      return true;
    }
    
    virtual bool setCmdqMode(JUINT32 enable) {
      // unused params
      (void)enable;

      return false;
    }

    virtual bool getCmdqMode(void) { return false; }
    
};

#endif 