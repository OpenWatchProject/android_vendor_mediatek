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
package com.mediatek.sensorhub.service;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.NotificationChannel;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.hardware.TriggerEventListener;
import android.hardware.TriggerEvent;
import android.media.AudioManager;
import android.media.SoundPool;
import android.os.Binder;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.Process;
import android.support.v4.content.LocalBroadcastManager;
import android.text.TextUtils;
import android.util.Log;

import com.mediatek.sensorhub.ui.R;
import com.mediatek.sensorhub.settings.MtkSensor;
import com.mediatek.sensorhub.settings.Utils;

import java.util.Arrays;
import java.util.HashMap;
import java.lang.Math;

public class SensorEventListenerService extends Service implements SensorEventListener {
    private static final String TAG = "SH/SensorEventListenerService";
    private static final String NOTIFICATION_CHANNEL = "Sensorhub";
    private static final CharSequence SENSORHUB_NOTIFICATION_NAME = "Sensorhub test is running";
    private static final int ID_SENSORHUB_SERVICE = 180131;
    private static final int ONE_SENCOND = 1000;
    private SensorManager mSensorManager;
    private NotificationManager mNotificationManager;
    private NotificationChannel mChannel;
    private TriggerListener mTriggerListener = new TriggerListener();
    // record log
    private String mRecordPath;
    private String mFullResultRecordPath;
    private String mRandomResultRecordPath;
    private StringBuilder mRecordBuilder = new StringBuilder();
    private StringBuilder mFullResultRecordBuilder = new StringBuilder();
    private StringBuilder mRandomResultRecordBuilder = new StringBuilder();
    private boolean mIsAccRegisted;
    private boolean mIsGyroRegisted;
    private boolean mIsMagRegisted;
    private boolean mIsAlsRegisted;
    private boolean mIsAccGetData;
    private boolean mIsGyroGetData;
    private boolean mIsMagGetData;
    private boolean mIsAlsGetData;
    private boolean mTestResult;

    private boolean mIsRandomAccGetData;
    private boolean mIsRandomGyroGetData;
    private boolean mIsRandomMagGetData;
    private boolean mIsRandomAlsGetData;
    private boolean mRandomTestResult;
    // Play notify @{
    private SoundPool mSounds;
    private int mSoundIdF;
    private int mSoundIdT;
    private int mSoundStreamId;
    private AudioManager mAudioManager;
    private int mUiSoundsStreamType;

    private final IBinder mBinder = new LocalBinder();

    public SensorEventListenerService() {

    }

    public interface OnSensorChangedListener {
        public void onSensorChanged(float[] value);

        public void onSensorChanged(String sensorType, float[] value);

        public void onSensorChanged(String sensorType, float[] value, long time, boolean register);

        public void onAccuracyChanged(int accuracy);

        public void onAccuracyChanged(String sensorType, int accuracy);
    }

    public interface OnTestTimeoutListener {
        public void onTestTimeout(boolean isPassed);
    }

    private HashMap<String, OnSensorChangedListener> onSensorChangedListenerMap =
        new HashMap<String, OnSensorChangedListener>();

    public void putOnSensorChangedListener(String key, OnSensorChangedListener listener) {
        onSensorChangedListenerMap.put(key, listener);
    }

    public void removeOnSensorChangedListener(String key) {
        onSensorChangedListenerMap.remove(key);
    }

    private HashMap<String, OnTestTimeoutListener> onTestTimeoutListenerMap =
            new HashMap<String, OnTestTimeoutListener>();

    public void putOnTestTimeoutListener(String key, OnTestTimeoutListener listener) {
        onTestTimeoutListenerMap.put(key, listener);
    }

    public void removeOnTestTimeoutListener(String key) {
        onTestTimeoutListenerMap.remove(key);
    }

    /**
     * Class used for the client Binder. Because we know this service always
     * runs in the same process as its clients, we don't need to deal with IPC.
     */
    public class LocalBinder extends Binder {
        public SensorEventListenerService getService(String key, OnSensorChangedListener listener, OnTestTimeoutListener listener2) {
            putOnSensorChangedListener(key, listener);
            putOnTestTimeoutListener(key, listener2);
            // Return this instance of SensorEventListenerService so clients can
            // call public methods
            return SensorEventListenerService.this;
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.d(TAG, "onReceive: " + action);
            if (Intent.ACTION_SHUTDOWN.equals(action)) {
                stopSelf();
            } else if (Intent.ACTION_SCREEN_ON.equals(action)) {
                // when screen on, flush data
                flush();
            }
        }
    };

