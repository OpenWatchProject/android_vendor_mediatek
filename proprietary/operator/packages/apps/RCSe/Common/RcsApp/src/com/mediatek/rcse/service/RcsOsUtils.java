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

package com.mediatek.rcse.service;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.os.Build;

import com.mediatek.rcse.service.MediatekFactory;
import com.mediatek.rcse.api.Logger;

import java.util.ArrayList;
import java.util.Hashtable;
import java.util.Set;

/**
 * Android OS version utilities
 */
public class RcsOsUtils {

    private static final String TAG = "RcsOsUtils";
    private static Hashtable<String, Integer> sPermissions = new Hashtable<String, Integer>();


    /**
     * Check if the app has the specified permission. If it does not, the app
     * needs to use {@link android.app.Activity#requestPermission}. Note that if
     * it returns true, it cannot return false in the same process as the OS
     * kills the process when any permission is revoked.
     *
     * @param permission
     *            A permission from {@link android.Manifest.permission}
     */
    public static boolean hasPermission(final String permission) {
        // It is safe to cache the PERMISSION_GRANTED result as the process gets
        // killed if the
        // user revokes the permission setting. However, PERMISSION_DENIED
        // should not be
        // cached as the process does not get killed if the user enables the
        // permission setting.

        final Context context = MediatekFactory.getApplicationContext();
        final int permissionState = context.checkSelfPermission(permission);
        sPermissions.put(permission, permissionState);
        boolean result = (sPermissions.get(permission) == PackageManager.PERMISSION_GRANTED);

        Logger.d(TAG, "Check Permission-> " + permission + " is "
                + ((result) ? " granted" : " denied"));
        return result;
    }

    /** Does the app have all the specified permissions */
    public static boolean hasPermissions(final String[] permissions) {
        for (final String permission : permissions) {
            if (!hasPermission(permission)) {
                return false;
            }
        }
        return true;
    }

    public static boolean hasPhonePermission() {
        return hasPermission(Manifest.permission.READ_PHONE_STATE);
    }

    public static boolean hasSmsPermission() {
        return hasPermission(Manifest.permission.READ_SMS);
    }

    public static boolean hasSendSmsPermission() {
        return hasPermission(Manifest.permission.SEND_SMS);
    }

    public static boolean hasSendSmsNoConfirmationPermission() {
        return hasPermission(Manifest.permission.SEND_SMS_NO_CONFIRMATION);
    }

    public static boolean hasLocationPermission() {
        return RcsOsUtils
                .hasPermission(Manifest.permission.ACCESS_FINE_LOCATION);
    }

    public static boolean hasCameraPermission() {
        return RcsOsUtils.hasPermission(Manifest.permission.CAMERA);
    }

    public static boolean hasStoragePermission() {
        // Note that READ_EXTERNAL_STORAGE and WRITE_EXTERNAL_STORAGE are
        // granted or denied
        // together.
        return RcsOsUtils
                .hasPermission(Manifest.permission.READ_EXTERNAL_STORAGE);
    }

    public static boolean hasRecordAudioPermission() {
        return RcsOsUtils.hasPermission(Manifest.permission.RECORD_AUDIO);
    }

    /**
     * Returns array with the set of permissions that have not been granted from
     * the given set. The array will be empty if the app has all of the
     * specified permissions. Note that calling
     * {@link Activity#requestPermissions} for an already granted permission can
     * prompt the user again, and its up to the app to only request permissions
     * that are missing.
     */
    public static String[] getMissingPermissions(final String[] permissions) {
        final ArrayList<String> missingList = new ArrayList<String>();
        for (final String permission : permissions) {
            Logger.d(TAG, "Check Permission-> " + permission);
            if (!hasPermission(permission)) {
                missingList.add(permission);
            }
        }

        final String[] missingArray = new String[missingList.size()];
        missingList.toArray(missingArray);
        Logger.d(TAG, "missingList-> " + missingList.size());
        return missingArray;
    }

    private static String[] sRequiredPermissions = new String[] {
        // required to read rcs contacts
        Manifest.permission.READ_CONTACTS,
        Manifest.permission.READ_EXTERNAL_STORAGE
    };

    private static String[] sAppRequiredPermissions = new String[] {
        // required to read rcs contacts
        Manifest.permission.READ_CONTACTS,
        Manifest.permission.READ_EXTERNAL_STORAGE,
        Manifest.permission.WRITE_EXTERNAL_STORAGE,
        Manifest.permission.WRITE_CONTACTS,
        Manifest.permission.READ_PHONE_STATE,
        Manifest.permission.RECORD_AUDIO
    };

    /** Does the app have the minimum set of permissions required to operate. */
    public static boolean hasRequiredPermissions() {
        return hasPermissions(sRequiredPermissions) ||
            hasPermission(Manifest.permission.READ_CONTACTS);
    }

    public static String[] getMissingRequiredPermissions() {
        Logger.d(TAG, "Request Permission-> " + sRequiredPermissions.length);
        return getMissingPermissions(sRequiredPermissions);
    }

    public static String[] getApkMissingRequiredPermissions() {
        Logger.d(TAG, "Request APK UI Permission-> " + sAppRequiredPermissions.length);
        return getMissingPermissions(sAppRequiredPermissions);
    }

}
