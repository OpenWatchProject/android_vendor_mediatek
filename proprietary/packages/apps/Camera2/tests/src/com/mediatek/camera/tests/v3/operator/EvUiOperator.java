package com.mediatek.camera.tests.v3.operator;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

/**
 * Operator used to scroll on screen to updateEv.
 */
public class EvUiOperator extends OnSingleTapUpOperator {
    private static final LogUtil.Tag TAG = Utils.getTestTag(
            EvUiOperator.class.getSimpleName());
    private static final int SLEEP_TIME = 100;

    /**
     * Drag ev scroll bar to the bottom and then to the top.
     */
    public static final int INDEX_DOWN_UP = 0;
    /**
     * Drag ev scroll bar to the bottom.
     */
    public static final int INDEX_DOWN = 1;
    /**
     * Drag ev scroll bar to the top.
     */
    public static final int INDEX_UP = 2;


    @Override
    public int getOperatorCount() {
        return 1;
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
        switch (index) {
            case INDEX_DOWN_UP:
                return "Update EV scroll bar down and up";
            case INDEX_DOWN:
                return "Update EV scroll bar down";
            case INDEX_UP:
                return "Update EV scroll bar up";
            default:
                break;
        }
        return null;
    }

    @Override
    protected void doOperate(int index) {
        int startX = Utils.getUiDevice().getDisplayWidth() / 2;
        int endX = startX;
        int startY = Utils.getUiDevice().getDisplayWidth() / 4;
        int endY = Utils.getUiDevice().getDisplayWidth() / 4 * 3;
        int swipeSteps = 5;
        switch (index) {
            case INDEX_DOWN_UP:
                for (int i = 0; i < 10; i++) {
                    Utils.getUiDevice().swipe(startX, startY + i, endX, endY + i, swipeSteps);
                    try {
                        Thread.sleep(SLEEP_TIME);
                    } catch (InterruptedException e) {
                        LogHelper.d(TAG, "sleep error ");
                    }
                }
                for (int i = 0; i < 10; i++) {
                    Utils.getUiDevice().swipe(startX, endY + i, endX, startY, swipeSteps);
                    try {
                        Thread.sleep(SLEEP_TIME);
                    } catch (InterruptedException e) {
                        LogHelper.d(TAG, "sleep error ");
                    }
                }
                break;
            case INDEX_DOWN:
                for (int i = 0; i < 10; i++) {
                    Utils.getUiDevice().swipe(startX, startY + i, endX, endY + i, swipeSteps);
                    try {
                        Thread.sleep(SLEEP_TIME);
                    } catch (InterruptedException e) {
                        LogHelper.d(TAG, "sleep error ");
                    }
                }
                break;
            case INDEX_UP:
                for (int i = 0; i < 10; i++) {
                    Utils.getUiDevice().swipe(startX, endY + i, endX, startY, swipeSteps);
                    try {
                        Thread.sleep(SLEEP_TIME);
                    } catch (InterruptedException e) {
                        LogHelper.d(TAG, "sleep error ");
                    }
                }
                break;
            default:
                break;
        }


    }

}
