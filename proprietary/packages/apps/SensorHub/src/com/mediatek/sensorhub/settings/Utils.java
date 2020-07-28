/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2019. All rights reserved.
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
package com.mediatek.sensorhub.settings;

import android.content.Context;
import android.content.SharedPreferences;
import android.hardware.Sensor;
import android.hardware.SensorManager;
import android.os.Environment;
import android.os.PowerManager;
import android.preference.Preference;
import android.preference.PreferenceScreen;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.SwitchPreference;
import android.text.format.DateFormat;
import android.util.Log;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import com.mediatek.sensorhub.ui.ButtonPreference;
import com.mediatek.sensorhub.ui.ButtonPreference.OnButtonClickCallback;

public class Utils {

    private static final String TAG = "SH/SensorHubUtils";
    // for add preference to screen @ {
    public static final int TYPE_PREFERENCE = 0;
    public static final int TYPE_SWITCH = 1;
    // @ }
    // For restore status in SharedPreference @
    public static final String SHARED_PREF_SENSOR_HUB = "shared_sensorhub";
    public static final String KEY_AUTO_TRIGGER_STATUS_SUFFIX = "_auto_trigger_status";
    public static final String KEY_NOTIFY_STATUS_SUFFIX = "_notify_status";
    public static final String KEY_SCREEN_ON = "_screen_on";
    public static final String KEY_TRIGGER_COUNT = "_trigger_count";

    // Pedometer status
    public static final String KEY_PRE_TOTAL_STEPS = "pre_total_steps";
    public static final String KEY_PRE_TOTAL_DISTANCE = "pre_total_distance";
    // Composite status
    public static final String KEY_COMPOSITE_STATUS = "composite_status";
    // Log status
    public static final String LOG_STATUS = "log_status";
    // Step counter
    public static final String KEY_PRE_TOTAL_STEP_COUNTER = "pre_total_step_counter";
    public static final String KEY_TOTAL_STEP_COUNTER = "total_step_counter";
    // Floor count
    public static final String KEY_PRE_TOTAL_FLOOR_COUNT = "pre_total_floor_count";
    public static final String KEY_TOTAL_FLOOR_COUNT = "total_floor_count";

    public static final String KEY_STRESS_FULL_TEST = "stress_full_test";
    public static final String KEY_STRESS_TEST = "stress_random_test";
    public static final String KEY_SUMMARY = "_summary";
    public static final String KEY_DURATION = "duration";

    public static final String KEY_PICK_UP_SUFFIX = "_pick_up";
    public static final String KEY_PUT_DOWN_SUFFIX = "_put_down";

    public static final String KEY_MAX_VALUE_SUFFIX = "_max";
    public static final String KEY_MIN_VALUE_SUFFIX = "_min";
    // @}

