/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensor. Without
 * the prior written permission of MediaTek inc. and/or its licensor, any
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
 * NON-INFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
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
package com.mediatek.camera.common.relation.tests;

import com.mediatek.camera.common.SettingCreator;
import com.mediatek.camera.common.relation.Relation;
import com.mediatek.camera.common.relation.RelationGroup;
import com.mediatek.camera.common.relation.RestrictionDispatcher;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.SettingTable;

import junit.framework.Assert;

import org.junit.Test;

import java.util.ArrayList;
import java.util.List;

/**
 * Unit test for {@link com.mediatek.camera.common.relation.RestrictionDispatcher}.
 */
public class RestrictionDispatcherTest {
    private SettingTable mTable = new SettingTable();
    private RestrictionDispatcher mDispatcher = new RestrictionDispatcher(mTable);
    private SettingCreator mCreator = new SettingCreator();

    /**
     * Test the restriction dispatcher can dispatch the override value to
     * target settings.
     */
    @Test
    public void testDispatchNormalRelation() {
        ICameraSetting flashSetting = createTestFlashSetting();
        ICameraSetting sceneSetting = createTestSceneSetting();
        ICameraSetting hdrSetting = createTestHdrSetting();
        mTable.add(flashSetting);
        mTable.add(sceneSetting);
        mTable.add(hdrSetting);

        List<String> flashEntryValues = new ArrayList<>();
        flashEntryValues.add("on");
        flashEntryValues.add("off");
        flashEntryValues.add("auto");
        List<String> sceneEntryValues = new ArrayList<>();
        sceneEntryValues.add("hdr");

        RelationGroup group = configureHdrRelation();
        mDispatcher.dispatch(group.getRelation("on", true));

        Assert.assertEquals("off", flashSetting.getValue());
        Assert.assertEquals("hdr", sceneSetting.getValue());
        Assert.assertEquals(flashEntryValues, flashSetting.getEntryValues());
        Assert.assertEquals(sceneEntryValues, sceneSetting.getEntryValues());
    }

    /**
     * Test the restriction dispatcher can normally parse the relation info that its body keys has
     * some needless space and dispatch the override value to target settings.
     */
    @Test
    public void testDispatchRelationHasSpace() {
        ICameraSetting flashSetting = createTestFlashSetting();
        ICameraSetting sceneSetting = createTestSceneSetting();
        ICameraSetting hdrSetting = createTestHdrSetting();
        mTable.add(flashSetting);
        mTable.add(sceneSetting);
        mTable.add(hdrSetting);

        List<String> flashEntryValues = new ArrayList<>();
        flashEntryValues.add("on");
        flashEntryValues.add("off");
        flashEntryValues.add("auto");
        List<String> sceneEntryValues = new ArrayList<>();
        sceneEntryValues.add("hdr");

        RelationGroup group = configureHdrRelationHasSpace();
        mDispatcher.dispatch(group.getRelation("on", true));

        Assert.assertEquals("off", flashSetting.getValue());
        Assert.assertEquals("hdr", sceneSetting.getValue());
        Assert.assertEquals(flashEntryValues, flashSetting.getEntryValues());
        Assert.assertEquals(sceneEntryValues, sceneSetting.getEntryValues());
    }

    /**
     * Test the restriction dispatcher can dispatch the relation that its value and entry values
     * are null object to target settings.
     */
    @Test
    public void testDispatchNullRelation() {
        ICameraSetting flashSetting = createTestFlashSetting();
        ICameraSetting sceneSetting = createTestSceneSetting();
        ICameraSetting hdrSetting = createTestHdrSetting();
        mTable.add(flashSetting);
        mTable.add(sceneSetting);
        mTable.add(hdrSetting);

        RelationGroup group = configureHdrNullRelation();
        mDispatcher.dispatch(group.getRelation("on", true));

        Assert.assertEquals(null, flashSetting.getValue());
        Assert.assertEquals(null, sceneSetting.getValue());
        Assert.assertTrue(flashSetting.getEntryValues().size() == 0);
        Assert.assertTrue(sceneSetting.getEntryValues().size() == 0);
    }


    private ICameraSetting createTestFlashSetting() {
        SettingCreator.SettingInfo info = mCreator.new SettingInfo();
        info.key = "key_flash";
        info.settingType = ICameraSetting.SettingType.PHOTO_AND_VIDEO;
        return mCreator.createSetting(info, null, null, null, null, null);
    }

    private ICameraSetting createTestSceneSetting() {
        SettingCreator.SettingInfo info = mCreator.new SettingInfo();
        info.key = "key_scene_mode";
        info.settingType = ICameraSetting.SettingType.PHOTO_AND_VIDEO;
        return mCreator.createSetting(info, null, null, null, null, null);
    }

    private ICameraSetting createTestHdrSetting() {
        SettingCreator.SettingInfo info = mCreator.new SettingInfo();
        info.key = "key_hdr";
        info.settingType = ICameraSetting.SettingType.PHOTO;
        return mCreator.createSetting(info, null, null, null, null, null);
    }

    private RelationGroup configureHdrRelation() {
        RelationGroup group = new RelationGroup();
        group.setHeaderKey("key_hdr");
        group.setBodyKeys("key_flash, key_scene_mode");
        group.addRelation(
                new Relation.Builder("key_hdr", "on")
                    .addBody("key_flash", "off", "on,off,auto")
                    .addBody("key_scene_mode", "hdr", "hdr")
                    .build());
        return group;
    }

    private RelationGroup configureHdrRelationHasSpace() {
        RelationGroup group = new RelationGroup();
        group.setHeaderKey("key_hdr");
        group.setBodyKeys("key_flash, key_scene_mode");
        group.addRelation(
                new Relation.Builder("key_hdr", "on")
                        .addBody("key_flash", "off", "on,    off,     auto    ")
                        .addBody("key_scene_mode", "hdr", "hdr    ")
                        .build());
        return group;
    }

    private RelationGroup configureHdrNullRelation() {
        RelationGroup group = new RelationGroup();
        group.setHeaderKey("key_hdr");
        group.setBodyKeys("key_flash, key_scene_mode");
        group.addRelation(
                new Relation.Builder("key_hdr", "on")
                        .addBody("key_flash", null, null)
                        .addBody("key_scene_mode", null, null)
                        .build());
        return group;
    }
}
