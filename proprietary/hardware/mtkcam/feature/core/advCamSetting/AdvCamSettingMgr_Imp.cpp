/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#define LOG_TAG "MtkCam/AdvCamSettingMgr"

#include "AdvCamSettingMgr_Imp.h"

#include <cutils/properties.h>
#include <utils/Mutex.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/utils/metadata/IMetadataConverter.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metastore/ITemplateRequest.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/std/TypeTraits.h>
#include <mtkcam/utils/hw/HwTransform.h>
#include <hardware/camera3.h>

#include <mtkcam/feature/utils/FeatureProfileHelper.h>
#include <mtkcam/feature/3dnr/3dnr_defs.h>

#include <camera_custom_eis.h>

#define KEY_ENABLE_TSQ "debug.advcam.enable.tsq"
#define VAL_ENABLE_TSQ 1

#define KEY_ENABLE_DFT_ADV_P2 "debug.advcam.enable.dftAdvP2"

using namespace NSCam;
using namespace NSCamHW;
using namespace android;

#undef __func__
#define __func__ __FUNCTION__

#define ACSM_LOG(fmt, arg...)    CAM_LOGD("[%s]" fmt, __func__, ##arg)
#define ACSM_INF(fmt, arg...)    CAM_LOGI("[%s]" fmt, __func__, ##arg)
#define ACSM_WRN(fmt, arg...)    CAM_LOGW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define ACSM_ERR(fmt, arg...)    CAM_LOGE("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#define MY_LOGD_IF(cond, ...)       do { if ( (cond) >= (1) ) { ACSM_LOG(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) >= (1) ) { ACSM_INF(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) >= (1) ) { ACSM_WRN(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) >= (1) ) { ACSM_ERR(__VA_ARGS__); } }while(0)

#define MAX_INST_NUM 4
AdvCamSettingMgr_Imp sImp0(0);
AdvCamSettingMgr_Imp sImp1(1);
AdvCamSettingMgr_Imp sImp2(2);
AdvCamSettingMgr_Imp sImp3(3);
AdvCamSettingMgr_Imp *s_inst[MAX_INST_NUM] = {&sImp0, &sImp1, &sImp2, &sImp3};

AdvCamSettingMgr_Imp*
getImpInstance(const uint32_t aSensorIdx)
{
    if(aSensorIdx >= MAX_INST_NUM){
        ACSM_ERR("sensorID(%d) Over max instance number", aSensorIdx);
        return NULL;
    }

    return s_inst[aSensorIdx];
}


/*******************************************************************************
* Get Instance
********************************************************************************/
extern "C" {
CamCfgSetter*
getSetterInstance(const uint32_t aSensorIdx)
{
    AdvCamSettingMgr_Imp* inst = getImpInstance(aSensorIdx);
    if (inst != NULL){
        ACSM_LOG( "aSensorIdx(%u), (%p)",aSensorIdx, inst);
        return inst;
    }
    ACSM_ERR("getInstance Failed!");
    return NULL;
}
}



AdvCamSettingMgr *AdvCamSettingMgr::getInstance(const MUINT32 aSensorIdx)
{
    AdvCamSettingMgr_Imp* inst = getImpInstance(aSensorIdx);
    if (inst != NULL){
        ACSM_LOG( "aSensorIdx(%u), (%p)",aSensorIdx, inst);
        return inst;
    }
    ACSM_ERR("getInstance Failed!");
    return NULL;
}

/*******************************************************************************
* AdvCamSetttingMgr_Imp  cons/destructors
********************************************************************************/
AdvCamSettingMgr_Imp::AdvCamSettingMgr_Imp(const MUINT32 sensorIdx)
    : mSensorIdx(sensorIdx)
    , mAppMode(MTK_FEATUREPIPE_PHOTO_PREVIEW)
{
}

AdvCamSettingMgr_Imp::~AdvCamSettingMgr_Imp()
{
}

/*******************************************************************************
* ConfigMetaHolder Implementation
********************************************************************************/
MBOOL
AdvCamSettingMgr_Imp::ConfigMetaHolder::extractConfigMeta(IMetadata& outMeta)
{
    AutoMutex lock(mConfigLock);
    if(mIsConfigMetaValid)
        outMeta = mConfigMeta;
    return mIsConfigMetaValid;
}

