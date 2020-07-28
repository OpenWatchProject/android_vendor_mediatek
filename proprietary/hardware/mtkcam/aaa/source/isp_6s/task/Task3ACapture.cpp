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
#define LOG_TAG "Task3ACapture"

#include <task/ITask.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Trace.h>
#include <property_utils.h>
#include <sys/stat.h>
// 3A framework
#include <aaa_trace.h>
#include <aaa_types.h>
#include <aaa_timer.h>
#include <aaa_common_custom.h>
#include <aaa_hal_sttCtrl.h>
#include <IResultPool.h>
#include <ResultPool4Module.h>
#include <IThreadRaw.h>
// 3A module
#include <af_feature.h>
#include <af_algo_if.h>
#include <ae_mgr/ae_mgr_if.h>
#include <af_mgr/af_mgr_if.h>
#include <awb_mgr/awb_mgr_if.h>
#include <lsc_mgr/ILscTsf.h>
#include <flash_mgr.h>
#include <flash_feature.h>

// 3A Custom
#include <awb_tuning_custom.h>
#include <flash_awb_tuning_custom.h>

#define FILE_NAME_SIZE 64

using namespace NS3Av3;

static MINT32 m_i4PreCapFrmId=0;

/*********************************************************
 *
 *   Interface of Task3ACaptureImp
 *
 *********************************************************/

class Task3ACaptureImp : public Task3ACapture
{
public:
    virtual MVOID destroy();
            Task3ACaptureImp(MINT32 const i4SensorDev);
    virtual ~Task3ACaptureImp();

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
    MINT32 m_i4DumpEn;
    MINT32 m_i4RequestMagic;
    AaaTimer m_rTimer;
    mutable std::mutex m_StatusLock;
    mutable std::mutex m_RunningLock;
};

/*********************************************************
 *
 *   Task3ACapture implement functions
 *
 *********************************************************/

Task3ACapture*
Task3ACapture::create(MINT32 const i4SensorDev, MINT32 i4Prioirty)
{
    return new Task3ACaptureImp(i4SensorDev);
}

/*********************************************************
 *
 *   Task3ACaptureImp implement functions
 *
 *********************************************************/

Task3ACaptureImp::Task3ACaptureImp(MINT32 const i4SensorDev)
    : m_i4SensorDev(i4SensorDev)
    , m_i4FrameCount(0)
    , m_i4Status(TASK_STATUS_INIT)
    , m_i4LogEn(0)
    , m_i4RequestMagic(0)
{
    getPropInt("vendor.debug.hal3a.task", &m_i4LogEn, 0);
    getPropInt("vendor.debug.camera.AAO.dump", &m_i4DumpEn, 0);
}

Task3ACaptureImp::~Task3ACaptureImp()
{
}

MVOID
Task3ACaptureImp::destroy()
{
    CAM_LOGW("[%s] +", __FUNCTION__);
    waitFinish();
    delete this;
    CAM_LOGW("[%s] -", __FUNCTION__);
}

static MBOOL _dumpHwBuf(char* fileName, char* folderName, StatisticBufInfo& rLastBuf, MBOOL isHwBuf = 0)
{
    FILE *fp = fopen(fileName, "w");
    if (NULL == fp)
    {
        CAM_LOGE("fail to open file to save img: %s", fileName);
        MINT32 err = mkdir(folderName, S_IRWXU | S_IRWXG | S_IRWXO);
        CAM_LOGD("err = %d", err);
        return MFALSE;
    } else
    {
        fwrite(reinterpret_cast<void *>(rLastBuf.mVa), 1, rLastBuf.mSize, fp);
        fclose(fp);
        return MTRUE;
    }
}

