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
#define LOG_TAG "af_mgr_if"

#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG       (1)
#endif

#include <utils/threads.h>  // For Mutex::Autolock.
#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <kd_camera_feature.h>
#include <mtkcam/utils/std/Log.h>
//#include <faces.h>
//#include <private/aaa_hal_private.h>
#include <aaa_hal_if.h>
#include <camera_custom_nvram.h>
#include <af_param.h>
#include <pd_param.h>
#include <awb_param.h>
#include <ae_param.h>
#include <af_tuning_custom.h>
//#include <drv/isp_reg.h>
//
#include <flash_param.h>
#include <isp_tuning.h>
#include <isp_tuning_mgr.h>
#include <mcu_drv.h>


#include <af_feature.h>
class NvramDrvBase;
using namespace android;

#include <af_algo_if.h>
//#include <CamDefs.h>
#include "af_mgr_if.h"
#include "af_mgr.h"


using namespace NS3Av3;
static  IAfMgr singleton;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IAfMgr& IAfMgr::getInstance()
{
    return  singleton;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::init(MINT32 i4SensorDev, MINT32 i4SensorIdx, MINT32 isInitMCU)
{
    return AfMgr::getInstance(i4SensorDev).init(i4SensorIdx, isInitMCU);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::uninit(MINT32 i4SensorDev, MINT32 isInitMCU)
{
    return AfMgr::getInstance(i4SensorDev).uninit(isInitMCU);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::SetPauseAF(MINT32 i4SensorDev, MBOOL bIsPause)
{
    return AfMgr::getInstance(i4SensorDev).SetPauseAF(bIsPause);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::triggerAF(MINT32 i4SensorDev)
{
    return AfMgr::getInstance(i4SensorDev).triggerAF(AF_CMD_CALLER);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::WaitTriggerAF(MINT32 i4SensorDev, MBOOL bWait)
{
    return AfMgr::getInstance(i4SensorDev).WaitTriggerAF(bWait);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::setAFMode(MINT32 i4SensorDev,MINT32 a_eAFMode)
{
    return AfMgr::getInstance(i4SensorDev).setAFMode(a_eAFMode, AF_CMD_CALLER);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT  IAfMgr::getFocusAreaResult(MINT32 i4SensorDev, android::Vector<MINT32> &vecOutPos, android::Vector<MUINT8> &vecOutRes, MINT32 &i4OutSzW, MINT32 &i4OutSzH)
{
    return AfMgr::getInstance(i4SensorDev).getFocusAreaResult(vecOutPos, vecOutRes, i4OutSzW, i4OutSzH);

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::getFocusArea( MINT32 i4SensorDev, android::Vector<MINT32> &vecOut)
{
    return AfMgr::getInstance(i4SensorDev).getFocusArea(vecOut);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT8 IAfMgr::getAFState(MINT32 i4SensorDev)
{
    return AfMgr::getInstance(i4SensorDev).getAFState();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::setAFArea(MINT32 i4SensorDev, CameraFocusArea_T a_sAFArea)
{
    return AfMgr::getInstance(i4SensorDev).setAFArea( a_sAFArea);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::setCamScenarioMode(MINT32 i4SensorDev, MUINT32 a_eCamScenarioMode)
{
    return AfMgr::getInstance(i4SensorDev).setCamScenarioMode(a_eCamScenarioMode);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::SetCropRegionInfo(MINT32 i4SensorDev,MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height)
{
    return AfMgr::getInstance(i4SensorDev).SetCropRegionInfo(u4XOffset, u4YOffset, u4Width, u4Height, AF_CMD_CALLER);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::setFDInfo(MINT32 i4SensorDev, MVOID* a_sFaces)
{
    return AfMgr::getInstance(i4SensorDev).setFDInfo(a_sFaces);
}//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::setOTInfo(MINT32 i4SensorDev, MVOID* a_sObtinfo)
{
    return AfMgr::getInstance(i4SensorDev).setOTInfo(a_sObtinfo);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::getAFMaxAreaNum(MINT32 i4SensorDev)
{
    return AfMgr::getInstance(i4SensorDev).getAFMaxAreaNum();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::isAFSupport(MINT32 i4SensorDev)
{
    return AfMgr::getInstance(i4SensorDev).isAFSupport();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::getMaxLensPos(MINT32 i4SensorDev)
{
    return AfMgr::getInstance(i4SensorDev).getMaxLensPos();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::getMinLensPos(MINT32 i4SensorDev)
{
    return AfMgr::getInstance(i4SensorDev).getMinLensPos();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::getAFBestPos(MINT32 i4SensorDev)
{
    return AfMgr::getInstance(i4SensorDev).getAFBestPos();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::getAFPos(MINT32 i4SensorDev)
{
    return AfMgr::getInstance(i4SensorDev).getAFPos();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::getAFStable(MINT32 i4SensorDev)
{
    return AfMgr::getInstance(i4SensorDev).getAFStable();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::getAFTableOffset(MINT32 i4SensorDev)
{
    return AfMgr::getInstance(i4SensorDev).getAFTableOffset();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::getAFTableMacroIdx(MINT32 i4SensorDev)
{
    return AfMgr::getInstance(i4SensorDev).getAFTableMacroIdx();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::getAFTableIdxNum(MINT32 i4SensorDev)
{
    return AfMgr::getInstance(i4SensorDev).getAFTableIdxNum();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID* IAfMgr::getAFTable(MINT32 i4SensorDev)
{
    return AfMgr::getInstance(i4SensorDev).getAFTable();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID IAfMgr::getAF2AEInfo(MINT32 i4SensorDev, AF2AEInfo_T &rAFInfo)
{
    return AfMgr::getInstance(i4SensorDev).getAF2AEInfo(rAFInfo);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT64 IAfMgr::MoveLensTo(MINT32 i4SensorDev, MINT32 &i4TargetPos)
{
    return AfMgr::getInstance(i4SensorDev).MoveLensTo(i4TargetPos, AF_CMD_CALLER);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::setMFPos(MINT32 i4SensorDev, MINT32 a_i4Pos)
{
    return AfMgr::getInstance(i4SensorDev).setMFPos(a_i4Pos, AF_CMD_CALLER);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::setFullScanstep(MINT32 i4SensorDev, MINT32 a_i4Step)
{
    return AfMgr::getInstance(i4SensorDev).setFullScanstep(a_i4Step);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AF_FULL_STAT_T IAfMgr::getFLKStat(MINT32 i4SensorDev)
{
    return AfMgr::getInstance(i4SensorDev).getFLKStat();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MVOID IAfMgr::setAE2AFInfo(MINT32 i4SensorDev,AE2AFInfo_T rAEInfo)
{
    return AfMgr::getInstance(i4SensorDev).setAE2AFInfo(rAEInfo);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID IAfMgr::setAdptCompInfo(MINT32 i4SensorDev, const AdptCompTimeData_T &AdptCompTime)
{
    return AfMgr::getInstance(i4SensorDev).setAdptCompInfo(AdptCompTime);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID IAfMgr::setIspSensorInfo2AF(MINT32 i4SensorDev, ISP_SENSOR_INFO_T ispSensorInfo)
{
    return AfMgr::getInstance(i4SensorDev).setIspSensorInfo2AF(ispSensorInfo);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::SetAETargetMode(MINT32 i4SensorDev, MINT32 eAETargetMode)
{
    return AfMgr::getInstance(i4SensorDev).SetAETargetMode((eAETargetMODE)eAETargetMode);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID IAfMgr::getAFRefWin(MINT32 i4SensorDev, CameraArea_T &rWinSize)
{
    return AfMgr::getInstance(i4SensorDev).getAFRefWin(rWinSize);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::doAF(MINT32 i4SensorDev, MVOID *pAFStatBuf)
{
    return AfMgr::getInstance(i4SensorDev).doAF(pAFStatBuf);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::passAFBuffer(MINT32 i4SensorDev, MVOID *pAFStatBuf)
{
    return AfMgr::getInstance(i4SensorDev).passAFBuffer(pAFStatBuf);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::passPDBuffer(MINT32 i4SensorDev, MVOID *ptrInPDData, mcuMotorInfo *pLensInfo)
{
    return AfMgr::getInstance(i4SensorDev).passPDBuffer(ptrInPDData, pLensInfo);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID IAfMgr::doSWPDE(MINT32 i4SensorDev, MVOID *iHalMetaData, MVOID *iImgbuf)
{
    return AfMgr::getInstance(i4SensorDev).doSWPDE(iHalMetaData, iImgbuf);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::PDPureRawInterval(MINT32 i4SensorDev)
{
    return AfMgr::getInstance(i4SensorDev).PDPureRawInterval();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::enablePBIN(MINT32 i4SensorDev, MBOOL bEnable)
{
    return AfMgr::getInstance(i4SensorDev).enablePBIN(bEnable);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::setPdSeparateMode(MINT32 i4SensorDev, MUINT8 i4SMode)
{
    return AfMgr::getInstance(i4SensorDev).setPdSeparateMode(i4SMode);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::getPdInfoForSttCtrl(MINT32 i4SensorDev, MINT32 i4SensorIdx, MINT32 i4SensorMode,
        MUINT32 &u4PDOSizeW, MUINT32 &u4PDOSizeH, FEATURE_PDAF_STATUS &PDAFStatus, const ConfigInfo_T& rConfigInfo)
{
    return AfMgr::getInstance(i4SensorDev).getPdInfoForSttCtrl(i4SensorIdx, i4SensorMode, u4PDOSizeW, u4PDOSizeH, PDAFStatus, rConfigInfo);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::isFocusFinish(MINT32 i4SensorDev)
{
    return AfMgr::getInstance(i4SensorDev).isFocusFinish();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::isFocused(MINT32 i4SensorDev)
{
    return AfMgr::getInstance(i4SensorDev).isFocused();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::isLockAE(MINT32 i4SensorDev)
{
    return AfMgr::getInstance(i4SensorDev).isLockAE();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::getDebugInfo(MINT32 i4SensorDev, AF_DEBUG_INFO_T &rAFDebugInfo)
{
    return AfMgr::getInstance(i4SensorDev).getDebugInfo(rAFDebugInfo);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::setBestShotConfig(MINT32 i4SensorDev)
{
    return AfMgr::getInstance(i4SensorDev).setBestShotConfig();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::calBestShotValue(MINT32 i4SensorDev, MVOID *pAFStatBuf)
{
    return AfMgr::getInstance(i4SensorDev).calBestShotValue(pAFStatBuf);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT64 IAfMgr::getBestShotValue(MINT32 i4SensorDev)
{
    return AfMgr::getInstance(i4SensorDev).getBestShotValue();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID IAfMgr::setSGGPGN(MINT32 i4SensorDev, MINT32 i4SGG_PGAIN)
{
    AfMgr::getInstance(i4SensorDev).setSGGPGN(i4SGG_PGAIN);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID IAfMgr::autoFocus(MINT32 i4SensorDev)
{
    AfMgr::getInstance(i4SensorDev).autoFocus();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID IAfMgr::cancelAutoFocus(MINT32 i4SensorDev)
{
    AfMgr::getInstance(i4SensorDev).cancelAutoFocus();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID IAfMgr::TimeOutHandle(MINT32 i4SensorDev)
{
    AfMgr::getInstance(i4SensorDev).TimeOutHandle();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::setSensorMode(MINT32 i4SensorDev, MINT32 i4NewSensorMode, MUINT32 i4BINInfo_SzW, MUINT32 i4BINInfo_SzH)
{
    MINT32 ret = S_AE_OK;

    ret = AfMgr::getInstance(i4SensorDev).setSensorMode(i4NewSensorMode, i4BINInfo_SzW, i4BINInfo_SzH);

    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID IAfMgr::SetCurFrmNum(MINT32 i4SensorDev, MUINT32 u4FrmNum, MUINT32 u4FrmNumCur)
{
    AfMgr::getInstance(i4SensorDev).SetCurFrmNum(u4FrmNum, u4FrmNumCur);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::getDAFtbl(MINT32 i4SensorDev, MVOID ** ptbl)
{
    return AfMgr::getInstance(i4SensorDev).getDAFtbl(ptbl);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::setMultiZoneEnable( MINT32 i4SensorDev, MUINT8 bEn)
{
    return AfMgr::getInstance(i4SensorDev).setMultiZoneEnable( bEn);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID IAfMgr::updateSensorListenerParams(MINT32 i4SensorDev, MINT32 *i4SensorInfo)
{
    AfMgr::getInstance(i4SensorDev).updateSensorListenerParams(i4SensorInfo);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IAfMgr::sendAFCtrl(MINT32 i4SensorDev, MUINT32 eAFCtrl, MINTPTR iArg1, MINTPTR iArg2)
{
    return AfMgr::getInstance(i4SensorDev).sendAFCtrl(eAFCtrl, iArg1, iArg2);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//Camera 3.0
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID IAfMgr::setAperture(MINT32 i4SensorDev, MFLOAT lens_aperture)
{
    AfMgr::getInstance(i4SensorDev).setAperture(lens_aperture);
}

MFLOAT IAfMgr::getAperture(MINT32 i4SensorDev)
{
    return AfMgr::getInstance(i4SensorDev).getAperture();
}
MVOID IAfMgr::setFocalLength(MINT32 i4SensorDev, MFLOAT lens_focalLength)
{
    AfMgr::getInstance(i4SensorDev).setFocalLength(lens_focalLength);
}

MFLOAT IAfMgr::getFocalLength(MINT32 i4SensorDev)
{
    return AfMgr::getInstance(i4SensorDev).getFocalLength();
}

MVOID IAfMgr::setFocusDistance(MINT32 i4SensorDev, MFLOAT lens_focusDistance)
{
    AfMgr::getInstance(i4SensorDev).setFocusDistance(lens_focusDistance);
}

MFLOAT IAfMgr::getFocusDistance(MINT32 i4SensorDev)
{
    return AfMgr::getInstance(i4SensorDev).getFocusDistance();
}

MVOID IAfMgr::setOpticalStabilizationMode(MINT32 i4SensorDev, MINT32 ois_OnOff)
{
    AfMgr::getInstance(i4SensorDev).setOpticalStabilizationMode(ois_OnOff);
}

MINT32 IAfMgr::getOpticalStabilizationMode(MINT32 i4SensorDev)
{
    return AfMgr::getInstance(i4SensorDev).getOpticalStabilizationMode();
}

MVOID IAfMgr::getFocusRange(MINT32 i4SensorDev, MFLOAT *vnear, MFLOAT *vfar)
{
    AfMgr::getInstance(i4SensorDev).getFocusRange(vnear,vfar);
}
MINT32 IAfMgr::getLensState(MINT32 i4SensorDev)
{
    return AfMgr::getInstance(i4SensorDev).getLensState();
}

MBOOL IAfMgr::Stop(MINT32 i4SensorDev)
{
    return AfMgr::getInstance(i4SensorDev).Stop();
}


MBOOL IAfMgr::Start(MINT32 i4SensorDev)
{
    return AfMgr::getInstance(i4SensorDev).Start();
}

MBOOL IAfMgr::CamPwrOnState(MINT32 i4SensorDev)
{
    return AfMgr::getInstance(i4SensorDev).CamPwrOnState();
}

MBOOL IAfMgr::CamPwrOffState(MINT32 i4SensorDev)
{
    return AfMgr::getInstance(i4SensorDev).CamPwrOffState();
}

MBOOL IAfMgr::AFThreadStart(MINT32 i4SensorDev)
{
    return AfMgr::getInstance(i4SensorDev).AFThreadStart();
}

MBOOL IAfMgr::VsyncUpdate(MINT32 i4SensorDev)
{
    return AfMgr::getInstance(i4SensorDev).VsyncUpdate();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    CCT feature
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::CCTMCUNameinit(MINT32 i4SensorDev, MINT32 i4SensorIdx)
{
    return AfMgr::getInstance(i4SensorDev).CCTMCUNameinit(i4SensorIdx);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IAfMgr::CCTMCUNameuninit(MINT32 i4SensorDev)
{
    return AfMgr::getInstance(i4SensorDev).CCTMCUNameuninit();
}
MINT32 IAfMgr::CCTOPAFOpeartion(MINT32 i4SensorDev)
{
    return AfMgr::getInstance(i4SensorDev).CCTOPAFOpeartion();
}
MINT32 IAfMgr::CCTOPMFOpeartion(MINT32 i4SensorDev,MINT32 a_i4MFpos)
{
    return AfMgr::getInstance(i4SensorDev).CCTOPMFOpeartion(a_i4MFpos);
}
MINT32 IAfMgr::CCTOPAFGetAFInfo(MINT32 i4SensorDev,MVOID *a_pAFInfo, MUINT32 *a_pOutLen)
{
    return AfMgr::getInstance(i4SensorDev).CCTOPAFGetAFInfo(a_pAFInfo,a_pOutLen);
}
MINT32 IAfMgr::CCTOPAFGetBestPos(MINT32 i4SensorDev,MINT32 *a_pAFBestPos, MUINT32 *a_pOutLen)
{
    return AfMgr::getInstance(i4SensorDev).CCTOPAFGetBestPos(a_pAFBestPos,a_pOutLen);
}
MINT32 IAfMgr::CCTOPAFCaliOperation(MINT32 i4SensorDev,MVOID *a_pAFCaliData, MUINT32 *a_pOutLen)
{
    return AfMgr::getInstance(i4SensorDev).CCTOPAFCaliOperation(a_pAFCaliData,a_pOutLen);
}
MINT32 IAfMgr::CCTOPAFSetFocusRange(MINT32 i4SensorDev,MVOID *a_pFocusRange)
{
    return AfMgr::getInstance(i4SensorDev).CCTOPAFSetFocusRange(a_pFocusRange);
}
MINT32 IAfMgr::CCTOPCheckAutoFocusDone(MINT32 i4SensorDev)
{
    return AfMgr::getInstance(i4SensorDev).CCTOPCheckAutoFocusDone();
}
MINT32 IAfMgr::CCTOPAFGetFocusRange(MINT32 i4SensorDev,MVOID *a_pFocusRange, MUINT32 *a_pOutLen)
{
    return AfMgr::getInstance(i4SensorDev).CCTOPAFGetFocusRange(a_pFocusRange,a_pOutLen);
}
MINT32 IAfMgr::CCTOPAFGetNVRAMParam(MINT32 i4SensorDev,MVOID *a_pAFNVRAM, MUINT32 *a_pOutLen)
{
    return AfMgr::getInstance(i4SensorDev).CCTOPAFGetNVRAMParam(a_pAFNVRAM,a_pOutLen);
}
MINT32 IAfMgr::CCTOPAFApplyNVRAMParam(MINT32 i4SensorDev,MVOID *a_pAFNVRAM, MUINT32 u4CamScenarioMode)
{
    return AfMgr::getInstance(i4SensorDev).CCTOPAFApplyNVRAMParam(a_pAFNVRAM, u4CamScenarioMode);
}
MINT32 IAfMgr::CCTOPAFSaveNVRAMParam(MINT32 i4SensorDev)
{
    return AfMgr::getInstance(i4SensorDev).CCTOPAFSaveNVRAMParam();
}
MINT32 IAfMgr::CCTOPAFGetFV(MINT32 i4SensorDev,MVOID *a_pAFPosIn, MVOID *a_pAFValueOut, MUINT32 *a_pOutLen)
{
    return AfMgr::getInstance(i4SensorDev).CCTOPAFGetFV(a_pAFPosIn,a_pAFValueOut,a_pOutLen);
}
MINT32 IAfMgr::CCTOPAFEnable(MINT32 i4SensorDev)
{
    return AfMgr::getInstance(i4SensorDev).CCTOPAFEnable();
}
MINT32 IAfMgr::CCTOPAFDisable(MINT32 i4SensorDev)
{
    return AfMgr::getInstance(i4SensorDev).CCTOPAFDisable();
}
MINT32 IAfMgr::CCTOPAFGetEnableInfo(MINT32 i4SensorDev,MVOID *a_pEnableAF, MUINT32 *a_pOutLen)
{
    return AfMgr::getInstance(i4SensorDev).CCTOPAFGetEnableInfo(a_pEnableAF,a_pOutLen);
}
MRESULT IAfMgr::CCTOPAFSetAfArea(MINT32 i4SensorDev,MUINT32 a_iPercent)
{
    return AfMgr::getInstance(i4SensorDev).CCTOPAFSetAfArea(a_iPercent);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    AF Sync
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID IAfMgr::SyncAFReadDatabase(MINT32 i4SensorDev)
{
    return AfMgr::getInstance(i4SensorDev).SyncAFReadDatabase();
}
MVOID IAfMgr::SyncAFWriteDatabase(MINT32 i4SensorDev)
{
    return AfMgr::getInstance(i4SensorDev).SyncAFWriteDatabase();
}
MVOID IAfMgr::SyncAFSetMode(MINT32 i4SensorDev, MINT32 a_i4SyncMode)
{
    AfMgr::getInstance(i4SensorDev).SyncAFSetMode(a_i4SyncMode);
}
MVOID IAfMgr::SyncAFGetMotorRange(MINT32 i4SensorDev, AF_SyncInfo_T& sCamInfo)
{
    AfMgr::getInstance(i4SensorDev).SyncAFGetMotorRange(sCamInfo);
}
MINT32 IAfMgr::SyncAFGetInfo(MINT32 i4SensorDev, AF_SyncInfo_T& sCamInfo)
{
    return AfMgr::getInstance(i4SensorDev).SyncAFGetInfo(sCamInfo);
}
MVOID IAfMgr::SyncAFSetInfo(MINT32 i4SensorDev, MINT32 a_i4Pos, AF_SyncInfo_T& sSlaveHisCamInfo)
{
    return AfMgr::getInstance(i4SensorDev).SyncAFSetInfo(a_i4Pos, sSlaveHisCamInfo);
}
MVOID IAfMgr::SyncAFGetCalibPos(MINT32 i4SensorDev, AF_SyncInfo_T& sCamInfo)
{
    AfMgr::getInstance(i4SensorDev).SyncAFGetCalibPos(sCamInfo);
}
MVOID IAfMgr::SyncAFCalibPos(MINT32 i4SensorDev, AF_SyncInfo_T& sCamInfo)
{
    AfMgr::getInstance(i4SensorDev).SyncAFCalibPos(sCamInfo);
}
