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
 *     MediaTek Inc. (C) 2016. All rights reserved.
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

package com.mediatek.camera.tests.common.utils;

import android.graphics.Point;
import android.graphics.Rect;

import com.mediatek.camera.common.utils.CoordinatesTransform;
import com.mediatek.camera.tests.CameraUnitTestCaseBase;
import com.mediatek.camera.tests.Log;

import junit.framework.Assert;

import java.util.ArrayList;
import java.util.List;

/**
 * Ths is for CoordinatesTransform test cases.
 */

public class CoordinatesTransformTest extends CameraUnitTestCaseBase {
    private static final String TAG = CoordinatesTransformTest.class.getSimpleName();

    /**
     * testUiToNormalizedPreviewBackCamera(): check ui coordinate transform to (-1000,1000)
     * coordinate for back camera. {@link CoordinatesTransform.uiToNormalizedPreview()}.
     * full screen case.
     */
    public void testUiToNormalizedPreviewBackCamera() {

        //portrait phone
        int w = 1080;
        int h = 1920;
        int orientation = 90;
        int centerx = w / 2;
        int centerY = h / 2;
        Rect previewArea = new Rect(0, 0, w, h);
        List<Rect> tapRectList = new ArrayList<Rect>();
        List<Rect> expectRectList = new ArrayList<Rect>();

        //left-top
        tapRectList.add(new Rect(234, 328, 360, 455));
        expectRectList.add(new Rect(-658, 333, -526, 566));

        //left-bottom
        tapRectList.add(new Rect(313, 1209, 433, 1329));
        expectRectList.add(new Rect(259, 198, 384, 420));

        //right-top
        tapRectList.add(new Rect(755, 445, 917, 607));
        expectRectList.add(new Rect(-536, -698, -368, -398));

        //right-bottom
        tapRectList.add(new Rect(874, 1403, 994, 1523));
        expectRectList.add(new Rect(461, -841, 586, -619));

        //center
        tapRectList.add(new Rect(centerx - 120, centerY - 120, centerx + 120, centerY + 120));
        expectRectList.add(new Rect(-125, -222, 125, 222));

        //boundary
        tapRectList.add(new Rect(0, 0, 120, 120));
        expectRectList.add(new Rect(-1000, 778, -875, 1000));

        assertUiToNormalizedPreview(tapRectList, expectRectList, previewArea,
                false, 90, "testUiToNormalizedPreviewBackCamera");
    }

    /**
     * testUiToNormalizedPreviewBackCamera(): check ui coordinate transform to (-1000,1000)
     * coordinate for back camera. {@link CoordinatesTransform.uiToNormalizedPreview()}.
     * 4:3 case
     */
    public void testUiToNormalizedPreviewBackCamera43() {
        //portrait phone
        int w = 1080;
        int h = 1440;
        int orientation = 90;
        int centerx = w / 2;
        int centerY = h / 2;
        Rect previewArea = new Rect(0, 0, w, h);
        List<Rect> tapRectList = new ArrayList<Rect>();
        List<Rect> expectRectList = new ArrayList<Rect>();

        //left-top
        tapRectList.add(new Rect(196, 223, 340, 367));
        expectRectList.add(new Rect(-690, 369, -490, 636));

        //right-top
        tapRectList.add(new Rect(773, 216, 916, 360));
        expectRectList.add(new Rect(-699, -698, -499, -432));

        //left-bottom
        tapRectList.add(new Rect(282, 1144, 484, 1346));
        expectRectList.add(new Rect(589, 102, 870, 477));

        //right-bottom
        tapRectList.add(new Rect(799, 1050, 911, 1162));
        expectRectList.add(new Rect(459, -688, 615, -480));

        assertUiToNormalizedPreview(tapRectList, expectRectList, previewArea,
                false, 90, "testUiToNormalizedPreviewBackCamera43");
    }

    /**
     * testUiToNormalizedPreviewFrontCamera(): check ui coordinate transform to (-1000,1000)
     * coordinate for front camera. {@link CoordinatesTransform.uiToNormalizedPreview()}.
     * full screen case.
     */
    public void testUiToNormalizedPreviewFrontCamera() {
        //portrait phone
        int w = 1080;
        int h = 1920;
        int orientation = 90;
        int centerx = w / 2;
        int centerY = h / 2;
        Rect previewArea = new Rect(0, 0, w, h);
        List<Rect> tapRectList = new ArrayList<Rect>();
        List<Rect> expectRectList = new ArrayList<Rect>();

        //left-top
        tapRectList.add(new Rect(56, 438, 494, 876));
        expectRectList.add(new Rect(87, 84, 543, 895));

        //right-top
        tapRectList.add(new Rect(618, 396, 1056, 834));
        expectRectList.add(new Rect(131, -956, 587, -145));

        //left-bottom
        tapRectList.add(new Rect(98, 1173, 458, 1533));
        expectRectList.add(new Rect(-597, 151, -222, 818));

        //right-bottom
        tapRectList.add(new Rect(628, 1215, 1042, 1629));
        expectRectList.add(new Rect(-697, -931, -266, -164));

        assertUiToNormalizedPreview(tapRectList, expectRectList, previewArea,
                true, 90, "testUiToNormalizedPreviewFrontCamera");
    }

