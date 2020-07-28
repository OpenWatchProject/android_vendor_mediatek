package com.mediatek.camera.tests.common.device;

import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCaptureSession.CaptureCallback;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CaptureRequest;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.view.Surface;

import com.mediatek.camera.common.device.CameraDeviceManagerFactory.CameraApi;
import com.mediatek.camera.common.device.v2.Camera2CaptureSessionProxy;
import com.mediatek.camera.common.device.v2.Camera2Proxy;
import com.mediatek.camera.tests.CameraUnitTestCaseBase;
import com.mediatek.camera.tests.Log;
import com.mediatek.camera.tests.Utils;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.timeout;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;

/**
 * Test camera2 capture session proxy.
 */
public class Camera2CaptureSessionProxyTestCase extends CameraUnitTestCaseBase {
    private static final String TAG = Camera2CaptureSessionProxyTestCase.class.getSimpleName();

    private static final String BACK_CAMERA_ID = "0";

    private Camera2CaptureSessionProxy mCaptureSessionProxy;
    private Camera2Proxy mCamera2Proxy = null;
    private Handler mRespondHandler;
    protected HandlerThread mRequestThread;
    private CameraCaptureSession.StateCallback mSessionStateCallback;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();

    }

    /**
     * test get device.
     */
    public void testGetDevice() {
        CameraCaptureSession session = mock(CameraCaptureSession.class);
        createCaptureSession(session);

        Camera2Proxy proxy = mCaptureSessionProxy.getDevice();
        assertTrue("get wrong device!!", proxy == mCamera2Proxy);
    }

    /**
     * test prepare.
     */
    public void testPrepare() {
        if (!isMNewerSdkVersion()) {
            Log.d(TAG, "<testPrepare> current sdk version too old!");
            return;
        }
        CameraCaptureSession session = mock(CameraCaptureSession.class);
        createCaptureSession(session);

        Surface surface = mock(Surface.class);
        try {
            mCaptureSessionProxy.prepare(surface);
        } catch (CameraAccessException e) {
            // TODO
        }
        try {
            verify(session, times(1)).prepare(surface);
        } catch (CameraAccessException e) {
            Log.d(TAG, "<testPrepare>verify session prepare exception:" + e);
        }
    }

    /**
     * test capture.
     */
    public void testCapture() {
        // TODO, can't create CaptureRequest object
        CameraCaptureSession session = mock(CameraCaptureSession.class);
        createCaptureSession(session);

        CaptureCallback listener = new CaptureCallback() {
        };

        int captureNum = 0;
        try {
            captureNum = mCaptureSessionProxy.capture(null, listener, null);
        } catch (CameraAccessException e) {
            // TODO
        }
        assertTrue("wrong parameter, don't capture!", captureNum == 0);
    }

    /**
     * test capture burst.
     */
    public void testCaptureBurst() {
        CameraCaptureSession session = mock(CameraCaptureSession.class);
        createCaptureSession(session);

        List<CaptureRequest> requests = new ArrayList<CaptureRequest>();
        CaptureCallback listener = new CaptureCallback() {
        };

        int testResult = 1;
        try {
            when(session.captureBurst(requests, listener, null)).thenReturn(testResult);
        } catch (CameraAccessException e) {
            Log.d(TAG, "<testCaptureBurst>when session captureBurst exception:" + e);
        }

        int res = -1;
        try {
            res = mCaptureSessionProxy.captureBurst(requests, listener, null);
        } catch (CameraAccessException e) {
            // TODO
        }

        try {
            verify(session, times(1)).captureBurst(requests, listener, null);
        } catch (CameraAccessException e) {
            Log.d(TAG, "<testCaptureBurst>verify session captureBurst exception:" + e);
        }
        assertTrue("get wrong result!!", res == testResult);
    }

    /**
     * test set repeating request.
     */
    public void testSetRepeatingRequest() {
        // TODO, can't create CaptureRequest object.
        CameraCaptureSession session = mock(CameraCaptureSession.class);
        createCaptureSession(session);

        CaptureCallback listener = new CaptureCallback() {
        };

        boolean hasThrowNullPointerException = false;

        int repeatNum = 0;
        try {
            repeatNum = mCaptureSessionProxy.setRepeatingRequest(null, listener, null);
        } catch (CameraAccessException e) {
            // TODO
        }
        assertTrue("wrong parameter, don't capture", repeatNum == 0);
    }

    /**
     * Test set repeating burst.
     */
    public void testSetRepeatingBurst() {
        CameraCaptureSession session = mock(CameraCaptureSession.class);
        createCaptureSession(session);

        List<CaptureRequest> requests = new ArrayList<CaptureRequest>();
        CaptureCallback listener = new CaptureCallback() {
        };

        int testResult = 1;
        try {
            when(session.setRepeatingBurst(requests, listener, null)).thenReturn(testResult);
        } catch (CameraAccessException e) {
            Log.d(TAG, "<testSetRepeatingBurst> when session set repeating burst"
                    + "throw exception" + e);
        }
        int res = -1;
        try {
            res = mCaptureSessionProxy.setRepeatingBurst(requests, listener, null);
        } catch (CameraAccessException e) {
            // TODO
        }
        try {
            verify(session, times(1)).setRepeatingBurst(requests, listener, null);
        } catch (CameraAccessException e) {
            Log.d(TAG, "<testSetRepeatingBurst> verify session set repeating burst"
                    + "throw exception" + e);
        }
        assertTrue("set repeating burst return wrong result!", res == testResult);
    }

    /**
     * Test stop repeating.
     */
    public void testStopRepeating() {
        CameraCaptureSession session = mock(CameraCaptureSession.class);
        createCaptureSession(session);

        try {
            mCaptureSessionProxy.stopRepeating();
        } catch (CameraAccessException e) {
            // TODO
        }

        try {
            verify(session, times(1)).stopRepeating();
        } catch (CameraAccessException e) {
            Log.d(TAG, "<testStopRepeating> verify session stop repeating exception:" + e);
        }
    }

    /**
     * Test Abort captures.
     */
    public void testAbortCaptures() {
        CameraCaptureSession session = mock(CameraCaptureSession.class);
        createCaptureSession(session);

        try {
            mCaptureSessionProxy.abortCaptures();
        } catch (CameraAccessException e) {
            // TODO
        }

        try {
            verify(session, times(1)).abortCaptures();
        } catch (CameraAccessException e) {
            Log.d(TAG, "<testAbortCaptures>verify session abort capture exception:" + e);
        }
    }

    /**
     * Test is reprcessable.
     */
    public void testIsReprocessable() {
        if (!isMNewerSdkVersion()) {
            Log.d(TAG, "<testIsReprocessable> current sdk version too old!");
            return;
        }
        CameraCaptureSession session = mock(CameraCaptureSession.class);
        createCaptureSession(session);
        when(session.isReprocessable()).thenReturn(true);

        boolean res = false;
        res = mCaptureSessionProxy.isReprocessable();
        verify(session, times(1)).isReprocessable();
        assertTrue("<testIsReprocessable> return wrong result", res);
    }

    /**
     * Test get input surface.
     */
    public void testGetInputSurface() {
        if (!isMNewerSdkVersion()) {
            Log.d(TAG, "<testGetInputSurface> current sdk version too old!");
            return;
        }
        CameraCaptureSession session = mock(CameraCaptureSession.class);
        createCaptureSession(session);

        Surface surface = mock(Surface.class);
        when(session.getInputSurface()).thenReturn(surface);

        Surface res = null;
        res = mCaptureSessionProxy.getInputSurface();
        verify(session, times(1)).getInputSurface();
        assertTrue("<testGetInputSurface> return wrong result", res == surface);
    }

    /**
     * Test close.
     */
    public void testClose() {
        CameraCaptureSession session = mock(CameraCaptureSession.class);
        createCaptureSession(session);

        mCaptureSessionProxy.close();
        verify(session, times(1)).close();
    }

    private void createCaptureSession(CameraCaptureSession session) {
        CameraDevice device = mock(CameraDevice.class);
        createCamera2CaptureSessionProxy(device);
        Camera2CaptureSessionProxy.StateCallback callback =
                new Camera2CaptureSessionProxy.StateCallback() {
            @Override
            public void onConfigured(Camera2CaptureSessionProxy session) {
            }

            @Override
            public void onConfigureFailed(Camera2CaptureSessionProxy session) {
            }
        };
        Handler handler = mock(Handler.class);
        List<Surface> outputs = mock(ArrayList.class);
        try {
            mCamera2Proxy.createCaptureSession(outputs, callback, handler);
        } catch (CameraAccessException e) {
            Log.d(TAG, "camera proxy createCaptureSession:" + e);
        }
        try {
            verify(device, timeout(Utils.TIME_OUT_MS).times(1)).createCaptureSession(outputs,
                    mSessionStateCallback, mRespondHandler);
        } catch (CameraAccessException e) {
            Log.d(TAG, "mock device createCaptureSession:" + e);
        }
        mSessionStateCallback.onConfigured(session);
    }

    private void createCamera2CaptureSessionProxy(CameraDevice device) {

        mRequestThread = new HandlerThread(CameraApi.API2 + "-Request-" + BACK_CAMERA_ID);
        mRequestThread.start();
        mRespondHandler = new Handler(mRequestThread.getLooper());
        try {
            Class handlerClass = Class
                    .forName("com.mediatek.camera.common.device.v2.Camera2Handler");
            Class[] innerClasses = handlerClass.getDeclaredClasses();
            Class deviceInfoListenerClass = null;
            for (Class clazz : innerClasses) {
                if (clazz.getSimpleName().equals("IDeviceInfoListener")) {
                    deviceInfoListenerClass = clazz;
                    break;
                }
            }
            try {
                Constructor handlerCon = handlerClass.getDeclaredConstructor(String.class,
                        Looper.class, Handler.class, CameraDevice.class, deviceInfoListenerClass);
                handlerCon.setAccessible(true);
                try {
                    Object camera2Handler = handlerCon.newInstance(BACK_CAMERA_ID,
                            mRequestThread.getLooper(), mRespondHandler, device,
                            mock(deviceInfoListenerClass));
                    Log.d(TAG, "create Camera2Handler");
                    Constructor proxyCon = Camera2Proxy.class.getDeclaredConstructor(String.class,
                            CameraDevice.class, camera2Handler.getClass(), Handler.class);
                    proxyCon.setAccessible(true);
                    mCamera2Proxy = (Camera2Proxy) proxyCon.newInstance(BACK_CAMERA_ID, device,
                            camera2Handler, mRespondHandler);
                    Log.d(TAG, "create Camera2Proxy");

                    Method updateCamera2Proxy = handlerClass.getDeclaredMethod(
                            "updateCamera2Proxy", Camera2Proxy.class);
                    updateCamera2Proxy.setAccessible(true);
                    updateCamera2Proxy.invoke(camera2Handler, mCamera2Proxy);
                    Log.d(TAG, "invoke Camera2Proxy.updateCamera2Proxy");
                    try {
                        Field field = handlerClass.getDeclaredField("mSessionStateCallback");
                        field.setAccessible(true);
                        mSessionStateCallback = (CameraCaptureSession.StateCallback) field
                                .get(camera2Handler);
                        Log.d(TAG, "set Camera2Handler mSessionStateCallback");
                    } catch (NoSuchFieldException e) {
                        Log.d(TAG, "getDeclaredField, NoSuchFieldException:" + e);
                    }
                    Constructor sessionProxy = Camera2CaptureSessionProxy.class.getConstructor(
                            Handler.class, Camera2Proxy.class);
                    mCaptureSessionProxy = (Camera2CaptureSessionProxy) sessionProxy.newInstance(
                            camera2Handler, mCamera2Proxy);

                } catch (InstantiationException e) {
                    Log.d(TAG, "newInstance, InstantiationException:" + e);
                } catch (IllegalAccessException e) {
                    Log.d(TAG, "newInstance, IllegalAccessException:" + e);
                } catch (IllegalArgumentException e) {
                    Log.d(TAG, "newInstance, IllegalArgumentException:" + e);
                } catch (InvocationTargetException e) {
                    Log.d(TAG, "newInstance, InvocationTargetException:" + e);
                }
            } catch (NoSuchMethodException e) {
                Log.d(TAG, "get declare constructor, NoSuchMethodException:" + e);
            } catch (SecurityException e) {
                Log.d(TAG, "get declare constructor, SecurityException:" + e);
            }
        } catch (ClassNotFoundException e) {
            Log.d(TAG, "Class.forName, ClassNotFoundException:" + e);
        }
    }

    private boolean isMNewerSdkVersion () {
        return android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.M;
    }
}
