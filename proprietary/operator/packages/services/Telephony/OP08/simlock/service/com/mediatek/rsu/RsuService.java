package com.mediatek.rsu;

import android.app.ActivityManager;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.Signature;

import android.os.AsyncResult;
import android.os.Binder;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.RemoteException;

import android.text.TextUtils;

import android.util.Log;

import com.mediatek.internal.telephony.uicc.IMtkRsuSml;
import com.mediatek.internal.telephony.uicc.MtkUiccController;

import java.util.Arrays;
import java.util.List;

import com.mediatek.rsu.RsuUtils;
import vendor.mediatek.hardware.mtkradioex.V1_9.RsuResponseInfo;

import static com.mediatek.rsu.RsuUtils.*;

public class RsuService extends Service {
    private static final String TAG = "RsuService:";
    private static final boolean DBG = false;

    public void onCreate() {
        logd("onCreate, thread name = " + Thread.currentThread().getName());
        super.onCreate();
        logd("On create service done");
    }

    public int onStartCommand(Intent intent, int flags, int startId) {
        logd("onStartCommand");
        return START_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        logd("onBind");
        return mBinder;
    }

    public void onRebind(Intent intent) {
        logd("onRebind");
        super.onRebind(intent);
    }

    @Override
    public boolean onUnbind(Intent intent) {
        logd("onUnbind");
        return super.onUnbind(intent);
    }

    public void onDestroy() {
        logd("onDestroy");
        super.onDestroy();
    }

    public void onStop() {
        logd("onStop");
        stopSelf();
    }

