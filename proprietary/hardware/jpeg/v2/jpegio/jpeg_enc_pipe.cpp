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
#include <stdlib.h>
#include <cutils/log.h>
#include <utils/Errors.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <cutils/log.h>
#include <string.h>

#include <linux/ion.h>
#include <linux/ion_drv.h>

#include "jpeg_enc_hal.h"
#include "jpeg_hal.h"
#include "img_mmu.h"

#include "img_common_def.h"
#include <cutils/properties.h>

#ifdef JENC_LOCK_VIDEO
#include <val_types_private.h>
#include <val_api_private.h>
#include <vdec_drv_base.h>
#include <bits_api.h>
#endif



#ifdef JENC_HW_SUPPORT
#define JPGENC_PORT_READ 1
#define JPGENC_PORT_READ_Y 1
#define JPGENC_PORT_READ_C 1
#define JPGENC_PORT_WRITE 0
#endif

#ifdef USE_DRVB_LIB
extern "C"
{
extern int drvb_f1(unsigned int mask);
}
#endif

extern "C" {
    #include "jpeglib_alpha.h"
    #include "jerror_alpha.h"
   }

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "JpgEncHal"

#define JPEG_IRQ_TIMEOUT_ENC 3000  /*3000*/

#define IS_NO_ALIGN(x,a) ((x)&((a)-1))
#define ALIGN(x,a,b) (((x+a-1)>>b)<<b)

static bool jc_fail_return(const char msg[]) {
#if 1
    JPG_ERR("[JPEG Encoder] - %s", msg);
#endif
    return false;   // must always return false
}


JpgEncHal::JpgEncHal()
{
    //JPG_DBG("JpgEncHal::JpgEncHal");

#ifdef JENC_HW_SUPPORT
  #ifdef MTK_M4U_SUPPORT
    pM4uDrv = NULL;
    fMemType = fMemTypeDefault = JPEG_ENC_MEM_M4U;
  #else
    fMemType = fMemTypeDefault = JPEG_ENC_MEM_ION;
  #endif
#else
  fMemType = fMemTypeDefault = JPEG_ENC_MEM_ION;
#endif

    islock = false;

    fDstWidth = 0;
    fDstHeight = 0;
    fQuality = 0;
    fSrcMinBufferSize = 0;
    fSrcMinCbCrSize = 0;
    fSrcMinBufferStride = 0;
    fSrcMinCbCrStride = 0;
    fEncSrcBufSize = 0;
    fSrcBufStride = 0;
    fSrcBufHeight = 0;
    fEncCbCrBufSize = 0;
    fSrcCbCrBufStride = 0;
    fSrcCbCrBufHeight = 0;
    fEncFormat = kENC_YUY2_Format;
    fSrcAddr = NULL;
    fSrcChromaAddr = NULL;
    fSrcCb = NULL;
    fSrcCr = NULL;
    fDstAddr = NULL;
    fDstAddr = 0;
    fDstSize = 0;
    fIsAddSOI = true;
#ifdef MTK_M4U_SUPPORT
    fDstM4uPA = 0;
    fSrcM4uPA = 0;
    fSrcChromaM4uPA = 0;
#endif
    fIsSrc2p = 0;
    fSrcPlaneNumber = 0;
    fSrcFD = -1;
    fSrcFD2 = -1;
    fDstFD = -1;
    fSrcIonPA = 0;
    fSrcChromaIonPA = 0;
    fDstIonPA = 0;
    fSrcIonVA = NULL ;
    fSrcChromaIonVA = NULL;
    fDstIonVA = NULL ;
    fSrcIonHdle = 0;
    fSrcChromaIonHdle = 0;
    fDstIonHdle = 0;
    fIonDevFD = 0;
    fDRI = 0;
    fIsSelfAlloc = true;
    fIsSrcBufNeedFlush = true;
    encID = 0;
}


JpgEncHal::~JpgEncHal()
{
    //JPG_DBG("JpgEncHal::~JpgEncHal");
}


bool JpgEncHal::lockVideo()
{
#ifdef JENC_LOCK_VIDEO

    VAL_UINT32_T LVDriverType = VAL_DRIVER_TYPE_JPEG_ENC;
    VAL_HW_LOCK_T LVLock;
    VAL_RESULT_T eValRet = VAL_RESULT_NO_ERROR;

    //VDEC_HANDLE_T hHandle ;
    //hHandle.hValHandle = VAL_DRIVER_TYPE_JPEG_ENC ;

    // Init Val once per instance
    //JPG_DBG("::lockVideo, L:%d!!\n", __LINE__);

    if(VAL_RESULT_NO_ERROR != eValInit((VAL_HANDLE_T *)&LVDriverType)){
        JPG_ERR("::lockVideo failed, L:%d!!\n", __LINE__);
        return false ;
    }

    //allocate Handle.
    fTempMem.eAlignment = VAL_MEM_ALIGN_1;
    fTempMem.eMemType = VAL_MEM_TYPE_FOR_HW_CACHEABLE;
    fTempMem.u4MemSize = sizeof(VBITS_HANDLE_T);
    fTempMem.eMemCodec = VAL_MEM_CODEC_FOR_VENC;
    eValRet = eVideoMemAlloc(&fTempMem, sizeof(VAL_MEMORY_T));
    if (eValRet != VAL_RESULT_NO_ERROR)
    {
      JPG_ERR(":: eVideoMemAlloc handle return fail. %d\n", eValRet);
      return false;
    }

    LVLock.pvHandle = (VAL_VOID_T*)fTempMem.pvAlignMemVa;//prParam->pvHandle; // VA of JPEG encoder instance
    LVLock.eDriverType = VAL_DRIVER_TYPE_JPEG_ENC;
    LVLock.u4TimeoutMs = 100;              // return immediately if lock failed

    eValRet = eVideoLockHW(&LVLock, sizeof(VAL_HW_LOCK_T));
    if(VAL_RESULT_NO_ERROR != eValRet )
    {
         JPG_ERR("lockVideo fail, type %d!!\n", eValRet);
         // Deinit Val once per instance
         eValDeInit((VAL_HANDLE_T *)&LVDriverType);
        // Lock HW failed - switch to SW JPEG ENC
        return false; //VAL_FALSE;

    }
    JPG_DBG("::lockVideo done, handle:%p, L:%d!!\n",fTempMem.pvAlignMemVa, __LINE__);

#endif

    return true ;
}


bool JpgEncHal::unlockVideo()
{
#ifdef JENC_LOCK_VIDEO

    VAL_UINT32_T LVDriverType = VAL_DRIVER_TYPE_JPEG_ENC;
    VAL_HW_LOCK_T LVLock;

    //JPG_DBG("::unlockVideo, L:%d!!\n", __LINE__);
    //VDEC_HANDLE_T hHandle ;
    //hHandle.hValHandle = VAL_DRIVER_TYPE_JPEG_ENC ;

    VAL_RESULT_T eValRet;
    JPG_DBG("::unlockVideo handle:%p, L:%d!!\n",fTempMem.pvAlignMemVa ,__LINE__);

    LVLock.pvHandle = (VAL_VOID_T*)fTempMem.pvAlignMemVa;//prParam->pvHandle; // VA of JPEG encoder instance
    LVLock.eDriverType = VAL_DRIVER_TYPE_JPEG_ENC;
    LVLock.u4TimeoutMs = 100;              // return immediately if lock failed
    // Unlock HW
    eValRet = eVideoUnLockHW(&LVLock, sizeof(VAL_HW_LOCK_T));
    if(VAL_RESULT_NO_ERROR != eValRet )
    {
        JPG_ERR("Unlock Video fail, type %d!!\n", eValRet);
    }

    eValRet = eVideoMemFree(&fTempMem, sizeof(VAL_MEMORY_T));
    if (VAL_RESULT_NO_ERROR != eValRet)
    {
        JPG_ERR("eVDecDrvRelease eVideoMemFree ERROR A %d\n", eValRet);
        return eValRet;
    }

    // Deinit Val once per instance
    eValDeInit((VAL_HANDLE_T *)&LVDriverType);
    JPG_DBG("::unlockVideo done, L:%d!!\n", __LINE__);

#endif
    return true ;
}


