package com.log.handler;

import android.os.Build;
import android.util.Log;

import java.util.HashSet;
import java.util.Set;

/**
 * @author MTK81255
 *
 */
public class LogHandlerUtils {
    public static final String TAG = "LogHandler";

    /**
     * @author MTK81255
     *
     */
    public static enum LogType {
        MOBILE_LOG(1), MODEM_LOG(2), NETWORK_LOG(3), CONNSYSFW_LOG(4), GPSHOST_LOG(5),
        BTHOST_LOG(6), MET_LOG(7);
        public final static int MAX_ID = 7;
        private int mId;

        private LogType(int id) {
            this.mId = id;
        }

        public int getLogTypeId() {
            return mId;
        }

        /**
         * @param logId
         *            int
         * @return LogType
         */
        public static LogType getLogTypeById(int logId) {
            switch (logId) {
            case 1:
                return MOBILE_LOG;
            case 2:
                return MODEM_LOG;
            case 3:
                return NETWORK_LOG;
            case 4:
                return CONNSYSFW_LOG;
            case 5:
                return GPSHOST_LOG;
            case 6:
                return BTHOST_LOG;
            case 7:
                return MET_LOG;
            default:
                return MOBILE_LOG;
            }
        }

        /**
         * @return Set<LogType>
         */
        public static Set<LogType> getAllLogTypes() {
            Set<LogType> logTypes = new HashSet<LogType>();
            for (int i = 1; i <= MAX_ID; i++) {
                logTypes.add(getLogTypeById(i));
            }
            return logTypes;
        }
    }

    /**
     * @author MTK81255
     *
     */
    public interface ILogExecute {
        /**
         * @param logType
         *            LogType
         * @return boolean
         */
        boolean execute(LogType logType);
    }

    /**
     * @author MTK81255
     *
     */
    public enum MobileLogSubLog {
        AndroidLog, KernelLog, SCPLog, ATFLog, BSPLog, MmediaLog, SSPMLog, ADSPLog, MCUPMLog
    }

    /**
     * @author MTK81255
     *
     */
    public enum ModemLogMode {
        USB("1"), SD("2"), PLS("3"),
        USB_USB("1_1"), USB_SD("1_2"), USB_PLS("1_3"),
        SD_USB("2_1"), SD_SD("2_2"), SD_PLS("2_3"),
        PLS_USB("3_1"), PLS_SD("3_2"), PLS_PLS("3_3");
        private String mId;

        private ModemLogMode(String id) {
            this.mId = id;
        }

        /**
         * @param id
         *            String
         * @return ModemLogMode
         */
        public static ModemLogMode getModemLogModeById(String id) {
            switch (id) {
            case "1":
                return USB;
            case "1_1":
                return USB_USB;
            case "1_2":
                return USB_SD;
            case "1_3":
                return USB_PLS;
            case "2":
                return SD;
            case "2_1":
                return SD_USB;
            case "2_2":
                return SD_SD;
            case "2_3":
                return SD_PLS;
            case "3":
                return PLS;
            case "3_1":
                return PLS_USB;
            case "3_2":
                return PLS_SD;
            case "3_3":
                return PLS_PLS;
            default:
                return SD;
            }
        }

        @Override
        public String toString() {
            return mId;
        }
    }

    /**
     * @author MTK81255
     *
     */
    public enum ModemLogStatus {
        PAUSE(0), RUNNING(1), POLLING(2), COPYING(3);
        private int mId;

        private ModemLogStatus(int id) {
            this.mId = id;
        }

        /**
         * @param id
         *            int
         * @return ModemLogStatus
         */
        public static ModemLogStatus getModemLogStatusById(int id) {
            switch (id) {
            case 0:
                return PAUSE;
            case 1:
                return RUNNING;
            case 2:
                return POLLING;
            case 3:
                return COPYING;
            default:
                return PAUSE;
            }
        }

        public int getId() {
            return mId;
        }
    }

    /**
     * @author MTK81255
     *
     */
    public interface IModemEEMonitor {
        /**
         * @param modemEEPath
         *            String
         */
        void modemEEHappened(String modemEEPath);
    }

    /**
     * @author MTK81255
     *
     */
    public enum BTFWLogLevel {
        OFF("0"), LOW_POWER("1"), SQC("2"), DEBUG("3");
        private String mID;

        private BTFWLogLevel(String logLevel) {
            this.mID = logLevel;
        }

        /**
         * @param id
         *            String
         * @return BTFWLogLevel
         */
        public static BTFWLogLevel getBTFWLogLevelByID(String id) {
            switch (id) {
            case "0":
                return OFF;
            case "1":
                return LOW_POWER;
            case "2":
                return SQC;
            case "3":
                return DEBUG;
            default:
                return SQC;
            }
        }

        @Override
        public String toString() {
            return mID;
        }
    }

    /**
     * @author MTK81255
     *
     */
    public interface IAbnormalEventMonitor {
        /**
         * @param logType
         *            LogType
         * @param abnormalEvent
         *            AbnormalEvent
         */
        void abnormalEvenHappened(LogType logType, AbnormalEvent abnormalEvent);
    }

    /**
     * @author MTK81255
     *
     */
    public static enum AbnormalEvent {
        DISCONNECT, STORAGE_FULL, LOG_FILE_LOST, WRITE_FILE_FAILED
    }

    /**
     * Log part.
     *
     * @param tag
     *            String
     * @param msg
     *            String
     */
    public static void logv(String tag, String msg) {
        if (Build.TYPE.equals("eng")) {
            Log.v(tag, msg);
        }
    }

    /**
     * Log part.
     *
     * @param tag
     *            String
     * @param msg
     *            String
     */
    public static void logd(String tag, String msg) {
        if (Build.TYPE.equals("eng")) {
            Log.d(tag, msg);
        }
    }

    /**
     * Log part.
     *
     * @param tag
     *            String
     * @param msg
     *            String
     */
    public static void logi(String tag, String msg) {
        Log.i(tag, msg);
    }

    /**
     * Log part.
     *
     * @param tag
     *            String
     * @param msg
     *            String
     */
    public static void logw(String tag, String msg) {
        Log.w(tag, msg);
    }

    /**
     * Log part.
     *
     * @param tag
     *            String
     * @param msg
     *            String
     */
    public static void loge(String tag, String msg) {
        Log.e(tag, msg);
    }

    /**
     * Log part.
     *
     * @param tag
     *            String
     * @param msg
     *            String
     * @param tr
     *            Throwable
     */
    public static void loge(String tag, String msg, Throwable tr) {
        Log.e(tag, msg, tr);
    }
}
