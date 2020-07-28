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
package com.android.camera.v2.util;


//TODO: need to be changed as xml
public class SettingKeys {

    public static final int UNKNOWN = -1;
    public static final String DEFAULT_CONINUOUS_CAPTURE_NUM = "20";
    public static final String KEY_VERSION                          = "pref_version_key";
    public static final String KEY_LOCAL_VERSION                    = "pref_local_version_key";

    public static final int SETTING_COUNT = 41;
    // setting key
    public static final String KEY_CAMERA_ID                 = "pref_camera_id_key";
    public static final String KEY_RECORD_LOCATION           = "pref_camera_recordlocation_key";
    public static final String KEY_VIDEO_QUALITY             = "pref_video_quality_key";
    public static final String KEY_SLOW_MOTION_VIDEO_QUALITY = "pref_slow_motion_video_quality_key";
    public static final String KEY_PICTURE_SIZE              = "pref_camera_picturesize_key";
    public static final String KEY_FLASH                     = "pref_camera_flashmode_key";
    public static final String KEY_WHITE_BALANCE             = "pref_camera_whitebalance_key";
    public static final String KEY_SCENE_MODE                = "pref_camera_scenemode_key";
    public static final String KEY_EXPOSURE                  = "pref_camera_exposure_key";
    public static final String KEY_ISO                       = "pref_camera_iso_key";
    public static final String KEY_COLOR_EFFECT              = "pref_camera_coloreffect_key";
    public static final String KEY_CAMERA_ZSD                = "pref_camera_zsd_key";
    public static final String KEY_STEREO3D_PICTURE_SIZE     =
            "pref_camera_picturesize_stereo3d_key";
    public static final String KEY_STEREO3D_MODE             = "pref_stereo3d_mode_key";
    public static final String KEY_STEREO3D_PICTURE_FORMAT   = "pref_camera_pictureformat_key";
    public static final String KEY_VIDEO_RECORD_AUDIO        = "pref_camera_recordaudio_key";
    public static final String KEY_VIDEO_HD_AUDIO_RECORDING  = "pref_camera_video_hd_recording_key";
    public static final String KEY_CAMERA_AIS                = "perf_camera_ais_key";
    public static final String KEY_IMAGE_PROPERTIES          =
            "pref_camera_image_properties_key"; //virtual item
    public static final String KEY_EDGE                      = "pref_camera_edge_key";
    public static final String KEY_HUE                       = "pref_camera_hue_key";
    public static final String KEY_SATURATION                = "pref_camera_saturation_key";
    public static final String KEY_BRIGHTNESS                = "pref_camera_brightness_key";
    public static final String KEY_CONTRAST                  = "pref_camera_contrast_key";
    public static final String KEY_SELF_TIMER                = "pref_camera_self_timer_key";
    public static final String KEY_ANTI_BANDING              = "pref_camera_antibanding_key";
    public static final String KEY_VIDEO_EIS                 = "pref_video_eis_key";
    public static final String KEY_VIDEO_3DNR                = "pref_video_3dnr_key";
    public static final String KEY_CONTINUOUS_NUMBER         = "pref_camera_shot_number";
    public static final String KEY_DUAL_CAMERA_MODE          =
                                                         "pref_dual_camera_key"; //virtual item
    public static final String KEY_FAST_AF                   = "pref_fast_af_key";
    public static final String KEY_DISTANCE                  = "pref_distance_key";
    public static final String KEY_PICTURE_RATIO             = "pref_camera_picturesize_ratio_key";
    public static final String KEY_VOICE                     = "pref_voice_key";
    public static final String KEY_SLOW_MOTION               = "pref_slow_motion_key";
    public static final String KEY_CAMERA_FACE_DETECT        = "pref_face_detect_key";
    public static final String KEY_PANORAMA                  = "pref_panorama_key";
    public static final String KEY_HDR                       = "pref_hdr_key";
    public static final String KEY_ASD                       = "pref_asd_key";
    public static final String KEY_PHOTO_PIP                 = "pref_photo_pip_key";
    public static final String KEY_VIDEO_PIP                 = "pref_video_pip_key";
    public static final String KEY_VIDEO                     = "video_key";
    public static final String KEY_REFOCUS                   = "refocus_key";
    public static final String KEY_NORMAL                    = "normal_key";
    public static final String KEY_DNG                       = "pref_dng_key";

