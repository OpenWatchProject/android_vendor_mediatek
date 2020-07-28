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
import android.content.pm.ResolveInfo;
import android.content.pm.PackageParser;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.util.ArrayMap;
import android.util.Log;
import android.util.SparseBooleanArray;

import com.mediatek.cta.CtaManager;
import com.mediatek.cta.CtaManager.ActionType;
import com.mediatek.cta.CtaManager.KeywordType;

import java.util.List;
import java.util.Map;

final public class CtaManagerImpl extends CtaManager {

    private final static String TAG = "CTA_CtaManagerImpl";
    private final AmsWrapper mAmsWrapper;
    private static IAutoBootService sAutoBootService;

    public CtaManagerImpl() {
        mAmsWrapper = AmsWrapper.getInstance();
    }

    public void addAutoBootService(Context context) {
        AutoBootService autoBootService = new AutoBootService(context);
        ServiceManager.addService("autoboot", autoBootService);
        sAutoBootService = IAutoBootService
                .Stub.asInterface(ServiceManager.getService("autoboot"));
    }

    private IAutoBootService getAutoBootService() {
        if (sAutoBootService == null) {
            sAutoBootService = IAutoBootService
                    .Stub.asInterface(ServiceManager.getService("autoboot"));
        }
        return sAutoBootService;
    }

    public void createCtaPermsController(Context context) {
        CtaPermsController.createInstance(context);
    }

    public void linkCtaPermissions(PackageParser.Package pkg) {
        if (isCtaSupported()) {
            CtaPermsController.linkCtaPermissions(pkg);
        }
    }

    public void reportPermRequestUsage(String permName, int uid) {
        CtaPermsController.reportPermRequestUsage(permName, uid);
    }

    public void shutdown() {
        CtaPermsController.shutdown();
    }

    public void systemReady() {
        CtaPermsController.systemReady();
    }

    public boolean isPermissionReviewRequired(PackageParser.Package pkg,
            int userId, boolean reviewRequiredByCache) {
        return CtaPermsController.isPermissionReviewRequired(pkg, userId, reviewRequiredByCache);
    }

    public List<String> getPermRecordPkgs() {
        return CtaPermsController.getPermRecordPkgs();
    }

    public List<String> getPermRecordPerms(String packageName) {
        return CtaPermsController.getPermRecordPerms(packageName);
    }

    public List<Long> getRequestTimes(String packageName, String permName) {
        return CtaPermsController.getRequestTimes(packageName, permName);
    }

    public boolean showPermErrorDialog(Context context, int uid, String processName, String pkgName, String exceptionMsg) {
        if (!isCtaSupported()) return false;
        boolean needshowMtkErrordialog = mAmsWrapper.needShowMtkPermDialog(context, uid, pkgName, exceptionMsg);
        if (needshowMtkErrordialog) {
            mAmsWrapper.showPermErrorDialog(context, processName, pkgName);
        }
        return needshowMtkErrordialog;
    }

    public void filterReceiver(Context context, Intent intent, List<ResolveInfo> receivers, int userId) {
        if (isCtaSupported() && receivers != null && receivers.size() > 0) {
            mAmsWrapper.filterReceiver(context, intent, receivers, userId);
        }
    }

    public boolean isCtaSupported() {
        return CtaUtils.isCtaSupported();
    }

    public void setCtaSupported(boolean enable)  {
         CtaUtils.setCtaSupported(enable);
    }

    public boolean isCtaOnlyPermission(String perm) {
        return CtaUtils.isCtaOnlyPermission(perm);
    }

    public boolean isCtaAddedPermGroup(String group) {
        return CtaUtils.isCtaAddedPermGroup(group);
    }

    public boolean isCtaMonitoredPerms(String perm) {
        return CtaUtils.isCtaMonitoredPerms(perm);
    }

    public boolean isPlatformPermissionGroup(String pkgName, String groupName) {
        return CtaUtils.isPlatformPermissionGroup(pkgName, groupName);
    }

    public String[] getCtaAddedPermissionGroups() {
        return CtaUtils.getCtaAddedPermissionGroups();
    }

    public ArrayMap<String, String> getCtaPlatformPerms() {
        return CtaUtils.getCtaPlatformPerms();
    }

    public boolean enforceCheckPermission(final String permission, final String action) {
        return CtaUtils.enforceCheckPermission(permission, action);
    }

