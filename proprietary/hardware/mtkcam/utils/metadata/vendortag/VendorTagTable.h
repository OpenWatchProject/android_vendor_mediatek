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

#include <system/camera_metadata.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
//
typedef NSCam::IVendorTagDescriptor::VendorTag          VendorTag_t;
typedef NSCam::IVendorTagDescriptor::VendorTagSection   VendorTagSection_t;

/******************************************************************************
 *
 ******************************************************************************/
#define _TAG_(_tagId_, _tagName_, _tagType_) \
    { \
        static_cast<uint32_t>(_tagId_), \
        { \
            .tagName = _tagName_, \
            .tagId   = static_cast<uint32_t>(_tagId_), \
            .tagType = static_cast<uint32_t>(_tagType_), \
        }, \
    }


#define _SECTION_(_sectionName_, _tagStart_, _tagEnd_, _tags_) \
    { \
        .sectionName= _sectionName_, \
        .tags       = _tags_, \
        .tagStart   = static_cast<uint32_t>(_tagStart_), \
        .tagEnd     = static_cast<uint32_t>(_tagEnd_), \
    }

// for custom vendortags
#include <custom_metadata/custom_vendortag_table.h>

/******************************************************************************
 *
 ******************************************************************************/
static auto& _FaceFeature_()
{
    static const std::map<uint32_t, VendorTag_t>
    sInst = {
        _TAG_(MTK_FACE_FEATURE_GESTURE_MODE,
            "gesturemode",   TYPE_INT32),
        _TAG_(MTK_FACE_FEATURE_GESTURE_RESULT,
            "gestureresult",   TYPE_INT32),
        _TAG_(MTK_FACE_FEATURE_SMILE_DETECT_MODE,
            "smiledetectmode",   TYPE_INT32),
        _TAG_(MTK_FACE_FEATURE_SMILE_DETECT_RESULT,
            "smiledetectresult",   TYPE_INT32),
        _TAG_(MTK_FACE_FEATURE_ASD_MODE,
            "asdmode",   TYPE_INT32),
        _TAG_(MTK_FACE_FEATURE_ASD_RESULT,
            "asdresult",   TYPE_INT32),
        _TAG_(MTK_FACE_FEATURE_AVAILABLE_GESTURE_MODES,
            "availablegesturemodes",   TYPE_INT32),
        _TAG_(MTK_FACE_FEATURE_AVAILABLE_SMILE_DETECT_MODES,
            "availablesmiledetectmodes",   TYPE_INT32),
        _TAG_(MTK_FACE_FEATURE_AVAILABLE_ASD_MODES,
            "availableasdmodes",   TYPE_INT32),
        _TAG_(MTK_FACE_FEATURE_FORCE_FACE_3A,
            "forceface3a",   TYPE_INT32),
        _TAG_(MTK_FACE_FEATURE_AVAILABLE_FORCE_FACE_3A,
            "availableforceface3a",   TYPE_INT32),
        _TAG_(MTK_FACE_FEATURE_FACE_ADDITIONAL_RESULT,
            "additionalresult",   TYPE_INT32),
    };
    //
    return sInst;
}

static auto& _NRFeature_()
{
    static const std::map<uint32_t, VendorTag_t>
    sInst = {
        _TAG_(MTK_NR_FEATURE_3DNR_MODE,
            "3dnrmode",   TYPE_INT32),
        _TAG_(MTK_NR_FEATURE_AVAILABLE_3DNR_MODES,
            "available3dnrmodes",   TYPE_INT32),
    };
    //
    return sInst;
}

