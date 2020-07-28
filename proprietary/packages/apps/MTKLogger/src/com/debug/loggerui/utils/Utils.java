package com.debug.loggerui.utils;

import android.app.ActivityManager;
import android.content.Context;
import android.content.Intent;
import android.hardware.usb.UsbManager;
import android.media.MediaScannerConnection;
import android.media.MediaScannerConnection.OnScanCompletedListener;
import android.net.Uri;
import android.os.Build;
import android.os.Environment;
import android.os.StatFs;
import android.os.SystemClock;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.os.storage.DiskInfo;
import android.os.storage.StorageManager;
import android.os.storage.StorageVolume;
import android.os.storage.VolumeInfo;
import android.util.AndroidRuntimeException;
import android.util.Log;
import android.util.SparseArray;
import android.util.SparseBooleanArray;
import android.util.SparseIntArray;

import com.debug.loggerui.MyApplication;
import com.debug.loggerui.R;
import com.debug.loggerui.controller.LogControllerUtils;
import com.debug.loggerui.file.LogFileManager;
import com.debug.loggerui.settings.SettingsActivity;
import com.debug.loggerui.taglog.TagLogUtils;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * @author MTK81255
 */
public class Utils {
    public static final String TAG = "DebugLoggerUI";
    public static final String ACTION_START_SERVICE = "com.debug.loggerui.DebugLoggerUIService";
    public static final String CONFIG_FILE_NAME = "log_settings";
    public static final String MTK_BSP_PACKAGE = "ro.mtk_bsp_package";

    public static final String NETWORK_STATE = "state";
    public static final String NETWORK_STATE_NONE = "none";
    public static final String NETWORK_STATE_WIFI = "wifi";
    public static final String NETWORK_STATE_MOBILE = "mobile";

    public static final String KEY_SYSTEM_PROPERTY_LOG_PATH_TYPE = "persist.mtklog.log2sd.path";
    public static final String KEY_SYSTEM_PROPERTY_NETLOG_SAVING_PATH =
            "vendor.netlog.writtingpath";
    public static final String KEY_SYSTEM_PROPERTY_NETLOG_RUNNING_FLAG =
            "vendor.mtklog.netlog.Running";
    public static final String KEY_CONFIG_FILE_LOG_PATH_TYPE = "mtklog_path";
    public static final String KEY_SYSTEM_PROPERTY_MODEM_LOG_PATH_TYPE =
            "persist.mtklog.mdlog.path";
    /**
     * Save DebugLoggerUI Self logs to buffer.
     */
    public static LinkedList<String> sLogBufferList = new LinkedList<String>();

    public static final String AEE_DB_HISTORY_FILE = "file_tree.txt";
    public static final String AEE_SYSTEM_PATH = "/data/aee_exp/";
    public static final String AEE_VENDOR_PATH = "/data/vendor/aee_exp/";

    public final static String LOG_BUFFER_FILTER = "TagLog";
    public final static int MAX_LOG_BUFFER_SIZE = 500;

    /**
     * Log related customer config parameters.
     */
    public static final String CUSTOMIZE_CONFIG_FILE = "/system/etc/mtklog-config.prop";
    public static final String CUSTOMIZE_CONFIG_FILE_N = "/system/vendor/etc/mtklog-config.prop";
    public static final String MODEM_MAP_FILE = "/data/mdlog/emdlogger_usb_config.prop";
    /**
     * Broadcast intent to notify other component about current log's state change event.
     */
    public static final String ACTION_LOG_STATE_CHANGED =
            "com.debug.loggerui.intent.action.LOG_STATE_CHANGED";
    /**
     * Which log's state have been changed. Sum of related log type.
     */
    public static final String EXTRA_AFFECTED_LOG_TYPE = "affected_log_type";
    /**
     * The upper affected log's new state. Sum of running log, order by bit.
     */
    public static final String EXTRA_LOG_NEW_STATE = "log_new_state";
    /**
     * If start/stop execution fail, this will contain detail reason.
     */
    public static final String EXTRA_FAIL_REASON = "fail_reason";

    /**
     * Broadcast intent to notify other component about start log result.
     */
    public static final String ACTION_LOG_START_DONE =
            "com.debug.loggerui.intent.action.LOG_START_DONE";
    /**
     * Broadcast intent to notify other component about start log result.
     */
    public static final String ACTION_LOG_STOP_DONE =
            "com.debug.loggerui.intent.action.LOG_STOP_DONE";
    /**
     * Broadcast intent to notify other component about start log result.
     */
    public static final String ACTION_LOG_REBOOT_DONE =
            "com.debug.loggerui.intent.action.LOG_REBOOT_DONE";
    /**
     * If start/stop execution fail, this will contain detail reason.
     */
    public static final String EXTRA_RESULT = "result";
    /**
     * If start/stop execution fail, this will contain detail reason.
     */
    public static final String EXTRA_SUCESS_LOG_TYPES = "sucess_log_types";
    /**
     * If start/stop execution fail, this will contain detail reason.
     */
    public static final String EXTRA_FAIL_LOG_TYPES = "fail_log_types";
    /**
     * The property of monkey running status.
     */
    public static final String PROP_MONKEY = "ro.monkey";

    /**
     * The property of Gemini support.
     */
    public static final String PROP_GEMINI = "ro.mediatek.gemini_support";

    /**
     * Give out command to deal with DebugLoggerUI from adb shell, by broadcast with this action.
     */
    public static final String ACTION_ADB_CMD = "com.debug.loggerui.ADB_CMD";

    public static final String EXTRA_MTKLOG_PATH = "mtklog_path";
    // From Demo : com.debug.loggerui.bypass -e cmd_name get_c2klog_status
    // Return Demo : com.via.bypass.loggerui -e cmd_name get_c2klog_status -ei
    // cmd_result 1/0
    public static final String ACTION_FROM_BYPASS = "com.debug.loggerui.bypass";
    public static final String ACTION_TO_BYPASS = "com.via.bypass.loggerui";
    public static final String VALUE_BYPASS_GET_STATUS = "get_c2klog_status";
    public static final String EXTRA_CMD_RESULT = "cmd_result";

    public static final String ACTION_TO_BTLOG = "com.debug.loggerui.to.btlog";
    public static final String EXTRA_BTLOG_OPERATE = "btlog_operate";
    public static final String VALUE_BTLOG_OPERATE = "clear_logs";
    public static final String BTLOG_PACKAGE = "com.mediatek.bluetooth.dtt";

    /**
     * Which log instance will this adb command happens on, maybe composition of more than one log.
     */
    public static final String EXTRA_ADB_CMD_TARGET = "cmd_target";
    public static final int DEFAULT_ADB_CMD_TARGET = 0;
    /**
     * Detail command operation, like start, stop.
     */
    public static final String EXTRA_ADB_CMD_NAME = "cmd_name";
    public static final String EXTRA_ADB_CMD_LOGNAME = "log_name";

    /**
     * Detail result name of command which DebugLoggerUI executes.
     */
    public static final String EXTRA_RESULT_NAME = "result_name";

    /**
     * Detail result value for loggerui executes command.
     */
    public static final String EXTRA_RESULT_VALUE = "result_value";

    /**
     * After modem reset, modem log will be restarted automatically. When receive this done event,
     * update service status
     */
    public static final String ACTION_MDLOGGER_RESTART_DONE =
            "com.mediatek.mdlogger.AUTOSTART_COMPLETE";

    /**
     * Used to monitor start/stop command.
     */
    public static final int TIMEOUT_CMD = 60000;
    /**
     * Command timeout signal message's what field is expressed by log type added by this field.
     */
    // public static final int MSG_CMD_TIMEOUT_BASE = 1000;
    /**
     * Signal for start/stop command have finish. Message's arg1 stand for message type, arg2 stand
     * for running state, 1 for running, 0 for stopped.
     *
     * If log's state changed itself, this message will also be send out from each log instance.
     */
    public static final int MSG_LOG_STATE_CHANGED = 1;
    /**
     * Some log instance's state event have affected the service's global stage, like memory
     * dump(polling) in ModemLog will blocking all log's operation.
     */
    public static final int MSG_RUNNING_STAGE_CHANGE = 3;
    public static final int MSG_START_LOGS_DONE = 4;
    public static final int MSG_STOP_LOGS_DONE = 5;
    public static final int MSG_RESTART_DONE = 6;
    public static final int MSG_CLEAR_ALL_LOGS_DONE = 7;
    public static final int MSG_GET_GPS_LOCATION_SUPPORT_DONE = 8;
    public static final int MSG_STOP_NETLOG_AT_SHUT_DOWN = 10;
    public static final int MSG_START_BTLOG = 11;
    public static final int MSG_INIT_LOGS_FOR_SERVER_FIRST_START = 12;

    /**
     * Service start up type will affect native behavior For example, for mobile log, at boot up
     * time, "copy" command is needed.
     */
    public static final String SERVICE_STARTUP_TYPE_BOOT = "boot";

    /**
     * At normal time, log service should can only be started at boot time, but the service maybe be
     * killed unexpected, then it will be restarted when user turn on log.
     */
    public static final String SERVICE_STARTUP_TYPE_USER = "user";
    /**
     * Operate log from adb shell command line, with the way of start it up with this type. By this
     * way, it can avoid service already die issue.
     */
    public static final String SERVICE_STARTUP_TYPE_ADB = "adb";
    public static final String EXTRA_SERVICE_STARTUP_TYPE = "startup_type";

    /**
     * When log storage is available again, the former stopped log should be restarted.
     */
    public static final String SERVICE_STARTUP_TYPE_STORAGE_RECOVERY = "storage_recovery";

    /**
     * When modem log or other instance's state changed, they need to notify service to update its
     * status. When service receive this event, just update log running status
     */
    public static final String SERVICE_STARTUP_TYPE_UPDATE = "update";

    /**
     * When exception happens, notify service to start TagLog process.
     */
    public static final String SERVICE_STARTUP_TYPE_EXCEPTION_HAPPEN = "exception_happen";

    /**
     * When storage status change and service miss this event, notify service about it.
     */
    public static final String SERVICE_STARTUP_TYPE_STORAGE_CHANGE = "storage_changed";
    // Transfer storge status changed action to service from LogReceiver
    public static final String EXTRA_MEDIA_ACTION = "media_action";
    public static final String EXTRA_MEDIA_AFFECTED_PATH = "media_affected_path";

    /**
     * When modem log running in USB mode and tether state changed, need to switch modem log port.
     */
    public static final String SERVICE_STARTUP_TYPE_USB_CHANGE = "usb_changed";
    public static final String ACTION_USB_STATE_CHANGED = UsbManager.ACTION_USB_STATE;

    /**
     * When operate log from UI, need to update log auto start status at the same time, so a
     * different start/stop command is needed.
     */
    public static final String LOG_START_STOP_REASON_FROM_UI = "from_ui";

    public static final String LOG_START_STOP_REASON_FROM_TAGLOG = "from_taglog";

    /**
     * At boot up time, wait SD card time out, need to stop related log, even they have not been
     * started from service.
     */
    public static final String SERVICE_SHUTDOWN_TYPE_SD_TIMEOUT = "sd_timeout";

    /**
     * If log storage is full or lost, we should stop log. For network log, a different
     * command(check, not stop) is needed.
     */
    public static final String SERVICE_SHUTDOWN_TYPE_BAD_STORAGE = "storage_full_or_lost";

    /**
     * Send 'check' command to native layer by period, to check native is still OK.
     */
    public static final int CHECK_CMD_DURATION = 10000;

    /**
     * Command to start log from adb command line.
     */
    public static final String ADB_COMMAND_START = "start";
    /**
     * Command to stop log from adb command line.
     */
    public static final String ADB_COMMAND_STOP = "stop";
    /**
     * Command to start log from adb command line.
     */
    public static final String ADB_COMMAND_RESTART = "restart";

    /**
     * Command to configure tag log on/off status.
     */
    public static final String ADB_COMMAND_SWITCH_TAGLOG = "switch_taglog";

    /**
     * Command to configure always tag modem log on/off status.
     */
    public static final String ADB_COMMAND_ALWAYS_TAG_MODEMLOG = "always_tag_modemlog";

    /**
     * Command to configure log save path internal/external.
     */
    public static final String ADB_COMMAND_SWITCH_LOGPATH = "switch_logpath";

    /**
     * Command to configure modem log mode, USB or SD.
     */
    public static final String ADB_COMMAND_SWITCH_MODEM_LOG_MODE = "switch_modem_log_mode";
    /**
     * Command to configure modem log size.
     */
    public static final String ADB_COMMAND_SET_MODEM_LOG_SIZE = "set_modem_log_size";
    /**
     * Command to configure modem abnormal event in PLS mode.
     */
    public static final String ADB_COMMAND_MONITOR_ABNORMAL_EVENT =
            "pls_monitor_modem_abnormal_event";
    /**
     * Command prefix to configure log auto start at boot time value.
     */
    public static final String ADB_COMMAND_SET_LOG_AUTO_START_PREFIX = "set_auto_start_";
    /**
     * Command prefix to configure log auto start at boot time value.
     */
    public static final String ADB_COMMAND_SET_LOG_UI_ENABLED_PREFIX = "set_ui_enabled_";

