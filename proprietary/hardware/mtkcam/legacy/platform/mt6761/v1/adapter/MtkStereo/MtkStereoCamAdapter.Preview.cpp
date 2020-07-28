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

#define LOG_TAG "MtkCam/CamAdapter"
//
#include <inc/CamUtils.h>
using namespace android;
using namespace MtkCamUtils;
//
#include <camera_custom_zsd.h>
#include <camera_custom_stereo.h>
#include <mtkcam/utils/fwk/MtkCamera.h>
//
#include <inc/ImgBufProvidersManager.h>
//
#include <mtkcam/v1/IParamsManager.h>
#include <mtkcam/v1/ICamAdapter.h>
#include <inc/BaseCamAdapter.h>
#include <inc/MtkStereoCamAdapter.h>
using namespace NSMtkStereoCamAdapter;
//
#include <mtkcam/drv/IHalSensor.h>
using namespace NSCam;
//
#include <camera_custom_vt.h>
//
#include <cutils/properties.h>
//
#include <sys/prctl.h>
//
#include <mtkcam/hwutils/CameraProfile.h>
using namespace CPTool;
//
#include <hwsync_drv.h>
//
/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, arg...)    if (cond) { MY_LOGV(arg); }
#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }

/******************************************************************************
*
*******************************************************************************/
bool
CamAdapter::
previewEnabled() const
{
    return (    mpStateManager->isState(IState::eState_Preview)||
                recordingEnabled());
}


/******************************************************************************
*
*******************************************************************************/
status_t
CamAdapter::
startPreview()
{
    return  mpStateManager->getCurrentState()->onStartPreview(this);
}


/******************************************************************************
*
*******************************************************************************/
void
CamAdapter::
stopPreview()
{
    mpStateManager->getCurrentState()->onStopPreview(this);
}


