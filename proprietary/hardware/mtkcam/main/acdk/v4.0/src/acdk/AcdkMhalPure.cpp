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

//! \file  AcdkMhalPure.cpp

#define LOG_TAG "AcdkMhalPure"

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
#include "AcdkMhalPure.h"

using namespace NSAcdkMhal;

#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/imgbuf/ImageBufferHeap.h>
#include <mtkcam/def/PriorityDefs.h>

using namespace android;
using namespace std;


#ifdef ACDK_FAKE_SENSOR
#define THIS_NAME           "iopipeUseTM"
#else
#define THIS_NAME           "AcdkMhalPure"
#endif

#define MEDIA_PATH          "/data/vendor/"

#define PTHREAD_CTRL_M      (1)
#define P1_STATIC_ENQ_CNT   (2)



using namespace NSCam::NSIoPipe::NSPostProc;
using namespace NSCam::NSIoPipe::NSCamIOPipe;
using namespace NSCam::NSIoPipe;
using namespace NSCam;

/*******************************************************************************
* Global variable
*******************************************************************************/
static sem_t g_SemMainHigh, g_SemMainHighBack, g_SemMainHighEnd, g_SemPurePreviewCb;
static pthread_t g_threadMainHigh;
static AcdkMhalPure *g_pAcdkMHalPureObj = NULL;
static acdkObserver g_acdkMhalObserver;

static MINT32 g_acdkMhalPureDebug = 0;
MVOID PurePreviewCb(QParams& rParams);

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
    pModule->createSubModule(sensorIndex, szCallerName, selected_version, (MVOID**)&pPipe);
    return pPipe;
}


/*******************************************************************************
* AcdkMhalPure
* brif : Constructor
*******************************************************************************/
AcdkMhalPure::AcdkMhalPure()
    : mu4SensorDelay(0)
    , mImgoRtbc()
	, mRrzoRtbc()
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

    g_pAcdkMHalPureObj = this;
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
MVOID AcdkMhalPure::acdkMhalSetState(acdkMhalState_e newState)
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
acdkMhalState_e AcdkMhalPure::acdkMhalGetState()
{
    Mutex::Autolock _l(mLock);
    return mAcdkMhalState;
}

/*******************************************************************************
* acdkMhalReadyForCap
* brif : get status of mReadyForCap falg
*******************************************************************************/
MBOOL AcdkMhalPure::acdkMhalReadyForCap()
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

    eState = g_pAcdkMHalPureObj->acdkMhalGetState();

    while(eState != ACDK_MHAL_UNINIT)
    {
        ACDK_LOGD("Wait semMainHigh");
        ::sem_wait(&g_SemMainHigh); // wait here until someone use sem_post() to wake this semaphore up
        ACDK_LOGD("Got semMainHigh");

        eState = g_pAcdkMHalPureObj->acdkMhalGetState();

        switch(eState)
        {
        case ACDK_MHAL_PREVIEW:
            g_pAcdkMHalPureObj->acdkMhalPreviewProc();
            ::sem_post(&g_SemMainHighBack);
            break;
        case ACDK_MHAL_CAPTURE:
            g_pAcdkMHalPureObj->acdkMhalCaptureProc();
            break;
        case ACDK_MHAL_UNINIT:
            break;
        default:
            ACDK_LOGD("T.B.D");
            break;
        }

        eState = g_pAcdkMHalPureObj->acdkMhalGetState();
    }


    ::sem_post(&g_SemMainHighEnd);

    ACDK_LOGD("-");

    return NULL;
}

/*******************************************************************************
* destroyInstanc
* brif : destroy AcdkMhalPure object
*******************************************************************************/
void AcdkMhalPure::destroyInstance()
{
    g_pAcdkMHalPureObj = NULL;
    delete this;
}

