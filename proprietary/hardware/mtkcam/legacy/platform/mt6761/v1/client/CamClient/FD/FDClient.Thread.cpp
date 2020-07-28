/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#define LOG_TAG "MtkCam/CamClient/FDClient"
//
#include "FDClient.h"
using namespace NSCamClient;
using namespace NSFDClient;
//
#include <sys/prctl.h>
#include <sys/resource.h>
#include <cutils/properties.h>
//
#include <mtkcam/hwutils/CameraProfile.h>  // For CPTLog*()/AutoCPTLog class.
using namespace CPTool;
//#include <mtkcam/featureio/fd_hal_base.h>

#if '1'==MTKCAM_HAVE_3A_HAL
#include <mtkcam/featureio/IHal3A.h>
using namespace NS3A;
#endif
#include <mtkcam/drv/IHalSensor.h>

bool g_last_SDenabled = 0;
bool g_last_GDenabled = 0;

/******************************************************************************
*
*******************************************************************************/
#define ENABLE_LOG_PER_FRAME        (1)
#define DELAY_ASDCLIENT_CREATE      (1)

//halFDBase*   mpFDHalObj;


/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

/******************************************************************************
 *
 ******************************************************************************/
// Ask this object's thread to exit. This function is asynchronous, when the
// function returns the thread might still be running. Of course, this
// function can be called from a different thread.
void
FDClient::
requestExit()
{
    MY_LOGD("+");
    Thread::requestExit();
    //
    postCommand(Command::eID_EXIT);
    //
    MY_LOGD("-");
}


/******************************************************************************
 *
 ******************************************************************************/
// Good place to do one-time initializations
status_t
FDClient::
readyToRun()
{
    ::prctl(PR_SET_NAME,"FDClient@Preview", 0, 0, 0);
    //
    mi4ThreadId = ::gettid();

    //  thread policy & priority
    //  Notes:
    //      Even if pthread_create() with SCHED_OTHER policy, a newly-created thread
    //      may inherit the non-SCHED_OTHER policy & priority of the thread creator.
    //      And thus, we must set the expected policy & priority after a thread creation.
    int const policy    = SCHED_OTHER;
    int const priority  = 0;
    //
    //
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    sched_p.sched_priority = priority;  //  Note: "priority" is nice value
    sched_setscheduler(0, policy, &sched_p);
    setpriority(PRIO_PROCESS, 0, priority);
    //
    //
    MY_LOGD(
        "policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d)"
        , policy, ::sched_getscheduler(0)
        , priority, getpriority(PRIO_PROCESS, 0)
    );

    return NO_ERROR;
}


/******************************************************************************
 *
 ******************************************************************************/
void
FDClient::
postCommand(Command::EID cmd)
{
    Mutex::Autolock _lock(mCmdQueMtx);
    //
    if  ( ! mCmdQue.empty() )
    {
        Command::EID const& rBegCmd = *mCmdQue.begin();
        MY_LOGW("que size:%d > 0 with begin cmd::%d", mCmdQue.size(), rBegCmd);
    }
    //
    mCmdQue.push_back(cmd);
    mCmdQueCond.broadcast();
    //
    MY_LOGD("- new command::%d", cmd);
}


/******************************************************************************
 *
 ******************************************************************************/
bool
FDClient::
getCommand(Command::EID &cmd)
{
    bool ret = false;
    //
    Mutex::Autolock _lock(mCmdQueMtx);
    //
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "+ que size(%d)", mCmdQue.size());
    //
    //  Wait until the queue is not empty or this thread will exit.
    while   ( mCmdQue.empty() && ! exitPending() )
    {
        status_t status = mCmdQueCond.wait(mCmdQueMtx);
        if  ( NO_ERROR != status )
        {
            MY_LOGW("wait status(%d), que size(%d), exitPending(%d)", status, mCmdQue.size(), exitPending());
        }
    }
    //
    if  ( ! mCmdQue.empty() )
    {
        //  If the queue is not empty, take the first command from the queue.
        ret = true;
        cmd = *mCmdQue.begin();
        mCmdQue.erase(mCmdQue.begin());
        MY_LOGD("command:%d", cmd);
    }
    //
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "- que size(%d), ret(%d)", mCmdQue.size(), ret);
    return  ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
FDClient::
threadLoop()
{
    Command::EID cmd;
    bool bExit = false;
    while  ( getCommand(cmd) )
    {
        bExit = false;
        switch  (cmd)
        {
        case Command::eID_WAKEUP:
            onClientThreadLoop();
            break;
        //
        case Command::eID_EXIT:
        default:
            bExit = true;
            MY_LOGD("Command::%d", cmd);
            mFDRunning = false;
            break;
        }
        if(bExit)
            break;
    }
    //
    MY_LOGD("bExit : %d -", bExit);
    return  !bExit;
}


