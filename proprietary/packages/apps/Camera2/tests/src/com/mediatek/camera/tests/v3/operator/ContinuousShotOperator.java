package com.mediatek.camera.tests.v3.operator;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.OperatorOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Condition;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

public class ContinuousShotOperator extends OperatorOne {
    private static final LogUtil.Tag TAG = Utils.getTestTag(ContinuousShotOperator.class
            .getSimpleName());
    private static final int DURATION_MILLISECONDS = 5000;

    private int mDuration = DURATION_MILLISECONDS;
    private boolean mNeedToCheckDialog = true;

    /**
     * Default constructor.
     */
    public ContinuousShotOperator() {
    }

    /**
     * Constructor attach duration to indicate long press time,
     * and whether check dialog after ending continuous shot.
     *
     * @param duration          The duration to long press.
     * @param needToCheckDialog Need to check dialog after ending continuous shot.
     */
    public ContinuousShotOperator(int duration, boolean needToCheckDialog) {
        mDuration = duration;
        mNeedToCheckDialog = needToCheckDialog;
    }

    @Override
    protected void doOperate() {
        LogHelper.d(TAG, "[doOperate]");
        UiObject2 shutter = Utils.findObject(Utils.getShutterSelector());

        LogHelper.d(TAG, "[doOperate] click down until shutter enable or time out");
        Utils.longPressUntil(shutter, new Condition() {
            @Override
            public boolean isSatisfied() {
                return false;
            }
        }, mDuration);

        if (mNeedToCheckDialog) {
            LogHelper.d(TAG, "[doOperate] wait saving");
            Utils.assertNoObject(By.res("com.mediatek.camera:id/dialog_progress"),
                    Utils.TIME_OUT_LONG_LONG);
        }
    }

    @Override
    public Page getPageBeforeOperate() {
        return Page.PREVIEW;
    }

    @Override
    public Page getPageAfterOperate() {
        return Page.PREVIEW;
    }

    @Override
    public String getDescription() {
        if (mNeedToCheckDialog) {
            return "Long press shutter button for "
                    + mDuration + "ms to do continuous shot, wait until saving completed";
        } else {
            return "Long press shutter button for "
                    + mDuration + "ms to do continuous shot, not wait saving completed";
        }
    }

    @Override
    public boolean isSupported(int index) {
        return Utils.isFeatureSupported("com.mediatek.camera.at.cs");
    }
}
