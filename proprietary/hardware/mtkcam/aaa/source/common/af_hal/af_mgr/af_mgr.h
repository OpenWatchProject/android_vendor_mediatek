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
/**
 * @file af_mgr.h
 * @brief AF manager, do focusing for raw sensor.
 */
#ifndef _AF_MGR_H_
#define _AF_MGR_H_
extern "C" {
#include <af_algo_if.h>
}
#include <queue>
#include <deque>
#include <isp_tuning.h>
#include <af_feature.h>
#include <af_param.h>
#include <private/IopipeUtils.h>
#include <aaa_hal_if.h>
#include <pd_buf_common.h>
#include <laser_mgr.h>
#include <vector>
#include <mcu_drv.h>
#include <mtkcam/drv/mem/cam_cal_drv.h>
#include <dbg_aaa_param.h>
#include <utils/Vector.h>
#include <af_flow_custom.h>
#include <mtkcam/aaa/aaa_hal_common.h>
#include "af_mgr_if.h"
#include "af_cxu/af_cxu_if.h"

class NvramDrvBase;
using namespace android;
using namespace NSIoPipe;
using namespace NSCamIOPipe;
using namespace std;

namespace NSCam
{
namespace Utils
{
class SensorProvider;
}
}

namespace NS3Av3
{

#define AF_START_MAGIC_NUMBER 4

// if AP WOULD NOT show FDROI automatically while detecting faces, set this macro to 1
#define AP_NOT_SHOW_FDROI_AUTO 0

//--- pre-tuned scenechange parameters start ---
#define SENSOR_ACCE_SCALE 100
#define SENSOR_GYRO_SCALE 100

//--- define AF EXIF size for af_mgr only ---
#define MGR_EXIF_SIZE (3+31)
#define MGR_CAPTURE_EXIF_SIZE 40

/* This struct is uesed to convert DMA buffer */
typedef struct
{
    MUINT32 byte_00_03;
    MUINT32 byte_04_07;
    MUINT32 byte_08_11;
    MUINT32 byte_12_15;
    MUINT32 byte_16_19; /* extend mode */
    MUINT32 byte_20_23;
    MUINT32 byte_24_27;
    MUINT32 byte_28_31;
} AF_HW_STAT_T;


/* Focus distance information. */
typedef struct AF_FOCUS_DIS_t
{
    MINT32 i4LensPos;
    MFLOAT fDist;

    AF_FOCUS_DIS_t()
    {
        i4LensPos = 0;
        fDist     = 0.33;
    }
} AF_FOCUS_DIS_T;

/* Focus distance information. */
typedef enum
{
    /**********************************************************
     * Please keep eIDX_ROI_ARRAY_CENTER as the first element *
     **********************************************************/
    eIDX_ROI_ARRAY_CENTER = 0, /* It is always valid for calculation center roi once crop region is set.*/
    eIDX_ROI_ARRAY_AP,         /* ap roi is calculated during af mode is configured as auto mode.*/
    eIDX_ROI_ARRAY_FD,         /* fd roi is calculated once a FD region is set*/
    eIDX_ROI_ARRAY_NUM,
} eIDX_ROI_ARRAY;

/* ROIs for PD calculation. */
typedef struct PD_CALCULATION_ROI_t
{
    MBOOL            valid;
    AFPD_BLOCK_ROI_T info;

    PD_CALCULATION_ROI_t() :
        valid(MFALSE)
    {
        memset(&info, 0, sizeof(AFPD_BLOCK_ROI_T));
    }
} PD_CALCULATION_ROI_T;

typedef struct ISPAEINFO_QUEUE_t
{
    const static MUINT32 queueSize = 64;
    ISPAEInfo_T content[queueSize];
    MUINT32 head = 0;
    MUINT32 tail = 0;

    VOID clear()
    {
        head = tail = 0;
    }

    MBOOL empty()
    {
        return (head == tail) ? MTRUE : MFALSE;
    }

    MBOOL full()
    {
        return ((head + 1) % queueSize == tail) ? MTRUE : MFALSE;
    }

    MBOOL pushHead(ISPAEInfo_T input)
    {
        if (full())
            return MFALSE;

        head = (head + 1) % queueSize;
        content[head] = input;

        return MTRUE;
    }

    MBOOL popTail(ISPAEInfo_T& output)
    {
        if (empty())
            return MFALSE;

        output = content[tail];
        tail = (tail + 1) % queueSize;;

        return MTRUE;
    }

    VOID popTail()
    {
        MUINT32 newTail = (tail + 1) % queueSize;
        tail = newTail;
    }

    VOID pushHeadAnyway(ISPAEInfo_T input)
    {
        if (full())
            popTail();

        head = (head + 1) % queueSize;
        content[head] = input;
    }
} ISPAEINFO_QUEUE_T;

template<typename T, typename Container = std::deque<T> >
class iterable_queue : public std::queue<T, Container>
{
public:
    typedef typename Container::iterator iterator;
    typedef typename Container::const_iterator const_iterator;

