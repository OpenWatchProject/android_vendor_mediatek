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

#define LOG_TAG "MtkCam/EngFlowControl"
//
#include "../MyUtils.h"
//
#include <mtkcam/pipeline/utils/streaminfo/MetaStreamInfo.h>
#include <mtkcam/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam/pipeline/utils/streambuf/StreamBuffers.h>

#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>

#include <mtkcam/middleware/v1/camutils/CamInfo.h>
#include <mtkcam/middleware/v1/IParamsManager.h>
#include <mtkcam/middleware/v1/LegacyPipeline/request/IRequestController.h>
#include <mtkcam/middleware/v1/LegacyPipeline/ILegacyPipeline.h>
#include "EngFlowControl.h"
//
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
//
//
#define FEATURE_MODIFY (1)
//
#if FEATURE_MODIFY
#include <mtkcam/feature/utils/FeatureProfileHelper.h>
#include <mtkcam/feature/hdrDetection/Defs.h>
#endif // FEATURE_MODIFY
//
#define SUPPORT_3A (1)
//
#if SUPPORT_3A
#include <mtkcam/aaa/IHal3A.h>
using namespace NS3Av3;
#include <isp_tuning/isp_tuning.h>  //EIspProfile_T, EOperMode_*
using namespace NSIspTuning;
#endif
//



using namespace NSCam;
using namespace NSCam::v1;
using namespace NSCam::v1::NSLegacyPipeline;
using namespace android;
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

