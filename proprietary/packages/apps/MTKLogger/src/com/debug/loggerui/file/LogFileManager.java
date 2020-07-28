package com.debug.loggerui.file;

import com.debug.loggerui.utils.Utils;

import java.io.File;
import java.io.FileOutputStream;
import java.util.Set;

/**
 * @author MTK81255
 *
 */
public class LogFileManager {
    private static final String TAG = Utils.TAG + "/LogFileManager";

    /**
     * @param file
     *            File
     * @return boolean
     */
    public static boolean delete(File file) {
        return delete(file, null);
    }

    /**
     * @param file File
     * @param filterFileNames Set<String>
     * @return boolean
     */
    public static boolean delete(File file, Set<String> filterFileNames) {
        Utils.logd(TAG, "delete(), file.getAbsolutePath() = " + file.getAbsolutePath());
        ILogFile logFile = getLogFileInstance();
        if (logFile != null) {
            return logFile.delete(file, filterFileNames);
        }
        return false;
    }

    private static ILogFile getLogFileInstance() {
        String currentLogPathType = Utils.getCurrentLogPathType();
        switch (currentLogPathType) {
        case Utils.LOG_PATH_TYPE_SYSTEM_DATA:
            // After make as System Application, it can access data partition directly
            return DeviceStorageLogFile.getInstance();
        case Utils.LOG_PATH_TYPE_DEVICE_STORAGE:
        case Utils.LOG_PATH_TYPE_PORTABLE_STORAGE:
            return DeviceStorageLogFile.getInstance();
        default:
            return null;
        }
    }

    /**
     * @param file
     *            File
     * @return boolean
     */
    public static boolean createNewFile(File file) {
        ILogFile logFile = getLogFileInstance();
        if (logFile != null) {
            return logFile.createNewFile(file);
        }
        return false;
    }

    /**
     * @param file
     *            File
     * @return boolean
     */
    public static boolean mkdir(File file) {
        ILogFile logFile = getLogFileInstance();
        if (logFile != null) {
            return logFile.mkdir(file);
        }
        return false;
    }

    /**
     * @param file
     *            File
     * @return boolean
     */
    public static boolean mkdirs(File file) {
        ILogFile logFile = getLogFileInstance();
        if (logFile != null) {
            return logFile.mkdirs(file);
        }
        return false;
    }

    /**
     * @param file
     *            File
     * @param dest
     *            File
     * @return boolean
     */
    public static boolean renameTo(File file, File dest) {
        ILogFile logFile = getLogFileInstance();
        if (logFile != null) {
            return logFile.renameTo(file, dest);
        }
        return false;
    }

    /**
     * @param file
     *            File
     * @return FileOutputStream
     */
    public static FileOutputStream getFileOutputStream(File file) {
        ILogFile logFile = getLogFileInstance();
        if (logFile != null) {
            return logFile.getFileOutputStream(file);
        }
        return null;
    }

}