static auto& _StereoFeature_()
{
    static const std::map<uint32_t, VendorTag_t>
    sInst = {
        _TAG_(MTK_STEREO_FEATURE_DOF_LEVEL,
            "doflevel",    TYPE_INT32),
        _TAG_(MTK_STEREO_FEATURE_SUPPORTED_DOF_LEVEL,
            "supporteddoflevel", TYPE_INT32),
        _TAG_(MTK_STEREO_FEATURE_RESULT_DOF_LEVEL,
            "resultdoflevel", TYPE_INT32),
        _TAG_(MTK_STEREO_FEATURE_WARNING,
            "stereowarning", TYPE_INT32),
        _TAG_(MTK_STEREO_FEATURE_DEPTH_AF_ON,
            "depthafon", TYPE_INT32),
        _TAG_(MTK_STEREO_FEATURE_DISTANCE_MEASURE_ON,
            "distancemeasureon", TYPE_INT32),
        _TAG_(MTK_STEREO_FEATURE_RESULT_DISTANCE,
            "resultdistance", TYPE_FLOAT),
        _TAG_(MTK_STEREO_FEATURE_TOUCH_POSITION,
            "touchposition", TYPE_INT32),
        _TAG_(MTK_STEREO_FEATURE_SHOT_MODE,
            "shotmode", TYPE_BYTE),
        _TAG_(MTK_STEREO_FEATURE_STATE,
            "stereostatus", TYPE_BYTE),
    };
    //
    return sInst;
}

static auto& _HDRFeature_()
{
    static const std::map<uint32_t, VendorTag_t>
    sInst = {
        _TAG_(MTK_HDR_FEATURE_HDR_MODE,
            "hdrMode",   TYPE_INT32),
        _TAG_(MTK_HDR_FEATURE_SESSION_PARAM_HDR_MODE,
            "SessionParamhdrMode",   TYPE_INT32),
        _TAG_(MTK_HDR_FEATURE_HDR_DETECTION_RESULT,
            "hdrDetectionResult",   TYPE_INT32),
        _TAG_(MTK_HDR_FEATURE_AVAILABLE_HDR_MODES_PHOTO,
            "availableHdrModesPhoto",   TYPE_INT32),
        _TAG_(MTK_HDR_FEATURE_AVAILABLE_HDR_MODES_VIDEO,
            "availableHdrModesVideo",   TYPE_INT32),
        _TAG_(MTK_HDR_FEATURE_AVAILABLE_VHDR_MODES,
            "availableVhdrModes",   TYPE_INT32),
        _TAG_(MTK_HDR_FEATURE_AVAILABLE_MSTREAM_HDR_MODES,
            "availableMStreamHdrModes",   TYPE_INT32),
        _TAG_(MTK_HDR_FEATURE_AVAILABLE_SINGLE_FRAME_HDR,
            "availableSingleFrameHdr",   TYPE_BYTE),
    };
    //
    return sInst;
}

static auto& _EISFeature_()
{
    static const std::map<uint32_t, VendorTag_t>
    sInst = {
        _TAG_(MTK_EIS_FEATURE_EIS_MODE,
            "eismode",   TYPE_INT32),
        _TAG_(MTK_EIS_FEATURE_ISNEED_OVERRIDE_TIMESTAMP,
            "isNeedOverrideTimestamp",   TYPE_BYTE),
        _TAG_(MTK_EIS_FEATURE_NEW_SHUTTER_TIMESTAMP,
            "newShutterTimestamp",   TYPE_INT64),
        _TAG_(MTK_EIS_FEATURE_PREVIEW_EIS,
            "previeweis",   TYPE_INT32),
    };
    //
    return sInst;
}

static auto& _StreamingFeature_()
{
    static const std::map<uint32_t, VendorTag_t>
    sInst = {
        _TAG_(MTK_STREAMING_FEATURE_RECORD_STATE,
            "recordState",   TYPE_INT32),
        _TAG_(MTK_STREAMING_FEATURE_AVAILABLE_RECORD_STATES,
            "availableRecordStates",   TYPE_INT32),
        _TAG_(MTK_STREAMING_FEATURE_HFPS_MODE,
            "hfpsMode",   TYPE_INT32),
        _TAG_(MTK_STREAMING_FEATURE_AVAILABLE_HFPS_MODES,
            "availableHfpsModes",   TYPE_INT32),
        _TAG_(MTK_STREAMING_FEATURE_AVAILABLE_HFPS_MAX_RESOLUTIONS,
            "availableHfpsMaxResolutions",   TYPE_INT32),
    };
    //
    return sInst;
}