    public static final String ACTION_STEP_COUNTER_LOGGING = "action.step_counter_logging";
    // for Sensor @{
    public static final String[] compositeType = { Sensor.STRING_TYPE_GRAVITY,
            Sensor.STRING_TYPE_LINEAR_ACCELERATION, Sensor.STRING_TYPE_ROTATION_VECTOR,
            Sensor.STRING_TYPE_MAGNETIC_FIELD_UNCALIBRATED,
            Sensor.STRING_TYPE_GYROSCOPE_UNCALIBRATED,
            Sensor.STRING_TYPE_GEOMAGNETIC_ROTATION_VECTOR,
            Sensor.STRING_TYPE_ACCELEROMETER_UNCALIBRATED };
    public static final String[] TRIGGER_SENSOR_TYPES = { MtkSensor.STRING_TYPE_ANSWERCALL,
            MtkSensor.STRING_TYPE_FLAT, MtkSensor.STRING_TYPE_IN_POCKET,
            Sensor.STRING_TYPE_MOTION_DETECT,
            Sensor.STRING_TYPE_SIGNIFICANT_MOTION,
            Sensor.STRING_TYPE_STATIONARY_DETECT };
    public static final String[] CUSTOMER_SENSOR_TYPES = { Sensor.STRING_TYPE_ACCELEROMETER,
            Sensor.STRING_TYPE_GAME_ROTATION_VECTOR,
            Sensor.STRING_TYPE_GYROSCOPE,
            Sensor.STRING_TYPE_LIGHT,
            Sensor.STRING_TYPE_PRESSURE,
            Sensor.STRING_TYPE_PROXIMITY,
            Sensor.STRING_TYPE_STEP_DETECTOR,
            Sensor.STRING_TYPE_MAGNETIC_FIELD,
            Sensor.STRING_TYPE_ORIENTATION,
            MtkSensor.STRING_TYPE_DEVICE_ORIENTATION};
    public static final String[] orginalGestureType = { MtkSensor.STRING_TYPE_WAKE_GESTURE,
            MtkSensor.STRING_TYPE_GLANCE_GESTURE, MtkSensor.STRING_TYPE_PICK_UP_GESTURE };
    public static final String[] fusionSensorType = { Sensor.STRING_TYPE_GAME_ROTATION_VECTOR,
            Sensor.STRING_TYPE_ORIENTATION };
    private static String[] REGISTER_RATE_LATENCY = { "report_rate_", "report_latency_",
            "receive_times_" };
    private static SensorManager mSensorManager;
    private static List<Sensor> mSensorsList;
    // <SensorStringType, Sensor>
    private static HashMap<String, Sensor> mSensorKeyMap = new HashMap<String, Sensor>();
    // @}
    private static PowerManager.WakeLock sScreenOnWakeLock;
    // for log @{
    private static String mExternalStoragePath;
    // @}
    private static Utils sUtilsPlus;
    private static Context mContext;

    private Utils(Context context) {
        mContext = context;
        mSensorManager = (SensorManager) mContext.getSystemService(Context.SENSOR_SERVICE);
        mSensorsList = mSensorManager.getSensorList(Sensor.TYPE_ALL);
        fillSensorKeyMap();
        Log.d(TAG, "mSensorsList size : " + mSensorsList.size());
    }

    public static synchronized Utils getInstance(Context context) {
        if (sUtilsPlus == null) {
            Log.d(TAG, "new UtilsPlus");
            sUtilsPlus = new Utils(context);
        }
        return sUtilsPlus;
    }

    public static SensorManager getSensorManager() {
        return mSensorManager;
    }

    public static List<Sensor> getSensorsList() {
        return mSensorsList;
    }

    public static HashMap<String, Sensor> getSensorKeyMap() {
        return mSensorKeyMap;
    }

    private void fillSensorKeyMap() {
        if (mSensorsList != null && mSensorsList.size() != 0) {
            for (Sensor sensor : mSensorsList) {
                if (sensor != null) {
                    String sensorType = sensor.getStringType();
                    mSensorKeyMap.put(sensorType, sensor);
                }
            }
        }
    }

    public static void setReprotTime(String key, int rate, int latency, int times) {
        SharedPreferences.Editor editor = getSharedPreferences(mContext, SHARED_PREF_SENSOR_HUB)
                .edit();
        editor.putInt((REGISTER_RATE_LATENCY[0] + key), rate);
        editor.putInt((REGISTER_RATE_LATENCY[1] + key), latency);
        editor.putInt((REGISTER_RATE_LATENCY[2] + key), times);
        editor.apply();
    }

    public static void setReprotRate(String key, int rate) {
        SharedPreferences.Editor editor = getSharedPreferences(mContext, SHARED_PREF_SENSOR_HUB)
                .edit();
        editor.putInt((REGISTER_RATE_LATENCY[0] + key), rate);
        editor.apply();
    }

    public static void setReprotLatency(String key, int latency) {
        SharedPreferences.Editor editor = getSharedPreferences(mContext, SHARED_PREF_SENSOR_HUB)
                .edit();
        editor.putInt((REGISTER_RATE_LATENCY[1] + key), latency);
        editor.apply();
    }

