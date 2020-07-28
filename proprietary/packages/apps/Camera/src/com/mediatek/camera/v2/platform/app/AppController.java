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
 * MediaTek Inc. (C) 2015. All rights reserved.
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

package com.mediatek.camera.v2.platform.app;

import com.mediatek.camera.v2.platform.ModeChangeListener;
import com.mediatek.camera.v2.platform.device.CameraDeviceManager;
import com.mediatek.camera.v2.platform.module.ModuleUi;
import com.mediatek.camera.v2.services.CameraServices;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;

import com.android.camera.v2.app.location.LocationManager;


public interface AppController {

    /**
     * @return the {@link android.content.Context} being used.
     */
    public Activity getActivity();

    /**
     * Returns the currently active mode index.
     */
    public int getCurrentModeIndex();

    /**
     * Get old mode index.
     * @return Return old mode index.
     */
    public int getOldModeIndex();
    /**
     * @return Returns the new camera API manager.
     *         the {@link com.mediatek.camera.v2.platform.device.CameraDeviceManager}
     */

    public CameraDeviceManager getNewCameraManager();

    /**
     * @return Returns the new camera API manager.
     *         the {@link com.mediatek.camera.v2.platform.device.CameraDeviceManager}
     */
    public CameraDeviceManager getCameraManager();

    /**
     * @return returns the LocationManager
     */
    public LocationManager getLocationManager();

    /********************************** Media saving ****************************/

    /**
     * Notifies the app of the newly captured media.
     */
    public void notifyNewMedia(Uri uri);

    /********************************** UI management ***************************/

    /**
     * Returns the {@link com.mediatek.camera.v2.platform.app.AppUi}.
     *
     * @return {@code null} if not available yet.
     */
    public AppUi getCameraAppUi();

    /**
     * Locks the system orientation.
     */
    public void lockOrientation();

    /**
     * Unlocks the system orientation.
     */
    public void unlockOrientation();

    /********************************** Preview management ************************/

    public void setModuleUiListener(ModuleUi moduleUi);

    public void updatePreviewSize(int previewWidth, int previewHeight);

    /**
     * Gets called from module when preview is started.
     */
    public void onPreviewStarted();

    /**
     * Gets called from module when preview aspect ratio has changed.
     *
     * @param aspectRatio aspect ratio of preview stream
     */
    public void updatePreviewAspectRatioAndSize(
            float aspectRatio, int previewWidth, int previewHeight);

    /**
     * Adds a listener to receive callbacks when preview area changes.
     */
    public void addPreviewAreaSizeChangedListener(
            ModuleUi.PreviewAreaChangedListener listener);

    /**
     * Removes a listener that receives callbacks when preview area changes.
     */
    public void removePreviewAreaSizeChangedListener(
            ModuleUi.PreviewAreaChangedListener listener);

    /***************************************app resources*****************************/
    /**
     * Keeps the screen turned on.
     *
     * @param enabled Whether to keep the screen on.
     */
    public void enableKeepScreenOn(boolean enabled);
    public AppContext     getAppContext();
    public CameraServices getServices();

    public void setModeChangeListener(ModeChangeListener modeChangeListener);

    /**
     * Set result and finish camera activity
     * @param resultCode
     */
    public void setResultExAndFinish(int resultCode);

    /**
     * Set result and finish camera activity
     * @param resultCode
     * @param data
     */
    public void setResultExAndFinish(int resultCode, Intent data);
    /**
     * Update storage space and show hint
     * to remind use storage state
     * this will call when storage state change.
     */
    public void updateStorageSpaceAndHint();

    /**
     * Show error message and finish camera activity.
     * @param messageId The resource id of message.
     */
    public void showErrorAndFinish(int messageId);
}
