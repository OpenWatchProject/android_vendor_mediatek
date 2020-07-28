package com.mediatek.camera.tests.v3.checker;

import android.database.Cursor;
import android.media.MediaPlayer;
import android.provider.MediaStore;

import com.mediatek.camera.tests.v3.arch.CheckerOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.arch.TestContext;
import com.mediatek.camera.tests.v3.util.Utils;

import java.io.IOException;


public class VideoFileSizeChecker extends CheckerOne {

    private static final String[] PROJECTION_VIDEO = new String[]{
            MediaStore.Video.VideoColumns._ID, MediaStore.Video.VideoColumns.WIDTH,
            MediaStore.Video.VideoColumns.HEIGHT
    };
    private static final int INDEX_WIDTH = 1;
    private static final int INDEX_HEIGHT = 2;
    private static final String WHERE_CLAUSE_VIDEO = MediaStore.Video.VideoColumns.DATA + " = ?";

    @Override
    protected void doCheck() {
        if (TestContext.mLatestVideoPath != null) {
            // read info from file
            try {
                MediaPlayer mediaPlayer = new MediaPlayer();
                mediaPlayer.setDataSource(TestContext.mLatestVideoPath);
                mediaPlayer.prepare();
                int fileWidth = mediaPlayer.getVideoWidth();
                int fileHeight = mediaPlayer.getVideoHeight();
                assertVideoSize(fileWidth, fileHeight);
            } catch (IOException e) {
                e.printStackTrace();
            }

            // query from media database
            Cursor cursor = Utils.getTargetContext().getContentResolver().query(
                    MediaStore.Video.Media.EXTERNAL_CONTENT_URI,
                    PROJECTION_VIDEO,
                    WHERE_CLAUSE_VIDEO,
                    new String[]{String.valueOf(TestContext.mLatestVideoPath)},
                    null);
            Utils.assertRightNow(cursor != null && cursor.moveToFirst() && cursor.getCount() == 1);
            int width = cursor.getInt(INDEX_WIDTH);
            int height = cursor.getInt(INDEX_HEIGHT);
            cursor.close();
            assertVideoSize(width, height);
        }
    }

    @Override
    public Page getPageBeforeCheck() {
        return null;
    }

    @Override
    public String getDescription() {
        return "Check the video size of latest captured video";
    }

    private void assertVideoSize(int readWidth, int readHeight) {
        boolean mapping =
                readWidth == TestContext.mLatestVideoSizeSettingWidth
                        && readHeight == TestContext.mLatestVideoSizeSettingHeight;
        boolean wrapMapping =
                readWidth == TestContext.mLatestVideoSizeSettingHeight
                        && readHeight == TestContext.mLatestVideoSizeSettingWidth;
        Utils.assertRightNow(mapping || wrapMapping, "Expected " +
                "(w = " + TestContext.mLatestVideoSizeSettingWidth
                + ", h = " + TestContext.mLatestVideoSizeSettingHeight
                + ") or (w = " + TestContext.mLatestVideoSizeSettingHeight
                + ", h = " + TestContext.mLatestVideoSizeSettingWidth
                + "), but find (w = " + readWidth + ", h = " + readHeight);
    }
}