package com.debug.loggerui.controller;

import android.text.TextUtils;

import com.debug.loggerui.settings.NetworkLogSettings;
import com.debug.loggerui.utils.Utils;
import com.log.handler.LogHandler;
import com.log.handler.LogHandlerUtils.LogType;

/**
 * @author MTK81255
 *
 */
public class NetworkLogController extends AbstractLogController {
    private static final String TAG = Utils.TAG + "/NetworkLogController";

    public static NetworkLogController sInstance = new NetworkLogController(LogType.NETWORK_LOG);
    private static final String ADB_COMMAND_ENVIRONMENT_CHECK_PREFIX = "environment_check_";
    private static final String ADB_COMMAND_SET_PACKAGE_LIMITATION_ENABLED =
            "set_package_limitation_enabled_";
    private static final String ADB_COMMAND_SET_NETWORK_PACKAGE_SIZE =
            "set_network_package_size_";
    private static final String ADB_COMMAND_SET_ROHC_COMPRESSION_ENABLED =
            "set_rohc_compression_enabled_";
    private static final String ADB_COMMAND_SET_ROHC_TOTAL_FILE_NUMBER =
            "set_rohc_total_file_number_";


    private NetworkLogController(LogType logType) {
        super(logType);
    }

    public static NetworkLogController getInstance() {
        return sInstance;
    }

    @Override
    protected boolean starTypeLog(String logPath) {
        return LogHandler.getInstance().startNetworkLog(logPath, getRecycleSize(),
                getLimitedPackageSize());
    }

    private int getRecycleSize() {
        int recycleSize = Utils.DEFAULT_LOG_SIZE;
        try {
            recycleSize = Integer.parseInt(mDefaultSharedPreferences.getString(
                    Utils.KEY_NETWORK_LOG_LOGSIZE, String.valueOf(Utils.DEFAULT_LOG_SIZE)));
        } catch (NumberFormatException e) {
            Utils.loge(TAG, "parser recycleSize failed!");
            recycleSize = Utils.DEFAULT_LOG_SIZE;
        }
        return recycleSize;
    }

    /**
     * Get each network log package limited size, 0 mean no limitation.
     *
     * @return
     */
    private int getLimitedPackageSize() {
        boolean limitPackageEnabled =
                mDefaultSharedPreferences
                        .getBoolean(NetworkLogSettings.KEY_NT_LIMIT_PACKAGE_ENABLER, false);
        if (!limitPackageEnabled) {
            return 0;
        }
        String limitedPackageSizeStr =
                mDefaultSharedPreferences.getString(NetworkLogSettings.KEY_NT_LIMIT_PACKAGE_SIZE,
                        String.valueOf(NetworkLogSettings.VALUE_NT_LIMIT_PACKAGE_DEFAULT_SIZE));

        int limitedPackageSize = NetworkLogSettings.VALUE_NT_LIMIT_PACKAGE_DEFAULT_SIZE;
        if (!TextUtils.isEmpty(limitedPackageSizeStr)) {
            try {
                int tempSize = Integer.parseInt(limitedPackageSizeStr);
                if (tempSize >= 0) {
                    limitedPackageSize = tempSize;
                }
            } catch (NumberFormatException e) {
                Utils.loge(TAG, "parser limitedPackageSizeStr failed : " + limitedPackageSizeStr);
            }
        }
        return limitedPackageSize;
    }

    @Override
    protected boolean stopTypeLog() {
        boolean isNeedCheck =
                mDefaultSharedPreferences.getBoolean(Utils.KEY_NETWORK_LOG_DO_PING, false);
        return LogHandler.getInstance().stopNetworkLog(isNeedCheck);
    }

    /**
     * @return boolean
     */
    public boolean isNetworkLogRohcCompressionSupport() {
        return LogHandler.getInstance().isNetworkLogRohcCompressionSupport();
    }

    /**
     * @param enable
     *            boolean
     * @return boolean
     */
    public boolean enableNetworkLogRohcCompression(boolean enable) {
        return LogHandler.getInstance().enableNetworkLogRohcCompression(enable);
    }

