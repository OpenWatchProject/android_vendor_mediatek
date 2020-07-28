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

#ifndef _CUSTOM_CAMERA_PLUGIN_IPLUGFASTENING_H_
#define _CUSTOM_CAMERA_PLUGIN_IPLUGFASTENING_H_
//
#include <mtkcam/def/common.h>
#include <mtkcam/utils/hw/IPlugParam.h>
#include <mtkcam/utils/hw/IPlugCmdDef.h>
/******************************************************************************
 *
 ******************************************************************************/
namespace NSCamCustom {

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface of Plug Fastening
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class IPlugFastening
{
public:
    enum OPEN_VENDOR_ID
    {
        OPEN_VENDOR_ID_BASIC               = 0,
        OPEN_VENDOR_ID_OVFCell,
        OPEN_VENDOR_ID_S4Cell,
        // OPEN_VENDOR_ID_A,
        // OPEN_VENDOR_ID_B.
        // OPEN_VENDOR_ID_C_MAIN,
        // OPEN_VENDOR_ID_C_SUB,
        // ...
        OPEN_VENDOR_ID_MAX                 // Just for check
                                    // DO NOT use it to create instance
    };

public:
                            IPlugFastening() {};
    virtual                 ~IPlugFastening() {};

    /**
     * This utility is for the plug-fastening to add its specific flow.
     *
     * While the first user called getInstance() with OPEN_VENDOR_ID_XXX,
     * it will return the OPEN_VENDOR_ID_XXX's singleton.
     *
     */

public:
    /**
     * Get the singleton with OPEN_VENDOR_ID_XXX.
     */
    static IPlugFastening*  getInstance(
                                MUINT32 const openId = OPEN_VENDOR_ID_BASIC);

public:
    /**
     * Execute the init process of the specific plug-fastening flow.
     */
    virtual MBOOL           init(NSCam::PlugInitParam initParam) = 0;

    /**
     * Execute the un-init process of the specific plug-fastening flow.
     */
    virtual MBOOL           uninit(void) = 0;

    /**
     * Execute the sent command process of the specific plug-fastening flow.
     */
    virtual MBOOL           sendCommand(MINT32 const cmd,
                                MINTPTR arg1 = 0, MINTPTR arg2 = 0,
                                MINTPTR arg3 = 0, MINTPTR arg4 = 0) = 0;
};

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSCamCustom

#endif  //_CUSTOM_CAMERA_PLUGIN_IPLUGFASTENING_H_

