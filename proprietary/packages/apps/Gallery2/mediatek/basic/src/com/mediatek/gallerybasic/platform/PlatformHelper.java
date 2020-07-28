package com.mediatek.gallerybasic.platform;


import com.mediatek.gallerybasic.base.MediaData;
import com.mediatek.gallerybasic.base.Work;

/**
 * Making a bridge which connect google packages and mediatek packages.
 * In mediatek feature, we can not access google packages directly for easy
 * migration purpose, so google packages implement this
 * interface(PlatformImpl.java) and register it, then mediatek feature could
 * access google packages by PlatformImpl.
 */
public class PlatformHelper {
    private static final String TAG = "MtkGallery2/PlatformHelper";
    private static final float SCALE_LIMIT = 4.0f; // Same as setting in PositionController
    private static Platform sPlatform;

    public static void setPlatform(Platform platform) {
        sPlatform = platform;
    }

    /**
     * Check if the photo is out of decoding spec.
     * @param fileSize size of this photo, in bytes
     * @param width    width of this photo
     * @param height   height of this photo
     * @param mimeType mime type of this photo
     * @return         true if out of spec, otherwise false
     */
    public static boolean isOutOfDecodeSpec(long fileSize, int width, int height, String mimeType) {
        if (sPlatform != null) {
            return sPlatform.isOutOfDecodeSpec(fileSize, width, height, mimeType);
        } else {
            return false;
        }
    }

    /**
     * Submit job to Gallery thread pool.
     * @param work job that has been wrapped as work
     */
    public static void submitJob(Work work) {
        if (sPlatform != null) {
            sPlatform.submitJob(work);
        }
    }

    /**
     * Get the min scale limit for one media data when show full image.
     *
     * @param data The current media data
     * @return The scale limit
     */
    public static float getMinScaleLimit(MediaData data) {
        if (sPlatform != null) {
            return sPlatform.getMinScaleLimit(data);
        } else {
            return SCALE_LIMIT;
        }
    }
}