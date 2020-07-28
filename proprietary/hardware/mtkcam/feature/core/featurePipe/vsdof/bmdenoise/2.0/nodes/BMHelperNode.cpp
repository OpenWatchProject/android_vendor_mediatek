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
#include "BMHelperNode.h"

#define PIPE_MODULE_TAG "BMDeNoise"
#define PIPE_CLASS_TAG "BMHelperNode"
#define PIPE_LOG_TAG PIPE_MODULE_TAG PIPE_CLASS_TAG

#define MAGIC_ID_SCENARIO_CTRL 918

#include <PipeLog.h>

using namespace NSCam::NSCamFeature::NSFeaturePipe;
using namespace NSIspTuning;
/*******************************************************************************
 *
 ********************************************************************************/
BMHelperNode::
BMHelperNode(const char *name,
    Graph_T *graph,
    MINT32 openId)
    : BMDeNoisePipeNode(name, graph)
    , miOpenId(openId)
{
    MY_LOGD("ctor(%p) %s", this, getName());
    this->addWaitQueue(&mRequests);
}
/*******************************************************************************
 *
 ********************************************************************************/
BMHelperNode::
~BMHelperNode()
{
    MY_LOGD("dctor(0x%x)", this);
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BMHelperNode::
onData(
    DataID id,
    PipeRequestPtr &request)
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    MBOOL ret = MFALSE;
    switch(id)
    {
        case SWNR_TO_HELPER:
            mRequests.enque(request);
            ret = MTRUE;
            break;
        default:
            ret = MFALSE;
            MY_LOGE("unknown data id :%d", id);
            break;
    }
    FUNC_END;
    return ret;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BMHelperNode::
onData(
    DataID id,
    MINT32 &data)
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    MBOOL ret = MFALSE;
    switch(id)
    {
        case ENTER_HIGH_PERF:
            enterHighPerMode(data);
            ret = MTRUE;
            break;
        case EXIT_HIGH_PERF:
            exitHighPerMode(data);
            ret = MTRUE;
            break;
        default:
            ret = MFALSE;
            MY_LOGE("unknown data id :%d", id);
            break;
    }

    return ret;
    // MY_LOGD("dont do scenarioControl, return true");
    // return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BMHelperNode::
onInit()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    if(!BMDeNoisePipeNode::onInit()){
        MY_LOGE("BMDeNoisePipeNode::onInit() failed!");
        return MFALSE;
    }
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BMHelperNode::
onUninit()
{
    FUNC_START;
    FUNC_END;
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
BMHelperNode::
cleanUp()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    Mutex::Autolock _l(mLock);

    if(!msHighPerfUser.empty()){
        MY_LOGW("exit when there is still highPerfUser?");
        for(auto e:msHighPerfUser){
            MY_LOGD("left highPerf reqs:%d", e);
        }
    }
    mpScenarioCtrl->exitScenario();
    mpScenarioCtrl = nullptr;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BMHelperNode::
onThreadStart()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    MY_LOGD("create scenarioControl with id:%d", MAGIC_ID_SCENARIO_CTRL);
    mpScenarioCtrl = IScenarioControl::create(MAGIC_ID_SCENARIO_CTRL);
    if( mpScenarioCtrl == NULL )
    {
        MY_LOGE("get Scenario Control fail");
        return UNKNOWN_ERROR;
    }
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BMHelperNode::
onThreadStop()
{
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BMHelperNode::
onThreadLoop()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    PipeRequestPtr pipeRequest = nullptr;

    if( !waitAllQueue() )// block until queue ready, or flush() breaks the blocking state too.
    {
        return MFALSE;
    }
    if( !mRequests.deque(pipeRequest) )
    {
        MY_LOGD("mRequests.deque() failed");
        return MFALSE;
    }

    this->incExtThreadDependency();

    if(shouldDumpRequest(pipeRequest)){
        doPorterJob(pipeRequest);
    }

    handleData(DENOISE_FINAL_RESULT_OUT, pipeRequest);

    handleData(P2_DONE,                  pipeRequest);

    this->decExtThreadDependency();
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
BMHelperNode::
copyISPProfiles(char* targetPath)
{
    char filepathISP[1024];
    snprintf(filepathISP, 1024, "%s/debug", targetPath);
    MY_LOGD("makePath: %s", filepathISP);
    makePath(filepathISP, 0777);

    IHalSensorList* sensorList = MAKE_HalSensorList();
    int sensorDev_Main1 = sensorList->querySensorDevIdx(mSensorIdx_Main1);
    int sensorDev_Main2 = sensorList->querySensorDevIdx(mSensorIdx_Main2);
    char cmd[1024];

    #define MAKE_CMD(profile_name) \
        snprintf(cmd, 1024, "mkdir %s/debug/%s_sensor_%d", targetPath, #profile_name, sensorDev_Main1);\
        systemCall(cmd);\
        snprintf(cmd, 1024, "cp /sdcard/debug/*-%d-*-%d.bin %s/debug/%s_sensor_%d", sensorDev_Main1, profile_name, targetPath, #profile_name, sensorDev_Main1);\
        systemCall(cmd);\
        snprintf(cmd, 1024, "mkdir %s/debug/%s_sensor_%d", targetPath, #profile_name, sensorDev_Main2);\
        systemCall(cmd);\
        snprintf(cmd, 1024, "cp /sdcard/debug/*-%d-*-%d.bin %s/debug/%s_sensor_%d", sensorDev_Main2, profile_name, targetPath, #profile_name, sensorDev_Main2);\
        systemCall(cmd);

    // BMDN
    MAKE_CMD(EIspProfile_N3D_Denoise);
    MAKE_CMD(EIspProfile_N3D_Denoise_toW);
    MAKE_CMD(EIspProfile_N3D_Denoise_toYUV);
    MAKE_CMD(EIspProfile_N3D_Denoise_toGGM);

    // MFHR
    MAKE_CMD(EIspProfile_N3D_HighRes);
    MAKE_CMD(EIspProfile_N3D_HighRes_toYUV);

    // common
    MAKE_CMD(EIspProfile_N3D_Capture);
    MAKE_CMD(EIspProfile_N3D_Capture_toW);
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
BMHelperNode::
doPorterJob(PipeRequestPtr request)
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    MINT32 iReqIdx = request->getRequestNo();

    // get unique ID
    uint32_t exif_TS = request->getParam(PID_EXIF_UNIQUE_ID);

    char targetPath[1024];
    snprintf(targetPath, 1024, "/sdcard/bmdenoise/%d_%d", iReqIdx, exif_TS);
    // make path
    MY_LOGD("makePath: %s", targetPath);
    makePath(targetPath, 0777);

    char cmd[1024];
    // copy ALG dump
    snprintf(cmd, 1024, "cp -R /sdcard/bmdenoise/CModelData/ %s/", targetPath);
    systemCall(cmd);

    // copy img dump
    snprintf(cmd, 1024, "cp -R /sdcard/bmdenoise/%d/ %s/", iReqIdx, targetPath);
    systemCall(cmd);

    // copy isp profiles
    copyISPProfiles(targetPath);

    snprintf(cmd, 1024, "rm -rf /sdcard/bmdenoise/%d", iReqIdx);
    systemCall(cmd);

    snprintf(cmd, 1024, "rm -rf /sdcard/bmdenoise/CModelData");
    systemCall(cmd);

    snprintf(cmd, 1024, "rm -rf /sdcard/debug");
    systemCall(cmd);

    snprintf(cmd, 1024, "mkdir /sdcard/debug");
    systemCall(cmd);
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
BMHelperNode::
enterHighPerMode(MINT32 reqNo)
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    Mutex::Autolock _l(mLock);
    MY_LOGD("new highPerf req:%d", reqNo);

    msHighPerfUser.insert(reqNo);

    if(msHighPerfUser.size() == 1){
        MY_LOGD("scenario control by BMHelperNode: enter capture");
        StereoArea area = StereoSizeProvider::getInstance()->getBufferSize(E_BM_PREPROCESS_W_1);

        MUINT featureFlagStereo = 0;
        FEATURE_CFG_ENABLE_MASK(featureFlagStereo, IScenarioControl::FEATURE_BMDENOISE_CAPTURE);

        IScenarioControl::ControlParam controlParam;
        controlParam.scenario = IScenarioControl::Scenario_ZsdPreview;
        controlParam.sensorSize = area.size;
        controlParam.sensorFps = 30;
        controlParam.featureFlag = featureFlagStereo;
        controlParam.enableBWCControl = MFALSE;

        mpScenarioCtrl->enterScenario(controlParam);
    }else{
        MY_LOGD("already in highPerf mode, ignored");
    }

    string msg("current highPerf reqs:");
    for(auto e:msHighPerfUser){
        msg = msg + to_string(e) +", ";
    }
    MY_LOGD("%s", msg.c_str());
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
BMHelperNode::
exitHighPerMode(MINT32 reqNo)
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    Mutex::Autolock _l(mLock);
    MY_LOGD("exit highPerf req:%d", reqNo);

    msHighPerfUser.erase(reqNo);

    string msg("left highPerf reqs:");
    for(auto e:msHighPerfUser){
        msg = msg + to_string(e) +", ";
    }
    MY_LOGD("%s", msg.c_str());

    if(msHighPerfUser.empty()){
        MY_LOGD("scenario control by BMHelperNode: exit capture");
        mpScenarioCtrl->exitScenario();
    }else{
        // do nothing
    }
}