    // setting index
    public static final int ROW_SETTING_PANORAMA                  = 0;
    public static final int ROW_SETTING_PHOTO_PIP                 = 1;
    public static final int ROW_SETTING_VIDEO_PIP                 = 2;
    public static final int ROW_SETTING_REFOCUS                   = 3;
    public static final int ROW_SETTING_SLOW_MOTION               = 4; //video
    public static final int ROW_SETTING_HDR                       = 5; //common
    public static final int ROW_SETTING_ASD                       = 6; //common
    public static final int ROW_SETTING_DUAL_CAMERA               = 7; //common
    public static final int ROW_SETTING_EXPOSURE                  = 8; //common
    public static final int ROW_SETTING_SCENCE_MODE               = 9; //common
    public static final int ROW_SETTING_WHITE_BALANCE             = 10; //common
    public static final int ROW_SETTING_COLOR_EFFECT              = 11; //common
    public static final int ROW_SETTING_SELF_TIMER                = 12; //camera
    public static final int ROW_SETTING_CONTINUOUS_NUM            = 13;
    public static final int ROW_SETTING_RECORD_LOCATION           = 14; //common
    public static final int ROW_SETTING_VIDEO_QUALITY             = 15; //camera
    public static final int ROW_SETTING_ISO                       = 16; //camera
    public static final int ROW_SETTING_ANTI_FLICKER              = 17; //common
    public static final int ROW_SETTING_VIDEO_STABLE              = 18; //video
    public static final int ROW_SETTING_MICROPHONE                = 19; //video
    public static final int ROW_SETTING_AUDIO_MODE                = 20; //video
    public static final int ROW_SETTING_PICTURE_RATIO             = 21; //video
    public static final int ROW_SETTING_PICTURE_SIZE              = 22; //camera
    public static final int ROW_SETTING_ZSD                       = 23; //camera
    public static final int ROW_SETTING_VOICE                     = 24; //camera
    public static final int ROW_SETTING_3DNR                      = 25;
    public static final int ROW_SETTING_SLOW_MOTION_VIDEO_QUALITY = 26; //video
    public static final int ROW_SETTING_AIS                       = 27; //camera
    public static final int ROW_SETTING_SHARPNESS                 = 28; //common
    public static final int ROW_SETTING_HUE                       = 29; //common
    public static final int ROW_SETTING_SATURATION                = 30; //common
    public static final int ROW_SETTING_BRIGHTNESS                = 31; //common
    public static final int ROW_SETTING_CONTRAST                  = 32; //common
    public static final int ROW_SETTING_IMAGE_PROPERTIES          = 33; //common
    public static final int ROW_SETTING_FLASH                     = 34; //common
    public static final int ROW_SETTING_STEREO_MODE               = 35;
    public static final int ROW_SETTING_CAMERA_FACE_DETECT        = 36; //camera
    public static final int ROW_SETTING_FAST_AF                   = 37; //common
    public static final int ROW_SETTING_DISTANCE                  = 38; //common
    public static final int ROW_SETTING_DUAL_CAMERA_MODE          = 39; //common
    public static final int ROW_SETTING_DNG                       = 40; //common

    public final static int DECIDE_BY_APP      = 0;
    public final static int DECIDE_BY_HARDWARE = 1;

    public static final int SUPPORTED_MAX_SOLUTION_WIDTH =  1920;

