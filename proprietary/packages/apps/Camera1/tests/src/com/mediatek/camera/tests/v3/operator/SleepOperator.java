package com.mediatek.camera.tests.v3.operator;

import com.mediatek.camera.tests.v3.arch.OperatorOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

public class SleepOperator extends OperatorOne {
    private int mSleepDuraionInSeconds;

    public SleepOperator(int sleepDurationInSeconds) {
        mSleepDuraionInSeconds = sleepDurationInSeconds;
    }

    @Override
    protected void doOperate() {
        Utils.waitSafely(mSleepDuraionInSeconds * 1000);
    }

    @Override
    public Page getPageBeforeOperate() {
        return null;
    }

    @Override
    public Page getPageAfterOperate() {
        return null;
    }

    @Override
    public String getDescription() {
        return "Wait for " + mSleepDuraionInSeconds + " s";
    }
}
