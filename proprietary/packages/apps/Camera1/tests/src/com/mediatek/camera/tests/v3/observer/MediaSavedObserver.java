package com.mediatek.camera.tests.v3.observer;

import com.mediatek.camera.tests.v3.arch.TestContext;
import com.mediatek.camera.tests.v3.util.Condition;
import com.mediatek.camera.tests.v3.util.Utils;

public class MediaSavedObserver extends SavedObserver {
    public static final int INDEX_ONE_SAVED = 0;
    public static final int INDEX_NO_SAVED = 1;
    public static final int INDEX_MULTI_SAVED = 2;

    @Override
    public int getObserveCount() {
        return 3;
    }

    @Override
    public String getDescription(int index) {
        if (index == INDEX_ONE_SAVED) {
            return "Observe if one photo or one video saved";
        } else if (index == INDEX_NO_SAVED) {
            return "Observe if no photo and video saved";
        } else if (index == INDEX_MULTI_SAVED) {
            return "Observe if multi-photos or multi-videos saved";
        } else {
            return null;
        }
    }

    @Override
    protected void doEndObserve(int index) {
        Utils.assertCondition(new Condition() {
            @Override
            public boolean isSatisfied() {
                updateImageFilePath();
                updateVideoFilePath();
                if (index == INDEX_ONE_SAVED) {
                    return (mImageFilePath.size() == 1 && mVideoFilePath.size() == 0) ||
                            (mImageFilePath.size() == 0 && mVideoFilePath.size() == 1);
                } else if (index == INDEX_NO_SAVED) {
                    return mImageFilePath.size() == 0 && mVideoFilePath.size() == 0;
                } else if (index == INDEX_MULTI_SAVED) {
                    return mImageFilePath.size() >= 1 || mVideoFilePath.size() >= 1;
                } else {
                    return true;
                }
            }
        });

        if (mImageFilePath.size() == 1) {
            TestContext.mLatestPhotoPath = mImageFilePath.get(0);
            TestContext.mLatestVideoPath = null;
            TestContext.mTotalCaptureAndRecordCount++;
        } else if (mVideoFilePath.size() == 1) {
            TestContext.mLatestPhotoPath = null;
            TestContext.mLatestVideoPath = mVideoFilePath.get(0);
            TestContext.mTotalCaptureAndRecordCount++;
        }
    }
}