    /**
     * testUiToNormalizedPreviewFrontCamera(): check ui coordinate transform to (-1000,1000)
     * coordinate for front camera. {@link CoordinatesTransform.uiToNormalizedPreview()}.
     * 4:3 case
     */
    public void testUiToNormalizedPreviewFrontCamera43() {
        //portrait phone
        int w = 1080;
        int h = 1440;
        int orientation = 90;
        int centerx = w / 2;
        int centerY = h / 2;
        Rect previewArea = new Rect(0, 0, w, h);
        List<Rect> tapRectList = new ArrayList<Rect>();
        List<Rect> expectRectList = new ArrayList<Rect>();

        //left-top
        tapRectList.add(new Rect(219, 293, 354, 428));
        expectRectList.add(new Rect(405, 343, 592, 593));

        //right-top
        tapRectList.add(new Rect(792, 297, 941, 445));
        expectRectList.add(new Rect(381, -743, 587, -468));

        //left-bottom
        tapRectList.add(new Rect(187, 1029, 340, 1182));
        expectRectList.add(new Rect(-643, 369, -430, 653));

        //right-bottom
        tapRectList.add(new Rect(706, 1131, 845, 1270));
        expectRectList.add(new Rect(-765, -566, -572, -308));

        assertUiToNormalizedPreview(tapRectList, expectRectList, previewArea,
                true, 90, "testUiToNormalizedPreviewFrontCamera43");
    }

    /**
     * testNormalizedPreviewToUi(): check (-1000,1000) coordinate to ui coordinate transform
     * for back camera. {@link CoordinatesTransform.normalizedPreviewToUi()}.
     */
    public void testNormalizedPreviewToUi() {
        //portrait phone
        int w = 1080;
        int h = 1920;
        int orientation = 90;
        int centerx = w / 2;
        int centerY = h / 2;
        List<Rect> dirverRectList = new ArrayList<Rect>();
        List<Rect> expectRectList = new ArrayList<Rect>();

        //left-top
        expectRectList.add(new Rect(234, 328, 360, 455));
        dirverRectList.add(new Rect(-658, 333, -526, 566));

        //left-bottom
        expectRectList.add(new Rect(313, 1209, 433, 1329));
        dirverRectList.add(new Rect(259, 198, 384, 420));

        //right-top
        expectRectList.add(new Rect(755, 445, 917, 607));
        dirverRectList.add(new Rect(-536, -698, -368, -398));

        //right-bottom
        expectRectList.add(new Rect(874, 1403, 994, 1523));
        dirverRectList.add(new Rect(461, -841, 586, -619));

        //center
        expectRectList.add(new Rect(centerx - 120, centerY - 120, centerx + 120, centerY + 120));
        dirverRectList.add(new Rect(-125, -222, 125, 222));

        //boundary
        expectRectList.add(new Rect(0, 0, 120, 120));
        dirverRectList.add(new Rect(-1000, 778, -875, 1000));

        for (int i = 0; i < dirverRectList.size(); i++) {
            Rect driveRect = dirverRectList.get(i);
            Rect expectRect = expectRectList.get(i);
            Log.d(TAG, "[testNormalizedPreviewToUi], i = " + i);

            Rect resultRect = CoordinatesTransform.normalizedPreviewToUi(
                    driveRect, w, h, orientation, false);
            int deltaL = Math.abs(expectRect.left - resultRect.left);
            int deltaT = Math.abs(expectRect.top - resultRect.top);
            int deltaR = Math.abs(expectRect.right - resultRect.right);
            int deltaB = Math.abs(expectRect.bottom - resultRect.bottom);

            Assert.assertTrue(deltaL <= 2);
            Assert.assertTrue(deltaT <= 2);
            Assert.assertTrue(deltaR <= 2);
            Assert.assertTrue(deltaB <= 2);
        }
    }

    private void assertUiToNormalizedPreview(List<Rect> tapRectList, List<Rect> expectRectList,
                                             Rect previewArea, boolean isMirror,
                                             int orientation, String msg) {
        for (int i = 0; i < tapRectList.size(); i++) {
            Rect tapRect = tapRectList.get(i);
            Rect expectRect = expectRectList.get(i);
            Log.d(TAG, msg + ", i = " + i);
            int rectLength = tapRect.right - tapRect.left;
            int w = Math.min(previewArea.width(), previewArea.height());
            float ratio = (float) (rectLength) / w;
            int px = tapRect.left + rectLength / 2;
            int py = tapRect.top + rectLength / 2;
            Point p = new Point(px, py);
            Rect resultRect = new Rect();
            try {
                resultRect = CoordinatesTransform.uiToNormalizedPreview(
                        p, previewArea, ratio, isMirror, orientation);
            } catch (IllegalArgumentException e) {
                e.printStackTrace();
                Log.e(TAG, "assertUiToNormalizedPreview IllegalArgumentException ");
            }
            int deltaL = Math.abs(expectRect.left - resultRect.left);
            int deltaT = Math.abs(expectRect.top - resultRect.top);
            int deltaR = Math.abs(expectRect.right - resultRect.right);
            int deltaB = Math.abs(expectRect.bottom - resultRect.bottom);
            Assert.assertTrue(deltaL <= 2);
            Assert.assertTrue(deltaT <= 2);
            Assert.assertTrue(deltaR <= 2);
            Assert.assertTrue(deltaB <= 2);
        }
    }
}
