package com.mediatek.presence.service;

import java.util.List;

import android.app.ActivityManager;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;

/**
 * Client API utils
 *
 * @author Jean-Marc AUFFRET
 */
public class ServiceUtils {
    /**
     * RCS service name
     */
    public static final String RCS_SERVICE_NAME = "com.mediatek.presence.SERVICE";

    /**
     * Startup service name
     */
    public static final String STARTUP_SERVICE_NAME = "com.mediatek.presence.STARTUP";

    /**
     * Provisioning service name
     */
    public static final String PROVISIONING_SERVICE_NAME = "com.mediatek.presence.PROVISIONING";

    /**
     * Is service started
     *
     * @param ctx Context
     * @return Boolean
     */
    public static boolean isServiceStarted(Context ctx) {
        ActivityManager activityManager = (ActivityManager)ctx.getSystemService(Context.ACTIVITY_SERVICE);
        List<ActivityManager.RunningServiceInfo> serviceList = activityManager.getRunningServices(Integer.MAX_VALUE);
         for(int i = 0; i < serviceList.size(); i++) {
               ActivityManager.RunningServiceInfo serviceInfo = serviceList.get(i);
               ComponentName serviceName = serviceInfo.service;
               if (serviceName.getClassName().equals("com.mediatek.presence.service.RcsCoreService")) {
                     if (serviceInfo.pid != 0) {
                          return true;
                     } else {
                          return false;
                     }
               }
         }
         return false;
    }

    /**
     * Start RCS service
     *
     * @param ctx Context
     */
    public static void startRcsService(Context ctx) {
        ctx.startService(new Intent(STARTUP_SERVICE_NAME));
    }

    /**
     * Stop RCS service
     *
     * @param ctx Context
     */
    public static void stopRcsService(Context ctx) {
        ctx.stopService(new Intent(STARTUP_SERVICE_NAME));
        ctx.stopService(new Intent(PROVISIONING_SERVICE_NAME));
        ctx.stopService(new Intent(RCS_SERVICE_NAME));
    }
}
