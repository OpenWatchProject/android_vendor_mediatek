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
import android.os.Build;
import android.os.ServiceManager;
import android.os.RemoteException;
import android.util.Log;
import android.content.pm.PackageParser;

import java.util.List;

class CtaPermsController {

    static boolean DEBUG = "eng".equals(Build.TYPE);

    private final static String TAG = "CTA_CtaPermsController";

    private static Context sContext;
    private static PermRecordsController sPermRecordsController;
    private static IPermRecordsController sPrc;
    private static CtaPermsController sInstance;

    private CtaPermsController(Context context) {
        sContext = context;
        sPermRecordsController = new PermRecordsController(context);
        ServiceManager.addService("permrecords", sPermRecordsController);
    }

    static void createInstance(Context context) {
        if (sInstance == null) {
            sInstance = new CtaPermsController(context);
        }
    }

    static void linkCtaPermissions(PackageParser.Package pkg) {
        if (sContext != null) {
            CtaPermLinker.getInstance(sContext).link(pkg);
        }
    }

    static void reportPermRequestUsage(String permName, int uid) {
        resetPermRecordsController();
        if (sPrc != null) {
            try {
                sPrc.reportPermRequestUsage(permName, uid);
            } catch (RemoteException e) {
                Log.d(TAG, "reportPermRequestUsage failed for service not running");
            }
        }
    }

    static void shutdown() {
        resetPermRecordsController();
        if (sPrc != null) {
            try {
                sPrc.shutdown();
            } catch (RemoteException e) {
                Log.d(TAG, "shutdown failed for service not running");
            }
        }
    }

    static void systemReady() {
        resetPermRecordsController();
        if (sPrc != null) {
            try {
                sPrc.systemReady();
            } catch (RemoteException e) {
                Log.d(TAG, "systemReady failed for service not running");
            }
        }
    }

    static boolean isPermissionReviewRequired(PackageParser.Package pkg,
            int userId, boolean reviewRequiredByCache) {
        if (sContext != null) {
            return PermReviewFlagHelper.getInstance(sContext)
                    .isPermissionReviewRequired(pkg, userId, reviewRequiredByCache);
        }
        return false;
    }

    static List<String> getPermRecordPkgs() {
        resetPermRecordsController();
        if (sPrc != null) {
            List<String> recordPkgs = null;
            try {
                recordPkgs = sPrc.getPermRecordPkgs();
            } catch (RemoteException e) {
                Log.d(TAG, "getPermRecordPkgs failed for service not running");
                return null;
            }
            return recordPkgs;
        }
        return null;
    }

    static List<String> getPermRecordPerms(String packageName) {
        resetPermRecordsController();
        if (sPrc != null) {
            List<String> recordPerms = null;
            try {
                recordPerms = sPrc.getPermRecordPerms(packageName);
            } catch (RemoteException e) {
                Log.d(TAG, "getPermRecordPerms failed for service not running");
                return null;
            }
            return recordPerms;
        }
        return null;
    }

    static List<Long> getRequestTimes(String packageName, String permName) {
        resetPermRecordsController();
        if (sPrc != null) {
            List<Long> recordTimes = null;
            try {
                recordTimes = sPrc.getPermRecords(packageName, permName).getRequestTimes();
            } catch (RemoteException e) {
                Log.d(TAG, "getPermRecords failed for service not running");
                return null;
            }
            return recordTimes;
        }
        return null;
    }

    static String parsePermName(int uid, String packageName, String exceptionMsg) {
        return PermErrorHelper.getInstance(sContext).parsePermName(uid, packageName,
                exceptionMsg);
    }

    private static void resetPermRecordsController() {
      if (!CtaUtils.isCtaSupported()) {
          sPrc = null;
          return;
      }
      if (sPrc == null) {
          sPrc = IPermRecordsController
                   .Stub.asInterface(ServiceManager.getService("permrecords"));
        }
    }
}
