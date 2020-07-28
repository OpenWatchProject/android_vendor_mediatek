package com.debug.loggerui.taglog;

import com.debug.loggerui.taglog.TagLogUtils.LogInfoTreatmentEnum;
import com.debug.loggerui.taglog.db.DBManager;
import com.debug.loggerui.taglog.db.FileInfoTable;
import com.debug.loggerui.taglog.db.MySQLiteHelper;
import com.debug.loggerui.utils.Utils;

import java.io.File;
import java.text.DecimalFormat;

/**
 * @author MTK81255
 *
 */
public class LogInformation {

    // Type of modem, mobile or network log
    private int mLogType;
    // Log's root file
    private File mLogFile;
    // The total files count of the logFile, for the taglog compressing progress
    // bar
    private int mLogFilesCount;
    // The total files size of the logFile
    private long mLogSize;
    // The forecast taglog zip file's size of the logFile
    private long mTagLogSize;
    // mLogSize - mTagLogSize
    private long mSaveSpace;
    // The style of log files will be deal with
    private LogInfoTreatmentEnum mTreatment;
    // The zip file name for mLogFile If need do zip
    private String mTargetFileName;
    private String mTargetTagFolder;

    private FileInfoTable mFileInfo;
    private int mProgress = 0;
    private long mLastUpdateFilePrgress = 0;
    private Boolean mNeedTag = false;
    /**
     * @param fileInfo FileInfoTable
     */
    public LogInformation(FileInfoTable fileInfo) {
        setFileInfo(fileInfo);
    }
    /**
     * @param logType
     *            int
     * @param logFile
     *            File
     */
    public LogInformation(int logType, File logFile) {
        this(logType, logFile, LogInfoTreatmentEnum.DO_NOTHING);
    }

    /**
     * @param logType int
     * @param logFile File
     * @param treatMent LogInfoTreatmentEnum
     */
    public LogInformation(int logType, File logFile, LogInfoTreatmentEnum treatMent) {
        mLogType = logType;
        mLogFile = logFile;
        mTreatment = treatMent;
        mTargetFileName = mLogFile.getName();
        if (mTreatment == LogInfoTreatmentEnum.ZIP
                || mTreatment == LogInfoTreatmentEnum.ZIP_DELETE) {
            mTargetFileName = mLogFile.getName() + TagLogUtils.ZIP_LOG_SUFFIX;
        }
        mLogFilesCount = getFolderFilesCount(mLogFile.getAbsolutePath());
        initFileInfoTable();
    }
    private void initFileInfoTable() {
        mFileInfo = new FileInfoTable();
        mFileInfo.setLogType(mLogType);
        mFileInfo.setOriginalPath(mLogFile.getAbsolutePath());
        mFileInfo.setSourcePath(mLogFile.getAbsolutePath());
        mFileInfo.setState(MySQLiteHelper.FILEINFO_STATE_PREPARE);
        mFileInfo.setFileCount(mLogFilesCount);
        mFileInfo.setTargetFile(mTargetFileName);
        mFileInfo.setTreatment(String.valueOf(mTreatment));
        mFileInfo.setTag(mNeedTag == true ? "1" : "0");
    }
    /**
     * void.
     */
    public void calculateLogFiles() {
        mLogSize = getFolderOrFileSize(mLogFile.getAbsolutePath());
        mTagLogSize = calculateTagLogSize();
        mSaveSpace = mLogSize - mTagLogSize;
    }

    private long calculateTagLogSize() {
        if (mTreatment != LogInfoTreatmentEnum.ZIP
                && mTreatment != LogInfoTreatmentEnum.ZIP_DELETE) {
            return mLogSize;
        }
        long ratioChangedLevel = TagLogUtils.LOG_COMPRESS_RATIO_CHANGE.get(mLogType, 0L);
        if (ratioChangedLevel == 0) {
            return mLogSize;
        }

        double logCompressRatio = 1.0;
        if (mLogSize <= TagLogUtils.LOG_COMPRESS_RATIO_CHANGE.get(mLogType)) {
            logCompressRatio = TagLogUtils.LOG_COMPRESS_RATIO_MAX.get(mLogType);
        } else {
            logCompressRatio = TagLogUtils.LOG_COMPRESS_RATIO_MIN.get(mLogType);
        }
        return Long.parseLong(new DecimalFormat("0").format(mLogSize * logCompressRatio));
    }

    public int getLogType() {
        return mLogType;
    }

    public File getLogFile() {
        return mLogFile;
    }

    /**
     * @param file
     *            File
     */
    public void setLogFile(File file) {
        mLogFile = file;
        mFileInfo.setSourcePath(file.getAbsolutePath());
        mFileInfo.setTargetFolder(file.getParent());
        this.calculateLogFiles();
    }
    /**
     * @return int
     */
    public int getLogFilesCount() {
        if (mLogFilesCount == 0) {
            mLogFilesCount = mFileInfo.getFileCount();
        }
        return mLogFilesCount;
    }