    private static final int[] SETTING_TYPE = new int[SETTING_COUNT];
    static {
        // setting decide by AP
        SETTING_TYPE[ROW_SETTING_SELF_TIMER]               = DECIDE_BY_APP;
        SETTING_TYPE[ROW_SETTING_CONTINUOUS_NUM]           = DECIDE_BY_APP;
        SETTING_TYPE[ROW_SETTING_RECORD_LOCATION]          = DECIDE_BY_APP;
        SETTING_TYPE[ROW_SETTING_MICROPHONE]               = DECIDE_BY_APP;
        SETTING_TYPE[ROW_SETTING_AUDIO_MODE]               = DECIDE_BY_APP;
        SETTING_TYPE[ROW_SETTING_VOICE]                    = DECIDE_BY_APP;
        SETTING_TYPE[ROW_SETTING_IMAGE_PROPERTIES]         = DECIDE_BY_APP;

        // setting decide by Hardware
        SETTING_TYPE[ROW_SETTING_PANORAMA]                  = DECIDE_BY_HARDWARE;
        SETTING_TYPE[ROW_SETTING_PHOTO_PIP]                 = DECIDE_BY_HARDWARE;
        SETTING_TYPE[ROW_SETTING_VIDEO_PIP]                 = DECIDE_BY_HARDWARE;
        SETTING_TYPE[ROW_SETTING_REFOCUS]                   = DECIDE_BY_HARDWARE;
        SETTING_TYPE[ROW_SETTING_SLOW_MOTION]               = DECIDE_BY_HARDWARE;
        SETTING_TYPE[ROW_SETTING_HDR]                       = DECIDE_BY_HARDWARE;
        SETTING_TYPE[ROW_SETTING_ASD]                       = DECIDE_BY_HARDWARE;
        SETTING_TYPE[ROW_SETTING_DUAL_CAMERA_MODE]          = DECIDE_BY_HARDWARE;
        SETTING_TYPE[ROW_SETTING_DUAL_CAMERA]               = DECIDE_BY_HARDWARE;
        SETTING_TYPE[ROW_SETTING_FLASH]                     = DECIDE_BY_HARDWARE;
        SETTING_TYPE[ROW_SETTING_EXPOSURE]                  = DECIDE_BY_HARDWARE;
        SETTING_TYPE[ROW_SETTING_SCENCE_MODE]               = DECIDE_BY_HARDWARE;
        SETTING_TYPE[ROW_SETTING_WHITE_BALANCE]             = DECIDE_BY_HARDWARE;
        SETTING_TYPE[ROW_SETTING_COLOR_EFFECT]              = DECIDE_BY_HARDWARE;
        SETTING_TYPE[ROW_SETTING_VIDEO_QUALITY]             = DECIDE_BY_HARDWARE;
        SETTING_TYPE[ROW_SETTING_ISO]                       = DECIDE_BY_HARDWARE;
        SETTING_TYPE[ROW_SETTING_ANTI_FLICKER]              = DECIDE_BY_HARDWARE;
        SETTING_TYPE[ROW_SETTING_VIDEO_STABLE]              = DECIDE_BY_HARDWARE;
        SETTING_TYPE[ROW_SETTING_PICTURE_RATIO]             = DECIDE_BY_HARDWARE;
        SETTING_TYPE[ROW_SETTING_PICTURE_SIZE]              = DECIDE_BY_HARDWARE;
        SETTING_TYPE[ROW_SETTING_ZSD]                       = DECIDE_BY_HARDWARE;
        SETTING_TYPE[ROW_SETTING_3DNR]                      = DECIDE_BY_HARDWARE;
        SETTING_TYPE[ROW_SETTING_SLOW_MOTION_VIDEO_QUALITY] = DECIDE_BY_HARDWARE;
        SETTING_TYPE[ROW_SETTING_AIS]                       = DECIDE_BY_HARDWARE;
        SETTING_TYPE[ROW_SETTING_SHARPNESS]                 = DECIDE_BY_HARDWARE;
        SETTING_TYPE[ROW_SETTING_HUE]                       = DECIDE_BY_HARDWARE;
        SETTING_TYPE[ROW_SETTING_SATURATION]                = DECIDE_BY_HARDWARE;
        SETTING_TYPE[ROW_SETTING_BRIGHTNESS]                = DECIDE_BY_HARDWARE;
        SETTING_TYPE[ROW_SETTING_CONTRAST]                  = DECIDE_BY_HARDWARE;
        SETTING_TYPE[ROW_SETTING_STEREO_MODE]               = DECIDE_BY_HARDWARE;
        SETTING_TYPE[ROW_SETTING_FAST_AF]                   = DECIDE_BY_HARDWARE;
        SETTING_TYPE[ROW_SETTING_DISTANCE]                  = DECIDE_BY_HARDWARE;
        SETTING_TYPE[ROW_SETTING_DNG]                       = DECIDE_BY_HARDWARE;
    }

