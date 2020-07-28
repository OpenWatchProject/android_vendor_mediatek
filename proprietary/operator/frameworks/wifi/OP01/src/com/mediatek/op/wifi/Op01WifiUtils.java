/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.op.wifi;

import android.app.ActivityManager;
import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.database.ContentObserver;
import android.net.Uri;
import android.net.wifi.ScanResult;
import android.net.wifi.SupplicantState;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiManager;
import android.os.Build;
import android.provider.Settings;
import android.util.Log;

import com.android.server.wifi.StateChangeResult;
import com.mediatek.provider.MtkSettingsExt;

import java.lang.reflect.Field;
import java.util.List;

public class Op01WifiUtils {

    public static String AUTOCONNECT_ENABLE_ALL_NETWORKS =
            "com.mediatek.common.wifi.AUTOCONNECT_ENABLE_ALL_NETWORK";

    public static String RESELECT_DIALOG_CLASSNAME =
            "com.mediatek.op.wifi.WifiReselectApDialog";

    public static String ACTION_RESELECTION_AP = "android.net.wifi.WIFI_RESELECTION_AP";

    public static String ACTION_SUSPEND_NOTIFICATION_NON_SYS =
            "com.mediatek.wifi.ACTION_SUSPEND_NOTIFICATION_NON_SYS";

    public static String EXTRA_SUSPEND_TYPE = "type";

    public static String WIFISETTINGS_CLASSNAME =
            "com.android.settings.Settings$WifiSettingsActivity";

    public static String ACTION_WIFI_FAILOVER_GPRS_DIALOG =
            "com.mediatek.intent.WIFI_FAILOVER_GPRS_DIALOG";

    // Used in Intent.setPackage(...), target to BroadcastReceiver runs in system
    public static String PACKAGE_ANDROID = "android";

    public static String PACKAGE_NAME_MANIFEST = "com.mediatek.server.wifi.op01";
    public static String CLASS_NAME_DATA_DIALOG = "com.mediatek.op.wifi.DataConnectionDialog";

    public static String WIFI_NOTIFICATION_ACTION = "android.net.wifi.WIFI_NOTIFICATION";
    public static String EXTRA_NOTIFICATION_SSID = "ssid";
    public static String EXTRA_NOTIFICATION_NETWORKID = "network_id";
    public static String EXTRA_SHOW_RESELECT_DIALOG_FLAG = "SHOW_RESELECT_DIALOG";

    public static long SUSPEND_NOTIFICATION_DURATION = 60 * 60 * 1000;

    public static int DEFAULT_FRAMEWORK_SCAN_INTERVAL_MS = 15000;
    public static int MIN_INTERVAL_CHECK_WEAK_SIGNAL_MS = 60000; /* 60 seconds */

    // If we scan too frequently then supplicant cannot disconnect from weak-signal network.
    public static int MIN_INTERVAL_SCAN_SUPRESSION_MS = 10000;
    public static int BEST_SIGNAL_THRESHOLD = -79;
    public static int WEAK_SIGNAL_THRESHOLD = -85;
    public static int MIN_NETWORKS_NUM = 2;
    public static int BSS_EXPIRE_AGE = 10;
    public static int BSS_EXPIRE_COUNT = 1;

    public static int NOTIFY_TYPE_SWITCH = 0;
    public static int NOTIFY_TYPE_RESELECT = 1;

    public static int WIFI_CONNECT_REMINDER_ALWAYS = 0;

    private static final String LOG_PREFIX = "Op01Wifi";

    // TODO: IS_USERDEBUG for debug only
    private static final boolean DEBUG = Build.IS_ENG || Log.isLoggable(LOG_PREFIX, Log.DEBUG)
            || Build.IS_USERDEBUG;

    private static final String TAG = "Op01WifiUtils";

    private static final int MAX_TASK_NUMBER = 1;

