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
#define LOG_TAG "Hal3ARawSMVR_thread"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG   (1)
#endif
#define LOG_PERFRAME_ENABLE (1)

#include <utils/threads.h>
#include <utils/List.h>
#include <mtkcam/def/PriorityDefs.h>
#include <mtkcam/utils/std/common.h>
#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>

#include <pthread.h>
#include <semaphore.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <IThreadRaw.h>
#include <IThreadRawSMVR.h>

#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include <af_feature.h>
#include <pd_buf_mgr/pd_buf_mgr.h>
#include <ae_mgr/ae_mgr_if.h>
#include <af_mgr/af_mgr.h>
#include <af_mgr/af_mgr_if.h>
#include <IEventIrq.h>
#include <debug/DebugUtil.h>

#include <cutils/properties.h>
#include <cutils/atomic.h>

#if defined(HAVE_AEE_FEATURE)
#include <aee.h>
#define AEE_ASSERT_3A_HAL(String) \
          do { \
              aee_system_exception( \
                  "Hal3A", \
                  NULL, \
                  DB_OPT_DEFAULT, \
                  String); \
          } while(0)
#else
#define AEE_ASSERT_3A_HAL(String)
#endif

#define GET_PROP(prop, init, val)\
{\
    val = property_get_int32(prop, (init));\
}