    iterator begin() { return this->c.begin(); }
    iterator end() { return this->c.end(); }
    const_iterator begin() const { return this->c.begin(); }
    const_iterator end() const { return this->c.end(); }
};

typedef enum
{
    EVENT_NONE                       = 0x0,
    EVENT_CMD_CHANGE_MODE            = 0x1,
    EVENT_CMD_AUTOFOCUS              = 0x1 << 1,
    EVENT_CMD_TRIGGERAF_WITH_AE_STBL = 0x1 << 2,
    EVENT_CMD_AUTOFOCUS_CANCEL       = 0x1 << 3,
    EVENT_CMD_SET_AF_REGION          = 0x1 << 4,
    EVENT_CMD_STOP                   = 0x1 << 5,
    EVENT_SEARCHING_START            = 0x1 << 6,
    EVENT_SEARCHING_END              = 0x1 << 7,
    EVENT_AE_IS_STABLE               = 0x1 << 8,
    EVENT_SET_WAIT_FORCE_TRIGGER     = 0x1 << 9,
    EVENT_CANCEL_WAIT_FORCE_TRIGGER  = 0x1 << 10,
    EVENT_CMD_START                  = 0x1 << 11,
    EVENT_SEARCHING_DONE_RESET_PARA  = 0x1 << 12
} AF_EVENT_T;

typedef enum
{
    EM_AF_FLAG_NONE             = 0x0,
    EM_AF_FLAG_FULLSCAN_NORMAL  = 0x1,
    EM_AF_FLAG_FULLSCAN_ADVANCE = 0x1 << 1,
    EM_AF_FLAG_TEMP_CALI        = 0x1 << 2
} AF_EM_FLAG_T;

// TAF flow receive autofocus and cancelAutoFocus.
// Don't do AF seraching when changing mode from auto mode to continuous mode.
typedef enum
{
    TAF_STATUS_RESET                   = 0x0,
    TAF_STATUS_RECEIVE_AUTOFOCUS       = 0x1,
    TAF_STATUS_RECEIVE_CANCELAUTOFOCUS = 0x1 << 1,
} AF_TAF_STATUS_T;

typedef enum
{
    E_LAUNCH_AF_IDLE      = -1,  // Default State
    E_LAUNCH_AF_WAITING   =  0,  // AE/PD + Face(Timeout)
    E_LAUNCH_AF_TRIGGERED =  1,  // LaunchCamTriggered
    E_LAUNCH_AF_DONE      =  2,  // AFSearch for LaunchCamTrigger Done
} E_LAUNCH_AF_STATE_T;



typedef enum
{
    E_RTV_AF_INVALID = 0,
    E_RTV_AF_CTRL_MODE,
    E_RTV_AF_CTRL_STATE,
    E_RTV_AF_ROI_TYPE,
    E_RTV_AF_SEARCH_STATE,
    E_RTV_AF_SEARCH_TYPE,
    E_RTV_AF_ALGO_MODE,
    E_RTV_AF_ALGO_STATE,
    E_RTV_AF_LENS_POS,
    E_AF_INDEX_RSV,
    E_RTV_AF_RSV_1 = E_AF_INDEX_RSV,
    E_RTV_AF_RSV_2,
    E_RTV_AF_RSV_3,
    E_RTV_AF_RSV_4,
    E_RTV_AF_RSV_5,
    E_RTV_AF_RSV_6,
    E_RTV_AF_RSV_7,
    E_RTV_AF_RSV_8,

    E_AF_INDEX_MAX, // the total number of possible rtv info.
} RTV_AF_INDEX;

typedef struct AFIrregularCommand_t
{
    FD_INFO_T FDROI;
    MUINT32 curNvramIndex;
    MUINT32 preNvramIndex;
    AFIrregularCommand_t()
    {
        curNvramIndex = AF_CAM_SCENARIO_NUM_2;
        preNvramIndex = AF_CAM_SCENARIO_NUM_2;
        memset(&FDROI, 0, sizeof(FD_INFO_T));
    }
} AFIrregularCommand_T;

/**
 * @brief AF manager class
 */
class AfMgr : public IAfMgr
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    /*  Copy constructor is disallowed. */
    AfMgr( AfMgr const&);

