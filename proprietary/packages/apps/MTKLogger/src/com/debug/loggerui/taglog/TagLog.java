package com.debug.loggerui.taglog;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Build;
import android.os.Handler;
import android.os.SystemProperties;
import android.provider.Settings;

import com.debug.loggerui.MyApplication;
import com.debug.loggerui.file.LogFileManager;
import com.debug.loggerui.taglog.TagLogUtils.LogInfoTreatmentEnum;
import com.debug.loggerui.taglog.db.DBManager;
import com.debug.loggerui.taglog.db.FileInfoTable;
import com.debug.loggerui.taglog.db.MySQLiteHelper;
import com.debug.loggerui.utils.Utils;

import java.io.File;
import java.io.FileFilter;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.regex.Pattern;

/**
 * @author MTK81255
 *
 */
public abstract class TagLog {
    protected static final String TAG = TagLogUtils.TAGLOG_TAG + "/TagLog";

    protected Context mContext;
    protected SharedPreferences mSharedPreferences;
    protected SharedPreferences mDefaultSharedPreferences;
    protected TagLogInformation mTaglogInformation;
    protected Intent mInputIntent = null;

    protected LogTManager mLogManager;
    protected List<LogInformation> mNeededTaglogFileList = new ArrayList<LogInformation>();
    protected String mTaglogFolder = "";
    protected TagLogData mTaglogData = null;

    protected Handler mTaglogManagerHandler;

    protected StatusBarNotify mStatusBarNotify = null;

    protected long mTaglogId = -1;
    protected String mFileListStr = "";

    /**
     * @param taglogManagerHandler
     *            Handler
     */
    public TagLog(Handler taglogManagerHandler) {
        this.mTaglogManagerHandler = taglogManagerHandler;
    }

    /**
     * @return Intent
     */
    public Intent getInputIntent() {
        return mInputIntent;
    }

    /**
     * @param intent
     *            Intent
     */
    public void beginTag(Intent intent) {
        Utils.logi(TAG, "-->beginTag");
        mInputIntent = intent;
        startTaglogThread();
    }
    /**
     * @param data
     *            TagLogData
     */
    public void resumeTag(final TagLogData data) {
        Utils.logi(TAG, "-->resumeTag");
        mTaglogData = data;
        startTaglogThread();
    }

