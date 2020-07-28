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

import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageParser;
import android.content.pm.PackageParser.Activity;
import android.content.pm.PackageParser.ActivityIntentInfo;
//import android.content.pm.UsesPermissionInfo;
import android.Manifest;
import android.text.TextUtils;
import android.util.Log;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;

/**
 * Auto link some permission for some apps. When add CTA permssion
 * CTA_CONFERENCE_CALL,CTA_CALL_FORWARD, CTA_SEND_EMAIL,CTA_SEND_MMS
 * in system, but the 3rd party apps not define these permissions in
 * their AndroidManifest.xml. So we should auto link these permissions
 * to app's requestedPermissions and usesPermissionInfos when apk parse.
 * @hide
 */
class CtaPermLinker {

    private static boolean DEBUG = CtaUtils.isCtaEnhanceLogEnable();
    private static final String TAG = "Cta_CtaPermLinker";
    private static CtaPermLinker sInstance;

    private static HashSet<String> sForceAddEmailPkgs;
    private static HashSet<String> sForceAddMmsPkgs;

    private CtaPermLinker(Context context) {
        if (sForceAddEmailPkgs == null) {
            sForceAddEmailPkgs = new HashSet<String>(
                    Arrays.asList(context.getResources().getStringArray(
                            com.mediatek.internal.R.array.force_add_send_email_pkgs)));
        }
        if (sForceAddMmsPkgs == null) {
            sForceAddMmsPkgs = new HashSet<String>(
                    Arrays.asList(context.getResources().getStringArray(
                            com.mediatek.internal.R.array.force_add_send_mms_pkgs)));
        }
        if (DEBUG) {
            for (String pkg : sForceAddEmailPkgs) {
                Log.d(TAG, "sForceAddEmailPkgs pkg = " + pkg);
            }
            for (String pkg : sForceAddMmsPkgs) {
                Log.d(TAG, "sForceAddMmsPkgs pkg = " + pkg);
            }
        }
    }

    static CtaPermLinker getInstance(Context context) {
        if (sInstance == null) {
            sInstance = new CtaPermLinker(context);
        }
        return sInstance;
    }

    void link(final PackageParser.Package pkg) {
        long start = System.currentTimeMillis();
        linkCtaPermissionsInternal(pkg);
        long duration = System.currentTimeMillis() - start;
        String packageName = pkg != null ? pkg.packageName : "null";
        if (DEBUG) {
            Log.i(TAG, "linkCtaPermissions takes " + duration + " ms for pkg: " + packageName);
        }
    }

    private void linkCtaPermissionsInternal(final PackageParser.Package pkg) {
        if (pkg == null) {
            Log.w(TAG, "linkCtaPermissionsInternal pkg is null");
            return;
        }

        // permissions mapping
        for (int i = 0; i < pkg.requestedPermissions.size(); i++) {
            String perm = pkg.requestedPermissions.get(i);
            if (!CtaPermissions.MAP.containsKey(perm)) {
                continue;
            }
            for (String ctaPerm : CtaPermissions.MAP.get(perm)) {
                addCtaPermission(pkg, ctaPerm);
            }
        }

        // BEGIN: handle CTA_SEND_MMS & CTA_SEND_EMAIL permissions
        boolean hasInternetPermission = pkg.requestedPermissions.contains(
                Manifest.permission.INTERNET);
        handleCtaSendEmailPerm(pkg, hasInternetPermission);
        handleCtaSendMmsPerm(pkg, hasInternetPermission);
        // END: handle CTA_SEND_MMS & CTA_SEND_EMAIL permissions
    }

