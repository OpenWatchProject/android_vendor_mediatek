package com.log.handler;

import com.log.handler.LogHandlerUtils.BTFWLogLevel;
import com.log.handler.LogHandlerUtils.IAbnormalEventMonitor;
import com.log.handler.LogHandlerUtils.ILogExecute;
import com.log.handler.LogHandlerUtils.IModemEEMonitor;
import com.log.handler.LogHandlerUtils.LogType;
import com.log.handler.LogHandlerUtils.MobileLogSubLog;
import com.log.handler.LogHandlerUtils.ModemLogMode;
import com.log.handler.LogHandlerUtils.ModemLogStatus;

import java.util.Set;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

/**
 * @author MTK81255
 *
 */
public class LogHandler {
    private static LogHandler sInstance = new LogHandler();

    public static LogHandler getInstance() {
        return sInstance;
    }

    // Common API
    /**
     * @param logType
     *            LogType
     * @param logPath
     *            String
     * @return boolean
     */
    public boolean startTypeLog(LogType logType, String logPath) {
        return LogFactory.getTypeLogInstance(logType).startLog(logPath);
    }

    /**
     * @param logType
     *            LogType
     * @return boolean
     */
    public boolean stopTypeLog(LogType logType) {
        return LogFactory.getTypeLogInstance(logType).stopLog();
    }

    /**
     * @param logType
     *            LogType
     * @return boolean
     */
    public boolean isTypeLogRunning(LogType logType) {
        return LogFactory.getTypeLogInstance(logType).isLogRunning();
    }

    /**
     * @param logType
     *            LogType
     * @param logSize
     *            int
     * @return boolean
     */
    public boolean setTypeLogRecycleSize(LogType logType, int logSize) {
        return LogFactory.getTypeLogInstance(logType).setLogRecycleSize(logSize);
    }

    /**
     * @param logType
     *            LogType
     * @param enable
     *            boolean
     * @return boolean
     */
    public boolean setBootupTypeLogSaved(LogType logType, boolean enable) {
        return LogFactory.getTypeLogInstance(logType).setBootupLogSaved(enable);
    }

    /**
     * @param abnormalEventMonitor
     *            IAbnormalEventMonitor
     * @return boolean
     */
    public boolean registerLogAbnormalEventMonitor(IAbnormalEventMonitor abnormalEventMonitor) {
        for (LogType logType : LogType.getAllLogTypes()) {
            LogFactory.getTypeLogInstance(logType)
                    .registerAbnormalEventMonitor(abnormalEventMonitor);
        }
        return true;
    }

    /**
     * @param abnormalEventMonitor
     *            IAbnormalEventMonitor
     * @return boolean
     */
    public boolean unregisterLogAbnormalEventMonitor(IAbnormalEventMonitor abnormalEventMonitor) {
        for (LogType logType : LogType.getAllLogTypes()) {
            LogFactory.getTypeLogInstance(logType)
                    .unregisterAbnormalEventMonitor(abnormalEventMonitor);
        }
        return true;
    }

    private boolean mIsExecuteSuccess = true;

