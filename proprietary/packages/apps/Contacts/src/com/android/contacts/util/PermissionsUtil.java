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

package com.android.contacts.util;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import android.Manifest.permission;
import android.app.AppOpsManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.pm.PermissionInfo;
import android.os.Process;
import androidx.core.content.ContextCompat;
import androidx.localbroadcastmanager.content.LocalBroadcastManager;

import com.mediatek.contacts.util.Log;

/**
 * Utility class to help with runtime permissions.
 */
public class PermissionsUtil {
    // Each permission in this list is a cherry-picked permission from a particular permission
    // group. Granting a permission group enables access to all permissions in that group so we
    // only need to check a single permission in each group.
    // Note: This assumes that the app has correctly requested for all the relevant permissions
    // in its Manifest file.
    public static final String PHONE = permission.CALL_PHONE;
    public static final String CONTACTS = permission.READ_CONTACTS;
    public static final String LOCATION = permission.ACCESS_FINE_LOCATION;

    public static boolean hasPhonePermissions(Context context) {
        return hasPermissions(context,/*M:checking full group*/ PHONE_FULL_GROUP);
    }

    public static boolean hasContactsPermissions(Context context) {
        return hasPermissions(context,/*M:checking full group*/ CONTACTS_FULL_GROUP);
    }

    public static boolean hasLocationPermissions(Context context) {
        return hasPermission(context, LOCATION);
    }

    public static boolean hasPermission(Context context, String permission) {
        return ContextCompat.checkSelfPermission(context, permission)
                == PackageManager.PERMISSION_GRANTED;
    }

    public static boolean hasAppOp(Context context, String appOp) {
        final AppOpsManager appOpsManager =
                (AppOpsManager) context.getSystemService(Context.APP_OPS_SERVICE);
        final int mode = appOpsManager.checkOpNoThrow(appOp, Process.myUid(),
                context.getPackageName());
        return mode == AppOpsManager.MODE_ALLOWED;
    }

    /**
     * Rudimentary methods wrapping the use of a LocalBroadcastManager to simplify the process
     * of notifying other classes when a particular fragment is notified that a permission is
     * granted.
     *
     * To be notified when a permission has been granted, create a new broadcast receiver
     * and register it using {@link #registerPermissionReceiver(Context, BroadcastReceiver, String)}
     *
     * E.g.
     *
     * final BroadcastReceiver receiver = new BroadcastReceiver() {
     *     @Override
     *     public void onReceive(Context context, Intent intent) {
     *         refreshContactsView();
     *     }
     * }
     *
     * PermissionsUtil.registerPermissionReceiver(getActivity(), receiver, READ_CONTACTS);
     *
     * If you register to listen for multiple permissions, you can identify which permission was
     * granted by inspecting {@link Intent#getAction()}.
     *
     * In the fragment that requests for the permission, be sure to call
     * {@link #notifyPermissionGranted(Context, String)} when the permission is granted so that
     * any interested listeners are notified of the change.
     */
    public static void registerPermissionReceiver(Context context, BroadcastReceiver receiver,
            String permission) {
        final IntentFilter filter = new IntentFilter(permission);
        LocalBroadcastManager.getInstance(context).registerReceiver(receiver, filter);
    }

    public static void unregisterPermissionReceiver(Context context, BroadcastReceiver receiver) {
        LocalBroadcastManager.getInstance(context).unregisterReceiver(receiver);
    }

    public static void notifyPermissionGranted(Context context, String permission) {
        final Intent intent = new Intent(permission);
        LocalBroadcastManager.getInstance(context).sendBroadcast(intent);
    }

    /// M: MTK modified the AOSP permission group logic.
    /// Now, add permissions full list for real permission group checking
    /// instead of AOSP. This is more logically, and meet the requirement of CTA. @{
    /**
     * The Phone group permissions defined by AOSP.
     */
    public static final String[] PHONE_FULL_GROUP = new String[] {
        permission.CALL_PHONE, permission.READ_PHONE_STATE,
        permission.ADD_VOICEMAIL, permission.USE_SIP,
        permission.PROCESS_OUTGOING_CALLS
    };
    /**
     * The CONTACTS group permissions defined by AOSP.
     */
    public static final String[] CONTACTS_FULL_GROUP = new String[] {
        permission.READ_CONTACTS, permission.WRITE_CONTACTS,
        permission.GET_ACCOUNTS
    };

    /**
     * M: Retrieve all the permissions in the phone group defined by system
     * @param context
     * @param groupName
     * @return permissions array or null
     */
    private static boolean hasPermissions(Context context, String[] permissions) {
        boolean result = false;
        if (permissions == null || permissions.length == 0) {
            return false;
        }

        // Collecting package requested permissions in AndroidManifest.
        PackageInfo packageInfo = null;
        boolean supportGetInfo = true;
        try {
            packageInfo = context.getPackageManager().getPackageInfo(
                context.getApplicationInfo().packageName, PackageManager.GET_PERMISSIONS);
        } catch (NameNotFoundException e) {
            return false;
        } catch (UnsupportedOperationException e) {
            // some mock context such as testcase can't support this.
            supportGetInfo = false;
            Log.d(PermissionsUtil.class.getSimpleName(), "NOT SUPPORTED : " + e.toString());
        }

        if (supportGetInfo) {
            if (packageInfo == null || packageInfo.requestedPermissions == null) {
                return false;
            }
            List<String> requestedPermissions = Arrays.asList(packageInfo.requestedPermissions);
            for (String permission : permissions) {
                // Only the permissions were requested need to be checking granted or not.
                if (requestedPermissions.contains(permission) &&
                        ContextCompat.checkSelfPermission(context, permission)
                        != PackageManager.PERMISSION_GRANTED) {
                    Log.d(PermissionsUtil.class.getSimpleName(), "NOT GRANTED : " + permission);
                    return false;
                }
            }
        } else {
            for (String permission : permissions) {
                if (ContextCompat.checkSelfPermission(context, permission)
                        != PackageManager.PERMISSION_GRANTED) {
                    Log.d(PermissionsUtil.class.getSimpleName(), "NOT GRANTED : " + permission);
                    return false;
                }
            }
        }
        return true;
    }
    /// @}
}
