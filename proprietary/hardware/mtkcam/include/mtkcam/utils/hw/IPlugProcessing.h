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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_HW_IPLUGPROCESSING_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_HW_IPLUGPROCESSING_H_
//
#include <utils/RefBase.h>
#include <mtkcam/def/common.h>
#include "IPlugParam.h"
#include "IPlugCmdDef.h"

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface of Plug Processing
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class IPlugProcessing
    : public virtual android::RefBase
{
public:
    enum PLUG_ID
    {
        PLUG_ID_BASIC               = 0,
        PLUG_ID_FCELL,
        PLUG_ID_MAX                 // Just for check
                                    // DO NOT use it to create instance
    };

    enum DEV_ID
    {
        DEV_ID_0                    = 0,
        DEV_ID_1,
        DEV_ID_2,
        DEV_ID_3,
        DEV_ID_4,
        DEV_ID_5,
        DEV_ID_6,
        DEV_ID_7,
        DEV_ID_MAX                  // Just for check
                                    // DO NOT use it to create instance
    };

    enum OP_MODE
    {
        OP_MODE_SYNC                = 0,
        OP_MODE_ASYNC
    };

    enum PARAM_TYPE
    {
        PARAM_INIT                         = 0,
        PARAM_CONFIG,
        PARAM_PROCESSING
    };
    /**
     * This utility is for the plug-processing to add its specific flow.
     *
     * While the first user called createInstance() with PLUG_ID_XXX,
     * it will new the PLUG_ID_XXX's instance and return its SP.
     *
     * If this user hold this instance, the next user calls createInstance()
     * with PLUG_ID_XXX will get the SP referred to the same instance
     * (previous one, not new).
     *
     * This instance will be destroyed while no user keeps this SP
     * and it will call the its uninit() automatically.
     *
     * If the re-init is needed, it must call uninit() first,
     * then calls init() again.
     */

public:
    /**
     * New the instance or get the previous one if this PLUG_ID existed.
     */
    static android::sp<IPlugProcessing>
                            createInstance(
                                MUINT32 const plugId = (MUINT32)PLUG_ID_BASIC,
                                DEV_ID const devId = DEV_ID_0);

public:
    /**
     * Execute the init process of the specific plug-processing flow.
     * If the init() was executed, it will return ALREADY_EXISTS directly.
     *
     * opMode == OP_MODE_SYNC:
     *     execute init process directly and return the function
     *     after the init processing done.
     * opMode == OP_MODE_ASYNC:
     *     execute init process by a new thread and return the function
     *     after the new thread started.
     *
     */
    virtual MERROR          init(OP_MODE const opMode) = 0;

    /**
     * Execute the un-init process of the specific plug-processing flow.
     * If the uninit() was executed, it will return ALREADY_EXISTS directly.
     */
    virtual MERROR          uninit() = 0;

    /**
     * If some interaction (sendCommand) must be after the init processing,
     * user must call waitInitDone() to make sure the previous init processing
     * was finished.
     */
    virtual MERROR          waitInitDone(void) = 0;

    /**
     * The interaction of the specific plug-processing flow with the reserved
     * NSCam::NSCamPlug::CMD and arguments.
     */
    virtual MERROR          sendCommand(MINT32 const cmd,
                                MINTPTR arg1 = 0, MINTPTR arg2 = 0,
                                MINTPTR arg3 = 0, MINTPTR arg4 = 0) = 0;
};

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSCam

#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_HW_IPLUGPROCESSING_H_

