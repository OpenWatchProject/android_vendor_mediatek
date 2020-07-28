package com.mediatek.sensorhub.ui;

import android.Manifest;
import android.app.Activity;
import android.content.pm.PackageManager;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.util.Log;


import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * the class is for the activity permission check.
 */
public class PermissionManager {
    private static final String TAG = "SH/PermissionManager";
    private static final int REQUEST_CODE_ASK_LAUNCH_PERMISSIONS = 300;
    private final Activity mActivity;
    private List<String> mLauchPermissionList = new ArrayList<String>();

    /**
     * it need keep the activity context, because permission checking need it.
     * @param activity
     *            the activity which need permission check.
     */
    public PermissionManager(Activity activity) {
        mActivity = activity;
        initLaunchPermissionList();
    }

    private void initLaunchPermissionList() {
        Log.i(TAG, "initLaunchPermissionList");
        mLauchPermissionList.add(Manifest.permission.WRITE_MEDIA_STORAGE);
        mLauchPermissionList.add(Manifest.permission.READ_EXTERNAL_STORAGE);
        mLauchPermissionList.add(Manifest.permission.WRITE_EXTERNAL_STORAGE);
        mLauchPermissionList.add(Manifest.permission.ACTIVITY_RECOGNITION);

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
            Log.i(TAG, "getNeedCheckPermissionList() Check permission ="
                    + permission);
            if (ContextCompat.checkSelfPermission(mActivity, permission)
                    != PackageManager.PERMISSION_GRANTED) {
                Log.i(TAG, "getNeedCheckPermissionList() Add permission ="
                        + permission);
                needCheckPermissionsList.add(permission);
            }
        }
        Log.i(TAG, "getNeedCheckPermissionList() listSize ="
                + needCheckPermissionsList.size());
        return needCheckPermissionsList;
    }

    /**
     * check launch permissions, the permission list is defined in the function.
     * @return true if all the launch permissions are already on.
     *         false if more than one launch permission is off.
     */
    public boolean checkLaunchPermissions() {
        List<String> needCheckPermissionsList = getNeedCheckPermissionList(mLauchPermissionList);
        if (needCheckPermissionsList.size() > 0) {
            return false;
        }
        Log.i(TAG, "CheckPermissions(), all on");
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
    public boolean requestLaunchPermissions() {

        List<String> needCheckPermissionsList = getNeedCheckPermissionList(mLauchPermissionList);
        if (needCheckPermissionsList.size() > 0) {
            // should show dialog
            Log.i(TAG, "requestLaunchPermissions(), user check");
            ActivityCompat.requestPermissions(
                            mActivity,
                            needCheckPermissionsList
                                    .toArray(new String[needCheckPermissionsList
                                            .size()]), REQUEST_CODE_ASK_LAUNCH_PERMISSIONS);
            return false;
        }
        Log.i(TAG, "requestLaunchPermissions(), all on");
        return true;
    }

    /**
     * used in the activity permission request result, onRequestPermissionsResult.
     * it is used by the activity to check the request type.
     * @return the request code
     */
    public int getLaunchPermissionRequestCode() {
        return REQUEST_CODE_ASK_LAUNCH_PERMISSIONS;
    }

    /**
     * it should be called in the activity request callback, onRequestPermissionsResult.
     * it is to check whether all the permission request results are on.
     * @param permissions the permission list get from onRequestPermissionsResult.
     * @param grantResults the results for requested permissions
     * @return true if all the request permissions are allowed.
     *         false if more than one request permission is denied.
     */
    public boolean isLaunchPermissionsResultReady(
            String permissions[], int[] grantResults) {
        Map<String, Integer> perms = new HashMap<String, Integer>();
        perms.put(Manifest.permission.READ_EXTERNAL_STORAGE,
                PackageManager.PERMISSION_GRANTED);
        perms.put(Manifest.permission.WRITE_EXTERNAL_STORAGE,
                PackageManager.PERMISSION_GRANTED);
        perms.put(Manifest.permission.WRITE_MEDIA_STORAGE,
                PackageManager.PERMISSION_GRANTED);
        perms.put(Manifest.permission.ACTIVITY_RECOGNITION,
                PackageManager.PERMISSION_GRANTED);
        for (int i = 0; i < permissions.length; i++) {
            perms.put(permissions[i], grantResults[i]);
        }
        if (perms.get(Manifest.permission.READ_EXTERNAL_STORAGE)
                == PackageManager.PERMISSION_GRANTED
                && perms.get(Manifest.permission.WRITE_EXTERNAL_STORAGE)
                == PackageManager.PERMISSION_GRANTED
                && perms.get(Manifest.permission.WRITE_MEDIA_STORAGE)
                == PackageManager.PERMISSION_GRANTED
                && perms.get(Manifest.permission.ACTIVITY_RECOGNITION)
                == PackageManager.PERMISSION_GRANTED) {
            Log.i(TAG, "isLaunchPermissionsResultReady(),true");
            return true;
        }
        return false;
    }

}
