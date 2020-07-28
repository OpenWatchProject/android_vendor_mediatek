package com.mediatek.mms.ui;

import android.app.Activity;
import android.content.Intent;
import android.content.res.Resources;
import android.os.Bundle;
import android.util.Log;
import android.widget.Toast;

import com.android.mms.MmsApp;
import com.android.mms.R;

import com.mediatek.mms.util.PermissionCheckUtil;

public class PermissionCheckActivity extends Activity {
    private static final String TAG = "PermissionCheckActivity";
    private static final int REQUIRED_PERMISSIONS_REQUEST_CODE = 1;
    private boolean mIsRequestingPermission = false;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d(TAG, " onCreate" + savedInstanceState);
        if (savedInstanceState == null) {
            final String[] missingArray
                    = getIntent().getStringArrayExtra(PermissionCheckUtil.MISSING_PERMISSIONS);
            PermissionCheckUtil.setPermissionActivityCount(true);
            requestPermissions(missingArray, REQUIRED_PERMISSIONS_REQUEST_CODE);
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions,
                int[] grantResults) {
        finish();
        PermissionCheckUtil.setPermissionActivityCount(false);
        Log.d(TAG, " onRequestPermissionsResult Activity Count: "
                + PermissionCheckUtil.sPermissionsActivityStarted);

        if (PermissionCheckUtil.onRequestPermissionsResult(
                this, requestCode, permissions, grantResults, true)) {
            try {
                Intent previousActivityIntent
                        = (Intent) getIntent().getExtras().get(
                                PermissionCheckUtil.PREVIOUS_ACTIVITY_INTENT);
                startActivity(previousActivityIntent);
            } catch (SecurityException e) {
                Log.e(TAG, " SecurityException happened: " + e);
                Resources res = getResources();
                String mediaType = res.getString(R.string.type_common_file);
                String message = res.getString(R.string.error_add_attachment, mediaType);
                Toast.makeText(this, message, Toast.LENGTH_SHORT).show();
            }
        }
    }
}
