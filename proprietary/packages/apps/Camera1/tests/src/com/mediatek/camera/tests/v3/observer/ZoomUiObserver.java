package com.mediatek.camera.tests.v3.observer;

import android.support.test.uiautomator.By;

import com.mediatek.camera.tests.v3.annotation.CoverPoint;
import com.mediatek.camera.tests.v3.util.Utils;

@CoverPoint(pointList = {"Check zoom ratio view has shown"})
public class ZoomUiObserver extends UiAppearedObserver {
    public ZoomUiObserver() {
        super(By.res("com.mediatek.camera:id/auto_hide_hint"), "zoom ratio view");
    }

    @Override
    public boolean isSupported(int index) {
        return !Utils.isFeatureSupported("com.mediatek.camera.at.dual-zoom");
    }

}
