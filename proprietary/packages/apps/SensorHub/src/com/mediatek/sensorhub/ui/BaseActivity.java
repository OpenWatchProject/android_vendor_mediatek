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
package com.mediatek.sensorhub.ui;

import android.app.ActionBar;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.hardware.Sensor;
import android.os.Bundle;
import android.os.IBinder;
import android.preference.PreferenceActivity;
import android.text.TextUtils;
import android.view.Gravity;
import android.view.KeyEvent;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.Switch;
import android.widget.TextView;

import com.mediatek.sensorhub.service.SensorEventListenerService;
import com.mediatek.sensorhub.service.SensorEventListenerService.LocalBinder;
import com.mediatek.sensorhub.service.SensorEventListenerService.OnSensorChangedListener;
import com.mediatek.sensorhub.service.SensorEventListenerService.OnTestTimeoutListener;
import com.mediatek.sensorhub.settings.Utils;
import com.mediatek.sensorhub.ui.R;

import java.util.HashMap;

abstract public class BaseActivity extends PreferenceActivity implements OnSensorChangedListener, OnTestTimeoutListener,
        CompoundButton.OnCheckedChangeListener {

    public SensorEventListenerService mSensorService;
    public boolean mBound = false;
    public String mListenerKey;
    public Switch mActionBarSwitch;
    private TextView mTestRestTV;
    private EditText mTestTime;
//    protected  CompoundButton.OnCheckedChangeListener mOnCheckedChangeListener;
    public static final HashMap<String, Sensor> mSensorKeyMap = Utils.getSensorKeyMap();

    public BaseActivity(String key) {
        mListenerKey = key;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        addPreferencesFromResource(R.xml.stress_test_pref);
        setContentView(R.layout.stress_test_layout);
        mTestRestTV = (TextView) findViewById(R.id.test_result);
        mTestTime = (EditText) findViewById(R.id.test_time);
        mTestTime.setText(String.valueOf(Utils.getTestDuration(mListenerKey)));
        addActionSwitch();
    }

    private void addActionSwitch() {
        mActionBarSwitch = new Switch(getLayoutInflater().getContext());
        final int padding = getResources().getDimensionPixelSize(R.dimen.action_bar_switch_padding);
        mActionBarSwitch.setPaddingRelative(0, 0, padding, 0);
        getActionBar().setDisplayOptions(
                ActionBar.DISPLAY_HOME_AS_UP | ActionBar.DISPLAY_SHOW_CUSTOM,
                ActionBar.DISPLAY_HOME_AS_UP | ActionBar.DISPLAY_SHOW_CUSTOM);
        getActionBar()
                .setCustomView(
                        mActionBarSwitch,
                        new ActionBar.LayoutParams(ActionBar.LayoutParams.WRAP_CONTENT,
                                ActionBar.LayoutParams.WRAP_CONTENT, Gravity.CENTER_VERTICAL
                                | Gravity.END));
        mActionBarSwitch.setOnCheckedChangeListener(this);
    }

    @Override
    public void onCheckedChanged (CompoundButton buttonView,boolean isChecked){
        boolean status = Utils.getSensorStatus(mListenerKey);
        if (isChecked && !status) {
            String time = mTestTime.getText().toString();
            if(!TextUtils.isEmpty(time)) {
                try {
                    int duration = Integer.valueOf(time);
                    Utils.setTestDuration(mListenerKey, duration);
                } catch (NumberFormatException e) {
                }
            }
            mTestRestTV.setText("");
            if( !Utils.getSensorsList().isEmpty()) {
                mSensorService.startTimer(mListenerKey);
                startTest();
            }
        } else if (!isChecked && status) {
            if( !Utils.getSensorsList().isEmpty()) {
                mSensorService.stopTimer(mListenerKey);
            }
        }
    }

    @Override
    protected void onStart() {
        super.onStart();
        // Bind to LocalService
        Intent intent = new Intent(this, SensorEventListenerService.class);
        bindService(intent, mConnection, Context.BIND_AUTO_CREATE);
    }

    @Override
    protected void onStop() {
        super.onStop();
        // Unbind from the service
        if (mBound) {
            unbindService(mConnection);
            mBound = false;
        }
    }

    @Override
    protected boolean isValidFragment(String fragmentName) {
        return false;
    }

    /** Defines callbacks for service binding, passed to bindService() */
    private ServiceConnection mConnection = new ServiceConnection() {

        @Override
        public void onServiceConnected(ComponentName className, IBinder service) {
            // We've bound to LocalService, cast the IBinder and get
            // LocalService instance
            LocalBinder binder = (LocalBinder) service;
            mSensorService = binder.getService(mListenerKey, BaseActivity.this, BaseActivity.this);
            mBound = true;
        }

        @Override
        public void onServiceDisconnected(ComponentName arg0) {
            mSensorService.removeOnSensorChangedListener(mListenerKey);
            mSensorService.removeOnTestTimeoutListener(mListenerKey);
            mBound = false;
        }
    };

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_VOLUME_DOWN) { // sync with onTouch for shutter up
            if (!mBound) {
                return false;
            }
            boolean check = mActionBarSwitch.isChecked();
            mActionBarSwitch.setChecked(!check);

//            mTestRestTV.setText("");
//            startTest();

            return false;
        }
        return super.onKeyUp(keyCode, event);
    }

    abstract protected void finishTest();

    abstract protected void startTest();

    @Override
    public void onSensorChanged(float[] value) {
    }

    @Override
    public void onSensorChanged(String sensorType, float[] value) {
    }

    @Override
    public void onAccuracyChanged(int accuracy) {
    }

    @Override
    public void onAccuracyChanged(String sensorType, int accuracy) {
    }

    @Override
    public void onSensorChanged(String sensorType, float[] value, long time, boolean register) {
    }

    @Override
    public void onTestTimeout(boolean isPassed) {
        finishTest();
        mTestRestTV.setText(isPassed? "Pass" : "Fail");
    }
}