    // setting key and index
    public static final String[] KEYS_FOR_SETTING = new String[SETTING_COUNT];

    static {
        KEYS_FOR_SETTING[ROW_SETTING_FLASH]                 = KEY_FLASH;
        KEYS_FOR_SETTING[ROW_SETTING_DUAL_CAMERA]           = KEY_CAMERA_ID; //need recheck
        KEYS_FOR_SETTING[ROW_SETTING_EXPOSURE]              = KEY_EXPOSURE;
        KEYS_FOR_SETTING[ROW_SETTING_SCENCE_MODE]           = KEY_SCENE_MODE;
        KEYS_FOR_SETTING[ROW_SETTING_WHITE_BALANCE]         = KEY_WHITE_BALANCE;
        KEYS_FOR_SETTING[ROW_SETTING_IMAGE_PROPERTIES]      = null; //KEY_IMAGE_PROPERTIES;
        KEYS_FOR_SETTING[ROW_SETTING_COLOR_EFFECT]          = null; //KEY_COLOR_EFFECT;
        KEYS_FOR_SETTING[ROW_SETTING_SELF_TIMER]            = KEY_SELF_TIMER;
        KEYS_FOR_SETTING[ROW_SETTING_ZSD]                   = KEY_CAMERA_ZSD;
        KEYS_FOR_SETTING[ROW_SETTING_RECORD_LOCATION]       = KEY_RECORD_LOCATION; //need recheck
        KEYS_FOR_SETTING[ROW_SETTING_PICTURE_SIZE]          = KEY_PICTURE_SIZE;
        KEYS_FOR_SETTING[ROW_SETTING_ISO]                   = null; //KEY_ISO;
        KEYS_FOR_SETTING[ROW_SETTING_ANTI_FLICKER]          = KEY_ANTI_BANDING;
        KEYS_FOR_SETTING[ROW_SETTING_VIDEO_STABLE]          = KEY_VIDEO_EIS;
        KEYS_FOR_SETTING[ROW_SETTING_MICROPHONE]            = KEY_VIDEO_RECORD_AUDIO;
        KEYS_FOR_SETTING[ROW_SETTING_AUDIO_MODE]            = KEY_VIDEO_HD_AUDIO_RECORDING;
        KEYS_FOR_SETTING[ROW_SETTING_VIDEO_QUALITY]         = KEY_VIDEO_QUALITY;
        KEYS_FOR_SETTING[ROW_SETTING_PICTURE_RATIO]         = KEY_PICTURE_RATIO;
        KEYS_FOR_SETTING[ROW_SETTING_VOICE]                 = null; //KEY_VOICE;
        KEYS_FOR_SETTING[ROW_SETTING_3DNR]                  = KEY_VIDEO_3DNR;
        KEYS_FOR_SETTING[ROW_SETTING_SLOW_MOTION]           = KEY_SLOW_MOTION;
        KEYS_FOR_SETTING[ROW_SETTING_SLOW_MOTION_VIDEO_QUALITY]     = KEY_SLOW_MOTION_VIDEO_QUALITY;
        KEYS_FOR_SETTING[ROW_SETTING_AIS]                   = KEY_CAMERA_AIS;
        KEYS_FOR_SETTING[ROW_SETTING_SHARPNESS]             = KEY_EDGE;
        KEYS_FOR_SETTING[ROW_SETTING_HUE]                   = KEY_HUE;
        KEYS_FOR_SETTING[ROW_SETTING_SATURATION]            = KEY_SATURATION;
        KEYS_FOR_SETTING[ROW_SETTING_BRIGHTNESS]            = KEY_BRIGHTNESS;
        KEYS_FOR_SETTING[ROW_SETTING_CONTRAST]              = KEY_CONTRAST;
        KEYS_FOR_SETTING[ROW_SETTING_CONTINUOUS_NUM]        = null; //KEY_CONTINUOUS_NUMBER;
        KEYS_FOR_SETTING[ROW_SETTING_STEREO_MODE]           = KEY_STEREO3D_MODE;
        KEYS_FOR_SETTING[ROW_SETTING_CAMERA_FACE_DETECT]    = KEY_CAMERA_FACE_DETECT;
        KEYS_FOR_SETTING[ROW_SETTING_HDR]                   = KEY_HDR;
        KEYS_FOR_SETTING[ROW_SETTING_ASD]                   = KEY_ASD;
        KEYS_FOR_SETTING[ROW_SETTING_DUAL_CAMERA_MODE]      = KEY_DUAL_CAMERA_MODE;
        KEYS_FOR_SETTING[ROW_SETTING_FAST_AF]               = KEY_FAST_AF;
        KEYS_FOR_SETTING[ROW_SETTING_DISTANCE]              = KEY_DISTANCE;
        KEYS_FOR_SETTING[ROW_SETTING_PANORAMA]              = KEY_PANORAMA;
        KEYS_FOR_SETTING[ROW_SETTING_PHOTO_PIP]             = KEY_PHOTO_PIP;
        KEYS_FOR_SETTING[ROW_SETTING_VIDEO_PIP]             = KEY_VIDEO_PIP;
        KEYS_FOR_SETTING[ROW_SETTING_REFOCUS]               = KEY_REFOCUS;
        KEYS_FOR_SETTING[ROW_SETTING_DNG]                   = KEY_DNG;
    };

