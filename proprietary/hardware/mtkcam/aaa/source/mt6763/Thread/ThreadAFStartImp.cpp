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
#define LOG_TAG "ThreadAFStart"

#include <IThread.h>

#include <mtkcam/def/PriorityDefs.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/prctl.h>
#include <sys/resource.h>

#include <cutils/properties.h>
#include <utils/threads.h>
#include <utils/List.h>
#include <mtkcam/utils/std/Log.h>
#include <aaa_error_code.h>

#if CAM3_AF_FEATURE_EN
#include <af_feature.h>
#include <af_algo_if.h>
#include <af_mgr/af_mgr_if.h>
#endif

#include <debug/DebugUtil.h>

#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>


#define GET_PROP(prop, init, val)\
{\
    char value[PROPERTY_VALUE_MAX] = {'\0'};\
    property_get(prop, value, (init));\
    (val) = atoi(value);\
}

namespace NS3Av3
{
using namespace android;

class ThreadAFStartImp : public ThreadAFStart
{
public:
    virtual MBOOL               destroyInstance();
    virtual MBOOL               postCmd(void* pArg);
    virtual MBOOL               waitFinished();

                                ThreadAFStartImp(MUINT32 u4SensorDev, MINT32 i4SensorIndex);
    virtual                     ~ThreadAFStartImp();
private:
    static MVOID*               onThreadLoop(MVOID*);
    MVOID                       changeThreadSetting();
    MBOOL                       getCmd(ThreadAFStart::Cmd_T& rCmd);

    MBOOL                       m_fgLogEn;
    MUINT32                     m_u4SensorDev;
    MINT32                      m_i4SensorIndex;
    pthread_t                   m_rThread;
    List<ThreadAFStart::Cmd_T>  m_rCmdQ;
    Mutex                       m_ModuleMtx;
    Condition                   m_rCmdQCond;

};

ThreadAFStart*
ThreadAFStart::
createInstance(MUINT32 u4SensorDev, MINT32 i4SensorIndex)
{
    ThreadAFStartImp* pObj = new ThreadAFStartImp(u4SensorDev, i4SensorIndex);
    return pObj;
}

MBOOL
ThreadAFStartImp::
destroyInstance()
{
    CAM_LOGD("[%s] +", __FUNCTION__);
    waitFinished();
    delete this;
    CAM_LOGD("[%s] -", __FUNCTION__);
    return MTRUE;
}

MBOOL
ThreadAFStartImp::
postCmd(void* pArg)
{
    Mutex::Autolock autoLock(m_ModuleMtx);

    ThreadAFStart::Cmd_T rCmd;
    // data clone
    if (pArg != NULL)
        rCmd = *reinterpret_cast<ThreadAFStart::Cmd_T*>(pArg);

    m_rCmdQ.push_back(rCmd);
    m_rCmdQCond.broadcast();

    CAM_LOGD("[%s] m_rCmdQ size(%d), AAOMode(%d), Scenario4AF(%d)",
        __FUNCTION__, (MINT32)m_rCmdQ.size(), rCmd.bAAOMode, rCmd.u4Scenario4AF);

    return MTRUE;
}

MBOOL
ThreadAFStartImp::
getCmd(ThreadAFStart::Cmd_T& rCmd)
{
    Mutex::Autolock autoLock(m_ModuleMtx);

    if (m_rCmdQ.size() == 0)
    {
        m_rCmdQCond.wait(m_ModuleMtx);
    }

    rCmd = *m_rCmdQ.begin();
    CAM_LOGD("[%s] m_rCmdQ size(%d), AAOMode(%d), Scenario4AF(%d)",
        __FUNCTION__, (MINT32)m_rCmdQ.size(), rCmd.bAAOMode, rCmd.u4Scenario4AF);

    m_rCmdQ.erase(m_rCmdQ.begin());

    return MTRUE;
}

MBOOL
ThreadAFStartImp::
waitFinished()
{
    CAM_LOGD("[%s] + sensor(%d)", __FUNCTION__, m_u4SensorDev);
    ::pthread_join(m_rThread, NULL);
    CAM_LOGD("[%s] - sensor(%d)", __FUNCTION__, m_u4SensorDev);
    return MTRUE;
}


ThreadAFStartImp::
ThreadAFStartImp(MUINT32 u4SensorDev, MINT32 i4SensorIndex)
    : m_fgLogEn(0)
    , m_u4SensorDev(u4SensorDev)
    , m_i4SensorIndex(i4SensorIndex)
    , m_rThread()
{
    CAM_LOGD("[%s] sensor(%d), sensoridx(%d)", __FUNCTION__, u4SensorDev, i4SensorIndex);
    // init something
    MUINT32 u4LogEn = 0;
    GET_PROP("vendor.debug.ThreadAFStart.log", "0", u4LogEn);
    m_fgLogEn = u4LogEn ? 1 : 0;
    // create thread
    ::pthread_create(&m_rThread, NULL, onThreadLoop, this);
}

ThreadAFStartImp::
~ThreadAFStartImp()
{
    CAM_LOGD("[%s] sensor(%d)", __FUNCTION__, m_u4SensorDev);
}

MVOID
ThreadAFStartImp::
changeThreadSetting()
{
    // (1) set name
    ::prctl(PR_SET_NAME, "ThreadAFStart", 0, 0, 0);
    //
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);

    // (2) set policy/priority
#if 0
    int const policy    = SCHED_RR;
    int const priority  = PRIO_RT_3A_THREAD;
    //  set
    sched_p.sched_priority = priority;  //  Note: "priority" is real-time priority.
    ::sched_setscheduler(0, policy, &sched_p);
    //  get
    ::sched_getparam(0, &sched_p);
#else
    int const policy    = SCHED_OTHER;
    int const priority  = NICE_CAMERA_AF_Start;
    //  set
    sched_p.sched_priority = priority;  //  Note: "priority" is nice value.
    ::sched_setscheduler(0, policy, &sched_p);
    ::setpriority(PRIO_PROCESS, 0, priority);
    //  get
    sched_p.sched_priority = ::getpriority(PRIO_PROCESS, 0);
#endif
    //
    CAM_LOGD(
        "[%s] sensor(%d), tid(%d), policy:(expect, result)=(%d, %d), priority:(expect, result)=(0x%08x, 0x%08x)", __FUNCTION__
        , m_u4SensorDev, ::gettid()
        , policy, ::sched_getscheduler(0)
        , priority, sched_p.sched_priority
    );
}

MVOID*
ThreadAFStartImp::
onThreadLoop(MVOID* pArg)
{
    CAM_LOGD("[%s] +", __FUNCTION__);
    ThreadAFStartImp* _this = reinterpret_cast<ThreadAFStartImp*>(pArg);
    MUINT32 u4SensorDev = _this->m_u4SensorDev;
    ThreadAFStart::Cmd_T rCmd;
    MRESULT err = S_3A_OK;

    _this->changeThreadSetting();

    if (_this->getCmd(rCmd))
    {
#if CAM3_AF_FEATURE_EN
        // AF start
        IAfMgr::getInstance().setCamScenarioMode(u4SensorDev, rCmd.u4Scenario4AF);
        AAA_TRACE_D("AF Start");
        err = IAfMgr::getInstance().Start(u4SensorDev);
        AAA_TRACE_END_D;
        if (FAILED(err)) {
            CAM_LOGE("AfMgr::getInstance().Start() fail\n");
        }
#endif
    }

    CAM_LOGD("[%s] -", __FUNCTION__);

    return NULL;
}



};
