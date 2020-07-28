package com.debug.loggerui.taglog;

import android.content.Intent;
import android.os.Handler;

import com.debug.loggerui.file.LogFileManager;
import com.debug.loggerui.taglog.TagLogUtils.LogInfoTreatmentEnum;
import com.debug.loggerui.utils.Utils;

import java.io.File;
import java.util.List;
/**
 *
 * @author MTK11515
 *
 */
public class Customer1TagLog extends TagLog {

    private String mTaglogFileName = ""; //TMP_Taglog_xx.zip
    private String mTargetFolderForFileInfoTable = ""; //xx/dumpfiles/
    /**
     *
     * @param taglogManagerHandler
     * Handler
     */
    public Customer1TagLog(Handler taglogManagerHandler) {
        super(taglogManagerHandler);
    }

    @Override
    protected void createTaglogFolder() {
        super.createTaglogFolder();
        mTaglogFileName = new File(mTaglogFolder).getName()
                          + TagLogUtils.ZIP_LOG_SUFFIX;
        mTargetFolderForFileInfoTable = createTargetFolderForFileInfoTable();
        Utils.logi(TAG, "<--createCustomer1TagLogFolder() done! ,taglogFolder = " +
                mTaglogFolder + ", LogFileaName = " + mTaglogFileName);
    }

    private String createTargetFolderForFileInfoTable() {
        String logPath = "";
        String sdCardPath = Utils.getExternalSdPath();
        if (sdCardPath != null && !sdCardPath.isEmpty()) {
            logPath += sdCardPath + "/dumpfiles";
        } else {
            sdCardPath = Utils.getInternalSdPath();
            if (sdCardPath != null && !sdCardPath.isEmpty()) {
                logPath += sdCardPath + "/dumpfiles";
            }
        }
        Utils.logi(TAG, "createCostomerTagLogFolder : " + logPath);
        File tagLogFolder = new File(logPath);
        if (!tagLogFolder.exists()) {
            LogFileManager.mkdirs(tagLogFolder);
        }
        return logPath;
    }

    @Override
    protected void setTargetForLogFiles(List<LogInformation> logInfoList) {
        String targetConnsysFileName = "ConnsysLog_" +
                TagLogUtils.getCurrentTimeString();
        for (LogInformation logInfo : logInfoList) {
            logInfo.setTargetTagFolder(mTaglogFolder);
            logInfo.setTargetFileFolder(mTargetFolderForFileInfoTable);
            logInfo.setTargetFileName(mTaglogFileName);

            if (Utils.CONNSYS_LOG_TYPE_SET.contains(logInfo.getLogType())) {
                if (logInfo.isNeedTag()) {
                File originalFile = new File(logInfo.getFileInfo().getOriginalPath());
                    String tagParentFolderName = mTaglogFolder + File.separator
                           + targetConnsysFileName + File.separator
                           + originalFile.getParentFile().getName();
                    if (!LogFileManager.mkdirs(new File(tagParentFolderName))) {
                        Utils.loge(TAG, "Create folder fail: " + tagParentFolderName);
                    }
                    logInfo.getFileInfo().setSourcePath(
                       tagParentFolderName + File.separator + originalFile.getName());
                }
            }
        }
        setFileTreatment(logInfoList);
    }

    private void setFileTreatment(List<LogInformation> logInfoList) {
        for (LogInformation logInfo : logInfoList) {
            if (LogInfoTreatmentEnum.COPY == logInfo.getTreatMent()) {
                logInfo.setTreatMent(LogInfoTreatmentEnum.ZIP);
            } else {
                logInfo.setTreatMent(LogInfoTreatmentEnum.ZIP_DELETE);
            }
        }
    }
    @Override
    protected String getTargetFile() {
        return new File(mTaglogFileName).getName();
    }

    @Override
    protected void addResultInTalogFolder() {
        if (mTaglogFileName.isEmpty()) {
            if (mNeededTaglogFileList.size() > 0) {
                mTaglogFileName = mNeededTaglogFileList.get(0).getTargetFileName();
            }
        }
        String tagLogzipName = mTargetFolderForFileInfoTable + "/" + mTaglogFileName;
        Utils.logw(TAG, "removeTempFromTalogName: "  + tagLogzipName);
        String newTaglogFileName = tagLogzipName.replace(TagLogUtils.TAGLOG_TEMP_FOLDER_PREFIX, "");
        int renameTimeout = 0;
        while (!LogFileManager.renameTo(new File(tagLogzipName), new File(newTaglogFileName))) {
            try {
                Thread.sleep(100);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            renameTimeout += 100;
            if (renameTimeout >= 5000) {
                Utils.logw(TAG, mTaglogFileName + " rename to "
                           + newTaglogFileName + " is failed!");
                break;
            }
        }
        if (renameTimeout < 5000) {
            mTaglogFileName = mTaglogFileName.replace(TagLogUtils.TAGLOG_TEMP_FOLDER_PREFIX, "");
        }
        checkDeleteLogFiles();
    }

    @Override
    protected void reportTaglogResult(boolean isSuccessful) {
        Utils.logi(TAG, "-->Customer1 reportTaglogResult(), isSuccessful = " + isSuccessful);
        Intent intent = new Intent();
        intent.setAction(Utils.ACTION_TAGLOG_TO_LOG2SERVER);
        String reportFilePath = mTargetFolderForFileInfoTable + "/" + mTaglogFileName;
        Utils.logi(TAG, "Broadcast out : reportFile = " + reportFilePath);
        intent.putExtra("reportFile", reportFilePath);
        // Fill the input intent to result
        if (mInputIntent != null) {
            intent.putExtras(mInputIntent);
        } else {
            Utils.logw(TAG, "Data From AEE is null, maybe this is a resume progress");
            for (LogInformation logInformation : mNeededTaglogFileList) {
                int logType = logInformation.getLogType();
                if (logType == TagLogUtils.LOG_TYPE_AEE) {
                    intent.putExtra(Utils.EXTRA_KEY_EXP_PATH,
                            logInformation.getLogFile().getAbsolutePath() + File.separator);
                    intent.putExtra(Utils.EXTRA_KEY_EXP_NAME, logInformation.getLogFile().getName()
                            + ".dbg");
                    intent.putExtra(Utils.EXTRA_KEY_EXP_ZZ, Utils.EXTRA_VALUE_EXP_ZZ);
                }
            }
        }
        Utils.sendBroadCast(intent);
        Utils.logi(TAG, "ReportTaglogResult done!");
    }

    private void checkDeleteLogFiles() {
        Utils.logd(TAG, "checkDeleteLogFiles-->");
        for (LogInformation logInformation : mNeededTaglogFileList) {
            String fileName = logInformation.getFileInfo().getSourcePath();
            int logType = logInformation.getLogType();
            if (logType != TagLogUtils.LOG_TYPE_AEE) {
                if (fileName != null && !fileName.contains("dumpfiles")) {
                    File logFile = new File(fileName);
                    Utils.deleteFile(logFile);
                    Utils.logd(TAG, "delete file :" + logFile.getAbsolutePath());

                    File[] listFiles = logFile.getParentFile().listFiles();
                    if (listFiles == null || listFiles.length == 0) {
                        Utils.deleteFile(logFile.getParentFile());
                        Utils.logd(TAG, "delete parent file :"
                                   + logFile.getParentFile().getAbsolutePath());
                    }
                } else {
                    Utils.logd(TAG, "no delete file :" + fileName + ", for it is other taglog");
                }
            }
        }
    }
}
