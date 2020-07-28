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

#define LOG_TAG "MtkCam/DefaultAdapter"
//

#include "MyUtils.h"
#include <mtkcam/utils/fwk/MtkCamera.h>
//
#include <inc/CamUtils.h>
using namespace android;
using namespace MtkCamUtils;
//
#include <inc/ImgBufProvidersManager.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/middleware/v1/LegacyPipeline/IResourceContainer.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProviderFactory.h>
#include <mtkcam/middleware/v1/IParamsManager.h>
#include <mtkcam/middleware/v1/IParamsManagerV3.h>
#include <mtkcam/middleware/v1/ICamAdapter.h>
//
#if MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT
#include <mtkcam/middleware/v1/ICommonCapturePipeline.h>
#endif
//
#include <inc/BaseCamAdapter.h>
#include <buffer/ClientBufferPool.h>
//
#include "inc/v3/DefaultAdapter.h"
using namespace NSDefaultAdapter;
//
#include <hardware/camera3.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/IResourceContainer.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/std/TypeTraits.h>
#include <mtkcam/utils/hw/JobMonitor.h>
using namespace NSCam::Utils;

#include <mtkcam/drv/IHalSensor.h>

// MFNR related
#if MTKCAM_HAVE_MFB_SUPPORT
#if MTKCAM_HAVE_IVENDOR_SUPPORT
#include <mtkcam/pipeline/extension/MFNR.h>
using NSCam::plugin::MFNRVendor;
#else
#include <mtkcam/pipeline/hwnode/MfllNode.h>
using NSCam::v3::MfllNode;
#endif // MTKCAM_HAVE_IVENDOR_SUPPORT
#endif // MTKCAM_HAVE_MFB_SUPPORT

#include <mtkcam/feature/hdrDetection/Defs.h>

#if (MTKCAM_HAVE_MTKSTEREO_SUPPORT == 1 || MTK_CAM_STEREO_DENOISE_SUPPORT == 1 || MTKCAM_HAVE_DUAL_ZOOM_FUSION_SUPPORT == 1 || MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1)
#include <mtkcam/feature/DualCam/DualCam.Common.h>
#include <mtkcam/feature/DualCam/ISyncManager.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#define DUALCAM_SHOT (1)
#else
#define DUALCAM_SHOT (0)
#endif

#include <mtkcam/feature/effectHalBase/ICallbackClientMgr.h>

//
#include <cutils/properties.h>
//
#include <mtkcam/utils/hw/HwInfoHelper.h>

#include <mtkcam/pipeline/extension/ThirdPartyFeatures.h>
using namespace NSCamHW;
//
/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)
#define FUNC_START                  MY_LOGD("+")
#define FUNC_END                    MY_LOGD("-")



#define ISO_THRESHOLD_4CellSENSOR           (200)  //ISO threshold for single 4cell shot trigger
#define ISO_THRESHOLD_4CellSENSOR_MFLL_ZSD  (800)  //ISO threshold for mfll 4cell zsd preview sensor mode

/******************************************************************************
*   Function Prototype.
*******************************************************************************/
//
bool
createShotInstance(
    sp<IShot>&          rpShot,
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId,
    sp<IParamsManager>  pParamsMgr
);
//

/******************************************************************************
*
*******************************************************************************/
void
CamAdapter::
updateShotMode()
{
    String8 s8ShotMode = getParamsManager()->getShotModeStr();
    uint32_t u4ShotMode = getParamsManager()->getShotMode();
    mb4CellReMosaicCapFlow = MFALSE;
    MY_LOGD("AppMode(%s), ShotMode(%s) before update", msAppMode.string(), s8ShotMode.string());

    if(CC_UNLIKELY(mpStateManager->isState(IState::eState_Recording)))
    {
        s8ShotMode = "VSS";
        u4ShotMode = eShotMode_VideoSnapShot;
    }
    else
    {
        auto getHDRDetectionResult = [this](
                String8& shotModeStr, uint32_t& shotMode, const bool isZSD = false)
        {
            const HDRMode HDR_MODE(getParamsManager()->getHDRMode());
            MINT32 temp = toLiteral(HDRDetectionResult::NONE);
            MY_LOGD("HDR mode(%hhu)", HDR_MODE);
            //AUTO for SWHDR and VIDEO_AUTO for zcHDR
            if (HDR_MODE == HDRMode::AUTO || HDR_MODE == HDRMode::VIDEO_AUTO)
            {
                // get the latest frame information
                sp<IFrameInfo> pFrameInfo =
                    IResourceContainer::getInstance(getOpenId())->queryLatestFrameInfo();
                MY_LOGW_IF((pFrameInfo == nullptr), "Can't query Latest FrameInfo!");

                // get HDR detection result from P1's result metadata
                IMetadata metadata;
                if (pFrameInfo.get())
                    pFrameInfo->getFrameMetadata(
                            eSTREAMID_META_APP_DYNAMIC_P1, metadata);

                IMetadata::getEntry<MINT32>(
                        &metadata, MTK_HDR_FEATURE_HDR_DETECTION_RESULT, temp);

                // NOTE: set any unexpected enumerations received to NONE
                if ((temp <= toLiteral(HDRDetectionResult::NONE)) ||
                    (temp >= toLiteral(HDRDetectionResult::NUM)))
                {
                    MY_LOGW("invalid HDR detection result(%d)", temp);
                    temp = toLiteral(HDRDetectionResult::NONE);
                }
            }

            MY_LOGD("HDR detection result(%d)", temp);
            const String8 NORMAL(MtkCameraParameters::CAPTURE_MODE_NORMAL);
            const String8 HDR(MtkCameraParameters::CAPTURE_MODE_HDR_SHOT);
            const String8 ZSD("ZSD");
            const String8 ZSD_HDR("ZSD HDR");
            switch (static_cast<HDRDetectionResult>(temp))
            {
                case HDRDetectionResult::NORMAL:
                    MY_LOGD("HDR detection result: normal");
                    //For zcHDR (HDR Sensor)
                    if(HDR_MODE == HDRMode::VIDEO_AUTO){
                        shotModeStr = isZSD ? ZSD.string() : HDR.string();
                        shotMode    = isZSD ? eShotMode_ZsdHdrShot : eShotMode_HdrShot;
                    }else{
                        shotModeStr = isZSD ? ZSD.string() : NORMAL.string();
                        shotMode    = isZSD ? eShotMode_ZsdShot : eShotMode_NormalShot;
                    }
                    break;
                case HDRDetectionResult::HDR:
                    MY_LOGD("HDR detection result: HDR");
                    shotModeStr = isZSD ? ZSD_HDR.string() : HDR.string();
                    shotMode    = isZSD ? eShotMode_ZsdHdrShot : eShotMode_HdrShot;
                    break;
                default:
                case HDRDetectionResult::NONE:
                    MY_LOGD("HDR detection is disabled");
                    if (isZSD)
                    {
                        shotModeStr = ZSD_HDR.string();
                        shotMode    = eShotMode_ZsdHdrShot;
                    }
            }
        };

        if (msAppMode == MtkCameraParameters::APP_MODE_NAME_MTK_ZSD
            || msAppMode == MtkCameraParameters::APP_MODE_NAME_MTK_STEREO)
        {
            switch (u4ShotMode)
            {
                case eShotMode_ContinuousShot:
                    s8ShotMode = "ZSD CShot";
                    u4ShotMode = eShotMode_ContinuousShot;
                    break;
                case eShotMode_HdrShot:
                    if(mbIs4CellMVHDR)
                    {
                        getHDRDetectionResult(s8ShotMode, u4ShotMode, false);
                    }
                    else
                    {
                        getHDRDetectionResult(s8ShotMode, u4ShotMode, true);
                    }
                    if(mpFlowControl->getNowSensorModeStatusForSwitchFlow()==IFlowControl::eSensorModeStatus_Binning)
                    {
                        s8ShotMode = "ZSD";
                        u4ShotMode  = NSCam::eShotMode_ZsdShot;
                        MY_LOGD("eSensorModeStatus_Binning, set eShotMode_HdrShot => eShotMode_ZsdShot");
                    }
                    break;
                default:
                    mbUsingDualShot = isDualCamShot(s8ShotMode, u4ShotMode);

                    if(!mbUsingDualShot)
                    {
                        mb4CellReMosaicCapFlow = is4CellShot(s8ShotMode, u4ShotMode);
                        //
                        if( mpParamsManagerV3->getParamsMgr()->getLowLightCaptureUsePreviewMode() &&
                            !mb4CellReMosaicCapFlow &&
                            mbIsLowLvCaptureEnviroment )
                        {
                            mbIsNeedSwitchModeToCapture = MTRUE;
                            MY_LOGD("low light switch to preview sensor mode capture flow");
                            s8ShotMode = "Normal";
                            u4ShotMode  = NSCam::eShotMode_NormalShot;
                        }

                        if( mbIs4CellMVHDR &&
                            !mb4CellReMosaicCapFlow )
                        {
                            MY_LOGD("ZSD still switch to preview sensor mode capture flow");
                            switch (u4ShotMode)
                            {
                                case NSCam::eShotMode_ZsdShot:
                                    mbIsNeedSwitchModeToCapture = MTRUE;
                                    s8ShotMode = "Normal";
                                    u4ShotMode  = NSCam::eShotMode_NormalShot;
                                    break;
                                case NSCam::eShotMode_ZsdVendorShot:
                                    MY_LOGD("Keep eShotMode_ZsdVendorShot");
                                    // mbIsNeedSwitchModeToCapture = MTRUE;
                                    // Still run Zsd Vendor Shot now
                                    // s8ShotMode = "Normal";
                                    // u4ShotMode  = NSCam::eShotMode_VendorShot;
                                    break;
                                default:
                                    mbIsNeedSwitchModeToCapture = MTRUE;
                                    break;
                                    // do notihing; use the initial values
                            }
                        }
                    }
                    MY_LOGD("[%d] name: (%s) mode: (%d)", getOpenId(), s8ShotMode.string(), u4ShotMode);
            }
        }
        else if (msAppMode == MtkCameraParameters::APP_MODE_NAME_MTK_ENG)
        {
            s8ShotMode = "ENG";
            u4ShotMode = NSCam::eShotMode_EngShot;
        }
        else
        {
            switch (u4ShotMode)
            {
                case eShotMode_HdrShot:
                    getHDRDetectionResult(s8ShotMode, u4ShotMode);
                    break;
                default:
                    break;
                    ;
                    // do notihing; use the initial values
            }
        }
    }
    //
    mShotMode = u4ShotMode;
    MY_LOGI("<shot mode> %#x(%s)", mShotMode, s8ShotMode.string());
    return;
}

