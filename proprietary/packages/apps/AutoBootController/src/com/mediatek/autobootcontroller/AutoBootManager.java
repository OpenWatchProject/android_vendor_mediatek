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
 * MediaTek Inc. (C) 2019. All rights reserved.
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

package com.mediatek.autobootcontroller;



import android.content.Context;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.IPackageManager;
import android.content.pm.ParceledListSlice;
import android.content.pm.ResolveInfo;
import android.os.Process;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.UserHandle;
import android.util.Log;

import com.mediatek.cta.CtaManager;
import com.mediatek.cta.CtaManagerFactory;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.Set;

public class AutoBootManager {
    private static final String TAG = "AutoBootManager";
    private static final String PKG_TAG = "pkg";
    private List<AutoBootRecord> mAutoBootList = new ArrayList<AutoBootRecord>();
    private Context mContext;
    private int mUid;
    private boolean mIsChanged = false;
    private static AutoBootManager mInstance = null;
    public boolean mIsUsed = false;
    private CtaManager mCtaManager;

    public static synchronized AutoBootManager getInstance(Context context, int uid){
        if (mInstance == null){
            mInstance = new AutoBootManager(context, uid);
        }
        Log.d(TAG, "AutoBootManager" + mInstance);
        return mInstance;
    }

    public AutoBootManager(Context context, int uid) {
        mContext = context;
        mUid = uid;
        mCtaManager = CtaManagerFactory.getInstance().makeCtaManager();
        Log.d(TAG, "AutoBootManager");
    }

    public void setflag(boolean used) {
        mIsUsed = used;
    }

    public List<AutoBootRecord> getBootList() {
        return mAutoBootList;
    }

    public boolean isSettingsChanged() {
        return mIsChanged;
    }

    public void setPgkBootPolicy(AutoBootRecord info) {
        synchronized (mAutoBootList){
            for (AutoBootRecord record : mAutoBootList) {
                if (info.mPackageName.equals(record.mPackageName)) {
                    if (info.isEnable() == record.isEnable()) {
                        Log.e(TAG, "setPgkBootPolicy, policy:same");
                        return;
                    }
                    record.setEnable(info.isEnable());
                    mIsChanged = true;
                    break;
                }
            }
        }
    }

    public void initAutoBootList() {
        List<String> packages = getThirdPartyApp(mContext);
        Map<String, Boolean> records = mCtaManager.queryAutoBootRecords(mContext, mUid);
        if (records == null) {
            return;
        }
        Set<String> pkgs = records.keySet();
        for (String pgkName : pkgs) {
            Log.d(TAG, "pgkName = " + pgkName + ", status = " + records.get(pgkName));
        }

        synchronized (mAutoBootList) {
            mAutoBootList.clear();
            for (String pkgName : packages) {
                boolean enable = true;
                Boolean enabled = records.get(pkgName);
                if (enabled != null) {
                    enable = enabled;
                    records.remove(pkgName);
                }
                mAutoBootList.add(new AutoBootRecord(pkgName, enable));
            }
        }

        if (!records.isEmpty()) {
            //can not find these packages, maybe they have been removed, reset status.
            for (String pkgName : records.keySet()) {
                mCtaManager.changeAppAutoBootStatus(mContext, pkgName, true, mUid);
            }
        }
    }

    private boolean isBuildInApp(ApplicationInfo appInfo) {
        if (appInfo != null && ((appInfo.flags & ApplicationInfo.FLAG_SYSTEM) != 0)
                                || (appInfo.flags & ApplicationInfo.FLAG_UPDATED_SYSTEM_APP) != 0
                                || (!appInfo.sourceDir.startsWith("/data/app/"))) {
            return true;
        }
        return false;
    }

    //control all the 3rd party app
    private List<String> getThirdPartyApp(Context context) {
         List<String> bootControlList = new ArrayList<String>();
         List<ApplicationInfo> originalAppList =
                mContext.getPackageManager().getInstalledApplications(
                        PackageManager.GET_UNINSTALLED_PACKAGES | PackageManager
                                .GET_DISABLED_COMPONENTS);
        if (originalAppList != null) {
            for (ApplicationInfo appInfo : originalAppList) {
                String pkgName = appInfo.packageName;

                if (isBuildInApp(appInfo)) {
                    continue;
                }
                if (pkgName != null && !bootControlList.contains(pkgName)) {
                    Log.d(TAG, "getPackageListReceivingSpecifiedIntent() " + "add "
                        + pkgName + " in the list");
                    bootControlList.add(pkgName);
                }
            }
        }
        return bootControlList;
    }

    public void saveAutoBootRecord() {
        synchronized (mAutoBootList) {
            for (AutoBootRecord record : mAutoBootList) {
                mCtaManager.changeAppAutoBootStatus(mContext, record.mPackageName,
                                                    record.isEnable(), mUid);
            }
        }
    }
}
