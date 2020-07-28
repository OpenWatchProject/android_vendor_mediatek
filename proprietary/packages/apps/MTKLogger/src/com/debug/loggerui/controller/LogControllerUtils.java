package com.debug.loggerui.controller;

import android.util.SparseArray;

import com.debug.loggerui.LogFolderListActivity;
import com.debug.loggerui.MyApplication;
import com.debug.loggerui.file.LogFileManager;
import com.debug.loggerui.taglog.TagLogUtils;
import com.debug.loggerui.utils.Utils;
import com.log.handler.LogHandler;
import com.log.handler.LogHandlerUtils.AbnormalEvent;
import com.log.handler.LogHandlerUtils.IAbnormalEventMonitor;
import com.log.handler.LogHandlerUtils.LogType;

import java.io.File;
import java.io.FileFilter;
import java.io.IOException;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

/**
 * @author MTK81255
 *
 */
public class LogControllerUtils {

    private static final String TAG = Utils.TAG + "/LogControllerUtils";

    public static final SparseArray<LogType> LOG_TYPE_INT_TO_OBJECT = new SparseArray<LogType>();
    static {
        LOG_TYPE_INT_TO_OBJECT.put(Utils.LOG_TYPE_MOBILE, LogType.MOBILE_LOG);
        LOG_TYPE_INT_TO_OBJECT.put(Utils.LOG_TYPE_MODEM, LogType.MODEM_LOG);
        LOG_TYPE_INT_TO_OBJECT.put(Utils.LOG_TYPE_NETWORK, LogType.NETWORK_LOG);
        LOG_TYPE_INT_TO_OBJECT.put(Utils.LOG_TYPE_MET, LogType.MET_LOG);
        LOG_TYPE_INT_TO_OBJECT.put(Utils.LOG_TYPE_GPSHOST, LogType.GPSHOST_LOG);
        LOG_TYPE_INT_TO_OBJECT.put(Utils.LOG_TYPE_CONNSYSFW, LogType.CONNSYSFW_LOG);
        LOG_TYPE_INT_TO_OBJECT.put(Utils.LOG_TYPE_BTHOST, LogType.BTHOST_LOG);
        LOG_TYPE_INT_TO_OBJECT.put(Utils.LOG_TYPE_CONNSYS, LogType.CONNSYSFW_LOG);
    }
    public static final Map<LogType, Integer> LOG_TYPE_OBJECT_TO_INT =
            new HashMap<LogType, Integer>();
    static {
        LOG_TYPE_OBJECT_TO_INT.put(LogType.MOBILE_LOG, Utils.LOG_TYPE_MOBILE);
        LOG_TYPE_OBJECT_TO_INT.put(LogType.MODEM_LOG, Utils.LOG_TYPE_MODEM);
        LOG_TYPE_OBJECT_TO_INT.put(LogType.NETWORK_LOG, Utils.LOG_TYPE_NETWORK);
        LOG_TYPE_OBJECT_TO_INT.put(LogType.MET_LOG, Utils.LOG_TYPE_MET);
        LOG_TYPE_OBJECT_TO_INT.put(LogType.GPSHOST_LOG, Utils.LOG_TYPE_GPSHOST);
        LOG_TYPE_OBJECT_TO_INT.put(LogType.CONNSYSFW_LOG, Utils.LOG_TYPE_CONNSYSFW);
        LOG_TYPE_OBJECT_TO_INT.put(LogType.BTHOST_LOG, Utils.LOG_TYPE_BTHOST);
    }

    /**
     * @param logTypeInt
     *            int
     * @return Set<LogType>
     */
    public static Set<LogType> convertLogTypeToObject(int logTypeInt) {
        Set<LogType> logTypeSet = new HashSet<LogType>();
        for (int logType : Utils.LOG_TYPE_SET) {
            if ((logType & logTypeInt) == 0) {
                continue;
            }
            logTypeSet.add(LOG_TYPE_INT_TO_OBJECT.get(logType));
        }
        return logTypeSet;
    }

    /**
     * @param logTypeSet
     *            Set<LogType>
     * @return int
     */
    public static int convertLogTypeToInt(Set<LogType> logTypeSet) {
        int logTypeInt = 0;
        for (LogType logType : logTypeSet) {
            logTypeInt += LOG_TYPE_OBJECT_TO_INT.get(logType);
        }
        return logTypeInt;
    }

    /**
     *
     */
    public static void registerLogAbnormalEventMonitor() {
        LogHandler.getInstance().registerLogAbnormalEventMonitor(new IAbnormalEventMonitor() {
            @Override
            public void abnormalEvenHappened(LogType logType, AbnormalEvent abnormalEvent) {
            }
        });
    }

    /**
     * @param logTypeInt
     *            int
     * @return AbstractLogController
     */
    public static AbstractLogController getLogControllerInstance(int logTypeInt) {
        return getLogControllerInstance(LOG_TYPE_INT_TO_OBJECT.get(logTypeInt));
    }

    /**
     * @param logType
     *            LogType
     * @return AbstractLogController
     */
    public static AbstractLogController getLogControllerInstance(LogType logType) {
        switch (logType) {
        case MOBILE_LOG:
            return MobileLogController.getInstance();
        case MODEM_LOG:
            return ModemLogController.getInstance();
        case NETWORK_LOG:
            return NetworkLogController.getInstance();
        case CONNSYSFW_LOG:
            return ConnsysFWLogController.getInstance();
        case GPSHOST_LOG:
            return GPSHostLogController.getInstance();
        case BTHOST_LOG:
            return BTHostLogController.getInstance();
        case MET_LOG:
            return METLogController.getInstance();
        default:
            return null;
        }
    }

