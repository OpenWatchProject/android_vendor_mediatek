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

#define LOG_TAG "MtkCam3/HwUtils"
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/utils/hw/CamManager.h>
#include <mtkcam/utils/hw/IScenarioControlV3.h>
#include <mtkcam/utils/hw/HwInfoHelper.h>
#include <bandwidth_control.h>
#include <dlfcn.h>
//
#include <string.h>
#include <cutils/properties.h>
#include <utils/KeyedVector.h>
//
#include <vendor/mediatek/hardware/power/2.0/IPower.h>
#include <vendor/mediatek/hardware/power/2.0/types.h>
using namespace vendor::mediatek::hardware::power::V2_0;
//
#include <camera_custom_scenario_control.h>
//
#ifdef ENABLE_STEREO_PERFSERVICE
#include <camera_custom_stereo.h>

#endif
CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_MODEL);

using namespace android;
using namespace NSCam;
using namespace NSCam::Utils;
using namespace NSCam::v3::pipeline::model;

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
        MY_LOGD_IF(1, "(id:%d) %s: scenario %d: size %dx%d@%d feature 0x%x", \
                _id,                                            \
                _str,                                           \
                _param.scenario,                                \
                _param.sensorSize.w, _param.sensorSize.h,       \
                _param.sensorFps,                               \
                _param.featureFlag                              \
                );                                              \
    } while(0)

#define SCENARIO_BOOST_MASK(x, y)          (x) |= (1<<(y))
#define SCENARIO_IS_ENABLED(x, y)          (((x) & (1<<(y))) ? MTRUE: MFALSE)

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace model {

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Bandwidth control & dvfs
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class ScenarioControlV3
    : public IScenarioControlV3
{
public:
                                    ScenarioControlV3(MINT32 const openId);
                                    ~ScenarioControlV3() {}

public:
    virtual MERROR                  enterScenario(
                                        ControlParam const& param
                                    );

    virtual MERROR                  enterScenario(
                                        MINT32 const scenario
                                    );

    virtual MERROR                  exitScenario();

    virtual MERROR                  getCurrentStatus(int &curScen, int &curBoost, int64_t &lastFrameNo, int &featureFlag);

    virtual MERROR                  boostScenario(int const scenario, int const feature, int64_t const frameNo);

    virtual MERROR                  checkIfNeedExitBoost(int64_t const frameNo, int const forceExit);

    virtual MERROR                  setQOSParams(std::vector<SensorParam> const &vParams);
    //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  RefBase Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Operations.
    virtual void                    onLastStrongRef(const void* id);

private:
    // Perfservice control
    virtual MERROR                  enterPerfService(
                                            ControlParam const & param);
    virtual MERROR                  exitPerfService();

    virtual MERROR                  changeBWCProfile(ControlParam const & param);

private:
    MINT32 const                    mOpenId;
    ControlParam                    mCurParam;
    //
    int                             mCurPerfHandle;
    MINT                            mEngMode;
    Mutex                           mLock;
    int                             mCurBoostMode; // bit
    int64_t                         mLastBoostFrameNo;
    std::vector<std::shared_ptr<MMDVFS_QOS_PARAMETER>>    mvpParams;
};
};
};
};
};

/******************************************************************************
 *
 ******************************************************************************/
BWC_PROFILE_TYPE mapToBWCProfileV3(MINT32 const scenario)
{
    switch(scenario)
    {
        case IScenarioControlV3::Scenario_NormalPreivew:
            return BWCPT_CAMERA_PREVIEW;
        case IScenarioControlV3::Scenario_ZsdPreview:
            return BWCPT_CAMERA_ZSD;
        case IScenarioControlV3::Scenario_VideoRecord:
            return BWCPT_VIDEO_RECORD_CAMERA;
        case IScenarioControlV3::Scenario_VSS:
            return BWCPT_VIDEO_SNAPSHOT;
        case IScenarioControlV3::Scenario_Capture:
            return BWCPT_CAMERA_CAPTURE;
        case IScenarioControlV3::Scenario_ContinuousShot:
            return BWCPT_CAMERA_ICFP;
        case IScenarioControlV3::Scenario_VideoTelephony:
            return BWCPT_VIDEO_TELEPHONY;
        case IScenarioControlV3::Scenario_HighSpeedVideo:
            return BWCPT_VIDEO_RECORD_SLOWMOTION;
        default:
            MY_LOGE("not supported scenario %d", scenario);
    }
    return BWCPT_NONE;
}

