package com.mediatek.camera.tests.v3.observer;

import com.mediatek.camera.tests.v3.util.Utils;

public abstract class LogListPrintObserver extends AbstractLogObserver {

    private int mExpectedIndex = 0;

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
    protected void onObserveBegin(int index) {
        mExpectedIndex = 0;
    }

    @Override
    protected void onObserveEnd(int index) {
        if (mExpectedIndex != getObservedTagList(index).length) {
            Utils.assertRightNow(false,
                    "Not find log with TAG [" + getObservedTagList(index)[mExpectedIndex] +
                            "], KEY [" + getObservedKeyList(index)[mExpectedIndex] + "]");
        }
    }

    @Override
    protected boolean isAlreadyFindTarget(int index) {
        return mExpectedIndex == getObservedTagList(index).length;
    }

    @Override
    public String getDescription(int index) {
        StringBuilder sb = new StringBuilder("Observe this group tag has printed out as order, ");
        for (int i = 0; i < getObservedTagList(index).length; i++) {
            sb.append("[TAG-" + i + "] ");
            sb.append(getObservedTagList(index)[i]);
            sb.append("[KEY-" + i + "] ");
            sb.append(getObservedKeyList(index)[i]);
        }
        return sb.toString();
    }
}
