package com.mediatek.mmsdk;

import com.mediatek.mmsdk.CameraEffectHalException;

@SuppressWarnings("serial")
public class CameraEffectHalRuntimeException extends RuntimeException {
    private final int mReason;
    private String mMessage;
    private Throwable mCause;
    
    public final int getReason() {
        return mReason;
    }
    
    public CameraEffectHalRuntimeException(int problem) {
        super();
        mReason = problem;
    }
    
    public CameraEffectHalRuntimeException(int problem,String msg) {
        super(msg);
        mReason = problem;
        mMessage = msg;
    }
    
    public CameraEffectHalRuntimeException(int problem, String msg, Throwable throwable) {
        super(msg,throwable);
        mReason = problem;
        mMessage = msg;
        mCause = throwable;
    }
    
    public CameraEffectHalRuntimeException(int problem, Throwable cause) {
        super(cause);
        mReason = problem;
        mCause = cause;
    }
    
    public CameraEffectHalException asChecked() {
        CameraEffectHalException e;
        if (mMessage != null && mCause != null) {
            e = new CameraEffectHalException(mReason,mMessage,mCause);
        } else if (mMessage != null) {
            e = new CameraEffectHalException(mReason, mMessage);
        } else if (mCause != null) {
            e = new CameraEffectHalException(mReason, mCause);
        } else {
            e = new CameraEffectHalException(mReason);
        }
        
        e.setStackTrace(this.getStackTrace());
        return e;
    }
}
