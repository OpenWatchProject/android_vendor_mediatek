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

package com.android.contacts.activities;

import android.Manifest.permission;
import android.app.Activity;
import android.content.Intent;

import com.android.contacts.model.AccountTypeManager;

import com.mediatek.contacts.util.ImportExportUtils;
import com.mediatek.contacts.util.Log;

/**
 * Activity that requests permissions needed for ImportVCardActivity.
 */
public class RequestImportVCardPermissionsActivity extends RequestPermissionsActivity {
    private static final String TAG = "RequestImportVCardPermissionsActivity";

    private static final String[] REQUIRED_PERMISSIONS = new String[] {
            // Contacts group
            permission.GET_ACCOUNTS,
            permission.READ_CONTACTS,
            permission.WRITE_CONTACTS,
            // Storage group
            permission.READ_EXTERNAL_STORAGE,
            /// M: [ALPS04208036]  ContactImportExportActivity need READ_PHONE_STATE permission
            /// to Load icc account. @{
            permission.READ_PHONE_STATE,
            /// @}

    };

    @Override
    protected String[] getPermissions() {
        return REQUIRED_PERMISSIONS;
    }

    /**
     * If any permissions the Contacts app needs are missing, open an Activity
     * to prompt the user for these permissions. Moreover, finish the current activity.
     *
     * This is designed to be called inside {@link android.app.Activity#onCreate}
     *
     * @param isCallerSelf whether the vcard import was started from the contacts app itself.
     */
    public static boolean startPermissionActivity(Activity activity, boolean isCallerSelf) {
        /* M: ALPS02742315. mtk no using.using startPermissionActivityEx.@{
        return startPermissionActivity(activity, REQUIRED_PERMISSIONS, isCallerSelf,
                RequestImportVCardPermissionsActivity.class);
        */
        return startPermissionActivityEx(activity, REQUIRED_PERMISSIONS, isCallerSelf,
                RequestImportVCardPermissionsActivity.class);
        /* @} */
    }

    /**
     * M: ALPS02742315.this function come from base startPermissionActivity.but for MTk importExport
     * caller will finish when start action,import/exportVcard activity will finish with default
     * result code which not match caller after get permission successful.so need to add set match
     * related result code to fit for mtk import/export action.@{
     */
    protected static boolean startPermissionActivityEx(Activity activity,
            String[] requiredPermissions, boolean isCallerSelf, Class<?> newActivityClass) {
        if (!hasPermissions(activity, requiredPermissions)) {
            final Intent intent = new Intent(activity, newActivityClass);
            activity.getIntent().putExtra(EXTRA_STARTED_PERMISSIONS_ACTIVITY, true);
            intent.putExtra(PREVIOUS_ACTIVITY_INTENT, activity.getIntent());
            intent.putExtra(EXTRA_IS_CALLER_SELF, isCallerSelf);
            activity.startActivity(intent);
            activity.setResult(ImportExportUtils.RESULT_CODE);
            activity.finish();
            Log.d(TAG, "[startPermissionActivityEx]" + activity.toString() + " finsih,result code: "
                    + ImportExportUtils.RESULT_CODE);
            return true;
        }

        AccountTypeManager.getInstance(activity);
        return false;
    }
    /* @} */
}