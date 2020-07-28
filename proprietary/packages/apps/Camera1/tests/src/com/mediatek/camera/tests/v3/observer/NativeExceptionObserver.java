package com.mediatek.camera.tests.v3.observer;

import com.mediatek.camera.tests.v3.util.Utils;

public class NativeExceptionObserver extends AbstractLogObserver {
    private static final String LOG_TAG = "AEE_AED";
    private static final String LOG_PREFIX = "Exception Class: Native (NE)";
    private boolean mFindNE = false;

    @Override
    public int getObserveCount() {
        return 1;
    }

    @Override
    public String getDescription(int index) {
        return "Observe no native exception pop up";
    }

    @Override
    protected String[] getObservedTagList(int index) {
        return new String[]{LOG_TAG};
    }

    @Override
    protected String[] getObservedKeyList(int index) {
        return new String[]{LOG_PREFIX};
    }

    @Override
    protected void onObserveEnd(int index) {
        Utils.assertRightNow(mFindNE == false);
    }

    @Override
    protected void onObserveBegin(int index) {
        mFindNE = false;
    }

    @Override
    protected boolean isAlreadyFindTarget(int index) {
        return true;
    }

    @Override
    protected void onLogComing(int index, String line) {
        mFindNE = true;
    }
}
