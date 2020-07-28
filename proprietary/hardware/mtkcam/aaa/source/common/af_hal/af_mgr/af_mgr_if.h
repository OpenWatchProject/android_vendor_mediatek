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

#include <af_define.h>
#include <af_param.h>
#include <af_feature.h>
#include <dbg_aaa_param.h>
#include <utils/Vector.h>
#include <dbg_aaa_param.h>
#include <isp_config/af_config.h>
#include <private/aaa_hal_private.h>
#include <ae_param.h>


//define command interface is called by host or af_mgr internally.
#define AF_MGR_CALLER  0
#define AF_CMD_CALLER  1
#define AF_SYNC_CALLER 2


namespace NS3Av3
{

typedef MINT32 MRESULT;

typedef enum name
{
    E_AFNOTIFY_SW,
    E_AFNOTIFY_ABORT,
    E_AFNOTIFY_UNLOCKAE,

    E_AFNOTIFY_COUNT,
} E_AF_NOTIFY;

typedef enum
{
    E_AF_INACTIVE,
    E_AF_PASSIVE_SCAN,
    E_AF_PASSIVE_FOCUSED,
    E_AF_ACTIVE_SCAN,
    E_AF_FOCUSED_LOCKED,
    E_AF_NOT_FOCUSED_LOCKED,
    E_AF_PASSIVE_UNFOCUSED,
} E_AF_STATE_T;

typedef enum
{
    E_AF_LOCK_AE_REQ_NO_LOCK,
    E_AF_LOCK_AE_REQ_LOCK,
    E_AF_LOCK_AE_REQ_FORCE_LOCK,
} E_AF_LOCK_AE_REQ_T;

typedef struct GMVInfo_t
{
    MINT32 GMV_X;
    MINT32 GMV_Y;
    MINT32 GMV_Conf_X;
    MINT32 GMV_Conf_Y;
    MINT32 GMV_Max;
    GMVInfo_t()
        : GMV_X(0)
        , GMV_Y(0)
        , GMV_Conf_X(0)
        , GMV_Conf_Y(0)
        , GMV_Max(0)
    {}
} GMVInfo_T;

typedef struct ISPAEInfo_t
{
    MINT32  FrameNum;
    MBOOL   isAEStable;
    MBOOL   isAELock;
    MBOOL   isAEScenarioChange;
    MINT32  deltaIndex;
    MUINT32 afeGain;
    MUINT32 ispGain;
    MUINT32 exposuretime;
    MUINT32 realISOValue;
    MUINT32 aeFinerEVIdxBase;
    MUINT32 aeIdxCurrentF;
    MUINT32 aeCWValue;
    MINT32  lightValue_x10; // sceneLV
    MUINT32  aeBlockV[25];
    MINT32  isLVChangeTooMuch;
    ISPAEInfo_t()
        : FrameNum(-1)
        , isAEStable(0)
        , isAELock(0)
        , isAEScenarioChange(0)
        , deltaIndex(0)
        , afeGain(0)
        , ispGain(0)
        , exposuretime(0)
        , realISOValue(0)
        , aeFinerEVIdxBase(0)
        , aeIdxCurrentF(0)
        , aeCWValue(0)
        , lightValue_x10(0)
        , aeBlockV()
        , isLVChangeTooMuch(0)
    {}
} ISPAEInfo_T;

typedef struct AFInputData_t
{
    ISPAEInfo_T   ispAeInfo;
    MBOOL         isFlashFrm;
    GMVInfo_T     gmvInfo;
    MVOID*        ptrAFStt;

    AFInputData_t()
        : ispAeInfo()
        , isFlashFrm(0)
        , gmvInfo()
        , ptrAFStt(NULL)
    {}
} AFInputData_T;

typedef struct AFCommand_t
{
    MUINT32 requestNum;
    MUINT32 afMode;
    LIB3A_AF_MODE_T afLibMode;
    MFLOAT focusDistance;
    MINT32 mfPos;
    CameraFocusArea_T afRegions;
    eAFControl eAutofocus;
    eAFControl ePrecapture;
    MUINT32 triggerAF;
    MUINT8 pauseAF;

    MINT32 cropRegion_X;
    MINT32 cropRegion_Y;
    MINT32 cropRegion_W;
    MINT32 cropRegion_H;

    MINT32 isCaptureIntent;

    MVOID clear()
    {
        eAutofocus = AfCommand_Idle;
        ePrecapture = AfCommand_Idle;
        triggerAF = 0;
        pauseAF = 0;
        cropRegion_X = 0;
        cropRegion_Y = 0;
        cropRegion_W = 0;
        cropRegion_H = 0;
        isCaptureIntent = 0;
    }
} AFCommand_T;

typedef struct AFResult_t
{
    MINT32 resultNum;
    MINT32 afMode;
    MINT32 afRegions[7]; // type, num, X, Y, W, H, isFDAF
    MINT32 afSceneChange;
    MINT32 afState;
    MINT32 lensState;
    MFLOAT lensFocusDistance;
    MFLOAT lensFocusRange_near;
    MFLOAT lensFocusRange_far;
    E_AF_LOCK_AE_REQ_T  lockAERequest;
    MINT32 lensPosition;
    MBOOL  isFocusFinish;
    MBOOL  isFocused;
    MUINT64 focusValue;
    MINT32 gyroValue[3];
    FSC_FRM_INFO_T fscInfo;
} AFResult_T;

typedef struct AFStaticInfo_t
{
    MINT32  isAfSupport;
    MINT32  isAfThreadEnable;
    MINT32  isAfHwEnableByP1;
    MINT32  isCCUAF;
    MINT32  lastFocusPos;
    MVOID*  afTable;
    MINT32  afTableStepCount;
    MINT32  minAfTablePos;
    MINT32  maxAfTablePos;
    FSC_DAC_INIT_INFO_T fscInitInfo;
    MINT32  crpR1_Sel_1;
    DAF_TBL_STRUCT* dafTbl;
} AFStaticInfo_T;


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
    virtual ~IAfMgr() {};

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //    Operations.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static IAfMgr& getInstance(MINT32 sensorDev);
    //=====Init=====//
    virtual MINT32 init(MINT32 sensorIdx);
    virtual MINT32 camPwrOn();
    virtual MINT32 config(); // ISP5.0: Start
    virtual MINT32 start();  // ISP5.0: AFThreadStart
    //=====Uninit=====//
    virtual MINT32 stop();
    virtual MINT32 camPwrOff();
    virtual MINT32 uninit();
    //=====Process=====//
    virtual MINT32 process(AFInputData_T data, AFCommand_T command);
    //=====Control=====//
    // Get
    virtual MINT32 getResult(AFResult_T &result);
    virtual MINT32 getStaticInfo(AFStaticInfo_T &staticInfo, char const * caller);
    virtual MINT32 getDAFTbl(AFStaticInfo_T &staticInfo);
    virtual MINT32 getAfSupport();
    virtual MRESULT getDebugInfo( AF_DEBUG_INFO_T &rAFDebugInfo);
    // Set
    virtual MVOID   timeOutHandle();
    virtual MRESULT setOTFDInfo( MVOID* sInROIs, MINT32 i4Type);
    virtual MRESULT setNVRAMIndex(MUINT32 a_eNVRAMIndex);
    virtual MVOID   getHWCfgReg(AFResultConfig_T * const pResultConfig);
    // ACDK
    virtual MRESULT setAFMode( MINT32 a_eAFMode, MUINT32 u4Caller);

