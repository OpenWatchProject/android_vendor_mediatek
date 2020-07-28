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
#define LOG_TAG "aaa_state_pcap"

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

#include <aaa/af_param.h>
#include <aaa/awb_param.h>
#include <aaa/flash_param.h>

#include <af_feature.h>
#include <af_algo_if.h>

#include <ae_mgr/ae_mgr_if.h>
#include <af_mgr/af_mgr_if.h>
#include <awb_mgr/awb_mgr_if.h>
#include <lsc/ILscMgr.h>
#include <flash_mgr/flash_mgr.h>
#include <flash_feature.h>
#include <flicker/flicker_hal_base.h>
#include <sensor_mgr/aaa_sensor_buf_mgr.h>
#include <sensor_mgr/aaa_sensor_mgr.h>
#include <awb_tuning_custom.h>
#include <flash_awb_param.h>
#include <flash_awb_tuning_custom.h>
#include <flash_tuning_custom.h>
#include <isp_tuning/isp_tuning_mgr.h>
#include <IHal3AResultBufInfo.h>

using namespace NS3Av3;
using namespace NSIspTuning;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StatePrecapture
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
StatePrecapture::
StatePrecapture(MINT32 sensorDevId, StateMgr* pStateMgr)
    : IState("StatePrecapture", sensorDevId, pStateMgr)
{
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_CameraPreviewEnd
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StatePrecapture::
sendIntent(intent2type<eIntent_CameraPreviewEnd>)
{
    CAM_LOGD("[StatePrecapture::sendIntent]<eIntent_CameraPreviewEnd>");

    //flash turn off
#if CAM3_FLASH_FEATURE_EN
    FlashMgr::getInstance().endPrecapture(m_SensorDevId);
#endif

    m_pStateMgr->resetPrecapState();//flash turn off
    // State transition: eState_Precapture --> eState_Init
    m_pStateMgr->transitState(eState_Precapture, eState_Init);
    return  S_3A_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_PrecaptureEnd
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StatePrecapture::
sendIntent(intent2type<eIntent_PrecaptureEnd>)
{
    CAM_LOGD("sendIntent(intent2type<eIntent_PrecaptureEnd>) line=%d",__LINE__);

  //flash turn off
#if CAM3_FLASH_FEATURE_EN
    FlashMgr::getInstance().endPrecapture(m_SensorDevId);
#endif

    m_pStateMgr->resetPrecapState();
    m_pStateMgr->transitState(eState_Precapture, eState_CameraPreview);
    m_pStateMgr->setUpdateStatus(MTRUE);//precapture cancel, update continue
    return  S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_VsyncUpdate
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
StatePrecapture::
sendIntent(intent2type<eIntent_VsyncUpdate>)
{
    MRESULT err = S_3A_OK;

    // Dequeue AAO buffer from SW buffer
    IBufMgr* pAAOBufMgr = Hal3ASttCtrl::getInstance(m_SensorDevId)->getBufMgr(BUF_AAO);
    if(pAAOBufMgr == NULL) return S_3A_OK;
    StatisticBufInfo* rBufInfo = pAAOBufMgr->dequeueSwBuf();
    if(rBufInfo == NULL) return S_3A_OK;

    // Dequeue PSO buffer form SW buffer
    IBufMgr* pPSOBufMgr = Hal3ASttCtrl::getInstance(m_SensorDevId)->getBufMgr(BUF_PSO);
    if(pPSOBufMgr == NULL) return S_3A_OK;
    StatisticBufInfo* rPSOBufInfo = pPSOBufMgr->dequeueSwBuf();
    if(rPSOBufInfo == NULL) return S_3A_OK;

    m_pStateMgr->updateMagicNumber(rBufInfo->mMagicNumber);
    m_pStateMgr->updatePrecapCount();

    m_pStateMgr->doNotifyCb(I3ACallBack::ECallBack_T::eID_NOTIFY_RAW_STTMAGICNUM, NULL, NULL, NULL);

    EPrecapState_T ePrecapState = m_pStateMgr->getPrecapState();
    MUINT32 u4MagicNum = m_pStateMgr->queryMagicNumber();
    MUINT32 u4FrmCnt = m_pStateMgr->getFrameCount();
    MUINT32 u4PrecapCnt = m_pStateMgr->getPrecapCount();
    CAM_LOGD("[StatePrecapture::sendIntent]<eIntent_VsyncUpdate> line=%d, frameCnt(%d), PrecapCnt(%d), EPrecapState(%d), #(%d)"
    , __LINE__, u4FrmCnt, u4PrecapCnt, static_cast<int>(ePrecapState), u4MagicNum);

    m_pStateMgr->mAFStateCntSet.resetAll(); //reset all AFState cnt, flags
    if (m_pStateMgr->getPrecapState() == ePrecap_AF_BeforeFlash)
    {
        err = sendPrecapIntent(intent2type<eIntent_VsyncUpdate>(), state2type<ePrecap_AF_BeforeFlash>());
        if (err != S_3A_OK) CAM_LOGE("sendPrecapIntent(eIntent_VsyncUpdate, ePrecap_AF_BeforeFlash) return(%d)\n", err);
    }
    if ((m_pStateMgr->getPrecapState() == ePrecap_AE) && (m_pStateMgr->mAFStateCntSet.bNeedToDoPrecapAF == 0))
    {
        err = sendPrecapIntent(intent2type<eIntent_VsyncUpdate>(), state2type<ePrecap_AE>(), rBufInfo, rPSOBufInfo);
        if (err != S_3A_OK) CAM_LOGE("sendPrecapIntent(eIntent_VsyncUpdate, ePrecap_AE) return(%d)\n", err);
    }
    //Note: not "else if ..." because after AE finishes, flash should continue to make Precapture faster.
    if ((m_pStateMgr->getPrecapState() == ePrecap_Flash) && (m_pStateMgr->mAFStateCntSet.bNeedToDoPrecapAF == 0))
    {
        err = sendPrecapIntent(intent2type<eIntent_VsyncUpdate>(), state2type<ePrecap_Flash>(), rBufInfo);
        if (err != S_3A_OK) CAM_LOGE("sendPrecapIntent(eIntent_VsyncUpdate, ePrecap_Flash) return(%d)\n", err);
    }
    if ((m_pStateMgr->getPrecapState() == ePrecap_AF_AfterFlash) && (m_pStateMgr->mAFStateCntSet.bNeedToDoPrecapAF == 0))
    {
        err = sendPrecapIntent(intent2type<eIntent_VsyncUpdate>(), state2type<ePrecap_AF_AfterFlash>());
        if (err != S_3A_OK) CAM_LOGE("sendPrecapIntent(eIntent_VsyncUpdate, ePrecap_AF_AfterFlash) return(%d)\n", err);
    }

#if CAM3_LSC_FEATURE_EN
    // TSF
    AWB_OUTPUT_T rAWBOutput;
    IAwbMgr::getInstance().getAWBOutput(m_SensorDevId, rAWBOutput);
    ILscMgr::TSF_AWB_INFO rAwbInfo;
    ILscMgr::TSF_INPUT_INFO_T rTsfInfo;
    rAwbInfo.m_FLASH_ON = MFALSE; // initialized
    ILscMgr* pLsc = ILscMgr::getInstance(static_cast<ESensorDev_T>(m_SensorDevId));
    rAwbInfo.m_i4LV   = IAeMgr::getInstance().getLVvalue(m_SensorDevId, MTRUE);
    rAwbInfo.m_u4CCT  = IAwbMgr::getInstance().getAWBCCT(m_SensorDevId);
    rAwbInfo.m_RGAIN  = rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R;
    rAwbInfo.m_GGAIN  = rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G;
    rAwbInfo.m_BGAIN  = rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B;
    rAwbInfo.m_FLUO_IDX = rAWBOutput.rAWBInfo.i4FluorescentIndex;
    rAwbInfo.m_DAY_FLUO_IDX = rAWBOutput.rAWBInfo.i4DaylightFluorescentIndex;
    rTsfInfo.eCmd = ILscMgr::E_TSF_CMD_RUN;
    rTsfInfo.u4FrmId = m_pStateMgr->getFrameCount();
    rTsfInfo.rAwbInfo = rAwbInfo;
    rTsfInfo.prAwbStat = reinterpret_cast<MUINT8*>(rBufInfo->getPart(AAOSepLSCBuf));
    rTsfInfo.u4SizeAwbStat = rBufInfo->getPartSize(AAOSepLSCBuf);

    MINT32 i4IsLockRto = NSIspTuningv3::IspTuningMgr::E_SHADING_RATIO_FREE_RUN;
    if(m_pStateMgr->getPrecapState() == ePrecap_Flash)
    {
        i4IsLockRto = NSIspTuningv3::IspTuningMgr::E_SHADING_RATIO_LOCK;
        CAM_LOGD("PreCapFlash state to LockRto %d\n", i4IsLockRto);
    }
    else
        i4IsLockRto = NSIspTuningv3::IspTuningMgr::E_SHADING_RATIO_FREE_RUN;
    NSIspTuningv3::IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_SensorDevId, NSIspTuningv3::IspTuningMgr::E_ISPTUNING_UPDATE_LSC_INFO, i4IsLockRto, 0);
    pLsc->updateTsf(rTsfInfo);

    CAM_LOGD_IF(m_i4EnableLog, "lv %d, cct %d, rgain %d, bgain %d, ggain %d, fluo idx %d, day flou idx %d, i4IsLockRto %d\n",
        rAwbInfo.m_i4LV,
        rAwbInfo.m_u4CCT,
        rAwbInfo.m_RGAIN,
        rAwbInfo.m_GGAIN,
        rAwbInfo.m_BGAIN,
        rAwbInfo.m_FLUO_IDX,
        rAwbInfo.m_DAY_FLUO_IDX,
        i4IsLockRto
    );
#endif

    //Note: not "else if ...", if flash finishes this frame, then we announce ReadyToCapture to Middleware
    if ((m_pStateMgr->getPrecapState() == ePrecap_Num) && (m_pStateMgr->mAFStateCntSet.bNeedToDoPrecapAF == 0))
    {
        // at this moment, all operation of Precapture must have finished. Then wait for Capture command
#if CAM3_FLASH_FEATURE_EN
        if(FlashMgr::getInstance().isFlashOnCapture(m_SensorDevId) && !m_pStateMgr->getIsFlashOpened())
        {

            // set short exposure time
            AE_MODE_CFG_T rPreviewParams;
            IAeMgr::getInstance().getPreviewParams(m_SensorDevId, rPreviewParams);
            if((rPreviewParams.u4Eposuretime > 120000))
            {
                CAM_LOGD("[%s] Enable Short Exposure ", __FUNCTION__);
                m_pStateMgr->setEnableShortExposure(MTRUE);

                AE_MODE_CFG_T rCaptureInfo;
                CaptureParam_T rCaptureParams;
                IAeMgr::getInstance().getCaptureParams(m_SensorDevId, rCaptureInfo);

                rCaptureParams.u4ExposureMode = rCaptureInfo.u4ExposureMode;
                rCaptureParams.u4Eposuretime = 29997;
                rCaptureParams.u4AfeGain = 16320;
                rCaptureParams.u4IspGain = 1296;
                rCaptureParams.u4RealISO = 2000;
                rCaptureParams.u4FlareGain = (MUINT32) rCaptureInfo.i2FlareGain;
                rCaptureParams.u4FlareOffset = (MUINT32) rCaptureInfo.i2FlareOffset;
                rCaptureParams.i4LightValue_x10 = IAeMgr::getInstance().getCaptureLVvalue(m_SensorDevId);

                IAeMgr::getInstance().setSensorDirectly(m_SensorDevId, rCaptureParams, 1);
            }
            FlashMgr::getInstance().setCapPara(m_SensorDevId);
        }
        FlashMgr::getInstance().endPrecapture(m_SensorDevId);
#endif
//        IAeMgr::getInstance().doRestoreAEInfo(m_SensorDevId, MTRUE);
        IAeMgr::getInstance().setAEState2Converge(m_SensorDevId);

        m_pStateMgr->resetPrecapState();
        m_pStateMgr->transitState(eState_Precapture, eState_CameraPreview);
        m_pStateMgr->setUpdateStatus(MFALSE); //shouldnt update after precapture state until capture again
    }
    return  err;
}

MRESULT
StatePrecapture::
sendPrecapIntent(intent2type<eIntent_VsyncUpdate>, state2type<ePrecap_AE>, MVOID* pBufInfo, StatisticBufInfo* rPSOBufInfo)
{
    CAM_LOGD_IF(m_i4EnableLog, "StatePrecapture::sendIntent(eIntent_VsyncUpdate, ePrecap_AE)");

    MRESULT err = S_3A_OK;
    MBOOL bIsStrobeFired = MFALSE;
    StatisticBufInfo* pBuf = reinterpret_cast<StatisticBufInfo*>(pBufInfo);
    MINT32 i4SceneLv = 80;
    MINT32 i4AoeCompLv = 80;
    MBOOL bAEStable = MTRUE;
    AAO_PROC_INFO_T rAAOInfo;

    // Update frame count
    m_pStateMgr->updateFrameCount();
    // One-shot AE or strobe AE
    if(Hal3ASttCtrl::getInstance(m_SensorDevId)->isMvHDREnable())
    {
        IBufMgr* pMvHDRBufMgr = Hal3ASttCtrl::getInstance(m_SensorDevId)->getBufMgr(BUF_MVHDR);
        if(pMvHDRBufMgr == NULL) return S_3A_OK;

        StatisticBufInfo* rMvHDRBufInfo = pMvHDRBufMgr->dequeueSwBuf();
        if(rMvHDRBufInfo == NULL) return S_3A_OK;
        IAeMgr::getInstance().doPreCapAE(m_SensorDevId, rMvHDRBufInfo->mTimeStamp, bIsStrobeFired, reinterpret_cast<MVOID *>(rMvHDRBufInfo->mVa), 0, MTRUE, 0);
    } else
    {
        // Get PSO info
        IHal3AResultBufInfo::getInstance(m_SensorDevId)->getAAOInfo(AE_V4P0_BLOCK_NO, pBuf->mMagicNumber, rAAOInfo);
        // Get PSO buf info
        rAAOInfo.Pso_Output_Path = (PSO_PATH_SEL_ENUM)rPSOBufInfo->mRawType; // PSO before OB or after OB
        // AE
        IAeMgr::getInstance().setAAOProcInfo(m_SensorDevId, reinterpret_cast<MVOID *>(rPSOBufInfo->getPart(PSOSepBuf)), &rAAOInfo);
        IAeMgr::getInstance().doPreCapAE(m_SensorDevId, pBuf->mTimeStamp, bIsStrobeFired, reinterpret_cast<MVOID *>(pBuf->getPart(AAOSepBuf)), 0, MTRUE, 0);
    }

    if (IAeMgr::getInstance().IsNeedUpdateSensor(m_SensorDevId))
    {
    #if USE_AE_THD
        //m_pHal3A->mbPostAESenThd = MTRUE;
        m_pStateMgr->postToAESenThread();
    #else
        IAeMgr::getInstance().updateSensorbyI2C(m_SensorDevId);
    #endif
    }
    NSIspTuningv3::IspTuningMgr::GMA_AE_DYNAMIC_INFO dynamicInfo;
    dynamicInfo.bStable = IAeMgr::getInstance().IsAEStable(m_SensorDevId);
    NSIspTuningv3::IspTuningMgr::getInstance().sendIspTuningIOCtrl(m_SensorDevId, NSIspTuningv3::IspTuningMgr::E_ISPTUNING_SET_GMA_AE_DYNAMIC, (MINTPTR)&dynamicInfo, 0);


    i4SceneLv = IAeMgr::getInstance().getLVvalue(m_SensorDevId, MFALSE);
    i4AoeCompLv = IAeMgr::getInstance().getAOECompLVvalue(m_SensorDevId, MFALSE);
    bAEStable = IAeMgr::getInstance().IsAEStable(m_SensorDevId);

    // workaround for iVHDR
    MUINT32 u4AFSGG1Gain;
    IAeMgr::getInstance().getAESGG1Gain(m_SensorDevId, &u4AFSGG1Gain);
    IAfMgr::getInstance().setSGGPGN(m_SensorDevId, (MINT32) u4AFSGG1Gain);

    if(bAEStable) // AE is stable
    {

        // One-shot AWB without strobe
        IAwbMgr::getInstance().doPreCapAWB(m_SensorDevId, i4AoeCompLv, reinterpret_cast<MVOID *>(pBuf->getPart(AAOSepLSCBuf)));

        // backup AE/AWB
        IAeMgr::getInstance().doBackAEInfo(m_SensorDevId);
        IAwbMgr::getInstance().backup(m_SensorDevId);
        CAM_LOGD("doBackAEInfo");

        // both one-shot AE and one-shot AWB are done
        m_pStateMgr->proceedPrecapState();
    }
    return err;
}


MRESULT
StatePrecapture::
sendPrecapIntent(intent2type<eIntent_VsyncUpdate>, state2type<ePrecap_Flash>, MVOID* pBufInfo)
{
    CAM_LOGD_IF(m_i4EnableLog, "StatePrecapture::sendIntent(eIntent_VsyncUpdate, ePrecap_Flash)");
    if(m_pStateMgr->getIsFlashOpened())
    {
        CAM_LOGD("[%s] Flash has opened, bypass Flash status", __FUNCTION__);
        m_pStateMgr->proceedPrecapState();
        return S_3A_OK;
    }
  MRESULT err = S_3A_OK;
    StatisticBufInfo* pBuf = reinterpret_cast<StatisticBufInfo*>(pBufInfo);
    FlashExePara para;
    FlashExeRep rep;
    FLASH_AWB_PASS_FLASH_INFO_T passFlashAwbData;
    MINT32 i4AoeCompLv = 80;
    para.staBuf =reinterpret_cast<MVOID *>(pBuf->getPart(AAOSepBuf));

    AWB_STAT_PARAM_T rAWBStatParam;

#if CAM3_FLASH_FEATURE_EN
    switch (m_SensorDevId)
    {
    case ESensorDev_Main: //  Main Sensor
        rAWBStatParam = getAWBStatParam<ESensorDev_Main>();
        break;
    case ESensorDev_MainSecond: //  Main Second Sensor
        rAWBStatParam = getAWBStatParam<ESensorDev_MainSecond>();
        break;
    case ESensorDev_Sub: //  Sub Sensor
        rAWBStatParam = getAWBStatParam<ESensorDev_Sub>();
        break;
    default:
        CAM_LOGE("m_eSensorDev = %d", m_SensorDevId);
        rAWBStatParam = getAWBStatParam<ESensorDev_Main>();
        break;
    }
    para.staX = rAWBStatParam.i4WindowNumX;
    para.staY = rAWBStatParam.i4WindowNumY;
    para.isBurst = 0;
    AWB_OUTPUT_T awb_out;
    IAwbMgr::getInstance().getAWBOutput(m_SensorDevId, awb_out);
    para.NonPreferencePrvAWBGain = awb_out.rPreviewFullWBGain;
    para.PreferencePrvAWBGain = awb_out.rPreviewAWBGain;
    para.i4AWBCCT = awb_out.rAWBInfo.i4CCT;
    int flickerMode;
    FlickerHalBase::getInstance().getFlickerResult(flickerMode);
    if(flickerMode==HAL_FLICKER_AUTO_50HZ)
        para.flickerMode = FlashMgr::e_Flicker50;
    else if(flickerMode==HAL_FLICKER_AUTO_60HZ)
        para.flickerMode = FlashMgr::e_Flicker60;
    else
        para.flickerMode = FlashMgr::e_FlickerUnknown;
    FlashMgr::getInstance().doPfOneFrame(m_SensorDevId, &para, &rep);

    // close flash flow is at doRestore().
    MINT32 flashOnCnt = FlashMgr::getInstance().getFlashOnFrmCnt(m_SensorDevId);
    if(flashOnCnt == 1)
    {
        // frame 1 : modify frame rate to prevent the timing of closing flash is when sensor exposuring.
        AE_MODE_CFG_T rPreviewInfo;
        IAeMgr::getInstance().getPreviewParams(m_SensorDevId,rPreviewInfo);
        MINT32 frmRate = 1000000*10/(40000+rPreviewInfo.u4Eposuretime);
        MY_LOG("[%s] update flash on for precapture , frmRate(%d)", __FUNCTION__, frmRate);
        AAASensorMgr::getInstance().setPreviewMaxFrameRate(
            m_SensorDevId,
            frmRate,
            IAeMgr::getInstance().getSensorMode(m_SensorDevId));
    }
    else if(flashOnCnt == 2 && m_pStateMgr->getPreCapFlashOn() == MFALSE)
    {
        // frame 2 : set Flag for pass1 done to open flash, and restore frame rate.
        AE_MODE_CFG_T rPreviewInfo;
        IAeMgr::getInstance().getPreviewParams(m_SensorDevId,rPreviewInfo);
        MINT32 frmRate = 1000000*10/(rPreviewInfo.u4Eposuretime);
        AAASensorMgr::getInstance().setPreviewMaxFrameRate(
            m_SensorDevId,
            frmRate,
            IAeMgr::getInstance().getSensorMode(m_SensorDevId));
        if(cust_getFlashFrameRateDelay(m_SensorDevId) == 0)
        {
            // set flag for p1done to query.
            m_pStateMgr->updatePreCapFlashOn(MTRUE);
            MY_LOG("[%s] update flash on for precapture , MaxFrameRate(%d)", __FUNCTION__, frmRate);
        }
    }
    else if(flashOnCnt == 3 && m_pStateMgr->getPreCapFlashOn() == MFALSE)
    {
        if(cust_getFlashFrameRateDelay(m_SensorDevId) == 1)
        {
            // set flag for p1done to query.
            m_pStateMgr->updatePreCapFlashOn(MTRUE);
            MY_LOG("[%s] update flash on for precapture", __FUNCTION__);
        }
    }

    if(rep.isFlashCalibrationMode == 1)
        ILscMgr::getInstance(static_cast<ESensorDev_T>(m_SensorDevId))->setTsfOnOff(0);

    if(FlashMgr::getInstance().isFlashOnCalibration(m_SensorDevId) == 1)
        IAfMgr::getInstance().SetPauseAF(m_SensorDevId, MTRUE);

    IAeMgr::getInstance().updateAEBV(m_SensorDevId, reinterpret_cast<MVOID *>(pBuf->getPart(AAOSepBuf)));

    i4AoeCompLv = IAeMgr::getInstance().getAOECompLVvalue(m_SensorDevId, MFALSE);
    if(rep.isCurFlashOn == 0)
        IAwbMgr::getInstance().setStrobeMode(m_SensorDevId, AWB_STROBE_MODE_OFF);
    else
        IAwbMgr::getInstance().setStrobeMode(m_SensorDevId, AWB_STROBE_MODE_ON);

    IAwbMgr::getInstance().doPreCapAWB(m_SensorDevId, i4AoeCompLv, reinterpret_cast<MVOID *>(pBuf->getPart(AAOSepLSCBuf)));

    if(rep.isFlashCalibrationMode == 1)
        FlashMgr::getInstance().cctCaliFixAwb2(m_SensorDevId);


    if(rep.isEnd==1)
    {
        IAfMgr::getInstance().SetPauseAF(m_SensorDevId, MFALSE);

        CAM_LOGD_IF(m_i4EnableLog, "Flash-AE precapture metering process is done");

        if (isFlashAWBv2Enabled()) {
            passFlashAwbData.flashDuty = rep.nextDuty;
            passFlashAwbData.flashStep = rep.nextStep;
            passFlashAwbData.flashAwbWeight = rep.flashAwbWeight;
            IAwbMgr::getInstance().setFlashAWBData(m_SensorDevId, passFlashAwbData);
        }
        m_pStateMgr->proceedPrecapState();
    }
#else
    m_pStateMgr->proceedPrecapState();
#endif
    return err;

}

MRESULT
StatePrecapture::
sendPrecapIntent(intent2type<eIntent_VsyncUpdate>, state2type<ePrecap_AF_BeforeFlash>)
{
    CAM_LOGD_IF(m_i4EnableLog, "StatePrecapture::sendIntent(eIntent_VsyncUpdate, ePrecap_AF_BeforeFlash)");
    if(m_pStateMgr->getIsFlashOpened())
    {
        IAeMgr::getInstance().IsAEContinueShot(m_SensorDevId, MTRUE);
    }

    m_pStateMgr->proceedPrecapState();
    return S_3A_OK;
}


MRESULT
StatePrecapture::
sendPrecapIntent(intent2type<eIntent_VsyncUpdate>, state2type<ePrecap_AF_AfterFlash>)
{
    CAM_LOGD_IF(m_i4EnableLog, "StatePrecapture::sendIntent(eIntent_VsyncUpdate, ePrecap_AF_AfterFlash)");
    m_pStateMgr->proceedPrecapState();
    return S_3A_OK;
}
