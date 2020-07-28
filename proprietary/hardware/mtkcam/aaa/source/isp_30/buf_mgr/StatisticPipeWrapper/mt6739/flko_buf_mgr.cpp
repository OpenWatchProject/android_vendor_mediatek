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
#define LOG_TAG "flko_buf_mgr"

#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG       (1)
#endif

#include <cutils/atomic.h>
#include <cutils/properties.h>
#include <string.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include <list>
#include <isp_tuning.h>

#include <iopipe/CamIO/INormalPipe.h>
#include "flko_buf_mgr.h"

extern Mutex m_FLKOLockReg;

using namespace std;
using namespace android;
using namespace NS3Av3;
using namespace NSIspTuning;
using namespace NSCam::NSIoPipeIsp3::NSCamIOPipe;

typedef list<BufInfo_T> BufInfoList_T;

#define MAX_FLKO_BUFFER_CNT (4)
#define FLKO_MAX_LENG (6000)
#define FLKO_BUF_SIZE (FLKO_MAX_LENG * 4 * 3)



#if (MAX_FLKO_BUFFER_CNT == 1)
#define getHwBufAddr getCurrHwBuf
#else
#define getHwBufAddr getNextHwBuf
#endif

#define IOPIPE_SET_MODUL_ENABLE(module, enable) { \
                                                  if (MTRUE != sendCommandNormalPipe(NSImageioIsp3::NSIspio::EPIPECmd_SET_MODULE_EN, module, enable, MNULL)) \
                                                  { \
                                                      CAM_LOGE("EPIPECmd_SET_MODULE_EN ConfigHWReg fail"); \
                                                  } \
                                                }
#define IOPIPE_GET_MODUL_HANDLE(module, handle) { \
                                                  if (MTRUE != sendCommandNormalPipe(NSImageioIsp3::NSIspio::EPIPECmd_GET_MODULE_HANDLE, module, (MINTPTR)&handle, (MINTPTR)(&("FLKOBufMgr::DMAConfig()")))) \
                                                  { \
                                                      CAM_LOGE("EPIPECmd_GET_MODULE_HANDLE ConfigHWReg fail"); \
                                                  } \
                                                }
#define IOPIPE_SET_MODUL_CFG_DONE(handle)       { \
                                                  if (MTRUE != sendCommandNormalPipe(NSImageioIsp3::NSIspio::EPIPECmd_SET_MODULE_CFG_DONE, handle, MNULL, MNULL)) \
                                                  { \
                                                      CAM_LOGE("EPIPECmd_SET_MODULE_CFG_DONE ConfigHWReg fail"); \
                                                  } \
                                                }
#define IOPIPE_RELEASE_MODUL_HANDLE(handle)     { \
                                                  if (MTRUE != sendCommandNormalPipe(NSImageioIsp3::NSIspio::EPIPECmd_RELEASE_MODULE_HANDLE, handle, MNULL, MNULL)) \
                                                  { \
                                                      CAM_LOGE("EPIPECmd_RELEASE_MODULE_HANDLE ConfigHWReg fail"); \
                                                  } \
                                                  handle = NULL; \
                                                }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class FLKOBufMgrWrapper
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    //  Copy constructor is disallowed.
    FLKOBufMgrWrapper(FLKOBufMgrWrapper const&);
    //  Copy-assignment operator is disallowed.
    FLKOBufMgrWrapper& operator=(FLKOBufMgrWrapper const&);

