package com.debug.loggerui.framework;

import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.Process;

import com.debug.loggerui.MyApplication;
import com.debug.loggerui.framework.DebugLoggerUIServiceManager.ServiceNullException;
import com.debug.loggerui.utils.Utils;


/**
 * @author MTK81255
 *
 */
public class ReceiverHandler extends Handler {
    private static final String TAG = Utils.TAG + "/ReceiverHandler";

    public static final int MSG_KILL_SELF = 1;
    public static final int MSG_RECEIVER_BOOT_COMPLETED = 2;
    public static final int MSG_RECEIVER_ADB_CMD = 3;
    public static final int MSG_RECEIVER_MDLOGGER_RESTART_DONE = 4;
    public static final int MSG_RECEIVER_EXP_HAPPENED = 5;
    public static final int MSG_RECEIVER_FROM_BYPASS = 6;
    // After receiver a kill self command, wait this time(ms) to avoid
    // duplicated kill in short time
    public static final int DELAY_KILL_SELF = 2000;
    public static final int DELAY_WAITING_BOOT_COMPLETE_DONE = 5000;

    /*
     * If true, the service will always start.
     */
    private static final boolean ALWAYS_START_SERVICE = false;
    private static ReceiverHandler sDefaultInstance;

    /**
     * @param looper Looper
     */
    public ReceiverHandler(Looper looper) {
        super(looper);
    }

    /**
     * @return ReceiverHandler
     */
    public static ReceiverHandler getDefaultInstance() {
        if (sDefaultInstance == null) {
            synchronized (ReceiverHandler.class) {
                if (sDefaultInstance == null) {
                    HandlerThread handlerThread = new HandlerThread("ReceiverHandlerThread");
                    handlerThread.start();
                    sDefaultInstance = new ReceiverHandler(handlerThread.getLooper());
                }
            }
        }
        return sDefaultInstance;
    }

    private SharedPreferences mDefaultSharedPreferences =
            MyApplication.getInstance().getDefaultSharedPreferences();

    @Override
    public void handleMessage(Message msg) {
        int what = msg.what;
        if (what == MSG_KILL_SELF) {
            Utils.logi(TAG, "Get a self-kill command. Need to kill me now");
            if (!DebugLoggerUIServiceManager.getInstance().isServiceUsed()) {
                Process.killProcess(Process.myPid());
            } else {
                Utils.logi(TAG, "But Log service was started already, maybe user enter UI."
                        + "Do not kill self any more.");
            }
            return;
        }
        try {
            Intent intent = new Intent();
            if (msg.obj instanceof Intent) {
                intent = (Intent) msg.obj;
            }
            if (what == MSG_RECEIVER_BOOT_COMPLETED) {
                dealWithBootcomplete(intent);
            } else if (what == MSG_RECEIVER_ADB_CMD) {
                if (!Utils.isDeviceOwner()) {
                    Utils.logi(TAG, "It is not device owner, ignore dealWithADBCommand()");
                    return;
                }
                dealWithADBCommand(intent);
            } else if (what == MSG_RECEIVER_MDLOGGER_RESTART_DONE) {
                dealWithMDLoggerRestart(intent);
            } else if (what == MSG_RECEIVER_EXP_HAPPENED) {
                dealWithExcptionHappend(intent);
            } else if (what == MSG_RECEIVER_FROM_BYPASS) {
                dealWithBypassAction(intent);
            }
        } catch (ServiceNullException e) {
            return;
        }
    };

    private void dealWithBootcomplete(Intent intent) throws ServiceNullException {
        LogConfig.getInstance().checkConfig();
        // Now start log service or just remove log process manually
        if (Utils.isTaglogEnable() || needStartLogAtBootTime() || ALWAYS_START_SERVICE) {
            if (SecurityWarning.isNeedAlert()) {
                SecurityWarning.DialogExcute dialogExcute = new SecurityWarning.DialogExcute() {
                    @Override
                    public void okButtonClicked() {
                        DebugLoggerUIServiceManager.getInstance().initService();
                    }

                    @Override
                    public void cancelButtonClicked() {
                        getDefaultInstance().removeMessages(MSG_KILL_SELF);
                        getDefaultInstance().sendMessageDelayed(
                                getDefaultInstance().obtainMessage(MSG_KILL_SELF), DELAY_KILL_SELF);
                    }

                };
                SecurityWarning.getInstance().getAlertHander()
                        .obtainMessage(SecurityWarning.MSG_BOOT_COMPLETE, dialogExcute)
                        .sendToTarget();
            } else {
                DebugLoggerUIServiceManager.getInstance().initService();
            }
        } else {
            getDefaultInstance().removeMessages(MSG_KILL_SELF);
            getDefaultInstance().sendMessageDelayed(
                    getDefaultInstance().obtainMessage(MSG_KILL_SELF), DELAY_KILL_SELF);
        }
    }

    private void dealWithADBCommand(Intent intent) throws ServiceNullException {
        DebugLoggerUIServiceManager.getInstance().getService().daelWithADBCommand(intent);
    }

    private void dealWithMDLoggerRestart(Intent intent) throws ServiceNullException {
        DebugLoggerUIServiceManager.getInstance().getService().dealWithMDLoggerRestart(intent);
    }

    private void dealWithExcptionHappend(Intent intent) throws ServiceNullException {
        DebugLoggerUIServiceManager.getInstance().getService().doTagLogForManually(intent);
    }

    private void dealWithBypassAction(Intent intent) throws ServiceNullException {
        DebugLoggerUIServiceManager.getInstance().getService().dealWithBypassAction(intent);
    }

    /**
     * Judge whether need to start up DebugLoggerUI service at boot time. If none log instance was
     * set to start automatically when boot up, just remove this process to avoid confuse user
     */
    private boolean needStartLogAtBootTime() {
        boolean needStart = false;
        for (Integer logType : Utils.LOG_TYPE_SET) {
            if (logType == Utils.LOG_TYPE_MET) {
                continue;
            }
            if (Utils.VALUE_START_AUTOMATIC_ON == mDefaultSharedPreferences.getBoolean(
                    Utils.KEY_START_AUTOMATIC_MAP.get(logType),
                    Utils.DEFAULT_CONFIG_LOG_AUTO_START_MAP.get(logType))) {
                needStart = true;
                break;
            }
        }
        Utils.logd(TAG, "-->needStartLogAtBootTime(), needStart=" + needStart);
        return needStart;
    }
}
