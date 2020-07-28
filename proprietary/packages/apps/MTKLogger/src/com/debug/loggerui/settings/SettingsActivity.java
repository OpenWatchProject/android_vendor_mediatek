package com.debug.loggerui.settings;

import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.StatFs;
import android.preference.CheckBoxPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.Preference.OnPreferenceClickListener;
import android.preference.PreferenceActivity;
import android.preference.PreferenceCategory;
import android.preference.PreferenceManager;
import android.util.SparseArray;
import android.view.View;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemLongClickListener;
import android.widget.ListView;

import com.debug.loggerui.R;
import com.debug.loggerui.controller.LogControllerUtils;
import com.debug.loggerui.controller.METLogController;
import com.debug.loggerui.controller.ModemLogController;
import com.debug.loggerui.framework.DebugLoggerUIServiceManager;
import com.debug.loggerui.framework.DebugLoggerUIServiceManager.ServiceNullException;
import com.debug.loggerui.permission.PermissionUtils;
import com.debug.loggerui.utils.SelfdefinedSwitchPreference;
import com.debug.loggerui.utils.Utils;

import java.util.ArrayList;
import java.util.List;

/**
 * @author MTK81255
 *
 */
public class SettingsActivity extends PreferenceActivity
        implements OnPreferenceChangeListener, ISettingsActivity {

    private static final String TAG = Utils.TAG + "/SettingsActivity";
    public static final String KEY_MB_SWITCH = "mobilelog_switch";
    public static final String KEY_MD_SWITCH = "modemlog_switch";
    public static final String KEY_NT_SWITCH = "networklog_switch";
    public static final String KEY_MT_SWITCH = "metlog_switch";
    public static final String KEY_GPS_SWITCH = "gpslog_switch";
    public static final String KEY_CONNSYS_SWITCH = "connsyslog_switch";
    public static final String KEY_BTHOST_SWITCH = "bthostlog_switch";
    public static final SparseArray<String> KEY_LOG_SWITCH_MAP = new SparseArray<String>();
    static {
        KEY_LOG_SWITCH_MAP.put(Utils.LOG_TYPE_MOBILE, KEY_MB_SWITCH);
        KEY_LOG_SWITCH_MAP.put(Utils.LOG_TYPE_MODEM, KEY_MD_SWITCH);
        KEY_LOG_SWITCH_MAP.put(Utils.LOG_TYPE_NETWORK, KEY_NT_SWITCH);
        KEY_LOG_SWITCH_MAP.put(Utils.LOG_TYPE_MET, KEY_MT_SWITCH);
        KEY_LOG_SWITCH_MAP.put(Utils.LOG_TYPE_CONNSYS, KEY_CONNSYS_SWITCH);
        KEY_LOG_SWITCH_MAP.put(Utils.LOG_TYPE_GPSHOST, KEY_GPS_SWITCH);
        KEY_LOG_SWITCH_MAP.put(Utils.LOG_TYPE_CONNSYSFW, KEY_CONNSYS_SWITCH);
        KEY_LOG_SWITCH_MAP.put(Utils.LOG_TYPE_BTHOST, KEY_BTHOST_SWITCH);
    }
    public static final SparseArray<Class<?>> LOG_SWITCH_CLASS_MAP = new SparseArray<Class<?>>();
    static {
        LOG_SWITCH_CLASS_MAP.put(Utils.LOG_TYPE_MOBILE, MobileLogSettings.class);
        LOG_SWITCH_CLASS_MAP.put(Utils.LOG_TYPE_MODEM, ModemLogSettings.class);
        LOG_SWITCH_CLASS_MAP.put(Utils.LOG_TYPE_NETWORK, NetworkLogSettings.class);
        LOG_SWITCH_CLASS_MAP.put(Utils.LOG_TYPE_CONNSYS, ConnsysLogSettings.class);
        LOG_SWITCH_CLASS_MAP.put(Utils.LOG_TYPE_MET, MetLogSettings.class);
    }
    private SparseArray<SelfdefinedSwitchPreference> mLogSwitchPreMap =
            new SparseArray<SelfdefinedSwitchPreference>();

    public static final String KEY_ADVANCED_SETTINGS_CATEGORY = "advanced_settings_category";
    public static final String KEY_GENERAL_SETTINGS_CATEGORY = "general_settings_category";
    public static final String KEY_TAGLOG_ENABLE = "taglog_enable";
    public static final String KEY_ALWAYS_TAG_MODEM_LOG_ENABLE = "always_tag_modem_log_enable";
    public static final String KEY_ADVANCED_LOG_STORAGE_LOCATION = "log_storage_location";
    public static final String KEY_MET_LOG_ENABLE = "metlog_enable";

    private CheckBoxPreference mTaglogEnable;
    private CheckBoxPreference mAlwaysTagModemLogEnable;
    private ListPreference mLogStorageLocationList;
    private CheckBoxPreference mMetLogEnable;

    private SharedPreferences mDefaultSharedPreferences;
    private SharedPreferences mSharedPreferences;
    private String mFilterFileInfoStr = null;

    private long mSdcardSize = 0;

    private boolean mIsRecording = false;

    private SettingsPreferenceFragement mPrefsFragement;

    private UpdateLogStorageEntriesTask mUpdateLogStorageEntriesTask =
            new UpdateLogStorageEntriesTask();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Utils.logd(TAG, "SettingsActivity onCreate() start");
        super.onCreate(savedInstanceState);
        mPrefsFragement = SettingsPreferenceFragement.getInstance(this, R.xml.settings);
        getFragmentManager().beginTransaction().replace(android.R.id.content, mPrefsFragement)
                .commit();
        mFilterFileInfoStr = null;
        new Thread(new Runnable() {
            @Override
            public void run() {
                mFilterFileInfoStr = ModemLogController.getInstance().getFilterFileInformation();
            }
        }).start();
        getSdcardSize();
    }

    private ListView mListView = null;

    @Override
    protected void onResume() {
        Utils.logd(TAG, "onResume()");
        if (mListView == null) {
            mListView = mPrefsFragement.getListView();
            mListView.setOnItemLongClickListener(new OnItemLongClickListener() {
                @Override
                public boolean onItemLongClick(AdapterView<?> parent, View view, int position,
                        long id) {
                    Object item = mListView.getItemAtPosition(position);
                    SelfdefinedSwitchPreference mdSwitchPre =
                            mLogSwitchPreMap.get(Utils.LOG_TYPE_MODEM);
                    if (mdSwitchPre.equals(item)) {
                        SelfdefinedSwitchPreference sSwitchPre = (SelfdefinedSwitchPreference) item;
                        Utils.logd(TAG, "onCreateContextMenu sSwitchPre.getTitle() = "
                                + sSwitchPre.getTitle());
                        if (mdSwitchPre.equals(sSwitchPre)) {
                            Utils.logi(TAG, "mFilterFileInfoStr = " + mFilterFileInfoStr);
                            String[] filterInfos = new String[] { "N/A", "N/A", "N/A" };
                            if (mFilterFileInfoStr == null || mFilterFileInfoStr.isEmpty()) {
                                Utils.logw(TAG, "The format for mFilterFileInfoStr is error!");
                            } else {
                                String[] filterInfoStrs = mFilterFileInfoStr.split(";");
                                int length = filterInfoStrs.length > 3 ? 3 : filterInfoStrs.length;
                                for (int i = 0; i < length; i++) {
                                    filterInfos[i] = filterInfoStrs[i];
                                }
                            }
                            String lineSeparator = System.getProperty("line.separator", "/n");
                            String filePath = getString(R.string.file_info_path) + lineSeparator
                                    + filterInfos[0] + lineSeparator + lineSeparator;
                            String modifiedTime =
                                    getString(R.string.file_info_modified_time) + lineSeparator
                                            + filterInfos[1] + lineSeparator + lineSeparator;
                            String fileSize = getString(R.string.file_info_size) + lineSeparator
                                    + filterInfos[2];
                            showLogSettingsInfoDialog(mdSwitchPre.getTitle().toString(),
                                    filePath + modifiedTime + fileSize);
                        }
                    }
                    return true;
                }
            });
        }
        updateUI();
        super.onResume();
    }

    private void showLogSettingsInfoDialog(String titile, String inforStr) {
        Utils.logi(TAG,
                "showLogSettingsInfoDialog titile = " + titile + ", inforStr = " + inforStr);
        Builder builder = new AlertDialog.Builder(this).setTitle(titile).setMessage(inforStr)
                .setPositiveButton(android.R.string.yes, new OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                    }
                });
        AlertDialog dialog = builder.create();
        dialog.getWindow().setType(WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY);
        dialog.show();
    }

    @Override
    public void findViews() {
        mLogSwitchPreMap.put(Utils.LOG_TYPE_MOBILE, (SelfdefinedSwitchPreference) mPrefsFragement.
                findPreference(KEY_LOG_SWITCH_MAP.get(Utils.LOG_TYPE_MOBILE)));
        mLogSwitchPreMap.put(Utils.LOG_TYPE_MODEM, (SelfdefinedSwitchPreference) mPrefsFragement.
                findPreference(KEY_LOG_SWITCH_MAP.get(Utils.LOG_TYPE_MODEM)));
        mLogSwitchPreMap.put(Utils.LOG_TYPE_NETWORK, (SelfdefinedSwitchPreference) mPrefsFragement.
                findPreference(KEY_LOG_SWITCH_MAP.get(Utils.LOG_TYPE_NETWORK)));
        mLogSwitchPreMap.put(Utils.LOG_TYPE_CONNSYS, (SelfdefinedSwitchPreference) mPrefsFragement.
                findPreference(KEY_LOG_SWITCH_MAP.get(Utils.LOG_TYPE_CONNSYS)));
        mLogSwitchPreMap.put(Utils.LOG_TYPE_MET, (SelfdefinedSwitchPreference) mPrefsFragement.
                findPreference(KEY_LOG_SWITCH_MAP.get(Utils.LOG_TYPE_MET)));

        mTaglogEnable = (CheckBoxPreference) mPrefsFragement.findPreference(KEY_TAGLOG_ENABLE);
        mAlwaysTagModemLogEnable = (CheckBoxPreference) mPrefsFragement
                .findPreference(KEY_ALWAYS_TAG_MODEM_LOG_ENABLE);

        mLogStorageLocationList =
                (ListPreference) mPrefsFragement.findPreference(KEY_ADVANCED_LOG_STORAGE_LOCATION);
        mMetLogEnable = (CheckBoxPreference) mPrefsFragement.findPreference(KEY_MET_LOG_ENABLE);
    }

    @Override
    public void initViews() {
        this.setTitle(R.string.settings_title);
        mDefaultSharedPreferences = PreferenceManager.getDefaultSharedPreferences(this);
        mSharedPreferences = getSharedPreferences(Utils.CONFIG_FILE_NAME, Context.MODE_PRIVATE);

        if (!Utils.BUILD_TYPE.equals("eng")) {
            Utils.logd(TAG, "initViews() BuildType is not eng.");
            mTaglogEnable.setChecked(mSharedPreferences.getBoolean(Utils.TAG_LOG_ENABLE, false));

        } else {
            mTaglogEnable.setChecked(mSharedPreferences.getBoolean(Utils.TAG_LOG_ENABLE, true));
        }
        mAlwaysTagModemLogEnable.setEnabled(mTaglogEnable.isChecked());

        // Hide AlwaysTagModemLog settings
        PreferenceCategory advancePreCategory =
                (PreferenceCategory) mPrefsFragement.findPreference(KEY_ADVANCED_SETTINGS_CATEGORY);
        advancePreCategory.removePreference(mAlwaysTagModemLogEnable);
        mAlwaysTagModemLogEnable = null;

        getSdcardSize();
        updateUI();
        mUpdateLogStorageEntriesTask = new UpdateLogStorageEntriesTask();
        mUpdateLogStorageEntriesTask.execute();
    }

    @Override
    public void setListeners() {
        for (int i = 0; i < mLogSwitchPreMap.size(); i++) {
            final int logType = mLogSwitchPreMap.keyAt(i);
            SelfdefinedSwitchPreference logSwitchPreference = mLogSwitchPreMap.valueAt(i);
            logSwitchPreference.setOnPreferenceClickListener(new OnPreferenceClickListener() {
                @Override
                public boolean onPreferenceClick(Preference preference) {
                    Intent intent = new Intent(
                            SettingsActivity.this, LOG_SWITCH_CLASS_MAP.get(logType));
                    intent.putExtra(Utils.SDCARD_SIZE, mSdcardSize);
                    startActivity(intent);
                    return true;
                }
            });
            logSwitchPreference.setOnPreferenceChangeListener(new OnPreferenceChangeListener() {
                @Override
                public boolean onPreferenceChange(Preference preference, Object newValue) {
                    boolean checked = (Boolean) newValue;
                    boolean isLogRunning = LogControllerUtils.getLogControllerInstance(logType)
                            .isLogRunning();
                    if (Utils.LOG_TYPE_CONNSYS == logType) {
                        isLogRunning = LogControllerUtils.isAnyConnsysLogRunning();
                    }
                    if (!checked && isLogRunning) {
                        new Thread(new Runnable() {
                            @Override
                            public void run() {
                                try {
                                    DebugLoggerUIServiceManager.getInstance().getService()
                                        .stopRecording(logType, "");
                                } catch (ServiceNullException e) {
                                    Utils.logw(TAG, "Service is null.");
                                }
                            }
                        }).start();
                        showLogNotControlledWarningDialog(logType);
                    }
                    return true;
                }
            });
        }

        mTaglogEnable.setOnPreferenceClickListener(new OnPreferenceClickListener() {
            @Override
            public boolean onPreferenceClick(Preference arg0) {
                mSharedPreferences.edit()
                        .putBoolean(Utils.TAG_LOG_ENABLE, mTaglogEnable.isChecked())
                        .apply();
                if (mAlwaysTagModemLogEnable != null) {
                    mAlwaysTagModemLogEnable.setEnabled(mTaglogEnable.isChecked());
                }
                if (mTaglogEnable.isChecked()) {
                    Utils.logi(TAG, "Request storage permission for taglog enable.");
                    PermissionUtils.requestStoragePermissions();
                }
                return true;
            }
        });
        if (mMetLogEnable != null) {
            mMetLogEnable.setOnPreferenceClickListener(new OnPreferenceClickListener() {
                @Override
                public boolean onPreferenceClick(Preference arg0) {
                    boolean isMETLogFeatureSupport =
                            METLogController.getInstance().isMETLogFeatureSupport();
                    if (isMETLogFeatureSupport) {
                        mSharedPreferences.edit()
                                .putBoolean(Utils.MET_LOG_ENABLE, mMetLogEnable.isChecked())
                                .apply();
                        updateUI();
                    } else {
                        showMETLogNotSupportWarningDialog();
                    }
                    return true;
                }
            });
        }

        mLogStorageLocationList.setOnPreferenceChangeListener(this);
    }

    private void showLogNotControlledWarningDialog(int logType) {
        Utils.logi(TAG, "showLogNotControlledWarningDialog.");
        Builder builder =
                new AlertDialog.Builder(this)
                        .setTitle(R.string.uncontrolled_warning_dialog_title)
                        .setMessage(R.string.uncontrolled_warning_dialog_message)
                        .setPositiveButton(android.R.string.ok, new OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                Utils.logi(TAG, "LogNotControlledWarningDialog is clicked!");
                            }
                        });
        AlertDialog dialog = builder.create();
        dialog.getWindow().setType(WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY);
        dialog.setCanceledOnTouchOutside(false);
        dialog.show();
    }

    private void showMETLogNotSupportWarningDialog() {
        Utils.logi(TAG, "Show METLog Not Support Warning Dialog.");
        // String message = getString(reason == 0 ? R.string.not_support_metlog_dialog_message
        // : R.string.not_support_metlog_dialog_message_timeout);
        String message = getString(R.string.not_support_metlog_dialog_message);
        Builder builder = new AlertDialog.Builder(this)
                .setTitle(R.string.not_support_metlog_dialog_title).setMessage(message)
                .setPositiveButton(android.R.string.yes, new OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        mMetLogEnable.setChecked(false);
                    }
                });

        AlertDialog dialog = builder.create();
        dialog.getWindow().setType(WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY);
        dialog.show();
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        Utils.logi(TAG,
                "Preference Change Key : " + preference.getKey() + " newValue : " + newValue);

        if (preference.getKey().equals(KEY_ADVANCED_LOG_STORAGE_LOCATION)) {
            String oldValue = Utils.getCurrentLogPathType();
            String newLogPathType = newValue.toString();
            if (oldValue.equals(newLogPathType)) {
                return true;
            }
            preference.setSummary(Utils.LOG_PATH_TYPE_STRING_MAPS.get(newLogPathType));
            getSdcardSize();
        }
        return true;
    }

    private void getSdcardSize() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    StatFs statFs = new StatFs(Utils.getCurrentLogPath());
                    long blockSize = statFs.getBlockSizeLong() / 1024;
                    mSdcardSize = statFs.getBlockCountLong() * blockSize / 1024;
                } catch (IllegalArgumentException e) {
                    Utils.loge(TAG,
                            "setSdcardSize() : StatFs error, maybe currentLogPath is invalid");
                    mSdcardSize = 0;
                }
            }
        }).run();
    }

    /**
     * @author MTK81255
     *
     */
    private class UpdateLogStorageEntriesTask extends AsyncTask<Void, Void, Void> {

        List<CharSequence> mEntriesList = new ArrayList<CharSequence>();
        List<CharSequence> mEntryValuesList = new ArrayList<CharSequence>();

        @Override
        protected Void doInBackground(Void... params) {
            // First check whether storage is mounted
            for (String logPathType : Utils.LOG_PATH_TYPE_STRING_MAPS.keySet()) {
                String logPath = Utils.getLogPath(logPathType);
                if (logPath != null && !logPath.isEmpty()) {
                    mEntryValuesList.add(logPathType);
                    mEntriesList.add(getString(Utils.LOG_PATH_TYPE_STRING_MAPS.get(logPathType)));
                }
            }
            return null;
        }

        // This is called when doInBackground() is finished
        @Override
        protected void onPostExecute(Void result) {
            setLogStorageEntries(mEntriesList, mEntryValuesList);
        }
    }

    /**
     * @param entriesList
     *            List<CharSequence>
     * @param entryValuesList
     *            List<CharSequence>
     */
    private void setLogStorageEntries(List<CharSequence> entriesList,
            List<CharSequence> entryValuesList) {
        if (entriesList.size() == 0) {
            mLogStorageLocationList.setEnabled(false);
            return;
        }
        mLogStorageLocationList.setEnabled(!mIsRecording);
        mLogStorageLocationList.setEntries(null);
        mLogStorageLocationList.setEntryValues(null);
        mLogStorageLocationList
                .setEntries(entriesList.toArray(new CharSequence[entriesList.size()]));
        mLogStorageLocationList
                .setEntryValues(entryValuesList.toArray(new CharSequence[entryValuesList.size()]));
        mLogStorageLocationList.setValue(Utils.getCurrentLogPathType());
        mLogStorageLocationList
                .setSummary(Utils.LOG_PATH_TYPE_STRING_MAPS.get(Utils.getCurrentLogPathType()));
    }

    private void updateUI() {
        Utils.logi(TAG, "updateUI()");
        mIsRecording = LogControllerUtils.isAnyControlledLogRunning();
        CharSequence[] logStorageEntries = mLogStorageLocationList.getEntries();
        if ((logStorageEntries == null) || (logStorageEntries.length == 0)) {
            Utils.logw(TAG, "Log storage entry is null or empty, disable storage set item");
            mLogStorageLocationList.setEnabled(false);
        } else {
            mLogStorageLocationList.setEnabled(!mIsRecording);
        }

        if (mMetLogEnable != null) {
            mMetLogEnable.setEnabled(!mIsRecording);
            mMetLogEnable.setChecked(mSharedPreferences.getBoolean(Utils.MET_LOG_ENABLE, false));
        }

        PreferenceCategory advancePreCategory =
                (PreferenceCategory) mPrefsFragement.findPreference(KEY_GENERAL_SETTINGS_CATEGORY);

        if (mSharedPreferences.getBoolean(Utils.MET_LOG_ENABLE, false) != true) {
            advancePreCategory.removePreference(mLogSwitchPreMap.get(Utils.LOG_TYPE_MET));
        } else {
            advancePreCategory.addPreference(mLogSwitchPreMap.get(Utils.LOG_TYPE_MET));
        }
    }

}