    /**
     * Command prefix to configure log default size.
     */
    public static final String ADB_COMMAND_SET_LOG_SIZE_PREFIX = "set_log_size_";

    /**
     * Command prefix to configure each log total default size.
     */
    public static final String ADB_COMMAND_SET_TOTAL_LOG_SIZE_PREFIX = "set_total_log_size_";

    /**
     * Command prefix to configure notification show enable/disable.
     */
    public static final String ADB_COMMAND_SET_NOTIFICATION_ENABLE = "show_notification_";

    /**
     * Command prefix to get mtklog path.
     */
    public static final String ADB_COMMAND_GET_MTKLOG_PATH_NAME = "get_mtklog_path";

    /**
     * Set GPS Log save path.
     */
    public static final String ADB_COMMAND_LOG_SAVE_PATH_PREFIX = "log_save_path_";

    /**
     * Command prefix to get type log recycle size.
     */
    public static final String ADB_COMMAND_GET_LOG_RECYCLE_SIZE_NAME = "get_log_recycle_size";
    public static final String ADB_COMMAND_GET_TAGLOG_STATUS_NAME = "get_taglog_status";
    public static final String ADB_COMMAND_GET_LOG_AUTO_STATUS_NAME = "get_log_auto_status";

    /**
     * Command prefix to clear all logs.
     */
    public static final String ADB_COMMAND_CLEAR_ALL_LOGS_NAME = "clear_all_logs";
    public static final String ADB_COMMAND_CLEAR_LOGS_NAME = "clear_logs";
    public static final String ADB_COMMAND_CLEAR_LOGS_VALUE_ALL = "all";

    /**
     * Give out result for loggerui executes command, by broadcast with this action.
     */
    public static final String ACTION_LOGGERUI_BROADCAST_RESULT = "com.debug.loggerui.result";

    /**
     * DebugLoggerUI release to customer from system property value. Common = "0" Customer_1 = "1"
     * Customer_2 = "2"
     */
    public static final String KEY_SYSTEM_PROPERTY_LOGGERUI_RELEASE_VERSION =
            "debug.loggerui.release.version";

    public static final String BUILD_TYPE = Build.TYPE;

    /**
     * @return boolean
     */
    public static boolean isCustomerUserLoad() {
        return Utils.BUILD_TYPE.equals("user")
                && !SystemProperties.get("ro.vendor.aee.build.info", "").equalsIgnoreCase("mtk");
    }

    /**
     * Current supported log type.
     */
    public static final int LOG_TYPE_MOBILE = 0x1;
    public static final int LOG_TYPE_MODEM = 0x2;
    public static final int LOG_TYPE_NETWORK = 0x4;
    public static final int LOG_TYPE_MET = 0x8; // new feature
    public static final int LOG_TYPE_GPSHOST = 0x10;
    public static final int LOG_TYPE_CONNSYSFW = 0x20;
    public static final int LOG_TYPE_BTHOST = 0x40;
    public static final int LOG_TYPE_ALL = -1;
    public static final Set<Integer> LOG_TYPE_SET = new HashSet<Integer>();
    static {
        LOG_TYPE_SET.add(LOG_TYPE_MOBILE);
        LOG_TYPE_SET.add(LOG_TYPE_MODEM);
        LOG_TYPE_SET.add(LOG_TYPE_NETWORK);
        LOG_TYPE_SET.add(LOG_TYPE_MET);
        LOG_TYPE_SET.add(LOG_TYPE_GPSHOST);
        LOG_TYPE_SET.add(LOG_TYPE_CONNSYSFW);
        LOG_TYPE_SET.add(LOG_TYPE_BTHOST);
    }
    public static final int LOG_TYPE_CONNSYS =
            LOG_TYPE_GPSHOST + LOG_TYPE_CONNSYSFW + LOG_TYPE_BTHOST;
    public static final Set<Integer> CONNSYS_LOG_TYPE_SET = new HashSet<Integer>();
    static {
        CONNSYS_LOG_TYPE_SET.add(LOG_TYPE_GPSHOST);
        CONNSYS_LOG_TYPE_SET.add(LOG_TYPE_CONNSYSFW);
        CONNSYS_LOG_TYPE_SET.add(LOG_TYPE_BTHOST);
    }

    public static final SparseIntArray LOG_NAME_MAP = new SparseIntArray();
    static {
        LOG_NAME_MAP.put(LOG_TYPE_MOBILE, R.string.mobile_log_name);
        LOG_NAME_MAP.put(LOG_TYPE_MODEM, R.string.modem_log_name);
        LOG_NAME_MAP.put(LOG_TYPE_NETWORK, R.string.network_log_name);
        LOG_NAME_MAP.put(LOG_TYPE_MET, R.string.met_log_name);
        LOG_NAME_MAP.put(LOG_TYPE_GPSHOST, R.string.gps_log_name);
        LOG_NAME_MAP.put(LOG_TYPE_CONNSYSFW, R.string.connsysFW_log_name);
        LOG_NAME_MAP.put(LOG_TYPE_BTHOST, R.string.bthost_log_name);
        LOG_NAME_MAP.put(LOG_TYPE_CONNSYS, R.string.connsys_log_name);
    }

    /**
     * Current running status.
     */
    public static final String KEY_STATUS_NETWORK = "networklog_enable";
    public static final String KEY_STATUS_MOBILE = "mobilelog_enable";
    public static final String KEY_STATUS_MODEM = "modemlog_enable";
    public static final String KEY_STATUS_MET = "metlog_enable";
    public static final String KEY_STATUS_GPSHOST = "gpshostlog_enable";
    public static final String KEY_STATUS_CONNSYSFW = "connsysfwlog_enable";
    public static final String KEY_STATUS_BTHOST = "btlog_enable";
    public static final int VALUE_STATUS_RUNNING = 1;
    public static final int VALUE_STATUS_STOPPED = 0;
    public static final int VALUE_STATUS_DEFAULT = 0;

    /**
     * Boot up enable/disable option.
     */
    public static final String KEY_START_AUTOMATIC_MOBILE = "mobilelog_autostart";
    public static final String KEY_START_AUTOMATIC_MODEM = "modemlog_autostart";
    public static final String KEY_START_AUTOMATIC_NETWORK = "networklog_autostart";
    public static final String KEY_START_AUTOMATIC_GPSHOST = "gpslog_autostart";
    public static final String KEY_START_AUTOMATIC_CONNSYSFW = "connsysfwlog_autostart";
    public static final String KEY_START_AUTOMATIC_BTHOST = "bthostlog_autostart";
    public static final String KEY_START_AUTOMATIC_MET = "metlog_autostart";

    public static final SparseArray<String> KEY_START_AUTOMATIC_MAP = new SparseArray<String>();
    static {
        KEY_START_AUTOMATIC_MAP.put(LOG_TYPE_MOBILE, KEY_START_AUTOMATIC_MOBILE);
        KEY_START_AUTOMATIC_MAP.put(LOG_TYPE_MODEM, KEY_START_AUTOMATIC_MODEM);
        KEY_START_AUTOMATIC_MAP.put(LOG_TYPE_MET, KEY_START_AUTOMATIC_MET);
        KEY_START_AUTOMATIC_MAP.put(LOG_TYPE_NETWORK, KEY_START_AUTOMATIC_NETWORK);
        KEY_START_AUTOMATIC_MAP.put(LOG_TYPE_GPSHOST, KEY_START_AUTOMATIC_GPSHOST);
        KEY_START_AUTOMATIC_MAP.put(LOG_TYPE_CONNSYSFW, KEY_START_AUTOMATIC_CONNSYSFW);
        KEY_START_AUTOMATIC_MAP.put(LOG_TYPE_BTHOST, KEY_START_AUTOMATIC_BTHOST);
    }
    /**
     * Log size key in settings page.
     */
    public static final SparseArray<String> KEY_LOG_SIZE_MAP = new SparseArray<String>();
    static {
        KEY_LOG_SIZE_MAP.put(LOG_TYPE_MOBILE, "mobilelog_logsize");
        KEY_LOG_SIZE_MAP.put(LOG_TYPE_MODEM, "modemlog_logsize");
        KEY_LOG_SIZE_MAP.put(LOG_TYPE_MET, "metlog_logsize");
        KEY_LOG_SIZE_MAP.put(LOG_TYPE_NETWORK, "networklog_logsize");
        KEY_LOG_SIZE_MAP.put(LOG_TYPE_GPSHOST, "gpslog_logsize");
        KEY_LOG_SIZE_MAP.put(LOG_TYPE_CONNSYSFW, "connsysfwlog_logsize");
        KEY_LOG_SIZE_MAP.put(LOG_TYPE_BTHOST, "bthostlog_logsize");
    }

    /**
     * Log total size key in settings page, at present only applied to mobile log.
     */
    public static final SparseArray<String> KEY_TOTAL_LOG_SIZE_MAP = new SparseArray<String>();
    static {
        KEY_TOTAL_LOG_SIZE_MAP.put(LOG_TYPE_MOBILE, "mobilelog_total_logsize");
        KEY_TOTAL_LOG_SIZE_MAP.put(LOG_TYPE_MODEM, "modemlog_total_logsize");
        KEY_TOTAL_LOG_SIZE_MAP.put(LOG_TYPE_MET, "metlog_total_logsize");
        KEY_TOTAL_LOG_SIZE_MAP.put(LOG_TYPE_NETWORK, "networklog_total_logsize");
        KEY_TOTAL_LOG_SIZE_MAP.put(LOG_TYPE_GPSHOST, "gpslog_total_logsize");
    }

    /**
     * Log running status stored in System Property by native layer Since upper service maybe
     * killed, when restarted, need to query current log running status.
     */
    public static final SparseArray<String> KEY_LOG_RUNNING_STATUS_IN_SYSPROP_MAP =
            new SparseArray<String>();
    static {
        KEY_LOG_RUNNING_STATUS_IN_SYSPROP_MAP.put(LOG_TYPE_NETWORK,
                KEY_SYSTEM_PROPERTY_NETLOG_RUNNING_FLAG);
        KEY_LOG_RUNNING_STATUS_IN_SYSPROP_MAP.put(LOG_TYPE_MOBILE, "vendor.MB.running");
        KEY_LOG_RUNNING_STATUS_IN_SYSPROP_MAP.put(LOG_TYPE_MODEM, "vendor.mdlogger.Running");
        KEY_LOG_RUNNING_STATUS_IN_SYSPROP_MAP.put(LOG_TYPE_MET, "vendor.met.running");
        KEY_LOG_RUNNING_STATUS_IN_SYSPROP_MAP.put(LOG_TYPE_GPSHOST, "vendor.gpsdbglog.enable");
        KEY_LOG_RUNNING_STATUS_IN_SYSPROP_MAP.put(LOG_TYPE_CONNSYSFW, "vendor.connsysfw.running");
        KEY_LOG_RUNNING_STATUS_IN_SYSPROP_MAP.put(LOG_TYPE_BTHOST, "vendor.bthcisnoop.running");
    }
    public static final String VALUE_LOG_RUNNING_STATUS_IN_SYSPROP_ON = "1";
    public static final String VALUE_LOG_RUNNING_STATUS_IN_SYSPROP_OFF = "0";

    /**
     * Log title string which will be shown in status bar when this log is on.
     */
    public static final SparseIntArray KEY_LOG_TITLE_RES_IN_STSTUSBAR_MAP = new SparseIntArray();
    static {
        KEY_LOG_TITLE_RES_IN_STSTUSBAR_MAP.put(LOG_TYPE_NETWORK,
                R.string.notification_title_network);
        KEY_LOG_TITLE_RES_IN_STSTUSBAR_MAP.put(LOG_TYPE_MOBILE, R.string.notification_title_mobile);
        KEY_LOG_TITLE_RES_IN_STSTUSBAR_MAP.put(LOG_TYPE_MODEM, R.string.notification_title_modem);
        KEY_LOG_TITLE_RES_IN_STSTUSBAR_MAP.put(LOG_TYPE_MET, R.string.notification_title_met);
        KEY_LOG_TITLE_RES_IN_STSTUSBAR_MAP.put(LOG_TYPE_GPSHOST, R.string.notification_title_gps);
        KEY_LOG_TITLE_RES_IN_STSTUSBAR_MAP.put(LOG_TYPE_CONNSYSFW,
                R.string.notification_title_connsysfw);
        KEY_LOG_TITLE_RES_IN_STSTUSBAR_MAP.put(LOG_TYPE_BTHOST, R.string.notification_title_bthost);
    }

    public static final boolean VALUE_START_AUTOMATIC_ON = true;
    public static final boolean VALUE_START_AUTOMATIC_OFF = false;
    public static final boolean VALUE_START_AUTOMATIC_DEFAULT = false;

