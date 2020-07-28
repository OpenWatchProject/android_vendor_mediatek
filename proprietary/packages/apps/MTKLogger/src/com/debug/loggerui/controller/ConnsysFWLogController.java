package com.debug.loggerui.controller;

import android.os.SystemProperties;

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
public class ConnsysFWLogController extends AbstractLogController {
    public static ConnsysFWLogController sInstance =
            new ConnsysFWLogController(LogType.CONNSYSFW_LOG);

    private ConnsysFWLogController(LogType logType) {
        super(logType);
    }

    public static ConnsysFWLogController getInstance() {
        return sInstance;
    }

    @Override
    protected boolean starTypeLog(String logPath) {
        String btFWLogLevel = mDefaultSharedPreferences
                .getString(ConnsysLogSettings.KEY_BTFW_LOG_LEVEL, "2");
        return LogHandler.getInstance().startConnsysFWLog(logPath,
                BTFWLogLevel.getBTFWLogLevelByID(btFWLogLevel));
    }

    @Override
    public boolean isLogFeatureSupport() {
        return SystemProperties.get("ro.vendor.connsys.dedicated.log", "0").equals("1");
    }

    @Override
    public boolean isLogControlled() {
        return mDefaultSharedPreferences.getBoolean(SettingsActivity.KEY_LOG_SWITCH_MAP.get(
                    Utils.LOG_TYPE_CONNSYS), true)
                && super.isLogControlled();
    }

}