    /*  Copy-assignment operator is disallowed. */
    AfMgr& operator=( AfMgr const&);

public:
    AfMgr( MINT32 eSensorDev);
    virtual ~AfMgr();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static AfMgr& getInstance(MINT32 sensorDev);
    //=====Init=====//
    MINT32 init(MINT32 sensorIdx);
    MINT32 camPwrOn();
    MINT32 config(); // ISP5.0: Start
    MINT32 start();  // ISP5.0: AFThreadStart
    //=====Uninit=====//
    MINT32 stop();
    MINT32 camPwrOff();
    MINT32 uninit();
    //=====Process=====//
    MINT32 process(AFInputData_T data, AFCommand_T command);

    //=====Control=====//
    // Get
    MINT32 getStaticInfo(AFStaticInfo_T &staticInfo, char const * caller);
    MINT32 getResult(AFResult_T &result);
    MVOID  getHWCfgReg(AFResultConfig_T * const pResultConfig);
    MINT32 getDAFTbl(AFStaticInfo_T &staticInfo);
    MRESULT getDebugInfo( AF_DEBUG_INFO_T &rAFDebugInfo);

    // Set
    MVOID   timeOutHandle();
    MRESULT setOTFDInfo( MVOID* sInROIs, MINT32 i4Type);
    MVOID notify(E_AF_NOTIFY flag);
    MRESULT setNVRAMIndex(MUINT32 a_eNVRAMIndex);

