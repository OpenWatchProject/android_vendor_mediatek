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

package com.mediatek.engineermode.cwtest;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.net.LocalSocketAddress.Namespace;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.PowerManager;
import android.os.StrictMode;
import android.os.SystemProperties;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.mnldinterface.Debug2MnldInterface;
import com.mediatek.engineermode.mnldinterface.Debug2MnldInterface.Debug2MnldInterfaceSender;
import com.mediatek.socket.base.UdpClient;

/**
 * GPS CW test.
 *
 */
public class CWTest extends Activity {

    private static final String TAG = "CWTest";
    private static final String COMMAND_END = "*";
    private static final String COMMAND_START = "$";

    private static final int HANDLE_START_TEST = 1001;
    private static final int HANDLE_STOP_TEST = 1002;
    private static final int HANDLE_UPDATE_RESULT = 1003;

    private static final int RESPONSE_ARRAY_LENGTH = 4;

    private static final int DIALOG_REENTER = 1;

    private static final String MNL_PROP_NAME = "persist.vendor.radio.mnl.prop";
    private static final String DEFAULT_MNL_PROP = "00010001";
    static final String START_CMD = "$PMTK817,1";
    private static final String STOP_CMD = "$PMTK817,0";

    private ClientSocket mSocketClient = null;

    private LocationManager mLocationManager = null;
    private CwTestWakeLock mCwTestWakeLock = null;
    private Button mBtnStart = null;
    private Button mBtnStop = null;

    private TextView mCnrTv = null;
    private TextView mDriftTv = null;

    private EditText mEtTimes = null;
    private EditText mEtInterval = null;

    private TextView mCurrentTimesTv = null;

    private TextView mMaxCnrTv = null;
    private TextView mMinCnrTv = null;

    private int mMaxCnr = 0;
    private int mMinCnr = 0;

    private int mTotalTimes = 0;
    private int mInterval = 0;
    private int mCurrentTimes = 0;

    private boolean mRequestLocationFix = false;
    private boolean mDebugFile = false;
    private static final String CHANNEL_OUT = "mtk_debugService2mnld"; // send cmd to mnld
    private UdpClient mUdpClient;
    private Debug2MnldInterfaceSender mToMnldSender;


    private final LocationListener mLocListener = new LocationListener() {

        // @Override
        public void onLocationChanged(Location location) {
        }

        // @Override
        public void onProviderDisabled(String provider) {
        }

        // @Override
        public void onProviderEnabled(String provider) {
        }

        // @Override
        public void onStatusChanged(String provider, int status, Bundle extras) {
        }
    };

    private final OnClickListener mBtnClickListener = new OnClickListener() {

        @Override
        public void onClick(View v) {
            Bundle extras = new Bundle();
            if (v == (View) mBtnStart) {
                startCWTest();
            } else if (v == (View) mBtnStop) {
                stopCWTest();
            }
        }
    };
    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case HANDLE_START_TEST:
                mCurrentTimes++;
                Elog.i(TAG, "send start command times = " + Integer.toString(mCurrentTimes));
                sendCommand(START_CMD);
                if (mCurrentTimes >= mTotalTimes) {
                    Elog.i(TAG, "Test done");
                    break;
                }
                sendEmptyMessageDelayed(HANDLE_START_TEST, mInterval * 1000);
                break;

