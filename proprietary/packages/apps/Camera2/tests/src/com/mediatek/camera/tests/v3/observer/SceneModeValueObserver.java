package com.mediatek.camera.tests.v3.observer;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory;
import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

import java.util.Locale;

/**
 * Observer the scene mode value is set right to MW by Ap layer or not.
 */

public class SceneModeValueObserver extends AbstractLogObserver {
    private static final LogUtil.Tag TAG = Utils.getTestTag(
            SceneModeValueObserver.class.getSimpleName());

    private static final String LOG_TAG_FOR_API1 = "MtkCam/ParamsManager";
    private static final String KEY_STRING_FOR_API1 = "scene-mode=";
    private static final String LOG_TAG_FOR_ASD = "CamAp_SceneMode";

    private static final String LOG_TAG_FOR_API2 = "MtkCam/MetadataConverter";
    private static final String KEY_STRING_FOR_API2 = "android.control.sceneMode";
    private static final String KEY_STRING_FOR_ASD = "onSceneDetected";

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

    private static final String[] SCENE_MODE_NAME = {
            "off",
            "auto",
            "night",
            "sunset",
            "party",
            "portrait",
            "landscape",
            "night-portrait",
            "theatre",
            "beach",
            "snow",
            "steadyphoto",
            "fireworks",
            "sports",
            "candlelight"
    };

    /**
     * Scene mode enum value.
     */
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

    private boolean mPass = false;

    @Override
    public int getObserveCount() {
        return SCENE_MODE_NAME.length;
    }

    @Override
    public String getDescription(int index) {
        if (INDEX_AUTO == index) {
            return "check capture mode is asd";
        } else {
            return "check scene mode is " + SCENE_MODE_NAME[index];
        }
    }

    @Override
    protected String[] getObservedTagList(int index) {
        if (INDEX_AUTO == index) {
            return new String[]{LOG_TAG_FOR_ASD};
        }

        if (CameraApiHelper.getCameraApiType(null)
                == CameraDeviceManagerFactory.CameraApi.API1) {
            return new String[]{LOG_TAG_FOR_API1};
        } else if (CameraApiHelper.getCameraApiType(null)
                == CameraDeviceManagerFactory.CameraApi.API2) {
            return new String[]{LOG_TAG_FOR_API2};
        }
        return null;
    }

    @Override
    protected String[] getObservedKeyList(int index) {
        if (INDEX_AUTO == index) {
            return new String[]{KEY_STRING_FOR_ASD};
        }

        if (CameraApiHelper.getCameraApiType(null)
                == CameraDeviceManagerFactory.CameraApi.API1) {
            return new String[]{KEY_STRING_FOR_API1};
        } else if (CameraApiHelper.getCameraApiType(null)
                == CameraDeviceManagerFactory.CameraApi.API2) {
            return new String[]{KEY_STRING_FOR_API2};
        }
        return null;
    }

    @Override
    protected void onLogComing(int index, String line) {
        if (INDEX_AUTO == index) {
            // if there are [onSceneDetected] log print, it means ASD is worked.
            mPass = true;
            return;
        }

        if (CameraApiHelper.getCameraApiType(null)
                == CameraDeviceManagerFactory.CameraApi.API1) {
            int beginIndex = line.indexOf(KEY_STRING_FOR_API1) + KEY_STRING_FOR_API1.length();
            int endIndex = line.indexOf(";", beginIndex);
            String subString = line.substring(beginIndex, endIndex);
            LogHelper.d(TAG, "[onLogComing], scene mode=" + subString);
            if (index == 0) {
                mPass = "auto".equals(subString);
            } else {
                mPass = SCENE_MODE_NAME[index].equals(subString);
            }
        } else if (CameraApiHelper.getCameraApiType(null)
                == CameraDeviceManagerFactory.CameraApi.API2) {
            int beginIndex = line.indexOf("=> ") + "=> ".length();
            String subString = line.substring(beginIndex, line.length() - 1);
            String effectName = convertEnumToString(Integer.parseInt(subString));
            LogHelper.d(TAG, "[onLogComing], android.control.sceneMode="
                    + subString + "(" + effectName + ")");
            mPass = SCENE_MODE_NAME[index].equals(effectName);
        }
    }

    @Override
    protected void onObserveEnd(int index) {
        Utils.assertRightNow(mPass);
    }

    @Override
    protected void onObserveBegin(int index) {
        mPass = false;
    }

    @Override
    protected boolean isAlreadyFindTarget(int index) {
        return mPass;
    }

    @Override
    protected void applyAdbCommandBeforeStarted() {
        if (CameraApiHelper.getCameraApiType(null)
                == CameraDeviceManagerFactory.CameraApi.API2) {
            setAdbCommand("debug.camera.log.AppStreamMgr", "3");
        }
    }

    @Override
    protected void clearAdbCommandAfterStopped() {
        if (CameraApiHelper.getCameraApiType(null)
                == CameraDeviceManagerFactory.CameraApi.API2) {
            setAdbCommand("debug.camera.log.AppStreamMgr", "0");
        }
    }

    private String convertEnumToString(int enumIndex) {
        String name = null;
        ModeEnum[] modes = ModeEnum.values();
        for (ModeEnum mode : modes) {
            if (mode.getValue() == enumIndex) {
                name = mode.getName().replace('_', '-').toLowerCase(Locale.ENGLISH);
                break;
            }
        }
        return name;
    }
}
