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

package com.mediatek.cta;

import android.Manifest;
import android.app.Activity;
import android.app.ActivityThread;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.PendingIntent;
import android.app.ProgressDialog;
import android.bluetooth.BluetoothAdapter;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.location.LocationRequest;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.net.ConnectivityManager;
import android.net.Uri;
import android.net.http.AndroidHttpClient;
import android.net.wifi.WifiManager;
import android.nfc.NfcAdapter;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.provider.Settings;
import android.telephony.SmsManager;
import android.telephony.SubscriptionManager;
import static android.telephony.SubscriptionManager.INVALID_SUBSCRIPTION_ID;
import android.telephony.TelephonyManager;
import android.text.InputType;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import com.android.internal.telephony.IPhoneSubInfo;
import com.android.internal.telephony.ISms;
import com.android.internal.telephony.ITelephony;

import com.mediatek.cta.camera.Camera;

import org.apache.http.HttpHost;
import org.apache.http.HttpRequest;
import org.apache.http.HttpResponse;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.entity.ByteArrayEntity;

import java.io.BufferedOutputStream;
import java.io.DataOutputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.net.URI;
import java.net.URISyntaxException;
import java.util.ArrayList;
import java.util.List;

public class CtaActivity extends Activity implements OnClickListener {
    private static final String TAG = "CTA";

    private static final boolean FOR_CUSTOMER = false;
    private static final int TEST_01_PHONE_CALL = 0;
    private static final int TEST_01_PHONE_CALL_INTENT = 1;
    private static final int TEST_01_PHONE_CALL_REFLECT = 2;
    private static final int TEST_01_PHONE_CALL_END = 3;
    private static final int TEST_02_SEND_SMS = 4;
    private static final int TEST_02_SEND_SMS2 = 5;
    private static final int TEST_02_SEND_SMS_REFLECT1 = 6;
    private static final int TEST_02_SEND_SMS_REFLECT2 = 7;
    private static final int TEST_02_SEND_SMS_REFLECT3 = 8;
    private static final int TEST_02_SEND_MMS = 9;
    private static final int TEST_02_SEND_MMS_INTENT = 10;
    private static final int TEST_03_DATA_CONN = 11;
    private static final int TEST_03_DATA_CONN_REFLECT = 12;
    private static final int TEST_03_DATA_CONN_INTENT = 13;
    private static final int TEST_04_WIFI = 14;
    private static final int TEST_05_LOC_INTENT = 15;
    private static final int TEST_05_LOC_INTENT2 = 16;
    private static final int TEST_05_LOC_REQ = 17;
    private static final int TEST_05_LOC_GET = 18;
    private static final int TEST_06_RECORD_PHONE = 19;
    private static final int TEST_06_RECORD_PHONE2 = 20;
    private static final int TEST_07_RECORD = 21;
    private static final int TEST_07_RECORD2 = 22;
    private static final int TEST_08_CAMERA = 23;
    private static final int TEST_08_CAMERA_RECORD = 24;
    private static final int TEST_09_READ_SMS = 25;
    private static final int TEST_09_READ_MMS = 26;
    private static final int TEST_09_READ_CONTACT = 27;
    private static final int TEST_09_READ_CALLLOG = 28;
    private static final int TEST_10_BT = 29;
    private static final int TEST_10_NFC = 30;
    private static final int TEST_11_READ_CALENDAR = 31 ;
    private static final int TOTAL_TEST_CASE = 32;

    // Message IDs
    private static final int MSG_START_TEST = 1;
    private static final int MSG_TEST_FINISH = 2;
    private static final int MSG_TEST_PRECONDITION_FAIL = 3;
    private static final int MSG_TEST_FAILED = 4;
    private static final int MSG_TEST_SIM_NOT_READY = 5;

    // Dialog IDs
    private static final int DIALOG_WAIT = 0;
    private static final int DIALOG_DISABLE_NETWORK = 1;
    private static final int DIALOG_DISABLE_WIFI = 2;
    private static final int DIALOG_DISABLE_BT = 3;
    private static final int DIALOG_LOC = 5;
    private static final int DIALOG_PHONE_NUMBER = 6;
    private static final int DIALOG_SMS_NUMBER = 7;

    private static final String KEY_TEST_ID = "TestId";
    private static final String KEY_LOC = "Loc";
   private static final String PREF = "pref";
    private static final String KEY_PHONE_NUMBER = "call_phone_number";
    private static final String KEY_SMS_NUMBER = "sms_phone_number";
    private static final String DEFAULT_NUMBER = "10086";

    /**
     * All pdu header fields.
     */
    private static final int MESSAGE_ID = 0x8B;
    private static final int MESSAGE_TYPE = 0x8C;
    /**
     * X-Mms-Message-Type field types.
     */
    private static final int MESSAGE_TYPE_SEND_REQ = 0x80;
    private static final int MESSAGE_TYPE_SEND_CONF = 0x81;
    private static final int MESSAGE_TYPE_NOTIFICATION_IND = 0x82;
    private static final int MESSAGE_TYPE_NOTIFYRESP_IND = 0x83;

    private Button[] mButtons = new Button[TOTAL_TEST_CASE];
    private ProgressDialog mDialog = null;
    private Toast mToast = null;

    private MediaRecorder mRecorder;
    private LocationManager mLocationManager;
    private AudioRecord mAudioRecord;
    private String[] mStringArray = null;
    private boolean mIsRecordingPhone = false;
    private boolean mIsRecordingAudio = false;
    private SurfaceHolder mHolder;

    private LocationListener mLocListener = new LocationListener() {
        public void onLocationChanged(Location location) {
            showToast("onLocationChanged() return: "
                    + " getLatitude: " + location.getLatitude()
                    + " getLongitude: " + location.getLongitude()
                    + " getAltitude: " + location.getAltitude()
                    + " getAccuracy: " + location.getAccuracy()
                    + " getBearing: " + location.getBearing()
                    + " getSpeed: " + location.getSpeed());
        }

        public void onStatusChanged(String provider, int status, Bundle extras) {
            // Do nothing
        }

        public void onProviderEnabled(String provider) {
            // Do nothing
        }

        public void onProviderDisabled(String provider) {
            // Do nothing
        }
    };

    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            if (msg.what == MSG_START_TEST) {
                doTest(msg.arg1);
            } else {
                showResult(msg.what, msg.arg1);
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.setContentView(R.layout.cta_test);
        mStringArray = getResources().getStringArray(R.array.test_case_array);

        // Add buttons
        ViewGroup root = (ViewGroup) findViewById(R.id.layout_root);
        for (int i = 0; i < TOTAL_TEST_CASE; i++) {
            mButtons[i] = new Button(this);
            mButtons[i].setId(i);
            mButtons[i].setText(mStringArray[i]);
            mButtons[i].setOnClickListener(this);

            // Phase out some APIs
            if (isPhaseOut(i) == true) {
                continue;
            }

            // hide for customer
            if (FOR_CUSTOMER && (i == TEST_01_PHONE_CALL_END
                    || i == TEST_02_SEND_MMS_INTENT
                    || i == TEST_03_DATA_CONN_INTENT
                    || i == TEST_05_LOC_INTENT
                    || i == TEST_05_LOC_INTENT2
                    || i == TEST_10_NFC)) {
                continue;
            }
            if (i == TEST_10_NFC && NfcAdapter.getDefaultAdapter(this) == null) {
                continue;
            }
            root.addView(mButtons[i]);
        }
    }

