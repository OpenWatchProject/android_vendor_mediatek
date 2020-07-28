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
package com.mediatek.camera.v2.util;


//TODO: need to be changed as xml
public class SettingKeys {

    public static final int UNKNOWN = -1;
    public static final String DEFAULT_CONINUOUS_CAPTURE_NUM = "20";
    public static final String KEY_VERSION                          = "pref_version_key";
    public static final String KEY_JPEG_QUALITY                     = "pref_camera_jpegquality_key";
    public static final String KEY_LOCAL_VERSION                    = "pref_local_version_key";

    public static final int SETTING_COUNT = 49;
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
    public static final String KEY_IMAGE_PROPERTIES          = "pref_camera_image_properties_key";
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
    public static final String KEY_CAMERA_MODE               = "camera_mode_key";
    public static final String KEY_CAPTURE_MODE              = "capture_mode_key";
    public static final String KEY_RECORDING_HIHT            = "recoding_hint_key";
    public static final String KEY_MUTE_RECORDING_SOUND      = "mute_recoding_sound_key";
    public static final String KEY_PANORAMA                  = "panorama_key";
    public static final String KEY_HDR                       = "pref_hdr_key";
    public static final String KEY_ASD                       = "pref_asd_key";
    public static final String KEY_PHOTO_PIP                 = "photo_pip_key";
    public static final String KEY_VIDEO_PIP                 = "video_pip_key";
    public static final String KEY_VIDEO                     = "video_key";
    public static final String KEY_REFOCUS                   = "refocus_key";
    public static final String KEY_NORMAL                    = "normal_key";
    public static final String KEY_OBJECT_TRACKING           = "object_tracking_key";
    public static final String KEY_DNG                       = "dng_key";

    // setting index
    public static final int ROW_SETTING_PANORAMA                  = 0;
    public static final int ROW_SETTING_PHOTO_PIP                 = 1;
    public static final int ROW_SETTING_VIDEO_PIP                 = 2;
    public static final int ROW_SETTING_VIDEO                     = 3;
    public static final int ROW_SETTING_REFOCUS                   = 4;
    public static final int ROW_SETTING_NORMAL                    = 5;
    public static final int ROW_SETTING_SLOW_MOTION               = 6; //video
    public static final int ROW_SETTING_HDR                       = 7; //common
    public static final int ROW_SETTING_ASD                       = 8; //common
    public static final int ROW_SETTING_DUAL_CAMERA               = 9; //common
    public static final int ROW_SETTING_EXPOSURE                  = 10; //common
    public static final int ROW_SETTING_SCENCE_MODE               = 11; //common
    public static final int ROW_SETTING_WHITE_BALANCE             = 12; //common
    public static final int ROW_SETTING_COLOR_EFFECT              = 13; //common
    public static final int ROW_SETTING_SELF_TIMER                = 14; //camera
    public static final int ROW_SETTING_CONTINUOUS_NUM            = 15;
    public static final int ROW_SETTING_RECORD_LOCATION           = 16; //common
    public static final int ROW_SETTING_VIDEO_QUALITY             = 17; //camera
    public static final int ROW_SETTING_ISO                       = 18; //camera
    public static final int ROW_SETTING_ANTI_FLICKER              = 19; //common
    public static final int ROW_SETTING_VIDEO_STABLE              = 20; //video
    public static final int ROW_SETTING_MICROPHONE                = 21; //video
    public static final int ROW_SETTING_AUDIO_MODE                = 22; //video
    public static final int ROW_SETTING_PICTURE_RATIO             = 23; //video
    public static final int ROW_SETTING_PICTURE_SIZE              = 24; //camera
    public static final int ROW_SETTING_ZSD                       = 25; //camera
    public static final int ROW_SETTING_VOICE                     = 26; //camera
    public static final int ROW_SETTING_3DNR                      = 27;
    public static final int ROW_SETTING_SLOW_MOTION_VIDEO_QUALITY = 28; //video
    public static final int ROW_SETTING_AIS                       = 29; //camera
    public static final int ROW_SETTING_SHARPNESS                 = 30; //common
    public static final int ROW_SETTING_HUE                       = 31; //common
    public static final int ROW_SETTING_SATURATION                = 32; //common
    public static final int ROW_SETTING_BRIGHTNESS                = 33; //common
    public static final int ROW_SETTING_CONTRAST                  = 34; //common
    public static final int ROW_SETTING_IMAGE_PROPERTIES          = 35; //common
    public static final int ROW_SETTING_CAMERA_MODE               = 36; //camera mode
    public static final int ROW_SETTING_CAPTURE_MODE              = 37; //not in preference
    public static final int ROW_SETTING_RECORDING_HINT            = 38; //not in preference
    public static final int ROW_SETTING_FLASH                     = 39; //common
    public static final int ROW_SETTING_JPEG_QUALITY              = 40; //not in preference
    public static final int ROW_SETTING_STEREO_MODE               = 41;
    public static final int ROW_SETTING_CAMERA_FACE_DETECT        = 42; //camera
    public static final int ROW_SETTING_MUTE_RECORDING_SOUND      = 43;
    public static final int ROW_SETTING_FAST_AF                   = 44; //common
    public static final int ROW_SETTING_DISTANCE                  = 45; //common
    public static final int ROW_SETTING_DUAL_CAMERA_MODE          = 46; //common
    public static final int ROW_SETTING_OBJECT_TRACKING           = 47;
    public static final int ROW_SETTING_DNG                       = 48;

