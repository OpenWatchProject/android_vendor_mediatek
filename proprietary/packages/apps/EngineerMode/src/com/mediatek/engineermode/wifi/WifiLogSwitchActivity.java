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

package com.mediatek.engineermode.wifi;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.SystemClock;
import android.preference.PreferenceManager;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.Spinner;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EngineerMode;
import com.mediatek.engineermode.R;

import java.util.ArrayList;

/**
 * Class for switch Wifi log.
 *
 */
public class WifiLogSwitchActivity extends Activity
    implements OnItemSelectedListener, OnItemClickListener {

    private static final String TAG = "WifiLogSwitch";
    private static final String DEV_FW_LOG_ACTION =
        "android.settings.APPLICATION_DEVELOPMENT_SETTINGS";
    private static final String PREF_DRIVER_LOG_LEVEL = "pref_wifilog_driver";
    private static final String PREF_FW_LOG_LEVEL = "pref_wifilog_fw";
    private static final int VALUE_DEFAULT_LOG_LEVEL = 0;
    private static final int LOG_MODULE_DRIVER = 0;
    private static final int LOG_MODULE_FW = 1;

    private static final int LOG_LEVEL_OFF = 0;
    private static final int LOG_LEVEL_DEFAULT = 1;
    private static final int LOG_LEVEL_EXTREME = 2;

    private static final long DEFAULT_WAIT_TIME = 100;
    private static final int CHIP_ID_6620 = 0x6620;
    private static final int DLG_INIT_ITEMS = 0;
    private static final int DLG_ENABLE_WIFI_ERROR = 1;

    private Spinner mSpDriver = null;
    private Spinner mSpFw = null;
    private ListView mListLog = null;
    private boolean mInited = false;
    private ArrayList<String> mListData;

    private WifiManager mWifiManager = null;
    private ArrayAdapter<String> mLogAdapter;
    private Intent mFwLogIntent;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (EngineerMode.isAutoTest()) {
            finish();
            return;
        }
        setContentView(R.layout.wifi_log_switch);
        mSpDriver = (Spinner) findViewById(R.id.driver_log_spinner);
        mSpFw = (Spinner) findViewById(R.id.fw_log_spinner);
        mListData = new ArrayList<String>();

        mLogAdapter = new ArrayAdapter<String>(
                this, android.R.layout.simple_list_item_1, mListData);
        mListLog = (ListView) findViewById(R.id.log_listview);
        mListLog.setAdapter(mLogAdapter);
        mListLog.setOnItemClickListener(this);
        mFwLogIntent = new Intent(DEV_FW_LOG_ACTION);
        mWifiManager = (WifiManager) this
                .getSystemService(Context.WIFI_SERVICE);
        showDialog(DLG_INIT_ITEMS);
        new Thread() {
            public void run() {
                // TODO Auto-generated method stub
                Elog.d(TAG, "initItems");
                if (!enableWifi()) {
                    runOnUiThread(new Runnable() {
                        public void run() {
                            removeDialog(DLG_INIT_ITEMS);
                            showDialog(DLG_ENABLE_WIFI_ERROR);
                        }
                    });
                    return;
                }
                initItems();
            }
        } .start();


    }


    @Override
    public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2,
            long arg3) {
        // TODO Auto-generated method stub
        Elog.i(TAG, "Selected:" + arg2);
        if (arg0.equals(mSpDriver)) {
            EMWifi.setWifiLogLevel(LOG_MODULE_DRIVER, arg2);
            saveSettings(true, arg2);
        } else if (arg0.equals(mSpFw)) {
            EMWifi.setWifiLogLevel(LOG_MODULE_FW, arg2);
            saveSettings(false, arg2);
        }
    }

    @Override
    public void onNothingSelected(AdapterView<?> arg0) {
        // TODO Auto-generated method stub
        return;
    }


    @Override
    protected void onStart() {
        // TODO Auto-generated method stub
        super.onStart();
        updateItems();
    }

    private void initItems() {
        EMWifi.initial();
        runOnUiThread(new Runnable() {
            public void run() {
                Elog.i(TAG, "driver log enable: " + EMWifi.isWifiLogUiEnable(LOG_MODULE_DRIVER));
                Elog.i(TAG, "FW log enable: " + EMWifi.isWifiLogUiEnable(LOG_MODULE_FW));
                mInited = true;
                if (!EMWifi.isWifiLogUiEnable(LOG_MODULE_DRIVER)) {
                    View view = findViewById(R.id.driver_log_layout);
                    view.setVisibility(View.GONE);
                } else {
                    mSpDriver.setOnItemSelectedListener(WifiLogSwitchActivity.this);
                }

                if (!EMWifi.isWifiLogUiEnable(LOG_MODULE_FW)) {
                    View view = findViewById(R.id.fw_log_layout);
                    view.setVisibility(View.GONE);
                } else {
                    mSpFw.setOnItemSelectedListener(WifiLogSwitchActivity.this);
                }
                updateItems();
                removeDialog(DLG_INIT_ITEMS);
            }
        });
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        switch (id) {
        case DLG_INIT_ITEMS:
            ProgressDialog innerDialog = new ProgressDialog(this);
            innerDialog.setTitle(R.string.wifi_dialog_init);
            innerDialog
                    .setMessage(getString(R.string.wifi_log_init_items));
            innerDialog.setCancelable(false);
            innerDialog.setIndeterminate(true);
            return innerDialog;
        case DLG_ENABLE_WIFI_ERROR:
            return new AlertDialog.Builder(this).setTitle(R.string.wifi_dialog_fail)
                    .setCancelable(false)
                    .setMessage(getString(R.string.wifi_dialog_fail_message))
                    .setPositiveButton(R.string.dialog_ok, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            finish();
                        }
            }).create();
        default:
            break;
        }
        return null;
    }


    @Override
    protected void onDestroy() {
        // TODO Auto-generated method stub
        if (mInited) {
            EMWifi.unInitial();
            mInited = false;
        }

        Elog.i(TAG, "mInited:" + mInited);
        super.onDestroy();
    }

    private void updateItems() {
        mListData.clear();
        boolean on = mWifiManager.getVerboseLoggingLevel() > 0;
        Elog.i(TAG, "VerboseLog:" + on);
        mListData.add(getString(on ? R.string.wifi_log_switch_fwk_suppli_on
                : R.string.wifi_log_switch_fwk_suppli_off));
        mLogAdapter.notifyDataSetChanged();
        Elog.i(TAG, "mInited:" + mInited);
        if (mInited) {
            if (mSpDriver.getVisibility() == View.VISIBLE) {
                Elog.i(TAG, "Driver:" + EMWifi.getWifiLogLevel(LOG_MODULE_DRIVER));
                mSpDriver.setSelection(EMWifi.getWifiLogLevel(LOG_MODULE_DRIVER));
            }
            if (mSpFw.getVisibility() == View.VISIBLE) {
                Elog.i(TAG, "FW:" + EMWifi.getWifiLogLevel(LOG_MODULE_FW));
                mSpFw.setSelection(EMWifi.getWifiLogLevel(LOG_MODULE_FW));
            }
        }
    }

    private boolean enableWifi() {
        if (mWifiManager != null) {
            if (mWifiManager.getWifiState() == WifiManager.WIFI_STATE_ENABLED) {
                return true;
            } else {
                if (mWifiManager.setWifiEnabled(true)) {
                    while (mWifiManager.getWifiState() != WifiManager.WIFI_STATE_ENABLED) {
                        SystemClock.sleep(DEFAULT_WAIT_TIME);
                    }
                    return true;
                } else {
                    Elog.e(TAG, "enable wifi failed");
                    return false;
                }
            }

        }
        return false;
    }


    @Override
    public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
        // TODO Auto-generated method stub

        String itemName = mListData.get(arg2);
        if (itemName == null) {
            return;
        }
        if (arg2 == 0) {
            if (getPackageManager().resolveActivity(mFwLogIntent, 0) == null) {
                Elog.i(TAG, "no intent receiver");
                Toast.makeText(this, getString(R.string.wifi_log_switch_fwk_not_support),
                        Toast.LENGTH_SHORT).show();
            } else {
                startActivity(mFwLogIntent);
            }
        }

    }

    /**
     * Re-set Wifi log level when Wifi is re-enable.
     * @param context The environment contxt
     */
    public static void onWifiStateChanged(Context context) {
        WifiManager wifiMgr = (WifiManager) context.getSystemService(Context.WIFI_SERVICE);
        if (wifiMgr.getWifiState() == WifiManager.WIFI_STATE_ENABLED) {
            int driverLogLevel = restoreSettings(true, context);
            int fwLogLevel = restoreSettings(false, context);

            Elog.i(TAG, "[Wifi Enabled] driverLogLevel " + driverLogLevel +
                    " fwLogLevel " + fwLogLevel);
            if ((driverLogLevel == 0) && (fwLogLevel == 0)) {
                return;
            }
            EMWifi.initial();
            if (driverLogLevel != 0) {
                EMWifi.setWifiLogLevel(LOG_MODULE_DRIVER, driverLogLevel);
            }
            if (fwLogLevel != 0) {
                EMWifi.setWifiLogLevel(LOG_MODULE_FW, fwLogLevel);
            }
            EMWifi.unInitial();
        }

    }

    private void saveSettings(boolean driverLog, int value) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
        Editor editor = prefs.edit();
        editor.putInt(driverLog ? PREF_DRIVER_LOG_LEVEL : PREF_FW_LOG_LEVEL, value);
        editor.commit();
        Elog.i(TAG, "save driver log:" + driverLog + " value:" + value);
    }

    private static int restoreSettings(boolean driverLog, Context context) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        int value = prefs.getInt(driverLog ? PREF_DRIVER_LOG_LEVEL : PREF_FW_LOG_LEVEL,
                VALUE_DEFAULT_LOG_LEVEL);
        Elog.i(TAG, "restore driver log:" + driverLog + " value:" + value);
        return value;
    }
}