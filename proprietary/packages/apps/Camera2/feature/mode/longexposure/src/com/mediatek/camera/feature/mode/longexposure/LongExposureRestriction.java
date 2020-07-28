/*
 *   Copyright Statement:
 *
 *     This software/firmware and related documentation ("MediaTek Software") are
 *     protected under relevant copyright laws. The information contained herein is
 *     confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *     the prior written permission of MediaTek inc. and/or its licensors, any
 *     reproduction, modification, use or disclosure of MediaTek Software, and
 *     information contained herein, in whole or in part, shall be strictly
 *     prohibited.
 *
 *     MediaTek Inc. (C) 2017. All rights reserved.
 *
 *     BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *    THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *     RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *     ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *     WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *     NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *     RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *     TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *     RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *     OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *     SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *     RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *     STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *     ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *     RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *     MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *     CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     The following software/firmware and/or related documentation ("MediaTek
 *     Software") have been modified by MediaTek Inc. All revisions are subject to
 *     any receiver's applicable license agreements with MediaTek Inc.
 */
package com.mediatek.camera.feature.mode.longexposure;

import com.mediatek.camera.common.relation.Relation;
import com.mediatek.camera.common.relation.RelationGroup;

/**
 * Slow motion restriction.
 */

class LongExposureRestriction {
    private static final String LONG_EXPOSURE_MODE_KEY =
            "com.mediatek.camera.feature.mode.longexposure.LongExposureMode";
    private static final String VALUE_OFF = "off";
    private static RelationGroup sRelationGroup = new RelationGroup();


    private static final String KEY_EIS = "key_eis";
    private static final String KEY_SCENE_MODE = "key_scene_mode";
    private static final String KEY_MICROPHONE = "key_microphone";
    private static final String KEY_HDR = "key_hdr";
    private static final String KEY_COLOR_EFFECT = "key_color_effect";
    private static final String KEY_FLASH = "key_flash";
    private static final String KEY_WHITE_BALANCE = "key_white_balance";
    private static final String KEY_NOISE_REDUCTION = "key_noise_reduction";
    private static final String KEY_CAMERA_SWITCHER = "key_camera_switcher";
    private static final String KEY_ANTI_FLICKER = "key_anti_flicker";
    private static final String KEY_IMAGE_PROPERTIES = "key_image_properties";
    private static final String KEY_BRIGHTNESS = "key_brightness";
    private static final String KEY_CONTRAST = "key_contrast";
    private static final String KEY_HUE = "key_hue";
    private static final String KEY_SATURATION = "key_saturation";
    private static final String KEY_SHARPNESS = "key_sharpness";
    private static final String KEY_FACE_DETECTION = "key_face_detection";
    private static final String KEY_AIS = "key_ais";
    private static final String KEY_DNG = "key_dng";
    private static final String KEY_SELF_TIMER = "key_self_timer";
    private static final String KEY_EXPOSURE = "key_exposure";
    private static final String KEY_ZSD = "key_zsd";
    private static final String KEY_DUAL_ZOOM = "key_dual_zoom";
    private static final String KEY_CSHOT = "key_continuous_shot";


    static {
        sRelationGroup.setHeaderKey(LONG_EXPOSURE_MODE_KEY);
        sRelationGroup.setBodyKeys(
                KEY_EIS + "," +
                        KEY_SCENE_MODE + "," +
                        KEY_MICROPHONE + "," +
                        KEY_HDR + "," +
                        KEY_COLOR_EFFECT + "," +
                        KEY_FLASH + "," +
                        KEY_WHITE_BALANCE + "," +
                        KEY_NOISE_REDUCTION + "," +
                        KEY_CAMERA_SWITCHER + "," +
                        KEY_ANTI_FLICKER + "," +
                        KEY_IMAGE_PROPERTIES + "," +
                        KEY_BRIGHTNESS + "," +
                        KEY_CONTRAST + "," +
                        KEY_HUE + "," +
                        KEY_SATURATION + "," +
                        KEY_SHARPNESS + "," +
                        KEY_FACE_DETECTION + "," +
                        KEY_AIS + "," +
                        KEY_DNG + "," +
                        KEY_SELF_TIMER + "," +
                        KEY_EXPOSURE + "," +
                        KEY_ZSD + "," +
                        KEY_DUAL_ZOOM + "," +
                        KEY_CSHOT);
        sRelationGroup.addRelation(
                new Relation.Builder(LONG_EXPOSURE_MODE_KEY, "on")
                        .addBody(KEY_EIS, VALUE_OFF, VALUE_OFF)
                        .addBody(KEY_SCENE_MODE, VALUE_OFF, VALUE_OFF)
                        .addBody(KEY_HDR, VALUE_OFF, VALUE_OFF)
                        .addBody(KEY_NOISE_REDUCTION, VALUE_OFF, VALUE_OFF)
                        .addBody(KEY_MICROPHONE, VALUE_OFF, VALUE_OFF)
                        .addBody(KEY_FLASH, VALUE_OFF, VALUE_OFF)
                        .addBody(KEY_BRIGHTNESS, "middle", "middle")
                        .addBody(KEY_CONTRAST, "middle", "middle")
                        .addBody(KEY_HUE, "middle", "middle")
                        .addBody(KEY_SATURATION, "middle", "middle")
                        .addBody(KEY_SHARPNESS, "middle", "middle")
                        .addBody(KEY_WHITE_BALANCE, "auto", "auto")
                        .addBody(KEY_ANTI_FLICKER, "auto", "auto")
                        .addBody(KEY_CAMERA_SWITCHER, "back", "back")
                        .addBody(KEY_COLOR_EFFECT, "none", "none")
                        .addBody(KEY_FACE_DETECTION, "off", "off")
                        .addBody(KEY_AIS, "off", "off")
                        .addBody(KEY_DNG, "off", "off")
                        .addBody(KEY_SELF_TIMER, "0", "0")
                        .addBody(KEY_EXPOSURE, "0", "0")
                        .addBody(KEY_ZSD, "off", "off")
                        .addBody(KEY_DUAL_ZOOM, "off", "off")
                        .addBody(KEY_CSHOT, "off", "off")
                        .build());
    }

    /**
     * Slow motion restriction witch are have setting ui.
     *
     * @return restriction list.
     */
    static RelationGroup getRestriction() {
        return sRelationGroup;
    }

}
