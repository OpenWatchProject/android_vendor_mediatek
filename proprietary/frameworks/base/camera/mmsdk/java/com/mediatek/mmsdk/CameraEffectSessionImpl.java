package com.mediatek.mmsdk;

import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import com.android.internal.util.Preconditions;

public class CameraEffectSessionImpl extends CameraEffectSession {
    private static final String TAG = "CameraEffectSessionImpl";

    private CameraEffectImpl mCameraMmEffectImpl;

    private static final boolean VERBOSE = true;

    /** This session is closed; all further calls will throw ISE */
    private boolean mClosed = false;
    // User-specified state handler used for outgoing state callback events
    // this handler is from AP calling
    private final Handler mStateHandler;
    // this callback is from CameraMmEffect's
    // createEffectSession(xxx,xxx,callback),but have been
    // wrapped by proxy by MmSdkCallbackProxies.SessionStateCallbackProxy
    // used for notify AP current state
    private final CameraEffectSession.SessionStateCallback mStateCallback;
    /** Internal handler; used for all incoming events to preserve total order */
    private final Handler mDeviceHandler;

    // Is the session in the process of aborting? Pay attention to BUSY->IDLE
    // transitions.
    private volatile boolean mAborting;

    public CameraEffectSessionImpl(
            CameraEffectSession.SessionStateCallback callback, Handler effectStateHandler,
            CameraEffectImpl effectImpl, Handler deviceStateHandler, boolean configureSuccess) {
        Log.i(TAG, "[CameraEffectHalSessionImpl]++++ configureSuccess = " + configureSuccess);

        if (callback == null) {
            throw new IllegalArgumentException("callback must not be null");
        }

        mStateCallback = callback;
        mStateHandler = checkHandler(effectStateHandler);
        mDeviceHandler = Preconditions.checkNotNull(deviceStateHandler,
                "deviceStateHandler must not be null");
        mCameraMmEffectImpl = Preconditions.checkNotNull(effectImpl, "deviceImpl must not be null");

        if (configureSuccess) {
            mStateHandler.post(mConfiguredRunnable);
        } else {
            mStateHandler.post(mConfiguredFailRunnable);
        }
    }

    @Override
    public void startCapture(CaptureCallback callback, Handler handler) {
        if (VERBOSE) {
            Log.v(TAG, "[" + Thread.currentThread().getStackTrace()[2].getMethodName()
                    + "]++++ callback " + callback + " handler " + handler);
        }

        checkNotClosed();
        handler = checkHandler(handler, callback);
        CameraEffectImpl.CaptureCallback cb = createCaptureCallback(handler, callback);
        mCameraMmEffectImpl.startEffectHal(mDeviceHandler, cb);

        if (VERBOSE) {
            Log.v(TAG, "[" + Thread.currentThread().getStackTrace()[2].getMethodName() + "]----");
        }
    }

    @Override
    public void setFrameParameters(boolean isInput, int index, BaseParameters baseParameters,
            long timestamp, boolean repeating) {
        if (baseParameters == null) {
            throw new IllegalArgumentException("[addInputFrameParameter] parameters is null");
        }
        mCameraMmEffectImpl
                .setFrameParameters(isInput, index, baseParameters, timestamp, repeating);
    }

    @Override
    public int setFrameSyncMode(boolean isInput, int index, boolean sync) {
        int status_t = mCameraMmEffectImpl.setFrameSyncMode(isInput, index, sync);
        if (VERBOSE) {
            Log.i(TAG, "[setInputsyncMode] status_t = " + status_t);
        }
        return status_t;
    }

    @Override
    public boolean getFrameSyncMode(boolean isInputSync, int index) {
        boolean value = mCameraMmEffectImpl.getFrameSyncMode(isInputSync, index);
        if (VERBOSE) {
            Log.i(TAG, "[getInputsyncMode] value = " + value);
        }
        return value;
    }

    @Override
    public void stopCapture(BaseParameters baseParameters) {
        // baseParameters may be null
        if (VERBOSE) {
            Log.v(TAG, "[abort]baseParameters " + baseParameters);
        }
        mCameraMmEffectImpl.abortCapture(baseParameters);
    }

    @Override
    public void closeSession() {
        mCameraMmEffectImpl.abortCapture(null);
    }

    @Override
    public void close() {
        if (mClosed) {
            if (VERBOSE) {
                Log.i(TAG, "[close],current session is closed,so return");
            }
            return;
        }

        if (VERBOSE) {
            Log.i(TAG, "[close] on going");
        }
        mClosed = true;
    }

    // Replace this session with another session.
    // <p>After this call completes, the session will not call any further
    // methods on the camera
    // device.</p>
    void replaceSessionClose() {
        if (VERBOSE) {
            Log.i(TAG, "[replaceSessionClose]");
        }
        close();
    }

    private void checkNotClosed() {
        if (mClosed) {
            throw new IllegalStateException("Session has been closed; further changes are illegal.");
        }
    }

    /**
     * Default handler management, conditional on there being a callback.
     * 
     * <p>
     * If the callback isn't null, check the handler, otherwise pass it through.
     * </p>
     */
    private static <T> Handler checkHandler(Handler handler, T callback) {
        if (callback != null) {
            return checkHandler(handler);
        }
        return handler;
    }

