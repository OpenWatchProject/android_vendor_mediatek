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
 * MediaTek Inc. (C) 2014. All rights reserved.
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
package com.mediatek.camera.platform;

import android.hardware.Camera.Area;
import android.hardware.Camera.Size;

import java.util.List;

public interface Parameters {

    public static final String WHITE_BALANCE_TUNGSTEN = "tungsten";

    // Values for white balance settings.
    public static final String WHITE_BALANCE_AUTO = "auto";
    public static final String WHITE_BALANCE_INCANDESCENT = "incandescent";
    public static final String WHITE_BALANCE_FLUORESCENT = "fluorescent";
    public static final String WHITE_BALANCE_WARM_FLUORESCENT = "warm-fluorescent";
    public static final String WHITE_BALANCE_DAYLIGHT = "daylight";
    public static final String WHITE_BALANCE_CLOUDY_DAYLIGHT = "cloudy-daylight";
    public static final String WHITE_BALANCE_TWILIGHT = "twilight";
    public static final String WHITE_BALANCE_SHADE = "shade";

    // Values for color effect settings.
    public static final String EFFECT_NONE = "none";
    public static final String EFFECT_MONO = "mono";
    public static final String EFFECT_NEGATIVE = "negative";
    public static final String EFFECT_SOLARIZE = "solarize";
    public static final String EFFECT_SEPIA = "sepia";
    public static final String EFFECT_POSTERIZE = "posterize";
    public static final String EFFECT_WHITEBOARD = "whiteboard";
    public static final String EFFECT_BLACKBOARD = "blackboard";
    public static final String EFFECT_AQUA = "aqua";

    // Values for antibanding settings.
    public static final String ANTIBANDING_AUTO = "auto";
    public static final String ANTIBANDING_50HZ = "50hz";
    public static final String ANTIBANDING_60HZ = "60hz";
    public static final String ANTIBANDING_OFF = "off";

