package com.mediatek.apmonitor;

import com.mediatek.apmonitor.monitor.*;
import com.mediatek.apmonitor.proxy.ApmSession;
import com.mediatek.apmonitor.proxy.IApmSessionCallback;

import android.app.Application;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.IBinder;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.SystemProperties;
import android.support.v4.app.NotificationCompat;
import android.util.Log;

import com.mediatek.common.carrierexpress.CarrierExpressManager;

import java.util.ArrayList;
import java.util.List;


public class ApmService extends Service {
    public static final String TAG = "APM-ServiceJ";
    private static final String APM_NOTIFICATION_CHANNEL = "apm_service_channel";
    private static final int APM_NOTIFICATION_ID = 1;
    private static final String ACTION_START_APM_SERVICE = "com.mediatek.apmonitor.ACTION_START_APM_SERVICE";
    private static final String ACTION_STOP_APM_SERVICE = "com.mediatek.apmonitor.ACTION_STOP_APM_SERVICE";

    private boolean mStartForeground = false;
    private String mNotificationTitle;
    private String mNotificationContent;

    private ApmHandler mHandler = null;
    private HandlerThread mHandlerThread = null;
    private List<KpiMonitor> mKpiMonitors = null;
    private ApmKpiReceiver mReceiver = null;
    protected ApmSession mApmSession = null;
    private Object mLock = new Object();
    private IApmSessionCallback mSessionCallback = null;
    private ArrayList<Short> mSubscribedMsgList = null;

    private boolean mInitialized = false;