    public static int getReprotRate(String key) {
        return getSharedPreferences(mContext, SHARED_PREF_SENSOR_HUB).getInt(
                (REGISTER_RATE_LATENCY[0] + key), 0);
    }

    public static int getReprotLatency(String key) {
        return getSharedPreferences(mContext, SHARED_PREF_SENSOR_HUB).getInt(
                (REGISTER_RATE_LATENCY[1] + key), 1000000);
    }

    public static int getReceiveDataTimes(String key) {
        return getSharedPreferences(mContext, SHARED_PREF_SENSOR_HUB).getInt(
                (REGISTER_RATE_LATENCY[2] + key), 0);
    }

    public static void setReceiveDataTimes(String key, int times) {
        SharedPreferences.Editor editor = getSharedPreferences(mContext, SHARED_PREF_SENSOR_HUB)
                .edit();
        editor.putInt((REGISTER_RATE_LATENCY[2] + key), times);
        editor.apply();
    }

    // For restore status in SharedPreference @{
    public static void setSensorStatus(String key, boolean status) {
        SharedPreferences.Editor editor = getSharedPreferences(mContext, SHARED_PREF_SENSOR_HUB)
                .edit();
        editor.putBoolean(key, status);
        editor.apply();
    }

    public static boolean getSensorStatus(String sensorType) {
        return getSharedPreferences(mContext, SHARED_PREF_SENSOR_HUB).getBoolean(sensorType, false);
    }

    public static void setSensorValues(String key, float value) {
        SharedPreferences.Editor editor = getSharedPreferences(mContext, SHARED_PREF_SENSOR_HUB)
                .edit();
        editor.putFloat(key, value);
        editor.apply();
    }

    public static float getSensorValues(String valueKey) {
        return getSharedPreferences(mContext, SHARED_PREF_SENSOR_HUB).getFloat(valueKey, 0.0f);
    }

    public static void setTriggerCount(String key, int count) {
        SharedPreferences.Editor editor = getSharedPreferences(mContext, SHARED_PREF_SENSOR_HUB)
                .edit();
        editor.putInt(key, count);
        editor.apply();
    }

    public static int getTriggerCount(String key) {
        return getSharedPreferences(mContext, SHARED_PREF_SENSOR_HUB).getInt(key, 0);
    }

    public static SharedPreferences getSharedPreferences(Context context, String name) {
        return context.getSharedPreferences(name, Context.MODE_PRIVATE);
    }

    public static void setSensorSummary(String key, String summary) {
        SharedPreferences.Editor editor = getSharedPreferences(mContext, SHARED_PREF_SENSOR_HUB)
                .edit();
        editor.putString(key + KEY_SUMMARY, summary);
        editor.apply();
    }

    public static String getSensorSummary(String key) {
        return getSharedPreferences(mContext, SHARED_PREF_SENSOR_HUB)
                .getString(key + KEY_SUMMARY, "");
    }

    public static void setTestDuration(String key, int duration) {
        SharedPreferences.Editor editor = getSharedPreferences(mContext, SHARED_PREF_SENSOR_HUB)
                .edit();
        editor.putInt(key + KEY_DURATION, duration);
        editor.apply();
    }

    public static int getTestDuration(String key) {
        return getSharedPreferences(mContext, SHARED_PREF_SENSOR_HUB).getInt(key + KEY_DURATION, 24*60);
    }