static auto& _MFNRFeature_()
{
    static const std::map<uint32_t, VendorTag_t>
    sInst = {
        _TAG_(MTK_MFNR_FEATURE_MFB_MODE,
            "mfbmode",   TYPE_INT32),
        _TAG_(MTK_MFNR_FEATURE_MFB_RESULT,
            "mfbresult",   TYPE_INT32),
        _TAG_(MTK_MFNR_FEATURE_AVAILABLE_MFB_MODES,
            "availablemfbmodes",   TYPE_INT32),
        _TAG_(MTK_MFNR_FEATURE_AIS_MODE,
            "aismode",   TYPE_INT32),
        _TAG_(MTK_MFNR_FEATURE_AIS_RESULT,
            "aisresult",   TYPE_INT32),
        _TAG_(MTK_MFNR_FEATURE_AVAILABLE_AIS_MODES,
            "availableaismodes",   TYPE_INT32),
        _TAG_(MTK_MFNR_FEATURE_BSS_GOLDEN_INDEX,
            "goldenindex",   TYPE_INT32),
    };
    //
    return sInst;
}

static auto& _3AFeature_()
{
    static const std::map<uint32_t, VendorTag_t>
    sInst = {
        _TAG_(MTK_3A_FEATURE_AE_ROI,
            "aeroi",    TYPE_INT32),
        _TAG_(MTK_3A_FEATURE_AF_ROI,
            "afroi",    TYPE_INT32),
        _TAG_(MTK_3A_FEATURE_AWB_ROI,
            "awbroi",   TYPE_INT32),
        _TAG_(MTK_3A_FEATURE_AE_REQUEST_ISO_SPEED,
            "aeIsoSpeed",   TYPE_INT32),
        _TAG_(MTK_3A_FEATURE_AE_AVAILABLE_METERING,
            "aeAvailableMetering",   TYPE_BYTE),
        _TAG_( MTK_3A_FEATURE_AE_PLINE_INDEX,
            "aePlineIndex",   TYPE_INT32),
        _TAG_( MTK_3A_FEATURE_AE_CUSTOM_PLINE_MODE,
            "aeCustomPlineMode",   TYPE_INT32),
        _TAG_(MTK_3A_FEATURE_AE_REQUEST_METERING_MODE,
            "aeMeteringMode",   TYPE_BYTE),
        _TAG_(MTK_3A_FEATURE_AE_AVERAGE_BRIGHTNESS,
            "aeAverageBrightness",   TYPE_INT32),
        _TAG_(MTK_3A_FEATURE_AWB_AVAILABL_RANGE,
            "awbAvailableRange",   TYPE_INT32),
        _TAG_(MTK_3A_FEATURE_AWB_REQUEST_VALUE,
            "awbValue",   TYPE_INT32),
        _TAG_(MTK_3A_FEATURE_AE_SENSOR_GAIN_VALUE,
            "aeSensorGain",   TYPE_INT32),
        _TAG_(MTK_3A_FEATURE_AE_ISP_GAIN_VALUE,
            "aeIspGain",   TYPE_INT32),
        _TAG_(MTK_3A_FEATURE_AE_LUX_INDEX_VALUE,
            "aeLuxIndex",   TYPE_INT32),
        _TAG_(MTK_3A_FEATURE_ADRC_GAIN_VALUE,
            "adrcGain",   TYPE_INT32),
    };
    //
    return sInst;
}

