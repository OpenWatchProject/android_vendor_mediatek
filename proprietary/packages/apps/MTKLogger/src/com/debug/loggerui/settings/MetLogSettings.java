package com.debug.loggerui.settings;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.preference.CheckBoxPreference;
import android.preference.EditTextPreference;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.Preference.OnPreferenceClickListener;
import android.preference.PreferenceActivity;
import android.preference.PreferenceManager;
import android.text.Editable;
import android.text.InputType;
import android.text.TextWatcher;
import android.util.SparseIntArray;
import android.widget.Toast;

import com.debug.loggerui.R;
import com.debug.loggerui.controller.LogControllerUtils;
import com.debug.loggerui.controller.METLogController;
import com.debug.loggerui.utils.Utils;

/**
 * @author mtk80130
 *
 */
public class MetLogSettings extends PreferenceActivity
                            implements OnPreferenceChangeListener, ISettingsActivity {
    private static final String TAG = Utils.TAG + "/MetLogSettings";
    public static final String KEY_MT_PREFERENCE_SCREEN = "metlog_preference_screen";
    public static final String KEY_MT_CATEGORY = "metlog_category";
    public static final String KEY_MT_PRIOD = "metlog_period";
    public static final String KEY_MT_CPU_BUFFER = "metlog_cpu_buffer";
    public static final String KEY_MT_HEAVY_LOAD_RECORDING = "metlog_heavy_load_recording";

    public static final String KEY_MT_LOGSIZE = "metlog_logsize";
    public static final String KEY_SSPM_LOGSIZE = "sspmlog_logsize";

    private SharedPreferences mDefaultSharedPreferences;

    private ProgressDialog mWaitingDialog;

    private static final int MSG_SHOW_DIALOG = 1;
    private static final int MSG_UPDATE_DEFAULT_VALUES = 2;
    private static final int MSG_DISMISS_DIALOG = 3;

    private EditTextPreference mMtLogPreiodPre;
    private EditTextPreference mMtLogCPUBufferPre;
    private CheckBoxPreference mMtLogHeavyLoadRecordingPre;
    private EditTextPreference mMtLogSizeLimitPre;
    private EditTextPreference mSSPMLogSizeLimitPre;

    private static SparseIntArray sInitValuesMap = new SparseIntArray();
    public static final int KEY_MET_LOG_MAX_PERIOD = 0;
    public static final int KEY_MET_LOG_MIN_PERIOD = 1;
    public static final int KEY_MET_LOG_CURRENT_PERIOD = 2;
    public static final int KEY_MET_LOG_MAX_CPU_BUFFER = 3;
    public static final int KEY_MET_LOG_MIN_CPU_BUFFER = 4;
    public static final int KEY_MET_LOG_CURRENT_CPU_BUFFER = 5;
    public static final int KEY_MET_LOG_MAX_LOG_SIZE = 6;
    public static final int KEY_MET_LOG_MIN_LOG_SIZE = 7;
    public static final int KEY_MET_LOG_CURRENT_LOG_SIZE = 8;
    public static final int KEY_MET_LOG_MAX_SSPM_LOG_SIZE = 9;
    public static final int KEY_MET_LOG_MIN_SSPM_LOG_SIZE = 10;
    public static final int KEY_MET_LOG_CURRENT_SSPM_LOG_SIZE = 11;
    {
        sInitValuesMap.put(KEY_MET_LOG_MAX_PERIOD, 0);
        sInitValuesMap.put(KEY_MET_LOG_MIN_PERIOD, 0);
        sInitValuesMap.put(KEY_MET_LOG_CURRENT_PERIOD, 0);
        sInitValuesMap.put(KEY_MET_LOG_MAX_CPU_BUFFER, 0);
        sInitValuesMap.put(KEY_MET_LOG_MIN_CPU_BUFFER, 0);
        sInitValuesMap.put(KEY_MET_LOG_CURRENT_CPU_BUFFER, 0);
        sInitValuesMap.put(KEY_MET_LOG_MAX_LOG_SIZE, 0);
        sInitValuesMap.put(KEY_MET_LOG_MIN_LOG_SIZE, 0);
        sInitValuesMap.put(KEY_MET_LOG_CURRENT_LOG_SIZE, 0);
        sInitValuesMap.put(KEY_MET_LOG_MAX_SSPM_LOG_SIZE, 0);
        sInitValuesMap.put(KEY_MET_LOG_MIN_SSPM_LOG_SIZE, 0);
        sInitValuesMap.put(KEY_MET_LOG_CURRENT_SSPM_LOG_SIZE, 0);
    }

    private SettingsPreferenceFragement mPrefsFragement;

    private Toast mToastShowing;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mPrefsFragement = SettingsPreferenceFragement.getInstance(this, R.layout.metlog_settings);
        getFragmentManager().beginTransaction().replace(
                android.R.id.content, mPrefsFragement).commit();
    }

    @Override
    public void findViews() {
        Utils.logd(TAG, "findViews()");
        mMtLogPreiodPre = (EditTextPreference) mPrefsFragement.findPreference(KEY_MT_PRIOD);
        mMtLogCPUBufferPre = (EditTextPreference) mPrefsFragement.findPreference(KEY_MT_CPU_BUFFER);
        mMtLogHeavyLoadRecordingPre = (CheckBoxPreference) mPrefsFragement.findPreference(
                KEY_MT_HEAVY_LOAD_RECORDING);
        mMtLogSizeLimitPre = (EditTextPreference) mPrefsFragement.findPreference(KEY_MT_LOGSIZE);
        mSSPMLogSizeLimitPre = (EditTextPreference)
                mPrefsFragement.findPreference(KEY_SSPM_LOGSIZE);
    }

    @Override
    public void initViews() {
        Utils.logd(TAG, "initViews()");
        this.setTitle(R.string.metlog_settings);
        mDefaultSharedPreferences = PreferenceManager.getDefaultSharedPreferences(this);
        setAllPreferencesEnable(!LogControllerUtils.getLogControllerInstance(
                Utils.LOG_TYPE_MET).isLogRunning());
        Utils.logd(TAG, "initViews,get value from native");
        mMtLogPreiodPre.getEditText().setInputType(InputType.TYPE_CLASS_NUMBER);
        mMtLogCPUBufferPre.getEditText().setInputType(InputType.TYPE_CLASS_NUMBER);
        mMtLogSizeLimitPre.getEditText().setInputType(InputType.TYPE_CLASS_NUMBER);
        mMtLogSizeLimitPre.setSummary(mDefaultSharedPreferences.getString(
                Utils.KEY_LOG_SIZE_MAP.get(Utils.LOG_TYPE_MET),
                String.valueOf(Utils.DEFAULT_CONFIG_LOG_SIZE_MAP.get(Utils.LOG_TYPE_MET))));
        mSSPMLogSizeLimitPre.getEditText().setInputType(InputType.TYPE_CLASS_NUMBER);

        Thread getValue = new Thread() {
            @Override
            public void run() {
                initMETLogValues();
            }
        };
        Utils.logd(TAG, "before thread start()");
        mMessageHandler.obtainMessage(MSG_SHOW_DIALOG).sendToTarget();
        getValue.start();
        Utils.logd(TAG, "after thread start()");
    }

    @Override
    public void setListeners() {
        mMtLogPreiodPre.setOnPreferenceChangeListener(this);
        mMtLogCPUBufferPre.setOnPreferenceChangeListener(this);
        mMtLogSizeLimitPre.setOnPreferenceChangeListener(this);
        mSSPMLogSizeLimitPre.setOnPreferenceChangeListener(this);
        mMtLogSizeLimitPre.getEditText().addTextChangedListener(new TextWatcher() {

            @Override
            public void afterTextChanged(Editable editable) {
            }

            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {

                Dialog dialog = mMtLogSizeLimitPre.getDialog();
                if (dialog != null && dialog instanceof AlertDialog) {
                    if ("".equals(String.valueOf(s))) {
                        ((AlertDialog) dialog).getButton(AlertDialog.BUTTON_POSITIVE).setEnabled(
                                false);
                        return;
                    }

                    try {
                        int inputSize = Integer.parseInt(String.valueOf(s));
                        int minLogSize = sInitValuesMap.get(KEY_MET_LOG_MIN_LOG_SIZE);
                        int maxLogSize = sInitValuesMap.get(KEY_MET_LOG_MAX_LOG_SIZE);
                        boolean isEnable = (inputSize >= minLogSize && inputSize <= maxLogSize);
                        ((AlertDialog) dialog).getButton(AlertDialog.BUTTON_POSITIVE).setEnabled(
                                isEnable);
                        String msg = "Please input a valid integer value ("
                                + minLogSize + "~" + maxLogSize + ").";
                        showToastMsg(isEnable, msg);
                    } catch (NumberFormatException e) {
                        Utils.loge(TAG, "Integer.parseInt(" + String.valueOf(s) + ") is error!");
                        ((AlertDialog) dialog).getButton(AlertDialog.BUTTON_POSITIVE).setEnabled(
                                false);
                    }
                }
            }

        });

        mSSPMLogSizeLimitPre.getEditText().addTextChangedListener(new TextWatcher() {

            @Override
            public void afterTextChanged(Editable editable) {
            }

            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {

                Dialog dialog = mSSPMLogSizeLimitPre.getDialog();
                if (dialog != null && dialog instanceof AlertDialog) {
                    if ("".equals(String.valueOf(s))) {
                        ((AlertDialog) dialog).getButton(AlertDialog.BUTTON_POSITIVE).setEnabled(
                                false);
                        return;
                    }

                    try {
                        int inputSize = Integer.parseInt(String.valueOf(s));
                        int minSSPMSize = sInitValuesMap.get(KEY_MET_LOG_MIN_SSPM_LOG_SIZE);
                        int maxSSPMLogSize = sInitValuesMap.get(KEY_MET_LOG_MAX_SSPM_LOG_SIZE);
                        boolean isEnable = (inputSize >= minSSPMSize
                                && inputSize <= maxSSPMLogSize);
                        ((AlertDialog) dialog).getButton(AlertDialog.BUTTON_POSITIVE).setEnabled(
                                isEnable);
                        String msg = "Please input a valid integer value ("
                                + minSSPMSize + "~" + maxSSPMLogSize + ").";
                        showToastMsg(isEnable, msg);
                    } catch (NumberFormatException e) {
                        Utils.loge(TAG, "Integer.parseInt(" + String.valueOf(s) + ") is error!");
                        ((AlertDialog) dialog).getButton(AlertDialog.BUTTON_POSITIVE).setEnabled(
                                false);
                    }
                }
            }

        });

        mMtLogPreiodPre.getEditText().addTextChangedListener(new TextWatcher() {

            @Override
            public void afterTextChanged(Editable s) {
            }

            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
                Dialog dialog = mMtLogPreiodPre.getDialog();
                if (dialog != null && dialog instanceof AlertDialog) {
                    if ("".equals(String.valueOf(s))) {
                        ((AlertDialog) dialog).getButton(AlertDialog.BUTTON_POSITIVE).setEnabled(
                                false);
                        return;
                    }
                    try {
                        int inputSize = Integer.parseInt(String.valueOf(s));
                        int minPeriod = sInitValuesMap.get(KEY_MET_LOG_MIN_PERIOD);
                        int maxPeriod = sInitValuesMap.get(KEY_MET_LOG_MAX_PERIOD);
                        boolean isEnable = (inputSize >= minPeriod && inputSize <= maxPeriod);
                        ((AlertDialog) dialog).getButton(AlertDialog.BUTTON_POSITIVE).setEnabled(
                                isEnable);
                    } catch (NumberFormatException e) {
                        Toast.makeText(MetLogSettings.this,
                                "Please input a valid integer value (less than 2147483648).",
                                Toast.LENGTH_SHORT).show();
                        ((AlertDialog) dialog).getButton(AlertDialog.BUTTON_POSITIVE).setEnabled(
                                false);
                    }
                }
            }

        });
        mMtLogCPUBufferPre.getEditText().addTextChangedListener(new TextWatcher() {
            @Override
            public void afterTextChanged(Editable s) {
                Utils.logd(TAG, "afterTextChanged:" + s);
            }

            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                Utils.logd(TAG, "beforeTextChanged:" + s + "," + "start," + start + "count,"
                        + count + "after," + after);
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
                Utils.logd(TAG, "onTextChanged:" + s + "start," + start + "before," + before
                        + "count," + count);
                Dialog dialog = mMtLogCPUBufferPre.getDialog();
                if (dialog != null && dialog instanceof AlertDialog) {
                    if ("".equals(String.valueOf(s))) {
                        ((AlertDialog) dialog).getButton(AlertDialog.BUTTON_POSITIVE).setEnabled(
                                false);
                        return;
                    }
                    try {
                        int inputSize = Integer.parseInt(String.valueOf(s));
                        int minCpuBuffer = sInitValuesMap.get(KEY_MET_LOG_MIN_CPU_BUFFER);
                        int maxCpuBuffer = sInitValuesMap.get(KEY_MET_LOG_MAX_CPU_BUFFER);
                        boolean isEnable =
                                (inputSize >= minCpuBuffer && inputSize <= maxCpuBuffer);
                        ((AlertDialog) dialog).getButton(AlertDialog.BUTTON_POSITIVE).setEnabled(
                                isEnable);
                    } catch (NumberFormatException e) {
                        Toast.makeText(MetLogSettings.this,
                                "Please input a valid integer value (less than 2147483648).",
                                Toast.LENGTH_SHORT).show();
                        ((AlertDialog) dialog).getButton(AlertDialog.BUTTON_POSITIVE).setEnabled(
                                false);
                    }
                }
            }

        });
        if (mMtLogHeavyLoadRecordingPre != null) {
            mMtLogHeavyLoadRecordingPre.setOnPreferenceClickListener(
                    new OnPreferenceClickListener() {
                @Override
                public boolean onPreferenceClick(Preference arg0) {
                    return METLogController.getInstance().setHeavyLoadRecordingEnable(
                            mMtLogHeavyLoadRecordingPre.isChecked());
                }
            });
        }
    }

    @Override
    protected void onDestroy() {
        if (mWaitingDialog != null) {
            mWaitingDialog.cancel();
            mWaitingDialog = null;
        }
        super.onDestroy();
    }

    private void showToastMsg(boolean isEnable, String msg) {
        if (!isEnable) {
            if (mToastShowing == null) {
                mToastShowing = Toast.makeText(MetLogSettings.this, msg,
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
        if (preference.getKey().equals(KEY_MT_PRIOD)) {
            int currentPeriod = getIntByObj(newValue);
            sInitValuesMap.put(KEY_MET_LOG_CURRENT_PERIOD, currentPeriod);
            METLogController.getInstance().setMETLogPeriod(currentPeriod);
            mMtLogPreiodPre.setText(String.valueOf(currentPeriod));
        } else if (preference.getKey().equals(KEY_MT_CPU_BUFFER)) {
            int currentCpuBuffer = getIntByObj(newValue);
            sInitValuesMap.put(KEY_MET_LOG_CURRENT_CPU_BUFFER, currentCpuBuffer);
            METLogController.getInstance().setMETLogCPUBuffer(currentCpuBuffer);
            mMtLogCPUBufferPre.setText(String.valueOf(currentCpuBuffer));
        } else if (preference.getKey().equals(KEY_MT_LOGSIZE)) {
            int currentLogsize = getIntByObj(newValue);
            sInitValuesMap.put(KEY_MET_LOG_CURRENT_LOG_SIZE, currentLogsize);
            LogControllerUtils.getLogControllerInstance(Utils.LOG_TYPE_MET)
                .setLogRecycleSize(currentLogsize);
            mMtLogSizeLimitPre.setText(String.valueOf(currentLogsize));
        } else if (preference.getKey().equals(KEY_SSPM_LOGSIZE)) {
            int currentSSPMLogsize = getIntByObj(newValue);
            sInitValuesMap.put(KEY_MET_LOG_CURRENT_SSPM_LOG_SIZE, currentSSPMLogsize);
            METLogController.getInstance().setMETLogSSPMSize(currentSSPMLogsize);
            mSSPMLogSizeLimitPre.setText(String.valueOf(currentSSPMLogsize));
        }
        return true;

    }

    private void setAllPreferencesEnable(boolean isEnable) {
        mMtLogPreiodPre.setEnabled(isEnable);
        mMtLogCPUBufferPre.setEnabled(isEnable);
        mMtLogSizeLimitPre.setEnabled(isEnable);
        mSSPMLogSizeLimitPre.setEnabled(isEnable);

    }

    private int getIntByObj(Object obj) {
        try {
            return Integer.parseInt(obj.toString());
        } catch (NumberFormatException e) {
            return 0;
        }
    }

    private void showWaitingDialog() {
        Utils.logd(TAG, "showWaitingDialog()");
        mWaitingDialog =
                ProgressDialog.show(MetLogSettings.this, getString(R.string.met_log_name),
                        "get value from Met native layer", true, false);
    }

    private void releaseDialog() {
        Utils.logd(TAG, "releaseDialog()");
        if (mWaitingDialog != null) {
            mWaitingDialog.cancel();
            mWaitingDialog = null;
        }

    }

    private Handler mMessageHandler = new Handler(Looper.getMainLooper()) {
        @Override
        public void handleMessage(Message message) {
            Utils.logd(TAG, " MyHandler handleMessage --> start " + message.what);
            switch (message.what) {
            case MSG_SHOW_DIALOG:
                showWaitingDialog();
                break;
            case MSG_UPDATE_DEFAULT_VALUES:
                Object[] cpuBufferObjs =
                    { getString(R.string.met_log_name),
                            sInitValuesMap.get(KEY_MET_LOG_MIN_CPU_BUFFER),
                            sInitValuesMap.get(KEY_MET_LOG_MAX_CPU_BUFFER)};
                mMtLogCPUBufferPre.setDialogMessage(
                        getString(R.string.limit_cpu_buffer_dialog_message,
                        cpuBufferObjs));
                mMtLogCPUBufferPre.setText(String.valueOf(
                        sInitValuesMap.get(KEY_MET_LOG_CURRENT_CPU_BUFFER)));

                Object[] periodObjs = {
                        getString(R.string.met_log_name),
                        sInitValuesMap.get(KEY_MET_LOG_MIN_PERIOD),
                        sInitValuesMap.get(KEY_MET_LOG_MAX_PERIOD)};
                mMtLogPreiodPre.setDialogMessage(getString(R.string.limit_period_dialog_message,
                        periodObjs));
                mMtLogPreiodPre.setText(String.valueOf(
                        sInitValuesMap.get(KEY_MET_LOG_CURRENT_PERIOD)));

                Object[] logSizeObjs = {
                        getString(R.string.met_log_name),
                        sInitValuesMap.get(KEY_MET_LOG_MIN_LOG_SIZE),
                        sInitValuesMap.get(KEY_MET_LOG_MAX_LOG_SIZE)};
                mMtLogSizeLimitPre.setDialogMessage(
                        getString(R.string.limit_met_log_size_dialog_message,
                        logSizeObjs));
                mMtLogSizeLimitPre.setText(String.valueOf(
                        sInitValuesMap.get(KEY_MET_LOG_CURRENT_LOG_SIZE)));

                Object[] sspmLogSizeObjs = {
                        sInitValuesMap.get(KEY_MET_LOG_MIN_SSPM_LOG_SIZE),
                        sInitValuesMap.get(KEY_MET_LOG_MAX_SSPM_LOG_SIZE)};
                mSSPMLogSizeLimitPre.setDialogMessage(
                        getString(R.string.limit_sspm_log_size_dialog_message,
                                sspmLogSizeObjs));
                mSSPMLogSizeLimitPre.setText(String.valueOf(
                        sInitValuesMap.get(KEY_MET_LOG_CURRENT_SSPM_LOG_SIZE)));
                mMessageHandler.obtainMessage(MSG_DISMISS_DIALOG).sendToTarget();
                break;
            case MSG_DISMISS_DIALOG:
                releaseDialog();
                break;
            default:
                Utils.logw(TAG, "Not supported message: " + message.what);
                break;
            }
        }
    };

    private void initMETLogValues() {
        String initValueStr = METLogController.getInstance().getInitValues();
        Utils.logw(TAG, "initMETLogValues() initValueStr = " + initValueStr);
        String[] initValues = initValueStr.split(",");
        int maxIndex = initValues.length;
        if (initValues.length > sInitValuesMap.size()) {
            maxIndex = sInitValuesMap.size();
        }
        if (initValues.length != sInitValuesMap.size()) {
            Utils.logw(TAG, "Init METLog value format is error! initValueStr = " + initValueStr);
        }
        for (int i = 0; i < maxIndex; i++) {
            int key = sInitValuesMap.keyAt(i);
            int value = 0;
            try {
                value = Integer.parseInt(initValues[i]);
            } catch (NumberFormatException nfe) {
                value = 0;
            }
            sInitValuesMap.put(key, value);
        }
        mMessageHandler.obtainMessage(MSG_UPDATE_DEFAULT_VALUES).sendToTarget();
    }
}
