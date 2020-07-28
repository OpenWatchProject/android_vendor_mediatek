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

#include "SwType.h"
#include "SwStream.h"
#include "SwJpegUtility.h"
#include <stdlib.h>

#define ATRACE_TAG ATRACE_TAG_GRAPHICS
#include "utils/Trace.h"


#include <stdio.h>
extern "C" {
    #include "jpeglib_alpha.h"
    #include "jerror_alpha.h"
}
#if 0
#include <setjmp.h>

/* Our error-handling struct.
 *
*/
struct swjpeg_error_mgr : jpeg_error_mgr {
    jmp_buf fJmpBuf;
};
#endif

//#include "jpeg_dec_hal.h"
#include "jpeg_sw_dec.h"

#include "img_dec_comp.h"   

//#define JPEG_PROFILING   

#define JPEG_IRQ_TIMEOUT_DEC 5000  /*3000*/ 

#define JPEG_SW_SUPPORT_YUV444

static bool fail_return(const char msg[]) {
#if 1
    JPG_ERR("[JPEG Decoder] - %s", msg);
#endif 
    return false;   // must always return false
}


static int code_return(const char msg[], int line,int code) {
#if 1
    JPG_ERR("[JpegComp] - %s, L:%d!!\n", msg, line);
#endif
    return code;   // must always return code
}

//#define CHECK_FAIL(check, action)   
//{
//   if(!check)
//     action;   
//}


JpgSwDec::JpgSwDec()
{
    islock = false;
    fDecType = IMG_DEC_MODE_NONE ;
    fJpgWidth = fJpgHeight = 0;
    fOutWidth = fOutHeight = 0;
    fBRZ = 0;

    fMinLumaBufSize = 0;
    fMinCbCrBufSize = 0;
    fMinLumaBufStride = 0;
    fMinCbCrBufStride = 0;

    fLumaImgStride = 0;
    fCbCrImgStride = 0;
    fLumaImgHeight = 0;
    fCbCrImgHeight = 0;

    fFirstDecRowNum = 1;
    fDecRowCnt = 0;
    fOutFormat = IMGDEC_3PLANE_NONE ;

    fSrcAddr = fDstAddr = NULL;
    fSrcBufferSize = 0;
    fSrcBsSize = 0;
    fDstTileBufRowNum =  0;

    fMcuRow = 0;
    fMcuHeightY = 0;
    fMcuHeightC = 0;

    fMemType = fMemTypeDefault = IMG_MEM_TYPE_PHY ;

    cinfo = new jpeg_decompress_struct_ALPHA;
    memset(cinfo, 0, sizeof(jpeg_decompress_struct_ALPHA));

    jpegStream = NULL ;
    srcStream = NULL ;
    fQuality = 0;
    rowptr = NULL;
    bpr = 0;
    fImgFormat = IMG_COLOR_NONE ;

}

   
JpgSwDec::~JpgSwDec()
{
    unlock();
}

//void JpgSwDec::setRangeDecode(JUINT32 left, JUINT32 top, JUINT32 right, JUINT32 bottom)
//{
//    isRangeDecode = true;
//    fLeft = left;
//    fTop = top;
//    fRight = right;
//    fBottom = bottom;
//}
 
bool JpgSwDec::lock()
{

    create();
    islock = true;
    return true;
}

bool JpgSwDec::unlock()
{
    if(islock)
    {   
#if 1
    // All objects need to be instantiated before this setjmp call so that
    // they will be cleaned up properly if an error occurs.
    if (setjmp(sw_err.fJmpBuf)) {
        return code_return(" swJpeg return fail ",__LINE__, false);//return_false(cinfo, *bm, "setjmp");
    }
#endif 

        if(srcStream){
          delete srcStream;
          srcStream = NULL;
        }

        if(jpegStream){
          delete jpegStream;
          jpegStream = NULL;
        }
        if(rowptr){
          free(rowptr) ;
          rowptr = NULL ;
        }
        if (cinfo){
          jpeg_destroy_decompress_ALPHA(cinfo);
          delete cinfo;
          cinfo = NULL;
        }

        islock = false;
    }

    return true;
}