static auto& _VSDOFFeature_()
{
    static const std::map<uint32_t, VendorTag_t>
    sInst = {
        _TAG_(MTK_VSDOF_FEATURE_CALLBACK_BUFFERS,
            "vsdofCallbackBuffers",    TYPE_BYTE),
        _TAG_(MTK_VSDOF_FEATURE_DENOISE_MODE,
            "vsdofDenoiseMode",    TYPE_BYTE),
        _TAG_(MTK_VSDOF_FEATURE_DENOISE_SUPPORTED_MODULE,
            "vsdofDenoiseSupportedModule",   TYPE_BYTE),
        _TAG_(MTK_VSDOF_FEATURE_3RDPARTY_MODE,
            "vsdof3rdPartyModule",    TYPE_BYTE),
        _TAG_(MTK_VSDOF_FEATURE_3RDPARTY_SUPPORTED_MODULE,
            "vsdof3rdPartySupportedModule",    TYPE_BYTE),
        _TAG_(MTK_VSDOF_FEATURE_PREVIEW_ENABLE,
            "vsdofPreviewEnable",    TYPE_BYTE),
        _TAG_(MTK_VSDOF_FEATURE_SHOT_MODE,
            "vsdofShotMode",    TYPE_BYTE),
        _TAG_(MTK_VSDOF_FEATURE_PREVIEW_SIZE,
            "vsdofFeaturePreviewSize",   TYPE_INT32),
        _TAG_(MTK_VSDOF_FEATURE_PREVIEW_MODE,
            "vsdofFeaturePreviewMode",   TYPE_INT32),
        _TAG_(MTK_VSDOF_FEATURE_AVAILABLE_PREVIEW_MODE,
            "availableVsdofFeaturePreviewMode",   TYPE_INT32),
        _TAG_(MTK_VSDOF_FEATURE_WARNING,
            "vsdofFeatureWarning",   TYPE_INT32),
        _TAG_(MTK_VSDOF_FEATURE_CAPTURE_WARNING_MSG,
            "vsdofFeatureCaptureWarningMsg",   TYPE_INT32),
    };
    //
    return sInst;
}


static auto& _CShotFeature_()
{
    static const std::map<uint32_t, VendorTag_t>
    sInst = {
        _TAG_(MTK_CSHOT_FEATURE_AVAILABLE_MODES,
            "availableCShotModes",    TYPE_INT32),
        _TAG_(MTK_CSHOT_FEATURE_CAPTURE,
            "capture",    TYPE_INT32),
    };
    //
    return sInst;
}


