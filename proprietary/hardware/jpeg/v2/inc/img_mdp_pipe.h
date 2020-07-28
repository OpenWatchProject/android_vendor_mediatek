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

#ifndef __IMG_MDP_PIPE_H__
#define __IMG_MDP_PIPE_H__

//#include <cutils/pmem.h>
//#include "uvvf.h"//test

 

//#define TILE_ROW_SIZE  (10)
#define TILE_ROW_SIZE  (128)
#define TILE_MAX_ROW_IRQ_SIZE (30)
/*******************************************************************************
*
********************************************************************************/
#ifndef JUINT32
typedef unsigned int JUINT32;
#endif

#ifndef JINT32
typedef int JINT32;
#endif
 

#include "img_common_def.h"

#undef bool
#define bool bool

//#define CDP_TYPE NSImageio::NSIspio::

#define SCALER_LINK_MAX_WIDTH (4095)

#include <DpDataType.h>
#include <DpFragStream.h>

class DpStream;
class DpChannel;
class DpRingBufferPool;
class DpBasicBufferPool;


/*******************************************************************************
* class ImgCdpPipe
********************************************************************************/
class ImgCdpPipe {
   
public:
    ImgCdpPipe();
    virtual ~ImgCdpPipe();

    enum {
        SCALER_IN_NONE
       ,SCALER_IN_1P_GRAY
       ,SCALER_IN_3P_YUV420
       ,SCALER_IN_3P_YUV422
       ,SCALER_IN_1P_ARGB8888
       ,SCALER_IN_1P_RGB565
       ,SCALER_IN_1P_RGB888
       ,SCALER_IN_3P_YUV444
    };

    enum {
        SCALER_OUT_NONE
       ,SCALER_OUT_1P_ARGB8888
       ,SCALER_OUT_1P_RGB565
       ,SCALER_OUT_1P_RGB888
       ,SCALER_OUT_1P_YUV422
       ,SCALER_OUT_3P_YVU420  //YV12
       ,SCALER_OUT_3P_YV12  //YV12
       ,SCALER_OUT_3P_YV12_ANDROID  //YV12
    };   

    enum {
        SCALER_MODE_NONE 
       ,SCALER_MODE_FRAME
       ,SCALER_MODE_LINK
       ,SCALER_MODE_TILE
       ,SCALER_MODE_SIZE
    }; 

    enum {
        SCALER_GDMA_SRC_FMT
       ,SCALER_GDMA_SRC_JPEG
    };    

    enum SCALER_PIPE_RESULT {
      SCALER_PIPE_RST_CFG_ERR,
      SCALER_PIPE_RST_DONE,      
      SCALER_PIPE_RST_ROW_DONE,
      SCALER_PIPE_RST_HUFF_ERROR,
      SCALER_PIPE_RST_DECODE_FAIL,
      SCALER_PIPE_RST_BS_UNDERFLOW
    };

    enum {
      IMG_MEM_TYPE_PHY,
      IMG_MEM_TYPE_PMEM,
      IMG_MEM_TYPE_M4U,
      IMG_MEM_TYPE_ION
    };

    bool lock(JUINT32);
    bool unlock();
    bool onFrameStart();
    bool NStart() ;
    bool Start();
    bool Wait();
    bool setConnectType(JUINT32 mode); 

    JUINT32 getScalerOpMode(){ return fScalerMode; }

    bool setDstImageSize(JUINT32 dstWidth, JUINT32 dstHeight){ fOutWidth = dstWidth; fOutHeight = dstHeight; return true;};
    bool setDstImageColor(JUINT32 dstFormat){ fDstFormat = dstFormat; return true; }

    bool setDstBufSize(JUINT32 size, JUINT32 size1, JUINT32 size2){ fDstBufSize = size; 
                                                                       fDstBufSize1 = size1;  
                                                                       fDstBufSize2 = size2; 
                                                                       return true; }

    void setDstAddr(void* dstAddr, void* dstAddr1, void* dstAddr2) { fDstBufAddr = dstAddr;  
                                                                        fDstBufAddr1 = dstAddr1;  
                                                                        fDstBufAddr2 = dstAddr2; }   
   
    bool setDstBufStride(JUINT32 stride_Y, JUINT32 stride_C) { 
                            fDstBufStride[0] = stride_Y ;  
                            fDstBufStride[1] = fDstBufStride[2] = stride_C ;
                            return true; }

