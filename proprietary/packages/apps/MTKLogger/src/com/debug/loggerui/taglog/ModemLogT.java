package com.debug.loggerui.taglog;

import com.debug.loggerui.MyApplication;
import com.debug.loggerui.controller.LogControllerUtils;
import com.debug.loggerui.controller.ModemLogController;
import com.debug.loggerui.file.LogFileManager;
import com.debug.loggerui.settings.SettingsActivity;
import com.debug.loggerui.taglog.db.DBManager;
import com.debug.loggerui.utils.ExceptionInfo;
import com.debug.loggerui.utils.Utils;
import com.log.handler.LogHandlerUtils.ModemLogStatus;

import java.io.File;
import java.io.FileFilter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Locale;
import java.util.Set;

/**
 * @author MTK81255
 *
 */
public class ModemLogT extends LogInstanceForTaglog {

    private static final int WAIT_MODEM_INTENT = 1000;
    private boolean mIsModemEE = false;
    private boolean mIsModemFlush = false;
    private boolean mIsModemEEDumpTimeOut = false;
    private String mCurrentMode = Utils.MODEM_MODE_SD;
    private boolean mIsSpecialModemExp = false;

    private static int sMaxZipNormalLogNum = 3;
    public static final String MODEM_LOG_NO_NEED_ZIP = "_NO_NEED_ZIP";
    private LinkedList<String> mFindEEPathLogBuffer = new LinkedList<String>();
    /**
     * @param logType int
     * @param tagLogInformation TagLogInformation
     */
    public ModemLogT(int logType, TagLogInformation tagLogInformation) {
        super(logType, tagLogInformation);
        init();
    }

    private void init() {
        ExceptionInfo expInfo = new ExceptionInfo();
        try {
            expInfo.initFieldsFromZZ(mTagLogInformation.getExpPath() + File.separator
                    + Utils.EXTRA_VALUE_EXP_ZZ);
            mIsModemEE = expInfo.getSubExpType()
                        .toLowerCase(Locale.getDefault()).contains("modem");
            mIsSpecialModemExp = isSpecialMdExp(expInfo);
        } catch (IOException e) {
            Utils.loge(TAG, "fail to init exception info:" + e.getMessage());
        }
        if (mIsModemEE) {
            sMaxZipNormalLogNum = 4;
        } else {
            sMaxZipNormalLogNum = 3;
        }
        mCurrentMode =
                MyApplication.getInstance().getDefaultSharedPreferences()
                        .getString(Utils.KEY_MD_MODE_1, Utils.MODEM_MODE_SD);
        mIsModemFlush = Utils.MODEM_MODE_PLS.equals(mCurrentMode);

        setExceptionInfoForReport();
        Utils.logd(TAG, "mIsModemEE = " + mIsModemEE + ", mIsModemFlush =" + mIsModemFlush);
    }
    private void setEEProcessInfoForReport() {
        TagEEResultReport.getInstance().setFindEEPathLog(mFindEEPathLogBuffer);
    }

    private void setExceptionInfoForReport() {
        //result_0 result_7
        if ("EE".equals(mTagLogInformation.getTaglogFolderSuffix()) && !mIsModemEE) {
            TagEEResultReport.getInstance().setResult(TagEEResultReport.RESULT_0, "");
        } else if (mIsModemEE &&
                ModemLogStatus.PAUSE.getId()
                == ModemLogController.getInstance().getLogStatus().getId()) {
            TagEEResultReport.getInstance().setResult(TagEEResultReport.RESULT_7, "");
        }
        //set exception info
        StringBuilder exceptionInfo = new StringBuilder();
        exceptionInfo.append("EE : " + mTagLogInformation.getDbFileName() + "\n");
        exceptionInfo.append("EE zz_internal time : "
                             + mTagLogInformation.getZzInternalTime() + "\n");
        TagEEResultReport.getInstance().setExceptionInfo(exceptionInfo.toString());
    }
    private void writFindEEPathLogToBuffer(String log) {
        mFindEEPathLogBuffer.addLast(Utils.integratedOutLog(log));
    }
    @Override
    public boolean isNeedDoTag() {
        if (mCurrentMode.equals(Utils.MODEM_MODE_USB)) {
            // For md log is in usb mode
            Utils.logi(TAG, "isNeedZipModemLog ? false. Modem is in USB mode.");
            return false;
        }
        if (MyApplication.getInstance().getDefaultSharedPreferences()
                .getBoolean(SettingsActivity.KEY_ALWAYS_TAG_MODEM_LOG_ENABLE, false)) {
            return true;
        }
        if (mTagLogInformation.isFromException() && !mIsModemEE && !mIsSpecialModemExp) {
            // For normal exception, no need tag modem log
            Utils.logi(TAG, "isNeedZipModemLog ? false." + " mIsModemEE = " + mIsModemEE
                    + ", mIsSpecialModemExp = " + mIsSpecialModemExp);
            return false;
        }
        return super.isNeedDoTag();
    }

