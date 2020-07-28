package com.log.handler.instance;

import com.log.handler.LogHandlerUtils.LogType;
import com.log.handler.connection.ILogConnection;

/**
 * @author MTK81255
 *
 */
public class NetworkLog extends AbstractLogInstance {

    private static final String SYSTEM_PROPERTY_LOG_STATUS = "vendor.mtklog.netlog.Running";

    private static final String COMMAND_NETWORKLOG_START = "tcpdump_sdcard_start";
    private static final String COMMAND_NETWORKLOG_STOP = "tcpdump_sdcard_stop";
    private static final String COMMAND_NETWORKLOG_STOP_WITHOUT_PING = "tcpdump_sdcard_stop_noping";
    private static final String COMMAND_IS_ROHC_COMPRESSION_SUPPORT = "is_rohc_compression_support";
    private static final String COMMAND_ENABLE_ROHC_COMPRESSION = "enable_rohc_compression";
    private static final String COMMAND_SET_ROHC_TOTAL_FILE = "set_rohc_total_file";
    private static final int DEFAULT_LOG_RECYCLE_SIZE = 600;

    /**
     * @param logConnection
     *            ILogConnection
     * @param logType
     *            LogType
     */
    public NetworkLog(ILogConnection logConnection, LogType logType) {
        super(logConnection, logType);
    }

    @Override
    public String getLogStatusSystemProperty() {
        return SYSTEM_PROPERTY_LOG_STATUS;
    }

    @Override
    public synchronized boolean startLog(String logPath) {
        return startLog(logPath, 600, 90);
    }

    /**
     * @param logPath
     *            String
     * @param recycleSize
     *            int
     * @param packageSize
     *            int
     * @return boolean
     */
    public synchronized boolean startLog(String logPath, int recycleSize, int packageSize) {
        if (recycleSize < 100) {
            recycleSize = DEFAULT_LOG_RECYCLE_SIZE;
        }
        String startCommand = COMMAND_NETWORKLOG_START + "_" + recycleSize;
        if (packageSize > 0) {
            startCommand += ",-s" + packageSize;
        }
        setStartCommand(startCommand);
        return super.startLog(logPath);
    }

    @Override
    public synchronized boolean stopLog() {
        return stopLog(false);
    }

    /**
     * @param isCheckEnvironment
     *            boolean
     * @return boolean
     */
    public boolean stopLog(boolean isCheckEnvironment) {
        return executeCommand(
                isCheckEnvironment ? COMMAND_NETWORKLOG_STOP : COMMAND_NETWORKLOG_STOP_WITHOUT_PING,
                true);
    }

    @Override
    public boolean setBootupLogSaved(boolean enable) {
        // For network log, no need set.
        return true;
    }

    @Override
    public boolean setLogRecycleSize(int logSize) {
        // For network log, no need set.
        return true;
    }

    /**
     * "is_rohc_compression_support,1" is support and "is_rohc_compression_support,0" is not
     * support.
     *
     * @return boolean
     */
    public boolean isRohcCompressionSupport() {
        String isSupportValue = getValueFromServer(COMMAND_IS_ROHC_COMPRESSION_SUPPORT);
        return isSupportValue.equals("1");
    }

    /**
     * @param enable
     *            boolean
     * @return boolean
     */
    public boolean enableRohcCompression(boolean enable) {
        return executeCommand(COMMAND_ENABLE_ROHC_COMPRESSION + "," + (enable ? "1" : "0"));
    }

    /**
     * @param totalNumber
     *            int
     * @return boolean
     */
    public boolean setRohcTotalFileNumber(int totalNumber) {
        return executeCommand(COMMAND_SET_ROHC_TOTAL_FILE + "," + totalNumber);
    }
}
