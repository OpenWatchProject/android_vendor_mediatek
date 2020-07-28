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
#include <cutils/log.h>
#include <utils/Errors.h>
#include <fcntl.h>
#include <sys/mman.h>

//#include <errno.h>
#include <unistd.h>
#include <sys/types.h>

#include "Trace.h"
//#define ATRACE_TAG ATRACE_TAG_GRAPHICS
//#define ATRACE_CALL(...)

#include <img_mdp_pipe.h>

#define SCALER_EP

#define USE_DPF_METHOD

#define JPG_MDP_MAX_ELEMENT 3

#define JPEG_IRQ_TIMEOUT_DEC 5000  /*3000*/


//#define SCALER_TILE_BUF_REG_SIZE (36*1024)
#define SCALER_TILE_BUF_CFG_SIZE (10*1024)
//#define SCALER_TILE_BUF_CFG_SIZE (750*1024)



#undef bool
#define bool bool


#include <DpDataType.h>
#include <DpFragStream.h>

class DpStream;
class DpChannel;
class DpRingBufferPool;
class DpBasicBufferPool;


static bool fail_return(const char msg[]) {
#if 1
    JPG_ERR("[ImageScaler] - %s", msg);
#endif
    return false;   // must always return false
}

static int code_return(const char msg[], int line,int code) {
#if 1
    JPG_ERR("[ImageScaler] - %s, L:%d!!\n", msg, line);
#endif
    return code;   // must always return code
}

#define SEL_MIN(a,b) ( ((a)<(b)) ? (a):(b) )
#define SEL_MAX(a,b) ( ((a)>(b)) ? (a):(b) )
#define CLIP_255(x) ( ((x) < 0) ? (0): ( SEL_MIN( (x), 255 ) ) )


#define DUMP_CDP_LVL_OUT 1
#define DUMP_CDP_LVL_STEP_IN 2
#define DUMP_CDP_LVL_STEP_OUT 3


int dump2file(unsigned int level, const char filename[], unsigned int *index, unsigned char *SrcAddr, unsigned int size)
{

   FILE *fp = NULL;
   FILE *fpEn = NULL;
   unsigned char* cptr ;
   const char tag[64] = "DUMP_CDP_PIPE";
   char filepath[128];
   char dumpEn[128] ;

#ifndef ENABLE_IMG_CODEC_DUMP_RAW
   return false ;
#endif

   //sprintf(filepath, "/data/otis/dec_pipe_scaler_step_%04d.raw", fileNameIdx);
   sprintf(  dumpEn, "//data//otis//%s_%d", tag, level);
   sprintf(filepath, "//data//otis//%s_%04d.raw", filename, *index);

   fpEn = fopen(dumpEn, "r");
   if(fpEn == NULL)
   {
       //JPG_ERR("Check Dump En is zero!!\n");
       return false;
   }
   fclose(fpEn);

   fp = fopen(filepath, "w");
   if (fp == NULL)
   {
       JPG_ERR("open Dump file fail: %s\n", filepath);
       return false;
   }

   JPG_DBG("\nDumpRaw -> %s, En %s, addr %p, size %x !!", filepath,dumpEn,SrcAddr, size);
   cptr = (unsigned char*)SrcAddr ;
   for( unsigned int i=0;i<size;i++){  /* total size in comp */
     fprintf(fp,"%c", *cptr );
     cptr++;
   }

   fclose(fp);
   //*index++;

   return true ;
}


#define IMGDEC_ALLOC(ptr, size, align, align_ptr) \
{  \
   ptr = (ISPIO_TDRI_INFORMATION_STRUCT*)malloc(size+align) ; \
   align_ptr = (ISPIO_TDRI_INFORMATION_STRUCT*)TO_CEIL(ptr, align);    \
   /*JPG_DBG("[IMGDEC_ALLOC] memory_alloc addr %x(%x), size %x, align %x, L:%d!!\n", (unsigned int)(ptr), (unsigned int)(align_ptr), size,align,__LINE__);*/ \
   /*memset(ptr, 0 , size+align);*/ \
}

#define IMGDEC_FREE(ptr) \
{  \
   /*JPG_DBG("[IMGDEC_FREE] memory_free addr %x, L:%d!!\n", (unsigned int)(ptr), __LINE__);*/ \
   if(ptr != NULL)   \
     free(ptr) ;  \
   ptr = NULL ;   \
}


