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
#define LOG_TAG "aao_buf_mgr_wrapper"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/atomic.h>
#include <sys/stat.h>
#include <cutils/properties.h>
#include <string.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include <camera_custom_nvram.h>
#include <awb_feature.h>
#include <awb_param.h>
#include <awb_tuning_custom.h>
#include <ispdrv_mgr.h>
#include <isp_tuning.h>
#include <utils/threads.h>
#include <list>
#include <iopipe/CamIO/INormalPipe.h>
#include "aao_buf_mgr.h"

using namespace std;
using namespace android;
using namespace NS3Av3;
using namespace NSIspTuning;
using namespace NSCam::NSIoPipeIsp3::NSCamIOPipe;

typedef list<BufInfo_T> BufInfoList_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  AAO buffer
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define AAO_OFFSET_ADDR (0)
#define AAO_YSIZE (0)
#define AAO_STRIDE_BUS_SIZE (3)

#define MAX_AAO_BUFFER_CNT (4)

#define IOPIPE_SET_MODUL_ENABLE(module, enable) \
        { \
            if (MTRUE != sendCommandNormalPipe(NSImageioIsp3::NSIspio::EPIPECmd_SET_MODULE_EN, module, enable, MNULL)) \
            { \
                CAM_LOGE("EPIPECmd_SET_MODULE_EN ConfigHWReg fail"); \
            } \
        }
#define IOPIPE_GET_MODUL_HANDLE(module, handle) \
        { \
        if (MTRUE != sendCommandNormalPipe(NSImageioIsp3::NSIspio::EPIPECmd_GET_MODULE_HANDLE, module, (MINTPTR)&handle, (MINTPTR)(&("AAOBufMgr::DMAConfig()")))) \
            { \
                CAM_LOGE("EPIPECmd_GET_MODULE_HANDLE ConfigHWReg fail"); \
            } \
        }
#define IOPIPE_SET_MODUL_CFG_DONE(handle) \
        { \
            if (MTRUE != sendCommandNormalPipe(NSImageioIsp3::NSIspio::EPIPECmd_SET_MODULE_CFG_DONE, handle, MNULL, MNULL)) \
            { \
                CAM_LOGE("EPIPECmd_SET_MODULE_CFG_DONE ConfigHWReg fail"); \
            } \
        }
#define IOPIPE_RELEASE_MODUL_HANDLE(handle) \
        { \
            if (MTRUE != sendCommandNormalPipe(NSImageioIsp3::NSIspio::EPIPECmd_RELEASE_MODULE_HANDLE, handle, MNULL, MNULL)) \
            { \
                CAM_LOGE("EPIPECmd_RELEASE_MODULE_HANDLE ConfigHWReg fail"); \
            } \
            handle = (MUINTPTR)NULL; \
        }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class AAOBufMgrWrapper
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    //  Copy constructor is disallowed.
    AAOBufMgrWrapper(AAOBufMgrWrapper const&);
    //  Copy-assignment operator is disallowed.
    AAOBufMgrWrapper& operator=(AAOBufMgrWrapper const&);

