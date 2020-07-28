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

#ifndef _MTK_PLATFORM_HARDWARE_MTKCAM_AAA_FAKEDRV_NORMALPIPE_H_
#define _MTK_PLATFORM_HARDWARE_MTKCAM_AAA_FAKEDRV_NORMALPIPE_H_

#include <utils/threads.h>
#include <cutils/atomic.h>
#include <semaphore.h>
#include <pthread.h>

#include "INormalPipe.h"

#include <IEventIrq.h>
#include "../EventIrq/DefaultEventIrq.h"

#include <deque>
#include <vector>
#include <list>
#include <map>



//#define IOPIPE_SET_MODUL_REG(handle,RegName,Value) HWRWCTL_SET_MODUL_REG(handle,RegName,Value)
//#define IOPIPE_SET_MODUL_REGS(handle, StartRegName, size, ValueArray) HWRWCTL_SET_MODUL_REGS(handle, StartRegName, size, ValueArray)

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSIoPipe {
namespace NSCamIOPipe {

//#ifndef USING_MTK_LDVT
#define USE_IMAGEBUF_HEAP
//#define ENABLE_FRAME_MGR
//#endif
/******************************************************************************
 *
 *
 ******************************************************************************/
#define MaxPortIdx                  (5) //dmao ports  //because of UFO, value change from 4 to 5
#define IMG_HEADER_BUF_NUM          (16)
#define IMG_HEADER_SIZE             (64)
#define SOF_IDX_MSB                 (0x80) //0~255

typedef enum {
   NPIPE_CAM_A      = 1,
   NPIPE_CAM_B      = 2,
   NPIPE_CAMSV_1    = 4,
   NPIPE_CAMSV_2    = 8,
} NPIPE_PATH_E;

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


// class platSensorsInfo {
// public :
//     platSensorsInfo (void) {
//         mUserCnt = 0;
//         mExistedSCnt = 0;
//         mSList = NULL;
//         memset(mSenInfo, 0, sizeof(mSenInfo));
//     }

//    IHalSensorList *mSList;
//    MUINT32        mExistedSCnt;
//    _sensorInfo_t  mSenInfo[IOPIPE_MAX_SENSOR_CNT];
//    MUINT32        mUserCnt;
// };

// class NormalPipe_Thread;

class NormalPipe : public INormalPipe {

public:
    NormalPipe(MUINT32 pSensorIdx, char const* szCallerName);

                   ~NormalPipe();
public:
    virtual MVOID   destroyInstance(char const* szCallerName);

    virtual MBOOL   init();

    virtual MBOOL   uninit();

    virtual MINT32  attach(const char* UserName);
    virtual MBOOL   wait(EPipeSignal eSignal, EPipeSignalClearType eClear, const MINT32 mUserKey,
                            MUINT32 TimeoutMs = 0xFFFFFFFF, SignalTimeInfo *pTime = NULL);

    virtual MBOOL   signal(EPipeSignal eSignal, const MINT32 mUserKey);

    virtual MBOOL   sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3);

    virtual MUINT32 getIspReg(MUINT32 RegAddr, MUINT32 RegCount, MUINT32 RegData[], MBOOL bPhysical = MTRUE);
    virtual MUINT32 getIspReg(IspP1RegIo *pRegs, MUINT32 RegCount, MBOOL bPhysical = MTRUE);

//     typedef enum {
//         _DMAO_NONE   = 0x0000,
//         _IMGO_ENABLE = 0x0001,
//         _RRZO_ENABLE = 0x0002,
//         _EISO_ENABLE = 0x0004,
//         _LCSO_ENABLE = 0x0008,
//         _UFEO_ENABLE = 0x0010,
//         //_RSSO_ENABLE = 0x400,
//     } eDMAO;
//     typedef enum {
//         _GetPort_Opened     = 0x1,
//         _GetPort_OpendNum   = 0x2,
//         _GetPort_Index      = 0x3,
//         _GetPort_ImgFmt     = 0x4,
//     } ePortInfoCmd;

public:
    MBOOL           addUser(char const* szCallerName);
    MBOOL           delUser(char const* szCallerName);

//     virtual MBOOL   abortDma(PortID port, char const* szCallerName);

//     virtual MUINT32 getIspUniReg(IspP1RegIo *pRegs, MUINT32 RegCount, MBOOL bPhysical = MTRUE);

//     //MBOOL           _setTgPixelAndDualMode(void);

//     MINT32          getTotalUserNum(void);
//     inline MINT64   getTimeStamp_ns(MUINT32 i4TimeStamp_sec, MUINT32 i4TimeStamp_us) const
//     {
//         return  i4TimeStamp_sec * 1000000000LL + i4TimeStamp_us * 1000LL;
//     }
//     static MVOID    dumpSeninfDebugCB(MUINT32 ispModule, MUINT32 tgStat);
// protected:
//     MBOOL           checkEnque(QBufInfo const& rQBuf);
//     MBOOL           checkDropEnque(QBufInfo *pQBuf);

// public:

//     class FrameMgr {
//     protected:
//         #ifdef USE_IMAGEBUF_HEAP
//         typedef DefaultKeyedVector<int, IImageBuffer* >  MapType;
//         #else
//         typedef DefaultKeyedVector<int, BufInfo* >       MapType;
//         #endif

//         MapType                 mvFrameQ;
//         mutable Mutex           mFrameMgrLock;   //for frameMgr enque/deque operation
//     public:
//                                 FrameMgr()
//                                     : mvFrameQ()
//                                     {}

//         virtual                 ~FrameMgr(){};
//         ////
//         void                    init();
//         void                    uninit();
//         #ifdef USE_IMAGEBUF_HEAP
//         void                    enque(IImageBuffer* pframe);
//         IImageBuffer*           deque(MINTPTR key);
//         #else
//         void                    enque(const BufInfo& pframe);
//         BufInfo*                deque(MINTPTR key);
//         #endif
//     };
//     typedef enum {
//         eCmd_getsize = 0,
//         eCmd_push = 1,
//         eCmd_pop = 2,
//         eCmd_front = 3,
//         eCmd_end = 4,
//         eCmd_at = 5,
//         eCmd_pop_back = 6
//     } QueueMgr_cmd;
//     template <class _T>
//     class QueueMgr {
//         /////
//         protected:
//             std::deque<_T>          m_v_pQueue;
//             mutable Mutex           mQueueLock;   //for frameMgr enque/deque operation
//         /////
//         public:
//                                     QueueMgr()
//                                         : m_v_pQueue()
//                                         {}

//             virtual                 ~QueueMgr(){};
//             ////
//             void                    init();
//             void                    uninit();
//             MBOOL                   sendCmd(QueueMgr_cmd cmd,MUINTPTR arg1,MUINTPTR arg2);
//     };

protected:
    NS3Av3::DefaultEventIrq*          mpEventIrq;
    //sensor Info
    NSCam::SensorStaticInfo     mSensorStaticInfo[3];
    IHalSensor*                 mpHalSensor;
    // IHalSensor::ConfigParam     m_sensorConfigParam;

    MUINT32                     mBurstQNum;

    // FrameMgr*                   mpFrameMgr;
    const char*                 mpName;

    MSize                       mTgOut_size[3];//user marco _TGMapping for array indexing
    MSize                       mImgoOut_size;
    MSize                       mRrzoOut_size;
    MSize                       mCamSvImgoOut_size;
    MSize                       mCamSv2ImgoOut_size;

    //mutable Mutex               mLock;           //mutex for container
    //mutable Mutex               mDeQLock;        //mutex
    mutable Mutex               mEnQLock;    //protect enque() invoked by multi-thread
    mutable Mutex               mCfgLock;    //mutex
    MBOOL                       mConfigDone;

public:
    MINT32                      mTotalUserCnt;
    MINT32                      mUserCnt[IOPIPE_MAX_NUM_USERS];
    char                        mUserName[IOPIPE_MAX_NUM_USERS][IOPIPE_MAX_USER_NAME_LEN];

    MUINT32                     mpSensorIdx;//0/1/2
    MUINT32                     mpSensorDev;//0/1/2

private:
    MBOOL                       m_bStarted;
    MBOOL                       mInitSettingApplied;
    MBOOL                       mDynSwtRawType;//1:dynamically switch processed/pure raw
    MBOOL                       mPureRaw;//0: processed raw, 1:pure raw
    MBOOL                       m_bN3DEn;   //n3d on/off record
    MUINT32                     mResetCount;
#define MAX_P1_RST_COUNT    (2)

    //array indexing r mapping by macro:_PortMap(...)
    // QueueMgr<QBufInfo>*         mpEnqueRequest[MaxPortIdx];
    // QueueMgr<QBufInfo>*         mpEnqueReserved[MaxPortIdx];
    // QueueMgr<QBufInfo>*         mpDeQueue[MaxPortIdx];
    MUINT32                     PortIndex[MaxPortIdx];  //record index in portmap.h
    // EImageFormat                PortImgFormat[MaxPortIdx];
    MUINT32                     mPrvEnqSOFIdx;
    MUINT32                     mReqDepthPreStart;  //enq count of every opened ports
    MBOOL                       m_b1stEnqLoopDone;

public:
    mutable Mutex               mEnqContainerLock;  //protect sw enque container which is accessed by drv_thread & other user threads
    mutable Mutex               mDeqContainerLock;  //protect sw deque container which is accessed by drv_thread & other user threads
    mutable Mutex               mEnqSofCBLock;
    // NormalPipe_Thread           *mpNPipeThread;
    // fp_DropCB                   m_DropCB;
    // void*                       m_returnCookie; //acompany with m_DropCB;
    // fp_EventCB                  m_SofCB;
    // void*                       m_SofCBCookie;

    // IMemDrv*                    mpIMem;
    // IMEM_BUF_INFO               mImgHeaderPool;
    // QueueMgr<IMEM_BUF_INFO>     mpImgHeaderMgr[MaxPortIdx];
    // vector<IMEM_BUF_INFO>       mvSttMem;

    static Mutex                NPipeGLock; // NormalPipe Global Lock
    static Mutex                NPipeCfgGLock; // NormalPipe Global Lock
    static Mutex                NPipeDbgLock; // for seninf dump callback
    static NormalPipe*          pNormalPipe[MAX_SENSOR_CNT];
    // static platSensorsInfo      *pAllSensorInfo;
    static MUINT32              mEnablePath;
    static MUINT32              mFirstEnableTG;
};

};
};
};
#endif

