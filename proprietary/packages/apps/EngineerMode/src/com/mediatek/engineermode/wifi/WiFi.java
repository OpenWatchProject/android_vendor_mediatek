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
import android.content.res.Resources;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.TextView;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.FeatureHelpPage;
import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.R;

import java.util.ArrayList;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Class for EM wifi main UI activity.
 *
 */
public class WiFi extends Activity implements OnItemClickListener {

    private static final String TAG = "WifiMainpage";
    private static final int HANDLER_EVENT_INIT = 0x011;
    private static final int DIALOG_WIFI_INIT = 0;
    private static final int DIALOG_WIFI_WARN = 1;
    private static final int DIALOG_WIFI_FAIL = 2;
    private static final int DIALOG_WIFI_ERROR = 3;
    protected static final String KEY_CHIP_ID = "WiFiChipID";
    private static final long FUNC_INDEX_VERSION = 47;
    private static final long MASK_HIGH_16_BIT = 0xFFFF0000;
    private static final long MASK_HIGH_8_BIT = 0xFF00;
    private static final long MASK_8_BIT = 0xFF;
    private static final long MASK_32_BIT = 0xFFFFFFFF;
    private static final int BIT_16 = 16;
    private static final int BIT_8 = 8;
    private WifiManager mWifiManager = null;
    private int mChipID = 0x00;
    private WiFiStateManager mWiFiStateManager = null;
    private ListView mListTestItem = null;
    private static final String UNINIT_WIFI_CHIP = "UNINIT_WIFI_CHIP";
    private static final String BRANCH_PATTERN = "t-neptune[\\w-]*SOC[a-zA-Z0-9]*_[a-zA-Z0-9]*_";
    private static final String VER_SPLIT = "-";
    private static String sWifiChip = UNINIT_WIFI_CHIP;
    private static final String CHIP_6632 = "6632";
    private static final String CHIP_ADV = "0000";
    private static final String PROPERTY = "persist.vendor.connsys.chipid";
    private static final String PROPERTY_6779 = "0x6779";
    private boolean mMT6632Support = false;
    private ArrayList<String> mListData;

