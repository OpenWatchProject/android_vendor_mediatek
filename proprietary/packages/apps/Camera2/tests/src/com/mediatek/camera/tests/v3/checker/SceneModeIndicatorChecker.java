package com.mediatek.camera.tests.v3.checker;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.Checker;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Condition;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

/**
 * Check scene mode indicator.
 */

public class SceneModeIndicatorChecker extends Checker {
    private static final LogUtil.Tag TAG = Utils.getTestTag(SceneModeIndicatorChecker.class
            .getSimpleName());

    private static final String[] OPTIONS = {
            "off",
            "auto",
            "night",
            "sunset",
            "party",
            "portrait",
            "landscape",
            "night-portrait",
            "theatre",
            "beach",
            "snow",
            "steadyphoto",
            "fireworks",
            "sports",
            "candlelight"
    };

    @Override
    public int getCheckCount() {
        return OPTIONS.length;
    }

    @Override
    public Page getPageBeforeCheck(int index) {
        return Page.PREVIEW;
    }

    @Override
    public String getDescription(int index) {
        if (index == 0 || index == 1) {
            return null;
        }
        return "check indicator[" + OPTIONS[index] + "]";
    }

    @Override
    protected void doCheck(int index) {
        if (index == 0 || index == 1) {
            return;
        }

        Utils.assertCondition(new Condition() {
            @Override
            public boolean isSatisfied() {
                UiObject2 indicator = Utils.findObject(
                        By.res("com.mediatek.camera:id/scene_mode_indicator"));
                if (indicator == null) {
                    LogHelper.d(TAG, "[doCheck], Can not find scene_mode_indicator");
                    return false;
                }
                String description = indicator.getContentDescription();
                LogHelper.d(TAG, "[doCheck], description:" + description);
                return OPTIONS[index].equals(description);
            }
        });
    }
}
