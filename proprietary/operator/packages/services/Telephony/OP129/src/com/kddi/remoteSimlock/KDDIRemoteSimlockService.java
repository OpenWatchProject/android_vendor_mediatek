package com.kddi.remoteSimlock;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;

import com.android.internal.telephony.uicc.IccUtils;
import com.mediatek.internal.telephony.uicc.MtkUiccController;

public class KDDIRemoteSimlockService extends Service {
    private static final String TAG = "KDDIRemoteSimlockService:";

    private static final int REMOTE_SIM_UNLOCK_ERROR = 1;

    private int mRsuResult = 1;

    public void onCreate() {
        logi("onCreate, thread name = " + Thread.currentThread().getName());
        super.onCreate();
        logi("On create service done");
    }

    public int onStartCommand(Intent intent, int flags, int startId) {
        logi("onStartCommand");
        return START_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        logi("onBind");
        return mBinder;
    }

    public void onRebind(Intent intent) {
        logi("onRebind");
        super.onRebind(intent);
    }

    @Override
    public boolean onUnbind(Intent intent) {
        logi("onUnbind");
        return super.onUnbind(intent);
    }

    public void onDestroy() {
        logi("onDestroy");
        super.onDestroy();
    }

    public void onStop() {
        logi("onStop");
        stopSelf();
    }

    private IKDDIRemoteSimlockService.Stub mBinder = new IKDDIRemoteSimlockService.Stub() {
        @Override
        public synchronized int registerCallback(IKDDIRemoteSimlockServiceCallback cb) {
            logi("registerCallback IKDDIRemoteSimlockServiceCallback = " + cb);
            try {
                mRsuResult = ((MtkUiccController) MtkUiccController.getInstance()).getRsuSml()
                        .registerCallback(cb);
            } catch (NullPointerException e) {
                mRsuResult = REMOTE_SIM_UNLOCK_ERROR;
                loge("registerCallback NullPointerException");
            }
            return mRsuResult;
        }

        @Override
        public synchronized int deregisterCallback(IKDDIRemoteSimlockServiceCallback cb) {
            logi("deregisterCallback IKDDIRemoteSimlockServiceCallback = " + cb);
            try {
                mRsuResult = ((MtkUiccController) MtkUiccController.getInstance()).getRsuSml()
                        .deregisterCallback(cb);
            } catch (NullPointerException e) {
                mRsuResult = REMOTE_SIM_UNLOCK_ERROR;
                loge("deregisterCallback NullPointerException");
            }
            return mRsuResult;
        }

        @Override
        public int setSimLockPolicyData(int token, byte[] data, int slotId) {
            logi("setSimLockPolicyData data = " + bytes2Hexs(data) + " token = "
                    + token + " slotId = " + slotId);
            try {
                mRsuResult = ((MtkUiccController) MtkUiccController.getInstance()).getRsuSml()
                        .setSimLockPolicyData(token, data, slotId);
            } catch (NullPointerException e) {
                mRsuResult = REMOTE_SIM_UNLOCK_ERROR;
                loge("setSimLockPolicyData NullPointerException");
            }
            return mRsuResult;
        }

        @Override
        public int getAllowedSimLockData(int token, int slotId) {
            logi("getAllowedSimLockData token = " + token + " slotId = " + slotId);
            try {
                mRsuResult = ((MtkUiccController) MtkUiccController.getInstance()).getRsuSml()
                        .getAllowedSimLockData(token, slotId);
            } catch (NullPointerException e) {
                mRsuResult = REMOTE_SIM_UNLOCK_ERROR;
                loge("getAllowedSimLockData NullPointerException");
            }
            return mRsuResult;
        }

        @Override
        public int getExcludedSimLockData(int token, int slotId) {
            logi("getExcludedSimLockData token = " + token + " slotId = " + slotId);
            try {
                mRsuResult = ((MtkUiccController) MtkUiccController.getInstance()).getRsuSml()
                        .getExcludedSimLockData(token, slotId);
            } catch (NullPointerException e) {
                mRsuResult = REMOTE_SIM_UNLOCK_ERROR;
                loge("getExcludedSimLockData NullPointerException");
            }
            return mRsuResult;
        }

        private String bytes2Hexs(byte[] bytes) {
            return IccUtils.bytesToHexString(bytes);
        }
    };
    private void logi(String s) {
        Log.e(TAG, "[RSU-SIMLOCK] " + s);
    }

    private void loge(String s) {
        Log.e(TAG, "[RSU-SIMLOCK] " + s);
    }
}
