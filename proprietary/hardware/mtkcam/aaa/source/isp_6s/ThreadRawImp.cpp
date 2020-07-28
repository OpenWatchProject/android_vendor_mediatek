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

#include <list>
#include <vector>
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
#include <af_mgr/af_mgr.h>
#include <af_mgr/af_mgr_if.h>
#include <pd_mgr_if.h>
#include <af_define.h>
#include <iccu_ctrl_3actrl.h>
#include <ae_mgr/ae_mgr_if.h>
#include <flash_mgr.h>
#include <IEventIrq.h>
#include <debug/DebugUtil.h>
#include <aaa_hal_sttCtrl.h>
#include <task/ITaskMgr.h>

#include <property_utils.h>
#include <IResultPool.h>

#if CAM3_FLASH_FEATURE_EN
#include <flash_hal.h>
#include <flash_mgr.h>
#endif

// For AF focus value
#include <mtkcam/utils/hw/IFVContainer.h>

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

using namespace NSCam::Utils;
using namespace NS3Av3;

/******************************************************************************
*
*******************************************************************************/
class ThreadRawImp : public IThreadRaw
{
public:
    //
    static IThreadRaw* createInstance(I3AWrapper* pHal3A, MINT32 iSensorDev, MINT32 iSensorIdx, MINT32 i4SubsampleCount = 1);
    virtual MBOOL destroyInstance();
    virtual MINT32 config(const ConfigInfo_T& rConfigInfo);
    virtual MVOID postToEventThread(E_3AEvent_T eEvent, MINT32 opt = 0);
    virtual MVOID enableAFThread(MVOID* pTaskMgr);
    virtual MVOID disableAFThread();
    virtual MVOID pauseAFThread();
    virtual MVOID resumeAFThread();

    virtual MBOOL sendRequest(ECmd_T const eCmd, MUINTPTR const i4Arg = 0);
    virtual MVOID notifyPreStop();
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
    ThreadRawImp(I3AWrapper* pHal3A, MINT32 iSensorDev, MINT32 iSensorIdx, MINT32 i4SubsampleCount);
    virtual ~ThreadRawImp();

    ThreadRawImp(const ThreadRawImp&);
    ThreadRawImp& operator=(const ThreadRawImp&);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Event Thread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /**
    * @brief change Event thread setting
    */
    virtual MVOID changeEventThreadSetting();
    /**
    * @brief Event thread execution function
    */
    static  MVOID*  onEventThreadLoop(MVOID*);
    /**
    * @brief get Event from the head of Event request queue
    * @param [in] Event; please refer to IThreadRaw.h
    */
    virtual MBOOL getEvent(IThreadRaw::EventQ_T &rEvent);
    /**
    * @brief clear all requests in current event queue
    */
    virtual MVOID clearEventQ();

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

    virtual MBOOL getCurrResult4AF(MINT32& i4MagicNumber, AFParam_T& AFParam);

    MBOOL sem_wait_rt(sem_t *pSem, nsecs_t reltime, const char* info);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected: //private:
    I3AWrapper*     mpHal3A;
    MINT32          mi4LogEn;
    static MUINT32  mu4DebugLogWEn;
    std::atomic<int> mi4User;
    MINT32          mi4SensorDev;
    MINT32          mi4SensorIdx;
    std::mutex      mLock;
    std::mutex      mModuleMtx;
    MUINT32         mbPreStop;
    MUINT32         mbPreStop4Event;
    // Event Thread
    std::mutex      mEventMtx;
    MBOOL           mbEnEventThd;
    pthread_t       mEventThread;
    std::list<EventQ_T>  mEventQ;
    std::condition_variable       mEventQCond;
    IEventIrq*      m_pEventIrq;
    // AF Thread
    MUINT32         mLastMagicNumber;
    IEventIrq*      mpAFEventIrq;
    pthread_t       mAFThread;
    MINT32          mbAFThreadLoop;
    ITaskMgr*       mpTaskMgr;
    std::list<ReqQ_T>    mReqQ;
#if 1
    std::condition_variable       mCmdQCond;
#endif
    MBOOL           mbAFPause;
    sem_t           semAFPause;
    IResultPool*    m_pResultPoolObj;
    IEventIrq*      mpEventIrq;
    MUINT8          mu1LastAfState;
    MINT32          mi4SubsampleCount;
    IEventIrq*      m_pEventDELAY;
};

/*******************************************************************************
* utilities
********************************************************************************/
MUINT32 ThreadRawImp::mu4DebugLogWEn = 0;

IThreadRaw*
IThreadRaw::createInstance(I3AWrapper* pHal3A, MINT32 iSensorDev, MINT32 iSensorIdx, MINT32 i4SubsampleCount)
{
    return ThreadRawImp::createInstance(pHal3A, iSensorDev, iSensorIdx, i4SubsampleCount);
}

