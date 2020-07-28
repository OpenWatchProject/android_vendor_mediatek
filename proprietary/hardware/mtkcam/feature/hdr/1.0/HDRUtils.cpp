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

/********************************************************************************************
 *       LEGAL DISCLAIMER
 *
 *       (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *       BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *       THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *       FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *       ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *       INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *       A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *       WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *       INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *       ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *       NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *       OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *       BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *       RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *       FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *       THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *       OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#include "MyHdr.h"

#include <sys/resource.h>

#include <cutils/properties.h>

#include <mtkcam/hal/aaa_hal_base.h>
#include <mtkcam/iopipe/SImager/IImageTransform.h>
#include <mtkcam/IImageBuffer.h>
#include <mtkcam/feature/hdr/1.0/utils/ImageBufferUtils.h>

// TODO: decouple this part from v1
#include <CamUtils.h>

using namespace android;
using namespace NSCam;
using namespace NSCam::NSIoPipe::NSSImager;

#define ANNOUNCE(mutex)         announce(mutex, #mutex)
#define WAIT(mutex)             wait(mutex, #mutex)

#define SKIP_MEMCPY 0

//systrace
#if 1
#ifndef ATRACE_TAG
#define ATRACE_TAG                           ATRACE_TAG_CAMERA
#endif
#include <utils/Trace.h>

#define HDR_TRACE_CALL()                      ATRACE_CALL()
#define HDR_TRACE_NAME(name)                  ATRACE_NAME(name)
#define HDR_TRACE_BEGIN(name)                 ATRACE_BEGIN(name)
#define HDR_TRACE_END()                       ATRACE_END()
#else
#define HDR_TRACE_CALL()
#define HDR_TRACE_NAME(name)
#define HDR_TRACE_BEGIN(name)
#define HDR_TRACE_END()
#endif

// 4-digit running number (range: 0 ~ 9999)
MUINT32    HdrShot::mu4RunningNumber = 0;

struct HdrFileInfo {
    String8    filename;
    MUINT32    size;
    MUINT8    *buffer;
};

/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::
configureForSingleRun(void)
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;
    if(mfgIsForceBreak) {MY_DBG("force break at %s", __FUNCTION__); return MFALSE;}

    mHdrRound = 1;

    FUNCTION_LOG_END;
    return ret;
}


MBOOL
HdrShot::
configureForFirstRun(void)
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;
    if(mfgIsForceBreak) {MY_DBG("force break at %s", __FUNCTION__); return MFALSE;}

    mHdrRound = 1;

    FUNCTION_LOG_END;
    return ret;
}


MBOOL
HdrShot::
configureForSecondRun(void)
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;
    if(mfgIsForceBreak) {MY_DBG("force break at %s", __FUNCTION__); return MFALSE;}

    mHdrRound = 2;

    ret = ret
    &&    do_SecondRound()
    ;

    FUNCTION_LOG_END;
    return ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::
WeightingMapGeneration(void)
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;
    if(mfgIsForceBreak) {MY_DBG("force break at %s", __FUNCTION__); return MFALSE;}

#if (HDR_PROFILE_CAPTURE2)
    MyDbgTimer DbgTmr("capture");
#endif

    if(mHdrRound==1) {
        ret = ret
        && requestOriWeightMapBuf()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&  DbgTmr.print("HdrProfiling2:: requestOriWeightingTblBuf Time")
                                            #endif
        ;
    }


    ret = ret
      //  ()  Do Alignment (includeing "Feature Matching" and "Weighting Map Generation").
    &&    do_Alignment()
    ;
                                            #if (HDR_PROFILE_CAPTURE2)
                                            DbgTmr.print("HdrProfiling2:: do_Alignment Time");
                                            #endif
    ret = ret
      //  ()  Request Original Weighting Table Buffer.
      //  ()  Get original Weighting Map.
    &&  do_OriWeightMapGet()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&  DbgTmr.print("HdrProfiling2:: do_OriWeightMapGet Time")
                                            #endif
      // Blur Weighting Map by downsize-then-upscale it.
    &&  requestDownSizedWeightMapBuf()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&  DbgTmr.print("HdrProfiling2:: requestDownSizedWeightMapBuf Time")
                                            #endif
      //  ()  Down-scale original weighting map, and put into DownSizedWeightMapBuf.
    &&  do_DownScaleWeightMap()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&  DbgTmr.print("HdrProfiling2:: do_DownScaleWeightMap Time")
                                            #endif
      //  ()  Request Blurred weighting map buffer.
    &&  requestBlurredWeightMapBuf()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&  DbgTmr.print("HdrProfiling2:: requestBlurredWeightMapBuf Time")
                                            #endif
      //  ()  Up-scale DownSized WeightMap Buf, and put into blurred weighting map.
    &&  do_UpScaleWeightMap()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&  DbgTmr.print("HdrProfiling2:: do_UpScaleWeightMap Time")
                                            #endif
      // Release DownSizedWeightMapBuf[i] because it's no longer needed.
    &&  releaseDownSizedWeightMapBuf()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&  DbgTmr.print("HdrProfiling2:: releaseDownSizedWeightMapBuf Time")
                                            #endif
    ;

    if(mHdrRoundTotal==1 || mHdrRound==2) {
        ret = ret
      //  ()  Release OriWeightMapBuf, because it's not needed anymore. Note: Some info of OriWeightMap are needed when requestBlurredWeightMapBuf(), so must release it after requestBlurredWeightMapBuf().
        &&  releaseOriWeightMapBuf()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&  DbgTmr.print("HdrProfiling2:: releaseOriWeightMapBuf Time")
                                            #endif
        ;
    }

    FUNCTION_LOG_END;
    return ret;
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::
requestSourceImgBuf(void)
{
    FUNCTION_LOG_START;
    MBOOL    ret = MTRUE;
    MUINT32 u4OutputFrameNum = OutputFrameNumGet();

    MY_VERB("[requestBufs] mu4SourceSize: %d.", mu4SourceSize);
    for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
    {
#if    HDR_SPEEDUP_MALLOC == 1
        if(i==0) {
            //mpSourceImgBuf[0] has been allocated in allocateMemoryTask()
            continue;
        }
#endif

        char szPoolName[100];
        szPoolName[0] = '\0';
        ::sprintf(szPoolName, "%s%d", "HdrSrcImgBuf", i);

        mpSourceImgBuf[i].size = mu4SourceSize;
        if (allocMem(&mpSourceImgBuf[i]))    // mpSourceImgBuf[i].virtAddr is NULL, allocation fail.
        {
            MY_ERR("[requestBufs] mpSourceImgBuf[%d] fails to request %d bytes.", i, mu4SourceSize);
            ret = MFALSE;
            goto lbExit;
        }

        MY_VERB("[requestBufs] mpSourceImgBuf[%d].virtAddr: %zu.", i, mpSourceImgBuf[i].virtAddr);
        MY_VERB("[requestBufs] mpSourceImgBuf[%d].phyAddr : %zu.", i, mpSourceImgBuf[i].phyAddr);
        MY_VERB("[requestBufs] mpSourceImgBuf[%d].size: %d.", i, mpSourceImgBuf[i].size);
    }

lbExit:
    if    ( ! ret )
    {
        releaseSourceImgBuf();
    }

    FUNCTION_LOG_END;
    return    ret;
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::
releaseSourceImgBuf(void)
{
    FUNCTION_LOG_START;
    MBOOL    ret = MTRUE;
    MUINT32 u4OutputFrameNum = OutputFrameNumGet();

    for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
    {
        deallocMem(&mpSourceImgBuf[i]);
    }

    FUNCTION_LOG_END;
    return    ret;
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::
requestFirstRunSourceImgBuf(void)
{
    FUNCTION_LOG_START;
    MBOOL    ret = MTRUE;

    MUINT32 u4OutputFrameNum = OutputFrameNumGet();

    mu4FirstRunSourceSize = mu4W_first * mu4H_first * 3/2;    // I420 Size.

    MY_VERB("[requestBufs] mu4SourceSize: %d.", mu4FirstRunSourceSize);
    for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
    {
#if    HDR_SPEEDUP_MALLOC == 1
        if(i==0) {
            //mpFirstRunSourceImgBuf[0] has been allocated in allocateMemoryTask()
            continue;
        }
#endif

        char szPoolName[100];
        szPoolName[0] = '\0';
        ::sprintf(szPoolName, "%s%d", "HdrFirstRunSrcImgBuf", i);


        mpFirstRunSourceImgBuf[i].size = mu4FirstRunSourceSize;
        if (allocMem(&mpFirstRunSourceImgBuf[i]))    // mpSourceImgBuf[i].virtAddr is NULL, allocation fail.
        {
            MY_ERR("[requestBufs] mpSourceImgBuf[%d] fails to request %d bytes.", i, mu4SourceSize);
            ret = MFALSE;
            goto lbExit;
        }

        MY_VERB("[requestBufs] mpFirstRunSourceImgBuf[%d].virtAddr: %zu.", i, mpFirstRunSourceImgBuf[i].virtAddr);
        MY_VERB("[requestBufs] mpFirstRunSourceImgBuf[%d].phyAddr : %zu.", i, mpFirstRunSourceImgBuf[i].phyAddr);
        MY_VERB("[requestBufs] mpFirstRunSourceImgBuf[%d].size: %d.", i, mpFirstRunSourceImgBuf[i].size);
    }

lbExit:
    if    ( ! ret )
    {
        releaseFirstRunSourceImgBuf();
    }

    FUNCTION_LOG_END;
    return    ret;
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::
releaseFirstRunSourceImgBuf(void)
{
    FUNCTION_LOG_START;
    MBOOL    ret = MTRUE;
    MUINT32 u4OutputFrameNum = OutputFrameNumGet();

    for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
    {
        deallocMem(&mpFirstRunSourceImgBuf[i]);
    }

    FUNCTION_LOG_END;
    return    ret;
}

///////////////////////////////////////////////////////////////////////////
/// @brief Request Small Image buffers.
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
HdrShot::
requestSmallImgBuf(void)
{
    FUNCTION_LOG_START;
    MBOOL    ret = MTRUE;

    mu4SmallImgSize = mu4W_small * mu4H_small;    // Y800 size.

    switch (mu4OutputFrameNum)    // Allocate buffers according to u4OutputFrameNum, note that there are no "break;" in case 3/case 2.
    {
        case 3:
        mpSmallImgBuf[2].size = mu4SmallImgSize;
        if(allocMem(&mpSmallImgBuf[2]))
            ret = MFALSE;
        case 2:
        mpSmallImgBuf[1].size = mu4SmallImgSize;
        if(allocMem(&mpSmallImgBuf[1]))
            ret = MFALSE;

#if    HDR_SPEEDUP_MALLOC == 0
        case 1:
        mpSmallImgBuf[0].size = mu4SmallImgSize;
        if(allocMem(&mpSmallImgBuf[0]))
            ret = MFALSE;
        break;
#endif
    }

    for (MUINT32 i = 0; i < mu4OutputFrameNum; i++)
    {
        MY_VERB("[requestSmallImgBuf] mu4SmallImgSize: %d.", mu4SmallImgSize);
        MY_VERB("[requestSmallImgBuf] mpSmallImgBuf[%d].virtAddr: %zu.",    i, mpSmallImgBuf[i].virtAddr);
        MY_VERB("[requestSmallImgBuf] mpSmallImgBuf[%d].phyAddr : %zu.",    i, mpSmallImgBuf[i].phyAddr);
        MY_VERB("[requestSmallImgBuf] mpSmallImgBuf[%d].size: %d.",        i, mpSmallImgBuf[i].size);
    }

lbExit:
    if    ( ! ret )
    {
        releaseSmallImgBuf();
    }

    FUNCTION_LOG_END;
    return    ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::
releaseSmallImgBuf(void)
{
    FUNCTION_LOG_START;
    MBOOL    ret = MTRUE;

    for (MUINT32 i = 0; i < mu4OutputFrameNum; i++)
    {
        // For SmallImg Buffer.
        deallocMem(&mpSmallImgBuf[i]);
    }

    FUNCTION_LOG_END;
    return    ret;
}

///////////////////////////////////////////////////////////////////////////
/// @brief Request SE Image buffers.
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
HdrShot::
requestSEImgBuf(void)
{
    FUNCTION_LOG_START;
    MBOOL    ret = MTRUE;
    MY_DBG("[requestSEImgBuf] - E. u4OutputFrameNum: %d.", mu4OutputFrameNum);

    mu4SEImgSize = mu4W_se * mu4H_se;    // Y800 size.

    switch (mu4OutputFrameNum)    // Allocate buffers according to u4OutputFrameNum, note that there are no "break;" in case 3/case 2.
    {
        case 3:
        MY_DBG("[requestSEImgBuf] - alloc mpSEImgBuf[2].");
        mpSEImgBuf[2].size = mu4SEImgSize;
        if(allocMem(&mpSEImgBuf[2]))
            ret = MFALSE;
        case 2:
        MY_DBG("[requestSEImgBuf] - alloc mpSEImgBuf[1].");
        mpSEImgBuf[1].size = mu4SEImgSize;
        if(allocMem(&mpSEImgBuf[1]))
            ret = MFALSE;
        case 1:
        MY_DBG("[requestSEImgBuf] - alloc mpSEImgBuf[0].");
        mpSEImgBuf[0].size = mu4SEImgSize;
        if(allocMem(&mpSEImgBuf[0]))
            ret = MFALSE;
        break;
    }

    for (MUINT32 i = 0; i < mu4OutputFrameNum; i++)
    {
        MY_VERB("[requestSEImgBuf] mu4SEImgSize: %d.", mu4SEImgSize);
        MY_VERB("[requestSEImgBuf] mpSEImgBuf[%d].virtAddr: %zu.",    i, mpSEImgBuf[i].virtAddr);
        MY_VERB("[requestSEImgBuf] mpSEImgBuf[%d].phyAddr : %zu.",    i, mpSEImgBuf[i].phyAddr);
        MY_VERB("[requestSEImgBuf] mpSEImgBuf[%d].size(): %d.",        i, mpSEImgBuf[i].size);
    }

lbExit:
    if    ( ! ret )
    {
        releaseSEImgBuf();
    }

    FUNCTION_LOG_END;
    return    ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::
releaseSEImgBuf(void)
{
    FUNCTION_LOG_START;
    MBOOL    ret = MTRUE;

    for (MUINT32 i = 0; i < mu4OutputFrameNum; i++)
    {
        // For SE Image Buffer.
        deallocMem(&mpSEImgBuf[i]);
    }

    FUNCTION_LOG_END;
    return    ret;
}



///////////////////////////////////////////////////////////////////////////
/// @brief Request HDR working buffers.
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
HdrShot::
requestHdrWorkingBuf(void)
{
    FUNCTION_LOG_START;
    MBOOL    ret = MTRUE;

    // compare the size of HDR & MAV
    // a)    HDR
    MUINT32 mavBufferSize = 0;
    MUINT32 hdrBufferSize = 0;
    hdrBufferSize = mpHdrHal->HdrWorkingBuffSizeGet();
    MY_DBG("hdrBufferSize=%d", hdrBufferSize);

    // b)    MAV
    MY_DBG("mu4W_small=%d mu4H_small=%d", mu4W_small, mu4H_small);
    ret = ret && mpHdrHal->MavWorkingBuffSizeGet(mu4W_small, mu4H_small, &mavBufferSize);
    if(!ret) {
        MY_ERR("can't get mav working buffer size");
        ret = MFALSE;
        goto lbExit;
    }
    MY_DBG("mavBufferSize=%d", mavBufferSize);

    // c) use large one as common buffer
    mu4HdrWorkingBufSize = (hdrBufferSize>mavBufferSize) ? hdrBufferSize : mavBufferSize;
    MY_DBG("mu4HdrWorkingBufSize=%d", mu4HdrWorkingBufSize);

    // d) For HDR Working Buffer.
    mpHdrWorkingBuf.size = mu4HdrWorkingBufSize;
    //if(allocMem_User(&mpHdrWorkingBuf, MFALSE, MFALSE)) {
    if(allocMem_Kernel(&mpHdrWorkingBuf)) {
        ret = MFALSE;
        goto lbExit;
    }

    MY_DBG("[requestHdrWorkingBuf] mu4HdrWorkingBufSize    : %d.", mu4HdrWorkingBufSize);
    MY_DBG("[requestHdrWorkingBuf] mpHdrWorkingBuf.virtAddr: %zu.",    mpHdrWorkingBuf.virtAddr);
    MY_DBG("[requestHdrWorkingBuf] mpHdrWorkingBuf.phyAddr : %zu.",    mpHdrWorkingBuf.phyAddr);
    MY_DBG("[requestHdrWorkingBuf] mpHdrWorkingBuf.size    : %d.",        mpHdrWorkingBuf.size);

lbExit:
    if    ( ! ret )
    {
        releaseHdrWorkingBuf();
    }

    FUNCTION_LOG_END;
    return    ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::
releaseHdrWorkingBuf(void)
{
    FUNCTION_LOG_START;
    MBOOL    ret = MTRUE;

    // For HDR Working Buffer.
    #if 0
    if(mpHdrWorkingBuf.virtAddr) {
        free((void*)mpHdrWorkingBuf.virtAddr);
        mTotalBufferSize -= mpHdrWorkingBuf.size;
        mTotalUserBufferSize -= mpHdrWorkingBuf.size;
        MY_DBG("deallocMem total=%d user=%d kernel=%d\n"
            , mTotalBufferSize
            , mTotalUserBufferSize
            , mTotalKernelBufferSize);
    }
    mpHdrWorkingBuf.virtAddr = 0;
    #else
        deallocMem(&mpHdrWorkingBuf);
    #endif

    FUNCTION_LOG_END;
    return    ret;
}

MUINT32
HdrShot::
getAlignedSize(MUINT32 const u4Size)
{
    return (u4Size + (L1_CACHE_BYTES)) & ~(L1_CACHE_BYTES-1);
}


///////////////////////////////////////////////////////////////////////////
/// @brief Request Original WeightingTable buffers.
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
HdrShot::
requestOriWeightMapBuf(void)
{
    FUNCTION_LOG_START;
    MBOOL    ret = MTRUE;

    //       Allocate memory for original and blurred Weighting Map.
    MUINT32 u4Size = sizeof(HDR_PIPE_WEIGHT_TBL_INFO*) * mu4OutputFrameNum;
    MUINT32 u4AlignedSize = getAlignedSize(u4Size);
    MUINT32 u4TableSize = sizeof(HDR_PIPE_WEIGHT_TBL_INFO);
    MUINT32 u4AlignedTableSize = getAlignedSize(u4TableSize);
    MY_VERB("[requestOriWeightMapBuf] u4Size: %d. u4AlignedSize: %d. u4TableSize: %d. u4AlignedTableSize: %d.", u4Size, u4AlignedSize, u4TableSize, u4AlignedTableSize);

    OriWeight = (HDR_PIPE_WEIGHT_TBL_INFO**)memalign(L1_CACHE_BYTES, u4AlignedSize);
    if(OriWeight == nullptr) {
        MY_ERR("OriWeight == nullptr");
        return MFALSE;
    }
    for (MUINT32 i = 0; i < mu4OutputFrameNum; i++) {
        OriWeight[i] = (HDR_PIPE_WEIGHT_TBL_INFO*)memalign(L1_CACHE_BYTES, u4AlignedTableSize);
        if (OriWeight[i] == nullptr) {
            MY_ERR("OriWeight[%d] is nullptr", i, OriWeight[i]);
            return MFALSE;
        }
    }

    //[ION]
    mHdrSetBmapInfo.bmap_width = mu4W_yuv / 2;
    mHdrSetBmapInfo.bmap_height = mu4H_yuv / 2;
    for (MUINT32 i = 0; i < mu4OutputFrameNum; i++)
    {
        mWeightingBuf[i].size = mu4W_yuv * mu4H_yuv / 4;
        if(allocMem(&mWeightingBuf[i])) {
            ret = MFALSE;
            goto lbExit;
        }
        mHdrSetBmapInfo.bmap_image_addr[i] = (MUINT8 *)mWeightingBuf[i].virtAddr;
        MY_DBG("[requestOriWeightMapBuf] addr[%d] = %zu", i, mWeightingBuf[i].virtAddr);
    }
    mHdrSetBmapInfo.bmap_image_size = mWeightingBuf[0].size * mu4OutputFrameNum;

lbExit:
    if    ( ! ret )
    {
        releaseOriWeightMapBuf();
    }

    FUNCTION_LOG_END;
    return    ret;
}


///////////////////////////////////////////////////////////////////////////
/// @brief Release Original WeightingTable buffers.
///
///    Note: Some info of OriWeightMap are needed when requestBlurredWeightMapBuf(),
///          so must release it after requestBlurredWeightMapBuf().
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
HdrShot::
releaseOriWeightMapBuf(void)
{
    FUNCTION_LOG_START;
    MBOOL    ret = MTRUE;

    if (OriWeight)
    {
        for (MUINT32 i = 0; i < mu4OutputFrameNum; i++)
        {
            deallocMem(&mWeightingBuf[i]);
        }

        delete [] OriWeight;
        OriWeight = NULL;
    }

    FUNCTION_LOG_END;
    return    ret;

}


///////////////////////////////////////////////////////////////////////////
/// @brief Request DownSizedWeightMap buffers.
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
HdrShot::
requestDownSizedWeightMapBuf(void)
{
    FUNCTION_LOG_START;
    MBOOL    ret = MTRUE;

    if(mHdrRound == 1) {
        mu4W_dsmap = (OriWeight[0]->weight_table_width+31)  / 32;
        mu4H_dsmap = (OriWeight[0]->weight_table_height+31) / 32;
    } else {
        mu4W_dsmap = (OriWeight[0]->weight_table_width+39)  / 40;
        mu4H_dsmap = (OriWeight[0]->weight_table_height+39) / 40;
    }
    //MT6589 only accept odd output
    mu4W_dsmap = (mu4W_dsmap+1)&~1;
    mu4H_dsmap = (mu4H_dsmap+1)&~1;

    //       Calculate width/height of Down-sized Weighting Map.
    mu4DownSizedWeightMapSize = mu4W_dsmap * mu4H_dsmap;    // Y800 size.
    //       Allocate memory for Down-sized Weighting Map.
    switch (mu4OutputFrameNum)    // Allocate buffers according to u4OutputFrameNum, note that there are no "break;" in case 3/case 2.
    {
        case 3:
        mpDownSizedWeightMapBuf[2].size = mu4DownSizedWeightMapSize;
        if(allocMem(&mpDownSizedWeightMapBuf[2]))
            ret = MFALSE;
        case 2:
        mpDownSizedWeightMapBuf[1].size = mu4DownSizedWeightMapSize;
        if(allocMem(&mpDownSizedWeightMapBuf[1]))
            ret = MFALSE;
        case 1:
        mpDownSizedWeightMapBuf[0].size = mu4DownSizedWeightMapSize;
        if(allocMem(&mpDownSizedWeightMapBuf[0]))
            ret= MFALSE;
        break;
    }

    for (MUINT32 i = 0; i < mu4OutputFrameNum; i++)
    {
        MY_VERB("[requestDownSizedWeightMapBuf] mu4DownSizedWeightMapSize: %d.", mu4DownSizedWeightMapSize);
        MY_VERB("[requestDownSizedWeightMapBuf] mpDownSizedWeightMapBuf[%d].virtAddr: %zu.",    i, mpDownSizedWeightMapBuf[i].virtAddr);
        MY_VERB("[requestDownSizedWeightMapBuf] mpDownSizedWeightMapBuf[%d].phyAddr : %zu.",    i, mpDownSizedWeightMapBuf[i].phyAddr);
        MY_VERB("[requestDownSizedWeightMapBuf] mpDownSizedWeightMapBuf[%d].size: %d.",        i, mpDownSizedWeightMapBuf[i].size);
    }

lbExit:
    if    ( ! ret )
    {
        releaseDownSizedWeightMapBuf();
    }

    FUNCTION_LOG_END;
    return    ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::
releaseDownSizedWeightMapBuf(void)
{
    FUNCTION_LOG_START;
    MBOOL    ret = MTRUE;

    for (MUINT32 i = 0; i < mu4OutputFrameNum; i++)
    {
        // For DownSized Weight Map Buffer.
        deallocMem(&mpDownSizedWeightMapBuf[i]);
    }

    FUNCTION_LOG_END;
    return    ret;
}


///////////////////////////////////////////////////////////////////////////
/// @brief Request Blurred WeightingTable buffers. Must execute after OriWeightTbl is gottn.
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
HdrShot::
requestBlurredWeightMapBuf(void)
{
    FUNCTION_LOG_START;
    MBOOL    ret = MTRUE;

    //       Allocate memory for original and blurred Weighting Map.
    MUINT32 u4Size = sizeof(HDR_PIPE_WEIGHT_TBL_INFO*) * mu4OutputFrameNum;
    MUINT32 u4AlignedSize = getAlignedSize(u4Size);
    MUINT32 u4TableSize = sizeof(HDR_PIPE_WEIGHT_TBL_INFO);
    MUINT32 u4AlignedTableSize = getAlignedSize(u4TableSize);
    MY_VERB("[requestBlurredWeightMapBuf] u4Size: %d. u4AlignedSize: %d. u4TableSize: %d. u4AlignedTableSize: %d.", u4Size, u4AlignedSize, u4TableSize, u4AlignedTableSize);

    BlurredWeight = (HDR_PIPE_WEIGHT_TBL_INFO**)memalign(L1_CACHE_BYTES, u4AlignedSize);

    for (MUINT32 i = 0; i < mu4OutputFrameNum; i++)
    {
        BlurredWeight[i] = (HDR_PIPE_WEIGHT_TBL_INFO*)memalign(L1_CACHE_BYTES, u4AlignedTableSize);

        // Init BlurredWeight[i], and allocate memory for Blurred Weighting Map.
        BlurredWeight[i]->weight_table_width  = OriWeight[i]->weight_table_width;
        BlurredWeight[i]->weight_table_height = OriWeight[i]->weight_table_height;
        mpBlurredWeightMapBuf[i].size = (BlurredWeight[i]->weight_table_width * BlurredWeight[i]->weight_table_height);
        if(allocMem(&mpBlurredWeightMapBuf[i])) {
            ret = MFALSE;
            goto lbExit;
        }
        BlurredWeight[i]->weight_table_data = (MUINT8*)mpBlurredWeightMapBuf[i].virtAddr;

    }
lbExit:
    if    ( ! ret )
    {
        releaseBlurredWeightMapBuf();
    }

    FUNCTION_LOG_END;
    return    ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::
releaseBlurredWeightMapBuf(void)
{
    FUNCTION_LOG_START;
    MBOOL    ret = MTRUE;
    MUINT32 u4OutputFrameNum = OutputFrameNumGet();

    // Free allocated memory
    if (BlurredWeight)
    {
        for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
        {
            deallocMem(&mpBlurredWeightMapBuf[i]);
        }
        delete [] BlurredWeight;
        BlurredWeight = NULL;
    }

    FUNCTION_LOG_END;
    return    ret;

}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::
releasePostviewImgBuf()
{
    FUNCTION_LOG_START;
    MBOOL    ret = MTRUE;

    deallocMem(&mpPostviewImgBuf);

    FUNCTION_LOG_END;
    return    ret;
}


#if 1
/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::
requestBlendingBuf()
{
    FUNCTION_LOG_START;
    MBOOL    ret = MTRUE;

    mBlendingBuf.size = mu4W_yuv * mu4H_yuv * 3/2;
    if(allocMem(&mBlendingBuf)) {
        ret = MFALSE;
        goto lbExit;
    }

lbExit:
    if    ( ! ret )
    {
        releaseBlendingBuf();
    }

    FUNCTION_LOG_END;
    return    ret;
}


MBOOL
HdrShot::
releaseBlendingBuf()
{
    FUNCTION_LOG_START;
    MBOOL    ret = MTRUE;

    deallocMem(&mBlendingBuf);

    FUNCTION_LOG_END;
    return    ret;
}
#endif

/*******************************************************************************
*
*******************************************************************************/
unsigned int
HdrShot::
dumpToFile(
    char const *fname,
    unsigned char *pbuf,
    unsigned int size
)
{
    int nw, cnt = 0;
    unsigned int written = 0;

    MY_DBG("opening file [%s]\n", fname);
    int fd = open(fname, O_RDWR | O_CREAT, S_IRWXU);
    if (fd < 0) {
        MY_ERR("failed to create file [%s]: %s", fname, strerror(errno));
        return 0x80000000;
    }

    MY_DBG("writing %d bytes to file [%s]\n", size, fname);
    while (written < size) {
        nw = ::write(fd, pbuf + written, size - written);
        if (nw < 0) {
            MY_ERR("failed to write to file [%s]: %s", fname, strerror(errno));
            break;
        }
        written += nw;
        cnt++;
    }
    MY_DBG("done writing %d bytes to file [%s] in %d passes\n", size, fname, cnt);
    ::close(fd);

    return 0;
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::
handleYuvData(MUINT8* const puBuf, MUINT32 const u4Size)
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;
    MY_DBG("[handleYuvData] (puBuf, size) = (%p, %d)", puBuf, u4Size);

    if(mTestMode) {
        MY_ERR("[%s] mTestMode", __FUNCTION__);
        return MTRUE;
    }

#if !HDR_DEBUG_SKIP_HANDLER
#endif

    FUNCTION_LOG_END;
    return ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::
GetStride(MUINT32 srcWidth, EImageFormat srcFormat, MUINT32 *pStride)
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

    switch(srcFormat) {
        case eImgFmt_NV12:
        case eImgFmt_NV21:
        case eImgFmt_YUY2:
            pStride[0] = pStride[1] = pStride[2] = srcWidth;
            break;
        case eImgFmt_I420:
        case eImgFmt_YV12:
            pStride[0] = srcWidth;
            pStride[1] = pStride[2] = srcWidth / 2;
            break;
        case eImgFmt_Y800:
            pStride[0] = srcWidth;
            pStride[1] = pStride[2] = 0;
            break;
        default:
            pStride[0] = pStride[1] = pStride[2] = srcWidth;
            MY_ERR("GetStride: unspported format %d", srcFormat);
            ret = MFALSE;
    }

    FUNCTION_LOG_END;
    return ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::
CDPResize(IMEM_BUF_INFO *srcMem, MUINT32 srcWidth, MUINT32 srcHeight, EImageFormat srcFormat,
    IMEM_BUF_INFO *desMem, MUINT32 desWidth, MUINT32 desHeight, EImageFormat desFormat,
    MUINT32 rotation)
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

    IMEM_BUF_INFO tempInfo[2];
    MUINT32 tempWidth[2];
    MUINT32 tempHeight[2];
    MFLOAT tempSizeMulti = (MFLOAT)srcMem->size / (srcWidth*srcHeight);

     //init
    if((desWidth>32*srcWidth)
            || (desHeight>32*srcHeight)) {
        //prepare source
        tempWidth[0] = srcWidth;
        tempHeight[0] = srcHeight;
        MBOOL isFirstRun = MTRUE;
        tempInfo[0] = *srcMem;

        while(1) {
            //prepare target
            MY_DBG("[CDPResize] - prepare target");
            tempWidth[1] = desWidth;
            tempHeight[1] = desHeight;

            while(tempWidth[1] > tempWidth[0]*32)
                tempWidth[1] = (tempWidth[1]+31)/32;
            while(tempHeight[1] > tempHeight[0]*32)
                tempHeight[1] = (tempHeight[1]+31)/32;
            tempWidth[1] = (tempWidth[1]+1)&~1;
            tempHeight[1] = (tempHeight[1]+1)&~1;
            MY_DBG("[CDPResize] - desWidth=%d desHeight=%d", desWidth, desHeight);
            MY_DBG("[CDPResize] - tempWidth[0]=%d tempHeight[0]=%d", tempWidth[0], tempHeight[0]);
            MY_DBG("[CDPResize] - tempWidth[1]=%d tempHeight[1]=%d", tempWidth[1], tempHeight[1]);

            //scale up - last round
            if(tempWidth[1]==desWidth && tempHeight[1]==desHeight) {
                MY_DBG("[CDPResize] - scale up - final round");
                MBOOL ret;
                ret = CDPResize_simple(&tempInfo[0], tempWidth[0], tempHeight[0], srcFormat
                    , desMem, desWidth, desHeight, desFormat
                    , rotation);
                #if 1
                deallocMem(&tempInfo[0]);
                #else
                if(!isFirstRun)
                    deallocMem(&tempInfo[0]);
                #endif
                return ret;
            }

            //scale up
            MY_DBG("[CDPResize] - scale up");
            tempInfo[1].size = tempWidth[1] * tempHeight[1] * tempSizeMulti;
            if(allocMem(&tempInfo[1])) {
                ret = MFALSE;
                goto lbExit;
            }
            CDPResize_simple(&tempInfo[0], tempWidth[0], tempHeight[0], srcFormat
                , &tempInfo[1], tempWidth[1], tempHeight[1], srcFormat
                , rotation);
            if(!isFirstRun)
                deallocMem(&tempInfo[0]);
            tempWidth[0] = tempWidth[1];
            tempHeight[0] = tempHeight[1];
            tempInfo[0] = tempInfo[1];

            isFirstRun = MFALSE;
        }

    }

    ret = CDPResize_simple(srcMem, srcWidth, srcHeight, srcFormat,
            desMem, desWidth, desHeight, desFormat, rotation);
lbExit:
    FUNCTION_LOG_END;
    return ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::
CDPResize_simple(IMEM_BUF_INFO *srcMem, MUINT32 srcWidth, MUINT32 srcHeight, EImageFormat srcFormat,
    IMEM_BUF_INFO *desMem, MUINT32 desWidth, MUINT32 desHeight, EImageFormat desFormat,
    MUINT32 rotation)
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

    MY_DBG("[CDPResize] - srcMem=%p", srcMem);
    MY_DBG("[CDPResize] - srcWidth=%d, srcHeight=%d", srcWidth, srcHeight);
    MY_DBG("[CDPResize] - srcFormat=%d", srcFormat);
    MY_DBG("[CDPResize] - desMem=%p", desMem);
    MY_DBG("[CDPResize] - desWidth=%d, desHeight=%d", desWidth, desHeight);
    MY_DBG("[CDPResize] - desFormat=%d", desFormat);

    MUINT32 u4Stride[3] = {0, 0, 0};

    // (1). Create Instance
    MY_DBG("[CDPResize] - (1). Create Instance");
    NSCamHW::ImgInfo rSourceImgInfo(srcFormat, srcWidth, srcHeight);
    NSCamHW::BufInfo rSourceBufInfo(srcMem->size, srcMem->virtAddr, srcMem->phyAddr, srcMem->memID);
    GetStride(srcWidth, srcFormat, u4Stride);
    NSCamHW::ImgBufInfo rSourceImgBufInfo(rSourceImgInfo, rSourceBufInfo, u4Stride);
    MY_DBG("[CDPResize] - source stride %d,%d,%d", u4Stride[0], u4Stride[1], u4Stride[2]);

    NSCamShot::ISImager *pISImager = NSCamShot::ISImager::createInstance(rSourceImgBufInfo);
    if(!pISImager) {
        return MFALSE;
    }

    // (2). Set Output info for small image
    MY_DBG("[CDPResize] - (2). Set Output info for small image");
    NSCamHW::BufInfo rTargetBufInfo(desMem->size, desMem->virtAddr, desMem->phyAddr, desMem->memID);

    // (3). init setting
    MY_DBG("[CDPResize] - (3). init setting");
    pISImager->setTargetBufInfo(rTargetBufInfo);
    pISImager->setFormat(desFormat);
    pISImager->setRotation(rotation);
    pISImager->setFlip(0);
    pISImager->setResize(desWidth, desHeight);
    pISImager->setEncodeParam(1, 90);
    pISImager->setROI(Rect(0, 0, srcWidth, srcHeight));
    MY_DBG("[CDPResize] - before execute() t5");
    ret = pISImager->execute();
    MY_DBG("[CDPResize] - after execute()");
    pISImager->destroyInstance();
    MY_DBG("[CDPResize] - finish");

lbExit:
    FUNCTION_LOG_END;
    return ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::
createSmallImg(void)
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;
    MBOOL  ret = MTRUE;
    MUINT32 u4OutputFrameNum = OutputFrameNumGet();

#if (HDR_PROFILE_CAPTURE)
    MyDbgTimer DbgTmr("createSmallImg");
#endif

    for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
    {
        ret = CDPResize(
            &mpSourceImgBuf[i], mu4W_yuv, mu4H_yuv, eImgFmt_I420,
            &mpSmallImgBuf[i], mu4W_small, mu4H_small, eImgFmt_Y800, 0);
    }


#if (HDR_PROFILE_CAPTURE)
    DbgTmr.print("HdrProfiling:: createSmallImg Time");
#endif

    FUNCTION_LOG_END;
    return    ret;
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::
createSEImg(void)
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;
    MBOOL  ret = MTRUE;
    MUINT32 u4OutputFrameNum = OutputFrameNumGet();

#if (HDR_PROFILE_CAPTURE)
    MyDbgTimer DbgTmr("createSEImg");
#endif

    for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
    {
        MY_DBG("[createSEImg] - CDP %d/%d.", i, u4OutputFrameNum);
        ret = ret
        &&    CDPResize(&mpSmallImgBuf[i], mu4W_small, mu4H_small, eImgFmt_Y800,
                        &mpSEImgBuf[i], mu4W_se, mu4H_se, eImgFmt_Y800, 0);
    }

#if (HDR_PROFILE_CAPTURE)
    DbgTmr.print("HdrProfiling:: createSEImg Time");
#endif

    if(HDR_DEBUG_SAVE_SE_IMAGE || mDebugMode) {
        for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
        {
            char szFileName[100];
            ::sprintf(szFileName, HDR_DEBUG_OUTPUT_FOLDER "%04d_4_mpSEImgBuf[%d]_%dx%d_r%d.y", mu4RunningNumber, i, mu4W_se, mu4H_se, mHdrRound);
            dumpToFile(szFileName, (MUINT8*)mpSEImgBuf[i].virtAddr, mu4SEImgSize);
        }
    }

    FUNCTION_LOG_END;
    return    ret;
}

