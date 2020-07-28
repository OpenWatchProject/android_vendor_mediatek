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
#define LOG_TAG "MtkCam/HDRShot"

#include "MyHdr.h"
#include <utils/threads.h>
#include <sys/prctl.h>  // For prctl()/PR_SET_NAME.

//
#include <mtkcam/Log.h>
#include <mtkcam/common.h>
#include <mtkcam/exif/IBaseCamExif.h>
#include <mtkcam/exif/CamExif.h>
//
#include <Shot/IShot.h>
//
#include "ImpShot.h"
#include "Hdr.h"
//
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
//Use PowerHal instead of PerfService
#include <vendor/mediatek/hardware/power/1.1/IPower.h>
#include <vendor/mediatek/hardware/power/1.1/types.h>

using namespace vendor::mediatek::hardware::power::V1_1;

static android::sp<IPower> gPowerHal = NULL;

//-----------------------------------------------------------------------------
#ifdef HDRSHOT_USE_SCHED
#include <libsched/libsched.h>

static int enableForceToBigCore()
{
    HDR_TRACE_CALL();

    MBOOL ret;
    pid_t tid = gettid();
    cpu_set_t cpuset;
    int s;
    unsigned int Msk, cpu_no;
    int ScenHandle = -1;

    // use PerfServeice to enable 4 big core to highest freq
    int cpu_msk = 0xF0;
    // put current thread to big cores
    CPU_ZERO(&cpuset);
    for (Msk=1, cpu_no=0; Msk<0xFF; Msk<<=1, cpu_no++) {
        if (Msk&cpu_msk) {
            CPU_SET(cpu_no, &cpuset);
        }
    }

    s = mt_sched_setaffinity(tid, sizeof(cpu_set_t), &cpuset);
    if (s != 0) {
        CAM_LOGE("mt_sched_setaffinity error!!");
		ret = MFALSE;
        goto lbExit;
    }

    ret = MTRUE;

lbExit:
    return ScenHandle;
}

static MBOOL disableForceToBigCore()
{
    HDR_TRACE_CALL();

    MBOOL ret = MTRUE;

    pid_t tid = gettid();
    int status = mt_sched_exitaffinity(tid);
    if (status != 0) {
        CAM_LOGE("mt_sched_exitaffinity() status=%d, error!!", status);
        ret = MFALSE;
    }

lbExit:
    return ret;
}
#else
static int enableForceToBigCore()
{
    return -1;
}

static MBOOL disableForceToBigCore()
{
    return MTRUE;
}
#endif

static int EnablePerfService()
{
    // The following is using PowerHal instead of PerfService
    // Cluster 0: Little
    // Cluster 1: Big
    //int PerfServiceHandle = PerfServiceNative_userRegBigLittle(4, 1950000, 4, 870000);

    if(gPowerHal == NULL) {
        CAM_LOGE("IPower error!!");
        return -1;
    }

    int PerfServiceHandle = gPowerHal->scnReg();

    gPowerHal->scnConfig(PerfServiceHandle, MtkPowerCmd::CMD_SET_CLUSTER_CPU_CORE_MIN, 0, 4, 0, 0);
    gPowerHal->scnConfig(PerfServiceHandle, MtkPowerCmd::CMD_SET_CLUSTER_CPU_FREQ_MIN, 0, 870000, 0, 0);
    gPowerHal->scnConfig(PerfServiceHandle, MtkPowerCmd::CMD_SET_CLUSTER_CPU_CORE_MIN, 1, 4, 0, 0);
    gPowerHal->scnConfig(PerfServiceHandle, MtkPowerCmd::CMD_SET_CLUSTER_CPU_FREQ_MIN, 1, 1950000, 0, 0);


    if (PerfServiceHandle != -1) {
        gPowerHal->scnEnable(PerfServiceHandle, 3000);
    } else {
        CAM_LOGE("register PerfService scenario error!!");
    }
    return PerfServiceHandle;
}

