package com.debug.loggerui.framework;

import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.pm.PackageManager.NameNotFoundException;
import android.hardware.usb.UsbManager;
import android.net.Uri;
import android.os.Binder;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.Process;
import android.os.SystemClock;
import android.preference.PreferenceManager;
import android.view.WindowManager;

import com.debug.loggerui.R;
import com.debug.loggerui.controller.AbstractLogController;
import com.debug.loggerui.controller.BTHostLogController;
import com.debug.loggerui.controller.LogControllerUtils;
import com.debug.loggerui.controller.MobileLogController;
import com.debug.loggerui.controller.ModemLogController;
import com.debug.loggerui.controller.MultiLogTypesController;
import com.debug.loggerui.controller.NetworkLogController;
import com.debug.loggerui.permission.PermissionUtils;
import com.debug.loggerui.settings.ModemLogSettings;
import com.debug.loggerui.settings.SettingsActivity;
import com.debug.loggerui.taglog.TagLogManager;
import com.debug.loggerui.taglog.TagLogUtils;
import com.debug.loggerui.utils.Utils;
import com.log.handler.LogHandlerUtils.ModemLogStatus;

import java.io.File;

/**
 * @author MTK81255
 *
 */
public class DebugLoggerUIService extends Service {
    private static final String TAG = Utils.TAG + "/DebugLoggerUIService";

    private SharedPreferences mSharedPreferences = null;
    /**
     * Some settings like log auto start status will be stored in default preference, so UI do not
     * need to take care these values manually.
     */
    private SharedPreferences mDefaultSharedPreferences = null;

    /**
     * Which stage this service is running in, like starting/stopping log, memory dumping.
     */
    private int mGlobalRunningStage = Utils.RUNNING_STAGE_IDLE;

    /**
     * Current log storing path type, one of phone, internal SD card and external SD card.
     */

    /**
     * When service destroy, or log stopped, need to stop monitor thread. true: should stop monitor
     * thread; false: monitor thread can be running
     */
    private boolean mLogFolderMonitorThreadStopFlag = true;
    LogFolderMonitor mMonitorLogFolderThread = null;

    private NotificationManager mNM = null;

    /**
     * Monitor remaining log storage, remember it. When it become too less, give a notification
     */
    private int mRemainingStorage = 0;

    /**
     * When sd card unmount, if log stopped by itself before DebugLoggerUI receive unmount broadcast
     * in SD_STATUS_CHANGE_CHECK_TIME ms, still recover the log when sd card mount.
     */
    private final static long SD_STATUS_CHANGE_CHECK_TIME = 15000;

    private long mLastSDStatusChangedTime = 0;
    private Handler mServiceHandler;
    private static final int MSG_SHOW_LOW_MEMORY_DIALOG = 1;
    private ServiceStatusManager mServiceStatusManager;
    private boolean mIsServiceFirstStart = true;
    private boolean mIsDoingInitLogs = false;
    private boolean mIsServiceInitDone = false;

    @Override
    public void onCreate() {
        Utils.logi(TAG, "-->onCreate()");
        super.onCreate();
        this.setTheme(android.R.style.Theme_Holo_Light);
        mSharedPreferences = getSharedPreferences(Utils.CONFIG_FILE_NAME, Context.MODE_PRIVATE);
        mDefaultSharedPreferences = PreferenceManager.getDefaultSharedPreferences(this);

        // Monitor USB status
        mUSBStatusIntentFilter = new IntentFilter();
        mUSBStatusIntentFilter.addAction(Utils.ACTION_USB_STATE_CHANGED);
        registerReceiver(mUSBStatusReceiver, mUSBStatusIntentFilter, "android.permission.DUMP",
                null);

        // Monitor SD card storage status
        mSDStatusIntentFilter = new IntentFilter();
        mSDStatusIntentFilter.addAction(Intent.ACTION_MEDIA_BAD_REMOVAL);
        mSDStatusIntentFilter.addAction(Intent.ACTION_MEDIA_EJECT);
        mSDStatusIntentFilter.addAction(Intent.ACTION_MEDIA_REMOVED);
        mSDStatusIntentFilter.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
        mSDStatusIntentFilter.addAction(Intent.ACTION_MEDIA_MOUNTED);
        mSDStatusIntentFilter.addDataScheme("file");
        registerReceiver(mStorageStatusReceiver, mSDStatusIntentFilter, "android.permission.DUMP",
                null);

        // Monitor Phone internal storage status change
        mPhoneStorageIntentFilter = new IntentFilter();
        mPhoneStorageIntentFilter.addAction(Intent.ACTION_DEVICE_STORAGE_OK);
        mPhoneStorageIntentFilter.addAction(Intent.ACTION_DEVICE_STORAGE_LOW);
        registerReceiver(mStorageStatusReceiver, mPhoneStorageIntentFilter,
                "android.permission.DUMP", null);

        // Monitor shutdown event
        IntentFilter shutdownIntentFilter = new IntentFilter();
        shutdownIntentFilter.addAction(Intent.ACTION_SHUTDOWN);
        registerReceiver(mShutdonwReceiver, shutdownIntentFilter, "android.permission.DUMP", null);

        // Monitor time changed
        IntentFilter timeChangedIntentFilter = new IntentFilter();
        timeChangedIntentFilter.addAction(Intent.ACTION_TIMEZONE_CHANGED);
        timeChangedIntentFilter.addAction(Intent.ACTION_DATE_CHANGED);
        timeChangedIntentFilter.addAction(Intent.ACTION_TIME_CHANGED);
        registerReceiver(mTimeChangedReceiver, timeChangedIntentFilter, "android.permission.DUMP",
                null);

        if (Utils.isDeviceOwner()) {
            IntentFilter btFilter = new IntentFilter();
            btFilter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED);
            registerReceiver(mBTStateChangedReceiver, btFilter, "android.permission.DUMP", null);
        }

        HandlerThread handlerThread = new HandlerThread(this.getClass().getName());
        handlerThread.start();
        mServiceHandler = new ServiceHandler(handlerThread.getLooper());

        // Print version info
        try {
            String versionName =
                    getPackageManager().getPackageInfo(getPackageName(), 0).versionName;
            Utils.logi(TAG, "Version name=" + versionName);
        } catch (NameNotFoundException e) {
            Utils.loge(TAG, "Fail to get application version name.");
        }
        mServiceStatusManager = new ServiceStatusManager(this);
        mServiceStatusManager.statusChanged(
                              ServiceStatusManager.ServiceStatus.ONCREATE_DONE);
        DebugLoggerUIServiceManager.getInstance().initService();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Utils.logi(TAG, "-->onStartCommand()");
        if (!Utils.isDeviceOwner()) {
            Utils.logi(TAG, "It is not device owner, do nothing!");
            mServiceStatusManager.statusChanged(
                    ServiceStatusManager.ServiceStatus.ONSTARTCOMMAND_DONE);
            return Service.START_NOT_STICKY;
        }
        // If intent == null means service is auto restart for killed by system
        if (intent == null) {
            mIsServiceInitDone = true;
            mIsServiceFirstStart = false;
        } else if (mIsServiceFirstStart) {
            mIsServiceFirstStart = false;
            doInitForFirstStart();
        }
        if (Utils.isTaglogEnable()) {
            Utils.logi(TAG, "Request storage permission for eng load or taglog enable.");
            PermissionUtils.requestStoragePermissions();
            // Restarted by system, check whether need to resume TagLog process
            Utils.logd(TAG,
                    "Service is first started," + " check whether need to resume TagLog process");
        }
        TagLogManager.getInstance().startTagLogManager();
        startSocketService();

