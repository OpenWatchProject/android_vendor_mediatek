package com.mediatek.camera.tests.v3.operator;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.Direction;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.tests.v3.arch.OperatorOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

/**
 * Drag dual camera zoom to change zoom ratio.
 */
public class DragDualZoomBarOperator extends OperatorOne {

    @Override
    public boolean isSupported(int index) {
        return Utils.isFeatureSupported("com.mediatek.camera.at.dual-zoom");
    }

    @Override
    public Page getPageBeforeOperate() {
        return Page.PREVIEW;
    }

    @Override
    public Page getPageAfterOperate() {
        return Page.PREVIEW;
    }

    @Override
    public String getDescription() {
            return "Drag Dual Zoom Bar to right";
    }

    @Override
    protected void doOperate() {
        UiObject2 zoomBar = Utils.findObject(By.res("com.mediatek.camera:id/scroller")
                .enabled(true));
        do {
            UiObject2 zoomBarText =
                    Utils.findObject(By.res("com.mediatek.camera:id/zoom_rotate_layout")
                            .enabled(true));
            if (zoomBarText == null) {
                break;
            }
            zoomBarText.longClick();
            zoomBar = Utils.findObject(By.res("com.mediatek.camera:id/scroller").enabled(true));
        } while (zoomBar == null);
        zoomBar.scroll(Direction.RIGHT, 1f, 2000);
    }
}
