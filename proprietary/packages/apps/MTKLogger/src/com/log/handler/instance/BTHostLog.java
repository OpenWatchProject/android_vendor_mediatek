package com.log.handler.instance;

import com.log.handler.LogHandlerUtils.BTFWLogLevel;
import com.log.handler.LogHandlerUtils.LogType;
import com.log.handler.connection.ILogConnection;

/**
 * @author MTK81255
 *
 */
public class BTHostLog extends AbstractLogInstance {

    private static final String SYSTEM_PROPERTY_LOG_STATUS = "vendor.bthcisnoop.running";
    private static final String COMMAND_SET_BTHOST_DEBUGLOG_ENABLE = "set_bthost_debuglog_enable";
    private static final String COMMAND_SET_BTFW_LOG_LEVEL = "set_btfw_log_level";

    /**
     * @param logConnection
     *            ILogConnection
     * @param logType
     *            LogType
     */
    public BTHostLog(ILogConnection logConnection, LogType logType) {
        super(logConnection, logType);
    }

    @Override
    public String getLogStatusSystemProperty() {
        return SYSTEM_PROPERTY_LOG_STATUS;
    }

    /**
     * @param enable
     *            boolean
     * @return boolean
     */
    public boolean setBTHostDebuglogEnable(boolean enable) {
        return executeCommand(COMMAND_SET_BTHOST_DEBUGLOG_ENABLE + "," + (enable ? "1" : "0"));
    }

    /**
     * @param btFWLogLevel
     *            BTFWLogLevel
     * @return boolean
     */
    public boolean setBTFWLogLevel(BTFWLogLevel btFWLogLevel) {
        return executeCommand(COMMAND_SET_BTFW_LOG_LEVEL + "," + btFWLogLevel);
    }

}