/******************************************************************************
*   Before CamAdapter::takePicture() -> IState::onCapture()
*
*   To determine if using Multi Frame Noise Reduction based on the current ISO/EXP,
*   if yes, mShotMode will be updated to
*     1. eShotMode_NormalShot  -->  eShotMode_MfllShot
*     2. eShotMode_ZsdShot     -->  eShotMode_ZsdMfllShot
*******************************************************************************/
void
CamAdapter::
updateShotModeForMultiFrameBlending()
{
#if MTKCAM_HAVE_MFB_SUPPORT
    MY_LOGD("%s", __FUNCTION__);

    int mfbMode = MTK_MFB_MODE_OFF;
    int doMfb = 0; // No need
    int forceMfb = 0; // force MFB
    MERROR err = 0;
    MUINT8 sceneMode = MTK_CONTROL_SCENE_MODE_DISABLED;

#if MTKCAM_HAVE_IVENDOR_SUPPORT
    NSCam::plugin::MFNRVendor::FeatureInfo fo;
    NSCam::plugin::MFNRVendor::ConfigParams params;
#endif
    sp<IParamsManager> pParamsMgr = getParamsManager();

    if ( CC_UNLIKELY(pParamsMgr.get() == nullptr) ) {
        MY_LOGE("get parameter manager failed (it's null), "\
                "%s update failed", __FUNCTION__);
        return;
    }

    // Determine if it's necessary to do Multi-frame Noise Reduction, because
    // MFNR has no scene mode for UI which means, MFNR should be auto truned
    // on while capturing with NormalShot or ZsdShot.
    if (mShotMode == eShotMode_NormalShot
        || mShotMode == eShotMode_HdrShot /* support MFNR for z-zag sensor*/
#if MTKCAM_HAVE_ZSDMFB_SUPPORT
        || mShotMode == eShotMode_ZsdShot
        || mShotMode == eShotMode_ZsdHdrShot /* support MFNR for z-zag sensor*/
#endif
        || mShotMode == eShotMode_EngShot
        || mShotMode == eShotMode_AsdShot
        )
    {
    int enableAEB = (property_get_int32("debug.enable.normalAEB", 0));
        if(CC_UNLIKELY(mIsRaw16CBEnable)||(enableAEB)) {
            MY_LOGD("Not supprot MFB if DNG is enabled or enable AEB(%d)", enableAEB);
            doMfb = 0;
        }
        else {
            // get MFB mode from APP
            mfbMode = pParamsMgr->getMultFrameBlending();

#if MTKCAM_HAVE_MFB_BUILDIN_SUPPORT
            if (mfbMode == MTK_MFB_MODE_OFF) {
                mfbMode = MTKCAM_HAVE_MFB_BUILDIN_SUPPORT;
                MY_LOGD("APK set mfbMode to 0, but MTKCAM_HAVE_MFB_BUILDIN_SUPPORT " \
                        "has been set to %d", MTKCAM_HAVE_MFB_BUILDIN_SUPPORT);
            }
#endif

            MY_LOGD("original: mfbMode(%d), sceneMode(%d)", mfbMode, sceneMode);
            // support PLine changing strategy "only" at normal scene.
            // unless add multiple cpature PLine for each capture scene.
            if (mfbMode != MTK_MFB_MODE_OFF &&
                mfbMode != MTK_MFB_MODE_MFLL) {

                // get the latest frame to query scene information
                sp<IFrameInfo> pFrameInfo = IResourceContainer::getInstance(getOpenId())->queryLatestFrameInfo();

                if (CC_UNLIKELY(!pFrameInfo.get())) {
                    MY_LOGW("Can't query Latest FrameInfo!");
                    sceneMode = MTK_CONTROL_SCENE_MODE_DISABLED;
                }
                else {
                    IMetadata metadata;
                    // get scene mode result from P1's result metadata
                    pFrameInfo->getFrameMetadata(eSTREAMID_META_APP_DYNAMIC_P1, metadata);
                    if (CC_UNLIKELY(!IMetadata::getEntry<MUINT8>(&metadata, MTK_CONTROL_SCENE_MODE, sceneMode))) {
                        MY_LOGW("Can't query MTK_CONTROL_SCENE_MODE!");
                        sceneMode = MTK_CONTROL_SCENE_MODE_DISABLED;
                    }
                    if (CC_UNLIKELY(sceneMode >= MTK_CONTROL_SCENE_MODE_NUM)) {
                        MY_LOGW("Invalid sceneMode(%d), over MTK_CONTROL_SCENE_MODE_NUM(%d)", sceneMode, MTK_CONTROL_SCENE_MODE_NUM);
                        sceneMode = MTK_CONTROL_SCENE_MODE_DISABLED;
                    }
                }

                //  PLine Changing only support below scene
                if ((sceneMode != MTK_CONTROL_SCENE_MODE_DISABLED) &&
                    (sceneMode != MTK_CONTROL_SCENE_MODE_NORMAL)) {
                    MY_LOGW("not support changed PLine in current sceneMode(%d)", sceneMode);
                    mfbMode = MTK_MFB_MODE_MFLL; // only support MFLL (no chaning PLine strategy)
                }
            }

#if MTKCAM_HAVE_IVENDOR_SUPPORT
            // zHDR feature combination strategy
            // In EM, we don't need to check if it's zHDR or not, because in EM,
            // zHDR information will be given by EM config
            if (mShotMode == eShotMode_HdrShot ||
                mShotMode == eShotMode_ZsdHdrShot ||
                sceneMode == MTK_CONTROL_SCENE_MODE_HDR) {
                // query from feature table to decide whether
                // single-frame HDR or multi-frame HDR capture is supported
                bool isSingleFrameCaptureHDR = pParamsMgr->getSingleFrameCaptureHDR();
                // query current VHDR mode
                uint32_t VHDRMode = pParamsMgr->getVHdr();
                if (isSingleFrameCaptureHDR && VHDRMode == SENSOR_VHDR_MODE_ZVHDR) {
                    params.isZHDRMode = MTRUE;
                    params.isAutoHDR = (pParamsMgr->getHDRMode() == HDRMode::AUTO) ||
                                       (pParamsMgr->getHDRMode() == HDRMode::VIDEO_AUTO);
                }
                else {
                    MY_LOGD("only support MFB for z-zag HDR sensor while HdrShot");
                    mfbMode = MTK_MFB_MODE_OFF;
                    params.isZHDRMode = MFALSE;
                    params.isAutoHDR = MFALSE;
                }
                MY_LOGD("isSingleFrameCaptureHDR(%d), VHDRMode(0x%X)",
                        isSingleFrameCaptureHDR, VHDRMode);
            }

            if( msAppMode == MtkCameraParameters::APP_MODE_NAME_MTK_ZSD && !mbIs4CellMVHDR ) {
                params.isZSDMode = MTRUE;
            }
            else {
                params.isZSDMode = MFALSE;
            }

            params.realIso = m3ACaptureParamDuringStrategy.u4RealISO;
            params.exposureTime = m3ACaptureParamDuringStrategy.u4Eposuretime;
            params.customHint = pParamsMgr->getCustomHint();
            params.isFlashOn = mbFlashOn;
            HwInfoHelper helper(getOpenId());
            if(helper.get4CellSensorSupported()) {
                // in eShotMode_MfllShot for performance requirement,
                // 4-cell sensor only support preview sensor mode (binnig-output)
                params.sensorMode =  SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
            }
            else if(mbIsNeedSwitchModeToCapture)
            {
                params.sensorMode = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
            }
            else if(mpParamsManagerV3->getParamsMgr()->getDynamicSwitchSensorMode())
            {
                // get the latest frame to query scene information
                sp<IFrameInfo> pFrameInfo = IResourceContainer::getInstance(getOpenId())->queryLatestFrameInfo();

                if (CC_UNLIKELY(!pFrameInfo.get())) {
                    MY_LOGW("Can't query Latest FrameInfo!");
                    params.sensorMode =  SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
                }
                else {
                    MINT32 p1NodeSensorMode =  SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
                    if(!getLastSensorMode(p1NodeSensorMode))
                    {
                        MY_LOGW("getLastSensorMode Fail, p1NodeSensorMode set as SENSOR_SCENARIO_ID_NORMAL_CAPTURE");
                    }
                    params.sensorMode = p1NodeSensorMode;
                    MY_LOGD("getDynamicSwitchSensorMode:MTRUE, MTK_P1NODE_SENSOR_MODE(%d), MFLL capture will use this mode",p1NodeSensorMode);
                }
            }
            else {
                params.sensorMode =  SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
            }

            // if using EM, and mfbMode is not OFF, always use MFLL to mfbMode
            // because we want to know if it's necessary to do MFNR or not.
            if (CC_UNLIKELY( mShotMode == eShotMode_EngShot && mfbMode != MTK_MFB_MODE_OFF )) {
                mfbMode = MTK_MFB_MODE_MFLL;
                MY_LOGD("force set mfbMode to MFNR due to EM");
            }

            fo = MFNRVendor::queryFeatureInfo(getOpenId(), mfbMode, params);
            doMfb = fo.doMfb;
#else
            // get information
            err = MfllNode::getCaptureInfo(mfbMode, doMfb, getOpenId());
            if (err != OK) {
                MY_LOGE("MfllNode::getCaptureInfo returns code %#x, use normal capture", err);
                doMfb = 0;
            }
#endif // MTKCAM_HAVE_IVENDOR_SUPPORT
        }

        MY_LOGD("result: doMfb(%d), mfbMode(%d), sceneMode(%d), mIsRaw16CBEnable(%d), enableAEB(%d)",
                doMfb, mfbMode, sceneMode, mIsRaw16CBEnable, enableAEB);
        // update shot param here
        mi4MfbMode = mfbMode;

        //
        HwInfoHelper helper(getOpenId());
        MUINT32 u4RawFmtType = 0;
        //
        switch (mShotMode)
        {
            case eShotMode_AsdShot:
                if (doMfb == 0)
                    return;
                if( msAppMode != MtkCameraParameters::APP_MODE_NAME_MTK_ZSD )
                {
                    mShotMode = eShotMode_MfllShot;
                    MY_LOGD("<shot mode> ASD update shot mode to MfllShot");
                }
                else
                {
                    mShotMode = eShotMode_ZsdMfllShot;
                    MY_LOGD("<shot mode> ASD update shot mode to ZsdMfllShot");
                }
                break;
            case eShotMode_NormalShot:
            case eShotMode_HdrShot:
                if (doMfb == 0)
                    return;
                //
                mShotMode = eShotMode_MfllShot;
                MY_LOGD("<shot mode> update shot mode to MfllShot");
                break;
            case eShotMode_ZsdHdrShot:
                if( helper.get4CellSensorSupported() &&
                    helper.getSensorRawFmtType(u4RawFmtType) &&
                    u4RawFmtType == SENSOR_RAW_4CELL_HW_BAYER )
                {
                    mShotMode = eShotMode_HdrShot;
                    MY_LOGD("<shot mode> update shot mode to HDRShot for SENSOR_RAW_4CELL_HW_BAYER");
                }
                if( mbIs4CellMVHDR )
                {
                    mShotMode = eShotMode_HdrShot;
                    MY_LOGD("<shot mode> update shot mode to HDRShot for 4Cell MVHDR");
                }
                break;
            case eShotMode_ZsdShot:
                if (doMfb == 0)
                    return;
                //
                if( helper.get4CellSensorSupported() &&
                    helper.getSensorRawFmtType(u4RawFmtType) &&
                    u4RawFmtType==SENSOR_RAW_4CELL_HW_BAYER &&
                    mpFlowControl->getNowSensorModeStatusForSwitchFlow() != IFlowControl::eSensorModeStatus_Binning)
                {
                    // get iso threshold setting to trigger ZSD MFLL (preview sensor mode) for hw remosa' 4cell sensor
                    int debug_iso_threshold_4cell_zsd_mfll = property_get_int32("debug.camera.threshold_4cell_zsd_mfll", -1);
                    int iso_threshold_4cell_zsd_mfll = (debug_iso_threshold_4cell_zsd_mfll >= 0) ? debug_iso_threshold_4cell_zsd_mfll : ISO_THRESHOLD_4CellSENSOR_MFLL_ZSD;
                    // get current real ISO
                    uint32_t currentIso = m3ACaptureParamDuringStrategy.u4RealISO;
                    // get manual ISO
                    sp<IParamsManager> pParamsMgr = getParamsManager();
                    int appIsoSpeed = 0; // auto = 0
                    if (CC_UNLIKELY( pParamsMgr.get() == nullptr )) {
                        MY_LOGE("get pParamsMgr failed, assume ISO speed to AUTO(0)");
                    }
                    else {
                        appIsoSpeed = pParamsMgr->getInt(MtkCameraParameters::KEY_ISO_SPEED);
                    }
                    MY_LOGD("4cell flow mfll condition: iso_threshold_4cell_zsd_mfll(default:%d, debug:%d, apply:%d), currentIso(%d), appIsoSpeed(%d)",
                    ISO_THRESHOLD_4CellSENSOR_MFLL_ZSD, debug_iso_threshold_4cell_zsd_mfll, iso_threshold_4cell_zsd_mfll, currentIso, appIsoSpeed);

                    // check these conditions:
                    //  1. The current ISO < 4cell zsd mfll iso threshold
                    //  2. The manual ISO by App < 4cell zsd mfll iso threshold
                    if ( currentIso < iso_threshold_4cell_zsd_mfll &&
                         appIsoSpeed < iso_threshold_4cell_zsd_mfll )
                    {
                        mShotMode = eShotMode_MfllShot;
                        MY_LOGD("<shot mode> update shot mode to MfllShot for SENSOR_RAW_4CELL_HW_BAYER (normal: capture sensor mode)");
                    }
                    else {
                        mShotMode = eShotMode_ZsdMfllShot;
                        MY_LOGD("<shot mode> update shot mode to ZsdMfllShot for hw SENSOR_RAW_4CELL_HW_BAYER (zsd: preview sensor mode)");
                    }
                }
                else
                {
                    mShotMode = eShotMode_ZsdMfllShot;
                    MY_LOGD("<shot mode> update shot mode to ZsdMfllShot");
                }
                // for 4-cell mVHDR need always use non-ZSD Shot
                if( mShotMode == eShotMode_ZsdMfllShot &&
                    mbIs4CellMVHDR )
                {
                    mShotMode = eShotMode_MfllShot;
                }
                break;

            case eShotMode_EngShot:
                do {
                    // check if MFLL_PICTURE_COUNT, if it's <= 0, if no, break!
                    int _MFLLpc = pParamsMgr->getInt(MtkCameraParameters::KEY_ENG_MFLL_PICTURE_COUNT);
                    if (_MFLLpc > 0)
                        break;

                    MY_LOGD("get MFLL_PICTURE_COUNT = %d", _MFLLpc);

                    // auto MFLL mode only supports if IVENDOR_SUPPORT
#if MTKCAM_HAVE_IVENDOR_SUPPORT
                    _MFLLpc = static_cast<int>(fo.frameCapture); // update MFLL_PICTURE_COUNT
                    // if no need to do MFNR or picture count <= 0, uses single
                    // EM as default
                    if (CC_UNLIKELY( doMfb == 0 || _MFLLpc <= 0 )) {
                        MY_LOGD("MFNR returns no need to do MFNR or frame " \
                                "capture number <= 0, use single EM as default");
                        _MFLLpc = 1;
                    }

                    // We have to check MtkCameraParameters::KEY_MFB_MODE, if it's "off", which
                    // means UI disabled MFNR.
                    bool isDisableMfllByUi =
                        pParamsMgr->getStr(MtkCameraParameters::KEY_MFB_MODE) == MtkCameraParameters::OFF;

                    if (_MFLLpc == 1 || isDisableMfllByUi) {
                        // single frame, do not use EM+MFLL
                        MY_LOGD("EM+MFLL auto defected as single EM");
                        if (isDisableMfllByUi) {
                            MY_LOGD("using single EM due to UI disabled");
                        }
                        else {
                            MY_LOGD("using single EM due to MFNR strategy reported");
                        }
                        pParamsMgr->set(MtkCameraParameters::KEY_MFB_MODE, "");
                        pParamsMgr->set(MtkCameraParameters::KEY_ENG_MFLL_PICTURE_COUNT, 0);
                    }
                    else {
                        pParamsMgr->set(MtkCameraParameters::KEY_MFB_MODE, MtkCameraParameters::KEY_MFB_MODE_MFLL);
                        pParamsMgr->set(MtkCameraParameters::KEY_ENG_MFLL_PICTURE_COUNT, _MFLLpc);
                        MY_LOGD("MFLL_PICTURE_COUNT <= 0 && ENG w/ MFLL, "\
                                "set MFLL_PICTURE_COUNT to %d", _MFLLpc);
                    }
#else
                    MY_LOGW("Auto EM+MFLL only supports if IVENDOR_SUPPORT");
#endif
                } while(0);
                break;
            default:
                MY_LOGE("Not support eShotMode(%d)", mShotMode);
                break;
        }
    }
#else
    MY_LOGI("MTKCAM_HAVE_MFB_SUPPORT has been disabled");
#endif // MTKCAM_HAVE_MFB_SUPPORT
}

