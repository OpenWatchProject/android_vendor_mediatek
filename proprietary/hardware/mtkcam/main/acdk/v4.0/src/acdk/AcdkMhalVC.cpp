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

///////////////////////////////////////////////////////////////////////////////
// No Warranty
// Except as may be otherwise agreed to in writing, no warranties of any
// kind, whether express or implied, are given by MTK with respect to any MTK
// Deliverables or any use thereof, and MTK Deliverables are provided on an
// "AS IS" basis.  MTK hereby expressly disclaims all such warranties,
// including any implied warranties of merchantability, non-infringement and
// fitness for a particular purpose and any warranties arising out of course
// of performance, course of dealing or usage of trade.  Parties further
// acknowledge that Company may, either presently and/or in the future,
// instruct MTK to assist it in the development and the implementation, in
// accordance with Company's designs, of certain softwares relating to
// Company's product(s) (the "Services").  Except as may be otherwise agreed
// to in writing, no warranties of any kind, whether express or implied, are
// given by MTK with respect to the Services provided, and the Services are
// provided on an "AS IS" basis.  Company further acknowledges that the
// Services may contain errors, that testing is important and Company is
// solely responsible for fully testing the Services and/or derivatives
// thereof before they are used, sublicensed or distributed.  Should there be
// any third party action brought against MTK, arising out of or relating to
// the Services, Company agree to fully indemnify and hold MTK harmless.
// If the parties mutually agree to enter into or continue a business
// relationship or other arrangement, the terms and conditions set forth
// hereunder shall remain effective and, unless explicitly stated otherwise,
// shall prevail in the event of a conflict in the terms in any agreements
// entered into between the parties.
////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008, MediaTek Inc.
// All rights reserved.
//
// Unauthorized use, practice, perform, copy, distribution, reproduction,
// or disclosure of this information in whole or in part is prohibited.
////////////////////////////////////////////////////////////////////////////////

//! \file  AcdkMhalVC.cpp

#define LOG_TAG "AcdkMhalVC"

#include <stdio.h>
#include <stdlib.h>
#include <utils/threads.h>
#include <sys/prctl.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <vector>
#include <sys/resource.h>
#include <cutils/properties.h>
#include <utils/StrongPointer.h>

#include <mtkcam/main/acdk/AcdkTypes.h>
#include "AcdkLog.h"
#include "AcdkErrCode.h"
#include <mtkcam/main/acdk/AcdkCommon.h>
#include "AcdkCallback.h"
#include "AcdkSurfaceView.h"
#include "AcdkMhalBase.h"
#include "AcdkMhalVC.h"

using namespace NSAcdkMhal;

#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/imgbuf/ImageBufferHeap.h>
#include <mtkcam/def/PriorityDefs.h>

using namespace android;
using namespace std;


#ifdef ACDK_FAKE_SENSOR
#define THIS_NAME           "iopipeUseTM"
#else
#define THIS_NAME           "AcdkMhalVC"
#endif

#define MEDIA_PATH          "/data/vendor/"

#define PTHREAD_CTRL_M      (1)
#define P1_STATIC_ENQ_CNT   (2)
#define _unused(x) ((void)(x))


using namespace NSCam::NSIoPipe::NSPostProc;
using namespace NSCam::NSIoPipe::NSCamIOPipe;
using namespace NSCam::NSIoPipe;
using namespace NSCam;

/*******************************************************************************
* Global variable
*******************************************************************************/
static sem_t g_SemMainHigh, g_SemMainHighBack, g_SemMainHighEnd, g_SemPurePreviewCb;
static pthread_t g_threadMainHigh;
static AcdkMhalVC *g_pAcdkMHalVCObj = NULL;
static acdkObserver g_acdkMhalObserver;

static MINT32 g_acdkMhalVCDebug = 0;
MVOID VCPreviewCb(QParams& rParams);

/******************************************************************************
 *
 ******************************************************************************/
static INormalPipeModule* getNormalPipeModule()
{
    static auto pModule = INormalPipeModule::get();
    if  ( ! pModule ) {
        ACDK_LOGE("INormalPipeModule::get() fail");
    }
    return pModule;
}


static MVOID* createDefaultNormalPipe(MUINT32 sensorIndex, char const* szCallerName)
{
    auto pModule = getNormalPipeModule();
    if  ( ! pModule ) {
        ACDK_LOGE("getNormalPipeModule() fail");
        return NULL;
    }

    //  Select CamIO version
    size_t count = 0;
    MUINT32 const* version = NULL;
    int err = pModule->get_sub_module_api_version(&version, &count, sensorIndex);
    if  ( err < 0 || ! count || ! version ) {
        ACDK_LOGE(
            "[%d] INormalPipeModule::get_sub_module_api_version - err:%#x count:%zu version:%p",
            sensorIndex, err, count, version
        );
        return NULL;
    }

    MUINT32 const selected_version = *(version + count - 1); //Select max. version
    ACDK_LOGD("[%d] count:%zu Selected CamIO Version:%0#x", sensorIndex, count, selected_version);

    MVOID* pPipe = NULL;
    pModule->createSubModule(sensorIndex, szCallerName, selected_version, (MVOID**)&pPipe, EPipeSelect_NormalSv);
    return pPipe;
}


/*******************************************************************************
* AcdkMhalVC
* brif : Constructor
*******************************************************************************/
AcdkMhalVC::AcdkMhalVC()
    : mu4SensorDelay(0)
    , mImgoRtbc()
    , mWrotoBufCtl()
{
    mAcdkMhalState = ACDK_MHAL_NONE;

    m_pINormPipe    = NULL;
    m_pINormStream  = NULL;

    mFocusDone = MFALSE;
    mFocusSucceed = MFALSE;

    mReadyForCap = MFALSE;
    mRawPostProcSupport = MTRUE;

    memset(&mAcdkMhalPrvParam,0,sizeof(acdkMhalPrvParam_t));

    g_pAcdkMHalVCObj = this;
}

