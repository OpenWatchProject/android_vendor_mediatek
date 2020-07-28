package com.debug.loggerui.taglog;

import android.os.Handler;

import com.debug.loggerui.MyApplication;
import com.debug.loggerui.controller.LogControllerUtils;
import com.debug.loggerui.framework.DebugLoggerUIServiceManager;
import com.debug.loggerui.framework.DebugLoggerUIServiceManager.ServiceNullException;
import com.debug.loggerui.taglog.TagLogUtils.LogInfoTreatmentEnum;
import com.debug.loggerui.taglog.db.DBManager;
import com.debug.loggerui.taglog.db.FileInfoTable;
import com.debug.loggerui.taglog.db.MySQLiteHelper;
import com.debug.loggerui.utils.Utils;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

/**
 * @author MTK81255
 *
 */
public class LogTManager {
    private static final String TAG = TagLogUtils.TAGLOG_TAG + "/LogTManager";

    private List<LogInstanceForTaglog> mLogForTaglogList = new ArrayList<LogInstanceForTaglog>();
    private TagLogInformation mTagLogInformation;
    private Handler mTaglogManagerHandler;
    private int mLogTypeRestart = 0;

    /**
     * @param taglog TagLog
     */
    public LogTManager(TagLog taglog) {
        mTagLogInformation = taglog.getTaglogInformation();
        mTaglogManagerHandler = taglog.getTaglogManagerHandler();
        init();
    }
    private void init() {
        Utils.logd(TAG, "-->init start.");
        for (int logType : Utils.LOG_TYPE_SET) {
            LogInstanceForTaglog logForTaglog = getLogInstance(logType);
            if (logForTaglog != null) {
                mLogForTaglogList.add(logForTaglog);
            }
        }
        Utils.logi(TAG, "mLogForTaglogList.size() = " + mLogForTaglogList.size());
    }

    private LogInstanceForTaglog getLogInstance(int logType) {
        LogInstanceForTaglog logInstance = null;
        switch (logType) {
        case Utils.LOG_TYPE_MOBILE:
            logInstance = new MobileLogT(logType, mTagLogInformation);
            break;
        case Utils.LOG_TYPE_MODEM:
            logInstance = new ModemLogT(logType, mTagLogInformation);
            break;
        case Utils.LOG_TYPE_NETWORK:
            logInstance = new NetworkLogT(logType, mTagLogInformation);
            break;
        case Utils.LOG_TYPE_GPSHOST:
            logInstance = new GPSHostLogT(logType, mTagLogInformation);
            break;
        case Utils.LOG_TYPE_CONNSYSFW:
            logInstance = new ConnsysFWLogT(logType, mTagLogInformation);
            break;
        case Utils.LOG_TYPE_BTHOST:
            logInstance = new BTHostLogT(logType, mTagLogInformation);
            break;
        default :
            Utils.loge(TAG, "Unspported logType = " + logType + " for Taglog.");
            break;
        }
        return logInstance;
    }

    public List<LogInformation> getSavingLogInformation() {
        return getLogInformation(false);
    }

    public List<LogInformation> getSavingLogParentInformation() {
        return getLogInformation(true);
    }

