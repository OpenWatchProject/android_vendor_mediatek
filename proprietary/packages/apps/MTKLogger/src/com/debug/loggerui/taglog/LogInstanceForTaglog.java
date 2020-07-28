package com.debug.loggerui.taglog;

import com.debug.loggerui.controller.LogControllerUtils;
import com.debug.loggerui.utils.Utils;

import java.io.File;
import java.util.List;
/**
 * @author MTK81255
 *
 */
public class LogInstanceForTaglog {
    protected static final String TAG = TagLogUtils.TAGLOG_TAG + "/LogInstanceForTaglog";

    protected int mLogType;
    protected TagLogInformation mTagLogInformation;

    /**
     * @param logType int
     * @param tagLogInformation TagLogInformation
     */
    public LogInstanceForTaglog(int logType, TagLogInformation tagLogInformation) {
        mLogType = logType;
        mTagLogInformation = tagLogInformation;
    }

    /**
     * @return boolean
     */
    public boolean isNeedDoTag() {
        int needLogType = mTagLogInformation.getNeedLogType();
        if (needLogType != 0 && (needLogType & mLogType) == 0) {
            Utils.logi(TAG, "isNeedDoTag ? false. mLogType = " + mLogType + ", needLogType = "
                    + needLogType);
            return false;
        }
        Utils.logi(TAG, "isNeedDoTag ? true. mLogType = " + mLogType);
        return true;
    }

    /**
     * @return boolean
     */
    public boolean isNeedRestart() {
        if (!isNeedDoTag()) {
            Utils.logi(TAG, "isNeedRestart ? false. No need do tag, so no need restart."
                    + " mLogType = " + mLogType);
            return false;
        }

        if (!LogControllerUtils.getLogControllerInstance(mLogType).isLogRunning()) {
            Utils.logi(TAG,
                "isNeedRestart ? false. Log is stopped before do tag."
                + " mLogType = " + mLogType);
            return false;
        }

        String needTagPath = getNeedTagPath();
        if (needTagPath.isEmpty()) {
            Utils.logi(TAG, "isNeedRestart ? false. getNeedTagPath  is empty.");
            return false;
        }
        if (!needTagPath.contains(getSavingPath())) {
            String[] savingPaths = getSavingPath().split(";");
            boolean isRealContain = false;
            for (String path : savingPaths) {
                if (needTagPath.contains(path)) {
                    isRealContain = true;
                    break;
                }
            }
            if (!isRealContain) {
                Utils.logi(TAG, "isNeedRestart ? false. "
                        + "getNeedTagPath not contains subSavingPath!");
                return false;
            }
        }
        return true;
    }
    /**
     * @return boolean
     */
    public boolean canDoTag() {
        String needTagPath = getNeedTagPath();
        return needTagPath.isEmpty() || getSavingPath().isEmpty()
                || !needTagPath.contains(getSavingPath());
    }

    protected String mNeedTagPath = null;

    /**
     * @return log need be tagged folder path like : /debuglogger/mobilelog/APLog_***
     */
    public String getNeedTagPath() {
        if (mNeedTagPath != null) {
            Utils.logd(TAG, "getNeedTagPath() mNeedTagPath is not null, no need reinit it!"
                    + " mNeedTagPath = " + mNeedTagPath);
            return mNeedTagPath;
        }
        String needTagPath = "";
        File fileTree = new File(
                getSavingParentPath() + File.separator + Utils.LOG_TREE_FILE);
        String logFolderPath = Utils.getLogFolderFromFileTree(fileTree,
                         mTagLogInformation.getExpTime());
        if (logFolderPath == null) {
            List<String> logList = Utils.getLogFolderFromFileTree(fileTree);
            if (logList.size() >= 1) {
                logFolderPath = logList.get(logList.size() - 1);
            }
            if (logList.size() >= 2) {
                logFolderPath += ";" + logList.get(logList.size() - 2);
            }
        }
        if (logFolderPath != null) {
            needTagPath = logFolderPath;
        }
        Utils.logi(TAG, "getNeedTagPath() needTagPath = " + needTagPath
                    + ", for logtype = " + mLogType);
        mNeedTagPath = needTagPath;
        return needTagPath;
    }

    /**
     * @return log saving folder path like : /debuglogger/mobilelog/APLog_***
     */
    public String getSavingPath() {
        String savingPath = "";

        if (!LogControllerUtils.getLogControllerInstance(mLogType).isLogRunning()) {
            Utils.logw(TAG, "Log mLogType = " + mLogType + " is stopped,"
            + " just return null string for saving path!");
            return savingPath;
        }

        File fileTree = new File(getSavingParentPath() + File.separator + Utils.LOG_TREE_FILE);
        String logFolderPath = Utils.getLogFolderFromFileTree(fileTree, "");
        if (logFolderPath != null) {
            savingPath = logFolderPath;
        }
        Utils.logi(TAG, "getSavingPath() savingPath = " + savingPath);
        return savingPath;
    }

    /**
     * @return type log parent path like : /debuglogger/mobilelog
     */
    public String getSavingParentPath() {
        String savingParentPath = Utils.geMtkLogPath() + Utils.LOG_PATH_MAP.get(mLogType);
        Utils.logd(TAG, "savingParentPath = " + savingParentPath);
        return savingParentPath;
    }

    public int getLogType() {
        return mLogType;
    }
}
