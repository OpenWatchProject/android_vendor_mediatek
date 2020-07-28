package com.mediatek.camera.tests.v3.operator;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.Operator;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

public class ZoomOperator extends Operator {
    private static final LogUtil.Tag TAG = Utils.getTestTag(ZoomOperator.class.getSimpleName());
    public static final int INDEX_ZOOM_IN = 0;
    public static final int INDEX_ZOOM_OUT = 1;

    private static final float ZOOM_PERCENT_LARGE = 0.5f;
    private static final float ZOOM_PERCENT_SMALL = 0.1f;
    private static final long ZOOM_TIME_MILLS = 1000;

    private long mZoomTimeMills = ZOOM_TIME_MILLS;

    public ZoomOperator() {
        mZoomTimeMills = ZOOM_TIME_MILLS;
    }

    public ZoomOperator(long zoomTimeMills) {
        mZoomTimeMills = zoomTimeMills;
    }

    @Override
    public int getOperatorCount() {
        return 2;
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
            case INDEX_ZOOM_IN:
                return "Zoom in preview";
            case INDEX_ZOOM_OUT:
                return "Zoom out preview";
            default:
                break;
        }
        return null;
    }

    @Override
    protected void doOperate(int index) {
        UiObject2 preview = Utils.getUiDevice().findObject(By.res("com.mediatek" +
                ".camera:id/preview_surface"));
        Utils.assertRightNow(preview != null);

        switch (index) {
            case INDEX_ZOOM_IN:
                LogHelper.d(TAG, "[doOperate] zoom 1");
                Utils.zoom(preview, ZOOM_PERCENT_SMALL, ZOOM_PERCENT_LARGE, mZoomTimeMills);
                break;
            case INDEX_ZOOM_OUT:
                LogHelper.d(TAG, "[doOperate] zoom 2");
                Utils.zoom(preview, ZOOM_PERCENT_LARGE, ZOOM_PERCENT_SMALL, mZoomTimeMills);
                break;
            default:
                LogHelper.d(TAG, "[doOperate] zoom 3");
                break;
        }
    }
}