        mServiceStatusManager.statusChanged(
                ServiceStatusManager.ServiceStatus.ONSTARTCOMMAND_DONE);
        if (!"eng".equals(Utils.BUILD_TYPE) && !Utils.isTaglogEnable()) {
            Utils.logd(TAG,
                    "Taglog disabled and it not eng load, no need auto start service");
            return Service.START_NOT_STICKY;
        } else {
            return Service.START_STICKY;
        }
    }

    private void doInitForFirstStart() {
        LogConfig.getInstance().checkConfig();
        initPreferenceValues();
        mServiceHandler.obtainMessage(Utils.MSG_INIT_LOGS_FOR_SERVER_FIRST_START).sendToTarget();
    }

    private void initPreferenceValues() {
        Utils.logd(TAG, "-->initLogStatus()");
        for (Integer logType : Utils.LOG_TYPE_SET) {
            // Reset need recovery log running status flag
            if (mSharedPreferences.getBoolean(Utils.KEY_NEED_RECOVER_RUNNING_MAP.get(logType),
                    Utils.DEFAULT_VALUE_NEED_RECOVER_RUNNING)) {
                mSharedPreferences.edit()
                        .putBoolean(Utils.KEY_NEED_RECOVER_RUNNING_MAP.get(logType),
                        Utils.DEFAULT_VALUE_NEED_RECOVER_RUNNING).apply();
            }
        }
        // Reset log start up time
        mSharedPreferences.edit()
                .putLong(Utils.KEY_BEGIN_RECORDING_TIME, Utils.VALUE_BEGIN_RECORDING_TIME_DEFAULT)
                .apply();
        mSharedPreferences.edit()
        .putLong(Utils.KEY_END_RECORDING_TIME, Utils.VALUE_BEGIN_RECORDING_TIME_DEFAULT)
        .apply();
        // Reset modem assert file path
        mSharedPreferences.edit().remove(Utils.KEY_USB_MODE_VALUE) // After reboot, usb tether
                                                  // will be reset
                .remove(Utils.KEY_USB_CONNECTED_VALUE) // After reboot, assume
                                                       // usb not connected
                .apply();
        if (Utils.isDenaliMd3Solution()) {
            mSharedPreferences.edit().putString(Utils.KEY_C2K_MODEM_LOGGING_PATH, "").apply();
        }
    }

    @Override
    public void onDestroy() {
        Utils.logi(TAG, "-->onDestroy()");
        mServiceStatusManager.statusChanged(
                ServiceStatusManager.ServiceStatus.ONDESTROYING);
        Utils.setServiceOnDestroying(true);

        unregisterReceiver(mUSBStatusReceiver);
        unregisterReceiver(mStorageStatusReceiver);
        unregisterReceiver(mShutdonwReceiver);
        unregisterReceiver(mTimeChangedReceiver);
        mLogFolderMonitorThreadStopFlag = true;

        if (Utils.isDeviceOwner()) {
            unregisterReceiver(mBTStateChangedReceiver);
        }
        super.onDestroy();
        Process.killProcess(Process.myPid());
    }

    /**
     * return void.
     */
    private void startSocketService() {
        AEEConnection.getInstance().startSocketServer();
    }

    private void initLogsForServiceFirstStart() {
        Utils.logi(TAG, "initLogsForBootup(), " + "mIsDoingInitLogs = "
                + mIsDoingInitLogs);
        if (mIsDoingInitLogs) {
            return;
        }
        mIsDoingInitLogs = true;
        String currentLogPath = Utils.getCurrentLogPath();
        long timeout = 15000;
        while (currentLogPath == null || currentLogPath.isEmpty()) {
            try {
                Thread.sleep(1000);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            timeout -= 1000;
            if (timeout <= 0) {
                Utils.logw(TAG, "Waiting currentLogPath timeout for 15s!");
                break;
            }
            currentLogPath = Utils.getCurrentLogPath();
        }

        changeLogRunningStatus(true, Utils.SERVICE_STARTUP_TYPE_BOOT);
        Utils.logd(TAG,
                "Service is first started," + " check whether need to resume TagLog process");

        mIsServiceInitDone = true;
    }

    /**
     * @param intent
     *            Intent
     */
    public void doTagLogForManually(Intent intent) {
        String dbPath = intent.getStringExtra(Utils.EXTRA_KEY_EXP_PATH);
        if (Utils.MANUAL_SAVE_LOG.equalsIgnoreCase(dbPath)) {
            intent.putExtra(Utils.EXTRA_KEY_TAG_TYPE, TagLogUtils.TAG_MANUAL);
            intent.putExtra(Utils.EXTRA_KEY_EXP_TIME, TagLogUtils.getCurrentTimeString());
            TagLogManager.getInstance().startNewTaglog(intent);
        }
    }

    /**
     * @param intent
     *            Intent
     */
    public void dealWithBypassAction(Intent intent) {
        Intent intentToBypass = new Intent(Utils.ACTION_TO_BYPASS);
        String cmdName = intent.getStringExtra(Utils.EXTRA_ADB_CMD_NAME);
        Utils.logi(TAG, "dealWithBypassAction(), cmdName = " + cmdName);
        if (cmdName != null && cmdName.length() > 0
                && Utils.VALUE_BYPASS_GET_STATUS.equalsIgnoreCase(cmdName)) {
            intentToBypass.putExtras(intent.getExtras());
            int returnValue = LogControllerUtils.getLogControllerInstance(Utils.LOG_TYPE_MODEM)
                    .isLogRunning() ? 1 : 0;
            intentToBypass.putExtra(Utils.EXTRA_CMD_RESULT, returnValue);
            Utils.logi(TAG, " returnValue = " + returnValue);
            Utils.sendBroadCast(intentToBypass);
        } else {
            Utils.loge(TAG, "The intent from " + Utils.ACTION_FROM_BYPASS + " is not support!");
        }
    }

    /**
     * @param intent
     *            Intent
     */
    public void daelWithADBCommand(Intent intent) {
        int logCluster =
                intent.getIntExtra(Utils.EXTRA_ADB_CMD_TARGET, Utils.DEFAULT_ADB_CMD_TARGET);
        String command = intent.getStringExtra(Utils.EXTRA_ADB_CMD_NAME);
        String logname = intent.getStringExtra(Utils.EXTRA_ADB_CMD_LOGNAME);
        Utils.logd(TAG, "Receive adb command, logCluster=" + logCluster + ", command=" + command
                + ", logname" + logname);
        if (Utils.ADB_COMMAND_SWITCH_TAGLOG.equals(command)) {
            int iTagLogEnabled = intent.getIntExtra(Utils.EXTRA_ADB_CMD_TARGET,
                    Utils.TAGLOG_CONFIG_VALUE_INVALID);
            Utils.logd(TAG,
                    "Receive a Taglog configuration broadcast, target value=" + iTagLogEnabled);
            if (Utils.TAGLOG_CONFIG_VALUE_ENABLE == iTagLogEnabled) {
                mSharedPreferences.edit().putBoolean(Utils.TAG_LOG_ENABLE, true).apply();
                PermissionUtils.requestStoragePermissions();
            } else if (Utils.TAGLOG_CONFIG_VALUE_DISABLE == iTagLogEnabled) {
                mSharedPreferences.edit().putBoolean(Utils.TAG_LOG_ENABLE, false).apply();
            } else {
                Utils.logw(TAG, "Configure taglog value invalid: " + iTagLogEnabled);
            }
            return;
        } else if (Utils.ADB_COMMAND_ALWAYS_TAG_MODEMLOG.equals(command)) {
            int iAlwaysTagModemLogEnabled = intent.getIntExtra(Utils.EXTRA_ADB_CMD_TARGET,
                    Utils.TAGLOG_CONFIG_VALUE_INVALID);
            Utils.logd(TAG, "Receive a Always Tag ModemLog configuration broadcast,"
                    + " target value = " + iAlwaysTagModemLogEnabled);
            if (iAlwaysTagModemLogEnabled == 1) {
                mDefaultSharedPreferences.edit()
                        .putBoolean(SettingsActivity.KEY_ALWAYS_TAG_MODEM_LOG_ENABLE, true).apply();
            } else if (iAlwaysTagModemLogEnabled == 0) {
                mDefaultSharedPreferences.edit()
                        .putBoolean(SettingsActivity.KEY_ALWAYS_TAG_MODEM_LOG_ENABLE, false)
                        .apply();
            } else {
                Utils.logw(TAG, "Configure Always Tag ModemLog value invalid: "
                        + iAlwaysTagModemLogEnabled);
            }
            return;
        } else if (Utils.ADB_COMMAND_MONITOR_ABNORMAL_EVENT.equals(command)) {
            // Can not move to modemLogController for the EXTRA_ADB_CMD_TARGET is not logCluster
            // adb shell am broadcast -a com.mediatek.mtklogger.ADB_CMD -e cmd_name
            // pls_monitor_modem_abnormal_event --ei cmd_target 1
            int iMonitor = intent.getIntExtra(Utils.EXTRA_ADB_CMD_TARGET,
                    Utils.TAGLOG_CONFIG_VALUE_INVALID);
            Utils.logd(TAG,
                    "Receive a monitor abnormal event broadcast, iMonitor value=" + iMonitor);
            if (1 == iMonitor) {
                mDefaultSharedPreferences.edit()
                        .putBoolean(ModemLogSettings.KEY_MD_MONITOR_MODEM_ABNORMAL_EVENT, true)
                        .apply();
            } else if (0 == iMonitor) {
                mDefaultSharedPreferences.edit()
                        .putBoolean(ModemLogSettings.KEY_MD_MONITOR_MODEM_ABNORMAL_EVENT, false)
                        .apply();
            } else {
                Utils.logw(TAG, "Configure monitor abnormal event value invalid: " + iMonitor);
            }
            return;
        } else if (command.startsWith(Utils.ADB_COMMAND_SWITCH_LOGPATH)) {
            // adb shell am broadcast -a com.debug.loggerui.ADB_CMD
            // -e switch_logpath_external_sd --ei cmd_target 0
            if (command.length() == Utils.ADB_COMMAND_SWITCH_LOGPATH.length()) {
                // adb shell am broadcast -a com.debug.loggerui.ADB_CMD
                // -e switch_logpath -e cmd_target external_sd
                String targetLogpath = intent.getStringExtra(Utils.EXTRA_ADB_CMD_TARGET);
                Utils.logd(TAG, "Receive a log path swithc configuration broadcast, target value="
                        + targetLogpath);
                command = command + "_" + targetLogpath;
            }
        } else if (command.startsWith(Utils.ADB_COMMAND_SWITCH_MODEM_LOG_MODE)) {
            int targetLogMode = intent.getIntExtra(Utils.EXTRA_ADB_CMD_TARGET, 0);
            Utils.logd(TAG, "Receive a modem log mode configuration broadcast, target value="
                    + targetLogMode);
            // Let modem log instance to handle this command
            command = command + "," + targetLogMode;
            logCluster = Utils.LOG_TYPE_MODEM;
        } else if (command.startsWith(Utils.ADB_COMMAND_SET_MODEM_LOG_SIZE)) {
            // Can not move to modemLogController for the EXTRA_ADB_CMD_TARGET is not logCluster
            int targetMDLogIndex = intent.getIntExtra(Utils.EXTRA_ADB_CMD_TARGET, 0);
            Utils.logd(TAG, "Receive a set modem log size configuration broadcast,"
                    + " target modemindex=" + targetMDLogIndex);
            command = command + "," + targetMDLogIndex;
            logCluster = Utils.LOG_TYPE_MODEM;
        }
        dealWithAdbCommand(logCluster, command);
    }

    /**
     * @param intent
     *            Intent
     */
    public void dealWithMDLoggerRestart(Intent intent) {
        String resetModemIndexStr = intent.getStringExtra(Utils.EXTRA_RESET_MD_INDEX);
        int resetModemIndex = 0;
        if (resetModemIndexStr != null && resetModemIndexStr.length() != 0) {
            try {
                resetModemIndex = Integer.parseInt(resetModemIndexStr);
            } catch (NumberFormatException e) {
                Utils.loge(TAG, "Reset modem log instance index format is error!");
            }
        }
        Utils.logi(TAG, "reset modem log instance index=" + resetModemIndex);
        // Receive update command from native, just update log running
        // status.

        ModemLogController.getInstance().reconnectToModemLog(Utils.getCurrentLogPath());
    }

    /**
     * Update log folder monitor's running state, when log started, start monitor, when log stopped,
     * stop the former monitor thread.
     */
    private void updateLogFolderMonitor() {
        boolean isLogRunning = LogControllerUtils.isAnyControlledLogRunning();

        Utils.logd(TAG, "-->updateLogFolderMonitor(), isLogRunning=" + isLogRunning
                + ", mLogFolderMonitorThreadStopFlag=" + mLogFolderMonitorThreadStopFlag);
        synchronized (mLock) {
            if (isLogRunning && mLogFolderMonitorThreadStopFlag) {
                mMonitorLogFolderThread = new LogFolderMonitor();
                mMonitorLogFolderThread.start();
                mLogFolderMonitorThreadStopFlag = false;
                Utils.logv(TAG, "Log is running, so start monitor log folder");
            } else if (!isLogRunning && !mLogFolderMonitorThreadStopFlag) {
                Utils.logv(TAG,
                        "Log is stopped," + " so need to stop log folder monitor if any exist.");
                mLogFolderMonitorThreadStopFlag = true;
                if (mMonitorLogFolderThread != null) {
                    mMonitorLogFolderThread.interrupt();
                    mMonitorLogFolderThread = null;
                }
                // Since log were all stopped, reset storage monitor status
                if (mNM == null) {
                    mNM = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
                }
                if (mDefaultSharedPreferences.getBoolean(Utils.KEY_PREFERENCE_NOTIFICATION_ENABLED,
                        Utils.DEFAULT_NOTIFICATION_ENABLED_VALUE)) {
                    mNM.cancel(R.drawable.ic_notification_low_storage);
                }
                mRemainingStorage = 0;
            }
        }
    }

    private ServiceBinder mServiceBinder = new ServiceBinder();

    @Override
    public IBinder onBind(Intent intent) {
        Utils.logi(TAG, "-->onBind()");
        return mServiceBinder;
    }

    /**
     * @author MTK81255
     *
     */
    public class ServiceBinder extends Binder {
        public DebugLoggerUIService getDebugLoggerUIService() {
            return DebugLoggerUIService.this;
        }
    }

    private IntentFilter mUSBStatusIntentFilter = null;
    private BroadcastReceiver mUSBStatusReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (Utils.ACTION_USB_STATE_CHANGED.equals(action)) {
                Utils.logi(TAG, " Current init status is " + mIsServiceInitDone);
                if (!mIsServiceInitDone) {
                    Utils.logi(TAG, "The service is not inited,"
                            + " ignore ACTION_USB_STATE_CHANGED broadcast!");
                    return;
                }
                boolean isModemLogRunning = LogControllerUtils.getLogControllerInstance(
                        Utils.LOG_TYPE_MODEM).isLogRunning();
                boolean isModemLogControlled = LogControllerUtils.getLogControllerInstance(
                        Utils.LOG_TYPE_MODEM).isLogControlled();
                String modemLogRunningMode = mDefaultSharedPreferences
                        .getString(Utils.KEY_MD_MODE_1, Utils.MODEM_MODE_SD);
                Utils.logv(TAG, "isModemLogRunning ? " + isModemLogRunning
                        + ", isModemLogControlled ? " + isModemLogControlled
                        + ", modemLogRunningMode = " + modemLogRunningMode);
                if (!isModemLogRunning || !isModemLogControlled
                        || !Utils.MODEM_MODE_USB.equals(modemLogRunningMode)) {
                    return;
                }

                boolean usbConfigured = intent.getBooleanExtra(UsbManager.USB_CONFIGURED, false);
                boolean newUsbConnected = intent.getBooleanExtra(UsbManager.USB_CONNECTED, false);
                int newUsbMode = Utils.getCurrentUsbMode(intent);
                int oldUsbModeValue = mSharedPreferences.getInt(Utils.KEY_USB_MODE_VALUE,
                        Utils.VALUE_USB_MODE_UNKNOWN);
                boolean oldUsbConnected =
                        mSharedPreferences.getBoolean(Utils.KEY_USB_CONNECTED_VALUE, false);
                boolean needNotifyModemLog =
                        (newUsbMode != oldUsbModeValue) || (newUsbConnected != oldUsbConnected);

                Utils.logi(TAG,
                        " usbConfigured?" + usbConfigured + ", newUsbConnected=" + newUsbConnected
                                + ", oldUsbConnected=" + oldUsbConnected + ", newUsbMode="
                                + newUsbMode + ", oldUSBModeValue=" + oldUsbModeValue
                                + ", needNotifyModemLog=" + needNotifyModemLog);
                if (needNotifyModemLog) {
                    mSharedPreferences.edit().putInt(Utils.KEY_USB_MODE_VALUE, newUsbMode)
                            .putBoolean(Utils.KEY_USB_CONNECTED_VALUE, newUsbConnected).apply();
                    Utils.logv(TAG,
                            "Modem log is running in USB mode, need to send down switch command.");
                    ModemLogController.getInstance().notifyUSBModeChanged();
                } else {
                    Utils.logv(TAG,
                            "Modem log is not running in USB mode or USB status not change, "
                                    + "do not need to send down switch command. newUsbMode="
                                    + newUsbMode + ", usbConnected=" + newUsbConnected);
                }
            }
        }
    };

    private IntentFilter mSDStatusIntentFilter = null;
    private IntentFilter mPhoneStorageIntentFilter = null;
    private BroadcastReceiver mStorageStatusReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            String currentLogPathType = Utils.getCurrentLogPathType();
            Utils.logi(TAG, "Storage status changed, action=" + action + ", current logPathType="
                    + currentLogPathType);

            if (!mIsServiceInitDone) {
                Utils.logd(TAG, "DebugLoggerUIservice is not inited just return!");
                return;
            }
            if (Utils.LOG_PATH_TYPE_PORTABLE_STORAGE.equals(currentLogPathType)
                    || Utils.LOG_PATH_TYPE_DEVICE_STORAGE.equals(currentLogPathType)) {
                Uri data = intent.getData();
                String affectedPath = null;
                if (data != null) {
                    affectedPath = data.getPath();
                }

                Utils.logd(TAG, "AffectedPath=" + affectedPath);
                currentStorageStatusChange(action, affectedPath);

            } else {
                if (Intent.ACTION_DEVICE_STORAGE_LOW.equals(action)) {
                    Utils.logw(TAG, "Phone storage is low now. What should I do? ");
                    // Stop all running log now because log storage is
                    // unavailable
                    changeLogRunningStatus(false, Utils.SERVICE_SHUTDOWN_TYPE_BAD_STORAGE);
                }
            }
        }
    };

    private void currentStorageStatusChange(final String action, final String affectedPath) {
        new Thread() {
            @Override
            public void run() {
                if (affectedPath == null) {
                    Utils.logi(TAG, "affectedPath, ignore.");
                    return;
                }
                if (!isAffectCurrentLogType(affectedPath)) {
                    Utils.logi(TAG, "isAffectCurrentLogType = false, ignore.");
                    return;
                }
                if (Intent.ACTION_MEDIA_BAD_REMOVAL.equals(action)
                        || Intent.ACTION_MEDIA_EJECT.equals(action)
                        || Intent.ACTION_MEDIA_REMOVED.equals(action)
                        || Intent.ACTION_MEDIA_UNMOUNTED.equals(action)) {
                    long currentTime = System.currentTimeMillis();
                    long intervalTime = currentTime - mLastSDStatusChangedTime;
                    Utils.logi(TAG,
                            "The SD card status changed time is currentTime = " + currentTime
                                    + ", mLastSDStatusChangedTime = " + mLastSDStatusChangedTime
                                    + ", intervalTime = " + intervalTime);
                    mLastSDStatusChangedTime = currentTime;
                    if (intervalTime >= 0 && intervalTime <= SD_STATUS_CHANGE_CHECK_TIME) {
                        Utils.logw(TAG, "The SD card status changed time is < "
                                + SD_STATUS_CHANGE_CHECK_TIME + ". Ignore this changed!");
                        return;
                    }
                    // Stop all running log now because log storage is
                    // unavailable
                    changeLogRunningStatus(false, Utils.SERVICE_SHUTDOWN_TYPE_BAD_STORAGE);
                } else if (Intent.ACTION_MEDIA_MOUNTED.equals(action)) {
                    changeLogRunningStatus(true, Utils.SERVICE_STARTUP_TYPE_STORAGE_RECOVERY);
                } else {
                    Utils.loge(TAG, "Unsupported broadcast action for SD card. action=" + action);
                }
            }
        }.start();
    }

    /**
     * @param affectLogPath
     *            String
     * @return boolean
     */
    public boolean isAffectCurrentLogType(String affectLogPath) {
        String internalLogPath = Utils.getInternalSdPath();
        String externalLogPath = Utils.getExternalSdPath();
        String currentLogPath = Utils.getCurrentLogPath();

        Utils.logd(TAG, "affectLogPath:" + affectLogPath + ", internalLogPath :" + internalLogPath
                + ", externalLogPath :" + externalLogPath + ", currentLogPath :" + currentLogPath);

        String affectLogPathType = "";
        if (internalLogPath != null && affectLogPath.startsWith(internalLogPath)) {
            affectLogPathType = Utils.LOG_PATH_TYPE_DEVICE_STORAGE;
        }
        if ((externalLogPath != null && affectLogPath.startsWith(externalLogPath))) {
            affectLogPathType = Utils.LOG_PATH_TYPE_PORTABLE_STORAGE;
        }
        boolean isUnmountEvent = affectLogPathType.isEmpty();
        String currentLogPathType = Utils.getCurrentLogPathType();
        Utils.logd(TAG,
                "affectLogPath:" + affectLogPath + ", affectLogPathType :" + affectLogPathType
                        + ", isUnmountEvent :" + isUnmountEvent + ", currentLogPathType :"
                        + currentLogPathType);
        if (!isUnmountEvent) {
            return affectLogPathType.equals(currentLogPathType);
        } else {
            // If unmount event && current log path is null, return true.
            if (currentLogPath == null || currentLogPath.isEmpty()) {
                return true;
            }
        }
        return false;
    }

    // Broadcastreceiver for ACTION_SHUTDOWN
    private BroadcastReceiver mShutdonwReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context arg0, Intent arg1) {
            String action = arg1.getAction();
            Utils.logi(TAG, " mShutdonwReceiver intent action: " + action
                    + ", mIsAlreadySendShutDown ?" + Utils.getAlreadySendShutDown());
            if (Intent.ACTION_SHUTDOWN.equals(action)) {
                Utils.logd(TAG, "Get a Normal SHUTDOWN event!");

                if (Utils.getAlreadySendShutDown()) {
                    Utils.logd(TAG, "Already send stop to network for normal shutdown,return!");
                    return;
                }
                Utils.setAlreadySendShutDown(true);
                if (!Utils.isDeviceOwner()) {
                    // Do not need stop network log for not device owner.
                    return;
                }
                mServiceHandler.obtainMessage(Utils.MSG_STOP_NETLOG_AT_SHUT_DOWN).sendToTarget();
            }
        }
    };

    /**
     * Broadcast receiver for monitor Time settings changed.
     */
    private BroadcastReceiver mTimeChangedReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Utils.logd(TAG, "TimeChangedReceiver, action = " + action);
        }
    };

    /**
     * Broadcast receiver for bluetooth state changed event.
     */
    private BroadcastReceiver mBTStateChangedReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(BluetoothAdapter.ACTION_STATE_CHANGED)) {
                int state =
                        intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, BluetoothAdapter.ERROR);
                Utils.logi(TAG, "Monitor bt statue changed event. State = " + state);
                switch (state) {
                case BluetoothAdapter.STATE_ON:
                    mServiceHandler.obtainMessage(Utils.MSG_START_BTLOG).sendToTarget();
                    break;
                default:
                    break;
                }
            }
        }
    };

    private void startBTLog() {
        AbstractLogController btHostLogController = BTHostLogController.getInstance();
        if (!btHostLogController.isLogFeatureSupport() || !btHostLogController.isLogControlled()) {
            Utils.logi(TAG, "BTLog is disabled in UI settings, so no need do start.");
            return;
        }
        if (mDefaultSharedPreferences
                .getBoolean(Utils.KEY_START_AUTOMATIC_MAP.get(Utils.LOG_TYPE_BTHOST), false)) {
            Utils.logi(TAG, "Start BTHostLog for reason : BluetoothAdapter.STATE_ON!");
            LogControllerUtils.getLogControllerInstance(Utils.LOG_TYPE_BTHOST)
                    .startLog(Utils.getCurrentLogPath());
        }
    }

    /**
     * Start or stop all log at the same time when environment has changed, or at bootup/shutdown
     * time.
     *
     * @param enable
     *            true for enable, false for disable
     * @param reason
     *            Why this method is called, boot up, or nothing stand for user's operation
     */
    private void changeLogRunningStatus(boolean enable, String reason) {
        Utils.logd(TAG,
                "-->changeLogRunningStatus(), enable?" + enable + ", reason=[" + reason + "]");
        if (mSharedPreferences == null) {
            Utils.loge(TAG, "SharedPreference instance is null");
            return;
        }
        // Which log type will be affected in this operation
        int affectedLog = 0;
        if (enable) {
            // If this method is called because of boot up, try to enable all
            // boot-automatic log instance
            // If just because of storage recovery, restart former interrupted
            // log
            if (Utils.SERVICE_STARTUP_TYPE_BOOT.equals(reason)) {
                for (Integer logType : Utils.LOG_TYPE_SET) {
                    if (logType == Utils.LOG_TYPE_MET) {
                        continue;
                    }
                    if (Utils.VALUE_START_AUTOMATIC_ON == mDefaultSharedPreferences.getBoolean(
                            Utils.KEY_START_AUTOMATIC_MAP.get(logType),
                            Utils.DEFAULT_CONFIG_LOG_AUTO_START_MAP.get(logType))) {
                        // At boot time, no matter native status, just send down
                        // start command
                        // /**Need to enable log at boot time*/
                        // && Utils.VALUE_STATUS_STOPPED ==
                        // mSharedPreferences.getInt(
                        // Utils.KEY_STATUS_MAP.get(logType),
                        // Utils.VALUE_STATUS_STOPPED)/**not be on yet*/){
                        affectedLog += logType;
                    }
                }
            } else if (Utils.SERVICE_STARTUP_TYPE_STORAGE_RECOVERY.equals(reason)) {
                for (Integer logType : Utils.LOG_TYPE_SET) {
                    if (logType == Utils.LOG_TYPE_MET) {
                        continue;
                    }
                    boolean needRecovery = mSharedPreferences.getBoolean(
                            Utils.KEY_NEED_RECOVER_RUNNING_MAP.get(logType),
                            Utils.DEFAULT_VALUE_NEED_RECOVER_RUNNING);
                    boolean controlledStatus = LogControllerUtils.getLogControllerInstance(logType)
                            .isLogControlled();
                    // autostart/stop
                    boolean autostart = mDefaultSharedPreferences.getBoolean(
                            Utils.KEY_START_AUTOMATIC_MAP.get(logType),
                            Utils.DEFAULT_CONFIG_LOG_AUTO_START_MAP.get(logType));
                    Utils.logd(TAG,
                            "For log[" + logType + "], needRecovery?" + needRecovery
                                    + ",controlledStatus=" + controlledStatus + ", autostart = "
                                    + autostart);
                    if (needRecovery || autostart) {
                        if (controlledStatus) {
                            affectedLog += logType;
                        }
                        mSharedPreferences.edit()
                                .putBoolean(Utils.KEY_NEED_RECOVER_RUNNING_MAP.get(logType), false)
                                .apply();
                    }
                }
            }
            Utils.logv(TAG, " affectedLog=" + affectedLog);
            if (affectedLog > 0) {
                startRecording(affectedLog, reason);
            }
        } else { // Try to disable all running log instance
            for (Integer logType : Utils.LOG_TYPE_SET) {
                if (logType == Utils.LOG_TYPE_MET) {
                    continue;
                }
                boolean isRunning = LogControllerUtils.getLogControllerInstance(logType)
                        .isLogRunning();
                /** Be running right now */
                boolean shouldAutoStarted =
                        (Utils.VALUE_START_AUTOMATIC_ON == mDefaultSharedPreferences.getBoolean(
                                Utils.KEY_START_AUTOMATIC_MAP.get(logType),
                                Utils.DEFAULT_CONFIG_LOG_AUTO_START_MAP.get(logType)));

                boolean isStoppedInShortTime = isStoppedInShortTime(logType);
                if (isRunning || isStoppedInShortTime || (shouldAutoStarted
                        && Utils.SERVICE_SHUTDOWN_TYPE_SD_TIMEOUT.equals(reason))) {
                    // Should be on at boot time, but SD time out, since native
                    // layer may already running, need to stop
                    // but for USB mode modem log, ignore this timeout
                    if (logType == Utils.LOG_TYPE_MODEM
                            && Utils.SERVICE_SHUTDOWN_TYPE_SD_TIMEOUT.equals(reason)) {
                        String currentMDLogMode = mDefaultSharedPreferences
                                .getString(Utils.KEY_MD_MODE_1, Utils.MODEM_MODE_SD);
                        if (Utils.MODEM_MODE_USB.equals(currentMDLogMode)) {
                            Utils.logd(TAG, "For USB mode modem log, ignore SD timeout event.");
                            continue;
                        }
                    }
                    affectedLog += logType;
                    if ((isRunning || isStoppedInShortTime)
                            && Utils.SERVICE_SHUTDOWN_TYPE_BAD_STORAGE.equals(reason)) {
                        // Storage become unavailable, need to recovery log when
                        // storage is available again
                        // set a flag for this
                        mSharedPreferences.edit()
                                .putBoolean(Utils.KEY_NEED_RECOVER_RUNNING_MAP.get(logType), true)
                                .apply();
                    }
                }
            }
            Utils.logv(TAG, " affectedLog=" + affectedLog);
            if (affectedLog > 0) {
                stopRecording(affectedLog, reason);
            }
        }
    }

    private boolean isStoppedInShortTime(int logType) {
        boolean rs = false;
        long currentTime = System.currentTimeMillis();
        long stopTime = mSharedPreferences.getLong(Utils.KEY_SELF_STOP_TIME_MAP.get(logType), 0);
        long intervalTime = currentTime - stopTime;
        if (intervalTime >= 0 && intervalTime <= SD_STATUS_CHANGE_CHECK_TIME) {
            rs = true;
        }
        Utils.logi(TAG, "isStoppedInShortTime() logType = " + logType + ". Rs = " + rs);
        return rs;
    }

    /**
     * At running time, log folder may be deleted, so we need to monitor each log related log
     * folder. If the folder is deleted, we should consider whether stop that log manually(For
     * Network Log)
     */
    class LogFolderMonitor extends Thread {
        @Override
        public void run() {
            Utils.logd(TAG, "Begin to monitor log folder status...");
            while (!mLogFolderMonitorThreadStopFlag) {
                String currentLogPath = Utils.getCurrentLogPath();
                boolean isStorageReady = (currentLogPath != null && !currentLogPath.isEmpty());
                if (isStorageReady) {
                    checkRemainingStorage(currentLogPath);
                }
                try {
                    Thread.sleep(Utils.DURATION_CHECK_LOG_FOLDER);
                } catch (InterruptedException e) {
                    Utils.logw(TAG, "Waiting check log folder been interrupted.");
                    continue;
                }
            }

            Utils.logd(TAG, "End monitor log folder status.");
        }
    }

    private Notification.Builder mNotificationBuilder;
    /**
     * We will set a storage water level, when not too much storage is remaining, give user a
     * notification to delete old logs.
     */
    private void checkRemainingStorage(String currentLogPath) {
        int remainingSize = Utils.getAvailableStorageSize(currentLogPath);
        // Utils.logv(TAG,
        // "-->checkRemainingStorage(), remainingSize="+remainingSize+
        // ", former storage="+mRemainingStorage);
        if (remainingSize < Utils.RESERVED_STORAGE_SIZE + 2) {
            stopRecording(Utils.LOG_TYPE_GPSHOST | Utils.LOG_TYPE_BTHOST | Utils.LOG_TYPE_CONNSYSFW,
                    Utils.SERVICE_SHUTDOWN_TYPE_BAD_STORAGE);
            Utils.logd(TAG, "stop connsys log,for SD card is almost full");
            Intent intent = new Intent();
            intent.setAction(Utils.ACTION_REMAINING_STORAGE_LOW);
            intent.putExtra(Utils.EXTRA_REMAINING_STORAGE, remainingSize);
            Utils.sendBroadCast(intent);
        }
        if (remainingSize < Utils.DEFAULT_STORAGE_WATER_LEVEL && (mRemainingStorage == 0
                || mRemainingStorage >= Utils.DEFAULT_STORAGE_WATER_LEVEL)) {
            Utils.logi(TAG,
                    "Remaining log storage drop below water level," + " give a notification now");
            if (mNM == null) {
                mNM = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
            }
            Utils.logd(TAG, "Log storage drop down below water level, give out a notification");

            Intent backIntent = new Intent();
            backIntent.setComponent(
                    new ComponentName("com.debug.loggerui", "com.debug.loggerui.MainActivity"));
            backIntent.setFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP);
            PendingIntent pendingIntent = PendingIntent.getActivity(this, 0, backIntent, 0);
            if (mNotificationBuilder == null) {
                mNotificationBuilder = new Notification.Builder(DebugLoggerUIService.this,
                        ServiceStatusManager.getNotificationChannelId());
            }
            mNotificationBuilder
                    .setContentText(getText(R.string.notification_out_of_storage_summary))
                    .setTicker(getText(R.string.notification_nearly_out_of_storage))
                    .setContentTitle(getText(R.string.notification_nearly_out_of_storage))
                    .setSmallIcon(R.drawable.ic_notification_low_storage)
                    .setContentIntent(pendingIntent);

            if (mDefaultSharedPreferences.getBoolean(Utils.KEY_PREFERENCE_NOTIFICATION_ENABLED,
                    Utils.DEFAULT_NOTIFICATION_ENABLED_VALUE)) {
                mNM.notify(365002, mNotificationBuilder.build());
            } else {
                Utils.logw(TAG, "Notification is disabled, does not show any notification.");
            }
            Intent intent = new Intent();
            intent.setAction(Utils.ACTION_REMAINING_STORAGE_LOW);
            intent.putExtra(Utils.EXTRA_REMAINING_STORAGE, remainingSize);
            Utils.sendBroadCast(intent);

            if (Utils.isReleaseToCustomer1()) {
                mUIHandler.sendEmptyMessage(MSG_SHOW_LOW_MEMORY_DIALOG);
            }
        } else if (mRemainingStorage > 0 && mRemainingStorage < Utils.DEFAULT_STORAGE_WATER_LEVEL
                && remainingSize >= Utils.DEFAULT_STORAGE_WATER_LEVEL) {
            if (mNM == null) {
                mNM = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
            }
            Utils.logd(TAG, "Log storage resume upto water level, clear former notification");
            if (mDefaultSharedPreferences.getBoolean(Utils.KEY_PREFERENCE_NOTIFICATION_ENABLED,
                    true)) {
                mNM.cancel(R.drawable.ic_notification_low_storage);
            }
        }

        mRemainingStorage = remainingSize;
    }

    private Handler mUIHandler = new Handler(Looper.getMainLooper()) {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case MSG_SHOW_LOW_MEMORY_DIALOG:
                showLowStorageDialog();
                break;
            default:
                break;
            }
        };
    };

    private void showLowStorageDialog() {
        Utils.logd(TAG, "showLowStorageDialog");
        Context sContext = DebugLoggerUIService.this;
        String message = sContext.getString(R.string.low_storage_warning_dialog_msg,
                Utils.DEFAULT_STORAGE_WATER_LEVEL);
        Builder builder = new AlertDialog.Builder(sContext)
                .setTitle(sContext.getText(R.string.low_storage_warning_dialog_title).toString())
                .setMessage(message).setPositiveButton(android.R.string.yes, new OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                    }
                });
        AlertDialog dialog = builder.create();
        dialog.getWindow().setType(WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY);
        dialog.setCancelable(false);
        dialog.setInverseBackgroundForced(true);
        dialog.show();
    }

    private void dealWithAdbCommand(int logTypeCluster, String command) {
        Utils.logi(TAG, "-->dealWithAdbCommand(), logTypeCluster=" + logTypeCluster + ", command="
                + command);
        if (Utils.ADB_COMMAND_START.equals(command)) { // start log command
            if (SecurityWarning.isNeedAlert()) {
                SecurityWarning.DialogExcute dialogExcute = new SecurityWarning.DialogExcute() {
                    @Override
                    public void okButtonClicked() {
                        new Thread(new Runnable() {
                            @Override
                            public void run() {
                                startRecording(logTypeCluster, Utils.SERVICE_STARTUP_TYPE_ADB);
                            }
                        }).start();
                    }

                    @Override
                    public void cancelButtonClicked() {
                    }

                };
                SecurityWarning.getInstance().getAlertHander()
                        .obtainMessage(SecurityWarning.MSG_START_LOGS, dialogExcute).sendToTarget();
            } else {
                startRecording(logTypeCluster, Utils.SERVICE_STARTUP_TYPE_ADB);
            }
        } else if (Utils.ADB_COMMAND_STOP.equals(command)) {
            stopRecording(logTypeCluster, Utils.SERVICE_STARTUP_TYPE_ADB);
            if (SecurityWarning.isNeedAlert()) {
                SecurityWarning.DialogExcute dialogExcute = new SecurityWarning.DialogExcute() {
                    @Override
                    public void okButtonClicked() {
                    }

                    @Override
                    public void cancelButtonClicked() {
                    }

                };
                SecurityWarning.getInstance().getAlertHander()
                        .obtainMessage(SecurityWarning.MSG_STOP_LOGS, dialogExcute)
                        .sendToTarget();
                return;
            }
        } else if (Utils.ADB_COMMAND_RESTART.equals(command)) {
            restartRecording(logTypeCluster, Utils.SERVICE_STARTUP_TYPE_ADB);
        } else if (command != null
                && command.startsWith(Utils.ADB_COMMAND_SET_LOG_AUTO_START_PREFIX)) {
            String newValue = command.substring(command.length() - 1);
            if (newValue.equals("0") || newValue.equals("1")) {
                for (Integer logType : Utils.LOG_TYPE_SET) {
                    if ((logType & logTypeCluster) == 0 || logType == Utils.LOG_TYPE_MET) {
                        continue;
                    }
                    LogControllerUtils.getLogControllerInstance(logType)
                            .setBootupLogSaved(newValue.equals("1"));
                }
            } else {
                Utils.logw(TAG, "Unsupported auto start value");
            }
        } else if (command != null
                && command.startsWith(Utils.ADB_COMMAND_SET_LOG_UI_ENABLED_PREFIX)) {
            String newValue = command.substring(command.length() - 1);
            if (newValue.equals("0") || newValue.equals("1")) {
                for (Integer logType : Utils.LOG_TYPE_SET) {
                    if ((logType & logTypeCluster) == 0) {
                        continue;
                    }
                    if (logType == Utils.LOG_TYPE_MET) {
                        continue;
                    }
                    mDefaultSharedPreferences.edit()
                            .putBoolean(SettingsActivity.KEY_LOG_SWITCH_MAP.get(logType),
                                    newValue.equals("1"))
                            .apply();
                }
            } else {
                Utils.logw(TAG, "Unsupported auto start value");
            }
        } else if (command != null && command.startsWith(Utils.ADB_COMMAND_SWITCH_LOGPATH)) {
            // adb shell am broadcast -a com.debug.loggerui.ADB_CMD
            // -e switch_logpath_external_sd --ei cmd_target 0
            String newLogPathType =
                    command.substring(Utils.ADB_COMMAND_SWITCH_LOGPATH.length() + 1);
            if (Utils.LOG_PATH_TYPE_INTERNAL_SD.equalsIgnoreCase(newLogPathType)) {
                newLogPathType = Utils.LOG_PATH_TYPE_DEVICE_STORAGE;
            } else if (Utils.LOG_PATH_TYPE_EXTERNAL_SD.equalsIgnoreCase(newLogPathType)) {
                newLogPathType = Utils.LOG_PATH_TYPE_PORTABLE_STORAGE;
            }
            String oldValue = Utils.getCurrentLogPathType();
            if (!oldValue.equals(newLogPathType)) {
                mDefaultSharedPreferences.edit().putString(
                        SettingsActivity.KEY_ADVANCED_LOG_STORAGE_LOCATION,
                        newLogPathType).apply();
            }
        } else if (command != null && command.startsWith(Utils.ADB_COMMAND_SET_LOG_SIZE_PREFIX)) {
            String newValueStr = command.substring(Utils.ADB_COMMAND_SET_LOG_SIZE_PREFIX.length());

            int newLogSize = 0;
            try {
                newLogSize = Integer.parseInt(newValueStr);
            } catch (NumberFormatException e) {
                Utils.loge(TAG, "Invalid set log size parameter: " + newValueStr);
                return;
            }
            if (newLogSize <= 0) {
                Utils.loge(TAG, "Given log size should bigger than zero, but got " + newValueStr);
                return;
            }

            for (Integer logType : Utils.LOG_TYPE_SET) {
                if ((logType & logTypeCluster) == 0) {
                    continue;
                }
                if (logType == Utils.LOG_TYPE_MET) {
                    continue;
                }
                mDefaultSharedPreferences.edit()
                        .putString(Utils.KEY_LOG_SIZE_MAP.get(logType), newValueStr).apply();
                LogControllerUtils.getLogControllerInstance(logType).setLogRecycleSize(newLogSize);
            }

        } else if (command != null
                && command.startsWith(Utils.ADB_COMMAND_SET_TOTAL_LOG_SIZE_PREFIX)) {
            if (logTypeCluster != Utils.LOG_TYPE_MOBILE) {
                Utils.logw(TAG, "Only mobile log support for setMobileLogTotalRecycleSize."
                        + " logTypeCluster = " + logTypeCluster);
                return;
            }
            String newValueStr =
                    command.substring(Utils.ADB_COMMAND_SET_TOTAL_LOG_SIZE_PREFIX.length());
            int newLogSize = 0;
            try {
                newLogSize = Integer.parseInt(newValueStr);
            } catch (NumberFormatException e) {
                Utils.loge(TAG, "Invalid set total log size parameter: " + newValueStr);
                return;
            }
            if (newLogSize <= 0) {
                Utils.loge(TAG,
                        "Given total log size should bigger than zero, but got " + newValueStr);
                return;
            }
            mDefaultSharedPreferences.edit()
                    .putString(Utils.KEY_TOTAL_LOG_SIZE_MAP.get(Utils.LOG_TYPE_MOBILE), newValueStr)
                    .apply();
            MobileLogController.getInstance().setMobileLogTotalRecycleSize(newLogSize);
        } else if (command != null
                && command.startsWith(Utils.ADB_COMMAND_SET_NOTIFICATION_ENABLE)) {
            // adb shell am broadcast -a com.debug.loggerui.ADB_CMD
            // -e cmd_name show_notification_1 --ei cmd_target -1
            String newValue = command.substring(command.length() - 1);
            if (newValue.equals("0") || newValue.equals("1")) {
                mDefaultSharedPreferences.edit()
                        .putBoolean(Utils.KEY_PREFERENCE_NOTIFICATION_ENABLED, newValue.equals("1"))
                        .apply();
                mServiceStatusManager.setNotificationsEnabledForPackage(newValue.equals("1"));
            } else {
                Utils.logw(TAG, "Unsupported set NOTIFICATION value");
            }
        } else if (command != null
                && command.equalsIgnoreCase(Utils.ADB_COMMAND_GET_MTKLOG_PATH_NAME)) {
            // adb shell am broadcast -a com.debug.loggerui.ADB_CMD
            // -e cmd_name get_mtklog_path
            String mtklogPath = Utils.getCurrentLogPath() + Utils.LOG_PATH_PARENT;
            Intent intent = new Intent(Utils.ACTION_LOGGERUI_BROADCAST_RESULT);
            intent.putExtra(Utils.EXTRA_RESULT_NAME, Utils.ADB_COMMAND_GET_MTKLOG_PATH_NAME);
            intent.putExtra(Utils.EXTRA_RESULT_VALUE, mtklogPath);
            Utils.sendBroadCast(intent);
            Utils.logd(TAG,
                    "Broadcast " + Utils.ACTION_LOGGERUI_BROADCAST_RESULT
                            + " is sent out with extra :" + Utils.EXTRA_RESULT_NAME + " = "
                            + Utils.ADB_COMMAND_GET_MTKLOG_PATH_NAME + ", "
                            + Utils.EXTRA_RESULT_VALUE + " = " + mtklogPath);
        } else if (command != null
                && command.equalsIgnoreCase(Utils.ADB_COMMAND_GET_LOG_RECYCLE_SIZE_NAME)) {
            // adb shell am broadcast -a com.debug.loggerui.ADB_CMD
            // -e cmd_name get_log_recycle_size --ei cmd_target typeLog
            String logRecycleSizeStr = "0";
            if (logTypeCluster == Utils.LOG_TYPE_MOBILE) {
                logRecycleSizeStr = mDefaultSharedPreferences.getString(
                        Utils.KEY_TOTAL_LOG_SIZE_MAP.get(logTypeCluster),
                        String.valueOf(Utils.DEFAULT_CONFIG_LOG_SIZE_MAP.get(logTypeCluster) * 2));
            } else {
                logRecycleSizeStr = mDefaultSharedPreferences.getString(
                        Utils.KEY_LOG_SIZE_MAP.get(logTypeCluster),
                        String.valueOf(Utils.DEFAULT_CONFIG_LOG_SIZE_MAP.get(logTypeCluster)));
            }
            Intent intent = new Intent(Utils.ACTION_LOGGERUI_BROADCAST_RESULT);
            intent.putExtra(Utils.EXTRA_RESULT_NAME, Utils.ADB_COMMAND_GET_LOG_RECYCLE_SIZE_NAME);
            intent.putExtra(Utils.EXTRA_RESULT_VALUE, logRecycleSizeStr);
            Utils.sendBroadCast(intent);
            Utils.logd(TAG,
                    "Broadcast " + Utils.ACTION_LOGGERUI_BROADCAST_RESULT
                            + " is sent out with extra :" + Utils.EXTRA_RESULT_NAME + " = "
                            + Utils.ADB_COMMAND_GET_LOG_RECYCLE_SIZE_NAME + ", "
                            + Utils.EXTRA_RESULT_VALUE + " = " + logRecycleSizeStr);
        } else if (command != null
                && command.equalsIgnoreCase(Utils.ADB_COMMAND_GET_TAGLOG_STATUS_NAME)) {
            // adb shell am broadcast -a com.debug.loggerui.ADB_CMD
            // -e cmd_name get_taglog_status
            boolean isTaglogEnable = mSharedPreferences.getBoolean(Utils.TAG_LOG_ENABLE, false);
            Intent intent = new Intent(Utils.ACTION_LOGGERUI_BROADCAST_RESULT);
            intent.putExtra(Utils.EXTRA_RESULT_NAME, Utils.ADB_COMMAND_GET_TAGLOG_STATUS_NAME);
            intent.putExtra(Utils.EXTRA_RESULT_VALUE, isTaglogEnable);
            Utils.sendBroadCast(intent);
            Utils.logd(TAG,
                    "Broadcast " + Utils.ACTION_LOGGERUI_BROADCAST_RESULT
                            + " is sent out with extra :" + Utils.EXTRA_RESULT_NAME + " = "
                            + Utils.ADB_COMMAND_GET_TAGLOG_STATUS_NAME + ", "
                            + Utils.EXTRA_RESULT_VALUE + " = " + isTaglogEnable);
        } else if (command != null
                && command.equalsIgnoreCase(Utils.ADB_COMMAND_GET_LOG_AUTO_STATUS_NAME)) {
            // adb shell am broadcast -a com.debug.loggerui.ADB_CMD
            // -e cmd_name get_log_auto_status --ei cmd_target typeLog
            boolean isLogAutoStart = mDefaultSharedPreferences
                    .getBoolean(Utils.KEY_START_AUTOMATIC_MAP.get(logTypeCluster), false);
            Intent intent = new Intent(Utils.ACTION_LOGGERUI_BROADCAST_RESULT);
            intent.putExtra(Utils.EXTRA_RESULT_NAME, Utils.ADB_COMMAND_GET_LOG_AUTO_STATUS_NAME);
            intent.putExtra(Utils.EXTRA_RESULT_VALUE, isLogAutoStart);
            Utils.sendBroadCast(intent);
            Utils.logd(TAG,
                    "Broadcast " + Utils.ACTION_LOGGERUI_BROADCAST_RESULT
                            + " is sent out with extra :" + Utils.EXTRA_RESULT_NAME + " = "
                            + Utils.ADB_COMMAND_GET_LOG_AUTO_STATUS_NAME + ", "
                            + Utils.EXTRA_RESULT_VALUE + " = " + isLogAutoStart);
        } else if (command != null
                && command.startsWith(Utils.ADB_COMMAND_CLEAR_ALL_LOGS_NAME)) {
            // adb shell am broadcast -a com.debug.loggerui.ADB_CMD
            // -e cmd_name clear_all_logs
           LogControllerUtils.clearAllLogs();
        } else if (command != null
                && command.startsWith(Utils.ADB_COMMAND_CLEAR_LOGS_NAME)) {
            // adb shell am broadcast -a com.debug.loggerui.ADB_CMD
            // -e cmd_name clear_logs_all/detailLogPath cmd_target 0
            if (command.length() <= (Utils.ADB_COMMAND_CLEAR_LOGS_NAME.length() + 1)) {
                Utils.logw(TAG, "The format for adb command "
                        + Utils.ADB_COMMAND_CLEAR_LOGS_NAME + " is error!");
                return;
            }
            String clearLogPath =
                    command.substring(Utils.ADB_COMMAND_CLEAR_LOGS_NAME.length() + 1);
            if (clearLogPath.equalsIgnoreCase(Utils.ADB_COMMAND_CLEAR_LOGS_VALUE_ALL)) {
                LogControllerUtils.clearAllLogs();
                return;
            }
            File clearLogFile = new File(clearLogPath);
            if (clearLogFile.exists()) {
                LogControllerUtils.clearLogs(clearLogFile);
            }
        } else {
            // Other case-special command, let each log instance to handle it
            for (int logType : Utils.LOG_TYPE_SET) {
                if ((logType & logTypeCluster) == 0) {
                    continue;
                }
                Utils.logd(TAG, "Send adb command [" + command + "] to log " + logType);
                LogControllerUtils.getLogControllerInstance(logType).dealWithADBCommand(command);
            }
        }
    }

    /**
     * After send a command to native layer, this handler will monitor response from native, or
     * timeout signal. It will also monitor each log instance's self-driven change from native
     */
    class ServiceHandler extends Handler {

        public ServiceHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            int what = msg.what;
            Utils.logi(TAG, " mNativeStateHandler receive message," + " what=" + what + ", arg1="
                    + msg.arg1 + ", arg2=" + msg.arg2);
            if (what == Utils.MSG_LOG_STATE_CHANGED) {
                // start/stop command finish or self state change event
                // No use any more
            } else if (what == Utils.MSG_RUNNING_STAGE_CHANGE) {
                int stageEvent = msg.arg1;
                handleGlobalRunningStageChange(stageEvent);
            } else if (what == Utils.MSG_START_LOGS_DONE) {
                int logType = msg.arg1;
                if ((logType & mStartLogCluster) != 0) {
                    mStartLogCluster = mStartLogCluster ^ logType;
                }
            } else if (what == Utils.MSG_STOP_LOGS_DONE) {
                int logType = msg.arg1;
                if ((logType & mStopLogCluster) != 0) {
                    mStopLogCluster = mStopLogCluster ^ logType;
                }
            } else if (what == Utils.MSG_RESTART_DONE) {
                int logType = msg.arg1;
                mRestartLogCluster = mRestartLogCluster ^ logType;
            } else if (what == Utils.MSG_CLEAR_ALL_LOGS_DONE) {
                int result = msg.arg1;
                Intent intent = new Intent(Utils.ACTION_LOGGERUI_BROADCAST_RESULT);
                intent.putExtra(Utils.EXTRA_RESULT_NAME, Utils.ADB_COMMAND_CLEAR_ALL_LOGS_NAME);
                intent.putExtra(Utils.EXTRA_RESULT_VALUE, result);
                Utils.sendBroadCast(intent);
                Utils.logd(TAG,
                        "Broadcast " + Utils.ACTION_LOGGERUI_BROADCAST_RESULT
                                + " is sent out with extra :" + Utils.EXTRA_RESULT_NAME + " = "
                                + Utils.ADB_COMMAND_CLEAR_ALL_LOGS_NAME + ", "
                                + Utils.EXTRA_RESULT_VALUE + " = " + result);
            } else if (what == Utils.MSG_STOP_NETLOG_AT_SHUT_DOWN) {
                NetworkLogController.getInstance().dumpNetwork();
            } else if (what == Utils.MSG_START_BTLOG) {
                startBTLog();
            } else if (what == Utils.MSG_INIT_LOGS_FOR_SERVER_FIRST_START) {
                initLogsForServiceFirstStart();
            } else {
                Utils.loge(TAG, "Unknown message");
                return;
            }
        }
    }

    private Object mLock = new Object();
    /**
     * The service's global running stage have changed, like try to start/stop log, begin memory
     * dump.
     */
    private void handleGlobalRunningStageChange(int stageEvent) {
        Utils.logd(TAG, "-->handleGlobalRunningStageChange(), stageEvent=" + stageEvent
                + ", 1:start; 2:stop; 3:polling; 4:polling done.");
        mGlobalRunningStage = stageEvent;
        Intent intent = new Intent(Utils.EXTRA_RUNNING_STAGE_CHANGE_EVENT);
        intent.putExtra(Utils.EXTRA_RUNNING_STAGE_CHANGE_VALUE, stageEvent);
        Utils.sendBroadCast(intent);
    }

    /**
     * @return int
     */
    public int getCurrentRunningStage() {
        int stage = mGlobalRunningStage;
        ModemLogStatus modemLogStatus = ModemLogController.getInstance().getLogStatus();
        int modemLogStage = Utils.RUNNING_STAGE_IDLE;
        switch (modemLogStatus) {
        case POLLING:
            modemLogStage = Utils.RUNNING_STAGE_POLLING_LOG;
            break;
        default:
            modemLogStage = Utils.RUNNING_STAGE_IDLE;
        }
        if (modemLogStage > stage) {
            stage = modemLogStage;
        }
        Utils.logd(TAG, "<--getGlobalRunningStage(), current stage=" + stage);
        return stage;
    }

    /**
     * Mark DebugLoggerUI start time which is stored in shared preference.
     */
    private void updateStartRecordingTime(long time) {
        Utils.logd(TAG, "-->updateStartRecordingTime(), time=" + time);
        mSharedPreferences.edit().putLong(Utils.KEY_BEGIN_RECORDING_TIME, time).apply();
        mServiceStatusManager.updateNotificationTime();
    }

    /**
     * Mark DebugLoggerUI end time which is stored in shared preference.
     */
    private void updateStopRecordingTime(long time) {
        Utils.logd(TAG, "-->updateEndRecordingTime(), time=" + time);
        mSharedPreferences.edit().putLong(Utils.KEY_END_RECORDING_TIME, time).apply();
        mServiceStatusManager.updateNotificationTime();
    }

    /**
     * Function for judging is any log being running right now.
     *
     * @return boolean
     */
    private boolean isAnyLogRunning() {
        boolean isRunning = false;
        for (Integer logType : Utils.LOG_TYPE_SET) {
            if (LogControllerUtils.getLogControllerInstance(
                    logType).isLogRunning()) {
                isRunning = true;
                break;
            }
        }
        Utils.logv(TAG, "<--isAnyLogRunning()? " + isRunning);
        return isRunning;
    }

    /**
     * Trigger tag log process, called from UI, so treat this as user trigger.
     *
     * @param tagString
     *            String
     * @return boolean
     */
    public boolean beginTagLog(String tagString) {
        Utils.logi(TAG, "-->beginTagLog(), tagString=" + tagString);
        Intent intent = new Intent();
        intent.putExtra(Utils.EXTRA_KEY_EXP_PATH, Utils.MANUAL_SAVE_LOG);
        intent.putExtra(Utils.EXTRA_KEY_EXP_NAME, tagString);
        intent.putExtra(Utils.EXTRA_KEY_TAG_TYPE, TagLogUtils.TAG_MANUAL);
        intent.putExtra(Utils.EXTRA_KEY_EXP_TIME, TagLogUtils.getCurrentTimeString());
        TagLogManager.getInstance().startNewTaglog(intent);
        return true;
    }

    private int filterFeatureSupportLogs(int logTypes) {
        int featureSupportLogs = 0;
        for (int logType : Utils.LOG_TYPE_SET) {
            if (logTypes != Utils.LOG_TYPE_ALL && (logType & logTypes) == 0) {
                continue;
            }
            if (LogControllerUtils.getLogControllerInstance(logType).isLogFeatureSupport()) {
                featureSupportLogs |= logType;
            }
        }
        return featureSupportLogs;
    }

    private int mStartLogCluster = 0;
    /**
     * @param logTypeCluster
     *            The type of log.
     * @param reason
     *            The reason for start of log.
     * @return boolean
     */
    synchronized public boolean startRecording(int logTypeCluster, String reason) {
        Utils.logi(TAG,
                "-->startRecording(), logTypeCluster=" + logTypeCluster + ", reason=" + reason);
        if (!Utils.isDeviceOwner()) {
            Utils.logi(TAG, "It is not device owner, do not start logs!");
            return true;
        }
        mServiceStatusManager.statusChanged(
                           ServiceStatusManager.ServiceStatus.LOG_STARTING);

        logTypeCluster = filterFeatureSupportLogs(logTypeCluster);

        boolean result = true;
        mStartLogCluster = logTypeCluster;
        handleGlobalRunningStageChange(Utils.RUNNING_STAGE_STARTING_LOG);

        // For is need restart record time
        int logTypeAffect = 0;
        for (int logType : Utils.LOG_TYPE_SET) {
            if ((logType & mStartLogCluster) == 0) {
                continue;
            }
            if (!LogControllerUtils.getLogControllerInstance(logType).isLogRunning()) {
                logTypeAffect |= logType;
            }
        }
        Utils.logd(TAG, "startRecording(), logTypeAffect=" + logTypeAffect);

        boolean isAnyLogRunningBeforeDoStart = isAnyLogRunning();
        // First start mobile log
        if ((Utils.LOG_TYPE_MOBILE & mStartLogCluster) != 0) {
            result = LogControllerUtils.getLogControllerInstance(Utils.LOG_TYPE_MOBILE)
                    .startLog(Utils.getCurrentLogPath());
            mStartLogCluster ^= Utils.LOG_TYPE_MOBILE;
        }

        if (mStartLogCluster > 0) {
            result = MultiLogTypesController.getInstance().startTypeLogs(mStartLogCluster,
                    Utils.getCurrentLogPath());
        }
        mStartLogCluster = 0;
        if (Utils.SERVICE_STARTUP_TYPE_BOOT.equals(reason) || logTypeAffect != 0) {
            // During boot start, system performance will be Poor,
            // So sleep 3 seconds to wait log status really ready.
            try {
                Thread.sleep(3000);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        if (Utils.SERVICE_STARTUP_TYPE_BOOT.equals(reason)
                || (!isAnyLogRunningBeforeDoStart && isAnyLogRunning())) {
            updateStartRecordingTime(SystemClock.elapsedRealtime());
        }
        mServiceStatusManager.statusChanged(
                ServiceStatusManager.ServiceStatus.LOG_STARTING_DONE);
        handleGlobalRunningStageChange(Utils.RUNNING_STAGE_IDLE);
        Intent intent = new Intent(Utils.ACTION_LOG_STATE_CHANGED);
        intent.putExtra(Utils.EXTRA_AFFECTED_LOG_TYPE, logTypeCluster);
        intent.putExtra(Utils.EXTRA_LOG_NEW_STATE, (result ? 1 : 0));
        Utils.sendBroadCast(intent);
        // add for new broadcast
        int sucessLogTypes = getStartStopSuccessLogType(logTypeCluster, true);
        int failLogTypes = logTypeCluster ^ sucessLogTypes;

        Intent startintent = new Intent(Utils.ACTION_LOG_START_DONE);
        startintent.putExtra(Utils.EXTRA_RESULT, (result ? 1 : 0));
        startintent.putExtra(Utils.EXTRA_SUCESS_LOG_TYPES, sucessLogTypes);
        startintent.putExtra(Utils.EXTRA_FAIL_LOG_TYPES, failLogTypes);
        Utils.sendBroadCast(startintent);

        updateLogFolderMonitor();
        Utils.logi(TAG, "<--startRecordingDone(), result = " + result + ", sucessType = "
                + sucessLogTypes + ", failType = " + failLogTypes);

        return result;
    }
    private int getStartStopSuccessLogType(int logTypeCluser, boolean isRunning) {
        int sucessLogType = 0;
        for (int logType : Utils.LOG_TYPE_SET) {
            if ((logType & logTypeCluser) == 0) {
                continue;
            }
            if (isRunning == LogControllerUtils.getLogControllerInstance(logType).isLogRunning()) {
                sucessLogType |= logType;
            }
        }
        return sucessLogType;
    }

    private int mStopLogCluster = 0;

    /**
     * @param logTypeCluster
     *            The type of log.
     * @param reason
     *            The reason for stop of log.
     * @return boolean
     */
    synchronized public boolean stopRecording(int logTypeCluster, String reason) {
        Utils.logi(TAG,
                "-->stopRecording(), logTypeCluster=" + logTypeCluster + ", reason=" + reason);
        if (!Utils.isDeviceOwner()) {
            Utils.logi(TAG, "It is not device owner, do not stop logs!");
            return true;
        }
        mServiceStatusManager.statusChanged(
                ServiceStatusManager.ServiceStatus.LOG_STOPPING);
        logTypeCluster = filterFeatureSupportLogs(logTypeCluster);

        boolean isAnyLogRunningBeforeDoStop = isAnyLogRunning();

        boolean result = true;
        mStopLogCluster = logTypeCluster;
        handleGlobalRunningStageChange(Utils.RUNNING_STAGE_STOPPING_LOG);

        if (mStopLogCluster > 0) {
            result = MultiLogTypesController.getInstance()
                    .stopTypeLogs(((Utils.LOG_TYPE_MOBILE & mStopLogCluster) == 0)
                                ? mStopLogCluster : mStopLogCluster ^ Utils.LOG_TYPE_MOBILE);
        }
        if ((Utils.LOG_TYPE_MOBILE & mStopLogCluster) != 0) {
            result = LogControllerUtils.getLogControllerInstance(Utils.LOG_TYPE_MOBILE).stopLog();
            mStopLogCluster ^= Utils.LOG_TYPE_MOBILE;
        }
        mStopLogCluster = 0;
        mServiceStatusManager.statusChanged(
                ServiceStatusManager.ServiceStatus.LOG_STOPPING_DONE);

        if (isAnyLogRunningBeforeDoStop && !isAnyLogRunning()) {
            updateStopRecordingTime(SystemClock.elapsedRealtime());
        }
        handleGlobalRunningStageChange(Utils.RUNNING_STAGE_IDLE);
        Intent intent = new Intent(Utils.ACTION_LOG_STATE_CHANGED);
        intent.putExtra(Utils.EXTRA_AFFECTED_LOG_TYPE, logTypeCluster);
        intent.putExtra(Utils.EXTRA_LOG_NEW_STATE, (result ? 0 : 1));
        Utils.sendBroadCast(intent);
        // add for new broadcast
        int sucessLogTypes = getStartStopSuccessLogType(logTypeCluster, false);
        int failLogTypes = logTypeCluster ^ sucessLogTypes;

        Intent stopintent = new Intent(Utils.ACTION_LOG_STOP_DONE);
        stopintent.putExtra(Utils.EXTRA_RESULT, (result ? 1 : 0));
        stopintent.putExtra(Utils.EXTRA_SUCESS_LOG_TYPES, sucessLogTypes);
        stopintent.putExtra(Utils.EXTRA_FAIL_LOG_TYPES, failLogTypes);
        Utils.sendBroadCast(stopintent);

        updateLogFolderMonitor();
        if (Utils.LOG_START_STOP_REASON_FROM_UI.equals(reason)) {
            Utils.updateLogFilesInMediaProvider();
        }
        Utils.logi(TAG, "<--stopRecording(), result=" + result + ", sucessType = " + sucessLogTypes
                + ", failType = " + failLogTypes);
        return result;
    }

    private int mRestartLogCluster = 0;

    /**
     * @param logTypeCluster
     *            The type of log.
     * @param reason
     *            The reason for stop of log.
     * @return boolean
     */
    synchronized public boolean restartRecording(int logTypeCluster, String reason) {
        Utils.logi(TAG,
                "-->restartRecording(), logTypeCluster=" + logTypeCluster + ", reason=" + reason);
        boolean result = true;
        logTypeCluster = filterFeatureSupportLogs(logTypeCluster);
        mRestartLogCluster = logTypeCluster;

        Utils.logd(TAG, "restartRecording() affectLogCluster = " + mRestartLogCluster);
        if (mRestartLogCluster <= 0) {
            Intent intent = new Intent(Utils.ACTION_LOGGERUI_BROADCAST_RESULT);
            intent.putExtra(Utils.EXTRA_RESULT_NAME, Utils.ADB_COMMAND_RESTART);
            intent.putExtra(Utils.EXTRA_RESULT_VALUE, result ? 1 : 0);
            Utils.sendBroadCast(intent);
            return true;
        }
        mServiceStatusManager.statusChanged(
                ServiceStatusManager.ServiceStatus.LOG_RESTARTING);
        handleGlobalRunningStageChange(Utils.RUNNING_STAGE_RESTARTING_LOG);
        result = MultiLogTypesController.getInstance().rebootTypeLogs(mRestartLogCluster,
                Utils.getCurrentLogPath());
        mServiceStatusManager.statusChanged(
                ServiceStatusManager.ServiceStatus.LOG_RESTART_DONE);
        mRestartLogCluster = 0;
        Intent intent = new Intent(Utils.ACTION_LOGGERUI_BROADCAST_RESULT);
        intent.putExtra(Utils.EXTRA_RESULT_NAME, Utils.ADB_COMMAND_RESTART);
        intent.putExtra(Utils.EXTRA_RESULT_VALUE, result ? 1 : 0);
        Utils.sendBroadCast(intent);
        Utils.logd(TAG,
                "Broadcast " + Utils.ACTION_LOGGERUI_BROADCAST_RESULT + " is sent out with extra :"
                        + Utils.EXTRA_RESULT_NAME + " = " + Utils.ADB_COMMAND_RESTART + ", "
                        + Utils.EXTRA_RESULT_VALUE + " = " + (result ? 1 : 0));

        // add for new broadcast
        int sucessLogTypes = getStartStopSuccessLogType(logTypeCluster, true);
        int failLogTypes = logTypeCluster ^ sucessLogTypes;

        Intent rebootintent = new Intent(Utils.ACTION_LOG_REBOOT_DONE);
        rebootintent.putExtra(Utils.EXTRA_RESULT, (result ? 1 : 0));
        rebootintent.putExtra(Utils.EXTRA_SUCESS_LOG_TYPES, sucessLogTypes);
        rebootintent.putExtra(Utils.EXTRA_FAIL_LOG_TYPES, failLogTypes);
        Utils.sendBroadCast(rebootintent);

        handleGlobalRunningStageChange(Utils.RUNNING_STAGE_IDLE);
        Intent logStateChangedIntent = new Intent(Utils.ACTION_LOG_STATE_CHANGED);
        logStateChangedIntent.putExtra(Utils.EXTRA_AFFECTED_LOG_TYPE, logTypeCluster);
        logStateChangedIntent.putExtra(Utils.EXTRA_LOG_NEW_STATE, (result ? 1 : 0));
        Utils.sendBroadCast(logStateChangedIntent);
        Utils.logd(TAG,
                "Broadcast " + Utils.ACTION_LOG_STATE_CHANGED + " is sent out with extra :"
                        + Utils.EXTRA_AFFECTED_LOG_TYPE + " = " + logTypeCluster + ", "
                        + Utils.EXTRA_LOG_NEW_STATE + " = " + (result ? 1 : 0));

        Utils.logi(TAG, "<--restartRecording(), result =  " + result + ", sucessLogTypes = "
                + sucessLogTypes + ", failLogTypes = " + failLogTypes);
        return result;
    }
}
