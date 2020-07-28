package com.mediatek.camera.tests.v3.checker;

import android.database.Cursor;
import android.graphics.BitmapFactory;
import android.media.ExifInterface;
import android.provider.MediaStore;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.annotation.CoverPoint;
import com.mediatek.camera.tests.v3.arch.CheckerOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.arch.TestContext;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

@CoverPoint(pointList = {"Check photo size by decode image file",
        "Check photo size by the record in media database",
        "Check photo size by exif info"})
public class CapturedPhotoPictureSizeChecker extends CheckerOne {
    private static final LogUtil.Tag TAG = Utils.getTestTag(CapturedPhotoPictureSizeChecker.class
            .getSimpleName());
    private static final String[] PROJECTION_IMAGE = new String[]{
            MediaStore.Images.ImageColumns._ID, MediaStore.Images.ImageColumns.WIDTH,
            MediaStore.Images.ImageColumns.HEIGHT
    };
    private static final int INDEX_WIDTH = 1;
    private static final int INDEX_HEIGHT = 2;

    private static final String WHERE_CLAUSE_IMAGE = MediaStore.Images.ImageColumns.DATA + " = ?";

    @Override
    protected void doCheck() {
        if (TestContext.mLatestPhotoPath != null) {
            // decode from image
            BitmapFactory.Options option = new BitmapFactory.Options();
            option.inJustDecodeBounds = true;
            BitmapFactory.decodeFile(TestContext.mLatestPhotoPath, option);

            LogHelper.d(TAG, "[doCheck] decode from image, width = " + option.outWidth +
                    ", height = " + option.outHeight);
            assertPictureSize(option.outWidth, option.outHeight);

            // query from media database
            Cursor cursor = Utils.getTargetContext().getContentResolver().query(
                    MediaStore.Images.Media.EXTERNAL_CONTENT_URI,
                    PROJECTION_IMAGE,
                    WHERE_CLAUSE_IMAGE,
                    new String[]{String.valueOf(TestContext.mLatestPhotoPath)},
                    null);
            Utils.assertRightNow(cursor != null && cursor.moveToFirst() && cursor.getCount() == 1,
                    "Cannot find item in media db for " + TestContext.mLatestPhotoPath);
            int width = cursor.getInt(INDEX_WIDTH);
            int height = cursor.getInt(INDEX_HEIGHT);
            cursor.close();
            LogHelper.d(TAG, "[doCheck] read from media db, width = " + width +
                    ", height = " + height);
            assertPictureSize(width, height);

            // read info from exif
            width = Utils.getIntInExif(TestContext.mLatestPhotoPath, ExifInterface
                    .TAG_IMAGE_WIDTH, 0);
            height = Utils.getIntInExif(TestContext.mLatestPhotoPath, ExifInterface
                    .TAG_IMAGE_LENGTH, 0);
            LogHelper.d(TAG, "[doCheck] read from exif, width = " + width +
                    ", height = " + height);
            assertPictureSize(width, height);
        }
    }

    @Override
    public Page getPageBeforeCheck() {
        return null;
    }

    @Override
    public String getDescription() {
        return "Check the picture size of latest captured photo";
    }

    private void assertPictureSize(int readWidth, int readHeight) {
        boolean mapping =
                readWidth == TestContext.mLatestPictureSizeSettingWidth
                        && readHeight == TestContext.mLatestPictureSizeSettingHeight;
        boolean wrapMapping =
                readWidth == TestContext.mLatestPictureSizeSettingHeight
                        && readHeight == TestContext.mLatestPictureSizeSettingWidth;
        Utils.assertRightNow(mapping || wrapMapping, "Expected " +
                "(w = " + TestContext.mLatestPictureSizeSettingWidth
                + ", h = " + TestContext.mLatestPictureSizeSettingHeight
                + ") or (w = " + TestContext.mLatestPictureSizeSettingHeight
                + ", h = " + TestContext.mLatestPictureSizeSettingWidth
                + "), but find (w = " + readWidth + ", h = " + readHeight);
    }
}
