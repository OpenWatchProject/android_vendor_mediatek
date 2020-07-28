package com.debug.loggerui.framework;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.os.Looper;

import com.debug.loggerui.MyApplication;
import com.debug.loggerui.framework.DebugLoggerUIService.ServiceBinder;
import com.debug.loggerui.utils.Utils;

/**
 * @author MTK81255
 *
 */
public class DebugLoggerUIServiceManager {

    private static final String TAG = Utils.TAG + "/DebugLoggerUIServiceManager";
    private static DebugLoggerUIServiceManager sServiceManager = new DebugLoggerUIServiceManager();
    private DebugLoggerUIService mService;
    private static Context sContext = MyApplication.getInstance().getApplicationContext();

    private ServiceConnection mServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            Utils.logi(TAG, "Bind to service successfully");
            if (service instanceof ServiceBinder) {
                ServiceBinder binder = (ServiceBinder) service;
                mService = binder.getDebugLoggerUIService();
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            Utils.logi(TAG, "Service is unbind!");
            mService = null;
        }
    };

    private DebugLoggerUIServiceManager() {
    }

    public static final DebugLoggerUIServiceManager getInstance() {
        return sServiceManager;
    }

    /**
     * Start & Bind service.
     */
    public void initService() {
        Utils.logi(TAG, "Service is init!");
        startService();
        bindService();
    }

    private void startService() {
        Intent intent = new Intent(sContext, DebugLoggerUIService.class);
        sContext.startForegroundService(intent);
    }

    private void bindService() {
        Intent intent = new Intent(sContext, DebugLoggerUIService.class);
        sContext.bindService(intent, mServiceConnection, Context.BIND_AUTO_CREATE);
    }

    /**
     * void.
     */
    public void stopService() {
        Intent intent = new Intent(sContext, DebugLoggerUIService.class);
        sContext.unbindService(mServiceConnection);
        sContext.stopService(intent);
    }

    private boolean mIsServiceUsed = false;

    /**
     * @return DebugLoggerUIService
     * @throws ServiceNullException
     *             throw for service is not bind done.
     */
    public DebugLoggerUIService getService() throws ServiceNullException {
        mIsServiceUsed = true;
        if (mService == null) {
            initService();
            if (Looper.getMainLooper() == Looper.myLooper()) {
                Utils.logw(TAG, "Service is not bind when Main Thread try to get it.");
                throw new ServiceNullException();
            }
            synchronized (this) {
                if (mService == null) {
                    int timeout = 300 * 1000;
                    while (mService == null) {
                        try {
                            Thread.sleep(100);
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                        timeout -= 100;
                        if (timeout <= 0) {
                            Utils.logw(TAG, "Service is not bind ok in "
                                    + 300 * 1000 + " seconds");
                            throw new ServiceNullException();
                        }
                    }
                    Utils.logi(TAG, "Service start time: time = " + (300 * 1000 - timeout));
                }
            }
        }
        return mService;
    }

    /**
     * @return boolean
     */
    public boolean isServiceUsed() {
        return mIsServiceUsed;
    }

    /**
     * @author MTK81255
     * If service is not bind before used, throw ServiceNullException.
     */
    public class ServiceNullException extends Exception {
        private static final long serialVersionUID = -430298482097527072L;
    }

}
