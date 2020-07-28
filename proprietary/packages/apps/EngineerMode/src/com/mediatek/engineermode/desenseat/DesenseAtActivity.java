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

package com.mediatek.engineermode.desenseat;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
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
import android.util.DisplayMetrics;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.LinearLayout.LayoutParams;
import android.widget.ListView;
import android.widget.RadioButton;
import android.widget.Spinner;
import android.widget.TableLayout;
import android.widget.TableRow;
import android.widget.TextView;
import android.widget.Toast;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.Date;
import java.util.HashMap;
import java.util.List;

/**
 * GPS de-sense auto test Activity.
 */
public class DesenseAtActivity extends Activity implements OnItemClickListener,
        View.OnClickListener, CheckBox.OnCheckedChangeListener {
    private static final String QUERY_SUPPORT_COMMAND = "AT+EPBSE=?";
    static final String SAME_COMMAND = "+EPBSE:";
    private static final String QUERY_SUPPORT_COMMAND_CDMA = "AT+ECBAND=?";
    static final String QUERY_CURRENT_COMMAND_CDMA = "AT+ECBANDCFG?";
    private static final String SET_COMMAND_CDMA = "AT+ECBANDCFG=";
    static final String SAME_COMMAND_CDMA = "+ECBANDCFG:";
    static final int SUPPORT_QUERY = 0;
    static final int SUPPORT_QUERY_CDMA = 1;
    static final int MAX_SV_COUNT = 5;
    private static final int BAND_NAME_INDEX = 0;
    private static final int BAND_VALUE_INDEX = 1;
    private static final int CHANNEL_DEFAULT_INDEX = 2;
    private static final int POWER_DEFAULT_INDEX = 3;
    private static final int SUPPORT_BIT_INDEX = 4;
    private static final String TAG = "DesenseAT/MainActivity";
    private static final int DIALOG_TEST_ITEMS = 1;
    private static final int DIALOG_CURVE = 2;
    private static final int DIALOG_SV_IDS = 3;
    private static final int DIALOG_OMIT_WARNING = 5;
    private static final int DIALOG_EXIT = 6;
    private static final int DIALOG_REENTER = 7;
    private static final String PREF_FILE = "prefs_location";
    private static final String KEY_ITEM = "desense_at_item";
    private static final String KEY_MODE = "desense_at_mode";
    private static final String KEY_SVID = "desense_at_svid";
    private static final String KEY_SELECTED_ALL = "selectedAll";
    private static final String KEY_ATC_SEND_DONE = "atc_send";
    private static final String NEW_LINE = System.getProperty("line.separator");

    private static final int SV_GPS = 0;
    private static final int SV_GLONASS_L = 1;
    private static final int SV_GLONASS_M = 2;
    private static final int SV_GLONASS_H = 3;
    private static final int SV_BEIDOU = 4;
    private static final String GSM_BASEBAND = "gsm.baseband.capability";
    private static final int WCDMA = 0x04;
    private static final int TDSCDMA = 0x08;
    private static final int FDD_BAND_MIN = 1;
    private static final int FDD_BAND_MAX = 31;
    private static final int TDD_BAND_MIN = 33;
    private static final int TDD_BAND_MAX = 44;
    private static final int FILE_NUMBER = 5;
    private static final int SV_TYPE_NUMBER = 5;
    private static final DateFormat DATE_FORMAT = new SimpleDateFormat("yyyyMMddhhmmss");
    static List<BandItem> sGsmItems;
    static List<BandItem> sTddItems;
    static List<BandItem> sFddItems;
    static List<BandItem> sLteItems;
    static List<BandItem> sCdmaItems;
    private static HashMap<String, String> sItemsToCheck;
    private static boolean sTestCancelled = false;
    private boolean mRequestLocationFix = false;

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
    private List<String> mItemsToOmit = new ArrayList<String>();
    private boolean mSelectedAll = false;
    // UI components
    private Button mTestItemButton;
    private Button mSvIdButton;
    private Button mStartButton;
    private Button mStopButton;
    private TableLayout mTestModeRadio;
    private ListView mResultList;
    private MyAdapter mAdapter;
    private List<RadioButton> mRadioButtons;
    private ClientSocket mSocketClient = null;
    private boolean mIsRunning = false;
    private int[] mSelectedSvid = new int[MAX_SV_COUNT];
    private List<List<String>> mSvIdLists;
    private List<TestItem> mTestItems;
    private List<TestItem> mSelectedTestItems;
    private List<TestItem> mGenTestItems;
    private List<TestMode> mTestModes;
    private TestMode mCurrentMode;
    private FunctionTask mTask;
    private HashMap<TestItem, TestResult> mTestResults;
    private LocationManager mLocationManager;
    private boolean mInitGPSState;
    private String mOmitWarning = new String();
    private FileOutputStream mOutputData = null;
    private long[] bandSupported = new long[TestItem.INDEX_BAND_MAX];
    private final Handler responseHandler = new Handler() {
        public void handleMessage(final Message msg) {
            AsyncResult asyncResult = (AsyncResult) msg.obj;
            switch (msg.what) {
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
                        EmUtils.invokeOemRilRequestStringsEm(true, cmd_s,
                                responseHandler.obtainMessage(SUPPORT_QUERY_CDMA));

                    } else {
                        Elog.w(TAG, "ATC_SUPPORT_QUERY: responseHandler get have exception!");
                    }
                    break;
                case SUPPORT_QUERY_CDMA:
                    if (asyncResult.exception == null) {
                        bandSupported[TestItem.INDEX_CDMA_BAND] =
                                Util.getSupportedBandCdma((String[]) asyncResult.result);
                    } else {
                        Elog.w(TAG, "ATC_SUPPORT_CDMA_QUERY: responseHandler get have exception!");
                    }
                    initRfConfigList(DesenseAtActivity.this, bandSupported);
                    restoreSettings();
                    Elog.v(TAG, "query support bands finished.");
                    break;
                default:
                    break;
            }
        }
    };

    static void initItemToCheck(Context context) {
        sItemsToCheck = new HashMap<String, String>();
        sItemsToCheck.put(new Display3DTest().toString(),
                context.getResources().getString(R.string.desense_at_no_3d_display_apk));
        sItemsToCheck.put(new LiveWallpaperTest().toString(),
                context.getResources().getString(R.string.desense_at_no_live_wallpaper));
        sItemsToCheck.put(new Mp3PlayTest().toString(),
                context.getResources().getString(R.string.desense_at_no_mp3_warning));
        sItemsToCheck.put(new VideoPlayTest().toString(),
                context.getResources().getString(R.string.desense_at_no_3gp_warning));
        sItemsToCheck.put(new ExternalSdCardReadTest().toString(),
                context.getResources().getString(R.string.desense_at_no_sd_warning));
        sItemsToCheck.put(new ExternalSdCardWriteTest().toString(),
                context.getResources().getString(R.string.desense_at_no_sd_warning));
        sItemsToCheck.put(new FrontCameraPreviewTest().toString(),
                context.getResources().getString(R.string.desense_at_front_camera_not_support));
        sItemsToCheck.put(new BackCameraPreviewTest().toString(),
                context.getResources().getString(R.string.desense_at_back_camera_not_support));
    }

    static synchronized boolean getCancelled() {
        return sTestCancelled;
    }

    private static boolean isSupportedBand(long bits, int idx) {
        if ((bits & (1L << idx)) == 0)
            return false;
        return true;
    }

    private static void initGsmItems(Context context, long bitsSupported) {
        String[] bandDefault = context.getResources().getStringArray(
                R.array.clk_rf_desense_gsm_data);

        sGsmItems = new ArrayList<BandItem>();
        Elog.i(TAG, "initGsmItems bitsSupported: " + bitsSupported);
        int bandAarayLength = bandDefault.length;
        for (int k = 0; k < bandAarayLength; k++) {
            if (bandDefault[k] == null) {
                continue;
            }
            String[] values = bandDefault[k].split(",");
            if (!isSupportedBand(bitsSupported, Integer.parseInt(values[SUPPORT_BIT_INDEX])))
                continue;
            Elog.i(TAG, "band name = " + values[BAND_NAME_INDEX]);
            sGsmItems.add(new BandItem(values[BAND_NAME_INDEX],
                    Integer.parseInt(values[BAND_VALUE_INDEX]),
                    BandItem.BandType.BAND_GSM,
                    Integer.parseInt(values[CHANNEL_DEFAULT_INDEX]),
                    Integer.parseInt(values[POWER_DEFAULT_INDEX])));
        }

        for (BandItem item : sGsmItems) {
            Elog.d(TAG, "item content: " + item.getBandName() + " " + item.getSummary());
        }
    }

    private static void initTddItems(Context context, long bitsSupported) {
        String[] bandDefault = context.getResources().getStringArray(
                R.array.clk_rf_desense_td_scdma_data);
        Elog.i(TAG, "initTddItems bitsSupported: " + bitsSupported);
        int bandAarayLength = bandDefault.length;
        sTddItems = new ArrayList<BandItem>();
        for (int k = 0; k < bandAarayLength; k++) {
            if (bandDefault[k] == null) {
                continue;
            }
            String[] values = bandDefault[k].split(",");
            if (!isSupportedBand(bitsSupported, Integer.parseInt(values[SUPPORT_BIT_INDEX])))
                continue;
            sTddItems.add(new BandItem(values[BAND_NAME_INDEX],
                    Integer.parseInt(values[BAND_VALUE_INDEX]),
                    BandItem.BandType.BAND_TD,
                    Integer.parseInt(values[CHANNEL_DEFAULT_INDEX]),
                    Integer.parseInt(values[POWER_DEFAULT_INDEX])));
        }
    }

    private static void initFddItems(Context context, long bitsSupported) {
        String[] bandDefault = context.getResources().getStringArray(
                R.array.clk_rf_desense_wcdma_data);
        Elog.i(TAG, "initFddItems bitsSupported: " + bitsSupported);
        int bandAarayLength = bandDefault.length;
        sFddItems = new ArrayList<BandItem>();
        for (int k = 0; k < bandAarayLength; k++) {
            if (bandDefault[k] == null) {
                continue;
            }
            String[] values = bandDefault[k].split(",");
            if (!isSupportedBand(bitsSupported, Integer.parseInt(values[SUPPORT_BIT_INDEX])))
                continue;
            sFddItems.add(new BandItem(values[BAND_NAME_INDEX],
                    Integer.parseInt(values[BAND_VALUE_INDEX]),
                    BandItem.BandType.BAND_WCDMA,
                    Integer.parseInt(values[CHANNEL_DEFAULT_INDEX]),
                    Integer.parseInt(values[POWER_DEFAULT_INDEX])));
        }
    }

    private static void initLteItems(Context context, long bitsLSupported, long bitsHSupported) {
        String[] bandDefault = context.getResources().getStringArray(
                R.array.clk_rf_desense_lte_fdd_data);
        Elog.i(TAG, "initGsmItems bitsLSupported: " + bitsLSupported);
        int bandAarayLength = bandDefault.length;
        sLteItems = new ArrayList<BandItem>();
        for (int k = 0; k < bandAarayLength; k++) {
            if (bandDefault[k] == null) {
                continue;
            }
            String[] values = bandDefault[k].split(",");
            if (!isSupportedBand(bitsLSupported, Integer.parseInt(values[SUPPORT_BIT_INDEX])))
                continue;
            sLteItems.add(new BandItem(values[BAND_NAME_INDEX],
                    Integer.parseInt(values[BAND_VALUE_INDEX]),
                    BandItem.BandType.BAND_LTE_FDD,
                    Integer.parseInt(values[CHANNEL_DEFAULT_INDEX]),
                    Integer.parseInt(values[POWER_DEFAULT_INDEX]),
                    true)
            );
        }

        bandDefault = context.getResources().getStringArray(
                R.array.clk_rf_desense_lte_tdd_data);
        Elog.i(TAG, "initGsmItems bitsHSupported: " + bitsHSupported);
        bandAarayLength = bandDefault.length;

        for (int k = 0; k < bandAarayLength; k++) {
            if (bandDefault[k] == null) {
                continue;
            }
            String[] values = bandDefault[k].split(",");
            if (!isSupportedBand(bitsHSupported, Integer.parseInt(values[SUPPORT_BIT_INDEX])))
                continue;
            sLteItems.add(new BandItem(values[BAND_NAME_INDEX],
                    Integer.parseInt(values[BAND_VALUE_INDEX]),
                    BandItem.BandType.BAND_LTE_TDD,
                    Integer.parseInt(values[CHANNEL_DEFAULT_INDEX]),
                    Integer.parseInt(values[POWER_DEFAULT_INDEX]),
                    true));
        }
    }

    private static void initCdmaItems(Context context, long bitsSupported) {
        String[] bandDefault = context.getResources().getStringArray(
                R.array.clk_rf_desense_cdma_data);
        Elog.i(TAG, "initCdmaItems bitsSupported: " + bitsSupported);
        int bandAarayLength = bandDefault.length;
        sCdmaItems = new ArrayList<BandItem>();
        for (int k = 0; k < bandAarayLength; k++) {
            if (bandDefault[k] == null) {
                continue;
            }
            String[] values = bandDefault[k].split(",");
            if (!isSupportedBand(bitsSupported, Integer.parseInt(values[SUPPORT_BIT_INDEX])))
                continue;
            sCdmaItems.add(new BandItem(values[BAND_NAME_INDEX],
                    Integer.parseInt(values[BAND_VALUE_INDEX]),
                    BandItem.BandType.BAND_CDMA,
                    Integer.parseInt(values[CHANNEL_DEFAULT_INDEX]),
                    Integer.parseInt(values[POWER_DEFAULT_INDEX])));
        }
    }

    public static void initRfConfigList(Context context, long[] bandSupported) {

        BandItem.initDefaultData();
        initGsmItems(context, bandSupported[TestItem.INDEX_GSM_BAND]);
        if (ModemCategory.getModemType() == ModemCategory.MODEM_TD) {
            initTddItems(context, bandSupported[TestItem.INDEX_UMTS_BAND]);
        } else if (ModemCategory.getModemType() == ModemCategory.MODEM_FDD) {
            initFddItems(context, bandSupported[TestItem.INDEX_UMTS_BAND]);
        }
        if (ModemCategory.isLteSupport()) {
            initLteItems(context, bandSupported[TestItem.INDEX_LTE_FDD_BAND],
                    bandSupported[TestItem.INDEX_LTE_TDD_BAND]);
        }
        if (ModemCategory.isCdma()) {
            initCdmaItems(context, bandSupported[TestItem.INDEX_CDMA_BAND]);
        }
    }

    private boolean ctrlLocationFix(boolean on) {
        if (on && !mRequestLocationFix) {
            try {
                if (mLocationManager != null) {
                    if (!mLocationManager
                            .isProviderEnabled(LocationManager.GPS_PROVIDER)) {
                        Elog.w(TAG, "provider disabled");
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
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.desense_at);

        mTestItems = TestItem.getTestItems();
        mTestModes = TestMode.getTestModes();
        mSelectedTestItems = new ArrayList<TestItem>();
        mGenTestItems = new ArrayList<TestItem>();
        mTestResults = new HashMap<TestItem, TestResult>();
        //initRfConfigList();
        initComponents();
        initSvIdLists();
        restoreSettings();
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
        querySupportMode();
    }

    private Phone getPhoneHandler(boolean isCDMA) {
        Phone phone = null;
        if (isCDMA) {
            phone = ModemCategory.getCdmaPhone();
        } else {
            int phoneId = ModemCategory.getCapabilitySim();
            phone = PhoneFactory.getPhone(phoneId);
        }
        if (phone == null)
            Elog.i(TAG, "phone is null, isCDMA: " + isCDMA);
        else
            Elog.v(TAG, "getPhoneHandler successfully, isCDMA:" + isCDMA);
        return phone;
    }

    private void querySupportMode() {
        final String[] queryCommon = {"AT+EPBSE=?", "+EPBSE:"};
        Elog.v(TAG, "sendAtCommand: " + queryCommon[0] + ", " + queryCommon[1]);

        EmUtils.invokeOemRilRequestStringsEm(false, queryCommon, responseHandler.obtainMessage
                (SUPPORT_QUERY));
    }

    @Override
    protected void onResume() {
        // TODO Auto-generated method stub
        super.onResume();
        TestItem.updateContext(this);

        if (EmUtils.checkLocationProxyAppPermission(this, true) && !mRequestLocationFix) {
            showDialog(DIALOG_REENTER);
        }
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
        Elog.d(TAG, "onDestroy");
        super.onDestroy();
    }

    private void initResolution() {
        DisplayMetrics metric = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(metric);

        int width = metric.widthPixels;
        int height = metric.heightPixels;
        Util.setResolution(width, height);
    }

    @SuppressWarnings("deprecation")
    @Override
    public void onClick(View view) {
        if (view == mTestItemButton) {
            showDialog(DIALOG_TEST_ITEMS);
        } else if (view == mSvIdButton) {
            showDialog(DIALOG_SV_IDS);
        } else if (view.getId() == R.id.desense_at_start) {

            Elog.d(TAG, "desense_at_start click: " + mCurrentMode);

            if (mSelectedTestItems.size() <= 0) {
                Toast.makeText(this, R.string.desense_at_test_item_warning,
                        Toast.LENGTH_LONG).show();
                return;
            }
            if (mCurrentMode == null) {
                Toast.makeText(this, R.string.desense_at_test_mode_warning,
                        Toast.LENGTH_LONG).show();
                return;
            }
            if (("Signal".equals(mCurrentMode.toString())) && (!validSvIdSelected())) {
                Toast.makeText(this, R.string.desense_at_sv_id_warning,
                        Toast.LENGTH_LONG).show();
                return;
            }
            showReminderItems();
        } else if (view.getId() == R.id.desense_at_stop) {
            // stop test
            Elog.d(TAG, "cancel click");
            Toast.makeText(this, R.string.desense_at_test_stop_warning,
                    Toast.LENGTH_LONG).show();
            if (mCurrentMode != null) {
                Elog.d(TAG, "task cancel");
                setCancelled(true);
            }
        }
    }

    private void startTest() {
        // clear current result list
        mTestResults.clear();
        mAdapter.notifyDataSetChanged();
        // start test
        mTask = new FunctionTask();
        mTask.execute();
        enableButtons(false);
    }

    @SuppressWarnings("deprecation")
    private void showReminderItems() {

        mItemsToOmit.clear();
        mOmitWarning = this.getResources().getString(R.string.desense_at_test_start_warning_pre);
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
            mOmitWarning = mOmitWarning + strWarning.toString();
        }
        removeDialog(DIALOG_OMIT_WARNING);
        showDialog(DIALOG_OMIT_WARNING);
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
    public void onCheckedChanged(CompoundButton button, boolean checked) {
        if (checked) {
            mCurrentMode = (TestMode) button.getTag();
            // uncheck other radio buttons
            for (RadioButton r : mRadioButtons) {
                if (r != button) {
                    r.setChecked(false);
                }
            }
            saveSettings();
            Elog.d(TAG, "GpsCwMode: " + new GpsCwMode() +
                    "mCurrentMode:" + mCurrentMode);
            if ((new GpsCwMode()).toString().equals
                    (mCurrentMode.toString())) {
                Elog.d(TAG, "mSvIdButton.setEnabled(false): ");
                mSvIdButton.setEnabled(false);
            } else {
                mSvIdButton.setEnabled(true);
            }
        }
    }

    @Override
    protected Dialog onCreateDialog(int id, Bundle bundle) {
        switch (id) {
            case DIALOG_TEST_ITEMS:
                return createTestItemDialog();
            case DIALOG_SV_IDS:
                return createSvIdDialog();
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
            case DIALOG_EXIT:
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
            case DIALOG_REENTER:
                return new AlertDialog.Builder(this)
                        .setTitle(R.string.desense_at)
                        .setMessage(R.string.lbs_permission_granted_take_effect_msg)
                        .setCancelable(false)
                        .setPositiveButton(R.string.dialog_ok,
                                new DialogInterface.OnClickListener() {
                                    public void onClick(DialogInterface dialog, int which) {
                                        finish();
                                    }
                        }).create();
            default:
                return null;
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // TODO Auto-generated method stub
        menu.add(Menu.NONE, 0, 0, R.string.desense_at_rf_band_config);
        menu.add(Menu.NONE, 1, 0, R.string.desense_at_api_test);
        menu.add(Menu.NONE, 2, 0, R.string.desense_at_show_history);
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case 0:
                startActivity(new Intent(this, RfBandConfigActivity.class));
                return true;
            case 1:
                startActivity(new Intent(this, ApiTestActivity.class));
                return true;
            case 2:
                startActivity(new Intent(this, HistoryListActivity.class));
                return true;
            default:
                break;
        }
        return false;
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
        mTestItemButton = (Button) findViewById(R.id.desense_at_test_items);
        mTestItemButton.setOnClickListener(this);
        mSvIdButton = (Button) findViewById(R.id.desense_at_sv_ids);
        mSvIdButton.setOnClickListener(this);
        mStartButton = (Button) findViewById(R.id.desense_at_start);
        mStopButton = (Button) findViewById(R.id.desense_at_stop);
        mTestModeRadio = (TableLayout) findViewById(R.id.desense_at_test_modes);
        mAdapter = new MyAdapter(this);
        mResultList = (ListView) findViewById(R.id.desense_at_test_result);
        mResultList.setAdapter(mAdapter);
        mResultList.setOnItemClickListener(this);

        // Radio buttons for test modes
        TableRow row = new TableRow(this);
        mRadioButtons = new ArrayList<RadioButton>();
        for (int i = 0; i < mTestModes.size(); i++) {
            TestMode m = mTestModes.get(i);
            RadioButton button = new RadioButton(this);
            button.setText(m.toString());
            button.setTag(m);
            button.setOnCheckedChangeListener(this);
            TableRow.LayoutParams layoutParams =
                    new TableRow.LayoutParams(0, LayoutParams.WRAP_CONTENT, 1);
            TableLayout.LayoutParams layoutParams2 =
                    new TableLayout.LayoutParams(LayoutParams.MATCH_PARENT,
                            LayoutParams.WRAP_CONTENT);
            if (i % 3 == 0) {
                row = new TableRow(this);
                mTestModeRadio.addView(row, layoutParams2);
            }
            row.addView(button, layoutParams);
            mRadioButtons.add(button);
        }
    }

    private void initSvIdLists() {
        mSvIdLists = new ArrayList<List<String>>();
        for (int i = 0; i < MAX_SV_COUNT; i++) {
            mSvIdLists.add(new ArrayList<String>());
            mSvIdLists.get(i).add("NA");
        }
        // GPS sv IDs: 1 ~ 32
        for (int i = 1; i <= 32; i++) {
            mSvIdLists.get(0).add(String.valueOf(i));
        }
        // Glonass(L) sv IDs
        for (int i = 65; i <= 74; i++) {
            mSvIdLists.get(1).add(String.valueOf(i));
        }
        // Glonass(M) sv IDs
        for (int i = 75; i <= 85; i++) {
            mSvIdLists.get(2).add(String.valueOf(i));
        }
        // Glonass(H) sv IDs
        for (int i = 86; i <= 96; i++) {
            mSvIdLists.get(3).add(String.valueOf(i));
        }
        // Beidou sv IDs
        for (int i = 1; i <= 32; i++) {
            mSvIdLists.get(4).add(String.valueOf(i));
        }
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
        // selected sv id
        for (int i = 0; i < mSelectedSvid.length; i++) {
            mSelectedSvid[i] = Util.INVALID_SV_ID;
        }
        String[] svids = pref.getString(KEY_SVID, "").split(",");
        for (int i = 0; i < svids.length; i++) {
            try {
                mSelectedSvid[i] = Integer.parseInt(svids[i]);
            } catch (NumberFormatException e) {
                mSelectedSvid[i] = Util.INVALID_SV_ID;
            }
        }
        for (TestMode m : mTestModes) {
            m.setSvIds(mSelectedSvid);
        }

        // selected test mode
        String mode = pref.getString(KEY_MODE, "");
        for (RadioButton r : mRadioButtons) {
            r.setChecked(r.getText().toString().equals(mode));
        }
        if ((new GpsCwMode()).toString().equals(mode)) {
            Elog.d(TAG, "mSvIdButton.setEnabled(false): ");
            mSvIdButton.setEnabled(false);
        }

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

        // selected sv id
        str = "";
        for (int i = 0; i < MAX_SV_COUNT; i++) {
            str += String.valueOf(mSelectedSvid[i]) + ",";
        }
        pref.putString(KEY_SVID, str);

        // selected test mode
        if (mCurrentMode != null) {
            pref.putString(KEY_MODE, mCurrentMode.toString());
        }

        Elog.d(TAG, "putBoolean: " + mSelectedAll);
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
        AlertDialog dialog = new AlertDialog.Builder(this)
                .setTitle("Test Items")
                .setMultiChoiceItems(labels, values,
                        new DialogInterface.OnMultiChoiceClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which, boolean
                                    isChecked) {
                                if (which == 0) {
                                    values[which] = isChecked;
                                    Elog.d(TAG, "values[which]: " + values[which]);
                                    ListView lv = ((AlertDialog) dialog).getListView();
                                    for (int i = 1; i < labels.length; i++) {
                                        lv.setItemChecked(i, isChecked);
                                        values[i] = isChecked;
                                    }
                                    return;
                                } else {
                                    if (isChecked) {
                                        checkTestItemValid(which - 1);
                                    } else {
                                        ListView lv = ((AlertDialog) dialog).getListView();
                                        lv.setItemChecked(0, isChecked);
                                        values[0] = isChecked;
                                    }
                                    values[which] = isChecked;
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

                        Elog.d(TAG, "values[which]: " + values[0]);
                        mSelectedAll = values[0];

                        // Need to refresh ListView because test items changed
//                    updateListView();
                        saveSettings();
                    }
                })
                .setNegativeButton(android.R.string.cancel, null)
                .create();
        return dialog;
    }

    private boolean checkTestItemValid(int itemIndex) {

        TestItem testItem = mTestItems.get(itemIndex);
        String strToast = sItemsToCheck.get(testItem.toString());
        if ((strToast != null)
                && (!testItem.doApiTest())) {
            Toast.makeText(this, strToast,
                    Toast.LENGTH_LONG).show();
            Elog.d(TAG, "show toast: " + strToast);
            return false;
        }
        return true;
    }

    // user can select at most 5 SV IDs, show 5 Spinners.
    @SuppressLint("InflateParams")
    private Dialog createSvIdDialog() {
        View view = getLayoutInflater().inflate(R.layout.desense_at_sv_id, null);
        final Spinner[] spinners = new Spinner[MAX_SV_COUNT];
        spinners[SV_GPS] = (Spinner) view.findViewById(R.id.spinner_gps);
        spinners[SV_GLONASS_L] = (Spinner) view.findViewById(R.id.spinner_glonass_l);
        spinners[SV_GLONASS_M] = (Spinner) view.findViewById(R.id.spinner_glonass_m);
        spinners[SV_GLONASS_H] = (Spinner) view.findViewById(R.id.spinner_glonass_h);
        spinners[SV_BEIDOU] = (Spinner) view.findViewById(R.id.spinner_bd);

        for (int i = 0; i < MAX_SV_COUNT; i++) {
            List<String> svIdList = mSvIdLists.get(i);
            ArrayAdapter<String> adatper = new ArrayAdapter<String>(this,
                    android.R.layout.simple_spinner_item, svIdList);
            adatper.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            spinners[i].setAdapter(adatper);

            int index = svIdList.indexOf(String.valueOf(mSelectedSvid[i]));
            if (index > 0) {
                spinners[i].setSelection(index);
            } else {
                spinners[i].setSelection(0); // Select "NA"
            }
        }

        AlertDialog dialog = new AlertDialog.Builder(this)
                .setTitle("GNSS Band")
                .setView(view)
                .setPositiveButton(android.R.string.ok, new OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        for (int i = 0; i < spinners.length; i++) {
                            try {
                                int id = Integer.parseInt(spinners[i].getSelectedItem().toString());
                                mSelectedSvid[i] = id;
                            } catch (NumberFormatException e) {
                                mSelectedSvid[i] = Util
                                        .INVALID_SV_ID;
                            }
                        }
                        for (TestMode m : mTestModes) {
                            m.setSvIds(mSelectedSvid);
                            Elog.d(TAG, "mSelectedSvid: " + mSelectedSvid);
                        }
                        saveSettings();
                    }
                })
                .setNegativeButton(android.R.string.cancel, null)
                .create();
        return dialog;
    }

    // show curve for clicked test item
    @SuppressLint("InflateParams")
    private Dialog createCurveDialog(Bundle bundle) {
        int position = bundle.getInt("position", 0);
        if ((mAdapter == null) || (mAdapter.getCount() <= position)) {
            return null;
        }
        View view = getLayoutInflater().inflate(R.layout.desense_at_curve, null);
        TextView title = (TextView) view.findViewById(R.id.desense_at_curve_title);
        TextView info = (TextView) view.findViewById(R.id.desense_at_curve_info);
        TextView x = (TextView) view.findViewById(R.id.desense_at_curve_x_label);
        TextView y = (TextView) view.findViewById(R.id.desense_at_curve_y_label);
        CurveView curve = (CurveView) view.findViewById(R.id.desense_at_curve);
        TestItem item = mAdapter.getItem(position);
        TestResult r = mTestResults.get(item);
        if (r == null) {
            return null;
        }
        title.setText(item.toString());
        info.setText(r.getSummary());
        x.setText(r.x);
        y.setText(r.y);
        curve.setData(r);
        AlertDialog dialog = new AlertDialog.Builder(this).setView(view).create();
        return dialog;
    }

    private void updateListView() {
        mAdapter.clear();
        mAdapter.addAll(mGenTestItems);
        mAdapter.notifyDataSetChanged();
    }

    private void enableButtons(boolean enable) {
        mStartButton.setEnabled(enable);
        mStopButton.setEnabled(!enable);
        mTestItemButton.setEnabled(enable);
        mTestModeRadio.setEnabled(enable);
        mSvIdButton.setEnabled(enable);
        for (RadioButton r : mRadioButtons) {
            r.setEnabled(enable);
        }
    }

    private boolean validSvIdSelected() {
        boolean ret = false;
        Elog.d(TAG, "validSvIdSelected");
        for (int id : mSelectedSvid) {
            Elog.d(TAG, "id:" + id);
            if (id != Util.INVALID_SV_ID) {
                ret = true;
            }
        }
        return ret;
    }

    /**
     * @return if user cancel the test
     */
    private boolean isCancelled() {
        boolean ret = mTask.isCancelled();
        Elog.d(TAG, "isCancelled: " + ret);
        return ret;
    }

    static synchronized void setCancelled(boolean cancelled) {
        sTestCancelled = cancelled;
    }

    private void genRealTestItems() {
        mGenTestItems.clear();
        for (TestItem testitem : mSelectedTestItems) {
            if (mItemsToOmit.contains(testitem.toString())) {
                continue;
            }
            if (GsmBandVirtualTest.ITEM_TAG.equals(testitem
                    .toString())) {
                genGsmTestItem();
            } else if (FddBandVirtualTest.ITEM_TAG.equals
                    (testitem.toString())) {
                genFddTestItem();
            } else if (TddBandVirtualTest.ITEM_TAG.equals
                    (testitem.toString())) {
                genTddTestItem();
            } else if (LteBandVirtualTest.ITEM_TAG.equals
                    (testitem.toString())) {
                genLteTestItem();
            } else if (CdmaBandVirtualTest.ITEM_TAG.equals
                    (testitem.toString())) {
                genCdmaTestItem();
            } else {
                mGenTestItems.add(testitem);
            }
        }

        int size = mGenTestItems.size();
        for (int k = 0; k < size; k++) {
            Elog.d(TAG, "TestItem = " + mGenTestItems.get(k).toString());
            Elog.d(TAG, "TestItem index = " + (k + 1));
            mGenTestItems.get(k).setItemIndex(k + 1);
        }
    }

    private void genFddTestItem() {
        for (BandItem banditem : sFddItems) {
            if (!banditem.isSelected()) {
                continue;
            }
            int power = banditem.getPower();
            if (power != -1) {
                int[] channel = banditem.getChannel();
                for (int k = 0; k < 3; k++) {
                    if (channel[k] != -1) {
                        mGenTestItems.add(new FddBandTest
                                (banditem.getBandName(),
                                        channel[k], banditem.getBandValue(), power, this));
                    }
                }
            }
        }
    }

    private void genTddTestItem() {
        for (BandItem banditem : sTddItems) {
            if (!banditem.isSelected()) {
                continue;
            }
            int power = banditem.getPower();
            if (power != -1) {
                int[] channel = banditem.getChannel();
                for (int k = 0; k < 3; k++) {
                    if (channel[k] != -1) {
                        mGenTestItems.add(new TddBandTest
                                (banditem.getBandName(),
                                        channel[k], banditem.getBandValue(), power, this));
                    }
                }
            }
        }
    }

    private void genGsmTestItem() {
        for (BandItem banditem : sGsmItems) {
            if (!banditem.isSelected()) {
                continue;
            }
            int power = banditem.getPower();
            if (power != -1) {
                int[] channel = banditem.getChannel();
                for (int k = 0; k < 3; k++) {
                    if (channel[k] != -1) {
                        mGenTestItems.add(new GsmBandTest
                                (banditem.getBandName(),
                                        channel[k], banditem.getBandValue(), power, this));
                    }
                }
            }
        }
    }

    private void genLteTestItem() {
        for (BandItem banditem : sLteItems) {
            if (!banditem.isSelected()) {
                continue;
            }
            int power = banditem.getPower();
            if (power != -1) {
                int[] channel = banditem.getChannel();
                for (int k = 0; k < 3; k++) {
                    if (channel[k] != -1) {
                        mGenTestItems.add(new LteBandTest
                                (banditem.getBandName(), channel[k],
                                        banditem.getBandValue(), power,
                                        (banditem.getType() ==
                                                BandItem.BandType.BAND_LTE_FDD), this));
                    }
                }
            }
        }
    }

    private void genCdmaTestItem() {
        for (BandItem banditem : sCdmaItems) {
            if (!banditem.isSelected()) {
                continue;
            }
            int power = banditem.getPower();
            if (power != -1) {
                int[] channel = banditem.getChannel();
                for (int k = 0; k < 3; k++) {
                    if (channel[k] != -1) {
                        mGenTestItems.add(new CdmaBandTest
                                (banditem.getBandName(),
                                        channel[k], banditem.getBandValue(), power, this));
                    }
                }
            }
        }
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        // TODO Auto-generated method stub
        switch (keyCode) {
            case KeyEvent.KEYCODE_VOLUME_UP:
                if (mCurrentMode != null) {
                    Elog.d(TAG, "task cancel");
                    setCancelled(true);
                }
                return true;
            default:
                break;
        }
        return super.onKeyDown(keyCode, event);
    }

    private void initRecordData() {
        String folderPath = Util.getTestFilePath(this, com
                .mediatek.engineermode.desenseat.Util.HISTORY_PATH);
        File parentFolder = new File(folderPath);
        if (!parentFolder.exists()) {
            parentFolder.mkdirs();
        }
        File[] fileArray = parentFolder.listFiles();
        if (fileArray.length >= FILE_NUMBER) {
            List<File> fileList = Arrays.asList(fileArray);
            Collections.sort(fileList, new Comparator<File>() {

                @Override
                public int compare(File lhs, File rhs) {
                    // TODO Auto-generated method stub
                    return lhs.getName().compareTo(rhs.getName());
                }

            });
            int filesToDeleted = fileArray.length - FILE_NUMBER + 1;
            for (int k = 0; k < filesToDeleted; k++) {
                File file = fileList.get(k);
                file.delete();
            }
        }

        String strTime = DATE_FORMAT
                .format(new Date(System.currentTimeMillis()));

        String filePath = folderPath + "/" + strTime;
        Elog.d(TAG, "filePath = " + filePath);
        File logFile = new File(filePath);

        try {
            logFile.createNewFile();
            mOutputData = new FileOutputStream(logFile);
            String settings = "Mode: " + mCurrentMode.toString() + " BAND: ";
            for (int id : mSelectedSvid) {
                if (id != Util.INVALID_SV_ID) {
                    settings = settings + id + ' ';
                }
            }
            mOutputData.write(settings.getBytes());
            mOutputData.write(NEW_LINE.getBytes());
        } catch (IOException e) {
            mOutputData = null;
        }
    }

    private void saveResult(String testItem, TestResult result) {
        if ((mOutputData == null) || (result == null)) {
            return;
        }
        String title = testItem + "--" + result.result;
        String value = new String();

        if (mCurrentMode.toString().equals("Signal")) {
            for (int i = 0; i < SV_TYPE_NUMBER; i++) {
                Float desense = result.getDesenseValue(i);
                value = value + String.valueOf(desense) + ' ';
            }
        } else {
            Float desense = result.getDesenseValue(0);
            value = String.valueOf(desense);
        }

        ArrayList<float[]> dataArray = new ArrayList<float[]>();

        for (int k = 0; k < DesenseAtActivity.MAX_SV_COUNT; k++) {
            TestResult.Entry entry = result.subResults.get(k);
            if (entry == null) {
                dataArray.add(null);
            } else {
                float[] data = new float[entry.data.size()];
                for (int i = 0; i < data.length; i++) {
                    data[i] = entry.data.get(i);
                }
                dataArray.add(data);
            }
        }
        String strData = new String();
        for (float[] data : dataArray) {
            if (data == null) {
                strData += Util.INVALID_DATA;
            } else {
                for (float detailData : data) {
                    strData = strData + detailData + ' ';
                }
            }
            strData = strData.trim();
            strData = strData + ';';
        }
        try {
            mOutputData.write(title.getBytes());
            mOutputData.write(NEW_LINE.getBytes());

            mOutputData.write(value.getBytes());
            mOutputData.write(NEW_LINE.getBytes());

            mOutputData.write(result.getSummary().getBytes());
            mOutputData.write(NEW_LINE.getBytes());

            mOutputData.write(strData.getBytes());
            mOutputData.write(NEW_LINE.getBytes());

            mOutputData.flush();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void endRecordData() {
        if (mOutputData != null) {
            try {
                mOutputData.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
            mOutputData = null;
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
        MyAdapter(Context activity) {
            super(activity, 0);
        }

        @SuppressLint("InflateParams")
        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            ViewHolder holder;
            LayoutInflater inflater = DesenseAtActivity.this.getLayoutInflater();
            if (convertView == null) {
                convertView = inflater.inflate(R.layout.desense_at_result_entry, null);
                holder = new ViewHolder();
                holder.label = (TextView) convertView.findViewById(R.id.column1);
                holder.result = (TextView) convertView.findViewById(R.id.column2);
                holder.values = new TextView[SV_TYPE_NUMBER];
                holder.values[SV_GPS] = (TextView) convertView.findViewById(R.id.column3);
                holder.values[SV_GLONASS_L] = (TextView) convertView.findViewById(R.id.column4);
                holder.values[SV_GLONASS_M] = (TextView) convertView.findViewById(R.id.column5);
                holder.values[SV_GLONASS_H] = (TextView) convertView.findViewById(R.id.column6);
                holder.values[SV_BEIDOU] = (TextView) convertView.findViewById(R.id.column7);
                convertView.setTag(holder);
            } else {
                holder = (ViewHolder) convertView.getTag();
            }
            TestItem testItem = getItem(position);
            TestResult r = mTestResults.get(testItem);
            holder.label.setText(testItem.toString());

            if (r == null) {
                holder.result.setText("-");
            } else {
                switch (r.result) {
                    case TestResult.NONE:
                        holder.result.setText(Util.TESTING);
                        break;
                    case TestResult.PASS:
                        holder.result.setText(Util.PASS);
                        break;
                    case TestResult.FAIL:
                        holder.result.setText(Util.FAIL);
                        break;
                    case TestResult.CONNECTION_FAIL:
                        holder.result.setText(Util.CONN_FAIL);
                        break;
                    case TestResult.CNR_FAIL:
                        holder.result.setText(Util.CNR_FAIL);
                        break;
                    case TestResult.MODEM_FAIL:
                        holder.result.setText(Util.MODEM_FAIL);
                        break;
                    default:
                        break;
                }
            }
            // show desense values
            for (int i = 0; i < holder.values.length; i++) {
                holder.values[i].setText("-");
            }
            if (r != null && r.result != TestResult.NONE) {
                if (mCurrentMode.toString().equals("Signal")) {
                    for (int i = 0; i < holder.values.length; i++) {
                        Float desense = r.getDesenseValue(i);
                        if (desense != null) {
                            holder.values[i].setText(String.valueOf(desense));
                        }
                    }
                } else {
                    if (r.subResults.get(0) != null) {
                        Float desense = r.getDesenseValue(0);
                        if (desense != null) {
                            holder.values[0].setText(String.valueOf(desense));
                        }
                    }
                }
            }

            return convertView;
        }

        /**
         * ViewHolder.
         */
        private class ViewHolder {
            public TextView label;
            public TextView result;
            public TextView[] values;

            @Override
            public String toString() {
                return "ViewHolder [label=" + label + ", result=" + result
                        + ", values=" + Arrays.toString(values) + "]";
            }

        }
    }

    /**
     * Task to run test items.
     */
    private class FunctionTask extends AsyncTask<Void, Void, Boolean> {
        private TestCondition mTestCondition = null;

        @Override
        protected Boolean doInBackground(Void... params) {

            int size = mGenTestItems.size();
            if (size == 0) {
                return true;
            }
            mTestCondition.setCondition(DesenseAtActivity.this);
            StringBuilder strCmd = new StringBuilder(String.valueOf(size));

            for (TestItem item : mGenTestItems) {
                strCmd.append(',').append(item.getDbSpec());
            }
            if (!mCurrentMode.setSpec(strCmd.toString())) {
                runOnUiThread(new Runnable() {
                    public void run() {
                        Toast.makeText(DesenseAtActivity.this,
                                R.string.desense_at_set_spec_warning, Toast.LENGTH_LONG)
                                .show();
                    }
                });
            }

            setCancelled(false);
            initRecordData();
            mCurrentMode.setmTriggerSource(0, null);
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
                i.setTestMode(mCurrentMode);
                TestResult r = i.run();
                mTestResults.put(i, r);
                Elog.d(TAG, "TestResult = " + r);
                saveResult(i.toString(), r);
                final int index = k;
                runOnUiThread(new Runnable() {
                    public void run() {
                        mResultList.requestFocusFromTouch();
                        mResultList.setSelection(index);
                        mAdapter.notifyDataSetChanged();
                    }
                });
            }
            endRecordData();
            Util.flightMode(DesenseAtActivity.this, false);
            mTestCondition.resetCondition(DesenseAtActivity.this);
            return true;
        }

        @Override
        protected void onPostExecute(Boolean result) {
            enableButtons(true);
            mIsRunning = false;
            DesenseAtActivity.this.getWindow().clearFlags(
                    WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
            Elog.d(TAG, "clear FLAG_KEEP_SCREEN_ON");
            Util.notifyFinish(DesenseAtActivity.this);
        }

        @Override
        protected void onPreExecute() {
            // TODO Auto-generated method stub
            super.onPreExecute();
            mIsRunning = true;
            genRealTestItems();
            updateListView();
            DesenseAtActivity.this.getWindow().addFlags(
                    WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
            Elog.d(TAG, "add FLAG_KEEP_SCREEN_ON");
            mTestCondition = new TestCondition();
            mTestCondition.init(DesenseAtActivity.this);
        }

        @Override
        protected void onCancelled(Boolean result) {
            enableButtons(true);
        }

    }
}
