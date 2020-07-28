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
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "MtkCam/HDRShot"
#include "MyHdr.h"

#include <cstdio>
#include <string.h>

#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/prctl.h>
#include <limits.h>
#include <cutils/properties.h>
#include <cutils/compiler.h>

#include <mtkcam/utils/fwk/MtkCamera.h>


#include <mtkcam/common.h>
#include <mtkcam/utils/common.h>
#include <mtkcam/utils/ImageBufferHeap.h>
#include <mtkcam/exif/IBaseCamExif.h>
#include <mtkcam/exif/CamExif.h>
#include <mtkcam/hwutils/HwMisc.h>

#include <mtkcam/camshot/_callbacks.h>
#include <mtkcam/camshot/ICamShot.h>
#include <mtkcam/camshot/IBurstShot.h>
#include <mtkcam/camshot/_params.h>
#include <mtkcam/camnode/IspSyncControl.h>
#include <mtkcam/camnode/IFeaturePipe.h>
#include <mtkcam/iopipe/PostProc/INormalStream.h>
#include <mtkcam/iopipe/PostProc/IPortEnum.h>

#include <mtkcam/drv/imem_drv.h>

#include <mtkcam/featureio/IHal3A.h>
#include <mtkcam/featureio/aaa_hal_if.h>

#include <CamUtils.h>
#include <MyHdr.h>

#include <camera_feature.h>
using namespace NSFeature;
using namespace NSCamNode;


//#undef HDR_DEBUG_OUTPUT_FOLDER
//#define HDR_DEBUG_OUTPUT_FOLDER       "/storage/sdcard0/DCIM/Camera"



using namespace android;
using namespace MtkCamUtils;
using namespace NSShot;
using namespace NS3A;
using namespace NSCam;
using namespace NSCam::NSIoPipe::NSSImager;
using namespace NSCam::NSIoPipe::NSPostProc;
using namespace NSCamShot;

#include <camera_custom_raw.h>

//
#define DEALLOCMEM(imem)        do {deallocBuffer(imem); imem=NULL;} while(0)
//
#define ANNOUNCE(mutex)         announce(mutex, #mutex)
#define WAIT(mutex)             wait(mutex, #mutex)

//
#define ROUND_TO_2X(x) ((x) & (~0x1))

#define ISP_GAMMA_SIZE 144

#define DONT_MEMCPY 1

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


MUINT32 HdrShot::mu4RunningNumber = 0;
MUINT32 HdrShot::mHDRShotMode = 0;

struct HdrFileInfo {
    String8 filename;
    MUINT32 size;
    MUINT8  *buffer;
};

/*******************************************************************************
*
*******************************************************************************/
#define NAME "HDRUtils"