    /**
     * When storage is not available any more at log running time, log will be stopped. Then when
     * storage is OK again, we need to recovery the former running log.
     */
    public static final SparseArray<String> KEY_NEED_RECOVER_RUNNING_MAP =
            new SparseArray<String>();
    static {
        KEY_NEED_RECOVER_RUNNING_MAP.put(LOG_TYPE_MOBILE, "need_recovery_mobile");
        KEY_NEED_RECOVER_RUNNING_MAP.put(LOG_TYPE_MODEM, "need_recovery_modem");
        KEY_NEED_RECOVER_RUNNING_MAP.put(LOG_TYPE_NETWORK, "need_recovery_network");
        KEY_NEED_RECOVER_RUNNING_MAP.put(LOG_TYPE_MET, "need_recovery_met");
        KEY_NEED_RECOVER_RUNNING_MAP.put(LOG_TYPE_GPSHOST, "need_recovery_gpshost");
        KEY_NEED_RECOVER_RUNNING_MAP.put(LOG_TYPE_CONNSYSFW, "need_recovery_connsysfw");
        KEY_NEED_RECOVER_RUNNING_MAP.put(LOG_TYPE_BTHOST, "need_recovery_bthost");
    }
    public static final boolean DEFAULT_VALUE_NEED_RECOVER_RUNNING = false;

    /**
     * When log stopped by it self(from native), record the stop time.
     */
    public static final SparseArray<String> KEY_SELF_STOP_TIME_MAP = new SparseArray<String>();
    static {
        KEY_SELF_STOP_TIME_MAP.put(LOG_TYPE_MOBILE, "self_stop_time_mobile");
        KEY_SELF_STOP_TIME_MAP.put(LOG_TYPE_MODEM, "self_stop_time_modem");
        KEY_SELF_STOP_TIME_MAP.put(LOG_TYPE_MET, "self_stop_time_met");
        KEY_SELF_STOP_TIME_MAP.put(LOG_TYPE_NETWORK, "self_stop_time_network");
        KEY_SELF_STOP_TIME_MAP.put(LOG_TYPE_GPSHOST, "self_stop_time_gpshost");
        KEY_SELF_STOP_TIME_MAP.put(LOG_TYPE_CONNSYSFW, "self_stop_time_connsysfw");
        KEY_SELF_STOP_TIME_MAP.put(LOG_TYPE_BTHOST, "self_stop_time_bthost");
    }

    /**
     * DebugLoggerUI begin to recording time. Milli-second in UTC. -1/0 mean all logs are stopped
     * now
     */
    public static final String KEY_BEGIN_RECORDING_TIME = "begin_recording_time";
    public static final String KEY_END_RECORDING_TIME = "end_recording_time";
    public static final long VALUE_BEGIN_RECORDING_TIME_DEFAULT = 0;


    public static final String LOG_PATH_TYPE_SYSTEM_DATA = "system_data";
    public static final String LOG_PATH_TYPE_DEVICE_STORAGE = "device_storage";
    public static final String LOG_PATH_TYPE_PORTABLE_STORAGE = "portable_storage";
    public static final String LOG_PATH_TYPE_DEFAULT = LOG_PATH_TYPE_DEVICE_STORAGE;

    public static final Map<String, Integer> LOG_PATH_TYPE_STRING_MAPS =
            new HashMap<String, Integer>();
    static {
        LOG_PATH_TYPE_STRING_MAPS.put(LOG_PATH_TYPE_SYSTEM_DATA,
                R.string.log_path_type_label_system_data);
        LOG_PATH_TYPE_STRING_MAPS.put(LOG_PATH_TYPE_DEVICE_STORAGE,
                R.string.log_path_type_label_emmc);
        LOG_PATH_TYPE_STRING_MAPS.put(LOG_PATH_TYPE_PORTABLE_STORAGE,
                R.string.log_path_type_label_sd);
    }
    // LOG_PATH_TYPE_INTERNAL_SD is instead of LOG_PATH_TYPE_DEVICE_STORAGE, and should be removed
    // for Android R.
    public static final String LOG_PATH_TYPE_INTERNAL_SD = "internal_sd";
    // LOG_PATH_TYPE_EXTERNAL_SD is instead of LOG_PATH_TYPE_PORTABLE_STORAGE, and should be removed
    // for Android R.
    public static final String LOG_PATH_TYPE_EXTERNAL_SD = "external_sd";

    public static final SparseArray<String> LOG_CONFIG_PATH_KEY = new SparseArray<String>();
    static {
        LOG_CONFIG_PATH_KEY.put(LOG_TYPE_MOBILE, "com.mediatek.log.mobile.path");
        LOG_CONFIG_PATH_KEY.put(LOG_TYPE_MODEM, "com.mediatek.log.modem.path");
        LOG_CONFIG_PATH_KEY.put(LOG_TYPE_NETWORK, "com.mediatek.log.network.path");
        LOG_CONFIG_PATH_KEY.put(LOG_TYPE_MET, "com.mediatek.log.met.path");
        LOG_CONFIG_PATH_KEY.put(LOG_TYPE_CONNSYSFW, "com.mediatek.log.connsysfw.path");
        LOG_CONFIG_PATH_KEY.put(LOG_TYPE_GPSHOST, "com.mediatek.log.gpshost.path");
        LOG_CONFIG_PATH_KEY.put(LOG_TYPE_BTHOST, "com.mediatek.log.bthost.path");
    }

    public static final String LOG_PATH_PARENT = "/debuglogger/";
    public static final String LOG_PATH_SYSTEM_DATA = "data/";
    public static final String AEE_EXP_PATH = "aee_exp";
    public static String sNetworklogpath = "netlog";
    public static String sMobilelogpath = "mobilelog";
    static {
        if (isMultiLogFeatureOpen()) {
            sNetworklogpath = "aplog";
            sMobilelogpath = "aplog";
        }
    }
    public static final String MODEM_LOG_PATH = "mdlog";
    public static final String MET_LOG_PATH = "metlog";
    public static final String CONNSYS_LOG_PATH = "connsyslog";
    public static final String CONNSYS_DUMP_LOG_PATH = "connsysdump";
    public static final String GPSHOST_LOG_PATH = CONNSYS_LOG_PATH + "/gpshost";
    public static final String CONNSYSFW_LOG_PATH = CONNSYS_LOG_PATH + "/fw";
    public static final String BTHOST_LOG_PATH = CONNSYS_LOG_PATH + "/bthci";
    public static final SparseArray<String> LOG_PATH_MAP = new SparseArray<String>();
    static {
        LOG_PATH_MAP.put(LOG_TYPE_NETWORK, sNetworklogpath);
        LOG_PATH_MAP.put(LOG_TYPE_MOBILE, sMobilelogpath);
        LOG_PATH_MAP.put(LOG_TYPE_MODEM, MODEM_LOG_PATH);
        LOG_PATH_MAP.put(LOG_TYPE_MET, MET_LOG_PATH);
        LOG_PATH_MAP.put(LOG_TYPE_GPSHOST, GPSHOST_LOG_PATH);
        LOG_PATH_MAP.put(LOG_TYPE_CONNSYSFW, CONNSYSFW_LOG_PATH);
        LOG_PATH_MAP.put(LOG_TYPE_BTHOST, BTHOST_LOG_PATH);
    }
    public static final String TAG_LOG_PATH = "taglog";
    public static final String EXT_MODEM_LOG_PATH = "extmdlog";
    public static final String C2K_MODEM_LOG_PATH = "c2kmdlog";
    public static final String C2K_MODEM_EXCEPTION_LOG_PATH = "mdlog3";

    public static final Set<String> CLEAR_LOG_PRE_FIX_FILTERS = new HashSet<String>();
    static {
        CLEAR_LOG_PRE_FIX_FILTERS.add("file_tree.txt");
        CLEAR_LOG_PRE_FIX_FILTERS.add("is_trigger");
        CLEAR_LOG_PRE_FIX_FILTERS.add("_config");
    }

    public static final Set<String> CLEAR_LOG_FILES_LIST = new HashSet<String>();
    static {
        CLEAR_LOG_FILES_LIST.add(sMobilelogpath);
        CLEAR_LOG_FILES_LIST.add(sNetworklogpath);
        CLEAR_LOG_FILES_LIST.add(MODEM_LOG_PATH);
        CLEAR_LOG_FILES_LIST.add(C2K_MODEM_LOG_PATH);
        CLEAR_LOG_FILES_LIST.add(MET_LOG_PATH);
        CLEAR_LOG_FILES_LIST.add(GPSHOST_LOG_PATH);
        CLEAR_LOG_FILES_LIST.add(CONNSYSFW_LOG_PATH);
        CLEAR_LOG_FILES_LIST.add(BTHOST_LOG_PATH);
        CLEAR_LOG_FILES_LIST.add(TAG_LOG_PATH);
    }
    // There maybe more than one modem log type, use this to indicate the modem
    // type index
    public static final int MODEM_LOG_TYPE_DEFAULT = 1;
    public static final int MODEM_LOG_TYPE_EXT = 4;
    // for K2 project
    public static final int MODEM_LOG_K2_INDEX = 8;
    public static final int MODEM_MAX_COUNT = 8;
    public static final String EMDLOGGER_INDEX = "EMDLOGGER_";

    public static List<Integer> sAvailableModemList = new ArrayList<Integer>();
    public static final Set<Integer> MODEM_INDEX_SET = new HashSet<Integer>();
    static {
        MODEM_INDEX_SET.add(MODEM_LOG_TYPE_DEFAULT);
        MODEM_INDEX_SET.add(MODEM_LOG_TYPE_EXT);
        for (int i = 1; i <= MODEM_MAX_COUNT; i++) {
            if (isTypeMDEnable(i)) {
                MODEM_INDEX_SET.add(MODEM_LOG_K2_INDEX + i);
                sAvailableModemList.add(i);
                Utils.logd(TAG + "/Utils", "MODEM_INDEX_SET added index: " + i);
            }
        }
    }

    /**
     * @param modemIndex
     *            int
     * @return boolean
     */
    public static boolean isTypeMDEnable(int modemIndex) {
        // From M1 version, the property changed to "ro.vendor.mtk_md" + modemIndex + "_support"
        return !SystemProperties.get("ro.vendor.mtk_md" + modemIndex + "_support", "0").equals("0")
                // Before M1, the property key is "ro.mtk_enable_md" + modemIndex
                || SystemProperties.get("ro.mtk_enable_md" + modemIndex, "0").equals("1");
    }

    public static final int C2KLOGGER_INDEX = 3;
    // need modify for K2 project mdlogger rename
    public static final SparseArray<String> MODEM_INDEX_FOLDER_MAP = new SparseArray<String>();
    static {
        MODEM_INDEX_FOLDER_MAP.put(MODEM_LOG_TYPE_DEFAULT, MODEM_LOG_PATH);
        MODEM_INDEX_FOLDER_MAP.put(MODEM_LOG_TYPE_EXT, EXT_MODEM_LOG_PATH);
        for (int i = 1; i <= MODEM_MAX_COUNT; i++) {
            if (isTypeMDEnable(i)) {
                MODEM_INDEX_FOLDER_MAP.put(MODEM_LOG_K2_INDEX + i, MODEM_LOG_PATH + i);
                Utils.logd(TAG + "/Utils", "MODEM_INDEX_FOLDER_MAP added index: " + i);
            }
        }
    }

    // need modify for K2 project mdlogger rename
    public static final SparseIntArray MODEM_LOG_NAME_MAP = new SparseIntArray();
    static {
        MODEM_LOG_NAME_MAP.put(MODEM_LOG_TYPE_DEFAULT, R.string.modem_log_name);
        MODEM_LOG_NAME_MAP.put(MODEM_LOG_TYPE_EXT, R.string.ext_modem_log_name);
        for (int i = 1; i <= MODEM_MAX_COUNT; i++) {
            if (isTypeMDEnable(i)) {
                MODEM_LOG_NAME_MAP.put(MODEM_LOG_K2_INDEX + i, R.string.modem_log_name + i);
                Utils.logd(TAG + "/Utils", "MODEM_LOG_NAME_MAP added index: " + i);
            }
        }
    }

    public static final String FILTER_FILE = "catcher_filter.bin";

    public static final String KEY_NETWORK_LOG_DO_PING = "networklog_ping_flag";
    /**
     * Default log size, in the unit of MB.
     */
    public static final String KEY_NETWORK_LOG_LOGSIZE = KEY_LOG_SIZE_MAP.get(LOG_TYPE_NETWORK);
    public static final int DEFAULT_LOG_SIZE = 200;
    public static final int RESERVED_STORAGE_SIZE = 10;
    /**
     * When storage become less than this value, give user notification to delete old log.
     */
    public static final int DEFAULT_STORAGE_WATER_LEVEL = 30;
    /**
     * When monitor not too much storage remaining, give out a notification.
     */
    public static final String ACTION_REMAINING_STORAGE_LOW =
            "com.debug.loggerui.REMAINING_STORAGE_LOW";
    public static final String EXTRA_REMAINING_STORAGE = "remaining_storage";

