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
package com.mediatek.camera.common.setting;

import android.hardware.Camera;

import com.mediatek.camera.common.SettingCreator;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory;
import com.mediatek.camera.common.device.v1.CameraProxy;

import junit.framework.Assert;

import org.junit.Test;

import java.util.List;

/**
 * The unit test for {@link SettingTable}.
 */
public class SettingTableTest {
    private SettingCreator mCreator = new SettingCreator();

    /**
     * Test add setting and can be queried this setting setting table by its key
     * from setting table.
     */
    @Test
    public void testAddSetting() {
        SettingTable table = new SettingTable();
        ICameraSetting flashSetting = createTestFlashSetting();
        ICameraSetting hdrSetting = createTestHdrSetting();

        table.add(flashSetting);
        table.add(hdrSetting);
        ICameraSetting querySetting = table.get(flashSetting.getKey());
        Assert.assertEquals(querySetting, flashSetting);
    }

    /**
     * Test setting table don't contain this setting after it removed.
     */
    @Test
    public void testRemoveSetting() {
        SettingTable table = new SettingTable();
        ICameraSetting flashSetting = createTestFlashSetting();
        ICameraSetting hdrSetting = createTestHdrSetting();
        ICameraSetting videoQualitySetting = createTestVideoQualitySetting();

        table.add(flashSetting);
        table.add(hdrSetting);
        table.add(videoQualitySetting);

        table.remove(flashSetting);
        table.remove(videoQualitySetting);

        ICameraSetting queryflashSetting = table.get(flashSetting.getKey());
        ICameraSetting queryVideoQualitySetting = table.get(videoQualitySetting.getKey());
        Assert.assertEquals(queryflashSetting, null);
        Assert.assertEquals(queryVideoQualitySetting, null);

        List<ICameraSetting> photoSettings = table
                .getSettingListByType(ICameraSetting.SettingType.PHOTO);
        Assert.assertTrue(!photoSettings.contains(flashSetting));
        List<ICameraSetting> videoSettings = table
                .getSettingListByType(ICameraSetting.SettingType.VIDEO);
        Assert.assertTrue(!videoSettings.contains(videoQualitySetting));
    }

    /**
     * Test setting table return all the settings when get all settings.
     */
    @Test
    public void testGetAllSettings() {
        SettingTable table = new SettingTable();
        ICameraSetting flashSetting = createTestFlashSetting();
        ICameraSetting hdrSetting = createTestHdrSetting();
        ICameraSetting videoQualitySetting = createTestVideoQualitySetting();

        table.add(flashSetting);
        table.add(hdrSetting);
        table.add(videoQualitySetting);

        List<ICameraSetting> allSettings = table.getAllSettings();
        Assert.assertTrue(allSettings.contains(flashSetting));
        Assert.assertTrue(allSettings.contains(hdrSetting));
        Assert.assertTrue(allSettings.contains(videoQualitySetting));

    }

    /**
     * Test setting table has no any setting after removing all the settings.
     */
    @Test
    public void testRemoveAllSettings() {
        SettingTable table = new SettingTable();
        ICameraSetting flashSetting = createTestFlashSetting();
        ICameraSetting hdrSetting = createTestHdrSetting();
        ICameraSetting videoQualitySetting = createTestVideoQualitySetting();

        table.add(flashSetting);
        table.add(hdrSetting);
        table.add(videoQualitySetting);

        table.removeAll();
        List<ICameraSetting> allSettings = table.getAllSettings();
        Assert.assertTrue(allSettings.size() == 0);
    }

    /**
     * Test setting table return all the settings that has the input setting type.
     */
    @Test
    public void testQuerySettingByType() {
        SettingTable table = new SettingTable();
        ICameraSetting flashSetting = createTestFlashSetting();
        ICameraSetting hdrSetting = createTestHdrSetting();
        ICameraSetting videoQualitySetting = createTestVideoQualitySetting();

        table.add(flashSetting);
        table.add(hdrSetting);
        table.add(videoQualitySetting);
        table.classify(CameraDeviceManagerFactory.CameraApi.API1);

        List<ICameraSetting> photoSettings = table
                .getSettingListByType(ICameraSetting.SettingType.PHOTO);
        Assert.assertTrue(photoSettings.contains(flashSetting));
        Assert.assertTrue(photoSettings.contains(hdrSetting));

        List<ICameraSetting> videoSettings = table
                .getSettingListByType(ICameraSetting.SettingType.VIDEO);
        Assert.assertTrue(videoSettings.contains(videoQualitySetting));
        Assert.assertTrue(videoSettings.contains(flashSetting));

        List<ICameraSetting> photoAndVideoSettings = table
                .getSettingListByType(ICameraSetting.SettingType.PHOTO_AND_VIDEO);
        Assert.assertTrue(photoAndVideoSettings.contains(flashSetting));
    }

    /**
     * Test setting table can return a setting that has parameters configure when
     * query a parameters configure settings by setting key.
     */
    @Test
    public void testGetConfigureParametersSetting() {
        SettingTable table = new SettingTable();

        /*ICameraSetting flashSetting = createTestFlashSetting();
        table.add(flashSetting);
        table.classify();
        ICameraSetting parametersSetting = table
                .getConfigParameterSetting(flashSetting.getKey());
        Assert.assertNull(parametersSetting);
        table.removeAll();

        ICameraSetting flashSettingWithParamConfig = createTestFlashSettingWithParamConfig();
        table.add(flashSettingWithParamConfig);
        table.classify();
        parametersSetting = table
                .getConfigParameterSetting(flashSetting.getKey());
        Assert.assertEquals(parametersSetting, flashSettingWithParamConfig);*/
    }

