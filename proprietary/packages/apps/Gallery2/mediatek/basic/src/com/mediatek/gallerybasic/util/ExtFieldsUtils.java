package com.mediatek.gallerybasic.util;

import com.mediatek.gallerybasic.base.MediaData;

public class ExtFieldsUtils {

    private static final String TAG = "MtkGallery2/ExtFieldsUtils";

    public static final String VIDEO_ROTATION_FIELD = "orientation";
    public static final int INVALID_VIDEO_ROTATION = -1;
    public static final int NOT_SUPPORT_VIDEO_ROTATION = -2;
    /**
     * Get video rotation from media data.
     * @param data media data
     * @return video rotation
     */
    public static int getVideoRotation(MediaData data) {
        if (data == null) {
            Log.d(TAG, "<getVideoRotation> data is null, return " + INVALID_VIDEO_ROTATION);
            return INVALID_VIDEO_ROTATION;
        }
        if (data.extFileds == null) {
            Log.d(TAG, "<getVideoRotation> extFileds is null, return " + INVALID_VIDEO_ROTATION);
            return INVALID_VIDEO_ROTATION;
        }
        Object value = data.extFileds.getVideoField(VIDEO_ROTATION_FIELD);
        if (value == null) {
            Log.d(TAG, "<getVideoRotation> not support video rotation, return "
                    + NOT_SUPPORT_VIDEO_ROTATION);
            return NOT_SUPPORT_VIDEO_ROTATION;
        }
        if (value instanceof Integer) {
            int rotation = ((Integer) value).intValue();
            Log.d(TAG, "<getVideoRotation> file: " + data.filePath + ", rotation: " + rotation);
            return rotation;
        }
        Log.d(TAG, "<getVideoRotation> incorrect format, return " + INVALID_VIDEO_ROTATION);
        return INVALID_VIDEO_ROTATION;
    }
}