bool JpgSwDec::checkParam()
{
    if(fJpgWidth == 0 || fJpgHeight == 0)
    {
        JPG_ERR("Invalid JPEG width/height %u/%u", fJpgWidth, fJpgHeight);
        return false;
    }

#if 0    
    if(fOutWidth < 3 || fOutHeight < 3)
    {
        JPG_ERR("Invalid width/height %u/%u [Range 3~4095]", fOutWidth, fOutHeight);
        return false;
    }

    if(fOutWidth > 4095 || fOutHeight > 4095)
    {
        JPG_ERR("Invalid width/height %u/%u [Range 3~4095]", fOutWidth, fOutHeight);
        return false;
    }

    if(fSrcAddr == NULL || fDstAddr == NULL) 
    {
        return fail_return("Invalid Address");
    }
#endif
    return true;
}

bool JpgSwDec::create()
{
    ATRACE_CALL();

    /* Initialize the JPEG decompression object with default error handling. */
    //cinfo.err = jpeg_std_error(&jerr);
    cinfo->err = jpeg_std_error_ALPHA(&sw_err);
    sw_err.error_exit = swjpeg_error_exit;

#if 1
    // All objects need to be instantiated before this setjmp call so that
    // they will be cleaned up properly if an error occurs.
    if (setjmp(sw_err.fJmpBuf)) {
        return code_return(" swJpeg return fail ",__LINE__, false);//return_false(cinfo, *bm, "setjmp");
    }
#endif

    jpeg_create_decompress_ALPHA(cinfo);

    return true;   

}

bool JpgSwDec::parse()
{
    if(fSrcAddr == NULL)
    {
        return fail_return("source address is null");
    }
    ATRACE_CALL();

    srcStream = new SwBufStream(fSrcAddr, fSrcBufferSize);   
    jpegStream = new swjpeg_source_mgr(srcStream, false);    
    
    cinfo->src = jpegStream;

#if 1
    // All objects need to be instantiated before this setjmp call so that
    // they will be cleaned up properly if an error occurs.
    if (setjmp(sw_err.fJmpBuf)) {
        return code_return(" swJpeg return fail ",__LINE__, false);//return_false(cinfo, *bm, "setjmp");
    }
#endif

    int status = jpeg_read_header_ALPHA(cinfo, true);
    if (status != JPEG_HEADER_OK_ALPHA) {
        return code_return(" swJpeg parse invalid file ",__LINE__, false);//return_false(cinfo, *bm, "read_header");
    }

    fJpgWidth = cinfo->image_width;
    fJpgHeight = cinfo->image_height;
    fMcuRow = cinfo->total_iMCU_rows ;
    fMcuHeightY = cinfo->cur_comp_info[0]->v_samp_factor << 3;
    if(cinfo->comps_in_scan > 1)
      fMcuHeightC = cinfo->cur_comp_info[1]->v_samp_factor << 3;


    fLumaImgHeight = TO_CEIL(fJpgHeight, 2);
    if(!cinfo->progressive_mode){
      // we can only handle YUV planer color format
      if(cinfo->comps_in_scan == 3 && cinfo->jpeg_color_space == JCS_YCbCr_ALPHA){
        if(   cinfo->cur_comp_info[0]->h_samp_factor == 2 && cinfo->cur_comp_info[0]->v_samp_factor == 2
           && cinfo->cur_comp_info[1]->h_samp_factor == 1 && cinfo->cur_comp_info[1]->v_samp_factor == 1){
          fOutFormat = IMGDEC_3PLANE_YUV420 ;
          fImgFormat = IMG_COLOR_YUV420 ;

          fLumaImgStride    =  TO_CEIL( cinfo->cur_comp_info[0]->downsampled_width, 4);
          fCbCrImgStride    =  TO_CEIL( fLumaImgStride/2, 2);

          fCbCrImgHeight = TO_CEIL(fJpgHeight/2, 2);

        }else if(   cinfo->cur_comp_info[0]->h_samp_factor == 2 && cinfo->cur_comp_info[0]->v_samp_factor == 1
                 && cinfo->cur_comp_info[1]->h_samp_factor == 1 && cinfo->cur_comp_info[1]->v_samp_factor == 1){
          
          fOutFormat = IMGDEC_3PLANE_YUV422 ;
          fImgFormat = IMG_COLOR_YUV422 ;

          fLumaImgStride    =  TO_CEIL( cinfo->cur_comp_info[0]->downsampled_width, 4);
          fCbCrImgStride    =  TO_CEIL( fLumaImgStride/2, 2);
          fCbCrImgHeight = TO_CEIL(fJpgHeight, 2);
        }
#ifdef JPEG_SW_SUPPORT_YUV444           
        else if(   cinfo->cur_comp_info[0]->h_samp_factor == 1 && cinfo->cur_comp_info[0]->v_samp_factor == 1
                 && cinfo->cur_comp_info[1]->h_samp_factor == 1 && cinfo->cur_comp_info[1]->v_samp_factor == 1){
                  
          fOutFormat = IMGDEC_3PLANE_YUV444 ;
          fImgFormat = IMG_COLOR_YUV444 ;
          fLumaImgStride    =  TO_CEIL( cinfo->cur_comp_info[0]->downsampled_width, 2);  
          fCbCrImgStride    =  TO_CEIL( fLumaImgStride, 2);            
          fCbCrImgHeight = TO_CEIL(fJpgHeight, 2); 
        }       
#endif        
      }
#if 1 //def JPEG_SW_SUPPORT_GRAY     
      else if(cinfo->comps_in_scan == 1 ) {
        fLumaImgStride    =  TO_CEIL( cinfo->cur_comp_info[0]->downsampled_width, 2);  
        fCbCrImgStride    =  TO_CEIL( fLumaImgStride, 2);           
        fOutFormat = IMGDEC_3PLANE_GRAY ; 
        fImgFormat = IMG_COLOR_GRAY ;
        
        fCbCrImgHeight = TO_CEIL(fJpgHeight, 2);       
      }
#endif      
      else{       
        fImgFormat = IMG_COLOR_NO_SUPPORT ;  
      }
         
    } 

#if 0 // shrink log
    if(cinfo->comps_in_scan > 1 ){
      JPG_DBG("COMP_S_PARSE prog : %d Outfmt %d, w %d, h %d, row %d, Sample %d %d, %d %d, %d %d!!\n", 
      cinfo->progressive_mode, fOutFormat, cinfo->image_width, cinfo->image_height, fMcuRow
      , cinfo->cur_comp_info[0]->h_samp_factor, cinfo->cur_comp_info[0]->v_samp_factor
      , cinfo->cur_comp_info[1]->h_samp_factor, cinfo->cur_comp_info[1]->v_samp_factor
      , cinfo->cur_comp_info[2]->h_samp_factor, cinfo->cur_comp_info[2]->v_samp_factor);    
    }else if(cinfo->comps_in_scan == 1 ){
      JPG_DBG("COMP_S_PARSE prog : %d Outfmt %d, w %d, h %d, row %d, Sample %d %d!!\n", 
      cinfo->progressive_mode, fOutFormat, cinfo->image_width, cinfo->image_height, fMcuRow
      , cinfo->cur_comp_info[0]->h_samp_factor, cinfo->cur_comp_info[0]->v_samp_factor);         
    }
#endif

    if(fOutFormat == IMGDEC_3PLANE_NONE){
       return code_return(" swJpeg parse unsupport format file ",__LINE__, false);
    }
    
    return true;
}