    private static final int SECURITY_NONE = 0;
    private static final int SECURITY_WEP = 1;
    private static final int SECURITY_PSK = 2;
    private static final int SECURITY_EAP = 3;
    private static final int SECURITY_WAPI_PSK = 4;
    private static final int SECURITY_WAPI_CERT = 5;
    private static final int SECURITY_WPA2_PSK = 6;

    // NOTE: keep these values in-sync with WifiSettingsStore
    private static final int WIFI_DISABLED = 0;
    private static final int WIFI_DISABLED_AIRPLANE_ON = 3;

    public static int getSecurity(ScanResult result) {
        if (result.capabilities.contains("WAPI-PSK")) {
            return SECURITY_WAPI_PSK;
        } else if (result.capabilities.contains("WAPI-CERT")) {
            return SECURITY_WAPI_CERT;
        } else if (result.capabilities.contains("WEP")) {
            return SECURITY_WEP;
        } else if (result.capabilities.contains("PSK")) {
            return SECURITY_PSK;
        } else if (result.capabilities.contains("EAP")) {
            return SECURITY_EAP;
        } else if (result.capabilities.contains("WPA2-PSK")) {
            return SECURITY_WPA2_PSK;
        }
        return SECURITY_NONE;
    }

    public static int getSecurity(WifiConfiguration config) {
        if (config.allowedKeyManagement.get(WifiConfiguration.KeyMgmt.WPA_PSK)) {
            return SECURITY_PSK;
        }
        if (config.allowedKeyManagement.get(WifiConfiguration.KeyMgmt.WPA_EAP)
                || config.allowedKeyManagement.get(WifiConfiguration.KeyMgmt.IEEE8021X)) {
            return SECURITY_EAP;
        }
        if (config.allowedKeyManagement.get(WifiConfiguration.KeyMgmt.WAPI_PSK)) {
            return SECURITY_WAPI_PSK;
        }
        if (config.allowedKeyManagement.get(WifiConfiguration.KeyMgmt.WAPI_CERT)) {
            return SECURITY_WAPI_CERT;
        }
        if (config.wepTxKeyIndex >= 0 && config.wepTxKeyIndex < config.wepKeys.length
                && config.wepKeys[config.wepTxKeyIndex] != null) {
            return SECURITY_WEP;
        }
        if (config.allowedKeyManagement.get(WifiConfiguration.KeyMgmt.WPA2_PSK)) {
            return SECURITY_WPA2_PSK;
        }
        return SECURITY_NONE;
    }

    public static boolean isWifiOnAndEnabled(Context context, ClientModeAdapter adapter) {
        if (!isWifOnInSettings(context)) {
            log("[isWifiOnAndEnabled] off in settings");
            return false;
        }

        if (!isWifiEnabled(adapter)) {
            log("[isWifiOnAndEnabled] not enable in ClientModeImpl");
            return false;
        }
        return true;
    }

    private static boolean isWifOnInSettings(Context context) {
        ContentResolver cr = context.getContentResolver();
        int persistedWifiState;
        try {
            persistedWifiState = Settings.Global.getInt(cr, Settings.Global.WIFI_ON);
        } catch (Settings.SettingNotFoundException e) {
            Settings.Global.putInt(cr, Settings.Global.WIFI_ON, WIFI_DISABLED);
            persistedWifiState = WIFI_DISABLED;
        }

        return !(persistedWifiState == WIFI_DISABLED ||
                persistedWifiState == WIFI_DISABLED_AIRPLANE_ON);
    }

    private static boolean isWifiEnabled(ClientModeAdapter adapter) {
        int state = adapter.syncGetWifiState();
        return state == WifiManager.WIFI_STATE_ENABLED;
    }

    public static void startScan(Context context) {
        WifiManager wifiManager = (WifiManager) context.getSystemService(Context.WIFI_SERVICE);
        wifiManager.startScan();
    }