ImgCdpPipe::ImgCdpPipe()
{
    ATRACE_CALL();
    fDpStatus = DP_STATUS_UNKNOWN_ERROR ;
    fStartFragStreamFlag = 0;
    pStream = NULL;
    fIsScalerEnd = 0;
    fDecRowStart = 0;
    fDecRowEnd = 0;
    fDecRowNum = 0;
    fPrvDecRowStart = 0;
    fPrvDecRowEnd = 0;
    fPrvDecRowNum = 0;
    fDqBufID = 0;

    islock = false;
    isDither = false;

    fFrameLevelCMDQ = 0;

    fCropInWidth  = 0;
    fCropInHeight = 0;

    fLockCnt = 0;

    fMemType = fMemTypeDefault = IMG_MEM_TYPE_M4U ;

    fScalerMode = SCALER_MODE_TILE ;
    fDstBufAddr = NULL;
    fDstBufAddr1 = NULL;
    fDstBufAddr2 = NULL;
    fDstBufSize = 0;
    fDstBufSize1 = 0;
    fDstBufSize2 = 0;

    fOutWidth = fOutHeight = 0;
    fSrcFormat = SCALER_IN_NONE;
    fDstFormat = SCALER_OUT_NONE;

    fCurRowStart =
    fCurRowEnd   =
    fNxtRowStart =
    fNxtRowEnd   = 0;
    fTileRunCnt = 0;
    fTileBufMcuRow = 0;
    fSrcImgMcuRow = 0;
    fMcuInRow = 0;

    for(unsigned int i = 0;i<3; i++){
      fSrcBufAddr0[i] = 0;
      fSrcBufAddr1[i] = 0;
    }

    fDstConfigAddr = NULL;
    fDstConfigAddrPA = 0;

    fIsUseCmodel = 0;
    fIsCmdqMode = 0;

    fTdsFlag = 0;
    fPPParam = NULL;
    fISOSpeed = 0;

    fDstFD = -1;
}


ImgCdpPipe::~ImgCdpPipe()
{
    unlock();
}


void yuv2rgb(unsigned char *R, unsigned char *G, unsigned char *B, int Y,int U,int V)
{
  int C,D,E;

  C = Y - 16  ;
  D = U - 128 ;
  E = V - 128 ;

  *R = (unsigned char)(CLIP_255(( (298 * C)             + (409 * E) + 128) >> 8)) ;
  *G = (unsigned char)(CLIP_255(( (298 * C) - (100 * D) - (208 * E) + 128) >> 8)) ;
  *B = (unsigned char)(CLIP_255(( (298 * C) + (516 * D)             + 128) >> 8)) ;
}


typedef struct {

  unsigned int srcWidth;
  unsigned int srcHeight;
  unsigned int srcMemStride[3];

  unsigned char *srcAddr[3];
  unsigned char *dst_buf;

  unsigned int srcFormat;  //0: YUV420, 1: YUV422
  unsigned int dstFormat;  //0: ARGB8888, 1:RGB888, 2:RGB565
  unsigned int pixelSize ;


} CONV_YUV2RGB_DATA ;


bool convYUV2RGB(CONV_YUV2RGB_DATA* convY2R)
{

  int y, cb, cr;

  unsigned char* inptr0 ;
  unsigned char* inptr1 ;
  unsigned char* inptr2 ;

  unsigned char *outptr;
  int num_rows = convY2R->srcHeight ;
  unsigned int pixelSzie = convY2R->pixelSize ;
  unsigned int dstByteInRow = convY2R->srcWidth * convY2R->pixelSize ;
  unsigned char * output_buf = convY2R->dst_buf ;
  unsigned int row_ctr = 0;
  unsigned int num_cols = convY2R->srcWidth ;
  unsigned int input_row =0, input_row_uv = 0;

  unsigned int is422 = 0;
  unsigned int col = 0;
  unsigned char *R;
  unsigned char *G;
  unsigned char *B;

  if(convY2R->srcFormat == 1)
   is422 = 1;

  printf("\nYUV2RGB : %d %d ,dstByteInRow %d, stride %d %d", convY2R->srcWidth, convY2R->srcHeight,dstByteInRow, convY2R->srcMemStride[0], convY2R->srcMemStride[1]);

  while (--num_rows >= 0) {

    inptr0 = convY2R->srcAddr[0] + (input_row*convY2R->srcMemStride[0]);

      inptr1 = convY2R->srcAddr[1] + (input_row_uv*convY2R->srcMemStride[1]);
      inptr2 = convY2R->srcAddr[2] + (input_row_uv*convY2R->srcMemStride[2]);


    outptr = output_buf+(dstByteInRow*row_ctr);

    //printf("\nY2R: row %d %d->%d, src %x %x %x -> %x !!",input_row, input_row_uv, row_ctr, inptr0, inptr1, inptr2, outptr );

    for (col = 0; col < num_cols; col++) {
      //printf(" %d", col);
      y  = inptr0[col];
      cb = inptr1[col/2];
      cr = inptr2[col/2];

      yuv2rgb( outptr, outptr+1, outptr+2, y, cb, cr);
      R = outptr+0;
      G = outptr+1;
      B = outptr+2;

      outptr += pixelSzie;

      /* Range-limiting is essential due to noise introduced by DCT losses. */
      //outptr[RGB_RED] =   range_limit[y + Crrtab[cr]];
      //outptr[RGB_GREEN] = range_limit[y +  ((int) RIGHT_SHIFT(Cbgtab[cb] + Crgtab[cr], SCALEBITS))];
      //outptr[RGB_BLUE] =  range_limit[y + Cbbtab[cb]];
    }
    //printf("\nY2R: row %d %d->%d, src %x %x %x -> %x, YUV %d %d %d -> %d %d %d !!",input_row, input_row_uv, row_ctr, inptr0, inptr1, inptr2, outptr, y, cb, cr, *R, *G, *B );

    row_ctr++;
    input_row++;

    if(is422)
      input_row_uv ++ ;
    else
      input_row_uv = input_row/2;
  }

  return true;
}