    /**
     * Error handler part.
     */
    public static final String REASON_DAEMON_UNKNOWN = "1"; // "Temp: DeamonUnable";
    public static final String REASON_STORAGE_NOT_READY = "2"; // "Temp: Storage is not ready yet.";
    public static final String REASON_STORAGE_FULL = "3"; // "Temp: Not enough storage space.";
    // "Temp: Fail to send command to native layer.";
    public static final String REASON_SEND_CMD_FAIL = "4";
    public static final String REASON_DAEMON_DIE = "5"; // "Temp: daemon is dead.";
    public static final String REASON_UNSUPPORTED_LOG = "6"; // "Temp: Not supported log type.";
    // "Temp: storage is not available any more, like unmounted.";
    public static final String REASON_STORAGE_UNAVAILABLE = "7";
    // "Temp: log folder was lost, maybe deleted by user.";
    public static final String REASON_LOG_FOLDER_DELETED = "8";
    public static final String REASON_CMD_TIMEOUT = "9"; // "Start/stop command time out";
    public static final String REASON_LOG_FOLDER_CREATE_FAIL = "10"; // Fail to
                                                                     // create
                                                                     // log
                                                                     // folder
    public static final String REASON_WAIT_SD_TIMEOUT = "11"; // Wait SD card
                                                              // ready timeout
    public static final String REASON_COMMON = "12";
    public static final String REASON_TCPDUMP_FAILED = "13"; // Tcpdump failed
    public static final String REASON_START_FAIL = "14"; // for met log start
                                                         // faliled
    // "Modem log does not work in flight mode";
    public static final String REASON_MODEM_LOG_IN_FLIGHT_MODE = "15";

    public static final Map<String, Integer> FAIL_REASON_DETAIL_MAP =
            new HashMap<String, Integer>();
    static {
        FAIL_REASON_DETAIL_MAP.put(REASON_DAEMON_UNKNOWN, R.string.error_deamon_unable);
        FAIL_REASON_DETAIL_MAP.put(REASON_STORAGE_NOT_READY, R.string.error_storage_not_ready);
        FAIL_REASON_DETAIL_MAP.put(REASON_STORAGE_FULL, R.string.error_storage_full);
        FAIL_REASON_DETAIL_MAP.put(REASON_SEND_CMD_FAIL, R.string.error_send_cmd_fail);
        FAIL_REASON_DETAIL_MAP.put(REASON_DAEMON_DIE, R.string.error_deamon_die);
        FAIL_REASON_DETAIL_MAP.put(REASON_UNSUPPORTED_LOG, R.string.error_unsupport_log);
        FAIL_REASON_DETAIL_MAP.put(REASON_STORAGE_UNAVAILABLE, R.string.error_storage_unavailable);
        FAIL_REASON_DETAIL_MAP.put(REASON_LOG_FOLDER_DELETED, R.string.error_log_folder_deleted);
        FAIL_REASON_DETAIL_MAP.put(REASON_CMD_TIMEOUT, R.string.error_cmd_timeout);
        FAIL_REASON_DETAIL_MAP.put(REASON_LOG_FOLDER_CREATE_FAIL,
                R.string.error_create_log_folder_fail);
        FAIL_REASON_DETAIL_MAP.put(REASON_WAIT_SD_TIMEOUT, R.string.error_wait_sd_timeout);
        FAIL_REASON_DETAIL_MAP.put(REASON_COMMON, R.string.error_common);
        FAIL_REASON_DETAIL_MAP.put(REASON_TCPDUMP_FAILED, R.string.error_tcpdump_failed);
        FAIL_REASON_DETAIL_MAP.put(REASON_START_FAIL, R.string.met_start_failed);
        FAIL_REASON_DETAIL_MAP.put(REASON_MODEM_LOG_IN_FLIGHT_MODE,
                R.string.info_modem_log_in_flight_mode);
    }

    /**
     * Shell command start/stop part.
     */
    public static final String START_CMD_PREFIX = "runshell_command_start_";
    public static final String STOP_CMD_PREFIX = "runshell_command_stop_";

    /**
     * Modemlog SD/USB mode.
     */
    // public static final String KEY_MODEM_MODE = "log_mode";
    public static final String KEY_MD_MODE_1 = "log_mode_1";
    public static final String KEY_MD_MODE_2 = "log_mode_2";
    public static final String MODEM_MODE_IDLE = "0";
    public static final String MODEM_MODE_USB = "1";
    public static final String MODEM_MODE_SD = "2";
    public static final String MODEM_MODE_PLS = "3";

    /**
     * Modemlog clear logs automatic.
     */
    public static final String LOG_TREE_FILE = "file_tree.txt";
    public static final int LOG_SIZE_MODEM1_SIZE = 2000;
    public static final String TAG_PS = "MDLog_PS";
    public static final String TAG_DAK = "MDLog_DAK";
    public static final String TAG_DMDSPMLT = "MDLog_DMDSPMLT";
    public static final String TAG_MD2GMLT = "MDLog_MD2GMLT";
    public static final String TAG_ASTL1 = "MDLog_ASTL1";
    public static final String TAG_L1 = "MDLog_L1";
    public static final String TAG_LOG_FILTER_FILE = "filter_out.txt";

    /**
     * For Settings.
     */
    public static final String SETTINGS_IS_SWITCH_CHECKED = "isSwitchChecked";
    public static final String SDCARD_SIZE = "sdcardSize";

    public static final String TAG_LOG_ENABLE = "tagLogEnable";
    public static final String MET_LOG_ENABLE = "MetLogEnable";

    /**
     * For storage status constant.
     */
    public static final int STORAGE_STATE_OK = 1;
    public static final int STORAGE_STATE_NOT_READY = -1;
    public static final int STORAGE_STATE_FULL = -2;

    /**
     * At running time, log folder maybe deleted by user, so need to check whether related folder
     * still exist in period.
     */
    public static final int DURATION_CHECK_LOG_FOLDER = 60000;

    /**
     * Add for DebugLoggerUI customer configuration, if can not find default value, use this one.
     */
    public static final SparseArray<String> KEY_CONFIG_LOG_AUTO_START_MAP =
            new SparseArray<String>();
    static {
        KEY_CONFIG_LOG_AUTO_START_MAP.put(LOG_TYPE_MOBILE, "com.mediatek.log.mobile.enabled");
        KEY_CONFIG_LOG_AUTO_START_MAP.put(LOG_TYPE_MODEM, "com.mediatek.log.modem.enabled");
        KEY_CONFIG_LOG_AUTO_START_MAP.put(LOG_TYPE_NETWORK, "com.mediatek.log.net.enabled");
        KEY_CONFIG_LOG_AUTO_START_MAP.put(LOG_TYPE_MET, "com.mediatek.log.met.enabled");
        KEY_CONFIG_LOG_AUTO_START_MAP.put(LOG_TYPE_CONNSYSFW, "com.mediatek.log.connsysfw.enabled");
        KEY_CONFIG_LOG_AUTO_START_MAP.put(LOG_TYPE_GPSHOST, "com.mediatek.log.gpshost.enabled");
        KEY_CONFIG_LOG_AUTO_START_MAP.put(LOG_TYPE_BTHOST, "com.mediatek.log.bthost.enabled");
    }
    public static final SparseArray<String> KEY_CONFIG_LOG_SIZE_MAP = new SparseArray<String>();
    static {
        KEY_CONFIG_LOG_SIZE_MAP.put(LOG_TYPE_MOBILE, "com.mediatek.log.mobile.maxsize");
        KEY_CONFIG_LOG_SIZE_MAP.put(LOG_TYPE_MODEM, "com.mediatek.log.modem.maxsize");
        KEY_CONFIG_LOG_SIZE_MAP.put(LOG_TYPE_MET, "com.mediatek.log.met.maxsize");
        KEY_CONFIG_LOG_SIZE_MAP.put(LOG_TYPE_NETWORK, "com.mediatek.log.net.maxsize");
        KEY_CONFIG_LOG_SIZE_MAP.put(LOG_TYPE_CONNSYSFW, "com.mediatek.log.connsysfw.maxsize");
        KEY_CONFIG_LOG_SIZE_MAP.put(LOG_TYPE_GPSHOST, "com.mediatek.log.gps.maxsize");
        KEY_CONFIG_LOG_SIZE_MAP.put(LOG_TYPE_BTHOST, "com.mediatek.log.bthost.maxsize");
    }
    public static final SparseArray<String> KEY_CONFIG_LOG_TOTAL_SIZE_MAP =
            new SparseArray<String>();
    static {
        KEY_CONFIG_LOG_TOTAL_SIZE_MAP.put(LOG_TYPE_MOBILE, "com.mediatek.log.mobile.totalmaxsize");
        KEY_CONFIG_LOG_TOTAL_SIZE_MAP.put(LOG_TYPE_MODEM, "com.mediatek.log.modem.totalmaxsize");
        KEY_CONFIG_LOG_TOTAL_SIZE_MAP.put(LOG_TYPE_MET, "com.mediatek.log.met.totalmaxsize");
        KEY_CONFIG_LOG_TOTAL_SIZE_MAP.put(LOG_TYPE_NETWORK, "com.mediatek.log.net.totalmaxsize");
        KEY_CONFIG_LOG_TOTAL_SIZE_MAP.put(LOG_TYPE_CONNSYSFW,
                "com.mediatek.log.connsysfw.totalmaxsize");
        KEY_CONFIG_LOG_TOTAL_SIZE_MAP.put(LOG_TYPE_GPSHOST, "com.mediatek.log.gps.totalmaxsize");
        KEY_CONFIG_LOG_TOTAL_SIZE_MAP.put(LOG_TYPE_BTHOST, "com.mediatek.log.bthost.totalmaxsize");
    }
    public static final SparseBooleanArray DEFAULT_CONFIG_LOG_AUTO_START_MAP =
            new SparseBooleanArray();
    static {
        DEFAULT_CONFIG_LOG_AUTO_START_MAP.put(LOG_TYPE_MOBILE, true);
        DEFAULT_CONFIG_LOG_AUTO_START_MAP.put(LOG_TYPE_MODEM, true);
        DEFAULT_CONFIG_LOG_AUTO_START_MAP.put(LOG_TYPE_MET, false);
        DEFAULT_CONFIG_LOG_AUTO_START_MAP.put(LOG_TYPE_NETWORK, true);
        DEFAULT_CONFIG_LOG_AUTO_START_MAP.put(LOG_TYPE_CONNSYSFW, true);
        DEFAULT_CONFIG_LOG_AUTO_START_MAP.put(LOG_TYPE_GPSHOST, true);
        DEFAULT_CONFIG_LOG_AUTO_START_MAP.put(LOG_TYPE_BTHOST, true);
    }
    public static final SparseIntArray DEFAULT_CONFIG_LOG_SIZE_MAP = new SparseIntArray();
    static {
        DEFAULT_CONFIG_LOG_SIZE_MAP.put(LOG_TYPE_MOBILE, 500);
        DEFAULT_CONFIG_LOG_SIZE_MAP.put(LOG_TYPE_MODEM, LOG_SIZE_MODEM1_SIZE);
        DEFAULT_CONFIG_LOG_SIZE_MAP.put(LOG_TYPE_MET, 500);
        DEFAULT_CONFIG_LOG_SIZE_MAP.put(LOG_TYPE_NETWORK, 600);
        DEFAULT_CONFIG_LOG_SIZE_MAP.put(LOG_TYPE_CONNSYSFW, 2000);
        DEFAULT_CONFIG_LOG_SIZE_MAP.put(LOG_TYPE_GPSHOST, 200);
        DEFAULT_CONFIG_LOG_SIZE_MAP.put(LOG_TYPE_BTHOST, 2000);
    }
    public static final String KEY_CONFIG_TAGLOG_ENABLED = "com.mediatek.log.taglog.enabled";
    public static final String KEY_CONFIG_ALWAYS_TAG_MODEMLOG_ENABLED =
            "com.mediatek.log.always.tag.modemlog.enabled";
    public static final String KEY_CONFIG_MODEM_AUTORESET_ENABLED =
            "com.mediatek.log.modem.autoreset.enabled";
    public static final String KEY_CONFIG_TAGLOG_ZIP2ONE = "com.mediatek.log.taglog.zip2one";
    private static final String KEY_PREFERENCE_TAGLOG_ZIP2ONE = "taglogZip2one";
    public static final String KEY_CONFIG_TAGLOG_ZIPFILEPATH =
            "com.mediatek.log.taglog.zipfilepath";
    private static final String KEY_PREFERENCE_TAGLOG_ZIPFILEPATH = "taglogZipfilepath";
    public static final String KEY_CONFIG_MODEM_LOG_MODE = "com.mediatek.log.modem.mode";
    public static final String KEY_CONFIG_NOTIFICATION_ENABLED =
            "com.mediatek.log.notification.enabled";
    public static final String KEY_PREFERENCE_NOTIFICATION_ENABLED = "notificationEnabled";
    public static final boolean DEFAULT_NOTIFICATION_ENABLED_VALUE = !isCustomerUserLoad();

    public static final String KEY_CONFIG_MONITOR_MODEM_ABNORMAL_EVENT =
            "com.mediatek.log.monitor.modem.abnormal.event";

