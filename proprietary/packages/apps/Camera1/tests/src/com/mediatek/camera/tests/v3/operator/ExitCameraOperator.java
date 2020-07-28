package com.mediatek.camera.tests.v3.operator;

import android.support.test.uiautomator.By;

import com.mediatek.camera.tests.v3.arch.OperatorOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

public class ExitCameraOperator extends OperatorOne {
    public static final String MTK_CAMERA_PACKAGE = "com.mediatek.camera";

    @Override
    protected void doOperate() {
        Utils.pressBackUtilFindNoObject(By.pkg(MTK_CAMERA_PACKAGE), 3);
    }

    @Override
    public Page getPageBeforeOperate() {
        return Page.PREVIEW;
    }

    @Override
    public Page getPageAfterOperate() {
        return null;
    }

    @Override
    public String getDescription() {
        return "Press back key to exit camera activity";
    }
}