    public static void restoreStatusToDefult() {
        // restore log
        setSensorStatus(LOG_STATUS, false);

        // restore all sensors
        Utils.setSensorStatus(KEY_STRESS_FULL_TEST, false);
        for (Sensor sensor : mSensorsList) {
            String sensorTypeStr = sensor.getStringType();
            setSensorStatus(sensorTypeStr, false);
            boolean isTriggerSensor = (sensor.getReportingMode() == Sensor.REPORTING_MODE_ONE_SHOT);
            if (isTriggerSensor) {
                // restore notify & auto enabled status
                setSensorStatus(sensorTypeStr + KEY_AUTO_TRIGGER_STATUS_SUFFIX, false);
                setSensorStatus(sensorTypeStr + KEY_NOTIFY_STATUS_SUFFIX, false);
            }
        }
        // restore Pedometer
        setSensorValues(KEY_PRE_TOTAL_DISTANCE, 0.0f);
        setSensorValues(KEY_PRE_TOTAL_STEPS, 0.0f);
        // restore Step counter
        setSensorValues(KEY_PRE_TOTAL_STEP_COUNTER, 0.0f);
        setSensorValues(KEY_TOTAL_STEP_COUNTER, 0.0f);
        // restore Floor counter
        setSensorValues(KEY_PRE_TOTAL_FLOOR_COUNT, 0.0f);
        setSensorValues(KEY_TOTAL_FLOOR_COUNT, 0.0f);

        closeAllStressSensors();
    }

    private static void closeAllStressSensors() {
        Utils.setSensorStatus(KEY_STRESS_TEST, false);
        List<Sensor> sensorsList = Utils.getSensorsList();
        for (Sensor sensor : sensorsList) {
            String type = sensor.getStringType();
            String sharedStutusKeyString = KEY_STRESS_TEST + type;
            Utils.setSensorStatus(sharedStutusKeyString, false);
            Utils.setSensorSummary(sharedStutusKeyString, "");
        }
    }

    // @}

    // For record logs @{
    public static String initRecordFileName() {
        String sensorTestDirStr = Environment
                .getExternalStorageDirectory().getAbsolutePath() + "/MtkSensorTest/";
        File sensorTestDir = new File(sensorTestDirStr);
        boolean makeDir = false;
        if (!sensorTestDir.isDirectory()) {
            makeDir = sensorTestDir.mkdirs();
        }
        mExternalStoragePath = sensorTestDirStr + "sensor_test_" + getTimeString() + ".txt";
        Log.d(TAG, "mExternalStoragePath: " + mExternalStoragePath + " makeDir: " + makeDir);
        return mExternalStoragePath;
    }

    public static void recordToSdcard(byte[] data, String path) {
        FileOutputStream fos = null;
        try {
            if (path == null || path.equals("")) {
                path = initRecordFileName();
                Log.d(TAG, "path is null");
            }
            fos = new FileOutputStream(path, true);
            fos.write(data);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
            Log.w(TAG, "FileNotFoundException " + e.getMessage());
            return;
        } catch (IOException e) {
            Log.w(TAG, "IOException " + e.getMessage());
            return;
        } finally {
            try {
                if (fos != null) {
                    fos.flush();
                    fos.close();
                }
            } catch (IOException e2) {
                Log.w(TAG, "IOException " + e2.getMessage());
                return;
            }
        }
    }

    public static String getTimeString() {
        return DateFormat.format("yyyy-MM-dd-kk-mm-ss", System.currentTimeMillis()).toString();
    }

    // @}
    // For screen wake lock @{
    public static void acquireScreenWakeLock(Context context) {
        Log.d(TAG, "Acquiring screen on and cpu wake lock");
        if (sScreenOnWakeLock != null) {
            return;
        }

        PowerManager pm = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
        sScreenOnWakeLock = pm
                .newWakeLock(PowerManager.FULL_WAKE_LOCK | PowerManager.ACQUIRE_CAUSES_WAKEUP
                        | PowerManager.ON_AFTER_RELEASE, "SchPwrOnOff");
        sScreenOnWakeLock.acquire();
    }

    public static void releaseScreenWakeLock() {
        if (sScreenOnWakeLock != null) {
            sScreenOnWakeLock.release();
            sScreenOnWakeLock = null;
        }
    }

    // @}

