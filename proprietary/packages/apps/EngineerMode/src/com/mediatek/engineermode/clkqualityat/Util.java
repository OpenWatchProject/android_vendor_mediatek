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

package com.mediatek.engineermode.clkqualityat;

import android.app.ActivityManager;
import android.app.Notification;
import android.app.NotificationManager;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.graphics.Point;
import android.net.wifi.WifiManager;
import android.support.v4.content.LocalBroadcastManager;
import android.text.Html;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmApplication;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.ShellExe;
import com.mediatek.engineermode.bandselect.BandModeContent;
import com.mediatek.engineermode.wifi.EMWifi;
import com.mediatek.engineermode.wifi.WiFiStateManager;

import java.io.IOException;
import java.util.Arrays;
import java.util.List;

/**
 * Common functions.
 */
public class Util {
    public static final String TAG = "ClkQualityAT/Util";
    public static final int RETURN_SUCCESS = 0;
    private static String sWifiChipId = null;
    static final CharSequence TESTING = Html.fromHtml("<font color='#FFFF00'>Testing</font>");
    static final CharSequence PASS = Html.fromHtml("<font color='#00FF00'>PASS</font>");
    static final CharSequence VENIAL = Html.fromHtml("<font color='#FFFF00'>Warning</font>");
    static final CharSequence SERIOUS = Html.fromHtml("<font color='#FF0000'>Warning</font>");
    static final CharSequence FAIL = Html.fromHtml("<font color='#FF0000'>FAIL</font>");
    static final CharSequence CONN_FAIL = Html.fromHtml("<font color='#FF0000'>CONN FAIL</font>");
    static final CharSequence CNR_FAIL = Html.fromHtml("<font color='#FF0000'>CNR FAIL</font>");
    static final CharSequence MODEM_FAIL = Html.fromHtml("<font color='#FF0000'>MODEM FAIL</font>");
    private static final String FAIL_STRING = "FFFFFFFF";
    private static final int RETURN_FAIL = -1;
    private static final int ID_FINISH_NOTIFICATION = 10;
    static final long TIME_ONE_SEC = 1000;
    static final long TIME_THREE_SEC = 3000;
    static final long TIME_FIVE_SEC = 5000;
    private static final long FUNC_INDEX_WIFI_VERSION = 47;
    private static final long MASK_HIGH_16_BIT = 0xFFFF0000;
    private static final int BIT_16 = 16;
    private static int sWidth;
    private static int sHeight;

    static void setResolution(int x, int y) {
        Elog.i(TAG, "setResolution: " + x + "  " + y);
        sWidth = x;
        sHeight = y;
    }