    // ACDK
    MRESULT setAFMode( MINT32 a_eAFMode, MUINT32 u4Caller);
    MRESULT setFullScanstep( MINT32 a_i4Step);
    MRESULT setPauseAF( MBOOL bIsPause);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //                             CCT feature
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    MRESULT CCTMCUNameinit( MINT32 i4SensorIdx);
    MRESULT CCTMCUNameuninit();
    MINT32  CCTOPAFOpeartion();
    MINT32  CCTOPCheckAutoFocusDone();
    MINT32  CCTOPWaitAutoFocusDone();
    MINT32  CCTOPMFOpeartion( MINT32 a_i4MFpos);
    MINT32  CCTOPAFGetAFInfo( MVOID *a_pAFInfo, MUINT32 *a_pOutLen);
    MINT32  CCTOPAFGetBestPos( MINT32 *a_pAFBestPos, MUINT32 *a_pOutLen);
    MINT32  CCTOPAFCaliOperation( MVOID *a_pAFCaliData, MUINT32 *a_pOutLen);
    MINT32  CCTOPAFSetFocusRange( MVOID *a_pFocusRange);
    MINT32  CCTOPAFGetFocusRange( MVOID *a_pFocusRange, MUINT32 *a_pOutLen);
    MINT32  CCTOPAFGetNVRAMParam( MVOID *a_pAFNVRAM, MUINT32 *a_pOutLen);
    MINT32  CCTOPAFApplyNVRAMParam( MVOID *a_pAFNVRAM, MUINT32 u4CamScenarioMode);
    MINT32  CCTOPAFSaveNVRAMParam();
    MINT32  CCTOPAFGetFV( MVOID *a_pAFPosIn, MVOID *a_pAFValueOut, MUINT32 *a_pOutLen);
    MINT32  CCTOPAFEnable();
    MINT32  CCTOPAFDisable();
    MINT32  CCTOPAFGetEnableInfo( MVOID *a_pEnableAF, MUINT32 *a_pOutLen);
    MRESULT CCTOPAFSetAfArea(MUINT32 a_iPercent);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //                             Sync AF feature
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    MVOID  SyncAFReadDatabase();
    MVOID  SyncAFWriteDatabase();
    MVOID  SyncAFGetSensorInfoForCCU(MINT32& slaveDevCCU, MINT32& slaveIdx);
    MVOID  SyncAFProcess(MINT32 slaveDevCCU, MINT32 slaveIdx, MINT32 sync2AMode, AF_SyncInfo_T syncInfo);
    MVOID  SyncAFSetMode( MINT32 a_i4SyncMode);
    MVOID  SyncAFGetMotorRange(AF_SyncInfo_T& sCamInfo);
    MINT32 SyncAFGetInfo(AF_SyncInfo_T& sCamInfo);
    MVOID  SyncAFSetInfo( MINT32 a_i4Pos, AF_SyncInfo_T& sSlaveHisCamInfo);
    MVOID  SyncAFGetCalibPos(AF_SyncInfo_T& sCamInfo);
    MVOID  SyncAFCalibPos(AF_SyncInfo_T& sCamInfo);
    MVOID  SyncSetOffMode();                // master/slave : set MF mose
    MVOID  SyncSetMFPos(MINT32 targetPos);  // master       : move lens via mf control
    MVOID  SyncMoveLens(MINT32 targetPos);  // slave        : move lens directly


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                           Private functions
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:  // af_mgr_private.cpp
    MVOID checkStaticProperty(AF_DBG_S_PROPERTY_T& property);
    MVOID checkDynamicProperty(AF_DBG_D_PROPERTY_T& property);
    MVOID resetAFParams();
    MRESULT setCropRegionInfo( MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height, MUINT32 u4Caller);
    MRESULT setAFArea( CameraFocusArea_T &sInAFArea);
    MVOID setIspAEInfo(ISPAEInfo_T ispAeInfo);
    MINT32 setMultiZoneEnable( MUINT8 bEn __unused)
    {
        return 0;
    }
    MINT32 process_doStt(MUINT32 reqNumber, MVOID* prtAfStt);
    MINT32 process_doCommand(AFCommand_T& command);
    MINT32 process_doData(AFInputData_T& data);
    MINT32 process_doAF();
    MINT32 process_doOutput();
    MRESULT updateNVRAM(MUINT32 a_eNVRAMIndex);
    MUINT64 MoveLensTo( MINT32 &i4TargetPos, MUINT32 u4Caller);
    MVOID   setFocusDistance( MFLOAT lens_focusDistance);
    MRESULT checkPrecapture( eAFControl ePrecap);
    MRESULT triggerAF( MUINT32 u4Caller);
    MRESULT checkAutoFocus(eAFControl eAF);
    MVOID autoFocus();
    MVOID cancelAutoFocus();
    MVOID setAdptCompInfo( MUINT64 TS_SOF);
    ISPAEInfo_T* getMatchedISPAEInfoFromFrameId(MINT32 frameId);
    MUINT32 getSensorTemperature();
    MBOOL sendAFNormalPipe( MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3);
    MRESULT readOTP(CAMERA_CAM_CAL_TYPE_ENUM enCamCalEnum);
    MVOID ConvertDMABufToStat( MINT32 &i4CurPos, MVOID *ptrInStatBuf, AF_STAT_PROFILE_T &sOutSata);
    MVOID HybridAFPreprocessing();
    MUINT32 GetHybridAFMode();
    MVOID PrintHWRegSetting( AF_CONFIG_T &sAFHWCfg);
    AREA_T& SelROIToFocusing( AF_OUTPUT_T &sInAFInfo);
    MUINT64 getTimeStamp_us();
    MRESULT WDBGInfo( MUINT32 i4InTag, MUINT32 i4InVal, MUINT32 i4InLineKeep);
    MRESULT WDBGCapInfo( MUINT32 i4InTag, MUINT32 i4InVal, MUINT32 i4InLineKeep);
    MRESULT WDBGTSInfo( MUINT32 i4InTag, MUINT32 i4InVal, MUINT32 i4InLineKeep);
    MRESULT SetMgrDbgInfo();
    MRESULT SetMgrCapDbgInfo();
    MRESULT CleanMgrDbgInfo();
    MRESULT CleanMgrCapDbgInfo();
    MRESULT GetMgrDbgInfo( AF_DEBUG_INFO_T &sOutMgrDebugInfo);
    E_AF_STATE_T UpdateStateMFMode( E_AF_STATE_T &eInCurSate, AF_EVENT_T &sInEvent);
    E_AF_STATE_T UpdateStateOFFMode( E_AF_STATE_T &eInCurSate, AF_EVENT_T &sInEvent);
    E_AF_STATE_T UpdateStateContinuousMode( E_AF_STATE_T &eInCurSate, AF_EVENT_T &sInEvent);
    E_AF_STATE_T UpdateStateAutoMode( E_AF_STATE_T &eInCurSate, AF_EVENT_T &sInEvent);
    E_AF_STATE_T UpdateState( AF_EVENT_T sInEvent);
    MVOID UpdateCenterROI( AREA_T &sOutAreaCenter); // To apply the zoom effect to AFROI.
    MBOOL LockAlgo(MUINT32 u4Caller = 0);
    MBOOL UnlockAlgo(MUINT32 u4Caller = 0);
    MBOOL isSkipHandleAF();
    MVOID IspMgrAFGetROIFromHw(AREA_T &Area, MUINT32 &isTwin);
    MVOID* getAFTable();

