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
#define LOG_TAG "Task3APrecap"

#include <mtkcam/utils/std/Log.h>
#include <property_utils.h>
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
#include <ResultPool4Module.h>
#include <IThreadRaw.h>
// 3A module
#include <ae_mgr/ae_mgr_if.h>
#include <awb_mgr/awb_mgr_if.h>
#include <lsc_mgr/ILscTsf.h>
#include <flash_hal.h>
#include <flash_mgr.h>
#include <flash_feature.h>

using namespace NS3Av3;

/*********************************************************
 *
 *   Interface of Task3APreviewImp
 *
 *********************************************************/

class Task3APrecaptureImp : public Task3APrecapture
{
public:
    virtual MVOID destroy();
            Task3APrecaptureImp(MINT32 const i4SensorDev);
    virtual ~Task3APrecaptureImp();

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
    mutable std::mutex m_StatusLock;
    mutable std::mutex m_RunningLock;
};

/*********************************************************
 *
 *   Task3APrecapture implement functions
 *
 *********************************************************/

Task3APrecapture*
Task3APrecapture::create(MINT32 const i4SensorDev, MINT32 i4Prioirty)
{
    return new Task3APrecaptureImp(i4SensorDev);
}

/*********************************************************
 *
 *   Task3APrecaptureImp implement functions
 *
 *********************************************************/

Task3APrecaptureImp::Task3APrecaptureImp(MINT32 const i4SensorDev)
    : m_i4SensorDev(i4SensorDev)
    , m_i4FrameCount(0)
    , m_i4Status(TASK_STATUS_INIT)
    , m_i4LogEn(0)
{
    getPropInt("vendor.debug.hal3a.task", &m_i4LogEn, 0);
}

Task3APrecaptureImp::~Task3APrecaptureImp()
{
}

MVOID
Task3APrecaptureImp::destroy()
{
    CAM_LOGW("[%s] +", __FUNCTION__);
    waitFinish();
    delete this;
    CAM_LOGW("[%s] -", __FUNCTION__);
}

