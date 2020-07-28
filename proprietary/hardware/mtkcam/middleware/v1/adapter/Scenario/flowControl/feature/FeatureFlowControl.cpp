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

#define LOG_TAG "MtkCam/FeatureFlowControl"
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
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/middleware/v1/LegacyPipeline/NodeId.h>
#include "FeatureFlowControl.h"
#include <mtkcam/pipeline/pipeline/PipelineContext.h>

#include <mtkcam/utils/fwk/MtkCamera.h>

#if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1) || (MTKCAM_HAVE_DUALCAM_DENOISE_SUPPORT == 1) || (MTKCAM_HAVE_VSDOF_SUPPORT == 1) || (MTKCAM_HAVE_STEREO_CAMERA_SUPPORT == 1)
// DualCam support
#include <mtkcam/feature/DualCam/ISyncManager.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
#define DUAL_CAM_SUPPORT 1
#else
#define DUAL_CAM_SUPPORT 0
#endif

#include <sys/prctl.h>

#include <future>
#include <vector>
#include <hardware/camera3.h>

#ifdef FEATURE_MODIFY
#include <mtkcam/feature/utils/FeatureProfileHelper.h>
#include <mtkcam/feature/hdrDetection/Defs.h>
#define VHDR_PROFILE_READY 1
#include <mtkcam/feature/eis/eis_ext.h>
#include <camera_custom_eis.h>
#endif // FEATURE_MODIFY

#include <custom/camera_custom_init_request.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>

using namespace NSCam;
using namespace NSCam::v1;
using namespace NSCam::v1::NSLegacyPipeline;
using namespace android;
using namespace NSCam::v3;
using namespace NSCam::v3::NSPipelineContext;
using namespace NSCamHW;
using namespace NSCam::Utils;

#define SHUTTERDELAYCNT (4)


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)
//
#define MY_LOGD1(...)               MY_LOGD_IF((mLogLevel>=1),__VA_ARGS__)
#define MY_LOGD2(...)               MY_LOGD_IF((mLogLevel>=2),__VA_ARGS__)
#define MY_LOGD3(...)               MY_LOGD_IF((mLogLevel>=3),__VA_ARGS__)
//
#define FUNC_START                  MY_LOGD("+")
#define FUNC_END                    MY_LOGD("-")
//
/******************************************************************************
 *
 ******************************************************************************/
static
MVOID
prepare_stream(BufferList& vDstStreams, StreamId id, MBOOL criticalBuffer)
{
    vDstStreams.push_back(
        BufferSet{
            .streamId       = id,
            .criticalBuffer = criticalBuffer,
        }
    );
}

/******************************************************************************
 *
 ******************************************************************************/
Mutex FeatureFlowControl:: sPreviewLock;

FeatureFlowControl::
FeatureFlowControl(
    char const*                 pcszName,
    MINT32 const                i4OpenId,
    sp<IParamsManagerV3>          pParamsManagerV3,
    sp<ImgBufProvidersManager>  pImgBufProvidersManager,
    sp<INotifyCallback>         pCamMsgCbInfo
)
    : mName(const_cast<char*>(pcszName))
    , mOpenId(i4OpenId)
    , mLogLevel(1)
    , mpRequestThreadLoopCnt(1)
    , mEnDng(0)
    , mEnLtm(0)
    , mAppCameraMode(MTK_FEATUREPIPE_PHOTO_PREVIEW)
    , mpParamsManagerV3(pParamsManagerV3)
    , mpImgBufProvidersMgr(pImgBufProvidersManager)
    , mpCamMsgCbInfo(pCamMsgCbInfo)
    , mb4K2KVideoRecord(MFALSE)
    , mpDeviceHelper(NULL)
    , mRequestType(MTK_CONTROL_CAPTURE_INTENT_PREVIEW)
    , mLastZoomRatio(100)
    , mbIsLastImgoPreview(MFALSE)
    , mbNeedHighQualityZoom(MFALSE)
    , mbVHDRUseImgoPreview(MFALSE)
    , mConstructVideoPipe(0)
    , mCurrentMode3DNR(0)
#if MTK_CAM_DISPLAY_INIT_REQUEST_FRAME_SUPPORT
    , mbNeedInitRequest(MTRUE)
    , mInitRequest(MTK_CAM_DISPLAY_INIT_REQUEST_FRAME_NUM)
#else
    , mbNeedInitRequest(MFALSE)
    , mInitRequest(0)
#endif
    , mbLowPowerVssExif(MFALSE)
    , mRRZOFmt(0)
    , mIMGOFmt(0)
{
    mLPBConfigParams.mode = LegacyPipelineMode_T::PipelineMode_Feature_Preview;
    mLPBConfigParams.enableEIS = MFALSE;
    mLPBConfigParams.enableLCS = MFALSE;
    mLPBConfigParams.enableRSS = MFALSE;
    mLPBConfigParams.enableFSC = MFALSE;
    //
    MINT lowPowerVss = ::property_get_int32("vendor.debug.camera.lowPowerVSS", MTKCAM_LOW_POWER_VSS_DEFAULT);
    MINT lowPowerVssExif = ::property_get_int32("vendor.debug.camera.lowPowerVSS.exif", -1);
    if(lowPowerVssExif >= 0)
    {
        mbLowPowerVssExif = (lowPowerVssExif > 0) ? MTRUE : MFALSE;
    }
    mbLowPowerVssExif = (lowPowerVss > 0) ? mbLowPowerVssExif : MFALSE;
    //
    MY_LOGD("ResourceContainer::getInstance(%d), low power vss(%d), low power vss exif(%d)", mOpenId, lowPowerVss, mbLowPowerVssExif);
    mpResourceContainer = IResourceContainer::getInstance(mOpenId);
    mForceEnableIMGO = property_get_int32("vendor.debug.feature.forceEnableIMGO", 0);
    mNeedDumpIMGO = property_get_int32("vendor.rawDump.imgo.enable", 0);
    mNeedDumpRRZO = property_get_int32("vendor.rawDump.rrzo.enable", 0);
    mDisableEISProfile = property_get_int32("vendor.debug.eis.disableprofile", 0);
    //Default enable Imgo Preview for ZSD preview
    #if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 0)
    mbNeedHighQualityZoom = (property_get_int32("vendor.debug.feature.enableImgoPrv", 1) > 0) ? MTRUE : MFALSE;
    #endif
    int debugInitReq = property_get_int32("vendor.debug.camera.initreq", -1);
    if( debugInitReq > 0 )
    {
        mbNeedInitRequest = MTRUE;
        mInitRequest = 4;
        MY_LOGD("forced to set mbNeedInitRequest(%d) mInitRequest(%d)",mbNeedInitRequest,mInitRequest);
    }
    else if ( debugInitReq == 0 )
    {
        mbNeedInitRequest = MFALSE;
        mInitRequest = 0;
        MY_LOGD("forced to set mbNeedInitRequest(%d) mInitRequest(%d)",mbNeedInitRequest,mInitRequest);
    }
    //
    mbSwitchModeEnable = mpParamsManagerV3->getParamsMgr()->getDynamicSwitchSensorMode();
    if(mbSwitchModeEnable)
    {
        int entryHighLvTh = property_get_int32("vendor.debug.switch.mode.highlv_x10", 999);
        int entryLowLvTh = property_get_int32("vendor.debug.switch.mode.lowlv_x10", -999);
        if(entryHighLvTh!=999)
        {
            mfSwitchModeEnterHighLvTh = (float)entryHighLvTh/10;
            MY_LOGD("forced to set mfSwitchModeEnterHighLvTh(%.1f)",mfSwitchModeEnterHighLvTh);
        }
        else
        {
            mfSwitchModeEnterHighLvTh = SWITCH_MODE_FROM_LOW_LV_TO_HIGH_LV_TH;
            MY_LOGD("use defualt mfSwitchModeEnterHighLvTh(%.1f)",mfSwitchModeEnterHighLvTh);
        }
        //
        if(entryLowLvTh!=-999)
        {
            mfSwitchModeEnterLowLvTh = (float)entryLowLvTh/10;
            MY_LOGD("forced to set mfSwitchModeEnterLowLvTh(%.1f)",mfSwitchModeEnterLowLvTh);
        }
        else
        {
            mfSwitchModeEnterLowLvTh = SWITCH_MODE_FROM_HIGH_LV_TO_LOW_LV_TH;
            MY_LOGD("use defualt mfSwitchModeEnterLowLvTh(%.1f)",mfSwitchModeEnterLowLvTh);
        }
    }
    //
    mpHal3a = MAKE_Hal3A(getOpenId(),LOG_TAG);
    //
    mP1NodeConcurrency = IResourceConcurrency::createInstance("mP1NodeConcurrency", RESOURCE_CONCURRENCY_TIMEOUT_MS);
    mP2NodeConcurrency = IResourceConcurrency::createInstance("mP2NodeConcurrency", RESOURCE_CONCURRENCY_TIMEOUT_MS);
    //
    mpResourceContainer->setResourceConcurrency(IResourceContainer::RES_CONCUR_P1, mP1NodeConcurrency);
    mpResourceContainer->setResourceConcurrency(IResourceContainer::RES_CONCUR_P2, mP2NodeConcurrency);
    //
    mpCpuCtrl = CpuCtrl::createInstance();
    if(mpCpuCtrl)
    {
        mpCpuCtrl->init();
    }

    ::memset(&mstartP1NodeFlushThreadHandle, 0x00, sizeof(mstartP1NodeFlushThreadHandle ));
}

/******************************************************************************
 *
 ******************************************************************************/
FeatureFlowControl::
~FeatureFlowControl()
{
    if(mpCpuCtrl)
    {
        mpCpuCtrl->disable();
        mpCpuCtrl->uninit();
        mpCpuCtrl->destroyInstance();
        mpCpuCtrl = NULL;
    }
    //
    if  ( mpHal3a )
    {
        mpHal3a->destroyInstance(LOG_TAG);
        mpHal3a = NULL;
    }
    //
    mpResourceContainer->clearResourceConcurrency(IResourceContainer::RES_CONCUR_P1);
    mpResourceContainer->clearResourceConcurrency(IResourceContainer::RES_CONCUR_P2);
}


/******************************************************************************
 *
 ******************************************************************************/
char const*
FeatureFlowControl::
getName()   const
{
    return mName;
}

/******************************************************************************
 *
 ******************************************************************************/
