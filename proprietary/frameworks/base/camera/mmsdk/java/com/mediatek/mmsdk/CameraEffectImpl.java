package com.mediatek.mmsdk;

import android.os.Handler;
import android.os.Looper;
import android.os.RemoteException;
import android.util.Log;
import android.util.SparseArray;
import android.view.Surface;

import com.mediatek.mmsdk.CameraEffectStatus.CameraEffectHalStatus;

import java.util.ArrayList;
import java.util.List;

public class CameraEffectImpl extends CameraEffect {

    private static final String TAG = "CameraEffectImpl";
    private static final boolean DEBUG = true;
    private static final int SUCCESS_VALUE = 0;

    private boolean mInError = false;

    private CameraEffectSessionImpl mCurrentSession;
    private CameraEffectStatus mEffectHalStatus;
    private final Object mInterfaceLock = new Object();

    // this callback is used to notify AP about current camera effect HAL Client
    // status
    private StateCallback mEffectStateCallback;

    // according to current Effect HAL state,notify EffectHalSession to change
    // session's state
    private DeviceStateCallback mSessionStateCallback;

    // this handler is from AP call openCameraMmEffect(xxx,xxx,handler)'s
    // parameters
    private Handler mEffectHalHandler;
    private IEffectHalClient mIEffectHalClient;
    private BaseParameters mBaseParameters;

    public CameraEffectImpl(
            StateCallback callback, Handler handler) {
        mEffectStateCallback = callback;
        mEffectHalHandler = handler;
        mEffectHalStatus = new CameraEffectStatus();
    }

    @Override
    public CameraEffectSession createCaptureSession(List<Surface> outputs,
            List<BaseParameters> surfaceParameters,
            CameraEffectSession.SessionStateCallback callback, Handler handler)
            throws CameraEffectHalException {

        if (DEBUG) {
            Log.i(TAG, "[" + Thread.currentThread().getStackTrace()[2].getMethodName() + "]");
        }
        checkIfCameraClosedOrInError();

        // check surface
        if (outputs == null) {
            throw new IllegalArgumentException(
                    "createEffectSession: the outputSurface must not be null");
        }

        // check handler
        handler = checkHandler(handler);

        // Notify current session that it's going away, before starting camera
        // operations After this call completes, the session is not allowed to
        // call into CameraDeviceImpl
        if (mCurrentSession != null) {
            mCurrentSession.replaceSessionClose();
        }

        boolean configureSuccess = false;
        CameraEffectHalException pendingException = null;
        try {
            configureSuccess = configureOutputs(outputs, surfaceParameters);
        } catch (CameraEffectHalException e) {
            configureSuccess = false;
            pendingException = e;
            if (DEBUG) {
                Log.v(TAG, "createCaptureSession- failed with exception ", e);
            }
        }

        CameraEffectSessionImpl newSessionImpl = new CameraEffectSessionImpl(callback, handler,
                this, mEffectHalHandler, configureSuccess);

        mCurrentSession = newSessionImpl;

        if (pendingException != null) {
            throw pendingException;
        }

        mSessionStateCallback = mCurrentSession.getDeviceStateCallback();
        mEffectHalHandler.post(mCallOnIdle);
        return mCurrentSession;
    }

    @Override
    public void setParamters(BaseParameters baseParameters) {
        
        try {
            mIEffectHalClient.setParameters(baseParameters);
        } catch (RemoteException e) {
            Log.e(TAG, "RemoteException during setParameters [BaseParameters]",e);
        }
    }

    @Override
    public List<Surface> getInputSurface() {
        Log.d(TAG, "[getInputSurface],current status = " + mEffectHalStatus.getEffectHalStatus());

        List<Surface> surface = new ArrayList<>();
        try {
            mIEffectHalClient.configure();
            mIEffectHalClient.prepare();
            mIEffectHalClient.getInputSurfaces(surface);
            mEffectHalStatus.setEffectHalStatus(CameraEffectHalStatus.STATUS_CONFINGURED);
        } catch (RemoteException e) {
            Log.e(TAG, "RemoteException during configure or prepare or getInputSurfaces",e);
        }

        return surface;
    }

