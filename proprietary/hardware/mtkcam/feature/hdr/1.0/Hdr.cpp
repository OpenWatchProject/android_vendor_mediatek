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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#include "MyHdr.h"
#include <utils/threads.h>
#include <sys/prctl.h>  // For prctl()/PR_SET_NAME.

//
#include <mtkcam/Log.h>
#include <mtkcam/common.h>
//
//#include <Shot/IShot.h>
//
//#include "ImpShot.h"
#include "Hdr.h"
//
using namespace android;

#if 1

/*******************************************************************************
*
*******************************************************************************/
HdrShot::
HdrShot(char const*const pszShotName __attribute__((unused))
    , uint32_t const u4ShotMode __attribute__((unused))
    , int32_t const i4OpenId __attribute__((unused)))
  //
  : mu4W_first(1600)
  , mu4H_first(1200)
  , mu4W_se(0)
  , mu4H_se(0)
  , mu4W_dsmap(0)
  , mu4H_dsmap(0)
  , mu4W_yuv(0)
  , mu4H_yuv(0)
  , mu4W_small(0)
  , mu4H_small(0)
  , mSensorType(0)
  , mCaptureIMemThread(0)
  , mProcessIMemThread(0)
  //
  , mpHdrHal(NULL)
  //
  , mpIMemDrv(NULL)
  , mTotalBufferSize(0)
  , mTotalKernelBufferSize(0)
  , mTotalUserBufferSize(0)
  //
  , OriWeight(NULL)
  , BlurredWeight(NULL)
  //
  , mu4OutputFrameNum(0) //318
  , mu4TargetTone(0) //321
  //, mu4RunningNumber(0)
  , mfgIsForceBreak(MFALSE) //326
  , mHdrState(HDR_STATE_INIT) //328
  , mHdrRound(1) //330
  , mHdrRoundTotal(2) //331
  //
  , mShutterCBDone(0) //333
  , mRawCBDone(0) //334
  , mJpegCBDone(0) //335
  , mCaptueIndex(0) //340
  , mTestMode(0) //345
{
    mu4FinalGainDiff[0] = 0;
    mu4FinalGainDiff[1] = 0;

    mpCamExif[0] = NULL;
    mpCamExif[1] = NULL;
    mpCamExif[2] = NULL;

#if 1    //setShotParam() default values
    HDRProc_ShotParam param;
#if HDR_DEBUG_FORCE_SINGLE_RUN
    param.mi4PictureWidth = 1600;
    param.mi4PictureHeight = 1200;
#else
    //param.mi4PictureWidth = 4000;
    //param.mi4PictureHeight = 3000;
    param.mi4PictureWidth = 3264;
    param.mi4PictureHeight = 2448;
#endif
#if HDR_DEBUG_FORCE_ROTATE
    param.mi4Rotation = 90;
#endif
    param.mi4PostviewWidth = 800;
    param.mi4PostviewHeight = 600;
    setShotParam(&param, sizeof(HDRProc_ShotParam));

    mPostviewFormat = eImgFmt_YV12;

    #if 0
    mu4W_yuv = 1280;
    mu4H_yuv = 960;
    #endif
    #if 0    //from EV Bracket
    mu4W_yuv = 4000;
    mu4H_yuv = 3000;
    #endif
    #if 0    //from EV Bracket
    mu4W_yuv = 2048;    //2592;
    mu4H_yuv = 1536;    //1944;
    #endif
    mu4SourceSize = mu4W_yuv * mu4H_yuv * 3/2;    //eImgFmt_I420
    mu4FirstRunSourceSize = mu4W_first * mu4H_first * 3/2;    //eImgFmt_I420

    mPostviewWidth = 800;
    mPostviewHeight = 600;
    mPostviewFormat = eImgFmt_YV12;
#endif

#if 1
    mu4OutputFrameNum = 3;
    #if 1    //ev bracket
    mu4FinalGainDiff[0]    = 2048;
    mu4FinalGainDiff[1]    = 512;
    mu4TargetTone        = 150;
    #endif
#endif

    for(MUINT32 i=0; i<eMaxOutputFrameNum; i++) {
        mpSourceImgBuf[i].virtAddr = 0;
        mpFirstRunSourceImgBuf[i].virtAddr = 0;
        mpSmallImgBuf[i].virtAddr = 0;
        mpSEImgBuf[i].virtAddr = 0;
        mWeightingBuf[i].virtAddr = 0;
        mpBlurredWeightMapBuf[i].virtAddr = 0;
        mpDownSizedWeightMapBuf[i].virtAddr = 0;
    }
    mpPostviewImgBuf.virtAddr = 0;
    mpResultImgBuf.virtAddr = 0;
    mpHdrWorkingBuf.virtAddr = 0;
    mpMavWorkingBuf.virtAddr = 0;
    mRawBuf.virtAddr = 0;
    mNormalJpegBuf.virtAddr = 0;
    mNormalThumbnailJpegBuf.virtAddr = 0;
    mHdrJpegBuf.virtAddr = 0;
    mHdrThumbnailJpegBuf.virtAddr = 0;
    mBlendingBuf.virtAddr = 0;

}


