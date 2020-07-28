package com.mediatek.camera.tests.v3.util;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiWatcher;

import com.mediatek.camera.common.debug.LogUtil;

public class ConnectCameraWatcher implements UiWatcher {
    private static final LogUtil.Tag TAG = Utils.getTestTag(ConnectCameraWatcher.class
            .getSimpleName());
    private static final String NOT_CONNECT_CAMERA = "Can't connect to the camera. Please make" +
            " sure to close other apps that may use camera or flashlight.";

    @Override
    public boolean checkForCondition() {
        if (Utils.getUiDevice().hasObject(By.text(NOT_CONNECT_CAMERA))) {
            LogHelper.d(TAG, "[checkForCondition] not connect camera, try to reboot device");
            Utils.rebootDevice();
            return true;
        } else {
            return false;
        }
    }
}
