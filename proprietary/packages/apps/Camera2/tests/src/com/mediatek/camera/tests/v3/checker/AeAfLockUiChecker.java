package com.mediatek.camera.tests.v3.checker;

import android.support.test.uiautomator.By;

import com.mediatek.camera.tests.v3.annotation.CoverPoint;
import com.mediatek.camera.tests.v3.annotation.NotCoverPoint;
import com.mediatek.camera.tests.v3.arch.Checker;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

/**
 * Checker used to check AE/AF Lock UI is right.
 */
@CoverPoint(pointList = {"Check the AE/AF Lock Ui and indicator is shown normally"})
@NotCoverPoint(pointList = {"Not check the position and orientation of the focus UI", "Not check " +
        "AE and AF really locked"})
public class AeAfLockUiChecker extends Checker {
    private static final String RES_FOCUS_VIEW = "com.mediatek.camera:id/focus_view";
    private static final String DESCRIPTION_TAF = "touch focus";
    /**
     * AE/AF Lock is supported.
     */
    public static final int INDEX_EXIST = 0;
    /**
     * AE/AF Lock is not supported.
     */
    public static final int INDEX_NOT_EXIST = 1;


    @Override
    public int getCheckCount() {
        return 2;
    }

    @Override
    public Page getPageBeforeCheck(int index) {
        return Page.PREVIEW;
    }


    @Override
    protected void doCheck(int index) {
        if (index == INDEX_EXIST) {
            Utils.assertObject(By.text("AE/AF Lock"));
            Utils.assertObject(By.res(RES_FOCUS_VIEW).descContains(DESCRIPTION_TAF));
        } else {
            Utils.assertNoObject(By.text("AE/AF Lock"));
            Utils.assertNoObject(By.res(RES_FOCUS_VIEW).descContains(DESCRIPTION_TAF));
        }
    }

    @Override
    public String getDescription(int index) {
        if (index == INDEX_EXIST) {
            return "Check touch focus ring, AE/AF Lock UI is showing";
        } else if (index == INDEX_NOT_EXIST) {
            return "Check no touch focus ring and AE/AF Lock UI";
        } else {
            return null;
        }
    }
}
