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
#define LOG_TAG "Hal3ARaw_thread"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG   (1)
#endif
#define LOG_PERFRAME_ENABLE (1)

#include <utils/threads.h>
#include <utils/List.h>
#include <mtkcam/def/PriorityDefs.h>
#include <mtkcam/utils/std/common.h>
#include <mtkcam/utils/std/Trace.h>

#include <pthread.h>
#include <semaphore.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <IThreadRaw.h>

#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include <aaa/af_feature.h>
#include <pd_buf_mgr/pd_buf_mgr.h>
#include <ae_mgr/ae_mgr_if.h>
#include <af_mgr/af_mgr.h>
#include <af_mgr/af_mgr_if.h>
#include <flash_mgr/flash_mgr.h>
#include <IEventIrq.h>
#include <debug/DebugUtil.h>
#include <aaa_hal_sttCtrl.h>

#include <cutils/properties.h>
#include <cutils/atomic.h>

#include <isp_mgr_af_stat.h>
#include <af_define.h>


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
class ThreadRawImp : public IThreadRaw
{
public:
    //
    static IThreadRaw* createInstance(I3AWrapper* pHal3A, MINT32 iSensorDev, MINT32 iSensorIdx);
    virtual MBOOL destroyInstance();
    virtual MVOID postToAESenThread();
    virtual MVOID enableAFThread(AfStateMgr* pAfStateMgr);
    virtual MVOID disableAFThread();
    virtual MVOID pauseAFThread();
    virtual MVOID resumeAFThread();

    virtual MBOOL sendRequest(ECmd_T const eCmd, MUINTPTR const i4Arg = 0);
    virtual MVOID notifyPreStop();
    /**
    * @brief create AE thread
    */
    virtual MVOID createAEThread();
    /**
    * @brief destroy AE thread
    */
    virtual MVOID destroyAEThread();

protected: //private:
    ThreadRawImp(){}
    ThreadRawImp(I3AWrapper* pHal3A, MINT32 iSensorDev, MINT32 iSensorIdx);
    virtual ~ThreadRawImp();

    ThreadRawImp(const ThreadRawImp&);
    ThreadRawImp& operator=(const ThreadRawImp&);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  AE Thread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /**
    * @brief change AE thread setting
    */
    virtual MVOID changeAEThreadSetting();
    /**
    * @brief AE thread execution function
    */
    static  MVOID*  onAEThreadLoop(MVOID*);

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
    * @param [in] AF requests; please refer to IThreadRaw.h
    */
    virtual MVOID addReqestQ(IThreadRaw::ReqQ_T &rReq);
    /**
    * @brief clear all requests in current request queue
    */
    virtual MVOID clearReqestQ();
    /**
    * @brief get AF request from the head of AF request queue
    * @param [in] AF requests; please refer to IThreadRaw.h
    */
    virtual MBOOL getRequest(IThreadRaw::ReqQ_T &rReq, MINT32 i4Count = 0);

    MBOOL sem_wait_rt(sem_t *pSem, nsecs_t reltime, const char* info);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected: //private:
    I3AWrapper*     mpHal3A;
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
    // AF Thread
    MUINT32         mLastMagicNumber;
    IEventIrq*      mpAFEventIrq;
    pthread_t       mAFThread;
    MINT32          mbAFThreadLoop;
    AfStateMgr*     mpAfStateMgr;
    List<ReqQ_T>    mReqQ;
    Condition       mCmdQCond;
    MBOOL           mbAFPause;
    sem_t           semAFPause;
};

/*******************************************************************************
* utilities
********************************************************************************/
MUINT32 ThreadRawImp::mu4DebugLogWEn = 0;

IThreadRaw*
IThreadRaw::createInstance(I3AWrapper* pHal3A, MINT32 iSensorDev, MINT32 iSensorIdx)
{
    return ThreadRawImp::createInstance(pHal3A, iSensorDev, iSensorIdx);
}

IThreadRaw*
ThreadRawImp::
createInstance(I3AWrapper* pHal3A, MINT32 iSensorDev, MINT32 iSensorIdx)
{
    ThreadRawImp* pObj = new ThreadRawImp(pHal3A, iSensorDev, iSensorIdx);
    return pObj;
}

