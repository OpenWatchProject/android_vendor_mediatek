package com.debug.loggerui.controller;

import android.content.SharedPreferences;

import com.debug.loggerui.MyApplication;
import com.debug.loggerui.settings.SettingsActivity;
import com.debug.loggerui.utils.Utils;
import com.log.handler.LogHandler;
import com.log.handler.LogHandlerUtils.LogType;

import java.io.File;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;

/**
 * @author MTK81255
 *
 */
public abstract class AbstractLogController {
    private static final String TAG = Utils.TAG + "/LogController";

    protected LogType mLogType;

    private static LinkedList<String> sSelfKeyLogBufferList = new LinkedList<String>();
    private static int sMaxLogBufferSize = 100;

    protected SharedPreferences mDefaultSharedPreferences =
            MyApplication.getInstance().getDefaultSharedPreferences();
    /**
     * @param logType
     *            LogType
     */
    protected AbstractLogController(LogType logType) {
        mLogType = logType;
    }

    public LogType getLogType() {
        return mLogType;
    }

    /**
     * @param logPath
     *            String
     * @return boolean
     */
    public boolean startLog(String logPath) {
        String configPath = mDefaultSharedPreferences.getString(Utils.LOG_CONFIG_PATH_KEY
                .get(LogControllerUtils.LOG_TYPE_OBJECT_TO_INT.get((mLogType))), null);
        if (configPath != null && !configPath.isEmpty()) {
            logPath = configPath;
        }
        setBootupLogSaved(true);
        return starTypeLog(logPath);
    }

    protected boolean starTypeLog(String logPath) {
        return LogHandler.getInstance().startTypeLog(mLogType, logPath);
    }

    /**
     * @return boolean
     */
    public boolean stopLog() {
        setBootupLogSaved(false);
        return stopTypeLog();
    }

    protected boolean stopTypeLog() {
        return LogHandler.getInstance().stopTypeLog(mLogType);
    }

    /**
     * @param logPath
     *            String
     * @return boolean
     */
    public boolean rebootLog(String logPath) {
        boolean isStopSuccess = stopTypeLog();
        boolean isStartSuccess = starTypeLog(logPath);
        return isStopSuccess && isStartSuccess;
    }

    /**
     * @param logSize
     *            int
     * @return boolean
     */
    public boolean setLogRecycleSize(int logSize) {
        return LogHandler.getInstance().setTypeLogRecycleSize(mLogType, logSize);
    }

    /**
     * @param enable
     *            boolean
     * @return boolean
     */
    public boolean setBootupLogSaved(boolean enable) {
        if (enable != mDefaultSharedPreferences.getBoolean(
                Utils.KEY_START_AUTOMATIC_MAP
                        .get(LogControllerUtils.LOG_TYPE_OBJECT_TO_INT.get((mLogType))),
                false)) {
            mDefaultSharedPreferences.edit()
                    .putBoolean(
                            Utils.KEY_START_AUTOMATIC_MAP
                                    .get(LogControllerUtils.LOG_TYPE_OBJECT_TO_INT.get((mLogType))),
                            enable)
                    .apply();
        }
        return LogHandler.getInstance().setBootupTypeLogSaved(mLogType, enable);
    }

    /**
     * @return boolean
     */
    public boolean isLogControlled() {
        return mDefaultSharedPreferences.getBoolean(
                SettingsActivity.KEY_LOG_SWITCH_MAP.get(
                        LogControllerUtils.LOG_TYPE_OBJECT_TO_INT.get(mLogType)), true);
    }

    /**
     * @return boolean
     */
    public boolean isLogFeatureSupport() {
        return true;
    }

    /**
     * @return boolean
     */
    public boolean isLogRunning() {
        return LogHandler.getInstance().isTypeLogRunning(mLogType);
    }

    /**
     * @return String
     */
    public String getRunningLogPath() {
        String runningLogPath = null;
        if (!isLogRunning()) {
            return null;
        }
        String parentPath = Utils.getCurrentLogPath() + Utils.LOG_PATH_PARENT
                + Utils.LOG_PATH_MAP.get(LogControllerUtils.LOG_TYPE_OBJECT_TO_INT.get((mLogType)));
        File fileTree = new File(parentPath + File.separator + Utils.LOG_TREE_FILE);
        File logFile = Utils.getLogFolderFromFileTree(fileTree, false);
        if (null != logFile && logFile.exists()) {
            runningLogPath = logFile.getAbsolutePath();
        }
        Utils.logi(TAG, "<--getRunningLogPath(), logType = " + mLogType + ", runningLogPath = "
                + runningLogPath);
        return runningLogPath;
    }

    /**
     * @param command
     *            String
     * @return boolean
     */
    public boolean dealWithADBCommand(String command) {
        return true;
    }

    /**
     * @return List<String>
     */
    // add for self key log for output----->start
    public List<String> getSelfKeyLog() {
        List<String> keyLogList = new ArrayList<String>();
        synchronized (sSelfKeyLogBufferList) {
            keyLogList.addAll(sSelfKeyLogBufferList);
        }
        return keyLogList;
    }

    protected void addSelfKeyLogToBuffer(String msg) {
        synchronized (sSelfKeyLogBufferList) {
            if (sSelfKeyLogBufferList.size() >= sMaxLogBufferSize) {
                sSelfKeyLogBufferList.removeFirst();
            }
            sSelfKeyLogBufferList.addLast(Utils.integratedOutLog(msg));
        }
    }
    protected void setKeyLogBufferSize(int bufferSize) {
        sMaxLogBufferSize = bufferSize;
    }
    //add for self key log for output<-----end
}