    MRESULT setMFPos( MINT32 a_i4Pos, MUINT32 u4Caller);
    MINT32 isFocusFinish();
    MINT32 isFocused();
    MINT32 isLockAE()
    {
        return i4IsLockAERequest;
    }
    E_AF_STATE_T getAFState();
    MVOID getAFRefWin( CameraArea_T &rWinSize);
    MVOID getAF2AEInfo( AF2AEInfo_T &rAFInfo);
    MINT32 getLensState();
    MFLOAT  getFocusDistance();
    MVOID   getFocusRange( MFLOAT *vnear, MFLOAT *vfar);
    MINT32 getAFPos()
    {
        return m_sAFOutput->i4AFPos;
    }

    ////// AFStaticInfo_T
    MINT32 getAfSupport();
    MINT32 getAfThreadEnable();
    MINT32 getAfHwEnableByP1();
    MINT32 getAFBestPos() // lastFocusPos
    {
        return m_sAFOutput->i4AFBestPos;
    }
    MINT32 getMaxLensPos(); // maxAfTablePos
    MINT32 getMinLensPos(); // minAfTablePos

    MINT64 TransStatProfileToAlgo( AF_STAT_PROFILE_T &sInSataProfile);

    MVOID updateFscFrmInfo();
    MINT32 updateRTVString(char* stringBuffer);

    /* Phase difference AF */
    virtual MVOID preparePDData();
    virtual MVOID UpdatePDParam( MINT32 &i4InROISel);

    /* Laser distance AF */
    virtual MVOID initLD();
    virtual MVOID startLD();
    virtual MVOID camPwrOffLD();
    virtual MVOID prepareLDData();

    /* Stereo depth AF */
    virtual MVOID initSD();
    virtual MVOID startSD();
    virtual MVOID prepareSDData();
    virtual MVOID updateSDParam();

    /* Sensor Provider */
    virtual MVOID initSP();
    virtual MVOID unintSP();
    virtual MVOID prepareSPData();





//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public :
    /**
     *  Instance.
     */
    static AfMgr  *s_pAfMgr;

private :
    // for config
    ConfigAFInput_T  m_sConfigInput;
    ConfigAFOutput_T m_sConfigOutput;
    // for start
    StartAFInput_T   m_sStartInput;
    StartAFOutput_T  m_sStartOutput;
    // for process
    DoSttIn_T        m_sDoSttInput;
    DoSttOut_T       m_sDoSttOutput;
    DoAFInput_T      m_sDoAFInput;
    DoAFOutput_T     m_sDoAFOutput;
    AF_INPUT_T*      m_sAFInput;
    AF_OUTPUT_T*     m_sAFOutput;

