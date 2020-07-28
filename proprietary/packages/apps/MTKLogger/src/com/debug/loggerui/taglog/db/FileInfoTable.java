package com.debug.loggerui.taglog.db;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * This class is used to record owner info.
 */
public class FileInfoTable implements Parcelable {
    private long mFileId;
    private int mLogType;
    private String mOriginalPath;
    private String mSourcePath;
    private String mTargetfolder;
    private String mTargetFile;
    private String mTreatment;
    private String mState;
    private int mFileCount;
    private int mFileProgress;
    private int mLevel;
    private String mNeedTag;
    /**
     * @param id long
     * @param logType int
     * @param originalPath String
     * @param sourcePath String
     * @param targetFolder String
     * @param targetFile String
     * @param treatment String
     * @param state String
     * @param fileCount int
     * @param fileProgress int
     * @param level int
     * @param needTag String
     */
    public FileInfoTable(long id, int logType, String originalPath, String sourcePath,
            String targetFolder, String targetFile, String treatment, String state,
            int fileCount, int fileProgress, int level, String needTag) {
        super();
        this.mFileId = id;
        this.mLogType = logType;
        this.mOriginalPath = originalPath;
        this.mSourcePath = sourcePath;
        this.mTargetfolder = targetFolder;
        this.mTargetFile = targetFile;
        this.mTreatment = treatment;
        this.mState = state;
        this.mFileCount = fileCount;
        this.mFileProgress = fileProgress;
        this.mLevel = level;
        this.mNeedTag = needTag;
    }
    /**
     * @param logType int
     * @param originalpath String
     * @param treatment String
     * @param targetfile String
     */
//    public FileInfoTable(int logType, String originalpath, String treatment,
//            String targetfile) {
//        this.mLogType = logType;
//        this.mOriginalPath = originalpath;
//        this.mTreatment = treatment;
//        this.mTargetFile = targetfile;
//    }
    /**.
     * FileInfoTable
     */
    public FileInfoTable(){

    }
    @Override
    public String toString() {
        return " id:" + mFileId + ", mLogType:" + mLogType + ", mOriginalPath:" + mOriginalPath
                + ", mSourcePath:" + mSourcePath + ", mTargetfolder:" + mTargetfolder
                + ", mTargetFile:" + mTargetFile + ", mTreatment:" + mTreatment
                + ", mState:" + mState + ", mFileCount:" + mFileCount
                + ", mFileProgress:" + mFileProgress + ", mLevel:" + mLevel
                + ", mNeedTag:" + mNeedTag;
    }

    public long getFileId() {
        return mFileId;
    }

    public void setFileId(long id) {
        this.mFileId = id;
    }

    public int getLogType() {
        return mLogType;
    }

    public void setLogType(int logType) {
        this.mLogType = logType;
    }

    public String getOriginalPath() {
        return mOriginalPath;
    }

    public void setOriginalPath(String originalPath) {
        this.mOriginalPath = originalPath;
    }

    public void setSourcePath(String sourcePath) {
        this.mSourcePath = sourcePath;
    }

    public String getSourcePath() {
        return mSourcePath;
    }

    public String getTargetFolder() {
        return mTargetfolder;
    }
    public void setTargetFolder(String targetfolder) {
        this.mTargetfolder = targetfolder;
    }

    public String getTargetFile() {
        return mTargetFile;
    }

    public void setTargetFile(String targetfile) {
        this.mTargetFile = targetfile;
    }

    public String getTreatment() {
        return mTreatment;
    }

    public void setTreatment(String targetfile) {
        this.mTreatment = targetfile;
    }

    public String getState() {
        return mState;
    }

    public void setState(String state) {
        this.mState = state;
    }

    public int getFileCount() {
        return mFileCount;
    }

    public void setFileCount(int filecount) {
        this.mFileCount = filecount;
    }

    public int getFileProgress() {
        return mFileProgress;
    }

    public void setFileProgress(int fileProgress) {
        this.mFileProgress = fileProgress;
    }

    public int getFileLevel() {
        return mLevel;
    }

    public void setFileLevel(int fileLevel) {
        this.mLevel = fileLevel;
    }

    public String isNeedTag() {
        return this.mNeedTag;
    }

    public void setTag(String tag) {
        this.mNeedTag = tag;
    }
    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(mLogType);
        dest.writeString(mOriginalPath);
        dest.writeString(mSourcePath);
        dest.writeString(mTargetfolder);
        dest.writeString(mTargetFile);

        dest.writeString(mTreatment);
        dest.writeString(mState);
        dest.writeInt(mFileCount);
        dest.writeInt(mFileProgress);
        dest.writeInt(mLevel);
        dest.writeString(mNeedTag);

    }

    public static final Creator<FileInfoTable> CREATOR = new Creator<FileInfoTable>() {

        @Override
        public FileInfoTable createFromParcel(Parcel source) {
            long id = source.readLong();
            int logtype = source.readInt();
            String originalpath = source.readString();
            String sourcepath = source.readString();
            String targetfolder = source.readString();

            String targetfile = source.readString();
            String treatment = source.readString();
            String state = source.readString();
            int filecount = source.readInt();
            int fileprogress = source.readInt();
            int level = source.readInt();
            String tag = source.readString();

            FileInfoTable ownerInfo = new FileInfoTable(id, logtype, originalpath, sourcepath,
                    targetfolder, targetfile, treatment, state,
                    filecount, fileprogress, level, tag);
            return ownerInfo;
        }

        @Override
        public FileInfoTable[] newArray(int size) {
            return new FileInfoTable[size];
        }
    };
}
