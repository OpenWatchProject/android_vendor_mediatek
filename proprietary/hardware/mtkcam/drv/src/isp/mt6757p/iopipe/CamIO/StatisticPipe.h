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
#include <ispio_utility.h>

#include <imem_drv.h>

#include <deque>

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSIoPipe {
namespace NSCamIOPipe {

#ifndef USING_MTK_LDVT
#endif

/******************************************************************************
 *
 *
 ******************************************************************************/
#define STT_ENQ_IMM             (1)

#define MaxStatPortIdx          (5) //dmao ports
#define MAX_STAT_BUF_NUM        (4)
#define IMG_HEADER_SIZE         (64)
#define LOG_STR_LENGTH          (256)

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
    MUINT32         GetOpenedPortInfo(StatisticPipe::ePortInfoCmd cmd,MUINT32 arg1=0);

    static void     suspendCB(void *cookie, MBOOL rst);
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
    inline MINT64   getTimeStamp_ns(MUINT32 i4TimeStamp_sec, MUINT32 i4TimeStamp_us) const
    {
        return  i4TimeStamp_sec * 1000000000LL + i4TimeStamp_us * 1000LL;
    }
    static MUINT32  sttStateNotify(CAM_STATE_OP state, MVOID *Obj);
    static MUINT32  sttIdleStateHoldCB(CAM_STATE_OP state, MVOID *Obj);
private:
    MBOOL           immediateEnque(QBufInfo const& rQBuf);

public:
    typedef enum {
        eCmd_getsize = 0,
        eCmd_push = 1,
        eCmd_pop = 2,
        eCmd_front = 3,
        eCmd_end = 4,
        eCmd_at = 5,
        eCmd_pop_back = 6
    } QueueMgr_cmd;
    template <class _T>
    class QueueMgr {
        /////
        protected:
            std::deque<_T>          m_v_pQueue;
            mutable Mutex           mQueueLock;   //for frameMgr enque/deque operation
        /////
        public:
                                    QueueMgr()
                                        : m_v_pQueue()
                                        {}

            virtual                 ~QueueMgr(){};
            ////
            void                    init();
            void                    uninit();
            MBOOL                   sendCmd(QueueMgr_cmd cmd,MUINTPTR arg1,MUINTPTR arg2);
    };

protected:
    INormalPipe*                mpNormalPipe;
    NSImageio::NSIspio::ICamIOPipe* mpSttIOPipe;
    const char*                 mpName;
    //mutable Mutex               mLock;
    //mutable Mutex               mDeQLock;         //mutex
    mutable Mutex               mEnQLock;         //protect enque() invoked by multi-thread
    mutable Mutex               mCfgLock;         //mutex
    MBOOL                       mConfigDone;

    MUINT32                     mBurstQNum;
    MINT32                      mTgInfo;

public:
    MINT32                      mTotalUserCnt;
    MINT32                      mUserCnt[IOPIPE_MAX_NUM_USERS];
    char                        mUserName[IOPIPE_MAX_NUM_USERS][IOPIPE_MAX_USER_NAME_LEN];

    MUINT32                     mpSensorIdx;

private:
    MBOOL                       m_bStarted;

    MUINT32                     mOpenedPort;

    //array indexing r mapping by macro:_PortMap(...)
    #if (STT_ENQ_IMM == 1)
    QueueMgr<QBufInfo>*         mpDeqContainer[MaxStatPortIdx];
    #else
    QueueMgr<QBufInfo>*         mpEnqueRequest[MaxStatPortIdx];
    #endif

    MUINT32                     PortIndex[MaxStatPortIdx];  //record index in portmap.h

    //for log reduction
    char                        _STTDeqBuf[MaxStatPortIdx][LOG_STR_LENGTH];

public:

    #if (STT_ENQ_IMM == 0)
    mutable Mutex               mEnqContainerLock;  //protect sw enque container which is accessed by drv_thread & other user threads
    #endif
    mutable Mutex               mThreadIdleLock[MaxStatPortIdx];

    IMemDrv*                    mpIMem;
    vector<IMEM_BUF_INFO>       mpMemInfo[MaxStatPortIdx]; //index by _PortMap
    MINT32                      mMemNextLegalEnqIdx[MaxStatPortIdx];
    IMEM_BUF_INFO               mImgHeaderPool;
    QueueMgr<IMEM_BUF_INFO>     mpImgHeaderMgr[MaxStatPortIdx];
    MINT32                      mImgHeaderSize[MaxStatPortIdx];

    static Mutex                SPipeGLock; // StatisticPipe Global Lock
    static StatisticPipe*       pStatisticPipe[EPIPE_Sensor_RSVD];

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

