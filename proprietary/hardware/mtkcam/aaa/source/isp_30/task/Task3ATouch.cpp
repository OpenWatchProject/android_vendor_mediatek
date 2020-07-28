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
#define LOG_TAG "Task3ATouch"

#include <mtkcam/utils/std/Log.h>
#include <cutils/properties.h>
#include <mtkcam/utils/std/Trace.h>
// 3A framework
#include <task/ITask.h>
#include <aaa_trace.h>
#include <aaa_types.h>
#include <aaa_timer.h>
#include <aaa_common_custom.h>
#include <aaa_scheduling_custom.h>
#include <aaa_hal_sttCtrl.h>
#include <IResultPool.h>
#include <IThreadRaw.h>
// 3A module
#include <ae_mgr/ae_mgr_if.h>
#include <awb_mgr/awb_mgr_if.h>
#include <flash_mgr.h>
#include <flash_hal.h>
#include <flash_tuning_custom.h>
// ISP
#include <isp_tuning_mgr.h>

using namespace NS3Av3;

/*********************************************************
 *
 *   Interface of Task3ATouchImp
 *
 *********************************************************/

class Task3ATouchImp : public Task3ATouch
{
public:
    virtual MVOID destroy();
            Task3ATouchImp(MINT32 const i4SensorDev);
    virtual ~Task3ATouchImp();

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
 *   Task3ATouch implement functions
 *
 *********************************************************/

Task3ATouch*
Task3ATouch::create(MINT32 const i4SensorDev)
{
    return new Task3ATouchImp(i4SensorDev);
}

/*********************************************************
 *
 *   Task3ATouchImp implement functions
 *
 *********************************************************/

Task3ATouchImp::Task3ATouchImp(MINT32 const i4SensorDev)
    : m_i4SensorDev(i4SensorDev)
    , m_i4FrameCount(0)
    , m_i4Status(TASK_STATUS_INIT)
    , m_i4LogEn(0)
{
    m_i4LogEn = property_get_int32("debug.hal3a.task",0);
}

Task3ATouchImp::~Task3ATouchImp()
{
}

MVOID
Task3ATouchImp::destroy()
{
    CAM_LOGW("[%s] +", __FUNCTION__);
    waitFinish();
    delete this;
    CAM_LOGW("[%s] -", __FUNCTION__);
}

TASK_RESULT
Task3ATouchImp::run(MINT32 i4Opt, const TaskData& rData)
{
    Mutex::Autolock lock(m_RunningLock);
    TASK_RESULT eResult = TASK_RESULT_OK;

    if(getStatus() >= TASK_STATUS_CANCELLED)
        return eResult;

    //===== prepare statistic data =====
    StatisticBufInfo* pAAOBuffer   = NULL;
    IThreadRaw*       pThreadRaw   = NULL;

    if(rData.pAAOBuffer)
        pAAOBuffer = reinterpret_cast<StatisticBufInfo*>(rData.pAAOBuffer);
    if(rData.pThreadRaw)
        pThreadRaw = reinterpret_cast<IThreadRaw*>(rData.pThreadRaw);

    if(pAAOBuffer == NULL)
    {
        CAM_LOGW("[%s] AAO buffer is NULL", __FUNCTION__);
        eResult = TASK_RESULT_ERROR;
        return eResult;
    }

    //===== check falsh ready =====
    {
        MBOOL bFlashReady = MFALSE;
        if(rData.bFlashOnOff || rData.bFlashOpenByTask)
        {
            if(pAAOBuffer->mFlashState == MTK_FLASH_STATE_FIRED)
                bFlashReady = MTRUE;
            else
                CAM_LOGD("[%s] Flash(%d/%d) FlashState(%d)",__FUNCTION__, rData.bFlashOnOff, rData.bFlashOpenByTask, pAAOBuffer->mFlashState);
        }
        else
            bFlashReady = MTRUE;

        if(!bFlashReady)
        {
            CAM_LOGD("[%s] Flash not ready, Cam %d : Req(#%d)/Stt(#%d)/i4Opt(%d)/FlashOnOff(%d)/FlashTypeByTask(%d)/FlashOpenByTask(%d)", __FUNCTION__, m_i4SensorDev,
                rData.i4RequestMagic, pAAOBuffer->mMagicNumber, i4Opt, rData.bFlashOnOff, rData.i4FlashTypeByTask, rData.bFlashOpenByTask);
            return eResult;
        }
    }

    //===== Open Flash or not =====
    if(getStatus() == TASK_STATUS_INIT)
    {
        /*MBOOL isNeedAFLamp = cust_isNeedAFLamp(
                  FlashMgr::getInstance(m_i4SensorDev)->getFlashMode(),
                  FlashMgr::getInstance(m_i4SensorDev)->getAfLampMode(),
                  IAeMgr::getInstance().IsStrobeBVTrigger(m_i4SensorDev));
        if(isNeedAFLamp && !rData.bFlashOnOff)
            eResult = TASK_RESULT_AFLAMP_REQUIRE;
        CAM_LOGD("eResult:%d isNeedAFLamp:%d flashM:%d AfLampM:%d triger:%d ",
                  eResult, isNeedAFLamp,
                  FlashMgr::getInstance(m_i4SensorDev)->getFlashMode(),
                  FlashMgr::getInstance(m_i4SensorDev)->getAfLampMode(),
                  IAeMgr::getInstance().IsStrobeBVTrigger(m_i4SensorDev));*/
    }

    //===== AE calculation =====
    if( (!(i4Opt & TASK_OPTION_BYP_AE)) )
    {
        MBOOL bUpdateAeSetting = MTRUE;
        MBOOL bIsScheduleBy3A = MFALSE; // if bIsScheduleBy3A = 1, we can control AE behavior.
                                        // for example : E_AE_IDLE, E_AE_AE_CALC, E_AE_AE_APPLY, E_AE_FLARE
        MINT32 i4ActiveAEItem = 0; // if bIsScheduleBy3A = 0, AE don't care this value.

        if(i4Opt & TASK_OPTION_ONLY_AE_CALC)
        {
            bUpdateAeSetting = MFALSE;
            bIsScheduleBy3A = MTRUE;
            i4ActiveAEItem = E_AE_AE_CALC;
        }

        AAA_TRACE_D("doAFAE");
        m_rTimer.start("AE calc",m_i4SensorDev, m_i4LogEn, 1);

        MINT32 isMvHDREnable = Hal3ASttCtrl::getInstance(m_i4SensorDev)->isMvHDREnable();
        CAM_LOGD_IF(m_i4LogEn, "[%s] prepare MvHDR(%d)",__FUNCTION__, isMvHDREnable);
        {
            // Normal AE
            if(pAAOBuffer)
            {
                IAeMgr::getInstance().doAFAE(m_i4SensorDev,
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
        AAA_TRACE_END_D;

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
        m_i4FrameCount++;
    }

    if(getStatus() >= TASK_STATUS_CANCELLED)
        return TASK_RESULT_OK;

    //===== AWB calculation =====
    if(!(i4Opt & TASK_OPTION_BYP_AWB))
    {
        AAA_TRACE_D("doAFAWB");
        m_rTimer.start("AWB calc",m_i4SensorDev, m_i4LogEn, 1);
        MINT32 i4AoeCompLv = IAeMgr::getInstance().getAOECompLVvalue(m_i4SensorDev, MFALSE);
        if(pAAOBuffer)
            IAwbMgr::getInstance().doAFAWB(m_i4SensorDev, i4AoeCompLv, reinterpret_cast<MVOID *>(pAAOBuffer->mVa));
        else
            CAM_LOGW("[%s] AAO buffer is NULL", __FUNCTION__);
        m_rTimer.End();
        AAA_TRACE_END_D;

        if(getStatus() >= TASK_STATUS_CANCELLED)
            return TASK_RESULT_OK;
    }

    //===== Print log =====
    MINT32 i4AeLv = 0;
    MUINT32 u4RealISO = 0;
    AWB_OUTPUT_T rAWBOutput;
    AE_MODE_CFG_T rAeInfo;

    IAeMgr::getInstance().getPreviewParams(m_i4SensorDev, rAeInfo);
    u4RealISO = rAeInfo.u4RealISO;
    i4AeLv = IAeMgr::getInstance().getLVvalue(m_i4SensorDev, MTRUE);
    IAwbMgr::getInstance().getAWBOutput(m_i4SensorDev, rAWBOutput);
    CAM_LOGD("[%s] Cam %d : Req(#%d)/Stt(#%d)/i4Opt(%d)/FlashOnOff(%d)/FlashTypeByTask(%d)/FlashOpenByTask(%d) Lv(%d)/ISO(%d)/RGB(%d,%d,%d)",__FUNCTION__, m_i4SensorDev,
        rData.i4RequestMagic, pAAOBuffer->mMagicNumber, i4Opt, rData.bFlashOnOff, rData.i4FlashTypeByTask, rData.bFlashOpenByTask,
        i4AeLv, u4RealISO,
        rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R,
        rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G,
        rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B);

    updateStatus(TASK_STATUS_RUNNING);

    //===== done =====
    if(IAeMgr::getInstance().IsAEStable(m_i4SensorDev))
    {
        if(rData.bFlashOnOff || rData.bFlashOpenByTask)
            IAeMgr::getInstance().doBackAEInfo(m_i4SensorDev, MTRUE);
#if CAM3_FLASH_FEATURE_EN
        if(rData.i4FlashTypeByTask == FLASH_HAL_SCENARIO_PRE_FLASH && rData.bFlashOpenByTask && !rData.bFlashOnOff)
            FlashMgr::getInstance(m_i4SensorDev)->setCapPara();
#endif
        updateStatus(TASK_STATUS_DONE);
        CAM_LOGD("[%s] Frame Count(%d)",__FUNCTION__, m_i4FrameCount);
        return eResult;
    }

    //===== Timeout =====
    if(m_i4FrameCount >= 30)
    {
        if(rData.bFlashOnOff || rData.bFlashOpenByTask)
            IAeMgr::getInstance().doBackAEInfo(m_i4SensorDev, MTRUE);
#if CAM3_FLASH_FEATURE_EN
        if(rData.i4FlashTypeByTask ==  FLASH_HAL_SCENARIO_PRE_FLASH && rData.bFlashOpenByTask && !rData.bFlashOnOff)
            FlashMgr::getInstance(m_i4SensorDev)->setCapPara();
#endif
        updateStatus(TASK_STATUS_DONE);
        CAM_LOGD("[%s] Task is timeout(%d)",__FUNCTION__, m_i4FrameCount);
        return eResult;
    }

    if(getStatus() >= TASK_STATUS_CANCELLED)
        return eResult;

    return eResult;
}

MVOID
Task3ATouchImp::cancel()
{
    updateStatus(TASK_STATUS_CANCELLED);
}

MVOID
Task3ATouchImp::waitFinish()
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
Task3ATouchImp::getStatus()
{
    Mutex::Autolock lock(m_StatusLock);
    return m_i4Status;
}

MVOID
Task3ATouchImp::updateStatus(MINT32 i4Status)
{
    Mutex::Autolock lock(m_StatusLock);
    if(m_i4Status == TASK_STATUS_INIT && i4Status == TASK_STATUS_RUNNING)
    {
        AAA_TRACE_D("Task3ATouch_start");
        AAA_TRACE_END_D;
    }
    if(m_i4Status == TASK_STATUS_RUNNING && i4Status == TASK_STATUS_DONE)
    {
        AAA_TRACE_D("Task3ATouch_end");
        AAA_TRACE_END_D;
    }
    if(m_i4Status != i4Status)
    {
        CAM_LOGD("[%s] status(%d->%d)", __FUNCTION__, m_i4Status, i4Status);
        m_i4Status = i4Status;
    }
}