/******************************************************************************
*   CamAdapter::startPreview() -> IState::onStartPreview() ->
*   IStateHandler::onHandleStartPreview() -> CamAdapter::onHandleStartPreview()
*******************************************************************************/
status_t
CamAdapter::
onHandleStartPreview()
{
    MY_LOGD("+");
    //
    status_t ret = INVALID_OPERATION;
    //
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.featurepipe.enable", value, "1");
    bool feature_enable = atoi(value);
    bool const recordHint = mpParamsMgr->getRecordingHint();
    StereoNodeType const stereoNodeType = (feature_enable) ? STEREO_PASS2_FEATURE_PRV : STEREO_PASS2_PREVIEW;
    StereoCtrlType const stereoCtrlType = (recordHint) ? STEREO_CTRL_RECORD : STEREO_CTRL_PREVIEW;
    //
    MINT32 stereoPrvFps = mpParamsMgr->getInt(MtkCameraParameters::KEY_STEREO_PREVIEW_FRAME_RATE);
    MINT32 stereoCapFps = mpParamsMgr->getInt(MtkCameraParameters::KEY_STEREO_CAPTURE_FRAME_RATE);
    //
    MBOOL bIsForceRatation = MFALSE;
    MINT32 rawType = 1, vdoWidth = 0, vdoHeight = 0, prvWidth = 0, prvHeight = 0;
    MINT32 targetWidth = 0, targetHeight = 0;
    MUINT32 pass1RingBufNum = 3, sensorScenario = 0, sensorMain2Scenario = 0, rotationAnagle = 270;
    MUINT32 pass1RingBufNum_main2 = 3;
    MUINT32 main1_sensor_bits = 10, main2_sensor_bits = 10;
    pass1RingBufNum = ::property_get_int32("debug.camera.stereo.p1rm1", 3);
    pass1RingBufNum_main2 = ::property_get_int32("debug.camera.stereo.p1rm2", 3);

    DefaultCtrlNode::MODE_TYPE modeType = DefaultCtrlNode::MODE_IDLE;
    DefaultCtrlNode::CONFIG_INFO configInfo, cfgInfo_Main2;
    ResMgrDrv::MODE_STRUCT mode;
    mpParamsMgr->getPreviewSize(&prvWidth, &prvHeight);
    mpParamsMgr->getVideoSize(&vdoWidth, &vdoHeight);

    // Shane 20150120
    // 4:3     prvWidth*prvHeight = 1440x1080 (From APP settings)
    // 16:9    prvWidth*prvHeight = 1920x1080 (From APP settings)
    MY_LOGD("prvSize %dx%d", prvWidth, prvHeight);
    if(prvWidth*3 == prvHeight*4){
        MY_LOGD("prvSize ratio is 4:3");
        targetWidth = 1920;
        targetHeight = 1440;
    }else if(prvWidth*9 == prvHeight*16){
        MY_LOGD("prvSize ratio is 16:9");
        targetWidth = StereoSettingProvider::getAlgoImageSize(0).w;
        targetHeight = StereoSettingProvider::getAlgoImageSize(0).h;
    }else{
        MY_LOGE("prvSize ratio is not supported. use 16:9 instead");
        targetWidth = StereoSettingProvider::getAlgoImageSize(0).w;
        targetHeight = StereoSettingProvider::getAlgoImageSize(0).h;
    }

    MY_LOGD("Preview TARGET:%dx%d, PRV:%dx%d = %d, VDO:%dx%d = %d, 1080P(%d)",
            targetWidth, targetHeight,
            prvWidth, prvHeight, prvWidth*prvHeight,
            vdoWidth, vdoHeight, vdoWidth*vdoHeight,
            IMG_1080P_SIZE);
    mbZsdMode = ( ::strcmp(mpParamsMgr->getStr(MtkCameraParameters::KEY_ZSD_MODE), MtkCameraParameters::ON) == 0 ) ? MTRUE : MFALSE;
    mFrameRate = ( mbZsdMode ) ? stereoCapFps : stereoPrvFps;
    //
    mLastVdoWidth = 0;
    mLastVdoHeight = 0;
    //
    configInfo.replaceBufNumResize      = 0;
    configInfo.replaceBufNumFullsize    = 0;
    configInfo.rollbackBufNum           = 0;
    configInfo.rollbackBufNumPrv        = 0;
    configInfo.rollbackBufPeriod        = 1;
    cfgInfo_Main2.replaceBufNumResize   = 0;
    cfgInfo_Main2.replaceBufNumFullsize = 0;
    cfgInfo_Main2.rollbackBufNum        = 0;
    cfgInfo_Main2.rollbackBufNumPrv     = 0;
    cfgInfo_Main2.rollbackBufPeriod     = 1;
    //
    IHalSensorList* const pHalSensorList = IHalSensorList::get();
    IHalSensor* pSensorHalObj = NULL;
    if( pHalSensorList != NULL )
    {
         pSensorHalObj = pHalSensorList->createSensor(LOG_TAG, getOpenId());
    }
    else
    {
        MY_LOGE("pHalSensorList == NULL");
    }
    //
    if(mpResMgrDrv)
    {
        mpResMgrDrv->getMode(&mode);
    }
    //
    if( mbZsdMode )
    {
        sensorScenario      = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
        sensorMain2Scenario = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
        //
        MY_LOGD("main2_sensorScenario = %d, get_stereo_zsd_cap_stored_frame_cnt:%d",
                sensorMain2Scenario, get_stereo_zsd_cap_stored_frame_cnt());
        configInfo.rollbackBufNum       = get_stereo_zsd_cap_stored_frame_cnt();
        configInfo.rollbackBufNumPrv    = get_stereo_zsd_cap_stored_frame_cnt();
        cfgInfo_Main2.rollbackBufNum    = get_stereo_zsd_cap_stored_frame_cnt();
        cfgInfo_Main2.rollbackBufNumPrv = get_stereo_zsd_cap_stored_frame_cnt();
        configInfo.rollbackBufPeriod    = get_zsd_cap_stored_update_period();
        cfgInfo_Main2.rollbackBufPeriod = get_zsd_cap_stored_update_period();

        modeType = DefaultCtrlNode::MODE_STEREO_ZSD_PREVIEW;
        mode.scenSw = ResMgrDrv::SCEN_SW_ZSD;
        if(mpResMgrDrv!=NULL)
        {
            MY_LOGD("Switch MMDVFS to High");
            mpResMgrDrv->setItem(ResMgrDrv::ITEM_STEREO, 1);
            mpHwSyncDrv-> sendCommand (HW_SYNC_CMD_UNDATE_CLOCK, 0x0, 0x0, 0x0, HW_SYNC_CLOCK_STAGE_1);

        }
        mUserName = "SZSD";
    }
    else
    {
        sensorScenario      = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
        sensorMain2Scenario = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
        //
        configInfo.rollbackBufNum       = 0;
        configInfo.rollbackBufNumPrv    = 0;
        cfgInfo_Main2.rollbackBufNum    = 0;
        cfgInfo_Main2.rollbackBufNumPrv = 0;
        modeType = (recordHint) ? DefaultCtrlNode::MODE_STEREO_VDO_PREVIEW : DefaultCtrlNode::MODE_STEREO_PREVIEW;
        mode.scenSw = ResMgrDrv::SCEN_SW_CAM_PRV;
        if(mpResMgrDrv!=NULL)
        {
            MY_LOGD("Switch MMDVFS to LOW");
            mpResMgrDrv->setItem(ResMgrDrv::ITEM_STEREO, 0);
            mpHwSyncDrv-> sendCommand (HW_SYNC_CMD_UNDATE_CLOCK, 0x0, 0x0, 0x0, HW_SYNC_CLOCK_STAGE_0);

        }
        //
        mUserName = "STEREO";
    }
    //
    if(mFrameRate > 30)
    {
        configInfo.replaceBufNumResize      += ((mFrameRate/30)*2+1);
        cfgInfo_Main2.replaceBufNumResize   += ((mFrameRate/30)*2+1);
    }
    // get sensor bits
    StereoSettingProvider::getStereoCameraSensorBit(main1_sensor_bits, main2_sensor_bits);
    //
    MY_LOGD("VHdr(%d),Name(%s),userName(%s),sensorScenario(%d),pass1RingBufNum(%d),pass1RingBufNum_main2(%d),stereo:prvFps(%d),capFps(%d)->frameRate(%d), main1:%d bits, main2:%d bits",
            mpParamsMgr->getVHdr(),
            mName.string(),
            mUserName.string(),
            sensorScenario,
            pass1RingBufNum,
            pass1RingBufNum_main2,
            stereoPrvFps,
            stereoCapFps,
            mFrameRate,
            main1_sensor_bits,
            main2_sensor_bits);
    //
    if(mpResMgrDrv)
    {
        mpResMgrDrv->setMode(&mode);
    }
    //
    Pass1NodeInitCfg p1NodeInitCfg(
                        sensorScenario,
                        pass1RingBufNum,
                        main1_sensor_bits,
                        mFrameRate,
                        rawType);
    Pass1NodeInitCfg p1NodeInitCfg_Main2(
                        sensorMain2Scenario,
                        pass1RingBufNum_main2,
                        main2_sensor_bits,
                        mFrameRate,
                        rawType);
    SCNodeInitCfg SCNodeInitCfg(
                        stereoCtrlType,
                        targetWidth, targetHeight,  //algo size
                        targetWidth, targetHeight,  //main size
                        0,
                        sensorScenario,
                        sensorMain2Scenario);
    SNodeInitCfg SNodeInitCfg(
                        stereoNodeType,
                        targetWidth, targetHeight); // algo target size
    //
    if  ( ! BaseCamAdapter::init() )
    {
        goto lbExit;
    }
    //
    mpCamGraph          = ICamGraph::createInstance(
                                        3,    // TODO: Main+Main2
                                        mUserName.string());
    //
    mpPass1Node         = Pass1Node::createInstance(p1NodeInitCfg);
    mpPass1Node_Main2   = Pass1Node::createInstance(p1NodeInitCfg_Main2);
    mpPass1Node->setSensorIdx( getOpenId_Main() );
    mpPass1Node_Main2->setSensorIdx( getOpenId_Main2() );
    mpStereoCtrlNode    = StereoCtrlNode::createInstance(SCNodeInitCfg);
    mpStereoNode        = StereoNode::createInstance(SNodeInitCfg);
    mpStereoNode_Main2  = StereoNode::createInstance(SNodeInitCfg);
    mpStereoNode->setSensorIdx( getOpenId_Main() );
    mpStereoNode->setParameters( mpParamsMgr, getOpenId_Main() );
    mpStereoNode_Main2->setSensorIdx( getOpenId_Main2() );
    mpStereoNode_Main2->setParameters( mpParamsMgr, getOpenId_Main2() );
    mpStereoCtrlNode->setSensorIdx( getOpenId_Main() );
    //
    if(mpCapBufMgr)
    {
       mpCapBufMgr->setMaxDequeNum(configInfo.rollbackBufNum);
    }
    if(mpCapBufMgr_Main2)
    {
       mpCapBufMgr_Main2->setMaxDequeNum(cfgInfo_Main2.rollbackBufNum);
    }
    //
    configInfo.spParamsMgr              = mpParamsMgr;
    configInfo.spCamMsgCbInfo           = mpCamMsgCbInfo;
    configInfo.pCapBufMgr               = mpCapBufMgr;
    configInfo.replaceBufNumResize      = configInfo.rollbackBufNumPrv;
    configInfo.replaceBufNumFullsize    = configInfo.rollbackBufNum;
    configInfo.previewMaxFps            = mFrameRate;
    cfgInfo_Main2.spParamsMgr           = mpParamsMgr;
    cfgInfo_Main2.spCamMsgCbInfo        = mpCamMsgCbInfo;
    cfgInfo_Main2.pCapBufMgr            = mpCapBufMgr_Main2;
    cfgInfo_Main2.replaceBufNumResize   = cfgInfo_Main2.rollbackBufNumPrv;
    cfgInfo_Main2.replaceBufNumFullsize = cfgInfo_Main2.rollbackBufNum;
    cfgInfo_Main2.previewMaxFps         = mFrameRate;
    if(mpDefaultCtrlNode)
    {
        mpDefaultCtrlNode->setSensorIdx( getOpenId_Main() );
        mpDefaultCtrlNode->setConfig(configInfo);
        mpDefaultCtrlNode->setParameters();
        mpDefaultCtrlNode->setMode(modeType);
    }
    if(mpDefaultCtrlNode_Main2)
    {
        mpDefaultCtrlNode_Main2->setSensorIdx( getOpenId_Main2() );
        mpDefaultCtrlNode_Main2->setConfig(cfgInfo_Main2);
        mpDefaultCtrlNode_Main2->setParameters();
        mpDefaultCtrlNode_Main2->setMode(modeType);
    }
    if(mpImgTransformNode)
    {
        mpImgTransformNode->setSensorIdx( getOpenId_Main() );
    }
    //
    if(bIsForceRatation)
    {
        mpDefaultBufHdl->setForceRotation( MTRUE, rotationAnagle);
        mpDefaultCtrlNode->setForceRotation( MTRUE, rotationAnagle);
        mpDefaultCtrlNode_Main2->setForceRotation( MTRUE, rotationAnagle);
    }

    //
    MY_LOGD("rawType(%d)",rawType);
    //
    MY_LOGD("AllocBufHandler:init");
    mpAllocBufHdl->init();
    mpAllocBufHdl_Main2->init();
    //
    MY_LOGD("DefaultBufHandler:map port");
    mpDefaultBufHdl->mapPort(DefaultBufHandler::eBuf_Disp,      TRANSFORM_DST_0, 0, MTRUE);
    mpDefaultBufHdl->mapPort(DefaultBufHandler::eBuf_AP,        TRANSFORM_DST_1);
    mpDefaultBufHdl->mapPort(DefaultBufHandler::eBuf_Generic,   TRANSFORM_DST_1);
    mpDefaultBufHdl->mapPort(DefaultBufHandler::eBuf_FD,        STEREO_DST);
    mpDefaultBufHdl->mapPort(DefaultBufHandler::eBuf_OT,        STEREO_DST);
    //
    MY_LOGD("CamGraph:setBufferHandler");
    mpCamGraph->setBufferHandler(   PASS1_RESIZEDRAW,   mpAllocBufHdl,      getOpenId_Main());
    mpCamGraph->setBufferHandler(   PASS1_FULLRAW,      mpAllocBufHdl,      getOpenId_Main());
    mpCamGraph->setBufferHandler(   STEREO_IMG,         mpAllocBufHdl,      getOpenId_Main());
    mpCamGraph->setBufferHandler(   TRANSFORM_DST_0,    mpDefaultBufHdl,    getOpenId_Main());
    mpCamGraph->setBufferHandler(   TRANSFORM_DST_1,    mpDefaultBufHdl,    getOpenId_Main());
    mpCamGraph->setBufferHandler(   STEREO_DST,         mpDefaultBufHdl,    getOpenId_Main());
    // set buffer handle for stereo ctrl node output
    mpCamGraph->setBufferHandler(   STEREO_CTRL_DST_M,  mpAllocBufHdl,    getOpenId_Main());

    //
    mpCamGraph->setBufferHandler(   PASS1_RESIZEDRAW,   mpAllocBufHdl_Main2,    getOpenId_Main2());
    mpCamGraph->setBufferHandler(   PASS1_FULLRAW,      mpAllocBufHdl_Main2,    getOpenId_Main2());
    mpCamGraph->setBufferHandler(   STEREO_IMG,         mpAllocBufHdl_Main2,    getOpenId_Main2());
    //
    MY_LOGD("CamGraph:connectData");
    // [ P1Node -- DCNode ] : connect main2 first, because need to use main1 to hold EIS hahdle (init flow will be main1 and then main2)
    mpCamGraph->connectData(    PASS1_RESIZEDRAW,   CONTROL_RESIZEDRAW, mpPass1Node_Main2,          mpDefaultCtrlNode_Main2);
    mpCamGraph->connectData(    PASS1_RESIZEDRAW,   CONTROL_RESIZEDRAW, mpPass1Node,                mpDefaultCtrlNode);
    mpCamGraph->connectData(    PASS1_FULLRAW,      CONTROL_FULLRAW,    mpPass1Node_Main2,          mpDefaultCtrlNode_Main2);
    if ( mbZsdMode )
    {
        mpSyncNode->setZSDMode(true, mpCapBufMgr, mpCapBufMgr_Main2, get_stereo_zsd_cap_stored_frame_cnt());
        mpCamGraph->connectData(    PASS1_FULLRAW,      CONTROL_FULLRAW,    mpPass1Node,                mpDefaultCtrlNode);
    }else{
        mpSyncNode->setZSDMode(false, NULL, NULL, 0);
    }
    // [ DCNode -- SyncNode ]
    mpCamGraph->connectData(    CONTROL_PRV_SRC,    SYNC_SRC_1,         mpDefaultCtrlNode_Main2,    mpSyncNode);
    mpCamGraph->connectData(    CONTROL_PRV_SRC,    SYNC_SRC_0,         mpDefaultCtrlNode,          mpSyncNode);
    mpCamGraph->connectData(    CONTROL_CAP_SRC,    SYNC_SRC_2,         mpDefaultCtrlNode,          mpSyncNode);
    // [ SyncNode -- SNode (prepare stereo algo input data) ]
    mpCamGraph->connectData(    SYNC_DST_1,         STEREO_SRC,         mpSyncNode,                 mpStereoNode_Main2);
    mpCamGraph->connectData(    SYNC_DST_0,         STEREO_SRC,         mpSyncNode,                 mpStereoNode);
    // [ SNode -- SCNode (do stereo feature 3rd party algo) ]
    mpCamGraph->connectData(    STEREO_IMG,         STEREO_CTRL_IMG_1,  mpStereoNode_Main2,         mpStereoCtrlNode);
    mpCamGraph->connectData(    STEREO_IMG,         STEREO_CTRL_IMG_0,  mpStereoNode,               mpStereoCtrlNode);
    // // [ SCNode -- ITNode ]
    mpCamGraph->connectData(    STEREO_CTRL_DST_M,  TRANSFORM_SRC,      mpStereoCtrlNode,           mpImgTransformNode);

    MY_LOGD("CamGraph:connectNotify");
    mpCamGraph->connectNotify(  PASS1_START_ISP,    mpPass1Node_Main2,  mpDefaultCtrlNode_Main2);
    mpCamGraph->connectNotify(  PASS1_STOP_ISP,     mpPass1Node_Main2,  mpDefaultCtrlNode_Main2);
    mpCamGraph->connectNotify(  PASS1_EOF,          mpPass1Node_Main2,  mpDefaultCtrlNode_Main2);
    mpCamGraph->connectNotify(  PASS1_CONFIG_FRAME, mpPass1Node_Main2,  mpDefaultCtrlNode_Main2);
    mpCamGraph->connectNotify(  PASS1_START_ISP,    mpPass1Node,        mpDefaultCtrlNode);
    mpCamGraph->connectNotify(  PASS1_STOP_ISP,     mpPass1Node,        mpDefaultCtrlNode);
    mpCamGraph->connectNotify(  PASS1_EOF,          mpPass1Node,        mpDefaultCtrlNode);
    mpCamGraph->connectNotify(  PASS1_CONFIG_FRAME, mpPass1Node,        mpDefaultCtrlNode);
    //
    if ( !mpCamGraph->init() ) {
        MY_LOGE("CamGraph:init");
        goto lbExit;
    }
    //
    if(mpResMgrDrv)
    {

        MY_LOGD("CKH: Trigger");
        mpResMgrDrv->triggerCtrl();
    }
    //
    //
    if ( !mpCamGraph->start() ) {
        // use CAMERA_ERROR_SERVER_DIED callback to handle startPreview error
        mpCamMsgCbInfo->mNotifyCb(
                CAMERA_MSG_ERROR,
                CAMERA_ERROR_SERVER_DIED, 0,
                mpCamMsgCbInfo->mCbCookie
                );
        // do not return fail to avoid java exception
        mpStateManager->transitState(IState::eState_Preview);
        ret = OK;
        //
        MY_LOGE("CamGraph:start");
        goto lbExit;
    }
    //
    if( !mpDefaultCtrlNode->waitPreviewReady()
     || !mpDefaultCtrlNode_Main2->waitPreviewReady() )
    {
        MY_LOGE("wait preview ready failed");
        goto lbExit;
    }
    //
    MY_LOGD("transitState->eState_Preview");
    mpStateManager->transitState(IState::eState_Preview);
    MY_LOGD("-");
    ret = OK;
   //
lbExit:
    if(pSensorHalObj)
    {
        pSensorHalObj->destroyInstance(LOG_TAG);
        pSensorHalObj = NULL;
    }
    //
    if(ret != OK)
    {
        forceStopAndCleanPreview();
    }
    //
    return ret;
}


