package com.mediatek.camera.tests.v3.observer;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.BySelector;

import com.mediatek.camera.tests.v3.util.Utils;

/**
 * Zoom bar value observer.
 */
public class DualZoomBarValueObserver extends UiAppearedObserver {
    private static BySelector[] sUiSelectorListBack = {
            By.text("1.0X"), By.text("3.1X"), By.text("10.0X")
    };
    private static BySelector[] sUiSelectorListFront = {
            By.text("1.0X"), By.text("2.0X"), By.text("4.0X")
    };
    private static String[] sUiDescriptionListBack = {"1.0X", "3.1X", "10.0X"};
    private static String[] sUiDescriptionListFront = {"1.0X", "2.0X", "4.0X"};

    /**
     * Constructor.
     *
     * @param isFront front camera
     */
    public DualZoomBarValueObserver(boolean isFront) {
        super(isFront ? sUiSelectorListFront : sUiSelectorListBack,
                isFront ? sUiDescriptionListFront : sUiDescriptionListBack);
    }

    @Override
    public boolean isSupported(int index) {
        return Utils.isFeatureSupported("com.mediatek.camera.at.dual-zoom");
    }
}