    // Notify
    virtual MVOID notify(E_AF_NOTIFY flag);
    ///// TOBE remove
    // Flash control
    virtual MRESULT setPauseAF( MBOOL bIsPause);
    virtual MRESULT setFullScanstep( MINT32 a_i4Step);

    // CCT feature
    virtual MRESULT CCTMCUNameinit( MINT32 i4SensorIdx);
    virtual MRESULT CCTMCUNameuninit();
    virtual MINT32 CCTOPAFOpeartion();
    virtual MINT32 CCTOPMFOpeartion(MINT32 a_i4MFpos);
    virtual MINT32 CCTOPAFGetAFInfo(MVOID *a_pAFInfo, MUINT32 *a_pOutLen);
    virtual MINT32 CCTOPAFGetBestPos(MINT32 *a_pAFBestPos, MUINT32 *a_pOutLen);
    virtual MINT32 CCTOPAFCaliOperation(MVOID *a_pAFCaliData, MUINT32 *a_pOutLen);
    virtual MINT32 CCTOPAFSetFocusRange(MVOID *a_pFocusRange);
    virtual MINT32 CCTOPAFGetFocusRange(MVOID *a_pFocusRange, MUINT32 *a_pOutLen);
    virtual MINT32 CCTOPAFGetNVRAMParam(MVOID *a_pAFNVRAM, MUINT32 *a_pOutLen);
    virtual MINT32 CCTOPAFApplyNVRAMParam(MVOID *a_pAFNVRAM, MUINT32 u4CamScenarioMode = 0);
    virtual MINT32 CCTOPAFSaveNVRAMParam();
    virtual MINT32 CCTOPAFGetFV(MVOID *a_pAFPosIn, MVOID *a_pAFValueOut, MUINT32 *a_pOutLen);
    virtual MINT32 CCTOPAFEnable();
    virtual MINT32 CCTOPAFDisable();
    virtual MINT32 CCTOPAFGetEnableInfo(MVOID *a_pEnableAF, MUINT32 *a_pOutLen);

    // AF Sync
    virtual MVOID  SyncAFReadDatabase();
    virtual MVOID  SyncAFWriteDatabase();
    // CCU ver. SyncAF process
    virtual MVOID  SyncAFGetSensorInfoForCCU(MINT32& slaveDevCCU, MINT32& slaveIdx);
    virtual MVOID  SyncAFProcess(MINT32 slaveDevCCU, MINT32 slaveIdx, MINT32 sync2AMode, AF_SyncInfo_T syncInfo); // CCU only
    // CPU ver. SyncAF process
    virtual MVOID  SyncAFSetMode( MINT32 a_i4SyncMode);
    virtual MVOID  SyncAFGetMotorRange( AF_SyncInfo_T& sCamInfo);
    virtual MINT32 SyncAFGetInfo( AF_SyncInfo_T& sCamInfo);
    virtual MVOID  SyncAFSetInfo( MINT32 a_i4Pos, AF_SyncInfo_T& sSlaveHisCamInfo);
    virtual MVOID  SyncAFGetCalibPos( AF_SyncInfo_T& sCamInfo);
    virtual MVOID  SyncAFCalibPos( AF_SyncInfo_T& sCamInfo);
    virtual MVOID  SyncSetOffMode();                // master/slave : set MF mose
    virtual MVOID  SyncSetMFPos(MINT32 targetPos);  // master       : move lens via mf control
    virtual MVOID  SyncMoveLens(MINT32 targetPos);  // slave        : move lens directly

};
};    //    namespace NS3Av3
#endif // _AE_MGR_N3D_H_