    AFIrregularCommand_T m_sAFIrgCommand;

    //pre frame information
    vector< AF_FRAME_INFO_T> m_vFrmInfo;

    IAfCxU *s_pIAfCxU;
    MINT32 m_i4IsCCUAF;
    AF_STAT_T* m_pAfoBuf;

    //===================================================================================================
    //===================================================================================================
    //===================================================================================================
    //===================================================================================================
    //===================================================================================================


    /**
     *  Flow control
     */
    mutable Mutex    m_Lock;
    volatile MINT32  m_i4Users;
    volatile MINT32  m_CCTUsers;
    MINT32 m_i4IsCctOper;
    MINT32 m_i4CctAfMode;
    MINT32 m_i4CctTriggerAf;
    MINT32 m_i4CctMfPos;
    CameraFocusArea_T m_i4CctAfArea;
    MINT32 m_i4ForceCPU;

    MINT32  m_i4EnableAF;
    MINT32  m_i4isAFStarted;
    MBOOL   m_bLDAFEn; /* Laser AF */
    MBOOL   m_bSDAFEn; /* Stereo Depth AF */

    LIB3A_AF_MODE_T  m_eLIB3A_AFMode;
    E_AF_STATE_T m_eAFState;
    E_AF_STATE_T m_eAFStatePre;
    MINT32 m_lensState;
    NVRAM_LENS_DATA_PARA_STRUCT *m_ptrNVRam;      /* nv ram scenario data */
    NVRAM_LENS_PARA_STRUCT   *m_ptrLensNVRam;     /* AF related NV ram pointer from nvram driver */
    MBOOL   m_bForceTrigger;   /* Control timing of triggering searching from host. */
    MBOOL   m_bTriggerCmdVlid; /* Trigger command which is sent from host is valid or not. */
    MBOOL   m_bLatchROI;       /* Control timing of latching ROI from host. */
    MBOOL   m_bLock;
    MBOOL   m_bNeedLock;
    MINT32  m_i4IsEnterCam;
    MINT32  m_i4IsZSD;
    MBOOL   m_bIsFullScan;
    MUINT8  m_aCurEMAFFlag;  // bit0: FullScan, bit1: AdvFullScan, bit2: temperature calibration
    MUINT8  m_aPreEMAFFlag;  // bit0: FullScan, bit1: AdvFullScan, bit2: temperature calibration
    MUINT8  m_aAdvFSRepeatTime;
    MBOOL   m_bWaitForceTrigger; /* Wait 3A HAL force to trigger CAF*/
    MBOOL   m_bPauseAF; /* The is for HAL only*/
    MINT32  m_i4UnPauseReqNum;
    MUINT8  m_aAEBlkVal[25];
    MUINT32 m_eEvent; /* Record event history at one request*/
    MUINT32 m_u4ReqMagicNum; /* Magic number of requerst*/
    MUINT32 m_u4StaMagicNum; /* Magic number which is dequeue from statistic buffer*/
    MUINT32 m_u4ConfigHWNum; /* Config number which get from Af Algo */
    MUINT32 m_u4LatestResNum;
    MUINT32 m_u4ConfigLatency;
    MUINT64 m_u8SofTimeStamp;
    MINT32  m_i4IsAFSearch_CurState; // 0: Done, 1: Hybrid(PD+FS), 2: Contrast
    MINT32  m_i4IsAFSearch_PreState;
    MINT32  m_i4IsSelHWROI_CurState; /* Record current status of "i4IsSelHWROI" which is output from algorithm*/
    MINT32  m_i4IsSelHWROI_PreState; /* Record previours status of "i4IsSelHWROI" which is output from algorithm*/
    MINT32  m_i4TAFStatus;     /* TAF flow receive autofocus and cancelAutoFocus. Don't do AF seraching when changing mode from auto mode to continuous mode. */
    MINT32  m_i4IsFocused;
    MINT32  m_i4EnThermalComp; /* Thermal Compensation */