MVOID*
HdrShot::
allocateCaptureMemoryTask_First(MVOID* arg)
{
    SetThreadProp(SCHED_OTHER, -20);

    FUNCTION_LOG_START;
    MUINTPTR     ret = MTRUE;
    HdrShot *self = (HdrShot*)arg;

    MUINT32    i = 0;

    //extraced from requestSourceImgBuf()
    self->mpSourceImgBuf[i].size = self->mu4SourceSize;
    if (self->allocMem(&self->mpSourceImgBuf[i]))    // mpSourceImgBuf[i].virtAddr is NULL, allocation fail.
    {
        MY_ERR("[requestBufs] mpSourceImgBuf[%d] fails to request %d bytes.", i, self->mu4SourceSize);
        ret = MFALSE;
        goto lbExit;
    }

    //extraced from requestFirstRunSourceImgBuf()
    self->mu4FirstRunSourceSize = self->mu4W_first * self->mu4H_first * 3/2;    // I420 Size.
    //MY_VERB("[requestBufs] mu4SourceSize: %d.", mu4FirstRunSourceSize);
    self->mpFirstRunSourceImgBuf[i].size = self->mu4FirstRunSourceSize;
    if (self->allocMem(&self->mpFirstRunSourceImgBuf[i]))    // mpSourceImgBuf[i].virtAddr is NULL, allocation fail.
    {
        MY_ERR("[requestBufs] mpSourceImgBuf[%d] fails to request %d bytes.", i, self->mu4SourceSize);
        ret = MFALSE;
        goto lbExit;
    }

    //extraced from requestSmallImgBuf()
    self->mu4SmallImgSize = self->mu4W_small * self->mu4H_small;    // Y800 size.
    self->mpSmallImgBuf[i].size = self->mu4SmallImgSize;
    if(self->allocMem(&self->mpSmallImgBuf[i]))
    {
        MY_ERR("[requestBufs] mpSmallImgBuf[%d] fails to request %d bytes.", i, self->mu4SmallImgSize);
        ret = MFALSE;
        goto lbExit;
    }

    if(!ret) {
        MY_ERR("can't alloc memory");
    }
lbExit:
    FUNCTION_LOG_END;
    return (MVOID*)ret;
}


