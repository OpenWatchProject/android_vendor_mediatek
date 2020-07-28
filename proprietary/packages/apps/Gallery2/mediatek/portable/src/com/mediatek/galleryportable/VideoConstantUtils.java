package com.mediatek.galleryportable;

import android.media.MediaPlayer;
import android.os.SystemClock;

public class VideoConstantUtils {

    private static final String TAG = "VP_VideoConstantUtils";
    private static final boolean DEBUG = false;

    public static final int MEDIA_INFO_VIDEO_NOT_SUPPORTED = 860;
    public static final int MEDIA_INFO_AUDIO_NOT_SUPPORTED = 862;
    public static final String ORIENTATION = "orientation";
    public static final String IS_DRM = "is_drm";

    private static boolean sIsConstantExisted = false;
    private static boolean sHasChecked = false;

    private static boolean isConstantExisted() {
        if (!sHasChecked) {
            long checkStart = SystemClock.elapsedRealtime();
            try {
                MediaPlayer.class.getDeclaredField("MEDIA_INFO_VIDEO_NOT_PLAYING");
                MediaPlayer.class.getDeclaredField("MEDIA_INFO_AUDIO_NOT_PLAYING");
                sIsConstantExisted = true;
            } catch (NoSuchFieldException e) {
                sIsConstantExisted = false;
            }
            sHasChecked = true;
            Log.d(TAG, "isConstantExisted, sIsConstantExisted = " + sIsConstantExisted);
            if (DEBUG) {
                Log.d(TAG, "isConstantExisted elapsed time = "
                        + (SystemClock.elapsedRealtime() - checkStart));
            }
        }
        return sIsConstantExisted;
    }

    public static int get(int key) {
        int value = -1;
        switch (key) {
            case MEDIA_INFO_VIDEO_NOT_SUPPORTED:
                if (isConstantExisted()) {
                    value = MediaPlayer.MEDIA_INFO_VIDEO_NOT_PLAYING;
                } else {
                    value = MEDIA_INFO_VIDEO_NOT_SUPPORTED;
                }
                break;
            case MEDIA_INFO_AUDIO_NOT_SUPPORTED:
                if (isConstantExisted()) {
                    value = MediaPlayer.MEDIA_INFO_AUDIO_NOT_PLAYING;
                } else {
                    value = MEDIA_INFO_AUDIO_NOT_SUPPORTED;
                }
                break;
        }
        Log.d(TAG, "get key = " + key + ", value = " + value);
        return value;
    }

    public static String get(String key) {
        String value = null;
        if (key.equals(ORIENTATION)) {
            value = ORIENTATION;
        } else if (key.equals(IS_DRM)) {
            value = IS_DRM;
        }
        Log.d(TAG, "get key = " + key + ", value = " + value);
        return value;
    }
}
