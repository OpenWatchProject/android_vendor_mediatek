package com.mediatek.camera.feature.setting.slowmotionquality;

import android.app.Activity;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.media.CamcorderProfile;
import android.util.Range;

import com.mediatek.camera.R;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.loader.DeviceDescription;
import com.mediatek.camera.portability.CamcorderProfileEx;
import com.mediatek.camera.portability.SystemProperties;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;

public class SlowMotionQualityHelper {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(
            SlowMotionQualityHelper.class.getSimpleName());

    private static final int SLOW_MOTION_QUALITY_HIGH = 2222;
    private static final int SLOW_MOTION_QUALITY_LOW = 2220;

    private static final int[] SLOW_MOTION_PROFILE_INDEX = new int[]{
            SLOW_MOTION_QUALITY_HIGH,
            SLOW_MOTION_QUALITY_LOW,
            CamcorderProfile.QUALITY_HIGH_SPEED_2160P,
            CamcorderProfile.QUALITY_HIGH_SPEED_1080P,
            CamcorderProfile.QUALITY_HIGH_SPEED_720P,
            CamcorderProfile.QUALITY_HIGH_SPEED_480P,
    };

    private static final int QUALITY_FHD_1 = 1920 * 1080;
    private static final int QUALITY_FHD_2 = 1920 * 1088;
    private static final int QUALITY_HD = 1280 * 720;
    private static final int QUALITY_VGA_1 = 640 * 480;
    private static final int QUALITY_VGA_2 = 720 * 480;

    enum Quality {
        FHD,
        HD,
        VGA
    }

    // Integer - camera id
    private static HashMap<Integer, List<SlowMotionSpec>> sSlowMotionQualities
            = new HashMap<>();

    private synchronized static void initSlowMotionQualities(CameraCharacteristics characteristics,
                                                             int cameraId,
                                                             DeviceDescription deviceDescription) {
        LogHelper.i(TAG, "[initSlowMotionQualities] cameraId = " + cameraId + " +");
        if (!sSlowMotionQualities.containsKey(cameraId)) {
            sSlowMotionQualities.put(cameraId, new ArrayList<SlowMotionSpec>());
        }

//        StreamConfigurationMap config =
//                characteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);

        SlowMotionSpec.MetaData[] metadata = getAllSlowMotionMetaData(characteristics,
                deviceDescription);

        for (int profileIndex : SLOW_MOTION_PROFILE_INDEX) {
            // no this profile, continue
            CamcorderProfile profile = null;
            if (!CamcorderProfile.hasProfile(cameraId, profileIndex)) {
                LogHelper.i(TAG, "[initSlowMotionQualities] profile [" + profileIndex + "] "
                        + "no this profile for camera " + cameraId);
                continue;
            } else {
                profile = CamcorderProfileEx.getProfile(cameraId, profileIndex);
            }

//            try {
//                Range<Integer>[] availableFpsRanges = config.getHighSpeedVideoFpsRangesFor(
//                        new Size(profile.videoFrameWidth, profile.videoFrameHeight));
//                // no fps ranges for this size, continue
//                if (availableFpsRanges == null) {
//                    LogHelper.i(TAG, "[initSlowMotionQualities] profile [" + profileIndex + "] "
//                            + "no high speed video fps range for this size, width = "
//                            + profile.videoFrameWidth + ", height = " + profile.videoFrameHeight);
//                    continue;
//                }
//            } catch (IllegalArgumentException e) {
//                LogHelper.i(TAG, "[initSlowMotionQualities] profile [" + profileIndex + "] " +
//                        "not support high speed video recording");
//                continue;
//            }
            List<SlowMotionSpec> specs = getSlowMotionSpec(metadata, profile);
            // not find suitable spec, continue
            if (specs == null || specs.size() <= 0) {
                LogHelper.i(TAG, "[initSlowMotionQualities] profile [" + profileIndex + "] "
                        + "no suitable spec for this size, width = "
                        + profile.videoFrameWidth + ", height = " + profile.videoFrameHeight);
                continue;
            }
            for (SlowMotionSpec spec : specs) {
                spec.cameraId = cameraId;
                spec.profile = profile;
                spec.profileIndex = profileIndex;
                LogHelper.i(TAG, "[initSlowMotionQualities] profile [" + profileIndex + "] "
                        + "add slow motion spec = " + spec);
                sSlowMotionQualities.get(cameraId).add(spec);
            }
        }
        // add AOSP spec
        SlowMotionSpec spec = getNormalSpec(cameraId, characteristics);
        if (spec != null) {
            sSlowMotionQualities.get(cameraId).add(spec);
            LogHelper.i(TAG, "[initSlowMotionQualities] add normal AOSP spec = " + spec);
        }
        // to show spec in order, sorted
        Collections.sort(sSlowMotionQualities.get(cameraId), new Comparator<SlowMotionSpec>() {
            @Override
            public int compare(SlowMotionSpec entry1, SlowMotionSpec entry2) {
                return entry2.compareTo(entry1);
            }
        });
        LogHelper.i(TAG, "[initSlowMotionQualities] cameraId = " + cameraId
                + " -, available quality count = " + sSlowMotionQualities.get(cameraId).size());
    }

