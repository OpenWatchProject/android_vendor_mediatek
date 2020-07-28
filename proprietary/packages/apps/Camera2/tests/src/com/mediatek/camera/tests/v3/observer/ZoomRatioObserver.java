package com.mediatek.camera.tests.v3.observer;

import com.mediatek.camera.common.device.CameraDeviceManagerFactory;
import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.tests.v3.util.Utils;

public class ZoomRatioObserver extends LogKeyValueObserver {
    public static final int INDEX_ZOOM_IN_LEVEL_MAX = 0;
    public static final int INDEX_ZOOM_OUT_LEVEL_MIN = 1;

    private static final String LOG_TAG_API1 = "CamAp_ZoomParameterCon";
    private static final String ZOOM_RATIO_LOG_PREFIX_API1 = "mZoomLevel = ";
    private static final int ZOOM_RATIO_NUM_LENGTH_MAX_API1 = "10".length();
    private static final int ZOOM_RATIO_NUM_LENGTH_MIN_API1 = "1".length();

    private static final String LOG_TAG_API2 = "CamAp_ZoomCaptureReque";
    private static final String ZOOM_RATIO_LOG_PREFIX_API2 = "mCurZoomRatio = ";
    private static final int ZOOM_RATIO_NUM_LENGTH_API2 = "1.0".length();

    private boolean mAppeared = false;

    @Override
    protected String getObservedTag(int index) {
        if (CameraApiHelper.getCameraApiType(null) == CameraDeviceManagerFactory.CameraApi.API1) {
            return LOG_TAG_API1;
        } else {
            return LOG_TAG_API2;
        }
    }

    @Override
    protected String getObservedKey(int index) {
        if (CameraApiHelper.getCameraApiType(null) == CameraDeviceManagerFactory.CameraApi.API1) {
            return ZOOM_RATIO_LOG_PREFIX_API1;
        } else {
            return ZOOM_RATIO_LOG_PREFIX_API2;
        }
    }

    @Override
    protected ValueType getValueType(int index) {
        if (CameraApiHelper.getCameraApiType(null) == CameraDeviceManagerFactory.CameraApi.API1) {
            return ValueType.INTEGER;
        } else {
            return ValueType.FLOAT;
        }
    }

    @Override
    protected int getMinValueStringLength(int index) {
        if (CameraApiHelper.getCameraApiType(null) == CameraDeviceManagerFactory.CameraApi.API1) {
            return ZOOM_RATIO_NUM_LENGTH_MIN_API1;
        } else {
            return ZOOM_RATIO_NUM_LENGTH_API2;
        }
    }

    @Override
    protected int getMaxValueStringLength(int index) {
        if (CameraApiHelper.getCameraApiType(null) == CameraDeviceManagerFactory.CameraApi.API1) {
            return ZOOM_RATIO_NUM_LENGTH_MAX_API1;
        } else {
            return ZOOM_RATIO_NUM_LENGTH_API2;
        }
    }

    @Override
    protected void onObserveBegin(int index) {
        mAppeared = false;
    }

    @Override
    protected void onValueComing(int index, Object value) {
        if (CameraApiHelper.getCameraApiType(null) == CameraDeviceManagerFactory.CameraApi.API1) {
            int targetLevel = index == INDEX_ZOOM_IN_LEVEL_MAX ? 10 : 0;
            if (((Integer) value) == targetLevel) {
                mAppeared = true;
            } else {
                mAppeared = false;
            }
        } else {
            float targetLevel = index == INDEX_ZOOM_IN_LEVEL_MAX ? 4.0f : 1.0f;
            if (((Float) value) == targetLevel) {
                mAppeared = true;
            } else {
                mAppeared = false;
            }
        }
    }

    @Override
    protected void onObserveEnd(int index) {
        if (CameraApiHelper.getCameraApiType(null) == CameraDeviceManagerFactory.CameraApi.API1) {
            int targetLevel = index == INDEX_ZOOM_IN_LEVEL_MAX ? 10 : 0;
            Utils.assertRightNow(mAppeared, "[" + getObservedKey(index) + targetLevel + "] not " +
                    "print out");
        } else {
            float targetLevel = index == INDEX_ZOOM_IN_LEVEL_MAX ? 4.0f : 1.0f;
            Utils.assertRightNow(mAppeared, "[" + getObservedKey(index) + targetLevel + "] not " +
                    "print out");
        }
    }

    @Override
    protected boolean isAlreadyFindTarget(int index) {
        return mAppeared;
    }

    @Override
    public boolean isSupported(int index) {
        return !Utils.isFeatureSupported("com.mediatek.camera.at.dual-zoom");
    }

    @Override
    public int getObserveCount() {
        return 2;
    }

    @Override
    public String getDescription(int index) {
        if (CameraApiHelper.getCameraApiType(null) == CameraDeviceManagerFactory.CameraApi.API1) {
            int targetLevel = index == INDEX_ZOOM_IN_LEVEL_MAX ? 10 : 0;
            return "Observe has zoomed to ratio " + targetLevel;
        } else {
            float targetLevel = index == INDEX_ZOOM_IN_LEVEL_MAX ? 4.0f : 1.0f;
            return "Observe has zoomed to ratio " + targetLevel;
        }
    }
}
