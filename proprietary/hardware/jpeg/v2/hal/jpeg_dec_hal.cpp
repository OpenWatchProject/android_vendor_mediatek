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

/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2009
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE. 
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
#define MTK_LOG_ENABLE 1
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
//========================

#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sched.h>
#include <fcntl.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <dirent.h>
#include <math.h>

#ifdef IMG_USE_PMEM
#include <cutils/pmem.h>
#endif
#include <log/log.h>

// Mutex for protecting lock/unlock API
#include "Mutex.h"
using namespace android;

#include "Trace.h"

//#define ATRACE_TAG ATRACE_TAG_GRAPHICS
//===========================

#include "jpeg_drv.h"

#include "jpeg_dec_hal.h"
#include "jpeg_dec_data.h"

// log and property related
#include <cutils/properties.h>
#include <cutils/log.h>
#include <utils/Errors.h>

#define JPEG_DRV_NAME "/proc/mtk_jpeg"
#define JPEG_HAL_ID_UNLOCK 0

static int currentID = JPEG_HAL_ID_UNLOCK;
static Mutex g_jpgDecHalMutex;
static JPEG_FILE_INFO j_info;

//static int sys_id;
//static unsigned int allocated;

static JPEG_DEC_CONFIG_CMDQ cmdqParams ;

//===============================================================================

//#define SKIP_JPEGDEC

#include "jpeg_dec_data.h"

//#include "gdma_drv.h"

#include "jpeg_drv.h"

#define JPEG_HEX_PATTERN

#ifdef  USB_DISK
    #define ROOT_PATH   "/mnt/udiska"
#else
    #define ROOT_PATH   "/system"
#endif

//#define TEST_GDMA_SWAP_UV

typedef enum
{
    ERR_DECODE_TIMEOUT = 1,
    ERR_FILE_NOT_FOUND,
    ERR_AC_FILE_NOT_FOUND,
    ERR_JPG_FILE_NOT_FOUND,
    ERR_BIT_TRUE_NOT_FOUND,
    ERR_CHKSUM_NOT_FOUND,    
    ERR_ALLOC_MEM_FAILED,
    ERR_PARSING,
    ERR_INIT_FAILED,
    ERR_CONFIG_FAILED,
    ERR_BIT_TRUE,
    ERR_CHECKSUM,
    ERR_CONFIG    
} DVT_ERR_CODE;

#define MAX(a,b) a > b ? a : b

//#define ALIGN_MASK(BIT)		(((unsigned int)(BIT) >> 3) - 1)
#define ALIGN_TO(value,align) ( (value + (align-1)) & (~(align-1)) )


void wait_for_ice()
{
    char key[256];
    printf("Wait for ICE, Press enter to conutine...");
    scanf("%s", key);
}


void wait_pr()
{
    unsigned int timeout1 = 0xF;
    unsigned int timeout2 = 0xFFFFF;    
    unsigned int timeout3 = 0xFFFFFFF;
    
    while (timeout1>0)
    {
        while (timeout2>0)
        {
            while (timeout3>0)
            {
                timeout3--;
            }
            timeout2--;
        }
        timeout1--;
    }
}

#ifdef FPGA_VERSION

int gdma_cmodel_compare(unsigned char* targetAddr[3], unsigned int targetSize[3],
    const char* jpegFilename, int numPlanes, int srcFormat, int tgtFormat, unsigned char scale[])
{
    char cmodelFile[3][100];

    sprintf(cmodelFile[0], "%s/jpeg_dec_cmodel/gdma_%s_[%d]_[%dx%d].y", ROOT_PATH, jpegFilename, srcFormat, tgtFormat, numPlanes);

    if (numPlanes == 2)
    {
        sprintf(cmodelFile[1], "%s/jpeg_dec_cmodel/gdma_%s_[%d]_[%dx%d].c", ROOT_PATH, jpegFilename, srcFormat, tgtFormat, numPlanes);
    }

    for (int i=0; i<numPlanes; i++)
    {
        if (targetSize[i] == 0)
            continue;
        
        printf("Compare with pattern: %s \n", cmodelFile[i]);
        wait_pr();

        FILE* fp = fopen(cmodelFile[i], "rb");
        if (fp == NULL)
        {
            printf("No C model pattern file: %s\n", cmodelFile[i]);
            return -ERR_BIT_TRUE_NOT_FOUND;
        }
        fseek(fp, 0L, SEEK_END);
        unsigned long filesize = ftell(fp);
        fseek(fp, 0L, SEEK_SET);

        if (filesize != targetSize[i])
        {
            printf("Warning! Pattern size %d is different from encoded size: %d\n", filesize, targetSize[i]);
            //return -ERR_BIT_TRUE;
        }

        unsigned char* cmodel = (unsigned char*)malloc(filesize);
        fread(cmodel, 1, filesize, fp);
        
        if (memcmp(targetAddr[i], cmodel, filesize))
        {
            free(cmodel);
            fclose(fp);
            return -ERR_BIT_TRUE;
        }
        free(cmodel);        
        fclose(fp);
    }

    return 0;
}


int dump_dispo_uyvy_to_yuv3p(unsigned char* targetAddr, unsigned int targetSize, const char* jpegFilename)
{
    char bsFile[100];   
    sprintf(bsFile, "/system/jpeg_dec_cmodel/dumpDispoPlayer_%s.yuv", jpegFilename);
    //sprintf(bsFile[0], "/system/jpeg_dec_cmodel/dump_%s.y", jpegFilename);
    //sprintf(bsFile[1], "/system/jpeg_dec_cmodel/dump_%s.cb", jpegFilename);
    //sprintf(bsFile[2], "/system/jpeg_dec_cmodel/dump_%s.cr", jpegFilename);
    unsigned char* cptr ;//= targetAddr[] ;
    unsigned int i, j ;
    
    FILE* fp = fopen(bsFile, "wb");
    if (fp == NULL)
    {
        printf("No C model pattern file: %s\n", bsFile);
        return 1;
    }   
    
    //uyvy_uyvy -> yuv
    cptr = targetAddr+1 ;
    for( i=0;i< targetSize/4 ;i++){  /* total size in comp */
      fprintf(fp,"%c", *cptr );  
      cptr+=2;
      fprintf(fp,"%c", *cptr );  
      cptr+=2;      
    }
    cptr = targetAddr ;    
    for( i=0;i< targetSize/4 ;i++){  /* total size in comp */
      fprintf(fp,"%c", *cptr );  
      cptr+=4;
    }
    cptr = targetAddr+2 ;    
    for( i=0;i< targetSize/4 ;i++){  /* total size in comp */
      fprintf(fp,"%c", *cptr );  
      cptr+=4;
    }    
    
    fclose(fp);
   
    return 0;   
}


