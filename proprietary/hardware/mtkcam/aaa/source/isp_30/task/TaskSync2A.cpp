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
#define LOG_TAG "TaskSync2A"

#include <mtkcam/utils/std/Log.h>
#include <cutils/properties.h>
#include <mtkcam/utils/std/Trace.h>
// 3A framework
#include <task/ITask.h>
#include <aaa_trace.h>
#include <aaa_types.h>
#include <aaa_timer.h>
#include <aaa_common_custom.h>
#include <aaa_hal_sttCtrl.h>
#include <aaa_scheduling_custom.h>
#include <IResultPool.h>
#include <IThreadRaw.h>
// Sync 3A
#include <ISync3A.h>
// 3A module
#include <ae_mgr/ae_mgr_if.h>
#include <awb_mgr/awb_mgr_if.h>
// ISP
#include <isp_tuning_mgr.h>

using namespace NS3Av3;

/*********************************************************
 *
 *   Interface of TaskSync2AImp
 *
 *********************************************************/

class TaskSync2AImp : public TaskSync2A
{
public:
    virtual MVOID destroy();
            TaskSync2AImp(MINT32 const i4SensorDev);
    virtual ~TaskSync2AImp();

    virtual TASK_RESULT run(MINT32 i4Opt, const TaskData& rData);
    virtual MVOID cancel();
    virtual MVOID waitFinish();
    virtual MINT32 getStatus();
    virtual TaskInfo getInfo(){return m_rTaskInfo;};

private:
    virtual MVOID updateStatus(MINT32 i4Status);
    MINT32 m_i4SensorDev;
    MINT32 m_i4FrameCount;
    MINT32 m_i4Status;
    MINT32 m_i4LogEn;
    AaaTimer m_rTimer;
    mutable Mutex m_StatusLock;
    mutable Mutex m_RunningLock;
};

/*********************************************************
 *
 *   TaskSync2A implement functions
 *
 *********************************************************/

TaskSync2A*
TaskSync2A::create(MINT32 const i4SensorDev)
{
    return new TaskSync2AImp(i4SensorDev);
}

/*********************************************************
 *
 *   TaskSync2AImp implement functions
 *
 *********************************************************/

TaskSync2AImp::TaskSync2AImp(MINT32 const i4SensorDev)
    : m_i4SensorDev(i4SensorDev)
    , m_i4FrameCount(0)
    , m_i4Status(TASK_STATUS_INIT)
    , m_i4LogEn(0)
{
    m_i4LogEn = property_get_int32("debug.hal3a.task",0);
}

TaskSync2AImp::~TaskSync2AImp()
{
}

MVOID
TaskSync2AImp::destroy()
{
    CAM_LOGW("[%s] +", __FUNCTION__);
    waitFinish();
    delete this;
    CAM_LOGW("[%s] -", __FUNCTION__);
}

