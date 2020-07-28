package com.mediatek.camera.tests.v3.observer;

import com.mediatek.camera.tests.v3.annotation.CoverPoint;
import com.mediatek.camera.tests.v3.arch.TestContext;
import com.mediatek.camera.tests.v3.util.Condition;
import com.mediatek.camera.tests.v3.util.Utils;

@CoverPoint(pointList = {"Check photo saved status by query media db"})
public class PhotoSavedObserver extends SavedObserver {
    private int mObserveCount;

    public PhotoSavedObserver(int observeCount) {
        mObserveCount = observeCount;
    }

    @Override
    protected void doEndObserve(int index) {
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

    @Override
    public String getDescription(int index) {
        return "Observe if " + mObserveCount + " photo saved";
    }

    @Override
    public int getObserveCount() {
        return 1;
    }
}
