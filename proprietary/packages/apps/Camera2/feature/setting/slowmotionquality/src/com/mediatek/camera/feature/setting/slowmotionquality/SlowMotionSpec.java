package com.mediatek.camera.feature.setting.slowmotionquality;

import android.media.CamcorderProfile;
import android.support.annotation.NonNull;
import android.util.Range;

public class SlowMotionSpec implements Comparable<SlowMotionSpec> {
    @Override
    public String toString() {
        return "SlowMotionSpec{" +
                "meta=" + meta +
                ", pattern=" + pattern +
                ", fpsRangeForPreview=" + fpsRangeForPreview +
                ", fpsRangeForRecording=" + fpsRangeForRecording +
                ", fpsRangeForBurstRecording=" + fpsRangeForBurstRecording +
                ", profile=" + profile +
                ", profileIndex=" + profileIndex +
                ", cameraId=" + cameraId +
                ", isVssSupported=" + isVssSupported +
                ", isBurstSupported=" + isBurstSupported +
                '}';
    }

    @Override
    public int compareTo(@NonNull SlowMotionSpec slowMotionSpec) {
        if (pattern != slowMotionSpec.pattern) {
            return pattern.compareTo(slowMotionSpec.pattern) > 0 ? 1 : -1;
        }
        if (meta.fpsMax != slowMotionSpec.meta.fpsMax) {
            return meta.fpsMax > slowMotionSpec.meta.fpsMax ? 1 : -1;
        }
        if (meta.width * meta.height != slowMotionSpec.meta.width * slowMotionSpec.meta.height) {
            return meta.width * meta.height > slowMotionSpec.meta.width * slowMotionSpec.meta.height
                    ? 1 : -1;
        }
        return 0;
    }

    public enum Pattern {
        NORMAL,
        BURST,
        CONTINUOUS
    }

    public MetaData meta;
    public Pattern pattern;
    public Range<Integer> fpsRangeForPreview;
    public Range<Integer> fpsRangeForRecording;
    public Range<Integer> fpsRangeForBurstRecording;
    public CamcorderProfile profile;
    public int profileIndex;
    public int cameraId;
    public boolean isVssSupported = false;
    public boolean isBurstSupported = false;

    static public class MetaData {
        static final int DATA_NUM = 4;
        public int width;
        public int height;
        public int fpsMax;
        public int cusP2BatchSize;

        public int[] toRequestParams() {
            int[] res = new int[2];
            res[0] = fpsMax;
            res[1] = cusP2BatchSize;
            return res;
        }

        @Override
        public String toString() {
            return "MetaData{" +
                    "width=" + width +
                    ", height=" + height +
                    ", fpsMax=" + fpsMax +
                    ", cusP2BatchSize=" + cusP2BatchSize +
                    '}';
        }
    }
}