    private IRsuService.Stub mBinder = new IRsuService.Stub() {
        /**
         * Request SLE Vendor Service Adapter for the shared symmetric key data that will be
         * sent to the Unlock Server for registration.
         *
         * @param data the input data that contains the IMEI
         * @return the formatted shared symmetric key data or error
         */
        public byte[] getRegisterRequest(byte[] data) {
            if (DBG) {
                logd("getRegisterRequest data: " + RsuUtils.bytesToHexString(data));
            } else {
                logd("getRegisterRequest...");
            }

            final long identity = Binder.clearCallingIdentity();

            try {
                final RsuHelper rsuHelper = new RsuHelper();
                rsuHelper.start();

                return rsuHelper.getRegisterRequest(data);
            } catch (NullPointerException e) {
                loge("getRegisterRequest NullPointerException!");

                byte[] result = createErrorResponseBytes(SLE_VENDOR_CODE_MEDIATEK, SLE_FAILURE,
                        SLE_RETURN_CODE_GENERAL_FAILURE, null);
                return result;
            } finally {
                Binder.restoreCallingIdentity(identity);
            }
        }

        /**
         * Request Vendor Service Adapter to generate and return the formatted unlock
         * request that will be sent to the Unlock Server.
         *
         * @param data the input data that contains the unlock request type and the IMSI
         * @return the formatted unlock request or error
         */
        public byte[] createUnlockRequest(byte[] data) {
            if (DBG) {
                logd("createUnlockRequest data: " + RsuUtils.bytesToHexString(data));
            } else {
                logd("createUnlockRequest...");
            }

            final long identity = Binder.clearCallingIdentity();

            try {
                final RsuHelper rsuHelper = new RsuHelper();
                rsuHelper.start();

                return rsuHelper.createUnlockRequest(data);
            } catch (NullPointerException e) {
                loge("createUnlockRequest NullPointerException!");

                byte[] result = createErrorResponseBytes(SLE_VENDOR_CODE_MEDIATEK, SLE_FAILURE,
                        SLE_RETURN_CODE_GENERAL_FAILURE, null);
                return result;
            } finally {
                Binder.restoreCallingIdentity(identity);
            }
        }

        /**
         * Request Vendor Service Adapter to process the input message that was received
         * from the Unlock Server.
         *
         * @param msg  the message that needs to be processed by the SLE service
         *             (e.g.: an unlock request)
         * @param imei the device IMEI as bytes
         * @return the formatted response that needs to be sent back to the RSU server or error
         */
        public byte[] processMessage(byte[] msg, byte[] imei) {
            if (DBG) {
                logd("processMessage msg: " + RsuUtils.bytesToHexString(msg) + " imei: "
                        + RsuUtils.bytesToHexString(imei));
            } else {
                logd("processMessage...");
            }

            final long identity = Binder.clearCallingIdentity();

            try {
                final RsuHelper rsuHelper = new RsuHelper();
                rsuHelper.start();

                return rsuHelper.processMessage(msg, imei);
            } catch (NullPointerException e) {
                loge("processMessage NullPointerException!");

                byte[] result = createErrorResponseBytes(SLE_VENDOR_CODE_MEDIATEK, SLE_FAILURE,
                        SLE_RETURN_CODE_GENERAL_FAILURE, null);
                return result;
            } finally {
                Binder.restoreCallingIdentity(identity);
            }
        }

        /**
         * Retrieve the current SIM-Lock status from the SLE.
         *
         * @param data the input data is always null (used for future usages and compatibility)
         * @return the lock state data (lock status and the remaining unlock time
         * in case it is temporarily unlocked) or error
         */
        public byte[] getSimlockStatus(byte[] data) {
            if (DBG) {
                logd("getSimlockStatus data: " + RsuUtils.bytesToHexString(data));
            } else {
                logd("getSimlockStatus...");
            }

            final long identity = Binder.clearCallingIdentity();

            try {
                final RsuHelper rsuHelper = new RsuHelper();
                rsuHelper.start();
                return rsuHelper.getSimlockStatus(data);
            } catch (NullPointerException e) {
                loge("getSimlockStatus NullPointerException!");

                byte[] result = createErrorResponseBytes(SLE_VENDOR_CODE_MEDIATEK, SLE_FAILURE,
                        SLE_RETURN_CODE_GENERAL_FAILURE, null);
                return result;
            } finally {
                Binder.restoreCallingIdentity(identity);
            }
        }

        /**
         * Retrieve the SLE vendor adapter configuration (Unlock Server URL and other specific
         * information for delivering the request messages to the server).
         *
         * @param data the input data that contains the caller version code
         * @return the SLE vendor adapter configuration data
         */
        public byte[] getAdapterConfig(byte[] data) {
            if (DBG) {
                logd("getAdapterConfig data: " + RsuUtils.bytesToHexString(data));
            } else {
                logd("getAdapterConfig...");
            }

            final long identity = Binder.clearCallingIdentity();

            try {
                final RsuHelper rsuHelper = new RsuHelper();
                rsuHelper.start();

                return rsuHelper.getAdapterConfig(data);
            } catch (NullPointerException e) {
                loge("getAdapterConfig NullPointerException!");

                byte[] result = createErrorResponseBytes(SLE_VENDOR_CODE_MEDIATEK, SLE_FAILURE,
                        SLE_RETURN_CODE_GENERAL_FAILURE, null);
                return result;
            } finally {
                Binder.restoreCallingIdentity(identity);
            }
        }

        /**
         * Helper thread to turn async call to MtkUiccController into
         * a synchronous one.
         */
        class RsuHelper extends Thread {
            private IMtkRsuSml mMtkRsuSml;

            private boolean mDone = false;
            private byte[] mResult = null;

            // For replies from MtkUiccController interface
            private Handler mHandler;

            // For async handler to identify request type
            private static final int RSU_SLE_BASE = 100;
            private static final int RSU_SLE_GET_REGEGISTER_REQUEST = RSU_SLE_BASE + 1;
            private static final int RSU_SLE_CREATE_UNLOCK_REQUEST = RSU_SLE_BASE + 2;
            private static final int RSU_SLE_PROCESS_MESSAGE = RSU_SLE_BASE + 3;
            private static final int RSU_SLE_GET_SIMLOCK_STATUS = RSU_SLE_BASE + 4;
            private static final int RSU_SLE_GET_ADAPTER_CONFIG = RSU_SLE_BASE + 5;

            public RsuHelper() {
                mMtkRsuSml = ((MtkUiccController)MtkUiccController.getInstance()).getRsuSml("OP08");
            }

            @Override
            public void run() {
                Looper.prepare();
                synchronized (RsuHelper.this) {
                    mHandler = new Handler() {
                        @Override
                        public void handleMessage(Message msg) {
                            AsyncResult ar = (AsyncResult) msg.obj;
                            RsuResponseInfo rri = (RsuResponseInfo) ar.result;

                            logd(" what:" + msg.what + " exception:" + ar.exception + " rri:"
                                    + rri + " errCode:" + (rri != null ? rri.errCode : "NULL"));

                            switch (msg.what) {
                                case RSU_SLE_GET_REGEGISTER_REQUEST:
                                    synchronized (RsuHelper.this) {
                                        if ((ar.exception == null) && (rri != null)) {
                                            byte[] lockData = RsuUtils.hexStringToBytes(rri.data);

                                            mResult = new byte[lockData.length];
                                            System.arraycopy(lockData, 0, mResult, 0,
                                                    lockData.length);
                                        } else {
                                            mResult = createErrorResponseBytes(
                                                    SLE_VENDOR_CODE_MEDIATEK, SLE_FAILURE,
                                                    SLE_RETURN_CODE_GENERAL_FAILURE, null);
                                        }

                                        mDone = true;
                                        RsuHelper.this.notifyAll();
                                    }
                                    break;

                                case RSU_SLE_CREATE_UNLOCK_REQUEST:
                                    synchronized (RsuHelper.this) {
                                        if ((ar.exception == null) && (rri != null)) {
                                            byte[] lockData = RsuUtils.hexStringToBytes(rri.data);

                                            mResult = new byte[lockData.length];
                                            System.arraycopy(lockData, 0, mResult, 0,
                                                    lockData.length);
                                        } else {
                                            mResult = createErrorResponseBytes(
                                                    SLE_VENDOR_CODE_MEDIATEK, SLE_FAILURE,
                                                    SLE_RETURN_CODE_GENERAL_FAILURE, null);
                                        }

                                        mDone = true;
                                        RsuHelper.this.notifyAll();
                                    }
                                    break;

                                case RSU_SLE_PROCESS_MESSAGE:
                                    synchronized (RsuHelper.this) {
                                        if ((ar.exception == null) && (rri != null)) {
                                            byte[] lockData = RsuUtils.hexStringToBytes(rri.data);

                                            mResult = new byte[lockData.length];
                                            System.arraycopy(lockData, 0, mResult, 0,
                                                    lockData.length);
                                        } else {
                                            mResult = createErrorResponseBytes(
                                                    SLE_VENDOR_CODE_MEDIATEK, SLE_FAILURE,
                                                    SLE_RETURN_CODE_GENERAL_FAILURE, null);
                                        }

                                        mDone = true;
                                        RsuHelper.this.notifyAll();
                                    }
                                    break;

                                case RSU_SLE_GET_SIMLOCK_STATUS:
                                    synchronized (RsuHelper.this) {
                                        if ((ar.exception == null) && (rri != null)) {
                                            byte[] lockData = RsuUtils.hexStringToBytes(rri.data);

                                            mResult = new byte[lockData.length];
                                            System.arraycopy(lockData, 0, mResult, 0,
                                                    lockData.length);
                                        } else {
                                            mResult = createErrorResponseBytes(
                                                    SLE_VENDOR_CODE_MEDIATEK, SLE_FAILURE,
                                                    SLE_RETURN_CODE_GENERAL_FAILURE, null);
                                        }

                                        mDone = true;
                                        RsuHelper.this.notifyAll();
                                    }
                                    break;

                                case RSU_SLE_GET_ADAPTER_CONFIG:
                                    synchronized (RsuHelper.this) {
                                        if ((ar.exception == null) && (rri != null)) {
                                            byte[] lockData = RsuUtils.hexStringToBytes(rri.data);

                                            mResult = new byte[lockData.length];
                                            System.arraycopy(lockData, 0, mResult, 0,
                                                    lockData.length);
                                        } else {
                                            mResult = createErrorResponseBytes(
                                                    SLE_VENDOR_CODE_MEDIATEK, SLE_FAILURE,
                                                    SLE_RETURN_CODE_GENERAL_FAILURE, null);
                                        }

                                        mDone = true;
                                        RsuHelper.this.notifyAll();
                                    }
                                    break;

                                default:
                                    break;
                            }
                        }
                    };
                    RsuHelper.this.notifyAll();
                }
                Looper.loop();
            }

            synchronized byte[] getRegisterRequest(byte[] data) {
                while (mHandler == null) {
                    try {
                        wait();
                    } catch (InterruptedException e) {
                        Thread.currentThread().interrupt();
                    }
                }
                Message callback = Message.obtain(mHandler, RSU_SLE_GET_REGEGISTER_REQUEST);

                mMtkRsuSml.sleGetRegisterRequest(data, callback);

                while (!mDone) {
                    try {
                        logd("getRegisterRequest, wait for done");
                        wait();
                    } catch (InterruptedException e) {
                        // Restore the interrupted status
                        Thread.currentThread().interrupt();
                    }
                }
                logd("getRegisterRequest, done");

                return mResult;
            }

            synchronized byte[] createUnlockRequest(byte[] data) {
                while (mHandler == null) {
                    try {
                        wait();
                    } catch (InterruptedException e) {
                        Thread.currentThread().interrupt();
                    }
                }
                Message callback = Message.obtain(mHandler, RSU_SLE_CREATE_UNLOCK_REQUEST);

                mMtkRsuSml.sleCreateUnlockRequest(data, callback);

                while (!mDone) {
                    try {
                        logd("createUnlockRequest, wait for done");
                        wait();
                    } catch (InterruptedException e) {
                        // Restore the interrupted status
                        Thread.currentThread().interrupt();
                    }
                }
                logd("createUnlockRequest, done");

                return mResult;
            }

            synchronized byte[] processMessage(byte[] msg, byte[] imei) {
                while (mHandler == null) {
                    try {
                        wait();
                    } catch (InterruptedException e) {
                        Thread.currentThread().interrupt();
                    }
                }
                Message callback = Message.obtain(mHandler, RSU_SLE_PROCESS_MESSAGE);

                mMtkRsuSml.sleProcessMessage(msg, imei, callback);

                while (!mDone) {
                    try {
                        logd("processMessage, wait for done");
                        wait();
                    } catch (InterruptedException e) {
                        // Restore the interrupted status
                        Thread.currentThread().interrupt();
                    }
                }
                logd("processMessage, done");

                return mResult;
            }

            synchronized byte[] getSimlockStatus(byte[] data) {
                while (mHandler == null) {
                    try {
                        wait();
                    } catch (InterruptedException e) {
                        Thread.currentThread().interrupt();
                    }
                }
                Message callback = Message.obtain(mHandler, RSU_SLE_GET_SIMLOCK_STATUS);

                mMtkRsuSml.sleGetSimlockStatus(data, callback);

                while (!mDone) {
                    try {
                        logd("getSimlockStatus, wait for done");
                        wait();
                    } catch (InterruptedException e) {
                        // Restore the interrupted status
                        Thread.currentThread().interrupt();
                    }
                }
                logd("getSimlockStatus, done");

                return mResult;
            }

            synchronized byte[] getAdapterConfig(byte[] data) {
                while (mHandler == null) {
                    try {
                        wait();
                    } catch (InterruptedException e) {
                        Thread.currentThread().interrupt();
                    }
                }
                Message callback = Message.obtain(mHandler, RSU_SLE_GET_ADAPTER_CONFIG);

                mMtkRsuSml.sleGetAdapterConfig(data, callback);

                while (!mDone) {
                    try {
                        logd("getAdapterConfig, wait for done");
                        wait();
                    } catch (InterruptedException e) {
                        // Restore the interrupted status
                        Thread.currentThread().interrupt();
                    }
                }
                logd("getAdapterConfig, done");

                return mResult;
            }
        }
    };

