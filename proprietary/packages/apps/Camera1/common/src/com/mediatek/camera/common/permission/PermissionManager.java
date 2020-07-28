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

package com.mediatek.camera.common.permission;

import android.Manifest;
import android.app.Activity;
import android.content.pm.PackageManager;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * the class is for the activity permission check.
 */
public class PermissionManager {
    private static final Tag TAG = new Tag(PermissionManager.class.getSimpleName());
    private static final int CAM_REQUEST_CODE_ASK_LAUNCH_PERMISSIONS = 100;
    private static final int CAM_REQUEST_CODE_ASK_LOCATION_PERMISSIONS = 101;
    private final Activity mActivity;
    private List<String> mAllPermissionList = new ArrayList<>();
    private List<String> mLaunchPermissionList = new ArrayList<>();
    private List<String> mLocationPermissionList = new ArrayList<>();

    /**
     * it need keep the activity context, because permission checking need it.
     * @param activity
     *            the activity which need permission check.
     */
    public PermissionManager(Activity activity) {
        mActivity = activity;
        initCameraAllPermissionList();
        initCameraLaunchPermissionList();
        initCameraLocationPermissionList();
    }

    /**
     * check camera all permissions, the permission list is defined in the function.
     * @return true if all permissions are already on.
     *         false if more than one launch permission is off.
     */
    public boolean checkCameraAllPermissions() {
        List<String> needCheckPermissionsList = getNeedCheckPermissionList(mAllPermissionList);
        if (needCheckPermissionsList.size() > 0) {
            return false;
        }
        LogHelper.d(TAG, "CheckCameraPermissions(), all on");
        return true;
    }

    /**
     * check camera launch permissions, the permission list is defined in the function.
     * @return true if all the launch permissions are already on.
     *         false if more than one launch permission is off.
     */
    public boolean checkCameraLaunchPermissions() {
        List<String> needCheckPermissionsList = getNeedCheckPermissionList(mLaunchPermissionList);
        if (needCheckPermissionsList.size() > 0) {
            return false;
        }
        LogHelper.d(TAG, "CheckCameraPermissions(), all on");
        return true;
    }

    /**
     * check camera location permission.the permission list is defined in the function.
     * @return true if all the launch permissions are already on.
     *         false if more than one launch permission is off.
     */
    public boolean checkCameraLocationPermissions() {
        List<String> needCheckPermissionsList = getNeedCheckPermissionList(mLocationPermissionList);
        if (needCheckPermissionsList.size() > 0) {
            return false;
        }
        LogHelper.d(TAG, "checkCameraLocationPermissions(), all on");
        return true;
    }

    /**
     * request for the activity all permission.
     * if it need check some permissions, it will show a dialog to user, and
     * the user will decide whether the permission should be granted to the activity.
     * if user denied, the feature for the activity can not be used. and the request result
     * will be sent to the activity
     * which should implements ActivityCompat.OnRequestPermissionsResultCallback.
     * @return true if the activity already had all the permissions in permissionList.
     *         false if the activity had not all the permissions in permissionList.
     */
    public boolean requestCameraAllPermissions() {

        List<String> needCheckPermissionsList = getNeedCheckPermissionList(mAllPermissionList);
        if (needCheckPermissionsList.size() > 0) {
            // should show dialog
            LogHelper.d(TAG, "requestCameraAllPermissions(), user check");
            ActivityCompat.requestPermissions(
                    mActivity,
                    needCheckPermissionsList
                            .toArray(new String[needCheckPermissionsList
                                    .size()]), CAM_REQUEST_CODE_ASK_LAUNCH_PERMISSIONS);
            return false;
        }
        LogHelper.d(TAG, "requestCameraAllPermissions(), all on");
        return true;
    }

    /**
     * request for the activity launch permission.
     * if it need check some permissions, it will show a dialog to user, and
     * the user will decide whether the permission should be granted to the activity.
     * if user denied, the feature for the activity can not be used. and the request result
     * will be sent to the activity
     * which should implements ActivityCompat.OnRequestPermissionsResultCallback.
     * @return true if the activity already had all the permissions in permissionList.
     *         false if the activity had not all the permissions in permissionList.
     */
    public boolean requestCameraLaunchPermissions() {

        List<String> needCheckPermissionsList = getNeedCheckPermissionList(mLaunchPermissionList);
        if (needCheckPermissionsList.size() > 0) {
            // should show dialog
            LogHelper.d(TAG, "requestCameraLaunchPermissions(), user check");
            ActivityCompat.requestPermissions(
                            mActivity,
                            needCheckPermissionsList
                                    .toArray(new String[needCheckPermissionsList
                                            .size()]), CAM_REQUEST_CODE_ASK_LAUNCH_PERMISSIONS);
            return false;
        }
        LogHelper.d(TAG, "requestCameraLaunchPermissions(), all on");
        return true;
    }

    /**
     * request for the camera location permission.
     * if all the location permissions are on, it is ok to go the flow. if not, need show
     * the dialog to ask user to grant the location permission. if user denied to grant
     * the permission. The location feature can not be used.
     * @return true if all the permissions are already on
     *         false if more than one permission is off
     */
    public boolean requestCameraLocationPermissions() {
        List<String> needCheckPermissionsList = getNeedCheckPermissionList(mLocationPermissionList);
        if (needCheckPermissionsList.size() > 0) {
            // should show dialog
            LogHelper.d(TAG, "requestCameraLocationPermissions(), user check");
            ActivityCompat.requestPermissions(
                            mActivity,
                            needCheckPermissionsList
                                    .toArray(new String[needCheckPermissionsList
                                            .size()]), CAM_REQUEST_CODE_ASK_LOCATION_PERMISSIONS);
            return false;
        }
        LogHelper.d(TAG, "requestCameraLocationPermissions(), all on");
        return true;
    }