MBOOL
AdvCamSettingMgr_Imp::ConfigMetaHolder::setConfigMeta(const camera_metadata_t* inputAndroidMeta)
{
    sp<IMetadataConverter> pMetadataConverter =
            IMetadataConverter::createInstance(IDefaultMetadataTagSet::singleton()->getTagSet());
    {
        AutoMutex lock(mConfigLock);
        mConfigMeta.clear();
        mIsConfigMetaValid = MTRUE;
        ACSM_LOG("app set config Metadata(%p), input(%p)", &mConfigMeta, inputAndroidMeta);
        return pMetadataConverter->convert(inputAndroidMeta, mConfigMeta);
    }
}

MBOOL
AdvCamSettingMgr_Imp::ConfigMetaHolder::clear()
{
    AutoMutex lock(mConfigLock);
    MBOOL previousValid = mIsConfigMetaValid;
    mIsConfigMetaValid = MFALSE;
    mConfigMeta.clear();
    return previousValid;
}


/*******************************************************************************
* AdvCamSettingMgr_Imp  function Implementation
********************************************************************************/
bool
AdvCamSettingMgr_Imp::setConfigureParam(const camera_metadata_t* inputAndroidMeta)
{
    return mConfigHolder.setConfigMeta(inputAndroidMeta);
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
AdvCamSettingMgr_Imp::init()
{
    mLogLevel = ::property_get_int32("vendor.debug.camera.log", 0);
    if ( mLogLevel == 0 ) {
        mLogLevel = ::property_get_int32("vendor.debug.camera.log.AdvCamSetting", 0);
    }
    mEnableTSQ = ::property_get_int32(KEY_ENABLE_TSQ, VAL_ENABLE_TSQ);
    mDefaultAdvP2 = ::property_get_int32(KEY_ENABLE_DFT_ADV_P2, 0);

    return MTRUE;
}

MBOOL
AdvCamSettingMgr_Imp::notifyDeviceClose()
{
    MBOOL lastValid = mConfigHolder.clear();
    mIsTkApp = MFALSE;
    ACSM_LOG("original isConfigValid(%d), set to 0 and clean mLastFrameInfo & mIsTkApp", lastValid);
    mLastInfo.reset();
    return MTRUE;
}

MBOOL
AdvCamSettingMgr_Imp::notifyFirstReq()
{
    MBOOL lastValid = mConfigHolder.clear();
    mAppMode = MTK_FEATUREPIPE_PHOTO_PREVIEW;
    mLastInfo.reset();
     ACSM_LOG("original isConfigValid(%d), set to 0 and clean mLastFrameInfo. set mAppMode to PHOTO_PREV", lastValid);
    return MTRUE;
}

sp<AdvCamSetting>
AdvCamSettingMgr_Imp::generateAdvSetting(const AdvCamInputParam &inputParam, const UserParam &userParam)
{
    IMetadata configMeta;
    MBOOL metaValid = mConfigHolder.extractConfigMeta(configMeta); // app has set configParam or not

    mIsTkApp |= (metaValid || isCustomerApp(inputParam.operationMode));
    MBOOL useAdvP2 = needAdvP2(mIsTkApp , inputParam);
    ACSM_LOG("App set new config metadata ? (%d), isTkApp(%d), useAdvP2(%d), opeMode(%d)",
                metaValid, mIsTkApp, useAdvP2, inputParam.operationMode);

    if( !mIsTkApp && !useAdvP2)
        return NULL;

    if( !metaValid )
        genDefaultConfig(inputParam, configMeta);

    // Don't clear until first frame arrived or device close.
    //mConfigHolder.clear();

    return convertMetaToSetting(mIsTkApp, useAdvP2 ,configMeta, inputParam, userParam);
}

sp<AdvCamSetting>
AdvCamSettingMgr_Imp::regenAdvSetting(IMetadata* appMetadata, const AdvCamInputParam &inputParam, const UserParam &userParam)
{
    if(appMetadata == NULL){
        ACSM_ERR("app metadata is NULL!, regen AdvCamSetting FAILED!");
        return NULL;
    }

    MBOOL useAdvP2 = (inputParam.lastAdvCamSetting != NULL)
                                ? inputParam.lastAdvCamSetting->useAdvP2
                                : MFALSE;

    MBOOL isTkApp = (inputParam.lastAdvCamSetting != NULL)
                                ? inputParam.lastAdvCamSetting->isTkApp
                                : MFALSE;

    return convertMetaToSetting(isTkApp, useAdvP2 , *appMetadata, inputParam, userParam);;
}

sp<AdvCamSetting>
AdvCamSettingMgr_Imp::convertMetaToSetting(       MBOOL             isTkApp,
                                                  MBOOL             useAdvP2,
                                            const IMetadata         &configMeta,
                                            const AdvCamInputParam  &inputPara,
                                            const UserParam         &userParam)
{
    sp<AdvCamSetting> outSet = new AdvCamSetting();
    outSet->useAdvP2 = useAdvP2;
    outSet->isTkApp = isTkApp;

    // parse metadata to adv camera setting
    // ===== HDR =====
    HDRMode hdrMode = getHDRMode(&configMeta);
    outSet->vhdrMode = getVHDRMode(hdrMode,NULL);

    // ===== 3DNR ======
    outSet->nr3dMode = get3DNRMode(configMeta, inputPara, userParam);
    outSet->needLMV = (outSet->nr3dMode &
        (NR3D::E3DNR_MODE_MASK_UI_SUPPORT | NR3D::E3DNR_MODE_MASK_HAL_FORCE_SUPPORT)) != 0;

    // ===== EIS ======
    calEisSettings(&configMeta, inputPara, outSet->vhdrMode, outSet->isTkApp, outSet->useAdvP2,
                    outSet->eisMode, outSet->eisFactor, outSet->eisExtraBufNum);
    outSet->useTSQ = mEnableTSQ && isAdvEisEnabled(outSet);

    ACSM_LOG("adv Cam Setting, hdr(%hhu), vhdr(%d), 3dnr(0x%x), needLMV(%d), \
eis(0x%x/%u/%u), isRec/isTkApp(%d/%d), is4K(%d), videoSize(%dx%d), useAdvP2(%d)",
            hdrMode, outSet->vhdrMode, outSet->nr3dMode, outSet->needLMV,
            outSet->eisMode, outSet->eisFactor, outSet->eisExtraBufNum,
            inputPara.isRecordPipe, outSet->isTkApp,  inputPara.is4K2K, inputPara.videoSize.w, inputPara.videoSize.h,
            outSet->useAdvP2);

    return outSet;
}

/*******************************************************************************
* App doesn't set config param before pipeline config. We need to generate default config from template
********************************************************************************/
MBOOL AdvCamSettingMgr_Imp::genDefaultConfig(const AdvCamInputParam& inputParam,
                                                          IMetadata&        outMeta)
{
    ITemplateRequest* obj = NSTemplateRequestManager::valueFor(mSensorIdx);
    if( obj == NULL)
    {
        ACSM_ERR("Default template request not built yet !! Can not generate default config.");
        return MFALSE;
    }

    MUINT32 reqType = CAMERA3_TEMPLATE_PREVIEW;
    if(inputParam.operationMode == CAMERA3_STREAM_CONFIGURATION_NORMAL_MODE
        || inputParam.operationMode >= CAMERA3_VENDOR_STREAM_CONFIGURATION_MODE_START)
    {
        if(inputParam.isRecordPipe)
            reqType = CAMERA3_TEMPLATE_VIDEO_RECORD;
    }
    else
    {
        ACSM_WRN("Default Config not support stream operation mode (%d)", inputParam.operationMode);
        return MFALSE;
    }

    outMeta = obj->getMtkData(reqType);

    // Customize config meta by vendor's pipe operation mode
    customConfigMeta(inputParam, outMeta);


    ACSM_LOG("generate Default config done. Template/OperationMode(%u,%u)", reqType, inputParam.operationMode);

    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
AdvCamSettingMgr_Imp::needReconfigPipeline(IMetadata* appMetadata , const PipelineParam &pipelineParam,
                                           const AdvCamInputParam &inputParam)
{
    MBOOL needReconfig = MFALSE;
    MBOOL reconfig = MFALSE;
    sp<AdvCamSetting> curSet = pipelineParam.currentAdvSetting;

    // Check need reconfig
    reconfig |= needReconfigByHDR(appMetadata, curSet);
    reconfig |= needReconfigByEIS(appMetadata, inputParam, curSet);


    return reconfig;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
AdvCamSettingMgr_Imp::updateRequestMeta(
                        IMetadata*  halMetadata,
                        IMetadata*  appMetadata,
                  const PipelineParam &pipelineParam,
                  const RequestParam  &reqParam,
                  const AdvCamInputParam &inputParam
)
{
    if(!appMetadata)
    {
       ACSM_ERR("app metadata is NULL!");
       return MFALSE;
    }

    // Need update common meta first
    updateStreamingState(appMetadata, reqParam, inputParam, halMetadata);

    // Prepare query Feature Streaming ISP Profile
    HDRMode hdrMode = getHDRMode(appMetadata);

    // update HDR mode to 3A
    IMetadata::setEntry<MUINT8>(halMetadata, MTK_3A_HDR_MODE, (MUINT8)toLiteral(hdrMode));

    // EIS
    updateEISRequestMeta(appMetadata, pipelineParam, halMetadata);

    // update ISP Profile
    updateIspProfile(hdrMode, halMetadata, appMetadata, pipelineParam);

    // ======== App Metadata Update ========
    if(!updateCropRegion(halMetadata, appMetadata, pipelineParam))
        return MFALSE;


    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
AdvCamSettingMgr_Imp::updateStreamingState(
                               const IMetadata*          appMetadata,
                               const RequestParam&       reqParam,
                               const AdvCamInputParam&   inputParam,
                                     IMetadata*          halMetadata)
{
    MINT32 recordState = -1;

    // 1. Decide App Mode
    if (IMetadata::getEntry<MINT32>(appMetadata, MTK_STREAMING_FEATURE_RECORD_STATE, recordState) )
    {   // App has set recordState Tag
        if(recordState == MTK_STREAMING_FEATURE_RECORD_STATE_PREVIEW)
        {
            if(reqParam.mHasEncodeBuf)
                mAppMode = MTK_FEATUREPIPE_VIDEO_STOP;
            else
                mAppMode = MTK_FEATUREPIPE_VIDEO_PREVIEW;
        }
        else
        {
            ACSM_WRN("Unknown or Not Supported app recordState(%d)", recordState);
        }
    }
    else
    {   // App has NOT set recordState Tag
        // (slow motion has no repeating request)
        if( reqParam.mIsRepeatingReq
            || inputParam.operationMode == CAMERA3_STREAM_CONFIGURATION_CONSTRAINED_HIGH_SPEED_MODE)
        {
            if(reqParam.mHasEncodeBuf)
                mAppMode = MTK_FEATUREPIPE_VIDEO_RECORD;
            else if (inputParam.isRecordPipe)
                mAppMode = MTK_FEATUREPIPE_VIDEO_PREVIEW;
            else
                mAppMode = MTK_FEATUREPIPE_PHOTO_PREVIEW;
        }
    }

    IMetadata::setEntry<MINT32>(halMetadata, MTK_FEATUREPIPE_APP_MODE, mAppMode);

    MBOOL isRecStaChange = (mLastInfo.recordState != recordState);
    MBOOL isAppModeChange = (mLastInfo.appMode != mAppMode);

    if(isAppModeChange)
    {
        if( (mLastInfo.appMode == MTK_FEATUREPIPE_VIDEO_PREVIEW || mLastInfo.appMode == MTK_FEATUREPIPE_PHOTO_PREVIEW)
                && mAppMode == MTK_FEATUREPIPE_VIDEO_STOP)
            ACSM_WRN("Error state machine change : App mode video/photo preview -> video stop");
        else if(mLastInfo.appMode == MTK_FEATUREPIPE_VIDEO_STOP && mAppMode == MTK_FEATUREPIPE_VIDEO_RECORD)
            ACSM_WRN("Error state machine change : App mode video stop -> video record");
    }

    mLastInfo.recordState = recordState;
    mLastInfo.appMode = mAppMode;

    MY_LOGD_IF((mLogLevel || isRecStaChange || isAppModeChange),
            "rec/appChange(%d/%d), recState/appMode(%d/%d),isRep/hasEnc(%d,%d)",
            isRecStaChange, isAppModeChange ,recordState, mAppMode, reqParam.mIsRepeatingReq, reqParam.mHasEncodeBuf);
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
AdvCamSettingMgr_Imp::updateIspProfile( HDRMode     hdrMode,
                                        IMetadata*  halMetadata,
                                        IMetadata*  appMetadata,
                                  const PipelineParam &pipelineParam)
{
    MUINT32 vhdrMode = SENSOR_VHDR_MODE_NONE;
    MINT32 fMask = ProfileParam::FMASK_NONE;
    if ((hdrMode == HDRMode::AUTO) || (hdrMode == HDRMode::VIDEO_AUTO))
        fMask |= ProfileParam::FMASK_AUTO_HDR_ON;
    vhdrMode = getVHDRMode(hdrMode, pipelineParam.currentAdvSetting);

    if( isEisEnabled(pipelineParam.currentAdvSetting) )
    {
        fMask |= ProfileParam::FMASK_EIS_ON;
    }

    //
    MUINT8 profile = 0;

    ProfileParam profileParam(
        pipelineParam.mResizedRawSize,
        vhdrMode,
        pipelineParam.mSensorMode,
        ProfileParam::FLAG_NONE, // TODO set flag by isZSDPureRawStreaming or not
        fMask
    );
    //for streaming ISP profile
    if (FeatureProfileHelper::getStreamingProf(profile, profileParam))
    {
        IMetadata::setEntry<MUINT8>(halMetadata, MTK_3A_ISP_PROFILE, profile);
    }
    //for VSS/normal capture ISP profile
    if (FeatureProfileHelper::getShotProf(profile, profileParam))
    {
        IMetadata::setEntry<MUINT8>(halMetadata, MTK_VHDR_IMGO_3A_ISP_PROFILE, profile);
    }

    if (mLogLevel >= 1)
    {
        // Print debug info
        MUINT8 RhdrMode = 0;
        if(IMetadata::getEntry<MUINT8>(halMetadata, MTK_3A_HDR_MODE, RhdrMode)){
            MY_LOGD_IF((mLogLevel >= 2), "MTK_3A_HDR_MODE Get!");
        }
        MUINT8 Rprofile = 0;
        if(IMetadata::getEntry<MUINT8>(halMetadata, MTK_3A_ISP_PROFILE, Rprofile)){
            MY_LOGD_IF((mLogLevel >= 2), "MTK_3A_ISP_PROFILE Get!");
        }
        MUINT8 RIMGOprofile = 0;
        if(IMetadata::getEntry<MUINT8>(halMetadata, MTK_VHDR_IMGO_3A_ISP_PROFILE, RIMGOprofile)){
            MY_LOGD_IF((mLogLevel >= 2), "MTK_VHDR_IMGO_3A_ISP_PROFILE:%d",RIMGOprofile);
        }
        MINT32 RrequestID = 0;
        if(IMetadata::getEntry<MINT32>(appMetadata, MTK_REQUEST_ID, RrequestID)){
            MY_LOGD_IF((mLogLevel >= 2), "get APP:MTK_REQUEST_ID:%d",RrequestID);
        }

        ACSM_LOG("Get MTK_3A_HDR_MODE:%d MTK_3A_ISP_PROFILE:%d, \
                MTK_VHDR_IMGO_3A_ISP_PROFILE:%d, MTK_REQUEST_ID:%d",
                RhdrMode, Rprofile, RIMGOprofile, RrequestID);
    }
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
AdvCamSettingMgr_Imp::updateCropRegion(IMetadata*  halMetadata, IMetadata*  appMetadata,const PipelineParam &pipelineParam)
{
    MRect cropActive;
    if(! IMetadata::getEntry<MRect>(appMetadata, MTK_SCALER_CROP_REGION, cropActive) )
    {
       ACSM_ERR("Can not get MTK_SCALER_CROP_REGION !");
       return MFALSE;
    }

    if(pipelineParam.mSensorSize.h <= 0 || pipelineParam.mMaxStreamSize.h <= 0){
        ACSM_ERR("Size height <= 0. sensorSize.h(%d), maxStreamSize.h(%d)",
                    pipelineParam.mSensorSize.h, pipelineParam.mMaxStreamSize.h);
        return MFALSE;
    }
    MUINT32 vhdrMode = (pipelineParam.currentAdvSetting != NULL)
                                ? pipelineParam.currentAdvSetting->vhdrMode
                                : SENSOR_VHDR_MODE_NONE;
#define abs(x,y) ((x)>(y)?(x)-(y):(y)-(x))
#define THRESHOLD 0.2
    float aspRatioDiff = abs((float)pipelineParam.mSensorSize.w / (float)pipelineParam.mSensorSize.h ,
                     (float)pipelineParam.mMaxStreamSize.w / (float)pipelineParam.mMaxStreamSize.h);
    // TODO currently only vHDR need to run this flow
    if (aspRatioDiff > THRESHOLD && vhdrMode != SENSOR_VHDR_MODE_NONE){
        // --- Update Crop Region ----
        MRect sensorCrop, sensorCropAlign, activeCropAlign;
        HwMatrix matSensor2Active;
        HwMatrix matActive2SensorRatioAlign;
        HwTransHelper hwTransHelper(mSensorIdx);
        if (!hwTransHelper.getMatrixToActive(pipelineParam.mSensorMode, matSensor2Active) ||
            !hwTransHelper.getMatrixFromActiveRatioAlign(pipelineParam.mSensorMode, matActive2SensorRatioAlign) ) {
            ACSM_ERR("get matrix fail");
            return MFALSE;
        }

        // Transfer active crop to sensor center_align crop, then transfer to active axis back.
        matActive2SensorRatioAlign.transform(cropActive, sensorCropAlign);
        matSensor2Active.transform(sensorCropAlign, activeCropAlign);

        // Calculate P1 sensor crop region (match sensor aspect ratio) according to sensorCropAlign.
        HwTransHelper::cropAlignRatioInverse(sensorCropAlign, pipelineParam.mSensorSize, sensorCrop);

        if( IMetadata::setEntry(appMetadata, MTK_SCALER_CROP_REGION, activeCropAlign) != OK)
        {
            ACSM_ERR("reset MTK_SCALER_CROP_REGION failed!");
            return MFALSE;
        }

        if( IMetadata::setEntry(halMetadata, MTK_P1NODE_SENSOR_CROP_REGION, sensorCrop) != OK)
        {
            ACSM_ERR("set MTK_P1NODE_SENSOR_CROP_REGION failed!");
            return MFALSE;
        }

        MY_LOGD_IF(mLogLevel, "aspRatioDiff(%f), maxStream(%d,%d), sensor(%d,%d),\
activeCrop(%d,%d,%dx%d),sensorCrop(%d,%d,%dx%d), activeCropAlign(%d,%d,%dx%d),\
sesnorCropAlgin(%d,%d,%dx%d),use Active2SensorFOVAlign",aspRatioDiff,
                pipelineParam.mMaxStreamSize.w, pipelineParam.mMaxStreamSize.h,
                pipelineParam.mSensorSize.w, pipelineParam.mSensorSize.h,
                cropActive.p.x, cropActive.p.y, cropActive.s.w, cropActive.s.h,
                sensorCrop.p.x, sensorCrop.p.y, sensorCrop.s.w, sensorCrop.s.h,
                activeCropAlign.p.x, activeCropAlign.p.y, activeCropAlign.s.w, activeCropAlign.s.h,
                sensorCropAlign.p.x, sensorCropAlign.p.y, sensorCropAlign.s.w, sensorCropAlign.s.h);
    }else{

        MY_LOGD_IF(mLogLevel, "aspRatioDiff(%f), maxStream(%d,%d), sensor(%d,%d),\
activeCrop(%d,%d,%dx%d), remain app crop region",aspRatioDiff,
                pipelineParam.mMaxStreamSize.w, pipelineParam.mMaxStreamSize.h,
                pipelineParam.mSensorSize.w, pipelineParam.mSensorSize.h,
                cropActive.p.x, cropActive.p.y, cropActive.s.w, cropActive.s.h);
    }
#undef THRESHOLD
#undef abs
    return MTRUE;
}

