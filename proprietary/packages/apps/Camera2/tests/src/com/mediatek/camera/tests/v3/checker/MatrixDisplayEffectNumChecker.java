package com.mediatek.camera.tests.v3.checker;

import android.graphics.Rect;
import android.support.test.uiautomator.By;
import android.support.test.uiautomator.Direction;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.CheckerOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

import java.util.ArrayList;
import java.util.List;

/**
 * Check the effect num is meeting to spec.
 */

public class MatrixDisplayEffectNumChecker extends CheckerOne {
    private static final LogUtil.Tag TAG = Utils.getTestTag(
            MatrixDisplayEffectNumChecker.class.getSimpleName());

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
    protected void doCheck() {
        LogHelper.e(TAG, "[doCheck]");
        List<String> effectNames = new ArrayList<>();
        UiObject2 matrixDisplayLayout = Utils.findObject(
                By.res("com.mediatek.camera:id/lomo_effect_layout"));
        Rect rect = matrixDisplayLayout.getVisibleBounds();

        int layoutWidth = rect.right - rect.left;
        int layoutHeight = rect.bottom - rect.top;
        int columnWidth = layoutWidth / 3;

        boolean hasChild;
        do {
            hasChild = false;
            List<UiObject2> nameViewLists = matrixDisplayLayout
                    .findObjects(By.res("com.mediatek.camera:id/effects_name"));
            for (int i = 0; i < nameViewLists.size(); i++) {
                UiObject2 nameView = nameViewLists.get(i);
                String effectName = nameView.getText();
                LogHelper.e(TAG, "[doCheck], effectName:" + effectName);
                if (!effectNames.contains(effectName)) {
                    effectNames.add(effectName);
                    hasChild = true;
                }
            }

            matrixDisplayLayout.scroll(Direction.LEFT, 1, columnWidth);
        } while (hasChild);

        boolean isContained = true;
        for (int i = 0; i < EFFECT_NAME.length; i++) {
            if (!effectNames.contains(EFFECT_NAME[i])) {
                isContained = false;
                LogHelper.e(TAG, "[doCheck], effectName[" + EFFECT_NAME[i]
                        + "] isn't in " + effectNames);
                break;
            }
        }
        LogHelper.e(TAG, "[doCheck], check done");
        Utils.assertRightNow(isContained && EFFECT_NAME.length == effectNames.size());
    }

    @Override
    public Page getPageBeforeCheck() {
        return null;
    }

    @Override
    public String getDescription() {
        return "check the effect name and num is meeting to spec";
    }

    @Override
    public boolean isSupported(int index) {
        return Utils.isFeatureSupported("com.mediatek.camera.at.matrix-display");
    }
}
