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
package com.mediatek.camera.common.mode;

import com.mediatek.camera.common.device.CameraDeviceManagerFactory.CameraApi;

import junit.framework.Assert;

import org.junit.Test;

import java.util.ArrayList;

/**
 * Unit test for {@link com.mediatek.camera.common.mode.DeviceUsage}.
 */
public class DeviceUsageTest {

    /**
     * Test getNeedClosedCameraIds method, test no need restart camera.
     */
    @Test
    public void testNoNeedClosedCameraIds() {
        ArrayList<String> openedCameraIdList = new ArrayList<>();
        openedCameraIdList.add("0");
        openedCameraIdList.add("1");
        openedCameraIdList.add("2");
        DeviceUsage curDeviceUsage =
                new DeviceUsage("test-device-type", CameraApi.API1, openedCameraIdList);

        // no need restart camera
        DeviceUsage newDeviceUsage =
                new DeviceUsage("test-device-type", CameraApi.API1, openedCameraIdList);
        ArrayList<String> needClosedCameraIds =
                curDeviceUsage.getNeedClosedCameraIds(newDeviceUsage);
        Assert.assertEquals(0, needClosedCameraIds.size());
    }

    /**
     * Test getNeedClosedCameraIds method, test device type is different, restart all.
     */
    @Test
    public void testNeedClosedCameraIdsWithDiffDeviceType() {
        ArrayList<String> openedCameraIdList = new ArrayList<>();
        openedCameraIdList.add("0");
        openedCameraIdList.add("1");
        openedCameraIdList.add("2");
        DeviceUsage curDeviceUsage =
                new DeviceUsage("test-device-type", CameraApi.API1, openedCameraIdList);
        // device type is different, restart all
        DeviceUsage newDeviceUsage =
                new DeviceUsage("test-device-type-2", CameraApi.API1, openedCameraIdList);
        ArrayList<String> needClosedCameraIds =
                curDeviceUsage.getNeedClosedCameraIds(newDeviceUsage);
        Assert.assertEquals(3, needClosedCameraIds.size());
    }

    /**
     * Test getNeedClosedCameraIds method, test api type is different, restart all.
     */
    @Test
    public void testNeedClosedCameraIdsWithDiffApiType() {
        ArrayList<String> openedCameraIdList = new ArrayList<>();
        openedCameraIdList.add("0");
        openedCameraIdList.add("1");
        openedCameraIdList.add("2");
        DeviceUsage curDeviceUsage =
                new DeviceUsage("test-device-type", CameraApi.API1, openedCameraIdList);
        // api type is different, restart all
        DeviceUsage newDeviceUsage =
                new DeviceUsage("test-device-type", CameraApi.API2, openedCameraIdList);
        ArrayList<String> needClosedCameraIds =
                curDeviceUsage.getNeedClosedCameraIds(newDeviceUsage);
        Assert.assertEquals(3, needClosedCameraIds.size());
    }

    /**
     * Test getNeedClosedCameraIds method, test api type and device type is the same,
     * but camera id list is not the same.
     */
    @Test
    public void testNeedClosedCameraIdsWithDiffOpenedCameraIds() {
        ArrayList<String> openedCameraIdList = new ArrayList<>();
        openedCameraIdList.add("0");
        openedCameraIdList.add("1");
        openedCameraIdList.add("2");
        DeviceUsage curDeviceUsage =
                new DeviceUsage("test-device-type", CameraApi.API1, openedCameraIdList);
        // api type and device type is the same, but camera id list is not the same
        ArrayList<String> openedCameraIdList2 = new ArrayList<>();
        openedCameraIdList2.add("0");
        openedCameraIdList2.add("1");
        DeviceUsage newDeviceUsage =
                new DeviceUsage("test-device-type", CameraApi.API1, openedCameraIdList2);
        ArrayList<String> needClosedCameraIds =
                curDeviceUsage.getNeedClosedCameraIds(newDeviceUsage);
        Assert.assertEquals(3, needClosedCameraIds.size());
    }
}