    /*******************************************************/
    //                  TILE API
    /*******************************************************/
    //JUINT32 getCurTileStart(){ return fCurRowStart; }
    //JUINT32 getCurTileEnd(){ return fCurRowEnd; }
    //JUINT32 getNxtTileEnd(){ return fNxtRowEnd; }

    JINT32 getTileStart(JUINT32 RunCnt){ 
        if(RunCnt == fTileRunCnt)
            return fDecRowStart ;
        else if(RunCnt == fTileRunCnt-1)
            return fPrvDecRowStart ;
        else
            return -1;
    }

    JINT32 getTileEnd(JUINT32 RunCnt){ 
        if(RunCnt == fTileRunCnt)
            return fDecRowEnd ;
        else if(RunCnt == fTileRunCnt-1)
            return fPrvDecRowEnd ;
        else
            return -1;      
    }

    JUINT32 isScalingFinish(){ return fIsScalerEnd; }

    /*******************************************************/
    //                  API
    /*******************************************************/
    bool setSrcRowHeight(JUINT32 height_Y, JUINT32 height_C) { fRowHeight[0] = height_Y; fRowHeight[2] = fRowHeight[1] = height_C; return true ; }

    bool setSrcImageColor(JUINT32 srcFormat){ fSrcFormat = srcFormat; return true;};

    bool setSrcImageSize(JUINT32 width_Y, JUINT32 height_Y, JUINT32 width_C, JUINT32 height_C){ 
                             fSrcImgWidth[0] = width_Y ;
                             fSrcImgHeight[0] = height_Y ;
                             fSrcImgWidth[1] =  fSrcImgWidth[2] = width_C ;
                             fSrcImgHeight[1] = fSrcImgHeight[2] = height_C ; 
                             return true; };

    bool setCropImageSize( JUINT32 src_width, JUINT32 src_height ){
                                fCropInWidth  = src_width ;
                                fCropInHeight = src_height ;
                                return true; };

    //bool setSrcBufSize(JUINT32 size_Y, JUINT32 size_U){ fSrcBufSize[0] = size_Y; fSrcBufSize[1] = fSrcBufSize[2] = size_U; return true;}
                             
    bool setSrcBufStride(JUINT32 stride_Y, JUINT32 stride_C) { 
                            fSrcBufStride[0] = stride_Y ;  
                            fSrcBufStride[1] = fSrcBufStride[2] = stride_C ;
                            return true; }

    void setSrcAddr0(void* srcAddr, void* srcAddr1, void* srcAddr2) {
                        fSrcBufAddr0[0] = srcAddr; 
                        fSrcBufAddr0[1] = srcAddr1; 
                        fSrcBufAddr0[2] = srcAddr2; }

    void setSrcAddr1(void* srcAddr, void* srcAddr1, void* srcAddr2) {
                        fSrcBufAddr1[0] = srcAddr; 
                        fSrcBufAddr1[1] = srcAddr1; 
                        fSrcBufAddr1[2] = srcAddr2; }

    void setDither(JUINT32 ditherEn){ isDither = ditherEn; }
    bool setTdsp(JUINT32 tdspEn){ fTdsFlag = tdspEn; return true ;}
    bool setPostProcParam(void* pParam, JUINT32 isoSpeed){ fPPParam = pParam; fISOSpeed = isoSpeed; return true ;}
    void setCmodel(JUINT32 en_c_model){ fIsUseCmodel = en_c_model; }

    bool setTileBufRowNum(JUINT32 row_no){ fTileBufMcuRow = row_no; return true;  }
    bool setSrcImgMcuNum(JUINT32 mcu_row){fSrcImgMcuRow = mcu_row; return true; }

    void setDstFD( JINT32 dstFD)
    {
        if( dstFD >= 0 )
        {
            fMemType = IMG_MEM_TYPE_ION; 
            fDstFD = dstFD ;
        }
        else
        { 
            fMemType = fMemTypeDefault ;      
            fDstFD = -1 ;
        }
    }
                                  
    JINT32 TileInit();
    bool getSrcBuffer(void **pSrcBuf, JINT32 *pSrcBufFD, JUINT32 *pSrcBufStride, JUINT32 *tBufRowNum );

    bool setRszCmdqMode(JUINT32 enable){  fIsCmdqMode = enable;  return true; }

    bool setMcuInRow(JUINT32 mcuInRow){  fMcuInRow = mcuInRow;  return true; }

