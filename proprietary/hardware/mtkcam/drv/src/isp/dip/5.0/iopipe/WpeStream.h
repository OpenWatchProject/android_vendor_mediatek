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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_WPE_WpeStream_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_WPE_WpeStream_H_
//

#include <mtkcam/drv/iopipe/PostProc/IWpeStream.h>
#include <mtkcam/drv/iopipe/Port.h>
#include <IWarpEnginePipe.h>


#include <ispio_pipe_ports.h>


#include <utils/threads.h>
#include <list>
#include <vector>
//------------Thread-------------
#include <sys/prctl.h>
#include <semaphore.h>
//-------------------------------
//#include <mtkcam/def/common.h>
#include "isp_datatypes.h"


#define CmdBuf_Cache  0   //CmdQ using by Cache is 1

#define WPE_TBLI_SIZE 256
#define MAX_WPE_ENQUEUE_USER  1
#define MAX_SUPPORT_WPE_FRAME_NUM  16


/*******************************************************************************
* real implement in imageio
********************************************************************************/
namespace NSImageio{
namespace NSIspio{
namespace NSWpe{
    class IWarpEnginePipe;
};
};
};

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSIoPipe {
namespace NSWpe {

//IWpeStream* getWpeStreamImp(char const* szCallerName);

struct WPEFrameUnit
{
    QParams rParams;        //image buffer information
    MINT32  wpeDupIdx;     //wpe duplicate index(pinpon here)
    MINT32  wpeBurstIdx;   //wpe burst index (for multi-frames of one EQ request)
    MUINT32 callerID;
	MUINT32 wpeDupCmdIdx;
	MINT32  idxofwaitDQFrmPackList; //element index in waitDQFramePackageList
	ISP_WPE_MODULE_IDX wpeModeIdx;          //0: WPE A; 1: WPE B
    WPEFrameUnit()
        : wpeDupIdx(0)
        , wpeBurstIdx(0)
        , callerID(0x0)
        , wpeDupCmdIdx(0)
        , idxofwaitDQFrmPackList(0)
        , wpeModeIdx(WPE_MODULE_IDX_WPE_A)
    {}
};

/******************************************************************************
 *
 * @enum ELockEnum
 * @brief enum for recognize lock type.
 * @details
 *
 ******************************************************************************/
enum EWPELockEnum
{
    EWPELockEnum_wDQFrameUnitList        = 0,
    EWPELockEnum_wDQFramePackList
};

enum EWPEBufferListTag
{
    EWPEBufferListTag_UNKNOWN = 0,
    EWPEBufferListTag_Package,
    EWPEBufferListTag_Unit
};


/******************************************************************************
 *
 * @struct WPEFramePackage
 * @brief frame package for enqueue/dequeue (one package may contain multiple frames)
 * @details
 *
 ******************************************************************************/
struct WPEFramePackage
{
    MUINT32 callerID;       //different thread, special buffer id to recognize the buffer source(in default we use address of dequeuedBufList from caller)
    QParams rParams;        //image buffer information
    MINT32  DupIdx;     //p2 cq duplicate index(pinpon here)
    MINT32  frameNum;   //total number of frame units in a single package
    MINT32  dequedNum;      //number of dequeued frame
    MINT32  idxofwaitDQFrmPackList; //element index in waitDQFramePackageList
    MINT32 idx4semEQDone;
    WPEFramePackage()
        : callerID(0x0)
        , DupIdx(0x0)
        , frameNum(0)
        , dequedNum(0)
        , idxofwaitDQFrmPackList(0)
        , idx4semEQDone(0)
    {}
};


struct  WPERequest
{
    void* m_pWpeStream;
    unsigned int m_Num;
    WPEParams m_WPEParams;
    unsigned int wpeEQDupIdx;
    WpePackage m_wpepackage;
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
    ECmd_WPE_ENQUE,
    ECmd_ENQUE,
    ECmd_DEQUENEXTCQ,
    ECmd_INIT,
    ECmd_UNINIT
};

typedef enum {
	WPE_BUF_CTRL_STATUS_EMPTY,	    /* 0 */
	WPE_BUF_CTRL_STATUS_ENQUE,	    /* 1 */
	WPE_BUF_CTRL_STATUS_RUNNING,	/* 2 */
	WPE_BUF_CTRL_STATUS_DEQFRAMEEND,/* 3 */
	WPE_BUF_CTRL_STATUS_WAITDQ,	    /* 4 */
	WPE_BUF_CTRL_STATUS_TOTAL
} WPE_BUF_STATUS_ENUM;

typedef enum {
	WPE_BUF_CTRL_CMD_ENQUEUE,	    /* 0 */
	WPE_BUF_CTRL_CMD_DEQFRAMEEND,	/* 1 */
	WPE_BUF_CTRL_CMD_DEQUEUE,	    /* 2 */
	WPE_BUF_CTRL_CMD_TOTAL
} WPE_BUF_CTRL_ENUM;


struct WPEBufParamNode
{
    volatile MINT32  wpecqDupIdx;     //wpe cq duplicate index(pinpon here)
    volatile MUINT32 wpecallerID;     //different thread, special buffer id to recognize the buffer source(in default we use address of dequeuedBufList from caller)
    volatile WPE_BUF_STATUS_ENUM bufSts;	/* buffer status     */
    WPEBufParamNode()
    : wpecqDupIdx(0)
    , wpecallerID(0x0)
    , bufSts(WPE_BUF_CTRL_STATUS_EMPTY)
    {}
};


/******************************************************************************
 *
 * @class IWpeStream
 * @brief Post-Processing Pipe Interface for Normal Stream.
 * @details 
 * The data path will be Mem --> ISP--XDP --> Mem. 
 *
 ******************************************************************************/
class WpeStream : public IWpeStream
{
    friend  class IWpeStream;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipe Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
	  static WpeStream*		  createInstance();
	
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
	  static WpeStream*		  getWpeStreamImp();


	/**
     * @brief destroy the pipe instance
     *
     * @details
     *
     * @note
     */
    virtual MVOID                   destroyInstance();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
                                    WpeStream();
public:
    virtual                         ~WpeStream();
public:
    /**
     * @brief init the pipe
     *
     * @details
     *
     * @note
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().  
     */
    virtual MBOOL                   init(MUINT32 secTag);

    /**
     * @brief uninit the pipe
     *
     * @details
     *
     * @note
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().
     */
    virtual MBOOL                   uninit();

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
     * @param[in] rParams: Reference to a request of QParams structure.
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure, and an error code can be retrived by \n
     *   getLastErrorCode().
     */
    
    virtual MBOOL                   WPEenque(
                                        QParams const& rParams,
                                        MUINT32 callerID
                                    );
    /**
     * @brief De-queue a result from the pipe.
     *
     * @details
     *
     * @note
     * 
     * @param[in] rParams: Reference to a result of QParams structure.
     *
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
    virtual MBOOL                   WPEdeque(
										MUINT32 callerID,
	                                	MINT64 i8TimeoutNs = -1
                                    );


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

    virtual MERROR                  getLastErrorCode() const {return 0;}


    /**
    * @brief Check the current wpe enqueued user number.
    *
    * @details
    *
    * @note
    *
    * @return
    *      - [true]   current enqueued user number meet the maximum number wpe support
    *      - [false]  current enqueued user number is less than the maximum number wpe support
    */
    virtual MBOOL   checkWPEEnqueuedUserNum();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Thread Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
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
    MVOID addCommand(ECmd const &cmd);
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


    MBOOL getCommand(ECmd &cmd);
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


    MVOID clearCommands();
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

    static  MVOID*  onThreadLoop(MVOID*);
     /**
     * @brief Reset all dequeue related variables in dequeue thread.
     *
     * @details
     *
     * @note
     *
     * @return
     *
     */
    MBOOL  resetDequeVariables();
	MBOOL  Dump_Tpipe(unsigned int *tpipeTableVa);
	MVOID  getWPELock(EWPELockEnum lockType);
	MVOID  releaseWPELock(EWPELockEnum lockType);


    MVOID  getEQUserLock();
    MVOID  releaseEQUserLock();
    MINT32 getUs();
    MBOOL  saveToFile(char const* filepath, WPEBufInfo* buffer);
    MVOID  getEQLock();
    MVOID  releaseEQLock();
	MBOOL  dumpToFile(char const* filepath, MUINT8* pBufVA, size_t  size);
	//MBOOL  Dump_Warp_Tpipe(MUINT32 m_curEQUserundex, QParams rParams);
	MBOOL  dequeueJudgement();
	MBOOL  eraseNotify(EWPEBufferListTag bufferListTag, MINT32 burstIdx, MINT32 dupIdx, NSImageio::NSIspio::EPIPE_P2BUFQUECmd cmd,MUINT32 callerID);
	MBOOL  wpeGlobalListCtrl(WpePackage wpeframepackage, MUINT32 framenumber, MUINT32 IsFirstFrame, MINT32& GlobalListIndex);


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Variables.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    static volatile MINT32     mInitCount;     //Record the user count


    char                       m_UserName[MAX_USER_NAME_SIZE];
    static Mutex               mMutex_EQUser;  //protect the enque sequence!!
    static Mutex               mModuleMtx;

	//Enque Request
    Mutex                      mMutex_WPEwaitDQFrameUnitList;
    Mutex                      mMutex_WPEwaitDQFramePackList;

    //Thread Variable
    static pthread_t           mThread;
    static list<ECmd>          mCmdList;
    static MINT32              mTotalCmdNum;
    static sem_t               mSemWpeThread;
    static Mutex               mWpeCmdLock;
    static Condition           mCmdListCond;
    static volatile MINT32     m_wpeEQUserNum;
    static volatile MINT32     m_wpeEQDupIdx;
    static sem_t               mWPESemEQDone;
    static Mutex               mMutex_EQDone;
    static volatile MBOOL      m_WpeStreamInit; 
    static volatile MINT32     m_iWpeEnqueReq;
    static volatile MINT32     m_bSaveReqToFile;
    //list<WpeBufParamNode>      mvWpeEnqueueBufNode;  //enqueued bufer node list
	static volatile MINT32	   mDumpCount;
	NSImageio::NSIspio::IWarpEnginePipe*                 mwarpengine[2];

	
    //Block Deque Used.
    mutable  Condition         mWpeDequeCond;
    Mutex                      mDequeMtx;
	list<WPEFrameUnit> mLWPEwaitDQFrameUnitList;  //list for frame unit which waits for deque operation using WPE
    vector<WPEFramePackage> mLWPEDQFramePackList; //list for frame package dequeued from hw using WPE
    volatile MINT32     m_wpeEQUserNums[WPE_MODULE_IDX_MAX];
    volatile MINT32     m_wpeEQDupIdxs[MAX_WPE_DUP_IDX_NUM];
};



/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSWpe
};  //namespace NSIoPipe
};  //namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_WPE_WpeStream_H_

