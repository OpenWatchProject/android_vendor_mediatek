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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_EGN_EgnStream_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_EGN_EgnStream_H_
//
#include <mtkcam/drv/iopipe/PostProc/IEgnStream.h>
/*
 * TODO: engine_drv.h
 */
#include "engine_drv.h"

#include <utils/threads.h>
#include <list>
#include <vector>
//------------Thread-------------
#include <sys/prctl.h>
#include <semaphore.h>
//-------------------------------


#define EGN_TBLI_SIZE 256

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSIoPipe {
namespace NSEgn {

#if 0
template <class T>
IEgnStream<T>* getEgnStreamImp(char const* szCallerName);
#endif

template <class T>
struct  EGNRequest
{
    void* m_pEgnStream;
    unsigned int m_Num;
    EGNParams<T> m_EGNParams;
};

/******************************************************************************
 *
 * @class IEgnStream
 * @brief Post-Processing Pipe Interface for Normal Stream.
 * @details 
 * The data path will be Mem --> ISP--XDP --> Mem. 
 *
 ******************************************************************************/
template <class T>
class EgnStream : public IEgnStream<T>
{
    friend  class IEgnStream<T>;
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
                                    EgnStream(char const* szCallerName);
public:
    virtual                         ~EgnStream();
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
    virtual MBOOL                   init(
                                        INITParams<T> const& rInitParams
                                    );
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
    virtual MBOOL                   EGNenque(
                                        EGNParams<T> const& rEgnParams
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
    virtual MBOOL                   EGNdeque(
                                        EGNParams<T>& rEgnParams,
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
    MVOID addCommand(ECmd const &cmd, SUB_ENGINE_ID const &eid);
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


    MBOOL getCommand(ECmd &cmd, SUB_ENGINE_ID &eid);
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
    MBOOL saveToFile(char const* filepath, EGNBufInfo* buffer);


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Variables.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    static MINT32     mInitCount;     //Record the user count

    /*
     * TODO: EngineDrv instread of EgnDrv
     *      static EngineDrv*      m_pEngineDrv;
     */
    static EngineDrv<T>*      m_pEngineDrv;
#ifdef DUAL_ENGINE
    static EngineDrv<T2>*     m_pEngineDrv2;
#endif
    char                m_UserName[MAX_USER_NAME_SIZE];
    static Mutex        mModuleMtx;  //protect the enque sequence!!

    //Enque Request
    static list<EGNRequest<T>>  m_QueueEGNReqList; //EGN Req List
#ifdef DUAL_ENGINE
    static list<EGNRequest<T2>>  m_QueueEGNReqList2;      //EGN Req List2
#endif
    //Thread Variable
    static pthread_t           mThread;
    static list<ECmd>          mCmdList;
    static list<SUB_ENGINE_ID> mEgnList;
    static MINT32              mTotalCmdNum;
    static sem_t               mSemEgnThread;
    static Mutex      mEgnCmdLock;
    static Condition  mCmdListCond;

    //static mutable  Mutex      mEgnCmdLock;
    //static mutable  Condition  mCmdListCond;
    MBOOL m_EgnStreamInit;
    int m_iEgnEnqueReq[eENGINE_MAX];
    int m_bSaveReqToFile;
    EGNInitInfo m_InitInfo;
    //Block Deque Used.
    mutable  Condition  mEgnDequeCond[eENGINE_MAX];
    Mutex               mDequeMtx;
};



/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSEgn
};  //namespace NSIoPipe
};  //namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_EGN_EgnStream_H_