static auto& _ControlCapture_()
{
    static const std::map<uint32_t, VendorTag_t>
    sInst = {
        _TAG_(MTK_CONTROL_CAPTURE_EARLY_NOTIFICATION_SUPPORT,
            "early.notification.support",    TYPE_INT32),
        _TAG_(MTK_CONTROL_CAPTURE_EARLY_NOTIFICATION_TRIGGER,
            "early.notification.trigger",    TYPE_INT32),
        _TAG_(MTK_CONTROL_CAPTURE_NEXT_READY,
            "next.ready",    TYPE_INT32),
        _TAG_(MTK_CONTROL_CAPTURE_JPEG_FLIP_MODE,
            "flipmode",     TYPE_INT32),
        _TAG_(MTK_CONTROL_CAPTURE_AVAILABLE_POSTVIEW_MODES,
            "availablepostviewmodes",    TYPE_INT32),
        _TAG_(MTK_CONTROL_CAPTURE_POSTVIEW_SIZE,
            "postviewsize",    TYPE_INT32),
        _TAG_(MTK_CONTROL_CAPTURE_REMOSAIC_EN,
            "remosaicenable",    TYPE_INT32),
        _TAG_(MTK_CONTROL_CAPTURE_AVAILABLE_ZSL_MODES,
            "available.zsl.modes",    TYPE_BYTE),
        _TAG_(MTK_CONTROL_CAPTURE_DEFAULT_ZSL_MODE,
            "default.zsl.mode",    TYPE_BYTE),
        _TAG_(MTK_CONTROL_CAPTURE_ZSL_MODE,
            "zsl.mode",    TYPE_BYTE),
        _TAG_(MTK_CONTROL_CAPTURE_SINGLE_YUV_NR,
            "singleYuvNr", TYPE_INT32),
        _TAG_(MTK_CONTROL_CAPTURE_HIGH_QUALITY_YUV,
            "highQualityYuv", TYPE_INT32),
        _TAG_(MTK_CONTROL_CAPTURE_HINT_FOR_RAW_REPROCESS,
            "hintForRawReprocess", TYPE_BYTE),
        _TAG_(MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING,
            "hintForIspTuning", TYPE_INT32),
        _TAG_(MTK_CONTROL_CAPTURE_HINT_FOR_ISP_FRAME_COUNT,
            "hintForIspFrameCount", TYPE_INT32),
        _TAG_(MTK_CONTROL_CAPTURE_HINT_FOR_ISP_FRAME_INDEX,
            "hintForIspFrameIndex", TYPE_INT32),
        _TAG_(MTK_CONTROL_CAPTURE_HINT_FOR_ISP_FRAME_TUNING_INDEX,
            "hintForIspFrameTuningIndex", TYPE_INT64),
        _TAG_(MTK_CONTROL_CAPTURE_ISP_TUNING_DATA_ENABLE,
            "ispMetaEnable", TYPE_BYTE),
        _TAG_(MTK_CONTROL_CAPTURE_ISP_TUNING_DATA_SIZE_FOR_RAW,
            "ispMetaSizeForRaw", TYPE_INT32),
        _TAG_(MTK_CONTROL_CAPTURE_ISP_TUNING_DATA_SIZE_FOR_YUV,
            "ispMetaSizeForYuv", TYPE_INT32),
        _TAG_(MTK_CONTROL_CAPTURE_ISP_TUNING_REQUEST,
            "ispTuningRequest", TYPE_BYTE),
        _TAG_(MTK_CONTROL_CAPTURE_ISP_TUNING_DATA_RAW,
            "ispTuningDataRaw", TYPE_BYTE),
        _TAG_(MTK_CONTROL_CAPTURE_ISP_TUNING_DATA_YUV,
            "ispTuningDataYuv", TYPE_BYTE),
        _TAG_(MTK_CONTROL_CAPTURE_PACKED_RAW_ENABLE,
            "packedRaw.enable", TYPE_INT32),
        _TAG_(MTK_CONTROL_CAPTURE_PROCESS_RAW_ENABLE,
            "processRaw.enable", TYPE_INT32),
        _TAG_(MTK_CONTROL_CAPTURE_RAW_BPP,
            "raw.bpp", TYPE_INT32),
        _TAG_(MTK_CONTROL_CAPTURE_YUV_NO_MARGIN,
            "yuvNoMargin",   TYPE_BYTE),
    };
    //
    return sInst;
}

static auto& _BgServiceFeature_()
{
    static const std::map<uint32_t, VendorTag_t>
    sInst = {
        _TAG_(MTK_BGSERVICE_FEATURE_PRERELEASE_AVAILABLE_MODES,
            "availableprereleasemodes",    TYPE_INT32),
        _TAG_(MTK_BGSERVICE_FEATURE_PRERELEASE,
            "prerelease",    TYPE_INT32),
        _TAG_(MTK_BGSERVICE_FEATURE_IMAGEREADERID,
            "imagereaderid",    TYPE_INT32),
    };
    //
    return sInst;
}

static auto& _ConfigureSetting_()
{
    static const std::map<uint32_t, VendorTag_t>
    sInst = {
        _TAG_(MTK_CONFIGURE_SETTING_INIT_REQUEST,
            "initrequest",    TYPE_INT32),
        _TAG_(MTK_CONFIGURE_ISP_YUV_DIRECT_JPEG,
            "yuvdirectjpeg",    TYPE_INT32),
        _TAG_(MTK_CONFIGURE_SETTING_PROPRIETARY,
            "proprietaryRequest",    TYPE_INT32),
    };
    //
    return sInst;
}