#define MY_LOGD(fmt, arg...) \
    do { \
        if (mu4DebugLogWEn) { \
            CAM_LOGW(fmt, ##arg); \
        } else { \
            CAM_LOGD(fmt, ##arg); \
        } \
    }while(0)

#define MY_LOGD_IF(cond, ...) \
    do { \
        if (mu4DebugLogWEn) { \
            CAM_LOGW_IF(cond, __VA_ARGS__); \
        } else { \
            if ( (cond) ) { CAM_LOGD(__VA_ARGS__); } \
        } \
    }while(0)


#define EN_LOG_RAW_THREAD 1
#define SEM_TIME_LIMIT_NS       16000000000L

using namespace android;
using namespace NSCam::Utils;
using namespace NS3Av3;

/******************************************************************************
*
*******************************************************************************/
class ThreadRawSMVRImp : public IThreadRaw
{
public:
    //
    static IThreadRaw* createInstance(Hal3ARaw* pHal3A, MINT32 iSensorDev, MINT32 iSensorIdx);
    virtual MBOOL destroyInstance();
    virtual MVOID postToAESenThread();
    virtual MVOID enableAFThread(AfStateMgr* pAfStateMgr);
    virtual MVOID disableAFThread();
    virtual MVOID enableAFVsyncThread();
    virtual MVOID disableAFVsyncThread();
    virtual MVOID pauseAFThread(){}
    virtual MVOID resumeAFThread(){}

    virtual MBOOL sendRequest(ECmd_T const eCmd, MUINTPTR const i4Arg = 0);
    virtual MVOID notifyPreStop();

protected: //private:
    ThreadRawSMVRImp();
    ThreadRawSMVRImp(Hal3ARaw* pHal3A, MINT32 iSensorDev, MINT32 iSensorIdx);
    virtual ~ThreadRawSMVRImp();

    ThreadRawSMVRImp(const ThreadRawSMVRImp&);
    ThreadRawSMVRImp& operator=(const ThreadRawSMVRImp&);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  AE Thread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /**
    * @brief create AE thread
    */
    virtual MVOID createAEThread();
    /**
    * @brief destroy AE thread
    */
    virtual MVOID destroyAEThread();
    /**
    * @brief change AE thread setting
    */
    virtual MVOID changeAEThreadSetting();
    /**
    * @brief AE thread execution function
    */
    static  MVOID*  onAEThreadLoop(MVOID*);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  AF Vsync Thread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    /**
    * @brief change AF Vsync thread setting
    */
    virtual MVOID changeAFVsyncThreadSetting();
    /**
    * @brief AF Vsync thread execution function
    */
    static  MVOID*  onAFVsyncThreadLoop(MVOID*);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  AF Thread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /**
    * @brief change AF thread setting
    */
    virtual MVOID changeAFThreadSetting();
    /**
    * @brief AF thread execution function
    */
    static  MVOID*  onAFThreadLoop(MVOID*);
    /**
    * @brief add AF requests in request queue
    * @param [in] AF requests; please refer to IThreadRawSMVR.h
    */
    virtual MVOID addReqestQ(IThreadRaw::ReqQ_T &rReq);
    /**
    * @brief clear all requests in current request queue
    */
    virtual MVOID clearReqestQ();
    /**
    * @brief get AF request from the head of AF request queue
    * @param [in] AF requests; please refer to IThreadRawSMVR.h
    */
    virtual MBOOL getRequest(IThreadRaw::ReqQ_T &rReq, MINT32 i4Count = 0);

    MBOOL sem_wait_rt(sem_t *pSem, nsecs_t reltime, const char* info);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected: //private:
    Hal3ARaw*       mpHal3A;
    MUINT32         mu4LogEn;
    static MUINT32  mu4DebugLogWEn;
    MINT32          mi4User;
    MINT32          mi4SensorDev;
    MINT32          mi4SensorIdx;
    Mutex           mLock;
    Mutex           mModuleMtx;
    MUINT32         mbPreStop;
    // AE Thread
    Mutex           mAESenMtx;
    sem_t           semAESen;
    sem_t           semAESenThdEnd;
    MBOOL           mbEnAESenThd;
    pthread_t       mAEThread;
    IEventIrq*      m_pEvent;
    MINT32          mi4SubsampleCount;
    // AF Thread
    MUINT32         mLastMagicNumber;
    IEventIrq*      mpAFEventIrq;
    pthread_t       mAFThread;
    MINT32          mbAFThreadLoop;
    AfStateMgr*     mpAfStateMgr;
    List<ReqQ_T>    mReqQ;

    // AF Vsync Thread
    Mutex           mAFVsyncMtx;
    IEventIrq*      mpAFVsyncEventIrq;
    MINT32          mbAFVsyncThreadLoop;
    sem_t           semAFVsync;
    sem_t           semAFVsyncThdEnd;
    MBOOL           mbEnAFVsyncThd;
    pthread_t       mAFVsyncThread;
};

/*******************************************************************************
* utilities
********************************************************************************/
MUINT32 ThreadRawSMVRImp::mu4DebugLogWEn = 0;

IThreadRaw*
IThreadRawSMVR::createInstance(Hal3ARaw* pHal3A, MINT32 iSensorDev, MINT32 iSensorIdx)
{
    return ThreadRawSMVRImp::createInstance(pHal3A, iSensorDev, iSensorIdx);
}

IThreadRaw*
ThreadRawSMVRImp::
createInstance(Hal3ARaw* pHal3A, MINT32 iSensorDev, MINT32 iSensorIdx)
{
    ThreadRawSMVRImp* pObj = new ThreadRawSMVRImp(pHal3A, iSensorDev, iSensorIdx);
    if(pObj)
        pObj->createAEThread();
    return pObj;
}

MBOOL
ThreadRawSMVRImp::
destroyInstance()
{
    destroyAEThread();            // user count protected
    delete this;
    return MTRUE;
}

ThreadRawSMVRImp::
ThreadRawSMVRImp()
    : mpHal3A(NULL)
    , mu4LogEn(MFALSE)
    , mi4User(0)
    , mLock()
    , mi4SensorDev(0)
    , mi4SensorIdx(0)
    , mbEnAESenThd(MFALSE)
    , mbAFThreadLoop(0)
    , mLastMagicNumber(0)
    , mi4SubsampleCount(1)
    , m_pEvent(NULL)
    , mpAFEventIrq(NULL)
    , mpAfStateMgr(NULL)
    , mbPreStop(MFALSE)
{
}

ThreadRawSMVRImp::
ThreadRawSMVRImp(Hal3ARaw* pHal3A, MINT32 iSensorDev, MINT32 iSensorIdx)
    : mpHal3A(pHal3A)
    , mu4LogEn(MFALSE)
    , mi4User(0)
    , mLock()
    , mi4SensorDev(iSensorDev)
    , mi4SensorIdx(iSensorIdx)
    , mbEnAESenThd(MFALSE)
    , mbAFThreadLoop(0)
    , mbAFVsyncThreadLoop(0)
    , mLastMagicNumber(0)
    , mi4SubsampleCount(1)
    , m_pEvent(NULL)
    , mpAFEventIrq(NULL)
    , mpAfStateMgr(NULL)
    , mbPreStop(MFALSE)
{
    GET_PROP("vendor.debug.thread_raw.log", 0, mu4LogEn);
    mu4DebugLogWEn = DebugUtil::getDebugLevel(DBG_3A);
    MY_LOGD("[%s] mi4SensorDev(%d) mi4SensorIdx(%d)", __FUNCTION__, mi4SensorDev, mi4SensorIdx);
}

ThreadRawSMVRImp::
~ThreadRawSMVRImp()
{
    MY_LOGD("[%s]", __FUNCTION__);
}

/******************************************************************************
* create AE thread
*******************************************************************************/
MVOID
ThreadRawSMVRImp::createAEThread()
{
    Mutex::Autolock lock(mLock);

    if (mi4User > 0)
    {
        MY_LOGD_IF(mu4LogEn, "[%s] mi4User(%d)", __FUNCTION__, mi4User);
    }
    else
    {
        MY_LOGD("[%s] +", __FUNCTION__);
        mbEnAESenThd = MTRUE;
        sem_init(&semAESen, 0, 0);

        MINT32 result = pthread_create(&mAEThread, NULL, onAEThreadLoop, this);
        if(result != 0)
          CAM_LOGE("[%s] result(%d)", __FUNCTION__, result);
        MY_LOGD("[%s] -", __FUNCTION__);
    }
    android_atomic_inc(&mi4User);
}

/******************************************************************************
* destroy AE thread
*******************************************************************************/
MVOID
ThreadRawSMVRImp::destroyAEThread()
{
    Mutex::Autolock lock(mLock);

    if (mi4User > 0)
    {
        // More than one user, so decrease one User.
        android_atomic_dec(&mi4User);

        if (mi4User == 0) // There is no more User after decrease one User
        {
            MY_LOGD("[%s] +", __FUNCTION__);
            mbEnAESenThd = MFALSE;
            mi4SubsampleCount = 1;
            ::sem_post(&semAESen);
            pthread_join(mAEThread, NULL);
            //
            MY_LOGD("[%s] -", __FUNCTION__);
        }
        else    // There are still some users.
        {
             MY_LOGD_IF(mu4LogEn, "[%s] mi4User(%d)", __FUNCTION__, mi4User);
        }
    }
}

/******************************************************************************
* change AE thread setting
*******************************************************************************/
MVOID
ThreadRawSMVRImp::changeAEThreadSetting()
{
    // (1) set name
    ::prctl(PR_SET_NAME, "AESenThd", 0, 0, 0);

    // (2) set policy/priority
    {
        int const expect_policy     = SCHED_OTHER;
        int const expect_priority   = NICE_CAMERA_3A_MAIN;
        int policy = 0, priority = 0;
        setThreadPriority(expect_policy, expect_priority);
        getThreadPriority(policy, priority);
        //
        MY_LOGD(
            "[Hal3AFlowCtrl::onAEThreadLoop] policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d)"
            , expect_policy, policy, expect_priority, priority
        );
    }

}

/******************************************************************************
* AE thread execution function
*******************************************************************************/
MVOID*
ThreadRawSMVRImp::onAEThreadLoop(MVOID *arg)
{
    MY_LOGD("[%s] +" ,__FUNCTION__);
    // (1) change thread setting
    ThreadRawSMVRImp *_this = reinterpret_cast<ThreadRawSMVRImp*>(arg);
    IEventIrq::Duration rDuration;
    _this->changeAEThreadSetting();

    // (2) thread-in-loop
    while(1)
    {
        ::sem_wait(&_this->semAESen);
        if ( ! _this->mbEnAESenThd) break;

        _this->m_pEvent->wait(rDuration);
        MY_LOGD_IF(_this->mu4LogEn, "[AESensorThreadLoop] updateSensorbyI2C\n");
        AAA_TRACE_D("AE Sensor I2C");
        IAeMgr::getInstance().updateSensorbyI2CBufferMode(_this->mi4SensorDev);
        AAA_TRACE_END_D;

        //sync with 3A thread when PreviewEnd
        {
            int Val;
            MY_LOGD_IF(_this->mu4LogEn, "[Hal3A::AESensorThreadLoop] start waiting mAESenMtx lock (3)\n");
            Mutex::Autolock autoLock(_this->mAESenMtx);
            MY_LOGD_IF(_this->mu4LogEn, "[Hal3A::AESensorThreadLoop] get mAESenMtx lock (3)\n");

            ::sem_getvalue(&_this->semAESenThdEnd, &Val);
            MY_LOGD_IF(_this->mu4LogEn, "[AESensorThreadLoop] semAESenThdEnd before post = %d\n", Val);
            if (Val == 0) ::sem_post(&_this->semAESenThdEnd); //to be 1, 1 means AE set Sensor done, this can tolerate I2C delay too long
        }
    }

    MY_LOGD("[%s] -" ,__FUNCTION__);

    return NULL;
}

/******************************************************************************
* trigger AE thread to updateSensorByI2C
*******************************************************************************/
MVOID
ThreadRawSMVRImp::postToAESenThread()
{
    Mutex::Autolock autoLock(mAESenMtx);
    int Val;
    ::sem_getvalue(&semAESenThdEnd, &Val);
    if (Val == 1) ::sem_wait(&semAESenThdEnd); //to be 0, it won't block, 0 means AE set Sensor not ready yet, this can tolerate I2C delay too long
    ::sem_post(&semAESen);
    MY_LOGD_IF(mu4LogEn, "[postToAESenThread] sem_post semAESen, semAESenThdEnd before wait = %d\n", Val);
}

/******************************************************************************
* change AF thread setting
*******************************************************************************/
MVOID
ThreadRawSMVRImp::changeAFThreadSetting()
{
    // (1) set name
    ::prctl(PR_SET_NAME,"AFthread", 0, 0, 0);

    // (2) set policy/priority
    {
        int const expect_policy     = SCHED_OTHER;
        int const expect_priority   = NICE_CAMERA_AF;
        int policy = 0, priority = 0;
        setThreadPriority(expect_policy, expect_priority);
        getThreadPriority(policy, priority);
        //
        MY_LOGD(
            "[changeAFThreadSetting] policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d)"
            , expect_policy, policy, expect_priority, priority
        );
    }

}

/******************************************************************************
* AF thread execution function
*******************************************************************************/
MVOID*
ThreadRawSMVRImp::onAFThreadLoop(MVOID *arg)
{
#define AF_START_MAGIC_NUMBER 4
    MY_LOGD("[%s] +" ,__FUNCTION__);
    MY_LOGD("[AFThread] tid: %d \n", gettid());

    ThreadRawSMVRImp *_this = reinterpret_cast<ThreadRawSMVRImp*>(arg);
    IEventIrq::Duration duration;
    _this->changeAFThreadSetting();

    // StartPreview speed up
    IAfMgr::getInstance().AFThreadStart(_this->mi4SensorDev);

    while (_this->mbAFThreadLoop) {

        MY_LOGD_IF(_this->mu4LogEn, "[Hal3A::AFThreadFunc] AfMgr::DoCallback() done\n");

        if(_this->mbPreStop)
        {
            MY_LOGD("[onAFThreadLoop] mbPreStop done\n");
            break;
        }

        if(_this->mpAFEventIrq->waitDequeue(BUF_AFO) >= 0)
        {
            if(!_this->mbAFThreadLoop || _this->mbPreStop)
                break;

            ReqQ_T rReq;
            MBOOL bIsAFStart = MFALSE;
            MBOOL bIsAFEnd = MFALSE;
            MBOOL bIsAFTrigger = MFALSE;
            MBOOL bIsPrecapTrigger = MFALSE;

            while(_this->getRequest(rReq))
            {
                MY_LOGD_IF(_this->mu4LogEn, "[%s] u4ReqMagicNum(%d) u4StatMagicNum(%d) \n", __FUNCTION__, rReq.rParam.u4MagicNum, rReq.rParam.u4MagicNumCur);

                switch(rReq.eCmd)
                {
                case ECmd_AFTrigger:
                    bIsAFTrigger = MTRUE;
                    break;
                case ECmd_AFUpdate:
                    switch (rReq.rParam.u1AfTrig)
                    {
                    case MTK_CONTROL_AF_TRIGGER_START:
                        bIsAFStart = 1;
                        bIsAFEnd = 0;
                        break;
                    case MTK_CONTROL_AF_TRIGGER_CANCEL:
                        bIsAFStart = 0;
                        bIsAFEnd = 1;
                        break;
                    default:
                        bIsAFStart = 0;
                        bIsAFEnd = 0;
                        break;
                    }
                    switch(rReq.rParam.u1PrecapTrig)
                    {
                    case MTK_CONTROL_AE_PRECAPTURE_TRIGGER_START:
                        bIsPrecapTrigger = 1;
                        break;
                    case MTK_CONTROL_AE_PRECAPTURE_TRIGGER_CANCEL:
                        bIsPrecapTrigger = 0;
                        break;
                    }

                    _this->mLastMagicNumber = rReq.rParam.u4MagicNum;

                    IAfMgr::getInstance().SetCurFrmNum(_this->mi4SensorDev, _this->mLastMagicNumber, rReq.rParam.u4MagicNumCur);
                    if(rReq.rParam.bEnable3ASetParams)
                    {
                       IAfMgr::getInstance().setAFMode(_this->mi4SensorDev, rReq.rParam.u4AfMode);
                    }
                    IAfMgr::getInstance().SetCropRegionInfo(_this->mi4SensorDev,
                                                            rReq.rParam.rScaleCropArea.i4Left,
                                                            rReq.rParam.rScaleCropArea.i4Top,
                                                            rReq.rParam.rScaleCropArea.i4Right - rReq.rParam.rScaleCropArea.i4Left,
                                                            rReq.rParam.rScaleCropArea.i4Bottom- rReq.rParam.rScaleCropArea.i4Top);

                    IAfMgr::getInstance().setAFArea(_this->mi4SensorDev, rReq.rParam.rFocusAreas);

                    IAfMgr::getInstance().SetPauseAF(_this->mi4SensorDev, rReq.rParam.u1AfPause);

                    // update af state
                    if (rReq.rParam.u4AfMode == MTK_CONTROL_AF_MODE_OFF ||
                         rReq.rParam.u4AfMode == MTK_CONTROL_AF_MODE_CONTINUOUS_PICTURE||
                         rReq.rParam.u4AfMode == MTK_CONTROL_AF_MODE_CONTINUOUS_VIDEO)
                    {
                        EState_T eCurrState = _this->mpAfStateMgr->getStateStatus().eCurrState;
                        if(eCurrState == eState_TAF)
                            _this->mpAfStateMgr->transitState(eCurrState,eState_CAF);
                    } else
                    {
                        EState_T eCurrState = _this->mpAfStateMgr->getStateStatus().eCurrState;
                        if(eCurrState == eState_CAF)
                            _this->mpAfStateMgr->transitState(eCurrState,eState_TAF);
                    }

                    if (rReq.rParam.u4AfMode == MTK_CONTROL_AF_MODE_OFF)
                    {
                        // focus dist
                        IAfMgr::getInstance().setFocusDistance(_this->mi4SensorDev, rReq.rParam.fFocusDistance);
                    }
                    break;
                }
                if(bIsAFStart)
                {
                    if(bIsPrecapTrigger)
                    {
                        MY_LOGD("[%s] WaitTriggerAF(%d) - (%d)\n", __FUNCTION__, _this->mLastMagicNumber, rReq.rParam.u4MagicNumCur);
                        IAfMgr::getInstance().WaitTriggerAF(_this->mi4SensorDev,MTRUE);
                    }
                    _this->mpAfStateMgr->sendCmd(ECmd_AFStart);
                }
                else if(bIsAFEnd)
                {
                    _this->mpAfStateMgr->sendCmd(ECmd_AFEnd);
                }

                // AF trigger when AE is stable
                if(bIsAFTrigger)
                    IAfMgr::getInstance().triggerAF(_this->mi4SensorDev);
            }

            MY_LOGD_IF(_this->mu4LogEn, "[Hal3A::AFThreadFunc] AF waitIrq done\n");
            AAA_TRACE_D("AFUpdate");
            _this->mpAfStateMgr->sendCmd(ECmd_AFUpdate);
            AAA_TRACE_END_D;

            MY_LOGD_IF(_this->mu4LogEn, "[Hal3A::AFThreadFunc] StateMgr::sendCmd(ECmd_AFUpdate) done\n");
        }
        else
        {
            //CAM_LOGE("[AFThread] AF irq timeout or error\n");
            IAfMgr::getInstance().TimeOutHandle(_this->mi4SensorDev);
            usleep(1000);
        }
    }
    MY_LOGD("[%s] [AFThread] End - \n", __FUNCTION__);

    return NULL;
}

/******************************************************************************
* Enable AF thread when previewStart.
*******************************************************************************/
MVOID
ThreadRawSMVRImp::enableAFThread(AfStateMgr* pAfStateMgr)
{
    if (mbAFThreadLoop== 0)
    {
        MY_LOGD("[%s] +", __FUNCTION__);

        mpAfStateMgr = pAfStateMgr;
        mbAFThreadLoop= 1;
        mbPreStop = MFALSE;

        // set AF irq for Vsync signal
        IEventIrq::ConfigParam IrqConfig(mi4SensorDev, mi4SensorIdx, 5000000, IEventIrq::E_Event_Af);
        mpAFEventIrq = IEventIrq::createInstance(IrqConfig, "AFThread");
        IEventIrq::ConfigParam IrqConfig_AE(mi4SensorDev, mi4SensorIdx, 5000, IEventIrq::E_Event_Vsync_Sensor);
        m_pEvent = IEventIrq::createInstance(IrqConfig_AE, "VS_DELAY");

        // create AF thread
        MY_LOGD("[%s][AFThread] Create", __FUNCTION__);
        pthread_attr_t const attr = {0, NULL, 1024 * 1024, 4096, SCHED_OTHER, NICE_CAMERA_AF};
        MINT32 result = pthread_create(&mAFThread, &attr, onAFThreadLoop, this);
        if(result != 0)
          CAM_LOGE("[%s] result(%d)", __FUNCTION__, result);

        MY_LOGD("[%s] -", __FUNCTION__);
    } else
    {
        MY_LOGD("[AFThread] AF is working");
    }
}

/******************************************************************************
* Disable AF thread when previewEnd.
*******************************************************************************/
MVOID
ThreadRawSMVRImp::disableAFThread()
{
    if (mbAFThreadLoop == 1)
    {
        MY_LOGD("[%s] +", __FUNCTION__);
        mbAFThreadLoop = 0;

        // destroy AF irq
        mpAFEventIrq->flush();
        mpAFEventIrq->destroyInstance("AFIrq");
        mpAFEventIrq = NULL;
        // destroy AE irq
        m_pEvent->flush();
        m_pEvent->destroyInstance("VS_DELAY");
        m_pEvent = NULL;

        MY_LOGD("[AFThread] Wait for pthread_join");
        pthread_join(mAFThread, NULL);

        MY_LOGD("[%s] -", __FUNCTION__);
    } else
    {
        MY_LOGD("[AFThread] AF isn't working");
    }
}

/******************************************************************************
*
*******************************************************************************/
MVOID
ThreadRawSMVRImp::
notifyPreStop()
{
    MY_LOGD("[%s] mbPreStop(%d)", __FUNCTION__, mbPreStop);
    mbPreStop = MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
ThreadRawSMVRImp::
sem_wait_rt(sem_t *pSem, nsecs_t reltime, const char* info)
{
    MY_LOGD_IF(mu4LogEn, "[%s] pSem(%p), reltime(%lld), info(%s)\n", __FUNCTION__, pSem, reltime, info);
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
        CAM_LOGE("error in clock_gettime! Please check\n");

    ts.tv_sec  += reltime/1000000000;
    ts.tv_nsec += reltime%1000000000;
    if (ts.tv_nsec >= 1000000000)
    {
        ts.tv_nsec -= 1000000000;
        ts.tv_sec += 1;
    }
    int s = sem_timedwait(pSem, &ts);
    if (s == -1)
    {
        if (errno == ETIMEDOUT)
        {
            CAM_LOGE("[%s][%s]sem_timedwait() timed out\n", __FUNCTION__, info);
            return MFALSE;
        }
        else
            CAM_LOGE("[%s][%s]sem_timedwait() errno = %d\n", __FUNCTION__, info, errno);
    }
    else
        MY_LOGD_IF(mu4LogEn, "[%s][%s]sem_timedwait() succeeded\n", __FUNCTION__, info);

    return MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
ThreadRawSMVRImp::
sendRequest(ECmd_T const r3ACmd, MUINTPTR const i4Arg)
{
    IThreadRaw::ReqQ_T rReq;
    rReq.eCmd = r3ACmd;
    if (i4Arg != 0)
        rReq.rParam = *reinterpret_cast<AFParam_T*>(i4Arg);
    MY_LOGD_IF(mu4LogEn, "[%s] r3ACmd(%d)\n", __FUNCTION__, r3ACmd);

    addReqestQ(rReq);
    return MTRUE;
}

MVOID
ThreadRawSMVRImp::
addReqestQ(IThreadRaw::ReqQ_T &rReq)
{
    Mutex::Autolock autoLock(mModuleMtx);

    mReqQ.push_back(rReq);

    MY_LOGD_IF(mu4LogEn, "[%s] mReqQ size(%d) + cmd(%d) magic(%d)", __FUNCTION__, mReqQ.size(), rReq.eCmd, rReq.rParam.u4MagicNum);
}

MVOID
ThreadRawSMVRImp::
clearReqestQ()
{
    MY_LOGD("[%s]+", __FUNCTION__);
    Mutex::Autolock autoLock(mModuleMtx);

    int Qsize = mReqQ.size();

    for (List<ReqQ_T>::iterator it = mReqQ.begin(); it != mReqQ.end();)
    {
        ECmd_T eCmd = it->eCmd;
        MY_LOGD("[%s] cmd size(%d), clear(%d)", __FUNCTION__, mReqQ.size(), eCmd);
        it = mReqQ.erase(it);
    }
    MY_LOGD("[%s]- Qsize(%d)", __FUNCTION__, Qsize);
}

MBOOL
ThreadRawSMVRImp::
getRequest(IThreadRaw::ReqQ_T &rReq, MINT32 i4Count)
{
    MY_LOGD_IF(mu4LogEn, "[%s] +", __FUNCTION__);
    Mutex::Autolock autoLock(mModuleMtx);
    MY_LOGD_IF(mu4LogEn, "[%s] mCmdQ.size()=%d ", __FUNCTION__, mReqQ.size());

    if (mReqQ.size() == 0)
    {
        return MFALSE;
    }

    rReq = *mReqQ.begin();
    MY_LOGD_IF(mu4LogEn, "mReqQ-size(%d), eCmd(%d)", mReqQ.size(), rReq.eCmd);

    mReqQ.erase(mReqQ.begin());
    MY_LOGD_IF(mu4LogEn, "[%s] -", __FUNCTION__);

    return MTRUE;
}

/******************************************************************************
* Enable AF thread when previewStart.
*******************************************************************************/
MVOID
ThreadRawSMVRImp::enableAFVsyncThread()
{
    if (mbAFVsyncThreadLoop== 0)
    {
        MY_LOGD("[%s] +", __FUNCTION__);

        mbAFVsyncThreadLoop= 1;

        // set AF irq for Vsync signal
        IEventIrq::ConfigParam IrqConfig(mi4SensorDev, mi4SensorIdx, 5000000, IEventIrq::E_Event_Vsync);
        mpAFVsyncEventIrq = IEventIrq::createInstance(IrqConfig, "AFVsyncThread");

        // create AF Sensor thread
        MY_LOGD("[%s][AFVsyncThread] Create", __FUNCTION__);
        pthread_attr_t const attr = {0, NULL, 1024 * 1024, 4096, SCHED_OTHER, NICE_CAMERA_AF};
        MINT32 result = pthread_create(&mAFVsyncThread, &attr, onAFVsyncThreadLoop, this);
        if(result != 0)
          CAM_LOGE("[%s] result(%d)", __FUNCTION__, result);

        MY_LOGD("[%s] -", __FUNCTION__);
    } else
    {
        MY_LOGD("[AFVsyncThread] AF is working");
    }
}

/******************************************************************************
* Disable AF thread when previewEnd.
*******************************************************************************/
MVOID
ThreadRawSMVRImp::disableAFVsyncThread()
{
    if (mbAFVsyncThreadLoop == 1)
    {
        MY_LOGD("[%s] +", __FUNCTION__);
        mbAFVsyncThreadLoop = 0;

        // destroy AF irq
        mpAFVsyncEventIrq->flush();

        MY_LOGD("[AFVsyncThread] Wait for pthread_join");
        pthread_join(mAFVsyncThread, NULL);

        mpAFVsyncEventIrq->destroyInstance("AFVsyncThread");
        mpAFVsyncEventIrq = NULL;

        MY_LOGD("[%s] -", __FUNCTION__);
    } else
    {
        MY_LOGD("[AFVsyncThread] AF isn't working");
    }
}

/******************************************************************************
* change AF thread setting
*******************************************************************************/
MVOID
ThreadRawSMVRImp::changeAFVsyncThreadSetting()
{
    // (1) set name
    ::prctl(PR_SET_NAME,"AFVsyncThread", 0, 0, 0);

    // (2) set policy/priority
    {
        int const expect_policy     = SCHED_OTHER;
        int const expect_priority   = NICE_CAMERA_AF;
        int policy = 0, priority = 0;
        setThreadPriority(expect_policy, expect_priority);
        getThreadPriority(policy, priority);
        //
        MY_LOGD(
            "[changeAFThreadSetting] policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d)"
            , expect_policy, policy, expect_priority, priority
        );
    }

}

/******************************************************************************
* AF thread execution function
*******************************************************************************/
MVOID*
ThreadRawSMVRImp::onAFVsyncThreadLoop(MVOID *arg)
{
    MY_LOGD("[%s] +" ,__FUNCTION__);
    MY_LOGD("[AFVsyncThread] tid: %d \n", gettid());

    ThreadRawSMVRImp *_this = reinterpret_cast<ThreadRawSMVRImp*>(arg);
    IEventIrq::Duration duration;
    _this->changeAFVsyncThreadSetting();

    while (_this->mbAFVsyncThreadLoop) {
        if ( _this->mpAFVsyncEventIrq->wait(duration) >= 0) // success
        {
            AAA_TRACE_D("VsyncUpdate");
            IAfMgr::getInstance().VsyncUpdate(_this->mi4SensorDev);
            AAA_TRACE_END_D;
        }
        else
        {
            CAM_LOGE("[AFThread] AF irq timeout or error\n");
            IAfMgr::getInstance().TimeOutHandle(_this->mi4SensorDev);
        }
    }
    MY_LOGD("[AFVsyncThread] End \n");
    MY_LOGD("[%s] -" ,__FUNCTION__);

    return NULL;
}
