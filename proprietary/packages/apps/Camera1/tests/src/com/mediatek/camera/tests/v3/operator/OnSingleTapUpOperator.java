package com.mediatek.camera.tests.v3.operator;

import android.graphics.Point;

import com.mediatek.camera.tests.v3.arch.Operator;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

/**
 * Operator used to single tap up on the screen,36 points will be clicked when cross to use the
 * operator.
 */
public class OnSingleTapUpOperator extends Operator {
    private static final int COLUMN = 6;
    private static final int ROW = 6;
    protected static Point[] sPointList = new Point[COLUMN * ROW];

    static {
        int stepSizeX = Utils.getUiDevice().getDisplayWidth() / COLUMN;
        int stepSizeY = Utils.getUiDevice().getDisplayHeight() * 2 / 3 / ROW;
        int startX = stepSizeX / 2;
        int startY = Utils.getUiDevice().getDisplayHeight() / 12 + stepSizeY / 2;

        for (int column = 0; column < COLUMN; column++) {
            for (int row = 0; row < ROW; row++) {
                sPointList[column * ROW + row] =
                        new Point(startX + column * stepSizeX, startY + row * stepSizeY);
            }
        }
    }

    @Override
    public int getOperatorCount() {
        return sPointList.length;
    }

    @Override
    public Page getPageBeforeOperate(int index) {
        return Page.PREVIEW;
    }

    @Override
    public Page getPageAfterOperate(int index) {
        return Page.PREVIEW;
    }

    @Override
    public String getDescription(int index) {
        return "OnSingleTapUp at point " + index +
                " (" + sPointList[index].x + ", " + sPointList[index].y + ")";
    }

    @Override
    protected void doOperate(int index) {
        Utils.getUiDevice().click(sPointList[index].x, sPointList[index].y);
        Utils.waitSafely(Utils.TIME_OUT_SHORT_SHORT);
    }

}