    public static final String KEY_STEREO3D_TYPE = "type";
    public static final String KEY_STEREO3D_MODE = "mode";
    public static final String STEREO3D_TYPE_OFF = "off";
    public static final String STEREO3D_TYPE_FRAMESEQ = "frame_seq";
    public static final String STEREO3D_TYPE_SIDEBYSIDE = "sidebyside";
    public static final String STEREO3D_TYPE_TOPBOTTOM = "topbottom";
    public static final String EIS_MODE_ON = "on";
    public static final String EIS_MODE_OFF = "off";
    public static final String FLASH_MODE_OFF = "off";
    public static final String FLASH_MODE_AUTO = "auto";
    public static final String FLASH_MODE_ON = "on";
    public static final String FLASH_MODE_RED_EYE = "red-eye";
    public static final String FLASH_MODE_TORCH = "torch";
    public static final String SCENE_MODE_AUTO = "auto";
    public static final String SCENE_MODE_ACTION = "action";
    public static final String SCENE_MODE_PORTRAIT = "portrait";
    public static final String SCENE_MODE_LANDSCAPE = "landscape";
    public static final String SCENE_MODE_NIGHT = "night";
    public static final String SCENE_MODE_NIGHT_PORTRAIT = "night-portrait";
    public static final String SCENE_MODE_THEATRE = "theatre";
    public static final String SCENE_MODE_BEACH = "beach";
    public static final String SCENE_MODE_SNOW = "snow";
    public static final String SCENE_MODE_SUNSET = "sunset";
    public static final String SCENE_MODE_STEADYPHOTO = "steadyphoto";
    public static final String SCENE_MODE_FIREWORKS = "fireworks";
    public static final String SCENE_MODE_SPORTS = "sports";
    public static final String SCENE_MODE_PARTY = "party";
    public static final String SCENE_MODE_CANDLELIGHT = "candlelight";
    public static final String SCENE_MODE_BARCODE = "barcode";
    public static final String SCENE_MODE_HDR = "hdr";
    public static final String FOCUS_MODE_AUTO = "auto";
    public static final String FOCUS_MODE_INFINITY = "infinity";
    public static final String FOCUS_MODE_MACRO = "macro";
    public static final String FOCUS_MODE_FIXED = "fixed";
    public static final String FOCUS_MODE_EDOF = "edof";
    public static final int CAMERA_MODE_NORMAL = 0;
    public static final int CAMERA_MODE_MTK_PRV = 1;
    public static final int CAMERA_MODE_MTK_VDO = 2;
    public static final int CAMERA_MODE_MTK_VT = 3;
    public static final int FOCUS_ENG_MODE_NONE = 0;
    public static final int FOCUS_ENG_MODE_BRACKET = 1;
    public static final int FOCUS_ENG_MODE_FULLSCAN = 2;
    public static final int FOCUS_ENG_MODE_FULLSCAN_REPEAT = 3;
    public static final int FOCUS_ENG_MODE_REPEAT = 4;
    public static final String FOCUS_MODE_MANUAL = "manual";
    public static final String FOCUS_MODE_FULLSCAN = "fullscan";
    public static final int PREVIEW_DUMP_RESOLUTION_NORMAL = 0;
    public static final int PREVIEW_DUMP_RESOLUTION_CROP = 1;
    public static final String CAPTURE_MODE_NORMAL = "normal";
    public static final String CAPTURE_MODE_BEST_SHOT = "bestshot";
    public static final String CAPTURE_MODE_EV_BRACKET_SHOT = "evbracketshot";
    public static final String CAPTURE_MODE_BURST_SHOT = "burstshot";
    public static final String CAPTURE_MODE_PANORAMA_SHOT = "autorama";
    public static final String CAPTURE_MODE_HDR = "hdr";
    public static final String CAPTURE_MODE_ASD = "asd";
    public static final String CAPTURE_MODE_FB = "face_beauty";
    public static final String KEY_MAX_NUM_DETECTED_OBJECT = "max-num-ot";
    public static final String CAPTURE_MODE_S3D = "single3d";
    public static final String CAPTURE_MODE_PANORAMA3D = "panorama3dmode";
    public static final String CAPTURE_MODE_CONTINUOUS_SHOT = "continuousshot";
    public static final String SENSOR_DEV_MAIN = "main";
    public static final String SENSOR_DEV_SUB = "sub";
    public static final String SENSOR_DEV_ATV = "atv";
    public static final String FOCUS_MODE_CONTINUOUS_VIDEO = "continuous-video";
    public static final String FOCUS_MODE_CONTINUOUS_PICTURE = "continuous-picture";
    public static final int FOCUS_DISTANCE_NEAR_INDEX = 0;
    public static final int FOCUS_DISTANCE_OPTIMAL_INDEX = 1;
    public static final int FOCUS_DISTANCE_FAR_INDEX = 2;
    public static final int PREVIEW_FPS_MIN_INDEX = 0;
    public static final int PREVIEW_FPS_MAX_INDEX = 1;

    public static final String TRUE = "true";
    public static final String FALSE = "false";
    public static final String ON = "on";
    public static final String OFF = "off";
    // Parameter key suffix for supported values.
    public static final String SUPPORTED_VALUES_SUFFIX = "-values";

    public static final String KEY_CAMERA_MODE = "mtk-cam-mode";
    public static final String KEY_ISOSPEED_MODE = "iso-speed";
    public static final String KEY_EDGE_MODE = "edge";
    public static final String KEY_HUE_MODE = "hue";
    public static final String KEY_SATURATION_MODE = "saturation";
    public static final String KEY_BRIGHTNESS_MODE = "brightness";
    public static final String KEY_CONTRAST_MODE = "contrast";
    public static final String KEY_CAPTURE_MODE = "cap-mode";
    public static final String KEY_CAPTURE_PATH = "capfname";
    public static final String KEY_BURST_SHOT_NUM = "burst-num";
    public static final String KEY_ZSD_MODE = "zsd-mode";
    public static final String KEY_MUTE_RECORDING_SOUND = "rec-mute-ogg";
    public static final String KEY_MAX_FRAME_RATE_ZSD_ON = "pip-fps-zsd-on";
    public static final String KEY_MAX_FRAME_RATE_ZSD_OFF = "pip-fps-zsd-off";
    public static final String KEY_DYNAMIC_FRAME_RATE = "dynamic-frame-rate";
    public static final String KEY_DYNAMIC_FRAME_RATE_SUPPORTED = "dynamic-frame-rate-supported";
    public static final String KEY_REFOCUS_JPS_FILE_NAME = "refocus-jps-file-name";
    public static final String KEY_STEREO_DEPTHAF_MODE = "stereo-depth-af";
    public static final String KEY_STEREO_DISTANCE_MODE = "stereo-distance-measurement";
    public static final String KEY_STEREO_REFOCUS_MODE = "stereo-image-refocus";

