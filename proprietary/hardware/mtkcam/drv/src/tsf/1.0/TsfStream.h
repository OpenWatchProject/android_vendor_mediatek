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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_TSF_TsfStream_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_TSF_TsfStream_H_
//
#include <ITsfStream.h>
#include <tsf_drv.h>
#include <utils/threads.h>
#include <list>
#include <vector>
//------------Thread-------------
#include <sys/prctl.h>
#include <semaphore.h>
//-------------------------------


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSIoPipe {
namespace NSTsf {




struct  TSFRequest
{
    void* m_pTsfStream;
    TSFParams m_TSFParams;
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
    ECmd_TSF_ENQUE,
    ECmd_TSF_DEQUE,
    ECmd_INIT,
    ECmd_UNINIT
};


/******************************************************************************
 *
 * @class ITsfStream
 * @brief Post-Processing Pipe Interface for Normal Stream.
 * @details 
 * The data path will be Mem --> ISP--XDP --> Mem. 
 *
 ******************************************************************************/
class TsfStream : public ITsfStream
{
    friend  class ITsfStream;
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
protected:
                                    TsfStream(char const* szCallerName);  
public:
    virtual                         ~TsfStream();
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
     * @param[in] rTsfParams: Reference to a request of TSFParams structure.
     *
     * @return 
     * - MTRUE indicates success;
     * - MFALSE indicates failure, and an error code can be retrived by \n
     *   getLastErrorCode().
     */
    virtual MBOOL                   TSFenque(
                                        TSFParams const& rTsfParams
                                    );

    /**
     * @brief De-queue a result from the pipe.
     *
     * @details
     *
     * @note
     * 
     * @param[in] rParams: Reference to a result of TSFParams structure.
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
    virtual MBOOL                   TSFdeque(
                                        TSFParams& rTsfParams, 
                                        MINT64 i8TimeoutNs = -1
                                    );

    /**
     * @brief get the key
     *
     * @details
     *
     * @note
     * 
     * @param[out] DeCryptionKey: get the key of input decryption, EnCryptionKey: get the key of output encryption
     *
     * @return
     * - MTRUE indicates success; 
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().
     *
     */
    virtual MBOOL            getTSFKey(MUINT32& DeCryptionKey, MUINT32& EnCryptionKey);


    /**
     * @brief set up input decry or ouptut encry or not ?
     *
     * @details
     *
     * @note
     * 
     * @param[in] InputDeCryption: set up input decryption or not, OutputEnCryption: set up output encryption or not
     *
     * @return
     * - MTRUE indicates success; 
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().
     *
     */
    virtual MBOOL            setCryption(MBOOL InputDeCryption, MBOOL OutputEnCryption);


    virtual MERROR                  getLastErrorCode() const {return 0;}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Thread Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MBOOL saveToFile(char const* filepath, TSFBufInfo* buffer);
    MBOOL DrvMakePath(char const*const path, uint_t const mode);
    bool DrvMkdir(char const*const path, uint_t const mode);

 
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Variables.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    static MINT32     mInitCount;     //Record the user count

    //Tsf Driver
    static TsfDrv*      m_pTsfDrv;
    char                m_UserName[MAX_USER_NAME_SIZE];
    static Mutex        mModuleMtx;  //protect the enque sequence!!

    //Enque Request
    static list<TSFRequest>  m_QueueTsfReqList; //TSF Req List
    int m_iTsfEnqueReq;

    MUINT32 m_DeCryptionKey;  //DeCryKey
    MUINT32 m_EnCryptionKey;  //EnCryKey
    MBOOL m_InputDeCryption;
    MBOOL m_OutputEnCryption;

    //Thread Variable
    static pthread_t           mThread;
    static list<ECmd>          mCmdList;
    static MINT32              mTotalCmdNum;
    static sem_t               mSemTsfThread;
    static Mutex      mTsfCmdLock;
    static Condition  mCmdListCond;

    int m_bSaveReqToFile;

    //Block Deque Used.
    mutable  Condition  mTsfDequeCond;
    Mutex               mDequeMtx;
};



/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSTsf
};  //namespace NSIoPipe
};  //namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_TSF_TsfStream_H_

