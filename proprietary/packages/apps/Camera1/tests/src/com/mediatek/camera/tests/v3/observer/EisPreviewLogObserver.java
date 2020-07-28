package com.mediatek.camera.tests.v3.observer;

import com.mediatek.camera.tests.v3.util.Utils;

public class EisPreviewLogObserver extends AbstractLogObserver {
    public static final int INDEX_ON = 0;
    public static final int INDEX_OFF = 1;

    private boolean mEisHalPrintOut;

    private static final String[] TAGS = new String[]{
            "EisHal"
    };

    @Override
    public boolean isSupported(int index) {
        if (index == INDEX_ON) {
            return Utils.isFeatureSupported("com.mediatek.camera.at.eis");
        } else {
            return true;
        }
    }

    @Override
    public int getObserveCount() {
        return 2;
    }

    @Override
    public String getDescription(int index) {
        switch (index) {
            case INDEX_ON:
                return "Observe EisHal log print out";
            case INDEX_OFF:
                return "Observe no EisHal log print out";
            default:
                return null;
        }
    }

    @Override
    protected void onLogComing(int index, String line) {
        if (line.contains("EisHal")) {
            mEisHalPrintOut = true;
        }
    }

    @Override
    protected void onObserveEnd(int index) {
        switch (index) {
            case INDEX_ON:
                Utils.assertRightNow(mEisHalPrintOut);
                break;
            case INDEX_OFF:
                Utils.assertRightNow(!mEisHalPrintOut);
                break;
        }
    }

    @Override
    protected void onObserveBegin(int index) {
        mEisHalPrintOut = false;
    }

    @Override
    protected boolean isAlreadyFindTarget(int index) {
        switch (index) {
            case INDEX_ON:
                return mEisHalPrintOut;
            case INDEX_OFF:
                return true;
            default:
                return true;
        }
    }

    @Override
    protected String[] getObservedTagList(int index) {
        return TAGS;
    }

    @Override
    protected String[] getObservedKeyList(int index) {
        return null;
    }
}