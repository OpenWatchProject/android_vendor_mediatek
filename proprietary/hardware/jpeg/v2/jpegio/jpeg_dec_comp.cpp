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

#include <stdio.h>
#include <cutils/log.h>
#include <utils/Errors.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include "Trace.h"


#include "jpeg_dec_hal.h"
#include "jpeg_dec_comp.h"

#include "img_dec_comp.h"

#include "img_mmu.h"


#define JPGDEC_PORT_READ 1
#define JPGDEC_PORT_WRITE 0

//#define ATRACE_TAG ATRACE_TAG_GRAPHICS

//#define JPEG_PROFILING

#define JPEG_IRQ_TIMEOUT_DEC 3000 /*5000*/  /*3000*/

//#define IMG_COMP_NOT_SUPPORT_GRAY

static bool fail_return(const char msg[]) {
#if 1
    JPG_ERR("::[CODE_RETURN] - %s", msg);
#endif
    return false;   // must always return false
}


static int code_return(const char msg[], int line,int code) {
#if 1
    JPG_ERR("::[CODE_RETURN] - %s, L:%d!!\n", msg, line);
#endif
    return code;   // must always return code
}


JpgDecComp::JpgDecComp()
{
    ATRACE_CALL();

    fBRZ      = 0;
    fMinLumaBufSize = 0;
    fMinCbCrBufSize = 0;

    fMinLumaBufStride = 0;
    fMinCbCrBufStride = 0;

    fLumaImgStride = 0;
    fCbCrImgStride = 0;
    fLumaImgHeight = 0;
    fCbCrImgHeight = 0;

    fOutFormat = 0;

    fSrcBufferSize = 0;
    fSrcBsSize     = 0;

    fSrcConfigAddr = 0;
    fDstConfigAddr = 0;

    fMcuHeightY = 0;
    fMcuHeightC = 0;

#ifdef MTK_M4U_SUPPORT
    pM4uDrv = NULL ;
#endif
    fDecSrcM4UAddrMva = 0;

    // ION
    fIonDevFD = 0;
    fSrcFD = 0;
    fDstFD = 0;
    fSrcIonHdle = 0;
    fDstIonHdle = 0;
    fDecSrcIonPA = 0;

    decID =0;

    fMemType = fMemTypeDefault = IMG_MEM_TYPE_M4U ;

    islock = false;
    fDecRowCnt = 0;
    fDstTileBufRowNum =  0;
    fFirstDecRowNum = 1;
    fDecType = IMG_DEC_MODE_NONE ;
    fOutWidth = fOutHeight = 0;
    fJpgWidth = fJpgHeight = 0;
    fSrcAddr = fDstAddr = NULL;

    for(int i =0; i<3;i++)
    {
        fDecDstBufAddr0[i] = 0 ; //set HW
        fDecDstBufAddr1[i] = 0 ; //set HW
        fDecDstBufSize[i]  = 0 ;
        fDecOutBufAddr0[i] = 0 ;
        fDecOutBufAddr1[i] = 0 ;
        fDstBufStride[i] = 0;
        fDecDstM4UAddrMva0[i] = 0;
        fDecDstM4UAddrMva1[i] = 0;
        fDecDstIonPA[i] = 0;
    }

    fMcuRow = 0;
    fMcuInRow = 0;

    fLumaImgHeight = 0;
    fCbCrImgHeight = 0;

    memset( &t1, 0, sizeof(timeval));
    memset( &t2, 0, sizeof(timeval));
    memset( &t3, 0, sizeof(timeval));

    fCmdqMode = 0;
}


JpgDecComp::~JpgDecComp()
{
    unlock();
}


//void JpgDecComp::setRangeDecode(JUINT32 left, JUINT32 top, JUINT32 right, JUINT32 bottom)
//{
//    isRangeDecode = true;
//    fLeft = left;
//    fTop = top;
//    fRight = right;
//    fBottom = bottom;
//}


bool JpgDecComp::lock()
{
    ATRACE_CALL();
    if(JPEG_DEC_STATUS_OK != jpegDecLockDecoder(&decID)) {
        return fail_return("Jpeg decoder resource is busy");
    }

    islock = true;
    return true;
}