    public void set(String key, String value);

    public void set(String key, int value);

    public String get(String key);

    public int getInt(String key);

    public void setPreviewSize(int width, int height);

    public Size getPreviewSize();

    public List<Size> getSupportedPreviewSizes();

    public List<Size> getSupportedVideoSizes();

    public Size getPreferredPreviewSizeForVideo();

    public void setJpegQuality(int quality);

    public int getJpegQuality();

    public void setPreviewFrameRate(int fps);

    public List<Integer> getSupportedPreviewFrameRates();

    public void setPreviewFormat(int pixel_format);

    public int getPreviewFormat();

    public void setPictureSize(int width, int height);

    public Size getPictureSize();

    public List<Size> getSupportedPictureSizes();

    public void setRotation(int rotation);

    public String getWhiteBalance();

    public void setWhiteBalance(String value);

    public List<String> getSupportedWhiteBalance();

    public String getColorEffect();

    public void setColorEffect(String value);

    public List<String> getSupportedColorEffects();

    public String getAntibanding();

    public void setAntibanding(String antibanding);

    public List<String> getSupportedAntibanding();

    public String getSceneMode();

    public void setSceneMode(String value);

    public List<String> getSupportedSceneModes();

    public String getFlashMode();

    public void setFlashMode(String value);

    public List<String> getSupportedFlashModes();

    public String getFocusMode();

    public void setFocusMode(String value);

    public List<String> getSupportedFocusModes();

    public int getExposureCompensation();

    public void setExposureCompensation(int value);

    public int getMaxExposureCompensation();

    public int getMinExposureCompensation();

    public float getExposureCompensationStep();

    public void setAutoExposureLock(boolean toggle);

    public void setAutoWhiteBalanceLock(boolean toggle);

    public int getZoom();

    public boolean isZoomSupported();

    public void setCameraMode(int value);

    public String getISOSpeed();

    public void setISOSpeed(String value);

    public List<String> getSupportedISOSpeed();

    public int getMaxNumDetectedObjects();

    public String getEdgeMode();

    public void setEdgeMode(String value);

    public List<String> getSupportedEdgeMode();

    public String getHueMode();

    public void setHueMode(String value);

    public List<String> getSupportedHueMode();

    public String getSaturationMode();

    public void setSaturationMode(String value);

    public List<String> getSupportedSaturationMode();

    public String getBrightnessMode();

    public void setBrightnessMode(String value);

    public List<String> getSupportedBrightnessMode();

    public String getContrastMode();

    public void setContrastMode(String value);

    public List<String> getSupportedContrastMode();

    public String getCaptureMode();

    public void setCaptureMode(String value);

    public List<String> getSupportedCaptureMode();

    public void setCapturePath(String value);

    public void setBurstShotNum(int value);

    public String getZSDMode();

    public void setZSDMode(String value);

    public List<String> getSupportedZSDMode();

    public void setFocusAreas(List<Area> focusAreas);

    public int getMaxNumFocusAreas();

    public void setMeteringAreas(List<Area> meteringAreas);

    public void setRecordingHint(boolean hint);

    public boolean isVideoSnapshotSupported();

    public void enableRecordingSound(String value);

    public void setVideoStabilization(boolean toggle);

    public boolean isVideoStabilizationSupported();

    public List<Integer> getPIPFrameRateZSDOn();

    public List<Integer> getPIPFrameRateZSDOff();

    public void setDynamicFrameRate(boolean toggle);

    public boolean isDynamicFrameRateSupported();

    public void setRefocusJpsFileName(String fineName);

    public String getDepthAFMode();

    public String getDistanceMode();

    public void setDepthAFMode(boolean isDepthAfMode);

    public void setDistanceMode(boolean isDistanceMode);
}
