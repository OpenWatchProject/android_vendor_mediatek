package com.mediatek.camera.tests.v3.observer;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

/**
 * Check the image properties value in the parameters is meeting to expectation or not.
 */

public class ImagePropertiesValueObserver extends AbstractLogObserver {
    private static final LogUtil.Tag TAG = Utils.getTestTag(
            ImagePropertiesValueObserver.class.getSimpleName());

    public static final int LEVEL_LOW = 0;
    public static final int LEVEL_MEDIUM = 1;
    public static final int LEVEL_HIGH = 2;

    private static final String LOG_TAG_FOR_API1 = "MtkCam/ParamsManager";
    private static final String LOG_TAG_FOR_API1_EXTRA = "CameraClient";
    private static final String KEY_BRIGHTNESS_MODE = "brightness=";
    private static final String KEY_CONTRAST_MODE = "contrast=";
    private static final String KEY_HUE_MODE = "hue=";
    private static final String KEY_SATURATION_MODE = "saturation=";
    private static final String KEY_EDGE_MODE = "edge=";
    private static final String KEY_GET_PARAMETERS = "getParameters";

    private static final String[] KEYS = {
            KEY_BRIGHTNESS_MODE,
            KEY_CONTRAST_MODE,
            KEY_HUE_MODE,
            KEY_SATURATION_MODE,
            KEY_EDGE_MODE,
            KEY_GET_PARAMETERS
    };

    private static final String[] LEVEL = {
            "low",
            "middle",
            "high",
    };
    private boolean mPass = false;
    private boolean[] mValuesPassed = {
            false,
            false,
            false,
            false,
            false,
    };

    @Override
    public boolean isSupported(int index) {
        return Utils.isFeatureSupported("com.mediatek.camera.at.image-properties");
    }

    @Override
    public int getObserveCount() {
        return LEVEL.length;
    }

    @Override
    public String getDescription(int index) {
        return "check image properties value is[" + LEVEL[index] + "]";
    }

    @Override
    protected void onLogComing(int index, String line) {
        int beginIndex;
        int endIndex;

        for (int i = 0; i < KEYS.length; i++) {
            int indexStr = line.indexOf(KEYS[i]);
            if (indexStr < 0) {
                continue;
            }
            if (KEY_EDGE_MODE.equals(KEYS[i])) {
                // Because "MtkCam/ParamsManager" log will be print in multi lines, the
                // log "edge=low", "edge=middle" or "edge=high" may be separated in two line.
                // like "edge=" in one line tail, "middle" in other line head.
                mValuesPassed[i] = true;
                continue;
            }
            if (KEY_GET_PARAMETERS.equals(KEYS[i])) {
                // sometimes there is no "edge=" log in "MtkCam/ParamsManager",
                // so get it from "getParameters" as extra.
                beginIndex = line.indexOf(KEY_EDGE_MODE) + KEY_EDGE_MODE.length();
                endIndex = line.indexOf(";", beginIndex);
                String subString = line.substring(beginIndex, endIndex);
                LogHelper.d(TAG, "[onLogComing from getParameters: ] " + KEYS[i - 1] + subString);
                mValuesPassed[i - 1] = LEVEL[index].equals(subString);
                continue;
            }
            beginIndex = line.indexOf(KEYS[i]) + KEYS[i].length();
            endIndex = line.indexOf(";", beginIndex);
            String subString = line.substring(beginIndex, endIndex);
            LogHelper.d(TAG, "[onLogComing]," + KEYS[i] + subString);
            mValuesPassed[i] = LEVEL[index].equals(subString);
        }
        mPass = true;
        for (int i = 0; i < mValuesPassed.length; i++) {
            if (!mValuesPassed[i]) {
                mPass = false;
                break;
            }
        }
    }

    @Override
    protected void onObserveEnd(int index) {
        Utils.assertRightNow(mPass);
    }

    @Override
    protected void onObserveBegin(int index) {
        mPass = false;
    }

    @Override
    protected boolean isAlreadyFindTarget(int index) {
        return mPass;
    }

    @Override
    protected String[] getObservedTagList(int index) {
        return new String[]{LOG_TAG_FOR_API1, LOG_TAG_FOR_API1,
                LOG_TAG_FOR_API1, LOG_TAG_FOR_API1, LOG_TAG_FOR_API1, LOG_TAG_FOR_API1_EXTRA};
    }

    @Override
    protected String[] getObservedKeyList(int index) {
        return KEYS;
    }
}
