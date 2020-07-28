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

#ifndef _MTK_PLATFORM_HARDWARE_MTKCAM_IOPIPE_CAMIO_NormalPipe_FRMB_H_
#define _MTK_PLATFORM_HARDWARE_MTKCAM_IOPIPE_CAMIO_NormalPipe_FRMB_H_

#include <isp_30/iopipe/CamIO/IHalCamIO.h>
#include <isp_30/iopipe/CamIO/INormalPipe.h>
//#include <mtkcam/drv/sensor_hal.h>
#include <imageio/IPipe.h>
#include <imageio/ICamIOPipe.h>
#include <imageio/ispio_utility.h>
#include <mtkcam/drv/def/ispio_sw_scenario.h>
#include <mtkcam/drv/def/ispio_port_index.h>
//#include "NormalPipe.h"     //include this for using class:normalpipe
#include <deque>
//for thread/semapher
#include <utils/threads.h>
#include <cutils/atomic.h>
#include <utils/KeyedVector.h>
#include <utils/Mutex.h>
#include <semaphore.h>
#include <pthread.h>
#include <mtkcam/def/PriorityDefs.h>
#include <mtkcam/drv/IHalSensor.h>
#include "imageio/ispio_stddef.h"
#include <drv/utils.h>

//#include "mtkcam/imageio/p1HwCfg.h"
//#include "mtkcam/drv/isp_drv.h"

//#define IOPIPE_SET_MODUL_REG(handle,RegName,Value) HWRWCTL_SET_MODUL_REG(handle,RegName,Value)
//#define IOPIPE_SET_MODUL_REGS(handle, StartRegName, size, ValueArray) HWRWCTL_SET_MODUL_REGS(handle, StartRegName, size, ValueArray)

