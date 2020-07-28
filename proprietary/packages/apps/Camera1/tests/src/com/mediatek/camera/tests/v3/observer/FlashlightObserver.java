package com.mediatek.camera.tests.v3.observer;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.util.Utils;

/**
 * Observer used to check flashlight driver log is right.
 */
public class FlashlightObserver extends AbstractLogObserver {
    private static final LogUtil.Tag TAG = Utils.getTestTag(FlashlightObserver.class
            .getSimpleName());
    private static final String LOG_TAG_FLASHLIGHT = "flashlight_drv.cpp";
    private int mExpectedIndex = 0;

    public static final int INDEX_OFF = 0;
    public static final int INDEX_ON = 1;

    private static final String[] LOG_KEY_LIST_OFF = new String[]{
            "setOnOff() isOn = 0"
    };

    private static final String[] LOG_KEY_LIST_ON = new String[]{
            "setOnOff() isOn = 1"
    };

    @Override
    protected String[] getObservedTagList(int index) {
        return new String[]{LOG_TAG_FLASHLIGHT};
    }

    @Override
    protected String[] getObservedKeyList(int index) {
        switch (index) {
            case INDEX_OFF:
                return LOG_KEY_LIST_OFF;
            case INDEX_ON:
                return LOG_KEY_LIST_ON;
            default:
                return null;
        }
    }

    @Override
    protected boolean isAlreadyFindTarget(int index) {
        return mExpectedIndex == getObservedTagList(index).length;
    }

    @Override
    public int getObserveCount() {
        return 2;
    }


    @Override
    protected void onLogComing(int index, String line) {
        if (mExpectedIndex >= getObservedTagList(index).length) {
            return;
        }
        if (line.contains(getObservedTagList(index)[mExpectedIndex])
                && line.contains(getObservedKeyList(index)[mExpectedIndex])) {
            mExpectedIndex++;
        }
    }

    @Override
    protected void onObserveEnd(int index) {
        int length = getObservedTagList(index).length;
        Utils.assertRightNow(mExpectedIndex == length, "mExpectedIndex " + mExpectedIndex +
                ",length = " + length);
    }

    @Override
    protected void onObserveBegin(int index) {
        mExpectedIndex = 0;
    }

    @Override
    public String getDescription(int index) {
        StringBuilder sb = new StringBuilder("Observe this group tag has printed out as order, ");
        for (int i = 0; i < getObservedTagList(index).length; i++) {
            sb.append("[TAG-" + i + "] ");
            sb.append(getObservedTagList(index)[i]);
            sb.append(",[KEY-" + i + "] ");
            sb.append(getObservedKeyList(index)[i]);
            sb.append("\n");
        }
        return sb.toString();
    }

}
