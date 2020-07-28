/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.rcse.activities;

import android.app.Activity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.graphics.Color;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.SystemClock;
import android.preference.PreferenceManager;
import android.provider.Settings;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.rcs.R;
import com.mediatek.rcse.api.RegistrationApi;
import com.mediatek.rcse.service.CoreApplication;
import com.mediatek.rcse.service.MediatekFactory;
import com.mediatek.rcse.service.RcsOsUtils;
import com.mediatek.rcse.settings.RcsSettings;
import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.api.Logger;

import com.gsma.services.rcs.JoynServiceConfiguration;

/**
 * Activity to check if the user has required permissions. If not, it will try
 * to prompt the user to grant permissions. However, the OS may not actually
 * prompt the user if the user had previously checked the "Never ask again"
 * checkbox while denying the required permissions.
 */
public class RcsPermissionCheckActivity extends Activity {
    private static final int REQUIRED_PERMISSIONS_REQUEST_CODE = 1;
    private static final long AUTOMATED_RESULT_THRESHOLD_MILLLIS = 250;
    private static final String PACKAGE_URI_PREFIX = "package:";
    private static final String TAG = "RcsPermissionCheckActivity";
    public static final String ACTION_CONTACTS_PERMISSION_ALLOW = "com.mediatek.rcs.contacts.permission.allow";
    private long mRequestTimeMillis;
    private TextView mNextView;
    private TextView mSettingsView;
    private boolean mRequested = false;
    private static final int DEACTIVATE_JOYN_TEMP = 1;
    private static final int ENABLE_JOYN = 0;

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (redirectIfNeeded()) {
            Logger.d(TAG, "Having all permission So redirect-> ");
            return;
        }

        setContentView(R.layout.permission_check_activity);
        int color = getColor(R.color.permission_check_activity_background);
        // To achieve the appearance of an 80% opacity blend against a black
        // background,
        // each color channel is reduced in value by 20%.
        final int blendedRed = (int) Math.floor(0.8 * Color.red(color));
        final int blendedGreen = (int) Math.floor(0.8 * Color.green(color));
        final int blendedBlue = (int) Math.floor(0.8 * Color.blue(color));

        this.getWindow().setStatusBarColor(
                Color.rgb(blendedRed, blendedGreen, blendedBlue));

        findViewById(R.id.exit).setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(final View view) {
                //exit button clicked
                stopRcsService();
                finish();
            }
        });

        mNextView = (TextView) findViewById(R.id.next);
        mNextView.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(final View view) {
                if (!mRequested) {
                    // prevent click twice to cause permission grant failed.
                    mRequested = true;
                    tryRequestPermission();
                }
            }
        });

        mSettingsView = (TextView) findViewById(R.id.settings);
        mSettingsView.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(final View view) {
                Logger.d(TAG, "Detail Settings -> ");
                final Intent intent = new Intent(
                        Settings.ACTION_APPLICATION_DETAILS_SETTINGS, Uri
                                .parse(PACKAGE_URI_PREFIX + getPackageName()));
                // Uri.parse(PACKAGE_URI_PREFIX + "com.mediatek.rcs"));
                startActivity(intent);
            }
        });
    }

    @Override
    public void onResume() {
        super.onResume();
        if (redirectIfNeeded()) {
            return;
        }
    }

    private void tryRequestPermission() {

        final String[] missingPermissions = RcsOsUtils.getMissingRequiredPermissions();

        if (missingPermissions.length == 0) {
            Logger.d(TAG, "tryRequestPermission - NoMissingRequiredPermissions");
            redirect();
        } else {
            Logger.d(TAG, "tryRequestPermission - requestPermissions for missing:"
                    + missingPermissions.length);
            mRequestTimeMillis = SystemClock.elapsedRealtime();
            requestPermissions(missingPermissions,
                    REQUIRED_PERMISSIONS_REQUEST_CODE);
        }
    }

    @Override
    public void onRequestPermissionsResult(final int requestCode,
            final String permissions[], final int[] grantResults) {

        if (requestCode != REQUIRED_PERMISSIONS_REQUEST_CODE) {
            Logger.e(TAG, "invalid onRequestPermissionsResult");
            return;
        }

        if (permissions.length == 0 && grantResults.length == 0) {
            Logger.e(TAG, "[cancellation] duplicated requestPermissions calls");
            return;
        }

        Logger.d(TAG, "requestCode:" + requestCode + ":" + grantResults.length);

        // We do not use grantResults as some of the granted permissions
        // might have been
        // revoked while the permissions dialog box was being shown for the
        // missing permissions.
        if (RcsOsUtils.hasRequiredPermissions()) {
            Logger.d(TAG, "Permission granted");
            startRcsService();
            redirect();
        } else {
            //permission not granted
            Logger.d(TAG, "Permission not granted");
            stopRcsService();
            // If the permission request completes very quickly, it must be
            // because the system
            // automatically denied. This can happen if the user had
            // previously denied it
            // and checked the "Never ask again" check box.
            if (permissions.length != 0 && isNeverGrantedPermission(permissions[0])) {
                Logger.d(TAG, "onRequestPermissionsResult() never ask again selected");
                mNextView.setVisibility(View.GONE);

                mSettingsView.setVisibility(View.VISIBLE);
                findViewById(R.id.enable_permission_procedure)
                        .setVisibility(View.VISIBLE);
                SharedPreferences sPrefer = PreferenceManager
                        .getDefaultSharedPreferences(getApplicationContext());
                Editor alwaysdeny = sPrefer.edit();
                alwaysdeny.putBoolean("alwaysdenypermission", true);
                alwaysdeny.commit();
                Toast.makeText(getApplicationContext(), "Permission denied. You can change them in Settings->Apps.", Toast.LENGTH_LONG).show();
            }
            finish();
            return;
        }
    }

    public boolean isNeverGrantedPermission(String permission) {
        return !(this.shouldShowRequestPermissionRationale(permission));
    }

    /** Returns true if the redirecting was performed */
    private boolean redirectIfNeeded() {
        if (!RcsOsUtils.hasRequiredPermissions()) {
            Logger.d(TAG, "redirectIfNeeded() hasRequiredPermissions false");
            return false;
        }

        redirect();
        return true;
    }

    private void redirect() {
        Logger.d(TAG, "redirect() entry");
        ContactsListManager.initialize(MediatekFactory.getApplicationContext());
        new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... params) {
                ContactsListManager.getInstance().onContactsDbChange();
                return null;
            }
        } .execute();
        finish();
    }

    private void startRcsService() {
        JoynServiceConfiguration.setServicePermissionState(true,
                MediatekFactory.getApplicationContext());
        this.getApplicationContext().sendBroadcast(new Intent(CoreApplication.LAUNCH_SERVICE));
        RegistrationApi.setServiceActivationState(true);
        RcsSettings.getInstance().setDisableServiceStatus(ENABLE_JOYN);
    }

    private void stopRcsService() {
        JoynServiceConfiguration.setServicePermissionState(false,
                MediatekFactory.getApplicationContext());
        Intent intent = new Intent();
        intent.setAction(CoreApplication.STOP_SERVICE);
        this.getApplicationContext().sendBroadcast(intent);
        Toast.makeText(getApplicationContext(),
                "Joyn service will disconnect due to contact permission not there",
                Toast.LENGTH_LONG).show();
        RegistrationApi.setServiceActivationState(false);
        RcsSettings.getInstance().setDisableServiceStatus(DEACTIVATE_JOYN_TEMP);
    }

}