bool ImgCdpPipe::lock(JUINT32 mode)
{
   if(!setConnectType(mode))
      return code_return("setConnectType fail!!",__LINE__,false);

    islock = true;
    return true;
}


bool ImgCdpPipe::unlock()
{
    if(islock)
    {
       ATRACE_CALL();

       onHWScalerDeInit();

#if 1
{
        static unsigned int index = 0;
        if(dump2file(DUMP_CDP_LVL_OUT,"dumpCdpPipe_out", &index, (unsigned char*)fDstBufAddr, fDstBufSize))
         index++;
}
#endif
       //freeIOMEM();

       islock = false;
    }

    return true;
}


bool ImgCdpPipe::checkParam()
{
    if(fScalerMode == SCALER_MODE_NONE || fScalerMode >= SCALER_MODE_SIZE)
    {
        JPG_ERR("Invalide scalerMode %d", fScalerMode);
        return false;
    }

    if(fSrcImgWidth[0] == 0 || fSrcImgHeight[0] == 0)
    {
        JPG_ERR("Invalide JPEG width/height %u/%u", fSrcImgWidth[0], fSrcImgHeight[0]);
        return false;
    }

    if(fSrcBufAddr0[0] == 0 )
    {
        return fail_return("Invalide Address");
    }

    if( fDstFD < 0 && fDstBufAddr == 0 )
    {
        return fail_return("Invalide Address");
    }

    return true;
}


bool ImgCdpPipe::setConnectType(JUINT32 mode)
{
   if(mode >= SCALER_MODE_SIZE || mode == SCALER_MODE_NONE)
      return false;

   if( !onHWScalerCreate(mode) ){
     onHWScalerDeInit();
     return false;
   }else
     return true;
}

/*
enum EImageFormat
{
    eImgFmt_UNKNOWN         = 0x00000,   //unknow
    eImgFmt_BAYER8          = 0x00001,   //Bayer format, 8-bit
    eImgFmt_BAYER10         = 0x00002,   //Bayer format, 10-bit
    eImgFmt_BAYER12         = 0x00004,   //Bayer format, 12-bit
    eImgFmt_YV12            = 0x00008,   //420 format, 3 plane(YVU)
    eImgFmt_NV21            = 0x00010,   //420 format, 2 plane (VU)
    eImgFmt_NV21_BLK        = 0x00020,   //420 format block mode, 2 plane (UV)
    eImgFmt_NV12            = 0x00040,   //420 format, 2 plane (UV)
    eImgFmt_NV12_BLK        = 0x00080,   //420 format block mode, 2 plane (VU)
    eImgFmt_YUY2            = 0x00100,   //422 format, 1 plane (YUYV)
    eImgFmt_UYVY            = 0x00200,   //422 format, 1 plane (UYVY)
    eImgFmt_RGB565          = 0x00400,   //RGB 565 (16-bit), 1 plane
    eImgFmt_RGB888          = 0x00800,   //RGB 888 (24-bit), 1 plane
    eImgFmt_ARGB888         = 0x01000,   //ARGB (32-bit), 1 plane
    eImgFmt_JPEG            = 0x02000,   //JPEG format
    eImgFmt_YV16            = 0x04000,   //422 format, 3 plane
    eImgFmt_NV16            = 0x08000,   //422 format, 2 plane (UV)
    eImgFmt_NV61            = 0x10000,   //422 format, 2 plane (VU)
    eImgFmt_I420            = 0x20000,   //420 format, 3 plane(YUV)
};
*/


#define TILE_STRIDE_UNIT 256
int ImgCdpPipe::configCdpIO()
{
   return true ;
}


int ImgCdpPipe::onHWScalerCreate(JUINT32 preferMode)
{
   ATRACE_CALL();

   if(preferMode == SCALER_MODE_TILE || preferMode == SCALER_MODE_FRAME){
      ;//do nothing
   }else
      return false ;

   //if(pStream == NULL)
   {
#if 0
     pStream = new DpFragStream();
     if(pStream == NULL)
       return code_return("::onHWScaler_Create fail, ",__LINE__,false) ;
#endif
     fScalerMode =  preferMode ;


     if(!lockScaler() ){
         return code_return("::lockScaler fail, ",__LINE__,false) ;
     }
   }

  return true;
}