    public static boolean isSlowMotionSupported(int cameraId, CameraCharacteristics characteristics,
                                                DeviceDescription deviceDescription) {
        if (!sSlowMotionQualities.containsKey(cameraId)) {
            initSlowMotionQualities(characteristics, cameraId, deviceDescription);
        }
        return sSlowMotionQualities.get(cameraId).size() > 0;
    }

    public static List<String> getAvailableQualities(CameraCharacteristics characteristics,
                                                     int cameraId,
                                                     DeviceDescription deviceDescription) {
        if (!sSlowMotionQualities.containsKey(cameraId)) {
            initSlowMotionQualities(characteristics, cameraId, deviceDescription);
        }

        List<String> res = new ArrayList<String>();
        for (SlowMotionSpec spec : sSlowMotionQualities.get(cameraId)) {
            LogHelper.d(TAG, "[getAvailableQualities] res add " + spec);
            res.add(String.valueOf(sSlowMotionQualities.get(cameraId).indexOf(spec)));
        }
        return res;
    }

    public static SlowMotionSpec getSlowMotionSpec(int cameraId, int quality,
                                                   CameraCharacteristics characteristics,
                                                   DeviceDescription deviceDescription) {
        if (!sSlowMotionQualities.containsKey(cameraId)) {
            initSlowMotionQualities(characteristics, cameraId, deviceDescription);
        }

        return sSlowMotionQualities.get(cameraId).get(quality);
    }

    public static int getNormalSlowMotionQuality(CameraCharacteristics characteristics,
                                                 int cameraId,
                                                 DeviceDescription deviceDescription) {
        if (!sSlowMotionQualities.containsKey(cameraId)) {
            initSlowMotionQualities(characteristics, cameraId, deviceDescription);
        }

        for (SlowMotionSpec spec : sSlowMotionQualities.get(cameraId)) {
            if (spec.pattern == SlowMotionSpec.Pattern.NORMAL) {
                return sSlowMotionQualities.get(cameraId).indexOf(spec);
            }
        }
        return -1;
    }

    public static String getQualityResolution(int cameraId, int quality) {
        SlowMotionSpec spec = sSlowMotionQualities.get(cameraId).get(quality);
        return spec.meta.width + "x" + spec.meta.height;
    }

    public static String getQualityTitle(Activity activity, int cameraId, int quality) {
        SlowMotionSpec spec = sSlowMotionQualities.get(cameraId).get(quality);
        String title = "";
        switch (spec.meta.width * spec.meta.height) {
            case QUALITY_FHD_1:
            case QUALITY_FHD_2:
                title = activity.getResources().getString(R.string.slowmotion_quality_fhd);
                break;
            case QUALITY_HD:
                title = activity.getResources().getString(R.string.slowmotion_quality_hd);
                break;
            case QUALITY_VGA_1:
            case QUALITY_VGA_2:
                title = activity.getResources().getString(R.string.slowmotion_quality_vga);
                break;
            default:
                break;
        }
        title += ", " + spec.meta.fpsMax + "fps";
        return title;
    }

