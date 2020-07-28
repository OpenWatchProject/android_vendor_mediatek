package com.mediatek.camera.tests.v3.operator;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.OperatorOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.arch.TestContext;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

public class SlideInGalleryOperator extends OperatorOne {
    private static final LogUtil.Tag TAG = Utils.getTestTag(SlideInGalleryOperator.class
            .getSimpleName());

    @Override
    protected void doOperate() {
        int width = Utils.getUiDevice().getDisplayWidth();
        int height = Utils.getUiDevice().getDisplayHeight();
        int startX = width * 9 / 10;
        int endX = width / 10;
        int startY = height / 2;
        int endY = startY;
        for (int i = 0; i < TestContext.mTotalCaptureAndRecordCount - 1; i++) {
            LogHelper.d(TAG, "[doOperate] mTotalCaptureAndRecordCount = "
                    + TestContext.mTotalCaptureAndRecordCount + ", slide times = " + (i + 1));
            Utils.getUiDevice().swipe(startX, startY, endX, endY,
                    Utils.getSwipeStepsByDuration(100));
            // wait gallery animation done, slide to a new image
            Utils.waitSafely(500);
        }

    }

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
        return "Slide in gallery to view pictures/videos one by one";
    }
}
