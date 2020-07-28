/*
 * Copyright Statement:
 *
 *   This software/firmware and related documentation ("MediaTek Software") are
 *   protected under relevant copyright laws. The information contained herein is
 *   confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *   the prior written permission of MediaTek inc. and/or its licensors, any
 *   reproduction, modification, use or disclosure of MediaTek Software, and
 *   information contained herein, in whole or in part, shall be strictly
 *   prohibited.
 *
 *   MediaTek Inc. (C) 2016. All rights reserved.
 *
 *   BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *   THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *   RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *   ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *   WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *   WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *   NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *   RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *   INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *   TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *   RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *   OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *   SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *   RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *   STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *   ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *   RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *   MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *   CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *   The following software/firmware and/or related documentation ("MediaTek
 *   Software") have been modified by MediaTek Inc. All revisions are subject to
 *   any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.camera.feature.mode.vsdof.photo;

import android.hardware.camera2.CameraCharacteristics;
import com.mediatek.camera.common.relation.DataStore;
import com.mediatek.camera.common.relation.Relation;
import com.mediatek.camera.common.relation.RelationGroup;

/**
 * This class is stereo mode restriction.
 */

public class SdofPhotoRestriction {
    // This key must same as every setting keys define, otherwise will have no role about the
    // restriction.
    private static final String KEY_CSHOT = "key_continuous_shot";
    private static final String KEY_FLASH = "key_flash";
    private static final String KEY_ZSD = "key_zsd";
    private static final String KEY_DNG = "key_dng";
    private static final String KEY_CAMERA_SWITCHER = "key_camera_switcher";
    private static final String KEY_CAMERA_ZOOM = "key_camera_zoom";
    private static final String KEY_ANTI_FLICKER = "key_anti_flicker";
    private static final String KEY_DUAL_ZOOM = "key_dual_zoom";

    private static CameraCharacteristics sCharacteristics = null;
    private static final String PHOTO_MODE_KEY = SdofPhotoMode.class.getName();
    private static DataStore sDataStore = null;
    private static RelationGroup sRelation = new RelationGroup();

    /**
     * Restriction witch are have setting ui.
     * @return restriction list.
     */
    public static RelationGroup getRestriction() {
        sRelation.setHeaderKey(PHOTO_MODE_KEY);
        sRelation.setBodyKeys(KEY_CSHOT + "," + KEY_FLASH + "," +
                KEY_ZSD + "," + KEY_DNG + "," +
                KEY_CAMERA_SWITCHER + "," +
                KEY_CAMERA_ZOOM + "," + KEY_ANTI_FLICKER + "," +
                KEY_DUAL_ZOOM);
        sRelation.addRelation(
                new Relation.Builder(PHOTO_MODE_KEY, "on")
                        .addBody(KEY_CSHOT, "off", "off")
                        .addBody(KEY_FLASH, "off", "off")
                        .addBody(KEY_ZSD, "on", "on")
                        .addBody(KEY_DNG, "off", "off")
                        .addBody(KEY_CAMERA_SWITCHER, "back", "back")
                        .addBody(KEY_CAMERA_ZOOM, "off", "off")
                        .addBody(KEY_ANTI_FLICKER, "off", "off")
                        .addBody(KEY_DUAL_ZOOM, "off", "off")
                        .build());
        return sRelation;
    }

    public static void setCameraCharacteristics(CameraCharacteristics characteristics,
                                                DataStore dataStore) {
        sCharacteristics = characteristics;
        sDataStore = dataStore;
    }

}
