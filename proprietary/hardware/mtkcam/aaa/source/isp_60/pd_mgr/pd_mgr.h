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
 * @file pd_mgr.h
 * @brief PD manager, do focusing for raw sensor.
 */
#ifndef _PD_MGR_H_
#define _PD_MGR_H_

// driver related
#include <kd_imgsensor_define.h>
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/drv/mem/cam_cal_drv.h>

// pd hal
#include <camera_custom_nvram.h>
#include <pd_buf_common.h>
#include <dbg_aaa_param.h>

// 3a defined type
#include "af_feature.h"
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <isp_tuning.h>
#include <isp_tuning_sensor.h>
#include "hal/inc/custom/aaa/ae_param.h"

// thread control
#include <pthread.h>
#include <semaphore.h>
#include <vector>
#include <utils/Mutex.h>

// ISP HAL
#include <isp_config/isp_pdo_config.h>

// pd statistic data from
#include <StatisticBuf.h>

//
#include <string>

//
#include "mcu_drv.h"

using namespace std;
using namespace android;
using namespace NSCam;
using namespace NSIspTuning;

class PDBufMgrOpen;
class PDBufMgr;

namespace NS3A
{
class IAfAlgo;
class IPdAlgo;
};

namespace NS3Av3
{

typedef enum
{
    ePDHAL_STATE_IDLE = 0,
    ePDHAL_STATE_SET_CALIBRATION_DATA_DONE,
    ePDHAL_STATE_SET_BLOCK_INFO_DONE,
    ePDHAL_STATE_INIT_ALGO_DONE, /* algo is ready for calculation */
    ePDHAL_STATE_CALCULATING,
    ePDHAL_STATE_NOT_SUPPORT
} E_PDHAL_STATE_T;

typedef enum
{
    PDType_Legacy = 0,
    PDType_DualPD = 1
} PDTYPE_t;

//
#define ANALYSIS_DATA_BUF_SZ 2
typedef struct
{
    typedef struct
    {
        MUINT32          lensAdpCompPos;
        AFPD_BLOCK_ROI_T sRoiInfo;
        PD_INPUT_T       sHandlePDInput;

    } S_ROIDATA_T;

    MUINT32       magicNumber;
    MINT64        sof_time_stamp;
    mcuMotorInfo  lensInfo;
    MBOOL         bAEStable;
    MINT32        afeGain;
    MINT32        iso;
    MUINT64       exposureTime; /* ns */
    MUINT32       isFlashOn;

    //
    MVOID        *pBufForExtractPD;
    MUINT32       numDa;
    S_ROIDATA_T   Da[AF_PSUBWIN_NUM];

    // 3rd party
    MUINT32  mode;
    MUINT32  cfgNum;
    MUINT32  sttBuf_size;
    MUINT32  sttBuf_stride;
    MUINT8  *sttBuf;
} S_ANALYSIS_DATA_T;


//
typedef struct RAW_BUF_CTRL_t
{
    MUINT32 mMagicNumber;
    MUINT32 mImgoFmt;
    MUINT32 mImgHeapID;

    RAW_BUF_CTRL_t( MUINT32 magicNumber,
                    MUINT32 fmt,
                    MUINT32 heapID)
        : mMagicNumber(magicNumber)
        , mImgoFmt(fmt)
        , mImgHeapID(heapID)
    {}

} RAW_BUF_CTRL_T;

//
typedef struct dbg_verify_t
{
    SPDResult_T cal_res;
    MINT32      afeGain;
    MINT32      iso;
    MUINT64     exposureTime;

    dbg_verify_t()
        : afeGain(0)
        , iso(0)
        , exposureTime(0)
    {}

} dbg_verify_T;

//
typedef struct dbg_linear_regression_t
{
    //
    double s_x;
    double s_y;
    double s_xx;
    double s_xy;
    //
    double slope;
    double intercept;
    //
    double s_yres;
    double s_res;
    double Rsqr;

    dbg_linear_regression_t()
        : s_x(0)
        , s_y(0)
        , s_xx(0)
        , s_xy(0)
        , slope(0)
        , intercept(0)
        , s_yres(0)
        , s_res(0)
        , Rsqr(0)
    {}
} dbg_linear_regression_T;

/**
 * @brief PD manager class
 */
class PDMgr
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:    ////    Disallowed.
    //  Copy constructor is disallowed.
    PDMgr(PDMgr const&);
    //  Copy-assignment operator is disallowed.
    PDMgr& operator=(PDMgr const&);

public:  ////
    PDMgr(MINT32 const i4SensorDev);
    ~PDMgr();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief Get AF manager instance.
     */
    static PDMgr& getInstance(MINT32 const i4SensorDev);
    static PDMgr* s_pPDMgr; // FIXME: REMOVED LATTER