#if 1
#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")
#else
#define FUNC_START
#define FUNC_END
#endif
//
#define CHECK_OBJECT(x)  do{                                        \
    if (x == nullptr) { MY_LOGE("Null %s Object", #x); return MFALSE;} \
} while(0)


/*******************************************************************************
*
********************************************************************************/
MetaListener::
MetaListener(
    char const* pcszName,
    MINT32 const i4OpenId,
    sp<IParamsManagerV3> pParamsManagerV3
)
: mName(pcszName),
mOpenId(i4OpenId),
mpParamsManagerV3(pParamsManagerV3)
{
}

/*******************************************************************************
*
********************************************************************************/
void MetaListener::
onResultReceived(
    MUINT32         const /*requestNo*/,
    StreamId_T      const /*streamId*/,
    MBOOL           const /*errorResult*/,
    IMetadata       const result
)
{
#if SUPPORT_3A
    {
        MINT32 iAECurrentEV;
        MUINT32 iOutLen;
        iAECurrentEV = 0;
        //
        MINT32 i4BVOffset = 0;
        //
        IHal3A* pHal3a = MAKE_Hal3A(mOpenId, LOG_TAG);
        if ( ! pHal3a )
        {
            MY_LOGE("pHal3a == NULL");
            return;
        }
        //
        // update preview real time params value (3A --> AP)
        FrameOutputParam_T RTParams;
        if ( ! pHal3a->send3ACtrl(NS3Av3::E3ACtrl_GetRTParamsInfo, reinterpret_cast<MINTPTR>(&RTParams), 0) )
        {
            MY_LOGE("getSupportedParams fail");
            return;
        }
        // BV value
        int rt_BV = RTParams.i4BrightValue_x10;
        //
        //Get AE index, shutter, isp_gain, sensor_gain
        int index, shutter, isp_gain, sensor_gain, iso;
        index = RTParams.u4AEIndex;
        shutter = RTParams.u4PreviewShutterSpeed_us;
        isp_gain = RTParams.u4PreviewISPGain_x1024;
        sensor_gain = RTParams.u4PreviewSensorGain_x1024;
        iso = RTParams.u4RealISOValue;
        //
        //Get Capture shutter, isp_gain, sensor_gain from Preview
        int shutter_Cap, isp_gain_Cap, sensor_gain_Cap;
        shutter_Cap = RTParams.u4CapShutterSpeed_us;
        isp_gain_Cap = RTParams.u4CapISPGain_x1024;
        sensor_gain_Cap = RTParams.u4CapSensorGain_x1024;
        //
        // EV Calibration
        pHal3a->send3ACtrl(NS3Av3::E3ACtrl_GetCurrentEV, reinterpret_cast<MINTPTR>(&iAECurrentEV), reinterpret_cast<MINTPTR>(&iOutLen));
        //
        pHal3a->send3ACtrl(NS3Av3::E3ACtrl_GetBVOffset, reinterpret_cast<MINTPTR>(&i4BVOffset), reinterpret_cast<MINTPTR>(&iOutLen));
        //
        // update fps
        static MINT64 i8TimeStamp_Old = 0;//0xFFFFFFF
        static MINT64 i8TimeStamp_New = 0;
        //
        {
            IMetadata::IEntry entry = result.entryFor(MTK_SENSOR_TIMESTAMP);
            if( !entry.isEmpty() ) {
            i8TimeStamp_New = entry.itemAt(0, Type2Type<MINT64>());
            }
            //else
                //MY_LOGE("i8TimeStamp_New didn't exist in eSTREAMID_META_APP_DYNAMIC_P1");
        }
        //
        if (mpParamsManagerV3 != NULL)
        {
            mpParamsManagerV3->getParamsMgr()->updateBrightnessValue(rt_BV);// BV value
            mpParamsManagerV3->getParamsMgr()->updatePreviewAEIndexShutterGain(index, shutter, isp_gain, sensor_gain, iso);//preview AE params
            //
            mpParamsManagerV3->getParamsMgr()->updateCaptureShutterGain(shutter_Cap, isp_gain_Cap, sensor_gain_Cap);//Get Capture shutter, isp_gain, sensor_gain from Preview
            //
            mpParamsManagerV3->getParamsMgr()->set(MtkCameraParameters::KEY_ENG_EV_CALBRATION_OFFSET_VALUE, iAECurrentEV + i4BVOffset);
            //mpParamsManagerV3->getParamsMgr()->getStr(MtkCameraParameters::KEY_ENG_EV_CALBRATION_OFFSET_VALUE).string();
            //
            {
            //
                if (i8TimeStamp_New > i8TimeStamp_Old)
                {
                    mpParamsManagerV3->getParamsMgr()->updatePreviewFrameInterval((i8TimeStamp_New - i8TimeStamp_Old)/1000);
                }
                //MY_LOGD("Old(%lld), New(%lld)",i8TimeStamp_Old,i8TimeStamp_New);
                i8TimeStamp_Old = i8TimeStamp_New;
            }
        }
        else
        {
            // Show some message for spParamsMgr is NULL
            MY_LOGE("[Engflowcontrol] - onResultReceived - spParamsMgr is NULL");
        }
        pHal3a->destroyInstance(LOG_TAG);
    }
#endif
    //
}

/*******************************************************************************
*
********************************************************************************/
String8 MetaListener::
getUserName()
{
    return String8(LOG_TAG);
}

/******************************************************************************
 *
 ******************************************************************************/
EngFlowControl::
EngFlowControl(
    char const*                 pcszName,
    MINT32 const                i4OpenId,
    sp<IParamsManagerV3>        pParamsManagerV3,
    sp<ImgBufProvidersManager>  pImgBufProvidersManager,
    sp<INotifyCallback>         pCamMsgCbInfo
)
    : mName(const_cast<char*>(pcszName))
    , mOpenId(i4OpenId)
    , mpParamsManagerV3(pParamsManagerV3)
    , mpImgBufProvidersMgr(pImgBufProvidersManager)
    , mpCamMsgCbInfo(pCamMsgCbInfo)
    , mEnLtm(0)
{
    mLPBConfigParams.mode = 0;
    mLPBConfigParams.enableEIS = MFALSE;
    mLPBConfigParams.enableLCS = MFALSE;
    mLPBConfigParams.enableBurstNum = MFALSE;
    //
    MY_LOGD("ResourceContainer::getInstance(%d)",mOpenId);
    mpResourceContainer = IResourceContainer::getInstance(mOpenId);

}

/******************************************************************************
 *
 ******************************************************************************/
char const*
EngFlowControl::
getName()   const
{
    return mName;
}

/******************************************************************************
 *
 ******************************************************************************/
int32_t
EngFlowControl::
getOpenId() const
{
    return mOpenId;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
EngFlowControl::
startPreview()
{
    FUNC_START;
    mpRequestController = IRequestController::createInstance(
                                                mName,
                                                mOpenId,
                                                mpCamMsgCbInfo,
                                                mpParamsManagerV3
                                            );

    {
        mPipelineMode = PipelineMode_PREVIEW;
        constructNormalPreviewPipeline();
        int camera_mode = mpParamsManagerV3->getParamsMgr()->getInt(MtkCameraParameters::KEY_CAMERA_MODE);
        if (camera_mode != 0)
        {
            mpRequestController->setRequestType(MTK_CONTROL_CAPTURE_INTENT_VIDEO_RECORD);
            MY_LOGD("eng video raw dump flow : MTK_CONTROL_CAPTURE_INTENT_VIDEO_RECORD");
        }
        else
        {
            mpRequestController->setRequestType(MTK_CONTROL_CAPTURE_INTENT_PREVIEW);
            MY_LOGD("eng normal preview flow : MTK_CONTROL_CAPTURE_INTENT_PREVIEW");
        }
        mpParamsManagerV3->setScenario(IParamsManagerV3::eScenario::eScenario_Camera_Preview);
    }

    if ( mpPipeline == 0 ) {
        MY_LOGE("Cannot get pipeline. start preview fail.");
        return BAD_VALUE;
    }
    //

    MERROR ret = mpRequestController->startPipeline(
                                    0,/*start*/
                                    1000, /*end*/
                                    mpPipeline,
                                    this,
                                    mLPBConfigParams.mode
                                );
    //
    //EV calibration
    {
        sp<ResultProcessor> pResultProcessor = mpPipeline->getResultProcessor().promote();
        CHECK_OBJECT(pResultProcessor);
        //partial
        mpListener = new MetaListener(mName, mOpenId, mpParamsManagerV3);
        //
        pResultProcessor->registerListener(eSTREAMID_META_APP_DYNAMIC_P1, mpListener);
        //
    }
    //
    FUNC_END;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
EngFlowControl::
stopPreview()
{
    FUNC_START;
    mpRequestController->stopPipeline();
    mpRequestController = NULL;

    if ( mpPipeline != 0 ) {
        mpPipeline->flush();
        mpPipeline->waitUntilDrained();
        mpPipeline = NULL;
    }
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
EngFlowControl::
pausePreview(
    MBOOL stopPipeline
)
{
    Vector< BufferSet > vDstStreams;
    if ( !stopPipeline ) {
        vDstStreams.push_back(
        BufferSet{
            .streamId       = eSTREAMID_IMAGE_PIPE_RAW_OPAQUE,
            .criticalBuffer = false
            }
        );

        if (mLPBConfigParams.enableLCS) {
            vDstStreams.push_back(
            BufferSet{
                .streamId       = eSTREAMID_IMAGE_PIPE_RAW_LCSO,
                .criticalBuffer = false
                }
            );
        }
    }
    return ( mpRequestController == nullptr ) ? UNKNOWN_ERROR :
        mpRequestController->pausePipeline(vDstStreams);
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
EngFlowControl::
resumePreview()
{
    return ( mpRequestController == nullptr ) ? UNKNOWN_ERROR :
        mpRequestController->resumePipeline();
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
EngFlowControl::
highQualityZoom()
{
    return;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
EngFlowControl::
startRecording()
{

#if 0
    if ( mPipelineMode != PipelineMode_T::RECORDING ){
        mpRequestController->stopPipeline();

        if ( mpPipeline != 0 ) {
            mpPipeline->flush();
            mpPipeline->waitUntilDrained();
            mpPipeline = NULL;
        }
        //
        constructRecordingPipeline();
    }
#endif

    if ( mpPipeline == 0 ) {
        MY_LOGE("Cannot get pipeline. start preview fail.");
        return BAD_VALUE;
    }
    //
    mpRequestController->setRequestType(MTK_CONTROL_CAPTURE_INTENT_VIDEO_RECORD);


    return mpRequestController->startPipeline(
                                    1001,/*start*/
                                    2000, /*end*/
                                    mpPipeline,
                                    this
                                );
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
EngFlowControl::
stopRecording()
{
    // set capture intent back to preview
    if ( mpRequestController != 0 ) {
        mpRequestController->setRequestType(MTK_CONTROL_CAPTURE_INTENT_PREVIEW);
        mpRequestController->setParameters( this );
    } else {
        MY_LOGW("No RequestController.");
    }

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
EngFlowControl::
autoFocus()
{
    return (mpRequestController != 0) ?
        mpRequestController->autoFocus() : OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
EngFlowControl::
cancelAutoFocus()
{
    return (mpRequestController != 0) ?
        mpRequestController->cancelAutoFocus() : OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
EngFlowControl::
precapture(int& flashRequired)
{
    return (mpRequestController != 0) ?
        mpRequestController->precapture(flashRequired, 0) : OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
EngFlowControl::
setParameters()
{
    FUNC_START;

#if SUPPORT_3A

    IHal3A* pHal3a = MAKE_Hal3A(getOpenId(), LOG_TAG);
    if ( ! pHal3a )
    {
        MY_LOGE("pHal3a == NULL");
        return INVALID_OPERATION;
    }
    //
    pHal3a->send3ACtrl(NS3Av3::E3ACtrl_SetOperMode,NSIspTuning::EOperMode_EM, 0);
    //
    pHal3a->send3ACtrl(NS3Av3::E3ACtrl_SetAECamMode, eAppMode_EngMode, 0);
    //
    //AF Manual focus position
    MINT32 i4MFPos = mpParamsManagerV3->getParamsMgr()->getInt(MtkCameraParameters::KEY_FOCUS_ENG_STEP);
    pHal3a->send3ACtrl(NS3Av3::E3ACtrl_SetAFMFPos, i4MFPos, NULL);
    MY_LOGD("i4MFPos(%d)", i4MFPos);
    //
    //AF Full Scan
    MINT32 i4FullScanStep = mpParamsManagerV3->getParamsMgr()->getInt(MtkCameraParameters::KEY_ENG_FOCUS_FULLSCAN_FRAME_INTERVAL);
    MINT32 i4FullScanDacStep = 1;
    i4FullScanDacStep = mpParamsManagerV3->getParamsMgr()->getInt(MtkCameraParameters::KEY_ENG_FOCUS_FULLSCAN_DAC_STEP);
    if(mpParamsManagerV3->getParamsMgr()->getStr(CameraParameters::KEY_FOCUS_MODE) == "fullscan") 
    {
        i4FullScanStep = ((i4FullScanStep & 0xFFFF) << 16) + ((i4FullScanDacStep) & 0xFFFF);
        MY_LOGD("i4FullScanStep(%d)", i4FullScanStep);
        pHal3a->send3ACtrl(NS3Av3::E3ACtrl_SetAFFullScanStep, i4FullScanStep, NULL);
    }
    else
    {
        i4FullScanStep = 0;
        pHal3a->send3ACtrl(NS3Av3::E3ACtrl_SetAFFullScanStep, i4FullScanStep, NULL);
    }
    //
    // AWB enable
    MUINT32 u4AWBEnable = mpParamsManagerV3->getParamsMgr()->getInt(MtkCameraParameters::KEY_ENG_MTK_AWB_ENABLE) == 1 ?
                              MFALSE:MTRUE;
    pHal3a->send3ACtrl(NS3Av3::E3ACtrl_SetAwbBypCalibration, u4AWBEnable, 0 ); //MFALSE :ByPassAWB, MTRUE: apply OTP (CCTOPAWBBypassCalibration)
    //
    //shading
    int save_shading = mpParamsManagerV3->getParamsMgr()->getInt(MtkCameraParameters::KEY_ENG_SAVE_SHADING_TABLE);
    String8 ms3ACollectionFilePath;
    if (1 == save_shading)
    {
        String8 ms8RawFilePath(mpParamsManagerV3->getParamsMgr()->getStr(MtkCameraParameters::KEY_RAW_PATH)); // => /storage/sdcard1/DCIM/CameraEM/Preview01000108ISO0.raw
        ms8RawFilePath = ms8RawFilePath.getBasePath(); // => /storage/sdcard1/DCIM/CameraEM/Preview01000108ISO0

        MY_LOGD("shading_table_path = %s", ms8RawFilePath.string());
        pHal3a->send3ACtrl(NS3Av3::E3ACtrl_SetShadingSdblkCfg, MTRUE, reinterpret_cast<MINTPTR>(ms8RawFilePath.string()));
    }
    else
    {
        String8 ms8RawFilePath("");
        pHal3a->send3ACtrl(NS3Av3::E3ACtrl_SetShadingSdblkCfg, MFALSE, reinterpret_cast<MINTPTR>(ms8RawFilePath.string()));
    }
    // MTK Shading Enable
    MUINT32 u4ShadingEnable = mpParamsManagerV3->getParamsMgr()->getInt(MtkCameraParameters::KEY_ENG_MTK_SHADING_ENABLE) == 1 ?
                                             MTRUE : MFALSE;
    //shading table select
    int shading_table = mpParamsManagerV3->getParamsMgr()->getInt(MtkCameraParameters::KEY_ENG_SHADING_TABLE);
    //
    (u4ShadingEnable == 0) ?
        (pHal3a->send3ACtrl(NS3Av3::E3ACtrl_SetShadingEngMode, u4ShadingEnable, NULL)):
        (pHal3a->send3ACtrl(NS3Av3::E3ACtrl_SetShadingEngMode, (shading_table = shading_table + 1), NULL));
    MY_LOGD("save_shading=%d,u4ShadingEnable = %d,shading_table = %d", save_shading, u4ShadingEnable, shading_table);
    //
    // MTK shading 1 to 3 enable
    MUINT32 u4Shading123Enable = mpParamsManagerV3->getParamsMgr()->getInt(MtkCameraParameters::KEY_ENG_MTK_1to3_SHADING_ENABLE) == 1 ?
                                                   MFALSE : MTRUE;
    pHal3a->send3ACtrl(NS3Av3::E3ACtrl_SetShadingByp123, u4Shading123Enable, 0 );

    // Sensor OTP
    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
    IHalSensor* pSensorObj = NULL;
    if( pHalSensorList != NULL )
    {
        pSensorObj = pHalSensorList->createSensor(LOG_TAG, getOpenId());
        //
        // Sensor AWB OTP
        MUINT32 u4SensorAWBOTPEn = mpParamsManagerV3->getParamsMgr()->getInt(MtkCameraParameters::KEY_ENG_SENSOR_AWB_ENABLE);
        pSensorObj->sendCommand(pHalSensorList->querySensorDevIdx(getOpenId()),
                                                    SENSOR_CMD_SET_SENSOR_OTP_AWB_CMD,(MUINTPTR)&u4SensorAWBOTPEn,0,0);
        // Sensor Shading OTP
        MUINT32 u4SensorLSCOTPEn = mpParamsManagerV3->getParamsMgr()->getInt(MtkCameraParameters::KEY_ENG_SENSOR_SHADNING_ENABLE);
        pSensorObj->sendCommand(pHalSensorList->querySensorDevIdx(getOpenId()),
                                                    SENSOR_CMD_SET_SENSOR_OTP_LSC_CMD,(MUINTPTR)&u4SensorLSCOTPEn, 0, 0);
        //
        MY_LOGD("u4SensorAWBOTPEn=%d,u4SensorLSCOTPEn=%d", u4SensorAWBOTPEn, u4SensorLSCOTPEn );
    }
    else
    {
        MY_LOGE("pHalSensorList == NULL: Sensor Shading OTP and Sensor AWB OTP cant work");
    }

    //
    if(pSensorObj)
    {
        pSensorObj->destroyInstance(LOG_TAG);
        pSensorObj = NULL;
    }
    //
    MY_LOGD("i4MFPos= %d,i4FullScanStep= %d,u4AWBEnable=%d,u4Shading123Enable=%d",
             i4MFPos, i4FullScanStep, u4AWBEnable, u4Shading123Enable);
    //
#if FEATURE_MODIFY
    MUINT32 u4VHDmode = SENSOR_VHDR_MODE_NONE;
    u4VHDmode = mpParamsManagerV3->getParamsMgr()->getVHdr();
    if(u4VHDmode!= SENSOR_VHDR_MODE_NONE)
    {
        MINT32 iRatio= mpParamsManagerV3->getParamsMgr()->getInt(MtkCameraParameters::KEY_ENG_VIDEO_HDR_RATIO);
        pHal3a->send3ACtrl(NS3Av3::E3ACtrl_SetAEVHDRratio, iRatio, NULL);
        MY_LOGD("iRatio(%d)", iRatio);
    }
#endif

    //
    pHal3a->destroyInstance(LOG_TAG);

#endif

    FUNC_END;

    return (mpRequestController != 0) ?
        mpRequestController->setParameters( this ) : OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
EngFlowControl::
sendCommand(
    int32_t cmd,
    int32_t arg1,
    int32_t arg2
)
{
    // the argument "arg2" is NOT 0 if the command is sent from middleware,
    // we assume that the LTM on/off command should be sent from middleware hence
    // we need to check arg2 here
    if (cmd == IFlowControl::eExtCmd_setLtmEnable && arg2 != 0)
    {
        mEnLtm = arg1;
        MY_LOGD("set LTM enable to %d", mEnLtm);
        return OK; // this command is un-necessary to be sent to mpRequestController
    }

    return (mpRequestController != 0) ?
        mpRequestController->sendCommand( cmd, arg1, arg2 ) : INVALID_OPERATION;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
EngFlowControl::
dump(
    int /*fd*/,
    Vector<String8>const& /*args*/
)
{
//#warning "TODO"
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
EngFlowControl::
updateParameters(
    IMetadata* setting
)
{
    status_t ret;
    if( mpParamsManagerV3 != 0 )
    {
       ret = mpParamsManagerV3->updateRequest(setting, mSensorParam.mode);
       ret = mpParamsManagerV3->updateRequestEng(setting);

       return ret;
    }
    else
    {
        MY_LOGE("init in thread failed");
        goto lbExit;
    }

lbExit:
    return UNKNOWN_ERROR;

}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
EngFlowControl::
updateRequestSetting(
    IMetadata* appSetting,
    IMetadata* halSetting
)
{
    MBOOL isRepeating = true;
    // update app control

    // update hal control
    {
        IMetadata::IEntry entry(MTK_HAL_REQUEST_SENSOR_SIZE);
        entry.push_back(mSensorParam.size, Type2Type< MSize >());
        halSetting->update(entry.tag(), entry);
    }
    //video raw dump debugxif setting
    {
        int camera_mode = mpParamsManagerV3->getParamsMgr()->getInt(MtkCameraParameters::KEY_CAMERA_MODE);
        if (camera_mode != 0)
        {
            IMetadata::IEntry entry(MTK_HAL_REQUEST_REQUIRE_EXIF);
            entry.push_back(true, Type2Type< MUINT8 >());
            halSetting->update(entry.tag(), entry);
        }
    }
    //
    //isp profile
    EIspProfile_T profile;
    {
        MUINT32 apProfile =  mpParamsManagerV3->getParamsMgr()->getInt(MtkCameraParameters::KEY_ENG_ISP_PROFILE);
        //
        MY_LOGD("apProfile(KEY_ENG_ISP_PROFILE) = %d", apProfile);
        //
        switch(apProfile)
        {
             case 0:
                profile = EIspProfile_Preview;
                break;
             case 1:
                profile = EIspProfile_Capture;
                break;
             case 2:
                profile = EIspProfile_Video;
                break;
             default:
                profile = EIspProfile_Capture;
                break;
        }
        //
        IMetadata::IEntry entry(MTK_3A_ISP_PROFILE);
        entry.push_back(profile, Type2Type< MUINT8 >());
        halSetting->update(entry.tag(), entry);
    }
    //
#if FEATURE_MODIFY
    MUINT32 u4VHDmode = SENSOR_VHDR_MODE_NONE;
    u4VHDmode = mpParamsManagerV3->getParamsMgr()->getVHdr();
    MINT32 fMask = ProfileParam::FMASK_NONE;
    // update HDR mode to 3A
    HDRMode kHDRMode = mpParamsManagerV3->getParamsMgr()->getHDRMode();
    {
        IMetadata::IEntry entry(MTK_3A_HDR_MODE);
        entry.push_back(static_cast<MUINT8>(kHDRMode), Type2Type< MUINT8 >());
        halSetting->update(entry.tag(), entry);
    }
    if ((kHDRMode == HDRMode::AUTO) || (kHDRMode == HDRMode::VIDEO_AUTO))
        fMask |= ProfileParam::FMASK_AUTO_HDR_ON;
    //
    if(u4VHDmode!= SENSOR_VHDR_MODE_NONE)
    {
        // Prepare query Feature Streaming ISP Profile
        ProfileParam profileParam(
            mpPipeline->queryImageStreamInfo(eSTREAMID_IMAGE_PIPE_RAW_RESIZER)->getImgSize(),
            u4VHDmode,
            mSensorParam.mode,
            ProfileParam::FLAG_NONE,
            fMask, // TODO set FMask by isEIS and is Auto HDR
            profile
        );

        MUINT8 featureProf = 0;
        if (FeatureProfileHelper::getEngStreamingProf(featureProf, profileParam))
        {
            IMetadata::IEntry entry(MTK_3A_ISP_PROFILE);
            entry.push_back(featureProf, Type2Type< MUINT8 >());
            halSetting->update(entry.tag(), entry);
        }
    }
#endif
    // update default HAL settings
    mpParamsManagerV3->updateRequestHal(halSetting,mSensorParam.mode);
    mpParamsManagerV3->updateBasedOnScenario(appSetting, isRepeating);

    if ( !isRepeating ) {
        IMetadata::IEntry entry(MTK_HAL_REQUEST_REPEAT);
        entry.push_back(isRepeating, Type2Type< MUINT8 >());
        halSetting->update(entry.tag(), entry);
    }
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
EngFlowControl::
submitRequest(
    Vector< SettingSet > rvSettings,
    BufferList           rvDstStreams,
    Vector< MINT32 >&    rvRequestNo
)
{
    status_t ret = UNKNOWN_ERROR;
    if( mpRequestController == NULL)
    {
        MY_LOGE("mpRequestController is NULL");
        return UNKNOWN_ERROR;
    }
    //
    Vector< BufferList >  vDstStreams;
    for (size_t i = 0; i < rvSettings.size(); ++i) {
        vDstStreams.add(rvDstStreams);
    }
    ret = mpRequestController->submitRequest( rvSettings, vDstStreams, rvRequestNo );

    if( ret != OK)
    {
        MY_LOGE("submitRequest Fail!");
        return UNKNOWN_ERROR;
    }
    //
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
EngFlowControl::
submitRequest(
    Vector< SettingSet > rvSettings,
    Vector< BufferList > rvDstStreams,
    Vector< MINT32 >&    rvRequestNo
)
{
    status_t ret = UNKNOWN_ERROR;
    if( mpRequestController == NULL)
    {
        MY_LOGE("mpRequestController is NULL");
        return UNKNOWN_ERROR;
    }
    //
    ret = mpRequestController->submitRequest( rvSettings, rvDstStreams, rvRequestNo );

    if( ret != OK)
    {
        MY_LOGE("submitRequest Fail!");
        return UNKNOWN_ERROR;
    }
    //
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
EngFlowControl::
getRequestNo(
    MINT32 &requestNo
)
{
    status_t ret = UNKNOWN_ERROR;

    if (mpRequestController == NULL) {
        MY_LOGE("mpRequestController is NULL");
    }
    else {
        ret = mpRequestController->getRequestNo(requestNo);
    }

    if (ret != OK) {
        MY_LOGE("getRequestNo Fail!");
        requestNo = 0;
    }
    //
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
EngFlowControl::
onLastStrongRef( const void* /*id*/)
{
    FUNC_START;
    mpRequestController = NULL;
    if ( mpPipeline != 0 ) {
        mpPipeline->flush();
        mpPipeline->waitUntilDrained();
        mpPipeline = NULL;
    }
    FUNC_END;
}
