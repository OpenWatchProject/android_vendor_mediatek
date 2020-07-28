package com.mediatek.mmsdk;

import android.content.Context;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.util.Log;

import com.mediatek.mmsdk.CameraEffect;
import com.mediatek.mmsdk.CameraEffectHalRuntimeException;
import com.mediatek.mmsdk.IEffectFactory;
import com.mediatek.mmsdk.IFeatureManager;
import com.mediatek.mmsdk.IMMSdkService;

import java.util.ArrayList;
import java.util.List;

public class CameraEffectManager {

    private static final String TAG = "CameraEffectManager";
    private static final String CAMERA_MM_SERVICE_BINDER_NAME = "media.mmsdk";
    private final Context mContext;
    private IMMSdkService mIMmsdkService;
    private IFeatureManager mIFeatureManager;
    private IEffectFactory mIEffectFactory;

    public CameraEffectManager(
            Context context) {
        mContext = context;
    }

    public CameraEffect openEffectHal(EffectHalVersion version,
            CameraEffect.StateCallback callback, Handler handler) throws CameraEffectHalException {
        if (version == null) {
            throw new IllegalArgumentException("effect version is null");
        } else if (handler == null) {
            if (Looper.myLooper() != null) {
                handler = new Handler();
            } else {
                throw new IllegalArgumentException("Looper doesn't exist in the calling thread");
            }
        }

        return openEffect(version, callback, handler);
    }

    public List<EffectHalVersion> getSupportedVersion(String effectName)
            throws CameraEffectHalException {
        List<EffectHalVersion> version = new ArrayList<EffectHalVersion>();
        getEffectFactory();
        try {
            mIEffectFactory.getSupportedVersion(effectName, version);
        } catch (RemoteException e) {
            Log.e(TAG, "RemoteException during getSupportedVersion", e);
        }
        return version;
    }

    private CameraEffect openEffect(EffectHalVersion version, CameraEffect.StateCallback callback,
            Handler handler) throws CameraEffectHalException {

        CameraEffect cameraEffect = null;

        // <Step1> first get the camera MM service
        getMmSdkService();
        // <Step2> get the Feature Manager
        getFeatureManager();

        // <Step3> get the Effect Factory
        getEffectFactory();

        // <Step4> get the effect HAL Client
        IEffectHalClient effectHalClient = createEffectHalClient(version);

        // <Step6> init the EffectHalClient
        int initValue = -1;
        try {
            // now native status: uninit -> init
            initValue = effectHalClient.init();
        } catch (RemoteException e1) {
            Log.e(TAG, "RemoteException during init", e1);

            throw new CameraEffectHalException(CameraEffectHalException.EFFECT_INITIAL_ERROR);
        }
        // <Step5> create the Camera effect
        CameraEffectImpl cameraEffectImpl = new CameraEffectImpl(callback, handler);

        // <Step7> set effect listener
        IEffectListener effectListener = cameraEffectImpl.getEffectHalListener();
        int setListenerValue = -1;
        try {
            setListenerValue = effectHalClient.setEffectListener(effectListener);
        } catch (RemoteException e) {
            Log.e(TAG, "RemoteException during setEffectListener", e);

            CameraEffectHalRuntimeException exception = new CameraEffectHalRuntimeException(
                    CameraEffectHalException.EFFECT_HAL_LISTENER_ERROR);
            cameraEffectImpl.setRemoteCameraEffectFail(exception);

            throw exception.asChecked();
        }

        // <Step8> set remote effect camera
        cameraEffectImpl.setRemoteCameraEffect(effectHalClient);

        cameraEffect = cameraEffectImpl;

        Log.i(TAG, "[openEffect],version = " + version + ",initValue = " + initValue
                + ",setListenerValue = " + setListenerValue + ",cameraEffect = " + cameraEffect);

        return cameraEffect;
    }

    private IMMSdkService getMmSdkService() throws CameraEffectHalException {
        if (mIMmsdkService == null) {
            IBinder mmsdkService = ServiceManager.getService(CAMERA_MM_SERVICE_BINDER_NAME);
            if (mmsdkService == null) {
                throw new CameraEffectHalException(
                        CameraEffectHalException.EFFECT_HAL_SERVICE_ERROR);
            }
            mIMmsdkService = IMMSdkService.Stub.asInterface(mmsdkService);
        }

        return mIMmsdkService;
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

    private IEffectHalClient createEffectHalClient(EffectHalVersion version)
            throws CameraEffectHalException {
        getEffectFactory();
        BinderHolder effectHalClientHolder = new BinderHolder();
        try {
            mIEffectFactory.createEffectHalClient(version, effectHalClientHolder);
        } catch (RemoteException e) {
            throw new CameraEffectHalException(CameraEffectHalException.EFFECT_HAL_CLIENT_ERROR);
        }
        return IEffectHalClient.Stub.asInterface(effectHalClientHolder.getBinder());

    }
}
