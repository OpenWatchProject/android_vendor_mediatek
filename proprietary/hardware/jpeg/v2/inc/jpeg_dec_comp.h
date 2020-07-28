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

#ifndef __JPEG_HAL_COMP_H__
#define __JPEG_HAL_COMP_H__

#include <fcntl.h>
#include <sys/mman.h>

#ifdef MTK_M4U_SUPPORT
#include "m4u_lib.h"
#endif

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
* class JpgDecComp
********************************************************************************/

class JpgDecComp : public ImageDecoder{
public:
    JpgDecComp();
    virtual ~JpgDecComp();

//    enum ColorFormat {
        
//        IMGDEC_3PLANE_YUV420,       
//        IMGDEC_3PLANE_YUV422
        
//    }; 
    
//    enum JPEG_DEC_RESULT {
//      JPEG_DEC_RST_CFG_ERR,
//      JPEG_DEC_RST_DONE,      
//      JPEG_DEC_RST_ROW_DONE,
//      JPEG_DEC_RST_HUFF_ERROR,
//      JPEG_DEC_RST_DECODE_FAIL,
//      JPEG_DEC_RST_BS_UNDERFLOW
//      
//    };
    
//    enum JPEG_DEC_COMP_MODE
//    {
//        JPEG_DEC_COMP_NONE    ,
//        JPEG_DEC_COMP_FRAME   ,                   ///< Jpeg baseline picture, decode whole picture
//        JPEG_DEC_COMP_LINK ,                   ///< Jpeg baseline picture, decode whole picture
//        JPEG_DEC_COMP_MCU_ROW                     ///< Jpeg baseline picture, decode MCU row
//    } ;

    bool lock();
    bool unlock();
    bool parse();

    //bool Start();
    //bool ResumeDecRow( JUINT32 decRowNum);
    JUINT32 StartConfig() {return true;}
    JUINT32 Start();
    JUINT32 ResumeDecRow( JUINT32 decRowNum);    
    bool Config(JUINT32 mode);
    
    JUINT32 getOutFormat(){ return fOutFormat; }

    JUINT32 getJpgWidth() { return fJpgWidth; }
    JUINT32 getJpgHeight() { return fJpgHeight; }
    JUINT32 getMcuRow() { return fMcuRow; }
    JUINT32 getMcuInRow() { return fMcuInRow; }
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
                                                         if(fSrcBsSize> fSrcBufferSize) fSrcBsSize = fSrcBufferSize ;}  //must be TO_CEIL(fileSize, 128)  + 128 

    // out VA
    void setDstAddr0(unsigned char *dstAddr, unsigned char *dstAddr1, unsigned char *dstAddr2) 
                    { fDecOutBufAddr0[0] = dstAddr; fDecOutBufAddr0[1] = dstAddr1; fDecOutBufAddr0[2] = dstAddr2;
                      fDstAddr = dstAddr; }
    
    void setDstAddr1(unsigned char *dstAddr, unsigned char *dstAddr1, unsigned char *dstAddr2) 
                    { fDecOutBufAddr1[0] = dstAddr; fDecOutBufAddr1[1] = dstAddr1; fDecOutBufAddr1[2] = dstAddr2;}

    void setIonMode(bool ionEn) { if( ionEn ) fMemType = IMG_MEM_TYPE_ION; 
                                         else fMemType = fMemTypeDefault ;      }
    
    void setSrcFD( JINT32 srcFD ) { fSrcFD = srcFD; }
    
    void setDstFD( JINT32 dstFD ) { fDstFD = dstFD; }
    
    //current NOUSE
    void setDstBufStride( JUINT32 dstStride, JUINT32 dstStride1){ fDstBufStride[0] = dstStride; fDstBufStride[1] = fDstBufStride[2] = dstStride1 ;}

    /* make sure output valid buffer size */
    void setDstBufSize(JUINT32 dstSize, JUINT32 dstSize1) 
                     { fDecDstBufSize[0] = dstSize; fDecDstBufSize[1] = fDecDstBufSize[2] =  dstSize1; }
    // set additional brz actor, should check if valid factor
    void setBRZ(JUINT32 brz){ 
      if(brz>3) brz = 3;
      fBRZ = brz; }

    void setTileBufNum(JUINT32 ringBufRowNum){ fDstTileBufRowNum = ringBufRowNum ;}
    bool setFirstDecRow(JUINT32 rowNum){fFirstDecRowNum = rowNum ; return true ;}
    
    //JPEG_DEC_RESULT JPEG_PMEM_ALLOC( unsigned char *mva, unsigned int *pa, int *FD, unsigned int size, unsigned int lineNo);
    
    bool setCmdqMode(JUINT32 enable){  fCmdqMode = enable;  return true; }
    
    bool getCmdqMode(void){  return (fCmdqMode>0)? true : false; }

private:
    bool onStart();
    bool checkParam();
    bool islock;
    bool alloc_pmem();
    bool alloc_m4u();
    bool free_m4u();
    bool alloc_ion();
    bool free_ion();
    
    //bool isDither;
    //bool isRangeDecode;
    
    JUINT32 fDecType;
    
    JUINT32 fJpgWidth;
    JUINT32 fJpgHeight;
    JUINT32 fOutWidth;
    JUINT32 fOutHeight;
    
    JUINT32 fBRZ;
    JUINT32 fMinLumaBufSize;
    JUINT32 fMinCbCrBufSize;

    JUINT32 fMinLumaBufStride;
    JUINT32 fMinCbCrBufStride;
    
    JUINT32 fLumaImgStride;
    JUINT32 fCbCrImgStride;
    JUINT32 fLumaImgHeight;
    JUINT32 fCbCrImgHeight;
    
    JUINT32 fFirstDecRowNum;
    JUINT32 fDecRowCnt;
    
    JUINT32 fDstBufStride[3];
    
    JUINT32 fOutFormat;

    unsigned char *fSrcAddr;
    unsigned int fSrcAddrPA;
    
    unsigned char *fDstAddr;

    JUINT32 fSrcBufferSize;    
    JUINT32 fSrcBsSize;    

    JUINT32 fSrcConfigAddr;
    JUINT32 fDstConfigAddr;

    JUINT32 fDecDstBufAddr0[3]; //set HW
    JUINT32 fDecDstBufAddr1[3]; //set HW   
    JUINT32 fDecDstBufSize[3];    
    JUINT32 fDstTileBufRowNum;
    unsigned char *fDecOutBufAddr0[3];
    unsigned char *fDecOutBufAddr1[3];    

    //for tile
    JUINT32 fMcuRow;
    JUINT32 fMcuInRow;
    JUINT32 fMcuHeightY;
    JUINT32 fMcuHeightC;
    
    JUINT32 fMemType;
    JUINT32 fMemTypeDefault;

    // m4u
#ifdef MTK_M4U_SUPPORT
    MTKM4UDrv *pM4uDrv;
#endif
    JUINT32 fDecSrcM4UAddrMva;
    JUINT32 fDecDstM4UAddrMva0[3];
    JUINT32 fDecDstM4UAddrMva1[3];

    // ion
    JINT32 fIonDevFD;
    JINT32 fSrcFD;
    JINT32 fDstFD;
    JUINT32 fDecSrcIonPA;
    JUINT32 fDecDstIonPA[3];
    JINT32 fSrcIonHdle;
    JINT32 fDstIonHdle;

    JUINT32 fCmdqMode;
    
    struct timeval t1, t2, t3;    
    
    int decID;
};

#endif 
