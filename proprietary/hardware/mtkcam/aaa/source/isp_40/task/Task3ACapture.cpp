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
#include <cutils/properties.h>
#include <mtkcam/utils/std/Trace.h>
#include <sys/stat.h>
// 3A framework
#include <aaa_trace.h>
#include <aaa_types.h>
#include <aaa_timer.h>
#include <aaa_common_custom.h>
#include <aaa_hal_sttCtrl.h>
#include <IResultPool.h>
#include <IThreadRaw.h>
// 3A module
#include <af_feature.h>
#include <af_algo_if.h>
#include <ae_mgr_if.h>
#include <af_mgr_if.h>
#include <awb_mgr_if.h>
#include <lsc_mgr/ILscTsf.h>
#include <flash_mgr.h>
#include <flash_feature.h>

// 3A Custom
#include <awb_tuning_custom.h>
#include <flash_awb_tuning_custom.h>
// ISP
#include <isp_tuning_mgr.h>

#define FILE_NAME_SIZE 64

using namespace NS3Av3;

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
    mutable Mutex m_StatusLock;
    mutable Mutex m_RunningLock;
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
    m_i4LogEn = property_get_int32("vendor.debug.hal3a.task",0);
    m_i4DumpEn = property_get_int32("vendor.debug.camera.AAO.dump", 0);
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
    Mutex::Autolock lock(m_RunningLock);

    if(getStatus() >= TASK_STATUS_CANCELLED)
        return TASK_RESULT_OK;

    //===== prepare statistic data =====
    StatisticBufInfo* pAAOBuffer = NULL;
    StatisticBufInfo* pPSOBuffer = NULL;
    if(rData.pAAOBuffer)
        pAAOBuffer = reinterpret_cast<StatisticBufInfo*>(rData.pAAOBuffer);
    if(rData.pPSOBuffer)
        pPSOBuffer = reinterpret_cast<StatisticBufInfo*>(rData.pPSOBuffer);

    if(pAAOBuffer == NULL && getStatus() != TASK_STATUS_INIT)
    {
        CAM_LOGW("[%s] AAO buffer is NULL", __FUNCTION__);
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
            IAeMgr::getInstance().setStrobeMode(m_i4SensorDev, (bFlashOnOff ? MTRUE : MFALSE));
            //CAM_LOGD("[%s] Non Torch ae setStrobeMode", __FUNCTION__);
        }
        //add 4-cell condition here
        if(!rData.bIsHqCap)
        {
            if(bFlashOnOff){
                AE_MODE_CFG_T previewInfo;
                if(FlashMgr::getInstance(m_i4SensorDev)->getFlashMode() != LIB3A_FLASH_MODE_FORCE_TORCH) // Torch capture no need to do restore
                    IAeMgr::getInstance().doRestoreAEInfo(m_i4SensorDev, MFALSE, MTRUE);
                IAeMgr::getInstance().getPreviewParams(m_i4SensorDev, previewInfo);
                IAeMgr::getInstance().updateCaptureParams(m_i4SensorDev, previewInfo);
            }
            if(rData.bRemosaicEn)
                IAeMgr::getInstance().prepareCapParams(m_i4SensorDev,rData.bRemosaicEn,MFALSE);
        }
        // AE: update capture parameter
        if(rData.bMainFlashOn){
            CAM_LOGD("[%s] FLASH UPDATED + #(%d)", __FUNCTION__, i4StatisticMagic);
            IAeMgr::getInstance().doBackAEInfo(m_i4SensorDev);
            FlashMgr::getInstance(m_i4SensorDev)->setCapPara();
            CAM_LOGD("[%s] FLASH UPDATED - #(%d)", __FUNCTION__, i4StatisticMagic);
        }
        IAeMgr::getInstance().doCapAE(m_i4SensorDev);

        //force unlock shading ratio
        NSIspTuningv3::IspTuningMgr::getInstance().sendIspTuningIOCtrl(
            m_i4SensorDev, NSIspTuningv3::IspTuningMgr::E_ISPTUNING_UPDATE_LSC_INFO, NSIspTuningv3::IspTuningMgr::E_SHADING_RATIO_UNLOCK, 0);

        m_rTimer.End();

        if(!rData.bIsHqCap)
        {
            //===== done =====
            CAM_LOGD("[%s] Capture End #(%d)", __FUNCTION__, i4StatisticMagic);
            if(pAAOBuffer)
            {
                //Because NVRAM index need to update, so call doPvAWB when non HQC
                CAM_LOGD("[%s] doPvAWB to update NVRAM index", __FUNCTION__);
                FrameOutputParam_T rRTParams;
                IAeMgr::getInstance().getRTParams(m_i4SensorDev,rRTParams);
                MINT32 i4AoeCompLv = IAeMgr::getInstance().getAOECompLVvalue(m_i4SensorDev, MFALSE);
                MBOOL bAEStable = IAeMgr::getInstance().IsAEStable(m_i4SensorDev);
                IAwbMgr::getInstance().doPvAWB(m_i4SensorDev,
                                               pAAOBuffer->mMagicNumber,
                                               bAEStable,
                                               i4AoeCompLv,
#ifndef MT6757P_API
                                               reinterpret_cast<MVOID *>(pAAOBuffer->getPart(AAOSepLSCBuf)),
                                               rRTParams.u4PreviewShutterSpeed_us);
#else
                                               reinterpret_cast<MVOID *>(pAAOBuffer->getPart(AAOSepAWBBuf)));
#endif
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
            // PSO feature
            if(pPSOBuffer)
            {
                AAO_PROC_INFO_T rPSOInfo;
                // Get the previous setting of PSO, for example : OB, AWB, CCM, LSC info.
                IResultPool::getInstance(m_i4SensorDev)->getAAOInfo(AE_V4P0_BLOCK_NO, pPSOBuffer->mMagicNumber, rPSOInfo);
                // Get PSO buf info
                rPSOInfo.Pso_Output_Path = (PSO_PATH_SEL_ENUM)pPSOBuffer->mRawType; // PSO before OB or after OB
                // AE
                IAeMgr::getInstance().setAAOProcInfo(m_i4SensorDev, reinterpret_cast<MVOID *>(pPSOBuffer->getPart(PSOSepBuf)), &rPSOInfo);
            } else
                CAM_LOGW("[%s] PSO buffer is NULL", __FUNCTION__);

            // Normal AE
            if(pAAOBuffer)
                IAeMgr::getInstance().doCapFlare(m_i4SensorDev, reinterpret_cast<MVOID *>(pAAOBuffer->getPart(AAOSepBuf)), FlashMgr::getInstance(m_i4SensorDev)->isFlashOnCapture());
            else
                CAM_LOGW("[%s] AAO or PSO buffer is NULL(%p, %p)", __FUNCTION__, pAAOBuffer, pPSOBuffer);
            m_rTimer.End();

            //===== AWB calc =====
            m_rTimer.start("AWB calc",m_i4SensorDev, m_i4LogEn, 1);
            MINT32 i4AoeCompLv = IAeMgr::getInstance().getAOECompLVvalue(m_i4SensorDev, MFALSE);

            if(pAAOBuffer)
#ifndef MT6757P_API
                IAwbMgr::getInstance().doCapAWB(m_i4SensorDev, i4AoeCompLv, reinterpret_cast<MVOID *>(pAAOBuffer->getPart(AAOSepLSCBuf)));
#else
                IAwbMgr::getInstance().doCapAWB(m_i4SensorDev, i4AoeCompLv, reinterpret_cast<MVOID *>(pAAOBuffer->getPart(AAOSepAWBBuf)));
#endif
            else
                CAM_LOGW("[%s] AAO buffer is NULL", __FUNCTION__);
            m_rTimer.End();

            //===== Flash dump AAO =====
            if(pAAOBuffer)
            {
                MBOOL bAaoMainFlashOn = (pAAOBuffer->mFlashState == MTK_FLASH_STATE_FIRED) ? MTRUE : MFALSE;
                if(bAaoMainFlashOn)
                {
                    FlashMgr::getInstance(m_i4SensorDev)->dumpMainFlashAAO(reinterpret_cast<MVOID *>(pAAOBuffer->getPart(AAOSepBuf)));
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
            AWB_OUTPUT_T rAWBOutput;
            IAwbMgr::getInstance().getAWBOutput(m_i4SensorDev, rAWBOutput);
            ILscMgr::TSF_AWB_INFO rAwbInfo;
            ILscMgr::TSF_INPUT_INFO_T rTsfInfo;
            ILscMgr* pLsc = ILscMgr::getInstance(static_cast<MUINT32>(m_i4SensorDev));
            rAwbInfo.m_i4LV        = IAeMgr::getInstance().getLVvalue(m_i4SensorDev, MTRUE);
            rAwbInfo.m_u4CCT    = IAwbMgr::getInstance().getAWBCCT(m_i4SensorDev);
            rAwbInfo.m_RGAIN    = rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R;
            rAwbInfo.m_GGAIN    = rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G;
            rAwbInfo.m_BGAIN    = rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B;
            rAwbInfo.m_FLUO_IDX = rAWBOutput.rAWBInfo.i4FluorescentIndex;
            rAwbInfo.m_DAY_FLUO_IDX = rAWBOutput.rAWBInfo.i4DaylightFluorescentIndex;
            rAwbInfo.m_FLASH_ON = 0;        // Nelson Coverity
            rTsfInfo.eCmd = ILscMgr::E_TSF_CMD_BATCH_CAP;
            rTsfInfo.u4FrmId = pAAOBuffer->mMagicNumber;

            MINT32 i4IsLockRto = NSIspTuningv3::IspTuningMgr::E_SHADING_RATIO_FREE_RUN;
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
            MINT32 i4PreCapFrmId = pLsc->getCapAAOMagicNum();

            if((rData.bFlashOnOff || rData.bFlashOpenByTask || lsciIsFlashOn )) //is out open flash, Ex: [out]torch or main flash, [3A HAL(Task)] pre-flash or touch
            {
                CAM_LOGD("[%s] MFNR + flash (%d, %d, %d)",__FUNCTION__, rData.bFlashOnOff, rData.bFlashOpenByTask, lsciIsFlashOn);

                if((i4PreCapFrmId>=0) && (abs(int(i4PreCapFrmId-pAAOBuffer->mMagicNumber))<=2) && (i4PreCapFrmId!=pAAOBuffer->mMagicNumber))
                {
                    CAM_LOGD("[%s] keep shading table and lock ratio (pre, cur)=(%d, %d)",__FUNCTION__, i4PreCapFrmId, pAAOBuffer->mMagicNumber);
                    rTsfInfo.eCmd = ILscMgr::E_TSF_CMD_KEEP;
                    i4IsLockRto = NSIspTuningv3::IspTuningMgr::E_SHADING_RATIO_LOCK;
                } else
                    CAM_LOGD("[%s] Batch CAP + unlock ratio (pre, cur)=(%d, %d)",__FUNCTION__, i4PreCapFrmId, pAAOBuffer->mMagicNumber);
            }
#endif
            rTsfInfo.rAwbInfo = rAwbInfo;
            rTsfInfo.prAwbStat = reinterpret_cast<MUINT8*>(pAAOBuffer->getPart(AAOSepLSCBuf));
            rTsfInfo.u4SizeAwbStat = pAAOBuffer->getPartSize(AAOSepLSCBuf);

            NSIspTuningv3::IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_i4SensorDev, NSIspTuningv3::IspTuningMgr::E_ISPTUNING_UPDATE_LSC_INFO, i4IsLockRto, 0);
            pLsc->updateTsf(rTsfInfo);
            m_rTimer.End();

            pLsc->setCapAAOMagicNum(pAAOBuffer->mMagicNumber);

            //===== done =====
            updateStatus(TASK_STATUS_DONE);
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
    Mutex::Autolock lock(m_RunningLock);

    MINT32 i4Status = getStatus();
    if(i4Status < TASK_STATUS_CANCELLED)
        cancel();

    updateStatus(TASK_STATUS_DONE);
    CAM_LOGD("[%s] wait for Task done -", __FUNCTION__);
}

MINT32
Task3ACaptureImp::getStatus()
{
    Mutex::Autolock lock(m_StatusLock);
    return m_i4Status;
}

MVOID
Task3ACaptureImp::updateStatus(MINT32 i4Status)
{
    Mutex::Autolock lock(m_StatusLock);
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