bool JpgEncHal::LevelLock(EncLockType type)
{
    JPG_DBG("::LevelLock type %d, islock %d, L:%d!!", type, islock, __LINE__);

#ifdef JENC_HW_SUPPORT

    if(islock){
        JPG_ERR("::LevelLock encoder already lock in type %d, islock %d, L:%d!!", type, islock, __LINE__);
        //JPG_DBG("encoder already lock %d, L:%d!!", islock, __LINE__);
        return false ;
    }


    if(type == JPEG_ENC_LOCK_SW_ONLY){
        fEncoderType = JPEG_ENC_SW;
    }
    else
    {
        // Lock HW
        if(!lockVideo())
        {
            if(type == JPEG_ENC_LOCK_HW_ONLY){
                JPG_DBG("::LevelLock, video is busy, return fail bcuz lock mode is HW only, L:%d!!",	__LINE__);
                return false ;
            }else{
                JPG_DBG("::LevelLock, video is busy, change to SW encoder(%d), L:%d!!", type, __LINE__);
                fEncoderType = JPEG_ENC_SW;
            }
        }
        else
        { //lock video success
            unsigned int ret = JPEG_ENC_STATUS_ERROR_CODEC_UNAVAILABLE ;
            do
            {
            #ifdef JENC_LOCK_HWENC
                ret = jpegEncLockEncoder(&encID) ;
            #endif
                if (JPEG_ENC_STATUS_OK != ret)
                {
                    if (type == JPEG_ENC_LOCK_HW_FIRST)
                        unlockVideo();
                    else
                        usleep(10000); // sleep 10 ms for retrying to lock HW
                    fEncoderType = JPEG_ENC_SW;
                }
                else
                {
                    fEncoderType = JPEG_ENC_HW;
                }
            } while(0);
        }
    }
    //JPG_DBG("JpgEncHal::LevelLock enc_type %d, L:%d!!", fEncoderType,  __LINE__);

    #ifdef JENC_PLATFORM_MT6757
    fEncoderType = JPEG_ENC_SW;
    JPG_DBG("::LevelLock, force sw\n");
    #endif

    islock = true;
#else

    fEncoderType = JPEG_ENC_SW;

#endif
    return true;
}


bool JpgEncHal::lock()
{
    JPG_DBG("JpgEncHal::lock %d!!", islock);

#ifdef JENC_HW_SUPPORT

    if(islock)
    {
        JPG_ERR("encoder already lock %d, L:%d!!", islock, __LINE__);
        return false ;
    }

    // Lock HW
    if(!lockVideo())
    {
        fEncoderType = JPEG_ENC_SW;
        JPG_ERR("Lock HW failed switch to sw, L:%d!!",  __LINE__);
    }
    else
    { //lock video success
        unsigned int ret = JPEG_ENC_STATUS_ERROR_CODEC_UNAVAILABLE ;
    #ifdef JENC_LOCK_HWENC
        ret = jpegEncLockEncoder(&encID) ;
    #endif
        if (JPEG_ENC_STATUS_OK != ret)
        {
            unlockVideo();
            JPG_ERR("Lock ENC HW failed switch to sw, L:%d!!",  __LINE__);
            fEncoderType = JPEG_ENC_SW;
        }
        else
        {
            fEncoderType = JPEG_ENC_HW;
        }
    }
    //JPG_DBG("JpgEncHal::lock enc_type %d, L:%d!!", fEncoderType,  __LINE__);

    islock = true;
#else

    fEncoderType = JPEG_ENC_SW;

#endif
    return true;
}


bool JpgEncHal::unlock()
{
    //JPG_DBG("JpgEncHal::unlock");

#ifdef JENC_HW_SUPPORT

    if(islock)
    {
        if(JPEG_ENC_HW == fEncoderType)
        {
            jpegEncUnlockEncoder(encID);
            unlockVideo();

            if( fMemType == JPEG_ENC_MEM_M4U )
            {
                free_m4u() ;
            }
            else if( fMemType == JPEG_ENC_MEM_ION )
            {
                free_ion() ;
            }
        }
        islock = false;
    }

#endif
    return true;
}


bool JpgEncHal::setEncSize(JUINT32 width, JUINT32 height, EncFormat encformat, bool isSrcBufNeedFlush)
{
    unsigned int width2, width_c;

    if( (width * height) > JPEG_MAX_ENC_SIZE)
    {
        JPG_ERR("JpgEncHal:: Unsupported Encode Image Size (%d, %d)!!\n", width, height);
        return false;
    }

    fDstWidth = width;
    fDstHeight = height;
    fEncFormat = encformat;
    fIsSrcBufNeedFlush = isSrcBufNeedFlush;

    width2 = ((width + 1) >> 1) << 1;

    if (fEncFormat == kENC_YUY2_Format || fEncFormat == kENC_UYVY_Format)
    {
        fSrcMinBufferStride  = TO_CEIL((width2 << 1), 32);
        fSrcBufHeight  = TO_CEIL((height), 8);

        fSrcMinBufferSize = fSrcMinBufferStride * fSrcBufHeight;

        fSrcMinCbCrSize = fSrcMinCbCrStride = fSrcCbCrBufHeight = 0;
        fIsSrc2p = 0;
        fSrcPlaneNumber = 1;
    }
    else if (fEncFormat == kENC_NV12_Format || fEncFormat == kENC_NV21_Format)
    {
        fSrcMinBufferStride    = TO_CEIL((width2), 16);
        fSrcBufHeight    = TO_CEIL((height), 16);
        fSrcMinBufferSize   = fSrcMinBufferStride * fSrcBufHeight;

        fSrcMinCbCrStride = fSrcMinBufferStride;
        fSrcCbCrBufHeight = fSrcBufHeight / 2;
        fSrcMinCbCrSize = fSrcMinCbCrStride * fSrcCbCrBufHeight;
        fIsSrc2p = 1;
        fSrcPlaneNumber = 2;
    }
    else if (fEncFormat == kENC_YV12_Format)
    {
        fSrcMinBufferStride = TO_CEIL((width2), 16);
        fSrcBufHeight       = TO_CEIL((height), 16);
        fSrcMinBufferSize   = fSrcMinBufferStride * fSrcBufHeight;

        fSrcMinCbCrStride = TO_CEIL((fSrcMinBufferStride/2), 16);
        fSrcCbCrBufHeight = fSrcBufHeight / 2;
        fSrcMinCbCrSize   = fSrcMinCbCrStride * fSrcCbCrBufHeight;
        printf("ENCHAL:: Y: stride %d, height %d, size %x!!\n", fSrcMinBufferStride, fSrcBufHeight, fSrcMinBufferSize);
        printf("ENCHAL:: C: stride %d, height %d, size %x!!\n", fSrcMinCbCrStride, fSrcCbCrBufHeight, fSrcMinCbCrSize);
        fIsSrc2p = 1;
        fSrcPlaneNumber = 3;
    }
    else
    {
        JPG_ERR("JpgEncHal:: Unsupport JPEG Encode format!");
    }
    //JPG_DBG("JpgEncHal:: stride %d, height %d , min Size %x %x, L:%d!!\n",fSrcMinBufferStride, fSrcBufHeight,fSrcMinBufferSize, fSrcMinCbCrSize, __LINE__);
    return true;
}


bool JpgEncHal::setSrcAddr(void *srcAddr, void *srcChromaAddr)
{
    if( ((unsigned long)srcAddr & 0x0f) || (srcAddr == NULL) )
    {
        JPG_ERR("JpgEncHal:: Src Luma Address is NULL or not 16-byte alignment %lx!!\n", (unsigned long)srcAddr);
        return false;
    }

    if ( fIsSrc2p && ( (srcChromaAddr == NULL) || ((unsigned long)srcChromaAddr & 0x0f) ) )
    {
        JPG_ERR("JpgEncHal::set NV Src CbCr Address NULL or not 16-byte align %lx!!\n", (unsigned long)srcChromaAddr);
        return false;
    }

    fSrcAddr = srcAddr;
    // if chroma address is valid, record this address for later usage
    if( fIsSrc2p && (srcChromaAddr != NULL))
        fSrcChromaAddr = srcChromaAddr;

    return true;
}


bool JpgEncHal::setSrcPAddr(void *srcPAddr, void *srcChromaPAddr)
{
    if( ((unsigned long)srcPAddr & 0x0f) || (srcPAddr == NULL) )
    {
        JPG_ERR("JpgEncHal:: Src Luma PAddress is NULL or not 16-byte alignment %lx!!\n", (unsigned long)srcPAddr);
        return false;
    }

    if ( fIsSrc2p && ( (srcChromaPAddr == NULL) || ((unsigned long)srcChromaPAddr & 0x0f) ) )
    {
        JPG_ERR("JpgEncHal::set NV Src CbCr PAddress NULL or not 16-byte align %lx!!\n", (unsigned long)srcChromaPAddr);
        return false;
    }

    fSrcM4uPA = fSrcIonPA = ((unsigned long)srcPAddr & 0xFFFFFFFFL);

    if( fIsSrc2p )
        fSrcChromaM4uPA = fSrcChromaIonPA = ((unsigned long)srcChromaPAddr & 0xFFFFFFFFL);

    fIsSelfAlloc = false;

    return true ;
}