static void DisablePerfService(int PerfServiceHandle)
{
    if(gPowerHal == NULL) {
        CAM_LOGE("IPower error!!");
        return;
    }

    if (PerfServiceHandle != -1) {
        //PerfServiceNative_userDisable(PerfServiceHandle);
        //PerfServiceNative_userUnreg(PerfServiceHandle);
        gPowerHal->scnDisable(PerfServiceHandle);
        gPowerHal->scnUnreg(PerfServiceHandle);
        gPowerHal = NULL;
    }
}

//-----------------------------------------------------------------------------


using namespace android;
using namespace NSShot;

typedef void* hdr_object_t;
static Mutex sHDRListLock;
static DefaultKeyedVector<hdr_object_t, sp<HdrShot>> sHDRList;

extern
sp<IShot>
createInstance_HdrShot(
    char const*const    pszShotName,
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId
)
{
    HDR_TRACE_CALL();

    sp<IShot>       pShot = NULL;
    sp<HdrShot>  pImpShot = NULL;
    //
    //  (1.1) new Implementator.
    pImpShot = new HdrShot(pszShotName, u4ShotMode, i4OpenId);
    if  ( pImpShot == 0 ) {
        CAM_LOGE("new HdrShot error!!");
        goto lbExit;
    }
    //
    //  (1.2) initialize Implementator if needed.
    if  ( ! pImpShot->onCreate() ) {
        CAM_LOGE("onCreate() error!!");
        goto lbExit;
    }
    //
    //  (2)   new Interface.
    pShot = new IShot(pImpShot);
    if  ( pShot == 0 ) {
        CAM_LOGE("new IShot() error!!");
        goto lbExit;
    }
    //
lbExit:
    //
    //  Free all resources if this function fails.
    if  ( pShot == 0 && pImpShot != 0 ) {
        pImpShot->uninit();
        pImpShot = NULL;
    }
    //
    return  pShot;
}


/******************************************************************************
 *  This function is invoked when this object is firstly created.
 *  All resources can be allocated here.
 ******************************************************************************/
bool
HdrShot::onCreate()
{
    HDR_TRACE_CALL();

    return true;
}


/******************************************************************************
 *  This function is invoked when this object is ready to destryoed in the
 *  destructor. All resources must be released before this returns.
 ******************************************************************************/
void
HdrShot::onDestroy()
{
    HDR_TRACE_CALL();
}


