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
 *     MediaTek Inc. (C) 2019. All rights reserved.
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
package com.mediatek.camera.feature.mode.hdr;

import com.mediatek.camera.common.relation.Relation;
import com.mediatek.camera.common.relation.RelationGroup;

/**
 * Slow motion restriction.
 */

class HdrRestriction {
    private static final String HDR_MODE_KEY =
            "com.mediatek.camera.feature.mode.hdr.HdrMode";
    private static RelationGroup sRelationGroup = new RelationGroup();

    static {
        sRelationGroup.setHeaderKey(HDR_MODE_KEY);
        sRelationGroup.setBodyKeys("key_hdr,key_camera_switcher,key_flash,key_dng,key_zsd,key_white_balance,key_anti_flicker," +
                "key_image_properties,key_continuous_shot,key_dual_zoom," +
                "key_scene_mode,key_self_timer,key_iso,key_ais");
        sRelationGroup.addRelation(
                new Relation.Builder(HDR_MODE_KEY, "on")
                        .addBody("key_hdr", "off", "off")
                        .addBody("key_camera_switcher", "back", "back")
                        .addBody("key_flash", "off", "off")
                        .addBody("key_dng", "off", "off")
                        .addBody("key_zsd", "off", "off")
                        .addBody("key_white_balance", "auto", "auto")
                        .addBody("key_anti_flicker", "off", "off")
                        .addBody("key_image_properties", "off", "off")
                        .addBody("key_continuous_shot", "off", "off")
                        .addBody("key_dual_zoom", "off", "off")
                        .addBody("key_scene_mode", "off", "off")
                        .addBody("key_self_timer", "0", "0")
                        .addBody("key_iso", "0", "0")
                        .addBody("key_ais", "off", "off")
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