    public static final int[] UN_SUPPORT_BY_3RDPARTY = new int[] {
        ROW_SETTING_ZSD,
        ROW_SETTING_VOICE,
        ROW_SETTING_3DNR,
        ROW_SETTING_CONTINUOUS_NUM,
        ROW_SETTING_AIS,
        ROW_SETTING_CAMERA_FACE_DETECT,
        ROW_SETTING_HDR,
        ROW_SETTING_ASD,
        ROW_SETTING_SLOW_MOTION,
        ROW_SETTING_IMAGE_PROPERTIES,
        ROW_SETTING_FAST_AF,
        ROW_SETTING_DISTANCE,
        ROW_SETTING_DNG,
        ROW_SETTING_PHOTO_PIP,
        ROW_SETTING_PANORAMA,
        ROW_SETTING_DUAL_CAMERA_MODE,
    };

    public static final int[] SUPPORT_BY_3RDPARTY_BUT_HIDDEN = new int[] {
        ROW_SETTING_SCENCE_MODE,
        ROW_SETTING_WHITE_BALANCE,
        ROW_SETTING_ISO,
        ROW_SETTING_ANTI_FLICKER,
        ROW_SETTING_COLOR_EFFECT,
        ROW_SETTING_RECORD_LOCATION,
        ROW_SETTING_VIDEO_QUALITY,
    };

    public static final int[] RESET_SETTING_ITEMS = new int[] {
        ROW_SETTING_EXPOSURE,
        ROW_SETTING_SCENCE_MODE,
        ROW_SETTING_WHITE_BALANCE,
        ROW_SETTING_COLOR_EFFECT,
        ROW_SETTING_SELF_TIMER,
        ROW_SETTING_SHARPNESS, // common
        ROW_SETTING_HUE, // common
        ROW_SETTING_SATURATION, // common
        ROW_SETTING_BRIGHTNESS, // common
        ROW_SETTING_CONTRAST, // common
        ROW_SETTING_ISO,
        ROW_SETTING_HDR,
        ROW_SETTING_ASD,
        ROW_SETTING_SLOW_MOTION,
    };

    public static final int[] THIRDPART_RESET_SETTING_ITEMS = new int[] {
        ROW_SETTING_EXPOSURE,
        ROW_SETTING_SELF_TIMER,
   };

