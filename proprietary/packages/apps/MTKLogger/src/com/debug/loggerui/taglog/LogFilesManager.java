package com.debug.loggerui.taglog;

import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;

import com.debug.loggerui.file.LogFileManager;
import com.debug.loggerui.taglog.TagLogUtils.LogInfoTreatmentEnum;
import com.debug.loggerui.taglog.db.DBManager;
import com.debug.loggerui.taglog.db.MySQLiteHelper;
import com.debug.loggerui.utils.Utils;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.zip.ZipEntry;
import java.util.zip.ZipException;
import java.util.zip.ZipInputStream;
import java.util.zip.ZipOutputStream;

/**
 * @author MTK81255
 *
 */
public class LogFilesManager extends Handler {
    private static final String TAG = TagLogUtils.TAGLOG_TAG + "/LogFilesManager";

    private static final int ZIP_BUFFER_SIZE = 100 * 1024;
    private static final long CHECK_ZIP_TIMER = 60000;

    private static LogFilesManager sInstance = null;
    private Boolean mIsDoZiping = false;

    private LogFilesManager(Looper looper) {
        super(looper);
    }

    /**
     * @return LogFilesManager
     */
    public static LogFilesManager getInstance() {
        if (sInstance == null) {
            synchronized (LogFilesManager.class) {
                if (sInstance == null) {
                    HandlerThread myHandler = new HandlerThread("filemanagerThread");
                    myHandler.setPriority(Thread.MIN_PRIORITY);
                    myHandler.start();
                    sInstance = new LogFilesManager(myHandler.getLooper());
                }
            }
        }
        return sInstance;
    }

    @Override
    public void handleMessage(Message msg) {
        switch (msg.what) {
        case TagLogUtils.MSG_DO_FILEMANAGER:
            checkDoFileManager();
            break;

        default:
            Utils.logw(TAG,
                    "-->mTaglogManagerHandler msg.what = " + msg.what + " is not supported!");
        }
    }