MVOID*
HdrShot::
allocateCaptureMemoryTask_Others(MVOID* arg)
{
    SetThreadProp(SCHED_OTHER, -20);

    FUNCTION_LOG_START;
    MUINTPTR     ret = MTRUE;

    HdrShot *self = (HdrShot*)arg;

    //allocate buffers for 2rd & 3nd capture
#if 1
    ret = ret
        &&    self->requestSourceImgBuf()
        &&    self->requestFirstRunSourceImgBuf()
        &&    self->requestSmallImgBuf()
        ;
#else
    ret = MFALSE;
#endif

    if(!ret) {
        MY_ERR("can't alloc memory");
    }
lbExit:
    FUNCTION_LOG_END;
    return (MVOID*)ret;
}


MVOID*
HdrShot::
allocateProcessMemoryTask(MVOID* arg)
{
    FUNCTION_LOG_START;
    MUINTPTR     ret = MTRUE;

    HdrShot *self = (HdrShot*)arg;

    //allocate buffers for MAV & HDR Core
    ret = ret
        &&    self->requestHdrWorkingBuf()
        ;

    if(!ret) {
        MY_ERR("can't alloc memory");
    }
lbExit:
    FUNCTION_LOG_END;
    return (MVOID*)ret;
}


MVOID*
HdrShot::
saveFileTask(MVOID* arg)
{
    FUNCTION_LOG_START;
    MUINTPTR     ret = MTRUE;

    HdrFileInfo *pFileInfo = (HdrFileInfo*)arg;
    dumpToFile(pFileInfo->filename.string()
                , pFileInfo->buffer
                , pFileInfo->size
                );
    delete pFileInfo->buffer;
    delete pFileInfo;

    pthread_detach(pthread_self());

    FUNCTION_LOG_END;
    return (MVOID*)ret;
}


