package com.mediatek.camera.tests.v3.checker;

import android.support.test.uiautomator.By;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.CheckerOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

public class RecordingChecker extends CheckerOne {
    private static final LogUtil.Tag TAG = Utils.getTestTag(RecordingChecker.class
            .getSimpleName());

    @Override
    protected void doCheck() {
        // check has show recording time view
        Utils.assertObject(By.res("com.mediatek.camera:id/recording_time"));
        // check the content of recording time is not 00:00
        Utils.assertNoObject(By.res("com.mediatek.camera:id/recording_time").text("00:00"));
    }

    @Override
    public Page getPageBeforeCheck() {
        return Page.PREVIEW;
    }

    @Override
    public String getDescription() {
        return "Check camera is recording now";
    }
}