/******************************************************************************
*
*******************************************************************************/
bool
CamAdapter::
updateShotInstance(uint32_t shotMode)
{
    CAM_TRACE_CALL();
    MY_LOGI("<shot mode> %#x", shotMode);
    bool bRet = true;
    //
    MINT32 p1NodeSensorMode = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
    if(!getLastSensorMode(p1NodeSensorMode))
    {
        MY_LOGW("getLastSensorMode Fail, p1NodeSensorMode set as SENSOR_SCENARIO_ID_NORMAL_CAPTURE");
    }
    //
    /* check if the shot mode should be managed by IResourceContainer */
    if ([&shotMode]()->bool{
                switch (shotMode) {
                case NSCam::eShotMode_ZsdShot:
                case NSCam::eShotMode_ZsdMfllShot:
                case NSCam::eShotMode_ZsdHdrShot:
#if DUALCAM_SHOT
                case NSCam::eShotMode_BMDNShot:
                case NSCam::eShotMode_MFHRShot:
                case NSCam::eShotMode_DCMFShot:
                case NSCam::eShotMode_DCMFHdrShot:
                case NSCam::eShotMode_FusionShot:
#endif
                    return true;
                default:
                    return false;
                }
            }()
            &&
        msAppMode != MtkCameraParameters::APP_MODE_NAME_MTK_ENG
        )
    {
        /* check if using BackgroundService */
        MBOOL bUsingBackgroundService = isUsingBackgroundService();

        MY_LOGD("Try to get last ZsdShot");
        sp<IPipelineResource> pPipelineReource = IResourceContainer::getInstance(getOpenId())->queryPipelineResource(shotMode);
        if( pPipelineReource != 0 )
        {
            mpShot = pPipelineReource->getShot();
#if MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT
            ICommonCapturePipeline::returnCommonCapturePipeline(getOpenId());
#endif
            /* if using BackgroundService, clear shot instance in Resource Container */
            if (bUsingBackgroundService) {
                pPipelineReource->setShot(nullptr);
            }
        }
        else
        {
            sp<PipelineResource> pPplRes = new PipelineResource();
            IResourceContainer::getInstance(getOpenId())->setPipelineResource(shotMode, pPplRes);
            pPipelineReource = IResourceContainer::getInstance(getOpenId())->queryPipelineResource(shotMode);
        }

#if (MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT == 0)
        // ask to the existed shot instance if it's necessary to be re-created
        auto isNeedToReconstruct = [this]()->bool
        {
            bool result = false; // no need as default
            // if shot instance is null, return yes
            if (mpShot.get() == nullptr)
                return true;
            //
            do {
                // ask shot if it's necessary to be re-created one
                MBOOL _r = MFALSE;
                mpShot->sendCommand(
                        eCmd_getIsNeedNewOne,
                        (MUINTPTR)(long long)(&_r), 0, 0
                        );
                if (_r) {
                    MY_LOGD("re-create SamrtShot due to need it(eCmd_getIsNeedNewOne)");
                }
                result = (_r == MTRUE) ? true : false;
            } while(0);

            return result;
        }
        (); // isNeedToReconstruct
#endif
        //
        static bool bZsdMfllShotNeedBeClearAfterSwitchMode = false;
        static bool bZsdHdrShotNeedBeClearAfterSwitchMode = false;
        bool bAlreadySwitchSensorMode = false;
        if(p1NodeSensorMode != mLastCreateShotSensorMode || mbIs4CellMVHDR)
        {
            bAlreadySwitchSensorMode = true;
            bZsdMfllShotNeedBeClearAfterSwitchMode = true;
            bZsdHdrShotNeedBeClearAfterSwitchMode = true;
            MY_LOGD("bAlreadySwitchSensorMode == true (%d->%d), need to create new Shot Instance",mLastCreateShotSensorMode,p1NodeSensorMode);
        }
        //
        if( mpShot == 0
#if (MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT == 0)
                || isNeedToReconstruct
#endif
#if (MTKCAM_HAVE_IVENDOR_SUPPORT == 0)
                || shotMode == NSCam::eShotMode_ZsdMfllShot
#endif
                || shotMode == NSCam::eShotMode_ZsdHdrShot
                || bAlreadySwitchSensorMode
                || (bZsdMfllShotNeedBeClearAfterSwitchMode && shotMode == NSCam::eShotMode_ZsdMfllShot)
                || (bZsdHdrShotNeedBeClearAfterSwitchMode && shotMode == NSCam::eShotMode_ZsdMfllShot)
          )
        {
            MY_LOGD("Need to create ZsdShot");

            // if the shot instance exists, remove it and re-create one
            if (mpShot.get()) {
#if MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT
                ICommonCapturePipeline::returnCommonCapturePipeline(getOpenId());
#endif
                MY_LOGD("%s: clear the previous shot instance", __FUNCTION__);
                mpShot = NULL;
                if(bZsdMfllShotNeedBeClearAfterSwitchMode && shotMode == NSCam::eShotMode_ZsdMfllShot)
                {
                    bZsdMfllShotNeedBeClearAfterSwitchMode = false;
                    MY_LOGD("Need to create ZsdShot due ZsdMfllShot need be clear after switch mode (even ZsdShot had re-construct)");
                }
                if(bZsdHdrShotNeedBeClearAfterSwitchMode && shotMode == NSCam::eShotMode_ZsdMfllShot)
                {
                    bZsdHdrShotNeedBeClearAfterSwitchMode = false;
                    MY_LOGD("Need to create ZsdShot due ZsdHdrShot need be clear after switch mode (even ZsdShot had re-construct)");
                }
            }
            bRet = createShotInstance(mpShot,
                                        (msAppMode == MtkCameraParameters::APP_MODE_NAME_MTK_ENG)?(NSCam::eShotMode_EngShot):shotMode,
                                        getOpenId(), getParamsManager());

            // only set shot to resource container if non-using BackgroundService
            if (pPipelineReource.get() && ! bUsingBackgroundService) {
                pPipelineReource->setShot(mpShot);
            }
        }
    }
    else
    {
#if MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT
        ICommonCapturePipeline::returnCommonCapturePipeline(getOpenId());
#endif
        IResourceContainer::getInstance(getOpenId())->clearPipelineResource();
        //
        {
            Mutex::Autolock lock(mShotLock);
            bRet = createShotInstance(mpShot,
                                        (msAppMode == MtkCameraParameters::APP_MODE_NAME_MTK_ENG)?(NSCam::eShotMode_EngShot):shotMode,
                                        getOpenId(), getParamsManager());
            mWaitShotLock.signal();
        }
    }
    //
    mLastCreateShotSensorMode = p1NodeSensorMode;
    //
    return bRet;
}

/******************************************************************************
*
*******************************************************************************/
bool
CamAdapter::
isTakingPicture() const
{
    FUNC_START;
    bool ret =  mpStateManager->isState(IState::ENState(IState::eState_PreCapture   |
                                                        IState::eState_NormalCapture|
                                                        IState::eState_VideoSnapshot));
    if  ( ret )
    {
        MY_LOGD("isTakingPicture(1):%s", mpStateManager->getCurrentState()->getName());
    }
    //
    FUNC_END;
    return  ret;
}

/******************************************************************************
*
*******************************************************************************/
bool
CamAdapter::
isSupportVideoSnapshot()
{
    if( mpParamsManagerV3->getParamsMgr()->getStr(MtkCameraParameters::KEY_VIDEO_FRAME_FORMAT) == MtkCameraParameters::PIXEL_FORMAT_BITSTREAM ||
            mpParamsManagerV3->getParamsMgr()->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE) > 30)
    {
        return false;
    }
    else
    {
        return true;
    }
}

/******************************************************************************
* Normal Capture: Preview -> PreCapture->Idle
* ZSD Capture: Preview -> Capture
* Recording capture: Record -> Capture
*******************************************************************************/
status_t
CamAdapter::
takePicture()
{
    FUNC_START;
    //
    mpFlowControl->pauseSwitchModeFlow();
    //
    status_t status = OK;
    //
    get3AInfo();
    //
    mbInTakePicture = MTRUE;
    mbCaptureFinished = MFALSE;
    mbIsNeedSwitchModeToCapture = MFALSE;
    mbIs4CellMVHDR = MFALSE;
    //
    {
        HwInfoHelper helper(getOpenId());
        if( helper.get4CellSensorSupported())
        {
            sp<IParamsManager> pParamsMgr = getParamsManager();
            if ( CC_UNLIKELY(pParamsMgr.get() == nullptr) ) {
                MY_LOGE("get parameter manager failed (it's null), "\
                        "%s update failed", __FUNCTION__);
                goto lbExit;
            }
            if(pParamsMgr->getVHdr() == SENSOR_VHDR_MODE_MVHDR)
            {
                mbIs4CellMVHDR = MTRUE;
            }
        }
    }
    //
    if (isTakingPicture())
    {
        MY_LOGW("already taking picture...");
        goto lbExit;
    }
    //
    updateShotMode();
    //
    MTK3rdParty::setParamsManager( getParamsManager() );
    //
    // State:Recording->VideoSnapShot
    if(mpStateManager->isState(IState::eState_Recording))
    {
        if (isSupportVideoSnapshot())
        {
            status = mpStateManager->getCurrentState()->onCapture(this);
            if (OK != status)
            {
                MY_LOGE("onCapture Fail!");
                goto lbExit;
            }
        }
        else
        {
            MY_LOGE("Not support VSS");
            status = INVALID_OPERATION;
            goto lbExit;
        }
    }
    else
    {
        //  ZSD Capture, direct capture Preview -> TakePicture
        if (msAppMode == MtkCameraParameters::APP_MODE_NAME_MTK_ZSD
            || msAppMode == MtkCameraParameters::APP_MODE_NAME_MTK_STEREO)
        {
            //
            if( mbIsNeedSwitchModeToCapture )
            {
                MY_LOGD("mSetCaptureMaxFpsCount:%d will reset to 0, because need swith to preview mode to capture",mSetCaptureMaxFpsCount);
                mSetCaptureMaxFpsCount = 0;
            }
            else if(mpParamsManagerV3->getParamsMgr()->getHighFpsCapture() &&
               mShotMode != NSCam::eShotMode_ContinuousShot)
            {
                // get the latest frame to query scene information
                sp<IFrameInfo> pFrameInfo = IResourceContainer::getInstance(getOpenId())->queryLatestFrameInfo();
                MINT32 p1NodeSensorMode = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
                if(!getLastSensorMode(p1NodeSensorMode))
                {
                    MY_LOGW("getLastSensorMode Fail, p1NodeSensorMode set as SENSOR_SCENARIO_ID_NORMAL_CAPTURE");
                }
                //
                if(p1NodeSensorMode == SENSOR_SCENARIO_ID_NORMAL_CAPTURE)
                {
                    HwInfoHelper helper(getOpenId());
                    if( ! helper.updateInfos() ) {
                        MY_LOGW("cannot properly update infos");
                    }
                    else
                    {
                        if(mSetCaptureMaxFpsCount==0)
                        {
                            MUINT sensorFps = 0;
                            if(helper.getSensorFps( (MUINT32)p1NodeSensorMode, (MINT32&)sensorFps))
                            {
                                MINT32 minFps = 0;
                                MINT32 maxFps = 0;
                                mpParamsManagerV3->getPreviewFpsRange(minFps,maxFps);
                                mOldMinFps = minFps;
                                mOldMaxFps = maxFps;
#if '1'==MTKCAM_HAVE_3A_HAL
                                IHal3A *hal3A = MAKE_Hal3A(getOpenId(), LOG_TAG);
                                if ( ! hal3A )
                                {
                                    MY_LOGE("hal3A == NULL");
                                    return MFALSE;
                                }
                                else
                                {
                                    hal3A->send3ACtrl( E3ACtrl_SetMinMaxFps, (MINTPTR)(minFps*1000), (MINTPTR)(sensorFps*1000));
                                    hal3A->send3ACtrl( E3ACtrl_SetCaptureMaxFPS, (MINTPTR)(1), (MINTPTR)(sensorFps*1000));
                                    MY_LOGD("E3ACtrl_SetMinMaxFps set (%d,%d)",minFps*1000,sensorFps*1000);
                                    MY_LOGD("E3ACtrl_SetCaptureMaxFPS set (1,%d)",sensorFps*1000);
                                    hal3A->destroyInstance(LOG_TAG);
                                    //
                                    mpParamsManagerV3->setPreviewFpsRange(minFps,maxFps);
                                    mpFlowControl->setParameters();
                                }
#else
                                MY_LOGW("MTKCAM_HAVE_3A_HAL not define, can't set max/min fps to 3A");
#endif
                            }
                            else
                            {
                                MY_LOGW("cannot getSensorFps");
                            }
                            //
                            mSetCaptureMaxFpsCount++;
                        }
                        else
                        {
                            MY_LOGD("mSetCaptureMaxFpsCount:%d ,no need set capture max fps again",mSetCaptureMaxFpsCount);
                        }
                    }
                }
            }
            //
            MY_LOGD("msAppMode == ZSD or Stereo");
            if(!mbUsingDualShot)
             {
                MY_LOGD("onHandlePreCapture");
                status = onHandlePreCapture();
                if (OK != status)
                {
                    MY_LOGE("onPreCapture Fail!");
                    goto lbExit;
                }
            }
            MY_LOGD("MFNR is only applied when flash-mfll(%d), no flash(%d) w/o flash mfll, and no 4-cell-remosaic(%d)",
                    mbFlashMfllSupport, mbFlashOn, mb4CellReMosaicCapFlow);
            //
            if((mbFlashMfllSupport?MTRUE:!mbFlashOn) && !mbUsingDualShot && !mb4CellReMosaicCapFlow )
            {
                CAM_TRACE_BEGIN("updateShotModeForMultiFrameBlending");
                // MFNR is only applied when no flash
                updateShotModeForMultiFrameBlending();
                CAM_TRACE_END();
            }
            //
            // obtain hardware info helper to query if 4 cell sensor is supported
            HwInfoHelper helper(getOpenId());
            const bool bNeedSwitch4CellSensorMode = [&](){
                if (mShotMode == eShotMode_ZsdShot)     return false;
                if (mShotMode == eShotMode_ZsdMfllShot) return false;
                if (mShotMode == eShotMode_ZsdHdrShot)  return false;
                return true;
            }();
            //
            MUINT32 u4RawFmtType;
            helper.getSensorRawFmtType(u4RawFmtType);
            if( (helper.get4CellSensorSupported() && (mb4CellReMosaicCapFlow || u4RawFmtType==SENSOR_RAW_4CELL_HW_BAYER)
                && bNeedSwitch4CellSensorMode)
                || mbIsNeedSwitchModeToCapture)
            {
                //
                status_t ret = INVALID_OPERATION;
                //
                ret = mpFlowControl->stopPreview();
                //ret = mpFlowControl->pausePreviewP1NodeFlow();
                if (ret != OK)
                {
                    MY_LOGD("mpFlowControl->pausePreviewP1NodeFlow() fail");
                }
                //
            }
            //
            status = mpStateManager->getCurrentState()->onCapture(this);
            if(OK != status)
            {
                goto lbExit;
            }
        }
        else // Normal Capture, Preview -> preCapture- > Idle
        {
            status = mpStateManager->getCurrentState()->onPreCapture(this);
            if (OK != status)
            {
                goto lbExit;
            }
            // for manual shutter
#if '1'==MTKCAM_HAVE_3A_HAL
            MY_LOGD("get 3A ExposureParam +");
            IHal3A *hal3A = MAKE_Hal3A(getOpenId(), LOG_TAG);;
            if ( ! hal3A )
            {
                MY_LOGE("hal3A == NULL");
                return MFALSE;
            }
            if (mShotMode == eShotMode_EngShot)
            {
                CaptureParam_T tmpCap3AParam;
                hal3A->send3ACtrl( E3ACtrl_GetExposureParam, reinterpret_cast<MINTPTR>(&tmpCap3AParam), 0);
                //
                mEposuretime  = tmpCap3AParam.u4Eposuretime;
                mAfeGain  = tmpCap3AParam.u4AfeGain;
                mIspGain  = tmpCap3AParam.u4IspGain;
                mRealIso  = tmpCap3AParam.u4RealISO;
                MY_LOGD("Get Capture params from ae_mgr (speed,sensor gain,isp gain)=(%d,%d,%d)",mEposuretime,mAfeGain,mIspGain);
            }
            else
            {
                sp<IParamsManager> pParamsMgr = getParamsManager();
                if(pParamsMgr->getManualCap())
                {
                    AE_EXP_SETTING_T tmpCapInput3AParam, tmpCapOutput3AParam;
                    pParamsMgr->getIsoSpeed(&tmpCapInput3AParam.u4Sensitivity);
                    pParamsMgr->getExposureTime(&tmpCapInput3AParam.u4ExposureTime);
                    tmpCapInput3AParam.u4ExposureTime = tmpCapInput3AParam.u4ExposureTime * 1000; // ms => us
                    hal3A->send3ACtrl( E3ACtrl_GetPrioritySetting, reinterpret_cast<MINTPTR>(&tmpCapInput3AParam), reinterpret_cast<MINTPTR>(&tmpCapOutput3AParam));
                    mEposuretime  = tmpCapOutput3AParam.u4ExposureTime;
                    mRealIso  = tmpCapOutput3AParam.u4Sensitivity;
                    MY_LOGD("Get Capture params from ae_mgr (speed,ISO)=(%d,%d)",mEposuretime,mRealIso);
                }
            }
            //
            if (hal3A != NULL) hal3A->destroyInstance(LOG_TAG);
            MY_LOGD("get 3A ExposureParam -");
#endif
            //
            status = mpStateManager->getCurrentState()->onStopPreview(this);
            if (OK != status)
            {
                goto lbExit;
            }
            // MFNR is only applied when flash-mfll, no flash w/o flash mfll, and no 4-cell-remosaic
            if ( (mbFlashMfllSupport?MTRUE:!mbFlashOn) && !mb4CellReMosaicCapFlow )
                updateShotModeForMultiFrameBlending();
            //
            status = mpStateManager->getCurrentState()->onCapture(this);
            if (OK != status)
            {
                goto lbExit;
            }
        }
    }
    //
lbExit:
    mpFlowControl->resumeSwitchModeFlow();
    //
    mbInTakePicture = MFALSE;
    FUNC_END;
    return status;
}