    /**
     * @brief set NVRAMindex.
     */
    MRESULT setNVRAMIndex(MUINT32 afNVRamIdx);
    /**
     * @brief init pd mgr.
     */
    MRESULT config(MINT32 sensorOpenIndex, MUINT32 sensorMode, MUINT32 afNVRamIdx);
    /**
     * @brief notify pre-stop pd mgr.
     */
    MRESULT preStop();
    /**
     * @brief stop pd mgr.
     */
    MRESULT stop();
    /**
     * @brief camera power on state.
     */
    MBOOL CamPwrOnState();
    /**
     * @brief camera power off state.
     */
    MBOOL CamPwrOffState();
    /**
     * @brief for pipeline flow control.
     */
    MRESULT doSWPDE(MVOID *iHalMetaData, MVOID *iImgbuf);
    /**
     * @brief run pd task with setting data buffer and ROI.
     */
    MRESULT postToPDTask( StatisticBufInfo *pSttData, mcuMotorInfo *pLensInfo=NULL);
    /**
     * @brief pd result..
     */
    MRESULT getPDTaskResult( PD_CALCULATION_OUTPUT **ptrResultOutput);
    /**
     * @brief get pd library version..
     */
    MRESULT GetVersionOfPdafLibrary( SPDLibVersion_t &tOutSWVer);
    /**
     * @brief get pd library exif data.
     */
    MRESULT GetDebugInfo( AF_DEBUG_INFO_T &sOutDbgInfo);
    /**
     * @brief update PD parameters
     */
    MRESULT UpdatePDParam( MUINT32 u4FrmNum, MINT32 i4InputPDAreaNum, AFPD_BLOCK_ROI_T *tInputPDArea, MINT32 i4MinLensPos, MINT32 i4MaxLensPos, MBOOL bForceCalculation);
    /**
     * @brief update PDO HW setting
     */
    MRESULT getPDOHWCfg(ISP_PDO_CFG_T *pCfg);
    /**
     * @brief query PDO information
     */
    MRESULT getPDInfoForSttCtrl(MINT32 sensorIdx, MINT32 sensorMode, MUINT32 &oPDOSizeW, MUINT32 &oPDOSizeH, FEATURE_PDAF_STATUS &oPDAFStatus);
    /**
     * @brief set PBN information
     */
    MRESULT setPBNen(MBOOL bEnable);
    /**
     * @brief set AE target mode
     */
    MRESULT setAETargetMode(eAETargetMODE eAETargetMode);
    /**
     * @brief query module status
     */
    MRESULT isModuleEnable(MBOOL &oModuleEn);
    /**
     * @brief set request(from 3a framework)
     */
    MRESULT setRequest(MINT32 iRequestNum);
    /**
     * @brief notify pd manager that imgo pure raw is enqueued so swpde node can return.
     */
    MRESULT imgoIsEnqueued(StatisticBufInfo *pSttData);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Private function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    /**
     * @brief create pd mgr thread
     */
    MVOID createThread();
    /**
     * @brief close pd mgr thread
     */
    MVOID closeThread();
    /**
     * @brief thread setting
     */
    MVOID changePDBufThreadSetting();
    /**
     * @brief PD thread execution function
     */
    static MVOID* PDBufThreadLoop(MVOID*);
    /**
     * @brief pd core flow
     */
    MRESULT PDCoreFlow(SPDResult_T *pResBuf);
    /**
     * @brief set calibration data to pd core
     */
    MRESULT SetCaliData2PDCore(PD_ALGO_TUNING_T* ptrInTuningData, PD_CALIBRATION_DATA_T* ptrInCaliData);
    /**
     * @brief set pd calibration data.
     */
    MRESULT setPDCaliData(PD_NVRAM_T *ptrInPDNvRam);
    /**
     * @brief pd open core flow
     */
    MRESULT PDOpenCoreFlow(SPDResult_T *pResBuf);
    /**
     * @brief set calibration data to pd  open core
     */
    MRESULT SetCaliData2PDOpenCore(PD_ALGO_TUNING_T* ptrInTuningData, PD_CALIBRATION_DATA_T* ptrInCaliData);
    /**
     * @brief config PD hw module setting, need to be called after setPDCaliData.
     */
    MRESULT ConfigurePDHWSetting(SPDProfile_t *iPDProfile, PD_ALGO_TUNING_T *ptrInTuningData);
    /**
     * @brief operator
     */
    MINT32 Boundary(MINT32, MINT32, MINT32);
    /**
     * @brief verification flow
     */
    MVOID PDVerificationFlow(S_ANALYSIS_DATA_T *pAnalysisDa, SPDResult_T *pResBuf);
    /**
     * @brief lens adaptive compensation
     */
    MUINT32 GetLensAdpCompPos( MINT32 pixel_clk, MINT32 line_length, MUINT64 expTime, MINT64 time_stamp_sof, mcuMotorInfo &lensInfo, AREA_T &roi);


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:

