package com.mediatek.camera.tests.v3.checker;

import android.support.test.uiautomator.By;

import com.mediatek.camera.tests.v3.annotation.CoverPoint;
import com.mediatek.camera.tests.v3.arch.CheckerOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

@CoverPoint(pointList = {"Check current active package is not camera package"})
public class CameraExitedChecker extends CheckerOne {
    public static final String MTK_CAMERA_PACKAGE = "com.mediatek.camera";

    @Override
    protected void doCheck() {
        Utils.assertNoObject(By.pkg(MTK_CAMERA_PACKAGE));
    }

    @Override
    public Page getPageBeforeCheck() {
        return null;
    }

    @Override
    public String getDescription() {
        return "Check camera is existed";
    }
}
