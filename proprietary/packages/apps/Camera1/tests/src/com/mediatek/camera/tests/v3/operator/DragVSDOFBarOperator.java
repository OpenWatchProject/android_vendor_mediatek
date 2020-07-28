package com.mediatek.camera.tests.v3.operator;

import android.graphics.Rect;
import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.tests.v3.arch.Operator;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

public class DragVSDOFBarOperator extends Operator {
    public static final int LEFT_VALUE = 0;
    public static final int MIDDLE_VALUE = 1;
    public static final int RIGHT_VALUE = 2;

    @Override
    public boolean isSupported(int index) {
        return Utils.isFeatureSupported("com.mediatek.camera.at.vsdof");
    }

    @Override
    public int getOperatorCount() {
        return 3;
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
        return "Drag VSDOF Bar to many value";
    }

    @Override
    protected void doOperate(int index) {
        UiObject2 vsdofBar =
                Utils.findObject(By.res("com.mediatek.camera:id/sdof_bar")
                        .enabled(true));
        Rect rect = vsdofBar.getVisibleBounds();
        int y = (rect.top + rect.bottom) / 2;
        switch (index) {
            case LEFT_VALUE:
                Utils.getUiDevice().click(getValueOfX(0.01, rect), y);
                break;
            case MIDDLE_VALUE:
                Utils.getUiDevice().click(getValueOfX(0.5, rect), y);
                break;
            case RIGHT_VALUE:
                Utils.getUiDevice().click(getValueOfX(0.99, rect), y);
                break;
        }
    }

    private int getValueOfX(double persent, Rect rect) {
        return rect.left + 5 + (int) ((rect.right - rect.left - 10) * persent);
    }
}
