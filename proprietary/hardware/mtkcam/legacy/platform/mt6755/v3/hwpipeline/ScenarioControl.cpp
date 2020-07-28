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

#define LOG_TAG "MtkCam/HwPipelineSC"
//
#include "ScenarioControl.h"
#include <bandwidth_control.h>
#include <dlfcn.h>
//
#include <vendor/mediatek/hardware/power/1.1/IPower.h>
#include <vendor/mediatek/hardware/power/1.1/types.h>
using namespace vendor::mediatek::hardware::power::V1_1;
//
#include <utils/Errors.h>
#include <utils/Mutex.h>
#include <utils/KeyedVector.h>
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;


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


/******************************************************************************
 *
 ******************************************************************************/
#define DUMP_SCENARIO_PARAM(_id, _str, _param)                  \
    do{                                                         \
        MY_LOGD_IF(1, "(id:%d) %s: scenario %d: size %dx%d@%d", \
                _id,                                            \
                _str,                                           \
                _param.scenario,                                \
                _param.sensorSize.w, _param.sensorSize.h,       \
                _param.sensorFps                                \
                );                                              \
    } while(0)

/******************************************************************************
 *
 ******************************************************************************/
BWC_PROFILE_TYPE mapToBWCProfile(ScenarioControl::ePipelineScenario const scenario)
{
    switch(scenario)
    {
        case ScenarioControl::Scenario_NormalStreaming:
            return BWCPT_CAMERA_ZSD;
        case ScenarioControl::Scenario_VideoRecording:
            return BWCPT_VIDEO_RECORD_CAMERA;
        default:
            MY_LOGE("not supported scenario %d", scenario);
    }
    return BWCPT_NONE;
}


/******************************************************************************
 *
 ******************************************************************************/
Mutex                               gLock;
DefaultKeyedVector<
    MINT32, wp<ScenarioControl>
    >                               gScenarioControlMap;


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
isMultiOpen()
{
    Mutex::Autolock _l(gLock);
    return gScenarioControlMap.size() > 1;
}


/******************************************************************************
 *
 ******************************************************************************/
sp<IScenarioControl>
ScenarioControl::
create(MINT32 const openId)
{
    Mutex::Autolock _l(gLock);
    sp<ScenarioControl> pControl = NULL;
    ssize_t index = gScenarioControlMap.indexOfKey(openId);
    if( index < 0 ) {
        pControl = new ScenarioControl(openId);
        gScenarioControlMap.add(openId, pControl);
    }
    else {
        MY_LOGW("dangerous, already have user with open id %d", openId);
        pControl = gScenarioControlMap.valueAt(index).promote();
    }
    //
    if( ! pControl.get() ) {
        MY_LOGE("cannot create properly");
    }
    //
    return pControl;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
ScenarioControl::
setScenario(
    MVOID *pBWCparam
)
{
    BWC_Scenario_Param *bwcparam = static_cast<BWC_Scenario_Param*>(pBWCparam);
    MERROR err = enterScenario(*bwcparam);
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
ScenarioControl::
ScenarioControl(MINT32 const openId)
    : mOpenId(openId)
    , mCurPerfHandle(-1)
{
}


/******************************************************************************
 *
 ******************************************************************************/
void
ScenarioControl::
onLastStrongRef(const void* /*id*/)
{
    // reset
    if( mCurParam.scenario != Scenario_None ) {
        exitScenario();
    }
    //
    {
        Mutex::Autolock _l(gLock);
        ssize_t index = gScenarioControlMap.indexOfKey(mOpenId);
        if( index >= 0 ) {
            gScenarioControlMap.removeItemsAt(index);
        }
        else {
            MY_LOGW("dangerous, has been removed (open id %d)", mOpenId);
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
ScenarioControl::
enterScenario(
    BWC_Scenario_Param const & param
)
{
    if( mCurParam.scenario != Scenario_None ) {
        MY_LOGD("exit previous setting");
        exitScenario();
    }
    DUMP_SCENARIO_PARAM(mOpenId, "enter:", param);
    //
    BWC_PROFILE_TYPE type = mapToBWCProfile(param.scenario);
    if( type == BWCPT_NONE )
        return BAD_VALUE;
    //
#ifndef BUILD_ENG
    if( param.scenario == Scenario_VideoRecording )
    {
        sp<IPower> pPerf = IPower::getService();
        //
        int handle = pPerf->scnReg();
        if( handle != -1 )
        {
            pPerf->scnConfig(handle, MtkPowerCmd::CMD_SET_CLUSTER_CPU_CORE_MIN, 0, 2, 0, 0); // min 2xLL 0xL
            pPerf->scnConfig(handle, MtkPowerCmd::CMD_SET_CLUSTER_CPU_CORE_MAX, 0, 4, 0, 0); // max 4xLL 0xL
            pPerf->scnConfig(handle, MtkPowerCmd::CMD_SET_CPUFREQ_HISPEED_FREQ, 598000, 0, 0, 0); // freq 598mhz
            pPerf->scnConfig(handle, MtkPowerCmd::CMD_SET_CPUFREQ_MIN_SAMPLE_TIME, 20, 0, 0, 0); // quick fall 20ms
            //
            pPerf->scnEnable(handle, 0);
            MY_LOGD("perfService enable");
        }
        else
        {
            MY_LOGW("cannot regscn");
        }
        //
        mCurPerfHandle = handle;
    }
#endif
    //
    mmdvfs_set(
            type,
            MMDVFS_SENSOR_SIZE,         param.sensorSize.size(),
            MMDVFS_SENSOR_FPS,          param.sensorFps,
            MMDVFS_CAMERA_MODE_PIP,     isMultiOpen(),
            MMDVFS_CAMERA_MODE_VFB,     0, //TODO
            MMDVFS_CAMERA_MODE_EIS_2_0, 0, //TODO
            MMDVFS_CAMERA_MODE_IVHDR,   0, //TODO
            MMDVFS_PARAMETER_EOF);
    //
    BWC BwcIns;
    BwcIns.Profile_Change(type,true);
    //
    // keep param
    mCurParam = param;
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
ScenarioControl::
exitScenario()
{
    if( mCurParam.scenario == Scenario_None ) {
        MY_LOGD("already exit");
        return OK;
    }
    DUMP_SCENARIO_PARAM(mOpenId, "exit:", mCurParam);
    BWC_PROFILE_TYPE type = mapToBWCProfile(mCurParam.scenario);
    if( type == BWCPT_NONE )
        return BAD_VALUE;
    //
    BWC BwcIns;
    BwcIns.Profile_Change(type,false);
    //
    if( mCurPerfHandle != -1 )
    {
        sp<IPower> pPerf = IPower::getService();
        //
        pPerf->scnDisable(mCurPerfHandle);
        pPerf->scnUnreg(mCurPerfHandle);
        //
        mCurPerfHandle = -1;
        MY_LOGD("perfService disable");
    }
    //
    // reset param
    mCurParam.scenario = Scenario_None;
    //
    return OK;
}
