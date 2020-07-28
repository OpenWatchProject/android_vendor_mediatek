package com.mediatek.camera.tests.v3.checker;

import android.support.test.uiautomator.By;

import com.mediatek.camera.tests.v3.annotation.CoverPoint;
import com.mediatek.camera.tests.v3.annotation.NotCoverPoint;
import com.mediatek.camera.tests.v3.arch.Checker;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

/**
 * Checker used to check focus uI is shown and hidden normally.
 */
@CoverPoint(pointList = {"Check the focus UI is shown and hidden immediately"})
@NotCoverPoint(pointList = {"Not check the position and orientation of the focus UI"})
public class FocusUiChecker extends Checker {
    public static final int INDEX_NO_AF_UI = 0;
    public static final int INDEX_HAS_CAF_UI = 1;
    public static final int INDEX_HAS_TAF_UI = 2;

    private static final String RES_FOCUS_VIEW = "com.mediatek.camera:id/focus_view";
    private static final String DESCRIPTION_CAF = "continue focus";
    private static final String DESCRIPTION_TAF = "touch focus";

    @Override
    public int getCheckCount() {
        return 3;
    }

    @Override
    public Page getPageBeforeCheck(int index) {
        return Page.PREVIEW;
    }

    @Override
    public String getDescription(int index) {
        switch (index) {
            case INDEX_NO_AF_UI:
                return "Check no focus ui is shown";
            case INDEX_HAS_CAF_UI:
                return "Check continuous focus ring is showing";
            case INDEX_HAS_TAF_UI:
                return "Check touch focus ring is showing";
            default:
                return null;
        }
    }

    @Override
    protected void doCheck(int index) {
        switch (index) {
            case INDEX_NO_AF_UI:
                Utils.assertNoObject(By.res(RES_FOCUS_VIEW));
                break;
            case INDEX_HAS_CAF_UI:
                Utils.assertObject(By.res(RES_FOCUS_VIEW).descContains(DESCRIPTION_CAF));
                break;
            case INDEX_HAS_TAF_UI:
                Utils.assertObject(By.res(RES_FOCUS_VIEW).descContains(DESCRIPTION_TAF));
                break;
            default:
                break;
        }
    }
}
