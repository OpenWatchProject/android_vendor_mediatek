package com.log.handler.instance;

import android.os.SystemProperties;

import com.log.handler.LogHandlerUtils;
import com.log.handler.LogHandlerUtils.AbnormalEvent;
import com.log.handler.LogHandlerUtils.IAbnormalEventMonitor;
import com.log.handler.LogHandlerUtils.LogType;
import com.log.handler.connection.ILogConnection;

import java.util.HashSet;
import java.util.Observable;
import java.util.Observer;
import java.util.Set;

/**
 * Supper class for each log instance, like network log, mobile log and modem log.
 */
public abstract class AbstractLogInstance implements Observer {
    private static final String TAG = LogHandlerUtils.TAG + "/AbstractLogInstance";
    protected static final String COMMAND_EXECUTE_SUCCESS = "1";

    protected static final String COMMAND_START = "deep_start";
    protected static final String COMMAND_STOP = "deep_stop";
    protected static final String COMMAND_SET_STORAGE_PATH = "set_storage_path,";
    /**
     * Flag for configuring native log size parameter from Java.
     */
    protected static final String COMMAND_SET_LOG_SIZE = "logsize=";
    protected static final String COMMAND_SET_SAVE_BOOTUP_LOG = "autostart=";

    protected ILogConnection mLogConnection;
    protected LogType mLogType;

    /**
     * @param logConnection
     *            ILogConnection
     * @param logType
     *            LogType
     */
    public AbstractLogInstance(ILogConnection logConnection, LogType logType) {
        mLogConnection = logConnection;
        mLogType = logType;
        mLogConnection.addServerObserver(this);
    }

    /**
     * @param command
     *            String
     * @return boolean
     */
    public boolean executeCommand(String command) {
        return executeCommand(command, false);
    }

    /**
     * @param command
     *            String
     * @param isWaitingResponse
     *            boolean
     * @return boolean
     */
    public boolean executeCommand(String command, boolean isWaitingResponse) {
        LogHandlerUtils.logi(TAG, "-->executeCommand(), command = " + command
                + ", isWaitingResponse = " + isWaitingResponse);
        boolean sendSuccess = mLogConnection.sendToServer(command);
        if (!sendSuccess || !isWaitingResponse) {
            LogHandlerUtils.logd(TAG, "executeCommand result, sendSuccess = " + sendSuccess);
            return sendSuccess;
        }
        boolean isSuccess = false;
        String response = getResponse(command);
        if (response != null && response.startsWith(command + ",")) {
            isSuccess = response.substring(command.length() + 1).equals(COMMAND_EXECUTE_SUCCESS);
        }
        LogHandlerUtils.logd(TAG, "<--executeCommand result, isSuccess = " + isSuccess);
        return isSuccess;
    }

    /**
     * @param command
     *            String
     * @return String
     */
    public String getValueFromServer(String command) {
        LogHandlerUtils.logi(TAG, "-->getValueFromServer(), command = " + command);
        if (!mLogConnection.sendToServer(command)) {
            LogHandlerUtils.loge(TAG, "sendToServer failed, command = " + command);
            return "";
        }
        String serverValue = "";
        String response = getResponse(command);
        if (response != null && response.startsWith(command + ",")) {
            serverValue = response.substring(command.length() + 1);
        }
        LogHandlerUtils.logi(TAG, "<--getValueFromServer(), serverValue = " + serverValue);
        return serverValue;
    }

    private String getResponse(String command) {
        String responseStr = mLogConnection.getResponseFromServer(command);
        long timeout = 15000;
        while (responseStr == null || responseStr.isEmpty()) {
            try {
                Thread.sleep(100);
                timeout -= 100;
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            if (!mLogConnection.isConnection() || timeout <= 0) {
                LogHandlerUtils.logw(TAG, "receiveFromServer timeout, command = " + command);
                break;
            }
            responseStr = mLogConnection.getResponseFromServer(command);
        }
        LogHandlerUtils.logi(TAG, "getResponse, responseStr = " + responseStr
                + " and waiting time = " + (15000 - timeout));
        return responseStr;
    }

    /**
     * @param logPath
     *            String
     * @return boolean
     */
    public synchronized boolean startLog(String logPath) {
        executeCommand(COMMAND_SET_STORAGE_PATH + logPath);
        return executeCommand(getStartCommand(), true);
    }

    private String mStartCommand = COMMAND_START;

    protected String getStartCommand() {
        return mStartCommand;
    }

    protected void setStartCommand(String startCommand) {
        mStartCommand = startCommand;
    }

    /**
     * @return boolean
     */
    public synchronized boolean stopLog() {
        return executeCommand(COMMAND_STOP, true);
    }

    /**
     * @param logSize
     *            int
     * @return boolean
     */
    public boolean setLogRecycleSize(int logSize) {
        return executeCommand(COMMAND_SET_LOG_SIZE + logSize);
    }

    /**
     * @param enable
     *            boolean
     * @return boolean
     */
    public boolean setBootupLogSaved(boolean enable) {
        return executeCommand(COMMAND_SET_SAVE_BOOTUP_LOG + (enable ? "1" : "0"));
    }

    public boolean isLogRunning() {
        return SystemProperties.get(getLogStatusSystemProperty(), "0").equals("1");
    }

    /**
     * @return String
     */
    public abstract String getLogStatusSystemProperty();

    public LogType getLogType() {
        return mLogType;
    }

    /**
     *
     */
    public void disConnect() {
        mLogConnection.disConnect();
        mLogConnection.deleteServerObserver(this);
    }

    private Set<IAbnormalEventMonitor> mAbnormalEventMonitorList =
            new HashSet<IAbnormalEventMonitor>();

    /**
     * @param abnormalEventMonitor
     *            IAbnormalEventMonitor
     * @return boolean
     */
    public boolean
            registerAbnormalEventMonitor(IAbnormalEventMonitor abnormalEventMonitor) {
        synchronized (mAbnormalEventMonitorList) {
            if (abnormalEventMonitor != null
                    && !mAbnormalEventMonitorList.contains(abnormalEventMonitor)) {
                return mAbnormalEventMonitorList.add(abnormalEventMonitor);
            }
        }
        return false;
    }

    /**
     * @param abnormalEventMonitor
     *            IAbnormalEventMonitor
     * @return boolean
     */
    public boolean
            unregisterAbnormalEventMonitor(IAbnormalEventMonitor abnormalEventMonitor) {
        synchronized (mAbnormalEventMonitorList) {
            if (abnormalEventMonitor != null
                    && mAbnormalEventMonitorList.contains(abnormalEventMonitor)) {
                return mAbnormalEventMonitorList.remove(abnormalEventMonitor);
            }
        }
        return false;
    }

    @Override
    public void update(Observable o, Object arg) {
        synchronized (mAbnormalEventMonitorList) {
            for (IAbnormalEventMonitor abnormalEventMonitor : mAbnormalEventMonitorList) {
                abnormalEventMonitor.abnormalEvenHappened(getLogType(),
                        AbnormalEvent.WRITE_FILE_FAILED);
            }
        }
    }
}
