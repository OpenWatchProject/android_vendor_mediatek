package com.mediatek.gallery3d.layout;

import android.annotation.TargetApi;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.BitmapFactory;
import android.graphics.BitmapFactory.Options;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.util.DisplayMetrics;

import com.android.gallery3d.common.ApiHelper;
import com.android.gallery3d.common.BitmapUtils;
import com.android.gallery3d.common.Utils;
import com.android.gallery3d.data.LocalMediaItem;
import com.android.gallery3d.data.LocalVideo;
import com.android.gallery3d.data.MediaItem;
import com.android.gallery3d.data.Path;
import com.android.gallery3d.util.ThreadPool.CancelListener;
import com.android.gallery3d.util.ThreadPool.JobContext;
import com.mediatek.gallery3d.util.FeatureConfig;
import com.mediatek.gallery3d.util.Log;
import com.mediatek.gallerybasic.base.MediaData;

import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;

/**
 * Helper utils for fancy.
 */
public class FancyHelper {
    private static final String TAG = "MtkGallery2/FancyHelper";
    private static final boolean ENABLE_FANCY = FeatureConfig.SUPPORT_FANCY_HOMEPAGE;
    public static final int ORIENTATION_90 = 90;
    public static final int ORIENTATION_270 = 270;
    public static final float FANCY_CROP_RATIO = 2.5f; //5:2
    public static final float FANCY_CROP_RATIO_LAND = 0.4f; //2:5
    public static final float FANCY_CROP_RATIO_CAMERA = 1.78f; //16:9
    public static final int ALBUMSETPAGE_COL_LAND = 4;
    public static final int ALBUMSETPAGE_COL_PORT = 2;
    // keep same with "albumset_slot_gap" in dimens.xml
    public static final int ALBUMSETPAGE_GAP = 7;
    public static final int ALBUMPAGE_COL_LAND = 4;
    public static final int ALBUMPAGE_COL_PORT = 3;
    public static final float FANCY_CAMERA_ICON_SIZE_RATE = 0.11f; //1:9.5
    public static final int FANCY_THUMBNAIL_RATIO = 3;
    public static final float HALF_SCALE = 0.5f;

    /// M: [FEATURE.ADD] Multi-window. @{
    public static final int INVALID_LAYOUT = -1;
    public static final int DEFAULT_LAYOUT = 0;
    public static final int FANCY_LAYOUT = 1;
    public static final int MULTI_WINDOW_LAYOUT = 2;
    /// @}

    // for video overlay
    public static final float VIDEO_OVERLAY_RECT_HEIGHT = 20.0f;
    public static final float VIDEO_OVERLAY_LEFT_OFFSET = 10.0f;
    public static final float VIDEO_OVERLAY_RECT_GAP = 27.0f; // Gap:rectHeight ~= 4:3
    public static final int VIDEO_OVERLAY_COLOR = 0xff323232;

    private static int sHeightPixels = -1;
    private static int sWidthPixels = -1;

    public static boolean isFancyLayoutSupported() {
        return ENABLE_FANCY;
    }

    /**
     * Check current item is land(width > height) or not.
     * @param  item mediaItem
     * @return true if land
     */
    public static final boolean isLandItem(MediaItem item) {
        if (item == null) {
            return false;
        }

        int rotation = item instanceof LocalVideo ? ((LocalVideo) item).getOrientation() :
            item.getRotation();
        if (rotation == ORIENTATION_90 || rotation == ORIENTATION_270) {
            return (item.getWidth() >= item.getHeight() ? false : true);
        } else {
            return (item.getHeight() >= item.getWidth() ? false : true);
        }
    }

    /**
     * Get mediaset path.
     * @param item media item
     * @return     Mediaset Path for current media item
     */
    public static Path getMediaSetPath(LocalMediaItem item) {
        if (item == null) {
            return null;
        }

        int bucketId = item.getBucketId();
        return Path.fromString("/local/all/" + bucketId);
    }

