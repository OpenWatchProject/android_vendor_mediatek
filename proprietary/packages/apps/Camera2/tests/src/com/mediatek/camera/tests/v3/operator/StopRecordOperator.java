package com.mediatek.camera.tests.v3.operator;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.OperatorOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.arch.TestContext;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

public class StopRecordOperator extends OperatorOne {
    private static final LogUtil.Tag TAG = Utils.getTestTag(StopRecordOperator.class
            .getSimpleName());
    private boolean mNotAssertWhenNotSupport;

    public StopRecordOperator(boolean notAssertWhenNotSupport) {
        mNotAssertWhenNotSupport = notAssertWhenNotSupport;
    }

    @Override
    protected void doOperate() {
        Utils.runWithoutStaleObjectException(new Runnable() {
            @Override
            public void run() {
                UiObject2 shutterStop = Utils.findObject(
                        By.res("com.mediatek.camera:id/video_stop_shutter"), Utils.TIME_OUT_SHORT);

                if (mNotAssertWhenNotSupport && shutterStop == null) {
                    LogHelper.d(TAG, "[doOperate] not find video shutter button, return");
                    return;
                } else {
                    Utils.assertRightNow(shutterStop != null);
                }

                shutterStop.click();
            }
        });

        TestContext.mLatestStopRecordTime = System.currentTimeMillis();

        TestContext.mLatestRecordVideoDurationInSeconds = (int)
                ((TestContext.mLatestStopRecordTime - TestContext.mLatestStartRecordTime) / 1000);
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
        if (mNotAssertWhenNotSupport) {
            return "Press video shutter button to stop record, " +
                    "if current not in recording, do nothing";
        } else {
            return "Press video shutter button to stop record";
        }

    }
}