    private boolean isSpecialMdExp(ExceptionInfo expInfo) {
        if (!mTagLogInformation.isFromException()) {
            return false;
        }
        boolean isModemException = false;
        String expType = expInfo.getType();
        String expProcess = expInfo.getProcess();
        String expSubType = expInfo.getSubExpType();

        if (expType == null) {
            isModemException = false;
        } else if (expType.endsWith("Native (NE)")
                && (expProcess.contains("volte_") || expProcess.contains("mtkmal") ||
                    expProcess.contains("mtkrild") || expProcess.contains("mtkfusionrild")
                        || expProcess.contains("gsm0710muxd"))) {
            isModemException = true;
        } else if (expType.endsWith("System API Dump")
                && (expProcess.contains("AT command pending too long")
                   || expProcess.contains("Gsm0710Muxd:ASSERT!")
                   || expProcess.contains("Speech"))) {
            isModemException = true;
            sMaxZipNormalLogNum = 5;
        } else if (expType.endsWith("Modem Warning")) {
            isModemException = true;
        } else if (expType.endsWith("Externel (EE)")
                   && expSubType.contains("adsp")
                   && expProcess.contains("CRDISPATCH_KEY:ADSP exception/call")) {
            isModemException = true;
        } else if (expType.endsWith("ANR")
                && (expProcess.contains("com.android.phone")
                        || expProcess.contains("com.android.mms"))) {
            isModemException = true;
        } else if (expType.endsWith("Java (JE)")
                && expProcess.contains("com.android.mms")) {
            isModemException = true;
        }
        Utils.logi(TAG, "isSpecialMdExp ? " + isModemException);
        return isModemException;
    }

    @Override
    public boolean isNeedRestart() {
        if (Utils.MODEM_MODE_USB.equals(mCurrentMode)) {
            // For md log is in usb mode
            Utils.logi(TAG, "isNeedRestartModemLog ? false. Modem is in USB mode." + " mLogType = "
                    + mLogType);
            return false;
        }
        if (Utils.MODEM_MODE_PLS.equals(mCurrentMode)) {
            // For md log is in PLS mode
            Utils.logi(TAG, "isNeedRestartModemLog ? false. Modem is in PLS mode." + " mLogType = "
                    + mLogType);
            return false;
        }
        if (mIsModemEE && (mIsModemEEDumpTimeOut
                 || getNeedTagPath().toUpperCase().contains("_EE_"))) {
            Utils.logi(TAG, "isNeedRestartModemLog ? false. Modem is dump done no need restart."
                            + " mLogType = " + mLogType);
            return false;
        }
        return super.isNeedRestart();
    }

    private String getModemLogPath(String beforTime) {
        waitMdLogReady();
        String allModemLogPath = "";
        String[] modemLogParentPaths = getSavingParentPath().split(";");
        for (String modemParentLogPath : modemLogParentPaths) {
            String modemLogPath = "";
            File fileTree = new File(modemParentLogPath + File.separator + Utils.LOG_TREE_FILE);
            String logFolderPath = Utils.getLogFolderFromFileTree(fileTree, beforTime);
            //no need check file exist here, because LogTManager.getLogInformation
            //will first from db
            if (logFolderPath != null) {
                modemLogPath = logFolderPath;
            }
            if (!modemLogPath.isEmpty()) {
                allModemLogPath += modemLogPath + ";";
            }
        }
        if (Utils.isSupportC2KModem() && Utils.isDenaliMd3Solution()) {
            allModemLogPath += getC2kLogPath(beforTime);
        }
        if (allModemLogPath.endsWith(";")) {
            // Remove ";" from modemLogSavingPath
            allModemLogPath = allModemLogPath.substring(0, allModemLogPath.length() - 1);
        }
        Utils.logi(TAG, "getModemLogPath() beforTime = " + beforTime
                + ", allModemLogPath = " + allModemLogPath);
        return allModemLogPath;
    }

