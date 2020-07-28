package com.mediatek.camera.tests.v3.checker;

import android.media.MediaExtractor;
import android.media.MediaFormat;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.CheckerOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.arch.TestContext;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

import java.io.IOException;

public class VideoFpsChecker extends CheckerOne {
    private static final LogUtil.Tag TAG = Utils.getTestTag(VideoFpsChecker.class.getSimpleName());
    private int mTargetVideoFps = 30;
    private int mFindFps;

    public VideoFpsChecker(int targetVideoFps) {
        mTargetVideoFps = targetVideoFps;
    }

    @Override
    protected void doCheck() {
        if (TestContext.mLatestVideoPath != null) {
            MediaExtractor mediaExtractor = new MediaExtractor();
            try {
                mediaExtractor.setDataSource(TestContext.mLatestVideoPath);
                int numTracks = mediaExtractor.getTrackCount();
                LogHelper.e(TAG, "[doCheck] numTracks = " + numTracks);
                for (int i = 0; i < numTracks; i++) {
                    MediaFormat format = mediaExtractor.getTrackFormat(i);
                    if (format.containsKey(MediaFormat.KEY_MIME)
                            && format.getString(MediaFormat.KEY_MIME).startsWith("video/")
                            && format.containsKey(MediaFormat.KEY_FRAME_RATE)) {
                        LogHelper.e(TAG, "[doCheck] track " + i + ", fps = "
                                + format.getInteger(MediaFormat.KEY_FRAME_RATE));
                        Utils.assertRightNow(
                                format.getInteger(MediaFormat.KEY_FRAME_RATE) == mTargetVideoFps
                                , "Target fps is " + mTargetVideoFps + ", but find out "
                                        + format.getInteger(MediaFormat.KEY_FRAME_RATE));
                        mFindFps = format.getInteger(MediaFormat.KEY_FRAME_RATE);
                    }
                }
            } catch (IOException e) {
                LogHelper.e(TAG, "[doCheck] IOException", e);
            } finally {
                mediaExtractor.release();
            }
            Utils.assertRightNow(mFindFps == mTargetVideoFps, "Not find fps info");
        }
    }

    @Override
    public Page getPageBeforeCheck() {
        return null;
    }

    @Override
    public String getDescription() {
        return "Check latest saved video fps = " + mTargetVideoFps;
    }
}
