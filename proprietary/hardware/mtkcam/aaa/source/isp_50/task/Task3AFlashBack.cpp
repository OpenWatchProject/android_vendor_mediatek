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
#define LOG_TAG "Task3AFlashBack"

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
#include <file_utils.h>
// 3A module
#include <af_feature.h>
#include <af_algo_if.h>
#include <ae_mgr/ae_mgr_if.h>
#include <af_mgr/af_mgr_if.h>
#include <awb_mgr/awb_mgr_if.h>
#include <lsc_mgr/ILscTsf.h>
#include <flash_hal.h>
#include <flash_mgr.h>
// 3A Custom
#include <awb_tuning_custom.h>
#include <flash_awb_tuning_custom.h>
#include <aaa_scheduling_custom.h>
// ISP
#include <isp_tuning/isp_tuning_mgr.h>

//ResultPool
#include <IResultPool.h>

using namespace NS3Av3;

/*********************************************************
 *
 *   Interface of Task3AFlashBackImp
 *
 *********************************************************/

class Task3AFlashBackImp : public Task3AFlashBack
{
public:
    virtual MVOID destroy();
            Task3AFlashBackImp(MINT32 const i4SensorDev);
    virtual ~Task3AFlashBackImp();

    virtual TASK_RESULT run(MINT32 i4Opt, const TaskData& rData);
    virtual MVOID cancel();
    virtual MVOID waitFinish();
    virtual MINT32 getStatus();
    virtual TaskInfo getInfo(){return m_rTaskInfo;};

private:
    virtual MVOID updateStatus(MINT32 i4Status);
    MINT32 m_i4SensorDev;
    MINT32 m_i4FrameCount;
    MINT32 m_i4FlashOffFrameCount;
    MVOID* m_pLastAAOBuffer;
    MINT32 m_i4Exp;
    MINT32 m_i4Afe;
    MINT32 m_i4Isp;
    MINT32 m_i4Status;
    MINT32 m_i4LogEn;
    AaaTimer m_rTimer;
    mutable Mutex m_StatusLock;
    mutable Mutex m_RunningLock;
};

/*********************************************************
 *
 *   Task3AFlash implement functions
 *
 *********************************************************/

Task3AFlashBack*
Task3AFlashBack::create(MINT32 const i4SensorDev, MINT32 i4Prioirty)
{
    return new Task3AFlashBackImp(i4SensorDev);
}

/*********************************************************
 *
 *   Task3AFlashBackImp implement functions
 *
 *********************************************************/

Task3AFlashBackImp::Task3AFlashBackImp(MINT32 const i4SensorDev)
    : m_i4SensorDev(i4SensorDev)
    , m_i4FrameCount(0)
    , m_i4FlashOffFrameCount(0)
    , m_pLastAAOBuffer(NULL)
    , m_i4Exp(0)
    , m_i4Afe(0)
    , m_i4Isp(0)
    , m_i4Status(TASK_STATUS_INIT)
    , m_i4LogEn(0)
{
    m_i4LogEn = property_get_int32("vendor.debug.hal3a.task",0);
}

Task3AFlashBackImp::~Task3AFlashBackImp()
{
}

MVOID
Task3AFlashBackImp::destroy()
{
    CAM_LOGW("[%s] +", __FUNCTION__);
    waitFinish();
    delete this;
    CAM_LOGW("[%s] -", __FUNCTION__);
}