int ImgCdpPipe::onHWScalerDeInit()
{
   //ATRACE_CALL();
   if(fIsUseCmodel )
      return true;

   if(pStream != NULL){
     if(fStartFragStreamFlag){
       //atrace_begin(ATRACE_TAG_GRAPHICS,"stopFrag");
       fDpStatus = pStream->stopFrag();
       fStartFragStreamFlag = 0;
       //atrace_end(ATRACE_TAG_GRAPHICS);
     }
     //atrace_begin(ATRACE_TAG_GRAPHICS,"delete Stream");
     delete pStream ;
     //atrace_end(ATRACE_TAG_GRAPHICS);
     pStream = NULL;
     if(DP_STATUS_RETURN_SUCCESS != fDpStatus){
       return code_return("::[DEINIT] ScalerDeInit fail, ",__LINE__,false);
     }
   }

   if(!unlockScaler(1) ){
       return code_return("::unlockScaler fail, ",__LINE__,false) ;
   }

   //JPG_DBG("::ScalerDeInit done, L:%d!!\n", __LINE__);
   return true;
}


int ImgCdpPipe::onTileConfig()
{
   return true;
}


bool ImgCdpPipe::lockScaler()
{
   if(!fLockCnt)
   {
     //try to lock pipeMgr

     fLockCnt++;
   }

   return true ;
}


bool ImgCdpPipe::unlockScaler(unsigned int isUnlockAll)
{
   if(fLockCnt){
      if(isUnlockAll){
        for(;fLockCnt > 0; fLockCnt--){
           //unlock pipeMgr
        }
      }else{
           //fLockCnt--;
      }
   }
   return true ;
}

int ImgCdpPipe::onTileRun(unsigned int tileOffset)
{
    // unused params
   (void)tileOffset;

   ATRACE_CALL();

   if( !onCdpGo())
      return code_return("trigger CDP fail, ",__LINE__, false);

   if(!onCdpWaitIrq())
      return code_return("wait CDP fail, ",__LINE__, false);

   return true;
}


bool ImgCdpPipe::onCdpWaitIrq()
{
   {
     static unsigned int fileNameIdx = 0;
     if(dump2file(DUMP_CDP_LVL_STEP_OUT,"dumpCdpPipe_OutStepAf", &fileNameIdx, (unsigned char*)fDstConfigAddr, fDstBufSize))
       fileNameIdx++;
   }

   return true ;
}


bool ImgCdpPipe::onCdpGo()
{
   return true;
}


bool ImgCdpPipe::onTileStart()
{
    JINT32 tmpFD;
    fDpStatus = pStream->queueFrag(fDqBufID);

    if(DP_STATUS_RETURN_SUCCESS != fDpStatus){
      return code_return("::[START] scaler trigger fail, ",__LINE__,false);
    }

    fTileRunCnt++;
    fPrvDecRowStart = fDecRowStart ;
    fPrvDecRowEnd   = fDecRowEnd  ;
    fPrvDecRowNum   = fDecRowNum  ;

    if(!waitDecodeBuffer(&fDqBufID, &tmpFD, &fDecRowStart, &fDecRowEnd, &fDecRowNum, &fIsScalerEnd ))
      return code_return("::[START] scaler wait_Decode_Buffer fail, ",__LINE__,false);

    if(fScalerMode == SCALER_MODE_TILE ){
    }

    return true;
}


