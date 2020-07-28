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
 * MediaTek Inc. (C) 2016. All rights reserved.
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
package com.mediatek.camera.feature.setting.scenemode;

import com.mediatek.camera.common.relation.Relation;
import com.mediatek.camera.common.relation.RelationGroup;

/**
 * Scene mode restriction.
 */

public class SceneModeRestriction {
    private static RelationGroup sRelationGroup = new RelationGroup();
    private static RelationGroup sAsdRelationGroup = new RelationGroup();

    static {
        sRelationGroup.setHeaderKey("key_scene_mode");
        sRelationGroup.setBodyKeys("key_iso,key_exposure,key_flash,key_white_balance"
                + ",key_brightness,key_contrast,key_hue,key_saturation,key_sharpness"
                + ",key_color_effect");
        sRelationGroup.addRelation(
                new Relation.Builder("key_scene_mode", "night")
                        .addBody("key_iso", "0", "0")
                        .addBody("key_exposure", "0", "0")
                        .addBody("key_brightness", "middle", "middle")
                        .addBody("key_contrast", "middle", "middle")
                        .addBody("key_hue", "middle", "middle")
                        .addBody("key_saturation", "middle", "middle")
                        .addBody("key_sharpness", "low", "low")
                        .build());
        sRelationGroup.addRelation(
                new Relation.Builder("key_scene_mode", "sunset")
                        .addBody("key_iso", "0", "0")
                        .addBody("key_exposure", "0", "0")
                        .addBody("key_white_balance", "daylight", "daylight")
                        .addBody("key_brightness", "middle", "middle")
                        .addBody("key_contrast", "middle", "middle")
                        .addBody("key_hue", "middle", "middle")
                        .addBody("key_saturation", "middle", "middle")
                        .addBody("key_sharpness", "high", "high")
                        .build());
        sRelationGroup.addRelation(
                new Relation.Builder("key_scene_mode", "party")
                        .addBody("key_iso", "0", "0")
                        .addBody("key_exposure", "0", "0")
                        .addBody("key_brightness", "middle", "middle")
                        .addBody("key_contrast", "middle", "middle")
                        .addBody("key_hue", "middle", "middle")
                        .addBody("key_saturation", "middle", "middle")
                        .addBody("key_sharpness", "middle", "middle")
                        .build());
        sRelationGroup.addRelation(
                new Relation.Builder("key_scene_mode", "portrait")
                        .addBody("key_iso", "0", "0")
                        .addBody("key_exposure", "0", "0")
                        .addBody("key_brightness", "middle", "middle")
                        .addBody("key_contrast", "middle", "middle")
                        .addBody("key_hue", "middle", "middle")
                        .addBody("key_saturation", "middle", "middle")
                        .addBody("key_sharpness", "low", "low")
                        .build());
        sRelationGroup.addRelation(
                new Relation.Builder("key_scene_mode", "landscape")
                        .addBody("key_iso", "0", "0")
                        .addBody("key_exposure", "0", "0")
                        .addBody("key_white_balance", "daylight", "daylight")
                        .addBody("key_brightness", "middle", "middle")
                        .addBody("key_contrast", "middle", "middle")
                        .addBody("key_hue", "middle", "middle")
                        .addBody("key_saturation", "middle", "middle")
                        .addBody("key_sharpness", "high", "high")
                        .build());
        sRelationGroup.addRelation(
                new Relation.Builder("key_scene_mode", "night-portrait")
                        .addBody("key_iso", "0", "0")
                        .addBody("key_exposure", "0", "0")
                        .addBody("key_brightness", "middle", "middle")
                        .addBody("key_contrast", "middle", "middle")
                        .addBody("key_hue", "middle", "middle")
                        .addBody("key_saturation", "middle", "middle")
                        .addBody("key_sharpness", "low", "low")
                        .build());
        sRelationGroup.addRelation(
                new Relation.Builder("key_scene_mode", "theatre")
                        .addBody("key_iso", "0", "0")
                        .addBody("key_exposure", "0", "0")
                        .addBody("key_brightness", "middle", "middle")
                        .addBody("key_contrast", "middle", "middle")
                        .addBody("key_hue", "middle", "middle")
                        .addBody("key_saturation", "low", "low")
                        .addBody("key_sharpness", "high", "high")
                        .build());
        sRelationGroup.addRelation(
                new Relation.Builder("key_scene_mode", "beach")
                        .addBody("key_iso", "0", "0")
                        .addBody("key_exposure", "1", "1")
                        .addBody("key_brightness", "middle", "middle")
                        .addBody("key_contrast", "middle", "middle")
                        .addBody("key_hue", "middle", "middle")
                        .addBody("key_saturation", "middle", "middle")
                        .addBody("key_sharpness", "high", "high")
                        .build());
        sRelationGroup.addRelation(
                new Relation.Builder("key_scene_mode", "snow")
                        .addBody("key_iso", "0", "0")
                        .addBody("key_exposure", "1", "1")
                        .addBody("key_brightness", "middle", "middle")
                        .addBody("key_contrast", "middle", "middle")
                        .addBody("key_hue", "middle", "middle")
                        .addBody("key_saturation", "middle", "middle")
                        .addBody("key_sharpness", "high", "high")
                        .build());
        sRelationGroup.addRelation(
                new Relation.Builder("key_scene_mode", "steadyphoto")
                        .addBody("key_iso", "0", "0")
                        .addBody("key_exposure", "0", "0")
                        .addBody("key_brightness", "middle", "middle")
                        .addBody("key_contrast", "middle", "middle")
                        .addBody("key_hue", "middle", "middle")
                        .addBody("key_saturation", "middle", "middle")
                        .addBody("key_sharpness", "middle", "middle")
                        .build());
        sRelationGroup.addRelation(
                new Relation.Builder("key_scene_mode", "fireworks")
                        .addBody("key_iso", "0", "0")
                        .addBody("key_exposure", "0", "0")
                        .addBody("key_flash", "off", "off")
                        .addBody("key_brightness", "middle", "middle")
                        .addBody("key_contrast", "middle", "middle")
                        .addBody("key_hue", "middle", "middle")
                        .addBody("key_saturation", "middle", "middle")
                        .addBody("key_sharpness", "middle", "middle")
                        .build());
        sRelationGroup.addRelation(
                new Relation.Builder("key_scene_mode", "sports")
                        .addBody("key_iso", "0", "0")
                        .addBody("key_exposure", "0", "0")
                        .addBody("key_brightness", "middle", "middle")
                        .addBody("key_contrast", "middle", "middle")
                        .addBody("key_hue", "middle", "middle")
                        .addBody("key_saturation", "middle", "middle")
                        .addBody("key_sharpness", "middle", "middle")
                        .build());
        sRelationGroup.addRelation(
                new Relation.Builder("key_scene_mode", "candlelight")
                        .addBody("key_iso", "0", "0")
                        .addBody("key_exposure", "0", "0")
                        .addBody("key_white_balance", "incandescent", "incandescent")
                        .addBody("key_brightness", "middle", "middle")
                        .addBody("key_contrast", "middle", "middle")
                        .addBody("key_hue", "middle", "middle")
                        .addBody("key_saturation", "middle", "middle")
                        .addBody("key_sharpness", "middle", "middle")
                        .build());
        sRelationGroup.addRelation(
                new Relation.Builder("key_scene_mode", "auto-scene-detection")
                        .addBody("key_iso", "0", "0")
                        .addBody("key_exposure", "0", "0")
                        .addBody("key_white_balance", "auto", "auto")
                        .addBody("key_brightness", "middle", "middle")
                        .addBody("key_contrast", "middle", "middle")
                        .addBody("key_hue", "middle", "middle")
                        .addBody("key_saturation", "middle", "middle")
                        .addBody("key_sharpness", "middle", "middle")
                        .addBody("key_color_effect", "none", "none")
                        .build());
    }

    /**
     * Scene mode restriction which are have setting ui.
     *
     * @return restriction list.
     */
    static RelationGroup getRestrictionGroup() {
        return sRelationGroup;
    }
}