    /**
     * @param totalNumber
     *            int
     * @return boolean
     */
    public boolean setNetworkLogRohcTotalFileNumber(int totalNumber) {
        return LogHandler.getInstance().setNetworkLogRohcTotalFileNumber(totalNumber);
    }
    /**
     * dumpNetwork is for dump logs at device shut down. it should stop log.
     * but no need change boot up saved settings.
     * @return boolean
     */
    public boolean dumpNetwork() {
        return stopTypeLog();
    }
    @Override
    public boolean dealWithADBCommand(String command) {
        if (command != null && command.startsWith(ADB_COMMAND_ENVIRONMENT_CHECK_PREFIX)) {
            // adb shell am broadcast -a com.debug.loggerui.ADB_CMD
            // -e cmd_name environment_check_value --ei cmd_target 4
            String newValue = command.substring(command.length() - 1);
            if (newValue.equals("0") || newValue.equals("1")) {
                mDefaultSharedPreferences.edit()
                        .putBoolean(Utils.KEY_NETWORK_LOG_DO_PING, newValue.equals("1")).apply();
            } else {
                Utils.logw(TAG, "Unsupported set ENVIRONMENT CHECK value!");
            }
        } else if (command != null
                && command.startsWith(ADB_COMMAND_SET_PACKAGE_LIMITATION_ENABLED)) {
            // adb shell am broadcast -a com.debug.loggerui.ADB_CMD
            // -e cmd_name set_package_limitation_enabled_value --ei cmd_target 4
            String newValue = command.substring(command.length() - 1);
            if (newValue.equals("0") || newValue.equals("1")) {
                mDefaultSharedPreferences.edit()
                        .putBoolean(NetworkLogSettings.KEY_NT_LIMIT_PACKAGE_ENABLER,
                                newValue.equals("1"))
                        .apply();
                if (newValue.equals("1") && mDefaultSharedPreferences
                        .getBoolean(NetworkLogSettings.KEY_NT_ROHC_COMPRESSION_ENABLER, false)) {
                    mDefaultSharedPreferences.edit()
                            .putBoolean(NetworkLogSettings.KEY_NT_ROHC_COMPRESSION_ENABLER, false)
                            .apply();
                    enableNetworkLogRohcCompression(false);
                }
            } else {
                Utils.logw(TAG, "Unsupported set PACKAGE LIMITATION value!");
            }
        } else if (command != null
                && command.startsWith(ADB_COMMAND_SET_NETWORK_PACKAGE_SIZE)) {
            // adb shell am broadcast -a com.debug.loggerui.ADB_CMD
            // -e cmd_name set_network_package_size_128 --ei cmd_target 4
            String newValueStr =
                    command.substring(ADB_COMMAND_SET_NETWORK_PACKAGE_SIZE.length());
            int newLogSize = 0;
            try {
                newLogSize = Integer.parseInt(newValueStr);
            } catch (NumberFormatException e) {
                Utils.loge(TAG, "Invalid set network package size parameter: " + newValueStr);
                return false;
            }
            if (newLogSize <= 0) {
                Utils.loge(TAG, "Given network package size should bigger than zero, but got "
                        + newValueStr);
                return false;
            }
            mDefaultSharedPreferences.edit()
                    .putBoolean(NetworkLogSettings.KEY_NT_LIMIT_PACKAGE_ENABLER, true)
                    .apply();
            if (mDefaultSharedPreferences
                    .getBoolean(NetworkLogSettings.KEY_NT_ROHC_COMPRESSION_ENABLER, false)) {
                mDefaultSharedPreferences.edit()
                        .putBoolean(NetworkLogSettings.KEY_NT_ROHC_COMPRESSION_ENABLER, false)
                        .apply();
                enableNetworkLogRohcCompression(false);
            }
            mDefaultSharedPreferences.edit()
                    .putString(NetworkLogSettings.KEY_NT_LIMIT_PACKAGE_SIZE, newValueStr)
                    .apply();
        } else if (command != null
                && command.startsWith(ADB_COMMAND_SET_ROHC_COMPRESSION_ENABLED)) {
            // adb shell am broadcast -a com.debug.loggerui.ADB_CMD
            // -e cmd_name set_rohc_compression_enabled_value --ei cmd_target 4
            String newValue = command.substring(command.length() - 1);
            if (newValue.equals("0") || newValue.equals("1")) {
                boolean isEnabled = newValue.equals("1");
                mDefaultSharedPreferences.edit()
                        .putBoolean(NetworkLogSettings.KEY_NT_ROHC_COMPRESSION_ENABLER, isEnabled)
                        .apply();
                enableNetworkLogRohcCompression(isEnabled);
                if (isEnabled && mDefaultSharedPreferences
                        .getBoolean(NetworkLogSettings.KEY_NT_LIMIT_PACKAGE_ENABLER, false)) {
                    mDefaultSharedPreferences.edit()
                            .putBoolean(NetworkLogSettings.KEY_NT_LIMIT_PACKAGE_ENABLER, false)
                            .apply();
                }
            } else {
                Utils.logw(TAG, "Unsupported set PACKAGE LIMITATION value!");
            }
        } else if (command != null
                && command.startsWith(ADB_COMMAND_SET_ROHC_TOTAL_FILE_NUMBER)) {
            // adb shell am broadcast -a com.debug.loggerui.ADB_CMD
            // -e cmd_name set_rohc_total_file_number_10 --ei cmd_target 4
            String newValueStr = command.substring(ADB_COMMAND_SET_ROHC_TOTAL_FILE_NUMBER.length());
            int newLogSize = 0;
            try {
                newLogSize = Integer.parseInt(newValueStr);
            } catch (NumberFormatException e) {
                Utils.loge(TAG, "Invalid set network package size parameter: " + newValueStr);
                return false;
            }
            if (newLogSize <= 0) {
                Utils.loge(TAG, "Given network package size should bigger than zero, but got "
                        + newValueStr);
                return false;
            }
            mDefaultSharedPreferences.edit()
                    .putBoolean(NetworkLogSettings.KEY_NT_ROHC_COMPRESSION_ENABLER, true).apply();
            if (mDefaultSharedPreferences
                    .getBoolean(NetworkLogSettings.KEY_NT_LIMIT_PACKAGE_ENABLER, false)) {
                mDefaultSharedPreferences.edit()
                        .putBoolean(NetworkLogSettings.KEY_NT_LIMIT_PACKAGE_ENABLER, false).apply();
            }
            mDefaultSharedPreferences.edit()
                    .putString(NetworkLogSettings.KEY_NT_ROHC_TOTAL_FILE_NUMBER, newValueStr)
                    .apply();
        }
        return super.dealWithADBCommand(command);
    }

}