bool ImgCdpPipe::onSwStart()
{
      CONV_YUV2RGB_DATA convYR ;
      unsigned int srcRowOffset[3];
      unsigned int dstRowOffset;

      memset(&convYR, 0, sizeof(CONV_YUV2RGB_DATA));

      if(fSrcFormat == SCALER_IN_3P_YUV420){
        convYR.srcFormat = 0;  //0: YUV420, 1: YUV422, 2:GRAY
      }else if(fSrcFormat == SCALER_IN_3P_YUV422)
        convYR.srcFormat = 1;
      else if(fSrcFormat == SCALER_IN_1P_GRAY)
         convYR.srcFormat = 2;

      convYR.pixelSize = 0;

      if( fDstFormat == SCALER_OUT_1P_ARGB8888 ){
        convYR.dstFormat = 0;  //0: ARGB8888, 1:RGB888, 2:RGB565
        convYR.pixelSize = 4;
      }else if( fDstFormat == SCALER_OUT_1P_RGB888 ){
         convYR.dstFormat = 1;
         convYR.pixelSize = 3;
      }else if( fDstFormat == SCALER_OUT_1P_RGB565 ){
         convYR.dstFormat = 2;
         convYR.pixelSize = 2;
      }

      srcRowOffset[0] = fCurRowStart * fSrcBufStride[0] * fRowHeight[0] ;
      srcRowOffset[1] = fCurRowStart * fSrcBufStride[1] * fRowHeight[1] ;
      srcRowOffset[2] = fCurRowStart * fSrcBufStride[2] * fRowHeight[2] ;
         dstRowOffset = fCurRowStart * fSrcImgWidth[0] * fRowHeight[0]* convYR.pixelSize ;

      convYR.srcWidth = fSrcImgWidth[0];
      convYR.srcHeight = fRowHeight[0];
      convYR.srcMemStride[0] = fSrcBufStride[0];
      convYR.srcMemStride[1] = fSrcBufStride[1];
      convYR.srcMemStride[2] = fSrcBufStride[2];

      convYR.srcAddr[0] = (unsigned char*)( (unsigned long)fSrcBufAddr0[0] + srcRowOffset[0]);
      convYR.srcAddr[1] = (unsigned char*)( (unsigned long)fSrcBufAddr0[1] + srcRowOffset[1]);
      convYR.srcAddr[2] = (unsigned char*)( (unsigned long)fSrcBufAddr0[2] + srcRowOffset[2]);
      convYR.dst_buf = (unsigned char*)((unsigned long)fDstBufAddr + dstRowOffset);

      JPG_DBG("IMG_RSZ_PATH_CMODEL:[%d, %d] %lx %lx %lx %lx !! \n", fTileRunCnt, fSrcImgMcuRow,
              (unsigned long)convYR.srcAddr[0], (unsigned long)convYR.srcAddr[1],
              (unsigned long)convYR.srcAddr[2], (unsigned long)convYR.dst_buf);

      convYUV2RGB(&convYR);

      //update tile idx
      fCurRowStart = fNxtRowStart ;
      fCurRowEnd = fNxtRowEnd ;
      if(fNxtRowStart   < fSrcImgMcuRow){
         fNxtRowStart++ ;
         fNxtRowEnd = fNxtRowStart+1;
         }

       JPG_DBG("IMG_RSZ_PATH_CMODEL:[%d, %d] {%d, %d}, {%d, %d}!! \n", fTileRunCnt, fSrcImgMcuRow,fCurRowStart, fCurRowEnd, fNxtRowStart, fNxtRowEnd);

       fTileRunCnt++;
      return true;
}


//(00) start_no(0000) end_no(0002) stop_f(0001) offset(0000)
//(01) start_no(0002) end_no(0004) stop_f(0001) offset(0028)
//(02) start_no(0004) end_no(0006) stop_f(0001) offset(0056)


//if there is next tile, go find the next tileRow Start & End index
bool ImgCdpPipe::findNxtTileWindow(JUINT32 entry_start, JUINT32 run_cnt, TILE_ROW_DATA *row_data )
{
    // unused params
   (void)entry_start;
   (void)run_cnt;
   (void)row_data;

   return true;
}


bool ImgCdpPipe::waitDecodeBuffer(JINT32 *dqBufID, JINT32 *dqBufFD, JUINT32 *dqBufMcuStart, JUINT32 *dqBufMcuEnd, JUINT32 *dqBufMcuNum, JUINT32 *isFinish)
{
   ATRACE_CALL();
   DpColorFormat  dqFormat  ;
   int dqBufWidth = 0;
   int dqBufHeight = 0;
   int dqBufStride = 0;
   int dqBufRowIdx = 0;
   int dqBufMCUx = 0;
   //unsigned int dqBufID     ;
   void *srcBuf[3]= {0};

   int decodeRowNum = 0;

   fDpStatus = pStream->dequeueFrag(dqBufID,
                                 &dqFormat,
                                 srcBuf,
                                 dqBufFD,
                                 &dqBufMCUx,
                                 &dqBufRowIdx,
                                 &dqBufWidth,
                                 &dqBufHeight,
                                 &dqBufStride);

   if(DP_STATUS_RETURN_SUCCESS != fDpStatus && DP_STATUS_BUFFER_DONE != fDpStatus){
     return code_return("::[INIT] scaler dequeueFrag fail, ",__LINE__,false);
   }
   if(DP_STATUS_BUFFER_DONE == fDpStatus){
      //JPG_DBG("wait_Decode_Buffer:DONE st %d, L:%d!!\n", fDpStatus, __LINE__);
      *isFinish = 1;
   }

   if(dqBufRowIdx == 0){

     //if(fSrcFormat == SCALER_IN_3P_YUV444){
       fSrcBufAddr0[0] = srcBuf[0] ;
       fSrcBufAddr0[1] = srcBuf[1] ;
       fSrcBufAddr0[2] = srcBuf[2] ;
     //}else{
     //  fSrcBufAddr0[0] = (unsigned int)srcBuf[0] ;
     //  fSrcBufAddr0[1] = (unsigned int)srcBuf[2] ;
     //  fSrcBufAddr0[2] = (unsigned int)srcBuf[1] ;
     //}
   }
    *dqBufMcuStart = dqBufRowIdx ;
    *dqBufMcuNum = decodeRowNum = dqBufHeight/fRowHeight[0] ;
    *dqBufMcuEnd = dqBufRowIdx + decodeRowNum -1;

    //if(DP_STATUS_BUFFER_DONE != fDpStatus)
    //  JPG_DBG("wait_Decode_Buffer: MCUROW(%d->%d,%d), BufID %d, w %d, stride %d, st %d, L:%d!!\n", dqBufRowIdx, *dqBufMcuEnd,decodeRowNum, *dqBufID, dqBufWidth, dqBufStride, fDpStatus, __LINE__);

    return true ;

}


