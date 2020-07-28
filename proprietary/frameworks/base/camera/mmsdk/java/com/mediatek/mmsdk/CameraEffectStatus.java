package com.mediatek.mmsdk;

import android.util.Log;

public class CameraEffectStatus {

    private static final String TAG = "CameraEffectStatus";
    private static final boolean DEBUG = true;

    public enum CameraEffectHalStatus {
        STATUS_UNINITIALIZED, STATUS_INITIALIZED, STATUS_CONFINGURED, STATUS_RUNNING,
    }

    private CameraEffectHalStatus mCurrentStatus = CameraEffectHalStatus.STATUS_UNINITIALIZED;

    public CameraEffectStatus() {

    }

    public void setEffectHalStatus(CameraEffectHalStatus status) {
        if (DEBUG) {
            Log.i(TAG, "[" + Thread.currentThread().getStackTrace()[2].getMethodName()
                    + "] ,mCurrentStatus = " + mCurrentStatus + ",next status = " + status);
        }
        mCurrentStatus = status;
    }

    public CameraEffectHalStatus getEffectHalStatus() {
        if (DEBUG) {
            Log.i(TAG, "[" + Thread.currentThread().getStackTrace()[2].getMethodName()
                    + "] ,mCurrentStatus = " + mCurrentStatus);
        }

        return mCurrentStatus;
    }
}