bool JpgDecComp::unlock()
{
    if(islock)
    {
        ATRACE_CALL();
        jpegDecUnlockDecoder(decID);

        if(fMemType == IMG_MEM_TYPE_M4U )
        {
            if(!free_m4u())
                JPG_ERR("free M4U fail, L:%d!!\n", __LINE__);
        }
        else if(fMemType == IMG_MEM_TYPE_ION )
        {
            if(!free_ion())
                JPG_ERR("free ION fail, L:%d!!\n", __LINE__);
        }

        islock = false;
    }

    return true;
}

bool JpgDecComp::checkParam()
{
    return true;
}


bool JpgDecComp::parse()
{
    if(fSrcAddr == NULL)
    {
        return fail_return("source address is null");
    }

    if(JPEG_DEC_STATUS_OK != jpegDecSetSourceFile(decID, fSrcAddr, fSrcBsSize))
    {
        return false;
    }
    JPEG_FILE_INFO_IN info;
    if(JPEG_DEC_STATUS_OK != jpegDecGetFileInfo(decID, &info))
    {
        return false;
    }

    fJpgWidth = info.width;
    fJpgHeight = info.height;
    fMcuRow = info.McuRow ;
    fMcuInRow = info.McuInRow ;

#if 1

    if(info.samplingFormat == JPEG_DEC_YUV_420)
    {
        fOutFormat = IMGDEC_3PLANE_YUV420 ;
    }
    else if(info.samplingFormat == JPEG_DEC_YUV_422)
    {
        fOutFormat = IMGDEC_3PLANE_YUV422 ;
    }
    //else if(info.samplingFormat == JPEG_DEC_YUV_444)
    //{
    //    fOutFormat = IMGDEC_3PLANE_YUV444 ;
    //}
    else if(info.samplingFormat == JPEG_DEC_YUV_GRAY)
    {
        fOutFormat = IMGDEC_3PLANE_GRAY ;
    }
    else
    {
        fOutFormat = IMGDEC_3PLANE_NONE ;
    }
    //JPG_DBG("COMP_PARSE sfmt %d, Outfmt %d, w %d, h %d, row %d, vSample %d %d!!\n", info.samplingFormat, fOutFormat,info.width, info.height, info.McuRow, info.y_vSamplingFactor, info.u_vSamplingFactor);

#else
    fMcuHeightY = info.y_vSamplingFactor << 3;
    if(info.samplingFormat == JPEG_DEC_YUV_GRAY)
      fMcuHeightC = 8;
    else
      fMcuHeightC = info.u_vSamplingFactor << 3;

    fLumaImgHeight = TO_CEIL(fJpgHeight, 4);

    if(info.samplingFormat == JPEG_DEC_YUV_420){
      fLumaImgStride = TO_CEIL(fJpgWidth, 4);
      fCbCrImgStride = fLumaImgStride/2;
      fOutFormat = IMGDEC_3PLANE_YUV420 ;
      fCbCrImgHeight = TO_CEIL(fLumaImgHeight/2, 2);

    }else if(info.samplingFormat == JPEG_DEC_YUV_422){
      fLumaImgStride = TO_CEIL(fJpgWidth, 4);
      fCbCrImgStride = fLumaImgStride/2;
      fOutFormat = IMGDEC_3PLANE_YUV422 ;
      fCbCrImgHeight = TO_CEIL(fLumaImgHeight, 2);
    }else if(info.samplingFormat == JPEG_DEC_YUV_GRAY){
      fLumaImgStride = TO_CEIL(fJpgWidth, 4);
      fCbCrImgStride = fLumaImgStride/2;
      fOutFormat = IMGDEC_3PLANE_GRAY ;
      fCbCrImgHeight = TO_CEIL(fLumaImgHeight, 2);
    }else{
      fOutFormat = IMGDEC_3PLANE_NONE ;
    }
    JPG_DBG("COMP_PARSE sfmt %d, Outfmt %d, w %d, h %d, row %d, McuHeight %d %d, H %d %d!!\n ", info.samplingFormat, fOutFormat,info.width, info.height, info.McuRow, fMcuHeightY, fMcuHeightC, fLumaImgHeight, fCbCrImgHeight);
#endif

    if(fOutFormat == IMGDEC_3PLANE_NONE )
        return false ;
    else
        return true;
}


