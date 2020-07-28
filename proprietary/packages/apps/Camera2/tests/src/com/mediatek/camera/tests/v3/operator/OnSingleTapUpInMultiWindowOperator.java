package com.mediatek.camera.tests.v3.operator;

import android.graphics.Point;
import android.graphics.Rect;
import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.Operator;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

/**
 * Operator used to single tap up on the multi-window,36 points will be clicked when cross to use
 * the
 * operator.
 */
public class OnSingleTapUpInMultiWindowOperator extends Operator {
    private static final int COLUMN = 6;
    private static final int ROW = 6;
    protected static Point[] sPointList = new Point[COLUMN * ROW];
    private static final LogUtil.Tag TAG = Utils.getTestTag(OnSingleTapUpInMultiWindowOperator
            .class.getSimpleName());

    static {
        UiObject2 uiRoot = Utils.findObject(By.res("com.mediatek.camera:id/app_ui_root"));
        Utils.assertRightNow(uiRoot != null);
        Rect rect = uiRoot.getVisibleBounds();
        int width = rect.width();
        int height = rect.height();
        LogHelper.d(TAG, "the window size is ( " + width + ",height " + height + ")");
        int stepSizeX = width / COLUMN;
        int stepSizeY = height * 2 / 3 / ROW;
        int startX = stepSizeX / 2;
        int startY = height / 12 + stepSizeY / 2;
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
                " (" + sPointList[index].x + ", " + sPointList[index].y + ")" + " in multi window";
    }

    @Override
    protected void doOperate(int index) {
        Utils.getUiDevice().click(sPointList[index].x, sPointList[index].y);
        Utils.waitSafely(Utils.TIME_OUT_SHORT_SHORT);
    }

}
