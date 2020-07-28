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

package com.mediatek.engineermode;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.BatteryManager;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.os.SystemClock;
import android.text.format.DateUtils;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Calendar;

/**
 *
 * <p>
 * Description: Show current battery's information and record battery log.
 *
 */
public class BatteryLog extends Activity implements OnClickListener {

    private static final int EVENT_TICK = 1;
    private static final int EVENT_LOG_RECORD = 2;

    private static final int DELAY_TIME = 1000;
    private static final int MAX_NUMBER = 100;
    private static final int MAX_NUMBER_LENGTH = 3;
    private static final int FORMART_UPTIME = 1000;
    private static final int DEFAULT_INTERVAL = 10000;

    private static final int TEMP_CONVER_UNIT = 10;

    // battery attr

    private static final String TAG = "BatteryLog";

    private static final String CMD_CURRENT = "echo %1$d 0 > /proc/mtk_battery_cmd/current_cmd";
    private static final String CMD_CHARGING = "echo 0 %1$d > /proc/mtk_battery_cmd/current_cmd";
    private static final String CMD_SET_TEMPERATURE
            = "echo 1 1 %1$d > /proc/mtk_battery_cmd/battery_cmd";
    private static final String CMD_SET_TEMPERATURE2
            = "echo %1$d > /sys/devices/platform/battery/Battery_Temperature";

    private int mLogRecordInterval;
    private boolean mIsRecording = false;

    private File mLogFile;
    private File mBatteryLogFile;

    private TextView mTvStatus;
    private TextView mTvLevel;
    private TextView mTvScale;
    private TextView mTvHealth;
    private TextView mTvVoltage;
    private TextView mTvTemp;
    private TextView mTvTech;
    private TextView mUptime;
    private EditText mIntervalEdit;
    private Button mLogRecord;

    private Button mBtnChargeStop;
    private Button mBtnChargeRestart;
    private Button mBtnCurrentRise;
    private Button mBtnCurrentReduce;
    private Button mBtnTemperatureSet;
    private EditText mEditTemperature;

    private IntentFilter mIntentFilterBattery;
    private IntentFilter mIntentFilterSDCard;

    private OnClickListener mSettingClickListener = new OnClickListener() {
        @Override
        public void onClick(View view) {
            if (mBtnChargeStop == view) {
                enableBatteryCharging(false);
            } else if (mBtnChargeRestart == view) {
                enableBatteryCharging(true);
            } else if (mBtnCurrentRise == view) {
                adjustBatteryCurrent(true);
            } else if (mBtnCurrentReduce == view) {
                adjustBatteryCurrent(false);
            } else if (mBtnTemperatureSet == view) {
                setBatteryTemperature(mEditTemperature);
            }
        }
    };

    private void initBatterySettingUi() {
        mTvStatus = (TextView) findViewById(R.id.status_tv);
        mTvLevel = (TextView) findViewById(R.id.level_tv);
        mTvScale = (TextView) findViewById(R.id.scale_tv);
        mTvHealth = (TextView) findViewById(R.id.health_tv);
        mTvTech = (TextView) findViewById(R.id.tech_tv);
        mTvVoltage = (TextView) findViewById(R.id.voltage_tv);
        mTvTemp = (TextView) findViewById(R.id.temperature_tv);
        mUptime = (TextView) findViewById(R.id.bootup_time_tv);
        mIntervalEdit = (EditText) findViewById(R.id.log_record_interval_et);
        mBtnChargeStop = (Button) findViewById(R.id.battery_charging_stop_btn);
        mBtnChargeStop.setOnClickListener(mSettingClickListener);
        mBtnChargeRestart = (Button) findViewById(R.id.battery_charging_restart_btn);
        mBtnChargeRestart.setOnClickListener(mSettingClickListener);
        mBtnCurrentRise = (Button) findViewById(R.id.battery_current_rise_btn);
        mBtnCurrentRise.setOnClickListener(mSettingClickListener);
        mBtnCurrentReduce = (Button) findViewById(R.id.battery_current_reduce_btn);
        mBtnCurrentReduce.setOnClickListener(mSettingClickListener);
        mBtnTemperatureSet = (Button) findViewById(R.id.battery_temperature_set_btn);
        mBtnTemperatureSet.setOnClickListener(mSettingClickListener);

        mEditTemperature = (EditText) findViewById(R.id.battery_temperature_edit);
    }

    private void enableBatteryCharging(boolean charging) {
        int val = 1;
        if (charging) {
            val = 0;
        }
        String cmd = String.format(CMD_CHARGING, val);
        executeCmd(cmd, true);
    }

    private void adjustBatteryCurrent(boolean rise) {
        int val = 0;
        if (rise) {
            val = 1;
        }
        String cmd = String.format(CMD_CURRENT, val);
        executeCmd(cmd, true);
    }