    private static SlowMotionSpec.MetaData[] getAllSlowMotionMetaData(
            CameraCharacteristics characteristics, DeviceDescription deviceDescription) {
        CameraCharacteristics.Key<int[]> key = deviceDescription.getKeySMVRAvailableModes();
        int[] modes;
        if (key == null) {
            LogHelper.i(TAG, "[getAllSlowMotionMetaData] no key, return null");
            return null;
        } else {
            LogHelper.i(TAG, "[getAllSlowMotionMetaData] has key, read from meta data");
            modes = characteristics.get(deviceDescription.getKeySMVRAvailableModes());
        }

        if (modes == null || modes.length == 0
                || modes.length % SlowMotionSpec.MetaData.DATA_NUM != 0) {
            LogHelper.i(TAG, "[getAllSlowMotionMetaData] available modes info in meta data " +
                    "is not right, return null");
            return null;
        }
        int modeNum = modes.length / SlowMotionSpec.MetaData.DATA_NUM;
        SlowMotionSpec.MetaData[] metaDatas = new SlowMotionSpec.MetaData[modeNum];
        for (int i = 0; i < modeNum; i++) {
            metaDatas[i] = new SlowMotionSpec.MetaData();
            metaDatas[i].width = modes[i * SlowMotionSpec.MetaData.DATA_NUM];
            metaDatas[i].height = modes[i * SlowMotionSpec.MetaData.DATA_NUM + 1];
            metaDatas[i].fpsMax = modes[i * SlowMotionSpec.MetaData.DATA_NUM + 2];
            metaDatas[i].cusP2BatchSize = modes[i * SlowMotionSpec.MetaData.DATA_NUM + 3];
            LogHelper.i(TAG, "[getAllSlowMotionMetaData] metaDatas[" + i + "] = "
                    + metaDatas[i]);
        }

        return metaDatas;
    }

    private static SlowMotionSpec getNormalSpec(int cameraId,
                                                CameraCharacteristics characteristics) {
        for (int i = 0; i < SLOW_MOTION_PROFILE_INDEX.length; i++) {
            if (CamcorderProfile.hasProfile(cameraId, SLOW_MOTION_PROFILE_INDEX[i])) {
                CamcorderProfile profile = CamcorderProfileEx.getProfile(cameraId,
                        SLOW_MOTION_PROFILE_INDEX[i]);
                Range<Integer> range = getHighSpeedFixedFpsRangeForSize(new android.util.Size(
                        profile.videoFrameWidth, profile.videoFrameHeight), characteristics);
                if (range != null && range.getLower() == profile.videoFrameRate) {
                    LogHelper.i(TAG, "[getNormalSpec] find profile videoFrameRate = "
                            + profile.videoFrameRate + ", cameraId = " + cameraId + ", size = "
                            + profile.videoFrameWidth + " x" + profile.videoFrameHeight);
                    SlowMotionSpec spec = new SlowMotionSpec();
                    spec.meta = new SlowMotionSpec.MetaData();
                    spec.meta.width = profile.videoFrameWidth;
                    spec.meta.height = profile.videoFrameHeight;
                    spec.meta.fpsMax = profile.videoFrameRate;
                    spec.pattern = SlowMotionSpec.Pattern.NORMAL;
                    spec.fpsRangeForPreview = new Range<>(30, spec.meta.fpsMax);
                    spec.fpsRangeForRecording = new Range<>(spec.meta.fpsMax, spec.meta.fpsMax);
                    spec.fpsRangeForBurstRecording = null;
                    spec.isVssSupported = false;
                    spec.isBurstSupported = false;
                    spec.cameraId = cameraId;
                    spec.profile = profile;
                    spec.profileIndex = SLOW_MOTION_PROFILE_INDEX[i];
                    return spec;
                }
            }
        }
        return null;
    }

