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

#ifndef _HDR_H_
#define _HDR_H_

#include <utils/Vector.h>
#include <utils/RefBase.h>
#include <utils/Condition.h>
#include <semaphore.h>

#include <common.h>

#include <mtkcam3/featureio/hdr_hal_base.h>
#include <camera_custom_hdr.h> // HDR customer parameters
#include <mtkcam/hal/IHalSensor.h>

#include <pthread.h>

#include <mtkcam3/feature/hdr/HDRDefs.h>

#define HDR_DUMP_PATH "/sdcard/hdr_output/"

using namespace android;
using namespace NSCam;
using namespace HDRCommon;


// ---------------------------------------------------------------------------

//#define JPG_SAVING_OPTIMIZE     1   // Save JPEG while HDR thread are doing things.
typedef MBOOL (*HDRProcCompleteCallback_t)(MVOID* user,MBOOL ret);

// ---------------------------------------------------------------------------

namespace NS3Av3 {
    struct CaptureParam_T;
};

// ---------------------------------------------------------------------------

namespace NSCam {

class HDR
{
public:
    HDR(char const * const pszShotName,
            uint32_t const u4ShotMode, int32_t const i4OpenId);
    virtual ~HDR();

    MBOOL uninit();

    MBOOL setParam(MUINT32 paramId, MUINTPTR iArg1, MUINTPTR iArg2);
    MBOOL setShotParam(void const* pParam);
    MBOOL setJpegParam(void const* pParam);

    MBOOL getParam(MUINT32 paramId, MUINT32& rArg1, MUINT32& rArg2);

    MVOID setCompleteCallback(
            HDRProcCompleteCallback_t completeCB, MVOID* user);

    inline MINT32 getOutputFrameCount() const { return mu4OutputFrameNum; }

    // getCaptureInfo_cam3() is used to
    // 1. get 3A capture settings
    // 2. record the number of HDR input frames to mu4OutputFrameNum
    MBOOL getCaptureInfo_cam3(Vector<NS3Av3::CaptureParam_T>& vCap3AParam, MINT32& hdrFrameNum);

    MBOOL updateInfo_cam3();

    MBOOL EVBracketCapture_cam3();

    // addInputFrame_cam3() is used to set HDR input working buffers
    // for HDR post-processing
    // [NOTE] the frame index ordering should be
    // {0, 2, 4, ...} for main YUV and {1, 3, 5, ...} for small Y8 frames
    MBOOL addInputFrame_cam3(
            MINT32 frameIndex, const sp<IImageBuffer>& inBuffer);

    // addOutputFrame_cam3() is used to set HDR output buffers
    // for HDR post-processing
    // [NOTE] the type should de defined in HDROutputType
    MBOOL addOutputFrame_cam3(
            HDROutputType type, sp<IImageBuffer>& outBuffer);

    // process_cam3() is used to create a worker thread for HDR post-processing
    MBOOL process_cam3();

    // release_cam3() is used to
    // 1. clear HDR settings
    // 2. all HDR image buffers
    MBOOL release_cam3();

    // waitInputFrame() is used to wait for HDR input buffers
    MVOID waitInputFrame();

    // waitOutputFrame() is used to wait for HDR output buffers
    MVOID waitOutputFrame();

    // notify() is used to notify the user that HDR post-processing is done
    MVOID notify(MBOOL ret) const;

private:
    // number of sensor output frames (i.e. HDR input frames)
    MINT32     mu4OutputFrameNum;

    // YUV buffer size
    MUINT32    mu4W_yuv;
    MUINT32    mu4H_yuv;

    // small image buffer size
    MUINT32    mu4W_small;
    MUINT32    mu4H_small;

    // SW EIS image buffer size
    MUINT32    mu4W_se;
    MUINT32    mu4H_se;

    // down-scaled weighting map
    // NOTE: should be set after obtaining the dimension of OriWeight[0]
    MUINT32    mu4W_dsmap;
    MUINT32    mu4H_dsmap;

