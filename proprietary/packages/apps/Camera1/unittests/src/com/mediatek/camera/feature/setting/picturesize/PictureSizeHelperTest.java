/*
 * Copyright Statement:
 *
 *   This software/firmware and related documentation ("MediaTek Software") are
 *   protected under relevant copyright laws. The information contained herein is
 *   confidential and proprietary to MediaTek Inc. and/or its licensor. Without
 *   the prior written permission of MediaTek inc. and/or its licensor, any
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
 *   NON-INFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
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
package com.mediatek.camera.feature.setting.picturesize;

import junit.framework.Assert;

import org.junit.Test;

import java.util.ArrayList;
import java.util.List;

/**
 * Picture size helper test.
 */

public class PictureSizeHelperTest {

    /**
     * Get get pixels and ratio.
     */
    @Test
    public void testGetPixelsAndRatio() {
        List<Double> desiredAspectRatios = new ArrayList<>();
        desiredAspectRatios.add(16d / 9);
        desiredAspectRatios.add(4d / 3);
        PictureSizeHelper.setDesiredAspectRatios(desiredAspectRatios);

        String value1 = "1920x1080";
        String expectedResult1 = "2M(16:9)";
        Assert.assertEquals(expectedResult1, PictureSizeHelper.getPixelsAndRatio(value1));

        String value2 = "3600x2160"; // 8M(5:3)
        Assert.assertNull(PictureSizeHelper.getPixelsAndRatio(value2));

        String value3 = "1280x960";
        String expectedResult3 = "1M(4:3)";
        Assert.assertEquals(expectedResult3, PictureSizeHelper.getPixelsAndRatio(value3));

        String value4 = "1024x768"; // low than 1 000 000 but high than 500 000
        String expectedResult4 = "1M(4:3)";
        Assert.assertEquals(expectedResult4, PictureSizeHelper.getPixelsAndRatio(value4));

        String value5 = "640x480"; // low then 500 000
        String expectedResult5 = "VGA";
        Assert.assertEquals(expectedResult5, PictureSizeHelper.getPixelsAndRatio(value5));
    }

    /**
     * Test filter rule.
     */
    @Test
    public void testFilterRule() {
        List<Double> desiredAspectRatios = new ArrayList<>();
        desiredAspectRatios.add(16d / 9);
        desiredAspectRatios.add(4d / 3);
        PictureSizeHelper.setDesiredAspectRatios(desiredAspectRatios);

        List<String> testPictureSizes1 = new ArrayList<>();
        testPictureSizes1.add("5120x2880"); // (16:9, 14.74)
        testPictureSizes1.add("2560x1920"); // (4:3, 4.91)
        testPictureSizes1.add("2560x1440"); // (16:9 3.68)
        testPictureSizes1.add("2048x1536"); // (4:3 3.14)
        testPictureSizes1.add("1920x1088"); // (16:9 2.08)
        testPictureSizes1.add("1600x1200"); // (43: 1.92)
        testPictureSizes1.add("1280x960"); // (4:3 1.22)
        testPictureSizes1.add("1280x720"); // (16:9 0.92)
        testPictureSizes1.add("1024x768"); // (4:3 1.78)
        testPictureSizes1.add("640x480"); // (4:3 0.30)
        testPictureSizes1.add("320x240"); // (4:3 0.07)

        PictureSizeHelper.setFilterParameters(0.5, 3);
        List<String> pictureSizes1 = PictureSizeHelper.filterSizes(testPictureSizes1);

        List<String> expectPictureSizes1 = new ArrayList<>();
        expectPictureSizes1.add("5120x2880");
        expectPictureSizes1.add("2560x1920");
        expectPictureSizes1.add("2560x1440");
        expectPictureSizes1.add("1920x1088");
        expectPictureSizes1.add("1600x1200");
        expectPictureSizes1.add("1280x960");
        Assert.assertEquals(expectPictureSizes1, pictureSizes1);


        List<String> testPictureSizes2 = new ArrayList<>();
        testPictureSizes2.add("5120x2880"); // (16:9, 14.74)
        testPictureSizes2.add("2560x1920"); // (4:3, 4.91)
        testPictureSizes2.add("1920x1088"); // (16:9 2.08)
        testPictureSizes2.add("1600x1200"); // (4:3 1.92)
        testPictureSizes2.add("1280x720"); // (16:9 0.92)
        testPictureSizes2.add("640x480"); // (4:3 0.30)
        testPictureSizes2.add("320x240"); // (4:3 0.07)
        List<String> pictureSizes2 = PictureSizeHelper.filterSizes(testPictureSizes2);

        List<String> expectPictureSizes2 = new ArrayList<>();
        expectPictureSizes2.add("5120x2880");
        expectPictureSizes2.add("2560x1920");
        expectPictureSizes2.add("1920x1088");
        expectPictureSizes2.add("1600x1200");
        expectPictureSizes2.add("1280x720");

        expectPictureSizes2.add("640x480");
        Assert.assertEquals(expectPictureSizes2, pictureSizes2);
    }
}