    private final BroadcastReceiver mLocalBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.d(TAG, "local onReceive: " + action);
            if (Utils.ACTION_STEP_COUNTER_LOGGING.equals(action)) {
                recordLogs(Utils.getSensorStatus(Utils.LOG_STATUS));
            }
        }
    };
    private void flush() {
        if (Utils.getSensorStatus(MtkSensor.STRING_TYPE_ACTIVITY)) {
            Log.d(TAG, "when screen on, register activity use normal mode");
            mSensorManager.flush(this);
        }
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Log.d(TAG, "onCreate");
        mSensorManager = Utils.getSensorManager();

        mChannel = new NotificationChannel(NOTIFICATION_CHANNEL, SENSORHUB_NOTIFICATION_NAME,
                NotificationManager.IMPORTANCE_LOW);
        mAudioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        mSounds = new SoundPool.Builder().setMaxStreams(1)
                .setAudioAttributes(mChannel.getAudioAttributes()).build();
        mSoundIdF = mSounds.load(this, R.raw.in_pocket, 0);
        mSoundIdT = mSounds.load(this, R.raw.non_in_pocket, 0);

        IntentFilter filter = new IntentFilter();
        filter.addAction(Intent.ACTION_SHUTDOWN);
        filter.addAction(Intent.ACTION_SCREEN_ON);
        filter.addAction(Intent.ACTION_SCREEN_OFF);
        registerReceiver(mReceiver, filter);

        IntentFilter localIntentFilter = new IntentFilter(Utils.ACTION_STEP_COUNTER_LOGGING);
        LocalBroadcastManager.getInstance(this).registerReceiver(mLocalBroadcastReceiver,
                localIntentFilter);

    }

    @Override
    public void onStart(Intent intent, int startId) {
        super.onStart(intent, startId);
    }

    @Override
    public int onStartCommand(Intent intent, int flag, int startId) {
        NotificationManager mNotificationManager =
                (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);

        mNotificationManager.createNotificationChannel(mChannel);
        Notification notification = new Notification.Builder(getApplicationContext())
                .setContentTitle(SENSORHUB_NOTIFICATION_NAME)
                .setSmallIcon(android.R.drawable.ic_dialog_info)
                .setChannelId(NOTIFICATION_CHANNEL)
                .build();
        startForeground(ID_SENSORHUB_SERVICE, notification);
        return super.onStartCommand(intent, flag, startId);
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy");
        unRegisterStressFullTestSensor();
        unRegisterStressTestSensor();
        Utils.restoreStatusToDefult();
        if (Utils.getSensorStatus(Utils.LOG_STATUS)) {
            recordLogs(false);
        }
        mStressTimerHandler.removeCallbacksAndMessages(null);
        unregisterReceiver(mReceiver);
        LocalBroadcastManager.getInstance(this)
                .unregisterReceiver(mLocalBroadcastReceiver);
        mSounds.release();
        super.onDestroy();
    }

    // Register or unRegister sensor
    public void registerSensor(String sensorType, boolean isRegister) {
        Log.d(TAG, "registerSensor " + sensorType + " status " + isRegister);
        // Preference key is the sensor string type
        Sensor sensor = Utils.getSensorKeyMap().get(sensorType);
        checkReigstedStatus(sensorType, isRegister);
        boolean isTriggerSensor = (sensor.getReportingMode() == Sensor.REPORTING_MODE_ONE_SHOT);

        if (isTriggerSensor) {
            if (isRegister) {
                mSensorManager.requestTriggerSensor(mTriggerListener, sensor);
                mFullTriggerSensors.put(sensorType, sensor);
            } else {
                mSensorManager.cancelTriggerSensor(mTriggerListener, sensor);
                mFullTriggerSensors.remove(sensorType);
            }
        } else {
            if (isRegister) {
                if (MtkSensor.STRING_TYPE_ACTIVITY.equals(sensorType)) {
                    mSensorManager.registerListener(this, sensor, 1000000, 600000000); // 1s 10min
                } else {
                    mSensorManager
                            .registerListener(this, sensor, SensorManager.SENSOR_DELAY_NORMAL);
                }
            } else {
                mSensorManager.unregisterListener(this, sensor);
            }
        }
    }

    public void registerSensorWithCustomeTime(String sensorType, boolean isRegister, int rate,
            int latency) {
        Log.d(TAG, "registerSensorWithCustomeTime " + sensorType + " status " + isRegister
                + " rate " + rate + " latency " + latency);
        // Preference key is the sensor string type
        checkReigstedStatus(sensorType, isRegister&&(latency==0));
        Sensor sensor = Utils.getSensorKeyMap().get(sensorType);
        if (isRegister) {
            mSensorManager.registerListener(this, sensor, rate, latency);
        } else {
            mSensorManager.unregisterListener(this, sensor);
        }
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {
        String sensorTypeStr = sensor.getStringType();
        onSensorChangedListenerMap.get(Utils.KEY_STRESS_FULL_TEST)
                .onAccuracyChanged(sensorTypeStr, accuracy);
    }

    @Override
    public void onSensorChanged(SensorEvent event) {
        String sensorTypeStr = event.sensor.getStringType();
        // For step_counter record log
        if (Utils.getSensorStatus(Utils.LOG_STATUS) &&
                !Utils.getSensorStatus(Utils.KEY_STRESS_FULL_TEST) &&
                (Sensor.STRING_TYPE_STEP_COUNTER.equals(sensorTypeStr)
                        || Sensor.STRING_TYPE_STEP_COUNTER.equals(sensorTypeStr))) {
            mRecordBuilder.append(event.sensor.getName())
                    .append("@").append(event.timestamp)
                    .append(Arrays.toString(event.values)).append("\n");
        }
        if (Sensor.STRING_TYPE_STEP_COUNTER.equals(sensorTypeStr)) {
            Utils.setSensorValues(Utils.KEY_TOTAL_STEP_COUNTER, event.values[0]);
        } else if (MtkSensor.STRING_TYPE_FLOOR_COUNT.equals(sensorTypeStr)){
            Utils.setSensorValues(Utils.KEY_TOTAL_FLOOR_COUNT, event.values[0]);
        } else if (MtkSensor.SENSOR_STRING_TYPE_TILT_DETECTOR.equals(sensorTypeStr)) {
            notifyUser(sensorTypeStr, event.values[0]);
        }
        if(Utils.getSensorStatus(Utils.KEY_STRESS_FULL_TEST) == true) {
            checkResult(Utils.KEY_STRESS_FULL_TEST, event);
        }
        OnSensorChangedListener listener = onSensorChangedListenerMap.get(Utils.KEY_STRESS_FULL_TEST);
        if(listener != null) {
            listener.onSensorChanged(sensorTypeStr, event.values);
        }
    }

    class TriggerListener extends TriggerEventListener {
        public void onTrigger(TriggerEvent event) {
            if (Utils.getSensorStatus(Utils.LOG_STATUS)  &&
            !Utils.getSensorStatus(Utils.KEY_STRESS_FULL_TEST)) {
                mRecordBuilder.append("\n").append(event.sensor.getName()).append("@")
                        .append(event.timestamp).append(Arrays.toString(event.values));
            }

            String sensorTypeStr = event.sensor.getStringType();
            OnSensorChangedListener listener = null;

            // Notify
            notifyUser(sensorTypeStr, event.values[0]);

            // For others on-shot sensors
            if (sensorTypeStr != null) {
                listener = onSensorChangedListenerMap.get(Utils.KEY_STRESS_FULL_TEST);
                if (listener != null) {
                    listener.onSensorChanged(sensorTypeStr, event.values);
                }
                if (!Utils.getSensorStatus(sensorTypeStr + Utils.KEY_AUTO_TRIGGER_STATUS_SUFFIX)) {
                    Utils.setSensorStatus(sensorTypeStr, false);
                } else {
                    // auto enabled
                    if (Sensor.STRING_TYPE_SIGNIFICANT_MOTION.equals(sensorTypeStr)) {
                        try {
                            Thread.sleep(1000); // SMD sleep 1s
                        } catch (InterruptedException e) {
                            Log.d(TAG, "interrupt");
                        }
                    }
                    mSensorManager.requestTriggerSensor(mTriggerListener, event.sensor);
                }
            }

        }
    }

    // notify user when sensor event change
    private void notifyUser(String key, float value) {
        Log.d(TAG, "notifyUser: " + key + ",value=" + value);
        if (Utils.getSensorStatus(key + Utils.KEY_NOTIFY_STATUS_SUFFIX)) {
            if (value == 1) {
                playSound(mSoundIdF);
            } else {
                if (key.equals(MtkSensor.SENSOR_STRING_TYPE_TILT_DETECTOR)) {
                    playSound(mSoundIdT);
                }
            }
        }
    }

    private void playSound(int soundId) {
        mSounds.stop(mSoundStreamId);
//        if (mAudioManager != null) {
//            mUiSoundsStreamType = mAudioManager.getUiSoundsStreamType();
//        }
//        // If the stream is muted, don't play the sound
//        if (mAudioManager.isStreamMute(mUiSoundsStreamType))
//            return;
        mSoundStreamId = mSounds.play(soundId, 1, 1, 1/* priortiy */, 0/* loop */, 1.0f/* rate */);
    }

    private void checkResult (String key, SensorEvent event){
        boolean isFullTest = Utils.KEY_STRESS_FULL_TEST.equals(key);
        String sensorTypeStr = event.sensor.getStringType();
        if(Sensor.STRING_TYPE_ACCELEROMETER.equals(sensorTypeStr)) {
            if (isFullTest) {
                mIsAccGetData = true;
            } else {
                mIsRandomAccGetData = true;
            }
            if(event.values[2] > Utils.getMaxValue(sensorTypeStr, 11.0f)
                    || event.values[2] < Utils.getMinValue(sensorTypeStr, 8.0f)) {
                Log.d(TAG, "ACC failed: Z = " + event.values[2]);
                if (isFullTest) {
                    mTestResult = false;
                    mFullResultRecordBuilder.append("\n").append(event.sensor.getName()).append("@")
                            .append(event.timestamp).append(Arrays.toString(event.values));
                    stopTimer(Utils.KEY_STRESS_FULL_TEST);
                } else {
                    mRandomTestResult = false;
                    mRandomResultRecordBuilder.append("\n").append(event.sensor.getName()).append("@")
                            .append(event.timestamp).append(Arrays.toString(event.values));
                    stopOneSensorForStressTest(sensorTypeStr);
                    //stopTimer(Utils.KEY_STRESS_TEST);
                }
            }
        } else if(Sensor.STRING_TYPE_GYROSCOPE.equals(sensorTypeStr)) {
            if (isFullTest) {
                mIsGyroGetData = true;
            } else {
                mIsRandomGyroGetData = true;
            }
        } else if(Sensor.STRING_TYPE_LIGHT.equals(sensorTypeStr)) {
            if (isFullTest) {
                mIsAlsGetData = true;
            } else {
                mIsRandomAlsGetData = true;
            }
        } else if(Sensor.STRING_TYPE_MAGNETIC_FIELD.equals(sensorTypeStr)) {
            float vectorSum = getVectorSum(event.values);
            if(vectorSum < Utils.getMinValue(sensorTypeStr, 20.0f)
                    || vectorSum > Utils.getMaxValue(sensorTypeStr, 80.0f)) {
                Log.d(TAG, "MAGNETIC failed: vector sum = " + vectorSum);
                if (isFullTest) {
                    mTestResult = false;
                    mFullResultRecordBuilder.append("\n").append(event.sensor.getName()).append("@")
                            .append(event.timestamp).append(Arrays.toString(event.values));
                    stopTimer(Utils.KEY_STRESS_FULL_TEST);
                } else {
                    mRandomTestResult = false;
                    mRandomResultRecordBuilder.append("\n").append(event.sensor.getName()).append("@")
                            .append(event.timestamp).append(Arrays.toString(event.values));
                    stopOneSensorForStressTest(sensorTypeStr);
                    //stopTimer(Utils.KEY_STRESS_TEST);
                }
            }
            if (isFullTest) {
                mIsMagGetData = true;
            } else {
                mIsRandomMagGetData = true;
            }
        }
    }

    private float getVectorSum(float[] value) {
        return (float) Math.sqrt(Math.pow(value[0],2) + Math.pow(value[1],2) + Math.pow(value[2],2));
    }

    private void checkReigstedStatus (String sensorTypeStr, boolean isRegisted) {
        if(Sensor.STRING_TYPE_ACCELEROMETER.equals(sensorTypeStr)) {
            mIsAccRegisted = isRegisted;
            mIsRandomAccGetData = false;
            accRandomCounter = 0;
        } else if(Sensor.STRING_TYPE_GYROSCOPE.equals(sensorTypeStr)) {
            mIsGyroRegisted = isRegisted;
            mIsRandomGyroGetData = false;
            gyroRandomCounter = 0;
        } else if(Sensor.STRING_TYPE_LIGHT.equals(sensorTypeStr)) {
            mIsAlsRegisted = isRegisted;
            mIsRandomAlsGetData = false;
            alsRandomCounter = 0;
        } else if(Sensor.STRING_TYPE_MAGNETIC_FIELD.equals(sensorTypeStr)) {
            mIsMagRegisted = isRegisted;
            mIsRandomMagGetData = false;
            magRandomCounter = 0;
        }
    }

    public void unRegisterStressFullTestSensor() {
        mSensorManager.unregisterListener(this);
        for (Sensor sensor: mFullTriggerSensors.values()) {
            Log.d(TAG, "unRegisterStressFullTestSensor sensor : " + sensor.getName());
            mSensorManager.cancelTriggerSensor(mTriggerListener, sensor);
        }
    }

    // Add for Stress test @{
    private SensorEventListener mStressTestEventListener = new SensorEventListener() {

        @Override
        public void onSensorChanged(SensorEvent event) {
            String sensorType = event.sensor.getStringType();
            checkResult(Utils.KEY_STRESS_TEST, event);
            onSensorChangedListenerMap.get(Utils.KEY_STRESS_TEST).onSensorChanged(sensorType,
                    event.values, event.timestamp, true);
        }

        @Override
        public void onAccuracyChanged(Sensor sensor, int accuracy) {
            String sensorType = sensor.getStringType();
            onSensorChangedListenerMap.get(Utils.KEY_STRESS_TEST)
                .onAccuracyChanged(sensorType, accuracy);
        }
    };

    private TriggerEventListener mStressTestTriggerListener = new TriggerEventListener() {

        public void onTrigger(TriggerEvent event) {
            String sensorType = event.sensor.getStringType();
            onSensorChangedListenerMap.get(Utils.KEY_STRESS_TEST).onSensorChanged(sensorType,
                    event.values, event.timestamp, true);
        }
    };

    private final int[] SENSOR_DELAY_ARRAY = { SensorManager.SENSOR_DELAY_NORMAL,
            SensorManager.SENSOR_DELAY_GAME, SensorManager.SENSOR_DELAY_FASTEST,
            SensorManager.SENSOR_DELAY_UI };

    private final HashMap<String, Sensor> mTriggerSensors = new HashMap<>();
    private final HashMap<String, Sensor> mFullTriggerSensors = new HashMap<>();

    public void stressTestRegisterSensor(Sensor sensor, boolean isRegister, int rateIndex) {

        checkReigstedStatus(sensor.getStringType(), isRegister);
        boolean isTriggerSensor = (sensor.getReportingMode() == Sensor.REPORTING_MODE_ONE_SHOT);
        if (isTriggerSensor) {
            if (isRegister) {
                mSensorManager.requestTriggerSensor(mStressTestTriggerListener, sensor);
                mTriggerSensors.put(sensor.getStringType(), sensor);
            } else {
                mSensorManager.cancelTriggerSensor(mStressTestTriggerListener, sensor);
                mTriggerSensors.remove(sensor.getStringType());
            }
        } else {
            if (isRegister) {
                mSensorManager.registerListener(mStressTestEventListener, sensor,
                        SENSOR_DELAY_ARRAY[rateIndex]);
            } else {
                mSensorManager.unregisterListener(mStressTestEventListener, sensor);
                // Can't receive sensor data after unregister listener,
                // so fake a float array data a for updating UI summary
                float[] a = {1f};
                onSensorChangedListenerMap.get(Utils.KEY_STRESS_TEST).onSensorChanged(
                        sensor.getStringType(), a, 1l, false);
            }
        }
    }

    public void unRegisterStressTestSensor() {
        Log.d(TAG, "unRegisterStressTestSensor ");
        mSensorManager.unregisterListener(mStressTestEventListener);
        for (Sensor sensor: mTriggerSensors.values()) {
            Log.d(TAG, "unRegisterStressTestSensor sensor : " + sensor.getName());
            mSensorManager.cancelTriggerSensor(mStressTestTriggerListener, sensor);
        }
    }
    // @}

    // user start test
    public void startTimer(String key) {
        int time = Utils.getTestDuration(key);
        mIsAccRegisted = false;
        mIsGyroRegisted = false;
        mIsMagRegisted = false;
        mIsAlsRegisted = false;
        Log.d(TAG, "startTimer:TestDuration (h): " + time);
        long delayMillis = (long) time * 60 * ONE_SENCOND;// min
        if(Utils.KEY_STRESS_TEST.equals(key)) {
            mRandomTestResult = true;
            recordRandomTestLogs(true);
            mStressTimerHandler.sendEmptyMessageDelayed(MSG_STRESS_RANDOM_TEST_GET_REPORT,ONE_SENCOND);
            mStressTimerHandler.sendEmptyMessageDelayed(MSG_STRESS_RANDOM_TEST_STOP_TIMER, delayMillis);
        } else if(Utils.KEY_STRESS_FULL_TEST.equals(key)) {
            mTestResult = true;
            mIsAccGetData = false;
            mIsGyroGetData = false;
            mIsMagGetData = false;
            mIsAlsGetData = false;
            accCounter = 0;
            gyroCounter = 0;
            magCounter = 0;
            alsCounter = 0;
            recordFullTestLogs(true);
            mStressTimerHandler.sendEmptyMessageDelayed(MSG_STRESS_FULL_TEST_GET_REPORT,ONE_SENCOND);
            mStressTimerHandler.sendEmptyMessageDelayed(MSG_STRESS_FULL_TEST_STOP_TIMER, delayMillis);
        }
    }

    // user stop test
    public void stopTimer(String key) {
        if(Utils.KEY_STRESS_TEST.equals(key)) {
            mStressTimerHandler.removeMessages(MSG_STRESS_RANDOM_TEST_GET_REPORT);
            mStressTimerHandler.removeMessages(MSG_STRESS_RANDOM_TEST_STOP_TIMER);
            mStressTimerHandler.sendEmptyMessage(MSG_STRESS_RANDOM_TEST_STOP_TIMER);
        } else if(Utils.KEY_STRESS_FULL_TEST.equals(key)) {
            mStressTimerHandler.removeMessages(MSG_STRESS_FULL_TEST_GET_REPORT);
            mStressTimerHandler.removeMessages(MSG_STRESS_FULL_TEST_STOP_TIMER);
            mStressTimerHandler.sendEmptyMessage(MSG_STRESS_FULL_TEST_STOP_TIMER);
        }
    }

    private void stopOneSensorForStressTest(String sensorString) {
        Utils.setSensorStatus(Utils.KEY_STRESS_TEST + sensorString, false);
        Utils.setSensorSummary(Utils.KEY_STRESS_TEST + sensorString, "Failed");
        stressTestRegisterSensor(Utils.getSensorKeyMap().get(sensorString), false, 0);
    }

    int accCounter = 0;
    int gyroCounter = 0;
    int magCounter = 0;
    int alsCounter = 0;
    int accRandomCounter = 0;
    int gyroRandomCounter = 0;
    int magRandomCounter = 0;
    int alsRandomCounter = 0;
    static final int MSG_STRESS_RANDOM_TEST_STOP_TIMER = 101;
    static final int MSG_STRESS_RANDOM_TEST_GET_REPORT = 102;
    static final int MSG_STRESS_FULL_TEST_STOP_TIMER = 103;
    static final int MSG_STRESS_FULL_TEST_GET_REPORT = 104;
    Handler mStressTimerHandler = new Handler() {

        @Override
        public void handleMessage(Message message) {
            switch (message.what) {
                case MSG_STRESS_RANDOM_TEST_STOP_TIMER:
                    Log.d(TAG, "MSG_STRESS_RANDOM_TEST_STOP_TIMER");
                    mStressTimerHandler.removeMessages(MSG_STRESS_RANDOM_TEST_GET_REPORT);
                    recordRandomTestLogs(false);
                    onTestTimeoutListenerMap.get(Utils.KEY_STRESS_TEST).onTestTimeout(mRandomTestResult);
                    break;
                case MSG_STRESS_FULL_TEST_STOP_TIMER:
                    Log.d(TAG, "MSG_STRESS_FULL_TEST_STOP_TIMER");
                    mStressTimerHandler.removeMessages(MSG_STRESS_RANDOM_TEST_GET_REPORT);
                    recordFullTestLogs(false);
                    onTestTimeoutListenerMap.get(Utils.KEY_STRESS_FULL_TEST).onTestTimeout(mTestResult);
                    break;
                case MSG_STRESS_RANDOM_TEST_GET_REPORT:
                    if(mIsAccRegisted && (++accRandomCounter) >= Utils.getAccCounter()) {
                        if(!mIsRandomAccGetData) {
                            mRandomTestResult = false;
                            mRandomResultRecordBuilder.append("\n ")
                                    .append(Utils.getTimeString())
                                    .append(": Get ===ACC=== Data fail!!");
                            Log.w(TAG, "STRESS_TEST:Get ===ACC==== Data fail!!");
                            stopOneSensorForStressTest(Sensor.STRING_TYPE_ACCELEROMETER);
                        } else {
                            mIsRandomAccGetData = false;
                            accRandomCounter = 0;
                        }
                    }
                    if(mIsGyroRegisted && (++gyroRandomCounter) >= Utils.getGyroCounter()) {
                        if(!mIsRandomGyroGetData) {
                            mRandomTestResult = false;
                            mRandomResultRecordBuilder.append("\n ")
                                    .append(Utils.getTimeString())
                                    .append(": Get ===GYRO=== Data fail!!");
                            Log.w(TAG, "STRESS_TEST:Get ===GYRO==== Data fail!!");
                            stopOneSensorForStressTest(Sensor.STRING_TYPE_GYROSCOPE);
                        } else {
                            mIsRandomGyroGetData = false;
                            gyroRandomCounter = 0;
                        }
                    }
                    if(mIsMagRegisted && (++magRandomCounter) >= Utils.getMagCounter()) {
                        if(!mIsRandomMagGetData) {
                            mRandomTestResult = false;
                            mRandomResultRecordBuilder.append("\n ")
                                    .append(Utils.getTimeString())
                                    .append(": Get ===MAG=== Data fail!!");
                            Log.w(TAG, "STRESS_TEST:Get ===MAG==== Data fail!!");
                            stopOneSensorForStressTest(Sensor.STRING_TYPE_MAGNETIC_FIELD);
                        } else {
                            mIsRandomMagGetData = false;
                            magCounter = 0;
                        }
                    }
                    if( mIsAlsRegisted && (++alsRandomCounter) >= Utils.getAlsCounter()) {
                        if(!mIsRandomAlsGetData) {
                            mRandomTestResult = false;
                            mRandomResultRecordBuilder.append("\n ")
                                    .append(Utils.getTimeString())
                                    .append(": Get ===ALS=== Data fail!!");
                            Log.w(TAG, "STRESS_TEST:Get ===ALS==== Data fail!!");
                            stopOneSensorForStressTest(Sensor.STRING_TYPE_LIGHT);
                        } else {
                            mIsAlsRegisted = false;// ALS only requires"first 1s has data"
                            mIsRandomAlsGetData = false;
                            alsRandomCounter = 0;
                        }
                    }

                    mStressTimerHandler.sendEmptyMessageDelayed(MSG_STRESS_RANDOM_TEST_GET_REPORT,
                            ONE_SENCOND);
                    break;
                case MSG_STRESS_FULL_TEST_GET_REPORT:
                    if(mIsAccRegisted && (++accCounter) >= Utils.getAccCounter()) {
                        if(!mIsAccGetData) {
                            mTestResult = false;
                            mFullResultRecordBuilder.append("\n ")
                                    .append(Utils.getTimeString())
                                    .append(": Get ===ACC=== Data fail!!");
                            Log.w(TAG, "FULL_TEST:Get ===ACC==== Data fail!!");
                        } else {
                            mIsAccGetData = false;
                            accCounter = 0;
                        }
                    }
                    if(mIsGyroRegisted && (++gyroCounter) >= Utils.getGyroCounter()) {
                        if(!mIsGyroGetData) {
                            mTestResult = false;
                            mFullResultRecordBuilder.append("\n ")
                                    .append(Utils.getTimeString())
                                    .append(": Get ===GYRO=== Data fail!!");
                            Log.w(TAG, "FULL_TEST:Get ===GYRO==== Data fail!!");
                        } else {
                            mIsGyroGetData = false;
                            gyroCounter = 0;
                        }
                    }
                    if(mIsMagRegisted && (++magCounter) >= Utils.getMagCounter()) {
                        if(!mIsMagGetData) {
                            mTestResult = false;
                            mFullResultRecordBuilder.append("\n ")
                                    .append(Utils.getTimeString())
                                    .append(": Get ===MAG=== Data fail!!");
                            Log.w(TAG, "FULL_TEST:Get ===MAG==== Data fail!!");
                        } else {
                            mIsMagGetData = false;
                            magCounter = 0;
                        }
                    }
                    if( mIsAlsRegisted && (++alsCounter) >= Utils.getAlsCounter()) {
                        if(!mIsAlsGetData) {
                            mTestResult = false;
                            mFullResultRecordBuilder.append("\n ")
                                    .append(Utils.getTimeString())
                                    .append(": Get ===ALS=== Data fail!!");
                            Log.w(TAG, "FULL_TEST:Get ===ALS==== Data fail!!");
                        } else {
                            mIsAlsRegisted = false;// ALS only requires"first 1s has data"
                            mIsAlsGetData = false;
                            alsCounter = 0;
                        }
                    }

                    if(mTestResult == false) {
                        stopTimer(Utils.KEY_STRESS_FULL_TEST);
                    } else {
                        mStressTimerHandler.sendEmptyMessageDelayed(MSG_STRESS_FULL_TEST_GET_REPORT,
                                ONE_SENCOND);
                    }
                    break;
                default:
                    break;
            }
        }
    };

    public void recordLogs(boolean isRecord) {
        if (isRecord) {
            Log.d(TAG, "sensor algorithm test start");
            mRecordPath = Utils.initRecordFileName();
        } else {
            Log.d(TAG, "sensor algorithm test end");
            String record = mRecordBuilder.toString();
            if (!TextUtils.isEmpty(record)) {
                Utils.recordToSdcard(record.getBytes(), mRecordPath);
            }
            mRecordPath = "";
        }
        mRecordBuilder = new StringBuilder();
        return;
    }

    public void recordFullTestLogs(boolean isRecord) {
        if (isRecord) {
            Log.d(TAG, "Full Test record start");
            String path = Utils.initRecordFileName();
            mFullResultRecordPath = path.substring(0,path.length()-4)+"_FullTest.txt" ;
            Log.d(TAG, "mFullResultRecordPath = "+mFullResultRecordPath);
        } else {
            Log.d(TAG, "Full Test record end");
            mFullResultRecordBuilder.append("\n").append("Test Result:")
                    .append(mTestResult ? "Pass!!":"Fail!!");
            String record = mFullResultRecordBuilder.toString();
            if (!TextUtils.isEmpty(record)) {
                Utils.recordToSdcard(record.getBytes(), mFullResultRecordPath);
            }
            mFullResultRecordPath = "";
        }
        mFullResultRecordBuilder = new StringBuilder("FullTest:\n");
        return;
    }

    public void recordRandomTestLogs(boolean isRecord) {
        if (isRecord) {
            Log.d(TAG, "Random test record start");
            String path = Utils.initRecordFileName();
            mRandomResultRecordPath = path.substring(0,path.length()-4)+"_RandomTest.txt" ;
            Log.d(TAG, "mRandomResultRecordPath = "+mRandomResultRecordPath);
        } else {
            Log.d(TAG, "Random test record end");
            mRandomResultRecordBuilder.append("\n").append("Test Result:")
                    .append(mRandomTestResult ? "Pass!!":"Fail!!");
            String record = mRandomResultRecordBuilder.toString();
            if (!TextUtils.isEmpty(record)) {
                Utils.recordToSdcard(record.getBytes(), mRandomResultRecordPath);
            }
            mRandomResultRecordPath = "";
        }
        mRandomResultRecordBuilder = new StringBuilder("RandomTest:\n");
        return;
    }

}