public:
    FLKOBufMgrWrapper(ESensorDev_T const eSensorDev);
    ~FLKOBufMgrWrapper();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static FLKOBufMgrWrapper& getInstance(MINT32 const i4SensorDev);
    MBOOL init(MINT32 const i4SensorIdx);
    MBOOL uninit();
    MBOOL debugPrint();
    MBOOL enqueueHwBuf(BufInfo_T& rBufInfo);
    MBOOL dequeueHwBuf(BufInfo_T& rBufInfo);
    MUINT32 getCurrHwBuf();
    MUINT32 getNextHwBuf();
    MBOOL allocateBuf(BufInfo_T &rBufInfo, MUINT32 u4BufSize);
    MBOOL freeBuf(BufInfo_T &rBufInfo);
    MBOOL updateDMABaseAddr(MUINT32 u4BaseAddr);
    MBOOL DMAInit(MBOOL En);
    MBOOL DMAUninit();

    /* ISP 4.x */
    BufInfo_T m_rBufInfo;
    MBOOL initPipe(MINT32 const i4SensorIdx, MINT32 const i4TGInfo);     // create Buffer
    MBOOL uninitPipe();
    MBOOL startPipe();
    MBOOL stopPipe();
    MBOOL dequePipe(BufInfo_T& rBufInfo);
    MBOOL enquePipe(BufInfo_T& rBufInfo);
    //BOOL configPipe();

    inline MBOOL sendCommandNormalPipe(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3)
    {
        INormalPipe_FrmB* pPipe = INormalPipe_FrmB::createInstance(m_i4SensorIdx, "flko_buf_mgr");
        MBOOL fgRet = pPipe->sendCommand(cmd, arg1, arg2, arg3);
        pPipe->destroyInstance("flko_buf_mgr");
        return fgRet;
    }

    inline MBOOL setTGInfo(MINT32 const i4TGInfo)
    {
        m_TgInfo = i4TGInfo;
        MY_LOG("[%s()]i4TGInfo: %d\n", __FUNCTION__, i4TGInfo);
        switch (i4TGInfo)
        {
        case CAM_TG_1:
            m_eSensorTG = ESensorTG_1;
            break;
        case CAM_TG_2:
            m_eSensorTG = ESensorTG_2;
            break;
        default:
            MY_ERR("i4TGInfo= %d", i4TGInfo);
            return MFALSE;
        }
        return MTRUE;
    }


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    ESensorDev_T const     m_eSensorDev;
    ESensorTG_T            m_eSensorTG;
    IMemDrv*               m_pIMemDrv;
    volatile MINT32        m_Users;
    mutable android::Mutex m_Lock;
    MBOOL                  m_bDebugEnable;
    MINT32                 m_i4SensorIdx;
    BufInfoList_T          m_rHwBufList;
    BufInfo_T              m_rFLKOBufInfo[MAX_FLKO_BUFFER_CNT];

    /* FLK irq */
    IspDrv*                mpIspDrv;
    MUINT                  m_TgInfo;
    MINT32                 FLKIrq_key;

    /* ISP 4.x */
    BufInfo_T              m_rHwBuf;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
template <ESensorDev_T const eSensorDev>
class FLKOBufMgrDev : public FLKOBufMgrWrapper
{
public:
    static FLKOBufMgrWrapper& getInstance()
    {
        static FLKOBufMgrDev<eSensorDev> singleton;
        return singleton;
    }

