package com.debug.loggerui.utils;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.Serializable;

/**
 * Keep base information of exception in its object.
 *
 * @author MTK80766
 *
 */
public class ExceptionInfo implements Serializable {

    private static final long serialVersionUID = 1L;
    private String mType = ""; // e.g.JE/NE
    private String mSubExpType = ""; // e.g. modem, adsp, system_anr
    private String mLevel = ""; // e.g. FATAL/EXCEPTION
    /*
     * module or process. For not EE, it is module or process,
     * but for EE, it is discription .
     */
    private String mProcess = "";
    private String mTime = "";
    private String mPath = "";
    private String mBuildVersion = ""; // build version of load. e.g. W10.48.MP
    private String mDeviceName = "";
    private String mToolVersion = "2.0";

    private static final int PRE_FILE_SIZE = 1024;
    private static final int TYPE_INDEX = 0;
    private static final int LEVEL_INDEX = 5;
    private static final int DISCRIPTION_INDEX = 6;
    private static final int PROCESS_INDEX = 7;
    private static final int TIME_INDEX = 8;
    private static final int MIN_ZZ_INTERNAL_LENGTH = TIME_INDEX + 1;

    private static final String TAG = Utils.TAG + "/ExceptionInfo";

    /**
     * Empty constructor for init build version and device name.
     */
    public ExceptionInfo() {
        setBuildVersion(android.os.Build.DISPLAY);
        setDeviceName(android.os.Build.DEVICE);
    }

    /**
     * Init fields from ZZ_INTERNAL file which created by AEE, In which record
     * base information about exception such as name, time,level etc. The
     * ZZ_INTERNAL file is in the same folder of exception bin file.
     *
     * @param zzPath
     *            path of ZZ_INTERNAL file.
     * @throws IOException
     *             IOException Operations of file may cause this exception
     */
    public void initFieldsFromZZ(String zzPath) throws IOException {
        Utils.logd(TAG, "ZZ_INTERNAL's Path:" + zzPath);
        File zzFile = new File(zzPath);
        if (!zzFile.exists()) {
            throw new IOException("ZZ_INTERNAL file is not exist!");
        }
        if (!zzFile.isFile()) {
            throw new IOException("ZZ_INTERNAL file is not a file!");
        }
        FileInputStream fis = new FileInputStream(zzFile);
        byte[] buf = new byte[PRE_FILE_SIZE];
        StringBuilder sb = new StringBuilder();
        int len = 0;
        while ((len = fis.read(buf)) != -1) {
            sb.append(new String(buf, 0, len));
        }
        fis.close();
        String[] arr = sb.toString().split(",");
        Utils.logd(TAG, "split array size = " + arr.length);
        if (arr.length < MIN_ZZ_INTERNAL_LENGTH) {
            // throw new IOException("fields count in ZZ_INTERNAL file are not "
            // + Utils.ZZ_INTERNAL_LENGTH);
            Utils.loge(TAG, "fields count in ZZ_INTERNAL file are not " + MIN_ZZ_INTERNAL_LENGTH);
            if (arr.length > TYPE_INDEX) {
                setType(arr[TYPE_INDEX]);
                setLevel("");
                setSubType("");
                setProcess("");
                setTime("");
            }
            return;
        }
        setType(arr[TYPE_INDEX]);
        setLevel(arr[LEVEL_INDEX]);
        setSubType(arr[DISCRIPTION_INDEX]);
        setProcess(arr[PROCESS_INDEX]);
        setTime(arr[TIME_INDEX]);

    }

    /**
     *
     * @return Exception type
     */
    public String getType() {
        return mType;
    }

    /**
     *
     * @return Exception description
     */
    public String getSubExpType() {
        return mSubExpType;
    }

    /**
     *
     * @return Exception level
     */
    public String getLevel() {
        return mLevel;
    }

    /**
     *
     * @return Witch process occur exception
     */
    public String getProcess() {
        return mProcess;
    }

    /**
     *
     * @return When exception occur
     */
    public String getTime() {
        return mTime;
    }

    /**
     *
     * @return db path
     */
    public String getPath() {
        return mPath;
    }

    //
    // public String getmIndex() {
    // return mIndex;
    // }

    /**
     *
     * @return build version
     */
    public String getBuildVersion() {
        return mBuildVersion;
    }

    /**
     *
     * @return device name
     */
    public String getDeviceName() {
        return mDeviceName;
    }

    /**
     *
     * @return tool version
     */
    public String getToolVersion() {
        return mToolVersion;
    }

    /**
     *
     * @param discription
     *            Exception description
     */
    public void setSubType(String discription) {
        this.mSubExpType = discription;
    }

    /**
     *
     * @param level
     *            Exception level
     */
    public void setLevel(String level) {
        if (level.trim().equals("0")) {
            this.mLevel = "FATAL";
        } else if (level.trim().equals("1")) {
            this.mLevel = "EXCEPTION";
        } else {
            Utils.loge(TAG, "mLevel is not a valid value:" + level);
            this.mLevel = level;
        }
    }

    /**
     *
     * @param process
     *            Witch process occur exception
     */
    public void setProcess(String process) {
        this.mProcess = process;
    }

    /**
     *
     * @param type
     *            Exception type
     */
    public void setType(String type) {
        this.mType = type;
    }

    /**
     *
     * @param time
     *            Exception time
     */
    public void setTime(String time) {
        this.mTime = time;
    }

    /**
     *
     * @param path
     *            db file's path
     */
    public void setPath(String path) {
        this.mPath = path;
    }

    /**
     *
     * @param buildVersion
     *            build version
     */
    public void setBuildVersion(String buildVersion) {
        this.mBuildVersion = buildVersion;
    }

    /**
     *
     * @param deviceName
     *            device name
     */
    public void setDeviceName(String deviceName) {
        this.mDeviceName = deviceName;
    }

    /**
     *
     * @param toolVersion
     *            tool version
     */
    public void setToolVersion(String toolVersion) {
        this.mToolVersion = toolVersion;
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append("[Device Name]: " + mDeviceName + "\n\n");
        sb.append("[Build Version]: " + mBuildVersion + "\n\n");
        sb.append("[Exception Level]: " + mLevel + "\n\n");
        sb.append("[Exception Class]: " + mType + "\n\n");
        sb.append("[Exception Type]: " + mSubExpType + "\n\n");
        sb.append("[Process]: " + mProcess + "\n\n");
        sb.append("[Datetime]: " + mTime + "\n");
        return sb.toString();

    }

}
