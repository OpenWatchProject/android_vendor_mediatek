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

package com.mediatek.engineermode.clkqualityat;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.AsyncResult;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.provider.Settings;
import android.support.v4.content.LocalBroadcastManager;
import android.text.Html;
import android.util.DisplayMetrics;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.RadioGroup.OnCheckedChangeListener;
import android.widget.TextView;
import android.widget.Toast;



import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;

import java.io.FileOutputStream;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

/**
 * GPS clock quality auto test Activity.
 */
public class ClkQualityAtActivity extends Activity implements OnItemClickListener,
        View.OnClickListener, OnCheckedChangeListener {
    private static final String PASSWORD = "";
    public static final String QUERY_SUPPORT_COMMAND = "AT+EPBSE=?";
    public static final String SAME_COMMAND = "+EPBSE:";
    public static final String QUERY_SUPPORT_COMMAND_CDMA = "AT+ECBAND=?";
    public static final String QUERY_CURRENT_COMMAND_CDMA = "AT+ECBANDCFG?";
    public static final String SET_COMMAND_CDMA = "AT+ECBANDCFG=";
    public static final String SAME_COMMAND_CDMA = "+ECBANDCFG:";
    public static final String ATC_SEND_ACTION = "com.mediatek.engineermode.clkqualityat.atc_send";
    public static final String ATC_EXTRA_CMD = "atc_send.cmd";
    public static final String ATC_EXTRA_MODEM_TYPE = "atc_send.modem";
    public static final String ATC_EXTRA_MSG_ID = "atc_send.msgId";
    protected static final String TAG = "ClkQualityAt/MainActivity";
    static final String PREF_FILE = "prefs_location";
    static final String KEY_ATC_SEND_DONE = "atc_send";
    static final String INVALID_DATA = "invalid";
    private static final int DIALOG_TEST_ITEMS = 1;
    private static final int DIALOG_CURVE = 2;
    private static final int DIALOG_REENTER = 3;
    private static final int DIALOG_OMIT_WARNING = 5;
    private static final int DIALOG_EXIT = 6;
    private static final int DIALOG_CHECK_PASSWORD = 7;
    private static final int DEFAULT_COLL_TIME = 60;
    private static final int DEFAULT_HEATING_TIME = 12;
    private static final String KEY_ITEM = "clk_item";
    private static final String KEY_MODE = "clk_mode";
    private static final String KEY_SELECTED_ALL = "clk_selectedAll";
    private static final String NEW_LINE = System.getProperty("line.separator");
    private static final CharSequence TESTING =
            Html.fromHtml("<font color='#FFFF00'>Testing</font>");
    private static final DateFormat DATE_FORMAT = new SimpleDateFormat("yyyyMMddhhmmss");
    private static boolean sTestCancelled = false;
    private static boolean sIsForeground = false;
    public static HashMap<String, String> sItemsToCheck;
    private final BroadcastReceiver atcReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            // TODO Auto-generated method stub
            if (ATC_SEND_ACTION.equals(intent.getAction())) {
                String param = intent.getStringExtra(ATC_EXTRA_CMD);
                Elog.d(TAG, "atcReceiver receive ATC_SEND_ACTION and param is " + param);
                String[] cmd = new String[2];
                String[] cmdCdma = new String[3];
                cmdCdma[0] = cmd[0] = param;
                cmdCdma[1] = cmd[1] = "";
                cmdCdma[2] = "DESTRILD:C2K";
                String[] cmd_s = ModemCategory.getCdmaCmdArr(cmdCdma);
                Elog.d(TAG, "query1XTimeStatus: " + cmd_s[0] + ",cmd_s.length = " + cmd_s.length);

                boolean isCDMA = intent.getBooleanExtra(ATC_EXTRA_MODEM_TYPE, false);
                int msgId = intent.getIntExtra(ATC_EXTRA_MSG_ID, -1);

                if ( msgId > 0) {
                    EmUtils.invokeOemRilRequestStringsEm(isCDMA, isCDMA ? cmd_s:cmd,
                            ClkQualityAtActivity.this.getAtCmdHandler().obtainMessage(msgId));
                } else {
                    BandTest.sAtcDone = false;
                }
            }
        }
    };
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
    private boolean mSelectedAll = false;
    private int mTestModeValue = TestMode.MODE_NONE;
    // UI components
    private Button mBtnTestItem;
    private Button mBtnStart;
    private Button mBtnStop;
    private EditText mEditHeatTime;
    private EditText mEditCoolTime;
    private RadioGroup mRGTestMode;
    private RadioButton mBtnSignalMode;
    private RadioButton mBtnCWMode;
    private View mResultLayout;
    private TextView mSetFail;
    private ListView mResultList;
    private MyAdapter mAdapter;
    private boolean mIsRunning = false;
    private boolean mRequestLocationFix = false;
    private List<TestItem> mTestItems;
    private List<TestItem> mSelectedTestItems;
    private List<TestItem> mGenTestItems;
    private List<String> mItemsToOmit = new ArrayList<String>();
    private FunctionTask mTask;
    private HashMap<TestItem, TestResult> mTestResults;
    private LocationManager mLocationManager;
    private boolean mInitGPSState;
    private String mOmitWarning = new String();
    private FileOutputStream mOutputData = null;
    private ClkQualityAtActivity.ClockType mClockType = ClockType.TCXO;
    private boolean isHandlerAlive = false;
    private long[] bandSupported = new long[TestItem.INDEX_BAND_MAX];
    private final Handler responseHandler = new Handler() {
        public void handleMessage(final Message msg) {
            Elog.v(TAG, "responseHander receive a message: " + msg.what);
            if (!isHandlerAlive) {
                Elog.v(TAG, "isHandlerAlive is false, exit handleMessage");
                return;
            }
            AsyncResult asyncResult = (AsyncResult) msg.obj;
            switch (AtcMsg.getAtcMsg(msg.what)) {
                case SUPPORT_QUERY:
                    if (asyncResult.exception == null) {

                        System.arraycopy(Util.getSupportedBand((String[]) asyncResult.result), 0,
                                bandSupported, 0, TestItem.INDEX_BAND_MAX - 1);
                        // query cdma supported modes
                        String[] cmd = new String[3];
                        cmd[0] = QUERY_CURRENT_COMMAND_CDMA;
                        cmd[1] = SAME_COMMAND_CDMA;
                        cmd[2] = "DESTRILD:C2K";
                        String[] cmd_s = ModemCategory.getCdmaCmdArr(cmd);
                        Elog.d(TAG, "query at cmd: " + cmd_s[0] +
                               ",cmd_s.length = " + cmd_s.length);
                        EmUtils.invokeOemRilRequestStringsEm(true,cmd_s,
                                responseHandler.obtainMessage(AtcMsg.SUPPORT_QUERY_CDMA.getValue()));
                    } else {
                        Elog.w(TAG, "ATC_SUPPORT_QUERY: responseHandler get have exception!");
                        Toast.makeText(ClkQualityAtActivity.this,
                                "Query Modem type failed",Toast.LENGTH_SHORT).show();
                    }
                    break;
                case SUPPORT_QUERY_CDMA:
                    if (asyncResult.exception == null) {
                        bandSupported[TestItem.INDEX_CDMA_BAND] =
                                Util.getSupportedBandCdma((String[]) asyncResult.result);
                    } else {
                        Elog.w(TAG, "ATC_SUPPORT_CDMA_QUERY: responseHandler get have exception!");
                    }
                    mTestItems = TestItem.getTestItems(ClkQualityAtActivity.this, bandSupported);
                    restoreSettings();
                    Elog.v(TAG, "query support bands finished.");
                    break;
                case FLIGHT_MODE:
                case FLIGHT_MODE_CDMA:
                case START_CDMA:
                case START_FDDTDD:
                case START_GSM:
                case START_LTE:
                case PAUSE_CDMA:
                case PAUSE_FDDTDD:
                case PAUSE_GSM:
                case PAUSE_LTE:
                case REBOOT_LTE:
                case REBOOT_CDMA: {
                    if (asyncResult.exception == null) {
                        BandTest.sAtcDone = true;
                        Elog.v(TAG, "Succeed to execute atc for "
                                + AtcMsg.getAtcMsg(msg.what).getName());
                    } else {
                        Elog.i(TAG, "Fail to execute atc for "
                                + AtcMsg.getAtcMsg(msg.what).getName());
                        BandTest.sAtcDone = false;
                        Toast.makeText(ClkQualityAtActivity.this, "Fail to execute AT Command:" +
                                        AtcMsg.getAtcMsg(msg.what).getName(),
                                Toast.LENGTH_SHORT).show();
                    }
                    break;
                }
                default:
                    break;
            }
        }
    };

    /**
     * Check if DesenseAt is running on foreground.
     *
     * @return true if DesenseAt is running on foreground
     */
    public static boolean isForeGround() {
        Elog.d(TAG, "isForeGround:" + sIsForeground);
        return sIsForeground;
    }

    public synchronized static boolean getCancelled() {
        return sTestCancelled;
    }

    synchronized static void setCancelled(boolean cancelled) {
        sTestCancelled = cancelled;
    }

    public Handler getAtCmdHandler() {
        return responseHandler;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.clk_quality_at);
        mSelectedTestItems = new ArrayList<TestItem>();
        mTestResults = new HashMap<TestItem, TestResult>();
        mGenTestItems = new ArrayList<TestItem>();

        initComponents();
        updateListView();
        enableButtons(true);
        initItemToCheck(this);
        initResolution();
        mLocationManager = (LocationManager) getSystemService(Context.LOCATION_SERVICE);
        if (EmUtils.checkLocationProxyAppPermission(this, false)) {
            if (!ctrlLocationFix(true)) {
                Toast.makeText(this, getString(R.string.start_fix_error_message),
                        Toast.LENGTH_LONG).show();
                finish();
            }
        }
        isHandlerAlive = true;

        querySupportMode();
        checkPassWord();
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
    protected void onStart() {
        // TODO Auto-generated method stub
        super.onStart();
        sIsForeground = true;
        Elog.i(TAG, "sIsForeground = true");
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (EmUtils.checkLocationProxyAppPermission(this, true) && !mRequestLocationFix) {
            showDialog(DIALOG_REENTER);
        }
    }

    @Override
    protected void onStop() {
        // TODO Auto-generated method stub
        sIsForeground = false;
        Elog.i(TAG, "sIsForeground = false");
        super.onStop();

    }

    @Override
    protected void onPause() {
        // TODO Auto-generated method stub
        super.onPause();
        removeDialog(DIALOG_CURVE);
    }

    @Override
    protected void onDestroy() {
        // TODO Auto-generated method stub
        ctrlLocationFix(false);
        saveSettings();
        isHandlerAlive = false;
        super.onDestroy();
    }

    private void checkPassWord() {
        if (FeatureSupport.isUserLoad() && (PASSWORD != null) && (!PASSWORD.isEmpty())) {
            showDialog(DIALOG_CHECK_PASSWORD);
        }

    }
    /**
     * Query Modem supported band modes.
     */
    private void querySupportMode() {
            final String[] queryCommon = {QUERY_SUPPORT_COMMAND, SAME_COMMAND};
            Elog.v(TAG, "sendAtCommand: " + queryCommon[0] + ", " + queryCommon[1]);
            EmUtils.invokeOemRilRequestStringsEm(false,queryCommon, responseHandler.obtainMessage
                    (AtcMsg.SUPPORT_QUERY.getValue()));
    }

    private void initResolution() {
        DisplayMetrics metric = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(metric);

        int width = metric.widthPixels;
        int height = metric.heightPixels;
        Util.setResolution(width, height);
    }

    public static void initItemToCheck(Context context) {
        sItemsToCheck = new HashMap<String, String>();
        sItemsToCheck.put(new Display3DTest().toString(),
                context.getResources().getString(R.string.desense_at_no_3d_display_apk));
    }

    @SuppressWarnings("deprecation")
    @Override
    public void onClick(View view) {
        if (view == mBtnTestItem) {
            if (mTestItems == null) {
                Toast.makeText(this, "Query Modem type failed,please backto the test item or " +
                        "reboot phone ",Toast.LENGTH_SHORT).show();
                return;
            } else {
                showDialog(DIALOG_TEST_ITEMS);
            }
        } else if (view.getId() == R.id.clk_quality_at_start) {
            Elog.d(TAG, "clk_quality_at_start click: " + mTestModeValue);
            if (mSelectedTestItems.size() <= 0) {
                Toast.makeText(this, R.string.desense_at_test_item_warning,
                        Toast.LENGTH_LONG).show();
                return;
            }
            if (mTestModeValue == TestMode.MODE_NONE) {
                Toast.makeText(this, R.string.desense_at_test_mode_warning,
                        Toast.LENGTH_LONG).show();
                return;
            }
            showReminderItems();
        } else if (view.getId() == R.id.clk_quality_at_stop) {
            // stop test
            Elog.d(TAG, "cancel click");
            Toast.makeText(this, R.string.desense_at_test_stop_warning,
                    Toast.LENGTH_LONG).show();
            if (mTestModeValue != TestMode.MODE_NONE) {
                Elog.d(TAG, "task cancel");
                setCancelled(true);
            }
        }
    }

    private void startTest() {
        // clear current result list
        mTestResults.clear();
        mAdapter.notifyDataSetChanged();
        mSetFail.setVisibility(View.GONE);
        mResultLayout.setVisibility(View.GONE);
        // start test
        mTask = new FunctionTask(this);
        mTask.execute();
        enableButtons(false);
    }

    @SuppressWarnings("deprecation")
    private void showReminderItems() {

        mItemsToOmit.clear();
        StringBuilder strWarning = new StringBuilder("------ Following items will be omitted:\r\n");
        int k = 0;
        boolean hasOmitItem = false;
        for (TestItem testitem : mSelectedTestItems) {
            String strName = testitem.toString();
            String strToast = sItemsToCheck.get(strName);
            Elog.d(TAG, "strName: " + strName);
            Elog.d(TAG, "strToast: " + strToast);
            if ((strToast != null)
                    && (!testitem.doApiTest())) {

                mItemsToOmit.add(strName);
                strWarning.append(++k).append("  ").
                        append(strName).append(": ").append(strToast).append("\r\n");
                hasOmitItem = true;
                Elog.d(TAG, "strWarning: " + strWarning);
            }
        }

        if (hasOmitItem) {
            mOmitWarning = strWarning.toString();
            removeDialog(DIALOG_OMIT_WARNING);
            showDialog(DIALOG_OMIT_WARNING);
        } else {
            startTest();
        }

    }

    @SuppressWarnings("deprecation")
    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        Bundle bundle = new Bundle();
        bundle.putInt("position", position);
        removeDialog(DIALOG_CURVE);
        showDialog(DIALOG_CURVE, bundle);
    }

    @Override
    protected Dialog onCreateDialog(int id, Bundle bundle) {
        switch (id) {
            case DIALOG_TEST_ITEMS:
                return createTestItemDialog();
            case DIALOG_CURVE:
                return createCurveDialog(bundle);
            case DIALOG_OMIT_WARNING:
                return new AlertDialog.Builder(this)
                        .setTitle(R.string.desense_at_warning_title)
                        .setMessage(mOmitWarning)
                        .setPositiveButton(android.R.string.yes,
                                new DialogInterface.OnClickListener() {
                                    @SuppressWarnings("deprecation")
                                    public void onClick(DialogInterface dialog, int which) {
                                        startTest();

                                    }
                                })
                        .setNegativeButton(android.R.string.no,
                                new DialogInterface.OnClickListener() {
                                    @SuppressWarnings("deprecation")
                                    public void onClick(DialogInterface dialog, int which) {
                                        return;

                                    }
                                }).create();
            case DIALOG_EXIT: {
                return new AlertDialog.Builder(this)
                        .setTitle(R.string.desense_at_warning_title)
                        .setMessage(R.string.desense_at_exit_warning)
                        .setPositiveButton(android.R.string.yes,
                                new DialogInterface.OnClickListener() {
                                    @SuppressWarnings("deprecation")
                                    public void onClick(DialogInterface dialog, int which) {
                                        mTask.cancel(true);

                                    }
                                })
                        .setNegativeButton(android.R.string.no,
                                new DialogInterface.OnClickListener() {
                                    @SuppressWarnings("deprecation")
                                    public void onClick(DialogInterface dialog, int which) {
                                        return;

                                    }
                                }).create();
            }
            case DIALOG_CHECK_PASSWORD:
                return createPasswordDialog();
            case DIALOG_REENTER:
                return new AlertDialog.Builder(this)
                        .setTitle(R.string.clk_quality_at)
                        .setMessage(R.string.lbs_permission_granted_take_effect_msg)
                        .setCancelable(false)
                        .setPositiveButton(R.string.dialog_ok,
                                new DialogInterface.OnClickListener() {
                                    public void onClick(DialogInterface dialog, int which) {
                                        finish();
                                    }
                        }).create();
            default:
                break;
        }
        return super.onCreateDialog(id);
    }


    @SuppressWarnings("deprecation")
    @Override
    public void onBackPressed() {
        // TODO Auto-generated method stub
        if (mIsRunning) {
            showDialog(DIALOG_EXIT);
        }
        super.onBackPressed();
    }

    private void initComponents() {
        mBtnTestItem = (Button) findViewById(R.id.clk_quality_at_test_items);
        mBtnTestItem.setOnClickListener(this);
        mEditHeatTime = (EditText) findViewById(R.id.heating_time_edit);
        mEditHeatTime.setText(String.valueOf(DEFAULT_HEATING_TIME));
        mEditCoolTime = (EditText) findViewById(R.id.cool_time_edit);
        mEditCoolTime.setText(String.valueOf(DEFAULT_COLL_TIME));
        mBtnStart = (Button) findViewById(R.id.clk_quality_at_start);
        mBtnStop = (Button) findViewById(R.id.clk_quality_at_stop);
        mRGTestMode = (RadioGroup) findViewById(R.id.clk_quality_at_test_modes);
        mRGTestMode.setOnCheckedChangeListener(this);
        mBtnSignalMode = (RadioButton) findViewById(R.id.clk_quality_at_signal_mode);
        mBtnCWMode = (RadioButton) findViewById(R.id.clk_quality_at_cw_mode);
        if (mTestModeValue == TestMode.MODE_CW) {
            mBtnCWMode.setChecked(true);
        } else if (mTestModeValue == TestMode.MODE_SIGNAL) {
            mBtnSignalMode.setChecked(true);
        }
        RadioGroup rgClockType = (RadioGroup) findViewById(R.id.clk_quality_at_clock_type);
        rgClockType.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
            public void onCheckedChanged(RadioGroup group, int checkedId) {
                if (group != rgClockType)
                    return;
                if (checkedId == R.id.clk_quality_at_clock_txco) {
                    mClockType = ClockType.TCXO;
                    // set wifi tx item unchecked when clock type is TCXO
                    int idxWifi = getTestItemIndex(mSelectedTestItems, WifiTxTest.NAME);
                    if (idxWifi > 0) {
                        mSelectedTestItems.remove(idxWifi);
                        saveSettings();
                        Elog.d(TAG, "mSelectedTestItems remove wifi tx item in TCXO, idxWifi:"
                                + idxWifi);
                    }
                } else if (checkedId == R.id.clk_quality_at_clock_co_tms) {
                    mClockType = ClockType.Co_TMS;
                }
                Elog.d(TAG, "select clock type : " + mClockType.toString());
            }
        });
        mAdapter = new MyAdapter(this);
        mResultList = (ListView) findViewById(R.id.clk_quality_at_test_result);
        mResultList.setAdapter(mAdapter);
        mResultList.setOnItemClickListener(this);
        mSetFail = (TextView) findViewById(R.id.set_result);
        mSetFail.setVisibility(View.GONE);
        mResultLayout = findViewById(R.id.clk_quality_at_test_result_layout);
        mResultLayout.setVisibility(View.GONE);
    }

    // read settings from preference
    private void restoreSettings() {
        // selected test items
        SharedPreferences pref = getSharedPreferences(PREF_FILE, MODE_PRIVATE);
        String[] items = pref.getString(KEY_ITEM, "").split(",");
        mSelectedTestItems.clear();
        for (int i = 0; i < items.length; i++) {
            for (TestItem item : mTestItems) {
                if (items[i].equals(item.toString())) {
                    mSelectedTestItems.add(item);
                }
            }
        }
        mSelectedAll = pref.getBoolean(KEY_SELECTED_ALL, false);
        // selected test mode
        mTestModeValue = pref.getInt(KEY_MODE, TestMode.MODE_NONE);
    }

    // write settings to preference
    private void saveSettings() {
        // selected test items
        SharedPreferences.Editor pref = getSharedPreferences(PREF_FILE, MODE_PRIVATE).edit();
        String str = "";
        for (TestItem item : mSelectedTestItems) {
            str += item.toString() + ",";
        }
        pref.putString(KEY_ITEM, str);
        pref.putInt(KEY_MODE, mTestModeValue);
        pref.putBoolean(KEY_SELECTED_ALL, mSelectedAll);
        pref.commit();
    }

    // Show check box list with all test items
    private Dialog createTestItemDialog() {
        final String[] labels = new String[mTestItems.size() + 1];
        final boolean[] values = new boolean[mTestItems.size() + 1];
        labels[0] = "Select All";
        values[0] = mSelectedAll;
        for (int i = 1; i < labels.length; i++) {
            labels[i] = mTestItems.get(i - 1).toString();
            values[i] = mSelectedTestItems.contains(mTestItems.get(i - 1));
        }
        int idxWifi = getTestItemIndex(mTestItems, WifiTxTest.NAME) + 1;
        if (idxWifi > 0 && mClockType.equals(ClockType.TCXO)) {
            values[idxWifi] = false;
            Elog.d(TAG, "set wifi tx item unchecked in clock type TCXO");
        }
        AlertDialog dialog = new AlertDialog.Builder(this)
                .setTitle("Test Items")
                .setMultiChoiceItems(labels, values,
                        new DialogInterface.OnMultiChoiceClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which, boolean
                                    isChecked) {
                                ListView lv = ((AlertDialog) dialog).getListView();
                                if (which == 0) {
                                    values[which] = isChecked;
                                    Elog.d(TAG, "values[0]: " + values[which]);
                                    for (int i = 1; i < labels.length; i++) {
                                        lv.setItemChecked(i, isChecked);
                                        values[i] = isChecked;
                                    }
                                } else {
                                    if (isChecked) {
                                        if (!checkTestItemValid(which - 1)) {
                                            Elog.d(TAG, "item#" + which + "test valid failed");
                                            lv.setItemChecked(which, false);
                                            values[which] = false;
                                            return;
                                        }
                                    } else {
                                        lv.setItemChecked(0, isChecked);
                                        values[0] = isChecked;
                                    }
                                    values[which] = isChecked;
                                }
                                // set wifi tx item unchecked when clock type is TCXO
                                if (idxWifi > 0 && values[idxWifi] && mClockType.equals(ClockType
                                        .TCXO)) {
                                    values[idxWifi] = false;
                                    lv.setItemChecked(idxWifi, false);
                                    Toast.makeText(ClkQualityAtActivity.this,
                                            "wifi tx is invalid in TCXO clock type",
                                            Toast.LENGTH_SHORT)
                                            .show();
                                    Elog.d(TAG, "set wifi tx item unchecked in clock type TCXO");
                                }
                            }
                        })
                .setPositiveButton(android.R.string.ok, new OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        mSelectedTestItems.clear();
                        for (int i = 1; i < labels.length; i++) {
                            if (values[i]) {
                                mSelectedTestItems.add(mTestItems.get(i - 1));
                            }
                        }
                        mSelectedAll = values[0];
                        saveSettings();
                        dialog.dismiss();
                    }
                })
                .setNegativeButton(android.R.string.cancel, null)
                .create();
        dialog.setOnDismissListener(new DialogInterface.OnDismissListener() {
            public void onDismiss(DialogInterface dialog) {
                removeDialog(DIALOG_TEST_ITEMS);
            }
        });
        return dialog;
    }

    private int getTestItemIndex(List<TestItem> items, String name) {
        int idx = 0;
        for (TestItem item : items) {
            if (item.toString().equals(name)) {
                return idx;
            }
            idx++;
        }
        return -1;
    }

    private boolean checkTestItemValid(int itemIndex) {
        TestItem testItem = mTestItems.get(itemIndex);
        String strToast = sItemsToCheck.get(testItem.toString());
        if ((strToast != null) && (!testItem.doApiTest())) {
            Toast.makeText(this, strToast, Toast.LENGTH_LONG).show();
            Elog.d(TAG, "show toast: " + strToast);
            return false;
        }
        return true;
    }

    // show curve for clicked test item
    @SuppressLint("InflateParams")
    private Dialog createCurveDialog(Bundle bundle) {
        int position = bundle.getInt("position", 0);
        if ((mAdapter == null) || (mAdapter.getCount() <= position)) {
            return null;
        }
        View view = getLayoutInflater().inflate(R.layout.clk_quality_at_curve, null);
        TextView title = (TextView) view.findViewById(R.id.clk_quality_at_curve_title);
        CurveView curveDriftRate = (CurveView) view.findViewById(
                R.id.clk_quality_at_curve_clkdriftrate);
        CurveView curveCompClkCriftRate = (CurveView) view.findViewById(
                R.id.clk_quality_at_curve_compclkdriftrate);

        TestItem item = mAdapter.getItem(position);
        TestResult r = mTestResults.get(item);
        if (r == null) {
            return null;
        }
        title.setText(item.toString());
        curveDriftRate.setDataList(r.getClkdriftrate());
        curveCompClkCriftRate.setDataList(r.getCompClkDriftRate());
        AlertDialog dialog = new AlertDialog.Builder(this).setView(view).create();
        return dialog;
    }

    private Dialog createPasswordDialog() {
        View view = getLayoutInflater().inflate(R.layout.clk_quality_at_password, null);

        AlertDialog dialog = new AlertDialog.Builder(this)
        .setTitle(R.string.clk_quality_at_input_password_title)
        .setView(view).setCancelable(false).create();
        Button btnOK = (Button) view.findViewById(R.id.btn_password_ok);
        btnOK.setOnClickListener(new Button.OnClickListener() {
            @Override
            public void onClick(View v) {
                EditText etPassword = (EditText) view.findViewById(R.id.et_password);
                if ((etPassword.getText() == null)
                        || (etPassword.getText().toString().isEmpty())) {
                    Toast.makeText(ClkQualityAtActivity.this,
                            R.string.clk_quality_at_input_password_empty,
                            Toast.LENGTH_LONG)
                            .show();
                } else {
                    String strPassword = etPassword.getText().toString();
                    if (!PASSWORD.equals(etPassword.getText().toString())) {
                        Toast.makeText(ClkQualityAtActivity.this,
                                R.string.clk_quality_at_input_password_error,
                                Toast.LENGTH_LONG).show();
                    } else {
                        removeDialog(DIALOG_CHECK_PASSWORD);
                    }
                }
            }
        });
        Button btnCancel = (Button) view.findViewById(R.id.btn_password_cancel);
        btnCancel.setOnClickListener(new Button.OnClickListener() {
            @Override
            public void onClick(View v) {
                finish();
            }
        });
        return dialog;
    }

    private void updateListView() {
        mAdapter.clear();
        mAdapter.addAll(mGenTestItems);
        mAdapter.notifyDataSetChanged();
    }

    private void enableButtons(boolean enable) {
        mBtnStart.setEnabled(enable);
        mBtnStop.setEnabled(!enable);
        mBtnTestItem.setEnabled(enable);
    }


    /**
     * @return if user cancel the test
     */
    public boolean isCancelled() {
        boolean ret = mTask.isCancelled();
        Elog.d(TAG, "isCancelled: " + ret);
        return ret;
    }

    private void genRealTestItems() {
        mGenTestItems.clear();
        for (TestItem testitem : mSelectedTestItems) {
            if (mItemsToOmit.contains(testitem.toString())) {
                continue;
            }
            mGenTestItems.add(testitem);
        }

        int size = mGenTestItems.size();
        for (int k = 0; k < size; k++) {
            Elog.d(TAG, "TestItem = " + mGenTestItems.get(k).toString());
            Elog.d(TAG, "TestItem index = " + (k + 1));
            mGenTestItems.get(k).setItemIndex(k + 1);
        }
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        // TODO Auto-generated method stub
        switch (keyCode) {
            case KeyEvent.KEYCODE_VOLUME_UP: {
                if (mTestModeValue != TestMode.MODE_NONE) {
                    Elog.d(TAG, "task cancel");
                    setCancelled(true);
                }
                return true;
            }
            default:
                break;
        }
        return super.onKeyDown(keyCode, event);
    }

    @Override
    public void onCheckedChanged(RadioGroup group, int checkedId) {
        // TODO Auto-generated method stub
        if (checkedId == R.id.clk_quality_at_cw_mode) {
            mTestModeValue = TestMode.MODE_CW;
        } else if (checkedId == R.id.clk_quality_at_signal_mode) {
            mTestModeValue = TestMode.MODE_SIGNAL;
        }
    }



    public enum ClockType {
        TCXO("tcxo"),
        Co_TMS("co_tms");
        private final String type;

        ClockType(String type) {
            this.type = type;
        }

        public String getType() {
            return type;
        }

        @Override
        public String toString() {
            return "ClockType: " + this.type;
        }

    }

    public enum AtcMsg {
        ATCMSG_NONE(0, "xxxx"),
        SUPPORT_QUERY(100, "query lte"),
        SUPPORT_QUERY_CDMA(101, "query cdma"),
        FLIGHT_MODE(102, "flight mode"),
        FLIGHT_MODE_CDMA(103, "flight mode cdma"),
        START_CDMA(104, "start cdma"),
        START_FDDTDD(105, "start FddTdd"),
        START_GSM(106, "start gsm"),
        START_LTE(107, "start lte"),
        PAUSE_CDMA(108, "pause cdma"),
        PAUSE_FDDTDD(109, "pause FddTdd"),
        PAUSE_GSM(110, "pause gsm"),
        PAUSE_LTE(111, "pause lte"),
        REBOOT_LTE(112, "reboot lte"),
        REBOOT_CDMA(113, "reboot cdma");

        public final int value;
        private final String name;

        AtcMsg(int value, String name) {
            this.value = value;
            this.name = name;
        }

        public static AtcMsg getAtcMsg(int id) {
            for (AtcMsg msg : AtcMsg.values()) {
                if (id == msg.getValue()) {
                    return msg;
                }
            }
            return ATCMSG_NONE;
        }

        public final int getValue() {
            return this.value;
        }

        public final String getName() {
            return this.name;
        }
    }

    /**
     * Adapter for ListView.
     */
    private class MyAdapter extends ArrayAdapter<TestItem> {
        /**
         * Default constructor.
         *
         * @param activity the context
         */
        public MyAdapter(Context activity) {
            super(activity, 0);
        }

        @SuppressLint("InflateParams")
        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            ViewHolder holder;
            LayoutInflater inflater = ClkQualityAtActivity.this.getLayoutInflater();
            if (convertView == null) {
                convertView = inflater.inflate(R.layout.clk_quality_at_result_entry, null);
                holder = new ViewHolder();
                holder.label = (TextView) convertView.findViewById(R.id.column1);
                holder.result = (TextView) convertView.findViewById(R.id.column2);
                holder.clkdriftrtate = (TextView) convertView.findViewById(R.id.column3);
                holder.comclkdriftrtate = (TextView) convertView.findViewById(R.id.column4);
                convertView.setTag(holder);
            } else {
                holder = (ViewHolder) convertView.getTag();
            }
            TestItem testItem = getItem(position);
            holder.label.setText(testItem.toString());

            TestResult r = mTestResults.get(testItem);
            if (r == null) {
                holder.result.setText("-");
            } else {
                switch (r.result) {
                    case TestResult.NONE:
                        holder.result.setText(TESTING);
                        break;
                    case TestResult.PASS:
                        holder.result.setText(Util.PASS);
                        break;
                    case TestResult.VENIAL:
                        holder.result.setText(Util.VENIAL);
                        break;
                    case TestResult.SERIOUS:
                        holder.result.setText(Util.SERIOUS);
                        break;
                    case TestResult.FAIL:
                        holder.result.setText(Util.FAIL);
                        break;
                    case TestResult.CONNECTION_FAIL:
                        holder.result.setText(Util.CONN_FAIL);
                        break;
                    case TestResult.MODEM_FAIL:
                        holder.result.setText(Util.MODEM_FAIL);
                        break;
                    default:
                        break;
                }
            }
            // show values

            if (r != null && r.result != TestResult.NONE) {
                float clkdriftrate = r.getMaxClkDriftRate();
                if (clkdriftrate != -1) {
                    holder.clkdriftrtate.setText(String.valueOf(clkdriftrate));
                } else {
                    holder.clkdriftrtate.setText("-");
                }
                float comclkdriftrtate = r.getMaxCompclkdriftrate();
                if (comclkdriftrtate != -1) {
                    holder.comclkdriftrtate.setText(String.valueOf(comclkdriftrtate));
                } else {
                    holder.comclkdriftrtate.setText("-");
                }
            } else {
                holder.clkdriftrtate.setText("-");
                holder.comclkdriftrtate.setText("-");
                holder.comclkdriftrtate.setText("-");
            }

            return convertView;
        }

        /**
         * ViewHolder.
         */
        private class ViewHolder {
            public TextView label;
            public TextView result;
            public TextView clkdriftrtate;
            public TextView comclkdriftrtate;
        }
    }

    /**
     * Task to run test items.
     */
    public class FunctionTask extends AsyncTask<Void, Void, Boolean> {
        private Context context;
        private TestCondition mTestCondition = null;
        public FunctionTask(Context context) {
            this.context = context;
        }

        @Override
        protected void onPreExecute() {
            // TODO Auto-generated method stub
            super.onPreExecute();
            mIsRunning = true;
            genRealTestItems();
            updateListView();
            ClkQualityAtActivity.this.getWindow().addFlags(
                    WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
            Elog.d(TAG, "add FLAG_KEEP_SCREEN_ON");
            mTestCondition = new TestCondition();
            mTestCondition.init(ClkQualityAtActivity.this);
        }

        @Override
        protected Boolean doInBackground(Void... params) {
            mTestCondition.setCondition(ClkQualityAtActivity.this);
            String strHeatingTimes = mEditHeatTime.getText().toString();
            String strCoolTimes = mEditCoolTime.getText().toString();

            TestMode testMode = new TestMode(Integer.valueOf(strHeatingTimes), Integer.valueOf(strCoolTimes), mTestModeValue);
            Elog.d(TAG, "checkReadyBit");
            if (!testMode.checkReadyBit(mClockType)) {
                runOnUiThread(new Runnable() {
                    public void run() {
                        if (testMode.isClockTypeCorrect())
                            mSetFail.setText(context.getString(R.string.clk_quality_at_check_bit_fail));
                        else
                            mSetFail.setText(context.getString(R.string.clk_quality_at_clock_type_wrong));
                        mSetFail.setVisibility(View.VISIBLE);
                    }
                });
                mTestCondition.resetCondition(ClkQualityAtActivity.this);
                return true;
            } else {
                runOnUiThread(new Runnable() {
                    public void run() {
                        mResultLayout.setVisibility(View.VISIBLE);
                    }
                });
            }
            testMode.setmTriggerSource(0, null);
            int size = mGenTestItems.size();
            Elog.d(TAG, "size = " + size);
            if (size == 0) {
                mTestCondition.resetCondition(ClkQualityAtActivity.this);
                return true;
            }

            IntentFilter atcFilter = new IntentFilter();
            atcFilter.addAction(ATC_SEND_ACTION);
            LocalBroadcastManager.getInstance(ClkQualityAtActivity.this).registerReceiver(
                    atcReceiver, atcFilter);

            setCancelled(false);
            Elog.d(TAG, "start test");
            Util.flightMode(ClkQualityAtActivity.this, true);
            for (int k = 0; k < size; k++) {
                TestItem i = mGenTestItems.get(k);
                if (getCancelled()) {
                    Elog.d(TAG, "cancel and break");
                    break;
                }
                mTestResults.put(i, new TestResult());
                runOnUiThread(new Runnable() {
                    public void run() {
                        mAdapter.notifyDataSetChanged();
                    }
                });
                // run the test item
                Elog.d(TAG, "run " + i.toString());
                i.setTestMode(testMode);
                TestResult r = i.run();
                mTestResults.put(i, r);
                Elog.d(TAG, "TestResult = " + r);
                final int index = k;
                runOnUiThread(new Runnable() {
                    public void run() {
                        mResultList.requestFocusFromTouch();
                        mResultList.setSelection(index);
                        mAdapter.notifyDataSetChanged();
                    }
                });
                if (!BandTest.class.isAssignableFrom(i.getClass()))
                    continue;
            }
            // reboot modem
            Util.flightMode(ClkQualityAtActivity.this, false);
            Util.sleep(1000);
            LocalBroadcastManager.getInstance(ClkQualityAtActivity.this).unregisterReceiver(
                    atcReceiver);
            mTestCondition.resetCondition(ClkQualityAtActivity.this);
            return true;
        }

        @Override
        protected void onPostExecute(Boolean result) {
            enableButtons(true);
            mIsRunning = false;
            ClkQualityAtActivity.this.getWindow().clearFlags(
                    WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
            Util.notifyFinish(ClkQualityAtActivity.this);
        }

        @Override
        protected void onCancelled(Boolean result) {
            enableButtons(true);
        }
    }
}