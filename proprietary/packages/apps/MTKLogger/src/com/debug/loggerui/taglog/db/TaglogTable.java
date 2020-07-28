package com.debug.loggerui.taglog.db;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * This class is used to record owner info.
 */
public class TaglogTable implements Parcelable {
    private int mTaglogId;
    private String mTargetfolder;
    private String mState;
    private String mFileList;
    private String mDBPath;
    private String mDBFileName;
    private String mDBZZFileName;
    private String mIsNeedZip;
    private String mIsNeedAllLogs;
    private int mNeedlogType;
    private String mReason;
    private String mFromWhere;
    private String mZzInternalTime;
    /**.
     * TaglogTable
     */
    public TaglogTable(){

    }
    /**
     * @param id int
     * @param targetFolder String
     * @param state String
     * @param filelist String
     * @param dbPath String
     * @param dbfileName String
     * @param zzFileName String
     * @param isneedzip String
     * @param isneedalllogs String
     * @param needlogtype String
     * @param reason String
     * @param fromwhere String
     * @param expTime String
     */
    public TaglogTable(int id, String targetFolder, String state, String filelist
             , String dbPath, String dbfileName, String zzFileName,
             String isneedzip, String isneedalllogs, int needlogtype,
             String reason, String fromwhere, String expTime) {
        super();
        this.mTaglogId = id;
        this.mTargetfolder = targetFolder;
        this.mState = state;
        this.mFileList = filelist;
        this.mDBPath = dbPath;
        this.mDBFileName = dbfileName;
        this.mDBZZFileName = zzFileName;
        this.mIsNeedZip = isneedzip;
        this.mIsNeedAllLogs = isneedalllogs;
        this.mNeedlogType = needlogtype;
        this.mReason = reason;
        this.mFromWhere = fromwhere;
        this.mZzInternalTime = expTime;
    }

    @Override
    public String toString() {
        return " id:" + mTaglogId + ", mTargetfolder:" + mTargetfolder + ", mState:" + mState
                + ", mFileList:" + mFileList + ", mDBPath:" + mDBPath
                + ", mDBFileName:" + mDBFileName + ", mDBZZFileName:" + mDBZZFileName
                + ", mIsNeedZip:" + mIsNeedZip + ", mIsNeedAllLogs:" + mIsNeedAllLogs
                + ", mNeedlogType:" + mNeedlogType + ", mReason:" + mReason
                + ", mFromWhere:" + mFromWhere + ", mExpTime: " + mZzInternalTime;
    }

    public int getTagLogId() {
        return mTaglogId;
    }

    public void setTagLogId(int id) {
        this.mTaglogId = id;
    }

    public String getTargetFolder() {
        return mTargetfolder;
    }

    public void setTargetFolder(String targetfolder) {
        this.mTargetfolder = targetfolder;
    }

    public String getState() {
        return mState;
    }

    public void setState(String state) {
        this.mState = state;
    }

    public String getFileList() {
        return mFileList;
    }

    public void setFileList(String filelist) {
        this.mFileList = filelist;
    }

    public String getDBPath() {
        return mDBPath;
    }

    public void setDBPath(String dbPath) {
        this.mDBPath = dbPath;
    }

    public String getDBFileName() {
        return mDBFileName;
    }

    public void setDBFileName(String dbfilename) {
        this.mDBFileName = dbfilename;
    }
    /**
     * @return String
     */
    public String getmDBZZFileName() {
        return mDBZZFileName;
    }

    public void setDBZZFileName(String dbzzfile) {
        this.mDBZZFileName = dbzzfile;
    }

    public String isNeedZip() {
        return this.mIsNeedZip;
    }

    public void setIsNeedZip(String needzip) {
        this.mIsNeedZip = needzip;
    }

    public String isNeedAllLogs() {
        return this.mIsNeedAllLogs;
    }

    public void setIsNeedAllLogs(String needzip) {
        this.mIsNeedAllLogs = needzip;
    }
    public int getNeedLogType() {
        return this.mNeedlogType;
    }

    public void setNeedLogType(int logtype) {
        this.mNeedlogType = logtype;
    }
    public String getReason() {
        return this.mReason;
    }

    public void setReason(String reason) {
        this.mReason = reason;
    }

    public String getFromWhere() {
        return this.mFromWhere;
    }

    public void setFromWhere(String where) {
        this.mFromWhere = where;
    }

    public void setZzInternalTime(String expTime) {
        this.mZzInternalTime = expTime;
    }

    public String getZzInternalTime() {
        return this.mZzInternalTime;
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(mTargetfolder);
        dest.writeString(mState);
        dest.writeString(mFileList);
        dest.writeString(mDBPath);
        dest.writeString(mDBFileName);
        dest.writeString(mDBZZFileName);

        dest.writeString(mIsNeedZip);
        dest.writeString(mIsNeedAllLogs);
        dest.writeInt(mNeedlogType);
        dest.writeString(mReason);
        dest.writeString(mFromWhere);
        dest.writeString(mZzInternalTime);
    }

    public static final Creator<TaglogTable> CREATOR = new Creator<TaglogTable>() {

        @Override
        public TaglogTable createFromParcel(Parcel source) {
            int id = source.readInt();
            String tartgetfolder = source.readString();
            String state = source.readString();
            String filelist = source.readString();
            String dbpath = source.readString();
            String dbfilename = source.readString();
            String dbzzfilename = source.readString();

            String isneedzip = source.readString();
            String isneedalllog = source.readString();
            int needlogtype = source.readInt();
            String reason = source.readString();
            String fromwhere = source.readString();
            String expTime = source.readString();

            TaglogTable tagloginfo = new TaglogTable(id, tartgetfolder, state, filelist, dbpath,
                    dbfilename, dbzzfilename, isneedzip, isneedalllog,
                    needlogtype, reason, fromwhere, expTime);
            return tagloginfo;
        }

        @Override
        public TaglogTable[] newArray(int size) {
            return new TaglogTable[size];
        }
    };
}
