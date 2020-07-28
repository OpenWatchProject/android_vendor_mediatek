package com.debug.loggerui.framework;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.text.TextUtils;

import com.debug.loggerui.MyApplication;
import com.debug.loggerui.controller.MobileLogController;
import com.debug.loggerui.settings.ModemLogSettings;
import com.debug.loggerui.settings.SettingsActivity;
import com.debug.loggerui.utils.Utils;
import com.log.handler.LogHandlerUtils.MobileLogSubLog;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.Properties;

/**
 * @author MTK81255
 *
 */
public class LogConfig {
    private static final String TAG = Utils.TAG + "/LogConfig";
    private static LogConfig sLogConfig = new LogConfig();
    private static final String CONFIG_FILE_SUFFIX =
            "shared_prefs/com.debug.loggerui_preferences.xml";
    private File mConfigFile = null;
    private Context mContext = MyApplication.getInstance().getApplicationContext();
    // This path may change with multi user feature
    private static String sConfigFilePathRoot = File.separator
            + "data/data/com.debug.loggerui/";

    private LogConfig() {
        String path = mContext.getFilesDir().getAbsolutePath();
        Utils.logd(TAG, "APK private file root path = " + path);
        if (!TextUtils.isEmpty(path) && path.contains("files")) {
            sConfigFilePathRoot = path.substring(0, path.indexOf("files"));
        }
        Utils.logd(TAG, "APK private file root sConfigFilePathRoot = " + sConfigFilePathRoot);
    }

    /**
     * @return LogConfig
     */
    public static LogConfig getInstance() {
        return sLogConfig;
    }

    /**
     * Initiate log related configuration. If configure file(SharedPreference) already, do nothing.
     */
    public void checkConfig() {
        Utils.logi(TAG, "-->checkConfig()");
        mConfigFile = new File(sConfigFilePathRoot + CONFIG_FILE_SUFFIX);

        if (mConfigFile == null || !mConfigFile.exists()) {
            Utils.logi(TAG, "Config file has not been initialized, create it now");
            initConfig();
        } else {
            Utils.logd(TAG, " configuration file already OK.");
        }
    }

    /**
     * @return boolean
     */
    public boolean isConfigDone() {
        File configFile = new File(sConfigFilePathRoot + CONFIG_FILE_SUFFIX);
        if (configFile == null || !configFile.exists()) {
            return false;
        }
        return true;
    }

