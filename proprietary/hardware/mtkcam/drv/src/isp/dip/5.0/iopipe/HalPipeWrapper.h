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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_POSTPROC_HALPIPEWRAPPER_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_POSTPROC_HALPIPEWRAPPER_H_
//
//------------Thread-------------
#include <pthread.h>
#include <semaphore.h>
//-------------------------------
//
#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/common.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <utils/threads.h>
#include <sys/prctl.h>

#include <list>
#include <vector>
#include <string>
//
#include <mtkcam/drv/iopipe/Port.h>
#include <mtkcam/drv/iopipe/PostProc/IHalPostProcPipe.h>
#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>
#include <ispio_pipe_ports.h>
#include <ispio_pipe_scenario.h>
#include <mtkcam/drv/IHalSensor.h>
#include <isp_drv_dip_platform.h>
#include <DipUtility.h>
#include <dip_reg.h>

#include <WpeStream.h>


using namespace std;
using namespace android;

#define _no_matter_ 0
#define _SEM_EQDONE_NUM_ (NSImageio::NSIspio::eScenarioID_NUM*2)
#define _DIP_NUM_ DIP_HW_MAX
#define _DIP_HW_A_ DIP_HW_A

/*******************************************************************************
* real implement in imageio
********************************************************************************/
namespace NSImageio{
namespace NSIspio{
    class IPostProcPipe;
};
};

