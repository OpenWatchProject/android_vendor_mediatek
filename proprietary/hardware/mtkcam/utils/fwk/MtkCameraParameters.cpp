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

/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#define LOG_TAG "MTKCameraParams"
#include <utils/Log.h>
#include <mtkcam/utils/std/ULog.h>

#include <string.h>
#include <stdlib.h>
#include <mtkcam/utils/fwk/MtkCameraParameters.h>
#include <mtkcam/def/ImageFormat.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_UTILITY);

namespace android {

int g_nLastPreviewWidth     = -1;
int g_nLastPreviewHeight    = -1;

void MtkCameraParameters::setPreviewSize(int width, int height)
{
    CameraParameters::setPreviewSize(width, height);

    g_nLastPreviewWidth  = width;
    g_nLastPreviewHeight = height;
    if(width < 0 || height < 0) {
        CAM_ULOGMW("Set preview size to %dx%d", width, height);
    }
}

void MtkCameraParameters::getPreviewSize(int *width, int *height) const
{
    CameraParameters::getPreviewSize(width, height);

    if (*width < 0 || *height < 0) {
        CAM_ULOGMW("Cannot get KEY_PREVIEW_SIZE(%s)", KEY_PREVIEW_SIZE);

        if(g_nLastPreviewWidth > 0 && g_nLastPreviewHeight > 0) {
            CAM_ULOGMW("Use last preview size: %dx%d", g_nLastPreviewWidth, g_nLastPreviewHeight);
            *width  = g_nLastPreviewWidth;
            *height = g_nLastPreviewHeight;
        }
    }
}


/******************************************************************************
 *  String-Enum Format Map Manager
 ******************************************************************************/
namespace {
using namespace NSCam;
struct  StrEnumMapMgr
{
public:     ////            Data Members.
    typedef android::DefaultKeyedVector<android::String8, EImageFormat>  MapType;
    MapType                 mvMapper;

public:     ////            Instantiation.
                            StrEnumMapMgr()
                                : mvMapper(eImgFmt_UNKNOWN)
                            {
                                mvMapper.clear();
                                //
#define DO_MAP(_format_, _eImageFormat) \
            do { \
                mvMapper.add(android::String8(android::MtkCameraParameters::PIXEL_FORMAT_##_format_), eImgFmt_##_eImageFormat); \
            } while (0)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            DO_MAP(YUV420SP         , NV21);
                            DO_MAP(YUV420SP_NV12    , NV12);
                            DO_MAP(YUV420P          , YV12);
                            DO_MAP(YV12_GPU         , YV12);
                            DO_MAP(YUV420I          , I420);
                            DO_MAP(YUV422SP         , NV16);
                            DO_MAP(YUV422I          , YUY2);
                            DO_MAP(YUV422I_YVYU     , YVYU);
                            DO_MAP(YUV422I_UYVY     , UYVY);
                            DO_MAP(YUV422I_VYUY     , VYUY);
                            DO_MAP(RGB565           , RGB565);
                            DO_MAP(RGBA8888         , RGBA8888);
                            DO_MAP(BAYER8           , BAYER8);
                            DO_MAP(BAYER10          , BAYER10);
                            DO_MAP(JPEG             , JPEG);
                            DO_MAP(RGB_888          , RGB888);
                            DO_MAP(YCrCb_420_SP     , NV21);
//------------------------------------------------------------------------------
#undef  DO_MAP
                            }
};
};
static  StrEnumMapMgr       gStrEnumMap;


/*****************************************************************************
 * @brief Query the image format constant.
 *
 * @details Given a MtkCameraParameters::PIXEL_FORMAT_xxx, return its
 * corresponding image format constant.
 *
 * @note
 *
 * @param[in] s8PixelFormat: A String8 string for pixel format (i.e.
 * String8(MtkCameraParameters::PIXEL_FORMAT_xxx))
 *
 * @param[in] szPixelFormat: A null-terminated string for pixel format (i.e.
 * MtkCameraParameters::PIXEL_FORMAT_xxx)
 *
 * @return its corresponding image format.
 *
 ******************************************************************************/
int MtkCameraParameters::queryImageFormat(String8 const& s8PixelFormat)
{
    int format = gStrEnumMap.mvMapper.valueFor(s8PixelFormat);
    ALOGW_IF(
        eImgFmt_UNKNOWN==format,
        "Unsupported MtkCameraParameters::PIXEL_FORMAT_xxx (%s)", s8PixelFormat.string()
    );
    return  format;
}


int MtkCameraParameters::queryImageFormat(char const* szPixelFormat)
{
    LOG_FATAL_IF(NULL==szPixelFormat, "NULL==szPixelFormat");
    return MtkCameraParameters::queryImageFormat(android::String8(szPixelFormat));
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  App Mode.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
const char MtkCameraParameters::PROPERTY_KEY_CLIENT_APPMODE[]   = "vendor.mtk.client.appmode";
//
const char MtkCameraParameters::APP_MODE_NAME_DEFAULT[]         = "Default";
const char MtkCameraParameters::APP_MODE_NAME_MTK_ENG[]         = "MtkEng";
const char MtkCameraParameters::APP_MODE_NAME_MTK_ATV[]         = "MtkAtv";
const char MtkCameraParameters::APP_MODE_NAME_MTK_STEREO[]      = "MtkStereo";
const char MtkCameraParameters::APP_MODE_NAME_MTK_VT[]          = "MtkVt";
const char MtkCameraParameters::APP_MODE_NAME_MTK_PHOTO[]       = "MtkPhoto";
const char MtkCameraParameters::APP_MODE_NAME_MTK_VIDEO[]       = "MtkVideo";
const char MtkCameraParameters::APP_MODE_NAME_MTK_ZSD[]         = "MtkZsd";
const char MtkCameraParameters::APP_MODE_NAME_MTK_DUALCAM[]    = "MtkDualCam";


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Scene Mode
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
const char MtkCameraParameters::SCENE_MODE_NORMAL[] = "normal";

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Face Beauty
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
const char MtkCameraParameters::KEY_FB_SMOOTH_LEVEL[]       = "fb-smooth-level";
const char MtkCameraParameters::KEY_FB_SMOOTH_LEVEL_MIN[]   = "fb-smooth-level-min";
const char MtkCameraParameters::KEY_FB_SMOOTH_LEVEL_MAX[]   = "fb-smooth-level-max";
const char MtkCameraParameters::KEY_FB_SMOOTH_LEVEL_Default[]   = "fb-smooth-level-default";
//
const char MtkCameraParameters::KEY_FB_SKIN_COLOR[]         = "fb-skin-color";
const char MtkCameraParameters::KEY_FB_SKIN_COLOR_MIN[]     = "fb-skin-color-min";
const char MtkCameraParameters::KEY_FB_SKIN_COLOR_MAX[]     = "fb-skin-color-max";
const char MtkCameraParameters::KEY_FB_SKIN_COLOR_Default[]     = "fb-skin-color-default";
//
const char MtkCameraParameters::KEY_FB_SHARP[]              = "fb-sharp";
const char MtkCameraParameters::KEY_FB_SHARP_MIN[]          = "fb-sharp-min";
const char MtkCameraParameters::KEY_FB_SHARP_MAX[]          = "fb-sharp-max";
//
const char MtkCameraParameters::KEY_FB_ENLARGE_EYE[]        = "fb-enlarge-eye";
const char MtkCameraParameters::KEY_FB_ENLARGE_EYE_MIN[]    = "fb-enlarge-eye-min";
const char MtkCameraParameters::KEY_FB_ENLARGE_EYE_MAX[]    = "fb-enlarge-eye-max";
//
const char MtkCameraParameters::KEY_FB_SLIM_FACE[]          = "fb-slim-face";
const char MtkCameraParameters::KEY_FB_SLIM_FACE_MIN[]      = "fb-slim-face-min";
const char MtkCameraParameters::KEY_FB_SLIM_FACE_MAX[]      = "fb-slim-face-max";
//
const char MtkCameraParameters::KEY_FB_EXTREME_BEAUTY[]     = "fb-extreme-beauty";
//
const char MtkCameraParameters::KEY_FB_TOUCH_POS[]          = "fb-touch-pos";
//
const char MtkCameraParameters::KEY_FB_FACE_POS[]           = "fb-face-pos";
//
const char MtkCameraParameters::KEY_FACE_BEAUTY[]           = "face-beauty";
//
const char MtkCameraParameters::KEY_FB_EXTREME_SUPPORTED[]  = "fb-extreme-beauty-supported";
//
const char MtkCameraParameters::KEY_FEATURE_MAX_FPS[]       = "feature-max-fps";
//
const char MtkCameraParameters::KEY_VIDEO_FACE_BEAUTY_SUPPORTED[] = "vfb-supported";


//
const char MtkCameraParameters::KEY_EXPOSURE[] = "exposure";
const char MtkCameraParameters::KEY_EXPOSURE_METER[] = "exposure-meter";
const char MtkCameraParameters::KEY_EXPOSURE_TIME[] = "exposure-time";
const char MtkCameraParameters::KEY_MAX_EXPOSURE_TIME[] = "max-exposure-time";
const char MtkCameraParameters::KEY_MANUAL_CAP[] = "manual-cap";

const char MtkCameraParameters::KEY_ISO_SPEED[] = "iso-speed";
const char MtkCameraParameters::KEY_AE_MODE[] = "ae-mode";
const char MtkCameraParameters::KEY_FOCUS_METER[] = "focus-meter";
const char MtkCameraParameters::KEY_EDGE[] = "edge";
const char MtkCameraParameters::KEY_HUE[] = "hue";
const char MtkCameraParameters::KEY_SATURATION[] = "saturation";
const char MtkCameraParameters::KEY_BRIGHTNESS[] = "brightness";
const char MtkCameraParameters::KEY_CONTRAST[] = "contrast";
const char MtkCameraParameters::KEY_AF_LAMP_MODE [] = "aflamp-mode";
const char MtkCameraParameters::KEY_STEREO_3D_PREVIEW_SIZE[] = "stereo3d-preview-size";
const char MtkCameraParameters::KEY_STEREO_3D_PICTURE_SIZE[] = "stereo3d-picture-size";
const char MtkCameraParameters::KEY_STEREO_3D_TYPE [] = "stereo3d-type";
const char MtkCameraParameters::KEY_STEREO_3D_MODE [] = "stereo3d-mode";
const char MtkCameraParameters::KEY_STEREO_3D_IMAGE_FORMAT [] = "stereo3d-image-format";

// ZSD
const char MtkCameraParameters::KEY_ZSD_MODE[] = "zsd-mode";
const char MtkCameraParameters::KEY_SUPPORTED_ZSD_MODE[] = "zsd-supported";
//
const char MtkCameraParameters::KEY_DYNAMIC_SWITCH_SENSOR_MODE[] = "dynamic-switch-sensor-mode";
const char MtkCameraParameters::KEY_HIGH_FPS_CAPTURE[] = "high-fps-capture";
const char MtkCameraParameters::KEY_NON_SLOWMOTION_PREVIEW_LIMIT_MAX_FPS[] = "non-slowmotion-preview-limit-max-fps";
const char MtkCameraParameters::KEY_LOW_LIGHT_CAPTURE_USE_PREVIEW_MODE[] = "low-light-capture-use-preview-mode";
//
const char MtkCameraParameters::KEY_FPS_MODE[] = "fps-mode";
//
const char MtkCameraParameters::KEY_FOCUS_DRAW[] = "af-draw";
//
const char MtkCameraParameters::KEY_CAPTURE_MODE[] = "cap-mode";
const char MtkCameraParameters::KEY_SUPPORTED_CAPTURE_MODES[] = "cap-mode-values";
const char MtkCameraParameters::KEY_CAPTURE_PATH[] = "capfname";
const char MtkCameraParameters::KEY_BURST_SHOT_NUM[] = "burst-num";
//
const char MtkCameraParameters::KEY_MATV_PREVIEW_DELAY[] = "tv-delay";
const char MtkCameraParameters::KEY_PANORAMA_IDX[] = "pano-idx";
const char MtkCameraParameters::KEY_PANORAMA_DIR[] = "pano-dir";

// Values for KEY_EXPOSURE
const char MtkCameraParameters::EXPOSURE_METER_SPOT[] = "spot";
const char MtkCameraParameters::EXPOSURE_METER_CENTER[] = "center";
const char MtkCameraParameters::EXPOSURE_METER_AVERAGE[] = "average";

// Values for KEY_EXPOSURE_TIME
const char MtkCameraParameters::EXPOSURE_TIME_AUTO[] = "auto";

// Valeus for KEY_ISO_SPEED
const char MtkCameraParameters::ISO_SPEED_AUTO[] = "auto";
const char MtkCameraParameters::ISO_SPEED_100[] = "100";
const char MtkCameraParameters::ISO_SPEED_200[] = "200";
const char MtkCameraParameters::ISO_SPEED_400[] = "400";
const char MtkCameraParameters::ISO_SPEED_800[] = "800";
const char MtkCameraParameters::ISO_SPEED_1600[] = "1600";

// Values for KEY_AE_MODE = "ae-mode"

// Values for KEY_FOCUS_METER
const char MtkCameraParameters::FOCUS_METER_SPOT[] = "spot";
const char MtkCameraParameters::FOCUS_METER_MULTI[] = "multi";

// AWB2PASS
const char MtkCameraParameters::KEY_AWB2PASS[] = "awb-2pass";


//
//  Camera Mode
const char MtkCameraParameters::KEY_CAMERA_MODE[] = "mtk-cam-mode";
// Values for KEY_CAMERA_MODE
const int MtkCameraParameters::CAMERA_MODE_NORMAL  = 0;
const int MtkCameraParameters::CAMERA_MODE_MTK_PRV = 1;
const int MtkCameraParameters::CAMERA_MODE_MTK_VDO = 2;
const int MtkCameraParameters::CAMERA_MODE_MTK_VT  = 3;

// Values for KEY_FPS_MODE
const int MtkCameraParameters::FPS_MODE_NORMAL = 0;
const int MtkCameraParameters::FPS_MODE_FIX = 1;

// Values for raw save mode

// Values for KEY_FOCUS_DRAW

// Values for capture mode
const char MtkCameraParameters::CAPTURE_MODE_PANORAMA_SHOT[] = "panoramashot";
const char MtkCameraParameters::CAPTURE_MODE_BURST_SHOT[] = "burstshot";
const char MtkCameraParameters::CAPTURE_MODE_NORMAL[] = "normal";
const char MtkCameraParameters::CAPTURE_MODE_BEST_SHOT[] = "bestshot";
const char MtkCameraParameters::CAPTURE_MODE_EV_BRACKET_SHOT[] = "evbracketshot";
const char MtkCameraParameters::CAPTURE_MODE_SMILE_SHOT[] = "smileshot";
const char MtkCameraParameters::CAPTURE_MODE_MAV_SHOT[] = "mav";
const char MtkCameraParameters::CAPTURE_MODE_AUTO_PANORAMA_SHOT[] = "autorama";
const char MtkCameraParameters::CAPTURE_MODE_MOTION_TRACK_SHOT[] = "motiontrack";
const char MtkCameraParameters::CAPTURE_MODE_HDR_SHOT[] = "hdr";
const char MtkCameraParameters::CAPTURE_MODE_ASD_SHOT[] = "asd";
const char MtkCameraParameters::CAPTURE_MODE_ZSD_SHOT[] = "zsd";
const char MtkCameraParameters::CAPTURE_MODE_PANO_3D[] = "pano_3d";
const char MtkCameraParameters::CAPTURE_MODE_SINGLE_3D[] = "single_3d";
const char MtkCameraParameters::CAPTURE_MODE_FACE_BEAUTY[] = "face_beauty";
const char MtkCameraParameters::CAPTURE_MODE_CONTINUOUS_SHOT[] = "continuousshot";
const char MtkCameraParameters::CAPTURE_MODE_MULTI_MOTION[] = "multi_motion";
const char MtkCameraParameters::CAPTURE_MODE_GESTURE_SHOT[] = "gestureshot";

// Values for panorama direction settings
const char MtkCameraParameters::PANORAMA_DIR_RIGHT[] = "right";
const char MtkCameraParameters::PANORAMA_DIR_LEFT[] = "left";
const char MtkCameraParameters::PANORAMA_DIR_TOP[] = "top";
const char MtkCameraParameters::PANORAMA_DIR_DOWN[] = "down";

//
const int MtkCameraParameters::ENABLE = 1;
const int MtkCameraParameters::DISABLE = 0;

// Values for KEY_EDGE, KEY_HUE, KEY_SATURATION, KEY_BRIGHTNESS, KEY_CONTRAST
const char MtkCameraParameters::HIGH[] = "high";
const char MtkCameraParameters::MIDDLE[] = "middle";
const char MtkCameraParameters::LOW[] = "low";

// Preview Internal Format.
const char MtkCameraParameters::KEY_PREVIEW_INT_FORMAT[] = "prv-int-fmt";

// Pixel color formats for KEY_PREVIEW_FORMAT, KEY_PICTURE_FORMAT,
// and KEY_VIDEO_FRAME_FORMAT
const char MtkCameraParameters::PIXEL_FORMAT_YUV420I[] = "yuv420i-yyuvyy-3plane";
const char MtkCameraParameters::PIXEL_FORMAT_YV12_GPU[] = "yv12-gpu";
const char MtkCameraParameters::PIXEL_FORMAT_YUV422I_UYVY[] = "yuv422i-uyvy";
const char MtkCameraParameters::PIXEL_FORMAT_YUV422I_VYUY[] = "yuv422i-vyuy";
const char MtkCameraParameters::PIXEL_FORMAT_YUV422I_YVYU[] = "yuv422i-yvyu";

const char MtkCameraParameters::PIXEL_FORMAT_BAYER8[] = "bayer8";
const char MtkCameraParameters::PIXEL_FORMAT_BAYER10[] = "bayer10";

const char MtkCameraParameters::PIXEL_FORMAT_BITSTREAM[] = "bitstream";
const char MtkCameraParameters::PIXEL_FORMAT_YUV420SP_NV12[] = "yuv420sp-nv12";

const char MtkCameraParameters::PIXEL_FORMAT_RGB_888[] = "rgb-888";
const char MtkCameraParameters::PIXEL_FORMAT_YCrCb_420_SP[] = "ycrcb-420-sp";

const char MtkCameraParameters::KEY_BRIGHTNESS_VALUE[] = "brightness_value";

// ISP Operation mode for meta mode use
const char MtkCameraParameters::KEY_ISP_MODE[] = "isp-mode";
// AF
const char MtkCameraParameters::KEY_AF_X[] = "af-x";
const char MtkCameraParameters::KEY_AF_Y[] = "af-y";
// Effect
const char MtkCameraParameters::EFFECT_SEPIA_BLUE[] = "sepiablue";
const char MtkCameraParameters::EFFECT_SEPIA_GREEN[] = "sepiagreen";
const char MtkCameraParameters::EFFECT_NASHVILLE[] = "nashville";
const char MtkCameraParameters::EFFECT_HEFE[] = "hefe";
const char MtkCameraParameters::EFFECT_VALENCIA[] = "valencia";
const char MtkCameraParameters::EFFECT_XPROII[] = "xproll";
const char MtkCameraParameters::EFFECT_LOFI[] = "lofi";
const char MtkCameraParameters::EFFECT_SIERRA[] = "sierra";
const char MtkCameraParameters::EFFECT_KELVIN[] = "kelvin";
const char MtkCameraParameters::EFFECT_WALDEN[] = "walden";
const char MtkCameraParameters::EFFECT_F1977[] = "f1977";
//
//  on/off => FIXME: should be replaced with TRUE[]
const char MtkCameraParameters::ON[] = "on";
const char MtkCameraParameters::OFF[] = "off";
//
const char MtkCameraParameters::WHITE_BALANCE_TUNGSTEN[] = "tungsten";
//
const char MtkCameraParameters::ISO_SPEED_ENG[] = "iso-speed-eng";
const char MtkCameraParameters::KEY_RAW_SAVE_MODE[] = "rawsave-mode";
const char MtkCameraParameters::KEY_RAW_PATH[] = "rawfname";

const char MtkCameraParameters::KEY_FAST_CONTINUOUS_SHOT[] = "fast-continuous-shot";

const char MtkCameraParameters::KEY_CSHOT_INDICATOR[] = "cshot-indicator";

// AF EM MODE
const char MtkCameraParameters::KEY_FOCUS_ENG_MODE[]        = "afeng-mode";
const char MtkCameraParameters::KEY_FOCUS_ENG_STEP[]        = "afeng-pos";
const char MtkCameraParameters::KEY_FOCUS_ENG_MAX_STEP[]    = "afeng-max-focus-step";
const char MtkCameraParameters::KEY_FOCUS_ENG_MIN_STEP[]    = "afeng-min-focus-step";
const char MtkCameraParameters::KEY_FOCUS_ENG_BEST_STEP[]   = "afeng-best-focus-step";
const char MtkCameraParameters::KEY_RAW_DUMP_FLAG[]         = "afeng_raw_dump_flag";
const char MtkCameraParameters::KEY_PREVIEW_DUMP_RESOLUTION[] = "prv-dump-res";
// Values for KEY_PREVIEW_DUMP_RESOLUTION
const int MtkCameraParameters::PREVIEW_DUMP_RESOLUTION_NORMAL  = 0;
const int MtkCameraParameters::PREVIEW_DUMP_RESOLUTION_CROP  = 1;
//
const char MtkCameraParameters::KEY_MAX_NUM_DETECTED_OBJECT[] = "max-num-ot";

// HRD
const char MtkCameraParameters::KEY_HEARTBEAT_MONITOR[] = "mtk-heartbeat-monitor";
const char MtkCameraParameters::KEY_HEARTBEAT_MONITOR_SUPPORTED[] = "mtk-heartbeat-monitor-supported";

// KEY for Video HDR
const char MtkCameraParameters::KEY_VIDEO_HDR[] = "video-hdr";
const char MtkCameraParameters::KEY_VIDEO_HDR_MODE[] = "video-hdr-mode";
const char MtkCameraParameters::VIDEO_HDR_MODE_NONE[] = "video-hdr-mode-none";
const char MtkCameraParameters::VIDEO_HDR_MODE_IVHDR[] = "video-hdr-mode-ivhdr";
const char MtkCameraParameters::VIDEO_HDR_MODE_MVHDR[] = "video-hdr-mode-mvhdr";
const char MtkCameraParameters::VIDEO_HDR_MODE_ZVHDR[] = "video-hdr-mode-zvhdr";
const char MtkCameraParameters::VIDEO_HDR_SIZE_DEVISOR[] = "video-hdr-size-devisor";
const char MtkCameraParameters::KEY_SINGLE_FRAME_CAPTURE_HDR_SUPPORTED[] = "single-frame-cap-hdr-supported";

// KEY for MZAF from config table
const char MtkCameraParameters::KEY_SUPPORT_MZAF_FEATURE[] = "support-mzaf-feature";

// HDR Detection
const char MtkCameraParameters::KEY_HDR_DETECTION_SUPPORTED[] = "hdr-detection-supported";
const char MtkCameraParameters::KEY_HDR_AUTO_MODE[] = "hdr-auto-mode";

// HDR mode
// TODO: use this mode to replace KEY_HDR_AUTO_MODE and KEY_VIDEO_HDR
const char MtkCameraParameters::KEY_HDR_MODE[] = "hdr-mode";
const char MtkCameraParameters::HDR_MODE_OFF[] = "off";
const char MtkCameraParameters::HDR_MODE_ON[] = "on";
const char MtkCameraParameters::HDR_MODE_AUTO[] = "auto";
const char MtkCameraParameters::HDR_MODE_VIDEO_ON[] = "video-on";
const char MtkCameraParameters::HDR_MODE_VIDEO_AUTO[] = "video-auto";

// KEY for [Engineer Mode] Add new camera paramters for new requirements
const char MtkCameraParameters::KEY_ENG_AE_ENABLE[] = "ae-e";
const char MtkCameraParameters::KEY_ENG_PREVIEW_SHUTTER_SPEED[] = "prv-ss";
const char MtkCameraParameters::KEY_ENG_PREVIEW_SENSOR_GAIN[] = "prv-sr-g";
const char MtkCameraParameters::KEY_ENG_PREVIEW_ISP_GAIN[] = "prv-isp-g";
const char MtkCameraParameters::KEY_ENG_PREVIEW_AE_INDEX[] = "prv-ae-i";
const char MtkCameraParameters::KEY_ENG_PREVIEW_ISO[]="prv-iso";
const char MtkCameraParameters::KEY_ENG_CAPTURE_SENSOR_GAIN[] = "cap-sr-g";
const char MtkCameraParameters::KEY_ENG_CAPTURE_ISP_GAIN[] = "cap-isp-g";
const char MtkCameraParameters::KEY_ENG_CAPTURE_SHUTTER_SPEED[] = "cap-ss";
const char MtkCameraParameters::KEY_ENG_CAPTURE_ISO[] = "cap-iso";
const char MtkCameraParameters::KEY_ENG_FLASH_DUTY_VALUE[] = "flash-duty-value";
const char MtkCameraParameters::KEY_ENG_FLASH_DUTY_MIN[] = "flash-duty-min";
const char MtkCameraParameters::KEY_ENG_FLASH_DUTY_MAX[] = "flash-duty-max";
const char MtkCameraParameters::KEY_ENG_ZSD_ENABLE[] = "eng-zsd-e";
const char MtkCameraParameters::KEY_SENSOR_TYPE[] = "sensor-type";
const char MtkCameraParameters::KEY_ENG_PREVIEW_FPS[] = "eng-prv-fps";
const char MtkCameraParameters::KEY_ENG_MSG[] = "eng-msg";
const int  MtkCameraParameters::KEY_ENG_FLASH_DUTY_DEFAULT_VALUE = -1;
const int  MtkCameraParameters::KEY_ENG_FLASH_STEP_DEFAULT_VALUE = -1;
const char MtkCameraParameters::KEY_ENG_FLASH_STEP_MIN[] = "flash-step-min";
const char MtkCameraParameters::KEY_ENG_FLASH_STEP_MAX[] = "flash-step-max";
const char MtkCameraParameters::KEY_ENG_FOCUS_FULLSCAN_FRAME_INTERVAL[] = "focus-fs-fi";
const char MtkCameraParameters::KEY_ENG_FOCUS_FULLSCAN_FRAME_INTERVAL_MAX[] = "focus-fs-fi-max";
const char MtkCameraParameters::KEY_ENG_FOCUS_FULLSCAN_FRAME_INTERVAL_MIN[] = "focus-fs-fi-min";
const int  MtkCameraParameters::KEY_ENG_FOCUS_FULLSCAN_FRAME_INTERVAL_MAX_DEFAULT = 65535;
const int  MtkCameraParameters::KEY_ENG_FOCUS_FULLSCAN_FRAME_INTERVAL_MIN_DEFAULT = 0;
const char MtkCameraParameters::KEY_ENG_FOCUS_FULLSCAN_DAC_STEP[] = "focus-fs-dac-step";
const char MtkCameraParameters::KEY_ENG_PREVIEW_FRAME_INTERVAL_IN_US[] = "eng-prv-fius";
const char MtkCameraParameters::KEY_ENG_PARAMETER1[] = "eng-p1";
const char MtkCameraParameters::KEY_ENG_PARAMETER2[] = "eng-p2";
const char MtkCameraParameters::KEY_ENG_PARAMETER3[] = "eng-p3";

//ENG KEY for RAW output port
const char MtkCameraParameters::KEY_ENG_RAW_OUTPUT_PORT[] = "raw-output-port";
const int  MtkCameraParameters::KEY_ENG_RAW_IMGO = 0;
const int  MtkCameraParameters::KEY_ENG_RAW_RRZO = 1;

//ENG KEY for ISP PROFILE
const char MtkCameraParameters::KEY_ENG_ISP_PROFILE[] = "isp-profile";
const int  MtkCameraParameters::KEY_ENG_ISP_PREVIEW = 0;
const int  MtkCameraParameters::KEY_ENG_ISP_CAPTURE = 1;
const int  MtkCameraParameters::KEY_ENG_ISP_VIDEO = 2;

// KEY for [Engineer Mode] Add new camera paramters for ev value
const char MtkCameraParameters::KEY_ENG_EV_VALUE[] = "eng-ev-value";
const char MtkCameraParameters::KEY_ENG_EVB_ENABLE[] = "eng-evb-enable";

//KEY for [Engineer Mode]  3ADB_Flash  precapture dump raw
const char MtkCameraParameters::KEY_ENG_3ADB_FLASH_ENABLE[] = "eng-3adb-flash-enable";


const char MtkCameraParameters::KEY_ENG_SAVE_SHADING_TABLE[] = "eng-s-shad-t";
const char MtkCameraParameters::KEY_ENG_SHADING_TABLE[] = "eng-shad-t";
const int MtkCameraParameters::KEY_ENG_SHADING_TABLE_AUTO = 0;
const int MtkCameraParameters::KEY_ENG_SHADING_TABLE_LOW = 1;
const int MtkCameraParameters::KEY_ENG_SHADING_TABLE_MIDDLE = 2;
const int MtkCameraParameters::KEY_ENG_SHADING_TABLE_HIGH = 3;
const int MtkCameraParameters::KEY_ENG_SHADING_TABLE_TSF = 4;

const char MtkCameraParameters::KEY_VR_BUFFER_COUNT[] = "vr-buf-count";

// KEY for [Engineer Mode] Add new camera paramters for ev calibration
const char MtkCameraParameters::KEY_ENG_EV_CALBRATION_OFFSET_VALUE[] = "ev-cal-o";

// KEY for [Engineer Mode] MFLL: Multi-frame lowlight capture
const char MtkCameraParameters::KEY_ENG_MFLL_SUPPORTED[] = "eng-mfll-s";
const char MtkCameraParameters::KEY_ENG_MFLL_ENABLE[] = "eng-mfll-e";
const char MtkCameraParameters::KEY_ENG_MFLL_PICTURE_COUNT[] = "eng-mfll-pc";

// KEY for [Engineer Mode] Two more sensor mode
const char MtkCameraParameters::KEY_ENG_SENOSR_MODE_SLIM_VIDEO1_SUPPORTED[] = "sv1-s";
const char MtkCameraParameters::KEY_ENG_SENOSR_MODE_SLIM_VIDEO2_SUPPORTED[] = "sv2-s";

// KEY for [Engineer Mode] Video raw dump
const char MtkCameraParameters::KEY_ENG_VIDEO_RAW_DUMP_RESIZE_TO_2M_SUPPORTED[] = "vdr-r2m-s";
const char MtkCameraParameters::KEY_ENG_VIDEO_RAW_DUMP_RESIZE_TO_4K2K_SUPPORTED[] = "vdr-r4k2k-s";
const char MtkCameraParameters::KEY_ENG_VIDEO_RAW_DUMP_CROP_CENTER_2M_SUPPORTED[] = "vdr-cc2m-s";
const char MtkCameraParameters::KEY_ENG_VIDEO_RAW_DUMP_RESIZE[] = "vdr-r";
const char MtkCameraParameters::KEY_ENG_VIDEO_RAW_DUMP_SUPPORTED[] = "vrd-s";

const char MtkCameraParameters::KEY_ENG_VIDEO_RAW_DUMP_MANUAL_FRAME_RATE_SUPPORTED[] = "vrd-mfr-s";
const char MtkCameraParameters::KEY_ENG_VIDEO_RAW_DUMP_MANUAL_FRAME_RATE_ENABLE[] = "vrd-mfr-e";
const char MtkCameraParameters::KEY_ENG_VIDEO_RAW_DUMP_MANUAL_FRAME_RATE_MIN[] = "vrd-mfr-min";
const char MtkCameraParameters::KEY_ENG_VIDEO_RAW_DUMP_MANUAL_FRAME_RATE_MAX[] = "vrd-mfr-max";
const char MtkCameraParameters::KEY_ENG_VIDEO_RAW_DUMP_MANUAL_FRAME_RATE_RANGE_LOW[] = "vrd-mfr-low";
const char MtkCameraParameters::KEY_ENG_VIDEO_RAW_DUMP_MANUAL_FRAME_RATE_RANGE_HIGH[] = "vrd-mfr-high";

const char MtkCameraParameters::KEY_ENG_MTK_AWB_SUPPORTED[] = "mtk-awb-s";
const char MtkCameraParameters::KEY_ENG_SENSOR_AWB_SUPPORTED[] = "sr-awb-s";
const char MtkCameraParameters::KEY_ENG_MTK_AWB_ENABLE[] = "mtk-awb-e";
const char MtkCameraParameters::KEY_ENG_SENSOR_AWB_ENABLE[] = "sr-awb-e";

const char MtkCameraParameters::KEY_ENG_MTK_SHADING_SUPPORTED[] = "mtk-shad-s";
const char MtkCameraParameters::KEY_ENG_MTK_1to3_SHADING_SUPPORTED[] = "mtk-123-shad-s";
const char MtkCameraParameters::KEY_ENG_SENSOR_SHADNING_SUPPORTED[] = "sr-shad-s";
const char MtkCameraParameters::KEY_ENG_MTK_SHADING_ENABLE[] = "mtk-shad-e";
const char MtkCameraParameters::KEY_ENG_MTK_1to3_SHADING_ENABLE[] = "mtk-123-shad-e";
const char MtkCameraParameters::KEY_ENG_SENSOR_SHADNING_ENABLE[] = "sr-shad-e";

const char MtkCameraParameters::KEY_ENG_MANUAL_MULTI_NR_SUPPORTED[] = "mnr-s";
const char MtkCameraParameters::KEY_ENG_MANUAL_MULTI_NR_ENABLE[] = "mnr-e";
const char MtkCameraParameters::KEY_ENG_MANUAL_MULTI_NR_TYPE[] = "mnr-t";
const char MtkCameraParameters::KEY_ENG_VIDEO_HDR_SUPPORTED[] = "vhdr-s";
const char MtkCameraParameters::KEY_ENG_VIDEO_HDR_MODE[]= "vhdr-m";
const char MtkCameraParameters::KEY_ENG_VIDEO_HDR_RATIO[]= "vhdr-ratio";

// Slow motion
const char MtkCameraParameters::KEY_HSVR_PRV_SIZE[] = "hsvr-prv-size";
const char MtkCameraParameters::KEY_SUPPORTED_HSVR_PRV_SIZE[] = "hsvr-prv-size-values";
const char MtkCameraParameters::KEY_HSVR_PRV_FPS[] = "hsvr-prv-fps";
const char MtkCameraParameters::KEY_SUPPORTED_HSVR_PRV_FPS[] = "hsvr-prv-fps-values";
const char MtkCameraParameters::KEY_HSVR_SIZE_FPS[] = "hsvr-size-fps";
const char MtkCameraParameters::KEY_SUPPORTED_HSVR_SIZE_FPS[] = "hsvr-size-fps-values";


// MFB
const char MtkCameraParameters::KEY_MFB_MODE[] = "mfb";
const char MtkCameraParameters::KEY_MFB_MODE_MFLL[] = "mfll";
const char MtkCameraParameters::KEY_MFB_MODE_AIS[] = "ais";
const char MtkCameraParameters::KEY_MFB_MODE_AUTO[] = "auto";

// Parameters for customize
const char MtkCameraParameters::KEY_CUSTOM_HINT[] = "isp-mode";
const char MtkCameraParameters::KEY_CUSTOM_HINT_0[] = "0";
const char MtkCameraParameters::KEY_CUSTOM_HINT_1[] = "1";
const char MtkCameraParameters::KEY_CUSTOM_HINT_2[] = "2";
const char MtkCameraParameters::KEY_CUSTOM_HINT_3[] = "3";
const char MtkCameraParameters::KEY_CUSTOM_HINT_4[] = "4";

//PIP
const char MtkCameraParameters::KEY_PIP_MAX_FRAME_RATE_ZSD_ON[] = "pip-fps-zsd-on";
const char MtkCameraParameters::KEY_PIP_MAX_FRAME_RATE_ZSD_OFF[] = "pip-fps-zsd-off";

// Dynamic Frame Rate
const char MtkCameraParameters::KEY_DYNAMIC_FRAME_RATE[] = "dynamic-frame-rate";
const char MtkCameraParameters::KEY_DYNAMIC_FRAME_RATE_SUPPORTED[] = "dynamic-frame-rate-supported";

// Stereo Feature
const char MtkCameraParameters::KEY_STEREO_REFOCUS_MODE[] = "stereo-image-refocus";
const char MtkCameraParameters::KEY_STEREO_CAPTURE_MODE[] = "stereo-capture-mode";
const char MtkCameraParameters::KEY_STEREO_DEPTHAF_MODE[] = "stereo-depth-af";
const char MtkCameraParameters::KEY_STEREO_DISTANCE_MODE[] = "stereo-distance-measurement";
const char MtkCameraParameters::KEY_STEREO_PICTURE_SIZE[] = "stereo-picture-size";
const char MtkCameraParameters::KEY_SUPPORTED_STEREO_PICTURE_SIZE[] = "stereo-picture-size-values";
const char MtkCameraParameters::KEY_REFOCUS_PICTURE_SIZE[] = "refocus-picture-size";
const char MtkCameraParameters::KEY_SUPPORTED_REFOCUS_PICTURE_SIZE[] = "refocus-picture-size-values";
const char MtkCameraParameters::KEY_STEREO_PREVIEW_FRAME_RATE[] = "stereo-preview-frame-rate";
const char MtkCameraParameters::KEY_STEREO_CAPTURE_FRAME_RATE[] = "stereo-capture-frame-rate";
const char MtkCameraParameters::KEY_STEREO_SENSOR_INDEX_MAIN[]  = "stereo-sensor-index-main";
const char MtkCameraParameters::KEY_STEREO_SENSOR_INDEX_MAIN2[] = "stereo-sensor-index-main2";
const char MtkCameraParameters::KEY_STEREO_CAPTURE_SUPPORTED_MODULE[] = "stereo-capture-supported-module";
const char MtkCameraParameters::REAR[]  = "rear";   //camera module rear
const char MtkCameraParameters::FRONT[] = "front";  //camera module front
const char MtkCameraParameters::KEY_SUPPORTED_STEREO_POSTVIEW_SIZE[] = "stereo-postview-size-values";

// Stereo Feature: VSDoF
const char MtkCameraParameters::KEY_STEREO_VSDOF_MODE[] = "stereo-vsdof-mode";
const char MtkCameraParameters::KEY_STEREO_DOF_LEVEL[] = "stereo-dof-level";
const char MtkCameraParameters::KEY_STEREO_SUPPORTED_DOF_LEVEL[] = "stereo-supported-dof-level";
const char MtkCameraParameters::KEY_STEREO_RESULT_DOF_LEVEL[] = "stereo-result-dof-level";
const char MtkCameraParameters::KEY_STEREO_VSDOF_SUPPORTED_MODULE[] = "stereo-vsdof-supported-module";
const char MtkCameraParameters::KEY_STEREO_TOUCH_POSITION[] = "stereo-touch-position";
const char MtkCameraParameters::KEY_STEREO_LDC_SIZE[] = "stereo-ldc-size";
const char MtkCameraParameters::KEY_STEREO_N3D_SIZE[] = "stereo-n3d-size";
const char MtkCameraParameters::KEY_STEREO_EXTRA_SIZE[] = "stereo-extra-size";
const char MtkCameraParameters::KEY_STEREO_DEPTH_SIZE[] = "stereo-depth-size";
const char MtkCameraParameters::KEY_VSDOF_SCENARIO[] = "vsdof-scenario";    //preview,record,capture
const char MtkCameraParameters::KEY_DUALCAM_CALLBACK_BUFFERS[] = "dualcam-callback-buffers";
const char MtkCameraParameters::PREVIEW[] = "preview";
const char MtkCameraParameters::RECORD[]  = "record";
const char MtkCameraParameters::CAPTURE[] = "capture";
const char MtkCameraParameters::KEY_VSDOF_SCENE_MODE[] = "vsdof-scene-mode";

// Stereo Feature: De-noise
const char MtkCameraParameters::KEY_STEREO_DENOISE_MODE[] = "stereo-denoise-mode";
const char MtkCameraParameters::KEY_STEREO_DENOISE_SUPPORTED_MODULE[] = "stereo-denoise-supported-module";

// Stereo Feature: 3rd-Party
const char MtkCameraParameters::KEY_STEREO_3RDPARTY_MODE[] = "stereo-3rdparty-mode";
const char MtkCameraParameters::KEY_STEREO_3RDPARTY_SUPPORTED_MODULE[] = "stereo-3rdparty-supported-module";

// Stereo Feature: DualCam P2 control
const char MtkCameraParameters::KEY_STEREO_PREVIEW_ENABLE[] = "stereo-preview-enable";

// Dual cam feature: capture
const char MtkCameraParameters::KEY_DUALCAM_CAPTURE_MODE[] = "dualcam-capture-mode";

// Image refocus
const char MtkCameraParameters::KEY_REFOCUS_JPS_FILE_NAME[] = "refocus-jps-file-name";

// 3DNR
const char MtkCameraParameters::KEY_3DNR_MODE[] = "3dnr-mode";
const char MtkCameraParameters::KEY_3DNR_QUALITY_SUPPORTED[] = "3dnr-quality-supported";

const char MtkCameraParameters::KEY_EIS_SUPPORTED_FRAMES[] = "eis-supported-frames";
const char MtkCameraParameters::KEY_EIS25_MODE[] = "eis25-mode";
const char MtkCameraParameters::KEY_ADV_EIS[] = "advanced-eis";

// Flash Calibration
const char MtkCameraParameters::KEY_ENG_FLASH_CALIBRATION[] = "flash-cali";

//KEY for [Engineer Mode] GIS CALIBRATION
const char MtkCameraParameters::KEY_ENG_GIS_CALIBRATION_SUPPORTED[] = "eng-gis-cali-s";

// manual shutter speed/gain
const char MtkCameraParameters::KEY_ENG_MANUAL_SHUTTER_SPEED[] = "m-ss";
const char MtkCameraParameters:: KEY_ENG_MANUAL_SENSOR_GAIN[] = "m-sr-g";

// sensor mode
const char MtkCameraParameters::KEY_ENG_SENOSR_MODE_SUPPORTED[] = "sen-mode-s";

// Gesture Shot
const char MtkCameraParameters::KEY_GESTURE_SHOT[] = "gesture-shot";
const char MtkCameraParameters::KEY_GESTURE_SHOT_SUPPORTED[] = "gesture-shot-supported";

// Native PIP
const char MtkCameraParameters::KEY_NATIVE_PIP[] = "native-pip";
const char MtkCameraParameters::KEY_NATIVE_PIP_SUPPORTED[] = "native-pip-supported";

// PDAF
const char MtkCameraParameters::KEY_PDAF[] = "pdaf";
const char MtkCameraParameters::KEY_PDAF_SUPPORTED[] = "pdaf-supported";

// DNG
const char MtkCameraParameters::KEY_DNG_SUPPORTED[] = "dng-supported";

// Display Rotation
const char MtkCameraParameters::KEY_DISPLAY_ROTATION_SUPPORTED[] = "disp-rot-supported";
const char MtkCameraParameters::KEY_PANEL_SIZE[] = "panel-size";

// Multi-zone AF window
const char MtkCameraParameters::KEY_IS_SUPPORT_MZAF[] = "is-mzaf-supported";
const char MtkCameraParameters::KEY_MZAF_ENABLE[] = "mzaf-enable";

// post-view callback, describes post-view YUV format, the default should be NV21
const char MtkCameraParameters::KEY_POST_VIEW_FMT[] = "post-view-fmt";

// dual zoom
const char MtkCameraParameters::KEY_SUPCAM_ZOOM_RATIO[] = "main2-zoom-ratio";
const char MtkCameraParameters::KEY_DUALCAM_DEVICE_STATE[] = "dualcam1device-state";
const int  MtkCameraParameters::KEY_DUALCAM_DEVICE_STATE_IDLE = 0;
const int  MtkCameraParameters::KEY_DUALCAM_DEVICE_STATE_PREVIEWING = 1;
const int  MtkCameraParameters::KEY_DUALCAM_DEVICE_STATE_RECORDING = 2;
const char MtkCameraParameters::KEY_DUALCAM_SWITCH_ZOOM_RATIO[] = "dualcam-switch-zoom-ratio";

// Background Service (MMSDK)
const char MtkCameraParameters::KEY_MTK_BACKGROUND_SERVICE[] = "bgservice";
const int  MtkCameraParameters::VAL_MTK_BACKGROUND_SERVICE_DISABLE = 0;
const int  MtkCameraParameters::VAL_MTK_BACKGROUND_SERVICE_ENABLE = 1;
}; // namespace android