//bool JpgDecComp::ResumeDecRow(JUINT32 decRowNum)
JUINT32 JpgDecComp::ResumeDecRow(JUINT32 decRowNum)
{
    JUINT32 ret ;
    //ATRACE_CALL();

    // Trigger & Wait JPEG Decoder
    // Set Timeout accroding to image pixels
    JPEG_DEC_RESULT_ENUM result;
    long time_out;
    time_out = (fJpgWidth * fJpgHeight * 100) / (1024 * 1024);

    if(time_out > 3000) time_out = 3000;
    if(time_out < 100) time_out = 100;

    if(decRowNum == 0)
    {
        JPG_ERR("jpeg decoder resume %d+%d->%d !!\n ", fDecRowCnt, decRowNum,fDecRowCnt+decRowNum);
        return IMG_DEC_RST_DONE ;
    }

    if( fDecType != IMG_DEC_MODE_TILE  )
    {
        JPG_ERR("JpgDecComp::ResumeDecRow decodeMode is not ROW_MODE %d!!\n", fDecType );
        return IMG_DEC_RST_DONE ;
    }

    if(fCmdqMode == 0)
    {
    #if 1
        time_out  =  JPEG_IRQ_TIMEOUT_DEC ;
    #endif

    #ifdef JPEG_PROFILING
        gettimeofday(&t1, NULL);
    #endif
        if(JPEG_DEC_STATUS_OK != jpegDecResumeRow(decID, decRowNum,time_out, &result))
        {
            JPG_DBG("JPEG Decoder Start fail, L:%d", __LINE__);
            return false;
        }
    #ifdef JPEG_PROFILING
        gettimeofday(&t2, NULL);
        t3.tv_sec  += t2.tv_sec  - t1.tv_sec ;
        t3.tv_usec += t2.tv_usec - t1.tv_usec ;
    #endif
    }
    else
    {
        result = JPEG_DEC_RESULT_PAUSE ;
    }


    JPG_DBG("jpeg decoder resume %d+%d->%d result:%d ", fDecRowCnt, decRowNum,fDecRowCnt+decRowNum, result);
    fDecRowCnt +=  decRowNum ;

    if(result != JPEG_DEC_RESULT_EOF_DONE && result != JPEG_DEC_RESULT_PAUSE)
        ret = IMG_DEC_RST_DECODE_FAIL ;
    else if( fDecRowCnt < fMcuRow && result == JPEG_DEC_RESULT_PAUSE )
        ret = IMG_DEC_RST_ROW_DONE ;
    else
        ret = IMG_DEC_RST_DONE ;

#ifdef JPEG_PROFILING
    if(ret == IMG_DEC_RST_DONE )
    {
        JPG_DBG("::[DECTIME] DecComp type %d, rst %d, time %u, pixels %dK(%d %d)!!\n",
        fDecType, ret ,(t3.tv_sec)*1000000 + (t3.tv_usec)
        , fJpgWidth*fJpgHeight/(1024), fJpgWidth, fJpgHeight);
        //printf("go SUCCESS %d !!\n", __LINE__);
    }
#endif

   return (ret == IMG_DEC_RST_ROW_DONE || ret == IMG_DEC_RST_DONE)? true : false;
}


