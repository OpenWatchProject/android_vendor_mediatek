package com.mediatek.camera.tests.v3.operator;

import android.graphics.Rect;
import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.tests.v3.arch.Operator;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

/**
 * Switch pip.
 */
public class SwitchPipOperator extends Operator {
    public static final int INDEX_TOP_GRAPHIC = 0;
    public static final int INDEX_SWITCH_CAMERA = 1;

    @Override
    protected void doOperate(int index) {
        switch (index) {
            case INDEX_TOP_GRAPHIC:
                UiObject2 shutterStop = Utils.findObject(By.res("com.mediatek" +
                        ".camera:id/preview_surface"));
                Rect rect = shutterStop.getVisibleBounds();
                // x*2/3  y*1/4
                int x = rect.left + rect.width() * 2 / 3;
                int y = rect.top + rect.height() * 1 / 4;
                Utils.getUiDevice().click(x, y);
                break;
            case INDEX_SWITCH_CAMERA:
                UiObject2 switchIcon = Utils.findObject(By.res("com.mediatek" +
                        ".camera:id/camera_switcher").checkable(true));
                switchIcon.click();
                break;
            default:
                break;
        }
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
            case INDEX_TOP_GRAPHIC:
                return "click top graphic to switch pip";
            case INDEX_SWITCH_CAMERA:
                return "click switch camera to switch pip";
            default:
                return "";
        }
    }

    @Override
    public boolean isSupported(int index) {
        return Utils.isFeatureSupported("com.mediatek.camera.at.pip");
    }

    @Override
    public int getOperatorCount() {
        return 2;
    }

}
