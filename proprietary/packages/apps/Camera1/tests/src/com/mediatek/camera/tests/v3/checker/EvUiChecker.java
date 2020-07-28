package com.mediatek.camera.tests.v3.checker;

import android.support.test.uiautomator.By;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.annotation.CoverPoint;
import com.mediatek.camera.tests.v3.annotation.NotCoverPoint;
import com.mediatek.camera.tests.v3.arch.Checker;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

/**
 * Checker used to check EV uI is shown or hidden normally.
 */
@CoverPoint(pointList = {"Check the EV UI is shown or hidden immediately"})
@NotCoverPoint(pointList = {"Not check the position and orientation of the EV UI"})
public class EvUiChecker extends Checker {
    private static final LogUtil.Tag TAG = Utils.getTestTag(EvUiChecker.class
            .getSimpleName());
    /**
     * Index of no EV UI.
     */
    public static final int INDEX_EXIST = 0;
    /**
     * Index of has EV UI.
     */
    public static final int INDEX_NOT_EXIST = 1;

    private static final String RES_EV_BAR = "com.mediatek.camera:id/ev_seekbar";


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
            case INDEX_EXIST:
                return "Check EV ui is shown";
            case INDEX_NOT_EXIST:
                return "Check no EV ui shown";
            default:
                return null;
        }
    }

    @Override
    protected void doCheck(int index) {
        switch (index) {
            case INDEX_EXIST:
                Utils.assertObject(By.res(RES_EV_BAR));
                break;
            case INDEX_NOT_EXIST:
                Utils.assertNoObject(By.res(RES_EV_BAR));
                break;
            default:
                break;
        }
    }
}
