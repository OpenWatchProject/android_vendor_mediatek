package com.mediatek.camera.tests.v3.observer;

import com.mediatek.camera.tests.v3.util.Utils;

public abstract class LogPrintObserver extends AbstractLogObserver {
    protected boolean mHasPrintOut = false;

    @Override
    protected void onLogComing(int index, String line) {
        mHasPrintOut = true;
    }

    @Override
    protected void onObserveEnd(int index) {
        Utils.assertRightNow(mHasPrintOut, "Not find log with TAG [" + getObserveLogTag(index) +
                "], KEY [" + getObserveLogKey(index) + "]");
    }

    @Override
    protected void onObserveBegin(int index) {
        mHasPrintOut = false;
    }

    @Override
    protected boolean isAlreadyFindTarget(int index) {
        return mHasPrintOut;
    }

    @Override
    protected String[] getObservedTagList(int index) {
        return new String[]{getObserveLogTag(index)};
    }

    @Override
    protected String[] getObservedKeyList(int index) {
        return new String[]{getObserveLogKey(index)};
    }

    @Override
    public String getDescription(int index) {
        if (getObserveLogKey(index) == null) {
            return "Observe log with KEY [" + getObserveLogTag(index) + "] has printed out";
        } else {
            return "Observe log with TAG [" + getObserveLogTag(index)
                    + "], KEY [" + getObserveLogKey(index) + "] has printed out";
        }
    }

    protected abstract String getObserveLogTag(int index);

    protected abstract String getObserveLogKey(int index);

}