bool JpgDecComp::Config(JUINT32 decMode)
{
    fDecType = decMode;
    unsigned char horDecimate[2]  ;
    unsigned char verDecimate[2]  ;
    unsigned int i = 0;

    JPEG_DEC_HAL_MODE decHalMode    ;
    JPEG_MIN_BUF_INFO min_info ;

    //unsigned int ds_width = fJpgWidth >> fBRZ ;
    //unsigned int ds_height = fJpgHeight >> fBRZ ;
    unsigned int ds_align_pixel = (fBRZ == 3)? 8 : 4;

    switch(fDecType)
    {
      case IMG_DEC_MODE_NONE:
          decHalMode = JPEG_DEC_HAL_NONE; break;
      case IMG_DEC_MODE_FRAME:
         decHalMode = JPEG_DEC_HAL_FRAME; break;
      case IMG_DEC_MODE_LINK:
         decHalMode = JPEG_DEC_HAL_DC_LINK;  break;
      case IMG_DEC_MODE_TILE:
         decHalMode = JPEG_DEC_HAL_MCU_ROW;  break;

      default:
         decHalMode = JPEG_DEC_HAL_NONE; break;
    }

    JPEG_FILE_INFO_IN info;
    if(JPEG_DEC_STATUS_OK != jpegDecGetFileInfo(decID, &info))
    {
        return false;
    }


    for(i = 0; i<2;i++)
    {
        horDecimate[i] = fBRZ ;
        verDecimate[i] = fBRZ ;
    }


    fMcuHeightY = info.y_vSamplingFactor << (3 - fBRZ);
    if(info.samplingFormat == JPEG_DEC_YUV_GRAY)
        fMcuHeightC = fMcuHeightY; //8
    else
        fMcuHeightC = info.u_vSamplingFactor << (3 - fBRZ);


    fLumaImgStride = TO_CEIL(fJpgWidth, ds_align_pixel);
    fLumaImgStride >>= fBRZ ;
    fCbCrImgStride = fLumaImgStride/2;

    fLumaImgHeight = TO_CEIL(fJpgHeight, ds_align_pixel);
    fLumaImgHeight >>= fBRZ ;

    if(fOutFormat == IMGDEC_3PLANE_YUV420)
    {
        fCbCrImgHeight = TO_CEIL(fLumaImgHeight/2, 2);
    }
    else if( fOutFormat == IMGDEC_3PLANE_YUV422)
    {
        fCbCrImgHeight = TO_CEIL(fLumaImgHeight, 2);
    }
    else if(fOutFormat == IMGDEC_3PLANE_GRAY)
    {
    #ifdef IMG_COMP_NOT_SUPPORT_GRAY
        if(decHalMode == JPEG_DEC_HAL_MCU_ROW )
        {
            JPG_ERR("JPEG_CONFIG: Current not support MCUROW gray!!\n");
            return false ;
        }
    #endif
        fCbCrImgHeight = TO_CEIL(fLumaImgHeight, 2);
    }
    //else
    //{
    //  //fOutFormat = IMGDEC_3PLANE_NONE ;
    //}

    //JPG_DBG("JPEG_CONFIG: fmt %d, BRZ %d, dsap %d, MCU_H %d %d, imgStride %d %d, imgH %d %d!!\n", fOutFormat,fBRZ, ds_align_pixel, fMcuHeightY, fMcuHeightC, fLumaImgStride, fCbCrImgStride, fLumaImgHeight, fCbCrImgHeight);

    if(JPEG_DEC_STATUS_OK != jpegComputeDstBuffer(decID, &min_info, decHalMode, horDecimate, verDecimate))
    {
        return false;
    }

    fMinLumaBufSize   =  min_info.minLumaBufSize   ;
    fMinCbCrBufSize   =  min_info.minCbCrBufSize   ;
    fMinLumaBufStride =  min_info.minLumaBufStride ;
    fMinCbCrBufStride =  min_info.minCbCrBufStride ;
    //fLumaImgStride    =  min_info.ds_lumaImgStride ;
    //fCbCrImgStride    =  min_info.ds_cbcrImgStride ;

    return true ;
}

