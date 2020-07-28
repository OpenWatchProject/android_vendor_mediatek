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
package com.mediatek.camera.feature.setting.hdr;

import com.mediatek.camera.common.relation.Relation;
import com.mediatek.camera.common.relation.RelationGroup;

import java.util.concurrent.CopyOnWriteArrayList;

/**
 * HDR restriction.
 * There are two different types of restriction which are has setting ui and with out setting ui.
 * In this class RelationGroup will handle those restriction witch has setting ui and
 * sExtendRelation will handle those restriction that without setting ui.
 */

class HdrRestriction {

    private static final String KEY_STEREO_DISTANCE_MODE = "stereo-distance-measurement";
    private static final String KEY_STEREO_DEPTH_AF_MODE = "stereo-depth-af";
    private static final String KEY_SATURATION_MODE = "saturation";
    private static final String KEY_BRIGHTNESS_MODE = "brightness";
    private static final String KEY_CONTRAST_MODE = "contrast";
    private static final String KEY_EDGE_MODE = "edge";
    private static final String KEY_HUE_MODE = "hue";
    private static final String VALUE_MIDDLE = "middle";
    private static final String VALUE_OFF = "off";

    private static CopyOnWriteArrayList<HdrRelation> sExtendRelation =
            new CopyOnWriteArrayList<>();

    private static RelationGroup sRelationGroup = new RelationGroup();

    static {
        sRelationGroup.setHeaderKey("key_hdr");
        sRelationGroup.setBodyKeys("key_flash, key_scene_mode, key_dng, key_continuous_shot" +
                ", key_white_balance, key_color_effect, key_zsd, key_iso, key_ais, key_asd" +
                ",key_brightness,key_contrast,key_hue,key_saturation,key_sharpness");
        sRelationGroup.addRelation(
                new Relation.Builder("key_hdr", "on")
                        .addBody("key_flash", "off", "off,on,auto")
                        .addBody("key_scene_mode", "hdr", "hdr")
                        .addBody("key_dng", "off", "off,on")
                        .addBody("key_continuous_shot", "off", "off")
                        .addBody("key_white_balance", "auto", "auto")
                        .addBody("key_color_effect", "none", "none")
                        .addBody("key_iso", "0", "0")
                        .addBody("key_ais", "off", "off")
                        .addBody("key_asd", "off", "off")
                        .addBody("key_brightness", "middle", "middle")
                        .addBody("key_contrast", "middle", "middle")
                        .addBody("key_hue", "middle", "middle")
                        .addBody("key_saturation", "middle", "middle")
                        .addBody("key_sharpness", "middle", "middle")
                        .build());
        sRelationGroup.addRelation(
                new Relation.Builder("key_hdr", "auto")
                        .addBody("key_flash", "off", "off,on,auto")
                        .addBody("key_scene_mode", "hdr", "hdr")
                        .addBody("key_dng", "off", "off,on")
                        .addBody("key_continuous_shot", "off", "off")
                        .addBody("key_white_balance", "auto", "auto")
                        .addBody("key_color_effect", "none", "none")
                        .addBody("key_iso", "0", "0")
                        .addBody("key_ais", "off", "off")
                        .addBody("key_asd", "off", "off")
                        .addBody("key_brightness", "middle", "middle")
                        .addBody("key_contrast", "middle", "middle")
                        .addBody("key_hue", "middle", "middle")
                        .addBody("key_saturation", "middle", "middle")
                        .addBody("key_sharpness", "middle", "middle")
                        .build());
    }

    /**
     * HDR restriction to the settings without ui.
     * @return restriction list.
     */
     static  CopyOnWriteArrayList<HdrRelation> getHdrRelation() {
        if (sExtendRelation.isEmpty()) {
            sExtendRelation.add(new HdrRelation(KEY_STEREO_DISTANCE_MODE, VALUE_OFF));
            sExtendRelation.add(new HdrRelation(KEY_STEREO_DEPTH_AF_MODE, VALUE_OFF));
            sExtendRelation.add(new HdrRelation(KEY_BRIGHTNESS_MODE, VALUE_MIDDLE));
            sExtendRelation.add(new HdrRelation(KEY_SATURATION_MODE, VALUE_MIDDLE));
            sExtendRelation.add(new HdrRelation(KEY_CONTRAST_MODE, VALUE_MIDDLE));
            sExtendRelation.add(new HdrRelation(KEY_EDGE_MODE, VALUE_MIDDLE));
            sExtendRelation.add(new HdrRelation(KEY_HUE_MODE, VALUE_MIDDLE));
        }
        return sExtendRelation;
    }

    /**
     * HDR restriction witch are have setting ui.
     * @return restriction list.
     */
     static RelationGroup getHdrRestriction() {
        return sRelationGroup;
    }

    /**
     * Internal class use to keep restriction key-value.
     */
     static class HdrRelation {
        private String mKey;
        private String mValue;
        /**
         * The construction function.
         * @param key for setting
         * @param value for the key
         */
         HdrRelation(String key, String value) {
            this.mKey = key;
            this.mValue = value;
        }

        /**
         * Used get the setting key.
         * @return setting key.
         */
         String getKey() {
            return mKey;
        }

        /**
         * Used get the setting value correspond to the key.
         * @return setting value correspond to the key.
         */
         String getValue() {
            return mValue;
        }
    }
}
