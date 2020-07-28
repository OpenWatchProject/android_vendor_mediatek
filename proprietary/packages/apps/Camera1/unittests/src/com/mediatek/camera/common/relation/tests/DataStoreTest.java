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

import com.mediatek.camera.common.BuildConfig;
import com.mediatek.camera.common.relation.DataStore;

import junit.framework.Assert;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.RuntimeEnvironment;
import org.robolectric.annotation.Config;

import java.util.ArrayList;
import java.util.List;

/**
 * Unit test for {@link com.mediatek.camera.common.relation.DataStore}
 */
@RunWith(RobolectricTestRunner.class)
@Config(constants = BuildConfig.class, sdk = 22)
public class DataStoreTest {
    /**
     * Tet global scope's non cache set & get method.
     *
     * Step1: set value in one data store, assert set successfully.
     * Step2: get value in another data store, and assert it's value assert dataStore1.
     */
    @Test
    public void testNonCacheGlobalScopeValue() {
        DataStore dataStore1 = new DataStore(RuntimeEnvironment.application);
        dataStore1.setValue("test-key","test-value", dataStore1.getGlobalScope(),false);
        Assert.assertEquals("test-value",
                dataStore1.getValue("test-key", "default-value", dataStore1.getGlobalScope()));

        //new another data store, we can get "test-key"'s value
        DataStore dataStore2 = new DataStore(RuntimeEnvironment.application);
        Assert.assertEquals("test-value",
                dataStore2.getValue("test-key", "default-value", dataStore2.getGlobalScope()));
    }

    /**
     * Tet camera scope's non cache set & get method.
     *
     * Step1: set value in one data store, assert set successfully.
     * Step2: get value in another data store, and assert it's value assert dataStore1.
     */
    @Test
    public void testNonCacheCameraScopeValue() {
        DataStore dataStore1 = new DataStore(RuntimeEnvironment.application);
        dataStore1.setValue("test-key","test-value", dataStore1.getCameraScope(0),false);
        Assert.assertEquals("test-value",
                dataStore1.getValue("test-key", "default-value", dataStore1.getCameraScope(0)));

        //new another data store, we can get "test-key"'s value
        DataStore dataStore2 = new DataStore(RuntimeEnvironment.application);
        Assert.assertEquals("test-value",
                dataStore2.getValue("test-key", "default-value", dataStore2.getCameraScope(0)));
    }

    /**
     * Tet global scope's cache set & get method.
     *
     * Step1: set value in one data store, assert set successfully.
     * Step2: get value in another data store, and assert it's value not the same with dataStore1.
     */
    @Test
    public void testCacheGlobalScopeValue() {
        DataStore dataStore1 = new DataStore(RuntimeEnvironment.application);
        dataStore1.setValue("test-key","test-value",dataStore1.getGlobalScope(),true);
        Assert.assertEquals("test-value",
                dataStore1.getValue("test-key", "default-value", dataStore1.getGlobalScope()));

        //new another data store, we can't get "test-key"'s value
        DataStore dataStore2 = new DataStore(RuntimeEnvironment.application);
        Assert.assertFalse("test-value".equals(
                dataStore2.getValue("test-key", "default-value", dataStore2.getGlobalScope())));
    }

    /**
     * Tet global scope's cache set & get method.
     *
     * Step1: set value in one data store, assert set successfully.
     * Step2: get value in another data store, and assert it's value not the same with dataStore1.
     */
    @Test
    public void testCacheCameraScopeValue() {
        DataStore dataStore1 = new DataStore(RuntimeEnvironment.application);
        dataStore1.setValue("test-key","test-value",dataStore1.getCameraScope(0),true);
        Assert.assertEquals("test-value",
                dataStore1.getValue("test-key", "default-value", dataStore1.getCameraScope(0)));

        //new another data store, we can't get "test-key"'s value
        DataStore dataStore2 = new DataStore(RuntimeEnvironment.application);
        Assert.assertFalse("test-value".equals(
                dataStore2.getValue("test-key", "default-value", dataStore2.getCameraScope(0))));
    }

    /**
     * Test if key not exists, will return the specified default value.
     * Test global scope & camera scope.
     */
    @Test
    public void testGetNonExistKeyValue() {
        DataStore dataStore = new DataStore(RuntimeEnvironment.application);
        Assert.assertEquals("not_exit_default_value",
                dataStore.getValue("not_exist_default_key",
                                   "not_exit_default_value",
                                    dataStore.getGlobalScope()));
        Assert.assertEquals("not_exit_default_value",
                dataStore.getValue("not_exist_default_key",
                                   "not_exit_default_value",
                                    dataStore.getCameraScope(0)));
    }

    /**
     * Test if set global scope value, can get from camera scope.
     * Test include cache & non cache case.
     */
    @Test
    public void testGetGlobalKeyFromCameraScope() {
        DataStore dataStore = new DataStore(RuntimeEnvironment.application);
        // non cache case
        dataStore.setValue("test-key-nonCache", "test-value-nonCache",
            dataStore.getGlobalScope(), false);
        Assert.assertEquals("test-value-nonCache",
            dataStore.getValue("test-key-nonCache", "default-value", dataStore.getCameraScope(0)));
        // cache case
        dataStore.setValue("test-key-cache", "test-value-cache", dataStore.getGlobalScope(), true);
        Assert.assertEquals("test-value-cache",
            dataStore.getValue("test-key-cache", "default-value", dataStore.getCameraScope(0)));
    }

    /**
     * Test can get the settings that keeps timestamp of saving values from data store.
     * And the setting that set value in data store more later has more front position in
     * the return list.
     *
     * @throws Exception Throw exception.
     */
    @Test
    public void testKeepSavingTimestamp() throws Exception {
        DataStore dataStore = new DataStore(RuntimeEnvironment.application);

        dataStore.setValue("test3-key-nonCache", "test-value-nonCache",
                dataStore.getGlobalScope(), false, true);
        // Sleep 50ms to make sure the setting keep different timestamp.
        Thread.sleep(50);

        dataStore.setValue("test4-key-nonCache", "test-value-nonCache",
                dataStore.getGlobalScope(), false, true);
        Thread.sleep(50);

        dataStore.setValue("test5-key-nonCache", "test-value-nonCache",
                dataStore.getGlobalScope(), false, true);
        Thread.sleep(50);

        dataStore.setValue("test6-key-nonCache", "test-value-nonCache",
                dataStore.getGlobalScope(), false, true);

        List<String> keys = dataStore.getSettingsKeepSavingTime(0);
        List<String> expectedKeys = new ArrayList<>();
        expectedKeys.add("test6-key-nonCache");
        expectedKeys.add("test5-key-nonCache");
        expectedKeys.add("test4-key-nonCache");
        expectedKeys.add("test3-key-nonCache");
        Assert.assertEquals(expectedKeys, keys);
    }
}