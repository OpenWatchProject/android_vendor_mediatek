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
 * MediaTek Inc. (C) 2017. All rights reserved.
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
package com.mediatek.camera.tests.v3.operator;

import android.graphics.Point;

import com.mediatek.camera.tests.v3.arch.Operator;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.arch.TestContext;
import com.mediatek.camera.tests.v3.util.Utils;

/**
 * Operator used to touch focus on the screen,36 points will be clicked when cross to use the
 * operator.
 */
public class TouchFocusOperator extends Operator {
    private static final int COLUMN = 6;
    private static final int ROW = 6;
    protected static Point[] sPointList = new Point[COLUMN * ROW];

    /**
     * Index of back camera.
     */
    public static final int INDEX_BACK = 0;
    /**
     * Index of front camera.
     */
    public static final int INDEX_FRONT = 1;
    /**
     * Tag which used to check whether touch focus is supported.
     */
    public static final String[] OPTIONS_TAG = {
            "com.mediatek.camera.at.touch-focus.back",
            "com.mediatek.camera.at.touch-focus.front"};

    static {
        int stepSizeX = Utils.getUiDevice().getDisplayWidth() / COLUMN;
        int stepSizeY = Utils.getUiDevice().getDisplayHeight() * 2 / 3 / ROW;
        int startX = stepSizeX / 2;
        int startY = Utils.getUiDevice().getDisplayHeight() / 12 + stepSizeY / 2;

        for (int column = 0; column < COLUMN; column++) {
            for (int row = 0; row < ROW; row++) {
                sPointList[column * ROW + row] =
                        new Point(startX + column * stepSizeX, startY + row * stepSizeY);
            }
        }
    }

    @Override
    public int getOperatorCount() {
        return sPointList.length;
    }

    @Override
    public Page getPageBeforeOperate(int index) {
        return Page.PREVIEW;
    }

    @Override
    public Page getPageAfterOperate(int index) {
        return Page.PREVIEW;
    }

    @Override
    public String getDescription(int index) {
        return "Touch auto focus at point " + index +
                " (" + sPointList[index].x + ", " + sPointList[index].y + ")";
    }

    @Override
    protected void doOperate(int index) {
        Utils.getUiDevice().click(sPointList[index].x, sPointList[index].y);
        Utils.waitSafely(Utils.TIME_OUT_SHORT_SHORT);
    }

    @Override
    public boolean isSupported(int index) {
        if (TestContext.mLatestCameraFacing == INDEX_BACK) {
            return Utils.isFeatureSupported(OPTIONS_TAG[0]);
        } else if (TestContext.mLatestCameraFacing == INDEX_FRONT) {
            return Utils.isFeatureSupported(OPTIONS_TAG[1]);
        } else {
            return false;
        }
    }
}