bool JpgSwDec::Config(JUINT32 decMode)
{
    // unused params
	(void)decMode;

    ATRACE_CALL();

#if 1
    // All objects need to be instantiated before this setjmp call so that
    // they will be cleaned up properly if an error occurs.
    if (setjmp(sw_err.fJmpBuf)) {
        return code_return(" swJpeg return fail ",__LINE__, false);//return_false(cinfo, *bm, "setjmp");
    }
#endif  

    if(fQuality)
      cinfo->dct_method = JDCT_ISLOW_ALPHA;
    else
      cinfo->dct_method = JDCT_IFAST_ALPHA;

    cinfo->scale_num = 1;
    cinfo->scale_denom = 1<<fBRZ;//sampleSize;

    /* this gives about 30% performance improvement. In theory it may
       reduce the visual quality, in practice I'm not seeing a difference
     */
    cinfo->do_fancy_upsampling = 0;

    /* this gives another few percents */
    cinfo->do_block_smoothing = 0;

    /* default format is RGB */
    cinfo->out_color_space = JCS_RGB_ALPHA;

    cinfo->dither_mode = JDITHER_NONE_ALPHA;

    //if (config == SkBitmap::kARGB_8888_Config) {

    cinfo->out_color_space = JCS_EXT_RGBA_ALPHA;

    //} else if (config == SkBitmap::kRGB_565_Config) {
    //    cinfo.out_color_space = JCS_RGB565;
    //    if (this->getDitherImage()) {
    //        cinfo.dither_mode = JDITHER_ORDERED_ALPHA;
    //    }
    //}

    bpr = fJpgWidth *4;
    //rowptr = (unsigned char *)fDecOutBufAddr0[0] ;

    if (!jpeg_start_decompress_ALPHA(cinfo)) {
       return code_return(" swJpeg start decompress fail ",__LINE__, false);//return_false(cinfo, *bm, "start_decompress");
    }

#if 1

   if(fBRZ < 1){ //NO deciamtion: brz ==  0
      fMinLumaBufStride =  TO_CEIL( cinfo->cur_comp_info[0]->downsampled_width, cinfo->cur_comp_info[0]->h_samp_factor<<3);  //min_info.minLumaBufStride ;
      if(cinfo->comps_in_scan > 1 )
        fMinCbCrBufStride =  TO_CEIL( cinfo->cur_comp_info[1]->downsampled_width, cinfo->cur_comp_info[1]->h_samp_factor<<3);//min_info.minCbCrBufStride ;
      else
        fMinCbCrBufStride =  TO_CEIL( cinfo->cur_comp_info[0]->downsampled_width, 8);//min_info.minCbCrBufStride ;
   }else{  //3, 2, 1
      if(fImgFormat == IMG_COLOR_YUV420){  //regard as 444
        fMinLumaBufStride =  TO_CEIL( cinfo->cur_comp_info[0]->downsampled_width, (cinfo->cur_comp_info[0]->h_samp_factor-1)<<3);  
        fImgFormat = IMG_COLOR_YUV420 ;  //TBD
        fOutFormat = IMGDEC_3PLANE_YUV444 ;
      }else{
        fMinLumaBufStride =  TO_CEIL( cinfo->cur_comp_info[0]->downsampled_width, cinfo->cur_comp_info[0]->h_samp_factor<<3);  
      }

      if(cinfo->comps_in_scan > 1 )
        fMinCbCrBufStride =  TO_CEIL( cinfo->cur_comp_info[1]->downsampled_width, cinfo->cur_comp_info[1]->h_samp_factor<<3); 
      else
        fMinCbCrBufStride =  TO_CEIL( cinfo->cur_comp_info[0]->downsampled_width, 8);  
   }

#else

   fMinLumaBufStride =  TO_CEIL( cinfo.cur_comp_info[0]->downsampled_width, 2);//min_info.minLumaBufStride ;
   if(cinfo.comps_in_scan > 1 )
     fMinCbCrBufStride =  TO_CEIL( cinfo.cur_comp_info[1]->downsampled_width, 2);//min_info.minCbCrBufStride ;
   else
     fMinCbCrBufStride =  TO_CEIL( cinfo.cur_comp_info[0]->downsampled_width, 2);//min_info.minCbCrBufStride ;

#endif

   fMinLumaBufSize   =  fMinLumaBufStride*((cinfo->cur_comp_info[0]->v_samp_factor)*cinfo->cur_comp_info[0]->DCT_scaled_size); //min_info.minLumaBufSize   ;
   if(cinfo->comps_in_scan > 1 )
     fMinCbCrBufSize   =  fMinCbCrBufStride*(cinfo->cur_comp_info[1]->v_samp_factor*cinfo->cur_comp_info[1]->DCT_scaled_size); //min_info.minCbCrBufSize   ;
   else
     fMinCbCrBufSize   =  fMinCbCrBufStride*(cinfo->cur_comp_info[0]->v_samp_factor*cinfo->cur_comp_info[0]->DCT_scaled_size); //min_info.minCbCrBufSize   ;

    if(fBRZ > 0 && fBRZ < 4){//adjust with BRZ factor
      fLumaImgStride    =  fMinLumaBufStride;//TO_CEIL( cinfo.cur_comp_info[0]->downsampled_width, cinfo.cur_comp_info[0]->DCT_scaled_size);  
      fCbCrImgStride    =  fMinCbCrBufStride;//TO_CEIL( cinfo.cur_comp_info[1]->downsampled_width, cinfo.cur_comp_info[1]->DCT_scaled_size);
      fLumaImgHeight    =   cinfo->cur_comp_info[0]->downsampled_height ; 
      if(cinfo->comps_in_scan > 1)
        fCbCrImgHeight    =   cinfo->cur_comp_info[1]->downsampled_height ;

      fMcuHeightY = cinfo->cur_comp_info[0]->v_samp_factor * cinfo->cur_comp_info[0]->DCT_scaled_size;
      if(cinfo->comps_in_scan > 1)
        fMcuHeightC = cinfo->cur_comp_info[1]->v_samp_factor * cinfo->cur_comp_info[1]->DCT_scaled_size;

    }

   return true ;
}


