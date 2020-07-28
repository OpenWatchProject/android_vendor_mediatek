package com.mediatek.galleryfeature.picturequality;

import android.graphics.BitmapFactory;
import android.graphics.BitmapFactory.Options;

import com.mediatek.gallerybasic.base.IDecodeOptionsProcessor;
import com.mediatek.gallerybasic.util.Log;

import java.lang.reflect.Field;

/**
 * Add pq process.
 */
public class PictureQuality implements IDecodeOptionsProcessor {
    private final static String TAG = "MtkGallery/PictureQuality";
    // Picture quality enhancement feature avails Camera ISP hardware
    // to improve image quality displayed on the screen.
    public static final boolean SUPPORT_PICTURE_QUALITY_ENHANCE = true;
    public static final String MIME_TYPE_JPEG = "image/jpeg";
    public static boolean mHasInitializedField;
    public static Field mField = null;

    private boolean supportPQEnhance(String mimeType) {
        return (SUPPORT_PICTURE_QUALITY_ENHANCE && MIME_TYPE_JPEG.equals(mimeType));
    }

    @Override
    public boolean processThumbDecodeOptions(String mimeType, Options options) {
        initOptions(mimeType, options);
        return true;
    }

    @Override
    public boolean processRegionDecodeOptions(String mimeType, Options options) {
        initOptions(mimeType, options);
        return true;
    }

    /**
     * set inPostProc.
     *
     * @param option The decode option.
     */
    private void initOptions(Options option) {
        if (!mHasInitializedField) {
            try {
                Class<? extends BitmapFactory.Options> clazz = option.getClass();
                mField = clazz.getField("inPostProc");
            } catch (NoSuchFieldException e1) {
                Log.e(TAG, "NoSuchFieldException: " + e1);
            }
            mHasInitializedField = true;
        }
        if (mHasInitializedField) {
            if (mField != null) {
                try {
                    mField.set(option, true);
                    Log.v(TAG, "<initOptions> inPostPro = " + mField.getBoolean(option));
                } catch (IllegalAccessException e) {
                    Log.e(TAG, "IllegalAccessException: " + e);
                } catch (IllegalArgumentException e) {
                    Log.e(TAG, "IllegalArgumentException: " + e);
                }
            }
        }
    }

    /**
     * set inPostProc.
     *
     * @param mimeType the image mimetype
     * @param option   The decode option.
     */
    private void initOptions(String mimeType, Options option) {
        if (supportPQEnhance(mimeType)) {
            initOptions(option);
        }
    }
}
