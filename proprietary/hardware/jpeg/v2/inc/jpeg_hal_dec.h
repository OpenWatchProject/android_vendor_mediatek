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

#ifndef __JPEG_HAL_DEC_H__
#define __JPEG_HAL_DEC_H__

#include "img_dec_comp.h"
#include "img_common_def.h"
//#include "jpeg_sw_dec.h"

#ifdef JPEG_HW_DECODE_COMP
#include "jpeg_dec_comp.h"
#endif 

#include <img_mdp_pipe.h>

/*******************************************************************************
*
********************************************************************************/
#ifndef JUINT32
typedef unsigned int JUINT32;
#endif

#ifndef JINT32
typedef int JINT32;
#endif
 
#undef bool
#define bool bool

/*******************************************************************************
* class JpgDecHal
********************************************************************************/
class JpgDecHal {
public:
    JpgDecHal();
    virtual ~JpgDecHal();

    enum ColorFormat {
        kRGB_565_Format,
        kRGB_888_Format,
        kBGR_888_Format,
        kARGB_8888_Format,
        kABGR_8888_Format,
        kYUY2_Pack_Format,           // YUYV
        kUYVY_Pack_Format,           // UYVY        
        
        kYUV_3PLANE_Format,          // I420
        kYVU420_YV12_Format,         // YV12, YVU 420 3 plane
        kYVU420_3PLANE_Format,       // I420
        kYVU420_YV12_ANDROID_Format, // YV12, YVU 420 3 plane
        kYUV_FORMAT_SIZE
    };
    
    enum {
        IMG_RESCALE_TYPE_NONE,        
        IMG_RESCALE_TYPE_FRAME,
        IMG_RESCALE_TYPE_LINK,
        IMG_RESCALE_TYPE_TILE
    };
    
    enum JPEG_DEC_HAL_RESULT {
      JPEG_DEC_HAL_RST_CFG_ERR,
      JPEG_DEC_HAL_RST_DONE,      
      JPEG_DEC_HAL_RST_ROW_DONE,
      JPEG_DEC_HAL_RST_HUFF_ERROR,
      JPEG_DEC_HAL_RST_DECODE_FAIL,
      JPEG_DEC_HAL_RST_BS_UNDERFLOW
      
    };
    
    enum {
      IMG_DEC_TYPE_WEBP,
      IMG_DEC_TYPE_JPEG,
    };

    enum {
      IMG_MEM_TYPE_PHY,
      IMG_MEM_TYPE_PMEM,
      IMG_MEM_TYPE_M4U,
      IMG_MEM_TYPE_ION
      
    };
    
    enum {
      IMG_MEM_DEC_BUF_SCALER,
      IMG_MEM_DEC_BUF_TOP,
    };

    bool setDecType(JUINT32 decType)
    {
        if( decType == IMG_DEC_TYPE_WEBP )
        {
            return false ;//fImgDecType = IMG_DEC_TYPE_WEBP;
        }
        else
        {
            fImgDecType = IMG_DEC_TYPE_JPEG; 
        }
        return true ;
    };
    
    bool setScalerType(JUINT32 scalerType) ;  
                                      
    bool lock();
    bool lockResizer();
    bool unlock();
    bool parse();
    bool start();

    JUINT32 getJpgWidth() { return fJpgWidth; }
    JUINT32 getJpgHeight() { return fJpgHeight; }
    
    void setOutWidth(JUINT32 width) { fOutWidth = width; }
    void setOutHeight(JUINT32 height) { fOutHeight = height; }
    void setOutStride(JUINT32 strideY, JUINT32 strideCbCr)
    {
        fOutStride[0] = strideY;
        fOutStride[1] = fOutStride[2] = strideCbCr;
    }