    public final static int APPLY_TO_NATIVE                         = 0;
    public final static int DO_NOT_APPLY_TO_NATIVE                  = 1;


    private static final int[] SETTING_TYPE = new int[SETTING_COUNT];
    static {
        // do not need to apply to native currently.
        SETTING_TYPE[ROW_SETTING_DUAL_CAMERA]               = DO_NOT_APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_IMAGE_PROPERTIES]          = DO_NOT_APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_RECORD_LOCATION]           = DO_NOT_APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_MICROPHONE]                = DO_NOT_APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_AUDIO_MODE]                = DO_NOT_APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_VIDEO_QUALITY]             = DO_NOT_APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_VOICE]                     = DO_NOT_APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_STEREO_MODE]               = DO_NOT_APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_CAMERA_MODE]               = DO_NOT_APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_CAPTURE_MODE]              = DO_NOT_APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_JPEG_QUALITY]              = DO_NOT_APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_MUTE_RECORDING_SOUND]      = DO_NOT_APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_RECORDING_HINT]            = DO_NOT_APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_PANORAMA]                  = DO_NOT_APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_PHOTO_PIP]                 = DO_NOT_APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_VIDEO_PIP]                 = DO_NOT_APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_VIDEO]                     = DO_NOT_APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_REFOCUS]                   = DO_NOT_APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_NORMAL]                    = DO_NOT_APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_OBJECT_TRACKING]           = DO_NOT_APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_SELF_TIMER]                = DO_NOT_APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_CONTINUOUS_NUM]            = DO_NOT_APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_ZSD]                       = DO_NOT_APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_SLOW_MOTION]               = DO_NOT_APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_SLOW_MOTION_VIDEO_QUALITY] = DO_NOT_APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_AIS]                       = DO_NOT_APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_SHARPNESS]                 = DO_NOT_APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_HUE]                       = DO_NOT_APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_SATURATION]                = DO_NOT_APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_BRIGHTNESS]                = DO_NOT_APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_CONTRAST]                  = DO_NOT_APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_FAST_AF]                   = DO_NOT_APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_DISTANCE]                  = DO_NOT_APPLY_TO_NATIVE;

        // need to apply to native currently.
        SETTING_TYPE[ROW_SETTING_CAMERA_FACE_DETECT]        = APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_HDR]                       = APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_ASD]                       = APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_EXPOSURE]                  = APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_SCENCE_MODE]               = APPLY_TO_NATIVE; //APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_WHITE_BALANCE]             = APPLY_TO_NATIVE; //APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_COLOR_EFFECT]              =
                                                       DO_NOT_APPLY_TO_NATIVE; //APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_PICTURE_RATIO]             = APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_PICTURE_SIZE]              = APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_ISO]                       = DO_NOT_APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_ANTI_FLICKER]              = APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_VIDEO_STABLE]              = APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_FLASH]                     = APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_DNG]                       = APPLY_TO_NATIVE;
        SETTING_TYPE[ROW_SETTING_3DNR]                      = APPLY_TO_NATIVE;
    }

    // setting key and index
    public static final String[] KEYS_FOR_SETTING = new String[SETTING_COUNT];

    static {
        KEYS_FOR_SETTING[ROW_SETTING_FLASH]                 = KEY_FLASH;
        KEYS_FOR_SETTING[ROW_SETTING_DUAL_CAMERA]           = KEY_CAMERA_ID; //need recheck
        KEYS_FOR_SETTING[ROW_SETTING_EXPOSURE]              = KEY_EXPOSURE;
        KEYS_FOR_SETTING[ROW_SETTING_SCENCE_MODE]           = KEY_SCENE_MODE;
        KEYS_FOR_SETTING[ROW_SETTING_WHITE_BALANCE]         = KEY_WHITE_BALANCE;
        KEYS_FOR_SETTING[ROW_SETTING_IMAGE_PROPERTIES]      = KEY_IMAGE_PROPERTIES;
        KEYS_FOR_SETTING[ROW_SETTING_COLOR_EFFECT]          = KEY_COLOR_EFFECT;
        KEYS_FOR_SETTING[ROW_SETTING_SELF_TIMER]            = KEY_SELF_TIMER;
        KEYS_FOR_SETTING[ROW_SETTING_ZSD]                   = KEY_CAMERA_ZSD;
        KEYS_FOR_SETTING[ROW_SETTING_RECORD_LOCATION]       = KEY_RECORD_LOCATION; //need recheck
        KEYS_FOR_SETTING[ROW_SETTING_PICTURE_SIZE]          = KEY_PICTURE_SIZE;
        KEYS_FOR_SETTING[ROW_SETTING_ISO]                   = KEY_ISO;
        KEYS_FOR_SETTING[ROW_SETTING_ANTI_FLICKER]          = KEY_ANTI_BANDING;
        KEYS_FOR_SETTING[ROW_SETTING_VIDEO_STABLE]          = KEY_VIDEO_EIS;
        KEYS_FOR_SETTING[ROW_SETTING_MICROPHONE]            = KEY_VIDEO_RECORD_AUDIO;
        KEYS_FOR_SETTING[ROW_SETTING_AUDIO_MODE]            = KEY_VIDEO_HD_AUDIO_RECORDING;
        KEYS_FOR_SETTING[ROW_SETTING_VIDEO_QUALITY]         = KEY_VIDEO_QUALITY;
        KEYS_FOR_SETTING[ROW_SETTING_PICTURE_RATIO]         = KEY_PICTURE_RATIO;
        KEYS_FOR_SETTING[ROW_SETTING_VOICE]                 = KEY_VOICE;
        KEYS_FOR_SETTING[ROW_SETTING_3DNR]                  = KEY_VIDEO_3DNR;
        KEYS_FOR_SETTING[ROW_SETTING_SLOW_MOTION]           = KEY_SLOW_MOTION;
        KEYS_FOR_SETTING[ROW_SETTING_SLOW_MOTION_VIDEO_QUALITY]     = KEY_SLOW_MOTION_VIDEO_QUALITY;

        KEYS_FOR_SETTING[ROW_SETTING_AIS]                   = KEY_CAMERA_AIS;


        KEYS_FOR_SETTING[ROW_SETTING_SHARPNESS]             = KEY_EDGE;
        KEYS_FOR_SETTING[ROW_SETTING_HUE]                   = KEY_HUE;
        KEYS_FOR_SETTING[ROW_SETTING_SATURATION]            = KEY_SATURATION;
        KEYS_FOR_SETTING[ROW_SETTING_BRIGHTNESS]            = KEY_BRIGHTNESS;
        KEYS_FOR_SETTING[ROW_SETTING_CONTRAST]              = KEY_CONTRAST;
        KEYS_FOR_SETTING[ROW_SETTING_CAMERA_MODE]           = KEY_CAMERA_MODE;
        KEYS_FOR_SETTING[ROW_SETTING_CAPTURE_MODE]          = KEY_CAPTURE_MODE;
        KEYS_FOR_SETTING[ROW_SETTING_CONTINUOUS_NUM]        = KEY_CONTINUOUS_NUMBER;
        KEYS_FOR_SETTING[ROW_SETTING_RECORDING_HINT]        = KEY_RECORDING_HIHT;
        KEYS_FOR_SETTING[ROW_SETTING_JPEG_QUALITY]          = KEY_JPEG_QUALITY;
        KEYS_FOR_SETTING[ROW_SETTING_STEREO_MODE]           = KEY_STEREO3D_MODE;
        KEYS_FOR_SETTING[ROW_SETTING_CAMERA_FACE_DETECT]    = KEY_CAMERA_FACE_DETECT;
        KEYS_FOR_SETTING[ROW_SETTING_HDR]                   = KEY_HDR;
        KEYS_FOR_SETTING[ROW_SETTING_ASD]                   = KEY_ASD;
        KEYS_FOR_SETTING[ROW_SETTING_MUTE_RECORDING_SOUND]  = KEY_MUTE_RECORDING_SOUND;
        KEYS_FOR_SETTING[ROW_SETTING_DUAL_CAMERA_MODE]      = KEY_DUAL_CAMERA_MODE;
        KEYS_FOR_SETTING[ROW_SETTING_FAST_AF]               = KEY_FAST_AF;
        KEYS_FOR_SETTING[ROW_SETTING_DISTANCE]              = KEY_DISTANCE;
        KEYS_FOR_SETTING[ROW_SETTING_PANORAMA]              = KEY_PANORAMA;
        KEYS_FOR_SETTING[ROW_SETTING_PHOTO_PIP]             = KEY_PHOTO_PIP;
        KEYS_FOR_SETTING[ROW_SETTING_VIDEO_PIP]             = KEY_VIDEO_PIP;
        KEYS_FOR_SETTING[ROW_SETTING_VIDEO]                 = KEY_VIDEO;
        KEYS_FOR_SETTING[ROW_SETTING_REFOCUS]               = KEY_REFOCUS;
        KEYS_FOR_SETTING[ROW_SETTING_NORMAL]                = KEY_NORMAL;
        KEYS_FOR_SETTING[ROW_SETTING_OBJECT_TRACKING]       = KEY_OBJECT_TRACKING;
        KEYS_FOR_SETTING[ROW_SETTING_DNG]                   = KEY_DNG;
    };

    public static String getSettingKey(int settingId) {
        return KEYS_FOR_SETTING[settingId];
    }

    public static int getSettingId(String key) {
        int settingIndex = -1;
        for (int i = 0; i < KEYS_FOR_SETTING.length; i++) {
            if (KEYS_FOR_SETTING[i].equals(key)) {
                settingIndex = i;
                break;
            }
        }
        return settingIndex;
    }

    public static int getSettingType(String key) {
        int settingId = getSettingId(key);
        return getSettingType(settingId);
    }

    public static int getSettingType(int settingId) {
        return SETTING_TYPE[settingId];
    }
}
