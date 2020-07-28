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

import android.app.ActivityManager.RunningAppProcessInfo;
import android.app.ActivityManager;
import android.app.AppGlobals;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.Build;
import android.util.Log;

import com.mediatek.cta.CtaManager.ActionType;
import com.mediatek.cta.CtaManager.KeywordType;

import java.text.SimpleDateFormat;
import java.util.Date;

final class CtaInforPrinter {
    private static final String TAG = "CTA_CtaInforPrinter";
    private static final String TAG_CTA_INFORMATION = "ctaifs";

    static void printCtaInfor(Context context, KeywordType keyWordType, String functionName,
            ActionType actionType, String parameter) {
        // Only print in eng/userdebug && cta support load.
        if (Build.TYPE.equals("user") || !CtaUtils.isCtaSupported()) {
            if (CtaUtils.isCtaEnhanceLogEnable()) {
                Log.d(TAG, "userload or cta not support load cannot print cta information");
            }
            return;
        }
        String appName = getAppName(context);
        String processName = getProcessName(context);
        printCtaInforInternal(appName, processName, keyWordType, functionName, actionType,
                parameter);
    }

    static void printCtaInfor(int callingPid, int callingUid, KeywordType keyWordType,
            String functionName, ActionType actionType, String parameter) {
        // Only print in eng/userdebug && cta support load.
        if (Build.TYPE.equals("user") || !CtaUtils.isCtaSupported()) {
            if (CtaUtils.isCtaEnhanceLogEnable()) {
                Log.d(TAG, "userload or cta not support load cannot print cta information");
            }
            return;
        }
        // here use processName as packgeName
        String callingPackage = CtaUtils.getCallingPkgName(callingPid, callingUid);
        String appName = getAppName(callingPackage);
        String processName = callingPackage;
        printCtaInforInternal(appName, processName, keyWordType, functionName, actionType,
                parameter);
    }

    static void printCtaInfor(int appUid, KeywordType keyWordType, String functionName,
            ActionType actionType, String parameter) {
        // Only print in eng/userdebug && cta support load.
        if (Build.TYPE.equals("user") || !CtaUtils.isCtaSupported()) {
            if (CtaUtils.isCtaEnhanceLogEnable()) {
                Log.d(TAG, "userload or cta not support load cannot print cta information");
            }
            return;
        }
        // here use processName as packgeName
        String callingPackage = CtaUtils.getCallingPkgName(appUid);
        String appName = getAppName(callingPackage);
        String processName = callingPackage;
        printCtaInforInternal(appName, processName, keyWordType, functionName, actionType,
                parameter);
    }

    static void printCtaInforInternal(
            String appName, String processName, KeywordType keyWordType, String functionName,
            ActionType actionType, String parameter) {
        // appName is null means the API is called by framework, so no need print.
        if(appName == null) {
            if (CtaUtils.isCtaEnhanceLogEnable()) {
                Log.d(TAG, "appName is null, no need print cta infor");
            }
            return;
        }
        String currentTime = getCurrentTime();
        String keyWord = getKeywordString(keyWordType);
        String action = getActionString(actionType);
        // Because the log is too much, so default is disable.
        // When use command: adb shell setprop "vendor.cta.log.enable" 1
        // the CTA need log will print.
        //if (CtaUtils.isCtaEnhanceLogEnable()) {
            Log.i(TAG_CTA_INFORMATION,
                    currentTime + " <" + appName + ">[" + keyWord + "][" + processName + "]:["
                            + functionName + "] " + action + ".."
                            + ((parameter == null) ? "" : parameter));
        //}
    }

    private static String getCurrentTime() {
        SimpleDateFormat sdf = new SimpleDateFormat();
        sdf.applyPattern("MM月dd日 HH:mm:ss");
        Date date = new Date();
        String currentTime = sdf.format(date);
        return currentTime;
    }

    private static String getAppName(Context context) {
        PackageManager pm = context.getPackageManager();
        ApplicationInfo appInfo;
        try {
            appInfo = pm.getApplicationInfo(context.getPackageName(), 0);
        } catch (NameNotFoundException e) {
            return null;
        }
        if (appInfo != null) {
            CharSequence label = pm.getApplicationLabel(appInfo);
            return (label == null) ? null : label.toString();
        }
        return null;
    }