    private void startTaglogThread() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                if (!doInit()) {
                    deInit();
                    Utils.loge(TAG, "doInit() failed!");
                    return;
                }
                createTaglogFolder();
                prepareNeededLogFiles();
                notifyTaglogSize();
                doFilesManager();
                reportTaglogResult(true);
                deInit();
            }
        }).start();
    }

    private boolean doInit() {
        Utils.logi(TAG, "-->doInit() for Intent");
        mContext = MyApplication.getInstance();
        mSharedPreferences = MyApplication.getInstance().getSharedPreferences();
        mDefaultSharedPreferences = MyApplication.getInstance().getDefaultSharedPreferences();

        if (mTaglogData != null) {
            mTaglogInformation = new TagLogInformation(mTaglogData);
        } else {
            mTaglogInformation = new TagLogInformation(mInputIntent);
        }
        if (!mTaglogInformation.isAvailable()) {
            Utils.loge(TAG, "The intent for taglog is error, just return!");
            return false;
        }
        mLogManager = new LogTManager(this);
        if (mTaglogData != null) {
            mTaglogId = mTaglogData.getTaglogTable().getTagLogId();
        } else {
            mTaglogId = DBManager.getInstance().updateTaglogToDb(mTaglogInformation);
        }
        return true;
    }

    protected void checkFileState() {
        // 1. get file state 2.update state bar
        int startTotalFileCount = startNotificationBar();
        if (startTotalFileCount == 0) {
            return;
        }
        String[] fileIds = mFileListStr.split(",");
        Utils.logi(TAG, "CheckFileState, mFileListStr = " + mFileListStr);

        long timeOut = 20 * 60 * 1000;
        while (true) {
            // For update progress bar
            int totalFileCount = 0;
            // For log file treatment status
            boolean isAllDone = true;
            for (String strFileId : fileIds) {
                if (strFileId == null || strFileId.isEmpty()) {
                    continue;
                }
                long fileId = Long.valueOf(strFileId);
                FileInfoTable fileInfo = DBManager.getInstance().getFileInfoById(fileId);
                if (fileInfo == null) {
                    continue;
                }
                // update progress
                totalFileCount += fileInfo.getFileProgress();

                if (!MySQLiteHelper.FILEINFO_STATE_DONE.equals(fileInfo.getState())) {
                    isAllDone = false;
                }
            }
            Utils.logd(TAG, "CheckFileState, totalFileCount = " + totalFileCount
                    + "isAllDone = " + isAllDone);
            mStatusBarNotify.updateState(TagLogUtils.MSG_ZIPPED_FILE_COUNT, totalFileCount);
            if (isAllDone) {
                Utils.logi(TAG, "CheckFileState, break for isAllDone = " + isAllDone);
                break;
            }
            try {
                Thread.sleep(1000);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            timeOut -= 1000;
            if (timeOut <= 0) {
                Utils.logi(TAG, "CheckFileState, break for timeOut = " + timeOut);
                break;
            }
        }
        stopNotificationBar(startTotalFileCount);
    }

    protected void createTaglogFolder() {
        if (mTaglogData != null) {
            String taglogFolder = mTaglogData.getTaglogTable().getTargetFolder();
            File tagLogFolder = new File(taglogFolder);
            if (!tagLogFolder.exists()) {
                LogFileManager.mkdirs(tagLogFolder);
            }
            Utils.logi(TAG, "createTagLogFolder : " + taglogFolder);
            mTaglogFolder = taglogFolder;
            return;
        }
        String logPath = mTaglogInformation.getTaglogTargetFolder();
        File tagLogFolder = new File(logPath);
        if (!tagLogFolder.exists()) {
            LogFileManager.mkdirs(tagLogFolder);
        } else {
            try {
                Thread.sleep(1000);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            String type = mTaglogInformation.getTaglogFolderSuffix();
            logPath = getTaglogParentPath() + TagLogUtils.TAGLOG_TEMP_FOLDER_PREFIX + "TagLog_"
                      + TagLogUtils.getCurrentTimeString()
                      + ((type == null || "".equals(type)) ? "" : ("_" + type));
            tagLogFolder = new File(logPath);
            LogFileManager.mkdirs(tagLogFolder);
            mTaglogInformation.setTaglogTargetFolder(logPath);
            DBManager.getInstance().updateTaglogFolder(mTaglogId, logPath);
        }
        Utils.logi(TAG, "createTagLogFolder : " + logPath);
        mTaglogFolder = tagLogFolder.getAbsolutePath();
    }

    protected void prepareNeededLogFiles() {
        Utils.logi(TAG, "-->prepareNeededLogFiles()");
        if (mTaglogData != null) {
            String fileListStr = mTaglogData.getTaglogTable().getFileList();
            if (!fileListStr.isEmpty()) {
                List<LogInformation> logInfoList =
                        DBManager.getInstance().getFileInfoByIds(fileListStr);
                if (logInfoList != null && logInfoList.size() > 0) {
                    Utils.logi(TAG, "-->getNeededTagLogFiles() from db");
                    mFileListStr = fileListStr;
                    mNeededTaglogFileList = logInfoList;
                    return;
                }
            }
        }
        //prepare db & logs
        synchronized (TagLog.class) {
            Utils.logi(TAG, "-->prepareNeededLogFiles() start");
            boolean isNeedRestartLogs = false;
            switch (mTaglogInformation.getTagLogType()) {
            case TagLogUtils.TAG_ONLY_COPY_DB:
                prepareDB();
                isNeedRestartLogs = false;
                break;
            case TagLogUtils.TAG_MANUAL:
                prepareLogs();
                isNeedRestartLogs = true;
                break;
            case TagLogUtils.TAG_EXCEPTION:
                prepareDB();
                prepareLogs();
                isNeedRestartLogs = true;
                break;
            default:
                break;
            }
            // restart logs
            if (isNeedRestartLogs) {
                if (mTaglogInformation.isNeedAllLogs()) {
                    mLogManager.stopLogs();
                    moveLogToTagFolder();
                    mLogManager.startLogs();
                } else {
                    mLogManager.restartLogs(mNeededTaglogFileList);
                    moveLogToTagFolder();
                }
            }
            DBManager.getInstance().updateTaglogState(mTaglogId,
                    MySQLiteHelper.TAGLOG_STATE_DOTAG);
            mFileListStr = DBManager.getInstance().insertFileInfoToDb(
                    mNeededTaglogFileList);
            DBManager.getInstance().updateTaglogFileList(mTaglogId,
                    mFileListStr);
            Utils.logi(TAG, "<--getNeededTagLogFiles() done");
        }
    }

    private void prepareDB() {
        mNeededTaglogFileList.add(new LogInformation(TagLogUtils.LOG_TYPE_AEE, new File(
                mTaglogInformation.getExpPath()), LogInfoTreatmentEnum.COPY));
    }

    private void prepareLogs() {
        if (mTaglogInformation.isNeedAllLogs()) {
            mNeededTaglogFileList.addAll(mLogManager.getSavingLogParentInformation());
        } else {
            mNeededTaglogFileList.addAll(mLogManager.getSavingLogInformation());
        }
        addAdditionalFiles(mNeededTaglogFileList);
        setTargetForLogFiles(mNeededTaglogFileList);
    }

    protected void addAdditionalFiles(List<LogInformation> logInfoList) {
        logInfoList.add(new LogInformation(TagLogUtils.LOG_TYPE_SOP, createSopFile()));

        // Add connsysDump file
        File connsysDumpFile = new File(Utils.getCurrentLogPath() + File.separator + "debuglogger"
                + File.separator + Utils.CONNSYS_LOG_PATH + File.separator + "connsysdump");
        if ("EE".equals(mTaglogInformation.getTaglogFolderSuffix()) && connsysDumpFile.exists()
                && (connsysDumpFile.list() != null && connsysDumpFile.list().length > 0)) {
            LogInformation connsysDumpLogInfor =
                    new LogInformation(TagLogUtils.LOG_TYPE_CONNSYS_DUMP, connsysDumpFile,
                            LogInfoTreatmentEnum.ZIP_DELETE);
            connsysDumpLogInfor.setTagFlag(true);
            logInfoList.add(connsysDumpLogInfor);
            Utils.logi(TAG,
                    "addAdditionalFiles(), connsysDumpFile = " + connsysDumpFile.getAbsolutePath());
        } else {
            Utils.logi(TAG, "addAdditionalFiles(), Do not add connsysDumpFile for it is null.");
        }
    }

    protected String getTaglogParentPath() {
        return Utils.geMtkLogPath() + "/" + TagLogUtils.TAGLOG_FOLDER_NAME + "/";
    }

    protected void moveLogToTagFolder() {
        for (LogInformation logInformation : mNeededTaglogFileList) {
            File neededLogFolder = new File(logInformation.getFileInfo().getOriginalPath());
            String targetLogFileName = logInformation.getFileInfo().getSourcePath();
            if (neededLogFolder.getName().contains(TagLogUtils.MD_BOOTUP_LOG_NAME)) {
                Utils.doCopy(logInformation.getFileInfo().getOriginalPath(),
                             targetLogFileName);
                Utils.logi(TAG, "Copy MD bootupLogs from " + neededLogFolder.getAbsolutePath()
                           + " to " + targetLogFileName);
                continue;
            }
            if (!logInformation.isNeedTag()) {
                Utils.logi(TAG, "no need do tag for " + neededLogFolder);
                continue;
            }
            File tagetFolder = new File(targetLogFileName);
            boolean isNeedFilter = logInformation.getLogType() != Utils.LOG_TYPE_MODEM
                       && TagLogUtils.TAG_EXCEPTION.equals(mTaglogInformation.getTagLogType())
                       && !mTaglogInformation.isNeedAllLogs();
            Utils.logi(TAG, "Tag Logs from " + neededLogFolder.getAbsolutePath()
                    + " to " + targetLogFileName
                    + ". isNeedFilter ? " + isNeedFilter);
            if (!isNeedFilter) {
                LogFileManager.renameTo(neededLogFolder, tagetFolder);
            } else {
                renameLogs(logInformation.getLogType(),
                        neededLogFolder, new File(targetLogFileName));
            }
        }
    }

    private void renameLogs(int logType, File neededLogFolder, File tagetFolder) {
        Utils.logi(TAG, "logType = " + logType
                + ", neededLogFolder = " + neededLogFolder.getAbsolutePath()
                + ", targetFolder = " + tagetFolder.getAbsolutePath());
        if (neededLogFolder.listFiles() == null
                || neededLogFolder.listFiles().length == 0) {
            Utils.logw(TAG, "no file in folder " + neededLogFolder.getAbsolutePath());
            return;
        }
        final Pattern pattern = TagLogUtils.TAG_LOG_FILTER_PATTERN.get(logType);
        if (pattern == null) {
            LogFileManager.renameTo(neededLogFolder, tagetFolder);
            return;
        }
        List<File> needFilesList = new ArrayList<File>();
        final Set<File> patternMatchedFiles = new HashSet<File>();
        File[] notMatchedPatternFiles = neededLogFolder.listFiles(new FileFilter() {
            @Override
            public boolean accept(File file) {
                //do filter with last modify time
                if (pattern.matcher(file.getName()).matches()) {
                    patternMatchedFiles.add(file);
                    return false;
                }
                return true;
            }
        });
        if (notMatchedPatternFiles != null) {
            Collections.addAll(needFilesList, notMatchedPatternFiles);
        }
        List<File> nearExpTimeFilesList = getFilesNearExpTime(patternMatchedFiles);
        if (nearExpTimeFilesList.size() > 0) {
            needFilesList.addAll(nearExpTimeFilesList);
        }
        Utils.logi(TAG, "needFilesList.size() = " + needFilesList.size()
                + ", neededLogFolder.listFiles().length = " + neededLogFolder.listFiles().length);
        if (needFilesList.size() == neededLogFolder.listFiles().length) {
            LogFileManager.renameTo(neededLogFolder, tagetFolder);
        } else {
            tagetFolder.mkdirs();
            for (File file : needFilesList) {
                LogFileManager.renameTo(file,
                        new File(tagetFolder + File.separator + file.getName()));
            }
        }
    }

    /**
     * Filter out all the files that satisfy the following conditions from the various files
     *  in which the log folder participates in the recycle (patternMatchedFiles):
     * a. Only one file
     * b. expTime - file's last modify time < TagLogUtils.TAG_LOG_FILTER_TIME
     *
     * @param patternMatchedFiles Set<File>
     * @return List<File>
     */
    private List<File> getFilesNearExpTime(Set<File> patternMatchedFiles) {
        List<File> nearExpTimeFilesList = new ArrayList<File>();;
        Map<String, List<File>> patternMatchedFileGroups = new HashMap<String, List<File>>();
        for (File patternMatchedFile : patternMatchedFiles) {
            String prefixFileName = patternMatchedFile.getName().split("_")[0] + "_";
            if (patternMatchedFileGroups.containsKey(prefixFileName)) {
                patternMatchedFileGroups.get(prefixFileName).add(patternMatchedFile);
            } else {
                List<File> patternMatchedFileGroup = new ArrayList<File>();
                patternMatchedFileGroup.add(patternMatchedFile);
                patternMatchedFileGroups.put(prefixFileName, patternMatchedFileGroup);
            }
        }
        long expTime = TagLogUtils.getCurrentTime(
                mTaglogInformation.getExpTime(), "yyyy_MMdd_HHmmss");
        for (List<File> fileList : patternMatchedFileGroups.values()) {
            if (fileList.size() == 1) {
                nearExpTimeFilesList.add(fileList.get(0));
            } else {
                for (File file : fileList) {
                    if (expTime - file.lastModified() <= TagLogUtils.TAG_LOG_FILTER_TIME) {
                        nearExpTimeFilesList.add(file);
                    } else {
                        Utils.logw(TAG, file + " is filter out!");
                    }
                }
            }
        }
        return nearExpTimeFilesList;
    }

    protected void notifyTaglogSize() {
        long totalTaglogSize = 0;
        for (LogInformation logInfo : mNeededTaglogFileList) {
            logInfo.calculateLogFiles();
            totalTaglogSize += logInfo.getLogSize();
        }
        Intent taglogSizeIntent = new Intent();
        taglogSizeIntent.setAction(Utils.ACTION_TAGLOG_SIZE);
        taglogSizeIntent.putExtra(Utils.BROADCAST_KEY_LOG_SIZE, totalTaglogSize);
        if (mInputIntent != null) {
            taglogSizeIntent.putExtras(mInputIntent);
        }
        Utils.logi(TAG, "sent out broadcast : com.mediatek.syslogger.taglog.size with "
                + "compress_size = " + totalTaglogSize);
        Utils.sendBroadCast(taglogSizeIntent);
    }

    protected void doFilesManager() {
        Utils.logi(TAG, "-->doFilesManager");
        outputBufferLogs();
        DBManager.getInstance().changeFileStateToWaiting(mFileListStr);

        Utils.logd(TAG, "insert taglogid = " + mTaglogId + ", filelist = " + mFileListStr);
        mTaglogManagerHandler.obtainMessage(TagLogUtils.MSG_DO_FILEMANAGER, null).sendToTarget();

        checkFileState();
        addResultInTalogFolder();
    }

    protected void outputBufferLogs() {
        TagLogUtils.outputBufferLogs(mTaglogFolder);
        List<String> result = TagEEResultReport.getInstance().getResultReportStr(mTaglogFolder);
        if (result == null) {
            Utils.logd(TAG, "no need out put modem ee report.");
            return;
        }
        File eeReportFile = new File(mTaglogFolder + File.separator
                            + TagLogUtils.MODEM_EE_TAG_REPORT);
        if (!eeReportFile.exists()) {
            LogFileManager.createNewFile(eeReportFile);
            TagLogUtils.writeListContentToFile(result, eeReportFile);
        }
    }

    protected void addResultInTalogFolder() {
        // Remove "Temp_" from taglog folder name
        int waitTimeOut = 20 * 60;
        while (!DBManager.getInstance().isFileInDependence(mTaglogFolder)) {
            try {
                if (waitTimeOut == 0) {
                    Utils.logw(TAG, "wait time out for file dependence!");
                    break;
                }
                Utils.logw(TAG, "cannot remove tmg, wait 1s");
                waitTimeOut--;
                Thread.sleep(1000);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        String newTaglogFolder = mTaglogFolder.replace(TagLogUtils.TAGLOG_TEMP_FOLDER_PREFIX, "");
        String failMessage = getFailMessage();
        if (!failMessage.isEmpty()) {
            newTaglogFolder += failMessage;
        }
        int renameTimeout = 0;
        while (!LogFileManager.renameTo(new File(mTaglogFolder),
                new File(newTaglogFolder))) {
            try {
                Thread.sleep(100);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            renameTimeout += 100;
            if (renameTimeout >= 5000) {
                Utils.logw(TAG, mTaglogFolder + " rename to " + newTaglogFolder + " is failed!");
                break;
            }
        }
        if (renameTimeout < 5000) {
            mTaglogFolder = newTaglogFolder;
        }
    }

    protected String getFailMessage() {
        String result = "";
        if (isLogLost()) {
            result = "_fail_for_log_lost";
        } else {
            result = TagEEResultReport.getInstance().getTagEEResultForFolderName();
        }
        Utils.logi(TAG, "getFailMessage : " + result);
        return result;
    }
    private boolean isLogLost() {
        for (LogInformation logInformation : mNeededTaglogFileList) {
            int logType = logInformation.getLogType();
            if (Utils.LOG_TYPE_SET.contains(logType)) {
                return false;
            }
        }
        return true;
    }
    private File createSopFile() {
        Utils.logi(TAG, "-->createSopFile");
        File checkSopFile =
                new File(new File(mTaglogFolder).getAbsolutePath() + File.separator
                        + "checksop.txt");
        if (checkSopFile != null) {
            if (!checkSopFile.exists()) {
                LogFileManager.createNewFile(checkSopFile);
            }
            TagLogUtils.writeStringToFile(getSOPContent().toString(), checkSopFile);
        }
        return checkSopFile;
    }

    protected StringBuffer getSOPContent() {
        StringBuffer content = new StringBuffer("Check SOP result:\n");
        SharedPreferences preferences =
                MyApplication.getInstance().getSharedPreferences("calibration_data",
                        Context.MODE_PRIVATE);
        boolean isCalibrated = preferences.getBoolean("calibrationData", false);
        String calibration = new String("Calibration data is downloaded: " + isCalibrated + "\n");

        String keyIVSR = "ivsr_setting";
        long checked =
                Settings.System.getLong(MyApplication.getInstance().getContentResolver(), keyIVSR,
                        0);
        String ivsr = "The IVSR is " + String.valueOf(checked) + "\n";
        Utils.logd(TAG, "IVSR enable status: " + ivsr);

        String buildNumberStr = SystemProperties.get("ro.build.display.id");
        String buildProduct = SystemProperties.get("ro.build.product");
        String basebandVersion = SystemProperties.get("gsm.version.baseband");
        StringBuilder buildInfo = new StringBuilder("===Build Version Information===");
        buildInfo.append("\nBuild Number: " + buildNumberStr)
                .append("\nThe production is " + buildProduct)
                .append(" with " + Utils.BUILD_TYPE + " build")
                .append("\nAnd the baseband version is " + basebandVersion + "\n");
        Utils.logd(TAG, "Build number is " + buildInfo.toString());

        content.append(calibration).append(ivsr).append(buildInfo.toString());
        //for Customer1
        String buildDisplayId = "version: " + Build.DISPLAY + "\n";
        String buildId = "buildid: " + SystemProperties.get("ro.build.vendor_sw_ver")
                          + "\n";
        String buildType = "buildtype: " + Utils.BUILD_TYPE + "\n";
        content.append(buildDisplayId).append(buildId).append(buildType);
        return content;
    }

    protected void setTargetForLogFiles(List<LogInformation> logInfoList) {
        Utils.logi(TAG, "-->setTargetForLogFiles");
        String currentTime = TagLogUtils.getCurrentTimeString();
        String targetConnsysFileName = "ConnsysLog_" + currentTime;
        String targetModemFileName = "ModemLog_" + currentTime;

        for (LogInformation logInfo : logInfoList) {
            logInfo.setTargetTagFolder(mTaglogFolder);
            if (logInfo.getLogType() == Utils.LOG_TYPE_MODEM
                && logInfo.isNeedTag()) {
                File originalFile = new File(logInfo.getFileInfo().getOriginalPath());
                String tagParentFolderName = mTaglogFolder + File.separator + targetModemFileName;
                if (!LogFileManager.mkdirs(new File(tagParentFolderName))) {
                    Utils.loge(TAG, "Create folder fail: " + tagParentFolderName);
                }
                logInfo.getFileInfo().setSourcePath(
                   tagParentFolderName + File.separator + originalFile.getName());
                logInfo.setTargetFileName(targetModemFileName + TagLogUtils.ZIP_LOG_SUFFIX);
            }
            if (Utils.CONNSYS_LOG_TYPE_SET.contains(logInfo.getLogType())) {
                if (logInfo.isNeedTag()) {
                    File originalFile = new File(logInfo.getFileInfo().getOriginalPath());
                    String tagParentFolderName = mTaglogFolder + File.separator
                           + targetConnsysFileName + File.separator
                           + originalFile.getParentFile().getName();
                    if (!LogFileManager.mkdirs(new File(tagParentFolderName))) {
                        Utils.loge(TAG, "Create folder fail: " + tagParentFolderName);
                    }
                    logInfo.setTargetFileName(targetConnsysFileName + TagLogUtils.ZIP_LOG_SUFFIX);
                    logInfo.getFileInfo().setSourcePath(
                       tagParentFolderName + File.separator + originalFile.getName());
                }
            }
         }
    }

    protected void reportTaglogResult(boolean isSuccessful) {
        Utils.logi(TAG, "-->reportTaglogResult(), isSuccessful = " + isSuccessful);
        Intent intent = new Intent();
        intent.setAction(Utils.ACTION_TAGLOG_TO_LOG2SERVER);
        intent.putExtra(Utils.KEY_NOTIFY_LOG2SERVER_REASON, Utils.NOTIFY_LOG2SERVER_REASON_DONE);
        intent.putExtra(Utils.BROADCAST_KEY_TAGLOG_RESULT, isSuccessful
                ? Utils.BROADCAST_VAL_TAGLOG_SUCCESS : Utils.BROADCAST_VAL_TAGLOG_FAILED);

        String taglogFolder = mTaglogFolder;
        intent.putExtra(Utils.BROADCAST_KEY_TAGLOG_PATH, taglogFolder);
        Utils.logi(TAG, "Broadcast out : taglogFolder = " + taglogFolder);
        String dbPathInTaglog = "";
        for (LogInformation logInformation : mNeededTaglogFileList) {
            int logType = logInformation.getLogType();
            if (logType == TagLogUtils.LOG_TYPE_AEE) {
                dbPathInTaglog = taglogFolder + File.separator
                        + logInformation.getLogFile().getName() + File.separator;
                continue;
            }
            String logPathResultKey = TagLogUtils.LOGPATH_RESULT_KEY.get(logType);
            if (logPathResultKey == null) {
                continue;
            }
            String logPath = taglogFolder + File.separator + logInformation.getTargetFileName();
            intent.putExtra(logPathResultKey, logPath);
            Utils.logi(TAG, logPathResultKey + " = " + logPath);
        }

        // Fill the input intent to result
        if (mInputIntent != null) {
            intent.putExtras(mInputIntent);
        } else {
            Utils.logw(TAG, "Data From AEE is null, maybe this is a resume progress");
            for (LogInformation logInformation : mNeededTaglogFileList) {
                int logType = logInformation.getLogType();
                if (logType == TagLogUtils.LOG_TYPE_AEE) {
                    intent.putExtra(Utils.EXTRA_KEY_EXP_PATH, taglogFolder + File.separator
                            + logInformation.getLogFile().getName() + File.separator);
                    intent.putExtra(Utils.EXTRA_KEY_EXP_NAME, logInformation.getLogFile().getName()
                            + ".dbg");
                    intent.putExtra(Utils.EXTRA_KEY_EXP_ZZ, Utils.EXTRA_VALUE_EXP_ZZ);
                }
            }
        }
        if (!dbPathInTaglog.isEmpty()) {
            intent.putExtra(Utils.EXTRA_KEY_EXP_PATH, dbPathInTaglog);
        }
        Utils.logd(TAG, "expPath = " + intent.getStringExtra(Utils.EXTRA_KEY_EXP_PATH));
        Utils.sendBroadCast(intent);

        Utils.logd(TAG, "ReportTaglogResult done!");
    }

    protected void deInit() {
        Utils.logi(TAG, "deInit()");
        mTaglogManagerHandler.obtainMessage(TagLogUtils.MSG_TAGLOG_DONE, this).sendToTarget();
        DBManager.getInstance().updateTaglogState(mTaglogId, MySQLiteHelper.TAGLOG_STATE_DONE);
    }

    protected int startNotificationBar() {
        Utils.logd(TAG, "-->treatLogFilesStart()");
        if (null == mNeededTaglogFileList ||
            mNeededTaglogFileList.size() < 1) {
            mNeededTaglogFileList = DBManager.getInstance().getFileInfoByIds(mFileListStr);
            Utils.logw(TAG, "-->treatLogFilesStart() get fielList from DB");
        }

        int totalFileCount = 0;
        boolean showNotificationBar = false;
        for (LogInformation logInfo : mNeededTaglogFileList) {
            totalFileCount += logInfo.getFileInfo().getFileCount();
            if (logInfo.getTreatMent() != LogInfoTreatmentEnum.DO_NOTHING) {
                showNotificationBar = true;
            }
        }
        if (!showNotificationBar) {
            Utils.logd(TAG, "no need startNotificationBar!");
            return 0;
        }
        if (mStatusBarNotify == null) {
            mStatusBarNotify = new StatusBarNotify(
                     getTargetFile(), (int) mTaglogId);
        }
        Utils.logd(TAG, "Total file count = " + totalFileCount);
        mStatusBarNotify.updateState(TagLogUtils.MSG_ZIP_FILE_TOTAL_COUNT,
                         totalFileCount);
        return totalFileCount;
    }

    protected void stopNotificationBar(int startTotalFileCount) {
        Utils.logd(TAG, "stopNotificationBar, startTotalFileCount = " + startTotalFileCount);
        mStatusBarNotify.updateState(TagLogUtils.MSG_ZIPPED_FILE_COUNT, startTotalFileCount);
    }

    public String getTaglogFolder() {
        return mTaglogFolder;
    }

    public void setTaglogFolder(String taglogFolder) {
        this.mTaglogFolder = taglogFolder;
    }

    protected String getTargetFile() {
        return new File(mTaglogFolder).getName();
    }
    public TagLogInformation getTaglogInformation() {
        return mTaglogInformation;
    }

    public Handler getTaglogManagerHandler() {
        return mTaglogManagerHandler;
    }
}