    private static Range<Integer> getHighSpeedFixedFpsRangeForSize(
            android.util.Size size, CameraCharacteristics characteristics) {
        StreamConfigurationMap config =
                characteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
        try {
            Range<Integer>[] availableFpsRanges = config.getHighSpeedVideoFpsRangesFor(size);
            for (Range<Integer> range : availableFpsRanges) {
                if (range.getLower().equals(range.getUpper())) {
                    LogHelper.i(TAG, "[getHighSpeedFixedFpsRangeForSize] range = "
                            + range.toString());
                    return range;
                }
            }
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
        }
        return null;
    }

    private static List<SlowMotionSpec> getSlowMotionSpec(SlowMotionSpec.MetaData[] metaData,
                                                    CamcorderProfile profile) {
        if (metaData == null || metaData.length == 0) {
            if (metaData != null) {
                LogHelper.d(TAG, "[getSlowMotionSpec] metaData.length = " + metaData.length
                        + ", return null");
            } else {
                LogHelper.d(TAG, "[getSlowMotionSpec] metaData = " + metaData + ", return null");
            }
            return null;
        }

        List<SlowMotionSpec> specs = new ArrayList<>();
        for (SlowMotionSpec.MetaData meta : metaData) {
            if (meta.width != profile.videoFrameWidth ||
                    meta.height != profile.videoFrameHeight) {
                continue;
            }
            String p2batchString;
            Quality quality = getQuality(profile.videoFrameWidth, profile.videoFrameHeight);
            if (quality == Quality.FHD) {
                p2batchString = "vender.ro.p2batchString.fhd";
            } else if (quality == Quality.HD) {
                p2batchString = "vender.ro.p2batchString.hd";
            } else if (quality == Quality.VGA) {
                p2batchString = "vender.ro.p2batchString.vga";
            } else {
                LogHelper.i(TAG, "[getSlowMotionSpec] not FHD/HD/VGA resolution, continue");
                continue;
            }
            int p2batchSize = SystemProperties.getInt(p2batchString, meta.cusP2BatchSize);
            if (Math.min(meta.cusP2BatchSize, p2batchSize) < meta.fpsMax / 30) {
                SlowMotionSpec spec = new SlowMotionSpec();
                spec.meta = meta;
                spec.pattern = SlowMotionSpec.Pattern.BURST;
                spec.fpsRangeForPreview = new Range<>(30, spec.meta.fpsMax);
                spec.fpsRangeForRecording = new Range<>(30, spec.meta.fpsMax);
                spec.fpsRangeForBurstRecording = new Range<>(spec.meta.fpsMax, spec.meta.fpsMax);
                spec.isVssSupported = false;
                spec.isBurstSupported = true;
                specs.add(spec);
            } else {
                SlowMotionSpec spec = new SlowMotionSpec();
                spec.meta = meta;
                spec.pattern = SlowMotionSpec.Pattern.CONTINUOUS;
                spec.fpsRangeForPreview = new Range<>(30, spec.meta.fpsMax);
                spec.fpsRangeForRecording = new Range<>(spec.meta.fpsMax, spec.meta.fpsMax);
                spec.fpsRangeForBurstRecording = null;
                spec.isVssSupported = true;
                spec.isBurstSupported = false;
                specs.add(spec);
            }
        }
        return specs;
    }

    private static final List<String> convertIntegerToStringList(Object[] array) {
        List<String> res = new ArrayList<>(array.length);
        for (int i = 0; i < array.length; i++) {
            res.add(i, String.valueOf(array[i]));
        }
        return res;
    }

    private static Quality getQuality(int width, int height) {
        switch (width * height) {
            case QUALITY_FHD_1:
            case QUALITY_FHD_2:
                return Quality.FHD;
            case QUALITY_HD:
                return Quality.HD;
            case QUALITY_VGA_1:
            case QUALITY_VGA_2:
                return Quality.VGA;
        }
        return null;
    }
}