MBOOL
ThreadRawImp::
destroyInstance()
{
    delete this;
    return MTRUE;
}

ThreadRawImp::
ThreadRawImp(I3AWrapper* pHal3A, MINT32 iSensorDev, MINT32 iSensorIdx)
    : mpHal3A(pHal3A)
    , mu4LogEn(MFALSE)
    , mi4User(0)
    , mi4SensorDev(iSensorDev)
    , mi4SensorIdx(iSensorIdx)
    , mLock()
    , mModuleMtx()
    , mbPreStop(MFALSE)
    , mAESenMtx()
    , semAESen()
    , semAESenThdEnd()
    , mbEnAESenThd(MFALSE)
    , mAEThread()
    , mLastMagicNumber(0)
    , mpAFEventIrq(NULL)
    , mAFThread()
    , mbAFThreadLoop(0)
    , mpAfStateMgr(NULL)
    , mbAFPause(MFALSE)
    , semAFPause()
{
    GET_PROP("vendor.debug.thread_raw.log", 0, mu4LogEn);
    mu4DebugLogWEn = DebugUtil::getDebugLevel(DBG_3A);
    MY_LOGD("[%s] mi4SensorDev(%d) mi4SensorIdx(%d)", __FUNCTION__, mi4SensorDev, mi4SensorIdx);
}

ThreadRawImp::
~ThreadRawImp()
{
    MY_LOGD("[%s]", __FUNCTION__);
}

