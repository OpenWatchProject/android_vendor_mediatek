package com.mediatek.camera.tests.v3.observer;

import android.database.Cursor;
import android.provider.MediaStore;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.Observer;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

import java.util.ArrayList;
import java.util.List;


public abstract class SavedObserver extends Observer {
    private static final LogUtil.Tag TAG = Utils.getTestTag(SavedObserver.class.getSimpleName());

    private static final String[] PROJECTION_IMAGE = new String[]{
            MediaStore.Images.ImageColumns._ID, MediaStore.Images.ImageColumns.DATE_TAKEN,
            MediaStore.Images.ImageColumns.DATA
    };
    private static final String ORDER_CLAUSE_IMAGE = MediaStore.Images.ImageColumns.DATE_TAKEN +
            " DESC";
    private static final String WHERE_CLAUSE_IMAGE =
            "(" + MediaStore.Images.ImageColumns.DATE_TAKEN +
                    " >= ?) AND (" + MediaStore.Images.ImageColumns.DATE_TAKEN + " <= ?)";

    private static final String[] PROJECTION_VIDEO = new String[]{
            MediaStore.Video.VideoColumns._ID, MediaStore.Video.VideoColumns.DATE_TAKEN,
            MediaStore.Video.VideoColumns.DATA
    };
    private static final String ORDER_CLAUSE_VIDEO = MediaStore.Video.VideoColumns.DATE_TAKEN +
            " DESC";
    private static final String WHERE_CLAUSE_VIDEO =
            "(" + MediaStore.Video.VideoColumns.DATE_TAKEN +
                    " >= ?) AND (" + MediaStore.Video.VideoColumns.DATE_TAKEN + " <= ?)";

    private static final int PROJECTION_DATA = 2;
    private static final int PROJECTION_DATE_TAKEN = 1;
    private static final int PROJECTION_ID = 0;

    protected long mTimeStartObserve;
    protected List<String> mVideoFilePath = new ArrayList<String>();
    protected List<String> mImageFilePath = new ArrayList<String>();

    @Override
    protected void doBeginObserve(int index) {
        mTimeStartObserve = System.currentTimeMillis();
    }

    protected void updateVideoFilePath() {
        mVideoFilePath.clear();
        Cursor cursor = Utils.getTargetContext().getContentResolver().query(
                MediaStore.Video.Media.EXTERNAL_CONTENT_URI,
                PROJECTION_VIDEO,
                WHERE_CLAUSE_VIDEO,
                new String[]{String.valueOf(mTimeStartObserve),
                        String.valueOf(System.currentTimeMillis())},
                ORDER_CLAUSE_VIDEO
        );
        if (cursor != null && cursor.moveToFirst()) {
            do {
                mVideoFilePath.add(cursor.getString(PROJECTION_DATA));
                LogHelper.d(TAG, "[updateVideoFilePath] find out video " + cursor.getString
                        (PROJECTION_DATA));
            } while (cursor.moveToNext());
            cursor.close();
        }
    }

    protected void updateImageFilePath() {
        mImageFilePath.clear();
        Cursor cursor = Utils.getTargetContext().getContentResolver().query(
                MediaStore.Images.Media.EXTERNAL_CONTENT_URI,
                PROJECTION_IMAGE,
                WHERE_CLAUSE_IMAGE,
                new String[]{String.valueOf(mTimeStartObserve),
                        String.valueOf(System.currentTimeMillis())},
                ORDER_CLAUSE_IMAGE
        );
        if (cursor != null) {
            if (cursor.moveToFirst()) {
                do {
                    mImageFilePath.add(cursor.getString(PROJECTION_DATA));
                    LogHelper.d(TAG, "[updateImageFilePath] find out image " + cursor.getString
                            (PROJECTION_DATA));
                } while (cursor.moveToNext());
            }
            cursor.close();
        }
    }
}
