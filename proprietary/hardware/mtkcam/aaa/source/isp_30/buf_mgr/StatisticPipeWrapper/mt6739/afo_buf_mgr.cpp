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
#define LOG_TAG "afo_buf_mgr"

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
#include <debug/DebugUtil.h>

#include <iopipe/CamIO/INormalPipe.h>
#include "afo_buf_mgr.h"

using namespace std;
using namespace android;
using namespace NS3Av3;
using namespace NSIspTuning;
using namespace NSCam::NSIoPipeIsp3::NSCamIOPipe;

typedef list<BufInfo_T> BufInfoList_T;
/*******************************************************************************
*  AFO buffer
*******************************************************************************/
#if 0
#define AF_HW_WIN 36
#define AF_HW_FLOWIN 3
#define AF_WIN_DATA 8
#define AF_FLOWIN_DATA 12
#define AFO_BUFFER_SIZE (AF_WIN_DATA * AF_HW_WIN + AF_FLOWIN_DATA * AF_HW_FLOWIN)
#define AFO_XSIZE (AFO_BUFFER_SIZE-1)
#define MAX_AFO_BUFFER_CNT (1)
#else
// AF in ISP4.0
#define AFO_WIN_NUM (128)
#define AFO_WIN_DATA (32)    // byte
#define AFO_STRIDE_NUM (1)
#define AFO_BUFFER_SIZE ((AFO_WIN_NUM + AFO_STRIDE_NUM) * AFO_WIN_NUM * AFO_WIN_DATA)
#define MAX_AFO_BUFFER_CNT (1)
#endif

#if (MAX_AFO_BUFFER_CNT == 1)
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
                                                  if (MTRUE != sendCommandNormalPipe(NSImageioIsp3::NSIspio::EPIPECmd_GET_MODULE_HANDLE, module, (MINTPTR)&handle, (MINTPTR)(&("AFOBufMgrWrapper::DMAConfig()")))) \
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
                                                  handle = (MUINTPTR)NULL; \
                                                }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class AFOBufMgrWrapper
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    //  Copy constructor is disallowed.
    AFOBufMgrWrapper(AFOBufMgrWrapper const&);
    //  Copy-assignment operator is disallowed.
    AFOBufMgrWrapper& operator=(AFOBufMgrWrapper const&);

