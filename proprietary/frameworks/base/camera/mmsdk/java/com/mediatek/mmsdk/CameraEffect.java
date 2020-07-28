package com.mediatek.mmsdk;

import android.os.Handler;
import android.view.Surface;

import com.mediatek.mmsdk.CameraEffectHalException;

import java.util.List;

public abstract class CameraEffect implements AutoCloseable {

    /**
     * Create the effect session.
     * 
     * @param outputs
     *            The new set of Surfaces that should be made available as
     *            targets for captured image data.
     * @param surfaceParameters
     *            the parameters which is need for the surface.
     * @param callback
     *            The callback is be notified about the status of the new
     *            capture session.
     * @param handler
     *            The handler on which the callback should be invoked, or
     *            {@code null} to use the current thread's
     *            {@link android.os.Looper looper}.
     * @return Return the current created CameraEffectSession.
     * @throws CameraEffectHalException
     *             if the effect HAL cann't be configured or in error state.
     */
    public abstract CameraEffectSession createCaptureSession(List<Surface> outputs,
            List<BaseParameters> surfaceParameters,
            CameraEffectSession.SessionStateCallback callback, Handler handler)
            throws CameraEffectHalException;

    /**
     * Change the settings for Effect HAL.
     * 
     * @param baseParameters
     *            the parameters to use for this Effect Hal.
     */
    public abstract void setParamters(BaseParameters baseParameters);
    
    /**
     * Get the input surface which is created by EffectHalClient,the set of
     * surface will be set to cameraService.when you have called this function
     * ,you can not change the parameters that influence the picture size and
     * surface format.
     * 
     * @return return the set of surface which is created by EffectHalClient.
     */
    public abstract List<Surface> getInputSurface();

    /**
     * Get the parameters which need for current effect.
     * 
     * @param parameters
     *            set the parameters to effect HAL.so Effect Hal will prepare
     *            the picture with this parameters.
     * @return according the set parameters will return other parameters must be
     *         change.
     */
    public abstract List<BaseParameters> getCaputreRequirement(BaseParameters parameters);

    /**
     * Close current Effect.
     */
    public abstract void closeEffect();

    /**
     * A callback objects for receiving updates about the state of a camera
     * Effect Hal
     */
    public static abstract class StateCallback {

        /**
         * indicate the effect hal is in use already.
         */
        public static final int ERROR_EFFECT_HAL_IN_USE = 1;

        /**
         * indicate the effect hal cannot be used due to Effect Hal in error
         * state or RemoteException.
         */
        public static final int ERROR_EFFECT_DISABLED = 3;

        /**
         * indicate the effect Hal is in error.
         */
        public static final int ERROR_EFFECT_DEVICE = 4;

        /**
         * indicate set the effect Hal listener is error.
         */
        public static final int ERROR_EFFECT_LISTENER = 6;

        /**
         * The method called when Effect Hal is no longer available for use.
         * 
         * @param effect
         *            the Effect Hal that has been disconnected
         */
        public abstract void onDisconnected(CameraEffect effect);

        /**
         * The method called when a camera device has encountered a serious
         * error.
         * 
         * @param effect
         *            the Effect Hal that has been in error state
         * @param error
         *            the detail error,see the :
         *            ERROR_EFFECT_HAL_IN_USE,ERROR_EFFECT_DISABLED
         *            ,ERROR_EFFECT_DEVICE,ERROR_EFFECT_LISTENER
         */
        public abstract void onError(CameraEffect effect, int error);

        /**
         * The method called when Effect Hal has been closed.
         * 
         * @param effect
         *            the Effect Hal that has been closed.
         */
        public void onClosed(CameraEffect effect) {
            // Default empty implementation
        }
    }

    @Override
    public abstract void close();
}
