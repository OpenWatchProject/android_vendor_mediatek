package com.mediatek.camera.tests.common.device;

import android.content.Context;
import android.hardware.Camera;
import android.hardware.Camera.AutoFocusMoveCallback;
import android.hardware.Camera.Parameters;
import android.os.HandlerThread;
import android.os.Looper;

import com.mediatek.camera.common.device.CameraDeviceManagerFactory.CameraApi;
import com.mediatek.camera.common.device.v1.CameraHandler;
import com.mediatek.camera.common.device.v1.CameraProxy;
import com.mediatek.camera.tests.CameraUnitTestCaseBase;
import com.mediatek.camera.tests.Log;

import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

/**
 * Test camera proxy.
 */
public class CameraProxyTestCase extends CameraUnitTestCaseBase {
    private static final String TAG = CameraProxyTestCase.class.getSimpleName();

    private HandlerThread mRequestThread;
    private HandlerThread mRespondThread;
    private CameraProxy mCameraProxy;
    private static final String BACK_CAMERA_ID = "0";

    /**
     * Test get camera id.
     */
    public void testGetId() {
        Camera camera = mock(Camera.class);
        Camera.Parameters param = mock(Camera.Parameters.class);
        when(camera.getParameters()).thenReturn(param);

        createCameraProxy(camera);
        String cameraId = mCameraProxy.getId();
        assertTrue("exception, get wrong camera id!", cameraId.equals(BACK_CAMERA_ID));
    }

    /**
     * Test get camera.
     */
    public void testGetCamera() {
        Camera camera = mock(Camera.class);
        Camera.Parameters param = mock(Camera.Parameters.class);
        when(camera.getParameters()).thenReturn(param);

        createCameraProxy(camera);
        Camera res = mCameraProxy.getCamera();
        assertTrue("camera proxy get wrong camera!", res == camera);
    }

    /**
     * Test set auto focus move callback.
     */
    public void testSetAutoFocusMoveCallback() {
        Camera camera = mock(Camera.class);
        Camera.Parameters param = mock(Camera.Parameters.class);
        when(camera.getParameters()).thenReturn(param);

        createCameraProxy(camera);
        AutoFocusMoveCallback callback = mock(AutoFocusMoveCallback.class);

        mCameraProxy.setAutoFocusMoveCallback(callback);
        verify(camera, times(1)).setAutoFocusMoveCallback(callback);
    }

    /**
     * Test get parameters.
     */
    public void testGetParameters() {
        Camera camera = mock(Camera.class);
        Camera.Parameters param = mock(Camera.Parameters.class);
        when(camera.getParameters()).thenReturn(param);

        createCameraProxy(camera);

        Parameters parameters = mCameraProxy.getParameters();
        assertTrue("getParameters, return wrong parameters!", param == parameters);
    }

    private void createCameraProxy(Camera camera) {
        mRequestThread = new HandlerThread(CameraApi.API1 + "-Request-" + BACK_CAMERA_ID);
        mRespondThread = new HandlerThread(CameraApi.API1 + "-Response-" + BACK_CAMERA_ID);
        mRequestThread.start();
        mRespondThread.start();

        try {
            Class handlerClass = Class
                    .forName("com.mediatek.camera.common.device.v1.CameraHandler");
            try {
                Constructor handlerCon = handlerClass.getDeclaredConstructor(Context.class,
                        String.class, Looper.class, Camera.class,
                        CameraHandler.IDeviceInfoListener.class);
                handlerCon.setAccessible(true);

                try {
                    Object instance = mock(CameraHandler.IDeviceInfoListener.class);
                    Object cameraHandler = handlerCon.newInstance(getContext(), BACK_CAMERA_ID,
                            mRequestThread.getLooper(), camera, instance);
                    Constructor proxyCon = CameraProxy.class.getConstructor(String.class,
                            handlerClass, Camera.class);
                    mCameraProxy = (CameraProxy) proxyCon.newInstance(BACK_CAMERA_ID,
                            cameraHandler, camera);
                } catch (InstantiationException e) {
                    Log.d(TAG, "InstantiationException" + e);
                } catch (IllegalAccessException e) {
                    Log.d(TAG, "IllegalAccessException" + e);
                } catch (IllegalArgumentException e) {
                    Log.d(TAG, "IllegalArgumentException" + e);
                } catch (InvocationTargetException e) {
                    Log.d(TAG, "InvocationTargetException" + e);
                }
            } catch (NoSuchMethodException e) {
                Log.d(TAG, "NoSuchMethodException" + e);
            } catch (SecurityException e) {
                Log.d(TAG, "SecurityException" + e);
            }
        } catch (ClassNotFoundException e) {
            Log.d(TAG, "ClassNotFoundException" + e);
        }
    }
}