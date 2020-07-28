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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_RSC_RscStream_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_RSC_RscStream_H_
//
#include <mtkcam/drv/iopipe/PostProc/IRscStream.h>
#include <rsc_drv.h>
#include <utils/threads.h>
#include <list>
#include <vector>
//------------Thread-------------
#include <sys/prctl.h>
#include <semaphore.h>
//-------------------------------


#define RSC_TBLI_SIZE 256

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSIoPipe {
namespace NSRsc {

IRscStream* getRscStreamImp(char const* szCallerName);

struct  RSCRequest
{
    void* m_pRscStream;
    unsigned int m_Num;
    RSCParams m_RSCParams;
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
    ECmd_RSC_ENQUE,
    ECmd_ENQUE,
    ECmd_DEQUENEXTCQ,
    ECmd_INIT,
    ECmd_UNINIT
};


/******************************************************************************
 *
 * @class IRscStream
 * @brief Post-Processing Pipe Interface for Normal Stream.
 * @details 
 * The data path will be Mem --> ISP--XDP --> Mem. 
 *
 ******************************************************************************/
class RscStream : public IRscStream
{
    friend  class IRscStream;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipe Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.
    /**
     * @brief destroy the pipe instance
     *
     * @details
     *
     * @note
     */
    virtual MVOID                   destroyInstance(char const* szCallerName);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
                                    RscStream(char const* szCallerName);
public:
    virtual                         ~RscStream();
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
    virtual MBOOL                   init();

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
    virtual MBOOL                   RSCenque(
                                        RSCParams const& rRscParams
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
    virtual MBOOL                   RSCdeque(
                                        RSCParams& rRscParams,
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
    MBOOL resetDequeVariables();

    MVOID getLock();
    MVOID releaseLock();
    MINT32 getUs();
    MBOOL saveToFile(char const* filepath, RSCBufInfo* buffer);


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Variables.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    static MINT32     mInitCount;     //Record the user count

    //Rsc Driver
    static RscDrv*      m_pRscDrv;
    char                m_UserName[MAX_USER_NAME_SIZE];
    static Mutex        mModuleMtx;  //protect the enque sequence!!

    //Enque Request
    static list<RSCRequest>  m_QueueRSCReqList; //RSC Req List

    //Thread Variable
    static pthread_t           mThread;
    static list<ECmd>          mCmdList;
    static MINT32              mTotalCmdNum;
    static sem_t               mSemRscThread;
    static Mutex      mRscCmdLock;
    static Condition  mCmdListCond;

    //static mutable  Mutex      mRscCmdLock;
    //static mutable  Condition  mCmdListCond;
    MBOOL m_RscStreamInit;
    int m_iRscEnqueReq;
    int m_bSaveReqToFile;

    //Block Deque Used.
    mutable  Condition  mRscDequeCond;
    Mutex               mDequeMtx;
};



/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSRsc
};  //namespace NSIoPipe
};  //namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_RSC_RscStream_H_