    /**
     * Default handler management.
     * 
     * <p>
     * If handler is null, get the current thread's Looper to create a Handler
     * with. If no looper exists, throw {@code IllegalArgumentException}.
     * </p>
     */
    private static Handler checkHandler(Handler handler) {
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

    // ***************************TODO ***************************
    // Need check this state callback whether need ***************
    // ***********************************************************
    // Create an internal state callback, to be invoked on the mDeviceHandler
    CameraEffectImpl.DeviceStateCallback getDeviceStateCallback() {
        final CameraEffectSession session = this;
        return new CameraEffectImpl.DeviceStateCallback() {
            private boolean mBusy = false;
            private boolean mActive = false;

            @Override
            public void onDisconnected(CameraEffect effect) {
                if (VERBOSE)
                    Log.v(TAG, "[" + Thread.currentThread().getStackTrace()[2].getMethodName()
                            + "]");
                close();
            }

            @Override
            public void onUnconfigured(CameraEffect effect) {
                if (VERBOSE)
                    Log.v(TAG, "[" + Thread.currentThread().getStackTrace()[2].getMethodName()
                            + "]");
            }

            @Override
            public void onActive(CameraEffect effect) {
                mActive = true;

                if (VERBOSE)
                    Log.v(TAG, "[" + Thread.currentThread().getStackTrace()[2].getMethodName()
                            + "]");
            }

            @Override
            public void onBusy(CameraEffect effect) {
                mBusy = true;

                // TODO: Queue captures during abort instead of failing them
                // since the app won't be able to distinguish the two actives
                // Don't signal the application since there's no clean mapping
                // here
                if (VERBOSE)
                    Log.v(TAG, "[" + Thread.currentThread().getStackTrace()[2].getMethodName()
                            + "]");
            }

            @Override
            public void onIdle(CameraEffect effect) {
                boolean isAborting;
                if (VERBOSE)
                    Log.v(TAG, "[" + Thread.currentThread().getStackTrace()[2].getMethodName()
                            + "]");
                synchronized (session) {
                    isAborting = mAborting;
                }

                /*
                 * Check which states we transitioned through:
                 * 
                 * (ACTIVE -> IDLE) (BUSY -> IDLE)
                 * 
                 * Note that this is also legal: (ACTIVE -> BUSY -> IDLE)
                 * 
                 * and mark those tasks as finished
                 */
                if (mBusy && isAborting) {
                    mAborting = false;
                }

                mBusy = false;
                mActive = false;

            }

            @Override
            public void onError(CameraEffect effect, int error) {
                Log.wtf(TAG, "Got device error " + error);
            }

        };
    }

    private CameraEffectImpl.CaptureCallback createCaptureCallback(Handler handler,
            final CaptureCallback callback) {

        CameraEffectImpl.CaptureCallback loCallback = new CameraEffectImpl.CaptureCallback() {

            @Override
            public void onInputFrameProcessed(CameraEffectSession session,
                    BaseParameters parameter, BaseParameters partialResult) {
                if (VERBOSE) {
                    Log.i(TAG, "[" + Thread.currentThread().getStackTrace()[2].getMethodName()
                            + "]" + ",callback = " + callback);
                }
                callback.onInputFrameProcessed(session, parameter, partialResult);
            }

            @Override
            public void onOutputFrameProcessed(CameraEffectSession session,
                    BaseParameters parameter, BaseParameters partialResult) {
                if (VERBOSE) {
                    Log.i(TAG, "[" + Thread.currentThread().getStackTrace()[2].getMethodName()
                            + "]" + ",callback = " + callback);
                }
                callback.onOutputFrameProcessed(session, parameter, partialResult);
            }

            @Override
            public void onCaptureSequenceCompleted(CameraEffectSession session,
                    BaseParameters result, long uid) {
                if (VERBOSE) {
                    Log.i(TAG, "[" + Thread.currentThread().getStackTrace()[2].getMethodName()
                            + "]" + ",callback = " + callback);
                }
                callback.onCaptureSequenceCompleted(session, result, uid);
            }

            @Override
            public void onCaptureSequenceAborted(CameraEffectSession session, BaseParameters result) {
                if (VERBOSE) {
                    Log.i(TAG, "[" + Thread.currentThread().getStackTrace()[2].getMethodName()
                            + "]" + ",callback = " + callback);
                }
                callback.onCaptureSequenceAborted(session, result);
            }

            @Override
            public void onCaptureFailed(CameraEffectSession session, BaseParameters result) {
                if (VERBOSE) {
                    Log.i(TAG, "[" + Thread.currentThread().getStackTrace()[2].getMethodName()
                            + "]" + ",callback = " + callback);
                }
                callback.onCaptureFailed(session, result);
            }

        };

        return loCallback;
    }
    
    private final Runnable mConfiguredRunnable = new Runnable() {
        
        @Override
        public void run() {
            Log.v(TAG, "[mConfiguredRunnable] Created session successfully");
            mStateCallback.onConfigured(CameraEffectSessionImpl.this);
        }
    };
    
    private final Runnable mConfiguredFailRunnable = new Runnable() {
        
        @Override
        public void run() {
            Log.e(TAG, "[mConfiguredFailRunnable]Failed to create capture session: configuration failed");
            mStateCallback.onConfigureFailed(CameraEffectSessionImpl.this);
        }
    };
}
