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
#include <fcntl.h>
#include <dlfcn.h>
#include "ScenarioControl.h"
#include <vector>
#include <utils/Errors.h>
#include <utils/Mutex.h>
#include <utils/KeyedVector.h>
#include <bandwidth_control.h>
//
#include <vendor/mediatek/hardware/power/1.1/IPower.h>
#include <vendor/mediatek/hardware/power/1.1/types.h>
using namespace vendor::mediatek::hardware::power::V1_1;
//
using namespace std;
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
#define DUMP_SCENARIO_PARAM(_str, _param)                 \
    do{                                                   \
        MY_LOGD_IF(1, "%s: scenario %d: size %dx%d@%d",   \
                _str,                                     \
                _param.scenario,                          \
                _param.sensorSize.w, _param.sensorSize.h, \
                _param.sensorFps                          \
                );                                        \
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

vector<int>                         gvPerfHandle;

/******************************************************************************
 *
 ******************************************************************************/
sp<ScenarioControl>
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
    if(gScenarioControlMap.size()==2)
    {
        MY_LOGI("enable termal policy for front&back cam both open");
        char thermal_str[]="thermal_policy_01";
        if(!setThermalPolicy(thermal_str, 1))
        {
            MY_LOGE("fail to set thermal policy");
        }
    }
    return pControl;
}


/******************************************************************************
 *
 ******************************************************************************/
ScenarioControl::
ScenarioControl(MINT32 const openId)
    : mOpenId(openId)
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
        if(gScenarioControlMap.size()==1)
        {
            MY_LOGI("disable termal policy");
            char thermal_str[]="thermal_policy_01";
            if(!setThermalPolicy(thermal_str, 0))
            {
                MY_LOGE("fail to set thermal policy");
            }
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
    DUMP_SCENARIO_PARAM("enter:", param);
    //
    BWC_PROFILE_TYPE type = mapToBWCProfile(param.scenario);
    if( type == BWCPT_NONE )
        return BAD_VALUE;
    //
    if( (param.scenario == Scenario_VideoRecording)||
        (param.scenario == Scenario_NormalStreaming))
    {
        sp<IPower> pPerf = IPower::getService();
        //
        int handle = pPerf->scnReg();
        if( handle != -1 )
        {
            Mutex::Autolock _l(gLock);
            // single cam case
            if(0 == gvPerfHandle.size())
            {
                pPerf->scnConfig(handle, MtkPowerCmd::CMD_SET_CLUSTER_CPU_CORE_MIN, 0, 4, 0, 0); // min 4xLL 0xL
                pPerf->scnEnable(handle, 0);
                MY_LOGI("PowerHal enable (cnt=%u) for single cam (min CPU: 4LL)",(gvPerfHandle.size()+1));
            }
            else    // open front&back cam case (PIP)
            {
                pPerf->scnConfig(handle, MtkPowerCmd::CMD_SET_CLUSTER_CPU_CORE_MIN, 0, 4, 0, 0); // min 4 cpu for cluster 0
                pPerf->scnConfig(handle, MtkPowerCmd::CMD_SET_CLUSTER_CPU_FREQ_MIN, 0, 1300000, 0, 0); // 1.3GHz for cluster 0

                pPerf->scnConfig(handle, MtkPowerCmd::CMD_SET_CLUSTER_CPU_CORE_MIN, 1, 2, 0, 0); // min 2 cpu for cluster 1
                pPerf->scnConfig(handle, MtkPowerCmd::CMD_SET_CLUSTER_CPU_FREQ_MIN, 1, 1300000, 0, 0); // 1.3GHz for cluster 1

                pPerf->scnConfig(handle, MtkPowerCmd::CMD_SET_CPUFREQ_HISPEED_FREQ, 1300000, 0, 0, 0); // 1.3GHz for high speed
                pPerf->scnEnable(handle, 0);
                MY_LOGI("PowerHal enable (cnt=%u) for front/back cam both open (min CPU<1.3GHz>: cluster0 ->4 cpu; cluster1 ->2 cpu)",
                    (gvPerfHandle.size()+1));
            }
            gvPerfHandle.push_back(handle);
        }
        else
        {
            MY_LOGW("cannot regscn");
        }
    }
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
    DUMP_SCENARIO_PARAM("exit:", mCurParam);
    BWC_PROFILE_TYPE type = mapToBWCProfile(mCurParam.scenario);
    if( type == BWCPT_NONE )
        return BAD_VALUE;
    //
    BWC BwcIns;
    BwcIns.Profile_Change(type,false);

    // disable power hal
    {
        Mutex::Autolock _l(gLock);
        int handle = -1;
        if(gvPerfHandle.size()>0)
            handle = gvPerfHandle.back();
        if( handle != -1 )
        {
            sp<IPower> pPerf = IPower::getService();
            //
            pPerf->scnDisable(handle);
            pPerf->scnUnreg(handle);
            MY_LOGI("PowerHal disable (cnt=%lu)",gvPerfHandle.size());
            //
            gvPerfHandle.pop_back();
        }
    }
    //
    // reset param
    mCurParam.scenario = Scenario_None;
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
ScenarioControl::
setThermalPolicy(char* policy, bool usage)
{
    void *handle = dlopen("/system/vendor/lib/libmtcloader.so", RTLD_NOW);
    if (NULL == handle)
    {
        MY_LOGW("%s, can't load thermal library: %s", __FUNCTION__, dlerror());
        return false;
    }
    else
    {
        typedef int (*load_change_policy)(char *, int);

        void *func = dlsym(handle, "change_policy");
        load_change_policy change_policy = reinterpret_cast<load_change_policy>(func);

        change_policy(policy, usage);

        dlclose(handle);
    }
    return true;
}

