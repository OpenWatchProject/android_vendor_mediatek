package com.mediatek.camera.tests.v3.operator;

import android.hardware.Camera;
import android.hardware.camera2.CameraCharacteristics;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory;
import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.tests.v3.arch.TestContext;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

public class SceneModeOperator extends SettingRadioButtonOperator {
    private static final LogUtil.Tag TAG = Utils.getTestTag(
            SceneModeOperator.class.getSimpleName());
    public static final int INDEX_OFF = 0;
    public static final int INDEX_AUTO = 1;
    public static final int INDEX_NIGHT = 2;
    public static final int INDEX_SUNSET = 3;
    public static final int INDEX_PARTY = 4;
    public static final int INDEX_PORTRAIT = 5;
    public static final int INDEX_LANDSCAPE = 6;
    public static final int INDEX_NIGHT_PORTRAIT = 7;
    public static final int INDEX_THEATRE = 8;
    public static final int INDEX_BEACH = 9;
    public static final int INDEX_SNOW = 10;
    public static final int INDEX_STEADY_PHOTO = 11;
    public static final int INDEX_FIREWORKS = 12;
    public static final int INDEX_SPORTS = 13;
    public static final int INDEX_CANDLE_LIGHTS = 14;

    private static final String[] OPTIONS = {
            "Off",
            "Auto",
            "Night",
            "Sunset",
            "Party",
            "Portrait",
            "Landscape",
            "Night portrait",
            "Theatre",
            "Beach",
            "Snow",
            "Steady photo",
            "Fireworks",
            "Sports",
            "Candle light"
    };

    @Override
    protected int getSettingOptionsCount() {
        return OPTIONS.length;
    }

    @Override
    protected String getSettingTitle() {
        return "Scene mode";
    }

    @Override
    protected String getSettingOptionTitle(int index) {
        return OPTIONS[index];
    }

    @Override
    public boolean isSupported(int index) {
        if (index != INDEX_NIGHT) {
            return super.isSupported(index);
        } else {
            // Since pre-sanity case will set scene mode as night,
            // check night scene mode supported status by parameters.
            List<String> supportedModes;
            if (CameraApiHelper.getCameraApiType(null)
                    == CameraDeviceManagerFactory.CameraApi.API1) {
                Camera.Parameters parameters =
                        TestContext.mLatestCameraFacing == Camera.CameraInfo.CAMERA_FACING_BACK
                                ? TestContext.mBackCameraParameters
                                : TestContext.mFrontCameraParameters;
                supportedModes = parameters.getSupportedSceneModes();
                LogHelper.d(TAG, "[isSupported] API1, TestContext.mLatestCameraFacing = "
                        + TestContext.mLatestCameraFacing);
            } else {
                CameraCharacteristics characteristics =
                        TestContext.mLatestCameraFacing == Camera.CameraInfo.CAMERA_FACING_BACK
                                ? TestContext.mBackCameraCharacteristics
                                : TestContext.mFrontCameraCharacteristics;
                int[] scenes = characteristics.get(
                        CameraCharacteristics.CONTROL_AVAILABLE_SCENE_MODES);
                supportedModes = convertEnumToString(scenes);
                LogHelper.d(TAG, "[isSupported] API2, TestContext.mLatestCameraFacing = "
                        + TestContext.mLatestCameraFacing);
            }
            for (String mode : supportedModes) {
                if ("night".equalsIgnoreCase(mode)) {
                    LogHelper.d(TAG, "[isSupported] return true for night mode");
                    return true;
                }
            }
            LogHelper.d(TAG, "[isSupported] return false for night mode");
            for (String mode : supportedModes) {
                LogHelper.d(TAG, "[isSupported] support mode contains " + mode);
            }
            return false;
        }
    }

    private List<String> convertEnumToString(int[] enumIndexs) {
        ModeEnum[] modes = ModeEnum.values();
        List<String> names = new ArrayList<>(enumIndexs.length);
        for (int i = 0; i < enumIndexs.length; i++) {
            int enumIndex = enumIndexs[i];
            for (ModeEnum mode : modes) {
                if (mode.getValue() == enumIndex) {
                    String name = mode.getName().replace('_', '-').toLowerCase(Locale.ENGLISH);
                    names.add(name);
                    break;
                }
            }
        }
        return names;
    }

    enum ModeEnum {
        OFF(0),
        FACE_PORTRAIT(1),
        ACTION(2),
        PORTRAIT(3),
        LANDSCAPE(4),
        NIGHT(5),
        NIGHT_PORTRAIT(6),
        THEATRE(7),
        BEACH(8),
        SNOW(9),
        SUNSET(10),
        STEADYPHOTO(11),
        FIREWORKS(12),
        SPORTS(13),
        PARTY(14),
        CANDLELIGHT(15),
        BARCODE(16),
        HIGH_SPEED_VIDEO(17),
        HDR(18),
        BACKLIGHT_PORTRAIT(32);

        private int mValue = 0;

        ModeEnum(int value) {
            this.mValue = value;
        }

        /**
         * Get enum value which is in integer.
         *
         * @return The enum value.
         */
        public int getValue() {
            return this.mValue;
        }

        /**
         * Get enum name which is in string.
         *
         * @return The enum name.
         */
        public String getName() {
            return this.toString();
        }
    }
}