int dump_dispo_yuv(unsigned char* targetAddr, unsigned int targetSize, const char* jpegFilename)
{
    char bsFile[100];   
    sprintf(bsFile, "/system/jpeg_dec_cmodel/dumpDispo_%s.yuv", jpegFilename);
    //sprintf(bsFile[0], "/system/jpeg_dec_cmodel/dump_%s.y", jpegFilename);
    //sprintf(bsFile[1], "/system/jpeg_dec_cmodel/dump_%s.cb", jpegFilename);
    //sprintf(bsFile[2], "/system/jpeg_dec_cmodel/dump_%s.cr", jpegFilename);
    unsigned char* cptr ;//= targetAddr[] ;
    unsigned int i, j ;

    FILE* fp = fopen(bsFile, "wb");
    if (fp == NULL)
    {
        printf("No C model pattern file: %s\n", bsFile);
        return 1;
    }   
    
    //for( j=0;j<3;j++)
    { /* runcomp 0~2 */
      cptr = targetAddr ;
      for( i=0;i<targetSize;i++){  /* total size in comp */
        fprintf(fp,"%c", *cptr );  
        cptr++;
      }
    }
    
    fclose(fp);
   
    return 0;   
}


int dump_yuv_rsz(JpegDrvDecConfig* config, unsigned char* targetAddr[3], unsigned int targetSize[3], const char* jpegFilename)
{
    char bsFile[100];   
    sprintf(bsFile, "/system/jpeg_dec_cmodel/dump_%s.yuv", jpegFilename);
    unsigned char* cptr ;//= targetAddr[] ;
    unsigned int i, j ;
    //unsigned int image_width[3]        =  {120, 60 , 60}  ;  //= {400, 200, 200}; //= {200, 100, 100};  //= {100, 50, 50};
    //unsigned int image_height[3]       =  {120, 120, 120}  ;  //=  {304, 152, 152};  //=  {152, 76, 76};  //= {76, 38, 38};
    unsigned int image_width[3]        ;
    unsigned int image_height[3]       ;    
    unsigned int image_align_width[3]  ;  //= {400, 208, 208}; //= {208, 112, 112};  //= {112, 64, 64};
    //unsigned int image_align_height[2] = { 76, 38};
    unsigned int dump_x, dump_y;
    unsigned int srcFormat ;
    //unsigned char yHScale, unsigned char yVScale, unsigned char cbcrHScale, unsigned char cbcrVScale
    
    for(i=0;i<3;i++){
      
      image_width[i]  = config->u4ds_width[i] ;
      image_height[i] = config->u4ds_height[i] ;
      
      image_align_width[i] = ALIGN_TO( image_width[i], 16 );
    }
    
    printf("DUMP_YUV: align %d %d %d!!\n", image_align_width[0], image_align_width[1], image_align_width[2]);

    FILE* fp = fopen(bsFile, "wb");
    if (fp == NULL)
    {
        printf("No C model pattern file: %s\n", bsFile);
        return 1;
    }   
     
    for( j=0;j<3;j++){ /* runcomp 0~2 */
      cptr = targetAddr[j] ;
      dump_x = dump_y = 0;
      for( i=0;i<targetSize[j];i++){  /* total size in comp */
        
        if( dump_x < image_width[j] && dump_y < image_height[j] )
          fprintf(fp,"%c", *cptr );  
        cptr++;
        if(++dump_x  == image_align_width[j]){
           dump_x = 0;
           dump_y++ ;
        }
         
      }
   }
    
    fclose(fp);
   
    return 0;   
}

int dump_yuv(unsigned char* targetAddr[3], unsigned int targetSize[3], const char* jpegFilename)
{
    char bsFile[100];   
    sprintf(bsFile, "/system/jpeg_dec_cmodel/dump_%s.yuv", jpegFilename);
    unsigned char* cptr ;//= targetAddr[] ;
    unsigned int i, j ;

    FILE* fp = fopen(bsFile, "wb");
    if (fp == NULL)
    {
        printf("No C model pattern file: %s\n", bsFile);
        return 1;
    }   
    
    for( j=0;j<3;j++){ /* runcomp 0~2 */
      cptr = targetAddr[j] ;
      for( i=0;i<targetSize[j];i++){  /* total size in comp */
        fprintf(fp,"%c", *cptr );  
        cptr++;
      }
   }
    
    fclose(fp);
   
    return 0;   
}


int cmodel_compare_regression(unsigned char* targetAddr[3], unsigned int targetSize[3], const char* jpegFilename, unsigned char scale[])
{
    char cmodelFile[128];
    int i ;
    
    unsigned int outputSize = targetSize[0] + targetSize[1] + targetSize[2] ;
    unsigned int diff =0;

#ifdef JPEG_HEX_PATTERN   

    sprintf(cmodelFile, "%s/auto/%s/yuv_golden.bin", ROOT_PATH,jpegFilename);
#else
    sprintf(cmodelFile, "%s/auto/%s/%s.yuv", ROOT_PATH,jpegFilename, jpegFilename);
#endif
    //if (targetSize[i] == 0)
    //    continue;
    
    printf("Compare with pattern: %s\n", cmodelFile);
    wait_pr();

    FILE* fp = fopen(cmodelFile, "rb");
    if (fp == NULL)
    {
        printf("No C model pattern file: %s\n", cmodelFile);
        return -ERR_BIT_TRUE_NOT_FOUND;
    }
    fseek(fp, 0L, SEEK_END);
    unsigned long filesize = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    if (filesize != outputSize)
    {
        printf("Warning! Pattern size %d is different from encoded size: %d\n", filesize, outputSize);
        //return -ERR_BIT_TRUE;
    }

    unsigned char* cmodel = (unsigned char*)malloc(filesize);
    
#ifdef JPEG_HEX_PATTERN    
    if(filesize%4) printf("WARNING: file is not 4 byte-align, L:%d!!\n", __LINE__);
    for(unsigned int i =0; i< filesize ;){
      fread(cmodel+i+3, 1, 1, fp);
      fread(cmodel+i+2, 1, 1, fp);
      fread(cmodel+i+1, 1, 1, fp);
      fread(cmodel+i+0, 1, 1, fp);      
      i+=4;
    }
#else    
    fread(cmodel, 1, filesize, fp);
#endif    
    
    for(i=0;i<3;i++){
      
      if (memcmp(targetAddr[i], cmodel + diff, targetSize[i])){
          free(cmodel);
          fclose(fp);
          return -ERR_BIT_TRUE;
      }
      diff += targetSize[i] ;
    }
    free(cmodel);        
    fclose(fp);
    
    return 0;
}


