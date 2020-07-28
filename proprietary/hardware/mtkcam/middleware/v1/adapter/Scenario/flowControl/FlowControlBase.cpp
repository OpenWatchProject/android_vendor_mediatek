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

#define LOG_TAG "MtkCam/FlowControlBase"
//
#include "inc/FlowControlBase.h"
#include "MyUtils.h"
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/aaa/IIspMgr.h>

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#if 1
#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")
#else
#define FUNC_START
#define FUNC_END
#endif

using namespace android;
using namespace NSCamHW;
/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v1 {
namespace NSLegacyPipeline {
/******************************************************************************
 *
 ******************************************************************************/
status_t
FlowControlBase::
changeToPreviewStatus()
{
    if( enterScenarioControl(getScenario()) == NULL )
    {
        return BAD_VALUE;
    }
    return OK;
}

sp<IScenarioControl>
FlowControlBase::
enterScenarioControl(MINT32 scenario)
{
    CAM_TRACE_NAME("BWC");
    sp<IScenarioControl> pScenarioCtrl = IScenarioControl::create(getOpenId());
    if( pScenarioCtrl == NULL )
    {
        MY_LOGE("get Scenario Control fail");
    }
    else
    {
        pScenarioCtrl->enterScenario(scenario);
    }
    return pScenarioCtrl;
}

sp<IScenarioControl>
FlowControlBase::
enterScenarioControl(MINT32 scenario, const MSize &sensorSize, const MINT32 &sensorFps, MINT32 featureconfig, MSize videoSize, MINT32 camMode)
{
    CAM_TRACE_NAME("BWC");
    sp<IScenarioControl> pScenarioCtrl = IScenarioControl::create(getOpenId());
    if( pScenarioCtrl == NULL )
    {
        MY_LOGE("get Scenario Control fail");
    }
    else
    {
        IScenarioControl::ControlParam param;
        param.scenario = scenario;
        param.sensorSize = sensorSize;
        param.sensorFps  = sensorFps;
        param.featureFlag = featureconfig;
        param.videoSize = videoSize;
        param.camMode = camMode;
        pScenarioCtrl->enterScenario(param);
    }
    return pScenarioCtrl;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
FlowControlBase::
decideLcsoImage(
    HwInfoHelper& helper,
    MUINT32 const bitDepth,
    MSize         referenceSize,
    MUINT const   usage,
    MINT32 const  minBuffer,
    MINT32 const  maxBuffer,
    sp<IImageStreamInfo>& rpImageStreamInfo
)
{
    NS3Av3::LCSO_Param lcsoParam;
    if ( auto pIspMgr = MAKE_IspMgr() ) {
        pIspMgr->queryLCSOParams(lcsoParam);
    }
    //
    MY_LOGI("lcso num:%d-%d bitDepth:%d format:%d referenceSize:%dx%d, actual size:%dx%d, stride:%d",
                minBuffer,
                maxBuffer,
                bitDepth,
                lcsoParam.format,
                referenceSize.w, referenceSize.h,
                lcsoParam.size.w, lcsoParam.size.h,
                lcsoParam.stride
            );
    rpImageStreamInfo =
        createRawImageStreamInfo(
            "Hal:Image:LCSraw",
            eSTREAMID_IMAGE_PIPE_RAW_LCSO,
            eSTREAMTYPE_IMAGE_INOUT,
            maxBuffer, minBuffer,
            usage, lcsoParam.format, lcsoParam.size, lcsoParam.stride
            );

    return OK;
}

/******************************************************************************
*
*******************************************************************************/
int
FlowControlBase::
getPreviewRawBitDepth(HwInfoHelper& helper)
{
    //return NSCam::Utils::getRawBitDepth();
    // if you remove "MINT32 bitDepth = 10" or force change value, please tell
    // dual cam for sync.
    MINT32 bitDepth = 10;
    helper.getRecommendRawBitDepth(bitDepth);
    return bitDepth;
}

/******************************************************************************
*
*******************************************************************************/
void
FlowControlBase::
pauseSwitchModeFlow()
{
    if(mbSwitchModeEnable)
    {
        MY_LOGD("SwitchModeFlow: %s before switchSensorModeLock (+)",__FUNCTION__);
        Mutex::Autolock _l(mSwitchModeLock);
        MY_LOGD("SwitchModeFlow: %s after switchSensorModeLock (-)",__FUNCTION__);
        mPauseSwitchCount++;
        MY_LOGD("SwitchModeFlow: pauseSwitchModeFlow PauseCnt:%d",mPauseSwitchCount);
        mbPauseSwitchModeFlow = MTRUE;
    }
}

/******************************************************************************
*
*******************************************************************************/
void
FlowControlBase::
resumeSwitchModeFlow()
{
    if(mbSwitchModeEnable)
    {
        MY_LOGD("SwitchModeFlow: %s before switchSensorModeLock (+)",__FUNCTION__);
        Mutex::Autolock _l(mSwitchModeLock);
        MY_LOGD("SwitchModeFlow: %s after switchSensorModeLock (-)",__FUNCTION__);
        mPauseSwitchCount--;

        if(mPauseSwitchCount<=0)
        {
            mbPauseSwitchModeFlow = MFALSE;
            mPauseSwitchCount=0;
            MY_LOGD("SwitchModeFlow: resumeSwitchModeFlow PauseCnt:%d, resume flow successfully",mPauseSwitchCount);
        }
        else
        {
            MY_LOGD("resumeSwitchModeFlow PauseCnt:%d, need to wait PauseCnt==0 to resume flow",mPauseSwitchCount);
        }
    }
}
/******************************************************************************
*
*******************************************************************************/
int
FlowControlBase::
getNowSensorModeStatusForSwitchFlow()
{
    if(mbSwitchModeEnable)
    {
        MY_LOGD("SwitchModeFlow: %s before getNowSensorModeStatusForSwitchFlow (+)",__FUNCTION__);
        Mutex::Autolock _l(mNowSensorModeStatusForSwitchFlowLock);
        MY_LOGD("SwitchModeFlow: %s after getNowSensorModeStatusForSwitchFlow (-)",__FUNCTION__);

        MY_LOGD("getNowSensorModeStatusForSwitchFlow return (%d)",mNowSensorModeStatusForSwitchFlow);
        return mNowSensorModeStatusForSwitchFlow;
    }
    return 0;
}

/******************************************************************************
*
*******************************************************************************/
void
FlowControlBase::
setNowSensorModeStatusForSwitchFlow(int nowStatus)
{
    if(mbSwitchModeEnable)
    {
        MY_LOGD("SwitchModeFlow: %s before setNowSensorModeStatusForSwitchFlow (+)",__FUNCTION__);
        Mutex::Autolock _l(mNowSensorModeStatusForSwitchFlowLock);
        MY_LOGD("SwitchModeFlow: %s after setNowSensorModeStatusForSwitchFlow (-)",__FUNCTION__);

        mNowSensorModeStatusForSwitchFlow = nowStatus;
        MY_LOGD("setNowSensorModeStatusForSwitchFlow (%d)",mNowSensorModeStatusForSwitchFlow);
    }
}

};  //namespace NSPipelineContext
};  //namespace v1
};  //namespace NSCam
