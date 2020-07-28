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

#define LOG_TAG "MtkCam/DefaultFlowControl"
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
#include "DefaultFlowControl.h"
#include <mtkcam/pipeline/pipeline/PipelineContext.h>

#include <mtkcam/utils/fwk/MtkCamera.h>

#include <sys/prctl.h>

#include <future>
#include <vector>
#include <hardware/camera3.h>

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
Mutex DefaultFlowControl:: sPreviewLock;

DefaultFlowControl::
DefaultFlowControl(
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
    , mpParamsManagerV3(pParamsManagerV3)
    , mpImgBufProvidersMgr(pImgBufProvidersManager)
    , mpCamMsgCbInfo(pCamMsgCbInfo)
    , mb4K2KVideoRecord(MFALSE)
    , mpDeviceHelper(NULL)
    , mRequestType(MTK_CONTROL_CAPTURE_INTENT_PREVIEW)
    , mLastZoomRatio(100)
    , mbIsLastImgoPreview(MFALSE)
    , mbNeedHighQualityZoom(MFALSE)
    , mEnLtm(0)
#if MTK_CAM_DISPLAY_INIT_REQUEST_FRAME_SUPPORT
        , mbNeedInitRequest(MTRUE)
        , mInitRequest(MTK_CAM_DISPLAY_INIT_REQUEST_FRAME_NUM)
#else
        , mbNeedInitRequest(MFALSE)
        , mInitRequest(0)
#endif
    , mbLowPowerVssExif(MFALSE)
{
    mLPBConfigParams.mode = 0;
    mLPBConfigParams.enableEIS = MFALSE;
    mLPBConfigParams.enableLCS = MFALSE;
    //
    MINT lowPowerVss = ::property_get_int32("debug.camera.lowPowerVSS", MTKCAM_LOW_POWER_VSS_DEFAULT);
    MINT lowPowerVssExif = ::property_get_int32("debug.camera.lowPowerVSS.exif", -1);
    if(lowPowerVssExif >= 0)
    {
        mbLowPowerVssExif = (lowPowerVssExif > 0) ? MTRUE : MFALSE;
    }
    mbLowPowerVssExif = (lowPowerVss > 0) ? mbLowPowerVssExif : MFALSE;
    //
    MY_LOGD("ResourceContainer::getInstance(%d), low power vss(%d), low power vss exif(%d)", mOpenId, lowPowerVss, mbLowPowerVssExif);
    mpResourceContainer = IResourceContainer::getInstance(mOpenId);
    //Default enable Imgo Preview for ZSD preview
    mbNeedHighQualityZoom = (property_get_int32("debug.feature.enableImgoPrv", 1) > 0) ? MTRUE : MFALSE;
    int debugInitReq = property_get_int32("debug.camera.initreq", -1);
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
        int entryHighLvTh = property_get_int32("debug.switch.mode.highlv_x10", 999);
        int entryLowLvTh = property_get_int32("debug.switch.mode.lowlv_x10", -999);
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
}

/******************************************************************************
 *
 ******************************************************************************/
DefaultFlowControl::
~DefaultFlowControl()
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
DefaultFlowControl::
getName()   const
{
    return mName;
}

/******************************************************************************
 *
 ******************************************************************************/
int32_t
DefaultFlowControl::
getOpenId() const
{
    return mOpenId;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
DefaultFlowControl::
startPreview()
{
    FUNC_START;
    Mutex::Autolock _l(sPreviewLock);
    //
    pauseSwitchModeFlow();
    //
    MINT32 lcsOpen = mEnLtm;
    mLPBConfigParams.enableLCS = (lcsOpen > 0); // LCS test

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
    //
    mLPBConfigParams.enableEIS = helper.isYuv() ? MFALSE : mpDeviceHelper->isFirstUsingDevice();
    if ( mpParamsManagerV3->getParamsMgr()->getRecordingHint() )
    {
        constructRecordingPipeline();
        mpParamsManagerV3->setScenario(IParamsManagerV3::eScenario::eScenario_Video_Preview);
        mRequestType = MTK_CONTROL_CAPTURE_INTENT_PREVIEW;
    } else if ( PARAMSMANAGER_MAP_INST(eMapAppMode)->stringFor(mpParamsManagerV3->getParamsMgr()->getHalAppMode())
                ==  MtkCameraParameters::APP_MODE_NAME_MTK_ZSD)
    {
        constructZsdPreviewPipeline();
        mRequestType = MTK_CONTROL_CAPTURE_INTENT_PREVIEW;
        mpParamsManagerV3->setScenario(IParamsManagerV3::eScenario::eScenario_Camera_Preview);
    }
    else
    {
        constructNormalPreviewPipeline();
        mRequestType = MTK_CONTROL_CAPTURE_INTENT_PREVIEW;
        mpParamsManagerV3->setScenario(IParamsManagerV3::eScenario::eScenario_Camera_Preview);
    }
    mpDeviceHelper->configDone();
    mpRequestController->setRequestType(mRequestType);
    //
    mpParamsManagerV3->setPreviewMaxFps(
                            (mpParamsManagerV3->getParamsMgr()->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE) <= mSensorParam.fps) ?
                            mpParamsManagerV3->getParamsMgr()->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE) : mSensorParam.fps);
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
DefaultFlowControl::
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

    for( auto &fut : vFutures ) {
        MERROR result = fut.get();
    }
    mpRequestController = NULL;

    mb4K2KVideoRecord = MFALSE;

    mvwpClientBufferPool.clear();

    resumeSwitchModeFlow();

    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
DefaultFlowControl::
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

    {
        Mutex::Autolock _l(sPreviewLock);
        for ( size_t i = 0; i < mvwpClientBufferPool.size(); ++i )
        {
            sp<ClientBufferPool> pClientBufferPool = mvwpClientBufferPool[i].promote();

            if(pClientBufferPool.get()) {
                pClientBufferPool->enableBuffer(false);
            }
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
DefaultFlowControl::
resumePreview()
{
    FUNC_START;
    //
    pauseSwitchModeFlow();
    //
    {
        Mutex::Autolock _l(sPreviewLock);
        for ( size_t i = 0; i < mvwpClientBufferPool.size(); ++i )
        {
            sp<ClientBufferPool> pClientBufferPool = mvwpClientBufferPool[i].promote();

            if(pClientBufferPool.get()) {
                pClientBufferPool->enableBuffer(true);
            }
        }
    }
    status_t ret = ( mpRequestController == nullptr ) ? UNKNOWN_ERROR :
        mpRequestController->resumePipeline();
    //
    pauseSwitchModeFlow();
    //
    FUNC_END;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
DefaultFlowControl::
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
DefaultFlowControl::
startRecording()
{
    FUNC_START;
    //
    pauseSwitchModeFlow();
    //
    status_t status;
#if 1
    if( ( mLPBConfigParams.mode != LegacyPipelineMode_T::PipelineMode_VideoRecord &&
          mLPBConfigParams.mode != LegacyPipelineMode_T::PipelineMode_HighSpeedVideo ) ||
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
DefaultFlowControl::
stopRecording()
{
    FUNC_START;
    //
    pauseSwitchModeFlow();
    //
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
DefaultFlowControl::
autoFocus()
{
    FUNC_START;
    //
    pauseSwitchModeFlow();
    //
    status_t ret = OK;
    if ( mpRequestController != 0 ) {
        ret = mpRequestController->autoFocus();
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
DefaultFlowControl::
cancelAutoFocus()
{
    FUNC_START;
    //
    pauseSwitchModeFlow();
    //
    status_t ret = OK;
    if ( mpRequestController != 0 ) {
        ret = mpRequestController->cancelAutoFocus();
    }
    else {
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
DefaultFlowControl::
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
DefaultFlowControl::
takePicture()
{
#if 1
    FUNC_START;

    if( mLPBConfigParams.mode == LegacyPipelineMode_T::PipelineMode_VideoRecord )
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
DefaultFlowControl::
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
            }
            mLastZoomRatio = zoomRatio;
        }
    }
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
DefaultFlowControl::
setNewZsdPreviewRequestBuilder(MBOOL useImgoPreview)
{
    MY_LOGD("useImgoPreview = %d", useImgoPreview);
    //
    BufferList vDstStreams;
    prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_RAW_OPAQUE, false);
    prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_RAW_RESIZER, false);
    if(mLPBConfigParams.enableLCS)
        prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_RAW_LCSO, false);
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
DefaultFlowControl::
setParameters()
{
    FUNC_START;
    //
    pauseSwitchModeFlow();
    //
    if( mbNeedHighQualityZoom )
    {
        runHighQualityZoomFlow();
    }
    //
    status_t ret = (mpRequestController != 0) ?
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
DefaultFlowControl::
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
    // the argument "arg2" is NOT 0 if the command is sent from middleware,
    // we assume that the LTM on/off command should be sent from middleware hence
    // we need to check arg2 here
    if (cmd == IFlowControl::eExtCmd_setLtmEnable && arg2 != 0)
    {
        mEnLtm = arg1;
        MY_LOGD("set LTM enable to %d", mEnLtm);
        //
        resumeSwitchModeFlow();
        //
        FUNC_END;
        return OK; // this command is un-necessary to be sent to mpRequestController
    }

    //
    status_t ret = (mpRequestController != 0) ?
        mpRequestController->sendCommand( cmd, arg1, arg2 ) : INVALID_OPERATION;
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
DefaultFlowControl::
dump(
    int /*fd*/,
    Vector<String8>const& /*args*/
)
{
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
DefaultFlowControl::
updateParameters(
    IMetadata* setting
)
{
    if (mpParamsManagerV3 != 0)
    {
        if( mLPBConfigParams.mode == LegacyPipelineMode_T::PipelineMode_VideoRecord ||
            mLPBConfigParams.mode == LegacyPipelineMode_T::PipelineMode_HighSpeedVideo )
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
DefaultFlowControl::
runRequestFlow(
    IMetadata* appSetting,
    IMetadata* halSetting
)
{
    CAM_TRACE_NAME("DFC:runRequestFlow");
    #if 0
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
    #endif
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
DefaultFlowControl::
updateRequestSetting(
    IMetadata* appSetting,
    IMetadata* halSetting
)
{
    CAM_TRACE_NAME("DFC:updateRequestSetting");
    MBOOL isRepeating = true;
    // update app control
    // update hal control
    {
        IMetadata::IEntry entry(MTK_HAL_REQUEST_SENSOR_SIZE);
        entry.push_back(mSensorParam.size, Type2Type< MSize >());
        halSetting->update(entry.tag(), entry);
    }
    // update default HAL settings
    mpParamsManagerV3->updateRequestHal(halSetting, mSensorParam.mode);

    //
    if( mLPBConfigParams.mode == LegacyPipelineMode_T::PipelineMode_ZsdPreview)
    {
        IMetadata::IEntry entry(MTK_HAL_REQUEST_REQUIRE_EXIF);
        entry.push_back(true, Type2Type< MUINT8 >());
        halSetting->update(entry.tag(), entry);
    }
    //
    if( mbLowPowerVssExif && mLPBConfigParams.mode == LegacyPipelineMode_T::PipelineMode_VideoRecord)
    {
        IMetadata::IEntry entry(MTK_HAL_REQUEST_REQUIRE_EXIF);
        entry.push_back(true, Type2Type< MUINT8 >());
        halSetting->update(entry.tag(), entry);
    }

    mpParamsManagerV3->updateBasedOnScenario(appSetting, isRepeating);

    if ( !isRepeating ) {
        IMetadata::IEntry entry(MTK_HAL_REQUEST_REPEAT);
        entry.push_back(isRepeating, Type2Type< MUINT8 >());
        halSetting->update(entry.tag(), entry);
    }

    if (mLPBConfigParams.enableEIS)
    {
        MINT64 eisPackedInfo = mLPBConfigParams.packedEisInfo;
        IMetadata::IEntry entry(MTK_EIS_INFO);
        entry.push_back(eisPackedInfo, Type2Type< MINT64 >());
        halSetting->update(entry.tag(), entry);
    }
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
DefaultFlowControl::
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
DefaultFlowControl::
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
DefaultFlowControl::
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
DefaultFlowControl::
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
    //
    FUNC_END;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
DefaultFlowControl::
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
DefaultFlowControl::
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

/******************************************************************************
 *
 ******************************************************************************/
MERROR
DefaultFlowControl::
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
    //update crop info
    {
        MRect reqCropRegion, reqSensorCropRegion, reqPreviewCropRegion, reqSensorPreviewCropRegion;
        MSize sensorSize;
        mpParamsManagerV3->getCropRegion(sensorSize, reqCropRegion, reqSensorCropRegion, reqPreviewCropRegion, reqSensorPreviewCropRegion);
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
    prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_RAW_OPAQUE, false);
    if( helper.hasRrzo() )
    {
        prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_RAW_RESIZER, false);
    }
     if(mLPBConfigParams.enableLCS)
        prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_RAW_LCSO, false);
    prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_YUV_00, false);
    prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_YUV_01, false);
    prepare_stream(vDstStreams, eSTREAMID_IMAGE_YUV_FD, false);

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
DefaultFlowControl::
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

    MINT fps = (mpParamsManagerV3->getParamsMgr()->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE) <= (MINT)mSensorParam.fps) ?
                mpParamsManagerV3->getParamsMgr()->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE) : mSensorParam.fps;
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
        mpParamsManagerV3->getCropRegion(sensorSize, reqCropRegion, reqSensorCropRegion, reqPreviewCropRegion, reqSensorPreviewCropRegion);
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
    prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_YUV_00, false);
    prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_YUV_01, false);
    prepare_stream(vDstStreams, eSTREAMID_IMAGE_YUV_FD, false);

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
DefaultFlowControl::
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
        mpParamsManagerV3->getCropRegion(sensorSize, reqCropRegion, reqSensorCropRegion, reqPreviewCropRegion, reqSensorPreviewCropRegion);
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
    prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_YUV_00, false);
    prepare_stream(vDstStreams, eSTREAMID_IMAGE_PIPE_YUV_01, false);

    return OK;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
DefaultFlowControl::
isNeedSwitchMode()
{
    //3HDR only be used in feature flow control, so no need to switch mode in default flow control
    return MFALSE;
#if 0

    MY_LOGD("SwitchModeFlow: %s before switchSensorModeLock (+)",__FUNCTION__);
    Mutex::Autolock _l(mSwitchModeLock);
    MY_LOGD("SwitchModeFlow: %s after switchSensorModeLock (-)",__FUNCTION__);
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
    int iIspAEPlineMinIndexStable = 0;
    if (mpHal3a)
    {
        mpHal3a->send3ACtrl(E3ACtrl_GetAsdInfo, (MINTPTR)(&ASDInfo), 0);
        ////////////////////////////////////////////////////////////////
        // light->dark: use first parameter iIsAePlineMaxStable
        // dark->light: use second parameter iIspAEPlineMinIndexStable
        mpHal3a->send3ACtrl(E3ACtrl_GetIsAEPlineIndexMaxStable, (MINTPTR)&iIsAePlineMaxStable, (MINTPTR)&iIspAEPlineMinIndexStable);
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
    int debugVal = ::property_get_int32("debug.simu.light.env", -1);
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
#endif
    //
    MY_LOGD("SwitchModeFlow: LV:%.1f mSwitchModeStatus:%d",fLv,mSwitchModeStatus);

    ////////////////////////////////////
    // switch mode condition:
    // only zsd and non-zoom scenario
    // /////////////////////////////////
    MBOOL enableSwitchSensorMode = (mLPBConfigParams.mode == LegacyPipelineMode_T::PipelineMode_ZsdPreview ||
                                    mLPBConfigParams.mode == LegacyPipelineMode_T::PipelineMode_Feature_ZsdPreview) &&
                                   (mpParamsManagerV3->getZoomRatio() == 100);
    //for debug, to fix the enableSwitchSensorMode to trigger switch sensor mode
    int debugVal = ::property_get_int32("debug.simu.switchSensorMode.enable", -1); // 1: enable, 2: disable
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
        if(iIspAEPlineMinIndexStable)
        {
            mSwitchModeStatus = eSwitchMode_HighLightMode;
            if(SENSOR_SCENARIO_ID_NORMAL_CAPTURE != mSensorParam.mode ) { // not in Capture sensor mode, need switch
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
        if(iIsAePlineMaxStable)
        {
            mSwitchModeStatus = eSwitchMode_LowLightLvMode;
            if(SENSOR_SCENARIO_ID_NORMAL_PREVIEW != mSensorParam.mode ) { // not in Preview sensor mode, need switch
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
#endif
}

/******************************************************************************
*
*******************************************************************************/
android::status_t
DefaultFlowControl::
switchMode()
{
    MY_LOGD("SwitchModeFlow: %s before switchSensorModeLock (+)",__FUNCTION__);
    Mutex::Autolock _l(mSwitchModeLock);
    MY_LOGD("SwitchModeFlow: %s after switchSensorModeLock (-)",__FUNCTION__);
    //
    mbConstruct2ndPipeline = MTRUE;
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
    if ( mpPipeline == 0 ) {
        MY_LOGE("Cannot get pipeline. switchMode fail, can't start preview.");
        mpDeviceHelper = NULL;
        return BAD_VALUE;
    }
    //
    mpParamsManagerV3->setScenario(IParamsManagerV3::eScenario::eScenario_Camera_Preview);
    mRequestType = MTK_CONTROL_CAPTURE_INTENT_PREVIEW;
    mpRequestController->setRequestType(mRequestType);
    //mAppCameraMode = MTK_FEATUREPIPE_PHOTO_PREVIEW;
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
DefaultFlowControl::
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
*******************************************************************************/
android::status_t
DefaultFlowControl::
resumePreviewP1NodeFlow()
{
    FUNC_START;
    Mutex::Autolock _l(sPreviewLock);
    //
    pauseSwitchModeFlow();
    //
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
    if( mpRequestController != 0 )
    {
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
DefaultFlowControl::
startP1NodeFlushThread(void* arg)
{
    ::prctl(PR_SET_NAME, (unsigned long)"startP1NodeFlush", 0, 0, 0);
    CAM_LOGD("[FeatureFlowControl::startP1NodeFlushThread] +");
    DefaultFlowControl* pFlowControl = (DefaultFlowControl*)arg;
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


