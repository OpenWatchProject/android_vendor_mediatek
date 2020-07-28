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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

#define LOG_TAG "MtkCamCustom/PlugFasteningBasic"
//
#include "PlugFasteningBasic.h"

using namespace android;
using namespace NSCamCustom;


/******************************************************************************
 *
 ******************************************************************************/
PLUGFASTENING_CLASSNAME::
PLUGFASTENING_CLASSNAME(OPEN_VENDOR_ID id)
{
    FUNC_BGN;
    mOpenID = id; // must set OpenID
    //
    // add implementation here
    //
    FUNC_END;
};


/******************************************************************************
 *
 ******************************************************************************/
PLUGFASTENING_CLASSNAME::
~PLUGFASTENING_CLASSNAME()
{
    FUNC_BGN;
    //
    // add implementation here
    //
    FUNC_END;
};


/******************************************************************************
 * init function implementation
 ******************************************************************************/
MBOOL
PLUGFASTENING_CLASSNAME::
initImp(NSCam::PlugInitParam initParam)
{
    FUNC_BGN;
    MBOOL res = MTRUE;
    //
    // add implementation here
    //
    FUNC_END;
    return res;
};


/******************************************************************************
 * uninit function implementation
 ******************************************************************************/
MBOOL
PLUGFASTENING_CLASSNAME::
uninitImp(void)
{
    FUNC_BGN;
    MBOOL res = MTRUE;
    //
    // add implementation here
    //
    FUNC_END;
    return res;
};


/******************************************************************************
 * send command function implementation
 ******************************************************************************/
MBOOL
PLUGFASTENING_CLASSNAME::
sendCommandImp(MINT32 const cmd,
    MINTPTR arg1, MINTPTR arg2, MINTPTR arg3, MINTPTR arg4)
{
    FUNC_BGN;
    MBOOL res = MTRUE;
    MY_LOGI_IF(mLogLevel > 0, "CMD[%d]"
        "(%" PRIxPTR ", %" PRIxPTR ", %" PRIxPTR ", %" PRIxPTR ")",
        cmd, arg1, arg2, arg3, arg4);
    //
    // add implementation here
    //
    FUNC_END;
    return res;
};