    /**
     * Get mediaset path.
     *
     * @param data
     *            media data
     * @return Mediaset Path for current item
     */
    public static Path getMediaSetPath(MediaData data) {
        if (data == null) {
            return null;
        }

        int bucketId = data.bucketId;
        return Path.fromString("/local/all/" + bucketId);
    }

    /**
     * Get mediaset path from bucketId.
     * @param bucketId mediaset's bucketId
     * @return         Path of current mediaSet
     */
    public static Path getMediaSetPath(int bucketId) {
        return Path.fromString("/local/all/" + bucketId);
    }

    public static int getHeightPixels() {
        return sHeightPixels;
    }

    public static int getWidthPixels() {
        return sWidthPixels;
    }

    public static int getScreenWidthAtFancyMode() {
        return Math.min(sHeightPixels, sWidthPixels);
    }

    /**
     * Get label width of full screen.
     * @param labelWidth normal label width
     * @return full screen label width
     */
    public static int getFullScreenLabelWidth(int labelWidth) {
        if (labelWidth > 1) {
            return labelWidth * ALBUMSETPAGE_COL_PORT + (ALBUMSETPAGE_COL_PORT - 1)
                    * ALBUMSETPAGE_GAP;
        }
        return getScreenWidthAtFancyMode();
    }

    /**
     * Get slot width at fancy mode.
     * @return slot width
     */
    public static int getSlotWidthAtFancyMode() {
        int width = getScreenWidthAtFancyMode();
        return (width - (ALBUMSETPAGE_COL_PORT - 1) * ALBUMSETPAGE_GAP) / ALBUMSETPAGE_COL_PORT;
    }

    /**
     * Do fancy initialization.
     * @param widthPixel  screen width
     * @param heightPixel screen height
     */
    public static void doFancyInitialization(int widthPixel, int heightPixel) {
        // just update screen width
        int screenWidth = Math.min(widthPixel, heightPixel);
        if (sHeightPixels > sWidthPixels) {
            sWidthPixels = screenWidth;
        } else {
            sHeightPixels = screenWidth;
        }
        Log.d(TAG, "<doFancyInitialization> <Fancy> w x h: " + sWidthPixels + ", "
                + sHeightPixels);
        MediaItem.setFancyThumbnailSizes(Math.min(sHeightPixels, sWidthPixels)
                / FANCY_THUMBNAIL_RATIO);
    }

    /**
     * initialize FancyThumbnailSizes.
     * @param metrics screen metrics
     */
    public static void initializeFancyThumbnailSizes(DisplayMetrics metrics) {
        MediaItem.setFancyThumbnailSizes(Math.min(metrics.heightPixels,
                metrics.widthPixels) / FANCY_THUMBNAIL_RATIO);
        sHeightPixels = metrics.heightPixels;
        sWidthPixels = metrics.widthPixels;
    }

    /**
     * Resize bitmap.
     * @param bitmap        src bitmap
     * @param targetSize    resize target size
     * @param resizeByWidth resize by width or height
     * @param recycle       recycle current bitmap
     * @return              resized bitmap
     */
    public static Bitmap resizeByWidthOrLength(Bitmap bitmap, int targetSize,
            boolean resizeByWidth, boolean recycle) {
        if (bitmap == null) {
            return null;
        }

        int w = bitmap.getWidth();
        int h = bitmap.getHeight();
        int targetWidth = w;
        int targetHeight = h;
        float scale = 1.0f;

        if (resizeByWidth) {
            if (w == targetSize) {
                return bitmap;
            }
            scale = (float) targetSize / w;
            targetWidth = targetSize;
            targetHeight = Math.round(scale * h);
        } else {
            if (h == targetSize) {
                return bitmap;
            }
            scale = (float) targetSize / h;
            targetHeight = targetSize;
            targetWidth = Math.round(scale * w);
        }

        Bitmap target = Bitmap.createBitmap(targetWidth, targetHeight,
                getConfig(bitmap));
        Canvas canvas = new Canvas(target);
        canvas.scale(scale, scale);
        Paint paint = new Paint(Paint.FILTER_BITMAP_FLAG | Paint.DITHER_FLAG);
        canvas.drawBitmap(bitmap, 0, 0, paint);
        if (recycle) {
            bitmap.recycle();
        }
        return target;
    }

