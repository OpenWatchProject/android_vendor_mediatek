package com.debug.loggerui.controller;

import com.debug.loggerui.settings.SettingsActivity;
import com.log.handler.LogHandler;
import com.log.handler.LogHandlerUtils.LogType;

/**
 * @author MTK81255
 *
 */
public class METLogController extends AbstractLogController {

    public static METLogController sInstance = new METLogController(LogType.MET_LOG);

    private METLogController(LogType logType) {
        super(logType);
    }

    public static METLogController getInstance() {
        return sInstance;
    }

    @Override
    public boolean isLogFeatureSupport() {
        return mDefaultSharedPreferences
                .getBoolean(SettingsActivity.KEY_MET_LOG_ENABLE, false);
    }

    @Override
    public boolean setLogRecycleSize(int size) {
        return true;
    }

    public boolean isMETLogFeatureSupport() {
        return LogHandler.getInstance().isMETLogFeatureSupport();
    }

    /**
     * @param periodSize
     *            int
     * @return boolean
     */
    public boolean setMETLogPeriod(int periodSize) {
        return LogHandler.getInstance().setMETLogPeriod(periodSize);
    }

    /**
     * @param bufferSize
     *            int
     * @return boolean
     */
    public boolean setMETLogCPUBuffer(int bufferSize) {
        return LogHandler.getInstance().setMETLogCPUBuffer(bufferSize);
    }

    /**
     * @param sspmSize
     *            int
     * @return boolean
     */
    public boolean setMETLogSSPMSize(int sspmSize) {
        return LogHandler.getInstance().setMETLogSSPMSize(sspmSize);
    }

    /**
     * @param enable
     *            boolean
     * @return boolean
     */
    public boolean setHeavyLoadRecordingEnable(boolean enable) {
        return LogHandler.getInstance().setHeavyLoadRecordingEnable(enable);
    }

    public String getInitValues() {
        return LogHandler.getInstance().getMETLogInitValues();
    }
}