/******************************************************************************
*
*******************************************************************************/
status_t
CamAdapter::
cancelPicture()
{
    FUNC_START;
    CAM_TRACE_NAME("Adapter:cancelPicture");
    sp<JobMonitor::Helper> monitor = new JobMonitor::Helper(String8(__PRETTY_FUNCTION__), 10*1000);

    if( mpStateManager->isState(IState::ENState(IState::eState_NormalCapture |
                                                IState::eState_ZSLCapture    |
                                                IState::eState_VideoSnapshot)))
    {
        mpStateManager->getCurrentState()->onCancelCapture(this);
    }
    FUNC_END;
    return OK;
}


/******************************************************************************
*
*******************************************************************************/
status_t
CamAdapter::
setCShotSpeed(int32_t i4CShotSpeed)
{
    FUNC_START;
    if(i4CShotSpeed <= 0 )
    {
        MY_LOGE("cannot set continuous shot speed as %d fps)", i4CShotSpeed);
        return BAD_VALUE;
    }

    sp<IShot> pShot = mpShot;
    if( pShot != 0 )
    {
        pShot->sendCommand(eCmd_setCShotSpeed, i4CShotSpeed, 0);
    }
    FUNC_END;
    return OK;
}


/******************************************************************************
*   CamAdapter::takePicture() -> IState::onCapture() ->
*   (Idle)IStateHandler::onHandleNormalCapture() -> CamAdapter::onHandleNormalCapture()
*******************************************************************************/
status_t
CamAdapter::
onHandleNormalCapture()
{
    FUNC_START;
    CAM_TRACE_NAME("Adapter:onHandleNormalCapture");
    status_t status = DEAD_OBJECT;
    sp<ZipImageCallbackThread> pCallbackThread = mpZipCallbackThread.promote();
    sp<ICaptureCmdQueThread> pCaptureCmdQueThread = mpCaptureCmdQueThread;
    //
    if( !pCallbackThread.get() )
    {
        MY_LOGE("no callback thread");
        goto lbExit;
    }
    //
    pCallbackThread->setShotMode(mShotMode, IState::eState_Idle, msAppMode);
    pCallbackThread = 0;
    //
    if  ( pCaptureCmdQueThread != 0 ) {
        status = pCaptureCmdQueThread->onCapture(mShotMode);
    }
    //
lbExit:
    FUNC_END;
    return  status;
}

/******************************************************************************
*
*******************************************************************************/
int
CamAdapter::
getFlashQuickCalibrationResult()
{
    MY_LOGD("+");
    //
    #if '1'==MTKCAM_HAVE_3A_HAL
    //
    IHal3A* pHal3a = MAKE_Hal3A(getOpenId(), getName());

    if ( ! pHal3a )
    {
        MY_LOGE("pHal3a == NULL");
        return 1;
    }

    int result = 0;
    pHal3a->send3ACtrl(NS3Av3::E3ACtrl_GetQuickCalibration, (MINTPTR)&result, 0);

    pHal3a->destroyInstance(getName());
    #endif
    //
    MY_LOGD("-");
    return result;
}


/******************************************************************************
*
*******************************************************************************/
status_t
CamAdapter::
onHandleCaptureDone()
{
    CAM_TRACE_NAME("Adapter:onHandleCaptureDone");
    //
    FUNC_START;
    status_t st = OK;
    if (mpStateManager->isState(IState::eState_NormalCapture))
    {
        st = onHandleNormalCaptureDone();
    }
    else if (mpStateManager->isState(IState::eState_ZSLCapture))
    {
        st = onHandleZSLCaptureDone();
    }
    else if (mpStateManager->isState(IState::eState_VideoSnapshot))
    {
        st = onHandleVideoSnapshotDone();
    }
    //
    if( mpFlowControl.get() != NULL )
    {
        mpFlowControl->sendCommand(StreamingProcessor::eSP_extCmd_resume_focus,0,0);
    }
    //
    if(mpParamsManagerV3->getParamsMgr()->getHighFpsCapture())
    {
        if(mSetCaptureMaxFpsCount > 0)
        {
            MINT32 p1NodeSensorMode = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
            if(!getLastSensorMode(p1NodeSensorMode))
            {
                MY_LOGW("getLastSensorMode Fail, p1NodeSensorMode set as SENSOR_SCENARIO_ID_NORMAL_CAPTURE");
            }
            if(p1NodeSensorMode != SENSOR_SCENARIO_ID_NORMAL_CAPTURE)
            {
                mSetCaptureMaxFpsCount = 0; //reset
            }
            else
            {
                mSetCaptureMaxFpsCount--;
                //
                if(mSetCaptureMaxFpsCount == 0)
                {
#if '1'==MTKCAM_HAVE_3A_HAL
                    IHal3A *hal3A = MAKE_Hal3A(getOpenId(), LOG_TAG);
                    if ( ! hal3A )
                    {
                        MY_LOGE("hal3A == NULL");
                        return MFALSE;
                    }
                    else
                    {
                        hal3A->send3ACtrl( E3ACtrl_SetCaptureMaxFPS, (MINTPTR)(0), (MINTPTR)(mOldMaxFps*1000));
                        hal3A->send3ACtrl( E3ACtrl_SetMinMaxFps, (MINTPTR)(mOldMinFps*1000), (MINTPTR)(mOldMaxFps*1000));
                        MY_LOGD("E3ACtrl_SetCaptureMaxFPS set (0,%d)",mOldMaxFps*1000);
                        MY_LOGD("E3ACtrl_SetMinMaxFps set (%d,%d)",mOldMinFps*1000,mOldMaxFps*1000);
                        hal3A->destroyInstance(LOG_TAG);
                        //
                        mpParamsManagerV3->setPreviewFpsRange(mOldMinFps,mOldMaxFps);
                        mpFlowControl->setParameters();
                    }
#else
                    MY_LOGW("MTKCAM_HAVE_3A_HAL not define, can't set max/min fps to 3A");
#endif
                    MY_LOGD("mSetCaptureMaxFpsCount==0 , need reset capture max fps: (%d,%d)",mOldMinFps,mOldMaxFps);
                }
                else
                {
                    MY_LOGD("mSetCaptureMaxFpsCount:%d , no need reset capture max fps",mSetCaptureMaxFpsCount);
                }
            }
        }
    }
    //
    mpFlowControl->resumeSwitchModeFlow();
    //
    FUNC_END;
    return  st;
}


/******************************************************************************
*
*******************************************************************************/
status_t
CamAdapter::
onHandleNormalCaptureDone()
{
    CAM_TRACE_NAME("Adapter:onHandleNormalCaptureDone");
    //
    FUNC_START;
    mpStateManager->transitState(IState::eState_Idle);
    if(mb4CellReMosaicCapFlow)
    {
        msAppMode = MtkCameraParameters::APP_MODE_NAME_MTK_ZSD;
        mb4CellReMosaicCapFlow = MFALSE;
    }
    //
    mpParamsManagerV3->setCancelAF(MTRUE); //to unlock AF (locked by capture flow)
    //
    FUNC_END;
    return OK ;
}


/******************************************************************************
*   CamAdapter::cancelPicture() -> IState::onCancelCapture() ->
*   IStateHandler::onHandleCancelCapture() -> CamAdapter::onHandleCancelCapture()
*******************************************************************************/
status_t
CamAdapter::
onHandleCancelCapture()
{
    CAM_TRACE_NAME("Adapter:onHandleCancelCapture");
    FUNC_START;
    //
    {
        Mutex::Autolock lock(mCaptureBeCanceledMtx);
        if(!mbCaptureBeCanceled)
        {
            MY_LOGD("mbCaptureBeCanceled = MFALSE, mShotMode(%d)",mShotMode);
            if(mShotMode == NSCam::eShotMode_ContinuousShot)
            {
                Mutex::Autolock _l(mShotLock);
                if(mpShot == NULL && mbCaptureFinished == MFALSE)
                {
                    nsecs_t timeout = 2000000000LL; // wait for most 2 secs
                    MY_LOGD("pShot (CShot) is NULL, need to wait (%" PRId64 ") ns +",timeout);
                    mWaitShotLock.waitRelative(mShotLock,timeout);
                    MY_LOGD("pShot (CShot) is NULL, need to wait (%" PRId64 ") ns -",timeout);
                }
            }
            //
            {
                Mutex::Autolock _l(mShotLock);
                sp<IShot> pShot = mpShot;
                //
                if  ( pShot != 0 )
                {
                    pShot->sendCommand(eCmd_cancel);
                }
                else
                {
                    MY_LOGI("pShot is NULL");
                }
                mbCaptureBeCanceled = MTRUE;
            }
        }
    }
    //
    {
        Mutex::Autolock _l(mShotLock);
        sp<IShot> pShot = mpShot;
        //
        if  ( pShot != 0 )
        {
            pShot->sendCommand(eCmd_cancel);
        }
        else
        {
            MY_LOGI("pShot is NULL");
        }
    }
    //
    status_t st = OK;
    if (mpStateManager->isState(IState::eState_ZSLCapture))
    {
        st = onHandleCancelZSLCapture();
    }
    else if (mpStateManager->isState(IState::eState_NormalCapture))
    {
        st = onHandleCancelNormalCapture();
    }
    else if (mpStateManager->isState(IState::eState_VideoSnapshot))
    {
        st = onHandleCancelVideoSnapshot();
    }
    //
    if(mpParamsManagerV3->getParamsMgr()->getHighFpsCapture())
    {
        if(mSetCaptureMaxFpsCount > 0)
        {
            MINT32 p1NodeSensorMode = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
            if(!getLastSensorMode(p1NodeSensorMode))
            {
                MY_LOGW("getLastSensorMode Fail, p1NodeSensorMode set as SENSOR_SCENARIO_ID_NORMAL_CAPTURE");
            }
            if(p1NodeSensorMode != SENSOR_SCENARIO_ID_NORMAL_CAPTURE)
            {
                mSetCaptureMaxFpsCount = 0; //reset
            }
            else
            {
                mSetCaptureMaxFpsCount--;
                //
                if(mSetCaptureMaxFpsCount == 0)
                {
#if '1'==MTKCAM_HAVE_3A_HAL
                    IHal3A *hal3A = MAKE_Hal3A(getOpenId(), LOG_TAG);
                    if ( ! hal3A )
                    {
                        MY_LOGE("hal3A == NULL");
                        return MFALSE;
                    }
                    else
                    {
                        hal3A->send3ACtrl( E3ACtrl_SetCaptureMaxFPS, (MINTPTR)(0), (MINTPTR)(mOldMaxFps*1000));
                        hal3A->send3ACtrl( E3ACtrl_SetMinMaxFps, (MINTPTR)(mOldMinFps*1000), (MINTPTR)(mOldMaxFps*1000));
                        MY_LOGD("E3ACtrl_SetCaptureMaxFPS set (0,%d)",mOldMaxFps*1000);
                        MY_LOGD("E3ACtrl_SetMinMaxFps set (%d,%d)",mOldMinFps*1000, mOldMaxFps*1000);
                        hal3A->destroyInstance(LOG_TAG);
                        //
                        mpParamsManagerV3->setPreviewFpsRange(mOldMinFps,mOldMaxFps);
                        mpFlowControl->setParameters();
                    }
#else
                    MY_LOGW("MTKCAM_HAVE_3A_HAL not define, can't set max/min fps to 3A");
#endif
                    MY_LOGD("mSetCaptureMaxFpsCount==0 , need reset capture max fps: (%d,%d)",mOldMinFps,mOldMaxFps);
                }
                else
                {
                    MY_LOGD("mSetCaptureMaxFpsCount:%d , no need reset capture max fps",mSetCaptureMaxFpsCount);
                }
            }
        }
    }
    //
    mpFlowControl->resumeSwitchModeFlow();
    //
    FUNC_END;
    return  st;
}


/******************************************************************************
*   CamAdapter::takePicture() -> IState::onCapture() ->
*   (Preview)IStateHandler::onHandleZSLCapture() -> CamAdapter::onHandleZSLCapture()
*******************************************************************************/
status_t
CamAdapter::
onHandleZSLCapture()
{
    FUNC_START;
    CAM_TRACE_BEGIN("Adapter:onHandleZSLCapture_before_onCapture");
    status_t status = DEAD_OBJECT;
    sp<ZipImageCallbackThread> pCallbackThread = mpZipCallbackThread.promote();
    sp<ICaptureCmdQueThread> pCaptureCmdQueThread = mpCaptureCmdQueThread;
    //
    if( !pCallbackThread.get() )
    {
        MY_LOGE("no callback thread");
        goto lbExit;
    }
    //
    pCallbackThread->setShotMode(mShotMode, IState::eState_Preview, msAppMode);
    pCallbackThread = 0;
    CAM_TRACE_END();
    //
    if(mShotMode == eShotMode_ContinuousShot)
    {
        mpParamsManagerV3->setControlCALTM(MTRUE, MFALSE);
    }
    //
    if  ( pCaptureCmdQueThread != 0 ) {
        status = pCaptureCmdQueThread->onCapture(mShotMode);
    }
    else
    {
        MY_LOGE("pCaptureCmdQueThread == NULL");
    }
    //
lbExit:
    FUNC_END;
    return  status;

}


