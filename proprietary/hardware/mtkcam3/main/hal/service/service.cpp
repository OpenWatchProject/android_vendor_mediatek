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
//lomoEffect will be removed after android P only support HAL3
#include <vendor/mediatek/hardware/camera/lomoeffect/1.0/ILomoEffect.h>
#endif
#if MTK_CCAP_HIDL_SUPPORT
#include <vendor/mediatek/hardware/camera/ccap/1.0/ICCAPControl.h>
#endif

#include <vendor/mediatek/hardware/camera/frhandler/1.0/IFRHandler.h>

#ifdef MTK_CAM_ISP_HIDL_SUPPORT
#include <vendor/mediatek/hardware/camera/isphal/1.0/IISPModule.h>
#endif

#include <vendor/mediatek/hardware/camera/postproc/1.0/IPostDevice.h>

#ifdef MTKCAM_BGSERVICE_SUPPORT
#include <vendor/mediatek/hardware/camera/bgservice/1.1/IBGService.h>
// #include <bgservice/BGService.h>
#endif

#include <vendor/mediatek/hardware/camera/atms/1.0/IATMs.h>

#include <cutils/compiler.h>

#include <hidl/HidlTransportSupport.h>
#include <hidl/LegacySupport.h>

#include <binder/ProcessState.h>
#include <mtkcam/utils/std/ULog.h>

#include <mtkcam/utils/gralloc/IGrallocHelper.h>

using namespace android;
using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using android::hardware::registerPassthroughServiceImplementation;

#define MY_LOGI_IF(cond, ...)   do { if (            (cond) ) { ALOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)   do { if ( CC_UNLIKELY(cond) ) { ALOGW(__VA_ARGS__); } }while(0)

int main()
{
    NSCam::Utils::ULog::ULogInitializer __ulogInitializer;

    ALOGI("Camera HAL Server is starting..., ADV_CAM_SUPPORT(%d)", MTKCAM_ADV_CAM_SUPPORT);

    signal(SIGPIPE, SIG_IGN);

    // The camera HAL may communicate to other vendor components via
    // /dev/vndbinder
    android::ProcessState::initWithDriver("/dev/vndbinder");


    configureRpcThreadpool(16, true /*callerWillJoin*/);

    //  AOSP ICameraProvider HAL Interface
    {
        using android::hardware::camera::provider::V2_4::ICameraProvider;
        auto err = registerPassthroughServiceImplementation<ICameraProvider>("internal/0" /*"internal" for binderized mode*/);
        MY_LOGW_IF( err!=OK, "%s: register ICameraProvider err:%d(%s)", __FUNCTION__, err, ::strerror(-err));
    }
    //
    //  MTK IAdvCamControl HAL Interface
    {
#if MTKCAM_ADV_CAM_SUPPORT
        using vendor::mediatek::hardware::camera::advcam::V1_0::IAdvCamControl;
        auto err = registerPassthroughServiceImplementation<IAdvCamControl>("internal/0" /*"internal" for binderized mode*/);
        MY_LOGW_IF( err!=OK, "%s: register IAdvCamControl err:%d(%s)", __FUNCTION__, err, ::strerror(-err));
#endif
    }

    {
#if MTKCAM_LOMO_SUPPORT
//lomoEffect will be removed after android P only support HAL3
        using vendor::mediatek::hardware::camera::lomoeffect::V1_0::ILomoEffect;
        auto err = registerPassthroughServiceImplementation<ILomoEffect>("internal/0" /*"internal" for binderized mode*/);
        MY_LOGW_IF( err!=OK, "%s: register ILomoEffect err:%d(%s)", __FUNCTION__, err, ::strerror(-err));
#endif
    }

#if MTK_CCAP_HIDL_SUPPORT
    {
        using vendor::mediatek::hardware::camera::ccap::V1_0::ICCAPControl;
        auto err = registerPassthroughServiceImplementation<ICCAPControl>("internal/0" /*"internal" for binderized mode*/);
        MY_LOGW_IF( err!=OK, "%s: register ICCAPControl err:%d(%s)", __FUNCTION__, err, ::strerror(-err));
    }
#endif

#ifdef MTK_CAM_ISP_HIDL_SUPPORT
    {
        using ::vendor::mediatek::hardware::camera::isphal::V1_0::IISPModule;
        auto err = registerPassthroughServiceImplementation<IISPModule>("internal/0" /*"internal" for binderized mode*/);
        MY_LOGW_IF( err!=OK, "%s: register IISPModule err:%d(%s)", __FUNCTION__, err, ::strerror(-err));
    }
#endif

    {
        using ::vendor::mediatek::hardware::camera::postproc::V1_0::IPostDevice;
        auto err = registerPassthroughServiceImplementation<IPostDevice>("internal/0" /*"internal" for binderized mode*/);
        MY_LOGW_IF( err!=OK, "%s: register IPostDevice err:%d(%s)", __FUNCTION__, err, ::strerror(-err));
    }

    {
#if 0
        using ::vendor::mediatek::hardware::camera::frhandler::V1_0::IFRHandler;
        auto err = registerPassthroughServiceImplementation<IFRHandler>("internal/0" /*"internal" for binderized mode*/);
        MY_LOGW_IF( err!=OK, "%s: register IFRHandler err:%d(%s)", __FUNCTION__, err, ::strerror(-err));
#endif
    }

    {
#ifdef MTKCAM_BGSERVICE_SUPPORT
        using ::vendor::mediatek::hardware::camera::bgservice::V1_1::IBGService;
        auto err = registerPassthroughServiceImplementation<IBGService>("internal/0" /*"internal" for binderized mode*/);
        MY_LOGW_IF( err!=OK, "%s: register IBGService err:%d(%s)", __FUNCTION__, err, ::strerror(-err));
#endif
    }

    //pre-link the graphic HAL implementation
    auto helper = NSCam::IGrallocHelper::singleton();

    {
        using ::vendor::mediatek::hardware::camera::atms::V1_0::IATMs;
        auto err = registerPassthroughServiceImplementation<IATMs>();
        MY_LOGW_IF( err!=OK, "%s: register IATMs err:%d(%s)", __FUNCTION__, err, ::strerror(-err));
    }

    joinRpcThreadpool();
    return 0;
}