    public static final String KEY_CONFIG_MODEMLOG_AUTO_RESET_MODEM =
            "com.mediatek.log.modem.ee.autoreset";
    public static final String KEY_PREFERENCE_MODEMLOG_AUTO_RESET_MODEM =
            "modemLogAutoResetModem";
    // Add for Tag log begin
    /**
     * When exception happened, this broadcast will be sent to ExceptionReporter.
     */
    public static final String ACTION_EXP_HAPPENED = "com.mediatek.log2server.EXCEPTION_HAPPEND";
    // String : If equals "SaveLogManually", the taglog from user manual
    public static final String EXTRA_KEY_EXP_PATH = "path";
    public static final String EXTRA_KEY_EXP_NAME = "db_filename";
    public static final String EXTRA_KEY_EXP_ZZ = "zz_filename";
    public static final String EXTRA_KEY_EXP_TIME = "zz_time";
    public static final String EXTRA_VALUE_EXP_ZZ = "ZZ_INTERNAL";
    public static final String EXTRA_KEY_EXP_FROM_REBOOT = "from_reboot";
    public static final String EXTRA_KEY_TAG_TYPE = "tag_type";
    // Boolean : Is need do zip or just tag
    public static final String EXTRAL_KEY_IS_NEED_ZIP = "is_need_zip";
    // Boolean : Tag all type log folder like debuglogger/mobilelog
    // or saving log like debuglogger/mobilelog/APLog_***
    public static final String EXTRAL_KEY_IS_NEED_ALL_LOGS = "is_need_all_logs";

    // Add for PLS mode
    // String : If equals "SmartLogging", it will work for PLS mode
    public static final String EXTRA_KEY_EXP_REASON = "Reason";
    public static final String EXTRA_KEY_EXP_FROM_WHERE = "from_where";

    // Add for IssueSubmitter
    public static final String EXTRAL_KEY_IS_TAG = "from_is";
    public static final String EXTRAL_IS_VALUE_OF_EXP_NAME = "is_trigger";
    // int : Combination of each Log type
    public static final String EXTRAL_KEY_IS_NEED_LOG_TYPE = "need_log_type";

    public static final String EXTRA_VALUE_EXP_REASON = "SmartLogging";
    public static final String EXTRA_VALUE_FROM_REBOOT = "FROM_REBOOT";
    public static final String MANUAL_SAVE_LOG = "SaveLogManually";

    public static final String EXTRA_KEY_EXTMD_EXP_PATH = "log_path";

    /**
     * When tag log process begin, modem log should not recycle old log folder when compressing was
     * ongoing but wait until compress finished. We use this flag to indicate whether tag log was
     * already started. The stored value means tag log hope us wait such a long time before recycle
     * Time unit: second
     */
    public static final String KEY_TAG_LOG_ONGOING = "tag_log_ongoing";

    /**
     * When taglog is compressing, the stored value is true, else is false.
     */
    public static final String KEY_TAG_LOG_COMPRESSING = "tag_log_compressing";
    /**
     * Taglog receives broadcast from mainActivity.
     */
    public static final String EXTRA_KEY_FROM_MAIN_ACTIVITY = "extra_key_from_main_activity";

    public static final String KEY_MODEM_LOG_FLUSH_PATH = "modem_log_flush_path";
    public static final String KEY_C2K_MODEM_LOGGING_PATH = "c2k_modem_logging_path";
    public static final String KEY_C2K_MODEM_EXCEPTIONG_PATH = "key_c2k_modem_exceptiong_path";

    /**
     * Taglog broadcast configuration possible values.
     */
    public static final int TAGLOG_CONFIG_VALUE_ENABLE = 1;
    public static final int TAGLOG_CONFIG_VALUE_DISABLE = 0;
    public static final int TAGLOG_CONFIG_VALUE_INVALID = -1;

    /**
     * Communication between TagLog and ExceptionReporter.
     */
    public static final String ACTION_TAGLOG_TO_LOG2SERVER = "com.mediatek.syslogger.taglog";
    public static final String BROADCAST_KEY_TAGLOG_RESULT = "TaglogResult";
    public static final String BROADCAST_KEY_TAGLOG_PATH = "TaglogPath";
    public static final String BROADCAST_KEY_MDLOG_PATH = "ModemLogPath";
    public static final String BROADCAST_KEY_MOBILELOG_PATH = "MobileLogPath";
    public static final String BROADCAST_KEY_NETLOG_PATH = "NetLogPath";
    // public static final String BROADCAST_KEY_GPSLOG_PATH = "GPSLogPath";
    // public static final String BROADCAST_KEY_BTLOG_PATH = "BTLogPath";
    public static final String BROADCAST_VAL_TAGLOG_CANCEL = "Cancel";
    public static final String BROADCAST_VAL_TAGLOG_SUCCESS = "Successful";
    public static final String BROADCAST_VAL_TAGLOG_FAILED = "Failed";
    public static final String BROADCAST_VAL_LOGTOOL_STOPPED = "LogToolStopped";
    public static final String ACTION_TAGLOG_SIZE = "com.mediatek.syslogger.taglog.size";
    public static final String BROADCAST_KEY_LOG_SIZE = "compress_size";
    public static final String LOG_TYPE_STOP_BY_TAGLOG = "log_type_stop_by_taglog";

    /**
     * SD card status code.
     */
    public static final int SD_NORMAL = 401;
    public static final int SD_LACK_OF_SPACE = 402;
    public static final int SD_NOT_EXIST = 403;
    public static final int SD_NOT_WRITABLE = 404;

    // Add for restart tagging after killed by system
    public static final String KEY_DEST_MOBILE = "dest_mobile";
    public static final String KEY_DEST_MODEM = "dest_modem";
    public static final String KEY_DEST_MET = "dest_met";
    public static final String KEY_DEST_NETWORK = "dest_network";

    public static final String KEY_DEST_AEE = "dest_aee";
    public static final String KEY_DEST_SOP = "dest_sop";
    public static final String KEY_DEST_LAST_TAGLOG = "dest_last";

    public static final String KEY_TAGGING_DB = "tagging_db";
    public static final String KEY_TAGGING_LAST = "tagging_last";
    /**
     * Where to store tag log.
     */
    public static final String KEY_TAGGING_DEST = "tagging_dest";

    /**
     * Use this key to get whether log2server was enabled from settings provider.
     */
    public static final String KEY_LOG2SERVER_SWITCH = "log2server_dialog_show";

    /**
     * When Taglog begin, it will notify Log2Server can ask user's answer whether to trigger
     * log2server. When Taglog done, Log2Server can start its upload work flow.
     */
    public static final String KEY_NOTIFY_LOG2SERVER_REASON = "reason";
    public static final String NOTIFY_LOG2SERVER_REASON_BEGIN = "begin";
    public static final String NOTIFY_LOG2SERVER_REASON_DONE = "done";

    // Add for Tag log end

    // Add for modem assert begin
    /**
     * When EE happened, our tool maybe dead, so may lost some state information because modem will
     * reset automatically. Add broadcast receiver to receive these informations.
     */
    public static final String ACTION_MDLOGGER_DUMP_BEGIN =
            "com.mediatek.mdlogger.MEMORYDUMP_START";
    public static final String ACTION_MDLOGGER_DUMP_DONE = "com.mediatek.mdlogger.MEMORYDUMP_DONE";
    public static final String EXTRA_MDLOGGER_DUMP_PATH = "LogPath";
    /**
     * Use this flag to indicate mdlogger already begin polling.
     */
    public static final String FLAG_MDLOGGER_POLLING = "polling";
    /**
     * Use this flag to indicate modem log already begin flushing.
     */
    public static final String FLAG_MDLOGGER_FLUSHING = "flushing";

    public static final int MEMORYDUMP_START_TIMEOUT = 60000 * 2;
    public static final int MEMORYDUMP_DONE_TIMEOUT = 60000 * 10;
    // Add for modem assert end

    /**
     * Add for multi user.
     */
    public static final int USER_ID_OWNER = 0;
    public static final int USER_ID_UNDEFINED = -1;
    public static final int USER_ID = UserHandle.myUserId();
    public static final String EXTRA_NEW_USER_ID = Intent.EXTRA_USER_HANDLE;

    /**
     * @return false if is Guest or new user.
     */
    public static boolean isDeviceOwner() {
        return Utils.USER_ID == Utils.USER_ID_OWNER || Utils.USER_ID == Utils.USER_ID_UNDEFINED;
    }

    /**
     * Whether modem log is already running in tether mode, to void duplicated control command.
     */
    public static final String KEY_USB_MODE_VALUE = "usb_mode";
    public static final int VALUE_USB_MODE_UNKNOWN = 0;
    public static final String KEY_USB_CONNECTED_VALUE = "usb_connected";
    // All possible usb mode will be listed here
    public static final int USB_MODE_NONE = 0x00000001;
    public static final int USB_MODE_ADB = 0x00000002;
    public static final int USB_MODE_RNDIS = 0x00000004;
    public static final int USB_MODE_MTP = 0x00000008;
    public static final int USB_MODE_PTP = 0x00000010;
    public static final int USB_MODE_AUDIO_SOURCE = 0x00000020;
    public static final int USB_MODE_MIDI = 0x00000040;
    public static final int USB_MODE_ACCESSORY = 0x00000080;

    public static final String USB_FUNCTION_NONE = "none";
    public static final String USB_FUNCTION_ADB = "adb";
    public static final String USB_FUNCTION_RNDIS = "rndis";
    public static final String USB_FUNCTION_MTP = "mtp";
    public static final String USB_FUNCTION_PTP = "ptp";
    public static final String USB_FUNCTION_AUDIO_SOURCE = "audio_source";
    public static final String USB_FUNCTION_MIDI = "midi";
    public static final String USB_FUNCTION_ACCESSORY = "accessory";

    public static final Set<Integer> USB_MODE_INDEX_SET = new HashSet<Integer>();
    static {
        USB_MODE_INDEX_SET.add(USB_MODE_NONE);
        USB_MODE_INDEX_SET.add(USB_MODE_ADB);
        USB_MODE_INDEX_SET.add(USB_MODE_RNDIS);
        USB_MODE_INDEX_SET.add(USB_MODE_MTP);
        USB_MODE_INDEX_SET.add(USB_MODE_PTP);
        USB_MODE_INDEX_SET.add(USB_MODE_AUDIO_SOURCE);
        USB_MODE_INDEX_SET.add(USB_MODE_MIDI);
        USB_MODE_INDEX_SET.add(USB_MODE_ACCESSORY);
    }

    public static final SparseArray<String> USB_MODE_KEY_SET = new SparseArray<String>();
    static {
        USB_MODE_KEY_SET.put(USB_MODE_NONE, UsbManager.USB_FUNCTION_NONE);
        USB_MODE_KEY_SET.put(USB_MODE_ADB, UsbManager.USB_FUNCTION_ADB);
        USB_MODE_KEY_SET.put(USB_MODE_RNDIS, UsbManager.USB_FUNCTION_RNDIS);
        USB_MODE_KEY_SET.put(USB_MODE_MTP, UsbManager.USB_FUNCTION_MTP);
        USB_MODE_KEY_SET.put(USB_MODE_PTP, UsbManager.USB_FUNCTION_PTP);
        USB_MODE_KEY_SET.put(USB_MODE_AUDIO_SOURCE, UsbManager.USB_FUNCTION_AUDIO_SOURCE);
        USB_MODE_KEY_SET.put(USB_MODE_MIDI, UsbManager.USB_FUNCTION_MIDI);
        USB_MODE_KEY_SET.put(USB_MODE_ACCESSORY, UsbManager.USB_FUNCTION_ACCESSORY);
    }

    /**
     * @param intent
     *            Intent
     * @return int
     */
    public static int getCurrentUsbMode(Intent intent) {
        int result = 0;
        for (int index : USB_MODE_INDEX_SET) {
            boolean value = intent.getBooleanExtra(USB_MODE_KEY_SET.get(index), false);
            Utils.logv(TAG + "/Utils", USB_MODE_KEY_SET.get(index) + "=" + value);
            if (value) {
                result |= index;
            }
        }

        return result;
    }

    /**
     * Get current log path type, one of phone, internal SD card and external SD card.
     *
     * @return String
     */
    public static String getCurrentLogPathType() {
        return MyApplication.getInstance().getDefaultSharedPreferences().getString(
                SettingsActivity.KEY_ADVANCED_LOG_STORAGE_LOCATION, LOG_PATH_TYPE_DEFAULT);
    }

    /**
     * @return debuglogger folder in storage path
     */
    public static String geMtkLogPath() {
        return getCurrentLogPath() + LOG_PATH_PARENT;
    }

    private static StorageManager sStorageManager = null;

    /**
     * @return String
     */
    public static String getCurrentLogPath() {
        String logPathType = getCurrentLogPathType();
        return getLogPath(logPathType);
    }