    /**
     * used in the activity permission request result, onRequestPermissionsResult.
     * it is used by the activity to check the request type.
     * @return the request code
     */
    public int getCameraLaunchPermissionRequestCode() {
        return CAM_REQUEST_CODE_ASK_LAUNCH_PERMISSIONS;
    }

    /**
     * used in the activity permission request result, onRequestPermissionsResult.
     * it is used by the activity to check the request type.
     * @return the request code
     */
    public int getCameraLocationPermissionRequestCode() {
        return CAM_REQUEST_CODE_ASK_LOCATION_PERMISSIONS;
    }

    /**
     * it should be called in the activity request callback, onRequestPermissionsResult.
     * it is to check whether all the permission request results are on.
     * @param permissions the permission list get from onRequestPermissionsResult.
     * @param grantResults the results for requested permissions
     * @return true if all the request permissions are allowed.
     *         false if more than one request permission is denied.
     */
    public boolean isCameraLaunchPermissionsResultReady(
            String permissions[], int[] grantResults) {
        Map<String, Integer> perms = new HashMap<>();
        perms.put(Manifest.permission.CAMERA,
                PackageManager.PERMISSION_GRANTED);
        perms.put(Manifest.permission.RECORD_AUDIO,
                PackageManager.PERMISSION_GRANTED);
        perms.put(Manifest.permission.WRITE_EXTERNAL_STORAGE,
                PackageManager.PERMISSION_GRANTED);
        perms.put(Manifest.permission.READ_EXTERNAL_STORAGE,
                PackageManager.PERMISSION_GRANTED);
        for (int i = 0; i < permissions.length; i++) {
            perms.put(permissions[i], grantResults[i]);
        }
        if (perms.get(Manifest.permission.CAMERA)
                == PackageManager.PERMISSION_GRANTED
                && perms.get(Manifest.permission.RECORD_AUDIO)
                == PackageManager.PERMISSION_GRANTED
                && perms.get(Manifest.permission.WRITE_EXTERNAL_STORAGE)
                == PackageManager.PERMISSION_GRANTED
                && perms.get(Manifest.permission.READ_EXTERNAL_STORAGE)
                == PackageManager.PERMISSION_GRANTED) {
            return true;
        }
        return false;
    }

    /**
     * it should be called in the activity request callback, onRequestPermissionsResult.
     * it is to check whether all the permission request results are on.
     * @param permissions the permission list get from onRequestPermissionsResult.
     * @param grantResults the results for requested permissions
     * @return true if all the request permissions are allowed.
     *         false if more than one request permission is denied.
     */
    public boolean isCameraLocationPermissionsResultReady(
            String permissions[], int[] grantResults) {
        Map<String, Integer> perms = new HashMap<>();
        perms.put(Manifest.permission.ACCESS_COARSE_LOCATION,
                PackageManager.PERMISSION_GRANTED);
        perms.put(Manifest.permission.ACCESS_FINE_LOCATION,
                PackageManager.PERMISSION_GRANTED);
        for (int i = 0; i < permissions.length; i++) {
            perms.put(permissions[i], grantResults[i]);
        }
        if (perms.get(Manifest.permission.ACCESS_COARSE_LOCATION)
                == PackageManager.PERMISSION_GRANTED
                && perms.get(Manifest.permission.ACCESS_FINE_LOCATION)
                == PackageManager.PERMISSION_GRANTED) {
            return true;
        }
        return false;
    }

    private void initCameraAllPermissionList() {
        mAllPermissionList.add(Manifest.permission.CAMERA);
        mAllPermissionList.add(Manifest.permission.RECORD_AUDIO);
        mAllPermissionList.add(Manifest.permission.WRITE_EXTERNAL_STORAGE);
        mAllPermissionList.add(Manifest.permission.READ_EXTERNAL_STORAGE);
        mAllPermissionList.add(Manifest.permission.ACCESS_COARSE_LOCATION);
        mAllPermissionList.add(Manifest.permission.ACCESS_FINE_LOCATION);
    }

    private void initCameraLaunchPermissionList() {
        mLaunchPermissionList.add(Manifest.permission.CAMERA);
        mLaunchPermissionList.add(Manifest.permission.RECORD_AUDIO);
        mLaunchPermissionList.add(Manifest.permission.WRITE_EXTERNAL_STORAGE);
        mLaunchPermissionList.add(Manifest.permission.READ_EXTERNAL_STORAGE);
    }

    private void initCameraLocationPermissionList() {
        mLocationPermissionList.add(Manifest.permission.ACCESS_COARSE_LOCATION);
        mLocationPermissionList.add(Manifest.permission.ACCESS_FINE_LOCATION);
    }

    /**
     * get the need check permission list.
     * some permissions may be already on.
     * @param permissionList
     *            all the dangerous permission for the activity.
     * @return the need check permission list.
     */
    private List<String> getNeedCheckPermissionList(List<String> permissionList) {
        // all needed permissions, may be on or off
        if (permissionList.size() <= 0) {
            return permissionList;
        }
        List<String> needCheckPermissionsList = new ArrayList<>();
        for (String permission : permissionList) {
            if (ContextCompat.checkSelfPermission(mActivity, permission)
                    != PackageManager.PERMISSION_GRANTED) {
                LogHelper.d(TAG, "getNeedCheckPermissionList() permission ="
                        + permission);
                needCheckPermissionsList.add(permission);
            }
        }
        LogHelper.d(TAG, "getNeedCheckPermissionList() listSize ="
                + needCheckPermissionsList.size());
        return needCheckPermissionsList;
    }
}