    @Override
    public List<BaseParameters> getCaputreRequirement(BaseParameters parameters) {
        int getRequirementValue = -1;
        List<BaseParameters> requireParameters = new ArrayList<BaseParameters>();
        CameraEffectHalStatus currentStatus = mEffectHalStatus.getEffectHalStatus();

        Log.i(TAG, "[getCaputreRequirement] currentStatus = " + currentStatus);
        try {
            if (CameraEffectHalStatus.STATUS_CONFINGURED != currentStatus) {
                // current must be init status,not can be running status
                mIEffectHalClient.configure();
                mEffectHalStatus.setEffectHalStatus(CameraEffectHalStatus.STATUS_CONFINGURED);
            }
            getRequirementValue = mIEffectHalClient.getCaptureRequirement(parameters,
                    requireParameters);
        } catch (RemoteException e) {
            Log.e(TAG, "RemoteException during getCaptureRequirement",e);
        }

        if (DEBUG) {
            Log.i(TAG, "[getCaputreRequirement] return value from native : " + getRequirementValue
                    + ",parameters = " + requireParameters.toString());
        }

        return requireParameters;
    }

    @Override
    public void closeEffect() {
        
        Log.i(TAG, "[closeEffect] +++,mIEffectHalClient = " + mIEffectHalClient);
        abortCapture(null);
        unConfigureEffectHal();
        unInitEffectHal();
        Log.i(TAG, "[closeEffect] ---");
    }

    public EffectHalClientListener getEffectHalListener() {
        return new EffectHalClientListener();
    }

    public void setRemoteCameraEffect(IEffectHalClient client) {
        synchronized (mInterfaceLock) {
            if (mInError) {
                return;
            }
            mIEffectHalClient = client;
            mEffectHalStatus.setEffectHalStatus(CameraEffectHalStatus.STATUS_INITIALIZED);
        }
    }

    // Call to indicate failed connection to a remote camera effect service
    // will use the android errors.h tag [Need Check]
    public void setRemoteCameraEffectFail(final CameraEffectHalRuntimeException exception) {
        int failureCode = DeviceStateCallback.ERROR_EFFECT_DEVICE;
        boolean failureIsError = true;
        switch (exception.getReason()) {
        case CameraEffectHalException.EFFECT_HAL_IN_USE:
            failureCode = DeviceStateCallback.ERROR_EFFECT_HAL_IN_USE;
            break;

        case CameraEffectHalException.EFFECT_HAL_SERVICE_ERROR:
            failureCode = DeviceStateCallback.ERROR_EFFECT_DISABLED;
            break;

        case CameraEffectHalException.EFFECT_HAL_FEATUREMANAGER_ERROR:
            failureIsError = false;
            break;

        case CameraEffectHalException.EFFECT_HAL_FACTORY_ERROR:
            failureCode = DeviceStateCallback.ERROR_EFFECT_DEVICE;
            break;

        case CameraEffectHalException.EFFECT_HAL_LISTENER_ERROR:
            failureCode = DeviceStateCallback.ERROR_EFFECT_LISTENER;
            break;

        default:
            Log.wtf(TAG, "Unknown failure in opening camera device: " + exception.getReason());
            break;
        }

        final int code = failureCode;
        final boolean isError = failureIsError;
        synchronized (mInterfaceLock) {
            mInError = true;
            mEffectHalStatus.setEffectHalStatus(CameraEffectHalStatus.STATUS_INITIALIZED);
            mEffectHalHandler.post(new Runnable() {

                @Override
                public void run() {
                    if (isError) {
                        mEffectStateCallback.onError(CameraEffectImpl.this, code);
                    } else {
                        mEffectStateCallback.onDisconnected(CameraEffectImpl.this);
                    }
                }
            });
        }

    }