/// For YUV420(3P), the source bitstream Y, U, V might not be continuous
/// Add a new member function for support Y, U, V are seperate address
/// when fSrcPlaneNumber = 3, there contain 3 plane
bool JpgEncHal::setSrcAddr(void *srcAddr, void *srcCb, void *srcCr)
{
    /// checking source buffer addres alignment
    if(((unsigned long)srcAddr & 0x0f) ||
       (NULL == srcAddr))
    {
        JPG_ERR("JpgEncHal:: Src Luma Address is NULL or not 16-byte alignment %lx!!\n", (unsigned long)srcAddr);
        return false;
    }

    fSrcAddr = srcAddr;

    if(fIsSrc2p)
    {
        fSrcChromaAddr = srcCb;
    }

    if (3 == fSrcPlaneNumber)
    {
        fSrcCb = srcCb;
        fSrcCr = srcCr;
    }

    return true ;
}


bool JpgEncHal::setSrcBufSize( JUINT32 srcStride,JUINT32 srcSize, JUINT32 srcSize2)//, JUINT32 srcStride2)
{
#if 0 // remove checking since we have already provided min stride query function, here we keep the settings as user expected
    if( ((!fIsSrc2p) && (srcSize < fSrcMinBufferSize ))
       && IS_NO_ALIGN(srcStride, 32)  )
    {
        JPG_ERR("JpgEncHal::set YUYV Src Size Fail, %x >= %x, align %x, %x !!\n", srcSize, fSrcMinBufferSize, srcStride);
        return false;
    }
    else if ( (fIsSrc2p && (srcSize2 < fSrcMinCbCrSize))
       && IS_NO_ALIGN(srcStride, 16)  )
    {
        JPG_ERR("JpgEncHal::set NV Src Size Fail, %x >= %x, %x >= %x, stride %x !!\n", srcSize, fSrcMinBufferSize, srcSize2, fSrcMinCbCrSize, srcStride);
        return false;
    }
#endif

    fEncSrcBufSize    = srcSize;
    fSrcBufStride     = srcStride ;

    if(fIsSrc2p)
    {
        fEncCbCrBufSize = srcSize2 ;
        fSrcCbCrBufStride = srcStride ;
    }
    else
    {
        fEncCbCrBufSize = 0 ;
        fSrcCbCrBufStride = 0 ;
    }

    return true ;
};


bool JpgEncHal::setSrcBufSize(JUINT32 srcStride, JUINT32 srcSize, JUINT32 srcSize2, JUINT32 srcSize3, JUINT32 srcUVStride)
{
#if 0 // remove checking since we have already provided min stride query function, here we keep the settings as user expected
    if( ((!fIsSrc2p) &&
        (srcSize < fSrcMinBufferSize)) &&
        IS_NO_ALIGN(srcStride, 32))
    {
        JPG_ERR("JpgEncHal::set YUYV Src Size Fail, %x >= %x, align %x, %x !!\n", srcSize, fSrcMinBufferSize, srcStride);
        return false;
    }
    else if ( (fIsSrc2p &&
             (srcSize2 < fSrcMinCbCrSize)) &&
             IS_NO_ALIGN(srcStride, 16))
    {
        JPG_ERR("JpgEncHal::set NV Src Size Fail, %x >= %x, %x >= %x, stride %x !!\n", srcSize, fSrcMinBufferSize, srcSize2, fSrcMinCbCrSize, srcStride);
        return false;
    }
#endif

    fEncSrcBufSize = srcSize;
    fSrcBufStride  = srcStride;

    if ((3 == fSrcPlaneNumber) &&
        (srcSize2 != srcSize3))
    {
        return false;
    }

    if(fIsSrc2p)
    {
        fEncCbCrBufSize = srcSize2;
        // YV12 format
        if (3 == fSrcPlaneNumber)
            if (0 == srcUVStride)
                fSrcCbCrBufStride = (srcStride >> 1);
            else
                fSrcCbCrBufStride = srcUVStride;
        else
            fSrcCbCrBufStride = srcStride;
    }
    else
    {
        fEncCbCrBufSize   = 0;
        fSrcCbCrBufStride = 0;
    }

    return true ;
}


bool JpgEncHal::setQuality(JUINT32 quality)
{
    if( quality > 100)
        return false ;
    else
        fQuality = quality;

    return true;
}


bool JpgEncHal::setDstAddr(void *dstAddr)
{
    if(dstAddr == NULL)
        return false;
    else
        fDstAddr = dstAddr;

    return true;
}


bool JpgEncHal::setDstPAddr(void *dstPAddr)
{
    if(dstPAddr == NULL)
        return false;
    else
        fDstM4uPA = fDstIonPA = ((unsigned long)dstPAddr & 0xFFFFFFFFL);

    return true;
}


bool JpgEncHal::setDstSize(JUINT32 size)
{
    if(size<624)
        return false;
    else
        fDstSize = size;

    return true;
}


void JpgEncHal::enableSOI(bool b)
{
    fIsAddSOI = b;
}


void JpgEncHal::setIonMode(bool ionEn)
{
    if( ionEn )
        fMemType = JPEG_ENC_MEM_ION;
    else
        fMemType = fMemTypeDefault;
}


void JpgEncHal::setSrcFD( JINT32 srcFD, JINT32 srcFD2 )
{
    fSrcFD = srcFD;
    fSrcFD2 = srcFD2;
}


void JpgEncHal::setDstFD( JINT32 dstFD )
{
    fDstFD = dstFD;
}


void JpgEncHal::setDRI( JINT32 dri )
{
    fDRI = dri;
}


bool JpgEncHal::alloc_m4u()
{
#if defined(MTK_M4U_SUPPORT) && defined(JENC_HW_SUPPORT)
    if(!imgMmu_create(&pM4uDrv, JPGENC_PORT_READ))
        return false;
    if(!imgMmu_create(&pM4uDrv, JPGENC_PORT_WRITE))
        return false;

    if (fIsSrc2p && (fSrcChromaAddr == NULL))
    {
        //src Luma
        if (0x0 == fSrcM4uPA)
        {
            if(!imgMmu_alloc_pa(&pM4uDrv, JPGENC_PORT_READ, fSrcAddr, fEncSrcBufSize + fEncCbCrBufSize, &fSrcM4uPA))
                return false;

            JPG_DBG("self src mva %x, va %lx, size %x", fSrcM4uPA, (unsigned long)fSrcAddr, fEncSrcBufSize);
        }
        else
        {
            JPG_DBG("given src mva %x, va %lx, size %x", fSrcM4uPA, (unsigned long)fSrcAddr, fEncSrcBufSize);
        }

        if (fSrcChromaAddr)
            fSrcChromaM4uPA = fSrcM4uPA + (((long)fSrcChromaAddr - (long)fSrcAddr) & 0xFFFFFFFFL);
        else
            fSrcChromaM4uPA = fSrcM4uPA + fEncSrcBufSize;
    }
    else
    {
        if (0x0 == fSrcM4uPA)
        {
            if(!imgMmu_alloc_pa(&pM4uDrv, JPGENC_PORT_READ, fSrcAddr, fEncSrcBufSize, &fSrcM4uPA))
                return false;

            JPG_DBG("self src mva %x, va %lx, size %x", fSrcM4uPA, (unsigned long)fSrcAddr, fEncSrcBufSize);
        }
        else
        {
            JPG_DBG("given src mva %x, va %lx, size %x", fSrcM4uPA, (unsigned long)fSrcAddr, fEncSrcBufSize);
        }

        //src chroma
        if (0x0 == fSrcChromaM4uPA)
        {
            if(fIsSrc2p && !imgMmu_alloc_pa(&pM4uDrv, JPGENC_PORT_READ, fSrcChromaAddr, fEncCbCrBufSize, &fSrcChromaM4uPA))
                return false;

            JPG_DBG("self src chroma mva %x, va %lx, size %d", fSrcChromaM4uPA, (unsigned long)fSrcChromaAddr, fEncCbCrBufSize);
        }
        else
        {
            JPG_DBG("given src chroma mva %x, va %lx, size %d", fSrcChromaM4uPA, (unsigned long)fSrcChromaAddr, fEncCbCrBufSize);
        }
    }

    //dst bitstream
    if (0x0 == fDstM4uPA)
    {
        if(!imgMmu_alloc_pa(&pM4uDrv, JPGENC_PORT_WRITE, fDstAddr, fDstSize, &fDstM4uPA))
            return false;

        JPG_DBG("self dst mva %x, va %lx, size %x", fDstM4uPA, (unsigned long)fDstAddr, fDstSize);
    }
    else
    {
        JPG_DBG("given dst mva %x, va %lx, size %x", fDstM4uPA, (unsigned long)fDstAddr, fDstSize);
    }

    //config module port
    imgMmu_cfg_port(pM4uDrv, JPGENC_PORT_READ, JPGENC_PORT_READ);
    imgMmu_cfg_port(pM4uDrv, JPGENC_PORT_WRITE, JPGENC_PORT_WRITE);

    if (fIsSrc2p && (fSrcChromaAddr == NULL))
    {
        //flush cache for src buffer
        if (!imgMmu_sync(pM4uDrv, JPGENC_PORT_READ, fSrcAddr, fSrcM4uPA, fEncSrcBufSize + fEncCbCrBufSize, SYNC_HW_READ))
        {
            JPG_ERR("srcBuf m4u cache sync fail mva %x, va %lx, size %x", fSrcM4uPA, (unsigned long)fSrcAddr, (fEncSrcBufSize + fEncCbCrBufSize));
        }
    }
    else
    {
        imgMmu_sync(pM4uDrv, JPGENC_PORT_READ, fSrcAddr, fSrcM4uPA, fEncSrcBufSize, SYNC_HW_READ);
        if(fIsSrc2p) imgMmu_sync(pM4uDrv, JPGENC_PORT_READ, fSrcChromaAddr, fSrcChromaM4uPA, fEncCbCrBufSize, SYNC_HW_READ);
    }

    // invalidate dst buffer
    if (!imgMmu_sync(pM4uDrv, JPGENC_PORT_WRITE, fDstAddr, fDstM4uPA, fDstSize, SYNC_HW_WRITE))
    {
        JPG_ERR("dstBuf m4u cache invalid fail mva %x, va %lx, size %x", fDstM4uPA, (unsigned long)fDstAddr, fDstSize);
    }
    return true;

#else
    // return false if we don't support m4u
    return false;
#endif
}