    /**
     * check if volume is USB OTG.
     *
     * @return boolean
     */
    private static boolean isUSBOTG(VolumeInfo volumeInfo) {
        if (volumeInfo == null) {
            return false;
        }
        DiskInfo diskInfo = volumeInfo.getDisk();
        if (diskInfo == null) {
            return false;
        }

        String diskID = diskInfo.getId();
        if (diskID != null) {
            // for usb otg, the disk id same as disk:8:x
            String[] idSplit = diskID.split(":");
            if (idSplit != null && idSplit.length == 2) {
                if (idSplit[1].startsWith("8,")) {
                    Utils.logd(TAG, "this is a usb otg");
                    return true;
                }
            }
        }
        return false;
    }

    /**
     * @return String
     */
    public static String getInternalSdPath() {
        Utils.logd(TAG + "/Utils", "-->getDefalueInternalSdPath()");
        String internalPath = null;
        StorageManager storageManager = (StorageManager) MyApplication.getInstance()
                .getApplicationContext().getSystemService(Context.STORAGE_SERVICE);
        StorageVolume[] volumes = storageManager.getVolumeList();
        for (StorageVolume volume : volumes) {
            String volumePathStr = volume.getPath();
            if (Environment.MEDIA_MOUNTED.equalsIgnoreCase(volume.getState())) {
                Utils.logd(TAG + "/Utils", volumePathStr + " is mounted!");
                VolumeInfo volumeInfo = storageManager.findVolumeById(volume.getId());
                if (isUSBOTG(volumeInfo)) {
                    continue;
                }
                if (volume.isEmulated()) {
                    String viId = volumeInfo.getId();
                    Utils.logd(TAG + "/Utils", "Is emulated and volumeInfo.getId() : " + viId);
                    // If external sd card, the viId will be like
                    // "emulated:179,130"
                    if (viId.equalsIgnoreCase("emulated")) {
                        internalPath = volumePathStr;
                        break;
                    }
                } else {
                    DiskInfo diskInfo = volumeInfo.getDisk();
                    if (diskInfo == null) {
                        continue;
                    }
                    if (diskInfo.isSd()) {
                        continue;
                    }
                    String diId = diskInfo.getId();
                    String emmcSupport = SystemProperties.get("ro.vendor.mtk_emmc_support", "");
                    File bootTypeFile = new File("/sys/class/BOOT/BOOT/boot/boot_type");
                    List<String> bootTypeList = getLogFolderFromFileTree(bootTypeFile);
                    if (bootTypeList != null && bootTypeList.size() >= 1) {
                        emmcSupport = bootTypeList.get(0);
                    }
                    Utils.logi(TAG + "/Utils", "Is not emulated and diskInfo.getId() : " + diId);
                    // If is emmcSupport and is internal sd card, the diId will be like "disk:179,0"
                    // if is not emmcSupport and is internal sd card, the diId will be like
                    // "disk:7,1"
                    if ((emmcSupport.equals("1") && diId.equalsIgnoreCase("disk:179,0"))
                            || (!emmcSupport.equals("1") && diId.equalsIgnoreCase("disk:7,1"))) {
                        internalPath = volumePathStr;
                        break;
                    }
                }
            } else {
                Utils.logd(TAG + "/Utils", volumePathStr + " is not mounted!");
            }
        }
        Utils.logd(TAG + "/Utils", "<--getDefalueInternalSdPath() = " + internalPath);
        return internalPath;
    }

    /**
     * @return String
     */
    public static String getExternalSdPath() {
        Utils.logd(TAG + "/Utils", "-->getDefaultExternalSdPath()");
        String externalPath = null;
        StorageManager storageManager = (StorageManager) MyApplication.getInstance()
                .getApplicationContext().getSystemService(Context.STORAGE_SERVICE);
        StorageVolume[] volumes = storageManager.getVolumeList();
        for (StorageVolume volume : volumes) {
            String volumePathStr = volume.getPath();
            if (Environment.MEDIA_MOUNTED.equalsIgnoreCase(volume.getState())) {
                Utils.logd(TAG + "/Utils", volumePathStr + " is mounted!");
                VolumeInfo volumeInfo = storageManager.findVolumeById(volume.getId());
                if (isUSBOTG(volumeInfo)) {
                    continue;
                }
                if (volume.isEmulated()) {
                    String viId = volumeInfo.getId();
                    Utils.logd(TAG + "/Utils", "Is emulated and volumeInfo.getId() : " + viId);
                    // If external sd card, the viId will be like
                    // "emulated:179,130"
                    if (!viId.equalsIgnoreCase("emulated")) {
                        externalPath = volumePathStr;
                        break;
                    }
                } else {
                    DiskInfo diskInfo = volumeInfo.getDisk();
                    if (diskInfo == null) {
                        continue;
                    }
                    if (diskInfo.isSd()) {
                        externalPath = volumePathStr;
                        break;
                    }
                    String diId = diskInfo.getId();
                    String emmcSupport = SystemProperties.get("ro.vendor.mtk_emmc_support", "");
                    File bootTypeFile = new File("/sys/class/BOOT/BOOT/boot/boot_type");
                    List<String> bootTypeList = getLogFolderFromFileTree(bootTypeFile);
                    if (bootTypeList != null && bootTypeList.size() >= 1) {
                        emmcSupport = bootTypeList.get(0);
                    }
                    Utils.logi(TAG + "/Utils", "Is not emulated and diskInfo.getId() = " + diId
                            + ", emmcSupport = " + emmcSupport);
                    // If is emmcSupport and is internal sd card, the diId will be like "disk:179,0"
                    // if is not emmcSupport and is internal sd card, the diId will be like
                    // "disk:7,1"
                    if ((emmcSupport.equals("1") && diId.equalsIgnoreCase("disk:179,0"))
                            || (!emmcSupport.equals("1") && diId.equalsIgnoreCase("disk:7,1"))) {
                        continue;
                    } else {
                        externalPath = volumePathStr;
                        break;
                    }
                }
            } else {
                Utils.logd(TAG + "/Utils", volumePathStr + " is not mounted!");
            }
        }
        Utils.logd(TAG + "/Utils", "<--getDefaultExternalSdPath() = " + externalPath);
        return externalPath;
    }

