package com.mediatek.camera.tests.v3.operator;


import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.Operator;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

import junit.framework.Assert;

import java.util.regex.Pattern;

public class CancelPanoramaOperator extends Operator {
    private static final LogUtil.Tag TAG = Utils.getTestTag(CapturePhotoOperator.class
            .getSimpleName());
    public static final int CANCEL_INDEX = 0;
    public static final int CANCEL_THEN_SAVE_QUICKLY_INDEX = 1;

    @Override
    protected void doOperate(int index) {
        Pattern btnSavePattern = Pattern.compile(".*btn.*_save");
        Pattern btnCancelPattern = Pattern.compile(".*btn.*_cancel");

        UiObject2 panoramaCancel = Utils.findObject(By.res(btnCancelPattern));
        Assert.assertNotNull(panoramaCancel);
        UiObject2 panoramaSave = Utils.findObject(By.res(btnSavePattern));
        Assert.assertNotNull(panoramaSave);
        switch (index) {
            case CANCEL_INDEX:
                if (panoramaCancel != null) {
                    panoramaCancel.click();
                }
                break;
            case CANCEL_THEN_SAVE_QUICKLY_INDEX:
                if (panoramaCancel != null) {
                    panoramaCancel.click();
                }
                if (panoramaSave != null) {
                    panoramaSave.click();
                }
                break;
            default:
                break;
        }
    }

    @Override
    public boolean isSupported(int index) {
        return Utils.isFeatureSupported("com.mediatek.camera.at.panorama");
    }

    @Override
    public int getOperatorCount() {
        return 2;
    }

    @Override
    public Page getPageBeforeOperate(int index) {
        return Page.PREVIEW;
    }

    @Override
    public Page getPageAfterOperate(int index) {
        return Page.PREVIEW;
    }

    @Override
    public String getDescription(int index) {
        switch (index) {
            case CANCEL_INDEX:
                return "Cancel capturing panorama photo.";
            case CANCEL_THEN_SAVE_QUICKLY_INDEX:
                return "Click cancel button and then save button";
        }
        return null;
    }
}