    MINT32  m_i4MFPos;
    MINT32  m_i4InitPos;
    MINT32  m_i4InfPos;
    MINT32  m_i4MacroPos;
    MINT32  m_i450cmPos;
    MINT32  m_i4MiddlePos;
    MINT32  m_i4AFTabStr;
    MINT32  m_i4AFTabEnd;
    MINT32  m_i4SensorIdx;
    MINT32  m_eCurAFMode;
    MINT32  m_i4FullScanStep;
    MUINT64  m_u8MvLensTS; /* ms, time stamp as moving lens*/
    ISPAEINFO_QUEUE_T m_vISPAEQueue;
    MUINT32 m_eCamScenarioMode;   /* camera scenario mode */
    MUINT32 m_eNVRAMIndex;
    MINT32  m_i4OISDisable;
    mutable Mutex m_AFStateLock;
    E_AF_LOCK_AE_REQ_T  i4IsLockAERequest;
    MBOOL   m_bForceUnlockAE;
    MINT32  m_i4LensPosExit;
    MINT32  m_i4LensSupport;//m_i4AFMaxAreaNum;
    MINT32  m_i4isAEStable;
    MINT32 m_i4GyroValue;
    iterable_queue<MINT32> m_gyroValueQueue;
    MBOOL  m_bForceCapture;
    MINT32 m_i4ContinuePDMovingCount;
    AFRESULT_ISPREG_T m_sAFResultConfig;

    AF_FRAME_INFO_T m_sFRMInfo;
    MINT32 m_i4IsEnableFVInFixedFocus;
    MINT32 m_i4SkipAlgoDueToHwConstaint;

    // LaunchCamTrigger
    MINT32  m_i4LaunchCamTriggered;     // the first time trigger while launching camera
    MINT32  m_i4AEStableFrameCount;
    MINT32  m_i4AEStableTriggerTimeout;
    MINT32  m_i4ValidPDFrameCount;
    MINT32  m_i4ValidPDTriggerTimeout;
    MINT32  m_i4IsLockForLaunchCamTrigger;

    // FSC
    MINT32 m_i4ReadOutTimePerLine;
    MINT32 m_i4P1TimeReadOut;
    android::Vector<MINT32> DACList;
    FSC_FRM_INFO_T fscInfo;

    /**
     * Debug
     */
    AAA_DEBUG_TAG_T m_sMgrExif[MGR_EXIF_SIZE];
    AAA_DEBUG_TAG_T m_sMgrCapExif[MGR_CAPTURE_EXIF_SIZE];
    MINT32   m_i4DgbDisableAF;
    MINT32   m_i4DbgAutoBitTrue;
    MINT32   m_i4DgbLogLv;
    MINT32   m_i4DbgAfegainQueue;
    MINT32   m_i4DbgOISDisable;
    MUINT32  m_i4DbgOISPos;
    MUINT32  m_i4DbgPDVerifyEn;
    MINT32   m_i4DbgAdpAlarm;
    MINT32   m_i4DgbRtvEnable;

    MINT32   m_i4OTFDLogLv;
    MUINT32  m_i4MgrExifSz;
    MUINT32  m_i4MgrCapExifSz;

    /**
     * HybridAF IO
     */
    AF_PARAM_T *m_pAFParam;
    MBOOL       m_bGryoVd;
    MBOOL       m_bAcceVd;
    MINT32      m_i4AcceInfo[3];  /* g/gyro sensor listener handler and data*/
    MINT32      m_i4GyroInfo[3];
    MUINT32     m_u4ACCEScale;
    MUINT32     m_u4GyroScale;
    MUINT64     m_u8AcceTS;
    MUINT64     m_u8PreAcceTS;
    MUINT64     m_u8GyroTS;
    MUINT64     m_u8PreGyroTS;
    android::sp<NSCam::Utils::SensorProvider> mpSensorProvider;