inline EImageFormat _MapFG(EImageFormat bayer_fmt)
{
    switch(bayer_fmt){
    case eImgFmt_BAYER8: return eImgFmt_FG_BAYER8;
    case eImgFmt_BAYER10: return eImgFmt_FG_BAYER10;
    case eImgFmt_BAYER12: return eImgFmt_FG_BAYER12;
    case eImgFmt_BAYER14: return eImgFmt_FG_BAYER14;
    default: break;
    }
    return eImgFmt_UNKNOWN;
}
/*******************************************************************************
* acdkMhalGetState
* brif : get state of acdk mhal
*******************************************************************************/
MVOID AcdkMhalVC::acdkMhalSetState(acdkMhalState_e newState)
{
    Mutex::Autolock lock(mLock);

    ACDK_LOGD("Now(0x%04x), Next(0x%04x)", mAcdkMhalState, newState);

    if(newState == ACDK_MHAL_ERROR)
    {
        goto ACDK_MHAL_SET_STATE_EXIT;
    }

    switch(mAcdkMhalState)
    {
    case ACDK_MHAL_NONE:
        switch(newState)
        {
        case ACDK_MHAL_INIT:
        case ACDK_MHAL_UNINIT:
            break;
        default:
            //ACDK_ASSERT(0, "State error ACDK_MHAL_NONE");
            ACDK_LOGE("State error ACDK_MHAL_NONE");
            break;
        }
        break;
    case ACDK_MHAL_INIT:
        switch(newState)
        {
        case ACDK_MHAL_IDLE:
            break;
        default:
            //ACDK_ASSERT(0, "State error MHAL_CAM_INIT");
            ACDK_LOGE("State error ACDK_MHAL_INIT");
            break;
        }
        break;
    case ACDK_MHAL_IDLE:
        switch(newState)
        {
        case ACDK_MHAL_IDLE:
        case ACDK_MHAL_PREVIEW:
        case ACDK_MHAL_CAPTURE:
        case ACDK_MHAL_UNINIT:
            break;
        default:
            //ACDK_ASSERT(0, "State error MHAL_CAM_IDLE");
            ACDK_LOGE("State error ACDK_MHAL_IDLE");
            break;
        }
        break;
    case ACDK_MHAL_PREVIEW:
        switch(newState)
        {
        case ACDK_MHAL_IDLE:
        case ACDK_MHAL_PREVIEW:
        case ACDK_MHAL_PRE_CAPTURE:
        case ACDK_MHAL_PREVIEW_STOP:
            break;
        default:
            //ACDK_ASSERT(0, "State error MHAL_CAM_PREVIEW");
            ACDK_LOGE("State error ACDK_MHAL_PREVIEW");
            break;
        }
        break;
    case ACDK_MHAL_PRE_CAPTURE:
        switch(newState)
        {
        case ACDK_MHAL_PREVIEW_STOP:
            break;
        default:
            //ACDK_ASSERT(0, "State error MHAL_CAM_PRE_CAPTURE");
            ACDK_LOGE("State error ACDK_MHAL_PRE_CAPTURE");
            break;
        }
        break;
    case ACDK_MHAL_PREVIEW_STOP:
        switch(newState)
        {
        case ACDK_MHAL_IDLE:
            break;
        default:
            //ACDK_ASSERT(0, "State error ACDK_MHAL_PREVIEW_STOP");
            ACDK_LOGE("State error ACDK_MHAL_PREVIEW_STOP");
            break;
        }
        break;
    case ACDK_MHAL_CAPTURE:
        switch(newState)
        {
        case ACDK_MHAL_IDLE:
            break;
        default:
            //ACDK_ASSERT(0, "State error MHAL_CAM_CAPTURE");
            ACDK_LOGE("State error ACDK_MHAL_CAPTURE");
            break;
        }
        break;
    case ACDK_MHAL_ERROR:
        switch(newState)
        {
        case ACDK_MHAL_IDLE:
        case ACDK_MHAL_UNINIT:
            break;
        default:
            //ACDK_ASSERT(0, "State error ACDK_MHAL_ERROR");
            ACDK_LOGE("State error ACDK_MHAL_ERROR");
            break;
        }
        break;
    default:
        //ACDK_ASSERT(0, "Unknown state");
        ACDK_LOGE("Unknown state");
        break;
    }

ACDK_MHAL_SET_STATE_EXIT:

    mAcdkMhalState = newState;

    ACDK_LOGD("X, state(0x%04x)", mAcdkMhalState);
}

/*******************************************************************************
* acdkMhalGetState
* brif : get state of acdk mhal
*******************************************************************************/
acdkMhalState_e AcdkMhalVC::acdkMhalGetState()
{
    Mutex::Autolock _l(mLock);
    return mAcdkMhalState;
}

/*******************************************************************************
* acdkMhalReadyForCap
* brif : get status of mReadyForCap falg
*******************************************************************************/
MBOOL AcdkMhalVC::acdkMhalReadyForCap()
{
    return mReadyForCap;
}

/*******************************************************************************
* acdkMhalProcLoop
* brif : preview and capture thread executing function
*******************************************************************************/
static MVOID *acdkMhalProcLoop(MVOID *arg)
{
    MINT32 const policy = SCHED_OTHER;
    MINT32 const priority = NICE_CAMERA_PASS1;
    struct sched_param sched_p;
    acdkMhalState_e eState;
    (void)arg;

    ::prctl(PR_SET_NAME,"acdkMhalProcLoop",0,0,0);

    ::sched_getparam(0, &sched_p);
    sched_p.sched_priority = priority;
    ::sched_setscheduler(0, policy, &sched_p);
    ::sched_getparam(0, &sched_p);

    ACDK_LOGD("policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d),tid=%d", policy,
                                                                                             ::sched_getscheduler(0),
                                                                                             priority,
                                                                                             sched_p.sched_priority,
                                                                                             ::gettid());

    #if (PTHREAD_CTRL_M == 0)
    //detach thread => cannot be join
    ::pthread_detach(::pthread_self());
    #endif

    eState = g_pAcdkMHalVCObj->acdkMhalGetState();

    while(eState != ACDK_MHAL_UNINIT)
    {
        ACDK_LOGD("Wait semMainHigh");
        ::sem_wait(&g_SemMainHigh); // wait here until someone use sem_post() to wake this semaphore up
        ACDK_LOGD("Got semMainHigh");

        eState = g_pAcdkMHalVCObj->acdkMhalGetState();

        switch(eState)
        {
        case ACDK_MHAL_PREVIEW:
            g_pAcdkMHalVCObj->acdkMhalPreviewProc();
            ::sem_post(&g_SemMainHighBack);
            break;
        case ACDK_MHAL_CAPTURE:
            g_pAcdkMHalVCObj->acdkMhalCaptureProc();
            break;
        case ACDK_MHAL_UNINIT:
            break;
        default:
            ACDK_LOGD("T.B.D");
            break;
        }

        eState = g_pAcdkMHalVCObj->acdkMhalGetState();
    }


    ::sem_post(&g_SemMainHighEnd);

    ACDK_LOGD("-");

    return NULL;
}

