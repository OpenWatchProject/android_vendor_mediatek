package com.debug.loggerui.settings;

import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.app.Dialog;
import android.bluetooth.BluetoothAdapter;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.EditTextPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.Preference.OnPreferenceClickListener;
import android.preference.PreferenceActivity;
import android.preference.PreferenceCategory;
import android.preference.PreferenceManager;
import android.preference.PreferenceScreen;
import android.text.Editable;
import android.text.InputType;
import android.text.TextWatcher;
import android.view.WindowManager;
import android.widget.Toast;

import com.debug.loggerui.R;
import com.debug.loggerui.controller.BTHostLogController;
import com.debug.loggerui.controller.LogControllerUtils;
import com.debug.loggerui.utils.Utils;

/**
 * @author MTK81255
 *
 */
public class ConnsysLogSettings extends PreferenceActivity
                                implements OnPreferenceChangeListener, ISettingsActivity {
    private static final String TAG = Utils.TAG + "/ConnsysLogSettings";

    public static final String KEY_CONNSYSFW_LOGSIZE = "connsysfwlog_logsize";
    public static final String KEY_BTHOST_LOGSIZE = "bthostlog_logsize";
    public static final String KEY_BTSTACK_LOG_ENABLE = "btstacklog_enable";
    public static final String KEY_BTFW_LOG_LEVEL = "btfw_log_level";
    public static final String KEY_WIFI_LOG_TOOL = "wifi_log_tool";

    private static final String WIFI_LOG_TOOL_INTENT_ACTION =
            "mediatek.intent.action.engineermode.wifilogswitch";
    private static final int LIMIT_LOG_SIZE = 100;

    private EditTextPreference mConnsysFWLogSizeLimitPre;
    private CheckBoxPreference mGPSHostLogCheckBox;
    private CheckBoxPreference mBTHCILogCheckBox;
    private EditTextPreference mBTHostLogSizeLimitPre;
    private CheckBoxPreference mBTStackLogCheckBox;
    private ListPreference mBTFWLogLevelList;
    private Preference mWIFILogTool;

    private SharedPreferences mDefaultSharedPreferences;

    private long mSdcardSize;
    private Toast mToastShowing;

    private SettingsPreferenceFragement mPrefsFragement;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mPrefsFragement =
                SettingsPreferenceFragement.getInstance(this, R.layout.connsyslog_settings);
        getFragmentManager().beginTransaction().replace(
                android.R.id.content, mPrefsFragement).commit();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    @Override
    public void findViews() {
        mConnsysFWLogSizeLimitPre = (EditTextPreference) mPrefsFragement.
                findPreference(KEY_CONNSYSFW_LOGSIZE);
        mGPSHostLogCheckBox = (CheckBoxPreference) mPrefsFragement.
                findPreference(SettingsActivity.KEY_LOG_SWITCH_MAP.get(Utils.LOG_TYPE_GPSHOST));
        mBTHCILogCheckBox = (CheckBoxPreference) mPrefsFragement.
                findPreference(SettingsActivity.KEY_LOG_SWITCH_MAP.get(Utils.LOG_TYPE_BTHOST));
        mBTHostLogSizeLimitPre = (EditTextPreference) mPrefsFragement.
                findPreference(KEY_BTHOST_LOGSIZE);
        mBTStackLogCheckBox = (CheckBoxPreference) mPrefsFragement.
                findPreference(KEY_BTSTACK_LOG_ENABLE);
        mBTFWLogLevelList = (ListPreference) mPrefsFragement.findPreference(KEY_BTFW_LOG_LEVEL);
        mWIFILogTool = mPrefsFragement.findPreference(KEY_WIFI_LOG_TOOL);
    }

    @Override
    public void initViews() {
        Utils.logd(TAG, "initViews()");
        this.setTitle(R.string.connsyslog_settings);
        mDefaultSharedPreferences = PreferenceManager.getDefaultSharedPreferences(this);

        setAllPreferencesEnable(!LogControllerUtils.isAnyConnsysLogRunning());

        mConnsysFWLogSizeLimitPre.getEditText().setInputType(InputType.TYPE_CLASS_NUMBER);
        mBTHostLogSizeLimitPre.getEditText().setInputType(InputType.TYPE_CLASS_NUMBER);
        mSdcardSize = getIntent().getLongExtra(Utils.SDCARD_SIZE, LIMIT_LOG_SIZE);

        Object[] connsysFWLogObjs =
                {
                        getString(R.string.connsysFW_log_name),
                        LIMIT_LOG_SIZE,
                        mSdcardSize,
                        getString(Utils.LOG_PATH_TYPE_STRING_MAPS
                                .get(Utils.getCurrentLogPathType())) };
        mConnsysFWLogSizeLimitPre
                .setDialogMessage(getString(
                        R.string.limit_log_size_dialog_message, connsysFWLogObjs));
        mConnsysFWLogSizeLimitPre.setSummary(mDefaultSharedPreferences.getString(
                Utils.KEY_LOG_SIZE_MAP.get(Utils.LOG_TYPE_CONNSYSFW),
                String.valueOf(Utils.DEFAULT_CONFIG_LOG_SIZE_MAP.get(Utils.LOG_TYPE_CONNSYSFW)))
                + "MB");

        Object[] bthostObjs =
            {
                getString(R.string.bthost_log_name),
                LIMIT_LOG_SIZE,
                mSdcardSize,
                        getString(Utils.LOG_PATH_TYPE_STRING_MAPS
                                .get(Utils.getCurrentLogPathType())) };
        mBTHostLogSizeLimitPre
        .setDialogMessage(getString(R.string.limit_log_size_dialog_message, bthostObjs));
        mBTHostLogSizeLimitPre.setSummary(mDefaultSharedPreferences.getString(
                Utils.KEY_LOG_SIZE_MAP.get(Utils.LOG_TYPE_BTHOST),
                String.valueOf(Utils.DEFAULT_CONFIG_LOG_SIZE_MAP.get(Utils.LOG_TYPE_BTHOST)))
                + "MB");

        mBTFWLogLevelList.setSummary(mBTFWLogLevelList.getEntry());
        mBTFWLogLevelList.setOnPreferenceChangeListener(this);

        PackageManager pm = getPackageManager();
        PreferenceScreen screen = mPrefsFragement.getPreferenceScreen();
        if (!LogControllerUtils.getLogControllerInstance(Utils.LOG_TYPE_CONNSYSFW)
                .isLogFeatureSupport()) {
            PreferenceCategory connsysFWLogPreCategory =
                    (PreferenceCategory) mPrefsFragement.
                    findPreference("connsysfw_log_settings_category");
            screen.removePreference(connsysFWLogPreCategory);
        }

        Intent wifiLogToolIntent = new Intent(WIFI_LOG_TOOL_INTENT_ACTION);
        ResolveInfo ri = pm.resolveActivity(wifiLogToolIntent, 0);
        if (ri == null) {
            Utils.logw(TAG, "WIFI Log Tool does not exist! Remove Start UI.");
            PreferenceCategory wifiLogPreCategory =
                    (PreferenceCategory) mPrefsFragement.
                    findPreference("wifi_logl_settings_category");
            screen.removePreference(wifiLogPreCategory);
            mWIFILogTool = null;
        }

    }

    @Override
    public void setListeners() {
        Utils.logd(TAG, "setListeners()");
        mConnsysFWLogSizeLimitPre.setOnPreferenceChangeListener(this);
        mBTHostLogSizeLimitPre.setOnPreferenceChangeListener(this);
        mBTStackLogCheckBox.setOnPreferenceChangeListener(this);

        mConnsysFWLogSizeLimitPre.getEditText().addTextChangedListener(new TextWatcher() {

            @Override
            public void afterTextChanged(Editable editable) {
            }

            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
                Dialog dialog = mConnsysFWLogSizeLimitPre.getDialog();
                if (dialog != null && dialog instanceof AlertDialog) {
                    if ("".equals(String.valueOf(s))) {
                        ((AlertDialog) dialog).getButton(AlertDialog.BUTTON_POSITIVE).setEnabled(
                                false);
                        return;
                    }

                    try {
                        int inputSize = Integer.parseInt(String.valueOf(s));
                        boolean isEnable = (inputSize >= LIMIT_LOG_SIZE
                          && inputSize <= mSdcardSize );
                        String msg = "Please input a valid integer value ("
                                + LIMIT_LOG_SIZE + "~" + mSdcardSize + ").";
                        showToastMsg(isEnable, msg);
                        ((AlertDialog) dialog).getButton(AlertDialog.BUTTON_POSITIVE).setEnabled(
                                isEnable);
                    } catch (NumberFormatException e) {
                        Utils.loge(TAG, "Integer.parseInt(" + String.valueOf(s) + ") is error!");
                        ((AlertDialog) dialog).getButton(AlertDialog.BUTTON_POSITIVE).setEnabled(
                                false);
                    }
                }
            }

        });

        mBTHostLogSizeLimitPre.getEditText().addTextChangedListener(new TextWatcher() {

            @Override
            public void afterTextChanged(Editable editable) {
            }

            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
                Dialog dialog = mBTHostLogSizeLimitPre.getDialog();
                if (dialog != null && dialog instanceof AlertDialog) {
                    if ("".equals(String.valueOf(s))) {
                        ((AlertDialog) dialog).getButton(AlertDialog.BUTTON_POSITIVE).setEnabled(
                                false);
                        return;
                    }

                    try {
                        int inputSize = Integer.parseInt(String.valueOf(s));
                        boolean isEnable = (inputSize >= LIMIT_LOG_SIZE
                                && inputSize <= mSdcardSize );
                        String msg = "Please input a valid integer value ("
                                + LIMIT_LOG_SIZE + "~" + mSdcardSize + ").";
                        showToastMsg(isEnable, msg);
                        ((AlertDialog) dialog).getButton(AlertDialog.BUTTON_POSITIVE).setEnabled(
                                isEnable);
                    } catch (NumberFormatException e) {
                        Utils.loge(TAG, "Integer.parseInt(" + String.valueOf(s) + ") is error!");
                        ((AlertDialog) dialog).getButton(AlertDialog.BUTTON_POSITIVE).setEnabled(
                                false);
                    }
                }
            }

        });

        if (mWIFILogTool != null) {
            mWIFILogTool.setOnPreferenceClickListener(new OnPreferenceClickListener() {
                @Override
                public boolean onPreferenceClick(Preference arg0) {
                    Utils.logi(TAG, "Sent intent to open WIFI Log Tool.");
                    Intent wifiLogToolIntent = new Intent(WIFI_LOG_TOOL_INTENT_ACTION);
                    startActivity(wifiLogToolIntent);
                    return true;
                }
            });
        }

    }
    private void showToastMsg(boolean isEnable, String msg) {
        if (!isEnable) {
            if (mToastShowing == null) {
                mToastShowing = Toast.makeText(ConnsysLogSettings.this, msg,
                        Toast.LENGTH_LONG);
            } else {
                mToastShowing.setText(msg);
            }
            mToastShowing.show();
        }
        if (isEnable && mToastShowing != null) {
            mToastShowing.cancel();
            mToastShowing = null;
        }
    }
    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        Utils.logi(TAG, "Preference Change Key : " + preference.getKey() + " newValue : "
                + newValue);
        if (preference.getKey().equals(KEY_CONNSYSFW_LOGSIZE)) {
            LogControllerUtils.getLogControllerInstance(Utils.LOG_TYPE_CONNSYSFW)
                    .setLogRecycleSize(getIntByObj(newValue));
            mConnsysFWLogSizeLimitPre.setSummary(newValue + "MB");
        } else if (preference.getKey().equals(KEY_BTHOST_LOGSIZE)) {
            LogControllerUtils.getLogControllerInstance(Utils.LOG_TYPE_BTHOST)
                    .setLogRecycleSize(getIntByObj(newValue));
            mBTHostLogSizeLimitPre.setSummary(newValue + "MB");
        } else if (preference.getKey().equals(KEY_BTSTACK_LOG_ENABLE)) {
            BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
            boolean isBTEnabled = (bluetoothAdapter != null && bluetoothAdapter.isEnabled());
            showNeedRestartBTWarningDialog(isBTEnabled, (Boolean) newValue);
        } else if (preference.getKey().equals(KEY_BTFW_LOG_LEVEL)) {
            int valueIndex = mBTFWLogLevelList.findIndexOfValue((String) newValue);
            mBTFWLogLevelList.setValueIndex(valueIndex);
            mBTFWLogLevelList.setSummary(mBTFWLogLevelList.getEntries()[valueIndex]);
            BTHostLogController.getInstance().setBTFWLogLevel();
        }
        return true;
    }

    private void showNeedRestartBTWarningDialog(
            final boolean isBTEnabled, final boolean isStackLogEnabled) {
        Utils.logi(TAG, "showNeedRestartBTWarningDialog."
                + " isBTEnabled = " + isBTEnabled + ", isStackLogEnabled = " + isStackLogEnabled);
        Object[] msgObjects = {isStackLogEnabled ? "" : getString(R.string.reduce_to),
                isStackLogEnabled ? getString(R.string.log_too_much_warning_info) : ""};
        String message = getString(isBTEnabled ?
                R.string.btstacklog_restart_once_info : R.string.btstacklog_restart_twice_info
                , msgObjects);
        Builder builder =
                new AlertDialog.Builder(this)
                        .setTitle(R.string.bt_stack_log_name)
                        .setMessage(message)
                        .setPositiveButton(android.R.string.ok, new OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                if (isBTEnabled) {
                                    BTHostLogController.getInstance().setBTStackLogEnable();
                                }
                            }
                        })
                        .setNegativeButton(android.R.string.cancel, new OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog,
                                    int whichButton) {
                                mBTStackLogCheckBox.setChecked(!isStackLogEnabled);
                            }
                        });
        AlertDialog dialog = builder.create();
        dialog.setOnCancelListener(new OnCancelListener() {
            @Override
            public void onCancel(DialogInterface dialog) {
                mBTStackLogCheckBox.setChecked(!isStackLogEnabled);
            }
        });
        dialog.getWindow().setType(WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY);
        dialog.setCanceledOnTouchOutside(false);
        dialog.show();
    }

    private void setAllPreferencesEnable(boolean isEnable) {
        mConnsysFWLogSizeLimitPre.setEnabled(isEnable);
        mGPSHostLogCheckBox.setEnabled(isEnable);
        mBTHCILogCheckBox.setEnabled(isEnable);
        mBTHostLogSizeLimitPre.setEnabled(isEnable);
        mBTStackLogCheckBox.setEnabled(isEnable);
        mBTFWLogLevelList.setEnabled(isEnable);
        mWIFILogTool.setEnabled(isEnable);
    }

    private int getIntByObj(Object obj) {
        try {
            return Integer.parseInt(obj.toString());
        } catch (NumberFormatException e) {
            return 0;
        }
    }

}
