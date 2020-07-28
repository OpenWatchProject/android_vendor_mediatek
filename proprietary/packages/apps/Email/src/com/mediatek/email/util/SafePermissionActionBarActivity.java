package com.mediatek.email.util;

import android.app.Activity;
import android.app.ActivityManager;
import android.content.pm.PackageManager;
import android.os.Bundle;
import androidx.appcompat.app.AppCompatActivity;
import android.widget.Toast;

import com.android.mail.utils.LogUtils;

public class SafePermissionActionBarActivity extends AppCompatActivity {

    private static int REQUEST_PERMISSION_REQUEST_CODE = 100;
    private Bundle mSavedInstanceState = null;

    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mSavedInstanceState = savedInstanceState;
        if (ActivityManager.isUserAMonkey()) {
            LogUtils.d(this.getLocalClassName(), "Finish activity as it is being started by monkey");
            finish();
        }
        checkPermission();
    }

    private void checkPermission() {
        String[] permissions = PermissionsUtil.needPermissionList(this);
        LogUtils.d(this.getLocalClassName(), "checkPermission size:" + permissions.length);
        if (permissions.length > 0) {
            LogUtils.d(this.getLocalClassName(), "requestPermissions size:" + permissions.length);
            requestPermissions(permissions, REQUEST_PERMISSION_REQUEST_CODE);
        } else {
             //Do Nothing
            //resetActivityAfterPermission(mSavedInstanceState);
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    public void resetActivityAfterPermission(Bundle savedInstanceState) {
        LogUtils.d(this.getLocalClassName(), "doTaskAfterPermission");
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        if (requestCode == REQUEST_PERMISSION_REQUEST_CODE) {
            LogUtils.d(this.getLocalClassName(), "onRequestPermissionsResult permissions:"
                    + permissions.length);
            for (int result : grantResults) {
                if (PackageManager.PERMISSION_GRANTED != result) {
                    Toast.makeText(this, "You have disabled some required permissions.",
                            Toast.LENGTH_LONG).show();
                    finish();
                    return;
                }
            }
            // other case
            resetActivityAfterPermission(mSavedInstanceState);
        }
    }
}
