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

#define LOG_TAG "mtkcam-PipelineSettingPolicy"

#include "PipelineSettingPolicyImpl.h"

#include <mtkcam/utils/std/ULog.h>
#include "MyUtils.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_POLICY);


/******************************************************************************
 *
 ******************************************************************************/
using namespace android;
using namespace NSCam;
using namespace NSCam::v3::pipeline::policy::pipelinesetting;


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)


/******************************************************************************
 *
 ******************************************************************************/
#define TRY_INVOKE_WITH_ERROR_RETURN(_policy_table_, _function_, ...) \
    do { \
        if  ( CC_UNLIKELY(_policy_table_==nullptr) ) { \
            MY_LOGE("bad policy table"); \
            return -ENODEV; \
        } \
        if  ( CC_UNLIKELY(_policy_table_->_function_==nullptr) ) { \
            MY_LOGE("bad policy table"); \
            return -ENOSYS; \
        } \
        auto err = _policy_table_->_function_(__VA_ARGS__); \
        if  ( CC_UNLIKELY(0 != err) ) { \
            MY_LOGE("err:%d(%s)", err, ::strerror(-err)); \
            return err; \
        } \
    } while (0)


/******************************************************************************
 *
 ******************************************************************************/
PipelineSettingPolicyImpl::
PipelineSettingPolicyImpl(
    CreationParams const& creationParams
)
    : IPipelineSettingPolicy()
    , mPipelineStaticInfo(creationParams.pPipelineStaticInfo)
    , mPipelineUserConfiguration(creationParams.pPipelineUserConfiguration)
    , mPolicyTable(creationParams.pPolicyTable)
    , mMediatorTable(creationParams.pMediatorTable)
{
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineSettingPolicyImpl::
decideConfiguredAppImageStreamMaxBufNum(
    ParsedAppImageStreamInfo* pInOut __unused,
    StreamingFeatureSetting const* pStreamingFeatureSetting __unused,
    CaptureFeatureSetting const* pCaptureFeatureSetting __unused
) -> int
{
    TRY_INVOKE_WITH_ERROR_RETURN(
        mPolicyTable,
        fConfigStreamInfo_AppImageStreamInfoMaxBufNum,
        pInOut, pStreamingFeatureSetting, pCaptureFeatureSetting,
        mPipelineStaticInfo.get(), mPipelineUserConfiguration.get()
    );
    return 0;//OK
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineSettingPolicyImpl::
evaluateConfiguration(
    ConfigurationOutputParams& out __unused,
    ConfigurationInputParams const& in __unused
) -> int
{
    auto const pMediator = mMediatorTable->pConfigSettingPolicyMediator;
    if  ( CC_UNLIKELY(pMediator==nullptr) ) {
        MY_LOGE("Bad pConfigSettingPolicyMediator");
        return -ENODEV;
    }

    return pMediator->evaluateConfiguration(out, in);
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineSettingPolicyImpl::
evaluateRequest(
    RequestOutputParams& out __unused,
    RequestInputParams const& in __unused
) -> int
{
    auto const pMediator = mMediatorTable->pRequestSettingPolicyMediator;
    if  ( CC_UNLIKELY(pMediator==nullptr) ) {
        MY_LOGE("Bad pRequestSettingPolicyMediator");
        return -ENODEV;
    }

    return pMediator->evaluateRequest(out, in);
}


/******************************************************************************
 *
 ******************************************************************************/
auto
PipelineSettingPolicyImpl::
sendPolicyDataCallback(
    PolicyType policyType,
    MUINTPTR arg1,
    MUINTPTR arg2,
    MUINTPTR arg3
) -> bool
{
    if(mPolicyTable == nullptr)
    {
        return false;
    }
    switch(policyType)
    {
        case PolicyType::SensorControlPolicy:
            if(mPolicyTable->pRequestSensorControlPolicy != nullptr)
            {
                mPolicyTable->pRequestSensorControlPolicy->sendPolicyDataCallback(
                                            arg1,
                                            arg2,
                                            arg3);
            }
        break;
        default:
            MY_LOGW("cannot find specific policy, please check setting.");
        break;
    }
    return true;
}