    private boolean isPhaseOut(int testIndex) {
        boolean result = false;
        switch (testIndex) {
        case TEST_01_PHONE_CALL:
        case TEST_02_SEND_SMS:
        case TEST_03_DATA_CONN:
        case TEST_05_LOC_INTENT2:
        case TEST_03_DATA_CONN_REFLECT:
            result = true;
            break;
        default:
        }
        return result;
    }

    @Override
    public void onDestroy() {
        if (mButtons[TEST_05_LOC_REQ].getText()
                .equals(getString(R.string.cta_test_stop_request))) {
            Log.i(TAG, "onDestroy removeUpdates()");
            mLocationManager.removeUpdates(mLocListener);
        }
        String stop = getString(R.string.cta_test_stop_record);
        if (mButtons[TEST_06_RECORD_PHONE].getText().equals(stop)
                || mButtons[TEST_07_RECORD].getText().equals(stop)) {
            Log.i(TAG, "onDestroy MediaRecorder.stop()");
            try {
                mRecorder.stop();
            } catch (IllegalStateException e) {
                Log.i(TAG, "onDestroy MediaRecorder.stop() IllegalStateException");
            }
        }
        mIsRecordingPhone = false;
        mIsRecordingAudio = false;
        super.onDestroy();
    }

    @Override
    public void onClick(View v) {
        int testId = v.getId();
        Bundle bundle = new Bundle();
        switch (testId) {
        case TEST_01_PHONE_CALL:
        case TEST_01_PHONE_CALL_INTENT:
        case TEST_01_PHONE_CALL_REFLECT:
            bundle.putInt(KEY_TEST_ID, testId);
            removeDialog(DIALOG_PHONE_NUMBER);
            showDialog(DIALOG_PHONE_NUMBER, bundle);
            return;
        case TEST_02_SEND_SMS:
        case TEST_02_SEND_SMS2:
        case TEST_02_SEND_SMS_REFLECT1:
        case TEST_02_SEND_SMS_REFLECT2:
        case TEST_02_SEND_SMS_REFLECT3:
        case TEST_02_SEND_MMS:
        case TEST_02_SEND_MMS_INTENT:
            bundle.putInt(KEY_TEST_ID, testId);
            removeDialog(DIALOG_SMS_NUMBER);
            showDialog(DIALOG_SMS_NUMBER, bundle);
            return;
        case TEST_03_DATA_CONN:
        case TEST_04_WIFI:
        case TEST_10_BT:
            showDialog(DIALOG_WAIT);
            break;
        case TEST_08_CAMERA_RECORD:
            SurfaceView view = new SurfaceView(this);
            view.getHolder().addCallback(new SurfaceHolder.Callback() {
                @Override
                public void surfaceChanged(SurfaceHolder arg0, int arg1, int arg2, int arg3) {
                }

                @Override
                public void surfaceCreated(SurfaceHolder holder) {
                    mHolder = holder;
                    mHandler.sendMessage(mHandler.obtainMessage(
                            MSG_START_TEST, TEST_08_CAMERA_RECORD, 0));
                }

                @Override
                public void surfaceDestroyed(SurfaceHolder arg0) {
                }
            });
            setContentView(view);
            showDialog(DIALOG_WAIT);
            return;
        default:
            break;
        }
        enableButtons(false);
        mHandler.sendMessage(mHandler.obtainMessage(MSG_START_TEST, testId, 0));
    }

    @Override
    protected Dialog onCreateDialog(int id, Bundle args) {
        final int testId;
        final EditText input = new EditText(this);
        final SharedPreferences preferences = getSharedPreferences(PREF, MODE_PRIVATE);
        DialogInterface.OnClickListener listener;
        switch (id) {
        case DIALOG_WAIT:
            mDialog = new ProgressDialog(this);
            mDialog.setMessage(getString(R.string.cta_test_testing));
            mDialog.setCancelable(false);
            return mDialog;
        case DIALOG_DISABLE_NETWORK:
            return new AlertDialog.Builder(this)
                    .setTitle(getString(R.string.cta_test_error))
                    .setMessage(getString(R.string.cta_test_disable_data_conn))
                    .setNeutralButton(android.R.string.ok, null).create();
        case DIALOG_DISABLE_WIFI:
            return new AlertDialog.Builder(this)
                    .setTitle(getString(R.string.cta_test_error))
                    .setMessage(getString(R.string.cta_test_disable_wifi))
                    .setNeutralButton(android.R.string.ok, null).create();
        case DIALOG_DISABLE_BT:
            return new AlertDialog.Builder(this)
                    .setTitle(getString(R.string.cta_test_error))
                    .setMessage(getString(R.string.cta_test_disable_bt))
                    .setNeutralButton(android.R.string.ok, null).create();
        case DIALOG_LOC:
            return new AlertDialog.Builder(this)
                    .setTitle("Location")
                    .setMessage("getLastLocation() return:\n" + args.getString(KEY_LOC))
                    .setNeutralButton(android.R.string.ok, null).create();
        case DIALOG_PHONE_NUMBER:
            testId = args.getInt(KEY_TEST_ID, TEST_01_PHONE_CALL);
            input.setInputType(InputType.TYPE_CLASS_NUMBER);
            input.setText(preferences.getString(KEY_PHONE_NUMBER, DEFAULT_NUMBER));

            listener = new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    if (input.getText() != null && (!input.getText().toString().equals(""))) {
                        String number = input.getText().toString();
                        final SharedPreferences preferences = CtaActivity.this.getSharedPreferences(
                                PREF, Context.MODE_PRIVATE);
                        preferences.edit().putString(KEY_PHONE_NUMBER, number).commit();
                        mHandler.sendMessage(mHandler.obtainMessage(MSG_START_TEST, testId, 0));
                    }
                }
            };

            return new AlertDialog.Builder(this)
                    .setTitle("Phone number:")
                    .setView(input)
                    .setPositiveButton("OK", listener)
                    .setNegativeButton("Cancel", null).create();
        case DIALOG_SMS_NUMBER:
            testId = args.getInt(KEY_TEST_ID, TEST_02_SEND_SMS);
            input.setInputType(InputType.TYPE_CLASS_NUMBER);
            input.setText(preferences.getString(KEY_SMS_NUMBER, DEFAULT_NUMBER));