/*******************************************************************************
* destroyInstanc
* brif : destroy AcdkMhalVC object
*******************************************************************************/
void AcdkMhalVC::destroyInstance()
{
    g_pAcdkMHalVCObj = NULL;
    delete this;
}

/*******************************************************************************
* acdkMhalInit
* brif : initialize camera
*******************************************************************************/
MINT32 AcdkMhalVC::acdkMhalInit()
{
    MINT32 err = ACDK_RETURN_NO_ERROR, r = 0;
    char value[PROPERTY_VALUE_MAX] = {'\0'};

    ACDK_LOGD("+");

    mAcdkMhalState = ACDK_MHAL_INIT;    //set state to Init state

    property_get("vendor.camera.acdk.debug", value, "0");
    g_acdkMhalVCDebug = atoi(value);

    ACDK_LOGD("g_acdkMhalVCDebug(%d)",g_acdkMhalVCDebug);

    //====== Init Thread for Preview and Capture ======

    // Init semphore
    ::sem_init(&g_SemMainHigh, 0, 0);
    ::sem_init(&g_SemMainHighBack, 0, 0);
    ::sem_init(&g_SemMainHighEnd, 0, 0);
    ::sem_init(&g_SemPurePreviewCb, 0, 0);

    // Create main thread for preview and capture
    #if (PTHREAD_CTRL_M == 0)
    {
        pthread_attr_t const attr = {0, NULL, 1024 * 1024, 4096, SCHED_OTHER, NICE_CAMERA_PASS1};
        pthread_create(&g_threadMainHigh, &attr, acdkMhalProcLoop, NULL);
    }
    #else
    {
        pthread_attr_t attr;
        if ((r = pthread_attr_init(&attr)) != 0) {
            ACDK_LOGE("pthread_attr_init fail: %d", r);
            return MFALSE;
        }
        if ((r = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)) != 0) {
            ACDK_LOGE("pthread_attr_setdetachstate fail: %d", r);
            return MFALSE;
        }
        if ((r = pthread_create(&g_threadMainHigh, &attr, acdkMhalProcLoop, NULL)) != 0) {
            ACDK_LOGE("pthread_create fail: %d", r);
            return MFALSE;
        }
        if ((r = pthread_attr_destroy(&attr)) != 0) {
            ACDK_LOGE("pthread_attr_destroy fail: %d", r);
            return MFALSE;
        }
    }
    #endif

    acdkMhalSetState(ACDK_MHAL_IDLE);

    ACDK_LOGD("-");
    return err;
}

/*******************************************************************************
* acdkMhalUninit
* brif : Uninitialize camera
*******************************************************************************/
MINT32 AcdkMhalVC::acdkMhalUninit()
{
    ACDK_LOGD("+");

    //====== Local Variable Declaration ======

    MINT32 err = ACDK_RETURN_NO_ERROR;
    acdkMhalState_e eState;

    //====== Uninitialization ======

    // Check it is in the idle mode
    // If it is not, has to wait until idle
    eState = acdkMhalGetState();

    ACDK_LOGD("eState(0x%x)",eState);

    if(eState != ACDK_MHAL_NONE)
    {
        if((eState != ACDK_MHAL_IDLE) && (eState != ACDK_MHAL_ERROR))
        {
            ACDK_LOGD("Camera is not in the idle state");
            if(eState & ACDK_MHAL_PREVIEW_MASK)
            {
                err = acdkMhalPreviewStop();

                if(err != ACDK_RETURN_NO_ERROR)
                {
                    ACDK_LOGE("acdkMhalPreviewStop fail(0x%x)",err);
                }
            }
            else if(eState & ACDK_MHAL_CAPTURE_MASK)
            {
                err = acdkMhalCaptureStop();

                if(err != ACDK_RETURN_NO_ERROR)
                {
                    ACDK_LOGE("acdkMhalCaptureStop fail(0x%x)",err);
                }
            }

            // Polling until idle
            while(eState != ACDK_MHAL_IDLE)
            {
                // Wait 10 ms per time
                usleep(10000);
                eState = acdkMhalGetState();
            }
            ACDK_LOGD("Now camera is in the idle state");
        }

        acdkMhalSetState(ACDK_MHAL_UNINIT);

        ACDK_LOGD("post g_SemMainHigh");
        ::sem_post(&g_SemMainHigh);

        ACDK_LOGD("wait for g_SemMainHighEnd");

        ::sem_wait(&g_SemMainHighEnd);

        ACDK_LOGD("got g_SemMainHighEnd");
    }
    else
    {
        acdkMhalSetState(ACDK_MHAL_UNINIT);
    }

    ACDK_LOGD("-");
    return err;
}

/*******************************************************************************
* acdkMhalCBHandle
* brif : callback handler
*******************************************************************************/
MVOID AcdkMhalVC::acdkMhalCBHandle(MUINT32 a_type, MUINT8* a_addr1, MUINT8* a_addr2, MUINT32 const a_dataSize)
{
    ACDK_LOGD_DYN(g_acdkMhalVCDebug,"+");

    if(!g_acdkMhalObserver)
    {
        ACDK_LOGE("callback is NULL");
    }

    // Callback to upper layer
    g_acdkMhalObserver.notify(a_type, a_addr1, a_addr2, a_dataSize);
}

/*******************************************************************************
* acdkMhal3ASetParm
* brif : set 3A parameter
*******************************************************************************/
MINT32 AcdkMhalVC::acdkMhal3ASetParam(MINT32 devID, MUINT8 IsFactory)
{
    (void)IsFactory;

    ACDK_LOGD("devID(%d)", devID);

    return ACDK_RETURN_NO_ERROR;
}

