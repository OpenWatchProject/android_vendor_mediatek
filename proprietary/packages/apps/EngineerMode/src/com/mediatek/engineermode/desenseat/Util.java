/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * public under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.engineermode.desenseat;

import android.annotation.TargetApi;
import android.app.ActivityManager;
import android.app.Notification;
import android.app.NotificationManager;
import android.bluetooth.BluetoothAdapter;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.graphics.Point;
import android.hardware.Camera.CameraInfo;
import android.net.wifi.WifiManager;
import android.os.Build;
import android.os.Environment;
import android.os.PowerManager;
import android.os.storage.StorageManager;
import android.os.storage.StorageVolume;
import android.provider.Settings;
import android.text.Html;

import com.mediatek.engineermode.ChipSupport;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmApplication;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.ShellExe;
import com.mediatek.engineermode.bandselect.BandModeContent;
import com.mediatek.engineermode.bluetooth.BtTest;
import com.mediatek.engineermode.wifi.EMWifi;
import com.mediatek.engineermode.wifi.WiFiStateManager;

import java.io.File;
import java.io.IOException;
import java.util.Arrays;
import java.util.List;

/**
 * Common functions.
 */
public class Util {
    public static final String TAG = "DesenseAT/Util";

    private static final String FAIL_STRING = "FFFFFFFF";
    private static final String OPERATION_ERROR_PREFIX = "#$ERROR^&";
    private static String sWifiChipId = null;
    static final String HISTORY_PATH = "desense_at_history_data";
    private static final int RETURN_FAIL = -1;
    static final int RETURN_SUCCESS = 0;
    private static final int FB0_LCM_POWER_ON = 4;
    private static final int FB0_LCM_POWER_OFF = 5;
    private static final int ID_FINISH_NOTIFICATION = 10;
    private static final long FUNC_INDEX_WIFI_VERSION = 47;
    private static final long MASK_HIGH_16_BIT = 0xFFFF0000;
    private static final int BIT_16 = 16;
    static final long TIME_ONE_SEC = 1000;
    static final long TIME_THREE_SEC = 3000;
    static final long TIME_FIVE_SEC = 5000;
    static final long TIME_TEN_SEC = 10000;
    static final long TIME_FIFTEEN_SEC = 10;
    static final long TIME_THIRTY_SEC = 10;
    static final long TIME_TEN_MILI_SEC = 10;
    static final long TIME_HUNDRED_MILI_SEC = 100;
    static final CharSequence TESTING = Html.fromHtml("<font color='#FFFF00'>Testing</font>");
    static final CharSequence PASS = Html.fromHtml("<font color='#00FF00'>PASS</font>");
    static final CharSequence FAIL = Html.fromHtml("<font color='#FF0000'>FAIL</font>");
    static final CharSequence CONN_FAIL = Html.fromHtml("<font color='#FF0000'>CONN FAIL</font>");
    static final CharSequence CNR_FAIL = Html.fromHtml("<font color='#FF0000'>CNR FAIL</font>");
    static final CharSequence MODEM_FAIL = Html.fromHtml("<font color='#FF0000'>MODEM FAIL</font>");
    static final String INVALID_DATA = "invalid";
    static final int INVALID_SV_ID = 0;
    private static int sWidth;
    private static int sHeight;

    static void setResolution(int x, int y) {
        Elog.i(TAG, "setResolution: " + x + "  " + y);
        sWidth = x;
        sHeight = y;
    }

