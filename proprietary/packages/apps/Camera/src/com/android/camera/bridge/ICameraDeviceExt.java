package com.android.camera.bridge;

import java.util.List;

import com.android.camera.CameraManager;

import android.graphics.SurfaceTexture;
import android.hardware.Camera.Area;
import android.hardware.Camera.ErrorCallback;
import android.hardware.Camera.FaceDetectionListener;
import android.hardware.Camera.Parameters;
import android.hardware.Camera.PreviewCallback;
import android.hardware.Camera.Size;
import android.location.Location;
import android.view.SurfaceHolder;

public interface ICameraDeviceExt {
    public Parameters getInitialParams();

    public Parameters getParameters();

    public ParametersExt getParametersExt();

    public int getCameraId();

    public CameraManager.CameraProxy getCameraDevice();

    public void setPreviewDisplayAsync(SurfaceHolder holder);

    public void setPreviewTextureAsync(SurfaceTexture surfaceTexture);

    public void stopFaceDetection();

    /**
     * Apply parameters for photo capture mode.
     * @param isNonePickerIntent True if camera is launched from launcher.
     */
    public void setPhotoModeParameters(final boolean isNonePickerIntent);

    //TODO
    public void setPreviewSize();

    public void setErrorCallback(ErrorCallback cb);

    public void setFaceDetectionListener(FaceDetectionListener listener);

    public boolean isHdrChanged();

    public boolean isSceneModeChanged();
    public boolean isPictureSizeChanged();

    public void updateParameters();

    public void applyParametersToServer();

    public Size getPreviewSize();

    public String getZsdMode();

    public List<Size> getSupportedPreviewSizes();

    public void setFocusMode(String value);

    public boolean isSupportFocusMode(String value);

    public void setRefocusMode(boolean isOpen);

    public List<String> getSupportedFocusModes();

    public void fetchParametersFromServer();

    public void setPreviewFormat(int format);

    public void setDisplayOrientation(boolean isUseDisplayOrientation);

    public int getDisplayOrientation();

    public int getCameraDisplayOrientation();

    public void setJpegRotation(int orientation);

    public int getJpegRotation();

    public void setGpsParameters(Location loc) ;

    public void setAutoExposureLock(boolean toggle);

    public void setAutoWhiteBalanceLock(boolean toggle);

    public void setFocusAreas(List<Area> focusAreas);

    public void setMeteringAreas(List<Area> meteringAreas);

    public void setCapturePath(String value);

    /**
     * update dng state to on or off.
     * @param state "on" or "off".
     */
    public void updateDngState(String state);

    /**
     * get dng state.
     * @return "on" or "off".
     */
    public String getDngState();

    public boolean isZoomSupported();

    public int getZoom();

    public void setZoom(int value);
    public void setOneShotPreviewCallback(PreviewCallback cb);
    /**
     * Set photo mode basic parameters such as mtk-cam-mode.
     */
    public void setPhotoModeBasicParameters();
}