    FLKOBufMgrDev(): FLKOBufMgrWrapper(eSensorDev) {}
    virtual ~FLKOBufMgrDev() {}
};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IFLKOBufMgrWrapper::init(MINT32 const i4SensorDev, MINT32 const i4SensorIdx)
{
    MBOOL ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = FLKOBufMgrWrapper::getInstance(ESensorDev_Main).init(i4SensorIdx);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = FLKOBufMgrWrapper::getInstance(ESensorDev_Sub).init(i4SensorIdx);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = FLKOBufMgrWrapper::getInstance(ESensorDev_MainSecond).init(i4SensorIdx);

    return ret_main && ret_sub && ret_main2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IFLKOBufMgrWrapper::uninit(MINT32 const i4SensorDev)
{
    MBOOL ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = FLKOBufMgrWrapper::getInstance(ESensorDev_Main).uninit();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = FLKOBufMgrWrapper::getInstance(ESensorDev_Sub).uninit();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = FLKOBufMgrWrapper::getInstance(ESensorDev_MainSecond).uninit();

    return ret_main && ret_sub && ret_main2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IFLKOBufMgrWrapper::DMAInit(MINT32 const i4SensorDev)
{
    MBOOL ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = FLKOBufMgrWrapper::getInstance(ESensorDev_Main).DMAInit(MTRUE);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = FLKOBufMgrWrapper::getInstance(ESensorDev_Sub).DMAInit(MTRUE);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = FLKOBufMgrWrapper::getInstance(ESensorDev_MainSecond).DMAInit(MTRUE);

    return ret_main && ret_sub && ret_main2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IFLKOBufMgrWrapper::DMAUninit(MINT32 const i4SensorDev)
{
    MBOOL ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = FLKOBufMgrWrapper::getInstance(ESensorDev_Main).DMAUninit();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = FLKOBufMgrWrapper::getInstance(ESensorDev_Sub).DMAUninit();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = FLKOBufMgrWrapper::getInstance(ESensorDev_MainSecond).DMAUninit();

    return ret_main && ret_sub && ret_main2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IFLKOBufMgrWrapper::enqueueHwBuf(MINT32 const i4SensorDev, BufInfo_T& rBufInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return FLKOBufMgrWrapper::getInstance(ESensorDev_Main).enqueueHwBuf(rBufInfo);
    else if (i4SensorDev & ESensorDev_Sub)
        return FLKOBufMgrWrapper::getInstance(ESensorDev_Sub).enqueueHwBuf(rBufInfo);
    else if (i4SensorDev & ESensorDev_MainSecond)
        return FLKOBufMgrWrapper::getInstance(ESensorDev_MainSecond).enqueueHwBuf(rBufInfo);

    MY_ERR("Incorrect sensor device: i4SensorDev = %d", i4SensorDev);
    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IFLKOBufMgrWrapper::dequeueHwBuf(MINT32 const i4SensorDev, BufInfo_T& rBufInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return FLKOBufMgrWrapper::getInstance(ESensorDev_Main).dequeueHwBuf(rBufInfo);
    else if (i4SensorDev & ESensorDev_Sub)
        return FLKOBufMgrWrapper::getInstance(ESensorDev_Sub).dequeueHwBuf(rBufInfo);
    else if (i4SensorDev & ESensorDev_MainSecond)
        return FLKOBufMgrWrapper::getInstance(ESensorDev_MainSecond).dequeueHwBuf(rBufInfo);

    MY_ERR("Incorrect sensor device: i4SensorDev = %d", i4SensorDev);
    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IFLKOBufMgrWrapper::updateDMABaseAddr(MINT32 const i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return FLKOBufMgrWrapper::getInstance(ESensorDev_Main).updateDMABaseAddr(FLKOBufMgrWrapper::getInstance(ESensorDev_Main).getNextHwBuf());
    else if (i4SensorDev & ESensorDev_Sub)
        return FLKOBufMgrWrapper::getInstance(ESensorDev_Sub).updateDMABaseAddr(FLKOBufMgrWrapper::getInstance(ESensorDev_Sub).getNextHwBuf());
    else if (i4SensorDev & ESensorDev_MainSecond)
        return FLKOBufMgrWrapper::getInstance(ESensorDev_MainSecond).updateDMABaseAddr(FLKOBufMgrWrapper::getInstance(ESensorDev_MainSecond).getNextHwBuf());

    MY_ERR("Incorrect sensor device: i4SensorDev = %d", i4SensorDev);
    return MFALSE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IFLKOBufMgrWrapper::setTGInfo(MINT32 const i4SensorDev, MINT32 const i4TGInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return FLKOBufMgrWrapper::getInstance(ESensorDev_Main).setTGInfo(i4TGInfo);
    else if (i4SensorDev & ESensorDev_Sub)
        return FLKOBufMgrWrapper::getInstance(ESensorDev_Sub).setTGInfo(i4TGInfo);
    else if (i4SensorDev & ESensorDev_MainSecond)
        return FLKOBufMgrWrapper::getInstance(ESensorDev_MainSecond).setTGInfo(i4TGInfo);

    MY_ERR("Incorrect sensor device: i4SensorDev = %d", i4SensorDev);
    return MFALSE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                               ISP 3.0 Wrapper
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IFLKOBufMgrWrapper::initPipe(MINT32 const i4SensorDev, MINT32 const i4SensorIdx, MINT32 const i4TGInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return FLKOBufMgrWrapper::getInstance(ESensorDev_Main).initPipe(i4SensorIdx, i4TGInfo);
    else if (i4SensorDev & ESensorDev_Sub)
        return FLKOBufMgrWrapper::getInstance(ESensorDev_Sub).initPipe(i4SensorIdx, i4TGInfo);
    else if (i4SensorDev & ESensorDev_MainSecond)
        return FLKOBufMgrWrapper::getInstance(ESensorDev_MainSecond).initPipe(i4SensorIdx, i4TGInfo);

    MY_ERR("Incorrect sensor device: i4SensorDev = %d", i4SensorDev);
    return MFALSE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IFLKOBufMgrWrapper::uninitPipe(MINT32 const i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return FLKOBufMgrWrapper::getInstance(ESensorDev_Main).uninitPipe();
    else if (i4SensorDev & ESensorDev_Sub)
        return FLKOBufMgrWrapper::getInstance(ESensorDev_Sub).uninitPipe();
    else if (i4SensorDev & ESensorDev_MainSecond)
        return FLKOBufMgrWrapper::getInstance(ESensorDev_MainSecond).uninitPipe();

    MY_ERR("Incorrect sensor device: i4SensorDev = %d", i4SensorDev);
    return MFALSE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IFLKOBufMgrWrapper::startPipe(MINT32 const i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return FLKOBufMgrWrapper::getInstance(ESensorDev_Main).startPipe();
    else if (i4SensorDev & ESensorDev_Sub)
        return FLKOBufMgrWrapper::getInstance(ESensorDev_Sub).startPipe();
    else if (i4SensorDev & ESensorDev_MainSecond)
        return FLKOBufMgrWrapper::getInstance(ESensorDev_MainSecond).startPipe();

    MY_ERR("Incorrect sensor device: i4SensorDev = %d", i4SensorDev);
    return MFALSE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IFLKOBufMgrWrapper::stopPipe(MINT32 const i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return FLKOBufMgrWrapper::getInstance(ESensorDev_Main).stopPipe();
    else if (i4SensorDev & ESensorDev_Sub)
        return FLKOBufMgrWrapper::getInstance(ESensorDev_Sub).stopPipe();
    else if (i4SensorDev & ESensorDev_MainSecond)
        return FLKOBufMgrWrapper::getInstance(ESensorDev_MainSecond).stopPipe();

    MY_ERR("Incorrect sensor device: i4SensorDev = %d", i4SensorDev);
    return MFALSE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IFLKOBufMgrWrapper::dequePipe(MINT32 const i4SensorDev, BufInfo_T& rBufInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return FLKOBufMgrWrapper::getInstance(ESensorDev_Main).dequePipe(rBufInfo);
    else if (i4SensorDev & ESensorDev_Sub)
        return FLKOBufMgrWrapper::getInstance(ESensorDev_Sub).dequePipe(rBufInfo);
    else if (i4SensorDev & ESensorDev_MainSecond)
        return FLKOBufMgrWrapper::getInstance(ESensorDev_MainSecond).dequePipe(rBufInfo);

    MY_ERR("Incorrect sensor device: i4SensorDev = %d", i4SensorDev);
    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IFLKOBufMgrWrapper::enquePipe(MINT32 const i4SensorDev, BufInfo_T& rBufInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return FLKOBufMgrWrapper::getInstance(ESensorDev_Main).enquePipe(rBufInfo);
    else if (i4SensorDev & ESensorDev_Sub)
        return FLKOBufMgrWrapper::getInstance(ESensorDev_Sub).enquePipe(rBufInfo);
    else if (i4SensorDev & ESensorDev_MainSecond)
        return FLKOBufMgrWrapper::getInstance(ESensorDev_MainSecond).enquePipe(rBufInfo);

    MY_ERR("Incorrect sensor device: i4SensorDev = %d", i4SensorDev);
    return MFALSE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FLKOBufMgrWrapper& FLKOBufMgrWrapper::getInstance(MINT32 const i4SensorDev)
{
    switch (i4SensorDev)
    {
    case ESensorDev_Main: //  Main Sensor
        return  FLKOBufMgrDev<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  FLKOBufMgrDev<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  FLKOBufMgrDev<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("i4SensorDev = %d", i4SensorDev);
        return  FLKOBufMgrDev<ESensorDev_Main>::getInstance();
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FLKOBufMgrWrapper::FLKOBufMgrWrapper(ESensorDev_T const eSensorDev)
    : m_eSensorDev(eSensorDev)
    , m_eSensorTG(ESensorTG_None)
    , m_pIMemDrv(IMemDrv::createInstance())
    , m_Users(0)
    , m_Lock()
    , m_bDebugEnable(MFALSE)
    , m_i4SensorIdx(0)
    , mpIspDrv(NULL)
    , m_TgInfo(CAM_TG_ERR)
    , FLKIrq_key(0)
{}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FLKOBufMgrWrapper::~FLKOBufMgrWrapper() {}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL FLKOBufMgrWrapper::init(MINT32 const i4SensorIdx)
{
    MBOOL ret = MTRUE;

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.flko_buf_mgr.enable", value, "0");
    m_bDebugEnable = atoi(value);
    // sensor index
    m_i4SensorIdx = i4SensorIdx;

    MY_LOG("[%s()] m_eSensorDev: %d, m_i4SensorIdx: %d, m_Users: %d \n", __FUNCTION__, m_eSensorDev, m_i4SensorIdx, m_Users);

    Mutex::Autolock lock(m_Lock);

    if (m_Users > 0)
    {
        MY_LOG("[%s()] %d has created \n", __FUNCTION__, m_Users);
        android_atomic_inc(&m_Users);
        return MTRUE;
    }

    MY_LOG("[%s()]FLKO TG = %d, SensorIdx %d\n", __FUNCTION__, m_eSensorTG, m_i4SensorIdx);

    ret = m_pIMemDrv->init();
    MY_LOG("[%s()]m_pIMemDrv->init() %d\n", __FUNCTION__, ret);
    m_rHwBufList.clear();

    MY_LOG("[%s()] allocateBuf \n", __FUNCTION__);
    for(MINT32 i = 0; i < MAX_FLKO_BUFFER_CNT; i++)
    {
        m_rFLKOBufInfo[i].useNoncache = 0;
        allocateBuf(m_rFLKOBufInfo[i], FLKO_BUF_SIZE);
        enqueueHwBuf(m_rFLKOBufInfo[i]);
    }
    android_atomic_inc(&m_Users);
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL FLKOBufMgrWrapper::uninit()
{
    MBOOL ret = MTRUE;
    IMEM_BUF_INFO buf_info;

    MY_LOG("[%s()] - E. m_Users: %d \n", __FUNCTION__, m_Users);

    Mutex::Autolock lock(m_Lock);

    // If no more users, return directly and do nothing.
    if (m_Users <= 0)    return MTRUE;

    // More than one user, so decrease one User.
    android_atomic_dec(&m_Users);

    if (m_Users == 0) // There is no more User after decrease one User
    {
        MY_LOG("[%s()] freeBuf\n", __FUNCTION__);
        for (MINT32 i = 0; i < MAX_FLKO_BUFFER_CNT; i++)
        {
            freeBuf(m_rFLKOBufInfo[i]);
        }
        MY_LOG("[%s()]m_pIMemDrv uninit\n", __FUNCTION__);
        m_rHwBufList.clear();
        m_pIMemDrv->uninit();
    }
    else    // There are still some users.
    {
        MY_LOG_IF(m_bDebugEnable,"[%s()] Still %d users \n", __FUNCTION__, m_Users);
    }
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL FLKOBufMgrWrapper::debugPrint()
{
    BufInfoList_T::iterator it;
    for (it = m_rHwBufList.begin(); it != m_rHwBufList.end(); it++ )
        MY_LOG("m_rHwBufList.virtAddr:[0x%x]/phyAddr:[0x%x] \n",it->virtAddr,it->phyAddr);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL FLKOBufMgrWrapper::enqueueHwBuf(BufInfo_T& rBufInfo)
{
    MY_LOG_IF(m_bDebugEnable,"FLKO %s() m_eSensorDev(%d)\n", __FUNCTION__, m_eSensorDev);
    MY_LOG_IF(m_bDebugEnable,"FLKO rBufInfo.virtAddr:[0x%x]/phyAddr:[0x%x] \n",rBufInfo.virtAddr,rBufInfo.phyAddr);
    m_rHwBufList.push_back(rBufInfo);
    m_pIMemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_INVALID, &rBufInfo);
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL FLKOBufMgrWrapper::dequeueHwBuf(BufInfo_T& rBufInfo)
{
    MY_LOG_IF(m_bDebugEnable,"%s() m_eSensorDev(%d)\n", __FUNCTION__, m_eSensorDev);
    if (m_rHwBufList.size())
    {
        rBufInfo = m_rHwBufList.front();
        m_rHwBufList.pop_front();
    }
    MY_LOG_IF(m_bDebugEnable,"rBufInfo.virtAddr:[0x%x]/phyAddr:[0x%x] \n",rBufInfo.virtAddr,rBufInfo.phyAddr);
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32 FLKOBufMgrWrapper::getCurrHwBuf()
{
    if (m_rHwBufList.size() > 0)
    {
        return m_rHwBufList.front().phyAddr;
    }
    else
    {
        MY_ERR("FLKO No free buffer\n");
        return 0;
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32 FLKOBufMgrWrapper::getNextHwBuf()
{
    BufInfoList_T::iterator it;
    if (m_rHwBufList.size() > 1)
    {
        it = m_rHwBufList.begin();
        it++;
        MY_LOG_IF(m_bDebugEnable,"getNextHwBuf : it->virtAddr:[0x%x]/phyAddr:[0x%x] \n",it->virtAddr,it->phyAddr);
        return it->phyAddr;
    }
    else
    {
        // No free buffer
        MY_ERR("FLKO No free buffer\n");
        return 0;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL FLKOBufMgrWrapper::allocateBuf(BufInfo_T &rBufInfo, MUINT32 u4BufSize)
{
    rBufInfo.size = u4BufSize;
    MY_LOG("FLKO allocVirtBuf size %d",u4BufSize);
    if (m_pIMemDrv->allocVirtBuf(&rBufInfo))
    {
        MY_ERR("m_pIMemDrv->allocVirtBuf() error");
        return MFALSE;
    }
    if (m_pIMemDrv->mapPhyAddr(&rBufInfo))
    {
        MY_ERR("m_pIMemDrv->mapPhyAddr() error");
        return MFALSE;
    }
    return MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL FLKOBufMgrWrapper::freeBuf(BufInfo_T &rBufInfo)
{
    if (m_pIMemDrv->unmapPhyAddr(&rBufInfo))
    {
        MY_ERR("m_pIMemDrv->unmapPhyAddr() error");
        return MFALSE;
    }
    if (m_pIMemDrv->freeVirtBuf(&rBufInfo))
    {
        MY_ERR("m_pIMemDrv->freeVirtBuf() error");
        return MFALSE;
    }
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL FLKOBufMgrWrapper::updateDMABaseAddr(MUINT32 u4BaseAddr)
{
    MY_LOG_IF(m_bDebugEnable,"FLKO %s() m_eSensorDev(%d) u4BaseAddr=0x%x\n", __FUNCTION__, m_eSensorDev, u4BaseAddr);

    if(!u4BaseAddr)
    {
        MY_ERR("u4BaseAddr is NULL\n");
        return E_ISPMGR_NULL_ADDRESS;
    }
    MUINTPTR handle = NULL;

    if (m_eSensorTG == ESensorTG_1)
    {
        Mutex::Autolock lock(m_FLKOLockReg);
        IOPIPE_GET_MODUL_HANDLE(NSImageioIsp3::NSIspio::EModule_ESFKO, handle);
        if (NULL != handle)
        {
            IOPIPE_SET_MODUL_REG(handle, CAM_ESFKO_BASE_ADDR,  u4BaseAddr);
            IOPIPE_SET_MODUL_REG(handle, CAM_ESFKO_OFST_ADDR,  0);
            IOPIPE_SET_MODUL_CFG_DONE(handle);
            IOPIPE_RELEASE_MODUL_HANDLE(handle);
        }
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL FLKOBufMgrWrapper::DMAInit(MBOOL En)
{
    MY_LOG_IF(m_bDebugEnable,"FLKO %s() m_eSensorDev(%d)\n", __FUNCTION__, m_eSensorDev);
    if (En)
    {
        /* MAX_FLKO_BUFFER_CNT > 1 , getNextHwBuf
           MAX_FLKO_BUFFER_CNT = 1 , getCurrHwBuf
        */
        updateDMABaseAddr(getNextHwBuf());
    }

    if (m_eSensorTG == ESensorTG_1)
    {
        IOPIPE_SET_MODUL_ENABLE(NSImageioIsp3::NSIspio::EModule_ESFKO, En);
    }

    if (mpIspDrv == NULL)
    {
        mpIspDrv = IspDrv::createInstance();

        if (!mpIspDrv) {
            CAM_LOGE("IspDrv::createInstance() fail \n");
            return MFALSE;
        }

        if (mpIspDrv->init("FLKIrq") < 0) {
            CAM_LOGE("pIspDrv->init() fail \n");
            return MFALSE;
        }

        FLKIrq_key = mpIspDrv->registerIrq(LOG_TAG);
    }
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL FLKOBufMgrWrapper::DMAUninit()
{
    MY_LOG_IF(m_bDebugEnable,"FLKO %s() m_eSensorDev(%d)\n", __FUNCTION__, m_eSensorDev);

    if (m_eSensorTG == ESensorTG_1)
    {
        IOPIPE_SET_MODUL_ENABLE(NSImageioIsp3::NSIspio::EModule_ESFKO, MFALSE);
    }

    /* flushFLKirq */
    if ((m_TgInfo != CAM_TG_1) && (m_TgInfo != CAM_TG_2))
    {
        CAM_LOGE("m_TgInfo = %d, Suggestion: Need to call queryTGInfoFromSensorHal() before waitVSirq().", m_TgInfo);
        return MFALSE;
    }
    ISP_DRV_WAIT_IRQ_STRUCT waitIrq;
    waitIrq.UserInfo.Type = (m_TgInfo == CAM_TG_1) ?
        ISP_DRV_IRQ_TYPE_INT_P1_ST : ISP_DRV_IRQ_TYPE_INT_P1_ST_D;
    waitIrq.UserInfo.Status = CAM_CTL_INT_P1_STATUS_FLK_DON_ST;
    waitIrq.UserInfo.UserKey = FLKIrq_key;

    if (mpIspDrv == NULL)
    {
        CAM_LOGE("isp drv = NULL");
    }
    else
    {
        mpIspDrv->flushIrq(waitIrq);
    }

    if (mpIspDrv)
    {
        mpIspDrv->uninit("FLKIrq");
        mpIspDrv->destroyInstance();
        mpIspDrv = NULL;
    }

    return MTRUE;
}


////////////////////////////////////////////////////////////////////////////

MBOOL FLKOBufMgrWrapper::initPipe(MINT32 const i4SensorIdx, MINT32 const i4TGInfo)
{
    init(i4SensorIdx);
    setTGInfo(i4TGInfo);

    return MTRUE;
}

MBOOL FLKOBufMgrWrapper::uninitPipe()
{
    uninit();

    return MTRUE;
}

MBOOL FLKOBufMgrWrapper::startPipe()
{
    DMAInit(MTRUE);

    return MTRUE;
}

MBOOL FLKOBufMgrWrapper::stopPipe()
{
    DMAUninit();

    return MTRUE;
}

MBOOL FLKOBufMgrWrapper::dequePipe(BufInfo_T& rBufInfo)
{
    // FLK irq
    MY_LOG_IF(m_bDebugEnable,"FLKO %s() m_eSensorDev(%d)\n", __FUNCTION__, m_eSensorDev);

    dequeueHwBuf(rBufInfo);
    updateDMABaseAddr(getNextHwBuf());

    return MFALSE;
}

MBOOL FLKOBufMgrWrapper::enquePipe(BufInfo_T& rBufInfo)
{
    MY_LOG_IF(m_bDebugEnable,"enquePipe : m_rHwBuf.virtAddr:[0x%x]/phyAddr:[0x%x] \n",m_rHwBuf.virtAddr,m_rHwBuf.phyAddr);
    enqueueHwBuf(rBufInfo);
    return MTRUE;
}