bool JpgDecComp::onStart()
{
    ATRACE_CALL();

    JPEG_DEC_CONFIG dec_config ;

    JPEG_FILE_INFO_IN info;
    if(JPEG_DEC_STATUS_OK != jpegDecGetFileInfo(decID, &info))
    {
        return false;
    }

    if(fFirstDecRowNum == 0 && fCmdqMode == 0) fFirstDecRowNum = 1 ;
    fDecRowCnt = fFirstDecRowNum ;

    for(int comp=0;comp<3;comp++)
    {
        dec_config.decDstBufAddr0[comp] = fDecDstBufAddr0[comp];
        dec_config.decDstBufAddr1[comp] = fDecDstBufAddr1[comp];
    }
    dec_config.tileBufRowNum = fDstTileBufRowNum ;
    dec_config.srcVA = fSrcConfigAddr ;
    dec_config.srcSize = fSrcBsSize ;

    dec_config.firstDecRowNum = fFirstDecRowNum ;
    dec_config.dstBufStride[0] = fMinLumaBufStride;
    dec_config.dstBufStride[1] = dec_config.dstBufStride[2] = fMinCbCrBufStride ;

    if(JPEG_DEC_STATUS_OK != jpegDecSetSourceSize(decID, &dec_config))
    {
        return fail_return("JPEG Configure Driver fail");
    }

    if(fCmdqMode == 0)
    {
        // Trigger & Wait JPEG Decoder
        // Set Timeout accroding to image pixels
        JPEG_DEC_RESULT_ENUM result;
        long time_out;
        time_out = ((fJpgWidth/1024) * (fJpgHeight/1024) * 100) ;/// (1024 * 1024);
    #if 1
        if(time_out > 3000) time_out = 3000;
        if(time_out < 100) time_out = 100;

        time_out  =  JPEG_IRQ_TIMEOUT_DEC ;
    #endif

    #ifdef JPEG_PROFILING
        gettimeofday(&t1, NULL);
    #endif
        if(JPEG_DEC_STATUS_OK != jpegDecStart(decID, time_out, &result))
        {
            return fail_return("JPEG Decoder Start fail");
            //return IMG_DEC_RST_DECODE_FAIL ;
        }
    #ifdef JPEG_PROFILING
        gettimeofday(&t2, NULL);

        if(fDecType == IMG_DEC_MODE_LINK)
        {
            JPG_DBG("::[DECTIME] DecComp type %d, rst %d, time %u, pixels %dK(%d %d)!!\n",
              fDecType, result ,(t2.tv_sec - t1.tv_sec)*1000000 + (t2.tv_usec - t1.tv_usec)
              , fJpgWidth*fJpgHeight/(1024), fJpgWidth, fJpgHeight);
              //printf("go SUCCESS %d !!\n", __LINE__);
        }
        else if(fDecType == IMG_DEC_MODE_TILE)
        {
            t3.tv_sec  += t2.tv_sec  - t1.tv_sec ;
            t3.tv_usec += t2.tv_usec - t1.tv_usec ;
            JPG_DBG("jpeg decoder result:%d ", result);
        }
    #endif

        if(JPEG_DEC_RESULT_EOF_DONE != result && result != JPEG_DEC_RESULT_PAUSE)
        {
            return fail_return("JPEG Decode Fail");
            //return IMG_DEC_RST_DECODE_FAIL ;
        }
    }

    return true;
}