    private void setBatteryTemperature(EditText input) {
        int val = 0;
        boolean valid = true;
        String inputStr = input.getText().toString();
        try {
            val = Integer.parseInt(inputStr);
        } catch (NumberFormatException e) {
            valid = false;
        }
        if (val < -20 || val > 80) {
            valid = false;
        }
        if (!valid) {
            Toast.makeText(this, "please input a valid number -20~80", Toast.LENGTH_SHORT).show();
            return;
        }

        String cmd = String.format(CMD_SET_TEMPERATURE, val);
        if (executeCmd(cmd, false)) {
            Toast.makeText(this, "Execute success", Toast.LENGTH_SHORT).show();
            return;
        }

        String cmd2 = String.format(CMD_SET_TEMPERATURE2, val);
        if (executeCmd(cmd2, false)) {
            Toast.makeText(this, "Execute success", Toast.LENGTH_SHORT).show();
            return;
        }
        Toast.makeText(this, "Execute fail", Toast.LENGTH_SHORT).show();

    }

    private boolean executeCmd(String cmd, boolean toastResult) {
        boolean result = true;
        try {
            int ret = ShellExe.execCommand(cmd);
            if (ret != ShellExe.RESULT_SUCCESS) {
                result = false;
            }
        } catch (IOException e) {
            Elog.e(TAG, "IOException: " + e.getMessage());
            result = false;
        }
        if (toastResult) {
            if (result) {
                Toast.makeText(this, "Execute success", Toast.LENGTH_SHORT).show();
            } else {
                Toast.makeText(this, "Execute fail", Toast.LENGTH_SHORT).show();
            }
        }
        return result;
    }

