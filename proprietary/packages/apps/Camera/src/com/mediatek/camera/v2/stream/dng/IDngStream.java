package com.mediatek.camera.v2.stream.dng;

import android.hardware.camera2.CameraCharacteristics;

import com.mediatek.camera.v2.stream.ICaptureStream;

public interface IDngStream extends ICaptureStream {
    public static String               CAPUTRE_RAW_SURFACE_KEY = "PreviewStream.RawSurface";
    public void updateCameraCharacteristics(CameraCharacteristics csdata);
}