    /**
     * Test setting table return all the setting that has parameters configure when
     * querying all parameters configure settings.
     */
    @Test
    public void testGetAllConfigureParametersSettings() {
        SettingTable table = new SettingTable();

        ICameraSetting flashSetting = createTestFlashSetting();
        ICameraSetting hdrSetting = createTestHdrSetting();
        table.add(flashSetting);
        table.add(hdrSetting);
        table.classify(CameraDeviceManagerFactory.CameraApi.API1);
        List<ICameraSetting> parametersSettings = table.getAllConfigParametersSettings();
        Assert.assertTrue(parametersSettings.size() == 0);
        table.removeAll();

        ICameraSetting flashSettingWithParamConfig = createTestFlashSettingWithParamConfig();
        ICameraSetting hdrSettingWithParamConfig = createTestHdrSettingWithParamConfig();
        table.add(flashSettingWithParamConfig);
        table.add(hdrSettingWithParamConfig);
        table.classify(CameraDeviceManagerFactory.CameraApi.API1);
        parametersSettings = table.getAllConfigParametersSettings();
        Assert.assertTrue(parametersSettings.contains(flashSettingWithParamConfig));
        Assert.assertTrue(parametersSettings.contains(hdrSettingWithParamConfig));
    }

    /**
     * Test setting table return all the settings implements {@link com.mediatek.camera.common
     * .setting.ICameraSetting.PreviewStateCallback} when querying.
     */
    @Test
    public void testGetAllPreviewStateSettings() {
        SettingTable table = new SettingTable();

        ICameraSetting flashSetting = createTestFlashSetting();
        ICameraSetting hdrSetting = createTestHdrSetting();
        table.add(flashSetting);
        table.add(hdrSetting);
        table.classify(CameraDeviceManagerFactory.CameraApi.API1);
        List<ICameraSetting> stateSettings = table.getAllPreviewStatusSettings();
        Assert.assertTrue(stateSettings.size() == 0);
        table.removeAll();

        ICameraSetting flashSettingWithPreviewState = createTestFlashSettingWithPreviewState();
        ICameraSetting hdrSettingWithPreviewState = createTestHdrSettingWithPreviewState();
        table.add(flashSettingWithPreviewState);
        table.add(hdrSettingWithPreviewState);
        table.classify(CameraDeviceManagerFactory.CameraApi.API1);
        stateSettings = table.getAllPreviewStatusSettings();
        Assert.assertTrue(stateSettings.contains(flashSettingWithPreviewState));
        Assert.assertTrue(stateSettings.contains(hdrSettingWithPreviewState));

    }

    private ICameraSetting createTestFlashSetting() {
        SettingCreator.SettingInfo info = mCreator.new SettingInfo();
        info.key = "key_flash";
        info.settingType = ICameraSetting.SettingType.PHOTO_AND_VIDEO;
        return mCreator.createSetting(info, null, null, null, null, null);
    }

    private ICameraSetting createTestHdrSetting() {
        SettingCreator.SettingInfo info = mCreator.new SettingInfo();
        info.key = "key_hdr";
        info.settingType = ICameraSetting.SettingType.PHOTO;
        return mCreator.createSetting(info, null, null, null, null, null);
    }

    private ICameraSetting createTestVideoQualitySetting() {
        SettingCreator.SettingInfo info = mCreator.new SettingInfo();
        info.key = "key_video_quality";
        info.settingType = ICameraSetting.SettingType.VIDEO;
        return mCreator.createSetting(info, null, null, null, null, null);
    }

    private ICameraSetting createTestFlashSettingWithParamConfig() {
        SettingCreator.SettingInfo info = mCreator.new SettingInfo();
        info.key = "key_flash";
        info.settingType = ICameraSetting.SettingType.PHOTO_AND_VIDEO;
        return mCreator.createSetting(info, null, null, null,
                new MyParametersConfig(), null);
    }

    private ICameraSetting createTestHdrSettingWithParamConfig() {
        SettingCreator.SettingInfo info = mCreator.new SettingInfo();
        info.key = "key_hdr";
        info.settingType = ICameraSetting.SettingType.PHOTO;
        return mCreator.createSetting(info, null, null, null,
                new MyParametersConfig(), null);
    }

    private ICameraSetting createTestFlashSettingWithPreviewState() {
        SettingCreator.SettingInfo info = mCreator.new SettingInfo();
        info.key = "key_flash";
        info.settingType = ICameraSetting.SettingType.PHOTO_AND_VIDEO;
        return mCreator.createSetting(info, null, null, null,
                null, new MyPreviewStateCallback());
    }

    private ICameraSetting createTestHdrSettingWithPreviewState() {
        SettingCreator.SettingInfo info = mCreator.new SettingInfo();
        info.key = "key_hdr";
        info.settingType = ICameraSetting.SettingType.PHOTO;
        return mCreator.createSetting(info, null, null, null,
                null, new MyPreviewStateCallback());
    }

    /**
     * Parameters configure implementer.
     */
    private class MyParametersConfig implements ICameraSetting.IParametersConfigure {

        @Override
        public void setOriginalParameters(Camera.Parameters originalParameters) {

        }

        @Override
        public boolean configParameters(Camera.Parameters parameters) {
            return false;
        }

        @Override
        public void configCommand(CameraProxy cameraProxy) {

        }

        @Override
        public void sendSettingChangeRequest() {

        }
    }

    /**
     * Preview state callback implementer.
     */
    private class MyPreviewStateCallback implements ICameraSetting.PreviewStateCallback {

        @Override
        public void onPreviewStopped() {

        }

        @Override
        public void onPreviewStarted() {

        }
    }
}
