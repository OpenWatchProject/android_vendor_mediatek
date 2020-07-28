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
#define LOG_TAG "TaskAFNormal"

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
#include <IThreadRaw.h>
#include <debug/DebugUtil.h>

// 3A module
#include <af_feature.h>
#include <af_algo_if.h>
#include <ae_mgr/ae_mgr_if.h>
#include <af_mgr/af_mgr_if.h>
#include <flash_hal.h>
#include <flash_mgr.h>
// ISP
#include <isp_tuning/isp_tuning_mgr.h>

using namespace NS3Av3;

/*********************************************************
 *
 *   Interface of TaskAFNormalImp
 *
 *********************************************************/

class TaskAFNormalImp : public TaskAFNormal
{
public:
    virtual MVOID destroy();
    TaskAFNormalImp(MINT32 const i4SensorDev);
    virtual ~TaskAFNormalImp();

    virtual TASK_RESULT run(MINT32 i4Opt, const TaskData& rData);
    virtual MVOID cancel();
    virtual MVOID waitFinish();
    virtual MINT32 getStatus();
    virtual TaskInfo getInfo()
    {
        return m_rTaskInfo;
    };

private:
    virtual MVOID updateStatus(MINT32 i4Status);
    MINT32 m_i4SensorDev;
    MINT32 m_i4FrameCount;
    MINT32 m_i4Status;
    MINT32 m_i4LogEn;
    MUINT8 m_uAEBlockAreaY[120*90];
    AaaTimer m_rTimer;
    mutable Mutex m_StatusLock;
    mutable Mutex m_RunningLock;
};

/*********************************************************
 *
 *   TaskAFNormal implement functions
 *
 *********************************************************/

TaskAFNormal*
TaskAFNormal::create(MINT32 const i4SensorDev, MINT32 i4Prioirty)
{
    return new TaskAFNormalImp(i4SensorDev);
}

/*********************************************************
 *
 *   TaskAFNormalImp implement functions
 *
 *********************************************************/

TaskAFNormalImp::TaskAFNormalImp(MINT32 const i4SensorDev)
    : m_i4SensorDev(i4SensorDev)
    , m_i4FrameCount(0)
    , m_i4Status(TASK_STATUS_INIT)
    , m_i4LogEn(0)
{
    m_i4LogEn = property_get_int32("vendor.debug.hal3a.task",0);
}

TaskAFNormalImp::~TaskAFNormalImp()
{
}

MVOID
TaskAFNormalImp::destroy()
{
    CAM_LOGW("[%s] +", __FUNCTION__);
    waitFinish();
    delete this;
    CAM_LOGW("[%s] -", __FUNCTION__);
}

