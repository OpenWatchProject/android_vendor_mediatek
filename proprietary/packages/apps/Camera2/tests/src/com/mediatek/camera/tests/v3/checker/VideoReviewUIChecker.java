package com.mediatek.camera.tests.v3.checker;

import android.support.test.uiautomator.By;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.CheckerOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

/**
 * Check review ui for intent video.
 */

public class VideoReviewUIChecker extends CheckerOne {
    private static final LogUtil.Tag TAG = Utils.getTestTag(VideoReviewUIChecker.class
            .getSimpleName());

    @Override
    protected void doCheck() {
        // check has show play button view
        Utils.assertObject(By.res("com.mediatek.camera:id/btn_play"));
        // check has show retake button view
        Utils.assertObject(By.res("com.mediatek.camera:id/btn_retake"));
        // check has show save button view
        Utils.assertObject(By.res("com.mediatek.camera:id/btn_save"));
    }

    @Override
    public Page getPageBeforeCheck() {
        return Page.PREVIEW;
    }

    @Override
    public String getDescription() {
        return "Check camera is intent video review ui now";
    }
}