    /**
     * Resize and crop photo center.
     * @param bitmap       src bitmap
     * @param targetWidth  resize target width
     * @param targetHeight resize target height
     * @param scaleByWidth scale by width or height
     * @param recycle      recycle current bitmap
     * @return             resized and croped bitmap
     */
    public static Bitmap resizeAndCropCenter(Bitmap bitmap, int targetWidth,
            int targetHeight, boolean scaleByWidth, boolean recycle) {
        if (bitmap == null) {
            return null;
        }

        int w = bitmap.getWidth();
        int h = bitmap.getHeight();
        if (w == targetWidth && h == targetHeight) {
            return bitmap;
        }

        float scaleX = (float) targetWidth / w;
        float scaleY = (float) targetHeight / h;
        float scale = scaleByWidth ? scaleX : scaleY;

        Bitmap target = Bitmap.createBitmap(targetWidth, targetHeight,
                getConfig(bitmap));
        w = Math.round(scale * bitmap.getWidth());
        h = Math.round(scale * bitmap.getHeight());
        Canvas canvas = new Canvas(target);
        canvas.translate((targetWidth - w) / 2f, (targetHeight - h) / 2f);
        canvas.scale(scale, scale);
        Paint paint = new Paint(Paint.FILTER_BITMAP_FLAG | Paint.DITHER_FLAG);
        canvas.drawBitmap(bitmap, 0, 0, paint);
        if (recycle) {
            bitmap.recycle();
        }
        return target;
    }

    /**
     * Resize and crop photo center.
     * @param bitmap src bitmap
     * @param size target size
     * @param recycle recycle current bitmap
     * @return resized and croped bitmap
     */
    public static Bitmap resizeAndCropCenter(Bitmap bitmap, int size, boolean recycle) {
        if (bitmap == null) {
            return null;
        }

        int w = bitmap.getWidth();
        int h = bitmap.getHeight();
        if (w == size && h == size) return bitmap;

        // scale the image so that the shorter side equals to the target;
        // the longer side will be center-cropped.
        float scale = (float) size / Math.min(w,  h);
        Bitmap target = Bitmap.createBitmap(size, size, getConfig(bitmap));

        int width = Math.round(scale * bitmap.getWidth());
        int height = Math.round(scale * bitmap.getHeight());

        Canvas canvas = new Canvas(target);
        canvas.translate((size - width) / 2f, (size - height) / 2f);
        canvas.scale(scale, scale);
        Paint paint = new Paint(Paint.FILTER_BITMAP_FLAG | Paint.DITHER_FLAG);
        canvas.drawBitmap(bitmap, 0, 0, paint);

        if (recycle) bitmap.recycle();
        return target;
    }

    private static Bitmap.Config getConfig(Bitmap bitmap) {
        if (bitmap == null) {
            return Bitmap.Config.ARGB_8888;
        }

        Bitmap.Config config = bitmap.getConfig();
        if (config == null) {
            config = Bitmap.Config.ARGB_8888;
        }
        return config;
    }

