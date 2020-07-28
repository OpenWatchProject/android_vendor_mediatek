package com.mediatek.camera.tests.v3.checker;

import android.support.test.uiautomator.By;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.annotation.CoverPoint;
import com.mediatek.camera.tests.v3.arch.Checker;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

@CoverPoint(pointList = {"Check current active package is camera package",
        "Check shutter ui status"})
public class CameraLaunchedChecker extends Checker {
    public static final int INDEX_NORMAL = 0;
    public static final int INDEX_INTENT_PHOTO = 1;
    public static final int INDEX_INTENT_VIDEO = 2;
    public static final int INDEX_INTENT_PHOTO_NO_URI = 3;
    public static final int INDEX_INTENT_VIDEO_NO_URI = 4;
    public static final int INDEX_SECURE_CAMERA = 5;

    private static final LogUtil.Tag TAG = Utils.getTestTag(CameraLaunchedChecker.class
            .getSimpleName());
    public static final String MTK_CAMERA_PACKAGE = "com.mediatek.camera";

    @Override
    protected void doCheck(int index) {
        //Utils.assertObject(By.pkg(MTK_CAMERA_PACKAGE));
        switch (index) {
            case INDEX_NORMAL:
                Utils.assertObject(By.res("com.mediatek.camera:id/shutter_text").text("Picture"));
                Utils.assertObject(By.res("com.mediatek.camera:id/shutter_text").text("Video"));
                break;
            case INDEX_INTENT_PHOTO:
            case INDEX_INTENT_PHOTO_NO_URI:
                Utils.assertObject(By.res("com.mediatek.camera:id/shutter_text").text("Picture"));
                Utils.assertNoObject(By.res("com.mediatek.camera:id/shutter_text").text("Video"));
                break;
            case INDEX_INTENT_VIDEO:
            case INDEX_INTENT_VIDEO_NO_URI:
                Utils.assertNoObject(By.res("com.mediatek.camera:id/shutter_text").text("Picture"));
                Utils.assertObject(By.res("com.mediatek.camera:id/shutter_text").text("Video"));
                break;
            case INDEX_SECURE_CAMERA:
                Utils.assertObject(By.res("com.mediatek.camera:id/shutter_text").text("Picture"));
                Utils.assertObject(By.res("com.mediatek.camera:id/shutter_text").text("Video"));
                break;
            default:
                break;
        }
    }

    @Override
    public int getCheckCount() {
        return 6;
    }

    @Override
    public Page getPageBeforeCheck(int index) {
        return null;
    }

    @Override
    public String getDescription(int index) {
        switch (index) {
            case INDEX_NORMAL:
            case INDEX_SECURE_CAMERA:
                return "Check camera is launched in normal mode";
            case INDEX_INTENT_PHOTO:
            case INDEX_INTENT_PHOTO_NO_URI:
                return "Check camera is launched in intent photo mode";
            case INDEX_INTENT_VIDEO:
            case INDEX_INTENT_VIDEO_NO_URI:
                return "Check camera is launched in intent video mode";
            default:
                return null;
        }
    }
}
