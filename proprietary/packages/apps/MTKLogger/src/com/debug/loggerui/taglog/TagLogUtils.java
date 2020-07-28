package com.debug.loggerui.taglog;

import android.util.SparseArray;

import com.debug.loggerui.file.LogFileManager;
import com.debug.loggerui.utils.Utils;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.sql.Date;
import java.text.DecimalFormat;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.GregorianCalendar;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.regex.Pattern;

/**
 * @author MTK81255
 *
 */
public class TagLogUtils {
    public static final String TAGLOG_TAG = Utils.TAG + "/TagLog";

    private static final String TAG = TAGLOG_TAG + "/TagLogUtils";

    public static final int MSG_UI_LOCK = 1;
    public static final int MSG_UI_RELEASE = 2;
    public static final int MSG_ZIPPED_FILE_COUNT = 3;
    public static final int MSG_ZIP_FILE_TOTAL_COUNT = 4;
    public static final int MSG_TAGLOG_DONE = 5;
    public static final int MSG_ALL_LOG_STOPPED = 6;
    public static final int MSG_DO_FILEMANAGER = 7;
    public static final int MSG_TAGLOG_MANAGER_INIT = 8;

    public static final String TAGLOG_FOLDER_NAME = "taglog";
    public static final String TAGLOG_TEMP_FOLDER_PREFIX = "TMP_";
    public static final String ZIP_LOG_TEMP_FOLDER_PREFIX = "TMP_";
    public static final String ZIP_LOG_SUFFIX = ".zip";
    public static final String ZIP_MODEMLOG_NAME = "ModemLog" + ZIP_LOG_SUFFIX;
    public static final String ZIP_CONNSYSLOG_NAME = "ConnsysLog" + ZIP_LOG_SUFFIX;
    public static final String MD_BOOTUP_LOG_NAME = "bootupLog";

    public static final long CONFIRM_LOG_STOP_DONE_SLEEP = 2000;
    public static final String LOG_FOR_TAGLOG_FILE_NAME = "taglog.log";
    public static final String MODEM_EE_TAG_REPORT = "ModemEE_TagReport.txt";
    public static final String TAG_ONLY_COPY_DB = "only_copy_db";
    public static final String TAG_EXCEPTION = "tag_exception";
    public static final String TAG_MANUAL = "tag_manual";

    public static final int LOG_STATUS_CHECK_TIME_OUT = 30000;
    public static final int LOG_STATUS_CHECK_TIME_PERIOD = 1000;

    public static final long TAG_LOG_FILTER_TIME = 2 * 60 * 1000;
    public static final SparseArray<Pattern> TAG_LOG_FILTER_PATTERN = new SparseArray<Pattern>();
    static {
        TAG_LOG_FILTER_PATTERN.put(Utils.LOG_TYPE_MOBILE,
                Pattern.compile("^[A-Za-z0-9]+_log_\\w+$"));
        TAG_LOG_FILTER_PATTERN.put(Utils.LOG_TYPE_NETWORK,
                Pattern.compile("^tcpdump_NTLog_\\w+\\.cap\\w*$"));
        TAG_LOG_FILTER_PATTERN.put(Utils.LOG_TYPE_CONNSYSFW,
                Pattern.compile("^[A-Za-z0-9]+_FW_\\w+\\.clog\\w*$"));
        TAG_LOG_FILTER_PATTERN.put(Utils.LOG_TYPE_GPSHOST,
                Pattern.compile("^GPS_HOST_\\w+\\.nma\\w*$"));
        TAG_LOG_FILTER_PATTERN.put(Utils.LOG_TYPE_BTHOST,
                Pattern.compile("^BT_HCI_\\w+\\.cfa\\w*$"));
    }

    public static final String CALIBRATION_DATA_KEY = "calibrationData";

