package com.log.handler.instance;

import com.log.handler.LogHandlerUtils;
import com.log.handler.LogHandlerUtils.IModemEEMonitor;
import com.log.handler.LogHandlerUtils.LogType;
import com.log.handler.LogHandlerUtils.ModemLogMode;
import com.log.handler.LogHandlerUtils.ModemLogStatus;
import com.log.handler.connection.ILogConnection;
import com.log.handler.connection.LogSocketConnection;

import java.util.HashSet;
import java.util.Observable;
import java.util.Set;

/**
 * @author MTK81255
 *
 */
public class ModemLog extends AbstractLogInstance {
    private static final String TAG = LogHandlerUtils.TAG + "/ModemLog";
    // 93/95 modem chip is not used socket3
    private static final String MODEM_LOG_SERVER_NAME_3 = "com.mediatek.mdlogger.socket3";
    private ILogConnection mModem3LogConnection = new LogSocketConnection(MODEM_LOG_SERVER_NAME_3);
    // For 3G modem
    private static final String MODEM_LOG_SERVER_NAME_3G = "com.mediatek.mdlogger.socket";
    private ILogConnection mModem3GLogConnection =
            new LogSocketConnection(MODEM_LOG_SERVER_NAME_3G);
    // C2K memory dump process socket
    private static final String MODEM_LOG_SERVER_NAME_C2K_MEMORY_DUMP =
            "com.mediatek.mdlogger.socket3";
    private ILogConnection mC2KModemLogConnection =
            new LogSocketConnection(MODEM_LOG_SERVER_NAME_C2K_MEMORY_DUMP);
    private static final String SYSTEM_PROPERTY_LOG_STATUS = "vendor.mdlogger.Running";

    private static final String COMMAND_DEEP_PAUSE = "deep_pause";
    /**
     * Get Value of if ModemSpecialFeatureSupport. 1 means gps_location support 2 means CCB_feature
     * support 3 is 1 + 2 means gps_location & CCB_feature all support
     */
    private static final String COMMAND_IS_GPS_SUPPORT = "is_gps_support";
    private static final String COMMAND_ENABLE_GPS_LOCATION = "enable_gps_location";
    private static final String COMMAND_DISABLE_GPS_LOCATION = "disable_gps_location";
    private static final String COMMAND_GET_CCB_BUFFER_CONFIGURE_LIST = "get_ccb_gear_id_list";
    private static final String COMMAND_GET_CCB_GEAR_ID = "get_ccb_gear_id";
    private static final String COMMAND_SET_CCB_GEAR_ID = "set_ccb_gear_id";
    /**
     * Query modem log filter file information. Response format:
     * get_filter_info,filterpath;modifiedtime;filesize;
     */
    private static final String COMMAND_GET_FILTER_INFO = "get_filter_info";
    /**
     * After memory dump done, use this to reset modem.
     */
    private static final String COMMAND_RESET = "resetmd";
    private static final String COMMAND_SETAUTO = "setauto,";
    /**
     * Command for begin memory dump.
     */
    private static final String COMMAND_POLLING = "polling";
    /**
     * Query modem log current status. 0:pause, 1:common running, 2:polling, 3:copying.
     */
    public static final String COMMAND_GET_STATUS = "getstatus";
    private static final String COMMAND_SET_FLUSH_LOG_PATH = "set_flush_log_path";
    private static final String COMMAND_LOG_FLUSH = "log_flush";
    private static final String COMMAND_SET_EE_LOG_PATH = "set_ee_log_path";
    private static final String COMMAND_NOTIFY_TETHER_CHANGE = "usbtethering";
    private static final String COMMAND_SET_FILE_SIZE = "setfilesize,";
    private static final String COMMAND_SET_LOGSIZE = "setlogsize,";

    private static final String RESPONSE_START_MEMORY_DUMP = "MEMORYDUMP_START";
    private static final String RESPONSE_FINISH_MEMORY_DUMP = "MEMORYDUMP_DONE";
    private static final String RESPONSE_MEMORYDUMP_FILE = "MEMORYDUMP_FILE";

    private static final String[] COMMANDS_WITH_MODE = { COMMAND_START, COMMAND_SETAUTO };
    private String[] mModemLogModes;

    /**
     * @param logConnection
     *            ILogConnection
     * @param logType
     *            LogType
     */
    public ModemLog(ILogConnection logConnection, LogType logType) {
        super(logConnection, logType);
        doInit();
    }

