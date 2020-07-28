package com.debug.loggerui.controller;

import com.log.handler.LogHandler;
import com.log.handler.LogHandlerUtils.ILogExecute;
import com.log.handler.LogHandlerUtils.LogType;

import java.util.HashSet;
import java.util.Set;

/**
 * @author MTK81255
 *
 */
public class MultiLogTypesController {

    public static MultiLogTypesController sInstance = new MultiLogTypesController();

    private MultiLogTypesController() {
    }

    public static MultiLogTypesController getInstance() {
        return sInstance;
    }

    /**
     * @param logTypes
     *            int
     * @param logPath
     *            String
     * @return boolean boolean
     */
    public boolean startTypeLogs(int logTypes, String logPath) {
        return LogHandler.getInstance().executeMultiLogThreads(
                LogControllerUtils.convertLogTypeToObject(logTypes), 15000, new ILogExecute() {
                    @Override
                    public boolean execute(LogType logType) {
                        return LogControllerUtils.getLogControllerInstance(logType)
                                .startLog(logPath);
                    }
                });
    }

    /**
     * @param logTypes
     *            int
     * @return boolean
     */
    public boolean stopTypeLogs(int logTypes) {
        return LogHandler.getInstance().executeMultiLogThreads(
                LogControllerUtils.convertLogTypeToObject(logTypes), 15000, new ILogExecute() {
                    @Override
                    public boolean execute(LogType logType) {
                        return LogControllerUtils.getLogControllerInstance(logType).stopLog();
                    }
                });
    }

    /**
     * @param logTypes
     *            int
     * @param logPath
     *            String
     * @return boolean
     */
    public boolean rebootTypeLogs(int logTypes, String logPath) {
        return LogHandler.getInstance().executeMultiLogThreads(
                LogControllerUtils.convertLogTypeToObject(logTypes), 30000, new ILogExecute() {
                    @Override
                    public boolean execute(LogType logType) {
                        return LogControllerUtils.getLogControllerInstance(logType)
                                .rebootLog(logPath);
                    }
                });
    }

    /**
     * @return boolean
     */
    public Set<LogType> getAllRunningLogTypes() {
        Set<LogType> allRunningLogTypes = new HashSet<LogType>();
        LogHandler.getInstance().executeMultiLogThreads(
                LogType.getAllLogTypes(), 30000, new ILogExecute() {
                @Override
                public boolean execute(LogType logType) {
                    if (LogControllerUtils
                            .getLogControllerInstance(logType)
                            .isLogRunning()) {
                        synchronized (allRunningLogTypes) {
                            allRunningLogTypes
                                    .add(logType);
                        }
                    }
                    return true;
                }
            });
        return allRunningLogTypes;
    }
}
