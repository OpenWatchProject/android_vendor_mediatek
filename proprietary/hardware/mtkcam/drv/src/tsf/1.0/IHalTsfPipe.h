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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_TSF_IHalTsfPipe_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_TSF_IHalTsfPipe_H_
//
#include "tsfcommon.h"
#include <vector>
using namespace std;


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSIoPipe {
namespace NSTsf {




/******************************************************************************
 *
 * @struct TSFParams
 *
 * @brief Queuing parameters for the pipe.
 *      input left and right image, mask, preview disparity vector
 *      output disparity vector, edge output,
 *
 * @param[in] mpfnCallback: a pointer to a callback function.
 *      If it is NULL, the pipe must put the result into its result queue, and 
 *      then a user will get the result by deque() from the pipe later.
 *      If it is not NULL, the pipe does not put the result to its result queue.
 *      The pipe must invoke a callback with the result.
 *
 * @param[in] mvTSFReq: a vector of input requests.
 *
 ******************************************************************************/
struct  TSFParams
{
    typedef MVOID                   (*PFN_CALLBACK_T)(TSFParams& rParams);
    PFN_CALLBACK_T                  mpfnCallback;
    TSFConfig                       mTSFConfig;
    MVOID*                          mpCookie;
    TSFParams()     //HW Default value
        : mpfnCallback(NULL),mpCookie(NULL)
    {
    }

};

/******************************************************************************
 *
 * @class IHalTsfPipe
 * @brief Depth Engine Pipe Interface.
 * @details 
 * The data path will be Mem --> TSF --> Mem.
 *
 ******************************************************************************/
class IHalTsfPipe
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Destructor.

    /**
     * @brief Disallowed to directly delete a raw pointer.
     */
    virtual                         ~IHalTsfPipe() {};

public:     ////                    Instantiation.

    /**
     * @brief destroy the pipe instance 
     *
     * @details      
     *
     * @note 
     */
    virtual MVOID                   destroyInstance(char const* szCallerName)   = 0;

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
    virtual MBOOL                   init() = 0;

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
    virtual MBOOL                   uninit()                                    = 0;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Attributes.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    

    /**
     * @brief get the pipe name
     *
     * @details
     *
     * @note
     *
     * @return
     * - A null-terminated string indicating the name of the pipe.
     *
     */
    virtual char const*             getPipeName() const                         = 0;

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
                                    )                                           = 0;

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
                                    )                                           = 0;



    /**
     * @brief get the key
     *
     * @details
     *
     * @note
     *
     * 
     * @param[out] DeCryptionKey: get the key of input decryption, EnCryptionKey: get the key of output encryption
     *
     * @return
     * - MTRUE indicates success; 
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().
     *
     */
    virtual MBOOL            getTSFKey(MUINT32& DeCryptionKey, MUINT32& EnCryptionKey)  = 0;


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
    virtual MBOOL            setCryption(MBOOL InputDeCryption, MBOOL OutputEnCryption)  = 0;


};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSTsf
};  //namespace NSIoPipe
};  //namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_TSF_IHalTsfPipe_H_