    private final Handler mHandler = new Handler() {

        public void handleMessage(Message msg) {

            if (HANDLER_EVENT_INIT == msg.what) {
                removeDialog(DIALOG_WIFI_INIT);
                checkWiFiChipStatus();

                mMT6632Support = (CHIP_6632.equals(getWifiChip())
                                  || CHIP_ADV.equals(getWifiChip()));
                Elog.i(TAG, "HANDLER_EVENT_INIT, miChipID = " + mChipID +
                       " mMT6632Support = " + mMT6632Support);
                mListData = new ArrayList<String>();
                mListData.add(getString(R.string.wifi_item_tx));
                mListData.add(getString(R.string.wifi_item_rx));
                if (!FeatureSupport.isUserLoad()) {
                    mListData.add(getString(R.string.wifi_item_mcr));
                    if (mMT6632Support) {
                        mListData.add(getString(R.string.wifi_item_rfcr));
                    }
                    mListData.add(getString(R.string.wifi_item_nvram));
                }

                if (showDbdc()) {
                    mListData.add(getString(R.string.wifi_item_dbdc_mode));
                }
                mListData.add(getString(R.string.help));
                ArrayAdapter<String> adapter = new ArrayAdapter<String>(
                        WiFi.this, android.R.layout.simple_list_item_1, mListData);
                mListTestItem.setAdapter(adapter);
                setListViewItemsHeight(mListTestItem);
                showVersion();
                ChannelInfo.getSupportChannels();
            }
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.wifi);

        mWifiManager = (WifiManager) this
                .getSystemService(Context.WIFI_SERVICE);
        if (mWifiManager == null) {
            showDialog(DIALOG_WIFI_ERROR);
            return;
        } else {
            if (WifiManager.WIFI_STATE_DISABLED != mWifiManager.getWifiState()
                    || mWifiManager.isScanAlwaysAvailable()) {
                showDialog(DIALOG_WIFI_WARN);
                return;
            }
        }
        mListTestItem = (ListView) findViewById(R.id.ListView_WiFi);
        mListTestItem.setOnItemClickListener(this);
        showDialog(DIALOG_WIFI_INIT);
        new Thread() {
            public void run() {
                if (mWiFiStateManager == null) {
                    mWiFiStateManager = new WiFiStateManager(WiFi.this);
                }
                mChipID = mWiFiStateManager.checkState(WiFi.this);
                mHandler.sendEmptyMessage(HANDLER_EVENT_INIT);
            }
        } .start();
        startService(new Intent(this, WifiStateListener.class));
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        Dialog dialog = null;
        AlertDialog.Builder builder = null;
        switch (id) {
        case DIALOG_WIFI_INIT:
            ProgressDialog innerDialog = new ProgressDialog(this);
            innerDialog.setTitle(R.string.wifi_dialog_init);
            innerDialog
                    .setMessage(getString(R.string.wifi_dialog_init_message));
            innerDialog.setCancelable(false);
            innerDialog.setIndeterminate(true);
            dialog = innerDialog;
            break;
        case DIALOG_WIFI_WARN:
            builder = new AlertDialog.Builder(this);
            builder.setTitle(R.string.wifi_dialog_warn);
            builder.setCancelable(false);
            builder.setMessage(getString(R.string.wifi_dialog_warn_message));
            builder.setPositiveButton(R.string.dialog_ok,
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            finish();
                        }
                    });
            dialog = builder.create();
            break;
        case DIALOG_WIFI_FAIL:
            builder = new AlertDialog.Builder(this);
            builder.setTitle(R.string.wifi_dialog_fail);
            builder.setCancelable(false);
            builder.setMessage(getString(R.string.wifi_dialog_fail_message));
            builder.setPositiveButton(R.string.dialog_ok,
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            finish();
                        }
                    });
            dialog = builder.create();
            break;
        case DIALOG_WIFI_ERROR:
            builder = new AlertDialog.Builder(this);
            builder.setTitle(R.string.wifi_dialog_error);
            builder.setCancelable(false);
            builder.setMessage(getString(R.string.wifi_dialog_error_message));
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


    @Override
    public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
        String itemName = mListData.get(arg2);
        if (itemName == null) {
            return;
        }
        Resources resources = getResources();
        Intent intent = null;
        if (WiFiStateManager.CHIP_ID_6620 == mChipID) {
            if (itemName.equals(resources.getString(R.string.wifi_item_tx))) {
                if (mMT6632Support) {
                    intent = new Intent(this, WiFiTx6632.class);
                } else {
                    intent = new Intent(this, WiFiTx6620.class);
                }

            } else if (itemName.equals(resources.getString(R.string.wifi_item_rx))) {
                if (mMT6632Support) {
                    intent = new Intent(this, WiFiRx6632.class);
                } else {
                    intent = new Intent(this, WiFiRx6620.class);
                }

            } else if (itemName.equals(resources.getString(R.string.wifi_item_mcr))) {
                Class<?> mcrClass = null;
                try {
                    mcrClass = Class.forName("com.mediatek.engineermode.wifi.WiFiMcr");
                } catch (ClassNotFoundException e) {
                    e.printStackTrace();
                    return;
                }
                intent = new Intent(this, mcrClass);
            } else if (itemName.equals(resources.getString(R.string.wifi_item_rfcr))) {
                Class<?> rfcrClass = null;
                try {
                    rfcrClass = Class.forName("com.mediatek.engineermode.wifi.WiFiRFCR");
                } catch (ClassNotFoundException e) {
                    e.printStackTrace();
                    return;
                }
                intent = new Intent(this, rfcrClass);
            } else if (itemName.equals(resources.getString(R.string.wifi_item_nvram))) {
                Class<?> eepromClass = null;
                try {
                    eepromClass = Class.forName("com.mediatek.engineermode.wifi.WiFiEeprom");
                } catch (ClassNotFoundException e) {
                    e.printStackTrace();
                    return;
                }
                intent = new Intent(this, eepromClass);
            } else if (itemName.equals(resources.getString(R.string.help))) {
                intent = new Intent(this, FeatureHelpPage.class);
                intent.putExtra(FeatureHelpPage.HELP_TITLE_KEY, R.string.help);
                intent.putExtra(FeatureHelpPage.HELP_MESSAGE_KEY, R.string.wifi_help_msg);
            } else if (itemName.equals(resources.getString(R.string.wifi_item_dbdc_mode))) {
                intent = new Intent(this, WiFiDBDC.class);
            }
        }
        intent.putExtra(KEY_CHIP_ID, mChipID);
        this.startActivity(intent);
    }

    /**
     * Show WiFi firmware version.
     */
    private void showVersion() {
        TextView mVersion = (TextView) findViewById(R.id.wifi_version);
        if (EMWifi.sIsInitialed) {
            //Show Manifest info
            String strManifestInfo = EMWifi.getFwManifestVersion();
            if ((strManifestInfo != null) && (!strManifestInfo.isEmpty())) {
                Elog.v(TAG, "strManifestInfo:" + strManifestInfo);
                View vBranch = findViewById(R.id.wifi_branch_layout);
                vBranch.setVisibility(View.VISIBLE);
                View vVer = findViewById(R.id.wifi_ver_layout);
                vVer.setVisibility(View.VISIBLE);
                mVersion.setVisibility(View.GONE);
                String strBranch = getBranch(strManifestInfo);

                if (strBranch != null) {
                    TextView tvBranch = (TextView) findViewById(R.id.wifi_branch_tv);
                    strBranch = strBranch.substring(0, strBranch.length() - 1);
                    Elog.v(TAG, "strBranch:" + strBranch);
                    tvBranch.setText(strBranch);
                }

                String strVer = getVer(strManifestInfo);
                Elog.v(TAG, "strVer:" + strVer);
                if (strVer != null) {
                    TextView tvVer = (TextView) findViewById(R.id.wifi_ver_tv);
                    tvVer.setText(strVer);
                }

            } else {
                //If Manifest info not support, EM shows version info
                StringBuilder stringBuild = new StringBuilder();
                stringBuild.append("VERSION: CHIP = MT");
                long[] version = new long[2];
                int result = EMWifi.getATParam(FUNC_INDEX_VERSION, version);
                if (0 == result) {
                    Elog.v(TAG, "version number is: 0x" + Long.toHexString(version[0]));
                    stringBuild.append(Long.toHexString((version[0] & MASK_HIGH_16_BIT) >> BIT_16));
                    stringBuild.append("  FW VER = v");
                    stringBuild.append(Long.toHexString((version[0] & MASK_HIGH_8_BIT) >> BIT_8));
                    stringBuild.append(".");
                    stringBuild.append(Long.toHexString(version[0] & MASK_8_BIT));
                    stringBuild.append(".");
                    stringBuild.append(Long.toHexString(version[1] & MASK_32_BIT));
                    mVersion.setText(stringBuild.toString());
                } else {
                    mVersion.setText("VERSION: Get fail");
                }
            }


        } else {
            mVersion.setText("VERSION: UNKNOWN");
        }
        Elog.i(TAG, "Wifi Chip Version is: " + mVersion.getText());
    }

    private String getBranch(String strInfo) {
        Pattern pattern = Pattern.compile(BRANCH_PATTERN);
        Matcher matcher = pattern.matcher(strInfo);
        if (matcher.find()) {
            return matcher.group(0);
        }
        return null;
    }

    private String getVer(String strInfo) {
        String strVersion = strInfo;
        int index = strInfo.lastIndexOf(VER_SPLIT);
        if ((index > 0) && (strInfo.length() > (index + 1))) {
            strVersion = strInfo.substring(index + 1);
        }
        int length = 8; //Date of year/month/day
        int year = 4; //End pos of year
        int month = 6; //End pos of month
        int day = 8; //End pos of day
        if (strVersion.length() < length) {
            return null;
        }
        String strYear = strVersion.substring(0, year);
        String strMonth = strVersion.substring(year, month);
        String strDay = strVersion.substring(month, day);
        String strSplit = "-";
        StringBuilder sbVer = new StringBuilder(strYear);
        sbVer.append(strSplit).append(strMonth).append(strSplit).append(strDay);
        if (strVersion.length() > length) {
            sbVer.append(strSplit).append(strVersion.substring(day));
        }
        return sbVer.toString();
    }

    static String getWifiChip() {
        if (UNINIT_WIFI_CHIP.equals(sWifiChip)) {
            long[] version = new long[2];
            int result = EMWifi.getATParam(FUNC_INDEX_VERSION, version);
            if (result != 0) {
                sWifiChip = null;
                return sWifiChip;
            }
            sWifiChip = Long.toHexString((version[0] & MASK_HIGH_16_BIT) >> BIT_16);
            return sWifiChip;
        } else {
            return sWifiChip;
        }

    }

    static boolean is11acSupported() {
        String[] supportedChips = {"6630", "6797", "6759"};
        String wifiChip = getWifiChip();
        Elog.d(TAG, "wifiChip:" + wifiChip);
        for (int i = 0; i < supportedChips.length; i++) {
            if (supportedChips[i].equals(wifiChip)) {
                return true;
            }
        }
        return false;
    }

    @Override
    protected void onStop() {
        super.onStop();
    }

    @Override
    protected void onDestroy() {
        mWiFiStateManager = null;
        stopService(new Intent(this, WifiStateListener.class));
        if (EMWifi.sIsInitialed) {
            mHandler.removeMessages(HANDLER_EVENT_INIT);
            removeDialog(DIALOG_WIFI_INIT);
            EMWifi.setNormalMode();
            EMWifi.unInitial();
            EMWifi.sIsInitialed = false;
            mWifiManager.setWifiEnabled(false);
        }
        super.onDestroy();
    }

    /**
     * Check WiFi chip status.
     */
    private void checkWiFiChipStatus() {
        switch (mChipID) {
        case WiFiStateManager.ENABLE_WIFI_FAIL:
            showDialog(DIALOG_WIFI_FAIL);
            break;
        case WiFiStateManager.CHIP_READY:
        case WiFiStateManager.INVALID_CHIP_ID:
        case WiFiStateManager.SET_TEST_MODE_FAIL:
            showDialog(DIALOG_WIFI_ERROR);
            break;
        case WiFiStateManager.CHIP_ID_6620:
        case WiFiStateManager.CHIP_ID_5921:
            break;
        default:
            break;
        }
    }

    static void setListViewItemsHeight(ListView listview) {
        if (listview == null) {
            return;
        }
        ListAdapter adapter = listview.getAdapter();
        int totalHeight = 0;
        for (int i = 0; i < adapter.getCount(); i++) {
            View itemView = adapter.getView(i, null, listview);
            itemView.measure(0, 0);
            totalHeight += itemView.getMeasuredHeight();
        }
        totalHeight += (adapter.getCount() - 1) * listview.getDividerHeight();
        ViewGroup.LayoutParams params = listview.getLayoutParams();
        params.height = totalHeight;
        listview.setLayoutParams(params);
    }

    private boolean showDbdc() {
        return PROPERTY_6779.equals(SystemProperties.get(PROPERTY));
    }

}
