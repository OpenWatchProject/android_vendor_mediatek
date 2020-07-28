package com.mediatek.rsu;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;

import android.text.TextUtils;

import android.util.Log;

import com.mediatek.rsu.IRsuService;
import static com.mediatek.rsu.RsuUtils.SLE_VENDOR_CODE_MEDIATEK;
import static com.mediatek.rsu.RsuUtils.SLE_FAILURE;
import static com.mediatek.rsu.RsuUtils.SLE_RETURN_CODE_GENERAL_FAILURE;

/**
 * API wrapper of RsuService.
 */
public class RsuManager {
    private static final String TAG = "RsuManager:";
    private static final String ACTTION_RSU_SERVICE_INTENT = "com.mediatek.rsu.IRsuService";
    private static final String RSU_SERVICE_COMPONENT = "com.mediatek.rsu.RsuService";
    private static final String RSU_SERVICE_PACKAGE = "com.mediatek.rsu";

    private Context mContext = null;
    private Handler mCbHandler = null;
    private IRsuService mService = null;
    private int mMessageId = -1;

    private ServiceConnection mConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName className, IBinder service) {
            synchronized (RsuManager.class) {
                mService = IRsuService.Stub.asInterface(service);
                mCbHandler.sendMessage(Message.obtain(mCbHandler, mMessageId, true));
                logd("onServiceConnected, mService = " + mService + ", mMessageId = "
                        + mMessageId);
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName className) {
            synchronized (RsuManager.class) {
                mCbHandler.sendMessage(Message.obtain(mCbHandler, mMessageId, false));
                logd("onServiceDisconnected, mService = " + mService + ", mMessageId = "
                        + mMessageId);
                mMessageId = -1;
                mService = null;
            }
        }
    };

    /**
     * Constructor of RsuManager.
     *
     * @param context - context in which AIDL Service will be bind .
     */
    public RsuManager(Context context) {
        logd("initialize RsuManager");
        mContext = context;
    }

    /**
    * This API Start connection with the Service.
    *
    * @param cbHandler
    * Callback Handler to which message will be sent when bind service complete.
    * @param what
    * Message.what sent to the handler.
    * Message.obj -- boolean value.
    *   true  -- service connected
    *   false -- service disconnected.
    *
    * @return
    *   Immediate result of the operaton. This response does not confirm that service
    *   is connected. To know if service is connected, client need to listen to handler message.
    */
    public boolean connectService(Handler cbHandler, int what) {
        logd("connectService, mMessageId = " + mMessageId);

        if (cbHandler == null) {
            loge("connectService, cbHandler == null");
            return false;
        }

        synchronized (RsuManager.class) {
            if (mService != null || mCbHandler != null) {
                loge("connectService, already connecting/connected!");
                if (cbHandler == mCbHandler && what == mMessageId) {
                    return true;
                } else {
                    return false;
                }
            }

            Intent serviceIntent = new Intent(ACTTION_RSU_SERVICE_INTENT);
            serviceIntent.setComponent(
                    new ComponentName(RSU_SERVICE_PACKAGE, RSU_SERVICE_COMPONENT));
            mCbHandler = cbHandler;
            mMessageId = what;
            if (mContext != null) {
                try {
                    logd("connectService, bindService start");
                    boolean bindService = mContext.bindService(serviceIntent, mConnection,
                            Context.BIND_AUTO_CREATE);
                    logd("connectService, bindService end:" + bindService);
                    return true;
                } catch (SecurityException e) {
                    loge("connectService exception, mMessageId = " + mMessageId);
                    mCbHandler.sendMessage(Message.obtain(mCbHandler, mMessageId, false));
                    mMessageId = -1;
                    mCbHandler = null;
                    e.printStackTrace();
                    return false;
                }
            } else {
                mCbHandler.sendMessage(Message.obtain(mCbHandler, mMessageId, false));
                loge("connectService fail, mContext is null!");
                mMessageId = -1;
                mCbHandler = null;
                return false;
            }
        }
    }

    /**
    * This API Disconnect the connection with the service.
    */
    public void disconnectService() {
        logd("disconnectService");
        mContext.unbindService(mConnection);
    }

    /**
    * This API checks if Service is connected or Not.
    *
    * @return
    * boolean type: True if service connected, false if service not connected.
    */
    public boolean isServiceConnected() {
        synchronized (RsuManager.class) {
            if (mService == null) {
                return false;
            } else {
                return true;
            }
        }
    }

    /**
    * Request SLE Vendor Service Adapter for the shared symmetric key data that will be
    * sent to the Unlock Server for registration.
    *
    * @param data the input data that contains the IMEI
    *
    * @return the formatted shared symmetric key data or error
    */
    public byte[] getRegisterRequest(byte[] data) {
        synchronized (RsuManager.class) {
            if (mService == null) {
                byte[] result = RsuUtils.createErrorResponseBytes(SLE_VENDOR_CODE_MEDIATEK,
                        SLE_FAILURE,
                        SLE_RETURN_CODE_GENERAL_FAILURE,
                        null);
                return result;
            } else {
                try {
                    return mService.getRegisterRequest(data);
                } catch (RemoteException e) {
                    e.printStackTrace();
                    byte[] result = RsuUtils.createErrorResponseBytes(SLE_VENDOR_CODE_MEDIATEK,
                            SLE_FAILURE,
                            SLE_RETURN_CODE_GENERAL_FAILURE,
                            null);
                    return result;
                }
            }
        }
    }

    /**
    * Request Vendor Service Adapter to generate and return the formatted unlock
    * request that will be sent to the Unlock Server.
    *
    * @param data the input data that contains the unlock request type and the IMSI
    *
    * @return the formatted unlock request or error
    */
    public byte[] createUnlockRequest(byte[] data) {
        synchronized (RsuManager.class) {
            if (mService == null) {
                byte[] result = RsuUtils.createErrorResponseBytes(SLE_VENDOR_CODE_MEDIATEK,
                        SLE_FAILURE,
                        SLE_RETURN_CODE_GENERAL_FAILURE,
                        null);
                return result;
            } else {
                try {
                    return mService.createUnlockRequest(data);
                } catch (RemoteException e) {
                    e.printStackTrace();
                    byte[] result = RsuUtils.createErrorResponseBytes(SLE_VENDOR_CODE_MEDIATEK,
                            SLE_FAILURE,
                            SLE_RETURN_CODE_GENERAL_FAILURE,
                            null);
                    return result;
                }
            }
        }
    }

    /**
    * Request Vendor Service Adapter to process the input message that was received
    * from the Unlock Server.
    *
    * @param msg the message that needs to be processed by the SLE service
    * (e.g.: an unlock request)
    *
    * @param imei the device IMEI as bytes
    *
    * @return the formatted response that needs to be sent back to the RSU server or error
    */
    public byte[] processMessage(byte[] msg, byte[] imei) {
        synchronized (RsuManager.class) {
            if (mService == null) {
                loge("processMessage: mService is null!");
                byte[] result = RsuUtils.createErrorResponseBytes(SLE_VENDOR_CODE_MEDIATEK, SLE_FAILURE,
                        SLE_RETURN_CODE_GENERAL_FAILURE, null);

                return result;
            } else {
                try {
                    return mService.processMessage(msg, imei);
                } catch (RemoteException e) {
                    e.printStackTrace();

                    byte[] result = RsuUtils.createErrorResponseBytes(SLE_VENDOR_CODE_MEDIATEK, SLE_FAILURE,
                            SLE_RETURN_CODE_GENERAL_FAILURE, null);

                    return result;
                }
            }
        }
    }

    /**
    * Retrieve the current SIM-Lock status from the SLE.
    *
    * @param data the input data is always null (used for future usages and compatibility)
    *
    * @return the lock state data (lock status and the remaining unlock time
    * in case it is temporarily unlocked or error
    */
    public byte[] getSimlockStatus(byte[] data) {
        synchronized (RsuManager.class) {
            if (mService == null) {
                loge("getSimlockStatus: mService is null!");
                byte[] result = RsuUtils.createErrorResponseBytes(SLE_VENDOR_CODE_MEDIATEK, SLE_FAILURE,
                        SLE_RETURN_CODE_GENERAL_FAILURE, null);

                return result;
            } else {
                try {
                    return mService.getSimlockStatus(data);
                } catch (RemoteException e) {
                    e.printStackTrace();

                    byte[] result = RsuUtils.createErrorResponseBytes(SLE_VENDOR_CODE_MEDIATEK, SLE_FAILURE,
                            SLE_RETURN_CODE_GENERAL_FAILURE, null);

                    return result;
                }
            }
        }
    }

    /**
    * Retrieve the SLE vendor adapter configuration (Unlock Server URL and other specific
    * information for delivering the request messages to the server).
    *
    * @param data the input data that contains the caller version code
    *
    * @return the SLE vendor adapter configuration data
    */
    public byte[] getAdapterConfig(byte[] data) {
        synchronized (RsuManager.class) {
            if (mService == null) {
                loge("getAdapterConfig: mService is null!");
                byte[] result = RsuUtils.createErrorResponseBytes(SLE_VENDOR_CODE_MEDIATEK, SLE_FAILURE,
                        SLE_RETURN_CODE_GENERAL_FAILURE, null);

                return result;
            } else {
                try {
                    return mService.getAdapterConfig(data);
                } catch (RemoteException e) {
                    e.printStackTrace();

                    byte[] result = RsuUtils.createErrorResponseBytes(SLE_VENDOR_CODE_MEDIATEK, SLE_FAILURE,
                            SLE_RETURN_CODE_GENERAL_FAILURE, null);

                    return result;
                }
            }
        }
    }

    private void logd(String s) {
        Log.d(TAG, "[RSU-SIMLOCK] " + s);
    }

    private void loge(String s) {
        Log.e(TAG, "[RSU-SIMLOCK] " + s);
    }
}