TASK_RESULT
TaskAFNormalImp::run(MINT32 i4Opt, const TaskData& rData)
{
    Mutex::Autolock lock(m_RunningLock);
    TASK_RESULT eResult = TASK_RESULT_OK;

    if(getStatus() >= TASK_STATUS_CANCELLED)
        return eResult;

    //===== prepare statistic data =====
    AAA_TRACE_HAL(prepareStatisticData);
    CameraArea_T  rWinSize;
    AEMeterArea_T rAeWinSize = {};
    AE2AFInfo_T rAEInfo = {};

    //===== prepare statistic data =====
    StatisticBufInfo* pAFOBuffer   = NULL;
    IThreadRaw*       pThreadRaw   = NULL;

    if(rData.pAFOBuffer)
        pAFOBuffer = reinterpret_cast<StatisticBufInfo*>(rData.pAFOBuffer);
    if(rData.pThreadRaw)
        pThreadRaw = reinterpret_cast<IThreadRaw*>(rData.pThreadRaw);

    AAA_TRACE_END_HAL;

    if(pAFOBuffer == NULL)
    {
        CAM_LOGW("[%s] AFO buffer is NULL", __FUNCTION__);
        eResult = TASK_RESULT_ERROR;
        return eResult;
    }

    if(getStatus() >= TASK_STATUS_CANCELLED)
        return eResult;

    //===== AF calculation =====
    //if(!(i4Opt & TASK_OPTION_BYP_AF))
    {
        AAA_TRACE_D("doPvAF");
        AAA_TRACE_MGR(doPvAF);
        m_rTimer.start("AF calc",m_i4SensorDev, m_i4LogEn, 1);
        //get AF window from AF, and set to AE meter, then get Y value.
        IAfMgr::getInstance().getAFRefWin(m_i4SensorDev, rWinSize);
        rAeWinSize.i4Left  = rWinSize.i4Left;
        rAeWinSize.i4Right = rWinSize.i4Right;
        rAeWinSize.i4Top   = rWinSize.i4Top;
        rAeWinSize.i4Bottom= rWinSize.i4Bottom;
        rAeWinSize.i4Weight= rWinSize.i4Weight;
        CAM_LOGD_IF(m_i4LogEn, "AFAEInfo[L]%d[R]%d[T]%d[B]%d\n",rAeWinSize.i4Left,rAeWinSize.i4Right,rAeWinSize.i4Top,rAeWinSize.i4Bottom);

        // AE2AFInfo
        rAEInfo.i4IsFlashFrm = FlashHal::getInstance(m_i4SensorDev)->isAFLampOn();
        rAEInfo.pAEBlockAreaYvalue = m_uAEBlockAreaY;

        IAeMgr::getInstance().getAE2AFInfo(m_i4SensorDev, rAeWinSize, rAEInfo);
        IAfMgr::getInstance().setAE2AFInfo(m_i4SensorDev, rAEInfo);
        CAM_LOGD_IF(m_i4LogEn,
                    "AFAEInfo : AEStable(%d), ISO(%d), LV(%d), Yvalue(%d), FlashFrm(%d)",
                    rAEInfo.i4IsAEStable,
                    rAEInfo.i4ISO,
                    rAEInfo.i4SceneLV,
                    (MINT32)rAEInfo.iYvalue,
                    rAEInfo.i4IsFlashFrm);
        // Adaptive Compensation
        AdptCompTimeData_T AdptCompTime;
        AdptCompTime.TS_AFDone = DebugUtil::getInstance(m_i4SensorDev)->TS_AFDone;
        IAfMgr::getInstance().setAdptCompInfo(m_i4SensorDev, AdptCompTime);
        IAfMgr::getInstance().doAF(m_i4SensorDev, reinterpret_cast<MVOID *>(pAFOBuffer));
        m_rTimer.End();
        AAA_TRACE_END_MGR;
        AAA_TRACE_END_D;
    }

    CAM_LOGD("[%s] Cam %d : Req(#%d)/Stt(#%d)/i4Opt(%d)/FlashOnOff(%d)",__FUNCTION__, m_i4SensorDev,
             rData.i4RequestMagic, pAFOBuffer->mMagicNumber, i4Opt,
             rData.bFlashOnOff);

    if(getStatus() >= TASK_STATUS_CANCELLED)
        return eResult;

    updateStatus(TASK_STATUS_RUNNING);
    m_i4FrameCount++;
    return TASK_RESULT_OK;
}

MVOID
TaskAFNormalImp::cancel()
{
    updateStatus(TASK_STATUS_CANCELLED);
}

MVOID
TaskAFNormalImp::waitFinish()
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
TaskAFNormalImp::getStatus()
{
    Mutex::Autolock lock(m_StatusLock);
    return m_i4Status;
}

MVOID
TaskAFNormalImp::updateStatus(MINT32 i4Status)
{
    Mutex::Autolock lock(m_StatusLock);
    if(m_i4Status != i4Status)
    {
        CAM_LOGD("[%s] status(%d->%d)", __FUNCTION__, m_i4Status, i4Status);
        m_i4Status = i4Status;
    }
}
