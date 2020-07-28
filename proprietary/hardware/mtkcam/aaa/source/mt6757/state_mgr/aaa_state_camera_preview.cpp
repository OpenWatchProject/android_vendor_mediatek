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
#define LOG_TAG "aaa_state_pv"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <aaa_types.h>
#include <aaa_state.h>
#include <aaa_state_mgr.h>
#include <aaa_common_custom.h>
#include <aaa_hal_if.h>
#include <aaa_hal_sttCtrl.h>

#include <dbg_aaa_param.h>
#include <dbg_af_param.h>

#include <af_param.h>
#include <awb_param.h>
#include <aaa/flash_param.h>

#include <af_feature.h>
#include <af_algo_if.h>

#include <ae_mgr/ae_mgr_if.h>
#include <af_mgr/af_mgr_if.h>
#include <awb_mgr/awb_mgr_if.h>
#include <lsc_mgr/ILscTsf.h>
#include <flash_mgr/flash_mgr.h>
#include <flash_feature.h>
#include <flicker/flicker_hal_base.h>
#include <sensor_mgr/aaa_sensor_buf_mgr.h>

#include <awb_tuning_custom.h>
#include <flash_awb_param.h>
#include <flash_awb_tuning_custom.h>
#include <flash_tuning_custom.h>
#include <isp_tuning_mgr.h>
#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>
#include <debug/DebugUtil.h>
#include <cutils/properties.h>

