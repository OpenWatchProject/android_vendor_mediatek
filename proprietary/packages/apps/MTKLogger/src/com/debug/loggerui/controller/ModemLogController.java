package com.debug.loggerui.controller;

import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.view.WindowManager;

import com.debug.loggerui.MyApplication;
import com.debug.loggerui.R;
import com.debug.loggerui.framework.C2KLoggerService;
import com.debug.loggerui.settings.ModemLogSettings;
import com.debug.loggerui.taglog.TagLogUtils;
import com.debug.loggerui.utils.Utils;
import com.log.handler.LogHandler;
import com.log.handler.LogHandlerUtils.IModemEEMonitor;
import com.log.handler.LogHandlerUtils.LogType;
import com.log.handler.LogHandlerUtils.ModemLogMode;
import com.log.handler.LogHandlerUtils.ModemLogStatus;

import dalvik.system.PathClassLoader;

import java.io.File;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

/**
 * @author MTK81255
 *
 */
public class ModemLogController extends AbstractLogController {
    private static final String TAG = Utils.TAG + "/ModemLogController";

    private Context mContext = MyApplication.getInstance().getApplicationContext();
    private SharedPreferences mSharedPreferences = MyApplication.getInstance()
            .getSharedPreferences(Utils.CONFIG_FILE_NAME, Context.MODE_PRIVATE);
    private static final String ADB_COMMAND_FORCE_MODEM_ASSERT = "force_modem_assert";
    private static final String ADB_COMMAND_MODEM_AUTO_RESET = "modem_auto_reset_";
    private static final String ADB_COMMAND_SET_FILE_SIZE = "set_file_size_";
    private static final String ADB_COMMAND_SET_GPS_LOCATION_ENABLE = "set_gps_location_";
    private static final String ADB_COMMAND_SET_CCB_BUFFER_GEAR_ID = "set_ccb_buffer_gear_id_";

    private Handler mHandler;
    private static final int MSG_MODEM_DUMP_START = 0;
    private static final int MSG_MODEM_DUMP_FINISH = 1;
    private boolean mIsModemResetDialogShowing = false;
    private static final String MEMORYDUMP_FILE = "MEMORYDUMP_FILE";
    private C2KLogger mC2KLogger;
    private static int sModemSelfKeyLogSize = 5;
    private String mLastTimeReceiveDumpStart = "";

    private static ModemLogController sInstance = new ModemLogController(LogType.MODEM_LOG);

    public static ModemLogController getInstance() {
        return sInstance;
    }

    protected ModemLogController(LogType logType) {
        super(logType);
        HandlerThread handlerThread = new HandlerThread("modemlogHandler");
        handlerThread.start();
        mHandler = new ModemLogHandler(handlerThread.getLooper());
        if (Utils.isDeviceOwner()) {
            LogHandler.getInstance().registerModemEEMonitor(mModemEEMonitor);
        }
        mC2KLogger = new C2KLogger();
        setKeyLogBufferSize(sModemSelfKeyLogSize);
    }