    @Override
    public void onCreate() {
        super.onCreate();
        Log.d(TAG, "ApmService onCreate()");
        if (ApmServiceUtils.isDmcSupport()) {
            mNotificationTitle = getString(R.string.apm_notification_title);
            mNotificationContent = getString(R.string.apm_notification_content);
            initialize();
        } else {
            Log.w(TAG, "DMC not enabled, bye~");
        }
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        super.onStartCommand(intent, flags, startId);

        switch (intent.getAction()) {
            case ACTION_START_APM_SERVICE:
                Log.d(TAG, "ApmService onStartCommand(START)");
                startForegroundService();
                break;
            case ACTION_STOP_APM_SERVICE:
                Log.d(TAG, "ApmService onStartCommand(STOP)");
                stopForegroundService();
                stopSelf();
                break;
            default:
                break;
        }
        return START_STICKY;
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "ApmService onDestroy()");
        if (ApmServiceUtils.isDmcSupport()) {
            this.unregisterReceiver(mReceiver);
        }
        stopForegroundService();
        super.onDestroy();
    }

    @Override
    public IBinder onBind(Intent intent) {
        // Not implemented
        Log.w(TAG, "Not support to bind APM service");
        return null;
    }

    private void startForegroundService() {
        NotificationManager notificationManager
                = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
        if (notificationManager != null) {
            NotificationChannel channel = new NotificationChannel(
                    APM_NOTIFICATION_CHANNEL, APM_NOTIFICATION_CHANNEL,
                    NotificationManager.IMPORTANCE_DEFAULT);
            notificationManager.createNotificationChannel(channel);

            startForeground(APM_NOTIFICATION_ID, new NotificationCompat.Builder(this, APM_NOTIFICATION_CHANNEL)
                    .setContentTitle(mNotificationTitle)
                    .setContentText(mNotificationContent)
                    .setSmallIcon(android.R.drawable.ic_dialog_alert)
                    .build());
            mStartForeground = true;
        }
    }

    private void stopForegroundService() {
        if (mStartForeground) {
            stopForeground(true);
            NotificationManager notificationManager
                    = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
            if (notificationManager != null) {
                notificationManager.cancel(APM_NOTIFICATION_ID);
            }
            mStartForeground = false;
        }
    }

    public void initialize() {
        mApmSession = new ApmSession();
        mSubscribedMsgList = new ArrayList<Short>();
        mSessionCallback = new ApmSessionCallback();
        mApmSession.registControlCallback(mSessionCallback);
        mApmSession.tryCreateApmSession(false);
        initHandlerThread();
        setupKpiMonitors();
    }

    public ApmSession getApmClient() {
        return mApmSession;
    }

    private void setupKpiMonitors() {
        synchronized(mLock) {
            if (mReceiver == null) {
                 mReceiver = new ApmKpiReceiver(this, mHandler);
            }

            if (mKpiMonitors == null) {
                mKpiMonitors = new ArrayList<KpiMonitor>();
                mKpiMonitors.add(new FooBarKpiMonitor(this, mHandler));
                mKpiMonitors.add(new NetworkServiceStateKpiMonitor(this, mHandler));
                mKpiMonitors.add(new LocationMessageKpiMonitor(this, mHandler));
                mKpiMonitors.add(new WifiKpiMonitor(this, mHandler));
            }

            IntentFilter filter = new IntentFilter();
            filter.addAction(CarrierExpressManager.ACTION_OPERATOR_CONFIG_CHANGED);
            // To initialize APM and create HIDL sessino to APM server
            filter.addAction(IApmKpiMonitor.ACTION_APM_INIT_EVENT);
            // To register APM KPI Intent
            filter.addAction(IApmKpiMonitor.ACTION_APM_KPI_EVENT);
            Log.d(TAG, "start monitoring APM KPI, APM activated = " + ApmServiceUtils.isApmActive());
            this.registerReceiver(mReceiver, filter);
        }
    }

    private void initHandlerThread() {
        if (mInitialized) {
            return;
        }
        mHandlerThread = new HandlerThread("apm_srv");
        mHandlerThread.start();
        mHandler = new ApmHandler(this, mHandlerThread.getLooper());
        mInitialized = true;
        Log.d(TAG, "APM monitor thread initialized");
    }

    class ApmHandler extends Handler {
        private Context mContext = null;

        private ApmHandler() {}

        public ApmHandler(Context context, Looper looper) {
            super(looper);
            mContext = context;
        }

        private boolean forceUpdateMessage(int what) {
            switch(what) {
                case KpiMonitor.MSG_ID_APM_KPI_STATE_CHANGED:
                    return true;
                default:
                    break;
            }
            return false;
        }

        @Override
        public void handleMessage(Message msg) {
            if (!forceUpdateMessage(msg.what) && !ApmServiceUtils.isApmActive()) {
                return;
            }

            synchronized(mLock) {
                for (KpiMonitor monitor : mKpiMonitors) {
                    if (monitor != null) {
                        monitor.handleMessage(msg);
                    }
                }
            }
        }
    }

    class ApmSessionCallback implements IApmSessionCallback {
        @Override
        public void onQueryKpi(Short msgId) {
            Log.d(TAG, "onQueryKpi(" + msgId + ")");
            Message msg = mHandler.obtainMessage(
                        KpiMonitor.MSG_ID_APM_KPI_ON_QUERY, msgId, 0);
            mHandler.sendMessage(msg);
        }

        @Override
        public void onKpiSubscriptionStateChanged(ArrayList<Short> subscribedMessageList) {
            // Check the difference and notify KpiMonitors
            List<Short> removedList = new ArrayList<Short>(mSubscribedMsgList);
            List<Short> addedList = new ArrayList<Short>(subscribedMessageList);
            removedList.removeAll(subscribedMessageList);
            addedList.removeAll(mSubscribedMsgList);

            for (Short msgId : removedList) {
                Message msg = mHandler.obtainMessage(
                        KpiMonitor.MSG_ID_APM_KPI_STATE_CHANGED,
                        KpiMonitor.EVENT_KPI_REMOVED, 0, (Object)msgId);
                mHandler.sendMessage(msg);
                Log.d(TAG, "Remove APM msgId = " + msgId);
            }
            for (Short msgId : addedList) {
                Message msg = mHandler.obtainMessage(
                        KpiMonitor.MSG_ID_APM_KPI_STATE_CHANGED,
                        KpiMonitor.EVENT_KPI_ADDED, 0, (Object)msgId);
                mHandler.sendMessage(msg);
                Log.d(TAG, "Add APM msgId = " + msgId);
            }
            // Update cache
            mSubscribedMsgList.clear();
            for (Short msgId : subscribedMessageList) {
                mSubscribedMsgList.add(msgId);
            }
        }
    }
}