int cmodel_compare(unsigned char* targetAddr[3], unsigned int targetSize[3], const char* jpegFilename, unsigned char scale[])
{
    char cmodelFile[128];
    int i ;
    
    unsigned int outputSize = targetSize[0] + targetSize[1] + targetSize[2] ;
    unsigned int diff =0;
    sprintf(cmodelFile, "%s/jpeg_dec_cmodel/%s_%d%d%d%d.yuv", ROOT_PATH, jpegFilename, scale[0], scale[1], scale[2], scale[3]);

    //if (targetSize[i] == 0)
    //    continue;
    
    printf("Compare with pattern: %s\n", cmodelFile);
    wait_pr();

    FILE* fp = fopen(cmodelFile, "rb");
    if (fp == NULL)
    {
        printf("No C model pattern file: %s\n", cmodelFile);
        return -ERR_BIT_TRUE_NOT_FOUND;
    }
    fseek(fp, 0L, SEEK_END);
    unsigned long filesize = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    if (filesize != outputSize)
    {
        printf("Warning! Pattern size %d is different from encoded size: %d\n", filesize, outputSize);
        //return -ERR_BIT_TRUE;
    }

    unsigned char* cmodel = (unsigned char*)malloc(filesize);
    fread(cmodel, 1, filesize, fp);
    
    for(i=0;i<3;i++){
      
      if (memcmp(targetAddr[i], cmodel + diff, targetSize[i])){
          free(cmodel);
          fclose(fp);
          return -ERR_BIT_TRUE;
      }
      diff += targetSize[i] ;
    }
    free(cmodel);        
    fclose(fp);
    
    return 0;
}

#endif


void jpegDecConfigDmaGroup(JPEG_FILE_INFO *info)
{
    unsigned int MCUinRow   = info->mcuPerRow        ; 
    unsigned int Y_H_rsz    = info->lumaHorDecimate  ;
    unsigned int srcFormat  = info->srcColorFormat ;
    unsigned int dstFormat  = info->dstColorFormat ;
    unsigned int McuInGroup ;
    unsigned int GroupNum ; 
    unsigned int LastMcuNum ;
    unsigned int mcuFactor = 3;

    if( srcFormat == JPG_COLOR_444 && dstFormat == JPG_COLOR_422 )      
      mcuFactor = 4 ;
    else if( srcFormat == JPG_COLOR_422V && dstFormat == JPG_COLOR_420 )      
      mcuFactor = 4 ;
    else if( srcFormat == JPG_COLOR_422x2 && dstFormat == JPG_COLOR_422 )      
      mcuFactor = 2 ;
    else if ((srcFormat == 0x00110000 || srcFormat == JPG_COLOR_400) || (srcFormat&0x0FFFF) == 0 )
      mcuFactor = 4 ;
        
    //McuInGroup = Y_H_rsz << mcuFactor ;
    McuInGroup = ((1<< mcuFactor) << Y_H_rsz) ;
    GroupNum = MCUinRow / McuInGroup ;
    LastMcuNum = MCUinRow % McuInGroup ;
    if(LastMcuNum)
      GroupNum++ ;
    else 
      LastMcuNum = McuInGroup ;

    info->dma_McuInGroup = McuInGroup  ;
    info->dma_GroupNum   = GroupNum    ;
    info->dma_LastMcuNum = LastMcuNum  ;
}


JPEG_DEC_STATUS_ENUM jpegDecLockDecoder(int* drvID)
{
    AutoMutex lock(g_jpgDecHalMutex);
    if (currentID == JPEG_HAL_ID_UNLOCK)
    {
        *drvID = open(JPEG_DRV_NAME, O_RDONLY, 0);
        if( *drvID == -1 )
        {
            JPG_ERR("::[DRIVER] Open JPEG Driver Error (%s)", strerror(errno));
            return JPEG_DEC_STATUS_ERROR_CODEC_UNAVAILABLE;
        }

        if(ioctl(*drvID, JPEG_DEC_IOCTL_INIT))
        {
            JPG_ERR("::[DRIVER] JPEG_DEC_IOCTL_INIT Error (%s)", strerror(errno));
            close(*drvID);
            return JPEG_DEC_STATUS_ERROR_CODEC_UNAVAILABLE;
        }
        currentID = *drvID;
        return JPEG_DEC_STATUS_OK;
    }
    else
    {
        return JPEG_DEC_STATUS_ERROR_CODEC_UNAVAILABLE;
    }
}


JPEG_DEC_STATUS_ENUM jpegDecResetDecoder(int drvID)
{
    if(ioctl(drvID, JPEG_DEC_IOCTL_RESET))
    {
    	JPG_ERR("::[DRIVER] JPEG_DEC_IOCTL_RESET Error (%s)", strerror(errno));
        return JPEG_DEC_STATUS_ERROR_CODEC_UNAVAILABLE;
    }

    return JPEG_DEC_STATUS_OK;
}


JPEG_DEC_STATUS_ENUM jpegDecUnlockDecoder(int drvID)
{
    AutoMutex lock(g_jpgDecHalMutex);

    //atrace_begin(ATRACE_TAG_GRAPHICS,"IOT DEINIT");
    if(ioctl(drvID, JPEG_DEC_IOCTL_DEINIT))
    {
        JPG_ERR("::[DRIVER] JPEG_DEC_IOCTL_DEINIT Error (%s)", strerror(errno));
        //atrace_end(ATRACE_TAG_GRAPHICS);
        return JPEG_DEC_STATUS_ERROR_CODEC_UNAVAILABLE;
    }
    //atrace_end(ATRACE_TAG_GRAPHICS);

    close(drvID);
    currentID = JPEG_HAL_ID_UNLOCK;
    return JPEG_DEC_STATUS_OK;
}


