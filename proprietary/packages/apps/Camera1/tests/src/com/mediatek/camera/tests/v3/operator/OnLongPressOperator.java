package com.mediatek.camera.tests.v3.operator;

import com.mediatek.camera.tests.v3.util.Utils;

/**
 * Operator used to do long press on the screen.
 */
public class OnLongPressOperator extends OnSingleTapUpOperator {
    private static final int STEPS = 50;

    @Override
    public String getDescription(int index) {
        return "Long press to at point " + index +
                " (" + sPointList[index].x + ", " + sPointList[index].y + ")";
    }

    @Override
    protected void doOperate(int index) {
        Utils.getUiDevice().swipe(sPointList[index].x, sPointList[index].y, sPointList[index].x,
                sPointList[index].y, STEPS);
        Utils.waitSafely(Utils.TIME_OUT_SHORT_SHORT);
    }
}