    /**
     * @author MTK81255
     *
     */
    class ModemLogHandler extends Handler {
        ModemLogHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case MSG_MODEM_DUMP_START:
                receiveDumpStart();
                break;
            case MSG_MODEM_DUMP_FINISH:
                String modemEEPath = "";
                if (msg.obj != null && msg.obj instanceof String) {
                    modemEEPath = (String) msg.obj;
                    showMemoryDumpDoneDialog(modemEEPath);
                } else {
                    Utils.logw(TAG, "The modemEEPath is null or formart is error!");
                }
                addSelfKeyLogToBuffer("Receive modem EE dump message : MEMORYDUMP_DONE, path = "
                                      + modemEEPath);
                break;
            default:
                Utils.logw(TAG, "The msg.what = " + msg.what + " is not support!");
            }
        }
    }

    private IModemEEMonitor mModemEEMonitor = new IModemEEMonitor() {
        @Override
        public void modemEEHappened(String modemResponse) {
            if (modemResponse.equals("need_dump_file")) {
                // need to do check wheter need dump file and then send to mdlogger.
                // mdlogger will wait this message 2 seconds, if time out. not send dump file.
                String cmd = "need_dump_file,1"; // need dump file
                //cmd ="need_dump_file,0" // not need dump file
                Utils.logi(TAG, "need_dump_file = " + modemResponse);
                LogHandler.getInstance().sendCommandToModemLog(cmd);
                return;
            } else if (modemResponse.startsWith(MEMORYDUMP_FILE)) {
                String modemDumpFile =
                    modemResponse.substring(MEMORYDUMP_FILE.length() + 1);
                Utils.logi(TAG, "MEMORYDUMP_FILE = " + modemDumpFile);
                // customize here
                return;
            } else if (modemResponse.startsWith("MEMORYDUMP_START")) {
                Message msg = mHandler.obtainMessage(MSG_MODEM_DUMP_START);
                msg.sendToTarget();
            } else {
                Message msg = mHandler.obtainMessage(MSG_MODEM_DUMP_FINISH);
                msg.obj = modemResponse;
                msg.sendToTarget();
            }
        }
    };

    private void showMemoryDumpDoneDialog(String modemEEPath) {
        Utils.logi(TAG, "-->showMemoryDumpDone(), modemEEPath = " + modemEEPath
                + ", isModemResetDialogShowing=" + mIsModemResetDialogShowing);
        if (Utils.isDenaliMd3Solution()
                && (modemEEPath.contains(Utils.C2K_MODEM_EXCEPTION_LOG_PATH))) {
            mSharedPreferences.edit()
                    .putString(Utils.KEY_C2K_MODEM_EXCEPTIONG_PATH,
                            mSharedPreferences.getString(Utils.KEY_C2K_MODEM_LOGGING_PATH, ""))
                    .apply();
            mC2KLogger.startStopC2KLoggerService(false);
        }

        if (mDefaultSharedPreferences.getBoolean(ModemLogSettings.KEY_MD_AUTORESET, false)) {
            Utils.logi(TAG, "Auto reset modem, does not need showMemoryDumpDoneDialog!");
            resetModem();
            mIsModemResetDialogShowing = false;
            return;
        }
        if (mIsModemResetDialogShowing) {
            Utils.logd(TAG, "Modem reset dialog is already showing, just return");
            return;
        }
        Utils.logi(TAG, "Show memory dump done dialog.");
        Context context = MyApplication.getInstance().getApplicationContext();
        String message = context.getText(R.string.memorydump_done).toString() + modemEEPath;
        Builder builder = new AlertDialog.Builder(context)
                .setTitle(context.getText(R.string.dump_warning).toString()).setMessage(message)
                .setPositiveButton(android.R.string.yes, new OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        Utils.logd(TAG, "Click OK in memory dump done dialog");
                        resetModem();
                        Utils.logd(TAG, "After confirm, no need to show reset dialog next time");
                        mIsModemResetDialogShowing = false;
                        mC2KLogger.startStopC2KLoggerService(true);
                    }
                });
        AlertDialog dialog = builder.create();
        dialog.getWindow().setType(WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY);
        dialog.setOnCancelListener(new OnCancelListener() {
            @Override
            public void onCancel(DialogInterface dialog) {
                Utils.logd(TAG, "Press cancel in memory dump done dialog");
                resetModem();
                Utils.logd(TAG, "After cancel, no need to show reset dialog next time");
                mIsModemResetDialogShowing = false;
                mC2KLogger.startStopC2KLoggerService(true);
            }
        });
        mIsModemResetDialogShowing = true;
        dialog.show();
    }

    private void resetModem() {
        boolean isModemLogAutoResetEE = mDefaultSharedPreferences
                .getBoolean(Utils.KEY_PREFERENCE_MODEMLOG_AUTO_RESET_MODEM, false);
        if (isModemLogAutoResetEE) {
            Utils.logi(TAG, "ModemLog Daemon will do auto reset modem after EE,"
                    + " there is no need to send reset command to ModemLog daemon!");
            return;
        }
        LogHandler.getInstance().resetModem();
    }

    /**
     * @param logPath
     *            String
     * @return boolean
     */
    public boolean reconnectToModemLog(String logPath) {
        Utils.logi(TAG, "reconnectToModemLog logPath = " + logPath);
        if (!isLogRunning() || !isLogControlled()) {
            Utils.logi(TAG, "modem log reconnect, but log status stop or not controlled,"
                    + " no need send start command!");
            return true;
        }
        return startLog(logPath);
    }

    @Override
    protected boolean starTypeLog(String logPath) {
        mC2KLogger.startStopC2KLoggerService(true);
        return LogHandler.getInstance().startModemLog(logPath,
                ModemLogMode.getModemLogModeById(getCurrentMode()));
    }

    @Override
    protected boolean stopTypeLog() {
        int modemLogStatus = ModemLogController.getInstance().getLogStatus().getId();
        if (ModemLogStatus.POLLING.getId() == modemLogStatus) {
            Utils.logw(TAG, "ignore mdlog stop command,"
                    + " because current status is " + modemLogStatus);
            return true;
        }
        mC2KLogger.startStopC2KLoggerService(false);
        return super.stopTypeLog();
    }

    @Override
    public boolean rebootLog(String logPath) {
        mC2KLogger.startStopC2KLoggerService(true);
        mC2KLogger.startStopC2KLoggerService(false);
        return super.rebootLog(logPath);
    }

    private String getCurrentMode() {
        String md1Mode = mDefaultSharedPreferences.
                         getString(Utils.KEY_MD_MODE_1, Utils.MODEM_MODE_SD);
        if (Utils.sAvailableModemList.size() == 1) {
            return md1Mode;
        }
        String md2Mode = mDefaultSharedPreferences.
                getString(Utils.KEY_MD_MODE_2, Utils.MODEM_MODE_SD);
        return md1Mode + "_" + md2Mode;
    }

    private boolean setCurrentMode(String newMode, String modemType) {
        Utils.logi(TAG, "-->setCurrentMode(), newMode=" + newMode + "; modemType=" + modemType);
        if (Utils.MODEM_MODE_IDLE.equals(newMode) || Utils.MODEM_MODE_USB.equals(newMode)
                || Utils.MODEM_MODE_SD.equals(newMode) || Utils.MODEM_MODE_PLS.equals(newMode)) {
            if (mDefaultSharedPreferences != null) {
                Utils.logv(TAG, "Persist new modem log mode");
                if (Utils.sAvailableModemList.size() == 1 || modemType.equals("1")) {
                    mDefaultSharedPreferences.edit().putString(Utils.KEY_MD_MODE_1, newMode)
                            .apply();
                } else {
                    mDefaultSharedPreferences.edit().putString(Utils.KEY_MD_MODE_2, newMode)
                            .apply();
                }
                return true;
            } else {
                Utils.loge(TAG, "mDefaultSharedPreferences is null");
            }
        } else {
            Utils.logw(TAG, "Unsupported log mode");
        }
        return false;
    }

    @Override
    public boolean dealWithADBCommand(String command) {
        if (command.startsWith(ADB_COMMAND_FORCE_MODEM_ASSERT)) {
            if (!Utils.MODEM_MODE_USB.equals(mDefaultSharedPreferences
                    .getString(Utils.KEY_MD_MODE_1, Utils.MODEM_MODE_SD))) {
                return LogHandler.getInstance().forceModemAssert();
            } else {
                Utils.logw(TAG, "In USB mode, force modem assert command is not supported");
                return true;
            }
        } else if (command.startsWith(Utils.ADB_COMMAND_SWITCH_MODEM_LOG_MODE)) {
            String[] tmpArray = command.split(",");
            if (tmpArray.length == 2) {
                String newModeStr = "";
                boolean setModeSuccess = false;
                if (tmpArray[0].startsWith(Utils.ADB_COMMAND_SWITCH_MODEM_LOG_MODE + "_")) {
                    newModeStr = tmpArray[0]
                            .substring(Utils.ADB_COMMAND_SWITCH_MODEM_LOG_MODE.length() + 1);
                    setModeSuccess = setCurrentMode(newModeStr, tmpArray[1]);
                } else {
                    newModeStr = tmpArray[1];
                    setModeSuccess = setCurrentMode(newModeStr, "1");
                    if (Utils.sAvailableModemList.size() == 2) {
                        setModeSuccess = setCurrentMode(newModeStr, "3");
                    }
                }
                if (setModeSuccess && getAutoStartValue()) {
                    setBootupLogSaved(true);
                }
                return true;
            } else {
                Utils.loge(TAG, "Invalid configuration from adb command");
                return true;
            }
        } else if (command.startsWith(Utils.ADB_COMMAND_SET_MODEM_LOG_SIZE)) {
            String[] tmpArray = command.split(",");
            if (tmpArray.length == 2) {
                String newlogSize =
                        tmpArray[0].substring(Utils.ADB_COMMAND_SET_MODEM_LOG_SIZE.length() + 1);
                int logSize = 600;
                try {
                    logSize = Integer.parseInt(newlogSize);
                } catch (NumberFormatException nfe) {
                    Utils.logw(TAG,
                            "The format for newlogSize is error! newlogSize = " + newlogSize);
                    logSize = 600;
                }
                return setLogRecycleSize(logSize);
            } else {
                Utils.loge(TAG, "Invalid mdlog size configuration from adb command");
                return true;
            }
        } else if (command.startsWith(ADB_COMMAND_MODEM_AUTO_RESET)) {
            String newValue = command.substring(command.length() - 1);
            if (newValue.equals("0") || newValue.equals("1")) {
                mDefaultSharedPreferences.edit()
                        .putBoolean(ModemLogSettings.KEY_MD_AUTORESET, newValue.equals("1"))
                        .apply();
            } else {
                Utils.logw(TAG, "Unsupported adb command modem_auto_reset value");
            }
            return true;
        } else if (command.startsWith(ADB_COMMAND_SET_FILE_SIZE)) {
            // adb shell am broadcast -a com.debug.loggerui.ADB_CMD -e cmd_name
            // set_file_size_200 --ei cmd_target 2
            String newLogFileSizeStr = command.substring(ADB_COMMAND_SET_FILE_SIZE.length());
            Utils.logi(TAG, "set_file_size_ newLogFileSizeStr = " + newLogFileSizeStr);
            int logFileSize = 200;
            try {
                logFileSize = Integer.parseInt(newLogFileSizeStr);
            } catch (NumberFormatException nfe) {
                logFileSize = 200;
            }
            if (logFileSize < 50) {
                Utils.logw(TAG, "The min size for modem log file is 50M.");
                logFileSize = 50;
            }
            if (logFileSize > 200) {
                Utils.logw(TAG, "The max size for modem log file is 200M.");
                logFileSize = 200;
            }
            return setModemLogFileSize(logFileSize);
        } else if (command.startsWith(ADB_COMMAND_SET_GPS_LOCATION_ENABLE)) {
            String newValue = command.substring(command.length() - 1);
            mDefaultSharedPreferences.edit()
                    .putBoolean(ModemLogSettings.KEY_MD_SAVE_LOCATIN_IN_LOG, newValue.equals("1"))
                    .apply();
            return setSaveGPSLocationToModemLog(newValue.equals("1"));
        } else if (command.startsWith(ADB_COMMAND_SET_CCB_BUFFER_GEAR_ID)) {
            String newValue = command.substring(command.length() - 1);
            mDefaultSharedPreferences.edit()
                    .putBoolean(
                            Utils.KEY_START_AUTOMATIC_MAP
                                    .get(LogControllerUtils.LOG_TYPE_OBJECT_TO_INT.get((mLogType))),
                            true)
                    .apply();
            setBootupLogSaved(true);
            mDefaultSharedPreferences.edit()
                    .putString(ModemLogSettings.KEY_CCB_GEAR, newValue).apply();
            return setCCBBufferGearID(newValue);
        } else {
            Utils.logw(TAG, "not official adb command:" + command);
            return LogHandler.getInstance().sendCommandToModemLog(command);
        }
    }

    private boolean getAutoStartValue() {
        boolean defaultValue = Utils.DEFAULT_CONFIG_LOG_AUTO_START_MAP.get(Utils.LOG_TYPE_MODEM);
        if (mDefaultSharedPreferences != null) {
            defaultValue = mDefaultSharedPreferences.getBoolean(Utils.KEY_START_AUTOMATIC_MODEM,
                    defaultValue);
        }
        Utils.logv(TAG, " getAutoStartValue(), value=" + defaultValue);
        return defaultValue;
    }

    @Override
    public boolean setBootupLogSaved(boolean enable) {
        if (enable != mDefaultSharedPreferences.getBoolean(Utils.KEY_START_AUTOMATIC_MODEM,
                false)) {
            mDefaultSharedPreferences.edit().putBoolean(Utils.KEY_START_AUTOMATIC_MODEM, enable)
                    .apply();
        }
        return LogHandler.getInstance().setBootupLogSaved(enable,
                ModemLogMode.getModemLogModeById(getCurrentMode()));
    }

    @Override
    public String getRunningLogPath() {
        Utils.logd(TAG, "-->getRunningLogPath(), mLogType = " + mLogType);
        if (!isLogRunning()) {
            return null;
        }
        if (Utils.sAvailableModemList.size() == 0) {
            return super.getRunningLogPath();
        }
        String runningLogPath = null;
        for (int modemIndex : Utils.sAvailableModemList) {
            if (modemIndex == (Utils.MODEM_LOG_K2_INDEX + Utils.C2KLOGGER_INDEX)
                    && Utils.isDenaliMd3Solution()) {
                String c2kModemPath = MyApplication.getInstance().getSharedPreferences()
                        .getString(Utils.KEY_C2K_MODEM_LOGGING_PATH, "");
                Utils.logd(TAG, "c2kModemExceptionPath = " + c2kModemPath);
                File c2kModemFile = new File(c2kModemPath);
                if (null != c2kModemFile && c2kModemFile.exists()) {
                    if (runningLogPath != null) {
                        runningLogPath += ";" + c2kModemFile.getAbsolutePath();
                    } else {
                        runningLogPath = c2kModemFile.getAbsolutePath();
                    }
                }
                continue;
            }
            String modemLogFolder =
                    Utils.MODEM_INDEX_FOLDER_MAP.get(modemIndex + Utils.MODEM_LOG_K2_INDEX);
            String logPath = Utils.getCurrentLogPath() + Utils.LOG_PATH_PARENT + modemLogFolder;

            File fileTree = new File(logPath + File.separator + Utils.LOG_TREE_FILE);
            File logFile = Utils.getLogFolderFromFileTree(fileTree, false);
            if (null != logFile && logFile.exists()) {
                if (runningLogPath != null) {
                    runningLogPath += ";" + logFile.getAbsolutePath();
                } else {
                    runningLogPath = logFile.getAbsolutePath();
                }
            }
        }
        Utils.logi(TAG, "<--getRunningLogPath(), runningLogPath = " + runningLogPath);
        return runningLogPath;
    }

    /**
     * Return the current running stage which may affect UI display behaviors, for example, if modem
     * log is dumping, the whole UI should be disabled. By default, detail log instance will not
     * affect the global UI
     *
     * @return ModemLogStatus
     */
    public ModemLogStatus getLogStatus() {
        if (!Utils.isDeviceOwner()) {
            return isLogRunning() ? ModemLogStatus.RUNNING : ModemLogStatus.PAUSE;
        }
        return LogHandler.getInstance().getModemLogStatus();
    }

    /**
     * @return boolean
     */
    public boolean notifyUSBModeChanged() {
        return LogHandler.getInstance().notifyUSBModeChanged();
    }

    /**
     * @param enable
     *            boolean
     * @return boolean
     */
    public boolean setSaveGPSLocationToModemLog(boolean enable) {
        return LogHandler.getInstance().setSaveGPSLocationToModemLog(enable);
    }

    public boolean isSaveGPSLocationFeatureSupport() {
        return LogHandler.getInstance().isSaveGPSLocationFeatureSupport();
    }

    public boolean isCCBBufferFeatureSupport() {
        return LogHandler.getInstance().isCCBBufferFeatureSupport();
    }

    /**
     * @return String
     */
    public String getCCBBufferConfigureList() {
        return LogHandler.getInstance().getCCBBufferConfigureList();
    }

    /**
     * @return String
     */
    public String getCCBBufferGearID() {
        return LogHandler.getInstance().getCCBBufferGearID();
    }

    /**
     * @param id
     *            CCBBufferGearID
     * @return boolean
     */
    public boolean setCCBBufferGearID(String id) {
        return LogHandler.getInstance().setCCBBufferGearID(id);
    }

    /**
     * @param size
     *            int
     * @return boolean
     */
    public boolean setModemLogFileSize(int size) {
        return LogHandler.getInstance().setModemLogFileSize(size);
    }

    /**
     * @return String
     */
    public String triggerPLSModeFlush() {
        return LogHandler.getInstance().triggerModemLogPLSModeFlush();
    }

    public String getFilterFileInformation() {
        return LogHandler.getInstance().getModemLogFilterFileInformation();
    }

    private void receiveDumpStart() {
        mLastTimeReceiveDumpStart = TagLogUtils.getCurrentTimeString();
        addSelfKeyLogToBuffer("Receive modem EE dump message : MEMORYDUMP_START");
    }
    /**
     * @param exceptTime String
     * @return boolean
     */
    public boolean isReceiveDumpStart(String exceptTime) {
        if (!mLastTimeReceiveDumpStart.isEmpty()
                && mLastTimeReceiveDumpStart.compareTo(exceptTime) >= 0) {
            Utils.logw(TAG, "isReceiveDumpStart ? true," + "exceptTime : " + exceptTime
                       + ",lastTimeReceiveDumpStart : " + mLastTimeReceiveDumpStart);
            return true;
        }
        return false;
    }

    /**
     * @author MTK81255
     *
     */
    class C2KLogger {

        public C2KLogger() {
        }

        public void startStopC2KLoggerService(boolean isStart) {
            Utils.logd(TAG, "StartStopC2KLoggerService isStart ? " + isStart);
            if (!Utils.isSupportC2KModem()) {
                Utils.logd(TAG, "The md3 is not support, so it is not support C2KModem!");
                return;
            }
            if (!Utils.isDenaliMd3Solution()) {
                Utils.logd(TAG, "It is not C2K modem solutin,"
                        + " so no need start/stop c2klogger service!");
                return;
            }
            String currentMode = getCurrentMode();
            if (Utils.MODEM_MODE_USB.equals(currentMode)) {
                Utils.logd(TAG, "Modem mode is USB, ignore C2KLogger!");
                return;
            }
            new Thread(new Runnable() {
                @Override
                public void run() {
                    Intent intent = new Intent();
                    intent.setClass(mContext, C2KLoggerService.class);
                    if (isStart) {
                        mSharedPreferences.edit().putString("InternalLogPath",
                                Utils.getLogPath(Utils.LOG_PATH_TYPE_DEVICE_STORAGE)).apply();
                        mSharedPreferences.edit().putString("ExternalLogPath",
                                Utils.getLogPath(Utils.LOG_PATH_TYPE_PORTABLE_STORAGE)).apply();
                        mSharedPreferences.edit()
                                .putString("ModemLogPath", Utils.getCurrentLogPath())
                                .apply();
                        if (isC2KLoggerRunning()) {
                            Utils.logw(TAG,
                                    "C2KLoggerService has been running,"
                                            + " try later for waiting 15s!");
                            try {
                                for (int i = 0; i < 15; i++) {
                                    Thread.sleep(1000);
                                    if (!isC2KLoggerRunning()) {
                                        Utils.logd(TAG, "Waiting stop C2KLoggerService for "
                                                + (i + 1) + " seconds");
                                        mContext.startService(intent);
                                        startStopC2KLoggerLibService(true);
                                        return;
                                    }
                                }
                                Utils.loge(TAG, "Waiting C2KLoggerService stop timeout!");
                                if (!isC2KLoggerRunning()) {
                                    mContext.startService(intent);
                                    startStopC2KLoggerLibService(true);
                                }
                            } catch (InterruptedException e) {
                                e.printStackTrace();
                                    }
                        } else {
                            mContext.startService(intent);
                            startStopC2KLoggerLibService(true);
                        }
                    } else {
                        if (!isC2KLoggerRunning()) {
                            Utils.logw(TAG, "C2KLoggerService has been destroyed");
                        } else {
                            mContext.stopService(intent);
                            startStopC2KLoggerLibService(false);
                        }
                    }
                }
            }).start();
        }

        private Class<?> mC2kLogService;

        /**
         * @param isStart
         *            boolean
         */
        private synchronized void startStopC2KLoggerLibService(boolean isStart) {
            Utils.logd(TAG, "startStopC2KLoggerLibService isStart ? " + isStart);
            try {
                if (mC2kLogService == null) {
                    mC2kLogService = getC2KLogService();
                    if (mC2kLogService == null) {
                        Utils.loge(TAG, "Load c2kloggerService failed!");
                        return;
                    }
                }
                Class<?>[] parameterTypes = new Class[] { Class.forName("android.app.Service") };
                Method getVolumeStateMethod = mC2kLogService.getDeclaredMethod(
                        isStart ? "startService" : "stopService", parameterTypes);
                int i = 0;
                while (C2KLoggerService.sService == null) {
                    try {
                        Utils.logw(TAG, "C2KLogService.sService is null,"
                                + " waiting it onCreate finished!");
                        Thread.sleep(200);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                    i++;
                    if (i >= 100) {
                        Utils.loge(TAG, "Start C2KLogService failed in 20s!");
                        return;
                    }
                }
                Object[] args = new Object[] { C2KLoggerService.sService };
                getVolumeStateMethod.invoke(null, args);
            } catch (ClassNotFoundException e) {
                e.printStackTrace();
            } catch (NoSuchMethodException e) {
                e.printStackTrace();
            } catch (IllegalAccessException e) {
                e.printStackTrace();
            } catch (IllegalArgumentException e) {
                e.printStackTrace();
            } catch (InvocationTargetException e) {
                e.printStackTrace();
            }
        }

        /**
         * @return Class<?>
         */
        private Class<?> getC2KLogService() {
            try {
                Utils.logd(TAG, "getC2KLoggerService Start!");
                String path = "/system/framework/via-plugin.jar";
                File c2kloggerLibFile = new File(path);
                if (!c2kloggerLibFile.exists()) {
                    return null;
                }
                String className = "com.debug.loggerui.c2klogger.C2KLoggerProxy";
                PathClassLoader classLoader =
                        new PathClassLoader(path, null, ClassLoader.getSystemClassLoader());
                return classLoader.loadClass(className);
            } catch (ClassNotFoundException e) {
                e.printStackTrace();
            }
            return null;
        }

        /**
         * @return boolean
         */
        private boolean isC2KLoggerRunning() {
            return Utils.isServiceRunning(mContext, C2KLoggerService.class.getName())
                    || !mSharedPreferences.getString(Utils.KEY_C2K_MODEM_LOGGING_PATH, "")
                            .isEmpty();
        }

    }
}