    public static void showWifiNotifyDialog(Context context, String ssid, int networkId) {
        Intent intent = new Intent(WIFI_NOTIFICATION_ACTION);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        intent.putExtra(EXTRA_NOTIFICATION_SSID, ssid);
        intent.putExtra(EXTRA_NOTIFICATION_NETWORKID, networkId);
        context.startActivity(intent);
    }

    public static void showReselectDialog(Context context) {
        Intent intent = new Intent(ACTION_RESELECTION_AP);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        context.startActivity(intent);
    }

    public static boolean isTopReselectDialog(Context context) {
        String classname = Op01WifiUtils.getTopTaskClass(context);
        return RESELECT_DIALOG_CLASSNAME.equals(classname);
    }

    public static boolean isTopReselectDialog(ActivityManager activityManager) {
        String classname = Op01WifiUtils.getTopTaskClass(activityManager);
        return RESELECT_DIALOG_CLASSNAME.equals(classname);
    }

    public static boolean isTopWifiSettings(ActivityManager activityManager) {
        String classname = getTopTaskClass(activityManager);
        return WIFISETTINGS_CLASSNAME.equals(classname);
    }

    private static String getTopTaskClass(Context context) {
        ActivityManager activityManager = (ActivityManager)
                context.getSystemService(Context.ACTIVITY_SERVICE);
        return getTopTaskClass(activityManager);
    }

    private static String getTopTaskClass(ActivityManager activityManager) {
        ComponentName cn = null;
        List<ActivityManager.RunningTaskInfo> runningTasks =
                activityManager.getRunningTasks(MAX_TASK_NUMBER);
        if (runningTasks != null && runningTasks.size() > 0 && runningTasks.get(0) != null) {
            cn = runningTasks.get(0).topActivity;
        }

        String classname = null;
        if (cn != null) {
            classname = cn.getClassName();
            log("[getTopTaskClass] Class Name:" + classname);
        } else {
            log("[getTopTaskClass] ComponentName is null!");
        }
        return classname;
    }

    public static boolean isSupplicantConnecting(StateChangeResult stateChangeResult) {
        SupplicantState state = null;
        try {
            Field field = StateChangeResult.class.getDeclaredField("state");
            field.setAccessible(true);
            state = (SupplicantState) field.get(stateChangeResult);
        } catch (Exception e) {
            e.printStackTrace();
        }
        return SupplicantState.isConnecting(state);
    }

    public static void registerTypeObserver(Context context, ContentObserver observer) {
        registerObserver(context, MtkSettingsExt.System.WIFI_CONNECT_TYPE, observer);
    }

    public static void registerRemindObserver(Context context, ContentObserver observer) {
        registerObserver(context, MtkSettingsExt.System.WIFI_CONNECT_REMINDER, observer);
    }

    private static void registerObserver(Context context, String name, ContentObserver observer) {
        ContentResolver resolver = context.getContentResolver();
        Uri uri = Settings.System.getUriFor(name);
        resolver.registerContentObserver(uri, false, observer);
    }

    public static int getConnectType(Context context) {
        return Settings.System.getInt(context.getContentResolver(),
                MtkSettingsExt.System.WIFI_CONNECT_TYPE,
                MtkSettingsExt.System.WIFI_CONNECT_TYPE_AUTO);
    }

    public static int getRemindType(Context context) {
        return Settings.System.getInt(context.getContentResolver(),
                MtkSettingsExt.System.WIFI_CONNECT_REMINDER,
                WIFI_CONNECT_REMINDER_ALWAYS);
    }

    public static  boolean isOutOfSuspend(long suspendTime) {
        long interval = System.currentTimeMillis() - suspendTime;
        return interval > SUSPEND_NOTIFICATION_DURATION;
    }

    private static void log(String message) {
        log(TAG, message);
    }

    public static void log(String TAG, String message) {
        if (DEBUG) {
            Log.d(LOG_PREFIX + "/" + TAG, message);
        }
    }
}