    /**
     * Decode bitmap thumbnail.
     * @param jc         job context
     * @param filePath   file path for decoded bitmap
     * @param options    decoding options
     * @param targetSize target size
     * @param type       thumbnail type
     * @return           thumbnail bitmap
     */
    public static Bitmap decodeThumbnail(
            JobContext jc, String filePath, Options options, int targetSize, int type) {
        if (jc == null) {
            Log.d(TAG, "<decodeThumbnail> jc is null");
            return null;
        }

        FileInputStream fis = null;
        try {
            fis = new FileInputStream(filePath);
            FileDescriptor fd = fis.getFD();
            return decodeThumbnail(jc, fd, options, targetSize, type);
        } catch (FileNotFoundException e) {
            Log.d(TAG, "<decodeThumbnail> FileNotFoundException ", e);
            return null;
        } catch (IOException e) {
            Log.d(TAG, "<decodeThumbnail> IOException ", e);
            return null;
        } finally {
            Utils.closeSilently(fis);
        }
    }

    private static Bitmap decodeThumbnail(JobContext jc, FileDescriptor fd, Options options,
            int targetSize, int type) {
        if (type != MediaItem.TYPE_FANCYTHUMBNAIL) {
            return null;
        }

        if (options == null) {
            options = new Options();
        }
        jc.setCancelListener(new DecodeCanceller(options));

        options.inJustDecodeBounds = true;
        BitmapFactory.decodeFileDescriptor(fd, null, options);

        if (jc.isCancelled()) {
            return null;
        }

        int w = options.outWidth;
        int h = options.outHeight;

        if (((float) w / (float) h >= FancyHelper.FANCY_CROP_RATIO
                || (float) h / (float) w >= FancyHelper.FANCY_CROP_RATIO)) {
            int halfScreenWidth = getScreenWidthAtFancyMode() / 2;
            float scale = (float) halfScreenWidth / Math.min(w, h);
            options.inSampleSize = BitmapUtils.computeSampleSizeLarger(scale);
        } else {
            // For screen nail, we only want to keep the longer side >= targetSize.
            float scale = (float) targetSize / Math.max(w, h);
            options.inSampleSize = BitmapUtils.computeSampleSizeLarger(scale);
        }
        Log.d(TAG, "<decodeThumbnail> set samplesize to " + options.inSampleSize);
        options.inJustDecodeBounds = false;
        setOptionsMutable(options);

        long logTimeBeforDecode = System.currentTimeMillis();
        Bitmap result = BitmapFactory.decodeFileDescriptor(fd, null, options);
        Log.d(TAG, "<decodeThumbnail> decoding bmp costs "
                + (System.currentTimeMillis() - logTimeBeforDecode));

        if (result == null) {
            return null;
        }

        // resize down if the decoder does not support inSampleSize
        float scale = (float) targetSize / Math.min(result.getWidth(), result.getHeight());
        if (scale <= HALF_SCALE) {
            Log.d(TAG, "<decodeThumbnail> resize down to scale " + scale);
            result = BitmapUtils.resizeBitmapByScale(result, scale, true);
        }

        return ensureGLCompatibleBitmap(result);
    }

    /**
     * SetOptionsMutable.
     * @param options decoding options
     */
    @TargetApi(ApiHelper.VERSION_CODES.HONEYCOMB)
    public static void setOptionsMutable(Options options) {
        if (ApiHelper.HAS_OPTIONS_IN_MUTABLE) {
            options.inMutable = true;
        }
    }

    /**
     * Ensure GLCompatibleBitmap.
     * @param bitmap src bitmap
     * @return       bitmap compatible with OPENGL
     */
    public static Bitmap ensureGLCompatibleBitmap(Bitmap bitmap) {
        if (bitmap == null || bitmap.getConfig() != null) {
            return bitmap;
        }
        Bitmap newBitmap = bitmap.copy(Config.ARGB_8888, false);
        bitmap.recycle();
        return newBitmap;
    }

    /**
     * DecodeCanceller.
     */
    private static class DecodeCanceller implements CancelListener {
        Options mOptions;

        public DecodeCanceller(Options options) {
            mOptions = options;
        }

        @Override
        public void onCancel() {
            mOptions.requestCancelDecode();
        }
    }
}