/*******************************************************************************
* acdkMhalInit
* brif : initialize camera
*******************************************************************************/
MINT32 AcdkMhalPure::acdkMhalInit()
{
    MINT32 err = ACDK_RETURN_NO_ERROR, r = 0;
    char value[PROPERTY_VALUE_MAX] = {'\0'};

    ACDK_LOGD("+");

    mAcdkMhalState = ACDK_MHAL_INIT;    //set state to Init state

    property_get("vendor.camera.acdk.debug", value, "0");
    g_acdkMhalPureDebug = atoi(value);

    ACDK_LOGD("g_acdkMhalPureDebug(%d)",g_acdkMhalPureDebug);

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
MINT32 AcdkMhalPure::acdkMhalUninit()
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
MVOID AcdkMhalPure::acdkMhalCBHandle(MUINT32 a_type, MUINT8* a_addr1, MUINT8* a_addr2, MUINT32 const a_dataSize)
{
    ACDK_LOGD_DYN(g_acdkMhalPureDebug,"+");

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
MINT32 AcdkMhalPure::acdkMhal3ASetParam(MINT32 devID, MUINT8 IsFactory)
{
    (void)IsFactory;

    ACDK_LOGD("devID(%d)", devID);

    return ACDK_RETURN_NO_ERROR;
}

/*******************************************************************************
* acdkMhalPreviewStart
* brif : Start preview
*******************************************************************************/
MINT32 AcdkMhalPure::acdkMhalPreviewStart(MVOID *a_pBuffIn)
{
    MINT32      err = ACDK_RETURN_NO_ERROR;
    MUINT32     bufStridesInBytes[3] = {0, 0, 0};
    MINT32      bufBoundaryInBytes[3] = {0, 0, 0};
    NormalPipe_QueryInfo    imgoQry, rrzoQry;
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

    //(mAcdkMhalPrvParam.sensorType == SENSOR_TYPE_YUV)? 1 : 0; //sensorType: 1-RAW,2-YUV


    /* PASS1 */
    m_pINormPipe = (INormalPipe*)createDefaultNormalPipe(mAcdkMhalPrvParam.sensorIndex, THIS_NAME);

    if (MTRUE != m_pINormPipe->init()) {
        err = ACDK_RETURN_API_FAIL;
        ACDK_LOGE("m_pINormPipe init() fail");
        goto acdkMhalPreviewStart_Exit;
    }

    /*
    mAcdkMhalPrvParam.sensorType;   // SENSOR_TYPE_RAW, SENSOR_TYPE_YUV, SENSOR_TYPE_JPG
    mAcdkMhalPrvParam.sensorFormat; // eImgFmt_XXX
    mAcdkMhalPrvParam.sensorColorOrder;
    mAcdkMhalPrvParam.sensorWidth;  // Image Width
    mAcdkMhalPrvParam.sensorHeight; // Image Height

    if (is_yuv_sensor) {
        mAcdkMhalPrvParam.sensorStride[0]*2;
    }
    else {
        mAcdkMhalPrvParam.sensorStride[0]*10/8;    //  unit:PIXEL
    }
    mAcdkMhalPrvParam.sensorStride[1]; //  unit:PIXEL
    mAcdkMhalPrvParam.sensorStride[2]; //  unit:PIXEL

    mAcdkMhalPrvParam.imgImemBuf;
    mAcdkMhalPrvParam.rrzoImemBuf;
    mAcdkMhalPrvParam.dispImemBuf;
    */

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

    getNormalPipeModule()->query(PORT_RRZO.index, ENPipeQueryCmd_STRIDE_BYTE,
                        _MapFG((EImageFormat)mAcdkMhalPrvParam.sensorFormat),
                        mAcdkMhalPrvParam.sensorWidth, rrzoQry);
    do {
        portInfo OutPort(
                    PORT_RRZO,
                    _MapFG((EImageFormat)mAcdkMhalPrvParam.sensorFormat),
                    MSize((mAcdkMhalPrvParam.sensorWidth/2), (mAcdkMhalPrvParam.sensorHeight/2)), //dst size
                    MRect((mAcdkMhalPrvParam.sensorWidth/2), (mAcdkMhalPrvParam.sensorHeight/2)), //crop
                    rrzoQry.stride_byte, 0, 0,
                    0, //pureraw
                    MTRUE); //packed
        vPortInfo.push_back(OutPort);

        ACDK_LOGD("config portID(0x%x), fmt(%u), size(%dx%d), crop(%u,%u,%u,%u)\n",
                OutPort.mPortID.index, OutPort.mFmt, OutPort.mDstSize.w, OutPort.mDstSize.h,
                OutPort.mCropRect.p.x, OutPort.mCropRect.p.y,
                OutPort.mCropRect.s.w, OutPort.mCropRect.s.h);
    } while(0);

    do {
        MUINT32 bitDepth = 10;
        QInitParam param(0/*2: sensor uses pattern*/, bitDepth/*unused*/, vSensorCfg, vPortInfo);

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
        bufStridesInBytes[0] = rrzoQry.stride_byte;
        do { //RRZO
#if ACDK_IPBASE_ISP30_SUPPORT
            MSize rrzoDstSize(mAcdkMhalPrvParam.sensorWidth/2, 2);
#else
            MSize rrzoDstSize(mAcdkMhalPrvParam.sensorWidth/2, mAcdkMhalPrvParam.sensorHeight/2);
#endif
            PortBufInfo_v1 portBufInfo(mAcdkMhalPrvParam.rrzoImemBuf[i].memID, mAcdkMhalPrvParam.rrzoImemBuf[i].virtAddr,
                            0, mAcdkMhalPrvParam.rrzoImemBuf[i].bufSecu, mAcdkMhalPrvParam.rrzoImemBuf[i].bufCohe);
            IImageBufferAllocator::ImgParam imgParam(_MapFG((EImageFormat)mAcdkMhalPrvParam.sensorFormat),
                                                rrzoDstSize, bufStridesInBytes, bufBoundaryInBytes, 1);
            IImageBuffer*   pIBuf;

            sp<ImageBufferHeap> pHeap = ImageBufferHeap::create(THIS_NAME, imgParam ,portBufInfo, MTRUE);
            if (pHeap != NULL) {
                pHeap->setColorArrangement((MINT32)mSensorFormatOrder);
            }

            pIBuf = pHeap->createImageBuffer();
            pIBuf->incStrong(pIBuf);
            pIBuf->lockBuf(THIS_NAME, eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);

            mRrzoRtbc.mvBuffer.push_back((MUINTPTR)pIBuf);

            if (i < P1_STATIC_ENQ_CNT) {
                p1enqQ.mvOut.push_back(BufInfo(PORT_RRZO, pIBuf, rrzoDstSize, MRect(rrzoDstSize.w, rrzoDstSize.h),\
                                        mRrzoRtbc.mEnqCount, 0, 0xFFFF));
                mRrzoRtbc.mEnqCount++;

                ACDK_LOGD("p1_enq: dma:x%x V:%p P:x%zx", PORT_RRZO.index, (void*)pIBuf->getBufVA(0), pIBuf->getBufPA(0));
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

    /*
    mAcdkMhalPrvParam.frmParam.frmFormat;  // eImgFmt_XXX
    mAcdkMhalPrvParam.frmParam.orientation;    // Rotation
    mAcdkMhalPrvParam.frmParam.flip;
    mAcdkMhalPrvParam.frmParam.w;  // Dispaly Width. concept is assign memory footprint width and height
    mAcdkMhalPrvParam.frmParam.h;  // Display Height. concept is assign memory footprint width and height
    mAcdkMhalPrvParam.frmParam.stride[1];
    mAcdkMhalPrvParam.frmParam.stride[2];
    mAcdkMhalPrvParam.dispImemBuf[0].virtAddr;
    mAcdkMhalPrvParam.dispImemBuf[0].phyAddr;
    mAcdkMhalPrvParam.dispImemBuf[0].memID;
    mAcdkMhalPrvParam.dispImemBuf[0].size;  //bytes
    */

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
        for (MUINT32 i = 0; i < mRrzoRtbc.mvBuffer.size(); i++) {
            if (mRrzoRtbc.mvBuffer.at(i)) {
                ((IImageBuffer*)mRrzoRtbc.mvBuffer.at(i))->unlockBuf(THIS_NAME);
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
MINT32 AcdkMhalPure::acdkMhalPreviewStop()
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
    for (MUINT32 i = 0; i < mRrzoRtbc.mvBuffer.size(); i++) {
        if (mRrzoRtbc.mvBuffer.at(i)) {
            ((IImageBuffer*)mRrzoRtbc.mvBuffer.at(i))->unlockBuf(THIS_NAME);
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
MINT32 AcdkMhalPure::acdkMhalPreCapture()
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
MINT32 AcdkMhalPure::acdkMhalCaptureStart(MVOID *a_pBuffIn)
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
MINT32 AcdkMhalPure::acdkMhalCaptureStop()
{
    ACDK_LOGD("+");
#if 0

    //====== Local Variable Declaration ======

    MINT32 err = ACDK_RETURN_NO_ERROR;
    acdkMhalState_e state = acdkMhalGetState();

    //====== Check State ======

    //check AcdkMhal state
    if(state == ACDK_MHAL_IDLE)
    {
        ACDK_LOGD("in IDLE state");
        return err;
    }
    else if(state & ACDK_MHAL_PREVIEW_MASK)
    {
        ACDK_LOGD("in preview group state");
        return err;
    }
    else if(state & ACDK_MHAL_CAPTURE_MASK)
    {
        ACDK_LOGD("in capture group state");
        acdkMhalSetState(ACDK_MHAL_IDLE);
    }
    else
    {
        ACDK_LOGE("state:%d", state);
        ACDK_ASSERT(0, "[acdkMhalCaptureStop] state is unknow");
    }

    //====== Initialize Member Variable ======

    memset(&mAcdkMhalCapParam,0,sizeof(acdkMhalCapParam_t));

    //====== Set AcdkMhal State ======

    mReadyForCap = MFALSE;

    acdkMhalSetState(ACDK_MHAL_IDLE);

    ACDK_LOGD("X");
    return err;
#else
    mReadyForCap = MFALSE;
    mFocusDone = MFALSE;

    ACDK_LOGD("-");
    return ACDK_RETURN_NO_ERROR;
#endif
}

/*******************************************************************************
* acdkMhalPreviewProc
* brif : handle flow control of preview
*******************************************************************************/
MINT32 AcdkMhalPure::acdkMhalPreviewProc()
{
    MUINTPTR    imgo_virAddr[OVERLAY_BUFFER_CNT] = {0}, imgo_phyAddr[OVERLAY_BUFFER_CNT] = {0};
    MUINTPTR    rrzo_virAddr[OVERLAY_BUFFER_CNT] = {0}, rrzo_phyAddr[OVERLAY_BUFFER_CNT] = {0};
    MUINTPTR    dispo_virAddr, dispo_phyAddr, imgo_virA, imgo_phyA;
    MUINTPTR    rrzo_virA, rrzo_phyA;
    MUINT32     p1DeqDepth = 0;
    MUINT32     delayframecnt = 0, frameCount = 0;
    MUINT32     dumpEnable = 0, i = 0;
    MUINT32     scenarioMode = 0; /* 0: preview, 1: capture*/
    MBOOL       isPreCap = MFALSE;
    QPortID     deqPorts;
    QBufInfo    dequeBufInfo;
    IImageBuffer*   imgo_ibuf = NULL;
    IImageBuffer*   rrzo_ibuf = NULL;
    map<EDIPInfoEnum, MUINT32> mDipInfo;
    mDipInfo[EDIPINFO_DIPVERSION] = EDIPHWVersion_40;
#ifndef ACDK_BYPASS_P2
    MBOOL r = NSCam::NSIoPipe::NSPostProc::INormalStream::queryDIPInfo(mDipInfo);
    if (!r) {
        ACDK_LOGE("queryDIPInfo fail!");
    }
#endif

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    char value2[PROPERTY_VALUE_MAX] = {'\0'};

    property_get("vendor.camera.acdkdump.enable", value, "0");

    dumpEnable = atoi(value);

    ACDK_LOGD("+");

    deqPorts.mvPortId.push_back(PORT_IMGO);
    deqPorts.mvPortId.push_back(PORT_RRZO);

    /* add wait vsync to check sensor status */

    ACDK_LOGD("Preview pass1 start");
    m_pINormPipe->start();

    while ((acdkMhalGetState() & ACDK_MHAL_PREVIEW_MASK) || (ACDK_MHAL_PRE_CAPTURE == acdkMhalGetState())) {
        #ifndef ACDK_BYPASS_P2
        MUINT32 bypass_p2 = 0;
        #else
        MUINT32 bypass_p2 = 1;
        #endif
        MUINT32 itr_imgo = 0, itr_rrzo = 0;

        property_get("vendor.camera.scenario", value2, "1");
        scenarioMode = atoi(value2);
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
            for (i = 0; i < p1DeqDepth; i++) {
                if (dequeBufInfo.mvOut.at(i).mPortID.index == PORT_IMGO.index) {
                    imgo_virAddr[itr_imgo] = dequeBufInfo.mvOut.at(i).mBuffer->getBufVA(0);
                    imgo_phyAddr[itr_imgo] = dequeBufInfo.mvOut.at(i).mBuffer->getBufPA(0);
                    imgo_ibuf = dequeBufInfo.mvOut.at(i).mBuffer;
                    itr_imgo++;

                    ACDK_LOGD("dma:x%x[%d] imgoV(%p) imgoP(%p)", dequeBufInfo.mvOut.at(i).mPortID.index,
                        i, (void*)imgo_virAddr[i], imgo_phyAddr[i]);
                }
                if (dequeBufInfo.mvOut.at(i).mPortID.index == PORT_RRZO.index) {
                    rrzo_virAddr[itr_rrzo] = dequeBufInfo.mvOut.at(i).mBuffer->getBufVA(0);
                    rrzo_phyAddr[itr_rrzo] = dequeBufInfo.mvOut.at(i).mBuffer->getBufPA(0);
                    rrzo_ibuf = dequeBufInfo.mvOut.at(i).mBuffer;
                    itr_rrzo++;
                    ACDK_LOGD("dma:x%x[%d] rrzoV(%p) rrzoP(%p)", dequeBufInfo.mvOut.at(i).mPortID.index,
                        i, (void*)rrzo_virAddr[i], rrzo_phyAddr[i]);
                }
            }
            mImgoRtbc.mDeqCount += itr_imgo;
            mRrzoRtbc.mDeqCount += itr_rrzo;
            imgo_virA = imgo_virAddr[0];
            imgo_phyA = imgo_phyAddr[0];
            rrzo_virA = rrzo_virAddr[0];
            rrzo_phyA = rrzo_phyAddr[0];
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

            sprintf(szFileName, "%s/acdkPrv1_%d_%d_rrzo.raw" , MEDIA_PATH, mAcdkMhalPrvParam.sensorWidth,
                mAcdkMhalPrvParam.sensorHeight);

            pFp = fopen(szFileName, "wb");

            if (NULL == pFp) {
                ACDK_LOGE("Can't open file to save image: %s", szFileName);
            }
            else {
                MUINT8 *pVirBufin = (MUINT8 *)rrzo_virAddr[0];

                i4WriteCnt = fwrite(pVirBufin, 1, mAcdkMhalPrvParam.rrzoImemBuf[0].size, pFp);

                fflush(pFp);

                if (0 != fsync(fileno(pFp))) {
                    ACDK_LOGE("fync fail");
                    //fclose(pFp);
                    //return ACDK_RETURN_NULL_OBJ;
                }
                else {
                       ACDK_LOGD("Save image: %s", szFileName);
                }

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
            if (scenarioMode == 1) {
                src.mBuffer = imgo_ibuf;
                //ACDK_LOGD("imgo");
            } else if (scenarioMode == 0) {
                src.mBuffer = rrzo_ibuf;
                //ACDK_LOGD("rrzo");
            }
            src.mPortID.group = 0;
            frameParams.mvIn.push_back(src);

            crop.mGroupID   = 1;//img2o, p2 internal
            crop.mCropRect.p_fractional.x   = 0;
            crop.mCropRect.p_fractional.y   = 0;
            crop.mCropRect.p_integral.x     = 0;
            crop.mCropRect.p_integral.y     = 0;
            if (scenarioMode == 1) {
                crop.mCropRect.s.w              = mAcdkMhalPrvParam.sensorWidth;
                crop.mCropRect.s.h              = mAcdkMhalPrvParam.sensorHeight;
                crop.mResizeDst.w               = mAcdkMhalPrvParam.sensorWidth;
                crop.mResizeDst.h               = mAcdkMhalPrvParam.sensorHeight;
            } else if (scenarioMode == 0) {
                crop.mCropRect.s.w              = mAcdkMhalPrvParam.sensorWidth/2;
                crop.mCropRect.s.h              = mAcdkMhalPrvParam.sensorHeight/2;
                crop.mResizeDst.w               = mAcdkMhalPrvParam.sensorWidth/2;
                crop.mResizeDst.h               = mAcdkMhalPrvParam.sensorHeight/2;
            }
            crop2.mGroupID  = 2;//wdmao, mdp
            crop2.mCropRect.p_fractional.x  = 0;
            crop2.mCropRect.p_fractional.y  = 0;
            crop2.mCropRect.p_integral.x    = 0;
            crop2.mCropRect.p_integral.y    = 0;
            if (scenarioMode == 1) {
                crop2.mCropRect.s.w             = mAcdkMhalPrvParam.sensorWidth;
                crop2.mCropRect.s.h             = mAcdkMhalPrvParam.sensorHeight;
                crop2.mResizeDst.w              = mAcdkMhalPrvParam.sensorWidth;
                crop2.mResizeDst.h              = mAcdkMhalPrvParam.sensorHeight;
            } else if (scenarioMode == 0) {
                crop2.mCropRect.s.w             = mAcdkMhalPrvParam.sensorWidth/2;
                crop2.mCropRect.s.h             = mAcdkMhalPrvParam.sensorHeight/2;
                crop2.mResizeDst.w              = mAcdkMhalPrvParam.sensorWidth/2;
                crop2.mResizeDst.h              = mAcdkMhalPrvParam.sensorHeight/2;
            }
            crop3.mGroupID  = 3;//wroto, mdp. _BEFORE_ rotate
            crop3.mCropRect.p_fractional.x  = 0;
            crop3.mCropRect.p_fractional.y  = 0;
            crop3.mCropRect.p_integral.x    = 0;
            crop3.mCropRect.p_integral.y    = 0;
            if (scenarioMode == 1) {
                crop3.mCropRect.s.w             = mAcdkMhalPrvParam.sensorWidth;
                crop3.mCropRect.s.h             = mAcdkMhalPrvParam.sensorHeight;
            } else if (scenarioMode == 0) {
                crop3.mCropRect.s.w             = mAcdkMhalPrvParam.sensorWidth/2;
                crop3.mCropRect.s.h             = mAcdkMhalPrvParam.sensorHeight/2;
            }
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
                enqueParams.mpfnCallback = PurePreviewCb;
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
                ACDK_LOGD_DYN(g_acdkMhalPureDebug,"pass2 done");
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

        ACDK_LOGD_DYN(g_acdkMhalPureDebug, "dispo_virAddr(0x%p),dispo_phyAddr(0x%p)", (MUINT8 *)dispo_virAddr,(MUINT8 *)dispo_phyAddr);

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
            if (1) { //RRZO
                MUINT32         enqBufIdx = mRrzoRtbc.mEnqCount % mRrzoRtbc.mvBuffer.size();
                IImageBuffer*   pIBuf = (IImageBuffer*)mRrzoRtbc.mvBuffer.at(enqBufIdx);
#if ACDK_IPBASE_ISP30_SUPPORT
                MSize rrzoDstSize(mAcdkMhalPrvParam.sensorWidth/2, 2);
#else
                MSize rrzoDstSize(DUMMY_RRZ_W, DUMMY_RRZ_H);
#endif
                enqQ.mvOut.push_back(BufInfo(PORT_RRZO, pIBuf, rrzoDstSize, MRect(rrzoDstSize.w, rrzoDstSize.h),\
                                        mRrzoRtbc.mEnqCount, sof_idx, 0xFFFF));
            }

            if (m_pINormPipe->enque(enqQ)) {
                mImgoRtbc.mEnqCount++;
                mRrzoRtbc.mEnqCount++;
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
MINT32 AcdkMhalPure::acdkMhalCaptureProc()
{
#if 0
    ACDK_LOGD("+");

    //====== Loca Variable Declaration ======

    MINT32 imgo_memID,imgi_memID,vido_memID;
    MUINT32 imgi_virAddr,imgi_phyAddr,imgo_virAddr,imgo_phyAddr,vido_virAddr,vido_phyAddr;
    QBufInfo rQBufInfo;
    QTimeStampBufInfo rQTSBufInfo;

    //====== Variable Setting ======

    g_vPostProcInPorts.resize(1);
    g_vPostProcOutPorts.resize(1);

    //====== Get Property ======

    char value[PROPERTY_VALUE_MAX] = {'\0'};

    property_get("vendor.camera.acdkdump.enable", value, "0");
    MINT32 dumpEnable = atoi(value);

    //====== Pass1 ======

    //set pass1 out DMA register before CQ0 start
    //use CQ0 to update DMA register
    ACDK_LOGD("EPIPECmd_SET_CURRENT_BUFFER");
    m_pICamIOPipe->sendCommand((MINT32)EPIPECmd_SET_CURRENT_BUFFER, (MINT32)EPortIndex_IMGO,0,0);

    ACDK_LOGD("CQ0 start");

    m_pICamIOPipe->startCQ0();

    ACDK_LOGD("pass1 start");

    //3AHal control
    m_p3AHal->setIspProfile(EIspProfile_NormalCapture);

    m_pICamIOPipe->start(); //one time capture

    //3A scenario control
    m_p3AScenario->sendCommand(ECmd_CaptureStart);

    ACDK_LOGD("waiting pass1 done");

#if 0 // all these 3 steps are not required because FBC is implemented in CamIOPipe

    m_pICamIOPipe->irq(EPipePass_PASS1_TG1,EPIPEIRQ_PATH_DONE,ISP_DRV_IRQ_USER_ACDK);

    ACDK_LOGD("zsd pass1 done");
#endif
    //dequeue pass1 OUT buffer
    mPortID.index = EPortIndex_IMGO;
    m_pICamIOPipe->dequeOutBuf(mPortID, rQTSBufInfo);

    //MUST do
    ACDK_LOGD("clear VFDATA_EN bit");
    m_pICamIOPipe->stop();

    if(rQTSBufInfo.vBufInfo.size() == 0)
    {
        ACDK_LOGD("pass1 done deque no buffer");
    }
    else
    {
        imgo_virAddr = rQTSBufInfo.vBufInfo[0].u4BufVA[0];
        imgo_phyAddr = rQTSBufInfo.vBufInfo[0].u4BufPA[0];
        imgo_memID = rQTSBufInfo.vBufInfo[0].memID[0];
        ACDK_LOGD("pass1 done,imgo_virAddr(0x%x)",imgo_virAddr);

        if(dumpEnable == 4)
        {
            char szFileName[256];
            MINT32 i4WriteCnt = 0;

            ACDK_LOGD("prv VA(0x%x)",imgo_virAddr);

            sprintf(szFileName, "%s/acdkCap1.bin" , MEDIA_PATH);

            //====== Write RAW Data ======

            FILE *pFp = fopen(szFileName, "wb");

            if(NULL == pFp )
            {
                ACDK_LOGE("Can't open file to save image");
                fclose(pFp);
            }
            else
            {
                MUINT8 *pVirBufin = (MUINT8 *)imgo_virAddr;
                i4WriteCnt = fwrite(pVirBufin, 1, mAcdkMhalCapParam.capPass1ImemBuf.size, pFp);

                fflush(pFp);

                if(0 != fsync(fileno(pFp)))
                {
                    ACDK_LOGE("[.capPass1ImemBuf.size] fync fail");
                    fclose(pFp);
                    return ACDK_RETURN_NULL_OBJ;
                }

                ACDK_LOGD("[.capPass1ImemBuf.size] Save image file name:%s, w(%u), h(%u)", szFileName, m_tgi.u4ImgWidth, m_tgi.u4ImgHeight);

                fclose(pFp);
            }
        }

        if(mAcdkMhalCapParam.captureType == RAW_TYPE)
        {
            //====== Raw Capture Callback ======

            acdkMhalCBHandle(ACDK_CB_RAW, imgo_virAddr);

            ACDK_LOGD("ACDK_CB_RAW back");
        }

        //====== Pass2 ======

        for(MUINT32 i = 0; i < 2; ++i)
        {
            //enqueue pass2 IN buffer
            rQBufInfo.vBufInfo.resize(1);

            mPortID.index = EPortIndex_IMGI;
            rQBufInfo.vBufInfo[0].u4BufVA[0] = imgo_virAddr;
            rQBufInfo.vBufInfo[0].u4BufPA[0] = imgo_phyAddr;
            rQBufInfo.vBufInfo[0].memID[0]   = imgo_memID;
            rQBufInfo.vBufInfo[0].u4BufSize[0] = mAcdkMhalCapParam.capPass1ImemBuf.size;
            m_pPostProcPipe->enqueInBuf(mPortID, rQBufInfo);

            ACDK_LOGD("Update pass2 tuning cmdQ");

            //config pass2 every time

            g_vPostProcInPorts.at(0)  = &m_imgi;
            g_vPostProcOutPorts.at(0) = &m_vido;

            if(MTRUE != m_pPostProcPipe->configPipe(g_vPostProcInPorts, g_vPostProcOutPorts))
            {
                ACDK_LOGE("m_pPostProcPipe->configPipe() fail");
            }

            //set pass2 IN DMA register before pass2 start
            m_pPostProcPipe->sendCommand((MINT32)EPIPECmd_SET_CURRENT_BUFFER, (MINT32)EPortIndex_IMGI,0,0);

            //set pass2 OUT DMA register before pass2 start
            //m_pPostProcPipe->sendCommand((MINT32)EPIPECmd_SET_CURRENT_BUFFER, (MINT32)EPortIndex_DISPO,0,0);
            m_pPostProcPipe->sendCommand((MINT32)EPIPECmd_SET_CURRENT_BUFFER, (MINT32)EPortIndex_VIDO,0,0);

            ACDK_LOGD("pass2 start");

            m_pPostProcPipe->start();

            ACDK_LOGD("waiting pass2 done");

            m_pPostProcPipe->irq(EPipePass_Dip_Th0,EPIPEIRQ_PATH_DONE,ISP_DRV_IRQ_USER_ACDK);

            ACDK_LOGD("pass2 done");

            //====== Dequeue ======

            //dequeue pass2 out buffer
            mPortID.index = EPortIndex_VIDO;
            m_pPostProcPipe->dequeOutBuf(mPortID, rQTSBufInfo);
            vido_virAddr = rQTSBufInfo.vBufInfo[0].u4BufVA[0];
            vido_phyAddr = rQTSBufInfo.vBufInfo[0].u4BufPA[0];
            vido_memID   = rQTSBufInfo.vBufInfo[0].memID[0];

            //dequeue pass2 in buffer
            mPortID.index = EPortIndex_IMGI;
            m_pPostProcPipe->dequeInBuf(mPortID, rQTSBufInfo);
            imgi_virAddr = rQTSBufInfo.vBufInfo[0].u4BufVA[0];
            imgi_phyAddr = rQTSBufInfo.vBufInfo[0].u4BufPA[0];
            imgi_memID   = rQTSBufInfo.vBufInfo[0].memID[0];

            m_pPostProcPipe->stop();
            ACDK_LOGD("pass2 stop");

            //====== Capture Callback ======

            ACDK_LOGD("vido_virAddr(0x%x)", vido_virAddr);

            if(mAcdkMhalCapParam.captureType == RAW_TYPE || i == 1)
            {
                acdkMhalCBHandle(ACDK_CB_QV, vido_virAddr);

                ACDK_LOGD("pass2 ACDK_CB_QV back");

                break;
            }
            else
            {
                acdkMhalCBHandle(ACDK_CB_JPEG, vido_virAddr);

                ACDK_LOGD("pass2 ACDK_CB_JPEG back");

                //====== Config PostProcPipe 2nd Run for QV after JPEG Capture ======

                m_imgi.eImgFmt     = m_imgo.eImgFmt;        //  Image Pixel Format
                m_imgi.u4ImgWidth  = m_imgo.u4ImgWidth;     //  Image Width
                m_imgi.u4ImgHeight = m_imgo.u4ImgHeight;    //  Image Height
                m_imgi.u4Stride[ESTRIDE_1ST_PLANE] = m_imgo.u4Stride[ESTRIDE_1ST_PLANE];    // unit:PIXEL
                m_imgi.u4Stride[ESTRIDE_2ND_PLANE] = m_imgo.u4Stride[ESTRIDE_2ND_PLANE];    // unit:PIXEL
                m_imgi.u4Stride[ESTRIDE_3RD_PLANE] = m_imgo.u4Stride[ESTRIDE_3RD_PLANE];    // unit:PIXEL
                m_imgi.crop.x = 0;                          // unit:PIXEL
                m_imgi.crop.y = 0;                          // unit:PIXEL
                m_imgi.crop.w = m_imgi.u4ImgWidth;          // unit:PIXEL
                m_imgi.crop.h = m_imgi.u4ImgHeight;         // unit:PIXEL
                m_imgi.type        = EPortType_Memory;      //  EPortType
                m_imgi.index       = EPortIndex_IMGI;       //  port index
                m_imgi.inout       = EPortDirection_In;     //  0:in/1:out
                m_imgi.pipePass    = EPipePass_Dip_Th0;       //  select pass

                // config qv
                m_vido.eImgFmt     = (EImageFormat)mAcdkMhalCapParam.qvParam.frmFormat;   // Image Pixel Format
                m_vido.eImgRot     = (EImageRotation)mAcdkMhalCapParam.qvParam.orientation;    //dispo NOT support rotation
                m_vido.eImgFlip    = mAcdkMhalCapParam.qvParam.flip ? eImgFlip_ON : eImgFlip_OFF;  //dispo NOT support flip
                m_vido.u4ImgWidth  = mAcdkMhalCapParam.qvParam.w;           // Image Width
                m_vido.u4ImgHeight = mAcdkMhalCapParam.qvParam.h;           // Image Height
                m_vido.u4Stride[ESTRIDE_1ST_PLANE] = mAcdkMhalCapParam.qvParam.stride[0];
                m_vido.u4Stride[ESTRIDE_2ND_PLANE] = mAcdkMhalCapParam.qvParam.stride[1];
                m_vido.u4Stride[ESTRIDE_3RD_PLANE] = mAcdkMhalCapParam.qvParam.stride[2];
                m_vido.type        = EPortType_DISP_RDMA;   // EPortType
                m_vido.index       = EPortIndex_VIDO;       // port index
                m_vido.inout       = EPortDirection_Out;    // 0:in/1:out

                //set CQ first before pipe config
                m_pPostProcPipe->sendCommand(EPIPECmd_SET_CQ_CHANNEL,(MINT32)EPIPE_PASS2_CQ1,0,0);

                // config IO port
                g_vPostProcInPorts.at(0) = &m_imgi;
                g_vPostProcOutPorts.at(0) = &m_vido;

                if(MTRUE != m_pPostProcPipe->configPipe(g_vPostProcInPorts, g_vPostProcOutPorts))
                {
                    ACDK_LOGE("m_pPostProcPipe->configPipe() fail");
                    break;
                }

                // enque buffer
                ACDK_LOGD("capQVImemBuf : virAddr=0x%x",mAcdkMhalCapParam.capQVImemBuf.virtAddr);

                mPortID.index = EPortIndex_VIDO;    //VIDO
                rQBufInfo.vBufInfo.resize(1);
                rQBufInfo.vBufInfo[0].u4BufSize[0] = mAcdkMhalCapParam.capQVImemBuf.size;    //bytes
                rQBufInfo.vBufInfo[0].u4BufVA[0]   = mAcdkMhalCapParam.capQVImemBuf.virtAddr;
                rQBufInfo.vBufInfo[0].u4BufPA[0]   = mAcdkMhalCapParam.capQVImemBuf.phyAddr;
                rQBufInfo.vBufInfo[0].memID[0]     = mAcdkMhalCapParam.capQVImemBuf.memID;

                m_pPostProcPipe->enqueOutBuf(mPortID, rQBufInfo);
            }
        }
    }

    ACDK_LOGD("-");
#endif
    return ACDK_RETURN_NO_ERROR;
}

/*******************************************************************************
*
*******************************************************************************/
MUINT32 AcdkMhalPure::acdkMhalGetShutTime()
{
    return ACDK_RETURN_NO_ERROR;
}

/*******************************************************************************
*
*******************************************************************************/
MVOID AcdkMhalPure::acdkMhalSetShutTime(MUINT32 a_time)
{
    (void)a_time;
}

/*******************************************************************************
*
*******************************************************************************/
MUINT32 AcdkMhalPure::acdkMhalGetAFInfo()
{
    MUINT32 u32AFInfo;

    ACDK_LOGD("acdkMhalGetAFInfo");
    u32AFInfo = g_pAcdkMHalPureObj->mFocusSucceed;

    return u32AFInfo;
}
/*******************************************************************************
* doNotifyCb
*******************************************************************************/
void AcdkMhalPure::doNotifyCb(int32_t _msgType,
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
void AcdkMhalPure::doDataCb(int32_t _msgType,
                               void *_data,
                               uint32_t _size)
{
    (void)_msgType; (void)_data; (void)_size;
}

/*******************************************************************************
* PreviewCb
*******************************************************************************/
MVOID PurePreviewCb(QParams& rParams)
{
    ::sem_post(&g_SemPurePreviewCb);
}