/******************************************************************************
 *
 ******************************************************************************/
HdrShot::
~HdrShot()
{
}

/*******************************************************************************
*
*******************************************************************************/
bool
HdrShot::
setShotParam(void const* pParam __attribute__((unused))
    , size_t const size __attribute__((unused)))
{
    FUNCTION_LOG_START;
    bool ret = true;

    if(NULL == pParam){
        MY_ERR("[setShotParam] NULL pParam.");
        return false;
    }

    HDRProc_ShotParam * in_param = (HDRProc_ShotParam *)pParam;
    mShotParam.mi4PictureWidth = in_param->mi4PictureWidth;
    mShotParam.mi4PictureHeight = in_param->mi4PictureHeight;
    mShotParam.mi4PostviewWidth = 800;
    mShotParam.mi4PostviewHeight = 600;
    mShotParam.mu4Transform = mPhotoTransform;
    mShotParam.mScalerCropRegion = in_param->mScalerCropRegion;
    mShotParam.mu4ZoomRatio= in_param->mu4ZoomRatio;

    FUNCTION_LOG_END;
    return ret;
}

/*******************************************************************************
*
*******************************************************************************/
bool
HdrShot::
setJpegParam(void const* pParam __attribute__((unused))
    , size_t const size __attribute__((unused)))
{
    FUNCTION_LOG_START;
    bool ret = true;

    if(NULL == pParam){
        MY_ERR("[setJpegParam] NULL pParam.");
        return false;
    }

    HDRProc_JpegParam * in_param = (HDRProc_JpegParam*)pParam;
    mJpegParam.mu4JpegQuality            = in_param->mu4JpegQuality;
    mJpegParam.mu4JpegThumbQuality       = in_param->mu4JpegQuality;
    mJpegParam.mi4JpegThumbWidth         = in_param->mi4JpegThumbWidth;
    mJpegParam.mi4JpegThumbHeight        = in_param->mi4JpegThumbHeight;

    FUNCTION_LOG_END;
    return ret;
}

#endif


#if HDR_USE_THREAD

/*******************************************************************************
*
********************************************************************************/
HdrState_e
HdrShot::GetHdrState(void)
{
    FUNCTION_LOG_START;
    return mHdrState;
}


/*******************************************************************************
*
********************************************************************************/
void
HdrShot::SetHdrState(HdrState_e eHdrState)
{
    FUNCTION_LOG_START;
    mHdrState = eHdrState;
}

/*******************************************************************************
*
********************************************************************************/
MVOID *mHalCamHdrThread(MVOID *arg)
{
    FUNCTION_LOG_START;
    MBOOL   ret = MTRUE;

    ::prctl(PR_SET_NAME,"mHalCamHdrThread", 0, 0, 0);   // Give this thread a name.
    ::pthread_detach(::pthread_self()); // Make this thread releases all its resources when it's finish.

    //
    MINT32  err = 0;     // 0: No error.
    HdrState_e eHdrState;

    //
    MY_DBG("[mHalCamHdrThread] tid: %d.", gettid());
    eHdrState = pHdrObj->GetHdrState();
    while (eHdrState != HDR_STATE_UNINIT)
    {
        ::sem_wait(&semHdrThread);
        eHdrState = pHdrObj->GetHdrState();
        MY_DBG("[mHalCamHdrThread] Got semHdrThread. eHdrState: %d.", eHdrState);

        pHdrObj->mHalCamHdrProc(eHdrState);
        ::sem_post(&semHdrThreadBack);

    }

    ::sem_post(&semHdrThreadEnd);

    MY_DBG("[mHalCamHdrThread] - X. err: %d.", err);
    return NULL;
}


#if 1
/*******************************************************************************
*
********************************************************************************/
MINT32
HdrShot::mHalCamHdrProc(HdrState_e eHdrState)
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;
    MY_DBG("[mHalCamHdrProc] - E. eHdrState: %d.", eHdrState);

    #if (HDR_PROFILE_CAPTURE2)
    MyDbgTimer DbgTmr("mHalCamHdrProc");
    #endif


    //