static MVOID copyExpSettingParam(
        const ExpSettingParam_T& exposureInfo, HDRExpSettingInputParam_T& HDRInputSetting)
{
    HDR_TRACE_CALL();

    HDRInputSetting.u4AOEMode             = exposureInfo.u4AOEMode;
    HDRInputSetting.u4MaxSensorAnalogGain = exposureInfo.u4MaxSensorAnalogGain;
    HDRInputSetting.u4MaxAEExpTimeInUS    = exposureInfo.u4MaxAEExpTimeInUS;
    HDRInputSetting.u4MinAEExpTimeInUS    = exposureInfo.u4MinAEExpTimeInUS;
    HDRInputSetting.u4ShutterLineTime     = exposureInfo.u4ShutterLineTime;
    HDRInputSetting.u4MaxAESensorGain     = exposureInfo.u4MaxAESensorGain;
    HDRInputSetting.u4MinAESensorGain     = exposureInfo.u4MinAESensorGain;
    HDRInputSetting.u4ExpTimeInUS0EV      = exposureInfo.u4ExpTimeInUS0EV;
    HDRInputSetting.u4SensorGain0EV       = exposureInfo.u4SensorGain0EV;
    HDRInputSetting.u1FlareOffset0EV      = exposureInfo.u1FlareOffset0EV;
    HDRInputSetting.i4GainBase0EV         = exposureInfo.i4GainBase0EV;
    HDRInputSetting.i4LE_LowAvg           = exposureInfo.i4LE_LowAvg;
    HDRInputSetting.i4SEDeltaEVx100       = exposureInfo.i4SEDeltaEVx100;
    HDRInputSetting.bDetectFace           = exposureInfo.bDetectFace;

    memcpy(&HDRInputSetting.u4Histogram, &exposureInfo.u4Histogram,
            sizeof(ExpSettingParam_T::u4Histogram));
    memcpy(&HDRInputSetting.u4FlareHistogram, &exposureInfo.u4FlareHistogram,
            sizeof(ExpSettingParam_T::u4FlareHistogram));

    // NOTE: unused data member in NS3A
    HDRInputSetting.u4ISOValue = 0;
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::EVBracketCapture(void)
{
    HDR_TRACE_CALL();

    MBOOL ret = MTRUE;

    // Increase 4-digit running number (range: 0 ~ 9999).
    if (mu4RunningNumber >= 9999)
        mu4RunningNumber = 0;
    else
        mu4RunningNumber++;

    // ()   set output as yuv & small
    ret = ret && createSourceAndSmallImg();

lbExit:
    return ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::ImageRegistratoin(void)
{
    HDR_TRACE_CALL();

    MBOOL ret = MTRUE;

    // (1)  Normalize
    ret = do_Normalization(1);

    // (2)  SE
    ret = ret && WAIT(&mMemoryReady_se);
    ret = ret && createSEImg();
    ret = ret && do_SE();

    ret = ret && WAIT(&mMemoryReady_working);

    // (3)  MAV
    ret = ret &&  do_FeatureExtraction();

    ret = ret &&  releaseSEImgBuf();
    ret = ret &&  releaseSmallImgBuf();

lbExit:
    return  ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::WeightingMapGeneration(void)
{
    HDR_TRACE_CALL();

    MBOOL ret = MTRUE;

    ret = ret && WAIT(&mMemoryReady_bmap0);

    ret = ret && do_Alignment();
    ret = ret && do_OriWeightMapGet();

    ret = ret && ANNOUNCE(&mTrigger_alloc_bmap1);
    ret = ret && WAIT(&mMemoryReady_bmap1);

    ret = ret && do_DownScaleWeightMap();

    ret = ret && WAIT(&mMemoryReady_bmap2);

    ret = ret && do_UpScaleWeightMap();

    ret = ret && releaseDownSizedWeightMapBuf();
    ret = ret && releaseOriWeightMapBuf();

    return ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::Blending(void)
{
    HDR_TRACE_CALL();

    MBOOL ret = MTRUE;

    ret = ret && WAIT(&mMemoryReady_blending);
    ret = ret && do_Fusion();

    ret = ret && releaseBlurredWeightMapBuf();

    ret = ret && do_HdrCroppedResultGet();
    ret = ret && do_HdrNR();

    ret = ret && WAIT(&mMemoryReady_postview);
    ret = ret && do_CroppedPostviewResize();

    return ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::requestSourceImgBuf(void)
{
    HDR_TRACE_CALL();

    MBOOL   ret = MTRUE;
    MUINT32 u4OutputFrameNum = OutputFrameNumGet();
    for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
    {
        if(i==0) {
            //mpSourceImgBuf[0] has been allocated in allocateCaptureMemoryTask_First()
            continue;
        }
        ret = ret && allocBuffer(&mpSourceImgBuf[i], mu4W_yuv, mu4H_yuv, eImgFmt_I420);
    }

lbExit:
    if (!ret) {
        releaseSourceImgBuf();
    }

    return  ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::releaseSourceRawImgBuf(void)
{
    HDR_TRACE_CALL();

    MBOOL   ret = MTRUE;
    MUINT32 u4OutputFrameNum = OutputFrameNumGet();

    if (!DONT_MEMCPY) {
        for (MUINT32 i = 0; i < u4OutputFrameNum; i++) {
            CAM_LOGD("mpSourceRawImgBuf[%d] = %p", i, mpSourceRawImgBuf[i]);
            DEALLOCMEM(mpSourceRawImgBuf[i]);
        }
    }

    return  ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::releaseSourceImgBuf(void)
{
    HDR_TRACE_CALL();

    MBOOL   ret = MTRUE;
    MUINT32 u4OutputFrameNum = OutputFrameNumGet();

    for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
    {
        CAM_LOGD("mpSourceImgBuf[%d] = %p", i, mpSourceImgBuf[i]);
        DEALLOCMEM(mpSourceImgBuf[i]);
    }

    return  ret;
}


///////////////////////////////////////////////////////////////////////////
/// @brief Request Small Image buffers.
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
HdrShot::requestSmallImgBuf(void)
{
    HDR_TRACE_CALL();

    MBOOL   ret = MTRUE;
    switch (mu4OutputFrameNum)  // Allocate buffers according to u4OutputFrameNum, note that there are no "break;" in case 3/case 2.
    {
        case 3: ret = ret && allocBuffer(&mpSmallImgBuf[2], mu4W_small, mu4H_small, eImgFmt_Y800);
        case 2: ret = ret && allocBuffer(&mpSmallImgBuf[1], mu4W_small, mu4H_small, eImgFmt_Y800);
    }
lbExit:
    if(!ret) {
        releaseSmallImgBuf();
    }

    return  ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::releaseSmallImgBuf(void)
{
    HDR_TRACE_CALL();

    MBOOL   ret = MTRUE;

    for (MUINT32 i = 0; i < mu4OutputFrameNum; i++)
    {
#if HDR_DEBUG_SKIP_REUSE_BUFFER
        DEALLOCMEM(mpSmallImgBuf[i]);
#endif
    }

    return  ret;
}


///////////////////////////////////////////////////////////////////////////
/// @brief Request SE Image buffers.
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
HdrShot::requestSEImgBuf(void)
{
    HDR_TRACE_CALL();

    MBOOL   ret = MTRUE;
    CAM_LOGD("[requestSEImgBuf] - E. u4OutputFrameNum: %d.", mu4OutputFrameNum);
    switch (mu4OutputFrameNum)  // Allocate buffers according to u4OutputFrameNum, note that there are no "break;" in case 3/case 2.
    {
        case 3: ret = ret && allocBuffer(&mpSEImgBuf[2], mu4W_se, mu4H_se, eImgFmt_Y800);
        case 2: ret = ret && allocBuffer(&mpSEImgBuf[1], mu4W_se, mu4H_se, eImgFmt_Y800);
        case 1: ret = ret && allocBuffer(&mpSEImgBuf[0], mu4W_se, mu4H_se, eImgFmt_Y800);
    }
lbExit:
    if(!ret) {
        releaseSEImgBuf();
    }

    return  ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::releaseSEImgBuf(void)
{
    HDR_TRACE_CALL();

    MBOOL   ret = MTRUE;

    for (MUINT32 i = 0; i < mu4OutputFrameNum; i++)
    {
#if HDR_DEBUG_SKIP_REUSE_BUFFER
        DEALLOCMEM(mpSEImgBuf[i]); //@todo remove this if reused as mpDownSizedWeightMapBuf
#endif
    }

    return  ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::requestNormalJpegBuf(void)
{
    HDR_TRACE_CALL();

    MBOOL   ret = MTRUE;
    MUINT32 jpegSize = mShotParam.mi4PictureWidth * mShotParam.mi4PictureHeight;
    ret = ret && allocBuffer(&mNormalJpegBuf
                            , mRotPicWidth
                            , mRotPicHeight
                            , eImgFmt_JPEG
                            );
lbExit:
    if(!ret) {
        releaseNormalJpegBuf();
    }

    return  ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::releaseNormalJpegBuf(void)
{
    HDR_TRACE_CALL();

    MBOOL   ret = MTRUE;

    DEALLOCMEM(mNormalJpegBuf);

    return  ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::requestNormalThumbnailJpegBuf(void)
{
    HDR_TRACE_CALL();

    MBOOL   ret = MTRUE;
    MUINT32 jpegSize = mJpegParam.mi4JpegThumbWidth * mJpegParam.mi4JpegThumbHeight;
    ret = ret && allocBuffer(&mNormalThumbnailJpegBuf
                            , mRotThuWidth
                            , mRotThuHeight
                            , eImgFmt_JPEG
                            );
lbExit:
    if(!ret) {
        releaseNormalThumbnailJpegBuf();
    }

    return  ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::releaseNormalThumbnailJpegBuf(void)
{
    HDR_TRACE_CALL();

    MBOOL   ret = MTRUE;

    DEALLOCMEM(mNormalThumbnailJpegBuf);

    return  ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::requestHdrJpegBuf(void)
{
    HDR_TRACE_CALL();

    MBOOL   ret = MTRUE;
    MUINT32 jpegSize = mShotParam.mi4PictureWidth * mShotParam.mi4PictureHeight;
    ret = ret && allocBuffer(&mHdrJpegBuf
                            , mRotPicWidth
                            , mRotPicHeight
                            , eImgFmt_JPEG
                            );
lbExit:
    if(!ret) {
        releaseHdrJpegBuf();
    }

    return  ret;
}


MBOOL
HdrShot::releaseHdrJpegBuf(void)
{
    HDR_TRACE_CALL();

    MBOOL   ret = MTRUE;

    DEALLOCMEM(mHdrJpegBuf);

    return  ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::requestHdrThumbnailJpegBuf(void)
{
    HDR_TRACE_CALL();

    MBOOL   ret = MTRUE;
    MUINT32 jpegSize = mJpegParam.mi4JpegThumbWidth * mJpegParam.mi4JpegThumbHeight;


    ret = ret && allocBuffer(&mHdrThumbnailJpegBuf
                            , mRotThuWidth
                            , mRotThuHeight
                            , eImgFmt_JPEG
                            );

lbExit:
    if  ( ! ret )   {
        releaseHdrThumbnailJpegBuf();
    }

    return  ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::releaseHdrThumbnailJpegBuf(void)
{
    HDR_TRACE_CALL();

    MBOOL   ret = MTRUE;

    DEALLOCMEM(mHdrThumbnailJpegBuf);

    return  ret;
}


///////////////////////////////////////////////////////////////////////////
/// @brief Request HDR working buffers.
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
HdrShot::requestHdrWorkingBuf(void)
{
    HDR_TRACE_CALL();

    MBOOL ret = MTRUE;

    // get working buffer size for HDR & FEFM
    MUINT32 workingbufferSize = mpHdrHal->HdrWorkingBuffSizeGet();
    CAM_LOGD("workingbufferSize(%d)", workingbufferSize);

    // allocate working buffer
    ret = ret && allocBuffer(&mpHdrWorkingBuf, workingbufferSize, 1, eImgFmt_BLOB);

lbExit:
    if (!ret) {
        releaseHdrWorkingBuf();
    }

    return ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::releaseHdrWorkingBuf(void)
{
    HDR_TRACE_CALL();

    MBOOL   ret = MTRUE;

    DEALLOCMEM(mpHdrWorkingBuf);

    return  ret;
}


MUINT32
HdrShot::getAlignedSize(MUINT32 const u4Size)
{
    HDR_TRACE_CALL();

    return (u4Size + (L1_CACHE_BYTES)) & ~(L1_CACHE_BYTES-1);
}


///////////////////////////////////////////////////////////////////////////
/// @brief Request Original WeightingTable buffers.
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
HdrShot::requestOriWeightMapBuf(void)
{
    HDR_TRACE_CALL();

    MBOOL   ret = MTRUE;

    //     Allocate memory for original and blurred Weighting Map.
    MUINT32 u4Size = sizeof(HDR_PIPE_WEIGHT_TBL_INFO*) * mu4OutputFrameNum;
    MUINT32 u4AlignedSize = getAlignedSize(u4Size);
    MUINT32 u4TableSize = sizeof(HDR_PIPE_WEIGHT_TBL_INFO);
    MUINT32 u4AlignedTableSize = getAlignedSize(u4TableSize);
    CAM_LOGV("[requestOriWeightMapBuf] u4Size: %d. u4AlignedSize: %d. u4TableSize: %d. u4AlignedTableSize: %d.", u4Size, u4AlignedSize, u4TableSize, u4AlignedTableSize);

    OriWeight = (HDR_PIPE_WEIGHT_TBL_INFO**)memalign(L1_CACHE_BYTES, u4AlignedSize);
    for (MUINT32 i = 0; i < mu4OutputFrameNum; i++)
        {
           OriWeight[i] = (HDR_PIPE_WEIGHT_TBL_INFO*)memalign(L1_CACHE_BYTES, u4AlignedTableSize);
           if (CC_UNLIKELY(!OriWeight[i]))
                 CAM_LOGD("Warning OriWeight[%d] is NULL\n", i);
        }

    //[ION]
    mHdrSetBmapInfo.bmap_width = mu4W_yuv / 2;
    mHdrSetBmapInfo.bmap_height = mu4H_yuv / 2;
    for (MUINT32 i = 0; i < mu4OutputFrameNum; i++)
    {
        #if HDR_DEBUG_SKIP_REUSE_BUFFER
        ret = ret && allocBuffer(&mWeightingBuf[i], mu4W_yuv * mu4H_yuv / 4, 1, eImgFmt_BLOB);
        mHdrSetBmapInfo.bmap_image_addr[i] = (MUINT8*)mWeightingBuf[i]->getBufVA(0);
        CAM_LOGD("[requestOriWeightMapBuf] addr[%d] = %p", i, (void*)mWeightingBuf[i]->getBufVA(0));
        #endif
    }
    mHdrSetBmapInfo.bmap_image_size = mWeightingBuf[0]->getBufSizeInBytes(0) * mu4OutputFrameNum;

lbExit:
    if  ( ! ret )   {
        releaseOriWeightMapBuf();
    }

    return  ret;
}


///////////////////////////////////////////////////////////////////////////
/// @brief Release Original WeightingTable buffers.
///
/// Note: Some info of OriWeightMap are needed when requestBlurredWeightMapBuf(),
///       so must release it after requestBlurredWeightMapBuf().
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
HdrShot::releaseOriWeightMapBuf(void)
{
    HDR_TRACE_CALL();

    MBOOL   ret = MTRUE;

    if (OriWeight)
    {
        for (MUINT32 i = 0; i < mu4OutputFrameNum; i++)
        {
            free(OriWeight[i]);
        #if HDR_DEBUG_SKIP_REUSE_BUFFER
            DEALLOCMEM(mWeightingBuf[i]);   //@todo remove this if reuse small
        #endif
        }

        delete [] OriWeight;
        OriWeight = NULL;
    }

    return  ret;

}


///////////////////////////////////////////////////////////////////////////
/// @brief Request DownSizedWeightMap buffers.
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
HdrShot::requestDownSizedWeightMapBuf(void)
{
    HDR_TRACE_CALL();

    MBOOL   ret = MTRUE;

    // fit scaler ability
    mu4W_dsmap = (OriWeight[0]->weight_table_width+31)  / 32;
    mu4H_dsmap = (OriWeight[0]->weight_table_height+31) / 32;

    // MT6589 only accept odd output
    mu4W_dsmap = (mu4W_dsmap+1)&~1;
    mu4H_dsmap = (mu4H_dsmap+1)&~1;

    // Allocate memory for Down-sized Weighting Map.
    for(MUINT32 i=0; i<mu4OutputFrameNum; i++) {
    #if HDR_DEBUG_SKIP_REUSE_BUFFER
        ret = ret && allocBuffer(&mpDownSizedWeightMapBuf[i], mu4W_dsmap, mu4H_dsmap, eImgFmt_Y8);
    #endif
    }

lbExit:
    if  ( ! ret )
    {
        releaseDownSizedWeightMapBuf();
    }
    return  ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::releaseDownSizedWeightMapBuf(void)
{
    HDR_TRACE_CALL();

    MBOOL   ret = MTRUE;

    for (MUINT32 i = 0; i < mu4OutputFrameNum; i++)
    {
        // For DownSized Weight Map Buffer.
    #if HDR_DEBUG_SKIP_REUSE_BUFFER
        DEALLOCMEM(mpDownSizedWeightMapBuf[i]);    //@todo remove this if reuse mpSEImgBuf[i]
    #endif
    }

    return  ret;
}


///////////////////////////////////////////////////////////////////////////
/// @brief Request Blurred WeightingTable buffers. Must execute after OriWeightTbl is gottn.
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
HdrShot::requestBlurredWeightMapBuf(void)
{
    HDR_TRACE_CALL();

    MBOOL   ret = MTRUE;

    //     Allocate memory for original and blurred Weighting Map.
    MUINT32 u4Size = sizeof(HDR_PIPE_WEIGHT_TBL_INFO*) * mu4OutputFrameNum;
    MUINT32 u4AlignedSize = getAlignedSize(u4Size);
    MUINT32 u4TableSize = sizeof(HDR_PIPE_WEIGHT_TBL_INFO);
    MUINT32 u4AlignedTableSize = getAlignedSize(u4TableSize);
    CAM_LOGV("[requestBlurredWeightMapBuf] u4Size: %d. u4AlignedSize: %d. u4TableSize: %d. u4AlignedTableSize: %d.", u4Size, u4AlignedSize, u4TableSize, u4AlignedTableSize);

    BlurredWeight = (HDR_PIPE_WEIGHT_TBL_INFO**)memalign(L1_CACHE_BYTES, u4AlignedSize);
    if (CC_UNLIKELY(!BlurredWeight))
              CAM_LOGD("Warning BlurredWeight is NULL\n");

    for (MUINT32 i = 0; i < mu4OutputFrameNum; i++)
    {
        BlurredWeight[i] = (HDR_PIPE_WEIGHT_TBL_INFO*)memalign(L1_CACHE_BYTES, u4AlignedTableSize);
       if (CC_UNLIKELY(!BlurredWeight[i]))
              CAM_LOGD("Warning BlurredWeight[%d] is NULL\n", i);

        // Init BlurredWeight[i], and allocate memory for Blurred Weighting Map.
        BlurredWeight[i]->weight_table_width  = OriWeight[i]->weight_table_width;
        BlurredWeight[i]->weight_table_height = OriWeight[i]->weight_table_height;

    #if HDR_DEBUG_SKIP_REUSE_BUFFER
        ret = ret && allocBuffer(&mpBlurredWeightMapBuf[i]
                                , BlurredWeight[i]->weight_table_width
                                , BlurredWeight[i]->weight_table_height
                                , eImgFmt_Y8
                                );
    #endif
        BlurredWeight[i]->weight_table_data = (MUINT8*)mpBlurredWeightMapBuf[i]->getBufVA(0);
    }

lbExit:
    if(!ret) {
        releaseBlurredWeightMapBuf();
    }

    return  ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::releaseBlurredWeightMapBuf(void)
{
    HDR_TRACE_CALL();

    MBOOL   ret = MTRUE;
    MUINT32 u4OutputFrameNum = OutputFrameNumGet();

    // Free allocated memory
    if (BlurredWeight)
    {
        for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
        {
            free(BlurredWeight[i]);
    #if HDR_DEBUG_SKIP_REUSE_BUFFER
            DEALLOCMEM(mpBlurredWeightMapBuf[i]);
    #endif
        }
        delete [] BlurredWeight;
        BlurredWeight = NULL;
    }

    return  ret;
}


///////////////////////////////////////////////////////////////////////////
/// @brief Request PostviewImg buffer.
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
HdrShot::requestPostviewImgBuf(void)
{
    HDR_TRACE_CALL();

    MBOOL   ret = MTRUE;

    ret = ret && allocBuffer(&mpPostviewImgBuf, mPostviewWidth, mPostviewHeight, mPostviewFormat);

lbExit:
    if(!ret) {
        releasePostviewImgBuf();
    }
    return  ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::releasePostviewImgBuf()
{
    HDR_TRACE_CALL();

    MBOOL   ret = MTRUE;

    DEALLOCMEM(mpPostviewImgBuf);

    return  ret;
}



/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::requestBlendingBuf()
{
    HDR_TRACE_CALL();

    MBOOL   ret = MTRUE;

    ret = ret && allocBuffer(&mBlendingBuf, mu4W_yuv*mu4H_yuv*3/2, 1, eImgFmt_BLOB);

lbExit:
    if(!ret) {
        releaseBlendingBuf();
    }
    return  ret;
}


MBOOL
HdrShot::releaseBlendingBuf()
{
    HDR_TRACE_CALL();

    MBOOL   ret = MTRUE;
    DEALLOCMEM(mBlendingBuf);
    return  ret;
}


/*******************************************************************************
*
*******************************************************************************/
unsigned int
HdrShot::dumpToFile(
    char const *fname,
    unsigned char *pbuf,
    unsigned int size
)
{
    HDR_TRACE_CALL();

    int nw, cnt = 0;
    unsigned int written = 0;

    CAM_LOGD("opening file [%s]\n", fname);
    int fd = open(fname, O_RDWR | O_CREAT, S_IRWXU);
    if (fd < 0) {
        CAM_LOGE("failed to create file [%s]: %s, error!!", fname, strerror(errno));
        return 0x80000000;
    }

    CAM_LOGD("writing %d bytes to file [%s]\n", size, fname);
    while (written < size) {
        nw = ::write(fd, pbuf + written, size - written);
        if (nw < 0) {
            CAM_LOGE("failed to write to file [%s]: %s, error!!", fname, strerror(errno));
            break;
        }
        written += nw;
        cnt++;
    }
    CAM_LOGD("done writing %d bytes to file [%s] in %d passes\n", size, fname, cnt);
    ::close(fd);

    return 0;
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::fgCamShotNotifyCb(MVOID* user, NSCamShot::CamShotNotifyInfo const msg)
{
    HDR_TRACE_CALL();

    MBOOL   ret = MTRUE;

    HdrShot *self = reinterpret_cast <HdrShot *>(user);
    if (NULL != self)
    {
        if (NSCamShot::ECamShot_NOTIFY_MSG_EOF == msg.msgType)
        {
            if (!self->mpShotCallback->onCB_Shutter(true,0)) {
                CAM_LOGE("onCB_Shutter() error!!");
            }
            self->mShutterCBDone = MTRUE;
        }
    }

    return ret;
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::fgCamShotDataCb(MVOID* user, NSCamShot::CamShotDataInfo  const msg)
{
    HDR_TRACE_CALL();

    MBOOL   ret = MTRUE;

#if 1   //HDR_SPEEDUP_BURSTSHOT
    HdrShot *self = reinterpret_cast<HdrShot *>(user);
    if (NULL != self)
    {
        switch(msg.msgType) {
            case NSCamShot::ECamShot_DATA_MSG_POSTVIEW:
                CAM_LOGD("[fgCamShotDataCb] ECamShot_DATA_MSG_POSTVIEW start");
                self->mpCamExif[self->mCaptueIndex] = new CamExif;
                self->update3AExif(self->mpCamExif[self->mCaptueIndex]);
                self->mCaptueIndex++;
                CAM_LOGD("[fgCamShotDataCb] ECamShot_DATA_MSG_POSTVIEW end");
                break;
            case NSCamShot::ECamShot_DATA_MSG_YUV:
                CAM_LOGD("[fgCamShotDataCb] ECamShot_DATA_MSG_YUV start");
                if(self->mCaptueIndex == 1) {   //dbginfo
                    IDbgInfoContainer* pDbgInfo = reinterpret_cast<IDbgInfoContainer*>(msg.ext2);
                    pDbgInfo->copyTo(self->mDebugInfo);
                }
                CAM_LOGD("[fgCamShotDataCb] ECamShot_DATA_MSG_YUV end");
                break;
        }
    }
#endif
    return ret;
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::handleJpegData(MUINT8* const puJpegBuf, MUINT32 const u4JpegSize, MUINT8* const puThumbBuf, MUINT32 const u4ThumbSize, MUINT32 const u4Index, MBOOL bFinal)
{
    HDR_TRACE_CALL();

    MBOOL ret = MTRUE;
    CAM_LOGD("[%s:%d] + (puJpgBuf, jpgSize, puThumbBuf, thumbSize, u4Index, bFinal) = (%p, %d, %p, %d, %d, %d)"
            , __FUNCTION__, __LINE__
            , puJpegBuf
            , u4JpegSize
            , puThumbBuf
            , u4ThumbSize
            , u4Index
            , bFinal);

    if (mTestMode) {
        CAM_LOGW("mTestMode");
        return MTRUE;
    }

    MUINT8 *puExifHeaderBuf = new MUINT8[256 * 1024];
    MUINT32 u4ExifHeaderSize = 0;

    makeExifHeader(eAppMode_PhotoMode, puThumbBuf, u4ThumbSize, puExifHeaderBuf, u4ExifHeaderSize, mDebugInfo);
    CAM_LOGD("(thumbbuf, size, exifHeaderBuf, size) = (%p, %d, %p, %d)",
                      puThumbBuf, u4ThumbSize, puExifHeaderBuf, u4ExifHeaderSize);

    // Jpeg callback
    ret = mpShotCallback->onCB_CompressedImage(0
                                         , u4JpegSize
                                         , reinterpret_cast<uint8_t const*>(puJpegBuf)
                                         , u4ExifHeaderSize //header size
                                         , puExifHeaderBuf  //header buf
                                         , u4Index          //callback index
                                         , true               //final image
                                         , bFinal ? MTK_CAMERA_MSG_EXT_DATA_COMPRESSED_IMAGE : MTK_CAMERA_MSG_EXT_DATA_HDR
                                         );
    // NOTE: in ZSD mode, camrea APP will start preview when receiving
    // either P2Done or CompressedImage callback
    if (ret != MTRUE) {
        if (mHDRShotMode == eShotMode_ZsdHdrShot)
            CAM_LOGW("onCB_CompressedImage() error!!");
        else
            CAM_LOGE("onCB_CompressedImage() error!!");
    }

lbExit:
    if (ret == MTRUE) {
        mJpegCBDone = MTRUE;
    }
    delete [] puExifHeaderBuf;

    return ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::createSourceAndSmallImg(void)
{
    HDR_TRACE_CALL();

    MBOOL ret = MTRUE;
    MUINT32 capBufLen = 0;

    //  allocate memory
    pthread_create(&mMemoryAllocateThread, NULL, HdrShot::allocateMemoryTask, this);

    if(mHDRShotMode == eShotMode_ZsdHdrShot){

        IHal3A *p3AHal = IHal3A::createInstance(IHal3A::E_Camera_1, getOpenId(), LOG_TAG);
        CHECK_OBJECT(p3AHal);

        NS3A::CaptureParam_T cap3AParam;
        p3AHal->getCaptureParams(cap3AParam);
        CAM_LOGD("Current ISO value = %d",cap3AParam.u4RealISO) ;

        ret = ret && WAIT(&mMemoryReady_Capbuf);
        CHECK_OBJECT(mpCapBufMgr);
        mpCapBufMgr->getBufLen(capBufLen,CapBufMgr::CAP_BUF_LEN_MAX);
        IImageBuffer * pImgBuf;

        IspSyncControl* pIspSyncCtrl = NULL;
        pIspSyncCtrl = IspSyncControl::createInstance(getOpenId());
        CHECK_OBJECT(pIspSyncCtrl);

        for(MUINT32 i=0; i < capBufLen; i++) {
            ret = ret && mpCapBufMgr->dequeBuf(pImgBuf);
            CAM_LOGD("dequeBuf[%d]:%p from CapBufMgr",i, (void *)pImgBuf);

            if (DONT_MEMCPY){
                CAM_LOGD("Skip MEMCPY");
                mpSourceRawImgBuf[i] = pImgBuf;
            } else {
                CAM_LOGD("Do MEMCPY");
                memcpy((void*)mpSourceRawImgBuf[i]->getBufVA(0), (void*)pImgBuf->getBufVA(0), mpSourceRawImgBuf[i]->getBufSizeInBytes(0));
            }

            //backup private data
            MUINT32 magicNum = 0;
            MBOOL isRrzo = MFALSE;
            MVOID* pPrivateData = NULL;
            MUINT32 privateDataSize = 0;

            //
            pIspSyncCtrl->queryImgBufInfo(pImgBuf
                                        , magicNum
                                        , isRrzo
                                        , pPrivateData
                                        , privateDataSize
                                        );
            if(mPrivateDataSize == 0) {
                mPrivateDataSize = privateDataSize;
                mPrivateData = operator new(mPrivateDataSize);
                memcpy(mPrivateData, pPrivateData, mPrivateDataSize);
                CAM_LOGD("mPrivateDataSize = %d", mPrivateDataSize);
            }
            EIspProfile_T profile;
            profile = (mNrtype==ECamShot_NRTYPE_SWNR ? EIspProfile_VSS_Capture_SWNR : EIspProfile_VSS_Capture);
            CAM_LOGD("ZSD mNrtype = %d profile = %d", mNrtype, profile);

            CAM_LOGD("enqueBuf:%p to CapBufMgr", (void *)pImgBuf);
            mpCapBufMgr->enqueBuf(pImgBuf);
            pImgBuf = NULL;

            if (i == 0) {
                ret = ret && WAIT(&mMemoryReady_pass2_first);
            } else {
                ret = ret && WAIT(&mMemoryReady_pass2_others);
            }

            ret = ret && encodeRawToYuv(mpSourceRawImgBuf[i],mpSourceImgBuf[i],mpSmallImgBuf[i], profile ,mPrivateData);
            CHECK_RET("encodeRawToYuv() error!!");

            mpCamExif[mCaptueIndex] = new CamExif;
            update3AExif(mpCamExif[mCaptueIndex]);
            mCaptueIndex++;
        }

        pIspSyncCtrl->destroyInstance();

        CAM_LOGD("Roll back 3A value iso = %d",cap3AParam.u4RealISO) ;
        p3AHal->send3ACtrl(E3ACtrl_SetIspTuningISO,cap3AParam.u4RealISO, 0);
        p3AHal->destroyInstance(LOG_TAG);

        if (!mpShotCallback->onCB_Shutter(true,0)) {
            CAM_LOGE("onCB_Shutter() error!!");
        }
        mShutterCBDone = MTRUE;

    } else {

        // shot param
        NSCamShot::ShotParam rShotParam(
                            // picture - mpSourceImgBuf
                            eImgFmt_I420,           //yuv format
                            mu4W_yuv,               //picutre width
                            mu4H_yuv,               //picture height
                            0,                      //picture transform
                            // postview - mpSmallImgBuf
                            eImgFmt_Y800,           //postview format
                            mu4W_small,             //postview width
                            mu4H_small,             //postview height
                            0,                      //postview transform
                            mShotParam.mu4ZoomRatio //zoom
                            );
        // sensor param
        NSCamShot::SensorParam rSensorParam(getOpenId(),                             //sensor idx
                                            SENSOR_SCENARIO_ID_NORMAL_CAPTURE,       //Scenaio
                                            get_raw_bit_depth(),                     //bit depth
                                            MFALSE,                                  //bypass delay
                                            MFALSE                                   //bypass scenario
                                            );
        //

#if !HDR_DEBUG_SKIP_3A
        IHal3A *p3AHal = IHal3A::createInstance(IHal3A::E_Camera_1, getOpenId(), LOG_TAG);
        CHECK_OBJECT(p3AHal);
#endif

        // get pCap3AParam for burst shot
        NS3A::CaptureParam_T pCap3AParam[3];
        getCaptureParams(pCap3AParam);

        //  burstshot config
        NSCamShot::IBurstShot *pBurstShot = NSCamShot::IBurstShot::createInstance(eShotMode_HdrShot, "hdrshot");

        ret = ret && pBurstShot->init();
        ret = ret && pBurstShot->setShotParam(rShotParam);
        EIspProfile_T profile;
        profile = ( mNrtype==ECamShot_NRTYPE_SWNR ? EIspProfile_Capture_SWNR : EIspProfile_Capture );
        CAM_LOGD("ZSD mNrtype = %d profile = %d", mNrtype, profile);
        ret = ret && pBurstShot->setIspProfile( profile);
        if(ret) {
            pBurstShot->setCallbacks(fgCamShotNotifyCb, fgCamShotDataCb, this);
            pBurstShot->disableNotifyMsg(0xFFFFFFFF);
            pBurstShot->enableNotifyMsg(NSCamShot::ECamShot_NOTIFY_MSG_SOF
                                        | NSCamShot::ECamShot_NOTIFY_MSG_EOF);
            pBurstShot->enableDataMsg(NSCamShot::ECamShot_DATA_MSG_YUV
                                    | NSCamShot::ECamShot_DATA_MSG_POSTVIEW
                                    );
        }

        //
        CAM_LOGD("[createSourceAndSmallImg] Modify i4YuvEvIdx [%d, %d]"
                , pCap3AParam[0].i4YuvEvIdx
                , pCap3AParam[1].i4YuvEvIdx
                );
        ret = ret && pBurstShot->registerCap3AParam(pCap3AParam, mu4OutputFrameNum);
        CHECK_RET("registerCap3AParam() error!!");

        ret = ret && WAIT(&mMemoryReady_pass2_first);
        ret = ret && WAIT(&mMemoryReady_pass2_others);

        //  burstshot regist buffer
        for(MUINT32 i=0; i<mu4OutputFrameNum; i++) {
            ret = ret && pBurstShot->registerImageBuffer(NSCamShot::ECamShot_BUF_TYPE_YUV, mpSourceImgBuf[i]);
            ret = ret && pBurstShot->registerImageBuffer(NSCamShot::ECamShot_BUF_TYPE_POSTVIEW, mpSmallImgBuf[i]);
        }
        CHECK_RET("registerImageBuffer() error!!");

        //  burstshot start
        ret = ret && pBurstShot->start(rSensorParam, mu4OutputFrameNum);
        CHECK_RET("pBurstShot->start() error!!");

        //  burstshot done
        ret = ret && pBurstShot->uninit();
        pBurstShot->destroyInstance();


#if !HDR_DEBUG_SKIP_3A
        p3AHal->destroyInstance(LOG_TAG);
#endif

    }

lbCaptureDone:
    if(!ret) goto lbExit;

    if(mHDRShotMode == eShotMode_ZsdHdrShot && (CUST_HDR_DEBUG || HDR_DEBUG_SAVE_SOURCE_IMAGE || mDebugMode)) {
        for (MUINT32 i = 0; i < mu4OutputFrameNum; i++)
        {
            char szFileName[100];
            ::sprintf(szFileName, HDR_DEBUG_OUTPUT_FOLDER "%04d_0_mpSourceRawImgBuf[%d]_%dx%d.raw", mu4RunningNumber, i, mRaw_Width, mRaw_Height);
            mpSourceRawImgBuf[i]->saveToFile(szFileName);
        }
    }

    if(CUST_HDR_DEBUG || HDR_DEBUG_SAVE_SOURCE_IMAGE || mDebugMode) {
        for (MUINT32 i = 0; i < mu4OutputFrameNum; i++)
        {
            char szFileName[100];
            ::sprintf(szFileName, HDR_DEBUG_OUTPUT_FOLDER "%04d_1_mpSourceImgBuf[%d]_%dx%d.i420", mu4RunningNumber, i, mu4W_yuv, mu4H_yuv);
            mpSourceImgBuf[i]->saveToFile(szFileName);
        }
    }

    if(HDR_DEBUG_SAVE_SMALL_IMAGE || mDebugMode) {
        for (MUINT32 i = 0; i < mu4OutputFrameNum; i++)
        {
            char szFileName[100];

            ::sprintf(szFileName, HDR_DEBUG_OUTPUT_FOLDER "%04d_2_mpSmallImgBuf[%d]_%dx%d.y", mu4RunningNumber, i, mu4W_small, mu4H_small);
            mpSmallImgBuf[i]->saveToFile(szFileName);
        }
    }

lbExit:
    return ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::GetStride(MUINT32 srcWidth, EImageFormat srcFormat, MUINT32 *pStride)
{
    HDR_TRACE_CALL();

    MBOOL ret = MTRUE;

    for(MUINT32 i=0; i<3; i++) {
        pStride[i] = (NSCam::Utils::Format::queryPlaneWidthInPixels(srcFormat, i, srcWidth)
                    * NSCam::Utils::Format::queryPlaneBitsPerPixel(srcFormat, i) + 7) / 8;
    }
    CAM_LOGD("stride(%d,%d,%d)", pStride[0], pStride[1], pStride[2]);

    return ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::CDPResize(IImageBuffer* pInputBuf, IImageBuffer* pOutputBuf, MUINT32 transform)
{
    HDR_TRACE_CALL();

    MBOOL ret = MTRUE;

    IImageBuffer *tempInfo[2];
    MUINT32 tempWidth[2];
    MUINT32 tempHeight[2];
    //
    MSize srcSize = pInputBuf->getImgSize();
    MUINT32 srcWidth = srcSize.w;
    MUINT32 srcHeight = srcSize.h;
    EImageFormat srcFormat = (EImageFormat)pInputBuf->getImgFormat();
    //
    MSize desSize = pOutputBuf->getImgSize();
    MUINT32 desWidth = desSize.w;
    MUINT32 desHeight = desSize.h;
    EImageFormat desFormat = (EImageFormat)pOutputBuf->getImgFormat();

    //init
    if((desWidth>32*srcWidth)
            || (desHeight>32*srcHeight)) {
        //prepare source
        tempWidth[0] = srcWidth;
        tempHeight[0] = srcHeight;
        MBOOL isFirstRun = MTRUE;
        tempInfo[0] = pInputBuf;

        while(1) {
            //prepare target
            CAM_LOGD("[CDPResize] - prepare target");
            tempWidth[1] = desWidth;
            tempHeight[1] = desHeight;

            while(tempWidth[1] > tempWidth[0]*32)
                tempWidth[1] = (tempWidth[1]+31)/32;
            while(tempHeight[1] > tempHeight[0]*32)
                tempHeight[1] = (tempHeight[1]+31)/32;
            tempWidth[1] = (tempWidth[1]+1)&~1;
            tempHeight[1] = (tempHeight[1]+1)&~1;
            CAM_LOGD("[CDPResize] - desWidth=%d desHeight=%d", desWidth, desHeight);
            CAM_LOGD("[CDPResize] - tempWidth[0]=%d tempHeight[0]=%d", tempWidth[0], tempHeight[0]);
            CAM_LOGD("[CDPResize] - tempWidth[1]=%d tempHeight[1]=%d", tempWidth[1], tempHeight[1]);

            //scale up - last round
            if(tempWidth[1]==desWidth && tempHeight[1]==desHeight) {
                CAM_LOGD("[CDPResize] - scale up - final round");
                MBOOL ret;
                //ret = CDPResize_simple(tempInfo[0], desMem, rotation);
                ret = CDPResize_simple(tempInfo[0], pOutputBuf, transform);
                DEALLOCMEM(tempInfo[0]);
                return ret;
            }

            //scale up
            CAM_LOGD("[CDPResize] - scale up");
            if(!allocBuffer(&tempInfo[1], tempWidth[1], tempHeight[1], srcFormat)) {
                ret = MFALSE;
                goto lbExit;
            }
            CDPResize_simple(tempInfo[0], tempInfo[1]);
            if(!isFirstRun)
                DEALLOCMEM(tempInfo[0]);
            tempWidth[0] = tempWidth[1];
            tempHeight[0] = tempHeight[1];
            tempInfo[0] = tempInfo[1];

            isFirstRun = MFALSE;
        }

    }

    ret = CDPResize_simple(pInputBuf, pOutputBuf, transform);
lbExit:
    return ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::CDPResize_simple(IImageBuffer* pInputBuf, IImageBuffer* pOutputBuf, MUINT32 transform)
{
    HDR_TRACE_CALL();

    MBOOL ret = MTRUE;

    //
    MSize srcSize = pInputBuf->getImgSize();
    MUINT32 srcWidth = srcSize.w;
    MUINT32 srcHeight = srcSize.h;
    EImageFormat srcFormat = (EImageFormat)pInputBuf->getImgFormat();
    //
    MSize desSize = pOutputBuf->getImgSize();
    MUINT32 desWidth = desSize.w;
    MUINT32 desHeight = desSize.h;
    EImageFormat desFormat = (EImageFormat)pOutputBuf->getImgFormat();

    CAM_LOGD("[CDPResize] - srcMem=%p", pInputBuf);
    CAM_LOGD("[CDPResize] - srcWidth=%d, srcHeight=%d", srcWidth, srcHeight);
    CAM_LOGD("[CDPResize] - srcFormat=%d", srcFormat);
    CAM_LOGD("[CDPResize] - desMem=%p", pOutputBuf);
    CAM_LOGD("[CDPResize] - desWidth=%d, desHeight=%d", desWidth, desHeight);
    CAM_LOGD("[CDPResize] - desFormat=%d", desFormat);
    CAM_LOGD("[CDPResize] - transform=%d", transform);

    // create Instance
    ISImager *pISImager = ISImager::createInstance(pInputBuf);
    if (pISImager == NULL)
    {
        CAM_LOGE("Null ISImager Obj, error!!");
        return 0;
    }

    // init setting
    ret = ret && pISImager->setTransform(transform);
    ret = ret && pISImager->setTargetImgBuffer(pOutputBuf);
    CHECK_RET("init pISImager error!!");

    // execute simager
    ret = ret && pISImager->execute();
    CHECK_RET("pISImager->execute() error!!");

    // destory
    pISImager->destroyInstance();

lbExit:
    return ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::createSEImg(void)
{
    HDR_TRACE_CALL();

    MBOOL  ret = MTRUE;
    MUINT32 u4OutputFrameNum = OutputFrameNumGet();

    for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
    {
        CAM_LOGD("[createSEImg] - CDP %d/%d.", i, u4OutputFrameNum);
        ret = ret && CDPResize(mpSmallImgBuf[i], mpSEImgBuf[i]);
    }

    if(HDR_DEBUG_SAVE_SE_IMAGE || mDebugMode) {
        for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
        {
            char szFileName[100];
            ::sprintf(szFileName, HDR_DEBUG_OUTPUT_FOLDER "%04d_4_mpSEImgBuf[%d]_%dx%d.y", mu4RunningNumber, i, mu4W_se, mu4H_se);
            mpSEImgBuf[i]->saveToFile(szFileName);
        }
    }

    return  ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::encodeRawToYuv(IImageBuffer *rawBuffer, IImageBuffer *yuvBuffer, IImageBuffer *smallyuvBuffer, EIspProfile_T profile, void* /*privateData*/)
{
    HDR_TRACE_CALL();

    MBOOL   ret = MTRUE;

    //
    NSCam::NSIoPipe::NSPostProc::Input minput;
    NSCam::NSIoPipe::NSPostProc::Output moutput;
    NSCam::NSIoPipe::NSPostProc::Output moutput2;
    QParams params;
    QParams dequeParams;
    //
    INormalStream* mpINormalStream = NULL;
    IHal3A *p3A = NULL;
    //
    MSize srcSize = rawBuffer->getImgSize();
    MSize dstSize = yuvBuffer->getImgSize();
    MSize dstSize2 = smallyuvBuffer->getImgSize();
    NSCamHW::Rect srcRect(0, 0, srcSize.w, srcSize.h);
    NSCamHW::Rect dstRect(0, 0, dstSize.w, dstSize.h);
    NSCamHW::Rect dstRect2(0, 0, dstSize2.w, dstSize2.h);
    NSCamHW::Rect cropRect = MtkCamUtils::calCrop(srcRect, dstRect, 100);
    NSCamHW::Rect cropRect2 = MtkCamUtils::calCrop(srcRect, dstRect2, 100);
    CAM_LOGD("srcRect xywh(%d,%d,%d,%d)", srcRect.x, srcRect.y, srcRect.w, srcRect.h);
    CAM_LOGD("dstRect xywh(%d,%d,%d,%d)", dstRect.x, dstRect.y, dstRect.w, dstRect.h);
    CAM_LOGD("cropRect xywh(%d,%d,%d,%d)", cropRect.x, cropRect.y, cropRect.w, cropRect.h);
    CAM_LOGD("dstRect2 xywh(%d,%d,%d,%d)", dstRect2.x, dstRect2.y, dstRect2.w, dstRect2.h);
    CAM_LOGD("cropRect2 xywh(%d,%d,%d,%d)", cropRect2.x, cropRect2.y, cropRect2.w, cropRect2.h);

    p3A = IHal3A::createInstance(IHal3A::E_Camera_1, getOpenId(), LOG_TAG);
    if(!p3A) {
        CAM_LOGE("IHal3A::createInstance() error!!");
        ret = MFALSE;
        goto lbExit;
    }

    {
        ParamIspProfile_T _3A_profile(profile
                                    , 0  //magicNum
                                    , MFALSE
                                    , ParamIspProfile_T::EParamValidate_P2Only
                                    );
        p3A->setIspProfile(_3A_profile);
    }
    p3A->destroyInstance(LOG_TAG);

    // init
    if (eShotMode_ZsdHdrShot == mHDRShotMode){
        mpINormalStream = NSCam::NSIoPipe::NSPostProc::INormalStream::createInstance("hdr_P2iopipe", ENormalStreamTag_Vss, getOpenId());
    } else {
        mpINormalStream = NSCam::NSIoPipe::NSPostProc::INormalStream::createInstance("hdr_P2iopipe", ENormalStreamTag_Cap, getOpenId());
    }
    CHECK_OBJECT(mpINormalStream);
    ret = ret && mpINormalStream->init();
    CHECK_RET("mpINormalStream->init() error!!");

    //crop
    {
        MCrpRsInfo crop1;
        crop1.mGroupID    = 1;
        crop1.mCropRect.s = srcSize;
        crop1.mResizeDst  = srcSize;
        //
        MCrpRsInfo crop2;
        crop2.mGroupID    = 2;
        crop2.mCropRect.p_integral.x = cropRect.x; //0
        crop2.mCropRect.p_integral.y = cropRect.y; //0
        crop2.mCropRect.p_fractional.x = 0;
        crop2.mCropRect.p_fractional.y = 0;
        crop2.mCropRect.s.w = cropRect.w;
        crop2.mCropRect.s.h = cropRect.h;
        //
        MCrpRsInfo crop3;
        crop3.mGroupID    = 2;
        crop3.mCropRect.p_integral.x = cropRect2.x; //0
        crop3.mCropRect.p_integral.y = cropRect2.y; //0
        crop3.mCropRect.p_fractional.x = 0;
        crop3.mCropRect.p_fractional.y = 0;
        crop3.mCropRect.s.w = cropRect2.w;
        crop3.mCropRect.s.h = cropRect2.h;
        params.mvCropRsInfo.push_back(crop1);
        params.mvCropRsInfo.push_back(crop2);
        params.mvCropRsInfo.push_back(crop3);
    }

    // query sensor static information
    MBOOL isRWBEnabled;
    NSCam::SensorStaticInfo sensorStaticInfo;
    querySensorInfo(sensorStaticInfo);
    // use MDP to do color correction for RWB sensor
    isRWBEnabled = (sensorStaticInfo.rawFmtType == SENSOR_RAW_RWB) ? MTRUE : MFALSE;
    if (isRWBEnabled)
    {
        CAM_LOGD("RWB is enabled");
    }

    //
    params.mvIn.clear();
    minput.mBuffer = rawBuffer;
    //minput.mPortID=((inPorts.itemAt(inputIdx, Type2Type< IMetadata >())).entryFor(MTK_IOPIPE_INFO_PORT_ID)).itemAt(0,Type2Type< MINT32 >());
    minput.mPortID.inout = 0; //in
    minput.mPortID.index = NSCam::NSIoPipe::NSPostProc::EPipePortIndex_IMGI;
    minput.mPortID.type = NSCam::NSIoPipe::EPortType_Memory;
    params.mvIn.push_back(minput);

    //
    params.mvOut.clear();
    moutput.mBuffer = yuvBuffer;
    moutput.mPortID.inout = 1; //out
    moutput.mPortID.index = NSCam::NSIoPipe::NSPostProc::EPipePortIndex_WDMAO;
    moutput.mPortID.type = NSCam::NSIoPipe::EPortType_Memory;
    moutput.bIsRwbEn = isRWBEnabled;
    params.mvOut.push_back(moutput);

    moutput2.mBuffer = smallyuvBuffer;
    moutput2.mPortID.inout = 1; //out
    moutput2.mPortID.index = NSCam::NSIoPipe::NSPostProc::EPipePortIndex_WROTO;
    moutput2.mPortID.type = NSCam::NSIoPipe::EPortType_Memory;
    moutput2.bIsRwbEn = isRWBEnabled;
    params.mvOut.push_back(moutput2);

    //
    if(mPrivateDataSize!=0 && mPrivateData!=NULL) {
        params.mpPrivaData = mPrivateData;
    } else {
        CAM_LOGE("must init mPrivateData first, mPrivateData=%p, mPrivateDataSize=%d, error!!", mPrivateData, mPrivateDataSize);
    }

    // enqueue buffer
    params.mFrameNo = 0;    //i;
    mpINormalStream->enque(params);

    // dequeue buffer
    dequeParams.mFrameNo = 0;   //i;
    ret = ret && mpINormalStream->deque(dequeParams, 5000000000);   //5s,unit is nsec
    CHECK_RET("mpINormalStream->deque() error!!");
    CAM_LOGD("deque result magicNum(%d)", dequeParams.mFrameNo);
    CAM_LOGD("deque mvIn.size(%zu),mvOut.size(%zu)", dequeParams.mvIn.size(), dequeParams.mvOut.size());

lbExit:
    if(mpINormalStream) {
        mpINormalStream->uninit();
        mpINormalStream->destroyInstance("hdr_P2iopipe");
    }
    return ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::encodeJpeg(IImageBuffer *pSrcImgBufInfo
            , NSCamShot::JpegParam const & rJpgParm
            , MUINT32 const u4Transform
            , IImageBuffer *pJpgImgBufInfo
            )
{
    HDR_TRACE_CALL();

    MBOOL   ret = MTRUE;
    MRect crop;
    MSize const srcSize(pSrcImgBufInfo->getImgSize());
    MSize const dstSize =
        ((u4Transform & eTransform_ROT_90)||(u4Transform & eTransform_ROT_270)) ?
        MSize( pJpgImgBufInfo->getImgSize().h, pJpgImgBufInfo->getImgSize().w ) :
        pJpgImgBufInfo->getImgSize();

    // (0). debug
    CAM_LOGD("[createJpegImg] - u4Transform=%d", u4Transform);

    // (1). Create Instance
    ISImager *pISImager = ISImager::createInstance(pSrcImgBufInfo);
    if (pISImager == NULL)
    {
        CAM_LOGE("ISImager::createInstance() error!!");
        return 0;
    }

    // init setting
    ret = ret && pISImager->setTransform(u4Transform);
    ret = ret && pISImager->setEncodeParam(rJpgParm.fgIsSOI, rJpgParm.u4Quality);
    // crop to keep aspect ratio
#define align2(x)   (((x) + 1) & (~1))
    if( srcSize.w * dstSize.h > srcSize.h * dstSize.w ) {
        crop.s.w = align2(dstSize.w * srcSize.h / dstSize.h);
        crop.s.h = align2(srcSize.h);
        crop.p.x = (srcSize.w - crop.s.w) / 2;
        crop.p.y = 0;
    } else if( srcSize.w * dstSize.h < srcSize.h * dstSize.w ) {
        crop.s.w = align2(srcSize.w);
        crop.s.h = align2(dstSize.h * srcSize.w / dstSize.w);
        crop.p.x = 0;
        crop.p.y = (srcSize.h - crop.s.h) / 2;
    }
    pISImager->setCropROI(crop);
#undef align2
    ret = ret && pISImager->setTargetImgBuffer(pJpgImgBufInfo);
    ret = ret && pISImager->execute();
    pISImager->destroyInstance();

    return ret;
}


/*******************************************************************************
*
*******************************************************************************/
MVOID*
HdrShot::allocateCapBufMemoryTask()
{
    HDR_TRACE_CALL();

    MUINTPTR    ret = MTRUE;

    MUINT32 i = 0;
    MUINT32 u4OutputFrameNum = OutputFrameNumGet();

    if (!DONT_MEMCPY){
        for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
        {
            ret = ret && allocBuffer(&mpSourceRawImgBuf[i]
                                                , mRaw_Width
                                                , mRaw_Height
                                                , eImgFmt_BAYER10
                                                );
        }

        if(!ret) {
            CAM_LOGE("can't alloc memory, error!!");
            releaseSourceRawImgBuf();
        }
    }

lbExit:
    return (MVOID*)ret;
}


/*******************************************************************************
*
*******************************************************************************/
MVOID*
HdrShot::allocateCaptureMemoryTask_First()
{
    HDR_TRACE_CALL();

    MUINTPTR    ret = MTRUE;

    MUINT32 i = 0;

    //extraced from requestSourceImgBuf()
    ret = ret && allocBuffer(&mpSourceImgBuf[i]
                                        , mu4W_yuv
                                        , mu4H_yuv
                                        , eImgFmt_I420
                                        );

    //extraced from requestSmallImgBuf()
    ret = ret && allocBuffer(&mpSmallImgBuf[i]
                                        , mu4W_small
                                        , mu4H_small
                                        , eImgFmt_Y800
                                        );

    if(!ret) {
        CAM_LOGE("can't alloc memory, error!!");
    }

lbExit:
    return (MVOID*)ret;
}


/*******************************************************************************
*
*******************************************************************************/
MVOID*
HdrShot::allocateCaptureMemoryTask_Others()
{
    HDR_TRACE_CALL();

    MUINTPTR    ret = MTRUE;

    //allocate buffers for 2rd & 3nd capture
    ret = ret && requestSourceImgBuf();
    ret = ret && requestSmallImgBuf();

    if(!ret) {
        CAM_LOGE("can't alloc memory");
    }
lbExit:
    return (MVOID*)ret;
}


/*******************************************************************************
*
*******************************************************************************/
MVOID*
HdrShot::allocateProcessMemoryTask()
{
    HDR_TRACE_CALL();

    MUINTPTR    ret = MTRUE;

    //allocate buffers for MAV & HDR Core
    ret = ret && requestHdrWorkingBuf();

    if(!ret) {
        CAM_LOGE("can't alloc memory, error!!");
    }
lbExit:
    return (MVOID*)ret;
}


/*******************************************************************************
*
*******************************************************************************/


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::encodeHdrThumbnailJpeg()
{
    HDR_TRACE_CALL();

    MUINTPTR    ret = MTRUE;


    if (0 != mJpegParam.mi4JpegThumbWidth && 0 != mJpegParam.mi4JpegThumbHeight)
    {
        ret = ret && WAIT(&mMemoryReady_jpeg_thumbnail);

        //EXIF add SOI tag
        NSCamShot::JpegParam rParam(mJpegParam.mu4JpegThumbQuality, MTRUE);

        ret = ret && encodeJpeg(mpPostviewImgBuf
                                , rParam
                                , 0
                                , mHdrThumbnailJpegBuf
                                );
    }

    return ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::encodeHdrJpeg()
{
    HDR_TRACE_CALL();

    MUINTPTR    ret = MTRUE;

    NSCamShot::JpegParam yuvJpegParam(mJpegParam.mu4JpegQuality, MFALSE);


    IImageBuffer *pImageHdr;


    pImageHdr = makeBufferAlias(mpHdrWorkingBuf
                                    , mRotPicWidth
                                    , mRotPicHeight
                                    , eImgFmt_YUY2
                                    );
    CHECK_OBJECT(pImageHdr);

    //  jpeg
    ret = ret && WAIT(&mMemoryReady_jpeg_full);
    ret = ret && encodeJpeg(pImageHdr
                            , yuvJpegParam
                            , 0
                            , mHdrJpegBuf
                            );

lbExit:
    if(pImageHdr != NULL) {
        removeBufferAlias(mpHdrWorkingBuf, pImageHdr);
    }
    return ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::saveHdrJpeg()
{
    HDR_TRACE_CALL();

    MUINTPTR    ret = MTRUE;

    MUINT32 u4Index = 0;
    MBOOL   bFinal = MTRUE;

    if( mfgIsSkipthumb )
    {
        ret = handleJpegData((MUINT8*)mHdrJpegBuf->getBufVA(0)
            , mHdrJpegBuf->getBitstreamSize()
            , NULL
            , 0
            , u4Index, bFinal);
        CHECK_RET("handleJpegData() error!!");
    }
    else
    {
        // Jpeg callback, it contains thumbnail in ext1, ext2.
        ret = handleJpegData((MUINT8*)mHdrJpegBuf->getBufVA(0)
            , mHdrJpegBuf->getBitstreamSize()
            , (MUINT8*)mHdrThumbnailJpegBuf->getBufVA(0)
            , mHdrThumbnailJpegBuf->getBitstreamSize()
            , u4Index, bFinal);
        CHECK_RET("handleJpegData() error!!");
    }

lbExit:
    return ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::init()
{
    HDR_TRACE_CALL();

    MBOOL ret = MTRUE;

    // init debug
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.mediatek.hdr.debug", value, "0");
    mDebugMode = atoi(value);
    CAM_LOGD("mDebugMode=%d", mDebugMode);

    // init default thread priority
    GetThreadProp(&mCapturePolicy, &mCapturePriority);
    CAM_LOGD("mCapturePolicy=%d, mCapturePriority=%d", mCapturePolicy, mCapturePriority);

    // init image width, height, format
    ret = ret && init_imageParams();
    CHECK_RET("init_imageParams() error!!");

    // decide capture mode
    ret = ret && init_decideCaptureMode(); //must before init_compoment()
    CHECK_RET("init_decideCaptureMode() error!!");

    // decide compoments (ex. mpHdrHal)
    ret = ret && init_compoment();
    CHECK_RET("init_compoment() error!!");

lbExit:
    if (!ret) {
        uninit();
    }
    return ret;
}


MBOOL
HdrShot::init_imageParams()
{
    HDR_TRACE_CALL();

    MINT32  ret = MTRUE;

    #if 0
    mShotParam.mi4PictureWidth = 4096;
    mShotParam.mi4PictureHeight = 3072;
    mShotParam.mu4ZoomRatio = 200;
    #endif

    // for jpeg buffer
    if( mShotParam.mu4Transform == eTransform_ROT_90 ||
            mShotParam.mu4Transform == eTransform_ROT_270 )
    {
        mRotPicWidth  = mShotParam.mi4PictureHeight;
        mRotPicHeight = mShotParam.mi4PictureWidth;
        mRotThuWidth  = mJpegParam.mi4JpegThumbHeight;
        mRotThuHeight = mJpegParam.mi4JpegThumbWidth;
        mPostviewWidth = mShotParam.mi4PostviewHeight;
        mPostviewHeight = mShotParam.mi4PostviewWidth;
    }
    else
    {
        mRotPicWidth  = mShotParam.mi4PictureWidth;
        mRotPicHeight = mShotParam.mi4PictureHeight;
        mRotThuWidth  = mJpegParam.mi4JpegThumbWidth;
        mRotThuHeight = mJpegParam.mi4JpegThumbHeight;
        mPostviewWidth = mShotParam.mi4PostviewWidth;
        mPostviewHeight = mShotParam.mi4PostviewHeight;
    }

    if( mJpegParam.mi4JpegThumbHeight==0 && mJpegParam.mi4JpegThumbWidth==0 )
        mfgIsSkipthumb = MTRUE;
    else
        mfgIsSkipthumb = MFALSE;


    // NrType
    mNrtype = queryCapNRType( getCaptureIso(), MFALSE);
    CAM_LOGD("mNrtype(%d)", mNrtype);

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.mediatek.hdr.nr", value, "-1");
    MINT32 nr = atoi(value);
    if(nr != -1)
    {
        mNrtype = nr;
        CAM_LOGD("force mNrtype as %d", mNrtype);
    }

    if( mNrtype == NSCamShot::ECamShot_NRTYPE_NONE && HDR_BURST_JPEG_SIZE)
    {
        mu4W_yuv = mShotParam.mi4PictureWidth;
        mu4H_yuv = mShotParam.mi4PictureHeight;

    }
    else
    {


        NSCam::SensorStaticInfo sensorInfo;
        querySensorInfo(sensorInfo);

        mRaw_Width = sensorInfo.captureWidth;
        mRaw_Height = sensorInfo.captureHeight;
        mu4W_yuv = sensorInfo.captureWidth;
        mu4H_yuv = sensorInfo.captureHeight;


        MFLOAT fJpegRatio = (mShotParam.mi4PictureWidth / (MFLOAT)mShotParam.mi4PictureHeight);
        MFLOAT fSensorRatio = (sensorInfo.captureWidth / (MFLOAT)sensorInfo.captureHeight);

        CAM_LOGD("fJpegRatio=%f, fSensorRatio=%f \n", fJpegRatio, fSensorRatio);

        if( fSensorRatio > fJpegRatio )
        {
            mu4W_yuv = (mu4H_yuv * fJpegRatio);
            mu4W_yuv &= ~(0x0F);

        }
        else
        {
            mu4H_yuv = (mu4W_yuv / fJpegRatio);
            mu4H_yuv &= ~(0x0F);
        }

    }

    CAM_LOGD("[updateInfo] HDR:width=%d,height=%d, mu4W_yuv:%d, mu4H_yuv:%d\n"
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
            CAM_LOGE("SensorHal::createInstance() error!!");
    return 0;
        }
        pSensorHal->init();
        pSensorHal->sendCommand(SENSOR_DEV_MAIN
                                    , SENSOR_CMD_GET_SENSOR_FULL_RANGE
                                    , (int)&u4SensorWidth
                                    , (int)&u4SensorHeight
                                    , 0
                                    );
        CAM_LOGD("[updateInfo] SensorHal:sensor width:%d, height:%d\n", u4SensorWidth, u4SensorHeight);
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
            CAM_LOGD("[updateInfo] HDR:width=%d,height=%d, mu4W_yuv:%d, mu4H_yuv:%d\n"
                    , mShotParam.mi4PictureWidth
                    , mShotParam.mi4PictureHeight
                    , mu4W_yuv
                    , mu4H_yuv
                    );
        }
    }
#endif

    //postview
    mPostviewFormat = static_cast<EImageFormat>(mShotParam.miPostviewDisplayFormat);



lbExit:
    return  ret;
}


MBOOL
HdrShot::init_decideCaptureMode()
{
    HDR_TRACE_CALL();

    MINT32  ret = MTRUE;
    MUINT32 capBufLen = 0;

    HDRExpSettingOutputParam_T strHDROutputSetting;
    ret = ret && getCaptureExposureSettings(strHDROutputSetting);
    if(mTestMode) {
        strHDROutputSetting.u4OutputFrameNum = 3;
        strHDROutputSetting.u4FinalGainDiff[0] = 4096;
        strHDROutputSetting.u4FinalGainDiff[1] = 256;
        strHDROutputSetting.u4TargetTone = 150;
    }

    // Record value for later use.
    if(strHDROutputSetting.u4OutputFrameNum) {
        mu4OutputFrameNum   = strHDROutputSetting.u4OutputFrameNum;
        if(mHDRShotMode == eShotMode_ZsdHdrShot){
            CHECK_OBJECT(mpCapBufMgr);
            mpCapBufMgr->getBufLen(capBufLen,CapBufMgr::CAP_BUF_LEN_MAX);
            if(capBufLen != mu4OutputFrameNum)
              {
                 CAM_LOGW("Warning Update HDR Output frame number(%d--->>%d)", mu4OutputFrameNum, capBufLen);
                 mu4OutputFrameNum = capBufLen;
              }
           }
        mu4FinalGainDiff[0] = strHDROutputSetting.u4FinalGainDiff[0];
        mu4FinalGainDiff[1] = strHDROutputSetting.u4FinalGainDiff[1];
        mu4TargetTone       = strHDROutputSetting.u4TargetTone;
        CAM_LOGD("mu4OutputFrameNum(%d) mu4FinalGainDiff(%d,%d) TargetTone(%d)"
                , mu4OutputFrameNum
                , mu4FinalGainDiff[0]
                , mu4FinalGainDiff[1]
                , mu4TargetTone
                );
    } else {
        CAM_LOGE("u4OutputFrameNum=%d, this should be 2 or 3, error!!"
                , strHDROutputSetting.u4OutputFrameNum);
        ret = MFALSE;
    }

    if(mTestMode) {
        mu4OutputFrameNum = 3;
    }


lbExit:
    return  ret;
}


MBOOL
HdrShot::init_compoment()
{
    HDR_TRACE_CALL();

    MINT32  ret = MTRUE;
    IHal3A *p3AHal;
    MINT32 ispGamma[ISP_GAMMA_SIZE];
    MBOOL isGGMEnabled = false;

    //1. init IMem
    mpIMemDrv = IMemDrv::createInstance();
    if(!mpIMemDrv) {
        CAM_LOGE("HdrShot::init can't alloc mpIMemDrv, error!!");
        goto lbExit;
    }
    mpIMemDrv->init(NAME);  //check this, see fd

    mpIImageBufAllocator =  IImageBufferAllocator::getInstance();
    if (mpIImageBufAllocator == NULL)
    {
        CAM_LOGE("mpIImageBufAllocator is NULL, error!!");
        return 0;
    }

    //2. get information for HDR Hal
    CAM_LOGD("[init] - HDR Pipe Init");
    // Config HDR Pipe init info structure.
    HDR_PIPE_INIT_INFO rHdrPipeInitInfo;
    rHdrPipeInitInfo.u4ImgW         = mu4W_yuv;
    rHdrPipeInitInfo.u4ImgH         = mu4H_yuv;
    rHdrPipeInitInfo.u4OutputFrameNum = OutputFrameNumGet();
    rHdrPipeInitInfo.u4FinalGainDiff0 = mu4FinalGainDiff[0];
    rHdrPipeInitInfo.u4FinalGainDiff1 = mu4FinalGainDiff[1];
    rHdrPipeInitInfo.u4TargetTone   = mu4TargetTone;
    for(MUINT32 i=0; i<OutputFrameNumGet(); i++) {
        CAM_LOGD("[init] - mpSourceImgBuf[%d]", i);
        rHdrPipeInitInfo.pSourceImgBufAddr[i] = NULL;
    }

    //2'.get sensor info
    {
        NSCam::SensorStaticInfo rSensorInfo;
        querySensorInfo(rSensorInfo);

        rHdrPipeInitInfo.u4SensorType   = rSensorInfo.sensorType;
        mSensorType = rSensorInfo.sensorType;
    }

    // Create HDR hal object.
    CAM_LOGD("[init] - Create HDR hal object");
    mpHdrHal = HdrHalBase::createInstance();
    CHECK_OBJECT(mpHdrHal);

#if !HDR_DEBUG_SKIP_3A
    p3AHal = IHal3A::createInstance(IHal3A::E_Camera_1, getOpenId(), LOG_TAG);
    CHECK_OBJECT(p3AHal);
    ret = ret && !p3AHal->send3ACtrl(E3ACtrl_GetIspGamma,(MINTPTR)(&ispGamma[0]),(MINTPTR)(&isGGMEnabled));
    if(false == isGGMEnabled)
    {
        CAM_LOGD("[init] - GGM disabled");
        rHdrPipeInitInfo.pIsp_gamma = NULL;
    } else {
        CAM_LOGD("[init] - GGM enabled");
        rHdrPipeInitInfo.pIsp_gamma = &ispGamma[0];
    }
    p3AHal->destroyInstance(LOG_TAG);
#endif

    // NOTE: due to MAV cannot be used by multiple users
    // use this lock to guarantee the MAV lifecycle
    mMAVLock.lock();

    // Init HDR hal object.
    ret = mpHdrHal->init((void*)(&rHdrPipeInitInfo));
    CHECK_RET("mpHdrHal->init() error!!");

    ret = ret && ANNOUNCE(&mTrigger_alloc_working);   //mpHdrHal is needed for allocate working buffer

    // For SmallImg Buffer.
    CAM_LOGD("[init] - QuerySmallImgResolution");
    mpHdrHal->QuerySmallImgResolution(mu4W_small, mu4H_small);
    // For SE Image Buffer.
    CAM_LOGD("[init] - QuerySEImgResolution");
    mpHdrHal->QuerySEImgResolution(mu4W_se, mu4H_se);

    // debug info
    mDebugInfo = IDbgInfoContainer::createInstance();

lbExit:
    return  ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::uninit()
{
    HDR_TRACE_CALL();

    MBOOL ret = MTRUE;

    if (mpHdrHal)
    {
        mpHdrHal->uninit();
        delete mpHdrHal;
        mpHdrHal = NULL;
    }
    for (MUINT32 i = 0; i < OutputFrameNumGet(); i++) {
        if(mpCamExif[i]) {
            mpCamExif[i]->uninit();
            delete mpCamExif[i];
            mpCamExif[i] = 0;
        }
    }
    if(mDebugInfo != NULL) {
        mDebugInfo->destroyInstance();
    }

    mu4W_yuv = mu4H_yuv = 0;

    uninitMemoryMap();

    if(mpIMemDrv) {
        mpIMemDrv->uninit(NAME);
        mpIMemDrv->destroyInstance();
        mpIMemDrv = NULL;
    }

    return  ret;
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::allocBuffer(IImageBuffer** ppBuf, MUINT32 w, MUINT32 h, MUINT32 fmt)
{
    HDR_TRACE_CALL();

    MBOOL ret = MTRUE;

    IImageBuffer* pBuf = NULL;

    if( fmt != eImgFmt_JPEG )
    {
        /* To avoid non-continuous multi-plane memory, allocate ION memory and map it to ImageBuffer */
        MUINT32 plane = NSCam::Utils::Format::queryPlaneCount(fmt);
        ImageBufferMap bufMap;

        bufMap.memBuf.size = 0;
        for (MUINT32 i=0; i<plane; i++) {
            bufMap.memBuf.size += ((NSCam::Utils::Format::queryPlaneWidthInPixels(fmt,i, w) * NSCam::Utils::Format::queryPlaneBitsPerPixel(fmt,i) + 7) / 8) * NSCam::Utils::Format::queryPlaneHeightInPixels(fmt, i, h);
        }

        CHECK_OBJECT(mpIMemDrv);
        if (mpIMemDrv->allocVirtBuf(NAME, &bufMap.memBuf)) {
            CAM_LOGE("g_pIMemDrv->allocVirtBuf() error!!");
            return MFALSE;
        }
        CHECK_OBJECT(mpIMemDrv);
        if (mpIMemDrv->mapPhyAddr(&bufMap.memBuf)) {
            CAM_LOGE("mpIMemDrv->mapPhyAddr() error!!");
            return MFALSE;
        }
        CAM_LOGD("allocBuffer at PA(%p) VA(%p) Size(0x%x)"
                , (void*)bufMap.memBuf.phyAddr
                , (void*)bufMap.memBuf.virtAddr
                , bufMap.memBuf.size
                );

        MINT32 bufBoundaryInBytes[3] = {0, 0, 0};

        MUINT32 strideInBytes[3] = {0};
        for (MUINT32 i = 0; i < plane; i++) {
            strideInBytes[i] = (NSCam::Utils::Format::queryPlaneWidthInPixels(fmt,i, w) * NSCam::Utils::Format::queryPlaneBitsPerPixel(fmt, i) + 7) / 8;
        }
        IImageBufferAllocator::ImgParam imgParam(fmt
                                                , MSize(w,h)
                                                , strideInBytes
                                                , bufBoundaryInBytes
                                                , plane
                                                );

        PortBufInfo_v1 portBufInfo = PortBufInfo_v1(bufMap.memBuf.memID
                                                    , bufMap.memBuf.virtAddr
                                                    , bufMap.memBuf.useNoncache
                                                    , bufMap.memBuf.bufSecu
                                                    , bufMap.memBuf.bufCohe
                                                    );

        sp<ImageBufferHeap> pHeap = ImageBufferHeap::create(LOG_TAG
                                                            , imgParam
                                                            , portBufInfo
                                                            );
        if(pHeap == 0) {
            CAM_LOGE("ImageBufferHeap::create() error!!");
            return MFALSE;
        }

        //
        pBuf = pHeap->createImageBuffer();
        pBuf->incStrong(pBuf);

        bufMap.pImgBuf = pBuf;
        mvImgBufMap.push_back(bufMap);
    }
    else
    {
        MINT32 bufBoundaryInBytes = 0;
        IImageBufferAllocator::ImgParam imgParam(
                MSize(w,h),
                w * h * 6 / 5,  //FIXME
                bufBoundaryInBytes
                );

        CHECK_OBJECT(mpIImageBufAllocator);
        pBuf = mpIImageBufAllocator->alloc_ion(LOG_TAG, imgParam);
    }
    if (!pBuf || !pBuf->lockBuf( LOG_TAG, eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_OFTEN ) )
    {
        CAM_LOGE("Null allocated or lock Buffer failed, error!!");
        ret = MFALSE;

    }
    else
    {
        // flush
        pBuf->syncCache(eCACHECTRL_INVALID);  //hw->cpu
        CAM_LOGD("allocBuffer addr(%p), width(%d), height(%d), format(0x%x)", pBuf, w, h, fmt);
        *ppBuf = pBuf;
    }
lbExit:
    return ret;
}


/*******************************************************************************
*
*******************************************************************************/
void
HdrShot::deallocBuffer(IImageBuffer* pBuf)
{
    HDR_TRACE_CALL();

    if(!pBuf) {
        CAM_LOGD("free a null buffer");
        return;
    }

    pBuf->unlockBuf(LOG_TAG);
    switch (pBuf->getImgFormat())
    {
        case eImgFmt_JPEG:
            mpIImageBufAllocator->free(pBuf);
            break;
        //case eImgFmt_I422:
        //case eImgFmt_I420:
        default:
            pBuf->decStrong(pBuf);
            for (std::vector<ImageBufferMap>::iterator it = mvImgBufMap.begin();
                    it != mvImgBufMap.end();
                    it++)
            {
                if (it->pImgBuf == pBuf)
                {
                    if (mpIMemDrv->unmapPhyAddr(&it->memBuf) != 0)
                        CAM_LOGE("m_pIMemDrv->unmapPhyAddr() error");

                    if (mpIMemDrv->freeVirtBuf(NAME, &it->memBuf) != 0)
                        CAM_LOGE("m_pIMemDrv->freeVirtBuf() error!!");

                    mvImgBufMap.erase(it);
                    break;
                }
            }
    }

    pBuf = NULL;
}


/*******************************************************************************
*
*******************************************************************************/
IImageBuffer*
HdrShot::makeBufferAlias(IImageBuffer *pBaseBuf, MINT32 w, MINT32 h, MINT32 imgFormat)
{
    HDR_TRACE_CALL();

    MBOOL ret = MTRUE;

    MSize imgSize(w, h);
    size_t bufStridesInBytes[3] = {0};

    IImageBuffer *pBuf = NULL;
    IImageBufferHeap *pBufHeap = NULL;

    if(!pBaseBuf) {
        CAM_LOGE("pBasebuf is null, error!!");
        ret = MFALSE;
        goto lbExit;
    }
    CAM_LOGD("pBasebuf is %p", pBaseBuf);

    pBufHeap = pBaseBuf->getImageBufferHeap();
    if(!pBufHeap) {
        CAM_LOGE("pBufHeap is null, error!!");
        ret = MFALSE;
        goto lbExit;
    }

    if(pBufHeap->getImgFormat() != eImgFmt_BLOB) {
        CAM_LOGE("heap buffer type must be BLOB=0x%x, this is 0x%x, error!!"
            , eImgFmt_BLOB
            , pBufHeap->getImgFormat()
            );
        ret = MFALSE;
        goto lbExit;
    }

    //@todo use those functions to replace this block
    //MINT32 const planeImgWidthStrideInPixels = Format::queryPlaneWidthInPixels(getImgFormat(), planeIndex, getImgSize().w);
    //MINT32 const planeImgHeightStrideInPixels= Format::queryPlaneHeightInPixels(getImgFormat(), planeIndex, getImgSize().h);
    switch(imgFormat) {
        case eImgFmt_Y8:
        case eImgFmt_JPEG:
            bufStridesInBytes[0] = w;
            break;
        case eImgFmt_I420:
            bufStridesInBytes[0] = w;
            bufStridesInBytes[1] = w / 2;
            bufStridesInBytes[2] = w / 2;
            break;
        case eImgFmt_YUY2:
            bufStridesInBytes[0] = w * 2;
            bufStridesInBytes[1] = w;
            bufStridesInBytes[2] = w;
            break;
        default:
            CAM_LOGE("unsupported imgFormat %d, error!!", imgFormat);
            ret = MFALSE;
            goto lbExit;
    }

    // create new buffer
    pBuf = pBufHeap->createImageBuffer_FromBlobHeap((size_t)0
                                                    , imgFormat
                                                    , imgSize
                                                    , bufStridesInBytes
                                                    );

    if(!pBuf) {
        CAM_LOGE("can't makeBufferAlias w(%d) h(%d) format(0x%x), error!!", w, h, imgFormat);
        ret = MFALSE;
        goto lbExit;
    }
    pBuf->incStrong(pBuf);

    // unlock old buffer
    if(!pBaseBuf->unlockBuf(LOG_TAG)) {
        CAM_LOGE("can't unlock pBaseBuf, error!!");
    };

    // lock new buffer
    if (!pBuf->lockBuf( LOG_TAG, eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_OFTEN ) )
    {
        CAM_LOGE("can't lock pBuf, error!!");
        ret = MFALSE;
        goto lbExit;
    }


lbExit:
    return pBuf;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::removeBufferAlias(IImageBuffer *pBase, IImageBuffer *pAlias)
{
    HDR_TRACE_CALL();

    MBOOL ret = MTRUE;

    // pre-check
    CHECK_OBJECT(pBase);
    CHECK_OBJECT(pAlias);

    // destroy alias
    if(!pAlias->unlockBuf(LOG_TAG)) {
        CAM_LOGE("can't unlock pBaseBuf, error!!");
        ret = MFALSE;
        goto lbExit;
    };
    if(pAlias) {
        pAlias->decStrong(pAlias);
    }

    // re-lock base buffer
    if (!pBase->lockBuf( LOG_TAG, eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_OFTEN ) )
    {
        CAM_LOGE("can't lock pBase, error!!");
        ret = MFALSE;
        goto lbExit;
    }
lbExit:
    return ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::uninitMemoryMap()
{
    HDR_TRACE_CALL();

    if (mvImgBufMap.size() != 0)
    {
        CAM_LOGE("ImageBuffer leakage here!! error!!");
        for (std::vector<ImageBufferMap>::iterator it = mvImgBufMap.begin();
                it != mvImgBufMap.end();
                it++)
        {
            CAM_LOGE("the leakage occur at PA(%p) VA(%p) Size(0x%x), error!!"
                    , (void*)it->memBuf.phyAddr
                    , (void*)it->memBuf.virtAddr
                    , it->memBuf.size
                    );
            if (mpIMemDrv->unmapPhyAddr(&it->memBuf) != 0)
                CAM_LOGE("m_pIMemDrv->unmapPhyAddr() error");

            if (mpIMemDrv->freeVirtBuf(NAME, &it->memBuf) != 0)
                CAM_LOGE("m_pIMemDrv->freeVirtBuf() error");
        }
    }
    return MTRUE;
}


/*******************************************************************************
*
*******************************************************************************/
MVOID*
HdrShot::allocateMemoryTask(MVOID* arg)
{
    ::prctl(PR_SET_NAME,__FUNCTION__, 0, 0, 0);
    HDR_TRACE_CALL();

    MUINTPTR    ret = MTRUE;
    SetThreadProp(SCHED_OTHER, ANDROID_PRIORITY_FOREGROUND);

    HdrShot *self = (HdrShot*)arg;
    CHECK_OBJECT(self);

    //@todo - add error checking


    // pass1
    if(mHDRShotMode == eShotMode_ZsdHdrShot){
        self->allocateCapBufMemoryTask();
        self->ANNOUNCE(&self->mMemoryReady_Capbuf);
    }

    // pass2
    self->allocateCaptureMemoryTask_First();
    self->ANNOUNCE(&self->mMemoryReady_pass2_first);
    self->allocateCaptureMemoryTask_Others();
    self->ANNOUNCE(&self->mMemoryReady_pass2_others);

    self->WAIT(&self->mTrigger_alloc_working);  //@todo move init mav instance before EVCapture

    // se
    self->requestSEImgBuf();
    self->ANNOUNCE(&self->mMemoryReady_se);

    // working
    self->allocateProcessMemoryTask(); //working buffer
    self->ANNOUNCE(&self->mMemoryReady_working);

    // bmap
    self->requestOriWeightMapBuf();
    self->ANNOUNCE(&self->mMemoryReady_bmap0);
    self->WAIT(&self->mTrigger_alloc_bmap1);
    self->requestDownSizedWeightMapBuf();   //do this boefore releaseOriWeightMapBuf
    self->ANNOUNCE(&self->mMemoryReady_bmap1);
    self->requestBlurredWeightMapBuf();
    self->ANNOUNCE(&self->mMemoryReady_bmap2);

    // blending
    self->requestBlendingBuf();
    self->ANNOUNCE(&self->mMemoryReady_blending);
    self->requestPostviewImgBuf();
    self->ANNOUNCE(&self->mMemoryReady_postview);

    // jpeg
    self->requestHdrJpegBuf();
    self->ANNOUNCE(&self->mMemoryReady_jpeg_full);
    self->requestHdrThumbnailJpegBuf();
    self->ANNOUNCE(&self->mMemoryReady_jpeg_thumbnail);

    //
    pthread_detach(pthread_self());

lbExit:
    return (MVOID*)ret;
}



/******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::update3AExif(CamExif *pCamExif)
{
    HDR_TRACE_CALL();

    MBOOL ret = MTRUE;

    //CAM_LOGD("+ (u4CamMode) = (%d)", u4CamMode);

    CamExifParam rExifParam;
    CamDbgParam rDbgParam;

    // ExifParam (for Gps)
    if (! mJpegParam.ms8GpsLatitude.isEmpty() && !mJpegParam.ms8GpsLongitude.isEmpty())
    {
        rExifParam.u4GpsIsOn = 1;
        ::strncpy(reinterpret_cast<char*>(rExifParam.uGPSLatitude), mJpegParam.ms8GpsLatitude.string(), strlen(mJpegParam.ms8GpsLatitude.string()));
        ::strncpy(reinterpret_cast<char*>(rExifParam.uGPSLongitude), mJpegParam.ms8GpsLongitude.string(), strlen(mJpegParam.ms8GpsLongitude.string()));
        ::strncpy(reinterpret_cast<char*>(rExifParam.uGPSTimeStamp), mJpegParam.ms8GpsTimestamp.string(), strlen(mJpegParam.ms8GpsTimestamp.string()));
        ::strncpy(reinterpret_cast<char*>(rExifParam.uGPSProcessingMethod), mJpegParam.ms8GpsMethod.string(), strlen(mJpegParam.ms8GpsMethod.string()));
        rExifParam.u4GPSAltitude = ::atoi(mJpegParam.ms8GpsAltitude.string());
    }
    // the bitstream already rotated. rotation should be 0
    rExifParam.u4Orientation = 0;
    rExifParam.u4ZoomRatio = mShotParam.mu4ZoomRatio;
    //
    rExifParam.u4Facing = IHalSensorList::get()->queryFacingDirection(getOpenId());

    //
    pCamExif->init(rExifParam,  rDbgParam);
    //
    IHal3A *p3AHal = IHal3A::createInstance(IHal3A::E_Camera_1, getOpenId(), LOG_TAG);
    CHECK_OBJECT(p3AHal);
    p3AHal->set3AEXIFInfo(pCamExif);
    //p3AHal->setDebugInfo(pCamExif, MFALSE); //@todo why ImpShot (didn't use this?)
    p3AHal->destroyInstance(LOG_TAG);

    return ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::SetThreadProp(int policy, int priority)
{
    HDR_TRACE_CALL();

#if !HDR_DEBUG_SKIP_MODIFY_POLICY
    //@see http://www.kernel.org/doc/man-pages/online/pages/man2/sched_setscheduler.2.html
    //int const policy    = pthreadAttr_ptr->sched_policy;
    //int const priority  = pthreadAttr_ptr->sched_priority;
    //CAM_LOGD("policy=%d, priority=%d", policy, priority);

    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);

    switch(policy)
    {
        //non-real-time
        case SCHED_OTHER:
            sched_p.sched_priority = 0;
            sched_setscheduler(0, policy, &sched_p);
            setpriority(PRIO_PROCESS, 0, priority); //-20(high)~19(low)
            break;

        //real-time
        case SCHED_FIFO:
        default:
            sched_p.sched_priority = priority;  //1(low)~99(high)
            sched_setscheduler(0, policy, &sched_p);
    }
#endif

    return MTRUE;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::GetThreadProp(int *policy, int *priority)
{
    HDR_TRACE_CALL();

#if !HDR_DEBUG_SKIP_MODIFY_POLICY
    //@see http://www.kernel.org/doc/man-pages/online/pages/man2/sched_setscheduler.2.html
    struct sched_param sched_p;
    *policy = ::sched_getscheduler(0);

    switch(*policy)
    {
        //non-real-time
        case SCHED_OTHER:
            *priority = getpriority(PRIO_PROCESS, 0);   //-20(high)~19(low)
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


//------------------------------------------------------------------------------
// utility: memory
//------------------------------------------------------------------------------
MBOOL
HdrShot::announce(pthread_mutex_t *mutex, const char *note)
{
    HDR_TRACE_CALL();

    MBOOL   ret = MTRUE;
    CAM_LOGD("announce %s", note);
    pthread_mutex_unlock(mutex);
    return ret;
}



/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrShot::wait(pthread_mutex_t *mutex, const char* note)
{
    HDR_TRACE_CALL();

    MBOOL   ret = MTRUE;

    MUINT32 start, end;

    CAM_LOGD("wait %s start", note);
    start = getUs();
    pthread_mutex_lock(mutex);
    pthread_mutex_unlock(mutex);
    end = getUs();
    CAM_LOGD("wait %s ready, pass %d", note, (end-start)/1000);


    return ret;
}


/*******************************************************************************
*
*******************************************************************************/
MUINT32
HdrShot::getUs()
{
    HDR_TRACE_CALL();

    struct timeval tv;
    ::gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
}


//------------------------------------------------------------------------------
// utility: sensor
//------------------------------------------------------------------------------
MBOOL
HdrShot::querySensorInfo(NSCam::SensorStaticInfo &mSensorInfo)
{
    HDR_TRACE_CALL();

    MBOOL ret = MTRUE;

    IHalSensorList* const pHalSensorList = IHalSensorList::get();

    MINT32 const sensorNum = pHalSensorList->queryNumberOfSensors();

    if( getOpenId() >= sensorNum )
    {
        CAM_LOGE("wrong sensor idx(0x%x), sensorNum(%d), error!!", getOpenId(), sensorNum);
        goto lbExit;
    }

    pHalSensorList->querySensorStaticInfo(pHalSensorList->querySensorDevIdx(getOpenId())
                                            , &mSensorInfo);

    CAM_LOGD("sensorDevID(0x%x), sensorType(0x%x), sensorFormatOrder(0x%x), rawSensorBit(0x%x)"
            "captureWidth(%d), captureHeight(%d)"
            , mSensorInfo.sensorDevID
            , mSensorInfo.sensorType
            , mSensorInfo.sensorFormatOrder
            , mSensorInfo.rawSensorBit
            , mSensorInfo.captureWidth
            , mSensorInfo.captureHeight
            );

lbExit:
    return ret;
}


MBOOL
HdrShot::getCaptureInfo(MUINT32 sensorId, std::vector<NS3A::CaptureParam_T> & vCap3AParam, MUINT32 &hdrFrameNum)
{
    HDR_TRACE_CALL();

    ExpSettingParam_T strExposureInfo;
    HDRExpSettingOutputParam_T strHDROutputSetting;
    HDRExpSettingInputParam_T strHDRInputSetting;
    IHal3A *p3AHal = NULL;
    MBOOL ret = MTRUE;

    //get sensor info
    NSCam::SensorStaticInfo rSensorInfo;
    IHalSensorList* const pHalSensorList = IHalSensorList::get();
    pHalSensorList->querySensorStaticInfo(pHalSensorList->querySensorDevIdx(sensorId), &rSensorInfo);

    CAM_LOGD("sensorType(%d)",rSensorInfo.sensorType);

    //read 3A and cal Target 3A condtion by HDR algo

    if(rSensorInfo.sensorType == SENSOR_TYPE_YUV) {
        // for yuv sensor - take 2 pictures, -1.0 & 1.5 ev
        strHDROutputSetting.u4OutputFrameNum = 2;
        strHDROutputSetting.u4FinalGainDiff[0] = 5793;  //capture -1.0, 1.5 ev, 2^(1.5 - -1.0) * 1024 = 5793
        strHDROutputSetting.u4FinalGainDiff[1] = 5793;  //capture -1.0, 1.5 ev, 2^(1.5 - -1.0) * 1024 = 5793
        strHDROutputSetting.u4TargetTone = 150;
    } else {

    #if !HDR_DEBUG_SKIP_3A
        IHal3A *p3AHal = IHal3A::createInstance(IHal3A::E_Camera_1, sensorId, LOG_TAG);
        CHECK_OBJECT(p3AHal);
        ret = ret && !p3AHal->getExposureInfo(strExposureInfo);
    #endif

        copyExpSettingParam(strExposureInfo, strHDRInputSetting);

    #if !HDR_DEBUG_SKIP_3A
        ret = ret && !p3AHal->send3ACtrl(E3ACtrl_GetCapPLineTable , (MINTPTR)& strHDRInputSetting.i4aeTableCurrentIndex,(MINTPTR)&strHDRInputSetting.PLineAETable);
        CAM_LOGD(" send3ACtrl bDetectFace(%d)", strHDRInputSetting.bDetectFace);
        CAM_LOGD(" send3ACtrl E3ACtrl_GetCapPLineTable aeTableCurrentIndex(%d)", strHDRInputSetting.i4aeTableCurrentIndex);
        CAM_LOGD(" send3ACtrl E3ACtrl_GetCapPLineTable u4TotalIndex(%d)", strHDRInputSetting.PLineAETable.u4TotalIndex);
        CAM_LOGD(" send3ACtrl E3ACtrl_GetCapPLineTable eID(%d)", strHDRInputSetting.PLineAETable.eID);
        p3AHal->destroyInstance(LOG_TAG);
    #endif

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

    CAM_LOGD("u4OutputFrameNum(%d), u4ExpTimeInUS(%d,%d,%d), u4SensorGain(%d,%d,%d)"
            , strHDROutputSetting.u4OutputFrameNum
            , strHDROutputSetting.u4ExpTimeInUS[0]
            , strHDROutputSetting.u4ExpTimeInUS[1]
            , strHDROutputSetting.u4ExpTimeInUS[2]
            , strHDROutputSetting.u4SensorGain[0]
            , strHDROutputSetting.u4SensorGain[1]
            , strHDROutputSetting.u4SensorGain[2]
            );
    CAM_LOGD("u1FlareOffset(%d,%d,%d), u4FinalGainDiff(%d,%d), u4TargetTone(%d)"
            , strHDROutputSetting.u1FlareOffset[0]
            , strHDROutputSetting.u1FlareOffset[1]
            , strHDROutputSetting.u1FlareOffset[2]
            , strHDROutputSetting.u4FinalGainDiff[0]
            , strHDROutputSetting.u4FinalGainDiff[1]
            , strHDROutputSetting.u4TargetTone
            );

    //filled vCap3AParam with HDR target 3A condition.

#if !HDR_DEBUG_SKIP_3A
    p3AHal = IHal3A::createInstance(IHal3A::E_Camera_1, sensorId, LOG_TAG);
    NS3A::CaptureParam_T tmpCap3AParam;
    CHECK_OBJECT(p3AHal);
    for(MUINT32 i=0; i<strHDROutputSetting.u4OutputFrameNum; i++) {
        p3AHal->getCaptureParams(tmpCap3AParam);
        vCap3AParam.push_back(tmpCap3AParam);
    }
    p3AHal->destroyInstance(LOG_TAG);
#endif

    MUINT32 i = 0;

    for(MUINT32 i=0; i<strHDROutputSetting.u4OutputFrameNum; i++) {

        vCap3AParam[i].u4Eposuretime = strHDROutputSetting.u4ExpTimeInUS[i];
        vCap3AParam[i].u4AfeGain = strHDROutputSetting.u4SensorGain[i];
        vCap3AParam[i].u4IspGain = 1024;
        vCap3AParam[i].u4FlareOffset = strHDROutputSetting.u1FlareOffset[i];
        CAM_LOGD("[getCaptureInfo] Modify Exposuretime[%d] : %d AfeGain[%d]:%d IspGain[%d]:%d ISO[%d]:%d\n"
                , i, vCap3AParam[i].u4Eposuretime
                , i, vCap3AParam[i].u4AfeGain
                , i, vCap3AParam[i].u4IspGain
                , i, vCap3AParam[i].u4FlareOffset
                );
    }

    if(rSensorInfo.sensorType == SENSOR_TYPE_YUV){
        // for yuv sensor - take 2 pictures, -1.0 & 1.5 ev
        //vCap3AParam[0].i4YuvEvIdx = AE_EV_COMP_n10;
        //vCap3AParam[1].i4YuvEvIdx = AE_EV_COMP_15;
        CAM_LOGD("[getCaptureInfo] Modify i4YuvEvIdx [%d, %d]"
                , vCap3AParam[0].i4YuvEvIdx
                , vCap3AParam[1].i4YuvEvIdx
                );
    }

    hdrFrameNum = strHDROutputSetting.u4OutputFrameNum;

lbExit:
    return ret;
}


MBOOL
HdrShot::getCaptureParams(NS3A::CaptureParam_T *pCap3AParam)
{
    HDR_TRACE_CALL();

    MBOOL ret = MTRUE;

    HDRExpSettingOutputParam_T strHDROutputSetting;
    ret = ret && getCaptureExposureSettings(strHDROutputSetting);

    for(MUINT32 i=0; i<strHDROutputSetting.u4OutputFrameNum; i++) {
        CAM_LOGD("[getCaptureParams] HDR Exposuretime[%d] : %d Gain[%d]:%d Flare[%d]:%d\n"
                , i, strHDROutputSetting.u4ExpTimeInUS[i]
                , i, strHDROutputSetting.u4SensorGain[i]
                , i, strHDROutputSetting.u1FlareOffset[i]
                );
    }

#if !HDR_DEBUG_SKIP_3A
    IHal3A *p3AHal = IHal3A::createInstance(IHal3A::E_Camera_1, getOpenId(), LOG_TAG);
    CHECK_OBJECT(p3AHal);
    for(MUINT32 i=0; i<strHDROutputSetting.u4OutputFrameNum; i++) {
        p3AHal->getCaptureParams(pCap3AParam[i]);
    }
    p3AHal->destroyInstance(LOG_TAG);
#endif

    for(MUINT32 i=0; i<strHDROutputSetting.u4OutputFrameNum; i++) {
        pCap3AParam[i].u4Eposuretime = strHDROutputSetting.u4ExpTimeInUS[i];
        pCap3AParam[i].u4AfeGain = strHDROutputSetting.u4SensorGain[i];
        pCap3AParam[i].u4IspGain = 1024;
        pCap3AParam[i].u4FlareOffset = strHDROutputSetting.u1FlareOffset[i];
        CAM_LOGD("[getCaptureParams] Modify Exposuretime[%d] : %d AfeGain[%d]:%d IspGain[%d]:%d ISO[%d]:%d\n"
                , i, pCap3AParam[i].u4Eposuretime
                , i, pCap3AParam[i].u4AfeGain
                , i, pCap3AParam[i].u4IspGain
                , i, pCap3AParam[i].u4FlareOffset
                );
    }

    if(mSensorType == SENSOR_TYPE_YUV)
    {
        // for yuv sensor - take 2 pictures, -1.0 & 1.5 ev
        pCap3AParam[0].i4YuvEvIdx = AE_EV_COMP_n10;
        pCap3AParam[1].i4YuvEvIdx = AE_EV_COMP_15;
        CAM_LOGD("[getCaptureParams] Modify i4YuvEvIdx [%d, %d]"
                , pCap3AParam[0].i4YuvEvIdx
                , pCap3AParam[1].i4YuvEvIdx
                );
    }

lbExit:
    return ret;
}


MBOOL
HdrShot::getCaptureExposureSettings(HDRExpSettingOutputParam_T &strHDROutputSetting)
{
    HDR_TRACE_CALL();

    MBOOL ret = MTRUE;

    if(mSensorType == SENSOR_TYPE_YUV) {
        // for yuv sensor - take 2 pictures, -1.0 & 1.5 ev
        strHDROutputSetting.u4OutputFrameNum = 2;
        strHDROutputSetting.u4FinalGainDiff[0] = 5793;  //capture -1.0, 1.5 ev, 2^(1.5 - -1.0) * 1024 = 5793
        strHDROutputSetting.u4FinalGainDiff[1] = 5793;  //capture -1.0, 1.5 ev, 2^(1.5 - -1.0) * 1024 = 5793
        strHDROutputSetting.u4TargetTone = 150;
    } else {
        ExpSettingParam_T strExposureInfo;
        HDRExpSettingInputParam_T strHDRInputSetting;

#if !HDR_DEBUG_SKIP_3A
        IHal3A *p3AHal = IHal3A::createInstance(IHal3A::E_Camera_1, getOpenId(), LOG_TAG);
        CHECK_OBJECT(p3AHal);
        ret = ret && !p3AHal->getExposureInfo(strExposureInfo);
#endif

        copyExpSettingParam(strExposureInfo, strHDRInputSetting);
#if !HDR_DEBUG_SKIP_3A
        ret = ret && !p3AHal->send3ACtrl(E3ACtrl_GetCapPLineTable , (MINTPTR)& strHDRInputSetting.i4aeTableCurrentIndex,(MINTPTR)&strHDRInputSetting.PLineAETable);
        CAM_LOGD(" send3ACtrl bDetectFace(%d)", strHDRInputSetting.bDetectFace);
        CAM_LOGD(" send3ACtrl E3ACtrl_GetCapPLineTable aeTableCurrentIndex(%d)", strHDRInputSetting.i4aeTableCurrentIndex);
        CAM_LOGD(" send3ACtrl E3ACtrl_GetCapPLineTable u4TotalIndex(%d)", strHDRInputSetting.PLineAETable.u4TotalIndex);
        CAM_LOGD(" send3ACtrl E3ACtrl_GetCapPLineTable eID(%d)", strHDRInputSetting.PLineAETable.eID);
        p3AHal->destroyInstance(LOG_TAG);
#endif

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

    CAM_LOGD("u4OutputFrameNum(%d), u4ExpTimeInUS(%d,%d,%d), u4SensorGain(%d,%d,%d)"
            , strHDROutputSetting.u4OutputFrameNum
            , strHDROutputSetting.u4ExpTimeInUS[0]
            , strHDROutputSetting.u4ExpTimeInUS[1]
            , strHDROutputSetting.u4ExpTimeInUS[2]
            , strHDROutputSetting.u4SensorGain[0]
            , strHDROutputSetting.u4SensorGain[1]
            , strHDROutputSetting.u4SensorGain[2]
            );
    CAM_LOGD("u1FlareOffset(%d,%d,%d), u4FinalGainDiff(%d,%d), u4TargetTone(%d)"
            , strHDROutputSetting.u1FlareOffset[0]
            , strHDROutputSetting.u1FlareOffset[1]
            , strHDROutputSetting.u1FlareOffset[2]
            , strHDROutputSetting.u4FinalGainDiff[0]
            , strHDROutputSetting.u4FinalGainDiff[1]
            , strHDROutputSetting.u4TargetTone
            );

lbExit:
    return ret;
}


MBOOL
HdrShot::convertImage( IImageBuffer *pSrcImgBuf
            , IImageBuffer *pDesImgBuf
            , MUINT32 const u4Transform
            , MBOOL /*bIsCrop*/
            , NSCam::MRect cropRect
            )
{
    HDR_TRACE_CALL();

    MBOOL   ret = MTRUE;

    ISImager *pISImager = NULL;


    CAM_LOGD("[createJpegImg] - u4Transform=%d", u4Transform);
    CHECK_OBJECT(pSrcImgBuf);
    CHECK_OBJECT(pDesImgBuf);

    // Create Instance
    pISImager = ISImager::createInstance(pSrcImgBuf);
    CHECK_OBJECT(pISImager);

    // init setting
    ret = ret && pISImager->setTransform(u4Transform);
    CHECK_RET("pISmage->setTransform() error!!");

    CAM_LOGD("cropRect xywh(%d,%d,%d,%d)", cropRect.p.x, cropRect.p.y, cropRect.s.w, cropRect.s.h);
    ret = ret && pISImager->setCropROI(cropRect);
    ret = ret && pISImager->setTargetImgBuffer(pDesImgBuf);
    CHECK_RET("pISmage config error!!");
    ret = ret && pISImager->execute();
    CHECK_RET("pISImager->execute() error!!");


lbExit:
    if(pISImager)
        pISImager->destroyInstance();
    return ret;
}


MRect
HdrShot::calCrop(MRect const &rSrc, MRect const &rDst, uint32_t ratio)
{
    HDR_TRACE_CALL();

    MRect rCrop;

    // srcW/srcH < dstW/dstH
    if (rSrc.s.w * rDst.s.h < rDst.s.w * rSrc.s.h) {
        rCrop.s.w = rSrc.s.w;
        rCrop.s.h = rSrc.s.w * rDst.s.h / rDst.s.w;
    }
    //srcW/srcH > dstW/dstH
    else if (rSrc.s.w * rDst.s.h > rDst.s.w * rSrc.s.h) {
        rCrop.s.w = rSrc.s.h * rDst.s.w / rDst.s.h;
        rCrop.s.h = rSrc.s.h;
    }
    else {
        rCrop.s.w = rSrc.s.w;
        rCrop.s.h = rSrc.s.h;
    }
    //
    rCrop.s.w =  ROUND_TO_2X(rCrop.s.w * 100 / ratio);
    rCrop.s.h =  ROUND_TO_2X(rCrop.s.h * 100 / ratio);
    //
    rCrop.p.x = (rSrc.s.w - rCrop.s.w) / 2;
    rCrop.p.y = (rSrc.s.h - rCrop.s.h) / 2;

    return rCrop;
}
