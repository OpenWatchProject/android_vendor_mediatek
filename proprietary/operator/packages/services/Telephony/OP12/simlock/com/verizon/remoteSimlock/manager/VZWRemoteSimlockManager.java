package com.verizon.remoteSimlock.manager;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.util.Log;

import com.verizon.remoteSimlock.IVZWRemoteSimlockService;

/**
 * VZWRemoteSimlockManager class which exposes Asynchronous APIs to allow Agent appliction to
 * perform lock/unlock operation. These APIs shall internally call AIDL service APIs to interact
 * with Modem.
 */
public class VZWRemoteSimlockManager {
    private static final String TAG = "VZWRemoteSimlockManager:";
    private static final int REMOTE_SIM_UNLOCK_ERROR = 1;

    // Stores latest version (MAJOR.MINOR) of VZWRemoteSimlockManager interface.
    public static final String VERSIONINFO = "1.1";

    private Context mContext = null;
    private Handler mCbHandler = null;
    private IVZWRemoteSimlockService mService = null;
    private int mServiceConnId = -1;

    private ServiceConnection mConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName className, IBinder service) {
            synchronized (VZWRemoteSimlockManager.class) {
                mService = IVZWRemoteSimlockService.Stub.asInterface(service);
                mCbHandler.sendMessage(Message.obtain(mCbHandler, mServiceConnId, true));
                logi("onServiceConnected, mService = " + mService + ", mServiceConnId = "
                        + mServiceConnId);
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName className) {
            synchronized (VZWRemoteSimlockManager.class) {
                mCbHandler.sendMessage(Message.obtain(mCbHandler, mServiceConnId, false));
                logi("onServiceDisconnected, mService = " + mService + ", mServiceConnId = "
                        + mServiceConnId);
                mServiceConnId = -1;
                mService = null;
            }
        }
    };

    /**
     * Constructor of VZWRemoteSimlockManager.
     *
     * @param context - context in which AIDL Service will be bind .
     */
    public VZWRemoteSimlockManager(Context context) {
        logi("initialize VZWRemoteSimlockManager");
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
        Intent serviceIntent = new Intent("com.verizon.remoteSimlock.RSU_SERVICE");
        serviceIntent.setComponent(
                new ComponentName("com.verizon.remoteSimlock",
                "com.verizon.remoteSimlock.VZWRemoteSimlockService"));

        mCbHandler = cbHandler;
        if (mCbHandler == null) {
            return false;
        }
        mServiceConnId = serviceConnId;
        if (mContext != null) {
            try {
                boolean r = false;
                r = mContext.bindService(serviceIntent, mConnection, Context.BIND_AUTO_CREATE);
                logi("connectService, mServiceConnId = " + mServiceConnId + ", r = " + r);
                if (r) {
                    return true;
                }
            } catch (SecurityException e) {
                loge("connectService exception, mServiceConnId = " + mServiceConnId);
                e.printStackTrace();
            }
        } else {
            logi("connectService fail, mContext = null, mServiceConnId = " + mServiceConnId);
        }
        mCbHandler.sendMessage(Message.obtain(mCbHandler, mServiceConnId, false));
        mServiceConnId = -1;
        mCbHandler = null;
        return false;
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
        synchronized (VZWRemoteSimlockManager.class) {
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
    public int registerCallback(VZWRemoteSimlockManagerCallback callback) {
        synchronized (VZWRemoteSimlockManager.class) {
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
    public int deregisterCallback(VZWRemoteSimlockManagerCallback callback) {
       synchronized (VZWRemoteSimlockManager.class) {
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
    * @return
    * SIMLOCK_SUCCESS =  0;
    * SIMLOCK_ERROR  =  1;
    */
    public int remoteSimlockProcessSimlockData(int token, byte[] simlockData) {
        synchronized (VZWRemoteSimlockManager.class) {
            if (mService == null) {
                return REMOTE_SIM_UNLOCK_ERROR;
            } else {
                try {
                    return mService.remoteSimlockProcessSimlockData(token, simlockData);
                } catch (RemoteException e) {
                    e.printStackTrace();
                    return REMOTE_SIM_UNLOCK_ERROR;
                }
            }
        }
    }

    /**
    * This API gets highest major/minor version of incoming blob (Response message) that is
    * supported by Network Lock Module.
    *
    * @param token
    * Identifier that can be used by Client to match the asynchronous response for this request.
    *
    * @return
    * SIMLOCK_SUCCESS =  0;
    * SIMLOCK_ERROR  =  1;
    */
    public int remoteSimlockGetVersion(int token) {
        synchronized (VZWRemoteSimlockManager.class) {
            if (mService == null) {
                return REMOTE_SIM_UNLOCK_ERROR;
            } else {
                try {
                    return mService.remoteSimlockGetVersion(token);
                } catch (RemoteException e) {
                    e.printStackTrace();
                    return REMOTE_SIM_UNLOCK_ERROR;
                }
            }
        }
    }

    /**
    * This API gets Sim lock status of the device.
    *
    * @param token
    * Identifier that can be used by Client to match the asynchronous response for this request.
    *
    * @return
    * SIMLOCK_SUCCESS =  0;
    * SIMLOCK_ERROR  =  1;
    */
    public int remoteSimlockGetSimlockStatus(int token) {
        synchronized (VZWRemoteSimlockManager.class) {
            if (mService == null) {
                return REMOTE_SIM_UNLOCK_ERROR;
            } else {
                try {
                    return mService.remoteSimlockGetSimlockStatus(token);
                } catch (RemoteException e) {
                    e.printStackTrace();
                    return REMOTE_SIM_UNLOCK_ERROR;
                }
            }
        }
    }

    /**
    * This API requests Network Lock Module to generate a Request Message.
    *
    * @param token
    * Identifier that can be used by Client to match the asynchronous response for this request.
    *
    * @param requestType
    * Indicates the type of request to generate.
    * Current supported value
    * SIMLOCK_REMOTE_GENERATE_BLOB_REQUEST_TYPE = 1;
    *
    * @return
    * SIMLOCK_SUCCESS =  0;
    * SIMLOCK_ERROR  =  1;
    */
    public int remoteSimlockGenerateRequest(int token, int requestType) {
        synchronized (VZWRemoteSimlockManager.class) {
            if (mService == null) {
                return REMOTE_SIM_UNLOCK_ERROR;
            } else {
                try {
                    return mService.remoteSimlockGenerateRequest(token, requestType);
                } catch (RemoteException e) {
                    e.printStackTrace();
                    return REMOTE_SIM_UNLOCK_ERROR;
                }
            }
        }
    }

    /**
    * This API request Network Lock Module to start/stop unlock device timer to unlock the device
    * for the specific time. Temporary Unlocking allows device to have data connectivity to
    * download the blob.
    * The Network Lock Module unlocks the device only once per UICC power cycle.
    *
    * @param token
    * Identifier that can be used by Client to match the asynchronous response for this request.
    *
    * @param requestType
    * SIMLOCK_REMOTE_START_UNLOCK_TIMER = 1;
    * SIMLOCK_REMOTE_STOP_UNLOCK_TIMER = 2;
    *
    * @return
    * SIMLOCK_SUCCESS =  0;
    * SIMLOCK_ERROR  =  1;
    */
    public int remoteSimlockUnlockTimer(int token, int requestType) {
        synchronized (VZWRemoteSimlockManager.class) {
            if (mService == null) {
                return REMOTE_SIM_UNLOCK_ERROR;
            } else {
                try {
                    return mService.remoteSimlockUnlockTimer(token, requestType);
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