//    if (!mfdObj)
//    {
//        break;
//    }
//    ///
//    mfdObj->lock();


    switch (eHdrState)
    {
        case HDR_STATE_INIT:
        MY_DBG("[mHalCamHdrProc] HDR_STATE_INIT.");
        break;

        case HDR_STATE_NORMALIZATION:
        {
            MY_DBG("[mHalCamHdrProc] HDR_STATE_NORMALIZATION.");
          ret =
              //  ()  Normalize small images, and put them back to SmallImg[].
              do_Normalization()
                                                    #if (HDR_PROFILE_CAPTURE2)
                                                    &&  DbgTmr.print("HdrProfiling:: do_Normalization Time")
                                                    #endif
              ;
        }
        break;

        case HDR_STATE_FEATURE_EXTRACITON:
        {
            MY_DBG("[mHalCamHdrProc] HDR_STATE_FEATURE_EXTRACITON.");
          ret =
              //  ()  Do Feature Extraciton.
              do_FeatureExtraction()
                                                    #if (HDR_PROFILE_CAPTURE2)
                                                    &&  DbgTmr.print("HdrProfiling:: do_FeatureExtraction Time")
                                                    #endif
              ;
        }
        break;

        case HDR_STATE_ALIGNMENT:
        {
            MY_DBG("[mHalCamHdrProc] HDR_STATE_ALIGNMENT.");
            ret =
                //  ()  Do Alignment (includeing "Feature Matching" and "Weighting Map Generation").
                do_Alignment()
                                                    #if (HDR_PROFILE_CAPTURE2)
                                                    &&  DbgTmr.print("HdrProfiling:: do_Alignment Time")
                                                    #endif
                ;
        }
        break;

        case HDR_STATE_BLEND:
        {
            MY_DBG("[mHalCamHdrProc] HDR_STATE_BLEND.");
          ret =
              //  ()  Do Fusion.
              do_Fusion()
                                                    #if (HDR_PROFILE_CAPTURE2)
                                                    &&  DbgTmr.print("HdrProfiling:: do_Fusion Time")
                                                    #endif
              ;
        }
        break;

        case HDR_STATE_UNINIT:
        MY_DBG("[mHalCamHdrProc] HDR_STATE_UNINIT.");
        // Do nothing. Later will leave while() and post semHdrThreadEnd to indicate that mHalCamHdrThread is end and is safe to uninit.
        break;

        default:
            MY_DBG("[mHalCamHdrProc] undefined HDR_STATE, do nothing.");

    }


    //
//    if (mfdObj)
//    {
//        mfdObj->unlock();
//    }

    //

    #if (HDR_PROFILE_CAPTURE2)
    DbgTmr.print("HdrProfiling:: mHalCamHdrProc Finish.");
    #endif

    FUNCTION_LOG_END;
    return ret;

}
#endif

#endif  // HDR_USE_THREAD


static HdrShot*  mpHdrShot = NULL;

MBOOL HDRProcInit()
{
    FUNCTION_LOG_START;
    MBOOL   ret = MTRUE;

    if (mpHdrShot == NULL)
	{
        mpHdrShot = new HdrShot("hdr", 0, 0);
		if (mpHdrShot == NULL)
		{
            CAM_LOGE("[%s] new HdrShot", __FUNCTION__);
            goto lbExit;
		}
    }
lbExit:

    FUNCTION_LOG_END;
    return ret;
}

MBOOL HDRProcUnInit()
{
    FUNCTION_LOG_START;
    MBOOL   ret = MTRUE;

    if (mpHdrShot != NULL) {
        delete mpHdrShot;
    }
    mpHdrShot = NULL;
    FUNCTION_LOG_END;
    return ret;
}

MBOOL
HDRProcSetShotParam(void * pParam)
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

    if(NULL == pParam){
        MY_ERR("[HDRProcSetShotParam] NULL pParam.");
        return false;
    }

    HDRProc_ShotParam * in_param = (HDRProc_ShotParam *)pParam;
    HDRProc_ShotParam param;
    param.mi4PictureWidth = in_param->mi4PictureWidth;
    param.mi4PictureHeight = in_param->mi4PictureHeight;
    param.mi4PostviewWidth = 800;
    param.mi4PostviewHeight = 600;
    param.mu4Transform = mpHdrShot->mPhotoTransform;
    param.mScalerCropRegion = in_param->mScalerCropRegion;
    param.mu4ZoomRatio= in_param->mu4ZoomRatio;

    MY_DBG("[HDRProcSetShotParam] Picture(%d X %d)",param.mi4PictureWidth,param.mi4PictureHeight);
    if(!mpHdrShot->setShotParam(&param, sizeof(HDRProc_ShotParam))) {
        MY_ERR("[HDRProc] HDRProcSetShotParam fail.");
        ret = false;
    }

    FUNCTION_LOG_END;
    return ret;
}

