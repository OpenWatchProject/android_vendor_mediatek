package com.mediatek.camera.tests.v3.checker;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.tests.v3.arch.Checker;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

/**
 * Check zoom bar value is correct.
 */
public class DualZoomBarValueChecker extends Checker {
    public static final int INDEX_IS_WILD = 0;
    public static final int INDEX_IS_TELE = 1;

    @Override
    public boolean isSupported(int index) {
        return Utils.isFeatureSupported("com.mediatek.camera.at.dual-zoom");
    }

    @Override
    public int getCheckCount() {
        return 2;
    }

    @Override
    public Page getPageBeforeCheck(int index) {
        return Page.PREVIEW;
    }

    @Override
    public String getDescription(int index) {
        switch (index) {
            case INDEX_IS_WILD:
                return "Check switch to wild, 1X";
            case INDEX_IS_TELE:
                return "Check switch to tele, 2X";
            default:
                break;
        }
        return null;
    }

    @Override
    protected void doCheck(int index) {
        UiObject2 zoomBarText =
                Utils.findObject(By.res("com.mediatek.camera:id/ext_zoom_text_view")
                        .enabled(true));
        switch (index) {
            case INDEX_IS_WILD:
                Utils.assertRightNow(zoomBarText.getText().equals("1.0X") || zoomBarText.getText
                        ().equals("1X"));
                break;
            case INDEX_IS_TELE:
                Utils.assertRightNow(zoomBarText.getText().equals("2.0X") || zoomBarText.getText
                        ().equals("2X"));
                break;
            default:
                break;
        }
    }
}
