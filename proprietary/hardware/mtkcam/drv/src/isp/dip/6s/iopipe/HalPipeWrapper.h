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
#include <mtkcam/utils/std/Trace.h>
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
    ECmd_ENQUE_RDY, //receive enque ready, for preview case
    ECmd_ENQUE_VSSFRAME_RDY, //receive enque ready, for vss case
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
 * @struct FParam
 * @brief frame unit for enqueue/dequeue (one unit for one frame)
 * @details
 *
 ******************************************************************************/
struct FParam
{
    MINT32  p2cqIdx;        //p2 cq user use
    MINT32  p2RingBufIdx;
    MINT32  p2burstQIdx;
    EBufferTag  bufTag;     //blocking or vss
    MUINT32 frameNum;
    FParam()
        : p2cqIdx(0)
        , p2RingBufIdx(0)
        , p2burstQIdx(0)
        , bufTag(EBufferTag_Blocking)
        , frameNum(0)
    {}
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
    MINT32 frameNum;   //total number of frame units in a single package
    MINT32  dequedNum;      //number of dequeued frame
    MINT32 idx4semEQDone;
    char m_UserName[MAX_USER_NAME_LEN];
    list<FParam> m_FParamVec;
    FramePackage()
        : callerPipe(ESatremPipeID_None)
        , callerSensorDev(0x0)
        , pixID(0x0)
        , callerID(0x0)
        , frameNum(0)
        , dequedNum(0)
        , idx4semEQDone(0)
        , m_FParamVec()
    {
        memset((char*)m_UserName, 0x0, MAX_USER_NAME_LEN);
    }
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
    ELockEnum_EQFramePackList,
    ELockEnum_wDoMdpStartUnitList,
    ELockEnum_EnqueLock,
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
    MBOOL                   init(char const* szCallerName, NSCam::NSIoPipe::EStreamPipeID streamPipeID, DipUserParam UserParam = {EDipUserEnum_DEFAUT, 0, 0} , MUINT32 secTag=0);
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
    MBOOL                   uninit(char const* szCallerName, NSCam::NSIoPipe::EStreamPipeID streamPipeID, DipUserParam UserParam = {EDipUserEnum_DEFAUT, 0, 0} );

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
    MBOOL compareTuningWithWorkingBuffer(char const* filepath, MUINT32* workingBuf, MUINT32* pTuningIspReg, size_t size);
    MBOOL doEnque(FramePackage &framePack);
    static void MDPCallback(void* pParam);
    NSImageio::NSIspio::ERawPxlID PixelIDMapping(MUINT32 pixIdP2);

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
    //HalPipeWrapper_Thread* mHalPWrapperEQThread[EThreadProperty_NUM];   //enque thread
    HalPipeWrapper_Thread* mHalPMdpConfigThread[EThreadProperty_NUM];   //thread for mdp configuration in enque 
    HalPipeWrapper_Thread* mHalPWrapperDQThread[EThreadProperty_NUM];   //deque thread
    NSImageio::NSIspio::IPostProcPipe* mpPostProcPipe;
    list<FramePackage> mLDIPEQFramePackList;  //list for frame package enqueued from user using DIP
    list<FramePackage> mDIPVSSFramePackList;  //list for VSS frame package enqueued from user using DIP
    list<FParam> mLDIPwaitDoMDPStartUnitList; //list for frame unit whaiting for mdp start

    MINT32 mFps;
    MINT32 mDIPEQThreadIdx; //index for adopting enque thread when handling DIP related buffer
    mutable Mutex mLock;
    pthread_mutex_t mMutex_EQFramePackList;                     //mutex to protect the list that preserves frame packages enqueued from user
    pthread_mutex_t mMutex_waitDoMdpStartUnitList;           //mutex to protect the list that preserves frame units which are waiting for mdp start procedure
    pthread_mutex_t mMutex_VencPortCnt;                 //
    pthread_mutex_t mEnqMutex;                 //mutex to protect the list that preserves frame packages which are dequeued done
#if 0
    volatile MINT32     mVencPortCnt;
    sem_t               mSemVencPortCnt;

    //enque blocking check
    sem_t mSemEQDone[_SEM_EQDONE_NUM_];
    int sem_index;
    pthread_mutex_t mMutex_SemEQDone;
#endif
    //
    //
    //volatile MBOOL mbVssOccupied;    //record vss path is occupiede or not
    //sem_t               mSemVssOccupied; // For Vss Concurrency Check
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
     //MBOOL enqueJudgement();
    /**
     * @brief Enqueue flow
     *
     * @details
     *
     * @note
     */     
     //MINT32 doEnque(FramePackage &framePack);

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
     MBOOL dequeJudgement(list<FramePackage>& FramePackListRef);

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

    //NSImageio::NSIspio::ERawPxlID PixelIDMapping(MUINT32 pixIdP2);
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

