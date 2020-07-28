package com.mediatek.camera.addition.remotecamera.service;

import android.os.Message;
import com.mediatek.camera.addition.remotecamera.service.ICameraClientCallback;

interface IMtkCameraService {
    void openCamera();
    void releaseCamera();
    void capture();
    void sendMessage(in Message msg);
    void registerCallback(ICameraClientCallback cb);
    void unregisterCallback(ICameraClientCallback cb);
    void setFrameRate(int frameRate);
    // add for release
    void cameraServerExit();
    String getSupportedFeatureList();
}