    private String getCallerPackageName() {
        int callerPid = Binder.getCallingPid();
        int callerUid = Binder.getCallingUid();
        String packageName = RsuService.this.getPackageManager().getNameForUid(callerUid);
        if (TextUtils.isEmpty(packageName)) {
            ActivityManager activityManager = (ActivityManager) RsuService.this.getSystemService(
                    Context.ACTIVITY_SERVICE);
            if (activityManager != null) {
                List<ActivityManager.RunningAppProcessInfo> list = activityManager
                        .getRunningAppProcesses();
                for (ActivityManager.RunningAppProcessInfo info : list) {
                    if (info.pid == callerPid) {
                        packageName = info.processName;
                        break;
                    }
                }
            }
        }
        logd("getCallerPackageName callerPid = " + callerPid + "callerUid = " + callerUid
                + " processName = " + packageName);
        return packageName;
    }

    private static final String PREBUILT_SIGNATURE = "";

    private void printCallerSigHash() {
        byte[] signatureHash = RsuUtils.getSignatureHash(this, getCallerPackageName());
        logd("caller signature: " + RsuUtils.bytesToHexString(signatureHash));
    }

    private boolean enforcePermission() {
        byte[] signatureHash = RsuUtils.getSignatureHash(this, getCallerPackageName());
        return Arrays.equals(signatureHash, RsuUtils.hexStringToBytes(PREBUILT_SIGNATURE));
    }

    private void logd(String s) {
        Log.d(TAG, "[RSU-SIMLOCK] " + s);
    }

    private void loge(String s) {
        Log.e(TAG, "[RSU-SIMLOCK] " + s);
    }
}
