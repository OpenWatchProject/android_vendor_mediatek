/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2013. All rights reserved.
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

package com.mediatek.providers.drm;

import static com.mediatek.providers.drm.OmaDrmHelper.DEBUG;

import android.bluetooth.BluetoothAdapter;
import android.content.Context;
import android.drm.DrmInfo;
import android.drm.DrmInfoRequest;
import android.drm.DrmManagerClient;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Build;
import android.os.SystemProperties;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;

import java.io.FileInputStream;
import java.io.IOException;
import java.io.UnsupportedEncodingException;

import com.mediatek.omadrm.OmaDrmInfoRequest;
import com.mediatek.omadrm.OmaDrmStore;

/**
 * OMA DRM utility class
 */
public class OmaDrmHelper {
    private static final String TAG = "DRM/OmaDrmHelper";
    public static final boolean DEBUG = Log.isLoggable("DrmProvider", Log.DEBUG)
            || !"user".equals(Build.TYPE);
    private static final String INVALID_DEVICE_ID = "000000000000000";
    private static final String EMPTY_STRING = "";

    // Define actions
    public static final String KEY_ACTION = "action";
    public static final String ACTION_SYNC_SECURE_TIME = "sync_secure_timer";
    public static final String ACTION_UPDATE_TIME_OFFSET = "update_time_offset";

    /**
     * Update secure time offset
     * @param client The OMA DRM client
     * @return the status of updating offset. ERROR_NONE for success, ERROR_UNKOWN for failed.
     */
    public static boolean updateOffset(DrmManagerClient client) {
        DrmInfoRequest request = new DrmInfoRequest(DrmInfoRequest.TYPE_SET_OMA_DRM_INFO,
                OmaDrmStore.DrmObjectMimeType.MIME_TYPE_DRM_CONTENT);
        request.put(OmaDrmInfoRequest.KEY_ACTION, OmaDrmInfoRequest.ACTION_UPDATE_OFFSET);
        Log.d(TAG, "client.acquireDrmInfo OmaDrmHelper 1 ");
        DrmInfo info = client.acquireDrmInfo(request);
        String message = getStringFromDrmInfo(info);
        if (DEBUG) Log.d(TAG, "updateOffset: " + message);
        return OmaDrmInfoRequest.DrmRequestResult.RESULT_SUCCESS.equals(message);
    }

    /**
     * Check the secure timer is valid in drmserver.
     *
     * @param client DrmManagerClient.
     * @return Valid return true, otherwise false.
     */
    public static boolean checkSecureTime(DrmManagerClient client) {
        DrmInfoRequest request = new DrmInfoRequest(DrmInfoRequest.TYPE_GET_OMA_DRM_INFO,
                OmaDrmStore.DrmObjectMimeType.MIME_TYPE_DRM_CONTENT);
        request.put(OmaDrmInfoRequest.KEY_ACTION, OmaDrmInfoRequest.ACTION_CHECK_SECURE_TIME);
        Log.d(TAG, "client.acquireDrmInfo OmaDrmHelper 2 ");
        DrmInfo info = client.acquireDrmInfo(request);
        String message = getStringFromDrmInfo(info);
        if (DEBUG) Log.d(TAG, "checkSecureTime: " + message);
        return message.equals("valid");
    }

    /**
     * Save secure time to drmserver, use offset between NTP time and device time to store
     * oma drm secure time.
     *
     * @param client DrmManagerClient.
     * @param offset Offset between NTP time and device time
     * @return Save success, return true, otherwise false.
     */
    public static boolean saveSecureTime(DrmManagerClient client, long offset) {
        DrmInfoRequest request = new DrmInfoRequest(DrmInfoRequest.TYPE_SET_OMA_DRM_INFO,
                OmaDrmStore.DrmObjectMimeType.MIME_TYPE_DRM_CONTENT);
        request.put(OmaDrmInfoRequest.KEY_ACTION, OmaDrmInfoRequest.ACTION_SAVE_SECURE_TIME);
        request.put(OmaDrmInfoRequest.KEY_DATA, String.valueOf(offset));
        Log.d(TAG, "client.acquireDrmInfo OmaDrmHelper 3 ");
        DrmInfo info = client.acquireDrmInfo(request);
        String message = getStringFromDrmInfo(info);
        if (DEBUG) Log.d(TAG, "saveSecureTime: " + message);
        return OmaDrmInfoRequest.DrmRequestResult.RESULT_SUCCESS.equals(message);
    }

