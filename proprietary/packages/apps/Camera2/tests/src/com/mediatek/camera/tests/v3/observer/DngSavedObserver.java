package com.mediatek.camera.tests.v3.observer;

import com.mediatek.camera.tests.v3.annotation.CoverPoint;
import com.mediatek.camera.tests.v3.arch.TestContext;
import com.mediatek.camera.tests.v3.util.Condition;
import com.mediatek.camera.tests.v3.util.Utils;

@CoverPoint(pointList = {"Check jpeg and dng saved status by query media db"})
public class DngSavedObserver extends SavedObserver {
    public static final int INDEX_ON = 0;
    public static final int INDEX_OFF = 1;

    @Override
    protected void doEndObserve(int index) {
        if (index == INDEX_ON) {
            Utils.assertCondition(new Condition() {
                @Override
                public boolean isSatisfied() {
                    updateImageFilePath();
                    return mImageFilePath.size() == 2;
                }
            });
            TestContext.mLatestDngPath = null;
            TestContext.mLatestPhotoPath = null;
            for (String filePath : mImageFilePath) {
                if (filePath.endsWith(".dng")) {
                    TestContext.mLatestDngPath = filePath;
                    TestContext.mTotalCaptureAndRecordCount++;
                } else if (filePath.endsWith(".jpg")) {
                    TestContext.mLatestPhotoPath = filePath;
                    TestContext.mTotalCaptureAndRecordCount++;
                }
            }
            Utils.assertRightNow(TestContext.mLatestDngPath != null);
            Utils.assertRightNow(TestContext.mLatestPhotoPath != null);
            return;
        }

        if (index == INDEX_OFF) {
            Utils.assertCondition(new Condition() {
                @Override
                public boolean isSatisfied() {
                    updateImageFilePath();
                    return mImageFilePath.size() == 1;
                }
            });
            Utils.assertRightNow(mImageFilePath.get(0).endsWith(".jpg"));
            TestContext.mLatestPhotoPath = mImageFilePath.get(0);
            TestContext.mTotalCaptureAndRecordCount++;
            return;
        }
    }

    @Override
    public String getDescription(int index) {
        switch (index) {
            case INDEX_ON:
                return "Observe if one jpg and one dng saved";
            case INDEX_OFF:
                return "Observe if one jpg and no dng saved";
        }
        return null;
    }

    @Override
    public boolean isSupported(int index) {
        if (index == INDEX_ON) {
            return Utils.isFeatureSupported("com.mediatek.camera.at.dng.on");
        } else {
            return true;
        }
    }

    @Override
    public int getObserveCount() {
        return 2;
    }
}
