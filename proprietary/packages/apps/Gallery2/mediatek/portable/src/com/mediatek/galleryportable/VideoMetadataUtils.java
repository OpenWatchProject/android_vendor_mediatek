package com.mediatek.galleryportable;

import android.media.MediaPlayer;
import android.media.Metadata;
import android.os.SystemClock;

public class VideoMetadataUtils {

    private static final String TAG = "VP_VideoMetadataUtils";
    private static final boolean DEBUG = true;

    private static boolean sIsMetadataSupported = false;
    private static boolean sHasChecked = false;

    private static boolean isMetadataSupported() {
        if (!sHasChecked) {
            try {
                Class<?> clazz =
                        VideoMetadataUtils.class.getClassLoader().loadClass(
                                "android.media.Metadata");
                sIsMetadataSupported = true;
            } catch (ClassNotFoundException e) {
                sIsMetadataSupported = false;
            }
            sHasChecked = true;
            Log.d(TAG, "isMetadataSupported = " + sIsMetadataSupported);
        }
        return sIsMetadataSupported;
    }

    public static boolean canPause(MediaPlayer mp) {
        boolean canPause = true;
        if (isMetadataSupported()) {
            long checkStart = SystemClock.elapsedRealtime();
            final Metadata data = mp.getMetadata(MediaPlayer.METADATA_ALL,
                    MediaPlayer.BYPASS_METADATA_FILTER);
            if (data != null) {
                canPause = !data.has(Metadata.PAUSE_AVAILABLE)
                        || data.getBoolean(Metadata.PAUSE_AVAILABLE);
                Log.d(TAG, "canPause, data.has(Metadata.PAUSE_AVAILABLE) = "
                        + data.has(Metadata.PAUSE_AVAILABLE) +
                        ", data.getBoolean(Metadata.PAUSE_AVAILABLE) = "
                        + data.getBoolean(Metadata.PAUSE_AVAILABLE));
            } else {
                canPause = true;
            }
            if (DEBUG) {
                Log.d(TAG, "canPause, getMetadata elapsed time = "
                        + (SystemClock.elapsedRealtime() - checkStart));
            }
        }
        return canPause;
    }

    public static boolean canSeekBack(MediaPlayer mp) {
        boolean canSeekBack = true;
        if (isMetadataSupported()) {
            long checkStart = SystemClock.elapsedRealtime();
            final Metadata data = mp.getMetadata(MediaPlayer.METADATA_ALL,
                    MediaPlayer.BYPASS_METADATA_FILTER);
            if (data != null) {
                canSeekBack = !data.has(Metadata.SEEK_BACKWARD_AVAILABLE)
                        || data.getBoolean(Metadata.SEEK_BACKWARD_AVAILABLE);
                Log.d(TAG, "canSeekBack, data.has(Metadata.SEEK_BACKWARD_AVAILABLE) = "
                        + data.has(Metadata.SEEK_BACKWARD_AVAILABLE) +
                        ", data.getBoolean(Metadata.SEEK_BACKWARD_AVAILABLE) = "
                        + data.getBoolean(Metadata.SEEK_BACKWARD_AVAILABLE));
            } else {
                canSeekBack = true;
            }
            if (DEBUG) {
                Log.d(TAG, "canSeekBack, getMetadata elapsed time = "
                        + (SystemClock.elapsedRealtime() - checkStart));
            }
        }
        return canSeekBack;
    }

    public static boolean canSeekForward(MediaPlayer mp) {
        boolean canSeekForward = true;
        if (isMetadataSupported()) {
            long checkStart = SystemClock.elapsedRealtime();
            final Metadata data = mp.getMetadata(MediaPlayer.METADATA_ALL,
                    MediaPlayer.BYPASS_METADATA_FILTER);
            if (data != null) {
                canSeekForward = !data.has(Metadata.SEEK_FORWARD_AVAILABLE)
                        || data.getBoolean(Metadata.SEEK_FORWARD_AVAILABLE);
                Log.d(TAG, "canSeekBack, data.has(Metadata.SEEK_FORWARD_AVAILABLE) = "
                        + data.has(Metadata.SEEK_FORWARD_AVAILABLE) +
                        ", data.getBoolean(Metadata.SEEK_FORWARD_AVAILABLE) = "
                        + data.getBoolean(Metadata.SEEK_FORWARD_AVAILABLE));
            } else {
                canSeekForward = true;
            }
            if (DEBUG) {
                Log.d(TAG, "canSeekForward, getMetadata elapsed time = "
                        + (SystemClock.elapsedRealtime() - checkStart));
            }
        }
        return canSeekForward;
    }

    public static byte[] getAlbumArt(MediaPlayer mp) {
        byte[] albumArt = null;
        if (isMetadataSupported()) {
            long checkStart = SystemClock.elapsedRealtime();
            final Metadata data = mp.getMetadata(MediaPlayer.METADATA_ALL,
                    MediaPlayer.BYPASS_METADATA_FILTER);
            if (data != null && data.has(Metadata.ALBUM_ART)) {
                albumArt = data.getByteArray(Metadata.ALBUM_ART);
            }
            if (DEBUG) {
                Log.d(TAG, "getAlbumArt, getMetadata elapsed time = "
                        + (SystemClock.elapsedRealtime() - checkStart));
            }
        }
        return albumArt;
    }
}
