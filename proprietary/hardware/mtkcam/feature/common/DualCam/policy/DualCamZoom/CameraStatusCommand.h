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
 * MediaTek Inc. (C) 2016. All rights reserved.
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
/******************************************************************************
*
*******************************************************************************/
#ifndef _MTKCAM_DUALZOOM_CAMERA_STATUS_COMMAND_H_
#define _MTKCAM_DUALZOOM_CAMERA_STATUS_COMMAND_H_

// Standard C header file

// Android system/core header file

// mtkcam custom header file

// mtkcam global header file

// Module header file

// Local header file
#include "DualZoomPolicy_Common.h"
#include "CameraContext.h"
#include "CameraStatus.h"


/******************************************************************************
* namespace start
*******************************************************************************/
namespace NSCam
{
/******************************************************************************
 * CameraStatusCommand
 ******************************************************************************/
class CameraContext;
class CameraStatusBase;
class CameraStatusCommand : public RefBase
{
public:
    using ControlFlowMode = MINT32;

public:
    const std::string& getName() const;

    ControlFlowMode getControlFlowMode() const;

public:
    CameraStatusCommand(std::string name, MINT32 controlFlowAfterExcute);

public:
    virtual MVOID excute(sp<CameraContext>& context, sp<CameraStatusBase>& receiver, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl) = 0;

    virtual ~CameraStatusCommand();

private:
    ControlFlowMode     mControlFlowMode;
    const std::string   mName;
};
/******************************************************************************
 * TransitCommand
 ******************************************************************************/
class TransitCommand : public CameraStatusCommand
{
public:
    using CreateInstanceInvoker = std::function<sp<CameraStatusBase>(OpenId openId)>;

public:
    static sp<CameraStatusCommand> createInstance(CreateInstanceInvoker createInstanceInvoker);

public:
    MVOID excute(sp<CameraContext>& context, sp<CameraStatusBase>& receiver, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl);

private:
    TransitCommand(CreateInstanceInvoker createInstanceInvoker);

private:
    CreateInstanceInvoker mCreateInstanceInvoker;
};
/******************************************************************************
 * SwitchTwinModeCommand
 ******************************************************************************/
//--------------------------------------------
class SwitchTwinModeCommand : public CameraStatusCommand
{
public:
    static sp<CameraStatusCommand> createInstance(TwinTGMode switchTwinTGMode);

public:
    void excute(sp<CameraContext>& context, sp<CameraStatusBase>& receiver, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl);

private:
    SwitchTwinModeCommand(TwinTGMode twinTGMode);

private:
    TwinTGMode mSwitchTwinTGMode;
};
}// NSCam
#endif // _MTKCAM_DUALZOOM_CAMERA_STATUS_COMMAND_H_