    //bool setSrcBufRowSize(JUINT32 size_Y, JUINT32 size_U){ fSrcBufRowSize[0] = size_Y; fSrcBufRowSize[1] = fSrcBufRowSize[2] = size_U; return true;}

    bool setFrameLevelCMDQ(JUINT32 enable){ fFrameLevelCMDQ = enable ;  return true ; }

private:    

typedef struct {
   
   unsigned int start_cnt;    // trigget_cdp_cnt
   unsigned int row_start;    //input row base index
   unsigned int row_end;      //input row limit index
   unsigned int entry_start;  //point to conf entry index   
   unsigned int entry_nxt_start;    //point to next conf entry index
   unsigned int irq_per_row ;       //irq per row
   unsigned int start_oft[TILE_MAX_ROW_IRQ_SIZE];    // offset for HW setting
   
} TILE_ROW_DATA ;

    bool onSwStart();
    bool onTileStart();
    bool onGdmaStart();
    bool checkParam();
    bool allocIOMEM();    
    bool freeIOMEM();
    bool configGDMA();
    bool alloc_m4u();
    bool free_m4u();
    bool onCdpWaitIrq() ;
    

    int configCdpIO();
    int onHWScalerCreate(JUINT32 mode);
    int onHWScalerDeInit();
    int onTileConfig();
    int onTileRun(JUINT32 offset);
    bool findNxtTileWindow(JUINT32 entry_start, JUINT32 run_cnt, TILE_ROW_DATA *row_data );
    
private:
   
    DP_STATUS_ENUM fDpStatus;
    JUINT32 fStartFragStreamFlag ;
    DpFragStream *pStream;   
    JUINT32 fIsScalerEnd ;
    JUINT32 fDecRowStart ;
    JUINT32 fDecRowEnd   ;
    JUINT32 fDecRowNum   ;
    JUINT32 fPrvDecRowStart ;
    JUINT32 fPrvDecRowEnd   ;
    JUINT32 fPrvDecRowNum   ;
    JINT32 fDqBufID ;
    JINT32 fMcuSampleWidth[3];

    bool waitDecodeBuffer(JINT32 *dqBufID, JINT32 *dqBufFD, JUINT32 *dqBufMcuStart, JUINT32 *dqBufMcuEnd, JUINT32 *dqBufMcuNum, JUINT32 *isFinish);
    bool onCdpGo();
    bool lockScaler() ;
    bool unlockScaler(unsigned int isUnlockAll);
    
    bool islock;
    bool isDither;
    
    JUINT32 fFrameLevelCMDQ ;
    
    JUINT32 fCropInWidth  ;
    JUINT32 fCropInHeight ;
    
    JUINT32 fLockCnt ;
    
    JUINT32 fMemType ;
    JUINT32 fMemTypeDefault ;

    JUINT32 fScalerMode ; 
    void*   fDstBufAddr ;
    void*   fDstBufAddr1 ;
    void*   fDstBufAddr2 ;
    JUINT32 fDstBufSize;
    JUINT32 fDstBufSize1;
    JUINT32 fDstBufSize2;
    
    JUINT32 fOutWidth;
    JUINT32 fOutHeight;
    
    JUINT32 fSrcFormat ;
    JUINT32 fDstFormat ;
    
    /*Tile Config setting */
    JUINT32 fCurRowStart ;
    JUINT32 fCurRowEnd ;
    JUINT32 fNxtRowStart ;
    JUINT32 fNxtRowEnd ;
    JUINT32 fTileRunCnt;
    JUINT32 fTileBufMcuRow ; 
    JUINT32 fSrcImgMcuRow ;
    JUINT32 fMcuInRow ;

    void*   fSrcBufAddr0[3];
    void*   fSrcBufAddr1[3];
        
    JUINT32 fSrcImgWidth[3];
    JUINT32 fSrcImgHeight[3];
    JUINT32 fSrcBufStride[3];
    JUINT32 fDstBufStride[3];

    void*   fDstConfigAddr;
    JUINT32 fDstConfigAddrPA;
    
    JUINT32 fIsUseCmodel ;
    JUINT32 fIsCmdqMode ;
    
    JUINT32 fRowHeight[3] ;

    /***************************************************/
    //                  PQ param 
    /***************************************************/
    JUINT32 fTdsFlag ;
    void* fPPParam ;
    JUINT32 fISOSpeed;

    //ION
    JINT32 fDstFD ;
    
};


#endif 