int jpegComputeParam(JPEG_FILE_INFO *info)
{
    unsigned int membershipList = 0;
    unsigned int tmp = 0;
    unsigned int blkcnt = 0, comp = 0, blk= 0;  
    unsigned int i;  

    info->blkNumInMCU = 0;
    for( comp=0; comp<3; comp++ ){
      info->blkInCompMCU[comp] = 0;
      if( comp < info->componentNum  ){
          info->blkInCompMCU[comp] = info->hSamplingFactor[comp] * info->vSamplingFactor[comp];
          info->blkNumInMCU += info->blkInCompMCU[comp] ;
      }
    }

    info->uvFlag = (info->componentNum == 3)? 1: 0;
    info->srcColorFormat = ((info->hSamplingFactor[0]) << 20) | ( (info->vSamplingFactor[0]) << 16) |
                           ((info->hSamplingFactor[1]) << 12) | ( (info->vSamplingFactor[1]) << 8 ) |    
                           ((info->hSamplingFactor[2]) <<  4) | ( (info->vSamplingFactor[2])      ) ;    

    switch(info->srcColorFormat){
      case JPG_COLOR_444    : 
         info->dstColorFormat = JPG_COLOR_422 ;
         info->cbcrHorConvEn = 1;
         
         //info->dstColorFormat = JPG_COLOR_444 ;
         //info->cbcrHorConvEn = 0;
         break;         
      case JPG_COLOR_422V   : 
         info->dstColorFormat = JPG_COLOR_420 ;
         info->cbcrHorConvEn = 1;
         break;         
      case JPG_COLOR_422x2  : 
         info->dstColorFormat = JPG_COLOR_422 ;
         info->cbcrHorConvEn = 1;
         break;         
      case JPG_COLOR_422Vx2 : 
         info->dstColorFormat = JPG_COLOR_420 ;
         info->cbcrHorConvEn = 1;
         break;      
      case JPG_COLOR_420: 
      case JPG_COLOR_422:
      default:
         info->dstColorFormat = info->srcColorFormat ;
         info->cbcrHorConvEn = 0;
         break;         
      
    }

    for (blkcnt = 0, blk = 0, comp = 0; blkcnt < D_MAX_JPEG_BLOCK_NUM; blkcnt++)
    {
        if (blkcnt < info->blkNumInMCU && comp < info->componentNum)
        {
            tmp = ( 0x04 + (comp&0x3) ) ;              
            membershipList |= tmp << (blkcnt * 3);            
           if( ++blk == info->blkInCompMCU[comp] ){
             comp++;        
             blk = 0;
           }            
        }
        else
        {              
            membershipList |=  7 << (blkcnt * 3); 
        }
    }    
    
    info->membershipList = membershipList ;
    info->totalMCU = info->mcuPerRow * info->totalMcuRows ;
    
    return true;
}


JPEG_DEC_STATUS_ENUM jpegDecSetSourceFile(int drvID, unsigned char *srcVA, unsigned int srcSize)
{
    int ret;
    unsigned int i, dstPA, tempPA;

    memset(&j_info, 0, sizeof(JPEG_FILE_INFO));

    //j_info.dstStreamSize = 0;
    j_info.jpg_progressive = -1;

    if(jpeg_drv_parse_file(srcVA, srcSize, &j_info) != 0)
    {
        JPG_ERR("::[PARSE] jpeg_drv_parse_file fail");
        return JPEG_DEC_STATUS_ERROR_UNSUPPORTED_FORMAT;
    }
    
    if(jpeg_drv_calculate_info(&j_info) != 0)
    {
        JPG_ERR("::[PARSE] jpeg_drv_calculate_info fail");
        return JPEG_DEC_STATUS_ERROR_UNSUPPORTED_FORMAT;
    }

    if(j_info.jpg_progressive != 0 || j_info.samplingFormat == 0){
        JPG_ERR("::[PARSE] unsupported format, Prog %d, fmt %x!!\n", j_info.jpg_progressive, j_info.samplingFormat);
        return JPEG_DEC_STATUS_ERROR_UNSUPPORTED_FORMAT;
    }
    //printf(" --Format : %d\n", j_info.samplingFormat);
    jpegComputeParam(&j_info);

    return JPEG_DEC_STATUS_OK;
}