/******************************************************************************
*   CamAdapter::stopPreview() -> IState::onStopPreview() ->
*   IStateHandler::onHandleStopPreview() -> CamAdapter::onHandleStopPreview()
*******************************************************************************/
status_t
CamAdapter::
onHandleStopPreview()
{
    return forceStopAndCleanPreview();
}


/******************************************************************************
*   CamAdapter::takePicture() -> IState::onPreCapture() ->
*   IStateHandler::onHandlePreCapture() -> CamAdapter::onHandlePreCapture()
*******************************************************************************/
status_t
CamAdapter::
onHandlePreCapture()
{
    MY_LOGD("+");
    void* threadRet = NULL;
    if( pthread_create(&mThreadPrecap, NULL, doThreadPrecap, this) != 0 )
    {
        MY_LOGE("pthread create failed");
        goto lbExit;
    }
    if( !mpDefaultCtrlNode->precap() )
    {
        MY_LOGE("precap failed");
        goto lbExit;
    }
    if( pthread_join(mThreadPrecap, &threadRet) != 0 )
    {
        MY_LOGE("pthread join fail");
        goto lbExit;
    }
    if( !(bool)threadRet )
    {
        MY_LOGE("main2 precap failed");
        goto lbExit;
    }

    mpStateManager->transitState(IState::eState_PreCapture);
    MY_LOGD("-");
    return OK;

lbExit:
    return INVALID_OPERATION;
}