using namespace NS3Av3;
using namespace NSIspTuning;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateCameraPreview
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
StateCameraPreview::
StateCameraPreview(MINT32 sensorDevId, StateMgr* pStateMgr)
    : IState("StateCameraPreview", sensorDevId, pStateMgr)
{
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_Uninit
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCameraPreview::
sendIntent(intent2type<eIntent_Uninit>)
{
    CAM_LOGD("[StateCameraPreview::sendIntent]<eIntent_Uninit>");
    // State transition: eState_CameraPreview --> eState_Uninit
    m_pStateMgr->transitState(eState_CameraPreview, eState_Uninit);
    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_CameraPreviewEnd
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCameraPreview::
sendIntent(intent2type<eIntent_CameraPreviewEnd>)
{
    MRESULT err = S_3A_OK;

    CAM_LOGD("[StateCameraPreview::sendIntent]<eIntent_CameraPreviewEnd>");
    // State transition: eState_CameraPreview --> eState_Init
    m_pStateMgr->transitState(eState_CameraPreview, eState_Init);

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_VsyncUpdate
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCameraPreview::
sendIntent(intent2type<eIntent_VsyncUpdate>)
{
    CAM_LOGD_IF(m_i4EnableLog, "CapQueueEmpty(%d)",m_pStateMgr->isCapQueueEmpty());
    if (!m_pStateMgr->isCapQueueEmpty())
    {
        doCaptureEnd();
    }
    else
    {
        if(FlashMgr::getInstance().isAFLampOn(m_SensorDevId) && !m_pStateMgr->getIsFlashOpened())
            doRestore(MTRUE);
        else
            doUpdate();
    }
    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_PrecaptureStart
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCameraPreview::
sendIntent(intent2type<eIntent_PrecaptureStart>)
{
    CAM_LOGD("[StateCameraPreview::sendIntent]<eIntent_PrecaptureStart>");

    // Init
    IAeMgr::getInstance().setAeMeterAreaEn(m_SensorDevId, 1);

    m_pStateMgr->resetPrecapState(); //reset Precap state
    // State transition: eState_CameraPreview --> eState_Precapture
    m_pStateMgr->transitState(eState_CameraPreview, eState_Precapture);

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_CaptureStart
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCameraPreview::
sendIntent(intent2type<eIntent_CaptureStart>)
{
    if(m_pStateMgr->getEnableShortExposure())
    {
        m_pStateMgr->setEnableShortExposure(MFALSE);
        m_pStateMgr->resetCheckCount();
    }

    // reset restore count
    m_pStateMgr->resetRestoreCount();

    // Update frame count
    m_pStateMgr->updateFrameCount();
    CAM_LOGD("sendIntent(intent2type<eIntent_CaptureStart>) line(%d), frame(%d)",__LINE__, m_pStateMgr->getFrameCount());

    MINT32 bIsFlashOn = MFALSE;

#if CAM3_FLASH_FEATURE_EN
    bIsFlashOn = (MINT32)m_pStateMgr->getIsFlashOpened();
    MY_LOG_IF(m_i4EnableLog, "[%s]  CaptureStart, StrobeMode=%d", __FUNCTION__, bIsFlashOn);
#endif

    // AWB: update AWB statistics config
    IAwbMgr::getInstance().setStrobeMode(m_SensorDevId, (bIsFlashOn ? AWB_STROBE_MODE_ON : AWB_STROBE_MODE_OFF));


    IAeMgr::getInstance().setStrobeMode(m_SensorDevId, (bIsFlashOn ? MTRUE : MFALSE));
    // AE: update capture parameter
    MBOOL bIsStartCapture = m_pStateMgr->getStartCapture();
    CAM_LOGD("[%s] bIsStartCapture:(%d)", __FUNCTION__, bIsStartCapture);
    IAeMgr::getInstance().doCapAE(m_SensorDevId);

    CAM_LOGI("[%s] Update shading ratio for doCapAE", __FUNCTION__);
    // LSC: update shading ratio
    NSIspTuningv3::IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_SensorDevId, NSIspTuningv3::IspTuningMgr::E_ISPTUNING_UPDATE_LSC_INFO, NSIspTuningv3::IspTuningMgr::E_SHADING_RATIO_FREE_RUN, 0);
    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AFStart
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCameraPreview::
sendIntent(intent2type<eIntent_AFStart>)
{
    CAM_LOGD("[StateCameraPreview::sendIntent]<eIntent_AFStart>");

    IAeMgr::getInstance().setAeMeterAreaEn(m_SensorDevId, 1);

#if CAM3_FLASH_FEATURE_EN
    FlashMgr::getInstance().notifyAfEnter(m_SensorDevId);
#endif

    m_pStateMgr->setNextState(eState_Invalid); //reset 3A Next state
    m_pStateMgr->resetAFState(); //only single entrance point: EAFState_T=0
    m_pStateMgr->mAFStateCntSet.resetAll(); //reset all AFState cnt, flags
    m_pStateMgr->transitState(eState_CameraPreview, eState_AF);

    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AFEnd
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCameraPreview::
sendIntent(intent2type<eIntent_AFEnd>)
{
    CAM_LOGD("[StateCameraPreview::sendIntent]<eIntent_AFEnd>");

    return  S_3A_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  doCaptureEnd
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCameraPreview::
doCaptureEnd()
{
    MINT32 i4SceneLv = 80;
    MINT32 i4AoeCompLv = 80;
    MBOOL bAEStable = MTRUE;


    StatisticBufInfo* rBufInfo;

    rBufInfo = Hal3ASttCtrl::getInstance(m_SensorDevId)->getBufMgr(BUF_AAO)->dequeueSwBuf();
    if(rBufInfo == NULL) return S_3A_OK;
    CAM_LOGD("[StateCameraPreview::doCaptureEnd()] dequeueSwBuf AAO done\n");
    StateCameraPreview::m_pStateMgr->updateMagicNumber(rBufInfo->mMagicNumber);
    CAM_LOGD("[%s] magic number = %d",__FUNCTION__, m_pStateMgr->queryMagicNumber());

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
            IAwbMgr::getInstance().doCapAWB(m_SensorDevId, i4AoeCompLv, reinterpret_cast<MVOID *>(rBufInfo->getPart(AAOSepAWBBuf)));
        }
        CAM_LOGD_IF(m_i4EnableLog, "IAwbMgr::getInstance().doCapAWB() END");

        IAeMgr::getInstance().doCapFlare(m_SensorDevId, reinterpret_cast<MVOID *>(rBufInfo->getPart(AAOSepBuf)),
        FlashMgr::getInstance().isFlashOnCapture(m_SensorDevId) );
        CAM_LOGD_IF(m_i4EnableLog, "IAeMgr::getInstance().doCapFlare() END");

#if CAM3_LSC_FEATURE_EN
    // TSF
        AWB_OUTPUT_T rAWBOutput;
        IAwbMgr::getInstance().getAWBOutput(m_SensorDevId, rAWBOutput);
        ILscMgr::TSF_AWB_INFO rAwbInfo;
        ILscMgr::TSF_INPUT_INFO_T rTsfInfo;
        rAwbInfo.m_FLASH_ON = MFALSE; // initialized
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
        if(FlashMgr::getInstance().isAFLampOn(m_SensorDevId))
            doRestore();
    }
    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  doCaptureEnd
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCameraPreview::
doUpdate()
{
    MINT32 i4SceneLv = 80;
    MINT32 i4AoeCompLv = 80;
    MBOOL bAEStable = MTRUE;
    StatisticBufInfo* rBufInfo;

    // Update frame count
    AAA_TRACE_L(doFrameCount);
    m_pStateMgr->updateFrameCount();
    AAA_TRACE_END_L;
    CAM_LOGD_IF(m_i4EnableLog
            , "[StateCameraPreview::doUpdate()] enter, frameCnt=%d\n"
            , m_pStateMgr->getFrameCount());


    if (m_pStateMgr->getFrameCount() < 0)
    {   // AAO statistics is not ready
        //m_pHal3A->m_b3APvInitOK = MFALSE;
        return S_3A_OK;
    }

    // Dequeue AAO buffer from SW buffer
    AAA_TRACE_L(DequeueSwBuf);
    rBufInfo = Hal3ASttCtrl::getInstance(m_SensorDevId)->getBufMgr(BUF_AAO)->dequeueSwBuf();
    AAA_TRACE_END_L;
    if(rBufInfo == NULL) return S_3A_OK;
    AAA_TRACE_L(DetectPreframe);
    DebugUtil::getInstance(m_SensorDevId)->detectPreframe(DBG_AE, rBufInfo->mFrameCount);
    AAA_TRACE_END_L;
    CAM_LOGD_IF(m_i4EnableLog, "[StateCameraPreview::sendIntent<eIntent_VsyncUpdate>] dequeueSwBuf AAO done\n");
    AAA_TRACE_L(updateMagicNumber);
    StateCameraPreview::m_pStateMgr->updateMagicNumber(rBufInfo->mMagicNumber);
    AAA_TRACE_END_L;
    CAM_LOGD_IF(m_i4EnableLog, "[%s] magic number = %d",__FUNCTION__, m_pStateMgr->queryMagicNumber());


#if CAM3_LSC_FEATURE_EN
    // TSF
    AAA_TRACE_L(TSF);
    AWB_OUTPUT_T rAWBOutput;
    IAwbMgr::getInstance().getAWBOutput(m_SensorDevId, rAWBOutput);
    ILscMgr::TSF_AWB_INFO rAwbInfo;
    ILscMgr::TSF_INPUT_INFO_T rTsfInfo;
    rAwbInfo.m_FLASH_ON = MFALSE; // initialized
    ILscMgr* pLsc = ILscMgr::getInstance(static_cast<ESensorDev_T>(m_SensorDevId));
    rAwbInfo.m_i4LV        = IAeMgr::getInstance().getLVvalue(m_SensorDevId, MTRUE);
    rAwbInfo.m_u4CCT    = IAwbMgr::getInstance().getAWBCCT(m_SensorDevId);
    rAwbInfo.m_RGAIN    = rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R;
    rAwbInfo.m_GGAIN    = rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G;
    rAwbInfo.m_BGAIN    = rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B;
    rAwbInfo.m_FLUO_IDX = rAWBOutput.rAWBInfo.i4FluorescentIndex;
    rAwbInfo.m_DAY_FLUO_IDX = rAWBOutput.rAWBInfo.i4DaylightFluorescentIndex;
    rTsfInfo.eCmd = (0 == m_pStateMgr->getFrameCount()) ? ILscMgr::E_TSF_CMD_BATCH : ILscMgr::E_TSF_CMD_RUN;
    rTsfInfo.u4FrmId = rBufInfo->mMagicNumber;
    rTsfInfo.rAwbInfo = rAwbInfo;
    rTsfInfo.prAwbStat = reinterpret_cast<MUINT8*>(rBufInfo->getPart(AAOSepLSCBuf));
    rTsfInfo.u4SizeAwbStat = rBufInfo->getPartSize(AAOSepLSCBuf);

    NSIspTuningv3::IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_SensorDevId, NSIspTuningv3::IspTuningMgr::E_ISPTUNING_UPDATE_LSC_INFO, NSIspTuningv3::IspTuningMgr::E_SHADING_RATIO_FREE_RUN, 0);
    pLsc->updateTsf(rTsfInfo);

    CAM_LOGD_IF(m_i4EnableLog, "lv(%d),cct(%d),rgain(%d),bgain(%d),ggain(%d),fluoidx(%d), dayflouidx(%d)",
            rAwbInfo.m_i4LV,
            rAwbInfo.m_u4CCT,
            rAwbInfo.m_RGAIN,
            rAwbInfo.m_GGAIN,
            rAwbInfo.m_BGAIN,
            rAwbInfo.m_FLUO_IDX,
            rAwbInfo.m_DAY_FLUO_IDX
            );
    AAA_TRACE_END_L;
#endif

#if CAM3_AF_FEATURE_EN

    AAA_TRACE_L(set AFAE lock);
    if(CUST_LOCK_AE_DURING_CAF())
    {
        if ((IAfMgr::getInstance().isLockAE(m_SensorDevId)   == MFALSE) || //if =1, lens are fixed, do AE as usual; if =0, lens are moving, don't do AE
            (IAeMgr::getInstance().IsAEStable(m_SensorDevId) == MFALSE) ||    //guarantee AE can doPvAE at beginning, until IsAEStable()=1
            IAeMgr::getInstance().isLVChangeTooMuch(m_SensorDevId))
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
    AAA_TRACE_END_L;
#endif

    AAA_TRACE_L(Bypass doPvAE);
    if(m_pStateMgr->getEnableShortExposure())
    {
        if(m_pStateMgr->updateCheckCount() == 0)
        {
            m_pStateMgr->setEnableShortExposure(MFALSE);
            m_pStateMgr->resetCheckCount();
        }
        CAM_LOGD("[%s] bypass doPvAE", __FUNCTION__);
    }
    AAA_TRACE_END_L;

    AAA_TRACE_L(doPvAE);
    if(!m_pStateMgr->getEnableShortExposure())
    {
        if(Hal3ASttCtrl::getInstance(m_SensorDevId)->isMvHDREnable())
        {
            StatisticBufInfo* rMvHDRBufInfo = Hal3ASttCtrl::getInstance(m_SensorDevId)->getBufMgr(BUF_MVHDR)->dequeueSwBuf();
            if(rMvHDRBufInfo == NULL) return S_3A_OK;
            IAeMgr::getInstance().doPvAE(m_SensorDevId, m_pStateMgr->getFrameCount(), reinterpret_cast<MVOID *>(rMvHDRBufInfo->mVa), 0, rMvHDRBufInfo->mMagicNumber, 0);
        } else
        {
            IAeMgr::getInstance().doPvAE(m_SensorDevId, m_pStateMgr->getFrameCount(), reinterpret_cast<MVOID *>(rBufInfo->getPart(AAOSepBuf)), 0, rBufInfo->mMagicNumber, 0);
        }
    }
    AAA_TRACE_END_L;

    // Trigger FLK
    StatisticBufInfo* rFlkBufInfo = NULL;
    if(Hal3ASttCtrl::getInstance(m_SensorDevId)->getBufMgr(BUF_FLKO) != NULL){
        AAA_TRACE_L(FLKdequeueSwBuf);
        rFlkBufInfo = Hal3ASttCtrl::getInstance(m_SensorDevId)->getBufMgr(BUF_FLKO)->dequeueSwBuf();
        AAA_TRACE_END_L;
    }
    if(m_pStateMgr->m_pThreadRaw != NULL)
        m_pStateMgr->m_pThreadRaw->triggerFLKThread(rFlkBufInfo);

    // notify AE RT params.
    AAA_TRACE_L(Cb_vHDR);
    FrameOutputParam_T rRTParams;
    IAeMgr::getInstance().getRTParams(m_SensorDevId,rRTParams);
    m_pStateMgr->doNotifyCb(I3ACallBack::eID_NOTIFY_AE_RT_PARAMS, rBufInfo->mMagicNumber, (MINTPTR)&rRTParams, 0);
    // Auto HDR Detection, default is -1
    // -1: auto hdr off, 0: not detected, 1: detected
    MINT32 autoHdrRes = IAeMgr::getInstance().getAEHDROnOff(m_SensorDevId);
    CAM_LOGD_IF(m_i4EnableLog, "[%s] autoHdrRes(%d)",__FUNCTION__, autoHdrRes);
    m_pStateMgr->doNotifyCb(I3ACallBack::eID_NOTIFY_HDRD_RESULT, reinterpret_cast<MINTPTR> (&autoHdrRes), 0, 0);
    AAA_TRACE_END_L;

    AAA_TRACE_L(AePost);
    if (IAeMgr::getInstance().IsNeedUpdateSensor(m_SensorDevId) && !m_pStateMgr->getEnableShortExposure())
    {
    #if USE_AE_THD
        //m_pHal3A->mbPostAESenThd = MTRUE;
        m_pStateMgr->postToAESenThread();
    #else
        IAeMgr::getInstance().updateSensorbyI2C(m_SensorDevId);
    #endif
    }
    AAA_TRACE_END_L;

    NSIspTuningv3::IspTuningMgr::GMA_AE_DYNAMIC_INFO dynamicInfo;
    dynamicInfo.bStable = IAeMgr::getInstance().IsAEStable(m_SensorDevId);
    NSIspTuningv3::IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_SensorDevId, NSIspTuningv3::IspTuningMgr::E_ISPTUNING_SET_GMA_AE_DYNAMIC, (MINTPTR)&dynamicInfo, 0);

    // workaround for iVHDR
    AAA_TRACE_L(SGG);
    MUINT32 u4AFSGG1Gain;
    IAeMgr::getInstance().getAESGG1Gain(m_SensorDevId, &u4AFSGG1Gain);
    IAfMgr::getInstance().setSGGPGN(m_SensorDevId, (MINT32) u4AFSGG1Gain);
    AAA_TRACE_END_L;

    CAM_LOGD_IF(m_i4EnableLog, "[StateCameraPreview::sendIntent<eIntent_VsyncUpdate>] doPvAE done\n");

    // AE
    AAA_TRACE_L(getAEValue);
    //i4SceneLv = IAeMgr::getInstance().getLVvalue(m_SensorDevId, (m_pHal3A->get3APreviewMode() == EPv_Normal) ? MFALSE : MTRUE);
    //i4AoeCompLv = IAeMgr::getInstance().getAOECompLVvalue(m_SensorDevId, (m_pHal3A->get3APreviewMode() == EPv_Normal) ? MFALSE : MTRUE);
    i4SceneLv = IAeMgr::getInstance().getLVvalue(m_SensorDevId, MFALSE);
    i4AoeCompLv = IAeMgr::getInstance().getAOECompLVvalue(m_SensorDevId, MFALSE);
    bAEStable = IAeMgr::getInstance().IsAEStable(m_SensorDevId);
    MINT32 i4AeMode = IAeMgr::getInstance().getAEMode(m_SensorDevId);
    AAA_TRACE_END_L;

    // AWB
    AAA_TRACE_L(AWB);
    IAwbMgr::getInstance().doPvAWB(m_SensorDevId, m_pStateMgr->getFrameCount(), bAEStable, i4AoeCompLv, reinterpret_cast<MVOID *>(rBufInfo->getPart(AAOSepAWBBuf)));
    CAM_LOGD_IF(m_i4EnableLog, "[StateCameraPreview::sendIntent<eIntent_VsyncUpdate>] doPvAWB done\n");
    AAA_TRACE_END_L;

    return  S_3A_OK;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  doRestore
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StateCameraPreview::
doRestore(MBOOL bIsFlashOn)
{
    // Update frame count
    m_pStateMgr->updateFrameCount();
    m_pStateMgr->updateRestoreCount();

    CAM_LOGD_IF(m_i4EnableLog
            , "[StateCameraPreview::doRestore()] enter, frameCnt(%d), restoreCount(%d)\n"
            , m_pStateMgr->getFrameCount(), m_pStateMgr->getRestoreCount());
    if(bIsFlashOn)
    {
        if(m_pStateMgr->getRestoreCount() == 1 && !m_pStateMgr->getEnableShortExposure())
        {
/*
            // restore AE
            CAM_LOGD("[%s] restore AE", __FUNCTION__);
            IAeMgr::getInstance().doRestoreAEInfo(m_SensorDevId, MTRUE);
            IAeMgr::getInstance().setRestore(m_SensorDevId, m_pStateMgr->getRestoreCount());
            // restore AWB
            CAM_LOGD("[%s] restore AWB", __FUNCTION__);
            IAwbMgr::getInstance().restore(m_SensorDevId);
*/
        }
        if(m_pStateMgr->getRestoreCount() == 2 && !m_pStateMgr->getEnableShortExposure())
        {
            // restore AE
            CAM_LOGD("[%s] restore AE", __FUNCTION__);
            IAeMgr::getInstance().doRestoreAEInfo(m_SensorDevId, MTRUE);
            IAeMgr::getInstance().setRestore(m_SensorDevId, 0);
            // restore AWB
            CAM_LOGD("[%s] restore AWB", __FUNCTION__);
            IAwbMgr::getInstance().restore(m_SensorDevId);
        }
        else if(m_pStateMgr->getRestoreCount() == 3)
        {
#if CAM3_FLASH_FEATURE_EN
            FlashMgr::getInstance().setAFLampOnOff(m_SensorDevId,MFALSE);
            //FlashMgr::getInstance().setAFLampOnOff(m_SensorDevId, 0);
#endif
            IAwbMgr::getInstance().setStrobeMode(m_SensorDevId, AWB_STROBE_MODE_OFF);
            IAeMgr::getInstance().setStrobeMode(m_SensorDevId, MFALSE);
            m_pStateMgr->resetRestoreCount();
        }
    }else
    {
        if(m_pStateMgr->getRestoreCount() == 2)
        {
            // restore AE
            CAM_LOGD("[%s] FlashOff : restore AE", __FUNCTION__);
            IAeMgr::getInstance().doRestoreAEInfo(m_SensorDevId, MTRUE);
            IAeMgr::getInstance().setRestore(m_SensorDevId, 0);
            // restore AWB
            CAM_LOGD("[%s] FlashOff : restore AWB", __FUNCTION__);
            IAwbMgr::getInstance().restore(m_SensorDevId);
            m_pStateMgr->resetRestoreCount();
        }
    }

    CAM_LOGD_IF(m_i4EnableLog
            , "[StateCameraPreview::doRestore()] end, frameCnt=%d\n"
            , m_pStateMgr->getFrameCount());
    return  S_3A_OK;
}

