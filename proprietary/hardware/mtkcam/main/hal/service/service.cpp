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

#define LOG_TAG "camerahalserver"

#include <android/hardware/camera/provider/2.4/ICameraProvider.h>

#if MTKCAM_ADV_CAM_SUPPORT
#include <vendor/mediatek/hardware/camera/advcam/1.0/IAdvCamControl.h>
#endif
#if MTKCAM_LOMO_SUPPORT
#include <vendor/mediatek/hardware/camera/lomoeffect/1.0/ILomoEffect.h>
#endif
#include <vendor/mediatek/hardware/camera/ccap/1.0/ICCAPControl.h>
#if MTKCAM_MMSDK_SUPPORT
#include <vendor/mediatek/hardware/camera/callbackclient/1.1/IMtkCallbackClient.h>
#endif

#include <vendor/mediatek/hardware/camera/frhandler/1.0/IFRHandler.h>

#include <hidl/HidlTransportSupport.h>
#include <hidl/LegacySupport.h>

#include <binder/ProcessState.h>

using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using android::hardware::registerPassthroughServiceImplementation;

int main()
{
    ALOGI("Camera HAL Server is starting..., ADV_CAM_SUPPORT(%d)", MTKCAM_ADV_CAM_SUPPORT);

    signal(SIGPIPE, SIG_IGN);

    // The camera HAL may communicate to other vendor components via
    // /dev/vndbinder
    android::ProcessState::initWithDriver("/dev/vndbinder");


    configureRpcThreadpool(16, true /*callerWillJoin*/);

    //  AOSP ICameraProvider HAL Interface
    {
        using android::hardware::camera::provider::V2_4::ICameraProvider;
        registerPassthroughServiceImplementation<ICameraProvider>("internal/0" /*"internal" for binderized mode*/);
    }
    //
    //  MTK IAdvCamControl HAL Interface
    {
#if MTKCAM_ADV_CAM_SUPPORT
        using vendor::mediatek::hardware::camera::advcam::V1_0::IAdvCamControl;
        registerPassthroughServiceImplementation<IAdvCamControl>("internal/0" /*"internal" for binderized mode*/);
#endif
    }

    {
#if MTKCAM_LOMO_SUPPORT
        using vendor::mediatek::hardware::camera::lomoeffect::V1_0::ILomoEffect;
        registerPassthroughServiceImplementation<ILomoEffect>("internal/0" /*"internal" for binderized mode*/);
#endif
    }

    {
        using vendor::mediatek::hardware::camera::ccap::V1_0::ICCAPControl;
        registerPassthroughServiceImplementation<ICCAPControl>("internal/0" /*"internal" for binderized mode*/);
    }

    {
#if MTKCAM_MMSDK_SUPPORT
        using vendor::mediatek::hardware::camera::callbackclient::V1_1::IMtkCallbackClient;
        registerPassthroughServiceImplementation<IMtkCallbackClient>("internal/0" /*"internal" for binderized mode*/);
#endif
    }

    {
        using ::vendor::mediatek::hardware::camera::frhandler::V1_0::IFRHandler;
        registerPassthroughServiceImplementation<IFRHandler>("internal/0" /*"internal" for binderized mode*/);
    }

    joinRpcThreadpool();
    return 0;
}

