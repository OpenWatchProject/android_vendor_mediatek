package com.mediatek.camera.tests.v3.operator;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.Operator;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

import java.util.List;

public class SwitchPhotoVideoOperator extends Operator {
    private static final LogUtil.Tag TAG = Utils.getTestTag(
            SwitchPhotoVideoOperator.class.getSimpleName());

    public static final int INDEX_PHOTO = 0;
    public static final int INDEX_VIDEO = 1;

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
            case INDEX_PHOTO:
                return "Try to switch to photo mode, if current mode not support photo, keep " +
                        "current status";
            case INDEX_VIDEO:
                return "Try to switch to video mode, if current mode not support video, keep " +
                        "current status";
            default:
                break;
        }
        return null;
    }

    @Override
    protected void doOperate(int index) {
        UiObject2 shutterRoot = Utils.findObject(By.res("com.mediatek.camera:id/shutter_root"));
        Utils.assertRightNow(shutterRoot != null);

        // when there is only one shutter mode, check if supported
        List<UiObject2> shutterList = shutterRoot.getChildren();
        if (shutterList.size() == 1) {
            switch (index) {
                case INDEX_PHOTO:
                    if (Utils.findObject(By.res("com.mediatek.camera:id/shutter_text")).getText()
                            .equals("Video")) {
                        LogHelper.d(TAG, "[doOperate] only one shutter mode is video, can not " +
                                "switch to photo, return");
                        return;
                    }
                    break;
                case INDEX_VIDEO:
                    if (Utils.findObject(By.res("com.mediatek.camera:id/shutter_text")).getText()
                            .equals("Picture")) {
                        LogHelper.d(TAG, "[doOperate] only one shutter mode is photo, can not " +
                                "switch to video, return");
                        return;
                    }
                    break;
                default:
                    break;
            }
        }

        if (shutterRoot.getContentDescription().contains("PhotoMode")) {
            if (index == INDEX_PHOTO) {
                return;
            } else {
                switchToMode(INDEX_VIDEO);
            }
        }

        if (shutterRoot.getContentDescription().contains("VideoMode")) {
            if (index == INDEX_VIDEO) {
                return;
            } else {
                switchToMode(INDEX_PHOTO);
            }
        }
    }

    private void switchToMode(int index) {
        int startX = Utils.getUiDevice().getDisplayWidth() / 4;
        int endX = Utils.getUiDevice().getDisplayWidth() / 4 * 3;
        int startY = Utils.getUiDevice().getDisplayWidth() / 3 * 2;
        int endY = startY;
        int swipeSteps = 5;
        switch (index) {
            case INDEX_VIDEO:
                Utils.getUiDevice().swipe(endX, startY, startX, endY, swipeSteps);
            case INDEX_PHOTO:
                Utils.getUiDevice().swipe(startX, startY, endX, endY, swipeSteps);
            default:
                break;
        }
    }
}