/******************************************************************************
 *
 ******************************************************************************/
void
FDClient::
onClientThreadLoop()
{
    MY_LOGD("+");

    //(0) pre-check
    sp<IImgBufQueue> pBufQueue = NULL;
    {
        //Mutex::Autolock _l(mModuleMtx);
        //
        pBufQueue = mpImgBufQueue;
        if ( pBufQueue == 0 || ! isEnabledState())
        {
            MY_LOGE("pBufMgr(%p), isEnabledState(%d)", pBufQueue.get(), isEnabledState());
            return;
        }
    }

    //(1) prepare all TODO buffers
    if ( ! initBuffers(pBufQueue) )
    {
        MY_LOGE("initBuffers failed");
        return;
    }

    //(2) start
    if  ( !pBufQueue->startProcessor() )
    {
        MY_LOGE("startProcessor failed");
        return;
    }

    //BinChang 2012/11/01
    //Initial FD

    int srcWidth=0,  srcHeight=0;
    int pv_srcWidth =0, pv_srcHeight = 0;
    MINT32  FDNeedInit = 0;
    MINT32  FDInitRet = 0;

#if '1' == MTKCAM_HR_MONITOR_SUPPORT
    // HRD Result
    HR_RESULT HRDResult;
#endif
    //
    mpParamsMgr->getPreviewSize(&pv_srcWidth, &pv_srcHeight);

    srcWidth = 640;
    if(pv_srcWidth != 0)
        srcHeight = srcWidth * pv_srcHeight / pv_srcWidth;
    else
        srcHeight = 480;

    //mpFDHalObj =  new halFDBase();
    mpFDHalObj = halFDBase::createInstance(HAL_FD_OBJ_FDFT_SW);
    //mpFDHalObj->halFDInit(srcWidth, srcHeight, (MUINT32) FDWorkingBuffer, FDWorkingBufferSize, 1); //1:Enale SW resizer
    if (mpFDHalObj != NULL)
        FDInitRet = mpFDHalObj->halFDInit(srcWidth, srcHeight, FDWorkingBuffer, FDWorkingBufferSize, 1, mIsSDenabled); //1:Enale SW resizer
    g_last_SDenabled = 0;
    g_last_GDenabled = 0;

    //************************************************************//
    //BinChang 2014/01/07
    //Initial GD Start
    //mpGDHalObj = halGSBase::createInstance(HAL_GS_OBJ_SW);
    //mpGDHalObj->halGSInit(srcWidth, srcHeight, (MUINT32) FDWorkingBuffer, FDWorkingBufferSize);
    //************************************************************//

    #if (1 == AUTO_SCENE_DETECT_SUPPORT)
    #if (DELAY_ASDCLIENT_CREATE == 0)
    //ASD Init
    mpASDClient = IAsdClient::createInstance(mpParamsMgr);
    if  ( mpASDClient == 0 || ! mpASDClient->init() )
    {
        MY_LOGE("mpASDClient init failed");
    }
    mpASDClient->setCallbacks(mpCamMsgCbInfo);
    #endif
    #endif

    //(3) Do in loop until stopFaceDetection has been called
    //    either by sendCommand() or by stopPreview()
    while ( isEnabledState() )
    {
        if (mpFDHalObj == NULL)
        {
            mpFDHalObj = halFDBase::createInstance(HAL_FD_OBJ_FDFT_SW);
            if (mpFDHalObj != NULL)
                FDInitRet = mpFDHalObj->halFDInit(srcWidth, srcHeight, FDWorkingBuffer, FDWorkingBufferSize, 1, mIsSDenabled); //1:Enale SW resizer
            MY_LOGD("Multi-User Case. mpFDHalObj: %p", mpFDHalObj);
            //continue;
        }
        // (3.1) deque from processor
        ImgBufQueNode rQueNode;
        if ( ! waitAndHandleReturnBuffers(pBufQueue, rQueNode) )
        {
            MY_LOGD("No available deque-ed buffer; to be leaving");
            continue;
        }

        if ( rQueNode.getImgBuf() == 0 )
        {
            MY_LOGE("rQueNode.getImgBuf() == 0");
            continue;
        }

        // (3.2) do FD algorithm
        bool isDetected_FD = false;
        bool isDetected_SD = false;
        bool isDetected_GD = false;

        int const i4CamMode = mpParamsMgr->getInt(MtkCameraParameters::KEY_CAMERA_MODE);
        if  ( i4CamMode == MtkCameraParameters::CAMERA_MODE_NORMAL )
            Rotation_Info = 360;
        else
            Rotation_Info = mpParamsMgr->getInt(CameraParameters::KEY_ROTATION);
        //MY_LOGD("Rotation_Info:%d", Rotation_Info);

        if(FDInitRet) {
            MY_LOGE("FD reinit ++");
            if (mpFDHalObj != NULL)
                FDInitRet = mpFDHalObj->halFDInit(srcWidth, srcHeight, FDWorkingBuffer, FDWorkingBufferSize, 1, mIsSDenabled); //1:Enale SW resizer
            MY_LOGE("FD reinit --:%d", FDInitRet);
            if(FDInitRet)
                goto GET_NEXT_FRAME;
        }

        CPTLog(Event_Hal_Client_CamClient_FD, CPTFlagStart);    // Profiling Start.
        if ( ! isMsgEnabled() )
        {
            MY_LOGD("Don't do FD");
        }
        else
        {
            if( (g_last_SDenabled != mIsSDenabled || FDNeedInit) && isEnabledFD() )
            {
            mpFDHalObj->halFDUninit();
            mpFDHalObj->destroyInstance();
            mpFDHalObj = NULL;
            mpFDHalObj = halFDBase::createInstance(HAL_FD_OBJ_FDFT_SW);
            FDInitRet = mpFDHalObj->halFDInit(srcWidth, srcHeight, FDWorkingBuffer, FDWorkingBufferSize, 1, mIsSDenabled); //1:Enale SW resizer
            g_last_SDenabled = mIsSDenabled;
            }

            FDNeedInit = 0;

            //*******************************************************************//
            //if( g_last_GDenabled != mIsGDenabled)
            if( (g_last_GDenabled==0) && (mIsGDenabled == 1) && isEnabledFD())
            {
                //MY_LOGD("g_last_GDenabled:%d mIsGDenabled:%d,", g_last_GDenabled, mIsGDenabled);
                //Initial GD Start
                MY_LOGD("StartGD_Init: gsrcWidth:%d, gsrcHeight:%d", srcWidth, srcHeight);
                mpGDHalObj = halGSBase::createInstance(HAL_GS_OBJ_SW);
                mpGDHalObj->halGSInit(srcWidth, srcHeight, FDWorkingBuffer, FDWorkingBufferSize);
                g_last_GDenabled = mIsGDenabled;
            }
            //*******************************************************************//
            if (mpFDHalObj != NULL)
            {
              //if ( ! doFD(rQueNode, isDetected_FD, isDetected_SD,  mIsSDenabled) )
              if ( ! doFD(rQueNode, isDetected_FD, isDetected_SD,  mIsSDenabled, isDetected_GD,  mIsGDenabled) )
              {
                  MY_LOGE("doFD failed");
                  CPTLog(Event_Hal_Client_CamClient_FD, CPTFlagEnd);     // Profiling End.
                  FDNeedInit = 1;
                  goto GET_NEXT_FRAME;
              }
            }
        }

        CPTLog(Event_Hal_Client_CamClient_FD, CPTFlagEnd);     // Profiling End.

        // (3.3)
        #if '1'==MTKCAM_HAVE_3A_HAL
        //Hal3ABase* p3AHal = Hal3ABase::createInstance(NSCam::IHalSensorList::get()->querySensorDevIdx(mpParamsMgr->getOpenId()));
        if (mpFDHalObj != NULL && isEnabledFD())
        {
          IHal3A* p3AHal = IHal3A::createInstance(NS3A::IHal3A::E_Camera_1, mpParamsMgr->getOpenId(), LOG_TAG);
          if (p3AHal)
          {
              p3AHal->setFDInfo(mpDetectedFaces);
              p3AHal->destroyInstance(LOG_TAG);
          }
        }
        #endif

        if (mpFDHalObj != NULL && isEnabledFD())
        {
          // (3.4)
          //performCallback(isDetected_FD, isDetected_SD);
          performCallback(isDetected_FD, isDetected_SD, isDetected_GD);

          #if (1 == AUTO_SCENE_DETECT_SUPPORT)
          //Call ASD if doFD
          if(isMsgEnabled())
          {
              #if (DELAY_ASDCLIENT_CREATE == 1)
              //ASD Init
              bool const isAsdEnabled = mpParamsMgr->getShotModeStr() == MtkCameraParameters::CAPTURE_MODE_ASD_SHOT;
              if(isAsdEnabled) {
                  if(mpASDClient == NULL) {
                    mpASDClient = IAsdClient::createInstance(mpParamsMgr);
                    if  ( mpASDClient == 0 || ! mpASDClient->init() )
                    {
                        MY_LOGE("mpASDClient init failed");
                    }
                    mpASDClient->setCallbacks(mpCamMsgCbInfo);
                  }
                  if(mpASDClient != NULL) {
                      int FaceNum = mpFDHalObj->halFDGetFaceResult(mpDetectedFaces);
                      mpASDClient->update(DDPBuffer, srcWidth, srcHeight, FaceNum);
                  }
              }
              #else
              int FaceNum = mpFDHalObj->halFDGetFaceResult(mpDetectedFaces);
              mpASDClient->update(DDPBuffer, srcWidth, srcHeight, FaceNum);
              #endif
          }
          #endif
        }

#if '1' == MTKCAM_HR_MONITOR_SUPPORT
        if(isEnabledHR()) {
            // HRD Init
            if(mpHRObj == NULL) {
                mpHRObj = halHRBase::createInstance(HAL_HR_OBJ_SW);
                if(mpHRObj != NULL) {
                    mpHRObj->halHRInit(srcWidth, srcHeight, NULL, mLogLevel);
                }
            }
            if(mpHRObj != NULL) {
                int Starting = mHRStarting;
                int DoReset = 0;
                if(mpDetectedFaces->number_of_faces != 0 && Starting == false) {
                    Starting = mHRStarting = true;
                    DoReset = 1;
                }
                if(Starting || mHRMode >= 2) {
                    mpHRObj->halHRDo((MUINT8 *)rQueNode.getImgBuf()->getVirAddr(),
                                      Rotation_Info,
                                      rQueNode.getImgBuf()->getTimestamp(),
                                      mpDetectedFaces->number_of_faces,
                                      mpDetectedFaces->faces[0],
                                      mHRMode,
                                      DoReset);
                    mpHRObj->halHRGetResult(&HRDResult);
                    HRDResult.x1 = (HRDResult.x1 * pv_srcWidth / srcWidth);
                    HRDResult.x2 = (HRDResult.x2 * pv_srcWidth / srcWidth);
                    HRDResult.y1 = (HRDResult.y1 * pv_srcWidth / srcWidth);
                    HRDResult.y2 = (HRDResult.y2 * pv_srcWidth / srcWidth);

                    HRDResult.prev_w = pv_srcWidth;
                    HRDResult.prev_h = pv_srcHeight;
                    HRDResult.facing = IHalSensorList::get()->queryFacingDirection(mpParamsMgr->getOpenId());
                    performHRCallback(&HRDResult, 0);
                }
            }
        }
#endif

        // (3.5)
        // enque back to processor
    GET_NEXT_FRAME:
        handleReturnBuffers(pBufQueue, rQueNode); //enque to "TODO"
    }

    #if (1 == AUTO_SCENE_DETECT_SUPPORT)
    if  ( mpASDClient != 0 )
    {
        mpASDClient->uninit();
        mpASDClient = NULL;
    }
    #endif

    //(4) stop.
    pBufQueue->pauseProcessor();
    pBufQueue->flushProcessor(); // clear "TODO"
    pBufQueue->stopProcessor();  // clear "DONE"
/*
#if '1'==MTKCAM_HAVE_3A_HAL
    Hal3ABase* p3AHal = Hal3ABase::createInstance(NSCam::IHalSensorList::get()->querySensorDevIdx(mpParamsMgr->getOpenId()));
    if (p3AHal)
    {
        mpDetectedFaces->number_of_faces = 0;
        p3AHal->setFDInfo(mpDetectedFaces);
        p3AHal->destroyInstance();
    }
#endif
*/
    uninitBuffers();

    if(mpGDHalObj != NULL )
    {
      MY_LOGD("GD_Uninit");
      mpGDHalObj->halGSUninit();
      mpGDHalObj->destroyInstance();
      mpGDHalObj = NULL;
    }

    if (mpFDHalObj != NULL)
    {
      mpFDHalObj->halFDUninit();
      mpFDHalObj->destroyInstance();
      mpFDHalObj = NULL;
    }
#if '1' == MTKCAM_HR_MONITOR_SUPPORT
    if(mpHRObj != NULL) {
        mpHRObj->halHRUninit();
        mpHRObj->destroyInstance();
        mpHRObj = NULL;
    }
#endif

    MY_LOGD("-");
}