public:
    AAOBufMgrWrapper(ESensorDev_T const eSensorDev);
    ~AAOBufMgrWrapper();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static AAOBufMgrWrapper& getInstance(MINT32 const i4SensorDev);
    MBOOL init(MINT32 const i4SensorIdx);
    MBOOL uninit();
    MBOOL debugPrint();
    MBOOL enqueueHwBuf(BufInfo_T& rBufInfo);
    MBOOL dequeueHwBuf(BufInfo_T& rBufInfo);
    MUINT32 getCurrHwBuf();
    MUINT32 getNextHwBuf(MUINT32 u4BufIdx);
    MBOOL allocateBuf(BufInfo_T &rBufInfo, MUINT32 u4BufSize);
    MBOOL freeBuf(BufInfo_T &rBufInfo);

    MBOOL updateDMABaseAddr(MUINT32 u4BaseAddr);
    MBOOL DMAInit();
    MBOOL DMAUninit();
    MBOOL AAStatEnable(MBOOL En);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //                               ISP 3.0 Wrapper
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    BufInfo_T m_rBufInfo;
    MBOOL initPipe(MINT32 const i4SensorIdx, MINT32 const i4TGInfo);     // create Buffer
    MBOOL uninitPipe();
    MBOOL startPipe();
    MBOOL stopPipe();
    MBOOL dequePipe(BufInfo_T& rBufInfo);
    MBOOL enquePipe(BufInfo_T& rBufInfo);
    MBOOL abortDeque();

    inline MBOOL sendCommandNormalPipe(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3)
    {
        INormalPipe_FrmB* pPipe = INormalPipe_FrmB::createInstance(m_i4SensorIdx, "aao_buf_mgr");
        MBOOL fgRet = pPipe->sendCommand(cmd, arg1, arg2, arg3);
        pPipe->destroyInstance("aao_buf_mgr");
        return fgRet;
    }

    inline MBOOL setTGInfo(MINT32 const i4TGInfo)
    {
        MY_LOG("[%s()] i4TGInfo: %d \n", __FUNCTION__, i4TGInfo);

        switch (i4TGInfo)
        {
        case CAM_TG_1:
            m_eSensorTG = ESensorTG_1;
            break;
        case CAM_TG_2:
            m_eSensorTG = ESensorTG_2;
            break;
        default:
            MY_ERR("i4TGInfo = %d", i4TGInfo);
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
    IspDrv*                m_pIspDrv;
    MUINT32                m_u4AEStateSize;
    MUINT32                m_u4AEHistSize;
    MUINT32                m_u4AWBStateSize;
    MUINT32                m_u4AAOBufSize;
    MUINT32                m_u4AAOXSize;
    volatile MINT32        m_Users;
    mutable android::Mutex m_Lock;
    MBOOL                  m_bDebugEnable;
    MINT32                 m_i4SensorIdx;
    BufInfoList_T          m_rHwBufList;
    BufInfo_T              m_rAAOBufInfo[MAX_AAO_BUFFER_CNT];
    MINT32                 m_i4AAOIrqKey;
    mutable android::Mutex m_QueueLock; // avoid de/enqueue at the same time to protect queue size
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
template <ESensorDev_T const eSensorDev>
class AAOBufMgrDev : public AAOBufMgrWrapper
{
public:
    static
    AAOBufMgrWrapper&
    getInstance()
    {
        static AAOBufMgrDev<eSensorDev> singleton;
        return singleton;
    }

    AAOBufMgrDev()
        : AAOBufMgrWrapper(eSensorDev)
    {}

    virtual ~AAOBufMgrDev() {}
};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAAOBufMgrWrapper::
init(MINT32 const i4SensorDev, MINT32 const i4SensorIdx)
{
    MBOOL ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AAOBufMgrWrapper::getInstance(ESensorDev_Main).init(i4SensorIdx);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AAOBufMgrWrapper::getInstance(ESensorDev_Sub).init(i4SensorIdx);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AAOBufMgrWrapper::getInstance(ESensorDev_MainSecond).init(i4SensorIdx);

    return ret_main && ret_sub && ret_main2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAAOBufMgrWrapper::
uninit(MINT32 const i4SensorDev)
{
    MBOOL ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
       ret_main = AAOBufMgrWrapper::getInstance(ESensorDev_Main).uninit();
    if (i4SensorDev & ESensorDev_Sub)
       ret_sub = AAOBufMgrWrapper::getInstance(ESensorDev_Sub).uninit();
    if (i4SensorDev & ESensorDev_MainSecond)
       ret_main2 = AAOBufMgrWrapper::getInstance(ESensorDev_MainSecond).uninit();

    return ret_main && ret_sub && ret_main2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAAOBufMgrWrapper::
DMAInit(MINT32 const i4SensorDev)
{
    MBOOL ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AAOBufMgrWrapper::getInstance(ESensorDev_Main).DMAInit();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AAOBufMgrWrapper::getInstance(ESensorDev_Sub).DMAInit();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AAOBufMgrWrapper::getInstance(ESensorDev_MainSecond).DMAInit();

    return ret_main && ret_sub && ret_main2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAAOBufMgrWrapper::
DMAUninit(MINT32 const i4SensorDev)
{
    MBOOL ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AAOBufMgrWrapper::getInstance(ESensorDev_Main).DMAUninit();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AAOBufMgrWrapper::getInstance(ESensorDev_Sub).DMAUninit();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AAOBufMgrWrapper::getInstance(ESensorDev_MainSecond).DMAUninit();

    return ret_main && ret_sub && ret_main2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAAOBufMgrWrapper::
AAStatEnable(MINT32 const i4SensorDev, MBOOL En)
{
    MBOOL ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AAOBufMgrWrapper::getInstance(ESensorDev_Main).AAStatEnable(En);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AAOBufMgrWrapper::getInstance(ESensorDev_Sub).AAStatEnable(En);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AAOBufMgrWrapper::getInstance(ESensorDev_MainSecond).AAStatEnable(En);

    return ret_main && ret_sub && ret_main2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAAOBufMgrWrapper::
enqueueHwBuf(MINT32 const i4SensorDev, BufInfo_T& rBufInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return AAOBufMgrWrapper::getInstance(ESensorDev_Main).enqueueHwBuf(rBufInfo);
    else if (i4SensorDev & ESensorDev_Sub)
        return AAOBufMgrWrapper::getInstance(ESensorDev_Sub).enqueueHwBuf(rBufInfo);
    else if (i4SensorDev & ESensorDev_MainSecond)
        return AAOBufMgrWrapper::getInstance(ESensorDev_MainSecond).enqueueHwBuf(rBufInfo);

    MY_ERR("Incorrect sensor device: i4SensorDev = %d", i4SensorDev);
    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAAOBufMgrWrapper::
dequeueHwBuf(MINT32 const i4SensorDev, BufInfo_T& rBufInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return AAOBufMgrWrapper::getInstance(ESensorDev_Main).dequeueHwBuf(rBufInfo);
    else if (i4SensorDev & ESensorDev_Sub)
        return AAOBufMgrWrapper::getInstance(ESensorDev_Sub).dequeueHwBuf(rBufInfo);
    else if (i4SensorDev & ESensorDev_MainSecond)
        return AAOBufMgrWrapper::getInstance(ESensorDev_MainSecond).dequeueHwBuf(rBufInfo);

    MY_ERR("Incorrect sensor device: i4SensorDev = %d", i4SensorDev);
    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAAOBufMgrWrapper::
updateDMABaseAddr(MINT32 const i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AAOBufMgrWrapper::getInstance(ESensorDev_Main).updateDMABaseAddr(AAOBufMgrWrapper::getInstance(ESensorDev_Main).getNextHwBuf(1));
    else if (i4SensorDev & ESensorDev_Sub)
        return AAOBufMgrWrapper::getInstance(ESensorDev_Sub).updateDMABaseAddr(AAOBufMgrWrapper::getInstance(ESensorDev_Sub).getNextHwBuf(1));
    else if (i4SensorDev & ESensorDev_MainSecond)
        return AAOBufMgrWrapper::getInstance(ESensorDev_MainSecond).updateDMABaseAddr(AAOBufMgrWrapper::getInstance(ESensorDev_MainSecond).getNextHwBuf(1));

    MY_ERR("Incorrect sensor device: i4SensorDev = %d", i4SensorDev);
    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IAAOBufMgrWrapper::
setTGInfo(MINT32 const i4SensorDev, MINT32 const i4TGInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return AAOBufMgrWrapper::getInstance(ESensorDev_Main).setTGInfo(i4TGInfo);
    else if (i4SensorDev & ESensorDev_Sub)
        return AAOBufMgrWrapper::getInstance(ESensorDev_Sub).setTGInfo(i4TGInfo);
    else if (i4SensorDev & ESensorDev_MainSecond)
        return AAOBufMgrWrapper::getInstance(ESensorDev_MainSecond).setTGInfo(i4TGInfo);

    MY_ERR("Incorrect sensor device: i4SensorDev = %d", i4SensorDev);
    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                               ISP 3.0 Wrapper
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAAOBufMgrWrapper::initPipe(MINT32 const i4SensorDev, MINT32 const i4SensorIdx, MINT32 const i4TGInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return AAOBufMgrWrapper::getInstance(ESensorDev_Main).initPipe(i4SensorIdx, i4TGInfo);
    else if (i4SensorDev & ESensorDev_Sub)
        return AAOBufMgrWrapper::getInstance(ESensorDev_Sub).initPipe(i4SensorIdx, i4TGInfo);
    else if (i4SensorDev & ESensorDev_MainSecond)
        return AAOBufMgrWrapper::getInstance(ESensorDev_MainSecond).initPipe(i4SensorIdx, i4TGInfo);

    MY_ERR("Incorrect sensor device: i4SensorDev = %d", i4SensorDev);
    return MFALSE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAAOBufMgrWrapper::uninitPipe(MINT32 const i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AAOBufMgrWrapper::getInstance(ESensorDev_Main).uninitPipe();
    else if (i4SensorDev & ESensorDev_Sub)
        return AAOBufMgrWrapper::getInstance(ESensorDev_Sub).uninitPipe();
    else if (i4SensorDev & ESensorDev_MainSecond)
        return AAOBufMgrWrapper::getInstance(ESensorDev_MainSecond).uninitPipe();

    MY_ERR("Incorrect sensor device: i4SensorDev = %d", i4SensorDev);
    return MFALSE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAAOBufMgrWrapper::startPipe(MINT32 const i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AAOBufMgrWrapper::getInstance(ESensorDev_Main).startPipe();
    else if (i4SensorDev & ESensorDev_Sub)
        return AAOBufMgrWrapper::getInstance(ESensorDev_Sub).startPipe();
    else if (i4SensorDev & ESensorDev_MainSecond)
        return AAOBufMgrWrapper::getInstance(ESensorDev_MainSecond).startPipe();

    MY_ERR("Incorrect sensor device: i4SensorDev = %d", i4SensorDev);
    return MFALSE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAAOBufMgrWrapper::stopPipe(MINT32 const i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AAOBufMgrWrapper::getInstance(ESensorDev_Main).stopPipe();
    else if (i4SensorDev & ESensorDev_Sub)
        return AAOBufMgrWrapper::getInstance(ESensorDev_Sub).stopPipe();
    else if (i4SensorDev & ESensorDev_MainSecond)
        return AAOBufMgrWrapper::getInstance(ESensorDev_MainSecond).stopPipe();

    MY_ERR("Incorrect sensor device: i4SensorDev = %d", i4SensorDev);
    return MFALSE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAAOBufMgrWrapper::dequePipe(MINT32 const i4SensorDev, BufInfo_T& rBufInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return AAOBufMgrWrapper::getInstance(ESensorDev_Main).dequePipe(rBufInfo);
    else if (i4SensorDev & ESensorDev_Sub)
        return AAOBufMgrWrapper::getInstance(ESensorDev_Sub).dequePipe(rBufInfo);
    else if (i4SensorDev & ESensorDev_MainSecond)
        return AAOBufMgrWrapper::getInstance(ESensorDev_MainSecond).dequePipe(rBufInfo);

    MY_ERR("Incorrect sensor device: i4SensorDev = %d", i4SensorDev);
    return MFALSE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAAOBufMgrWrapper::enquePipe(MINT32 const i4SensorDev, BufInfo_T& rBufInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return AAOBufMgrWrapper::getInstance(ESensorDev_Main).enquePipe(rBufInfo);
    else if (i4SensorDev & ESensorDev_Sub)
        return AAOBufMgrWrapper::getInstance(ESensorDev_Sub).enquePipe(rBufInfo);
    else if (i4SensorDev & ESensorDev_MainSecond)
        return AAOBufMgrWrapper::getInstance(ESensorDev_MainSecond).enquePipe(rBufInfo);

    MY_ERR("Incorrect sensor device: i4SensorDev = %d", i4SensorDev);
    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAAOBufMgrWrapper::abortDeque(MINT32 const i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AAOBufMgrWrapper::getInstance(ESensorDev_Main).abortDeque();
    else if (i4SensorDev & ESensorDev_Sub)
        return AAOBufMgrWrapper::getInstance(ESensorDev_Sub).abortDeque();
    else if (i4SensorDev & ESensorDev_MainSecond)
        return AAOBufMgrWrapper::getInstance(ESensorDev_MainSecond).abortDeque();

    MY_ERR("Incorrect sensor device: i4SensorDev = %d", i4SensorDev);
    return MFALSE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AAOBufMgrWrapper&
AAOBufMgrWrapper::
getInstance(MINT32 const i4SensorDev)
{
    switch (i4SensorDev)
    {
    case ESensorDev_Main: //  Main Sensor
        return  AAOBufMgrDev<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  AAOBufMgrDev<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  AAOBufMgrDev<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("i4SensorDev = %d", i4SensorDev);
        return  AAOBufMgrDev<ESensorDev_Main>::getInstance();
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AAOBufMgrWrapper::
AAOBufMgrWrapper(ESensorDev_T const eSensorDev)
    : m_eSensorDev(eSensorDev)
    , m_eSensorTG(ESensorTG_None)
    , m_pIMemDrv(IMemDrv::createInstance())
    , m_pIspDrv(MNULL)
    , m_u4AEStateSize(0)
    , m_u4AEHistSize(0)
    , m_u4AWBStateSize(0)
    , m_u4AAOBufSize(0)
    , m_u4AAOXSize(0)
    , m_Users(0)
    , m_Lock()
    , m_bDebugEnable(MFALSE)
    , m_i4SensorIdx(0)
    , m_rHwBufList()
    , m_i4AAOIrqKey(0)
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AAOBufMgrWrapper::
~AAOBufMgrWrapper()
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
AAOBufMgrWrapper::
init(MINT32 const i4SensorIdx)
{
    MINT32 i4Coverity = 0;
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.aao_buf_mgr.enable", value, "0"); // temp for debug
    m_bDebugEnable = atoi(value);

    // sensor index
    m_i4SensorIdx = i4SensorIdx;

    MY_LOG("[%s()] m_eSensorDev: %d, m_i4SensorIdx: %d, m_Users: %d \n", __FUNCTION__, m_eSensorDev, m_i4SensorIdx, m_Users);

    Mutex::Autolock lock(m_Lock);

    if (m_Users > 0)
    {
        MY_LOG("%d has created \n", m_Users);
        i4Coverity = android_atomic_inc(&m_Users);
        return MTRUE;
    }

    // AAO statistics init
    AWB_STAT_PARAM_T rAWBStatParam;

    switch (m_eSensorDev)
    {
    case ESensorDev_Main: //  Main Sensor
        rAWBStatParam = getAWBStatParam<ESensorDev_Main>();
        break;
    case ESensorDev_MainSecond: //  Main Second Sensor
        rAWBStatParam = getAWBStatParam<ESensorDev_MainSecond>();
        break;
    case ESensorDev_Sub: //  Sub Sensor
        rAWBStatParam = getAWBStatParam<ESensorDev_Sub>();
        break;
    default:
        MY_ERR("m_eSensorDev = %d", m_eSensorDev);
        rAWBStatParam = getAWBStatParam<ESensorDev_Main>();
        break;
    }

    m_u4AEStateSize = static_cast<MUINT32>(((rAWBStatParam.i4WindowNumX + 3) / 4) * 4 * rAWBStatParam.i4WindowNumY);
    m_u4AEHistSize = 4 * 256;
    m_u4AWBStateSize = static_cast<MUINT32>(rAWBStatParam.i4WindowNumX * rAWBStatParam.i4WindowNumY * 4);
    m_u4AAOBufSize = m_u4AEStateSize + m_u4AEHistSize + m_u4AWBStateSize;
    m_u4AAOXSize = m_u4AAOBufSize - 1;

    MY_LOG("[%s()] m_u4AEStateSize = %d, m_u4AEHistSize = %d, m_u4AWBStateSize = %d, m_u4AAOBufSize = %d, m_u4AAOXSize = %d",
            __FUNCTION__, m_u4AEStateSize, m_u4AEHistSize, m_u4AWBStateSize, m_u4AAOBufSize, m_u4AAOXSize);

    // Normal pipe
    //m_pPipe = INormalPipe::createInstance(m_i4SensorIdx,"aao_buf_mgr");

    // imem driver init
    m_pIMemDrv->init();

    // AAO DMA buffer init

    // removes all elements from the list container
    m_rHwBufList.clear();

    // allocate and enqueue HW buffer
    for (MINT32 i = 0; i < MAX_AAO_BUFFER_CNT; i++) {
        m_rAAOBufInfo[i].useNoncache = 0;   // improve the performance
        allocateBuf(m_rAAOBufInfo[i], m_u4AAOBufSize);
        enqueueHwBuf(m_rAAOBufInfo[i]);
    }

    //DMAInit();

    // Enable AA stat
    //AAStatEnable(MTRUE);

    debugPrint();

    i4Coverity = android_atomic_inc(&m_Users);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
AAOBufMgrWrapper::
uninit()
{
    IMEM_BUF_INFO buf_info;

    MY_LOG("[%s()] m_eSensorDev: %d, m_Users: %d \n", __FUNCTION__, m_eSensorDev, m_Users);

    Mutex::Autolock lock(m_Lock);

    // If no more users, return directly and do nothing.
    if (m_Users <= 0)
    {
        return MTRUE;
    }

    // More than one user, so decrease one User.
    /*MINT32 i4Coverity = */android_atomic_dec(&m_Users);

    if (m_Users == 0) // There is no more User after decrease one User
    {
        // Disable AA stat
        //AAStatEnable(MFALSE);

        // AAO DMA buffer uninit
        //DMAUninit();

        for (MINT32 i = 0; i < MAX_AAO_BUFFER_CNT; i++) {
            freeBuf(m_rAAOBufInfo[i]);
        }
        m_rHwBufList.clear();

       // imem driver ininit
       m_pIMemDrv->uninit();

       // Normal pipe
       //m_pPipe->destroyInstance("aao_buf_mgr");
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
MBOOL
AAOBufMgrWrapper::
debugPrint()
{
    BufInfoList_T::iterator it;

    for (it = m_rHwBufList.begin(); it != m_rHwBufList.end(); it++ ) {
        MY_LOG("[%s()] m_eSensorDev(%d) m_rHwBufList.virtAddr:[0x%x]/phyAddr:[0x%x] \n", __FUNCTION__, m_eSensorDev, it->virtAddr, it->phyAddr);
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
AAOBufMgrWrapper::
enqueueHwBuf(BufInfo_T& rBufInfo)
{
    {
        Mutex::Autolock lock(m_QueueLock);
        // add element at the end
        m_rHwBufList.push_back(rBufInfo);
        MY_LOG_IF(m_bDebugEnable,"[%s()] rBufInfo.virtAddr:[0x%x]/phyAddr:[0x%x]/size:%d->%d\n", __FUNCTION__, rBufInfo.virtAddr, rBufInfo.phyAddr, m_rHwBufList.size() - 1, m_rHwBufList.size());
    }
    m_pIMemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_INVALID, &rBufInfo);

    return MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
AAOBufMgrWrapper::
dequeueHwBuf(BufInfo_T& rBufInfo)
{
    Mutex::Autolock lock(m_QueueLock);
    if (m_rHwBufList.size()) {
        rBufInfo = m_rHwBufList.front();
        m_rHwBufList.pop_front();
    }
    MY_LOG_IF(m_bDebugEnable,"[%s()] rBufInfo.virtAddr:[0x%x]/phyAddr:[0x%x]/size:%d->%d\n", __FUNCTION__, rBufInfo.virtAddr, rBufInfo.phyAddr, m_rHwBufList.size() + 1, m_rHwBufList.size());

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32
AAOBufMgrWrapper::
getCurrHwBuf()
{
    if (m_rHwBufList.size() > 0) {
        return m_rHwBufList.front().phyAddr;
    }
    else { // No free buffer
        MY_ERR("[getCurrHwBuf] No free buffer\n");
        return 0;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32
AAOBufMgrWrapper::
getNextHwBuf(MUINT32 u4BufIdx)
{
    Mutex::Autolock lock(m_QueueLock);
    MY_LOG_IF(m_bDebugEnable,"[%s] m_rHwBufList.size:%d\n", __FUNCTION__, m_rHwBufList.size());
    BufInfoList_T::iterator it;

    if (m_rHwBufList.size() > u4BufIdx) {
        it = m_rHwBufList.begin();
        for (MUINT32 i = 0; i < u4BufIdx; i++)
            it++;
        return it->phyAddr;
    }
    else { // No free buffer
       MY_ERR("[getNextHwBuf] No free buffer\n");
       return 0;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
AAOBufMgrWrapper::
allocateBuf(BufInfo_T &rBufInfo, MUINT32 u4BufSize)
{
    rBufInfo.size = u4BufSize;
    if (m_pIMemDrv->allocVirtBuf(&rBufInfo)) {
        MY_ERR("m_pIMemDrv->allocVirtBuf() error");
        return MFALSE;
    }

    if (m_pIMemDrv->mapPhyAddr(&rBufInfo)) {
        MY_ERR("m_pIMemDrv->mapPhyAddr() error");
        return MFALSE;
    }

    MY_LOG("[%s()] memID = 0x%x\n", __FUNCTION__, rBufInfo.memID);

    return MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
AAOBufMgrWrapper::
freeBuf(BufInfo_T &rBufInfo)
{
    if (m_pIMemDrv->unmapPhyAddr(&rBufInfo)) {
        MY_ERR("m_pIMemDrv->unmapPhyAddr() error");
        return MFALSE;
    }

    if (m_pIMemDrv->freeVirtBuf(&rBufInfo)) {
        MY_ERR("m_pIMemDrv->freeVirtBuf() error");
        return MFALSE;
    }

    MY_LOG("[%s()] memID = 0x%x\n", __FUNCTION__, rBufInfo.memID);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
AAOBufMgrWrapper::
updateDMABaseAddr(MUINT32 u4BaseAddr)
{
    MY_LOG_IF(m_bDebugEnable,"[%s] u4BaseAddr=0x%x\n", __FUNCTION__, u4BaseAddr);

    if (!u4BaseAddr) {
        MY_ERR("u4BaseAddr is NULL\n");
        return E_ISPMGR_NULL_ADDRESS;
    }

    MUINTPTR handle;
    if (m_eSensorTG == ESensorTG_1) { // TG1
        // get module handle
        IOPIPE_GET_MODUL_HANDLE(NSImageioIsp3::NSIspio::EModule_AAO, handle);
        if (handle != (MUINTPTR)NULL) { // set module register
            IOPIPE_SET_MODUL_REG(handle, CAM_AAO_BASE_ADDR, u4BaseAddr);
            IOPIPE_SET_MODUL_REG(handle, CAM_AAO_OFST_ADDR, AAO_OFFSET_ADDR);
            IOPIPE_SET_MODUL_REG(handle, CAM_AAO_XSIZE, m_u4AAOXSize);
            IOPIPE_SET_MODUL_REG(handle, CAM_AAO_STRIDE, (AAO_STRIDE_BUS_SIZE << 16));
            IOPIPE_SET_MODUL_CFG_DONE(handle);
            IOPIPE_RELEASE_MODUL_HANDLE(handle);
        }
    }
    else if (m_eSensorTG == ESensorTG_2) { // TG2
        // get module handle
//        IOPIPE_GET_MODUL_HANDLE(NSImageioIsp3::NSIspio::EModule_AAO_D, handle);
//        if (NULL != handle) { // set module register
//            IOPIPE_SET_MODUL_REG(handle, CAM_AAO_D_BASE_ADDR, u4BaseAddr);
//            IOPIPE_SET_MODUL_REG(handle, CAM_AAO_D_OFST_ADDR, AAO_OFFSET_ADDR);
//            IOPIPE_SET_MODUL_REG(handle, CAM_AAO_D_XSIZE, m_u4AAOXSize);
//            IOPIPE_SET_MODUL_REG(handle, CAM_AAO_D_STRIDE, (AAO_STRIDE_BUS_SIZE << 16));
//            IOPIPE_SET_MODUL_CFG_DONE(handle);
//            IOPIPE_RELEASE_MODUL_HANDLE(handle);
//        }
    }
    else {
        MY_ERR("Incorrect TG info: m_eSensorTG = %d", m_eSensorTG);
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
AAOBufMgrWrapper::
DMAInit()
{
    MY_LOG_IF(m_bDebugEnable,"[%s()] m_eSensorDev(%d)\n", __FUNCTION__, m_eSensorDev);

    updateDMABaseAddr(getNextHwBuf(1));

    if (m_eSensorTG == ESensorTG_1) { // TG1
        MY_LOG_IF(m_bDebugEnable,"[%s()] EPIPECmd_SET_MODULE_En(MTRUE): EModule_AWB\n", __FUNCTION__);
        IOPIPE_SET_MODUL_ENABLE(NSImageioIsp3::NSIspio::EModule_AWB, MTRUE);

        MY_LOG_IF(m_bDebugEnable,"[%s()] EPIPECmd_SET_MODULE_En(MTRUE): EModule_AAO\n", __FUNCTION__);
        IOPIPE_SET_MODUL_ENABLE(NSImageioIsp3::NSIspio::EModule_AAO, MTRUE);
    }
    else if (m_eSensorTG == ESensorTG_2) { // TG2
        MY_LOG_IF(m_bDebugEnable,"[%s()] EPIPECmd_SET_MODULE_En(MTRUE): EModule_AWB\n", __FUNCTION__);
        IOPIPE_SET_MODUL_ENABLE(NSImageioIsp3::NSIspio::EModule_AWB_D, MTRUE);

        MY_LOG_IF(m_bDebugEnable,"[%s()] EPIPECmd_SET_MODULE_En(MTRUE): EModule_AAO\n", __FUNCTION__);
        IOPIPE_SET_MODUL_ENABLE(NSImageioIsp3::NSIspio::EModule_AAO_D, MTRUE);
    }
    else {
        MY_ERR("Incorrect TG info: m_eSensorTG = %d", m_eSensorTG);
    }

    if (m_pIspDrv == NULL) {
        m_pIspDrv = IspDrv::createInstance();

        if (!m_pIspDrv) {
            CAM_LOGE("IspDrv::createInstance() fail \n");
            return MFALSE;
        }

        if (m_pIspDrv->init("AAOIrq") < 0) {
            CAM_LOGE("pIspDrv->init() fail \n");
            return MFALSE;
        }

        m_i4AAOIrqKey = m_pIspDrv->registerIrq(LOG_TAG);
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
AAOBufMgrWrapper::
DMAUninit()
{
    MY_LOG_IF(m_bDebugEnable,"[%s()] m_eSensorDev(%d)\n", __FUNCTION__, m_eSensorDev);

    if (m_eSensorTG == ESensorTG_1) { // TG1
        MY_LOG_IF(m_bDebugEnable,"[%s()] EPIPECmd_SET_MODULE_En(MFALSE): EModule_AAO\n", __FUNCTION__);
        IOPIPE_SET_MODUL_ENABLE(NSImageioIsp3::NSIspio::EModule_AAO, MFALSE);
        MY_LOG_IF(m_bDebugEnable,"[%s()] EPIPECmd_SET_MODULE_En(MFALSE): EModule_AWB\n", __FUNCTION__);
        IOPIPE_SET_MODUL_ENABLE(NSImageioIsp3::NSIspio::EModule_AWB, MFALSE);
    }
    else if (m_eSensorTG == ESensorTG_2) { // TG2
        MY_LOG_IF(m_bDebugEnable,"[%s()] EPIPECmd_SET_MODULE_En(MFALSE): EModule_AAO_D\n", __FUNCTION__);
        IOPIPE_SET_MODUL_ENABLE(NSImageioIsp3::NSIspio::EModule_AAO_D, MFALSE);
        MY_LOG_IF(m_bDebugEnable,"[%s()] EPIPECmd_SET_MODULE_En(MFALSE): EModule_AWB_D\n", __FUNCTION__);
        IOPIPE_SET_MODUL_ENABLE(NSImageioIsp3::NSIspio::EModule_AWB_D, MFALSE);
    }
    else {
        CAM_LOGE("m_eSensorTG = %d, Suggestion: Need to call queryTGInfoFromSensorHal() before waitVSirq().", m_eSensorTG);
        return MFALSE;
    }

    /* flushAAOirq */
    ISP_DRV_WAIT_IRQ_STRUCT waitIrq;
    waitIrq.UserInfo.Type    = (m_eSensorTG == ESensorTG_1) ?
                               ISP_DRV_IRQ_TYPE_INT_P1_ST : ISP_DRV_IRQ_TYPE_INT_P1_ST_D;
    waitIrq.UserInfo.Status  = (m_eSensorTG == ESensorTG_1) ?
                               CAM_CTL_INT_P1_STATUS_PASS1_DON_ST : CAM_CTL_INT_P1_STATUS_D_PASS1_DON_ST;
    waitIrq.UserInfo.UserKey = m_i4AAOIrqKey;

    if (m_pIspDrv == NULL) {
        CAM_LOGE("isp drv = NULL");
    }
    else {
        m_pIspDrv->flushIrq(waitIrq);
    }

    if (m_pIspDrv) {
        m_pIspDrv->uninit("AAOIrq");
        m_pIspDrv->destroyInstance();
        m_pIspDrv = NULL;
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
AAOBufMgrWrapper::
AAStatEnable(MBOOL En)
{
    MY_LOG_IF(m_bDebugEnable,"[%s()] m_eSensorDev(%d) AAStatEnable(%d)\n", __FUNCTION__, m_eSensorDev, En);

//    if (m_eSensorTG == ESensorTG_1) { // TG1
//        MY_LOG_IF(m_bDebugEnable,"[%s()] EPIPECmd_SET_MODULE_En(%d): EModule_AWB\n", __FUNCTION__, En);
//        IOPIPE_SET_MODUL_ENABLE(NSImageioIsp3::NSIspio::EModule_AWB, En);
//    }
//    else if (m_eSensorTG == ESensorTG_2) { // TG2
//        MY_LOG_IF(m_bDebugEnable,"[%s()] EPIPECmd_SET_MODULE_En(%d): EModule_AWB_D\n", __FUNCTION__, En);
//        IOPIPE_SET_MODUL_ENABLE(NSImageioIsp3::NSIspio::EModule_AWB_D, En);
//    }
//    else {
//        MY_ERR("Incorrect TG info: m_eSensorTG = %d", m_eSensorTG);
//        return MFALSE;
//    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                               ISP 3.0 Wrapper
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
AAOBufMgrWrapper::
initPipe(MINT32 const i4SensorIdx, MINT32 const i4TGInfo)
{
    init(i4SensorIdx);
    setTGInfo(i4TGInfo);
    return MTRUE;
}

MBOOL
AAOBufMgrWrapper::
uninitPipe()
{
    uninit();
    return MTRUE;
}

MBOOL
AAOBufMgrWrapper::
startPipe()
{
    DMAInit();
    return MTRUE;
}

MBOOL
AAOBufMgrWrapper::
stopPipe()
{
    DMAUninit();
    return MTRUE;
}

MBOOL
AAOBufMgrWrapper::
dequePipe(BufInfo_T& rBufInfo)
{
    ISP_DRV_WAIT_IRQ_STRUCT waitIrq;

    if (m_eSensorTG == ESensorTG_1) {
        waitIrq.Clear            = ISP_DRV_IRQ_CLEAR_WAIT;
        waitIrq.UserInfo.Type    = ISP_DRV_IRQ_TYPE_INT_P1_ST;
        waitIrq.UserInfo.Status  = CAM_CTL_INT_P1_STATUS_PASS1_DON_ST;
        waitIrq.UserInfo.UserKey = m_i4AAOIrqKey;
        waitIrq.Timeout          = 500;
    }
    else if (m_eSensorTG == ESensorTG_2) {
        waitIrq.Clear            = ISP_DRV_IRQ_CLEAR_WAIT;
        waitIrq.UserInfo.Type    = ISP_DRV_IRQ_TYPE_INT_P1_ST_D;
        waitIrq.UserInfo.Status  = CAM_CTL_INT_P1_STATUS_D_PASS1_DON_ST;
        waitIrq.UserInfo.UserKey = m_i4AAOIrqKey;
        waitIrq.Timeout          = 500;
    }
    else {
        MY_ERR("Incorrect TG info: m_eSensorTG = %d", m_eSensorTG);
    }

    MY_LOG_IF(m_bDebugEnable,"[%s()] wait pass1 start\n", __FUNCTION__);
    if(MTRUE != m_pIspDrv->waitIrq(&waitIrq)) { // wait for pass1 done
        MY_ERR("[%s()] wait pass1 fail\n", __FUNCTION__);
        return MFALSE;
    }
    MY_LOG_IF(m_bDebugEnable,"[%s()] wait pass1 done\n", __FUNCTION__);

    MUINT32 u4Next2BufAddr = getNextHwBuf(2);
    if(u4Next2BufAddr == 0) {
        MY_ERR("[%s()] deque fail, wait for enque\n", __FUNCTION__);
        return MFALSE;
    }
    else {
        dequeueHwBuf(rBufInfo);
        updateDMABaseAddr(u4Next2BufAddr);
    }

    return MTRUE;
}

MBOOL
AAOBufMgrWrapper::
enquePipe(BufInfo_T& rBufInfo)
{
    enqueueHwBuf(rBufInfo);
    return MTRUE;
}

MBOOL AAOBufMgrWrapper::abortDeque()
{
    MY_LOG_IF(m_bDebugEnable,"[%s()] Dev(%d) \n", __FUNCTION__, m_eSensorDev);

    /* flushAAOirq */
    ISP_DRV_WAIT_IRQ_STRUCT waitIrq;
    waitIrq.UserInfo.Type    = (m_eSensorTG == ESensorTG_1) ?
                               ISP_DRV_IRQ_TYPE_INT_P1_ST : ISP_DRV_IRQ_TYPE_INT_P1_ST_D;
    waitIrq.UserInfo.Status  = (m_eSensorTG == ESensorTG_1) ?
                               CAM_CTL_INT_P1_STATUS_PASS1_DON_ST : CAM_CTL_INT_P1_STATUS_D_PASS1_DON_ST;
    waitIrq.UserInfo.UserKey = m_i4AAOIrqKey;

    if (m_pIspDrv == NULL) {
        CAM_LOGE("isp drv = NULL");
    }
    else {
        m_pIspDrv->flushIrq(waitIrq);
    }

    return MTRUE;
}