TASK_RESULT
Task3APrecaptureImp::run(MINT32 i4Opt, const TaskData& rData)
{
    std::lock_guard<std::mutex> lock(m_RunningLock);
    TASK_RESULT eResult = TASK_RESULT_OK;

    if(getStatus() >= TASK_STATUS_CANCELLED)
        return eResult;

    //===== prepare statistic data =====
    StatisticBufInfo* pAAOBuffer    = NULL;
    StatisticBufInfo* pTSFOBuffer   = NULL;
    StatisticBufInfo* pMvHdrBuffer  = NULL;
    IThreadRaw*       pThreadRaw    = NULL;

    if(rData.pAAOBuffer)
        pAAOBuffer = reinterpret_cast<StatisticBufInfo*>(rData.pAAOBuffer);
    if(rData.pTSFOBuffer)
        pTSFOBuffer = reinterpret_cast<StatisticBufInfo*>(rData.pTSFOBuffer);
    if(rData.pMvHdrBuffer)
        pMvHdrBuffer = reinterpret_cast<StatisticBufInfo*>(rData.pMvHdrBuffer);
    if(rData.pThreadRaw)
        pThreadRaw = reinterpret_cast<IThreadRaw*>(rData.pThreadRaw);

    if(getStatus() >= TASK_STATUS_CANCELLED)
        return eResult;

    if(pAAOBuffer == NULL || pTSFOBuffer == NULL)
    {
        CAM_LOGW("[%s] AAO buffer is NULL(%p) or TSFO buffer is NULL(%p)", __FUNCTION__, pAAOBuffer, pTSFOBuffer);
        eResult = TASK_RESULT_ERROR;
        return eResult;
    }

    // prepare AWB info to TSF and AE
    AWB_OUTPUT_T rAWBOutput;
    IAwbMgr::getInstance().getAWBOutput(m_i4SensorDev, rAWBOutput);

    //===== Backup AE & AWB =====
    if(getStatus() == TASK_STATUS_INIT)
    {
        MBOOL isFlashOn = FlashHal::getInstance(m_i4SensorDev)->isAFLampOn();
        if(isFlashOn)
        {
            MBOOL bIsAEContinueShot = MTRUE;
            IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetContinueShot, bIsAEContinueShot, NULL, NULL, NULL);
        }
    }

    AE_PERFRAME_INFO_T AEPerframeInfo;
    //===== AE calculation =====
    if(!(i4Opt & TASK_OPTION_BYP_AE))
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

        AAA_TRACE_D("doPrecapAE");
        m_rTimer.start("AE calc",m_i4SensorDev, m_i4LogEn, 1);

        MINT32 isMvHDREnable = Hal3ASttCtrl::getInstance(m_i4SensorDev)->isMvHDREnable();
        CAM_LOGD_IF(m_i4LogEn, "[%s] prepare MvHDR(%d)",__FUNCTION__, isMvHDREnable);

        //set awb gain to ae_mgr
        CCU_AAO_PROC_INFO_T rProcInfo;
        rProcInfo.awb_gain.i4R = rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R;
        rProcInfo.awb_gain.i4G = rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G;
        rProcInfo.awb_gain.i4B = rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B;
        IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetAAOProcInfo, NULL, reinterpret_cast<MINTPTR>(&(rProcInfo)), NULL, NULL);

        if(isMvHDREnable == FEATURE_MVHDR_SUPPORT_VIRTUAL_CHANNEL)
        {
            // MvHDR feature
            if(pMvHdrBuffer)
                IAeMgr::getInstance().doPreCapAE(m_i4SensorDev, pMvHdrBuffer->mTimeStamp, 0, reinterpret_cast<MVOID *>(pMvHdrBuffer->mVa), i4ActiveAEItem, pMvHdrBuffer->mMagicNumber, bIsScheduleBy3A);
            else
                CAM_LOGW("[%s] MvHDR buffer is NULL", __FUNCTION__);
        }
        else if(isMvHDREnable == FEATURE_MVHDR_SUPPORT_3EXPO_VIRTUAL_CHANNEL)
        {
            // MvHDR3EXPO feature
            if(pMvHdrBuffer && pAAOBuffer)
            {
                IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetMVHDR3ExpoProcInfo, reinterpret_cast<MINTPTR>((MVOID *)(pMvHdrBuffer->mVa)), pMvHdrBuffer->mMVHDRRatio_x100, NULL, NULL);
                // Call doPreCapAE after setMVHDR3ExpoProcInfo
                IAeMgr::getInstance().doPreCapAE(m_i4SensorDev, pAAOBuffer->mTimeStamp, 0, reinterpret_cast<MVOID *>((MVOID *)(pAAOBuffer->mVa)), i4ActiveAEItem, pAAOBuffer->mMagicNumber, bIsScheduleBy3A);
            }
            else
                CAM_LOGW("[%s] MvHDR && AAO buffer is NULL(%p, %p)", __FUNCTION__, pMvHdrBuffer, pAAOBuffer);
        }
        else
        {
            // Normal AE
            if(pAAOBuffer)
                IAeMgr::getInstance().doPreCapAE(m_i4SensorDev, pAAOBuffer->mTimeStamp, 0, reinterpret_cast<MVOID *>(pAAOBuffer->mVa), i4ActiveAEItem, pAAOBuffer->mMagicNumber, bIsScheduleBy3A);
            else
                CAM_LOGW("[%s] AAO buffer is NULL", __FUNCTION__);
        }
        m_rTimer.End();
        AAA_TRACE_END_D;

        if(getStatus() >= TASK_STATUS_CANCELLED)
            return eResult;

        IAeMgr::getInstance().getAEInfo(m_i4SensorDev, AEPerframeInfo);
        if(bUpdateAeSetting)
        {
            // AE update sensor setting
            if (AEPerframeInfo.rAEUpdateInfo.bIsNeedUpdateCPUstatus)
            {
#if USE_AE_THD
                if(pThreadRaw)
                    pThreadRaw->postToEventThread(E_3AEvent_AE_I2C);
                else
                    CAM_LOGW("[%s] ThreadRaw is NULL", __FUNCTION__);
#else
                IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetSensorbyI2C, NULL, NULL, NULL, NULL);
#endif
            }
        }
        m_i4FrameCount++;
        CAM_LOGD("[%s] Frame Count(%d) - doPrecapAE",__FUNCTION__, m_i4FrameCount);
    }

    if(getStatus() >= TASK_STATUS_CANCELLED)
        return eResult;


    IAeMgr::getInstance().getAEInfo(m_i4SensorDev, AEPerframeInfo);
    //===== AWB calculation =====
    if(!(i4Opt & TASK_OPTION_BYP_AWB))
    {
        AAA_TRACE_D("doPrecapAWB");

        MINT32 i4AoeCompLv = AEPerframeInfo.rAEUpdateInfo.i4AOECompLVvalue;
        if(AEPerframeInfo.rAEISPInfo.bAEStable)
        {
            m_rTimer.start("AWB calc",m_i4SensorDev, m_i4LogEn, 1);
            // One-shot AWB without strobe
            if(pAAOBuffer)
                IAwbMgr::getInstance().doPreCapAWB(m_i4SensorDev, i4AoeCompLv, reinterpret_cast<MVOID *>(pAAOBuffer->mVa));
            else
                CAM_LOGW("[%s] AAO buffer is NULL", __FUNCTION__);
            m_rTimer.End();
        }
        AAA_TRACE_END_D;
    }

    if(getStatus() >= TASK_STATUS_CANCELLED)
        return eResult;

    //===== Shading calculation =====
    if(!(i4Opt & TASK_OPTION_BYP_TSF))
    {
        AAA_TRACE_D("doTsf");
        m_rTimer.start("Shading calc",m_i4SensorDev, m_i4LogEn, 1);
        // TSF
        AWB_OUTPUT_T rAWBOutput;
        IAwbMgr::getInstance().getAWBOutput(m_i4SensorDev, rAWBOutput);
        ILscMgr::TSF_AWB_INFO rAwbInfo;
        ILscMgr::TSF_INPUT_INFO_T rTsfInfo;
        ILscMgr* pLsc = ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev));
        rAwbInfo.m_i4LV   = AEPerframeInfo.rAEISPInfo.i4LightValue_x10;
        rAwbInfo.m_u4CCT  = IAwbMgr::getInstance().getAWBCCT(m_i4SensorDev);
        rAwbInfo.m_RGAIN  = rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R;
        rAwbInfo.m_GGAIN  = rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G;
        rAwbInfo.m_BGAIN  = rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B;
        rAwbInfo.m_FLUO_IDX = rAWBOutput.rAWBInfo.i4FluorescentIndex;
        rAwbInfo.m_DAY_FLUO_IDX = rAWBOutput.rAWBInfo.i4DaylightFluorescentIndex;
        rAwbInfo.m_FLASH_ON = 0;        // Nelson Coverity
        rTsfInfo.eCmd = ILscMgr::E_TSF_CMD_RUN;
        rTsfInfo.u4FrmId = pAAOBuffer->mMagicNumber;
        rTsfInfo.rAwbInfo = rAwbInfo;
        rTsfInfo.prAwbStat = reinterpret_cast<MUINT8*>(pAAOBuffer->mVa);
        rTsfInfo.prTSFStat = reinterpret_cast<MUINT8*>(pTSFOBuffer->mVa);
        //rTsfInfo.u4SizeAwbStat = pAAOBuffer->getPartSize(AAOSepLSCBuf);

        LSC_INPUT_INFO_T rLscInputInfo;

        rLscInputInfo.u4ShadingCCT = pLsc->getCTIdx();
        rLscInputInfo.i4AWBCCT = IAwbMgr::getInstance().getAWBCCT(m_i4SensorDev);
        if( (reinterpret_cast<StatisticBufInfo*>(pAAOBuffer)->mFlashState == MTK_FLASH_STATE_PARTIAL) ||
                (reinterpret_cast<StatisticBufInfo*>(pAAOBuffer)->mFlashState == MTK_FLASH_STATE_FIRED))
            rLscInputInfo.isFlash  = MTRUE;
        else
            rLscInputInfo.isFlash  = MFALSE;
        rLscInputInfo.u4RealISOValue = AEPerframeInfo.rAEISPInfo.u4P1RealISOValue;
        ::memcpy(rLscInputInfo.pu4AEBlock, AEPerframeInfo.rAEUpdateInfo.pu4AEBlock, sizeof(MUINT32)*AE_BLOCK_NO*AE_BLOCK_NO);
        rLscInputInfo.bEnableRAFastConverge = AEPerframeInfo.rAEUpdateInfo.bEnableRAFastConverge;
        rLscInputInfo.u4MgrCWValue = AEPerframeInfo.rAEUpdateInfo.u4CWValue;
        rLscInputInfo.TgCtrlRight = AEPerframeInfo.rAEUpdateInfo.TgCtrlRight;
        rLscInputInfo.i4deltaIndex = AEPerframeInfo.rAEISPInfo.i4deltaIndex;
        rLscInputInfo.u4AEFinerEVIdxBase = AEPerframeInfo.rAEISPInfo.u4AEFinerEVIdxBase;
        rLscInputInfo.bAEStable = AEPerframeInfo.rAEISPInfo.bAEStable;
        rLscInputInfo.u4AvgWValue = AEPerframeInfo.rAEUpdateInfo.u4AvgY;
        rLscInputInfo.bAELock = AEPerframeInfo.rAEUpdateInfo.bAELock;
        rLscInputInfo.bAELimiter = AEPerframeInfo.rAEUpdateInfo.bAELimiter;
        pLsc->evaluate_cus_Ratio(rLscInputInfo, 0);
        pLsc->updateTsf(rTsfInfo);
        m_rTimer.End();
        AAA_TRACE_END_D;
    }

    //===== Print log =====
    MINT32 i4AeLv = 0;
    MUINT32 u4RealISO = 0;
    u4RealISO = AEPerframeInfo.rAEISPInfo.u4P1RealISOValue;
    i4AeLv = AEPerframeInfo.rAEISPInfo.i4LightValue_x10;
    CAM_LOGD("[%s] Cam %d : Req(#%d)/Stt(#%d)/i4Opt(%d)/FlashOnOff(%d) Lv(%d)/ISO(%d)/RGB(%d,%d,%d) FlashTask(%d)",__FUNCTION__, m_i4SensorDev,
        rData.i4RequestMagic, pAAOBuffer->mMagicNumber, i4Opt, rData.bFlashOnOff,
        i4AeLv, u4RealISO,
        rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R,
        rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G,
        rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B,
        rData.bExistFlashTask);

    //===== done =====
    if(AEPerframeInfo.rAEISPInfo.bAEStable)
    {
        //===== Backup AE & AWB =====
        MBOOL isFlashOn = FlashHal::getInstance(m_i4SensorDev)->isAFLampOn();
        if (!isFlashOn){
            IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetDoBackAEInfo, NULL, NULL, NULL, NULL);
            IAwbMgr::getInstance().backup(m_i4SensorDev);
        }
        updateStatus(TASK_STATUS_DONE);
        //no flash calibration
        //1. torch mode ||2. off mode ||3. auto mode && AEisBvtrigger false ||4. flashlight is charging
        // return remove flash task
        if( !rData.bExistFlashTask )
            IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetState2Converge, NULL, NULL, NULL, NULL);
        CAM_LOGD("[%s] Frame Count(%d) - Done",__FUNCTION__, m_i4FrameCount);
        return eResult;
    }

    //===== Timeout =====
    if(m_i4FrameCount >= 30)
    {
        //===== Backup AE & AWB =====
        MBOOL isFlashOn = FlashHal::getInstance(m_i4SensorDev)->isAFLampOn();
        if (!isFlashOn){
            IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetDoBackAEInfo, NULL, NULL, NULL, NULL);
            IAwbMgr::getInstance().backup(m_i4SensorDev);
        }
        updateStatus(TASK_STATUS_DONE);
        if( (FlashMgr::getInstance(m_i4SensorDev)->getFlashMode() == LIB3A_FLASH_MODE_FORCE_TORCH) || (FlashMgr::getInstance(m_i4SensorDev)->getFlashMode()== LIB3A_FLASH_MODE_FORCE_OFF)
            || (FlashMgr::getInstance(m_i4SensorDev)->getFlashMode() == LIB3A_FLASH_MODE_AUTO && AEPerframeInfo.rAEUpdateInfo.bStrobeBVTrigger == MFALSE)
            || (FlashMgr::getInstance(m_i4SensorDev)->getFlashState() == MTK_FLASH_STATE_CHARGING) )
        {
            eResult = TASK_RESULT_NO_FLASH_CALIBRATION;
            IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetState2Converge, NULL, NULL, NULL, NULL);
        }
        CAM_LOGD("[%s] Task is timeout(%d)",__FUNCTION__, m_i4FrameCount);
        return eResult;
    }

    if(getStatus() >= TASK_STATUS_CANCELLED)
        return eResult;

    updateStatus(TASK_STATUS_RUNNING);
    return eResult;
}