//1. asume the scaling fator is valid
//2. asume chroma scaling factor is valid
JPEG_DEC_STATUS_ENUM jpegComputeDstBuffer(int drvID, JPEG_MIN_BUF_INFO* min_info, JPEG_DEC_HAL_MODE decHalMode, unsigned char horDecimate[2], unsigned char verDecimate[2])
{
    unsigned int mcuPerRow, comp;
    unsigned int padding_width ;
    unsigned int downsample_width ;
    unsigned int downsample_row_height[3] ;
    unsigned int horDec[3] ;
    unsigned int verDec[3] ;
    unsigned int strideAlign ;

    if(drvID != currentID)
        return JPEG_DEC_STATUS_ERROR_CODEC_UNAVAILABLE;

    j_info.decodeMode = decHalMode ;
    mcuPerRow = j_info.mcuPerRow;
    j_info.lumaHorDecimate  = horDecimate[0] ;
    j_info.cbcrHorDecimate  = horDecimate[1] + j_info.cbcrHorConvEn;
    j_info.lumaVerDecimate  = verDecimate[0] ;
    j_info.cbcrVerDecimate  = verDecimate[1] ;

    horDec[0]             =  j_info.lumaHorDecimate ;
    horDec[1] = horDec[2] =  j_info.cbcrHorDecimate;
    verDec[0]             =  j_info.lumaVerDecimate ;
    verDec[1] = verDec[2] =  j_info.cbcrVerDecimate;

    for (comp=0; comp<D_MAX_JPEG_HW_COMP; comp++){
        j_info.compImgStride[comp] = 0;
        j_info.compMemStride[comp] = 0;
    }

    for (comp=0; comp<j_info.componentNum; comp++)
    {
        if( horDec[comp] > 3 ||  verDec[comp] > 3 )
        {
            JPG_ERR("::[DST_BUFFER] unsupported format, too big images deci[%d] %d %d > 3!!\n", comp, horDec[comp], verDec[comp]);
            return JPEG_DEC_STATUS_ERROR_UNSUPPORTED_FORMAT;
        }

        padding_width = mcuPerRow * D_DCTSIZE * j_info.hSamplingFactor[comp]; 
        //align to 16 byte-align
        downsample_width = padding_width >> horDec[comp] ;             //output format is 420/422

        if( comp == 0 )
        {
            if(j_info.hSamplingFactor[comp] == 2)
                strideAlign = 32;
            else
                strideAlign = 16;
        }
        else
        {
            strideAlign = 16;
        }	  

        j_info.compImgStride[comp] = TO_CEIL(downsample_width, strideAlign);

        downsample_row_height[comp] = (D_DCTSIZE * j_info.vSamplingFactor[comp] ) >> verDec[comp];

    }
    //JPG_DBG("::[DST_BUFFER] DstImgStride: %d %d %d!!\n", j_info.compImgStride[0], j_info.compImgStride[1], j_info.compImgStride[2]);

    for ( comp = 0; comp < D_MAX_JPEG_HW_COMP; comp++){
        if( comp == 0 ){
            if( j_info.decodeMode == JPEG_DEC_HAL_MCU_ROW ){
                if(j_info.hSamplingFactor[comp] == 2)
                    strideAlign = 32;//256;
                else
                    strideAlign = 16;
            }else{
                strideAlign = 64 ;  //M4U cacheline size is 64
            }
        }else{
             strideAlign = 16;
        }
        j_info.compMemStride[comp] = TO_CEIL(j_info.compImgStride[comp], strideAlign);
        j_info.compMemSize[comp] = j_info.compMemStride[comp] * downsample_row_height[comp] * j_info.totalMcuRows;
    }

    j_info.buffer_Y_row_size = j_info.compMemStride[0] * downsample_row_height[0];
    j_info.buffer_C_row_size = j_info.compMemStride[1] * downsample_row_height[1];

    if ( j_info.componentNum == 1) 
        j_info.outputBuffer0[1] = j_info.outputBuffer0[2] = 0;

    if (  j_info.decodeMode != JPEG_DEC_HAL_DC_LINK ) 
        j_info.outputBuffer1[0] = j_info.outputBuffer1[1] = j_info.outputBuffer1[2] = 0; 

    if (  j_info.decodeMode == JPEG_DEC_HAL_DC_LINK ){    
        min_info->minLumaBufSize    = j_info.buffer_Y_row_size ;
        min_info->minCbCrBufSize    = j_info.buffer_C_row_size ;
    }else if( j_info.decodeMode == JPEG_DEC_HAL_MCU_ROW){
        min_info->minLumaBufSize    = j_info.buffer_Y_row_size ;
        min_info->minCbCrBufSize    = j_info.buffer_C_row_size ;
    }else{
        min_info->minLumaBufSize    = j_info.compMemSize[0] ;
        min_info->minCbCrBufSize    = j_info.compMemSize[1] ; 
    }

    min_info->minLumaBufStride  = j_info.compMemStride[0] ;
    min_info->minCbCrBufStride  = j_info.compMemStride[1] ;
    min_info->ds_lumaImgStride  = j_info.compImgStride[0] ;
    min_info->ds_cbcrImgStride  = j_info.compImgStride[1] ;

    //JPG_DBG("::[DST_BUFFER] DST_SIZE: row %x %x %x, %08x, %08x, %08x!!\n", j_info.buffer_Y_row_size, j_info.buffer_C_row_size, j_info.buffer_C_row_size 
    //                                                                   ,  j_info.compMemSize[0],   j_info.compMemSize[1],   j_info.compMemSize[2]);
    //JPG_DBG("::[DST_BUFFER] DST_STRI: %d, %d, %d, IMG_STRI: %d, %d, %d!!\n",  j_info.compMemStride[0], j_info.compMemStride[1], j_info.compMemStride[2]
    //,  j_info.compImgStride[0], j_info.compImgStride[1], j_info.compImgStride[2]);
    //JPG_DBG("::[DST_BUFFER] IMG_STRI: %d, %d, %d!!\n",  j_info.compImgStride[0], j_info.compImgStride[1], j_info.compImgStride[2]);
    return JPEG_DEC_STATUS_OK;
}