int32_t
FeatureFlowControl::
getOpenId() const
{
    return mOpenId;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
FeatureFlowControl::
startPreview()
{
    FUNC_START;
    //
    pauseSwitchModeFlow();
    //
    MY_LOGD("HDR mode(%u)", (MUINT32)mpParamsManagerV3->getParamsMgr()->getHDRMode());
    //
    if(mbSwitchModeEnable && isHdrUiOn())
    {
        setNowSensorModeStatusForSwitchFlow(eSensorModeStatus_3HDR);
        MY_LOGD("setNowSensorModeStatusForSwitchFlow(eSensorModeStatus_3HDR);");
    }
    //

    MINT32 lcsOpen = mEnLtm;
    MUINT32 vhdrMode = mpParamsManagerV3->getParamsMgr()->getVHdr();
    mLPBConfigParams.enableLCS = (lcsOpen > 0) || (vhdrMode == SENSOR_VHDR_MODE_ZVHDR)
                            || (vhdrMode == SENSOR_VHDR_MODE_IVHDR); // LCS always true

    mpRequestController = IRequestController::createInstance(
                                                mName,
                                                mOpenId,
                                                mpCamMsgCbInfo,
                                                mpParamsManagerV3
                                            );
    //
    mpResourceContainer->setFeatureFlowControl(this);
    //
    mpDeviceHelper = new CamManager::UsingDeviceHelper(getOpenId());
    HwInfoHelper helper(mOpenId);
    if( ! helper.updateInfos() )
    {
        MY_LOGE("cannot properly update infos");
        return BAD_VALUE;
    }
    //
    MUINT32 u4RawFmtType;
    if(helper.get4CellSensorSupported() &&
       helper.getSensorRawFmtType(u4RawFmtType) &&
       u4RawFmtType != SENSOR_RAW_4CELL_HW_BAYER)
    {
        sp<NSCam::IPlugProcessing> pPlugProcess = IPlugProcessing::createInstance((MUINT32)IPlugProcessing::PLUG_ID_FCELL, (NSCam::IPlugProcessing::DEV_ID)mOpenId);
        pPlugProcess->init(IPlugProcessing::OP_MODE_ASYNC);
    }
    //
    if(mbNeedInitRequest)
    {
        MUINT32  burstNum = 1;
        MINT32 previewFps = mpParamsManagerV3->getParamsMgr()->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE);
        bool recordinghint = mpParamsManagerV3->getParamsMgr()->getRecordingHint();
        if(previewFps > HighSpeedVideoFpsBase)
        {
            burstNum = previewFps/HighSpeedVideoFpsBase;
        }
        mbNeedInitRequest=(burstNum>1 && recordinghint) ? MFALSE : MTRUE;
        MY_LOGD("burstNum %d, mbNeedInitRequest %d, previewFps %d, recordinghint(%d)",burstNum, mbNeedInitRequest, previewFps, recordinghint);
    }
#if DUAL_CAM_SUPPORT
    if(NSCam::v1::Stereo::E_DUALCAM_FEATURE_ZOOM == StereoSettingProvider::getStereoFeatureMode())
    {
        mLPBConfigParams.enableEIS = MTRUE;
    }
    else if(StereoSettingProvider::isDualCamMode())
    {
        int main1id, main2id;
        StereoSettingProvider::getStereoSensorIndex(main1id, main2id);
        if(getOpenId() == main1id)
        {
            MY_LOGD("set stereo eis to main1");
            mLPBConfigParams.enableEIS = MTRUE;
        }
        else
        {
            mLPBConfigParams.enableEIS = MFALSE;
        }
    }
    else
    {
        mLPBConfigParams.enableEIS = mpDeviceHelper->isFirstUsingDevice();
    }
#else
    mLPBConfigParams.enableEIS = mpDeviceHelper->isFirstUsingDevice();
#endif // DUAL_CAM_SUPPORT
    if ( mpParamsManagerV3->getParamsMgr()->getRecordingHint() )
    {
        constructRecordingPipeline();
        mpParamsManagerV3->setScenario(IParamsManagerV3::eScenario::eScenario_Video_Preview);
        mRequestType = MTK_CONTROL_CAPTURE_INTENT_PREVIEW;
        mAppCameraMode = MTK_FEATUREPIPE_VIDEO_PREVIEW;
    } else if ( (PARAMSMANAGER_MAP_INST(eMapAppMode)->stringFor(mpParamsManagerV3->getParamsMgr()->getHalAppMode())
                ==  MtkCameraParameters::APP_MODE_NAME_MTK_ZSD)||
                (PARAMSMANAGER_MAP_INST(eMapAppMode)->stringFor(mpParamsManagerV3->getParamsMgr()->getHalAppMode())
                    ==  MtkCameraParameters::APP_MODE_NAME_MTK_STEREO))
    {
        // if current app mode is ZSD or stereo, it has to create preview pipeline as zsd pipeline.
        constructZsdPreviewPipeline();
#if MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT
        int featureMode = StereoSettingProvider::getStereoFeatureMode();
        MBOOL isVsdofMode =
                (featureMode == (Stereo::E_STEREO_FEATURE_CAPTURE|Stereo::E_STEREO_FEATURE_VSDOF)) ||
                (featureMode == (Stereo::E_STEREO_FEATURE_MTK_DEPTHMAP));
        if(isVsdofMode == MFALSE)
        {
            mFutureExecuteCreateCapPipeline = std::async(std::launch::async, [this]()->void
            {
                mpCommonCapturePipeline = ICommonCapturePipeline::createCommonCapturePipeline(getOpenId(), mpParamsManagerV3);
            });
        }
#endif
        mRequestType = MTK_CONTROL_CAPTURE_INTENT_PREVIEW;
        mpParamsManagerV3->setScenario(IParamsManagerV3::eScenario::eScenario_Camera_Preview);
        mAppCameraMode = MTK_FEATUREPIPE_PHOTO_PREVIEW;
    }
    else
    {
        constructNormalPreviewPipeline();
        mRequestType = MTK_CONTROL_CAPTURE_INTENT_PREVIEW;
        mpParamsManagerV3->setScenario(IParamsManagerV3::eScenario::eScenario_Camera_Preview);
        mAppCameraMode = MTK_FEATUREPIPE_PHOTO_PREVIEW;
    }
    mpDeviceHelper->configDone();
    mpRequestController->setRequestType(mRequestType);
    //
    mpParamsManagerV3->setPreviewMaxFps(
                            (mpParamsManagerV3->getParamsMgr()->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE) <= static_cast<int>(mSensorParam.fps)) ?
                            mpParamsManagerV3->getParamsMgr()->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE) : static_cast<int>(mSensorParam.fps));
    //
    if ( mpPipeline == 0 ) {
        MY_LOGE("Cannot get pipeline. start preview fail.");
        mpDeviceHelper = NULL;
        return BAD_VALUE;
    }

    //
    if(mbNeedInitRequest)//if(helper.isYuv() && mbNeedInitRequest)
    {
        //
        MY_LOGD("isYuv(%d) mInitRequest(%d) mbNeedInitRequest(%d)",
            helper.isYuv(), mInitRequest, mbNeedInitRequest);
        //
        Vector< SettingSet > vSettings;
        BufferList           vDstStreams;
        Vector< MINT32 >     vRequestNo;
        for(int i=0; i<mInitRequest; i++)
        {
            preparePreviewInfo(vSettings,vDstStreams);
        }
        mpRequestController->setDummpyRequest( vSettings, vDstStreams, vRequestNo );
    }
    //
    // TODO: fixme start & end request number
    MERROR ret = mpRequestController->startPipeline(
                                    0,/*start*/
                                    1000, /*end*/
                                    mpPipeline,
                                    this,
                                    mLPBConfigParams.mode,
                                    mpRequestThreadLoopCnt,
                                    &mLPBConfigParams);
    //
    for ( size_t i = 0; i < mFocusCommandList.size(); ++i) {
        switch( mFocusCommandList[i] ) {
            case COMMAND_AUTO_FOCUS:
                mpRequestController->autoFocus();
                MY_LOGE("should not happen.");
            break;
            case COMMAND_CANCEL_AUTO_FOCUS:
                mpRequestController->cancelAutoFocus();
                MY_LOGD("Cancel AF.");
            break;
        };
    }
    mFocusCommandList.clear();
    //
    resumeSwitchModeFlow();
    //
    FUNC_END;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
