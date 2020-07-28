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

import junit.framework.Assert;

import org.junit.Test;

import java.util.ArrayList;
import java.util.List;

/**
 * Unit test for {@link SettingBase}.
 */
public class SettingBaseTest {
    /**
     * Test Override Value with TT Case, that is currentValue and supportedValues are supported
     * in current platform (currentValue, supportedValues) -> (T, T).
     * We expected this override will affected.
     */
    @Test
    public void testOverrideValueTTCase() {
        SettingBase settingBase = createSettingBase();
        List<String> overrideSupportValues = new ArrayList<>();
        overrideSupportValues.add("off");
        overrideSupportValues.add("on");

        settingBase.overrideValues("test-key",
                "off",
                overrideSupportValues);
        Assert.assertEquals("off", settingBase.getValue());
        Assert.assertEquals(overrideSupportValues, settingBase.getEntryValues());
    }

    /**
     * Test Override Value with TF Case, that is currentValue is supported, but supportedValues
     * include not supported values in current platform (currentValue, supportedValues) -> (T, F).
     * We expected:
     * 1.currentValue is override success.
     * 2.entryValues will be override with supportedValues&platformSupportedValues.
     */
    @Test
    public void testOverrideValueTFCase() {
        SettingBase settingBase = createSettingBase();
        List<String> overrideSupportValues = new ArrayList<>();
        overrideSupportValues.add("off");
        overrideSupportValues.add("on");
        overrideSupportValues.add("torch"); // platform not supported
        List<String> expectedEntryValues = new ArrayList<>();
        expectedEntryValues.add("off");
        expectedEntryValues.add("on");

        settingBase.overrideValues("test-key",
                "off",
                overrideSupportValues);
        Assert.assertEquals("off", settingBase.getValue());
        Assert.assertEquals(expectedEntryValues, settingBase.getEntryValues());
    }
    /**
     * Test Override Value with FF Case, that is currentValue and supportedValues are not supported
     * in current platform (currentValue, supportedValues) -> (F, F).
     * We expected this override do nothing.
     */
    @Test
    public void testOverrideValueFFCase() {
        SettingBase settingBase = createSettingBase();
        List<String> overrideSupportValues = new ArrayList<>();
        overrideSupportValues.add("torch"); // platform not supported

        // Get current value & entryValues
        String expectedValue = settingBase.getValue();
        List<String> expectedEntryValues = settingBase.getEntryValues();

        settingBase.overrideValues("test-key",
                overrideSupportValues.get(0),
                overrideSupportValues);

        // Check value & entryValues not changed.
        Assert.assertEquals(expectedValue, settingBase.getValue());
        Assert.assertEquals(expectedEntryValues, settingBase.getEntryValues());
    }

    /**
     * Test Override Value with FT Case, that is currentValue not supported but supportedValues
     * include supported values in current platform (currentValue, supportedValues) -> (F, T).
     * We expected:
     * 1.entryValues will be override with supportedValues&platformSupportedValues.
     * 2.currentValue is not what we override, but in supportedValues&platformSupportedValues.
     */
    @Test
    public void testOverrideValueFTCase() {
        SettingBase settingBase = createSettingBase();
        List<String> overrideSupportValues = new ArrayList<>();
        overrideSupportValues.add("off");
        overrideSupportValues.add("on");
        overrideSupportValues.add("torch"); // platform not supported
        List<String> expectedEntryValues = new ArrayList<>();
        expectedEntryValues.add("off");
        expectedEntryValues.add("on");

        settingBase.overrideValues("test-key",
                "torch", // platform not supported
                overrideSupportValues);
        Assert.assertEquals(expectedEntryValues, settingBase.getEntryValues());
        Assert.assertTrue(expectedEntryValues.contains(settingBase.getValue()));
    }

    /**
     * Test clear overrides, test steps as follows.
     * 1.override with some value.
     * 2.override with null.
     * 3.check resume to before override.
     */
    @Test
    public void testClearOverrideValues() {
        SettingBase settingBase = createSettingBase();
        List<String> overrideSupportValues = new ArrayList<>();
        overrideSupportValues.add("off");
        overrideSupportValues.add("on");

        settingBase.setValue("on");
        String expectedValue = "on";
        List<String> expectedEntryValues = settingBase.getEntryValues();

        settingBase.overrideValues("test-key",
                "off",
                overrideSupportValues);
        settingBase.overrideValues("test-key", null, null);

        Assert.assertEquals(expectedValue, settingBase.getValue());
        Assert.assertEquals(expectedEntryValues, settingBase.getEntryValues());
    }

    private SettingBase createSettingBase() {
        SettingBase settingBase = new TestFlashSetting();
        List<String> platformSupportValues = new ArrayList<>();
        platformSupportValues.add("on");
        platformSupportValues.add("off");
        platformSupportValues.add("auto");
        List<String> entrySupportValues = new ArrayList<>();
        entrySupportValues.add("on");
        entrySupportValues.add("off");
        entrySupportValues.add("auto");
        entrySupportValues.add("torch");

        settingBase.setSupportedPlatformValues(platformSupportValues);
        settingBase.setSupportedEntryValues(entrySupportValues);
        settingBase.setValue("on");
        List<String> entryValues = new ArrayList<>();
        entryValues.addAll(entrySupportValues);
        entryValues.retainAll(platformSupportValues);
        settingBase.setEntryValues(entryValues);

        return settingBase;
    }

    /**
     * A class extends form {@link SettingBase} used for setting.
     */
    private class TestFlashSetting extends SettingBase {
        @Override
        public void unInit() {

        }

        @Override
        public String getKey() {
            return null;
        }

        @Override
        public SettingType getSettingType() {
            return null;
        }

        @Override
        public void postRestrictionAfterInitialized() {

        }

        @Override
        public IParametersConfigure getParametersConfigure() {
            return null;
        }

        @Override
        public ICaptureRequestConfigure getCaptureRequestConfigure() {
            return null;
        }
    }
}