    MINT32        m_i4CurrSensorDev;
    MINT32        m_i4SensorIdx;
    SPDProfile_t  m_profile;

    // sensor information
    IHalSensorList       *m_pIHalSensorList;
    IHalSensor           *m_pIHalSensor;
    SET_PD_BLOCK_INFO_T   m_PDBlockInfo;
    SensorCropWinInfo     m_SensorCropInfo;
    SINGLE_VC_INFO2       m_PDVCInfo;
    SensorStaticInfo      m_SensorStaticInfo;
    SensorDynamicInfo     m_SensorDynamicInfo;
    MINT32                m_Sensor_pixel_clk;
    MINT32                m_Sensor_line_length;
    MINT32                m_Sensor_frame_length;

    // NvRam
    NVRAM_LENS_DATA_PARA_STRUCT *m_pNVRam;      /* nv ram scenario data */
    NVRAM_LENS_PARA_STRUCT      *m_pLensNVRam;     /* AF related NV ram pointer from nvram driver */
    PD_CALIBRATION_DATA_T m_sCaliData;
    MUINT8                m_u1CaliDaSrc;
    MINT32  m_i4CaliAFPos_Inf;
    MINT32  m_i4CaliAFPos_Macro;
    MINT32  m_i4CaliAFPos_50cm;
    MINT32  m_i4CaliAFPos_Middle;
    MUINT32 m_i4NVRamIdx;

    // debug control
    MINT32      m_i4DbgSystraceLevel;
    MINT32      m_bDebugEnable;
    MINT32      m_i4DbgPdDump;
    std::string m_i4DbgPdDumpTS;
    MINT32      m_i4DbgDisPdHandle;
    MUINT32     m_i4DbgPDVerifyEn; /*cmd*/
    MUINT32     m_i4DbgPDVerifyRun; /*cmd*/
    MUINT32     m_i4DbgPDVerifyStepSize; /*calculated from cmd*/
    MUINT32     m_i4DbgPDVerifyCalculateNum; /*cmd*/
    MUINT32     m_i4DbgPDVerifyCalculateCnt;
    MUINT32     m_i4DbgPDVerifyRangeL;
    MUINT32     m_i4DbgPDVerifyRangeH;
    vector<dbg_verify_T> m_vDbgPDVerifyDa;
    dbg_linear_regression_T *m_pDbgPDVerifyRes;

    MUINT32 m_requestNum;
    Mutex   m_Lock_NvRamIdx;

    // thread
    MBOOL        m_bEnPDBufThd;
    sem_t        m_semPDBuf;
    pthread_t    m_PDBufThread;
    E_PDHAL_STATE_T m_Status;

    // calculation
    Mutex          m_Lock;
    NS3A::IPdAlgo *m_pIPdAlgo;
    PDBufMgr      *m_pPDBufMgrCore;
    PDBufMgrOpen  *m_pPDBufMgrOpen; //using 3rd party PD algo.
    PD_CONFIG_T    m_sPDBlockInfoCali;

    // data to be analyzed
    MBOOL              m_bIsAnalysisDataUpdated;
    Mutex              m_Lock_AnalysisData;
    S_ANALYSIS_DATA_T *m_pAnalysisData_Update;
    S_ANALYSIS_DATA_T *m_pAnalysisData_Calculate;
    S_ANALYSIS_DATA_T  m_sAnalysisData[ANALYSIS_DATA_BUF_SZ]; /*ping pong buffer.*/
    // data to be analyzed for 3rd party lib
    S_CONFIG_DATA_OPEN_T m_sConfigDataOpen;

    // I/O
    // input
    MINT32       m_minLensPos;
    MINT32       m_maxLensPos;

    // output
    Mutex        m_Lock_ResultBuf;
    SPDResult_T *m_pResultBuf_Readout;
    SPDResult_T *m_pResultBuf_Calculate;
    SPDResult_T  m_sResultBuf[PD_RES_BUF_SZ];

    // flow control
    SPDOHWINFO_T    m_sPDOHWInfo;
    ISP_PDO_CFG_T   m_sPDOHWCfg;
    SDUALPDVCINFO_T m_sDualPDVCInfo;
    MUINT8         *m_pPDORes;
    MBOOL           m_bEnablePBIN;
    eAETargetMODE   m_eAETargetMode;
    MBOOL           m_bConfigCompleted;
    MBOOL           m_bNeedPDResult;
    MBOOL           m_bForceCalculation;

    // flow control for SWPDE
    Mutex m_Lock_RawBufCtrl;
    vector<RAW_BUF_CTRL_T> m_vRawBufCtrl;
    sem_t                  m_semSWPDE;
    MBOOL                  m_bPreStop;

};

};  //  namespace NS3Av3
#endif // _PD_MGR_H_