static auto& _MultiCamFeature_()
{
    static const std::map<uint32_t, VendorTag_t>
    sInst = {
        _TAG_(MTK_MULTI_CAM_FEATURE_MODE,
            "multiCamFeatureMode",    TYPE_INT32),
        _TAG_(MTK_MULTI_CAM_FEATURE_AVAILABLE_MODE,
            "availableMultiCamFeatureMode",   TYPE_INT32),
        _TAG_(MTK_MULTI_CAM_FEATURE_SENSOR_MANUAL_UPDATED,
            "availableMultiCamFeatureSensorManualUpdated",   TYPE_INT64),
        _TAG_(MTK_MULTI_CAM_ZOOM_RANGE,
            "multiCamZoomRange",   TYPE_FLOAT),
        _TAG_(MTK_MULTI_CAM_ZOOM_VALUE,
            "multiCamZoomValue",   TYPE_FLOAT),
        _TAG_(MTK_MULTI_CAM_ZOOM_STEPS,
            "multiCamZoomSteps",   TYPE_FLOAT),
        _TAG_(MTK_MULTI_CAM_AF_ROI,
            "multiCamAfRoi",   TYPE_INT32),
        _TAG_(MTK_MULTI_CAM_MASTER_ID,
            "multiCamMasterId",   TYPE_INT32),
        _TAG_(MTK_MULTI_CAM_FOV_CROP_REGION,
            "multiCamFovCropRegion",   TYPE_INT32),
        _TAG_(MTK_MULTI_CAM_SENSOR_CROP_REGION,
            "multiCamSensorCropRegion",   TYPE_INT32),
        _TAG_(MTK_MULTI_CAM_STREAMING_ID,
            "multiCamStreamingId",   TYPE_INT32),
    };
    //
    return sInst;
}

static auto& _SMVRFeature_()
{
    static const std::map<uint32_t, VendorTag_t>
    sInst = {
        _TAG_(MTK_SMVR_FEATURE_SMVR_MODE,
            "smvrMode",   TYPE_INT32),
        _TAG_(MTK_SMVR_FEATURE_AVAILABLE_SMVR_MODES,
            "availableSmvrModes",   TYPE_INT32),
        _TAG_(MTK_SMVR_FEATURE_SMVR_RESULT,
            "smvrResult",   TYPE_INT32),
    };
    //
    return sInst;
}

static auto& _ABFFeature_()
{
    static const std::map<uint32_t, VendorTag_t>
    sInst = {
        _TAG_(MTK_ABF_FEATURE_ABF_MODE,
            "abfMode",   TYPE_INT32),
        _TAG_(MTK_ABF_FEATURE_AVAILABLE_ABF_MODES,
            "availableabfmode",   TYPE_INT32),
        _TAG_(MTK_ABF_FEATURE_ABF_RESULT,
            "abf",   TYPE_INT32),
    };
    //
    return sInst;
}

static auto& _FlashFeature_()
{
    static const std::map<uint32_t, VendorTag_t>
    sInst = {
        _TAG_(MTK_FLASH_FEATURE_CALIBRATION_AVAILABLE,
            "calibration.available",   TYPE_INT32),
        _TAG_(MTK_FLASH_FEATURE_CALIBRATION_ENABLE,
            "calibration.enable",   TYPE_INT32),
        _TAG_(MTK_FLASH_FEATURE_CALIBRATION_RESULT,
            "calibration.result",   TYPE_INT32),
        _TAG_(MTK_FLASH_FEATURE_CUSTOMIZATION_AVAILABLE,
            "customization.available",   TYPE_BYTE),
        _TAG_(MTK_FLASH_FEATURE_CUSTOMIZED_RESULT,
            "customizedResult",   TYPE_BYTE),
        _TAG_(MTK_FLASH_FEATURE_CUSTOMIZED_TORCH_DUTY,
            "customizedTorchDuty",   TYPE_BYTE),
     };
     //
     return sInst;
}



