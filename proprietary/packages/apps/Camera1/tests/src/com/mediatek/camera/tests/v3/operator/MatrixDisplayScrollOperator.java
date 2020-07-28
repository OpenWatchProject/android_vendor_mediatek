package com.mediatek.camera.tests.v3.operator;

import android.graphics.Rect;
import android.support.test.uiautomator.By;
import android.support.test.uiautomator.Direction;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.OperatorOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

/**
 * Operator first scroll to left, then scroll to right, last scroll out.
 */

public class MatrixDisplayScrollOperator extends OperatorOne {
    private static final LogUtil.Tag TAG = Utils.getTestTag(
            MatrixDisplayEffectSelectOperator.class.getSimpleName());

    @Override
    public Page getPageBeforeOperate() {
        return null;
    }

    @Override
    public Page getPageAfterOperate() {
        return null;
    }

    @Override
    public String getDescription() {
        return "Scroll on matrix display";
    }

    @Override
    protected void doOperate() {
        Utils.assertObject(By.res("com.mediatek.camera:id/lomo_effect_layout"));
        UiObject2 matrixDisplayLayout = Utils.findObject(
                By.res("com.mediatek.camera:id/lomo_effect_layout"));
        Rect rect = matrixDisplayLayout.getVisibleBounds();

        int layoutWidth = rect.right - rect.left;
        int layoutHeight = rect.bottom - rect.top;
        int columnWidth = layoutWidth / 3;
        int rowWidth = layoutHeight / 3;

        matrixDisplayLayout.scroll(Direction.LEFT, 1, columnWidth / 2);
        matrixDisplayLayout.scroll(Direction.RIGHT, 2f / 3f, columnWidth / 2);
        matrixDisplayLayout.scroll(Direction.RIGHT, 0.6f, columnWidth / 2);
    }

    @Override
    public boolean isSupported(int index) {
        return Utils.isFeatureSupported("com.mediatek.camera.at.matrix-display");
    }
}
