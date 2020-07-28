package com.log.handler.instance;

import com.log.handler.LogHandlerUtils.LogType;
import com.log.handler.connection.ILogConnection;

/**
 * @author MTK81255
 *
 */
public class METLog extends AbstractLogInstance {

    private static final String SYSTEM_PROPERTY_LOG_STATUS = "vendor.met.running";
    private static final String COMMAND_IS_METLOG_SUPPORT = "is_metlog_support";
    public static final String COMMAND_SET_CPU_BUFFER_SIZE = "cpu_buff_size=";
    public static final String COMMAND_SET_PEROID_SIZE = "period=";
    public static final String COMMAND_SET_MET_HEAVY_RECORD = "met_heavy_record=";
    public static final String COMMAND_SET_SSPM_SIZE = "set_sspmsize=";
    public static final String COMMAND_GET_METLOG_INIT_VALUES = "get_metlog_init_values";

    /**
     * @param logConnection
     *            ILogConnection
     * @param logType
     *            LogType
     */
    public METLog(ILogConnection logConnection, LogType logType) {
        super(logConnection, logType);
    }

    @Override
    public String getLogStatusSystemProperty() {
        return SYSTEM_PROPERTY_LOG_STATUS;
    }

    /**
     * Get Value of IS_METLOG_SUPPORT. 1 means METLOG SUPPORT 0 means METLOG not support
     */
    /**
     * @return return
     */
    public boolean isMETLogFeatureSupport() {
        String isFeatureSupportValue = getValueFromServer(COMMAND_IS_METLOG_SUPPORT);
        return "1".equals(isFeatureSupportValue);
    }

    /**
     * @param periodSize
     *            int
     * @return return
     */
    public boolean setMETLogPeriod(int periodSize) {
        return executeCommand(COMMAND_SET_PEROID_SIZE + periodSize);
    }

    /**
     * @param bufferSize
     *            int
     * @return return
     */
    public boolean setMETLogCPUBuffer(int bufferSize) {
        return executeCommand(COMMAND_SET_CPU_BUFFER_SIZE + bufferSize);
    }

    /**
     * @param sspmSize
     *            int
     * @return return
     */
    public boolean setMETLogSSPMSize(int sspmSize) {
        return executeCommand(COMMAND_SET_SSPM_SIZE + sspmSize);
    }

    /**
     * @param enable
     *            boolean
     * @return boolean
     */
    public boolean setHeavyLoadRecordingEnable(boolean enable) {
        return executeCommand(COMMAND_SET_MET_HEAVY_RECORD + (enable ? "1" : "0"));
    }

    public String getMETLogInitValues() {
        return getValueFromServer(COMMAND_GET_METLOG_INIT_VALUES);
    }

}
