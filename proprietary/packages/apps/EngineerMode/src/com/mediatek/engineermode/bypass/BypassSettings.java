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

package com.mediatek.engineermode.bypass;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.Bundle;
import android.os.SystemProperties;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.CheckBox;
import android.widget.CheckedTextView;
import android.widget.CompoundButton;
import android.widget.ListView;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.R;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;

import android.support.v4.content.LocalBroadcastManager;

/**
 * List for Bypass settings.
 */
public class BypassSettings extends Activity {
    public static final String USBMANAGER_ACTION_USB_STATE =
            "android.hardware.usb.action.USB_STATE";
    // PREFERENCE for service control
    public static final String PREF_SERV_ENABLE = "service_enable";
    private static final boolean DBG = true;
    private static final String TAG = "BypassSettings";
    private static final String USBMANAGER_USB_CONNECTED = "connected";
    private static final String ACTION_USB_BYPASS_SETFUNCTION =
            "com.via.bypass.action.setfunction";
    private static final String ACTION_USB_BYPASS_SETTETHERFUNCTION =
            "com.via.bypass.action.settetherfunction";
    private static final String VALUE_ENABLE_BYPASS =
            "com.via.bypass.enable_bypass";
    private static final String ACTION_USB_BYPASS_SETBYPASS =
            "com.via.bypass.action.setbypass";
    private static final String ACTION_USB_BYPASS_SETBYPASS_RESULT =
            "com.via.bypass.action.setbypass_result";
    private static final String VALUE_ISSET_BYPASS =
            "com.via.bypass.isset_bypass";
    private static final String ACTION_USB_BYPASS_GETBYPASS =
            "com.via.bypass.action.getbypass";
    private static final String ACTION_USB_BYPASS_GETBYPASS_RESULT =
            "com.via.bypass.action.getbypass_result";
    private static final String VALUE_BYPASS_CODE =
            "com.via.bypass.bypass_code";
    private static final String ACTION_MTK_LOGGER =
            "com.mediatek.mtklogger.bypass";
    private static final String ACTION_MTK_LOGGER_RESULT =
            "com.via.bypass.loggerui";
    private static final String CMD_NAME = "cmd_name";
    private static final String CMD_NAME_C2K_STATUS = "get_c2klog_status";
    private static final String CMD_RESULT = "cmd_result";
    private static final String USB_FUNCTION_BYPASS = "via_bypass";
    // Options menu item id
    private static final int OPT_MENU_ITEM_BYPASS_ALL = 1;
    private static final int OPT_MENU_ITEM_CLOSS_BYPASS = 2;
    private static final int DIALOG_WARNING = 1;
    private static final int BYPASS_CODE_ETS = 8;

    // Add for command line support
    private static final String CMD_CODE = "cmd_code";
    private static final String CMD_FILE = "cmd_file";
    private static final String CMD_NAME_QUERY = "query";
    private static final String CMD_NAME_ENABLE = "enable";
    private static final String CMD_NAME_DISABLE = "disable";
    private static final String CMD_CODE_ALL = "all";
    // bypass all code
    private final int mBypassNone = 0;
    private ListView mLvBypass;
    private String[] mBypassNames;
    private int[] mBypassCodes;
    private BypassAdapter<String> mBypassAdapter;
    // bypass mode enable flag
    private boolean mBypassEnabled = false;
    private int mBypassAllCode = mBypassNone;
    private int mBypassCode = 0;
    private boolean mIsBypassMode = false;
    private boolean mUsbConnected = false;
    private boolean mIsSettingBypass = false;
    private boolean mIsSettingUsb = false;
    private boolean mIsQueryingBypass = false;
    private Object mLock = new Object();
    private Dialog mDialog = null;
    private CheckBox mCbRndis;
    private CheckBox mCBBypassServiceEnable;

    private boolean mIsQueryingMtkLogger = false;
    private int mPendingBypassCode = 0;
    private boolean mIsPreparingForRndis = false;
    private boolean mIsSettingRndis = false;

