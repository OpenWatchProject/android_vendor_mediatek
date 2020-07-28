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
package com.mediatek.camera.feature.setting.focus;


import com.mediatek.camera.common.relation.Relation;
import com.mediatek.camera.common.relation.RelationGroup;

/**
 * Focus restriction.
 * There are two different types of restriction which are has setting ui and with out setting ui.
 * In this class RelationGroup will handle those restriction witch has setting ui and
 * sExtendRelation will handle those restriction that without setting ui.
 */

class FocusRestriction {
    static final String FOCUS_LOCK = "focus lock";
    static final String FOCUS_UNLOCK = "focus unlock";
    static final String EXPOSURE_LOCK = "exposure-lock";
    private static final String KEY_FOCUS = "key_focus";
    private static final String KEY_FACE_DETECTION = "key_face_detection";
    private static final String EXPOSURE_KEY = "key_exposure";
    private static final String FLASH_KEY = "key_flash";
    private static RelationGroup sRelation = new RelationGroup();

    static {
        sRelation.setHeaderKey(KEY_FOCUS);
        sRelation.setBodyKeys(KEY_FACE_DETECTION);
        sRelation.addRelation(
                new Relation.Builder(KEY_FOCUS, "auto")
                        .addBody(KEY_FACE_DETECTION, "off", "off")
                        .build());
    }

    private static RelationGroup sAfLockRelationGroup = new RelationGroup();

    static {
        sAfLockRelationGroup.setHeaderKey(KEY_FOCUS);
        sAfLockRelationGroup.setBodyKeys(EXPOSURE_KEY);
        sAfLockRelationGroup.addRelation(
                new Relation.Builder(KEY_FOCUS, FOCUS_LOCK)
                        .addBody(EXPOSURE_KEY, EXPOSURE_LOCK, "true")
                        .build());
        sAfLockRelationGroup.addRelation(
                new Relation.Builder(KEY_FOCUS, FOCUS_UNLOCK)
                        .addBody(EXPOSURE_KEY, EXPOSURE_LOCK, "false")
                        .build());
    }

    private static RelationGroup sAeAfLockRelationGroup = new RelationGroup();

    static {
        sAeAfLockRelationGroup.setHeaderKey(KEY_FOCUS);
        sAeAfLockRelationGroup.setBodyKeys(FLASH_KEY);
        sAeAfLockRelationGroup.addRelation(
                new Relation.Builder(KEY_FOCUS, FOCUS_LOCK)
                        .addBody(FLASH_KEY, "off", "off")
                        .build());
        sAeAfLockRelationGroup.addRelation(
                new Relation.Builder(KEY_FOCUS, FOCUS_UNLOCK)
                        .build());
    }

    /**
     * Restriction which are have setting ui.
     *
     * @return restriction list.
     */
    static RelationGroup getRestriction() {
        return sRelation;
    }

    /**
     * Restriction when Af has been loced.
     *
     * @return restriction list.
     */
    static RelationGroup getAfLockRestriction() {
        return sAfLockRelationGroup;
    }

    /**
     * Restriction when AE/AF has been loced.
     * @return restriction list.
     */
    static RelationGroup getAeAfLockRestriction() {
        return sAeAfLockRelationGroup;
    }

}