bool JpgEncHal::free_m4u()
{
#if defined(MTK_M4U_SUPPORT) && defined(JENC_HW_SUPPORT)
   if(pM4uDrv)
   {
      if (fIsSelfAlloc)
      {
          if (fIsSrc2p && (fSrcFD == fSrcFD2))
          {
              imgMmu_dealloc_pa(pM4uDrv, JPGENC_PORT_READ, fSrcAddr, fEncSrcBufSize + fEncCbCrBufSize, &fSrcM4uPA);
          }
          else
          {
              imgMmu_dealloc_pa(pM4uDrv, JPGENC_PORT_READ, fSrcAddr, fEncSrcBufSize ,&fSrcM4uPA);
              if(fIsSrc2p)
                  imgMmu_dealloc_pa(pM4uDrv, JPGENC_PORT_READ, fSrcChromaAddr, fEncCbCrBufSize, &fSrcChromaM4uPA);
          }
      }
      imgMmu_dealloc_pa(pM4uDrv, JPGENC_PORT_WRITE, fDstAddr, fDstSize, &fDstM4uPA);
      delete pM4uDrv;
   }
   pM4uDrv = NULL;
   return true;

#else
   // return false if we don't support m4u
   return false;
#endif

}


bool JpgEncHal::alloc_ion()
{
#ifdef JENC_HW_SUPPORT
#ifdef MTK_M4U_SUPPORT
    if(!imgMmu_create(&pM4uDrv, JPGENC_PORT_READ))
      return false;
    if(!imgMmu_create(&pM4uDrv, JPGENC_PORT_WRITE))
      return false;

    imgMmu_cfg_port(pM4uDrv, JPGENC_PORT_READ, JPGENC_PORT_READ);
    imgMmu_cfg_port(pM4uDrv, JPGENC_PORT_WRITE, JPGENC_PORT_WRITE);
#endif

    imgIon_open(&fIonDevFD );

    //src
    //imgIon_getVA(fSrcFD, fEncSrcBufSize, &fSrcIonVA);
    if (fIsSrc2p && (fSrcFD == fSrcFD2))
    {
        //src
        if (0x0 == fSrcIonPA)
        {
            if( !imgIon_getPA(fIonDevFD, fSrcFD, JPGENC_PORT_READ_Y, fSrcIonVA, fEncSrcBufSize + fEncCbCrBufSize, &fSrcIonHdle, &fSrcIonPA))
                return false;
        }

        if (fSrcChromaAddr)
            fSrcChromaIonPA = fSrcIonPA + (((long)fSrcChromaAddr - (long)fSrcAddr) & 0xFFFFFFFFL);
        else
            fSrcChromaIonPA = fSrcIonPA + fEncSrcBufSize;
    }
    else
    {
        if (0x0 == fSrcIonPA)
        {
            if( !imgIon_getPA(fIonDevFD, fSrcFD, JPGENC_PORT_READ_Y, fSrcIonVA, fEncSrcBufSize, &fSrcIonHdle, &fSrcIonPA))
                return false;
        }

        //src chroma
        if( fIsSrc2p )
        {
            if (0x0 == fSrcChromaIonPA)
            {
                //imgIon_getVA(fSrcFD2, fEncCbCrBufSize, &fSrcChromaIonVA);
                if( !imgIon_getPA(fIonDevFD, fSrcFD2, JPGENC_PORT_READ_C, fSrcChromaIonVA, fEncCbCrBufSize, &fSrcChromaIonHdle, &fSrcChromaIonPA))
                    return false;
            }
        }
    }

    //dst
    if (0x0 == fDstIonPA)
    {
        if( !imgIon_getPA(fIonDevFD, fDstFD, JPGENC_PORT_WRITE, fDstIonVA, fDstSize, &fDstIonHdle, &fDstIonPA))
            return false;
    }

    imgIon_sync(fIonDevFD, fDstIonHdle, SYNC_HW_WRITE);

    if (fIsSrcBufNeedFlush == true)
    {
        imgIon_sync(fIonDevFD, fSrcIonHdle, SYNC_HW_READ);
        if (fIsSrc2p && (fSrcFD != fSrcFD2))
            imgIon_sync(fIonDevFD, fSrcChromaIonHdle, SYNC_HW_READ);
    }
#endif

    return true;
}


bool JpgEncHal::free_ion()
{
#ifdef JENC_HW_SUPPORT
#ifdef MTK_M4U_SUPPORT
    if(pM4uDrv)
    {
        if (fIsSrc2p && (fSrcFD == fSrcFD2))
        {
            imgMmu_pa_unmap_tlb(pM4uDrv, JPGENC_PORT_READ, fEncSrcBufSize + fEncCbCrBufSize, &fSrcIonPA);
        }
        else
        {
            imgMmu_pa_unmap_tlb(pM4uDrv, JPGENC_PORT_READ, fEncSrcBufSize , &fSrcIonPA);
            if( fIsSrc2p )
                imgMmu_pa_unmap_tlb(pM4uDrv, JPGENC_PORT_READ, fEncCbCrBufSize, &fSrcChromaIonPA);
        }

        imgMmu_pa_unmap_tlb(pM4uDrv, JPGENC_PORT_WRITE, fDstSize, &fDstIonPA);
        delete pM4uDrv;
    }
    pM4uDrv = NULL;
#endif

    imgIon_close(fIonDevFD);

#endif
    return true;
}


