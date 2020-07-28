package com.debug.loggerui.framework;

import android.app.INotificationManager;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.RemoteException;
import android.os.ServiceManager;

import com.debug.loggerui.MainActivity;
import com.debug.loggerui.MyApplication;
import com.debug.loggerui.R;
import com.debug.loggerui.controller.LogControllerUtils;
import com.debug.loggerui.controller.MultiLogTypesController;
import com.debug.loggerui.utils.Utils;
import com.log.handler.LogHandlerUtils.LogType;

import java.util.Set;

/**
 * Supper class for each log instance, like network log, mobile log and modem. log
 */
public class ServiceStatusManager {
    private static final String TAG = Utils.TAG + "/ServiceStatusManager";

    private Service mService;
    private Notification.Builder mBuilder = null;
    private NotificationManager mNotificationManager = null;
    private static NotificationChannel sChannel = null;
    private static final int SERVICE_NOTIFICATION_ICON = R.drawable.notification;
    private static final int SERVICE_NOTIFICATION_ID = 365001;
    private static final String NOTIFICATION_CHANNEL_ID = "com.debug.loggerui.notification";

    private static INotificationManager sINM = INotificationManager.Stub
            .asInterface(ServiceManager.getService(Context.NOTIFICATION_SERVICE));
    private static final String PACKAGE_NAME = "com.debug.loggerui";

    private ServiceStatus mLastServiceStatus;

    /**
     * @author MTK11515 ServiceStatus is DebugLoggerUI service running state.
     */
    public static enum ServiceStatus {
        ONCREATE_DONE, ONSTARTCOMMAND_DONE, LOG_STARTING, LOG_STARTING_DONE,
        LOG_STOPPING, LOG_STOPPING_DONE,
        LOG_RESTARTING, LOG_RESTART_DONE, ONDESTROYING;
    }
    /**
     * @param service Service
     */
    public ServiceStatusManager(Service service) {
        mService = service;
    }
    /**
     * @param status ServiceStatus
     */
    public void statusChanged(ServiceStatus status) {
        Utils.logi(TAG, "statusChanged : " + status.toString()
                   + ", old status = " + mLastServiceStatus);
        String notificationMsg = "";
        switch (status) {
        case ONCREATE_DONE:
            notificationMsg = mService.getString(R.string.service_state_init);
            break;
         //NEED UPDATE LOG STATUS
        case ONSTARTCOMMAND_DONE:
        case LOG_STARTING_DONE:
        case LOG_STOPPING_DONE:
        case LOG_RESTART_DONE:
            notificationMsg = getLogStatusMessage();
            if (notificationMsg == null || notificationMsg.isEmpty()) {
                notificationMsg = mService.getString(R.string.service_state_all_log_stopped);
            }
            break;
        case LOG_STARTING:
            notificationMsg = mService.getString(R.string.service_state_starting_log);
            break;
        case LOG_STOPPING:
            notificationMsg = mService.getString(R.string.service_state_stopping_log);
            break;
        case LOG_RESTARTING:
            notificationMsg = mService.getString(R.string.service_state_restarting_log);
           break;
        case ONDESTROYING:
            mService.stopForeground(true);
            return;
        default:
           break;
        }
        mLastServiceStatus = status;
        if (!notificationMsg.isEmpty()) {
            showServiceNotification(notificationMsg);
        }
    }

