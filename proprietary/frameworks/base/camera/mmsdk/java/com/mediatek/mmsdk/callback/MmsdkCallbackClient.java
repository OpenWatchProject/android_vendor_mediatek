package com.mediatek.mmsdk.callback;

import android.content.Context;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.util.Log;
import android.view.Surface;

import com.mediatek.mmsdk.BaseParameters;
import com.mediatek.mmsdk.BinderHolder;
import com.mediatek.mmsdk.CameraEffectHalException;
import com.mediatek.mmsdk.CameraEffectHalRuntimeException;
import com.mediatek.mmsdk.EffectHalVersion;
import com.mediatek.mmsdk.IEffectFactory;
import com.mediatek.mmsdk.IFeatureManager;
import com.mediatek.mmsdk.IMMSdkService;

import java.util.List;

/**
 * Mmsdk callback client for stereo data callback to application.
 */
public class MmsdkCallbackClient {

    private static final String TAG = "MmsdkCallbackClient";
    public static final int CAMERA_MSG_COMPRESSED_IMAGE = 0x100;
    private IMMSdkService mIMmsdkService;
    private IFeatureManager mIFeatureManager;
    private IEffectFactory mIEffectFactory;
    private ICallbackClient mICallbackClient;

    /**
     * Constructor.
     * @param context application context.
     */
    public MmsdkCallbackClient(Context context) {
    }

    /**
     * Check callback client is support or not.
     *
     * @return true, if the callback client is support, otherwise will return
     *         false.
     */
    public boolean isCallbackClientSupported() {
        boolean isSupport = false;
        try {
            isSupport = getEffectFactory() != null && isCallbackSupported();
        } catch (CameraEffectHalException e) {
            Log.e(TAG, "Current not support Effect HAl", e);
        }
        return isSupport;
    }

    /**
     * Start callback client.
     * @throws CameraEffectHalException CameraEffectHalException
     */
    public void start() throws CameraEffectHalException {
        init();
        try {
            mICallbackClient.start();
        } catch (RemoteException e1) {
            Log.e(TAG, "RemoteException during start", e1);
            throw new CameraEffectHalException(CameraEffectHalException.EFFECT_INITIAL_ERROR);
        }
    }

    /**
     * Stop callback client.
     * @throws CameraEffectHalException CameraEffectHalException
     */
    public void stop() throws CameraEffectHalException {
        try {
            mICallbackClient.stop();
        } catch (RemoteException e1) {
            Log.e(TAG, "RemoteException during stop", e1);
            throw new CameraEffectHalException(CameraEffectHalException.EFFECT_HAL_ERROR);
        }
    }

    /**
     * Set output surfaces to mmsdk for buffer needed.
     * @param outputs outputs
     * @param parameters parameters
     * @throws CameraEffectHalException CameraEffectHalException
     */
    public void setOutputSurfaces(List<Surface> outputs, List<BaseParameters> parameters)
            throws CameraEffectHalException {
        try {
            mICallbackClient.setOutputSurfaces(outputs, parameters);
        } catch (RemoteException e) {
            Log.e(TAG, "RemoteException during set Listener", e);

            CameraEffectHalRuntimeException exception = new CameraEffectHalRuntimeException(
                    CameraEffectHalException.EFFECT_HAL_LISTENER_ERROR);
            throw exception.asChecked();
        }
    }

    private void init() throws CameraEffectHalException {
        // <Step1> first get the camera MM service
        getMmSdkService();
        // <Step2> get the Feature Manager
        getFeatureManager();

        // <Step3> get the Effect Factory
        getEffectFactory();

        // <Step4> get the effect HAL Client
        EffectHalVersion version = new EffectHalVersion();
        mICallbackClient = createCallbackClient(version);
    }

    private IMMSdkService getMmSdkService() throws CameraEffectHalException {
        if (mIMmsdkService == null) {
            IBinder mmsdkService = ServiceManager
                    .getService(BaseParameters.CAMERA_MM_SERVICE_BINDER_NAME);
            if (mmsdkService == null) {
                throw new CameraEffectHalException(
                        CameraEffectHalException.EFFECT_HAL_SERVICE_ERROR);
            }
            mIMmsdkService = IMMSdkService.Stub.asInterface(mmsdkService);
        }

        return mIMmsdkService;
    }

    private boolean isCallbackSupported() throws CameraEffectHalException {
        getMmSdkService();
        boolean isSupport;
        try {
            isSupport = mIMmsdkService.existCallbackClient() == 1 ? true : false;
        } catch (RemoteException e) {
            throw new CameraEffectHalException(CameraEffectHalException.EFFECT_HAL_SERVICE_ERROR);
        }
        return isSupport;
    }

    private IFeatureManager getFeatureManager() throws CameraEffectHalException {
        getMmSdkService();
        if (mIFeatureManager == null) {
            BinderHolder featureManagerHolder = new BinderHolder();
            try {
                mIMmsdkService.connectFeatureManager(featureManagerHolder);
            } catch (RemoteException e) {
                throw new CameraEffectHalException(
                        CameraEffectHalException.EFFECT_HAL_FEATUREMANAGER_ERROR);
            }
            mIFeatureManager = IFeatureManager.Stub.asInterface(featureManagerHolder.getBinder());
        }
        return mIFeatureManager;
    }

    private IEffectFactory getEffectFactory() throws CameraEffectHalException {
        getFeatureManager();
        if (mIEffectFactory == null) {
            BinderHolder effectFactoryHolder = new BinderHolder();
            try {
                mIFeatureManager.getEffectFactory(effectFactoryHolder);
            } catch (RemoteException e) {
                throw new CameraEffectHalException(
                        CameraEffectHalException.EFFECT_HAL_FACTORY_ERROR);
            }
            mIEffectFactory = IEffectFactory.Stub.asInterface(effectFactoryHolder.getBinder());
        }
        return mIEffectFactory;
    }

    private ICallbackClient createCallbackClient(EffectHalVersion version)
            throws CameraEffectHalException {
        getEffectFactory();
        BinderHolder callbackClientHolder = new BinderHolder();
        try {
            mIEffectFactory.createCallbackClient(version, callbackClientHolder);
        } catch (RemoteException e) {
            throw new CameraEffectHalException(CameraEffectHalException.EFFECT_HAL_CLIENT_ERROR);
        }
        return ICallbackClient.Stub.asInterface(callbackClientHolder.getBinder());

    }
}
