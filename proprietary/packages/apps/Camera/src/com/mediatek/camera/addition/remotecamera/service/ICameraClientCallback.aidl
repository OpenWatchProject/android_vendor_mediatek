package com.mediatek.camera.addition.remotecamera.service;

interface ICameraClientCallback {
     void onPreviewFrame(in byte[] previewData);
     void onPictureTaken(in byte[] pictureData);
     void cameraServerApExit();
}