    void setOutFormat(ColorFormat format) { fOutFormat = format; }
    void setSrcAddr(unsigned char *srcAddr) { fSrcAddr = srcAddr; }
    void setDstAddr(unsigned char *dstAddr) { fDstAddr = dstAddr; }
    void setDstChromaAddr(unsigned char *dstCbAddr, unsigned char *dstCrAddr) { fDstCbAddr = dstCbAddr; 
                                                                                fDstCrAddr = dstCrAddr;}
    void setSrcSize(JUINT32 srcBufSize, JUINT32 srcSize) { fSrcBufferSize = srcBufSize ;
                                                           fSrcBsSize = srcSize ; 
                                                         }  //must be TO_CEIL(fileSize, 128)  + 128 
    void setDstSize(JUINT32 dstSize) { fOutBufSize = dstSize; }
    void setDstChromaSize(JUINT32 dstCbSize, JUINT32 dstCrSize) { fOutCbBufSize = dstCbSize; 
                                                                  fOutCrBufSize = dstCrSize;}
    void setDither(bool doDither) { isDither = doDither; }
    void setRangeDecode(JUINT32 left, JUINT32 top, JUINT32 right, JUINT32 bottom);    
    void setProcHandler(void* h) { fProcHandler = h; }      
    void setSrcFD(JINT32 srcFD) { if(srcFD >=0 ){fIonEn= 1; fSrcFD = srcFD;}
                                 else{fIonEn = 0; fSrcFD = -1;} }
    void setDstFD(JINT32 dstFD) { if(dstFD >=0 ){fIonEn= 1; fDstFD = dstFD;}
                                 else{fIonEn = 0; fDstFD = -1;} }

    JUINT32 setPQlevel(JUINT32 level){
                         fPQlevel = level ;
                         return fPQlevel ; }

    void setPostProcParam(void* pParam, JUINT32 isoSpeed){
                               fPPParam = pParam ;
                               fISOSpeed = isoSpeed; }

    JUINT32 setQualityLevel(JUINT32 level){
                               fQualitylevel = level ;
                               return fQualitylevel ; }

    JUINT32 setPreferDecodeDownSample(JUINT32 enable){
                                               fPreferDecDownSample = enable ;
                                               return fPreferDecDownSample ; }

private:
    bool onStart();
    bool onStartTile();
    bool onStartGdma();
    bool checkParam();
    bool allocIOMEM();
    
    bool allocDecMEM();
    bool freeDecMEM();
    JUINT32 lockScaler(JUINT32 preferType) ;
    bool onConfigComp() ;
    bool onConfigCompTile();
    bool ConfigScalingRatio();
    bool ConfigScalingSrcBuffer();
    
    bool islock;
    bool isDither;
    bool isRangeDecode;
    
    JUINT32 fPQlevel ;
    JUINT32 fQualitylevel ;
    void* fPPParam ;
    JUINT32 fISOSpeed ;
    JUINT32 fDecOutFormat ;
    JUINT32 fScalerInFormat ;
    JUINT32 fCdpDstFormat ;
    void*   fCdpDstBufAddr ;
    
    JUINT32 fUseSwDecoder ;
    JUINT32 fPreferDecDownSample ;
    
    JUINT32 fImgDecType ;
    ImageDecoder* pJpgDecComp ;
    ImgCdpPipe* pImgCdp ;
    
    JUINT32 fScalerBufferType ;
    
    JUINT32 fTileBufRowNum ;
    JUINT32 fTileBufRowMargin ;
    JUINT32 fImgMcuRowNum ;
    JUINT32 fImgMcuHeight[3] ;
    JUINT32 fFirstDecRowNum ;

    JUINT32 fJpgWidth;
    JUINT32 fJpgHeight;
    JUINT32 fOutWidth;
    JUINT32 fOutHeight;
    JUINT32 fOutStride[3];

    JUINT32 fLeft;
    JUINT32 fRight;
    JUINT32 fTop;
    JUINT32 fBottom;
    
    ColorFormat fOutFormat;
    
    JUINT32 fScaleType ;

    unsigned char *fSrcAddr;
    unsigned char *fDstAddr;
    unsigned char *fDstCbAddr;
    unsigned char *fDstCrAddr;

    JUINT32 fSrcBufferSize;    
    JUINT32 fSrcBsSize;    
    JUINT32 fOutBufSize;
    JUINT32 fOutCbBufSize;
    JUINT32 fOutCrBufSize;

    JUINT32 fDecDstImgWidth[3] ; 
    JUINT32 fDecDstImgHeight[3] ;
    JUINT32 fDecDstBufStride[3] ;  
    
    JUINT32 fDecDstBufSize[3] ;    
    
    unsigned char *fDecDstBufAddrVA0[3];
    unsigned char *fDecDstBufAddrVA1[3];
    unsigned char *fDecDstBufAddrVA0_org[3];
    unsigned char *fDecDstBufAddrVA1_org[3];
    
    JUINT32 fUseImgRszCmodel ;

    void* fProcHandler;
    
    JUINT32 fMemType;
    JUINT32 fMemTypeDefault;    
    JUINT32 fDecBRZ ;
    JUINT32 fTotalBRZ ;
    
    bool fIonEn;
    JINT32 fSrcFD ;
    JINT32 fDstFD ;
};

#endif 
