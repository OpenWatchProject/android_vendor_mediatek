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
#ifndef _AF_MGR_IF_H_
#define _AF_MGR_IF_H_

#include <af_param.h>
#include <dbg_aaa_param.h>
#include <utils/Vector.h>
#include "mcu_drv.h"


namespace NS3Av3
{
/*******************************************************************************
*
*******************************************************************************/
class IAfMgr
{
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //    Ctor/Dtor.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////    Disallowed.
    //    Copy constructor is disallowed.
    IAfMgr(IAfMgr const&);
    //    Copy-assignment operator is disallowed.
    IAfMgr& operator=(IAfMgr const&);
public:  ////
    IAfMgr() {};
    ~IAfMgr() {};

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //    Operations.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static IAfMgr& getInstance();
    MRESULT init(MINT32 i4SensorDev, MINT32 i4SensorIdx,MINT32 isInitMCU=1);
    MRESULT uninit(MINT32 i4SensorDev,MINT32 isInitMCU=1);
    MRESULT SetPauseAF(MINT32 i4SensorDev, MBOOL bIsPause);
    MRESULT triggerAF(MINT32 i4SensorDev);
    MRESULT WaitTriggerAF(MINT32 i4SensorDev, MBOOL bWait);
    MRESULT setAFMode(MINT32 i4SensorDev,MINT32 a_eAFMode);
    MUINT8  getAFState(MINT32 i4SensorDev);
    MRESULT  getFocusAreaResult(MINT32 i4SensorDev, android::Vector<MINT32> &vecOutPos, android::Vector<MUINT8> &vecOutRes, MINT32 &i4OutSzW, MINT32 &i4OutSzH);
    MRESULT getFocusArea( MINT32 i4SensorDev, android::Vector<MINT32> &vecOut);
    MRESULT setAFArea(MINT32 i4SensorDev, CameraFocusArea_T a_sAFArea);
    MRESULT setCamScenarioMode(MINT32 i4SensorDev, MUINT32 a_eCamScenarioMode);
    MRESULT SetCropRegionInfo(MINT32 i4SensorDev,MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height);
    MRESULT setFDInfo(MINT32 i4SensorDev, MVOID* a_sFaces);
    MRESULT setOTInfo(MINT32 i4SensorDev, MVOID* a_sObtinfo);
    MINT32 getAFMaxAreaNum(MINT32 i4SensorDev);
    MINT32 isAFSupport(MINT32 i4SensorDev);
    MINT32 getMaxLensPos(MINT32 i4SensorDev);
    MINT32 getMinLensPos(MINT32 i4SensorDev);
    MINT32 getAFBestPos(MINT32 i4SensorDev);
    MINT32 getAFPos(MINT32 i4SensorDev);
    MINT32 getAFStable(MINT32 i4SensorDev);
    MINT32 getAFTableOffset(MINT32 i4SensorDev);
    MINT32 getAFTableMacroIdx(MINT32 i4SensorDev);
    MINT32 getAFTableIdxNum(MINT32 i4SensorDev);
    MVOID* getAFTable(MINT32 i4SensorDev);
    MVOID  getAF2AEInfo(MINT32 i4SensorDev, AF2AEInfo_T &rAFInfo);
    MINT64 MoveLensTo(MINT32 i4SensorDev, MINT32 &i4TargetPos);
    MRESULT setMFPos(MINT32 i4SensorDev, MINT32 a_i4Pos);
    MRESULT setFullScanstep(MINT32 i4SensorDev, MINT32 a_i4Step);
    AF_FULL_STAT_T getFLKStat(MINT32 i4SensorDev);
    MVOID  setAE2AFInfo(MINT32 i4SensorDev,AE2AFInfo_T rAEInfo);
    MVOID  setAdptCompInfo(MINT32 i4SensorDev, const AdptCompTimeData_T &AdptCompTime);
    MVOID  setIspSensorInfo2AF(MINT32 i4SensorDev, ISP_SENSOR_INFO_T ispSensorInfo);
    MRESULT SetAETargetMode(MINT32 i4SensorDev, MINT32 eAETargetMode);
    MVOID  getAFRefWin(MINT32 i4SensorDev, CameraArea_T &rWinSize);
    MRESULT doAF(MINT32 i4SensorDev, MVOID *pAFStatBuf);
    MRESULT passAFBuffer(MINT32 i4SensorDev, MVOID *pAFStatBuf);
    MVOID doSWPDE(MINT32 i4SensorDev, MVOID *iHalMetaData, MVOID *iImgbuf);
    MINT32 PDPureRawInterval(MINT32 i4SensorDev);
    MRESULT enablePBIN(MINT32 i4SensorDev, MBOOL bEnable);
    MRESULT setPdSeparateMode(MINT32 i4SensorDev, MUINT8 i4SMode);
    MRESULT getPdInfoForSttCtrl(MINT32 i4SensorDev, MINT32 i4SensorIdx, MINT32 i4SensorMode,
        MUINT32 &u4PDOSizeW, MUINT32 &u4PDOSizeH, FEATURE_PDAF_STATUS &PDAFStatus);
    MINT32 isFocusFinish(MINT32 i4SensorDev);
    MINT32 isFocused(MINT32 i4SensorDev);
    MINT32 isLockAE(MINT32 i4SensorDev);
    MRESULT getDebugInfo(MINT32 i4SensorDev, AF_DEBUG_INFO_T &rAFDebugInfo);
    MRESULT setBestShotConfig(MINT32 i4SensorDev);
    MRESULT calBestShotValue(MINT32 i4SensorDev, MVOID *pAFStatBuf);
    MINT64 getBestShotValue(MINT32 i4SensorDev);
    MVOID setSGGPGN(MINT32 i4SensorDev, MINT32 i4SGG_PGAIN);
    MVOID autoFocus(MINT32 i4SensorDev);
    MVOID cancelAutoFocus(MINT32 i4SensorDev);
    MVOID TimeOutHandle(MINT32 i4SensorDev);
    MRESULT setSensorMode(MINT32 i4SensorDev, MINT32 i4NewSensorMode, MUINT32 i4BINInfo_SzW, MUINT32 i4BINInfo_SzH);
    MVOID SetCurFrmNum(MINT32 i4SensorDev, MUINT32 u4FrmNum, MUINT32 u4FrmNumCur=0);
    MINT32 getDAFtbl(MINT32 i4SensorDev, MVOID ** ptbl);  /* Depth AF API */
    MINT32 setMultiZoneEnable( MINT32 i4SensorDev, MUINT8 bEn);
    MVOID updateSensorListenerParams(MINT32 i4SensorDev, MINT32 *i4SensorInfo);
    MINT32 sendAFCtrl(MINT32 i4SensorDev, MUINT32 eAFCtrl, MINTPTR iArg1, MINTPTR iArg2);
    //Camera 3.0
    MVOID   setAperture(MINT32 i4SensorDev,MFLOAT lens_aperture);
    MFLOAT  getAperture(MINT32 i4SensorDev);
    MVOID   setFocalLength(MINT32 i4SensorDev,MFLOAT lens_focalLength);
    MFLOAT  getFocalLength(MINT32 i4SensorDev);
    MVOID   setFocusDistance(MINT32 i4SensorDev,MFLOAT lens_focusDistance);
    MFLOAT  getFocusDistance(MINT32 i4SensorDev);
    MVOID   setOpticalStabilizationMode (MINT32 i4SensorDev,MINT32 ois_OnOff);
    MINT32  getOpticalStabilizationMode(MINT32 i4SensorDev);
    MVOID  getFocusRange(MINT32 i4SensorDev, MFLOAT *vnear, MFLOAT *vfar);
    MINT32  getLensState(MINT32 i4SensorDev);
    MBOOL Stop(MINT32 i4SensorDev);
    MBOOL Start(MINT32 i4SensorDev);
    MBOOL CamPwrOnState(MINT32 i4SensorDev);
    MBOOL CamPwrOffState(MINT32 i4SensorDev);
    MBOOL AFThreadStart(MINT32 i4SensorDev);
    MBOOL VsyncUpdate(MINT32 i4SensorDev);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //    CCT feature
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    MRESULT CCTMCUNameinit(MINT32 i4SensorDev, MINT32 i4SensorIdx);
    MRESULT CCTMCUNameuninit(MINT32 i4SensorDev);
    MINT32 CCTOPAFOpeartion(MINT32 i4SensorDev);
    MINT32 CCTOPCheckAutoFocusDone(MINT32 i4SensorDev);
    MINT32 CCTOPMFOpeartion(MINT32 i4SensorDev,MINT32 a_i4MFpos);
    MINT32 CCTOPAFGetAFInfo(MINT32 i4SensorDev,MVOID *a_pAFInfo, MUINT32 *a_pOutLen);
    MINT32 CCTOPAFGetBestPos(MINT32 i4SensorDev,MINT32 *a_pAFBestPos, MUINT32 *a_pOutLen);
    MINT32 CCTOPAFCaliOperation(MINT32 i4SensorDev,MVOID *a_pAFCaliData, MUINT32 *a_pOutLen);
    MINT32 CCTOPAFSetFocusRange(MINT32 i4SensorDev,MVOID *a_pFocusRange);
    MINT32 CCTOPAFGetFocusRange(MINT32 i4SensorDev,MVOID *a_pFocusRange, MUINT32 *a_pOutLen);
    MINT32 CCTOPAFGetNVRAMParam(MINT32 i4SensorDev,MVOID *a_pAFNVRAM, MUINT32 *a_pOutLen);
    MINT32 CCTOPAFApplyNVRAMParam(MINT32 i4SensorDev,MVOID *a_pAFNVRAM, MUINT32 u4CamScenarioMode = 0);
    MINT32 CCTOPAFSaveNVRAMParam(MINT32 i4SensorDev);
    MINT32 CCTOPAFGetFV(MINT32 i4SensorDev,MVOID *a_pAFPosIn, MVOID *a_pAFValueOut, MUINT32 *a_pOutLen);
    MINT32 CCTOPAFEnable(MINT32 i4SensorDev);
    MINT32 CCTOPAFDisable(MINT32 i4SensorDev);
    MINT32 CCTOPAFGetEnableInfo(MINT32 i4SensorDev,MVOID *a_pEnableAF, MUINT32 *a_pOutLen);
    MRESULT CCTOPAFSetAfArea(MINT32 i4SensorDev,MUINT32 a_iPercent);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //    AF Sync
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    MVOID SyncAFReadDatabase(MINT32 i4SensorDev);
    MVOID SyncAFWriteDatabase(MINT32 i4SensorDev);
    MVOID SyncAFSetMode(MINT32 i4SensorDev, MINT32 a_i4SyncMode);
    MVOID SyncAFGetMotorRange(MINT32 i4SensorDev, AF_SyncInfo_T& sCamInfo);
    MINT32 SyncAFGetInfo(MINT32 i4SensorDev, AF_SyncInfo_T& sCamInfo);
    MVOID SyncAFSetInfo(MINT32 i4SensorDev, MINT32 a_i4Pos, AF_SyncInfo_T& sSlaveHisCamInfo);
    MVOID SyncAFGetCalibPos(MINT32 i4SensorDev, AF_SyncInfo_T& sCamInfo);
    MVOID SyncAFCalibPos(MINT32 i4SensorDev, AF_SyncInfo_T& sCamInfo);
};
};    //    namespace NS3Av3
#endif // _AE_MGR_N3D_H_