    private void doInit() {
        if (!mModem3LogConnection.connect()) {
            mModem3LogConnection = null;
        }
        if (!isC2KModemSupport()) {
            mC2KModemLogConnection = null;
        } else {
            mC2KModemLogConnection.connect();
        }
        if (!mModem3GLogConnection.connect()) {
            mModem3GLogConnection = null;
        } else {
            mLogConnection = mModem3GLogConnection;
        }
        //if connection != null, need observer response from daemon.
        if (mModem3LogConnection != null) {
            mModem3LogConnection.addServerObserver(this);
        }
        if (mC2KModemLogConnection != null) {
            mC2KModemLogConnection.addServerObserver(this);
        }
        if (mModem3GLogConnection != null) {
            mModem3GLogConnection.addServerObserver(this);
        }
    }

    private boolean isC2KModemSupport() {
        return false;
    }

    @Override
    public boolean executeCommand(String command, boolean isWaitingResponse) {
        if (mModem3LogConnection != null) {
            String md3Command = command;
            for (String commandWithMode : COMMANDS_WITH_MODE) {
                if (md3Command.startsWith(commandWithMode) && mModemLogModes.length > 1) {
                    md3Command = commandWithMode + (commandWithMode.endsWith(",") ? "" : ",")
                            + mModemLogModes[1];
                    break;
                }
            }
            mModem3LogConnection.sendToServer(md3Command);
        }
        return super.executeCommand(command, isWaitingResponse);
    }

    @Override
    public String getLogStatusSystemProperty() {
        return SYSTEM_PROPERTY_LOG_STATUS;
    }

    @Override
    public synchronized boolean startLog(String logPath) {
        return startLog(logPath, ModemLogMode.SD);
    }
    @Override
    public boolean setLogRecycleSize(int logSize) {
        return executeCommand(COMMAND_SET_LOGSIZE + logSize);
    }

    /**
     * @param logPath
     *            String
     * @param modemMode
     *            ModemLogMode
     * @return boolean
     */
    public boolean startLog(String logPath, ModemLogMode modemMode) {
        mModemLogModes = modemMode.toString().split("_");
        String startCommand = COMMAND_START + "," + mModemLogModes[0];
        setStartCommand(startCommand);
        return super.startLog(logPath);
    }

    @Override
    public synchronized boolean stopLog() {
        return executeCommand(COMMAND_DEEP_PAUSE, true);
    }

    /**
     * Get Value of is_gps_support. 1 means gps_location support 2 means CCB_feature support 3 is 1
     * + 2 means gps_location & CCB_feature all support
     */
    /**
     * @return boolean
     */
    public boolean isSaveGPSLocationFeatureSupport() {
        String isFeatureSupportValue = getValueFromServer(COMMAND_IS_GPS_SUPPORT);
        return "1".equals(isFeatureSupportValue) || "3".equals(isFeatureSupportValue);
    }

    /**
     * @param enable
     *            boolean
     * @return boolean
     */
    public boolean setSaveGPSLocationToModemLog(boolean enable) {
        String command = enable ? COMMAND_ENABLE_GPS_LOCATION : COMMAND_DISABLE_GPS_LOCATION;
        return executeCommand(command);
    }

    /**
     * Get Value of is_gps_support. 1 means gps_location support 2 means CCB_feature support 3 is 1
     * + 2 means gps_location & CCB_feature all support
     */
    /**
     * @return boolean
     */
    public boolean isCCBBufferFeatureSupport() {
        String isFeatureSupportValue = getValueFromServer(COMMAND_IS_GPS_SUPPORT);
        return "2".equals(isFeatureSupportValue) || "3".equals(isFeatureSupportValue);
    }

    /**
     * @return Format as : gear_id(ControlBuffer,RawBuffer);gear_id(ControlBuffer,RawBuffer) like
     *         0(2,20);1(2,20);2(2,10);3(0,0);4(2,30);5(2,6.5)
     */
    public String getCCBBufferConfigureList() {
        return getValueFromServer(COMMAND_GET_CCB_BUFFER_CONFIGURE_LIST);
    }

    /**
     * @return String
     */
    public String getCCBBufferGearID() {
        return getValueFromServer(COMMAND_GET_CCB_GEAR_ID);
    }

    /**
     * @param id
     *            String
     * @return boolean
     */
    public boolean setCCBBufferGearID(String id) {
        String command = COMMAND_SET_CCB_GEAR_ID + "," + id;
        return executeCommand(command);
    }

    public String getFilterFileInformation() {
        return getValueFromServer(COMMAND_GET_FILTER_INFO);
    }

    /**
     * @return boolean
     */
    public boolean resetModem() {
        return executeCommand(COMMAND_RESET);
    }

    /**
     * @return boolean
     */
    public boolean forceModemAssert() {
        return super.executeCommand(COMMAND_POLLING, false);
    }