    /**
     * Check device id stored, if it's invalid, need get from system.
     *
     * @param client The OMA DRM Client
     * @param deviceId The device id to be saved in file
     */
    public static void saveDeviceIdIfNeed(Context context, DrmManagerClient client) {
        String deviceId = loadDeviceId(client);
        if (TextUtils.isEmpty(deviceId) || deviceId.equals(INVALID_DEVICE_ID)) {
            deviceId = getDeviceIdFromSystem(context);
            saveDeviceId(client, deviceId);
        }
    }

    /**
     *
     * @param client The OMA DRM Client
     * @return the device id
     */
    private static String loadDeviceId(DrmManagerClient client) {
        DrmInfoRequest request = new DrmInfoRequest(DrmInfoRequest.TYPE_GET_OMA_DRM_INFO,
                OmaDrmStore.DrmObjectMimeType.MIME_TYPE_DRM_CONTENT);
        request.put(OmaDrmInfoRequest.KEY_ACTION,
                OmaDrmInfoRequest.ACTION_LOAD_DEVICE_ID);
        Log.d(TAG, "client.acquireDrmInfo OmaDrmHelper 4 ");
        DrmInfo info = client.acquireDrmInfo(request);
        String deviceId = getStringFromDrmInfo(info);  // 16 byte
        if (DEBUG) Log.d(TAG, "loadDeviceId: " + deviceId);
        return deviceId;
    }

    private static boolean saveDeviceId(DrmManagerClient client, String deviceId) {
        DrmInfoRequest request = new DrmInfoRequest(DrmInfoRequest.TYPE_SET_OMA_DRM_INFO,
                OmaDrmStore.DrmObjectMimeType.MIME_TYPE_DRM_CONTENT);
        request.put(OmaDrmInfoRequest.KEY_ACTION, OmaDrmInfoRequest.ACTION_SAVE_DEVICE_ID);
        request.put(OmaDrmInfoRequest.KEY_DATA, deviceId);
        Log.d(TAG, "client.acquireDrmInfo OmaDrmHelper 5 ");
        DrmInfo info = client.acquireDrmInfo(request);
        String message = getStringFromDrmInfo(info);
        if (DEBUG) Log.d(TAG, "saveDeviceId: " + message);
        return OmaDrmInfoRequest.DrmRequestResult.RESULT_SUCCESS.equals(message);
    }

    /**
     * Get device id from system, use imei, wifi mac address, bluetooth mac address as it.
     *
     * @param context Context
     * @return device id
     */
    private static String getDeviceIdFromSystem(Context context) {
        String deviceId;
        // 1. First use IMEI
        deviceId = getImei(context);
        // 2. Second choose wifi mac address
        if (TextUtils.isEmpty(deviceId)) {
            deviceId = getWifiMacAddress(context);
        }
        // 3, Third choose bluetooth mac address
        if (TextUtils.isEmpty(deviceId)) {
            deviceId = getBluetoothMacAddress();
        }
        if (DEBUG) Log.d(TAG, "getDeviceIdFromSystem: " + deviceId);
        return deviceId;
    }

    // IMEI if GSM, a valid MEID or ESN if CDMA
    private static String getImei(Context context) {
        TelephonyManager tm = (TelephonyManager) context.getSystemService(
                Context.TELEPHONY_SERVICE);
        String imei = tm.getDeviceId();
        if (DEBUG) Log.d(TAG, "getImei: " + imei);
        return imei;
    }