    private String getModemEEPath(String beforTime) {
        String allModemLogPath = "";
        String modem1EEPath  = "";
        if (isModemLogReady()) {
            allModemLogPath = getEEPathFromFileTree(beforTime);
            modem1EEPath = allModemLogPath.split(";")[0];
            if (!checkFindEEFolderResult(modem1EEPath)) {
                //boot up ee, .bin exist at normal folder
                if (isExistBootBinFile(modem1EEPath)) {
                    allModemLogPath = updatPathForBootEE(allModemLogPath);
                    outPutResult(TagEEResultReport.RESULT_1, allModemLogPath);
                    return allModemLogPath;
                }
                if (isDumpStart()) {
                    outPutResult(TagEEResultReport.RESULT_2, allModemLogPath);
                    return allModemLogPath;
                }
                if (isWaitDumpStartTimeOut()) {
                    outPutResult(TagEEResultReport.RESULT_3, allModemLogPath);
                    return allModemLogPath;
                }
                if (isWaitDumpDoneTimeOut()) {
                    outPutResult(TagEEResultReport.RESULT_4, allModemLogPath);
                    return allModemLogPath;
                }
                allModemLogPath = getEEPathFromFileTree(beforTime);
                modem1EEPath = allModemLogPath.split(";")[0];
                if (!checkFindEEFolderResult(modem1EEPath)) {
                    if (isExistBootBinFile(modem1EEPath)) {
                        allModemLogPath = updatPathForBootEE(allModemLogPath);
                        outPutResult(TagEEResultReport.RESULT_1, allModemLogPath);
                        return allModemLogPath;
                    }
                }
            }
            return allModemLogPath;
        }
        boolean dumpTimeOut = isWaitDumpDoneTimeOut();
        allModemLogPath = getEEPathFromFileTree(beforTime);
        if (dumpTimeOut) {
            outPutResult(TagEEResultReport.RESULT_4, allModemLogPath);
            return allModemLogPath;
        }
        modem1EEPath = allModemLogPath.split(";")[0];
        if (!checkFindEEFolderResult(modem1EEPath)) {
            if (isExistBootBinFile(modem1EEPath)) {
                allModemLogPath = updatPathForBootEE(allModemLogPath);
                outPutResult(TagEEResultReport.RESULT_1, allModemLogPath);
                return allModemLogPath;
            }
        }

        String logStr = "getModemLogPath() beforTime = " + beforTime
                + ", allModemLogPath = " + allModemLogPath;
        Utils.logi(TAG, logStr);
        writFindEEPathLogToBuffer(logStr);
        return allModemLogPath;
    }
    private String updatPathForBootEE(String allModemLogPath) {
        String newEEPath = allModemLogPath;
        String[] paths = allModemLogPath.split(";");
        for (String logPath : paths) {
            String bootBinFilePath = "";
            bootBinFilePath = copyDumpFiles(logPath);
            if (!bootBinFilePath.isEmpty()) {
                newEEPath = newEEPath.replace(logPath, bootBinFilePath);
                continue;
            }
        }
        return newEEPath;
    }
    private boolean isExistBootBinFile(String allModemLogPath) {
        String[] paths = allModemLogPath.split(";");
        for (String logPath : paths) {
            File[] fileList = getFileList(logPath, "_data.bin");
            if (fileList != null && fileList.length > 1) {
                Utils.logd(TAG, "find _data.bin file at " + logPath);
                return true;
            }
        }
        return false;
    }
    private boolean isDumpStart() {
        return ModemLogController.getInstance().isReceiveDumpStart(
                mTagLogInformation.getExpTime());
    }

    private void outPutResult(int result, String eeFolder) {
        TagEEResultReport.getInstance().setResult(result, eeFolder);
    }
    private boolean checkFindEEFolderResult(String modem1LogPath) {
        if (modem1LogPath != null && modem1LogPath.contains("_EE")) {
            int result = TagEEResultReport.RESULT_DEFAULT;
            if (isEEFolderExist(modem1LogPath)) {
                result = TagEEResultReport.RESULT_1;
            } else {
                if (null != DBManager.getInstance().getFileInfoByOriginalPath(modem1LogPath)) {
                    result = TagEEResultReport.RESULT_1;
                } else {
                    result = TagEEResultReport.RESULT_6;
                }
            }
            outPutResult(result, modem1LogPath);
            return true;
        }
        return false;
    }

