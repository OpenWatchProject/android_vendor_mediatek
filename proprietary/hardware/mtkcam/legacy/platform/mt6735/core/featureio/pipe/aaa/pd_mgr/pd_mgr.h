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

#include <isp_tuning.h>
//------------Thread-------------
#include <pthread.h>
#include <semaphore.h>
#include <vector>
#include <utils/Mutex.h>
using namespace std;
using namespace android;
//-------------------------------

#include <camera_custom_nvram.h>
#include <af_param.h>
#include <pd_param.h>
#include <pd_buf_common.h>



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
    * @brief init pd mgr.
    */
    EPDBuf_Type_t init(SPDProfile_t *iPDProfile);

    /**
    * @brief uninit pd mgr.
    */
    MRESULT uninit();
    /**
    * @brief set pd calibration data.
    */
    MRESULT setPDCaliData(MVOID *ptrInCaliData, MINT32 &i4OutInfoSz, MINT32 **ptrOutInfo);
    /**
    * @brief run pd task with setting data buffer and ROI.
    */
    MRESULT postToPDTask(SPDInputData_t *ptrInputData);
    /**
    * @brief pd result..
    */
    MRESULT getPDTaskResult(SPDOutputData_t *ptrOutputRes);
    /**
    * @brief get pd library version..
    */
    MRESULT GetVersionOfPdafLibrary(SPDLibVersion_t &tOutSWVer);


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
    /**
    * @brief thread setting
    */
    MVOID changePDBufThreadSetting();
    /**
    * @brief PD thread execution function
    */
    static MVOID* PDBufThreadLoop(MVOID*);
    /**
    * @brief post command to pd buffer thread.
    */
    MRESULT postToPDBufThread();


    /**
    * @brief pd core flow
    */
    MRESULT PDCoreFlow();
    /**
    * @brief set calibration data to pd core
    */
    MRESULT SetCaliData2PDCore(MVOID* ptrInCaliData, MINT32 i4InPDInfoSz, MINT *ptrOutPDInfo);


    /**
    * @brief pd open core flow
    */
    MRESULT PDOpenCoreFlow();
    /**
    * @brief set calibration data to pd  open core
    */
    MRESULT SetCaliData2PDOpenCore(MVOID* ptrInCaliData, MINT32 i4InPDInfoSz, MINT32 *ptrOutPDInfo);
    /**
    * @brief operator
    */
    MINT32 Boundary(MINT32, MINT32, MINT32);

    /**
    * @brief flow control
    */
    MRESULT PDCoreExtract( MUINT8 *ptrDataBuf, MUINT32 &u4DataBufSz, AREA_T *ptrROIs, MUINT32 &u4ROINum, PD_EXTRACT_DATA_T *ptrExtractRes);


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:

    MINT32      m_i4CurrSensorDev;
    SPDProfile_t m_profile;

    //debug control
    MINT32 m_bDebugEnable;
    MBOOL mPDVCTest;
    MUINT32 m_frmCnt;
    Mutex m_Lock;

    //thread
    MBOOL           m_bEnPDBufThd;
    sem_t           m_semPDBuf;
    sem_t           m_semPDBufThdEnd;
    pthread_t       m_PDBufThread;
    EPD_Status_t    m_Status;

    //PD buffer mgr :
    PDBufMgr *m_PD_mgr;
    PDBufMgrOpen *m_PD_mgr_open; //using 3rd party PD algo.

    //I/O
    //input
    MUINT32   m_curLensPos;
    MUINT32   m_databuf_size;
    MUINT8   *m_databuf;
    MUINT32   m_numROI;
    AREA_T    m_ROI[AF_PSUBWIN_NUM];
    //output
    vector<SPDResult_T> m_vPDOutput;

    // core PDAF
    IPdAlgo*    m_pIPdAlgo;
    MINT32      m_i4PDInfo2HybirdAF[10]; //this size is depend on interface of core pd algorithm
    PD_OUTPUT_T m_sPDOutput[AF_PSUBWIN_NUM];
    PD_INPUT_T  m_sPDInput[AF_PSUBWIN_NUM];
    PD_EXTRACT_DATA_T  m_sExtractPDOut[AF_PSUBWIN_NUM];


};

};  //  namespace NS3A
#endif // _AF_MGR_H_