MBOOL
HDRProcSetJpegParam(void * pParam)
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

    if(NULL == pParam){
        MY_ERR("[HDRProcSetJpegParam] NULL pParam.");
        return false;
    }

    HDRProc_JpegParam * in_param = (HDRProc_JpegParam*)pParam;
    HDRProc_JpegParam jpegParam;
    jpegParam.mu4JpegQuality            = in_param->mu4JpegQuality;
    jpegParam.mu4JpegThumbQuality       = in_param->mu4JpegQuality;
    jpegParam.mi4JpegThumbWidth         = in_param->mi4JpegThumbWidth;
    jpegParam.mi4JpegThumbHeight        = in_param->mi4JpegThumbHeight;
    //jpegParam.mi4JpsHeight              = in_param->mi4JpsHeight;
    //jpegParam.mi4JpsWidth               = in_param->mi4JpsWidth;

    if(!mpHdrShot->setJpegParam(&jpegParam, sizeof(HDRProc_JpegParam))) {
        MY_ERR("[HDRProc] HDRProcSetJpegParam fail.");
        ret = false;
    }

    FUNCTION_LOG_END;
    return ret;
}

MBOOL
HDRProcPrepare()
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

    ret = mpHdrShot->updateInfo_cam3();

    if(MTRUE != ret){
        MY_ERR("[HDRProcPrepare] init_cam3 fail.");
        return MFALSE;
    }
    ret = mpHdrShot->EVBracketCapture_cam3();

    if(MTRUE != ret){
        MY_ERR("[HDRProcPrepare] createSourceAndSmallImg_cam3 fail.");
        return MFALSE;
    }
    FUNCTION_LOG_END;
    return ret;
}


MBOOL
HDRProcAddInputFrame(MINT32 frame_index, const android::sp<IImageBuffer> imgbuf)
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;


    ret = mpHdrShot->registerSrcSmallImg_cam3(frame_index,imgbuf);
    FUNCTION_LOG_END;
    return ret;
}

MBOOL
HDRProcAddOutFrame(MINT32 frame_index,sp<IImageBuffer> imgbuf)
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

    ret = mpHdrShot->registerOutputImg_cam3(frame_index,imgbuf);
    FUNCTION_LOG_END;
    return ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HDRProcStart()
{
	FUNCTION_LOG_START;
    MBOOL ret = MTRUE;
    // TODO, should start a thread to do process_cam3
    ret = mpHdrShot->process_cam3();

    if(MTRUE != ret){
        MY_ERR("[HDRProcStart] process_cam3 fail.");
        return MFALSE;
    }


    FUNCTION_LOG_END;
	return ret;
}

/*******************************************************************************
*
*******************************************************************************/

MBOOL
HDRProcRelease()
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

    ret = mpHdrShot->release_cam3();

    if(MTRUE != ret){
        MY_ERR("[HDRProcRelease] release_cam3 fail.");
        return MFALSE;
    }


    FUNCTION_LOG_END;
    return ret;
}


/*******************************************************************************
*
*******************************************************************************/

MBOOL
HDRProcGetBuf(IImageBuffer** src0 __attribute__((unused))
    , IImageBuffer** small0 __attribute__((unused))
    , IImageBuffer** src1 __attribute__((unused))
    , IImageBuffer** small1 __attribute__((unused)))
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;
    #if 0
    *src0 = mpHdrShot->mpSourceImgBuf[0];
    *src1 = mpHdrShot->mpSourceImgBuf[1];
    *small0 = mpHdrShot->mpSmallImgBuf[0];
    *small1 = mpHdrShot->mpSmallImgBuf[1];
    #endif
    FUNCTION_LOG_END;
    return ret;
}


/*******************************************************************************
*
*******************************************************************************/

MBOOL
HDRProcGetCapInfo(android::Vector<NS3A::CaptureParam_T> & vCap3AParam, MINT32 &hdrFrameNum)
{
    MBOOL ret = MTRUE;

    ret = mpHdrShot->getCaptureInfo_cam3(vCap3AParam,hdrFrameNum);

    return ret;
}