    // JPEG size
    MUINT32      mRotPicWidth;
    MUINT32      mRotPicHeight;

    // camera device ID
    MINT32       mOpenID;

    // shot mode
    MUINT32      mHDRShotMode;

    // sensor full size
    MSize        mSensorSize;

    // sensor type
    MUINT32      mSensorType;

    // HDR HAL
    HdrHalBase*  mpHdrHal;

    // HDR input buffer
    mutable Mutex       mSourceImgBufLock[NUM_MAX_INPUT_FRAME];
    sem_t               mSourceImgBufSem[NUM_MAX_INPUT_FRAME];
    sp<IImageBuffer>    mpSourceImgBuf[NUM_MAX_INPUT_FRAME];

    // TODO: reused as mWeightingBuf, mpBlurredWeightMapBuf
    mutable Mutex       mSmallImgBufLock[NUM_MAX_INPUT_FRAME];
    sem_t               mSmallImgBufSem[NUM_MAX_INPUT_FRAME];
    sp<IImageBuffer>    mpSmallImgBuf[NUM_MAX_INPUT_FRAME];

    // SW EIS image
    // TODO: reuse buffer of mpDownSizedWeightMapBuf
    mutable Mutex       mSEImgBufLock;
    sem_t               mSEImgBufSem;
    sp<IImageBuffer>    mpSEImgBuf[NUM_MAX_INPUT_FRAME];

    // HDR working buffer
    mutable Mutex       mHdrWorkingBufLock;
    sem_t               mHdrWorkingBufSem;
    sp<IImageBuffer>    mpHdrWorkingBuf;

    // TODO: reuse buffer of mpSmallImgBuf
    mutable Mutex       mWeightingBufLock;
    sem_t               mWeightingBufSem;
    sp<IImageBuffer>    mWeightingBuf[NUM_MAX_INPUT_FRAME];
    HDR_PIPE_WEIGHT_TBL_INFO** OriWeight;

    // down-sized weighting map
    sem_t               mDownSizedWeightMapBufAllocSem;
    mutable Mutex       mDownSizedWeightMapBufLock;
    sem_t               mDownSizedWeightMapBufSem;
    sp<IImageBuffer>    mpDownSizedWeightMapBuf[NUM_MAX_INPUT_FRAME];

    // blurred weighting map
    mutable Mutex       mBlurredWeightMapBufLock;
    sem_t               mBlurredWeightMapBufSem;
    sp<IImageBuffer>    mpBlurredWeightMapBuf[NUM_MAX_INPUT_FRAME];
    HDR_PIPE_WEIGHT_TBL_INFO** BlurredWeight;

    // blending buffer
    mutable Mutex       mBlendingBufLock;
    sem_t               mBlendingBufSem;
    sp<IImageBuffer>    mBlendingBuf;

    // HDR result buffer
    HDR_PIPE_HDR_RESULT_STRUCT mrHdrCroppedResult;

    // HDR output buffers
    sem_t               mHDROutputFramesSem[HDR_OUTPUT_NUM];
    sp<IImageBuffer>    mHDROutputFrames[HDR_OUTPUT_NUM];

    HDR_PIPE_BMAP_BUFFER       mHdrSetBmapInfo;

    // exposure setting
    HDRExpSettingInputParam_T  mExposureInputParam;
    HDRExpSettingOutputParam_T mExposureOutputParam;

    // HDRProc parameters
    HDRProc_ShotParam  mShotParam;

    // a worker thread that allocates HDR working buffers asynchronously
    pthread_t    mMemoryAllocateThread;

    // a workter thread that process HDR post-processing
    pthread_t    mHDRProcessThread_cam3;

    // complete callback
    mutable Mutex mCompleteCBLock;
    HDRProcCompleteCallback_t mCompleteCB;
    MVOID*                    mpCompleteCBUser;

    // capture policy & priority
    int mCapturePolicy;
    int mCapturePriority;

