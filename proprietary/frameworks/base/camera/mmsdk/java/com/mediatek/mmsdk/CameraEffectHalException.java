package com.mediatek.mmsdk;

import android.hardware.camera2.CameraDevice;
import android.util.AndroidException;


//this class used for EffectHalException,the exception need see the android errors.h tag
//the errors.h file :/system/core/include/utils/Errors.h
//TODO
public class CameraEffectHalException extends AndroidException{


    //TODO Need Check
    public enum EffectHalError {
        EFFECT_HAL_SERVICE_ERROR,
        EFFECT_HAL_FEATUREMANAGER_ERROR,
        EFFECT_HAL_FACTORY_ERROR,
        EFFECT_HAL_ERROR,
        EFFECT_HAL_CLIENT_ERROR,
        EFFECT_HAL_LISTENER_ERROR,
        EFFECT_HAL_IN_USE,
    }
    
    public static enum EffectHalStatusError {
        EFFECT_INITIAL_ERROR,
    }
    //***************************************************************************
    //***************************************************************************
    //*************************EffectHal Error***********************************
    //***************************************************************************
    //***************************************************************************
    /**
     * The camera is disabled due to a device policy, and cannot be opened.
     *
     * @see android.app.admin.DevicePolicyManager#setCameraDisabled(android.content.ComponentName, boolean)
     */
    public static final int EFFECT_HAL_SERVICE_ERROR = 101;

    /**
     * The camera device is removable and has been disconnected from the Android
     * device, or the camera id used with {@link android.hardware.camera2.CameraManager#openCamera}
     * is no longer valid, or the camera service has shut down the connection due to a
     * higher-priority access request for the camera device.
     */
    public static final int EFFECT_HAL_FEATUREMANAGER_ERROR = 102;

    /**
     * The camera device is currently in the error state.
     *
     * <p>The camera has failed to open or has failed at a later time
     * as a result of some non-user interaction. Refer to
     * {@link CameraDevice.StateCallback#onError} for the exact
     * nature of the error.</p>
     *
     * <p>No further calls to the camera will succeed. Clean up
     * the camera with {@link CameraDevice#close} and try
     * handling the error in order to successfully re-open the camera.
     * </p>
     *
     */
    public static final int EFFECT_HAL_FACTORY_ERROR = 103;

    /**
     * The camera device is in use already
     * @hide
     */
    public static final int EFFECT_HAL_ERROR = 104;

    /**
     * The system-wide limit for number of open cameras has been reached,
     * and more camera devices cannot be opened until previous instances are
     * closed.
     * @hide
     */
    public static final int EFFECT_HAL_CLIENT_ERROR = 105;
    
    public static final int EFFECT_HAL_LISTENER_ERROR = 106;
    
    public static final int EFFECT_HAL_IN_USE = 107;
    

    //***************************************************************************
    //***************************************************************************
    //*************************status error**************************************
    //***************************************************************************
    //***************************************************************************
    
    public static final int EFFECT_INITIAL_ERROR = 201;
    
    private final int mReason;
    
    public final int getReason() {
        return mReason;
    }
    
    public CameraEffectHalException(int problem) {
        super(getDefaultMessage(problem));
        mReason = problem;
    }
    
    public CameraEffectHalException(int problem, String msg) {
        super(msg);
        mReason = problem;
    }
    
    public CameraEffectHalException(int problem, String msg, Throwable throwable) {
        super(msg,throwable);
        mReason = problem;
    }
    
    public CameraEffectHalException(int problem, Throwable throwable) {
        super(getDefaultMessage(problem),throwable);
        mReason = problem;
    }
    
    //TODO Need improve this message
    public static String getDefaultMessage(int problem) {
        String message = null;
        switch (problem) {
        case EFFECT_HAL_IN_USE:
            message = "The camera hal is in use already";
            
        case EFFECT_HAL_FEATUREMANAGER_ERROR:
            message = "The camera device is removable and has been disconnected from the " +
                    "Android device, or the camera service has shut down the connection due " +
                    "to a higher-priority access request for the camera device.";
            
        case EFFECT_HAL_SERVICE_ERROR:
            message = "The camera is disabled due to a device policy, and cannot be opened.";
            
        case EFFECT_HAL_FACTORY_ERROR:
            message = "The camera device is currently in the error state; " +
                    "no further calls to it will succeed.";
        default:
            message = "the problem type not in the camera hal,please add that in CameraEffectHalException ";
        }
        return message;
    }

}