    /**
     * PDAF
     */
    MBOOL m_bEnablePD; /* PD manager is vaild.*/
    MBOOL m_bRunPDEn; /* PD result can be got from pd manager since PD calculation ROIs are set to PD manager.*/
    MUINT64 m_u8RecvFDTS; /* record timestamp which receive face ROI */
    PD_CALCULATION_ROI_T m_sPDRois[eIDX_ROI_ARRAY_NUM];
    MINT32               m_i4PDCalculateWinNum;
    AFPD_BLOCK_ROI_T     m_sPDCalculateWin[AF_PSUBWIN_NUM];

    /**
     * Stereo Depth AF
     */
    DAF_TBL_STRUCT m_sDAF_TBL;
    AREA_T         m_sArea_Bokeh;

    /**
     * Laser Distance AF
     */
    LaserMgr *m_pLaserMgr; /* Laser Mgr*/

    /**
     * Digital zoom controlling..
     */
    AREA_T  m_sCropRegionInfo;
    MBOOL   m_bPdInputExpected;

#define RTVMAXCOUNT 8
    MINT32 m_rtvId[RTVMAXCOUNT];


    /**
     * Configure HW flow controlling.
     */
    MINT32      m_i4HWBlkNumX;  // for parsing afo only
    MINT32      m_i4HWBlkNumY;  // for parsing afo only
    MINT32      m_i4HWBlkSizeX; // for parsing afo only
    MINT32      m_i4HWBlkSizeY; // for parsing afo only

    MUINT32     m_u4AfoStatMode;
    MUINT32     m_u4AfoBlkSzByte;

    /**
     * Sensor information
     */
    IHalSensor *m_ptrIHalSensor;
    SensorStaticInfo  rSensorStaticInfo;
    IMGSENSOR_PDAF_SUPPORT_TYPE_ENUM m_i4PDAF_support_type;
    MINT32 m_i4CurrSensorDev;
    ext_ESensorDev_T c_sensorDev;
    MINT32 m_i4CurrSensorId;
    MUINT32 m_i4BINSzW;
    MUINT32 m_i4BINSzH;
    MUINT32 m_i4TGSzW;
    MUINT32 m_i4TGSzH;
    MINT32 m_i4CurrModuleId; /* Ref : nvram_drv_dep.cpp, readModuleIdFromEEPROM */


    /**
     * ROI Control :
     * All stored ROIs' coordinate is depended on TG size and applied Zoom effect.
     */
    // CameraFocusArea_T m_CameraFocusArea; /*Receive ROIs from Host command*/
    AREA_T m_sArea_Focusing; /* Focusing area*/
    AREA_T m_sArea_Center;   /* Center area.*/
    AREA_T m_sArea_APCmd;    /* AP command.*/
    AREA_T m_sArea_APCheck;  /* AP command check.*/
    AREA_T m_sArea_OTFD;     /* OT/FD command.*/
    AREA_T m_sArea_HW;       /* Record the ROI coordinate which is applied HW min constraint.*/
    MUINT32 m_sArea_TypeSel;

    /**
     * Driver
     */
    MINT32       m_i4CurrLensId;
    MUINT8       m_u1LensFileName[32];
    MINT32       m_i4MvLensTo;
    MINT32       m_i4MvLensToPre;
    LENS_INFO_T  m_sCurLensInfo;
    MCUDrv      *m_pMcuDrv; /* VCM  driver*/
    INormalPipe *m_pPipe; /* Iopipe 2.0*/

    /**
     * AF_mgr information for host.
     */
    AF_FOCUS_DIS_T m_sFocusDis;
    MFLOAT m_fMfFocusDistance;
    MINT32 m_i4AFTable[AF_TABLE_NUM];  /* Reply AF table to host if host request. */

    CHECK_AF_CALLBACK_INFO_T m_sCallbackInfo;
    MBOOL m_bNeedCheckSendCallback;
    MBOOL m_bNeedSendCallback;
    MUINT64 TS_SOF_Pre;

    MINT32 m_i4IsCAFWithoutFace;

    MBOOL m_bForceDoAlgo;
    MINT32 m_i4FirsetCalPDFrameCount; // the FrameCount of PD is calculated

    ISPAEInfo_T m_sIspAeInfo;
};

};  //  namespace NS3Av3
#endif // _AF_MGR_H_