HdrState_e
HdrShot::
GetHdrState(void)
{
    return mHdrState;
}

void
HdrShot::
SetHdrState(HdrState_e eHdrState)
{
    mHdrState = eHdrState;
}

MINT32
HdrShot::
mHalCamHdrProc(HdrState_e eHdrState __attribute__((unused)))
{
    return 0;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::
uninit()
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

    if (mpHdrHal)
    {
        mpHdrHal->uninit();
        mpHdrHal->destroyInstance();
        mpHdrHal = NULL;
    }

    if(mpIMemDrv) {
        mpIMemDrv->uninit();
        mpIMemDrv->destroyInstance();
        mpIMemDrv = NULL;
    }

    for (MUINT32 i = 0; i < OutputFrameNumGet(); i++) {
        if(mpCamExif[i]) {
            delete mpCamExif[i];
            mpCamExif[i] = 0;
        }
    }

    mu4W_yuv = mu4H_yuv = 0;

#if HDR_USE_THREAD
    SetHdrState(HDR_STATE_UNINIT);
    ::sem_post(&semHdrThread);
    ::sem_wait(&semHdrThreadEnd);
    MY_DBG("semHdrThreadEnd received.");
#endif  // HDR_USE_THREAD


    FUNCTION_LOG_END;
    return  ret;
}


/******************************************************************************
*
*******************************************************************************/
MUINT32
HdrShot::
allocMem_User(IMEM_BUF_INFO *memBuf, MBOOL touch, MBOOL mapping)
{
    FUNCTION_LOG_START;
    MBOOL ret = 0;
    SetThreadProp(SCHED_OTHER, -20);

    MY_DBG("touch=%d mapping=%d\n", touch, mapping);

    mTotalBufferSize += memBuf->size;
    mTotalUserBufferSize += memBuf->size;
    MY_DBG("allocMem size=%d\n", memBuf->size);
    MY_DBG("allocMem total=%d user=%d kernel=%d\n"
        , mTotalBufferSize
        , mTotalUserBufferSize
        , mTotalKernelBufferSize);

    memBuf->memID = -1;
    memBuf->virtAddr = (MUINTPTR)malloc(memBuf->size);
    if(!memBuf->virtAddr) {
        MY_ERR("malloc() error \n");
        ret = 1;
        goto lbExit;
    }

    if(touch){
        // assign pages in this SCHED_OTHER thread to avoid memory killer pending
        touchVirtualMemory((MUINT8*)memBuf->virtAddr, memBuf->size);
    }

    if(mapping) {
        ret = mpIMemDrv->mapPhyAddr(memBuf);
        if (ret) {
            MY_ERR("mpIMemDrv->mapPhyAddr() error");
            ret = 1;
            goto lbExit;
        }
    }

lbExit:
    SetThreadProp(mCapturePolicy, mCapturePriority);
    FUNCTION_LOG_END;
    return ret;
}


/******************************************************************************
*
*******************************************************************************/
MUINT32
HdrShot::
allocMem_Kernel(IMEM_BUF_INFO *memBuf)
{
    FUNCTION_LOG_START;
    MBOOL ret = 0;
    SetThreadProp(SCHED_OTHER, -20);

    mTotalBufferSize += memBuf->size;
    mTotalKernelBufferSize += memBuf->size;
    MY_DBG("allocMem size=%d\n", memBuf->size);
    MY_DBG("allocMem total=%d user=%d kernel=%d\n"
        , mTotalBufferSize
        , mTotalUserBufferSize
        , mTotalKernelBufferSize);

    ret = mpIMemDrv->allocVirtBuf(memBuf);
    if (ret) {
        MY_ERR("g_pIMemDrv->allocVirtBuf() error");
        goto lbExit;
    }

    ret = mpIMemDrv->mapPhyAddr(memBuf);
    if (ret) {
        MY_ERR("mpIMemDrv->mapPhyAddr() error");
        goto lbExit;
    }

lbExit:
    SetThreadProp(mCapturePolicy, mCapturePriority);
    FUNCTION_LOG_END;
    return ret;
}


