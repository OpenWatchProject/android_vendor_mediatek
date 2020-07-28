package com.mediatek.camera.tests.v3.observer;

import com.mediatek.camera.tests.v3.arch.TestContext;
import com.mediatek.camera.tests.v3.util.Condition;
import com.mediatek.camera.tests.v3.util.Utils;

/**
 * For selftimer capture result check.
 */

public class SelfTimerPhotoSavedObserver extends SavedObserver {

    public static final int SELF_TIMER_OFF = 0;
    public static final int SELF_TIMER_2 = 1;
    public static final int SELF_TIMER_10 = 2;
    private int mObserveCount;

    public SelfTimerPhotoSavedObserver(int observeCount) {
        mObserveCount = observeCount;
    }

    @Override
    public int getObserveCount() {
        return 3;
    }

    @Override
    public String getDescription(int index) {
        switch (index) {
            case SELF_TIMER_OFF:
                return "Observe image saved result for capturing when selftimer off";
            case SELF_TIMER_2:
                return "Observe image saved result for capturing when selftimer 2s";
            case SELF_TIMER_10:
                return "Observe image saved result for capturing when selftimer 10s";
            default:
                break;
        }
        return null;
    }

    @Override
    protected void doEndObserve(int index) {

        switch (index) {
            case SELF_TIMER_OFF:
                break;
            case SELF_TIMER_2:
                Utils.waitSafely(2000);
                break;
            case SELF_TIMER_10:
                Utils.waitSafely(10000);
                break;
            default:
                break;
        }
        Utils.assertCondition(new Condition() {
            @Override
            public boolean isSatisfied() {
                updateImageFilePath();
                return mImageFilePath.size() == mObserveCount;
            }
        });
        if (mObserveCount == 1) {
            TestContext.mLatestPhotoPath = mImageFilePath.get(0);
            TestContext.mTotalCaptureAndRecordCount++;
        } else {
            TestContext.mLatestCsPhotoPath = mImageFilePath;
            TestContext.mTotalCaptureAndRecordCount += mImageFilePath.size();
        }
    }
}
