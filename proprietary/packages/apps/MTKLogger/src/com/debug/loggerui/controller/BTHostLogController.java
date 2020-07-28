package com.debug.loggerui.controller;


import com.debug.loggerui.settings.ConnsysLogSettings;
import com.debug.loggerui.settings.SettingsActivity;
import com.debug.loggerui.utils.Utils;
import com.log.handler.LogHandler;
import com.log.handler.LogHandlerUtils.BTFWLogLevel;
import com.log.handler.LogHandlerUtils.LogType;

/**
 * @author MTK81255
 *
 */
public class BTHostLogController extends AbstractLogController {
    private static final String TAG = Utils.TAG + "/BTHostLogController";

    public static BTHostLogController sInstance = new BTHostLogController(LogType.BTHOST_LOG);

    private static final String ADB_COMMAND_SET_STACK_LOG_ENABLED = "set_stack_log_enabled_";
    /**
     * Command to set BTFwLogLevel.
     */
    private static final String ADB_COMMAND_SET_BTFWLOG_LEVEL = "set_btfwlog_level_";

    private BTHostLogController(LogType logType) {
        super(logType);
    }

    public static BTHostLogController getInstance() {
        return sInstance;
    }

    @Override
    protected boolean starTypeLog(String logPath) {
        setHCISnoopLogSize();
        setBTStackLogEnable();
        setBTFWLogLevel();
        return super.starTypeLog(logPath);
    }

    /**
     * setBTFirmwareLogLevel.
     */
    public void setBTFWLogLevel() {
        String btFWLogLevel =
                mDefaultSharedPreferences.getString(ConnsysLogSettings.KEY_BTFW_LOG_LEVEL, "2");
        Utils.logi(TAG, "setBTFirmwareLogLevel. btFWLogLevel = " + btFWLogLevel);
        LogHandler.getInstance().setBTFWLogLevel(BTFWLogLevel.getBTFWLogLevelByID(btFWLogLevel));
    }

    /**
     * setHCISnoopLogSize.
     */
    public void setHCISnoopLogSize() {
        int hciSnoopLogSize = Utils.DEFAULT_CONFIG_LOG_SIZE_MAP.get(Utils.LOG_TYPE_BTHOST);
        String hciSnoopLogSizStr = "";
        try {
            hciSnoopLogSizStr = mDefaultSharedPreferences
                    .getString(ConnsysLogSettings.KEY_BTHOST_LOGSIZE,
                            String.valueOf(hciSnoopLogSize));
            hciSnoopLogSize = Integer.parseInt(String.valueOf(hciSnoopLogSizStr));
        } catch (NumberFormatException e) {
            Utils.loge(TAG,
                    "Integer.parseInt(" + String.valueOf(hciSnoopLogSizStr) + ") is error!");
        }
        setLogRecycleSize(hciSnoopLogSize);
    }

    /**
     * setBTStackLogEnable.
     */
    public void setBTStackLogEnable() {
        boolean btStackLogEnable = mDefaultSharedPreferences
                .getBoolean(ConnsysLogSettings.KEY_BTSTACK_LOG_ENABLE, false);
        Utils.logi(TAG, "setBTStackLogEnable. btStackLogEnable = " + btStackLogEnable);
        LogHandler.getInstance().setBTHostDebuglogEnable(btStackLogEnable);
    }

    @Override
    public boolean dealWithADBCommand(String command) {
        if (command != null && command.startsWith(ADB_COMMAND_SET_STACK_LOG_ENABLED)) {
            // adb shell am broadcast -a com.debug.loggerui.ADB_CMD
            // -e cmd_name set_stack_log_enabled_value --ei cmd_target 64
            String newValue = command.substring(command.length() - 1);
            if (newValue.equals("0") || newValue.equals("1")) {
                boolean isEnabled = newValue.equals("1");
                mDefaultSharedPreferences.edit()
                        .putBoolean(ConnsysLogSettings.KEY_BTSTACK_LOG_ENABLE, isEnabled).apply();
                setBTStackLogEnable();
            } else {
                Utils.logw(TAG, "Unsupported set PACKAGE LIMITATION value!");
            }
        } else if (command != null && command.startsWith(ADB_COMMAND_SET_BTFWLOG_LEVEL)) {
            // adb shell am broadcast -a com.mediatek.mtklogger.ADB_CMD
            // -e cmd_name set_btfwlog_level_value --ei cmd_target 64
            String newValue = command.substring(command.length() - 1);
            mDefaultSharedPreferences.edit()
                    .putString(ConnsysLogSettings.KEY_BTFW_LOG_LEVEL, newValue).apply();
            setBTFWLogLevel();
        }
        return super.dealWithADBCommand(command);
    }

    @Override
    public boolean isLogControlled() {
        return mDefaultSharedPreferences.getBoolean(SettingsActivity.KEY_LOG_SWITCH_MAP.get(
                    Utils.LOG_TYPE_CONNSYS), true)
                && super.isLogControlled();
    }

}
