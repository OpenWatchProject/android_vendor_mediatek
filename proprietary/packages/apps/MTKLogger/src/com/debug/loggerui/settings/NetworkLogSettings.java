package com.debug.loggerui.settings;

import android.app.AlertDialog;
import android.app.Dialog;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.EditTextPreference;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.PreferenceActivity;
import android.preference.PreferenceManager;
import android.preference.PreferenceScreen;
import android.text.Editable;
import android.text.InputType;
import android.text.TextWatcher;
import android.widget.Toast;

import com.debug.loggerui.R;
import com.debug.loggerui.controller.LogControllerUtils;
import com.debug.loggerui.controller.NetworkLogController;
import com.debug.loggerui.utils.Utils;

/**
 * @author MTK81255
 *
 */
public class NetworkLogSettings extends PreferenceActivity
                                implements OnPreferenceChangeListener, ISettingsActivity {
    private static final String TAG = Utils.TAG + "/NetworkLogSettings";

    /**
     * Add for LET network log.
     */
    public static final String KEY_NT_LIMIT_PACKAGE_ENABLER = "networklog_limit_package_enabler";
    public static final String KEY_NT_LIMIT_PACKAGE_SIZE = "networklog_limited_package_size";
    public static final int VALUE_NT_LIMIT_PACKAGE_DEFAULT_SIZE = 90;
    public static final String KEY_NT_ROHC_COMPRESSION_ENABLER = "enable_rohc_compression";
    public static final String KEY_NT_ROHC_TOTAL_FILE_NUMBER = "set_rohc_total_file_number";
    public static final int VALUE_NT_ROHC_TOTAL_FILE_DEFAULT_NUMBER = 10;

    private static final int LIMIT_LOG_SIZE = 100;

    private CheckBoxPreference mNtLogDoPingPre;
    private EditTextPreference mNtLogSizeLimitPre;
    private CheckBoxPreference mNtEnablePackageLimitationPre;
    private EditTextPreference mNtPackageSizeLimitationPre;
    private CheckBoxPreference mNtEnableRohcCompressionPre;
    private EditTextPreference mNtRohcTotalFileNumberPre;

    private SharedPreferences mDefaultSharedPreferences;

    private long mSdcardSize;
    private Toast mToastShowing;

    private SettingsPreferenceFragement mPrefsFragement;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mPrefsFragement =
                SettingsPreferenceFragement.getInstance(this, R.layout.networklog_settings);
        getFragmentManager().beginTransaction().replace(
                android.R.id.content, mPrefsFragement).commit();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    @Override
    public void findViews() {
        mNtLogDoPingPre = (CheckBoxPreference) mPrefsFragement
                .findPreference(Utils.KEY_NETWORK_LOG_DO_PING);
        mNtLogSizeLimitPre = (EditTextPreference) mPrefsFragement.
                findPreference(Utils.KEY_NETWORK_LOG_LOGSIZE);
        mNtEnablePackageLimitationPre = (CheckBoxPreference) mPrefsFragement
                .findPreference(KEY_NT_LIMIT_PACKAGE_ENABLER);
        mNtPackageSizeLimitationPre =
                (EditTextPreference) mPrefsFragement.
                        findPreference(KEY_NT_LIMIT_PACKAGE_SIZE);
        mNtEnableRohcCompressionPre = (CheckBoxPreference) mPrefsFragement
                .findPreference(KEY_NT_ROHC_COMPRESSION_ENABLER);
        mNtRohcTotalFileNumberPre = (EditTextPreference) mPrefsFragement
                .findPreference(KEY_NT_ROHC_TOTAL_FILE_NUMBER);
    }

    @Override
    public void initViews() {
        Utils.logd(TAG, "initViews()");
        this.setTitle(R.string.networklog_settings);
        mDefaultSharedPreferences = PreferenceManager.getDefaultSharedPreferences(this);

        setAllPreferencesEnable(!LogControllerUtils.getLogControllerInstance(
                Utils.LOG_TYPE_NETWORK).isLogRunning());
        mNtLogSizeLimitPre.getEditText().setInputType(InputType.TYPE_CLASS_NUMBER);
        mSdcardSize = getIntent().getLongExtra(Utils.SDCARD_SIZE, LIMIT_LOG_SIZE);

        Object[] objs =
                {
                        getString(R.string.network_log_name),
                        LIMIT_LOG_SIZE,
                        mSdcardSize,
                        getString(Utils.LOG_PATH_TYPE_STRING_MAPS
                                .get(Utils.getCurrentLogPathType())) };
        mNtLogSizeLimitPre
                .setDialogMessage(getString(R.string.limit_log_size_dialog_message, objs));
        mNtLogSizeLimitPre.setSummary(mDefaultSharedPreferences.getString(
                Utils.KEY_LOG_SIZE_MAP.get(Utils.LOG_TYPE_NETWORK),
                String.valueOf(Utils.DEFAULT_CONFIG_LOG_SIZE_MAP.get(Utils.LOG_TYPE_NETWORK)))
                + "MB");

        mNtPackageSizeLimitationPre.getEditText().setInputType(InputType.TYPE_CLASS_NUMBER);
        mNtPackageSizeLimitationPre
                .setDialogMessage(getString(R.string.networklog_limit_package_size_note));
        mNtPackageSizeLimitationPre.setSummary(mDefaultSharedPreferences.getString(
                KEY_NT_LIMIT_PACKAGE_SIZE,
                String.valueOf(VALUE_NT_LIMIT_PACKAGE_DEFAULT_SIZE)) + "Byte");

        mNtRohcTotalFileNumberPre.getEditText().setInputType(InputType.TYPE_CLASS_NUMBER);
        mNtRohcTotalFileNumberPre.setDialogMessage(
                getString(R.string.networklog_set_rohc_total_file_number_summary));
        mNtRohcTotalFileNumberPre.setSummary(mDefaultSharedPreferences.getString(
                KEY_NT_ROHC_TOTAL_FILE_NUMBER,
                String.valueOf(VALUE_NT_ROHC_TOTAL_FILE_DEFAULT_NUMBER)));

        PreferenceScreen advancePreCategory =
                (PreferenceScreen) mPrefsFragement.findPreference("networklog_preference_screen");
        boolean isRohcCompressionSupport =
                NetworkLogController.getInstance().isNetworkLogRohcCompressionSupport();
        if (!isRohcCompressionSupport) {
            advancePreCategory.removePreference(mNtEnableRohcCompressionPre);
            advancePreCategory.removePreference(mNtRohcTotalFileNumberPre);
        }

    }

    @Override
    public void setListeners() {
        Utils.logd(TAG, "setListeners()");
        mNtLogSizeLimitPre.setOnPreferenceChangeListener(this);
        mNtEnablePackageLimitationPre.setOnPreferenceChangeListener(this);
        mNtPackageSizeLimitationPre.setOnPreferenceChangeListener(this);
        mNtEnableRohcCompressionPre.setOnPreferenceChangeListener(this);
        mNtRohcTotalFileNumberPre.setOnPreferenceChangeListener(this);

        mNtLogSizeLimitPre.getEditText().addTextChangedListener(new TextWatcher() {

            @Override
            public void afterTextChanged(Editable editable) {
            }

            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
                Dialog dialog = mNtLogSizeLimitPre.getDialog();
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

        mNtPackageSizeLimitationPre.getEditText().addTextChangedListener(new TextWatcher() {
            @Override
            public void afterTextChanged(Editable editable) {
            }

            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
                Dialog dialog = mNtPackageSizeLimitationPre.getDialog();
                if (dialog != null && dialog instanceof AlertDialog) {
                    if ("".equals(String.valueOf(s))) {
                        ((AlertDialog) dialog).getButton(AlertDialog.BUTTON_POSITIVE).setEnabled(
                                false);
                        return;
                    }

                    try {
                        int inputSize = Integer.parseInt(String.valueOf(s));
                        boolean isEnable = (inputSize >= 0 && inputSize <= 65535);
                        String msg = "Please input a valid integer value ("
                                + 0 + "~" + 65535 + ").";
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

        mNtRohcTotalFileNumberPre.getEditText().addTextChangedListener(new TextWatcher() {
            @Override
            public void afterTextChanged(Editable editable) {
            }

            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
                Dialog dialog = mNtRohcTotalFileNumberPre.getDialog();
                if (dialog != null && dialog instanceof AlertDialog) {
                    if ("".equals(String.valueOf(s))) {
                        ((AlertDialog) dialog).getButton(AlertDialog.BUTTON_POSITIVE)
                                .setEnabled(false);
                        return;
                    }

                    try {
                        int inputSize = Integer.parseInt(String.valueOf(s));
                        boolean isEnable = (inputSize >= 2 && inputSize <= 65535);
                        String msg =
                                "Please input a valid integer value (" + 2 + "~" + 65535 + ").";
                        showToastMsg(isEnable, msg);
                        ((AlertDialog) dialog).getButton(AlertDialog.BUTTON_POSITIVE)
                                .setEnabled(isEnable);
                    } catch (NumberFormatException e) {
                        Utils.loge(TAG, "Integer.parseInt(" + String.valueOf(s) + ") is error!");
                        ((AlertDialog) dialog).getButton(AlertDialog.BUTTON_POSITIVE)
                                .setEnabled(false);
                    }
                }
            }
        });

    }
    private void showToastMsg(boolean isEnable, String msg) {
        if (!isEnable) {
            if (mToastShowing == null) {
                mToastShowing = Toast.makeText(NetworkLogSettings.this, msg,
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
        if (preference.getKey().equals(Utils.KEY_NETWORK_LOG_LOGSIZE)) {
            LogControllerUtils.getLogControllerInstance(Utils.LOG_TYPE_NETWORK)
                    .setLogRecycleSize(getIntByObj(newValue));
            mNtLogSizeLimitPre.setSummary(newValue + "MB");
        } else if (preference.getKey().equals(KEY_NT_LIMIT_PACKAGE_ENABLER)) {
            boolean newValueBoolean = Boolean.parseBoolean(newValue.toString());
            if (newValueBoolean && mNtEnableRohcCompressionPre.isChecked()) {
                mNtEnableRohcCompressionPre.setChecked(false);
                NetworkLogController.getInstance().enableNetworkLogRohcCompression(false);
            }
            mNtPackageSizeLimitationPre.setEnabled(newValueBoolean);
        } else if (preference.getKey().equals(KEY_NT_LIMIT_PACKAGE_SIZE)) {
            mNtPackageSizeLimitationPre.setSummary(newValue + "Byte");
        } else if (preference.getKey().equals(KEY_NT_ROHC_COMPRESSION_ENABLER)) {
            boolean newValueBoolean = Boolean.parseBoolean(newValue.toString());
            if (newValueBoolean && mNtEnablePackageLimitationPre.isChecked()) {
                mNtEnablePackageLimitationPre.setChecked(false);
            }
            NetworkLogController.getInstance()
                    .enableNetworkLogRohcCompression(newValueBoolean);
            mNtRohcTotalFileNumberPre.setEnabled(newValueBoolean);
        } else if (preference.getKey().equals(KEY_NT_ROHC_TOTAL_FILE_NUMBER)) {
            NetworkLogController.getInstance()
                    .setNetworkLogRohcTotalFileNumber(getIntByObj(newValue));
            mNtRohcTotalFileNumberPre.setSummary(String.valueOf(newValue));
        }
        return true;
    }

    private void setAllPreferencesEnable(boolean isEnable) {
        mNtLogDoPingPre.setEnabled(isEnable);
        mNtEnablePackageLimitationPre.setEnabled(isEnable);
        mNtPackageSizeLimitationPre.setEnabled(
                isEnable && mNtEnablePackageLimitationPre.isChecked());
        mNtEnableRohcCompressionPre.setEnabled(isEnable);
        mNtRohcTotalFileNumberPre.setEnabled(isEnable && mNtEnableRohcCompressionPre.isChecked());
        mNtLogSizeLimitPre.setEnabled(isEnable);
    }

    private int getIntByObj(Object obj) {
        try {
            return Integer.parseInt(obj.toString());
        } catch (NumberFormatException e) {
            return 0;
        }
    }

}
