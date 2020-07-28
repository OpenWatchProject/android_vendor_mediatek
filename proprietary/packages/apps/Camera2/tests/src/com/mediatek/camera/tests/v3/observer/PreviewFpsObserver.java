package com.mediatek.camera.tests.v3.observer;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.util.Utils;

public class PreviewFpsObserver extends LogKeyValueObserver {
    private static final LogUtil.Tag TAG =
            Utils.getTestTag(PreviewFpsObserver.class.getSimpleName());
    private static final String LOG_TAG = "MtkCam/DisplayClient";
    private static final String FPS_LOG_PREFIX = "[DisplayClient::enquePrvOps] fps: ";
    private static final int FPS_NUM_LENGTH_MAX = "29.932558".length();
    private static final int FPS_NUM_LENGTH_MIN = "9.932558".length();

    private int mFpsPassTarget = 29;
    private boolean mPass = true;

    public PreviewFpsObserver(int fpsPassTarget) {
        mFpsPassTarget = fpsPassTarget;
    }

    @Override
    protected String getObservedTag(int index) {
        return LOG_TAG;
    }

    @Override
    protected String getObservedKey(int index) {
        return FPS_LOG_PREFIX;
    }

    @Override
    protected ValueType getValueType(int index) {
        return ValueType.FLOAT;
    }

    @Override
    protected int getMinValueStringLength(int index) {
        return FPS_NUM_LENGTH_MIN;
    }

    @Override
    protected int getMaxValueStringLength(int index) {
        return FPS_NUM_LENGTH_MAX;
    }

    @Override
    public int getObserveCount() {
        return 1;
    }

    @Override
    public String getDescription(int index) {
        return "Observe preview fps >= " + mFpsPassTarget;
    }

    @Override
    protected void onObserveEnd(int index) {
        Utils.assertRightNow(mPass);
    }

    @Override
    protected void onObserveBegin(int index) {
        mPass = true;
    }

    @Override
    protected boolean isAlreadyFindTarget(int index) {
        return true;
    }

    @Override
    protected void onValueComing(int index, Object value) {
        mPass = mPass && (Float) (value) >= mFpsPassTarget;
    }
}
