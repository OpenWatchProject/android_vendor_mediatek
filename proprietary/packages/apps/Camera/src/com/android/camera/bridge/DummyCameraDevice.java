package com.android.camera.bridge;

import java.util.List;

import com.android.camera.CameraManager.CameraProxy;

import android.graphics.SurfaceTexture;
import android.hardware.Camera.Area;
import android.hardware.Camera.ErrorCallback;
import android.hardware.Camera.FaceDetectionListener;
import android.hardware.Camera.Parameters;
import android.hardware.Camera.PreviewCallback;
import android.hardware.Camera.Size;
import android.location.Location;
import android.view.SurfaceHolder;

public class DummyCameraDevice implements ICameraDeviceExt {

    public static final int UNKNOWN = -1;

    @Override
    public Parameters getInitialParams() {
        return null;
    }

    @Override
    public Parameters getParameters() {
        return null;
    }

    @Override
    public ParametersExt getParametersExt() {
        return null;
    }

    @Override
    public int getCameraId() {
        return UNKNOWN;
    }

    @Override
    public CameraProxy getCameraDevice() {
        return null;
    }

    @Override
    public void setPreviewDisplayAsync(SurfaceHolder holder) {

    }

    @Override
    public void setPreviewTextureAsync(SurfaceTexture surfaceTexture) {

    }

    @Override
    public void setPhotoModeParameters(final boolean isNonePickerIntent) {

    }

    @Override
    public void setPreviewSize() {

    }

    @Override
    public void setErrorCallback(ErrorCallback cb) {

    }

    @Override
    public void setFaceDetectionListener(FaceDetectionListener listener) {

    }

    @Override
    public boolean isHdrChanged() {
        return false;
    }

    @Override
    public boolean isSceneModeChanged() {
        return false;
    }
    @Override
    public boolean isPictureSizeChanged() {
        return false;
    }

    @Override
    public void applyParametersToServer() {

    }

    @Override
    public Size getPreviewSize() {
        return null;
    }

    @Override
    public List<Size> getSupportedPreviewSizes() {
        return null;
    }

    @Override
    public void setFocusMode(String value) {

    }

    @Override
    public boolean isSupportFocusMode(String value) {
        return false;
    }

    @Override
    public void setRefocusMode(boolean isOpen) {

    }

    @Override
    public List<String> getSupportedFocusModes() {
        return null;
    }

    @Override
    public void fetchParametersFromServer() {
        return ;
    }

    @Override
    public void setPreviewFormat(int format) {

    }

    @Override
    public void setDisplayOrientation(boolean isUseDisplayOrientation) {

    }

    @Override
    public int getDisplayOrientation() {
        return 0;
    }

    @Override
    public int getCameraDisplayOrientation() {
        return 0;
    }

    @Override
    public void setJpegRotation(int orientation) {

    }

    @Override
    public int getJpegRotation() {
        return 0;
    }

    @Override
    public void setGpsParameters(Location loc) {

    }

    @Override
    public void setAutoExposureLock(boolean toggle) {

    }

    @Override
    public void setAutoWhiteBalanceLock(boolean toggle) {

    }

    @Override
    public void setFocusAreas(List<Area> focusAreas) {

    }

    @Override
    public void setMeteringAreas(List<Area> meteringAreas) {

    }

    @Override
    public void setCapturePath(String value) {

    }

    @Override
    public boolean isZoomSupported() {
        return false;
    }

    @Override
    public int getZoom() {
        return 0;
    }

    @Override
    public void setZoom(int value) {

    }

    @Override
    public void stopFaceDetection() {

    }

    @Override
    public void updateDngState(String state) {
    }

    @Override
    public String getDngState() {
        return null;
    }

    @Override
    public void updateParameters() {

    }
    @Override
    public void setOneShotPreviewCallback(PreviewCallback cb) {

    }

    @Override
    public String getZsdMode() {
        return null;
    }

    @Override
    public void setPhotoModeBasicParameters() {

    }
}
