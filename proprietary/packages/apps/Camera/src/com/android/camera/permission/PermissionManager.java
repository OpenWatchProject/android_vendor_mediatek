package com.android.camera.permission;

import android.Manifest;
import android.content.pm.PackageManager;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;

import com.android.camera.CameraActivity;
import com.android.camera.Log;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * the class is for the activity permission check.
 */
public class PermissionManager {
    private static final String TAG = "PermissionManager";
    private static final int CAM_REQUEST_CODE_ASK_LAUNCH_PERMISSIONS = 100;
    private static final int CAM_REQUEST_CODE_ASK_LOCATION_PERMISSIONS = 101;
    private final CameraActivity mActivity;
    private List<String> mLauchPermissionList = new ArrayList<String>();
    private List<String> mLocationPermissionList = new ArrayList<String>();

    /**
     * it need keep the activity context, because permission checking need it.
     * @param activity
     *            the activity which need permission check.
     */
    public PermissionManager(CameraActivity activity) {
        mActivity = activity;
        initCameraLaunchPermissionList();
        initCameraLocationPermissionList();
    }

    private void initCameraLaunchPermissionList() {
        mLauchPermissionList.add(Manifest.permission.CAMERA);
        mLauchPermissionList.add(Manifest.permission.RECORD_AUDIO);
        mLauchPermissionList.add(Manifest.permission.WRITE_EXTERNAL_STORAGE);
        mLauchPermissionList.add(Manifest.permission.READ_EXTERNAL_STORAGE);
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
        List<String> needCheckPermissionsList = new ArrayList<String>();
        for (String permission : permissionList) {
            if (ContextCompat.checkSelfPermission(mActivity, permission)
                    != PackageManager.PERMISSION_GRANTED) {
                needCheckPermissionsList.add(permission);
            }
        }
        return needCheckPermissionsList;
    }

    /**
     * check camera launch permissions, the permission list is defined in the function.
     * @return true if all the launch permissions are already on.
     *         false if more than one launch permission is off.
     */
    public boolean checkCameraLaunchPermissions() {
        List<String> needCheckPermissionsList = getNeedCheckPermissionList(mLauchPermissionList);
        if (needCheckPermissionsList.size() > 0) {
            return false;
        }
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

        List<String> needCheckPermissionsList = getNeedCheckPermissionList(mLauchPermissionList);
        if (needCheckPermissionsList.size() > 0) {
            // should show dialog
            Log.d(TAG, "requestCameraLaunchPermissions(), user check");
            ActivityCompat.requestPermissions(
                            mActivity,
                            needCheckPermissionsList
                                    .toArray(new String[needCheckPermissionsList
                                            .size()]), CAM_REQUEST_CODE_ASK_LAUNCH_PERMISSIONS);
            return false;
        }
        Log.d(TAG, "requestCameraLaunchPermissions(), all on");
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
            Log.d(TAG, "requestCameraLocationPermissions(), user check");
            ActivityCompat.requestPermissions(
                            mActivity,
                            needCheckPermissionsList
                                    .toArray(new String[needCheckPermissionsList
                                            .size()]), CAM_REQUEST_CODE_ASK_LOCATION_PERMISSIONS);
            return false;
        }
        Log.d(TAG, "requestCameraLocationPermissions(), all on");
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
        Map<String, Integer> perms = new HashMap<String, Integer>();
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
        Map<String, Integer> perms = new HashMap<String, Integer>();
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

}
