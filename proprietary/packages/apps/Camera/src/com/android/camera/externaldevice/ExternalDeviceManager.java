package com.android.camera.externaldevice;

import com.android.camera.CameraActivity;

import java.util.Vector;

public class ExternalDeviceManager {

    private Vector<IExternalDeviceCtrl> mIDeviceConnected = new Vector<IExternalDeviceCtrl>();

    public ExternalDeviceManager(CameraActivity cameraActivity) {
        mIDeviceConnected.add(new WfdDeviceCtrl(cameraActivity));
    }

    public boolean onCreate() {
        for (IExternalDeviceCtrl iDeviceConnected : mIDeviceConnected) {
            iDeviceConnected.onCreate();
        }
        return false;
    }

    public boolean onResume() {
        for (IExternalDeviceCtrl iDeviceConnected : mIDeviceConnected) {
            iDeviceConnected.onResume();
        }
        return false;
    }

    public boolean onPause() {
        for (IExternalDeviceCtrl iDeviceConnected : mIDeviceConnected) {
            iDeviceConnected.onPause();
        }
        return false;
    }

    public boolean onDestory() {
        for (IExternalDeviceCtrl iDeviceConnected : mIDeviceConnected) {
            iDeviceConnected.onDestory();
        }
        return false;
    }


    public boolean onOrientationChanged(int orientation) {
        for (IExternalDeviceCtrl iDeviceConnected : mIDeviceConnected) {
            iDeviceConnected.onOrientationChanged(orientation);
        }
        return false;
    }

    public void addListener(Object listenr) {
        for (IExternalDeviceCtrl iDeviceConnected : mIDeviceConnected) {
            iDeviceConnected.addListener(listenr);
        }
    }

    public void removeListener(Object listenr) {
        for (IExternalDeviceCtrl iDeviceConnected : mIDeviceConnected) {
            iDeviceConnected.removeListener(listenr);
        }
    }

}
