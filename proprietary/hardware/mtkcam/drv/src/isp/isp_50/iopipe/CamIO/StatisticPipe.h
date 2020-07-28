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

#ifndef _MTK_PLATFORM_HARDWARE_MTKCAM_IOPIPE_CAMIO_STATPIPE_H_
#define _MTK_PLATFORM_HARDWARE_MTKCAM_IOPIPE_CAMIO_STATPIPE_H_

#include <utils/threads.h>
#include <cutils/atomic.h>
#include <semaphore.h>
#include <pthread.h>

#include <mtkcam/drv/iopipe/CamIO/IHalCamIO.h>
#include <mtkcam/drv/iopipe/CamIO/IStatisticPipe.h>

#include <IPipe.h>
#include <ICamIOPipe.h>

#include <imem_drv.h>

#include <deque>
#include <Cam/buf_que_ctrl.h>

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
class StatisticPipe_Thread;

class StatisticPipe : public IStatisticPipe {

public:
    StatisticPipe (MUINT32 pSensorIdx, char const* szCallerName);

                   ~StatisticPipe(){};
public:
    virtual MVOID   destroyInstance(char const* szCallerName);

    virtual MBOOL   start();

    virtual MBOOL   stop(MBOOL bNonblocking);

    virtual MBOOL   init();

    virtual MBOOL   uninit();

    virtual MBOOL   enque(QBufInfo const& rQBuf);

    virtual MBOOL   deque(PortID port, QBufInfo& rQBuf, ESTT_CACHE_SYNC_POLICY cacheSyncPolicy = ESTT_CacheInvalidByRange, MUINT32 u4TimeoutMs = 0xFFFFFFFF);

    virtual MBOOL   reset() {return MFALSE;};

    virtual MBOOL   configPipe(QInitStatParam const& vInPorts, MINT32 burstQnum = 1);

    virtual MBOOL   sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3);

    virtual MBOOL   suspend();

    virtual MBOOL   resume();

    typedef enum
    {
        eAAO = 0,
        eAFO,
        eFLKO,
        ePDO,
        ePSO,
        SttPortID_MAX,
    }eSttPortID;
    typedef enum {
        _DMAO_NONE         = 0x0000,
        _AAO_ENABLE        = 0x0100,
        _AFO_ENABLE        = 0x0200,
        _FLKO_ENABLE       = 0x0400,
        _PDO_ENABLE        = 0x0800,
        _PSO_ENABLE        = 0x1000,
        _CAMSV_IMGO_ENABLE = 0x2000,
    } eDMAO;
    typedef enum {
        _GetPort_Opened     = 0x1,
        _GetPort_OpendNum   = 0x2,
        _GetPort_Index      = 0x3,
    } ePortInfoCmd;
    typedef struct
    {
        MUINT32 SPipeAllocMemSum;
        MUINT32 SPipeFreedMemSum;
    }_SPipeMemInfo_t;

    MUINT32         GetOpenedPortInfo(StatisticPipe::ePortInfoCmd cmd,MUINT32 arg1=0);

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
    static MUINT32  sttStateNotify(CAM_STATE_OP state, MVOID *Obj);
    static MUINT32  sttIdleStateHoldCB(CAM_STATE_OP state, MVOID *Obj);
    static MUINT32  sttSuspendStateCB(CAM_STATE_OP state, MVOID *Obj);
    static MUINT32  sttEnqueStateCB(CAM_STATE_OP state, MVOID *Obj);
private:
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
    INormalPipe*                mpNormalPipe;
    NSImageio::NSIspio::ICamIOPipe* mpSttIOPipe;
    const char*                 mpName;
    const MUINT32               mBufMax;
    //mutable Mutex               mLock;
    //mutable Mutex               mDeQLock;         //mutex
    mutable Mutex               mEnQLock;         //protect enque() invoked by multi-thread
    mutable Mutex               mCfgLock;         //mutex

    MUINT32                     mBurstQNum;
    MINT32                      mTgInfo;

public:
    MINT32                      mTotalUserCnt;
    MINT32                      mUserCnt[IOPIPE_MAX_NUM_USERS];
    char                        mUserName[IOPIPE_MAX_NUM_USERS][IOPIPE_MAX_USER_NAME_LEN];

    MUINT32                     mpSensorIdx;

private:
    mutable Mutex               m_FSMLock;
    E_FSM                       m_FSM;

    MUINT32                     mOpenedPort;

    //array indexing r mapping by macro:_PortMap(...)
    QueueMgr<QBufInfo>*         mpDeqContainer[SttPortID_MAX];

    MUINT32                     PortIndex[SttPortID_MAX];  //record index in portmap.h

public:
    mutable Mutex               mThreadIdleLock[SttPortID_MAX];

    IMemDrv*                    mpIMem;
    vector<IMEM_BUF_INFO>       mpMemInfo[SttPortID_MAX]; //index by _PortMap
    IMEM_BUF_INFO               mImgHeaderPool;
    QueueMgr<IMEM_BUF_INFO>     mpImgHeaderMgr[SttPortID_MAX];

    static Mutex                SPipeGLock; // StatisticPipe Global Lock
    static StatisticPipe*       pStatisticPipe[EPIPE_Sensor_RSVD];
    static _SPipeMemInfo_t      mMemInfo;
	static const NSImageio::NSIspio::EPortIndex 	m_Slot2PortIndex[SttPortID_MAX];

    MBOOL configPipe_bh();
    pthread_t m_thread;
    void* m_thread_rst = NULL;
    std::vector<statPortInfo> mvp;
    QInitStatParam * mvInPorts;
};

};
};
};
#endif

