package com.mediatek.galleryfeature.drm;

import android.content.Context;
import android.drm.DrmStore;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.util.DisplayMetrics;
import android.view.WindowManager;

import com.mediatek.gallerybasic.base.*;
import com.mediatek.gallerybasic.platform.PlatformHelper;
import com.mediatek.gallerybasic.util.BitmapUtils;
import com.mediatek.gallerybasic.util.Log;

import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.HashMap;

/**
 * Create bitmap and get supported operations.
 */
public class DrmItem extends ExtItem {
    private static final String TAG = "MtkGallery2/DrmItem";
    public static int ACTION_WALLPAPER;
    public static int ACTION_PRINT;
    public static int sTargetSize = 0;
    // We try to scale up the image to fill the screen. But in order not to
    // scale too much for small icons, we limit the max up-scaling factor here.
    public final static float RATIO_WITH_SCREEN = 0.5f;
    private final static int FLAG_DRM = 0x100;
    private MediaCenter mMediaCenter;
    private ExtItem mRealItem;
    private HashMap<Integer, Boolean> mSupportedOperations = new HashMap<Integer, Boolean>();
    private static boolean mHasInitializedField;
    /**
     * Whether has rights to decode, it was set after decode drm bitmap succeed or fail.
     * If decode bitmap return null, set mHasRightsToDecode as false.
     * If decode bitmap return non-null, set mHasRightsToDecode as true.
     */
    private boolean mHasRightsToDecode = true;