MUINT32
HdrShot::
allocMem(IMEM_BUF_INFO *memBuf)
{
    FUNCTION_LOG_START;
    MBOOL     ret = MTRUE;
    //ret = allocMem_User(memBuf, MFALSE, MTRUE);
    ret = allocMem_Kernel(memBuf);

lbExit:
    FUNCTION_LOG_END;
    return ret;
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::
deallocMem(IMEM_BUF_INFO *memBuf)
{
    if(memBuf->virtAddr == 0) {
        return MTRUE;
    }

    mTotalBufferSize -= memBuf->size;
    if(memBuf->memID == -1) {
        mTotalUserBufferSize -= memBuf->size;
    } else {
        mTotalKernelBufferSize -= memBuf->size;
    }
    MY_DBG("deallocMem size=%d\n", memBuf->size);
    MY_DBG("deallocMem total=%d user=%d kernel=%d\n"
        , mTotalBufferSize
        , mTotalUserBufferSize
        , mTotalKernelBufferSize);


    if (mpIMemDrv->unmapPhyAddr(memBuf)) {
        MY_ERR("m_pIMemDrv->unmapPhyAddr() error");
    }

    if(memBuf->memID == -1) {
        free((void*)memBuf->virtAddr);
    } else {
        if (mpIMemDrv->freeVirtBuf(memBuf)) {
            MY_ERR("m_pIMemDrv->freeVirtBuf() error");
        }
    }
    memBuf->virtAddr = 0;

    return MTRUE;
}


MBOOL
HdrShot::
touchVirtualMemory(MUINT8* vm, MUINT32 size)
{
    FUNCTION_LOG_START;
    MBOOL    ret = MTRUE;
    for(MUINT32 i=0; i<size; i+=4096) {
        vm[i] = 0;
    }
    FUNCTION_LOG_END;
    return MTRUE;
}


MBOOL
HdrShot::
SetThreadProp(int policy, int priority)
{
#if !HDR_DEBUG_SKIP_MODIFY_POLICY
    //@see http://www.kernel.org/doc/man-pages/online/pages/man2/sched_setscheduler.2.html
    //int const policy    = pthreadAttr_ptr->sched_policy;
    //int const priority  = pthreadAttr_ptr->sched_priority;
    //MY_DBG("policy=%d, priority=%d", policy, priority);

    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);

    switch(policy)
    {
        //non-real-time
        case SCHED_OTHER:
            sched_p.sched_priority = 0;
            sched_setscheduler(0, policy, &sched_p);
            setpriority(PRIO_PROCESS, 0, priority);    //-20(high)~19(low)
            break;

        //real-time
        case SCHED_FIFO:
        default:
              sched_p.sched_priority = priority;    //1(low)~99(high)
            sched_setscheduler(0, policy, &sched_p);
    }
    //
    #if 0
    ::sched_getparam(0, &sched_p);
    ALOGD(
        "policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d-%d)"
        , policy, ::sched_getscheduler(0)
        , priority, getpriority(PRIO_PROCESS, 0), sched_p.sched_priority
    );
    #endif
#endif

    return MTRUE;
}

MBOOL
HdrShot::
GetThreadProp(int *policy, int *priority)
{
#if !HDR_DEBUG_SKIP_MODIFY_POLICY
    //@see http://www.kernel.org/doc/man-pages/online/pages/man2/sched_setscheduler.2.html
    struct sched_param sched_p;
    *policy = ::sched_getscheduler(0);

    switch(*policy)
    {
        //non-real-time
        case SCHED_OTHER:
            *priority = getpriority(PRIO_PROCESS, 0);    //-20(high)~19(low)
            break;

        //real-time
        case SCHED_FIFO:
        default:
            struct sched_param sched_p;
            ::sched_getparam(0, &sched_p);
            *priority = sched_p.sched_priority;
    }
#endif

    return MTRUE;
}


/******************************************************************************
*
*******************************************************************************/

MBOOL
HdrShot::
updateInfo_cam3()
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.mediatek.hdr.debug", value, "0");

    mDebugMode = atoi(value);
    MY_DBG("[updateInfo] - mDebugMode=%d", mDebugMode);

    //capture policy, priority
    GetThreadProp(&mCapturePolicy, &mCapturePriority);
    MY_DBG("[updateInfo] - mCapturePolicy=%d", mCapturePolicy);
    MY_DBG("[updateInfo] - mCapturePriority=%d", mCapturePriority);

    mu4W_yuv = mShotParam.mi4PictureWidth;
    mu4H_yuv = mShotParam.mi4PictureHeight;

    MY_DBG("!!! [updateInfo_cam3] HDR:width=%d,height=%d, mu4W_yuv:%d, mu4H_yuv:%d\n"
                    , mShotParam.mi4PictureWidth
                    , mShotParam.mi4PictureHeight
                    , mu4W_yuv
                    , mu4H_yuv
                    );

#if CUST_HDR_CAPTURE_POLICY==1
    {
        //  (1)  get sensor resolution
        SensorHal *pSensorHal = NULL;
        MUINT32 u4SensorWidth = 0;
        MUINT32 u4SensorHeight = 0;

        pSensorHal = SensorHal::createInstance();

        if (NULL == pSensorHal)
        {
            MY_ERR("pSensorHal is NULL");
            return 0;
        }
        pSensorHal->init();
        pSensorHal->sendCommand(SENSOR_DEV_MAIN
                                    , SENSOR_CMD_GET_SENSOR_FULL_RANGE
                                    , (int)&u4SensorWidth
                                    , (int)&u4SensorHeight
                                    , 0
                                    );
        MY_DBG("[updateInfo] SensorHal:sensor width:%d, height:%d\n", u4SensorWidth, u4SensorHeight);
        pSensorHal->uninit();
        pSensorHal->destroyInstance();

        //  (2)  Is capture size over sensor size ?
        if(mShotParam.mi4PictureWidth * mShotParam.mi4PictureHeight > u4SensorWidth  * u4SensorHeight)
        {
            float scaleRatio = (float)mShotParam.mi4PictureHeight / mShotParam.mi4PictureWidth;
            mu4W_yuv = u4SensorWidth;
            mu4H_yuv = u4SensorWidth * scaleRatio;
            mu4W_yuv &= ~0x01;
            mu4H_yuv &= ~0x01;
            MY_DBG("[updateInfo] HDR:width=%d,height=%d, mu4W_yuv:%d, mu4H_yuv:%d\n"
                    , mShotParam.mi4PictureWidth
                    , mShotParam.mi4PictureHeight
                    , mu4W_yuv
                    , mu4H_yuv
                    );
        }
    }
#endif
    mu4SourceSize = mu4W_yuv * mu4H_yuv * 3/2;    //eImgFmt_YV12
    MY_DBG("[updateInfo_cam3] - mu4SourceSize=%d", mu4SourceSize);

    //postview
    mPostviewWidth = mShotParam.mi4PostviewWidth;
    mPostviewHeight = mShotParam.mi4PostviewHeight;
    mPostviewFormat = eImgFmt_YV12;

    MY_DBG("!!! mPostviewWidth (%d)\n", mPostviewWidth);
    MY_DBG("!!! mPostviewHeight (%d)\n", mPostviewHeight);
    MY_DBG("!!! mPostviewFormat (%d)\n", mPostviewFormat);

    mHdrRoundTotal = 1;
    MY_DBG("[updateInfo_cam3] - mHdrRoundTotal=%d", mHdrRoundTotal);

    FUNCTION_LOG_END;
    return ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::
EVBracketCapture_cam3(void)
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;
    if(mfgIsForceBreak) {MY_DBG("force break at %s", __FUNCTION__); return MFALSE;}

#if (HDR_PROFILE_CAPTURE2)
    MyDbgTimer DbgTmr("EVBracketCapture");
#endif

    if (mu4RunningNumber >= 9999)
        mu4RunningNumber = 0;
    else
        mu4RunningNumber++;

    ret = ret
    &&    decideCaptureMode_cam3()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&  DbgTmr.print("HdrProfiling2:: decideCaptureMode Time")
                                            #endif
      &&    init_cam3()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&  DbgTmr.print("HdrProfiling2:: init Time")
                                            #endif
#if    HDR_SPEEDUP_MALLOC == 0
      //  ()  Request SmallImg Buffers.
    &&  requestSmallImgBuf()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&  DbgTmr.print("HdrProfiling2:: requestSmallImgBuf Time")
                                            #endif
    ;
#else
    ;

    //allocate buffer for first only for speed up capture time
    MUINT32    i = 0;
    if(ret) {
        #if HDR_SPEEDUP_BURSTSHOT
        pthread_create(&mCaptureIMemThread, NULL, HdrShot::allocateCaptureMemoryTask_All_cam3, this);
        #else
        //allocate buffers for first capture
        ret = ret && allocateCaptureMemoryTask_First(this);
        //allocate other buffers in thread for time saving
        pthread_create(&mCaptureIMemThread, NULL, HdrShot::allocateCaptureMemoryTask_Others, this);
        #endif
    }
#endif


    ret = ret
        // ()    set output as yuv & small
        && createSourceAndSmallImg_cam3();

lbExit:
    FUNCTION_LOG_END;
    return ret;
}



/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::
createSourceAndSmallImg_cam3(void)
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

#if    HDR_SPEEDUP_MALLOC == 1
    MUINT32    threadRet = 0;
    pthread_join(mCaptureIMemThread, (void**)&threadRet);
    mCaptureIMemThread = 0;
    if(!threadRet) {
        MY_ERR("join mCaptureIMemThread fail");
        return MFALSE;
    }
    pthread_create(&mProcessIMemThread, NULL, HdrShot::allocateProcessMemoryTask, this);
#endif

    FUNCTION_LOG_END;
    return ret;
}

/*******************************************************************************
*
*******************************************************************************/
MVOID*
HdrShot::
hdrProcess_cam3(MVOID* arg)
{
    FUNCTION_LOG_START;
    MUINTPTR    ret = MTRUE;
    #if 0
    ::prctl(PR_SET_NAME,"hdrProcessTask", 0, 0, 0);
    MY_DBG("[hdrProcess_cam3] SetThreadProp");
    SetThreadProp(SCHED_OTHER, ANDROID_PRIORITY_FOREGROUND);
    #endif

    HdrShot *self = (HdrShot*)arg;
    CHECK_OBJECT(self);
    #if 1  //TEST
    //mMemoryReady_pass2_first mMemoryReady_pass2_others, should come from add image
    for(MUINT32 i = 0; i < self->mu4OutputFrameNum; i ++){
        ret = ret && self->WAIT(&self->mMemoryReady_SrcLargeImg_cam3[i]);
    }

    for(MUINT32 i = 0; i < self->mu4OutputFrameNum; i ++){
        ret = ret && self->WAIT(&self->mMemoryReady_SrcSmallImg_cam3[i]);
    }
    #else
    self->::sem_wait(&self->mMemoryReady_pass2_first);
    #endif

    //ret = ret && self->WAIT(&self->mMemoryReady_DstImg_cam3[0]);
    //ret = ret && self->WAIT(&self->mMemoryReady_DstImg_cam3[1]);

    MY_DBG("hdrProcess_cam3 ::sem_wait mTrigger_alloc_working");

    //::sem_wait(&self->mTrigger_alloc_working);  //@todo move init mav instance before EVCapture

    MY_DBG("hdrProcess_cam3 ImageRegistratoin_cam3");

    ret = self->ImageRegistratoin_cam3();

    if(MTRUE != ret){
        MY_ERR("[hdrProcess_cam3] ImageRegistratoin_cam3 fail.");
        return MFALSE;
    }
    ret = self->WeightingMapGeneration();
    if(MTRUE != ret){
        MY_ERR("[hdrProcess_cam3] WeightingMapGeneration fail.");
        return MFALSE;
    }
    ret = self->Blending_cam3();
    if(MTRUE != ret){
        MY_ERR("[hdrProcess_cam3] Blending_cam3 fail.");
        return MFALSE;
    }
    ret = self->writeHDROutBuf_cam3();
    if(MTRUE != ret){
        MY_ERR("[hdrProcess_cam3] writeHDROutBuf_cam3 fail.");
        return MFALSE;
    }
    if(self->mCompleteCb && self->mpCbUser){
        MY_DBG("hdrProcess_cam3 mCompleteCb %zu",ret);
        self->mCompleteCb(self->mpCbUser,ret);
    }
    //
    pthread_detach(pthread_self());

lbExit:

    FUNCTION_LOG_END_MUM;
    return (MVOID*)ret;

}