    private List<LogInformation> getLogInformation(boolean isNeedParentPath) {
        Utils.logd(TAG, "-->getLogPath() isNeedParentPath = " + isNeedParentPath);
        List<LogInformation> logInformationList = new ArrayList<LogInformation>();
        for (LogInstanceForTaglog logInstanceForTaglog : mLogForTaglogList) {
            if (!logInstanceForTaglog.isNeedDoTag()) {
                continue;
            }
            int logType = logInstanceForTaglog.getLogType();
            String logPath = isNeedParentPath ? logInstanceForTaglog
                    .getSavingParentPath() : logInstanceForTaglog.getNeedTagPath();
            LogInfoTreatmentEnum logInfoTreatmentEnum = mTagLogInformation.isNeedZip()
                                                    ? LogInfoTreatmentEnum.ZIP_DELETE
                                                    : LogInfoTreatmentEnum.DO_NOTHING;

            String[] logPaths = logPath.split(";");
            for (String curLogPath : logPaths) {
                if (curLogPath != null &&
                    curLogPath.contains(TagLogUtils.MD_BOOTUP_LOG_NAME)) {
                    File curLogFile = new File(curLogPath);
                    LogInformation logInformation = new LogInformation(logType, curLogFile,
                                         logInfoTreatmentEnum);
                    Utils.logi(TAG, "-->getLogPath(), logPath = " + curLogPath);
                    if (curLogPath.contains(ModemLogT.MODEM_LOG_NO_NEED_ZIP)) {
                        logInformation.setTreatMent(LogInfoTreatmentEnum.DO_NOTHING);
                    }
                    logInformationList.add(logInformation);
                    logInformation.setTagFlag(true);
                    continue;
                }
                LogInformation logInformation = getLogInformationFromDB(curLogPath);
                if (logInformation != null && !isNeedParentPath) {
                    String targetFileNameTmp = logInformation.getTargetTagFolder() + File.separator
                             + logInformation.getTargetFileName().replace("TMP_", "");
                    Utils.logi(TAG, "-->getLogPath(), logPath = " + targetFileNameTmp);
                    String targetFileNameDone = targetFileNameTmp.replace("TMP_", "");
                    String sourcePath = logInformation.getFileInfo().getSourcePath();

                    /*String newSourcePath = targetFileNameTmp;
                    if (!newSourcePath.contains(targetFileNameDone)) {
                        newSourcePath += ";" + targetFileNameDone;
                    }
                    if (!newSourcePath.contains(sourcePath)) {
                        newSourcePath += ";" + sourcePath;
                    }*/
                    Utils.logi(TAG, "-->getLogPath(), setNewSourcePath = " + targetFileNameDone);
                    logInformation.getFileInfo().setOriginalPath(sourcePath);
                    logInformation.getFileInfo().setState(MySQLiteHelper.FILEINFO_STATE_PREPARE);
                    logInformation.getFileInfo().setSourcePath(targetFileNameDone);
                    logInformation.setTagFlag(false);
                    logInformation.setTreatMent(LogInfoTreatmentEnum.COPY);
                    logInformationList.add(logInformation);
                } else {
                    File curLogFile = new File(curLogPath);
                    if (curLogFile.exists()) {
                        logInformation = new LogInformation(logType, curLogFile,
                                             logInfoTreatmentEnum);
                        Utils.logi(TAG, "-->getLogPath(), logPath = " + curLogPath);
                        if (curLogPath.contains(ModemLogT.MODEM_LOG_NO_NEED_ZIP)) {
                            logInformation.setTreatMent(LogInfoTreatmentEnum.DO_NOTHING);
                        }
                        logInformationList.add(logInformation);
                        logInformation.setTagFlag(true);
                    }
                }
            }
        }
        return logInformationList;
    }

    /**
     * @param neededTaglogFileList
     *            List<LogInformation>
     */
    public void restartLogs(List<LogInformation> neededTaglogFileList) {
        Utils.logi(TAG, "-->restartLogs");
        int logTypeRestart = 0;
        for (LogInstanceForTaglog logInstanceForTaglog : mLogForTaglogList) {
            if (logInstanceForTaglog.isNeedRestart()) {
                logTypeRestart |= logInstanceForTaglog.getLogType();
            }
        }
        Utils.logi(TAG, "logTypeRestart = " + logTypeRestart);
        if (logTypeRestart == 0) {
            Utils.logi(TAG, "no need restartLogs");
            return;
        }
        lockUI();
        try {
            DebugLoggerUIServiceManager.getInstance().getService().restartRecording(logTypeRestart,
                        Utils.LOG_START_STOP_REASON_FROM_TAGLOG);
        } catch (ServiceNullException e1) {
            releaseUI();
            resetLogInformationNotTag(neededTaglogFileList, logTypeRestart);
            return;
        }

        int timeout = 0;
        while (true) {
           for (LogInstanceForTaglog logInstanceForTaglog : mLogForTaglogList) {
                if ((logInstanceForTaglog.getLogType() & logTypeRestart) != 0 &&
                    logInstanceForTaglog.canDoTag()) {
                    logTypeRestart ^= logInstanceForTaglog.getLogType();
                }
           }
           if (logTypeRestart == 0) {
               Utils.logi(TAG, "restartLogs success.");
               break;
           }
           try {
               Thread.sleep(TagLogUtils.LOG_STATUS_CHECK_TIME_PERIOD);
           } catch (InterruptedException e) {
               e.printStackTrace();
           }
           timeout += TagLogUtils.LOG_STATUS_CHECK_TIME_PERIOD;
           if (timeout >= TagLogUtils.LOG_STATUS_CHECK_TIME_OUT) {
              Utils.logw(TAG, "check can do tag timeout!");
              resetLogInformationNotTag(neededTaglogFileList, logTypeRestart);
              break;
           }
        }
        releaseUI();
        Utils.logi(TAG, "<--restartLogs");
    }
    private void resetLogInformationNotTag(List<LogInformation> neededTaglogFileList, int logType) {
        Utils.logw(TAG, "restart fail, reset not do tag for logtype = " + logType);
        for (LogInformation logInformation : neededTaglogFileList) {
            // No need do check for Log Type of only TagLog like AEE,SOP...
            if (Utils.LOG_TYPE_SET.contains(logInformation.getLogType())
                    && (logInformation.getLogType() & logType) != 0) {
                logInformation.setTagFlag(false);
                logInformation.setTreatMent(LogInfoTreatmentEnum.DO_NOTHING);
            }
        }
    }

