/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.ygps;

import android.app.Dialog;
import android.app.ProgressDialog;
import android.app.TabActivity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.content.res.Resources.NotFoundException;
import android.location.GnssStatus;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.location.OnNmeaMessageListener;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.PowerManager;
import android.os.StrictMode;
import android.support.v4.app.ActivityCompat;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TabHost;
import android.widget.TabHost.OnTabChangeListener;
import android.widget.TextView;
import android.widget.Toast;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Collections;
import java.util.Date;


/**
 * Activity for YGPS main view.
 *
 */
public class YgpsActivity extends TabActivity implements
        ActivityCompat.OnRequestPermissionsResultCallback {

    private static final String TAG = "YGPS/Mainview";
    private static final String TAG_BG = "YGPS/Background";
    private static final String COMMAND_END = "*";
    private static final String COMMAND_START = "$";
    private static final int LOCATION_MAX_LENGTH = 12;
    private static final String SHARED_PREF_KEY_BG = "RunInBG";
    private static final int MESSAGE_ARG_1 = 1;
    private static final int MESSAGE_ARG_0 = 0;
    private static final int INPUT_VALUE_MAX = 999;
    private static final int INPUT_VALUE_MIN = 0;
    private static final int ONE_SECOND = 1000;
    private static final int HANDLE_MSG_DELAY = 200;
    private static final int HANDLE_MSG_DELAY_300 = 300;
    private static final int LOC_TOAST_PERIOD = 5000;
    private static final double LATITUDE_MIN = -90.0;
    private static final double LATITUDE_MAX = 90.0;
    private static final double LONGITUDE_MIN = -180.0;
    private static final double LONGITUDE_MAX = 180.0;
    private static final double RATE_68 = 0.68;
    private static final double RATE_95 = 0.95;

    private static final int COUNT_PRECISION = 500;
    private static final int EXCEED_SECOND = 999;

    private static final int HANDLE_COUNTER = 1000;
    private static final int HANDLE_UPDATE_RESULT = 1001;
    private static final int HANDLE_CLEAR = 1002;
    private static final int HANDLE_CHECK_SATEREPORT = 1003;
    private static final int HANDLE_SET_CURRENT_TIMES = 1030;
    private static final int HANDLE_START_BUTTON_UPDATE = 1040;
    private static final int HANDLE_SET_COUNTDOWN = 1050;
    private static final int HANDLE_SET_MEANTTFF = 1070;
    private static final int HANDLE_EXCEED_PERIOD = 1080;
    private static final int HANDLE_SET_PARAM_RECONNECT = 1090;
    private static final int CMD_TYPE_JAMMINGSCAN = 1;
    private static final int CMD_TYPE_GETVERSION = 2;
    private static final int CMD_TYPE_OTHERS = 1103;
    private static final int HANDLE_RESTART_UPDATE = 1200;
    private static final int HANDLE_ENABLE_BUTTON = 1204;

    private static final int DIALOG_WAITING_FOR_STOP = 0;
    private static final int DIALOG_WAITING_FOR_START = 1;
    private static final String GPS_EXTRA_POSITION = "position";
    private static final String GPS_EXTRA_EPHEMERIS = "ephemeris";
    private static final String GPS_EXTRA_TIME = "time";
    private static final String GPS_EXTRA_IONO = "iono";
    private static final String GPS_EXTRA_UTC = "utc";
    private static final String GPS_EXTRA_HEALTH = "health";
    private static final String GPS_EXTRA_ALL = "all";
    private static final String GPS_EXTRA_RTI = "rti";
    private static final String GPS_EXTRA_A1LMANAC = "almanac";
    private static final String EXTRA_BG = "bg";
    private static final int RESPONSE_ARRAY_LENGTH = 4;
    private static final int SATE_RATE_TIMEOUT = 3;

    private static final int GPS_TEST_HOT_START = 0;
    private static final int GPS_TEST_WARM_START = 1;
    private static final int GPS_TEST_COLD_START = 2;
    private static final int GPS_TEST_FULL_START = 3;

    private boolean mLogHidden = false;

    private int mTtffValue = 0;
    private int mTotalTimes = 0;
    private int mCurrentTimes = 0;
    private int mTestInterval = 0;
    private int mTestType = 0;
    private int m68Ttff = 0;
    private int m95Ttff = 0;
    private int mMaxTtff = 0;
    private float mMeanTTFF = 0f;
    private double mTestLat = 0;
    private double mTestLon = 0;
    private double m68Cep = 0;
    private double m95Cep = 0;
    private double mMeanCep = 0;
    private double mMaxCep = 0;

    private boolean mShowLoc = false;
    private boolean mStartNmeaRecord = false;
    private boolean mFirstFix = false;
    private boolean mIsNeed3DFix = false;
    private boolean mIsTestRunning = false;
    private boolean mIsExit = false;
    private boolean mNeedCalCep = false;
    private boolean mTestTtffValid = false;
    private boolean mTestCepValid = false;

    private boolean mIsRunInBg = false;
    private boolean mShowFirstFixLocate = true;
    private boolean mIsShowVersion = false;
    private int mSateReportTimeOut = 0;

    private ClientSocket mSocketClient = null;

    private SatelLocationView mSatelliteView = null;
    private SatelSignalChartView mSignalView = null;
    private LocationManager mLocationManager = null;
    private YgpsWakeLock mYgpsWakeLock = null;
    private Location mLastLocation = null;
    private Button mBtnColdStart = null;
    private Button mBtnWarmStart = null;
    private Button mBtnHotStart = null;
    private Button mBtnFullStart = null;
    private Button mBtnReStart = null;
    private CheckBox mCbEpo = null;
    private CheckBox mCbQepo = null;
    private CheckBox mCbGpsLog = null;
    private Button mBtnNmeaStart = null;
    private Button mBtnNMEAStop = null;
    private Button mBtnGpsTestStart = null;
    private Button mBtnGpsTestStop = null;
    private EditText mEtTestTimes = null;
    private CheckBox mCbNeed3DFix = null;
    private EditText mEtTestInterval = null;
    private Spinner mTestSpinner = null;
    private EditText mEtTestLat = null;
    private EditText mEtTestLon = null;
    private TextView mTvTestLastCep = null;
    private TextView mTvTestMeanCep = null;
    private TextView mTvTest68Cep = null;
    private TextView mTvTest95Cep = null;
    private TextView mTvTestMaxCep = null;
    private TextView mTvTest68Ttff = null;
    private TextView mTvTest95Ttff = null;
    private TextView mTvTestMaxTtff = null;
    private TextView mTvCurTimes = null;
    private TextView mTvCountDown = null;
    private TextView mTvStatus = null;

    private YgpsService mYgpsService = null;

    private AutoTestThread mAutoTestThread = null;
    private String mProvider = "";
    private String mStatus = "";
    private boolean mStopPressedHandling = false;
    private boolean mStartPressedHandling = false;
    private TextView mTvNmeaLog = null;
    private Button mBtnGpsJamming = null;
    private EditText mEtGpsJammingTimes = null;
    private Toast mPrompt = null;
    private long mLocUpdateTime = 0;
    private static final String[] sTypeList =
        {"Hot start", "Warm start", "Cold start", "Full start"};
    private ArrayList<Integer> mTestTtffList = new ArrayList<Integer>();
    private ArrayList<Double> mTestCepList = new ArrayList<Double>();
    private SatelliteInfoManager mSatelInfoManager = null;
    private NmeaParser mNmeaParser = null;
    private NmeaParser.NmeaUpdateViewListener mNmeaUpdateListener = null;
    private boolean mRestarting = false;
    private boolean mNmeaFixed = false;
    private volatile boolean mIsForceStopGpsTest = false;
    private Intent mServiceIntent = null;

    private MnldConn mMnldConn = new MnldConn();
    private ServiceConnection mServiceConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName component, IBinder service) {
            Log.d(TAG, "YGPS onServiceConnected");
            mYgpsService = ((YgpsService.LocalYgpsBinder) service).getServiceInstance();
            if (mYgpsService == null) {
                throw new RuntimeException("fail to get bound YgpsService from connection");
            }
            if (mYgpsService.isForeground()) {
                mYgpsService.dismissForeground();
            }
        }

        public void onServiceDisconnected(ComponentName component) {
            Log.w(TAG, "YGPS onServiceDisconnected");
            mYgpsService = null;
        }
    };


    private boolean isGpsRestarting() {
        return mRestarting;
    }

    private PermissionManager mPermissionManager;
    /**
     * Store satellite status.
     *
     * @param adapter The list contains satellite status
     */
    private void setSatelliteStatus(NmeaSatelliteAdapter adapter) {
        if (null == adapter) {
            mSatelInfoManager.clearSatelInfos();
        } else {
            mSatelInfoManager.updateSatelliteInfo(adapter);
        }
        if (!mLogHidden) {
            Log.i(TAG, "setSatelliteStatus " + mSatelInfoManager.toString());
        }
        mNmeaFixed = mSatelInfoManager.isUsedInFix(SatelliteInfoManager.PRN_ANY);
        if (!mNmeaFixed) {
            clearLayout();
            mStatus = getString(R.string.gps_status_unavailable);
        } else {
            mStatus = getString(R.string.gps_status_available);
        }
        mTvStatus.setText(mStatus);
        Log.i(TAG, "setSatelliteStatus: status:" + mStatus);

        mSatelliteView.requestUpdate(mSatelInfoManager);
        mSignalView.requestUpdate(mSatelInfoManager);
    }

    private void setSatellitesLayout() {
        mSatelliteView = (SatelLocationView) findViewById(R.id.sky_view);
        mSignalView = (SatelSignalChartView) findViewById(R.id.signal_view);
    }

    private void initEpoUI() {
        mCbEpo = (CheckBox) findViewById(R.id.cb_gps_epo);
        mCbQepo = (CheckBox) findViewById(R.id.cb_gps_qepo);
        String ss = GpsMnlSetting.getMnlProp(GpsMnlSetting.KEY_GPS_EPO,
                GpsMnlSetting.PROP_VALUE_BOTH_EPO);
        if (ss.equals(GpsMnlSetting.PROP_VALUE_BOTH_EPO)) {
            mCbEpo.setChecked(true);
            mCbQepo.setChecked(true);
        } else if (ss.equals(GpsMnlSetting.PROP_VALUE_ONLY_EPO)) {
            mCbEpo.setChecked(true);
        } else if (ss.equals(GpsMnlSetting.PROP_VALUE_ONLY_QEPO)) {
            mCbQepo.setChecked(true);
        }
        mCbEpo.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) {
                    GpsMnlSetting.setMnlProp(mMnldConn, GpsMnlSetting.KEY_GPS_EPO,
                            mCbQepo.isChecked() ? GpsMnlSetting.PROP_VALUE_BOTH_EPO
                                    : GpsMnlSetting.PROP_VALUE_ONLY_EPO);
                } else {
                    GpsMnlSetting.setMnlProp(mMnldConn, GpsMnlSetting.KEY_GPS_EPO,
                            mCbQepo.isChecked() ? GpsMnlSetting.PROP_VALUE_ONLY_QEPO
                                    : GpsMnlSetting.PROP_VALUE_NONE_EPO);
                }
            }
        });
        mCbQepo.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) {
                    GpsMnlSetting.setMnlProp(mMnldConn, GpsMnlSetting.KEY_GPS_EPO,
                            mCbEpo.isChecked() ? GpsMnlSetting.PROP_VALUE_BOTH_EPO
                                    : GpsMnlSetting.PROP_VALUE_ONLY_QEPO);
                } else {
                    GpsMnlSetting.setMnlProp(mMnldConn, GpsMnlSetting.KEY_GPS_EPO,
                            mCbEpo.isChecked() ? GpsMnlSetting.PROP_VALUE_ONLY_EPO
                                    : GpsMnlSetting.PROP_VALUE_NONE_EPO);
                }
            }
        });
    }

    private void setInfoLayout() {
        mTvStatus = (TextView) findViewById(R.id.tv_status);
        mBtnColdStart = (Button) findViewById(R.id.btn_cold);
        mBtnColdStart.setOnClickListener(mBtnClickListener);
        mBtnWarmStart = (Button) findViewById(R.id.btn_warm);
        mBtnWarmStart.setOnClickListener(mBtnClickListener);
        mBtnHotStart = (Button) findViewById(R.id.btn_hot);
        mBtnHotStart.setOnClickListener(mBtnClickListener);
        mBtnFullStart = (Button) findViewById(R.id.btn_full);
        mBtnFullStart.setOnClickListener(mBtnClickListener);
        mBtnReStart = (Button) findViewById(R.id.btn_restart);
        mBtnReStart.setOnClickListener(mBtnClickListener);
        mCbGpsLog = (CheckBox) findViewById(R.id.cb_gps_log);
        String ss = GpsMnlSetting.getMnlProp(GpsMnlSetting.KEY_GPSLOG_ENABLED,
                GpsMnlSetting.PROP_VALUE_0);
        mCbGpsLog.setChecked(ss.equals(GpsMnlSetting.PROP_VALUE_1));
        mCbGpsLog.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {

            @Override
            public void onCheckedChanged(CompoundButton buttonView,
                    boolean isChecked) {
                // TODO Auto-generated method stub
                Log.i(TAG, "gpslog state changed to " + isChecked);
                GpsMnlSetting.setMnlProp(mMnldConn, GpsMnlSetting.KEY_GPSLOG_ENABLED,
                        isChecked ? GpsMnlSetting.PROP_VALUE_1 : GpsMnlSetting.PROP_VALUE_0);
            }
        });
        initEpoUI();
    }

    private void setNmeaLayout() {
        mTvNmeaLog = (TextView) findViewById(R.id.tv_nmea_log);
        mBtnNmeaStart = (Button) findViewById(R.id.btn_nmea_start);
        mBtnNmeaStart.setOnClickListener(mBtnClickListener);
        mBtnNMEAStop = (Button) findViewById(R.id.btn_nmea_stop);
        mBtnNMEAStop.setOnClickListener(mBtnClickListener);
        if (mLogHidden) {
            mBtnNmeaStart.setVisibility(View.GONE);
            mBtnNMEAStop.setVisibility(View.GONE);
        }
        mBtnNMEAStop.setEnabled(false);
    }

    private void setGpsTestLayout() {
        mTvTestLastCep = (TextView) findViewById(R.id.tv_last_cep);
        mTvTestMeanCep = (TextView) findViewById(R.id.tv_mean_cep);
        mTvTest68Cep = (TextView) findViewById(R.id.tv_percent_68_cep);
        mTvTest95Cep = (TextView) findViewById(R.id.tv_percent_95_cep);
        mTvTestMaxCep = (TextView) findViewById(R.id.tv_max_cep);
        mTvTest68Ttff = (TextView) findViewById(R.id.tv_percent_68_ttff);
        mTvTest95Ttff = (TextView) findViewById(R.id.tv_percent_95_ttff);
        mTvTestMaxTtff = (TextView) findViewById(R.id.tv_max_ttff);
        mTvCurTimes = (TextView) findViewById(R.id.tv_current_times);
        mTvCountDown = (TextView) findViewById(R.id.tv_reconnect_countdown);
        mBtnGpsTestStart = (Button) findViewById(R.id.btn_gps_test_start);
        mBtnGpsTestStart.setOnClickListener(mBtnClickListener);
        mBtnGpsTestStop = (Button) findViewById(R.id.btn_gps_test_stop);
        mBtnGpsTestStop.setOnClickListener(mBtnClickListener);
        mBtnGpsTestStop.setEnabled(false);
        mEtTestTimes = (EditText) findViewById(R.id.et_gps_test_times);
        mCbNeed3DFix = (CheckBox) findViewById(R.id.cb_need_3d_fix);
        mEtTestInterval = (EditText) findViewById(R.id.et_gps_test_interval);
        mEtTestLat = (EditText) findViewById(R.id.et_latitude);
        mEtTestLon = (EditText) findViewById(R.id.et_longitude);
        mBtnGpsJamming = (Button) findViewById(R.id.btn_gps_jamming_scan);
        mBtnGpsJamming.setOnClickListener(mBtnClickListener);
        mEtGpsJammingTimes = (EditText) findViewById(R.id.et_gps_jamming_times);
        mEtGpsJammingTimes.setText(getString(R.string.jamming_scan_times));
        mEtGpsJammingTimes.setSelection(mEtGpsJammingTimes.getText().length());
        mTestSpinner = (Spinner) findViewById(R.id.start_type_Spinner);
        ArrayAdapter<String> testAdapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item);
        testAdapter
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        for (int i = 0; i < sTypeList.length; i++) {
            testAdapter.add(sTypeList[i]);
        }
        mTestSpinner.setAdapter(testAdapter);
        mTestSpinner.setOnItemSelectedListener(new OnItemSelectedListener() {

            public void onItemSelected(AdapterView<?> arg0, View arg1,
                    int arg2, long arg3) {
                mTestType = arg2;
                Log.i(TAG, "The mTestType is : " + arg2);
            }

            public void onNothingSelected(AdapterView<?> arg0) {
            }
        });
    }
    /**
     * Component initial.
     */
    private void setLayout() {
        setSatellitesLayout();
        setInfoLayout();
        setNmeaLayout();
        setGpsTestLayout();
    }

    /**
     * Clear location information.
     */
    private void clearLayout() {
        // clear all information in layout
        ((TextView) findViewById(R.id.tv_date)).setText(R.string.empty);
        ((TextView) findViewById(R.id.tv_time)).setText(R.string.empty);
        ((TextView) findViewById(R.id.tv_latitude)).setText(R.string.empty);
        ((TextView) findViewById(R.id.tv_longitude)).setText(R.string.empty);
        ((TextView) findViewById(R.id.tv_altitude)).setText(R.string.empty);
        ((TextView) findViewById(R.id.tv_accuracy)).setText(R.string.empty);
        ((TextView) findViewById(R.id.tv_bearing)).setText(R.string.empty);
        ((TextView) findViewById(R.id.tv_speed)).setText(R.string.empty);
        ((TextView) findViewById(R.id.tv_distance)).setText(R.string.empty);
        if (mShowFirstFixLocate) {
            ((TextView) findViewById(R.id.first_longtitude_text))
                    .setText(R.string.empty);
            ((TextView) findViewById(R.id.first_latitude_text))
                    .setText(R.string.empty);
        }
    }

    /**
     * Refresh GPS auto test UI .
     */
    private void refreshATViewState(Boolean start) {
        mBtnGpsTestStart.setEnabled(!start);
        mEtTestTimes.setEnabled(!start);
        mCbNeed3DFix.setEnabled(!start);
        mEtTestInterval.setEnabled(!start);
        mEtTestLat.setEnabled(!start);
        mEtTestLon.setEnabled(!start);
        mBtnGpsTestStop.setEnabled(start);
        if (start) {
            clearLayout();
        }
    }

    boolean checkCepParamValid() {
        // check longitude and latitude
        mNeedCalCep = true;
        String strTestLat = mEtTestLat.getText().toString();
        if ((strTestLat == null) || (strTestLat.isEmpty())) {
            mNeedCalCep = false;
        } else {
            double latitude = Double.valueOf(strTestLat);
            if ((latitude < LATITUDE_MIN) || (latitude > LATITUDE_MAX)) {
                Toast.makeText(YgpsActivity.this,
                        R.string.toast_input_latitude_range, Toast.LENGTH_LONG)
                        .show();
                mBtnGpsTestStart.setEnabled(true);
                mNeedCalCep = false;
                return false;
            }
            mTestLat = latitude;
        }

        String strTestLon = mEtTestLon.getText().toString();
        if ((strTestLon == null) || (strTestLon.isEmpty())) {
            mNeedCalCep = false;
        } else {
            double longitude = Double.valueOf(strTestLon);
            if ((longitude < LONGITUDE_MIN) || (longitude > LONGITUDE_MAX)) {
                Toast.makeText(YgpsActivity.this,
                        R.string.toast_input_longitude_range, Toast.LENGTH_LONG)
                        .show();
                mBtnGpsTestStart.setEnabled(true);
                mNeedCalCep = false;
                return false;
            }
            mTestLon = longitude;
        }
        return true;
    }

    boolean checkAutoTestParamValid() {
        // check Times
        if (null != mEtTestTimes) {
            if (0 == mEtTestTimes.getText().length()) {
                Toast.makeText(YgpsActivity.this, R.string.toast_input_times,
                        Toast.LENGTH_LONG).show();
                mBtnGpsTestStart.setEnabled(true);
                return false;
            } else {
                Integer nTimes = Integer.valueOf(mEtTestTimes.getText()
                        .toString());
                if (nTimes.intValue() < INPUT_VALUE_MIN
                        || nTimes.intValue() > INPUT_VALUE_MAX) {
                    Toast.makeText(YgpsActivity.this,
                            R.string.toast_input_range, Toast.LENGTH_LONG)
                            .show();
                    mBtnGpsTestStart.setEnabled(true);
                    return false;
                }
                mTotalTimes = nTimes.intValue();
            }
        }

        // check Interval
        if (null != mEtTestInterval) {
            if (0 == mEtTestInterval.getText().length()) {
                Toast.makeText(YgpsActivity.this,
                        R.string.toast_input_interval, Toast.LENGTH_LONG)
                        .show();
                mBtnGpsTestStart.setEnabled(true);
                return false;
            } else {
                Integer nInterval = Integer.valueOf(mEtTestInterval.getText()
                        .toString());
                if (nInterval.intValue() < INPUT_VALUE_MIN
                        || nInterval.intValue() > INPUT_VALUE_MAX) {
                    Toast.makeText(YgpsActivity.this,
                            R.string.toast_input_range, Toast.LENGTH_LONG)
                            .show();
                    mBtnGpsTestStart.setEnabled(true);
                    return false;
                }
                mTestInterval = nInterval.intValue();
            }
        }
        return true;
    }

    /**
     * Start GPS auto test.
     */
    private void startGPSAutoTest() {

        if (!checkAutoTestParamValid()) {
            removeDialog(DIALOG_WAITING_FOR_START);
            return;
        }
        if (!checkCepParamValid()) {
            removeDialog(DIALOG_WAITING_FOR_START);
            return;
        }

        // need 3D fix? check it
        if (null != mCbNeed3DFix) {
            mIsNeed3DFix = mCbNeed3DFix.isChecked();
        }
        mIsForceStopGpsTest = false;
        mTestTtffList.clear();
        mTestCepList.clear();
        resetTestView();

        if (!mStartPressedHandling) {
            mStartPressedHandling = true;
            refreshATViewState(true);
            // original code
            mAutoTestThread = new AutoTestThread();
            if (null != mAutoTestThread) {
                Log.i(TAG, "3D fix:" + mIsNeed3DFix);

                mAutoTestThread.start();
            }

        }

    }

    /**
     * Stop GPS auto test.
     */
    private void stopGPSAutoTest() {
        resetTestParam();
        setTestParam();
    }

    /**
     * force Stop GPS Auto Test.
     */
    private void forceStopGpsAutoTest() {
        synchronized (this) {
            mIsForceStopGpsTest = true;
        }
        resetTestParam();
        mHandler.removeMessages(HANDLE_RESTART_UPDATE);
        refreshATViewState(false);
        mStartPressedHandling = false;
        mStopPressedHandling = false;
    }

    private void calTestResult() {
        int ttffNumber = mTestTtffList.size();
        Log.v(TAG, "ttffNumber:" + ttffNumber);
        for (Integer i:mTestTtffList) {
            Log.v(TAG, "value:" + i);
        }
        if (ttffNumber > 0) {
            Collections.sort(mTestTtffList);
            int index68 = (int) Math.floor(ttffNumber * RATE_68);
            int index95 = (int) Math.floor(ttffNumber * RATE_95);
            m68Ttff = mTestTtffList.get(index68);
            m95Ttff = mTestTtffList.get(index95);
            mMaxTtff = mTestTtffList.get(ttffNumber - 1);
            Log.i(TAG, "index68:" + index68 + "index95:" + index95 +
                    "m68Ttff:" + m68Ttff + "m95Ttff:" + m95Ttff + "mMaxTtff:" + mMaxTtff);
            mTestTtffValid = true;
        } else {
            mTestTtffValid = false;
        }
        int cepNumber = mTestCepList.size();
        Log.v(TAG, "cepNumber:" + cepNumber);
        for (Double d:mTestCepList) {
            Log.v(TAG, "value:" + d);
        }
        if (cepNumber > 0) {
            Collections.sort(mTestCepList);
            int index68 = (int) Math.floor(cepNumber * RATE_68);
            int index95 = (int) Math.floor(cepNumber * RATE_95);
            m68Cep = mTestCepList.get(index68);
            m95Cep = mTestCepList.get(index95);
            mMaxCep = mTestCepList.get(cepNumber - 1);
            Log.i(TAG, "index68:" + index68 + "index95:" + index95 +
                    "m68Cep:" + m68Cep + "m95Cep:" + m95Cep + "mMaxCep:" + mMaxCep);
            mTestCepValid = true;
        } else {
            mTestCepValid = false;
        }
    }

    private void showTestResult() {
        if (mTestTtffValid) {
            mTvTestMaxTtff.setText(String.valueOf(mMaxTtff) + getString(R.string.time_unit_ms));
            mTvTest68Ttff.setText(String.valueOf(m68Ttff) + getString(R.string.time_unit_ms));
            mTvTest95Ttff.setText(String.valueOf(m95Ttff) + getString(R.string.time_unit_ms));
        }
        if (mTestCepValid) {
            mTvTestMaxCep.setText(String.valueOf(mMaxCep) + getString(R.string.cep_unit_m));
            mTvTest68Cep.setText(String.valueOf(m68Cep) + getString(R.string.cep_unit_m));
            mTvTest95Cep.setText(String.valueOf(m95Cep) + getString(R.string.cep_unit_m));
        }

    }
    /**
     * Reset GPS auto test parameters.
     */
    private void resetTestParam() {
        mIsNeed3DFix = false;
        mTotalTimes = 0;
        mCurrentTimes = 0;
        mTestInterval = 0;
        mMeanTTFF = 0f;
        mIsTestRunning = false;
        mNeedCalCep = false;
    }

    /**
     * Reset GPS auto test UI.
     */
    private void resetTestView() {

        ((TextView) YgpsActivity.this.findViewById(R.id.tv_mean_ttff))
                .setText("");
        ((TextView) YgpsActivity.this.findViewById(R.id.tv_last_ttff))
                .setText("");
        mTvTest68Ttff.setText("");
        mTvTest95Ttff.setText("");
        mTvTestMaxTtff.setText("");
        mTvTest68Cep.setText("");
        mTvTest95Cep.setText("");
        mTvTestLastCep.setText("");
        mTvTestMaxCep.setText("");
        mTvTestMeanCep.setText("");
    }

    /**
     * Calculate mean TTFF value.
     *
     * @param n
     *            Test times
     * @return Mean TTFF value
     */
    private float meanTTFF(int n) {
        return (mMeanTTFF * (n - 1) + mTtffValue) / n;
    }

    private double meanCep(int n, double cep) {
        return (mMeanCep * (n - 1) + cep) / n;
    }
    /**
     * Update GPS auto test UI.
     */
    private Handler mAutoTestHandler = new Handler() {
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case HANDLE_SET_CURRENT_TIMES:
                mTvCurTimes.setText(Integer.valueOf(msg.arg1).toString());
                break;
            case HANDLE_SET_COUNTDOWN:
                mTvCountDown.setText(Integer.valueOf(msg.arg1).toString());
                break;
            case HANDLE_START_BUTTON_UPDATE:
                mBtnGpsTestStart.setEnabled(MESSAGE_ARG_1 == msg.arg1);
                mBtnGpsTestStop.setEnabled(MESSAGE_ARG_0 == msg.arg1);
                if (msg.arg1 == MESSAGE_ARG_1) {
                    refreshATViewState(false);
                }
                break;
            case HANDLE_EXCEED_PERIOD:
                Toast.makeText(
                        YgpsActivity.this,
                        String.format(getString(R.string.toast_exceed_time,
                                Integer.valueOf(msg.arg1).toString())),
                        Toast.LENGTH_LONG).show();
                break;
            case HANDLE_SET_MEANTTFF:
                ((TextView) YgpsActivity.this.findViewById(R.id.tv_mean_ttff))
                        .setText(Float.valueOf(mMeanTTFF).toString());
                break;
            case HANDLE_SET_PARAM_RECONNECT:
                if (!mBtnGpsTestStart.isEnabled()) {
                    setStartButtonEnable(true);
                    removeDialog(DIALOG_WAITING_FOR_STOP);
                    mStopPressedHandling = false;
                    mStartPressedHandling = false;
                }

                break;
            default:
                break;
            }
        }
    };

    /**
     * GPS auto test thread.
     *
     * @author mtk54046
     *
     */
    private class AutoTestThread extends Thread {
        @Override
        public void run() {
            super.run();
            Looper.prepare();
            setStartButtonEnable(false);
            mLocationManager.removeUpdates(mLocListener);

            YgpsActivity.this.sleep(ONE_SECOND * 2);
            mIsTestRunning = true;
            Log.v(TAG, "mIsTestRunning: true");
            reconnectTest();
            calTestResult();
            runOnUiThread(new Runnable() {
                public void run() {
                    showTestResult();
                }
            });
            setStartButtonEnable(true);
            interrupt();
        }
    }

    private void waitAutoTestInterval() {
        if (mTestInterval != 0) {
            for (int i = mTestInterval; i >= 0 && mIsTestRunning; --i) {
                setCountDown(i);
                sleep(ONE_SECOND);

            }
            if (!mIsTestRunning) {
                setCountDown(0);
            }
        } else {
            sleep(ONE_SECOND / 2);
        }
    }

    private void wait3DFix() {
        boolean bExceed = false;
        Long beginTime = Calendar.getInstance().getTime().getTime()
                / ONE_SECOND;
        for (; mIsTestRunning; ) {
            Long nowTime = Calendar.getInstance().getTime()
                    .getTime()
                    / ONE_SECOND;
            if (mFirstFix) {
                break;
            } else if (nowTime - beginTime > EXCEED_SECOND) {
                bExceed = true;
                showExceedPeriod(EXCEED_SECOND);
                break;
            }
        }
        if (bExceed) {
            //break;
            Log.d(TAG, "wait3DFix , Exceed period");
        }  else {
            sleep(ONE_SECOND / 2);
        }
    }
    /**
     * GPS re-connect test.
     */
    private void reconnectTest() {
        Bundle extras = new Bundle();

        if (mTestType == GPS_TEST_HOT_START) {
            extras.putBoolean(GPS_EXTRA_RTI, true);
        } else if (mTestType == GPS_TEST_WARM_START) {
            extras.putBoolean(GPS_EXTRA_EPHEMERIS, true);
        } else if (mTestType == GPS_TEST_COLD_START) {
            extras.putBoolean(GPS_EXTRA_EPHEMERIS, true);
            extras.putBoolean(GPS_EXTRA_POSITION, true);
            extras.putBoolean(GPS_EXTRA_TIME, true);
            extras.putBoolean(GPS_EXTRA_IONO, true);
            extras.putBoolean(GPS_EXTRA_UTC, true);
            extras.putBoolean(GPS_EXTRA_HEALTH, true);
        } else if (mTestType == GPS_TEST_FULL_START) {
            extras.putBoolean(GPS_EXTRA_ALL, true);
        }
        Log.i(TAG, "test type:" + mTestType + " times:" + mTotalTimes);

        removeDialog(DIALOG_WAITING_FOR_START);
        for (int i = 1; i <= mTotalTimes && mIsTestRunning; ++i) {
            mCurrentTimes = i;
            Log.i(TAG, "reconnectTest function: "
                    + Integer.valueOf(mCurrentTimes).toString());
            setCurrentTimes(i);

            synchronized (YgpsActivity.this) {
                if (mIsForceStopGpsTest) {
                    return;
                }
                resetParamForAutoTest(extras);
            }
            waitAutoTestInterval();
            if (mIsNeed3DFix) {
                wait3DFix();
            } else {
                sleep(2 * ONE_SECOND);
            }
        }
        sleep(ONE_SECOND);
        synchronized (YgpsActivity.this) {
            if (mIsForceStopGpsTest) {
                return;
            }
            stopGPSAutoTest();
        }
    }

    /**
     * Set test parameters.
     */
    private void setTestParam() {
        Message msg = mAutoTestHandler
                .obtainMessage(YgpsActivity.HANDLE_SET_PARAM_RECONNECT);
        mAutoTestHandler.sendMessage(msg);
    }

    /**
     * Refresh auto test UI.
     *
     * @param bEnable
     *            Auto test start or not
     */
    private void setStartButtonEnable(boolean bEnable) {
        Message msg = mAutoTestHandler
                .obtainMessage(YgpsActivity.HANDLE_START_BUTTON_UPDATE);
        msg.arg1 = bEnable ? MESSAGE_ARG_1 : MESSAGE_ARG_0;
        mAutoTestHandler.sendMessage(msg);
    }

    /**
     * Update current test time.
     *
     * @param nTimes
     *            Current test time
     */
    private void setCurrentTimes(int nTimes) {
        Message msg = mAutoTestHandler
                .obtainMessage(YgpsActivity.HANDLE_SET_CURRENT_TIMES);
        msg.arg1 = nTimes;
        mAutoTestHandler.sendMessage(msg);
    }

    /**
     * Update test time count down.
     *
     * @param num
     *            Count down number
     */
    private void setCountDown(int num) {
        Message msg = mAutoTestHandler
                .obtainMessage(YgpsActivity.HANDLE_SET_COUNTDOWN);
        msg.arg1 = num;
        mAutoTestHandler.sendMessage(msg);
    }

    /**
     * Show exceed period.
     *
     * @param period
     *            Time period
     */
    private void showExceedPeriod(int period) {
        Message msg = mAutoTestHandler
                .obtainMessage(YgpsActivity.HANDLE_EXCEED_PERIOD);
        msg.arg1 = period;
        mAutoTestHandler.sendMessage(msg);
    }

    private long mLastTimestamp = -1;

    /**
     * NmeaListener implementation, to receive NMEA log.
     */
    private OnNmeaMessageListener mNmeaListener = new OnNmeaMessageListener() {
        public void onNmeaMessage(String nmea, long timestamp) {

            if (!mIsShowVersion) {
                if (timestamp - mLastTimestamp > ONE_SECOND) {
                    showVersion();
                    mLastTimestamp = timestamp;
                }
            }
            if (mStartNmeaRecord) {
                mTvNmeaLog.setText(nmea);
            }
            NmeaParser.getNMEAParser().parse(nmea);
        
        }
    };

    private void initTabPage() {
        TabHost tabHost = getTabHost();
        LayoutInflater.from(this).inflate(R.layout.layout_tabs,
                tabHost.getTabContentView(), true);
        // tab1
        tabHost.addTab(tabHost.newTabSpec(this.getString(R.string.satellites))
                .setIndicator(this.getString(R.string.satellites)).setContent(
                        R.id.layout_satellites));

        // tab2
        tabHost.addTab(tabHost.newTabSpec(this.getString(R.string.information))
                .setIndicator(this.getString(R.string.information)).setContent(
                        R.id.layout_info));

        // tab3
        tabHost.addTab(tabHost.newTabSpec(this.getString(R.string.nmea_log))
                .setIndicator(this.getString(R.string.nmea_log)).setContent(
                        R.id.layout_nmea));

        // tab4
        tabHost.addTab(tabHost.newTabSpec(this.getString(R.string.gps_test))
                .setIndicator(this.getString(R.string.gps_test)).setContent(
                        R.id.layout_auto_test));

        tabHost.setOnTabChangedListener(new OnTabChangeListener() {
            public void onTabChanged(String tabId) {
                Log.d(TAG, "Select: " + tabId);
            }
        });
    }

    private void initBgStatus() {

        boolean initBG = getIntent().getBooleanExtra(EXTRA_BG, false);
        if (!initBG) {
            final SharedPreferences preferences = this.getSharedPreferences(
                    SHARED_PREF_KEY_BG, android.content.Context.MODE_PRIVATE);
            if (preferences.getBoolean(SHARED_PREF_KEY_BG, false)) {
                mIsRunInBg = true;
            } else {
                mIsRunInBg = false;
            }
        } else {
            Log.i(TAG, "Init in BG ");
            SharedPreferences preferences = getSharedPreferences(
                    SHARED_PREF_KEY_BG, android.content.Context.MODE_PRIVATE);
            preferences.edit().putBoolean(SHARED_PREF_KEY_BG, true).commit();
            mIsRunInBg = true;
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        StrictMode.setThreadPolicy(new StrictMode.ThreadPolicy.Builder()
                .detectNetwork()
                .build());
        mLogHidden = GpsMnlSetting.isLogHidden();

        initTabPage();
        setLayout();

        mYgpsWakeLock = new YgpsWakeLock();
        mYgpsWakeLock.acquireScreenWakeLock(this);
        mYgpsWakeLock.acquireCpuWakeLock(this);

        mPermissionManager = new PermissionManager(this);
        if (mPermissionManager.requestLocationLaunchPermissions()) {

            try {
                mLocationManager = (LocationManager) getSystemService(Context.LOCATION_SERVICE);
                if (mLocationManager != null) {
                    mLocationManager.requestLocationUpdates(
                            LocationManager.GPS_PROVIDER, 0, 0, mLocListener);
                    mMnldConn.setTriggerNe(true);
                    mLocationManager.registerGnssStatusCallback(mGnssStatusCallback);
                    mLocationManager.addNmeaListener(mNmeaListener);
                    if (mLocationManager
                            .isProviderEnabled(LocationManager.GPS_PROVIDER)) {
                        mProvider = String.format(getString(
                                R.string.provider_status_enabled,
                                LocationManager.GPS_PROVIDER));
                    } else {
                        mProvider = String.format(getString(
                                R.string.provider_status_disabled,
                                LocationManager.GPS_PROVIDER));
                    }
                    mStatus = getString(R.string.gps_status_unknown);
                }
            } catch (SecurityException e) {
                Toast.makeText(this, "security exception", Toast.LENGTH_LONG)
                        .show();
                Log.e(TAG, "SecurityException: " + e.getMessage());
            } catch (IllegalArgumentException e) {
                Log.e(TAG, "IllegalArgumentException: " + e.getMessage());
            }

            //refreshATViewState(true);
        }
        mHandler.sendEmptyMessage(HANDLE_COUNTER);

        initBgStatus();
        mShowFirstFixLocate = true;

        mSocketClient = new ClientSocket(this);
        mHandler.sendEmptyMessage(HANDLE_CHECK_SATEREPORT);
        mSatelInfoManager = new SatelliteInfoManager();
        if (mNmeaParser == null) {
            mNmeaParser = NmeaParser.getNMEAParser();
        }
        mNmeaUpdateListener = new NmeaParser.NmeaUpdateViewListener() {
            @Override
            public void onViewupdateNotify() {
                Log.d(TAG, "NmeaParser onViewupdateNotify");
                mSateReportTimeOut = 0;
                setSatelliteStatus(new NmeaSatelliteAdapter(mNmeaParser.getSatelliteList()));
            }
        };
        mNmeaParser.addSVUpdateListener(mNmeaUpdateListener);
        if (mIsRunInBg) {
            mServiceIntent = new Intent(this, YgpsService.class);
            startService(mServiceIntent);
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        boolean supRetVal = super.onCreateOptionsMenu(menu);

        if (mShowLoc) {
            menu.add(0, 1, 0, R.string.menu_hideloc);
        } else {
            menu.add(0, 1, 0, R.string.menu_showloc);
        }

        final SharedPreferences preferences = this.getSharedPreferences(
                SHARED_PREF_KEY_BG, android.content.Context.MODE_PRIVATE);
        if (preferences.getBoolean(SHARED_PREF_KEY_BG, false)) {
            menu.add(0, 2, 0, R.string.menu_run_bg_disable);
        } else {
            menu.add(0, 2, 0, R.string.menu_run_bg_enable);
        }
        return supRetVal;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
        case 1:
            if (mShowLoc) {
                mShowLoc = false;
                if (mPrompt != null) {
                    mPrompt.cancel();
                }
                item.setTitle(R.string.menu_showloc);
            } else {
                mShowLoc = true;
                item.setTitle(R.string.menu_hideloc);
            }
            return true;

        case 2:
            final SharedPreferences preferences = this.getSharedPreferences(
                    SHARED_PREF_KEY_BG, android.content.Context.MODE_PRIVATE);
            if (preferences.getBoolean(SHARED_PREF_KEY_BG, false)) {
                item.setTitle(R.string.menu_run_bg_enable);
                preferences.edit().putBoolean(SHARED_PREF_KEY_BG, false)
                        .commit();
                Log.i(TAG_BG, "now should *not* be in bg.");
            } else {
                item.setTitle(R.string.menu_run_bg_disable);
                preferences.edit().putBoolean(SHARED_PREF_KEY_BG, true)
                        .commit();
                Log.v( TAG_BG, "now should be in bg.");
            }
            return true;
        default:
            break;
        }
        return false;
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        ProgressDialog dialog = null;
        switch (id) {
        case DIALOG_WAITING_FOR_STOP:
            dialog = new ProgressDialog(this);
            dialog.setTitle(R.string.dialog_title_stop);
            dialog.setMessage(getString(R.string.dialog_message_wait));
            dialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
            dialog.setCancelable(false);
            dialog.setProgress(0);
            break;
        case DIALOG_WAITING_FOR_START:
            dialog = new ProgressDialog(this);
            dialog.setTitle(R.string.dialog_title_start);
            dialog.setMessage(getString(R.string.dialog_message_wait));
            dialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
            dialog.setCancelable(false);
            dialog.setProgress(0);
            break;
        default: 
            break;
        }
        return dialog;
    }

    @Override
    protected void onPause() {
        super.onPause();
        Log.v(TAG, "Enter onPause function");
        if (!mIsRunInBg) {
            if(mLocationManager != null) {
                mLocationManager.removeUpdates(mLocListener);
                mMnldConn.setTriggerNe(false);
                mLocationManager.unregisterGnssStatusCallback(mGnssStatusCallback);
            }
            removeNmeaParser();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.v(TAG, "Enter onResume function");
        if (!mIsRunInBg) {
            mFirstFix = false;
            if(mLocationManager!=null) {
                if (mLocationManager
                        .isProviderEnabled(LocationManager.GPS_PROVIDER)) {
                    mProvider = String.format(getString(
                            R.string.provider_status_enabled,
                            LocationManager.GPS_PROVIDER));
                } else {
                    mProvider = String.format(getString(
                            R.string.provider_status_disabled,
                            LocationManager.GPS_PROVIDER));
                }
                mLocationManager.requestLocationUpdates(
                        LocationManager.GPS_PROVIDER, 0, 0, mLocListener);
                mMnldConn.setTriggerNe(true);
                mLocationManager.registerGnssStatusCallback(mGnssStatusCallback);
            }
            mStatus = getString(R.string.gps_status_unknown);
            mNmeaParser.addSVUpdateListener(mNmeaUpdateListener);
        }
        TextView tvProvider = (TextView) findViewById(R.id.tv_provider);
        tvProvider.setText(mProvider);
        mTvStatus.setText(mStatus);
    }

    /**
     * Show GPS version.
     */
    private void showVersion() {
        Log.v(TAG, "Enter show version");
        if (mIsExit) {
            return;
        }
        TextView tvChipVersion = (TextView) findViewById(R.id.tv_chip_version);
        tvChipVersion.setText(GpsMnlSetting
                .getChipVersion(getString(R.string.gps_status_unknown)));
        sendCommand("PMTK605");

        // update clock type/buffer
        TextView tvClockType = (TextView) findViewById(R.id.tv_clock_type);
        TextView tvClockBuffer = (TextView) findViewById(R.id.tv_clock_buffer);
        String ss = GpsMnlSetting.getClockProp("unknown");
        if (ss.length() == 2) {
            if ("ff".equals(ss)) {
                tvClockType.setText("error");
                tvClockBuffer.setText("error");
            } else {
                char clockBuffer = ss.charAt(0);
                if (clockBuffer >= '1' && clockBuffer <= '4') {
                    tvClockBuffer.setText(String.valueOf(clockBuffer));
                } else if (clockBuffer == '0') {
                    tvClockBuffer.setText("2");
                } else if (clockBuffer == '9') {
                    tvClockBuffer.setText("9");
                }
                char clockType = ss.charAt(1);
                if (clockType == '0') {
                    tvClockType.setText("TCXO");
                } else if (clockType == '1') {
                    tvClockType.setText("Co-clock");
                }
            }
        }
    }

    private void removeNmeaParser() {
        Log.i(TAG, "removeNmeaParser()");
        mNmeaParser.removeSVUpdateListener(mNmeaUpdateListener);
        mNmeaParser.clearSatelliteList();
    }

    @Override
    protected void onStart() {
        super.onStart();
        if (mIsRunInBg) {
            bindService(mServiceIntent, mServiceConnection, Context.BIND_AUTO_CREATE);
        }
    }

    @Override
    protected void onStop() {
        super.onStop();
        Log.i(TAG_BG, "onstop and in BG: " + mIsRunInBg);
        if (!mIsRunInBg) {
            forceStopGpsAutoTest();
            mYgpsWakeLock.releaseCpuWakeLock();
        } else {
            if (mYgpsService != null) {
                mYgpsService.requestForeground(YgpsActivity.class);
            }
            unbindService(mServiceConnection);
        }
        mYgpsWakeLock.releaseScreenWakeLock();
        if (mPrompt != null) {
            mPrompt.cancel();
        }
    }

    @Override
    protected void onRestart() {
        Log.i(TAG_BG, "onRestart and in BG:" + mIsRunInBg);

        mYgpsWakeLock.acquireScreenWakeLock(this);
        if (!mIsRunInBg) {
            mYgpsWakeLock.acquireCpuWakeLock(this);
        }

        super.onRestart();
    }

    @Override
    protected void onDestroy() {
        forceStopGpsAutoTest();
        if(mLocationManager != null) {
            mLocationManager.removeUpdates(mLocListener);
            mMnldConn.setTriggerNe(false);
            mLocationManager.unregisterGnssStatusCallback(mGnssStatusCallback);
            mLocationManager.removeNmeaListener(mNmeaListener);
        }
        removeNmeaParser();
        mHandler.removeMessages(HANDLE_UPDATE_RESULT);
        mHandler.removeMessages(HANDLE_COUNTER);
        mHandler.removeMessages(HANDLE_CHECK_SATEREPORT);
        mIsExit = true;
        if (mStartNmeaRecord) {
            mStartNmeaRecord = false;
        }

        if (mServiceIntent != null) {
            stopService(mServiceIntent);
        }

        mSocketClient.endClient();

        if (mIsRunInBg) {
            mYgpsWakeLock.releaseCpuWakeLock();
        }

        super.onDestroy();
    }

    private void showLocToast(Location location) {
        long time = location.getTime();
        if ((time - mLocUpdateTime) > LOC_TOAST_PERIOD) {
            mLocUpdateTime = time;
            String str = new Date(time).toString() + "\n";
            String tmp = String.valueOf(location.getLatitude());
            if (tmp.length() > LOCATION_MAX_LENGTH) {
                tmp = tmp.substring(0, LOCATION_MAX_LENGTH);
            }
            str += tmp + ",";
            tmp = String.valueOf(location.getLongitude());
            if (tmp.length() > LOCATION_MAX_LENGTH) {
                tmp = tmp.substring(0, LOCATION_MAX_LENGTH);
            }
            str += tmp;

            mPrompt = Toast.makeText(YgpsActivity.this, str,
                    Toast.LENGTH_SHORT);
            mPrompt.show();
        }
    }

    private void showFirstFixInfo(Location location) {

        mShowFirstFixLocate = false;
        double latitude = location.getLatitude();
        double longitude = location.getLongitude();
        TextView firstLon = (TextView) findViewById(R.id.first_longtitude_text);
        firstLon.setText(String.valueOf(longitude));
        TextView firstLat = (TextView) findViewById(R.id.first_latitude_text);
        firstLat.setText(String.valueOf(latitude));
        if (mIsTestRunning && mNeedCalCep) {
            double cep = CepCal.calCEP(mTestLat, mTestLon, latitude, longitude);
            mTestCepList.add(cep);
            Log.d(TAG, "mTestCepList add: " + cep);
            mTvTestLastCep.setText(String.valueOf(cep)
                    + getString(R.string.cep_unit_m));

            mMeanCep = meanCep(mCurrentTimes, cep);
            Log.d(TAG, "mMeanCep: " + mMeanCep);
            mTvTestMeanCep.setText(String.valueOf(mMeanCep)
                    + getString(R.string.cep_unit_m));
        }
    }

    private LocationListener mLocListener = new LocationListener() {

        // @Override
        public void onLocationChanged(Location location) {
            Log.v(TAG, "Enter onLocationChanged function");
            if (!mFirstFix) {
                Log.w(TAG, "mFirstFix is false, onLocationChanged");
            }
            if (mShowLoc) {
                showLocToast(location);
            }
            Date d = new Date(location.getTime());
            SimpleDateFormat dateFormat = new SimpleDateFormat("z yyyy/MM/dd");
            String date = dateFormat.format(d);

            SimpleDateFormat timeFormat = new SimpleDateFormat("HH:mm:ss");
            String time = timeFormat.format(d);

            TextView tvTime = (TextView) findViewById(R.id.tv_time);
            if (tvTime != null) {
                tvTime.setText(time);
            }

            TextView tvDate = (TextView) findViewById(R.id.tv_date);
            tvDate.setText(date);

            if (mShowFirstFixLocate) {
                showFirstFixInfo(location);
            }
            TextView tvLat = (TextView) findViewById(R.id.tv_latitude);
            tvLat.setText(String.valueOf(location.getLatitude()));
            TextView tvLon = (TextView) findViewById(R.id.tv_longitude);
            tvLon.setText(String.valueOf(location.getLongitude()));
            TextView tvAlt = (TextView) findViewById(R.id.tv_altitude);
            tvAlt.setText(String.valueOf(location.getAltitude()));
            TextView tvAcc = (TextView) findViewById(R.id.tv_accuracy);
            tvAcc.setText(String.valueOf(location.getAccuracy()));
            TextView tvBear = (TextView) findViewById(R.id.tv_bearing);
            tvBear.setText(String.valueOf(location.getBearing()));
            TextView tvSpeed = (TextView) findViewById(R.id.tv_speed);
            tvSpeed.setText(String.valueOf(location.getSpeed()));
            if (mLastLocation != null) {
                TextView tvDist = (TextView) findViewById(R.id.tv_distance);
                tvDist.setText(String.valueOf(location
                        .distanceTo(mLastLocation)));
            }


            TextView tvProvider = (TextView) findViewById(R.id.tv_provider);
            tvProvider.setText(mProvider);
            mTvStatus.setText(mStatus);
            d = null;
            mLastLocation = location;

        }

        // @Override
        public void onProviderDisabled(String provider) {
            Log.i(TAG, "Enter onProviderDisabled function");
            mProvider = String.format(getString(R.string.provider_status_disabled,
                    LocationManager.GPS_PROVIDER));
            TextView tvProvider = (TextView) findViewById(R.id.tv_provider);
            tvProvider.setText(mProvider);
        }

        // @Override
        public void onProviderEnabled(String provider) {
            Log.i(TAG, "Enter onProviderEnabled function");
            mProvider = String.format(getString(R.string.provider_status_enabled,
                    LocationManager.GPS_PROVIDER));
            TextView tvProvider = (TextView) findViewById(R.id.tv_provider);
            tvProvider.setText(mProvider);
            mTtffValue = 0;
        }

        // @Override
        public void onStatusChanged(String provider, int status, Bundle extras) {
            Log.v(TAG, "Enter onStatusChanged function");
        }
    };


    private GnssStatus.Callback mGnssStatusCallback = new GnssStatus.Callback() {
        @Override
        public void onStarted() {
            mStatus = getString(R.string.gps_status_started);
            mTvStatus.setText(mStatus);

        }

        
        @Override
        public void onStopped() {
            // TODO Auto-generated method stub
            mStatus = getString(R.string.gps_status_stopped);
            mTvStatus.setText(mStatus);
        }


        @Override
        public void onSatelliteStatusChanged(GnssStatus status) {
            if (mNmeaFixed) {
                if (!mFirstFix) {
                    Log.i(TAG, "Already fixed");
                    mHandler.removeMessages(HANDLE_COUNTER);
                    mTtffValue = 0;
                    mFirstFix = true;
                    TextView tvTtff = (TextView) findViewById(R.id.tv_ttff);
                    tvTtff.setText(mTtffValue + getString(R.string.time_unit_ms) + "(Already fixed)");
                }
            } else {
                mFirstFix = false;
                if (!mHandler.hasMessages(HANDLE_COUNTER)) {
                    mHandler.sendEmptyMessage(HANDLE_COUNTER);
                }
            }
            if (!mIsShowVersion) {
                showVersion();
            }
        }

        @Override
        public void onFirstFix(int ttffMillis) {
            if (isGpsRestarting()) {
                Log.v(TAG, "Restarting GPS, ignore the FIRST_FIX event");
                return;
            }
            handleTtffReceived(ttffMillis);
            mStatus = getString(R.string.gps_status_first_fix);
            mTvStatus.setText(mStatus);
        }

        private void handleTtffReceived(int ttff) {
            Log.i(TAG, "Enter onFirstFix function: ttff = " + ttff);
            int currentTimes = mCurrentTimes;
            mHandler.removeMessages(HANDLE_COUNTER);
            mTtffValue = ttff;
            if (ttff != mTtffValue) {
                mTtffValue = ttff;
            }
            mFirstFix = true;
            Toast.makeText(
                    YgpsActivity.this,
                    String.format(getString(R.string.toast_first_fix), ttff,
                            getString(R.string.time_unit_ms)),
                    Toast.LENGTH_LONG).show();
            TextView tvTtff = (TextView) findViewById(R.id.tv_ttff);
            tvTtff.setText(mTtffValue + getString(R.string.time_unit_ms));
            Log.v(TAG, "show tvTtff = " + mTtffValue);
            if (mIsTestRunning) {
                mTestTtffList.add(mTtffValue);
                Log.d(TAG, "mTestTtffList add: " + mTtffValue);
                TextView tvLastTtff = (TextView) findViewById(R.id.tv_last_ttff);
                tvLastTtff.setText(mTtffValue
                        + getString(R.string.time_unit_ms));

                mMeanTTFF = meanTTFF(currentTimes);
                Log.d(TAG, "mMeanTTFF: " + mMeanTTFF);
                ((TextView) findViewById(R.id.tv_mean_ttff)).setText(Float
                        .valueOf(mMeanTTFF).toString()
                        + getString(R.string.time_unit_ms));
            }
        }
    };

    /**
     * only for hot/warm/cold/full/restart button.
     * order is stop->restart->start
     * the button should be enabled after the 3 steps is finished
     */
    private void resetParamForRestart(Bundle extras) {
        Log.i(TAG, "resetParamForRestart");
        /* Below code come from HANDLE_REMOVE_UPDATE
         * Avoid HANDLE_REMOVE_UPDATE->onLocationChanged(previous)
         */
        mRestarting = true;
        mLocationManager.removeUpdates(mLocListener);
        Message msg = mHandler
                .obtainMessage(YgpsActivity.HANDLE_RESTART_UPDATE);
        msg.setData(extras);
        mHandler.sendMessage(msg);
        mHandler.sendEmptyMessageDelayed(HANDLE_ENABLE_BUTTON, HANDLE_MSG_DELAY_300 * 3);
    }

     /**
     * only for Auto GPS test start button.
     * order is stop->restart->start
     * the button should be enabled after the 3 steps is finished
     */
    private void resetParamForAutoTest(Bundle extras) {
        Log.i(TAG, "resetParamForAutoTest");
        mLocationManager.removeUpdates(mLocListener);
        Message msg = mHandler
                .obtainMessage(YgpsActivity.HANDLE_RESTART_UPDATE);
        msg.setData(extras);
        mHandler.sendMessage(msg);
    }


    /**
     * Get GPS test status.
     *
     * @return Whether gps auto test is running
     */
    private boolean gpsTestRunning() {
        if (mIsTestRunning) {
            Toast.makeText(this, R.string.gps_test_running_warn,
                    Toast.LENGTH_LONG).show();
            return true;
        }
        return false;
    }

    private final OnClickListener mBtnClickListener = new OnClickListener() {

        @Override
        public void onClick(View v) {

            Bundle extras = new Bundle();
            if (v == (View) mBtnGpsTestStart) {
                showDialog(DIALOG_WAITING_FOR_START);
                mTvCurTimes.setText("1");
                mTvCountDown.setText("");
                mBtnGpsTestStart.refreshDrawableState();
                mBtnGpsTestStart.setEnabled(false);
                Log.v(TAG, "GPSTest Start button is pressed");
                mHandler.sendEmptyMessageDelayed(HANDLE_CLEAR, HANDLE_MSG_DELAY);
                startGPSAutoTest();
            } else if (v == (View) mBtnGpsTestStop) {
                mBtnGpsTestStop.setEnabled(false);
                mBtnGpsTestStop.refreshDrawableState();
                Log.v(TAG, "GPSTest Stop button is pressed");
                if (!mStopPressedHandling) {
                    mStopPressedHandling = true;
                    showDialog(DIALOG_WAITING_FOR_STOP);
                    mIsTestRunning = false;
                } else {
                    Log.v(TAG, "stop has been clicked.");
                }
            } else if (v == (View) mBtnHotStart) {
                if (gpsTestRunning()) {
                    return;
                }
                enableBtns(false);
                Log.i(TAG, "Hot Start button is pressed");
                extras.putBoolean(GPS_EXTRA_RTI, true);
                resetParamForRestart(extras);
            } else if (v == (View) mBtnWarmStart) {
                if (gpsTestRunning()) {
                    return;
                }
                enableBtns(false);
                Log.i(TAG, "Warm Start button is pressed");
                extras.putBoolean(GPS_EXTRA_EPHEMERIS, true);
                resetParamForRestart(extras);
            } else if (v == (View) mBtnColdStart) {
                if (gpsTestRunning()) {
                    return;
                }
                enableBtns(false);
                Log.i(TAG, "Cold Start button is pressed");
                extras.putBoolean(GPS_EXTRA_EPHEMERIS, true);
                extras.putBoolean(GPS_EXTRA_POSITION, true);
                extras.putBoolean(GPS_EXTRA_TIME, true);
                extras.putBoolean(GPS_EXTRA_IONO, true);
                extras.putBoolean(GPS_EXTRA_UTC, true);
                extras.putBoolean(GPS_EXTRA_HEALTH, true);
                resetParamForRestart(extras);
            } else if (v == (View) mBtnFullStart) {
                if (gpsTestRunning()) {
                    return;
                }
                enableBtns(false);
                Log.i(TAG, "Full Start button is pressed");
                extras.putBoolean(GPS_EXTRA_ALL, true);
                resetParamForRestart(extras);
            } else if (v == (View) mBtnReStart) {
                if (gpsTestRunning()) {
                    return;
                }
                enableBtns(false);
                Log.i(TAG, "Restart button is pressed");
                extras.putBoolean(GPS_EXTRA_EPHEMERIS, true);
                extras.putBoolean(GPS_EXTRA_A1LMANAC, true);
                extras.putBoolean(GPS_EXTRA_POSITION, true);
                extras.putBoolean(GPS_EXTRA_TIME, true);
                extras.putBoolean(GPS_EXTRA_IONO, true);
                extras.putBoolean(GPS_EXTRA_UTC, true);
                resetParamForRestart(extras);
            } else if (v == (View) mBtnNmeaStart) {
                Log.i(TAG, "NMEA Start button is pressed");
                mStartNmeaRecord = true;
                mBtnNmeaStart.setEnabled(false);
                mBtnNMEAStop.setEnabled(true);
            } else if (v == (View) mBtnNMEAStop) {
                Log.i(TAG, "NMEA Stop button is pressed");
                mStartNmeaRecord = false;

                mBtnNMEAStop.setEnabled(false);
                mBtnNmeaStart.setEnabled(true);
                mTvNmeaLog.setText(R.string.empty);

            } else if (v == (View) mBtnGpsJamming) {
                Log.i(TAG, "mBtnGPSJamming Button is pressed");
                onGpsJammingScanClicked();
            } else {
                return;
            }
        }
    };

    /**
     * Send command to MNL server.
     *
     * @param command
     *            PMTK command to be send
     */
    private void sendCommand(String command) {
        Log.i(TAG, "sendCommand:" + command);
        if (null == command || command.trim().length() == 0) {
            Toast.makeText(this, R.string.command_error, Toast.LENGTH_LONG)
                    .show();
            return;
        }
        int index1 = command.indexOf(COMMAND_START);
        int index2 = command.indexOf(COMMAND_END);
        String com = command;
        if (index1 != -1 && index2 != -1) {
            if (index2 < index1) {
                Toast.makeText(this, R.string.command_error, Toast.LENGTH_LONG)
                        .show();
                return;
            }
            com = com.substring(index1 + 1, index2);
        } else if (index1 != -1) {
            com = com.substring(index1 + 1);
        } else if (index2 != -1) {
            com = com.substring(0, index2);
        }
        mSocketClient.sendCommand(com.trim());
    }

    /**
     * Invoked when get GPS server respond.
     *
     * @param res
     *            Response message
     */
    public void onResponse(String res) {
        Log.i(TAG, "Enter getResponse: " + res);
        if (null == res || res.isEmpty()) {
            return;
        }
        Message m = mHandler.obtainMessage(HANDLE_UPDATE_RESULT);
        if (res.startsWith("$PMTK705")) {
            m.arg1 = CMD_TYPE_GETVERSION;
        } else if (res.contains("PMTK001")) {
            m.arg1 = CMD_TYPE_JAMMINGSCAN;
        } else {
            m.arg1 = CMD_TYPE_OTHERS;
        }
        m.obj = res;
        mHandler.sendMessage(m);
    }


    /**
     * Invoked when GPS Jamming Scan test button clicked.
     */
    private void onGpsJammingScanClicked() {
        if (0 == mEtGpsJammingTimes.getText().length()) {
            Toast.makeText(YgpsActivity.this,
                    "Please input Jamming scan times", Toast.LENGTH_LONG)
                    .show();
            return;
        } else {
            Integer times = Integer.valueOf(mEtGpsJammingTimes.getText()
                    .toString());
            if (times <= INPUT_VALUE_MIN || times > INPUT_VALUE_MAX) {
                Toast.makeText(YgpsActivity.this, "Jamming scan times error",
                        Toast.LENGTH_LONG).show();
                return;
            }
            sendCommand("PMTK837,1," + times);
        }
    }

    /**
     * Refresh button status.
     *
     * @param bEnable
     *            Set button status
     */
    private void enableBtns(boolean bEnable) {
        mBtnHotStart.setClickable(bEnable);
        mBtnWarmStart.setClickable(bEnable);
        mBtnColdStart.setClickable(bEnable);
        mBtnFullStart.setClickable(bEnable);
        mBtnReStart.setClickable(bEnable);
    }

    private void handleCmdResult(String response, int type) {
        switch (type) {
        case CMD_TYPE_JAMMINGSCAN:
            if (response.contains("PMTK001,837")) {
                Toast.makeText(YgpsActivity.this,
                        R.string.toast_jamming_succeed,
                        Toast.LENGTH_LONG).show();
            }
            break;
       case CMD_TYPE_GETVERSION:
            if (response.startsWith("$PMTK705")) {
                String[] strA = response.split(",");
                if (strA.length >= RESPONSE_ARRAY_LENGTH) {
                    TextView tMnlVersion = (TextView) findViewById(R.id.tv_mnl_version);
                    if (null != tMnlVersion) {
                        if (!tMnlVersion.getText().toString()
                                .startsWith("MNL")) {
                            tMnlVersion
                                    .setText(strA[RESPONSE_ARRAY_LENGTH - 1]);
                            mIsShowVersion = true;
                        }
                    }
                }
            }
            break;
        case CMD_TYPE_OTHERS:
            break;
        default:
            break;
        }
    }

    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case HANDLE_COUNTER:
                if (!mFirstFix) {
                    mTtffValue += COUNT_PRECISION;
                    TextView tvTtff = (TextView) findViewById(R.id.tv_ttff);
                    tvTtff.setText(mTtffValue % 1000 == 0 ? "Counting" : "");
                    this.sendEmptyMessageDelayed(HANDLE_COUNTER,
                            COUNT_PRECISION);
                }
                break;
            case HANDLE_UPDATE_RESULT:
                handleCmdResult(msg.obj.toString(), msg.arg1);
                break;
            case HANDLE_CLEAR:
                Log.i(TAG, "handleClear-msg");
                setSatelliteStatus(null);
                clearLayout();
                break;
            case HANDLE_CHECK_SATEREPORT:
                mSateReportTimeOut++;
                if (SATE_RATE_TIMEOUT < mSateReportTimeOut) {
                    mSateReportTimeOut = 0;
                    sendEmptyMessage(HANDLE_CLEAR);
                }
                sendEmptyMessageDelayed(HANDLE_CHECK_SATEREPORT, ONE_SECOND);
                break;
            case HANDLE_ENABLE_BUTTON:
                Log.i(TAG, "handleEnableButton-msg");
                enableBtns(true); // avoid continue press button
                break;
            case HANDLE_RESTART_UPDATE:
                //remove update
                Log.i(TAG, "restart update-msg");
                restartUpdate(this, msg.getData());
                break;
            default:
                break;
            }
            super.handleMessage(msg);
        }
    };

    private void restartUpdate(Handler handler, Bundle data) {
        removeNmeaParser();
        mFirstFix = false;
        mTtffValue = 0;
        mNmeaFixed = false;
        mShowFirstFixLocate = true;
        setSatelliteStatus(null);
        clearLayout();
        if (!handler.hasMessages(HANDLE_COUNTER)) {
            handler.sendEmptyMessage(HANDLE_COUNTER);
        }
        sleep(300);
        //delete data
        Log.i(TAG, "delete_aiding_data-msg");
        mLocationManager.sendExtraCommand(LocationManager.GPS_PROVIDER,
            "delete_aiding_data", data);
        sleep(300);
        //request update
        mLocationManager.requestLocationUpdates(
                LocationManager.GPS_PROVIDER, 0, 0, mLocListener);
            mNmeaParser.addSVUpdateListener(mNmeaUpdateListener);
            mRestarting = false;
            Log.i(TAG, "requestLocationUpdates-msg");
    }

    private void sleep(long millis) {
        try {
            Thread.sleep(millis);
        } catch (InterruptedException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }
    /**
     * Class for controlling wakelock.
     *
     */
    private class YgpsWakeLock {
        private PowerManager.WakeLock mScreenWakeLock = null;
        private PowerManager.WakeLock mCpuWakeLock = null;

        /**
         * Acquire CPU wake lock.
         *
         * @param context
         *            Getting lock context
         */
        void acquireCpuWakeLock(Context context) {
            Log.v(TAG, "Acquiring cpu wake lock");
            if (mCpuWakeLock != null) {
                return;
            }

            PowerManager pm = (PowerManager) context
                    .getSystemService(Context.POWER_SERVICE);

            mCpuWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK
                    | PowerManager.ACQUIRE_CAUSES_WAKEUP, TAG);
            mCpuWakeLock.acquire();
        }

        /**
         * Acquire screen wake lock.
         *
         * @param context
         *            Getting lock context
         */
        void acquireScreenWakeLock(Context context) {
            Log.v(TAG, "Acquiring screen wake lock");
            if (mScreenWakeLock != null) {
                return;
            }

            PowerManager pm = (PowerManager) context
                    .getSystemService(Context.POWER_SERVICE);

            mScreenWakeLock = pm.newWakeLock(PowerManager.FULL_WAKE_LOCK
                    | PowerManager.ACQUIRE_CAUSES_WAKEUP, TAG);
            mScreenWakeLock.acquire();
        }

        /**
         * Release wake locks.
         */
        void releaseScreenWakeLock() {
            Log.v(TAG, "Releasing wake lock");

            if (mScreenWakeLock != null) {
                mScreenWakeLock.release();
                mScreenWakeLock = null;
            }
        }

        void releaseCpuWakeLock() {
            Log.v(TAG, "Releasing cpu wake lock");
            if (mCpuWakeLock != null) {
                mCpuWakeLock.release();
                mCpuWakeLock = null;
            }
        }
    }

    public void onRequestPermissionsResult(int requestCode, String permissions[], int[] grantResults) {
        Log.i(TAG, "onRequestPermissionsResult(), requestCode = " + requestCode);
        if (grantResults == null || grantResults.length <= 0) {
            return;
        }
        if (mPermissionManager.getLocationLaunchPermissionRequestCode() == requestCode) {
            if (mPermissionManager.isLocationLaunchPermissionsResultReady(permissions, grantResults)) {
                Log.d(TAG, "mPermissionManager: isLocationLaunchPermissionsResultReady");
                try {
                    mLocationManager = (LocationManager) getSystemService(Context.LOCATION_SERVICE);
                    if (mLocationManager != null) {
                        mLocationManager.requestLocationUpdates(
                                LocationManager.GPS_PROVIDER, 0, 0, mLocListener);
                        mMnldConn.setTriggerNe(true);
                        mLocationManager.registerGnssStatusCallback(mGnssStatusCallback);
                        mLocationManager.addNmeaListener(mNmeaListener);
                        if (mLocationManager
                                .isProviderEnabled(LocationManager.GPS_PROVIDER)) {
                            mProvider = String.format(getString(
                                    R.string.provider_status_enabled,
                                    LocationManager.GPS_PROVIDER));
                        } else {
                            mProvider = String.format(getString(
                                    R.string.provider_status_disabled,
                                    LocationManager.GPS_PROVIDER));
                        }
                        mStatus = getString(R.string.gps_status_unknown);
                    }
                } catch (SecurityException e) {
                    Toast.makeText(this, "security exception", Toast.LENGTH_LONG)
                            .show();
                    Log.e(TAG, "SecurityException: " + e.getMessage());
                } catch (IllegalArgumentException e) {
                    Log.e(TAG, "IllegalArgumentException: " + e.getMessage());
                }

            } else {
                // more than one critical permission was denied activity finish, exit and destroy
                Toast.makeText(this, R.string.denied_required_permission,
                        Toast.LENGTH_LONG).show();
                finish();
            }
        } else {
            super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        }
    }

}