    public boolean configureOutputs(List<Surface> outputs, List<BaseParameters> surfaceParameters)
            throws CameraEffectHalException {

        if (DEBUG) {
            Log.i(TAG, "[" + Thread.currentThread().getStackTrace()[2].getMethodName()
                    + "]++++,current status = " + mEffectHalStatus.getEffectHalStatus());
        }
        // Treat a null input the same an empty list
        if (outputs == null) {
            outputs = new ArrayList<Surface>();
        }
        boolean success = false;
        synchronized (mInterfaceLock) {
            checkIfCameraClosedOrInError();

            mEffectHalHandler.post(mCallOnBusy);

            // set outputSurface,now EffectHal have established a link with
            // frameWrok
            // the output from EffectHal will be put into outputs
            try {
                // Notice:the outputs maybe null
                // setOutputSurface must be at the init status
                success = SUCCESS_VALUE == mIEffectHalClient.setOutputSurfaces(outputs,
                        surfaceParameters);
            } catch (RemoteException e) {
                Log.e(TAG, "RemoteException during setOutputSurfaces",e);
            }
        }
        Log.i(TAG, "[configureOutputs]----, success = " + success);

        return success;
    }

    public void startEffectHal(Handler handler, CaptureCallback callback) {

        if (DEBUG) {
            Log.i(TAG, "[" + Thread.currentThread().getStackTrace()[2].getMethodName()
                    + "]++++,status = " + mEffectHalStatus.getEffectHalStatus());
        }
        // Need a valid handler, or current thread needs to have a looper, if
        // callback is valid
        handler = checkHandler(handler, callback);
        try {

            if (CameraEffectHalStatus.STATUS_CONFINGURED != mEffectHalStatus.getEffectHalStatus()) {
                mIEffectHalClient.configure();
                mEffectHalStatus.setEffectHalStatus(CameraEffectHalStatus.STATUS_CONFINGURED);
            }
            mIEffectHalClient.prepare();
            mCurrentStartId = mIEffectHalClient.start();
            mEffectHalStatus.setEffectHalStatus(CameraEffectHalStatus.STATUS_RUNNING);
        } catch (RemoteException e) {
            Log.e(TAG, "RemoteException during prepare or start",e);
        }
        mCaptureCallbackHolderMap.put((int) mCurrentStartId, new CaptureCallbackHolder(callback,
                handler));
        mEffectHalHandler.post(mCallOnActive);

        if (DEBUG) {
            Log.i(TAG,
                    "[" + Thread.currentThread().getStackTrace()[2].getMethodName()
                            + "]----, mCurrentStartId = " + mCurrentStartId + ",callback = "
                            + callback + ",get the map's callback = "
                            + mCaptureCallbackHolderMap.get((int) mCurrentStartId));
        }

    }

    public void setFrameParameters(boolean isInput, int index, BaseParameters baseParameters,
            long timestamp, boolean repeating) {
        try {
            if (isInput) {
                mIEffectHalClient.addInputParameter(index, baseParameters, timestamp, repeating);
            } else {
                mIEffectHalClient.addOutputParameter(index, baseParameters, timestamp, repeating);
            }
        } catch (RemoteException e) {
            Log.e(TAG, "RemoteException during addInputParameter or addOutputParameter",e);
        }
    }

    public void addOutputParameter(int index, BaseParameters parameter, long timestamp,
            boolean repeat) {
        try {
            mIEffectHalClient.addOutputParameter(index, parameter, timestamp, repeat);
        } catch (RemoteException e) {
            Log.e(TAG, "RemoteException during addOutputParameter",e);
        }
    }

