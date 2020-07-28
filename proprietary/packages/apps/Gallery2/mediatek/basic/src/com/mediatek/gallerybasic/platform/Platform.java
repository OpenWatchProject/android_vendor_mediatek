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
public interface Platform {
    /**
     * Check if the photo is out of decoding spec.
     *
     * @param fileSize size of this photo, in bytes
     * @param width    width of this photo
     * @param height   height of this photo
     * @param mimeType mime type of this photo
     * @return true if out of spec, otherwise false
     */
    public boolean isOutOfDecodeSpec(long fileSize, int width, int height, String mimeType);

    /**
     * Submit job to Gallery thread pool.
     *
     * @param work job that has been wrapped as work
     */
    public void submitJob(Work work);

    /**
     * Get the min scale limit for one media data when show full image.
     *
     * @param data The current media data
     * @return The scale limit
     */
    public float getMinScaleLimit(MediaData data);
}