FeatureFlowControl::
stopPreview()
{
    FUNC_START;
    Mutex::Autolock _l(sPreviewLock);
    //
    pauseSwitchModeFlow();
    //
    std::vector< std::future<MERROR> > vFutures;
    if( mpRequestController != 0 ) {
        vFutures.push_back(
            std::async(std::launch::async,
                [ this ]() { return mpRequestController->stopPipeline(); }
            )
        );
    }

    if ( mpPipeline != 0 ) {
        mpPipeline->flush();
        mpPipeline->waitUntilDrained();
        mpPipeline = NULL;
        mpDeviceHelper = NULL;
    }

#if MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT
    if ( PARAMSMANAGER_MAP_INST(eMapAppMode)->stringFor(mpParamsManagerV3->getParamsMgr()->getHalAppMode())
                ==  MtkCameraParameters::APP_MODE_NAME_MTK_ZSD) {
        if ( mpCommonCapturePipeline != nullptr ) {
            mFutureExecuteRemoveCapPipeline = std::async(std::launch::async, [this]()->void
            {
                ICommonCapturePipeline::removeCommonCapturePipeline(getOpenId());
            });
        }
    }
#endif

    for( auto &fut : vFutures ) {
        MERROR result = fut.get();
    }
    mpRequestController = NULL;
    mb4K2KVideoRecord = MFALSE;
    //
    if(mbSwitchModeEnable && isHdrUiOn())
    {
        String8 str = mpParamsManagerV3->getParamsMgr()->getStaticVhdrMode();
        const char *pstr = str.string();
        mpParamsManagerV3->getParamsMgr()->set(MtkCameraParameters::KEY_VIDEO_HDR_MODE,pstr);
        MY_LOGD("restore KEY_VIDEO_HDR_MODE to (%s)",pstr);
    }
    //
    //
    resumeSwitchModeFlow();
    //
    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
FeatureFlowControl::
pausePreview(
    MBOOL stopPipeline
)
{
    FUNC_START;
    //
    pauseSwitchModeFlow();
    //
    Vector< BufferSet > vDstStreams;
    if ( !stopPipeline ) {
        vDstStreams.push_back(
            BufferSet{
                .streamId       = eSTREAMID_IMAGE_PIPE_RAW_OPAQUE,
                .criticalBuffer = false
            }
        );
        vDstStreams.push_back(
            BufferSet{
                .streamId       = eSTREAMID_IMAGE_PIPE_RAW_RESIZER,
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
    status_t ret = ( mpRequestController == nullptr ) ? UNKNOWN_ERROR :
        mpRequestController->pausePipeline(vDstStreams);
    //
    resumeSwitchModeFlow();
    //
    FUNC_END;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
FeatureFlowControl::
resumePreview()
{
    FUNC_START;
    //
    pauseSwitchModeFlow();
    //
    status_t ret = ( mpRequestController == nullptr ) ? UNKNOWN_ERROR :
        mpRequestController->resumePipeline();
    //
    resumeSwitchModeFlow();
    //
    FUNC_END;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
FeatureFlowControl::
highQualityZoom()
{
    if( mbNeedHighQualityZoom )
    {
        runHighQualityZoomFlow();
    }
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
FeatureFlowControl::
startRecording()
{
    FUNC_START;
    //
    pauseSwitchModeFlow();
    //
    status_t status;
    mAppCameraMode = MTK_FEATUREPIPE_VIDEO_RECORD;
    setEISParameter();
#if 1
    if( ( mLPBConfigParams.mode != LegacyPipelineMode_T::PipelineMode_VideoRecord &&
          mLPBConfigParams.mode != LegacyPipelineMode_T::PipelineMode_HighSpeedVideo &&
          mLPBConfigParams.mode != LegacyPipelineMode_T::PipelineMode_Feature_VideoRecord &&
          mLPBConfigParams.mode != LegacyPipelineMode_T::PipelineMode_Feature_HighSpeedVideo ) ||
        needReconstructRecordingPipe() )
    {
        mpRequestController->stopPipeline();

        if ( mpPipeline != 0 ) {
            mpPipeline->flush();
            mpPipeline->waitUntilDrained();
            mpPipeline = NULL;
            mpDeviceHelper = NULL;
        }
        //
        mpDeviceHelper = new CamManager::UsingDeviceHelper(getOpenId());
        constructRecordingPipeline();
        mpDeviceHelper->configDone();
        mpParamsManagerV3->setScenario(IParamsManagerV3::eScenario::eScenario_Video_Preview);
        mLPBConfigParams.enableEIS = MFALSE;
    }
#endif

    if ( mpPipeline == 0 ) {
        MY_LOGE("Cannot get pipeline. start preview fail.");
        mpDeviceHelper = NULL;
        //
        resumeSwitchModeFlow();
        //
        FUNC_END;
        return BAD_VALUE;
    }
    //
    mpParamsManagerV3->setScenario(IParamsManagerV3::eScenario::eScenario_Video_Record_Start);
    mRequestType = MTK_CONTROL_CAPTURE_INTENT_VIDEO_RECORD;
    mpRequestController->setRequestType(mRequestType);
    //
    // To avoid every time startPipeline request index is same as last time submit request index
    static int requestRangeIndex = 0;
    requestRangeIndex++;
    if(requestRangeIndex>9)
        requestRangeIndex=1;
    //
    status = mpRequestController->startPipeline(
                                    requestRangeIndex*1000+1,/*start*/
                                    requestRangeIndex*1000+1000, /*end*/
                                    mpPipeline,
                                    this,
                                    mLPBConfigParams.mode,
                                    mpRequestThreadLoopCnt,
                                    &mLPBConfigParams);

    Vector< SettingSet > vSettings;
    BufferList           vDstStreams;
    Vector< MINT32 >     vRequestNo;

    prepareVideoInfo(vSettings,vDstStreams);
    submitRequest( vSettings, vDstStreams, vRequestNo );
    //
    resumeSwitchModeFlow();
    //
    FUNC_END;
    return status;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
FeatureFlowControl::
stopRecording()
{
    FUNC_START;
    //
    pauseSwitchModeFlow();
    //
    mAppCameraMode = MTK_FEATUREPIPE_VIDEO_PREVIEW;
    if (mpParamsManagerV3->getParamsMgr()->getVideoStabilization())
    {
        if ((mLPBConfigParams.mode == LegacyPipelineMode_T::PipelineMode_VideoRecord) ||
            (mLPBConfigParams.mode == LegacyPipelineMode_T::PipelineMode_Feature_VideoRecord) )
        {
            //Theoretically, this should be set by Camera AP
            MY_LOGD("Set EIS25 Mode back to 0");
            mpParamsManagerV3->getParamsMgr()->set(MtkCameraParameters::KEY_EIS25_MODE,0);
            mpParamsManagerV3->getParamsMgr()->set(MtkCameraParameters::KEY_EIS_SUPPORTED_FRAMES,0);
        }
    }

    // set capture intent back to preview
    if ( mpRequestController != 0 ) {
        mpParamsManagerV3->setScenario(IParamsManagerV3::eScenario::eScenario_Video_Record_Stop);
        mRequestType = MTK_CONTROL_CAPTURE_INTENT_PREVIEW;
        mpRequestController->setRequestType(mRequestType);
        mpRequestController->setParameters( this );

        return mpRequestController->startPipeline(
                                0,/*start*/
                                1000, /*end*/
                                mpPipeline,
                                this,
                                mLPBConfigParams.mode,
                                mpRequestThreadLoopCnt,
                                &mLPBConfigParams);
    } else {
        MY_LOGW("No RequestController.");
    }
    //
    resumeSwitchModeFlow();
    //
    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
FeatureFlowControl::
autoFocus()
{
    FUNC_START;
    //
    pauseSwitchModeFlow();
    //
    status_t ret = OK;
    if ( mpRequestController != 0 ) {
        ret =  mpRequestController->autoFocus();
    } else {
        mFocusCommandList.push_back(COMMAND_AUTO_FOCUS);
    }
    //
    resumeSwitchModeFlow();
    //
    FUNC_END;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
FeatureFlowControl::
cancelAutoFocus()
{
    FUNC_START;
    //
    pauseSwitchModeFlow();
    //
    status_t ret = OK;
    if ( mpRequestController != 0 ) {
        ret = mpRequestController->cancelAutoFocus();
    } else {
        mFocusCommandList.push_back(COMMAND_CANCEL_AUTO_FOCUS);
    }
    //
    resumeSwitchModeFlow();
    //
    FUNC_END;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
FeatureFlowControl::
precapture(int& flashRequired)
{
    // FIXME: should get from 3A flow
    int Calibration = ::property_get_int32("vendor.flash_calibration", 0) ||
        ::property_get_int32("vendor.flash_quick_calibration_2", 0) ||
        ::property_get_int32("vendor.flash_ratio", 0) ||
        ::property_get_int32("vendor.flash_ratio_quick2", 0);
    nsecs_t timeout = (Calibration == 1) ? 0 : 6000000000LL; // default timeout 6 sec
    return (mpRequestController != 0) ?
        mpRequestController->precapture(flashRequired, timeout) : OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
FeatureFlowControl::
takePicture()
{
#if 1
    FUNC_START;

    if( mLPBConfigParams.mode == LegacyPipelineMode_T::PipelineMode_VideoRecord ||
        mLPBConfigParams.mode == LegacyPipelineMode_T::PipelineMode_Feature_VideoRecord )
    {
        Vector< SettingSet > vSettings;
        BufferList           vDstStreams;
        Vector< MINT32 >     vRequestNo;

        prepareVSSInfo(vSettings,vDstStreams);
        submitRequest( vSettings, vDstStreams, vRequestNo );

        FUNC_END;
        return OK;
    }
    else
    {
        MY_LOGW("Not support LegacyPipelineMode %d",mLPBConfigParams.mode);
    }

#endif
    //
    FUNC_END;
    return UNKNOWN_ERROR;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
FeatureFlowControl::
runHighQualityZoomFlow()
{
    if( mpParamsManagerV3 != NULL &&
        mpParamsManagerV3->getParamsMgr() != NULL &&
        mpPipeline != NULL)
    {
        //For ZSD preview only (because only ZSD preview has IMGO)
        if( mLPBConfigParams.mode == LegacyPipelineMode_T::PipelineMode_ZsdPreview ||
            mLPBConfigParams.mode == LegacyPipelineMode_T::PipelineMode_Feature_ZsdPreview)
        {
            MUINT32 zoomRatio = mpParamsManagerV3->getParamsMgr()->getZoomRatio();
            mpParamsManagerV3->calculateCropRegion(getSensorMode());
            MRect reqCropRegion, reqSensorCropRegion, reqPreviewCropRegion, reqSensorPreviewCropRegion;
            MSize sensorSize;
            mpParamsManagerV3->getCropRegion(sensorSize, reqCropRegion, reqSensorCropRegion, reqPreviewCropRegion, reqSensorPreviewCropRegion);
            MSize previewsize;
            mpParamsManagerV3->getParamsMgr()->getPreviewSize(&previewsize.w, &previewsize.h);
            //
            MY_LOGD("sensor mode = %d",getSensorMode());
            MY_LOGD("now zoomRatio(%d) last zoomratio(%d)",zoomRatio,mLastZoomRatio);
            MY_LOGD("previewsize(%d,%d)",previewsize.w, previewsize.h);
            MY_LOGD("reqSensorPreviewCropRegion(%d,%d,%d,%d)", reqSensorPreviewCropRegion.p.x,reqSensorPreviewCropRegion.p.y,reqSensorPreviewCropRegion.s.w,reqSensorPreviewCropRegion.s.h);
            MY_LOGD("reqSensorCropRegion(%d,%d,%d,%d)", reqSensorCropRegion.p.x,reqSensorCropRegion.p.y,reqSensorCropRegion.s.w,reqSensorCropRegion.s.h);
            //
            //the case is for saving pass2 power & performance
            MBOOL bUseImgoPreview = (reqSensorPreviewCropRegion.s.w < previewsize.w && reqSensorPreviewCropRegion.s.h < previewsize.h) ? MTRUE : MFALSE;
            MY_LOGD("bUseImgoPreview(%d) mbIsLastImgoPreview(%d)",bUseImgoPreview,mbIsLastImgoPreview);
            //
            if ( zoomRatio != mLastZoomRatio &&
                 bUseImgoPreview != mbIsLastImgoPreview )
            {
                setNewZsdPreviewRequestBuilder(bUseImgoPreview);
                mbIsLastImgoPreview = bUseImgoPreview;
                //for VHDR Imgo profile setting
                mbVHDRUseImgoPreview = bUseImgoPreview;
            }
            mLastZoomRatio = zoomRatio;
        }
    }
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
FeatureFlowControl::
setNewZsdPreviewRequestBuilder(MBOOL useImgoPreview)
{
    MY_LOGD("useImgoPreview = %d", useImgoPreview);
    //
    BufferList vDstStreams;
    prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_RAW_OPAQUE, false);
    prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_RAW_RESIZER, false);
    if(mLPBConfigParams.enableLCS)
        prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_RAW_LCSO, false);
    if(mLPBConfigParams.enableRSS)
        prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_RAW_RSSO, false);
    prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_YUV_00, false);
    prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_YUV_01, false);
    prepare_stream(vDstStreams, eSTREAMID_IMAGE_YUV_FD, false);
    //
    if( mpPipeline == NULL )
    {
        MY_LOGW("setNewZsdPreviewRequestBuilder Failed (mpPipeline is NULL)");
        return;
    }
    //
    sp<RequestBuilder> pRequestBuilder = new RequestBuilder();
    KeyedVector< NSCam::v3::Pipeline_NodeId_T, NSCam::v3::NSPipelineContext::IOMapSet > vMetaIOMapInfo;
    mpPipeline->getMetaIOMapInfo(vMetaIOMapInfo);
    //P1 node
    IOMap p1_Image_IOMap;
    for ( size_t i = 0; i < vDstStreams.size(); ++i ) {
        switch( vDstStreams[i].streamId ) {
            case eSTREAMID_IMAGE_PIPE_RAW_OPAQUE:
            case eSTREAMID_IMAGE_PIPE_RAW_RESIZER:
            case eSTREAMID_IMAGE_PIPE_RAW_LCSO:
            case eSTREAMID_IMAGE_PIPE_RAW_RSSO:
                p1_Image_IOMap.addOut(vDstStreams[i].streamId);
                break;
            default:
                break;
        }
    }
    pRequestBuilder->setIOMap(
            eNODEID_P1Node,
            IOMapSet().add(
                p1_Image_IOMap
                ),
            vMetaIOMapInfo.valueFor(eNODEID_P1Node)
            );
    pRequestBuilder->setRootNode(
        NodeSet().add(eNODEID_P1Node)
        );
    //P2 node
    IOMap p2_Image_IOMap;
    for ( size_t i = 0; i < vDstStreams.size(); ++i ) {
        switch( vDstStreams[i].streamId ) {
            case eSTREAMID_IMAGE_PIPE_YUV_00:
            case eSTREAMID_IMAGE_PIPE_YUV_01:
            case eSTREAMID_IMAGE_YUV_FD:
                p2_Image_IOMap.addOut(vDstStreams[i].streamId);
                break;
            case eSTREAMID_IMAGE_PIPE_RAW_RESIZER:
                if(!useImgoPreview)
                {
                    p2_Image_IOMap.addIn(eSTREAMID_IMAGE_PIPE_RAW_RESIZER);
                }
                break;
            case eSTREAMID_IMAGE_PIPE_RAW_LCSO:
                p2_Image_IOMap.addIn(eSTREAMID_IMAGE_PIPE_RAW_LCSO);
                break;
            case eSTREAMID_IMAGE_PIPE_RAW_RSSO:
                p2_Image_IOMap.addIn(eSTREAMID_IMAGE_PIPE_RAW_RSSO);
                break;
            case eSTREAMID_IMAGE_PIPE_RAW_OPAQUE:
                if(useImgoPreview)
                {
                    p2_Image_IOMap.addIn(eSTREAMID_IMAGE_PIPE_RAW_OPAQUE);
                }
                break;
            default:
                break;
        };
    }
    pRequestBuilder->setIOMap(
            eNODEID_P2Node,
            IOMapSet().add(
                p2_Image_IOMap
                ),
            vMetaIOMapInfo.valueFor(eNODEID_P2Node)
            );
    // edge
    pRequestBuilder->setNodeEdges(
        NodeEdgeSet().addEdge(eNODEID_P1Node, eNODEID_P2Node)
        );

    // update FrameCallback
    sp<ResultProcessor> pResultProcessor = mpPipeline->getResultProcessor().promote();
    if( pResultProcessor != NULL )
        pRequestBuilder->updateFrameCallback(pResultProcessor);

    // set RequestBuilder
    mpPipeline->setRequestBuilder(pRequestBuilder);

}

/******************************************************************************
 *
 ******************************************************************************/
status_t
FeatureFlowControl::
setParameters()
{
    FUNC_START;
    //
    pauseSwitchModeFlow();
    //
    status_t ret = OK;
    if( mbNeedHighQualityZoom )
    {
        runHighQualityZoomFlow();
    }
    //
    ret = (mpRequestController != 0) ?
        mpRequestController->setParameters( this ) : OK;
    //
    resumeSwitchModeFlow();
    //
    FUNC_END;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
FeatureFlowControl::
sendCommand(
    int32_t cmd,
    int32_t arg1,
    int32_t arg2
)
{
    FUNC_START;
    Mutex::Autolock _l(sPreviewLock);
    //
    MY_LOGD("cmd(0x%08X),arg1(0x%08X),arg2(0x%08X)", cmd, arg1, arg2);
    pauseSwitchModeFlow();
    //
    if( cmd == CAMERA_CMD_ENABLE_RAW16_CALLBACK)
    {
        CAM_LOGD("[defaultflowcontrol:sendCommand] CAMERA_CMD_ENABLE_RAW16_CALLBACK (%d)\n", arg1);
        mEnDng = arg1;
    }

    // arg2 should be 0 if send from AP, hence we check arg2, it's supposed to be 1
    // because we assume eExtCmd_setLtmEnable is always sent from middleware not AP
    if (cmd == IFlowControl::eExtCmd_setLtmEnable && arg2 != 0)
    {
        mEnLtm = arg1;
        CAM_LOGD("[defaultflowcontrol:sendCommand] set LTM enable to %d", mEnLtm);
        //
        resumeSwitchModeFlow();
        //
        FUNC_END;
        return OK; // no need to pass
    }
    //
    status_t st = OK;
    if(mpRequestController != 0)
    {
        st = mpRequestController->sendCommand( cmd, arg1, arg2 );
    }
    //
    resumeSwitchModeFlow();
    //
    FUNC_END;
    return st;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
FeatureFlowControl::
dump(
    int /*fd*/,
    Vector<String8>const& /*args*/
)
{
    // TODO
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
FeatureFlowControl::
updateParameters(
    IMetadata* setting
)
{
    if (mpParamsManagerV3 != 0)
    {
        if( mLPBConfigParams.mode == LegacyPipelineMode_T::PipelineMode_VideoRecord ||
            mLPBConfigParams.mode == LegacyPipelineMode_T::PipelineMode_HighSpeedVideo ||
            mLPBConfigParams.mode == LegacyPipelineMode_T::PipelineMode_Feature_VideoRecord ||
            mLPBConfigParams.mode == LegacyPipelineMode_T::PipelineMode_Feature_HighSpeedVideo )
        {
            mpParamsManagerV3->updateRequestRecord(setting);
        }else
        {
            mpParamsManagerV3->updateRequestPreview(setting);
        }
    }

    return ( mpParamsManagerV3 != 0 )
                ? mpParamsManagerV3->updateRequest(setting, mSensorParam.mode)
                : UNKNOWN_ERROR;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
FeatureFlowControl::
runRequestFlow(
    IMetadata* appSetting,
    IMetadata* halSetting
)
{
    if(mbSwitchModeEnable &&
       isNeedSwitchMode())
    {
        android::status_t st = OK;
        st = switchMode();
        if(st != OK)
        {
            MY_LOGW("switchMode() return fail:(%d)!!",st);
            return st;
        }
    }
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
FeatureFlowControl::
updateRequestSetting(
    IMetadata* appSetting,
    IMetadata* halSetting
)
{
    CAM_TRACE_NAME("FFC:updateRequestSetting");

    MBOOL isRepeating = true;

    // update app control
    if(mEnDng)
    {
        IMetadata::IEntry entry(MTK_STATISTICS_LENS_SHADING_MAP_MODE);
        entry.push_back(MTK_STATISTICS_LENS_SHADING_MAP_MODE_ON, Type2Type< MUINT8 >());
        appSetting->update(entry.tag(), entry);

        MINT imgo_format = eImgFmt_UNKNOWN;
        MINT rrzo_format = eImgFmt_UNKNOWN;

       // DNG : disable UFO format
        HwInfoHelper helper(mOpenId);
        if( ! helper.updateInfos() )
        {
            MY_LOGE("cannot properly update infos");
            return BAD_VALUE;
        }
        MUINT32 const bitDepth = getPreviewRawBitDepth(helper);

        if(!helper.getImgoFmt(bitDepth, imgo_format, MFALSE))
        {
            MY_LOGE("getImgoFmt return error: getImgoFmt(%d,%d,MFALSE)",bitDepth, imgo_format);
            return BAD_VALUE;
        }
        if(!helper.getRrzoFmt(bitDepth, rrzo_format, MFALSE))
        {
            MY_LOGE("getRrzoFmt return error: getRrzoFmt(%d,%d,MFALSE)",bitDepth, rrzo_format);
            return BAD_VALUE;
        }

        IMetadata::IEntry entry_imgo(MTK_HAL_REQUEST_IMG_IMGO_FORMAT);
        entry_imgo.push_back(imgo_format, Type2Type<MINT32>());
        halSetting->update(entry_imgo.tag(), entry_imgo);

        IMetadata::IEntry entry_rrzo(MTK_HAL_REQUEST_IMG_RRZO_FORMAT);
        entry_rrzo.push_back(rrzo_format, Type2Type<MINT32>());
        halSetting->update(entry_rrzo.tag(), entry_rrzo);

        MY_LOGD("DNG set MTK_STATISTICS_LENS_SHADING_MAP_MODE (ON)");
        isRepeating = false;

    }

#ifdef FEATURE_MODIFY
    {
        MINT32 mode3DNR = MTK_NR_FEATURE_3DNR_MODE_OFF;
        if( ::strcmp(mpParamsManagerV3->getParamsMgr()->getStr(MtkCameraParameters::KEY_3DNR_MODE), "on") == 0 )
        {
            if( mLPBConfigParams.mode != LegacyPipelineMode_T::PipelineMode_Capture &&
                mLPBConfigParams.mode != LegacyPipelineMode_T::PipelineMode_Feature_Capture )
            {
                mode3DNR = MTK_NR_FEATURE_3DNR_MODE_ON;

            }
        }
        IMetadata::IEntry entry(MTK_NR_FEATURE_3DNR_MODE);
        entry.push_back(mode3DNR, Type2Type< MINT32 >());
        appSetting->update(entry.tag(), entry);
        if ( mCurrentMode3DNR != (MUINT32)mode3DNR ) {
            mCurrentMode3DNR = mode3DNR;
            isRepeating = false;
        }
    }
    {
        MUINT8 eis = MTK_CONTROL_VIDEO_STABILIZATION_MODE_OFF;
        if(mpParamsManagerV3->getParamsMgr()->getVideoStabilization())
        {
            eis = MTK_CONTROL_VIDEO_STABILIZATION_MODE_ON;
        }
        IMetadata::IEntry entry(MTK_CONTROL_VIDEO_STABILIZATION_MODE);
        entry.push_back(eis, Type2Type< MUINT8 >());
        appSetting->update(entry.tag(), entry);
        MY_LOGD("update eis : %d", eis);
   }
#endif // FEATURE_MODIFY

    // update hal control
    {
        IMetadata::IEntry entry(MTK_HAL_REQUEST_SENSOR_SIZE);
        entry.push_back(mSensorParam.size, Type2Type< MSize >());
        halSetting->update(entry.tag(), entry);
    }
    // update default HAL settings
    mpParamsManagerV3->updateRequestHal(halSetting,mSensorParam.mode);

    //
    if( mLPBConfigParams.mode == LegacyPipelineMode_T::PipelineMode_ZsdPreview ||
        mLPBConfigParams.mode == LegacyPipelineMode_T::PipelineMode_Feature_ZsdPreview ||
        mNeedDumpIMGO ||
        mNeedDumpRRZO )
    {
        IMetadata::IEntry entry(MTK_HAL_REQUEST_REQUIRE_EXIF);
        entry.push_back(true, Type2Type< MUINT8 >());
        halSetting->update(entry.tag(), entry);
    }

    if( mbLowPowerVssExif &&
        ( mLPBConfigParams.mode == LegacyPipelineMode_T::PipelineMode_VideoRecord ||
          mLPBConfigParams.mode == LegacyPipelineMode_T::PipelineMode_Feature_VideoRecord ) )
    {
        IMetadata::IEntry entry(MTK_HAL_REQUEST_REQUIRE_EXIF);
        entry.push_back(true, Type2Type< MUINT8 >());
        halSetting->update(entry.tag(), entry);
    }

#ifdef FEATURE_MODIFY
    //
#if VHDR_PROFILE_READY
    // update HDR mode to 3A
    HDRMode kHDRMode = mpParamsManagerV3->getParamsMgr()->getHDRMode();
    IMetadata::setEntry<MUINT8>(
        halSetting, MTK_3A_HDR_MODE, static_cast<MUINT8>(kHDRMode));

    // prepare Stream Size
    if( mpPipeline == NULL )
    {
        MY_LOGW("mpPipeline is NULL");
        return UNKNOWN_ERROR;
    }
    if( mpPipeline->queryImageStreamInfo(eSTREAMID_IMAGE_PIPE_RAW_RESIZER) == NULL )
    {
        MY_LOGW("queryImageStreamInfo(eSTREAMID_IMAGE_PIPE_RAW_RESIZER) is NULL");
        return UNKNOWN_ERROR;
    }
    MSize streamSize = mpPipeline->queryImageStreamInfo(eSTREAMID_IMAGE_PIPE_RAW_RESIZER)->getImgSize();

     // Prepare query Feature Streaming ISP Profile
    MUINT8 profile = 0;
    if(getIspProfile(streamSize, kHDRMode, profile))
    {
        IMetadata::IEntry entry(MTK_3A_ISP_PROFILE);
        entry.push_back(profile, Type2Type< MUINT8 >());
        halSetting->update(entry.tag(), entry);
    }

#endif // VHDR_PROFILE_READY
    //

    if( ( mAppCameraMode == MTK_FEATUREPIPE_VIDEO_RECORD || mAppCameraMode == MTK_FEATUREPIPE_VIDEO_STOP ) &&
        ( EIS_MODE_IS_EIS_22_ENABLED(mEisInfo.mode) ||
          EIS_MODE_IS_EIS_25_ENABLED(mEisInfo.mode) ||
          EIS_MODE_IS_EIS_30_ENABLED(mEisInfo.mode) ) )
    {
        MSize vdoSize;
        mpParamsManagerV3->getParamsMgr()->getVideoSize(&vdoSize.w, &vdoSize.h);
        {
            IMetadata::IEntry entry(MTK_EIS_VIDEO_SIZE);
            entry.push_back(
                    vdoSize,
                    Type2Type< MSize >());
            halSetting->update(entry.tag(), entry);
        }
        if( EIS_MODE_IS_EIS_QUEUE_ENABLED(mEisInfo.mode) )
        {
            MUINT32 isEISQueue = mpParamsManagerV3->getParamsMgr()->getInt(MtkCameraParameters::KEY_EIS25_MODE);
            if( isEISQueue == 0)
            {
                mAppCameraMode = MTK_FEATUREPIPE_VIDEO_STOP;
            }
        }
    }
    {
        IMetadata::IEntry entry(MTK_FEATUREPIPE_APP_MODE);
        entry.push_back(mAppCameraMode, Type2Type< MINT32 >());
        halSetting->update(entry.tag(), entry);
    }

#endif // FEATURE_MODIFY

    mpParamsManagerV3->updateBasedOnScenario(appSetting, isRepeating);

    if ( !isRepeating ) {
        IMetadata::IEntry entry(MTK_HAL_REQUEST_REPEAT);
        entry.push_back(isRepeating, Type2Type< MUINT8 >());
        halSetting->update(entry.tag(), entry);
    }

    //Inform P1Node we want pure raw for normal case
    {
        IMetadata::IEntry entry(MTK_P1NODE_RAW_TYPE);
        entry.push_back(1, Type2Type< int >());
        halSetting->update(entry.tag(), entry);
    }

#if DUAL_CAM_SUPPORT
    {
        if (StereoSettingProvider::isDualCamMode()) {
            sp<ISyncManager> pSyncMgr = ISyncManager::getInstance(getOpenId());
            if (pSyncMgr != NULL) {
                SyncManagerParams syncParams;
                MY_LOGD("update setting from syncMgr");
                // check 4k2k
                MSize paramSize = MSize(0,0);
                mpParamsManagerV3->getParamsMgr()->getVideoSize(&paramSize.w, &paramSize.h);
                /*if (mpParamsManagerV3->getParamsMgr()->getRecordingHint())
                {
                    if (paramSize.w*paramSize.h > IMG_1080P_SIZE)
                        syncParams.mb4KVideo = true;
                }*/
                // zoom ratio
                syncParams.miZoomRatio = mpParamsManagerV3->getZoomRatio();
                // hdr mode
                syncParams.mHDRMode = mpParamsManagerV3->getParamsMgr()->getHDRMode();
                // EIS factor
                syncParams.mEisFactor = mEisInfo.factor;
                // RRZO orginal size
                syncParams.mRRZOSize = mOrignalRRZOSize;
                syncParams.mIsVideoRec = mConstructVideoPipe;
                syncParams.mDofLevel = mpParamsManagerV3->getParamsMgr()->getInt(MtkCameraParameters::KEY_STEREO_DOF_LEVEL);
                // update setting
                pSyncMgr->updateSetting(getOpenId(), appSetting, halSetting, syncParams);

            }
        }
    }
#endif // DUAL_CAM_SUPPORT
    //
    int debugRayType = property_get_int32("debug.camera.raw.type", -1);
    if(debugRayType >= 0)
    {
        MY_LOGD("set debug.camera.raw.type(%d) => MTK_P1NODE_RAW_TYPE(%d)  0:processed-raw 1:pure-raw",debugRayType,debugRayType);
        IMetadata::IEntry entry(MTK_P1NODE_RAW_TYPE);
        entry.push_back(debugRayType, Type2Type< int >());
        halSetting->update(entry.tag(), entry);
    }
    //
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
FeatureFlowControl::
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
    for ( size_t i = 0; i < rvSettings.size(); ++i ) {
        IMetadata::IEntry entry = rvSettings[i].appSetting.entryFor(MTK_CONTROL_CAPTURE_INTENT);
        if( entry.isEmpty() && !rvSettings[i].appSetting.isEmpty() ) {
            MY_LOGD("Does not contain MTK_CONTROL_CAPTURE_INTENT, overwrite it.");
            IMetadata appSetting;
            mpParamsManagerV3->updateRequest( &appSetting, mSensorParam.mode, mRequestType);
            appSetting += rvSettings[i].appSetting;
            *(const_cast<IMetadata*>(&rvSettings[i].appSetting)) = appSetting;
            updateRequestSetting( const_cast<IMetadata*>(&rvSettings[i].appSetting), const_cast<IMetadata*>(&rvSettings[i].halSetting) );
        }
    }
    //
    Vector< BufferList >  vDstStreams;
    for (size_t i = 0; i < rvSettings.size(); ++i) {
        vDstStreams.push_back(rvDstStreams);
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
FeatureFlowControl::
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
    for ( size_t i = 0; i < rvSettings.size(); ++i ) {
        IMetadata::IEntry entry = rvSettings[i].appSetting.entryFor(MTK_CONTROL_CAPTURE_INTENT);
        if( entry.isEmpty() && !rvSettings[i].appSetting.isEmpty() ) {
            MY_LOGD("Does not contain MTK_CONTROL_CAPTURE_INTENT, overwrite it.");
            IMetadata appSetting;
            mpParamsManagerV3->updateRequest( &appSetting, mSensorParam.mode, mRequestType);
            appSetting += rvSettings[i].appSetting;
            *(const_cast<IMetadata*>(&rvSettings[i].appSetting)) = appSetting;
            updateRequestSetting( const_cast<IMetadata*>(&rvSettings[i].appSetting), const_cast<IMetadata*>(&rvSettings[i].halSetting) );
        }
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
FeatureFlowControl::
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
FeatureFlowControl::
onLastStrongRef( const void* /*id*/)
{
    FUNC_START;
    mpRequestController = NULL;
    if ( mpPipeline != 0 ) {
        mpPipeline->flush();
        mpPipeline->waitUntilDrained();
        mpPipeline = NULL;
        mpDeviceHelper = NULL;
    }
    FUNC_END;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
FeatureFlowControl::
selectHighSpeedSensorScen(
    MUINT   /*fps*/,
    MUINT&  sensorScen)
{
    SensorSlimVideoInfo sensorSlimVideoselect;
    SensorSlimVideoInfo sensorSlimVideo[2];
    //
    HwInfoHelper helper(mOpenId);
    if( ! helper.updateInfos() )
    {
        MY_LOGE("cannot properly update infos");
        return BAD_VALUE;
    }
    //
    sensorSlimVideo[0].scenario = SENSOR_SCENARIO_ID_SLIM_VIDEO1;
    sensorSlimVideo[1].scenario = SENSOR_SCENARIO_ID_SLIM_VIDEO2;
    for(MUINT i = 0; i<2; i++)
    {
        helper.getSensorFps(sensorSlimVideo[i].scenario, (MINT32&)sensorSlimVideo[i].fps);
        MY_LOGD("Slim video(%d) FPS(%d)",
                i,
                sensorSlimVideo[i].fps);
    }
    //
    sensorSlimVideoselect.scenario = 0;
    sensorSlimVideoselect.fps = 0;
    for(MUINT i = 0; i<2; i++)
    {
        if(mpParamsManagerV3->getParamsMgr()->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE) <= sensorSlimVideo[i].fps)
        {
            if(sensorSlimVideoselect.fps > 0)
            {
                if(sensorSlimVideoselect.fps > sensorSlimVideo[i].fps)
                {
                    sensorSlimVideoselect.scenario = sensorSlimVideo[i].scenario;
                    sensorSlimVideoselect.fps = sensorSlimVideo[i].fps;
                }
            }
            else
            {
                sensorSlimVideoselect.scenario = sensorSlimVideo[i].scenario;
                sensorSlimVideoselect.fps = sensorSlimVideo[i].fps;
            }
        }
    }
    //
    if(sensorSlimVideoselect.fps > 0)
    {
        MY_LOGD("Use sensor scenario(%d) FPS(%d)",
                sensorSlimVideoselect.scenario,
                sensorSlimVideoselect.fps);
        sensorScen = sensorSlimVideoselect.scenario;
    }
    else
    {
        MY_LOGE("No sensor scenario FPS >= %d",
                mpParamsManagerV3->getParamsMgr()->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE));
        sensorScen = SENSOR_SCENARIO_ID_NORMAL_VIDEO;
        return UNKNOWN_ERROR;
    }
    //
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
FeatureFlowControl::
needReconstructRecordingPipe()
{
    MBOOL ret = MFALSE;
    MSize paramSize;
    mpParamsManagerV3->getParamsMgr()->getVideoSize(&paramSize.w, &paramSize.h);

    if( (paramSize.w*paramSize.h <= IMG_1080P_SIZE && mb4K2KVideoRecord) ||
        (paramSize.w*paramSize.h > IMG_1080P_SIZE && !mb4K2KVideoRecord) )
    {
        ret = MTRUE;
    }
    MY_LOGD("param(%dx%d), b4K2K(%d), ret(%d)",
             paramSize.w, paramSize.h, mb4K2KVideoRecord, ret);
    return ret;
}

MERROR
FeatureFlowControl::
prepareVSSInfo(
    Vector< SettingSet >& vSettings,
    BufferList&           vDstStreams
)
{
    SettingSet tempSetting;
    IMetadata appSetting;
    IMetadata halSetting;

    //update default seting
    if (mpParamsManagerV3 != 0) {
        mpParamsManagerV3->updateRequestRecord(&appSetting);
        mpParamsManagerV3->updateRequest(&appSetting, mSensorParam.mode, CAMERA3_TEMPLATE_VIDEO_SNAPSHOT);
        updateRequestSetting(&appSetting, &halSetting);
    } else {
        MY_LOGE("Fail to update app setting.");
    }

    //update hal seting
    {
        IMetadata::IEntry entry(MTK_HAL_REQUEST_REQUIRE_EXIF);
        entry.push_back(true, Type2Type< MUINT8 >());
        halSetting.update(entry.tag(), entry);
    }
    //

    //update crop info
    {
        MRect reqCropRegion, reqSensorCropRegion, reqPreviewCropRegion, reqSensorPreviewCropRegion;
        MSize sensorSize;
        MSize Margin;
        mpParamsManagerV3->getCropRegion(sensorSize, reqCropRegion, reqSensorCropRegion, reqPreviewCropRegion, reqSensorPreviewCropRegion, Margin);
        //
        {
            IMetadata::IEntry entry(MTK_SCALER_CROP_REGION);
            entry.push_back(
                    reqPreviewCropRegion,
                    Type2Type<MRect>());
            appSetting.update(entry.tag(), entry);
        }
        {
            IMetadata::IEntry entry(MTK_P1NODE_SENSOR_CROP_REGION);
            entry.push_back(
                    reqSensorPreviewCropRegion,
                    Type2Type<MRect>());
            halSetting.update(entry.tag(), entry);
        }
        #if DUAL_CAM_SUPPORT
        {
            IMetadata::IEntry entry(MTK_DUALZOOM_FOV_MARGIN_PIXEL);
            entry.push_back(
                    Margin,
                    Type2Type<MSize>());
            halSetting.update(entry.tag(), entry);
        }
        {
            if (Stereo::E_DUALCAM_FEATURE_ZOOM == StereoSettingProvider::getStereoFeatureMode()) {
                sp<ISyncManager> pSyncMgr = ISyncManager::getInstance(getOpenId());
                if (pSyncMgr != NULL) {
                    SyncManagerParams syncParams;
                    syncParams.mPartialUpdate = MTRUE;
                    // update setting
                    pSyncMgr->updateSetting(getOpenId(), &appSetting, &halSetting, syncParams);

                }
            }
        }
        #endif // DUAL_CAM_SUPPORT
    }
    {
        MINT64 eisPackedInfo = mLPBConfigParams.packedEisInfo;
        IMetadata::IEntry entry(MTK_EIS_INFO);
        entry.push_back(eisPackedInfo, Type2Type< MINT64 >());
        halSetting.update(entry.tag(), entry);
    }

    tempSetting.appSetting = appSetting;
    tempSetting.halSetting = halSetting;

    vSettings.push(tempSetting);

    // stream
    prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_RAW_OPAQUE, false);
    prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_RAW_RESIZER, false);
    if(mLPBConfigParams.enableLCS)
        prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_RAW_LCSO, false);
    if(mLPBConfigParams.enableRSS)
        prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_RAW_RSSO, false);
    prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_YUV_00, false);
    prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_YUV_01, false);
    prepare_stream(vDstStreams, eSTREAMID_IMAGE_YUV_FD, false);

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
FeatureFlowControl::
prepareVideoInfo(
    Vector< SettingSet >& vSettings,
    BufferList&           vDstStreams
)
{
    SettingSet tempSetting;
    IMetadata appSetting;
    IMetadata halSetting;

    //update default seting
    if (mpParamsManagerV3 != 0) {
        mpParamsManagerV3->updateRequestRecord(&appSetting);
        mpParamsManagerV3->updateRequest(&appSetting, mSensorParam.mode, CAMERA3_TEMPLATE_VIDEO_RECORD);
        updateRequestSetting(&appSetting, &halSetting);
    } else {
        MY_LOGE("Fail to update app setting.");
    }

    MINT fps = (mpParamsManagerV3->getParamsMgr()->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE) <= static_cast<int>(mSensorParam.fps)) ?
                mpParamsManagerV3->getParamsMgr()->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE) : static_cast<int>(mSensorParam.fps);

    MSize vdoSize;
    mpParamsManagerV3->getParamsMgr()->getVideoSize(&vdoSize.w, &vdoSize.h);

    if(decide_smvr_directlink(vdoSize.w, vdoSize.h, fps))
    {
        MY_LOGD("%dx%d@%d -> direct link",vdoSize.w, vdoSize.h, fps);
        mpParamsManagerV3->getParamsMgr()->set(CameraParameters::KEY_VIDEO_FRAME_FORMAT, MtkCameraParameters::PIXEL_FORMAT_BITSTREAM);
    }
    else
    {
        MY_LOGD("%dx%d@%d -> non direct link",vdoSize.w, vdoSize.h, fps);
        mpParamsManagerV3->getParamsMgr()->set(CameraParameters::KEY_VIDEO_FRAME_FORMAT, CameraParameters::PIXEL_FORMAT_YUV420P);
        fps = 0;
        vdoSize.w = 0;
        vdoSize.h = 0;
    }

    //update hal seting
    {
        IMetadata::IEntry entry(MTK_P2NODE_HIGH_SPEED_VDO_FPS);
        entry.push_back(
                fps,
                Type2Type< MINT32 >());
        halSetting.update(entry.tag(), entry);
    }
    {
        IMetadata::IEntry entry(MTK_P2NODE_HIGH_SPEED_VDO_SIZE);
        entry.push_back(
                vdoSize,
                Type2Type< MSize >());
        halSetting.update(entry.tag(), entry);
    }
    //update crop info
    {
        MRect reqCropRegion, reqSensorCropRegion, reqPreviewCropRegion, reqSensorPreviewCropRegion;
        MSize sensorSize;
        MSize Margin;
        mpParamsManagerV3->getCropRegion(sensorSize, reqCropRegion, reqSensorCropRegion, reqPreviewCropRegion, reqSensorPreviewCropRegion, Margin);
        //
        {
            IMetadata::IEntry entry(MTK_SCALER_CROP_REGION);
            entry.push_back(
                    reqPreviewCropRegion,
                    Type2Type<MRect>());
            appSetting.update(entry.tag(), entry);
        }
        {
            IMetadata::IEntry entry(MTK_P1NODE_SENSOR_CROP_REGION);
            entry.push_back(
                    reqSensorPreviewCropRegion,
                    Type2Type<MRect>());
            halSetting.update(entry.tag(), entry);
        }
        #if DUAL_CAM_SUPPORT
        {
            IMetadata::IEntry entry(MTK_DUALZOOM_FOV_MARGIN_PIXEL);
            entry.push_back(
                    Margin,
                    Type2Type<MSize>());
            halSetting.update(entry.tag(), entry);
        }

        {
            if (Stereo::E_DUALCAM_FEATURE_ZOOM == StereoSettingProvider::getStereoFeatureMode()) {
                sp<ISyncManager> pSyncMgr = ISyncManager::getInstance(getOpenId());
                if (pSyncMgr != NULL) {
                    SyncManagerParams syncParams;
                    syncParams.mPartialUpdate = MTRUE;
                    // update setting
                    pSyncMgr->updateSetting(getOpenId(), &appSetting, &halSetting, syncParams);

                }
            }
        }
        #endif // DUAL_CAM_SUPPORT
    }

    tempSetting.appSetting = appSetting;
    tempSetting.halSetting = halSetting;

    vSettings.push(tempSetting);

    // stream
    prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_RAW_RESIZER, false);
    if(mLPBConfigParams.enableLCS)
        prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_RAW_LCSO, false);
    if(mLPBConfigParams.enableRSS)
        prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_RAW_RSSO, false);
    prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_YUV_00, false);
    prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_YUV_01, false);
    prepare_stream(vDstStreams, eSTREAMID_IMAGE_YUV_FD, false);

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
FeatureFlowControl::
preparePreviewInfo(
    Vector< SettingSet >& vSettings,
    BufferList&           vDstStreams
)
{
    SettingSet tempSetting;
    IMetadata appSetting;
    IMetadata halSetting;

    //update default seting
    if (mpParamsManagerV3 != 0) {
        mpParamsManagerV3->updateRequestPreview(&appSetting);
        mpParamsManagerV3->updateRequest(&appSetting, mSensorParam.mode, CAMERA3_TEMPLATE_PREVIEW);
        updateRequestSetting(&appSetting, &halSetting);
    } else {
        MY_LOGE("Fail to update app setting.");
    }
    //update crop info
    {
        MRect reqCropRegion, reqSensorCropRegion, reqPreviewCropRegion, reqSensorPreviewCropRegion;
        MSize sensorSize;
#if DUAL_CAM_SUPPORT
        int featureMode = StereoSettingProvider::getStereoFeatureMode();
        if(featureMode == (Stereo::E_STEREO_FEATURE_CAPTURE|Stereo::E_STEREO_FEATURE_VSDOF) ||
                featureMode == (Stereo::E_STEREO_FEATURE_MTK_DEPTHMAP)){
            MY_LOGD("VSDOF mode setting crop region and sensor crop manually");
            // get open id to check current flow control is main1 or main2.
            int main1id, main2id;
            ENUM_STEREO_SENSOR sensorType = StereoHAL::eSTEREO_SENSOR_MAIN1;
            StereoSettingProvider::getStereoSensorIndex(main1id, main2id);
            // get scaler crop region
            if(main2id == getOpenId())
            {
                sensorType = StereoHAL::eSTEREO_SENSOR_MAIN2;
            }
            if(!StereoSizeProvider::getInstance()->getPass1ActiveArrayCrop(sensorType, reqPreviewCropRegion)){
                MY_LOGE("can't get active arrayCrop from StereoSizeProvider for eSTEREO_SENSOR_MAIN1");
            }
            // get sensor crop
            bool bRet = MFALSE;
            MUINT32 q_stride;
            NSCam::MSize size;
            bRet = StereoSizeProvider::getInstance()->getPass1Size(
                            sensorType,
                            (EImageFormat)mRRZOFmt,
                            NSImageio::NSIspio::EPortIndex_RRZO,
                            StereoHAL::eSTEREO_SCENARIO_CAPTURE, // in this mode, stereo only support zsd.
                            (MRect&)reqSensorPreviewCropRegion,
                            size,
                            q_stride);
            if(!bRet)
            {
                MY_LOGE("get sensor crop fail");
            }

            MY_LOGD("StereoSizeProvider => active array crop main1(%d,%d,%dx%d), sensor crop(%d,%d,%dx%d)",
                reqPreviewCropRegion.p.x,
                reqPreviewCropRegion.p.y,
                reqPreviewCropRegion.s.w,
                reqPreviewCropRegion.s.h,
                reqSensorPreviewCropRegion.p.x,
                reqSensorPreviewCropRegion.p.y,
                reqSensorPreviewCropRegion.s.w,
                reqSensorPreviewCropRegion.s.h
            );
        }
        else{
            mpParamsManagerV3->getCropRegion(sensorSize, reqCropRegion, reqSensorCropRegion, reqPreviewCropRegion, reqSensorPreviewCropRegion);
        }
#else
        mpParamsManagerV3->getCropRegion(sensorSize, reqCropRegion, reqSensorCropRegion, reqPreviewCropRegion, reqSensorPreviewCropRegion);
#endif
        //
        {
            IMetadata::IEntry entry(MTK_SCALER_CROP_REGION);
            entry.push_back(
                    reqPreviewCropRegion,
                    Type2Type<MRect>());
            appSetting.update(entry.tag(), entry);
        }
        {
            IMetadata::IEntry entry(MTK_P1NODE_SENSOR_CROP_REGION);
            entry.push_back(
                    reqSensorPreviewCropRegion,
                    Type2Type<MRect>());
            halSetting.update(entry.tag(), entry);
        }
    }

    tempSetting.appSetting = appSetting;
    tempSetting.halSetting = halSetting;

    vSettings.push(tempSetting);

    // stream
    HwInfoHelper helper(mOpenId);
    if( ! helper.updateInfos() )
    {
        MY_LOGE("cannot properly update infos");
        return BAD_VALUE;
    }
    if( !helper.hasRrzo() )
    {
        prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_RAW_OPAQUE, false);
    }
    else
    {
        prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_RAW_RESIZER, false);
    }
    if(mLPBConfigParams.enableLCS)
        prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_RAW_LCSO, false);
#if DUAL_CAM_SUPPORT
    if(StereoSettingProvider::isDualCamMode())
    {
        int main1id, main2id;
        StereoSettingProvider::getStereoSensorIndex(main1id, main2id);
        if(getOpenId() == main1id)
        {
            int featureMode = StereoSettingProvider::getStereoFeatureMode();
            if(featureMode == (Stereo::E_STEREO_FEATURE_CAPTURE|Stereo::E_STEREO_FEATURE_VSDOF))
            {
                prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_DMBGYUV, false);
            }
            else if(featureMode == (Stereo::E_STEREO_FEATURE_MTK_DEPTHMAP))
            {
                prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_DEPTHMAPYUV, false);
            }
            prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_DEPTHMAPNODE_MAINIMAGEYUV, false);
            prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_YUV_00, false);
            prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_YUV_01, false);
        }
    }
    else
    {
        prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_YUV_00, false);
        prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_YUV_01, false);
    }
