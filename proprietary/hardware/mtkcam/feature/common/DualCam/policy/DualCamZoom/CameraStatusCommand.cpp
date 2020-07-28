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
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY
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
#define LOG_TAG "MtkCam/DualCamZoomPolicy/CameraStatusCommand"

// Standard C header file

// Android system/core header file

// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

// Module header file
#include <mtkcam/feature/DualCam/utils/DualCameraUtility.h>

// Local header file
#include "CameraStatusCommand.h"


/******************************************************************************
*
*******************************************************************************/
#define COMMON_LOGV(fmt, arg...)    CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define COMMON_LOGD(fmt, arg...)    CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define COMMON_LOGI(fmt, arg...)    CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define COMMON_LOGW(fmt, arg...)    CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define COMMON_LOGE(fmt, arg...)    CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define COMMON_LOGA(fmt, arg...)    CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define COMMON_LOGF(fmt, arg...)    CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)

#define COMMON_FUN_IN()             COMMON_LOGD("+")
#define COMMON_FUN_OUT()            COMMON_LOGD("-")

class scoped_tracer
{
public:
    scoped_tracer(const char* functionName)
    : mFunctionName(functionName)
    {
        CAM_LOGD("[%s] +", mFunctionName);
    }
    ~scoped_tracer()
    {
        CAM_LOGD("[%s] -", mFunctionName);
    }
private:
    const char* const mFunctionName;
};
#define COMMON_SCOPED_TRACER() scoped_tracer ___scoped_tracer(__FUNCTION__ );


/******************************************************************************
* namespace start
*******************************************************************************/
namespace NSCam
{
using Utility   = DualCameraUtility;
using Helper    = DualZoomPolicyHelper;
using Converter = DualZoomPolicyConverter;
/******************************************************************************
 * CameraStatusCommand
 ******************************************************************************/
CameraStatusCommand::
CameraStatusCommand(std::string name, ControlFlowMode controlFlowMode)
: mName(name)
, mControlFlowMode(controlFlowMode)
{

}
//
CameraStatusCommand::ControlFlowMode
CameraStatusCommand::
getControlFlowMode() const
{
    return mControlFlowMode;
}
//
const std::string&
CameraStatusCommand::
getName() const
{
    return mName;
}
//
CameraStatusCommand::
~CameraStatusCommand()
{

}
//
/******************************************************************************
 * CameraStatusBase::TransitCommand
 ******************************************************************************/
sp<CameraStatusCommand>
TransitCommand::
createInstance(CreateInstanceInvoker createInstanceInvoker)
{
    return new TransitCommand(createInstanceInvoker);
}
//
TransitCommand::
TransitCommand(CreateInstanceInvoker createInstanceInvoker)
: CameraStatusCommand("transit_command", false)
, mCreateInstanceInvoker(createInstanceInvoker)
{

}
//
MVOID
TransitCommand::
excute(sp<CameraContext>& context, sp<CameraStatusBase>& receiver, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl)
{
    receiver->transit(context, mCreateInstanceInvoker(receiver->getOpenId()), appMetadata, halMetadata, frame3AControl);
}
//
/******************************************************************************
 * SwitchTwinModeCommand
 ******************************************************************************/
sp<CameraStatusCommand>
SwitchTwinModeCommand::
createInstance(TwinTGMode twinTGMode)
{
    return new SwitchTwinModeCommand(twinTGMode);
}
//
SwitchTwinModeCommand::
SwitchTwinModeCommand(TwinTGMode switchTwinTGMode)
: CameraStatusCommand("switch_twin_tg_command", true)
, mSwitchTwinTGMode(switchTwinTGMode)
{

}
//
void
SwitchTwinModeCommand::
excute(sp<CameraContext>& context, sp<CameraStatusBase>& receiver, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl)
{
    COMMON_SCOPED_TRACER()

    if(!Helper::getDynamicTwinSupported())
    {
        COMMON_LOGE("failed to execute switch twin mode command, is not support dynamic twin mode");
        return;
    }

    const MTK_SYNC_CAEMRA_STATE receiverStatus = receiver->getStatus();
    if(receiverStatus == MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_ACTIVE)
    {
        const TwinTGMode crurrentTwinMode = receiver->getTwinTGMode();
        switch(mSwitchTwinTGMode)
        {
            case MTK_P1_TWIN_STATUS_TG_MODE_1:
                {
                    if(crurrentTwinMode != MTK_P1_TWIN_STATUS_TG_MODE_1)
                    {
                        Utility::setMetadataAndUpdate<MINT32>(*halMetadata, MTK_P1NODE_TWIN_SWITCH, MTK_P1_TWIN_SWITCH_ONE_TG);

                        COMMON_LOGD("switch twin TG mode from %s(%d) to %s(%d)",
                            Converter::twinTGModeNameMap[crurrentTwinMode], crurrentTwinMode, Converter::twinTGModeNameMap[mSwitchTwinTGMode], mSwitchTwinTGMode);
                    }
                    else
                    {
                        COMMON_LOGE("failed to execute switch twin mode command, switch from %s(%d) to %s(%d)",
                            Converter::twinTGModeNameMap[crurrentTwinMode], crurrentTwinMode, Converter::twinTGModeNameMap[mSwitchTwinTGMode], mSwitchTwinTGMode);
                    }
                }
                break;
            case MTK_P1_TWIN_STATUS_TG_MODE_2:
                {
                    if(crurrentTwinMode != MTK_P1_TWIN_STATUS_TG_MODE_2)
                    {
                        Utility::setMetadataAndUpdate<MINT32>(*halMetadata, MTK_P1NODE_TWIN_SWITCH, MTK_P1_TWIN_SWITCH_TWO_TG);

                        COMMON_LOGD("switch twin TG mode from %s(%d) to %s(%d)",
                            Converter::twinTGModeNameMap[crurrentTwinMode], crurrentTwinMode, Converter::twinTGModeNameMap[mSwitchTwinTGMode], mSwitchTwinTGMode);
                    }
                    else
                    {
                        COMMON_LOGE("failed to execute switch twin mode command, switch from %s(%d) to %s(%d)",
                            Converter::twinTGModeNameMap[crurrentTwinMode], crurrentTwinMode, Converter::twinTGModeNameMap[mSwitchTwinTGMode], mSwitchTwinTGMode);
                    }
                }
                break;
            default:
            {
                COMMON_LOGE("failed to execute switch twin mode command, unknown twin mode: %d", mSwitchTwinTGMode);
            }
            break;
        }
    }
    else
    {
        COMMON_LOGE("failed to execute switch twin mode command, receiver status is %s(%u) and not active",
            Converter::statusNameMap[receiverStatus], static_cast<MUINT32>(receiverStatus));
    }
}
//
} // NSCam
