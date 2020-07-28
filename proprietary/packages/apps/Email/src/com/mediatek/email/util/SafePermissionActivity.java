package com.mediatek.email.util;

import android.app.Activity;
import android.app.ActivityManager;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.widget.Toast;

import com.android.email2.ui.MailActivityEmail;
import com.android.email.EmailApplication;
import com.android.email.setup.AuthenticatorSetupIntentHelper;
import com.android.mail.utils.LogUtils;

public class SafePermissionActivity extends Activity {

    private static int REQUEST_PERMISSION_REQUEST_CODE = 100;

    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
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
            LogUtils.d(this.getLocalClassName(), "Request Code: " + REQUEST_PERMISSION_REQUEST_CODE + "requestPermissions size:" + permissions.length);
					//MTK suggestion for ALPS02757121
            ((EmailApplication)getApplication()).setPermissionCheckingStateFlag(true);
            requestPermissions(permissions, REQUEST_PERMISSION_REQUEST_CODE);
        } else {
            //resetActivityAfterPermission();
            // Do nothing;
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    public void resetActivityAfterPermission() {
        LogUtils.d(this.getLocalClassName(), "doTaskAfterPermission");
        this.startActivity(new Intent(this, MailActivityEmail.class));
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        LogUtils.d(this.getLocalClassName(), "Permission grant result for request code: " + requestCode);
        if (requestCode == REQUEST_PERMISSION_REQUEST_CODE) {
            LogUtils.d(this.getLocalClassName(), "onRequestPermissionsResult permissions:"
                    + permissions.length);

			//MTK suggestion for ALPS02757121
            ((EmailApplication)getApplication()).setPermissionCheckingStateFlag(false);
            int flowMode = getIntent().getIntExtra(AuthenticatorSetupIntentHelper.EXTRA_FLOW_MODE,
                                       AuthenticatorSetupIntentHelper.FLOW_MODE_UNSPECIFIED);
            for (int result : grantResults) {
                if (PackageManager.PERMISSION_GRANTED != result) {
                    Toast.makeText(this, "You not assign some permission", Toast.LENGTH_LONG)
                            .show();

                    if(flowMode == AuthenticatorSetupIntentHelper.FLOW_MODE_NO_ACCOUNTS) {
                        ((EmailApplication)getApplication()).setDuplicateRequest(true);
                    }
                    finishAndRemoveTask();
                    return;
                }
            }
            // other case
            resetActivityAfterPermission();
        }
    }
}
