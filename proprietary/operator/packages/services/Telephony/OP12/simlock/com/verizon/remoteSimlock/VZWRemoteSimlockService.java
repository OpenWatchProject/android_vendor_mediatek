package com.verizon.remoteSimlock;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;

import com.android.internal.telephony.uicc.IccUtils;
import com.mediatek.internal.telephony.uicc.IMtkRsuSmlCallback;
import com.mediatek.internal.telephony.uicc.MtkUiccController;

public class VZWRemoteSimlockService extends Service {
    private static final String TAG = "VZWRemoteSimlockService:";

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

    private IVZWRemoteSimlockService.Stub mBinder = new IVZWRemoteSimlockService.Stub() {
        private IVZWRemoteSimlockServiceCallback mVcb = null;
        private MtkRsuSmlCallback mMcb = null;
        private IVZWRemoteSimlockServiceCallback mVcbTemp = null;
        private MtkRsuSmlCallback mMcbTemp = null;

        @Override
        public synchronized int registerCallback(IVZWRemoteSimlockServiceCallback cb) {
            logi("registerCallback IVZWRemoteSimlockServiceCallback = " + cb);
            if (cb == null) {
                return REMOTE_SIM_UNLOCK_ERROR;
            }
            try {
                mVcbTemp = cb;
                mMcbTemp = new MtkRsuSmlCallback(cb);
                mRsuResult = ((MtkUiccController) MtkUiccController.getInstance()).getRsuSml()
                        .registerCallback(mMcbTemp);
                if (mRsuResult == REMOTE_SIM_UNLOCK_ERROR) {
                    logi("registerCallback IVZWRemoteSimlockServiceCallback fail: "
                            + "mVcbTemp = " + mVcbTemp + ", mMcbTemp = " + mMcbTemp
                            + "keep previous register: mVcb = " + mVcb + ", mMcb = " + mMcb);
                } else {
                    mVcb = mVcbTemp;
                    mMcb = mMcbTemp;
                }
            } catch (NullPointerException e) {
                mRsuResult = REMOTE_SIM_UNLOCK_ERROR;
                loge("registerCallback NullPointerException, keep previous register: mVcb = " + mVcb
                        + ", mMcb = " + mMcb);
            }
            logi("registerCallback mVcb = " + mVcb + ", mMcb = " + mMcb);
            return mRsuResult;
        }

        @Override
        public synchronized int deregisterCallback(IVZWRemoteSimlockServiceCallback cb) {
            logi("deregisterCallback IVZWRemoteSimlockServiceCallback = " + cb);
            if (cb == null) {
                return REMOTE_SIM_UNLOCK_ERROR;
            }
            try {
                mRsuResult = ((MtkUiccController) MtkUiccController.getInstance()).getRsuSml()
                        .deregisterCallback(mMcb);
                if (mRsuResult == REMOTE_SIM_UNLOCK_ERROR) {
                    logi("deregisterCallback IVZWRemoteSimlockServiceCallback fail: "
                            + "Shouldn't remove current mMcb = " + mMcb);
                } else {
                    /* Because deregister by binder, so you cannot know whether cb is the the right
                       deregister cb. So deregister the active cb. that is to clear.*/
                    mVcb = null;
                    mMcb = null;
                }
            } catch (NullPointerException e) {
                mRsuResult = REMOTE_SIM_UNLOCK_ERROR;
                loge("deregisterCallback NullPointerException, keep current mMcb = " + mMcb);
            }
            logi("deregisterCallback mVcb = " + mVcb + ", mMcb = " + mMcb);
            return mRsuResult;
        }

        @Override
        public int remoteSimlockProcessSimlockData(int token, byte[] data) {
            logi("remoteSimlockProcessSimlockData data = " + bytes2Hexs(data) + " token = "
                    + token);
            try {
                mRsuResult = ((MtkUiccController) MtkUiccController.getInstance()).getRsuSml()
                        .remoteSimlockProcessSimlockData(token, data);
            } catch (NullPointerException e) {
                mRsuResult = REMOTE_SIM_UNLOCK_ERROR;
                loge("remoteSimlockProcessSimlockData NullPointerException");
            }
            return mRsuResult;
        }

        @Override
        public int remoteSimlockGetVersion(int token) {
            logi("remoteSimlockGetVersion token = " + token);
            try {
                mRsuResult = ((MtkUiccController) MtkUiccController.getInstance()).getRsuSml()
                        .remoteSimlockGetVersion(token);
            } catch (NullPointerException e) {
                mRsuResult = REMOTE_SIM_UNLOCK_ERROR;
                loge("remoteSimlockGetVersion NullPointerException");
            }
            return mRsuResult;
        }

        @Override
        public int remoteSimlockGetSimlockStatus(int token) {
            logi("remoteSimlockGetSimlockStatus token = " + token);
            try {
                mRsuResult = ((MtkUiccController) MtkUiccController.getInstance()).getRsuSml()
                        .remoteSimlockGetSimlockStatus(token);
            } catch (NullPointerException e) {
                mRsuResult = REMOTE_SIM_UNLOCK_ERROR;
                loge("remoteSimlockGetSimlockStatus NullPointerException");
            }
            return mRsuResult;
        }

        @Override
        public int remoteSimlockGenerateRequest(int token, int requestType) {
            logi("remoteSimlockGenerateRequest token = " + token + " requestType = " + requestType);
            try {
                mRsuResult = ((MtkUiccController) MtkUiccController.getInstance()).getRsuSml()
                        .remoteSimlockGenerateRequest(token, requestType);
            } catch (NullPointerException e) {
                mRsuResult = REMOTE_SIM_UNLOCK_ERROR;
                loge("remoteSimlockGenerateRequest NullPointerException");
            }
            return mRsuResult;
        }

        @Override
        public int remoteSimlockUnlockTimer(int token, int requestType) {
            logi("remoteSimlockUnlockTimer token = " + token + " requestType = " + requestType);
            try {
                mRsuResult = ((MtkUiccController) MtkUiccController.getInstance()).getRsuSml()
                        .remoteSimlockUnlockTimer(token, requestType);
            } catch (NullPointerException e) {
                mRsuResult = REMOTE_SIM_UNLOCK_ERROR;
                loge("remoteSimlockUnlockTimer NullPointerException");
            }
            return mRsuResult;
        }

        private String bytes2Hexs(byte[] bytes) {
            return IccUtils.bytesToHexString(bytes);
        }

        class MtkRsuSmlCallback extends IMtkRsuSmlCallback {
            private IVZWRemoteSimlockServiceCallback mCb = null;
            public MtkRsuSmlCallback(IVZWRemoteSimlockServiceCallback cb) {
                logi("MtkRsuSmlCallback constructor cb = " + cb + ", this = " + this);
                mCb = cb;
            }

            @Override
            public void remoteSimlockProcessSimlockDataResponse(int token, int responseCode,
                        byte[] simlockResponse) {
                logi("remoteSimlockProcessSimlockDataResponse simlockResponse = "
                        + bytes2Hexs(simlockResponse) + " token = " + token + " responseCode = "
                        + responseCode + " mCb = " + mCb);
                if (mCb != null) {
                    try {
                        mCb.remoteSimlockProcessSimlockDataResponse(token, responseCode,
                                simlockResponse);
                    } catch (RemoteException e) {
                        loge("RemoteException");
                    }
                }
            }

            @Override
            public void remoteSimlockGetVersionResponse(int token, int responseCode,
                        int majorVersion, int minorVersion) {
                logi("remoteSimlockGetVersionResponse majorVersion = " + majorVersion
                        + " token = " + token + " responseCode = " + responseCode
                        + " minorVersion = " + minorVersion + " mCb = " + mCb);
                if (mCb != null) {
                    try {
                        mCb.remoteSimlockGetVersionResponse(token, responseCode, majorVersion,
                                minorVersion);
                    } catch (RemoteException e) {
                        loge("RemoteException");
                    }
                }
            }

            @Override
            public void remoteSimlockGetSimlockStatusResponse(int token, int responseCode,
                    int unlockStatus, long unlockTime) {
                logi("remoteSimlockGetSimlockStatusResponse unlockStatus = "
                        + unlockStatus + " token = " + token + " responseCode = " + responseCode
                        + " unlockTime = " + unlockTime + " mCb = " + mCb);
                if (mCb != null) {
                    try {
                        mCb.remoteSimlockGetSimlockStatusResponse(token, responseCode,
                            unlockStatus, unlockTime);
                    } catch (RemoteException e) {
                        loge("RemoteException");
                    }
                }
            }

            @Override
            public void remoteSimlockGenerateRequestResponse(int token, int responseCode,
                    byte[] requestData) {
                logi("remoteSimlockGenerateRequestResponse requestData = "
                        + bytes2Hexs(requestData) + " token = " + token + " responseCode = "
                        + responseCode + " mCb = " + mCb);
                if (mCb != null) {
                    try {
                        mCb.remoteSimlockGenerateRequestResponse(token, responseCode, requestData);
                    } catch (RemoteException e) {
                        loge("RemoteException");
                    }
                }
            }

            @Override
            public void remoteSimlockUnlockTimerResponse(int token, int responseCode,
                    int timerValue) {
                logi("remoteSimlockGenerateRequestResponse timerValue = "
                        + timerValue + " token = " + token + " responseCode = " + responseCode
                        + " mCb = " + mCb);
                if (mCb != null) {
                    try {
                        mCb.remoteSimlockUnlockTimerResponse(token, responseCode, timerValue);
                    } catch (RemoteException e) {
                        loge("RemoteException");
                    }
                }
            }
        }
    };

    private void logi(String s) {
        Log.i(TAG, "[RSU-SIMLOCK] " + s);
    }

    private void loge(String s) {
        Log.e(TAG, "[RSU-SIMLOCK] " + s);
    }
}