    public boolean enforceCheckPermission(final String pkgName, final String permission,
            final String action) {
        return CtaUtils.enforceCheckPermission(pkgName, permission, action);
    }

    public boolean isPlatformPermission(String pkgName, String permName) {
        return CtaUtils.isPlatformPermission(pkgName, permName);
    }

    public boolean isSystemApp(Context context ,String pkgName) {
        return CtaUtils.isSystemApp(context, pkgName);
    }

    public boolean needGrantCtaRuntimePerm(boolean isUpdated, int targetSdkVersion) {
        return CtaUtils.needGrantCtaRuntimePerm(isUpdated, targetSdkVersion);
    }

    public String[] getCtaOnlyPermissions() {
        return CtaUtils.getCtaOnlyPermissions();
    }

    public int opToSwitch(int op) {
        return CtaAppOpsManager.opToSwitch(op);
    }

    public String opToName(int op) {
        return CtaAppOpsManager.opToName(op);
    }

    public String opToPublicName(int op) {
        return CtaAppOpsManager.opToPublicName(op);
    }

    public int strDebugOpToOp(String op) {
        return CtaAppOpsManager.strDebugOpToOp(op);
    }

    public String opToPermission(int op) {
        return CtaAppOpsManager.opToPermission(op);
    }

    public String opToRestriction(int op) {
        return CtaAppOpsManager.opToRestriction(op);
    }

    public int permissionToOpCode(String permission) {
        return CtaAppOpsManager.permissionToOpCode(permission);
    }

    public boolean opAllowSystemBypassRestriction(int op) {
        return CtaAppOpsManager.opAllowSystemBypassRestriction(op);
    }

    public int opToDefaultMode(int op) {
        return CtaAppOpsManager.opToDefaultMode(op);
    }

    public boolean opAllowsReset(int op) {
        return CtaAppOpsManager.opAllowsReset(op);
    }

    public String permissionToOp(String permission) {
        return CtaAppOpsManager.permissionToOp(permission);
    }

    public int strOpToOp(String op) {
        return CtaAppOpsManager.strOpToOp(op);
    }

    public String[] getOpStrs() {
        return CtaAppOpsManager.getOpStrs();
    }

    public int getOpNum() {
        return CtaAppOpsManager._NUM_OP;
    }

    public boolean needClearReviewFlagAfterUpgrade(boolean updatedPkgReviewRequired,
        String pkg, String name) {
        boolean needToClear = true;
        boolean isPlatformPermission = CtaUtils.isPlatformPermission(pkg,name);
        if (isPlatformPermission && updatedPkgReviewRequired) {
            needToClear = false;
        }
        return needToClear;
    }

    public boolean checkAutoBootPermission(Context context, String packageName, int userId) {
        if (isCtaSupported()) {
            return mAmsWrapper.checkAutoBootPermission(context, packageName, userId);
        } else {
            return true;
        }
    }

    public void changeAppAutoBootStatus(Context context, String packageName,
                                        boolean status, int userId) {
        try {
            if (isCtaSupported()) {
                getAutoBootService().changeAppAutoBootStatus(packageName, status, userId);
            }
        } catch (RemoteException e) {
            Log.e(TAG, "call changeAppAutoBootStatus method failed");
        }

    }

    public Map<String, Boolean> queryAutoBootRecords(Context context, int userId) {
        try {
            if (isCtaSupported()) {
                return getAutoBootService().queryAutoBootRecords(userId);
            }
        } catch (RemoteException e) {
            Log.e(TAG, "call queryAutoBootRecords method failed");
        }
        return null;
    }

    public void printCtaInfor(
            Context context, KeywordType keyWordType, String functionName, ActionType actionType,
            String parameter) {
        CtaInforPrinter.printCtaInfor(context, keyWordType, functionName, actionType, parameter);
    }

    public void printCtaInfor(
            int callingPid, int callingUid, KeywordType keyWordType, String functionName,
            ActionType actionType, String parameter) {
        CtaInforPrinter.printCtaInfor(callingPid, callingUid, keyWordType, functionName, actionType,
                parameter);
    }

    public void printCtaInfor(int appUid, KeywordType keyWordType, String functionName,
            ActionType actionType, String parameter) {
        CtaInforPrinter.printCtaInfor(appUid, keyWordType, functionName, actionType, parameter);
    }
}
