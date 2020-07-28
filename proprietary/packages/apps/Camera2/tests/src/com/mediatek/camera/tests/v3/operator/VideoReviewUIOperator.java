package com.mediatek.camera.tests.v3.operator;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.Operator;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

/**
 * operator the intent video for review.
 */

public class VideoReviewUIOperator extends Operator {
    private static final LogUtil.Tag TAG = Utils.getTestTag(VideoReviewUIOperator.class
            .getSimpleName());
    public static final int REVIEW_OPERATION_PLAY = 0;
    public static final int REVIEW_OPERATION_SAVE = 1;

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
        return "Play intent video and retake";
    }

    @Override
    protected void doOperate(int index) {
        switch (index) {
            case REVIEW_OPERATION_PLAY:
                UiObject2 play = Utils.findObject(
                        By.res("com.mediatek.camera:id/btn_play"), Utils.TIME_OUT_SHORT);
                Utils.assertRightNow(play != null);
                play.click();
                Utils.waitSafely(Utils.TIME_OUT_NORMAL);
                UiObject2 button = Utils.findObject(
                        By.res("com.mediatek.camera:id/btn_retake"), Utils.TIME_OUT_SHORT);
                Utils.assertRightNow(button != null);
                button.click();
                break;
            case REVIEW_OPERATION_SAVE:
                UiObject2 ok = Utils.findObject(
                        By.res("com.mediatek.camera:id/btn_save"), Utils.TIME_OUT_SHORT);
                Utils.assertRightNow(ok != null, "Can not find save button");
                ok.click();
                break;
            default:
                break;
        }
    }
}