bool ImgCdpPipe::getSrcBuffer(void **pSrcBuf, JINT32 *pSrcBufFD, JUINT32 *pSrcBufStride, JUINT32 *tBufRowNum )
{
    ATRACE_CALL();

    unsigned int i =0;
    unsigned int dstBufSize[3] ;
    void *dstBuf[3];
    void *srcBuf[3];
    unsigned int tileRowNum;
    DpColorFormat  dqFormat;
    unsigned int dqBufWidth;
    unsigned int dqBufHeight;
    unsigned int dqBufStride;
    unsigned int dqBufRowIdx ;
    unsigned int dqBufMCUx ;
    unsigned int dqBufID ;
    unsigned int decodeRowNum = 0;
    unsigned int dstBufBpp = 4;

    unsigned int cropInWidthAlign = (fSrcFormat == SCALER_IN_3P_YUV444)? fCropInWidth : TO_CEIL(fCropInWidth, 2);

    if(fScalerMode != SCALER_MODE_TILE){
       return true ;
    }

    DpColorFormat fInputFormat ;
    DpColorFormat fOutputFormat ;

    if(fSrcFormat == SCALER_IN_3P_YUV420){
      //fInputFormat = eYUV_420_3P ; //YUV 420 format, 3 plane
      //fInputFormat = eYV12 ; //YVU 420 3 plane
      fInputFormat = eI420 ; //YUV420
      fMcuSampleWidth[0] = 16 ;
    }else if(fSrcFormat == SCALER_IN_3P_YUV422){
      //fInputFormat = eYUV_422_3P;  //YUV, 422 format, 3 plane
      //fInputFormat = eYV16 ;   //YVU, 422 format, 3 plane
      fInputFormat = eI422 ; //YUV422
      fMcuSampleWidth[0] = 16 ;
    }else if(fSrcFormat == SCALER_IN_1P_GRAY){
      fInputFormat = eGREY;  //422 format, 3 plane
      fMcuSampleWidth[0] = 8 ;
    }else if (fSrcFormat == SCALER_IN_1P_RGB888){
      fInputFormat = eRGB888;  //
      fMcuSampleWidth[0] = 1 ;
    }else if (fSrcFormat == SCALER_IN_1P_ARGB8888){
      fInputFormat = eRGBA8888;  //eImgFmt_ARGB888 ;
      fMcuSampleWidth[0] = 1 ;
    }else if (fSrcFormat == SCALER_IN_1P_RGB565){
      fInputFormat = eRGB565;   //eImgFmt_RGB565 ;
      fMcuSampleWidth[0] = 1 ;
    }else if(fSrcFormat == SCALER_IN_3P_YUV444){
      fInputFormat = eI444 ;   //YUV, 444 format, 3 plane
      fMcuSampleWidth[0] = 8 ;
    }else{
      fInputFormat = eYV16;//eImgFmt_YV16 ; //422 format, 3 plane
      return code_return("::[IOINIT] invalid Scaler Source Format, ", __LINE__, false);
    }

    if(fDstFormat == SCALER_OUT_1P_ARGB8888){
      fOutputFormat = eRGBA8888;//eImgFmt_ARGB888;
      dstBufBpp = 4;
    }else if(fDstFormat == SCALER_OUT_1P_RGB565){
      fOutputFormat = eRGB565; //eImgFmt_RGB565;
      dstBufBpp = 2;
    }else if(fDstFormat == SCALER_OUT_1P_RGB888){
      fOutputFormat = eRGB888; //eImgFmt_RGB888;
      dstBufBpp = 3;
    }else if(fDstFormat == SCALER_OUT_1P_YUV422){
      fOutputFormat = eYUYV; //eImgFmt_YUY2;
      dstBufBpp = 2;
    }else if(fDstFormat == SCALER_OUT_3P_YVU420){
      fOutputFormat = eI420; //eImgFmt_YUY2;
      dstBufBpp = 1;
    }else if(fDstFormat == SCALER_OUT_3P_YV12 || fDstFormat == SCALER_OUT_3P_YV12_ANDROID){
      fOutputFormat = eYV12; //eImgFmt_YUY2;
      dstBufBpp = 1;
    }else{
      fOutputFormat = eRGBA8888;
      dstBufBpp = 4;
      return code_return("::[IOINIT] invalid Scaler destination format, ", __LINE__, false);
    }

    //JPG_DBG("ScalerInit: fmt src %d->0x%x, dst %d->%d, Mode %d!!\n", fSrcFormat, fInputFormat, fDstFormat, fOutputFormat, fScalerMode);

    DpRect tmpRect;
    tmpRect.x = 0;
    tmpRect.y = 0;
    tmpRect.w = fCropInWidth;
    tmpRect.h = fCropInHeight;
    dstBufSize[0] = fDstBufSize ;
    dstBuf[0] = fDstBufAddr ;

    if(fDstFormat == SCALER_OUT_3P_YVU420 || fDstFormat == SCALER_OUT_3P_YV12 || fDstFormat == SCALER_OUT_3P_YV12_ANDROID){
        dstBufSize[1] = fDstBufSize1 ;
        dstBufSize[2] = fDstBufSize2 ;
        dstBuf[1] = fDstBufAddr1 ;
        dstBuf[2] = fDstBufAddr2 ;
    }

    fSrcBufStride[0] = (unsigned int) pSrcBufStride[0] ;
    fSrcBufStride[1] = (unsigned int) pSrcBufStride[1] ;
    fSrcBufStride[2] = (unsigned int) pSrcBufStride[2] ;

    //atrace_begin(ATRACE_TAG_GRAPHICS,"new Stream");
    //if(pStream == NULL)
    //  pStream = new DpFragStream();
    if(pStream == NULL){
      pStream = new DpFragStream();
    if(pStream == NULL)
      return code_return("::[INIT] create RingStream fail, ",__LINE__,false);
    }else{
      return code_return("::getSrcBuffer fail(Stream not null) ",__LINE__,false) ;
    }
    //atrace_end(ATRACE_TAG_GRAPHICS);

#ifdef USE_DPF_METHOD
    fDpStatus = pStream->setSrcConfig(fInputFormat,
                                   cropInWidthAlign, //pSrcBufStride[0], //fSrcImgWidth[0],
                                   fRowHeight[0]*fSrcImgMcuRow,//fSrcImgHeight[0],
                                   fMcuSampleWidth[0],
                                   fRowHeight[0],
                                   pSrcBufStride[0], //fSrcImgWidth[0],
                                   pSrcBufStride[1], //fSrcImgWidth[1],
                                   &tmpRect);
#endif

    if(DP_STATUS_RETURN_SUCCESS != fDpStatus){
      return code_return("::[INIT] set setSrcConfig fail, ",__LINE__,false);
    }

    //JPG_DBG("ScalerInit: CONFIG_DST:fmt %d, w/h(%d %d), stride %d, addr0 %x, addr1 %x, addr2 %x, size0 %x, size1 %x, size2 %x!!\n", fOutputFormat, fOutWidth, fOutHeight, fOutWidth,
    //dstBuf[0], dstBuf[1], dstBuf[2], dstBufSize[0], dstBufSize[1], dstBufSize[2]);

    {
        unsigned int planeNumber = 1;
        if(fDstFormat == SCALER_OUT_3P_YVU420 || fDstFormat == SCALER_OUT_3P_YV12 || fDstFormat == SCALER_OUT_3P_YV12_ANDROID)
            planeNumber = 3;

        if (fDstFD >= 0)
            fDpStatus = pStream->setDstBuffer(fDstFD, dstBufSize, planeNumber);
        else
            fDpStatus = pStream->setDstBuffer(dstBuf, dstBufSize, planeNumber);
    }

    if(DP_STATUS_RETURN_SUCCESS != fDpStatus){
      return code_return("::[INIT] set setDstBuffer fail, ",__LINE__,false);
    }

    if (fDstBufStride[0] == 0) // use default dst buffer stride if user is not indicate
    {
        fDstBufStride[0] = (fDstFormat == SCALER_OUT_3P_YV12_ANDROID)?
                           TO_CEIL32(fOutWidth*dstBufBpp, 16) : fOutWidth*dstBufBpp;
        fDstBufStride[1] = (fDstFormat == SCALER_OUT_3P_YV12_ANDROID)?
                           TO_CEIL32(fOutWidth*dstBufBpp/2, 16) : fOutWidth*dstBufBpp;
    }

    if (fDstFormat == SCALER_OUT_3P_YV12_ANDROID)
        fDpStatus = pStream->setDstConfig(fOutputFormat,
                                       fOutWidth,
                                       fOutHeight,
                                       fDstBufStride[0],
                                       fDstBufStride[1]);
    else
        fDpStatus = pStream->setDstConfig(fOutputFormat,
                                       fOutWidth,
                                       fOutHeight,
                                       fDstBufStride[0]);

    if(DP_STATUS_RETURN_SUCCESS != fDpStatus){
      return code_return("::[INIT] scaler setDstConfig fail, ",__LINE__,false);
    }
    fStartFragStreamFlag = 1;
    pStream->setMcuCol(fMcuInRow);
    pStream->setEmbeddJPEG( (fFrameLevelCMDQ)? true : false );
#ifdef USE_DPF_METHOD
    {
        DpPqParam pqParam;
        uint32_t* pParam = &pqParam.u.image.info[0];

        pqParam.enable = (fTdsFlag == 0)? false:true;
        pqParam.scenario = MEDIA_PICTURE;
        pqParam.u.image.iso = fISOSpeed;
        if (fPPParam)
        {
            //JPG_DBG("ScalerInit: enable imgDc pParam %p\n", fPPParam);
            pqParam.u.image.withHist = true;
            memcpy((void*)pParam, fPPParam, 20 * sizeof(uint32_t));
        }
        else
            pqParam.u.image.withHist = false;

        pStream->setPQParameter(pqParam);
    }
    //atrace_begin(ATRACE_TAG_GRAPHICS,"startFrag");
    fDpStatus = pStream->startFrag(&tileRowNum, (fTdsFlag > 0)? 1:0 );
    //atrace_end(ATRACE_TAG_GRAPHICS);
#endif
    if(DP_STATUS_RETURN_SUCCESS != fDpStatus){
      return code_return("::[INIT] scaler startFrag fail, ",__LINE__,false);
    }

    *tBufRowNum = fTileBufMcuRow = tileRowNum ;

    //JPG_DBG("ScalerInit: CONFIG_ST: RINGMCU %d, tdsp %d!!\n", tileRowNum, fTdsFlag);

    if(!waitDecodeBuffer(&fDqBufID, pSrcBufFD, &fDecRowStart, &fDecRowEnd, &fDecRowNum, &fIsScalerEnd ))
      return code_return("::[INIT] scaler wait_Decode_Buffer fail, ",__LINE__,false);

    for(int i = 0; i < 3 ;i++){
      pSrcBuf[i]= fSrcBufAddr0[i] ;
    }

    //if (*pSrcBufFD >= 0 && fDstFD >= 0)
    //  JPG_DBG("::[INIT] scaler use ion (%d, %d)\n", *pSrcBufFD, fDstFD);

   return true ;
}


