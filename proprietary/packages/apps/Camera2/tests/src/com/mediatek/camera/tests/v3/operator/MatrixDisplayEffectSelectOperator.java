package com.mediatek.camera.tests.v3.operator;

import android.graphics.Rect;
import android.support.test.uiautomator.By;
import android.support.test.uiautomator.Direction;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.Operator;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

/**
 * Matrix display effect select operator.
 */

public class MatrixDisplayEffectSelectOperator extends Operator {
    private static final LogUtil.Tag TAG = Utils.getTestTag(
            MatrixDisplayEffectSelectOperator.class.getSimpleName());

    public static final int INDEX_EFFECT_MODE_NONE = 0;
    public static final int INDEX_EFFECT_MODE_MONO = 1;
    public static final int INDEX_EFFECT_MODE_NEGATIVE = 2;
    public static final int INDEX_EFFECT_MODE_SEPIA = 3;
    public static final int INDEX_EFFECT_MODE_POSTERIZE = 4;
    public static final int INDEX_EFFECT_MODE_AQUA = 5;
    public static final int INDEX_EFFECT_MODE_BLACKBOARD = 6;
    public static final int INDEX_EFFECT_MODE_WHITEBOARD = 7;

    public static final int INDEX_EFFECT_MODE_NASHVILLE = 8;
    public static final int INDEX_EFFECT_MODE_HEFE = 9;
    public static final int INDEX_EFFECT_MODE_VALENCIA = 10;
    public static final int INDEX_EFFECT_MODE_XPROII = 11;
    public static final int INDEX_EFFECT_MODE_LOFI = 12;
    public static final int INDEX_EFFECT_MODE_SIERRA = 13;
    public static final int INDEX_EFFECT_MODE_WALDEN = 14;

    private static final String[] EFFECT_NAME = {
            "None",
            "Mono",
            "Sepia",
            "Negative",
            "Posterize",
            "Aqua",
            "Blackboard",
            "Whiteboard",
            "Fresh",
            "Movie",
            "Calm",
            "Memory",
            "Gorgeous",
            "Elegant",
            "Cool",
    };

    @Override
    public void operate(int index) {
        super.operate(index);
    }

    @Override
    public boolean isSupported(int index) {
        return Utils.isFeatureSupported("com.mediatek.camera.at.matrix-display");
    }

    @Override
    public int getOperatorCount() {
        return 15;
    }

    @Override
    public Page getPageBeforeOperate(int index) {
        return Page.PREVIEW;
    }

    @Override
    public Page getPageAfterOperate(int index) {
        return null;
    }

    @Override
    public String getDescription(int index) {
        return "select " + EFFECT_NAME[index] + " effect";
    }

    @Override
    protected void doOperate(int index) {
        UiObject2 matrixDisplayLayout = Utils.findObject(
                By.res("com.mediatek.camera:id/lomo_effect_layout"));
        Rect rect = matrixDisplayLayout.getVisibleBounds();

        int layoutWidth = rect.right - rect.left;
        int layoutHeight = rect.bottom - rect.top;
        int columnWidth = layoutWidth / 3;
        int rowWidth = layoutHeight / 3;

        if (index == 9 || index == 12) {
            matrixDisplayLayout.scroll(Direction.LEFT,
                    (float) columnWidth / (float) layoutWidth);
        }

        UiObject2 effectNameView = null;
        if (index % 3 == 2) {
            // Because navigation bar will cover on matrix display layout
            // when layout is full screen. In this case, the uiautomator can't find the
            // effect name view which covered by navigation bar.
            effectNameView = matrixDisplayLayout.findObject(
                    By.text(EFFECT_NAME[index - 1]));
        } else {
            effectNameView = matrixDisplayLayout.findObject(
                    By.text(EFFECT_NAME[index]));
        }
        UiObject2 gridLayout = effectNameView.getParent().getParent();
        Rect gridLayoutRect = gridLayout.getVisibleBounds();
        int positionX = (gridLayoutRect.left + gridLayoutRect.right) / 2;
        int positionY = (gridLayoutRect.top + gridLayoutRect.bottom) / 2;
        if (index % 3 == 2) {
            positionY += rowWidth;
        }
        LogHelper.d(TAG, "[doOperate], click position(" + positionX + ", " + positionY + ")");
        Utils.getUiDevice().click(positionX, positionY);
    }
}