    public static final SparseArray<String> LOGPATH_RESULT_KEY = new SparseArray<String>();
    static {
        LOGPATH_RESULT_KEY.put(Utils.LOG_TYPE_MODEM, Utils.BROADCAST_KEY_MDLOG_PATH);
        LOGPATH_RESULT_KEY.put(Utils.LOG_TYPE_MOBILE, Utils.BROADCAST_KEY_MOBILELOG_PATH);
        LOGPATH_RESULT_KEY.put(Utils.LOG_TYPE_NETWORK, Utils.BROADCAST_KEY_NETLOG_PATH);
    }

    /**
     * @author MTK81255
     *
     */
    // For LogInformation
    public enum LogInfoTreatmentEnum {
        ZIP, CUT, COPY, DELETE, ZIP_DELETE, COPY_DELETE, DO_NOTHING;
        private static final Map<String, LogInfoTreatmentEnum> stringToEnum
                                    = new HashMap<String, LogInfoTreatmentEnum>();
        static {
            // Initialize map from constant name to enum constant
            for (LogInfoTreatmentEnum treatment : values()) {
                stringToEnum.put(treatment.toString(), treatment);
            }
        }

        /**
         * @param symbol String
         * @return LogInfoTreatmentEnum
         */
        public static LogInfoTreatmentEnum fromString(String symbol) {
            return stringToEnum.get(symbol);
        }
    }

    public static final int LOG_TYPE_OTHERS = 0x0;
    public static final int LOG_TYPE_AEE = -0x1;
    public static final int LOG_TYPE_SOP = -0x2;
    public static final int LOG_TYPE_CONNSYS_DUMP = -0x3;
    public static final SparseArray<Long> LOG_COMPRESS_RATIO_CHANGE = new SparseArray<Long>();
    static {
        LOG_COMPRESS_RATIO_CHANGE.put(Utils.LOG_TYPE_MODEM, 10 * 1024 * 1024L);
        LOG_COMPRESS_RATIO_CHANGE.put(Utils.LOG_TYPE_MOBILE, 10 * 1024 * 1024L);
        LOG_COMPRESS_RATIO_CHANGE.put(Utils.LOG_TYPE_NETWORK, 50 * 1024 * 1024L);
    }
    public static final SparseArray<Double> LOG_COMPRESS_RATIO_MAX = new SparseArray<Double>();
    static {
        LOG_COMPRESS_RATIO_MAX.put(Utils.LOG_TYPE_MODEM, 0.8);
        LOG_COMPRESS_RATIO_MAX.put(Utils.LOG_TYPE_MOBILE, 0.8);
        LOG_COMPRESS_RATIO_MAX.put(Utils.LOG_TYPE_NETWORK, 0.8);
    }
    public static final SparseArray<Double> LOG_COMPRESS_RATIO_MIN = new SparseArray<Double>();
    static {
        LOG_COMPRESS_RATIO_MIN.put(Utils.LOG_TYPE_MODEM, 0.3);
        LOG_COMPRESS_RATIO_MIN.put(Utils.LOG_TYPE_MOBILE, 0.2);
        LOG_COMPRESS_RATIO_MIN.put(Utils.LOG_TYPE_NETWORK, 0.3);
    }

