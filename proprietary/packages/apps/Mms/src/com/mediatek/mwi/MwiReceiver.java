package com.mediatek.mwi;


import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.PowerManager;

import com.android.mms.MmsConfig;
import com.android.mms.util.FeatureOption;
import com.android.mms.util.MmsLog;

import com.mediatek.mms.util.PermissionCheckUtil;


public class MwiReceiver extends BroadcastReceiver {
    private static boolean DEBUG = true;
    private static String TAG = "Mms/Mwi";

    static final Object STARTING_SERVICE_SYNC = new Object();
    static PowerManager.WakeLock sStartingService;

    @Override
    public void onReceive(Context context, Intent intent) {
        MmsLog.d(TAG, "onReceive, MTK_MWI_SUPPORT = " + FeatureOption.MTK_MWI_SUPPORT);
        if (FeatureOption.MTK_MWI_SUPPORT) {
            if (!PermissionCheckUtil.checkRequiredPermissions(context)) {
                MmsLog.dpi(TAG,
                        "MwiReceiver onReceive no runtime permissions, intent: " + intent);
                return;
            }
            if ("mediatek.intent.action.lte.mwi".equals(intent.getAction()) == false) {
                MmsLog.dpi(TAG,
                        "MwiReceiver onReceive a wrong action, action: " + intent.getAction());
                return;
            }
            onReceiveWithPrivilege(context, intent, true);
        }
    }

    protected void onReceiveWithPrivilege(Context context, Intent intent, boolean privileged) {
        // If 'privileged' is false, it means that the intent was delivered to the base
        // no-permissions receiver class.  If we get an WAP_PUSH_RECEIVED message that way, it
        // means someone has tried to spoof the message by delivering it outside the normal
        // permission-checked route, so we just ignore it.
        // Need to modify
        if (!privileged && ((intent != null) && intent.getAction().equals(
                "mediatek.intent.action.lte.mwi"))) {
            return;
        }


        MmsLog.d(TAG, "MwiReceiver: onReceiveWithPrivilege()"
            + ", Action = " + intent.getAction()
            + ", result = " + getResultCode());
        intent.setClassName(MmsConfig.MMS_APP_SERVICE_PACKAGE, MmsConfig.MWI_RECEIVER_SERVICE);
        intent.putExtra("result", getResultCode());
        beginStartingService(context, intent);
    }

    /**
     * Called back by the service when it has finished processing notifications,
     * releasing the wake lock if the service is now stopping.
     */
    public static void finishStartingService(Service service, int startId) {
        synchronized (STARTING_SERVICE_SYNC) {
            if (sStartingService != null) {
                if (service.stopSelfResult(startId)) {
                    sStartingService.release();
                }
            }
        }
    }

    // N.B.: <code>beginStartingService</code> and
    // <code>finishStartingService</code> were copied from
    // <code>com.android.calendar.AlertReceiver</code>.  We should
    // factor them out or, even better, improve the API for starting
    // services under wake locks.

    /**
     * Start the service to process the current event notifications, acquiring
     * the wake lock before returning to ensure that the service will run.
     */
    public static void beginStartingService(Context context, Intent intent) {
        synchronized (STARTING_SERVICE_SYNC) {
            if (sStartingService == null) {
                PowerManager pm =
                    (PowerManager) context.getSystemService(Context.POWER_SERVICE);
                sStartingService = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK,
                        "StartingAlertService");
                sStartingService.setReferenceCounted(false);
            }
            sStartingService.acquire();
            context.startService(intent);
        }
    }

}
