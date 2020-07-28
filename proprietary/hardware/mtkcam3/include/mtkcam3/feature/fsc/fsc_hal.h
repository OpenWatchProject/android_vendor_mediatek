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

#ifndef _FSC_HAL_H_
#define _FSC_HAL_H_

#include <utils/RefBase.h>
#include <mtkcam/def/common.h>
#include <mtkcam/aaa/IHal3A.h>
#include <mtkcam/drv/iopipe/CamIO/IHalCamIO.h>

using namespace android;
using NSCam::MSize;
using NS3Av3::IHal3A;
using NS3Av3::MetaSet_T;
using NSCam::IMetadata;
using NSCam::NSIoPipe::NSCamIOPipe::QBufInfo;


typedef enum
{
    FSC_HAL_RETURN_NO_ERROR       = 0,       //! The function work successfully
    FSC_HAL_RETURN_UNKNOWN_ERROR  = 0x0001,  //! Unknown error
    FSC_HAL_RETURN_INVALID_DRIVER = 0x0002,  //! invalid driver object
    FSC_HAL_RETURN_API_FAIL       = 0x0003,  //! api fail
    FSC_HAL_RETURN_INVALID_PARA   = 0x0004,  //! invalid parameter
    FSC_HAL_RETURN_NULL_OBJ       = 0x0005,  //! null object
    FSC_HAL_RETURN_MEMORY_ERROR   = 0x0006   //! memory error
}FSC_HAL_ERROR_ENUM;

/*******************************************************************************
*
********************************************************************************/

class FSCHal : public virtual android::RefBase
{
public:
    /**
         *@brief Create FSCHal object
         *@param[in] userName : user name,i.e. who create FSCHal object
         *@param[in] aSensorIdx : sensor index
         *@return
         *-VHdrHal object
       */
    static sp<FSCHal>   CreateInstance(char const *userName, MUINT32 sensorIdx);

    /**
         *@brief Initialization function
         *@param[in] IHal3A_T : 3A for AF callback
         *@return
         *-FSC_HAL_RETURN_NO_ERROR indicates success, otherwise indicates fail
           */
    virtual MINT32      Init(IHal3A* p3A, MSize sensorSize) = 0;

    /**
         *@brief Unitialization function
         *@return
         *-FSC_HAL_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32      Uninit(IHal3A* p3A) = 0;

    /**
         *@brief get AF Initialization parameter for fSC provider, must call after 3A->start()
         *@return
         *-FSC_HAL_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32      getAFInitParams(IHal3A* p3A) = 0;

    virtual MINT32      processResult(
                            MBOOL isBinEn,
                            IMetadata* pInAPP,//inAPP
                            IMetadata* pInHAL,//inHAL
                            MetaSet_T & result3A,
                            IHal3A* p3A,
                            MINT32 const currMagicNum,
                            QBufInfo const &deqBuf,
                            MUINT32 const bufIdxRss,
                            MUINT32 const bufIdxRrz,
                            MUINT32 const index,
                            IMetadata &rOutputFSC
                        ) = 0;

protected:
    virtual ~FSCHal();
};

#endif
