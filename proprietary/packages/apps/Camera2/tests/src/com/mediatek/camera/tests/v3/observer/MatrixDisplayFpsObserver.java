/*
 * Copyright Statement:
 *
 *   This software/firmware and related documentation ("MediaTek Software") are
 *   protected under relevant copyright laws. The information contained herein is
 *   confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *   the prior written permission of MediaTek inc. and/or its licensors, any
 *   reproduction, modification, use or disclosure of MediaTek Software, and
 *   information contained herein, in whole or in part, shall be strictly
 *   prohibited.
 *
 *   MediaTek Inc. (C) 2017. All rights reserved.
 *
 *   BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *   THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *   RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *   ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *   WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *   WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *   NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
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

package com.mediatek.camera.tests.v3.observer;

import com.mediatek.camera.tests.v3.util.Utils;

/**
 * Matrix display fps observer to check fps is meeting the spec.
 */
public class MatrixDisplayFpsObserver extends LogKeyValueObserver {
    private static final String LOG_TAG = "Lomo_J_151022_09";
    private static final String FPS_LOG_PREFIX = "displayFps = ";
    private static final int FPS_NUM_LENGTH_MAX = "29".length();
    private static final int FPS_NUM_LENGTH_MIN = "2".length();

    private int mFpsPassTarget = 20;
    private boolean mPass = false;

    public MatrixDisplayFpsObserver() {
    }

    public MatrixDisplayFpsObserver(int fpsPassTarget) {
        mFpsPassTarget = fpsPassTarget;
    }

    @Override
    protected void onObserveBegin(int index) {
        mPass = false;
    }

    @Override
    protected String getObservedTag(int index) {
        return LOG_TAG;
    }

    @Override
    protected String getObservedKey(int index) {
        return FPS_LOG_PREFIX;
    }

    @Override
    protected ValueType getValueType(int index) {
        return ValueType.INTEGER;
    }

    @Override
    protected int getMinValueStringLength(int index) {
        return FPS_NUM_LENGTH_MIN;
    }

    @Override
    protected int getMaxValueStringLength(int index) {
        return FPS_NUM_LENGTH_MAX;
    }

    @Override
    public int getObserveCount() {
        return 1;
    }

    @Override
    public String getDescription(int index) {
        return "Observe matrix display preview fps >= " + mFpsPassTarget;
    }

    @Override
    protected void onObserveEnd(int index) {
        Utils.assertRightNow(mPass);
    }

    @Override
    protected boolean isAlreadyFindTarget(int index) {
        return true;
    }

    @Override
    protected void onValueComing(int index, Object value) {
        // It thinks it's passed if mPass is true once.
        if (mPass) {
            return;
        } else {
            mPass = (Integer) (value) >= mFpsPassTarget;
        }
    }

    @Override
    public boolean isSupported(int index) {
        return Utils.isFeatureSupported("com.mediatek.camera.at.matrix-display");
    }
}