    private final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            if (msg.what == EVENT_TICK) {
                long uptime = SystemClock.elapsedRealtime();
                mUptime.setText(DateUtils.formatElapsedTime(uptime / FORMART_UPTIME));
                sendEmptyMessageDelayed(EVENT_TICK, DELAY_TIME);
            }
        }

    };

    private BroadcastReceiver mIntentReceiverBattery = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (Intent.ACTION_BATTERY_CHANGED.equals(action)) {

                int status = intent.getIntExtra(BatteryManager.EXTRA_STATUS,
                                                BatteryManager.BATTERY_STATUS_UNKNOWN);
                String strStatus;
                switch (status) {
                case BatteryManager.BATTERY_STATUS_CHARGING:
                    int pluggedType = intent.getIntExtra(BatteryManager.EXTRA_PLUGGED, 0);
                    String strPluggedType;
                    switch (pluggedType) {
                    case BatteryManager.BATTERY_PLUGGED_AC:
                        strPluggedType = getString(R.string.battery_status_charging_ac);
                        break;
                    case BatteryManager.BATTERY_PLUGGED_USB:
                        strPluggedType = getString(R.string.battery_status_charging_usb);
                        break;
                    case BatteryManager.BATTERY_PLUGGED_WIRELESS:
                        strPluggedType = getString(R.string.battery_status_charging_wireless);
                        break;
                    default:
                        strPluggedType = getString(R.string.battery_status_charging_unknown);
                        break;
                    }
                    strStatus = getString(R.string.battery_status_charging) + " " + strPluggedType;
                    break;


                case BatteryManager.BATTERY_STATUS_DISCHARGING:
                    strStatus = getString(R.string.battery_status_discharging);
                    break;
                case BatteryManager.BATTERY_STATUS_NOT_CHARGING:
                    strStatus = getString(R.string.battery_status_not_charging);
                    break;
                case BatteryManager.BATTERY_STATUS_FULL:
                    strStatus = getString(R.string.battery_status_full);
                    break;
                default:
                    strStatus = getString(R.string.battery_status_unknown);
                    break;
                }

                mTvStatus.setText(strStatus);

                mTvLevel.setText(
                    Integer.toString(intent.getIntExtra(BatteryManager.EXTRA_LEVEL, 0)));
                mTvScale.setText(
                    Integer.toString(intent.getIntExtra(BatteryManager.EXTRA_SCALE, 0)));
                int health = intent.getIntExtra(
                    BatteryManager.EXTRA_HEALTH, BatteryManager.BATTERY_HEALTH_UNKNOWN);
                String strHealth;
                switch (health) {
                case BatteryManager.BATTERY_HEALTH_GOOD:
                    strHealth = getString(R.string.battery_health_good);
                    break;
                case BatteryManager.BATTERY_HEALTH_OVERHEAT:
                    strHealth = getString(R.string.battery_health_overheat);
                    break;
                case BatteryManager.BATTERY_HEALTH_DEAD:
                    strHealth = getString(R.string.battery_health_dead);
                    break;
                case BatteryManager.BATTERY_HEALTH_OVER_VOLTAGE:
                    strHealth = getString(R.string.battery_health_over_voltage);
                    break;
                case BatteryManager.BATTERY_HEALTH_UNSPECIFIED_FAILURE:
                    strHealth = getString(R.string.battery_health_unspecified_failure);
                    break;
                case BatteryManager.BATTERY_HEALTH_COLD:
                    strHealth = getString(R.string.battery_health_cold);
                    break;
                default:
                    strHealth = getString(R.string.battery_health_unknown);
                    break;
                }

                mTvHealth.setText(strHealth);
                mTvVoltage.setText(
                        Integer.toString(intent.getIntExtra(BatteryManager.EXTRA_VOLTAGE, 0))
                        + " " + getString(R.string.battery_voltage_units));
                mTvTemp.setText(genTempStr(intent.getIntExtra(BatteryManager.EXTRA_TEMPERATURE, 0))
                        + getString(R.string.battery_temperature_units));
                mTvTech.setText(intent.getStringExtra(BatteryManager.EXTRA_TECHNOLOGY));
            }
        }
    };

    private String genTempStr(int temp) {
        int tens = temp / TEMP_CONVER_UNIT;
        return new String("" + tens + "." + (temp - TEMP_CONVER_UNIT * tens));
    }

    private BroadcastReceiver mIntentReceiverSDCard = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(Intent.ACTION_MEDIA_BAD_REMOVAL)
                    || action.equals(Intent.ACTION_MEDIA_REMOVED)
                    || action.equals(Intent.ACTION_MEDIA_EJECT)) {
                if (!mIsRecording) {
                    return;
                }
                mIsRecording = false;
                mLogHandler.removeMessages(EVENT_LOG_RECORD);
                mLogRecord.setText(R.string.battery_info_record_start);
                AlertDialog.Builder builder = new AlertDialog.Builder(BatteryLog.this);
                builder.setTitle("SD card error");
                builder.setMessage("SD card has been removed.");
                builder.setPositiveButton("OK", null);
                builder.create().show();
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.battery_log);

        mLogRecord = (Button) findViewById(R.id.log_record_btn);
        mLogRecord.setOnClickListener(this);

        // create the IntentFilter that will be used to listen
        // to battery status broadcasts
        mIntentFilterBattery = new IntentFilter();
        mIntentFilterBattery.addAction(Intent.ACTION_BATTERY_CHANGED);

        mIntentFilterSDCard = new IntentFilter();
        mIntentFilterSDCard.addAction(Intent.ACTION_MEDIA_BAD_REMOVAL);
        mIntentFilterSDCard.addAction(Intent.ACTION_MEDIA_REMOVED);
        mIntentFilterSDCard.addAction(Intent.ACTION_MEDIA_EJECT);
        mIntentFilterSDCard.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
        mIntentFilterSDCard.addDataScheme("file");

        // check whether the sdcard exists, if yes, set up batterylog directory,
        // and if not, notify user to plug in it
        mLogRecordInterval = DEFAULT_INTERVAL;
        File sdcard = null;
        if (!Environment.getExternalStorageState().equals(Environment.MEDIA_REMOVED)) {
            sdcard = Environment.getExternalStorageDirectory();
            mBatteryLogFile = new File(sdcard.getParent() + "/" + sdcard.getName()
                    + "/batterylog/");
            Elog.i(TAG, sdcard.getParent() + "/" + sdcard.getName() + "/batterylog/");
            if (!mBatteryLogFile.isDirectory()) {
                mBatteryLogFile.mkdirs();
            }
        }

        initBatterySettingUi();
    }

    @Override
    protected void onStart() {
        super.onStart();
        if (!mIsRecording) {
            mHandler.sendEmptyMessageDelayed(EVENT_TICK, DELAY_TIME);
            registerReceiver(mIntentReceiverBattery, mIntentFilterBattery);
            registerReceiver(mIntentReceiverSDCard, mIntentFilterSDCard);
        }


    }

    @Override
    protected void onStop() {
        if (!mIsRecording) {
            mHandler.removeMessages(EVENT_TICK);
            // we are no longer on the screen stop the observers
            unregisterReceiver(mIntentReceiverBattery);
            unregisterReceiver(mIntentReceiverSDCard);
        }
        super.onStop();
    }

    @Override
    public void onBackPressed() {
        Elog.i(TAG, "onBackPressed");
        if (mIsRecording) {
            Toast.makeText(this, getString(R.string.battery_stop_record_warning),
                    Toast.LENGTH_SHORT).show();
        } else {
            super.onBackPressed();
        }
    }
    /**
     * @param arg0
     *            view
     */
    public void onClick(View arg0) {
        if (arg0.getId() == mLogRecord.getId()) {
            if (!mIsRecording) {
                String storageState = Environment.getExternalStorageState();
                if (storageState.equals(Environment.MEDIA_REMOVED)
                        || storageState.equals(Environment.MEDIA_BAD_REMOVAL)
                        || storageState.equals(Environment.MEDIA_UNMOUNTED)) {
                    AlertDialog.Builder builder = new AlertDialog.Builder(this);
                    builder.setTitle("SD Card not available");
                    builder.setMessage("Please insert an SD Card.");
                    builder.setPositiveButton("OK", null);
                    builder.create().show();
                    return;
                }

                String state = Environment.getExternalStorageState();
                Elog.i(TAG, "Environment.getExternalStorageState() is : " + state);

                if (Environment.getExternalStorageState().equals(Environment.MEDIA_SHARED)) {
                    AlertDialog.Builder builder = new AlertDialog.Builder(this);
                    builder.setTitle("sdcard is busy");
                    builder.setMessage("Sorry, your SD card is busy.");
                    builder.setPositiveButton("OK", null);
                    builder.create().show();
                    return;
                }
                // check if the EditText control has no content, if not, check
                // the content whether is right

                if (MAX_NUMBER_LENGTH < mIntervalEdit.getText().toString().length()
                        || 0 == mIntervalEdit.getText().toString().length()) {
                    Toast.makeText(this,
                            "The input is not correct. Please input the number between 1 and 100.",
                            Toast.LENGTH_LONG).show();
                    return;
                }
                if (Integer.valueOf(mIntervalEdit.getText().toString()) > MAX_NUMBER
                        || Integer.valueOf(mIntervalEdit.getText().toString()) < 1) {
                    Toast.makeText(this,
                            "The input is not correct. Please input the number between 1 and 100.",
                            Toast.LENGTH_LONG).show();
                    return;
                }
                mLogRecordInterval = Integer.valueOf(mIntervalEdit.getText().toString())
                        * FORMART_UPTIME;
                Elog.i(TAG, String.valueOf(mLogRecordInterval));

                mLogRecord.setText(R.string.battery_info_record_stop);

                // Create a new file under the "/sdcard/batterylog" path
                Calendar rightNow = Calendar.getInstance();
                SimpleDateFormat fmt = new SimpleDateFormat("yyyyMMddhhmmss");
                String sysDateTime = fmt.format(rightNow.getTime());
                String fileName = "";
                fileName = fileName + sysDateTime;
                fileName = fileName + ".txt";
                Elog.i(TAG, fileName);

                // mLogFile = new File("/sdcard/batterylog/" + fileName);
                mLogFile = new File(Environment.getExternalStorageDirectory().getAbsolutePath()
                        + File.separator + "batterylog" + File.separator + fileName);
                try {
                    mLogFile.createNewFile();
                    String batteryInfoLable = "Battery status, level, scale, health, voltage, "
                            + "temperature, technology, time since boot:\n";
                    FileWriter fileWriter = new FileWriter(mLogFile);
                    fileWriter.write(batteryInfoLable);
                    fileWriter.flush();
                    fileWriter.close();
                } catch (IOException e1) {
                    e1.printStackTrace();
                }

                mLogHandler.sendEmptyMessageDelayed(EVENT_LOG_RECORD, DELAY_TIME);

                mIsRecording = true;
            } else {
                mLogRecord.setText(R.string.battery_info_record_start);
                mLogHandler.removeMessages(EVENT_LOG_RECORD);
                mIsRecording = false;
                AlertDialog.Builder builder = new AlertDialog.Builder(this);
                builder.setTitle("BatteryLog Saved");
                builder.setMessage("BatteryLog has been saved as " + mLogFile.getAbsolutePath());
                builder.setPositiveButton("OK", null);
                builder.create().show();
            }
        }
    }

    private Handler mLogHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            if (msg.what == EVENT_LOG_RECORD) {
                Elog.i(TAG, "Record one time");
                writeCurrentBatteryInfo();
                sendEmptyMessageDelayed(EVENT_LOG_RECORD, mLogRecordInterval);
            }
        }

        private void writeCurrentBatteryInfo() {
            String logContent = "";
            logContent = logContent + mTvStatus.getText() + ", "
                    + mTvLevel.getText() + ", " + mTvScale.getText() + ", "
                    + mTvHealth.getText() + ", " + mTvVoltage.getText() + ", "
                    + mTvTemp.getText() + ", " + mTvTech.getText() + ", "
                    + mUptime.getText() + "\n";
            FileWriter fileWriter = null;
            try {
                fileWriter = new FileWriter(mLogFile, true);
                fileWriter.write(logContent);
                fileWriter.flush();
            } catch (IOException e) {
                e.printStackTrace();
            } finally {
                if (null != fileWriter) {
                    try {
                        fileWriter.close();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            }
        }
    };
}