bool JpgEncHal::onSwEncode(JUINT32 *encSize)
{
	JPG_DBG("SW Encode Start!!!");
    /// using SP JPEG encoder(libjpeg)
    int quality  = fQuality;
    unsigned int image_width  = fDstWidth;
    unsigned int image_height = fDstHeight;
    unsigned int encodeBufferSize = 0;

    struct jpeg_compress_struct_ALPHA cinfo;
    struct jpeg_error_mgr_ALPHA jerr;
    unsigned int i, j, encLines;
    JSAMPROW_ALPHA  volatile y[16], cb[16], cr[16];
    JSAMPARRAY_ALPHA data[3];
    data[0] = (JSAMPARRAY_ALPHA)y;
    data[1] = (JSAMPARRAY_ALPHA)cb;
    data[2] = (JSAMPARRAY_ALPHA)cr;

    /// Step 1: allocate and initialize JPEG compression object
    cinfo.err = jpeg_std_error_ALPHA(&jerr);

    cinfo.client_data = NULL;
    jpeg_create_compress_ALPHA(&cinfo);

    /// Step 2: set parameters for compression
    cinfo.image_width      = image_width;    /* image width and height, in pixels */
    cinfo.image_height     = image_height;
    cinfo.input_components = 3;          /* # of color components per pixel */
    cinfo.in_color_space   = JCS_YCbCr_ALPHA;  /* colorspace of input image */
    cinfo.encode_speed = SLOW_ALPHA;           /* enable multi-thread jpeg encode by 2x */
    jpeg_set_defaults_ALPHA(&cinfo);

    cinfo.raw_data_in = true;

    if (quality > 96)
    {
        if (fEncFormat == kENC_YUY2_Format ||
            fEncFormat == kENC_UYVY_Format)
        {
            if (fDstWidth * fDstHeight * 2 * 1.5 > fDstSize) // 1.5 is worst CR if quality is 100
                quality = 96; // dst buffer size is not enough, limit quality to 96
        }
        else
        {
            if (fDstWidth * fDstHeight * 1.5 * 1.5 > fDstSize) // 1.5 is worst CR if quality is 100
                quality = 96; // dst buffer size is not enough, limit quality to 96
        }
    }

    jpeg_set_quality_ALPHA(&cinfo, quality, TRUE_ALPHA /* limit to baseline-JPEG values */);
    cinfo.en_soi = fIsAddSOI ;
    cinfo.dct_method = JDCT_IFAST_ALPHA;

    /// Step 3: specify data destination (eg, a memory)
    jpeg_mem_dest_ALPHA(&cinfo, (unsigned char **)&fDstAddr, (unsigned long *)&fDstSize);

    if ((kENC_NV21_Format == fEncFormat) ||      /// YUV420
        (kENC_NV12_Format == fEncFormat) ||
        (kENC_YV12_Format == fEncFormat))
    {
         /// supply downsampled data
         cinfo.comp_info[0].h_samp_factor = 2;
         cinfo.comp_info[0].v_samp_factor = 2;
         cinfo.comp_info[1].h_samp_factor = 1;
         cinfo.comp_info[1].v_samp_factor = 1;
         cinfo.comp_info[2].h_samp_factor = 1;
         cinfo.comp_info[2].v_samp_factor = 1;
    }
    else if ((kENC_YUY2_Format == fEncFormat) ||  /// YUV422
             (kENC_UYVY_Format == fEncFormat))
    {
         /// supply downsampled data
         cinfo.comp_info[0].h_samp_factor = 2;
         cinfo.comp_info[0].v_samp_factor = 1;
         cinfo.comp_info[1].h_samp_factor = 1;
         cinfo.comp_info[1].v_samp_factor = 1;
         cinfo.comp_info[2].h_samp_factor = 1;
         cinfo.comp_info[2].v_samp_factor = 1;
    }
    else
    {
        JPG_ERR("JPEG HAL:Unsupport format!\n");
        return false;
    }

    switch(fEncFormat)
    {
        case kENC_NV21_Format:  /// YUV420, 2x2 subsampled , interleaved V/U plane
        case kENC_NV12_Format:  /// YUV420, 2x2 subsampled , interleaved V/U plane
            {
                unsigned char *pCompY    = (unsigned char *)fSrcAddr;
                unsigned char *pCompCbCr;
                unsigned int k;
                if (fSrcChromaAddr)
                    pCompCbCr = (unsigned char* )fSrcChromaAddr;
                else
                    pCompCbCr = (unsigned char *)fSrcAddr+ fEncSrcBufSize;//fSrcChromaAddr;

                unsigned char *pSrcY = NULL;
                unsigned char *pSrcU = NULL;
                unsigned char *pSrcV = NULL;
                unsigned char *mcuTempY;
                unsigned char *mcuTempU;
                unsigned char *mcuTempV;
                bool useTempBuf = false;

                int countToSkip = 0;

                if (cinfo.encode_speed == SLOW_ALPHA)
                {
                    mcuTempU = (unsigned char *)malloc((image_width >> 1) * 8);
                    mcuTempV = (unsigned char *)malloc((image_width >> 1) * 8);
                }
                else // for multi-thread encode, we need to cache u/v buffer
                {
                    unsigned int h_8x = (((image_height + 7) >> 3) << 3);
                    mcuTempU = (unsigned char *)malloc((image_width >> 1) * (h_8x >> 1));
                    mcuTempV = (unsigned char *)malloc((image_width >> 1) * (h_8x >> 1));
                }

                if (image_height % 16)
                {
                    mcuTempY = (unsigned char *)malloc(image_width * 16);
                }

                /// Step 4: Start compressor
                jpeg_start_compress_ALPHA(&cinfo, TRUE_ALPHA);
                //JPG_DBG(" onSwEncode NV12 thread num %d\n", cinfo.thread_num);

                /// assign the JSAMPARRAY address
                for (i = 0; i < 8; i++)
                {
                    cb[i] = mcuTempU + i * (image_width >> 1);
                    cr[i] = mcuTempV + i * (image_width >> 1);
                }


                for (j = 0; j < image_height; j += 16)
                {
                    encLines = ((image_height - j) >= 16)? 16 : (image_height - j);
                    if (encLines < 16)
                    {
                        memcpy(mcuTempY, pCompY, fSrcBufStride * encLines);
                        useTempBuf = true;
                    }


                    for (i = 0; i < 16; i++)
                    {
                        if (i < encLines)
                        {
                            if (useTempBuf)
                            {
                                y[i]  = mcuTempY + i * image_width;
                            }
                            else
                            {
                                y[i]  = pCompY + i * image_width;
                            }
                        }// dummy pointer to avoid libjpeg access invalid address
                        else
                        {
                            if (useTempBuf)
                            {
                                y[i]  = mcuTempY;
                            }
                            else
                            {
                                y[i]  = pCompY;
                            }
                        }
                    }

                    if (cinfo.encode_speed == SLOW_ALPHA)
                    {
                        pSrcU = mcuTempU;
                        pSrcV = mcuTempV;
                    }
                    else
                    {
                        pSrcU = mcuTempU + (image_width >> 1) * (j >> 1);
                        pSrcV = mcuTempV + (image_width >> 1) * (j >> 1);

                        for (i = 0; i < 8; i++)
                        {
                            if (i < (encLines >> 1))
                            {
                                cb[i] = pSrcU + i * (image_width >> 1);
                                cr[i] = pSrcV + i * (image_width >> 1);
                            }
                            else
                            {
                                cb[i] = pSrcU;
                                cr[i] = pSrcV;
                            }
                        }
                    }

                    /// convert YUV420 UV-itlv to YUV420 UV SP temp buffer
                    countToSkip = fSrcCbCrBufStride - image_width;
                    for(k = 0; k < (encLines >> 1); k++)
                    {
                        for(i = 0; i < (image_width >> 1); i++)
                        {
                            if (kENC_NV21_Format == fEncFormat) /// NV12: Y, VU
                            {
                                *pSrcV++ = *pCompCbCr++;
                                *pSrcU++ = *pCompCbCr++;
                            }
                            else
                            {
                                *pSrcU++ = *pCompCbCr++;
                                *pSrcV++ = *pCompCbCr++;
                            }
                        }
                        pCompCbCr = pCompCbCr + countToSkip;
                    }

                    jpeg_write_raw_data_ALPHA(&cinfo, data, 16);
                    pCompY  = pCompY  + encLines * fSrcBufStride;
                }
                if (useTempBuf)
                    free(mcuTempY);
                free(mcuTempU);
                free(mcuTempV);
            }
            break;
       case kENC_YV12_Format:  /// YUV420 3P
            {
                unsigned char *compY  = (unsigned char *)fSrcAddr;
                unsigned char *compCb = (unsigned char *)fSrcCb;
                unsigned char *compCr = (unsigned char *)fSrcCr;

                unsigned char *mcuTempY = NULL;
                unsigned char *mcuTempU = NULL;
                unsigned char *mcuTempV = NULL;
                bool useTempBuf = false;

                // Non-MCU align case need to allocate extra buffer
                if (image_height % 16)
                {
                    mcuTempY = (unsigned char *)malloc(fSrcBufStride * 16);
                    if (fSrcCbCrBufStride != 0)
                    {
                        mcuTempU = (unsigned char *)malloc(fSrcCbCrBufStride * 8);
                        mcuTempV = (unsigned char *)malloc(fSrcCbCrBufStride * 8);
                    }

                    if (!mcuTempY || !mcuTempU || !mcuTempV)
                    {
                        JPG_ERR(" onSwEncode fail due to memory not enough\n");
                        return false;
                    }
                }

                /// Step 4: Start compressor
                jpeg_start_compress_ALPHA(&cinfo, TRUE_ALPHA);
                //JPG_DBG(" onSwEncode YV12 thread num %d\n", cinfo.thread_num);


                for (j = 0; j < image_height; j += 16)
                {
                    encLines = ((image_height - j) >= 16)? 16 : (image_height - j);

                    if (encLines < 16)
                    {
                        memcpy(mcuTempY, compY, fSrcBufStride * encLines);
                        memcpy(mcuTempU, compCb, fSrcCbCrBufStride * (encLines >> 1));
                        memcpy(mcuTempV, compCr, fSrcCbCrBufStride * (encLines >> 1));
                        useTempBuf = true;
                    }

                    for (i = 0; i < 16; i++)
                    {
                        if (i < encLines)
                        {
                            if (useTempBuf)
                            {
                                y[i] = mcuTempY + i * fSrcBufStride;
                            }
                            else
                            {
                                y[i] = compY + i * fSrcBufStride;
                            }

                            if (i%2 == 0)
                            {
                                if (useTempBuf)
                                {
                                    cb[i/2] = mcuTempU + (i/2) * ( fSrcCbCrBufStride );
                                    cr[i/2] = mcuTempV + (i/2) * ( fSrcCbCrBufStride );
                                }
                                else
                                {
                                    cb[i/2] = compCb + (i/2) * ( fSrcCbCrBufStride );
                                    cr[i/2] = compCr + (i/2) * ( fSrcCbCrBufStride );
                                }
                            }
                        }
                        // dummy pointer to avoid libjpeg access invalid address
                        else
                        {
                            if (useTempBuf)
                            {
                                y[i] = mcuTempY;
                            }
                            else
                            {
                                y[i] = compY;
                            }

                            if (i%2 == 0)
                            {
                                if (useTempBuf)
                                {
                                    cb[i/2] = mcuTempU;
                                    cr[i/2] = mcuTempV;
                                }
                                else
                                {
                                    cb[i/2] = compCb;
                                    cr[i/2] = compCr;
                                }
                            }
                        }
                    }

                    jpeg_write_raw_data_ALPHA(&cinfo, data, 16);
                    compY  = compY  + encLines * fSrcBufStride;
                    compCb = compCb + (encLines >> 1)  * fSrcCbCrBufStride;
                    compCr = compCr + (encLines >> 1)  * fSrcCbCrBufStride;
                }

                if (useTempBuf)
                {
                    free(mcuTempY);
                    free(mcuTempU);
                    free(mcuTempV);
                }
            }
            break;
       case kENC_YUY2_Format: /// YUV422 (YUYV, UYVY)
       case kENC_UYVY_Format:
            {
                unsigned int *pCompYUVpacked = (unsigned int *)fSrcAddr;
                unsigned char *pSrcY = NULL;
                unsigned char *pSrcU = NULL;
                unsigned char *pSrcV = NULL;
                unsigned char *mcuTempY;
                unsigned char *mcuTempU;
                unsigned char *mcuTempV;
                unsigned int alignMCUImageWidth = ALIGN(image_width, 16, 4);

                int  count = 0;
                if (cinfo.encode_speed == SLOW_ALPHA)
                {
                    mcuTempY = (unsigned char *)malloc(alignMCUImageWidth * 8);
                    mcuTempU = (unsigned char *)malloc((alignMCUImageWidth >> 1) * 8);
                    mcuTempV = (unsigned char *)malloc((alignMCUImageWidth >> 1) * 8);
                }
                else // for multi-thread encode, we need to cache whole buffer
                {
                    unsigned int h_8x = ALIGN(image_height, 8, 3);
                    mcuTempY = (unsigned char *)malloc(alignMCUImageWidth * h_8x);
                    mcuTempU = (unsigned char *)malloc((alignMCUImageWidth >> 1) * h_8x);
                    mcuTempV = (unsigned char *)malloc((alignMCUImageWidth >> 1) * h_8x);
                }

                /// Step 4: Start compressor
                jpeg_start_compress_ALPHA(&cinfo, TRUE_ALPHA);
                //JPG_DBG(" onSwEncode YUY2 thread num %d\n", cinfo.thread_num);

                /// assign the JSAMPARRAY address
                for (i = 0; i < 8; i++)
                {
                   y[i]  = mcuTempY + i * alignMCUImageWidth;
                   cb[i] = mcuTempU + i * (alignMCUImageWidth >> 1);
                   cr[i] = mcuTempV + i * (alignMCUImageWidth >> 1);
                }

                for (j = 0; j < image_height; j += 8)
                {
                    encLines = ((image_height - j) >= 8)? 8 : (image_height - j);

                    if (cinfo.encode_speed == SLOW_ALPHA)
                    {
                        pSrcY = mcuTempY;
                        pSrcU = mcuTempU;
                        pSrcV = mcuTempV;
                    }
                    else
                    {
                        pSrcY = mcuTempY + alignMCUImageWidth * j;
                        pSrcU = mcuTempU + (alignMCUImageWidth >> 1) * j;
                        pSrcV = mcuTempV + (alignMCUImageWidth >> 1) * j;

                        for (i = 0; i < 8; i++)
                        {
                           y[i]  = pSrcY + i * alignMCUImageWidth;
                           cb[i] = pSrcU + i * (alignMCUImageWidth >> 1);
                           cr[i] = pSrcV + i * (alignMCUImageWidth >> 1);
                        }
                    }

                    /// convert YUV422 itlv to YUV422 temp buffer
                    for (i = 0; i < encLines; i++)
                    {
                        pSrcY = y[i];
                        pSrcU = cb[i];
                        pSrcV = cr[i];
                        count = (image_width >> 1);
                        // pCompYUVpacked need to jump with fSrcBufStride, but we need to divide it by 4
                        // since it is unsigned int pointer
                        pCompYUVpacked = (unsigned int *)fSrcAddr + (i + j) * (fSrcBufStride >> 2);

                        while (--count >= 0)
                        {
                            unsigned int val = *pCompYUVpacked++;
                            if (kENC_UYVY_Format == fEncFormat)
                            {
                                *pSrcU++ = val & 0xFF;
                                *pSrcY++ = (val >> 8) & 0xFF;
                                *pSrcV++ = (val >> 16) & 0xFF;
                                *pSrcY++ = (val >> 24) & 0xFF;
                            }
                            else
                            {
                                *pSrcY++ = val & 0xFF;
                                *pSrcU++ = (val >> 8) & 0xFF;
                                *pSrcY++ = (val >> 16) & 0xFF;
                                *pSrcV++ = (val >> 24) & 0xFF;
                            }
                        }
                    }

                    jpeg_write_raw_data_ALPHA(&cinfo, data, 8);
                }
                free(mcuTempY);
                free(mcuTempU);
                free(mcuTempV);
            }
            break;
        default:
            break;
        }

    /// Step 6: Finish compression
    jpeg_finish_compress_ALPHA(&cinfo);

    /// Step 7: release JPEG compression object
    jpeg_destroy_compress_ALPHA(&cinfo);

    *encSize = fDstSize;
	JPG_DBG("SW Encode Done!!!");
    return true;
}


