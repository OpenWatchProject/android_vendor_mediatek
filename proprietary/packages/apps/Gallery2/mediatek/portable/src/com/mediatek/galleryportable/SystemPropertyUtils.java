package com.mediatek.galleryportable;

import android.os.SystemProperties;

public class SystemPropertyUtils {
    private static boolean sIsSystemPropertiesExist = false;
    private static boolean sHasChecked = false;

    public static int getInt(String key, int defaultValue) {
        if (isSystemPropertiesExist()) {
            return SystemProperties.getInt(key, defaultValue);
        } else {
            return defaultValue;
        }
    }

    public static boolean getBoolean(String key, boolean defaultValue) {
        if (isSystemPropertiesExist()) {
            return SystemProperties.getBoolean(key, defaultValue);
        } else {
            return defaultValue;
        }
    }

    public static String get(String key) {
        if (isSystemPropertiesExist()) {
            return SystemProperties.get(key);
        } else {
            return "";
        }
    }

    public static String get(String key, String defaultValue) {
        if (isSystemPropertiesExist()) {
            return SystemProperties.get(key, defaultValue);
        } else {
            return "";
        }
    }

    private static boolean isSystemPropertiesExist() {
        if (!sHasChecked) {
            try {
                Class<?> clazz =
                        SystemPropertyUtils.class.getClassLoader().loadClass(
                                "android.os.SystemProperties");
                sIsSystemPropertiesExist = (clazz != null);
                sHasChecked = true;
            } catch (ClassNotFoundException e) {
                sIsSystemPropertiesExist = false;
                sHasChecked = true;
            }
        }
        return sIsSystemPropertiesExist;
    }
}