TASK_RESULT
TaskSync2AImp::run(MINT32 i4Opt, const TaskData& rData)
{
    Mutex::Autolock lock(m_RunningLock);
    TASK_RESULT eResult = TASK_RESULT_OK;

    if(getStatus() >= TASK_STATUS_CANCELLED)
        return eResult;

    //===== prepare statistic data =====
    StatisticBufInfo* pAAOBuffer   = NULL;
    IThreadRaw*       pThreadRaw   = NULL;
    MINT32            i4FrameNum   = rData.i4FrameNum;

    if(rData.pAAOBuffer)
        pAAOBuffer = reinterpret_cast<StatisticBufInfo*>(rData.pAAOBuffer);
    if(rData.pThreadRaw)
        pThreadRaw = reinterpret_cast<IThreadRaw*>(rData.pThreadRaw);

    if(getStatus() >= TASK_STATUS_CANCELLED)
        return eResult;

    if(pAAOBuffer == NULL)
    {
        CAM_LOGW("[%s] AAO buffer is NULL", __FUNCTION__);
        CAM_LOGD("[%s] Cam %d : Req(#%d)/Frm(#%d)/i4Opt(%d)/FlashOnOff(%d)",__FUNCTION__, m_i4SensorDev,
            rData.i4RequestMagic, i4FrameNum, i4Opt,
            rData.bFlashOnOff);
#if 0 // remove this protect, because sync3A will timeout. Sync3A will protect AAOBuffer is NULL or not.
        eResult = TASK_RESULT_ERROR;
        return eResult;
#endif
    }
    else
    {
        CAM_LOGD("[%s] Cam %d : Req(#%d)/Stt(#%d)/Frm(#%d)/i4Opt(%d)/FlashOnOff(%d)",__FUNCTION__, m_i4SensorDev,
            rData.i4RequestMagic, pAAOBuffer->mMagicNumber, i4FrameNum, i4Opt,
            rData.bFlashOnOff);
    }

    //===== Sync2A calculation =====
    MINT32 i4SyncOpt = 0;
    MINT32 i4ActiveAeItem = 0;
    ISync3A* pSync3A = ISync3AMgr::getInstance()->getSync3A();

    if(i4Opt & TASK_OPTION_ONLY_AE_CALC)
        i4SyncOpt |= ISync3A::E_SYNC3A_DO_AE;
    i4SyncOpt |= ISync3A::E_SYNC3A_DO_AWB;
    i4SyncOpt |= ISync3AMgr::getInstance()->getFrmSyncOpt();

    m_rTimer.start("Sync2A calc",m_i4SensorDev, m_i4LogEn, 1);
    // 2A sync: independent AE/AWB
    CAM_LOGD_IF(m_i4LogEn, "[%s] Sync 2A: Sensor(%d) =============", __FUNCTION__, m_i4SensorDev);
    MINT32 i4Sync = pSync3A->sync(m_i4SensorDev, i4SyncOpt, pAAOBuffer, i4FrameNum);
    CAM_LOGD_IF(m_i4LogEn, "[%s] Sync 2A: Sensor(%d) Ready to validate (%d)", __FUNCTION__, m_i4SensorDev, i4Sync);
    m_rTimer.End();

    if(getStatus() >= TASK_STATUS_CANCELLED)
        return eResult;

    //===== AE apply =====
    MBOOL bUpdateAeSetting = MTRUE;
    MBOOL bIsScheduleBy3A = MTRUE; // if bIsScheduleBy3A = 1, we can control AE behavior.
                                   // for example : E_AE_IDLE, E_AE_AE_CALC, E_AE_AE_APPLY, E_AE_FLARE
    MINT32 i4ActiveAEItem = E_AE_AE_APPLY; // if bIsScheduleBy3A = 0, AE don't care this value.

    m_rTimer.start("AE apply",m_i4SensorDev, m_i4LogEn, 1);

    MINT32 isMvHDREnable = Hal3ASttCtrl::getInstance(m_i4SensorDev)->isMvHDREnable();
    CAM_LOGD_IF(m_i4LogEn, "[%s] prepare MvHDR(%d)",__FUNCTION__, isMvHDREnable);

    {
        // Normal AE
        if(pAAOBuffer)
        {
            IAeMgr::getInstance().doPvAE(m_i4SensorDev,
                                         pAAOBuffer->mTimeStamp,
                                         reinterpret_cast<MVOID *>(pAAOBuffer->mVa),
                                         i4ActiveAEItem,
                                         pAAOBuffer->mMagicNumber,
                                         bIsScheduleBy3A);
        }
        else
            CAM_LOGW("[%s] AAO buffer is NULL", __FUNCTION__);
    }
    m_rTimer.End();

    if(bUpdateAeSetting)
    {
        // AE update sensor setting
        if (IAeMgr::getInstance().IsNeedUpdateSensor(m_i4SensorDev))
        {
#if USE_AE_THD
            if(pThreadRaw)
                pThreadRaw->postToEventThread(E_3AEvent_AE_I2C);
            else
                CAM_LOGW("[%s] ThreadRaw is NULL", __FUNCTION__);
#else
            IAeMgr::getInstance().updateSensorbyI2C(m_i4SensorDev);
#endif
        }
    }

    if(getStatus() >= TASK_STATUS_CANCELLED)
        return eResult;

    updateStatus(TASK_STATUS_RUNNING);
    m_i4FrameCount++;
    return eResult;
}

MVOID
TaskSync2AImp::cancel()
{
    updateStatus(TASK_STATUS_CANCELLED);
}

MVOID
TaskSync2AImp::waitFinish()
{
    CAM_LOGD("[%s] wait for Task done +", __FUNCTION__);
    Mutex::Autolock lock(m_RunningLock);

    MINT32 i4Status = getStatus();
    if(i4Status < TASK_STATUS_CANCELLED)
        cancel();

    updateStatus(TASK_STATUS_DONE);
    CAM_LOGD("[%s] wait for Task done -", __FUNCTION__);
}

MINT32
TaskSync2AImp::getStatus()
{
    Mutex::Autolock lock(m_StatusLock);
    return m_i4Status;
}

MVOID
TaskSync2AImp::updateStatus(MINT32 i4Status)
{
    Mutex::Autolock lock(m_StatusLock);
    if(m_i4Status != i4Status)
    {
        CAM_LOGD("[%s] status(%d->%d)", __FUNCTION__, m_i4Status, i4Status);
        m_i4Status = i4Status;
    }
}
