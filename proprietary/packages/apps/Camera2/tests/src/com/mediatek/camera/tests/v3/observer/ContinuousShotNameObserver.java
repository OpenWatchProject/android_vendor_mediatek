package com.mediatek.camera.tests.v3.observer;

import com.mediatek.camera.tests.v3.util.Condition;
import com.mediatek.camera.tests.v3.util.Utils;

/**
 * Observe the continuous shot image name is end with "CS".
 */

public class ContinuousShotNameObserver extends SavedObserver {


    @Override
    public int getObserveCount() {
        return 1;
    }

    @Override
    public String getDescription(int index) {
        return "Observe the continuous shot image name is end with \"CS\" ";
    }

    @Override
    protected void doEndObserve(int index) {
        Utils.assertCondition(new Condition() {
            @Override
            public boolean isSatisfied() {
                updateImageFilePath();
                return mImageFilePath.size() >= 1;
            }
        });
        for (String s : mImageFilePath) {
            Utils.assertRightNow(s.endsWith("CS.jpg"));
        }
    }
}
