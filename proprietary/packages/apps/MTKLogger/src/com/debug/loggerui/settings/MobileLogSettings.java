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
import android.text.Editable;
import android.text.InputType;
import android.text.TextWatcher;
import android.widget.Toast;

import com.debug.loggerui.R;
import com.debug.loggerui.controller.LogControllerUtils;
import com.debug.loggerui.controller.MobileLogController;
import com.debug.loggerui.framework.DebugLoggerUIServiceManager.ServiceNullException;
import com.debug.loggerui.utils.Utils;
import com.log.handler.LogHandlerUtils.MobileLogSubLog;

import java.util.HashMap;
import java.util.Map;

/**
 * @author MTK81255
 *
 */
public class MobileLogSettings extends PreferenceActivity
                               implements OnPreferenceChangeListener, ISettingsActivity {
    private static final String TAG = Utils.TAG + "/MobileLogSettings";

    public static final String KEY_MB_LOGSIZE = "mobilelog_logsize";
    public static final String KEY_MB_TOTAL_LOGSIZE = "mobilelog_total_logsize";

    private static final int LIMIT_LOG_SIZE = 100;

    private Map<MobileLogSubLog, CheckBoxPreference> mSubLogCheckBoxPreferenceMap =
            new HashMap<MobileLogSubLog, CheckBoxPreference>();
    private EditTextPreference mMbLogSizeLimitPre;
    private EditTextPreference mMbTotalLogSizeLimitPre;
    private int mTotalLogSizeBottom;

    private SharedPreferences mDefaultSharedPreferences;

    private Toast mToastShowing;
    private long mSdcardSize;

    private SettingsPreferenceFragement mPrefsFragement;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mPrefsFragement =
                SettingsPreferenceFragement.getInstance(this, R.layout.mobilelog_settings);
        getFragmentManager().beginTransaction().replace(
                android.R.id.content, mPrefsFragement).commit();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    @Override
    public void findViews() {
        for (MobileLogSubLog mobileLogSubLog : MobileLogSubLog.values()) {
            CheckBoxPreference subLogCheckBoxPreference =
                    (CheckBoxPreference) mPrefsFragement.findPreference(
                            MobileLogController.SUB_LOG_SETTINGS_ID_MAP.get(mobileLogSubLog));
            mSubLogCheckBoxPreferenceMap.put(mobileLogSubLog, subLogCheckBoxPreference);
        }
        mMbLogSizeLimitPre = (EditTextPreference) mPrefsFragement.
                findPreference(KEY_MB_LOGSIZE);
        mMbTotalLogSizeLimitPre = (EditTextPreference) mPrefsFragement.
                findPreference(KEY_MB_TOTAL_LOGSIZE);
    }

    @Override
    public void initViews() {
        Utils.logd(TAG, "initViews()");
        this.setTitle(R.string.mobilelog_settings);
        mDefaultSharedPreferences = PreferenceManager.getDefaultSharedPreferences(this);

        setAllPreferencesEnable(!LogControllerUtils.getLogControllerInstance(
                Utils.LOG_TYPE_MOBILE).isLogRunning());
        doInitForSmartLogging();

        mMbLogSizeLimitPre.getEditText().setInputType(InputType.TYPE_CLASS_NUMBER);
        mSdcardSize = getIntent().getLongExtra(Utils.SDCARD_SIZE, LIMIT_LOG_SIZE);
        // The max mobile log size
        mSdcardSize = 128 * 1024;
        Object[] objs =
                {
                        getString(R.string.mobile_log_name),
                        LIMIT_LOG_SIZE,
                        mSdcardSize,
                        getString(R.string.limit_log_size_store_type,
                                new Object[] { getString(R.string.mobile_log_name) }) };
        mMbLogSizeLimitPre.setDialogMessage(getString(
                R.string.limit_single_log_size_dialog_message, objs));
        mMbLogSizeLimitPre.setSummary(mDefaultSharedPreferences.getString(
                Utils.KEY_LOG_SIZE_MAP.get(Utils.LOG_TYPE_MOBILE),
                String.valueOf(Utils.DEFAULT_CONFIG_LOG_SIZE_MAP.get(Utils.LOG_TYPE_MOBILE)))
                + "MB");

        mTotalLogSizeBottom = getIntByObj(getTotalLogSizeBottom());

        mMbTotalLogSizeLimitPre.getEditText().setInputType(InputType.TYPE_CLASS_NUMBER);
        mMbTotalLogSizeLimitPre.setSummary(mDefaultSharedPreferences.getString(
                Utils.KEY_TOTAL_LOG_SIZE_MAP.get(Utils.LOG_TYPE_MOBILE),
                String.valueOf(Utils.DEFAULT_CONFIG_LOG_SIZE_MAP.get(Utils.LOG_TYPE_MOBILE)))
                + "MB");
        try {
            formatTotalLogSize(mTotalLogSizeBottom);
        } catch (ServiceNullException e) {
            return;
        }
    }

    /**
     * Total log size should not less than current log size limitation, get the
     * minimum allowed value.
     *
     * @return String
     */
    private String getTotalLogSizeBottom() {
        return mDefaultSharedPreferences.getString(KEY_MB_LOGSIZE,
                String.valueOf(Utils.DEFAULT_CONFIG_LOG_SIZE_MAP.get(Utils.LOG_TYPE_MOBILE)));
    }

    @Override
    public void setListeners() {
        Utils.logd(TAG, "setListeners()");
        for (MobileLogSubLog mobileLogSubLog : MobileLogSubLog.values()) {
            mSubLogCheckBoxPreferenceMap.get(mobileLogSubLog).setOnPreferenceChangeListener(this);
        }
        mMbLogSizeLimitPre.setOnPreferenceChangeListener(this);
        mMbTotalLogSizeLimitPre.setOnPreferenceChangeListener(this);

        mMbLogSizeLimitPre.getEditText().addTextChangedListener(new TextWatcher() {

            @Override
            public void afterTextChanged(Editable editable) {
            }

            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
                Dialog dialog = mMbLogSizeLimitPre.getDialog();
                if (dialog != null && dialog instanceof AlertDialog) {
                    if ("".equals(String.valueOf(s))) {
                        ((AlertDialog) dialog).getButton(AlertDialog.BUTTON_POSITIVE).setEnabled(
                                false);
                        return;
                    }

                    try {
                        int inputSize = Integer.parseInt(String.valueOf(s));
                        boolean isEnable = (inputSize >= LIMIT_LOG_SIZE
                         && inputSize <= mSdcardSize
                        );
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

        mMbTotalLogSizeLimitPre.getEditText().addTextChangedListener(new TextWatcher() {

            @Override
            public void afterTextChanged(Editable editable) {
            }

            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
                Dialog dialog = mMbTotalLogSizeLimitPre.getDialog();
                if (dialog != null && dialog instanceof AlertDialog) {
                    if ("".equals(String.valueOf(s))) {
                        ((AlertDialog) dialog).getButton(AlertDialog.BUTTON_POSITIVE).setEnabled(
                                false);
                        return;
                    }

                    try {
                        int inputSize = Integer.parseInt(String.valueOf(s));
                        boolean isEnable = (inputSize >= mTotalLogSizeBottom)
                                && (inputSize <= mSdcardSize)
                                && (inputSize % mTotalLogSizeBottom == 0);

                        String msg = "Please input a valid integer value ( N*"
                                + mTotalLogSizeBottom + " and < " + mSdcardSize + " ).";
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
    }
    private void showToastMsg(boolean isEnable, String msg) {
        if (!isEnable) {
            if (mToastShowing == null) {
                mToastShowing = Toast.makeText(MobileLogSettings.this, msg,
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
        for (MobileLogSubLog mobileLogSubLog : MobileLogSubLog.values()) {
            if (preference.getKey()
                    .equals(MobileLogController.SUB_LOG_SETTINGS_ID_MAP.get(mobileLogSubLog))) {
                MobileLogController.getInstance().setSubLogEnable(
                        Boolean.parseBoolean(newValue.toString()), mobileLogSubLog);
                return true;
            }
        }
        if (preference.getKey().equals(KEY_MB_LOGSIZE)) {
            int newLogSize = getIntByObj(newValue);
            try {
                formatTotalLogSize(newLogSize);
            } catch (ServiceNullException e) {
                Utils.logw(TAG, "Service is null!");
            }
            mTotalLogSizeBottom = newLogSize;
            LogControllerUtils.getLogControllerInstance(Utils.LOG_TYPE_MOBILE)
                .setLogRecycleSize(getIntByObj(newValue));
            mMbLogSizeLimitPre.setSummary(newLogSize + "MB");
        } else if (preference.getKey().equals(KEY_MB_TOTAL_LOGSIZE)) {
            Utils.logd(TAG, "New total log size value: " + newValue);
            MobileLogController.getInstance().setMobileLogTotalRecycleSize(
                    getIntByObj(newValue));
            mMbTotalLogSizeLimitPre.setSummary(newValue + "MB");
        }
        return true;
    }

    private void formatTotalLogSize(int currentLogSize) throws ServiceNullException {
        int oldTotalLogSize =
                getIntByObj(mDefaultSharedPreferences.getString(KEY_MB_TOTAL_LOGSIZE,
                        String.valueOf(Utils.DEFAULT_CONFIG_LOG_SIZE_MAP
                                .get(Utils.LOG_TYPE_MOBILE) * 2)));
        int newTotalLogSize = 0;
        if (mSdcardSize > 0 && currentLogSize > mSdcardSize) {
            String msgStr =
                    "Current size exceed storage capability[" + currentLogSize
                            + "]MB, so log recycle maybe disabled";
            Toast.makeText(this, msgStr, Toast.LENGTH_LONG).show();
            Utils.logi(TAG, msgStr);
        }
        if (currentLogSize > oldTotalLogSize) {
            String msgStr =
                    "Current size exceed total size value, reset total log size value to "
                            + currentLogSize + "MB";
            Toast.makeText(this, msgStr, Toast.LENGTH_LONG).show();
            Utils.logi(TAG, msgStr);
            newTotalLogSize = currentLogSize;
        } else {
            newTotalLogSize = oldTotalLogSize / currentLogSize * currentLogSize;
        }
        mMbTotalLogSizeLimitPre.setText(String.valueOf(newTotalLogSize));
        MobileLogController.getInstance().setMobileLogTotalRecycleSize(
                getIntByObj(newTotalLogSize));
        mMbTotalLogSizeLimitPre.setDialogMessage(getString(
                R.string.limit_total_log_size_dialog_message, new Object[] {
                        getString(R.string.mobile_log_name), String.valueOf(currentLogSize),
                        String.valueOf(mSdcardSize), String.valueOf(currentLogSize)}));
    }

    private void setAllPreferencesEnable(boolean isEnable) {
        for (MobileLogSubLog mobileLogSubLog : MobileLogSubLog.values()) {
            mSubLogCheckBoxPreferenceMap.get(mobileLogSubLog).setEnabled(isEnable);
        }
        mMbLogSizeLimitPre.setEnabled(isEnable);
        mMbTotalLogSizeLimitPre.setEnabled(isEnable);
    }

    private void doInitForSmartLogging() {
        if (Utils.MODEM_MODE_PLS.equals(mDefaultSharedPreferences.getString(Utils.KEY_MD_MODE_1,
                Utils.MODEM_MODE_SD))) {
            for (MobileLogSubLog mobileLogSubLog : MobileLogSubLog.values()) {
                mSubLogCheckBoxPreferenceMap.get(mobileLogSubLog).setEnabled(false);
            }
            mMbLogSizeLimitPre.setEnabled(false);
        }
    }

    private int getIntByObj(Object obj) {
        try {
            return Integer.parseInt(obj.toString());
        } catch (NumberFormatException e) {
            return 0;
        }
    }

}