bool JpgDecComp::alloc_m4u()
{
#ifdef MTK_M4U_SUPPORT
    unsigned int en_mci = 0;

#ifdef JPEG_PROFILING
    struct timeval t11, t22;
    gettimeofday(&t11, NULL);
#endif

    //android::Tracer::traceBegin(ATRACE_TAG_GRAPHICS,"DEC_M4U_ALLOC");

    if(!imgMmu_create(&pM4uDrv, JPGDEC_PORT_READ))
      return false;

    if(!imgMmu_reset(&pM4uDrv, JPGDEC_PORT_READ))
      return false;

    //src bitstream
    if(en_mci)
    {
        if(!imgMmu_alloc_pa_mci(&pM4uDrv, JPGDEC_PORT_READ, (void*) fSrcAddr, fSrcBufferSize, &fDecSrcM4UAddrMva))
            return false;
    }
    else
    {
        if(!imgMmu_alloc_pa(&pM4uDrv, JPGDEC_PORT_READ, (void*) fSrcAddr, fSrcBufferSize, &fDecSrcM4UAddrMva))
            return false;
    }

#if 1
    //dst bank 0 Luma, Cb, Cr
    if(!imgMmu_alloc_pa(&pM4uDrv, JPGDEC_PORT_WRITE, (void*) fDecOutBufAddr0[0], fDecDstBufSize[0], &fDecDstM4UAddrMva0[0]))
        return false;
    if (fOutFormat != IMGDEC_3PLANE_GRAY)
    {
        if(!imgMmu_alloc_pa(&pM4uDrv, JPGDEC_PORT_WRITE, (void*) fDecOutBufAddr0[1], fDecDstBufSize[1], &fDecDstM4UAddrMva0[1]))
            return false;

        if(!imgMmu_alloc_pa(&pM4uDrv, JPGDEC_PORT_WRITE, (void*) fDecOutBufAddr0[2], fDecDstBufSize[2], &fDecDstM4UAddrMva0[2]))
            return false;
    }

    if(fDecType == IMG_DEC_MODE_LINK)
    {
        //dst bank 1 Luma, Cb, Cr
        if(!imgMmu_alloc_pa(&pM4uDrv, JPGDEC_PORT_WRITE, (void*) fDecOutBufAddr1[0], fDecDstBufSize[0], &fDecDstM4UAddrMva1[0]))
            return false;

        if(!imgMmu_alloc_pa(&pM4uDrv, JPGDEC_PORT_WRITE, (void*) fDecOutBufAddr1[1], fDecDstBufSize[1], &fDecDstM4UAddrMva1[1]))
            return false;

        if(!imgMmu_alloc_pa(&pM4uDrv, JPGDEC_PORT_WRITE, (void*) fDecOutBufAddr1[2], fDecDstBufSize[2], &fDecDstM4UAddrMva1[2]))
            return false;
    }
#endif

    //config module port
    if(en_mci)
        imgMmu_cfg_port_mci(pM4uDrv, JPGDEC_PORT_READ, JPGDEC_PORT_READ);
    else
        imgMmu_cfg_port(pM4uDrv, JPGDEC_PORT_READ, JPGDEC_PORT_READ);

    imgMmu_cfg_port(pM4uDrv, JPGDEC_PORT_WRITE, JPGDEC_PORT_WRITE);

    //flush cache
    if(!en_mci)
    {
        if (!imgMmu_sync(pM4uDrv, JPGDEC_PORT_READ, (void*) fSrcAddr, fDecSrcM4UAddrMva, fSrcBufferSize, SYNC_HW_READ))
        {
            JPG_ERR("srcBuf m4u cache sync fail mva %x, va %lx, size %x", fDecSrcM4UAddrMva, (unsigned long)fSrcAddr, fSrcBufferSize);
        }
    }

    //if(!en_mci)
    {
        if (!imgMmu_sync(pM4uDrv, JPGDEC_PORT_WRITE, (void*) fDecOutBufAddr0[0], fDecDstM4UAddrMva0[0], fDecDstBufSize[0], SYNC_HW_WRITE))
        {
            JPG_ERR("dstBuf[0] m4u cache sync fail mva %x, va %lx, size %x", fDecDstM4UAddrMva0[0], (unsigned long)fDecOutBufAddr0[0], fDecDstBufSize[0]);
        }
        if (fOutFormat != IMGDEC_3PLANE_GRAY)
        {
            if (!imgMmu_sync(pM4uDrv, JPGDEC_PORT_WRITE, (void*) fDecOutBufAddr0[1], fDecDstM4UAddrMva0[1], fDecDstBufSize[1], SYNC_HW_WRITE))
            {
                JPG_ERR("dstBuf[1] m4u cache sync fail mva %x, va %lx, size %x", fDecDstM4UAddrMva0[1], (unsigned long)fDecOutBufAddr0[1], fDecDstBufSize[1]);
            }
            if (!imgMmu_sync(pM4uDrv, JPGDEC_PORT_WRITE, (void*) fDecOutBufAddr0[2], fDecDstM4UAddrMva0[2], fDecDstBufSize[2], SYNC_HW_WRITE))
            {
                JPG_ERR("dstBuf[2] m4u cache sync fail mva %x, va %lx, size %x", fDecDstM4UAddrMva0[2], (unsigned long)fDecOutBufAddr0[2], fDecDstBufSize[2]);
            }
        }

        if(fDecType == IMG_DEC_MODE_LINK)
        {
            if (!imgMmu_sync(pM4uDrv, JPGDEC_PORT_WRITE, (void*) fDecOutBufAddr1[0], fDecDstM4UAddrMva1[0], fDecDstBufSize[0], SYNC_HW_WRITE))
            {
                JPG_ERR("dstBuf[0] m4u cache sync fail mva %x, va %lx, size %x", fDecDstM4UAddrMva1[0], (unsigned long)fDecOutBufAddr1[0], fDecDstBufSize[0]);
            }
            if (!imgMmu_sync(pM4uDrv, JPGDEC_PORT_WRITE, (void*) fDecOutBufAddr1[1], fDecDstM4UAddrMva1[1], fDecDstBufSize[1], SYNC_HW_WRITE))
            {
                JPG_ERR("dstBuf[1] m4u cache sync fail mva %x, va %lx, size %x", fDecDstM4UAddrMva1[1], (unsigned long)fDecOutBufAddr1[1], fDecDstBufSize[1]);
            }
            if (!imgMmu_sync(pM4uDrv, JPGDEC_PORT_WRITE, (void*) fDecOutBufAddr1[2], fDecDstM4UAddrMva1[2], fDecDstBufSize[2], SYNC_HW_WRITE))
            {
                JPG_ERR("dstBuf[2] m4u cache sync fail mva %x, va %lx, size %x", fDecDstM4UAddrMva1[2], (unsigned long)fDecOutBufAddr1[2], fDecDstBufSize[2]);
            }
        }
    }

#ifdef JPEG_PROFILING
    gettimeofday(&t22, NULL);
    JPG_WARN("JpegCompM4U (%dx%d), time : %u!!\n", fJpgWidth, fJpgHeight, (t22.tv_sec - t11.tv_sec)*1000000 + (t22.tv_usec - t11.tv_usec));
#endif
    //android::Tracer::traceEnd(ATRACE_TAG_GRAPHICS);

    return true;
#else

    // return false if we don't support m4u
    return false;
#endif
}