/*******************************************************************************
*
*******************************************************************************/
HdrShot::HdrShot(char const*const pszShotName, uint32_t const u4ShotMode, int32_t const i4OpenId)
    : ImpShot(pszShotName, u4ShotMode, i4OpenId)
    ////    Resolutions.
    , mRaw_Width(0)
    , mRaw_Height(0)
    , mu4W_yuv(0)
    , mu4H_yuv(0)
    , mu4W_small(0)
    , mu4H_small(0)
    , mu4W_se(0)
    , mu4H_se(0)
    , mu4W_dsmap(0)
    , mu4H_dsmap(0)
    , mPostviewWidth(800)
    , mPostviewHeight(600)
    , mPostviewFormat(eImgFmt_YV12)
    , mRotPicWidth(0)
    , mRotPicHeight(0)
    , mRotThuWidth(0)
    , mRotThuHeight(0)

    , mErrorFlag(0)
    //
    , mMainThread(0)
    , mMemoryAllocateThread(0)
    //
    , mTrigger_alloc_working(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mTrigger_alloc_bmap1(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mMemoryReady_Capbuf(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mMemoryReady_pass2_first(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mMemoryReady_pass2_others(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mMemoryReady_jpeg_full(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mMemoryReady_jpeg_thumbnail(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mMemoryReady_working(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mMemoryReady_se(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mMemoryReady_bmap0(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mMemoryReady_bmap1(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mMemoryReady_bmap2(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mMemoryReady_blending(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mMemoryReady_postview(PTHREAD_MUTEX_INITIALIZER_LOCK)

    // pipes
    , mpHdrHal(NULL)
    // Buffers
    , mpIMemDrv(NULL)
    , mpIImageBufAllocator(NULL)
    , mpCapBufMgr(NULL)
    //, std::vector<ImageBufferMap> mvImgBufMap;
    , mpHeap(NULL)
    , mTotalBufferSize(0)
    , mTotalKernelBufferSize(0)
    , mTotalUserBufferSize(0)
    //
    , mBlendingBuf(NULL)
    , mpPostviewImgBuf(NULL)
    //
    , mpHdrWorkingBuf(NULL)
    , mpMavWorkingBuf(NULL)
    //
    , mNormalJpegBuf(NULL)
    , mNormalThumbnailJpegBuf(NULL)
    , mHdrJpegBuf(NULL)
    , mHdrThumbnailJpegBuf(NULL)

    //
    , OriWeight(NULL)
    , BlurredWeight(NULL)

    //
    //, mu4RunningNumber(0)
    , mu4OutputFrameNum(0)
    //, mu4FinalGainDiff[2]
    , mu4TargetTone(0)
    //
    , mShutterCBDone(MFALSE)
    , mRawCBDone(MFALSE)
    , mJpegCBDone(MFALSE)
    , mfgIsSkipthumb(MFALSE)
    //
    , mCaptueIndex(0)
    , mSensorType(0)
    //
    , mNrtype(0)

    // for development
    , mTestMode(0)
    , mDebugMode(0)
    , mPrivateData(NULL)
    , mPrivateDataSize(0)
    , mSelfInstanceHolder(NULL)
{
    HDR_TRACE_CALL();

    // find an avaliable ID for debugging purpose
    {
        int i;
        Mutex::Autolock _l(mIDLock);
        mID = mIDNext;
        for (i=0;i<32;i++) {
            if(mIDUsingFlag & (1<<mID)) {
                mID = (mID + 1) & 0x1F;
            } else {
                mIDUsingFlag |= (1<<mID);
                break;
            }
        }
        mIDNext = (mID + 1) & 0x1F;
    }

    mHDRShotMode = u4ShotMode;
    for(MUINT32 i=0; i<eMaxOutputFrameNum; i++) {
        //mpCamExif[i] = new CamExif;
        mpSourceRawImgBuf[i] = NULL;
        mpSourceImgBuf[i] = NULL;
        mpSmallImgBuf[i] = NULL;
        mpSEImgBuf[i] = NULL;
        mWeightingBuf[i] = NULL;
        mpBlurredWeightMapBuf[i] = NULL;
        mpDownSizedWeightMapBuf[i] = NULL;
    }

    //setShotParam() default values
    ShotParam param;
    param.mi4PictureWidth = 3264;
    param.mi4PictureHeight = 2448;
    param.mi4PostviewWidth = 800;
    param.mi4PostviewHeight = 600;
    setShotParam(&param, sizeof(ShotParam));

    mu4OutputFrameNum = 3;
    mu4FinalGainDiff[0]    = 2048;
    mu4FinalGainDiff[1]    = 512;
    mu4TargetTone        = 150;

    //PowerHal
    if(gPowerHal == NULL) {
        gPowerHal = IPower::getService();
    }
}


/******************************************************************************
 *
 ******************************************************************************/
HdrShot::~HdrShot()
{
    HDR_TRACE_CALL();

    uninit();

    {
        Mutex::Autolock _l(mIDLock);
        mIDUsingFlag &= ~(1<<mID);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HdrShot::setCapBufMgr(MVOID* pCapBufMgr)
{
    HDR_TRACE_CALL();

    mpCapBufMgr = (CapBufMgr*)pCapBufMgr;
    return MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
HdrShot::sendCommand(
    uint32_t const  cmd,
    MUINTPTR const  arg1,
    uint32_t const  arg2,
    uint32_t const  arg3
)
{
    HDR_TRACE_CALL();

    bool ret = true;
    //
    switch  (cmd)
    {
    //  This command is to reset this class. After captures and then reset,
    //  performing a new capture should work well, no matter whether previous
    //  captures failed or not.
    //
    //  Arguments:
    //          N/A
    case eCmd_reset:
        ret = onCmd_reset();
        break;

    //  This command is to perform capture.
    //
    //  Arguments:
    //          N/A
    case eCmd_capture:
        ret = onCmd_capture();
        break;

    //  This command is to perform cancel capture.
    //
    //  Arguments:
    //          N/A
    case eCmd_cancel:
        onCmd_cancel();
        break;
    //
    default:
        ret = ImpShot::sendCommand(cmd, arg1, arg2, arg3);
    }

    //
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
HdrShot::onCmd_reset()
{
    HDR_TRACE_CALL();

    return true;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
HdrShot::onCmd_capture()
{
    HDR_TRACE_CALL();

    bool ret = true;
    pthread_t           mainThread;
    hdr_object_t        hdr_id;
    sp<HdrShot> obj;

    if (mHDRShotMode == eShotMode_ZsdHdrShot) {
        mpCapBufMgr = ImpShot::mpCapBufMgr;
    }

    CAM_LOGI("<%02d> Capture", mID);

    ret = ret
        && init()
        && EVBracketCapture()
        ;

    if (!ret) {
        CAM_LOGE("<%02d> Capture error!!", mID);
        goto lbExit;
    }

    mInstanceLock.lock();
    mInstanceCount++;

    // NOTE:
    // After this function return, HdrShot will be free by CamAdapter,
    // so HdrShot MUST hold on itself until HDRProcessTask() is done.
    // HdrShot::mSelfInstanceHolder is designed for this purpose

    //mSelfInstanceHolder = this;
    {
        AutoMutex l(sHDRListLock);
        obj = this;
        hdr_id = reinterpret_cast<hdr_object_t>(this);
        sHDRList.add(hdr_id, obj);
    }

    pthread_create(&mainThread, NULL, HdrShot::HDRProcessTask, this);

    if (mInstanceCount == 1) {
        mPerfServiceHandle = EnablePerfService();
    }

    mInstanceLock.unlock();

lbExit:
    return ret;
}


MVOID*
HdrShot::HDRProcessTask(MVOID *arg)
{
    ::prctl(PR_SET_NAME,__FUNCTION__, 0, 0, 0);
    HDR_TRACE_CALL();

    MUINTPTR ret = true;

    pthread_detach(pthread_self());

    HdrShot *self = (HdrShot*)arg;

    ret = self->mainflow();

    mInstanceLock.lock();
    mInstanceCount--;
    if (mInstanceCount == 0) {
        CAM_LOGI("All HDRProcessTask() done");
        DisablePerfService(mPerfServiceHandle);
        mPerfServiceHandle = -1;
        mCancelAck.broadcast();
    }

    {
        AutoMutex l(sHDRListLock);
        //free HdrShot HERE
        sHDRList.removeItem(arg);
    }
    mInstanceLock.unlock();

    return (MVOID*)ret;
}

Mutex HdrShot::mIDLock;
Mutex HdrShot::mHDRLock;
Mutex HdrShot::mMAVLock;
Mutex HdrShot::mJPEGLock;
int HdrShot::mIDNext = 0;
int HdrShot::mIDUsingFlag = 0;
int HdrShot::mInstanceCount = 0;
Condition HdrShot::mCancelAck;
Mutex HdrShot::mInstanceLock;
int HdrShot::mCancelFlag = 0;
int HdrShot::mPerfServiceHandle = -1;

bool
HdrShot::mainflow()
{
    HDR_TRACE_CALL();

    bool ret = true;

    enableForceToBigCore();

    if (mCancelFlag) {
        goto lbExit;
    }

    // HDR process
    do {
        Mutex::Autolock _l(mHDRLock);

        // P2 callback here!!;
        if (mHDRShotMode == eShotMode_ZsdHdrShot) {
            CAM_LOGI("<%02d> P2done callback", mID);
            if (!mpShotCallback->onCB_P2done()) {
                CAM_LOGE("onCB_P2done() error!!");
            }
        }

        CAM_LOGI("<%02d> process", mID);
        ret = ret && ImageRegistratoin();
        ret = ret && WeightingMapGeneration();
        ret = ret && Blending();
    } while (0);

    if (mCancelFlag || !ret) {
        goto lbExit;
    }

    {
        Mutex::Autolock _l(mJPEGLock);
        CAM_LOGI("<%02d> JPEG encode", mID);
        // JPEG encode and callback
        ret = ret && encodeHdrThumbnailJpeg();

        releasePostviewImgBuf();

        ret = ret && encodeHdrJpeg();
        ret = ret && saveHdrJpeg();

        releaseHdrJpegBuf();
        releaseHdrThumbnailJpegBuf();
    }

    if(mCancelFlag || !ret) {
        goto lbExit;
    }

    if(!mTestMode)
    {
        if(!mShutterCBDone) {
            CAM_LOGW("send fake onCB_Shutter");
            if (!mpShotCallback->onCB_Shutter(true,0)) {
                CAM_LOGE("onCB_Shutter() error!!");
            }
        }

        if(!mJpegCBDone) {
            CAM_LOGW("send fake onCB_CompressedImage");
            #define __EXIF_HDR_SIZE 512
            MUINT8  puExifHeaderBuf[__EXIF_HDR_SIZE];
            #define __JPEG_SIZE 512
            MUINT8  puJpegBuf[__JPEG_SIZE];

            ret = ret && mpShotCallback->onCB_CompressedImage(0,
                __JPEG_SIZE, reinterpret_cast<uint8_t const*>(puJpegBuf),
                __EXIF_HDR_SIZE, puExifHeaderBuf,
                0,                   //callback index
                MTRUE                //final image
                );
            if(mCancelFlag || !ret) {
                goto lbExit;
            }
        }
    }
    CAM_LOGI("<%02d> done", mID);

    //@TODO list
    //#fail
    //#document
    //#full frame
    //#speed up do_DownScaleWeightMap()

lbExit:
    //error handler
    if (!ret) {
        CAM_LOGE("<%02d> error!!", mID);
    }

    if (mCancelFlag) {
        CAM_LOGI("<%02d> cancelled", mID);
    }

    // Don't know exact time of lbExit in HDR flow, so release all again
    // (there is protection in each release function).
    releaseSourceRawImgBuf();
    releaseSourceImgBuf();
    releaseSmallImgBuf();
    releaseSEImgBuf();
    releaseHdrWorkingBuf();
    releaseOriWeightMapBuf();
    releaseDownSizedWeightMapBuf();
    releaseBlurredWeightMapBuf();
    releasePostviewImgBuf();

    releaseNormalJpegBuf();
    releaseNormalThumbnailJpegBuf();
    releaseHdrJpegBuf();
    releaseHdrThumbnailJpegBuf();

    releaseBlendingBuf();

    disableForceToBigCore();

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
void
HdrShot::onCmd_cancel()
{
    HDR_TRACE_CALL();
    CAM_LOGI("<%02d> %s(), ignore it", mID, __FUNCTION__);

    // NOTE: due to pass2done and jpeg callback should be paired
    // do not cancel the in-flight HDR post-processings
/*
    CAM_LOGI("<%02d> %s()", mID, __FUNCTION__);

    mInstanceLock.lock();
    mCancelFlag = 1;
    while (mInstanceCount)
        mCancelAck.wait(mInstanceLock);
    mCancelFlag = 0;
    mInstanceLock.unlock();

    CAM_LOGI("<%02d> %s() done", mID, __FUNCTION__);
*/
}


/*******************************************************************************
*
*******************************************************************************/
bool
HdrShot::setShotParam(void const* pParam, size_t const size)
{
    HDR_TRACE_CALL();

    bool ret = true;

    if(!ImpShot::setShotParam(pParam, size)) {
        CAM_LOGE("setShotParam() error!!");
        ret = false;
    }

    return ret;
}
