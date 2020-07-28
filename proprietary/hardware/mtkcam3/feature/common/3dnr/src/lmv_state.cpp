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

#define LOG_TAG "LmvState"

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>
#include <mtkcam3/feature/3dnr/lmv_state.h>
#include <lmv_state_impl.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_3DNR_HAL);

using namespace android;
using namespace NSCam::NSIoPipe::NSCamIOPipe;


static MUINT32 getNumOfUniOnPlatform()
{
#if 0
    auto pModule = INormalPipeModule::get();

    NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_QueryInfo info;
    pModule->query(
            NSCam::NSIoPipe::PORT_RRZO.index,
            NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_UNI_NUM,
            NSCam::eImgFmt_UNKNOWN,
            0,
            info);

    return info.uni_num;
#else // TODO: Remove after the API patched
    return 1;
#endif
}


LmvStateMachine* LmvStateMachine::spInstance = NULL;
Mutex LmvStateMachine::sSingletonMutex;


LmvStateMachine *LmvStateMachine::createInstance(LmvState initState)
{
    Mutex::Autolock lock(sSingletonMutex);

    if (spInstance == NULL)
    {
        MUINT32 numOfUni = getNumOfUniOnPlatform();

        CAM_ULOGMD("[LmvState] Uni: %u", numOfUni);

        if (numOfUni == 1)
            spInstance = new LmvStateMachineUni(initState);
        else // more than 1 UNI or no UNI
            spInstance = new LmvStateMachineDummy(initState);
    }

    return spInstance;
}


void LmvStateMachine::destroyInstance()
{
    Mutex::Autolock lock(sSingletonMutex);

    if (spInstance != NULL)
    {
        delete spInstance;
        spInstance = NULL;
    }
}


LmvStateMachine *LmvStateMachine::getInstance()
{
    return spInstance;
}


LmvStateMachine::LmvState LmvStateMachine::getCurrentState()
{
    Mutex::Autolock lock(mStateMutex);
    return mCurrentState;
}


LmvStateMachine::LmvStateMachine(LmvState initState)
{
    mCurrentState = initState;
}


LmvStateMachine::~LmvStateMachine()
{
    // Do nothing
}