    /**
     * @return boolean
     */
    public static boolean isAnyConnsysLogRunning() {
        boolean isAnyConnsysLogRunning = false;
        for (int logType : Utils.CONNSYS_LOG_TYPE_SET) {
            isAnyConnsysLogRunning = LogControllerUtils.getLogControllerInstance(logType)
                    .isLogRunning();
            if (isAnyConnsysLogRunning) {
                break;
            }
        }
        Utils.logi(TAG, "<--isAnyConnsysLogRunning()? " + isAnyConnsysLogRunning);
        return isAnyConnsysLogRunning;
    }

    /**
     * @return boolean
     */
    public static boolean isAnyControlledLogRunning() {
        boolean isAnyControlledLogRunning = false;
        for (int logType : Utils.LOG_TYPE_SET) {
            AbstractLogController logController = LogControllerUtils.getLogControllerInstance(
                    logType);
            isAnyControlledLogRunning = logController.isLogControlled()
                    && logController.isLogRunning();
            if (isAnyControlledLogRunning) {
                break;
            }
        }
        Utils.logi(TAG, "<--isAnyControlledLogRunning()? " + isAnyControlledLogRunning);
        return isAnyControlledLogRunning;
    }

    /**
     * @param clearLogFile File
     */
    public static void clearLogs(File clearLogFile) {
        try {
            String clearLogPath = clearLogFile.getCanonicalPath();
            Set<File> canbeDeletedFileList = getCanBeDeletedFileList();
            for (File file : canbeDeletedFileList) {
                if (file.getCanonicalPath().equals(clearLogPath)) {
                    LogFileManager.delete(clearLogFile);
                    break;
                }
            }
        } catch (IOException e) {
            Utils.logw(TAG, "clearLogs exception when get file getCanonicalPath!");
        }
    }

    /**
     * return void.
     */
    public static void clearAllLogs() {
        Set<File> canbeDeletedFileList = getCanBeDeletedFileList();
        for (File file : canbeDeletedFileList) {
            LogFileManager.delete(file);
        }
    }

    /**
     * @return Set<File>
     */
    public static Set<File> getCanBeDeletedFileList() {
        Set<File> deleteFileList = new HashSet<File>();
        String logParentPath = Utils.getCurrentLogPath() + Utils.LOG_PATH_PARENT;
        if (!new File(logParentPath).exists()) {
            Utils.logw(TAG, "getCanBeDeletedFileList()"
                    + " logParentPath = " + logParentPath + " is not exist!");
            return deleteFileList;
        }
        // Get normal log can be deleted files
        for (int logType : Utils.LOG_TYPE_SET) {
            if (logType == Utils.LOG_TYPE_MODEM) {
                continue; // Check modem log folder in the following loop
            }
            getLogFilteredFiles(
                    new File(logParentPath + File.separator + Utils.LOG_PATH_MAP.get(logType)),
                    LogControllerUtils.getLogControllerInstance(logType).getRunningLogPath(),
                    deleteFileList);
        }
        // Get modem log can be deleted files
        for (int modemIndex : Utils.MODEM_INDEX_SET) {
            String modemLogRunningPath = LogControllerUtils.getLogControllerInstance(
                    Utils.LOG_TYPE_MODEM).getRunningLogPath();
            if (!MyApplication.getInstance().getSharedPreferences().getBoolean(
                    LogFolderListActivity.IS_NEED_CLEAR_MD_BOOTUP_LOG_KEY, false)) {
                modemLogRunningPath += ";bootupLog";
            }
            getLogFilteredFiles(
                    new File(logParentPath + Utils.MODEM_INDEX_FOLDER_MAP.get(modemIndex)),
                    modemLogRunningPath, deleteFileList);
        }
        // Get c2k modem log can be deleted files
        getLogFilteredFiles(
                new File(logParentPath + Utils.C2K_MODEM_LOG_PATH),
                LogControllerUtils.getLogControllerInstance(Utils.LOG_TYPE_MODEM)
                    .getRunningLogPath(), deleteFileList);
        // Get TagLog can be deleted files
        File taglogFolder = new File(logParentPath + Utils.TAG_LOG_PATH);
        if (taglogFolder.exists()) {
            File[] filterFiles = taglogFolder.listFiles(new FileFilter() {
                @Override
                public boolean accept(File file) {
                    return file.getName().startsWith(TagLogUtils.TAGLOG_TEMP_FOLDER_PREFIX);
                }
            });
            String filterFilesName = "";
            if (filterFiles == null) {
                filterFilesName = null;
            } else {
                for (File filterFile : filterFiles) {
                    filterFilesName += filterFile.getName() + ";";
                }
            }
            getLogFilteredFiles(taglogFolder, filterFilesName, deleteFileList);
        } else {
            Utils.logw(TAG, "clearAllLogs() taglogFolder = " + taglogFolder + " is not exist!");
        }
        return deleteFileList;
    }

    private static void getLogFilteredFiles(File logFolder, String logRunningPath,
            Set<File> deleteFileList) {
        if (logFolder == null || !logFolder.exists()) {
            return;
        }
        Set<String> filterFileNames = new HashSet<String>();
        filterFileNames.add(Utils.LOG_TREE_FILE);
        if (logRunningPath != null) {
            String[] logRunningPaths = logRunningPath.split(";");
            for (String logPath : logRunningPaths) {
                filterFileNames.add(new File(logPath).getName());
            }
        }
        File[] deleteFileListArray = logFolder.listFiles(new FileFilter() {
            @Override
            public boolean accept(File file) {
                return !filterFileNames.contains(file.getName());
            }
        });
        if (deleteFileListArray != null && deleteFileListArray.length > 0) {
            Collections.addAll(deleteFileList, deleteFileListArray);
        }
    }
}
