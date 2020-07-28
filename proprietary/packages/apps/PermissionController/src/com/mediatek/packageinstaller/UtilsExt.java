/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

package com.mediatek.packageinstaller;

import android.content.Context;
import android.util.Log;
import com.android.packageinstaller.permission.model.AppPermissionGroup;
import com.android.packageinstaller.permission.model.AppPermissions;
import com.android.packageinstaller.permission.model.Permission;
import com.android.packageinstaller.permission.model.PermissionState;
import com.android.packageinstaller.permission.utils.Utils;
import com.mediatek.cta.CtaManager;
import com.mediatek.cta.CtaManagerFactory;

import java.util.List;
import java.util.HashMap;

/**
 * The class used for Pre-grant permissions in review UI
 *
 */
public class UtilsExt {

    private static final String TAG = "UtilsExt";
    public static CtaManager sCtaManager = CtaManagerFactory.getInstance().makeCtaManager();
    public static HashMap<String, PermissionState> mPermStateCache = new HashMap<>();

    public static void initPermStateCache(Context context, AppPermissions appPermissions,
            List<String> revokedPerms) {
        Log.i(TAG, "initPermStateCache");
        mPermStateCache.clear();
        for (AppPermissionGroup group : appPermissions.getPermissionGroups()) {
            for (Permission permission : group.getPermissions()) {
                if (!Utils.shouldShowPermission(context, group)) {
                    continue;
                }
                int state = revokedPerms != null && revokedPerms.contains(permission.getName())
                        ? PermissionState.STATE_DENIED
                        : PermissionState.STATE_ALLOWED;
                Log.d(TAG, "permission.getName() = " + permission.getName() + " state = " + state);
                mPermStateCache.put(permission.getName(),
                        new PermissionState(group, permission, state));
            }
            AppPermissionGroup backgroundGroup = group.getBackgroundPermissions();
            if (backgroundGroup != null) {
                for (Permission permission : backgroundGroup.getPermissions()) {
                    if (!Utils.shouldShowPermission(context, group)) {
                        continue;
                    }
                    int state = revokedPerms != null && revokedPerms.contains(permission.getName())
                            ? PermissionState.STATE_DENIED
                            : PermissionState.STATE_ALLOWED;
                    Log.d(TAG,
                            "permission.getName() = " + permission.getName() + " state = " + state);
                    mPermStateCache.put(permission.getName(),
                            new PermissionState(group, permission, state));
                }
            }
        }
    }

    public static void setPermGroupStateCache(AppPermissions appPermissions, String permGroupName,
            boolean grant) {
        AppPermissionGroup group = appPermissions.getPermissionGroup(permGroupName);
        for (Permission permission : group.getPermissions()) {
            setPermStateCache(permission,
                    grant ? PermissionState.STATE_ALLOWED : PermissionState.STATE_DENIED);
        }
        AppPermissionGroup backgroundGroup = group.getBackgroundPermissions();
        if (backgroundGroup != null) {
            for (Permission permission : backgroundGroup.getPermissions()) {
                setPermStateCache(permission,
                        grant ? PermissionState.STATE_ALLOWED : PermissionState.STATE_DENIED);
            }
        }
    }

    public static void setPermGroupStateCache(AppPermissionGroup group, boolean grant) {
        for (Permission permission : group.getPermissions()) {
            setPermStateCache(permission,
                    grant ? PermissionState.STATE_ALLOWED : PermissionState.STATE_DENIED);
        }
        AppPermissionGroup backgroundGroup = group.getBackgroundPermissions();
        if (backgroundGroup != null) {
            for (Permission permission : backgroundGroup.getPermissions()) {
                setPermStateCache(permission,
                        grant ? PermissionState.STATE_ALLOWED : PermissionState.STATE_DENIED);
            }
        }
    }

    public static void setPermStateCache(Permission permission, int state) {
        Log.d(TAG,
                "setPermStateCache(), permission: " + permission.getName() + ", state: " + state);
        if (mPermStateCache == null || mPermStateCache.get(permission.getName()) == null) {
            Log.w(TAG, "setPermStateCache(), can not find permission: " + permission.getName());
            return;
        }
        mPermStateCache.get(permission.getName()).setState(state);
    }

    public static boolean isPermGrantedByCache(Permission permission) {
        if (mPermStateCache == null || mPermStateCache.get(permission.getName()) == null) {
            Log.w(TAG, "isPermGrantedByCache(), can not find permission: " + permission.getName());
            return false;
        }
        return mPermStateCache.get(permission.getName())
                .getState() == PermissionState.STATE_ALLOWED;
    }

    public static boolean isPermGroupGrantedByCache(AppPermissionGroup group) {
        boolean grant = false;
        for (Permission permission : group.getPermissions()) {
            grant |= isPermGrantedByCache(permission);
        }
        AppPermissionGroup backgroundGroup = group.getBackgroundPermissions();
        if (backgroundGroup != null) {
            for (Permission permission : backgroundGroup.getPermissions()) {
                grant |= isPermGrantedByCache(permission);
            }
        }
        return grant;
    }

}