            case HANDLE_STOP_TEST:
                sendCommand(STOP_CMD);
                mCurrentTimesTv.setText("0");
                mCnrTv.setText("0");
                mDriftTv.setText("0");
                break;
            case HANDLE_UPDATE_RESULT:  /* $PMTK817,2,0037,-5.614*23 */
                //If start button is enabled, that means test is stoppped by user.
                if (mBtnStart.isEnabled()) {
                    return;
                }
                String res = msg.obj.toString();
                String[] strA = res.split(",");
                mCurrentTimesTv.setText(Integer.toString(mCurrentTimes));
                if (strA.length >= RESPONSE_ARRAY_LENGTH) {
                    Elog.i(TAG, "receive command times = " + Integer.toString(mCurrentTimes));
                    String strCNR = strA[RESPONSE_ARRAY_LENGTH - 2];
                    int cnr = Integer.parseInt(strCNR);
                    if (mCurrentTimes == 1) {
                        mMaxCnr = cnr;
                        mMinCnr = cnr;
                    } else {
                        if (mMaxCnr < cnr) {
                            mMaxCnr = cnr;
                        }
                        if (mMinCnr > cnr) {
                            mMinCnr = cnr;
                        }
                    }
                    mMaxCnrTv.setText(Integer.toString(mMaxCnr));
                    mMinCnrTv.setText(Integer.toString(mMinCnr));

                    mCnrTv.setText(Integer.toString(cnr));
                    char[] strB = strA[RESPONSE_ARRAY_LENGTH - 1].toCharArray();
                    int index = 0;
                    while (index < strB.length) {
                        if (strB[index] == '*') {
                            break;
                        }
                        index++;
                    }

                    String strDrift = new String(strB, 0, index);
                    mDriftTv.setText(strDrift);

                }
                if (mCurrentTimes >= mTotalTimes) {
                    Elog.i(TAG, "Test done, Reset button");
                    mBtnStart.setEnabled(true);
                    mBtnStop.setEnabled(false);
                    break;
                }
                break;
            default:
                break;
            }
            super.handleMessage(msg);
        }
    };
    /**
     * Convert Integer array to string with specified length.
     *
     * @param array
     *            Integer array
     * @param count
     *            Specified length
     * @return Integer array numbers string
     */
    private String toString(int[] array, int count) {
        StringBuilder strBuilder = new StringBuilder();
        strBuilder.append("(");
        for (int idx = 0; idx < count; idx++) {
            strBuilder.append(Integer.toString(array[idx]));
            strBuilder.append(",");
        }
        strBuilder.append(")");
        return strBuilder.toString();
    }

    private  String getDebug2FileProp(String defaultValue) {
        String result = defaultValue;
        String prop = SystemProperties.get(MNL_PROP_NAME);

        int index = 2;
        Elog.i(TAG, "getMnlProp: " + prop);
        if (null == prop || prop.isEmpty()) {
            result = defaultValue;
        } else {
            char c = prop.charAt(index);
            result = String.valueOf(c);
        }
        return result;
    }

    private void setDebug2FileMnlProp(String value) {
        String prop = SystemProperties.get(MNL_PROP_NAME);

        int index = 2;
        Elog.i(TAG, "getMnlProp: " + prop);
        if (null == prop || prop.isEmpty()) {
            prop = DEFAULT_MNL_PROP;
        }
        if (prop.length() > index) {
            char[] charArray = prop.toCharArray();
            charArray[index] = value.charAt(0);
            String newProp = String.valueOf(charArray);
            mToMnldSender.debugMnldRadioMsg(mUdpClient, newProp);
            Elog.i(TAG, "setMnlProp newProp: " + newProp);
        }

    }
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.cw_test);
        mToMnldSender = new Debug2MnldInterfaceSender();
        mUdpClient = new UdpClient(CHANNEL_OUT, Namespace.ABSTRACT,
                Debug2MnldInterface.MAX_BUFF_SIZE);

        mBtnStart = (Button) findViewById(R.id.cw_test_start_btn);
        mBtnStop = (Button) findViewById(R.id.cw_test_stop_btn);
        mCnrTv = (TextView) findViewById(R.id.cw_test_cnr_content);
        mDriftTv = (TextView) findViewById(R.id.cw_test_clock_drift_content);
        mCnrTv.setText("0");
        mDriftTv.setText("0");

        mEtTimes = (EditText) findViewById(R.id.cw_test_times_content);
        mEtInterval = (EditText) findViewById(R.id.cw_test_interval_content);

        mCurrentTimesTv = (TextView) findViewById(R.id.cw_test_current_times_content);
        mCurrentTimesTv.setText("0");

        mMaxCnrTv = (TextView) findViewById(R.id.cw_test_max_cnr_content);
        mMinCnrTv = (TextView) findViewById(R.id.cw_test_min_cnr_content);

        mMaxCnrTv.setText("0");
        mMinCnrTv.setText("0");

        mBtnStart.setOnClickListener(mBtnClickListener);
        mBtnStop.setOnClickListener(mBtnClickListener);
        mBtnStop.setEnabled(false);

        // open debug file
        String ss = getDebug2FileProp("0");

        if (ss.equals("0")) {
            mDebugFile = false;
            setDebug2FileMnlProp("1");
        } else {
            mDebugFile = true;
        }

        StrictMode.setThreadPolicy(new StrictMode.ThreadPolicy.Builder()
                .detectNetwork() // or .detectAll() for all detectable problems
                .build());
        mCwTestWakeLock = new CwTestWakeLock();
        mCwTestWakeLock.acquireScreenWakeLock(this);

        mLocationManager = (LocationManager) getSystemService(Context.LOCATION_SERVICE);
        if (EmUtils.checkLocationProxyAppPermission(this, false)) {
            if (!ctrlLocationFix(true)) {
                Toast.makeText(this, getString(R.string.start_fix_error_message),
                               Toast.LENGTH_LONG).show();
                finish();
            }
        }

        mSocketClient = new ClientSocket(this);
    }


    private boolean ctrlLocationFix(boolean on) {
        if (on && !mRequestLocationFix) {
            try {
                if (mLocationManager != null) {
                    if (!mLocationManager
                            .isProviderEnabled(LocationManager.GPS_PROVIDER)) {
                        Elog.i(TAG, "provider disabled");
                        return false;
                    } else {
                        mLocationManager.requestLocationUpdates(
                                LocationManager.GPS_PROVIDER, 0, 0, mLocListener);
                    }
                }
            } catch (SecurityException e) {
                Elog.w(TAG, "Exception: " + e.getMessage());
                return false;
            } catch (IllegalArgumentException e) {
                Elog.w(TAG, "Exception: " + e.getMessage());
                return false;
            }
            mRequestLocationFix = true;
        } else if (!on && mRequestLocationFix) {
            mLocationManager.removeUpdates(mLocListener);
            mRequestLocationFix = false;
        }
        return true;
    }

    @Override
    protected void onStop() {
        super.onStop();
        mCwTestWakeLock.release();

    }

    @Override
    protected void onRestart() {
        if (mCwTestWakeLock != null) {
            mCwTestWakeLock.acquireScreenWakeLock(this);
        }
        super.onRestart();
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (EmUtils.checkLocationProxyAppPermission(this, true) && !mRequestLocationFix) {
            showDialog(DIALOG_REENTER);
        }
    }

    @Override
    protected void onDestroy() {
        ctrlLocationFix(false);

       if (!mDebugFile) {
            setDebug2FileMnlProp("0");
        }
        mHandler.removeMessages(HANDLE_START_TEST);
        mSocketClient.endClient();
        super.onDestroy();
    }

    private void startCWTest() {

        try {
            mTotalTimes = Integer.parseInt(mEtTimes.getText().toString());
            mInterval = Integer.parseInt(mEtInterval.getText().toString());
            mCurrentTimes = 0;
            if (mTotalTimes <= 0 || mInterval < 3) {
                Toast.makeText(this, "please input right number, times > 0 and interval >=3s",
                    Toast.LENGTH_SHORT).show();
                return;
            }
            mCurrentTimesTv.setText("0");
            mCnrTv.setText("0");
            mDriftTv.setText("0");
            mMaxCnrTv.setText("0");
            mMinCnrTv.setText("0");
            mMaxCnr = 0;
            mMinCnr = 0;
            mBtnStart.setEnabled(false);
            mBtnStop.setEnabled(true);
        } catch (NumberFormatException e) {
            Toast.makeText(this, "invalid input value", Toast.LENGTH_SHORT).show();
            return;
        }
        mHandler.sendEmptyMessage(HANDLE_START_TEST);
    }

    private void stopCWTest() {
        mBtnStart.setEnabled(true);
        mBtnStop.setEnabled(false);
        mHandler.removeMessages(HANDLE_START_TEST);
        mHandler.sendEmptyMessage(HANDLE_STOP_TEST);
    }

    /**
     * Send command to MNL server.
     *
     * @param command
     *            PMTK command to be send
     */
    private void sendCommand(String command) {
        Elog.i(TAG, "sendCommand:" + command);

        int index1 = command.indexOf(COMMAND_START);
        int index2 = command.indexOf(COMMAND_END);
        String com = command;
        if (index1 != -1 && index2 != -1) {

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
     * @param ss
     *            Response message
     */
    public void onResponse(String ss) {
        if (null == ss || ss.isEmpty()) {
            return;
        }

        if (!ss.contains("PMTK817")) {
            return;
        }
        int startIndex = ss.indexOf("$PMTK817");

        String response = ss.substring(startIndex);

        int endIndex = response.indexOf("*");
        endIndex = endIndex + 3;

        String res = response.substring(0, endIndex);
        Elog.i(TAG, "get response:" + res);

        if (res.startsWith("$PMTK817")) {

            Message m = mHandler.obtainMessage(HANDLE_UPDATE_RESULT);
            m.obj = res;
            mHandler.sendMessage(m);
        }


    }

    @Override
    protected Dialog onCreateDialog(int id) {
        Dialog dialog = null;
        AlertDialog.Builder builder = null;
        switch (id) {
        case DIALOG_REENTER:
            builder = new AlertDialog.Builder(this);
            builder.setTitle(R.string.cw_test);
            builder.setCancelable(false);
            builder.setMessage(getString(R.string.lbs_permission_granted_take_effect_msg));
            builder.setPositiveButton(R.string.dialog_ok,
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            finish();
                        }
                    });
            dialog = builder.create();
            break;
        default:
            break;
        }
        return dialog;
    }


    /**
     * Wakelock class.
     *
     */
    private class CwTestWakeLock {
        private PowerManager.WakeLock mScreenWakeLock = null;
        private PowerManager.WakeLock mCpuWakeLock = null;

        /**
         * Acquire CPU wake lock.
         *
         * @param context
         *            Getting lock context
         */
        void acquireCpuWakeLock(Context context) {
            if (mCpuWakeLock != null) {
                return;
            }

            PowerManager pm = (PowerManager) context
                    .getSystemService(Context.POWER_SERVICE);

            mCpuWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK
                    | PowerManager.ACQUIRE_CAUSES_WAKEUP, TAG);
            // | PowerManager.ON_AFTER_RELEASE, TAG);
            mCpuWakeLock.acquire();
        }

        /**
         * Acquire screen wake lock.
         *
         * @param context
         *            Getting lock context
         */
        void acquireScreenWakeLock(Context context) {
            if (mScreenWakeLock != null) {
                return;
            }

            PowerManager pm = (PowerManager) context
                    .getSystemService(Context.POWER_SERVICE);

            mScreenWakeLock = pm.newWakeLock(PowerManager.FULL_WAKE_LOCK
                    | PowerManager.ACQUIRE_CAUSES_WAKEUP, TAG);
            // | PowerManager.ON_AFTER_RELEASE, TAG);
            mScreenWakeLock.acquire();
        }

        /**
         * Release wake locks.
         */
        void release() {
            if (mCpuWakeLock != null) {
                mCpuWakeLock.release();
                mCpuWakeLock = null;
            }
            if (mScreenWakeLock != null) {
                mScreenWakeLock.release();
                mScreenWakeLock = null;
            }
        }
    }

}