IThreadRaw*
ThreadRawImp::
createInstance(I3AWrapper* pHal3A, MINT32 iSensorDev, MINT32 iSensorIdx, MINT32 i4SubsampleCount)
{
    ThreadRawImp* pObj = new ThreadRawImp(pHal3A, iSensorDev, iSensorIdx, i4SubsampleCount);
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
ThreadRawImp(I3AWrapper* pHal3A, MINT32 iSensorDev, MINT32 iSensorIdx, MINT32 i4SubsampleCount)
    : mpHal3A(pHal3A)
    , mi4LogEn(MFALSE)
    , mi4User(0)
    , mi4SensorDev(iSensorDev)
    , mi4SensorIdx(iSensorIdx)
    , mLock()
    , mModuleMtx()
    , mbPreStop(MFALSE)
    , mbPreStop4Event(MFALSE)
    , mEventMtx()
    , mbEnEventThd(MFALSE)
    , mEventThread()
    , m_pEventIrq(NULL)
    , mLastMagicNumber(0)
    , mpAFEventIrq(NULL)
    , mAFThread()
    , mbAFThreadLoop(0)
    , mpTaskMgr(NULL)
    , mbAFPause(MFALSE)
    , semAFPause()
    , m_pResultPoolObj(NULL)
    , mpEventIrq(NULL)
    , mu1LastAfState(0)
    , mi4SubsampleCount(i4SubsampleCount)
    , m_pEventDELAY(NULL)
{
    getPropInt("vendor.debug.thread_raw.log", &mi4LogEn, 0);
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

MINT32
ThreadRawImp::config(const ConfigInfo_T& rConfigInfo)
{
    MY_LOGD("[%s] i4SubsampleCount(%d) i4RequestCount(%d)", __FUNCTION__, rConfigInfo.i4SubsampleCount, rConfigInfo.i4RequestCount);
    mi4SubsampleCount = rConfigInfo.i4SubsampleCount;
    return MTRUE;
}

/******************************************************************************
* create Event thread
*******************************************************************************/
MVOID
ThreadRawImp::createEventThread()
{
    std::lock_guard<std::mutex> lock(mLock);

    if (mi4User > 0)
    {
        MY_LOGD_IF(mi4LogEn, "[%s] mi4User(%d)", __FUNCTION__, std::atomic_load((&mi4User)));
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

        if(m_pEventDELAY == NULL && mi4SubsampleCount > 1)
        {
            IEventIrq::ConfigParam IrqConfig_AE(mi4SensorDev, mi4SensorIdx, 5000, IEventIrq::E_Event_Vsync_Sensor);
            m_pEventDELAY = IEventIrq::createInstance(IrqConfig_AE, "VS_DELAY");
        }

        mbEnEventThd = MTRUE;
        mbPreStop4Event = MFALSE;
        MINT32 result = pthread_create(&mEventThread, NULL, onEventThreadLoop, this);
        if(result != 0)
          CAM_LOGE("[%s] result(%d)", __FUNCTION__, result);
        MY_LOGD("[%s] -", __FUNCTION__);
    }
    MINT32 i4BeforeUserCount __unused = std::atomic_fetch_add((&mi4User), 1);
}

/******************************************************************************
* destroy Event thread
*******************************************************************************/
MVOID
ThreadRawImp::destroyEventThread()
{
    std::lock_guard<std::mutex> lock(mLock);

    if (mi4User > 0)
    {
        // More than one user, so decrease one User.
        MINT32 i4BeforeUserCount __unused = std::atomic_fetch_sub((&mi4User), 1);

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

            if(m_pEventDELAY && mi4SubsampleCount > 1)
            {
                // destroy AE irq
                m_pEventDELAY->flush();
                m_pEventDELAY->destroyInstance("VS_DELAY");
                m_pEventDELAY = NULL;
            }

            mbEnEventThd = MFALSE;
            mi4SubsampleCount = 1;
            mEventQCond.notify_all();
            pthread_join(mEventThread, NULL);
            clearEventQ();
            //
            MY_LOGD("[%s] -", __FUNCTION__);
        }
        else    // There are still some users.
        {
             MY_LOGD_IF(mi4LogEn, "[%s] mi4User(%d)", __FUNCTION__, std::atomic_load((&mi4User)));
        }
    }
}

/******************************************************************************
* change AE thread setting
*******************************************************************************/
MVOID
ThreadRawImp::changeEventThreadSetting()
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

    IEventIrq::Duration rDuration;
    _this->changeEventThreadSetting();

    IEventIrq::Duration duration;
    IThreadRaw::EventQ_T rEvent;

    // (2) thread-in-loop
    while(_this->getEvent(rEvent))
    {
        if ( ! _this->mbEnEventThd || _this->mbPreStop4Event) break;

        if(_this->m_pEventDELAY && _this->mi4SubsampleCount > 1)
            _this->m_pEventDELAY->wait(rDuration);

        if(rEvent.eEvent == E_3AEvent_AE_I2C){
            MY_LOGD_IF(_this->mi4LogEn, "[onEventThreadLoop] updateSensorbyI2C\n");

            if(_this->mi4SubsampleCount > 1)
            {
                CAM_TRACE_BEGIN("AE Sensor I2C");
                IAeMgr::getInstance().sendAECtrl(_this->mi4SensorDev, EAECtrl_SetSensorbyI2CBufferMode, NULL, NULL, NULL, NULL);
                CAM_TRACE_END();
            }
            else
            {
                if(cust_getIsSpecialLongExpOn())
                {
                    IEventIrq::Duration duration;
                    CAM_LOGI("[%s] start waitVSirq. (Special Long Exp)", __FUNCTION__);
                    _this->mpEventIrq->wait(duration);
                }

                CAM_TRACE_BEGIN("AE Sensor I2C");
                IAeMgr::getInstance().sendAECtrl(_this->mi4SensorDev, EAECtrl_SetSensorbyI2C, NULL, NULL, NULL, NULL);
                CAM_TRACE_END();
            }
        } else if(rEvent.eEvent == E_3AEvent_Flash_On){
            if(_this->m_pEventIrq)
                _this->m_pEventIrq->wait(duration);
            FlashHal::getInstance(_this->mi4SensorDev)->setOnOff(MTRUE, static_cast<FLASH_HAL_SCENARIO_ENUM>(rEvent.i4Opt));
        } else if(rEvent.eEvent == E_3AEvent_Flash_Off){
            if(_this->m_pEventIrq)
                _this->m_pEventIrq->wait(duration);
            FlashHal::getInstance(_this->mi4SensorDev)->setOnOff(MFALSE, static_cast<FLASH_HAL_SCENARIO_ENUM>(rEvent.i4Opt));
        } else if (rEvent.eEvent == E_3AEvent_CCU_CB) {
            _this->mpHal3A->notify4CCU();
        } else if(rEvent.eEvent == E_3AEvent_MainFlash_On){
            FlashHal::getInstance(_this->mi4SensorDev)->setOnOff(MTRUE, static_cast<FLASH_HAL_SCENARIO_ENUM>(rEvent.i4Opt));
        }
    }

    MY_LOGD("[%s] -" ,__FUNCTION__);

    return NULL;
}

