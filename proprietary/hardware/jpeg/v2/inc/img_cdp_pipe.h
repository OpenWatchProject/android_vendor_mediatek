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

#ifndef __IMG_CDP_PIPE_H__
#define __IMG_CDP_PIPE_H__

#include "m4u_lib.h"

#include "IPipe.h"
#include "ICamIOPipe.h"
#include "ICdpPipe.h"
#include "IPostProcPipe.h"
//
#include <cutils/pmem.h>
#include "isp_drv.h"
#include "sensor_hal.h"
//#include "uvvf.h"//test

#include "res_mgr_drv.h"
#include "pipe_mgr_drv.h"
 

//using NSImageio::NSIspio::ICdpPipe;

using namespace NSImageio ;
using namespace NSImageio::NSIspio;

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


/*******************************************************************************
* class ImgCdpPipe
********************************************************************************/
class ImgCdpPipe {
   
public:
   ImgCdpPipe();
   virtual ~ImgCdpPipe();
   
   
   typedef struct{
      JUINT32 x;
      JUINT32 y;
      JUINT32 w;
      JUINT32 h;
   }CDP_WIN;
   
   enum {
       SCALER_IN_NONE
      ,SCALER_IN_1P_GRAY
      ,SCALER_IN_3P_YUV420
      ,SCALER_IN_3P_YUV422
      ,SCALER_IN_1P_ARGB8888
      ,SCALER_IN_1P_RGB565
      ,SCALER_IN_1P_RGB888
   };
   
   enum {
       SCALER_OUT_NONE
      ,SCALER_OUT_1P_ARGB8888
      ,SCALER_OUT_1P_RGB565
      ,SCALER_OUT_1P_RGB888
      ,SCALER_OUT_1P_YUV422
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
   void * pt_TileStart(void *data) ;
   bool NStart() ;
   bool Start();
   bool Wait();
   bool setConnectType(JUINT32 mode); 
   
   JUINT32 getScalerOpMode(){ return fScalerMode ; };
   
   bool setDstImageSize(JUINT32 dstWidth, JUINT32 dstHeight){ fOutWidth = dstWidth; fOutHeight = dstHeight; return true;};
   bool setDstImageColor(JUINT32 dstFormat){ fDstFormat = dstFormat; return true;};
   
   bool setDstBufSize(JUINT32 size){ fDstBufSize = size; return true;}


    void setDstAddr(JUINT32 dstAddr) 
                    { fDstBufAddr = dstAddr;  
                      }   
    // top always send skia malloc memory, so no PA
    //void setDstAddrPA(JUINT32 dstAddrPA) 
    //                { fDstBufAddrPA = dstAddrPA;  
    //                  }                      
   
   /*******************************************************/
   //                  TILE API
   /*******************************************************/
   JUINT32 getCurTileStart(){ return fCurRowStart; }
   JUINT32 getCurTileEnd(){ return fCurRowEnd; }

   JUINT32 getNxtTileEnd(){ return fNxtRowEnd; }
   
   JINT32 getTileStart(JUINT32 RunCnt){ 
      if(RunCnt <= fTileTotalRun && tRowCfg[RunCnt%TILE_ROW_SIZE].start_cnt == RunCnt)
         return tRowCfg[RunCnt%TILE_ROW_SIZE].row_start ;
      else return -1 ; }

   JINT32 getTileEnd(JUINT32 RunCnt){ 
      if(RunCnt <= fTileTotalRun && tRowCfg[RunCnt%TILE_ROW_SIZE].start_cnt == RunCnt)
         return tRowCfg[RunCnt%TILE_ROW_SIZE].row_end ;
      else return -1 ; }
   
   
   JUINT32 getTileTotalRun(){ return fTileTotalRun; }
   

   /*******************************************************/
   //                  API
   /*******************************************************/
   bool  setSrcRowHeight(JUINT32 height_Y, JUINT32 height_C) { fRowHeight[0] = height_Y; fRowHeight[2] = fRowHeight[1] = height_C; return true ; }
   void  setGtrSrcIsJpeg(JUINT32 isJpeg) {  fGtrIsJpeg = isJpeg;}

   
   
   bool setSrcImageColor(JUINT32 srcFormat){ fSrcFormat = srcFormat; return true;};
   
   bool setSrcImageSize(JUINT32 width_Y, JUINT32 height_Y, JUINT32 width_C, JUINT32 height_C){ 
                            fSrcImgWidth[0] = width_Y ;
                           fSrcImgHeight[0] = height_Y ;
                            fSrcImgWidth[1] =  fSrcImgWidth[2] = width_C ;
                           fSrcImgHeight[1] = fSrcImgHeight[2] = height_C ; 
                           return true;
                            };
                            