    // indicates whether a cancelling capture event has been sent
    MBOOL mfgIsForceBreak;

    // used for debug purpose
    // a serial number for file saving
    static MUINT32 mu4RunningNumber;

    // option of debug mode
    MINT32 mDebugMode;

    // getCaptureExposureSettings_cam3() is used to
    // get HDR output exposure settings from HDR custom API - getHDRExpSetting()
    MBOOL getCaptureExposureSettings_cam3(
            HDRExpSettingOutputParam_T& exposureOutputParam);

    MBOOL init_cam3();

    MBOOL allocateProcessMemory_cam3();

    // the start routine of HDR post-processing
    static  MVOID* hdrProcess_cam3(MVOID* arg);

    // the start routine of allocating HDR working buffer
    static  MVOID*  allocateProcessMemoryTask(MVOID* arg);

    MBOOL   ImageRegistratoin_cam3();
    MBOOL   WeightingMapGeneration_cam3();
    MBOOL   Blending_cam3();
    MBOOL   writeHDROutputFrame_cam3();

    MBOOL   createSEImg();

    // do small image normalization
    // return MTRUE if success; otherwise MFALSE is returned
    MBOOL   do_Normalization(unsigned int method);

    // do SE to get GMV
    // return MTRUE if success; otherwise MFALSE is returned
    MBOOL   do_SE();

    // do Feature Extraction
    // return MTRUE if success; otherwise MFALSE is returned
    MBOOL   do_FeatureExtraction();

    // do Alignment
    // return MTRUE if success; otherwise MFALSE is returned
    MBOOL   do_Alignment();

    // set the resulting weighting map to HDR HAL
    // return MTRUE if success; otherwise MFALSE is returned
    MBOOL   do_SetBmapBuffer();

    // get original Weighting map
    // return MTRUE if success; otherwise MFALSE is returned
    MBOOL   do_OriWeightMapGet();

    // do down-scaled weighting map
    // return MTRUE if success; otherwise MFALSE is returned
    MBOOL   do_DownScaleWeightMap();

    // do up-scaled weighting map
    // return MTRUE if success; otherwise MFALSE is returned
    MBOOL   do_UpScaleWeightMap();

    // do fusion
    // return MTRUE if success; otherwise MFALSE is returned
    MBOOL   do_Fusion();

    // get HDR result from HDR HAL
    // return MTRUE if success; otherwise MFALSE is returned
    MBOOL   do_HdrCroppedResultGet();

    MBOOL   releaseSourceImgBufLocked(MUINT32 index);
    MBOOL   releaseSmallImgBufLocked(MUINT32 index);
    MBOOL   releaseSEImgBufLocked();

    MUINT32 getHdrWorkingbufferSize();
    MBOOL   releaseHdrWorkingBufLocked();

    MBOOL   requestOriWeightMapBuf();
    MBOOL   releaseOriWeightMapBufLocked();

    MBOOL   requestBlurredWeightMapBuf();
    MBOOL   releaseBlurredWeightMapBufLocked();

    MBOOL   requestDownSizedWeightMapBuf();
    MBOOL   releaseDownSizedWeightMapBufLocked();

    MBOOL   releaseBlendingBufLocked();

    MBOOL   requestImageBuffer(HDRBufferType type);
    MBOOL   releaseImageBuffer(HDRBufferType type);

    MBOOL CDPResize(
            IImageBuffer* pInputBuf,
            IImageBuffer* pOutputBuf,
            MUINT32 transform = 0);

    MBOOL CDPResize_simple(
            IImageBuffer* pInputBuf,
            IImageBuffer* pOutputBuf,
            MUINT32 transform = 0);

    MRect calCrop(MRect const &rSrc, MRect const &rDst, uint32_t ratio = 100);

    // used for debug purpose
    static int dumpToFile(
            char const* fileName, unsigned char *vaddr, size_t size);
};

}; // namespace android

#endif // _HDR_H_