JINT32 ImgCdpPipe::TileInit()
{
   unsigned int i =0;
   if(fScalerMode != SCALER_MODE_TILE){
      return true ;
   }

   ATRACE_CALL();

   return true;
}


bool ImgCdpPipe::onFrameStart()
{
   if(fIsUseCmodel){
     //{0,1} => {1,2}
     fCurRowStart = 0 ;
     fCurRowEnd   = 1 ;
     fNxtRowStart = 1 ;
     fNxtRowEnd   = 2 ;
   }

   if(!configCdpIO())
      return code_return("::[INIT] config Scaler IO Fail, ", __LINE__,false);

   if( !onCdpGo())
      return code_return("trigger CDP fail, ",__LINE__, false);

   //JPG_DBG("IMG_CDP_PATH cfg Tile [%d %d], [%d %d]!!", fCurRowStart, fCurRowEnd, fNxtRowStart, fNxtRowEnd);

   return true;
}


static unsigned int pixel_byte[5] = {4,4,2,3,4};
bool ImgCdpPipe::Start()
{
    unsigned int ret = 0;
    unsigned int draw_cnt= 0, draw_num = 4;
    unsigned int draw_base = fOutWidth*pixel_byte[fDstFormat]*(fOutHeight/2);

    if( fScalerMode != SCALER_MODE_TILE || fTileRunCnt == 0){
      //JPG_DBG("ReSizer Src Addr bank[0]:%p %p %p, [1]:%p %p %p !! ", fSrcBufAddr0[0], fSrcBufAddr0[1], fSrcBufAddr0[2], fSrcBufAddr1[0], fSrcBufAddr1[1], fSrcBufAddr1[2]);
      //JPG_DBG("ReSizer Dst Addr:0x%x, w/h:[%d %d]->[%d %d], format:%d->%d", fDstBufAddr, fSrcImgWidth[0], fSrcImgHeight[0],fOutWidth,fOutHeight, fSrcFormat,fDstFormat);
    }
    //Check param
    if(true != checkParam()) {
        return false;
    }

    if( fScalerMode == SCALER_MODE_TILE ){
      if(fIsUseCmodel ){
        if(!onSwStart())
           return code_return("start sw_conv fail, ",__LINE__, false);
      }else{
        if(!onTileStart())
           return code_return("start CDP tail fail, ",__LINE__, false);
      }
    }else if(fScalerMode == SCALER_MODE_FRAME){
      //if(!allocIOMEM())
      //  fail_return("Image Scaler allocate IOMEM fail");
      if( !onFrameStart())
       return fail_return("Image Scaler config frame fail");

      if(!onCdpWaitIrq())
        return code_return("wait CDP fail, ",__LINE__, false);
#if 0 //def JPEG_DEBUG_DRAW_LINE
      JPG_DBG("ReSizer::[DRAW_LINE] draw_line [%d %d] in range addr %x %d %d %d", fOutWidth, draw_num, (unsigned int)fDstBufAddr, fOutWidth, fOutHeight, fDstFormat);
      memset((unsigned char *)(fDstBufAddr+draw_base),0xCD,draw_num*fOutWidth*pixel_byte[fDstFormat]);
#endif

    }else{
       JPG_ERR("Unknow Scaler Operation Mode %d!!\n ", fScalerMode);
       return false;
    }

    return true;
}


bool ImgCdpPipe::Wait()
{
   return true;
}