    public void abortCapture(BaseParameters baseParameters) {
        try {
            if (CameraEffectHalStatus.STATUS_RUNNING == mEffectHalStatus.getEffectHalStatus()) {
                mIEffectHalClient.abort(mBaseParameters);
                mEffectHalStatus.setEffectHalStatus(CameraEffectHalStatus.STATUS_CONFINGURED);
            }
        } catch (RemoteException e) {
            Log.e(TAG, "RemoteException during abort", e);
        }
        mBaseParameters = baseParameters;
    }

    // this same as the CameraDeviceImpl's camera device callback
    public class EffectHalClientListener extends IEffectListener.Stub {

        // when call effect HAL client prepare() interface, effect HAL will
        // notify AP when prepare done.
        @Override
        public void onPrepared(IEffectHalClient effect, BaseParameters result)
                throws RemoteException {
            if (DEBUG) {
                Log.i(TAG, "[onPrepared] effect = " + effect + ",result = " + result.flatten());
            }
        }

        // when the buffer is dispatched to the Effect HAL,this is
        // notified,means current inputFrame is in processing
        @Override
        public void onInputFrameProcessed(IEffectHalClient effect, BaseParameters parameter,
                BaseParameters partialResult) throws RemoteException {

            Log.i(TAG, "[" + Thread.currentThread().getStackTrace()[2].getMethodName() + "]++++");
            final CaptureCallbackHolder callbackHolder;
            final BaseParameters parameters = partialResult;
            final BaseParameters result = partialResult;
            callbackHolder = (mCurrentStartId > 0) ? mCaptureCallbackHolderMap
                    .valueAt((int) mCurrentStartId) : null;
            if (DEBUG && parameter != null && partialResult != null) {
                Log.i(TAG, "[onInputFrameProcessed] effect = " + effect + ",parameter = "
                        + parameter.flatten() + ",partialResult = " + partialResult.flatten()
                        + ",callbackHolder = " + callbackHolder);
            }

            if (callbackHolder != null) {
                callbackHolder.getHandler().post(new Runnable() {

                    @Override
                    public void run() {
                        callbackHolder.getCaptureCallback().onInputFrameProcessed(mCurrentSession,
                                parameters, result);
                    }
                });
            }
            Log.i(TAG, "[" + Thread.currentThread().getStackTrace()[2].getMethodName() + "]----");

        }

        // when the frame is success processed by effect HAL,will notify this
        @Override
        public void onOutputFrameProcessed(IEffectHalClient effect, BaseParameters parameter,
                BaseParameters partialResult) throws RemoteException {
            Log.i(TAG, "[" + Thread.currentThread().getStackTrace()[2] + "]++++");

            final CaptureCallbackHolder callbackHolder;
            final BaseParameters parameters = partialResult;
            final BaseParameters result = partialResult;
            callbackHolder = (mCurrentStartId > 0) ? mCaptureCallbackHolderMap
                    .get((int) mCurrentStartId) : null;

            if (DEBUG && parameter != null && partialResult != null) {
                Log.i(TAG, "[onOutputFrameProcessed]++++, effect = " + effect + ",parameter = "
                        + parameter.flatten() + ",partialResult = " + partialResult.flatten()
                        + ",mCurrentStartId = " + mCurrentStartId + ",callbackHolder = "
                        + callbackHolder);
            }

            if (callbackHolder != null) {
                callbackHolder.getHandler().post(new Runnable() {

                    @Override
                    public void run() {
                        callbackHolder.getCaptureCallback().onOutputFrameProcessed(mCurrentSession,
                                parameters, result);
                    }
                });
            }

            Log.i(TAG, "[" + Thread.currentThread().getStackTrace()[2] + "]----");
        }