/******************************************************************************
*
*******************************************************************************/
status_t
CamAdapter::
onHandleZSLCaptureDone()
{
    //
    FUNC_START;
    CAM_TRACE_NAME("Adapter:onHandleZSLCaptureDone");
    if (mpStateManager->isState(IState::eState_ZSLCapture))
    {
        MY_LOGD("mbFlashOn(%d), mbCancelAF(%d), getCancelAF(%d), mShotMode(%d)",
                mbFlashOn, mbCancelAF, mpParamsManagerV3->getCancelAF(), mShotMode);
        if(mShotMode == eShotMode_ContinuousShot)
        {
            mpParamsManagerV3->setControlCALTM(MFALSE, MFALSE);
            if( mpFlowControl.get() != NULL )
            {
                if( mbFlashOn )
                {
                    mpParamsManagerV3->setForceTorch(MFALSE);
                    mpFlowControl->setParameters();
                }
                mpFlowControl->changeToPreviewStatus();
                //
                if( (mbFlashOn && mbCancelAF) || (!mbFlashOn && mpParamsManagerV3->getCancelAF()) )
                {
                    mpParamsManagerV3->setCancelAF(MFALSE);//reset
                    mpFlowControl->cancelAutoFocus();
                }
            }
            else
            {
                MY_LOGE("get Flow Control fail");
            }
            mbCancelAF = MFALSE;
        }
        else if(mShotMode== eShotMode_ZsdShot)
        {
            mpFlowControl->changeToPreviewStatus();
            //
            if( mpFlowControl.get() != NULL && mpParamsManagerV3.get() != NULL )
            {
                mpParamsManagerV3->setCancelAF(MFALSE);//reset
                mpFlowControl->cancelAutoFocus();
            }
            else
            {
                MY_LOGW("mpFlowControl or mpParamsManagerV3 is NULL, cancelAutoFocus is skiped");
            }
        }
        else if( mpParamsManagerV3->getCancelAF() )
        {
            if( mpFlowControl.get() != NULL && mpParamsManagerV3.get() != NULL )
            {
                mpParamsManagerV3->setCancelAF(MFALSE);//reset
                mpFlowControl->cancelAutoFocus();
            }
            else
            {
                MY_LOGW("mpFlowControl or mpParamsManagerV3 is NULL, cancelAutoFocus is skiped");
            }
        }
        mpStateManager->transitState(IState::eState_Preview);
    }
    else
    {
        MY_LOGW("Not in IState::ZSLCapture !!");
    }
    FUNC_END;
    return  OK;
}


/******************************************************************************
*   CamAdapter::cancelPicture() -> IState::onCancelCapture() ->
*   IStateHandler::onHandleCancelCapture() -> CamAdapter::onHandleCancelCapture()
*******************************************************************************/
status_t
CamAdapter::
onHandleCancelPreviewCapture()
{
    return  onHandleCancelCapture();
}


/******************************************************************************
*
*
*******************************************************************************/
status_t
CamAdapter::
onHandleVideoSnapshot()
{
    FUNC_START;
    CAM_TRACE_NAME("Adapter:onHandleVideoSnapshot");
    status_t status = DEAD_OBJECT;
    sp<ZipImageCallbackThread> pCallbackThread = mpZipCallbackThread.promote();
    sp<ICaptureCmdQueThread> pCaptureCmdQueThread = mpCaptureCmdQueThread;
    //
    if( !pCallbackThread.get() )
    {
        MY_LOGE("no callback thread");
        goto lbExit;
    }
    //
    pCallbackThread->setShotMode(mShotMode, IState::eState_Recording, msAppMode);
    pCallbackThread = 0;
    //
    if(!mbLowPowerVSS)
    {
        mpFlowControl->takePicture();
    }

    if  ( pCaptureCmdQueThread != 0 ) {
        status = pCaptureCmdQueThread->onCapture(mShotMode);
    }
    //
lbExit:
    FUNC_END;
    return  status;

}


/******************************************************************************
*
*
*******************************************************************************/
status_t
CamAdapter::
onHandleVideoSnapshotDone()
{
    FUNC_START;
    if(mpStateManager->isState(IState::eState_VideoSnapshot))
    {
        mpFlowControl->changeToPreviewStatus();
        mpStateManager->transitState(IState::eState_Recording);
    }
    else
    {
        MY_LOGW("Not in IState::eState_VideoSnapshot !!");
    }
    FUNC_END;
    return OK ;
}


/******************************************************************************
*
*
*******************************************************************************/
status_t
CamAdapter::
onHandleCancelVideoSnapshot()
{
    FUNC_START;
    FUNC_END;
    return  OK;
}

/******************************************************************************
*
*
*******************************************************************************/
status_t
CamAdapter::
onHandleCancelNormalCapture()
{
    FUNC_START;
    FUNC_END;
    return  OK;
}
/******************************************************************************
*
*
*******************************************************************************/
status_t
CamAdapter::
onHandleCancelZSLCapture()
{
    FUNC_START;
    FUNC_END;
    return  OK;
}

/******************************************************************************
 *
 ******************************************************************************/
template <typename T>
inline MVOID
updateEntry(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T const& val
)
{
    if( pMetadata == NULL ) {
        //MY_LOGE("pMetadata == NULL");
        return;
    }

    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    pMetadata->update(tag, entry);
}