/*******************************************************************************
* acdkMhalPreviewStart
* brif : Start preview
*******************************************************************************/
MINT32 AcdkMhalVC::acdkMhalPreviewStart(MVOID *a_pBuffIn)
{
    MINT32      err = ACDK_RETURN_NO_ERROR;
    MUINT32     bufStridesInBytes[3] = {0, 0, 0};
    MINT32      bufBoundaryInBytes[3] = {0, 0, 0};
    NormalPipe_QueryInfo    imgoQry;
    vector<portInfo>        vPortInfo;
    vector<IHalSensor::ConfigParam> vSensorCfg;
    MUINT32 mSensorFormatOrder = 0;

    ACDK_LOGD("+");

    ACDK_ASSERT(acdkMhalGetState() == ACDK_MHAL_IDLE, "[acdkMhalPreviewStart] Camera State is not IDLE");

    memcpy(&mAcdkMhalPrvParam, a_pBuffIn, sizeof(acdkMhalPrvParam_t));
    mSensorFormatOrder = mAcdkMhalPrvParam.sensorColorOrder;

    for(int i = 0; i < OVERLAY_BUFFER_CNT; ++i)
    {
        ACDK_LOGD("imgImemBuf  : virAddr[%d]=0x%zx",i,mAcdkMhalPrvParam.imgImemBuf[i].virtAddr);
        ACDK_LOGD("dispImemBuf : virAddr[%d]=0x%zx",i,mAcdkMhalPrvParam.dispImemBuf[i].virtAddr);
    }

    /* PASS1 */
    m_pINormPipe = (INormalPipe*)createDefaultNormalPipe(mAcdkMhalPrvParam.sensorIndex, THIS_NAME);

    if (MTRUE != m_pINormPipe->init()) {
        err = ACDK_RETURN_API_FAIL;
        ACDK_LOGE("m_pINormPipe init() fail");
        goto acdkMhalPreviewStart_Exit;
    }

    mu4SensorDelay = mAcdkMhalPrvParam.mu4SensorDelay; //Sensor Delay Frame

    getNormalPipeModule()->query(PORT_IMGO.index, ENPipeQueryCmd_X_PIX|ENPipeQueryCmd_STRIDE_BYTE,
                        (EImageFormat)mAcdkMhalPrvParam.sensorFormat, mAcdkMhalPrvParam.sensorWidth, imgoQry);

    if (imgoQry.x_pix != mAcdkMhalPrvParam.sensorWidth) {
        ACDK_LOGE("query x_pix ng: %d : %d", imgoQry.x_pix, mAcdkMhalPrvParam.sensorWidth);
    }

    vSensorCfg.push_back(mAcdkMhalPrvParam.sensorConfigParam[0]);

    do {
        portInfo OutPort(
                    PORT_IMGO,
                    (EImageFormat)mAcdkMhalPrvParam.sensorFormat,
                    MSize(mAcdkMhalPrvParam.sensorWidth, mAcdkMhalPrvParam.sensorHeight), //dst size
                    MRect(mAcdkMhalPrvParam.sensorWidth, mAcdkMhalPrvParam.sensorHeight), //crop
                    imgoQry.stride_byte, 0, 0,
                    1, //pureraw
                    MTRUE); //packed
        vPortInfo.push_back(OutPort);

        ACDK_LOGD("config portID(0x%x), fmt(%u), size(%dx%d), crop(%u,%u,%u,%u)\n",
                OutPort.mPortID.index, OutPort.mFmt, OutPort.mDstSize.w, OutPort.mDstSize.h,
                OutPort.mCropRect.p.x, OutPort.mCropRect.p.y,
                OutPort.mCropRect.s.w, OutPort.mCropRect.s.h);
    } while(0);

    do {
        MUINT32 bitDepth = 10;
#ifdef ACDK_FAKE_SENSOR
        QInitParam param(1/*1: test model*/, bitDepth/*unused*/, vSensorCfg, vPortInfo);
#else
        QInitParam param(0/*2: sensor uses pattern*/, bitDepth/*unused*/, vSensorCfg, vPortInfo);
#endif

        if (MTRUE != m_pINormPipe->configPipe(param)) {
            err = ACDK_RETURN_API_FAIL;
            ACDK_LOGE("NormalPipe configPipe fail");
            goto acdkMhalPreviewStart_Exit;
        }
    } while(0);

    ACDK_LOGD("sensorCfg %dx%d, sce %d, bypass %d, con %d, hdr %d, fps %d",
            mAcdkMhalPrvParam.sensorConfigParam[0].crop.w,
            mAcdkMhalPrvParam.sensorConfigParam[0].crop.h,
            mAcdkMhalPrvParam.sensorConfigParam[0].scenarioId,
            mAcdkMhalPrvParam.sensorConfigParam[0].isBypassScenario,
            mAcdkMhalPrvParam.sensorConfigParam[0].isContinuous,
            mAcdkMhalPrvParam.sensorConfigParam[0].HDRMode,
            mAcdkMhalPrvParam.sensorConfigParam[0].framerate);

    /* enque PASS1 buffer */
    bufStridesInBytes[0] = imgoQry.stride_byte;

    for (MINT32 i = 0; i < OVERLAY_BUFFER_CNT; ++i) {
        QBufInfo        p1enqQ;
        bufStridesInBytes[0] = imgoQry.stride_byte;
        do { //IMGO
            MSize imgoDstSize(mAcdkMhalPrvParam.sensorWidth, mAcdkMhalPrvParam.sensorHeight);
            PortBufInfo_v1 portBufInfo(mAcdkMhalPrvParam.imgImemBuf[i].memID, mAcdkMhalPrvParam.imgImemBuf[i].virtAddr,
                            0, mAcdkMhalPrvParam.imgImemBuf[i].bufSecu, mAcdkMhalPrvParam.imgImemBuf[i].bufCohe);
            IImageBufferAllocator::ImgParam imgParam(mAcdkMhalPrvParam.sensorFormat, imgoDstSize,
                                                bufStridesInBytes, bufBoundaryInBytes, 1);
            IImageBuffer*   pIBuf;

            sp<ImageBufferHeap> pHeap = ImageBufferHeap::create(THIS_NAME, imgParam ,portBufInfo, MTRUE);
            if (pHeap != NULL) {
                pHeap->setColorArrangement((MINT32)mSensorFormatOrder);
            }

            pIBuf = pHeap->createImageBuffer();
            pIBuf->incStrong(pIBuf);
            pIBuf->lockBuf(THIS_NAME, eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);

            mImgoRtbc.mvBuffer.push_back((MUINTPTR)pIBuf);

            if (i < P1_STATIC_ENQ_CNT) {
                p1enqQ.mvOut.push_back(BufInfo(PORT_IMGO, pIBuf, imgoDstSize, MRect(imgoDstSize.w, imgoDstSize.h),\
                                        mImgoRtbc.mEnqCount, 0, EPipe_PURE_RAW, 0xFFFF));
                mImgoRtbc.mEnqCount++;

                ACDK_LOGD("p1_enq: dma:x%x V:%p P:x%zx", PORT_IMGO.index, (void*)pIBuf->getBufVA(0), pIBuf->getBufPA(0));
            }
        } while(0);

        if (i < P1_STATIC_ENQ_CNT) {
            if (MFALSE == m_pINormPipe->enque(p1enqQ)) {
                err = ACDK_RETURN_API_FAIL;
                ACDK_LOGE("p1 enque fail");
                goto acdkMhalPreviewStart_Exit;
            }
        }
    }

    /* PASS2 */
    #ifndef ACDK_BYPASS_P2
    m_pINormStream = INormalStream::createInstance(mAcdkMhalPrvParam.sensorIndex);

    if (MTRUE != m_pINormStream->init(THIS_NAME)) {
        err = ACDK_RETURN_API_FAIL;
        ACDK_LOGE("NormalStream init() fail");
        goto acdkMhalPreviewStart_Exit;
    }
    #endif

    if (eImgFmt_YUY2 == (EImageFormat)mAcdkMhalPrvParam.frmParam.frmFormat) {
        bufStridesInBytes[0] = mAcdkMhalPrvParam.frmParam.w * 2;
        bufStridesInBytes[1] = 0;//mAcdkMhalPrvParam.frmParam.stride[1];
        bufStridesInBytes[2] = 0;//mAcdkMhalPrvParam.frmParam.stride[2];
    }
    do {
        PortBufInfo_v1 portBufInfo(mAcdkMhalPrvParam.dispImemBuf[0].memID, mAcdkMhalPrvParam.dispImemBuf[0].virtAddr,
                        0, mAcdkMhalPrvParam.dispImemBuf[0].bufSecu, mAcdkMhalPrvParam.dispImemBuf[0].bufCohe);
        IImageBufferAllocator::ImgParam imgParam(mAcdkMhalPrvParam.frmParam.frmFormat,
                                            MSize(mAcdkMhalPrvParam.frmParam.w, mAcdkMhalPrvParam.frmParam.h),
                                            bufStridesInBytes, bufBoundaryInBytes, 1);
        IImageBuffer* pIBuf;

        sp<ImageBufferHeap> pHeap = ImageBufferHeap::create(THIS_NAME, imgParam ,portBufInfo, MTRUE);

        pIBuf = pHeap->createImageBuffer();
        pIBuf->incStrong(pIBuf);
        pIBuf->lockBuf(THIS_NAME, eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);

        mWrotoBufCtl.mvBuffer.push_back((MUINTPTR)pIBuf);
    } while(0);

    if (mAcdkMhalPrvParam.testPatternEn) {
        MINT32 u32Enable = 1;
        err = mAcdkMhalPrvParam.pHalSensorObj->sendCommand(mAcdkMhalPrvParam.sensorID,
                                           SENSOR_CMD_SET_TEST_PATTERN_OUTPUT,
                                           (MUINTPTR)&u32Enable,
                                           0,
                                           0);
    }

    g_acdkMhalObserver = mAcdkMhalPrvParam.acdkMainObserver;

    acdkMhalSetState(ACDK_MHAL_PREVIEW);

    ::sem_post(&g_SemMainHigh);

acdkMhalPreviewStart_Exit:

    if (err != ACDK_RETURN_NO_ERROR) {
        if(m_pINormPipe != NULL) {
            ACDK_LOGE("release m_pINormPipe");
            m_pINormPipe->uninit();
            m_pINormPipe->destroyInstance(THIS_NAME);
            m_pINormPipe = NULL;
        }

        #ifndef ACDK_BYPASS_P2
        if (m_pINormStream != NULL) {
            ACDK_LOGE("release m_pINormStream");
            m_pINormStream->uninit(THIS_NAME);
            m_pINormStream->destroyInstance();
            m_pINormStream = NULL;
        }
        #endif

        for (MUINT32 i = 0; i < mImgoRtbc.mvBuffer.size(); i++) {
            if (mImgoRtbc.mvBuffer.at(i)) {
                ((IImageBuffer*)mImgoRtbc.mvBuffer.at(i))->unlockBuf(THIS_NAME);
            }
        }
        for (MUINT32 i = 0; i < mWrotoBufCtl.mvBuffer.size(); i++) {
            if (mWrotoBufCtl.mvBuffer.at(i)) {
                ((IImageBuffer*)mWrotoBufCtl.mvBuffer.at(i))->unlockBuf(THIS_NAME);
            }
        }
    }

    ACDK_LOGD("-");
    return err;
}

