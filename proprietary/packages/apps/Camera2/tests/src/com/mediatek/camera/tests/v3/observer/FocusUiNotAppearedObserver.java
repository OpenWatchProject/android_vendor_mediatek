package com.mediatek.camera.tests.v3.observer;

import android.support.test.uiautomator.By;

public class FocusUiNotAppearedObserver extends UiNotAppearedObserver {
    public FocusUiNotAppearedObserver() {
        super(By.res("com.mediatek.camera:id/focus_ring"), "focus ring");
    }
}
