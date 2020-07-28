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
#define LOG_TAG "aaa_state_pv_n3d"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <aaa_types.h>
#include <aaa_state_n3d.h>
#include <aaa_state_mgr.h>
#include <aaa_scheduling_custom.h>
#include <aaa_common_custom.h>
#include <aaa_hal_if.h>
#include <aaa_hal_sttCtrl.h>

#include <dbg_aaa_param.h>
#include <dbg_af_param.h>

#include <af_param.h>
#include <awb_param.h>
#include <flash_param.h>

#include <af_feature.h>
#include <af_algo_if.h>

#include <ae_mgr/ae_mgr_if.h>
#include <af_mgr/af_mgr_if.h>
#include <awb_mgr/awb_mgr_if.h>
#include <lsc_mgr/ILscTsf.h>
#include <flash_mgr/flash_mgr.h>
#include <flash_feature.h>
#include <flicker/flicker_hal_base.h>
#include <sensor_mgr/aaa_sensor_mgr.h>


#include <awb_tuning_custom.h>
#include <flash_awb_param.h>
#include <flash_awb_tuning_custom.h>
#include <flash_tuning_custom.h>
#include <isp_tuning_mgr.h>
#include <IHal3AResultBufInfo.h>
#include <debug/DebugUtil.h>
#include <cutils/properties.h>

#include <ISync3A.h>
#include <aaa_trace.h>
#include <mtkcam/utils/std/Trace.h>