TASK_RESULT
Task3ACaptureImp::run(MINT32 i4Opt, const TaskData& rData)
{
    std::lock_guard<std::mutex> lock(m_RunningLock);

    if(getStatus() >= TASK_STATUS_CANCELLED)
        return TASK_RESULT_OK;

    //===== prepare statistic data =====
    StatisticBufInfo* pAAOBuffer  = NULL;
    StatisticBufInfo* pTSFOBuffer = NULL;

    if(rData.pAAOBuffer)
        pAAOBuffer = reinterpret_cast<StatisticBufInfo*>(rData.pAAOBuffer);
    if(rData.pTSFOBuffer)
        pTSFOBuffer = reinterpret_cast<StatisticBufInfo*>(rData.pTSFOBuffer);

    if( (pAAOBuffer == NULL || pTSFOBuffer == NULL) && getStatus() != TASK_STATUS_INIT )
    {
        CAM_LOGW("[%s] AAO buffer is NULL(%p) or TSFO buffer is NULL(%p)", __FUNCTION__, pAAOBuffer, pTSFOBuffer);
        return TASK_RESULT_ERROR;
    }

    if(pAAOBuffer == NULL)
        CAM_LOGD("[%s] Cam %d : Req(#%d)/i4Opt(%d)/FlashOnOff(%d)/HQC(%d)",__FUNCTION__, m_i4SensorDev,
            rData.i4RequestMagic, i4Opt,
            rData.bFlashOnOff, rData.bIsHqCap);
    else
        CAM_LOGD("[%s] Cam %d : Req(#%d)/Stt(#%d)/i4Opt(%d)/FlashOnOff(%d)/HQC(%d)",__FUNCTION__, m_i4SensorDev,
            rData.i4RequestMagic, pAAOBuffer->mMagicNumber, i4Opt,
            rData.bFlashOnOff, rData.bIsHqCap);

    //===== prepare params =====
    MINT32 i4RequestMagic = -1;
    MINT32 i4StatisticMagic = -1;
    MINT32 bFlashOnOff = -1;

    if(pAAOBuffer)
        i4StatisticMagic = pAAOBuffer->mMagicNumber;
    i4RequestMagic = rData.i4RequestMagic;
    bFlashOnOff = rData.bFlashOnOff;

    if(getStatus() >= TASK_STATUS_CANCELLED)
        return TASK_RESULT_OK;

    // prepare AWB info to TSF and AE
    AWB_OUTPUT_T rAWBOutput;
    IAwbMgr::getInstance().getAWBOutput(m_i4SensorDev, rAWBOutput);

    if(getStatus() == TASK_STATUS_INIT)
    {
        m_i4RequestMagic = i4RequestMagic;
        //===== Capture Start =====
        m_rTimer.start("Capture Start",m_i4SensorDev, m_i4LogEn, 1);
        CAM_LOGD("[%s] Capture Start #(%d)", __FUNCTION__, m_i4RequestMagic);
        // Update Flash on/off information to AE/AWB
        IAwbMgr::getInstance().setStrobeMode(m_i4SensorDev, (bFlashOnOff ? AWB_STROBE_MODE_ON : AWB_STROBE_MODE_OFF));
        if(bFlashOnOff)
            IAwbMgr::getInstance().doCapFlashAWBDisplay(m_i4SensorDev);
        if((FlashMgr::getInstance(m_i4SensorDev)->getFlashMode() != LIB3A_FLASH_MODE_FORCE_TORCH)){
            MBOOL bStrobeMode = bFlashOnOff ? MTRUE : MFALSE;
            IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetStrobeMode, bStrobeMode, NULL, NULL, NULL);
            IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_NotifyMainFlashON, bStrobeMode, NULL, NULL, NULL);
            //CAM_LOGD("[%s] Non Torch ae setStrobeMode", __FUNCTION__);
        }
        // AE: update capture parameter
        if(rData.bMainFlashOn){
            CAM_LOGD("[%s] FLASH UPDATED + #(%d)", __FUNCTION__, i4StatisticMagic);
            IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetDoBackAEInfo, NULL, NULL, NULL, NULL);
            FlashMgr::getInstance(m_i4SensorDev)->setCapPara();
            CAM_LOGD("[%s] FLASH UPDATED - #(%d)", __FUNCTION__, i4StatisticMagic);
        }
        IAeMgr::getInstance().doCapAE(m_i4SensorDev);
        m_rTimer.End();

        if(!rData.bIsHqCap)
        {
            //===== done =====
            CAM_LOGD("[%s] Capture End #(%d)", __FUNCTION__, i4StatisticMagic);
            if(pAAOBuffer)
            {
                AE_PERFRAME_INFO_T AEPerframeInfo;
                IAeMgr::getInstance().getAEInfo(m_i4SensorDev, AEPerframeInfo);

                //Because NVRAM index need to update, so call doPvAWB when non HQC
                CAM_LOGD("[%s] doPvAWB to update NVRAM index", __FUNCTION__);
                MINT32 i4AoeCompLv = AEPerframeInfo.rAEUpdateInfo.i4AOECompLVvalue;
                MBOOL bAEStable = AEPerframeInfo.rAEISPInfo.bAEStable;
                IAwbMgr::getInstance().doPvAWB(m_i4SensorDev,
                                               pAAOBuffer->mMagicNumber,
                                               bAEStable,
                                               i4AoeCompLv,
                                               reinterpret_cast<MVOID *>(pAAOBuffer->mVa),
                                               AEPerframeInfo.rAEISPInfo.u8P1Exposuretime_ns/1000);
            }
            else
                CAM_LOGW("[%s] AAO buffer is NULL", __FUNCTION__);
            updateStatus(TASK_STATUS_DONE);
            return TASK_RESULT_OK;
        }
    } else
    {
        //===== Capture End =====
        m_rTimer.start("Capture End",m_i4SensorDev, m_i4LogEn, 1);
        if(m_i4RequestMagic <= i4StatisticMagic)
        {
            CAM_LOGD("[%s] Capture End #(%d)", __FUNCTION__, i4StatisticMagic);
            //===== AE calc =====
            m_rTimer.start("AE calc",m_i4SensorDev, m_i4LogEn, 1);

            //set awb gain to ae_mgr
            CCU_AAO_PROC_INFO_T rProcInfo;
            rProcInfo.awb_gain.i4R = rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R;
            rProcInfo.awb_gain.i4G = rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G;
            rProcInfo.awb_gain.i4B = rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B;
            IAeMgr::getInstance().sendAECtrl(m_i4SensorDev, EAECtrl_SetAAOProcInfo, NULL, reinterpret_cast<MINTPTR>(&(rProcInfo)), NULL, NULL);

            // Normal AE
            if(pAAOBuffer)
                IAeMgr::getInstance().doPostCapAE(m_i4SensorDev, reinterpret_cast<MVOID *>(pAAOBuffer->mVa), FlashMgr::getInstance(m_i4SensorDev)->isFlashOnCapture());
            else
                CAM_LOGW("[%s] AAO buffer is NULL", __FUNCTION__);
            m_rTimer.End();

            //===== AWB calc =====
            m_rTimer.start("AWB calc",m_i4SensorDev, m_i4LogEn, 1);
            AE_PERFRAME_INFO_T AEPerframeInfo;
            IAeMgr::getInstance().getAEInfo(m_i4SensorDev, AEPerframeInfo);
            MINT32 i4AoeCompLv = AEPerframeInfo.rAEUpdateInfo.i4AOECompLVvalue;

            if(pAAOBuffer)
                IAwbMgr::getInstance().doCapAWB(m_i4SensorDev, i4AoeCompLv, reinterpret_cast<MVOID *>(pAAOBuffer));
            else
                CAM_LOGW("[%s] AAO buffer is NULL", __FUNCTION__);
            m_rTimer.End();

            //===== Flash dump AAO =====
            if(pAAOBuffer)
            {
                MBOOL bAaoMainFlashOn = (pAAOBuffer->mFlashState == MTK_FLASH_STATE_FIRED) ? MTRUE : MFALSE;
                if(bAaoMainFlashOn)
                {
                    FlashMgr::getInstance(m_i4SensorDev)->dumpMainFlashAAO(reinterpret_cast<MVOID *>(pAAOBuffer->mVa));
                }
                else
                {
                    CAM_LOGW("[%s] No need dump AAO buffer (%p, %d)", __FUNCTION__, pAAOBuffer, bFlashOnOff);
                }
            }
            else
            {
                CAM_LOGW("[%s] AAO buffer is NULL", __FUNCTION__);
            }

            //===== dump AAO for adb command =====
            if(pAAOBuffer &&m_i4DumpEn)
            {
                char fileName[FILE_NAME_SIZE];
                char folderName[FILE_NAME_SIZE];
                strncpy(folderName, "/sdcard/camera_dump", FILE_NAME_SIZE);
                sprintf(fileName, "/sdcard/camera_dump/captue_end_aao_%d.hw_aao", pAAOBuffer->mMagicNumber);
                if(_dumpHwBuf(fileName, folderName, *pAAOBuffer))
                {
                    CAM_LOGD("%s\n", fileName);
                }
            }

            //===== Shadin calc =====
            m_rTimer.start("Shading calc",m_i4SensorDev, m_i4LogEn, 1);

            // TSF
            ILscMgr::TSF_AWB_INFO rAwbInfo;
            ILscMgr::TSF_INPUT_INFO_T rTsfInfo;
            ILscMgr* pLsc = ILscMgr::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev));
            rAwbInfo.m_i4LV     = AEPerframeInfo.rAEISPInfo.i4LightValue_x10;
            rAwbInfo.m_u4CCT    = IAwbMgr::getInstance().getAWBCCT(m_i4SensorDev);
            rAwbInfo.m_RGAIN    = rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R;
            rAwbInfo.m_GGAIN    = rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G;
            rAwbInfo.m_BGAIN    = rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B;
            rAwbInfo.m_FLUO_IDX = rAWBOutput.rAWBInfo.i4FluorescentIndex;
            rAwbInfo.m_DAY_FLUO_IDX = rAWBOutput.rAWBInfo.i4DaylightFluorescentIndex;
            rAwbInfo.m_FLASH_ON = 0;        // Nelson Coverity
            rTsfInfo.eCmd = ILscMgr::E_TSF_CMD_BATCH_CAP;
            rTsfInfo.u4FrmId = pAAOBuffer->mMagicNumber;

            MINT32 i4IsLockRto = MFALSE;
