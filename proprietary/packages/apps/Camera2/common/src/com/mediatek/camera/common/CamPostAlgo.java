package com.mediatek.camera.common;

import android.content.Context;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.SystemProperties;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.portability.ReflectUtil;
import com.mediatek.campostalgo.FeatureConfig;
import com.mediatek.campostalgo.FeatureParam;
import com.mediatek.campostalgo.FeatureResult;
import com.mediatek.campostalgo.ICamPostAlgoCallback;
import com.mediatek.campostalgo.ICamPostAlgoInterface;
import com.mediatek.campostalgo.ICamPostAlgoService;

import java.lang.reflect.Method;

public class CamPostAlgo {
    private static LogUtil.Tag TAG = new LogUtil.Tag(CamPostAlgo.class.getSimpleName());
    private static String SERVICE_NAME = "mediatek.campostalgo";
    private static String CMD_START_SERVICE = "ctl.start";
    private static String CMD_STOP_SERVICE = "ctl.stop";
    private static String PREFIX_GET_SERVICE_STATUS = "init.svc.";
    private static String CAMPOSTALGO = "camerapostalgo";
    private static String SERVICE_STATUS_RUNNING = "running";
    private static final int START_SERVICE_TIMEOUT_MS = 8192;
    private ICamPostAlgoService mService;
    private ICamPostAlgoInterface mInterface;
    private Context mContext;
    private boolean mStarted = false;

    public boolean init(Context context) {
        mContext = context;
        return getService() != null && getInterface(mContext) != null;
    }

    public void unInit() {
        stopService();
    }

    public synchronized FeatureResult start(FeatureConfig[] configs,
                                            ICamPostAlgoCallback callback) {
        LogHelper.d(TAG, "[start] +");
        try {
            FeatureResult result = mInterface.config(configs, callback);
            mStarted = true;
            return result;
        } catch (RemoteException ex) {
            LogHelper.e(TAG, "[start] RemoteException when config", ex);
        } finally {
            LogHelper.d(TAG, "[start] -");
        }
        return null;
    }

    public synchronized void stop() {
        if (!mStarted) {
            return;
        }
        LogHelper.d(TAG, "[stop] +");
        try {
            mInterface.disconnect();
        } catch (RemoteException ex) {
            LogHelper.e(TAG, "[stop] RemoteException when disconnect", ex);
        } finally {
            mStarted = false;
            LogHelper.d(TAG, "[stop] -");
        }
    }


    public synchronized void configParams(int type, FeatureParam params) {
        LogHelper.d(TAG, "[configParams] +");
        try {
            mInterface.configParams(type, params);
        } catch (RemoteException ex) {
            LogHelper.e(TAG, "[configParams] RemoteException when config", ex);
        } finally {
            LogHelper.d(TAG, "[configParams] -");
        }
    }

    private ICamPostAlgoService connect() throws RemoteException {
        Class serviceManagerClass = ReflectUtil.getClass("android.os.ServiceManager");
        Method getServiceMethod = ReflectUtil.getMethod(serviceManagerClass, "getService",
                String.class);
        IBinder binder = (IBinder) ReflectUtil.callMethodOnObject(serviceManagerClass,
                getServiceMethod,
                SERVICE_NAME);
        if(binder != null) {
            binder.linkToDeath(new ServiceDeathRecipient(), 0);
            return ICamPostAlgoService.Stub.asInterface(binder);
        } else {
            return null;
        }
    }

    private void stopService() {
        if(mService != null) {
            if(SERVICE_STATUS_RUNNING.equals(
                    SystemProperties.get(PREFIX_GET_SERVICE_STATUS + CAMPOSTALGO
                            /*"init.svc.camerapostalgo"*/))) {
                LogHelper.d(TAG, "Stop PostAlgo service");
                SystemProperties.set(CMD_STOP_SERVICE/*"ctl.stop"*/,
                        CAMPOSTALGO/*"camerapostalgo"*/);
            }
            mService = null;
        }

    }


    private ICamPostAlgoService getService() {
        if (mService == null) {
            //judge if post algo service is running or not.
            if(!SERVICE_STATUS_RUNNING.equals(
                    SystemProperties.get(PREFIX_GET_SERVICE_STATUS + "." + CAMPOSTALGO
                            /*"init.svc.camerapostalgo"*/))) {
                LogHelper.d(TAG, "Start PostAlgo service");
                SystemProperties.set(CMD_START_SERVICE/*"ctl.start"*/,
                        CAMPOSTALGO/*"camerapostalgo"*/);
            }

            for(int sleepMs = 64; sleepMs < (START_SERVICE_TIMEOUT_MS <<1); sleepMs <<=1) {
                LogHelper.d(TAG, "Connect to PostAlgo service");

                try {
                    mService = connect();
                } catch (Exception e) {
                    e.printStackTrace();
                    LogHelper.e(TAG, "get service error: " + e);
                } finally {
                    if(mService != null) {
                        LogHelper.d(TAG, "Got the PostAlgo service: " + mService);
                        return mService;
                    }
                }
                try {
                    LogHelper.d(TAG, "Service is not ready, wait for " + sleepMs + "ms");
                    Thread.sleep(sleepMs);
                } catch (InterruptedException e) {
                    LogHelper.e(TAG, "Interrupted when waiting for service");
                    return null;
                }
            }
        }
        return mService;
    }

    private class ServiceDeathRecipient implements IBinder.DeathRecipient {
        @Override
        public void binderDied() {
            LogHelper.e(TAG, "[binderDied] Postalgo Service died!");
        }
    }

    private ICamPostAlgoInterface getInterface(Context context) {
        if (mInterface == null) {
            try {
                mInterface = getService().connect(context.getPackageName(), -1);
            } catch (RemoteException ex) {
                LogHelper.e(TAG, "[getInterface] RemoteException when connect", ex);
            }
        }
        return mInterface;
    }
}
