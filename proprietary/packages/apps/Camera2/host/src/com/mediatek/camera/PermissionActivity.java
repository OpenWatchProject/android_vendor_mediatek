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

package com.mediatek.camera;

import android.os.AsyncTask;
import android.os.Bundle;
import android.support.v4.app.ActivityCompat;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.loader.FeatureLoader;
import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.common.permission.PermissionManager;

/**
 * This is for permission check.
 */

public abstract class PermissionActivity extends QuickActivity implements
        ActivityCompat.OnRequestPermissionsResultCallback {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(
            PermissionActivity.class.getSimpleName());
    private static final int CAM_ACTIVITY_PERMISSION_STATE_CREATED = 1;
    private static final int CAM_ACTIVITY_PERMISSION_STATE_RESUMED = 2;
    private static final int CAM_ACTIVITY_PERMISSION_STATE_PAUSED = 3;
    private static final int CAM_ACTIVITY_PERMISSION_STATE_DESTROYED = 4;
    private PermissionManager mPermissionManager;
    private Bundle mSavedInstanceState;
    private int mActivityState = CAM_ACTIVITY_PERMISSION_STATE_DESTROYED;

    /**
     * The result of CameraActivity permission check.
     * Launch permissions must be all on, that the camera can be launch normally,
     * otherwise exit the camera app.
     *
     * @param requestCode  camera permission check code, used when requested permissions and
     *                     the code will be back in the permissions requested result.
     * @param permissions  the dangerous permissions that the activity defined in manifest.
     * @param grantResults the permission result for every permission.
     */
    public void onRequestPermissionsResult(int requestCode,
                                           String permissions[], int[] grantResults) {
        LogHelper.d(TAG, "onRequestPermissionsResult(), grantResults = " + grantResults.length);
        if (grantResults.length <= 0) {
            return;
        }
        if (mPermissionManager.getCameraLaunchPermissionRequestCode()
                == requestCode) {
            // If permission all on, activity will resume.
            if (!mPermissionManager.isCameraLaunchPermissionsResultReady(
                    permissions, grantResults)) {
                // more than one critical permission was denied
                // activity finish, exit and destroy
                //toast
                finish();
            } else {
                new AsyncTask<Void, Void, Void>() {
                    @Override
                    protected Void doInBackground(Void... voids) {
                        CameraApiHelper.setForceCreateDeviceSpec(true);
                        CameraApiHelper.getCameraApiType(null);
                        CameraApiHelper.getDeviceSpec(getApplicationContext());
                        FeatureLoader.loadBuildInFeatures(getApplicationContext());
                        FeatureLoader.loadPluginFeatures(getApplicationContext());
                        CameraApiHelper.setForceCreateDeviceSpec(false);
                        return null;
                    }
                } .executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
            }
        }
    }

    protected void onPermissionCreateTasks(Bundle savedInstanceState) {
        mPermissionManager = new PermissionManager(this);
        mSavedInstanceState = savedInstanceState;
        /**
         * features used the dangerous permissions should follow the flow
         * of permission task: onCreatePermissionTasks(),
         * onResumePermissionTasks(),onPausePermissionTasks(),
         * onDestroyPermissionTasks().
         */
        if (mPermissionManager.checkCameraLaunchPermissions()) {
            onCreateTasks(savedInstanceState);
            mActivityState = CAM_ACTIVITY_PERMISSION_STATE_CREATED;
        }
    }

    protected void onPermissionStartTasks() {
        onStartTasks();
    }

    protected void onPermissionResumeTasks() {
        if (!mPermissionManager.checkCameraLaunchPermissions()) {
            if (!mPermissionManager.requestCameraAllPermissions()) {
                return;
            }
        }
        if (mActivityState == CAM_ACTIVITY_PERMISSION_STATE_DESTROYED) {
            onCreateTasks(mSavedInstanceState);
        }
        mSavedInstanceState = null;
        onResumeTasks();
        mActivityState = CAM_ACTIVITY_PERMISSION_STATE_RESUMED;
    }

    protected void onPermissionPauseTasks() {
        if (mActivityState == CAM_ACTIVITY_PERMISSION_STATE_RESUMED) {
            onPauseTasks();
            mActivityState = CAM_ACTIVITY_PERMISSION_STATE_PAUSED;
        }
    }

    protected void onPermissionStopTasks() {
        onStopTasks();
    }

    protected void onPermissionDestroyTasks() {
        if (mActivityState != CAM_ACTIVITY_PERMISSION_STATE_DESTROYED) {
            onDestroyTasks();
            mActivityState = CAM_ACTIVITY_PERMISSION_STATE_DESTROYED;
        }
    }

    /**
     * Subclasses should override this in place of {@link Activity#onCreate}.
     */
    protected void onCreateTasks(Bundle savedInstanceState) {
    }

    /**
     * Subclasses should override this in place of {@link Activity#onStart}.
     */
    protected void onStartTasks() {
    }

    /**
     * Subclasses should override this in place of {@link Activity#onResume}.
     */
    protected void onResumeTasks() {
    }

    /**
     * Subclasses should override this in place of {@link Activity#onPause}.
     */
    protected void onPauseTasks() {
    }

    /**
     * Subclasses should override this in place of {@link Activity#onStop}.
     */
    protected void onStopTasks() {
    }

    /**
     * Subclasses should override this in place of {@link Activity#onDestroy}.
     */
    protected void onDestroyTasks() {
    }
}