    public static void sleep(long time) {
        try {
            Thread.sleep(time);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    static void sendAtCommand(Context context, String cmd, boolean isCDMA, int msgId) {
        Elog.i(TAG, "sendAtCommand. str: " + cmd);
        Intent intent = new Intent(ClkQualityAtActivity.ATC_SEND_ACTION);
        intent.putExtra(ClkQualityAtActivity.ATC_EXTRA_CMD, cmd);
        intent.putExtra(ClkQualityAtActivity.ATC_EXTRA_MODEM_TYPE, isCDMA);
        intent.putExtra(ClkQualityAtActivity.ATC_EXTRA_MSG_ID, msgId);
        LocalBroadcastManager.getInstance(context).sendBroadcast(intent);
    }

    static boolean isIntentAvailable(Context context, Intent intent) {
        final PackageManager packageManager = context.getPackageManager();
        List<ResolveInfo> list = packageManager.queryIntentActivities(intent,
                PackageManager.GET_ACTIVITIES);
        return list.size() > 0;
    }


    static void flightMode(Context context, boolean isOpen) {
        Elog.d(TAG, "flightMode start, switch: " + isOpen);

        BandTest.sAtcDone = false;
        if (!isOpen) {
            sendAtCommand(context, "AT+CFUN=1,1", false,
                    ClkQualityAtActivity.AtcMsg.REBOOT_LTE.getValue());
        } else {
            if (FeatureSupport.is93Modem()) {
                sendAtCommand(context, "AT+EFUN=0", false,
                        ClkQualityAtActivity.AtcMsg.FLIGHT_MODE.getValue());
            } else {
                sendAtCommand(context, "AT+EFUN=0", false,
                        ClkQualityAtActivity.AtcMsg.FLIGHT_MODE.getValue());
                while (true) {
                    sleep(1000);
                    if (BandTest.sAtcDone) {
                        Elog.d(TAG, "@Util.flightMode(), BandTest.sAtcDone: true");
                        break;
                    }else{
                        Elog.d(TAG, "@Util.flightMode(), BandTest.sAtcDone: false");
                        break;
                    }
                }

                if (ModemCategory.isCdma()) {
                    BandTest.sAtcDone = false;
                    sendAtCommand(context, "AT+CPOF", true,
                            ClkQualityAtActivity.AtcMsg.FLIGHT_MODE_CDMA.getValue());
                }
            }
        }

        while (true) {
            sleep(1000);
            if (BandTest.sAtcDone) {
                Elog.d(TAG, "@Util.flightMode(), BandTest.sAtcDone: true");
                break;
            }else{
                Elog.d(TAG, "@Util.flightMode(), BandTest.sAtcDone: false");
                break;
            }
        }


        if (!isOpen) {
            sleep(10 * 1000);
            Elog.d(TAG, "flightMode switch end, " + isOpen);
            return;
        }

        Elog.d(TAG, "flightMode switch end, " + isOpen);
    }

    static void clickPoint(Point point, boolean landscape) {

        Point clickPoint = getClickPoint(point, landscape);
        String cmd = "input tap " + clickPoint.x + " " + clickPoint.y;
        try {
            ShellExe.execCommand(cmd, true);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    static void enableWifi(Context context) {
        Elog.d(TAG, "enableWifi");
        new WiFiStateManager(context).checkState(context);
    }

    static void disableWifi(Context context) {
        Elog.d(TAG, "disableWifi");
        new WiFiStateManager(context).uninitWifiTest();
    }

    static void pressBack() {
        try {
            ShellExe.execCommand("input keyevent 4", true);
        } catch (IOException e) {
            e.printStackTrace();
        }
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

    static void notifyFinish(Context context) {

        NotificationManager nm = (NotificationManager)
                context.getSystemService(Context.NOTIFICATION_SERVICE);

        Notification notification = new Notification.Builder(context,
                EmApplication.getSoundNotificationChannelID())
                .setSmallIcon(android.R.drawable.ic_dialog_info)
                .setContentText(context.getResources().
                        getString(R.string.clk_quality_at_finish_notification))
                .setContentTitle(context.getResources().getString(R.string.clk_quality_at))
                .setAutoCancel(true)
                .build();
        notification.flags |= Notification.FLAG_INSISTENT;
        nm.notify(ID_FINISH_NOTIFICATION, notification);
    }

    static long[] getSupportedBand(String[] strInfo) {
        long[] values = new long[TestItem.INDEX_BAND_MAX];
        for (int i = 0; i < TestItem.INDEX_BAND_MAX; ++i)
            values[i] = 0;
        for (final String value : strInfo) {
            Elog.i(TAG, "getSupportedBand strInfo: " + value);
            if (!value.substring(0, ClkQualityAtActivity.SAME_COMMAND.length())
                    .equals(ClkQualityAtActivity.SAME_COMMAND))
                continue;
            final String splitString = value.substring(ClkQualityAtActivity.SAME_COMMAND.length());
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

    static boolean isTopActivity(Context context, String classname) {
        ActivityManager ams = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
        ComponentName cn = ams.getRunningTasks(1).get(0).topActivity;
        Elog.i(TAG, "cn.getClassName(): " + cn.getClassName());
        Elog.i(TAG, "classname:" + classname);
        return cn.getClassName().equals(classname);

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

    static void switchAirplaneMode(Context context, boolean on) {
        EmUtils.setAirplaneModeEnabled(on);
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

}
