package com.mediatek.ygps;

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
    private static final String TAG = "YGPS/Permission";
    private static final int REQUEST_CODE_ASK_LAUNCH_PERMISSIONS = 100;
    private final Activity mActivity;
    private List<String> mLauchPermissionList = new ArrayList<String>();

    /**
     * it need keep the activity context, because permission checking need it.
     * @param activity
     *            the activity which need permission check.
     */
    public PermissionManager(Activity activity) {
        mActivity = activity;
        initLocationLaunchPermissionList();
    }

    private void initLocationLaunchPermissionList() {
        mLauchPermissionList.add(Manifest.permission.ACCESS_FINE_LOCATION);
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
                Log.i(TAG, "getNeedCheckPermissionList() permission ="
                        + permission);
                needCheckPermissionsList.add(permission);
            }
        }
        Log.i(TAG, "getNeedCheckPermissionList() listSize ="
                + needCheckPermissionsList.size());
        return needCheckPermissionsList;
    }

    /**
     * check Location launch permissions, the permission list is defined in the function.
     * @return true if all the launch permissions are already on.
     *         false if more than one launch permission is off.
     */
    public boolean checkLocationLaunchPermissions() {
        List<String> needCheckPermissionsList = getNeedCheckPermissionList(mLauchPermissionList);
        if (needCheckPermissionsList.size() > 0) {
            return false;
        }
        Log.i(TAG, "CheckLocationPermissions(), all on");
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
    public boolean requestLocationLaunchPermissions() {

        List<String> needCheckPermissionsList = getNeedCheckPermissionList(mLauchPermissionList);
        if (needCheckPermissionsList.size() > 0) {
            // should show dialog
            Log.i(TAG, "requestLocationLaunchPermissions(), user check");
            ActivityCompat.requestPermissions(
                            mActivity,
                            needCheckPermissionsList
                                    .toArray(new String[needCheckPermissionsList
                                            .size()]), REQUEST_CODE_ASK_LAUNCH_PERMISSIONS);
            return false;
        }
        Log.i(TAG, "requestLocationLaunchPermissions(), all on");
        return true;
    }

    /**
     * used in the activity permission request result, onRequestPermissionsResult.
     * it is used by the activity to check the request type.
     * @return the request code
     */
    public int getLocationLaunchPermissionRequestCode() {
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
    public boolean isLocationLaunchPermissionsResultReady(
            String permissions[], int[] grantResults) {
        Map<String, Integer> perms = new HashMap<String, Integer>();
        perms.put(Manifest.permission.ACCESS_FINE_LOCATION,
                PackageManager.PERMISSION_GRANTED);
        for (int i = 0; i < permissions.length; i++) {
            perms.put(permissions[i], grantResults[i]);
        }
        if (perms.get(Manifest.permission.ACCESS_FINE_LOCATION)
                == PackageManager.PERMISSION_GRANTED) {
            Log.i(TAG, "isLocationLaunchPermissionsResultReady(),true");
            return true;
        }
        return false;
    }

}