    // for create preference @{
    public static Preference createPreference(int type, String title, String key,
            PreferenceScreen screen, Context context) {
        Preference preference = null;
        switch (type) {
        case TYPE_PREFERENCE:
            preference = new Preference(context);
            break;
        case TYPE_SWITCH:
            preference = new SwitchPreference(context);
            preference.setOnPreferenceChangeListener((OnPreferenceChangeListener) context);
            break;
        default:
            break;
        }
        preference.setKey(key);
        preference.setTitle(title);
        screen.addPreference(preference);
        return preference;
    }

    // @}

    // for create clear button preference
    public static ButtonPreference CreateClearButtonPreference(String key, String title,
        PreferenceScreen screen, OnButtonClickCallback callback, Context context) {
        ButtonPreference preference = new ButtonPreference(context, callback);
        preference.setKey(key);
        preference.setTitle(title);
        screen.addPreference(preference);
        return preference;
    }

    public static boolean isCompositeSesnor(String sensorType) {
        return containsSensor(compositeType, sensorType);
    }

    public static boolean isGestureSensor(String sensorType) {
        return containsSensor(orginalGestureType, sensorType);
    }
    public static boolean isTriggerSensor (String sensorType) {
        return containsSensor(TRIGGER_SENSOR_TYPES, sensorType);
    }
    public static boolean isCustomerSensor (String sensorType) {
        return containsSensor(CUSTOMER_SENSOR_TYPES, sensorType);
    }

    // Return whether the array is contains the sensor type
    public static boolean containsSensor(String[] sensorTypes, String type) {
        ArrayList<String> sensors = new ArrayList();
        for (int i = 0; i < sensorTypes.length; i++) {
            sensors.add(sensorTypes[i]);
        }
        return sensors.contains(type);
    }

    public static boolean getMultiSensorsStatus(String[] sensorTypes) {
        boolean status = false;
        for (String type : sensorTypes) {
            if (Utils.getSensorStatus(type)) {
                status = true;
                break;
            }
        }
        return status;
    }

    public static float getMinValue(String key, float def) {
        return getSharedPreferences(mContext, SHARED_PREF_SENSOR_HUB).getFloat(
                key + KEY_MIN_VALUE_SUFFIX, def);
    }
    public static float getMaxValue(String key, float def) {
        return getSharedPreferences(mContext, SHARED_PREF_SENSOR_HUB).getFloat(
                key + KEY_MAX_VALUE_SUFFIX, def);
    }

    public static int getAccCounter() {
        return getSharedPreferences(mContext, SHARED_PREF_SENSOR_HUB).getInt(
                "Acc_Counter", 1);
    }

    public static int getGyroCounter() {
        return getSharedPreferences(mContext, SHARED_PREF_SENSOR_HUB).getInt(
                "Gyro_Counter", 1);
    }

    public static int getMagCounter() {
        return getSharedPreferences(mContext, SHARED_PREF_SENSOR_HUB).getInt(
                "Mag_Counter", 1);
    }

    public static int getAlsCounter() {
        return getSharedPreferences(mContext, SHARED_PREF_SENSOR_HUB).getInt(
                "Als_Counter", 1);
    }

    /**
     * Forces the device to go to sleep.
     *
     * @param powerManager The PowerManager instance
     *
     * @param time The time when the request to go to sleep was issued, in the
     * {@link SystemClock#uptimeMillis()} time base.  This timestamp is used to correctly
     * order the go to sleep request with other power management functions.  It should be set
     * to the timestamp of the input event that caused the request to go to sleep.
     */
    public static void goToSleep(PowerManager powerManager, long time) {
        try {
            Class<?> c = powerManager.getClass();
            Method sleep = c.getMethod("goToSleep", new Class[]{long.class});
            sleep.invoke(powerManager,time);
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        } catch (InvocationTargetException e) {
            e.printStackTrace();
        } catch (NoSuchMethodException e) {
            e.printStackTrace();
        }
    }
}
