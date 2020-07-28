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

import android.content.Intent;
import android.os.Build;
import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.Operator;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

/**
 * Operator used to entry and exit multi-window mode.
 */
public class MultiWindowModeOperator extends Operator {
    private static final LogUtil.Tag TAG = Utils.getTestTag(MultiWindowModeOperator.class
            .getSimpleName());
    private static final String MMS_PACKAGE = "com.android.mms";
    public static final String CAPTURE_PICTURE = "Capture picture";
    public static final String CAPTURE_VIDEO = "Capture video";
    private String mCaptureType = CAPTURE_PICTURE;

    /**
     * Entry multi-window mode.
     */
    public static final int INDEX_IN = 0;
    /**
     * Exit multi-window mode.
     */
    public static final int INDEX_OUT = 1;

    /**
     * Initialize capture type to enter camera by mms.
     *
     * @param captureType Capture picture or capture video in mms.
     * @return The multi-window operator with special capture type.
     */
    public MultiWindowModeOperator initEnv(String captureType) {
        mCaptureType = captureType;
        return this;
    }

    @Override
    public void operate(int index) {
        super.operate(index);
    }

    @Override
    public boolean isSupported(int index) {
        return Build.VERSION.SDK_INT > Build.VERSION_CODES.M;
    }

    @Override
    public int getOperatorCount() {
        return 2;
    }

    @Override
    public Page getPageBeforeOperate(int index) {
        return null;
    }

    @Override
    public Page getPageAfterOperate(int index) {
        return null;
    }

    @Override
    public String getDescription(int index) {
        switch (index) {
            case INDEX_IN:
                return "Press to go to multi window mode to " + mCaptureType;
            case INDEX_OUT:
                return "Press to exit multi window mode";
        }
        return null;
    }

    @Override
    protected void doOperate(int index) {
        switch (index) {
            case INDEX_IN:
                //start mms
                Intent intent = null;
                intent = Utils.getContext().getPackageManager()
                        .getLaunchIntentForPackage(MMS_PACKAGE);
                intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK);
                Utils.getContext().startActivity(intent);
                //click to new message
                UiObject2 newMessage = Utils.findObject(
                        By.res("com.android.mms:id/action_compose_new").clickable(true));
                Utils.assertRightNow(newMessage != null);
                newMessage.click();
                //click to add attach
                UiObject2 attach = Utils.findObject(
                        By.desc("Attach").clickable(true));
                Utils.assertRightNow(attach != null);
                attach.click();
                //click to open camera to capture a video or capture a picture
                UiObject2 captureVideo = Utils.findObject(
                        By.text(mCaptureType));
                Utils.assertRightNow(captureVideo != null);
                captureVideo.click();
                Utils.waitSafely(Utils.TIME_OUT_SHORT_SHORT);
                //change to multi-window mode
                Utils.activeMultiWindowMode();
                break;
            case INDEX_OUT:
                //change to leave multi-window mode
                Utils.activeMultiWindowMode();
                break;

        }
    }

}