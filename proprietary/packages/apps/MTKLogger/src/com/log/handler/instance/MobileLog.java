package com.log.handler.instance;

import com.log.handler.LogHandlerUtils.LogType;
import com.log.handler.connection.ILogConnection;

/**
 * @author MTK81255
 *
 */
public class MobileLog extends AbstractLogInstance {

    private static final String SYSTEM_PROPERTY_LOG_STATUS = "vendor.MB.running";

    /**
     * Flag for configuring native total log size parameter from Java.
     */
    private static final String PREFIX_CONFIG_TOTAL_LOG_SIZE = "totallogsize=";

    /**
     * Flag for configuring mobile log whether to record this kind of log, like. AndroidLog,
     * KernelLog
     */
    public static final String PREFIX_CONFIG_SUB_LOG = "sublog_";

    /**
     * @param logConnection
     *            ILogConnection
     * @param logType
     *            LogType
     */
    public MobileLog(ILogConnection logConnection, LogType logType) {
        super(logConnection, logType);
    }

    @Override
    public String getLogStatusSystemProperty() {
        return SYSTEM_PROPERTY_LOG_STATUS;
    }

    /**
     * @param logSize
     *            int
     * @return boolean
     */
    public boolean setTotalRecycleSize(int logSize) {
        return executeCommand(PREFIX_CONFIG_TOTAL_LOG_SIZE + logSize);
    }

    /**
     * @param subLogName
     *            String
     * @param enable
     *            boolean
     * @return boolean
     */
    public boolean setSubLogEnable(String subLogName, boolean enable) {
        return executeCommand(PREFIX_CONFIG_SUB_LOG + subLogName + "=" + (enable ? "1" : 0));
    }

}
