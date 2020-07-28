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

#define MTK_LOG_ENABLE 1
#include <stdio.h>
//#include <jpeglib_alpha.h>
extern "C" {
    #include "jpeglib_alpha.h"
    #include "jerror_alpha.h"
   }  
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
#define BUFFER_SIZE (6*1024*1024)

#undef xlog
#define xlog(...) printf(__VA_ARGS__)


#define TO_CEIL(x,a) ( ((unsigned int)(x) + ((a)-1)) & ~((a)-1) )

#define IMGDEC_ALLOC(ptr, size, align, align_ptr) \
{  \
   ptr = (unsigned char *)malloc(size+align) ; \
   align_ptr = (unsigned char *)TO_CEIL(ptr, align);    \
   memset(ptr, 0x0 , size+align); \
}

#define MAX_REG_FILE_NUM 512


void write_JPEG_file (char * filename, int quality, unsigned int image_width, unsigned int image_height, unsigned char image_buffer[] )
{
   
   unsigned long fDstSize ;
   unsigned char* cptr ;//= (unsigned char* )image_buffer;    
   unsigned char* cptr_base ;
   unsigned char* cptr_base_org ;
   
   //cptr_base_org = cptr_base = cptr = (unsigned char*) malloc(image_width*image_width*4);
   
  /* This struct contains the JPEG compression parameters and pointers to
   * working space (which is allocated as needed by the JPEG library).
   * It is possible to have several such structures, representing multiple
   * compression/decompression processes, in existence at once.  We refer
   * to any one struct (and its associated working data) as a "JPEG object".
   */
  struct jpeg_compress_struct cinfo;
  /* This struct represents a JPEG error handler.  It is declared separately
   * because applications often want to supply a specialized error handler
   * (see the second half of this file for an example).  But here we just
   * take the easy way out and use the standard error handler, which will
   * print a message on stderr and call exit() if compression fails.
   * Note that this struct must live as long as the main JPEG parameter
   * struct, to avoid dangling-pointer problems.
   */
  struct jpeg_error_mgr jerr;
  /* More stuff */
  FILE * outfile;		/* target file */
  JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
  int row_stride;		/* physical row width in image buffer */

  /* Step 1: allocate and initialize JPEG compression object */

  /* We have to set up the error handler first, in case the initialization
   * step fails.  (Unlikely, but it could happen if you are out of memory.)
   * This routine fills in the contents of struct jerr, and returns jerr's
   * address which we place into the link field in cinfo.
   */
  cinfo.err = jpeg_std_error(&jerr);
  /* Now we can initialize the JPEG compression object. */
  jpeg_create_compress(&cinfo);

  /* Step 2: specify data destination (eg, a file) */
  /* Note: steps 2 and 3 can be done in either order. */

  /* Here we use the library-supplied code to send compressed data to a
   * stdio stream.  You can also write your own code to do something else.
   * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
   * requires it in order to write binary files.
   */
  if ((outfile = fopen(filename, "w")) == NULL) {
    fprintf(stderr, "can't open %s\n", filename);
    exit(1);
  }
  /* Step 3: set parameters for compression */

  /* First we supply a description of the input image.
   * Four fields of the cinfo struct must be filled in:
   */
  cinfo.image_width = image_width; 	/* image width and height, in pixels */
  cinfo.image_height = image_height;
  cinfo.input_components = 3;		/* # of color components per pixel */
  cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
  cinfo.input_gamma = 1;
  /* Now use the library's routine to set default compression parameters.
   * (You must set at least cinfo.in_color_space before calling this,
   * since the defaults depend on the source color space.)
   */
  jpeg_set_defaults(&cinfo);
  /* Now you can set any non-default parameters you wish to.
   * Here we just illustrate the use of quality (quantization table) scaling:
   */
  jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);
  
  cinfo.dct_method = JDCT_IFAST;

  jpeg_stdio_dest(&cinfo, outfile);
  //jpeg_mem_dest(&cinfo, (unsigned char **)&cptr, (unsigned long *)&fDstSize);

  /* Step 4: Start compressor */

  /* TRUE ensures that we will write a complete interchange-JPEG file.
   * Pass TRUE unless you are very sure of what you're doing.
   */
  jpeg_start_compress(&cinfo, TRUE);

  /* Step 5: while (scan lines remain to be written) */
  /*           jpeg_write_scanlines(...); */

  /* Here we use the library's state variable cinfo.next_scanline as the
   * loop counter, so that we don't have to keep track ourselves.
   * To keep things simple, we pass one scanline per call; you can pass
   * more if you wish, though.
   */
  row_stride = image_width * 3;	/* JSAMPLEs per row in image_buffer */

  while (cinfo.next_scanline < cinfo.image_height) {
    /* jpeg_write_scanlines expects an array of pointers to scanlines.
     * Here the array is only one element long, but you could pass
     * more than one scanline at a time if that's more convenient.
     */
    row_pointer[0] = & image_buffer[cinfo.next_scanline * row_stride];
    (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }

  /* Step 6: Finish compression */

  jpeg_finish_compress(&cinfo);

  /* Step 7: release JPEG compression object */

  /* This is an important step since it will release a good deal of memory. */
  jpeg_destroy_compress(&cinfo);

  /* After finish_compress, we can close the output file. */
  //{    
  //  for( int i=0;i<(unsigned int)fDstSize;i++){  /* total size in comp */
  //    fprintf(outfile,"%c", *cptr_base );  
  //    cptr_base++;
  //  }
  //}   
  //free(cptr_base_org) ;
  
  fclose(outfile);
  /* And we're done! */
}

