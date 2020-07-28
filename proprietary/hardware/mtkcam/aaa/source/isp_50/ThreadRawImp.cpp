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
#include <af_define.h>
#include <pd_mgr/pd_mgr_if.h>
#include <flash_mgr.h>
#include <IEventIrq.h>
#include <debug/DebugUtil.h>
#include <aaa_hal_sttCtrl.h>
#include <task/ITaskMgr.h>

#include <cutils/properties.h>
#include <cutils/atomic.h>
#include <IResultPool.h>

#if CAM3_FLASH_FEATURE_EN
#include <flash_hal.h>
#include <flash_mgr.h>
#endif

// For AF focus value
#include <mtkcam/utils/hw/IFVContainer.h>

#if defined(MTKCAM_CCU_AF_SUPPORT)
#include <drv/isp_reg.h>
#include "ccu_ext_interface/ccu_af_reg.h"
#include "iccu_ctrl_af.h"
#include "iccu_mgr.h"
using namespace NSCcuIf;
#endif

// systrace
#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>

#include <aaa_common_custom.h>

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
    static IThreadRaw* createInstance(Hal3ARaw* pHal3A, MINT32 iSensorDev, MINT32 iSensorIdx);
    virtual MBOOL destroyInstance();
    virtual MVOID postToEventThread(E_3AEvent_T eEvent, MINT32 opt);
    virtual MVOID enableAFThread(MVOID* pTaskMgr);
    virtual MVOID disableAFThread();
    virtual MVOID pauseAFThread();
    virtual MVOID resumeAFThread();

    virtual MBOOL sendRequest(ECmd_T const eCmd, MUINTPTR const i4Arg = 0);
    virtual MVOID notifyPreStop(MBOOL isSecureCam = MFALSE);
    /**
    * @brief create Event thread
    */
    virtual MVOID createEventThread();
    /**
    * @brief destroy AE thread
    */
    virtual MVOID destroyEventThread();

protected: //private:
    ThreadRawImp(){}
    ThreadRawImp(Hal3ARaw* pHal3A, MINT32 iSensorDev, MINT32 iSensorIdx);
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
    static  MVOID*  onEventThreadLoop(MVOID*);

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

    virtual MBOOL getCurrResult4AF(AFParam_T& AFParam);

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
    MUINT32         mbPreStop4Event;
    // Event Thread
    Mutex           mEventMtx;
    sem_t           semEvent;
    sem_t           semEventThdEnd;
    MBOOL           mbEnEventThd;
    pthread_t       mEventThread;
    E_3AEvent_T     mEvent;
    MINT32          mEventOpt;
    IEventIrq*      m_pEventIrq;
    // AF Thread
    MUINT32         mLastMagicNumber;
    IEventIrq*      mpAFEventIrq;
    pthread_t       mAFThread;
    MINT32          mbAFThreadLoop;
    ITaskMgr*       mpTaskMgr;
    List<ReqQ_T>    mReqQ;
    Condition       mCmdQCond;
    MBOOL           mbAFPause;
    sem_t           semAFPause;
    IResultPool*    m_pResultPoolObj;
    IEventIrq*      mpEventIrq;
    MBOOL           m_bDbgInfoEnable;
};

/*******************************************************************************
* utilities
********************************************************************************/
MUINT32 ThreadRawImp::mu4DebugLogWEn = 0;

IThreadRaw*
IThreadRaw::createInstance(Hal3ARaw* pHal3A, MINT32 iSensorDev, MINT32 iSensorIdx)
{
    return ThreadRawImp::createInstance(pHal3A, iSensorDev, iSensorIdx);
}

