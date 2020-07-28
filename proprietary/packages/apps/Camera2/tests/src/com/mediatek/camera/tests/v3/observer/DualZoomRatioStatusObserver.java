package com.mediatek.camera.tests.v3.observer;

import android.support.test.uiautomator.By;

import com.mediatek.camera.tests.v3.annotation.CoverPoint;
import com.mediatek.camera.tests.v3.arch.Observer;
import com.mediatek.camera.tests.v3.util.Utils;

/**
 * Zoom ratio observer(hide or show).
 */
@CoverPoint(pointList = {"Check dual camera zoom status"})
public class DualZoomRatioStatusObserver extends Observer {
    private boolean mIsShow = false;

    /**
     * Constructor.
     *
     * @param isShow show or not show
     */
    public DualZoomRatioStatusObserver(boolean isShow) {
        mIsShow = isShow;
    }

    /**
     * Constructor.
     */
    public DualZoomRatioStatusObserver() {
        this(false);
    }

    @Override
    protected void doEndObserve(int index) {
        if (mIsShow) {
            Utils.assertObject(By.res("com.mediatek.camera:id/ext_zoom_text_view"));
        } else {
            Utils.assertNoObject(By.res("com.mediatek.camera:id/ext_zoom_text_view"));
        }
    }

    @Override
    protected void doBeginObserve(int index) {
        Utils.assertObject(By.res("com.mediatek.camera:id/ext_zoom_text_view"));
    }

    @Override
    public String getDescription(int index) {
        if (mIsShow) {
            return "Observe dual camera zoom ratio is showing";
        } else {
            return "Observe dual camera zoom ratio is hiding";
        }
    }

    @Override
    public int getObserveCount() {
        return 1;
    }

    @Override
    public boolean isSupported(int index) {
        if (mIsShow) {
            return Utils.isFeatureSupported("com.mediatek.camera.at.dual-zoom");
        } else {
            return true;
        }
    }
}