public:
    AFOBufMgrWrapper(ESensorDev_T const eSensorDev);
    ~AFOBufMgrWrapper();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static AFOBufMgrWrapper& getInstance(MINT32 const i4SensorDev);
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
    MBOOL AFStatEnable(MBOOL En);

    /* ISP 4.x */
    BufInfo_T m_rBufInfo;
    MBOOL initPipe(MINT32 const i4SensorIdx, MINT32 const i4TGInfo);     // create Buffer
    MBOOL uninitPipe();
    MBOOL startPipe();
    MBOOL stopPipe();
    MBOOL dequePipe();
    BufInfo_T* dequeSwPipe();
    MBOOL enquePipe();
    MBOOL abortDeque();
    //BOOL configPipe();

    inline MBOOL sendCommandNormalPipe(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3)
    {
        INormalPipe_FrmB* pPipe = INormalPipe_FrmB::createInstance(m_i4SensorIdx, "afo_buf_mgr");
        MBOOL fgRet = pPipe->sendCommand(cmd, arg1, arg2, arg3);
        pPipe->destroyInstance("afo_buf_mgr");
        return fgRet;
    }

    inline MBOOL setTGInfo(MINT32 const i4TGInfo)
    {
        m_TgInfo = i4TGInfo;
        MY_LOG("[%s()]i4TGInfo: %d\n", __FUNCTION__, m_TgInfo);
        switch (m_TgInfo)
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
    BufInfo_T              m_rAFOBufInfo[2];

    /* AF irq */
    IspDrv*                mpIspDrv;
    MUINT                  m_TgInfo;
    MINT32                 AFIrq_key;
    MINT32                 m_i4AFDone;
    MINT32                 m_i4AFOInitDone;

    /* ISP 4.x */
    BufInfo_T              m_rHwBuf;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
template <ESensorDev_T const eSensorDev>
class AFOBufMgrDev : public AFOBufMgrWrapper
{
public:
    static AFOBufMgrWrapper& getInstance()
    {
        static AFOBufMgrDev<eSensorDev> singleton;
        return singleton;
    }

    AFOBufMgrDev(): AFOBufMgrWrapper(eSensorDev) {}
    virtual ~AFOBufMgrDev() {}
};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAFOBufMgrWrapper::init(MINT32 const i4SensorDev, MINT32 const i4SensorIdx)
{
    MBOOL ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AFOBufMgrWrapper::getInstance(ESensorDev_Main).init(i4SensorIdx);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AFOBufMgrWrapper::getInstance(ESensorDev_Sub).init(i4SensorIdx);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AFOBufMgrWrapper::getInstance(ESensorDev_MainSecond).init(i4SensorIdx);

    return ret_main && ret_sub && ret_main2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAFOBufMgrWrapper::uninit(MINT32 const i4SensorDev)
{
    MBOOL ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AFOBufMgrWrapper::getInstance(ESensorDev_Main).uninit();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AFOBufMgrWrapper::getInstance(ESensorDev_Sub).uninit();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AFOBufMgrWrapper::getInstance(ESensorDev_MainSecond).uninit();

    return ret_main && ret_sub && ret_main2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAFOBufMgrWrapper::DMAInit(MINT32 const i4SensorDev)
{
    MBOOL ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AFOBufMgrWrapper::getInstance(ESensorDev_Main).DMAInit(MTRUE);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AFOBufMgrWrapper::getInstance(ESensorDev_Sub).DMAInit(MTRUE);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AFOBufMgrWrapper::getInstance(ESensorDev_MainSecond).DMAInit(MTRUE);

    return ret_main && ret_sub && ret_main2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAFOBufMgrWrapper::DMAUninit(MINT32 const i4SensorDev)
{
    MBOOL ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AFOBufMgrWrapper::getInstance(ESensorDev_Main).DMAUninit();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AFOBufMgrWrapper::getInstance(ESensorDev_Sub).DMAUninit();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AFOBufMgrWrapper::getInstance(ESensorDev_MainSecond).DMAUninit();

    return ret_main && ret_sub && ret_main2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAFOBufMgrWrapper::AFStatEnable(MINT32 const i4SensorDev, MBOOL En)
{
    MBOOL ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AFOBufMgrWrapper::getInstance(ESensorDev_Main).AFStatEnable(En);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AFOBufMgrWrapper::getInstance(ESensorDev_Sub).AFStatEnable(En);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AFOBufMgrWrapper::getInstance(ESensorDev_MainSecond).AFStatEnable(En);

    return ret_main && ret_sub && ret_main2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAFOBufMgrWrapper::enqueueHwBuf(MINT32 const i4SensorDev, BufInfo_T& rBufInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return AFOBufMgrWrapper::getInstance(ESensorDev_Main).enqueueHwBuf(rBufInfo);
    else if (i4SensorDev & ESensorDev_Sub)
        return AFOBufMgrWrapper::getInstance(ESensorDev_Sub).enqueueHwBuf(rBufInfo);
    else if (i4SensorDev & ESensorDev_MainSecond)
        return AFOBufMgrWrapper::getInstance(ESensorDev_MainSecond).enqueueHwBuf(rBufInfo);

    MY_ERR("Incorrect sensor device: i4SensorDev = %d", i4SensorDev);
    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAFOBufMgrWrapper::dequeueHwBuf(MINT32 const i4SensorDev, BufInfo_T& rBufInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return AFOBufMgrWrapper::getInstance(ESensorDev_Main).dequeueHwBuf(rBufInfo);
    else if (i4SensorDev & ESensorDev_Sub)
        return AFOBufMgrWrapper::getInstance(ESensorDev_Sub).dequeueHwBuf(rBufInfo);
    else if (i4SensorDev & ESensorDev_MainSecond)
        return AFOBufMgrWrapper::getInstance(ESensorDev_MainSecond).dequeueHwBuf(rBufInfo);

    MY_ERR("Incorrect sensor device: i4SensorDev = %d", i4SensorDev);
    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAFOBufMgrWrapper::updateDMABaseAddr(MINT32 const i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AFOBufMgrWrapper::getInstance(ESensorDev_Main).updateDMABaseAddr(AFOBufMgrWrapper::getInstance(ESensorDev_Main).getNextHwBuf());
    else if (i4SensorDev & ESensorDev_Sub)
        return AFOBufMgrWrapper::getInstance(ESensorDev_Sub).updateDMABaseAddr(AFOBufMgrWrapper::getInstance(ESensorDev_Sub).getNextHwBuf());
    else if (i4SensorDev & ESensorDev_MainSecond)
        return AFOBufMgrWrapper::getInstance(ESensorDev_MainSecond).updateDMABaseAddr(AFOBufMgrWrapper::getInstance(ESensorDev_MainSecond).getNextHwBuf());

    MY_ERR("Incorrect sensor device: i4SensorDev = %d", i4SensorDev);
    return MFALSE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAFOBufMgrWrapper::setTGInfo(MINT32 const i4SensorDev, MINT32 const i4TGInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return AFOBufMgrWrapper::getInstance(ESensorDev_Main).setTGInfo(i4TGInfo);
    else if (i4SensorDev & ESensorDev_Sub)
        return AFOBufMgrWrapper::getInstance(ESensorDev_Sub).setTGInfo(i4TGInfo);
    else if (i4SensorDev & ESensorDev_MainSecond)
        return AFOBufMgrWrapper::getInstance(ESensorDev_MainSecond).setTGInfo(i4TGInfo);

    MY_ERR("Incorrect sensor device: i4SensorDev = %d", i4SensorDev);
    return MFALSE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                               ISP 3.0 Wrapper
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAFOBufMgrWrapper::initPipe(MINT32 const i4SensorDev, MINT32 const i4SensorIdx, MINT32 const i4TGInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return AFOBufMgrWrapper::getInstance(ESensorDev_Main).initPipe(i4SensorIdx, i4TGInfo);
    else if (i4SensorDev & ESensorDev_Sub)
        return AFOBufMgrWrapper::getInstance(ESensorDev_Sub).initPipe(i4SensorIdx, i4TGInfo);
    else if (i4SensorDev & ESensorDev_MainSecond)
        return AFOBufMgrWrapper::getInstance(ESensorDev_MainSecond).initPipe(i4SensorIdx, i4TGInfo);

    MY_ERR("Incorrect sensor device: i4SensorDev = %d", i4SensorDev);
    return MFALSE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAFOBufMgrWrapper::uninitPipe(MINT32 const i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AFOBufMgrWrapper::getInstance(ESensorDev_Main).uninitPipe();
    else if (i4SensorDev & ESensorDev_Sub)
        return AFOBufMgrWrapper::getInstance(ESensorDev_Sub).uninitPipe();
    else if (i4SensorDev & ESensorDev_MainSecond)
        return AFOBufMgrWrapper::getInstance(ESensorDev_MainSecond).uninitPipe();

    MY_ERR("Incorrect sensor device: i4SensorDev = %d", i4SensorDev);
    return MFALSE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAFOBufMgrWrapper::startPipe(MINT32 const i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AFOBufMgrWrapper::getInstance(ESensorDev_Main).startPipe();
    else if (i4SensorDev & ESensorDev_Sub)
        return AFOBufMgrWrapper::getInstance(ESensorDev_Sub).startPipe();
    else if (i4SensorDev & ESensorDev_MainSecond)
        return AFOBufMgrWrapper::getInstance(ESensorDev_MainSecond).startPipe();

    MY_ERR("Incorrect sensor device: i4SensorDev = %d", i4SensorDev);
    return MFALSE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAFOBufMgrWrapper::stopPipe(MINT32 const i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AFOBufMgrWrapper::getInstance(ESensorDev_Main).stopPipe();
    else if (i4SensorDev & ESensorDev_Sub)
        return AFOBufMgrWrapper::getInstance(ESensorDev_Sub).stopPipe();
    else if (i4SensorDev & ESensorDev_MainSecond)
        return AFOBufMgrWrapper::getInstance(ESensorDev_MainSecond).stopPipe();

    MY_ERR("Incorrect sensor device: i4SensorDev = %d", i4SensorDev);
    return MFALSE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAFOBufMgrWrapper::dequePipe(MINT32 const i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AFOBufMgrWrapper::getInstance(ESensorDev_Main).dequePipe();
    else if (i4SensorDev & ESensorDev_Sub)
        return AFOBufMgrWrapper::getInstance(ESensorDev_Sub).dequePipe();
    else if (i4SensorDev & ESensorDev_MainSecond)
        return AFOBufMgrWrapper::getInstance(ESensorDev_MainSecond).dequePipe();

    MY_ERR("Incorrect sensor device: i4SensorDev = %d", i4SensorDev);
    return MFALSE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
BufInfo_T* IAFOBufMgrWrapper::dequeSwPipe(MINT32 const i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AFOBufMgrWrapper::getInstance(ESensorDev_Main).dequeSwPipe();
    else if (i4SensorDev & ESensorDev_Sub)
        return AFOBufMgrWrapper::getInstance(ESensorDev_Sub).dequeSwPipe();
    else if (i4SensorDev & ESensorDev_MainSecond)
        return AFOBufMgrWrapper::getInstance(ESensorDev_MainSecond).dequeSwPipe();

    MY_ERR("Incorrect sensor device: i4SensorDev = %d", i4SensorDev);
    return NULL;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAFOBufMgrWrapper::enquePipe(MINT32 const i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AFOBufMgrWrapper::getInstance(ESensorDev_Main).enquePipe();
    else if (i4SensorDev & ESensorDev_Sub)
        return AFOBufMgrWrapper::getInstance(ESensorDev_Sub).enquePipe();
    else if (i4SensorDev & ESensorDev_MainSecond)
        return AFOBufMgrWrapper::getInstance(ESensorDev_MainSecond).enquePipe();

    MY_ERR("Incorrect sensor device: i4SensorDev = %d", i4SensorDev);
    return MFALSE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAFOBufMgrWrapper::abortDeque(MINT32 const i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AFOBufMgrWrapper::getInstance(ESensorDev_Main).abortDeque();
    else if (i4SensorDev & ESensorDev_Sub)
        return AFOBufMgrWrapper::getInstance(ESensorDev_Sub).abortDeque();
    else if (i4SensorDev & ESensorDev_MainSecond)
        return AFOBufMgrWrapper::getInstance(ESensorDev_MainSecond).abortDeque();

    MY_ERR("Incorrect sensor device: i4SensorDev = %d", i4SensorDev);
    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AFOBufMgrWrapper& AFOBufMgrWrapper::getInstance(MINT32 const i4SensorDev)
{
    switch (i4SensorDev)
    {
    case ESensorDev_Main: //  Main Sensor
        return  AFOBufMgrDev<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  AFOBufMgrDev<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  AFOBufMgrDev<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("i4SensorDev = %d", i4SensorDev);
        return  AFOBufMgrDev<ESensorDev_Main>::getInstance();
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AFOBufMgrWrapper::AFOBufMgrWrapper(ESensorDev_T const eSensorDev)
    : m_eSensorDev(eSensorDev)
    , m_eSensorTG(ESensorTG_None)
    , m_pIMemDrv(IMemDrv::createInstance())
    , m_Users(0)
    , m_Lock()
    , m_bDebugEnable(MFALSE)
    , m_i4SensorIdx(0)
    , mpIspDrv(NULL)
    , m_TgInfo(CAM_TG_ERR)
    , m_i4AFDone(0)
    , m_i4AFOInitDone(0)
{
    AFIrq_key = 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AFOBufMgrWrapper::~AFOBufMgrWrapper() {}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AFOBufMgrWrapper::init(MINT32 const i4SensorIdx)
{
    MBOOL ret = MTRUE;

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.afo_buf_mgr.enable", value, "0");
    m_bDebugEnable = atoi(value);
    // sensor index
    m_i4SensorIdx = i4SensorIdx;

    MY_LOG("[%s()] m_eSensorDev: %d, m_i4SensorIdx: %d, m_Users: %d \n", __FUNCTION__, m_eSensorDev, m_i4SensorIdx, m_Users);

    Mutex::Autolock lock(m_Lock);

    if (m_Users > 0)
    {
        MY_LOG("%d has created \n", m_Users);
        android_atomic_inc(&m_Users);
        return MTRUE;
    }

    MY_LOG("AFO TG = %d, SensorIdx %d\n", m_eSensorTG, m_i4SensorIdx);

    ret = m_pIMemDrv->init();
    MY_LOG("m_pIMemDrv->init() %d\n", ret);
    m_rHwBufList.clear();

    MY_LOG("[AFOBufMgrWrapper] init - allocateBuf \n");
    MBOOL DebugEnable = m_bDebugEnable;
    m_bDebugEnable = 1;
    for(MINT32 i = 0; i < MAX_AFO_BUFFER_CNT; i++)
    {
        m_rAFOBufInfo[i].useNoncache = 0;
        allocateBuf(m_rAFOBufInfo[i], AFO_BUFFER_SIZE);
        enqueueHwBuf(m_rAFOBufInfo[i]);
    }
    m_bDebugEnable = DebugEnable;
    android_atomic_inc(&m_Users);
    m_i4AFOInitDone = 1;
    MY_LOG("[%s()] - AFOInitDone(%d)\n", __FUNCTION__, m_i4AFOInitDone);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AFOBufMgrWrapper::uninit()
{
    IMEM_BUF_INFO buf_info;

    MY_LOG("[%s()] - E. m_Users: %d \n", __FUNCTION__, m_Users);

    Mutex::Autolock lock(m_Lock);

    // If no more users, return directly and do nothing.
    if (m_Users <= 0)    return MTRUE;

    // More than one user, so decrease one User.
    android_atomic_dec(&m_Users);

    if (m_Users == 0) // There is no more User after decrease one User
    {
        MY_LOG("[AFOBufMgrWrapper] freeBuf (%d)\n", m_i4AFOInitDone);
        for (MINT32 i = 0; i < MAX_AFO_BUFFER_CNT; i++)
        {
            freeBuf(m_rAFOBufInfo[i]);
        }
        m_rHwBufList.clear();
        m_pIMemDrv->uninit();
        m_i4AFOInitDone = 0;
        MY_LOG("[AFOBufMgrWrapper]m_pIMemDrv uninit (%d)\n", m_i4AFOInitDone);
    }
    else    // There are still some users.
    {
        MY_LOG_IF(m_bDebugEnable,"Still %d users \n", m_Users);
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AFOBufMgrWrapper::debugPrint()
{
    BufInfoList_T::iterator it;
    for (it = m_rHwBufList.begin(); it != m_rHwBufList.end(); it++ )
        MY_LOG("m_rHwBufList.virtAddr:[0x%x]/phyAddr:[0x%x] \n",it->virtAddr,it->phyAddr);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AFOBufMgrWrapper::enqueueHwBuf(BufInfo_T& rBufInfo)
{
    MY_LOG_IF(m_bDebugEnable,"[%s()] Dev(%d) rBufInfo.virtAddr:[0x%x]/phyAddr:[0x%x]\n", __FUNCTION__, m_eSensorDev, rBufInfo.virtAddr,rBufInfo.phyAddr);
    m_rHwBufList.push_back(rBufInfo);
    m_pIMemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_INVALID, &rBufInfo);
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AFOBufMgrWrapper::dequeueHwBuf(BufInfo_T& rBufInfo)
{
    if (m_rHwBufList.size())
    {
        rBufInfo = m_rHwBufList.front();
        m_rHwBufList.pop_front();
    }
    MY_LOG_IF(m_bDebugEnable,"[%s] Dev(%d), rBufInfo.virtAddr:[0x%x]/phyAddr:[0x%x]\n", __FUNCTION__, m_eSensorDev, rBufInfo.virtAddr, rBufInfo.phyAddr);
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32 AFOBufMgrWrapper::getCurrHwBuf()
{
    if (m_rHwBufList.size() > 0)
    {
        return m_rHwBufList.front().phyAddr;
    }
    else
    {
        MY_ERR("AFO No free buffer\n");
        return 0;
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32 AFOBufMgrWrapper::getNextHwBuf()
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
        MY_ERR("AFO No free buffer\n");
        return 0;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AFOBufMgrWrapper::allocateBuf(BufInfo_T &rBufInfo, MUINT32 u4BufSize)
{
    rBufInfo.size = u4BufSize;
    MY_LOG("AFO allocVirtBuf size %d",u4BufSize);
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
MBOOL AFOBufMgrWrapper::freeBuf(BufInfo_T &rBufInfo)
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
MBOOL AFOBufMgrWrapper::updateDMABaseAddr(MUINT32 u4BaseAddr)
{
    MY_LOG_IF(m_bDebugEnable,"[%s()] Dev(%d) u4BaseAddr=0x%x\n", __FUNCTION__, m_eSensorDev, u4BaseAddr);

    if(!u4BaseAddr)
    {
        MY_ERR("u4BaseAddr is NULL\n");
        return E_ISPMGR_NULL_ADDRESS;
    }
    MUINTPTR handle = (MUINTPTR)NULL;

    if (m_eSensorTG == ESensorTG_1)
    {
        IOPIPE_GET_MODUL_HANDLE(NSImageioIsp3::NSIspio::EModule_AFO_D, handle);
        if ((MUINTPTR)NULL != handle)
        {
            IOPIPE_SET_MODUL_REG(handle, CAM_AFO_D_BASE_ADDR,  u4BaseAddr);
            IOPIPE_SET_MODUL_REG(handle, CAM_AFO_D_OFST_ADDR,  0);
            IOPIPE_SET_MODUL_CFG_DONE(handle);
            IOPIPE_RELEASE_MODUL_HANDLE(handle);
        }
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AFOBufMgrWrapper::DMAInit(MBOOL En)
{
    MY_LOG("[%s()] Dev(%d), AFOInitDone(%d)\n", __FUNCTION__, m_eSensorDev, m_i4AFOInitDone);
    if (En)
    {
        MBOOL DebugEnable = m_bDebugEnable;
        m_bDebugEnable = MTRUE;
        updateDMABaseAddr(getCurrHwBuf());
        m_bDebugEnable = DebugEnable;
    }

    if (m_eSensorTG == ESensorTG_1)
    {
        IOPIPE_SET_MODUL_ENABLE(NSImageioIsp3::NSIspio::EModule_AFO_D, En);

        if (MFALSE==sendCommandNormalPipe(NSImageioIsp3::NSIspio::EPIPECmd_SET_MODULE_SEL, EPipe_P1Sel_SGG_EN, 1,-1))
        {
            CAM_LOGE("EPipe_P1Sel_SGG_EN fail");
        }
        if (MFALSE==sendCommandNormalPipe(NSImageioIsp3::NSIspio::EPIPECmd_SET_MODULE_SEL, EPipe_P1Sel_SGG, 1,-1))
        {
            CAM_LOGE("EPipe_P1Sel_SGG  fail");
        }
    }

    if (mpIspDrv == NULL)
    {
        mpIspDrv = IspDrv::createInstance();

        if (!mpIspDrv) {
            m_i4AFOInitDone = 0;
            CAM_LOGE("IspDrv::createInstance() fail \n");
            return MFALSE;
        }

        if (mpIspDrv->init("AFIrq") < 0) {
            m_i4AFOInitDone = 0;
            CAM_LOGE("pIspDrv->init() fail \n");
            return MFALSE;
        }

        AFIrq_key = mpIspDrv->registerIrq(LOG_TAG);
    }
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AFOBufMgrWrapper::DMAUninit()
{
    MY_LOG("[%s()] Dev(%d), AFOInitDone(%d)\n", __FUNCTION__, m_eSensorDev, m_i4AFOInitDone);

    if (m_eSensorTG == ESensorTG_1)
    {
        IOPIPE_SET_MODUL_ENABLE(NSImageioIsp3::NSIspio::EModule_AFO_D, MFALSE);
    }

    /* flushAFirq */
    if ((m_TgInfo != CAM_TG_1) && (m_TgInfo != CAM_TG_2))
    {
        CAM_LOGE("m_TgInfo = %d, Suggestion: Need to call queryTGInfoFromSensorHal() before waitVSirq().", m_TgInfo);
        return MFALSE;
    }
    ISP_DRV_WAIT_IRQ_STRUCT waitIrq;
    waitIrq.UserInfo.Type = ISP_DRV_IRQ_TYPE_INT_P1_ST_D;
    waitIrq.UserInfo.Status = CAM_CTL_INT_P1_STATUS_D_AF_DON_ST;
    waitIrq.UserInfo.UserKey = AFIrq_key;

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
        mpIspDrv->uninit("AFIrq");
        mpIspDrv->destroyInstance();
        mpIspDrv = NULL;
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AFOBufMgrWrapper::AFStatEnable(MBOOL En)
{
    MY_LOG_IF(m_bDebugEnable,"AFO m_eSensorDev(%d) AFStatEnable(%d)\n",m_eSensorDev, En);

    return MTRUE;
}

////////////////////////////////////////////////////////////////////////////

MBOOL AFOBufMgrWrapper::initPipe(MINT32 const i4SensorIdx, MINT32 const i4TGInfo)
{
    init(i4SensorIdx);
    setTGInfo(i4TGInfo);

    return MTRUE;
}

MBOOL AFOBufMgrWrapper::uninitPipe()
{
    uninit();

    return MTRUE;
}

MBOOL AFOBufMgrWrapper::startPipe()
{
    DMAInit(MTRUE);

    return MTRUE;
}

MBOOL AFOBufMgrWrapper::stopPipe()
{
    DMAUninit();

    return MTRUE;
}

MBOOL AFOBufMgrWrapper::dequePipe()
{
    // AF irq
    MY_LOG_IF(m_bDebugEnable,"[%s()] Dev(%d) +\n", __FUNCTION__, m_eSensorDev);

    if (m_i4AFOInitDone == 0)
        return MFALSE;

    ISP_DRV_WAIT_IRQ_STRUCT waitIrq;

    if ((m_TgInfo != CAM_TG_1) && (m_TgInfo != CAM_TG_2))
    {
        MY_ERR("m_TgInfo = %d, AFOIrq err", m_TgInfo);
        waitIrq.Clear = ISP_DRV_IRQ_CLEAR_WAIT;
        waitIrq.UserInfo.Type = ISP_DRV_IRQ_TYPE_INT_P1_ST;
        waitIrq.UserInfo.Status = CAM_CTL_INT_P1_STATUS_AF_DON_ST;
        waitIrq.UserInfo.UserKey = AFIrq_key;
        waitIrq.Timeout = 500;
    }
    else
    {
        waitIrq.Clear = ISP_DRV_IRQ_CLEAR_WAIT;
        waitIrq.UserInfo.Type = ISP_DRV_IRQ_TYPE_INT_P1_ST_D;
        waitIrq.UserInfo.Status = CAM_CTL_INT_P1_STATUS_D_AF_DON_ST;
        waitIrq.UserInfo.UserKey = AFIrq_key;
        waitIrq.Timeout = 500;
    }

    if ( mpIspDrv->waitIrq(&waitIrq) > 0)
    {
        // Apaptive Compensation
        struct timespec t;
        t.tv_sec = t.tv_nsec = 0;
        clock_gettime(CLOCK_MONOTONIC, &t);
        MUINT64 timestamp =(MUINT64)(((t.tv_sec) * 1000000000LL + t.tv_nsec)/1000);
        DebugUtil::getInstance(m_eSensorDev)->TS_AFDone = timestamp;   // (unit:us)

        dequeueHwBuf(m_rHwBuf);
        #if (MAX_AFO_BUFFER_CNT > 1)
        updateDMABaseAddr(getCurrHwBuf());
        #endif
        m_i4AFDone = 1;
        MY_LOG_IF(m_bDebugEnable, "[%s()] Dev(%d), m_rHwBuf.virtAddr:[0x%x]/phyAddr:[0x%x] -\n", __FUNCTION__, m_eSensorDev, m_rHwBuf.virtAddr,m_rHwBuf.phyAddr);

        return MTRUE;
    }

    MY_LOG_IF(m_bDebugEnable, "[%s()] Dev(%d) -\n", __FUNCTION__, m_eSensorDev);

    return MFALSE;
}

BufInfo_T* AFOBufMgrWrapper::dequeSwPipe()
{
    MY_LOG_IF(m_bDebugEnable,"[%s()] Dev(%d) m_rHwBuf.virtAddr:[0x%x]/phyAddr:[0x%x]\n", __FUNCTION__, m_eSensorDev, m_rHwBuf.virtAddr, m_rHwBuf.phyAddr);
    if (m_i4AFDone == 1)
    {
        m_i4AFDone = 0;
        return &m_rHwBuf;
    }
    else
    {
        return NULL;
    }
}

MBOOL AFOBufMgrWrapper::enquePipe()
{
    MY_LOG_IF(m_bDebugEnable,"[%s()] Dev(%d) m_rHwBuf.virtAddr:[0x%x]/phyAddr:[0x%x]\n", __FUNCTION__, m_eSensorDev, m_rHwBuf.virtAddr, m_rHwBuf.phyAddr);
    enqueueHwBuf(m_rHwBuf);
    return MTRUE;
}

MBOOL AFOBufMgrWrapper::abortDeque()
{
    MY_LOG_IF(m_bDebugEnable,"[%s()] Dev(%d) \n", __FUNCTION__, m_eSensorDev);

    /* flushAFirq */
    if ((m_TgInfo != CAM_TG_1) && (m_TgInfo != CAM_TG_2))
    {
        CAM_LOGE("m_TgInfo = %d, Suggestion: Need to call queryTGInfoFromSensorHal() before waitVSirq().", m_TgInfo);
        return MFALSE;
    }
    ISP_DRV_WAIT_IRQ_STRUCT waitIrq;
    waitIrq.UserInfo.Type = ISP_DRV_IRQ_TYPE_INT_P1_ST_D;
    waitIrq.UserInfo.Status = CAM_CTL_INT_P1_STATUS_D_AF_DON_ST;
    waitIrq.UserInfo.UserKey = AFIrq_key;

    if (mpIspDrv == NULL)
    {
        CAM_LOGE("isp drv = NULL");
    }
    else
    {
        mpIspDrv->flushIrq(waitIrq);
    }

    return MTRUE;
}