JUINT32 JpgSwDec::ResumeDecRow(JUINT32 decRowNum)
{

   return code_return(" swJpeg return fail ",__LINE__, false);

#if 0	//due to libjpeg-alpha NOT support
   JUINT32 ret ;
   unsigned int i=0, j=0;
   unsigned int LumaMcuHeight = (cinfo->cur_comp_info[0]->v_samp_factor<<(3-fBRZ)) ;
   int rowlineNo= decRowNum*LumaMcuHeight ;
   struct timeval t1, t2;
   int row_count = 1;
   int ci = 0;
   int rowi = 0;
    ATRACE_CALL();

#ifdef JPEG_LOG_TIME
   gettimeofday(&t1, NULL);
#endif
   
#if 1
    // All objects need to be instantiated before this setjmp call so that
    // they will be cleaned up properly if an error occurs.
    if (setjmp(sw_err.fJmpBuf)) {
        return code_return(" swJpeg return fail ",__LINE__, false);//return_false(cinfo, *bm, "setjmp");
    }
#endif  

    for ( i=0; cinfo->output_scanline < cinfo->output_height && i<decRowNum; i++) 
    {
        for(ci=0; ci < cinfo->comps_in_scan ; ci++)
        {
            if((cinfo->input_iMCU_row % cinfo->idctBufRowNum) == 0)
                cinfo->idctBufRowAddrY0[ci][0] = cinfo->idctBufAddr[ci] ;
            else
                cinfo->idctBufRowAddrY0[ci][0] += cinfo->idctBufRowSize[ci] ;
            
            if(fImgFormat == IMG_COLOR_YUV420) 
                cinfo->idctBufRowAddrY1[ci][0] = cinfo->idctBufRowAddrY0[ci][0] + (cinfo->idctBufRowSize[ci] >> 1) ;

            for(rowi = 1; rowi < cinfo->cur_comp_info[ci]->DCT_scaled_size ;rowi++)
            {
                cinfo->idctBufRowAddrY0[ci][rowi] = cinfo->idctBufRowAddrY0[ci][rowi-1] + cinfo->idctBufStride[ci] ;
                if(fImgFormat == IMG_COLOR_YUV420) 
                    cinfo->idctBufRowAddrY1[ci][rowi] = cinfo->idctBufRowAddrY0[ci][rowi] + (cinfo->idctBufRowSize[ci] >> 1) ;
            }
        
          //for(rowi = 0; rowi < cinfo.cur_comp_info[ci]->DCT_scaled_size ;rowi++)
          //  JPG_DBG("JSD: [ROWADDR][%d] BufAddrY0[%d] %x, BufAddrY1[%d] %x, MCU %d!!\n", ci,rowi,cinfo.idctBufRowAddrY0[i][rowi],rowi,cinfo.idctBufRowAddrY1[i][rowi], cinfo.input_iMCU_row );
        }

        for( j=0; cinfo->output_scanline < cinfo->output_height && j < LumaMcuHeight;j++ ){

#if 0
         if(j==0){ //first line of MCU
           //JPG_DBG("decoder resume MCU_B %d/%d(%d/%d line) row to %d, rowptr %x, bpr %d!!\n", i, decRowNum, j,rowlineNo, fDecRowCnt, (unsigned int)rowptr, (unsigned int)bpr);    
           gettimeofday(&t1, NULL);   
         }else if(j==1){ //first line of MCU
           gettimeofday(&t2, NULL);
           //JPG_LOG("Jpeg HW config m4u time : %u", (t2.tv_sec - t1.tv_sec)*1000000 + (t2.tv_usec - t1.tv_usec));           
           JPG_DBG("decoder resume MCU:(%d/%d), LINE:(%d/%d) row to %d, time %u, (%d/%d)!!\n", i, decRowNum, j-1,rowlineNo, fDecRowCnt, (t2.tv_sec - t1.tv_sec)*1000000 + (t2.tv_usec - t1.tv_usec), cinfo.output_scanline , cinfo.output_height);             
           gettimeofday(&t1, NULL);
         }           
#endif
         //JPG_DBG("decoder resume line:(%d/%d), LINE:(%d/%d) row to %d, (%d/%d)!!\n", i, decRowNum, j-1,rowlineNo, fDecRowCnt,  cinfo.output_scanline , cinfo.output_height);             

#if 0
         if(j==0){
	         android::Tracer::traceBegin(ATRACE_TAG_GRAPHICS,"DECROW");
         }
#endif

         row_count = jpeg_read_scanlines_ALPHA(cinfo, &rowptr, 1);

#if 0
         // if row_count == 0, then we didn't get a scanline, so abort.
         // if we supported partial images, we might return true in this case
         if(j==0){
	         android::Tracer::traceEnd(ATRACE_TAG_GRAPHICS);
         }
#endif
         if (0 == row_count) {
             return code_return(" swJpeg decode hit invlid file ",__LINE__, IMG_DEC_RST_DECODE_FAIL);//return_false(cinfo, *bm, "read_scanlines");
         }
       }
#if 0
       gettimeofday(&t2, NULL);       
       JPG_DBG("decoder resume MCU:(%d/%d), LINE:(%d/%d) row to %d, time %u, (%d/%d)!!\n", i, decRowNum, j,rowlineNo, fDecRowCnt, (t2.tv_sec - t1.tv_sec)*1000000 + (t2.tv_usec - t1.tv_usec), cinfo.output_scanline , cinfo.output_height);             
#endif

#if 0
       if(cinfo.isUseMtk3plane)
         for(unsigned int i=0; i<3 ;i++)
           cinfo.upSampleBufAddr[i] += cinfo.dstBufStride[i] ;
#endif                          
        //rowptr += bpr;
    }  
#ifdef JPEG_LOG_TIME    
    gettimeofday(&t2, NULL);   
    JPG_DBG("decoder resume row %d+%d->%d(%d line), time %u, rowptr %x, bpr %d!!\n", fDecRowCnt, decRowNum, fDecRowCnt+decRowNum, rowlineNo
    ,(t2.tv_sec - t1.tv_sec)*1000000 + (t2.tv_usec - t1.tv_usec), (unsigned int)rowptr, (unsigned int)bpr);
#endif
    //JPG_DBG("jpeg decoder resume %d+%d->%d result:%d ", fDecRowCnt, decRowNum,fDecRowCnt+decRowNum, result);
    fDecRowCnt +=  decRowNum ;

   if( fDecRowCnt < fMcuRow )
      ret = IMG_DEC_RST_ROW_DONE ;
   else{
      ret = IMG_DEC_RST_DONE ;
      jpeg_finish_decompress_ALPHA(cinfo);
   }

   return ret ;
#endif
}