    private DrmRightsManager mManager;
    private DataChangeListener mListener;
    /**
     * Constructor.
     * @param context
     *            the Context.
     * @param data
     *            The drm data.
     * @param center
     *            The media center.
     */
    public DrmItem(Context context, MediaData data, MediaCenter center) {
        super(context, data);
        if (context == null || data == null || center == null) {
            throw new IllegalArgumentException("invalid arguments, context = " + context
                    + ", data = " + data + ", center = " + center);
        }
        mMediaCenter = center;
        mRealItem = mMediaCenter.getRealItem(mMediaData, DrmMember.sType);
        if (sTargetSize <= 0) {
            DisplayMetrics metrics = new DisplayMetrics();
            WindowManager wm =
                    (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
            wm.getDefaultDisplay().getMetrics(metrics);
            int sMinPixels = Math.min(metrics.heightPixels, metrics.widthPixels);
            float scaleLimit = PlatformHelper.getMinScaleLimit(mMediaData);
            int length = (int) (RATIO_WITH_SCREEN * (sMinPixels / scaleLimit));
            sTargetSize = Integer.highestOneBit(length);
            Log.d(TAG, "<DrmItem> sMinPixels = " + sMinPixels + "scaleLimit = " +
                    scaleLimit + " sTargetSize = " +
                    sTargetSize);
        }
        if (!mHasInitializedField) {
            ACTION_WALLPAPER = getAction("WALLPAPER");
            ACTION_PRINT = getAction("PRINT");
            mHasInitializedField = true;
        }

        mManager = DrmRightsManager.getDrmRightManager(context);
        mManager.addOrUpdateItem(data.filePath);

    }

    @Override
    public Thumbnail getThumbnail(ThumbType thumbType) {
        Log.d(TAG, "<getThumbnail> caption = " + mMediaData.caption + " mRealItem = "
                + mRealItem);
        if (mRealItem == null || mMediaData.filePath == null || mMediaData.filePath.equals("")) {
            Log.d(TAG, "<getThumbnail> mRealItem == null, return");
            return new Thumbnail(null, false);
        }
        Bitmap bitmap = null;
        if (DrmHelper.isDataProtectionFile(mMediaData.filePath)) {
            bitmap = getCTAThumbNail(thumbType);
        } else {
            bitmap = getDrmThumbnail(thumbType);
        }
        if (bitmap == null) {
            Log.d(TAG, "<getThumbnail> bitmap == null, return");
            return new Thumbnail(null, false);
        }
        bitmap = BitmapUtils.ensureGLCompatibleBitmap(bitmap);
        switch (thumbType) {
            case MICRO:
            case MIDDLE:
            case HIGHQUALITY:
            case FANCY:
                return new Thumbnail(bitmap, bitmap == null && mMediaData.isVideo, !mHasRightsToDecode);
            default:
                Log.d(TAG, "<getThumbnail> invalid thumb type " + thumbType + ", return");
                return new Thumbnail(null, false);
        }
    }

    @Override
    public ArrayList<SupportOperation> getSupportedOperations() {
        ArrayList<SupportOperation> res = new ArrayList<SupportOperation>();
        if ((!mMediaData.isVideo) && ACTION_WALLPAPER > 0 && checkRightsStatus(ACTION_WALLPAPER)) {
            res.add(SupportOperation.SETAS);
        }
        if (checkRightsStatus(DrmStore.Action.TRANSFER)) {
            res.add(SupportOperation.SHARE);
        }
        if ((!mMediaData.isVideo) && ACTION_PRINT > 0 && checkRightsStatus(ACTION_PRINT)) {
            res.add(SupportOperation.PRINT);
        }
        return res;
    }

    @Override
    public ArrayList<SupportOperation> getNotSupportedOperations() {
        ArrayList<SupportOperation> res = new ArrayList<SupportOperation>();
        res.add(SupportOperation.FULL_IMAGE);
        res.add(SupportOperation.EDIT);
        res.add(SupportOperation.CROP);
        res.add(SupportOperation.ROTATE);
        res.add(SupportOperation.MUTE);
        res.add(SupportOperation.TRIM);
        if (mMediaData.isVideo || ACTION_WALLPAPER < 0 || !checkRightsStatus(ACTION_WALLPAPER)) {
            res.add(SupportOperation.SETAS);
        }
        if (!checkRightsStatus(DrmStore.Action.TRANSFER)) {
            res.add(SupportOperation.SHARE);
        }
        if (mMediaData.isVideo || ACTION_PRINT < 0 || !checkRightsStatus(ACTION_PRINT)) {
            res.add(SupportOperation.PRINT);
        }
        return res;
    }

    @Override
    public boolean isNeedToCacheThumb(ThumbType thumbType) {
        return false;
    }

    @Override
    public boolean isNeedToGetThumbFromCache(ThumbType thumbType) {
        return false;
    }

    /**
     * Decode bitmap by thumb type.
     *
     * @param thumbType  The thumb type.
     * @return The bitmap for drm texture.
     */
    public Bitmap getDrmThumbnail(ThumbType thumbType) {
        int targetSize = getDrmTargeSize(thumbType);
        Log.d(TAG, "<getDrmThumbnail> filePath" + mMediaData.filePath
                + " targetSize = " + targetSize);
        Bitmap bitmap = getDrmBitmap(targetSize);
        if (thumbType == ThumbType.MICRO) {
            return BitmapUtils.resizeAndCropCenter(bitmap, targetSize, true);
        } else {
            return BitmapUtils.resizeDownBySideLength(bitmap, targetSize, true);
        }
    }

    @Override
    public Bitmap decodeBitmap(BitmapFactory.Options options) {
        if (mRealItem == null) {
            Bitmap bitmap = super.decodeBitmap(options);
            mManager.addOrUpdateItem(mMediaData.filePath);
            return bitmap;
        }
        Bitmap bitmap = mRealItem.decodeBitmap(options);
        mManager.addOrUpdateItem(mMediaData.filePath);

        return bitmap;
    }

    @Override
    public boolean supportHighQuality() {
        return false;
    }

    @Override
    public void decodeBounds() {
        if (mMediaData.width <= 0 || mMediaData.height <= 0) {
            if (mRealItem == null) {
                super.decodeBounds();
            } else {
                mRealItem.decodeBounds();
                mWidth = mRealItem.getWidth();
                mHeight = mRealItem.getHeight();
            }
        }

    }

    @Override
    public int getWidth() {
        if (super.getWidth() <= 0) {
            decodeBounds();
        }
        return super.getWidth() > 0 ? super.getWidth() : sTargetSize;
    }

    @Override
    public int getHeight() {
        if (super.getHeight() <= 0) {
            decodeBounds();
        }
        return super.getHeight() > 0 ? super.getHeight() : sTargetSize;
    }

    private boolean checkRightsStatus(int action) {
        Boolean value = mSupportedOperations.get(action);
        if (value == null) {
            if (mMediaData.filePath != null && !mMediaData.filePath.equals("")) {
                boolean right =
                        DrmHelper.checkRightsStatus(mContext, mMediaData.filePath, action);
                mSupportedOperations.put(action, right);
                return right;
            } else {
                return false;
            }
        } else {
            return value.booleanValue();
        }
    }

    private int getDrmTargeSize(ThumbType thumbType) {
        switch (thumbType) {
            case FANCY:
                return thumbType.getTargetSize();
            case MICRO:
            case MIDDLE:
            case HIGHQUALITY:
                return sTargetSize;
            default:
                return -1;
        }
    }

    private Bitmap getDrmBitmap(int targetSize) {
        int width = getWidth();
        int height = getHeight();
        BitmapFactory.Options options = new BitmapFactory.Options();
        if (targetSize != -1 && (width > 0 && height > 0)) {
            options.inSampleSize =
                    BitmapUtils.computeSampleSizeLarger(width, height, targetSize);
        }
        options.inSampleSize |= FLAG_DRM;
        Bitmap bitmap = decodeBitmap(options);
        if (bitmap == null) {
            mHasRightsToDecode = false;
            if (width > 0 && height > 0) {
                bitmap = createBackground(width, height);
            } else {
                bitmap = createBackground(targetSize, targetSize);
            }
        } else {
            mHasRightsToDecode = true;
            bitmap = BitmapUtils.replaceBackgroundColor(bitmap, true);

        }
        Log.d(TAG, "<getDrmBitmap> mHasRightsToDecode = " + mHasRightsToDecode);
        return bitmap;
    }

    private Bitmap createBackground(int width, int height) {
        Bitmap bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        Canvas canvas = new Canvas(bitmap);
        canvas.drawColor(Color.parseColor(DrmHelper.PLACE_HOLDER_COLOR));
        return bitmap;
    }

    private Bitmap getCTAThumbNail(ThumbType thumbType) {
        int targetSize = getDrmTargeSize(thumbType);
        Log.d(TAG, "<getCTAThumbNail> mMediaData " + mMediaData.filePath + " targetSize = "
                + targetSize);
        int width = getWidth();
        int height = getHeight();
        Bitmap bitmap = null;
        if (width > 0 && height > 0) {
            bitmap = createBackground(width, height);
        } else {
            bitmap = createBackground(targetSize, targetSize);
        }
        return bitmap;
    }

    private int getAction(String Attribute) {
        int value = -1;
        try {
            Field clazzField = DrmStore.Action.class.getField(Attribute);
            if (clazzField != null) {
                value = clazzField.getInt(null);
            }
        } catch (NoSuchFieldException e1) {
            Log.e(TAG, "NoSuchFieldException: " + e1);
        } catch (IllegalAccessException e) {
            Log.e(TAG, "IllegalAccessException: " + e);
        }
        return value;
    }

    public void registerListener(DataChangeListener listener) {
        mListener = listener;
        Log.v(TAG, "<DrmItem>registerListener,listener:" + listener);

        DrmRightsManager.RightsChangeListener rightsChangelistener =
                new DrmRightsManager.RightsChangeListener() {
            @Override
            public void onRightsChange(ArrayList<String> updateContentList) {
                if (mListener != null) {
                    mListener.onExtItemDataChange(updateContentList);
                }
            }
        };

        mManager.registerRightsChangeListener(mMediaData.filePath, rightsChangelistener);
    }

    public void unRegisterListener(DataChangeListener listener) {
        mListener = null;
    }

}