    /**
     * @return ModemLogStatus
     */
    public ModemLogStatus getStatus() {
        String statusStr = getValueFromServer(COMMAND_GET_STATUS);
        int statusId = 0;
        try {
            statusId = Integer.parseInt(statusStr);
        } catch (NumberFormatException nfe) {
            statusId = 0;
        }
        return ModemLogStatus.getModemLogStatusById(statusId);
    }

    /**
     * @return boolean
     */
    public boolean notifyUSBModeChanged() {
        return executeCommand(COMMAND_NOTIFY_TETHER_CHANGE);
    }

    /**
     * @param command
     *            String
     * @return boolean
     */
    public boolean sendCommandToServer(String command) {
        return executeCommand(command, true);
    }

    /**
     * @param size
     *            int
     * @return boolean
     */
    public boolean setModemLogFileSize(int size) {
        return executeCommand(COMMAND_SET_FILE_SIZE + size);
    }

    /**
     * @param flushLogPath
     *            String
     * @return String
     */
    public String triggerPLSModeFlush(String flushLogPath) {
        if (flushLogPath != null && !flushLogPath.isEmpty()) {
            executeCommand(COMMAND_SET_FLUSH_LOG_PATH + "," + flushLogPath, true);
        }
        return getValueFromServer(COMMAND_LOG_FLUSH);
    }

    /**
     * @param modemEEPath
     *            String
     * @return boolean
     */
    public boolean setModemEEPath(String modemEEPath) {
        return executeCommand(COMMAND_SET_EE_LOG_PATH + "," + modemEEPath);
    }

    private Set<IModemEEMonitor> mModemEEMonitorList = new HashSet<IModemEEMonitor>();

    /**
     * @param modemEEMonitor
     *            IModemEEMonitor
     * @return boolean
     */
    public boolean registerModemEEMonitor(IModemEEMonitor modemEEMonitor) {
        synchronized (mModemEEMonitorList) {
            if (modemEEMonitor != null) {
                // If need do register, log must be connected.
                if (!mLogConnection.isConnection() && !mLogConnection.connect()) {
                    return false;
                }
                return mModemEEMonitorList.add(modemEEMonitor);
            }
            return false;
        }
    }

    /**
     * @param modemEEMonitor
     *            IModemEEMonitor
     * @return boolean
     */
    public boolean unregisterModemEEMonitor(IModemEEMonitor modemEEMonitor) {
        synchronized (mModemEEMonitorList) {
            if (modemEEMonitor != null && mModemEEMonitorList.contains(modemEEMonitor)) {
                return mModemEEMonitorList.remove(modemEEMonitor);
            }
            return false;
        }
    }

    @Override
    public boolean setBootupLogSaved(boolean enable) {
        return setBootupLogSaved(enable, ModemLogMode.SD);
    }

    /**
     * @param enable
     *            boolean
     * @param modemMode
     *            ModemLogMode
     * @return boolean
     */
    public boolean setBootupLogSaved(boolean enable, ModemLogMode modemMode) {
        mModemLogModes = modemMode.toString().split("_");
        return executeCommand(COMMAND_SETAUTO + (enable ? mModemLogModes[0] : 0));
    }

    private String mFirstModemEEPath = "";

    @Override
    public void update(Observable o, Object arg) {
        String serverResponseStr = "";
        if (arg != null && arg instanceof String) {
            serverResponseStr = (String) arg;
        }
        LogHandlerUtils.logi(TAG, "update, serverResponseStr = " + serverResponseStr);
        String notifyStr = "";
        if (serverResponseStr.startsWith(RESPONSE_FINISH_MEMORY_DUMP)) {
            String modemEEPath =
                    serverResponseStr.substring(RESPONSE_FINISH_MEMORY_DUMP.length() + 1);
            if (isDualModemLogSupport()) {
                if (mFirstModemEEPath.isEmpty()) {
                    mFirstModemEEPath = modemEEPath;
                    return;
                } else {
                    modemEEPath = mFirstModemEEPath + ";" + modemEEPath;
                }
            }
            mFirstModemEEPath = "";
            notifyStr = modemEEPath;
        } else if (serverResponseStr.startsWith(RESPONSE_START_MEMORY_DUMP)) {
            notifyStr = RESPONSE_START_MEMORY_DUMP;
        } else if (serverResponseStr.startsWith("need_dump_file")) {
            notifyStr = "need_dump_file";
        } else if (serverResponseStr.startsWith(RESPONSE_MEMORYDUMP_FILE)) {
            notifyStr = serverResponseStr;
        }
        if (!notifyStr.isEmpty()) {
            for (IModemEEMonitor modemEEMonitor : mModemEEMonitorList) {
                modemEEMonitor.modemEEHappened(notifyStr);
            }
        }
        super.update(o, arg);
    }

    public boolean isDualModemLogSupport() {
        return mModem3LogConnection != null || mC2KModemLogConnection != null;
    }
}
