package com.log.handler.instance;

import android.os.SystemProperties;

import com.log.handler.LogHandlerUtils.BTFWLogLevel;
import com.log.handler.LogHandlerUtils.LogType;
import com.log.handler.connection.ILogConnection;

/**
 * @author MTK81255
 *
 */
public class ConnsysFWLog extends AbstractLogInstance {
    public static final String PREFIX_SET_BTHOST_DEBUGLOG_ENABLE = "set_bthost_debuglog_enable";
    private static final String COMMAND_SET_BTFW_LOG_LEVEL = "set_btfw_log_level";
    private static final String SYSTEM_PROPERTY_LOG_STATUS = "vendor.connsysfw.running";

    /**
     * @param logConnection
     *            ILogConnection
     * @param logType
     *            LogType
     */
    public ConnsysFWLog(ILogConnection logConnection, LogType logType) {
        super(logConnection, logType);
    }

    @Override
    public String getLogStatusSystemProperty() {
        return SYSTEM_PROPERTY_LOG_STATUS;
    }

    @Override
    public synchronized boolean startLog(String logPath) {
        return startLog(logPath, BTFWLogLevel.SQC);
    }

    /**
     * @param logPath
     *            String
     * @param btFWLogLevel
     *            BTFWLogLevel
     * @return boolean
     */
    public boolean startLog(String logPath, BTFWLogLevel btFWLogLevel) {
        return super.startLog(logPath);
    }

    /**
     * @param btFWLogLevel
     *            BTFWLogLevel
     * @return boolean
     */
    public boolean setBTFWLogLevel(BTFWLogLevel btFWLogLevel) {
        return executeCommand(COMMAND_SET_BTFW_LOG_LEVEL + "," + btFWLogLevel);
    }

    public boolean isConnsysFWFeatureSupport() {
        return SystemProperties.get("ro.vendor.connsys.dedicated.log", "0").equals("1");
    }

}