TASK_RESULT
Task3AFlashBackImp::run(MINT32 i4Opt, const TaskData& rData)
{
    Mutex::Autolock lock(m_RunningLock);
    TASK_RESULT eResult = TASK_RESULT_OK;

    if(getStatus() >= TASK_STATUS_CANCELLED)
        return eResult;

    //===== prepare statistic data =====
    StatisticBufInfo* pAAOBuffer = NULL;
    StatisticBufInfo* pMvHdrBuffer = NULL;
    StatisticBufInfo* pPSOBuffer   = NULL;
    IThreadRaw*       pThreadRaw = NULL;

    if(rData.pAAOBuffer)
        pAAOBuffer = reinterpret_cast<StatisticBufInfo*>(rData.pAAOBuffer);
    if(rData.pMvHdrBuffer)
        pMvHdrBuffer = reinterpret_cast<StatisticBufInfo*>(rData.pMvHdrBuffer);
    if(rData.pPSOBuffer)
        pPSOBuffer = reinterpret_cast<StatisticBufInfo*>(rData.pPSOBuffer);
    if(rData.pThreadRaw)
        pThreadRaw = reinterpret_cast<IThreadRaw*>(rData.pThreadRaw);

    if(pAAOBuffer == NULL)
    {
        CAM_LOGW("[%s] AAO buffer is NULL", __FUNCTION__);
        eResult = TASK_RESULT_ERROR;
        return eResult;
    }

    if(getStatus() >= TASK_STATUS_CANCELLED)
        return eResult;

    //===== Print log =====
    MINT32 i4AeLv = 0;
    MUINT32 u4RealISO = 0;
    AWB_OUTPUT_T rAWBOutput = {};
    AE_MODE_CFG_T rAeInfo;

    IAeMgr::getInstance().getPreviewParams(m_i4SensorDev, rAeInfo);
    u4RealISO = rAeInfo.u4RealISO;
    i4AeLv = IAeMgr::getInstance().getLVvalue(m_i4SensorDev, MTRUE);
    IAwbMgr::getInstance().getAWBOutput(m_i4SensorDev, rAWBOutput);
    CAM_LOGD("[%s] Cam %d : Req(#%d)/Stt(#%d)/i4Opt(%d)/FlashOnOff(%d) Lv(%d)/ISO(%d)/RGB(%d,%d,%d)",__FUNCTION__, m_i4SensorDev,
        rData.i4RequestMagic, pAAOBuffer->mMagicNumber, i4Opt, rData.bFlashOnOff,
        i4AeLv, u4RealISO,
        rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R,
        rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G,
        rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B);

#if CAM3_FLASH_FEATURE_EN
    if(!(i4Opt & TASK_OPTION_BYP_FLASH_BACK)){
        AAA_TRACE_D("doFlash");
        m_rTimer.start("Flash calc",m_i4SensorDev, m_i4LogEn, 1);

        //===== Flash Opened Outside, Bypass Task =====
        if(rData.bFlashOnOff){
            IAeMgr::getInstance().setAEState2Converge(m_i4SensorDev);
            FlashMgr::getInstance(m_i4SensorDev)->endPrecapture();
            updateStatus(TASK_STATUS_DONE);
            CAM_LOGD("[%s] Flash Opened by Torch/MainFlash, bypass this task",__FUNCTION__);
        }
        //===== Wait Close Flash =====
        if(rData.i4FlashTypeByTask == FLASH_HAL_SCENARIO_PRE_FLASH && rData.bFlashOpenByTask && !rData.bFlashOnOff){
            eResult = TASK_RESULT_CLOSE_PREFLASH_REQUIRE;
            m_i4Exp = rAeInfo.u4Eposuretime;
            m_i4Afe = rAeInfo.u4AfeGain;
            m_i4Isp = rAeInfo.u4IspGain;
            CAM_LOGD("[%s] Frame Count(%d), WAIT CLOSE FLASH, addr(%x)",__FUNCTION__, m_i4FrameCount, pAAOBuffer->mVa);

            // dump AE EXIF for Flash debug
            if(DebugUtil::getDebugLevel(DBG_FLASH))
            {
                MINT32 flashCount = 0;
                char fileName[512];
                char folderName[512];
                getFileCount("/sdcard/flash_file_cnt.txt", &flashCount, 0);
                sprintf(folderName, "/sdcard/flashdata/pf/");
                sprintf(fileName, "/sdcard/flashdata/pf/ae_exif_back_%03d.bin", flashCount);

                AE_DEBUG_INFO_T rAEDebugInfo;
                AE_PLINE_DEBUG_INFO_T rAEPlineDebugInfo;
                IAeMgr::getInstance().getDebugInfo(m_i4SensorDev, rAEDebugInfo, rAEPlineDebugInfo);
                dumpBufToFile(fileName, folderName, &rAEDebugInfo, sizeof(AE_DEBUG_INFO_T));
            }
        }
        //===== Get Flash Open Data =====
        if(rData.i4FlashTypeByTask == -1 && !rData.bFlashOpenByTask && !rData.bFlashOnOff && m_i4FlashOffFrameCount == 0){
            m_i4FlashOffFrameCount = m_i4FrameCount;
            m_pLastAAOBuffer = reinterpret_cast<MVOID *>(pAAOBuffer->mVa);
            FlashMgr::getInstance(m_i4SensorDev)->setFlashOnAAOBuffer(m_pLastAAOBuffer);
            CAM_LOGD("[%s] Frame Count(%d) Off(%d), GET FLASH OPEN DATA, addr(%x)",__FUNCTION__, m_i4FrameCount, m_i4FlashOffFrameCount, pAAOBuffer->mVa);
        }
        //===== Bypass Flash Partial Data =====
        if(m_i4FlashOffFrameCount != 0 && m_i4FrameCount - m_i4FlashOffFrameCount == 1){
            CAM_LOGD("[%s] Frame Count(%d) Off(%d), BYPASS PARTIAL, addr(%x)",__FUNCTION__, m_i4FrameCount, m_i4FlashOffFrameCount, pAAOBuffer->mVa);
        }
        //===== Get Flash Close Data =====
        if(m_i4FlashOffFrameCount != 0 && m_i4FrameCount - m_i4FlashOffFrameCount == 2){
            CAM_LOGD("[%s] Frame Count(%d) Off(%d), GET FLASH CLOSE DATA, addr(%p/%x)",
                        __FUNCTION__, m_i4FrameCount, m_i4FlashOffFrameCount, m_pLastAAOBuffer, pAAOBuffer->mVa);
            m_i4FlashOffFrameCount = 0;
            if(pAAOBuffer && m_pLastAAOBuffer)
            {
                //for temp solution
                FlashExePara para;
                FlashExeRep rep;
                FLASH_AWB_PASS_FLASH_INFO_T passFlashAwbData;
                AWB_STAT_PARAM_T rAWBStatParam;
                memset(&rep, 0, sizeof(rep));
                rAWBStatParam.i4WindowNumX = 120;
                rAWBStatParam.i4WindowNumY = 90;

                AWB_OUTPUT_T awb_out = {};
                IAwbMgr::getInstance().getAWBOutput(m_i4SensorDev, awb_out);

                para.staBuf =reinterpret_cast<MVOID *>(pAAOBuffer->mVa);
                para.staX = rAWBStatParam.i4WindowNumX;
                para.staY = rAWBStatParam.i4WindowNumY;
                para.NonPreferencePrvAWBGain = awb_out.rPreviewFullWBGain;
                para.PreferencePrvAWBGain = awb_out.rPreviewAWBGain;
                para.i4AWBCCT = awb_out.rAWBInfo.i4CCT;
                FlashMgr::getInstance(m_i4SensorDev)->setAlgoExpPara(rAeInfo.u4Eposuretime, rAeInfo.u4AfeGain, rAeInfo.u4IspGain, 0);
                FlashMgr::getInstance(m_i4SensorDev)->doPfOneFrame(&para, &rep);
                CAM_LOGW("[%s] doPfOneFrame no-flash", __FUNCTION__);
                //para.staBuf =reinterpret_cast<MVOID *>(m_pLastAAOBuffer);
                para.staBuf = reinterpret_cast<MVOID *>(FlashMgr::getInstance(m_i4SensorDev)->getFlashOnAAOBuffer());
                FlashMgr::getInstance(m_i4SensorDev)->setAlgoExpPara(m_i4Exp, m_i4Afe, m_i4Isp, 1);
                FlashMgr::getInstance(m_i4SensorDev)->doPfOneFrame(&para, &rep);
                CAM_LOGW("[%s] doPfOneFrame flash", __FUNCTION__);
                if (isFlashAWBv2Enabled(m_i4SensorDev)) {
                    passFlashAwbData.flashDuty = rep.nextDuty;
                    passFlashAwbData.flashAwbWeight = rep.flashAwbWeight;
                    passFlashAwbData.flashStep = 0;
                    IAwbMgr::getInstance().setFlashAWBData(m_i4SensorDev, passFlashAwbData);
                    CAM_LOGD("[%s] duty(%d) weight(%d)", __FUNCTION__, passFlashAwbData.flashDuty, passFlashAwbData.flashAwbWeight);
                }
                FlashMgr::getInstance(m_i4SensorDev)->endPrecapture();
            } else {
                CAM_LOGW("[%s] AAO buffer is NULL", __FUNCTION__);
            }
            IAeMgr::getInstance().setAEState2Converge(m_i4SensorDev);
            updateStatus(TASK_STATUS_DONE);
        }

        m_i4FrameCount++;
        CAM_LOGD("[%s] Frame Count(%d)",__FUNCTION__, m_i4FrameCount);
        m_rTimer.End();
        AAA_TRACE_END_D;
    }
#endif
    //===== Timeout =====
    if(m_i4FrameCount >= 70) // AF priority higher, set as AF Timeout + 10
    {
        IAeMgr::getInstance().setAEState2Converge(m_i4SensorDev);
        FlashMgr::getInstance(m_i4SensorDev)->endPrecapture();
        updateStatus(TASK_STATUS_DONE);
        CAM_LOGD("[%s] Task is timeout(%d)",__FUNCTION__, m_i4FrameCount);
    }

    if(getStatus() >= TASK_STATUS_CANCELLED)
        return eResult;

    updateStatus(TASK_STATUS_RUNNING);
    //m_i4FrameCount++;
    return eResult;
}

