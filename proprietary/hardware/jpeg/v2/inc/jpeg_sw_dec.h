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

#ifndef __JPEG_SW_DEC_H__
#define __JPEG_SW_DEC_H__



#include "SwType.h"
#include "SwStream.h"
#include "SwJpegUtility.h"

extern "C" {
    #include "jpeglib_alpha.h"
    #include "jerror_alpha.h"
}


#include "img_dec_comp.h"

#include "img_common_def.h"


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
* class JpgSwDec
********************************************************************************/


class JpgSwDec : public ImageDecoder{
public:
    JpgSwDec();
    virtual ~JpgSwDec();

//    enum ColorFormat {
//        
//        IMGDEC_3PLANE_YUV420,       
//        IMGDEC_3PLANE_YUV422
//        
//    }; 
    

    
    
    bool lock();
    bool unlock();
    bool parse();
    
    JUINT32 StartConfig() {return true;}
    JUINT32 Start();
    JUINT32 ResumeDecRow( JUINT32 decRowNum);
    bool Config(JUINT32 mode);
    
    
    JUINT32 getOutFormat(){ return fOutFormat; }

    JUINT32 getJpgWidth() { return fJpgWidth; }
    JUINT32 getJpgHeight() { return fJpgHeight; }
    JUINT32 getMcuRow() { return fMcuRow; }
    JUINT32 getMcuHeightY() { return fMcuHeightY; }
    JUINT32 getMcuHeightC() { return fMcuHeightC; }
    
    
    JUINT32 getMinLumaBufSize(){ return fMinLumaBufSize ;  }
    JUINT32 getMinCbCrBufSize(){ return fMinCbCrBufSize ;  }

    JUINT32 getMinLumaBufStride(){ return fMinLumaBufStride ;  }
    JUINT32 getMinCbCrBufStride(){ return fMinCbCrBufStride ;  }    

    JUINT32 getLumaImgStride(){ return fLumaImgStride ;  }
    JUINT32 getCbCrImgStride(){ return fCbCrImgStride ;  }  
 
    JUINT32 getLumaImgHeight(){ return fLumaImgHeight ; } 
    JUINT32 getCbCrImgHeight(){ return fCbCrImgHeight ; } 
    
    // in VA
    void setSrcAddr  (unsigned char * srcAddr) { fSrcAddr = srcAddr ;}

    void setSrcSize(JUINT32 srcBufSize, JUINT32 srcSize) { fSrcBufferSize = srcBufSize ;
                                                         fSrcBsSize = TO_CEIL(srcSize, 128) + 128 ; 
                                                         if(fSrcBsSize> fSrcBufferSize) fSrcBsSize = fSrcBufferSize ; }  //must be TO_CEIL(fileSize, 128)  + 128 

    // out VA
    void setDstAddr0(unsigned char *dstAddr, unsigned char *dstAddr1, unsigned char *dstAddr2) 
                    { fDecOutBufAddr0[0] = dstAddr; fDecOutBufAddr0[1] = dstAddr1; fDecOutBufAddr0[2] = dstAddr2;
                      fDstAddr = dstAddr; 
                      }
    
    void setDstAddr1(unsigned char *dstAddr, unsigned char *dstAddr1, unsigned char *dstAddr2) 
                    { fDecOutBufAddr1[0] = dstAddr; fDecOutBufAddr1[1] = dstAddr1; fDecOutBufAddr1[2] = dstAddr2;}

    //current NOUSE
    void setDstBufStride( JUINT32 dstStride, JUINT32 dstStride1){ fDstBufStride[0] = dstStride; fDstBufStride[1] = fDstBufStride[2] = dstStride1 ;}

    /* make sure output valid buffer size */
    void setDstBufSize(JUINT32 dstSize, JUINT32 dstSize1) 
                     { fDecDstBufSize[0] = dstSize; fDecDstBufSize[1] = fDecDstBufSize[2] =  dstSize1; }
    // set additional brz actor, should check if valid factor
    void setBRZ(JUINT32 brz){ fBRZ = brz; }
    void setTileBufNum(JUINT32 ringBufRowNum){ fDstTileBufRowNum = ringBufRowNum ;}
    bool setFirstDecRow(JUINT32 rowNum){fFirstDecRowNum = rowNum ; return true ;}
    
    bool setQuality(JUINT32 level){ fQuality = level ; 
                                    return true; }
    
    //JPEG_DEC_RESULT JPEG_PMEM_ALLOC( unsigned char *mva, unsigned int *pa, int *FD, unsigned int size, unsigned int lineNo);

    
private:
    bool onStart();
    bool checkParam();
    bool alloc_pmem();
    bool alloc_m4u();
    bool free_m4u();
    bool create();

    bool islock;
    JUINT32 fDecType ;
    
    JUINT32 fJpgWidth;
    JUINT32 fJpgHeight;
    JUINT32 fOutWidth;
    JUINT32 fOutHeight;
    
    JUINT32 fBRZ ;
    JUINT32 fMinLumaBufSize ;
    JUINT32 fMinCbCrBufSize ;

    JUINT32 fMinLumaBufStride ;
    JUINT32 fMinCbCrBufStride ;
    
    JUINT32 fLumaImgStride ;
    JUINT32 fCbCrImgStride ;
    JUINT32 fLumaImgHeight ;
    JUINT32 fCbCrImgHeight ;
    
    JUINT32 fFirstDecRowNum ;
    JUINT32 fDecRowCnt ;
    
    JUINT32 fDstBufStride[3];
    
    JUINT32 fOutFormat;

    unsigned char *fSrcAddr;
    unsigned char *fDstAddr;

    JUINT32 fSrcBufferSize;    
    JUINT32 fSrcBsSize ;

    JUINT32 fDstTileBufRowNum ;
    unsigned char *fDecOutBufAddr0[3] ;
    unsigned char *fDecOutBufAddr1[3] ;
    JUINT32 fDecDstBufSize[3];    

    //for tile
    JUINT32 fMcuRow ;
    JUINT32 fMcuHeightY ;
    JUINT32 fMcuHeightC ;

    JUINT32 fMemType;
    JUINT32 fMemTypeDefault;

    /* libjpeg sw codec parameter */
    jpeg_decompress_struct_ALPHA  *cinfo;
    swjpeg_error_mgr sw_err;
    swjpeg_source_mgr *jpegStream;
    SwStream* srcStream  ;
    jpeg_error_mgr_ALPHA jerr;

    JUINT32 fQuality ;
    unsigned char *rowptr ;
    INT32_ALPHA bpr ;
    JUINT32 fImgFormat;
};

#endif