#else
    prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_YUV_00, false);
    prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_YUV_01, false);
#endif

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
FeatureFlowControl::
getIspProfile(const MSize     &streamSize,
               const HDRMode   hdrMode,
                     MUINT8   &outProfile)
{
    // Prepare query Feature Streaming ISP Profile
    MINT32 fMask = ProfileParam::FMASK_NONE;

    if ((hdrMode == HDRMode::AUTO) || (hdrMode == HDRMode::VIDEO_AUTO))
        fMask |= ProfileParam::FMASK_AUTO_HDR_ON;
    //
    if (!mDisableEISProfile)
    {
        if( EIS_MODE_IS_EIS_12_ENABLED(mEisInfo.mode) ||
            EIS_MODE_IS_EIS_22_ENABLED(mEisInfo.mode) ||
            EIS_MODE_IS_EIS_25_ENABLED(mEisInfo.mode) ||
            EIS_MODE_IS_EIS_30_ENABLED(mEisInfo.mode) )
        {
            fMask |= ProfileParam::FMASK_EIS_ON;
        }
    }

    //
    ProfileParam profileParam(
        streamSize,
        mSensorParam.vhdrMode,
        mSensorParam.mode,
        ProfileParam::FLAG_NONE,
        fMask
    );

    if(mbVHDRUseImgoPreview == MTRUE)
    {
        profileParam.flag |= ProfileParam::FLAG_PURE_RAW_STREAM;
    }
    if (FeatureProfileHelper::getStreamingProf(outProfile, profileParam))
    {
        return MTRUE;
    }
    return MFALSE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
FeatureFlowControl::
isNeedSwitchMode()
{
    //
    MY_LOGD("SwitchModeFlow: %s before switchSensorModeLock (+)",__FUNCTION__);
    Mutex::Autolock _l(mSwitchModeLock);
    MY_LOGD("SwitchModeFlow: %s after switchSensorModeLock (-)",__FUNCTION__);
   //
    if(!isHdrUiOn())
    {
        MY_LOGD("isHdrUiOn == false, don't switch mode");
        return MFALSE;
    }
    //
    if(mbPauseSwitchModeFlow)
    {
        MY_LOGD("SwitchModeFlow: In Pause SwitchModeFlow",__FUNCTION__);
        return MFALSE;
    }
    //
    if ( mpParamsManagerV3->getParamsMgr()->getRecordingHint() )
    {
        MY_LOGD("SwitchModeFlow: Has RecordingHint no need SwitchModeFlow",__FUNCTION__);
        return MFALSE;
    }
    else if ( PARAMSMANAGER_MAP_INST(eMapAppMode)->stringFor(mpParamsManagerV3->getParamsMgr()->getHalAppMode())
                ==  MtkCameraParameters::APP_MODE_NAME_MTK_ZSD)
    {
        MY_LOGD("SwitchModeFlow: ZSD mode need SwitchModeFlow",__FUNCTION__);
    }
    else
    {
        MY_LOGD("SwitchModeFlow: Non-ZSD mode no need SwitchModeFlow",__FUNCTION__);
        return MFALSE;
    }
    // get LV/P-line infomation
    float fLv = -999.0f;
    ASDInfo_T ASDInfo;
    int iIsAePlineMaxStable = 0;
    int iIsoThresholdStable1 = 0;   //for low iso (ex: 2800)
    int iIsoThresholdStable2 = 0;   //for high iso (ex: 5600)
    int iIspAEPlineMinIndexStable = 0;
    if (mpHal3a)
    {
        mpHal3a->send3ACtrl(E3ACtrl_GetAsdInfo, (MINTPTR)(&ASDInfo), 0);
        ////////////////////////////////////////////////////////////////
        // iIsoThresholdStable1:
        //0:bigger than low iso threshold (ex:2800) & stable
        //1:smaller than low iso threshold (ex:2800) & stable
        //2:unstable
        //0:bigger than high iso threshold (ex:5600) & stable
        //1:smaller than high iso threshold (ex:5600) & stable
        //2:unstable

        mpHal3a->send3ACtrl(E3ACtrl_GetISOThresStatus, (MINTPTR)&iIsoThresholdStable1, (MINTPTR)&iIsoThresholdStable2);
        fLv = (float)ASDInfo.i4AELv_x10 / 10;
    }
    else
    {
        MY_LOGW("mpHal3A is NULL, can't get LV info, isNeedSwitchMode will return false");
        return MFALSE;
    }
    //
#if 0
    //for debug, to fix the LV to trigger switch mode
    int debugVal = ::property_get_int32("vendor.debug.simu.light.env", -1);
    if(debugVal > 0)
    {
        MY_LOGD("forced to simulate the enviroment : High Light");
        fLv = 99.0f;
    }
    else if(debugVal == 0)
    {
        MY_LOGD("forced to simulate the enviroment : Low Light");
        fLv = -99.0f;
    }
    //for debug, every xxx frame to trigger switch mode
    debugVal = ::property_get_int32("vendor.debug.simu.light.period", -1);
    static int debugPeriodCount = 0;
    static float debugPeriodLv = 99.0f;
    if(debugVal > 0)
    {
        debugPeriodCount++;
        if(debugPeriodCount >= debugVal)
        {
            debugPeriodLv*=(-1);
            debugPeriodCount = 0;
        }
        fLv = debugPeriodLv;
        MY_LOGD("forced to auto switch mode debugVal:%d debugPeriodCount:%d fLv:%.1f",debugVal,debugPeriodCount,fLv);
    }
    else
    {
        debugPeriodCount = 0;
    }
#endif
    //
    MY_LOGD("SwitchModeFlow: LV:%.1f mSwitchModeStatus:%d isoThresholdStable(%d,%d)",fLv,mSwitchModeStatus,iIsoThresholdStable1,iIsoThresholdStable2);


    ////////////////////////////////////
    // switch mode condition:
    // only zsd scenario
    // /////////////////////////////////
    MBOOL enableSwitchSensorMode = ((mLPBConfigParams.mode == LegacyPipelineMode_T::PipelineMode_ZsdPreview ||
                                    mLPBConfigParams.mode == LegacyPipelineMode_T::PipelineMode_Feature_ZsdPreview) /*&&
                                   (mpParamsManagerV3->getZoomRatio() == 100*/);

    //for debug, to fix the enableSwitchSensorMode to trigger switch sensor mode
    int debugVal = ::property_get_int32("vendor.debug.simu.switchSensorMode.enable", -1); // 1: enable, 2: disable
    if(debugVal > 0)
    {
        MY_LOGD("forced to enable switch sensor mode");
        enableSwitchSensorMode = true;
    }
    else if(debugVal == 0)
    {
        MY_LOGD("forced to disable switch sensor mode");
        enableSwitchSensorMode = false;
    }
    //
    if(!enableSwitchSensorMode) mSwitchModeStatus = eSwitchMode_Undefined;
    if(mSwitchModeStatus == eSwitchMode_Undefined)
    {
        if(enableSwitchSensorMode)
        {
            mSwitchModeStatus = eSwitchMode_HighLightMode;
        }
    }
    else if(mSwitchModeStatus == eSwitchMode_LowLightLvMode)
    {
        // when 3HDR feature on (from AP setting)
        // original enviroment : low light (high iso) (binning mode)
        // new enviroment : high light (low iso) and stable
        // need to change to 3hdr mode (from binning mode)
        // if iIsoThresholdStable1 == 1: now iso smaller than ISO 2800 & stable
        if(iIsoThresholdStable1 == 1) //orginal:binning mode, so need to check iIsoThresholdStable1 for ISO5600 (1:smaller than 5600)
        {
            mSwitchModeStatus = eSwitchMode_HighLightMode;
            if(SENSOR_SCENARIO_ID_NORMAL_CAPTURE != mSensorParam.mode ) { // not in Capture sensor mode, need switch
                MY_LOGD("eSwitchMode_HighLightMode && not SENSOR_SCENARIO_ID_NORMAL_CAPTURE, need switch mode to (Capture Mode)");                
                return MTRUE;
            }
        }
#if 0
        if(fLv >= mfSwitchModeEnterHighLvTh)
        {
            mSwitchModeStatus = eSwitchMode_HighLightMode; // before is high iso, now is low iso, need return true to switch mode
            if(SENSOR_SCENARIO_ID_NORMAL_CAPTURE != mSensorParam.mode)
            {
                MY_LOGD("LV:%.1f, eSwitchMode_HighLightMode && not SENSOR_SCENARIO_ID_NORMAL_CAPTURE, need switch mode to (Capture Mode)", fLv);
                return MTRUE;

            }
        }
#endif
    }
    else if(mSwitchModeStatus == eSwitchMode_HighLightMode)
    {
        // when 3HDR feature on (from AP setting)
        // original enviroment : high light (low iso) (binning mode)
        // new enviroment : low light (high iso) and stable
        // need to change to binning mode (from 3hdr mode)
        // if iIsoThresholdStable2 == 0: now iso bigger than ISO 5600 & stable
        if(iIsoThresholdStable2 == 0) //orginal:3hdr mode, so need to check iIsoThresholdStable2 for ISO5600
        {
            mSwitchModeStatus = eSwitchMode_LowLightLvMode;
            if(SENSOR_SCENARIO_ID_NORMAL_PREVIEW != mSensorParam.mode ) { // not in Preview sensor mode, need switch
                MY_LOGD("eSwitchMode_LowLightLvMode && not SENSOR_SCENARIO_ID_NORMAL_PREVIEW, need switch mode to (Preview Mode)"); 
                return MTRUE;
            }
        }
#if 0
        if(fLv <= mfSwitchModeEnterLowLvTh)
        {
            mSwitchModeStatus = eSwitchMode_LowLightLvMode; // before is low iso, now is high iso, need return true to switch mode
            if(SENSOR_SCENARIO_ID_NORMAL_PREVIEW != mSensorParam.mode)
            {
                MY_LOGD("LV:%.1f, eSwitchMode_LowLightLvMode && not SENSOR_SCENARIO_ID_NORMAL_PREVIEW, need switch mode to (Preview Mode)", fLv);
                return MTRUE;
            }
        }
#endif
    }
    else
    {
        MY_LOGW("unknown mSwitchModeStatus:%d, isNeedSwitchMode() will return false",mSwitchModeStatus);
        return MFALSE;
    }
    //
    return MFALSE;
}

/******************************************************************************
*
*******************************************************************************/
android::status_t
FeatureFlowControl::
switchMode()
{
    MY_LOGD("SwitchModeFlow: %s before switchSensorModeLock (+)",__FUNCTION__);
    Mutex::Autolock _l(mSwitchModeLock);
    MY_LOGD("SwitchModeFlow: %s after switchSensorModeLock (-)",__FUNCTION__);
    //
    mbConstruct2ndPipeline = MTRUE; //
    if(mSwitchModeStatus == eSwitchMode_LowLightLvMode)
    {
        MY_LOGD("eSwitchMode_LowLightLvMode will set eSensorModeStatus_Binning");
        setNowSensorModeStatusForSwitchFlow(eSensorModeStatus_Binning);
    }
    else if(mSwitchModeStatus == eSwitchMode_HighLightMode)
    {
        MY_LOGD("eSwitchMode_HighLightMode will set eSensorModeStatus_3HDR");
        setNowSensorModeStatusForSwitchFlow(eSensorModeStatus_3HDR);
    }
    else
    {
        MY_LOGW("mSwitchModeStatus is eSwitchMode_Undefined, don't switch mode!");
        return BAD_VALUE;
    }
    //
    //
    if(mpCpuCtrl)
    {
        mpCpuCtrl->cpuPerformanceMode(1);
    }
    //
    std::vector< std::future<MERROR> > vFutures;
    {
        vFutures.push_back(
            std::async(std::launch::async,
                [ this ]() {
                    MY_LOGD("Ready to constructZsdPreviewPipeline (mpSecondPipeline)");
                    return constructZsdPreviewPipeline();
                }
            )
        );
    }
    //
    MY_LOGD("Ready to stopPipeline");
    //
    mpRequestController->stopPipeline();
    //
    if ( mpPipeline != 0 ) {
        mpPipeline->flush();
        mpPipeline->waitUntilDrained();
        mpPipeline = NULL;
    }
    //
    for( auto &fut : vFutures ) {
        MY_LOGD("Ready to wait constructZsdPreviewPipeline done (+)");
        MERROR result = fut.get();
    }
    MY_LOGD("Ready to wait constructZsdPreviewPipeline done (-)");
    //
    mpPipeline = mpSecondPipeline;
    mpSecondPipeline = NULL;
    //
#if MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT
    mFutureExecuteCreateCapPipeline = std::async(std::launch::async, [this]()->void
    {
        mpCommonCapturePipeline = ICommonCapturePipeline::createCommonCapturePipeline(getOpenId(), mpParamsManagerV3);
    });
#endif
    //
    if ( mpPipeline == 0 ) {
        MY_LOGE("Cannot get pipeline. switchMode fail, can't start preview.");
        //
        mpDeviceHelper = NULL;
        mbConstruct2ndPipeline = MFALSE;
        //
        MY_LOGD("-");
        return BAD_VALUE;
    }
    //
    mpParamsManagerV3->setScenario(IParamsManagerV3::eScenario::eScenario_Camera_Preview);
    mRequestType = MTK_CONTROL_CAPTURE_INTENT_PREVIEW;
    mpRequestController->setRequestType(mRequestType);
    mAppCameraMode = MTK_FEATUREPIPE_PHOTO_PREVIEW;
    //
    mpParamsManagerV3->setPreviewMaxFps(
                            (mpParamsManagerV3->getParamsMgr()->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE) <= static_cast<int>(mSensorParam.fps)) ?
                            mpParamsManagerV3->getParamsMgr()->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE) : static_cast<int>(mSensorParam.fps));
    //
    if(mbNeedInitRequest)//if(helper.isYuv() && mbNeedInitRequest)
    {
        Vector< SettingSet > vSettings;
        BufferList           vDstStreams;
        Vector< MINT32 >     vRequestNo;
        for(int i=0; i<mInitRequest; i++)
        {
            preparePreviewInfo(vSettings,vDstStreams);
        }
        mpRequestController->setDummpyRequest( vSettings, vDstStreams, vRequestNo );
    }
    //
    MERROR ret = mpRequestController->startPipeline(
                                    0,
                                    1000,
                                    mpPipeline,
                                    this,
                                    mLPBConfigParams.mode,
                                    mpRequestThreadLoopCnt,
                                    &mLPBConfigParams);
    if(ret != OK)
    {
        MY_LOGE("startPipeline fail!");
        //
        mpDeviceHelper = NULL;
        mbConstruct2ndPipeline = MFALSE;
        //
        MY_LOGD("-");
        return UNKNOWN_ERROR;
    }
    //
    mbConstruct2ndPipeline = MFALSE;
    //
    MY_LOGD("-");
    return OK;
}

