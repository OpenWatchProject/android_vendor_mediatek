package com.mediatek.camera.tests.v3.operator;

import android.media.MediaPlayer;
import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.OperatorOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.arch.TestContext;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

import java.io.IOException;

public class GoToGalleryPlayVideoOperator extends OperatorOne {
    private static final LogUtil.Tag TAG = Utils.getTestTag(GoToGalleryPlayVideoOperator.class
            .getSimpleName());

    @Override
    protected void doOperate() {
        UiObject2 thumb = Utils.findObject(
                By.res("com.mediatek.camera:id/thumbnail").clickable(true).enabled(true));
        Utils.assertRightNow(thumb != null);
        LogHelper.d(TAG, "[doOperate] click thumbnail to launch gallery");
        thumb.click();
        Utils.assertObject(By.res("com.android.gallery3d:id/gl_root_view"));

        LogHelper.d(TAG, "[doOperate] gallery launched, click center to play video");
        int width = Utils.getUiDevice().getDisplayWidth();
        int height = Utils.getUiDevice().getDisplayHeight();
        Utils.getUiDevice().click(width / 2, height / 2);
        Utils.assertObject(By.res("com.android.gallery3d:id/movie_view"));

        LogHelper.d(TAG, "[doOperate] video playing, wait finished");
        Utils.assertNoObject(By.res("com.android.gallery3d:id/movie_view"), (int)
                getVideoDuration() + Utils.TIME_OUT_SHORT);
        Utils.assertObject(By.res("com.android.gallery3d:id/gl_root_view"));

        LogHelper.d(TAG, "[doOperate] play finished, press back to return gallery");
        Utils.pressBackUtilFindNoObject(By.res("com.android.gallery3d:id/gl_root_view"), 2);
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
        return "Click thumbnail to launch gallery, play video, back to camera until play finished";
    }

    private static final long getVideoDuration() {
        MediaPlayer mediaPlayer = new MediaPlayer();
        int realDuration = 0;
        try {
            mediaPlayer.setDataSource(TestContext.mLatestVideoPath);
            mediaPlayer.prepare();
            realDuration = mediaPlayer.getDuration();
            mediaPlayer.release();
        } catch (IOException e) {
            LogHelper.d(TAG, "[getVideoDuration] IOException pop up", e);
            Utils.assertRightNow(false);
        }
        return realDuration;
    }
}
