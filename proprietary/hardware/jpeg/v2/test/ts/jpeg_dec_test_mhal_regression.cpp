/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#include <stdio.h>
#include <cutils/log.h>
#include <cutils/pmem.h>
//#include <cutils/memutil.h>

#include <MediaHal.h>

#include <mhal_jpeg.h>

#include "jpeg_hal_dec.h"

#define xlog(...) \
        do { \
            LOGD(__VA_ARGS__); \
        } while (0)

#undef LOG_TAG
#define LOG_TAG "jpeg_test"

extern unsigned char jpg_file[];

//#define BUFFER_SIZE 739
#define BUFFER_SIZE (1920*1080*4)

#undef xlog
#define xlog(...) printf(__VA_ARGS__)


#define TO_CEIL(x,a) ( ((unsigned int)(x) + ((a)-1)) & ~((a)-1) )

#define IMGDEC_ALLOC(ptr, size, align, align_ptr) \
{  \
   ptr = (unsigned char *)malloc(size+align) ; \
   align_ptr = (unsigned char *)TO_CEIL(ptr, align);    \
   memset(ptr, 0x0 , size+align); \
}



int main(int argc, char *argv[])
{
    FILE *fp;
    int src_fd;
    unsigned long index;
    unsigned char *src_va;
    unsigned char *src_va_org;

    unsigned int    colorBufferPA[3];
    unsigned char*  colorBufferVA[3];
    int             colorBufferFD[3];
    unsigned int    bufferSizeArray[3];    

    unsigned int dst_size, dst_size1 ;  
    unsigned int dst_stride, dst_stride1 ;
    unsigned int img_stride, img_stride1 ;
    unsigned char *dst_buffer_org = NULL; 
    unsigned char *dst_buffer = NULL; 
    unsigned char *dst_buffer1 = NULL; 
    unsigned char *dst_buffer2 = NULL;
    unsigned int testTile = 1;
    unsigned int tileRowNum = 0;
    MHAL_JPEG_DEC_START_IN inParams ;
    MHAL_JPEG_DEC_INFO_OUT decOutInfo ;
    void* procHandler = NULL;

    int idx = 0;
    int test_no = 10;
    int test_base = 0;
    int i = 0;
    int run = 0;
    int success = 0;
    int run_fail = 0;
    int parse_err = 0;
    int skip = 0;
    
    char bsFile[128];   
    char outFile[128];
    
    unsigned int dst_w = 100;//501;//TO_CEIL(w, 4);      //w;//200;//
    unsigned int dst_h = 100;//501;//TO_CEIL(h, 4);    //h;//200;//

    if(argc < 3)
      return 0;

    dst_size    = dst_w*dst_h*4 ;             

    IMGDEC_ALLOC(src_va_org, BUFFER_SIZE, 64, src_va) ;

    IMGDEC_ALLOC(dst_buffer_org, dst_size, 64, dst_buffer) ;

      xlog("JpegTest:: test %d from %d seqs(arg %d) !!\n",atoi(argv[2]), atoi(argv[1]), argc  );
    
    test_base = atoi(argv[1]) ;
    test_no = atoi(argv[2]) ;

    idx = test_base;
    for( i =0; i< (test_no) ;i++, idx++){
       
       sprintf(bsFile , "/data/otis/seq/big/test_big_%04d.jpg", idx);

       sprintf(outFile, "/data/otis/out/test_out_%d_%d_%04d.raw", dst_w, dst_h,idx);
              
       
       printf("JpegTest:: testing %s->%s , L:%d !!\n", bsFile, outFile,__LINE__);  
       
       
       //src_va = (unsigned char *)(unsigned char *)malloc(BUFFER_SIZE); //pmem_alloc(BUFFER_SIZE , &src_fd);
       
       memset(&inParams  , 0, sizeof(MHAL_JPEG_DEC_START_IN));
       memset(&decOutInfo, 0, sizeof(MHAL_JPEG_DEC_INFO_OUT));
       
       
       if(src_va == NULL)
       {
           xlog("Can not allocate memory\n");
           return -1;
       }
       fp = fopen(bsFile, "r");
       if(fp == NULL){
         skip ++;
         continue ;
       }
       fread(src_va , 1 , BUFFER_SIZE , fp);
       fclose(fp);
       
       xlog("JpegTest::[%d] src_buffer : %x, size %x !!\n", idx,(unsigned int)src_va, BUFFER_SIZE);
       
       if( MHAL_NO_ERROR != mHalJpgDecParser(src_va, BUFFER_SIZE)){
         parse_err++;
         xlog("JpegTest::[%d] error[%d] unsupport format !!\n", idx, parse_err);
         continue ;
       }
       
       mHalJpgDecGetInfo(&decOutInfo);
       
       
       
       
       unsigned int w = decOutInfo.srcWidth;
       unsigned int h = decOutInfo.srcHeight;

       memset(dst_buffer, 0xAA , dst_size);

       
       xlog("JpegTest::[%d] %d %d -> %d %d, out: %x \n", idx, w, h, dst_w, dst_h ,(unsigned int)dst_buffer);
       
       if(w ==0 || h == 0){
         parse_err++;
         xlog("JpegTest::[%d] error[%d] get zero size %d %d!!\n", idx, parse_err, w, h);
         continue ;
      }
       
       inParams.dstFormat = JPEG_OUT_FORMAT_ARGB8888; //JPEG_OUT_FORMAT_YUY2 ;//
       
       inParams.srcBuffer = src_va;
       inParams.srcLength= BUFFER_SIZE;
       //inParams.srcFD = srcFD;
       
       inParams.dstWidth = dst_w;
       inParams.dstHeight = dst_h;
       inParams.dstVirAddr = (UINT8*) dst_buffer;
       inParams.dstPhysAddr = NULL;
       
       inParams.doDithering = 0;//getDitherImage() ? 1 : 0;
       inParams.doRangeDecode = 0;    
       
       
       
       
       //xlog("[test] file width/height: %d %d->%d %d!!\n", w, h, dst_w, dst_h);
       
       run++;
       if(MHAL_NO_ERROR == mHalJpgDecStart(&inParams, procHandler))
       {
           FILE *fp;
           fp = fopen(outFile, "w+");
           unsigned char* cptr ;        
       
           cptr = dst_buffer ;
           success ++;
           xlog("JpegTest::RESULT [%d] SUCCESS cnt %d!! dump : %s!!\n", idx,success, outFile);
           
           for( int i=0;i<dst_size;i++){  /* total size in comp */
             fprintf(fp,"%c", *cptr );  
             cptr++;
           }        
           fclose(fp);
           
       }
       else
       { 
         run_fail++;
           xlog("JpegTest::RESULT [%d] FAIL !!\n", idx);
       }
       xlog("JpegTest::TOTAL [%d] RUN: %d, Success %d, Fail %d, parse_fail %d, skip %d !! dump : %s!!\n", idx, run, success, run_fail, parse_err ,skip);
       
       
    }

    return 0;
}