IThreadRaw*
ThreadRawImp::
createInstance(Hal3ARaw* pHal3A, MINT32 iSensorDev, MINT32 iSensorIdx)
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
ThreadRawImp(Hal3ARaw* pHal3A, MINT32 iSensorDev, MINT32 iSensorIdx)
    : mpHal3A(pHal3A)
    , mu4LogEn(MFALSE)
    , mi4User(0)
    , mi4SensorDev(iSensorDev)
    , mi4SensorIdx(iSensorIdx)
    , mLock()
    , mbPreStop(MFALSE)
    , mbPreStop4Event(MFALSE)
    , mbEnEventThd(MFALSE)
    , mEvent(E_3AEvent_NONE)
    , mEventOpt(0)
    , m_pEventIrq(NULL)
    , mLastMagicNumber(0)
    , mbAFThreadLoop(0)
	, mpTaskMgr(NULL)
    , mbAFPause(MFALSE)
    , m_pResultPoolObj(NULL)
    , mpEventIrq(NULL)
{
    MBOOL dbgInfoEnable;
#if (IS_BUILD_USER)
    dbgInfoEnable = 0;
#else
    dbgInfoEnable = 1;
#endif

    GET_PROP("vendor.debug.camera.dbginfo", dbgInfoEnable, m_bDbgInfoEnable);

    GET_PROP("vendor.debug.thread_raw.log", 0, mu4LogEn);
    mu4DebugLogWEn = DebugUtil::getDebugLevel(DBG_3A);
    MY_LOGD("[%s] mi4SensorDev(%d) mi4SensorIdx(%d)", __FUNCTION__, mi4SensorDev, mi4SensorIdx);

    if(m_pResultPoolObj == NULL)
        m_pResultPoolObj = IResultPool::getInstance(mi4SensorDev);
    if(m_pResultPoolObj == NULL)
        MY_LOGE("ResultPool getInstance fail");
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
ThreadRawImp::createEventThread()
{
    Mutex::Autolock lock(mLock);

    if (mi4User > 0)
    {
        MY_LOGD_IF(mu4LogEn, "[%s] mi4User(%d)", __FUNCTION__, mi4User);
    }
    else
    {
        MY_LOGD("[%s] +", __FUNCTION__);

        if(m_pEventIrq == NULL)
        {
            IEventIrq::ConfigParam IrqConfig(mi4SensorDev, mi4SensorIdx, 5000000, IEventIrq::E_Event_P1_Done);
            m_pEventIrq = IEventIrq::createInstance(IrqConfig, "Sw_P1_Done");
        }

        if (mpEventIrq == NULL)
        {
            IEventIrq::ConfigParam IrqConfig(mi4SensorDev, mi4SensorIdx, 5000000, IEventIrq::E_Event_Vsync);
            mpEventIrq = IEventIrq::createInstance(IrqConfig, "EventThreadVSIrq");
        }

        mbEnEventThd = MTRUE;
        mbPreStop4Event = MFALSE;
        sem_init(&semEvent, 0, 0);
        sem_init(&semEventThdEnd, 0, 1);
        MINT32 result = pthread_create(&mEventThread, NULL, onEventThreadLoop, this);
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
ThreadRawImp::destroyEventThread()
{
    Mutex::Autolock lock(mLock);

    if (mi4User > 0)
    {
        // More than one user, so decrease one User.
        android_atomic_dec(&mi4User);

        if (mi4User == 0) // There is no more User after decrease one User
        {
            MY_LOGD("[%s] +", __FUNCTION__);
            if(m_pEventIrq)
            {
                m_pEventIrq->flush();
                m_pEventIrq->destroyInstance("Sw_P1_Done");
                m_pEventIrq = NULL;
            }

            if (mpEventIrq)
            {
                mpEventIrq->flush();
                mpEventIrq->destroyInstance("EventThreadVSIrq");
                mpEventIrq = NULL;
            }

            mbEnEventThd = MFALSE;
            ::sem_post(&semEvent);
            pthread_join(mEventThread, NULL);
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
    ::prctl(PR_SET_NAME, "3AEventThd", 0, 0, 0);

    // (2) set policy/priority
    {
        int const expect_policy     = SCHED_OTHER;
        int const expect_priority   = NICE_CAMERA_3A_MAIN;
        int policy = 0, priority = 0;
        setThreadPriority(expect_policy, expect_priority);
        getThreadPriority(policy, priority);
        //
        MY_LOGD(
            "[Hal3AFlowCtrl::onEventThreadLoop] policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d)"
            , expect_policy, policy, expect_priority, priority
        );
    }

}

/******************************************************************************
* Event thread execution function
*******************************************************************************/
MVOID*
ThreadRawImp::onEventThreadLoop(MVOID *arg)
{
    MY_LOGD("[%s] +" ,__FUNCTION__);
    // (1) change thread setting
    ThreadRawImp *_this = reinterpret_cast<ThreadRawImp*>(arg);
    _this->changeAEThreadSetting();

    IEventIrq::Duration duration;

    // (2) thread-in-loop
    while(1)
    {
        ::sem_wait(&_this->semEvent);
        if ( ! _this->mbEnEventThd || _this->mbPreStop4Event) break;

        if(_this->mEvent == E_3AEvent_AE_I2C){
            MY_LOGD_IF(_this->mu4LogEn, "[onEventThreadLoop] updateSensorbyI2C\n");

            if(cust_getIsSpecialLongExpOn())
            {
                IEventIrq::Duration duration;
                CAM_LOGI("[%s] start waitVSirq. (Special Long Exp)", __FUNCTION__);
                _this->mpEventIrq->wait(duration);
            }

            CAM_TRACE_BEGIN("AE Sensor I2C");
            IAeMgr::getInstance().updateSensorbyI2C(_this->mi4SensorDev);
            CAM_TRACE_END();
        } else if(_this->mEvent == E_3AEvent_Flash_On){
            if(_this->m_pEventIrq)
                _this->m_pEventIrq->wait(duration);
            FlashHal::getInstance(_this->mi4SensorDev)->setOnOff(MTRUE, static_cast<FLASH_HAL_SCENARIO_ENUM>(_this->mEventOpt));
        } else if(_this->mEvent == E_3AEvent_Flash_Off){
            if(_this->m_pEventIrq)
                _this->m_pEventIrq->wait(duration);
            FlashHal::getInstance(_this->mi4SensorDev)->setOnOff(MFALSE, static_cast<FLASH_HAL_SCENARIO_ENUM>(_this->mEventOpt));
        }

        //sync with 3A thread when PreviewEnd
        {
            int Val;
            MY_LOGD_IF(_this->mu4LogEn, "[Hal3A::onEventThreadLoop] start waiting mEventMtx lock (3)\n");
            Mutex::Autolock autoLock(_this->mEventMtx);
            MY_LOGD_IF(_this->mu4LogEn, "[Hal3A::onEventThreadLoop] get mEventMtx lock (3)\n");

            ::sem_getvalue(&_this->semEventThdEnd, &Val);
            MY_LOGD_IF(_this->mu4LogEn, "[onEventThreadLoop] semEventThdEnd before post = %d\n", Val);
            if (Val == 0) ::sem_post(&_this->semEventThdEnd); //to be 1, 1 means AE set Sensor done, this can tolerate I2C delay too long
        }
    }

    MY_LOGD("[%s] -" ,__FUNCTION__);

    return NULL;
}

/******************************************************************************
* trigger AE thread to updateSensorByI2C
*******************************************************************************/
MVOID
ThreadRawImp::postToEventThread(E_3AEvent_T eEvent, MINT32 opt)
{
    Mutex::Autolock autoLock(mEventMtx);
    int Val;
    ::sem_getvalue(&semEventThdEnd, &Val);
    if (Val == 1) ::sem_wait(&semEventThdEnd); //to be 0, it won't block, 0 means AE set Sensor not ready yet, this can tolerate I2C delay too long
    mEvent = eEvent;
    mEventOpt = opt;
    ::sem_post(&semEvent);
    MY_LOGD_IF(mu4LogEn, "[%s] Event(%d) Opt(%d) sem_post semEvent, semEventThdEnd before wait = %d\n", __FUNCTION__, mEvent, mEventOpt, Val);
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

    MINT32 IsSupportAF = MTRUE;
    IsSupportAF = IAfMgr::getInstance().isAFSupport(_this->mi4SensorDev);

    // create FVContainer
    auto fvWriter = IFVContainer::createInstance(LOG_TAG, IFVContainer::eFVContainer_Opt_Write);

    while (_this->mbAFThreadLoop && IsSupportAF) {

        MY_LOGD_IF(_this->mu4LogEn, "[Hal3A::AFThreadFunc] AfMgr::DoCallback() done\n");

        if(_this->mbPreStop)
        {
            MY_LOGD("[onAFThreadLoop] mbPreStop done\n");
            break;
        }

        if(_this->mpTaskMgr->dequeStt(Task_Update_AF) >= 0)
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

            while (_this->getRequest(rReq, i4RequestCount))
            {
                i4RequestCount++;
                MY_LOGD_IF(_this->mu4LogEn, "[%s] i4MagicNum(%d) \n", __FUNCTION__, rReq.rParam.i4MagicNum);

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

                    _this->mLastMagicNumber = rReq.rParam.i4MagicNum;

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
                    if(FlashMgr::getInstance(_this->mi4SensorDev)->isFlashOnCalibration() != 1)
                        IAfMgr::getInstance().SetPauseAF(_this->mi4SensorDev, rReq.rParam.u1AfPause);

                    IAfMgr::getInstance().setMultiZoneEnable(_this->mi4SensorDev, rReq.rParam.u1MZOn);

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
                    _this->mpTaskMgr->sendEvent(ECmd_AFStart);
                    if(bIsPrecapTrigger || rReq.rParam.u1FlashTrigAF)
                    {
                        MY_LOGD("[%s] WaitTriggerAF(%d) \n", __FUNCTION__, _this->mLastMagicNumber);
                        IAfMgr::getInstance().WaitTriggerAF(_this->mi4SensorDev,MTRUE);
                    }
                    IAfMgr::getInstance().autoFocus(_this->mi4SensorDev);
                }
                else if(bIsAFEnd)
                {
                    _this->mpTaskMgr->sendEvent(ECmd_AFEnd);
                    IAfMgr::getInstance().cancelAutoFocus(_this->mi4SensorDev);
                }

                // AF trigger when AE is stable
                if(bIsAFTrigger)
                    IAfMgr::getInstance().triggerAF(_this->mi4SensorDev);

                // post threadResultPool
                if(rReq.rParam.i4MagicNum > 0)
                {
                    AAA_TRACE_HAL(getCurrResult4AF);
                    _this->getCurrResult4AF(rReq.rParam);
                    AAA_TRACE_END_HAL;

                    AAA_TRACE_HAL(postCmdToThread);
                    ResultPoolCmd4Convert_T Cmd;
                    Cmd.i4MagicNumReq = rReq.rParam.i4MagicNum;
                    Cmd.eConvertType = E_AF_CONVERT;
                    Cmd.i4SubsampleCount = 0;
                    MY_LOGD_IF(_this->mu4LogEn, "[Hal3A::AFThreadFunc] postCmd\n");
                    _this->m_pResultPoolObj->postCmdToThread(&Cmd);
                    AAA_TRACE_END_HAL;
                }
            }

            /*****************************
             *     Prepare Statistic
             *****************************/
            AAA_TRACE_HAL(PrepareStatistic);
            MBOOL isExistPrecap = _this->mpTaskMgr->isExistTask(TASK_ENUM_3A_PRECAPTURE);

            TaskData rData;
            rData.i4RequestMagic = _this->mLastMagicNumber;
            rData.bIsAfTriggerInPrecap = bIsAFStart && (bIsPrecapTrigger || isExistPrecap);    // Nelson Coverity : bIsAfStart is always false under this condition (bIsAFStart && (bIsPrecapTrigger || isExistPrecap));
            MINT32 i4SttMagic = _this->mpTaskMgr->prepareStt(rData,Task_Update_AF);

            CAM_LOGD_IF(_this->mu4LogEn,"[%s] TaskData : Req(#%d)/Stt(#%d)/AfTrigger(%d)", __FUNCTION__,
                rData.i4RequestMagic,
                i4SttMagic,
                rData.bIsAfTriggerInPrecap);
            AAA_TRACE_END_HAL;

            /*****************************
             *     Execute task
             *****************************/
            AAA_TRACE_MGR(AFUpdate);
            CAM_TRACE_BEGIN("AFUpdate");
            TASK_RESULT eResult __unused = _this->mpTaskMgr->execute(Task_Update_AF);
            CAM_TRACE_END();
            AAA_TRACE_END_MGR;

            /*****************************
             *     Release Statistic
             *****************************/
            AAA_TRACE_HAL(ReleaseStatistic);
            _this->mpTaskMgr->releaseStt(Task_Update_AF);
            AAA_TRACE_END_HAL;

            MY_LOGD_IF(_this->mu4LogEn, "[Hal3A::AFThreadFunc] TaskMgr::execute(AFUpdate) done\n");

            // 1. Use AF API get FV
            // 2. Push FVContainer
            MINT32 i4SttMagicNum = 0;
            NS3Av3::AF_FRAME_INFO_T sAFFV;
            IAfMgr::getInstance().sendAFCtrl(_this->mi4SensorDev, EAFMgrCtrl_GetFocusValue, reinterpret_cast<MINTPTR>(&i4SttMagicNum), reinterpret_cast<MINTPTR>(&sAFFV));
            CAM_LOGD_IF(_this->mu4LogEn,
                        "[%s] sttNum(#%d) FV(%d) DAC(%d) Gyro(%d, %d, %d) ROI(%d,%d,%d,%d,%d)",
                        __FUNCTION__,
                        i4SttMagicNum,
                        (MUINT32)sAFFV.i8FocusValue,
                        sAFFV.i4LensPos,
                        sAFFV.GyroValue[0], sAFFV.GyroValue[1], sAFFV.GyroValue[2],
                        sAFFV.AFROI[0], sAFFV.AFROI[1], sAFFV.AFROI[2], sAFFV.AFROI[3], sAFFV.AFROI[4]);
            if ( fvWriter->push(i4SttMagicNum, sAFFV) == MFALSE)
            {
                CAM_LOGE("[%s] FV Push Fail", __FUNCTION__);
            }

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
        }

        if(_this->mbAFPause)
        {
            CAM_LOGD("Pause +");
            ::sem_wait(&_this->semAFPause);
            CAM_LOGD("Pause -");
        }
    }

    // Clear TaskAF
    _this->mpTaskMgr->clearTaskQueue(Task_Update_AF);
    // clear FVContainer
    fvWriter->clear();

    if (!IsSupportAF)
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
ThreadRawImp::enableAFThread(MVOID* pTaskMgr)
{
    if (mbAFThreadLoop== 0)
    {
        MY_LOGD("[%s] +", __FUNCTION__);

        mpTaskMgr = (ITaskMgr*)pTaskMgr;
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
        {
            mbAFThreadLoop= 0;
            CAM_LOGE("[%s] result(%d)", __FUNCTION__, result);
        }

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
notifyPreStop(MBOOL isSecureCam)
{
    MY_LOGD("[%s] (mbPreStop, mbPreStop4Event)(%d, %d)", __FUNCTION__, mbPreStop, mbPreStop4Event);
    IAfMgr::getInstance().sendAFCtrl(mi4SensorDev, EAFMgrCtrl_NotifyPreStop, NULL, NULL);
    IPDMgr::getInstance().preStop(mi4SensorDev);
    mbPreStop = MTRUE;
    mbPreStop4Event = MTRUE;

    if (!isSecureCam)
    {
        NSCcuIf::ICcuCtrlAf* CCUCtrlAF = NSCcuIf::ICcuCtrlAf::createInstance((const uint8_t *)"RawImp",(ESensorDev_T)mi4SensorDev);
        if (CCUCtrlAF->init((MUINT32)mi4SensorIdx, (ESensorDev_T)mi4SensorDev) == CCU_CTRL_SUCCEED){
            CCUCtrlAF->ccu_af_abort();
            CCUCtrlAF->uninit();
        }
        else {
            CAM_LOGD("[%s] CCU AF init failed, hence not abort", __FUNCTION__);
        }
        CCUCtrlAF->destroyInstance();
    }

    return ;
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

    MY_LOGD_IF(mu4LogEn, "[%s] mReqQ size(%d) + cmd(%d) magic(%d)", __FUNCTION__, (MINT32)mReqQ.size(), rReq.eCmd, rReq.rParam.i4MagicNum);
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
getRequest(IThreadRaw::ReqQ_T &rReq, MINT32 i4Count __unused)
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

MBOOL
ThreadRawImp::
getCurrResult4AF(AFParam_T& AFParam)
{
    MY_LOGD_IF(mu4LogEn, "[%s] + i4MagicNum(%d)", __FUNCTION__, AFParam.i4MagicNum);

    AAA_TRACE_HAL(getAllResult);
    AllResult_T *pAllResult = m_pResultPoolObj->getAllResult(AFParam.i4MagicNum);
    AAA_TRACE_END_HAL;

    if(pAllResult)
    {
#if CAM3_AF_FEATURE_EN
        AAA_TRACE_MGR(getAFResult);
        AFResultToMeta_T      rAFResult;

        //put rResult.vecROI to mgr interfance (order is Type,Number of ROI,left,top,right,bottom,Result, left,top,right,bottom,Result...)
        {
            Mutex::Autolock Vec_lock(pAllResult->LockVecResult);
            IAfMgr::getInstance().getFocusArea(mi4SensorDev, pAllResult->vecAFROI);
        }
        // AF
        rAFResult.u1AfState = IAfMgr::getInstance().getAFState(mi4SensorDev);
        IAfMgr::getInstance().getFocusAreaResult(mi4SensorDev, pAllResult->vecFocusAreaPos, pAllResult->vecFocusAreaRes, rAFResult.i4FocusAreaSz[0], rAFResult.i4FocusAreaSz[1]);
        rAFResult.u1LensState = IAfMgr::getInstance().getLensState(mi4SensorDev);
        rAFResult.fLensFocusDistance = IAfMgr::getInstance().getFocusDistance(mi4SensorDev);
        IAfMgr::getInstance().getFocusRange(mi4SensorDev, &rAFResult.fLensFocusRange[0], &rAFResult.fLensFocusRange[1]);

        // CCT 3A Need to overwrite AF state when supportAF
        MINT32 IsSupportAF = IAfMgr::getInstance().getAFMaxAreaNum(mi4SensorDev);
        MINT32 i4AFEnable = 0;
        MUINT32 i4OutLens = 0;
        IAfMgr::getInstance().CCTOPAFGetEnableInfo(mi4SensorDev, (MVOID*)&i4AFEnable, &i4OutLens);
        if( IsSupportAF && (!i4AFEnable) && rAFResult.u1AfState != MTK_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED &&  rAFResult.u1AfState != MTK_CONTROL_AF_STATE_FOCUSED_LOCKED )
        {
            CAM_LOGD_IF(mu4LogEn, "[%s] overwrite AF state(%d)", __FUNCTION__, rAFResult.u1AfState);
            rAFResult.u1AfState = MTK_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED;
        }

        MBOOL IsFlashBackExist = (mpTaskMgr != NULL) ? mpTaskMgr->isExistTask(TASK_ENUM_3A_FLASH_BACK) : 0;
        MBOOL IsFlashFrontExist = (mpTaskMgr != NULL) ? mpTaskMgr->isExistTask(TASK_ENUM_3A_FLASH_FRONT) : 0;

        if (IsFlashBackExist && !IsFlashFrontExist)
        {
            TaskInfo info = mpTaskMgr->getTaskInfo(TASK_ENUM_3A_FLASH_BACK);
            CAM_LOGD("[%s] FlashBackExist, i4CreatedCmd(%d)", __FUNCTION__, info.i4CreatedCmd);
            if(info.i4CreatedCmd == ECmd_TouchAEStart || info.i4CreatedCmd == ECmd_TouchAEEnd) {
                if( AFParam.u4AfMode == MTK_CONTROL_AF_MODE_AUTO || AFParam.u4AfMode == MTK_CONTROL_AF_MODE_MACRO)
                    rAFResult.u1AfState = MTK_CONTROL_AF_STATE_ACTIVE_SCAN;
                else if( AFParam.u4AfMode == MTK_CONTROL_AF_MODE_CONTINUOUS_PICTURE)
                    rAFResult.u1AfState = MTK_CONTROL_AF_STATE_PASSIVE_SCAN;
                else if( AFParam.u4AfMode == MTK_CONTROL_AF_MODE_CONTINUOUS_VIDEO )
                    rAFResult.u1AfState = MTK_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED;
            }
        }

        CAM_LOGD_IF(mu4LogEn, "[%s] FlashBack (%d) overwrites AF state(%d)", __FUNCTION__, IsFlashBackExist, rAFResult.u1AfState);

        //Exif
        if (m_bDbgInfoEnable) {
            Mutex::Autolock Vec_lock(pAllResult->LockVecResult);
            if(pAllResult->vecDbg3AInfo.size()==0)
                pAllResult->vecDbg3AInfo.resize(sizeof(AAA_DEBUG_INFO1_T));

            if(pAllResult->vecDbg3AInfo.size())
            {
                AAA_DEBUG_INFO1_T& rDbg3AInfo = *reinterpret_cast<AAA_DEBUG_INFO1_T*>(pAllResult->vecDbg3AInfo.editArray());
                IAfMgr::getInstance().getDebugInfo(mi4SensorDev, rDbg3AInfo.rAFDebugInfo);
            }
            else
                CAM_LOGD_IF(mu4LogEn, "[%s] vecDbg3AInfo size(%d)", __FUNCTION__, pAllResult->vecDbg3AInfo.size());
        }

        MY_LOGD_IF(mu4LogEn, "[%s] u1AfState:%d", __FUNCTION__, rAFResult.u1AfState);
        AAA_TRACE_END_MGR;

        AAA_TRACE_MGR(updateModuleResult4AF);
        m_pResultPoolObj->updateResult(LOG_TAG, AFParam.i4MagicNum, E_AF_RESULTTOMETA, &rAFResult);
        AAA_TRACE_END_MGR;
#endif
    }
    else
        MY_LOGE("[%s] pAllResult NULL", __FUNCTION__);
    MY_LOGD_IF(mu4LogEn, "[%s] -", __FUNCTION__);
    return MTRUE;
}