    private boolean isWaitDumpDoneTimeOut() {
        int modemLogStatus = ModemLogController.getInstance().getLogStatus().getId();
        boolean isModemLogReady = (ModemLogStatus.PAUSE.getId() == modemLogStatus
                                   || ModemLogStatus.RUNNING.getId() == modemLogStatus);
        writFindEEPathLogToBuffer("first getModemLogStatus = " + modemLogStatus);
        if (isModemLogReady) {
            return false;
        }
        int i = 0; // For printout log every 10s
        mIsModemEEDumpTimeOut = false;
        while (!isModemLogReady && i < 600) { // Wait at most 10min
            try {
                Thread.sleep(WAIT_MODEM_INTENT);
                i++;
                if (i % 5 == 0) {
                    Utils.logd(TAG, "Modem Log is not Ready , wait for 5s");
                }
                modemLogStatus = ModemLogController.getInstance().getLogStatus().getId();
                isModemLogReady = (ModemLogStatus.PAUSE.getId() == modemLogStatus
                              || ModemLogStatus.RUNNING.getId() == modemLogStatus);
                Utils.logd(TAG, "waitMdLogReady, modemLogStatus = " + modemLogStatus);
                writFindEEPathLogToBuffer(
                        "wait modemlog ready, next getModemLogStatus = " + modemLogStatus);
            } catch (InterruptedException e) {
                Utils.loge(TAG, "Catch InterruptedException");
            }
        }
        if (i >= 600) {
            mIsModemEEDumpTimeOut = true;
            Utils.loge(TAG, " Modem dump cost too much time!");
            writFindEEPathLogToBuffer("modem log dump time out");
            return true;
        }
        return false;
    }
    private boolean isWaitDumpStartTimeOut() {
        boolean receiveDumpStart = ModemLogController.getInstance().isReceiveDumpStart(
                                   mTagLogInformation.getExpTime());
        if (receiveDumpStart) {
            writFindEEPathLogToBuffer("isWaitDumpStartTimeOut = false");
            return false;
        }

        int i = 0; // For printout log every 10s
        while (!receiveDumpStart && i < 180) { // Wait at most 3min
            try {
                Thread.sleep(WAIT_MODEM_INTENT);
                i++;
                if (i % 5 == 0) {
                    Utils.logd(TAG, "Modem Log is not Ready , wait for 5s");
                }
                writFindEEPathLogToBuffer(
                        "wait modemlog dump start, wait time = " + i);
                receiveDumpStart = ModemLogController.getInstance().isReceiveDumpStart(
                        mTagLogInformation.getExpTime());
            } catch (InterruptedException e) {
                Utils.loge(TAG, "Catch InterruptedException");
            }
        }
        if (i >= 180) {
            Utils.loge(TAG, " wait Modem dump start cost too much time!");
            writFindEEPathLogToBuffer("wait modem start dump time out");
            return true;
        }
        return false;
    }
    private boolean isEEFolderExist(String allModemLogPath) {
        String[] eePaths = allModemLogPath.split(";");
        for (String eePath : eePaths) {
            if (new File(eePath).exists()) {
                return true;
            } else {
                writFindEEPathLogToBuffer("check file not exist: " + eePath);
            }
        }
        return false;
    }
    private String getEEPathFromFileTree(String beforTime) {
        String allModemLogPath = "";
        String[] modemLogParentPaths = getSavingParentPath().split(";");
        for (String modemParentLogPath : modemLogParentPaths) {
            String modemLogPath = "";
            File fileTree = new File(modemParentLogPath + File.separator + Utils.LOG_TREE_FILE);
            String logFolderPath = Utils.getLogFolderFromFileTree(fileTree, beforTime);
            //no need check file exist here, because LogTManager.getLogInformation
            //will first from db
            writFindEEPathLogToBuffer("find EE log path : " + logFolderPath);
            if (logFolderPath != null) {
                modemLogPath = logFolderPath;
            }
            if (!modemLogPath.isEmpty()) {
                allModemLogPath += modemLogPath + ";";
            }
        }
        return allModemLogPath;
    }
    private String getBootUpLogPath() {
         String allMdBootUpLogPath = "";
         String[] modemLogParentPaths = getSavingParentPath().split(";");
         for (String modemParentLogPath : modemLogParentPaths) {
             String logFolderPath = modemParentLogPath
                                    + File.separator + TagLogUtils.MD_BOOTUP_LOG_NAME;
             File bootupFile = new File(logFolderPath);
             if (bootupFile.exists()) {
                 allMdBootUpLogPath += logFolderPath + ";";
             }
         }
         if (allMdBootUpLogPath.endsWith(";")) {
             allMdBootUpLogPath = allMdBootUpLogPath.substring(0, allMdBootUpLogPath.length() - 1);
         }
         Utils.logi(TAG, "getBootUpLogPath(), allMdBootUpLogPath = " + allMdBootUpLogPath);
         return allMdBootUpLogPath;
    }
    @Override
    public String getNeedTagPath() {
        if (mNeedTagPath != null) {
            Utils.logd(TAG, "getNeedTagPath() mNeedTagPath is not null, no need reinit it!"
                    + " mNeedTagPath = " + mNeedTagPath);
            return mNeedTagPath;
        }
        String modemNeedTagPath = "";
        if (mIsModemEE) {
            modemNeedTagPath = getEEPath();
        } else if (mIsModemFlush) {
            modemNeedTagPath = getFlushPath();
        } else {
            modemNeedTagPath = getModemLogPath(mTagLogInformation.getExpTime());
        }
        String allmodemLogPath = doFilter(modemNeedTagPath);
        String bootUpPath = getBootUpLogPath();
        if (!bootUpPath.isEmpty()) {
            modemNeedTagPath += ";" + bootUpPath;
            allmodemLogPath += ";" + bootUpPath;
        }
        if (mIsModemEE) {
            TagEEResultReport.getInstance().setAllModemLogPath(allmodemLogPath);
        }
        //test
        boolean resutl = isExistBootBinFile("sdcard/mtklog/taglog/test");
        Utils.logi(TAG, "getNeedTagPath() isExistBootBinFile? " + resutl);
        Utils.logi(TAG, "getNeedTagPath() modemNeedTagPath = " + modemNeedTagPath);
        mNeedTagPath = modemNeedTagPath;
        return mNeedTagPath;
    }
/**
 * @param path String
 * @return String
 */
    public String copyDumpFiles(String path) {
        Utils.logi(TAG, "copyDumpFiles--> path =  " + path);
        String newPath = "";
        File[] fileList = getFileList(path, "_data");
        if (fileList != null && fileList.length >= 1) {
            newPath = mTagLogInformation.getTaglogTargetFolder() + File.separator
                                  + "data";
            File newFolder = new File(newPath);
            if (!newFolder.exists()) {
                LogFileManager.mkdir(newFolder);
            }
            for (File dataFile : fileList) {
                Utils.doCopy(dataFile.getAbsolutePath(), newPath + File.separator
                        + dataFile.getName());
            }
        }
        Utils.logi(TAG, "<--copyDumpFiles " + newPath);
        return newPath;
    }

