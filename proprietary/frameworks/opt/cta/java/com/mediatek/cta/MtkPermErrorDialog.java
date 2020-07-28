/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
package com.mediatek.cta;

import android.app.ActivityManager;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PermissionInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.Resources;
import android.os.UserHandle;
import android.provider.Settings;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.widget.FrameLayout;

class MtkPermErrorDialog extends MtkAppErrorDialog {
    private static final String TAG = "MtkPermErrorDialog";
    private final Context mContext;
    private String mPermission;
    private String mProcessName;
    private String mPkgName;
    private ActivityManager mActivityManager;

    private static final int FORCE_QUIT = 1;

    MtkPermErrorDialog(Context context, String permission,
                String processName, String pkgName) {
        super(context, processName);
        mContext = context;
        mPermission = permission;
        mProcessName = processName;
        mPkgName = pkgName;
        setupUiComponents();
        mActivityManager = (ActivityManager) context.getSystemService(
                           Context.ACTIVITY_SERVICE);
    }

    private void setupUiComponents() {
        Resources res = mContext.getResources();
        setTitle(null);
        CharSequence permissionName = "";
        CharSequence applicationName = "";
        try {
            PermissionInfo permissionInfo = mContext.getPackageManager().
                    getPermissionInfo(mPermission, 0);
            permissionName = permissionInfo.loadLabel(mContext.getPackageManager());
        } catch (NameNotFoundException e) {
            e.printStackTrace();
        }
        try {
            ApplicationInfo ai = mContext.getPackageManager().
                    getApplicationInfo(mPkgName, 0);
            applicationName = mContext.getPackageManager().getApplicationLabel(ai);
        } catch (NameNotFoundException e) {
            applicationName = mProcessName;
        }
        String message;
        boolean isWlanPerm =
            "com.mediatek.permission.CTA_ENABLE_WIFI".equals(mPermission);
        boolean isBtPerm =
            "com.mediatek.permission.CTA_ENABLE_BT".equals(mPermission);
        if (!TextUtils.isEmpty(permissionName)) {
            if (isWlanPerm || isBtPerm) {
                message = res.getString(
                        com.mediatek.internal.R.string.aerr_application_permission_connectivity,
                        applicationName,
                        permissionName);
            } else {
                message = res.getString(
                        com.mediatek.internal.R.string.aerr_application_permission,
                        applicationName,
                        permissionName);
            }
        } else {
            message = res.getString(
                    com.mediatek.internal.R.string.aerr_application_unknown_permission,
                    applicationName);
        }
        setMessage(message);
        setButton(DialogInterface.BUTTON_POSITIVE,
                res.getText(com.mediatek.internal.R.string.mtk_perm_err_dialog_ok),
                new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        String actionName = null ;
                        boolean showDetailedPerms = false;
                        String EXTRA_ALL_PERMISSIONS =
                              "com.android.packageinstaller.extra.ALL_PERMISSIONS";
                        if (isWlanPerm) {
                            actionName = Settings.ACTION_WIFI_SETTINGS ;
                        } else if (isBtPerm) {
                            actionName = Settings.ACTION_BLUETOOTH_SETTINGS;
                        } else {
                            actionName = Intent.ACTION_MANAGE_APP_PERMISSIONS;
                            showDetailedPerms = true;
                        }
                        Intent intent = new Intent(actionName);
                        intent.putExtra(Intent.EXTRA_PACKAGE_NAME, mPkgName);
                        intent.putExtra(Intent.EXTRA_PERMISSION_NAME, mPermission);
                        intent.putExtra(EXTRA_ALL_PERMISSIONS, showDetailedPerms);
                        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                        // [ALPS03027336] we must specify user info in multi-user environment
                        mContext.startActivityAsUser(intent,
                            new UserHandle(UserHandle.USER_CURRENT));
                        //clickButtonForResult(FORCE_QUIT);
                        cancel();
                        Log.d(TAG, "forceStopPackage " + mPkgName + " by click Settings button");
                        mActivityManager.forceStopPackage(mPkgName);
                    }
                });
        setButton(DialogInterface.BUTTON_NEUTRAL,
                res.getText(com.android.internal.R.string.cancel),
                new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        //clickButtonForResult(FORCE_QUIT);
                        cancel();
                        Log.d(TAG, "forceStopPackage " + mPkgName + " by click cancel button");
                        mActivityManager.forceStopPackage(mPkgName);
                    }
                });
    }
}