    /**
     * Get detail log path string according to given log path type.
     *
     * @param logPathType
     *            The type of log path
     * @return String
     *
     */
    public static String getLogPath(String logPathType) {
        if (sStorageManager == null) {
            sStorageManager = (StorageManager) MyApplication.getInstance().getApplicationContext()
                    .getSystemService(Context.STORAGE_SERVICE);
        }

        String logPathStr = null;
        if (LOG_PATH_TYPE_SYSTEM_DATA.equals(logPathType)) {
            logPathStr = LOG_PATH_SYSTEM_DATA;
        } else if (LOG_PATH_TYPE_DEVICE_STORAGE.equals(logPathType)) {
            logPathStr = getInternalSdPath();
        } else if (LOG_PATH_TYPE_PORTABLE_STORAGE.equals(logPathType)) {
            logPathStr = getExternalSdPath();
        } else {
            Utils.loge(TAG + "/Utils", "Unsupported log path type: " + logPathType);
        }

        if (logPathStr == null) {
            Utils.loge(TAG + "/Utils", "Fail to get detail log path string for type: " + logPathType
                    + ", return empty to avoid NullPointerException.");
            logPathStr = "";
        }

        File logParent = new File(logPathStr);
        try {
            if (logParent.exists()) {
                logPathStr = logParent.getCanonicalPath();
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
        if (!isDeviceOwner()) {
            Utils.logi(TAG, "Change sdcard/emulated/number to sdcard/emulated/0"
                    + " for not device owner." + " logPathStr = " + logPathStr);
            String[] logPathStrs = logPathStr.split("/");
            if (logPathStrs != null && logPathStrs.length >= 2
                    && logPathStrs[logPathStrs.length - 2].equalsIgnoreCase("emulated")
                    && !logPathStrs[logPathStrs.length - 1].equalsIgnoreCase("0")) {
                logPathStr = logPathStr.substring(0, logPathStr.lastIndexOf("/") + 1) + "0";
            }
        }
        Utils.logv(TAG, "<--getLogPath(), type=" + logPathType + ", string=" + logPathStr);

        return logPathStr;
    }

    /**
     * Get file or folder size of input filePath.
     *
     * @param filePath
     *            the path of file
     * @return filePath size
     */
    public static long getFileSize(String filePath) {
        long size = 0;
        if (filePath == null) {
            return 0;
        }
        File fileRoot = new File(filePath);
        if (fileRoot == null || !fileRoot.exists()) {
            return 0;
        }
        if (!fileRoot.isDirectory()) {
            size = fileRoot.length();
        } else {
            File[] files = fileRoot.listFiles();
            // why get a null here ?? maybe caused by permission denied
            if (files == null || files.length == 0) {
                Utils.logv(TAG, "Loop folder [" + filePath + "] get a null/empty list");
                return 0;
            }
            for (File file : files) {
                if (file == null) {
                    continue;
                }
                size += getFileSize(file.getAbsolutePath());
            }
        }
        return size;
    }

    /**
     * Delete the given folder or file.
     *
     * @param file
     *            which will be deleted
     */
    public static void deleteFile(File file) {
        if (file == null || !file.exists()) {
            return;
        }
        try {
            // Do a short sleep to avoid always lock sd card and ANR happened
            Thread.sleep(10);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        LogFileManager.delete(file);
    }

    /**
     * Get the remaining storage size in the given file path.
     *
     * @param storagePath
     *            String
     * @return remaining size in MB
     */
    public static int getAvailableStorageSize(String storagePath) {
        StatFs stat;
        int retryNum = 1;
        while (retryNum <= 3) {
            try {
                stat = new StatFs(storagePath);
                long blockSize = stat.getBlockSizeLong();
                long availableBlocks = stat.getAvailableBlocksLong();
                int availableSize = (int) (availableBlocks * blockSize / (1024 * 1024));
                Utils.logd(TAG, "-->getAvailableStorageSize(), path=" + storagePath + ", size="
                        + availableSize + "MB");
                return availableSize;
            } catch (IllegalArgumentException e) {
                Utils.logw(TAG, "Fail to get storage info from [" + storagePath
                        + "] by StatFs, try again(index=" + retryNum + ").");
                try {
                    Thread.sleep(200);
                } catch (InterruptedException e1) {
                    e1.printStackTrace();
                }
            }
            retryNum++;
        }
        Utils.loge(TAG, "-->getAvailableStorageSize(), fail to get it by StatFs,"
                + " unknown exception happen.");
        return 0;
    }

    /**
     * @return boolean
     */
    public static boolean isSupportC2KModem() {
        return isTypeMDEnable(C2KLOGGER_INDEX);
    }

    /**
     * @return boolean
     */
    public static boolean isMTKSvlteSupport() {
        // ro.vendor.mtk_c2k_lte_mode 0:None/1:SVLTE/2:SRLTE
        boolean isMTKSvlteSupport =
                !SystemProperties.get("ro.vendor.mtk_c2k_lte_mode", "0").equals("0");
        Utils.logi(TAG, "isMTKSvlteSupport ? " + isMTKSvlteSupport);
        return isMTKSvlteSupport;
    }

    /**
     * @return boolean
     */
    public static boolean isDenaliMd3Solution() {
        return isSupportC2KModem()
                && SystemProperties.get("ro.vendor.mtk_eccci_c2k", "0").equals("0");
    }

    /**
     * @return boolean
     */
    public static boolean isAutoTest() {
        // MTK method
        boolean isMonkeyRunning = SystemProperties.getBoolean(Utils.PROP_MONKEY, false);
        // Andriod default API
        boolean isUserAMonkey = ActivityManager.isUserAMonkey();
        Utils.logi(TAG, "isAutoTest()-> Monkey running flag is " + isMonkeyRunning
                + ", isUserAMonkey=" + isUserAMonkey);
        return (isMonkeyRunning || isUserAMonkey);
    }

    /**
     * Service may run in different stages, like starting log, stopping log, or polling modem log.
     * Use this broadcast to notify UI about the latest stage change event, to show or dismiss
     * progress dialog, if any.
     */
    public static final String ACTION_RUNNING_STAGE_CHANGED = "";
    /**
     * Indicate which stage have began.
     */
    public static final String EXTRA_RUNNING_STAGE_CHANGE_EVENT =
            "com.debug.loggerui.stage_event";

    public static final String EXTRA_RUNNING_STAGE_CHANGE_VALUE = "stage_value";
    /**
     * Start log now, should show a progress dialog of starting log in UI. When receive log state
     * change broadcast, which means start finished, dismiss this dialog
     */
    public static final int STAGE_EVENT_START_LOG = 1;
    /**
     * Stop log now, should show a progress dialog of stopping log in UI. When receive log state
     * change broadcast, which means stop finished, dismiss this dialog
     */
    public static final int STAGE_EVENT_STOP_LOG = 2;
    /**
     * Start memory dump for modem log.
     */
    public static final int STAGE_EVENT_START_POLLING = 3;
    /**
     * Memory dump finished for modem log.
     */
    public static final int STAGE_EVENT_POLLING_DONE = 4;
    /**
     * Used to query current running stage from UI.
     */
    public static final int RUNNING_STAGE_IDLE = 0;
    public static final int RUNNING_STAGE_STARTING_LOG = 1;
    public static final int RUNNING_STAGE_STOPPING_LOG = 2;
    public static final int RUNNING_STAGE_RESTARTING_LOG = 3;
    public static final int RUNNING_STAGE_POLLING_LOG = 4;
    public static final int RUNNING_STAGE_FLUSHING_LOG = 5;

    // Add for dual talk begin
    /**
     * Flag for whether log is running or not. 0 for stop, 1 for running For modem log, 1 for MD1
     * running, 2 for MD2 running, 3 for both MD1 and MD2 are running
     */
    public static final int LOG_RUNNING_STATUS_UNKNOWN = -1;
    public static final int LOG_RUNNING_STATUS_STOPPED = 0;
    public static final int LOG_RUNNING_STATUS_MD1 = 1;
    public static final int LOG_RUNNING_STATUS_MD2 = 2;
    public static final int LOG_RUNNING_STATUS_MD1_MD2 = 3;
    // Add for dual talk end

    /**
     * If start all logs at the same time, they may conflict when creating debuglogger folder, so
     * add a duration between each log instance.
     */
    public static final int DURATION_START_EACH_LOG = 300;

    /**
     * After initializing a log instance from main thread, it may take some time to be ready, so
     * wait at most this time in main thread, unit: ms.
     */
    public static final int DURATION_WAIT_LOG_INSTANCE_READY = 500;

    /**
     * Means this is a system server crash issue, which will only cause android restart, but native
     * layer will still keep its status.
     */
    public static final String FLAG_REBOOT_ISSUE_SYSTEM_CARSH = "2:";

    // Add for modem reset event
    /**
     * Since there maybe more than one modem log instance, once reset event happen, use this flag to
     * indicate which instance trigger this.
     */
    public static final String EXTRA_RESET_MD_INDEX = "modem_index";

    /**
     * return void.
     */
    public static void updateLogFilesInMediaProvider() {
        Utils.logv(TAG + "/Utils", "-->updateLogFilesInMediaProvider");
        new Thread() {
            @Override
            public void run() {
                int timeout = 0;
                while (timeout < 15000) {
                    if (!LogControllerUtils.isAnyControlledLogRunning()) {
                        break;
                    }
                    timeout += 1000;
                    try {
                        Thread.sleep(1000);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
                updateLogFilesInMediaProviderSync(
                        MyApplication.getInstance().getApplicationContext());
            };
        }.start();
    }

    synchronized private static void updateLogFilesInMediaProviderSync(Context context) {
        Utils.logv(TAG + "/Utils", "-->updateLogFilesInMediaProviderSync");
        String logRootPath = getCurrentLogPath() + "/debuglogger";
        if (!new File(logRootPath).exists()) {
            loge(TAG, "debuglogger folder not exists, no need to update.");
            return;
        }
        File mtklogFileTree = new File(logRootPath + "/" + LOG_TREE_FILE);
        List<String> oldFileList = getLogFolderFromFileTree(mtklogFileTree);

        List<String> newFileList = new ArrayList<String>();
        getFilesFromFolder(new File(logRootPath), newFileList, "aee_exp/temp");
        setLogFolderToFileTree(newFileList, mtklogFileTree);

        List<String> updateFileList = new ArrayList<String>();
        for (String oldFile : oldFileList) {
            if (!newFileList.contains(oldFile)) {
                updateFileList.add(oldFile);
            }
        }
        for (String newFile : newFileList) {
            if (!oldFileList.contains(newFile)) {
                updateFileList.add(newFile);
            }
        }
        updateLogFilesInMediaProviderSync(context, updateFileList);
    }

    private static int sUpdateMediaFileResult = 0;

    synchronized private static void updateLogFilesInMediaProviderSync(Context context,
            List<String> fileList) {
        if (fileList.size() == 0) {
            Utils.logv(TAG + "/Utils", "fileList size is 0, no need update MTP db.");
            return;
        }
        sUpdateMediaFileResult = 0;
        String[] fileArray = new String[fileList.size()];
        fileList.toArray(fileArray);
        Utils.logi(TAG + "/Utils",
                "-->updateLogFilesInMediaProviderSync(), pathArray.length=" + fileArray.length);
        try {
            MediaScannerConnection.scanFile(context, fileArray, null,
                    new OnScanCompletedListener() {
                        @Override
                        public void onScanCompleted(String path, Uri uri) {
                            sUpdateMediaFileResult++;
                        }
                    });
        } catch (AndroidRuntimeException are) {
            Utils.loge(TAG + "/Utils", "Something exception happend about"
                    + " MediaScannerConnection.scanFile, just return.");
            return;
        }
        int time = 0;
        while (sUpdateMediaFileResult < fileList.size()) {
            try {
                Thread.sleep(10);
                time += 10;
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            if (time >= 1000 * 300) {
                Utils.logw(TAG + "/Utils", "updateLogFilesInMediaProviderSync() time out!");
                break;
            }
        }
        Utils.logv(TAG + "/Utils", "<--updateLogFilesInMediaProviderSync() Done.");
    }

    /**
     * @param parent
     *            File
     * @param fileList
     *            List<File>
     * @param filterPath
     *            String
     */
    public static void getFilesFromFolder(File parent, List<String> fileList, String filterPath) {
        if (parent == null) {
            Utils.logd(TAG + "/Utils", "-->getFilesFromFolder() parent = null!");
            return;
        }
        if (parent.exists()) {
            if (parent.isDirectory()) {
                if (filterPath != null && !"".equals(filterPath)
                        && parent.getPath().endsWith(filterPath)) {
                    return;
                }
                File[] files = parent.listFiles();
                if (files != null) {
                    int counter = 0;
                    for (File file : files) {
                        counter++;
                        if (counter % 100 == 0) {
                            Utils.logd(TAG + "/Utils", "The counter of list file in "
                                    + parent.getAbsolutePath() + " is " + counter + ".");
                        }
                        getFilesFromFolder(file, fileList, filterPath);
                    }
                }
            }
            fileList.add(parent.getAbsolutePath());
        }
    }

    /**
     * @param context
     *            Context
     * @param className
     *            String
     * @return boolean
     */
    public static boolean isServiceRunning(Context context, String className) {
        boolean isRunning = false;
        ActivityManager activityManager =
                (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
        List<ActivityManager.RunningServiceInfo> serviceList = null;
        try {
            serviceList = activityManager.getRunningServices(100);
        } catch (SecurityException re) {
            return false;
        }

        if (!(serviceList.size() > 0)) {
            return false;
        }

        for (int i = 0; i < serviceList.size(); i++) {
            if (serviceList.get(i).service.getClassName().equals(className)) {
                isRunning = true;
                break;
            }
        }
        Utils.logi(TAG + "/Utils",
                "-->isServiceRunning() className = " + className + ", isRunning = " + isRunning);
        return isRunning;
    }

    private static boolean sIsAlreadySendShutDowntoNT = false;

    public static void setAlreadySendShutDown(boolean isInit) {
        sIsAlreadySendShutDowntoNT = isInit;
    }

    public static boolean getAlreadySendShutDown() {
        return sIsAlreadySendShutDowntoNT;
    }

    /**
     * @return boolean
     */
    public static boolean isMultiLogFeatureOpen() {
        // return SystemProperties.get("ro.multi_log_feature", "0").equals("1");
        return false;
    }

    /**
     * @return String time format with:yyyy_MMdd_HHmmss.
     */
    public static String getBootTimeString() {
        Calendar calendar = Calendar.getInstance();
        calendar.setTimeInMillis(getRebootTime());
        String bootTimeStr = new SimpleDateFormat("yyyy_MMdd_HHmmss").format(calendar.getTime());
        Utils.logi(TAG, "bootTimeStr = " + bootTimeStr);
        return bootTimeStr;
    }

    /*
     * Log folder name will match with pattern, use this to get log folder created time
     */
    private static final Pattern mLogFolderNamePattern =
            Pattern.compile("\\w*_(\\d{4}_\\d{4}_\\d{6})(\\w)*");

    /**
     * @param fileTree
     *            File
     * @param isFromReboot
     *            boolean
     * @return String
     */
    public static File getLogFolderFromFileTree(File fileTree, boolean isFromReboot) {
        Utils.logd(TAG + "/Utils", "-->getPathFromFileTree() isFromReboot = " + isFromReboot);
        String bootTimeStr = "";
        if (isFromReboot) {
            bootTimeStr = getBootTimeString();
        }
        String logFolderPath = getLogFolderFromFileTree(fileTree, bootTimeStr);
        File logFolder = null;
        if (logFolderPath != null && !logFolderPath.isEmpty()) {
            logFolder = new File(logFolderPath);
        }
        return logFolder;
    }

    /**
     * @param fileTree
     *            File
     * @param beforeTime
     *            String
     * @return String
     */
    public static String getLogFolderFromFileTree(File fileTree, String beforeTime) {
        Utils.logd(TAG + "/Utils", "-->getPathFromFileTree() beforeTime = " + beforeTime);
        String logFolderPath = null;
        File beforeTimelogFolder = null;
        File afterTimelogFolder = null;
        List<String> fileList = getLogFolderFromFileTree(fileTree);
        if (fileList != null && fileList.size() > 0) {
            afterTimelogFolder = new File(fileList.get(fileList.size() - 1));
            if (beforeTime == null || beforeTime.isEmpty()) {
                beforeTimelogFolder = new File(fileList.get(fileList.size() - 1));
            } else {
                for (int i = fileList.size() - 1; i >= 0; i--) {
                    File currentFile = new File(fileList.get(i));
                    String folderName = currentFile.getName();
                    Matcher matcher = mLogFolderNamePattern.matcher(folderName);
                    if (matcher.matches()) {
                        String createTimeStr = matcher.group(1);
                        Utils.logd(TAG,
                                "createTimeStr=" + createTimeStr + ", beforeTime=" + beforeTime);
                        if (createTimeStr.compareTo(beforeTime) >= 0) {
                            Utils.logw(TAG,
                                    folderName + ", this folder is created after setting time,"
                                            + " should not select this one.");
                            afterTimelogFolder = currentFile;
                            continue;
                        } else {
                            beforeTimelogFolder = currentFile;
                            break;
                        }
                    } else {
                        Utils.logw(TAG, "Unknown log folder name: " + folderName);
                    }
                }
            }
        }
        if (beforeTimelogFolder != null) {
            logFolderPath = beforeTimelogFolder.getAbsolutePath();
        }
        if (isTimeAfterFileModify(beforeTimelogFolder, beforeTime)) {
            if (afterTimelogFolder != null
                    && !afterTimelogFolder.getAbsolutePath().equals(logFolderPath)) {
                logFolderPath += ";" + afterTimelogFolder.getAbsolutePath();
                Utils.logi(TAG, "checkFileModifyTime not before set time, add next folder");
            }
        }
        Utils.logi(TAG + "/Utils", "<--getPathFromFileTree() logFolder = " + logFolderPath
                + ", beforeTime = " + beforeTime);
        return logFolderPath;
    }

    /**
     * @param fileTree
     *            File
     * @param currentFolder
     *            String
     * @return String
     */
    public static String getNextLogFolderFromFileTree(File fileTree, String currentFolder) {
        Utils.logd(TAG + "/Utils",
                "-->getNextLogFolderFromFileTree() " + "currentFolder = " + currentFolder);
        String logFolderPath = null;
        int currentFolderIndex = -1;
        List<String> fileList = getLogFolderFromFileTree(fileTree);
        if (fileList != null && fileList.size() > 0) {
            if (currentFolder == null || currentFolder.isEmpty()) {
                return logFolderPath;
            }
            for (int i = fileList.size() - 1; i >= 0; i--) {
                String folderName = fileList.get(i);
                if (currentFolder.equals(folderName)) {
                    currentFolderIndex = i;
                    break;
                }
            }
            if (currentFolderIndex == -1) {
                return logFolderPath;
            }
            for (int i = currentFolderIndex + 1; i < fileList.size(); i++) {
                String folderName = fileList.get(i);
                File nextFile = new File(folderName);
                if (logFolderPath == null) {
                    logFolderPath = nextFile.getAbsolutePath();
                } else {
                    logFolderPath += ";" + nextFile.getAbsolutePath();
                }
            }
        }

        Utils.logi(TAG + "/Utils", "<--getNextLogFolderFromFileTree() logFolder = " + logFolderPath
                + ", currentFolder = " + currentFolder);
        return logFolderPath;
    }

    private static boolean isTimeAfterFileModify(File beforeTimelogFolder, String beforeTime) {
        if (beforeTime == null || beforeTime.isEmpty() || beforeTimelogFolder == null
                || !beforeTimelogFolder.exists()) {
            Utils.logd(TAG, "isTimeAfterFileModify = false" + " for file not exist"
                    + ", file = " + beforeTimelogFolder
                    + ", beforTime = " + beforeTime);
            return false;
        }
        boolean result = false;
        String modifyTimeStr = "";
        long modifiedTime = getFolderLastModifyTime(beforeTimelogFolder);
        if (modifiedTime != 0) {
            modifyTimeStr = TagLogUtils.translateTime2(modifiedTime);
            if (beforeTime.compareTo(modifyTimeStr) >= 0) {
                result = true;
            }
        }
        Utils.logd(TAG, "checkFileModifyTime = " + result + ", file = " + beforeTimelogFolder
                + ", beforTime = " + beforeTime + ", modifyTime = " + modifyTimeStr);
        return result;
    }

    /**
     * @param file
     *            File
     * @return long
     */
    public static long getFolderLastModifyTime(File file) {
        long result = 0;
        if (file == null || !file.exists()) {
            Utils.logd(TAG, "Given file not exist.");
            return result;
        }
        if (file.isFile()) {
            result = file.lastModified();
        } else {
            File[] fileList = file.listFiles();
            if (fileList == null || fileList.length == 0) {
                Utils.loge(TAG, "No sub files in folder:" + file.getAbsolutePath());
                return result;
            }
            for (File subFile : fileList) {
                long lastModifiedTime = 0;
                if (subFile.isFile()) {
                    lastModifiedTime = subFile.lastModified();
                } else {
                    lastModifiedTime = getFolderLastModifyTime(subFile);
                }
                if (lastModifiedTime > result) {
                    result = lastModifiedTime;
                }
            }
        }
        return result;
    }

    /**
     * @param fileTree
     *            File
     * @return List<String>
     */
    public static List<String> getLogFolderFromFileTree(File fileTree) {
        Utils.logd(TAG + "/Utils", "-->getPathFromFileTree()");
        List<String> fileList = new ArrayList<String>();
        if (fileTree == null || !fileTree.exists()) {
            Utils.logw(TAG + "/Utils", "fileTree is null or does not exist!");
            return fileList;
        }
        FileReader fr = null;
        BufferedReader br = null;
        try {
            fr = new FileReader(fileTree);
            br = new BufferedReader(fr);
            String readLineStr = "";
            while ((readLineStr = br.readLine()) != null) {
                if (readLineStr.trim().length() > 0) {
                    fileList.add(readLineStr.trim());
                }
            }
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            try {
                if (br != null) {
                    br.close();
                }
                if (fr != null) {
                    fr.close();
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        Utils.logi(TAG + "/Utils", "<--getPathFromFileTree() " + "fileTree = "
                + fileTree.getAbsolutePath() + ", fileList.size = " + fileList.size());
        return fileList;
    }

    /**
     * @param fileList
     *            List<String>
     * @param fileTree
     *            File
     */
    public static void setLogFolderToFileTree(List<String> fileList, File fileTree) {
        Utils.logd(TAG + "/Utils", "-->setLogFolderToFileTree()");
        if (fileList == null || fileList.size() == 0) {
            Utils.logw(TAG + "/Utils", "fileList is null!");
            return;
        }
        if (fileTree == null) {
            Utils.logw(TAG + "/Utils", "fileTree is null!");
            return;
        }
        Utils.logd(TAG + "/Utils", " fileList.size = " + fileList.size() + " fileTree = "
                + fileTree.getAbsolutePath());
        if (fileTree.exists()) {
            LogFileManager.delete(fileTree);
        }
        LogFileManager.createNewFile(fileTree);
        FileWriter fw = null;
        try {
            fw = new FileWriter(fileTree);
            for (String filePath : fileList) {
                fw.write(filePath + "\n");
            }
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            try {
                if (fw != null) {
                    fw.close();
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        Utils.logd(TAG + "/Utils",
                "<--setLogFolderToFileTree() fileList.size = " + fileList.size());
    }

    /**
     * @return Is zip all log files to only one file ? true : false.
     */
    public static boolean isTaglogToOneFile() {
        boolean isTaglog2OneFile = MyApplication.getInstance()
                .getSharedPreferences(Utils.CONFIG_FILE_NAME, Context.MODE_PRIVATE)
                .getBoolean(KEY_PREFERENCE_TAGLOG_ZIP2ONE, false);
        if (isReleaseToCustomer1()) {
            isTaglog2OneFile = true;
        }
        return isTaglog2OneFile;
    }

    /**
     * @param isTaglog2OneFile
     *            boolean
     */
    public static void setTaglogToOnFile(boolean isTaglog2OneFile) {
        MyApplication.getInstance()
                .getSharedPreferences(Utils.CONFIG_FILE_NAME, Context.MODE_PRIVATE).edit()
                .putBoolean(KEY_PREFERENCE_TAGLOG_ZIP2ONE, isTaglog2OneFile).apply();
    }

    public static final String TAGLOG_ZIPFILEPATH_DEFAULT_VALUE = "debuglogger/taglog";

    /**
     * @return String
     */
    public static String getZipFilePath() {
        String zipFilePath = MyApplication.getInstance()
                .getSharedPreferences(Utils.CONFIG_FILE_NAME, Context.MODE_PRIVATE)
                .getString(KEY_PREFERENCE_TAGLOG_ZIPFILEPATH, TAGLOG_ZIPFILEPATH_DEFAULT_VALUE);
        if (isReleaseToCustomer1()) {
            zipFilePath = "dumpfiles";
        }
        Utils.logd(TAG + "/Utils", "<--getZipFilePath, zipFilePath=" + zipFilePath);
        return zipFilePath;
    }

    /**
     * @param zipFilePath
     *            String
     */
    public static void setZipFilePath(String zipFilePath) {
        Utils.logd(TAG + "/Utils", "-->setZipFilePath, zipFilePath=" + zipFilePath);
        if (zipFilePath != null) {
            MyApplication.getInstance()
                    .getSharedPreferences(Utils.CONFIG_FILE_NAME, Context.MODE_PRIVATE).edit()
                    .putString(KEY_PREFERENCE_TAGLOG_ZIPFILEPATH, zipFilePath).apply();
        }
    }

    /**
     * @return boolean
     */
    public static boolean isReleaseToCustomer1() {
        return SystemProperties.get(KEY_SYSTEM_PROPERTY_LOGGERUI_RELEASE_VERSION, "0").equals("1");
    }

    /**
     * @param intent
     *            Intent
     */
    public static void sendBroadCast(Intent intent) {
        if (isDeviceOwner()) {
            try {
                MyApplication.getInstance().getApplicationContext().sendBroadcast(intent,
                        "android.permission.DUMP");
            } catch (SecurityException se) {
                loge(TAG, "Some SecurityException happened, no permission to send broadcast!");
            }
        } else {
            logw(TAG, "Is not device owner, no permission to send broadcast!");
        }
    }

    /**
     * @return boolean
     */
    public static boolean isTaglogEnable() {
        boolean isTagLogEnabled = false;
        if (!Utils.isDeviceOwner()) {
            Utils.logi(TAG, "It is not device owner, set TagLog as disabled!");
            return false;
        }
        if (MyApplication.getInstance() == null) {
            return false;
        }
        if (!Utils.BUILD_TYPE.equals("eng")) {
            Utils.logd(TAG, "Build type is not eng");
            isTagLogEnabled = MyApplication.getInstance().getSharedPreferences()
                    .getBoolean(Utils.TAG_LOG_ENABLE, false);
        } else {
            Utils.logd(TAG, "Build type is eng");
            isTagLogEnabled = MyApplication.getInstance().getSharedPreferences()
                    .getBoolean(Utils.TAG_LOG_ENABLE, true);
        }
        Utils.logi(TAG, "isTaglogEnable ? " + isTagLogEnabled);
        return isTagLogEnabled;
    }

    /**
     * @param sourceFilePath
     *            String
     * @param targetFilePath
     *            String
     * @return boolean
     */
    public static boolean doCopy(String sourceFilePath, String targetFilePath) {
        Utils.logi(TAG, "-->doCopy() from " + sourceFilePath + " to " + targetFilePath);
        File sourceFile = new File(sourceFilePath);
        if (null == sourceFile || !sourceFile.exists()) {
            Utils.logw(TAG,
                    "The sourceFilePath = " + sourceFilePath + " is not existes, do copy failed!");
            return false;
        }
        // Get all files and sub directories under the current directory
        File[] files = sourceFile.listFiles();
        if (null == files) {
            // Current file is not a directory
            String tagLogPath = sourceFile.getAbsolutePath();
            return copyFile(tagLogPath, targetFilePath);
        } else {
            // Current file is a directory
            File targetFile = new File(targetFilePath);
            if (!targetFile.exists()) {
                LogFileManager.mkdirs(targetFile);
            }
            for (File subFile : files) {
                doCopy(subFile.getAbsolutePath(),
                        targetFilePath + File.separator + subFile.getName());
            }
        }
        return true;
    }

    private static final int COPY_BUFFER_SIZE = 1024;

    private static boolean copyFile(String sourceFilePath, String targetFilePath) {
        Utils.logi(TAG, "-->copyFile() from " + sourceFilePath + " to " + targetFilePath);
        File sourceFile = new File(sourceFilePath);
        if (!sourceFile.exists()) {
            Utils.logw(TAG,
                    "The sourceFilePath = " + sourceFilePath + " is not existes, do copy failed!");
            return false;
        }

        File targetFile = new File(targetFilePath);
        if (targetFile.exists()) {
            LogFileManager.delete(targetFile);
        }

        FileInputStream fis = null;
        FileOutputStream fos = null;
        try {
            File parentFile = targetFile.getParentFile();
            if (!parentFile.exists()) {
                LogFileManager.mkdirs(parentFile);
            }
            LogFileManager.createNewFile(targetFile);
            fis = new FileInputStream(sourceFile);
            fos = LogFileManager.getFileOutputStream(targetFile);
            if (fos == null) {
                return false;
            }
            byte[] temp = new byte[COPY_BUFFER_SIZE];
            int len;
            while ((len = fis.read(temp)) != -1) {
                fos.write(temp, 0, len);
            }
            fos.flush();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
            return false;
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        } finally {
            try {
                if (fos != null) {
                    fos.close();
                }
                if (fis != null) {
                    fis.close();
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        return true;
    }

    /**
     * @return long
     */
    public static long getRebootTime() {
        long rebootTime = new Date().getTime() - SystemClock.elapsedRealtime();
        Utils.logi(TAG, "getRebootTime(), rebootTime = " + rebootTime);
        return new Date().getTime() - SystemClock.elapsedRealtime();
    }

    private static boolean sIsServiceOnDestroying = false;

    /**
     * @param enable
     *            boolean
     */
    public static void setServiceOnDestroying(boolean enable) {
        sIsServiceOnDestroying = enable;
    }

    /**
     * @return boolean
     */
    public static boolean isServiceOnDestroying() {
        return sIsServiceOnDestroying;
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
        if (tag.contains("TagLog")) {
            synchronized (sLogBufferList) {
                if (sLogBufferList.size() >= MAX_LOG_BUFFER_SIZE) {
                    sLogBufferList.removeFirst();
                }
                sLogBufferList.addLast(integratedOutLog(msg));
            }
        }
        if ("eng".equals(Utils.BUILD_TYPE)) {
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
        if (tag.contains("TagLog")) {
            synchronized (sLogBufferList) {
                if (sLogBufferList.size() >= MAX_LOG_BUFFER_SIZE) {
                    sLogBufferList.removeFirst();
                }
                sLogBufferList.addLast(integratedOutLog(msg));
            }
        }
        if ("eng".equals(Utils.BUILD_TYPE)) {
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
        if (tag.contains("TagLog")) {
            synchronized (sLogBufferList) {
                if (sLogBufferList.size() >= MAX_LOG_BUFFER_SIZE) {
                    sLogBufferList.removeFirst();
                }
                sLogBufferList.addLast(integratedOutLog(msg));
            }
        }
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
        if (tag.contains("TagLog")) {
            synchronized (sLogBufferList) {
                if (sLogBufferList.size() >= MAX_LOG_BUFFER_SIZE) {
                    sLogBufferList.removeFirst();
                }
                sLogBufferList.addLast(integratedOutLog(msg));
            }
        }
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
        if (tag.contains("TagLog")) {
            synchronized (sLogBufferList) {
                if (sLogBufferList.size() >= MAX_LOG_BUFFER_SIZE) {
                    sLogBufferList.removeFirst();
                }
                sLogBufferList.addLast(integratedOutLog(msg));
            }
        }
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
        if (tag.contains("TagLog")) {
            synchronized (sLogBufferList) {
                if (sLogBufferList.size() >= MAX_LOG_BUFFER_SIZE) {
                    sLogBufferList.removeFirst();
                }
                sLogBufferList.addLast(integratedOutLog(msg));
            }
        }
        Log.e(tag, msg, tr);
    }

    /**
     * @param msg
     *            String
     * @return String
     */
    public static String integratedOutLog(String msg) {
        StringBuilder logsHead = new StringBuilder();
        Calendar calendar = Calendar.getInstance();
        calendar.setTimeInMillis(System.currentTimeMillis());
        String timeStr =
                new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSS ").format(calendar.getTime());
        logsHead.append(timeStr);
        logsHead.append(" " + android.os.Process.myPid());
        logsHead.append(" " + android.os.Process.myTid());
        logsHead.append(" " + msg);

        return logsHead.toString();
    }
}
