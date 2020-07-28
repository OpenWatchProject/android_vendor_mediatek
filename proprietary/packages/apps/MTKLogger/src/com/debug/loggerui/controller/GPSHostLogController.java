package com.debug.loggerui.controller;

import com.debug.loggerui.settings.SettingsActivity;
import com.debug.loggerui.utils.Utils;
import com.log.handler.LogHandlerUtils.LogType;

/**
 * @author MTK81255
 *
 */
public class GPSHostLogController extends AbstractLogController {

    public static GPSHostLogController sInstance = new GPSHostLogController(LogType.GPSHOST_LOG);

    private GPSHostLogController(LogType logType) {
        super(logType);
    }

    public static GPSHostLogController getInstance() {
        return sInstance;
    }

    @Override
    public boolean setLogRecycleSize(int logSize) {
        return true;
    }

    @Override
    public boolean isLogControlled() {
        return mDefaultSharedPreferences.getBoolean(SettingsActivity.KEY_LOG_SWITCH_MAP.get(
                    Utils.LOG_TYPE_CONNSYS), true)
                && super.isLogControlled();
    }

}
