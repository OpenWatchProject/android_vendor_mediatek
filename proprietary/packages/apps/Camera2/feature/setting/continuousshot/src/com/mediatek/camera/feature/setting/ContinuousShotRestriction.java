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
package com.mediatek.camera.feature.setting;

import com.mediatek.camera.common.relation.Relation;
import com.mediatek.camera.common.relation.RelationGroup;

/**
 * This restriction is used by continuous shot.
 */

public class ContinuousShotRestriction {
    // This key must same as every setting keys define, otherwise will have no role about the
    // restriction.
    private static final String KEY_CSHOT = "key_continuous_shot";
    private static final String KEY_DNG = "key_dng";
    private static final String KEY_FACE_DETECTION = "key_face_detection";
    private static final String KEY_DUAL_ZOOM = "key_dual_zoom";
    private static final String KEY_SCENE_MODE = "key_scene_mode";
    private static final String KEY_FOCUS = "key_focus";
    private static final String KEY_ZSD = "key_zsd";
    private static final String FOCUS_UI = "focus-ui";
    private static final String FOCUS_SOUND = "focus-sound";

    private static RelationGroup sRelation = new RelationGroup();
    static {
        sRelation.setHeaderKey(KEY_CSHOT);
        sRelation.setBodyKeys(KEY_DNG + "," + KEY_FACE_DETECTION
                + "," + KEY_DUAL_ZOOM);
        sRelation.addRelation(
                new Relation.Builder(KEY_CSHOT, "on")
                        .addBody(KEY_DNG, "off", "off")
                        .addBody(KEY_FACE_DETECTION, "off", "on, off")
                        .addBody(KEY_DUAL_ZOOM, "limit", "limit")
                        .build());
    }

    private static RelationGroup sAsdRelation = new RelationGroup();
    static {
        sAsdRelation.setHeaderKey(KEY_CSHOT);
        sAsdRelation.setBodyKeys(KEY_SCENE_MODE);
        sAsdRelation.addRelation(
                new Relation.Builder(KEY_CSHOT, "on")
                .addBody(KEY_SCENE_MODE, "off",
                        "off, night, sunset, party, portrait, landscape, night-portrait," +
                                " theatre, beach, snow, steadyphoto, fireworks, sports," +
                                " candlelight")
                .build()
        );
    }

    private static RelationGroup sFocusUiRelation = new RelationGroup();
    static {
        sFocusUiRelation.setHeaderKey(KEY_CSHOT);
        sFocusUiRelation.setBodyKeys(KEY_FOCUS);
        sFocusUiRelation.addRelation(
                new Relation.Builder(KEY_CSHOT, "on")
                        .addBody(KEY_FOCUS, FOCUS_UI, "false")
                        .build());
        sFocusUiRelation.addRelation(
                new Relation.Builder(KEY_CSHOT, "off")
                        .addBody(KEY_FOCUS, FOCUS_UI, "true")
                        .build());
    }

    private static RelationGroup sFocusSoundRelation = new RelationGroup();
    static {
        sFocusSoundRelation.setHeaderKey(KEY_CSHOT);
        sFocusSoundRelation.setBodyKeys(KEY_FOCUS);
        sFocusSoundRelation.addRelation(
                new Relation.Builder(KEY_CSHOT, "on")
                        .addBody(KEY_FOCUS, FOCUS_SOUND, "false")
                        .build());
        sFocusSoundRelation.addRelation(
                new Relation.Builder(KEY_CSHOT, "off")
                        .addBody(KEY_FOCUS, FOCUS_SOUND, "true")
                        .build());
    }

    /**
     * Restriction to disable and enable focus UI during continuous shot.
     *
     * @return The restriction list for focus UI.
     */
    static RelationGroup getFocusUiRestriction() {
        return sFocusUiRelation;
    }

    /**
     * Restriction to disable and enable focus sound during continuous shot.
     *
     * @return The restriction list for focus sound.
     */
    static RelationGroup getFocusSoundRestriction() {
        return sFocusSoundRelation;
    }

    /**
     * Restriction witch are have setting ui.
     * @return restriction list.
     */
    static RelationGroup getRestriction() {
        return sRelation;
    }

    /**
     * Restriction to auto scene detection.
     * @return restriction list.
     */
    static RelationGroup getAsdRestriction() {
        return sAsdRelation;
    }
}
