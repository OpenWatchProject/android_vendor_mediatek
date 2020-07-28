package com.mediatek.camera.tests.v3.checker;

import android.media.MediaPlayer;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.annotation.CoverPoint;
import com.mediatek.camera.tests.v3.arch.Checker;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.arch.TestContext;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

import java.io.IOException;

@CoverPoint(pointList = {"Check video duration by get info from MediaPlayer"})
public class VideoDurationChecker extends Checker {
    private static final LogUtil.Tag TAG = Utils.getTestTag(
            VideoDurationChecker.class.getSimpleName());
    private static final int ACCEPTABLE_BUFFER_DURATION_MS = 5000;
    private static final int SLOW_MOTION_SPEED = 4;
    public static final int INDEX_NORMAL = 0;
    public static final int INDEX_SLOW_MOTION = 1;

    @Override
    public int getCheckCount() {
        return 2;
    }

    @Override
    protected void doCheck(int index) {
        MediaPlayer mediaPlayer = new MediaPlayer();
        int realDuration = 0;
        int recordedDuration = 0;
        int sub = 0;
        try {
            mediaPlayer.setDataSource(TestContext.mLatestVideoPath);
            mediaPlayer.prepare();
            realDuration = mediaPlayer.getDuration();
            recordedDuration = TestContext.mLatestRecordVideoDurationInSeconds * 1000;
            sub = Math.abs(realDuration - recordedDuration);
            LogHelper.d(TAG, "[doCheck] real duration is " + realDuration
                    + " ms, recorded duration " + "is " + recordedDuration + " ms");
            mediaPlayer.release();
        } catch (IOException e) {
            LogHelper.d(TAG, "[doCheck] IOException pop up", e);
            Utils.assertRightNow(false, "IOException pop up");
        }
        switch (index) {
            case INDEX_NORMAL:
                Utils.assertRightNow(sub < ACCEPTABLE_BUFFER_DURATION_MS);
                break;
            case INDEX_SLOW_MOTION:
                Utils.assertRightNow(realDuration >= recordedDuration * SLOW_MOTION_SPEED);
                break;
        }
    }

    @Override
    public Page getPageBeforeCheck(int index) {
        return null;
    }

    @Override
    public String getDescription(int index) {
        switch (index) {
            case INDEX_NORMAL:
                return "Check latest recorded video duration";
            case INDEX_SLOW_MOTION:
                return "Check latest recorded slow motion video duration";
        }
        return null;
    }
}