static unsigned int InIndex = 0;
static unsigned int OutIndex = 0;
int dumpBuffer(unsigned char *SrcAddr, unsigned int size, bool is_out)
{

   FILE *fp = NULL;
   FILE *fpEn = NULL;
   unsigned char* cptr ;
   char filepath[128]="";

   //sprintf(filepath, "/data/otis/dec_pipe_scaler_step_%04d.raw", fileNameIdx);
   if (is_out)
       sprintf(filepath, "//sdcard//otis//%s_%04d.jpg", "JpgEncPipe", OutIndex);
   else
       sprintf(filepath, "//sdcard//otis//%s_%04d.yuv", "JpgEncPipe", InIndex);

   fp = fopen(filepath, "w");
   if (fp == NULL)
   {
       JPG_ERR("open Dump file fail: %s\n", filepath);
       return false;
   }

   JPG_DBG("\nDumpRaw -> %s, addr %p, size %x !!", filepath, SrcAddr, size);
   cptr = (unsigned char*)SrcAddr ;
   if (is_out)
   {
       // fill in SOI marker
       fprintf(fp, "%c", 0xFF);
       fprintf(fp, "%c", 0xD8);
   }

   for( unsigned int i=0;i<size;i++){  /* total size in comp */
     fprintf(fp,"%c", *cptr );
     cptr++;
   }

   fclose(fp);

   if (is_out)
       OutIndex++;
   else
       InIndex++;

   return true ;
}


