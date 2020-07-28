package com.mediatek.camera.tests.v3.observer;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.BySelector;

import com.mediatek.camera.tests.v3.util.Utils;

public class DofValueObserver extends UiAppearedObserver {
    private static BySelector[] uiSelectorList = {
            By.text("F11"), By.text("F4.5"), By.text("F0.8")
    };
    private static String[] uiDescriptionList = {"F11", "F4.5", "F0.8"};

    public DofValueObserver() {
        super(uiSelectorList, uiDescriptionList);
    }

    @Override
    public boolean isSupported(int index) {
        return Utils.isFeatureSupported("com.mediatek.camera.at.vsdof");
    }
}