MVOID
Task3APrecaptureImp::cancel()
{
    IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetState2Converge, NULL, NULL, NULL, NULL);
    updateStatus(TASK_STATUS_CANCELLED);
}

MVOID
Task3APrecaptureImp::waitFinish()
{
    CAM_LOGD("[%s] wait for Task done +", __FUNCTION__);
    std::lock_guard<std::mutex> lock(m_RunningLock);

    MINT32 i4Status = getStatus();
    if(i4Status < TASK_STATUS_CANCELLED)
        cancel();

    updateStatus(TASK_STATUS_DONE);
    CAM_LOGD("[%s] wait for Task done -", __FUNCTION__);
}

MINT32
Task3APrecaptureImp::getStatus()
{
    std::lock_guard<std::mutex> lock(m_StatusLock);
    return m_i4Status;
}

MVOID
Task3APrecaptureImp::updateStatus(MINT32 i4Status)
{
    std::lock_guard<std::mutex> lock(m_StatusLock);
    if(m_i4Status == TASK_STATUS_INIT && i4Status == TASK_STATUS_RUNNING)
    {
        AAA_TRACE_D("Task3APrecapture_start");
        AAA_TRACE_END_D;
    }
    if(m_i4Status == TASK_STATUS_RUNNING && i4Status == TASK_STATUS_DONE)
    {
        AAA_TRACE_D("Task3APrecapture_end");
        AAA_TRACE_END_D;
    }
    if(m_i4Status != i4Status)
    {
        CAM_LOGD("[%s] status(%d->%d)", __FUNCTION__, m_i4Status, i4Status);
        m_i4Status = i4Status;
    }
}