bool JpgEncHal::start(JUINT32 *encSize)
{
    JPEG_ENC_HAL_IN inJpgEncParam;
    int padding_line;
    unsigned char padding_Y, padding_U, padding_V;
    unsigned char* startAddr;
    int padding_index;

#ifdef USE_DRVB_LIB
    JPG_DBG("drvb S\n");
#ifdef MTK_HW_ENHANCE
    drvb_f1(0x3fffffff);
#else
    drvb_f1(0x1fffffff);
#endif
    JPG_DBG("drvb E\n");
#endif

    //JPG_DBG(" JpgEncHal::start -> config jpeg encoder, memMode %d", fMemType);
    JPG_DBG(" Src Addr:%p, %p, srcFD %d %d, width/height:[%u, %u], format:%u", fSrcAddr, fSrcChromaAddr, fSrcFD, fSrcFD2, fDstWidth, fDstHeight, fEncFormat);
    JPG_DBG(" Src Size:0x%x, 0x%x, Stride %x %x!!", (unsigned int)fEncSrcBufSize, fEncCbCrBufSize,fSrcBufStride, fSrcCbCrBufStride);
    JPG_DBG(" Dst Addr:%p, dstFD %d, Quality:%u, Buffer Size:%u, Need add SOI:%d, memType %d!!", fDstAddr, fDstFD, fQuality, fDstSize, fIsAddSOI, fMemType);


    /* MTK: WorkAround: non-aligned src buffer(YUY2), padding the pixel
    Padding with the value of most right pixel in every line (ALPS03837801) */
    if((kENC_YUY2_Format == fEncFormat) && (fDstWidth*2 != fSrcBufStride))
    {
        startAddr = (unsigned char*)fSrcAddr;
        for(padding_line = 0 ; padding_line < (fEncSrcBufSize / fSrcBufStride) ; padding_line++)
        {
            padding_Y = *(unsigned char*)(startAddr + fDstWidth*2 - 2);
            padding_U = *(unsigned char*)(startAddr + fDstWidth*2 - 3);
            padding_V = *(unsigned char*)(startAddr + fDstWidth*2 - 1);

            for(padding_index = fDstWidth*2 ; padding_index < fSrcBufStride ; padding_index+=4)
            {
                *(startAddr + padding_index + 0) = padding_Y;
                *(startAddr + padding_index + 1) = padding_U;
                if ((padding_index+3) < fSrcBufStride)
                {
                    *(startAddr + padding_index + 2) = padding_Y;
                    *(startAddr + padding_index + 3) = padding_V;
                }
            }
            startAddr += fSrcBufStride;
        }
    }

    if ((kENC_NV12_Format == fEncFormat || kENC_NV21_Format == fEncFormat) &&  (fDstWidth != fSrcBufStride))
    {
        JPG_DBG(" JpgEncHal::image need to padding dstwidth %d fSrcBufStride %d fEncFormat %d\n", fDstWidth, fSrcBufStride, fEncFormat);

        JPG_DBG(" JpgEncHal::image need to padding Y fEncSrcBufSize %d/fSrcBufStride %d ,  %d rows\n", fEncSrcBufSize, fSrcBufStride, (fEncSrcBufSize/fSrcBufStride));

        startAddr = (unsigned char*)fSrcAddr;


        JPG_DBG("0x%x  0x%x  0x%x  0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", *(startAddr + fDstWidth - 1),
            *(startAddr + fDstWidth),
            *(startAddr + fDstWidth + 1),
            *(startAddr + fDstWidth + 2),
            *(startAddr + fDstWidth + 3),
            *(startAddr + fDstWidth + 4),
            *(startAddr + fDstWidth + 5),
            *(startAddr + fDstWidth + 6),
            *(startAddr + fDstWidth + 7),
            *(startAddr + fDstWidth + 8));


        padding_Y = *(startAddr + fDstWidth - 1);

        *(startAddr + fDstWidth) = padding_Y;
        *(startAddr + fDstWidth + 1) = padding_Y;
        *(startAddr + fDstWidth + 2) = padding_Y;
        *(startAddr + fDstWidth + 3) = padding_Y;
        *(startAddr + fDstWidth + 4) = padding_Y;
        *(startAddr + fDstWidth + 5) = padding_Y;
        *(startAddr + fDstWidth + 6) = padding_Y;
        *(startAddr + fDstWidth + 7) = padding_Y;




        JPG_DBG("after padding 0x%x  0x%x  0x%x  0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", *(startAddr + fDstWidth - 1),
            *(startAddr + fDstWidth),
            *(startAddr + fDstWidth + 1),
            *(startAddr + fDstWidth + 2),
            *(startAddr + fDstWidth + 3),
            *(startAddr + fDstWidth + 4),
            *(startAddr + fDstWidth + 5),
            *(startAddr + fDstWidth + 6),
            *(startAddr + fDstWidth + 7),
            *(startAddr + fDstWidth + 8));


        for(padding_line = 0 ; padding_line < (fEncSrcBufSize / fSrcBufStride) ; padding_line++)
        {
            padding_Y = *(unsigned char*)(startAddr + fDstWidth - 1);
            for(padding_index = fDstWidth ; padding_index < fSrcBufStride ; padding_index++)
            {
                *(startAddr + padding_index) = padding_Y;
            }

            startAddr += fSrcBufStride;
        }


        JPG_DBG(" JpgEncHal::image need to padding UV fEncCbCrBufSize %d/fSrcCbCrBufStride %d ,  %d rows\n", fEncCbCrBufSize, fSrcCbCrBufStride, (fEncCbCrBufSize/fSrcCbCrBufStride));
        startAddr = (unsigned char*)fSrcChromaAddr;


        JPG_DBG("0x%x  0x%x  0x%x  0x%x  0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", *(startAddr + fDstWidth - 2) , *(startAddr + fDstWidth - 1),
            *(startAddr + fDstWidth),
            *(startAddr + fDstWidth + 1),
            *(startAddr + fDstWidth + 2),
            *(startAddr + fDstWidth + 3),
            *(startAddr + fDstWidth + 4),
            *(startAddr + fDstWidth + 5),
            *(startAddr + fDstWidth + 6),
            *(startAddr + fDstWidth + 7),
            *(startAddr + fDstWidth + 8));


        padding_U = *(unsigned char*)(startAddr + fDstWidth - 1);
        padding_V = *(unsigned char*)(startAddr + fDstWidth - 2);

        *(startAddr + fDstWidth) = padding_V;
        *(startAddr + fDstWidth + 1) = padding_U;
        *(startAddr + fDstWidth + 2) = padding_V;
        *(startAddr + fDstWidth + 3) = padding_U;
        *(startAddr + fDstWidth + 4) = padding_V;
        *(startAddr + fDstWidth + 5) = padding_U;
        *(startAddr + fDstWidth + 6) = padding_V;
        *(startAddr + fDstWidth + 7) = padding_U;

        JPG_DBG("after padding 0x%x  0x%x  0x%x  0x%x  0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", *(startAddr + fDstWidth - 2) , *(startAddr + fDstWidth - 1),
            *(startAddr + fDstWidth),
            *(startAddr + fDstWidth + 1),
            *(startAddr + fDstWidth + 2),
            *(startAddr + fDstWidth + 3),
            *(startAddr + fDstWidth + 4),
            *(startAddr + fDstWidth + 5),
            *(startAddr + fDstWidth + 6),
            *(startAddr + fDstWidth + 7),
            *(startAddr + fDstWidth + 8));

        for(padding_line = 0 ; padding_line < (fEncCbCrBufSize / fSrcCbCrBufStride) ; padding_line++)
        {
            padding_U = *(unsigned char*)(startAddr + fDstWidth - 1);
            padding_V = *(unsigned char*)(startAddr + fDstWidth - 2);

            for(padding_index = fDstWidth ; padding_index < fSrcCbCrBufStride ; padding_index+=2)
            {
                *(startAddr + padding_index) = padding_V;
                *(startAddr + padding_index + 1) = padding_U;
            }
            startAddr += fSrcCbCrBufStride;
        }


    }

#ifdef JENC_HW_SUPPORT

    if (JPEG_ENC_HW == fEncoderType)
    {
        bool switchToSw = false;
        char value[PROPERTY_VALUE_MAX];
        unsigned long forceToUseSWEnc;

        property_get("vendor.jpegEncode.forceEnable.SW", value, "0");
        forceToUseSWEnc = atol(value);

    #ifdef JENC_CORRECT_ALIGNMENT_CHECK
        // NV12/21 must 16x16 align
        if ((kENC_NV12_Format == fEncFormat || kENC_NV21_Format == fEncFormat) &&
            ((fSrcBufStride < ALIGN(fDstWidth, 16, 4)) || ((fEncSrcBufSize / fSrcBufStride) < ALIGN(fDstHeight, 16, 4))))
        {
            switchToSw = true;
            JPG_DBG(" JpgEncHal::start YUV420 Non-MCU align => force to use sw");
        }
    #else
        // NV12/21 must 16x8 align
        if ((kENC_NV12_Format == fEncFormat || kENC_NV21_Format == fEncFormat) &&
            ((fSrcBufStride < ALIGN(fDstWidth, 16, 4)) || ((fEncSrcBufSize / fSrcBufStride) < ALIGN(fDstHeight, 8, 3))))
        {
            switchToSw = true;
            JPG_DBG(" JpgEncHal::start YUV420 Non-MCU align => force to use sw");
        }
    #endif

        // YUY2 must 32x8 align
        if (kENC_YUY2_Format == fEncFormat &&
            ((fSrcBufStride < ALIGN(fDstWidth, 32, 5)) || ((fEncSrcBufSize / fSrcBufStride) < ALIGN(fDstHeight, 8, 3))))
        {
            switchToSw = true;
            JPG_DBG(" JpgEncHal::start YUV422 Non-MCU align => force to use sw");
        }

        // redirect encode to sw since HW did not support YV12 and UYVY format
        if (kENC_YV12_Format == fEncFormat || kENC_UYVY_Format == fEncFormat)
        {
            switchToSw = true;
            JPG_DBG(" JpgEncHal::unsupported format %d => force to use sw", fEncFormat);
        }

        if (switchToSw == true || forceToUseSWEnc != 0)
        {
            jpegEncUnlockEncoder(encID);
            unlockVideo();
            fEncoderType = JPEG_ENC_SW;
        }
    }

#endif

    if (JPEG_ENC_SW == fEncoderType)
    {
        if(!onSwEncode(encSize))
            return jc_fail_return("jpeg sw encode fail!!");
    }
#ifdef JENC_HW_SUPPORT
    else
    { /// if (JPEG_ENC_SW == fEncoderType)
        // config jpeg encoder
        memset(&inJpgEncParam, 0 , sizeof(JPEG_ENC_HAL_IN));

        inJpgEncParam.dstBufferSize     = fDstSize;
        inJpgEncParam.dstWidth          = fDstWidth;
        inJpgEncParam.dstHeight         = fDstHeight;
        inJpgEncParam.srcBufferStride   = fSrcBufStride ;     //TODO: check validation
        //inJpgEncParam.srcChromaStride = fSrcCbCrBufStride ; //TODO: check validation
        inJpgEncParam.restartInterval   = fDRI;

        /* add check to confirm the mem type */
        if (fMemType == JPEG_ENC_MEM_ION &&
            (fSrcFD == -1 || fDstFD == -1))
            fMemType = JPEG_ENC_MEM_M4U;

        if( fMemType == JPEG_ENC_MEM_PHY){
          // unused
          //inJpgEncParam.dstBufferAddr = (unsigned int) fDstAddr;
          //inJpgEncParam.srcBufferAddr = (unsigned int) fSrcAddr ;
          //inJpgEncParam.srcChromaAddr = (unsigned int) fSrcChromaAddr;
        }

        if( fMemType == JPEG_ENC_MEM_M4U )
        {
            if(!alloc_m4u())
            {
              JPG_ERR("Encoder Allocate M4U Fail!!\n");
              return false;
            }
            inJpgEncParam.srcBufferAddr = fSrcM4uPA       ;
            inJpgEncParam.srcChromaAddr = fSrcChromaM4uPA ;
            inJpgEncParam.dstBufferAddr = fDstM4uPA;
        }
        else if( fMemType == JPEG_ENC_MEM_ION )
        {
            if(!alloc_ion())
            {
              JPG_ERR("Encoder Allocate ION Fail!!\n");
              return false;
            }
            inJpgEncParam.srcBufferAddr = fSrcIonPA;
            inJpgEncParam.srcChromaAddr = fSrcChromaIonPA;
            inJpgEncParam.dstBufferAddr = fDstIonPA;
        }

        if(fQuality >= 97)
        {
            inJpgEncParam.dstQuality = JPEG_ENCODE_QUALITY_Q97;
        }
        else if(fQuality >= 95)
        {
            inJpgEncParam.dstQuality = JPEG_ENCODE_QUALITY_Q95;
        }
        else if(fQuality >= 92)
        {
            inJpgEncParam.dstQuality = JPEG_ENCODE_QUALITY_Q92;
        }
        else if(fQuality >= 90)
        {
            inJpgEncParam.dstQuality = JPEG_ENCODE_QUALITY_Q90;
        }
        else if(fQuality >= 87)
        {
            inJpgEncParam.dstQuality = JPEG_ENCODE_QUALITY_Q87;
        }
        else if(fQuality >= 84)
        {
            inJpgEncParam.dstQuality = JPEG_ENCODE_QUALITY_Q84;
        }
        else if(fQuality >= 80)
        {
            inJpgEncParam.dstQuality = JPEG_ENCODE_QUALITY_Q80;
        }
        else if(fQuality >= 74)
        {
            inJpgEncParam.dstQuality = JPEG_ENCODE_QUALITY_Q74;
        }
        else if(fQuality >= 64)
        {
            inJpgEncParam.dstQuality = JPEG_ENCODE_QUALITY_Q64;
        }
        else if(fQuality >= 60)
        {
            inJpgEncParam.dstQuality = JPEG_ENCODE_QUALITY_Q60;
        }
        else
        {
            inJpgEncParam.dstQuality = JPEG_ENCODE_QUALITY_Q48;
        }

        inJpgEncParam.isPhyAddr = 0;

        if(fIsAddSOI)
            inJpgEncParam.enableEXIF = 0;
        else
            inJpgEncParam.enableEXIF = 1;

        switch (fEncFormat)
        {
            case kENC_YUY2_Format:
                inJpgEncParam.dstFormat = JPEG_SAMPLING_FORMAT_YUY2;
                break;

            case kENC_UYVY_Format:
                inJpgEncParam.dstFormat = JPEG_SAMPLING_FORMAT_YVYU;
                break;

            case kENC_NV12_Format:
                inJpgEncParam.dstFormat = JPEG_SAMPLING_FORMAT_NV12;
                break;

            case kENC_NV21_Format:
                inJpgEncParam.dstFormat = JPEG_SAMPLING_FORMAT_NV21;
                break;

            default :
                JPG_ERR("Unsupport Encoder Format : %d", fEncFormat);
                return false;

        }

        if(JPEG_ENC_STATUS_OK != jpegEncConfigEncoder(encID, inJpgEncParam))
        {
            JPG_ERR("Config Encoder Fail");
            return false;
        }

        //JPG_DBG(" JpgEncHal::trigger jpeg encoder");
        // Trigger Jpeg Encoder
        if(JPEG_ENC_STATUS_OK != jpegEncStart(encID))
        {
            JPG_ERR("Trigger Encoder Fail");
            return false;
        }

        //JPG_DBG(" JpgEncHal::wait jpeg encoder");
        JPEG_ENC_RESULT_ENUM result;
        if(JPEG_ENC_STATUS_OK != jpegEncWaitIRQ(encID, JPEG_IRQ_TIMEOUT_ENC, encSize, &result))
        {
            JPG_ERR("Wait IRQ Fail");
            return false;
        }

        //JPG_DBG(" JpgEncHal::wait jpeg encoder done");
        if (fMemType == JPEG_ENC_MEM_ION) // unmap dma for cache sync/invalidate
        {
            imgIon_syncDone(fIonDevFD, fDstIonHdle, SYNC_HW_WRITE);

            if (fIsSrcBufNeedFlush == true)
            {
                imgIon_syncDone(fIonDevFD, fSrcIonHdle, SYNC_HW_READ);
                if (fIsSrc2p && (fSrcFD != fSrcFD2))
                    imgIon_syncDone(fIonDevFD, fSrcChromaIonHdle, SYNC_HW_READ);
            }
        }

        // we need to find EOI marker from the end of bitstream in order to get correct encode size
        // find maximum 128 bytes since HW is 128 bytes alignment
        if (*encSize > 128)
        {
            unsigned char *tmpDstAddr = (unsigned char *)fDstAddr + (*encSize - 1);
            unsigned int offset = 0;
            while(offset <= 128)
            {
                if ((*(tmpDstAddr - 1) == 0xFF) && (*tmpDstAddr == 0xD9))
                {
                    break;
                }
                tmpDstAddr--;
                offset++;
            }
            *encSize = *encSize - offset;
        }

        JPG_DBG(" result:%d, size:%u", result, *encSize);

        if(result != JPEG_ENC_RESULT_DONE)
            return false;
    }
#endif /*JENC_HW_SUPPORT*/

    //JPG_DBG("JpgEncHal::jpeg encoder done");
    return true;
}