/******************************************************************************
 *
 ******************************************************************************/
Mutex&                               gLock = *new Mutex();
DefaultKeyedVector<
    MINT32, wp<ScenarioControlV3>
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
MBOOL
isDualZoomMode(MINT32 featureFlag)
{
    bool ret;
    ret = FEATURE_CFG_IS_ENABLED(featureFlag, IScenarioControlV3::FEATURE_DUALZOOM_PREVIEW) ||
          FEATURE_CFG_IS_ENABLED(featureFlag, IScenarioControlV3::FEATURE_DUALZOOM_RECORD) ||
          FEATURE_CFG_IS_ENABLED(featureFlag, IScenarioControlV3::FEATURE_DUALZOOM_FUSION_CAPTURE);
    MY_LOGD("(dvfs) set dual cam: %d", ret);
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
isInStereoMode(MINT32 featureFlag)
{
    return FEATURE_CFG_IS_ENABLED(featureFlag, IScenarioControlV3::FEATURE_VSDOF_PREVIEW) ||
           FEATURE_CFG_IS_ENABLED(featureFlag, IScenarioControlV3::FEATURE_VSDOF_RECORD) ||
           FEATURE_CFG_IS_ENABLED(featureFlag, IScenarioControlV3::FEATURE_STEREO_CAPTURE) ||
           FEATURE_CFG_IS_ENABLED(featureFlag, IScenarioControlV3::FEATURE_BMDENOISE_PREVIEW) ||
           FEATURE_CFG_IS_ENABLED(featureFlag, IScenarioControlV3::FEATURE_BMDENOISE_CAPTURE) ||
           FEATURE_CFG_IS_ENABLED(featureFlag, IScenarioControlV3::FEATURE_BMDENOISE_MFHR_CAPTURE);
}

/******************************************************************************
 *
 ******************************************************************************/
sp<IScenarioControlV3>
IScenarioControlV3::
create(MINT32 const openId)
{
    int RetryCount = 0;
    sp<ScenarioControlV3> pControl = NULL;
GET_RETRY:
    {
        Mutex::Autolock _l(gLock);
        ssize_t index = gScenarioControlMap.indexOfKey(openId);
        if( index < 0 ) {
            pControl = new ScenarioControlV3(openId);
            gScenarioControlMap.add(openId, pControl);
        }
        else {
            MY_LOGW("dangerous, already have user with open id %d", openId);
            pControl = gScenarioControlMap.valueAt(index).promote();
        }
    }
    //
    if( ! pControl.get() ) {
        RetryCount++;
        MY_LOGW("cannot create properly, retry : %d", RetryCount);
        if (RetryCount <= 5)
        {
            usleep(10*1000); // sleep 10ms
            goto GET_RETRY;
        }
    }
    //
    return pControl;
}


/******************************************************************************
 *
 ******************************************************************************/
ScenarioControlV3::
ScenarioControlV3(MINT32 const openId)
    : mOpenId(openId)
    , mCurPerfHandle(-1)
    , mCurBoostMode(0)
    , mLastBoostFrameNo(-1)
{
    mCurParam.scenario = Scenario_None;
    mEngMode = -1;
    if (mEngMode == -1)
    {
        char value[PROPERTY_VALUE_MAX];
        property_get("ro.build.type", value, "eng");
        if (0 == strcmp(value, "eng")) {
            mEngMode = 1;
        }
        else
        {
            mEngMode = 0;
        }
    }
}

/******************************************************************************
 *
 ******************************************************************************/
void
ScenarioControlV3::
onLastStrongRef(const void* /*id*/)
{
    // reset
    if( mCurParam.scenario != Scenario_None ) {
        exitScenario();
    }
    exitPerfService();
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
ScenarioControlV3::
enterPerfService(
    ControlParam const & param
)
{
    //
    MY_LOGD("+");

    if (0)
    {
        sp<IPower> pPerf = IPower::getService();

        int handle = pPerf->scnReg();
        //
        if( handle != -1 && mEngMode != 1){
            bool useCpuCtrl = false;
            KeyedVector<ScenInfoKey, int> scenInfo;
            Vector<CpuCtrlParam> cpuCtrlParam;
            //
            scenInfo.clear();
            scenInfo.add(ScenInfoKey::KEY_SCENARIO, param.scenario);
            scenInfo.add(ScenInfoKey::KEY_FEATURE_FLAG, param.featureFlag);
            scenInfo.add(ScenInfoKey::KEY_DEVICE_COUNT, CamManager::getInstance()->getDeviceCount());
            scenInfo.add(ScenInfoKey::KEY_CAM_MODE, param.camMode);
            scenInfo.add(ScenInfoKey::KEY_CUSTOM_OPTION, param.supportCustomOption);//cutomOption
            //
            {
                size_t i = 0;
                for( i = 0 ; i < scenInfo.size() ; i++ )
                {
                    MY_LOGD("Scenario Info %d/%d, (key,value)=(%d,%d)",
                            i, scenInfo.size(),
                            scenInfo.keyAt(i),
                            scenInfo.valueAt(i));
                }
            }
            useCpuCtrl = get_cpu_ctrl(scenInfo, cpuCtrlParam);
            //
            switch(param.scenario){
                case Scenario_ContinuousShot:
                    MY_LOGD("CPU boost for cshot");
                    // enter cpu performance mode
                    pPerf->scnConfig(handle, MtkPowerCmd::CMD_SET_CPU_PERF_MODE , 1, 0, 0, 0);
                    pPerf->scnEnable(handle, (int)MtkHintOp::MTK_HINT_ALWAYS_ENABLE);
                    break;
                case Scenario_VideoRecord:{
                    //For Save Power
                    if( param.videoSize.h <= 1088 ) //For max 1080p
                    {
#define VDEC_CPU_MIN_SAMPLE_TIME 20000
#define VDEC_CPU_ABOVE_HISPEED_DELAY 80000
                        pPerf->scnConfig(handle, MtkPowerCmd::CMD_SET_CPUFREQ_MIN_SAMPLE_TIME , VDEC_CPU_MIN_SAMPLE_TIME, 0, 0, 0);         // quick fall - 20ms
                        pPerf->scnConfig(handle, MtkPowerCmd::CMD_SET_CPUFREQ_ABOVE_HISPEED_DELAY, VDEC_CPU_ABOVE_HISPEED_DELAY, 0, 0, 0);  // slow rise - 80ms
                        MY_LOGD("Enable CPU quick fall - 20ms & slow rise - 80ms");
#undef VDEC_CPU_MIN_SAMPLE_TIME
#undef VDEC_CPU_ABOVE_HISPEED_DELAY
                    }
                    //
                    break;
                }
                case Scenario_ZsdPreview:{
#ifdef ENABLE_STEREO_PERFSERVICE
                    std::vector<int> cpu_core;
                    std::vector<int> cpu_frequency_max;
                    std::vector<int> cpu_frequency_min;
                    MBOOL isUseStereoSetting = MFALSE;
                    int numOfCluster = pPerf->querySysInfo(MtkQueryCmd::CMD_GET_CLUSTER_NUM, 0);
                    int coreMax = 0;
                    int freqMax = 0;
                    int freqMin = 0;
                    MY_LOGD("Number of cluster: %d", numOfCluster);
                    //
                    if(FEATURE_CFG_IS_ENABLED(param.featureFlag, FEATURE_VSDOF_PREVIEW))
                    {
                        cpu_core = get_stereo_cam_cpu_num(ENUM_STEREO_CAM_FEATURE::eStereoCamVsdof, ENUM_STEREO_CAM_SCENARIO::eStereoCamPreview);
                        MBOOL result = get_stereo_cam_cpu_frequency(ENUM_STEREO_CAM_FEATURE::eStereoCamVsdof, ENUM_STEREO_CAM_SCENARIO::eStereoCamPreview, cpu_frequency_min, cpu_frequency_max);
                        if(!result)
                        {
                            MY_LOGE("get_stereo_cam_cpu_frequency(vsdof preview) fail");
                            return UNKNOWN_ERROR;
                        }
                        if(cpu_core.size() != numOfCluster &&
                           cpu_frequency_max.size() != numOfCluster &&
                           cpu_frequency_min.size() != numOfCluster)
                        {
                            MY_LOGE("cpu_core/cpu_frequency is not equal to cluster(%d)", numOfCluster);
                            return UNKNOWN_ERROR;
                        }
                        MY_LOGD("feature flag vsdof preview");
                        isUseStereoSetting = MTRUE;
                    }
                    if(FEATURE_CFG_IS_ENABLED(param.featureFlag, FEATURE_VSDOF_RECORD))
                    {
                        cpu_core = get_stereo_cam_cpu_num(ENUM_STEREO_CAM_FEATURE::eStereoCamVsdof, ENUM_STEREO_CAM_SCENARIO::eStereoCamRecord);
                        MBOOL result = get_stereo_cam_cpu_frequency(ENUM_STEREO_CAM_FEATURE::eStereoCamVsdof, ENUM_STEREO_CAM_SCENARIO::eStereoCamRecord, cpu_frequency_min, cpu_frequency_max);
                        if(!result)
                        {
                            MY_LOGE("get_vsdof_cpu_frequency fail");
                            return UNKNOWN_ERROR;
                        }
                        if(cpu_core.size() != numOfCluster &&
                           cpu_frequency_max.size() != numOfCluster &&
                           cpu_frequency_min.size() != numOfCluster)
                        {
                            MY_LOGE("cpu_core/cpu_frequency is not equal to cluster(%d)", numOfCluster);
                            return UNKNOWN_ERROR;
                        }
                        MY_LOGD("feature flag vsdof record");
                        isUseStereoSetting = MTRUE;
                    }
                    if(FEATURE_CFG_IS_ENABLED(param.featureFlag, FEATURE_STEREO_CAPTURE))
                    {
                        cpu_core = get_stereo_cam_cpu_num(ENUM_STEREO_CAM_FEATURE::eStereoCamStereo_Capture, ENUM_STEREO_CAM_SCENARIO::eStereoCamCapture);
                        MBOOL result = get_stereo_cam_cpu_frequency(ENUM_STEREO_CAM_FEATURE::eStereoCamStereo_Capture, ENUM_STEREO_CAM_SCENARIO::eStereoCamCapture, cpu_frequency_min, cpu_frequency_max);
                        if(!result)
                        {
                            MY_LOGE("get_vsdof_cpu_frequency fail");
                            return UNKNOWN_ERROR;
                        }
                        if(cpu_core.size() != numOfCluster &&
                           cpu_frequency_max.size() != numOfCluster &&
                           cpu_frequency_min.size() != numOfCluster)
                        {
                            MY_LOGE("cpu_core/cpu_frequency is not equal to cluster(%d)", numOfCluster);
                            return UNKNOWN_ERROR;
                        }
                        MY_LOGD("feature flag vsdof capture");
                        isUseStereoSetting = MTRUE;
                    }
                    if(FEATURE_CFG_IS_ENABLED(param.featureFlag, FEATURE_BMDENOISE_PREVIEW))
                    {
                        cpu_core = get_stereo_cam_cpu_num(ENUM_STEREO_CAM_FEATURE::eStereoBMDnoise, ENUM_STEREO_CAM_SCENARIO::eStereoCamPreview);
                        MBOOL result = get_stereo_cam_cpu_frequency(ENUM_STEREO_CAM_FEATURE::eStereoBMDnoise, ENUM_STEREO_CAM_SCENARIO::eStereoCamPreview, cpu_frequency_min, cpu_frequency_max);
                        if(!result)
                        {
                            MY_LOGE("get_bmdenoise_cpu_frequency fail");
                            return UNKNOWN_ERROR;
                        }
                        if(cpu_core.size() != numOfCluster &&
                           cpu_frequency_max.size() != numOfCluster &&
                           cpu_frequency_min.size() != numOfCluster)
                        {
                            MY_LOGE("cpu_core/cpu_frequency is not equal to cluster(%d)", numOfCluster);
                            return UNKNOWN_ERROR;
                        }
                        MY_LOGD("feature flag bmdenoise preview");
                        isUseStereoSetting = MTRUE;
                    }
                    if(FEATURE_CFG_IS_ENABLED(param.featureFlag, FEATURE_BMDENOISE_CAPTURE))
                    {
                        cpu_core = get_stereo_cam_cpu_num(ENUM_STEREO_CAM_FEATURE::eStereoBMDnoise, ENUM_STEREO_CAM_SCENARIO::eStereoCamCapture);
                        MBOOL result = get_stereo_cam_cpu_frequency(ENUM_STEREO_CAM_FEATURE::eStereoBMDnoise, ENUM_STEREO_CAM_SCENARIO::eStereoCamCapture, cpu_frequency_min, cpu_frequency_max);
                        if(!result)
                        {
                            MY_LOGE("get_bmdenoise_cpu_frequency fail");
                            return UNKNOWN_ERROR;
                        }
                        if(cpu_core.size() != numOfCluster &&
                           cpu_frequency_max.size() != numOfCluster &&
                           cpu_frequency_min.size() != numOfCluster)
                        {
                            MY_LOGE("cpu_core/cpu_frequency is not equal to cluster(%d)", numOfCluster);
                            return UNKNOWN_ERROR;
                        }
                        MY_LOGD("feature flag bmdenoise capture");
                        isUseStereoSetting = MTRUE;
                        // GPU control (not support in power hal)
                        //int GPULevels = pPerf->GetCapability(CMD_GET_GPU_FREQ_LEVEL_COUNT);
                        //pPerf->userRegScnConfig(handle, CMD_SET_GPU_FREQ_MIN, GPULevels-1, 0, 0, 0);
                        //MY_LOGD("Manually PerfService Control by Stereo Feature GPU min freq:%d", GPULevels);
                    }
                    if(FEATURE_CFG_IS_ENABLED(param.featureFlag, FEATURE_BMDENOISE_MFHR_CAPTURE))
                    {
                        cpu_core = get_stereo_cam_cpu_num(ENUM_STEREO_CAM_FEATURE::eStereoBMDnoise, ENUM_STEREO_CAM_SCENARIO::eStereoCamCapture);
                        MBOOL result = get_stereo_cam_cpu_frequency(ENUM_STEREO_CAM_FEATURE::eStereoBMDnoise, ENUM_STEREO_CAM_SCENARIO::eStereoCamCapture, cpu_frequency_min, cpu_frequency_max);
                        if(!result)
                        {
                            MY_LOGE("get_bmdenoise_cpu_frequency fail");
                            return UNKNOWN_ERROR;
                        }
                        if(cpu_core.size() != numOfCluster &&
                           cpu_frequency_max.size() != numOfCluster &&
                           cpu_frequency_min.size() != numOfCluster)
                        {
                            MY_LOGE("cpu_core/cpu_frequency is not equal to cluster(%d)", numOfCluster);
                            return UNKNOWN_ERROR;
                        }
                        MY_LOGD("feature flag bmdenoise capture");
                        isUseStereoSetting = MTRUE;
                        // GPU control (not support in power hal)
                        //int GPULevels = pPerf->GetCapability(CMD_GET_GPU_FREQ_LEVEL_COUNT);
                        //pPerf->userRegScnConfig(handle, CMD_SET_GPU_FREQ_MIN, GPULevels-1, 0, 0, 0);
                        //MY_LOGD("Manually PerfService Control by Stereo Feature GPU min freq:%d", GPULevels);
                    }
                    if(isUseStereoSetting)
                    {
                        MBOOL needSetMin = MFALSE, needSetMax = MFALSE;
                        for(int cluster = 0 ; cluster < numOfCluster; cluster ++){
                            coreMax = pPerf->querySysInfo(MtkQueryCmd::CMD_GET_CLUSTER_CPU_NUM, 0);
                            freqMax = pPerf->querySysInfo(MtkQueryCmd::CMD_GET_CLUSTER_CPU_FREQ_MAX, cluster);

                            if(coreMax == -1 || freqMax == -1){
                                MY_LOGE("Unablde to get cluster info! cluster:%d, coreMax:%d, freqMax:%d", cluster, coreMax, freqMax);
                                break;
                            }

                            coreMax = cpu_core[cluster];
                            if(cpu_frequency_min[cluster] == -2)
                            {
                                needSetMin = MFALSE;
                            }
                            else
                            {
                                if(cpu_frequency_min[cluster] == -1)
                                {
                                    freqMin = freqMax;
                                }
                                else
                                {
                                    freqMin = cpu_frequency_min[cluster];
                                }
                                needSetMin = MTRUE;
                            }
                            if(cpu_frequency_max[cluster] == -2)
                            {
                                needSetMax = MFALSE;
                            }
                            else
                            {
                                if(cpu_frequency_max[cluster] != -1)
                                {
                                    freqMax = cpu_frequency_max[cluster];
                                }
                                needSetMax = MTRUE;
                            }

                            MY_LOGD("Manually PerfService Control by Stereo Feature - cluster:%d, freqMin:%d, needSetMin:%d, freqMax:%d needSetMax:%d, coreMax:%d ",
                                                    cluster,
                                                    freqMin,
                                                    needSetMin,
                                                    freqMax,
                                                    needSetMax,
                                                    coreMax);
                            pPerf->scnConfig(handle, MtkPowerCmd::CMD_SET_CLUSTER_CPU_CORE_MIN, cluster, coreMax, 0, 0);
                            pPerf->scnConfig(handle, MtkPowerCmd::CMD_SET_CLUSTER_CPU_CORE_MAX, cluster, coreMax, 0, 0);
                            if(needSetMin)
                                pPerf->scnConfig(handle, MtkPowerCmd::CMD_SET_CLUSTER_CPU_FREQ_MIN, cluster, freqMin, 0, 0);
                            if(needSetMax)
                                pPerf->scnConfig(handle, MtkPowerCmd::CMD_SET_CLUSTER_CPU_FREQ_MAX, cluster, freqMax, 0, 0);
                        }
                    }
                    else
                    {
                        pPerf->scnConfig(handle, MtkPowerCmd::CMD_SET_CLUSTER_CPU_CORE_MIN, 0, 4, 0, 0); // min 4xLL
                    }
                    //control Dram clk
                    if(param.enableDramClkControl)
                    {
                        MY_LOGD("manual control dram clk (%d)", param.dramOPPLevel);
                        pPerf->scnConfig(handle, MtkPowerCmd::CMD_SET_OPP_DDR, param.dramOPPLevel, 0, 0, 0);
                    }
#else
                    pPerf->scnConfig(handle, MtkPowerCmd::CMD_SET_CLUSTER_CPU_CORE_MIN, 0, 4, 0, 0); // min 4xLL
#endif
                    pPerf->scnEnable(handle, 0);
                    MY_LOGD("perfService enable Scenario_ZsdPreview");
                    break;
                }
                default:{
                    // Do nothing
                    break;
                }
            }
            if( useCpuCtrl )
            {
                size_t i = 0;
                for( i = 0 ; i < cpuCtrlParam.size() ; i++ )
                {
                    MY_LOGD("Cpu Ctrl %d/%zu, cmd:%d, cluster:%d, arg:(%d,%d,%d)",
                            i, cpuCtrlParam.size(),
                            cpuCtrlParam[i].cmd,
                            cpuCtrlParam[i].cluster,
                            cpuCtrlParam[i].arg1,
                            cpuCtrlParam[i].arg2,
                            cpuCtrlParam[i].arg3);
                    pPerf->scnConfig(handle,
                                            (MtkPowerCmd)cpuCtrlParam[i].cmd,
                                            cpuCtrlParam[i].cluster,
                                            cpuCtrlParam[i].arg1,
                                            cpuCtrlParam[i].arg2,
                                            cpuCtrlParam[i].arg3);
                }
                pPerf->scnEnable(handle, 0);
            }
            //
            mCurPerfHandle = handle;
        }
        else
        {
            MY_LOGW("cannot regscn eng(%d)", mEngMode);
        }
    }
    //
    MY_LOGD("-");
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
ScenarioControlV3::
exitPerfService()
{
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
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
ScenarioControlV3::
changeBWCProfile(ControlParam const & param)
{
        //
    BWC_PROFILE_TYPE type = mapToBWCProfileV3(param.scenario);
    if( type == BWCPT_NONE )
        return BAD_VALUE;
    //
    if(param.enableBWCControl){
        BWC BwcIns;
        BwcIns.Profile_Change(type,true);
        //
        MUINT32 multiple = 1;
        if (FEATURE_CFG_IS_ENABLED(param.featureFlag, FEATURE_DUAL_PD)) {
            multiple = 2;
        }
        //
        MY_LOGD("mmdvfs_set type(%d) multiple(%d) sensorSize(%d) finalSize(%d) fps(%d) isMultiOpen(%d)",
            type, multiple, param.sensorSize.size(), param.sensorSize.size()*multiple, param.sensorFps, isMultiOpen());
        //
        mmdvfs_set(
                type,
                MMDVFS_SENSOR_SIZE,             param.sensorSize.size() * multiple,//param.sensorSize.size(),
                MMDVFS_SENSOR_FPS,              param.sensorFps,
                MMDVFS_PREVIEW_SIZE,            param.videoSize.w * param.videoSize.h,
                MMDVFS_CAMERA_MODE_PIP,         isMultiOpen() && !isDualZoomMode(param.featureFlag),
                MMDVFS_CAMERA_MODE_DUAL_ZOOM,   isDualZoomMode(param.featureFlag),
                MMDVFS_CAMERA_MODE_VFB,         FEATURE_CFG_IS_ENABLED(param.featureFlag, FEATURE_VFB),
                MMDVFS_CAMERA_MODE_EIS_2_0,     FEATURE_CFG_IS_ENABLED(param.featureFlag, FEATURE_ADV_EIS),
                MMDVFS_CAMERA_MODE_IVHDR,       FEATURE_CFG_IS_ENABLED(param.featureFlag, FEATURE_IVHDR),
                MMDVFS_CAMERA_MODE_MVHDR,       FEATURE_CFG_IS_ENABLED(param.featureFlag, FEATURE_MVHDR),
                MMDVFS_CAMERA_MODE_ZVHDR,       FEATURE_CFG_IS_ENABLED(param.featureFlag, FEATURE_ZVHDR),
                MMDVFS_CAMERA_MODE_STEREO,      isInStereoMode(param.featureFlag),
                MMDVFS_PARAMETER_EOF);
    }
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
ScenarioControlV3::
enterScenario(
    MINT32 const scenario
)
{
    ControlParam param;
    param.scenario = scenario;
    param.sensorSize = mCurParam.sensorSize;
    param.sensorFps = mCurParam.sensorFps;
    param.featureFlag = mCurParam.featureFlag;
    param.videoSize = mCurParam.videoSize;
    param.camMode = mCurParam.camMode;
#ifdef ENABLE_STEREO_PERFSERVICE
    param.enableDramClkControl = mCurParam.enableDramClkControl;
    param.dramOPPLevel = mCurParam.dramOPPLevel;
#endif

    return enterScenario(param);
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
ScenarioControlV3::
enterScenario(
    ControlParam const & param
)
{
    DUMP_SCENARIO_PARAM(mOpenId, "enter:", param);
    //
    //exit previous perfservice setting
    exitPerfService();
    //enter new perfservice setting
    enterPerfService(param);
    //
    changeBWCProfile(param);

    if( mCurParam.scenario != Scenario_None && mCurParam.scenario != param.scenario ) {
        MY_LOGD("exit previous scenario setting");
        exitScenario();
    }
    // keep param
    mCurParam = param;
    //
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
ScenarioControlV3::
getCurrentStatus(int &curScen, int &curBoost, int64_t &lastFrameNo, int &featureFlag)
{
    curScen = mCurParam.scenario;
    curBoost = mCurBoostMode;
    lastFrameNo = mLastBoostFrameNo;
    featureFlag = mCurParam.featureFlag;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
ScenarioControlV3::
exitScenario()
{
    if( mCurParam.scenario == Scenario_None ) {
        MY_LOGD("already exit");
        return OK;
    }
    DUMP_SCENARIO_PARAM(mOpenId, "exit:", mCurParam);
    BWC_PROFILE_TYPE type = mapToBWCProfileV3(mCurParam.scenario);
    if( type == BWCPT_NONE )
        return BAD_VALUE;
    //
    if(mCurParam.enableBWCControl){
        BWC BwcIns;
        BwcIns.Profile_Change(type,false);
    }
    checkIfNeedExitBoost(0, true);
    // reset param
    mCurParam.scenario = Scenario_None;
    //
    for (int i = 0; i < mvpParams.size(); i++)
    {
        MMDVFS_QOS_PARAMETER clear = {};
        clear.camera_id = mvpParams[i]->camera_id;
        mmdvfs_qos_set(1, &clear);
    }
    mvpParams.clear();
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
ScenarioControlV3::
setQOSParams(std::vector<SensorParam> const &vParams)
{
    if (mvpParams.size() != 0)
    {
        MY_LOGW("QOS setting is not zero");
        mvpParams.clear();
    }
    for (int i = 0; i < vParams.size(); i++)
    {
        std::shared_ptr<NSCamHW::HwInfoHelper> infohelper = std::make_shared<NSCamHW::HwInfoHelper>(vParams[i].id);
        auto setting = std::make_shared<MMDVFS_QOS_PARAMETER>();
        MUINT32 VBTime = 0;
        MUINT32 PDSize = 0;
        double  VBRatio = 0.0;
        if (infohelper != nullptr)
        {
            infohelper->getSensorVBTime(vParams[i].sensorMode, VBTime);
            infohelper->getSensorPDSize(vParams[i].sensorMode, PDSize);
            MY_LOGD("sensor mode : %d", vParams[i].sensorMode);
            MY_LOGD("fps : %d, VBTime : %d, PDSize : %d", vParams[i].fps, VBTime, PDSize);
            VBRatio = (double)VBTime / (1000000.0/vParams[i].fps);
            MY_LOGD("VBRatio : %f", VBRatio);
        }
        setting->camera_id = vParams[i].id;
        setting->sensor_fps = vParams[i].fps;
        setting->sensor_size = vParams[i].sensorSize;
        setting->pd_size = PDSize;
        setting->video_size = vParams[i].videoSize;
        setting->vb_ratio = VBRatio;
        setting->vb_time = (double)VBTime / 1000.0;
        setting->rawi_enable = vParams[i].P1RawIn;
        setting->yuv_enable = vParams[i].P1YuvOut;
        setting->raw_bbp = vParams[i].raw_bbp;
        mvpParams.push_back(setting);
    }
    if (mvpParams.size() == 1)
    {
        mmdvfs_qos_set(1, mvpParams[0].get());
    }
    else if (mvpParams.size() == 2)
    {
        mmdvfs_qos_set(2, mvpParams[0].get(), mvpParams[1].get());
    }
    else
    {
        for (int i = 0; i < mvpParams.size(); i++)
        {
            mmdvfs_qos_set(1, mvpParams[i].get());
        }
    }
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
ScenarioControlV3::
boostScenario(
    int const scenario,
    int const feature,
    int64_t const frameNo
)
{
    if ( scenario == Scenario_None || scenario == mCurParam.scenario)
    {
        return OK;
    }
    if ( SCENARIO_IS_ENABLED(mCurBoostMode, scenario) )
    {
        Mutex::Autolock _l(mLock);
        mLastBoostFrameNo = frameNo;
        return OK;
    }
    MY_LOGD("boost scenario : 0x%X, feature : 0x%X, frameNo : " PRId64 , scenario, feature, frameNo);
    ControlParam param = mCurParam;
    param.scenario = scenario;
    param.featureFlag = feature;
    //
    Mutex::Autolock _l(mLock);
    changeBWCProfile(param);

    SCENARIO_BOOST_MASK(mCurBoostMode, scenario);
    mLastBoostFrameNo = frameNo;
    //
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
ScenarioControlV3::
checkIfNeedExitBoost(
    int64_t const frameNo,
    int const forceExit
)
{
    if ( !forceExit && (mLastBoostFrameNo == -1 || mLastBoostFrameNo > frameNo || mCurBoostMode == 0) )
    {
        return OK;
    }
    Mutex::Autolock _l(mLock);
    if ( !forceExit && mLastBoostFrameNo != frameNo )
    {
        if ( (frameNo - mLastBoostFrameNo) < 30 )
        {
            MY_LOGD("last boost frameNo : %lld != current frameNo : %lld, delay 30 frame to leave boost", mLastBoostFrameNo, frameNo);
            return OK;
        }
    }
    for (int i = 0; i < Scenario_MaxScenarioNumber; i++)
    {
        if ( SCENARIO_IS_ENABLED(mCurBoostMode, i) )
        {
            BWC_PROFILE_TYPE type = mapToBWCProfileV3(i);
            if( type == BWCPT_NONE )
            {
                MY_LOGW("cannot find BWC : %X", i);
                continue;
            }
            BWC BwcIns;
            BwcIns.Profile_Change(type,false);
        }
    }
    MY_LOGD("leave boost scenario, last frameNo : " PRId64 ", current frameNo : " PRId64 , mLastBoostFrameNo, frameNo);
    mCurBoostMode = 0;
    mLastBoostFrameNo = -1;
    return OK;
}

