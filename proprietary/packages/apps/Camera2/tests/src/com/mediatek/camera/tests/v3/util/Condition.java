package com.mediatek.camera.tests.v3.util;

import android.os.SystemClock;
import android.support.test.uiautomator.StaleObjectException;
import android.support.test.uiautomator.UiDevice;

import com.mediatek.camera.common.debug.LogUtil;

import junit.framework.Assert;

public abstract class Condition {
    private static final LogUtil.Tag TAG = Utils.getTestTag(Condition.class.getSimpleName());

    private UiDevice mUiDevice;

    public abstract boolean isSatisfied();

    public Condition() {
        mUiDevice = Utils.getUiDevice();
    }

    public boolean waitMe(int timeout) {
        long timeOutPoint = SystemClock.elapsedRealtime() + timeout;
        while (SystemClock.elapsedRealtime() <= timeOutPoint) {
            try {
                if (isSatisfied()) {
                    return true;
                } else {
                    waitSafely(20);
                }
            } catch (StaleObjectException e) {
                LogHelper.e(TAG, "[waitMe] StaleObjectException pop up", e);
            }
        }
        return false;
    }

    public void assertMe(int timeout, String failMessage) {
        if (waitMe(timeout) == true) {
            return;
        }

        Assert.assertTrue(failMessage + Utils.dump(), false);
    }

    public void assertMe(int timeout) {
        assertMe(timeout, null);
    }


    private void waitSafely(long millseconds) {
        mUiDevice.waitForIdle();
        try {
            synchronized (mUiDevice) {
                mUiDevice.wait(millseconds);
            }
        } catch (InterruptedException e) {
        }
    }
}