/******************************************************************************
*
*******************************************************************************/
android::status_t
FeatureFlowControl::
pausePreviewP1NodeFlow()
{
    FUNC_START;
    Mutex::Autolock _l(sPreviewLock);
    //
    pauseSwitchModeFlow();
    //
    if( mpRequestController != 0 )
    {
        Vector< BufferSet > vDstStreams;
        mpRequestController->pausePipeline(vDstStreams);
    }
    //
    MY_LOGD("create thread to pause pipeline +");
    //
    if( pthread_create(&mstartP1NodeFlushThreadHandle, NULL, startP1NodeFlushThread, this) != 0 )
    {
        ALOGE("startP1NodeFlushThread pthread create failed");
    }
    //
    MY_LOGD("create thread to pause pipeline -");
    //
    resumeSwitchModeFlow();
    //
    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
FeatureFlowControl::
isHdrUiOn()
{
//customerized code:
//the define is only for tc16sp branch customer
#ifdef VENDOR_EDIT
    String8 sCaptureMode = mpParamsManagerV3->getParamsMgr()->getStr(MtkCameraParameters::KEY_CAPTURE_MODE);
    const char *pCaptureMode = sCaptureMode.string();
    const int autoHdr = mpParamsManagerV3->getParamsMgr()->getInt(MtkCameraParameters::KEY_AUTO_HDR_MODE);
    if(0 == ::strcmp(pCaptureMode, MtkCameraParameters::CAPTURE_MODE_CUSTOM_HDR) || autoHdr == 1)
    {
        MY_LOGD("Customer Vhdr Ui On");
        return MTRUE;
    }
    else
    {
        MY_LOGD("Customer Vhdr Ui Off");
        return MFALSE;
    }
#endif
    return MFALSE;
}


/******************************************************************************
*
*******************************************************************************/
android::status_t
FeatureFlowControl::
resumePreviewP1NodeFlow()
{
    FUNC_START;
    Mutex::Autolock _l(sPreviewLock);
    //
    pauseSwitchModeFlow();
    //
    if( mpRequestController != 0 )
    {
        if(mbNeedInitRequest)
        {
        //
        MY_LOGD("mInitRequest(%d) mbNeedInitRequest(%d)",
            mInitRequest, mbNeedInitRequest);
        //
        Vector< SettingSet > vSettings;
        BufferList           vDstStreams;
        Vector< MINT32 >     vRequestNo;
        for(int i=0; i<mInitRequest-1; i++)
        {
            preparePreviewInfo(vSettings,vDstStreams);
        }
        mpRequestController->setDummpyRequest( vSettings, vDstStreams, vRequestNo );
        }
        //
        mpRequestController->resumePipeline();
    }
    //
    resumeSwitchModeFlow();
    //
    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
void*
FeatureFlowControl::
startP1NodeFlushThread(void* arg)
{
    ::prctl(PR_SET_NAME, (unsigned long)"startP1NodeFlush", 0, 0, 0);
    CAM_LOGD("[FeatureFlowControl::startP1NodeFlushThread] +");
    FeatureFlowControl* pFlowControl = (FeatureFlowControl*)arg;
    //
    if(pFlowControl==NULL || pFlowControl->mpPipeline==NULL)
    {
        CAM_LOGW("[FeatureFlowControl::startP1NodeFlushThread] pFlowControl is NULL or pFlowControl->mpPipeline is NULL");
        pthread_detach( pthread_self() ); // detach self, avoid thread leak
        pthread_exit(NULL);
        return NULL;
    }
    //
    status_t ret = INVALID_OPERATION;
    //
    ret = pFlowControl->mpPipeline->flushNode(eNODEID_P1Node);
    if (ret != OK)
    {
        CAM_LOGE("[FeatureFlowControl::startP1NodeFlushThread] flushNode Fail!");
    }
    ret = pFlowControl->mpPipeline->waitUntilNodeDrained(eNODEID_P1Node);
    if (ret != OK)
    {
        CAM_LOGE("[FeatureFlowControl::startP1NodeFlushThread] waitUntilNodeDrained Fail!");
    }
    //
    pFlowControl->mstartP1NodeFlushThreadHandle = 0;
    CAM_LOGD("[FeatureFlowControl::startP1NodeFlushThread] -");
    pthread_detach( pthread_self() ); // detach self, avoid thread leak
    pthread_exit(NULL);
    return NULL;
}