    private String mCmdName;
    private String mCmdCode;
    private String mCmdFile;
    private boolean mLaunchByCommand;
    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            synchronized (mLock) {
                if (DBG) {
                    Elog.i(TAG, "onReceive = " + intent.getAction());
                }
                if (intent.getAction().equals(USBMANAGER_ACTION_USB_STATE)) {
                    mIsSettingUsb = false;
                    mUsbConnected = intent.getExtras().getBoolean(USBMANAGER_USB_CONNECTED);
                    mIsBypassMode = intent.getExtras().getBoolean(USB_FUNCTION_BYPASS);
                    queryBypassMode();
                    if (DBG) {
                        Elog.w(TAG, "UsbConnected = " + mUsbConnected
                                + ", mIsBypassMode = " + mIsBypassMode);
                    }
                } else if (intent.getAction().equals(ACTION_USB_BYPASS_SETBYPASS_RESULT)) {
                    boolean bypassMode = intent.getExtras().getBoolean(VALUE_ISSET_BYPASS);
                    if (DBG) {
                        Elog.w(TAG, "Set bypass mode is " + bypassMode);
                    }
                    mIsSettingBypass = false;
                    if (bypassMode) {
                        EmUtils.showToast(R.string.success, Toast.LENGTH_SHORT);
                    } else {
                        EmUtils.showToast(R.string.failed, Toast.LENGTH_SHORT);
                    }
                    mBypassCode = intent.getIntExtra(VALUE_BYPASS_CODE, mBypassCode);
                    updateBypassList(mBypassCode);

                    if (mIsPreparingForRndis) {
                        mIsPreparingForRndis = false;
                        enableRndis(true);
                    }
                } else if (intent.getAction().equals(ACTION_USB_BYPASS_GETBYPASS_RESULT)) {
                    mIsQueryingBypass = false;
                    mBypassCode = intent.getIntExtra(VALUE_BYPASS_CODE, mBypassCode);
                    updateBypassList(mBypassCode);
                    if (mLaunchByCommand) {
                        doCommand();
                    }
                } else if (intent.getAction().equals(ACTION_MTK_LOGGER_RESULT)) {
                    if (DBG) {
                        Elog.w(TAG, "mIsQueryingMtkLogger is " + mIsQueryingMtkLogger);
                    }
                    if (!mIsQueryingMtkLogger) {
                        return;
                    }
                    mIsQueryingMtkLogger = false;
                    String name = intent.getStringExtra(CMD_NAME);
                    int result = intent.getIntExtra(CMD_RESULT, 0);
                    if (DBG) {
                        Elog.w(TAG, CMD_NAME + "is " + name + ", " + CMD_RESULT + " is " + result);
                    }
                    if (name != null && name.equals(CMD_NAME_C2K_STATUS)) {
                        if (result == 1) {
                            showDialog(DIALOG_WARNING);
                        } else {
                            if (!mIsBypassMode) {
                                if (mIsSettingRndis) {
                                    enableBypassTether(true);
                                } else {
                                    enableBypass(true);
                                }
                            }
                            setBypassMode(mPendingBypassCode);
                        }
                    }
                    updateBypassList(mBypassCode);
                }
            }
        }
    };

    private AdapterView.OnItemClickListener mListener = new AdapterView.OnItemClickListener() {
        public void onItemClick(AdapterView parent, View view, int position, long id) {
            CheckedTextView ctview = (CheckedTextView) view;
            int bypassSet = mBypassCode;
            if ((mBypassCode & mBypassCodes[position]) != 0) {
                bypassSet ^= mBypassCodes[position];
                if (mIsBypassMode && bypassSet == 0) {
                    enableBypass(false);
                }
                setBypassMode(bypassSet);
            } else {
                if (mBypassCodes[position] == BYPASS_CODE_ETS) {
                    // Trying to enable ETS, should check MTK logger first
                    mPendingBypassCode = (bypassSet | BYPASS_CODE_ETS);
                    mIsSettingRndis = false;
                    queryMtkLogger();
                    updateBypassList(mBypassCode);
                    return;
                }
                bypassSet |= mBypassCodes[position];
                if (!mIsBypassMode) {
                    enableBypass(true);
                }
                setBypassMode(bypassSet);
            }
        }
    };

    private CompoundButton.OnCheckedChangeListener mCheckedChangeListener
            = new CompoundButton.OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            int id = buttonView.getId();
            if (id == R.id.cb_ets_rndis) {
                mLvBypass.setEnabled(!isChecked); // Disable other options when rndis is checked
                if (isChecked) {
                    if (mBypassCode > 0) {
                        // Clear other options when rndis is checked
                        enableBypass(false);
                        setBypassMode(mBypassNone);
                        updateBypassList(mBypassNone);
                        mIsPreparingForRndis = true;
                    } else {
                        enableRndis(true);
                    }
                } else {
                    enableRndis(false);
                }
            }
        }
    };
    private OnClickListener mServEnablePressed = new OnClickListener() {
        @Override
        public void onClick(View v) {
            Elog.w(TAG, "isChecked " + (((CheckBox) v).isChecked()));
            savePrefs();
            EmUtils.showToast("reboot to make change available", Toast.LENGTH_SHORT);
        }
    };

    private void savePrefs() {
        SharedPreferences settings = getSharedPreferences(PREF_SERV_ENABLE, 0);
        Editor editor = settings.edit();

        editor.putBoolean(PREF_SERV_ENABLE, mCBBypassServiceEnable.isChecked());
        editor.commit();
    }

    private void restorePrefsAndUpdateCheckBox() {
        SharedPreferences settings = getSharedPreferences(PREF_SERV_ENABLE, 0);
        boolean prefServEnable = settings.getBoolean(PREF_SERV_ENABLE, false);
        Elog.w(TAG, "prefServEnable : " + prefServEnable);

        if (prefServEnable) {
            mCBBypassServiceEnable.setChecked(true);
        } else {
            mCBBypassServiceEnable.setChecked(false);
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.bypass_settings);

        mBypassNames = getResources().getStringArray(R.array.bypass_names);
        mBypassCodes = getResources().getIntArray(R.array.bypass_codes);
        for (int i = 0; i < mBypassCodes.length; i++) {
            mBypassAllCode += mBypassCodes[i];
        }

        mLvBypass = (ListView) findViewById(R.id.lv_bypass);
        mBypassAdapter = new BypassAdapter<String>(this);
        mLvBypass.setAdapter(mBypassAdapter);

        mLvBypass.setItemsCanFocus(false);
        mLvBypass.setChoiceMode(ListView.CHOICE_MODE_MULTIPLE);
        mLvBypass.setOnItemClickListener(mListener);

        mCbRndis = (CheckBox) findViewById(R.id.cb_ets_rndis);
        mCbRndis.setOnCheckedChangeListener(mCheckedChangeListener);

        mCBBypassServiceEnable = (CheckBox) findViewById(R.id.bypass_service_eanble);
        mCBBypassServiceEnable.setOnClickListener(mServEnablePressed);
        restorePrefsAndUpdateCheckBox();

        mCbRndis.setVisibility(View.GONE);

        mCmdName = getIntent().getStringExtra(CMD_NAME);
        mCmdCode = getIntent().getStringExtra(CMD_CODE);
        mCmdFile = getIntent().getStringExtra(CMD_FILE);
        mLaunchByCommand = mCmdName != null && mCmdName.length() > 0;
    }

    @Override
    public Dialog onCreateDialog(int dialogId) {
        switch (dialogId) {
            case DIALOG_WARNING:
                mDialog = new AlertDialog.Builder(this)
                        .setTitle("Warning")
                        .setMessage("MD logger is running, enable ETS may cause Elog lost. "
                                + "Please stop MD logger first.\n\n"
                                + "Enable ETS anyway? (Not recommended)")
                        .setCancelable(false)
                        .setPositiveButton("Yes",
                                new DialogInterface.OnClickListener() {
                                    public void onClick(DialogInterface dialog, int which) {
                                        if (mIsSettingRndis) {
                                            enableBypassTether(true);
                                        } else {
                                            enableBypass(true);
                                        }
                                        setBypassMode(mPendingBypassCode);
                                    }
                                }
                        )
                        .setNegativeButton("No", null)
                        .create();
                return mDialog;
            default:
                return super.onCreateDialog(dialogId);
        }
    }

    /*
     * Set bypass interface status on list view
     */
    private void setBypassStatus(int bypassmode) {
        Elog.i(TAG, "setEnabled " + mBypassEnabled /*&& !mCbRndis.isChecked()*/);
        mLvBypass.setEnabled(mBypassEnabled /*&& !mCbRndis.isChecked()*/);
        //Elog.i(TAG, "mCbRndis.isChecked() " + mCbRndis.isChecked());
        //mCbRndis.setEnabled(mBypassEnabled);
        for (int i = 0; i < mLvBypass.getCount(); i++) {
            mLvBypass.setItemChecked(i, (bypassmode & mBypassCodes[i]) != 0);
        }
    }

    private void queryBypassMode() {
        Elog.i(TAG, "queryBypassMode()");
        mIsQueryingBypass = true;
        Intent intent = new Intent(ACTION_USB_BYPASS_GETBYPASS);
        LocalBroadcastManager.getInstance(this).sendBroadcast(intent);
    }

    private void queryMtkLogger() {
        Elog.i(TAG, "queryBypassMode()");
        mIsQueryingMtkLogger = true;
        Intent intent = new Intent(ACTION_MTK_LOGGER);
        intent.putExtra(CMD_NAME, CMD_NAME_C2K_STATUS);
        LocalBroadcastManager.getInstance(this).sendBroadcast(intent);
        EmUtils.showToast("Checking MTK logger status...", Toast.LENGTH_SHORT);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        menu.add(0, OPT_MENU_ITEM_BYPASS_ALL, Menu.NONE, "Bypass all");
        menu.add(0, OPT_MENU_ITEM_CLOSS_BYPASS, Menu.NONE, "Close bypass");
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        Elog.i(TAG, "onPrepareOptionsMenu() Enable = " + mBypassEnabled);
        menu.findItem(OPT_MENU_ITEM_BYPASS_ALL)
                .setEnabled(mBypassEnabled && !mCbRndis.isChecked());
        menu.findItem(OPT_MENU_ITEM_CLOSS_BYPASS)
                .setEnabled(mBypassEnabled && !mCbRndis.isChecked());
        return super.onPrepareOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        Elog.i(TAG, "onOptionsItemSelected");
        if (item.getItemId() == OPT_MENU_ITEM_BYPASS_ALL && mBypassCode != mBypassAllCode) {
            if ((mBypassCode & BYPASS_CODE_ETS) == 0) {
                // Trying to enable ETS, should check MTK logger first
                mPendingBypassCode = mBypassAllCode;
                mIsSettingRndis = false;
                queryMtkLogger();
                updateBypassList(mBypassCode);
                return super.onOptionsItemSelected(item);
            } else {
                if (!mIsBypassMode) {
                    enableBypass(true);
                }
                setBypassMode(mBypassAllCode);
            }
        } else if (item.getItemId() == OPT_MENU_ITEM_CLOSS_BYPASS && mIsBypassMode) {
            enableBypass(false);
            setBypassMode(mBypassNone);
        }
        return super.onOptionsItemSelected(item);
    }

    private void updateBypassList(int bypassmode) {
        Elog.i(TAG, "updateBypassList() mUsbConnected = " + mUsbConnected
                + ", mIsSettingBypass = " + mIsSettingBypass
                + ", mIsSettingUsb = " + mIsSettingUsb
                + ", mIsQueryingBypass = " + mIsQueryingBypass
                + ", mIsQueryingMtkLogger = " + mIsQueryingMtkLogger);
        mBypassEnabled = mUsbConnected && !mIsSettingBypass && !mIsSettingUsb && !mIsQueryingBypass
                && !mIsQueryingMtkLogger;
        if (DBG) {
            Elog.i(TAG, "updateBypassList() bypassmode = " + bypassmode
                    + ", mBypassEnabled = " + mBypassEnabled);
        }
        setBypassStatus(bypassmode);
        mBypassAdapter.notifyDataSetChanged();
    }

    @Override
    public void onResume() {
        super.onResume();
        if (DBG) {
            Elog.w(TAG, "onResume()");
        }

        queryBypassMode();

        updateBypassList(mBypassCode);

        restorePrefsAndUpdateCheckBox();

        //reister the mount service actions
        IntentFilter intentFilter = new IntentFilter(USBMANAGER_ACTION_USB_STATE);
        registerReceiver(mReceiver, intentFilter);

        IntentFilter bypass_intentFilter = new IntentFilter(ACTION_USB_BYPASS_SETBYPASS_RESULT);
        bypass_intentFilter.addAction(ACTION_USB_BYPASS_GETBYPASS_RESULT);
        bypass_intentFilter.addAction(ACTION_MTK_LOGGER_RESULT);
        LocalBroadcastManager.getInstance(this).registerReceiver(mReceiver, bypass_intentFilter);
    }

    @Override
    public void onPause() {
        super.onPause();
        if (DBG) {
            Elog.i(TAG, "onPause()");
        }
        if (!mLaunchByCommand) {
            LocalBroadcastManager.getInstance(this).unregisterReceiver(mReceiver);
        }
        if (mDialog != null) {
            mDialog.dismiss();
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (mLaunchByCommand) {
            LocalBroadcastManager.getInstance(this).unregisterReceiver(mReceiver);
        }
    }

    private void enableBypass(Boolean enable) {
        if (DBG) {
            Elog.i(TAG, "enableBypass(" + enable + ")");
        }
        mIsSettingUsb = true;
        Intent intent = new Intent(ACTION_USB_BYPASS_SETFUNCTION);
        intent.putExtra(VALUE_ENABLE_BYPASS, enable);
        LocalBroadcastManager.getInstance(this).sendBroadcast(intent);
    }

    private void enableBypassTether(Boolean enable) {
        if (DBG) {
            Elog.i(TAG, "enableBypassTether(" + enable + ")");
        }
        mIsSettingUsb = true;
        Intent intent = new Intent(ACTION_USB_BYPASS_SETTETHERFUNCTION);
        intent.putExtra(VALUE_ENABLE_BYPASS, enable);
        LocalBroadcastManager.getInstance(this).sendBroadcast(intent);
    }

    private void enableRndis(Boolean enable) {
        if (DBG) {
            Elog.i(TAG, "enableRndis(" + enable + ")");
        }
        if (enable) {
            if (!mLaunchByCommand) {
                // Trying to enable ETS, should check MTK logger first
                mPendingBypassCode = BYPASS_CODE_ETS;
                mIsSettingRndis = true;
                queryMtkLogger();
                updateBypassList(mBypassCode);
            } else {
                enableBypassTether(true);
                setBypassMode(BYPASS_CODE_ETS);
                updateBypassList(BYPASS_CODE_ETS);
            }
        } else {
            enableBypassTether(false);
            setBypassMode(mBypassNone);
            updateBypassList(mBypassNone);
        }
    }

    private void setBypassMode(int bypassmode) {
        if (DBG) {
            Elog.i(TAG, "setBypassMode(" + bypassmode + ")");
        }
        mIsSettingBypass = true;
        Intent intent = new Intent(ACTION_USB_BYPASS_SETBYPASS);
        intent.putExtra(VALUE_BYPASS_CODE, bypassmode);
        LocalBroadcastManager.getInstance(this).sendBroadcast(intent);
    }

    /* For command line use only */
    private void doCommand() {
        if (mCmdName == null || mCmdCode == null || mCmdFile == null) {
            return;
        }
        Elog.w(TAG, "mCmdName is " + mCmdName);
        Elog.w(TAG, "mCmdCode is " + mCmdCode);
        Elog.w(TAG, "mCmdFile is " + mCmdFile);

        int ret = -1;
        String config = SystemProperties.get("sys.usb.config");
        Elog.i(TAG, "sys.usb.config: " + config);
        boolean rndisEnable = config.contains("rndis") && config.contains("via_bypass");

        if (mCmdCode.equals("rndis")) {
            switch (mCmdName) {
                case CMD_NAME_QUERY:
                    ret = rndisEnable ? 1 : 0;
                    break;
                case CMD_NAME_ENABLE:
                    if (!rndisEnable) {
                        mLvBypass.setEnabled(false); // Disable other options when rndis is checked
                        if (mBypassCode > 0) {
                            // Clear other options when rndis is checked
                            enableBypass(false);
                            setBypassMode(mBypassNone);
                            updateBypassList(mBypassNone);
                            mIsPreparingForRndis = true;
                        } else {
                            enableRndis(true);
                        }
                    }
                    ret = 0;
                    break;
                case CMD_NAME_DISABLE:
                    if (rndisEnable) {
                        mLvBypass.setEnabled(true);
                        enableRndis(false);
                    }
                    ret = 0;
                    break;
                default:
                    break;
            }
        } else {
            int code = -1;
            if (mCmdCode.equals(CMD_CODE_ALL)) {
                code = mBypassAllCode;
            } else {
                for (int i = 0; i < mBypassNames.length; i++) {
                    if (mCmdCode.equals(mBypassNames[i])) {
                        code = mBypassCodes[i];
                    }
                }
            }
            Elog.w(TAG, "code is " + code);
            Elog.w(TAG, "mBypassCode is " + mBypassCode);
            Elog.w(TAG, "mBypassAllCode is " + mBypassAllCode);

            if (code != -1) {
                int bypassSet = mBypassCode;
                switch (mCmdName) {
                    case CMD_NAME_QUERY:
                        if (code == mBypassAllCode) {
                            ret = mBypassCode;
                        } else {
                            ret = (mBypassCode & code) > 0 ? 1 : 0;
                        }
                        break;
                    case CMD_NAME_ENABLE:
                        if (rndisEnable) {
                            ret = -1;
                            break;
                        }
                        if ((mBypassCode & code) != code) {
                            bypassSet |= code;
                            if (!mIsBypassMode) {
                                enableBypass(true);
                            }
                            setBypassMode(bypassSet);
                        }
                        ret = 0;
                        break;
                    case CMD_NAME_DISABLE:
                        if (rndisEnable) {
                            ret = -1;
                            break;
                        }
                        if ((mBypassCode & code) != 0) {
                            bypassSet &= ~code;
                            if (mIsBypassMode && bypassSet == 0) {
                                enableBypass(false);
                            }
                            setBypassMode(bypassSet);
                        }
                        ret = 0;
                        break;
                    default:
                        break;
                }
            }
        }

        File handle = new File(mCmdFile + ".tmp");
        OutputStream bos = null;
        try {
            bos = new FileOutputStream(handle, true);
            bos.write(String.valueOf(ret).getBytes());
        } catch (IOException ex) {
            ex.printStackTrace();
            handle.delete();
        } finally {
            if (bos != null) {
                try {
                    bos.close();
                } catch (IOException ex) {
                    ex.printStackTrace();
                }
            }
        }
        handle.renameTo(new File(mCmdFile));

        mCmdName = null;
        mCmdCode = null;
        mCmdFile = null;
    }

    /**
     * For instrumentation only.
     *
     * @param mask the mask of mode to enable
     * @return 0 for success, -1 if fail
     */
    public int enableBypassModeWait(int mask) {
        synchronized (mLock) {
            queryBypassModeWait(mask);
            int newMode = mBypassCode | mask;
            if (!mIsBypassMode) {
                enableBypass(true);
            }
            setBypassMode(newMode);
            waitForFinished();
            return ((mBypassCode & mask) != 0) ? 0 : -1;
        }
    }

    /**
     * For instrumentation only.
     *
     * @param mask the mask of mode to disable
     * @return 0 for success, -1 if fail
     */
    public int disableBypassModeWait(int mask) {
        synchronized (mLock) {
            queryBypassModeWait(mask);
            int newMode = mBypassCode & ~mask;
            if (newMode == 0) {
                enableBypass(false);
            }
            setBypassMode(newMode);
            waitForFinished();
            return ((mBypassCode & mask) == 0) ? 0 : -1;
        }
    }

    /**
     * For instrumentation only.
     *
     * @param mask the mask of mode to query
     * @return current mode
     */
    public int queryBypassModeWait(int mask) {
        synchronized (mLock) {
            IntentFilter intentFilter = new IntentFilter(USBMANAGER_ACTION_USB_STATE);
            registerReceiver(mReceiver, intentFilter);

            IntentFilter bypass_intentFilter = new IntentFilter(ACTION_USB_BYPASS_SETBYPASS_RESULT);
            bypass_intentFilter.addAction(ACTION_USB_BYPASS_GETBYPASS_RESULT);
            LocalBroadcastManager.getInstance(this)
                                 .registerReceiver(mReceiver, bypass_intentFilter);
            Elog.i(TAG, "registerReceiver");

            queryBypassMode();
            waitForFinished();
            int ret = mBypassCode & mask;
            if ((mask & 0xF) != 0xF) {
                // query for a specific mode, return 1 or 0
                return ret > 0 ? 1 : 0;
            } else {
                // query all
                return ret;
            }
        }
    }

    /* For instrumentation only. */
    private void waitForFinished() {
        while (mIsSettingBypass || mIsSettingUsb || mIsQueryingBypass) {
            Elog.i(TAG, "wait... "
                    + mIsSettingBypass + " " + mIsSettingUsb + " " + mIsQueryingBypass);
            try {
                mLock.wait(100);
            } catch (InterruptedException e) {
                continue;
            }
        }
    }

    /**
     * Adapter for bypass list view.
     */
    private class BypassAdapter<T> extends BaseAdapter {
        public BypassAdapter(Context context) {
            super();
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            LayoutInflater inflater;
            inflater = (LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            CheckedTextView ctview = (CheckedTextView) inflater.inflate(
                    android.R.layout.simple_list_item_multiple_choice, parent, false);
            ctview.setText(mBypassNames[position]);
            ctview.setEnabled(mBypassEnabled && !mCbRndis.isChecked());
            return ctview;
        }

        @Override
        public int getCount() {
            return mBypassNames.length;
        }

        @Override
        public Object getItem(int arg0) {
            return mBypassNames[arg0];
        }

        @Override
        public long getItemId(int arg0) {
            return mBypassCodes[arg0];
        }
    }
}