   bool setCropImageSize( JUINT32 src_width, JUINT32 src_height ){
                               fCropInWidth  = src_width ;
                               fCropInHeight = src_height ;
                               return true ;
                             };
                            
   bool setSrcBufSize(JUINT32 size_Y, JUINT32 size_U){ fSrcBufSize[0] = size_Y; fSrcBufSize[1] = fSrcBufSize[2] = size_U; return true;}
                            
   bool setSrcBufStride(JUINT32 stride_Y, JUINT32 stride_C) { 
                              fSrcBufStride[0] = stride_Y ;  
                              fSrcBufStride[1] = fSrcBufStride[2] = stride_C ;
                              return true;
                           };
                              
    void setSrcAddr0(JUINT32 srcAddr, JUINT32 srcAddr1, JUINT32 srcAddr2) 
                    { fSrcBufAddr0[0] = srcAddr; 
                      fSrcBufAddr0[1] = srcAddr1; 
                      fSrcBufAddr0[2] = srcAddr2; 
                      }
                              
    void setSrcAddr1(JUINT32 srcAddr, JUINT32 srcAddr1, JUINT32 srcAddr2) 
                    { fSrcBufAddr1[0] = srcAddr; 
                      fSrcBufAddr1[1] = srcAddr1; 
                      fSrcBufAddr1[2] = srcAddr2;
                      }

    void setSrcAddrPA0(JUINT32 srcAddrPA, JUINT32 srcAddrPA1, JUINT32 srcAddrPA2) 
                    { fSrcBufAddrPA0[0] = srcAddrPA; 
                      fSrcBufAddrPA0[1] = srcAddrPA1; 
                      fSrcBufAddrPA0[2] = srcAddrPA2; 
                      fMemType = IMG_MEM_TYPE_PHY ;
                      }
                              
    void setSrcAddrPA1(JUINT32 srcAddrPA, JUINT32 srcAddrPA1, JUINT32 srcAddrPA2) 
                    { fSrcBufAddrPA1[0] = srcAddrPA; 
                      fSrcBufAddrPA1[1] = srcAddrPA1; 
                      fSrcBufAddrPA1[2] = srcAddrPA2;
                      }

    
    bool setOutCrop(JUINT32 win_x, JUINT32 win_y, JUINT32 win_w, JUINT32 win_h ){ 
                     dstWin.x = win_x ;
                     dstWin.y = win_y ;
                     dstWin.w = win_w ;
                     dstWin.h = win_h ;
                     return true;
                     };
                     
    void setDither(JUINT32 ditherEn){ isDither = ditherEn; }
    void setCmodel(JUINT32 en_c_model){ fIsUseCmodel = en_c_model; }
    
    bool setTileBufRowNum(JUINT32 row_no){ fTileBufMcuRow = row_no; return true;  }
    bool setSrcImgMcuNum(JUINT32 mcu_row){fSrcImgMcuRow = mcu_row; return true; }
    
    void setDstFD( JINT32 dstFD) { if( dstFD >= 0 ){ 
                                     fMemType = IMG_MEM_TYPE_M4U; 
                                     fIonEn = 1;
                                     fDstFD = dstFD ;
                                   }else{ 
                                     fMemType = fMemTypeDefault ;      
                                     fIonEn = 0;
                                     fDstFD = -1 ;
                                   }
                                  }
                                  
   void setDstHandler(unsigned long handle){ fDstHandle = handle ; }
                                         
    JINT32 TileInit();
    
    
  
                                
    
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




typedef struct {
   
   JUINT32 tSrcImgWidth[3];
   JUINT32 tSrcImgHeight[3];
   JUINT32 tSrcBufStride[3];
   JUINT32 tSrcBufTotalSize[3];
   JUINT32 tSrcBufRingSize[3];
   JUINT32 tRingBufRowNum ;
   JUINT32 tRowHeight ;
   JUINT32 tSrcBufVA[3];
   JUINT32 tSrcBufPA[3];
   
   JUINT32 tDstImgWidth;
   JUINT32 tDstImgHeight;
   JUINT32 tDstBufSize;
   JUINT32 tDstBufVA;
   JUINT32 tDstBufPA;

   JUINT32 tCfgBufVA;
   JUINT32 tCfgEntryNum;
   JUINT32 tCfgTotalRun;
      
   //ETileMessage tConfigResult;
   ETpipeMessage tConfigResult;

} CONFIG_RESIZER_DATA ;

   
   
    
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
   
