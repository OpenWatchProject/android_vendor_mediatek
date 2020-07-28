package com.mediatek.camera.tests.v3.observer;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory;
import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

import java.util.Locale;

/**
 * Class to observer effect parameters.
 */

public class MatrixDisplayEffectObserver extends AbstractLogObserver {
    private static final LogUtil.Tag TAG = Utils.getTestTag(
            MatrixDisplayEffectObserver.class.getSimpleName());

    public static final int INDEX_EFFECT_MODE_NONE = 0;
    public static final int INDEX_EFFECT_MODE_MONO = 1;
    public static final int INDEX_EFFECT_MODE_NEGATIVE = 2;
    public static final int INDEX_EFFECT_MODE_SEPIA = 3;
    public static final int INDEX_EFFECT_MODE_POSTERIZE = 4;
    public static final int INDEX_EFFECT_MODE_AQUA = 5;
    public static final int INDEX_EFFECT_MODE_BLACKBOARD = 6;
    public static final int INDEX_EFFECT_MODE_WHITEBOARD = 7;

    public static final int INDEX_EFFECT_MODE_NASHVILLE = 8;
    public static final int INDEX_EFFECT_MODE_HEFE = 9;
    public static final int INDEX_EFFECT_MODE_VALENCIA = 10;
    public static final int INDEX_EFFECT_MODE_XPROII = 11;
    public static final int INDEX_EFFECT_MODE_LOFI = 12;
    public static final int INDEX_EFFECT_MODE_SIERRA = 13;
    public static final int INDEX_EFFECT_MODE_WALDEN = 14;

    private static final String LOG_TAG_FOR_API1 = "MtkCam/ParamsManager";
    private static final String KEY_STRING_FOR_API1 = "effect=";
    private static final String LOG_TAG_FOR_API2 = "MtkCam/MetadataConverter";
    private static final String KEY_STRING_FOR_API2 = "android.control.effectMode";

    private static final String[] EFFECT_NAME = {
            "none",
            "mono",
            "sepia",
            "negative",
            "posterize",
            "aqua",
            "blackboard",
            "whiteboard",
            "nashville",
            "hefe",
            "valencia",
            "xproll",
            "lofi",
            "sierra",
            "walden",
    };

    private boolean mPass = false;

    /**
     * Color effect enum value for API2.
     */
    enum ModeEnum {
        NONE(0),
        MONO(1),
        NEGATIVE(2),
        SOLARIZE(3),
        SEPIA(4),
        POSTERIZE(5),
        WHITEBOARD(6),
        BLACKBOARD(7),
        AQUA(8),

        NASHVILLE(11),
        HEFE(12),
        VALENCIA(13),
        XPROLL(14),
        LOFI(15),
        SIERRA(16),
        WALDEN(18);

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

    @Override
    protected String[] getObservedTagList(int index) {
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
    public boolean isSupported(int index) {
        if (index != INDEX_EFFECT_MODE_NONE) {
            return Utils.isFeatureSupported("com.mediatek.camera.at.matrix-display");
        } else {
            return true;
        }
    }

    @Override
    public int getObserveCount() {
        return 15;
    }

    @Override
    public String getDescription(int index) {
        return "check effect " + EFFECT_NAME[index];
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
            setAdbCommand("debug.camera.log.AppStreamMgr", "2");
        }
    }

    @Override
    protected void clearAdbCommandAfterStopped() {
        if (CameraApiHelper.getCameraApiType(null)
                == CameraDeviceManagerFactory.CameraApi.API2) {
            setAdbCommand("debug.camera.log.AppStreamMgr", "0");
        }
    }

    @Override
    protected void onLogComing(int index, String line) {
        if (CameraApiHelper.getCameraApiType(null)
                == CameraDeviceManagerFactory.CameraApi.API1) {
            int beginIndex = line.indexOf(KEY_STRING_FOR_API1) + KEY_STRING_FOR_API1.length();
            int endIndex = line.indexOf(";", beginIndex);
            String subString = line.substring(beginIndex, endIndex);
            LogHelper.d(TAG, "[onLogComing], effect=" + subString);
            mPass = EFFECT_NAME[index].equals(subString);
        } else if (CameraApiHelper.getCameraApiType(null)
                == CameraDeviceManagerFactory.CameraApi.API2) {
            int beginIndex = line.indexOf("=> ") + "=> ".length();
            String subString = line.substring(beginIndex, line.length() - 1);
            String effectName = convertEnumToString(Integer.parseInt(subString));
            LogHelper.d(TAG, "[onLogComing], android.control.effectMode="
                    + subString + "(" + effectName + ")");
            mPass = EFFECT_NAME[index].equals(effectName);
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
