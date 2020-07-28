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

#ifndef _AAA_HAL_STT_CTRL_H_
#define _AAA_HAL_STT_CTRL_H_

//------------Thread-------------
#include <pthread.h>
#include <semaphore.h>
#include <IThread.h>
//-------------------------------

#include <stdio.h>
#include <stdlib.h>

#include <aaa_types.h>
#include <buf_mgr/IBufMgr.h>
#if CAM3_FAKE_SENSOR_DRV
#include "fake_sensor_drv/IStatisticPipe.h"
#include "fake_sensor_drv/ICamsvStatisticPipe.h"
#else
#include <mtkcam/drv/iopipe/CamIO/IStatisticPipe.h>
#include <mtkcam/drv/iopipe/CamIO/ICamsvStatisticPipe.h>
#endif
#include <af_feature.h>
#include <af_param.h>
extern "C"{
#include <af_algo_if.h>
}
//#include <af_mgr/af_mgr_if.h>
#include <af_define.h>
#include <mtkcam/drv/IHalSensor.h>

using namespace NSCam;
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSCamIOPipe;

#define ENABLE_STT_FLOW_AAO 1
#define ENABLE_STT_FLOW_AFO 2
#define ENABLE_STT_FLOW_FLKO 4
#define ENABLE_STT_FLOW_MVHDR3EXPO 8
#define ENABLE_STT_FLOW_TSFO 16



namespace NS3Av3
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef enum
{
    FEATURE_MVHDR_UNSUPPORT = 0,
    FEATURE_MVHDR_SUPPORT_VIRTUAL_CHANNEL = 1,
    FEATURE_MVHDR_SUPPORT_3EXPO_VIRTUAL_CHANNEL = 2,
} FEATURE_MVHDR_STATUS;

class Hal3ASttCtrl
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  //    Ctor/Dtor.
    Hal3ASttCtrl();
    virtual ~Hal3ASttCtrl();

private: // disable copy constructor and copy assignment operator
    Hal3ASttCtrl(const Hal3ASttCtrl&);
    Hal3ASttCtrl& operator=(const Hal3ASttCtrl&);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    //
    static Hal3ASttCtrl* createInstance(MINT32 i4SensorDevId, MINT32 i4SensorOpenIndex);
    static Hal3ASttCtrl* getInstance(MINT32 i4SensorDevId);
    virtual MVOID destroyInstance();
    virtual MBOOL setSensorDevInfo(MINT32 i4SensorDevId, MINT32 i4SensorOpenIndex);
    virtual MBOOL getPdInfoForSttCtrl();
    virtual MBOOL initStt(MINT32 i4SensorDevId, MINT32 i4SensorOpenIndex, const ConfigInfo_T& rConfigInfo);
    virtual MBOOL startStt();
    virtual MBOOL preStopStt();
    virtual MBOOL stopStt();
    virtual MBOOL uninitStt();
    virtual MVOID pause();
    virtual MVOID resume();
    virtual MBOOL abortDeque();
    virtual MVOID setSensorMode(MINT32 i4SensorMode);
    FEATURE_PDAF_STATUS quertPDAFStatus(){return m_pPDAFStatus;}
    virtual IBufMgr* getBufMgr(BUF_PORTS port);
    MINT32 isMvHDREnable();
    MBOOL isAFOEnable();
private:
    MVOID querySensorStaticInfo();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    volatile std::atomic<int>        m_Users;
    mutable std::mutex  m_Lock;
    IBufMgr*            m_pBufMgrList[BUF_NUM];
    ThreadStatisticBuf* rAAOBufThread;
    ThreadStatisticBuf* rAFOBufThread;
    ThreadStatisticBuf* rPDOBufThread;
    ThreadStatisticBuf* rMVHDR3EXPOOBufThread;
    MINT32              m_i4SensorDev;
    MINT32              m_i4SensorIdx;
    MINT32              m_i4SensorMode;
    MINT32              m_i4SttPortEnable;
    MUINT32             m_u4TGSizeW;
    MUINT32             m_u4TGSizeH;
    MUINT32             m_u4PDOSizeW;
    MUINT32             m_u4PDOSizeH;
    IStatisticPipe*        m_pSttPipe;
    ICamsvStatisticPipe*   m_pCamsvSttPipe[VC_MAX_NUM];
    FEATURE_PDAF_STATUS    m_pPDAFStatus;
    FEATURE_MVHDR_STATUS   m_pMvHDRStatus;
    MBOOL               m_bPreStopSttEnable;
    //
    ConfigInfo_T        m_rConfigInfo;
    SensorVCInfo2       m_rVcInfo;
    MBOOL               m_bEnableCAMSVOf3EXPO;
    std::vector<MUINT32> m_vecCAMSVIndexOf3EXPO;
};

template <MINT32 sensorDevId>
class Hal3ASttCtrlDev : public Hal3ASttCtrl
{
public:
    static Hal3ASttCtrlDev* getInstance()
    {
        static Hal3ASttCtrlDev<sensorDevId> singleton;
        return &singleton;
    }
    Hal3ASttCtrlDev()
        : Hal3ASttCtrl()
    {
    }

private:

};

}; // namespace NS3Av3

#endif