    // Only when wifi system is enabled, it can get wifi mac address
    private static String getWifiMacAddress(Context context) {
        WifiManager wifiManager = (WifiManager) context.getSystemService(Context.WIFI_SERVICE);
        WifiInfo wifiInfo = wifiManager.getConnectionInfo();
        String macAddress = wifiInfo == null ? null : wifiInfo.getMacAddress();
        if (DEBUG) Log.d(TAG, "getWifiMacAddress: " + macAddress);
        return macAddress;
    }

    private static String getBluetoothMacAddress() {
        BluetoothAdapter adapter = BluetoothAdapter.getDefaultAdapter();
        String macAddress = adapter == null ? null : adapter.getAddress();
        if (DEBUG) Log.d(TAG, "getBluetoothMacAddress: " + macAddress);
        return macAddress;
    }

    /**
     * Check whether network is connected.
     *
     * @param context A context to get system service
     * @return True if there is a valid network connection, whether it is via WiFi,
     *          mobile data or other means.
     */
    public static boolean isNetworkConnected(Context context) {
        if (context == null) {
            return false;
        }
        final ConnectivityManager cm = (ConnectivityManager) context.getSystemService(
                Context.CONNECTIVITY_SERVICE);
        if (cm == null) {
            return false;
        }
        final NetworkInfo info = cm.getActiveNetworkInfo();

        if (DEBUG) Log.d(TAG, "isNetworkConnected: NetworkInfo = " + info);
        return info != null && info.isConnected();
    }

    /**
     * Check whether the insert Icc Card is test card or not
     *
     * @return  return true if the Icc Card is test card
     */
    public static boolean isTestIccCard() {
        int ret1 = SystemProperties.getInt("vendor.gsm.sim.ril.testsim", 0);
        int ret2 = SystemProperties.getInt("vendor.gsm.sim.ril.testsim.2", 0);
        int ret3 = SystemProperties.getInt("vendor.gsm.sim.ril.testsim.3", 0);
        int ret4 = SystemProperties.getInt("vendor.gsm.sim.ril.testsim.4", 0);
        int result = (ret1 | ret2 | ret3 | ret4);
        if (DEBUG) Log.d(TAG, "isTestIccCard: " + ret1 + "," + ret2 + "," + ret3 + "," + ret4);
        return result == 1;
    }

    /**
     * Add to workaround CT test case to disable send ntp package. need user push a special
     * file to phone storage, when we check this file exist with right value, disable sync
     * secure time.<p>
     * use below command to enable this function:<p>
     * adb shell "echo 1 > sdcard/.omadrm"
     *
     * @return true if request disable sync secure time
     */
    public static boolean isRequestDisableSyncSecureTime() {
        boolean isRequest = false;
        FileInputStream inputStream = null;
        try {
            inputStream = new FileInputStream("sdcard/.omadrm");
            int flag = inputStream.read();
            if (DEBUG) Log.d(TAG, "flag: " + flag);
            // 49 means char '1'
            if (flag == 49) {
                isRequest = true;
            }
        } catch (IOException e) {
            Log.w(TAG, "Read file fail with ", e);
            isRequest = false;
        } finally {
            if (inputStream != null) {
                try {
                    inputStream.close();
                    inputStream = null;
                } catch (IOException e) {
                    Log.w(TAG, "close FileInputStream with IOException ", e);
                }
            }
        }
        Log.d(TAG, "isRequestDisableSyncSecureTimer: " + isRequest);
        return isRequest;
    }

    private static String getStringFromDrmInfo(DrmInfo info) {
        String message = EMPTY_STRING;
        if (info == null) {
            Log.e(TAG, "getStringFromDrmInfo info is null");
            return message;
        }
        byte[] data = info.getData();
        if (null != data) {
            try {
                // the information shall be in format of ASCII string
                message = new String(data, "US-ASCII");
            }
            catch (UnsupportedEncodingException e) {
                Log.e(TAG, "Unsupported encoding type of the returned DrmInfo data");
                message = EMPTY_STRING;
            }
        }
        Log.v(TAG, "getStringFromDrmInfo: " + message);
        return message;
    }
}