    private File[] getFileList(String folderPath, final String filter) {
        File logPath = new File(folderPath);
        File[] fileList = null;
        if (logPath != null && logPath.exists()) {
            fileList = logPath.listFiles(new FileFilter() {
                @Override
                public boolean accept(File pathname) {
                    if (pathname.getName().contains(filter)) {
                        Utils.logi(TAG, "20190217 pathname = " + pathname.getName());
                        return true;
                    }
                    return false;
                }
            });
        }
        return fileList;
    }
    @Override
    public String getSavingPath() {
        String modemSavingPath = "";

        if (!LogControllerUtils.getLogControllerInstance(mLogType).isLogRunning()) {
            Utils.logw(TAG, "Log mLogType = " + mLogType + " is stopped,"
            + " just return null string for saving path!");
            return modemSavingPath;
        }

        if (mIsModemFlush) {
            Utils.logi(TAG, "getModemLogPath() = null,for modelog in PLS mode.");
            return modemSavingPath;
        }
        modemSavingPath = getModemLogPath("");
        Utils.logi(TAG, "getSavingPath() modemSavingPath = " + modemSavingPath);
        return modemSavingPath;
    }

    @Override
    public String getSavingParentPath() {
        Set<String> mdLogPathSets = new HashSet<String>();
        String mtkLogPath = Utils.geMtkLogPath();
        for (int modemIndex : Utils.MODEM_INDEX_SET) {
            String modemLogFolder = Utils.MODEM_INDEX_FOLDER_MAP.get(modemIndex);
            String logFilePath = mtkLogPath + modemLogFolder;
            Utils.logv(TAG, "Modem [" + modemIndex + "] root folder=" + logFilePath);
            File logFile = new File(logFilePath);
            if (null != logFile && logFile.exists()) {
                mdLogPathSets.add(logFilePath);
            }
        }

        String mdParentPath = "";
        for (String mdLogPath : mdLogPathSets) {
            mdParentPath += mdLogPath + ";";
        }
        if (mdParentPath.endsWith(";")) {
            // Remove ";" from mdParentPath
            mdParentPath = mdParentPath.substring(0, mdParentPath.length() - 1);
        }
        Utils.logi(TAG, "getSavingParentPath() mdParentPath = " + mdParentPath);
        return mdParentPath;
    }