    /*
     * Criteria of adding CTA_SEND_EMAIL permission:
         * 1. Declares "android.permission.INTERNET" permission
         * 2. Support one of the conditions below:
         *    2.1. Package name in force_add_send_email_pkgs list
         *    2.2. Package name contains "mail"
         *    2.3. Exists any Activity that supports
         *         send action & "mailto"
         *         data scheme
     */
    private void handleCtaSendEmailPerm(final PackageParser.Package pkg,
            boolean hasInternetPermission) {
        if (!hasInternetPermission)
            return;
        if (sForceAddEmailPkgs.contains(pkg.packageName)) {
            addCtaPermission(pkg, com.mediatek.Manifest.permission.CTA_SEND_EMAIL);
        }
        if (!TextUtils.isEmpty(pkg.packageName) && pkg.packageName.contains("mail")) {
            addCtaPermission(pkg, com.mediatek.Manifest.permission.CTA_SEND_EMAIL);
        }
        if (pkg.requestedPermissions.contains(com.mediatek.Manifest.permission.CTA_SEND_EMAIL))
            return;
        if (pkg.activities == null) return;
        final ArrayList<Activity> activities = pkg.activities;
        for (int i = 0; i < activities.size(); i++) {
            Activity activity = activities.get(i);
            ArrayList<ActivityIntentInfo> filters = activity.intents;
            if (filters == null) continue;
            final int countFilters = filters.size();
            for (int j = 0; j < countFilters; j++) {
                ActivityIntentInfo aii = filters.get(j);
                if (aii.hasAction(Intent.ACTION_SEND)
                        || aii.hasAction(Intent.ACTION_SENDTO)
                        || aii.hasAction(Intent.ACTION_SEND_MULTIPLE)) {
                    if (aii.hasDataScheme("mailto")) {
                        addCtaPermission(pkg, com.mediatek.Manifest.permission.CTA_SEND_EMAIL);
                        break;
                    }
                }
            }
        }
    }

    /*
     * Criteria of adding CTA_SEND_MMS permission:
     * 1. Declares "android.permission.INTERNET" permission
     * 2. Support one of the conditions below:
     *    2.1. Package name in force_add_send_mms_pkgs list
     *    2.2. Declares "android.permission.RECEIVE_MMS" permission
     *    2.3. Exists any Activity that supports
     *         send action & "mms"
     *         data scheme
     *    2.4. Exists any Activity that supports
     *         send action & "mmsto"
     *         data scheme
     */
    private void handleCtaSendMmsPerm(final PackageParser.Package pkg,
            boolean hasInternetPermission) {
        if (!hasInternetPermission)
            return;
        if (sForceAddMmsPkgs.contains(pkg.packageName)) {
            addCtaPermission(pkg, com.mediatek.Manifest.permission.CTA_SEND_MMS);
        }
        if (pkg.requestedPermissions.contains(Manifest.permission.RECEIVE_MMS)) {
            addCtaPermission(pkg, com.mediatek.Manifest.permission.CTA_SEND_MMS);
        }
        if (pkg.requestedPermissions.contains(com.mediatek.Manifest.permission.CTA_SEND_MMS))
            return;
        if (pkg.activities == null) return;
        final ArrayList<Activity> activities = pkg.activities;
        for (int i = 0; i < activities.size(); i++) {
            Activity activity = activities.get(i);
            ArrayList<ActivityIntentInfo> filters = activity.intents;
            if (filters == null) continue;
            final int countFilters = filters.size();
            for (int j = 0; j < countFilters; j++) {
                ActivityIntentInfo aii = filters.get(j);
                if (aii.hasAction(Intent.ACTION_SEND)
                        || aii.hasAction(Intent.ACTION_SENDTO)
                        || aii.hasAction(Intent.ACTION_SEND_MULTIPLE)) {
                    if (aii.hasDataScheme("mms") || aii.hasDataScheme("mmsto")) {
                        addCtaPermission(pkg, com.mediatek.Manifest.permission.CTA_SEND_MMS);
                        break;
                    }
                }
            }
        }
    }

    private void addCtaPermission(final PackageParser.Package pkg, final String perm) {
        int index = pkg.requestedPermissions.indexOf(perm);
        if (index == -1) {
            pkg.requestedPermissions.add(perm);
//            pkg.usesPermissionInfos.add(new UsesPermissionInfo(perm));
            if (DEBUG) {
                Log.i(TAG, "pkg.requestedPermissions add: " + perm);
            }
        }
    }
}