#if CAM3_FLASH_ON_SHADING_CT_3_TABLE
            if(FlashMgr::getInstance().isFlashOnCapture(m_i4SensorDev))
            {
                CAM_LOGD("[%s] (CT3) Inform ILscMgr that flash is ON.",__FUNCTION__);
                rAwbInfo.m_FLASH_ON = MTRUE;
            }
            else
            {
                rAwbInfo.m_FLASH_ON = MFALSE;
            }
#else

            MBOOL lsciIsFlashOn = ((pAAOBuffer->mFlashState == MTK_FLASH_STATE_FIRED) || (pAAOBuffer->mFlashState == MTK_FLASH_STATE_PARTIAL)) ? MTRUE : MFALSE;

            if((rData.bFlashOnOff || rData.bFlashOpenByTask || lsciIsFlashOn )) //is out open flash, Ex: [out]torch or main flash, [3A HAL(Task)] pre-flash or touch
            {
                CAM_LOGD("[%s] MFNR + flash (%d, %d, %d)",__FUNCTION__, rData.bFlashOnOff, rData.bFlashOpenByTask, lsciIsFlashOn);

                if(m_i4PreCapFrmId!=0 && abs(int(m_i4PreCapFrmId-pAAOBuffer->mMagicNumber))<=10)
                {
                    CAM_LOGD("[%s] keep shading table and lock ratio (pre, cur)=(%d, %d)",__FUNCTION__, m_i4PreCapFrmId, pAAOBuffer->mMagicNumber);
                    rTsfInfo.eCmd = ILscMgr::E_TSF_CMD_KEEP;
                    i4IsLockRto = MTRUE;
                } else
                    CAM_LOGD("[%s] Batch CAP + unlock ratio (pre, cur)=(%d, %d)",__FUNCTION__, m_i4PreCapFrmId, pAAOBuffer->mMagicNumber);
            }