    static void sleep(long time) {
        // TODO: not exactly
        try {
            Thread.sleep(time);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    static void sendAtCommand(Context context, String cmd, boolean isCDMA, int msgId) {
        Elog.i(TAG, "sendAtCommand. cmd: " + cmd + " isCDMA: " + isCDMA);
        Intent intent = new Intent(context, ATCSenderReceiver.class);
        intent.setAction(ATCSenderReceiver.ATC_SEND_ACTION);
        intent.putExtra(ATCSenderReceiver.ATC_EXTRA_CMD, cmd);
        intent.putExtra(ATCSenderReceiver.ATC_EXTRA_MODEM_TYPE, isCDMA);
        intent.putExtra(ATCSenderReceiver.ATC_EXTRA_MSG_ID, msgId);
        context.sendBroadcast(intent);
    }

    static boolean isIntentAvailable(Context context, Intent intent) {
        final PackageManager packageManager = context.getPackageManager();
        List<ResolveInfo> list = packageManager.queryIntentActivities(intent,
                PackageManager.GET_ACTIVITIES);
        return list.size() > 0;
    }

    static boolean isPkgInstalled(Context context, String pkgName) {
        PackageManager pkgManager = context.getPackageManager();
        try {
            pkgManager.getPackageInfo(pkgName, PackageManager.GET_GIDS);
            return true;
        } catch (PackageManager.NameNotFoundException exception) {
            return false;
        }
    }

    static boolean isCameraSupport(boolean front) {
        int availCameraNumber = android.hardware.Camera.getNumberOfCameras();
        for (int i = 0; i < availCameraNumber; i++) {
            CameraInfo info = new CameraInfo();
            android.hardware.Camera.getCameraInfo(i, info);
            if (info.facing == (front ? CameraInfo.CAMERA_FACING_FRONT :
                CameraInfo.CAMERA_FACING_BACK)) {
                return true;
            }
        }

        return false;
    }
    static long getSupportedBandCdma(String[] bandStr) {
        Elog.d(TAG, "query SupportedBandCdma get string array: " + Arrays.toString(bandStr));
        String strRes = bandStr[0];
        if ((strRes != null) && (strRes.contains(BandModeContent.SAME_COMMAND_CDMA))) {
            String splitString = strRes.substring(BandModeContent.SAME_COMMAND_CDMA.length());
            final String[] getDigitalVal = splitString.split(",");
            long[] value = new long[2];
            try {
                for (int i = 0; i < 2; i++) {
                    if (getDigitalVal[i] != null) {
                        value[i] = Integer.parseInt(getDigitalVal[i].substring(2), 16);
                    }
                }
            } catch (NumberFormatException e) {
                value[0] = 0;
            }
            Elog.d(TAG, "getSupportedBandCdma return: " + value[0]);
            return value[0];
        } else {
            //Default band 0
            return 1;
        }

    }
    static long[] getSupportedBand(String[] strInfo) {
        long[] values = new long[TestItem.INDEX_BAND_MAX];
        for (int i = 0; i < TestItem.INDEX_BAND_MAX; ++i)
            values[i] = 0;
        for (final String value : strInfo) {
            Elog.i(TAG, "getSupportedBand strInfo: " + value);
            if (!value.substring(0, DesenseAtActivity.SAME_COMMAND.length())
                    .equals(DesenseAtActivity.SAME_COMMAND))
                continue;
            final String splitString = value.substring(DesenseAtActivity.SAME_COMMAND.length());
            final String[] getDigitalVal = splitString.split(",");
            if (getDigitalVal != null && getDigitalVal.length > 1) {
                for (int i = 0; i < values.length; i++) {
                    if (i >= getDigitalVal.length || getDigitalVal[i] == null) {
                        values[i] = 0;
                        continue;
                    }
                    try {
                        values[i] = Long.valueOf(getDigitalVal[i].trim());
                        Elog.i(TAG, "getSupportedBand #" + i + ": " + values[i]);
                    } catch (NumberFormatException e) {
                        values[i] = 0;
                    }
                }
                break;
            }
        }
        return values;
    }
    static void flightMode(Context context, boolean isOpen) {
        Elog.d(TAG, "flightMode start, switch: " + isOpen);

        BandTest.sAtcDone = false;
        if (!isOpen) {
            sendAtCommand(context, "AT+CFUN=1,1", false, 0);
        } else {
            if (FeatureSupport.is93Modem()) {
                sendAtCommand(context, "AT+EFUN=0", false, 0);
            } else {
                sendAtCommand(context, "AT+CFUN=4", false, 0);
                while (true) {
                    sleep(TIME_TEN_MILI_SEC * 100);
                    if (BandTest.sAtcDone) {
                        Elog.d(TAG, "@Util.flightMode(), BandTest.sAtcDone: true");
                        break;
                    } else {
                        Elog.d(TAG, "@Util.flightMode(), BandTest.sAtcDone: false");
                        break;
                    }
                }

                if (ModemCategory.isCdma()) {
                    BandTest.sAtcDone = false;
                    sendAtCommand(context, "AT+CPOF", true, 0);
                }
            }
        }

        while (true) {
            if (BandTest.sAtcDone) {
                Elog.d(TAG, "@Util.flightMode(), BandTest.sAtcDone: true");
                break;
            }
            sleep(TIME_TEN_MILI_SEC);
        }
        if (!isOpen) {
            sleep(TIME_TEN_SEC);
            Elog.d(TAG, "flightMode switch end, " + isOpen);
            return ;
        }

        Elog.d(TAG, "flightMode switch end, " + isOpen);
    }

    static void switchAirplaneMode(Context context, boolean on) {
        EmUtils.setAirplaneModeEnabled(on);
    }


    static WakeLock sWakeLock;


    static void clickPoint(Point point, boolean landscape) {

        Point clickPoint = getClickPoint(point, landscape);
        String cmd = "input tap " + clickPoint.x + " " + clickPoint.y;
        try {
            ShellExe.execCommand(cmd, true);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    /**
     * Wake lock manager class.
     *
     */
    static class WakeLock {
        private PowerManager.WakeLock mScreenWakeLock = null;
        private PowerManager.WakeLock mCpuWakeLock = null;

        /**
         * Acquire CPU wake lock.
         *
         * @param context
         *            Global information about an application environment
         */
        void acquireCpuWakeLock(Context context) {
            Elog.v(TAG, "Acquiring cpu wake lock");
            if (mCpuWakeLock != null) {
                return;
            }

            PowerManager pm = (PowerManager) context
                    .getSystemService(Context.POWER_SERVICE);

            mCpuWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK
                    | PowerManager.ACQUIRE_CAUSES_WAKEUP, TAG);
            // | PowerManager.ON_AFTER_RELEASE, TAG);
            mCpuWakeLock.acquire();
        }

        /**
         * Acquire wake lock.
         *
         * @param context
         *            Global information about an application environment
         */
        void acquire(Context context) {
            acquireCpuWakeLock(context);
        }

        /**
         * Release wake lock.
         */
        void release() {
            Elog.v(TAG, "Releasing wake lock");
            if (mCpuWakeLock != null) {
                mCpuWakeLock.release();
                mCpuWakeLock = null;
            }
            if (mScreenWakeLock != null) {
                mScreenWakeLock.release();
                mScreenWakeLock = null;
            }
        }
    }

    static void enableWifiEm(Context context) {
        Elog.d(TAG, "enableWifiEm");
        new WiFiStateManager(context).checkState(context);
    }

    static void disableWifiEm(Context context) {
        Elog.d(TAG, "disableWifiEm");
        new WiFiStateManager(context).uninitWifiTest();
    }

    static String getWifiChipId() {
        if (sWifiChipId == null) {
            long[] version = new long[2];
            int result = EMWifi.getATParam(FUNC_INDEX_WIFI_VERSION, version);
            if (result == 0) {
                sWifiChipId = Long.toHexString((version[0] & MASK_HIGH_16_BIT) >> BIT_16);
            }
        }
        Elog.d(TAG, "sWifiChipId = " + sWifiChipId);
        return sWifiChipId;
    }

    static void switchWifi(Context context, boolean on) {
        WifiManager wifiManager = (WifiManager) context.getSystemService(Context.WIFI_SERVICE);
        if (wifiManager != null) {
            int targetState = on ? WifiManager.WIFI_STATE_ENABLED : WifiManager.WIFI_STATE_DISABLED;
            if (targetState != wifiManager.getWifiState()) {
                wifiManager.setWifiEnabled(on);
                for (int k = 0; k < 15; k++) {
                    if (targetState != wifiManager.getWifiState()) {
                        sleep(TIME_ONE_SEC);
                    } else {
                        Elog.i(TAG, "succeed to switchWifi " + on);
                        return;
                    }
                }
                Elog.i(TAG, "fail to switchWifi " + on);
            }
        }
    }


    static boolean switchBt(boolean on) {
        Elog.d(TAG, "switchBt: " + on);
        int targetState = on ? BluetoothAdapter.STATE_ON : BluetoothAdapter.STATE_OFF;
        BluetoothAdapter adapter = BluetoothAdapter.getDefaultAdapter();
        if (targetState != adapter.getState()) {
            if (on) {
                adapter.enable();
            } else {
                adapter.disable();
            }
            for (int k = 0; k < 15; k++) {
                if (targetState != adapter.getState()) {
                    sleep(TIME_ONE_SEC);
                } else {
                    Elog.i(TAG, "succeed to switchBt " + on);
                    return true;
                }
            }
            Elog.i(TAG, "fail to switchBt " + on);
            return false;
        }
        return true;

    }

    static boolean disableBtEm() {
        Elog.d(TAG, "disableBtEm");
        return (new BtTest().unInit() == RETURN_SUCCESS);
    }

    static boolean enableBtEm() {
        Elog.d(TAG, "enableBtEm");
        return (new BtTest().init() == RETURN_SUCCESS);
    }

    static boolean lcmOff() {
        Elog.d(TAG, "lcmOff");
        return (ChipSupport.lcmOff() == RETURN_SUCCESS);
    }

    static boolean lcmOn() {
        Elog.d(TAG, "lcmOn");
        return (ChipSupport.lcmOn() == RETURN_SUCCESS);

    }

    static boolean backlightOff() {

        Elog.d(TAG, "backlightOff");

        PowerManager powerMgr = (PowerManager) EmApplication.getContext().
                                         getSystemService(Context.POWER_SERVICE);
        int min = powerMgr.getMinimumScreenBrightnessSetting();
        Elog.d(TAG, "min :" + min);
        Settings.System.putInt(EmApplication.getContext().getContentResolver(),
                                  Settings.System.SCREEN_BRIGHTNESS, min);
        return true;
    }

    static boolean backlightOn() {

        Elog.d(TAG, "backlightOn");

        PowerManager powerMgr = (PowerManager) EmApplication.getContext().
                                         getSystemService(Context.POWER_SERVICE);
        int max = powerMgr.getMaximumScreenBrightnessSetting();
        Elog.d(TAG, "max :" + max);
        Settings.System.putInt(EmApplication.getContext().getContentResolver(),
                                  Settings.System.SCREEN_BRIGHTNESS, max);

        return true;

    }


    static boolean isFileExist(String filepath) {
        if (filepath == null) {
            return false;
        }
        File file = new File(filepath);
        return (file != null && file.exists());
    }

    static String getTestFilePath(Context context, String filepath) {
        String path = Environment.getExternalStorageDirectory().getPath() +  "/" + filepath;
        Elog.i(TAG, "path :" + path);
        return path;
    }

    private static String getStoragePath(Context context, int index) {
        //return Environment.getExternalStorageDirectory().getPath();
        StorageVolume sv = getMountedVolumeById(context, index);
        if (sv != null) {
             Elog.i(TAG, "sv.getPath() :" + sv.getPath());
            return sv.getPath();
        }
        return null;
    }

    @TargetApi(Build.VERSION_CODES.GINGERBREAD)
    private static StorageVolume getMountedVolumeById(Context context, int index) {
        StorageManager storageMgr = (StorageManager) context.
                getSystemService(Context.STORAGE_SERVICE);
        StorageVolume[] volumes = storageMgr.getVolumeList();
        int mountedIndx = 0;
        for (int i = 0; i < volumes.length; i++) {
            String path = volumes[i].getPath();
            String state = storageMgr.getVolumeState(path);
            if (state.equals(Environment.MEDIA_MOUNTED)) {
                if (mountedIndx == index) {
                    return volumes[i];
                }
                mountedIndx++;
            }
        }
        return null;
    }

    static Point getClickPoint(Point p720p, boolean landscape) {
        if (landscape) {
            Point point = new Point();
            point.x = sHeight * p720p.x / 1280;
            point.y = sWidth - sWidth * p720p.y / 720;
            return point;
        } else {
            Point point = new Point();
            point.x = sWidth * p720p.x / 720;
            point.y = sHeight * p720p.y / 1280;
            return point;
        }

    }

    static void pressBack() {
        try {
            ShellExe.execCommand("input keyevent 4", true);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    static boolean isTopActivity(Context context, String classname) {
        ActivityManager ams = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
        ComponentName cn = ams.getRunningTasks(1).get(0).topActivity;
        Elog.i(TAG, "cn.getClassName(): " + cn.getClassName());
        Elog.i(TAG, "classname:" + classname);
        return cn.getClassName().equals(classname);

    }


    static void notifyFinish(Context context) {

        NotificationManager nm = (NotificationManager)
                context.getSystemService(Context.NOTIFICATION_SERVICE);
        Notification notification = new Notification.Builder(context,
                EmApplication.getSoundNotificationChannelID())
                .setSmallIcon(android.R.drawable.ic_dialog_info)
                .setContentText(context.getResources().
                        getString(R.string.desense_at_finish_notification))
                .setContentTitle(context.getResources().getString(R.string.desense_at))
                .setAutoCancel(true)
                .build();
        notification.flags |= Notification.FLAG_INSISTENT;
        nm.notify(ID_FINISH_NOTIFICATION, notification);
    }


}
