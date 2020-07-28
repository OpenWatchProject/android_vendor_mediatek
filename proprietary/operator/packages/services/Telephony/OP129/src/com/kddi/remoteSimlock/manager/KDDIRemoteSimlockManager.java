package com.kddi.remoteSimlock.manager;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.util.Log;

import com.kddi.remoteSimlock.IKDDIRemoteSimlockService;

/**
 *KDDIRemoteSimlockManager class which exposes Asynchronous APIs to allow Agent appliction to
 * perform lock/unlock operation. These APIs shall internally call AIDL service APIs to interact
 * with Modem.
 */
public class KDDIRemoteSimlockManager {
    private static final String TAG = "KDDIRemoteSimlockManager:";
    private static final int REMOTE_SIM_UNLOCK_ERROR = 1;

    // Stores latest version (MAJOR.MINOR) of KDDIRemoteSimlockManager interface.
    public static final String VERSIONINFO = "1.1";

    private Context mContext = null;
    private Handler mCbHandler = null;
    private IKDDIRemoteSimlockService mService = null;
    private int mServiceConnId = -1;

    private ServiceConnection mConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName className, IBinder service) {
            synchronized (KDDIRemoteSimlockManager.class) {
                mService = IKDDIRemoteSimlockService.Stub.asInterface(service);
                mCbHandler.sendMessage(Message.obtain(mCbHandler, mServiceConnId, true));
                logi("onServiceConnected, mService = " + mService + ", mServiceConnId = "
                        + mServiceConnId);
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName className) {
            synchronized (KDDIRemoteSimlockManager.class) {
                mCbHandler.sendMessage(Message.obtain(mCbHandler, mServiceConnId, false));
                logi("onServiceDisconnected, mService = " + mService + ", mServiceConnId = "
                        + mServiceConnId);
                mServiceConnId = -1;
                mService = null;
            }
        }
    };

    /**
     * Constructor of KDDIRemoteSimlockManager.
     *
     * @param context - context in which AIDL Service will be bind .
     */
    public KDDIRemoteSimlockManager(Context context) {
        logi("initialize KDDIRemoteSimlockManager");
        mContext = context;
    }

    /**
    * This API Start connection with the Service.
    *
    * @param cbHandler
    * Callback Handler to which message will be send.
    * @param serviceConnId
    * This integer identifier will be put in the "what" variable of the message that is sent to
    * the handler, so the recepient can identify this message. Message will also contain
    * "msg.obj" with boolean value. This will be set to true if service connected and false
    * if service disconnected.
    *
    * @return
    * boolean type: Immediate result of the operaton.  This response does not confirm that service
    * is connected.  To know if service is connected, client need to listen to handler message.
    */
    public boolean connectService(Handler cbHandler, int serviceConnId) {
        Intent serviceIntent = new Intent("com.kddi.remoteSimlock.RSU_SERVICE");
        serviceIntent.setComponent(
                new ComponentName("com.kddi.remoteSimlock",
                "com.kddi.remoteSimlock.KDDIRemoteSimlockService"));
        logi("connectService KDDIRemoteSimlockManager");
        mCbHandler = cbHandler;
        if (mCbHandler == null) {
            logi("mCbHandler null");
            return false;
        }
        mServiceConnId = serviceConnId;
        if (mContext != null) {
            logi("bindService RSU_SERVICE");
            boolean bindService = mContext.bindService(serviceIntent, mConnection, Context.BIND_AUTO_CREATE);
            logi("bindService RSU_SERVICE = " + bindService);
            return true;
        } else {
            mCbHandler.sendMessage(Message.obtain(mCbHandler, mServiceConnId, false));
            logi("connectService fail, mServiceConnId = " + mServiceConnId);
            mServiceConnId = -1;
            mCbHandler = null;
            return false;
        }
    }

    /**
    * This API Disconnect the connection with the service.
    */
    public void disconnectService() {
        mContext.unbindService(mConnection);
    }

    /**
    * This API checks if Service is connected or Not.
    *
    * @return
    * boolean type: True if service connected, false if service not connected.
    */
    public boolean isServiceConnected() {
        synchronized (KDDIRemoteSimlockManager.class) {
            if (mService == null) {
                return false;
            } else {
                return true;
            }
        }
    }

    /**
    * This API registers the callback, so the client can receive asynchronous responses.
    *
    * @param callback
    * Instance of callback
    *
    * @return
    * SIMLOCK_SUCCESS =  0. Successfully registered callback with SimLock service;
    * SIMLOCK_ERROR  = 1. Failed to register call back;
    */
    public int registerCallback(KDDIRemoteSimlockManagerCallback callback) {
        synchronized (KDDIRemoteSimlockManager.class) {
            if (mService == null) {
                return REMOTE_SIM_UNLOCK_ERROR;
            } else {
                try {
                    return mService.registerCallback(callback);
                } catch (RemoteException e) {
                    e.printStackTrace();
                    return REMOTE_SIM_UNLOCK_ERROR;
                }
            }
        }
    }

    /**
    * This API de-registers the callback.
    *
    * @param callback
    * Instance of callback
    *
    * @return
    * SIMLOCK_SUCCESS =  0. Successfully deregistered callback with SimLock service;
    * SIMLOCK_ERROR  =  1. Failed to deregister call back;
    */
    public int deregisterCallback(KDDIRemoteSimlockManagerCallback callback) {
       synchronized (KDDIRemoteSimlockManager.class) {
           if (mService == null) {
               return REMOTE_SIM_UNLOCK_ERROR;
           } else {
               try {
                   return mService.deregisterCallback(callback);
               } catch (RemoteException e) {
                   e.printStackTrace();
                   return REMOTE_SIM_UNLOCK_ERROR;
               }
           }
       }
    }

    /**
    * This API sends lock/unlock blob (also known as Response Message) to Network Lock Module.
    * Note: Network Lock module use this blob to perform lock/unlock operation
    *
    * @param token
    * Identifier that can be used by Client to match the asynchronous response for this request.
    *
    * @param simlockData
    * This contains simlock blob for lock and unlock operation
    *
    * @param soltId
    * Identifier the SIM that the request to be sent. soltId = 0 (SIM1), slotId = 1 (SIM2)

    * @return
    * SIMLOCK_SUCCESS =  0;
    * SIMLOCK_ERROR  =  1;
    */
    public int setSimLockPolicyData(int token, byte[] simlockData, int slotId) {
        synchronized (KDDIRemoteSimlockManager.class) {
            if (mService == null) {
                return REMOTE_SIM_UNLOCK_ERROR;
            } else {
                try {
                    return mService.setSimLockPolicyData(token, simlockData, slotId);
                } catch (RemoteException e) {
                    e.printStackTrace();
                    return REMOTE_SIM_UNLOCK_ERROR;
                }
            }
        }
    }

    /**
    * This API gets Sim Lock Data from modem by specific slotId.
    *
    * @param token
    * Identifier that can be used by Client to match the asynchronous response for this request.
    *
    * @param soltId
    * Identifier the SIM that the request to be sent. soltId = 0 (SIM1), slotId = 1 (SIM2)
    *
    * @return
    * SIMLOCK_SUCCESS =  0;
    * SIMLOCK_ERROR  =  1;
    */
    public int getAllowedSimLockData(int token, int slotId) {
        synchronized (KDDIRemoteSimlockManager.class) {
            if (mService == null) {
                return REMOTE_SIM_UNLOCK_ERROR;
            } else {
                try {
                    return mService.getAllowedSimLockData(token, slotId);
                } catch (RemoteException e) {
                    e.printStackTrace();
                    return REMOTE_SIM_UNLOCK_ERROR;
                }
            }
        }
    }

    /**
    * This API gets Sim Lock Data from modem by specific slotId.
    *
    * @param token
    * Identifier that can be used by Client to match the asynchronous response for this request.
    *
    * @param soltId
    * Identifier the SIM that the request to be sent. soltId = 0 (SIM1), slotId = 1 (SIM2)
    *
    * @return
    * SIMLOCK_SUCCESS =  0;
    * SIMLOCK_ERROR  =  1;
    */
    public int getExcludedSimLockData(int token, int slotId) {
        synchronized (KDDIRemoteSimlockManager.class) {
            if (mService == null) {
                return REMOTE_SIM_UNLOCK_ERROR;
            } else {
                try {
                    return mService.getExcludedSimLockData(token, slotId);
                } catch (RemoteException e) {
                    e.printStackTrace();
                    return REMOTE_SIM_UNLOCK_ERROR;
                }
            }
        }
    }

    private void logi(String s) {
        Log.i(TAG, "[RSU-SIMLOCK] " + s);
    }

    private void loge(String s) {
        Log.e(TAG, "[RSU-SIMLOCK] " + s);
    }
}
