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
#ifndef _HWSYNC_DRV_IMP_H_
#define _HWSYNC_DRV_IMP_H_
//-----------------------------------------------------------------------------
//------------Thread-------------
#include <pthread.h>
#include <semaphore.h>
//-------------------------------
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/common.h>
#include <utils/threads.h>
#include <list>
#include <vector>
#include <map>
//
#include <mtkcam/drv/IHwSyncDrv.h>
#include <mtkcam/drv/IHalSensor.h>
#include "isp_drv.h" //for wait vsync only
//#include "isp_drv_stddef.h"

//-----------------------------------------------------------------------------
using namespace std;
using namespace android;
using namespace NSCam;
//-----------------------------------------------------------------------------

//support sensor type: delay time must <= this define
#define MAX_SUPPORT_FRAMELENGTH_DELAY_NUM 2

/******************************************************************************
 *
 * @enum HW_SYNC_STATE_ENUM
 * @brief cmd enum for sendCommand.
 * @details
 *
 ******************************************************************************/
typedef enum{
    HW_SYNC_STATE_NONE          = 0X00,
    HW_SYNC_STATE_READY2RUN     = 0X01,
    HW_SYNC_STATE_SYNCING       = 0x02,
    HW_SYNC_STATE_READY2LEAVE   = 0x03,
    HW_SYNC_STATE_MODESWITCH    = 0x04,
    HW_SYNC_STATE_IDLE                       = 0x05, //the duration between AE exection stage
}HW_SYNC_STATE;

/******************************************************************************
 *
 * @enum ExecCmd
 * @brief enum for execution stage.
 * @details
 *
 ******************************************************************************/
enum ExecCmd
{
    ExecCmd_UNKNOWN = 0,
    ExecCmd_DOSYNC,
    ExecCmd_SET_SYNC_FRMTIME_MAIN1,
    ExecCmd_SET_SYNC_FRMTIME_MAIN2,
    ExecCmd_LEAVE
};

/******************************************************************************
 *
 * @struct N3D_INFO
 * @brief parameter for n3d registers.
 * @details
 *
 ******************************************************************************/
struct N3D_INFO
{
    MUINT32 order;                  ///<the leading situation at the current frame(1 for vs1 leads vs2, and 0 for vs2 leads vs1)
    //trick point
    //the different value calculated from n3d DiffCNT_reg is the value of pre frame,
    //but the order[bit 31] we get is the situation of current frame
    //we could use dbgPort and cnt2 to estimate the different value of current frame
    MUINT32 vdDiffCnt;
    MUINT32 vdDiffTus;
    MUINT32 vs_regCnt[2];           ///<the time period count of sensor1 (main sensor) and sensor2 (main2 sensor)
                                    ///<vs_reg Cnt(n) actually is vs count(n-1) due to double buffer is adopted.
    MUINT32 vs_regTus[2];
    MUINT32 vs2_vs1Cnt;             ///<debug port is the count value of sensor2 sampled by sensor1 (unit:count)
    MUINT32 vs2_vs1Tus;             ///<debug port is the count value of sensor2 sampled by sensor1 (unit:us)
    //
    N3D_INFO()
    : order(0x0)
    , vdDiffCnt(0x0)
    , vdDiffTus(0x0)
    , vs_regCnt{0x0,0x0}
    , vs_regTus{0x0,0x0}
    , vs2_vs1Cnt(0x0)
    , vs2_vs1Tus(0x0)
    {}
};


/**
    N3D input source enum
*/
typedef enum{
    E_TG_A  = 0,
    E_TG_B  = 1,
    E_TG_MAX= 2,
}E_N3D_SOURCE;

/**
    sensor device.
*/
typedef enum{
    SEN1    = 0,    // mapping to info of sensor which is cfged 1st
    SEN2    = 1,    //mapping to info of sensor which is cfged 2nd
    MAX_SEN = 2,
}E_SENDEV;

typedef struct _T_DATACfg
{
    MUINT32 FrameTime[MAX_SEN]; // 2nd adjust frame time , in order to recover sensor's fps
    MUINT32 expTime[MAX_SEN];   // shutter speed , input by AE at 1st adjust frame time
}T_DATACfg;


class HWSyncInit;

/******************************************************************************
 *
 * @class HWSyncDrv Derived Class
 * @brief Driver to sync VDs.
 * @details
 *
 ******************************************************************************/
class HWSyncDrvImp : public HWSyncDrv
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    HWSyncDrvImp();
    virtual ~HWSyncDrvImp();