JPEG_DEC_STATUS_ENUM jpegDecSetSourceSize(int drvID, JPEG_DEC_CONFIG* config)
{
    unsigned int srcVA    = config->srcVA;
    unsigned int srcSize  = config->srcSize ;
    unsigned int comp ;
    JPEG_DEC_DRV_IN drvParams;
    unsigned char horDecimate[2] = {0,0};
    unsigned char verDecimate[2] = {0,0};

    if( drvID != currentID)
        return JPEG_DEC_STATUS_ERROR_CODEC_UNAVAILABLE;  

    jpegDecConfigDmaGroup( &j_info );     
    memset(&drvParams, 0, sizeof(JPEG_DEC_DRV_IN));     

    for(comp=0;comp<3;comp++){
      j_info.outputBuffer0[comp] = config->decDstBufAddr0[comp] ;
      j_info.outputBuffer1[comp] = config->decDstBufAddr1[comp] ;
    }
    j_info.tileBufRowNum = config->tileBufRowNum ;

    j_info.srcStreamAddrBase     = srcVA;           
    j_info.srcStreamSize         = srcSize;         
    j_info.srcStreamAddrWritePtr = srcVA + srcSize ;

    switch(j_info.decodeMode){
      case  JPEG_DEC_HAL_FRAME: 
        drvParams.decodeMode =  JPEG_DEC_MODE_FRAME ;
        j_info.reg_OpMode = 0;
        break;         
      case  JPEG_DEC_HAL_DC_LINK:  
        drvParams.decodeMode =  JPEG_DEC_MODE_DC_LINK ;
        j_info.reg_OpMode = 1;
        break;
      case  JPEG_DEC_HAL_MCU_ROW:  
        drvParams.decodeMode =  JPEG_DEC_MODE_MCU_ROW ;
        j_info.reg_OpMode = 2;
        j_info.pauseRow_en = 1;
        break;          
      default:
        drvParams.decodeMode =  JPEG_DEC_MODE_NONE ;
        j_info.reg_OpMode = 0;
        break;  
    }

    drvParams.reg_OpMode   = j_info.reg_OpMode ;
    drvParams.pauseRow_en  = j_info.pauseRow_en ;

    drvParams.srcStreamAddrBase     = j_info.srcStreamAddrBase     ;
    drvParams.srcStreamSize         = j_info.srcStreamSize         ;
    drvParams.srcStreamAddrWritePtr = j_info.srcStreamAddrWritePtr ;

    drvParams.componentNum = j_info.componentNum;
    drvParams.totalMCU     = j_info.totalMCU;
    drvParams.comp0_DU     = j_info.compDU[0] ;

    if (  j_info.decodeMode == JPEG_DEC_HAL_MCU_ROW ){
      drvParams.pauseMCU = j_info.mcuPerRow * config->firstDecRowNum ;
    }else
      drvParams.pauseMCU = j_info.totalMCU + 1;

    for(unsigned int i = 0 ; i < j_info.componentNum ; i++)
    {
      drvParams.componentID[i] = j_info.componentID[i];
      drvParams.hSamplingFactor[i] = j_info.hSamplingFactor[i];
      drvParams.vSamplingFactor[i] = j_info.vSamplingFactor[i];
      drvParams.qTableSelector[i] = j_info.qTableSelector[i];
        
      drvParams.outputBuffer0[i] =  j_info.outputBuffer0[i] ;
      drvParams.outputBuffer1[i] =  j_info.outputBuffer1[i] ;
        
      drvParams.compImgStride[i] =  j_info.compImgStride[i] ;
      drvParams.compMemStride[i] =  j_info.compMemStride[i] ;
    }

    drvParams.membershipList = j_info.membershipList ;
    drvParams.dma_McuInGroup = j_info.dma_McuInGroup ;
    drvParams.dma_GroupNum   = j_info.dma_GroupNum   ;
    drvParams.dma_LastMcuNum = j_info.dma_LastMcuNum ;   
    drvParams.gmcEn = 1;

    drvParams.lumaHorDecimate = j_info.lumaHorDecimate ;
    drvParams.lumaVerDecimate = j_info.lumaVerDecimate ;
    drvParams.cbcrHorDecimate = j_info.cbcrHorDecimate ;
    drvParams.cbcrVerDecimate = j_info.cbcrVerDecimate ;

    //atrace_begin(ATRACE_TAG_GRAPHICS,"IOT CONFIG");
    if(ioctl(drvID, JPEG_DEC_IOCTL_CONFIG, &drvParams) != 0)
    {
        JPG_ERR("::[DRIVER] JPEG_DEC_IOCTL_CONFIG Error (%s)", strerror(errno));
        //atrace_end(ATRACE_TAG_GRAPHICS);
        return JPEG_DEC_STATUS_ERROR_INVALID_FILE;
    }
    //atrace_end(ATRACE_TAG_GRAPHICS);

    JPG_DBG("jpegDecSetSourceSize srcAddr (0x%x 0x%x 0x%x), dstAddr (0x%x 0x%x 0x%x)",
            drvParams.srcStreamAddrBase, drvParams.srcStreamSize, drvParams.srcStreamAddrWritePtr,
            drvParams.outputBuffer0[0], drvParams.outputBuffer0[1], drvParams.outputBuffer0[2]);
    return JPEG_DEC_STATUS_OK;
}


JPEG_DEC_STATUS_ENUM jpegDecGetFileInfo(int drvID, JPEG_FILE_INFO_IN *fileInfo)
{
    unsigned int i = 0;
    //if( drvID != currentID)
    //    return JPEG_DEC_STATUS_ERROR_CODEC_UNAVAILABLE;
        
    fileInfo->width = j_info.width;
    fileInfo->height = j_info.height;
    
    for(i=0;i<3;i++){
      fileInfo->padded_width[i] = j_info.paddedWidth[i];
      fileInfo->padded_height[i] = j_info.paddedHeight[i];
    }

    //fileInfo->dst_buffer_pa = j_info.dstStreamAddr;
    //fileInfo->temp_buffer_pa = j_info.tempBufferAddr;

    fileInfo->total_mcu = j_info.mcuPerRow * j_info.totalMcuRows;
    fileInfo->McuRow = j_info.totalMcuRows;
    fileInfo->McuInRow = j_info.mcuPerRow ;

    if(j_info.dstColorFormat == JPG_COLOR_420)
      fileInfo->samplingFormat = JPEG_DEC_YUV_420;
    else if(j_info.dstColorFormat == JPG_COLOR_422)
      fileInfo->samplingFormat = JPEG_DEC_YUV_422;
    else if(j_info.dstColorFormat == JPG_COLOR_444)
      fileInfo->samplingFormat = JPEG_DEC_YUV_444;
    else if(j_info.dstColorFormat == JPG_COLOR_400)
      fileInfo->samplingFormat = JPEG_DEC_YUV_GRAY;
    else
      fileInfo->samplingFormat = JPEG_DEC_YUV_UNKNOW ;
      
    
    fileInfo->componentNum = j_info.componentNum;
    fileInfo->y_hSamplingFactor = j_info.hSamplingFactor[0];
    fileInfo->y_vSamplingFactor = j_info.vSamplingFactor[0];
    fileInfo->u_hSamplingFactor = j_info.hSamplingFactor[1];
    fileInfo->u_vSamplingFactor = j_info.vSamplingFactor[1];
    fileInfo->v_hSamplingFactor = j_info.hSamplingFactor[2];
    fileInfo->v_vSamplingFactor = j_info.vSamplingFactor[2];

    //JPG_DBG("::[FileInfo] prog %d, Fmt %x->%x, comp %d, w h %d %d ,pad %d %d, %d %d, %d %d, L:%d!!\n", 
    //j_info.jpg_progressive, 
    //fileInfo->samplingFormat,j_info.dstColorFormat, fileInfo->componentNum,
    //j_info.width, j_info.height,
    //j_info.paddedWidth[0], j_info.paddedHeight[0],
    //j_info.paddedWidth[1], j_info.paddedHeight[1],
    //j_info.paddedWidth[2], j_info.paddedHeight[2], __LINE__);

      //JPG_DBG("::[FileInfo] dFmt %x, sfmt %x, comp %d, HxV %d %d, %d %d, %d %d, L:%d!!\n", 
      //         j_info.dstColorFormat,  fileInfo->samplingFormat, fileInfo->componentNum
      //         , j_info.hSamplingFactor[0],  j_info.vSamplingFactor[0]
      //         , j_info.hSamplingFactor[1],  j_info.vSamplingFactor[1]
      //         , j_info.hSamplingFactor[2],  j_info.vSamplingFactor[2]
      //         , __LINE__);      
    return JPEG_DEC_STATUS_OK;
}