/*******************************************************************************
* acdkMhalPreviewStop
* brif : stop preview
*******************************************************************************/
MINT32 AcdkMhalVC::acdkMhalPreviewStop()
{
    ACDK_LOGD("+");

    //====== Local Variable Declaration ======

    MINT32 err = ACDK_RETURN_NO_ERROR;
    acdkMhalState_e state = acdkMhalGetState();

    //====== Check State ======

    //check AcdkMhal state
    ACDK_LOGD("state(%d)", state);
    if(state == ACDK_MHAL_IDLE)
    {
        ACDK_LOGD("is in IDLE state");
        return err;
    }
    else if(state != ACDK_MHAL_PREVIEW_STOP)
    {
        if(state & ACDK_MHAL_PREVIEW_MASK)
        {
            acdkMhalSetState(ACDK_MHAL_PREVIEW_STOP);
        }
        else if(state == ACDK_MHAL_PRE_CAPTURE)
        {
            // In preCapture, has to wait 3A ready flag
            ACDK_LOGD("it is ACDK_MHAL_PRE_CAPTURE state");
            acdkMhalSetState(ACDK_MHAL_PREVIEW_STOP);
        }
        else if(state == ACDK_MHAL_CAPTURE)
        {
            // It is in capture flow now, preview has been already stopped
            ACDK_LOGD("it is ACDK_MHAL_CAPTURE state");
            state = acdkMhalGetState();
            while(state == ACDK_MHAL_CAPTURE)
            {
                usleep(20);
                state = acdkMhalGetState();
            }

            acdkMhalSetState(ACDK_MHAL_IDLE);
        }
        else
        {
            // Unknown
            ACDK_LOGE("un know state(%d)", state);
        }
    }

    ACDK_LOGD("Wait g_SemMainHighBack");

    ::sem_wait(&g_SemMainHighBack);

    ACDK_LOGD("Got g_SemMainHighBack");


    if(MTRUE != m_pINormPipe->stop())
    {
        err = ACDK_RETURN_API_FAIL;
        ACDK_LOGE("m_pICamIOPipe->stop() fail");
    }

    //====== Uninit CamIO & PostProcPipe ======

    if(MTRUE != m_pINormPipe->uninit())
    {
        err = ACDK_RETURN_API_FAIL;
        ACDK_LOGE("m_pICamIOPipe->uninit() fail");
    }

    ACDK_LOGD("NormalPipe destroyInstance");
    m_pINormPipe->destroyInstance(THIS_NAME);
    m_pINormPipe = NULL;

    #ifndef ACDK_BYPASS_P2
    if(MTRUE != m_pINormStream->uninit(THIS_NAME))
    {
        err = ACDK_RETURN_API_FAIL;
        ACDK_LOGE("m_pINormStream uninit() fail");
    }

    ACDK_LOGD("NormalStream destroyInstance");
    m_pINormStream->destroyInstance();
    m_pINormStream = NULL;
    #endif

    for (MUINT32 i = 0; i < mImgoRtbc.mvBuffer.size(); i++) {
        if (mImgoRtbc.mvBuffer.at(i)) {
            ((IImageBuffer*)mImgoRtbc.mvBuffer.at(i))->unlockBuf(THIS_NAME);
        }
    }
    for (MUINT32 i = 0; i < mWrotoBufCtl.mvBuffer.size(); i++) {
        if (mWrotoBufCtl.mvBuffer.at(i)) {
            ((IImageBuffer*)mWrotoBufCtl.mvBuffer.at(i))->unlockBuf(THIS_NAME);
        }
    }

    memset(&mAcdkMhalPrvParam, 0, sizeof(acdkMhalPrvParam_t));

    acdkMhalSetState(ACDK_MHAL_IDLE);

    ACDK_LOGD("-");

    return err;
}

