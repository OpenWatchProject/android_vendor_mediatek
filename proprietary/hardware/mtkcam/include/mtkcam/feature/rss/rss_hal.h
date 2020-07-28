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

#ifndef _RSS_HAL_H_
#define _RSS_HAL_H_

#include <utils/RefBase.h>
#include <mtkcam/def/common.h>
#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>

using namespace android;
using namespace NSCam::NSIoPipe::NSCamIOPipe;

using NSCam::MSize;

typedef enum
{
    RSS_HAL_RETURN_NO_ERROR       = 0,       //! The function work successfully
    RSS_HAL_RETURN_UNKNOWN_ERROR  = 0x0001,  //! Unknown error
    RSS_HAL_RETURN_INVALID_DRIVER = 0x0002,  //! invalid driver object
    RSS_HAL_RETURN_API_FAIL       = 0x0003,  //! api fail
    RSS_HAL_RETURN_INVALID_PARA   = 0x0004,  //! invalid parameter
    RSS_HAL_RETURN_NULL_OBJ       = 0x0005,  //! null object
    RSS_HAL_RETURN_MEMORY_ERROR   = 0x0006   //! memory error
}RSS_HAL_ERROR_ENUM;

/*******************************************************************************
*
********************************************************************************/

class RssHal : public virtual android::RefBase
{
public:
    /**
         *@brief Create RssHal object
         *@param[in] userName : user name,i.e. who create VHdrHal object
         *@param[in] aSensorIdx : sensor index
         *@return
         *-VHdrHal object
       */
    static sp<RssHal>   CreateInstance(char const *userName, MUINT32 sensorIdx);

    /**
         *@brief Initialization function
         *@param[in] INormalPipe : normalPipe for callback
         *@param[in] MSize : rsso buffer size
         *@return
         *-RSS_HAL_RETURN_NO_ERROR indicates success, otherwise indicates fail
           */
    virtual MINT32      Init(INormalPipe *pipe, MSize bufSize) = 0;

    /**
         *@brief Unitialization function
         *@return
         *-RSS_HAL_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32      Uninit() = 0;

protected:
    virtual ~RssHal();
};

#endif