/******************************************************************************
* trigger Event thread to handle event
*******************************************************************************/
MVOID
ThreadRawImp::postToEventThread(E_3AEvent_T eEvent, MINT32 opt)
{
    std::lock_guard<std::mutex> autoLock(mEventMtx);

    IThreadRaw::EventQ_T rEvent(eEvent,opt);
    mEventQ.push_back(rEvent);

    while(mEventQ.size() > 10){
        mEventQ.erase(mEventQ.begin());
    }
    mEventQCond.notify_all();
    MY_LOGD_IF(mi4LogEn, "[%s] mEventQ size(%d) + rEvent(%d/%d)", __FUNCTION__, (MINT32)mEventQ.size(), rEvent.eEvent, rEvent.i4Opt);
}

MBOOL
ThreadRawImp::getEvent(IThreadRaw::EventQ_T &rEvent)
{
    MY_LOGD_IF(mi4LogEn, "[%s] +", __FUNCTION__);
    std::unique_lock<std::mutex> autoLock(mEventMtx);
    MY_LOGD_IF(mi4LogEn, "[%s] mEventQ.size()=%d ", __FUNCTION__, (MINT32)mEventQ.size());

    if (mEventQ.size() == 0)
    {
        MY_LOGD_IF(mi4LogEn, "[%s] mEventQCond.wait +", __FUNCTION__);
        mEventQCond.wait_for(autoLock, std::chrono::nanoseconds(20000000000));
        MY_LOGD_IF(mi4LogEn, "[%s] mEventQCond.wait -", __FUNCTION__);
    }
    if (mEventQ.size() == 0)
    {
        MY_LOGD_IF(mi4LogEn, "[%s] mEventQ.size() = %d after mEventQCond.wait/waitRelative\n", __FUNCTION__, (MINT32)mEventQ.size());
        return MFALSE;
    }

    rEvent = *mEventQ.begin();
    MY_LOGD_IF(mi4LogEn, "mEventQ-size(%d), event(%d/%d)", (MINT32)mEventQ.size(), rEvent.eEvent, rEvent.i4Opt);

    mEventQ.erase(mEventQ.begin());
    MY_LOGD_IF(mi4LogEn, "[%s] -", __FUNCTION__);

    return MTRUE;
}

