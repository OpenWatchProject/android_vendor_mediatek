/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef _MTK_PLATFORM_HARDWARE_MTKCAM_IOPIPE_CAMIO_CAMSVSTATPIPE_H_
#define _MTK_PLATFORM_HARDWARE_MTKCAM_IOPIPE_CAMIO_CAMSVSTATPIPE_H_

#include <utils/threads.h>
#include <cutils/atomic.h>
#include <semaphore.h>
#include <pthread.h>

//#include <mtkcam/drv/iopipe/CamIO/IHalCamIO.h>
#include <mtkcam/drv/iopipe/CamIO/ICamsvStatisticPipe.h>

#include <imageio/IPipe.h>
#include <imageio/ICamIOPipe.h>
//#include "isp_drv_camsv.h"


#include <drv/imem_drv.h>

#include <deque>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSIoPipe {
namespace NSCamIOPipe {

/******************************************************************************
 *
 *
 ******************************************************************************/
#define CAMSV_BUF_DEPTH                 (2)  // default


class CamsvStatisticPipe_Thread;

class CamsvStatisticPipe : public ICamsvStatisticPipe {

public:
    CamsvStatisticPipe (MUINT32 pSensorIdx, char const* szCallerName, MUINT32 FeatureIdx = 0);

    ~CamsvStatisticPipe(){};
public:
    virtual MVOID   destroyInstance(char const* szCallerName);

    virtual MBOOL   start();

    virtual MBOOL   stop(MBOOL bNonblocking = MFALSE);

    virtual MBOOL   init();

    virtual MBOOL   uninit();

    virtual MBOOL   enque(QBufInfo const& rQBuf);

    virtual MBOOL   deque(PortID port, QBufInfo& rQBuf, ESTT_CACHE_SYNC_POLICY cacheSyncPolicy = ESTT_CacheInvalidByRange, MUINT32 u4TimeoutMs = 0xFFFFFFFF);

    virtual MBOOL   reset() {return MFALSE;};

    virtual MBOOL   configPipe(QInitStatParam const& vInPorts, MINT32 burstQnum = 1);

    virtual MBOOL   sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3);

    virtual MBOOL   suspend(void);

    virtual MBOOL   resume(void);

    typedef enum {
        _DMAO_NONE     = 0x0000,
        _AAO_ENABLE    = 0x0100,
        _AFO_ENABLE    = 0x0200,
        _FLKO_ENABLE   = 0x0400,
        _PDO_ENABLE    = 0x0800,
        //_EISO_ENABLE = 0x1000,
        _CAMSV_IMGO_ENABLE = 0x2000,
    } eDMAO;

    typedef enum {
        _GetPort_Opened     = 0x1,
        _GetPort_OpendNum   = 0x2,
        _GetPort_Index      = 0x3,
    } ePortInfoCmd;
    MUINT32         GetOpenedPortInfo(CamsvStatisticPipe::ePortInfoCmd cmd,MUINT32 arg1=0);

    static void     enqueRequestCB(void *para, void *arg);
public:
    virtual MINT32  attach(const char* UserName);
    virtual MBOOL   wait(EPipeSignal eSignal, EPipeSignalClearType eClear, const MINT32 mUserKey,
                            MUINT32 TimeoutMs = 0xFFFFFFFF, SignalTimeInfo *pTime = NULL);
    virtual MBOOL   signal(EPipeSignal eSignal, const MINT32 mUserKey);
    virtual MBOOL   abortDma(PortID port, char const* szCallerName);

    virtual MUINT32 getIspReg(MUINT32 RegAddr, MUINT32 RegCount, MUINT32 RegData[], MBOOL bPhysical = MTRUE);
    virtual MUINT32 getIspReg(IspP1RegIo *pRegs, MUINT32 RegCount, MBOOL bPhysical = MTRUE);
    virtual MUINT32 getIspUniReg(IspP1RegIo *pRegs, MUINT32 RegCount, MBOOL bPhysical = MTRUE);

    MBOOL           addUser(char const* szCallerName);
    MBOOL           delUser(char const* szCallerName);
    MINT32          getTotalUserNum(void);
    inline MINT64   getTimeStamp_ns(MUINT64 i4TimeStamp_sec, MUINT32 i4TimeStamp_us) const
    {
        return  i4TimeStamp_sec * 1000000000LL + i4TimeStamp_us * 1000LL;
    }

    static MVOID    dumpSeninfDebugCB(MUINT32 ispModule, MUINT32 tgStat);
private:
    MBOOL           immediateEnque(QBufInfo const& rQBuf);


    typedef enum _E_FSM{
        op_unknown  = 0,
        op_init,
        op_cfg,
        op_start,
        op_stop,
        op_uninit,
        op_cmd,
        op_enque,
        op_deque,
        op_suspend
    } E_FSM;
    MBOOL           FSM_CHECK(E_FSM op, const char *callee);
    MBOOL           FSM_UPDATE(E_FSM op);
protected:
    INormalPipe*                    mpNormalPipe;
    //NSImageio3::NSIspio::ICamIOPipe* mpCamsvIOPipe;
    IHalSensor*                 mpHalSensor;
    const char*                 mpName;
    mutable Mutex               mEnQLock;         //protect enque() invoked by multi-thread
    mutable Mutex               mCfgLock;         //mutex

    MUINT32                     mBurstQNum;
    MUINT32                     mCamsvImgoBufSize;
    MINT32                      mTgInfo;
    MINT32                      mTgInfoCamsv;

public:
    MINT32                      mTotalUserCnt;
    MINT32                      mUserCnt[IOPIPE_MAX_NUM_USERS];
    char                        mUserName[IOPIPE_MAX_NUM_USERS][IOPIPE_MAX_USER_NAME_LEN];

    MUINT32                     mpSensorIdx;
    MUINT32                     mFeatureIdx;

private:
    MUINT32                     mOpenedPort;

    mutable Mutex               m_FSMLock;
    E_FSM                       m_FSM;

public:

    IMemDrv*                    mpIMem;
    MUINT32*                    mpbuf_camsv_mnum;
    IMEM_BUF_INFO*              mpbuf_camsv_imgo;
    IMEM_BUF_INFO*              mpbuf_camsv_imgo_fh;
    MUINTPTR*                   mp_org_va_imgo_fh;
    MUINTPTR*                   mp_org_pa_imgo_fh;

    static Mutex                SPipeGLock; // CamsvStatisticPipe Global Lock
    static Mutex                SPipeDbgLock; // for seninf dump callback
    static CamsvStatisticPipe*  pCamsvStatisticPipe[EPIPE_Sensor_RSVD][EPIPE_CAMSV_FEATURE_NUM];
    static NSCam::NSIoPipeIsp3::NSCamIOPipe::platSensorsInfo  *pAllSensorInfo;
    IHalSensor::ConfigParam     m_sensorConfigParam;
    MUINT                       mscenarioId;
    MUINT32                     m_subSample;
    MUINT32                     ring_cnt;
    MUINT32                     camsv_imgo_crop_x;
    MUINT32                     camsv_imgo_crop_y;
    MUINT32                     camsv_imgo_crop_floatX;
    MUINT32                     camsv_imgo_crop_floatY;
    MUINT32                     camsv_imgo_crop_w;
    MUINT32                     camsv_imgo_crop_h;
    MUINT32                     mErrorCode;
};

};
};
};
#endif