    /**
     * void.
     */
    public void stopLogs() {
        mLogTypeRestart = 0;
        for (LogInstanceForTaglog logInstanceForTaglog : mLogForTaglogList) {
            if (logInstanceForTaglog.isNeedRestart()) {
                mLogTypeRestart |= logInstanceForTaglog.getLogType();
            }
        }
        if (mLogTypeRestart == 0) {
            Utils.logi(TAG, "no need stop logs for mLogTypeRestart = " + mLogTypeRestart);
            return;
        }
        try {
            DebugLoggerUIServiceManager.getInstance().getService().stopRecording(
                    mLogTypeRestart, Utils.LOG_START_STOP_REASON_FROM_TAGLOG);
        } catch (ServiceNullException e1) {
            return;
        }

        int timeout = 0;
        boolean isStopDone = false;
        while (true) {
            isStopDone = true;
            for (int logType : Utils.LOG_TYPE_SET) {
                if ((logType & mLogTypeRestart) == 0) {
                    continue;
                }
                if (logType == Utils.LOG_TYPE_MODEM) {
                    if (LogControllerUtils.getLogControllerInstance(logType).isLogRunning()) {
                        isStopDone = false;
                        break;
                    } else if (Utils.isDenaliMd3Solution()
                            && !MyApplication.getInstance().getSharedPreferences()
                                    .getString(Utils.KEY_C2K_MODEM_LOGGING_PATH, "").isEmpty()) {
                        isStopDone = false;
                        break;
                    }
                }
                if (LogControllerUtils.getLogControllerInstance(logType).isLogRunning()) {
                    isStopDone = false;
                    break;
                }
            }
            if (isStopDone) {
                Utils.logi(TAG, "Type log[" + mLogTypeRestart + "] stop done!");
                break;
            }
            try {
                Thread.sleep(TagLogUtils.LOG_STATUS_CHECK_TIME_PERIOD);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            timeout += TagLogUtils.LOG_STATUS_CHECK_TIME_PERIOD;
            if (timeout >= TagLogUtils.LOG_STATUS_CHECK_TIME_OUT) {
                Utils.logw(TAG, "Type log[" + mLogTypeRestart + "] stop timeout!");
                break;
            }
        }
    }

    /**
     * void.
     */
    public void startLogs() {
        if (mLogTypeRestart == 0) {
            Utils.logi(TAG, "no need start logs for mLogTypeRestart = " + mLogTypeRestart);
            return;
        }
        try {
            DebugLoggerUIServiceManager.getInstance().getService().startRecording(
                    mLogTypeRestart, Utils.LOG_START_STOP_REASON_FROM_TAGLOG);
        } catch (ServiceNullException e1) {
            return;
        }
        int timeout = 0;
        boolean isStartDone = false;
        while (true) {
            isStartDone = true;
            for (int logType : Utils.LOG_TYPE_SET) {
                if ((logType & mLogTypeRestart) == 0) {
                    continue;
                }

                if (!LogControllerUtils.getLogControllerInstance(logType).isLogRunning()) {
                    isStartDone = false;
                    break;
                }
            }
            if (isStartDone) {
                Utils.logi(TAG, "Type log[" + mLogTypeRestart + "] start done!");
                break;
            }
            try {
                Thread.sleep(TagLogUtils.LOG_STATUS_CHECK_TIME_PERIOD);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            timeout += TagLogUtils.LOG_STATUS_CHECK_TIME_PERIOD;
            if (timeout >= TagLogUtils.LOG_STATUS_CHECK_TIME_OUT) {
                Utils.logw(TAG, "Type log[" + mLogTypeRestart + "] start timeout!");
                restartLog(mLogTypeRestart);
                break;
            }
        }
    }

    private void restartLog(int restartlogType) {
        Utils.logd(TAG, "restartLogType : " + restartlogType);
        int logTypeNeedRestart = 0;
        for (int logType : Utils.LOG_TYPE_SET) {
        if ((logType & restartlogType) == 0) {
           continue;
        }

        if (LogControllerUtils.getLogControllerInstance(logType).isLogRunning()) {
            continue;
        }

        logTypeNeedRestart |= logType;
        }
        if (logTypeNeedRestart != 0) {
            try {
                DebugLoggerUIServiceManager.getInstance().getService().startRecording(
                        logTypeNeedRestart, Utils.LOG_START_STOP_REASON_FROM_TAGLOG);
            } catch (ServiceNullException e) {
                return;
            }
        }
    }

    private LogInformation getLogInformationFromDB(String logPath) {
        LogInformation logInformation = null;
        FileInfoTable fileInfo = DBManager.getInstance().getFileInfoByOriginalPath(logPath);
        if (fileInfo != null) {
            logInformation = new LogInformation(fileInfo);
            Utils.logi(TAG, "-->getLogInformationFromDB, logPath = " + logPath);
        }
        return logInformation;
    }

    private void lockUI() {
        Utils.logi(TAG, "-->lockUI");
        mTaglogManagerHandler.obtainMessage(TagLogUtils.MSG_UI_LOCK, this).sendToTarget();
    }

    private void releaseUI() {
        Utils.logi(TAG, "-->releaseUI");
        mTaglogManagerHandler.obtainMessage(TagLogUtils.MSG_UI_RELEASE, this).sendToTarget();
    }

}