/*******************************************************************************
* acdkMhalPreCapture
* brif : change ACDK mhal state to preCapture
*******************************************************************************/
MINT32 AcdkMhalVC::acdkMhalPreCapture()
{
    ACDK_LOGD("+");

    MINT32 err = ACDK_RETURN_NO_ERROR;

    //====== Change State ======

    acdkMhalSetState(ACDK_MHAL_PRE_CAPTURE);

    ACDK_LOGD("-");

    return err;
}


/*******************************************************************************
* acdkMhalCaptureStart
* brif : init capture
*******************************************************************************/
MINT32 AcdkMhalVC::acdkMhalCaptureStart(MVOID *a_pBuffIn)
{
    (void)a_pBuffIn;

    ACDK_LOGD("+");

    mFocusDone = MTRUE;
    mFocusSucceed = MTRUE;

    return ACDK_RETURN_NO_ERROR;
}


/*******************************************************************************
* acdkMhalCaptureStop
* brif : stop capture
*******************************************************************************/
MINT32 AcdkMhalVC::acdkMhalCaptureStop()
{
    ACDK_LOGD("+");

    mReadyForCap = MFALSE;
    mFocusDone = MFALSE;

    ACDK_LOGD("-");
    return ACDK_RETURN_NO_ERROR;
}