    /**
     * @param logTypeSet
     *            Set<LogType>
     * @param timeout
     *            long
     * @param logExecute
     *            ILogExecute
     * @return boolean
     */
    public boolean executeMultiLogThreads(Set<LogType> logTypeSet, long timeout,
            ILogExecute logExecute) {
        if (logTypeSet == null || logTypeSet.size() == 0) {
            return false;
        }
        if (logTypeSet.size() == 1) {
            return logExecute.execute((LogType) logTypeSet.toArray()[0]);
        }
        mIsExecuteSuccess = true;
        ExecutorService executorPool = Executors.newCachedThreadPool();
        for (LogType logType : logTypeSet) {
            Thread thread = new Thread(new Runnable() {
                @Override
                public void run() {
                    if (!logExecute.execute(logType)) {
                        mIsExecuteSuccess = false;
                    }
                }
            });
            executorPool.execute(thread);
        }
        executorPool.shutdown();
        try {
            executorPool.awaitTermination(timeout, TimeUnit.MILLISECONDS);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        return mIsExecuteSuccess;
    }

    // MobileLog API
    /**
     * @param subMobileLog
     *            MobileLogSubLog
     * @param enable
     *            boolean
     * @return boolean
     */
    public boolean setSubMobileLogEnable(MobileLogSubLog subMobileLog, boolean enable) {
        return LogFactory.getMobileLogInstance().setSubLogEnable(subMobileLog.toString(), enable);
    }

    /**
     * @param size
     *            int
     * @return boolean
     */
    public boolean setMobileLogTotalRecycleSize(int size) {
        return LogFactory.getMobileLogInstance().setTotalRecycleSize(size);
    }

    // ModemLog API
    /**
     * @param logPath
     *            String
     * @param modemLogMode
     *            ModemLogMode
     * @return boolean
     */
    public boolean startModemLog(String logPath, ModemLogMode modemLogMode) {
        return LogFactory.getModemLogInstance().startLog(logPath, modemLogMode);
    }

    /**
     * @return boolean
     */
    public boolean isSaveGPSLocationFeatureSupport() {
        return LogFactory.getModemLogInstance().isSaveGPSLocationFeatureSupport();
    }

    /**
     * @param enable
     *            boolean
     * @return boolean
     */
    public boolean setSaveGPSLocationToModemLog(boolean enable) {
        return LogFactory.getModemLogInstance().setSaveGPSLocationToModemLog(enable);
    }

    /**
     * @return boolean
     */
    public boolean isCCBBufferFeatureSupport() {
        return LogFactory.getModemLogInstance().isCCBBufferFeatureSupport();
    }

    /**
     * @return String
     */
    public String getCCBBufferConfigureList() {
        return LogFactory.getModemLogInstance().getCCBBufferConfigureList();
    }

    /**
     * @return String
     */
    public String getCCBBufferGearID() {
        return LogFactory.getModemLogInstance().getCCBBufferGearID();
    }

    /**
     * @param id
     *            String
     * @return boolean
     */
    public boolean setCCBBufferGearID(String id) {
        return LogFactory.getModemLogInstance().setCCBBufferGearID(id);
    }

    /**
     * @return String
     */
    public String getModemLogFilterFileInformation() {
        return LogFactory.getModemLogInstance().getFilterFileInformation();
    }

    /**
     * @return boolean
     */
    public boolean resetModem() {
        return LogFactory.getModemLogInstance().resetModem();
    }

    /**
     * @return boolean
     */
    public boolean forceModemAssert() {
        return LogFactory.getModemLogInstance().forceModemAssert();
    }

    /**
     * @return String
     */
    public String triggerModemLogPLSModeFlush() {
        return triggerModemLogPLSModeFlush(null);
    }

    /**
     * @param logPath
     *            String
     * @return String
     */
    public String triggerModemLogPLSModeFlush(String logPath) {
        return LogFactory.getModemLogInstance().triggerPLSModeFlush(logPath);
    }

    /**
     * @param modemEELogPath
     *            String
     * @return boolean
     */
    public boolean setModemEEPath(String modemEELogPath) {
        return LogFactory.getModemLogInstance().setModemEEPath(modemEELogPath);
    }

    /**
     * @param modemEEMonitor
     *            IModemEEMonitor
     * @return boolean
     */
    public boolean registerModemEEMonitor(IModemEEMonitor modemEEMonitor) {
        return LogFactory.getModemLogInstance().registerModemEEMonitor(modemEEMonitor);
    }

    /**
     * @param modemEEMonitor
     *            IModemEEMonitor
     * @return boolean
     */
    public boolean unregisterModemEEMonitor(IModemEEMonitor modemEEMonitor) {
        return LogFactory.getModemLogInstance().unregisterModemEEMonitor(modemEEMonitor);
    }

    public ModemLogStatus getModemLogStatus() {
        return LogFactory.getModemLogInstance().getStatus();
    }

    /**
     * @return boolean
     */
    public boolean notifyUSBModeChanged() {
        return LogFactory.getModemLogInstance().notifyUSBModeChanged();
    }

    /**
     * @param commandStr
     *            String
     * @return boolean
     */
    public boolean sendCommandToModemLog(String commandStr) {
        return LogFactory.getModemLogInstance().sendCommandToServer(commandStr);
    }

    /**
     * @param size
     *            int
     * @return boolean
     */
    public boolean setModemLogFileSize(int size) {
        return LogFactory.getModemLogInstance().setModemLogFileSize(size);
    }

    /**
     * @param enable
     *            boolean
     * @param modemMode
     *            ModemLogMode
     * @return boolean
     */
    public boolean setBootupLogSaved(boolean enable, ModemLogMode modemMode) {
        return LogFactory.getModemLogInstance().setBootupLogSaved(enable, modemMode);
    }

    // NetworkLog API
    /**
     * @param logPath
     *            String
     * @param recycleSize
     *            int
     * @return boolean
     */
    public boolean startNetworkLogWithRecycleSize(String logPath, int recycleSize) {
        return startNetworkLog(logPath, recycleSize, 90);
    }

    /**
     * @param logPath
     *            String
     * @param packageSize
     *            int
     * @return boolean
     */
    public boolean startNetworkLogWithPackageSize(String logPath, int packageSize) {
        return startNetworkLog(logPath, 600, packageSize);
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
    public boolean startNetworkLog(String logPath, int recycleSize, int packageSize) {
        return LogFactory.getNetworkLogInstance().startLog(logPath, recycleSize, packageSize);
    }

    /**
     * @param isCheckEnvironment
     *            boolean
     * @return boolean
     */
    public boolean stopNetworkLog(boolean isCheckEnvironment) {
        return LogFactory.getNetworkLogInstance().stopLog(isCheckEnvironment);
    }

    /**
     * @return boolean
     */
    public boolean isNetworkLogRohcCompressionSupport() {
        return LogFactory.getNetworkLogInstance().isRohcCompressionSupport();
    }

    /**
     * @param enable
     *            boolean
     * @return boolean
     */
    public boolean enableNetworkLogRohcCompression(boolean enable) {
        return LogFactory.getNetworkLogInstance().enableRohcCompression(enable);
    }

    /**
     * @param totalNumber
     *            int
     * @return boolean
     */
    public boolean setNetworkLogRohcTotalFileNumber(int totalNumber) {
        return LogFactory.getNetworkLogInstance().setRohcTotalFileNumber(totalNumber);
    }

    // MetLog API
    public boolean isMETLogFeatureSupport() {
        return LogFactory.getMETLogInstance().isMETLogFeatureSupport();
    }

    /**
     * @param periodSize
     *            int
     * @return boolean
     */
    public boolean setMETLogPeriod(int periodSize) {
        return LogFactory.getMETLogInstance().setMETLogPeriod(periodSize);
    }

    /**
     * @param bufferSize
     *            int
     * @return boolean
     */
    public boolean setMETLogCPUBuffer(int bufferSize) {
        return LogFactory.getMETLogInstance().setMETLogCPUBuffer(bufferSize);
    }

    /**
     * @param sspmSize
     *            int
     * @return boolean
     */
    public boolean setMETLogSSPMSize(int sspmSize) {
        return LogFactory.getMETLogInstance().setMETLogSSPMSize(sspmSize);
    }

    /**
     * @param enable
     *            boolean
     * @return boolean
     */
    public boolean setHeavyLoadRecordingEnable(boolean enable) {
        return LogFactory.getMETLogInstance().setHeavyLoadRecordingEnable(enable);
    }

    public String getMETLogInitValues() {
        return LogFactory.getMETLogInstance().getMETLogInitValues();
    }

    // GPSHostLog API
    // All include in common API

    // ConnsysFWLog API
    /**
     * @param logPath
     *            String
     * @param btFWLogLevel
     *            BTFWLogLevel
     * @return boolean
     */
    public boolean startConnsysFWLog(String logPath, BTFWLogLevel btFWLogLevel) {
        return LogFactory.getConnsysFWLogInstance().startLog(logPath, btFWLogLevel);
    }

    /**
     * @param enable
     *            boolean
     * @return boolean
     */
    public boolean setConnsysFWLogDuringBootupSaved(boolean enable) {
        return LogFactory.getConnsysFWLogInstance().setBootupLogSaved(enable);
    }

    public boolean isConnsysFWFeatureSupport() {
        return LogFactory.getConnsysFWLogInstance().isConnsysFWFeatureSupport();
    }

    // BTHost API
    /**
     * @param enable
     *            boolean
     * @return boolean
     */
    public boolean setBTHostDebuglogEnable(boolean enable) {
        return LogFactory.getBTHostLogInstance().setBTHostDebuglogEnable(enable);
    }

    /**
     * @param btFWLogLevel
     *            BTFWLogLevel
     * @return boolean
     */
    public boolean setBTFWLogLevel(BTFWLogLevel btFWLogLevel) {
        if (isConnsysFWFeatureSupport()) {
            return LogFactory.getConnsysFWLogInstance().setBTFWLogLevel(btFWLogLevel);
        } else {
            return LogFactory.getBTHostLogInstance().setBTFWLogLevel(btFWLogLevel);
        }
    }

}
