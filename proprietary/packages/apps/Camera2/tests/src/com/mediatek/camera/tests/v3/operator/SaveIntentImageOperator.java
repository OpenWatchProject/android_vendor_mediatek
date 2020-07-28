package com.mediatek.camera.tests.v3.operator;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.tests.v3.arch.Operator;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

/**
 * For save image for the intent image capture.
 */

public class SaveIntentImageOperator extends Operator {
    public static final int INDEX_CANCEL = 0;
    public static final int INDEX_SAVE = 1;

    @Override
    public int getOperatorCount() {
        return 2;
    }

    @Override
    public Page getPageBeforeOperate(int index) {
        return null;
    }

    @Override
    public Page getPageAfterOperate(int index) {
        return null;
    }

    @Override
    public String getDescription(int index) {
        return "save image in the intent image capture page.";
    }

    @Override
    protected void doOperate(int index) {
        switch (index) {
            case INDEX_CANCEL:
                UiObject2 cancelBtn = Utils.findObject(
                        By.res("com.mediatek.camera:id/btn_retake"), 5000);
                Utils.assertRightNow(cancelBtn != null);
                cancelBtn.click();
                Utils.waitSafely(500);
                break;
            case INDEX_SAVE:
                UiObject2 saveBtn = Utils.findObject(
                        By.res("com.mediatek.camera:id/btn_save"), 5000);
                Utils.assertRightNow(saveBtn != null);
                saveBtn.click();
                Utils.waitSafely(500);
                break;
        }
    }
}
