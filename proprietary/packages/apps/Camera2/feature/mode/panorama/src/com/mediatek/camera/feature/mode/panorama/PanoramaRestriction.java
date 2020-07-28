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
package com.mediatek.camera.feature.mode.panorama;

import com.mediatek.camera.common.relation.Relation;
import com.mediatek.camera.common.relation.RelationGroup;

/**
 * Slow motion restriction.
 */

class PanoramaRestriction {
    // This key must same as every setting keys define, otherwise will have no role about the
    // restriction.
    private static final String KEY_PANORAMA = PanoramaMode.class.getName();
    private static final String KEY_CSHOT = "key_continuous_shot";
    private static final String KEY_FLASH = "key_flash";
    private static final String KEY_FACE_DETECTION = "key_face_detection";
    private static final String KEY_HDR = "key_hdr";
    private static final String KEY_ZSD = "key_zsd";
    private static final String KEY_DNG = "key_dng";
    private static final String KEY_FOCUS = "key_focus";
    private static final String KEY_EXPOSURE = "key_exposure";
    private static final String KEY_WHITE_BALANCE = "key_white_balance";
    private static final String KEY_SELF_TIMER = "key_self_timer";
    private static final String KEY_CAMERA_SWITCHER = "key_camera_switcher";
    private static final String KEY_SCENE_MODE = "key_scene_mode";
    private static final String KEY_COLOR_EFFECT = "key_color_effect";
    private static final String KEY_AIS = "key_ais";
    private static final String KEY_DUAL_ZOOM = "key_dual_zoom";

    private static RelationGroup sRelation = new RelationGroup();
    static {
        sRelation.setHeaderKey(KEY_PANORAMA);
        sRelation.setBodyKeys(KEY_CSHOT + "," + KEY_FLASH + "," + KEY_FACE_DETECTION + "," +
                KEY_HDR + "," + KEY_ZSD + "," + KEY_DNG + "," + KEY_SELF_TIMER + "," +
                KEY_CAMERA_SWITCHER + "," + KEY_SCENE_MODE + "," +
                KEY_COLOR_EFFECT + "," + KEY_AIS);
        sRelation.addRelation(
                new Relation.Builder(KEY_PANORAMA, "on")
                        .addBody(KEY_CSHOT, "off", "off")
                        .addBody(KEY_FLASH, "off", "off")
                        .addBody(KEY_FACE_DETECTION, "off", "off")
                        .addBody(KEY_HDR, "off", "off")
                        .addBody(KEY_ZSD, "off", "off")
                        .addBody(KEY_DNG, "off", "off")
                        .addBody(KEY_SELF_TIMER, "0", "0")
                        .addBody(KEY_CAMERA_SWITCHER, "back", "back")
                        .addBody(KEY_SCENE_MODE, "off", "off")
                        .addBody(KEY_COLOR_EFFECT, "none", "none")
                        .addBody(KEY_AIS, "off", "off")
                        .build());
    }

    private static RelationGroup s3aRelation = new RelationGroup();
    static {
        s3aRelation.setHeaderKey(KEY_PANORAMA);
        s3aRelation.setBodyKeys(KEY_FOCUS + "," + KEY_EXPOSURE + "," + KEY_WHITE_BALANCE
                + "," + KEY_DUAL_ZOOM);
        s3aRelation.addRelation(
                new Relation.Builder(KEY_PANORAMA, "on")
                        .addBody(KEY_FOCUS, "auto", "auto")
                        .addBody(KEY_EXPOSURE, "exposure-lock", "true")
                        .addBody(KEY_WHITE_BALANCE, "white-balance-lock", "true")
                        .addBody(KEY_DUAL_ZOOM, "limit", "limit")
                        .build());
        s3aRelation.addRelation(
                new Relation.Builder(KEY_PANORAMA, "off")
                        .addBody(KEY_FOCUS, null, null)
                        .addBody(KEY_EXPOSURE, "exposure-lock", "false")
                        .addBody(KEY_WHITE_BALANCE, "white-balance-lock", "false")
                        .addBody(KEY_DUAL_ZOOM, "on", "on, off")
                        .build());
    }

    /**
     * Restriction witch are have setting ui.
     * @return restriction list.
     */
    static RelationGroup getRestriction() {
        return sRelation;
    }

    /**
     * Restriction to AE, AWB, AF.
     * @return restriction list.
     */
    static RelationGroup get3aRestriction() {
        return s3aRelation;
    }

}