/*******************************************************************************
*
*  if mu4OutputFrameNum = 2,
*       frame_index 0 is mpSourceImgBuf[0]
*       frame_index 1 is mpSmallImgBuf[0]
*       frame_index 2 is mpSourceImgBuf[1]
*       frame_index 3 is mpSmallImgBuf[1]
*  if mu4OutputFrameNum = 3,
*       frame_index 0 is mpSourceImgBuf[0]
*       frame_index 1 is mpSmallImgBuf[0]
*       frame_index 2 is mpSourceImgBuf[1]
*       frame_index 3 is mpSmallImgBuf[1]
*       frame_index 4 is mpSourceImgBuf[2]
*       frame_index 5 is mpSmallImgBuf[2]
*******************************************************************************/
MBOOL
HdrShot::
registerSrcSmallImg_cam3(MINT32 frame_index, const android::sp<IImageBuffer>  imgbuf)
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

    MY_DBG("[RegisterSrcSmallImg] - frame_index:%d",frame_index);

    if(frame_index <0 || frame_index > (static_cast<MINT32>(mu4OutputFrameNum) << 1)){

        MY_DBG("[RegisterSrcSmallImg] - invalid frame_index:%d",frame_index);
        return MFALSE;
    }
    if(imgbuf == NULL ){

        MY_DBG("[RegisterSrcSmallImg] - invalid imgbuf");
        return MFALSE;
    }
    if((frame_index %2) == 0){
        // mpSourceImgBuf
        if(SKIP_MEMCPY){
            memset((void*)&mpSourceImgBuf[(frame_index/2)],0,sizeof(mpSourceImgBuf[(frame_index/2)]));
            for(int i = 0; i < imgbuf.get()->getPlaneCount(); i ++){
                mpSourceImgBuf[(frame_index/2)].size += imgbuf.get()->getBufSizeInBytes(i);
            }
            MY_DBG("mpSourceImgBuf[%d].size:%d",(frame_index/2),mpSourceImgBuf[(frame_index/2)].size);
            mpSourceImgBuf[(frame_index/2)].virtAddr = imgbuf.get()->getBufVA(0);
            mpSourceImgBuf[(frame_index/2)].phyAddr = imgbuf.get()->getBufPA(0);
            MY_DBG("mpSourceImgBuf[%d].virtAddr:%p",(frame_index/2),(void*)mpSourceImgBuf[(frame_index/2)].virtAddr);
            MY_DBG("mpSourceImgBuf[%d].phyAddr:%p",(frame_index/2),(void*)mpSourceImgBuf[(frame_index/2)].phyAddr);
        } else {
            int size[3];
            int offset = 0;
            for(size_t i = 0; i < imgbuf.get()->getPlaneCount(); i ++){
                size[i] = imgbuf.get()->getBufSizeInBytes(i);
                MY_DBG("memcpy[%zu] dst(%p) size(%d)",i,(MUINT8*)mpSourceImgBuf[(frame_index/2)].virtAddr + offset,size[i]);
                memcpy((MUINT8*)mpSourceImgBuf[(frame_index/2)].virtAddr + offset, (MUINT8*)(imgbuf.get()->getBufVA(i)), size[i]);
                offset += size[i];
            }
        }
        if(CUST_HDR_DEBUG || HDR_DEBUG_SAVE_SOURCE_IMAGE || mDebugMode) {
            char szFileName[100];
            ::sprintf(szFileName, HDR_DEBUG_OUTPUT_FOLDER "%04d_1_mpSourceImgBuf[%d]_%dx%d.i420", mu4RunningNumber, (frame_index/2), mu4W_yuv, mu4H_yuv);
            //mpSourceImgBuf[(frame_index/2)]->saveToFile(szFileName);
            dumpToFile(szFileName, (MUINT8*)mpSourceImgBuf[(frame_index/2)].virtAddr, mu4SourceSize);
        }
        ANNOUNCE(&mMemoryReady_SrcLargeImg_cam3[(frame_index/2)]);
    } else {
        // mpSmallImgBuf
        if(SKIP_MEMCPY){
            memset((void*)&mpSmallImgBuf[(frame_index/2)],0,sizeof(mpSmallImgBuf[(frame_index/2)]));
            for(int i = 0; i < imgbuf.get()->getPlaneCount(); i ++){
                mpSmallImgBuf[(frame_index/2)].size += imgbuf.get()->getBufSizeInBytes(i);
            }
            MY_DBG("mpSmallImgBuf[%d].size:%d",(frame_index/2),mpSmallImgBuf[(frame_index/2)].size);
            mpSmallImgBuf[(frame_index/2)].virtAddr = imgbuf.get()->getBufVA(0);
            mpSmallImgBuf[(frame_index/2)].phyAddr = imgbuf.get()->getBufPA(0);
            MY_DBG("mpSmallImgBuf[%d].virtAddr:%p",(frame_index/2),(void*)mpSmallImgBuf[(frame_index/2)].virtAddr);
            MY_DBG("mpSmallImgBuf[%d].phyAddr:%p",(frame_index/2),(void*)mpSmallImgBuf[(frame_index/2)].phyAddr);
        } else {
            int size[3];
            int offset = 0;
            for(size_t i = 0; i < imgbuf.get()->getPlaneCount(); i ++){
                size[i] = imgbuf.get()->getBufSizeInBytes(i);
                MY_DBG("memcpy[%zu] dst(%p) size(%d)",i,(MUINT8*)mpSmallImgBuf[(frame_index/2)].virtAddr + offset,size[i]);
                memcpy((MUINT8*)mpSmallImgBuf[(frame_index/2)].virtAddr + offset, (MUINT8*)(imgbuf.get()->getBufVA(i)), size[i]);
                offset += size[i];
            }
        }
        if(HDR_DEBUG_SAVE_SMALL_IMAGE || mDebugMode) {
            char szFileName[100];
            ::sprintf(szFileName, HDR_DEBUG_OUTPUT_FOLDER "%04d_2_mpSmallImgBuf[%d]_%dx%d.y", mu4RunningNumber, (frame_index/2), mu4W_small, mu4H_small);
            //mpSmallImgBuf[(frame_index/2)]->saveToFile(szFileName);
            dumpToFile(szFileName, (MUINT8*)mpSmallImgBuf[(frame_index/2)].virtAddr, mu4SmallImgSize);
        }
        ANNOUNCE(&mMemoryReady_SrcSmallImg_cam3[(frame_index/2)]);
    }
    FUNCTION_LOG_END;
    return ret;
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::
registerOutputImg_cam3(MINT32 frame_index, sp<IImageBuffer> imgbuf)
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

    if(frame_index < 0 || frame_index > 1){
        MY_ERR("[registerOutputImg_cam3] - only support JPEG_YUV and THUMBNAIL_YUV");
        return MFALSE;
    }

    if(imgbuf == NULL){
        MY_ERR("[registerOutputImg_cam3] - invalid imgbuf");
        return MFALSE;
    }

    mHdrOutBuf_cam3[frame_index] =  imgbuf.get();
    ALOGD("registerOutputImg_cam3 mHdrOutBuf_cam3[%d]  (VA)addr:%zu",frame_index, mHdrOutBuf_cam3[frame_index]->getBufVA(0));

    for (MUINT32 i=0; i < mHdrOutBuf_cam3[frame_index]->getPlaneCount(); i++) {
        ALOGD("!!! registerOutputImg_cam3 getBufVA(%d)(%zu)", i,mHdrOutBuf_cam3[frame_index]->getBufVA(i));
        ALOGD("!!! registerOutputImg_cam3 getBufPA(%d)(%zu)", i,mHdrOutBuf_cam3[frame_index]->getBufPA(i));
    }


    ANNOUNCE(&mMemoryReady_DstImg_cam3[frame_index]);

    FUNCTION_LOG_END;
    return ret;
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::
process_cam3()
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;
    pthread_create(&mHDRProcessThread_cam3, NULL, HdrShot::hdrProcess_cam3, this);

    FUNCTION_LOG_END;
    return ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::
release_cam3()
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

    //  ()  HDR finished, clear HDR setting.
    do_HdrSettingClear();
    // Don't know exact time of lbExit in HDR flow, so release all again
    // (there is protection in each release function).
    if(!SKIP_MEMCPY){
        MY_DBG("release_cam3 releaseSourceImgBuf");
        releaseSourceImgBuf();
        //releaseFirstRunSourceImgBuf();
        MY_DBG("release_cam3 releaseSmallImgBuf");
        releaseSmallImgBuf();
    }
    releaseSEImgBuf();
    releaseHdrWorkingBuf();
    releaseOriWeightMapBuf();
    releaseDownSizedWeightMapBuf();
    releaseBlurredWeightMapBuf();
    releasePostviewImgBuf();
    releaseBlendingBuf();

    FUNCTION_LOG_END;
    return ret;
}


/*******************************************************************************
*
*******************************************************************************/

MBOOL
HdrShot::
ImageRegistratoin_cam3(void)
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;
    if(mfgIsForceBreak) {MY_DBG("force break at %s", __FUNCTION__); return MFALSE;}


#if (HDR_PROFILE_CAPTURE2)
    MyDbgTimer DbgTmr("capture");
#endif

    // (1)    Normalize
    #if HDR_USE_THREAD
    SetHdrState(HDR_STATE_NORMALIZATION);
    ::sem_post(&semHdrThread);
    MY_DBG("[capture] semHdrThread (HDR_STATE_NORMALIZATION) posted.");
    ::sem_wait(&semHdrThreadBack);
    MY_DBG("[capture] semHdrThreadBack (HDR_STATE_NORMALIZATION) received.");
    #else
    ret =
    //  ()  Normalize small images, and put them back to SmallImg[].
        do_Normalization()
        ;
    #endif  // HDR_USE_THREAD
                                            #if (HDR_PROFILE_CAPTURE2)
                                            DbgTmr.print("HdrProfiling2:: do_Normalization Time");
                                            #endif

    // (2)    SE
    ret = ret
      //  ()  Request SEImg Buffers.
    &&    requestSEImgBuf()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&  DbgTmr.print("HdrProfiling2:: requestSEImgBuf Time")
                                            #endif
      //  ()  Create SEImg (resize 3 Small Img to 3 SE Img).
    &&  createSEImg()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&  DbgTmr.print("HdrProfiling2:: createSEImg Time")
                                            #endif
      //  ()  Do SE to get GMV.
    &&  do_SE()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&  DbgTmr.print("HdrProfiling2:: do_SE Time")
                                            #endif
      //  ()  Release SEImg Buffers.
    &&  releaseSEImgBuf()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&  DbgTmr.print("HdrProfiling2:: releaseSEImgBuf Time")
                                            #endif
      ;

#if    HDR_SPEEDUP_MALLOC == 1
    if(ret){
        MUINT32    threadRet = 0;
        pthread_join(mProcessIMemThread, (void**)&threadRet);
        mProcessIMemThread = 0;
        if(!threadRet) {
            MY_ERR("join mProcessIMemThread fail");
            ret = MFALSE;
        }
    }
#else
    ret = ret && requestHdrWorkingBuf();
#endif

    // (3)    MAV
    #if HDR_USE_THREAD
  //  ()  Do Feature Extraciton.
    SetHdrState(HDR_STATE_FEATURE_EXTRACITON);
    ::sem_post(&semHdrThread);
    MY_DBG("[capture] semHdrThread (HDR_STATE_FEATURE_EXTRACITON) posted.");
    ::sem_wait(&semHdrThreadBack);
    MY_DBG("[capture] semHdrThreadBack (HDR_STATE_FEATURE_EXTRACITON) received.");
    #else
    ret = ret
      //  ()  Do Feature Extraciton.
    &&    do_FeatureExtraction()
        ;
    #endif  // HDR_USE_THREAD
                                            #if (HDR_PROFILE_CAPTURE2)
                                            DbgTmr.print("HdrProfiling2:: do_FeatureExtraction Time");
                                            #endif

    MY_DBG("[ImageRegistratoin_cam3] skip releaseSmallImgBuf ");