            listener = new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int which) {
                    if (input.getText() != null && (!input.getText().toString().equals(""))) {
                        String number = input.getText().toString();
                        final SharedPreferences preferences = CtaActivity.this.getSharedPreferences(
                                PREF, Context.MODE_PRIVATE);
                        preferences.edit().putString(KEY_SMS_NUMBER, number).commit();
                        mHandler.sendMessage(mHandler.obtainMessage(MSG_START_TEST, testId, 0));
                        showDialog(DIALOG_WAIT);
                    }
                }
            };

            return new AlertDialog.Builder(this)
                    .setTitle("Phone number:")
                    .setView(input)
                    .setPositiveButton("OK", listener)
                    .setNegativeButton("Cancel", null).create();
        default:
            return super.onCreateDialog(id);
        }
    }



    private void doTest(int index) {
        int ret = MSG_TEST_FINISH;
        int prevSubId = INVALID_SUBSCRIPTION_ID;
        int newSubId = INVALID_SUBSCRIPTION_ID;

        switch (index) {
        case TEST_01_PHONE_CALL:
            ret = test01PhoneCall();
            break;
        case TEST_01_PHONE_CALL_REFLECT:
            if (alreadyGetPermission(Manifest.permission.CALL_PHONE, index)) {
                test01PhoneCallReflectIfGranted(index) ;
            }
            break;
        case TEST_01_PHONE_CALL_INTENT:
            if (alreadyGetPermission(Manifest.permission.CALL_PHONE, index)) {
                test01PhoneCallIntentIfGranted(index) ;
            }
            break;
        case TEST_01_PHONE_CALL_END:
            if (alreadyGetPermission(Manifest.permission.CALL_PHONE, index)) {
                test01PhoneCallEndIfGranted(index) ;
            }
            break;
        case TEST_02_SEND_SMS:
        case TEST_02_SEND_SMS2:
            if (alreadyGetPermission(Manifest.permission.SEND_SMS, index)) {
                test02SendSmsIfGranted(index) ;
            }
            break;
        case TEST_02_SEND_SMS_REFLECT1:
        case TEST_02_SEND_SMS_REFLECT2:
        case TEST_02_SEND_SMS_REFLECT3:
            if (alreadyGetPermission(Manifest.permission.SEND_SMS, index)) {
                test02SendSmsReflectIfGranted(index);
            }
            break;
        case TEST_02_SEND_MMS:
            test02SendMms();
            mHandler.sendMessage(mHandler.obtainMessage(ret, index, 700));
            break;
        case TEST_02_SEND_MMS_INTENT:
            test02SendMmsIntent();
            mHandler.sendMessage(mHandler.obtainMessage(ret, index, 700));
            break;
        case TEST_03_DATA_CONN:
            if (test03DataConnectPrecondition()) {
                test03DataConnect();
                mHandler.sendMessage(mHandler.obtainMessage(ret, index, 700));
            } else {
                mHandler.sendMessage(mHandler.obtainMessage(MSG_TEST_PRECONDITION_FAIL, index, 0));
                return;
            }
            break;
        case TEST_03_DATA_CONN_REFLECT:
            if (test03DataConnectPrecondition()) {
                ret = test03DataConnectReflect();
            } else {
                mHandler.sendMessage(mHandler.obtainMessage(MSG_TEST_PRECONDITION_FAIL, index, 0));
                return;
            }
            break;
        case TEST_03_DATA_CONN_INTENT:
            test03DataConnectIntent();
            mHandler.sendMessage(mHandler.obtainMessage(ret, index, 700));
            break;
        case TEST_04_WIFI:
            if (test04EnableWifiPrecondition()) {
                test04EnableWifi();
                mHandler.sendMessage(mHandler.obtainMessage(ret, index, 700));
            } else {
                mHandler.sendMessage(mHandler.obtainMessage(MSG_TEST_PRECONDITION_FAIL, index, 0));
                return;
            }
            break;
        case TEST_05_LOC_INTENT:
        case TEST_05_LOC_INTENT2:
            test05LocationIfGranted(index) ;
            break ;
        case TEST_05_LOC_REQ:
        case TEST_05_LOC_GET:
            String[] locationPermissions =
                new String[] {
                        Manifest.permission.ACCESS_FINE_LOCATION,
                        Manifest.permission.ACCESS_COARSE_LOCATION
                } ;
            if (alreadyGetPermissions(locationPermissions, index)) {
                test05LocationIfGranted(index) ;
            }
            break;
        case TEST_06_RECORD_PHONE:
        case TEST_06_RECORD_PHONE2:
        case TEST_07_RECORD:
        case TEST_07_RECORD2:
            if (alreadyGetPermission(Manifest.permission.RECORD_AUDIO, index)
            ) {
                test06RecordAudioIfGranted(index) ;
            }
            break ;
        case TEST_08_CAMERA:
            if (alreadyGetPermission(Manifest.permission.CAMERA, index)) {
                test08CameraIfGranted(index) ;
            }
            break;
        case TEST_08_CAMERA_RECORD:
            if (alreadyGetPermission(Manifest.permission.CAMERA, index)) {
                test08camerarecord();
            }
            return;
        case TEST_09_READ_CONTACT:
            if (alreadyGetPermission(Manifest.permission.READ_CONTACTS, index)) {
                test09ReadContactsIfGranted(index) ;
            }
            break;
        case TEST_09_READ_CALLLOG:
            if (alreadyGetPermission(Manifest.permission.READ_CALL_LOG, index)) {
                test09readcalllogIfGranted(index) ;
            }
            break;
        case TEST_09_READ_SMS:
        case TEST_09_READ_MMS:
            if (alreadyGetPermission(Manifest.permission.READ_SMS, index)) {
                test09readSmsMmsIfGranted(index) ;
            }
            break;
        case TEST_10_BT:
            if (test10EnableBluetoothPrecondition()) {
                test10EnableBluetooth();
                mHandler.sendMessage(mHandler.obtainMessage(ret, index, 700));
            } else {
                mHandler.sendMessage(mHandler.obtainMessage(MSG_TEST_PRECONDITION_FAIL, index, 0));
                return;
            }
            break;
        case TEST_10_NFC:
            test10EnableNfc();
            mHandler.sendMessage(mHandler.obtainMessage(ret, index, 700));
            break;
        case TEST_11_READ_CALENDAR:
            if (alreadyGetPermission(Manifest.permission.READ_CALENDAR, index)) {
                test11readcalendarIfGranted(index);
            }
            break;
        default:
            break;
        }
    }

    private void showResult(int result, int index) {
        enableButtons(true);
        if (mDialog != null) {
            mDialog.dismiss();
        }

        if (result == MSG_TEST_FINISH) {
            switch (index) {
            case TEST_06_RECORD_PHONE:
            case TEST_06_RECORD_PHONE2:
            case TEST_07_RECORD:
            case TEST_07_RECORD2:
                if (mButtons[index].getText().equals(getString(R.string.cta_test_stop_record))) {
                    mButtons[index].setText(mStringArray[index]);
                } else {
                    mButtons[index].setText(R.string.cta_test_stop_record);
                }
                break;
            case TEST_05_LOC_REQ:
                if (mButtons[TEST_05_LOC_REQ].getText()
                        .equals(getString(R.string.cta_test_stop_request))) {
                    mButtons[TEST_05_LOC_REQ].setText(R.string.cta_test_5_location_request);
                } else {
                    mButtons[TEST_05_LOC_REQ].setText(R.string.cta_test_stop_request);
                }
                break;
            case TEST_08_CAMERA_RECORD:
                finish();
                startActivity(new Intent(this, CtaActivity.class));
                return;
            default:
                break;
            }
        } else if (result == MSG_TEST_PRECONDITION_FAIL) {
            switch (index) {
            case TEST_03_DATA_CONN:
            case TEST_03_DATA_CONN_REFLECT:
                showDialog(DIALOG_DISABLE_NETWORK);
                break;
            case TEST_04_WIFI:
                showDialog(DIALOG_DISABLE_WIFI);
                break;
            case TEST_10_BT:
                showDialog(DIALOG_DISABLE_BT);
                break;
            default:
                break;
            }
        } else if (result == MSG_TEST_SIM_NOT_READY) {
            showToast("Sim card is not ready");
        } else if (result == MSG_TEST_FAILED) {
            showToast("Failed!");
        }
    }

    private String phoneNumber() {
        final SharedPreferences preferences = getSharedPreferences(PREF, MODE_PRIVATE);
        return preferences.getString(KEY_PHONE_NUMBER, DEFAULT_NUMBER);
    }

    private String smsNumber() {
        final SharedPreferences preferences = getSharedPreferences(PREF, MODE_PRIVATE);
        return preferences.getString(KEY_SMS_NUMBER, DEFAULT_NUMBER);
    }

    private int test01PhoneCall() {
        try {
            String packageName = ActivityThread.currentPackageName();
            ITelephony iTel =
                    ITelephony.Stub.asInterface(ServiceManager.getService(TELEPHONY_SERVICE));
            Log.i(TAG, "test01PhoneCall " + phoneNumber());
            iTel.call(packageName, phoneNumber());
            return MSG_TEST_FINISH;
        } catch (RemoteException e) {
            Log.e(TAG, e.toString());
            return MSG_TEST_FAILED;
        }
    }

    private void test01PhoneCallIntent() {
        Log.i(TAG, "test01PhoneCallIntent " + phoneNumber());
        Intent phoneIntent = new Intent("android.intent.action.CALL",
                Uri.parse("tel:" + phoneNumber()));
        phoneIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(phoneIntent);
    }

    private int test01PhoneCallReflect() {
        try {
            String packageName = ActivityThread.currentPackageName();
            Log.i(TAG, "test01PhoneCallReflect " + packageName + ", " + phoneNumber());
            Method method = Class.forName("android.os.ServiceManager")
                    .getMethod("getService", String.class);
            method.setAccessible(true);
            IBinder binder = (IBinder) method.invoke(null, new Object[] {TELEPHONY_SERVICE});
            ITelephony phone = (ITelephony) ITelephony.Stub.asInterface(binder);
            phone.call(packageName, phoneNumber());
            return MSG_TEST_FINISH;
        } catch (ClassNotFoundException e) {
            Log.e(TAG, e.toString());
        } catch (NoSuchMethodException e) {
            Log.e(TAG, e.toString());
        } catch (InvocationTargetException e) {
            Log.e(TAG, e.toString());
        } catch (IllegalAccessException e) {
            Log.e(TAG, e.toString());
        } catch (RemoteException e) {
            Log.e(TAG, e.toString());
        }
        return MSG_TEST_FAILED;
    }

    private int test01PhoneCallEnd(int subId) {
        try {
            Log.i(TAG, "test01PhoneCallEnd");
            Method method = Class.forName("android.os.ServiceManager")
                    .getMethod("getService", String.class);
            method.setAccessible(true);
            IBinder binder = (IBinder) method.invoke(null, new Object[] {TELEPHONY_SERVICE});
            ITelephony phone = (ITelephony) ITelephony.Stub.asInterface(binder);
            phone.endCallForSubscriber(subId);
            return MSG_TEST_FINISH;
        } catch (ClassNotFoundException e) {
            Log.e(TAG, e.toString());
        } catch (NoSuchMethodException e) {
            Log.e(TAG, e.toString());
        } catch (InvocationTargetException e) {
            Log.e(TAG, e.toString());
        } catch (IllegalAccessException e) {
            Log.e(TAG, e.toString());
        } catch (RemoteException e) {
            Log.e(TAG, e.toString());
        }
        return MSG_TEST_FAILED;
    }

    private int getDefaultVoiceSubId() {
        int subId = 0;
        try {
            Method method = Class.forName("android.telephony.SubscriptionManager")
                    .getMethod("getDefaultVoiceSubcriptionId", (Class[]) null);
            method.setAccessible(true);
            subId = (int) method.invoke(null, (Object[]) null);
        } catch (ClassNotFoundException e) {
            Log.e(TAG, e.toString());
        } catch (NoSuchMethodException e) {
            Log.e(TAG, e.toString());
        } catch (InvocationTargetException e) {
            Log.e(TAG, e.toString());
        } catch (IllegalAccessException e) {
            Log.e(TAG, e.toString());
        }
        return subId;
    }

    private void setDefaultVoiceSubId(int subId) {
        SubscriptionManager sm = SubscriptionManager.from((Context) this);
        try {
            Method method = Class.forName("android.telephony.SubscriptionManager")
                    .getMethod("setDefaultVoiceSubId", Integer.TYPE);
            method.setAccessible(true);
            method.invoke((Object)sm, subId);
        } catch (ClassNotFoundException e) {
            Log.e(TAG, e.toString());
        } catch (NoSuchMethodException e) {
            Log.e(TAG, e.toString());
        } catch (InvocationTargetException e) {
            Log.e(TAG, e.toString());
        } catch (IllegalAccessException e) {
            Log.e(TAG, e.toString());
        }
    }

    private int getDefaultSubId() {
        return SubscriptionManager.getDefaultSubscriptionId();
    }

    private void setDefaultSubId(int subId) {
        try {
            Method method = Class.forName("com.mediatek.internal.telephony.MtkSubscriptionManager")
                    .getMethod("setDefaultSubId", (Class[]) null);
            method.setAccessible(true);
            method.invoke(null, subId);
        } catch (ClassNotFoundException e) {
            Log.e(TAG, e.toString());
        } catch (NoSuchMethodException e) {
            Log.e(TAG, e.toString());
        } catch (InvocationTargetException e) {
            Log.e(TAG, e.toString());
        } catch (IllegalAccessException e) {
            Log.e(TAG, e.toString());
        }
    }

    // Reture new subId
    private int setupSubIdIfValid() {
        int newSubId = INVALID_SUBSCRIPTION_ID;
        boolean setSubIdSuccess = false;
        // Tty slots
        int[] subIds = SubscriptionManager.getSubId(0);
        if (subIds != null && subIds.length > 0 && subIds[0] > 0) {
            newSubId = subIds[0];
            setSubIdSuccess = true;
            Log.i(TAG, "Find slot 0 subId: " + newSubId);
        } else { // Try slot 2
            subIds = SubscriptionManager.getSubId(1);
            if (subIds != null && subIds.length > 0 && subIds[0] > 0) {
                newSubId = subIds[0];
                Log.i(TAG, "Find slot 1 subId: " + newSubId);
            } else {
                return INVALID_SUBSCRIPTION_ID;
            }
        }
        setDefaultSubId(newSubId);
        Log.i(TAG, "Set new subId: " + newSubId);
        return newSubId;
    }

    private int test02SendSms(int subId) {
        try {
            String packageName = ActivityThread.currentPackageName();
            ISms iccISms = ISms.Stub.asInterface(ServiceManager.getService("isms"));
            Log.i(TAG, "test02SendSms " + smsNumber());
            iccISms.sendDataForSubscriber(subId, packageName,
                    smsNumber(), null, 0, "Hi".getBytes(), null, null);
            return MSG_TEST_FINISH;
        } catch (RemoteException e) {
            Log.e(TAG, e.toString());
        return MSG_TEST_FAILED;
        }
    }

    private int test02SendSms2(int subId) {
        SmsManager smsManager = SmsManager.getSmsManagerForSubscriptionId(subId);
        Log.i(TAG, "test02SendSms2 " + smsNumber());
        smsManager.sendTextMessage(smsNumber(), null, "SMS message", null, null);
        return MSG_TEST_FINISH;
    }

    private int test02SendSmsReflect1(int subId) {
        try {
            String packageName = ActivityThread.currentPackageName();
            Log.i(TAG, "test02SendSmsReflect1 " + packageName + ", " + smsNumber());
            Method method = Class.forName("android.os.ServiceManager")
                    .getMethod("getService", String.class);
            method.setAccessible(true);
            IBinder binder = (IBinder) method.invoke(null, new Object[] {"isms"});
            ISms simISms = (ISms) ISms.Stub.asInterface(binder);
            simISms.sendTextForSubscriber(subId, packageName,
                    smsNumber(), null, "SMS message (Reflect 1)", null, null, true);
            return MSG_TEST_FINISH;
        } catch (ClassNotFoundException e) {
            Log.e(TAG, e.toString());
        } catch (NoSuchMethodException e) {
            Log.e(TAG, e.toString());
        } catch (InvocationTargetException e) {
            Log.e(TAG, e.toString());
        } catch (IllegalAccessException e) {
            Log.e(TAG, e.toString());
        } catch (RemoteException e) {
            Log.e(TAG, e.toString());
        }
        return MSG_TEST_FAILED;
    }

    private int test02SendSmsReflect2(int subId) {
        try {
            String packageName = ActivityThread.currentPackageName();
            Log.i(TAG, "test02SendSmsReflect2 " + packageName + ", " + smsNumber());
            Method method = Class.forName("android.os.ServiceManager")
              .getMethod("getService", String.class);
            method.setAccessible(true);
            IBinder binder = (IBinder) method.invoke(null, new Object[] {"isms"});
            ISms simISms = (ISms) ISms.Stub.asInterface(binder);
            byte[] bytes = "SMS message (Reflect 2)".getBytes("GBK");
            simISms.sendDataForSubscriber(subId, packageName,
                    smsNumber(), null, 0, bytes, null, null);
            return MSG_TEST_FINISH;
        } catch (ClassNotFoundException e) {
            Log.e(TAG, e.toString());
        } catch (NoSuchMethodException e) {
            Log.e(TAG, e.toString());
        } catch (InvocationTargetException e) {
            Log.e(TAG, e.toString());
        } catch (IllegalAccessException e) {
            Log.e(TAG, e.toString());
        } catch (UnsupportedEncodingException e) {
            Log.e(TAG, e.toString());
        } catch (RemoteException e) {
            Log.e(TAG, e.toString());
        }
        return MSG_TEST_FAILED;
    }

    private int test02SendSmsReflect3(int subId) {
        try {
            String packageName = ActivityThread.currentPackageName();
            Log.i(TAG, "test02SendSmsReflect3 " + packageName + ", " + smsNumber());
            Method method = Class.forName("android.os.ServiceManager")
                    .getMethod("getService", String.class);
            method.setAccessible(true);
            IBinder binder = (IBinder) method.invoke(null, new Object[] {"isms"});
            ISms simISms = (ISms) ISms.Stub.asInterface(binder);
            List<String> parts = new ArrayList<String>();
            parts.add("SMS message (Reflect 3)");
            List<PendingIntent> intents = new ArrayList<PendingIntent>();
            intents.add(null);
            simISms.sendMultipartTextForSubscriber(subId, packageName,
                    smsNumber(), null, parts, intents, null, true);
            return MSG_TEST_FINISH;
        } catch (ClassNotFoundException e) {
            Log.e(TAG, e.toString());
        } catch (NoSuchMethodException e) {
            Log.e(TAG, e.toString());
        } catch (InvocationTargetException e) {
            Log.e(TAG, e.toString());
        } catch (IllegalAccessException e) {
            Log.e(TAG, e.toString());
        } catch (RemoteException e) {
            Log.e(TAG, e.toString());
        }
        return MSG_TEST_FAILED;
    }

    private void test02SendMmsIntent() {
        Intent intent = new Intent(Intent.ACTION_SEND);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        intent.putExtra("subject", "CTA_TEST");
        intent.putExtra("address", smsNumber());
        intent.putExtra("sms_body", "This is CTA MMS Test");
        intent.putExtra(Intent.EXTRA_TEXT, "it's EXTRA_TEXT");
        intent.setType("image/*");
        intent.setClassName("com.android.mms", "com.android.mms.ui.ComposeMessageActivity");
        startActivity(intent);
    }

    private void createHttpClient2() {
        AndroidHttpClient httpClient = AndroidHttpClient.newInstance("MMS 1.0");
        String url = "http://www.baidu.com";
        byte[] pdu;

        try {
            URI hostUrl = new URI(url);
            HttpHost target = new HttpHost(hostUrl.getHost(), hostUrl.getPort(),
                    HttpHost.DEFAULT_SCHEME_NAME);
            HttpPost post = new HttpPost(url);
            HttpRequest req = null;

            pdu = new byte[2];
            pdu[0] = (byte) MESSAGE_TYPE;
            pdu[1] = (byte) MESSAGE_TYPE_SEND_REQ;

            ByteArrayEntity entity = new ByteArrayEntity(pdu);
            entity.setContentType("application/vnd.wap.mms-message");
            post.setEntity(entity);
            req = post;

            final HttpResponse execute = httpClient.execute(target, req);
        } catch (URISyntaxException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void test02SendMms() {
        Log.i(TAG, "test02SendMms()");
        // Some APIs JE in main thread
        new Thread(new Runnable() {
            public void run() {
                createHttpClient2();
            }
        }).start();
    }

    private boolean test03DataConnectPrecondition() {
        ConnectivityManager connService = ConnectivityManager.from(this);
        return !connService.getMobileDataEnabled();
    }

    private void test03DataConnect() {
        ConnectivityManager connService = ConnectivityManager.from(this);
        Log.i(TAG, "test03DataConnect call setMobileDataEnabled()");
//        connService.setMobileDataEnabled(true);
    }

    private int test03DataConnectReflect() {
        Log.i(TAG, "test03DataConnectReflect");
        try {
            TelephonyManager mTelephonyManager = (TelephonyManager)
                    getSystemService(Context.TELEPHONY_SERVICE);
            Class ownerClass = mTelephonyManager.getClass();
            Class[] argsClass = new Class[1];
            argsClass[0] = boolean.class;
            Method method = ownerClass.getMethod("setDataEnabled", argsClass);
            method.invoke(mTelephonyManager, true);
            return MSG_TEST_FINISH;
        } catch (NoSuchMethodException e) {
            Log.e(TAG, e.toString());
        } catch (IllegalAccessException e) {
            Log.e(TAG, e.toString());
        } catch (InvocationTargetException e) {
            Log.e(TAG, e.toString());
        }
        return MSG_TEST_FAILED;
    }

    private void test03DataConnectIntent() {
        Log.i(TAG, "test03DataConnectIntent send Settings.ACTION_WIRELESS_SETTINGS");
        Intent intent = new Intent(Settings.ACTION_WIRELESS_SETTINGS);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(intent);
    }

    private boolean test04EnableWifiPrecondition() {
        WifiManager wifiManager = (WifiManager) this.getSystemService(WIFI_SERVICE);
        return wifiManager.getWifiState() == WifiManager.WIFI_STATE_DISABLED;
    }

    private void test04EnableWifi() {
        WifiManager wifiManager = (WifiManager) this.getSystemService(WIFI_SERVICE);
        Log.i(TAG, "test04EnableWifi call setWifiEnabled()");
        try {
            wifiManager.setWifiEnabled(true);
        } catch (SecurityException e) {
            Log.e(TAG, e.toString());
        }
    }

    private void test05RequestLocation() {
        mLocationManager = (LocationManager) getSystemService(LOCATION_SERVICE);
        if (mButtons[TEST_05_LOC_REQ].getText().equals(getString(R.string.cta_test_stop_request))) {
            Log.i(TAG, "test05RequestLocation call removeUpdates()");
            mLocationManager.removeUpdates(mLocListener);
        } else {
            Log.i(TAG, "test05RequestLocation call requestLocationUpdates()");
            LocationRequest request = LocationRequest.create();
            request.setQuality(LocationRequest.POWER_HIGH);
            request.setInterval(1000);
            try {
                mLocationManager.requestLocationUpdates(request, mLocListener, null);
            } catch (SecurityException e) {
                Log.e(TAG, e.toString());
            }
        }
    }

    private void test05GetLastLocation() {
        mLocationManager = (LocationManager) getSystemService(LOCATION_SERVICE);
        Log.i(TAG, "test05GetLastLocation call getLastLocation()");
        Location location = null;
        Bundle bundle = new Bundle();
        try {
            location = mLocationManager.getLastLocation();
        } catch (SecurityException e) {
            Log.e(TAG, e.toString());
        }
        if (location != null) {
            Log.i(TAG, "getLastLocation() return not null");
            bundle.putString(KEY_LOC, "getLatitude: " + location.getLatitude()
                    + " getLongitude: " + location.getLongitude()
                    + " getAltitude: " + location.getAltitude()
                    + " getAccuracy: " + location.getAccuracy()
                    + " getBearing: " + location.getBearing()
                    + " getSpeed: " + location.getSpeed());
        } else {
            Log.i(TAG, "getLastLocation() return null");
        }
        removeDialog(DIALOG_LOC);
        showDialog(DIALOG_LOC, bundle);
    }

    private void test05RequestLocationIntent() {
        Log.i(TAG, "test05RequestLocationIntent() send ACTION_LOCATION_SOURCE_SETTINGS");
        Intent intent = new Intent(android.provider.Settings.ACTION_LOCATION_SOURCE_SETTINGS);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(intent);
    }

    private void test05RequestLocationIntent2() {
        Intent gpsIntent = new Intent();
        gpsIntent.setClassName("com.android.settings",
                "com.android.settings.widget.SettingsAppWidgetProvider");
        gpsIntent.addCategory("android.intent.category.ALTERNATIVE");
        gpsIntent.setData(Uri.parse("custom:3"));
        sendBroadcast(gpsIntent);
    }

    private void test06RecordPhone() {
        try {
            if (mButtons[TEST_06_RECORD_PHONE].getText()
                    .equals(getString(R.string.cta_test_stop_record))) {
                Log.i(TAG, "test06RecordPhone call MediaRecorder.stop()");
                mRecorder.stop();
            } else {
                Log.i(TAG, "test06RecordPhone call MediaRecorder.start()");
                mRecorder = new MediaRecorder();
                mRecorder.setAudioSource(MediaRecorder.AudioSource.MIC);
                mRecorder.setOutputFormat(MediaRecorder.OutputFormat.THREE_GPP);
                mRecorder.setAudioEncoder(MediaRecorder.AudioEncoder.AMR_NB);
                mRecorder.setOutputFile("/sdcard/cta_test_phone_record.3gpp");
                mRecorder.prepare();
                mRecorder.start();
            }
        } catch (IllegalStateException e) {
            Log.e(TAG, e.toString());
        } catch (IOException e) {
            Log.e(TAG, e.toString());
        } catch (RuntimeException e) {
            Log.e(TAG, e.toString());
        }
    }

    private void test06RecordPhone2() {
        if (mButtons[TEST_06_RECORD_PHONE2].getText()
                .equals(getString(R.string.cta_test_stop_record))) {
            Log.i(TAG, "test06RecordPhone2 stop");
            mIsRecordingPhone = false;
        } else {
            mIsRecordingPhone = true;
            Log.i(TAG, "test06RecordPhone2 start");
            new Thread(new Runnable() {
                public void run() {
                    try {
                        DataOutputStream dos = new DataOutputStream(new BufferedOutputStream(
                                new FileOutputStream("/sdcard/cta_test_phone_record.raw")));
                        int bufferSize = AudioRecord.getMinBufferSize(44100,
                                AudioFormat.CHANNEL_IN_STEREO, AudioFormat.ENCODING_PCM_16BIT) * 10;
                        short[] buffer = new short[bufferSize];
                        AudioRecord audioRecord = new AudioRecord(
                                MediaRecorder.AudioSource.MIC, 44100, AudioFormat.CHANNEL_IN_STEREO,
                                AudioFormat.ENCODING_PCM_16BIT, bufferSize);
                        Log.i(TAG, "test06RecordPhone2 startRecording()");
                        audioRecord.startRecording();

                        while (mIsRecordingPhone) {
                            Log.i(TAG, "test06RecordPhone2 read()");
                            int bufferReadResult = audioRecord.read(buffer, 0, buffer.length);
                            for (int i = 0; i < bufferReadResult; i++) {
                                dos.writeShort(buffer[i]);
                            }
                        }
                        Log.i(TAG, "test06RecordPhone2 stop()");
                        audioRecord.stop();
                        dos.close();
                    } catch (IllegalArgumentException e) { // Behavior on L
                        Log.e(TAG, e.toString());
                    } catch (IllegalStateException e) {
                        Log.e(TAG, e.toString());
                    } catch (FileNotFoundException e) {
                        Log.e(TAG, e.toString());
                    } catch (IOException e) {
                        Log.e(TAG, e.toString());
                    }
                }
            }).start();
        }
    }

    private void test07RecordAudio() {
        try {
            if (mButtons[TEST_07_RECORD].getText()
                    .equals(getString(R.string.cta_test_stop_record))) {
                Log.i(TAG, "test07RecordAudio call MediaRecorder.stop()");
                mRecorder.stop();
            } else {
                Log.i(TAG, "test07RecordAudio call MediaRecorder.start()");
                mRecorder = new MediaRecorder();
                mRecorder.setAudioSource(MediaRecorder.AudioSource.DEFAULT);
                mRecorder.setOutputFormat(MediaRecorder.OutputFormat.DEFAULT);
                mRecorder.setAudioEncoder(MediaRecorder.AudioEncoder.DEFAULT);
                mRecorder.setOutputFile("/sdcard/cta_test_audio_record.3gpp");
                mRecorder.prepare();
                mRecorder.start();
            }
        } catch (IllegalStateException e) {
            Log.e(TAG, e.toString());
        } catch (IOException e) {
            Log.e(TAG, e.toString());
        } catch (RuntimeException e) {
            Log.e(TAG, e.toString());
        }
    }

    private void test07RecordAudio2() {
        if (mButtons[TEST_07_RECORD2].getText().equals(getString(R.string.cta_test_stop_record))) {
            Log.i(TAG, "test07RecordAudio2 stop");
            mIsRecordingAudio = false;
        } else {
            mIsRecordingAudio = true;
            Log.i(TAG, "test07RecordAudio2 start");
            new Thread(new Runnable() {
                public void run() {
                    try {
                        DataOutputStream dos = new DataOutputStream(new BufferedOutputStream(
                                new FileOutputStream("/sdcard/cta_test_audio_record.raw")));
                        int bufferSize = AudioRecord.getMinBufferSize(44100,
                                AudioFormat.CHANNEL_IN_STEREO, AudioFormat.ENCODING_PCM_16BIT) * 10;
                        short[] buffer = new short[bufferSize];
                        AudioRecord audioRecord = new AudioRecord(
                                MediaRecorder.AudioSource.MIC, 44100, AudioFormat.CHANNEL_IN_STEREO,
                                AudioFormat.ENCODING_PCM_16BIT, bufferSize);
                        Log.i(TAG, "test07RecordAudio2 startRecording()");
                        audioRecord.startRecording();

                        while (mIsRecordingAudio) {
                            Log.i(TAG, "test07RecordAudio2 read()");
                            int bufferReadResult = audioRecord.read(buffer, 0, buffer.length);
                            for (int i = 0; i < bufferReadResult; i++) {
                                dos.writeShort(buffer[i]);
                            }
                        }
                        Log.i(TAG, "test07RecordAudio2 stop()");
                        audioRecord.stop();
                        dos.close();
                    } catch (IllegalArgumentException e) { // Behavior on L
                        Log.e(TAG, e.toString());
                    } catch (IllegalStateException e) {
                        Log.e(TAG, e.toString());
                    } catch (FileNotFoundException e) {
                        Log.e(TAG, e.toString());
                    } catch (IOException e) {
                        Log.e(TAG, e.toString());
                    }
                }
            }).start();
        }
    }

    private void test08camera() {
        Intent captureIntent = new Intent();
        captureIntent.setClass(this, Camera.class);
        this.startActivity(captureIntent);
    }

    private void test08camerarecord() {
        new Thread(new Runnable() {
            public void run() {
                try {
                    MediaRecorder recorder = new MediaRecorder();
                    recorder.setAudioSource(MediaRecorder.AudioSource.MIC);
                    recorder.setVideoSource(MediaRecorder.VideoSource.CAMERA);
                    recorder.setOutputFormat(MediaRecorder.OutputFormat.MPEG_4);
                    recorder.setVideoEncoder(MediaRecorder.VideoEncoder.H263);
                    recorder.setAudioEncoder(MediaRecorder.AudioEncoder.DEFAULT);
                    recorder.setVideoSize(176, 144);
                    recorder.setVideoFrameRate(30);
                    recorder.setPreviewDisplay(mHolder.getSurface());
                    recorder.setOutputFile("/sdcard/Record.mp4");
                    recorder.prepare();
                    recorder.start();
                    try {
                        Thread.sleep(10 * 1000);
                    } catch (InterruptedException e) {
                        // TODO Auto-generated catch block
                        e.printStackTrace();
                    }
                    recorder.stop();
                    recorder.reset();
                    recorder.release();
                } catch (IllegalStateException e) {
                    e.printStackTrace();
                    return;
                } catch (IOException e) {
                    e.printStackTrace();
                    return;
                } catch (RuntimeException e) {
                    Log.e(TAG, e.toString());
                    return;
                } finally {
                    mHandler.sendMessage(mHandler.obtainMessage(MSG_TEST_FINISH,
                            TEST_08_CAMERA_RECORD, 0));
                }
            }
        }).start();
    }

    private void test09readcontacts() {
        Intent intent = new Intent();
        intent.setClass(this, CtaListActivity.class);
        intent.putExtra(CtaListActivity.TYPE, CtaListActivity.TYPE_CONTACTS);
        this.startActivity(intent);
    }

    private void test09readcalllog() {
        Intent intent = new Intent();
        intent.setClass(this, CtaListActivity.class);
        intent.putExtra(CtaListActivity.TYPE, CtaListActivity.TYPE_CALL_LOG);
        this.startActivity(intent);
    }

    private void test09readsms() {
        Intent intent = new Intent();
        intent.setClass(this, CtaListActivity.class);
        intent.putExtra(CtaListActivity.TYPE, CtaListActivity.TYPE_SMS);
        this.startActivity(intent);
    }

    private void test09readmms() {
        Intent intent = new Intent();
        intent.setClass(this, CtaListActivity.class);
        intent.putExtra(CtaListActivity.TYPE, CtaListActivity.TYPE_MMS);
        this.startActivity(intent);
    }

    private boolean test10EnableBluetoothPrecondition() {
        BluetoothAdapter mBtAdapter = BluetoothAdapter.getDefaultAdapter();
        return mBtAdapter.getState() == BluetoothAdapter.STATE_OFF;
    }

    private void test10EnableBluetooth() {
        BluetoothAdapter mBtAdapter = BluetoothAdapter.getDefaultAdapter();
        Log.i(TAG, "test10EnableBluetooth call BluetoothAdapter.enable()");
        mBtAdapter.enable();
    }

    private void test10EnableNfc() {
        Intent intent = new Intent(Settings.ACTION_NFCSHARING_SETTINGS);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(intent);
    }

    private void test11readcalendar() {
        Intent intent = new Intent();
        intent.setClass(this, CtaListActivity.class);
        intent.putExtra(CtaListActivity.TYPE, CtaListActivity.TYPE_CALENDAR);
        this.startActivity(intent);
    }

    private void enableButtons(boolean enable) {
        new Throwable().printStackTrace() ;
        for (int i = 0; i < TOTAL_TEST_CASE; i++) {
            mButtons[i].setClickable(enable);
        }
    }

    private void showToast(String msg) {
        if (mToast != null) {
            mToast.cancel();
        }
        mToast = Toast.makeText(this, msg, Toast.LENGTH_LONG);
        mToast.show();
    }

    // Added for Runtime permission
    void showOnceDenyToast(String permission) {
        showDenyToast(permission, false) ;
    }

    void showAlwaysDenyToast(String permission) {
        showDenyToast(permission, true) ;
    }

    void showDenyToast(String permission, boolean isNeverAskAgain) {
        String msg = permission ;
        if (isNeverAskAgain) {
            msg = msg + " is alwyas denied. Goto Settings to re-grant." ;
        } else {
            msg = msg + " is denied." ;
        }
        Log.d(TAG, "showDenyToast() - msg = " + msg) ;
        Toast.makeText(CtaActivity.this, msg, Toast.LENGTH_SHORT).show();
    }

    boolean alreadyGetPermission(String permission, int requestCode) {
        Log.d(TAG, "alreadyGetPermission(perm = " + permission +
            ", requestCode = " + requestCode) ;
        if (checkSelfPermission(permission) !=
            PackageManager.PERMISSION_GRANTED) {
            Log.d(TAG, "alreadyGetPermission()" +
                permission + " is not granted, request it") ;
            if (!shouldShowRequestPermissionRationale(permission)) {
                showAlwaysDenyToast(permission);
                mHandler.sendMessage(mHandler.obtainMessage(MSG_TEST_FINISH, requestCode, 700));
                return false;
            }

            requestPermissions(new String[] {permission}, requestCode) ;
            return false;
        }
        return true;
    }

    boolean alreadyGetPermissions(String[] permissions, int requestCode) {
        List<String> needPermissions = new ArrayList<String>() ;
        boolean needToRequest = false ;

        for (int index = 0 ; index < permissions.length ; index++) {
            String permission = permissions[index] ;
            Log.d(TAG, "alreadyGetPermissions() - check perm = " +
                permission + " , requestCode = " + requestCode) ;

            if (checkSelfPermission(permission) !=
                    PackageManager.PERMISSION_GRANTED) {
                Log.d(TAG, "alreadyGetPermissionis()"
                    + permission + " is not granted, request it") ;
                needPermissions.add(permission) ;
                needToRequest = true ;
            }
        }

        if (needToRequest) {
            String[] permArray = new String[needPermissions.size()] ;
            permArray = needPermissions.toArray(permArray) ;

            for (int index = 0 ; index < permArray.length ; index++) {
                String permission = permArray[index] ;
                if (!shouldShowRequestPermissionRationale(permission)) {
                    showAlwaysDenyToast(permission);
                    mHandler.sendMessage(mHandler.obtainMessage(MSG_TEST_FINISH, requestCode, 700));
                    return false;
                }
            }

            requestPermissions(permArray, requestCode) ;
            return false;
        }
        return true;
    }

    @Override
    public void onRequestPermissionsResult(int requestCode,
        String[] permissions, int[] grantResults) {
        Log.d(TAG, "onRequestPermissionsResult() - " +
            "requestCode = "  + requestCode) ;
        boolean allPermissionsGranted = true ;
        String deniedPermission = null ;
        for (String permission : permissions) {
            if (checkSelfPermission(permission) !=
                PackageManager.PERMISSION_GRANTED) {
                Log.d(TAG, "onRequestPermissionsResult() - requestCode = " +
                    requestCode + " perm = " + permission + " is revoked.") ;
                allPermissionsGranted = false ;
                deniedPermission = permission ;
                break ;
            }
        }
        if (allPermissionsGranted) {
            switch (requestCode) {
                case TEST_01_PHONE_CALL_INTENT:
                    test01PhoneCallIntentIfGranted(requestCode) ;
                    break;
                case TEST_01_PHONE_CALL_REFLECT:
                    test01PhoneCallReflectIfGranted(requestCode) ;
                    break;
                case TEST_01_PHONE_CALL_END:
                    test01PhoneCallEndIfGranted(requestCode) ;
                    break;
                case TEST_02_SEND_SMS:
                case TEST_02_SEND_SMS2:
                    test02SendSmsIfGranted(requestCode) ;
                    break;
                case TEST_02_SEND_SMS_REFLECT1:
                case TEST_02_SEND_SMS_REFLECT2:
                case TEST_02_SEND_SMS_REFLECT3:
                    test02SendSmsReflectIfGranted(requestCode);
                    break;
                case TEST_05_LOC_REQ:
                case TEST_05_LOC_GET:
                    test05LocationIfGranted(requestCode) ;
                    break;
                case TEST_06_RECORD_PHONE:
                case TEST_06_RECORD_PHONE2:
                case TEST_07_RECORD:
                case TEST_07_RECORD2:
                    test06RecordAudioIfGranted(requestCode) ;
                    break;
                case TEST_08_CAMERA:
                    test08CameraIfGranted(requestCode) ;
                    break;
                case TEST_08_CAMERA_RECORD:
                    test08camerarecord() ;
                    break;
                case TEST_09_READ_CONTACT:
                    test09ReadContactsIfGranted(requestCode) ;
                    break ;
                case TEST_09_READ_CALLLOG:
                    test09readcalllogIfGranted(requestCode) ;
                    break;
                case TEST_09_READ_SMS:
                case TEST_09_READ_MMS:
                    test09readSmsMmsIfGranted(requestCode) ;
                    break;
                case TEST_11_READ_CALENDAR:
                    test11readcalendarIfGranted(requestCode);
                    break ;
                default:
                    break;
            }
        } else {
            showOnceDenyToast(deniedPermission) ;
            mHandler.sendMessage(
                mHandler.obtainMessage(MSG_TEST_FINISH, requestCode, 700));
        }

    }

    void test01PhoneCallIntentIfGranted(int requestCode) {
        int ret = MSG_TEST_FINISH;
        test01PhoneCallIntent();
        mHandler.sendMessage(mHandler.obtainMessage(ret, requestCode, 700));
    }

    void test01PhoneCallReflectIfGranted(int requestCode) {
        int ret = MSG_TEST_FINISH;
        int prevSubId = INVALID_SUBSCRIPTION_ID;
        int newSubId = INVALID_SUBSCRIPTION_ID;

        prevSubId = getDefaultVoiceSubId();
        newSubId = setupSubIdIfValid();
        if (newSubId != INVALID_SUBSCRIPTION_ID) {
            setDefaultVoiceSubId(newSubId);
            ret = test01PhoneCallReflect();
            setDefaultVoiceSubId(prevSubId);
            setDefaultSubId(prevSubId);
        } else {
            ret = MSG_TEST_SIM_NOT_READY;
        }
        mHandler.sendMessage(mHandler.obtainMessage(ret, requestCode, 700));
    }

    void test01PhoneCallEndIfGranted(int requestCode) {
        int ret = MSG_TEST_FINISH;
        int prevSubId = INVALID_SUBSCRIPTION_ID;
        int newSubId = INVALID_SUBSCRIPTION_ID;

        prevSubId = getDefaultSubId();
        newSubId = setupSubIdIfValid();
        if (newSubId != INVALID_SUBSCRIPTION_ID) {
            ret = test01PhoneCallEnd(newSubId);
            setDefaultSubId(prevSubId);
        } else {
            ret = MSG_TEST_SIM_NOT_READY;
        }

        mHandler.sendMessage(mHandler.obtainMessage(ret, requestCode, 700));
    }

    void test02SendSmsIfGranted(int requestCode) {
        int ret = MSG_TEST_FINISH;
        int prevSubId = INVALID_SUBSCRIPTION_ID;
        int newSubId = INVALID_SUBSCRIPTION_ID;

        prevSubId = getDefaultSubId();
        newSubId = setupSubIdIfValid();
        if (newSubId != INVALID_SUBSCRIPTION_ID) {
            ret = test02SendSms(newSubId);
            setDefaultSubId(prevSubId);
        } else {
            ret = MSG_TEST_SIM_NOT_READY;
        }

        mHandler.sendMessage(mHandler.obtainMessage(ret, requestCode, 700));
    }

    void test02SendSmsReflectIfGranted(int requestCode) {
        int ret = MSG_TEST_FINISH;
        int prevSubId = INVALID_SUBSCRIPTION_ID;
        int newSubId = INVALID_SUBSCRIPTION_ID;

        prevSubId = getDefaultSubId();
        newSubId = setupSubIdIfValid();
        if (newSubId != INVALID_SUBSCRIPTION_ID) {
            switch (requestCode) {
                case TEST_02_SEND_SMS_REFLECT1:
                    ret = test02SendSmsReflect1(newSubId);
                    break ;
                case TEST_02_SEND_SMS_REFLECT2:
                    ret = test02SendSmsReflect2(newSubId);
                    break ;
                case TEST_02_SEND_SMS_REFLECT3:
                    ret = test02SendSmsReflect3(newSubId);
                    break ;
                default:
                    break ;
            }
            setDefaultSubId(prevSubId);
        } else {
            ret = MSG_TEST_SIM_NOT_READY;
        }

        mHandler.sendMessage(mHandler.obtainMessage(ret, requestCode, 700));
    }
    void test05LocationIfGranted(int requestCode) {
        int ret = MSG_TEST_FINISH;
        switch (requestCode) {
            case TEST_05_LOC_INTENT:
                test05RequestLocationIntent();
                break;
            case TEST_05_LOC_INTENT2:
                test05RequestLocationIntent2();
                break;
            case TEST_05_LOC_REQ:
                test05RequestLocation();
                break;
            case TEST_05_LOC_GET:
                test05GetLastLocation();
                break;
            default:
                break;
        }
        mHandler.sendMessage(mHandler.obtainMessage(ret, requestCode, 700));
    }

    void test06RecordAudioIfGranted(int requestCode) {
        int ret = MSG_TEST_FINISH;
        switch(requestCode) {
            case TEST_06_RECORD_PHONE:
                test06RecordPhone();
                break;
            case TEST_06_RECORD_PHONE2:
                test06RecordPhone2();
                break;
            case TEST_07_RECORD:
                test07RecordAudio();
                break;
            case TEST_07_RECORD2:
                test07RecordAudio2();
                break;
            default:
                break;
        }
        mHandler.sendMessage(mHandler.obtainMessage(ret, requestCode, 700));
    }
    void test08CameraIfGranted(int requestCode) {
        int ret = MSG_TEST_FINISH;
        test08camera();
        mHandler.sendMessage(mHandler.obtainMessage(ret, requestCode, 700));
    }

    void test09ReadContactsIfGranted(int requestCode) {
        int ret = MSG_TEST_FINISH;
        test09readcontacts();
        mHandler.sendMessage(mHandler.obtainMessage(ret, requestCode, 700));
    }

    void test09readcalllogIfGranted(int requestCode) {
        int ret = MSG_TEST_FINISH;
        test09readcalllog();
        mHandler.sendMessage(mHandler.obtainMessage(ret, requestCode, 700));
    }

    void test09readSmsMmsIfGranted(int requestCode) {
        int ret = MSG_TEST_FINISH;
        switch(requestCode) {
            case TEST_09_READ_SMS:
                test09readsms();
                break;
            case TEST_09_READ_MMS:
                test09readmms();
                break;
            default:
                break;
        }
        mHandler.sendMessage(mHandler.obtainMessage(ret, requestCode, 700));
    }

    void test11readcalendarIfGranted(int requestCode) {
        int ret = MSG_TEST_FINISH;
        test11readcalendar();
        mHandler.sendMessage(mHandler.obtainMessage(ret, requestCode, 700));
    }

}
