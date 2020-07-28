package com.mediatek.camera.tests.v3.observer;

import com.mediatek.camera.tests.v3.util.Utils;

public class DualZoomRatioObserver extends LogKeyValueObserver {
    private static final String LOG_TAG = "CamAp_DualZoomParamete";
    private static final String ZOOM_RATIO_LOG_PREFIX = "mZoomLevel = ";
    private static final int ZOOM_RATIO_NUM_LENGTH_MAX = "200".length();
    private static final int ZOOM_RATIO_NUM_LENGTH_MIN = "1".length();
    private boolean mIsTargetZoomIn = false;
    private int mCurrentZoomLevel = -1;
    private int mFirstZoomLevel = -1;
    private boolean mAppeared = false;

    public DualZoomRatioObserver(boolean isZoomIn) {
        mIsTargetZoomIn = isZoomIn;
    }

    @Override
    protected void onValueComing(int index, Object value) {
        if (mFirstZoomLevel == -1 && mCurrentZoomLevel == -1) {
            mFirstZoomLevel = (Integer) value;
        }
        mCurrentZoomLevel = (Integer) value;
    }

    @Override
    protected String getObservedTag(int index) {
        return LOG_TAG;
    }

    @Override
    protected String getObservedKey(int index) {
        return ZOOM_RATIO_LOG_PREFIX;
    }

    @Override
    protected ValueType getValueType(int index) {
        return ValueType.INTEGER;
    }

    @Override
    protected int getMinValueStringLength(int index) {
        return ZOOM_RATIO_NUM_LENGTH_MIN;
    }

    @Override
    protected int getMaxValueStringLength(int index) {
        return ZOOM_RATIO_NUM_LENGTH_MAX;
    }

    @Override
    protected void onObserveBegin(int index) {
        mAppeared = false;
    }

    @Override
    protected void onObserveEnd(int index) {
        if (mIsTargetZoomIn) {
            Utils.assertRightNow(mCurrentZoomLevel > mFirstZoomLevel, "Zoom In succussfully!");
        } else {
            Utils.assertRightNow(mCurrentZoomLevel < mFirstZoomLevel, "Zoom Out succussfully!");
        }
    }

    @Override
    protected boolean isAlreadyFindTarget(int index) {
        return mAppeared;
    }

    @Override
    public boolean isSupported(int index) {
        return true;
        //return !Utils.isFeatureSupported("com.mediatek.camera.at.dual-zoom");
    }

    @Override
    public int getObserveCount() {
        return 1;
    }

    @Override
    public String getDescription(int index) {
        if (mIsTargetZoomIn) {
            return "Observe has Zoom In succussfully!";
        } else {
            return "Observe has Zoom Out succussfully!";
        }
    }
}