#if 0

    ret = ret
      //  ()  Release MAV working buffer, because it's not needed anymore.
    &&  releaseSmallImgBuf()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&  DbgTmr.print("HdrProfiling2:: releaseSmallImgBuf Time")
                                            #endif
      ;
#endif


lbExit:
    FUNCTION_LOG_END;
    return  ret;
}


/*******************************************************************************
*
*******************************************************************************/

MBOOL
HdrShot::
Blending_cam3(void)
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;
    if(mfgIsForceBreak) {MY_DBG("force break at %s", __FUNCTION__); return MFALSE;}

#if (HDR_PROFILE_CAPTURE2)
    MyDbgTimer DbgTmr("capture");
#endif


    // (A)    fusion
    ret = ret
    &&    requestBlendingBuf()    //[ION]
        //  ()  Do Fusion.
    &&    do_Fusion()
                                        #if (HDR_PROFILE_CAPTURE2)
                                        &&    DbgTmr.print("HdrProfiling2:: do_Fusion Time");
                                        #endif
    ;

    ret = ret
      //  ()  Release Blurred weighting map because it's no longer needed.
    &&    releaseBlurredWeightMapBuf()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&  DbgTmr.print("HdrProfiling2:: releaseBlurredWeightMapBuf Time")
                                            #endif
      //  ()  Get HDR Cropped Result image.
    &&  do_HdrCroppedResultGet()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&  DbgTmr.print("HdrProfiling2:: do_HdrResultGet Time")
                                            #endif
    ;

    ret = ret && WAIT(&mMemoryReady_DstImg_cam3[0]);
    ret = ret && WAIT(&mMemoryReady_DstImg_cam3[1]);

    CHECK_OBJECT(mHdrOutBuf_cam3[0]);
    CHECK_OBJECT(mHdrOutBuf_cam3[1]);

    if(mHdrRoundTotal==1 || mHdrRound==2) {
          //  ()  Release HDR working buffer because it's no longer needed.
        ret = ret && releaseHdrWorkingBuf()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&  DbgTmr.print("HdrProfiling2:: releaseHdrWorkingBuf Time")
                                            #endif
        ;
    }

lbExit:
    FUNCTION_LOG_END;
    return ret;
}

MBOOL HdrShot::writeHDROutBuf_cam3()
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

    IImageBuffer* hdrResult = NULL;

    // create hdr result image buffer (I420 format)
    ImageBufferUtils::getInstance().allocBuffer(
            &hdrResult,
            mrHdrCroppedResult.output_image_width,
            mrHdrCroppedResult.output_image_height, eImgFmt_I420);
    if (NULL == hdrResult)
    {
        MY_LOGE("image buffer is NULL");
        return false;
    }

    // get buffer size and copy hdr result to image buffer
    size_t hdrResultSize = 0;
    MUINT8 *srcPtr = mrHdrCroppedResult.output_image_addr;
    for (size_t i = 0; i < hdrResult->getPlaneCount(); i++)
    {
        size_t planeBufSize = hdrResult->getBufSizeInBytes(i);
        void *dstPtr = (void *)hdrResult->getBufVA(i);
        MUINT8 offset = 0;
        memcpy(dstPtr, srcPtr, planeBufSize);

        srcPtr += planeBufSize;
        hdrResultSize += planeBufSize;
    }
    MY_LOGD("hdr result YUV size(%zu)", hdrResultSize);

    // dump hdr result
    if (HDR_DEBUG_SAVE_HDR_RESULT || mDebugMode)
    {
        char szFileName[100];
        ::sprintf(szFileName, HDR_DEBUG_OUTPUT_FOLDER "%04d_8_hdrResult_%dx%d.i420",
                mu4RunningNumber,
                mrHdrCroppedResult.output_image_width,
                mrHdrCroppedResult.output_image_height);
        hdrResult->saveToFile(szFileName);
    }

    // write hdr result to jpeg and thumbnail buffer
    {
        // TODO: set the corresponding transform
        MUINT32 transform = 0;

#if 0
        MRect cropRegion;

        cropRegion.p = MPoint(0, 0);
        cropRegion.s = hdrResult->getImgSize();
#endif

        IImageTransform* pTrans = IImageTransform::createInstance();
        if (pTrans == NULL)
        {
            MY_LOGE("cannot create image transform");
            ret = MFALSE;
            goto lbExit;
        }

        MY_LOGD("hdrResult(%dx%d) jpeg(%dx%d) thumbnail(%dx%d) " \
                "crop(%d,%d,%dx%d)",
                hdrResult->getImgSize().w, hdrResult->getImgSize().h,
                mHdrOutBuf_cam3[0]->getImgSize().w, mHdrOutBuf_cam3[0]->getImgSize().h,
                mHdrOutBuf_cam3[1]->getImgSize().w, mHdrOutBuf_cam3[1]->getImgSize().h,
                mShotParam.mScalerCropRegion.p.x, mShotParam.mScalerCropRegion.p.y,
                mShotParam.mScalerCropRegion.s.w, mShotParam.mScalerCropRegion.s.h);

        // handle digital zoom here
        MBOOL status = pTrans->execute(
                hdrResult, mHdrOutBuf_cam3[0], mHdrOutBuf_cam3[1],
                mShotParam.mScalerCropRegion, transform, 0xFFFFFFFF);
        if (status != MTRUE)
        {
            MY_LOGE("bitblit fail");
            ret = MFALSE;
        }

        pTrans->destroyInstance();
        pTrans = NULL;
    }

lbExit:
    // release hdr result image buffer
    ImageBufferUtils::getInstance().deallocBuffer(hdrResult);
    MY_LOGD("dealloc input working buffer(%p)", hdrResult);
    hdrResult = NULL;

    releaseBlendingBuf();    //[ION]

    FUNCTION_LOG_END;
    return  ret;
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::
init_cam3()
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

    MINT32  err = 0;  // Error code. 0: no error. others: error.
    if(mTestMode) {
        mu4OutputFrameNum = 3;
    }

    //1. init IMem
    mpIMemDrv = IMemDrv::createInstance();
    if(!mpIMemDrv) {
        MY_ERR("HdrShot::init can't alloc mpIMemDrv");
        goto lbExit;
    }
    mpIMemDrv->init();    //check this, see fd
#if HDR_SPEEDUP_MALLOC == 0
    ret = requestSourceImgBuf();
    if  ( ! ret )
    {
        goto lbExit;
    }
#endif

    //2. get information for HDR Hal
    MY_DBG("[init] - HDR Pipe Init");
    // Config HDR Pipe init info structure.
    HDR_PIPE_INIT_INFO rHdrPipeInitInfo;
    rHdrPipeInitInfo.u4ImgW       = mu4W_yuv;
    rHdrPipeInitInfo.u4ImgH       = mu4H_yuv;
    rHdrPipeInitInfo.u4OutputFrameNum = OutputFrameNumGet();
    rHdrPipeInitInfo.u4FinalGainDiff0 = mu4FinalGainDiff[0];
    rHdrPipeInitInfo.u4FinalGainDiff1 = mu4FinalGainDiff[1];
    rHdrPipeInitInfo.u4TargetTone   = mu4TargetTone;
    for (MUINT32 i = 0; i < OutputFrameNumGet(); i++)
    {
        rHdrPipeInitInfo.pSourceImgBufAddr[i] = (MUINT8 *)(mpSourceImgBuf[i].virtAddr);
    }

    //2'.get sensor info
    MY_DBG("mSensorType:%d",mSensorType);
    rHdrPipeInitInfo.u4SensorType   = mSensorType;

    // Create HDR hal object.
    MY_DBG("[init] - Create HDR hal object");
    mpHdrHal = HdrHalBase::createInstance();
    if  ( ! mpHdrHal )
    {
        MY_ERR("HdrHalBase::createInstance fail.");
        goto lbExit;
    }
    // Init HDR hal object.
    ret = mpHdrHal->init((void*)(&rHdrPipeInitInfo));
    if  ( ! ret )
    {
        MY_ERR("mpHdrHal->init fail.");
        goto lbExit;
    }
    ret = ret && mpHdrHal->ConfigMavParam();
    if  ( ! ret )
    {
        MY_ERR("mpHdrHal->ConfigMavParam fail.");
        goto lbExit;
    }


    // For SmallImg Buffer.
    MY_DBG("[init] - QuerySmallImgResolution");
    mpHdrHal->QuerySmallImgResolution(mu4W_small, mu4H_small);
    // For FirstRunSourceImg Buffer.
    mu4W_first = mu4W_small;
    mu4H_first = mu4H_small;

    mu4SmallImgSize = mu4W_small * mu4H_small;
    MY_DBG("[init_cam3] - mu4SmallImgSize=%d", mu4SmallImgSize);

    // For SE Image Buffer.
    MY_DBG("[init] - QuerySEImgResolution");
    mpHdrHal->QuerySEImgResolution(mu4W_se, mu4H_se);


#if HDR_USE_THREAD
    // Create HDR thread.
    SetHdrState(HDR_STATE_INIT);
    ::sem_init(&semHdrThread, 0, 0);
    ::sem_init(&semHdrThreadBack, 0, 0);
    ::sem_init(&semHdrThreadEnd, 0, 0);
    pthread_create(&threadHdrNormal, NULL, mHalCamHdrThread, NULL);
    MY_DBG("[init] threadHdrNormal: %d.", threadHdrNormal);
#endif  // HDR_USE_THREAD

    for(int i =0; i < eMaxOutputFrameNum; i++){
        mMemoryReady_SrcLargeImg_cam3[i]=PTHREAD_MUTEX_INITIALIZER_LOCK;
        mMemoryReady_SrcSmallImg_cam3[i]=PTHREAD_MUTEX_INITIALIZER_LOCK;
    }
    mMemoryReady_DstImg_cam3[0]=PTHREAD_MUTEX_INITIALIZER_LOCK;
    mMemoryReady_DstImg_cam3[1]=PTHREAD_MUTEX_INITIALIZER_LOCK;