    /**
     * @param enabled
     *            boolean
     */
    public void setNotificationsEnabledForPackage(boolean enabled) {
        Utils.logi(TAG, "setNotificationsEnabledForPackage() enabled ? " + enabled);
        try {
            PackageManager pm = mService.getPackageManager();
            ApplicationInfo applicationInfo =
                    pm.getApplicationInfo(PACKAGE_NAME, PackageManager.GET_ACTIVITIES);
            Utils.logi(TAG, "setNotificationsEnabledForPackage() enabled = " + enabled
                    + ", applicationInfo.uid = " + applicationInfo.uid);
            sINM.setNotificationsEnabledForPackage(PACKAGE_NAME, applicationInfo.uid, enabled);
        } catch (NameNotFoundException e) {
            e.printStackTrace();
        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }

    private String getLogStatusMessage() {
        String status = "";
        Set<LogType> runningTypes = MultiLogTypesController.getInstance().getAllRunningLogTypes();
        Utils.logi(TAG, "getAllRunningLogTypes() runningTypes.Size = " + runningTypes.size());
        for (int key = 1; key <= LogType.MAX_ID; key++) {
            LogType curType = LogType.getLogTypeById(key);
            if (runningTypes.contains(curType)) {
                status += mService.getString(Utils.LOG_NAME_MAP.get(
                        LogControllerUtils.LOG_TYPE_OBJECT_TO_INT.get(curType))) + ",";
            }
        }
        if (status.isEmpty()) {
            return status;
        }
        // Format log string from MobileLog,ModemLog,NetworkLog,ConnsysFWLog,
        // to Mobile,Modem,Network,ConnsysFW Log
        if (status.endsWith("Log,")) {
            status = status.substring(0, status.length() - "Log,".length()) + " Log ";
            status += status.contains(",")
                    ? mService.getString(R.string.notification_on_summary_suffixes)
                    : mService.getString(R.string.notification_on_summary_suffix);
        }
        status = status.replaceAll("Log,", ",");
        return status;
    }
    /**
     * void.
     */
    public void updateNotificationTime() {
        if (mBuilder != null) {
            mBuilder.setWhen(System.currentTimeMillis());
        }
    }
    /**
     * @param text String
     */
    public synchronized void showServiceNotification(String text) {
        if (!MyApplication.getInstance().getDefaultSharedPreferences()
                .getBoolean(Utils.KEY_PREFERENCE_NOTIFICATION_ENABLED,
                        Utils.DEFAULT_NOTIFICATION_ENABLED_VALUE)) {
            Utils.logw(TAG, "Notification is disabled, does not show any notification.");
            setNotificationsEnabledForPackage(false);
        }
        if (mBuilder == null) {
            mBuilder =
                    new Notification.Builder(mService, getNotificationChannelId());
            mBuilder.setContentTitle(mService.getString(R.string.notification_title))
                    .setSmallIcon(SERVICE_NOTIFICATION_ICON).setOnlyAlertOnce(true);
        }
        if (!Utils.isAutoTest()) {
            Intent backIntent = new Intent(mService, MainActivity.class);
            backIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            PendingIntent contentIntent = PendingIntent.getActivity(mService, 0, backIntent, 0);
            mBuilder.setContentIntent(contentIntent);
        } else {
            mBuilder.setContentIntent(null);
            Utils.logw(TAG, "Do not allow open activity from status bar when isAutoTest!");
        }
        Utils.logw(TAG, "showServiceNotification(), context = " + text );
        mBuilder.setStyle(new Notification.BigTextStyle().bigText(text));
        if (mNotificationManager == null) {
            mNotificationManager = (NotificationManager) mService
                                   .getSystemService(Context.NOTIFICATION_SERVICE);
        }
        mService.startForeground(SERVICE_NOTIFICATION_ID, mBuilder.build());
    }
    /**
     * void.
     */
    public void disappearServiceNotification() {
        if (mNotificationManager == null) {
            mNotificationManager = (NotificationManager) mService
                                   .getSystemService(Context.NOTIFICATION_SERVICE);
        }
        Utils.logw(TAG, "disappearServiceNotification()");
        mNotificationManager.cancel(SERVICE_NOTIFICATION_ID);
        mNotificationManager = null;
        mBuilder = null;
    }

    /**
     * @return String
     */
    public synchronized static String getNotificationChannelId() {
        if (sChannel == null) {
            // Need set NOTIFICATION_CHANNEL_ID to NotificationManager
            sChannel = new NotificationChannel(NOTIFICATION_CHANNEL_ID, "DebugLoggerUI",
                    NotificationManager.IMPORTANCE_HIGH);
            NotificationManager notificationManager =
                    (NotificationManager) MyApplication.getInstance().getApplicationContext()
                            .getSystemService(Context.NOTIFICATION_SERVICE);
            notificationManager.createNotificationChannel(sChannel);
        }
        return NOTIFICATION_CHANNEL_ID;
    }

}