    bool onCdpGo();
    bool lockScaler() ;
    bool unlockScaler(unsigned int isUnlockAll);
    
    bool islock;
    bool isDither;
    
    JUINT32 fCropInWidth  ;
    JUINT32 fCropInHeight ;
    
    JUINT32 fResetISP ;
    
    JUINT32 fLockCnt ;
    
    
    JUINT32 fSrcPlaneNum ;
    
    
    JUINT32 fMemType ;
    JUINT32 fMemTypeDefault ;

    M4U_MODULE_ID_ENUM fm4uGdmaID ; 
    

    JUINT32 fScalerMode ; 
    JUINT32 fPreferMode ; 
    JUINT32 fDstBufAddr ;
    JUINT32 fDstBufAddrPA ;
    JUINT32 fDstBufSize;
    JUINT32 fSrcBufSize[3] ;
    
    CDP_WIN srcWin;
    CDP_WIN dstWin;

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
    JUINT32 fTileGoCntInRow ;
    JUINT32 fTileBufMcuRow ; 
    JUINT32 fSrcImgMcuRow ;

    JUINT32 fLeft;
    JUINT32 fRight;
    JUINT32 fTop;
    JUINT32 fBottom;


    JUINT32 fSrcBufAddr0[3];
    JUINT32 fSrcBufAddr1[3];
    
    JUINT32 fSrcBufAddrPA0[3];
    JUINT32 fSrcBufAddrPA1[3];    
    
    JUINT32 fSrcImgWidth[3];
    JUINT32 fSrcImgHeight[3];
    
    JUINT32 fSrcBufStride[3];
    

    JUINT32 fDstConfigAddr;
    JUINT32 fDstConfigAddrPA;
    
    JUINT32 fIsUseCmodel ;
    
    
    
    /********************************/
    /*         config GDMA          */
    /********************************/

    JUINT32 fGtrSrcIs422 ;
    JUINT32 fGtrUVcons   ;
    JUINT32 fGtrIsGray   ;
    JUINT32 fGtrIsJpeg   ;   
    JUINT32 fGtrBufFstHeight[2]  ;
    //JUINT32 fGtrBufHeight[2]     ;
    JUINT32 fGtrBufLastHeight[2] ;
    
    
    JUINT32 fRowHeight[3] ;


    /********************************/
    /*         config Tile          */
    /********************************/
    
    JUINT32 fTileCfgBufSize;
    
    JUINT32 fTileTotalRun ;
    JUINT32 fTileCfgCnt ;
    

    
    /***************************************************/
    //                  CDP 
    /***************************************************/


    CONFIG_RESIZER_DATA cfgCdpData;
    
    TILE_ROW_DATA tRowCfg[TILE_ROW_SIZE];
    
    
    ISPIO_TDRI_INFORMATION_STRUCT *tTileCfgBuf ;
    ISPIO_TDRI_INFORMATION_STRUCT *tTileCfgBuf_org ;

    
    EScenarioID scenario_init ;//= eScenarioID_N3D_IC;

    ResMgrDrv* pResMgr;
    RES_MGR_DRV_MODE_STRUCT ResMgrMode;
    PipeMgrDrv* pPipeMgr;
    
    

    ICdpPipe* pCdpPipeRing;
    

    /*** cdp parameter declaration ***/ 
    
    PortInfo imgi_ring;
    PortInfo vipi_ring;
    PortInfo vip2i_ring;
    PortInfo dispo_ring;



    unsigned char *fSrcPmemVA;
    unsigned char *fSrcCbPmemVA;
    unsigned char *fSrcCrPmemVA;
    unsigned char *fDstPmemVA;
    int fSrcPmemFD;
    int fSrcCbPmemFD;
    int fSrcCrPmemFD;
    int fDstPmemFD;
    
    
    //TDRI
    unsigned int fIRQinRow;
    unsigned char *fTileCfgPmemVA;    
    int fTileCfgPmemFD;    
    
    
    MTKM4UDrv *pM4uDrv;
    M4U_MODULE_ID_ENUM fm4uCdpID ; 
    
    JUINT32 fSrcM4UAddrMva0[3];
    JUINT32 fSrcM4UAddrMva1[3];
    JUINT32 fDstM4UAddrMva;

    //ION

    bool fIonEn ;
    JINT32 fIonDevFD ;
    JINT32 fDstFD ;
    void* fDstIonHdle ;
    void* fDstIonVA ;
    
    unsigned long fDstHandle ;
    
    

    int decID;
    
    
    

   
};


#endif 