//
public:
    /**
     * @brief get the singleton instance
     *
     * @details
     *
     * @note
     *
     * @return
     * A singleton instance to this class.
     *
     */
    static HWSyncDrv*   getInstance(void);
    /**
     * @brief destroy the pipe wrapper instance
     *
     * @details
     *
     * @note
     */
    virtual void        destroyInstance(void);
    /**
     * @brief init the hwsync driver
     *
     * @details
     * @param[in] sensorIdx: sensor index (0,1,2..).
     *
     * @note
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure.
     */
    virtual MINT32      init(HW_SYNC_USER_ENUM user, MUINT32 sensorIdx, MINT32 aeCyclePeriod = -1);
    /**
     * @brief uninit the hwsync driver
     *
     * @details
     * @param[in] sensorIdx: sensor index(0,1,2..).
     *
     * @note
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure.
     */
    virtual MINT32      uninit(HW_SYNC_USER_ENUM user, MUINT32 sensorIdx);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  General Function.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /**
     * @brief sendCommand to change setting
     *
     * @details
     * @param[in] cmd: command.
     * @param[in] senDev: sensor dev.
     * @param[in] senScen: sensor scenario.
     *
     * @note
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure.
     */
    virtual MINT32      sendCommand(HW_SYNC_CMD_ENUM cmd,MUINT32 senDev,MUINTPTR senScen, MUINTPTR currfps, MUINTPTR arg4);


private:
    /**
     * @brief mode changed
     *
     * @details
     *
     * @note
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure.
     */
    virtual MINT32      changeMode(MBOOL bTrigInitFlow = MTRUE);

    /**
     * @brief sync via set dummy line
     *
     * @details
     *
     * @note
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure.
     */
    virtual MINT32      doOneTimeSync(void);
    /**
     * @always enalrge frmT.
     *
     * @details
     *
     * @note
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure.
     */
    virtual MUINT32     doGeneralSync_AE(MUINT32 arg1, MUINTPTR aeg2, MUINTPTR arg3);
    /**
     * @bi-direction frmT control.  can shirnk/enlarge frmT
     *
     * @details
     *
     * @note
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure.
     */
    virtual MUINT32     doGeneralSync2_AE(MUINT32 arg1, MUINTPTR aeg2, MUINTPTR arg3);
    virtual MUINT32     checkNeedSync_INIT(void);
    /**
     * @brief check difference of VDs to judge that we need to do sync or not
     *
     * @details
     *
     * @note
     *
     * @return
     * - 0 indicates the difference is larger than threshold;
     * - others indicates the difference is smaller than threshold.
     */

    virtual MINT32     checkNeedSync_AE(MUINT32 arg1, MUINTPTR arg2, MUINTPTR arg3);

    //
    virtual MUINT32     calPixCnt2US(MUINT32 n3dCnt);

     /**
     * @brief calculate the current fps of sesnor
     *
     * @details
     *
     * @note
     *
     * @return
     * - MTRUE indicates yes;
     * - MFALSE indicates no.
     */
    virtual MINT32      calCurrentFps(int index);

    typedef enum{
        E_GET_SOF_CNT = 0,
        E_GET_VSYNC_CNT
    }E_CMD;

    MUINT32 DeviceInfo(E_CMD cmd,MUINT32 tgIdx);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Thread Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    /**
     * @brief Create a thread dedicated for sync.
     *
     * @details
     *
     * @note
     *
     * @return
     *
     */
    MVOID createThread();
    /**
     * @brief Destroy the thread dedicated for sync.
     *
     * @details
     *
     * @note
     *
     * @return
     *
     */
    MVOID destroyThread();
    /**
     * @brief add command to execute
     *
     * @details
     * @param[in] cmd: command to be executed.
     *
     * @note
     *
     * @return
     * - MTRUE indicates the difference is larger than threshold;
     * - MFALSE indicates the difference is smaller than threshold.
     */
    virtual MVOID addExecCmdMain1(ExecCmd const &cmd);
    virtual MVOID addExecCmdMain2(ExecCmd const &cmd);
    /**
     * @brief clear all the command
     *
     * @details
     *
     * @note
     *
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MVOID clearExecCmdsMain1();
    virtual MVOID clearExecCmdsMain2();
     /**
     * @brief get the first command in cmd list.
     *
     * @details
     *
     * @note
     *
     * @return
     *
     */
    virtual MBOOL get1stCmdMain1(ExecCmd &cmd);
    virtual MBOOL get1stCmdMain2(ExecCmd &cmd);
      /**
      * @brief Thread loop for dequeuing buffer.
      *
      * @details
      *
      * @note
      *
      * @return
      *
      */
     static  MVOID*  onThreadLoopMain1(MVOID*);
     static  MVOID*  onThreadLoopMain2(MVOID*);

     /**
      * @brief Set sync frame time
      *
      * @details
      *
      * @note
      *
      * @return
      *
      */
     virtual MINT32 setSyncFrmTimeMain1();
     virtual MINT32 setSyncFrmTimeMain2();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Variables.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    sem_t      mSemHWSyncLvMain1;
    sem_t      mSemHWSyncLvMain2;
    sem_t      mSemJumpoutSync;
    sem_t      mSem2Idle;
    sem_t      mSemGeneralSyncDoneMain1; //JJ, for desable hwsync to judge wether sync is done or not
    sem_t      mSemGeneralSyncDoneMain2; //JJ, for desable hwsync to judge wether sync is done or not
    bool        mbPostSem4Idle;
    //stastic vector<MUINT32> TG_Shutter;
    //stastic vector<MUINT32> Dev_temp;
    friend class HWSyncInit;