JPEG_DEC_STATUS_ENUM jpegDecStart(int drvID, long timeout, JPEG_DEC_RESULT_ENUM *result)
{
    int ret;
    int dec_drv_result;
    unsigned int drv_result;
    JPEG_DEC_DRV_OUT drvParams;
    unsigned int irq_st = 0;

    if( drvID != currentID)
        return JPEG_DEC_STATUS_ERROR_CODEC_UNAVAILABLE;

	/*
    { // property setting for dump jpeg decode registers
        char value[PROPERTY_VALUE_MAX];
        unsigned int dump_reg;

        property_get("vendor.jpegDecode.dumpReg", value, "0");
        dump_reg = atol(value);
        if(0 != dump_reg)
        {
            ioctl(drvID, JPEG_DEC_IOCTL_DUMP_REG);
        }
    }
	*/

    ATRACE_CALL();
#ifdef SKIP_JPEGDEC
    JPG_DBG("::[START_ROW] SKIP_JPEGDEC, L:%d!!\n", __LINE__);
    *result = JPEG_DEC_RESULT_PAUSE;
    return JPEG_DEC_STATUS_OK;
#endif
    // trigger decoder hw
    memset( &cmdqParams, 0 , sizeof(JPEG_DEC_CONFIG_CMDQ));

    //atrace_begin(ATRACE_TAG_GRAPHICS,"IOT START");
    ret = ioctl(drvID, JPEG_DEC_IOCTL_START);
    //atrace_end(ATRACE_TAG_GRAPHICS);
    if(0 != ret)
    {
        return JPEG_DEC_STATUS_ERROR_INVALID_FILE;
    }

    // wait decode result
    //drvParams.decID = decLockID;
    drvParams.result = &drv_result;
    drvParams.timeout = timeout;
    //atrace_begin(ATRACE_TAG_GRAPHICS,"IOT WAIT");
    ret = ioctl(drvID, JPEG_DEC_IOCTL_WAIT, &drvParams);
    //atrace_end(ATRACE_TAG_GRAPHICS);

    if(0 != ret)
        return JPEG_DEC_STATUS_ERROR_INVALID_FILE;
#if 1
    irq_st = drv_result & 0x00FF00 ;
    drv_result = drv_result & 0x0FF ;
#endif
    switch (drv_result)
    {
        case 0:
            *result = JPEG_DEC_RESULT_EOF_DONE;
            JPG_DBG("::[DECODE_RST] DONE SUCCESS, rst:%d, L:%d!!\n", drv_result, __LINE__);
            break;
        case 1:
            *result = JPEG_DEC_RESULT_PAUSE;
            j_info.pauseRowCnt = 1;
            JPG_DBG("::[DECODE_RST] PAUSE SUCCESS, rst:%d, L:%d!!\n", drv_result, __LINE__);
            break;
        case 2:
            *result = JPEG_DEC_RESULT_UNDERFLOW;
            JPG_ERR("::[DECODE_RST] UNDERFLOW, rst:%d, L:%d!!\n", drv_result, __LINE__);
            break;
        case 3:
            *result = JPEG_DEC_RESULT_OVERFLOW;
            JPG_ERR("::[DECODE_RST] OVERFLOW, rst:%d, L:%d!!\n", drv_result, __LINE__);
            break;
        case 4:
            *result = JPEG_DEC_RESULT_ERROR_BS;
            JPG_ERR("::[DECODE_RST] ERROR_BS, rst:%d, L:%d!!\n", drv_result, __LINE__);
            break;
        case 5:
        default:
            *result = JPEG_DEC_RESULT_HALT;
            JPG_ERR("::[DECODE_RST] TIMEOUT, rst:%d, L:%d!!\n", drv_result, __LINE__);
            break;
    }

    return JPEG_DEC_STATUS_OK;
}