MVOID
Task3AFlashBackImp::cancel()
{
    IAeMgr::getInstance().setAEState2Converge(m_i4SensorDev);
    FlashMgr::getInstance(m_i4SensorDev)->endPrecapture();
    updateStatus(TASK_STATUS_CANCELLED);
}

MVOID
Task3AFlashBackImp::waitFinish()
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
Task3AFlashBackImp::getStatus()
{
    Mutex::Autolock lock(m_StatusLock);
    return m_i4Status;
}

MVOID
Task3AFlashBackImp::updateStatus(MINT32 i4Status)
{
    Mutex::Autolock lock(m_StatusLock);
    if(m_i4Status == TASK_STATUS_INIT && i4Status == TASK_STATUS_RUNNING)
    {
        AAA_TRACE_D("Task3AFlashBack_start");
        AAA_TRACE_END_D;
    }
    if(m_i4Status == TASK_STATUS_RUNNING && i4Status == TASK_STATUS_DONE)
    {
        AAA_TRACE_D("Task3AFlashBack_end");
        AAA_TRACE_END_D;
    }
    if(m_i4Status != i4Status)
    {
        CAM_LOGD("[%s] status(%d->%d)", __FUNCTION__, m_i4Status, i4Status);
        m_i4Status = i4Status;
    }
}