#endif
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
            pLsc->evaluate_cus_Ratio(rLscInputInfo, i4IsLockRto);
            pLsc->updateTsf(rTsfInfo);
            m_rTimer.End();

            m_i4PreCapFrmId = pAAOBuffer->mMagicNumber;

            //===== done =====
            updateStatus(TASK_STATUS_DONE);
            if(lsciIsFlashOn)
                return TASK_RESULT_MAINFLASH_CAPTURE_END;
            else
                return TASK_RESULT_OK;
        } else
        {
            CAM_LOGD("request,statistic = (%d, %d), outControlFlashOnOff(%d), m_i4FrameCount(%d)", m_i4RequestMagic, i4StatisticMagic, bFlashOnOff, m_i4FrameCount);
            if(bFlashOnOff && m_i4FrameCount == 2)
                return TASK_RESULT_MAINFLASH_RESTORE_REQUIRE;
        }
        m_rTimer.End();
    }

    if(getStatus() >= TASK_STATUS_CANCELLED)
        return TASK_RESULT_OK;

    updateStatus(TASK_STATUS_RUNNING);
    m_i4FrameCount++;
    return TASK_RESULT_OK;
}

MVOID
Task3ACaptureImp::cancel()
{
    updateStatus(TASK_STATUS_CANCELLED);
}

MVOID
Task3ACaptureImp::waitFinish()
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
Task3ACaptureImp::getStatus()
{
    std::lock_guard<std::mutex> lock(m_StatusLock);
    return m_i4Status;
}

MVOID
Task3ACaptureImp::updateStatus(MINT32 i4Status)
{
    std::lock_guard<std::mutex> lock(m_StatusLock);
    if(m_i4Status == TASK_STATUS_INIT && i4Status == TASK_STATUS_RUNNING)
    {
        AAA_TRACE_D("Task3ACapture_start");
        AAA_TRACE_END_D;
    }
    if(m_i4Status == TASK_STATUS_RUNNING && i4Status == TASK_STATUS_DONE)
    {
        AAA_TRACE_D("Task3ACapture_end");
        AAA_TRACE_END_D;
    }
    if(m_i4Status != i4Status)
    {
        CAM_LOGD("[%s] status(%d->%d)", __FUNCTION__, m_i4Status, i4Status);
        m_i4Status = i4Status;
    }
}