    /**
     * Shared preference have not be initialized yet,
     * try to initialize it with customer config file.
     */
    private void initConfig() {
        Utils.logd(TAG, "-->initConfig()");
        String configFile = Utils.CUSTOMIZE_CONFIG_FILE;
        if (!new File(configFile).exists()) {
            configFile = Utils.CUSTOMIZE_CONFIG_FILE_N;
        }

        if (new File(configFile).exists()) {
            SharedPreferences defaultSharedPreferences = PreferenceManager
                    .getDefaultSharedPreferences(mContext);
            SharedPreferences.Editor editor = defaultSharedPreferences.edit();
            Properties customizeProp = new Properties();
            FileInputStream customizeInputStream = null;
            try {
                customizeInputStream = new FileInputStream(configFile);
                customizeProp.load(customizeInputStream);

                for (int logType : Utils.LOG_TYPE_SET) {
                    if (logType == Utils.LOG_TYPE_MET) {
                        continue;
                    }
                    boolean defaultAutoStartValue = Utils.DEFAULT_CONFIG_LOG_AUTO_START_MAP
                            .get(logType);
                    int defaultLogSize = Utils.DEFAULT_CONFIG_LOG_SIZE_MAP.get(logType);
                    int defaultTotalLogSize = -1;
                    String readAutoStartValue = customizeProp
                            .getProperty(Utils.KEY_CONFIG_LOG_AUTO_START_MAP.get(logType));
                    String readLogSize = customizeProp.getProperty(Utils.KEY_CONFIG_LOG_SIZE_MAP
                            .get(logType));
                    String readTotalLogSize = customizeProp
                            .getProperty(Utils.KEY_CONFIG_LOG_TOTAL_SIZE_MAP.get(logType));
                    if (readAutoStartValue != null) {
                        defaultAutoStartValue = Boolean.parseBoolean(readAutoStartValue);
                    }
                    if (readLogSize != null) {
                        try {
                            defaultLogSize = Integer.parseInt(readLogSize);
                        } catch (NumberFormatException e) {
                            Utils.logw(TAG, "Log size for log " + logType
                                    + " in config file is invalid");
                        }
                    }
                    if (readTotalLogSize != null) {
                        try {
                            defaultTotalLogSize = Integer.parseInt(readTotalLogSize);
                        } catch (NumberFormatException e) {
                            Utils.logv(TAG, "Total log size for log " + logType
                                    + " in config file is invalid");
                        }
                    }

                    Utils.logd(TAG, "Init log config, logType=" + logType + ", autoStart?"
                            + defaultAutoStartValue + ", logSize=" + defaultLogSize
                            + ", totalLogSize=" + defaultTotalLogSize);
                    editor.putBoolean(Utils.KEY_START_AUTOMATIC_MAP.get(logType),
                                    defaultAutoStartValue)
                            .putString(Utils.KEY_LOG_SIZE_MAP.get(logType),
                                    String.valueOf(defaultLogSize));
                    if (defaultTotalLogSize > 0) {
                        editor.putString(Utils.KEY_TOTAL_LOG_SIZE_MAP.get(logType),
                                String.valueOf(defaultTotalLogSize));
                    }

                    // For customized log path
                    String logCustomizedPath =
                            customizeProp.getProperty(Utils.LOG_CONFIG_PATH_KEY.get(logType));
                    Utils.logd(TAG, "Init log config, logCustomizedPath ? " + logCustomizedPath);
                    if (logCustomizedPath != null) {
                        editor.putString(Utils.LOG_CONFIG_PATH_KEY.get(logType), logCustomizedPath);
                    }
                }

                // For log path type customize
                String logPathType = customizeProp.getProperty(Utils.KEY_CONFIG_FILE_LOG_PATH_TYPE);
                Utils.logd(TAG, "Init log config, logPathType ? " + logPathType);
                if (logPathType != null) {
                    if (Utils.LOG_PATH_TYPE_INTERNAL_SD.equalsIgnoreCase(logPathType)) {
                        logPathType = Utils.LOG_PATH_TYPE_DEVICE_STORAGE;
                    } else if (Utils.LOG_PATH_TYPE_EXTERNAL_SD.equalsIgnoreCase(logPathType)) {
                        logPathType = Utils.LOG_PATH_TYPE_PORTABLE_STORAGE;
                    }
                    editor.putString(SettingsActivity.KEY_ADVANCED_LOG_STORAGE_LOCATION,
                            logPathType);
                }
                // For taglog customize
                String taglogEnableValue = customizeProp
                        .getProperty(Utils.KEY_CONFIG_TAGLOG_ENABLED);
                Utils.logd(TAG, "Init log config, taglogEnable ? " + taglogEnableValue);
                if (taglogEnableValue != null) {
                    mContext.getSharedPreferences(Utils.CONFIG_FILE_NAME, Context.MODE_PRIVATE)
                            .edit()
                            .putBoolean(Utils.TAG_LOG_ENABLE,
                                    Boolean.parseBoolean(taglogEnableValue)).apply();
                }
                // For always Tag Modem Log customize
                String alwaysTagModemLogEnableValue = customizeProp
                        .getProperty(Utils.KEY_CONFIG_ALWAYS_TAG_MODEMLOG_ENABLED);
                Utils.logd(TAG, "Init log config, alwaysTagModemLogEnableValue ? "
                        + alwaysTagModemLogEnableValue);
                if (alwaysTagModemLogEnableValue != null) {
                    editor.putBoolean(SettingsActivity.KEY_ALWAYS_TAG_MODEM_LOG_ENABLE,
                            Boolean.parseBoolean(alwaysTagModemLogEnableValue));
                }

                // For auto reset modem customize
                String autoResetModemEnableValue = customizeProp
                        .getProperty(Utils.KEY_CONFIG_MODEM_AUTORESET_ENABLED);
                Utils.logd(TAG, "Init log config, autoResetModemEnableValue ? "
                        + autoResetModemEnableValue);
                if (autoResetModemEnableValue != null) {
                    editor.putBoolean(ModemLogSettings.KEY_MD_AUTORESET,
                            Boolean.parseBoolean(autoResetModemEnableValue));
                }

                // For set modem log mode customize
                String modemLogMode = customizeProp.getProperty(Utils.KEY_CONFIG_MODEM_LOG_MODE);
                Utils.logd(TAG, "Init log config, modemLogMode ? " + modemLogMode);
                if (modemLogMode != null) {
                    editor.putString(Utils.KEY_MD_MODE_1, modemLogMode);
                    editor.putString(Utils.KEY_MD_MODE_2, modemLogMode);
                    // adb shell am broadcast -a com.debug.loggerui.ADB_CMD
                    // -e cmd_name switch_modem_log_mode --ei cmd_target 1
                    Intent intent = new Intent(Utils.ACTION_ADB_CMD);
                    intent.putExtra(Utils.EXTRA_ADB_CMD_NAME,
                            Utils.ADB_COMMAND_SWITCH_MODEM_LOG_MODE);
                    int modemLogModeInt = 2;
                    try {
                        modemLogModeInt = Integer.parseInt(modemLogMode);
                    } catch (NumberFormatException e) {
                        Utils.logv(TAG, "modemLogMode in config file is invalid");
                        modemLogModeInt = 2;
                    }
                    intent.putExtra(Utils.EXTRA_ADB_CMD_TARGET, modemLogModeInt);
                    Utils.sendBroadCast(intent);

                }
                // For notification show enable/disable customize
                String notificationEnableValue =
                        customizeProp.getProperty(Utils.KEY_CONFIG_NOTIFICATION_ENABLED);
                Utils.logd(TAG,
                        "Init log config, notificationEnableValue ? " + notificationEnableValue);
                if (notificationEnableValue != null) {
                    editor.putBoolean(Utils.KEY_PREFERENCE_NOTIFICATION_ENABLED,
                            Boolean.parseBoolean(notificationEnableValue));
                }

                // For taglog zip logs whether only to one file customize
                String taglog2one = customizeProp
                        .getProperty(Utils.KEY_CONFIG_TAGLOG_ZIP2ONE);
                Utils.logd(TAG, "Init log config, taglog2one ? "
                        + taglog2one);
                if (taglog2one != null) {
                    Utils.setTaglogToOnFile(Boolean.parseBoolean(taglog2one));
                }

                // For taglog zip file path customize
                String zipFilePath = customizeProp.getProperty(Utils.KEY_CONFIG_TAGLOG_ZIPFILEPATH);
                Utils.logd(TAG, "Init log config, zipFilePath ? " + zipFilePath);
                if (zipFilePath != null) {
                    Utils.setZipFilePath(zipFilePath);
                }

             // For pls monitor ee customize
                String iMonitor = customizeProp
                        .getProperty(Utils.KEY_CONFIG_MONITOR_MODEM_ABNORMAL_EVENT);
                Utils.logd(TAG, "monitor modem abnormal event, iMonitor ? " + iMonitor);
                if (iMonitor != null) {
                    editor.putBoolean(ModemLogSettings.KEY_MD_MONITOR_MODEM_ABNORMAL_EVENT,
                                    Boolean.parseBoolean(iMonitor));
                }

                // For mobile sub logs
                for (MobileLogSubLog mobileLogSubLog : MobileLogSubLog.values()) {
                    String mobileSubLogEnableValue = customizeProp.getProperty(
                            MobileLogController.SUB_LOG_CONFIG_STRING_MAP.get(mobileLogSubLog));
                    if (mobileSubLogEnableValue != null) {
                        editor.putBoolean(
                                MobileLogController.SUB_LOG_SETTINGS_ID_MAP.get(mobileLogSubLog),
                                Boolean.parseBoolean(mobileSubLogEnableValue));
                    }
                }

                // For Modem Log daemon do auto reset after modem EE
                String isModemLogAutoResetEE = customizeProp
                        .getProperty(Utils.KEY_CONFIG_MODEMLOG_AUTO_RESET_MODEM);
                Utils.logd(TAG,
                        "Init log config, isModemLogAutoResetEE ? " + isModemLogAutoResetEE);
                if (isModemLogAutoResetEE != null) {
                    editor.putBoolean(Utils.KEY_PREFERENCE_MODEMLOG_AUTO_RESET_MODEM,
                            Boolean.parseBoolean(isModemLogAutoResetEE));
                }

                editor.apply();
            } catch (IOException e) {
                Utils.loge(TAG, "read customize config file error!" + e.toString());
                initDefaultConfig();
            } finally {
                if (customizeInputStream != null) {
                    try {
                        customizeInputStream.close();
                    } catch (IOException e2) {
                        Utils.loge(TAG, "Fail to close opened customization file.");
                    }
                }
            }
        } else {
            Utils.logw(TAG, "Can not find config file, use default value.");
            initDefaultConfig();
        }
    }

    /**
     * If found no configuration file in device,
     * just initiate shared preference with default value in code.
     */
    private void initDefaultConfig() {
        Utils.logw(TAG, "-->initDefaultConfig()");
        SharedPreferences defaultSharedPreferences = PreferenceManager
                .getDefaultSharedPreferences(mContext);
        SharedPreferences.Editor editor = defaultSharedPreferences.edit();

        for (int logType : Utils.LOG_TYPE_SET) {
            if (logType == Utils.LOG_TYPE_MET) {
                continue;
            }
            boolean defaultAutoStartValue = Utils.DEFAULT_CONFIG_LOG_AUTO_START_MAP.get(logType);
            int defaultLogSize = Utils.DEFAULT_CONFIG_LOG_SIZE_MAP.get(logType);
            editor.putBoolean(Utils.KEY_START_AUTOMATIC_MAP.get(logType), defaultAutoStartValue)
                    .putString(Utils.KEY_LOG_SIZE_MAP.get(logType), String.valueOf(defaultLogSize));
        }
        editor.apply();
    }
}
