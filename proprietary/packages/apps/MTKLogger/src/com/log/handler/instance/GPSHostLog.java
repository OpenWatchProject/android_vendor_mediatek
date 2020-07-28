package com.log.handler.instance;

import com.log.handler.LogHandlerUtils.LogType;
import com.log.handler.connection.ILogConnection;

/**
 * @author MTK81255
 *
 */
public class GPSHostLog extends AbstractLogInstance {

    private static final String SYSTEM_PROPERTY_LOG_STATUS = "vendor.gpsdbglog.enable";

    /**
     * @param logConnection
     *            ILogConnection
     * @param logType
     *            LogType
     */
    public GPSHostLog(ILogConnection logConnection, LogType logType) {
        super(logConnection, logType);
    }

    @Override
    public String getLogStatusSystemProperty() {
        return SYSTEM_PROPERTY_LOG_STATUS;
    }
}
