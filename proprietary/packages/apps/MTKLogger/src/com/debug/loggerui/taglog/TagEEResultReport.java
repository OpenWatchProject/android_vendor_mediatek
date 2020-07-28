package com.debug.loggerui.taglog;

import android.util.LongSparseArray;
import android.util.SparseArray;

import com.debug.loggerui.controller.ModemLogController;
import com.debug.loggerui.utils.Utils;

import java.util.ArrayList;
import java.util.List;


/**
 * .
 * tag modem EE report class for create comment for report
 */
public class TagEEResultReport {

    public static final int RESULT_DEFAULT = -1;
    public static final int RESULT_0 = 0;
    public static final int RESULT_1 = 1;
    public static final int RESULT_2 = 2;
    public static final int RESULT_3 = 3;
    public static final int RESULT_4 = 4;
    public static final int RESULT_5 = 5;
    public static final int RESULT_6 = 6;
    public static final int RESULT_7 = 7;
    /*taglog result : 1, success form modem EE.
     * tag mdlog folder XXXmdlogFolder
     * to taglogXX folder
     */
    private static final String RESULT_FORMAT = "taglog result : %d, %s \n" +
                                             "tag mdlog folder %s, \n to %s \n";


    protected static final String TAG = TagLogUtils.TAGLOG_TAG + "/TagEEResultReport";

    public static final SparseArray<String> KEY_RESULT_FOLDER_NAME_MAP = new SparseArray<String>();
    static {
        KEY_RESULT_FOLDER_NAME_MAP.put(RESULT_DEFAULT, "");
        KEY_RESULT_FOLDER_NAME_MAP.put(RESULT_0, "_success_External_Exception");
        KEY_RESULT_FOLDER_NAME_MAP.put(RESULT_1, "_success");
        KEY_RESULT_FOLDER_NAME_MAP.put(RESULT_2, "_fail_mdlog_not_finish_dump");
        KEY_RESULT_FOLDER_NAME_MAP.put(RESULT_3, "_fail_mdlog_start_dump_time_out");
        KEY_RESULT_FOLDER_NAME_MAP.put(RESULT_4, "_fail_wait_finish_dump_time_out");
        KEY_RESULT_FOLDER_NAME_MAP.put(RESULT_5, "_fail_not_find_EE_folder_in_ftree");
        KEY_RESULT_FOLDER_NAME_MAP.put(RESULT_6, "_fail_EE_folder_not_exist");
        KEY_RESULT_FOLDER_NAME_MAP.put(RESULT_7, "_fail_mdlog_stop");
    }
    public static final SparseArray<String> KEY_RESULT_DISPRIPTION_MAP = new SparseArray<String>();
    static {
        KEY_RESULT_DISPRIPTION_MAP.put(RESULT_0, "success for common EE");
        KEY_RESULT_DISPRIPTION_MAP.put(RESULT_1, "success form modem EE");
        KEY_RESULT_DISPRIPTION_MAP.put(RESULT_2, "fail for mdlog not finish dump");
        KEY_RESULT_DISPRIPTION_MAP.put(RESULT_3, "fail for mdlog not start dump in 3mins");
        KEY_RESULT_DISPRIPTION_MAP.put(RESULT_4, "fail for mdlog not finish dump in 10mins");
        KEY_RESULT_DISPRIPTION_MAP.put(RESULT_5, "fail for not find dump folder in file tree");
        KEY_RESULT_DISPRIPTION_MAP.put(RESULT_6, "fail for EE folder not exist");
        KEY_RESULT_DISPRIPTION_MAP.put(RESULT_7, "fail for mdlog stopped when ee happened");
    }
    private int mResult = RESULT_DEFAULT;
    private String mEEFolderPath = "";
    private String mExceptionInfo = "";
    private List<String> mFindEEPathLog = new ArrayList<String>();
    private TagEEResultReport() {
    }

    private static LongSparseArray<TagEEResultReport> sTagEEResultReport =
            new LongSparseArray<TagEEResultReport>();

    /**
     *.
     * single class
     * @return TagEEResultReport
     */
    public synchronized static TagEEResultReport getInstance() {
            long threadId = Thread.currentThread().getId();
            if (sTagEEResultReport.get(threadId) != null) {
                return sTagEEResultReport.get(threadId);
            } else {
                TagEEResultReport newTagEEResultReport = new TagEEResultReport();
                sTagEEResultReport.put(threadId, newTagEEResultReport);
                return newTagEEResultReport;
            }
    }

    //be called by modemlogT.java
    /**
     * @param result int
     * @param eeFolder String
     */
    public void setResult(int result, String eeFolder) {
        mResult = result;
        mEEFolderPath = eeFolder;
        Utils.logi(TAG, "setResult(), result = " + result + ", tagFolder = " + eeFolder);
    }
    public void setAllModemLogPath(String mdlogPath) {
        mEEFolderPath = mdlogPath;
    }
    public void setExceptionInfo(String exceptionInfo) {
        mExceptionInfo = exceptionInfo;
    }
    public void setFindEEPathLog(List<String> findEEPathLog) {
        mFindEEPathLog = findEEPathLog;
    }
    //be called at rename taglog folder
    public String getTagEEResultForFolderName() {
        return KEY_RESULT_FOLDER_NAME_MAP.get(mResult);
    }
    //becalled by taglog.java output log
    //getlogListString
    //append result string
    /**
     * @param taglogFolder String
     * @return List<String>
     */
    public List<String> getResultReportStr(String taglogFolder) {
        if (mResult == RESULT_DEFAULT) {
            Utils.logw(TAG, "mResult = -1, it not be seted. no need output report ");
            return null;
        }

        List<String> result = new ArrayList<String>();
        //1. taglog result comment--todo
        String resultDiscrip = String.format(RESULT_FORMAT, mResult,
                                             KEY_RESULT_DISPRIPTION_MAP.get(mResult),
                                             mEEFolderPath, taglogFolder);
        result.add(resultDiscrip);
        //2. ee info
        result.add(mExceptionInfo);
        //3. dump log --auto recycle
        result.addAll(ModemLogController.getInstance().getSelfKeyLog());
        //4. taglog search file folow log
        result.add("\n");
        result.addAll(mFindEEPathLog);

        return result;
    }
}