        // when this session is processed done by the effect HAL,will notify
        @Override
        public void onCompleted(IEffectHalClient effect, BaseParameters partialResult, long uid)
                throws RemoteException {

            Log.i(TAG, "[" + Thread.currentThread().getStackTrace()[2].getMethodName() + "]++++");
            final int compleateId = (int) uid;
            final CaptureCallbackHolder callbackHolder;
            final BaseParameters parameters = partialResult;
            callbackHolder = (compleateId > 0) ? mCaptureCallbackHolderMap.get((int) compleateId)
                    : null;
            if (DEBUG && partialResult != null) {
                Log.i(TAG,
                        "[onCompleted]++++, effect = " + ",partialResult = "
                                + partialResult.flatten() + ",uid = " + uid + ",compleateId = "
                                + compleateId + ",mCurrentStartId = " + mCurrentStartId
                                + ",callbackHolder = " + callbackHolder);
            }

            if (callbackHolder != null) {
                callbackHolder.getHandler().post(new Runnable() {
                    @Override
                    public void run() {
                        callbackHolder.getCaptureCallback().onCaptureSequenceCompleted(
                                mCurrentSession, parameters, compleateId);
                    }
                });// end of runnable
            }
            mIEffectHalClient.abort(null);
            mEffectHalStatus.setEffectHalStatus(CameraEffectHalStatus.STATUS_CONFINGURED);

            Log.i(TAG, "[" + Thread.currentThread().getStackTrace()[2].getMethodName() + "]----");
        }

        // if abort this capture,will be notified when Effect HAL have aborted
        @Override
        public void onAborted(IEffectHalClient effect, BaseParameters result)
                throws RemoteException {
            Log.i(TAG, "[" + Thread.currentThread().getStackTrace()[2] + "]++++");

            final CaptureCallbackHolder callbackHolder;
            final BaseParameters baseParametersResult = result;
            callbackHolder = (mCurrentStartId > 0) ? mCaptureCallbackHolderMap
                    .get((int) mCurrentStartId) : null;

            if (DEBUG && result != null) {
                Log.i(TAG, "[" + Thread.currentThread().getStackTrace()[2].getMethodName()
                        + "] ++++,effect = " + effect + ",result = " + result.flatten());
            }

            if (callbackHolder != null) {
                callbackHolder.getHandler().post(new Runnable() {

                    @Override
                    public void run() {
                        callbackHolder.getCaptureCallback().onCaptureSequenceAborted(
                                mCurrentSession, baseParametersResult);
                    }
                });
            }
            mCaptureCallbackHolderMap.removeAt((int) mCurrentStartId);
            
            Log.i(TAG, "[" + Thread.currentThread().getStackTrace()[2].getMethodName() + "] ----");
        }

        // Effect HAL is failed in this session
        @Override
        public void onFailed(IEffectHalClient effect, BaseParameters result) throws RemoteException {
            Log.i(TAG, "[" + Thread.currentThread().getStackTrace()[2] + "]++++");

            final CaptureCallbackHolder callbackHolder;
            final BaseParameters baseParametersResult = result;
            callbackHolder = (mCurrentStartId > 0) ? mCaptureCallbackHolderMap
                    .get((int) mCurrentStartId) : null;

            if (DEBUG && result != null) {
                Log.i(TAG, "[" + Thread.currentThread().getStackTrace()[2].getMethodName()
                        + "] ++++,effect = " + effect + ",result = " + result.flatten());
            }

            if (callbackHolder != null) {
                callbackHolder.getHandler().post(new Runnable() {

                    @Override
                    public void run() {
                        callbackHolder.getCaptureCallback().onCaptureFailed(mCurrentSession,
                                baseParametersResult);
                    }
                });
            }
            mCaptureCallbackHolderMap.removeAt((int) mCurrentStartId);
            Log.i(TAG, "[" + Thread.currentThread().getStackTrace()[2].getMethodName() + "] ----");
        }

    }

    @Override
    public void close() {
        Log.i(TAG, "[close]");
        if (mIEffectHalClient != null || mInError) {
            mEffectHalHandler.post(mCallOnClosed);
        }
        mIEffectHalClient = null;
        mInError = false;
    }