MVOID
ThreadRawImp::clearEventQ()
{
    MY_LOGD("[%s]+", __FUNCTION__);
    std::lock_guard<std::mutex> autoLock(mEventMtx);

    int Qsize = mEventQ.size();

    for (std::list<EventQ_T>::iterator it = mEventQ.begin(); it != mEventQ.end();)
    {
        MY_LOGD("[%s] EventQ size(%d), clear(%d/%d)", __FUNCTION__, (MINT32)mEventQ.size(), it->eEvent, it->i4Opt);
        it = mEventQ.erase(it);
    }
    MY_LOGD("[%s]- Qsize(%d)", __FUNCTION__, Qsize);
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
    MY_LOGD("[%s] +",__FUNCTION__);
    MY_LOGD("[AFThread] tid: %d \n", gettid());

    ThreadRawImp *_this = reinterpret_cast<ThreadRawImp*>(arg);
    IEventIrq::Duration duration;
    _this->changeAFThreadSetting();

    // StartPreview speed up
    IAfMgr::getInstance(_this->mi4SensorDev).start();

    AFStaticInfo_T staticInfo;
    IAfMgr::getInstance(_this->mi4SensorDev).getStaticInfo(staticInfo, LOG_TAG);
    MINT32 IsSupportAF = staticInfo.isAfSupport;

    // FSC
    MY_LOGD_IF(_this->mi4LogEn, "3AHal GetFSCInitInfo %d,%d,%d,%d,%d,%d,%d,%d",
               staticInfo.fscInitInfo.macro_To_Inf_Ratio,
               staticInfo.fscInitInfo.dac_Inf,
               staticInfo.fscInitInfo.dac_Macro,
               staticInfo.fscInitInfo.damping_Time,
               staticInfo.fscInitInfo.readout_Time_us,
               staticInfo.fscInitInfo.init_DAC,
               staticInfo.fscInitInfo.af_Table_Start,
               staticInfo.fscInitInfo.af_Table_End);


    TaskData rData;
    std::vector<MINT32> VecI4MagicNumber;

    // create FVContainer
    auto fvWriter = IFVContainer::createInstance(LOG_TAG, IFVContainer::eFVContainer_Opt_Write);

    while (_this->mbAFThreadLoop && IsSupportAF)
    {
        MY_LOGD_IF(_this->mi4LogEn, "[Hal3A::AFThreadFunc] AfMgr::DoCallback() done\n");

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

            MY_LOGD_IF(_this->mi4LogEn, "[Hal3A::AFThreadFunc] AF waitIrq done and clear magicNumber buffer\n");

            rData.sAfCommand.clear();
            VecI4MagicNumber.clear();

            while (_this->getRequest(rReq, i4RequestCount))
            {
                i4RequestCount++;
                MY_LOGD_IF(_this->mi4LogEn, "[%s] i4MagicNum(%d) \n", __FUNCTION__, rReq.rParam.i4MagicNum);

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

                    rData.sAfCommand.requestNum        = _this->mLastMagicNumber;
                    rData.sAfCommand.afMode            = rReq.rParam.u4AfMode;
                    rData.sAfCommand.focusDistance     = rReq.rParam.fFocusDistance;
                    rData.sAfCommand.cropRegion_X      = rReq.rParam.rScaleCropArea.i4Left;
                    rData.sAfCommand.cropRegion_Y      = rReq.rParam.rScaleCropArea.i4Top;
                    rData.sAfCommand.cropRegion_W      = rReq.rParam.rScaleCropArea.i4Right - rReq.rParam.rScaleCropArea.i4Left;
                    rData.sAfCommand.cropRegion_H      = rReq.rParam.rScaleCropArea.i4Bottom - rReq.rParam.rScaleCropArea.i4Top;
                    rData.sAfCommand.afRegions         = rReq.rParam.rFocusAreas;
                    if(rReq.rParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_STILL_CAPTURE || rReq.rParam.u1ZSDCaptureIntent == MTK_CONTROL_CAPTURE_INTENT_STILL_CAPTURE)
                    {
                        MY_LOGD("[%s] i4MagicNum(%d) CaptureIntent(%d, %d) \n", __FUNCTION__, rReq.rParam.i4MagicNum, rReq.rParam.u1CaptureIntent, rReq.rParam.u1ZSDCaptureIntent);
                        rData.sAfCommand.isCaptureIntent = 1;
                    }
                    else
                        rData.sAfCommand.isCaptureIntent = 0;

                    // Bypass SetPauseAF when in flash-calibration
                    if(FlashMgr::getInstance(_this->mi4SensorDev)->isFlashOnCalibration() != 1)
                    {
                        rData.sAfCommand.pauseAF = rReq.rParam.u1AfPause;
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
                        rData.sAfCommand.ePrecapture = AfCommand_Start;
                    }
                    rData.sAfCommand.eAutofocus = AfCommand_Start;
                }
                else if(bIsAFEnd)
                {
                    _this->mpTaskMgr->sendEvent(ECmd_AFEnd);
                    rData.sAfCommand.eAutofocus = AfCommand_Cancel;
                }

                // AF trigger when AE is stable
                if(bIsAFTrigger)
                {
                    rData.sAfCommand.triggerAF = 1;
                }
                VecI4MagicNumber.push_back(rReq.rParam.i4MagicNum);
            }

            /****************************************************
             * MUST update AfInputData before Prepare Statistic *
             ****************************************************/
            // isFlashFrm
            rData.sAfInputData.isFlashFrm          = FlashHal::getInstance(_this->mi4SensorDev)->isAFLampOn();
            // GMV data
            P2Info_T rP2Info;
            _this->m_pResultPoolObj->getP2Info(rP2Info);
            rData.sAfInputData.gmvInfo.GMV_X       = rP2Info.AF_GMV_X;
            rData.sAfInputData.gmvInfo.GMV_Y       = rP2Info.AF_GMV_Y;
            rData.sAfInputData.gmvInfo.GMV_Conf_X  = rP2Info.AF_GMV_Conf_X;
            rData.sAfInputData.gmvInfo.GMV_Conf_Y  = rP2Info.AF_GMV_Conf_Y;
            rData.sAfInputData.gmvInfo.GMV_Max     = rP2Info.AF_GMV_Max;

            /*****************************
             *     Prepare Input Data
             *****************************/
            // AE2AFInfo
            AE_PERFRAME_INFO_T AEPerframeInfo;
            IAeMgr::getInstance().getAEInfo(_this->mi4SensorDev, AEPerframeInfo);
            rData.sAfInputData.ispAeInfo.FrameNum           = rData.sAfCommand.requestNum;
            rData.sAfInputData.ispAeInfo.isAEStable         = AEPerframeInfo.rAEISPInfo.bAEStable;
            rData.sAfInputData.ispAeInfo.isAELock           = AEPerframeInfo.rAEUpdateInfo.bAELock;
            rData.sAfInputData.ispAeInfo.isAEScenarioChange = AEPerframeInfo.rAEUpdateInfo.bAEScenarioChange;
            rData.sAfInputData.ispAeInfo.deltaIndex         = AEPerframeInfo.rAEISPInfo.i4deltaIndex;
            rData.sAfInputData.ispAeInfo.afeGain            = AEPerframeInfo.rAEISPInfo.u4P1SensorGain;
            rData.sAfInputData.ispAeInfo.ispGain            = AEPerframeInfo.rAEISPInfo.u4P1DGNGain;
            rData.sAfInputData.ispAeInfo.exposuretime       = (AEPerframeInfo.rAEISPInfo.u8P1Exposuretime_ns/1000);
            rData.sAfInputData.ispAeInfo.realISOValue       = AEPerframeInfo.rAEISPInfo.u4P1RealISOValue;
            rData.sAfInputData.ispAeInfo.aeFinerEVIdxBase   = AEPerframeInfo.rAEISPInfo.u4AEFinerEVIdxBase;
            rData.sAfInputData.ispAeInfo.aeCWValue          = AEPerframeInfo.rAEUpdateInfo.u4CWValue;
            rData.sAfInputData.ispAeInfo.aeIdxCurrentF      = AEPerframeInfo.rAEUpdateInfo.u4AEidxCurrentF;
            rData.sAfInputData.ispAeInfo.lightValue_x10     = AEPerframeInfo.rAEISPInfo.i4LightValue_x10;
            rData.sAfInputData.ispAeInfo.isLVChangeTooMuch  = AEPerframeInfo.rAEUpdateInfo.bLVChangeTooMuch;
            for(MUINT32 i = 0; i < AE_BLOCK_NO*AE_BLOCK_NO; i++)
                rData.sAfInputData.ispAeInfo.aeBlockV[i] = AEPerframeInfo.rAEUpdateInfo.pu4AEBlock[i/AE_BLOCK_NO][i%AE_BLOCK_NO];


            /*****************************
             *     Prepare Statistic
             *****************************/
            AAA_TRACE_HAL(PrepareStatistic);
            MBOOL isExistPrecap = _this->mpTaskMgr->isExistTask(TASK_ENUM_3A_PRECAPTURE);
            rData.i4RequestMagic = _this->mLastMagicNumber;
            rData.bIsAfTriggerInPrecap = (bIsAFStart && (bIsPrecapTrigger || isExistPrecap));    // Nelson Coverity : bIsAfStart is always false under this condition (bIsAFStart && (bIsPrecapTrigger || isExistPrecap));
            MINT32 i4SttMagic = _this->mpTaskMgr->prepareStt(rData, Task_Update_AF);
            CAM_LOGD_IF(_this->mi4LogEn,"[%s] TaskData : Req(#%d)/Stt(#%d)/AfTrigger(%d)", __FUNCTION__,
                        rData.i4RequestMagic,
                        i4SttMagic,
                        rData.bIsAfTriggerInPrecap);
            AAA_TRACE_END_HAL;

            /*****************************
             *     Execute task
             *****************************/
            AAA_TRACE_MGR(AFUpdate);
            CAM_TRACE_BEGIN("AFUpdate");
            TASK_RESULT eResult = _this->mpTaskMgr->execute(Task_Update_AF); // process
            CAM_TRACE_END();
            AAA_TRACE_END_MGR;

            /*****************************
             *     Release Statistic
             *****************************/
            AAA_TRACE_HAL(ReleaseStatistic);
            _this->mpTaskMgr->releaseStt(Task_Update_AF);
            AAA_TRACE_END_HAL;

            MY_LOGD_IF(_this->mi4LogEn, "[Hal3A::AFThreadFunc] TaskMgr::execute(AFUpdate) done\n");

            AAA_TRACE_HAL(getCurrResult4AF);
            AllResult_T *pAllResult = NULL;
            AllMetaResult_T* pMetaResult = NULL;
            for(MINT32 i = 0; i < VecI4MagicNumber.size(); i++)
            {
                MINT32 i4MagicNum = VecI4MagicNumber[i];
                 if(VecI4MagicNumber.size() > 1)
                    MY_LOGW("[%s] AF execute too long, so push current Magic(%d) result to previous Magic(%d) result\n", __FUNCTION__, rReq.rParam.i4MagicNum, i4MagicNum);

                if(i4MagicNum > 0)
                {
                    _this->getCurrResult4AF(i4MagicNum, rReq.rParam); // getResult
                    pAllResult = _this->m_pResultPoolObj->getResultByReqNum(i4MagicNum, __FUNCTION__);
                    pMetaResult = _this->m_pResultPoolObj->getMetadata(i4MagicNum, __FUNCTION__);

                    if(pAllResult != NULL && pMetaResult != NULL)
                        _this->m_pResultPoolObj->convertToMetadataFlow(i4MagicNum, E_AF_CONVERT, pAllResult, pMetaResult);
                    _this->m_pResultPoolObj->returnResult(pAllResult, __FUNCTION__);
                    _this->m_pResultPoolObj->returnMetadata(pMetaResult, __FUNCTION__);
                }
            }
            AAA_TRACE_END_HAL;

            // 1. Use AF API get FV
            // 2. Push FVContainer

            AFResult_T rAFResultFromMgr;
            IAfMgr::getInstance(_this->mi4SensorDev).getResult(rAFResultFromMgr);

            // info for BSS
            MINT32 i4SttMagicNum = 0;
            NS3Av3::AF_FRAME_INFO_T sAFFV;
            sAFFV.i8FocusValue = rAFResultFromMgr.focusValue;
            sAFFV.i4LensPos = rAFResultFromMgr.lensPosition;
            for(int i=0; i<3; i++)
            {
                sAFFV.GyroValue[i] = rAFResultFromMgr.gyroValue[i];
            }
            for(int i=0; i<5; i++)
            {
                sAFFV.AFROI[i] = rAFResultFromMgr.afRegions[i];
            }
            CAM_LOGD_IF(_this->mi4LogEn,
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

            // info for FSC
            FSC_FRM_INFO_T* FSCInfo = &(rAFResultFromMgr.fscInfo);
            MY_LOGD_IF(_this->mi4LogEn, "3AHal FSC SttNum %d, SetCount %d", FSCInfo->SttNum, FSCInfo->SetCount);
            for(int i=0;i<FSCInfo->SetCount;i++)
            {
                MY_LOGD_IF(_this->mi4LogEn, "3AHal GetFSCInfo Set %d : DAC (%d)->(%d), Percent (%d)",
                         i,
                         FSCInfo->DACInfo[i].DAC_From,
                         FSCInfo->DACInfo[i].DAC_To,
                         FSCInfo->DACInfo[i].Percent);
            }
            if(_this->mpHal3A)
                _this->mpHal3A->send3ACtrl(E3ACtrl_NOTIFY_AF_FSC_INFO, reinterpret_cast<MINTPTR>(FSCInfo), 0);
            else
                CAM_LOGE("[%s] mpHal3A NULL", __FUNCTION__);
        }
        else
        {
            //MY_ERR("[AFThread] AF irq timeout or error\n");
            IAfMgr::getInstance(_this->mi4SensorDev).timeOutHandle();
        }

        if(_this->mbAFPause)
        {
            CAM_LOGD("Pause +");
            ::sem_wait(&_this->semAFPause);
            CAM_LOGD("Pause -");
        }
    }

    VecI4MagicNumber.clear();
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
notifyPreStop()
{
    MY_LOGD("[%s] (mbPreStop, mbPreStop4Event)(%d, %d)", __FUNCTION__, mbPreStop, mbPreStop4Event);
    mbPreStop = MTRUE;
    mbPreStop4Event = MTRUE;
    IAfMgr::getInstance(mi4SensorDev).notify(E_AFNOTIFY_ABORT);
    return ;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
ThreadRawImp::
sem_wait_rt(sem_t *pSem, nsecs_t reltime, const char* info)
{
    MY_LOGD_IF(mi4LogEn, "[%s] pSem(%p), reltime(%ld), info(%s)\n", __FUNCTION__, pSem, (long)reltime, info);
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
        MY_LOGD_IF(mi4LogEn, "[%s][%s]sem_timedwait() succeeded\n", __FUNCTION__, info);

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
    MY_LOGD_IF(mi4LogEn, "[%s] r3ACmd(%d)\n", __FUNCTION__, r3ACmd);

    addReqestQ(rReq);
    return MTRUE;
}

MVOID
ThreadRawImp::
addReqestQ(IThreadRaw::ReqQ_T &rReq)
{
    std::lock_guard<std::mutex> autoLock(mModuleMtx);

    mReqQ.push_back(rReq);

    while(mReqQ.size() > 10){
        mReqQ.erase(mReqQ.begin());
    }
#if 1
    mCmdQCond.notify_all();
#endif
    MY_LOGD_IF(mi4LogEn, "[%s] mReqQ size(%d) + cmd(%d) magic(%d)", __FUNCTION__, (MINT32)mReqQ.size(), rReq.eCmd, rReq.rParam.i4MagicNum);
}

MVOID
ThreadRawImp::
clearReqestQ()
{
    MY_LOGD("[%s]+", __FUNCTION__);
    std::lock_guard<std::mutex> autoLock(mModuleMtx);

    int Qsize = mReqQ.size();

    for (std::list<ReqQ_T>::iterator it = mReqQ.begin(); it != mReqQ.end();)
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
    MY_LOGD_IF(mi4LogEn, "[%s] +", __FUNCTION__);
    std::unique_lock<std::mutex> autoLock(mModuleMtx);
    MY_LOGD_IF(mi4LogEn, "[%s] mCmdQ.size()=%d i4Count=%d", __FUNCTION__, (MINT32)mReqQ.size(), i4Count);

#if 1
    AFStaticInfo_T staticInfo;
    IAfMgr::getInstance(mi4SensorDev).getStaticInfo(staticInfo,LOG_TAG);
    //no request in queue need to wait request until 10ms
    if (mReqQ.size() == 0 && i4Count == 0)
    {
        if( mCmdQCond.wait_for(autoLock, std::chrono::nanoseconds(10000000)) == cv_status::no_timeout )
            MY_LOGD("[%s] finished waiting", __FUNCTION__);
        else
        {
            MY_LOGD("[%s] - waiting timed out", __FUNCTION__);
            return MFALSE;
        }
    }

    //DoAf after parse the lastest
    if(mReqQ.size() == 0 && i4Count != 0)
    {
        MY_LOGD_IF(mi4LogEn, "[%s] AF queue is zero, so break to doAF", __FUNCTION__);
        return MFALSE;
    }
#else

    if (mReqQ.size() == 0) //this is only for en_timeout == 1 & timeout case
    {
        MY_LOGD_IF(mi4LogEn, "[%s] mCmdQ.size() = %d after mCmdQCond.wait/waitRelative\n", __FUNCTION__, (MINT32)mReqQ.size());
        return MFALSE;
    }
#endif

    MY_LOGD_IF(mi4LogEn, "mReqQ-size(%d), eCmd(%d)", (MINT32)mReqQ.size(), rReq.eCmd);

    //RequestQ has many metadata, need parse the lastest.
    if(mReqQ.size() != 0)
    {
        rReq = *mReqQ.begin();
        mReqQ.erase(mReqQ.begin());
    }
    MY_LOGD_IF(mi4LogEn, "[%s] -", __FUNCTION__);

    return MTRUE;
}

MBOOL
ThreadRawImp::
getCurrResult4AF(MINT32& i4MagicNumber, AFParam_T& AFParam)
{
    MY_LOGD_IF(mi4LogEn, "[%s] + i4MagicNum(%d, %d)", __FUNCTION__, i4MagicNumber, AFParam.i4MagicNum);

    AAA_TRACE_HAL(getResultByReqNum);
    AllResult_T *pAllResult = m_pResultPoolObj->getResultByReqNum(i4MagicNumber, __FUNCTION__);
    AAA_TRACE_END_HAL;

    if(pAllResult)
    {
#if CAM3_AF_FEATURE_EN
        AAA_TRACE_MGR(getAFResult);
        AFResultToMeta_T      rAFResult;
        AFResult_T            rAFResultFromMgr;

        IAfMgr::getInstance(mi4SensorDev).getResult(rAFResultFromMgr);

        if(mi4SubsampleCount > 1)
        {
            // ResultPool - update previous setting to resultPool
            MY_LOGD_IF(mi4LogEn, "[%s] update old setting LastAfState%d", __FUNCTION__, mu1LastAfState);
            pAllResult->rOld3AInfo.rAfInfo.u1AfState = mu1LastAfState;
        }

        //put rResult.vecROI to mgr interfance (order is Type,Number of ROI,left,top,right,bottom,Result, left,top,right,bottom,Result...)
        for(int i=0; i<7; i++)
        {
            // type, num, left, top, right, bottom, isFDAF
            pAllResult->vecAFROI.push_back(rAFResultFromMgr.afRegions[i]);
        }
        // AF
        rAFResult.u1AfState = rAFResultFromMgr.afState;
        rAFResult.u1LensState = rAFResultFromMgr.lensState;
        rAFResult.fLensFocusDistance = rAFResultFromMgr.lensFocusDistance;
        rAFResult.fLensFocusRange[0] = rAFResultFromMgr.lensFocusRange_near;
        rAFResult.fLensFocusRange[1] = rAFResultFromMgr.lensFocusRange_far;

        // CCT 3A Need to overwrite AF state when supportAF
        AFStaticInfo_T rAFStaticInfo;
        IAfMgr::getInstance(mi4SensorDev).getStaticInfo(rAFStaticInfo, LOG_TAG);
        MINT32 IsSupportAF = rAFStaticInfo.isAfSupport;
        MINT32 i4AFEnable = 0;
        MUINT32 i4OutLens = 0;
        IAfMgr::getInstance(mi4SensorDev).CCTOPAFGetEnableInfo( (MVOID*)&i4AFEnable, &i4OutLens);
        if( IsSupportAF && (!i4AFEnable) && rAFResult.u1AfState != MTK_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED &&  rAFResult.u1AfState != MTK_CONTROL_AF_STATE_FOCUSED_LOCKED )
        {
            CAM_LOGD_IF(mi4LogEn, "[%s] overwrite AF state(%d)", __FUNCTION__, rAFResult.u1AfState);
            rAFResult.u1AfState = MTK_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED;
        }

        MBOOL IsFlashBackExist = (mpTaskMgr != NULL) ? mpTaskMgr->isExistTask(TASK_ENUM_3A_FLASH_BACK) : 0;
        MBOOL IsFlashFrontExist = (mpTaskMgr != NULL) ? mpTaskMgr->isExistTask(TASK_ENUM_3A_FLASH_FRONT) : 0;

        if (IsFlashBackExist && !IsFlashFrontExist) {
            if( AFParam.u4AfMode == MTK_CONTROL_AF_MODE_AUTO || AFParam.u4AfMode == MTK_CONTROL_AF_MODE_MACRO)
                rAFResult.u1AfState = MTK_CONTROL_AF_STATE_ACTIVE_SCAN;
            else if( AFParam.u4AfMode == MTK_CONTROL_AF_MODE_CONTINUOUS_PICTURE)
                rAFResult.u1AfState = MTK_CONTROL_AF_STATE_PASSIVE_SCAN;
            else if( AFParam.u4AfMode == MTK_CONTROL_AF_MODE_CONTINUOUS_VIDEO )
                rAFResult.u1AfState = MTK_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED;
        }

        CAM_LOGD_IF(mi4LogEn, "[%s] FlashBack (%d) overwrites AF state(%d)", __FUNCTION__, IsFlashBackExist, rAFResult.u1AfState);

        //Exif
        if(pAllResult->vecDbg3AInfo.size()==0)
            pAllResult->vecDbg3AInfo.resize(sizeof(AAA_DEBUG_INFO1_T));

        if(pAllResult->vecDbg3AInfo.size())
        {
            AAA_DEBUG_INFO1_T& rDbg3AInfo = *reinterpret_cast<AAA_DEBUG_INFO1_T*>(pAllResult->vecDbg3AInfo.editArray());
            IAfMgr::getInstance(mi4SensorDev).getDebugInfo( rDbg3AInfo.rAFDebugInfo);
        }
        //Update last AF status
        if(mi4SubsampleCount > 1)
        {
            mu1LastAfState = rAFResult.u1AfState;
        }
        AAA_TRACE_END_MGR;

        AAA_TRACE_MGR(updateModuleResult4AF);
        pAllResult->ModuleResult[E_AF_RESULTTOMETA]->write(&rAFResult);
        AAA_TRACE_END_MGR;

        AFResultConfig_T AFResultConfig;
        IAfMgr::getInstance(mi4SensorDev).getHWCfgReg(&AFResultConfig);
        pAllResult->ModuleResult[E_AF_CONFIGRESULTTOISP]->write(&AFResultConfig);

        // update PDO config result to ResutlPool
        isp_pdo_cfg_t PDOResultConfig;
        IPDMgr::getInstance().getPDOHWCfg(mi4SensorDev, &PDOResultConfig);
        pAllResult->ModuleResult[E_PDO_CONFIGRESULTTOISP]->write(&PDOResultConfig);
        MY_LOGD_IF(mi4LogEn, "[%s] AfState(%d), u1LensState(%d, %d), enableAFHw(%d), module update config and result(#%d) done", __FUNCTION__, rAFResult.u1AfState, rAFResult.u1LensState, rAFResultFromMgr.lensState, AFResultConfig.afIspRegInfo.enableAFHw, i4MagicNumber);
#endif
    }
    else
        MY_LOGE("[%s] pAllResult NULL", __FUNCTION__);
    m_pResultPoolObj->returnResult(pAllResult, __FUNCTION__);
    MY_LOGD_IF(mi4LogEn, "[%s] -", __FUNCTION__);
    return MTRUE;
}