/*******************************************************************************
* acdkMhalPreviewProc
* brif : handle flow control of preview
*******************************************************************************/
MINT32 AcdkMhalVC::acdkMhalPreviewProc()
{
    MUINTPTR    imgo_virAddr[OVERLAY_BUFFER_CNT] = {0}, imgo_phyAddr[OVERLAY_BUFFER_CNT] = {0};
    MUINTPTR    dispo_virAddr, dispo_phyAddr, imgo_virA, imgo_phyA;
    MUINT32     p1DeqDepth = 0;
    MUINT32     delayframecnt = 0, frameCount = 0;
    MUINT32     dumpEnable = 0, i = 0;
    MBOOL       isPreCap = MFALSE;
    QPortID     deqPorts;
    QBufInfo    dequeBufInfo;
    IImageBuffer*   imgo_ibuf = NULL;
    map<EDIPInfoEnum, MUINT32> mDipInfo;
    mDipInfo[EDIPINFO_DIPVERSION] = EDIPHWVersion_40;
#ifndef ACDK_BYPASS_P2
    MBOOL r = NSCam::NSIoPipe::NSPostProc::INormalStream::queryDIPInfo(mDipInfo);
    if (!r) {
        ACDK_LOGE("queryDIPInfo fail!");
    }
#endif

    char value[PROPERTY_VALUE_MAX] = {'\0'};

    property_get("vendor.camera.acdkdump.enable", value, "0");

    dumpEnable = atoi(value);

    ACDK_LOGD("+");

    deqPorts.mvPortId.push_back(PORT_IMGO);

    /* add wait vsync to check sensor status */

    ACDK_LOGD("Preview pass1 start");
    m_pINormPipe->start();

    while ((acdkMhalGetState() & ACDK_MHAL_PREVIEW_MASK) || (ACDK_MHAL_PRE_CAPTURE == acdkMhalGetState())) {
        #ifndef ACDK_BYPASS_P2
        MUINT32 bypass_p2 = 0;
        #else
        MUINT32 bypass_p2 = 1;
        #endif
        MUINT32 itr_imgo = 0;
        /* In PreCapture State Or Not
         * 3A need at least 3 frame to change  preview mode to capture mdoe
         */
        if (ACDK_MHAL_PRE_CAPTURE == acdkMhalGetState() && isPreCap == MFALSE && frameCount > 2) {
            ACDK_LOGD("in precapture state");

            isPreCap = MTRUE;
        }

        //====== Pass1 ======
        ACDK_LOGD("pass1 deque");
        if (MFALSE == m_pINormPipe->deque(deqPorts, dequeBufInfo, 3000)) {
            ACDK_LOGE("pass1 deque fail");
            continue;;
        }
        p1DeqDepth = dequeBufInfo.mvOut.size();

        if (0 == dequeBufInfo.mvOut.size()) {
            ACDK_LOGD("deque no buffer");
            continue;
        }
        else {
            for (i = 0; i < p1DeqDepth; ++i) {
                if (dequeBufInfo.mvOut.at(i).mPortID.index == PORT_IMGO.index) {
                    imgo_virAddr[itr_imgo] = dequeBufInfo.mvOut.at(i).mBuffer->getBufVA(0);
                    imgo_phyAddr[itr_imgo] = dequeBufInfo.mvOut.at(i).mBuffer->getBufPA(0);
                    itr_imgo++;
                }
                ACDK_LOGD("dma:x%x[%d] V(%p) P(x%zx)", dequeBufInfo.mvOut.at(i).mPortID.index,\
                    i, (void*)imgo_virAddr[i], imgo_phyAddr[i]);
            }
            mImgoRtbc.mDeqCount += itr_imgo;
            imgo_virA = imgo_virAddr[0];
            imgo_phyA = imgo_phyAddr[0];
            imgo_ibuf = dequeBufInfo.mvOut.at(0).mBuffer;
        }

        if(dumpEnable == 1 || dumpEnable == 3) {
            char    szFileName[256];
            MINT32  i4WriteCnt = 0;
            FILE*   pFp = NULL;

            ACDK_LOGD("prv VA(%p)", (MUINT8 *)imgo_virAddr[0]);

            sprintf(szFileName, "%s/acdkPrv1.raw" , MEDIA_PATH);

            pFp = fopen(szFileName, "wb");

            if (NULL == pFp ) {
                ACDK_LOGE("Can't open file to save image");
            }
            else {
                MUINT8 *pVirBufin = (MUINT8 *)imgo_virAddr[0];

                i4WriteCnt = fwrite(pVirBufin, 1, mAcdkMhalPrvParam.imgImemBuf[0].size, pFp);

                fflush(pFp);

                if (0 != fsync(fileno(pFp))) {
                    ACDK_LOGE("fync fail");
                    fclose(pFp);
                    return ACDK_RETURN_NULL_OBJ;
                }

                ACDK_LOGD("Save image: %s", szFileName);

                fclose(pFp);
            }
        }

        frameCount++;
        ACDK_LOGD("frameCount: %d", frameCount);


        /* PASS2 Process */
        #ifndef ACDK_BYPASS_P2
        if (!bypass_p2) {
            QParams     enqueParams; //pass2
            QParams     dequeParams;
            FrameParams frameParams;
            Input       src;
            Output      dst;

            /* Config pass2 every time & enque buffer at the same time */

            /*

                Implement PASS2 flow here
                    put deuqed buffer to : dispo_virAddr, dispo_phyAddr

             */
            frameParams.mStreamTag = NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Normal;

            MCrpRsInfo crop, crop2, crop3;

            mRawPostProcSupport = isPostProcRawSupported();
            if (mRawPostProcSupport == MFALSE) {
                ImageDescRawType rawType = eIMAGE_DESC_RAW_TYPE_PROCESSED;
                imgo_ibuf->setImgDesc(eIMAGE_DESC_ID_RAW_TYPE, rawType, MTRUE);
            }
            src.mPortID = PORT_IMGI;
            src.mBuffer = imgo_ibuf;
            src.mPortID.group = 0;
            frameParams.mvIn.push_back(src);

            crop.mGroupID   = 1;//img2o, p2 internal
            crop.mCropRect.p_fractional.x   = 0;
            crop.mCropRect.p_fractional.y   = 0;
            crop.mCropRect.p_integral.x     = 0;
            crop.mCropRect.p_integral.y     = 0;
            crop.mCropRect.s.w              = mAcdkMhalPrvParam.sensorWidth;
            crop.mCropRect.s.h              = mAcdkMhalPrvParam.sensorHeight;
            crop.mResizeDst.w               = mAcdkMhalPrvParam.sensorWidth;
            crop.mResizeDst.h               = mAcdkMhalPrvParam.sensorHeight;
            crop2.mGroupID  = 2;//wdmao, mdp
            crop2.mCropRect.p_fractional.x  = 0;
            crop2.mCropRect.p_fractional.y  = 0;
            crop2.mCropRect.p_integral.x    = 0;
            crop2.mCropRect.p_integral.y    = 0;
            crop2.mCropRect.s.w             = mAcdkMhalPrvParam.sensorWidth;
            crop2.mCropRect.s.h             = mAcdkMhalPrvParam.sensorHeight;
            crop2.mResizeDst.w              = mAcdkMhalPrvParam.sensorWidth;
            crop2.mResizeDst.h              = mAcdkMhalPrvParam.sensorHeight;
            crop3.mGroupID  = 3;//wroto, mdp. _BEFORE_ rotate
            crop3.mCropRect.p_fractional.x  = 0;
            crop3.mCropRect.p_fractional.y  = 0;
            crop3.mCropRect.p_integral.x    = 0;
            crop3.mCropRect.p_integral.y    = 0;
            crop3.mCropRect.s.w             = mAcdkMhalPrvParam.sensorWidth;
            crop3.mCropRect.s.h             = mAcdkMhalPrvParam.sensorHeight;
            switch (mAcdkMhalPrvParam.frmParam.orientation) {
            case NSCam::eTransform_ROT_90:
            case NSCam::eTransform_ROT_270:
                crop3.mResizeDst.w          = mAcdkMhalPrvParam.frmParam.h;
                crop3.mResizeDst.h          = mAcdkMhalPrvParam.frmParam.w;
                break;
            case 0:
            case NSCam::eTransform_ROT_180:
            default:
                crop3.mResizeDst.w          = mAcdkMhalPrvParam.frmParam.w;
                crop3.mResizeDst.h          = mAcdkMhalPrvParam.frmParam.h;
                break;
            }
            frameParams.mvCropRsInfo.push_back(crop);
            frameParams.mvCropRsInfo.push_back(crop2);
            frameParams.mvCropRsInfo.push_back(crop3);

            dst.mPortID = PORT_WROTO;
            dst.mBuffer = (IImageBuffer*)mWrotoBufCtl.mvBuffer.at(0);
            dst.mPortID.group = 0;
            #if 1//TBD TODO
            switch (mAcdkMhalPrvParam.frmParam.orientation) {
            case NSCam::eTransform_ROT_90:
                dst.mTransform = eTransform_ROT_90;
                break;
            case NSCam::eTransform_ROT_180:
                dst.mTransform = eTransform_ROT_180;
                break;
            case NSCam::eTransform_ROT_270:
                dst.mTransform = eTransform_ROT_270;
                break;
            case 0:
            default:
                break;
            }
            #endif
            frameParams.mvOut.push_back(dst);
            enqueParams.mvFrameParams.push_back(frameParams);
            if (mDipInfo[EDIPINFO_DIPVERSION] == EDIPHWVersion_60) {
                enqueParams.mpfnCallback = VCPreviewCb;
            } else {
                enqueParams.mpfnCallback = NULL;
            }

            ACDK_LOGD("pass2 enq P(x%zx) %dx%d",dst.mBuffer->getBufPA(0), crop3.mResizeDst.w, crop3.mResizeDst.h);
            if (MFALSE == m_pINormStream->enque(enqueParams)) {
                bypass_p2 = 1;
                ACDK_LOGE("pass2 enque fail");
            }
            else {
                ACDK_LOGD("pass2 deq");
                if (mDipInfo[EDIPINFO_DIPVERSION] == EDIPHWVersion_60) {
                    ::sem_wait(&g_SemPurePreviewCb);
                } else {
                    if (MFALSE == m_pINormStream->deque(dequeParams)) {
                        bypass_p2 = 1;
                        ACDK_LOGE("pass2 deque fail");
                    }
                }
            }

            if (!bypass_p2) {
                if (mDipInfo[EDIPINFO_DIPVERSION] == EDIPHWVersion_60) {
                    dispo_virAddr = enqueParams.mvFrameParams[0].mvOut[0].mBuffer->getBufVA(0);
                    dispo_phyAddr = enqueParams.mvFrameParams[0].mvOut[0].mBuffer->getBufPA(0);
                } else {
                    dispo_virAddr = dequeParams.mvFrameParams[0].mvOut[0].mBuffer->getBufVA(0);
                    dispo_phyAddr = dequeParams.mvFrameParams[0].mvOut[0].mBuffer->getBufPA(0);
                }
                ACDK_LOGD_DYN(g_acdkMhalVCDebug,"pass2 done");
            }
        }
	#endif
        if (bypass_p2) {
            IImageBuffer *pIbuf = (IImageBuffer*)(mWrotoBufCtl.mvBuffer.at(0));
            dispo_virAddr = pIbuf->getBufVA(0);
            dispo_phyAddr = pIbuf->getBufPA(0);

            memset((MUINT8*)(pIbuf->getBufVA(0)), 0xFF, mAcdkMhalPrvParam.dispImemBuf[0].size);

            ACDK_LOGD("pass2 bypass, fill 0xFF size %d", mAcdkMhalPrvParam.dispImemBuf[0].size);
        }

        ACDK_LOGD_DYN(g_acdkMhalVCDebug, "dispo_virAddr(0x%p),dispo_phyAddr(0x%p)", (MUINT8 *)dispo_virAddr,(MUINT8 *)dispo_phyAddr);

        if (delayframecnt >= mu4SensorDelay) {
           acdkMhalCBHandle(ACDK_CB_PREVIEW, (MUINT8 *)dispo_virAddr, (MUINT8 *)imgo_virA);
        }
        else {
           delayframecnt++;
        }

        /*
         *  PASS1 enque buffer, could be moved to another enque thread
         */
        do {
            MUINT32     sof_idx = 0;
            QBufInfo    enqQ;

            #if 0 //enable when having enque thread
            if ((mImgoRtbc.mEnqCount + p1DeqDepth) > (mImgoRtbc.mDeqCount + mImgoRtbc.mvBuffer.size())) {
                ACDK_LOGD("No empty buffer for enque, wait");
                do {
                    ACDK_LOGD("    enq/deq : %d/%d", _this->mEnqCount, _this->mDeqCount);
                    usleep(50000); //sleep 100ms to wait deque
                } while ((mImgoRtbc.mEnqCount + p1DeqDepth) > (mImgoRtbc.mDeqCount + mImgoRtbc.mvBuffer.size()));
            }

            m_pINormPipe->wait(EPipeSignal_SOF, EPipeSignal_ClearWait, userKey, 7788);
            #endif

            m_pINormPipe->sendCommand(ENPipeCmd_GET_CUR_SOF_IDX, (MINTPTR)&sof_idx, 0, 0);

            if (1) { //IMGO
                MUINT32         enqBufIdx = mImgoRtbc.mEnqCount % mImgoRtbc.mvBuffer.size();
                IImageBuffer*   pIBuf = (IImageBuffer*)mImgoRtbc.mvBuffer.at(enqBufIdx);
                MSize           _crop(mAcdkMhalPrvParam.sensorWidth, mAcdkMhalPrvParam.sensorHeight);

                enqQ.mvOut.push_back(BufInfo(PORT_IMGO, pIBuf, _crop, MRect(_crop.w, _crop.h),\
                                        mImgoRtbc.mEnqCount, sof_idx, EPipe_PURE_RAW, 0xFFFF));
            }

            if (m_pINormPipe->enque(enqQ)) {
                mImgoRtbc.mEnqCount++;
            }
        } while(0);

        /*
         *  Ready for Capture Or Not
         */

        if (isPreCap == MTRUE) {
            mReadyForCap = MTRUE;
        }
    }

    ACDK_LOGD("-");

    return ACDK_RETURN_NO_ERROR;
}


