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

package com.mediatek.sensorhub.sensor;

import android.app.Activity;
import android.hardware.SensorEventListener;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;
import java.util.Locale;
import com.mediatek.sensorhub.ui.R;

public class SensorCalibrationNew extends Activity implements OnClickListener {
    public static final String CALIBRAION_TYPE = "type";
    public static final int GSENSOR = 0;
    public static final int GYROSCOPE = 1;
    public static final int LIGHT = 2;
    private static final String TAG = "SH/CalibrationN";
    private static final int MSG_START_CALIBRARION = 2;
    private static final int MSG_GET_SUCCESS = 5;
    private static final int MSG_SET_FAILURE = 6;
    private static final int MSG_GET_FAILURE = 7;
    private static final String[] SENSOR_NAME = {"GSENSOR", "GYROSCOPE","LIGHT"};

    private Button mStartCalibration;
    private TextView mCaliData;
    private TextView mCaliStatus;
    private TextView mCurrentData;
    private TextView mCurrentStatus;

    private int mType;
    private int mSensorType;
    private String mData;

    private final HandlerThread mHandlerThread = new HandlerThread("async_handler");
    private Handler mHandler;
    private Handler mUiHandler;

    private SensorManager mSensorManager = null;
    private Sensor mSensor = null;
    private SensorEventListener mSensorEventListener = new SensorEventListener() {
        @Override
        public void onSensorChanged(SensorEvent event) {
            if (event.sensor.getType() == mSensorType) {
                if (mSensorType == Sensor.TYPE_LIGHT) {
                    mCurrentData.setText(String.format(Locale.ENGLISH, "%+8.4f",
                            event.values[0]));
                } else {
                    mCurrentData.setText(String.format(Locale.ENGLISH, "%+8.4f,%+8.4f,%+8.4f",
                            event.values[0], event.values[1], event.values[2]));
                }
                String status = "Status: ";
                switch (event.accuracy) {
                    case SensorManager.SENSOR_STATUS_UNRELIABLE:
                        status = status + "Unreliable";
                        break;
                    case SensorManager.SENSOR_STATUS_ACCURACY_HIGH:
                        status = status + "High";
                        break;
                    case SensorManager.SENSOR_STATUS_ACCURACY_LOW:
                        status = status + "Low";
                        break;
                    case SensorManager.SENSOR_STATUS_ACCURACY_MEDIUM:
                        status = status + "Medium";
                        break;
                    default:
                        break;
                }
                mCurrentStatus.setText(status);
            }
        }

        @Override
        public void onAccuracyChanged(Sensor sensor, int accuracy) {
            // Do nothing
        }
    };

    private String getSensorName(int type) {
        if (type < 0 || type >= SENSOR_NAME.length) {
            return "Sensor";
        }
        return SENSOR_NAME[type];
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.sensor_cali_new);

        mType = getIntent().getIntExtra(CALIBRAION_TYPE, GSENSOR);
        Log.d(TAG, String.format("onCreate(), type %d", mType));
        if (mType == GSENSOR) {
            mSensorType = Sensor.TYPE_ACCELEROMETER;
            setTitle(R.string.sensor_calibration_gsensor);
        } else if(mType == GYROSCOPE){
            mSensorType = Sensor.TYPE_GYROSCOPE;
            setTitle(R.string.sensor_calibration_gyroscope);
        } else if(mType == LIGHT){
            mSensorType = Sensor.TYPE_LIGHT;
            setTitle(R.string.sensor_calibration_light);
        }

        mStartCalibration = (Button) findViewById(R.id.button_sensor_calibration_start);
        mStartCalibration.setOnClickListener(this);

        mCurrentData = (TextView) findViewById(R.id.text_sensor_calibration_current_data);
        mCurrentData.setText("");
        mCurrentStatus = (TextView) findViewById(R.id.text_sensor_calibration_current_status);
        mCurrentStatus.setText("");
        mCaliData = (TextView) findViewById(R.id.text_sensor_calibration_cali_data);
        mCaliData.setText("");
        mCaliStatus = (TextView) findViewById(R.id.text_sensor_calibration_cali_status);
        mCaliStatus.setText("");