/*******************************************************************************
*
*******************************************************************************/

MBOOL
HDRProcSetParam(MUINT32 paramId, MUINTPTR iArg1, MUINTPTR iArg2)
{
    MBOOL ret = MTRUE;

    MY_DBG("[HDRProcSetParam] paramId:%d ,iArg1:%zu, iArg2:%zu\n",paramId,iArg1,iArg2);
    switch (paramId)
    {
        case HDRProcParam_Set_sensor_size:
            mpHdrShot->mSensorCaptureWidth = iArg1;
            mpHdrShot->mSensorCaptureHeight = iArg2;
            break;
        case HDRProcParam_Set_sensor_type:
            mpHdrShot->mSensorType = iArg1;
            break;
        case HDRProcParam_Set_transform:
            mpHdrShot->mPhotoTransform = iArg1;
            break;
        case HDRProcParam_Set_AOEMode:
            mpHdrShot->mu4AOEMode = iArg1;
            break;
        case HDRProcParam_Set_MaxSensorAnalogGain:
            mpHdrShot->mu4MaxSensorAnalogGain = iArg1;
            break;
        case HDRProcParam_Set_MaxAEExpTimeInUS:
            mpHdrShot->mu4MaxAEExpTimeInUS = iArg1;
            break;
        case HDRProcParam_Set_MinAEExpTimeInUS:
            mpHdrShot->mu4MinAEExpTimeInUS = iArg1;
            break;
        case HDRProcParam_Set_ShutterLineTime:
            mpHdrShot->mu4ShutterLineTime = iArg1;
            break;
        case HDRProcParam_Set_MaxAESensorGain:
            mpHdrShot->mu4MaxAESensorGain = iArg1;
            break;
        case HDRProcParam_Set_MinAESensorGain:
            mpHdrShot->mu4MinAESensorGain = iArg1;
            break;
        case HDRProcParam_Set_ExpTimeInUS0EV:
            mpHdrShot->mu4ExpTimeInUS0EV = iArg1;
            break;
        case HDRProcParam_Set_SensorGain0EV:
            mpHdrShot->mu4SensorGain0EV = iArg1;
            break;
        case HDRProcParam_Set_FlareOffset0EV:
            mpHdrShot->mu1FlareOffset0EV = iArg1;
            break;
        case HDRProcParam_Set_GainBase0EV:
            mpHdrShot->mi4GainBase0EV = iArg1;
            break;
        case HDRProcParam_Set_LE_LowAvg:
            mpHdrShot->mi4LE_LowAvg = iArg1;
            break;
        case HDRProcParam_Set_SEDeltaEVx100:
            mpHdrShot->mi4SEDeltaEVx100 = iArg1;
            break;
        case HDRProcParam_Set_Histogram:
        {
            MUINT32 * pHistogram = (MUINT32*)iArg1;
            if(NULL != pHistogram){
                memcpy((void*)mpHdrShot->mu4Histogram
                        , (void*)pHistogram
                        , sizeof(mpHdrShot->mu4Histogram));
            }
        }
            break;
        default:
            return MFALSE;
    }

    return ret;
}
/*******************************************************************************
*
*******************************************************************************/

MBOOL
HDRProcGetParam(MUINT32 paramId, MUINT32 & rArg1, MUINT32 & rArg2)
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

    switch (paramId)
    {
        case HDRProcParam_Get_src_main_format:
            rArg1 = eImgFmt_I420 ;
            break;
        case HDRProcParam_Get_src_main_size:
            rArg1 = mpHdrShot->mu4W_yuv ;
            rArg2 = mpHdrShot->mu4H_yuv ;
            break;
        case HDRProcParam_Get_src_small_format:
            rArg1 = eImgFmt_Y800 ;
            break;
        case HDRProcParam_Get_src_small_size:
            rArg1 = mpHdrShot->mu4W_small ;
            rArg2 = mpHdrShot->mu4H_small ;
            break;
        default:
            return MFALSE;
    }

    MY_DBG("[HDRProcGetParam] paramId:%d ,iArg1:%d, iArg2:%d\n",paramId,rArg1,rArg2);

    FUNCTION_LOG_END;
    return ret;
}

/*******************************************************************************
*
*******************************************************************************/
MVOID
HDRProcSetCallBack(HDRProcCompleteCallback_t complete_cb, MVOID* user)
{

    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

    if (NULL != complete_cb){
        mpHdrShot->mCompleteCb = complete_cb;
        mpHdrShot->mpCbUser = user;
    }

    FUNCTION_LOG_END;
    return;
}


