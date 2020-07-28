package com.mediatek.camera.tests.v3.operator;

import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.OperatorOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.arch.TestContext;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

public class StartRecordOperator extends OperatorOne {
    private static final LogUtil.Tag TAG = Utils.getTestTag(StartRecordOperator.class
            .getSimpleName());
    private boolean mNotAssertWhenNotSupport;

    public StartRecordOperator(boolean notAssertWhenNotSupport) {
        mNotAssertWhenNotSupport = notAssertWhenNotSupport;
    }

    @Override
    protected void doOperate() {
        UiObject2 shutter = Utils.findObject(
                Utils.getShutterSelector().desc("Video"), Utils.TIME_OUT_SHORT);

        if (mNotAssertWhenNotSupport && shutter == null) {
            LogHelper.d(TAG, "[doOperate] not find video shutter button, return");
            return;
        } else {
            Utils.assertRightNow(shutter != null);
        }

        shutter.click();
        TestContext.mLatestStartRecordTime = System.currentTimeMillis();
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
            return "Press video shutter button to start record, if current not support video, do " +
                    "nothing";
        } else {
            return "Press video shutter button to start record";
        }

    }
}