unsigned int compare_diff(unsigned char * src, unsigned char* golden, unsigned int size, unsigned int max_limit)
{
   unsigned int rst = 1;
   unsigned char *sPtr = src;
   unsigned char *gPtr = golden;
   int a, b, c, total = 0;
   
   unsigned int i = 0; 
   
   for(i = 0; i < size; i++){
      a = *sPtr ;
      b = *gPtr ;
      c = abs(a-b) ;
      total += c ;
      sPtr++;
      gPtr++;
   }
   
   if(total <= (max_limit*size))
     rst = 0;
     
   ALOGW("JpegTest:: compare_diff rst %d, %d pixel, total %d, limit %d !!\n", rst, size/3,total, max_limit*size );
    xlog("JpegTest:: compare_diff rst %d, %d pixel, total %d, limit %d !!\n", rst, size/3,total, max_limit*size );
   
   return rst ;
}

int main(int argc, char *argv[])
{
    FILE *fp;
    
    FILE *pRegFileList = NULL ;
    int src_fd;
    unsigned long index;
    unsigned char *src_va;
    unsigned char *src_va_org = NULL;

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
    unsigned int dumpFlag = 0 ;
    
    
    //regression test param
    char regSrcFilePath[256] ;
    char regNxtSrcFilePath[256] ;
    char regFolder0[256] = "Q0L0";
    char regFolder1[256] = "normal"; //"Q0L3";
    char regFolder2[256] = "Q4L3";
    //char regWorkBase[256] = "D:\\work\\project\\MT6593\\jpeg\\rdpattern";
    char regWorkBase[256] = "//data//otis//pattern";
    
    char regListFilePath[256] ;
    char *regFileFolder[MAX_REG_FILE_NUM][128];
    unsigned int test_base , test_no ;
    char regWorkPath[256] ;
    unsigned int i;
    unsigned int regTestNum = 0;
    unsigned int idx = 0;

    char regRstBase[256] = "//data//otis//output";
    char regRstFilePath[256] ;
    unsigned int test_rst = 0 ;
        
        
   
    printf("go %d !!\n", __LINE__);  

    if(argc < 3)
      return 0;
    
    ALOGW("JpegTest:: test %d from %d seqs(arg %d) !!\n",atoi(argv[2]), atoi(argv[1]), argc  );
    xlog("JpegTest:: test %d from %d seqs(arg %d) !!\n",atoi(argv[2]), atoi(argv[1]), argc  );

    test_base = atoi(argv[1]) ;
    test_no = atoi(argv[2]) ;



     //if(testFolderCnt == 0){
       sprintf(regWorkPath,"%s//%s", regWorkBase, regFolder1);
     //  regQFolder = (char *)regFolder1 ;
     //}else if(testFolderCnt == 1){
     //  sprintf(regWorkPath,"%s\\%s", regWorkBase, regFolder2);
     //  regQFolder = (char *)regFolder2 ;
     //}else if(testFolderCnt == 2){
     //  sprintf(regWorkPath,"%s\\%s", regWorkBase, regFolder0);
     //  regQFolder = (char *)regFolder0 ;
     //}
     
     sprintf(regListFilePath,"%s//regFileList", regWorkPath);
    
     {
        int getEOF = 0;
        regTestNum = 0;
        pRegFileList = fopen(regListFilePath, "r");        //fopen("D:\\work\\project\\MT6593\\jpeg\\rdpattern\\Q0L0\\regFileList", "r");        
        
         printf("open %s, L:%d !!\n", regListFilePath,__LINE__);  
        if(pRegFileList == NULL){
         return 0 ;
        }
        
        for( i = 0; i < MAX_REG_FILE_NUM && !getEOF ;i++){
          fscanf( pRegFileList, "%s\n",(char *)&regFileFolder[i][0] );
          if( !strcmp((const char *)&regFileFolder[i][0], "EOF") ){
           getEOF = 1;
           regTestNum = i ;
          }
        }
        
        printf("test: _%s, _%s, _%s\n", &regFileFolder[0][0], &regFileFolder[1][0], &regFileFolder[2][0]);
        
        fclose(pRegFileList) ;
        
        //return 0;
        
     }
    
     if(test_no != 0 && (test_base + test_no) < regTestNum){
        ;
     }else{
       test_no = regTestNum - test_base ;
     }


    
    
    for( i =0, idx = test_base; i< (test_no) ;i++, idx++){
          
      //src_va = (unsigned char *)(unsigned char *)malloc(BUFFER_SIZE); //pmem_alloc(BUFFER_SIZE , &src_fd);
      
      memset(&inParams  , 0, sizeof(MHAL_JPEG_DEC_START_IN));
      memset(&decOutInfo, 0, sizeof(MHAL_JPEG_DEC_INFO_OUT));
      
      sprintf(regSrcFilePath   ,"%s//%s", regWorkPath,&regFileFolder[idx][0]);
      if((idx)<regTestNum)
        sprintf(regNxtSrcFilePath,"%s//%s", regWorkPath,&regFileFolder[idx+1][0]);
      else
        sprintf(regNxtSrcFilePath,"%s//%s", regWorkPath,&regFileFolder[idx][0]);
      
      printf("test : %s!!\n", regSrcFilePath);
      
      IMGDEC_ALLOC(src_va_org, BUFFER_SIZE, 64, src_va) ;
      if(src_va == NULL)
      {
          xlog("Can not allocate memory\n");
          return -1;
      }
      //fp = fopen("//data/otis//test.jpg", "r");
      ALOGW("[test] wait for %s!!\n", regSrcFilePath);
      do{
        fp = fopen(regNxtSrcFilePath, "r");
      }while(fp == NULL);      
      fclose(fp); fp = NULL;
      ALOGW("[test] wait %s done !!\n", regSrcFilePath);
      
      do{
        fp = fopen(regSrcFilePath, "r");
      }while(fp == NULL);
      fread(src_va , 1 , BUFFER_SIZE , fp);
      fclose(fp); fp = NULL;
      
      ALOGW("[test] start test %s !!\n", regSrcFilePath);
      xlog("[test] start parse %s !!\n", regSrcFilePath);         
      
      //xlog("[test] src_buffer : %x, size %x !!\n", (unsigned int)src_va, BUFFER_SIZE);
      
      if( JPEG_ERROR_INVALID_DRIVER == mHalJpgDecParser(src_va, BUFFER_SIZE, 0, MHAL_IMAGE_TYPE_JPEG) )
      {
        ALOGW("[test] skip %s !!\n", regSrcFilePath);
        xlog("[test] skip %s !!\n", regSrcFilePath);
        continue ;      
      }
      
      mHalJpgDecGetInfo(&decOutInfo);
      
      
      
      
      unsigned int w = decOutInfo.srcWidth;
      unsigned int h = decOutInfo.srcHeight;
      unsigned int dst_w = 200;//640;//640;//640;//640;//501;//TO_CEIL(w, 4);      //w;//200;//
      unsigned int dst_h = 200;//100;//1000;//200;//480;//501;//TO_CEIL(h, 4);    //h;//200;//   
      
      
      
      //inParams.dstFormat = JPEG_OUT_FORMAT_ARGB8888;
      inParams.dstFormat = JPEG_OUT_FORMAT_RGB888; 

      dst_size    = dst_w*dst_h*3 ;  
      IMGDEC_ALLOC(dst_buffer_org, dst_size, 64, dst_buffer) ;
      memset(dst_buffer, 0xAA , dst_size);
      xlog("[test] out_buffer : %x \n", (unsigned int)dst_buffer);      
      
      inParams.srcBuffer = src_va;
      inParams.srcLength= BUFFER_SIZE;
      inParams.srcBufSize = BUFFER_SIZE;
      //inParams.srcFD = srcFD;
      
      inParams.dstWidth = dst_w;
      inParams.dstHeight = dst_h;
      inParams.dstVirAddr = (UINT8*) dst_buffer;
      inParams.dstPhysAddr = NULL;
      
      inParams.doDithering = 0;//getDitherImage() ? 1 : 0;
      inParams.doRangeDecode = 0;    
      
      
      
      ALOGW("[test] file width/height: %d %d->%d %d!!\n", w, h, dst_w, dst_h);
      xlog("[test] file width/height: %d %d->%d %d!!\n", w, h, dst_w, dst_h);
      
      
      if(MHAL_NO_ERROR == mHalJpgDecStart(&inParams, procHandler))
      {
        ALOGW("decode SUCCESS~~~\n");
        xlog("decode SUCCESS~~~\n");
        dumpFlag = 1;
        test_rst = 0 ;
      }
      else
      { 
        ALOGW("decode FAILED~~~\n");
        xlog("decode FAILED~~~\n");
        dumpFlag = 1 ;
        test_rst = 1 ;
      }
      
      #if 0
      {
         FILE *fp = NULL;
         unsigned char * golden_buffer = NULL;
         
         fp = fopen("//data//otis//golden.raw", "r");
         
         golden_buffer = (unsigned char*)malloc(dst_size);
        
         fread(golden_buffer , 1 , dst_size , fp);
         
         fclose(fp); 
         
         dumpFlag = compare_diff( dst_buffer, golden_buffer, dst_size, 0x03);
         dumpFlag = 1; 
         
         free(golden_buffer);
         
      }
      #endif
      
      
      
      
      if(dumpFlag){

          #if 0
            FILE *fp;
            fp = fopen("//data/otis//test_dec_pipe_out.raw", "w");
            unsigned char* cptr ;        
            
            cptr = dst_buffer ;
            xlog("[test] test_piep_dump : //data/otis//test_dec_pipe_out.raw!!\n");
            
            for( int i=0;i<dst_size;i++){  /* total size in comp */
              fprintf(fp,"%c", *cptr );  
              cptr++;
            }   
            fclose(fp);
          #endif      
          
          if(test_rst){
            sprintf(regRstFilePath,"%s//dump_%d_rst_fail_%s"   , regRstBase, idx, &regFileFolder[idx][0]);
          }else{
            sprintf(regRstFilePath,"%s//dump_%d_rst_success_%s", regRstBase, idx, &regFileFolder[idx][0]);
          }
          ALOGW("[test] test_rst_dump : %s!!\n", regRstFilePath); 
          xlog("[test] test_rst_dump : %s!!\n", regRstFilePath);
          write_JPEG_file(regRstFilePath , 100, dst_w, dst_h, (unsigned char *)dst_buffer);   
      } 
      if(dst_buffer_org)free(dst_buffer_org);
      if(src_va_org) free(src_va_org);
      
    }

    return 0;
}