lbExit:
    if  ( ! ret )
    {
        uninit();
    }

    FUNCTION_LOG_END;
    return  ret;
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::
getCaptureInfo_cam3(android::Vector<NS3A::CaptureParam_T> & vCap3AParam, MINT32 &hdrFrameNum)
{
    #if 1
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

    HDRExpSettingOutputParam_T strHDROutputSetting;
    ret = getCaptureExposureSettings_cam3(strHDROutputSetting);

    // Record value for later use.
    if(strHDROutputSetting.u4OutputFrameNum) {
        mu4OutputFrameNum   = strHDROutputSetting.u4OutputFrameNum;
        mu4FinalGainDiff[0] = strHDROutputSetting.u4FinalGainDiff[0];
        mu4FinalGainDiff[1] = strHDROutputSetting.u4FinalGainDiff[1];
        mu4TargetTone       = strHDROutputSetting.u4TargetTone;
        MY_DBG("mu4OutputFrameNum(%d) mu4FinalGainDiff(%d,%d) TargetTone(%d)"
                , mu4OutputFrameNum
                , mu4FinalGainDiff[0]
                , mu4FinalGainDiff[1]
                , mu4TargetTone
                );
    } else {
        MY_ERR("u4OutputFrameNum=%d, this should be 2 or 3"
                , strHDROutputSetting.u4OutputFrameNum);
        ret = MFALSE;
    }

    MY_LOGD("u4OutputFrameNum(%d), u4ExpTimeInUS(%d,%d,%d), u4SensorGain(%d,%d,%d)"
            , strHDROutputSetting.u4OutputFrameNum
            , strHDROutputSetting.u4ExpTimeInUS[0]
            , strHDROutputSetting.u4ExpTimeInUS[1]
            , strHDROutputSetting.u4ExpTimeInUS[2]
            , strHDROutputSetting.u4SensorGain[0]
            , strHDROutputSetting.u4SensorGain[1]
            , strHDROutputSetting.u4SensorGain[2]
            );
    MY_LOGD("u1FlareOffset(%d,%d,%d), u4FinalGainDiff(%d,%d), u4TargetTone(%d)"
            , strHDROutputSetting.u1FlareOffset[0]
            , strHDROutputSetting.u1FlareOffset[1]
            , strHDROutputSetting.u1FlareOffset[2]
            , strHDROutputSetting.u4FinalGainDiff[0]
            , strHDROutputSetting.u4FinalGainDiff[1]
            , strHDROutputSetting.u4TargetTone
            );

    //filled vCap3AParam with HDR target 3A condition.

    NS3A::CaptureParam_T tmpCap3AParam;
    for(MUINT32 i=0; i<strHDROutputSetting.u4OutputFrameNum; i++) {

        tmpCap3AParam.u4Eposuretime = strHDROutputSetting.u4ExpTimeInUS[i];
        tmpCap3AParam.u4AfeGain = strHDROutputSetting.u4SensorGain[i];
        tmpCap3AParam.u4IspGain = 1024;
        tmpCap3AParam.u4FlareOffset = strHDROutputSetting.u1FlareOffset[i];

        MY_LOGD("[getCaptureInfo_cam3] Modify Exposuretime[%d] : %d AfeGain[%d]:%d IspGain[%d]:%d FlareOffset[%d]:%d\n"
                , i, tmpCap3AParam.u4Eposuretime
                , i, tmpCap3AParam.u4AfeGain
                , i, tmpCap3AParam.u4IspGain
                , i, tmpCap3AParam.u4FlareOffset
                );

        vCap3AParam.push_back(tmpCap3AParam);
    }

    hdrFrameNum = strHDROutputSetting.u4OutputFrameNum;
    lbExit:
    FUNCTION_LOG_END;
    return ret;
    #endif
    return MTRUE;
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::
decideCaptureMode_cam3()
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;
    MINT32  ret = MTRUE;

    HDRExpSettingOutputParam_T strHDROutputSetting;
    ret = ret && getCaptureExposureSettings_cam3(strHDROutputSetting);

    if(mTestMode) {
        strHDROutputSetting.u4OutputFrameNum = 3;
        strHDROutputSetting.u4FinalGainDiff[0] = 4096;
        strHDROutputSetting.u4FinalGainDiff[1] = 256;
        strHDROutputSetting.u4TargetTone = 150;
    }

    // Record value for later use.
    if(strHDROutputSetting.u4OutputFrameNum) {
        mu4OutputFrameNum   = strHDROutputSetting.u4OutputFrameNum;
        mu4FinalGainDiff[0] = strHDROutputSetting.u4FinalGainDiff[0];
        mu4FinalGainDiff[1] = strHDROutputSetting.u4FinalGainDiff[1];
        mu4TargetTone       = strHDROutputSetting.u4TargetTone;
        MY_DBG("mu4OutputFrameNum(%d) mu4FinalGainDiff(%d,%d) TargetTone(%d)"
                , mu4OutputFrameNum
                , mu4FinalGainDiff[0]
                , mu4FinalGainDiff[1]
                , mu4TargetTone
                );
    }

lbExit:
    FUNCTION_LOG_END;
    return  ret;
}

/*******************************************************************************
*
*******************************************************************************/

MVOID*
HdrShot::
allocateCaptureMemoryTask_First_cam3(MVOID* arg)
{
    SetThreadProp(SCHED_OTHER, -20);

    FUNCTION_LOG_START;
    MUINTPTR     ret = MTRUE;
    HdrShot *self = (HdrShot*)arg;

    MUINT32    i = 0;

    MY_DBG("!!! TODO skip mpSourceImgBuf allocation and skip memcpy\n");
#if 1
    //extraced from requestSourceImgBuf()
    self->mpSourceImgBuf[i].size = self->mu4SourceSize;
    if (self->allocMem(&self->mpSourceImgBuf[i]))    // mpSourceImgBuf[i].virtAddr is NULL, allocation fail.
    {
        MY_ERR("[requestBufs] mpSourceImgBuf[%d] fails to request %d bytes.", i, self->mu4SourceSize);
        ret = MFALSE;
        goto lbExit;
    }
    #if 0
    //extraced from requestFirstRunSourceImgBuf()
    self->mu4FirstRunSourceSize = self->mu4W_first * self->mu4H_first * 3/2;    // I420 Size.
    //MY_VERB("[requestBufs] mu4SourceSize: %d.", mu4FirstRunSourceSize);
    self->mpFirstRunSourceImgBuf[i].size = self->mu4FirstRunSourceSize;
    if (self->allocMem(&self->mpFirstRunSourceImgBuf[i]))    // mpSourceImgBuf[i].virtAddr is NULL, allocation fail.
    {
        MY_ERR("[requestBufs] mpSourceImgBuf[%d] fails to request %d bytes.", i, self->mu4SourceSize);
        ret = MFALSE;
        goto lbExit;
    }
    #endif

#endif

    //extraced from requestSmallImgBuf()
    self->mu4SmallImgSize = self->mu4W_small * self->mu4H_small;    // Y800 size.
    self->mpSmallImgBuf[i].size = self->mu4SmallImgSize;
    if(self->allocMem(&self->mpSmallImgBuf[i]))
    {
        MY_ERR("[requestBufs] mpSmallImgBuf[%d] fails to request %d bytes.", i, self->mu4SmallImgSize);
        ret = MFALSE;
        goto lbExit;
    }

    if(!ret) {
        MY_ERR("can't alloc memory");
    }
lbExit:
    FUNCTION_LOG_END;
    return (MVOID*)ret;
}

/*******************************************************************************
*
*******************************************************************************/


MVOID*
HdrShot::
allocateCaptureMemoryTask_Others_cam3(MVOID* arg)
{
    SetThreadProp(SCHED_OTHER, -20);

    FUNCTION_LOG_START;
    MUINTPTR     ret = MTRUE;

    HdrShot *self = (HdrShot*)arg;

    //allocate buffers for 2rd & 3nd capture
#if 1
    ret = ret
        &&    self->requestSourceImgBuf()
        //&&    self->requestFirstRunSourceImgBuf()
        &&    self->requestSmallImgBuf()
        ;
#else
    ret = MFALSE;
#endif

    if(!ret) {
        MY_ERR("can't alloc memory");
    }
lbExit:
    FUNCTION_LOG_END;
    return (MVOID*)ret;
}

/*******************************************************************************
*
*******************************************************************************/


MVOID*
HdrShot::
allocateCaptureMemoryTask_All_cam3(MVOID* arg)
{
    SetThreadProp(SCHED_OTHER, -20);

    FUNCTION_LOG_START;
    MUINTPTR     ret = MTRUE;
    HdrShot *self = (HdrShot*)arg;

    if(!SKIP_MEMCPY){
        ret = ret
            && allocateCaptureMemoryTask_First_cam3(self)
            && allocateCaptureMemoryTask_Others_cam3(self)
            ;
    }

    if(!ret) {
        MY_ERR("can't alloc memory");
    }
lbExit:
    FUNCTION_LOG_END;
    return (MVOID*)ret;
}

//------------------------------------------------------------------------------
// utility: memory
//------------------------------------------------------------------------------
MBOOL
HdrShot::
announce(pthread_mutex_t *mutex, const char *note)
{
    FUNCTION_LOG_START;
    MBOOL   ret = MTRUE;
    MY_LOGD("announce %s", note);
    pthread_mutex_unlock(mutex);
    FUNCTION_LOG_END;
    return ret;
}



/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::
wait(pthread_mutex_t *mutex, const char* note)
{
    FUNCTION_LOG_START;
    MBOOL   ret = MTRUE;

    MUINT32 start, end;

    MY_LOGD("wait %s start", note);
    //start = getUs();
    pthread_mutex_lock(mutex);
    pthread_mutex_unlock(mutex);
    //end = getUs();
    //MY_LOGD("wait %s ready, pass %d", note, (end-start)/1000);

    MY_LOGD("wait %s ready", note);

    FUNCTION_LOG_END;
    return ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::
getCaptureExposureSettings_cam3(HDRExpSettingOutputParam_T &strHDROutputSetting)
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

    if(mSensorType == NSCam::SENSOR_TYPE_YUV) {
        // for yuv sensor - take 2 pictures, -1.0 & 1.5 ev
        strHDROutputSetting.u4OutputFrameNum = 2;
        strHDROutputSetting.u4FinalGainDiff[0] = 5793;  //capture -1.0, 1.5 ev, 2^(1.5 - -1.0) * 1024 = 5793
        strHDROutputSetting.u4FinalGainDiff[1] = 5793;  //capture -1.0, 1.5 ev, 2^(1.5 - -1.0) * 1024 = 5793
        strHDROutputSetting.u4TargetTone = 150;
    } else {
        HDRExpSettingInputParam_T strHDRInputSetting;

        strHDRInputSetting.u4AOEMode =             mu4AOEMode;
        strHDRInputSetting.u4MaxSensorAnalogGain = mu4MaxSensorAnalogGain;
        strHDRInputSetting.u4MaxAEExpTimeInUS =    mu4MaxAEExpTimeInUS;
        strHDRInputSetting.u4MinAEExpTimeInUS =    mu4MinAEExpTimeInUS;
        strHDRInputSetting.u4ShutterLineTime =     mu4ShutterLineTime;
        strHDRInputSetting.u4MaxAESensorGain =     mu4MaxAESensorGain;
        strHDRInputSetting.u4MinAESensorGain =     mu4MinAESensorGain;
        strHDRInputSetting.u4ExpTimeInUS0EV =      mu4ExpTimeInUS0EV;
        strHDRInputSetting.u4SensorGain0EV =       mu4SensorGain0EV;
        strHDRInputSetting.u1FlareOffset0EV =      mu1FlareOffset0EV;
        strHDRInputSetting.i4GainBase0EV =         mi4GainBase0EV;
        strHDRInputSetting.i4LE_LowAvg =           mi4LE_LowAvg;
        strHDRInputSetting.i4SEDeltaEVx100 =       mi4SEDeltaEVx100;
        memcpy((void*)strHDRInputSetting.u4Histogram,(void*)mu4Histogram,sizeof(mu4Histogram) );

        #if HDR_AE_12BIT_FLARE
        // getHDRExpSetting() suggests flare offset is in 8Bit
        strHDRInputSetting.u1FlareOffset0EV = strHDRInputSetting.u1FlareOffset0EV >> 4;
        #endif

        getHDRExpSetting(strHDRInputSetting, strHDROutputSetting);
        #if HDR_AE_12BIT_FLARE
        for(MUINT32 frame=0; frame<3; frame++) {
            strHDROutputSetting.u1FlareOffset[frame] = strHDROutputSetting.u1FlareOffset[frame] << 4;
        }
        #endif
    }

    MY_DBG("u4OutputFrameNum(%d), u4ExpTimeInUS(%d,%d,%d), u4SensorGain(%d,%d,%d)"
            , strHDROutputSetting.u4OutputFrameNum
            , strHDROutputSetting.u4ExpTimeInUS[0]
            , strHDROutputSetting.u4ExpTimeInUS[1]
            , strHDROutputSetting.u4ExpTimeInUS[2]
            , strHDROutputSetting.u4SensorGain[0]
            , strHDROutputSetting.u4SensorGain[1]
            , strHDROutputSetting.u4SensorGain[2]
            );
    MY_DBG("u1FlareOffset(%d,%d,%d), u4FinalGainDiff(%d,%d), u4TargetTone(%d)"
            , strHDROutputSetting.u1FlareOffset[0]
            , strHDROutputSetting.u1FlareOffset[1]
            , strHDROutputSetting.u1FlareOffset[2]
            , strHDROutputSetting.u4FinalGainDiff[0]
            , strHDROutputSetting.u4FinalGainDiff[1]
            , strHDROutputSetting.u4TargetTone
            );

lbExit:
    FUNCTION_LOG_END;
    return ret;
}

