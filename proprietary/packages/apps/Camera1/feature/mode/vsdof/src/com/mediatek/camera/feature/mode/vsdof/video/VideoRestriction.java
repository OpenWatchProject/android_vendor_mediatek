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
 *     MediaTek Inc. (C) 2016. All rights reserved.
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
package com.mediatek.camera.feature.mode.vsdof.video;

import com.mediatek.camera.common.relation.Relation;
import com.mediatek.camera.common.relation.RelationGroup;


/**
 * video mode restriction.
 */

public class VideoRestriction {
    private static final String VIDEO_MODE_KEY = SdofVideoMode.class.getName();
    public static final String KEY_VIDEO_QUALITY = "key_video_quality";
    private static final String KEY_CAMERA_SWITCHER = "key_camera_switcher";
    private static final String KEY_SCENE_MODE = "key_scene_mode";
    private static final String KEY_NOISE_REDUCTION = "key_noise_reduction";
    private static final String KEY_NOISE_EIS = "key_eis";
    private static final String KEY_HDR = "key_hdr";
    private static final String KEY_EXPOSURE = "key_exposure";
    private static final String KEY_CAMERA_ZOOM = "key_camera_zoom";
    private static final String KEY_DUAL_ZOOM = "key_dual_zoom";
    private static final String KEY_FOCUS = "key_focus";
    private static final String KEY_ZSD = "key_zsd";
    private static final String KEY_COLOR_EFFECT = "key_color_effect";
    private static final String KEY_WHITE_BALANCE = "key_white_balance";
    private static final String KEY_FLASH = "key_flash";
    private static final String KEY_ANTI_FLICKER = "key_anti_flicker";
    private static final String KEY_BRIGHTNESS = "key_brightness";
    private static final String KEY_CONTRAST = "key_contrast";
    private static final String KEY_HUE = "key_hue";
    private static final String KEY_SATURATION = "key_saturation";
    private static final String KEY_SHARPNESS = "key_sharpness";
    private static final String KEY_FACE_DETECTION = "key_face_detection";
    private static final String FOCUS_SOUND = "focus-sound";

    private static RelationGroup sPreviewRelationGroup = new RelationGroup();

    static {
        sPreviewRelationGroup.setHeaderKey(VIDEO_MODE_KEY);
        sPreviewRelationGroup.setBodyKeys(
                KEY_SCENE_MODE + "," +
                        KEY_CAMERA_SWITCHER + "," +
                        KEY_NOISE_REDUCTION + "," +
                        KEY_NOISE_EIS + "," +
                        KEY_HDR + "," +
                        KEY_SCENE_MODE + "," +
                        KEY_CAMERA_ZOOM + "," +
                        KEY_DUAL_ZOOM + "," +
                        KEY_FOCUS + "," +
                        KEY_VIDEO_QUALITY + "," +
                        KEY_COLOR_EFFECT + "," +
                        KEY_FLASH + "," +
                        KEY_ZSD + "," +
                        KEY_ANTI_FLICKER + "," +
                        KEY_BRIGHTNESS + "," +
                        KEY_CONTRAST + "," +
                        KEY_HUE + "," +
                        KEY_SATURATION + "," +
                        KEY_SHARPNESS + "," +
                        KEY_FACE_DETECTION);
        sPreviewRelationGroup.addRelation(
                new Relation.Builder(VIDEO_MODE_KEY, "preview")
                        .addBody(KEY_CAMERA_SWITCHER, "back", "back")
                        .addBody(KEY_HDR, "off", "off")
                        .addBody(KEY_SCENE_MODE, "off", getVideoSceneRestriction())
                        .addBody(KEY_CAMERA_ZOOM, "off", "off")
                        .addBody(KEY_DUAL_ZOOM, "off", "off")
                        .addBody(KEY_FOCUS, "continuous-video", "continuous-video,auto")
                        .addBody(KEY_ZSD, "off", "off")
                        .addBody(KEY_COLOR_EFFECT, "none", "none")
                        .addBody(KEY_FLASH, "off", "off")
                        .addBody(KEY_NOISE_EIS, "off", "off")
                        .addBody(KEY_ANTI_FLICKER, "auto", "auto")
                        .addBody(KEY_BRIGHTNESS, "middle", "middle")
                        .addBody(KEY_CONTRAST, "middle", "middle")
                        .addBody(KEY_HUE, "middle", "middle")
                        .addBody(KEY_SATURATION, "middle", "middle")
                        .addBody(KEY_SHARPNESS, "middle", "middle")
                        .addBody(KEY_FACE_DETECTION, "off", "off")
                        .build());
    }

    private static RelationGroup sRecordingRelationGroupForMode = new RelationGroup();
    private static RelationGroup sRecordingRelationGroupForUi = new RelationGroup();

    static {
        sRecordingRelationGroupForMode.setHeaderKey(VIDEO_MODE_KEY);
        sRecordingRelationGroupForMode.setBodyKeys("key_focus");
        sRecordingRelationGroupForMode.addRelation(
                new Relation.Builder(VIDEO_MODE_KEY, "recording")
                        .addBody("key_focus", "auto", "auto")
                        .build());
        sRecordingRelationGroupForMode.addRelation(
                new Relation.Builder(VIDEO_MODE_KEY, "stop-recording")
                        .addBody("key_focus", "continuous-video", "continuous-video,auto")
                        .build());
    }

    static {
        sRecordingRelationGroupForUi.setHeaderKey(VIDEO_MODE_KEY);
        sRecordingRelationGroupForUi.setBodyKeys("key_focus");
        sRecordingRelationGroupForUi.addRelation(
                new Relation.Builder(VIDEO_MODE_KEY, "recording")
                        .addBody("key_focus", FOCUS_SOUND, "false")
                        .build());
        sRecordingRelationGroupForUi.addRelation(
                new Relation.Builder(VIDEO_MODE_KEY, "stop-recording")
                        .addBody("key_focus", FOCUS_SOUND, "true")
                        .build());
    }

    /**
     * Video restriction witch are have setting ui.
     *
     * @return restriction list.
     */
    static RelationGroup getPreviewRelation() {
        return sPreviewRelationGroup;
    }

    /**
     * Video restriction witch are during recording.
     *
     * @return restriction list.
     */
    static RelationGroup getRecordingRelationForMode() {
        return sRecordingRelationGroupForMode;
    }

    /**
     * Video restriction witch are during recording.
     *
     * @return restriction list.
     */
    static RelationGroup getRecordingRelationForUi() {
        return sRecordingRelationGroupForUi;
    }

    /**
     * Video restriction for scene mode.
     * @return scene mode restriction
     */
    static String getVideoSceneRestriction() {
        return "off," +
                "night," +
                "sunset," +
                "party," +
                "portrait," +
                "landscape," +
                "night-portrait," +
                "theatre," +
                "beach," +
                "snow," +
                "steadyphoto," +
                "sports," +
                "candlelight";
    }
}