/******************************************************************************
*
*******************************************************************************/
bool
CamAdapter::
onCaptureThreadLoop(uint32_t shotMode)
{
    FUNC_START;
    CAM_TRACE_NAME("Adapter:onCaptureThreadLoop");
    //
    bool ret = false;
    MUINT32 transform = 0;
    camera3_request_template_t templateID = CAMERA3_TEMPLATE_STILL_CAPTURE;
    //
    mpFlowControl->pauseSwitchModeFlow();
    //
    {
        Mutex::Autolock _l(mbApInStopPreviewLock);
        if(mbApInStopPreview)
        {
            MY_LOGW("AP in stopPreview, don't handle capture mode(%d)",shotMode);
            FUNC_END;
            return  true;
        }
        //
        //  [1] transit to "Capture" state.
        if(mpStateManager->isState(IState::eState_Idle))
        {
            mpStateManager->transitState(IState::eState_NormalCapture);
            templateID = CAMERA3_TEMPLATE_STILL_CAPTURE;
        }
        else
        if(mpStateManager->isState(IState::eState_Preview))
        {
            mpStateManager->transitState(IState::eState_ZSLCapture);
            templateID = CAMERA3_TEMPLATE_STILL_CAPTURE;
        }
        else
        if(mpStateManager->isState(IState::eState_Recording))
        {
            mpStateManager->transitState(IState::eState_VideoSnapshot);
            templateID = CAMERA3_TEMPLATE_VIDEO_SNAPSHOT;
        }
        //
        if( mpStateManager->isState(IState::eState_VideoSnapshot))
        {
            MY_LOGD("In VSS");
            if(mpRecordBufferSnapShot != NULL)
            {
                mpRecordBufferSnapShot->takePicture();
            }
        }
    }
    //
    if( !mbLowPowerVSS ||
        !(mpStateManager->isState(IState::eState_VideoSnapshot)))
    {
        //
        //  [2.1] update mpShot instance.
        ret = updateShotInstance(shotMode);
        sp<IShot> pShot = mpShot;
        //
        {
            Mutex::Autolock lock(mCaptureBeCanceledMtx);
            if(mbCaptureBeCanceled)
            {
                MY_LOGD("mbCaptureBeCanceled = MTRUE, mShotMode(%d)", mShotMode);
                if(mShotMode == NSCam::eShotMode_ContinuousShot)
                {
                    MY_LOGD("pShot->sendCommand(eCmd_cancel)");
                    pShot->sendCommand(eCmd_cancel);
                }
            }
        }
        //  [2.2] return if no shot instance.
        if  ( ! ret || pShot == 0 )
        {
            //#warning "[TODO] perform a dummy compressed-image callback or CAMERA_MSG_ERROR to inform app of end of capture?"
            MY_LOGE("updateShotInstance(%d), pShot.get(%p)", ret, pShot.get());
            goto lbExit;
        }
        else
        {
            CAM_TRACE_BEGIN("prepare parameters");
            //  [3.1] prepare parameters
            sp<IParamsManager> pParamsMgr = getParamsManager();
            //
            int iPictureWidth = 0, iPictureHeight = 0;
            if(mpStateManager->isState(IState::eState_VideoSnapshot))
            {
                pParamsMgr->getVideoSize(&iPictureWidth, &iPictureHeight);
            }
            else
            {
                pParamsMgr->getPictureSize(&iPictureWidth, &iPictureHeight);
            }
            //
            int iPreviewWidth = 0, iPreviewHeight = 0;
            pParamsMgr->getPreviewSize(&iPreviewWidth, &iPreviewHeight);
            String8 s8DisplayFormat = mpImgBufProvidersMgr->queryFormat(IImgBufProvider::eID_DISPLAY);
            if  ( String8::empty() == s8DisplayFormat ) {
                MY_LOGW("Display Format is empty");
            }
            //
            // convert rotation to transform
            MINT32 rotation = pParamsMgr->getInt(CameraParameters::KEY_ROTATION);
            //
            switch(rotation)
            {
                case 0:
                    transform = 0;
                    break;
                case 90:
                    transform = eTransform_ROT_90;
                    break;
                case 180:
                    transform = eTransform_ROT_180;
                    break;
                case 270:
                    transform = eTransform_ROT_270;
                    break;
                default:
                    break;
            }

            //  [3.2] prepare parameters: ShotParam
            ShotParam shotParam;
            shotParam.mpParamsMgr = pParamsMgr;
            shotParam.miPictureFormat           = MtkCameraParameters::queryImageFormat(pParamsMgr->getStr(CameraParameters::KEY_PICTURE_FORMAT));
            shotParam.mi4MfbMode  = mi4MfbMode;
            shotParam.mbFlashOn = mbFlashOn;
            shotParam.mbHadPrecapture = mbHadPrecapture;
            shotParam.mbIsUsingBackgroundService = isUsingBackgroundService();

            if  ( 0 != iPictureWidth && 0 != iPictureHeight )
            {
                shotParam.mi4PictureWidth       = iPictureWidth;
                shotParam.mi4PictureHeight      = iPictureHeight;
            }
            else
            {   //  When 3rd-party apk sets "picture-size=0x0", replace it with "preview-size".
                shotParam.mi4PictureWidth       = iPreviewWidth;
                shotParam.mi4PictureHeight      = iPreviewHeight;
            }
            MY_LOGD("shotPicWxH(%dx%d)", shotParam.mi4PictureWidth, shotParam.mi4PictureHeight);
            shotParam.miPostviewDisplayFormat   = MtkCameraParameters::queryImageFormat(s8DisplayFormat.string());
            shotParam.miPostviewClientFormat    = MtkCameraParameters::queryImageFormat(pParamsMgr->getStr(MtkCameraParameters::KEY_POST_VIEW_FMT));
            // if postview YUV format is unknown, set NV21 as default
            if (shotParam.miPostviewClientFormat == eImgFmt_UNKNOWN)
                shotParam.miPostviewClientFormat = eImgFmt_NV21;

            shotParam.mi4PostviewWidth          = iPreviewWidth;
            shotParam.mi4PostviewHeight         = iPreviewHeight;
            shotParam.ms8ShotFileName           = pParamsMgr->getStr(MtkCameraParameters::KEY_CAPTURE_PATH);
#if DUALCAM_SHOT
            shotParam.mu4ZoomRatio              = pParamsMgr->getZoomRatio(getOpenId());
#else
            shotParam.mu4ZoomRatio              = pParamsMgr->getZoomRatio();
#endif
            if( shotMode == eShotMode_EngShot)
            {
                shotParam.muRealIso = 0;
                shotParam.muSensorGain              = pParamsMgr->getInt(MtkCameraParameters::KEY_ENG_MANUAL_SENSOR_GAIN);
                shotParam.muSensorSpeed             = pParamsMgr->getInt(MtkCameraParameters::KEY_ENG_MANUAL_SHUTTER_SPEED);
                //
                //for manual shutter
                if(shotParam.muSensorGain |shotParam.muSensorSpeed)
                {
                    if(shotParam.muSensorSpeed==0&&shotParam.muSensorGain!=0)
                    {
                        shotParam.muSensorSpeed=mEposuretime/1000;
                    }
                    if(shotParam.muSensorGain==0&&shotParam.muSensorSpeed!=0)
                    {
                        shotParam.muSensorGain= (MUINT64)mEposuretime*mAfeGain/shotParam.muSensorSpeed*mIspGain/1024;
                        if(shotParam.muSensorGain < 1024)
                        {
                            shotParam.muSensorGain = 1024;
                        }
                        // please set MaxSensorgain by sensor
                        else if(shotParam.muSensorGain > 16384)
                        {
                            shotParam.muSensorGain = 16384;
                        }
                    }
                }
            }
            else
            {
                shotParam.muSensorGain = 0;
                int iso=0;
                int exp=0;
                pParamsMgr->getIsoSpeed(&iso);
                pParamsMgr->getExposureTime(&exp);
                shotParam.muRealIso = (uint32_t)iso;
                shotParam.muSensorSpeed = (uint32_t)exp;
                shotParam.mbEnableManualCapture = pParamsMgr->getManualCap();
                if(shotParam.mbEnableManualCapture)
                {
                    if(!(shotParam.muRealIso==0 && shotParam.muSensorSpeed==0))
                    {
                        if(shotParam.muRealIso == 0)
                        {
                            shotParam.muRealIso = mRealIso;
                            MY_LOGD("muRealIso is auto(0), change to (%d)",mRealIso);
                        }
                        if(shotParam.muSensorSpeed == 0)
                        {
                            shotParam.muSensorSpeed = mEposuretime/1000;
                            MY_LOGD("muSensorSpeed is auto(0), change to (%d)",shotParam.muSensorSpeed);
                        }
                    }
                    else
                    {
                        MY_LOGD("muRealIso is auto(0), muSensorSpeed is auto(0), no need setManualAEControl flow");
                    }
                }
                else
                {
                    MY_LOGD("ManualCap == false, no need to check muRealIso and muSensorSpeed");
                }
            }
            //
            MY_LOGD("lpf muSensorGain = %d, muSensorSpeed = %d, muRealIso = %d, mbEnableManualCapture = %d",shotParam.muSensorGain,shotParam.muSensorSpeed,shotParam.muRealIso,shotParam.mbEnableManualCapture);
            //
            {
                MINT32 sensorGain, sensorSpeed, realIso;
                sensorGain = property_get_int32("debug.camera.sensor.gain", -1);
                if( sensorGain > 0 )
                {
                    shotParam.muSensorGain = sensorGain;
                }
                sensorSpeed = property_get_int32("debug.camera.sensor.speed", -1);
                if( sensorSpeed > 0 )
                {
                    shotParam.muSensorSpeed = sensorSpeed;
                }
                realIso = property_get_int32("debug.camera.sensor.iso", -1);
                if( realIso > 0 )
                {
                    shotParam.muRealIso = realIso;
                }
                MY_LOGD("prop (gain,speed,iso)=(%d,%d,%d), param (gain,speed,iso)=(%d,%d,%d)",
                        sensorGain, sensorSpeed, realIso, shotParam.muSensorGain, shotParam.muSensorSpeed, shotParam.muRealIso);
            }
            shotParam.mu4ShotCount              = pParamsMgr->getInt(MtkCameraParameters::KEY_BURST_SHOT_NUM);
            shotParam.mu4Transform              = transform;
            shotParam.mu4MultiFrameBlending     = pParamsMgr->getMultFrameBlending();

            // LTM module on/off information
            shotParam.mbEnableLtm               = isEnabledLTM() ? MTRUE : MFALSE;
            CAM_TRACE_END();
            //
            shotParam.muSensorMode = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
            String8 ms8SaveMode = pParamsMgr->getStr(MtkCameraParameters::KEY_RAW_SAVE_MODE);
            const char *strSaveMode = ms8SaveMode.string();
            switch (atoi(strSaveMode))
            {
                case 1:
                    shotParam.muSensorMode = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
                    break;
                case 2:
                    shotParam.muSensorMode = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
                    break;
                case 4:
                    shotParam.muSensorMode = SENSOR_SCENARIO_ID_NORMAL_VIDEO;
                    break;
                case 5:
                    shotParam.muSensorMode = SENSOR_SCENARIO_ID_SLIM_VIDEO1;
                    break;
                case 6:
                    shotParam.muSensorMode = SENSOR_SCENARIO_ID_SLIM_VIDEO2;
                    break;
                default:
                    if (atoi(strSaveMode) > 6)
                    {
                         shotParam.muSensorMode = atoi(strSaveMode) - 2;
                    }
                    break;
            }
            //
            {
                CAM_TRACE_BEGIN("update request");
                if( mpParamsManagerV3 != 0 ) {
                    ITemplateRequest* obj = NSTemplateRequestManager::valueFor(getOpenId());
                    if(obj == NULL) {
                        MY_LOGE("cannot get template request!");
                    }
                    else {
                        //template id is defined in camera3.h
                        shotParam.mAppSetting = obj->getMtkData(templateID);
                        mpParamsManagerV3->updateRequestJpeg(&shotParam.mAppSetting);
                        if( shotMode == eShotMode_VideoSnapShot )
                        {
                            shotParam.muSensorMode = SENSOR_SCENARIO_ID_NORMAL_VIDEO;
                            sp<IFeatureFlowControl> spFlowControl = IResourceContainer::getInstance(getOpenId())->queryFeatureFlowControl();
                            if( spFlowControl == NULL )
                            {
                                MY_LOGW("failed to queryFeatureFlowControl");
                            }
                            else
                            {
                                shotParam.muSensorMode = spFlowControl->getSensorMode();
                            }
                            mpParamsManagerV3->updateRequest(&shotParam.mAppSetting, shotParam.muSensorMode);
                            mpParamsManagerV3->updateRequestVSS(&shotParam.mAppSetting);
                        }
                        else if( shotMode == eShotMode_EngShot)
                        {
                            //[customize] 4-cell sensor update scenario sensor mode
                            HwInfoHelper helper(getOpenId());
                            if( ! helper.updateInfos() ) {
                                MY_LOGE("cannot properly update infos");
                                return BAD_VALUE;
                            }
                            //
                            shotParam.mbEnableRrzoCapture = ((helper.get4CellSensorSupported()&&(shotParam.muSensorMode == SENSOR_SCENARIO_ID_NORMAL_CAPTURE))?
                                                             MFALSE : MTRUE);
                            mpParamsManagerV3->updateRequest(&shotParam.mAppSetting, shotParam.muSensorMode);
                            //
                        }
                        else
                        {
                            //[customize] 4-cell sensor update scenario sensor mode
                            HwInfoHelper helper(getOpenId());
                            if( ! helper.updateInfos() ) {
                                MY_LOGE("cannot properly update infos");
                                return BAD_VALUE;
                            }
                            //
                            if(helper.get4CellSensorSupported())
                            {
                                if(mShotMode == eShotMode_4CellRemosaicShot) {
                                    // 4-cell shot support capture sensor mode and remosaic processing
                                    shotParam.mbEnableRrzoCapture = MFALSE;
                                    shotParam.muSensorMode = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
                                    mpParamsManagerV3->updateRequest(&shotParam.mAppSetting, shotParam.muSensorMode);
                                }
                                else if(mShotMode == eShotMode_MfllShot){
                                    // in eShotMode_MfllShot for performance requirement,
                                    // sw 4-cell sensor only support preview sensor mode (bayer pattern)
                                    MUINT32 u4RawFmtType = 0;
                                    if(helper.getSensorRawFmtType(u4RawFmtType)) {
                                        MY_LOGD("4 cell sensor raw format: %d", u4RawFmtType);
                                        if (u4RawFmtType == SENSOR_RAW_4CELL ||
                                            u4RawFmtType == SENSOR_RAW_4CELL_BAYER) {
                                            // sw 4 cell remosaic sensor, not support full capture sensor mode (mosaic pattern raw)
                                            // use preview sensor mode raw (bayer pattern)
                                            shotParam.muSensorMode =  SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
                                        }
                                        else if (u4RawFmtType == SENSOR_RAW_4CELL_HW_BAYER) {
                                            // hw 4 cell remosaic sensor, support full capture sensor mode (bayer pattern raw)
                                            shotParam.muSensorMode =  SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
                                        }
                                        else {
                                            MY_LOGW("not 4 cell raw format: %d", u4RawFmtType);
                                            shotParam.muSensorMode =  SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
                                        }
                                    }
                                    else {
                                        MY_LOGW("unknow raw format, getSensorRawFmtType() fail!");
                                        shotParam.muSensorMode =  SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
                                    }
                                    if(mbIsNeedSwitchModeToCapture)
                                    {
                                        shotParam.mbNeedP1DoneCb = MTRUE;
                                    }
                                    mpParamsManagerV3->updateRequest(&shotParam.mAppSetting, shotParam.muSensorMode);
                                }
                                else{
                                    shotParam.mbEnableRrzoCapture = MTRUE;
                                    shotParam.muSensorMode =  SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
                                    if(mbIsNeedSwitchModeToCapture)
                                    {
                                        shotParam.mbNeedP1DoneCb = MTRUE;
                                    }
                                    mpParamsManagerV3->updateRequest(&shotParam.mAppSetting, shotParam.muSensorMode);
                                }
                                MY_LOGD("4CellSensor, shotMode:%d sensorMode:%d, rrzoCap:%d, needP1DoneCb:%d",
                                        mShotMode, shotParam.muSensorMode, shotParam.mbEnableRrzoCapture, shotParam.mbNeedP1DoneCb);
                            }
                            else if(mbIsNeedSwitchModeToCapture)
                            {
                                shotParam.muSensorMode = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
                                shotParam.mbNeedP1DoneCb = MTRUE;
                                shotParam.mbNeedP2DoneCb = MTRUE;
                                mpParamsManagerV3->updateRequest(&shotParam.mAppSetting, shotParam.muSensorMode);
                                MY_LOGD("NeedSwitchModeToCapture, shotMode:%d sensorMode:%d",
                                        mShotMode, shotParam.muSensorMode);
                            }
                            else if(mpParamsManagerV3->getParamsMgr()->getDynamicSwitchSensorMode())
                            {
                                MINT32 p1NodeSensorMode =  SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
                                if(!getLastSensorMode(p1NodeSensorMode))
                                {
                                    MY_LOGW("getLastSensorMode Fail, p1NodeSensorMode set as SENSOR_SCENARIO_ID_NORMAL_CAPTURE");
                                }
                                shotParam.muSensorMode = p1NodeSensorMode;
                                mpParamsManagerV3->updateRequest(&shotParam.mAppSetting, shotParam.muSensorMode);
                                MY_LOGD("getDynamicSwitchSensorMode:MTRUE, MTK_P1NODE_SENSOR_MODE(%d), shotParam.muSensorMode will use this mode",p1NodeSensorMode);
                            }
                            else
                            {
                                shotParam.muSensorMode =  SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
                                mpParamsManagerV3->updateRequest(&shotParam.mAppSetting, shotParam.muSensorMode);
                            }
                        }
                        MRect reqCropRegion, reqSensorCropRegion, reqSensorPreviewCropRegion;
                        mpParamsManagerV3->calculateCropRegion(shotParam.muSensorMode);
                        mpParamsManagerV3->getCapCropRegion(reqCropRegion, reqSensorCropRegion, reqSensorPreviewCropRegion);
                        //
                        {
                            if ( iPictureWidth * iPictureHeight <= 0 ) {
                                MY_LOGE("Picture size error %dx%d", iPictureWidth, iPictureHeight);
                                goto lbExit;
                            }
                            IMetadata::IEntry entry1(MTK_SCALER_CROP_REGION);
                            MRect regionCapCropRegion;
                            if( reqCropRegion.s.w * iPictureHeight > reqCropRegion.s.h * iPictureWidth ) { // pillarbox
                                regionCapCropRegion.s.w = reqCropRegion.s.h * iPictureWidth / iPictureHeight;
                                regionCapCropRegion.s.h = reqCropRegion.s.h;
                                regionCapCropRegion.p.x = reqCropRegion.p.x + ((reqCropRegion.s.w - regionCapCropRegion.s.w) >> 1);
                                regionCapCropRegion.p.y = reqCropRegion.p.y;
                            }
                            else { // letterbox
                                regionCapCropRegion.s.w = reqCropRegion.s.w;
                                regionCapCropRegion.s.h = reqCropRegion.s.w * iPictureHeight / iPictureWidth;
                                regionCapCropRegion.p.x = reqCropRegion.p.x;
                                regionCapCropRegion.p.y = reqCropRegion.p.y + ((reqCropRegion.s.h - regionCapCropRegion.s.h) >> 1);
                            }
                            entry1.push_back(regionCapCropRegion, Type2Type<MRect>());
                            shotParam.mAppSetting.update(MTK_SCALER_CROP_REGION, entry1);
                            MY_LOGD("zoom crop(%d,%d,%dx%d), cap crop(%d,%d,%dx%d)"
                                    , reqCropRegion.p.x, reqCropRegion.p.y, reqCropRegion.s.w, reqCropRegion.s.h
                                    , regionCapCropRegion.p.x, regionCapCropRegion.p.y, regionCapCropRegion.s.w, regionCapCropRegion.s.h);
                        }
                        {
                            IMetadata::IEntry entry1(MTK_P1NODE_SENSOR_CROP_REGION);
                            if( mpStateManager->isState(IState::eState_VideoSnapshot) )
                            {
                                entry1.push_back(reqSensorPreviewCropRegion, Type2Type<MRect>());
                            }
                            else
                            {
                                entry1.push_back(reqSensorCropRegion, Type2Type<MRect>());
                            }
                            shotParam.mHalSetting.update(MTK_P1NODE_SENSOR_CROP_REGION, entry1);
                        }
                        #if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1)
                        if (mpParamsManagerV3->getZoomRatio() && NSCam::v1::Stereo::E_DUALCAM_FEATURE_ZOOM == StereoSettingProvider::getStereoFeatureMode())
                        {
                            IMetadata::IEntry entry1(MTK_DUALZOOM_ZOOMRATIO);
                            entry1.push_back(mpParamsManagerV3->getZoomRatio(), Type2Type<MINT32>());
                            shotParam.mHalSetting.update(MTK_DUALZOOM_ZOOMRATIO, entry1);
                        }
                        #endif

                        // update default HAL settings
                        mpParamsManagerV3->updateRequestHal(&shotParam.mHalSetting, shotParam.muSensorMode, MTRUE);

                        {
                            IMetadata::IEntry entry(MTK_HAL_REQUEST_REQUIRE_EXIF);
                            entry.push_back(true, Type2Type< MUINT8 >());
                            shotParam.mHalSetting.update(entry.tag(), entry);
                            MY_LOGD("set HalSetting REQUIRE_EXIF (true)");
                        }
                        if( mIsRaw16CBEnable )
                        {
                            IMetadata::IEntry entry(MTK_STATISTICS_LENS_SHADING_MAP_MODE);
                            entry.push_back(MTK_STATISTICS_LENS_SHADING_MAP_MODE_ON, Type2Type< MUINT8 >());
                            shotParam.mAppSetting.update(entry.tag(), entry);
                            MY_LOGD("DNG set MTK_STATISTICS_LENS_SHADING_MAP_MODE (ON)");
                        }

                        // update HDR mode to 3A
                        {
                            NSCam::HDRMode kHDRMode = mpParamsManagerV3->getParamsMgr()->getHDRMode();
                            uint32_t supportVHDRMode = SENSOR_VHDR_MODE_NONE;
                            uint32_t vhdrMode = pParamsMgr->getVHdr();
                            bool isSingleFrameCaptureHDR = pParamsMgr->getSingleFrameCaptureHDR();

                            HwInfoHelper helper(getOpenId());
                            if (! helper.querySupportVHDRMode(shotParam.muSensorMode, supportVHDRMode))
                            {
                                MY_LOGE("SensorMode(%d) querySupportVHDRMode fail", shotParam.muSensorMode);
                                return BAD_VALUE;
                            }

                            if(vhdrMode != supportVHDRMode || !isSingleFrameCaptureHDR)
                            {
                                if(kHDRMode == HDRMode::VIDEO_ON)
                                    kHDRMode = HDRMode::ON;
                                else if(kHDRMode == HDRMode::VIDEO_AUTO)
                                    kHDRMode = HDRMode::AUTO;
                                MY_LOGD("Not support VHDR capture, hdrMode(%d)", kHDRMode);
                            }

                            IMetadata::IEntry entry(MTK_3A_HDR_MODE);
                            entry.push_back(
                                    static_cast<MUINT8>(kHDRMode), Type2Type< MUINT8 >());
                            shotParam.mHalSetting.update(entry.tag(), entry);
                        }

                        // store customHint in metadata for customize feature
                        {
                            MINT32 customHint = pParamsMgr->getCustomHint();
                            IMetadata::IEntry entry(MTK_PLUGIN_CUSTOM_HINT);
                            entry.push_back(
                                    static_cast<MINT32>(customHint), Type2Type<MINT32>());
                            shotParam.mHalSetting.update(entry.tag(), entry);
                        }
                    }
                }
                else {
                    MY_LOGW("cannot create paramsmgr v3");
                }
                CAM_TRACE_END();
                // T.B.D.
                // shotParam.mHalSetting
            }

            //for continuous shot
            PreviewMetadata previewMetadata;
            sp<IResourceContainer> spContainer = NULL;
            {
                if( (shotMode == eShotMode_ContinuousShot) && mpStateManager->isState(IState::eState_NormalCapture) )
                {
                    MY_LOGD("normal CShot, get metadata and streambufferprovider");
                    //get preview metadata
                    if( mpParamsManagerV3 != 0 )
                    {
                        ITemplateRequest* obj = NSTemplateRequestManager::valueFor(getOpenId());
                        if(obj == NULL)
                        {
                            MY_LOGE("cannot get template request!");
                        }
                        else
                        {
                            shotParam.muSensorMode = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
                            //template id is defined in camera3.h
                            previewMetadata.mPreviewAppSetting = obj->getMtkData(CAMERA3_TEMPLATE_ZERO_SHUTTER_LAG);
                            mpParamsManagerV3->updateRequest(&previewMetadata.mPreviewAppSetting, shotParam.muSensorMode);
                            // update default HAL settings
                            mpParamsManagerV3->updateRequestHal(&previewMetadata.mPreviewHalSetting, shotParam.muSensorMode,MTRUE);
                            //
                            IMetadata::IEntry entry(MTK_HAL_REQUEST_REQUIRE_EXIF);
                            entry.push_back(true, Type2Type< MUINT8 >());
                            previewMetadata.mPreviewHalSetting.update(entry.tag(), entry);
                        }
                    }
                    else
                    {
                        MY_LOGE("cannot get mpParamsManagerV3");
                    }
                    //
                    if( mbFlashOn )
                    {
                        updateEntry<MUINT8>(&previewMetadata.mPreviewAppSetting , MTK_FLASH_MODE , MTK_FLASH_MODE_TORCH);
                        updateEntry<MUINT8>(&previewMetadata.mPreviewAppSetting , MTK_CONTROL_AE_MODE , MTK_CONTROL_AE_MODE_ON);
                    }

                    //create StreamBufferProvider and store in consumer container
                    sp<ClientBufferPool> pClient = new ClientBufferPool(getOpenId(), MTRUE);
                    if( pClient.get() == NULL )
                    {
                        MY_LOGE("pClient.get() == NULL");
                        goto lbExit;
                    }
                    pClient->setCamClient(
                                    "MShot:Preview:Image:yuvDisp",
                                    mpImgBufProvidersMgr,
                                    IImgBufProvider::eID_DISPLAY
                                    );
                    //
                    sp<StreamBufferProviderFactory> pFactory = StreamBufferProviderFactory::createInstance();
                    if( pFactory.get() == NULL )
                    {
                        MY_LOGE("pFactory.get() == NULL");
                        goto lbExit;
                    }
                    pFactory->setUsersPool(pClient);
                    //
                    sp<StreamBufferProvider> spProvider = pFactory->create();
                    if( spProvider.get() == NULL )
                    {
                        MY_LOGE("spProvider.get() == NULL");
                        goto lbExit;
                    }
                    //
                    spContainer = IResourceContainer::getInstance(getOpenId());
                    if( spContainer.get() == NULL )
                    {
                        MY_LOGE("spContainer.get() == NULL");
                        goto lbExit;
                    }
                    spContainer->setConsumer( eSTREAMID_IMAGE_PIPE_YUV_00, spProvider);
                }
            }
            //
            //  [3.3] prepare parameters: JpegParam
            JpegParam jpegParam;
            jpegParam.mu4JpegQuality            = pParamsMgr->getInt(CameraParameters::KEY_JPEG_QUALITY);
            jpegParam.mu4JpegThumbQuality       = pParamsMgr->getInt(CameraParameters::KEY_JPEG_THUMBNAIL_QUALITY);
            jpegParam.mi4JpegThumbWidth         = pParamsMgr->getInt(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH);
            jpegParam.mi4JpegThumbHeight        = pParamsMgr->getInt(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT);
            jpegParam.ms8GpsLatitude            = pParamsMgr->getStr(CameraParameters::KEY_GPS_LATITUDE);
            jpegParam.ms8GpsLongitude           = pParamsMgr->getStr(CameraParameters::KEY_GPS_LONGITUDE);
            jpegParam.ms8GpsAltitude            = pParamsMgr->getStr(CameraParameters::KEY_GPS_ALTITUDE);
            jpegParam.ms8GpsTimestamp           = pParamsMgr->getStr(CameraParameters::KEY_GPS_TIMESTAMP);
            jpegParam.ms8GpsMethod              = pParamsMgr->getStr(CameraParameters::KEY_GPS_PROCESSING_METHOD);
            //
            //check if raw16 CB is enable
            if( mIsRaw16CBEnable )//CAMERA_CMD_ENABLE_RAW16_CALLBACK
            {
                MY_LOGD("adapter.capture: mIsRaw16CBEnable == 1 to enableDataMsg(NSShot::EIShot_DATA_MSG_RAW)");
                pShot->enableDataMsg(NSShot::EIShot_DATA_MSG_RAW);
            }
            else
            {
                MY_LOGD("adapter.capture: mIsRaw16CBEnable == 0 to disableDataMsg(NSShot::EIShot_DATA_MSG_RAW)");
                pShot->disableDataMsg(NSShot::EIShot_DATA_MSG_RAW);
            }
            //
            // Default ZSD behavior for 3rd-party APP
            if(isThirdPartyZSD())
            {
                sp<IFeatureFlowControl> spFlowControl = IResourceContainer::getInstance(getOpenId())->queryFeatureFlowControl();
                if( spFlowControl == NULL ) {
                    MY_LOGW("failed to queryFeatureFlowControl");
                } else {
                    spFlowControl->pausePreview(false);
                }
                mbNeedResumPreview = MTRUE;
                MY_LOGD("mpFlowControl->suspendPreview() success");
            }
            //
            //  [4.1] perform Shot operations.
            {
                CAM_TRACE_BEGIN("SendCommand to Shot");
                if (property_get_int32("debug.enable.normalAEB", 0))
                {
                    int currentEV = 0;
                    shotParam.miCurrentEV=currentEV;
                    {
                         IMetadata::IEntry entry1(MTK_PIPELINE_EV_VALUE);
                         entry1.push_back(currentEV, Type2Type<MINT32>());
                         shotParam.mHalSetting.update(MTK_PIPELINE_EV_VALUE, entry1);
                    }
                }
                ret = pShot->sendCommand(eCmd_reset);
                ret = ret && pShot->setCallback(this);
                ret = ret && pShot->sendCommand(eCmd_setShotParam, (MUINTPTR)&shotParam, sizeof(ShotParam));
                ret = ret && pShot->sendCommand(eCmd_setJpegParam, (MUINTPTR)&jpegParam, sizeof(JpegParam));
                ret = ret && ( ( (shotMode == eShotMode_ContinuousShot) && mpStateManager->isState(IState::eState_NormalCapture))  ?
                        pShot->sendCommand(eCmd_setPreviewMetadata, (MUINTPTR)&previewMetadata, sizeof(PreviewMetadata)) : MTRUE );
                CAM_TRACE_END();
                CAM_TRACE_BEGIN("SendCommand eCmd_capture");

                ret = ret && pShot->sendCommand(eCmd_capture);

                CAM_TRACE_END();
            }
            //
            if (property_get_int32("debug.enable.normalAEB", 0))
            {
                MY_LOGD("enableAEB cpatureflow start");
                int maxEV = property_get_int32("debug.camera.maxEVvalue", +100);
                int minEV = property_get_int32("debug.camera.minEVvalue", -100);
                int interval = property_get_int32("debug.camera.evInterval", 100);
                int currentEV = minEV;
                MY_LOGD("currentEV (%d)", currentEV );
                do{
                    MY_LOGD("AEB-capture Start: min(%d), Max(%d), interval(%d), currentEV(%d)", minEV,maxEV,interval,currentEV);
                    {
                        //CAM_TRACE_BEGIN("SendCommand to Shot");
                        shotParam.miCurrentEV=currentEV;
                        {
                             IMetadata::IEntry entry1(MTK_PIPELINE_EV_VALUE);
                             entry1.push_back(currentEV, Type2Type<MINT32>());
                             shotParam.mHalSetting.update(MTK_PIPELINE_EV_VALUE, entry1);
                        }
                        ret = pShot->sendCommand(eCmd_reset);
                        ret = ret && pShot->setCallback(this);
                        ret = ret && pShot->sendCommand(eCmd_setShotParam, (MUINTPTR)&shotParam, sizeof(ShotParam));
                        ret = ret && pShot->sendCommand(eCmd_setJpegParam, (MUINTPTR)&jpegParam, sizeof(JpegParam));
                        ret = ret && ( ( (shotMode == eShotMode_ContinuousShot) && mpStateManager->isState(IState::eState_NormalCapture))  ?
                        pShot->sendCommand(eCmd_setPreviewMetadata, (MUINTPTR)&previewMetadata, sizeof(PreviewMetadata)) : MTRUE );
                        //CAM_TRACE_END();
                        //CAM_TRACE_BEGIN("SendCommand eCmd_capture");
                        ret = ret && pShot->sendCommand(eCmd_capture);
                        MY_LOGD("AEB-capture END: min(%d), Max(%d), interval(%d), currentEV(%d)", minEV,maxEV,interval,currentEV);
                        CAM_TRACE_END();
                    }
                    currentEV += interval;
                    shotParam.miCurrentEV=0;
                    MY_LOGD("AEB-capture currentEV(%d)", currentEV);
                }while(currentEV<=maxEV);
                MY_LOGD("enableAEB cpatureflow end");
            }
            if  ( ! ret )
            {
                MY_LOGE("fail to perform shot operations");
            }
            //
            spContainer = NULL;
        }
        //
        {
            Mutex::Autolock lock(mCaptureBeCanceledMtx);
            mbCaptureBeCanceled = MFALSE;
        }
        //
        lbExit:
        //
        //  [5.1] uninit shot instance.
        MY_LOGD("free shot instance: (mpShot/pShot)=(%p/%p)", mpShot.get(), pShot.get());
        CAM_TRACE_BEGIN("uninit");
        //
        {
            Mutex::Autolock _l(mShotLock);
            mbCaptureFinished = MTRUE;
            mpShot = NULL;
            pShot  = NULL;
        }
        //
        //  [5.11] update focus steps.
        //
        {
            sp<IParamsManager> pParamsMgr = getParamsManager();
            pParamsMgr->updateBestFocusStep();
        }
        //
        //  [5.2] notify capture done.
        CAM_TRACE_END();
    }
    // update flash calibration result.
    int flashCaliEn = getParamsManager()->getInt(MtkCameraParameters::KEY_ENG_FLASH_CALIBRATION);
    if (flashCaliEn == 1)
    {
        if (getFlashQuickCalibrationResult() != 0)
        {
            onCB_Error(CAMERA_ERROR_CALI_FLASH, 0);
        }
    }
    //
#if DUALCAM_SHOT
    if(StereoSettingProvider::isDualCamMode())
    {
        sp<ISyncManager> pSyncMgr = ISyncManager::getInstance(getOpenId());
        if(pSyncMgr!=nullptr)
        {
            pSyncMgr->unlock(nullptr);
        }
    }
#endif
    mpStateManager->getCurrentState()->onCaptureDone(this);
    //
    //
    FUNC_END;
    return  true;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
CamAdapter::
getLastSensorMode(MINT32& sensorMode)
{
    // get the latest frame to query scene information
    sp<IFrameInfo> pFrameInfo = IResourceContainer::getInstance(getOpenId())->queryLatestFrameInfo();
    if (CC_UNLIKELY(!pFrameInfo.get())) {
        MY_LOGW("Can't query Latest FrameInfo!");
        return MFALSE;
    }
    else {
        IMetadata metadata;
        // get scene mode result from P1's result metadata
        pFrameInfo->getFrameMetadata(eSTREAMID_META_HAL_DYNAMIC_P1, metadata);
        if (CC_UNLIKELY(!IMetadata::getEntry<MINT32>(&metadata, MTK_P1NODE_SENSOR_MODE, sensorMode))) {
            MY_LOGW("Can't query MTK_P1NODE_SENSOR_MODE!");
            return MFALSE;
        }
    }
    MY_LOGD("MTK_P1NODE_SENSOR_MODE(%d)",sensorMode);
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
CamAdapter::
get3AInfo()
{
    mbAeStateSearching = MFALSE;
    mbIsAeAfLock = MFALSE;
    mbFlashOn = MFALSE;
    mbHadPrecapture = MFALSE;
    mbIsLowLvCaptureEnviroment = MFALSE;

    // update mbFlashMfllSupport
    {
        // obtain latest real iso information for shot mode strategy
        std::unique_ptr <
                        IHal3A,
                        std::function<void(IHal3A*)>
                        > hal3a
                (
                    MAKE_Hal3A(getOpenId(), LOG_TAG),
                    [](IHal3A* p){ if (p) p->destroyInstance(LOG_TAG); }
                );

        static std::mutex _locker;
        if (hal3a.get()) {
            std::lock_guard<std::mutex> _l(_locker);
            mbFlashMfllSupport = hal3a->send3ACtrl(E3ACtrl_ChkMFNRFlash, 0, 0);
        }
        else {
            MY_LOGE("create IHal3A instance failed! support flash mfll...");
            mbFlashMfllSupport = MFALSE;
        }
    }

    //
    MUINT8 AeState = 0;
    MUINT8 AeMode = 0;
    MUINT8 AeLock = 0;
    MUINT8 AfMode = 0;
    MBOOL isBvTirgger = MFALSE;
    MUINT8 flashState = 0;
    IMetadata appMeta;
    IMetadata halMeta;

    String8 flashMode = mpParamsManagerV3->getParamsMgr()->getStr(CameraParameters::KEY_FLASH_MODE);
    //
    {
        // obtain latest real iso information for shot mode strategy
        std::unique_ptr <
                        IHal3A,
                        std::function<void(IHal3A*)>
                        > hal3a
                (
                    MAKE_Hal3A(getOpenId(), LOG_TAG),
                    [](IHal3A* p){ if (p) p->destroyInstance(LOG_TAG); }
                );

        static std::mutex _locker;
        int isoThresholdStable1 = 0;
        int isoThresholdStable2 = 0;
        float fLv = -999.0f;
        float fTh = LOW_LV_CAPTURE_THRESHOLD;
        ASDInfo_T ASDInfo;
        if (hal3a.get()) {
            std::lock_guard<std::mutex> _l(_locker);
            ExpSettingParam_T expParam;
            hal3a->send3ACtrl(E3ACtrl_GetExposureInfo,  (MINTPTR)&expParam, 0);
            hal3a->send3ACtrl(E3ACtrl_GetExposureParam, (MINTPTR)&m3ACaptureParamDuringStrategy, 0);
            hal3a->send3ACtrl(E3ACtrl_GetISOThresStatus, (MINTPTR)&isoThresholdStable1, (MINTPTR)&isoThresholdStable2);
            hal3a->send3ACtrl(E3ACtrl_GetAsdInfo, (MINTPTR)&ASDInfo, 0);
            fLv = (float)ASDInfo.i4AELv_x10 / 10;
            int debugLvTh = ::property_get_int32("debug.camera.lowlv.th_x10", -999);
            if( debugLvTh != -999)
            {
                fTh = (float)debugLvTh/10;
            }
            //
            MY_LOGD("fLv(%.1f) LOW_LV_CAPTURE_THRESHOLD(%.1f) debugLvTh(x10)(%d), fTh(%.1f), u4RealISO(%d)",
                fLv,LOW_LV_CAPTURE_THRESHOLD,debugLvTh,fTh,m3ACaptureParamDuringStrategy.u4RealISO);
            //
            MY_LOGD("isoThresholdStable1(%d) isoThresholdStable2(%d)",isoThresholdStable1,isoThresholdStable2);
        }
        else {
            MY_LOGE("create IHal3A instance failed! cannot get current real iso for strategy");
            ::memset(&m3ACaptureParamDuringStrategy, 0, sizeof(CaptureParam_T));
        }
     #if 0
        if(mpParamsManagerV3->getZoomRatio() == 100)
        {
            int lowLvCond = ::property_get_int32("debug.camera.lowlv.cond", -1);
            if(lowLvCond > 0)   //debug forced: by LV to decide if need to change sensor mode to capture
            {
                if(fLv < fTh)
                {
                    mbIsLowLvCaptureEnviroment = MTRUE;
                }
                else
                {
                    mbIsLowLvCaptureEnviroment = MFALSE;
                }
                MY_LOGD("debug forced cond > 0: mbIsLowLvCaptureEnviroment(%d) is decided by LV");
            }
            else if(lowLvCond == 0) //debug forced: by AE P-Line Index to decide if need to change sensor mode to capture
            {
                if(iIsAePlineMaxStable!=0)
                {
                    mbIsLowLvCaptureEnviroment = MTRUE;
                }
                else
                {
                    mbIsLowLvCaptureEnviroment = MFALSE;
                }
                MY_LOGD("debug forced cond == 0: mbIsLowLvCaptureEnviroment(%d) is decided by AEPlineIndexMaxStable");
            }
            else //Default: by AE P-Line Index to decide if need to change sensor mode to capture
            {
                if(iIsAePlineMaxStable!=0)
                {
                    mbIsLowLvCaptureEnviroment = MTRUE;
                }
                else
                {
                    mbIsLowLvCaptureEnviroment = MFALSE;
                }
                MY_LOGD("default cond: mbIsLowLvCaptureEnviroment(%d) is decided by AEPlineIndexMaxStable");
            }
        }
        else
        {
            mbIsLowLvCaptureEnviroment = MFALSE;
            MY_LOGD("ZoomRatio:%d (not 1.0x) dont switch mode to capture, set mbIsLowLvCaptureEnviroment = 0",mpParamsManagerV3->getZoomRatio());
        }
        //
        int debugForcedLowLvEnviroment = ::property_get_int32("debug.camera.lowlv.forced", -1);
        if(debugForcedLowLvEnviroment > 0)
        {
            mbIsLowLvCaptureEnviroment = MTRUE;
            MY_LOGD("forced to set mbIsLowLvCaptureEnviroment = 1");
        }
        else if(debugForcedLowLvEnviroment == 0)
        {
            mbIsLowLvCaptureEnviroment = MFALSE;
            MY_LOGD("forced to set mbIsLowLvCaptureEnviroment = 0");
        }
        #endif
    }
    //
    sp<IFrameInfo> pFrameInfo = IResourceContainer::getInstance(getOpenId())->queryLatestFrameInfo();
        if(pFrameInfo==NULL)
        {
            MY_LOGW("failed to queryLatestFrameInfo");
            goto get3AInfoWithNoInfoExit;
        }

        pFrameInfo->getFrameMetadata(eSTREAMID_META_APP_DYNAMIC_P1, appMeta);
        if ( !IMetadata::getEntry<MUINT8>(const_cast<IMetadata*>(&appMeta), MTK_CONTROL_AE_MODE, AeMode) ) {
            MY_LOGW("NO MTK_CONTROL_AE_MODE info");
            goto get3AInfoWithNoInfoExit;
        }
        if ( !IMetadata::getEntry<MUINT8>(const_cast<IMetadata*>(&appMeta), MTK_CONTROL_AE_STATE, AeState) ) {
            MY_LOGW("NO MTK_CONTROL_AE_STATE info");
            goto get3AInfoWithNoInfoExit;
        }
        if ( !IMetadata::getEntry<MUINT8>(const_cast<IMetadata*>(&appMeta), MTK_CONTROL_AF_MODE, AfMode) ) {
            MY_LOGW("NO MTK_CONTROL_AF_MODE info");
            goto get3AInfoWithNoInfoExit;
        }
        if ( !IMetadata::getEntry<MUINT8>(const_cast<IMetadata*>(&appMeta), MTK_CONTROL_AE_LOCK, AeLock) ) {
            MY_LOGW("NO MTK_CONTROL_AE_LOCK info");
            goto get3AInfoWithNoInfoExit;
        }
        //
        pFrameInfo->getFrameMetadata(eSTREAMID_META_HAL_DYNAMIC_P1, halMeta);
        if ( !IMetadata::getEntry<MBOOL>(const_cast<IMetadata*>(&halMeta), MTK_3A_AE_BV_TRIGGER, isBvTirgger) ) {
            MY_LOGW("NO MTK_3A_AE_BV_TRIGGER info");
            goto get3AInfoWithNoInfoExit;
        }


        // AE mode & flash mode check
        AeMode =
            flashMode == CameraParameters::FLASH_MODE_OFF  ?
                MTK_CONTROL_AE_MODE_ON :
            flashMode == CameraParameters::FLASH_MODE_AUTO ?
                MTK_CONTROL_AE_MODE_ON_AUTO_FLASH :
            flashMode == CameraParameters::FLASH_MODE_ON ?
                MTK_CONTROL_AE_MODE_ON_ALWAYS_FLASH :
            flashMode == CameraParameters::FLASH_MODE_RED_EYE ?
                MTK_CONTROL_AE_MODE_ON_AUTO_FLASH_REDEYE :
            // 2017-12-04: it doesn't matter that touch mode on or off, it always
            //             returns MTK_CONTROL_AE_MODE_ON. Hence we remove this
            //             sentence.
            //
            // !strcmp(flashMode, CameraParameters::FLASH_MODE_TORCH) ?
            //     MTK_CONTROL_AE_MODE_ON:
            MTK_CONTROL_AE_MODE_ON;

        // flash state
        if ( !IMetadata::getEntry<MUINT8>(const_cast<IMetadata*>(&appMeta), MTK_FLASH_STATE, flashState) ) {
            MY_LOGW("NO MTK_FLASH_STATE info");
            goto get3AInfoWithNoInfoExit;
        }

        //
        if( AeLock == MTK_CONTROL_AE_LOCK_ON &&
            AfMode == MTK_CONTROL_AF_MODE_AUTO )
        {
            // In AE/AF lock mode
            mbIsAeAfLock = MTRUE;
        }
        //
        if( flashState != MTK_FLASH_STATE_CHARGING && AeMode == MTK_CONTROL_AE_MODE_ON_AUTO_FLASH && (AeState == MTK_CONTROL_AE_STATE_SEARCHING || AeState == MTK_CONTROL_AE_STATE_INACTIVE) &&
            !mbIsAeAfLock)
        {
            mbAeStateSearching = MTRUE;
        }
        else if( (flashState != MTK_FLASH_STATE_CHARGING && AeMode == MTK_CONTROL_AE_MODE_ON_ALWAYS_FLASH) ||
            (flashState != MTK_FLASH_STATE_CHARGING && AeMode == MTK_CONTROL_AE_MODE_ON_AUTO_FLASH && isBvTirgger) )
        {
            mbFlashOn = MTRUE;
        }

    MY_LOGD("AE Mode(%d) AE State(%d) flash State(%d) mbFlashOn(%d) mbFlashMfllSupport(%d) bAeStateSearching(%d) AeLock(%d) "\
            "AfMode(%d) isBvTrigger(%d) bIsAeAfLock(%d) u4RealISO(%d)",
            AeMode, AeState, flashState, mbFlashOn, mbFlashMfllSupport, mbAeStateSearching, AeLock,
            AfMode, isBvTirgger, mbIsAeAfLock, m3ACaptureParamDuringStrategy.u4RealISO);

    return;

get3AInfoWithNoInfoExit:
    mbIsAeAfLock = MFALSE;
    mbAeStateSearching = MFALSE;
    if(::strcmp(flashMode, CameraParameters::FLASH_MODE_OFF) == 0)
    {
        mbFlashOn = MFALSE;
    }
    else
    {
        mbFlashOn = MTRUE;
    }
    MY_LOGD("get3AInfoWithNoInfoExit: mbIsAeAfLock(%d), mbAeStateSearching(%d), mbFlashOn(%d)",mbIsAeAfLock,mbAeStateSearching,mbFlashOn);
    return;
}

/******************************************************************************
*
*******************************************************************************/
bool
CamAdapter::
is4CellShot(String8& rs8ShotMode, uint32_t& ru4ShotMode) const
{
    // obtain hardware info helper to query if 4 cell sensor is supported
    HwInfoHelper helper(getOpenId());

    // check manual ISO too
    sp<IParamsManager> pParamsMgr = getParamsManager();
    int appISOSpeed = 0; // auto = 0
    if(CC_UNLIKELY( pParamsMgr.get() == nullptr )) {
        MY_LOGE("get pParamsMgr failed, assume ISO speed to AUTO(0)");
    }
    else {
        appISOSpeed = pParamsMgr->getInt(MtkCameraParameters::KEY_ISO_SPEED);
    }
    //
    int threshold_4Cell = property_get_int32("debug.camera.threshold_4Cell", -1);
    int iso_threshold = (threshold_4Cell>=0)? threshold_4Cell : ISO_THRESHOLD_4CellSENSOR;
    uint32_t currentIso = m3ACaptureParamDuringStrategy.u4RealISO;
    MY_LOGD("4cell flow condition: debug:threshold_4Cell(%d), apply:iso_threshold(%d), def:ISO_THRESHOLD_4CellSENSOR(%d), mbFlashOn(%d), currentIso(%d), appISOSpeed(%d)",
            threshold_4Cell, iso_threshold, ISO_THRESHOLD_4CellSENSOR, mbFlashOn, currentIso, appISOSpeed);
    //
    // check these conditions:
    //  1. Is supported 4-cell sensor.
    //  2. Flash is disabled.
    //  3. The current ISO < threshold(ISO_THRESHOLD_4CellSENSOR)
    if( helper.get4CellSensorSupported() && !mbFlashOn && currentIso < iso_threshold
            && appISOSpeed < iso_threshold    )
    {
        MY_LOGD("4cell non-ZSD(Normal) capture flow");
        rs8ShotMode = "4CellRemosaic";
        // 4-cell remosaic processing is in normal capture flow,
        // use the shot mode into capture command queue to hint 4-cell remosaic flow.
        ru4ShotMode  = NSCam::eShotMode_4CellRemosaicShot;
        return MTRUE;
    }
    else
    {
        MY_LOGD("4cell ZSD Capture flow");
        rs8ShotMode = "ZSD";
        ru4ShotMode = NSCam::eShotMode_ZsdShot;
        return MFALSE;
    }

}

/******************************************************************************
*
*******************************************************************************/
bool
CamAdapter::
isDualCamShot(String8& rs8ShotMode, uint32_t& ru4ShotMode) const
{
bool ret = false;
MINT32 shotMode = -1;

#if DUALCAM_SHOT
#if (MTKCAM_HAVE_DUAL_ZOOM_FUSION_SUPPORT == 1)
#error de-noise can not open with zoom fusion at the same time
#endif
#endif

#if DUALCAM_SHOT
    if(StereoSettingProvider::isDualCamMode()){
        shotMode = StereoSettingProvider::getStereoShotMode();
        if(shotMode == eShotMode_BMDNShot){
            rs8ShotMode = "BMDNShot";
            ru4ShotMode = eShotMode_BMDNShot;
            ret = true;
        }else if(shotMode == eShotMode_MFHRShot){
            rs8ShotMode = "MFHRShot";
            ru4ShotMode = eShotMode_MFHRShot;
            ret = true;
        }
        else if(shotMode == eShotMode_DCMFShot){
            if(isDualCamHdr()){
                rs8ShotMode = "DCMFHdrShot";
                ru4ShotMode = eShotMode_DCMFHdrShot;
            }else{
                rs8ShotMode = "DCMFShot";
                ru4ShotMode = eShotMode_DCMFShot;
            }
            ret = true;
        }
        else if(shotMode == eShotMode_FusionShot){
            rs8ShotMode = "FusionShot";
            ru4ShotMode = NSCam::eShotMode_FusionShot;
            ret = true;
        }
        else
        {
            ret  = false;
        }
    }
#endif

MY_LOGD("shot(%d) ret(%d)", shotMode, ret);
return ret;
}
/******************************************************************************
*
*******************************************************************************/
bool
CamAdapter::
isDualCamHdr() const
{
bool ret = false;

#if DUALCAM_SHOT
// todo: auto detection logic
int enableDCMFHdr = property_get_int32("vendor.debug.enable.dcmfhdr", 0);

ret = (enableDCMFHdr == 1) ? true : false;
#endif

MY_LOGD("ret(%d)", ret);
return ret;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
CamAdapter::
isUsingBackgroundService() const
{
    /* adb for force enable/disable BackgroundService */
    static int bForceEnable = ::property_get_int32("debug.enable.bgservice", -1);
    if (__builtin_expect( bForceEnable >= 0, false )) {
        MY_LOGD("force set BackgroundService enable to %d", bForceEnable);
        return bForceEnable == 0 ? MFALSE : MTRUE;
    }

    /* check if using BackgroundService */
    sp<IParamsManager> pParamsMgr = getParamsManager();
    if (pParamsMgr.get()) {
        int bgserv_val = pParamsMgr->getInt(MtkCameraParameters::KEY_MTK_BACKGROUND_SERVICE);
        /* both bgserv_val is greater than 0, and ICallbackClientMgr reports supported */
        if (bgserv_val > 0 && ICallbackClientMgr::getInstance()->canSupportBGService()) {
            return MTRUE;
        }
    }
    return MFALSE;
}