        mUiHandler = new Handler() {
            public void handleMessage(Message msg) {
                switch (msg.what) {
                    case MSG_GET_SUCCESS:
                        Log.d(TAG, "get success");
                        enableButtons(true);
                        mCaliData.setText(mData);
                        mCaliStatus.setText("Status: success");
                        break;
                    case MSG_SET_FAILURE:
                        Log.d(TAG, "set fail");
                        enableButtons(true);
                        mCaliStatus.setText("Status: fail");
                        break;
                    case MSG_GET_FAILURE:
                        Log.d(TAG, "get fail");
                        enableButtons(true);
                        mCaliStatus.setText("Status: fail");
                        break;
                    default:
                }
            }
        };

        mHandlerThread.start();
        mHandler = new Handler(mHandlerThread.getLooper()) {
            public void handleMessage(Message msg) {
                if(MSG_START_CALIBRARION == msg.what) {
                    startCalibration(msg.what);
                }
            }
        };
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.d(TAG, String.format("onResume(), type %d", mType));
        mSensorManager = (SensorManager) getSystemService(SENSOR_SERVICE);
        mSensor = mSensorManager.getDefaultSensor(mSensorType);
        if (mSensor != null) {
            Log.d(TAG, "registerListener");
            mSensorManager.registerListener(mSensorEventListener, mSensor,
                    SensorManager.SENSOR_DELAY_FASTEST);
        } else {
            Toast.makeText(this, getSensorName(mType) + " was not supported.",
                    Toast.LENGTH_SHORT).show();
            finish();
        }
    }

    @Override
    public void onPause() {
        Log.d(TAG, String.format("onPause(), type %d", mType));
        mSensorManager.unregisterListener(mSensorEventListener);
        mSensorManager = null;
        super.onPause();
    }

    @Override
    public void onClick(View arg0) {
        if (arg0.getId() == mStartCalibration.getId()) {
            Log.d(TAG, "start static calibration");
            mHandler.sendEmptyMessage(MSG_START_CALIBRARION);
        }
        enableButtons(false);
    }

    private boolean getCalibration() {
        Log.d(TAG, "getGsensorStaticCalibration()");
        float[] result = new float[3];
        int ret = 0;
        if (mType == GSENSOR) {
            ret = EmSensor.getGsensorStaticCalibration(result);
            Log.d(TAG, String.format("get Gsensor, ret %d, values %f, %f, %f",
                    ret, result[0], result[1], result[2]));
        } else if (mType == GYROSCOPE){
            ret = EmSensor.getGyroscopeStaticCalibration(result);
            Log.d(TAG, String.format("get Gyroscope, ret %d, values %f, %f, %f",
                    ret, result[0], result[1], result[2]));
        } else if (mType == LIGHT){
            ret = EmSensor.getLightStaticCalibration(result);
            Log.d(TAG, String.format("get Light, ret %d, values %f",ret, result[0]));
        }

        if (ret == EmSensor.RET_STATIC_CALI_SUCCESS) {
            if (mType == LIGHT) {
                mData = String.format(Locale.ENGLISH, "%+8.4f", result[0]);
            } else {
                mData = String.format(Locale.ENGLISH, "%+8.4f,%+8.4f,%+8.4f",
                        result[0], result[1], result[2]);
            }
            mUiHandler.sendEmptyMessage(MSG_GET_SUCCESS);
            return true;
        } else {
            mData = "";
            mUiHandler.sendEmptyMessage(MSG_GET_FAILURE);
            return false;
        }
    }

    private void startCalibration(int what) {
        int result = 0;
        Log.d(TAG, String.format("startCalibration(), operation %d", what));
        if (mType == GSENSOR) {
                result = EmSensor.startGsensorCalibration();
        } else if (mType == GYROSCOPE) {
                result = EmSensor.startGyroscopeCalibration();
        } else if (mType == LIGHT) {
            result = EmSensor.startLightCalibration();
        }
        Log.d(TAG, String.format("startCalibration(), ret %d", result));

        if (result == EmSensor.RET_STATIC_CALI_SUCCESS) {
            getCalibration();
        } else {
            mUiHandler.sendEmptyMessage(MSG_SET_FAILURE);
        }
    }

    private void enableButtons(boolean enable) {
        mStartCalibration.setClickable(enable);
    }

}

