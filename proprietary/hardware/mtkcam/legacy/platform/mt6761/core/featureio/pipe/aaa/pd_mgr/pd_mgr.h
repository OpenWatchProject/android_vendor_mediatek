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
#include <mtkcam/drv/IHalSensor.h>

// pd hal
#include <camera_custom_nvram.h>
#include <pd_buf_common.h>
#include <dbg_aaa_param.h>
#include <isp_tuning.h>

// thread control
#include <pthread.h>
#include <semaphore.h>
#include <vector>
#include <utils/Mutex.h>

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

namespace NS3A
{

typedef enum
{
    EPD_Not_Enabled = 0,
    EPD_Init        = 1,
    EPD_BZ          = 2,
    EPD_Data_Ready  = 3
} EPD_Status_t;

typedef enum
{
    PDType_Legacy = 0,
    PDType_DualPD = 1
} PDTYPE_t;

typedef struct
{
    typedef struct
    {
        AFPD_BLOCK_ROI_T sRoiInfo;
        PD_INPUT_T sHandlePDInput;

    } S_ROIDATA_T;

    MUINT32     numDa;
    S_ROIDATA_T Da[AF_PSUBWIN_NUM];

} S_ANALYSIS_DATA_T;

/**
 * @brief PD manager class
 */
class PDMgr
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////    Disallowed.
    //  Copy constructor is disallowed.
    PDMgr(PDMgr const&);
    //  Copy-assignment operator is disallowed.
    PDMgr& operator=(PDMgr const&);

public:  ////
    PDMgr(ESensorDev_T eSensorDev);
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
     * @brief start pd mgr.
    */
    EPDBUF_TYPE_t start(MINT32 i4SensorIdx, SPDProfile_t *iPDProfile, PD_NVRAM_T *ptrInPDNvRam);

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
    * @brief run pd task with setting data buffer and ROI.
    */
    MRESULT postToPDTask( PD_CALCULATION_INPUT *ptrInputData);
    /**
    * @brief pd result..
    */
    MRESULT getPDTaskResult( PD_CALCULATION_OUTPUT **ptrResultOutput);
    /**
    * @brief get pd library version..
    */
    MRESULT GetVersionOfPdafLibrary(SPDLibVersion_t &tOutSWVer);
    /**
     * @brief get pd library exif data.
     */
    MRESULT GetDebugInfo( AF_DEBUG_INFO_T &sOutDbgInfo);
    /**
     * @brief update PD parameters
     */
    MRESULT UpdatePDParam( MUINT32 u4FrmNum, MINT32 i4InputPDAreaNum, AFPD_BLOCK_ROI_T *tInputPDArea, MINT32 i4MinLensPos, MINT32 i4MaxLensPos);

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
    MRESULT PDCoreFlow();
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
    MRESULT PDOpenCoreFlow();
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
    MVOID PDVerificationFlow();


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:

    MINT32      m_i4CurrSensorDev;
    MINT32        m_i4SensorIdx;
    SPDProfile_t m_profile;

    //sensor information
    NSCam::IHalSensor*  m_pIHalSensor;
    SET_PD_BLOCK_INFO_T m_PDBlockInfo;
    SensorCropWinInfo   m_SensorCropInfo;
    SensorVCInfo m_SensorVCInfo;

    // NvRam
    PD_CALIBRATION_DATA_T  m_sCaliData;
    MUINT8                 m_u1CaliDaSrc;
    MINT32 m_i4CaliAFPos_Inf;
    MINT32 m_i4CaliAFPos_Macro;
    MINT32 m_i4CaliAFPos_50cm;
    MINT32 m_i4CaliAFPos_Middle;

    //debug control
    MINT32 m_bDebugEnable;
    MINT32  m_i4DbgPdDump;
    MINT32  m_i4DbgDisPdHandle;
    MUINT32  m_i4DbgPDVerifyEn; /*cmd*/
    MUINT32  m_i4DbgPDVerifyRun; /*cmd*/
    MUINT32  m_i4DbgPDVerifyStepSize; /*calculated from cmd*/
    MUINT32  m_i4DbgPDVerifyCalculateNum; /*cmd*/
    MUINT32  m_i4DbgPDVerifyCalculateCnt;

    MUINT32 m_MagicNumber;
    MUINT32 m_frmCnt;
    Mutex   m_Lock_Result;
    Mutex   m_Lock_PDWinCfg;

    //thread
    MBOOL           m_bEnPDBufThd;
    sem_t           m_semPDBuf;
    sem_t           m_semPDBufThdEnd;
    pthread_t       m_PDBufThread;
    EPD_Status_t    m_Status;

    //PD buffer manager
    PDBufMgr     *m_pPDBufMgrCore;
    PDBufMgrOpen *m_pPDBufMgrOpen; //using 3rd party PD algo.
    MUINT32      m_pdoPixels[MAX_PD_PAIR_NUM * 2][2];

    //data to be analyzed
    S_ANALYSIS_DATA_T m_sAnalysisData;
    S_ANALYSIS_DATA_OPEN_T m_sAnalysisDataList;

    //I/O
    //input
    MUINT32   m_curLensPos;
    MUINT32   m_databuf_size;
    MUINT32   m_databuf_stride;
    MUINT8   *m_databuf;
    MINT32    m_afeGain;
    MINT32    m_iso;
    MBOOL     m_bAEStable;
    MINT32    m_minLensPos;
    MINT32    m_maxLensPos;

    //output
    vector<SPDResult_T> m_vPDOutput;

    // Core flow
    NS3A::IPdAlgo      *m_pIPdAlgo;

    //flow control
    SPDOHWINFO_T    m_sPDOHWInfo;
    SDUALPDVCINFO_T m_sDualPDVCInfo;
    MUINT8         *m_pPDORes;

};

};  //  namespace NS3A
#endif // _PD_MGR_H_

