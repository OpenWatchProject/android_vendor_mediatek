package com.debug.loggerui.permission;

import android.app.Activity;
import android.content.pm.PackageManager;
import android.os.Bundle;

import com.debug.loggerui.utils.Utils;

/**
 * @author MTK81255
 *
 */
public class PermisssionActivity extends Activity {
    private static final String TAG = Utils.TAG + "/PermisssionActivity";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        String[] permission = getIntent().getStringArrayExtra(PermissionUtils.EXTRA_PERMISSIONS);
        int requestCode = getIntent().getIntExtra(PermissionUtils.EXTRA_REQUEST_CODE, 0);
        requestPermissions(permission, requestCode);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions,
            int[] grantResults) {
        if (permissions == null || permissions.length == 0 || grantResults == null
                || grantResults.length == 0) {
            Utils.logw(TAG, "onRequestPermissionsResult, Permission or grant result is null!");
            return;
        }
        switch (requestCode) {
        case PermissionUtils.PERMISSIONS_REQUEST_TO_WRITE_EXTERNAL_STORAGE:
        case PermissionUtils.PERMISSIONS_REQUEST_TO_READ_EXTERNAL_STORAGE:
            if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                Utils.logi(TAG, "The permission " + permissions[0] + " is granted!");
            } else {
                Utils.logw(TAG, "The permission " + permissions[0] + " is denied!");
            }
            PermissionUtils.sIsPermissionRequesting = false;
            finish();
            break;
        case PermissionUtils.PERMISSIONS_REQUEST_TO_READ_AND_WRITE_EXTERNAL_STORAGE:
            if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                Utils.logi(TAG, "The permission " + permissions[0] + " is granted!");
            } else {
                Utils.logw(TAG, "The permission " + permissions[0] + " is denied!");
            }
            if (permissions.length > 1 && grantResults.length > 1 && permissions[1] != null) {
                if (grantResults[1] == PackageManager.PERMISSION_GRANTED) {
                    Utils.logi(TAG, "The permission " + permissions[1] + " is granted!");
                } else {
                    Utils.logw(TAG, "The permission " + permissions[1] + " is denied!");
                }
            }
            PermissionUtils.sIsPermissionRequesting = false;
            finish();
            break;
        default:
            PermissionUtils.sIsPermissionRequesting = false;
            finish();
        }
    }
}