/*******************************************************************************
* acdkMhalCaptureProc
* brif : handle flow control of capture
*******************************************************************************/
MINT32 AcdkMhalVC::acdkMhalCaptureProc()
{
    return ACDK_RETURN_NO_ERROR;
}

/*******************************************************************************
*
*******************************************************************************/
MUINT32 AcdkMhalVC::acdkMhalGetShutTime()
{
    return ACDK_RETURN_NO_ERROR;
}

/*******************************************************************************
*
*******************************************************************************/
MVOID AcdkMhalVC::acdkMhalSetShutTime(MUINT32 a_time)
{
    (void)a_time;
}

/*******************************************************************************
*
*******************************************************************************/
MUINT32 AcdkMhalVC::acdkMhalGetAFInfo()
{
    MUINT32 u32AFInfo;

    ACDK_LOGD("acdkMhalGetAFInfo");
    u32AFInfo = g_pAcdkMHalVCObj->mFocusSucceed;

    return u32AFInfo;
}
/*******************************************************************************
* doNotifyCb
*******************************************************************************/
void AcdkMhalVC::doNotifyCb(int32_t _msgType,
                                 int32_t _ext1,
                                 int32_t _ext2,
                                 int32_t _ext3)
{
    (void)_ext2; (void)_ext3;
    ACDK_LOGD("_msgType(%d),_ext1(%d)",_msgType,_ext1);
}

/*******************************************************************************
* doDataCb
*******************************************************************************/
void AcdkMhalVC::doDataCb(int32_t _msgType,
                               void *_data,
                               uint32_t _size)
{
    (void)_msgType; (void)_data; (void)_size;
}

/*******************************************************************************
* PreviewCb
*******************************************************************************/
MVOID VCPreviewCb(QParams& rParams)
{
    _unused(rParams);
    ::sem_post(&g_SemPurePreviewCb);
}

