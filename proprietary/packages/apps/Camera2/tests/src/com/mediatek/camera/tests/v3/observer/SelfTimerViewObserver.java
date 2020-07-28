package com.mediatek.camera.tests.v3.observer;


import android.support.test.uiautomator.By;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

/**
 * When selftimer is running check there is timer view show.
 */

public class SelfTimerViewObserver extends BackgroundObserver {
    public static final int TIMER_OFF_VIEW = 0;
    public static final int TIMER_2_VIEW = 1;
    public static final int TIMER_10_VIEW = 2;
    public static final String SELF_TIMER_VIEW_RES_ID = "com.mediatek.camera:id/self_timer_num";

    private static final LogUtil.Tag TAG = Utils.getTestTag(
            SelfTimerViewObserver.class.getSimpleName());
    private boolean mHasAppeared;

    @Override
    protected void doEndObserve(int index) {
        super.doEndObserve(index);
        switch (index) {
            case TIMER_OFF_VIEW:
                Utils.assertRightNow(!mHasAppeared);
                break;
            case TIMER_2_VIEW:
            case TIMER_10_VIEW:
                Utils.assertRightNow(mHasAppeared);
                break;
            default:
                break;
        }
        Utils.waitNoObject(By.res(SELF_TIMER_VIEW_RES_ID), Utils.TIME_OUT_SHORT);
    }

    @Override
    protected void doObserveInBackground(int index) {
        switch (index) {
            case TIMER_OFF_VIEW:
                //no view
                mHasAppeared = false;
                break;
            case TIMER_2_VIEW:
            case TIMER_10_VIEW:
                //show view
                mHasAppeared = Utils.waitObject(
                        By.res(SELF_TIMER_VIEW_RES_ID),
                        Utils.TIME_OUT_SHORT);
                if (mHasAppeared == true) {
                    LogHelper.d(TAG, "[doObserveInBackground] <"
                            + "timer animation" + "> show");
                } else {
                    LogHelper.d(TAG, "[doObserveInBackground] <"
                            + "timer animation" + "> not show");
                }
                break;
            default:
                break;
        }
    }

    @Override
    public int getObserveCount() {
        return 3;
    }

    @Override
    public String getDescription(int index) {
        switch (index) {
            case TIMER_OFF_VIEW:
                return "Observe selftimer animation not shown";
            case TIMER_2_VIEW:
            case TIMER_10_VIEW:
                return "Observe selftimer animation shown";

        }
        return null;
    }
}