    public static final String[] MODE_KEYS = {
        KEY_PANORAMA,
        KEY_PHOTO_PIP,
        KEY_VIDEO_PIP,
    };

    public static final int[] SETTING_GROUP_COMMON_FOR_TAB = new int[]{
        ROW_SETTING_DUAL_CAMERA_MODE,
        ROW_SETTING_RECORD_LOCATION, //common
        ROW_SETTING_EXPOSURE, //common
        ROW_SETTING_COLOR_EFFECT, //common
        ROW_SETTING_SCENCE_MODE, //common
        ROW_SETTING_WHITE_BALANCE, //common
        ROW_SETTING_IMAGE_PROPERTIES,
        ROW_SETTING_ANTI_FLICKER, //common
    };

    public static final int[] SETTING_GROUP_MAIN_COMMON_FOR_TAB = new int[]{
        ROW_SETTING_RECORD_LOCATION, //common
        ROW_SETTING_IMAGE_PROPERTIES,
        ROW_SETTING_ANTI_FLICKER, //common
    };

    public static final int[] SETTING_GROUP_COMMON_FOR_LOMOEFFECT = new int[]{
        ROW_SETTING_DUAL_CAMERA_MODE,
        ROW_SETTING_RECORD_LOCATION, //common
        ROW_SETTING_EXPOSURE, //common
        ROW_SETTING_SCENCE_MODE, //common
        ROW_SETTING_WHITE_BALANCE, //common
        ROW_SETTING_IMAGE_PROPERTIES,
        ROW_SETTING_ANTI_FLICKER, //common
    };

    public static final int[] SETTING_GROUP_CAMERA_FOR_TAB = new int[]{
        ROW_SETTING_ZSD, //camera
        ROW_SETTING_AIS, //camera
        ROW_SETTING_VOICE, //camera
        ROW_SETTING_CAMERA_FACE_DETECT, //camera
        ROW_SETTING_ASD, //camera
        ROW_SETTING_DNG,
        ROW_SETTING_SELF_TIMER, //camera
        ROW_SETTING_CONTINUOUS_NUM, //camera
        ROW_SETTING_PICTURE_SIZE, //camera
        ROW_SETTING_PICTURE_RATIO, //camera
        ROW_SETTING_ISO, //camera
    };

    public static final int[] SETTING_GROUP_VIDEO_FOR_TAB = new int[]{
        ROW_SETTING_3DNR,
        ROW_SETTING_VIDEO_STABLE, //video
        ROW_SETTING_MICROPHONE, //video
        ROW_SETTING_VIDEO_QUALITY, //video
        ROW_SETTING_SLOW_MOTION_VIDEO_QUALITY,
    };

    public static final int[] UN_SUPPORT_BY_FRONT_CAMERA = new int[] {
        ROW_SETTING_SLOW_MOTION,
        ROW_SETTING_CONTINUOUS_NUM,
        ROW_SETTING_SLOW_MOTION_VIDEO_QUALITY,
        ROW_SETTING_STEREO_MODE,
    };

    // For Tablet feature.
    public static final int[] SETTING_GROUP_SUB_COMMON = new int[] {
        ROW_SETTING_EXPOSURE, // common
        ROW_SETTING_COLOR_EFFECT, // common
        ROW_SETTING_WHITE_BALANCE, // common
        ROW_SETTING_SCENCE_MODE, // common
    };

    public static String getSettingKey(int settingId) {
        return KEYS_FOR_SETTING[settingId];
    }

    public static int getSettingId(String key) {
        int settingIndex = -1;
        for (int i = 0; i < KEYS_FOR_SETTING.length; i++) {
            if (KEYS_FOR_SETTING[i] != null
                    && KEYS_FOR_SETTING[i].equals(key)) {
                settingIndex = i;
                break;
            }
        }
        return settingIndex;
    }

    public static int getSettingType(String key) {
        int settingId = getSettingId(key);
        return SETTING_TYPE[settingId];
    }
}