    private void waitMdLogReady() {
        boolean isModemLogReady = isModemLogReady();
        int i = 0; // For printout log every 10s
        while (!isModemLogReady && i < 600) { // Wait at most 10min
            try {
                Thread.sleep(WAIT_MODEM_INTENT);
                i++;
                if (i % 5 == 0) {
                    Utils.logd(TAG, "Modem Log is not Ready , wait for 5s");
                }
                isModemLogReady = isModemLogReady();
                Utils.logd(TAG, "waitMdLogReady, isModemLogReady = " + isModemLogReady);
            } catch (InterruptedException e) {
                Utils.loge(TAG, "Catch InterruptedException");
            }
        }
        if (i >= 600) {
            Utils.loge(TAG, "waitMdLogReady time out!");
        }
    }
    private String getEEPath() {
        String eePath = "";
        mFindEEPathLogBuffer.clear();
        writFindEEPathLogToBuffer("start find EE path");
        eePath = getModemEEPath(mTagLogInformation.getExpTime());
        if (Utils.isSupportC2KModem() && Utils.isDenaliMd3Solution()) {
            eePath += getC2kLogPath(mTagLogInformation.getExpTime());
        }
        if (eePath.endsWith(";")) {
            eePath = eePath.substring(0, eePath.length() - 1);
        }
        writFindEEPathLogToBuffer("check find EE path, reset it to : " + eePath);
        setEEProcessInfoForReport();
        return eePath;
    }

    private boolean isModemLogReady() {
        int modemLogStatus = ModemLogController.getInstance().getLogStatus().getId();
        boolean isModemLogReady = (ModemLogStatus.PAUSE.getId() == modemLogStatus
                                   || ModemLogStatus.RUNNING.getId() == modemLogStatus);
        writFindEEPathLogToBuffer("isModemLogReady ? " + isModemLogReady
                                   + ", modemLogStatus = " + modemLogStatus);
        return isModemLogReady;
    }

    private String getFlushPath() {
        boolean isFlushTimeout = false;
        String flushModemLogPath = ModemLogController.getInstance().triggerPLSModeFlush();
        Utils.logi(TAG, "MODEM_LOG_FLUSH_PATH : " + flushModemLogPath + ", mIsTimeoutFlushLog="
                + isFlushTimeout);
        return flushModemLogPath;
    }

    /**
     * @param savingLogPath String
     * @return String
     */
    public String doFilter(String savingLogPath) {
        Utils.logi(TAG, "doFilter for " + savingLogPath);
        String newLogPath = savingLogPath;
        String[] modemLogPaths = savingLogPath.split(";");
        for (String modemLogPath : modemLogPaths) {
            List<File> needRenameList = getNeedRenameLogList(modemLogPath);
            if (needRenameList == null || needRenameList.size() < 1) {
                continue;
            }

            newLogPath += ";" + doRename(needRenameList);
        }
        return newLogPath;
    }

