package com.debug.loggerui.permission;

import android.Manifest;
import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager;

import com.debug.loggerui.MyApplication;
import com.debug.loggerui.utils.Utils;

import java.util.ArrayList;
import java.util.List;

/**
 * @author MTK81255
 *
 */
public class PermissionUtils {
    private static final String TAG = Utils.TAG + "/PermissionUtils";

    public static final int PERMISSIONS_REQUEST_TO_READ_EXTERNAL_STORAGE = 1;
    public static final int PERMISSIONS_REQUEST_TO_WRITE_EXTERNAL_STORAGE = 2;
    public static final int PERMISSIONS_REQUEST_TO_READ_AND_WRITE_EXTERNAL_STORAGE = 3;
    public static final String EXTRA_PERMISSIONS = "extra_permissions";
    public static final String EXTRA_REQUEST_CODE = "extra_request_code";

    /**
     * @param permission
     *            String
     * @return boolean
     */
    public static boolean hasPermission(String permission) {
        return (MyApplication.getInstance().getApplicationContext()
                .checkSelfPermission(permission) == PackageManager.PERMISSION_GRANTED);
    }

    public static boolean sIsPermissionRequesting = false;

    /**
     * @param permissions
     *            String[]
     * @param requestCode
     *            int
     */
    public synchronized static void requestPermission(String[] permissions, int requestCode) {
        if (sIsPermissionRequesting) {
            return;
        }
        Utils.logi(TAG, "requestPermission, permissions.length = " + permissions.length
                + ", requestCode = " + requestCode);
        Intent permissionIntent = new Intent(MyApplication.getInstance().getApplicationContext(),
                PermisssionActivity.class);
        permissionIntent.putExtra(EXTRA_PERMISSIONS, permissions);
        permissionIntent.putExtra(EXTRA_REQUEST_CODE, requestCode);
        permissionIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        MyApplication.getInstance().startActivity(permissionIntent);
        sIsPermissionRequesting = true;
    }

    /**
     * @param activity
     *            Activity
     * @return boolean
     */
    public static boolean showWriteRational(Activity activity) {
        if (activity != null) {
            return activity.shouldShowRequestPermissionRationale(
                    Manifest.permission.WRITE_EXTERNAL_STORAGE);
        } else {
            return false;
        }

    }

    /**
     * requestStoragePermissions if needed.
     */
    public static void requestStoragePermissions() {
        List<String> permissions = new ArrayList<String>();
        if (!hasPermission(Manifest.permission.READ_EXTERNAL_STORAGE)) {
            permissions.add(Manifest.permission.READ_EXTERNAL_STORAGE);
        }
        if (!hasPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE)) {
            permissions.add(Manifest.permission.WRITE_EXTERNAL_STORAGE);
        }
        if (permissions.size() > 0) {
            requestPermission(permissions.toArray(new String[0]),
                    PERMISSIONS_REQUEST_TO_READ_AND_WRITE_EXTERNAL_STORAGE);
        }
    }

}