    public int setFrameSyncMode(boolean isInput, int index, boolean sync) {
        int status_t = -1;
        try {
            if (isInput) {
                status_t = mIEffectHalClient.setInputsyncMode(index, sync);
            } else {
                status_t = mIEffectHalClient.setOutputsyncMode(index, sync);
            }
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        if (DEBUG) {
            Log.i(TAG, "[setFrameSyncMode] status_t = " + status_t + ",isInput = " + isInput);
        }
        return status_t;
    }

    public int setOutputsyncMode(int index, boolean sync) {
        int status_t = -1;
        try {
            status_t = mIEffectHalClient.setOutputsyncMode(index, sync);
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        if (DEBUG) {
            Log.i(TAG, "[setOutputsyncMode] status_t = " + status_t);
        }
        return status_t;
    }

    public boolean getFrameSyncMode(boolean isInput, int index) {
        boolean value = false;
        try {
            value = mIEffectHalClient.getInputsyncMode(index);
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        if (DEBUG) {
            Log.i(TAG, "[getInputsyncMode] value = " + value);
        }
        return value;
    }

    public boolean getOutputsyncMode(int index) {
        boolean value = false;
        try {
            value = mIEffectHalClient.getOutputsyncMode(index);
        } catch (RemoteException e) {
            e.printStackTrace();
        }

        if (DEBUG) {
            Log.i(TAG, "[getOutputsyncMode] value = " + value);
        }
        return value;
    }

    // *************************************************************************
    // ***********************************Begin CaptureCallback*****************
    // *************************************************************************

    public static abstract class CaptureCallback {

        public void onInputFrameProcessed(CameraEffectSession session, BaseParameters parameter,
                BaseParameters partialResult) {
            // default do noting
        }

        public void onOutputFrameProcessed(CameraEffectSession session, BaseParameters parameter,
                BaseParameters partialResult) {
            // default do noting
        }

        public void onCaptureSequenceCompleted(CameraEffectSession session, BaseParameters result,
                long uid) {
            // default do noting
        }

        public void onCaptureSequenceAborted(CameraEffectSession session, BaseParameters result) {
            // default do noting
        }

        public void onCaptureFailed(CameraEffectSession session, BaseParameters result) {
            // default do noting
        }
    }

    // **************************************************************************
    // *******************Begin DeviceStateCallback***************************
    // **************************************************************************
    public static abstract class DeviceStateCallback extends StateCallback {

        /**
         * The method called when a camera device has no outputs configured.
         * 
         */
        public void onUnconfigured(CameraEffect effect) {
            // Default empty implementation
        }

        public void onActive(CameraEffect effect) {
            // Default empty implementation
        }

        /**
         * The method called when a camera device is busy.
         * 
         */
        public void onBusy(CameraEffect effect) {
            // Default empty implementation
        }

        /**
         * The method called when a camera device has finished processing all
         * submitted capture requests and has reached an idle state.
         * 
         */
        public void onIdle(CameraEffect effect) {
            // Default empty implementation
        }

    }

    // *******************************************************************
    // *******************************************************************
    // Runnable for all state transitions, except error, which needs the
    // error code argument

    private final Runnable mCallOnActive = new Runnable() {

        @Override
        public void run() {
            DeviceStateCallback stateCallback2 = null;
            synchronized (mInterfaceLock) {
                if (mIEffectHalClient == null) {
                    return;
                }
                stateCallback2 = mSessionStateCallback;
            }
            if (stateCallback2 != null) {
                stateCallback2.onActive(CameraEffectImpl.this);
            }
        }
    };

    private final Runnable mCallOnBusy = new Runnable() {

        @Override
        public void run() {

            DeviceStateCallback sessionCallback = null;
            synchronized (mInterfaceLock) {
                if (mIEffectHalClient == null)
                    return; // Camera already closed

                sessionCallback = mSessionStateCallback;
            }
            if (sessionCallback != null) {
                sessionCallback.onBusy(CameraEffectImpl.this);
            }
        }
    };

    private final Runnable mCallOnClosed = new Runnable() {
        private boolean isClosedOnce = false;

        @Override
        public void run() {
            if (isClosedOnce) {
                throw new AssertionError("Don't post #onClosed more than once");
            }
            DeviceStateCallback sessionCallback = null;
            synchronized (mInterfaceLock) {
                sessionCallback = mSessionStateCallback;
            }
            if (sessionCallback != null) {
                sessionCallback.onClosed(CameraEffectImpl.this);
            }
            mEffectStateCallback.onClosed(CameraEffectImpl.this);
            isClosedOnce = true;
        }
    };

    private final Runnable mCallOnIdle = new Runnable() {

        @Override
        public void run() {
            DeviceStateCallback sessionCallback = null;
            synchronized (mInterfaceLock) {
                if (mIEffectHalClient == null)
                    return;

                sessionCallback = mSessionStateCallback;
            }
            if (sessionCallback != null) {
                sessionCallback.onIdle(CameraEffectImpl.this);
            }
        }
    };

    private void checkIfCameraClosedOrInError() throws CameraEffectHalException {
        if (mInError) {
            throw new CameraEffectHalRuntimeException(
                    CameraEffectHalException.EFFECT_HAL_FACTORY_ERROR,
                    "The camera device has encountered a serious error");
        }
        if (mIEffectHalClient == null) {
            throw new IllegalStateException("effect hal client have closed");
        }
    }

    private void unConfigureEffectHal() {
        Log.i(TAG, "[unConfigureEffectHal]");
        if (CameraEffectHalStatus.STATUS_CONFINGURED == mEffectHalStatus.getEffectHalStatus()) {
            try {
                mIEffectHalClient.unconfigure();
                mEffectHalStatus.setEffectHalStatus(CameraEffectHalStatus.STATUS_INITIALIZED);
            } catch (RemoteException e) {
                Log.e(TAG, "RemoteException during unconfigure",e);

            }
        }
    }
    
    private void unInitEffectHal() {
        Log.i(TAG, "[unInitEffectHal]");
        if (CameraEffectHalStatus.STATUS_INITIALIZED == mEffectHalStatus.getEffectHalStatus()) {
            try {
                mIEffectHalClient.uninit();
                mEffectHalStatus.setEffectHalStatus(CameraEffectHalStatus.STATUS_UNINITIALIZED);
            } catch (RemoteException e) {
                Log.e(TAG, "RemoteException during uninit",e);

            }
        }
    }
    
    /**
     * this class used for store the capture callback,because user can change
     * the capture callback each capture time
     * 
     */
    private SparseArray<CaptureCallbackHolder> mCaptureCallbackHolderMap = new SparseArray<CaptureCallbackHolder>();
    private long mCurrentStartId = -1;

    private class CaptureCallbackHolder {

        private final CaptureCallback mCaptureCallback;
        private final Handler mHandler;

        CaptureCallbackHolder(
                CaptureCallback callback, Handler handler) {
            mCaptureCallback = callback;
            mHandler = handler;
        }

        public CaptureCallback getCaptureCallback() {
            return mCaptureCallback;
        }

        public Handler getHandler() {
            return mHandler;
        }

    }

    /**
     * Default handler management.
     * 
     * <p>
     * If handler is null, get the current thread's Looper to create a Handler
     * with. If no looper exists, throw {@code IllegalArgumentException}.
     * </p>
     */
    private Handler checkHandler(Handler handler) {
        if (handler == null) {
            Looper looper = Looper.myLooper();
            if (looper == null) {
                throw new IllegalArgumentException(
                        "No handler given, and current thread has no looper!");
            }
            handler = new Handler(looper);
        }
        return handler;
    }

    /**
     * Default handler management, conditional on there being a callback.
     * 
     * <p>
     * If the callback isn't null, check the handler, otherwise pass it through.
     * </p>
     */
    private <T> Handler checkHandler(Handler handler, T callback) {
        if (callback != null) {
            return checkHandler(handler);
        }
        return handler;
    }

}