/******************************************************************************
*
 ******************************************************************************/
bool
CamAdapter::
main2precapture()
{
    MY_LOGD("+");
    return mpDefaultCtrlNode_Main2->precap();
}


/******************************************************************************
 *
 ******************************************************************************/
void*
CamAdapter::
doThreadPrecap(void* arg)
{
    ::prctl(PR_SET_NAME,"precapThread", 0, 0, 0);
    CamAdapter* pSelf = reinterpret_cast<CamAdapter*>(arg);
    return (void*)pSelf->main2precapture();
}


/******************************************************************************
*
*******************************************************************************/
status_t
CamAdapter::
forceStopAndCleanPreview()
{
    MY_LOGD("+");
    //
    if ( mpDefaultCtrlNode != NULL)
    {
        mpDefaultCtrlNode->setMode(DefaultCtrlNode::MODE_IDLE);
    }
    if ( mpDefaultCtrlNode_Main2 != NULL)
    {
        mpDefaultCtrlNode_Main2->setMode(DefaultCtrlNode::MODE_IDLE);
    }
    if ( mpSyncNode != NULL)
    {
        mpSyncNode->stopPreview();
    }
    if ( mpCamGraph != NULL )
    {
        mpCamGraph->stop();
        mpCamGraph->uninit();
        mpCamGraph->disconnect();
        mpCamGraph->destroyInstance();
        mpCamGraph = NULL;
    }
    //
    if ( mpPass1Node != NULL )
    {
        mpPass1Node->destroyInstance();
        mpPass1Node = NULL;
    }
    //
    if ( mpPass1Node_Main2 != NULL )
    {
        mpPass1Node_Main2->destroyInstance();
        mpPass1Node_Main2 = NULL;
    }
    //
    if ( mpStereoNode != NULL )
    {
        mpStereoNode->destroyInstance();
        mpStereoNode = NULL;
    }
    //
    if ( mpStereoNode_Main2 != NULL )
    {
        mpStereoNode_Main2->destroyInstance();
        mpStereoNode_Main2 = NULL;
    }
    //
    if ( mpStereoCtrlNode != NULL )
    {
        mpStereoCtrlNode->uninit();
        mpStereoCtrlNode->destroyInstance();
        mpStereoCtrlNode = NULL;
    }
    //
    if ( mpAllocBufHdl != NULL )
    {
        mpAllocBufHdl->uninit();
    }
    //
    if ( mpAllocBufHdl_Main2 != NULL )
    {
        mpAllocBufHdl_Main2->uninit();
    }
    //
    if ( mpDefaultBufHdl != NULL )
    {
        mpDefaultBufHdl->unmapPort(DefaultBufHandler::eBuf_Disp);
        mpDefaultBufHdl->unmapPort(DefaultBufHandler::eBuf_AP);
        mpDefaultBufHdl->unmapPort(DefaultBufHandler::eBuf_Generic);
        mpDefaultBufHdl->unmapPort(DefaultBufHandler::eBuf_FD);
        mpDefaultBufHdl->unmapPort(DefaultBufHandler::eBuf_OT);
    }
    //
    MY_LOGD("transitState->eState_Idle");
    mpStateManager->transitState(IState::eState_Idle);
    //
    if(mpResMgrDrv)
    {
        ResMgrDrv::MODE_STRUCT mode;
        mpResMgrDrv->getMode(&mode);
        mode.scenSw = ResMgrDrv::SCEN_SW_CAM_IDLE;
        mpResMgrDrv->setMode(&mode);
        // when stop preview, set mmdvfs to low frequency.
        mpResMgrDrv->setItem(ResMgrDrv::ITEM_STEREO, 0);
        mpResMgrDrv->triggerCtrl();
        mpHwSyncDrv-> sendCommand (HW_SYNC_CMD_UNDATE_CLOCK, 0x0, 0x0, 0x0, HW_SYNC_CLOCK_STAGE_0);
    }
    //
    MY_LOGD("-");
    //
    return OK;
}