    /**
     * @param content String
     * @param file File
     * @return boolean
     */
    public static boolean writeStringToFile(String content, File file) {
        if (file == null || !file.exists() || !file.isFile()) {
            return false;
        }
        Utils.logi(TAG,
                "writeStringToFile() content = " + content + ", File = " + file.getAbsolutePath());
        try {
            FileOutputStream outStream = LogFileManager.getFileOutputStream(file);
            if (outStream == null) {
                return false;
            }
            OutputStreamWriter outStreamWriter = new OutputStreamWriter(outStream);
            outStreamWriter.write(content);
            outStreamWriter.flush();
            outStreamWriter.close();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return true;
    }

    /**
     * @param contentList List<String>
     * @param file File
     * @return boolean
     */
    public static boolean writeListContentToFile(List<String> contentList, File file) {
        if (file == null || !file.exists() || !file.isFile() || contentList == null) {
            return false;
        }
        Utils.logi(TAG, "writeListContentToFile() contentList.size() = " + contentList.size()
                + ", File = " + file.getAbsolutePath());
        try {
            FileOutputStream outStream = LogFileManager.getFileOutputStream(file);
            if (outStream == null) {
                return false;
            }
            OutputStreamWriter outStreamWriter = new OutputStreamWriter(outStream);
            for (String content : contentList) {
                outStreamWriter.write(content + "\n");
            }
            outStreamWriter.flush();
            outStreamWriter.close();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return true;
    }

    /**
     * @return String
     */
    public static synchronized String getCurrentTimeString() {
        String currentStr = TagLogUtils.translateTime2(System.currentTimeMillis());
        try {
            // Sleep 1 second for avoid to return the same time string
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        return currentStr;
    }

    /**
     * @param gmsTimeStr String
     * @param timeFormat String
     * @return String
     */
    public static synchronized String getCurrentTimeString(String gmsTimeStr, String timeFormat) {
        if (gmsTimeStr == null || gmsTimeStr.isEmpty()) {
            return getCurrentTimeString();
        }
        SimpleDateFormat sdf = new SimpleDateFormat(timeFormat, Locale.US);
        java.util.Date myDate;
        String currentStr = "";
        try {
            myDate = sdf.parse(gmsTimeStr);
            currentStr = TagLogUtils.translateTime2(myDate.getTime());
        } catch (ParseException e) {
            e.printStackTrace();
        }
        return currentStr;
    }
    /**
     * @param gmsTimeStr String
     * @param timeFormat String
     * @return long
     */
    public static synchronized long getCurrentTime(String gmsTimeStr, String timeFormat) {
        if (gmsTimeStr == null || gmsTimeStr.isEmpty()) {
            return 0;
        }
        SimpleDateFormat sdf = new SimpleDateFormat(timeFormat, Locale.US);
        java.util.Date myDate;
        long currentLong = 0;
        try {
            myDate = sdf.parse(gmsTimeStr);
            currentLong = myDate.getTime();
        } catch (ParseException e) {
            e.printStackTrace();
        }
        return currentLong;
    }
    /**
     * transfer long time to time string.
     *
     * @param time
     *            long type
     * @return ex: 2012_1221_2359
     */
    public static String translateTime2(long time) {
        GregorianCalendar calendar = new GregorianCalendar();
        DecimalFormat df = new DecimalFormat();
        String pattern = "00";
        df.applyPattern(pattern);
        calendar.setTime(new Date(time));

        int year = calendar.get(Calendar.YEAR);
        int month = calendar.get(Calendar.MONTH) + 1;
        int day = calendar.get(Calendar.DAY_OF_MONTH);
        int hour = calendar.get(Calendar.HOUR_OF_DAY);
        int minu = calendar.get(Calendar.MINUTE);
        int second = calendar.get(Calendar.SECOND);
        return "" + year + "_" + df.format(month) + df.format(day) + "_" + df.format(hour)
                + df.format(minu) + df.format(second);
    }

    /**
     * @param targetFolder String
     */
    public static synchronized void outputBufferLogs(String targetFolder) {
        Utils.logi(TAG, "-->outputBufferLogs, targetFolder = " + targetFolder);
        String logForTagLogPath = new File(targetFolder).getAbsolutePath()
                + File.separator + LOG_FOR_TAGLOG_FILE_NAME;
        File logForTagLogFile = new File(logForTagLogPath);
        if (logForTagLogFile != null) {
            if (!logForTagLogFile.exists()) {
                LogFileManager.createNewFile(logForTagLogFile);
            }
            synchronized (Utils.sLogBufferList) {
                TagLogUtils.writeListContentToFile(Utils.sLogBufferList, logForTagLogFile);
            }
        }
    }

}