    public long getLogSize() {
        return mLogSize;
    }

    public long getTagLogSize() {
        return mTagLogSize;
    }

    public long getSaveSpace() {
        return mSaveSpace;
    }

    public LogInfoTreatmentEnum getTreatMent() {
        return mTreatment;
    }

    /**
     * @param treatMent LogInfoTreatmentEnum
     */
    public void setTreatMent(LogInfoTreatmentEnum treatMent) {
        this.mTreatment = treatMent;
        mFileInfo.setTreatment(String.valueOf(treatMent));
    }

    public String getTargetTagFolder() {
        return this.mTargetTagFolder;
    }
    /**
     * @param targetFolder String
     */
    public void setTargetTagFolder(String targetFolder) {
        this.mTargetTagFolder = targetFolder;
        mFileInfo.setTargetFolder(targetFolder);
        if (this.isNeedTag()) {
            mFileInfo.setSourcePath(targetFolder + File.separator
                                    + mLogFile.getName());
        }
    }

    /**
     * @param targetFolder String
     */
    public void setTargetFileFolder(String targetFolder) {
        mFileInfo.setTargetFolder(targetFolder);
    }

    public String getTargetFileName() {
        return mTargetFileName;
    }
    /**
     * @param targetFileName String
     */
    public void setTargetFileName(String targetFileName) {
        this.mTargetFileName = targetFileName;
        mFileInfo.setTargetFile(targetFileName);
    }
    public FileInfoTable getFileInfo() {
        return this.mFileInfo;
    }

    private void setFileInfo(FileInfoTable fileInfo) {
        mFileInfo = fileInfo;
        mTargetTagFolder = fileInfo.getTargetFolder();
        mLogType = fileInfo.getLogType();
        mLogFile = new File(fileInfo.getSourcePath());
        String sourcePath = fileInfo.getSourcePath();
        if (sourcePath != null && sourcePath.contains(";")) {
            String[] filePaths = sourcePath.split(";");
            for (String path : filePaths) {
                File sourceFile = new File(path);
                if (sourceFile.exists()) {
                    mLogFile = sourceFile;
                    Utils.logd(Utils.TAG, "Select file " + path + ", in " + sourcePath);
                    break;
                }
            }
        }
        mTreatment = LogInfoTreatmentEnum.fromString(fileInfo.getTreatment());
        mTargetFileName = fileInfo.getTargetFile();
        mNeedTag = fileInfo.isNeedTag().equals("1") ? true : false;
        calculateLogFiles();
    }
    /**
     * @param fileCount int
     */
    public void addFileProgress(int fileCount) {
        mProgress += fileCount;
        int currentProgress = mProgress;
        if (currentProgress >= mFileInfo.getFileCount()) {
            currentProgress = mFileInfo.getFileCount();
            mProgress = 0;
        }
        if (currentProgress - mLastUpdateFilePrgress > 2
                || currentProgress >= mFileInfo.getFileCount()) {
            DBManager.getInstance().updateFileInfo(mFileInfo.getFileId(),
                    MySQLiteHelper.FILEINFO_STATE_DOING, currentProgress);
            mLastUpdateFilePrgress = currentProgress;
        }
    }
    /**
     * @return Boolean
     */
    public Boolean isNeedTag() {
        return this.mNeedTag;
    }
    /**
     * @param tag Boolean
     */
    public void setTagFlag(Boolean tag) {
        this.mNeedTag = tag;
        mFileInfo.setTag(tag == true ? "1" : "0");
    }
    /**
     *
     * @param filePath
     *            folder path
     *
     * @return file size
     */
    private static int getFolderFilesCount(String filePath) {
        int size = 0;
        File fileRoot = new File(filePath);
        if (!fileRoot.exists()) {
            return 0;
        }
        if (!fileRoot.isDirectory()) {
            size++;
            ;
        } else {
            File[] files = fileRoot.listFiles();
            if (files != null) {
                for (File file : files) {
                    size += getFolderFilesCount(file.getAbsolutePath());
                }
            }
        }
        return size;
    }

    /**
     *
     * @param filePath
     *            folder path
     *
     * @return file size
     */
    private static long getFolderOrFileSize(String filePath) {
        long size = 0;
        File fileRoot = new File(filePath);
        if (!fileRoot.exists()) {
            return 0;
        }
        if (!fileRoot.isDirectory()) {
            size = fileRoot.length();
        } else {
            File[] files = fileRoot.listFiles();
            if (files != null) {
                for (File file : files) {
                    size += getFolderOrFileSize(file.getAbsolutePath());
                }
            }
        }
        return size;
    }
}