JPEG_DEC_STATUS_ENUM jpegDecResumeRow(int drvID, unsigned int decodeRow,
                                      long timeout, JPEG_DEC_RESULT_ENUM *result)
{
    int ret;
    int dec_drv_result;

    unsigned int drv_result =0;
    JPEG_DEC_DRV_OUT drvParams;
    JPEG_DEC_CONFIG_ROW reParams;

    unsigned int pause_row_cnt = 0;
    unsigned int pauseMCU = 0;
    unsigned int base_Y  = 0;
    unsigned int base_CB = 0;
    unsigned int base_CR = 0;
    unsigned int ring_row_index = 0;
    unsigned int dstBuf0_Y;
    unsigned int dstBuf0_U;
    unsigned int dstBuf0_V;
    JPEG_DEC_RESULT_ENUM retcode = JPEG_DEC_RESULT_ERROR_UNKNOWN;
    unsigned int i = 0;
    unsigned int irq_st = 0;

    if( drvID != currentID)
        return JPEG_DEC_STATUS_ERROR_CODEC_UNAVAILABLE;

    ATRACE_CALL();

    base_Y  = j_info.outputBuffer0[0];
    base_CB = j_info.outputBuffer0[1];
    base_CR = j_info.outputBuffer0[2];

    //JPG_DBG("::[RESUME_ROW_SET] pause_row_cnt %d/%d, row %d, out_base %x %x %x, size %x %x %x, L:%d!!\n",
    //         j_info.pauseRowCnt, j_info.tileBufRowNum, decodeRow, base_Y, base_CB, base_CR
    //         , j_info.buffer_Y_row_size, j_info.buffer_C_row_size, j_info.buffer_C_row_size, __LINE__ );

#ifdef SKIP_JPEGDEC
   JPG_DBG("::[RESUME_ROW] SKIP_JPEGDEC, L:%d!!\n", __LINE__);
   *result = JPEG_DEC_RESULT_PAUSE ;
   return JPEG_DEC_STATUS_OK;
#endif

    //for( i=0; i< decodeRow ;i++ )
    {
        pause_row_cnt = j_info.pauseRowCnt;

    #if 0
        if(i==0){
           ring_row_index = pause_row_cnt % j_info.tileBufRowNum;
           pauseMCU = j_info.mcuPerRow * (pause_row_cnt+1);
           dstBuf0_Y = base_Y  + ring_row_index*(j_info.buffer_Y_row_size);
           dstBuf0_U = base_CB + ring_row_index*(j_info.buffer_C_row_size);
           dstBuf0_V = base_CR + ring_row_index*(j_info.buffer_C_row_size);
        }else{
          pauseMCU += j_info.mcuPerRow ;

          if(pause_row_cnt % j_info.tileBufRowNum == 0){
             dstBuf0_Y = base_Y  ;
             dstBuf0_U = base_CB ;
             dstBuf0_V = base_CR ;
          }else{
            dstBuf0_Y += (j_info.buffer_Y_row_size);
            dstBuf0_U += (j_info.buffer_C_row_size);
            dstBuf0_V += (j_info.buffer_C_row_size);
          }
        }
    #else
        ring_row_index = pause_row_cnt % j_info.tileBufRowNum;

        dstBuf0_Y = base_Y  + ring_row_index*(j_info.buffer_Y_row_size);
        dstBuf0_U = base_CB + ring_row_index*(j_info.buffer_C_row_size);
        dstBuf0_V = base_CR + ring_row_index*(j_info.buffer_C_row_size);

        j_info.pauseRowCnt += decodeRow;

        pauseMCU = j_info.mcuPerRow * (j_info.pauseRowCnt);

    #endif

        reParams.decRowBuf[0] =  dstBuf0_Y;
        reParams.decRowBuf[1] =  dstBuf0_U;
        reParams.decRowBuf[2] =  dstBuf0_V;
        reParams.pauseMCU = pauseMCU ;

    #ifdef JPEG_DEC_CPU_GO
    {
        //atrace_begin(ATRACE_TAG_GRAPHICS,"IOT RESUME");
        ret = ioctl(drvID, JPEG_DEC_IOCTL_RESUME, &reParams);
        //atrace_end(ATRACE_TAG_GRAPHICS);

        if(0 != ret)
            return JPEG_DEC_STATUS_ERROR_INVALID_FILE;
        // wait decode result
        drvParams.result = &drv_result;
        drvParams.timeout = timeout;

        //atrace_begin(ATRACE_TAG_GRAPHICS,"IOT WAIT");
        ret = ioctl(drvID, JPEG_DEC_IOCTL_WAIT, &drvParams);
        //atrace_end(ATRACE_TAG_GRAPHICS);
        if(0 != ret)
            return JPEG_DEC_STATUS_ERROR_INVALID_FILE;
    }
    #else
    {
        cmdqParams.pauseMCUidx[cmdqParams.goNum] = pauseMCU;
        cmdqParams.decRowBuf0[cmdqParams.goNum] = dstBuf0_Y ;
        cmdqParams.decRowBuf1[cmdqParams.goNum] = dstBuf0_U ;
        cmdqParams.decRowBuf2[cmdqParams.goNum] = dstBuf0_V ;
        cmdqParams.goNum++;
        if( j_info.pauseRowCnt == j_info.totalMcuRows ){
          ret = ioctl(drvID, JPEG_DEC_IOCTL_FLUSH_CMDQ, &cmdqParams);
        }
        drv_result = JPEG_DEC_RESULT_PAUSE;
    }
    #endif

        //JPG_DBG("::[RESUME_ROW] pause_row_cnt %d, ret %x, MCUinRow %d, p_idx %d, %x %x %x!!\n",
        //         pause_row_cnt, drv_result, j_info.mcuPerRow, pauseMCU, dstBuf0_Y, dstBuf0_U, dstBuf0_V );

        irq_st = drv_result & 0x00FF00 ;
        drv_result = drv_result & 0x00FF ;

        switch (drv_result)
        {
            case 0:
                retcode = JPEG_DEC_RESULT_EOF_DONE;
                break;
            case 1:
                retcode = JPEG_DEC_RESULT_PAUSE;
                break;
            case 2:
                retcode = JPEG_DEC_RESULT_UNDERFLOW;
                break;
            case 3:
                retcode = JPEG_DEC_RESULT_OVERFLOW;
                break;
            case 4:
                retcode = JPEG_DEC_RESULT_ERROR_BS;
                break;
            case 5:
            default:
                retcode = JPEG_DEC_RESULT_HALT;
                break;
        }

        //if(retcode != JPEG_DEC_RESULT_EOF_DONE && retcode !=JPEG_DEC_RESULT_PAUSE)
        //   break;
    }

    *result = retcode ;

    return JPEG_DEC_STATUS_OK;
}


//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
/*
JPEG_DEC_STATUS_ENUM jpegDecResume(int drvID, unsigned int srcFileAddr, unsigned int srcFileSize,
                                   unsigned char isPhyAddr, long timeout, JPEG_DEC_RESULT_ENUM *result)
{
    int ret;
    int dec_drv_result;
    unsigned int drv_result;
    JPEG_DEC_RESUME_IN reParams;
    JPEG_DEC_DRV_OUT drvParams;
    
    if( drvID != currentID)
        return JPEG_DEC_STATUS_ERROR_CODEC_UNAVAILABLE;

    // resume decoder hw
    reParams.decID = decLockID;
    reParams.isPhyAddr = isPhyAddr;
    reParams.srcStreamAddrBase =  srcFileAddr;
    reParams.srcStreamSize = srcFileSize;
    
    ret = ioctl(drvID, JPEG_DEC_IOCTL_RESUME, &reParams);
	if(0 != ret)
	    return JPEG_DEC_STATUS_ERROR_INVALID_FILE;

	// wait decode result
	drvParams.decID = decLockID;
	drvParams.result = &drv_result;
	drvParams.timeout = timeout;
	ret = ioctl(drvID, JPEG_DEC_IOCTL_WAIT, &drvParams);
	if(0 != ret)
	    return JPEG_DEC_STATUS_ERROR_INVALID_FILE;

	switch (drv_result)
    {
        case 0:
	        *result = JPEG_DEC_RESULT_EOF_DONE;
	        break;
	    case 1:
	        *result = JPEG_DEC_RESULT_PAUSE;
	        break; 
	    case 2:
	        *result = JPEG_DEC_RESULT_OVERFLOW;
	        break;
	    case 3:
	        *result = JPEG_DEC_RESULT_HALT;
	        break;
    }
	
    return JPEG_DEC_STATUS_OK;
}

JPEG_DEC_STATUS_ENUM jpegDecGetData(int drvID)
{
    int ret;

    if( drvID != currentID)
        return JPEG_DEC_STATUS_ERROR_CODEC_UNAVAILABLE;
        
    ret = ioctl(drvID, JPEG_DEC_IOCTL_COPY, &decLockID);
	if(0 != ret)
	    return JPEG_DEC_STATUS_ERROR_INVALID_FILE;

     return JPEG_DEC_STATUS_OK;	    
}
*/
