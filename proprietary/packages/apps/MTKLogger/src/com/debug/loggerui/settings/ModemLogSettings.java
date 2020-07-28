package com.debug.loggerui.settings;

import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.SystemProperties;
import android.preference.CheckBoxPreference;
import android.preference.EditTextPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.PreferenceActivity;
import android.preference.PreferenceManager;
import android.preference.PreferenceScreen;
import android.text.Editable;
import android.text.InputType;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.view.WindowManager;
import android.widget.Toast;

import com.debug.loggerui.R;
import com.debug.loggerui.controller.LogControllerUtils;
import com.debug.loggerui.controller.MobileLogController;
import com.debug.loggerui.controller.ModemLogController;
import com.debug.loggerui.framework.DebugLoggerUIServiceManager.ServiceNullException;
import com.debug.loggerui.utils.Utils;
import com.log.handler.LogHandlerUtils.MobileLogSubLog;

/**
 * @author MTK81255
 *
 */
public class ModemLogSettings extends PreferenceActivity
        implements OnPreferenceChangeListener, ISettingsActivity {
    private static final String TAG = Utils.TAG + "/ModemLogSettings";

    public static final String KEY_MD_PREFERENCE_SCREEN = "modemlog_preference_screen";
    public static final String KEY_MD_MODE = "log_mode";
    public static final String KEY_MD_LOGSIZE = "modemlog_logsize";
    public static final String KEY_MD_AUTORESET = "modem_autoreset";
    public static final String KEY_MD_MONITOR_MODEM_ABNORMAL_EVENT = "monitor_modme_abnormal_event";
    public static final String KEY_MD_SAVE_LOCATIN_IN_LOG = "save_location_in_log";
    public static final String KEY_CCB_GEAR = "ccb_gear";

    private static final int LIMIT_LOG_SIZE = 600;

    private ListPreference mMdLogModeList1;
    private ListPreference mMdLogModeList2;
    private EditTextPreference mMdLogSizeLimitPre;
    private CheckBoxPreference mMdMonitorAbnormalEventPre;
    private CheckBoxPreference mMdSavelocationInLogPre;
    private ListPreference mCCBGearList;
    private CheckBoxPreference mMdAutoResetPre;

    private SharedPreferences mDefaultSharedPreferences;

    private long mSdcardSize;

    private Toast mToastShowing;

    private SettingsPreferenceFragement mPrefsFragement;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mPrefsFragement = SettingsPreferenceFragement.getInstance(this, R.layout.modemlog_settings);
        getFragmentManager().beginTransaction().replace(android.R.id.content, mPrefsFragement)
                .commit();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    @Override
    public void findViews() {
        mMdLogModeList1 = (ListPreference) mPrefsFragement.findPreference(Utils.KEY_MD_MODE_1);
        mMdLogModeList2 = (ListPreference) mPrefsFragement.findPreference(Utils.KEY_MD_MODE_2);
        // distinction for PLS mode in 3G 4G modem
        // use [init.svc.mdlogger]: [stopped] to decide mdlogger running or not
        String hardware = SystemProperties.get("ro.hardware");
        Utils.logd(TAG, "current hardware is " + hardware.substring(0, 4));
        // chip is mt65xx not mt67xx
        if (((hardware.substring(0, 4)).compareToIgnoreCase("mt65") <= 0) &&
        // mdlogger is running
                ((SystemProperties.get("init.svc.mdlogger", "stopped"))
                        .compareToIgnoreCase("running")) == 0
                || Utils.isDenaliMd3Solution()) {
            CharSequence[] mode = mMdLogModeList1.getEntries();
            if (mode.length >= 3) { // if just SD and USB,not reset
                CharSequence[] modifyMode = new CharSequence[mode.length - 1];
                for (int i = 0; i < mode.length - 1; i++) {
                    Utils.logd(TAG, "findViews()================>" + mode[i]);
                    modifyMode[i] = mode[i];
                }
                if (mMdLogModeList1
                        .findIndexOfValue(mMdLogModeList1.getValue()) >= modifyMode.length) {
                    mMdLogModeList1.setValueIndex(modifyMode.length - 1);
                }
                if (mMdLogModeList2
                        .findIndexOfValue(mMdLogModeList2.getValue()) >= modifyMode.length) {
                    mMdLogModeList2.setValueIndex(modifyMode.length - 1);
                }
                mMdLogModeList1.setEntries(modifyMode);
                mMdLogModeList2.setEntries(modifyMode);
            }
        }
        // end
        mCCBGearList = (ListPreference) mPrefsFragement.findPreference(KEY_CCB_GEAR);
        mMdLogSizeLimitPre = (EditTextPreference) mPrefsFragement.findPreference(KEY_MD_LOGSIZE);

        mMdMonitorAbnormalEventPre = (CheckBoxPreference) mPrefsFragement
                .findPreference(KEY_MD_MONITOR_MODEM_ABNORMAL_EVENT);
        mMdSavelocationInLogPre = (CheckBoxPreference) mPrefsFragement
                .findPreference(KEY_MD_SAVE_LOCATIN_IN_LOG);
        mMdAutoResetPre = (CheckBoxPreference) mPrefsFragement
                .findPreference(KEY_MD_AUTORESET);
    }

    @Override
    public void initViews() {
        Utils.logd(TAG, "initViews()");
        this.setTitle(R.string.modemlog_settings);
        mDefaultSharedPreferences = PreferenceManager.getDefaultSharedPreferences(this);

        initLogMode(mMdLogModeList1, Utils.KEY_MD_MODE_1, "Md"
                + (Utils.sAvailableModemList.size() >= 1 ? Utils.sAvailableModemList.get(0) : ""));
        Utils.logd(TAG, "Utils.getModemSize(), " + Utils.sAvailableModemList.size());
        if (Utils.sAvailableModemList.size() == 2) {
            initLogMode(mMdLogModeList2, Utils.KEY_MD_MODE_2,
                    "Md" + Utils.sAvailableModemList.get(1));
        } else {
            PreferenceScreen preferenceScreen =
                    (PreferenceScreen) mPrefsFragement.findPreference(KEY_MD_PREFERENCE_SCREEN);
            preferenceScreen.removePreference(mMdLogModeList2);
        }
        setAllPreferencesEnable(!LogControllerUtils.getLogControllerInstance(
                Utils.LOG_TYPE_MODEM).isLogRunning());
        mMdLogSizeLimitPre.getEditText().setInputType(InputType.TYPE_CLASS_NUMBER);
        mSdcardSize = getIntent().getLongExtra(Utils.SDCARD_SIZE, LIMIT_LOG_SIZE);

        Object[] objs = { getString(R.string.modem_log_name), LIMIT_LOG_SIZE, mSdcardSize,
                getString(Utils.LOG_PATH_TYPE_STRING_MAPS.get(Utils.getCurrentLogPathType())) };
        mMdLogSizeLimitPre
                .setDialogMessage(getString(R.string.limit_log_size_dialog_message, objs));
        mMdLogSizeLimitPre.setSummary(mDefaultSharedPreferences.getString(
                Utils.KEY_LOG_SIZE_MAP.get(Utils.LOG_TYPE_MODEM),
                String.valueOf(Utils.DEFAULT_CONFIG_LOG_SIZE_MAP.get(Utils.LOG_TYPE_MODEM)))
                + "MB");

        PreferenceScreen advancePreCategory =
                (PreferenceScreen) mPrefsFragement.findPreference("modemlog_preference_screen");
        boolean isGpsLocationSupport =
                ModemLogController.getInstance().isSaveGPSLocationFeatureSupport();
        if (!isGpsLocationSupport) {
            advancePreCategory.removePreference(mMdSavelocationInLogPre);
        }

        boolean isCCBFeatureSupport = ModemLogController.getInstance().isCCBBufferFeatureSupport();
        if (!isCCBFeatureSupport) {
            advancePreCategory.removePreference(mCCBGearList);
        } else {
            initCCBGearList();
        }
    }

    private void initCCBGearList() {
        String ccbBufferConfigureListStr =
                ModemLogController.getInstance().getCCBBufferConfigureList();
        Utils.logi(TAG,
                "initCCBGearList(), ccbBufferConfigureListStr = " + ccbBufferConfigureListStr);
        if (ccbBufferConfigureListStr == null || "".equals(ccbBufferConfigureListStr)
                || !ccbBufferConfigureListStr.matches("^[\\d+\\(\\d+,\\d+\\);*]+$")) {
            ccbBufferConfigureListStr = "0(2,20);2(2,10);3(0,0)";
        }
        String[] ccbBufferConfigureList = ccbBufferConfigureListStr.split(";");
        String[] gearIdList = new String[ccbBufferConfigureList.length];
        for (int i = 0; i < ccbBufferConfigureList.length; i++) {
            int index = ccbBufferConfigureList[i].indexOf("(");
            if (index < 0) {
                continue;
            }
            gearIdList[i] =
                    ccbBufferConfigureList[i].substring(0, ccbBufferConfigureList[i].indexOf("("));
            ccbBufferConfigureList[i] =
                    ("gear " + ccbBufferConfigureList[i]).replace(",", "MB + ").replace(")", "MB)");
        }
        mCCBGearList.setEntries(ccbBufferConfigureList);
        mCCBGearList.setEntryValues(gearIdList);

        mCCBGearList.setOnPreferenceChangeListener(this);
        String currentGearId = ModemLogController.getInstance().getCCBBufferGearID();
        Utils.logi(TAG, "initCCBGearList(), currentGearId = " + currentGearId);
        mCCBGearList.setValue(currentGearId);
        mCCBGearList.setSummary(mCCBGearList.getEntry());
    }

    /**
     * @param mdLogModeList
     *            ListPreference
     * @param keyMdMode
     *            String
     */
    private void initLogMode(ListPreference mdLogModeList, String keyMdMode, String mdName) {
        String logModeValue = mDefaultSharedPreferences.getString(keyMdMode, "");
        Utils.logd(TAG, "mDefaultSharedPreferences.getString(KEY_MD_MODE, " + logModeValue);
        if (TextUtils.isEmpty(logModeValue)) {
            logModeValue = getString(R.string.default_mode_value);
            Utils.logw(TAG, "No default log mode value stored in default shared preference, "
                    + "try to get it from string res, logModeValue=" + logModeValue);
        }
        int selectedMode = mdLogModeList.findIndexOfValue(logModeValue);
        if (selectedMode < 0) {
            Utils.loge(TAG, "Fail to select the given mode, just take the first one.");
        }
        mdLogModeList.setTitle(mdName + " " + mdLogModeList.getTitle());
        mdLogModeList.setDialogTitle(mdName + " " + mdLogModeList.getDialogTitle());
        mdLogModeList.setValueIndex(selectedMode);
        mdLogModeList.setSummary(mdLogModeList.getEntries()[selectedMode]);
        mdLogModeList.setOnPreferenceChangeListener(this);
    }

    @Override
    public void setListeners() {
        Utils.logd(TAG, "setListeners()");
        mMdLogSizeLimitPre.setOnPreferenceChangeListener(this);
        mMdSavelocationInLogPre.setOnPreferenceChangeListener(this);

        mMdLogSizeLimitPre.getEditText().addTextChangedListener(new TextWatcher() {

            @Override
            public void afterTextChanged(Editable editable) {
            }

            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
                Dialog dialog = mMdLogSizeLimitPre.getDialog();
                if (dialog != null && dialog instanceof AlertDialog) {
                    if ("".equals(String.valueOf(s))) {
                        ((AlertDialog) dialog).getButton(AlertDialog.BUTTON_POSITIVE)
                                .setEnabled(false);
                        return;
                    }

                    try {
                        int inputSize = Integer.parseInt(String.valueOf(s));
                        boolean isEnable =
                                (inputSize >= LIMIT_LOG_SIZE && inputSize <= mSdcardSize);

                        String msg = "Please input a valid integer value (" + LIMIT_LOG_SIZE + "~"
                                + mSdcardSize + ").";

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
                mToastShowing = Toast.makeText(ModemLogSettings.this, msg, Toast.LENGTH_LONG);
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
        Utils.logi(TAG,
                "Preference Change Key : " + preference.getKey() + " newValue : " + newValue);
        try {
            if (preference.getKey().equals(Utils.KEY_MD_MODE_1)) {
                return logModePreferenceChange(mMdLogModeList1, (String) newValue,
                        Utils.KEY_MD_MODE_1);
            } else if (preference.getKey().equals(Utils.KEY_MD_MODE_2)) {
                return logModePreferenceChange(mMdLogModeList2, (String) newValue,
                        Utils.KEY_MD_MODE_2);
            } else if (preference.getKey().equals(KEY_CCB_GEAR)) {
                ModemLogController.getInstance().setBootupLogSaved(true);
                ModemLogController.getInstance().setCCBBufferGearID((String) newValue);
                mCCBGearList.setValue((String) newValue);
                mCCBGearList.setSummary(mCCBGearList.getEntry());
                showCCBGearWarningDialog("3".equals(newValue));
            } else if (preference.getKey().equals(KEY_MD_LOGSIZE)) {
                LogControllerUtils.getLogControllerInstance(Utils.LOG_TYPE_MODEM)
                        .setLogRecycleSize(getIntByObj(newValue));
                mMdLogSizeLimitPre.setSummary(newValue + "MB");
            } else if (preference.getKey().equals(KEY_MD_SAVE_LOCATIN_IN_LOG)) {
                ModemLogController.getInstance()
                        .setSaveGPSLocationToModemLog(Boolean.parseBoolean(newValue.toString()));
                boolean isCheck = Boolean.parseBoolean(newValue.toString());
                if (isCheck) {
                    showSaveLocationWarningDialog();
                }
            }
        } catch (ServiceNullException e) {
            return false;
        }
        return true;
    }

    private boolean logModePreferenceChange(ListPreference mdLogModeList, String newValue,
            String keyMdMode) throws ServiceNullException {
        String oldModeValue = mdLogModeList.getValue();
        int oldSelectedMode = mdLogModeList.findIndexOfValue(oldModeValue);
        String logModeValue = newValue;
        int selectedMode = mdLogModeList.findIndexOfValue(logModeValue);
        if (selectedMode < 0) {
            Utils.loge(TAG, "Fail to select the given mode, ignore.");
            return false;
        }
        if (selectedMode == oldSelectedMode) {
            Utils.loge(TAG, "The new selected mode is the same as old, ignore");
            return false;
        }

        mdLogModeList.setSummary(mdLogModeList.getEntries()[selectedMode]);
        Utils.logd(TAG, "preference.getKey().equals(KEY_MD_MODE) mode=" + selectedMode);
        if (selectedMode == 2 || oldSelectedMode == 2) {
            if (keyMdMode.equalsIgnoreCase(Utils.KEY_MD_MODE_1)) {
                mMdLogModeList2.setValueIndex(selectedMode);
                mMdLogModeList2.setSummary(mdLogModeList.getEntries()[selectedMode]);
            } else {
                mMdLogModeList1.setValueIndex(selectedMode);
                mMdLogModeList1.setSummary(mdLogModeList.getEntries()[selectedMode]);
            }
        }
        mMdMonitorAbnormalEventPre.setEnabled(selectedMode == 2);
        setMobileLogSmartLoggingStatus(selectedMode, oldSelectedMode);

        // Make mode log mode take effect right now
        ModemLogController.getInstance().setBootupLogSaved(true);
        return true;
    }

    /*
     * If modem is in Passive Log mode, mobile log only need tag android_log, kernal_log, and echo
     * type logs only 10M
     */
    private void setMobileLogSmartLoggingStatus(int selectedMode, int oldSelectedMode)
            throws ServiceNullException {
        Utils.logd(TAG, "setMobileLogSmartLoggingStatus()");
        if (selectedMode != 2) {
            if (oldSelectedMode == 2) {
                LogControllerUtils.getLogControllerInstance(Utils.LOG_TYPE_MOBILE)
                        .setLogRecycleSize(
                                Utils.DEFAULT_CONFIG_LOG_SIZE_MAP.get(Utils.LOG_TYPE_MOBILE));
                mDefaultSharedPreferences.edit()
                        .putString(MobileLogSettings.KEY_MB_LOGSIZE, String.valueOf(
                                Utils.DEFAULT_CONFIG_LOG_SIZE_MAP.get(Utils.LOG_TYPE_MOBILE)))
                        .apply();
            }
            return;
        } else {
            for (MobileLogSubLog mobileLogSubLog : MobileLogSubLog.values()) {
                if (mobileLogSubLog == MobileLogSubLog.AndroidLog
                        || mobileLogSubLog == MobileLogSubLog.KernelLog) {
                    continue;
                }
                mDefaultSharedPreferences.edit()
                        .putBoolean(
                                MobileLogController.SUB_LOG_SETTINGS_ID_MAP.get(mobileLogSubLog),
                                false)
                        .apply();
                MobileLogController.getInstance().setSubLogEnable(false, mobileLogSubLog);
            }
            // 40M = radio + event + main + kernal log
            LogControllerUtils.getLogControllerInstance(Utils.LOG_TYPE_MOBILE)
                    .setLogRecycleSize(40);
            mDefaultSharedPreferences.edit()
                    .putString(MobileLogSettings.KEY_MB_LOGSIZE, String.valueOf(40)).apply();
        }
    }

    private void setAllPreferencesEnable(boolean isEnable) {
        mMdLogModeList1.setEnabled(isEnable);
        mMdLogModeList2.setEnabled(isEnable);
        mMdMonitorAbnormalEventPre.setEnabled(isEnable && (mMdLogModeList1.findIndexOfValue(
                mMdLogModeList1.getValue()) == 2));
        mMdSavelocationInLogPre.setEnabled(isEnable);
        mCCBGearList.setEnabled(isEnable);
        mMdAutoResetPre.setEnabled(isEnable);
        mMdLogSizeLimitPre.setEnabled(isEnable);
    }

    private int getIntByObj(Object obj) {
        try {
            return Integer.parseInt(obj.toString());
        } catch (NumberFormatException e) {
            return 0;
        }
    }

    private void showCCBGearWarningDialog(boolean isNeedShowMoreWarningInfo) {
        Utils.logi(TAG, "showCCBGearWarningDialog()." + " isNeedShowMoreWarningInfo = "
                + isNeedShowMoreWarningInfo);
        String message =
                getString(R.string.ccb_gear_warning_dialog_context) + (isNeedShowMoreWarningInfo
                        ? "\n" + getString(R.string.ccb_gear_warning_dialog_context_more) : "");
        Builder builder = new AlertDialog.Builder(this)
                .setTitle(R.string.ccb_gear_warning_dialog_title).setMessage(message)
                .setPositiveButton(android.R.string.yes, new OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                    }
                });

        AlertDialog dialog = builder.create();
        dialog.getWindow().setType(WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY);
        dialog.show();
    }

    private void showSaveLocationWarningDialog() {
        Utils.logi(TAG, "Show save location warning dialog.");
        String message = getString(R.string.save_location_in_log_dialog_context);
        Builder builder = new AlertDialog.Builder(this)
                .setTitle(R.string.save_location_in_log_dialog_title).setMessage(message)
                .setPositiveButton(android.R.string.yes, new OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                    }
                });

        AlertDialog dialog = builder.create();
        dialog.getWindow().setType(WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY);
        dialog.show();
    }
}
