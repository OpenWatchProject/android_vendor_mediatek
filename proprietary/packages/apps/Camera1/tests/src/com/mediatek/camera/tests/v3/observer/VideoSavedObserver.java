package com.mediatek.camera.tests.v3.observer;

import com.mediatek.camera.tests.v3.annotation.CoverPoint;
import com.mediatek.camera.tests.v3.arch.TestContext;
import com.mediatek.camera.tests.v3.util.Condition;
import com.mediatek.camera.tests.v3.util.Utils;

@CoverPoint(pointList = {"Check video saved status by query media db"})
public class VideoSavedObserver extends SavedObserver {
    private boolean mObserveNoVideoSaved = false;

    public VideoSavedObserver(boolean observeNoVideoSaved) {
        mObserveNoVideoSaved = observeNoVideoSaved;
    }

    public VideoSavedObserver() {
        this(false);
    }

    @Override
    protected void doEndObserve(int index) {
        Utils.assertCondition(new Condition() {
            @Override
            public boolean isSatisfied() {
                updateVideoFilePath();
                return mVideoFilePath.size() == (mObserveNoVideoSaved ? 0 : 1);
            }
        });
        if (!mObserveNoVideoSaved) {
            TestContext.mLatestVideoPath = mVideoFilePath.get(0);
            TestContext.mTotalCaptureAndRecordCount++;
        }
    }

    @Override
    public String getDescription(int index) {
        if (mObserveNoVideoSaved) {
            return "Observe if no video saved";
        } else {
            return "Observe if one video saved";
        }
    }

    @Override
    public int getObserveCount() {
        return 1;
    }
}