private:
    //[general]
    mutable  Mutex      mLock;
    volatile MINT32     mUser;
    IHalSensor*         mHalSensor;
    IHalSensorList*     mHalSensorList;
    vector<MUINT32>     mSensorIdxList; //
    MUINT32             mSensorDevArray[2]; //sorted by TG(0: TG1 -> dev1, 1: TG2 -> dev2)
    map<MUINT32, MUINT32> mSensorDevMapHwsyncIndex; // Key: dev id, Value: index id, where index 0 is TG1, index1 is TG2
    MUINT32             mSensorTgArray[E_TG_MAX];  //sorted by the same order with mSensorIdxList (0:mSensorDevArray[0], 1:mSensorDevArray[1], maxNum=2)
    MINT32              mSensorCurFpsArray[2]; //real current fps of sensor, for DBG information, UNIT: 10 base
    MUINT32             mSensorScenArray[2];    //keep this info for sen_drv to cfg fps by scenario

    HW_SYNC_STATE       mState;
    MINT32              mRemainUninitIdx;    //remain starting index for uninit
    MUINT32             mAECyclePeriod;

    MUINT32             mFrameTimeDelayFrame[MAX_SEN]; // the delay time of framelength for sensor, which is sensor-dependent value. Need query it from sensor driver
    MUINT32             mAECalledNum; // record the AE thread called get frame time #, only do at the first time
    MBOOL               mPreAdjustFrmTime; // record whether previous hw-sync bypass or not
    MUINT32             mCurFrameTime[MAX_SEN]; // frame time setting of Cur hw-sync result  for A / B, this value maybe a predicted value when sensor's delay time > 1
    N3D_INFO            mN3dInfo;
    T_DATACfg           m2ndData;       //data for 2nd adjust frame time, in order to recover sensor's fps

    vector<MUINT32>     TG_Shutter;
    vector<MUINT32>     Dev_temp;

    //[thread]
    mutable  Mutex      mThreadMtxMain1;
    pthread_t           mThreadMain1;
    list<ExecCmd>       mExecCmdListMain1;
    Condition           mExecCmdListCondMain1;


    mutable  Mutex      mThreadMtxMain2;
    pthread_t           mThreadMain2;
    list<ExecCmd>       mExecCmdListMain2;
    Condition           mExecCmdListCondMain2;


    IspDrv*             m_pIspDrv;   //for wait vsync only
    MUINT32             m_MaxFrameTime; //unit : us

    E_SMI_LEVEL         m_DVFS;

    volatile MINT32     mSyncCount;

    class DBG_Thread
    {
        public:
            DBG_Thread(void){DBG_T = 0;m_working_tg = 0;};
            virtual ~DBG_Thread(){};

            static MVOID*   DBG_Trig(MVOID*);

            pthread_t       DBG_T;
        public:
            MUINT32         m_working_tg;
    };
    DBG_Thread          m_DBG_t;
};


/******************************************************************************
 *
 * @class HWSyncDrv init Class
 * @brief
 * @details
 *
 ******************************************************************************/

class HWSyncInit
{
public:
    HWSyncInit(void* pUserObj);
    virtual ~HWSyncInit();

            MBOOL   Trig(void);

            typedef enum{
                E_RESET_VSYNC_CNT = 0,
                E_GET_VSYNC_CNT,
            }E_CMD;
            MUINT32 DeviceInfo(E_CMD cmd,MUINT32 tgIdx);
private:
    static  MVOID*  InitThread_1(MVOID*);
    static  MVOID*  InitThread_2(MVOID*);
            MBOOL   InitSync(void); //for TG2 sw start too late

    static  void fillIspWaitIrqType(MUINT32 tgIdx, ISP_DRV_WAIT_IRQ_STRUCT* waitIrq);

public:
    MUINT32         m_Shutter[E_TG_MAX];

    MUINT32         mThread1_TG;
    MUINT32         mThread2_TG;
private:
    HWSyncDrvImp*   m_this;

    pthread_t       m_Thread_1;
    pthread_t       m_Thread_2;

    MUINT32         m_order;

    sem_t           m_Sem_1;
    sem_t           m_Sem_2;

    MUINT32         m_bRst;     //if rst = 2 , sw timing shift, using legacy sync method
    Mutex           m_ThreadMtx;

    MUINT32         m_DiffThres;    //us

    E_N3D_SOURCE    m_MaxSource;    //0:TG1, 1:TG2
    MUINT32         m_MaxFrameCnt;  //unit: clk
};


//-----------------------------------------------------------------------------
#endif


