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
 * MediaTek Inc. (C) 2016. All rights reserved.
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
package com.mediatek.camera.tests;

import android.support.test.uiautomator.UiObject;
import android.support.test.uiautomator.UiObject2;
import android.support.test.uiautomator.UiObjectNotFoundException;
import android.support.test.uiautomator.UiSelector;

import java.lang.reflect.Field;

import javax.annotation.Nonnull;

/**
 * Utils class of common tool.
 *
 */
public class Utils {
    private static final String TAG = Utils.class.getSimpleName();
    private static final int MS_TO_SECOND = 1000;
    private static final int SLEEP_TIME = 500;
    private static final String HAS_THUMBNAIL = "Has Content";
    public static final int TIME_OUT_MS = 300000; // 5min
    public static final String THUMBNAIL_RES_ID = "com.mediatek.camera:id/thumbnail";
    public static final String SWITCH_BUTTON_ID = "com.mediatek.camera:id/camera_switcher";
    public static final String VIDEO_STOP_BUTTON_ID = "com.mediatek.camera:id/video_stop_shutter";
    public static final String MODE_SELECT_BUTTON_ID = "com.mediatek.camera:id/mode";
    public static final String MODE_SETTING_BUTTON_ID = "com.mediatek.camera:id/setting_view";
    public static final String DNG_INDICATOR_ID = "com.mediatek.camera:id/dng_indicator";
    public static final String SHUTTER_LIST_ID = "com.mediatek.camera:id/shutter_root";
    public static final String SHUTTER_CHILD_TEXT = "com.mediatek.camera:id/shutter_text";
    public static final String MODE_LIST_ID = "com.mediatek.camera:id/mode_list";
    public static final String FEATURE_HDR_ICON = "com.mediatek.camera:id/hdr_icon";
    public static final String FEATURE_HDR_OPTION_MENU = "com.mediatek.camera:id/hdr_choice";

    public static final UiSelector SHUTTER_BUTTON_SELECTOR =
            new UiSelector().className("android.widget.ImageView")
                    .resourceIdMatches("com.mediatek.camera:id/shutter_.*");

    /**
     * Check status is ready.
     */
    public interface Checker {
        /**
         * Check status is ready.
         *
         * @return true if status ready.
         */
        public boolean check();
    }

    /**
     * Format the duration information of a test.
     * @param methodName The name of the method.
     * @param durationMs The duration of the test case.
     * @return The information of the case duration.
     */
    public static String formatDuration(String methodName, long durationMs) {
        StringBuilder builder = new StringBuilder();
        builder.append("CaseDuration [").append(methodName).append("] = ")
                .append(durationMs / MS_TO_SECOND).append("s");
        return builder.toString();
    }

    /**
     * Format current class name,after format,the package name will be removed. For example,before
     * format class name : com.mediatek.camera.common.unit.DeviceManagerFactoryTest after format
     * class name DeviceManagerFactoryTest
     *
     * @param name
     *            The class name with package name.
     * @return The class name without package name.
     */
    public static String formatClassName(String name) {
        String className = name.substring(name.lastIndexOf(".") + 1, name.length());
        return className;
    }

    /**
     * Wait status is ready.
     *
     * @param checker
     *            status checker instance.
     * @param timeOut
     *            the time wait status ready.
     * @return true if time out.
     */
    public static boolean waitForTrueWithTimeOut(Checker checker, long timeOut) {
        long startTime = System.currentTimeMillis();
        boolean timedOut = false;
        while (!checker.check()) {
            if (System.currentTimeMillis() - startTime > timeOut) {
                timedOut = true;
                break;
            }
            try {
                Thread.sleep(SLEEP_TIME);
            } catch (InterruptedException e) {
                Log.d(TAG, "<waitForTrueWithTimeOut>" + e);
            }
        }
        return timedOut;
    }

    /**
     * Get the value of specified member in object.
     * @param <O>
     *          The object which extends Object, the input object
     * @param <M>
     *          The object which extends Object, the ouput member
     * @param object
     *          The input object
     * @param memberName
     *          The member name in class
     * @return
     *          The value of specified member
     */
    public static <O extends Object, M extends Object> M
        getMemberInObject(O object, String memberName) {
        for (Class<?> clazz = object.getClass(); clazz != Object.class; clazz = clazz
                .getSuperclass()) {
            try {
                final Field memberField = clazz.getDeclaredField(memberName);
                memberField.setAccessible(true);
                return (M) memberField.get(object);
            } catch (NullPointerException e1) {
                Log.e(TAG, "<getMemberInObject> NullPointerException when fetching");
            } catch (IllegalArgumentException e2) {
                Log.e(TAG, "<getMemberInObject> IllegalArgumentException when fetching");
            } catch (IllegalAccessException e3) {
                Log.e(TAG, "<getMemberInObject> IllegalAccessException when fetching");
            } catch (NoSuchFieldException e4) {
                Log.e(TAG, "<getMemberInObject> NoSuchFieldException when fetching");
            }
        }
        return null;
    }


    /**
     * Set the value to specified member in object.
     *
     * @param object     The input object
     * @param memberName The member name in class
     * @param value      Set this value to member variable
     * @param <O>        The object which extends Object, the input object
     * @param <M>        The object which extends Object, the member
     */
    public static <O extends Object, M extends Object> void
    setMemberInObject(O object, String memberName, M value) {
        for (Class<?> clazz = object.getClass(); clazz != Object.class; clazz = clazz
                .getSuperclass()) {
            try {
                final Field memberField = clazz.getDeclaredField(memberName);
                memberField.setAccessible(true);
                memberField.set(object, value);
            } catch (NullPointerException e1) {
                Log.e(TAG, "<getMemberInObject> NullPointerException when fetching");
            } catch (IllegalArgumentException e2) {
                Log.e(TAG, "<getMemberInObject> IllegalArgumentException when fetching");
            } catch (IllegalAccessException e3) {
                Log.e(TAG, "<getMemberInObject> IllegalAccessException when fetching");
            } catch (NoSuchFieldException e4) {
                Log.e(TAG, "<getMemberInObject> NoSuchFieldException when fetching");
            }
        }
    }

    /**
     * Check whether thumbnail has content.
     * @param thumbnailObject thumbnail UiObject.
     * @return true if has content, false has no content.
     * @throws Exception Throw exception.
     */
    public static boolean isThumbnailHasContent(
            @Nonnull UiObject2 thumbnailObject) {
        String thumbnailContentDes = thumbnailObject.getContentDescription();
        return HAS_THUMBNAIL.equalsIgnoreCase(thumbnailContentDes);
    }
}