    private static String getAppName(String packageName) {
        PackageManager pm = AppGlobals.getInitialApplication().getPackageManager();
        ApplicationInfo appInfo;
        try {
            appInfo = pm.getApplicationInfo(packageName, 0);
        } catch (NameNotFoundException e) {
            return null;
        }
        if (appInfo != null) {
            CharSequence label = pm.getApplicationLabel(appInfo);
            return (label == null) ? null : label.toString();
        }
        return null;
    }

    private static String getProcessName(Context context) {
        int pid = android.os.Process.myPid();
        ActivityManager mActivityManager = (ActivityManager) context
                .getSystemService(Context.ACTIVITY_SERVICE);
        for (ActivityManager.RunningAppProcessInfo appProcess : mActivityManager
                .getRunningAppProcesses()) {
            if (appProcess.pid == pid) {
                return appProcess.processName;
            }
        }
        return null;
    }

    private static String getKeywordString(KeywordType type) {
        switch (type) {
            case MICROPHONE:
                return "microphone";
            case SCREENSHOTS:
                return "screenshots";
            case CAMERA:
                return "camera";
            case SMS:
                return "sms";
            case MMS:
                return "mms";
            case LOCATION:
                return "location";
            case LOCALACCOUNT:
                return "localaccount";
            case CONTACTS:
                return "contacts";
            case CALLLOG:
                return "calllog";
            case CALENDAR:
                return "calendar";
            case PHONE:
                return "phone";
            case BROWSER:
                return "browser";
            case DATATRANSTER:
                return "datatranster";
            case NETWORKCONNECT:
                return "networtconnect";
            case PICTURES:
                return "pictures";
            case VIDEOS:
                return "videos";
            case AUDIOS:
                return "audios";
        }
        return "unknown";
    }

    private static String getActionString(ActionType type) {
        switch (type) {
            case CALL_RECORDING:
                return "开启通话录音";
            case LOCAL_RECORDING:
                return "本地录音";
            case BACKGROUND_SCREENSHOTS:
                return "后台截屏";
            case TAKE_PICTUREORVIDEO:
                return "拍照/摄像";
            case RECEIVE_SMS:
                return "接收短信";
            case READ_SMS:
                return "读取短信数据";
            case SEND_SMS:
                return "发送短信";
            case MODIFY_SMS:
                return "修改短信数据";
            case DELETE_SMS:
                return "删除短信数据";
            case READ_MMS:
                return "读取彩信数据";
            case DELETE_MMS:
                return "删除彩信数据";
            case MODIFY_MMS:
                return "修改彩信数据";
            case SEND_MMS:
                return "发送彩信";
            case USE_LOCATION:
                return "定位";
            case READ_LOCATION_INFO:
                return "读取定位信息";
            case READ_LOCAL_ACCOUNTS:
                return "读取用户本机号码";
            case READ_CONTACTS:
                return "读取电话本数据";
            case MODIFY_CONTACTS:
                return "修改用户电话本数据";
            case DELETE_CONTACTS:
                return "删除电话本数据";
            case READ_CALLLOG:
                return "读取通话记录";
            case MODIFY_CALLLOG:
                return "修改通话记录";
            case DELETE_CALLLOG:
                return "删除通话记录";
            case READ_CALENDAR:
                return "读取日程表数据";
            case DELEATE_CALENDAR:
                return "删除日程表数据";
            case MODIFY_CALENDAR:
                return "修改日程表数据";
            case DIRECTLY_CALL_PHONE:
                return "拨打电话";
            case READ_BROWSER_HISTORY:
                return "读取上网记录";
            case READ_BROWSER_BOOKMARK:
                return "读取上网记录";
            case WIFI_DATATRANSTER:
                return "WLAN网络连接传送数据";
            case MOBILE_DATATRANSTER:
                return "移动通信网络数据连接传送数据";
            case WIRELESS_TRANSMITDATA:
                return "无线外围接口传送数据";
            case ENABLE_WIFI_NETWORKCONNECT:
                return "开启WIFI网络连接";
            case ENABLE_MOBILE_NETWORKCONNECT:
                return "开启移动数据网络连接";
            case READ_PICTURES:
                return "读取图片";
            case READ_VIDEOS:
                return "读取视频";
            case READ_AUDIOS:
                return "读取音频";
        }
        return "unknown";
    }
}