#define PR_SET_NAME 15
#define HIGH_FPS_THRESHOLD 480

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSIoPipe {
namespace NSPostProc {

/******************************************************************************
 *
 * @enum EThreadType
 * @brief type for each enque/deque thread
 * @details
 *
 ******************************************************************************/
enum EThreadType
{
    EThreadType_NONE = -1,
    EThreadType_ENQUE = 0,
    EThreadType_MDPSTART,
    EThreadType_DEQUE,
    EThreadType_NUM
};

/******************************************************************************
 *
 * @enum EThreadProperty
 * @brief property for each enque/deque thread, to handle which kind of hw 
 * @details
 *
 ******************************************************************************/
enum EThreadProperty
{
    EThreadProperty_DIP_1=0,
    EThreadProperty_NUM,     //should move after EThreadProperty_WarpEG in rocky
    EThreadProperty_DIP_2,              //for rocky
    EThreadProperty_WarpEG=EThreadProperty_DIP_2, //for rocky    
};

/******************************************************************************
 *
 * @enum EHWCategory
 * @brief HW category for each frame(adopt which hw module category)
 * @details
 *
 ******************************************************************************/
enum EHWCategory
{
    EHWCategory_NONE = 0x0000,
    EHWCategory_DIP_A = 0x0001,
    EHWCategory_DIP_B = 0x0010,  //for rocky
    EHWCategory_DIP_A_B = (EHWCategory_DIP_A|EHWCategory_DIP_B), //for rocky
    EHWCategory_WarpEG = 0x8000, //for rocky
};

/******************************************************************************
 *
 * @enum ECmd
 * @brief enum for enqueue/dequeue signal control.
 * @details
 *
 ******************************************************************************/
enum ECmd
{
    ECmd_UNKNOWN = 0,
    ECmd_ENQUE_REQ, //receive enque request
    ECmd_MDPSTART_REQ,//isp config ready, and request to do mdp configuration
    ECmd_ENQUE_RDY, //receive enque ready
    ECmd_UNINIT
};

/******************************************************************************
 *
 * @struct UserInfo
 * @brief caller information about the user who uses the pass2 pipe.
 * @details
 *
 ******************************************************************************/
struct UserInfo
{
    EStreamPipeID pipeID;
    char name[32];
    /*
    UserInfo()
    : pipeID(ESatremPipeID_None)
    , name({'\0'})
    {}
    */
};


/******************************************************************************
 *
 * @struct PortTypeMapping
 * @brief mapping relationship from cam port type to imageio port type.
 * @details
 *
 ******************************************************************************/
struct PortTypeMapping
{
    NSCam::NSIoPipe::EPortType      eCamPortType;
    NSImageio::NSIspio::EPortType   eImgIOPortType;
};

/******************************************************************************
 *
 * @enum bufferTag
 * @brief mapping relationship from sw scenario to hw path.
 * @details
 *
 ******************************************************************************/

enum EBufferTag
{
    EBufferTag_Blocking=1,
    EBufferTag_Vss=2
};


/******************************************************************************
 *
 * @struct FramePackage
 * @brief frame package for enqueue/dequeue (one package may contain multiple frames)
 * @details
 *
 ******************************************************************************/
struct FramePackage
{
    EStreamPipeID callerPipe;   //need or not???
    MUINT32 callerSensorDev;
    MINT32 pixID;
    MUINT32 callerID;       //different thread, special buffer id to recognize the buffer source(in default we use address of dequeuedBufList from caller)
    QParams rParams;        //image buffer information
    MINT32  p2cqIdx;        //p2 cq user use
    MINT32  p2cqDupIdx;     //p2 cq duplicate index(pinpon here)
    MINT32 frameNum;   //total number of frame units in a single package
    MINT32  dequedNum;      //number of dequeued frame
    MINT32  idxofwaitDQFrmPackList; //element index in waitDQFramePackageList
    MINT32 idx4semEQDone;
    char m_UserName[MAX_USER_NAME_LEN];
    FramePackage()
        : callerPipe(ESatremPipeID_None)
        , callerSensorDev(0x0)
        , pixID(0x0)
        , callerID(0x0)
        , p2cqIdx(0x0)
        , p2cqDupIdx(0x0)
        , frameNum(0)
        , dequedNum(0)
        , idxofwaitDQFrmPackList(0)
        , idx4semEQDone(0)
    {
        memset((char*)m_UserName, 0x0, MAX_USER_NAME_LEN);
    }
};

/******************************************************************************
 *
 * @struct FrameUnit
 * @brief frame unit for enqueue/dequeue (one unit for one frame)
 * @details
 *
 ******************************************************************************/
struct FrameUnit
{
    NSImageio::NSIspio::EDrvScenario drvScen; //
    MUINT32 portEn;         //enable dmas
    MINT32  p2RingBufIdx;
    MINT32  p2cqIdx;        //p2 cq user use
    MINT32  p2cqDupIdx;     //p2 cq duplicate index(pinpon here)
    MINT32  p2BurstQIdx;
    MUINT32 callerID;       //different thread, special buffer id to recognize the buffer source(in default we use address of dequeuedBufList from caller)
    EBufferTag  bufTag;     //blocking or vss
    MINT32  idxofwaitDQFrmPackList; //element index in waitDQFramePackageList
    FrameUnit()
        : drvScen(NSImageio::NSIspio::eDrvScenario_P2A)
        , portEn(0x0)
        , p2RingBufIdx(0)
        , p2cqIdx(0)
        , p2cqDupIdx(0)
        , p2BurstQIdx(0)
        , callerID(0x0)
        , bufTag(EBufferTag_Blocking)
        , idxofwaitDQFrmPackList(0)
    {}
};

/******************************************************************************
 *
 * @struct MdpStartInfo
 * @brief frame infor for mdp start flow
 * @details
 *
 ******************************************************************************/
struct MdpStartInfo
{
    FrameUnit frameUnit;
    MINT32 frameNum;   //total number of frame units in a single package
    MdpStartInfo()
        : frameNum(0)
    {}
};


/******************************************************************************
 *
 * @enum EBufferListTag
 * @brief enum for recognize buffer list type.
 * @details
 *
 ******************************************************************************/
enum EBufferListTag
{
    EBufferListTag_UNKNOWN = 0,
    EBufferListTag_Package,
    EBufferListTag_Unit
};

/******************************************************************************
 *
 * @enum ELockEnum
 * @brief enum for recognize lock type.
 * @details
 *
 ******************************************************************************/
enum ELockEnum
{
    ELockEnum_VssOccupied        = 0,
    ELockEnum_EQFramePackList,
    ELockEnum_wDQFramePackList,
    ELockEnum_wDoMdpStartUnitList,
    ELockEnum_wDQFrameUnitList,
    ELockEnum_DQFramePackList,
    ELockEnum_VencPortCnt,
    ELockEnum_SemEQDone
};

class HalPipeWrapper_Thread;

/******************************************************************************
 *
 * @class HalPipeWrapper
 * @brief Wrapper for user with PostProc Pipe.
 * @details
 *
 ******************************************************************************/
class HalPipeWrapper
{
    friend  class HalPipeWrapper_Thread;    //

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    ////                    Constructor.
    HalPipeWrapper();
    ////                    Destructor.
    /**
     * @brief Disallowed to directly delete a raw pointer.
     */
    ~HalPipeWrapper() {};

public:     ////                    Instantiation.
    /**
     * @brief Create the instance
     *
     * @details
     *
     * @note
     *
     * @return
     * An instance to this class.
     *
     */
    static HalPipeWrapper*          createInstance();

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
    static HalPipeWrapper*          getInstance();

    /**
     * @brief destroy the pipe wrapper instance
     *
     * @details
     *
     * @note
     */
    MVOID                   destroyInstance();

    /**
     * @brief init the pipe wrapper
     *
     * @details
     *
     * @note
     *
     * @param[in] swScenario: sw scenario enum.
     * @param[in] callerPostProc: postproc pipe object for caller.
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().
     */
    MBOOL                   init(char const* szCallerName, NSCam::NSIoPipe::EStreamPipeID streamPipeID, MUINT32 secTag=0);
    /**
     * @brief uninit the pipe wrapper
     *
     * @details
     *
     * @note
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().
     */
    MBOOL                   uninit(char const* szCallerName, NSCam::NSIoPipe::EStreamPipeID streamPipeID);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Attributes.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    /**
     * @brief get the last error code
     *
     * @details
     *
     * @note
     *
     * @return
     * - The last error code
     *
     */
    MERROR                  getLastErrorCode() const ;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Buffer Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    /**
     * @brief En-queue a request into the pipe.
     *
     * @details
     *
     * @note
     *
     * @param[in] 
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure, and an error code can be retrived by \n
     *   getLastErrorCode().
     */
    MBOOL                   enque(NSCam::NSIoPipe::EStreamPipeID streamPipeID,
                                  MUINT32 sensorDev,
                                  MINT32 pixID,
                                  QParams const& rParams,
                                  vector<QParams>& callerDeQList,
                                  char* userName);

    /**
     * @brief De-queue a result from the pipe.
     *
     * @details
     *
     * @note
     *
     * @param[in] rParams: Reference to a result of QParams structure.
     * @param[in] callerID: ID to recognize the specific buffer.
     * @param[in] i8TimeoutNs: timeout in nanoseconds \n
     *      If i8TimeoutNs > 0, a timeout is specified, and this call will \n
     *      be blocked until a result is ready. \n
     *      If i8TimeoutNs = 0, this call must return immediately no matter \n
     *      whether any buffer is ready or not. \n
     *      If i8TimeoutNs = -1, an infinite timeout is specified, and this call
     *      will block forever.
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().
     */
    MBOOL                   deque(
                                  NSCam::NSIoPipe::EStreamPipeID streamPipeID,
                                  QParams& rParams,
                                  MUINTPTR callerID,
                                  MINT64 i8TimeoutNs
                                  );
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  General Function.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   /**
     * @brief getLock.
     *
     * @details
     *
     * @note
     *
     * @return
     */
    MVOID   getLock(ELockEnum lockType);
    /**
     * @brief releaseLock.
     *
     * @details
     *
     * @note
     *
     * @return
     */
    MVOID   releaseLock(ELockEnum lockType);
    /**
     * @brief Query a specific buffer from deququed buf list.
     *
     * @details
     *
     * @note
     *
     * @param[in] params: the package(including frame number) we want to check whether is dequeued or not and return \n
     * @param[in] staParams: additional statistic info
     * @param[in] callerID: the caller id of the frame we want to query
     *
     * @return
     *      - [true]   get matched buffer
     *      - [false]  do not get matched buffer
     */
    MBOOL   queryFrame(NSCam::NSIoPipe::EStreamPipeID streamPipeID, QParams& params, MINT32 callerID);
    /**
         * @brief send isp extra command
         *
         * @details
         *
         * @note
         *
         * @param[in] cmd: command
         * @param[in] arg1: arg1
         * @param[in] arg2: arg2
         * @param[in] arg3: arg3
         *
         * @return
         * - MTRUE indicates success;
         * - MFALSE indicates failure, and an error code can be retrived by sendCommand().
         */
    MBOOL sendCommand(MINT32 cmd, MINTPTR arg1=0, MINTPTR arg2=0, MINTPTR arg3=0);
    /**
     * @brief set frame rate
     *
     * @details
     *
     * @note
     *
     *
     * @return
     * - MTRUE  indicates success;
     * - MFALSE indicates failure;
     */
    MINT32                   setFps(MINT32 fps);
        /**
     * @brief check whether Vss is been occupied
     *
     * @details
     *
     * @note
     */
    MVOID                   checkVssStatus();	 // For Vss Concurrency Check

        /**
     * @brief get Postproc object according to which DIP
     *
     * @details
     *
     * @note
     */
    NSImageio::NSIspio::IPostProcPipe* getPostProcPipeInstance(unsigned int DipModule);    // For Vss Concurrency Check

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Variables.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    //[general]
    //for dbg convenience,dump current user when uninit()
    volatile MINT32     mInitCount;
    static UserInfo mUserInfo[MAX_PIPE_USER_NUMBER]; //support max user number for each pipe: MAX_PIPE_USER_NUMBER.
    HalPipeWrapper_Thread* mHalPWrapperEQThread[EThreadProperty_NUM];   //enque thread
    HalPipeWrapper_Thread* mHalPMdpConfigThread[EThreadProperty_NUM];   //thread for mdp configuration in enque 
    HalPipeWrapper_Thread* mHalPWrapperDQThread[EThreadProperty_NUM];   //deque thread
    NSImageio::NSIspio::IPostProcPipe* mpPostProcPipe;
    list<FramePackage> mLDIPEQFramePackList;  //list for frame package enqueued from user using DIP
    vector<FramePackage> mLDIPwaitDQFramePackList;  //list for frame package which waits for deque operation using DIP, could not use operator [] in list structure
    list<MdpStartInfo> mLDIPwaitDoMDPStartUnitList; //list for frame unit whaiting for mdp start
    list<FrameUnit> mLDIPwaitDQFrameUnitList;  //list for frame unit which waits for deque operation using DIP
    list<FramePackage> mLDIPDQFramePackList; //list for frame package dequeued from hw using DIP
    //list<FramePackage> mLWarpEnquePackageList;  //rocky, list for frame package using Warping
    MINT32 mFps;
    MINT32 mDIPEQThreadIdx; //index for adopting enque thread when handling DIP related buffer
    mutable Mutex mLock;
    pthread_mutex_t mMutex_VssOccupied;                     //mutex to protect the list that preserves frame packages enqueued from user
    pthread_mutex_t mMutex_EQFramePackList;                     //mutex to protect the list that preserves frame packages enqueued from user
    pthread_mutex_t mMutex_waitDQFramePackList;         //mutex to protect the list that preserves frame packages which are configured done
    pthread_mutex_t mMutex_waitDQFrameUnitList;           //mutex to protect the list that preserves frame units which are configured done
    pthread_mutex_t mMutex_waitDoMdpStartUnitList;           //mutex to protect the list that preserves frame units which are waiting for mdp start procedure
    pthread_mutex_t mMutex_DQFramePackList;                 //mutex to protect the list that preserves frame packages which are dequeued done
    pthread_mutex_t mMutex_VencPortCnt;                 //
    //MUINT32       mP2PixId;
    volatile MINT32     mVencPortCnt;
    sem_t               mSemVencPortCnt;
    //enque blocking check
    sem_t mSemEQDone[_SEM_EQDONE_NUM_];
    int sem_index;
    pthread_mutex_t mMutex_SemEQDone;
    //
    IHalSensorList* mHalSensorList;
    //
    volatile MBOOL mbVssOccupied;    //record vss path is occupiede or not
    sem_t               mSemVssOccupied; // For Vss Concurrency Check
    volatile MINT32     mWPE_InitCount;
    NSCam::NSIoPipe::NSWpe::WpeStream* mWpeStream;
    static UserInfo mWPEUserInfo[MAX_PIPE_WPEUSER_NUMBER]; //support max user number for each pipe: MAX_PIPE_USER_NUMBER.

    NSImageio::NSIspio::IPostProcPipe* mpDipPipe[_DIP_NUM_];


     int     m_iSaveReqToFile;
     unsigned int  m_iEnqueFrmNum;
     unsigned int  m_iDequeFrmNum;

 };


/******************************************************************************
 *
 * @class HalPipeWrapper_Thread
 * @brief Class for enque/deque thread control.
 * @details
 *
 ******************************************************************************/
class HalPipeWrapper_Thread
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    ////                    Constructor.
    HalPipeWrapper_Thread(HalPipeWrapper* obj, EThreadType threadType, EThreadProperty threadProperty);
    ////                    Destructor.
    /**
     * @brief Disallowed to directly delete a raw pointer.
     */
    ~HalPipeWrapper_Thread() {};

public:     ////                    Instantiation.
    /**
     * @brief Create the instance
     *
     * @details
     *
     * @note
     *
     * @return
     * An instance to this class.
     *
     */
    static HalPipeWrapper_Thread*          createInstance(HalPipeWrapper* obj, EThreadType threadType, EThreadProperty threadProperty);
    /**
     * @brief destroy the pipe wrapper instance
     *
     * @details
     *
     * @note
     */
    MVOID                   destroyInstance();
    /**
     * @brief Create a thread dedicated for dequeue.
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
     * @brief Destroy the thread dedicated for dequeue.
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
     * @brief Add command to cmd list.
     *
     * @details
     *
     * @note
     *
     * @param[in] cmd: Command that we want to add to the cmd list.
     *
     * @return
     *
     */
    MVOID addCmd(ECmd const &cmd);
     /**
     * @brief clear all commands in cmd list.
     *
     * @details
     *
     * @note
     *
     * @return
     *
     */
    MVOID clearCmds();
     /**
     * @brief get the first commands in cmd list.
     *
     * @details
     *
     * @note
     *
     * @return
     *
     */
    MBOOL getCmd(ECmd &cmd);
     /**
     * @brief Thread loop for processing flow (enqueue or dequeue).
     *
     * @details
     *
     * @note
     *
     * @return
     *
     */
    static  MVOID*  onThreadLoop(MVOID*);
    /**
     * @brief update the thread property
     *
     * @details
     *
     * @note
     */
     MVOID updateThreadProperty(EThreadProperty property);
    /**
     * @brief buffer judgement for enqueue flow
     *
     * @details
     *
     * @note
     */     
     MBOOL enqueJudgement();
    /**
     * @brief Enqueue flow
     *
     * @details
     *
     * @note
     */     
     MINT32 doEnque(FramePackage &framePack);

    /**
     * @brief buffer do mdp start flow 
     *
     * @details
     *
     * @note
     */     
     MBOOL doMdpStart();

    /**
     * @brief buffer judgement for dequeue flow
     *
     * @details
     *
     * @note
     */     
     MBOOL dequeJudgement();
    /**
     * @brief Dequeue flow
     *
     * @details
     *
     * @note
     */     
      MBOOL IsDMAPortSupport(MUINT32  ePortIndex);
     /**
      * @brief check p2 support this dma port or not.
      *
      * @details
      *
      * @note
      *
      * @return
      *
      */
     MINT32 doDeque(MINT32 dequeP2Cq, FrameUnit &frameunit, MINT32 elementIdx);
    /**
     * @brief wait condition do enque vss buffer again.
     *
     * @details
     *
     * @note
     *
     * @return
     *
     */
    MBOOL waitEnqueVSSAgain();

    /**
     * @brief wait condition do deque vss buffer again.
     *
     * @details
     *
     * @note
     *
     * @return
     *
     */
    MBOOL waitDequeVSSAgain();
    /**
     * @brief notify after deque no matter deque success or fail.
     *
     * @details
     * @param[in] bufferListTag: type of buffer list we want to pop out element.
     * @param[in] index: index of the element that we want to pop out.
     * @param[in] p2dequeCQidx: p2 cq index in that deque turn.
     * @param[in] p2dupCQidx: p2 duplicate cq index in that deque turn.
     * @param[in] cmd: deque success/fail.
     * @param[in] callerID: caller ID(enque-threadID).
     *
     * @note
     *
     * @return
     *
     */
   MBOOL eraseNotify(EBufferListTag bufferListTag,
                                MINT32 index,
                                MINT32 p2CQ,
                                MINT32 p2dupCQidx,
                                NSImageio::NSIspio::EPIPE_P2BUFQUECmd cmd=NSImageio::NSIspio::EPIPE_P2BUFQUECmd_DEQUE_SUCCESS,
                                MUINT32 callerID=0x0);
    NSImageio::NSIspio::ERawPxlID PixelIDMapping(MUINT32 pixIdP2);
public:
        pthread_t mThread;
        mutable  Mutex      mThreadMtx;
        mutable  Condition  mCmdListCond;
        mutable  Mutex      mVssEnQCondMtx;
        mutable  Mutex      mVssDeQCondMtx;
        mutable  Condition  mVssEnQCond;
        mutable  Condition  mVssDeQCond;
        HalPipeWrapper* mpHalPipeWrapper; //to query inforation from HalpipeWrapper Class
        NSImageio::NSIspio::IPostProcPipe* mpPostProcPipe;
        EThreadType mThreadType;    //thread type, enque or deque
        EThreadProperty mThreadProperty;    //property for handling dip or warping engine or other combinations
        std::string mThreadName;
        list<ECmd> mLCmdList; //cmd list
        sem_t mSemThread;      
};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSPostProc
};  //namespace NSIoPipe
};  //namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_POSTPROC_HALPIPEWRAPPER_H_