/******************************************************************************
 *
 ******************************************************************************/
static auto& getGlobalSections()
{
    static std::vector<VendorTagSection_t>
    sMTKSections = {
        _SECTION_( "com.mediatek.facefeature",
                    MTK_FACE_FEATURE_START,
                    MTK_FACE_FEATURE_END,
                    _FaceFeature_() ),

        _SECTION_( "com.mediatek.nrfeature",
                    MTK_NR_FEATURE_START,
                    MTK_NR_FEATURE_END,
                    _NRFeature_() ),

        _SECTION_( "com.mediatek.stereofeature",
                    MTK_STEREO_FEATURE_START,
                    MTK_STEREO_FEATURE_END,
                    _StereoFeature_() ),

        _SECTION_( "com.mediatek.hdrfeature",
                    MTK_HDR_FEATURE_START,
                    MTK_HDR_FEATURE_END,
                    _HDRFeature_() ),

        _SECTION_( "com.mediatek.eisfeature",
                    MTK_EIS_FEATURE_START,
                    MTK_EIS_FEATURE_END,
                    _EISFeature_() ),

        _SECTION_( "com.mediatek.streamingfeature",
                    MTK_STREAMING_FEATURE_START,
                    MTK_STREAMING_FEATURE_END,
                    _StreamingFeature_() ),

        _SECTION_( "com.mediatek.mfnrfeature",
                    MTK_MFNR_FEATURE_START,
                    MTK_MFNR_FEATURE_END,
                    _MFNRFeature_() ),

        _SECTION_( "com.mediatek.3afeature",
                    MTK_3A_FEATURE_START,
                    MTK_3A_FEATURE_END,
                    _3AFeature_() ),

        _SECTION_( "com.mediatek.vsdoffeature",
                    MTK_VSDOF_FEATURE_START,
                    MTK_VSDOF_FEATURE_END,
                    _VSDOFFeature_() ),

        _SECTION_( "com.mediatek.cshotfeature",
                    MTK_CSHOT_FEATURE_START,
                    MTK_CSHOT_FEATURE_END,
                    _CShotFeature_() ),

        _SECTION_( "com.mediatek.control.capture",
                    MTK_CONTROL_CAPTURE_START,
                    MTK_CONTROL_CAPTURE_END,
                    _ControlCapture_() ),

        _SECTION_( "com.mediatek.bgservicefeature",
                    MTK_BGSERVICE_FEATURE_START,
                    MTK_BGSERVICE_FEATURE_END,
                    _BgServiceFeature_() ),

        _SECTION_( "com.mediatek.configure.setting",
                    MTK_CONFIGURE_SETTING_START,
                    MTK_CONFIGURE_SETTING_END,
                    _ConfigureSetting_() ),

        _SECTION_( "com.mediatek.multicamfeature",
                    MTK_MULTI_CAM_FEATURE_START,
                    MTK_MULTI_CAM_FEATURE_END,
                    _MultiCamFeature_() ),

        _SECTION_( "com.mediatek.flashfeature",
                    MTK_FLASH_FEATURE_START,
                    MTK_FLASH_FEATURE_END,
                   _FlashFeature_() ),
        _SECTION_( "com.mediatek.smvrfeature",
                    MTK_SMVR_FEATURE_START,
                    MTK_SMVR_FEATURE_END,
                    _SMVRFeature_() ),
        _SECTION_( "com.mediatek.abffeature",
                    MTK_ABF_FEATURE_START,
                    MTK_ABF_FEATURE_END,
                    _ABFFeature_() ),
    };

    // append custom vendor tags sections to mtk sections
    static const std::vector<VendorTagSection_t>
    sCustomSections = getCustomGlobalSections();
    sMTKSections.insert(sMTKSections.end(), sCustomSections.begin(), sCustomSections.end());
    static const std::vector<VendorTagSection_t> sGlobalSections(sMTKSections);

    return sGlobalSections;
}