bool JpgDecComp::free_m4u()
{
#ifdef MTK_M4U_SUPPORT
    if(pM4uDrv)
    {
        imgMmu_dealloc_pa(pM4uDrv, JPGDEC_PORT_READ, (void*) fSrcAddr           , fSrcBufferSize   , &fDecSrcM4UAddrMva       );

        imgMmu_dealloc_pa(pM4uDrv, JPGDEC_PORT_WRITE, (void*) fDecOutBufAddr0[0] , fDecDstBufSize[0], &fDecDstM4UAddrMva0[0]   );
        imgMmu_dealloc_pa(pM4uDrv, JPGDEC_PORT_WRITE, (void*) fDecOutBufAddr0[1] , fDecDstBufSize[1], &fDecDstM4UAddrMva0[1]   );
        imgMmu_dealloc_pa(pM4uDrv, JPGDEC_PORT_WRITE, (void*) fDecOutBufAddr0[2] , fDecDstBufSize[2], &fDecDstM4UAddrMva0[2]   );
        if( fDecType == IMG_DEC_MODE_LINK )
        {
            imgMmu_dealloc_pa(pM4uDrv, JPGDEC_PORT_WRITE, (void*) fDecOutBufAddr1[0] , fDecDstBufSize[0], &fDecDstM4UAddrMva1[0]   );
            imgMmu_dealloc_pa(pM4uDrv, JPGDEC_PORT_WRITE, (void*) fDecOutBufAddr1[1] , fDecDstBufSize[1], &fDecDstM4UAddrMva1[1]   );
            imgMmu_dealloc_pa(pM4uDrv, JPGDEC_PORT_WRITE, (void*) fDecOutBufAddr1[2] , fDecDstBufSize[2], &fDecDstM4UAddrMva1[2]   );
        }
        delete pM4uDrv;
    }
    pM4uDrv = NULL;
    return true;
#else

    // return false if we don't support m4u
    return false;
#endif
}


