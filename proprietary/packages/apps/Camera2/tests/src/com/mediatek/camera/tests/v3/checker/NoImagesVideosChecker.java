package com.mediatek.camera.tests.v3.checker;

import android.database.Cursor;
import android.provider.MediaStore;

import com.mediatek.camera.tests.v3.arch.CheckerOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Condition;
import com.mediatek.camera.tests.v3.util.Utils;


public class NoImagesVideosChecker extends CheckerOne {
    private static final String[] PROJECTION_IMAGE = new String[]{
            MediaStore.Images.ImageColumns._ID, MediaStore.Images.ImageColumns.DATE_TAKEN,
            MediaStore.Images.ImageColumns.DATA
    };
    private static final String ORDER_CLAUSE_IMAGE = MediaStore.Images.ImageColumns.DATE_TAKEN +
            " DESC";
    private static final String[] PROJECTION_VIDEO = new String[]{
            MediaStore.Video.VideoColumns._ID, MediaStore.Video.VideoColumns.DATE_TAKEN,
            MediaStore.Video.VideoColumns.DATA
    };
    private static final String ORDER_CLAUSE_VIDEO = MediaStore.Video.VideoColumns.DATE_TAKEN +
            " DESC";

    @Override
    protected void doCheck() {
        Utils.assertCondition(new Condition() {
            @Override
            public boolean isSatisfied() {
                Cursor cursorImage = Utils.getTargetContext().getContentResolver().query(
                        MediaStore.Images.Media.EXTERNAL_CONTENT_URI,
                        PROJECTION_IMAGE,
                        null,
                        null,
                        ORDER_CLAUSE_IMAGE
                );
                Cursor cursorVideo = Utils.getTargetContext().getContentResolver().query(
                        MediaStore.Video.Media.EXTERNAL_CONTENT_URI,
                        PROJECTION_VIDEO,
                        null,
                        null,
                        ORDER_CLAUSE_VIDEO
                );

                return (cursorImage == null || cursorImage.getCount() == 0) &&
                        (cursorVideo == null || cursorVideo.getCount() == 0);
            }
        });
    }

    @Override
    public Page getPageBeforeCheck() {
        return null;
    }

    @Override
    public String getDescription() {
        return "Check no images and videos in storage";
    }
}
