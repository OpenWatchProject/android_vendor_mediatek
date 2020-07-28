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
 * MediaTek Inc. (C) 2014. All rights reserved.
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

package com.mediatek.camera.v2.platform.module;

import android.app.Activity;

/**
 *  The controller at module level.
 */
public interface ModuleController {


    /********************** Life cycle management **********************/

    /**
     * Initializes the module.
     *
     * @param activity The camera activity.
     * @param isSecureCamera Whether the app is in secure camera mode.
     * @param isCaptureIntent Whether the app is in capture intent mode.
     */
    public void open(Activity activity, boolean isSecureCamera, boolean isCaptureIntent);

    /**
     * Resumes the module. Always call this method whenever it's being put in
     * the foreground.
     */
    public void resume();

    /**
     * Pauses the module. Always call this method whenever it's being put in the
     * background.
     */
    public void pause();

    /**
     * Destroys the module. Always call this method to release the resources used
     * by this module.
     */
    public void close();


    /********************** UI management *******************************/

    /**
     * Called when the preview becomes visible/invisible.
     *
     * @param visible Whether the preview is visible, one of
     *            {@link #VISIBILITY_VISIBLE}, {@link #VISIBILITY_COVERED},
     *            {@link #VISIBILITY_HIDDEN}
     */
    public void onPreviewVisibilityChanged(int visibility);

    /**
     * Called when the framework layout orientation changed.
     *
     * @param isLandscape Whether the new orientation is landscape or portrait.
     */
    public void onLayoutOrientationChanged(boolean isLandscape);

    /**
     * Called when the UI orientation is changed.
     *
     * @param orientation The new orientation, valid values are 0, 90, 180 and 270.
     */
    public void onOrientationChanged(int orientation);

    /**
     * Called when back key is pressed.
     *
     * @return Whether the back key event is processed.
     */
    public abstract boolean onBackPressed();

    /********************** App-level resources **********************/

    /**
     * Called when user click switch camera icon.
     * Note: this function is called before {@link #onCameraPicked(String)}.
     *
     * @param newCameraId the new camera id
     */
    public void onBeforeCameraPicked(String newCameraId);

    /**
     * Called when user click switch camera icon.
     * @param newCameraId the new camera id
     */
    public void onCameraPicked(String newCameraId);
}
