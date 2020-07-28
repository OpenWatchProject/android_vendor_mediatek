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
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.sensorhub.ui;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.hardware.Sensor;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.SystemClock;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.Preference.OnPreferenceClickListener;
import android.preference.SwitchPreference;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemLongClickListener;
import android.widget.EditText;
import android.widget.CompoundButton;
import android.widget.ListView;
import android.widget.Switch;

import com.mediatek.sensorhub.settings.Utils;
import com.mediatek.sensorhub.settings.MtkSensor;

import java.util.Arrays;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

public class SensorStressFullTestActivity extends BaseActivity implements OnPreferenceChangeListener,
        OnSharedPreferenceChangeListener, OnItemLongClickListener {

    public static final String TAG = "SH/SensorStressFullTest";
    private static final long TIME_OUT = 2000;
    private static final String ENTER_STR = "\n";
    // trigger senseor
    public static final String KEY_ENABLE_NOTIFY = "_enable_notify";
    public static final String KEY_AUTO_ENABLED = "_auto_enabled";

    private PowerManager mPowerManager;
    private List<Sensor> mSensorsList;
    private List<String> mSensorsTypeList;
    private HashMap<String, Boolean> mScreenOnMap = new HashMap<String, Boolean>();

    public SensorStressFullTestActivity() {
        super(Utils.KEY_STRESS_FULL_TEST);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // used for gesture sensor screen on
        mPowerManager = (PowerManager) getSystemService(Context.POWER_SERVICE);
        addSensorsList();

        ListView listView = getListView();
        listView.setOnItemLongClickListener(this);
    }

    @Override
    protected void onResume() {
        super.onResume();
        updateSensorsStatus();
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    protected void addSensorsList() {
        mSensorsList = Utils.getSensorsList();
        mSensorsTypeList = new ArrayList<String> ();
        for (Sensor sensor : mSensorsList) {
            Preference preference = Utils.createPreference(Utils.TYPE_SWITCH, sensor.getName(),
                    sensor.getStringType(), getPreferenceScreen(), this);
            mSensorsTypeList.add(sensor.getStringType());
        }
    }

    private void updateSensorsStatus() {
        mActionBarSwitch.setChecked(Utils.getSensorStatus(mListenerKey));
        for (Sensor sensor : mSensorsList) {
            String type = sensor.getStringType();
            String sharedSummaryKeyString = type;
            SwitchPreference preference = (SwitchPreference)findPreference(type);
            if (preference != null) {
                preference.setChecked(Utils.getSensorStatus(sharedSummaryKeyString));
                preference.setSummary(Utils.getSensorSummary(sharedSummaryKeyString));
                if(Utils.isGestureSensor(type)
                        || MtkSensor.SENSOR_STRING_TYPE_TILT_DETECTOR.equals(type))
                mScreenOnMap.put(type, Utils.getSensorStatus(type + Utils.KEY_SCREEN_ON));
            }
        }
    }

    @Override
    protected void startTest() {
        Log.d(TAG, "startTest");
        Utils.setSensorStatus(mListenerKey, true);
        for (String type : mSensorsTypeList) {
            Utils.setSensorStatus(type, true);
            clear(type);
            SwitchPreference preference = (SwitchPreference) findPreference(type);
            if (preference != null && mBound) {
                preference.setChecked(true);
                // Full test don't support stepcounter record log
                /*if(Sensor.STRING_TYPE_ACCELEROMETER.equals(type)
                        && Utils.getSensorStatus(Utils.LOG_STATUS)) {
                    mSensorService.registerSensorWithCustomeTime(
                            Sensor.STRING_TYPE_ACCELEROMETER, true, 20 * 1000, 1000 * 1000);
                    Utils.setReceiveDataTimes(type, 0);
                } else */if(Utils.isCustomerSensor(type)) {
                    mSensorService.registerSensorWithCustomeTime(type, true,
                            Utils.getReprotRate(type), Utils.getReprotLatency(type));
                    Utils.setReceiveDataTimes(type, 0);
                } else {
                    mSensorService.registerSensor(type, true);
                }
            }
        }
    }

    @Override
    protected void finishTest() {
        Log.d(TAG, "finishTest");
        Utils.setSensorStatus(mListenerKey, false);
        mSensorService.unRegisterStressFullTestSensor();
        for (String type : mSensorsTypeList) {
            clear(type);
            Utils.setSensorStatus(type, false);
            Utils.setSensorSummary(type, "");

            SwitchPreference preference = (SwitchPreference) findPreference(type);
            if (preference != null) {
                preference.setChecked(false);
                preference.setSummary("");
            }
        }
    }

    private void notifyByScreen(boolean notify, boolean screenOn) {
        if (!notify) {
            Log.d(TAG, "The notify switch is off");
            return;
        }
        try {
            if (screenOn && !mPowerManager.isScreenOn()) {
                Log.d(TAG, "Notify user by screen on");
                wakeup();
            } else if (!screenOn && mPowerManager.isScreenOn()){
                Log.d(TAG, "Notify user by screen off");
                suspend();
            } else {
                Log.d(TAG, "No need to notify, the screen is already on/off");
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void wakeup() {
        WakeLock wl = mPowerManager.newWakeLock(PowerManager.SCREEN_BRIGHT_WAKE_LOCK |
                PowerManager.ACQUIRE_CAUSES_WAKEUP, TAG);
        wl.acquire(TIME_OUT);
    }

    private void suspend() {
        try {
            Utils.goToSleep(mPowerManager, SystemClock.uptimeMillis());
        } catch (SecurityException e) {
            Log.d(TAG, "Was unable to call PowerManager.goToSleep.");
        }
    }


    @Override
    public void onSensorChanged(String sensorType, float[] value) {
//        Log.d(TAG, "onSensorChanged1:" + sensorType);
        Preference preference = findPreference(sensorType);
        if(Sensor.STRING_TYPE_STEP_COUNTER.equals(sensorType)) {
            Utils.setSensorValues(Utils.KEY_TOTAL_STEP_COUNTER, value[0]);
            float preToatalSteps = Utils.getSensorValues(Utils.KEY_PRE_TOTAL_STEP_COUNTER);
            if (value[0] < preToatalSteps) {
                preToatalSteps = 0;
                Utils.setSensorValues(Utils.KEY_PRE_TOTAL_STEP_COUNTER, preToatalSteps);
            }

            preference.setSummary(Arrays.toString(value));
        } else if(MtkSensor.STRING_TYPE_ACTIVITY.equals(sensorType)) {
            preference.setSummary(Arrays.toString(value));
        } else if(Utils.isTriggerSensor(sensorType)) {
            preference.setSummary(String.valueOf(value[0]));
        } else if(Utils.isCompositeSesnor(sensorType)
                || MtkSensor.STRING_TYPE_PDR.equals(sensorType)) {
            preference.setSummary(Arrays.toString(value));
        } else if (Utils.isCustomerSensor(sensorType)) {
            int receiveDataTimes = Utils.getReceiveDataTimes(sensorType);
            receiveDataTimes ++;
            preference.setSummary(Arrays.toString(value) + getString(R.string.receive_string)
                    + receiveDataTimes);
            Utils.setReceiveDataTimes(sensorType, receiveDataTimes);
        } else if (Utils.isGestureSensor(sensorType)) {
            notifyByScreen(mScreenOnMap.get(sensorType), true);
            int count = Utils.getTriggerCount(sensorType + Utils.KEY_TRIGGER_COUNT);
            count++;
            Utils.setTriggerCount(sensorType + Utils.KEY_TRIGGER_COUNT, count);
            preference.setSummary(String.valueOf(count));
        } else if (MtkSensor.SENSOR_STRING_TYPE_TILT_DETECTOR.equals(sensorType)) {

            int pickUpCount = Utils.getTriggerCount(sensorType + Utils.KEY_PICK_UP_SUFFIX);
            int putDownCount = Utils.getTriggerCount(sensorType + Utils.KEY_PUT_DOWN_SUFFIX);

            switch ((int)value[0]) {
                case 0 :
                    ++putDownCount;
                    notifyByScreen(mScreenOnMap.get(sensorType), false);
                    break;
                case 1 :
                    ++pickUpCount;
                    notifyByScreen(mScreenOnMap.get(sensorType), true);
                    break;
                default :
                    break;
            }
            preference.setSummary("pick up: " + pickUpCount + ", put down: " + putDownCount);
            Utils.setTriggerCount(sensorType + Utils.KEY_PICK_UP_SUFFIX, pickUpCount);
            Utils.setTriggerCount(sensorType + Utils.KEY_PUT_DOWN_SUFFIX, putDownCount);
        }else {
            //preference.setSummary(Arrays.toString(value));
        }
    }

    @Override
    public void onSensorChanged(String sensorType, float[] value, long time, boolean register) {
//        Log.d(TAG, "onSensorChanged2:" + sensorType);
        Preference preference = findPreference(sensorType);
        String summary = "";
        for (int i = 0; i < value.length; i++) {
            summary += value[i];
            summary += ENTER_STR;
        }
        preference.setSummary(summary + getString(R.string.stress_timestamp_summary) + time);
    }

    @Override
    public void onAccuracyChanged(String sensorType, int accuracy) {
//        Log.d(TAG, "onAccuracyChanged:" + sensorType);
        Preference preference = findPreference(sensorType);
        preference.setSummary(getString(R.string.stress_accuracy_summary)
                + accuracy);
    }

    private void clear(String key) {
        for (String type : Utils.orginalGestureType) {
            Utils.setTriggerCount(type + Utils.KEY_TRIGGER_COUNT, 0);
        }
        if (Sensor.STRING_TYPE_STEP_COUNTER.equals(key)) {
            clearStepCounter();
        } else if (MtkSensor.STRING_TYPE_FLOOR_COUNT.equals(key)) {
            //seems no need
            clearFloorCounter();
        } else if (MtkSensor.STRING_TYPE_PEDOMETER.equals(key)) {
            clearPedometer();
        } else if (MtkSensor.SENSOR_STRING_TYPE_TILT_DETECTOR.equals(key)) {
            clearTiltDetector(key);
        }
    }

    private void clearStepCounter() {
        Log.d(TAG, "clearStepCounter");
        Utils.setSensorValues(Utils.KEY_TOTAL_STEP_COUNTER, 0.0f);
        Utils.setSensorValues(Utils.KEY_PRE_TOTAL_STEP_COUNTER, 0.0f);
    }

    private void clearFloorCounter() {
        Log.d(TAG, "clearFloorCounter");
        Utils.setSensorValues(Utils.KEY_PRE_TOTAL_FLOOR_COUNT, 0.0f);
        Utils.setSensorValues(Utils.KEY_TOTAL_FLOOR_COUNT, 0.0f);
    }

    private void clearPedometer() {
        Log.d(TAG, "clearPedometer");
        Utils.setSensorValues(Utils.KEY_PRE_TOTAL_DISTANCE, 0.0f);
        Utils.setSensorValues(Utils.KEY_PRE_TOTAL_STEPS, 0.0f);
    }

    private void clearTiltDetector(String sensorType) {
        Log.d(TAG, "clearTiltDetector");
        Utils.setTriggerCount(sensorType + Utils.KEY_PICK_UP_SUFFIX, 0);
        Utils.setTriggerCount(sensorType + Utils.KEY_PUT_DOWN_SUFFIX, 0);
    }

    /**
     * Run single Sensor
     * @param preference
     * @param newValue
     * @return
     */
    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        boolean bNewValue = (Boolean) newValue;
        String key = preference.getKey();
        Log.v(TAG,"onPreferenceChange, key=" + key + ", value=" + bNewValue);
        if (mSensorsTypeList.contains(key) && mBound) {
            clear(key);
            Utils.setSensorStatus(key, bNewValue);
            if(Utils.isCustomerSensor(key)) {
                mSensorService.registerSensorWithCustomeTime(key, bNewValue,
                        Utils.getReprotRate(key), Utils.getReprotLatency(key));
                Utils.setReceiveDataTimes(key, 0);
            } else {
                mSensorService.registerSensor(key, bNewValue);
                if (Sensor.STRING_TYPE_STEP_COUNTER.equals(key)
                        && Utils.getSensorStatus(Utils.LOG_STATUS)
                        && mSensorsTypeList.contains(Sensor.STRING_TYPE_ACCELEROMETER)
                        && bNewValue != Utils.getSensorStatus(Sensor.STRING_TYPE_ACCELEROMETER)) {
                    // Note: enable ACCELEROMETER while enable STEP_COUNTER
                    ((SwitchPreference) findPreference(Sensor.STRING_TYPE_ACCELEROMETER))
                            .setChecked(bNewValue);
                    Utils.setSensorStatus(Sensor.STRING_TYPE_ACCELEROMETER, bNewValue);
                    mSensorService.registerSensorWithCustomeTime(
                            Sensor.STRING_TYPE_ACCELEROMETER, bNewValue, 20 * 1000, 1000 * 1000);
                }
            }
        }
        if (!bNewValue) {
//            clear(key); Use clear button instead
            boolean isAnySensorRunning = false;
            for (String type : mSensorsTypeList) {
                isAnySensorRunning |= Utils.getSensorStatus(type);
                if(isAnySensorRunning) {
                    break;
                }
            }

            if (!isAnySensorRunning) {
                Utils.setSensorStatus(mListenerKey, false);
                mActionBarSwitch.setChecked(false);
            }
            preference.setSummary("");
        }
        return true;
    }

    @Override
    public boolean onItemLongClick(AdapterView<?> parent, View view,
                                                           int position, long id) {
        Log.v(TAG,"OnItemLongClickListener " + mSensorsTypeList.get(position));
        if (Utils.getSensorStatus(mListenerKey)) {
            // Full Test is running ,return
            return true;
        }
        setSensorDetailSettings(position);
        return true;
    }

    private void setSensorDetailSettings (int position) {
        String sensorType = mSensorsTypeList.get(position);
        String sensorName = mSensorsList.get(position).getName();

        if (Sensor.STRING_TYPE_STEP_COUNTER.equals(sensorType)) {
            Intent intent = new Intent();
            intent.setClassName("com.mediatek.sensorhub.ui",
                    "com.mediatek.sensorhub.ui.StepCounterSensorActivity");
            startActivity(intent);
        } else if (MtkSensor.SENSOR_STRING_TYPE_TILT_DETECTOR.equals(sensorType)) {
            Log.v(TAG,"TILT_DETECTOR ");
            View view = getLayoutInflater().inflate(R.layout.tilt_detector_view, null);
            Switch enableNotifiSwitch = (Switch) view.findViewById(R.id.enable_notify_sound);
            Switch enableNotifiScreenSwitch = (Switch) view.findViewById(R.id.enable_notify_screen);
            enableNotifiSwitch.setChecked(Utils.getSensorStatus(sensorType + Utils.KEY_NOTIFY_STATUS_SUFFIX));
            enableNotifiScreenSwitch.setChecked(mScreenOnMap.get(sensorType));

            CompoundButton.OnCheckedChangeListener listener = new CompoundButton.OnCheckedChangeListener() {

                @Override
                public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                    mSwitchDialogListener.onCheckedChanged(sensorType, buttonView.getId(), isChecked);
                }
            };
            enableNotifiSwitch.setOnCheckedChangeListener(listener);
            enableNotifiScreenSwitch.setOnCheckedChangeListener(listener);

            final AlertDialog dialog = new AlertDialog.Builder(this).setCancelable(true)
                    .setTitle(sensorName).setView(view).create();
            dialog.show();
        } else if (Utils.isTriggerSensor(sensorType)) {
            Log.v(TAG,"isTriggerSensor ");
            View view = getLayoutInflater().inflate(R.layout.base_trigger_view, null);
            Switch autoEnabledSwitch = (Switch) view.findViewById(R.id.auto_enabled);
            Switch enableNotifiSwitch = (Switch) view.findViewById(R.id.enable_notify_sound);
            enableNotifiSwitch.setChecked(Utils.getSensorStatus(sensorType + Utils.KEY_NOTIFY_STATUS_SUFFIX));
            autoEnabledSwitch.setChecked(Utils.getSensorStatus(sensorType + Utils.KEY_AUTO_TRIGGER_STATUS_SUFFIX));

            CompoundButton.OnCheckedChangeListener listener = new CompoundButton.OnCheckedChangeListener() {

                @Override
                public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                    mSwitchDialogListener.onCheckedChanged(sensorType, buttonView.getId(), isChecked);
                }
            };
            enableNotifiSwitch.setOnCheckedChangeListener(listener);
            autoEnabledSwitch.setOnCheckedChangeListener(listener);

            final AlertDialog dialog = new AlertDialog.Builder(this).setCancelable(true)
                    .setTitle(sensorName).setView(view).create();
            dialog.show();
        } else if (Utils.isGestureSensor(sensorType)) {
            Log.v(TAG,"isGestureSensor ");
            View view = getLayoutInflater().inflate(R.layout.base_gesture_view, null);
            Switch autoEnabledSwitch = (Switch) view.findViewById(R.id.auto_enabled);
            Switch enableNotifiSwitch = (Switch) view.findViewById(R.id.enable_notify_sound);
            Switch enableNotifiScreenSwitch = (Switch) view.findViewById(R.id.enable_notify_screen);
            enableNotifiSwitch.setChecked(Utils.getSensorStatus(sensorType + Utils.KEY_NOTIFY_STATUS_SUFFIX));
            enableNotifiScreenSwitch.setChecked(mScreenOnMap.get(sensorType));
            autoEnabledSwitch.setChecked(Utils.getSensorStatus(sensorType + Utils.KEY_AUTO_TRIGGER_STATUS_SUFFIX));

            CompoundButton.OnCheckedChangeListener listener = new CompoundButton.OnCheckedChangeListener() {

                @Override
                public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                    mSwitchDialogListener.onCheckedChanged(sensorType, buttonView.getId(), isChecked);
                }
            };
            enableNotifiSwitch.setOnCheckedChangeListener(listener);
            enableNotifiScreenSwitch.setOnCheckedChangeListener(listener);
            autoEnabledSwitch.setOnCheckedChangeListener(listener);

            final AlertDialog dialog = new AlertDialog.Builder(this).setCancelable(true)
                    .setTitle(sensorName).setView(view).create();
            dialog.show();
        } else if (Utils.isCustomerSensor(sensorType)) {
            Log.v(TAG,"isCustomerSensor ");
            View view = getLayoutInflater().inflate(R.layout.sensor_customer_view, null);
            final EditText reportRate = (EditText) view.findViewById(R.id.report_rate);
            final EditText reportLatency = (EditText) view.findViewById(R.id.report_latency);

            boolean status = Utils.getSensorStatus(sensorType);
            reportRate.setEnabled(!status);
            reportLatency.setEnabled(!status);
            reportRate.setText(String.valueOf(Utils.getReprotRate(sensorType)));
            reportLatency.setText(String.valueOf(Utils.getReprotLatency(sensorType)));
            final AlertDialog dialog = new AlertDialog.Builder(this).setCancelable(true)
                    .setTitle(sensorName).setView(view)
                    .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int whichButton) {
                            Utils.setReprotRate(sensorType,
                                    Integer.valueOf(reportRate.getText().toString()));
                            Utils.setReprotRate(sensorType,
                                    Integer.valueOf(reportLatency.getText().toString()));
                        }
                    }).create();
            dialog.show();
        }
    }

    private interface SwitchDialogOnCheckedChangeListener {
        public void onCheckedChanged(String sensorType, int buttonId, boolean isChecked);
    }

    SwitchDialogOnCheckedChangeListener mSwitchDialogListener = new SwitchDialogOnCheckedChangeListener() {

        @Override
        public void onCheckedChanged(String sensorType, int buttonId, boolean isChecked) {
            Log.d(TAG, "onCheckedChanged: sensorType= " + sensorType
                    + ",buttonId=" + buttonId + ",isChecked=" + isChecked);
            switch (buttonId) {
                case R.id.enable_notify_screen:
                    Utils.setSensorStatus(sensorType + Utils.KEY_SCREEN_ON, isChecked);
                    mScreenOnMap.put(sensorType, isChecked);
                case R.id.enable_notify_sound:
                    Utils.setSensorStatus(sensorType + Utils.KEY_NOTIFY_STATUS_SUFFIX, isChecked);
                    break;
                case R.id.auto_enabled:
                    Utils.setSensorStatus(sensorType + Utils.KEY_AUTO_TRIGGER_STATUS_SUFFIX, isChecked);
                    break;
                default:
                    break;
            }
        }
    };

    @Override
    public void onSharedPreferenceChanged(SharedPreferences sharedPreferences, String key) {
            if (mSensorsTypeList.contains(key)) {
                boolean status = sharedPreferences.getBoolean(key, false);
                Log.d(TAG, "onSharedPreferenceChanged : " + key + " status " + status);
                SwitchPreference preference = (SwitchPreference) findPreference(key);
                if(status != preference.isChecked()) {
                    preference.setChecked(status);
                }
            }
    }


}