bool JpgDecComp::alloc_ion()
{
#ifdef MTK_M4U_SUPPORT
    if(!imgMmu_create(&pM4uDrv, JPGDEC_PORT_READ))
      return false;
    if(!imgMmu_create(&pM4uDrv, JPGDEC_PORT_WRITE))
      return false;

    imgMmu_cfg_port(pM4uDrv, JPGDEC_PORT_READ, JPGDEC_PORT_READ);
    imgMmu_cfg_port(pM4uDrv, JPGDEC_PORT_WRITE, JPGDEC_PORT_WRITE);
#endif

    imgIon_open(&fIonDevFD);

    //src
    if (fSrcFD)
    {
        if(!imgIon_getPA(fIonDevFD, fSrcFD, JPGDEC_PORT_READ, (void*) fSrcAddr, fSrcBufferSize, &fSrcIonHdle, &fDecSrcIonPA))
            return false;
    }

    //dst
    if (fDstFD)
    {
        if(!imgIon_getPA(fIonDevFD, fDstFD, JPGDEC_PORT_WRITE, NULL, fDecDstBufSize[0] + fDecDstBufSize[1] + fDecDstBufSize[2], &fDstIonHdle, &fDecDstIonPA[0]))
            return false;

        fDecDstIonPA[1] = fDecDstIonPA[0] + fDecDstBufSize[0];
        fDecDstIonPA[2] = fDecDstIonPA[1] + fDecDstBufSize[1];
    }

    imgIon_sync(fIonDevFD, fSrcIonHdle, SYNC_HW_READ);
    imgIon_sync(fIonDevFD, fDstIonHdle, SYNC_HW_WRITE);

    return true;
}


bool JpgDecComp::free_ion()
{
#ifdef MTK_M4U_SUPPORT
    if(pM4uDrv)
    {
        imgMmu_pa_unmap_tlb(pM4uDrv, JPGDEC_PORT_READ, fSrcBufferSize, &fDecSrcIonPA);

        imgMmu_pa_unmap_tlb(pM4uDrv, JPGDEC_PORT_WRITE, fDecDstBufSize[0] + fDecDstBufSize[1] + fDecDstBufSize[2], &fDecDstIonPA[0]);
        delete pM4uDrv;
    }
    pM4uDrv = NULL;
#endif

    imgIon_close(fIonDevFD);

    return true;
}


JUINT32 JpgDecComp::Start()
{
    //Check param
    if(true != checkParam())
    {
        return IMG_DEC_RST_CFG_ERR;
    }
    ATRACE_CALL();

    //JPG_DBG("Decoder Src Addr:%p, size %x, width/height:[%u, %u]           ", fSrcAddr, fSrcBufferSize, fJpgWidth, fJpgHeight);
    //JPG_DBG("Decoder Dst Addr:%p, size %d, width/height:[%u, %u], format:%u", fDstAddr, fDecDstBufSize[0], fOutWidth, fOutHeight, fOutFormat);

    if(fMemType == IMG_MEM_TYPE_M4U)
    {
        if(!alloc_m4u())
            return code_return(" allocate M4U fail, ",__LINE__, IMG_DEC_RST_CFG_ERR);

        //fDstConfigAddr = (unsigned int)(void *)fDstAddr;
        fSrcConfigAddr = fDecSrcM4UAddrMva;
        for(unsigned int i=0; i<3;i++)
        {
            fDecDstBufAddr0[i] = fDecDstM4UAddrMva0[i];
            fDecDstBufAddr1[i] = fDecDstM4UAddrMva1[i];
        }
    }
    else if(fMemType == IMG_MEM_TYPE_ION)
    {
        if(!alloc_ion())
            return code_return(" allocate ION fail, ",__LINE__, IMG_DEC_RST_CFG_ERR);

        //fDstConfigAddr = (unsigned int)(void *)fDstAddr;
        fSrcConfigAddr = fDecSrcIonPA;
        for(unsigned int i=0; i<3;i++)
        {
            fDecDstBufAddr0[i] = fDecDstIonPA[i];
        }
    }

    if(true == onStart())
    {
    }
    else
    {
        return code_return(" JpegComp Decode fail, ",__LINE__, IMG_DEC_RST_DECODE_FAIL);
    }

    return IMG_DEC_RST_DONE;
}

