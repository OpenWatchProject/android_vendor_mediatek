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

import android.hardware.Sensor;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.Process;
import android.preference.Preference;
import android.util.Log;

import com.mediatek.sensorhub.settings.Utils;

import java.util.Arrays;
import java.util.List;
import java.util.Random;

public class SensorStressTestActivity extends BaseActivity {

    public static final String TAG = "SH/SensorStressTestLog";
    private static final String ENTER_STR = "\n";
    private static final int REGISTE_INTERVAL_MS = 5 * 1000;

    private static StressTester mStressTestHanlder;
    public static HandlerThread mStressTestThread;
    // for background sensor change data handle
    private static UpdatePreferenceHandler mUiHanlder;
    private static final String BUNDLE_KEY_PERFERENCE = "perference_key";

    private int mSensorsCount = 0;
    private Random mRandom = new Random();

    public SensorStressTestActivity() {
        super(Utils.KEY_STRESS_TEST);
        Log.d(TAG, "SensorStressTestActivity");

        if (mStressTestHanlder == null) {
            if (mStressTestThread == null) {
                mStressTestThread = new HandlerThread("SensorEventListenerService.changed",
                        Process.THREAD_PRIORITY_BACKGROUND);
            }
            mStressTestThread.start();
            mStressTestHanlder = new StressTester(mStressTestThread.getLooper());
        }
        if (mUiHanlder == null) {
            mUiHanlder = new UpdatePreferenceHandler();
        }
    }

    class UpdatePreferenceHandler extends Handler {
        static final int MSG_UPDAET = 0;

        @Override
        public void handleMessage(Message message) {
            if (message.what != MSG_UPDAET) {
                return;
            }

            Bundle bundle = (Bundle) message.obj;
            String key = bundle.getString(BUNDLE_KEY_PERFERENCE);
            boolean registeState = bundle.getBoolean("registeState");
            int rateIndex = bundle.getInt("rateIndex");
            // Register sensor
            mSensorService.stressTestRegisterSensor(Utils.getSensorKeyMap().get(key), registeState,
                    rateIndex);
        }
    }

    class StressTester extends Handler {

        static final int MSG_REGISTER = 0;

        StressTester(Looper looper) {
            super(looper);
        }

        void resume() {
            if (!hasMessages(MSG_REGISTER)) {
                Log.d(TAG, "sendEmptyMessage: MSG_REGISTER" + MSG_REGISTER);
                sendEmptyMessage(MSG_REGISTER);
            }
        }

        void pause() {
            removeMessages(MSG_REGISTER);
        }

        @Override
        public void handleMessage(Message message) {
            if (message.what != MSG_REGISTER) {
                return;
            }
            int choose = Math.abs(mRandom.nextInt()) % mSensorsCount;
            int rateIndex = Math.abs(mRandom.nextInt()) % 4;

            Sensor sensor = Utils.getSensorsList().get(choose);
            String sharedStutusKeyString = mListenerKey + sensor.getStringType();
            boolean registeState = !Utils.getSensorStatus(sharedStutusKeyString);

            // Record sensor status
            Utils.setSensorStatus(sharedStutusKeyString, registeState);

            // Record sensor summary
            String summString = registeState ?
                    (getResources().getStringArray(R.array.rate_status)[rateIndex]) :
                    (getString(R.string.stress_sensor_close_summary));
            Utils.setSensorSummary(sharedStutusKeyString, summString);

            Bundle bundle = new Bundle();
            bundle.putString(BUNDLE_KEY_PERFERENCE, sensor.getStringType());
            bundle.putBoolean("registeState", registeState);
            bundle.putInt("rateIndex", rateIndex);

            Message msg = mUiHanlder.obtainMessage(UpdatePreferenceHandler.MSG_UPDAET, bundle);
            mUiHanlder.sendMessage(msg);

            sendEmptyMessageDelayed(MSG_REGISTER, REGISTE_INTERVAL_MS);
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mListenerKey = Utils.KEY_STRESS_TEST;
        Log.d(TAG, "onCreate");
        addSensorsList();
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

    @Override
    public void onSensorChanged(String sensorType, float[] value, long time, boolean register) {
        Preference preference = findPreference(sensorType);
        String sharedStutusKeyString = mListenerKey + sensorType;
        String summary = Utils.getSensorSummary(sharedStutusKeyString);
        if (summary.equals(getString(R.string.stress_sensor_close_summary)) || !register) {
            preference.setSummary(getString(R.string.stress_sensor_close_summary));
            return;
        }
        for (int i = 0; i < value.length; i++) {
            summary += ENTER_STR;
            summary += value[i];
        }
        preference.setSummary(summary + ENTER_STR + getString(R.string.stress_timestamp_summary)
                + time);
    }

    @Override
    public void onAccuracyChanged(String sensorType, int accuracy) {
        Preference preference = findPreference(sensorType);
        String sharedStutusKeyString = mListenerKey + sensorType;
        String rateString = Utils.getSensorSummary(sharedStutusKeyString);
        preference.setSummary(rateString + ENTER_STR + getString(R.string.stress_accuracy_summary)
                + accuracy);
    }

    @Override
    protected void startTest() {
        Log.d(TAG, "startTest");
        if(mSensorsCount == 0) {
            Log.w(TAG, "sensor count is 0, don't test");
            return;
        }
        mStressTestHanlder.resume();
        Utils.setSensorStatus(mListenerKey, true);
    }

    @Override
    protected void finishTest() {
        Log.d(TAG, "finishTest");
        mStressTestHanlder.pause();
        mSensorService.unRegisterStressTestSensor();
        Utils.setSensorStatus(mListenerKey, false);
        closeAllSensors();
    }

    protected void addSensorsList() {
        List<Sensor> sensorsList = Utils.getSensorsList();
        int i = 0;
        for (Sensor sensor : sensorsList) {
            Utils.createPreference(Utils.TYPE_PREFERENCE, sensor.getName(), sensor.getStringType(),
                    getPreferenceScreen(), this);
            i++;
        }
        mSensorsCount = i;
    }

    private void updateSensorsStatus() {
        mActionBarSwitch.setChecked(Utils.getSensorStatus(mListenerKey));
        List<Sensor> sensorsList = Utils.getSensorsList();
        for (Sensor sensor : sensorsList) {
            String type = sensor.getStringType();
            String sharedStutusKeyString = mListenerKey + type;
            Preference preference = findPreference(type);
            if (preference != null) {
                preference.setSummary(Utils.getSensorSummary(sharedStutusKeyString));
            }
        }
    }

    private void closeAllSensors() {
        List<Sensor> sensorsList = Utils.getSensorsList();
        for (Sensor sensor : sensorsList) {
            String type = sensor.getStringType();
            String sharedStutusKeyString = mListenerKey + type;
            Utils.setSensorStatus(sharedStutusKeyString, false);
            Utils.setSensorSummary(sharedStutusKeyString, "");

            Preference preference = findPreference(type);
            if (preference != null) {
                preference.setSummary("");
            }
        }
    }
}