bool JpgSwDec::onStart()
{
		return fail_return("JPEG Decode Fail");

#if 0	//due to libjpeg-alpha NOT support
        unsigned int i = 0;
        unsigned char* upSampleBufBase[3];

        cinfo->isUseMtk3plane = 1;
        cinfo->isUseIdctBuffer = 1;
        cinfo->isUseUpsampleBuffer = 0;

        cinfo->idctBufStride[0] = fMinLumaBufStride;  //TO_CEIL(cinfo.cur_comp_info[0]->downsampled_width, 256) ;
        cinfo->idctBufStride[1] = fMinCbCrBufStride;  //TO_CEIL(cinfo.cur_comp_info[1]->downsampled_width, 256) ;
        cinfo->idctBufStride[2] = fMinCbCrBufStride;  //TO_CEIL(cinfo.cur_comp_info[2]->downsampled_width, 256) ;

        cinfo->idctBufRowSize[0] =  cinfo->idctBufStride[0] * cinfo->cur_comp_info[0]->v_samp_factor * cinfo->cur_comp_info[0]->DCT_scaled_size ;
        if(cinfo->comps_in_scan > 1){
        cinfo->idctBufRowSize[1] =  cinfo->idctBufStride[1] * cinfo->cur_comp_info[1]->v_samp_factor * cinfo->cur_comp_info[1]->DCT_scaled_size ;
        cinfo->idctBufRowSize[2] =  cinfo->idctBufStride[2] * cinfo->cur_comp_info[2]->v_samp_factor * cinfo->cur_comp_info[2]->DCT_scaled_size ; 
        }
        
        cinfo->idctBufRowNum = fDstTileBufRowNum ; // cinfo.total_iMCU_rows ;
        cinfo->idctBufHeight[0] = fDstTileBufRowNum * fMcuHeightY ;
        cinfo->idctBufHeight[1] = fDstTileBufRowNum * fMcuHeightC ; 
        cinfo->idctBufHeight[2] = fDstTileBufRowNum * fMcuHeightC ;
        
        cinfo->idctBufSize[0] = cinfo->idctBufStride[0] * cinfo->idctBufHeight[0];
        cinfo->idctBufSize[1] = cinfo->idctBufStride[1] * cinfo->idctBufHeight[1];
        cinfo->idctBufSize[2] = cinfo->idctBufStride[2] * cinfo->idctBufHeight[2];

        cinfo->idctBufAddr[0] = (unsigned char *) fDecOutBufAddr0[0] ;   //(unsigned char *) malloc(cinfo.idctBufSize[0]) ;
        cinfo->idctBufAddr[1] = (unsigned char *) fDecOutBufAddr0[1] ;   //(unsigned char *) malloc(cinfo.idctBufSize[1]) ;
        cinfo->idctBufAddr[2] = (unsigned char *) fDecOutBufAddr0[2] ;   //(unsigned char *) malloc(cinfo.idctBufSize[2]) ;

        cinfo->dstBufSize[0] = 
        cinfo->dstBufSize[1] = 
        cinfo->dstBufSize[2] = cinfo->output_width * cinfo->output_height ;
        cinfo->dstBufStride[0] = 
        cinfo->dstBufStride[1] = 
        cinfo->dstBufStride[2] = cinfo->output_width ;

        if(cinfo->isUseUpsampleBuffer){
          cinfo->upSampleBufAddr[0] = upSampleBufBase[0] = (unsigned char *) malloc(cinfo->dstBufSize[0]) ;
          cinfo->upSampleBufAddr[1] = upSampleBufBase[1] = (unsigned char *) malloc(cinfo->dstBufSize[1]) ;
          cinfo->upSampleBufAddr[2] = upSampleBufBase[2] = (unsigned char *) malloc(cinfo->dstBufSize[2]) ;
        }
        if( cinfo->upSampleBufAddr[0] == NULL || cinfo->upSampleBufAddr[1] == NULL || cinfo->upSampleBufAddr[2] == NULL)
          cinfo->isUseUpsampleBuffer = 0; //cinfo.isUseMtk3plane = 0;
        

    //for(i=0;i<3;i++)
    //  JPG_DBG("JSD: [CONFIG][%d] BufAddr %x, Stride %d, BufH %d, size %x!!\n", i,cinfo->idctBufAddr[i], cinfo->idctBufStride[i], cinfo->idctBufHeight[i],cinfo->idctBufSize[i]);

    srcStream->setBufStream(fSrcAddr, fSrcBufferSize);   

    rowptr = (unsigned char *)malloc(bpr) ;
   
    if(!ResumeDecRow(1))
        return fail_return("JPEG Decode Fail");      
   
    if(fFirstDecRowNum == 0) fFirstDecRowNum = 1 ;      
    fDecRowCnt = fFirstDecRowNum ;
   
    return true;
#endif
}

JUINT32 JpgSwDec::Start()
{
    ATRACE_CALL();
    //Check param
    if(true != checkParam()) {
        return IMG_DEC_RST_CFG_ERR;
    }

    //JPG_DBG("SwDecoder Src Addr:%p, width/height:[%u, %u], Quality %d, BRZ %d!!", fSrcAddr, fJpgWidth, fJpgHeight, fQuality, fBRZ);
    //JPG_DBG("SwDecoder Dst Addr:%p, width/height:[%u, %u], format:%u", fDstAddr, fOutWidth, fOutHeight, fOutFormat);

    if(true == onStart()) {
       
    } else {
        
        return IMG_DEC_RST_DECODE_FAIL;
    }
    
    return IMG_DEC_RST_DONE;
}

bool JpgSwDec::alloc_pmem()
{         
      return true;
}

bool JpgSwDec::alloc_m4u()
{
   return true;
}

bool JpgSwDec::free_m4u()
{
   
   return true; 
}   