//class ICamIOPipe;
/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSIoPipeIsp3 {
namespace NSCamIOPipe {

#ifndef USING_MTK_LDVT
#define USE_IMAGEBUF_HEAP
//#define ENABLE_FRAME_MGR
#endif
/******************************************************************************
 *
 *
 ******************************************************************************/
#define MaxPortIdx  2       //so far, only 2 dmao r supported in 1 TG
#define NPIPE_MAX_SENSOR_CNT        5
#define NPIPE_MAX_ENABLE_SENSOR_CNT 2
#define HAL3_IPBASE (1)
#if (HAL3_IPBASE == 1)
//reference pipeline/hwnode/p1node/P1Common.h: P1_EISO_MIN_HEIGHT, P1_RSSO_MIN_HEIGHT, P1_RRZO_MIN_HEIGHT
#define YsizeofSmallBuf (0xA1)
#define recordMax (6)
#define cntMax (0x10000000*4)

struct _replaceBuf_
{
    MUINT32 usedPA;
    MUINT32 unusedPA;
    unsigned long VA;
    MUINT32 filled;
};
#endif
typedef struct
{
   MUINT32                    mIdx;
   MUINT32                    mTypeforMw;
   MUINT32                    mDevId;//main/sub/main0/...
   IHalSensor::ConfigParam    mConfig;
   SensorStaticInfo           mSInfo; //static
   SensorDynamicInfo          mDInfo; //Dynamic
   MUINTPTR                   mOccupiedOwner;
}_sensorInfo_t;

class platSensorsInfo{
public :
    platSensorsInfo(void){
        mUserCnt = 0;
        mExistedSCnt = 0;
        mSList = NULL;
        memset(mSenInfo, 0, sizeof(_sensorInfo_t) * NPIPE_MAX_SENSOR_CNT);
    }


   IHalSensorList *mSList;
   MUINT32        mExistedSCnt;
   _sensorInfo_t  mSenInfo[NPIPE_MAX_SENSOR_CNT];
   MUINT32        mUserCnt;
};

class NormalPipe_FrmB_Thread;

class NormalPipe_FrmB : public virtual IHalCamIO, public INormalPipe_FrmB
{
friend class NormalPipe_FrmB_Thread;
public:
     //burstQnum is only for mw member to update,3a member do not need to touch this
     NormalPipe_FrmB(MUINT32* pSensorIdx, MUINT32 SensorIdxCnt, char const* szCallerName, MINT32 burstQnum);

//protected:
                   ~NormalPipe_FrmB(){};
public:
    virtual MVOID   destroyInstance(char const* szCallerName);

    virtual MBOOL   start();

    virtual MBOOL   stop();

    virtual MBOOL   init();

    virtual MBOOL   uninit();

    virtual MBOOL   enque(QBufInfo const& rQBuf);

    virtual MBOOL   deque(QBufInfo& rQBuf, MUINT32 u4TimeoutMs = 0xFFFFFFFF);

    virtual MBOOL   Reset();

    virtual MBOOL   abort();

    //one-time conifg
    virtual MBOOL   configPipe(QInitParam const& vInPorts);

    virtual MBOOL   sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3);

    virtual MBOOL   DummyFrame(QBufInfo const& rQBuf);

    virtual MBOOL   configFrame(QFrameParam const& rQParam){UNUSED(rQParam); return MFALSE;};

    //replace queue
    virtual MBOOL   replace(BufInfo const& bufOld, BufInfo const& bufNew){UNUSED(bufOld); UNUSED(bufNew); return MFALSE;};

    typedef enum{
        _DMAO_NONE   = 0x0,
        _IMGO_ENABLE = 0x1,
        _RRZO_ENABLE = 0x10,
    }eDMAO;
    typedef enum{
        _GetPort_Opened     = 0x1,
        _GetPort_OpendNum   = 0x2,
        _GetPort_Index      = 0x3,
    }ePortInfoCmd;
    MUINT32        GetOpenedPortInfo(NormalPipe_FrmB::ePortInfoCmd cmd,MUINT32 arg1=0);

    MUINT32        GetRingBufferDepth(void);
public:
    virtual MVOID   attach(ISignalConsumer *pConsumer, MINT32 sensorIdx, EPipeSignal eType);
    virtual MBOOL   wait(ISignalConsumer *pConsumer, MINT32 sensorIdx, EPipeSignal eType, uint64_t ns_timeout = 0xFFFFFFFF);
private:
    MBOOL   wait(MINT32 sensorIdx, EPipeSignal eType, uint64_t ns_timeout = 0x300000);

#if (HAL3_IPBASE == 1)
    MBOOL                   ReplaceToDummy(MUINT32 _OpenedIndex, BufInfo* pBuf, BufInfo* dumBuf, MUINT32* _doReplace);
    MVOID                   DummyReqReturn(MUINT32 _OpenedIndex, BufInfo* dumBuf, MUINT32* _doReplace);
#endif

public:
    virtual MUINT32 getIspReg(::ESoftwareScenario scen);
    virtual MUINT32 getSensorTg(MINT32 sensorIdx);
    virtual MVOID   abortDeque();

protected:
    MBOOL _setTgPixelAndDualMode(MUINT32 ScenId, MUINT32 fps);
    inline MINT64   getTimeStamp_ns(MUINT32 i4TimeStamp_sec, MUINT32 i4TimeStamp_us) const
    {
        return  i4TimeStamp_sec * 1000000000LL + i4TimeStamp_us * 1000LL;
    }
    MBOOL    checkEnque(QBufInfo const& rQBuf);
    MBOOL    checkDropEnque(QBufInfo const& rQBuf);
#if (HAL3_IPBASE == 1)
    MUINT32  checkReplacePA(MUINT32 portIdx, unsigned long u4BufVA); /*return PA to MW, input VA, output PA*/
#endif
#if 0
    void Lock(void) ;
    void UnLock(void);
#endif

private:
    MBOOL   flushIrq(MINT32 sensorIdx, EPipeSignal eType, MINT32 u4TimeoutMs = 3000);

public:
    class FrameMgr {
    /////
    protected:
#ifdef USE_IMAGEBUF_HEAP
        typedef android::DefaultKeyedVector<MINTPTR, IImageBuffer* >  MapType;
#else
        typedef android::DefaultKeyedVector<MINTPTR, BufInfo* >       MapType;
#endif

        MapType                 mvFrameQ;
        mutable android::Mutex  mFrameMgrLock;   //for frameMgr enque/deque operation
    /////
    public:
                                FrameMgr()
                                    : mvFrameQ()
                                    {}

        virtual                 ~FrameMgr(){};
        ////
        void                    init();
        void                    uninit();
#ifdef USE_IMAGEBUF_HEAP
        void                    enque(IImageBuffer* pframe);
        IImageBuffer*           deque(MINTPTR key);
#else
        void                    enque(const BufInfo& pframe);
        BufInfo*                deque(MINTPTR key);
#endif

    };


    MINT32                     mUserCnt;
    MUINT32                    mpSensorIdx[NPIPE_MAX_SENSOR_CNT];   //0/1/2
    MUINT32                    mpEnabledSensorCnt;

class QueueMgr {
    /////
    protected:
        std::deque<BufInfo>    m_v_pQueue;
        mutable android::Mutex mQueueLock;   //for frameMgr enque/deque operation
    /////
    public:
                                QueueMgr()
                                    : m_v_pQueue()
                                    {}

        virtual                 ~QueueMgr(){};
        ////
        void                    init();
        void                    uninit();
        //FIFO cmd
    typedef enum{
        eCmd_getsize = 0,
        eCmd_push = 1,
        eCmd_pop = 2,
        eCmd_front = 3,
        eCmd_end = 4,
        eCmd_at = 5,
        eCmd_pop_back = 6
    }QueueMgr_cmd;
        MBOOL                   sendCmd(QueueMgr_cmd cmd,MUINTPTR arg1,MUINTPTR arg2);
    };


protected:
    IHalSensor                 *mpHalSensor;
    IHalSensor::ConfigParam    m_sensorConfigParam;

    MBOOL                         mbTwinEnable;
    static MUINT32                mEnablePath;
    static unsigned long          mTwinEnable[CAM_SV_2+1][2];//record this & twin mode

    NSImageioIsp3::NSIspio::ICamIOPipe    *mpCamIOPipe;
    FrameMgr                   *mpFrameMgr;
    const char                 *mpName;
    MSize                      mTgOut_size[3];//user marco _TGMapping for array indexing
    MSize                      mImgoOut_size;
    MSize                      mRrzoOut_size;
    MSize                      mImgodOut_size;
    MSize                      mRrzodOut_size;
    MSize                      mCamSvImgoOut_size;
    MSize                      mCamSv2ImgoOut_size;
    mutable android::Mutex     mLock;           //mutex for container
    mutable android::Mutex     mDeQLock;         //mutex
    mutable android::Mutex     mEnQLock;         //protect enque() invoked by multi-thread
    mutable android::Mutex     mCfgLock;         //mutex
    MBOOL                      mConfigDone;

    //
    MINT32                     mBurstQNum;
    MBOOL                      mUpdateBQNumDone;
private:
    MBOOL                      m_bStarted;
    MBOOL                      m_bAbortDeque;
    MUINT32                    mRingBufDepth;   //total enabled dmao ring-buf-depth sum.
    MBOOL                      mDynSwtRawType;//1:dynamically switch processed/pure raw
    MBOOL                      mPureRaw;//0: processed raw, 1:pure raw

    //array indexing r mapping by macro:_PortMap(...)
    QueueMgr*                   mpEnqueRequest[MaxPortIdx];
    QueueMgr*                   mpDummyRequest[MaxPortIdx];
    QueueMgr*                   mpEnqueReserved[MaxPortIdx];
    //container of dequed buffer, in order to chk replace or not
    QueueMgr*                   mpDeQueue[MaxPortIdx];
#if (HAL3_IPBASE == 1)
    QueueMgr*                   mpEnDeCheck[MaxPortIdx]; //check deque buf is match with enque buf
#endif
    MUINT32                     mOpenedPort;
    MUINT32                     PortIndex[MaxPortIdx];  //record index in portmap.h
    MUINT32                     mPrvEnqSOFIdx;
    MBOOL                       m_b1stEnqLoopDone;
    MBOOL                       m_bN3DEn;  //  n3d on or off record
public:
    mutable android::Mutex      mEnqContainerLock;  //protect sw enque container which is accessed by drv_thread & other user threads
#if (HAL3_IPBASE == 1)
    mutable android::Mutex      mDmyContainerLock;  //protect sw dummy container which is accessed by drv_thread & deque threads
    mutable android::Mutex      mEnDeContainerLock;  //protect sw mpEnDeCheck which is accessed by enq thread & deque thread

    mutable android::Mutex      mEnqueMutex;
    mutable android::Condition  mEnqueCond;
    MBOOL                       mIsCfgUpdating;
#endif
    sem_t                       m_semConfigStart;
    NormalPipe_FrmB_Thread      *mpFrmB_Thread;
    fp_DropCB                   m_DropCB;
    void*                       m_returnCookie; //acompany with m_DropCB;
    MBOOL                       m_PerFrameCtl;//1:supports per frame control
#if (HAL3_IPBASE == 1)
    MUINT32                     m_BufIdx;
    MUINT32                     m_r_enqCnt[2]; // if buf is small, record into table at enque, 0:imgo, 1:rrzo
    MUINT32                     m_r_deqCnt[2]; // read record table at deque, check if PA is changed, 0:imgo, 1:rrzo
    _replaceBuf_*               m_replaceTable[recordMax][2]; // record which PA is replaced, keep (recordMax) enque, [][0]:imgo, [][1]:rrzo
#endif
};


class NormalPipe_FrmB_Thread
{
public:
    /**
        @param obj:obj address
    */
    NormalPipe_FrmB_Thread(NormalPipe_FrmB* obj);
    ~NormalPipe_FrmB_Thread(){};
    static NormalPipe_FrmB_Thread* CreateInstance(NormalPipe_FrmB* obj);
    MBOOL                   DestroyInstance(void);
    MBOOL                   init(void);
    MBOOL                   uninit(void);
    static MVOID*           IspEnqueThread(void* arg);
    MBOOL                   Start(void);
    MBOOL                   Stop(void);
    MBOOL                   updateFrameTime(MUINT32 timeMs, MUINT32 offset);
    MUINT32                 estimateFrameTime(void);
    MVOID                   EnqueRequest(MUINT32 sof_idx);
    MVOID                   ImmediateRequest(MBOOL bByPassHwEnq);
private:
    //QBufInfo*               ReplaceChk(QBufInfo* prQBuf);
    MBOOL                   Enque(BufInfo* pBuf);
    MBOOL                   DmaCfg(BufInfo* pParam);
    MBOOL                   TuningUpdate(MUINT32 magNum);
    MVOID                   FrameToEnque(MUINT32 _OpenedPort);
    MVOID                   DummyFrameToEnque(MUINT32 _OpenedPort);

    sem_t                   m_semEnqueThread;
    pthread_t               m_EnqueThread;
    mutable android::Mutex  mLock;//lock m_bStart
    MBOOL                   m_bStart;
    volatile MUINT32        mCnt;
    NormalPipe_FrmB*        m_pNormalpipe_FrmB;
    MUINT32                 m_EnqueCnt;
    MUINT32                 m_ShutterTimeMs[2];  // [0]: after wait sof update, [1]: user enque update;
    std::vector<MUINT32>    m_DropEnQ;
};

};
};
};
#endif