    private void checkDoFileManager() {
        Utils.logi(TAG, "-->checkDoFileManager");
        if (mIsDoZiping) {
            Utils.logw(TAG, "is doing zip, just return");
            return;
        }
        if (!Utils.isTaglogEnable()) {
            return;
        }
        List<LogInformation> logInfomationList =
                DBManager.getInstance().getWaitingDoingLogInformationList();
        if (logInfomationList != null && logInfomationList.size() > 0) {
            mIsDoZiping = true;
            treatLogFiles(logInfomationList);
            try {
                Thread.sleep(1000);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            mIsDoZiping = false;
        }
        if (!this.hasMessages(TagLogUtils.MSG_DO_FILEMANAGER)) {
            this.sendMessageDelayed(this.obtainMessage(TagLogUtils.MSG_DO_FILEMANAGER),
                    CHECK_ZIP_TIMER);
        }
    }

    /**
     * @param logInfoList
     *            List<LogInformation>
     */
    public void treatLogFiles(List<LogInformation> logInfoList) {
        if (logInfoList == null || logInfoList.size() == 0) {
            Utils.logw(TAG, "No log infor need to do, just return!");
            return;
        }
        Utils.logi(TAG, "-->treatLogFiles() logInfoList.size = " + logInfoList.size());
        dealWithZip(logInfoList);
        for (LogInformation logInfo : logInfoList) {
            String sourceFilePath = logInfo.getFileInfo().getSourcePath();
            String targetFilePath = logInfo.getFileInfo().getTargetFolder() + File.separator
                    + logInfo.getTargetFileName();
            Utils.logi(TAG, "for sourceFilePath = " + sourceFilePath + ", treatMent = "
                    + logInfo.getTreatMent() + ", targetFilePath = " + targetFilePath);
            DBManager.getInstance().updateLogInforState(logInfo,
                                    MySQLiteHelper.FILEINFO_STATE_DOING, 0);
            switch (logInfo.getTreatMent()) {
            case ZIP:
                doZip(logInfo);
                if (!checkZip(new File(targetFilePath))) {
                    doDelete(targetFilePath);
                    Utils.logd(TAG, "do zip again for " + targetFilePath);
                    doZip(logInfo);
                    boolean result = checkZip(new File(targetFilePath));
                    if (!result) {
                        doDelete(targetFilePath);
                    }
                }
                break;
            case CUT:
                doCut(sourceFilePath, targetFilePath);
                break;
            case COPY:
                doCopy(sourceFilePath, targetFilePath);
                break;
            case DELETE:
                doDelete(sourceFilePath);
                break;
            case ZIP_DELETE:
                if (doZip(logInfo)) {
                    boolean checkResult = checkZip(new File(targetFilePath));
                    if (!checkResult) {
                        doDelete(targetFilePath);
                        Utils.logd(TAG, "do zip again for " + targetFilePath);
                        doZip(logInfo);
                        checkResult = checkZip(new File(targetFilePath));
                    }
                    if (checkResult) {
                        doDelete(sourceFilePath);
                    } else {
                        doDelete(targetFilePath);
                    }
                } else {
                    Utils.logi(TAG, "do zip fail for " + targetFilePath);
                }
                break;
            case COPY_DELETE:
                Utils.doCopy(sourceFilePath, targetFilePath);
                doDelete(sourceFilePath);
                break;
            case DO_NOTHING:
                break;
            default:
                Utils.logw(TAG, logInfo.getTreatMent() + " is not support!");
            }
            DBManager.getInstance().updateLogInforState(logInfo,
                    MySQLiteHelper.FILEINFO_STATE_DONE, logInfo.getLogFilesCount());
        }
    }

    private void doCopy(String sourcePath, String targetFilePath) {
        String[] subPaths = sourcePath.split(";");
        for (String subPath : subPaths) {
            Utils.doCopy(searchRightFileToCopy(subPath), targetFilePath);
        }
    }

    private String searchRightFileToCopy(String subPath) {
        if (new File(subPath).exists() || !subPath.contains("TagLog")) {
            return subPath;
        }
        // taglog folder will be rename, so it need find the new taglog folder.
        File logParentFile = (new File(subPath)).getParentFile(); // taglog/taglog_xx
        String parentFileName = logParentFile.getName(); // taglog_xx

        File logPath = logParentFile.getParentFile();
        if (logPath != null && logPath.exists()) {
            File[] fileList = logPath.listFiles();
            if (fileList == null) {
                return subPath;
            }
            for (File file : fileList) {
                if (file.getName().contains(parentFileName)) {
                    subPath = subPath.replace(parentFileName, file.getName());
                    break;
                }
            }
        }
        Utils.logi(TAG, "searchRightFileToCopy new path = " + subPath);
        return subPath;
    }

    /**
     * When multi log need zip to the same target, do them at one time.
     */
    private void dealWithZip(List<LogInformation> logInfoList) {
        setZipSourcePath(logInfoList);
        Map<String, List<LogInformation>> multiZipMap = new HashMap<String, List<LogInformation>>();
        List<String> needZipPath = new ArrayList<String>();
        for (LogInformation logInfo : logInfoList) {
            if (logInfo.getTreatMent() == LogInfoTreatmentEnum.ZIP
                    || logInfo.getTreatMent() == LogInfoTreatmentEnum.ZIP_DELETE) {
                String targetZip = logInfo.getFileInfo().getTargetFolder() + File.separator
                        + logInfo.getTargetFileName();

                Utils.logd(TAG, "fileId :" + logInfo.getFileInfo().getFileId());
                if (!multiZipMap.containsKey(targetZip)) {
                    List<LogInformation> curlogInfoList = new ArrayList<LogInformation>();
                    curlogInfoList.add(logInfo);
                    needZipPath.add(logInfo.getFileInfo().getSourcePath());
                    multiZipMap.put(targetZip, curlogInfoList);
                } else {
                    if (!needZipPath.contains(logInfo.getFileInfo().getSourcePath())) {
                        multiZipMap.get(targetZip).add(logInfo);
                        needZipPath.add(logInfo.getFileInfo().getSourcePath());
                    } else {
                        if (logInfo.getTreatMent() == LogInfoTreatmentEnum.ZIP) {
                            logInfo.setTreatMent(LogInfoTreatmentEnum.DO_NOTHING);
                        } else if (logInfo.getTreatMent() == LogInfoTreatmentEnum.ZIP_DELETE) {
                            logInfo.setTreatMent(LogInfoTreatmentEnum.DELETE);
                        }
                    }
                }
            }
        }
        for (String targetFileName : multiZipMap.keySet()) {
            Utils.logd(TAG, "dealWithZip targetFileName = " + targetFileName);
            List<LogInformation> curlogInfoList = multiZipMap.get(targetFileName);
            doZip(curlogInfoList);
            boolean checkZipResult = checkZip(new File(targetFileName));
            if (!checkZipResult) {
                doDelete(targetFileName);
                Utils.logd(TAG, "doZip again for " + targetFileName);
                doZip(curlogInfoList);
                checkZipResult = checkZip(new File(targetFileName));
                if (!checkZipResult) {
                    doDelete(targetFileName);
                }
            }
            for (LogInformation logInfo : curlogInfoList) {
                if (!checkZipResult) {
                    logInfo.setTreatMent(LogInfoTreatmentEnum.DO_NOTHING);
                } else {
                    if (logInfo.getTreatMent() == LogInfoTreatmentEnum.ZIP) {
                        logInfo.setTreatMent(LogInfoTreatmentEnum.DO_NOTHING);
                    } else if (logInfo.getTreatMent() == LogInfoTreatmentEnum.ZIP_DELETE) {
                        logInfo.setTreatMent(LogInfoTreatmentEnum.DELETE);
                    }
                }
            }
        }
    }

    private void setZipSourcePath(List<LogInformation> logInfoList) {
        Utils.logd(TAG, "SetZipSourcePath-->");
        for (LogInformation logInfo : logInfoList) {
            if (logInfo.getTreatMent() != LogInfoTreatmentEnum.ZIP
                    && logInfo.getTreatMent() != LogInfoTreatmentEnum.ZIP_DELETE) {
                continue;
            }
            String currentFilePath = logInfo.getFileInfo().getSourcePath();
            if (!currentFilePath.contains("TMP_Tag")) {
                Utils.logd(TAG, "current file not in taglog folder, no need set for "
                             + currentFilePath);
                continue;
            }
            File currentFile = new File(currentFilePath);
            while (!currentFile.getParentFile().getName().contains("TMP_Tag")) {
                currentFile = currentFile.getParentFile();
            }
            String newSourcePath = "";
            if (Utils.isReleaseToCustomer1()) {
                newSourcePath = currentFile.getParentFile().getAbsolutePath();
            } else {
                newSourcePath = currentFile.getAbsolutePath();
            }
            logInfo.getFileInfo().setSourcePath(newSourcePath);
            Utils.logd(TAG, "setNewSourcePath : " + currentFile.getAbsolutePath()
                    + ", for logType = " + logInfo.getLogType() + ", oldPath = " + currentFilePath);
        }
    }

    /**
     * @param sourceFileList
     *            List<String>
     * @param targetFilePath
     *            String
     * @param needAddFileCount
     *            boolean
     * @return boolean
     */
    public boolean doZip(List<String> sourceFileList, String targetFilePath,
            boolean needAddFileCount) {
        Utils.logi(TAG, "-->doZip() from sourceFileList.size() = " + sourceFileList.size() + " to "
                + targetFilePath);
        boolean result = false;
        ZipOutputStream outZip = null;
        try {
            FileOutputStream fos = LogFileManager.getFileOutputStream(new File(targetFilePath));
            if (fos == null) {
                return false;
            }
            outZip = new ZipOutputStream(fos);
            for (String sourceFilePath : sourceFileList) {
                File sourceFile = new File(sourceFilePath);
                result = zipFile(sourceFile.getParent(), sourceFile.getName(), outZip,
                        needAddFileCount);
            }
            outZip.flush();
            outZip.finish();
            outZip.close();
        } catch (FileNotFoundException e) {
            result = false;
            Utils.loge(TAG, "FileNotFoundException", e);
        } catch (IOException e) {
            result = false;
            Utils.loge(TAG, "FileNotFoundException", e);
        } finally {
            if (outZip != null) {
                try {
                    outZip.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
        return result;
    }

    /**
     * @param sourceFilePath
     *            String
     * @param targetFilePath
     *            String
     * @param needAddFileCount
     *            boolean
     * @return boolean
     */
    public boolean doZip(String sourceFilePath, String targetFilePath, boolean needAddFileCount) {
        Utils.logi(TAG, "-->doZip() from " + sourceFilePath + " to " + targetFilePath);
        boolean result = false;
        ZipOutputStream outZip = null;
        try {
            FileOutputStream fos = LogFileManager.getFileOutputStream(new File(targetFilePath));
            if (fos == null) {
                return false;
            }
            outZip = new ZipOutputStream(fos);
            File sourceFile = new File(sourceFilePath);
            result = zipFile(sourceFile.getParent(), sourceFile.getName(), outZip,
                    needAddFileCount);
            outZip.flush();
            outZip.finish();
            outZip.close();
        } catch (FileNotFoundException e) {
            result = false;
            Utils.loge(TAG, "FileNotFoundException", e);
        } catch (IOException e) {
            result = false;
            Utils.loge(TAG, "FileNotFoundException", e);
        } finally {
            if (outZip != null) {
                try {
                    outZip.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
        return result;
    }

    /**
     * @param logInfo
     *            LogInformation
     * @return boolean
     */
    public boolean doZip(LogInformation logInfo) {
        String targetFilePath = logInfo.getFileInfo().getTargetFolder() + File.separator
                + logInfo.getTargetFileName();
        Utils.logi(TAG, "-->doZip LogInformation for " + targetFilePath);
        boolean result = false;
        ZipOutputStream outZip = null;
        try {
            FileOutputStream fos = LogFileManager.getFileOutputStream(new File(targetFilePath));
            if (fos == null) {
                return false;
            }
            outZip = new ZipOutputStream(fos);
            result = zipFile(logInfo, logInfo.getFileInfo().getSourcePath(), "", outZip);
            outZip.flush();
            outZip.finish();
            outZip.close();
        } catch (FileNotFoundException e) {
            result = false;
            Utils.loge(TAG, "FileNotFoundException", e);
        } catch (IOException e) {
            result = false;
            Utils.loge(TAG, "FileNotFoundException", e);
        } finally {
            if (outZip != null) {
                try {
                    outZip.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
        return result;
    }

    /**
     * @param listLogInfo
     *            List<LogInformation>
     * @return boolean
     */
    public boolean doZip(List<LogInformation> listLogInfo) {
        if (listLogInfo == null || listLogInfo.size() < 1) {
            Utils.logd(TAG, "doZip fail fro listLogInfo == null");
            return false;
        }
        String targetFilePath = listLogInfo.get(0).getFileInfo().getTargetFolder() + File.separator
                + listLogInfo.get(0).getTargetFileName();
        boolean isNeedDoZip = false;
        for (LogInformation logInfo : listLogInfo) {
            if (logInfo.getFileInfo().getFileCount() != logInfo.getFileInfo().getFileProgress()) {
                isNeedDoZip = true;
                break;
            }
        }
        if (!isNeedDoZip) {
            Utils.logi(TAG, "doZip done for " + targetFilePath + ", no need zip again.");
            return true;
        }
        boolean result = false;
        ZipOutputStream outZip = null;
        FileOutputStream fileOutputStream = null;
        try {
            fileOutputStream =
                    LogFileManager.getFileOutputStream(new File(targetFilePath));
            if (fileOutputStream == null) {
                return false;
            }
            outZip = new ZipOutputStream(fileOutputStream);
            for (LogInformation logInfo : listLogInfo) {
                result = zipFile(logInfo, logInfo.getFileInfo().getSourcePath(), "", outZip);
                Utils.logi(TAG, "-->doZip LogInformation for " + targetFilePath + ", fileid = "
                        + logInfo.getFileInfo().getFileId());
            }
            outZip.flush();
            outZip.finish();
            fileOutputStream.close();
            outZip.close();
        } catch (FileNotFoundException e) {
            result = false;
            Utils.loge(TAG, "FileNotFoundException", e);
        } catch (IOException e) {
            result = false;
            Utils.loge(TAG, "FileNotFoundException", e);
        } finally {
            if (fileOutputStream != null) {
                try {
                    fileOutputStream.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
            if (outZip != null) {
                try {
                    outZip.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
        return result;
    }

    /**
     * @param srcRootPath
     *            String
     * @param fileRelativePath
     *            String
     * @param zout
     *            ZipOutputStream
     * @param needAddFileCount
     *            boolean
     * @return boolean
     */
    public boolean zipFile(String srcRootPath, String fileRelativePath, ZipOutputStream zout,
            boolean needAddFileCount) {
        Utils.logd(TAG,
                "zipFile(), srcRootPath=" + srcRootPath + ", fileRelativePath=" + fileRelativePath);
        if (zout == null) {
            Utils.loge(TAG, "Can not zip file into a null stream");
            return false;
        }
        boolean result = false;
        File file = new File(srcRootPath + File.separator + fileRelativePath);
        if (file.exists()) {
            if (file.isFile()) {
                FileInputStream in = null;
                try {
                    in = new FileInputStream(file);
                    ZipEntry entry = new ZipEntry(fileRelativePath);
                    zout.putNextEntry(entry);

                    int len = 0;
                    byte[] buffer = new byte[ZIP_BUFFER_SIZE];
                    while ((len = in.read(buffer)) > -1) {
                        zout.write(buffer, 0, len);
                    }
                    zout.closeEntry();
                    zout.flush();
                    result = true;
                } catch (FileNotFoundException e) {
                    Utils.loge(TAG, "FileNotFoundException", e);
                } catch (IOException e) {
                    Utils.loge(TAG, "IOException", e);
                } finally {
                    if (in != null) {
                        try {
                            in.close();
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                    }
                }
                try {
                    Thread.sleep(100);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                return result;
            } else {
                result = true;
                String[] fileList = file.list();
                if (fileList == null) {
                    return false;
                }
                if (fileList.length <= 0) {
                    ZipEntry entry = new ZipEntry(fileRelativePath + File.separator);
                    try {
                        zout.putNextEntry(entry);
                        zout.closeEntry();
                    } catch (IOException e) {
                        e.printStackTrace();
                        return false;
                    }
                }

                for (String subFileName : fileList) {
                    if (!zipFile(srcRootPath, fileRelativePath + File.separator + subFileName, zout,
                            needAddFileCount)) {
                        result = false;
                        Utils.loge(TAG, "File [" + subFileName + "] zip failed");
                    }
                }
                return result;
            }
        } else {
            Utils.loge(TAG, "File [" + file.getPath() + "] does not exitst");
            return false;
        }
    }

    /**
     * @param logInfo
     *            LogInformation
     * @param fileRelativePath
     *            String
     * @param zout
     *            ZipOutputStream
     * @param srcRootPath
     *            String
     * @return boolean
     */
    public boolean zipFile(LogInformation logInfo, String srcRootPath, String fileRelativePath,
            ZipOutputStream zout) {
        Utils.logd(TAG, "zipFile(), LogInformation, srcRootPath=" + srcRootPath
                + ", fileRelativePath = " + fileRelativePath);
        if (zout == null) {
            Utils.loge(TAG, "Can not zip file into a null stream");
            return false;
        }
        boolean result = false;
        File file = new File(srcRootPath);
        if (file.exists()) {
            if (file.isFile()) {
                FileInputStream in = null;
                try {
                    in = new FileInputStream(file);
                    ZipEntry entry = new ZipEntry(fileRelativePath);
                    zout.putNextEntry(entry);

                    int len = 0;
                    byte[] buffer = new byte[ZIP_BUFFER_SIZE];
                    while ((len = in.read(buffer)) > -1) {
                        zout.write(buffer, 0, len);
                    }
                    zout.closeEntry();
                    zout.flush();
                    result = true;
                } catch (FileNotFoundException e) {
                    Utils.loge(TAG, "FileNotFoundException", e);
                } catch (IOException e) {
                    Utils.loge(TAG, "IOException", e);
                } finally {
                    if (in != null) {
                        try {
                            in.close();
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                    }
                    logInfo.addFileProgress(1);
                }
                return result;
            } else {
                result = true;
                String[] fileList = file.list();
                if (fileList == null) {
                    return false;
                }
                if (fileList.length <= 0) {
                    ZipEntry entry = new ZipEntry(fileRelativePath + File.separator);
                    try {
                        zout.putNextEntry(entry);
                        zout.closeEntry();
                    } catch (IOException e) {
                        e.printStackTrace();
                        return false;
                    }
                }

                for (String subFileName : fileList) {
                    String newRelativePath = fileRelativePath.isEmpty() ? subFileName
                            : (fileRelativePath + File.separator + subFileName);
                    if (!zipFile(logInfo, srcRootPath + File.separator + subFileName,
                            newRelativePath, zout)) {
                        result = false;
                        Utils.loge(TAG, "File [" + subFileName + "] zip failed");
                    }
                }
                return result;
            }
        } else {
            Utils.loge(TAG, "File [" + file.getPath() + "] does not exitst");
            return false;
        }
    }

    /**
     * @param sourceFilePath
     *            String
     * @param targetFilePath
     *            String
     * @return boolean
     */
    public boolean doCut(String sourceFilePath, String targetFilePath) {
        Utils.logi(TAG, "-->doCut() from " + sourceFilePath + " to " + targetFilePath);
        File sourceFile = new File(sourceFilePath);
        if (!sourceFile.exists()) {
            Utils.logw(TAG,
                    "The sourceFilePath = " + sourceFilePath + " is not existes, do cut failed!");
            return false;
        }
        File targetFile = new File(targetFilePath);
        return LogFileManager.renameTo(sourceFile, targetFile);
    }

    /**
     * @param sourceFilePath
     *            String
     * @return boolean
     */
    public boolean doDelete(String sourceFilePath) {
        Utils.logi(TAG, "-->doDelete() for " + sourceFilePath);
        File sourceFile = new File(sourceFilePath);
        if (!sourceFile.exists()) {
            Utils.logw(TAG, "The sourceFilePath = " + sourceFilePath
                    + " is not existes, no need do delete!");
            return true;
        }
        Utils.deleteFile(sourceFile);
        return true;
    }

    /**
     * @param zipFile
     *            File
     * @return boolean
     */
    public boolean checkZip(File zipFile) {
        int buffer = 1024 * 1024 * 10;
        boolean checkResult = true;
        FileInputStream fis = null;
        try {
            fis = new FileInputStream(zipFile);
            ZipInputStream zis = new ZipInputStream(new BufferedInputStream(fis, buffer));
            while (zis.getNextEntry() != null) {
                continue;
            }
            zis.close();
            fis.close();
        } catch (FileNotFoundException e) {
            Utils.logw(TAG, "checkZip FileNotFoundException happen");
            checkResult = false;
        } catch (ZipException e) {
            Utils.logw(TAG, "checkZip ZipException happen");
            checkResult = false;
        } catch (IOException e) {
            Utils.logw(TAG, "checkZip IOException happen");
            checkResult = false;
        }
        Utils.logd(TAG,
                "checkZipFile result = " + checkResult + ", for file " + zipFile.getAbsolutePath());
        return checkResult;
    }
}