using namespace NS3Av3;
using namespace NSIspTuning;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateCameraPreview
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
StateCameraPreviewN3d::
StateCameraPreviewN3d(MINT32 sensorDevId, StateMgr* pStateMgr)
    : StateCameraPreview(sensorDevId, pStateMgr)
{
    CAM_LOGD("[%s] sensorDevId(%d)", __FUNCTION__, sensorDevId);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_CameraPreviewEnd
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCameraPreviewN3d::
sendIntent(intent2type<eIntent_CameraPreviewEnd>)
{
    return StateCameraPreview::sendIntent(intent2type<eIntent_CameraPreviewEnd>());
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_VsyncUpdate
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCameraPreviewN3d::
sendIntent(intent2type<eIntent_VsyncUpdate>)
{
    CAM_LOGD("CapQueueEmpty(%d)",m_pStateMgr->isCapQueueEmpty());
    if (!m_pStateMgr->isCapQueueEmpty())
    {
        doCaptureEnd();
    }
    else
        doUpdate();
    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  doCaptureEnd
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCameraPreviewN3d::
doCaptureEnd()
{
    MINT32 i4SceneLv = 80;
    MINT32 i4AoeCompLv = 80;
    MBOOL bAEStable = MTRUE;

    IBufMgr* pAAOBufMgr = Hal3ASttCtrl::getInstance(m_SensorDevId)->getBufMgr(BUF_AAO);
    if(pAAOBufMgr == NULL) return S_3A_OK;

    StatisticBufInfo* rBufInfo;
    rBufInfo = pAAOBufMgr->dequeueSwBuf();
    if(rBufInfo == NULL) return S_3A_OK;

    IBufMgr* pPSOBufMgr = Hal3ASttCtrl::getInstance(m_SensorDevId)->getBufMgr(BUF_PSO);
    if(pPSOBufMgr == NULL) return S_3A_OK;

    StatisticBufInfo* rPSOBufInfo;
    rPSOBufInfo = pPSOBufMgr->dequeueSwBuf();
    if(rBufInfo == NULL || rPSOBufInfo == NULL) return S_3A_OK;

    CAM_LOGD_IF(m_i4EnableLog, "[StateCameraPreview::doCaptureEnd()] dequeueSwBuf AAO done\n");
    StateCameraPreview::m_pStateMgr->updateMagicNumber(rBufInfo->mMagicNumber);
    CAM_LOGD_IF(m_i4EnableLog, "[%s] magic number = %d",__FUNCTION__, m_pStateMgr->queryMagicNumber());

    MINT32 i4Ret = 0;
    i4Ret = m_pStateMgr->queryCapQueue(rBufInfo->mMagicNumber);//if aao magic# is in queryCapQueue, do capEnd
    if (i4Ret)
    {
        CAM_LOGD("rBufInfo->mMagicNumber found in CapQueue, do captureEnd");
        i4SceneLv = IAeMgr::getInstance().getLVvalue(m_SensorDevId, MFALSE);
        i4AoeCompLv = IAeMgr::getInstance().getAOECompLVvalue(m_SensorDevId, MFALSE);

        if(FlashMgr::getFlashSpMode()!=e_SpModeCalibration
           && FlashMgr::getFlashSpMode()!=e_SpModeQuickCalibration
           && FlashMgr::getFlashSpMode()!=e_SpModeQuickCalibration2 )
        {// One-shot AWB
            if (FlashMgr::getInstance().getFlashMode(m_SensorDevId) == LIB3A_FLASH_MODE_FORCE_TORCH)
                IAwbMgr::getInstance().SetTorchMode(m_SensorDevId , MTRUE);
            else
                IAwbMgr::getInstance().SetTorchMode(m_SensorDevId , MFALSE);

               IAwbMgr::getInstance().doCapAWB(m_SensorDevId, i4AoeCompLv, reinterpret_cast<MVOID *>(rBufInfo->getPart(AAOSepLSCBuf)));
        }
        CAM_LOGD("IAwbMgr::getInstance().doCapAWB() END");


        // Get PSO info
        IHal3AResultBufInfo::getInstance(m_SensorDevId)->getAAOInfo(AE_V4P0_BLOCK_NO, rBufInfo->mMagicNumber, m_rAAOInfo);
        // Get PSO buf info
        m_rAAOInfo.Pso_Output_Path = (PSO_PATH_SEL_ENUM)rPSOBufInfo->mRawType; // PSO before OB or after OB
        // AE
        IAeMgr::getInstance().setAAOProcInfo(m_SensorDevId, reinterpret_cast<MVOID *>(rPSOBufInfo->getPart(PSOSepBuf)), &m_rAAOInfo);
        IAeMgr::getInstance().doCapFlare(m_SensorDevId, reinterpret_cast<MVOID *>(rBufInfo->getPart(AAOSepBuf)),
        FlashMgr::getInstance().isFlashOnCapture(m_SensorDevId) );
        CAM_LOGD("IAeMgr::getInstance().doCapFlare() END");

#if CAM3_LSC_FEATURE_EN
    // TSF
        AWB_OUTPUT_T rAWBOutput;
        IAwbMgr::getInstance().getAWBOutput(m_SensorDevId, rAWBOutput);
        ILscMgr::TSF_AWB_INFO rAwbInfo;
        ILscMgr::TSF_INPUT_INFO_T rTsfInfo;
        ILscMgr* pLsc = ILscMgr::getInstance(static_cast<ESensorDev_T>(m_SensorDevId));
        rAwbInfo.m_i4LV        = IAeMgr::getInstance().getLVvalue(m_SensorDevId, MTRUE);
        rAwbInfo.m_u4CCT    = IAwbMgr::getInstance().getAWBCCT(m_SensorDevId);
        rAwbInfo.m_RGAIN    = rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R;
        rAwbInfo.m_GGAIN    = rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G;
        rAwbInfo.m_BGAIN    = rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B;
        rAwbInfo.m_FLUO_IDX = rAWBOutput.rAWBInfo.i4FluorescentIndex;
        rAwbInfo.m_DAY_FLUO_IDX = rAWBOutput.rAWBInfo.i4DaylightFluorescentIndex;
        rTsfInfo.eCmd = ILscMgr::E_TSF_CMD_BATCH_CAP;
        rTsfInfo.u4FrmId = rBufInfo->mMagicNumber;
#if CAM3_FLASH_ON_SHADING_CT_3_TABLE
                if(FlashMgr::getInstance().isFlashOnCapture(m_SensorDevId))
                {
                    CAM_LOGD("[%s] Inform ILscMgr that flash is ON.",__FUNCTION__);
                    rAwbInfo.m_FLASH_ON = MTRUE;
                }
                else
                {
                    rAwbInfo.m_FLASH_ON = MFALSE;
                }
#endif
        rTsfInfo.rAwbInfo = rAwbInfo;
        rTsfInfo.prAwbStat = reinterpret_cast<MUINT8*>(rBufInfo->getPart(AAOSepLSCBuf));
        rTsfInfo.u4SizeAwbStat = rBufInfo->getPartSize(AAOSepLSCBuf);

        NSIspTuningv3::IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_SensorDevId, NSIspTuningv3::IspTuningMgr::E_ISPTUNING_UPDATE_LSC_INFO, NSIspTuningv3::IspTuningMgr::E_SHADING_RATIO_FREE_RUN, 0);
        pLsc->updateTsf(rTsfInfo);

        CAM_LOGD("lv(%d),cct(%d),rgain(%d),bgain(%d),ggain(%d),fluoidx(%d), dayflouidx(%d)",
                rAwbInfo.m_i4LV,
                rAwbInfo.m_u4CCT,
                rAwbInfo.m_RGAIN,
                rAwbInfo.m_GGAIN,
                rAwbInfo.m_BGAIN,
                rAwbInfo.m_FLUO_IDX,
                rAwbInfo.m_DAY_FLUO_IDX
                );
#endif

        CAM_LOGD_IF(m_i4EnableLog, "doCaptureEnd Finish");
        }else {
        CAM_LOGD_IF(m_i4EnableLog, "skip update only deque, waiting CaptureEnd");
    }
    return  S_3A_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  doCaptureEnd
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCameraPreviewN3d::
doUpdate()
{
    MINT32 i4SceneLv = 80;
    MINT32 i4AoeCompLv = 80;
    MINT32 i4FrmCount;
    MBOOL bAEStable = MTRUE;

    // N3D member
    MINT32 i4ActiveAeItem = 0;
    MINT32 i4SyncFrmCount = 0;
    MINT32 i4AfState = 0;
    MINT32 i4SyncOpt = 0;
    MBOOL bSyncEnable = MFALSE;

    ISync3A* pSync3A = ISync3AMgr::getInstance()->getSync3A();
    bSyncEnable = pSync3A->isSyncEnable();
    i4SyncFrmCount = pSync3A->getFrameCount();
    i4AfState = ISync3AMgr::getInstance()->getAFState();

    i4ActiveAeItem = pSync3A->getAeSchedule();
    i4SyncOpt |= ((i4ActiveAeItem & E_AE_AE_CALC) ? ISync3A::E_SYNC3A_DO_AE : 0);
    i4SyncOpt |= ISync3A::E_SYNC3A_DO_AWB;
    i4SyncOpt |= ISync3AMgr::getInstance()->getFrmSyncOpt();

    // Update frame count
    m_pStateMgr->updateFrameCount();
    i4FrmCount = m_pStateMgr->getFrameCount();
    CAM_LOGD_IF(m_i4EnableLog
            , "[StateCameraPreview::doUpdate()] Sensor(%d), bSyncEnable(%d), i4AfState(%d), frameCnt(%d)"
            , m_SensorDevId, bSyncEnable, i4AfState, i4FrmCount);

    if (i4FrmCount < 0)
    {   // AAO statistics is not ready
        return S_3A_OK;
    }

    // Dequeue AAO buffer from SW buffer
    IBufMgr* pAAOBufMgr = Hal3ASttCtrl::getInstance(m_SensorDevId)->getBufMgr(BUF_AAO);
    if(pAAOBufMgr == NULL) return S_3A_OK;

    // Dequeue PSO buffer from SW buffer
    IBufMgr* pPSOBufMgr = Hal3ASttCtrl::getInstance(m_SensorDevId)->getBufMgr(BUF_PSO);
    if(pPSOBufMgr == NULL) return S_3A_OK;

    StatisticBufInfo*  rBufInfo = pAAOBufMgr->dequeueSwBuf();
    StatisticBufInfo* rPSOBufInfo = pPSOBufMgr->dequeueSwBuf();
    if(rBufInfo == NULL || rPSOBufInfo == NULL) return S_3A_OK;
    if(ISync3AMgr::getInstance()->getMasterDev() == m_SensorDevId)
        DebugUtil::getInstance(m_SensorDevId)->detectPreframe(DBG_AE, rBufInfo->mFrameCount);
    CAM_LOGD_IF(m_i4EnableLog, "[StateCameraPreview::sendIntent<eIntent_VsyncUpdate>] dequeueSwBuf AAO done\n");
    StateCameraPreview::m_pStateMgr->updateMagicNumber(rBufInfo->mMagicNumber);
    CAM_LOGD_IF(m_i4EnableLog, "[%s] magic number = %d",__FUNCTION__, m_pStateMgr->queryMagicNumber());

#if CAM3_AF_FEATURE_EN
    if(CUST_LOCK_AE_DURING_CAF())
    {
        if ((IAfMgr::getInstance().isLockAE(m_SensorDevId)   == MFALSE) || //if =1, lens are fixed, do AE as usual; if =0, lens are moving, don't do AE
            (IAeMgr::getInstance().IsAEStable(m_SensorDevId) == MFALSE)) //guarantee AE can doPvAE at beginning, until IsAEStable()=1
        {
             IAeMgr::getInstance().setAFAELock(m_SensorDevId, MFALSE);
        }
        else
        {
             IAeMgr::getInstance().setAFAELock(m_SensorDevId, MTRUE);
        }
    }
    else //always do AE, no matter whether lens are moving or not
    {
        IAeMgr::getInstance().setAFAELock(m_SensorDevId, MFALSE);
    }
#endif

    // notify AE RT params.
    FrameOutputParam_T rRTParams;
    IAeMgr::getInstance().getRTParams(m_SensorDevId,rRTParams);

    // only Master do AE
    if(ISync3AMgr::getInstance()->getMasterDev() == m_SensorDevId)
    {
        if (i4AfState != ISync3AMgr::E_SYNC3AMGR_AF_STATE_SCANNING)
        {
            // Get PSO info
            IHal3AResultBufInfo::getInstance(m_SensorDevId)->getAAOInfo(AE_V4P0_BLOCK_NO, rBufInfo->mMagicNumber, m_rAAOInfo);
            // Get PSO buf info
            m_rAAOInfo.Pso_Output_Path = (PSO_PATH_SEL_ENUM)rPSOBufInfo->mRawType; // PSO before OB or after OB
            // AE
            IAeMgr::getInstance().setAAOProcInfo(m_SensorDevId, reinterpret_cast<MVOID *>(rPSOBufInfo->getPart(PSOSepBuf)), &m_rAAOInfo);
            IAeMgr::getInstance().doPvAE(m_SensorDevId, rBufInfo->mTimeStamp, reinterpret_cast<MVOID *>(rBufInfo->getPart(AAOSepBuf)),
            (i4ActiveAeItem & E_AE_AE_CALC), rBufInfo->mMagicNumber, 1);

            m_pStateMgr->doNotifyCb(I3ACallBack::eID_NOTIFY_AE_RT_PARAMS, rBufInfo->mMagicNumber, (MINTPTR)&rRTParams, 0);
        } else
        {
            i4SyncOpt |= ISync3A::E_SYNC3A_BYP_AE;
        }

        i4SceneLv = IAeMgr::getInstance().getLVvalue(m_SensorDevId, MFALSE);
        //i4AoeCompLv = IAeMgr::getInstance().getAOECompLVvalue(m_SensorDevId, MFALSE);
        bAEStable = IAeMgr::getInstance().IsAEStable(m_SensorDevId);
    }
    //For update both master & slave cams' LV
    i4AoeCompLv = IAeMgr::getInstance().getAOECompLVvalue(m_SensorDevId, MFALSE);
    //MY_LOG("Preview n3d i4AoeCompLv = %d\n", i4AoeCompLv);

    CAM_LOGD_IF(m_i4EnableLog, "[StateCameraPreview::sendIntent<eIntent_VsyncUpdate>] doPvAE done\n");

    // workaround for iVHDR
    MUINT32 u4AFSGG1Gain;
    IAeMgr::getInstance().getAESGG1Gain(m_SensorDevId, &u4AFSGG1Gain);
    IAfMgr::getInstance().setSGGPGN(m_SensorDevId, (MINT32) u4AFSGG1Gain);

    // AWB
    IAwbMgr::getInstance().doPvAWB(m_SensorDevId, i4SyncFrmCount, bAEStable, i4AoeCompLv, reinterpret_cast<MVOID *>(rBufInfo->getPart(AAOSepLSCBuf)), rRTParams.u4PreviewShutterSpeed_us);
    CAM_LOGD_IF(m_i4EnableLog, "[StateCameraPreview::sendIntent<eIntent_VsyncUpdate>] doPvAWB done\n");

#if CAM3_LSC_FEATURE_EN
    // TSF
    AWB_OUTPUT_T rAWBOutput;
    IAwbMgr::getInstance().getAWBOutput(m_SensorDevId, rAWBOutput);
    ILscMgr::TSF_AWB_INFO rAwbInfo;
    ILscMgr::TSF_INPUT_INFO_T rTsfInfo;
    ILscMgr* pLsc = ILscMgr::getInstance(static_cast<ESensorDev_T>(m_SensorDevId));
    rAwbInfo.m_i4LV        = IAeMgr::getInstance().getLVvalue(m_SensorDevId, MTRUE);
    rAwbInfo.m_u4CCT    = IAwbMgr::getInstance().getAWBCCT(m_SensorDevId);
    rAwbInfo.m_RGAIN    = rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R;
    rAwbInfo.m_GGAIN    = rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G;
    rAwbInfo.m_BGAIN    = rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B;
    rAwbInfo.m_FLUO_IDX = rAWBOutput.rAWBInfo.i4FluorescentIndex;
    rAwbInfo.m_DAY_FLUO_IDX = rAWBOutput.rAWBInfo.i4DaylightFluorescentIndex;
    //rTsfInfo.eCmd = (0 == i4FrmCount) ? ILscMgr::E_TSF_CMD_BATCH : ILscMgr::E_TSF_CMD_RUN;

    // updateTsf with batchCap first then 2~3 keep table when MFNR cap(cap 4 time)
    MINT32 i4AeMode = IAeMgr::getInstance().getAEMode(m_SensorDevId);
    AE_MODE_CFG_T rAeInfo;
    IAeMgr::getInstance().getPreviewParams(m_SensorDevId, rAeInfo);
    MUINT32 u4RealISO = rAeInfo.u4RealISO;

    if(0 == m_pStateMgr->getFrameCount())
        rTsfInfo.eCmd = ILscMgr::E_TSF_CMD_BATCH;
    else if(i4AeMode == MTK_CONTROL_AE_MODE_OFF)
    {
        if(m_i4PreAeMode != MTK_CONTROL_AE_MODE_OFF || m_u4PreRealISO != u4RealISO)
        {
            rTsfInfo.eCmd = ILscMgr::E_TSF_CMD_BATCH_CAP;
#if CAM3_FLASH_ON_SHADING_CT_3_TABLE
            if(FlashMgr::getInstance().isFlashOnCapture(m_SensorDevId))
            {
                CAM_LOGD("[%s] Inform ILscMgr that flash is ON.",__FUNCTION__);
                rAwbInfo.m_FLASH_ON = MTRUE;
            }
            else
            {
                rAwbInfo.m_FLASH_ON = MFALSE;
            }
#endif
        }
        else
            rTsfInfo.eCmd = ILscMgr::E_TSF_CMD_KEEP;
    }
    else
        rTsfInfo.eCmd = ILscMgr::E_TSF_CMD_RUN;

    MY_LOG_IF(m_i4EnableLog, "AeMode(%d), RealISO(%d) TsfCmd(%d)",i4AeMode, u4RealISO, rTsfInfo.eCmd);
    m_i4PreAeMode = i4AeMode;
    m_u4PreRealISO = u4RealISO;
    MY_LOG_IF(m_i4EnableLog, "PreAeMode(%d), PreRealISO(%d)",m_i4PreAeMode, m_u4PreRealISO);

    rTsfInfo.u4FrmId = rBufInfo->mMagicNumber;
    rTsfInfo.rAwbInfo = rAwbInfo;
    rTsfInfo.prAwbStat = reinterpret_cast<MUINT8*>(rBufInfo->getPart(AAOSepLSCBuf));
    rTsfInfo.u4SizeAwbStat = rBufInfo->getPartSize(AAOSepLSCBuf);

    NSIspTuningv3::IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_SensorDevId, NSIspTuningv3::IspTuningMgr::E_ISPTUNING_UPDATE_LSC_INFO, NSIspTuningv3::IspTuningMgr::E_SHADING_RATIO_FREE_RUN, 0);
    pLsc->updateTsf(rTsfInfo);

    CAM_LOGD("lv(%d),cct(%d),rgain(%d),bgain(%d),ggain(%d),fluoidx(%d), dayflouidx(%d)",
            rAwbInfo.m_i4LV,
            rAwbInfo.m_u4CCT,
            rAwbInfo.m_RGAIN,
            rAwbInfo.m_GGAIN,
            rAwbInfo.m_BGAIN,
            rAwbInfo.m_FLUO_IDX,
            rAwbInfo.m_DAY_FLUO_IDX
            );
#endif

    if (bSyncEnable)
    {
        // 2A sync: independent AE/AWB
        CAM_LOGD_IF(m_i4EnableLog, "[%s] Sync 2A: Sensor(%d) =============", __FUNCTION__, m_SensorDevId);
        MINT32 i4Sync = pSync3A->sync(m_SensorDevId, i4SyncOpt, rBufInfo);
        // apply
        CAM_LOGD_IF(m_i4EnableLog, "[%s] Sync 2A: Sensor(%d) Ready to validate (%d)", __FUNCTION__, m_SensorDevId, i4Sync);
    }

    AAA_TRACE_D("AAOUpdate");
    // AAOUpdate is already true if AE needs calculation.
    if (i4AfState != ISync3AMgr::E_SYNC3AMGR_AF_STATE_SCANNING)
    {
        MINT32 i4Master = ISync3AMgr::getInstance()->getMasterDev();
        MINT32 i4Slave = ISync3AMgr::getInstance()->getSlaveDev();
        MINT32 i4ActiveAEItem = E_AE_AE_MASTER_APPLY;
        if(m_SensorDevId == i4Slave)
        {
            // Get PSO info
            IHal3AResultBufInfo::getInstance(m_SensorDevId)->getAAOInfo(AE_V4P0_BLOCK_NO, rBufInfo->mMagicNumber, m_rAAOInfo);
            // Get PSO buf info
            m_rAAOInfo.Pso_Output_Path = (PSO_PATH_SEL_ENUM)rPSOBufInfo->mRawType; // PSO before OB or after OB
            // AE
            IAeMgr::getInstance().setAAOProcInfo(m_SensorDevId, reinterpret_cast<MVOID *>(rPSOBufInfo->getPart(PSOSepBuf)), &m_rAAOInfo);
            i4ActiveAEItem = E_AE_AE_SLAVE_APPLY;
        }

        IAeMgr::getInstance().doPvAE(m_SensorDevId, rBufInfo->mTimeStamp, reinterpret_cast<MVOID *>(rBufInfo->getPart(AAOSepBuf)),
            i4ActiveAEItem, rBufInfo->mMagicNumber, 1);
        AAA_TRACE_D("UpdateSensor");
        if (IAeMgr::getInstance().IsNeedUpdateSensor(m_SensorDevId))
        {
        #if USE_AE_THD
            //m_pHal3A->mbPostAESenThd = MTRUE;
            m_pStateMgr->postToAESenThread();
        #else
            IAeMgr::getInstance().updateSensorbyI2C(m_SensorDevId);
        #endif
        }
        AAA_TRACE_END_D;
    }
    AAA_TRACE_END_D;

    return  S_3A_OK;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_PrecaptureStart
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCameraPreviewN3d::
sendIntent(intent2type<eIntent_PrecaptureStart>)
{
    return StateCameraPreview::sendIntent(intent2type<eIntent_PrecaptureStart>());
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AFStart
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCameraPreviewN3d::
sendIntent(intent2type<eIntent_AFStart>)
{
    if (ISync3AMgr::getInstance()->isActive() && ISync3AMgr::getInstance()->getSync3A()->isSyncEnable())
    {
        CAM_LOGD("[%s] Sync 2A: Sensor(%d), AFAE Start", __FUNCTION__, m_SensorDevId);
        ISync3AMgr::getInstance()->setAFState(ISync3AMgr::E_SYNC3AMGR_AF_STATE_BEGIN);
    }

    return StateCameraPreview::sendIntent(intent2type<eIntent_AFStart>());
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AFEnd
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCameraPreviewN3d::
sendIntent(intent2type<eIntent_AFEnd>)
{
    return StateCameraPreview::sendIntent(intent2type<eIntent_AFEnd>());
}