/******************************************************************************
* create AE thread
*******************************************************************************/
MVOID
ThreadRawImp::createAEThread()
{
    Mutex::Autolock lock(mLock);

    if (mi4User > 0)
    {
        MY_LOGD_IF(mu4LogEn, "[%s] mi4User(%d)", __FUNCTION__, mi4User);
    }
    else
    {
        MY_LOGD("[%s] +", __FUNCTION__);
        mbPreStop = MFALSE;
        mbEnAESenThd = MTRUE;
        sem_init(&semAESen, 0, 0);
        sem_init(&semAESenThdEnd, 0, 1);
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
ThreadRawImp::destroyAEThread()
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
ThreadRawImp::changeAEThreadSetting()
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
ThreadRawImp::onAEThreadLoop(MVOID *arg)
{
    MY_LOGD("[%s] +" ,__FUNCTION__);
    // (1) change thread setting
    ThreadRawImp *_this = reinterpret_cast<ThreadRawImp*>(arg);
    _this->changeAEThreadSetting();

    // (2) thread-in-loop
    while(1)
    {
        ::sem_wait(&_this->semAESen);
        if ( ! _this->mbEnAESenThd || _this->mbPreStop) break;

        MY_LOGD_IF(_this->mu4LogEn, "[AESensorThreadLoop] updateSensorbyI2C\n");
        CAM_TRACE_BEGIN("AE Sensor I2C");
        IAeMgr::getInstance().updateSensorbyI2C(_this->mi4SensorDev);
        CAM_TRACE_END();

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
ThreadRawImp::postToAESenThread()
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
ThreadRawImp::changeAFThreadSetting()
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
ThreadRawImp::onAFThreadLoop(MVOID *arg)
{
    MY_LOGD("[%s] +" ,__FUNCTION__);
    MY_LOGD("[AFThread] tid: %d \n", gettid());

    ThreadRawImp *_this = reinterpret_cast<ThreadRawImp*>(arg);
    IEventIrq::Duration duration;
    _this->changeAFThreadSetting();

    // StartPreview speed up
    IAfMgr::getInstance().AFThreadStart(_this->mi4SensorDev);
    
    FSC_DAC_INIT_INFO_T FSCInitInfo;
    IAfMgr::getInstance().sendAFCtrl(_this->mi4SensorDev, EAFMgrCtrl_GetFSCInitInfo, reinterpret_cast<MINTPTR>(&FSCInitInfo), 0);
    MY_LOGD_IF(_this->mu4LogEn, "3AHal GetFSCInitInfo %d,%d,%d,%d,%d,%d,%d,%d",
            FSCInitInfo.macro_To_Inf_Ratio,
            FSCInitInfo.dac_Inf,
            FSCInitInfo.dac_Macro,
            FSCInitInfo.damping_Time,
            FSCInitInfo.readout_Time_us,
            FSCInitInfo.init_DAC,
            FSCInitInfo.af_Table_Start,
            FSCInitInfo.af_Table_End);

    MINT32 IsAFSupport = MTRUE;
    IsAFSupport = IAfMgr::getInstance().isAFSupport(_this->mi4SensorDev);

    Hal3ASttCtrl* p3ASttCtrl = NULL;
    IBufMgr* pAFOBufMgr = NULL;

    while (_this->mbAFThreadLoop && IsAFSupport) {

        MY_LOGD_IF(_this->mu4LogEn, "[Hal3A::AFThreadFunc] AfMgr::DoCallback() done\n");

        if(_this->mbPreStop)
        {
            MY_LOGD("[onAFThreadLoop] mbPreStop done\n");
            break;
        }

        p3ASttCtrl = Hal3ASttCtrl::getInstance(_this->mi4SensorDev);
        if(p3ASttCtrl)
            pAFOBufMgr = p3ASttCtrl->getBufMgr(BUF_AFO);

        if (pAFOBufMgr)
        {
            if(pAFOBufMgr->dequeueHwBuf() >= 0)
            {
                if(!_this->mbAFThreadLoop || _this->mbPreStop)
                    break;
                ReqQ_T rReq;
                MBOOL bIsAFStart = MFALSE;
                MBOOL bIsAFEnd = MFALSE;
                MBOOL bIsAFTrigger = MFALSE;
                MBOOL bIsPrecapTrigger = MFALSE;
                MINT32 i4RequestCount = 0;

                MY_LOGD_IF(_this->mu4LogEn, "[Hal3A::AFThreadFunc] AF waitIrq done\n");
                CAM_TRACE_BEGIN("AFUpdate");
                _this->mpAfStateMgr->sendCmd(ECmd_AFUpdate);
                CAM_TRACE_END();

                while (_this->getRequest(rReq, i4RequestCount))
                {
                    i4RequestCount++;
                    MY_LOGD_IF(_this->mu4LogEn, "[%s] u4MagicNum(%d) \n", __FUNCTION__, rReq.rParam.u4MagicNum);

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

                        IAfMgr::getInstance().SetCurFrmNum(_this->mi4SensorDev, _this->mLastMagicNumber);
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

                        // Bypass SetPauseAF when in flash-calibration
                        if(FlashMgr::getInstance().isFlashOnCalibration(_this->mi4SensorDev) != 1)
                            IAfMgr::getInstance().SetPauseAF(_this->mi4SensorDev, rReq.rParam.u1AfPause);

                        IAfMgr::getInstance().setMultiZoneEnable(_this->mi4SensorDev, rReq.rParam.u1MZOn);

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
                    default:
                        break;
                    }

                    if(bIsAFStart)
                    {
                        if(bIsPrecapTrigger)
                        {
                            MY_LOGD("[%s] WaitTriggerAF(%d) \n", __FUNCTION__, _this->mLastMagicNumber);
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

                AF2AEInfo_T AF2AEinfo;
                IAfMgr::getInstance().getAF2AEInfo(_this->mi4SensorDev, AF2AEinfo);
                AF2AEinfo.i4MagicNum = _this->mpHal3A->queryMagicNumber()+1;    // +1 for AE of the next frame
                _this->mpHal3A->send3ACtrl(E3ACtrl_GetAF2AEInfo, reinterpret_cast<MINTPTR>(&AF2AEinfo), NULL);

                MY_LOGD_IF(_this->mu4LogEn, "[Hal3A::AFThreadFunc] StateMgr::sendCmd(ECmd_AFUpdate) done\n");

                p3ASttCtrl = Hal3ASttCtrl::getInstance(_this->mi4SensorDev);
                if (p3ASttCtrl)
                    pAFOBufMgr = p3ASttCtrl->getBufMgr(BUF_AFO);
                if(_this->mbPreStop)
                    break;
                if (pAFOBufMgr)
                    pAFOBufMgr->enqueueHwBuf();
                
                //get AF FSC INFO BUFFER
                FSC_FRM_INFO_T FSCInfo;
                IAfMgr::getInstance().sendAFCtrl(_this->mi4SensorDev, EAFMgrCtrl_GetFSCInfo, reinterpret_cast<MINTPTR>(&FSCInfo), 0);
                MY_LOGD_IF(_this->mu4LogEn, "3AHal FSC SttNum %d, SetCount %d", FSCInfo.SttNum, FSCInfo.SetCount);
                for(int i=0;i<FSCInfo.SetCount;i++)
                {
                    MY_LOGD_IF(_this->mu4LogEn, "3AHal GetFSCInfo Set %d : DAC (%d)->(%d), Percent (%d)",
                             i,
                             FSCInfo.DACInfo[i].DAC_From,
                             FSCInfo.DACInfo[i].DAC_To,
                             FSCInfo.DACInfo[i].Percent);
                }
                if(_this->mpHal3A)
                    _this->mpHal3A->send3ACtrl(E3ACtrl_NOTIFY_AF_FSC_INFO, reinterpret_cast<MINTPTR>(&FSCInfo), 0);
                else
                    CAM_LOGE("[%s] mpHal3A NULL", __FUNCTION__);
            }
            else
            {
                //MY_ERR("[AFThread] AF irq timeout or error\n");
                IAfMgr::getInstance().TimeOutHandle(_this->mi4SensorDev);
                usleep(1000);
            }
        }

        if(_this->mbAFPause)
        {
            CAM_LOGD("Pause +");
            ::sem_wait(&_this->semAFPause);
            CAM_LOGD("Pause -");
        }
    }

    if (!IsAFSupport)
    {
        CAM_LOGW("[%s] disable AF", __FUNCTION__);
    }

    MY_LOGD("[AFThread] End \n");
    MY_LOGD("[%s] -" ,__FUNCTION__);

    return NULL;
}

/******************************************************************************
* Enable AF thread when previewStart.
*******************************************************************************/
MVOID
ThreadRawImp::enableAFThread(AfStateMgr* pAfStateMgr)
{
    if (mbAFThreadLoop== 0)
    {
        MY_LOGD("[%s] +", __FUNCTION__);

        mpAfStateMgr = pAfStateMgr;
        mbAFThreadLoop= 1;
        mbPreStop = MFALSE;

        mbAFPause = MFALSE;
        sem_init(&semAFPause, 0, 0);

        // set AF irq for Vsync signal
        IEventIrq::ConfigParam IrqConfig(mi4SensorDev, mi4SensorIdx, 5000000, IEventIrq::E_Event_Af);
        mpAFEventIrq = IEventIrq::createInstance(IrqConfig, "AFThread");

        // create AF thread
        MY_LOGD("[%s][AFThread] Create", __FUNCTION__);

        pthread_attr_t attr;
        struct sched_param pthread_param = {
            .sched_priority = NICE_CAMERA_AF
        };

        pthread_attr_init(&attr);
        pthread_attr_setstack(&attr, NULL, 1024*1024);
        pthread_attr_setguardsize(&attr, 4096);
        pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
        pthread_attr_setschedparam(&attr, &pthread_param);
        MY_LOGD("pthread attr has flags = %d", attr.flags);
        //pthread_attr_t const attr = {0, NULL, 1024 * 1024, 4096, SCHED_OTHER, NICE_CAMERA_AF};
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
ThreadRawImp::disableAFThread()
{
    if (mbAFThreadLoop == 1)
    {
        MY_LOGD("[%s] +", __FUNCTION__);
        mbAFThreadLoop = 0;

        resumeAFThread();

        // destroy AF irq
        mpAFEventIrq->flush();

        MY_LOGD("[AFThread] Wait for pthread_join");
        pthread_join(mAFThread, NULL);

        mpAFEventIrq->destroyInstance("AFIrq");
        mpAFEventIrq = NULL;

        MY_LOGD("[%s] -", __FUNCTION__);
    } else
    {
        MY_LOGD("[AFThread] AF isn't working");
    }
}

/******************************************************************************
* Pause AF thread
*******************************************************************************/
MVOID
ThreadRawImp::pauseAFThread()
{
    if(!mbAFPause)
        mbAFPause = MTRUE;
}

/******************************************************************************
* Pause AF thread
*******************************************************************************/
MVOID
ThreadRawImp::resumeAFThread()
{
    if(mbAFPause)
    {
        MINT32 i4SemValue = 0;
        ::sem_getvalue(&semAFPause, &i4SemValue);
        CAM_LOGD("[%s] semAFPause(%d)", __FUNCTION__, i4SemValue);
        mbAFPause = MFALSE;
        ::sem_post(&semAFPause);
    }
}

/******************************************************************************
*
*******************************************************************************/
MVOID
ThreadRawImp::
notifyPreStop()
{
    MY_LOGD("[%s] mbPreStop(%d)", __FUNCTION__, mbPreStop);
    mbPreStop = MTRUE;
    NSIspTuningv3::ISP_MGR_AF_STAT_CONFIG_T::getInstance(static_cast<ESensorDev_T>(mi4SensorDev)).notifyPreStop();
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
ThreadRawImp::
sem_wait_rt(sem_t *pSem, nsecs_t reltime, const char* info)
{
    MY_LOGD_IF(mu4LogEn, "[%s] pSem(%p), reltime(%ld), info(%s)\n", __FUNCTION__, pSem, (long)reltime, info);
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
ThreadRawImp::
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
ThreadRawImp::
addReqestQ(IThreadRaw::ReqQ_T &rReq)
{
    Mutex::Autolock autoLock(mModuleMtx);

    mReqQ.push_back(rReq);

    while(mReqQ.size() > 10){
        mReqQ.erase(mReqQ.begin());
    }

    mCmdQCond.broadcast();

    MY_LOGD_IF(mu4LogEn, "[%s] mReqQ size(%d) + cmd(%d) magic(%d)", __FUNCTION__, (MINT32)mReqQ.size(), rReq.eCmd, (MINT32)rReq.rParam.u4MagicNum);
}

MVOID
ThreadRawImp::
clearReqestQ()
{
    MY_LOGD("[%s]+", __FUNCTION__);
    Mutex::Autolock autoLock(mModuleMtx);

    int Qsize = mReqQ.size();

    for (List<ReqQ_T>::iterator it = mReqQ.begin(); it != mReqQ.end();)
    {
        ECmd_T eCmd = it->eCmd;
        MY_LOGD("[%s] cmd size(%d), clear(%d)", __FUNCTION__, (MINT32)mReqQ.size(), eCmd);
        it = mReqQ.erase(it);
    }
    MY_LOGD("[%s]- Qsize(%d)", __FUNCTION__, Qsize);
}

MBOOL
ThreadRawImp::
getRequest(IThreadRaw::ReqQ_T &rReq, MINT32 i4Count)
{
    MY_LOGD_IF(mu4LogEn, "[%s] +", __FUNCTION__);
    Mutex::Autolock autoLock(mModuleMtx);
    MY_LOGD_IF(mu4LogEn, "[%s] mCmdQ.size()=%d ", __FUNCTION__, (MINT32)mReqQ.size());

#if 0
    if (mReqQ.size() == 0 && i4Count == 0)
    {
        MY_LOGD_IF(mu4LogEn, "[%s] mCmdQCond.wait +", __FUNCTION__);
        mCmdQCond.waitRelative(mModuleMtx, (long long int)10000000);
        MY_LOGD_IF(mu4LogEn, "[%s] mCmdQCond.wait -", __FUNCTION__);
    }
#endif
    if (mReqQ.size() == 0) //this is only for en_timeout == 1 & timeout case
    {
        MY_LOGD_IF(mu4LogEn, "[%s] mCmdQ.size() = %d after mCmdQCond.wait/waitRelative\n", __FUNCTION__, (MINT32)mReqQ.size());
        return MFALSE;
    }

    rReq = *mReqQ.begin();
    MY_LOGD_IF(mu4LogEn, "mReqQ-size(%d), eCmd(%d)", (MINT32)mReqQ.size(), rReq.eCmd);

    mReqQ.erase(mReqQ.begin());
    MY_LOGD_IF(mu4LogEn, "[%s] -", __FUNCTION__);

    return MTRUE;
}