    /**
     * @param mdlogPath
     *            String
     * @return List<File>
     */
    private List<File> getNeedRenameLogList(String mdlogPath) {
        File modemLogFile = new File(mdlogPath);
        if (!modemLogFile.exists()) {
            Utils.logi(TAG, "mdlog not exist :" + mdlogPath);
            return null;
        }
        List<String> logInFileTreeList = (Utils.getLogFolderFromFileTree(new File(
                modemLogFile.getAbsolutePath() + File.separator + Utils.LOG_TREE_FILE)));
        if (logInFileTreeList == null || logInFileTreeList.size() < 1) {
            Utils.logi(TAG, "getLogFolderFromFileTree = null or size < 1" );
            return null;
        }
        String[] modemLogStrArray = logInFileTreeList.get(0).split("\\.");
        Utils.logi(TAG, "modemLogStrArray.length = " + modemLogStrArray.length);
        if (modemLogStrArray.length < 3) {
            return null;
        }
        List<String> logFileExistInTree = new ArrayList<String>();
        String treeSuffix = modemLogStrArray[modemLogStrArray.length - 1];
        for (String filePath : logInFileTreeList) {
            filePath = filePath.replace("." + treeSuffix, "");
            String[] logPathStrArray = filePath.split("\\/");
            String newPath = mdlogPath + File.separator +
                          logPathStrArray[logPathStrArray.length - 1];
            if (new File(newPath).exists()) {
                logFileExistInTree.add(newPath);
            }
        }
        Utils.logi(TAG, "new file in tree = " + logFileExistInTree.toString());

        if (logFileExistInTree.size() < sMaxZipNormalLogNum) {
            Utils.logi(TAG, "return. file in tree < " + sMaxZipNormalLogNum);
            return null;
        }
        List<File> needRenameLog = new ArrayList<File>();
        for (int i = 0; i <= logFileExistInTree.size() - sMaxZipNormalLogNum; i++) {
            Utils.logd(TAG, "need rename file: " + logFileExistInTree.get(i));
            needRenameLog.add(new File(logFileExistInTree.get(i)));
        }
        return needRenameLog;
    }
    private String doRename(List<File> fileList) {
        if (fileList == null || fileList.size() < 1) {
            return "";
        }
        String mdlogFolder = new File(fileList.get(0).getParent()).getName();
        String newFolderName = mTagLogInformation.getTaglogTargetFolder() + File.separator
                + mdlogFolder + MODEM_LOG_NO_NEED_ZIP;
        Utils.logi(TAG, "mdlog no need zip file rename to " + newFolderName);
        File newFolder = new File(newFolderName);
        if (!newFolder.exists()) {
            LogFileManager.mkdir(newFolder);
        }
        for (File file : fileList) {
            LogFileManager.renameTo(file, new File(newFolder + File.separator + file.getName()));
        }
        return newFolderName;
    }

    private String getC2kLogPath(String beforTime) {
        String c2kLogPath = "";
        String mtkLogPath = Utils.geMtkLogPath();
        String c2kLogParentPath = mtkLogPath + Utils.C2K_MODEM_LOG_PATH;
        File logFile = new File(c2kLogParentPath);
        if (null != logFile && logFile.exists()) {
            File fileTree = new File(c2kLogParentPath + File.separator
                    + Utils.LOG_TREE_FILE);
            String logFolderPath = Utils.getLogFolderFromFileTree(fileTree, beforTime);
            if (logFolderPath != null && !logFolderPath.isEmpty()) {
                c2kLogPath += logFolderPath + ";";
            }
        }
        if (mIsModemEE) {
            String md3DumpPath = "";
            String md3ParentLogPath = mtkLogPath + Utils.MODEM_INDEX_FOLDER_MAP.get(
                                      Utils.MODEM_LOG_K2_INDEX + Utils.C2KLOGGER_INDEX);
            Utils.logv(TAG, "Modem [" + 3 + "] root folder=" + md3ParentLogPath);
            File parentFile = new File(md3ParentLogPath);
            File[] listFiles = parentFile.listFiles();
            if (listFiles != null && listFiles.length > 0) {
                md3DumpPath = listFiles[listFiles.length - 1].getAbsolutePath();
                Utils.logi(TAG, "c2k dump path :" + md3DumpPath);
            }
            if (!md3DumpPath.isEmpty()) {
                c2kLogPath += md3DumpPath;
            }
        }
        if (c2kLogPath.endsWith(";")) {
            c2kLogPath = c2kLogPath.substring(0, c2kLogPath.length() - 1);
        }
        Utils.logi(TAG, "c2k Log Path :" + c2kLogPath);
        return c2kLogPath;
    }
}
