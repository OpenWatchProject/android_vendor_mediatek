package com.mediatek.gallery3d.util;

import android.util.Log;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

/**
 * SystemPropertyUtils.
 */
public class SystemPropertyUtils {
    private static final String TAG = "MtkGallery2/SystemPropertyUtils";
    private static boolean sIsSystemPropertiesExist = false;
    private static boolean sHasChecked = false;
    private static Class<?> sSystemPropertiesClass;
    private static Method sGetIntMethod;
    private static Method sGetMethod;

    /**
     * Get int properties.
     * @param key key
     * @param defaultValue default value
     * @return value
     */
    public static int getInt(String key, int defaultValue) {
        if (isSystemPropertiesExist() && sGetIntMethod != null) {
            try {
                return (int) sGetIntMethod.invoke(null, key, defaultValue);
            } catch (IllegalAccessException e1) {
                return defaultValue;
            } catch (InvocationTargetException e2) {
                return defaultValue;
            }
        } else {
            return defaultValue;
        }
    }

    /**
     * Get properties.
     * @param key key
     * @return value
     */
    public static String get(String key) {
        if (isSystemPropertiesExist() && sGetMethod != null) {
            try {
                return (String) sGetMethod.invoke(null, key);
            } catch (IllegalAccessException e1) {
                return "";
            } catch (InvocationTargetException e2) {
                return "";
            }
        } else {
            return "";
        }
    }

    private static boolean isSystemPropertiesExist() {
        if (!sHasChecked) {
            try {
                Class<?> sSystemPropertiesClass =
                        SystemPropertyUtils.class.getClassLoader().loadClass(
                                "android.os.SystemProperties");
                sGetIntMethod =
                        sSystemPropertiesClass.getDeclaredMethod("getInt", String.class,
                                int.class);
                sGetIntMethod.setAccessible(true);
                sGetMethod = sSystemPropertiesClass.getDeclaredMethod("get", String.class);
                sGetMethod.setAccessible(true);
                sIsSystemPropertiesExist = (sSystemPropertiesClass != null);
                sHasChecked = true;
            } catch (ClassNotFoundException e1) {
                sIsSystemPropertiesExist = false;
                sHasChecked = true;
            } catch (NoSuchMethodException e2) {
                Log.e(TAG, "<isSystemPropertiesExist> NoSuchMethodException", e2);
            }
        }
        return sIsSystemPropertiesExist;
    }
}
