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
#define LOG_TAG "MtkCam/HwPipeline/Adapter"
//
#include "PipelineModelManagerBase.h"
#include "MyUtils.h"
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("%d[PipelineModelManager::%s] " fmt, mId, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("%d[PipelineModelManager::%s] " fmt, mId, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("%d[PipelineModelManager::%s] " fmt, mId, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("%d[PipelineModelManager::%s] " fmt, mId, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("%d[PipelineModelManager::%s] " fmt, mId, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("%d[PipelineModelManager::%s] " fmt, mId, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("%d[PipelineModelManager::%s] " fmt, mId, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
PipelineModelManagerBase::
PipelineModelManagerBase(AppCreation const& params)
    : mId(params.openId)
    , mName(params.userName)
    , mAppCallback(params.appCallback)
{
    MY_LOGD("%p", this);
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineModelManagerBase::
waitUntilInitDone() -> bool
{
    for( auto &fut : mInitFutures ) {
        bool result = fut.get();
        if  ( ! result ) {
            MY_LOGE("Fail to init");
            return false;
        }
    }
    //
    mInitFutures.clear();
    return true;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineModelManagerBase::
onInit() -> bool
{
    mInitFutures.push_back(
        std::async(std::launch::async,
            [ this ]() {
                mHalDevice = IHalDeviceAdapter::create(mId, mName.string());

                return (mHalDevice != nullptr) && mHalDevice->powerOn();
            }
        )
    );

    return true;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineModelManagerBase::
onUninit() -> void
{
    NSCam::Utils::CamProfile profile(__FUNCTION__, mName.string());

    waitUntilInitDone();
    profile.print("waitUntilInitDone -");

    if  ( mHalDevice != nullptr ) {
        mHalDevice->powerOff();
        profile.print("Device powerOff -");
        mHalDevice = nullptr;
        profile.print("Device destroy -");
    }
}

